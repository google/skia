/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrProcessorSet.h"
#include "GrAppliedClip.h"
#include "GrCaps.h"
#include "GrXferProcessor.h"
#include "SkBlendModePriv.h"
#include "effects/GrPorterDuffXferProcessor.h"

const GrProcessorSet& GrProcessorSet::EmptySet() {
    static const GrProcessorSet gEmpty(GrProcessorSet::Empty::kEmpty);
    return gEmpty;
}

GrProcessorSet::GrProcessorSet(GrPaint&& paint) : fXP(paint.getXPFactory()) {
    fFlags = 0;
    if (paint.numColorFragmentProcessors() <= kMaxColorProcessors) {
        fColorFragmentProcessorCnt = paint.numColorFragmentProcessors();
        fFragmentProcessors.reset(paint.numTotalFragmentProcessors());
        int i = 0;
        for (auto& fp : paint.fColorFragmentProcessors) {
            SkASSERT(fp.get());
            fFragmentProcessors[i++] = fp.release();
        }
        for (auto& fp : paint.fCoverageFragmentProcessors) {
            SkASSERT(fp.get());
            fFragmentProcessors[i++] = fp.release();
        }
    } else {
        SkDebugf("Insane number of color fragment processors in paint. Dropping all processors.");
        fColorFragmentProcessorCnt = 0;
    }
}

GrProcessorSet::GrProcessorSet(SkBlendMode mode)
        : fXP(SkBlendMode_AsXPFactory(mode))
        , fColorFragmentProcessorCnt(0)
        , fFragmentProcessorOffset(0)
        , fFlags(0) {}

GrProcessorSet::GrProcessorSet(sk_sp<GrFragmentProcessor> colorFP)
        : fFragmentProcessors(1)
        , fXP((const GrXPFactory*)nullptr)
        , fColorFragmentProcessorCnt(1)
        , fFragmentProcessorOffset(0)
        , fFlags(0) {
    SkASSERT(colorFP);
    fFragmentProcessors[0] = colorFP.release();
}

GrProcessorSet::~GrProcessorSet() {
    for (int i = fFragmentProcessorOffset; i < fFragmentProcessors.count(); ++i) {
        if (this->isFinalized()) {
            fFragmentProcessors[i]->completedExecution();
        } else {
            fFragmentProcessors[i]->unref();
        }
    }
    if (this->isFinalized() && this->xferProcessor()) {
        this->xferProcessor()->unref();
    }
}

SkString dump_fragment_processor_tree(const GrFragmentProcessor* fp, int indentCnt) {
    SkString result;
    SkString indentString;
    for (int i = 0; i < indentCnt; ++i) {
        indentString.append("    ");
    }
    result.appendf("%s%s %s \n", indentString.c_str(), fp->name(), fp->dumpInfo().c_str());
    if (fp->numChildProcessors()) {
        for (int i = 0; i < fp->numChildProcessors(); ++i) {
            result += dump_fragment_processor_tree(&fp->childProcessor(i), indentCnt + 1);
        }
    }
    return result;
}

SkString GrProcessorSet::dumpProcessors() const {
    SkString result;
    if (this->numFragmentProcessors()) {
        if (this->numColorFragmentProcessors()) {
            result.append("Color Fragment Processors:\n");
            for (int i = 0; i < this->numColorFragmentProcessors(); ++i) {
                result += dump_fragment_processor_tree(this->colorFragmentProcessor(i), 1);
            }
        } else {
            result.append("No color fragment processors.\n");
        }
        if (this->numCoverageFragmentProcessors()) {
            result.append("Coverage Fragment Processors:\n");
            for (int i = 0; i < this->numColorFragmentProcessors(); ++i) {
                result += dump_fragment_processor_tree(this->coverageFragmentProcessor(i), 1);
            }
        } else {
            result.append("No coverage fragment processors.\n");
        }
    } else {
        result.append("No color or coverage fragment processors.\n");
    }
    if (this->isFinalized()) {
        result.append("Xfer Processor: ");
        if (this->xferProcessor()) {
            result.appendf("%s\n", this->xferProcessor()->name());
        } else {
            result.append("SrcOver\n");
        }
    } else {
        result.append("XP Factory dumping not implemented.\n");
    }
    return result;
}

bool GrProcessorSet::operator==(const GrProcessorSet& that) const {
    SkASSERT(this->isFinalized());
    SkASSERT(that.isFinalized());
    int fpCount = this->numFragmentProcessors();
    if (((fFlags ^ that.fFlags) & ~kFinalized_Flag) || fpCount != that.numFragmentProcessors() ||
        fColorFragmentProcessorCnt != that.fColorFragmentProcessorCnt) {
        return false;
    }

    for (int i = 0; i < fpCount; ++i) {
        int a = i + fFragmentProcessorOffset;
        int b = i + that.fFragmentProcessorOffset;
        if (!fFragmentProcessors[a]->isEqual(*that.fFragmentProcessors[b])) {
            return false;
        }
    }
    // Most of the time both of these are null
    if (!this->xferProcessor() && !that.xferProcessor()) {
        return true;
    }
    const GrXferProcessor& thisXP = this->xferProcessor()
                                            ? *this->xferProcessor()
                                            : GrPorterDuffXPFactory::SimpleSrcOverXP();
    const GrXferProcessor& thatXP = that.xferProcessor()
                                            ? *that.xferProcessor()
                                            : GrPorterDuffXPFactory::SimpleSrcOverXP();
    return thisXP.isEqual(thatXP);
}

