/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrProcOptInfo.h"
#include "GrGeometryProcessor.h"
#include "ops/GrDrawOp.h"

void GrProcOptInfo::analyzeProcessors(const GrFragmentProcessor* const* processors, int cnt) {
    for (int i = 0; i < cnt; ++i) {
        const GrFragmentProcessor* processor = processors[i];
        fInOut.resetWillUseInputColor();
        processor->computeInvariantOutput(&fInOut);
        SkDEBUGCODE(fInOut.validate());
        if (!fInOut.willUseInputColor()) {
            fFirstEffectiveProcessorIndex = i;
            fInputColorIsUsed = false;
        }
        if (kRGBA_GrColorComponentFlags == fInOut.validFlags()) {
            fFirstEffectiveProcessorIndex = i + 1;
            fInputColor = fInOut.color();
            fInputColorIsUsed = true;
            // Since we are clearing all previous color stages we are in a state where we have found
            // zero stages that don't multiply the inputColor.
            fInOut.resetNonMulStageFound();
        }
    }
}
