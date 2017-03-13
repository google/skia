/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrProcessorSet.h"
#include "GrAppliedClip.h"
#include "GrCaps.h"
#include "GrProcOptInfo.h"

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

//////////////////////////////////////////////////////////////////////////////

void GrProcessorSet::FragmentProcessorAnalysis::internalInit(const GrPipelineInput& colorInput,
                                                             const GrPipelineInput coverageInput,
                                                             const GrProcessorSet& processors,
                                                             const GrFragmentProcessor* clipFP,
                                                             const GrCaps& caps) {
    GrProcOptInfo colorInfo(colorInput);
    fCompatibleWithCoverageAsAlpha = !coverageInput.isLCDCoverage();
    fValidInputColor = colorInput.isConstant(&fInputColor);

    const GrFragmentProcessor* const* fps = processors.fFragmentProcessors.get();
    colorInfo.analyzeProcessors(fps, processors.fColorFragmentProcessorCnt);
    fCompatibleWithCoverageAsAlpha &= colorInfo.allProcessorsCompatibleWithCoverageAsAlpha();
    fps += processors.fColorFragmentProcessorCnt;
    int n = processors.numCoverageFragmentProcessors();
    bool hasCoverageFP = n > 0;
    fUsesLocalCoords = colorInfo.usesLocalCoords();
    for (int i = 0; i < n; ++i) {
        if (!fps[i]->compatibleWithCoverageAsAlpha()) {
            fCompatibleWithCoverageAsAlpha = false;
            // Other than tests that exercise atypical behavior we expect all coverage FPs to be
            // compatible with the coverage-as-alpha optimization.
            GrCapsDebugf(&caps, "Coverage FP is not compatible with coverage as alpha.\n");
        }
        fUsesLocalCoords |= fps[i]->usesLocalCoords();
    }

    if (clipFP) {
        fCompatibleWithCoverageAsAlpha &= clipFP->compatibleWithCoverageAsAlpha();
        fUsesLocalCoords |= clipFP->usesLocalCoords();
        hasCoverageFP = true;
    }
    fInitialColorProcessorsToEliminate = colorInfo.initialProcessorsToEliminate(&fInputColor);
    fValidInputColor |= SkToBool(fInitialColorProcessorsToEliminate);

    bool opaque = colorInfo.isOpaque();
    if (colorInfo.hasKnownOutputColor(&fKnownOutputColor)) {
        fOutputColorType = static_cast<unsigned>(opaque ? ColorType::kOpaqueConstant
                                                        : ColorType::kConstant);
    } else if (opaque) {
        fOutputColorType = static_cast<unsigned>(ColorType::kOpaque);
    } else {
        fOutputColorType = static_cast<unsigned>(ColorType::kUnknown);
    }

    if (coverageInput.isLCDCoverage()) {
        fOutputCoverageType = static_cast<unsigned>(CoverageType::kLCD);
    } else {
        fOutputCoverageType = hasCoverageFP || !coverageInput.isSolidWhite()
                                      ? static_cast<unsigned>(CoverageType::kSingleChannel)
                                      : static_cast<unsigned>(CoverageType::kNone);
    }
}

void GrProcessorSet::FragmentProcessorAnalysis::init(const GrPipelineInput& colorInput,
                                                     const GrPipelineInput coverageInput,
                                                     const GrProcessorSet& processors,
                                                     const GrAppliedClip* appliedClip,
                                                     const GrCaps& caps) {
    const GrFragmentProcessor* clipFP =
            appliedClip ? appliedClip->clipCoverageFragmentProcessor() : nullptr;
    this->internalInit(colorInput, coverageInput, processors, clipFP, caps);
    fIsInitializedWithProcessorSet = true;
}

GrProcessorSet::FragmentProcessorAnalysis::FragmentProcessorAnalysis(
        const GrPipelineInput& colorInput, const GrPipelineInput coverageInput, const GrCaps& caps)
        : FragmentProcessorAnalysis() {
    this->internalInit(colorInput, coverageInput, GrProcessorSet(GrPaint()), nullptr, caps);
}
