/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrProcessorAnalysis.h"
#include "GrGeometryProcessor.h"
#include "ops/GrDrawOp.h"

GrColorFragmentProcessorAnalysis::GrColorFragmentProcessorAnalysis(
        const GrProcessorAnalysisColor& input,
        const GrFragmentProcessor* const* processors,
        int cnt) {
    fCompatibleWithCoverageAsAlpha = true;
    fIsOpaque = input.isOpaque();
    fUsesLocalCoords = false;
    fProcessorsToEliminate = 0;
    fKnowOutputColor = input.isConstant(&fLastKnownOutputColor);
    for (int i = 0; i < cnt; ++i) {
        if (fUsesLocalCoords && !fKnowOutputColor && !fCompatibleWithCoverageAsAlpha &&
            !fIsOpaque) {
            break;
        }
        const GrFragmentProcessor* fp = processors[i];
        if (fKnowOutputColor &&
            fp->hasConstantOutputForConstantInput(fLastKnownOutputColor, &fLastKnownOutputColor)) {
            ++fProcessorsToEliminate;
            fIsOpaque = fLastKnownOutputColor.isOpaque();
            // We reset these since the caller is expected to not use the earlier fragment
            // processors.
            fCompatibleWithCoverageAsAlpha = true;
            fUsesLocalCoords = false;
        } else {
            fKnowOutputColor = false;
            if (fIsOpaque && !fp->preservesOpaqueInput()) {
                fIsOpaque = false;
            }
            if (fCompatibleWithCoverageAsAlpha && !fp->compatibleWithCoverageAsAlpha()) {
                fCompatibleWithCoverageAsAlpha = false;
            }
            if (fp->usesLocalCoords()) {
                fUsesLocalCoords = true;
            }
        }
    }
}
