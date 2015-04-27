/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrPipeline.h"

#include "GrBatch.h"
#include "GrDrawTargetCaps.h"
#include "GrGpu.h"
#include "GrPipelineBuilder.h"
#include "GrProcOptInfo.h"
#include "GrXferProcessor.h"

GrPipeline::GrPipeline(const GrPipelineBuilder& pipelineBuilder,
                       const GrProcOptInfo& colorPOI,
                       const GrProcOptInfo& coveragePOI,
                       const GrDrawTargetCaps& caps,
                       const GrScissorState& scissorState,
                       const GrDeviceCoordTexture* dstCopy) {
    // Create XferProcessor from DS's XPFactory
    SkAutoTUnref<GrXferProcessor> xferProcessor(
        pipelineBuilder.getXPFactory()->createXferProcessor(colorPOI, coveragePOI, dstCopy, caps));

    GrColor overrideColor = GrColor_ILLEGAL;
    if (colorPOI.firstEffectiveStageIndex() != 0) {
        overrideColor = colorPOI.inputColorToEffectiveStage();
    }

    GrXferProcessor::OptFlags optFlags;
    if (xferProcessor) {
        fXferProcessor.reset(xferProcessor.get());

        optFlags = xferProcessor->getOptimizations(colorPOI,
                                                   coveragePOI,
                                                   pipelineBuilder.getStencil().doesWrite(),
                                                   &overrideColor,
                                                   caps);
    }

    // When path rendering the stencil settings are not always set on the GrPipelineBuilder
    // so we must check the draw type. In cases where we will skip drawing we simply return a
    // null GrPipeline.
    if (!xferProcessor || (GrXferProcessor::kSkipDraw_OptFlag & optFlags)) {
        // Set the fields that don't default init and return. The lack of a render target will
        // indicate that this can be skipped.
        fFlags = 0;
        fDrawFace = GrPipelineBuilder::kInvalid_DrawFace;
        return;
    }

    fRenderTarget.reset(pipelineBuilder.fRenderTarget.get());
    SkASSERT(fRenderTarget);
    fScissorState = scissorState;
    fStencilSettings = pipelineBuilder.getStencil();
    fDrawFace = pipelineBuilder.getDrawFace();

    fFlags = 0;
    if (pipelineBuilder.isHWAntialias()) {
        fFlags |= kHWAA_Flag;
    }
    if (pipelineBuilder.isDither()) {
        fFlags |= kDither_Flag;
    }
    if (pipelineBuilder.snapVerticesToPixelCenters()) {
        fFlags |= kSnapVertices_Flag;
    }

    int firstColorStageIdx = colorPOI.firstEffectiveStageIndex();

    // TODO: Once we can handle single or four channel input into coverage stages then we can use
    // GrPipelineBuilder's coverageProcInfo (like color above) to set this initial information.
    int firstCoverageStageIdx = 0;

    this->adjustProgramFromOptimizations(pipelineBuilder, optFlags, colorPOI, coveragePOI,
                                         &firstColorStageIdx, &firstCoverageStageIdx);

    bool usesLocalCoords = false;

    // Copy Stages from PipelineBuilder to Pipeline
    for (int i = firstColorStageIdx; i < pipelineBuilder.numColorFragmentStages(); ++i) {
        SkNEW_APPEND_TO_TARRAY(&fFragmentStages,
                               GrPendingFragmentStage,
                               (pipelineBuilder.fColorStages[i]));
        usesLocalCoords = usesLocalCoords ||
                          pipelineBuilder.fColorStages[i].processor()->usesLocalCoords();
    }

    fNumColorStages = fFragmentStages.count();
    for (int i = firstCoverageStageIdx; i < pipelineBuilder.numCoverageFragmentStages(); ++i) {
        SkNEW_APPEND_TO_TARRAY(&fFragmentStages,
                               GrPendingFragmentStage,
                               (pipelineBuilder.fCoverageStages[i]));
        usesLocalCoords = usesLocalCoords ||
                          pipelineBuilder.fCoverageStages[i].processor()->usesLocalCoords();
    }

    // let the GP init the batch tracker
    fInitBT.fColorIgnored = SkToBool(optFlags & GrXferProcessor::kIgnoreColor_OptFlag);
    fInitBT.fOverrideColor = fInitBT.fColorIgnored ? GrColor_ILLEGAL : overrideColor;
    fInitBT.fCoverageIgnored = SkToBool(optFlags & GrXferProcessor::kIgnoreCoverage_OptFlag);
    fInitBT.fUsesLocalCoords = usesLocalCoords;
    fInitBT.fCanTweakAlphaForCoverage =
        SkToBool(optFlags & GrXferProcessor::kCanTweakAlphaForCoverage_OptFlag);
}

void GrPipeline::adjustProgramFromOptimizations(const GrPipelineBuilder& pipelineBuilder,
                                                GrXferProcessor::OptFlags flags,
                                                const GrProcOptInfo& colorPOI,
                                                const GrProcOptInfo& coveragePOI,
                                                int* firstColorStageIdx,
                                                int* firstCoverageStageIdx) {
    fReadsFragPosition = fXferProcessor->willReadFragmentPosition();

    if ((flags & GrXferProcessor::kIgnoreColor_OptFlag) ||
        (flags & GrXferProcessor::kOverrideColor_OptFlag)) {
        *firstColorStageIdx = pipelineBuilder.numColorFragmentStages();
    } else {
        if (coveragePOI.readsFragPosition()) {
            fReadsFragPosition = true;
        }
    }

    if (flags & GrXferProcessor::kIgnoreCoverage_OptFlag) {
        *firstCoverageStageIdx = pipelineBuilder.numCoverageFragmentStages();
    } else {
        if (coveragePOI.readsFragPosition()) {
            fReadsFragPosition = true;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

bool GrPipeline::isEqual(const GrPipeline& that) const {
    if (this->getRenderTarget() != that.getRenderTarget() ||
        this->fFragmentStages.count() != that.fFragmentStages.count() ||
        this->fNumColorStages != that.fNumColorStages ||
        this->fScissorState != that.fScissorState ||
        this->fFlags != that.fFlags ||
        this->fStencilSettings != that.fStencilSettings ||
        this->fDrawFace != that.fDrawFace) {
        return false;
    }

    if (!this->getXferProcessor()->isEqual(*that.getXferProcessor())) {
        return false;
    }

    // The program desc comparison should have already assured that the stage counts match.
    SkASSERT(this->numFragmentStages() == that.numFragmentStages());
    for (int i = 0; i < this->numFragmentStages(); i++) {

        if (this->getFragmentStage(i) != that.getFragmentStage(i)) {
            return false;
        }
    }
    return true;
}

