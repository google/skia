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
    static GrProcessorSet gEmpty(GrProcessorSet::Empty::kEmpty);
    return gEmpty;
}

GrProcessorSet GrProcessorSet::MakeEmptySet() {
    return GrProcessorSet(GrProcessorSet::Empty::kEmpty);
}

GrProcessorSet::GrProcessorSet(GrPaint&& paint)
        : fHeadColorFragmentProcessor(paint.detachColorFragmentProcessors())
        , fHeadCoverageFragmentProcessor(paint.detachCoverageFragmentProcessors())
        , fXP(paint.getXPFactory())
        , fFlags(0) {}

GrProcessorSet::GrProcessorSet(SkBlendMode mode)
        : fXP(SkBlendMode_AsXPFactory(mode))
        , fFlags(0) {}

GrProcessorSet::GrProcessorSet(gr_fp<GrFragmentProcessor> colorFP)
        : fHeadColorFragmentProcessor(std::move(colorFP))
        , fXP((const GrXPFactory*)nullptr)
        , fFlags(0) {}

GrProcessorSet::~GrProcessorSet() {
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
    if (fHeadColorFragmentProcessor || fHeadCoverageFragmentProcessor) {
        if (fHeadColorFragmentProcessor) {
            result.append("Color Fragment Processors:\n");
            for (auto fp : GrFragmentProcessor::Series(fHeadColorFragmentProcessor.get())) {
                result += dump_fragment_processor_tree(fp, 1);
            }
        } else {
            result.append("No color fragment processors.\n");
        }
        if (fHeadCoverageFragmentProcessor) {
            result.append("Coverage Fragment Processors:\n");
            for (auto fp : GrFragmentProcessor::Series(fHeadCoverageFragmentProcessor.get())) {
                result += dump_fragment_processor_tree(fp, 1);
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
    if (((fFlags ^ that.fFlags) & ~kFinalized_Flag)) {
        return false;
    }
    auto compareFPs = [](const GrFragmentProcessor* thisFP, const GrFragmentProcessor* thatFP) {
        while (thisFP || thatFP) {
            if (!thisFP || !thatFP) {
                return false;
            }
            if (!thisFP->isEqual(*thatFP)) {
                return false;
            }
        }
        return true;
    };

    if (!compareFPs(this->fHeadColorFragmentProcessor.get(),
                    that.fHeadColorFragmentProcessor.get())) {
        return false;
    }
    if (!compareFPs(this->fHeadCoverageFragmentProcessor.get(),
                    that.fHeadCoverageFragmentProcessor.get())) {
        return false;
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

    GrProcessorSet::Analysis analysis;
    analysis.fCompatibleWithCoverageAsAlpha = GrProcessorAnalysisCoverage::kLCD != coverageInput;

    const GrFragmentProcessor* clipFP = clip ? clip->clipCoverageFragmentProcessor() : nullptr;
    GrColorFragmentProcessorAnalysis colorAnalysis(colorInput, fHeadColorFragmentProcessor.get());
    analysis.fCompatibleWithCoverageAsAlpha &= colorAnalysis.compatibleWithAlphaAsCoverage();
    bool hasCoverageFP = SkToBool(fHeadCoverageFragmentProcessor);
    bool coverageUsesLocalCoords = false;
    for (auto coverageFP : GrFragmentProcessor::Series(fHeadCoverageFragmentProcessor.get())) {
        if (!coverageFP->compatibleWithCoverageAsAlpha()) {
            analysis.fCompatibleWithCoverageAsAlpha = false;
            // Other than tests that exercise atypical behavior we expect all coverage FPs to be
            // compatible with the coverage-as-alpha optimization.
            GrCapsDebugf(&caps, "Coverage FP is not compatible with coverage as alpha.\n");
        }
        coverageUsesLocalCoords |= coverageFP->usesLocalCoords();
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
    if (!fHeadCoverageFragmentProcessor && GrProcessorAnalysisCoverage::kNone == coverageInput) {
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
        colorFPsToEliminate = SK_MaxS32;
        analysis.fInputColorType =
                static_cast<Analysis::PackedInputColorType>(Analysis::kIgnored_InputColorType);
        analysis.fUsesLocalCoords = coverageUsesLocalCoords;
    } else {
        analysis.fUsesLocalCoords = coverageUsesLocalCoords | colorAnalysis.usesLocalCoords();
    }
    while (fHeadColorFragmentProcessor && colorFPsToEliminate-- > 0) {
        fHeadColorFragmentProcessor = fHeadColorFragmentProcessor->splitSeries();
    }
    for (auto fp : GrFragmentProcessor::Series(fHeadColorFragmentProcessor.get())) {
        fp->markPendeningExecution();
    }
    for (auto fp : GrFragmentProcessor::Series(fHeadCoverageFragmentProcessor.get())) {
        fp->markPendeningExecution();
    }
    auto xp = GrXPFactory::MakeXferProcessor(this->xpFactory(), colorAnalysis.outputColor(),
                                             outputCoverage, isMixedSamples, caps);
    fXP.fProcessor = xp.release();

    fFlags |= kFinalized_Flag;
    analysis.fIsInitialized = true;
    return analysis;
}
