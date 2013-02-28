/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDrawState.h"
#include "GrPaint.h"

void GrDrawState::setFromPaint(const GrPaint& paint) {
    for (int i = 0; i < GrPaint::kMaxColorStages; ++i) {
        int s = i + GrPaint::kFirstColorStage;
        if (paint.isColorStageEnabled(i)) {
            fStages[s] = paint.getColorStage(i);
        } else {
            fStages[s].setEffect(NULL);
        }
    }

    this->setFirstCoverageStage(GrPaint::kFirstCoverageStage);

    for (int i = 0; i < GrPaint::kMaxCoverageStages; ++i) {
        int s = i + GrPaint::kFirstCoverageStage;
        if (paint.isCoverageStageEnabled(i)) {
            fStages[s] = paint.getCoverageStage(i);
        } else {
            fStages[s].setEffect(NULL);
        }
    }

    // disable all stages not accessible via the paint
    for (int s = GrPaint::kTotalStages; s < GrDrawState::kNumStages; ++s) {
        this->disableStage(s);
    }

    this->setColor(paint.getColor());

    this->setState(GrDrawState::kDither_StateBit, paint.isDither());
    this->setState(GrDrawState::kHWAntialias_StateBit, paint.isAntiAlias());

    this->setBlendFunc(paint.getSrcBlendCoeff(), paint.getDstBlendCoeff());
    this->setColorFilter(paint.getColorFilterColor(), paint.getColorFilterMode());
    this->setCoverage(paint.getCoverage());
}

////////////////////////////////////////////////////////////////////////////////

namespace {

/**
 * This function generates a mask that we like to have known at compile
 * time. When the number of stages is bumped or the way bits are defined in
 * GrDrawState.h changes this function should be rerun to generate the new mask.
 * (We attempted to force the compiler to generate the mask using recursive
 * templates but always wound up with static initializers under gcc, even if
 * they were just a series of immediate->memory moves.)
 *
 */
void gen_tex_coord_mask(GrVertexLayout* texCoordMask) {
    *texCoordMask = 0;
    for (int s = 0; s < GrDrawState::kNumStages; ++s) {
        *texCoordMask |= GrDrawState::StageTexCoordVertexLayoutBit(s);
    }
}

const GrVertexLayout kTexCoordMask = (1 << GrDrawState::kNumStages)-1;

inline int num_tex_coords(GrVertexLayout layout) {
    return (kTexCoordMask & layout) ? 1 : 0;
}

} //unnamed namespace

static const size_t kVec2Size = sizeof(GrPoint);

size_t GrDrawState::VertexSize(GrVertexLayout vertexLayout) {
    size_t size = kVec2Size; // position
    size += num_tex_coords(vertexLayout) * kVec2Size;
    if (vertexLayout & kColor_VertexLayoutBit) {
        size += sizeof(GrColor);
    }
    if (vertexLayout & kCoverage_VertexLayoutBit) {
        size += sizeof(GrColor);
    }
    if (vertexLayout & kEdge_VertexLayoutBit) {
        size += 4 * sizeof(SkScalar);
    }
    return size;
}

////////////////////////////////////////////////////////////////////////////////

/**
 * Functions for computing offsets of various components from the layout
 * bitfield.
 *
 * Order of vertex components:
 * Position
 * Tex Coord
 * Color
 * Coverage
 */

int GrDrawState::VertexStageCoordOffset(int stageIdx, GrVertexLayout vertexLayout) {
    if (!StageUsesTexCoords(vertexLayout, stageIdx)) {
        return 0;
    }

    return kVec2Size;
}

int GrDrawState::VertexColorOffset(GrVertexLayout vertexLayout) {
    if (vertexLayout & kColor_VertexLayoutBit) {
        return kVec2Size * (num_tex_coords(vertexLayout) + 1); //+1 for pos
    }
    return -1;
}

