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

GrOptDrawState::GrOptDrawState(const GrDrawState& drawState,
                               const GrDrawTargetCaps& caps,
                               const ScissorState& scissorState,
                               const GrDeviceCoordTexture* dstCopy,
                               GrGpu::DrawType drawType)
    : fFinalized(false) {
    fDrawType = drawType;
    GrBlendCoeff optSrcCoeff;
    GrBlendCoeff optDstCoeff;
    GrDrawState::BlendOpt blendOpt = drawState.getBlendOpt(false, &optSrcCoeff, &optDstCoeff);

    // When path rendering the stencil settings are not always set on the draw state
    // so we must check the draw type. In cases where we will skip drawing we simply return a
    // null GrOptDrawState.
    if (GrDrawState::kSkipDraw_BlendOpt == blendOpt && GrGpu::kStencilPath_DrawType != drawType) {
        // Set the fields that don't default init and return. The lack of a render target will
        // indicate that this can be skipped.
        fFlags = 0;
        fDrawFace = GrDrawState::kInvalid_DrawFace;
        fSrcBlend = kZero_GrBlendCoeff;
        fDstBlend = kZero_GrBlendCoeff;
        fBlendConstant = 0x0;
        fViewMatrix.reset();
        return;
    }

    fRenderTarget.reset(drawState.fRenderTarget.get());
    SkASSERT(fRenderTarget);
    fScissorState = scissorState;
    fViewMatrix = drawState.getViewMatrix();
    fBlendConstant = drawState.getBlendConstant();
    fStencilSettings = drawState.getStencil();
    fDrawFace = drawState.getDrawFace();
    fSrcBlend = optSrcCoeff;
    fDstBlend = optDstCoeff;

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

    fDescInfo.fHasVertexColor = drawState.hasGeometryProcessor() &&
                                drawState.getGeometryProcessor()->hasVertexColor();

    fDescInfo.fHasVertexCoverage = drawState.hasGeometryProcessor() &&
                                   drawState.getGeometryProcessor()->hasVertexCoverage();

    bool hasLocalCoords = drawState.hasGeometryProcessor() &&
                          drawState.getGeometryProcessor()->hasLocalCoords();

    const GrProcOptInfo& colorPOI = drawState.colorProcInfo();
    int firstColorStageIdx = colorPOI.firstEffectiveStageIndex();
    fDescInfo.fInputColorIsUsed = colorPOI.inputColorIsUsed();
    fColor = colorPOI.inputColorToEffectiveStage();
    if (colorPOI.removeVertexAttrib()) {
        fDescInfo.fHasVertexColor = false;
    }

    // TODO: Once we can handle single or four channel input into coverage stages then we can use
    // drawState's coverageProcInfo (like color above) to set this initial information.
    int firstCoverageStageIdx = 0;
    fDescInfo.fInputCoverageIsUsed = true;
    fCoverage = drawState.getCoverage();

    this->adjustProgramForBlendOpt(drawState, blendOpt, &firstColorStageIdx,
                                   &firstCoverageStageIdx);

    this->getStageStats(drawState, firstColorStageIdx, firstCoverageStageIdx, hasLocalCoords);

    // Copy GeometryProcesssor from DS or ODS
    SkASSERT(GrGpu::IsPathRenderingDrawType(drawType) ||
             GrGpu::kStencilPath_DrawType ||
             drawState.hasGeometryProcessor());
    fGeometryProcessor.reset(drawState.getGeometryProcessor());

    // Create XferProcessor from DS's XPFactory
    const GrXferProcessor* xpProcessor = drawState.getXPFactory()->createXferProcessor();
    fXferProcessor.reset(xpProcessor);
    xpProcessor->unref();

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

    this->setOutputStateInfo(drawState, blendOpt, caps);

    // let the GP init the batch tracker
    if (drawState.hasGeometryProcessor()) {
        GrGeometryProcessor::InitBT init;
        init.fOutputColor = fDescInfo.fInputColorIsUsed;
        init.fOutputCoverage = fDescInfo.fInputCoverageIsUsed;
        init.fColor = this->getColor();
        init.fCoverage = this->getCoverage();
        fGeometryProcessor->initBatchTracker(&fBatchTracker, init);
    }
}

void GrOptDrawState::setOutputStateInfo(const GrDrawState& ds,
                                        GrDrawState::BlendOpt blendOpt,
                                        const GrDrawTargetCaps& caps) {
    // Set this default and then possibly change our mind if there is coverage.
    fDescInfo.fPrimaryOutputType = GrProgramDesc::kModulate_PrimaryOutputType;
    fDescInfo.fSecondaryOutputType = GrProgramDesc::kNone_SecondaryOutputType;

    // Determine whether we should use dual source blending or shader code to keep coverage
    // separate from color.
    bool keepCoverageSeparate = !(GrDrawState::kCoverageAsAlpha_BlendOpt == blendOpt ||
                                  GrDrawState::kEmitCoverage_BlendOpt == blendOpt);
    if (keepCoverageSeparate && !ds.hasSolidCoverage()) {
        if (caps.dualSourceBlendingSupport()) {
            if (kZero_GrBlendCoeff == fDstBlend) {
                // write the coverage value to second color
                fDescInfo.fSecondaryOutputType = GrProgramDesc::kCoverage_SecondaryOutputType;
                fDstBlend = (GrBlendCoeff)GrGpu::kIS2C_GrBlendCoeff;
            } else if (kSA_GrBlendCoeff == fDstBlend) {
                // SA dst coeff becomes 1-(1-SA)*coverage when dst is partially covered.
                fDescInfo.fSecondaryOutputType = GrProgramDesc::kCoverageISA_SecondaryOutputType;
                fDstBlend = (GrBlendCoeff)GrGpu::kIS2C_GrBlendCoeff;
            } else if (kSC_GrBlendCoeff == fDstBlend) {
                // SA dst coeff becomes 1-(1-SA)*coverage when dst is partially covered.
                fDescInfo.fSecondaryOutputType = GrProgramDesc::kCoverageISC_SecondaryOutputType;
                fDstBlend = (GrBlendCoeff)GrGpu::kIS2C_GrBlendCoeff;
            }
        } else if (fDescInfo.fReadsDst &&
                   kOne_GrBlendCoeff == fSrcBlend &&
                   kZero_GrBlendCoeff == fDstBlend) {
            fDescInfo.fPrimaryOutputType = GrProgramDesc::kCombineWithDst_PrimaryOutputType;
        }
    }
}

