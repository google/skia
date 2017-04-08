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

GrProcessorSet::GrProcessorSet(GrPaint&& paint) {
    fXPFactory = paint.fXPFactory;
    fFlags = 0;
    if (paint.numColorFragmentProcessors() <= kMaxColorProcessors) {
        fColorFragmentProcessorCnt = paint.numColorFragmentProcessors();
        fFragmentProcessors.reset(paint.numTotalFragmentProcessors());
        int i = 0;
        for (auto& fp : paint.fColorFragmentProcessors) {
            fFragmentProcessors[i++] = fp.release();
        }
        for (auto& fp : paint.fCoverageFragmentProcessors) {
            fFragmentProcessors[i++] = fp.release();
        }
        if (paint.usesDistanceVectorField()) {
            fFlags |= kUseDistanceVectorField_Flag;
        }
    } else {
        SkDebugf("Insane number of color fragment processors in paint. Dropping all processors.");
        fColorFragmentProcessorCnt = 0;
    }
    if (paint.getDisableOutputConversionToSRGB()) {
        fFlags |= kDisableOutputConversionToSRGB_Flag;
    }
    if (paint.getAllowSRGBInputs()) {
        fFlags |= kAllowSRGBInputs_Flag;
    }
}

GrProcessorSet::~GrProcessorSet() {
    for (int i = fFragmentProcessorOffset; i < fFragmentProcessors.count(); ++i) {
        if (this->isPendingExecution()) {
            fFragmentProcessors[i]->completedExecution();
        } else {
            fFragmentProcessors[i]->unref();
        }
    }
}

void GrProcessorSet::makePendingExecution() {
    SkASSERT(!(kPendingExecution_Flag & fFlags));
    fFlags |= kPendingExecution_Flag;
    for (int i = fFragmentProcessorOffset; i < fFragmentProcessors.count(); ++i) {
        fFragmentProcessors[i]->addPendingExecution();
        fFragmentProcessors[i]->unref();
    }
}

