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
                               GrColor color,
                               uint8_t coverage,
                               const GrDrawTargetCaps& caps,
                               const ScissorState& scissorState,
                               const GrDeviceCoordTexture* dstCopy,
                               GrGpu::DrawType drawType)
    : fFinalized(false) {
    GrColor coverageColor = GrColorPackRGBA(coverage, coverage, coverage, coverage);
    fDrawType = drawType;

    const GrProcOptInfo& colorPOI = drawState.colorProcInfo(color);
    const GrProcOptInfo& coveragePOI = drawState.coverageProcInfo(coverageColor);
    
    fColor = colorPOI.inputColorToEffectiveStage();
    fCoverage = coverage;

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
                                                   &fCoverage);
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

    fDescInfo.fHasVertexColor = drawState.hasGeometryProcessor() &&
                                drawState.getGeometryProcessor()->hasVertexColor();

    fDescInfo.fHasVertexCoverage = drawState.hasGeometryProcessor() &&
                                   drawState.getGeometryProcessor()->hasVertexCoverage();

    bool hasLocalCoords = drawState.hasGeometryProcessor() &&
                          drawState.getGeometryProcessor()->hasLocalCoords();

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
    fSrcBlend = blendInfo.fSrcBlend;
    fDstBlend = blendInfo.fDstBlend;
    fBlendConstant = blendInfo.fBlendConstant;

    this->adjustProgramFromOptimizations(drawState, optFlags, colorPOI, coveragePOI,
                                         &firstColorStageIdx, &firstCoverageStageIdx);

    fDescInfo.fRequiresLocalCoordAttrib = hasLocalCoords;

    // Copy GeometryProcesssor from DS or ODS
    SkASSERT(GrGpu::IsPathRenderingDrawType(drawType) ||
             GrGpu::kStencilPath_DrawType ||
             drawState.hasGeometryProcessor());
    fGeometryProcessor.reset(drawState.getGeometryProcessor());

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
    if (drawState.hasGeometryProcessor()) {
        GrGeometryProcessor::InitBT init;
        init.fOutputColor = fDescInfo.fInputColorIsUsed;
        init.fOutputCoverage = fDescInfo.fInputCoverageIsUsed;
        init.fColor = this->getColor();
        init.fCoverage = this->getCoverage();
        fGeometryProcessor->initBatchTracker(&fBatchTracker, init);
    }

    this->setOutputStateInfo(drawState, coverageColor, optFlags, caps);
}

void GrOptDrawState::setOutputStateInfo(const GrDrawState& ds,
                                        GrColor coverage,
                                        GrXferProcessor::OptFlags optFlags,
                                        const GrDrawTargetCaps& caps) {
    // Set this default and then possibly change our mind if there is coverage.
    fDescInfo.fPrimaryOutputType = GrProgramDesc::kModulate_PrimaryOutputType;
    fDescInfo.fSecondaryOutputType = GrProgramDesc::kNone_SecondaryOutputType;

    // Determine whether we should use dual source blending or shader code to keep coverage
    // separate from color.
    bool keepCoverageSeparate = !(optFlags & GrXferProcessor::kSetCoverageDrawing_OptFlag);
    if (keepCoverageSeparate && !ds.hasSolidCoverage(coverage)) {
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

void GrOptDrawState::adjustProgramFromOptimizations(const GrDrawState& ds,
                                                    GrXferProcessor::OptFlags flags,
                                                    const GrProcOptInfo& colorPOI,
                                                    const GrProcOptInfo& coveragePOI,
                                                    int* firstColorStageIdx,
                                                    int* firstCoverageStageIdx) {
    fDescInfo.fReadsDst = false;
    fDescInfo.fReadsFragPosition = false;

    if (flags & GrXferProcessor::kClearColorStages_OptFlag) {
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
        this->fSrcBlend != that.fSrcBlend ||
        this->fDstBlend != that.fDstBlend ||
        this->fDrawType != that.fDrawType ||
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

