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
    fColorFragmentProcessorCnt = paint.numColorFragmentProcessors();
    fFragmentProcessors.reset(paint.numTotalFragmentProcessors());
    int i = 0;
    for (auto& fp : paint.fColorFragmentProcessors) {
        fFragmentProcessors[i++] = fp.release();
    }
    for (auto& fp : paint.fCoverageFragmentProcessors) {
        fFragmentProcessors[i++] = fp.release();
    }
    fFlags = 0;
    if (paint.usesDistanceVectorField()) {
        fFlags |= kUseDistanceVectorField_Flag;
    }
    if (paint.getDisableOutputConversionToSRGB()) {
        fFlags |= kDisableOutputConversionToSRGB_Flag;
    }
    if (paint.getAllowSRGBInputs()) {
        fFlags |= kAllowSRGBInputs_Flag;
    }
}

//////////////////////////////////////////////////////////////////////////////

void GrProcessorSet::FragmentProcessorAnalysis::internalReset(const GrPipelineInput& colorInput,
                                                              const GrPipelineInput coverageInput,
                                                              const GrProcessorSet& processors,
                                                              bool usesPLSDstRead,
                                                              const GrFragmentProcessor* clipFP,
                                                              const GrCaps& caps) {
    GrProcOptInfo colorInfo(colorInput);
    fUsesPLSDstRead = usesPLSDstRead;
    fCompatibleWithCoverageAsAlpha = !coverageInput.isLCDCoverage();

    const GrFragmentProcessor* const* fps = processors.fFragmentProcessors.get();
    colorInfo.analyzeProcessors(fps, processors.fColorFragmentProcessorCnt);
    fCompatibleWithCoverageAsAlpha &= colorInfo.allProcessorsCompatibleWithCoverageAsAlpha();
    fps += processors.fColorFragmentProcessorCnt;
    int n = processors.numCoverageFragmentProcessors();
    bool hasCoverageFP = n > 0;
    for (int i = 0; i < n && fCompatibleWithCoverageAsAlpha; ++i) {
        if (!fps[i]->compatibleWithCoverageAsAlpha()) {
            fCompatibleWithCoverageAsAlpha = false;
            // Other than tests that exercise atypical behavior we expect all coverage FPs to be
            // compatible with the coverage-as-alpha optimization.
            GrCapsDebugf(&caps, "Coverage FP is not compatible with coverage as alpha.\n");
            break;
        }
    }

    if (clipFP) {
        fCompatibleWithCoverageAsAlpha &= clipFP->compatibleWithCoverageAsAlpha();
        hasCoverageFP = true;
    }
    fInitialColorProcessorsToEliminate =
            colorInfo.initialProcessorsToEliminate(&fOverrideInputColor);

    bool opaque = colorInfo.isOpaque();
    if (colorInfo.hasKnownOutputColor(&fKnownOutputColor)) {
        fColorType = opaque ? ColorType::kOpaqueConstant : ColorType::kConstant;
    } else if (opaque) {
        fColorType = ColorType::kOpaque;
    } else {
        fColorType = ColorType::kUnknown;
    }

    if (coverageInput.isLCDCoverage()) {
        fCoverageType = CoverageType::kLCD;
    } else {
        fCoverageType = hasCoverageFP || !coverageInput.isSolidWhite()
                                ? CoverageType::kSingleChannel
                                : CoverageType::kNone;
    }
}

void GrProcessorSet::FragmentProcessorAnalysis::reset(const GrPipelineInput& colorInput,
                                                      const GrPipelineInput coverageInput,
                                                      const GrProcessorSet& processors,
                                                      bool usesPLSDstRead,
                                                      const GrAppliedClip& appliedClip,
                                                      const GrCaps& caps) {
    this->internalReset(colorInput, coverageInput, processors, usesPLSDstRead,
                        appliedClip.clipCoverageFragmentProcessor(), caps);
}

GrProcessorSet::FragmentProcessorAnalysis::FragmentProcessorAnalysis(
        const GrPipelineInput& colorInput, const GrPipelineInput coverageInput, const GrCaps& caps)
        : FragmentProcessorAnalysis() {
    this->internalReset(colorInput, coverageInput, GrProcessorSet(GrPaint()), false, nullptr, caps);
}