bool GrProcessorSet::operator==(const GrProcessorSet& that) const {
    int fpCount = this->numFragmentProcessors();
    if (((fFlags ^ that.fFlags) & ~kPendingExecution_Flag) ||
        fpCount != that.numFragmentProcessors() ||
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
    if (fXPFactory != that.fXPFactory) {
        return false;
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////////

void GrProcessorSet::Analysis::internalInit(const GrProcessorAnalysisColor& colorInput,
                                            const GrProcessorAnalysisCoverage coverageInput,
                                            const GrProcessorSet& processors,
                                            const GrFragmentProcessor* clipFP,
                                            const GrCaps& caps) {
    GrColorFragmentProcessorAnalysis colorInfo(colorInput);
    fCompatibleWithCoverageAsAlpha = GrProcessorAnalysisCoverage::kLCD != coverageInput;
    fValidInputColor = colorInput.isConstant(&fInputColor);

    const GrFragmentProcessor* const* fps =
            processors.fFragmentProcessors.get() + processors.fFragmentProcessorOffset;
    colorInfo.analyzeProcessors(fps, processors.fColorFragmentProcessorCnt);
    fCompatibleWithCoverageAsAlpha &= colorInfo.allProcessorsCompatibleWithCoverageAsAlpha();
    fps += processors.fColorFragmentProcessorCnt;
    int n = processors.numCoverageFragmentProcessors();
    bool hasCoverageFP = n > 0;
    bool coverageUsesLocalCoords = false;
    for (int i = 0; i < n; ++i) {
        if (!fps[i]->compatibleWithCoverageAsAlpha()) {
            fCompatibleWithCoverageAsAlpha = false;
            // Other than tests that exercise atypical behavior we expect all coverage FPs to be
            // compatible with the coverage-as-alpha optimization.
            GrCapsDebugf(&caps, "Coverage FP is not compatible with coverage as alpha.\n");
        }
        coverageUsesLocalCoords |= fps[i]->usesLocalCoords();
    }

    if (clipFP) {
        fCompatibleWithCoverageAsAlpha &= clipFP->compatibleWithCoverageAsAlpha();
        coverageUsesLocalCoords |= clipFP->usesLocalCoords();
        hasCoverageFP = true;
    }
    fInitialColorProcessorsToEliminate = colorInfo.initialProcessorsToEliminate(&fInputColor);
    fValidInputColor |= SkToBool(fInitialColorProcessorsToEliminate);

    GrProcessorAnalysisColor outputColor = colorInfo.outputColor();
    if (outputColor.isConstant(&fKnownOutputColor)) {
        fOutputColorType = static_cast<unsigned>(ColorType::kConstant);
    } else if (outputColor.isOpaque()) {
        fOutputColorType = static_cast<unsigned>(ColorType::kOpaque);
    } else {
        fOutputColorType = static_cast<unsigned>(ColorType::kUnknown);
    }

    GrProcessorAnalysisCoverage outputCoverage;
    if (GrProcessorAnalysisCoverage::kLCD == coverageInput) {
        outputCoverage = GrProcessorAnalysisCoverage::kLCD;
    } else if (hasCoverageFP || GrProcessorAnalysisCoverage::kSingleChannel == coverageInput) {
        outputCoverage = GrProcessorAnalysisCoverage::kSingleChannel;
    } else {
        outputCoverage = GrProcessorAnalysisCoverage::kNone;
    }
    fOutputCoverageType = static_cast<unsigned>(outputCoverage);

    GrXPFactory::AnalysisProperties props = GrXPFactory::GetAnalysisProperties(
            processors.fXPFactory, colorInfo.outputColor(), outputCoverage, caps);
    if (!processors.numCoverageFragmentProcessors() &&
        GrProcessorAnalysisCoverage::kNone == coverageInput) {
        fCanCombineOverlappedStencilAndCover = SkToBool(
                props & GrXPFactory::AnalysisProperties::kCanCombineOverlappedStencilAndCover);
    } else {
        // If we have non-clipping coverage processors we don't try to merge stencil steps as its
        // unclear whether it will be correct. We don't expect this to happen in practice.
        fCanCombineOverlappedStencilAndCover = false;
    }
    fRequiresDstTexture = SkToBool(props & GrXPFactory::AnalysisProperties::kRequiresDstTexture);
    fIgnoresInputColor = SkToBool(props & GrXPFactory::AnalysisProperties::kIgnoresInputColor);
    fCompatibleWithCoverageAsAlpha &=
            SkToBool(props & GrXPFactory::AnalysisProperties::kCompatibleWithAlphaAsCoverage);
    fRequiresBarrierBetweenOverlappingDraws = SkToBool(
            props & GrXPFactory::AnalysisProperties::kRequiresBarrierBetweenOverlappingDraws);
    if (props & GrXPFactory::AnalysisProperties::kIgnoresInputColor) {
        fInitialColorProcessorsToEliminate = processors.numColorFragmentProcessors();
        // If the output of the last color stage is known then the kIgnoresInputColor optimization
        // may depend upon it being the input to the xp.
        if (!outputColor.isConstant(&fInputColor)) {
            // Otherwise, the only property the XP factory could have relied upon to compute
            // kIgnoresInputColor is opaqueness.
            fInputColor = GrColor_WHITE;
        }
        fValidInputColor = true;
        fUsesLocalCoords = coverageUsesLocalCoords;
    } else {
        fUsesLocalCoords = coverageUsesLocalCoords | colorInfo.usesLocalCoords();
    }
}

void GrProcessorSet::Analysis::init(const GrProcessorAnalysisColor& colorInput,
                                    const GrProcessorAnalysisCoverage coverageInput,
                                    const GrProcessorSet& processors,
                                    const GrAppliedClip* appliedClip,
                                    const GrCaps& caps) {
    const GrFragmentProcessor* clipFP =
            appliedClip ? appliedClip->clipCoverageFragmentProcessor() : nullptr;
    this->internalInit(colorInput, coverageInput, processors, clipFP, caps);
    fIsInitializedWithProcessorSet = true;
}

GrProcessorSet::Analysis::Analysis(const GrProcessorAnalysisColor& colorInput,
                                   const GrProcessorAnalysisCoverage coverageInput,
                                   const GrXPFactory* factory,
                                   const GrCaps& caps)
        : Analysis() {
    GrPaint paint;
    paint.setXPFactory(factory);
    this->internalInit(colorInput, coverageInput, GrProcessorSet(std::move(paint)), nullptr, caps);
}

void GrProcessorSet::analyzeAndEliminateFragmentProcessors(
        Analysis* analysis,
        const GrProcessorAnalysisColor& colorInput,
        const GrProcessorAnalysisCoverage coverageInput,
        const GrAppliedClip* clip,
        const GrCaps& caps) {
    analysis->init(colorInput, coverageInput, *this, clip, caps);
    if (analysis->fInitialColorProcessorsToEliminate > 0) {
        for (unsigned i = 0; i < analysis->fInitialColorProcessorsToEliminate; ++i) {
            if (this->isPendingExecution()) {
                fFragmentProcessors[i + fFragmentProcessorOffset]->completedExecution();
            } else {
                fFragmentProcessors[i + fFragmentProcessorOffset]->unref();
            }
            fFragmentProcessors[i + fFragmentProcessorOffset] = nullptr;
        }
        fFragmentProcessorOffset += analysis->fInitialColorProcessorsToEliminate;
        fColorFragmentProcessorCnt -= analysis->fInitialColorProcessorsToEliminate;
        SkASSERT(fFragmentProcessorOffset + fColorFragmentProcessorCnt <=
                 fFragmentProcessors.count());
        analysis->fInitialColorProcessorsToEliminate = 0;
    }
}
