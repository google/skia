/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrProcessorSet.h"
#include "GrAppliedClip.h"
#include "GrCaps.h"
#include "GrUserStencilSettings.h"
#include "GrXferProcessor.h"
#include "SkBlendModePriv.h"
#include "effects/GrPorterDuffXferProcessor.h"

const GrProcessorSet& GrProcessorSet::EmptySet() {
    static GrProcessorSet gEmpty(GrProcessorSet::Empty::kEmpty);
    return gEmpty;
}

GrProcessorSet GrProcessorSet::MakeEmptySet() {
    return GrProcessorSet(GrProcessorSet::Empty::kEmpty);
}

GrProcessorSet::GrProcessorSet(GrPaint&& paint) : fXP(paint.getXPFactory()) {
    fFlags = 0;
    if (paint.numColorFragmentProcessors() <= kMaxColorProcessors) {
        fColorFragmentProcessorCnt = paint.numColorFragmentProcessors();
        fFragmentProcessors.reset(paint.numTotalFragmentProcessors());
        int i = 0;
        for (auto& fp : paint.fColorFragmentProcessors) {
            SkASSERT(fp.get());
            fFragmentProcessors[i++] = std::move(fp);
        }
        for (auto& fp : paint.fCoverageFragmentProcessors) {
            SkASSERT(fp.get());
            fFragmentProcessors[i++] = std::move(fp);
        }
    } else {
        SkDebugf("Insane number of color fragment processors in paint. Dropping all processors.");
        fColorFragmentProcessorCnt = 0;
    }
    SkDEBUGCODE(paint.fAlive = false;)
}

GrProcessorSet::GrProcessorSet(SkBlendMode mode)
        : fXP(SkBlendMode_AsXPFactory(mode))
        , fColorFragmentProcessorCnt(0)
        , fFragmentProcessorOffset(0)
        , fFlags(0) {}

GrProcessorSet::GrProcessorSet(std::unique_ptr<GrFragmentProcessor> colorFP)
        : fFragmentProcessors(1)
        , fXP((const GrXPFactory*)nullptr)
        , fColorFragmentProcessorCnt(1)
        , fFragmentProcessorOffset(0)
        , fFlags(0) {
    SkASSERT(colorFP);
    fFragmentProcessors[0] = std::move(colorFP);
}

GrProcessorSet::GrProcessorSet(GrProcessorSet&& that)
        : fXP(std::move(that.fXP))
        , fColorFragmentProcessorCnt(that.fColorFragmentProcessorCnt)
        , fFragmentProcessorOffset(0)
        , fFlags(that.fFlags) {
    fFragmentProcessors.reset(that.fFragmentProcessors.count() - that.fFragmentProcessorOffset);
    for (int i = 0; i < fFragmentProcessors.count(); ++i) {
        fFragmentProcessors[i] =
                std::move(that.fFragmentProcessors[i + that.fFragmentProcessorOffset]);
    }
    that.fColorFragmentProcessorCnt = 0;
    that.fFragmentProcessors.reset(0);
}

GrProcessorSet::~GrProcessorSet() {
    if (this->isFinalized() && this->xferProcessor()) {
        this->xferProcessor()->unref();
    }
}

#ifdef SK_DEBUG
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
#endif

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

