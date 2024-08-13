/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/ganesh/GrProcessorSet.h"

#include "include/private/base/SkDebug.h"
#include "src/gpu/ganesh/GrAppliedClip.h"
#include "src/gpu/ganesh/GrPaint.h"
#include "src/gpu/ganesh/GrXferProcessor.h"
#include "src/gpu/ganesh/effects/GrPorterDuffXferProcessor.h"

const GrProcessorSet& GrProcessorSet::EmptySet() {
    static GrProcessorSet gEmpty(GrProcessorSet::Empty::kEmpty);
    return gEmpty;
}

GrProcessorSet GrProcessorSet::MakeEmptySet() {
    return GrProcessorSet(GrProcessorSet::Empty::kEmpty);
}

GrProcessorSet::GrProcessorSet(GrPaint&& paint) : fXP(paint.getXPFactory()) {
    fColorFragmentProcessor = std::move(paint.fColorFragmentProcessor);
    fCoverageFragmentProcessor = std::move(paint.fCoverageFragmentProcessor);

    SkDEBUGCODE(paint.fAlive = false;)
}

GrProcessorSet::GrProcessorSet(SkBlendMode mode) : fXP(GrXPFactory::FromBlendMode(mode)) {}

GrProcessorSet::GrProcessorSet(std::unique_ptr<GrFragmentProcessor> colorFP)
        : fXP((const GrXPFactory*)nullptr) {
    SkASSERT(colorFP);
    fColorFragmentProcessor = std::move(colorFP);
}

GrProcessorSet::GrProcessorSet(GrProcessorSet&& that)
        : fColorFragmentProcessor(std::move(that.fColorFragmentProcessor))
        , fCoverageFragmentProcessor(std::move(that.fCoverageFragmentProcessor))
        , fXP(std::move(that.fXP))
        , fFlags(that.fFlags) {}

GrProcessorSet::~GrProcessorSet() {
    if (this->isFinalized() && this->xferProcessor()) {
        this->xferProcessor()->unref();
    }
}

#if defined(GPU_TEST_UTILS)
SkString GrProcessorSet::dumpProcessors() const {
    SkString result;
    if (this->hasColorFragmentProcessor()) {
        result.append("Color Fragment Processor:\n");
        result += this->colorFragmentProcessor()->dumpTreeInfo();
    } else {
        result.append("No color fragment processor.\n");
    }
    if (this->hasCoverageFragmentProcessor()) {
        result.append("Coverage Fragment Processor:\n");
        result += this->coverageFragmentProcessor()->dumpTreeInfo();
    } else {
        result.append("No coverage fragment processors.\n");
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
    if (((fFlags ^ that.fFlags) & ~kFinalized_Flag) ||
        this->hasColorFragmentProcessor() != that.hasColorFragmentProcessor() ||
        this->hasCoverageFragmentProcessor() != that.hasCoverageFragmentProcessor()) {
        return false;
    }

    if (this->hasColorFragmentProcessor()) {
        if (!colorFragmentProcessor()->isEqual(*that.colorFragmentProcessor())) {
            return false;
        }
    }

    if (this->hasCoverageFragmentProcessor()) {
        if (!coverageFragmentProcessor()->isEqual(*that.coverageFragmentProcessor())) {
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
        const GrAppliedClip* clip, const GrUserStencilSettings* userStencil,
        const GrCaps& caps, GrClampType clampType, SkPMColor4f* overrideInputColor) {
    SkASSERT(!this->isFinalized());

    GrProcessorSet::Analysis analysis;
    analysis.fCompatibleWithCoverageAsAlpha = GrProcessorAnalysisCoverage::kLCD != coverageInput;

    GrColorFragmentProcessorAnalysis colorAnalysis(colorInput, &fColorFragmentProcessor,
                                                   this->hasColorFragmentProcessor() ? 1 : 0);
    bool hasCoverageFP = this->hasCoverageFragmentProcessor();
    bool coverageUsesLocalCoords = false;
    if (hasCoverageFP) {
        if (!fCoverageFragmentProcessor->compatibleWithCoverageAsAlpha()) {
            analysis.fCompatibleWithCoverageAsAlpha = false;
        }
        coverageUsesLocalCoords |= fCoverageFragmentProcessor->usesSampleCoords();
    }
    if (clip && clip->hasCoverageFragmentProcessor()) {
        hasCoverageFP = true;
        const GrFragmentProcessor* clipFP = clip->coverageFragmentProcessor();
        analysis.fCompatibleWithCoverageAsAlpha &= clipFP->compatibleWithCoverageAsAlpha();
        coverageUsesLocalCoords |= clipFP->usesSampleCoords();
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
    analysis.fRequiresDstTexture = (props & GrXPFactory::AnalysisProperties::kRequiresDstTexture) ||
                                   colorAnalysis.requiresDstTexture(caps);
    analysis.fCompatibleWithCoverageAsAlpha &=
            SkToBool(props & GrXPFactory::AnalysisProperties::kCompatibleWithCoverageAsAlpha);
    analysis.fRequiresNonOverlappingDraws =
            (props & GrXPFactory::AnalysisProperties::kRequiresNonOverlappingDraws) ||
            analysis.fRequiresDstTexture;
    analysis.fUsesNonCoherentHWBlending =
            SkToBool(props & GrXPFactory::AnalysisProperties::kUsesNonCoherentHWBlending);
    analysis.fUnaffectedByDstValue =
            SkToBool(props & GrXPFactory::AnalysisProperties::kUnaffectedByDstValue);
    if (props & GrXPFactory::AnalysisProperties::kIgnoresInputColor) {
        colorFPsToEliminate = this->hasColorFragmentProcessor() ? 1 : 0;
        analysis.fInputColorType =
                static_cast<Analysis::PackedInputColorType>(Analysis::kIgnored_InputColorType);
        analysis.fUsesLocalCoords = coverageUsesLocalCoords;
    } else {
        analysis.fCompatibleWithCoverageAsAlpha &=
            colorAnalysis.allProcessorsCompatibleWithCoverageAsAlpha();
        analysis.fUsesLocalCoords = coverageUsesLocalCoords || colorAnalysis.usesLocalCoords();
    }
    if (colorFPsToEliminate) {
        SkASSERT(colorFPsToEliminate == 1);
        fColorFragmentProcessor = nullptr;
    }
    analysis.fHasColorFragmentProcessor = this->hasColorFragmentProcessor();

    auto xp = GrXPFactory::MakeXferProcessor(this->xpFactory(), colorAnalysis.outputColor(),
                                             outputCoverage, caps, clampType);
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

void GrProcessorSet::visitProxies(const GrVisitProxyFunc& func) const {
    if (this->hasColorFragmentProcessor()) {
        fColorFragmentProcessor->visitProxies(func);
    }
    if (this->hasCoverageFragmentProcessor()) {
        fCoverageFragmentProcessor->visitProxies(func);
    }
}
