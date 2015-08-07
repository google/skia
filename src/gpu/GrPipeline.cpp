/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrPipeline.h"

#include "GrCaps.h"
#include "GrGpu.h"
#include "GrPipelineBuilder.h"
#include "GrProcOptInfo.h"
#include "GrXferProcessor.h"

#include "batches/GrBatch.h"

GrPipeline::GrPipeline(const GrPipelineBuilder& pipelineBuilder,
                       const GrProcOptInfo& colorPOI,
                       const GrProcOptInfo& coveragePOI,
                       const GrCaps& caps,
                       const GrScissorState& scissorState,
                       const GrXferProcessor::DstTexture* dstTexture) {
    // Create XferProcessor from DS's XPFactory
    SkAutoTUnref<GrXferProcessor> xferProcessor(
        pipelineBuilder.getXPFactory()->createXferProcessor(
            colorPOI, coveragePOI, pipelineBuilder.hasMixedSamples(), dstTexture, caps));

    GrColor overrideColor = GrColor_ILLEGAL;
    if (colorPOI.firstEffectiveStageIndex() != 0) {
        overrideColor = colorPOI.inputColorToEffectiveStage();
    }

    GrXferProcessor::OptFlags optFlags = GrXferProcessor::kNone_OptFlags;
    if (xferProcessor) {
        fXferProcessor.reset(xferProcessor.get());

        optFlags = xferProcessor->getOptimizations(colorPOI,
                                                   coveragePOI,
                                                   pipelineBuilder.getStencil().doesWrite(),
                                                   &overrideColor,
                                                   caps);
    }

    // No need to have an override color if it isn't even going to be used.
    if (SkToBool(GrXferProcessor::kIgnoreColor_OptFlag & optFlags)) {
        overrideColor = GrColor_ILLEGAL;
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
        const GrFragmentStage& fps = pipelineBuilder.fColorStages[i];
        const GrFragmentProcessor* fp = fps.processor();
        SkNEW_APPEND_TO_TARRAY(&fFragmentStages, GrPendingFragmentStage, (fps));
        usesLocalCoords = usesLocalCoords || fp->usesLocalCoords();
        fp->gatherCoordTransforms(&fCoordTransforms);
    }

    fNumColorStages = fFragmentStages.count();
    for (int i = firstCoverageStageIdx; i < pipelineBuilder.numCoverageFragmentStages(); ++i) {
        const GrFragmentStage& fps = pipelineBuilder.fCoverageStages[i];
        const GrFragmentProcessor* fp = fps.processor();
        SkNEW_APPEND_TO_TARRAY(&fFragmentStages, GrPendingFragmentStage, (fps));
        usesLocalCoords = usesLocalCoords || fp->usesLocalCoords();
        fp->gatherCoordTransforms(&fCoordTransforms);
    }

    // Setup info we need to pass to GrPrimitiveProcessors that are used with this GrPipeline.
    fInfoForPrimitiveProcessor.fFlags = 0;
    if (!SkToBool(optFlags & GrXferProcessor::kIgnoreColor_OptFlag)) {
        fInfoForPrimitiveProcessor.fFlags |= GrPipelineInfo::kReadsColor_GrPipelineInfoFlag;
    }
    if (GrColor_ILLEGAL != overrideColor) {
        fInfoForPrimitiveProcessor.fFlags |= GrPipelineInfo::kUseOverrideColor_GrPipelineInfoFlag;
        fInfoForPrimitiveProcessor.fOverrideColor = overrideColor;
    }
    if (!SkToBool(optFlags & GrXferProcessor::kIgnoreCoverage_OptFlag)) {
        fInfoForPrimitiveProcessor.fFlags |= GrPipelineInfo::kReadsCoverage_GrPipelineInfoFlag;
    }
    if (usesLocalCoords) {
        fInfoForPrimitiveProcessor.fFlags |= GrPipelineInfo::kReadsLocalCoords_GrPipelineInfoFlag;
    }
    if (SkToBool(optFlags & GrXferProcessor::kCanTweakAlphaForCoverage_OptFlag)) {
       fInfoForPrimitiveProcessor.fFlags |=
           GrPipelineInfo::kCanTweakAlphaForCoverage_GrPipelineInfoFlag; 
    }
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

bool GrPipeline::isEqual(const GrPipeline& that, bool ignoreCoordTransforms) const {
    // If we point to the same pipeline, then we are necessarily equal
    if (this == &that) {
        return true;
    }

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

    for (int i = 0; i < this->numFragmentStages(); i++) {
        if (!this->getFragmentStage(i).processor()->isEqual(*that.getFragmentStage(i).processor(),
                                                            ignoreCoordTransforms)) {
            return false;
        }
    }
    return true;
}

