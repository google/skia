/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrProcOptInfo.h"

#include "GrFragmentProcessor.h"
#include "GrFragmentStage.h"
#include "GrGeometryProcessor.h"

void GrProcOptInfo::calcWithInitialValues(const GrFragmentStage* stages,
                                          int stageCount,
                                          GrColor startColor,
                                          GrColorComponentFlags flags,
                                          bool areCoverageStages,
                                          const GrGeometryProcessor* gp) {
    fInOut.reset(startColor, flags, areCoverageStages);
    fFirstEffectStageIndex = 0;
    fInputColorIsUsed = true;
    fInputColor = startColor;
    fRemoveVertexAttrib = false;
    fReadsDst = false;
    fReadsFragPosition = false;

    if (areCoverageStages && gp) {
        gp->computeInvariantOutput(&fInOut);
    }

    for (int i = 0; i < stageCount; ++i) {
        const GrFragmentProcessor* processor = stages[i].getProcessor();
        fInOut.resetWillUseInputColor();
        processor->computeInvariantOutput(&fInOut);
#ifdef SK_DEBUG
        fInOut.validate();
#endif
        if (!fInOut.willUseInputColor()) {
            fFirstEffectStageIndex = i;
            fInputColorIsUsed = false;
            // Reset these since we don't care if previous stages read these values
            fReadsDst = false;
            fReadsFragPosition = false;
        }
        if (processor->willReadDstColor()) {
            fReadsDst = true;
        }
        if (processor->willReadFragmentPosition()) {
            fReadsFragPosition = true;
        }
        if (kRGBA_GrColorComponentFlags == fInOut.validFlags()) {
            fFirstEffectStageIndex = i + 1;
            fInputColor = fInOut.color();
            fInputColorIsUsed = true;
            fRemoveVertexAttrib = true;
            // Since we are clearing all previous color stages we are in a state where we have found
            // zero stages that don't multiply the inputColor.
            fInOut.resetNonMulStageFound();
            // Reset these since we don't care if previous stages read these values
            fReadsDst = false;
            fReadsFragPosition = false;
        }
    }
}

