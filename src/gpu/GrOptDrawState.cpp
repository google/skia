/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrOptDrawState.h"

#include "GrDrawState.h"
#include "GrDrawTargetCaps.h"

GrOptDrawState::GrOptDrawState(const GrDrawState& drawState,
                               BlendOptFlags blendOptFlags,
                               GrBlendCoeff optSrcCoeff,
                               GrBlendCoeff optDstCoeff,
                               const GrDrawTargetCaps& caps) {
    fRenderTarget.set(SkSafeRef(drawState.getRenderTarget()), kWrite_GrIOType);
    fColor = drawState.getColor();
    fCoverage = drawState.getCoverage();
    fViewMatrix = drawState.getViewMatrix();
    fBlendConstant = drawState.getBlendConstant();
    fFlagBits = drawState.getFlagBits();
    fVAPtr = drawState.getVertexAttribs();
    fVACount = drawState.getVertexAttribCount();
    fVAStride = drawState.getVertexStride();
    fStencilSettings = drawState.getStencil();
    fDrawFace = (DrawFace)drawState.getDrawFace();
    fBlendOptFlags = blendOptFlags;
    fSrcBlend = optSrcCoeff;
    fDstBlend = optDstCoeff;

    memcpy(fFixedFunctionVertexAttribIndices,
           drawState.getFixedFunctionVertexAttribIndices(),
           sizeof(fFixedFunctionVertexAttribIndices));

    fInputColorIsUsed = true;
    fInputCoverageIsUsed = true;

    int firstColorStageIdx = 0;
    int firstCoverageStageIdx = 0;
    bool separateCoverageFromColor;

    uint8_t fixedFunctionVAToRemove = 0;

    this->computeEffectiveColorStages(drawState, &firstColorStageIdx, &fixedFunctionVAToRemove);
    this->computeEffectiveCoverageStages(drawState, &firstCoverageStageIdx);
    this->adjustFromBlendOpts(drawState, &firstColorStageIdx, &firstCoverageStageIdx,
                              &fixedFunctionVAToRemove);
    // Should not be setting any more FFVA to be removed at this point
    this->removeFixedFunctionVertexAttribs(fixedFunctionVAToRemove);
    this->getStageStats(drawState, firstColorStageIdx, firstCoverageStageIdx);
    this->setOutputStateInfo(drawState, caps, firstCoverageStageIdx, &separateCoverageFromColor);

    // Copy GeometryProcesssor from DS or ODS
    if (drawState.hasGeometryProcessor()) {
        fGeometryProcessor.reset(SkNEW_ARGS(GrGeometryStage, (*drawState.getGeometryProcessor())));
    } else {
        fGeometryProcessor.reset(NULL);
    }

    // Copy Color Stages from DS to ODS
    if (firstColorStageIdx < drawState.numColorStages()) {
        fColorStages.reset(&drawState.getColorStage(firstColorStageIdx),
                           drawState.numColorStages() - firstColorStageIdx);
    } else {
        fColorStages.reset();
    }

    // Copy Coverage Stages from DS to ODS
    if (firstCoverageStageIdx < drawState.numCoverageStages() && separateCoverageFromColor) {
        fCoverageStages.reset(&drawState.getCoverageStage(firstCoverageStageIdx),
                              drawState.numCoverageStages() - firstCoverageStageIdx);
    } else {
        fCoverageStages.reset();
        if (firstCoverageStageIdx < drawState.numCoverageStages()) {
            // TODO: Once we have flag to know if we only multiply on stages, only push coverage
            // into color stages if everything is multiply
            fColorStages.push_back_n(drawState.numCoverageStages() - firstCoverageStageIdx,
                                     &drawState.getCoverageStage(firstCoverageStageIdx));
        }
    }
};

