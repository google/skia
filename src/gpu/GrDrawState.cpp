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

static size_t vertex_size(const GrVertexAttrib* attribs, int count) {
    // this works as long as we're 4 byte-aligned
#if GR_DEBUG
    uint32_t overlapCheck = 0;
#endif
    GrAssert(count <= GrDrawState::kMaxVertexAttribCnt);
    size_t size = 0;
    for (int index = 0; index < count; ++index) {
        size_t attribSize = GrVertexAttribTypeSize(attribs[index].fType);
        size += attribSize;
#if GR_DEBUG
        size_t dwordCount = attribSize >> 2;
        uint32_t mask = (1 << dwordCount)-1;
        size_t offsetShift = attribs[index].fOffset >> 2;
        GrAssert(!(overlapCheck & (mask << offsetShift)));
        overlapCheck |= (mask << offsetShift);
#endif
    }
    return size;
}

size_t GrDrawState::getVertexSize() const {
    return vertex_size(fCommon.fVertexAttribs.begin(), fCommon.fVertexAttribs.count());
}

////////////////////////////////////////////////////////////////////////////////

void GrDrawState::setVertexAttribs(const GrVertexAttrib* attribs, int count) {
    GrAssert(count <= kMaxVertexAttribCnt);
    fCommon.fVertexAttribs.reset(attribs, count);

    // Set all the indices to -1
    memset(fCommon.fFixedFunctionVertexAttribIndices,
           0xff,
           sizeof(fCommon.fFixedFunctionVertexAttribIndices));
#if GR_DEBUG
    uint32_t overlapCheck = 0;
#endif
    for (int i = 0; i < count; ++i) {
        if (attribs[i].fBinding < kGrFixedFunctionVertexAttribBindingCnt) {
            // The fixed function attribs can only be specified once
            GrAssert(-1 == fCommon.fFixedFunctionVertexAttribIndices[attribs[i].fBinding]);
            GrAssert(GrFixedFunctionVertexAttribVectorCount(attribs[i].fBinding) ==
                     GrVertexAttribTypeVectorCount(attribs[i].fType));
            fCommon.fFixedFunctionVertexAttribIndices[attribs[i].fBinding] = i;
        }
#if GR_DEBUG
        size_t dwordCount = GrVertexAttribTypeSize(attribs[i].fType) >> 2;
        uint32_t mask = (1 << dwordCount)-1;
        size_t offsetShift = attribs[i].fOffset >> 2;
        GrAssert(!(overlapCheck & (mask << offsetShift)));
        overlapCheck |= (mask << offsetShift);
#endif
    }
    // Positions must be specified.
    GrAssert(-1 != fCommon.fFixedFunctionVertexAttribIndices[kPosition_GrVertexAttribBinding]);
}

////////////////////////////////////////////////////////////////////////////////

void GrDrawState::setDefaultVertexAttribs() {
    static const GrVertexAttrib kPositionAttrib =
        {kVec2f_GrVertexAttribType, 0, kPosition_GrVertexAttribBinding};
    fCommon.fVertexAttribs.reset(&kPositionAttrib, 1);
    // set all the fixed function indices to -1 except position.
    memset(fCommon.fFixedFunctionVertexAttribIndices,
           0xff,
           sizeof(fCommon.fFixedFunctionVertexAttribIndices));
    fCommon.fFixedFunctionVertexAttribIndices[kPosition_GrVertexAttribBinding] = 0;
}

////////////////////////////////////////////////////////////////////////////////

