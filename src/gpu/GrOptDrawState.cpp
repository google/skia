/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrOptDrawState.h"

#include "GrDrawState.h"
#include "GrDrawTargetCaps.h"
#include "GrGpu.h"
#include "GrProcOptInfo.h"
#include "GrXferProcessor.h"

GrOptDrawState::GrOptDrawState(const GrDrawState& drawState,
                               const GrGeometryProcessor* gp,
                               const GrPathProcessor* pathProc,
                               const GrDrawTargetCaps& caps,
                               const ScissorState& scissorState,
                               const GrDeviceCoordTexture* dstCopy,
                               GrGpu::DrawType drawType)
    : fFinalized(false) {
    fDrawType = drawType;

    // Copy GeometryProcesssor from DS or ODS
    if (gp) {
        SkASSERT(!pathProc);
        SkASSERT(!(GrGpu::IsPathRenderingDrawType(drawType) ||
                   GrGpu::kStencilPath_DrawType == drawType));
        fGeometryProcessor.reset(gp);
        fPrimitiveProcessor.reset(gp);
    } else {
        SkASSERT(!gp && pathProc && (GrGpu::IsPathRenderingDrawType(drawType) ||
                               GrGpu::kStencilPath_DrawType == drawType));
        fPrimitiveProcessor.reset(pathProc);
    }


    const GrProcOptInfo& colorPOI = drawState.colorProcInfo(fPrimitiveProcessor);
    const GrProcOptInfo& coveragePOI = drawState.coverageProcInfo(fPrimitiveProcessor);
    
    fColor = colorPOI.inputColorToEffectiveStage();
    // TODO fix this when coverage stages work correctly
    // fCoverage = coveragePOI.inputColorToEffectiveStage();
    fCoverage = fPrimitiveProcessor->coverage();

    // Create XferProcessor from DS's XPFactory
    SkAutoTUnref<GrXferProcessor> xferProcessor(
        drawState.getXPFactory()->createXferProcessor(colorPOI, coveragePOI));

    GrXferProcessor::OptFlags optFlags;
    if (xferProcessor) {
        fXferProcessor.reset(xferProcessor.get());

        optFlags = xferProcessor->getOptimizations(colorPOI,
                                                   coveragePOI,
                                                   drawState.isCoverageDrawing(),
                                                   drawState.isColorWriteDisabled(),
                                                   drawState.getStencil().doesWrite(),
                                                   &fColor,
                                                   &fCoverage,
                                                   caps);
    }

    // When path rendering the stencil settings are not always set on the draw state
    // so we must check the draw type. In cases where we will skip drawing we simply return a
    // null GrOptDrawState.
    if (!xferProcessor || ((GrXferProcessor::kSkipDraw_OptFlag & optFlags) &&
                           GrGpu::kStencilPath_DrawType != drawType)) {
        // Set the fields that don't default init and return. The lack of a render target will
        // indicate that this can be skipped.
        fFlags = 0;
        fDrawFace = GrDrawState::kInvalid_DrawFace;
        fViewMatrix.reset();
        return;
    }

    fRenderTarget.reset(drawState.fRenderTarget.get());
    SkASSERT(fRenderTarget);
    fScissorState = scissorState;
    fViewMatrix = drawState.getViewMatrix();
    fStencilSettings = drawState.getStencil();
    fDrawFace = drawState.getDrawFace();
    // TODO move this out of optDrawState
    if (dstCopy) {
        fDstCopy = *dstCopy;
    }

    fFlags = 0;
    if (drawState.isHWAntialias()) {
        fFlags |= kHWAA_Flag;
    }
    if (drawState.isColorWriteDisabled()) {
        fFlags |= kDisableColorWrite_Flag;
    }
    if (drawState.isDither()) {
        fFlags |= kDither_Flag;
    }

    fDescInfo.fHasVertexColor = gp && gp->hasVertexColor();

    fDescInfo.fHasVertexCoverage = gp && gp->hasVertexCoverage();

    bool hasLocalCoords = gp && gp->hasLocalCoords();

    int firstColorStageIdx = colorPOI.firstEffectiveStageIndex();
    fDescInfo.fInputColorIsUsed = colorPOI.inputColorIsUsed();
    if (colorPOI.removeVertexAttrib()) {
        fDescInfo.fHasVertexColor = false;
    }

    // TODO: Once we can handle single or four channel input into coverage stages then we can use
    // drawState's coverageProcInfo (like color above) to set this initial information.
    int firstCoverageStageIdx = 0;
    fDescInfo.fInputCoverageIsUsed = true;

    GrXferProcessor::BlendInfo blendInfo;
    fXferProcessor->getBlendInfo(&blendInfo);

    this->adjustProgramFromOptimizations(drawState, optFlags, colorPOI, coveragePOI,
                                         &firstColorStageIdx, &firstCoverageStageIdx);

    fDescInfo.fRequiresLocalCoordAttrib = hasLocalCoords;

    // Copy Stages from DS to ODS
    for (int i = firstColorStageIdx; i < drawState.numColorStages(); ++i) {
        SkNEW_APPEND_TO_TARRAY(&fFragmentStages,
                               GrPendingFragmentStage,
                               (drawState.fColorStages[i], hasLocalCoords));
    }

    fNumColorStages = fFragmentStages.count();
    for (int i = firstCoverageStageIdx; i < drawState.numCoverageStages(); ++i) {
        SkNEW_APPEND_TO_TARRAY(&fFragmentStages,
                               GrPendingFragmentStage,
                               (drawState.fCoverageStages[i], hasLocalCoords));
    }

    // let the GP init the batch tracker
    if (gp) {
        GrGeometryProcessor::InitBT init;
        init.fOutputColor = fDescInfo.fInputColorIsUsed;
        init.fOutputCoverage = fDescInfo.fInputCoverageIsUsed;
        init.fColor = this->getColor();
        init.fCoverage = this->getCoverage();
        fGeometryProcessor->initBatchTracker(&fBatchTracker, init);
    }
}

