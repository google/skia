/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrProcOptInfo.h"

#include "GrGeometryProcessor.h"

#include "batches/GrDrawBatch.h"

void GrProcOptInfo::calcWithInitialValues(const GrFragmentProcessor * const processors[],
                                          int cnt,
                                          GrColor startColor,
                                          GrColorComponentFlags flags,
                                          bool areCoverageStages,
                                          bool isLCD) {
    GrInitInvariantOutput out;
    out.fIsSingleComponent = areCoverageStages;
    out.fColor = startColor;
    out.fValidFlags = flags;
    out.fIsLCDCoverage = isLCD;
    fInOut.reset(out);
    this->internalCalc(processors, cnt, false);
}

void GrProcOptInfo::initUsingInvariantOutput(GrInitInvariantOutput invOutput) {
    fInOut.reset(invOutput);
}

void GrProcOptInfo::completeCalculations(const GrFragmentProcessor * const processors[], int cnt) {
    this->internalCalc(processors, cnt, false);
}

void GrProcOptInfo::internalCalc(const GrFragmentProcessor* const processors[],
                                 int cnt,
                                 bool initWillReadFragmentPosition) {
    fFirstEffectiveProcessorIndex = 0;
    fInputColorIsUsed = true;
    fInputColor = fInOut.color();
    fReadsFragPosition = initWillReadFragmentPosition;

    for (int i = 0; i < cnt; ++i) {
        const GrFragmentProcessor* processor = processors[i];
        fInOut.resetWillUseInputColor();
        processor->computeInvariantOutput(&fInOut);
        SkDEBUGCODE(fInOut.validate());
        if (!fInOut.willUseInputColor()) {
            fFirstEffectiveProcessorIndex = i;
            fInputColorIsUsed = false;
            // Reset these since we don't care if previous stages read these values
            fReadsFragPosition = initWillReadFragmentPosition;
        }
        if (processor->willReadFragmentPosition()) {
            fReadsFragPosition = true;
        }
        if (kRGBA_GrColorComponentFlags == fInOut.validFlags()) {
            fFirstEffectiveProcessorIndex = i + 1;
            fInputColor = fInOut.color();
            fInputColorIsUsed = true;
            // Since we are clearing all previous color stages we are in a state where we have found
            // zero stages that don't multiply the inputColor.
            fInOut.resetNonMulStageFound();
            // Reset these since we don't care if previous stages read these values
            fReadsFragPosition = initWillReadFragmentPosition;
        }
    }
}