bool GrDrawState::validateVertexAttribs() const {
    // check consistency of effects and attributes
    GrSLType slTypes[kMaxVertexAttribCnt];
    for (int i = 0; i < kMaxVertexAttribCnt; ++i) {
        slTypes[i] = static_cast<GrSLType>(-1);
    }
    for (int s = 0; s < kNumStages; ++s) {
        if (this->isStageEnabled(s)) {
            const GrEffectStage& stage = fStages[s];
            const GrEffectRef* effect = stage.getEffect();
            // make sure that any attribute indices have the correct binding type, that the attrib
            // type and effect's shader lang type are compatible, and that attributes shared by
            // multiple effects use the same shader lang type.
            const int* attributeIndices = stage.getVertexAttribIndices();
            int numAttributes = stage.getVertexAttribIndexCount();
            for (int i = 0; i < numAttributes; ++i) {
                int attribIndex = attributeIndices[i];
                if (attribIndex >= fCommon.fVertexAttribs.count() ||
                    kEffect_GrVertexAttribBinding != fCommon.fVertexAttribs[attribIndex].fBinding) {
                    return false;
                }

                GrSLType effectSLType = (*effect)->vertexAttribType(i);
                GrVertexAttribType attribType = fCommon.fVertexAttribs[attribIndex].fType;
                int slVecCount = GrSLTypeVectorCount(effectSLType);
                int attribVecCount = GrVertexAttribTypeVectorCount(attribType);
                if (slVecCount != attribVecCount ||
                    (static_cast<GrSLType>(-1) != slTypes[attribIndex] &&
                     slTypes[attribIndex] != effectSLType)) {
                    return false;
                }
                slTypes[attribIndex] = effectSLType;
            }
        }
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////

bool GrDrawState::srcAlphaWillBeOne() const {
    uint32_t validComponentFlags;
    GrColor color;
    // Check if per-vertex or constant color may have partial alpha
    if (this->hasColorVertexAttribute()) {
        validComponentFlags = 0;
        color = 0; // not strictly necessary but we get false alarms from tools about uninit.
    } else {
        validComponentFlags = kRGBA_GrColorComponentFlags;
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
    return (kA_GrColorComponentFlag & validComponentFlags) && 0xff == GrColorUnpackA(color);
}

bool GrDrawState::hasSolidCoverage() const {
    // If we're drawing coverage directly then coverage is effectively treated as color.
    if (this->isCoverageDrawing()) {
        return true;
    }

    GrColor coverage;
    uint32_t validComponentFlags;
    // Initialize to an unknown starting coverage if per-vertex coverage is specified.
    if (this->hasCoverageVertexAttribute()) {
        validComponentFlags = 0;
    } else {
        coverage = fCommon.fCoverage;
        validComponentFlags = kRGBA_GrColorComponentFlags;
    }

    // Run through the coverage stages and see if the coverage will be all ones at the end.
    for (int s = this->getFirstCoverageStage(); s < GrDrawState::kNumStages; ++s) {
        const GrEffectRef* effect = this->getStage(s).getEffect();
        if (NULL != effect) {
            (*effect)->getConstantColorComponents(&coverage, &validComponentFlags);
        }
    }
    return (kRGBA_GrColorComponentFlags == validComponentFlags)  && (0xffffffff == coverage);
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

    bool srcAIsOne = this->srcAlphaWillBeOne();
    bool dstCoeffIsOne = kOne_GrBlendCoeff == *dstCoeff ||
                         (kSA_GrBlendCoeff == *dstCoeff && srcAIsOne);
    bool dstCoeffIsZero = kZero_GrBlendCoeff == *dstCoeff ||
                         (kISA_GrBlendCoeff == *dstCoeff && srcAIsOne);

    bool covIsZero = !this->isCoverageDrawing() &&
                     !this->hasCoverageVertexAttribute() &&
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

    // check for coverage due to constant coverage, per-vertex coverage, or coverage stage
    bool hasCoverage = forceCoverage ||
                       0xffffffff != this->getCoverage() ||
                       this->hasCoverageVertexAttribute();
    for (int s = this->getFirstCoverageStage(); !hasCoverage && s < GrDrawState::kNumStages; ++s) {
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
                                             const SkMatrix& preconcatMatrix) {
    this->restore();

    fDrawState = drawState;
    if (NULL == drawState) {
        return;
    }

    fRestoreMask = 0;
    fViewMatrix = drawState->getViewMatrix();
    drawState->preConcatViewMatrix(preconcatMatrix);
    for (int s = 0; s < GrDrawState::kNumStages; ++s) {
        if (drawState->isStageEnabled(s)) {
            fRestoreMask |= (1 << s);
            fDrawState->fStages[s].saveCoordChange(&fSavedCoordChanges[s]);
            drawState->fStages[s].localCoordChange(preconcatMatrix);
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

bool GrDrawState::AutoDeviceCoordDraw::set(GrDrawState* drawState) {
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
        if (drawState->isStageEnabled(s)) {
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
            stage->localCoordChange(invVM);
        }
    }
    drawState->viewMatrix()->reset();
    return true;
}
