/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrProcOptInfo.h"

#include "GrGeometryProcessor.h"

#include "batches/GrDrawBatch.h"

void GrProcOptInfo::calcColorWithBatch(const GrDrawBatch* batch,
                                       const GrFragmentProcessor* const processors[],
                                       int cnt) {
    GrInitInvariantOutput out;
    batch->getInvariantOutputColor(&out);
    fInOut.reset(out);
    this->internalCalc(processors, cnt, batch->willReadFragmentPosition());
}

void GrProcOptInfo::calcCoverageWithBatch(const GrDrawBatch* batch,
                                          const GrFragmentProcessor* const processors[],
                                          int cnt) {
    GrInitInvariantOutput out;
    batch->getInvariantOutputCoverage(&out);
    fInOut.reset(out);
    this->internalCalc(processors, cnt, batch->willReadFragmentPosition());
}

void GrProcOptInfo::calcColorWithPrimProc(const GrPrimitiveProcessor* primProc,
                                          const GrFragmentProcessor * const processors[],
                                          int cnt) {
    GrInitInvariantOutput out;
    primProc->getInvariantOutputColor(&out);
    fInOut.reset(out);
    this->internalCalc(processors, cnt, primProc->willReadFragmentPosition());
}

void GrProcOptInfo::calcCoverageWithPrimProc(const GrPrimitiveProcessor* primProc,
                                             const GrFragmentProcessor * const processors[],
                                             int cnt) {
    GrInitInvariantOutput out;
    primProc->getInvariantOutputCoverage(&out);
    fInOut.reset(out);
    this->internalCalc(processors, cnt, primProc->willReadFragmentPosition());
}

void GrProcOptInfo::calcWithInitialValues(const GrFragmentProcessor * const processors[],
                                          int cnt,
                                          GrColor startColor,
                                          GrColorComponentFlags flags,
                                          bool areCoverageStages) {
    GrInitInvariantOutput out;
    out.fIsSingleComponent = areCoverageStages;
    out.fColor = startColor;
    out.fValidFlags = flags;
    fInOut.reset(out);
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