int GrDrawState::VertexCoverageOffset(GrVertexLayout vertexLayout) {
    if (vertexLayout & kCoverage_VertexLayoutBit) {
        int offset =  kVec2Size * (num_tex_coords(vertexLayout) + 1);
        if (vertexLayout & kColor_VertexLayoutBit) {
            offset += sizeof(GrColor);
        }
        return offset;
    }
    return -1;
}

int GrDrawState::VertexEdgeOffset(GrVertexLayout vertexLayout) {
    // edge pts are after the pos, tex coords, and color
    if (vertexLayout & kEdge_VertexLayoutBit) {
        int offset = kVec2Size * (num_tex_coords(vertexLayout) + 1); //+1 for pos
        if (vertexLayout & kColor_VertexLayoutBit) {
            offset += sizeof(GrColor);
        }
        if (vertexLayout & kCoverage_VertexLayoutBit) {
            offset += sizeof(GrColor);
        }
        return offset;
    }
    return -1;
}

int GrDrawState::VertexSizeAndOffsets(
        GrVertexLayout vertexLayout,
        int* texCoordOffset,
        int* colorOffset,
        int* coverageOffset,
        int* edgeOffset) {
    int size = kVec2Size; // position

    if (kTexCoordMask & vertexLayout) {
        if (NULL != texCoordOffset) {
            *texCoordOffset = size;
        }
        size += kVec2Size;
    } else {
        if (NULL != texCoordOffset) {
            *texCoordOffset = -1;
        }
    }
    if (kColor_VertexLayoutBit & vertexLayout) {
        if (NULL != colorOffset) {
            *colorOffset = size;
        }
        size += sizeof(GrColor);
    } else {
        if (NULL != colorOffset) {
            *colorOffset = -1;
        }
    }
    if (kCoverage_VertexLayoutBit & vertexLayout) {
        if (NULL != coverageOffset) {
            *coverageOffset = size;
        }
        size += sizeof(GrColor);
    } else {
        if (NULL != coverageOffset) {
            *coverageOffset = -1;
        }
    }
    if (kEdge_VertexLayoutBit & vertexLayout) {
        if (NULL != edgeOffset) {
            *edgeOffset = size;
        }
        size += 4 * sizeof(SkScalar);
    } else {
        if (NULL != edgeOffset) {
            *edgeOffset = -1;
        }
    }
    return size;
}

int GrDrawState::VertexSizeAndOffsetsByStage(
        GrVertexLayout vertexLayout,
        int texCoordOffsetsByStage[GrDrawState::kNumStages],
        int* colorOffset,
        int* coverageOffset,
        int* edgeOffset) {

    int texCoordOffset;
    int size = VertexSizeAndOffsets(vertexLayout,
                                    &texCoordOffset,
                                    colorOffset,
                                    coverageOffset,
                                    edgeOffset);
    if (NULL != texCoordOffsetsByStage) {
        for (int s = 0; s < GrDrawState::kNumStages; ++s) {
            texCoordOffsetsByStage[s] = StageUsesTexCoords(vertexLayout, s) ?
                                                           texCoordOffset : 0;
        }
    }
    return size;
}

////////////////////////////////////////////////////////////////////////////////

bool GrDrawState::VertexUsesTexCoords(GrVertexLayout vertexLayout) {
    return SkToBool(kTexCoordMask & vertexLayout);
}

////////////////////////////////////////////////////////////////////////////////

