/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrProcOptInfo.h"

#include "GrGeometryProcessor.h"

#include "batches/GrBatch.h"

void GrProcOptInfo::calcColorWithBatch(const GrBatch* batch,
                                       const GrFragmentStage* stages,
                                       int stageCount) {
    GrInitInvariantOutput out;
    batch->getInvariantOutputColor(&out);
    fInOut.reset(out);
    this->internalCalc(stages, stageCount, batch->willReadFragmentPosition());
}

void GrProcOptInfo::calcCoverageWithBatch(const GrBatch* batch,
                                          const GrFragmentStage* stages,
                                          int stageCount) {
    GrInitInvariantOutput out;
    batch->getInvariantOutputCoverage(&out);
    fInOut.reset(out);
    this->internalCalc(stages, stageCount, batch->willReadFragmentPosition());
}

void GrProcOptInfo::calcColorWithPrimProc(const GrPrimitiveProcessor* primProc,
                                          const GrFragmentStage* stages,
                                          int stageCount) {
    GrInitInvariantOutput out;
    primProc->getInvariantOutputColor(&out);
    fInOut.reset(out);
    this->internalCalc(stages, stageCount, primProc->willReadFragmentPosition());
}

void GrProcOptInfo::calcCoverageWithPrimProc(const GrPrimitiveProcessor* primProc,
                                             const GrFragmentStage* stages,
                                             int stageCount) {
    GrInitInvariantOutput out;
    primProc->getInvariantOutputCoverage(&out);
    fInOut.reset(out);
    this->internalCalc(stages, stageCount, primProc->willReadFragmentPosition());
}

void GrProcOptInfo::calcWithInitialValues(const GrFragmentStage* stages,
                                          int stageCount,
                                          GrColor startColor,
                                          GrColorComponentFlags flags,
                                          bool areCoverageStages) {
    GrInitInvariantOutput out;
    out.fIsSingleComponent = areCoverageStages;
    out.fColor = startColor;
    out.fValidFlags = flags;
    fInOut.reset(out);
    this->internalCalc(stages, stageCount, false);
}

void GrProcOptInfo::internalCalc(const GrFragmentStage* stages,
                                 int stageCount,
                                 bool initWillReadFragmentPosition) {
    fFirstEffectStageIndex = 0;
    fInputColorIsUsed = true;
    fInputColor = fInOut.color();
    fReadsFragPosition = initWillReadFragmentPosition;

    for (int i = 0; i < stageCount; ++i) {
        const GrFragmentProcessor* processor = stages[i].processor();
        fInOut.resetWillUseInputColor();
        processor->computeInvariantOutput(&fInOut);
        SkDEBUGCODE(fInOut.validate());
        if (!fInOut.willUseInputColor()) {
            fFirstEffectStageIndex = i;
            fInputColorIsUsed = false;
            // Reset these since we don't care if previous stages read these values
            fReadsFragPosition = initWillReadFragmentPosition;
        }
        if (processor->willReadFragmentPosition()) {
            fReadsFragPosition = true;
        }
        if (kRGBA_GrColorComponentFlags == fInOut.validFlags()) {
            fFirstEffectStageIndex = i + 1;
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
