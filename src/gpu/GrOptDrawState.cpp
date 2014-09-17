/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrOptDrawState.h"

#include "GrDrawState.h"

GrOptDrawState::GrOptDrawState(const GrDrawState& drawState,
                               BlendOptFlags blendOptFlags,
                               GrBlendCoeff optSrcCoeff,
                               GrBlendCoeff optDstCoeff) : INHERITED(drawState) {
    fColor = drawState.getColor();
    fCoverage = drawState.getCoverage();
    fViewMatrix = drawState.getViewMatrix();
    fBlendConstant = drawState.getBlendConstant();
    fFlagBits = drawState.getFlagBits();
    fVAPtr = drawState.getVertexAttribs();
    fVACount = drawState.getVertexAttribCount();
    fVAStride = drawState.getVertexStride();
    fStencilSettings = drawState.getStencil();
    fDrawFace = drawState.getDrawFace();
    fBlendOptFlags = blendOptFlags;
    fSrcBlend = optSrcCoeff;
    fDstBlend = optDstCoeff;

    memcpy(fFixedFunctionVertexAttribIndices,
            drawState.getFixedFunctionVertexAttribIndices(),
            sizeof(fFixedFunctionVertexAttribIndices));


    fInputColorIsUsed = true;
    fInputCoverageIsUsed = true;

    if (drawState.hasGeometryProcessor()) {
        fGeometryProcessor.reset(SkNEW_ARGS(GrEffectStage, (*drawState.getGeometryProcessor())));
    } else {
        fGeometryProcessor.reset(NULL);
    }

    this->copyEffectiveColorStages(drawState);
    this->copyEffectiveCoverageStages(drawState);
    this->adjustFromBlendOpts();
    this->getStageStats();
};

void GrOptDrawState::adjustFromBlendOpts() {

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
            fColorStages.reset();
            this->removeFixedFunctionVertexAttribs(0x1 << kColor_GrVertexAttribBinding);
            break;
        case kEmitTransBlack_BlendOptFlag:
            fColor = 0;
            fCoverage = 0xff;
            fInputColorIsUsed = true;
            fInputCoverageIsUsed = true;
            fColorStages.reset();
            fCoverageStages.reset();
            this->removeFixedFunctionVertexAttribs(0x1 << kColor_GrVertexAttribBinding |
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

void GrOptDrawState::copyEffectiveColorStages(const GrDrawState& ds) {
    int firstColorStage = 0;

    // Set up color and flags for ConstantColorComponent checks
    GrColor color;
    uint32_t validComponentFlags;
    if (!this->hasColorVertexAttribute()) {
        color = ds.getColor();
        validComponentFlags = kRGBA_GrColorComponentFlags;
    } else {
        if (ds.vertexColorsAreOpaque()) {
            color = 0xFF << GrColor_SHIFT_A;
            validComponentFlags = kA_GrColorComponentFlag;
        } else {
            validComponentFlags = 0;
            color = 0; // not strictly necessary but we get false alarms from tools about uninit.
        }
    }

    for (int i = 0; i < ds.numColorStages(); ++i) {
        const GrEffect* effect = ds.getColorStage(i).getEffect();
        if (!effect->willUseInputColor()) {
            firstColorStage = i;
            fInputColorIsUsed = false;
        }
        effect->getConstantColorComponents(&color, &validComponentFlags);
        if (kRGBA_GrColorComponentFlags == validComponentFlags) {
            firstColorStage = i + 1;
            fColor = color;
            fInputColorIsUsed = true;
            this->removeFixedFunctionVertexAttribs(0x1 << kColor_GrVertexAttribBinding);
        }
    }
    if (firstColorStage < ds.numColorStages()) {
        fColorStages.reset(&ds.getColorStage(firstColorStage),
                           ds.numColorStages() - firstColorStage);
    } else {
        fColorStages.reset();
    }
}

void GrOptDrawState::copyEffectiveCoverageStages(const GrDrawState& ds) {
    int firstCoverageStage = 0;

    // We do not try to optimize out constantColor coverage effects here. It is extremely rare
    // to have a coverage effect that returns a constant value for all four channels. Thus we
    // save having to make extra virtual calls by not checking for it.

    // Don't do any optimizations on coverage stages. It should not be the case where we do not use
    // input coverage in an effect
#ifdef OptCoverageStages
    for (int i = 0; i < ds.numCoverageStages(); ++i) {
        const GrEffect* effect = ds.getCoverageStage(i).getEffect();
        if (!effect->willUseInputColor()) {
            firstCoverageStage = i;
            fInputCoverageIsUsed = false;
        }
    }
#endif
    if (ds.numCoverageStages() > 0) {
        fCoverageStages.reset(&ds.getCoverageStage(firstCoverageStage),
                              ds.numCoverageStages() - firstCoverageStage);
    } else {
        fCoverageStages.reset();
    }
}

static void get_stage_stats(const GrEffectStage& stage, bool* readsDst, bool* readsFragPosition) {
    if (stage.getEffect()->willReadDstColor()) {
        *readsDst = true;
    }
    if (stage.getEffect()->willReadFragmentPosition()) {
        *readsFragPosition = true;
    }
}
void GrOptDrawState::getStageStats() {
    // We will need a local coord attrib if there is one currently set on the optState and we are
    // actually generating some effect code
    fRequiresLocalCoordAttrib = this->hasLocalCoordAttribute() && this->numTotalStages() > 0;

    // if 1 == fVACount then that VA must be position, otherwise it contains some attribute which
    // will require a vertexShader
    fRequiresVertexShader = fVACount > 1;

    fReadsDst = false;
    fReadsFragPosition = false;

    for (int s = 0; s < this->numColorStages(); ++s) {
        const GrEffectStage& stage = this->getColorStage(s);
        get_stage_stats(stage, &fReadsDst, &fReadsFragPosition);
    }
    for (int s = 0; s < this->numCoverageStages(); ++s) {
        const GrEffectStage& stage = this->getCoverageStage(s);
        get_stage_stats(stage, &fReadsDst, &fReadsFragPosition);
    }
    if (this->hasGeometryProcessor()) {
        const GrEffectStage& stage = *this->getGeometryProcessor();
        get_stage_stats(stage, &fReadsDst, &fReadsFragPosition);
        SkASSERT(fRequiresVertexShader);
    }
}

bool GrOptDrawState::operator== (const GrOptDrawState& that) const {
    return this->isEqual(that);
}