void GrDrawState::VertexLayoutUnitTest() {
    // Ensure that our tex coord mask is correct
    GrVertexLayout texCoordMask;
    gen_tex_coord_mask(&texCoordMask);
    GrAssert(texCoordMask == kTexCoordMask);

    // not necessarily exhaustive
    static bool run;
    if (!run) {
        run = true;
        GrVertexLayout tcMask = 0;
        GrAssert(!VertexUsesTexCoords(0));
        for (int s = 0; s < GrDrawState::kNumStages; ++s) {
            tcMask |= StageTexCoordVertexLayoutBit(s);
            GrAssert(sizeof(GrPoint) == VertexStageCoordOffset(s, tcMask));
            GrAssert(VertexUsesTexCoords(tcMask));
            GrAssert(2*sizeof(GrPoint) == VertexSize(tcMask));
            GrAssert(StageUsesTexCoords(tcMask, s));
            for (int s2 = s + 1; s2 < GrDrawState::kNumStages; ++s2) {
                GrAssert(!StageUsesTexCoords(tcMask, s2));

            #if GR_DEBUG
                GrVertexLayout posAsTex = tcMask;
            #endif
                GrAssert(0 == VertexStageCoordOffset(s2, posAsTex));
                GrAssert(2*sizeof(GrPoint) == VertexSize(posAsTex));
                GrAssert(!StageUsesTexCoords(posAsTex, s2));
                GrAssert(-1 == VertexEdgeOffset(posAsTex));
            }
            GrAssert(-1 == VertexEdgeOffset(tcMask));
            GrAssert(-1 == VertexColorOffset(tcMask));
            GrAssert(-1 == VertexCoverageOffset(tcMask));
        #if GR_DEBUG
            GrVertexLayout withColor = tcMask | kColor_VertexLayoutBit;
        #endif
            GrAssert(-1 == VertexCoverageOffset(withColor));
            GrAssert(2*sizeof(GrPoint) == VertexColorOffset(withColor));
            GrAssert(2*sizeof(GrPoint) + sizeof(GrColor) == VertexSize(withColor));
        #if GR_DEBUG
            GrVertexLayout withEdge = tcMask | kEdge_VertexLayoutBit;
        #endif
            GrAssert(-1 == VertexColorOffset(withEdge));
            GrAssert(2*sizeof(GrPoint) == VertexEdgeOffset(withEdge));
            GrAssert(4*sizeof(GrPoint) == VertexSize(withEdge));
        #if GR_DEBUG
            GrVertexLayout withColorAndEdge = withColor | kEdge_VertexLayoutBit;
        #endif
            GrAssert(2*sizeof(GrPoint) == VertexColorOffset(withColorAndEdge));
            GrAssert(2*sizeof(GrPoint) + sizeof(GrColor) == VertexEdgeOffset(withColorAndEdge));
            GrAssert(4*sizeof(GrPoint) + sizeof(GrColor) == VertexSize(withColorAndEdge));
        #if GR_DEBUG
            GrVertexLayout withCoverage = tcMask | kCoverage_VertexLayoutBit;
        #endif
            GrAssert(-1 == VertexColorOffset(withCoverage));
            GrAssert(2*sizeof(GrPoint) == VertexCoverageOffset(withCoverage));
            GrAssert(2*sizeof(GrPoint) + sizeof(GrColor) == VertexSize(withCoverage));
        #if GR_DEBUG
            GrVertexLayout withCoverageAndColor = tcMask | kCoverage_VertexLayoutBit |
                                                    kColor_VertexLayoutBit;
        #endif
            GrAssert(2*sizeof(GrPoint) == VertexColorOffset(withCoverageAndColor));
            GrAssert(2*sizeof(GrPoint) + sizeof(GrColor) == VertexCoverageOffset(withCoverageAndColor));
            GrAssert(2*sizeof(GrPoint) + 2 * sizeof(GrColor) == VertexSize(withCoverageAndColor));
        }
        GrAssert(kTexCoordMask == tcMask);

        int stageOffsets[GrDrawState::kNumStages];
        int colorOffset;
        int edgeOffset;
        int coverageOffset;
        int size;
        size = VertexSizeAndOffsetsByStage(tcMask,
                                           stageOffsets, &colorOffset,
                                           &coverageOffset, &edgeOffset);
        GrAssert(2*sizeof(GrPoint) == size);
        GrAssert(-1 == colorOffset);
        GrAssert(-1 == coverageOffset);
        GrAssert(-1 == edgeOffset);
        for (int s = 0; s < GrDrawState::kNumStages; ++s) {
            GrAssert(sizeof(GrPoint) == stageOffsets[s]);
            GrAssert(sizeof(GrPoint) == VertexStageCoordOffset(s, tcMask));
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

bool GrDrawState::StageUsesTexCoords(GrVertexLayout layout, int stageIdx) {
    return SkToBool(layout & StageTexCoordVertexLayoutBit(stageIdx));
}

bool GrDrawState::srcAlphaWillBeOne(GrVertexLayout layout) const {

    uint32_t validComponentFlags;
    GrColor color;
    // Check if per-vertex or constant color may have partial alpha
    if (layout & kColor_VertexLayoutBit) {
        validComponentFlags = 0;
        color = 0; // not strictly necessary but we get false alarms from tools about uninit.
    } else {
        validComponentFlags = GrEffect::kAll_ValidComponentFlags;
        color = this->getColor();
    }

    // Run through the color stages
    int stageCnt = getFirstCoverageStage();
    for (int s = 0; s < stageCnt; ++s) {
        const GrEffectRef* effect = this->getStage(s).getEffect();
        if (NULL != effect) {
            (*effect)->getConstantColorComponents(&color, &validComponentFlags);
        }
    }

    // Check if the color filter could introduce an alpha.
    // We could skip the above work when this is true, but it is rare and the right fix is to make
    // the color filter a GrEffect and implement getConstantColorComponents() for it.
    if (SkXfermode::kDst_Mode != this->getColorFilterMode()) {
        validComponentFlags = 0;
    }

    // Check whether coverage is treated as color. If so we run through the coverage computation.
    if (this->isCoverageDrawing()) {
        GrColor coverageColor = this->getCoverage();
        GrColor oldColor = color;
        color = 0;
        for (int c = 0; c < 4; ++c) {
            if (validComponentFlags & (1 << c)) {
                U8CPU a = (oldColor >> (c * 8)) & 0xff;
                U8CPU b = (coverageColor >> (c * 8)) & 0xff;
                color |= (SkMulDiv255Round(a, b) << (c * 8));
            }
        }
        for (int s = this->getFirstCoverageStage(); s < GrDrawState::kNumStages; ++s) {
            const GrEffectRef* effect = this->getStage(s).getEffect();
            if (NULL != effect) {
                (*effect)->getConstantColorComponents(&color, &validComponentFlags);
            }
        }
    }
    return (GrEffect::kA_ValidComponentFlag & validComponentFlags) && 0xff == GrColorUnpackA(color);
}

bool GrDrawState::hasSolidCoverage(GrVertexLayout layout) const {
    // If we're drawing coverage directly then coverage is effectively treated as color.
    if (this->isCoverageDrawing()) {
        return true;
    }

    GrColor coverage;
    uint32_t validComponentFlags;
    // Initialize to an unknown starting coverage if per-vertex coverage is specified.
    if (layout & kCoverage_VertexLayoutBit) {
        validComponentFlags = 0;
    } else {
        coverage = fCommon.fCoverage;
        validComponentFlags = GrEffect::kAll_ValidComponentFlags;
    }

    // Run through the coverage stages and see if the coverage will be all ones at the end.
    for (int s = this->getFirstCoverageStage(); s < GrDrawState::kNumStages; ++s) {
        const GrEffectRef* effect = this->getStage(s).getEffect();
        if (NULL != effect) {
            (*effect)->getConstantColorComponents(&coverage, &validComponentFlags);
        }
    }
    return (GrEffect::kAll_ValidComponentFlags == validComponentFlags)  && (0xffffffff == coverage);
}

////////////////////////////////////////////////////////////////////////////////

// Some blend modes allow folding a fractional coverage value into the color's alpha channel, while
// others will blend incorrectly.
bool GrDrawState::canTweakAlphaForCoverage() const {
    /*
     The fractional coverage is f.
     The src and dst coeffs are Cs and Cd.
     The dst and src colors are S and D.
     We want the blend to compute: f*Cs*S + (f*Cd + (1-f))D. By tweaking the source color's alpha
     we're replacing S with S'=fS. It's obvious that that first term will always be ok. The second
     term can be rearranged as [1-(1-Cd)f]D. By substituting in the various possibilities for Cd we
     find that only 1, ISA, and ISC produce the correct destination when applied to S' and D.
     Also, if we're directly rendering coverage (isCoverageDrawing) then coverage is treated as
     color by definition.
     */
    return kOne_GrBlendCoeff == fCommon.fDstBlend ||
           kISA_GrBlendCoeff == fCommon.fDstBlend ||
           kISC_GrBlendCoeff == fCommon.fDstBlend ||
           this->isCoverageDrawing();
}

GrDrawState::BlendOptFlags GrDrawState::getBlendOpts(bool forceCoverage,
                                                     GrBlendCoeff* srcCoeff,
                                                     GrBlendCoeff* dstCoeff) const {
    GrVertexLayout layout = this->getVertexLayout();

    GrBlendCoeff bogusSrcCoeff, bogusDstCoeff;
    if (NULL == srcCoeff) {
        srcCoeff = &bogusSrcCoeff;
    }
    *srcCoeff = this->getSrcBlendCoeff();

    if (NULL == dstCoeff) {
        dstCoeff = &bogusDstCoeff;
    }
    *dstCoeff = this->getDstBlendCoeff();

    if (this->isColorWriteDisabled()) {
        *srcCoeff = kZero_GrBlendCoeff;
        *dstCoeff = kOne_GrBlendCoeff;
    }

    bool srcAIsOne = this->srcAlphaWillBeOne(layout);
    bool dstCoeffIsOne = kOne_GrBlendCoeff == *dstCoeff ||
                         (kSA_GrBlendCoeff == *dstCoeff && srcAIsOne);
    bool dstCoeffIsZero = kZero_GrBlendCoeff == *dstCoeff ||
                         (kISA_GrBlendCoeff == *dstCoeff && srcAIsOne);

    bool covIsZero = !this->isCoverageDrawing() &&
                     !(layout & GrDrawState::kCoverage_VertexLayoutBit) &&
                     0 == this->getCoverage();
    // When coeffs are (0,1) there is no reason to draw at all, unless
    // stenciling is enabled. Having color writes disabled is effectively
    // (0,1). The same applies when coverage is known to be 0.
    if ((kZero_GrBlendCoeff == *srcCoeff && dstCoeffIsOne) || covIsZero) {
        if (this->getStencil().doesWrite()) {
            return kDisableBlend_BlendOptFlag |
                   kEmitTransBlack_BlendOptFlag;
        } else {
            return kSkipDraw_BlendOptFlag;
        }
    }

    // check for coverage due to constant coverage, per-vertex coverage,
    // edge aa or coverage stage
    bool hasCoverage = forceCoverage ||
                       0xffffffff != this->getCoverage() ||
                       (layout & GrDrawState::kCoverage_VertexLayoutBit) ||
                       (layout & GrDrawState::kEdge_VertexLayoutBit);
    for (int s = this->getFirstCoverageStage();
         !hasCoverage && s < GrDrawState::kNumStages;
         ++s) {
        if (this->isStageEnabled(s)) {
            hasCoverage = true;
        }
    }

    // if we don't have coverage we can check whether the dst
    // has to read at all. If not, we'll disable blending.
    if (!hasCoverage) {
        if (dstCoeffIsZero) {
            if (kOne_GrBlendCoeff == *srcCoeff) {
                // if there is no coverage and coeffs are (1,0) then we
                // won't need to read the dst at all, it gets replaced by src
                return kDisableBlend_BlendOptFlag;
            } else if (kZero_GrBlendCoeff == *srcCoeff) {
                // if the op is "clear" then we don't need to emit a color
                // or blend, just write transparent black into the dst.
                *srcCoeff = kOne_GrBlendCoeff;
                *dstCoeff = kZero_GrBlendCoeff;
                return kDisableBlend_BlendOptFlag | kEmitTransBlack_BlendOptFlag;
            }
        }
    } else if (this->isCoverageDrawing()) {
        // we have coverage but we aren't distinguishing it from alpha by request.
        return kCoverageAsAlpha_BlendOptFlag;
    } else {
        // check whether coverage can be safely rolled into alpha
        // of if we can skip color computation and just emit coverage
        if (this->canTweakAlphaForCoverage()) {
            return kCoverageAsAlpha_BlendOptFlag;
        }
        if (dstCoeffIsZero) {
            if (kZero_GrBlendCoeff == *srcCoeff) {
                // the source color is not included in the blend
                // the dst coeff is effectively zero so blend works out to:
                // (c)(0)D + (1-c)D = (1-c)D.
                *dstCoeff = kISA_GrBlendCoeff;
                return  kEmitCoverage_BlendOptFlag;
            } else if (srcAIsOne) {
                // the dst coeff is effectively zero so blend works out to:
                // cS + (c)(0)D + (1-c)D = cS + (1-c)D.
                // If Sa is 1 then we can replace Sa with c
                // and set dst coeff to 1-Sa.
                *dstCoeff = kISA_GrBlendCoeff;
                return  kCoverageAsAlpha_BlendOptFlag;
            }
        } else if (dstCoeffIsOne) {
            // the dst coeff is effectively one so blend works out to:
            // cS + (c)(1)D + (1-c)D = cS + D.
            *dstCoeff = kOne_GrBlendCoeff;
            return  kCoverageAsAlpha_BlendOptFlag;
        }
    }
    return kNone_BlendOpt;
}

////////////////////////////////////////////////////////////////////////////////

void GrDrawState::AutoViewMatrixRestore::restore() {
    if (NULL != fDrawState) {
        fDrawState->setViewMatrix(fViewMatrix);
        for (int s = 0; s < GrDrawState::kNumStages; ++s) {
            if (fRestoreMask & (1 << s)) {
                fDrawState->fStages[s].restoreCoordChange(fSavedCoordChanges[s]);
            }
        }
    }
    fDrawState = NULL;
}

void GrDrawState::AutoViewMatrixRestore::set(GrDrawState* drawState,
                                             const SkMatrix& preconcatMatrix,
                                             uint32_t explicitCoordStageMask) {
    this->restore();

    fDrawState = drawState;
    if (NULL == drawState) {
        return;
    }

    fRestoreMask = 0;
    fViewMatrix = drawState->getViewMatrix();
    drawState->preConcatViewMatrix(preconcatMatrix);
    for (int s = 0; s < GrDrawState::kNumStages; ++s) {
        if (!(explicitCoordStageMask & (1 << s)) && drawState->isStageEnabled(s)) {
            fRestoreMask |= (1 << s);
            fDrawState->fStages[s].saveCoordChange(&fSavedCoordChanges[s]);
            drawState->fStages[s].preConcatCoordChange(preconcatMatrix);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

void GrDrawState::AutoDeviceCoordDraw::restore() {
    if (NULL != fDrawState) {
        fDrawState->setViewMatrix(fViewMatrix);
        for (int s = 0; s < GrDrawState::kNumStages; ++s) {
            if (fRestoreMask & (1 << s)) {
                fDrawState->fStages[s].restoreCoordChange(fSavedCoordChanges[s]);
            }
        }
    }
    fDrawState = NULL;
}

bool GrDrawState::AutoDeviceCoordDraw::set(GrDrawState* drawState,
                                           uint32_t explicitCoordStageMask) {
    GrAssert(NULL != drawState);

    this->restore();

    fDrawState = drawState;
    if (NULL == fDrawState) {
        return false;
    }

    fViewMatrix = drawState->getViewMatrix();
    fRestoreMask = 0;
    SkMatrix invVM;
    bool inverted = false;

    for (int s = 0; s < GrDrawState::kNumStages; ++s) {
        if (!(explicitCoordStageMask & (1 << s)) && drawState->isStageEnabled(s)) {
            if (!inverted && !fViewMatrix.invert(&invVM)) {
                // sad trombone sound
                fDrawState = NULL;
                return false;
            } else {
                inverted = true;
            }
            fRestoreMask |= (1 << s);
            GrEffectStage* stage = drawState->fStages + s;
            stage->saveCoordChange(&fSavedCoordChanges[s]);
            stage->preConcatCoordChange(invVM);
        }
    }
    drawState->viewMatrix()->reset();
    return true;
}