GrProcessorSet::Analysis GrProcessorSet::finalize(
        const GrProcessorAnalysisColor& colorInput, const GrProcessorAnalysisCoverage coverageInput,
        const GrAppliedClip* clip, const GrUserStencilSettings* userStencil, GrFSAAType fsaaType,
        const GrCaps& caps, GrClampType clampType, SkPMColor4f* overrideInputColor) {
    SkASSERT(!this->isFinalized());
    SkASSERT(!fFragmentProcessorOffset);

    GrProcessorSet::Analysis analysis;
    analysis.fCompatibleWithCoverageAsAlpha = GrProcessorAnalysisCoverage::kLCD != coverageInput;

    const std::unique_ptr<const GrFragmentProcessor>* fps =
            fFragmentProcessors.get() + fFragmentProcessorOffset;
    GrColorFragmentProcessorAnalysis colorAnalysis(
            colorInput, unique_ptr_address_as_pointer_address(fps), fColorFragmentProcessorCnt);
    analysis.fCompatibleWithCoverageAsAlpha &=
            colorAnalysis.allProcessorsCompatibleWithCoverageAsAlpha();
    fps += fColorFragmentProcessorCnt;
    int n = this->numCoverageFragmentProcessors();
    bool hasCoverageFP = n > 0;
    bool coverageUsesLocalCoords = false;
    for (int i = 0; i < n; ++i) {
        if (!fps[i]->compatibleWithCoverageAsAlpha()) {
            analysis.fCompatibleWithCoverageAsAlpha = false;
        }
        coverageUsesLocalCoords |= fps[i]->usesLocalCoords();
    }
    if (clip) {
        hasCoverageFP = hasCoverageFP || clip->numClipCoverageFragmentProcessors();
        for (int i = 0; i < clip->numClipCoverageFragmentProcessors(); ++i) {
            const GrFragmentProcessor* clipFP = clip->clipCoverageFragmentProcessor(i);
            clipFP->markPendingExecution();
            analysis.fCompatibleWithCoverageAsAlpha &= clipFP->compatibleWithCoverageAsAlpha();
            coverageUsesLocalCoords |= clipFP->usesLocalCoords();
        }
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
            this->xpFactory(), colorAnalysis.outputColor(), outputCoverage, caps, clampType);
    if (!this->numCoverageFragmentProcessors() &&
        GrProcessorAnalysisCoverage::kNone == coverageInput) {
    }
    analysis.fRequiresDstTexture =
            SkToBool(props & GrXPFactory::AnalysisProperties::kRequiresDstTexture);
    analysis.fCompatibleWithCoverageAsAlpha &=
            SkToBool(props & GrXPFactory::AnalysisProperties::kCompatibleWithCoverageAsAlpha);
    analysis.fRequiresNonOverlappingDraws = SkToBool(
            props & GrXPFactory::AnalysisProperties::kRequiresNonOverlappingDraws);
    if (props & GrXPFactory::AnalysisProperties::kIgnoresInputColor) {
        colorFPsToEliminate = this->numColorFragmentProcessors();
        analysis.fInputColorType =
                static_cast<Analysis::PackedInputColorType>(Analysis::kIgnored_InputColorType);
        analysis.fUsesLocalCoords = coverageUsesLocalCoords;
    } else {
        analysis.fUsesLocalCoords = coverageUsesLocalCoords | colorAnalysis.usesLocalCoords();
    }
    for (int i = 0; i < colorFPsToEliminate; ++i) {
        fFragmentProcessors[i].reset(nullptr);
    }
    for (int i = colorFPsToEliminate; i < fFragmentProcessors.count(); ++i) {
        fFragmentProcessors[i]->markPendingExecution();
    }
    fFragmentProcessorOffset = colorFPsToEliminate;
    fColorFragmentProcessorCnt -= colorFPsToEliminate;
    analysis.fHasColorFragmentProcessor = (fColorFragmentProcessorCnt != 0);

    bool hasMixedSampledCoverage = (GrFSAAType::kMixedSamples == fsaaType)
            && !userStencil->testAlwaysPasses((clip) ? clip->hasStencilClip() : false);
    auto xp = GrXPFactory::MakeXferProcessor(this->xpFactory(), colorAnalysis.outputColor(),
                                             outputCoverage, hasMixedSampledCoverage, caps,
                                             clampType);
    fXP.fProcessor = xp.release();

    fFlags |= kFinalized_Flag;
    analysis.fIsInitialized = true;
#ifdef SK_DEBUG
    bool hasXferBarrier =
            fXP.fProcessor &&
            GrXferBarrierType::kNone_GrXferBarrierType != fXP.fProcessor->xferBarrierType(caps);
    bool needsNonOverlappingDraws = analysis.fRequiresDstTexture || hasXferBarrier;
    SkASSERT(analysis.fRequiresNonOverlappingDraws == needsNonOverlappingDraws);
#endif
    return analysis;
}
