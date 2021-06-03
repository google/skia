/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrGeometryProcessor.h"
#include "src/gpu/GrProcessorAnalysis.h"
#include "src/gpu/ops/GrDrawOp.h"

GrColorFragmentProcessorAnalysis::GrColorFragmentProcessorAnalysis(
        const GrProcessorAnalysisColor& input,
        std::unique_ptr<GrFragmentProcessor> const fps[],
        int count) {
    fCompatibleWithCoverageAsAlpha = true;
    fIsOpaque = input.isOpaque();
    fUsesLocalCoords = false;
    fProcessorsToEliminate = 0;
    fOutputColorKnown = input.isConstant(&fLastKnownOutputColor);
    for (int i = 0; i < count; ++i) {
        const GrFragmentProcessor* fp = fps[i].get();
        if (fOutputColorKnown && fp->hasConstantOutputForConstantInput(fLastKnownOutputColor,
                                                                       &fLastKnownOutputColor)) {
            ++fProcessorsToEliminate;
            fIsOpaque = fLastKnownOutputColor.isOpaque();
            // We reset these flags since the earlier fragment processors are being eliminated.
            fCompatibleWithCoverageAsAlpha = true;
            fUsesLocalCoords = false;
            continue;
        }

        fOutputColorKnown = false;
        if (fIsOpaque && !fp->preservesOpaqueInput()) {
            fIsOpaque = false;
        }
        if (fCompatibleWithCoverageAsAlpha && !fp->compatibleWithCoverageAsAlpha()) {
            fCompatibleWithCoverageAsAlpha = false;
        }
        if (fp->usesVaryingCoords()) {
            fUsesLocalCoords = true;
        }
    }
}