GrOptDrawState* GrOptDrawState::Create(const GrDrawState& drawState, const GrDrawTargetCaps& caps,
                                       GrGpu::DrawType drawType) {
    if (NULL == drawState.fCachedOptState || caps.getUniqueID() != drawState.fCachedCapsID) {
        GrBlendCoeff srcCoeff;
        GrBlendCoeff dstCoeff;
        BlendOptFlags blendFlags = (BlendOptFlags) drawState.getBlendOpts(false,
                                                                          &srcCoeff,
                                                                          &dstCoeff);

        // If our blend coeffs are set to 0,1 we know we will not end up drawing unless we are
        // stenciling. When path rendering the stencil settings are not always set on the draw state
        // so we must check the draw type. In cases where we will skip drawing we simply return a
        // null GrOptDrawState.
        if (kZero_GrBlendCoeff == srcCoeff && kOne_GrBlendCoeff == dstCoeff &&
            !drawState.getStencil().doesWrite() && GrGpu::kStencilPath_DrawType != drawType) {
            return NULL;
        }

        drawState.fCachedOptState = SkNEW_ARGS(GrOptDrawState, (drawState, blendFlags, srcCoeff,
                                                                dstCoeff, caps));
        drawState.fCachedCapsID = caps.getUniqueID();
    } else {
#ifdef SK_DEBUG
        GrBlendCoeff srcCoeff;
        GrBlendCoeff dstCoeff;
        BlendOptFlags blendFlags = (BlendOptFlags) drawState.getBlendOpts(false,
                                                                          &srcCoeff,
                                                                          &dstCoeff);
        SkASSERT(GrOptDrawState(drawState, blendFlags, srcCoeff, dstCoeff, caps) ==
                 *drawState.fCachedOptState);
#endif
    }
    drawState.fCachedOptState->ref();
    return drawState.fCachedOptState;
}

void GrOptDrawState::setOutputStateInfo(const GrDrawState& ds,
                                        const GrDrawTargetCaps& caps,
                                        int firstCoverageStageIdx,
                                        bool* separateCoverageFromColor) {
    // Set this default and then possibly change our mind if there is coverage.
    fPrimaryOutputType = kModulate_PrimaryOutputType;
    fSecondaryOutputType = kNone_SecondaryOutputType;

    // If we do have coverage determine whether it matters.
    *separateCoverageFromColor = this->hasGeometryProcessor();
    if (!this->isCoverageDrawing() &&
        (ds.numCoverageStages() - firstCoverageStageIdx > 0 ||
         ds.hasGeometryProcessor() ||
         this->hasCoverageVertexAttribute())) {

        if (caps.dualSourceBlendingSupport()) {
            if (kZero_GrBlendCoeff == fDstBlend) {
                // write the coverage value to second color
                fSecondaryOutputType =  kCoverage_SecondaryOutputType;
                *separateCoverageFromColor = true;
                fDstBlend = (GrBlendCoeff)GrGpu::kIS2C_GrBlendCoeff;
            } else if (kSA_GrBlendCoeff == fDstBlend) {
                // SA dst coeff becomes 1-(1-SA)*coverage when dst is partially covered.
                fSecondaryOutputType = kCoverageISA_SecondaryOutputType;
                *separateCoverageFromColor = true;
                fDstBlend = (GrBlendCoeff)GrGpu::kIS2C_GrBlendCoeff;
            } else if (kSC_GrBlendCoeff == fDstBlend) {
                // SA dst coeff becomes 1-(1-SA)*coverage when dst is partially covered.
                fSecondaryOutputType = kCoverageISC_SecondaryOutputType;
                *separateCoverageFromColor = true;
                fDstBlend = (GrBlendCoeff)GrGpu::kIS2C_GrBlendCoeff;
            }
        } else if (fReadsDst &&
                   kOne_GrBlendCoeff == fSrcBlend &&
                   kZero_GrBlendCoeff == fDstBlend) {
            fPrimaryOutputType = kCombineWithDst_PrimaryOutputType;
            *separateCoverageFromColor = true;
        }
    }
}

void GrOptDrawState::adjustFromBlendOpts(const GrDrawState& ds,
                                         int* firstColorStageIdx,
                                         int* firstCoverageStageIdx,
                                         uint8_t* fixedFunctionVAToRemove) {
    switch (fBlendOptFlags) {
        case kNone_BlendOpt:
        case kSkipDraw_BlendOptFlag:
            break;
        case kCoverageAsAlpha_BlendOptFlag:
            fFlagBits |= kCoverageDrawing_StateBit;
            break;
        case kEmitCoverage_BlendOptFlag:
            fColor = 0xffffffff;
            fInputColorIsUsed = true;
            *firstColorStageIdx = ds.numColorStages();
            *fixedFunctionVAToRemove |= 0x1 << kColor_GrVertexAttribBinding;
            break;
        case kEmitTransBlack_BlendOptFlag:
            fColor = 0;
            fCoverage = 0xff;
            fInputColorIsUsed = true;
            fInputCoverageIsUsed = true;
            *firstColorStageIdx = ds.numColorStages();
            *firstCoverageStageIdx = ds.numCoverageStages();
            *fixedFunctionVAToRemove |= (0x1 << kColor_GrVertexAttribBinding |
                                         0x1 << kCoverage_GrVertexAttribBinding);
            break;
        default:
            SkFAIL("Unknown BlendOptFlag");
    }
}