void GrOptDrawState::adjustProgramFromOptimizations(const GrDrawState& ds,
                                                    GrXferProcessor::OptFlags flags,
                                                    const GrProcOptInfo& colorPOI,
                                                    const GrProcOptInfo& coveragePOI,
                                                    int* firstColorStageIdx,
                                                    int* firstCoverageStageIdx) {
    fDescInfo.fReadsDst = false;
    fDescInfo.fReadsFragPosition = false;

    if (flags & GrXferProcessor::kClearColorStages_OptFlag ||
        flags & GrXferProcessor::kOverrideColor_OptFlag) {
        fDescInfo.fInputColorIsUsed = true;
        *firstColorStageIdx = ds.numColorStages();
        fDescInfo.fHasVertexColor = false;
    } else {
        fDescInfo.fReadsDst = colorPOI.readsDst();
        fDescInfo.fReadsFragPosition = colorPOI.readsFragPosition();
    }

    if (flags & GrXferProcessor::kClearCoverageStages_OptFlag) {
        fDescInfo.fInputCoverageIsUsed = true;
        *firstCoverageStageIdx = ds.numCoverageStages();
        fDescInfo.fHasVertexCoverage = false;
    } else {
        if (coveragePOI.readsDst()) {
            fDescInfo.fReadsDst = true;
        }
        if (coveragePOI.readsFragPosition()) {
            fDescInfo.fReadsFragPosition = true;
        }
    }
}

void GrOptDrawState::finalize(GrGpu* gpu) {
    gpu->buildProgramDesc(*this, fDescInfo, fDrawType, &fDesc);
    fFinalized = true;
}

////////////////////////////////////////////////////////////////////////////////

bool GrOptDrawState::operator== (const GrOptDrawState& that) const {
    if (fDescInfo != that.fDescInfo) {
        return false;
    }

    if (!fDescInfo.fHasVertexColor && this->fColor != that.fColor) {
        return false;
    }

    if (this->getRenderTarget() != that.getRenderTarget() ||
        this->fFragmentStages.count() != that.fFragmentStages.count() ||
        this->fNumColorStages != that.fNumColorStages ||
        this->fScissorState != that.fScissorState ||
        !this->fViewMatrix.cheapEqualTo(that.fViewMatrix) ||
        this->fDrawType != that.fDrawType ||
        this->fFlags != that.fFlags ||
        this->fStencilSettings != that.fStencilSettings ||
        this->fDrawFace != that.fDrawFace ||
        this->fDstCopy.texture() != that.fDstCopy.texture()) {
        return false;
    }

    if (!fDescInfo.fHasVertexCoverage && this->fCoverage != that.fCoverage) {
        return false;
    }

    if (this->hasGeometryProcessor()) {
        if (!that.hasGeometryProcessor()) {
            return false;
        } else if (!this->getGeometryProcessor()->isEqual(*that.getGeometryProcessor())) {
            return false;
        }
    } else if (that.hasGeometryProcessor()) {
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