void GrOptDrawState::adjustProgramForBlendOpt(const GrDrawState& ds,
                                              GrDrawState::BlendOpt blendOpt,
                                              int* firstColorStageIdx,
                                              int* firstCoverageStageIdx) {
    switch (blendOpt) {
        case GrDrawState::kNone_BlendOpt:
        case GrDrawState::kSkipDraw_BlendOpt:
        case GrDrawState::kCoverageAsAlpha_BlendOpt:
            break;
        case GrDrawState::kEmitCoverage_BlendOpt:
            fColor = 0xffffffff;
            fDescInfo.fInputColorIsUsed = true;
            *firstColorStageIdx = ds.numColorStages();
            fDescInfo.fHasVertexColor = false;
            break;
        case GrDrawState::kEmitTransBlack_BlendOpt:
            fColor = 0;
            fCoverage = 0xff;
            fDescInfo.fInputColorIsUsed = true;
            fDescInfo.fInputCoverageIsUsed = true;
            *firstColorStageIdx = ds.numColorStages();
            *firstCoverageStageIdx = ds.numCoverageStages();
            fDescInfo.fHasVertexColor = false;
            fDescInfo.fHasVertexCoverage = false;
            break;
    }
}

static void get_stage_stats(const GrFragmentStage& stage, bool* readsDst, bool* readsFragPosition) {
    if (stage.getProcessor()->willReadDstColor()) {
        *readsDst = true;
    }
    if (stage.getProcessor()->willReadFragmentPosition()) {
        *readsFragPosition = true;
    }
}

void GrOptDrawState::getStageStats(const GrDrawState& ds, int firstColorStageIdx,
                                   int firstCoverageStageIdx, bool hasLocalCoords) {
    // We will need a local coord attrib if there is one currently set on the optState and we are
    // actually generating some effect code
    fDescInfo.fRequiresLocalCoordAttrib = hasLocalCoords &&
        ds.numTotalStages() - firstColorStageIdx - firstCoverageStageIdx > 0;

    fDescInfo.fReadsDst = false;
    fDescInfo.fReadsFragPosition = false;

    for (int s = firstColorStageIdx; s < ds.numColorStages(); ++s) {
        const GrFragmentStage& stage = ds.getColorStage(s);
        get_stage_stats(stage, &fDescInfo.fReadsDst, &fDescInfo.fReadsFragPosition);
    }
    for (int s = firstCoverageStageIdx; s < ds.numCoverageStages(); ++s) {
        const GrFragmentStage& stage = ds.getCoverageStage(s);
        get_stage_stats(stage, &fDescInfo.fReadsDst, &fDescInfo.fReadsFragPosition);
    }
    if (ds.hasGeometryProcessor()) {
        const GrGeometryProcessor& gp = *ds.getGeometryProcessor();
        fDescInfo.fReadsFragPosition = fDescInfo.fReadsFragPosition || gp.willReadFragmentPosition();
    }
}

void GrOptDrawState::finalize(GrGpu* gpu) {
    gpu->buildProgramDesc(*this, fDescInfo, fDrawType, &fDesc);
    fFinalized = true;
}

////////////////////////////////////////////////////////////////////////////////

bool GrOptDrawState::operator== (const GrOptDrawState& that) const {
    if (!fDescInfo.fHasVertexColor && this->fColor != that.fColor) {
        return false;
    }

    if (this->getRenderTarget() != that.getRenderTarget() ||
        this->fFragmentStages.count() != that.fFragmentStages.count() ||
        this->fNumColorStages != that.fNumColorStages ||
        this->fScissorState != that.fScissorState ||
        !this->fViewMatrix.cheapEqualTo(that.fViewMatrix) ||
        this->fSrcBlend != that.fSrcBlend ||
        this->fDstBlend != that.fDstBlend ||
        this->fDrawType != that.fDrawType ||
        fDescInfo.fInputColorIsUsed != that.fDescInfo.fInputColorIsUsed ||
        fDescInfo.fInputCoverageIsUsed != that.fDescInfo.fInputCoverageIsUsed ||
        fDescInfo.fReadsDst != that.fDescInfo.fReadsDst ||
        fDescInfo.fReadsFragPosition != that.fDescInfo.fReadsFragPosition ||
        fDescInfo.fRequiresLocalCoordAttrib != that.fDescInfo.fRequiresLocalCoordAttrib ||
        fDescInfo.fPrimaryOutputType != that.fDescInfo.fPrimaryOutputType ||
        fDescInfo.fSecondaryOutputType != that.fDescInfo.fSecondaryOutputType ||
        this->fScissorState != that.fScissorState ||
        this->fBlendConstant != that.fBlendConstant ||
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

    // The program desc comparison should have already assured that the stage counts match.
    SkASSERT(this->numFragmentStages() == that.numFragmentStages());
    for (int i = 0; i < this->numFragmentStages(); i++) {

        if (this->getFragmentStage(i) != that.getFragmentStage(i)) {
            return false;
        }
    }
    return true;
}

