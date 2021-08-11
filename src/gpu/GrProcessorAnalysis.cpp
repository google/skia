/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrCaps.h"
#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/GrGeometryProcessor.h"
#include "src/gpu/GrProcessorAnalysis.h"

GrColorFragmentProcessorAnalysis::GrColorFragmentProcessorAnalysis(
        const GrProcessorAnalysisColor& input,
        std::unique_ptr<GrFragmentProcessor> const fps[],
        int count) {
    fCompatibleWithCoverageAsAlpha = true;
    fIsOpaque = input.isOpaque();
    fUsesLocalCoords = false;
    fWillReadDstColor = false;
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
            fWillReadDstColor = false;
            continue;
        }

        fOutputColorKnown = false;
        if (fIsOpaque && !fp->preservesOpaqueInput()) {
            fIsOpaque = false;
        }
        if (fCompatibleWithCoverageAsAlpha && !fp->compatibleWithCoverageAsAlpha()) {
            fCompatibleWithCoverageAsAlpha = false;
        }
        if (fp->usesSampleCoords()) {
            fUsesLocalCoords = true;
        }
        if (fp->willReadDstColor()) {
            fWillReadDstColor = true;
        }
    }
}

bool GrColorFragmentProcessorAnalysis::requiresDstTexture(const GrCaps& caps) const {
    return this->willReadDstColor() && !caps.shaderCaps()->dstReadInShaderSupport();
}
