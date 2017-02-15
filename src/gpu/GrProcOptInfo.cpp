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
        bool knowCurrentOutput = fProcessorsVisitedWithKnownOutput == fTotalProcessorsVisited;
        if (!knowCurrentOutput && !fAllProcessorsCompatibleWithCoverageAsAlpha && !fIsOpaque) {
            fTotalProcessorsVisited += cnt - i;
            return;
        }
        const GrFragmentProcessor* fp = processors[i];
        if (knowCurrentOutput && fp->hasConstantOutputForConstantInput(fLastKnownOutputColor,
                                                                      &fLastKnownOutputColor)) {
            ++fProcessorsVisitedWithKnownOutput;
            fIsOpaque = fLastKnownOutputColor.isOpaque();
        } else if (fIsOpaque && !fp->preservesOpaqueInput()) {
            fIsOpaque = false;
        }
        if (fAllProcessorsCompatibleWithCoverageAsAlpha && !fp->compatibleWithCoverageAsAlpha()) {
            fAllProcessorsCompatibleWithCoverageAsAlpha = false;
        }
        ++fTotalProcessorsVisited;
    }
}
