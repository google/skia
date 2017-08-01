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
        const GrProcessorAnalysisColor& input, const GrFragmentProcessor* head) {
    fCompatibleWithCoverageAsAlpha = true;
    fIsOpaque = input.isOpaque();
    fUsesLocalCoords = false;
    fProcessorsToEliminate = 0;
    GrColor color;
    if ((fKnowOutputColor = input.isConstant(&color))) {
        fLastKnownOutputColor = GrColor4f::FromGrColor(color);
    }
    for (auto fp : GrFragmentProcessor::Series(head)) {
        if (fUsesLocalCoords && !fKnowOutputColor && !fCompatibleWithCoverageAsAlpha &&
            !fIsOpaque) {
            break;
        }
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