void GrOptDrawState::removeFixedFunctionVertexAttribs(uint8_t removeVAFlag) {
    int numToRemove = 0;
    uint8_t maskCheck = 0x1;
    // Count the number of vertex attributes that we will actually remove
    for (int i = 0; i < kGrFixedFunctionVertexAttribBindingCnt; ++i) {
        if ((maskCheck & removeVAFlag) && -1 != fFixedFunctionVertexAttribIndices[i]) {
            ++numToRemove;
        }
        maskCheck <<= 1;
    }

    fOptVA.reset(fVACount - numToRemove);

    GrVertexAttrib* dst = fOptVA.get();
    const GrVertexAttrib* src = fVAPtr;

    for (int i = 0, newIdx = 0; i < fVACount; ++i, ++src) {
        const GrVertexAttrib& currAttrib = *src;
        if (currAttrib.fBinding < kGrFixedFunctionVertexAttribBindingCnt) {
            uint8_t maskCheck = 0x1 << currAttrib.fBinding;
            if (maskCheck & removeVAFlag) {
                SkASSERT(-1 != fFixedFunctionVertexAttribIndices[currAttrib.fBinding]);
                fFixedFunctionVertexAttribIndices[currAttrib.fBinding] = -1;
                continue;
            }
            fFixedFunctionVertexAttribIndices[currAttrib.fBinding] = newIdx;
        }
        memcpy(dst, src, sizeof(GrVertexAttrib));
        ++newIdx;
        ++dst;
    }
    fVACount -= numToRemove;
    fVAPtr = fOptVA.get();
}

void GrOptDrawState::computeEffectiveColorStages(const GrDrawState& ds, int* firstColorStageIdx,
                                                 uint8_t* fixedFunctionVAToRemove) {
    // Set up color and flags for ConstantColorComponent checks
    GrProcessor::InvariantOutput inout;
    inout.fIsSingleComponent = false;
    if (!this->hasColorVertexAttribute()) {
        inout.fColor = ds.getColor();
        inout.fValidFlags = kRGBA_GrColorComponentFlags;
    } else {
        if (ds.vertexColorsAreOpaque()) {
            inout.fColor = 0xFF << GrColor_SHIFT_A;
            inout.fValidFlags = kA_GrColorComponentFlag;
        } else {
            inout.fValidFlags = 0;
            // not strictly necessary but we get false alarms from tools about uninit.
            inout.fColor = 0;
        }
    }

    for (int i = 0; i < ds.numColorStages(); ++i) {
        const GrFragmentProcessor* fp = ds.getColorStage(i).getProcessor();
        if (!fp->willUseInputColor()) {
            *firstColorStageIdx = i;
            fInputColorIsUsed = false;
        }
        fp->computeInvariantOutput(&inout);
        if (kRGBA_GrColorComponentFlags == inout.fValidFlags) {
            *firstColorStageIdx = i + 1;
            fColor = inout.fColor;
            fInputColorIsUsed = true;
            *fixedFunctionVAToRemove |= 0x1 << kColor_GrVertexAttribBinding;
        }
    }
}

