/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrProcessorAnalysis.h"
#include "GrGeometryProcessor.h"
#include "ops/GrDrawOp.h"

void GrColorFragmentProcessorAnalysis::analyzeProcessors(
        const GrFragmentProcessor* const* processors, int cnt) {
    for (int i = 0; i < cnt; ++i) {
        bool knowCurrentOutput = fProcessorsVisitedWithKnownOutput == fTotalProcessorsVisited;
        if (fUsesLocalCoords && !knowCurrentOutput &&
            !fAllProcessorsCompatibleWithCoverageAsAlpha && !fIsOpaque) {
            fTotalProcessorsVisited += cnt - i;
            return;
        }
        const GrFragmentProcessor* fp = processors[i];
        if (knowCurrentOutput &&
            fp->hasConstantOutputForConstantInput(fLastKnownOutputColor, &fLastKnownOutputColor)) {
            ++fProcessorsVisitedWithKnownOutput;
            fIsOpaque = fLastKnownOutputColor.isOpaque();
            // We reset these since the caller is expected to not use the earlier fragment
            // processors.
            fAllProcessorsCompatibleWithCoverageAsAlpha = true;
            fUsesLocalCoords = false;
        } else {
            if (fIsOpaque && !fp->preservesOpaqueInput()) {
                fIsOpaque = false;
            }
            if (fAllProcessorsCompatibleWithCoverageAsAlpha &&
                !fp->compatibleWithCoverageAsAlpha()) {
                fAllProcessorsCompatibleWithCoverageAsAlpha = false;
            }
            if (fp->usesLocalCoords()) {
                fUsesLocalCoords = true;
            }
        }
        ++fTotalProcessorsVisited;
    }
}
