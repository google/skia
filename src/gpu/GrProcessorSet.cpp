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

GrProcessorSet::Analysis GrProcessorSet::finalize(const GrProcessorAnalysisColor& colorInput,
                                                  const GrProcessorAnalysisCoverage coverageInput,
                                                  const GrAppliedClip* clip,
                                                  const GrCaps& caps) {
    SkASSERT(!this->isPendingExecution());
    SkASSERT(!fFragmentProcessorOffset);

    GrProcessorSet::Analysis analysis;

    const GrFragmentProcessor* clipFP = clip ? clip->clipCoverageFragmentProcessor() : nullptr;
    GrColorFragmentProcessorAnalysis colorAnalysis(colorInput);
    analysis.fCompatibleWithCoverageAsAlpha = GrProcessorAnalysisCoverage::kLCD != coverageInput;
    analysis.fValidInputColor = colorInput.isConstant(&analysis.fInputColor);

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
    int colorFPsToEliminate = colorAnalysis.initialProcessorsToEliminate(&analysis.fInputColor);
    analysis.fValidInputColor |= SkToBool(colorFPsToEliminate);

    GrProcessorAnalysisColor outputColor = colorAnalysis.outputColor();
    if (outputColor.isConstant(&analysis.fKnownOutputColor)) {
        analysis.fOutputColorType = static_cast<unsigned>(Analysis::ColorType::kConstant);
    } else if (outputColor.isOpaque()) {
        analysis.fOutputColorType = static_cast<unsigned>(Analysis::ColorType::kOpaque);
    } else {
        analysis.fOutputColorType = static_cast<unsigned>(Analysis::ColorType::kUnknown);
    }

    GrProcessorAnalysisCoverage outputCoverage;
    if (GrProcessorAnalysisCoverage::kLCD == coverageInput) {
        outputCoverage = GrProcessorAnalysisCoverage::kLCD;
    } else if (hasCoverageFP || GrProcessorAnalysisCoverage::kSingleChannel == coverageInput) {
        outputCoverage = GrProcessorAnalysisCoverage::kSingleChannel;
    } else {
        outputCoverage = GrProcessorAnalysisCoverage::kNone;
    }
    analysis.fOutputCoverageType = static_cast<unsigned>(outputCoverage);

    GrXPFactory::AnalysisProperties props = GrXPFactory::GetAnalysisProperties(
            fXPFactory, colorAnalysis.outputColor(), outputCoverage, caps);
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
    analysis.fIgnoresInputColor =
            SkToBool(props & GrXPFactory::AnalysisProperties::kIgnoresInputColor);
    analysis.fCompatibleWithCoverageAsAlpha &=
            SkToBool(props & GrXPFactory::AnalysisProperties::kCompatibleWithAlphaAsCoverage);
    analysis.fRequiresBarrierBetweenOverlappingDraws = SkToBool(
            props & GrXPFactory::AnalysisProperties::kRequiresBarrierBetweenOverlappingDraws);
    if (props & GrXPFactory::AnalysisProperties::kIgnoresInputColor) {
        colorFPsToEliminate = this->numColorFragmentProcessors();
        // If the output of the last color stage is known then the kIgnoresInputColor optimization
        // may depend upon it being the input to the xp.
        if (!outputColor.isConstant(&analysis.fInputColor)) {
            // Otherwise, the only property the XP factory could have relied upon to compute
            // kIgnoresInputColor is opaqueness.
            analysis.fInputColor = GrColor_WHITE;
        }
        analysis.fValidInputColor = true;
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
    fFlags |= kPendingExecution_Flag;
    fFragmentProcessorOffset = colorFPsToEliminate;
    fColorFragmentProcessorCnt -= colorFPsToEliminate;

    analysis.fIsInitializedWithProcessorSet = true;
    return analysis;
}