void GrOptDrawState::computeEffectiveCoverageStages(const GrDrawState& ds,
                                                    int* firstCoverageStageIdx) {
    // We do not try to optimize out constantColor coverage effects here. It is extremely rare
    // to have a coverage effect that returns a constant value for all four channels. Thus we
    // save having to make extra virtual calls by not checking for it.

    // Don't do any optimizations on coverage stages. It should not be the case where we do not use
    // input coverage in an effect
#ifdef OptCoverageStages
    for (int i = 0; i < ds.numCoverageStages(); ++i) {
        const GrProcessor* processor = ds.getCoverageStage(i).getProcessor();
        if (!processor->willUseInputColor()) {
            *firstCoverageStageIdx = i;
            fInputCoverageIsUsed = false;
        }
    }
#endif
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
                                   int firstCoverageStageIdx) {
    // We will need a local coord attrib if there is one currently set on the optState and we are
    // actually generating some effect code
    fRequiresLocalCoordAttrib = this->hasLocalCoordAttribute() &&
        ds.numTotalStages() - firstColorStageIdx - firstCoverageStageIdx > 0;

    fReadsDst = false;
    fReadsFragPosition = false;

    for (int s = firstColorStageIdx; s < ds.numColorStages(); ++s) {
        const GrFragmentStage& stage = ds.getColorStage(s);
        get_stage_stats(stage, &fReadsDst, &fReadsFragPosition);
    }
    for (int s = firstCoverageStageIdx; s < ds.numCoverageStages(); ++s) {
        const GrFragmentStage& stage = ds.getCoverageStage(s);
        get_stage_stats(stage, &fReadsDst, &fReadsFragPosition);
    }
    if (ds.hasGeometryProcessor()) {
        const GrGeometryStage& stage = *ds.getGeometryProcessor();
        fReadsFragPosition = fReadsFragPosition || stage.getProcessor()->willReadFragmentPosition();
    }
}

////////////////////////////////////////////////////////////////////////////////

bool GrOptDrawState::operator== (const GrOptDrawState& that) const {
    return this->isEqual(that);
}

bool GrOptDrawState::isEqual(const GrOptDrawState& that) const {
    bool usingVertexColors = this->hasColorVertexAttribute();
    if (!usingVertexColors && this->fColor != that.fColor) {
        return false;
    }

    if (this->getRenderTarget() != that.getRenderTarget() ||
        this->fColorStages.count() != that.fColorStages.count() ||
        this->fCoverageStages.count() != that.fCoverageStages.count() ||
        !this->fViewMatrix.cheapEqualTo(that.fViewMatrix) ||
        this->fSrcBlend != that.fSrcBlend ||
        this->fDstBlend != that.fDstBlend ||
        this->fBlendConstant != that.fBlendConstant ||
        this->fFlagBits != that.fFlagBits ||
        this->fVACount != that.fVACount ||
        this->fVAStride != that.fVAStride ||
        memcmp(this->fVAPtr, that.fVAPtr, this->fVACount * sizeof(GrVertexAttrib)) ||
        this->fStencilSettings != that.fStencilSettings ||
        this->fDrawFace != that.fDrawFace ||
        this->fInputColorIsUsed != that.fInputColorIsUsed ||
        this->fInputCoverageIsUsed != that.fInputCoverageIsUsed ||
        this->fReadsDst != that.fReadsDst ||
        this->fReadsFragPosition != that.fReadsFragPosition ||
        this->fRequiresLocalCoordAttrib != that.fRequiresLocalCoordAttrib ||
        this->fPrimaryOutputType != that.fPrimaryOutputType ||
        this->fSecondaryOutputType != that.fSecondaryOutputType) {
        return false;
    }

    bool usingVertexCoverage = this->hasCoverageVertexAttribute();
    if (!usingVertexCoverage && this->fCoverage != that.fCoverage) {
        return false;
    }

    bool explicitLocalCoords = this->hasLocalCoordAttribute();
    if (this->hasGeometryProcessor()) {
        if (!that.hasGeometryProcessor()) {
            return false;
        } else if (!GrProcessorStage::AreCompatible(*this->getGeometryProcessor(),
                                                    *that.getGeometryProcessor(),
                                                    explicitLocalCoords)) {
            return false;
        }
    } else if (that.hasGeometryProcessor()) {
        return false;
    }

    for (int i = 0; i < this->numColorStages(); i++) {
        if (!GrProcessorStage::AreCompatible(this->getColorStage(i), that.getColorStage(i),
                                             explicitLocalCoords)) {
            return false;
        }
    }
    for (int i = 0; i < this->numCoverageStages(); i++) {
        if (!GrProcessorStage::AreCompatible(this->getCoverageStage(i), that.getCoverageStage(i),
                                             explicitLocalCoords)) {
            return false;
        }
    }

    SkASSERT(0 == memcmp(this->fFixedFunctionVertexAttribIndices,
                         that.fFixedFunctionVertexAttribIndices,
                         sizeof(this->fFixedFunctionVertexAttribIndices)));

    return true;
}