GrProcessorSet::Analysis GrProcessorSet::finalize(const GrProcessorAnalysisColor& colorInput,
                                                  const GrProcessorAnalysisCoverage coverageInput,
                                                  const GrAppliedClip* clip, bool isMixedSamples,
                                                  const GrCaps& caps, GrColor* overrideInputColor) {
    SkASSERT(!this->isFinalized());
    SkASSERT(!fFragmentProcessorOffset);

    GrProcessorSet::Analysis analysis;

    const GrFragmentProcessor* clipFP = clip ? clip->clipCoverageFragmentProcessor() : nullptr;
    GrColorFragmentProcessorAnalysis colorAnalysis(colorInput);
    analysis.fCompatibleWithCoverageAsAlpha = GrProcessorAnalysisCoverage::kLCD != coverageInput;

    const GrFragmentProcessor* const* fps = fFragmentProcessors.get() + fFragmentProcessorOffset;
    colorAnalysis.analyzeProcessors(fps, fColorFragmentProcessorCnt);
    analysis.fCompatibleWithCoverageAsAlpha &=
            colorAnalysis.allProcessorsCompatibleWithCoverageAsAlpha();
    fps += fColorFragmentProcessorCnt;
    int n = this->numCoverageFragmentProcessors();
    bool hasCoverageFP = n > 0;
    bool coverageUsesLocalCoords = false;
    for (int i = 0; i < n; ++i) {
        if (!fps[i]->compatibleWithCoverageAsAlpha()) {
            analysis.fCompatibleWithCoverageAsAlpha = false;
            // Other than tests that exercise atypical behavior we expect all coverage FPs to be
            // compatible with the coverage-as-alpha optimization.
            GrCapsDebugf(&caps, "Coverage FP is not compatible with coverage as alpha.\n");
        }
        coverageUsesLocalCoords |= fps[i]->usesLocalCoords();
    }

    if (clipFP) {
        analysis.fCompatibleWithCoverageAsAlpha &= clipFP->compatibleWithCoverageAsAlpha();
        coverageUsesLocalCoords |= clipFP->usesLocalCoords();
        hasCoverageFP = true;
    }
    int colorFPsToEliminate = colorAnalysis.initialProcessorsToEliminate(overrideInputColor);
    analysis.fInputColorType = static_cast<Analysis::PackedInputColorType>(
            colorFPsToEliminate ? Analysis::kOverridden_InputColorType
                                : Analysis::kOriginal_InputColorType);

    GrProcessorAnalysisCoverage outputCoverage;
    if (GrProcessorAnalysisCoverage::kLCD == coverageInput) {
        outputCoverage = GrProcessorAnalysisCoverage::kLCD;
    } else if (hasCoverageFP || GrProcessorAnalysisCoverage::kSingleChannel == coverageInput) {
        outputCoverage = GrProcessorAnalysisCoverage::kSingleChannel;
    } else {
        outputCoverage = GrProcessorAnalysisCoverage::kNone;
    }

    GrXPFactory::AnalysisProperties props = GrXPFactory::GetAnalysisProperties(
            this->xpFactory(), colorAnalysis.outputColor(), outputCoverage, caps);
    if (!this->numCoverageFragmentProcessors() &&
        GrProcessorAnalysisCoverage::kNone == coverageInput) {
        analysis.fCanCombineOverlappedStencilAndCover = SkToBool(
                props & GrXPFactory::AnalysisProperties::kCanCombineOverlappedStencilAndCover);
    } else {
        // If we have non-clipping coverage processors we don't try to merge stencil steps as its
        // unclear whether it will be correct. We don't expect this to happen in practice.
        analysis.fCanCombineOverlappedStencilAndCover = false;
    }
    analysis.fRequiresDstTexture =
            SkToBool(props & GrXPFactory::AnalysisProperties::kRequiresDstTexture);
    analysis.fCompatibleWithCoverageAsAlpha &=
            SkToBool(props & GrXPFactory::AnalysisProperties::kCompatibleWithAlphaAsCoverage);
    analysis.fRequiresBarrierBetweenOverlappingDraws = SkToBool(
            props & GrXPFactory::AnalysisProperties::kRequiresBarrierBetweenOverlappingDraws);
    if (props & GrXPFactory::AnalysisProperties::kIgnoresInputColor) {
        colorFPsToEliminate = this->numColorFragmentProcessors();
        analysis.fInputColorType =
                static_cast<Analysis::PackedInputColorType>(Analysis::kIgnored_InputColorType);
        analysis.fUsesLocalCoords = coverageUsesLocalCoords;
    } else {
        analysis.fUsesLocalCoords = coverageUsesLocalCoords | colorAnalysis.usesLocalCoords();
    }
    for (int i = 0; i < colorFPsToEliminate; ++i) {
        fFragmentProcessors[i]->unref();
        fFragmentProcessors[i] = nullptr;
    }
    for (int i = colorFPsToEliminate; i < fFragmentProcessors.count(); ++i) {
        fFragmentProcessors[i]->addPendingExecution();
        fFragmentProcessors[i]->unref();
    }
    fFragmentProcessorOffset = colorFPsToEliminate;
    fColorFragmentProcessorCnt -= colorFPsToEliminate;

    auto xp = GrXPFactory::MakeXferProcessor(this->xpFactory(), colorAnalysis.outputColor(),
                                             outputCoverage, isMixedSamples, caps);
    fXP.fProcessor = xp.release();

    fFlags |= kFinalized_Flag;
    analysis.fIsInitialized = true;
    return analysis;
}
