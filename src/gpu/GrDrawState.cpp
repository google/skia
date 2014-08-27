/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDrawState.h"
#include "GrPaint.h"
#include "GrDrawTargetCaps.h"

//////////////////////////////////////////////////////////////////////////////s

GrDrawState::CombinedState GrDrawState::CombineIfPossible(
    const GrDrawState& a, const GrDrawState& b, const GrDrawTargetCaps& caps) {

    if (!a.isEqual(b)) {
        return kIncompatible_CombinedState;
    }

    // If the general draw states are equal (from check above) we know hasColorVertexAttribute()
    // is equivalent for both a and b
    if (a.hasColorVertexAttribute()) {
        // If one is opaque and the other is not then the combined state is not opaque. Moreover,
        // if the opaqueness affects the ability to get color/coverage blending correct then we
        // don't combine the draw states.
        bool aIsOpaque = (kVertexColorsAreOpaque_Hint & a.fHints);
        bool bIsOpaque = (kVertexColorsAreOpaque_Hint & b.fHints);
        if (aIsOpaque != bIsOpaque) {
            const GrDrawState* opaque;
            const GrDrawState* nonOpaque;
            if (aIsOpaque) {
                opaque = &a;
                nonOpaque = &b;
            } else {
                opaque = &b;
                nonOpaque = &a;
            }
            if (!opaque->hasSolidCoverage() && opaque->couldApplyCoverage(caps)) {
                SkASSERT(!nonOpaque->hasSolidCoverage());
                if (!nonOpaque->couldApplyCoverage(caps)) {
                    return kIncompatible_CombinedState;
                }
            }
            return aIsOpaque ? kB_CombinedState : kA_CombinedState;
        }
    }
    return kAOrB_CombinedState;
}

//////////////////////////////////////////////////////////////////////////////s

GrDrawState::GrDrawState(const GrDrawState& state, const SkMatrix& preConcatMatrix) {
    SkDEBUGCODE(fBlockEffectRemovalCnt = 0;)
    *this = state;
    if (!preConcatMatrix.isIdentity()) {
        for (int i = 0; i < this->numColorStages(); ++i) {
            fColorStages[i].localCoordChange(preConcatMatrix);
        }
        for (int i = 0; i < this->numCoverageStages(); ++i) {
            fCoverageStages[i].localCoordChange(preConcatMatrix);
        }
        this->invalidateBlendOptFlags();
    }
}

GrDrawState& GrDrawState::operator=(const GrDrawState& that) {
    SkASSERT(0 == fBlockEffectRemovalCnt || 0 == this->numTotalStages());
    this->setRenderTarget(that.fRenderTarget.get());
    fColor = that.fColor;
    fViewMatrix = that.fViewMatrix;
    fSrcBlend = that.fSrcBlend;
    fDstBlend = that.fDstBlend;
    fBlendConstant = that.fBlendConstant;
    fFlagBits = that.fFlagBits;
    fVACount = that.fVACount;
    fVAPtr = that.fVAPtr;
    fVertexSize = that.fVertexSize;
    fStencilSettings = that.fStencilSettings;
    fCoverage = that.fCoverage;
    fDrawFace = that.fDrawFace;
    fColorStages = that.fColorStages;
    fCoverageStages = that.fCoverageStages;
    fOptSrcBlend = that.fOptSrcBlend;
    fOptDstBlend = that.fOptDstBlend;
    fBlendOptFlags = that.fBlendOptFlags;

    fHints = that.fHints;

    memcpy(fFixedFunctionVertexAttribIndices,
            that.fFixedFunctionVertexAttribIndices,
            sizeof(fFixedFunctionVertexAttribIndices));
    return *this;
}

void GrDrawState::onReset(const SkMatrix* initialViewMatrix) {
    SkASSERT(0 == fBlockEffectRemovalCnt || 0 == this->numTotalStages());
    fColorStages.reset();
    fCoverageStages.reset();

    fRenderTarget.reset(NULL);

    this->setDefaultVertexAttribs();

    fColor = 0xffffffff;
    if (NULL == initialViewMatrix) {
        fViewMatrix.reset();
    } else {
        fViewMatrix = *initialViewMatrix;
    }
    fSrcBlend = kOne_GrBlendCoeff;
    fDstBlend = kZero_GrBlendCoeff;
    fBlendConstant = 0x0;
    fFlagBits = 0x0;
    fStencilSettings.setDisabled();
    fCoverage = 0xff;
    fDrawFace = kBoth_DrawFace;

    fHints = 0;

    this->invalidateBlendOptFlags();
}

bool GrDrawState::setIdentityViewMatrix()  {
    if (this->numTotalStages()) {
        SkMatrix invVM;
        if (!fViewMatrix.invert(&invVM)) {
            // sad trombone sound
            return false;
        }
        for (int s = 0; s < this->numColorStages(); ++s) {
            fColorStages[s].localCoordChange(invVM);
        }
        for (int s = 0; s < this->numCoverageStages(); ++s) {
            fCoverageStages[s].localCoordChange(invVM);
        }
    }
    fViewMatrix.reset();
    return true;
}

void GrDrawState::setFromPaint(const GrPaint& paint, const SkMatrix& vm, GrRenderTarget* rt) {
    SkASSERT(0 == fBlockEffectRemovalCnt || 0 == this->numTotalStages());

    fColorStages.reset();
    fCoverageStages.reset();

    for (int i = 0; i < paint.numColorStages(); ++i) {
        fColorStages.push_back(paint.getColorStage(i));
    }

    for (int i = 0; i < paint.numCoverageStages(); ++i) {
        fCoverageStages.push_back(paint.getCoverageStage(i));
    }

    this->setRenderTarget(rt);

    fViewMatrix = vm;

    // These have no equivalent in GrPaint, set them to defaults
    fBlendConstant = 0x0;
    fDrawFace = kBoth_DrawFace;
    fStencilSettings.setDisabled();
    this->resetStateFlags();
    fHints = 0;

    // Enable the clip bit
    this->enableState(GrDrawState::kClip_StateBit);

    this->setColor(paint.getColor());
    this->setState(GrDrawState::kDither_StateBit, paint.isDither());
    this->setState(GrDrawState::kHWAntialias_StateBit, paint.isAntiAlias());

    this->setBlendFunc(paint.getSrcBlendCoeff(), paint.getDstBlendCoeff());
    this->setCoverage(paint.getCoverage());
    this->invalidateBlendOptFlags();
}

////////////////////////////////////////////////////////////////////////////////

static size_t vertex_size(const GrVertexAttrib* attribs, int count) {
    // this works as long as we're 4 byte-aligned
#ifdef SK_DEBUG
    uint32_t overlapCheck = 0;
#endif
    SkASSERT(count <= GrRODrawState::kMaxVertexAttribCnt);
    size_t size = 0;
    for (int index = 0; index < count; ++index) {
        size_t attribSize = GrVertexAttribTypeSize(attribs[index].fType);
        size += attribSize;
#ifdef SK_DEBUG
        size_t dwordCount = attribSize >> 2;
        uint32_t mask = (1 << dwordCount)-1;
        size_t offsetShift = attribs[index].fOffset >> 2;
        SkASSERT(!(overlapCheck & (mask << offsetShift)));
        overlapCheck |= (mask << offsetShift);
#endif
    }
    return size;
}

////////////////////////////////////////////////////////////////////////////////

void GrDrawState::setVertexAttribs(const GrVertexAttrib* attribs, int count) {
    SkASSERT(count <= kMaxVertexAttribCnt);

    fVAPtr = attribs;
    fVACount = count;
    fVertexSize = vertex_size(fVAPtr, fVACount);

    // Set all the indices to -1
    memset(fFixedFunctionVertexAttribIndices,
           0xff,
           sizeof(fFixedFunctionVertexAttribIndices));
#ifdef SK_DEBUG
    uint32_t overlapCheck = 0;
#endif
    for (int i = 0; i < count; ++i) {
        if (attribs[i].fBinding < kGrFixedFunctionVertexAttribBindingCnt) {
            // The fixed function attribs can only be specified once
            SkASSERT(-1 == fFixedFunctionVertexAttribIndices[attribs[i].fBinding]);
            SkASSERT(GrFixedFunctionVertexAttribVectorCount(attribs[i].fBinding) ==
                     GrVertexAttribTypeVectorCount(attribs[i].fType));
            fFixedFunctionVertexAttribIndices[attribs[i].fBinding] = i;
        }
#ifdef SK_DEBUG
        size_t dwordCount = GrVertexAttribTypeSize(attribs[i].fType) >> 2;
        uint32_t mask = (1 << dwordCount)-1;
        size_t offsetShift = attribs[i].fOffset >> 2;
        SkASSERT(!(overlapCheck & (mask << offsetShift)));
        overlapCheck |= (mask << offsetShift);
#endif
    }
    this->invalidateBlendOptFlags();
    // Positions must be specified.
    SkASSERT(-1 != fFixedFunctionVertexAttribIndices[kPosition_GrVertexAttribBinding]);
}

////////////////////////////////////////////////////////////////////////////////

void GrDrawState::setDefaultVertexAttribs() {
    static const GrVertexAttrib kPositionAttrib =
        {kVec2f_GrVertexAttribType, 0, kPosition_GrVertexAttribBinding};

    fVAPtr = &kPositionAttrib;
    fVACount = 1;
    fVertexSize = GrVertexAttribTypeSize(kVec2f_GrVertexAttribType);

    // set all the fixed function indices to -1 except position.
    memset(fFixedFunctionVertexAttribIndices,
           0xff,
           sizeof(fFixedFunctionVertexAttribIndices));
    fFixedFunctionVertexAttribIndices[kPosition_GrVertexAttribBinding] = 0;
    this->invalidateBlendOptFlags();
}

////////////////////////////////////////////////////////////////////////////////

bool GrDrawState::couldApplyCoverage(const GrDrawTargetCaps& caps) const {
    if (caps.dualSourceBlendingSupport()) {
        return true;
    }
    // we can correctly apply coverage if a) we have dual source blending
    // or b) one of our blend optimizations applies
    // or c) the src, dst blend coeffs are 1,0 and we will read Dst Color
    GrBlendCoeff srcCoeff;
    GrBlendCoeff dstCoeff;
    GrRODrawState::BlendOptFlags flag = this->getBlendOpts(true, &srcCoeff, &dstCoeff);
    return GrRODrawState::kNone_BlendOpt != flag ||
           (this->willEffectReadDstColor() &&
            kOne_GrBlendCoeff == srcCoeff && kZero_GrBlendCoeff == dstCoeff);
}

//////////////////////////////////////////////////////////////////////////////

GrDrawState::AutoVertexAttribRestore::AutoVertexAttribRestore(
    GrDrawState* drawState) {
    SkASSERT(NULL != drawState);
    fDrawState = drawState;
    fVAPtr = drawState->fVAPtr;
    fVACount = drawState->fVACount;
    fDrawState->setDefaultVertexAttribs();
}

//////////////////////////////////////////////////////////////////////////////s

void GrDrawState::AutoRestoreEffects::set(GrDrawState* ds) {
    if (NULL != fDrawState) {
        int m = fDrawState->numColorStages() - fColorEffectCnt;
        SkASSERT(m >= 0);
        fDrawState->fColorStages.pop_back_n(m);

        int n = fDrawState->numCoverageStages() - fCoverageEffectCnt;
        SkASSERT(n >= 0);
        fDrawState->fCoverageStages.pop_back_n(n);
        if (m + n > 0) {
            fDrawState->invalidateBlendOptFlags();
        }
        SkDEBUGCODE(--fDrawState->fBlockEffectRemovalCnt;)
    }
    fDrawState = ds;
    if (NULL != ds) {
        fColorEffectCnt = ds->numColorStages();
        fCoverageEffectCnt = ds->numCoverageStages();
        SkDEBUGCODE(++ds->fBlockEffectRemovalCnt;)
    }
}

////////////////////////////////////////////////////////////////////////////////

GrRODrawState::BlendOptFlags GrDrawState::getBlendOpts(bool forceCoverage,
                                                       GrBlendCoeff* srcCoeff,
                                                       GrBlendCoeff* dstCoeff) const {
    GrBlendCoeff bogusSrcCoeff, bogusDstCoeff;
    if (NULL == srcCoeff) {
        srcCoeff = &bogusSrcCoeff;
    }
    if (NULL == dstCoeff) {
        dstCoeff = &bogusDstCoeff;
    }

    if (forceCoverage) {
        return this->calcBlendOpts(true, srcCoeff, dstCoeff);
    }

    if (0 == (fBlendOptFlags & kInvalid_BlendOptFlag)) {
        *srcCoeff = fOptSrcBlend;
        *dstCoeff = fOptDstBlend;
        return fBlendOptFlags;
    }

    fBlendOptFlags = this->calcBlendOpts(forceCoverage, srcCoeff, dstCoeff);
    fOptSrcBlend = *srcCoeff;
    fOptDstBlend = *dstCoeff;

    return fBlendOptFlags;
}

GrRODrawState::BlendOptFlags GrDrawState::calcBlendOpts(bool forceCoverage,
                                                          GrBlendCoeff* srcCoeff,
                                                          GrBlendCoeff* dstCoeff) const {
    *srcCoeff = this->getSrcBlendCoeff();
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

    // When coeffs are (0,1) there is no reason to draw at all, unless
    // stenciling is enabled. Having color writes disabled is effectively
    // (0,1).
    if ((kZero_GrBlendCoeff == *srcCoeff && dstCoeffIsOne)) {
        if (this->getStencil().doesWrite()) {
            return kEmitCoverage_BlendOptFlag;
        } else {
            return kSkipDraw_BlendOptFlag;
        }
    }

    bool hasCoverage = forceCoverage || !this->hasSolidCoverage();

    // if we don't have coverage we can check whether the dst
    // has to read at all. If not, we'll disable blending.
    if (!hasCoverage) {
        if (dstCoeffIsZero) {
            if (kOne_GrBlendCoeff == *srcCoeff) {
                // if there is no coverage and coeffs are (1,0) then we
                // won't need to read the dst at all, it gets replaced by src
                *dstCoeff = kZero_GrBlendCoeff;
                return kNone_BlendOpt;
            } else if (kZero_GrBlendCoeff == *srcCoeff) {
                // if the op is "clear" then we don't need to emit a color
                // or blend, just write transparent black into the dst.
                *srcCoeff = kOne_GrBlendCoeff;
                *dstCoeff = kZero_GrBlendCoeff;
                return kEmitTransBlack_BlendOptFlag;
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
        SkDEBUGCODE(--fDrawState->fBlockEffectRemovalCnt;)
        fDrawState->fViewMatrix = fViewMatrix;
        SkASSERT(fDrawState->numColorStages() >= fNumColorStages);
        int numCoverageStages = fSavedCoordChanges.count() - fNumColorStages;
        SkASSERT(fDrawState->numCoverageStages() >= numCoverageStages);

        int i = 0;
        for (int s = 0; s < fNumColorStages; ++s, ++i) {
            fDrawState->fColorStages[s].restoreCoordChange(fSavedCoordChanges[i]);
        }
        for (int s = 0; s < numCoverageStages; ++s, ++i) {
            fDrawState->fCoverageStages[s].restoreCoordChange(fSavedCoordChanges[i]);
        }
        fDrawState = NULL;
    }
}

void GrDrawState::AutoViewMatrixRestore::set(GrDrawState* drawState,
                                             const SkMatrix& preconcatMatrix) {
    this->restore();

    SkASSERT(NULL == fDrawState);
    if (NULL == drawState || preconcatMatrix.isIdentity()) {
        return;
    }
    fDrawState = drawState;

    fViewMatrix = drawState->getViewMatrix();
    drawState->fViewMatrix.preConcat(preconcatMatrix);

    this->doEffectCoordChanges(preconcatMatrix);
    SkDEBUGCODE(++fDrawState->fBlockEffectRemovalCnt;)
}

bool GrDrawState::AutoViewMatrixRestore::setIdentity(GrDrawState* drawState) {
    this->restore();

    if (NULL == drawState) {
        return false;
    }

    if (drawState->getViewMatrix().isIdentity()) {
        return true;
    }

    fViewMatrix = drawState->getViewMatrix();
    if (0 == drawState->numTotalStages()) {
        drawState->fViewMatrix.reset();
        fDrawState = drawState;
        fNumColorStages = 0;
        fSavedCoordChanges.reset(0);
        SkDEBUGCODE(++fDrawState->fBlockEffectRemovalCnt;)
        return true;
    } else {
        SkMatrix inv;
        if (!fViewMatrix.invert(&inv)) {
            return false;
        }
        drawState->fViewMatrix.reset();
        fDrawState = drawState;
        this->doEffectCoordChanges(inv);
        SkDEBUGCODE(++fDrawState->fBlockEffectRemovalCnt;)
        return true;
    }
}

void GrDrawState::AutoViewMatrixRestore::doEffectCoordChanges(const SkMatrix& coordChangeMatrix) {
    fSavedCoordChanges.reset(fDrawState->numTotalStages());
    int i = 0;

    fNumColorStages = fDrawState->numColorStages();
    for (int s = 0; s < fNumColorStages; ++s, ++i) {
        fDrawState->getColorStage(s).saveCoordChange(&fSavedCoordChanges[i]);
        fDrawState->fColorStages[s].localCoordChange(coordChangeMatrix);
    }

    int numCoverageStages = fDrawState->numCoverageStages();
    for (int s = 0; s < numCoverageStages; ++s, ++i) {
        fDrawState->getCoverageStage(s).saveCoordChange(&fSavedCoordChanges[i]);
        fDrawState->fCoverageStages[s].localCoordChange(coordChangeMatrix);
    }
}

bool GrDrawState::srcAlphaWillBeOne() const {
    uint32_t validComponentFlags;
    GrColor color;
    // Check if per-vertex or constant color may have partial alpha
    if (this->hasColorVertexAttribute()) {
        if (fHints & kVertexColorsAreOpaque_Hint) {
            validComponentFlags = kA_GrColorComponentFlag;
            color = 0xFF << GrColor_SHIFT_A;
        } else {
            validComponentFlags = 0;
            color = 0; // not strictly necessary but we get false alarms from tools about uninit.
        }
    } else {
        validComponentFlags = kRGBA_GrColorComponentFlags;
        color = this->getColor();
    }

    // Run through the color stages
    for (int s = 0; s < this->numColorStages(); ++s) {
        const GrEffect* effect = this->getColorStage(s).getEffect();
        effect->getConstantColorComponents(&color, &validComponentFlags);
    }

    // Check whether coverage is treated as color. If so we run through the coverage computation.
    if (this->isCoverageDrawing()) {
        // The shader generated for coverage drawing runs the full coverage computation and then
        // makes the shader output be the multiplication of color and coverage. We mirror that here.
        GrColor coverage;
        uint32_t coverageComponentFlags;
        if (this->hasCoverageVertexAttribute()) {
            coverageComponentFlags = 0;
            coverage = 0; // suppresses any warnings.
        } else {
            coverageComponentFlags = kRGBA_GrColorComponentFlags;
            coverage = this->getCoverageColor();
        }

        // Run through the coverage stages
        for (int s = 0; s < this->numCoverageStages(); ++s) {
            const GrEffect* effect = this->getCoverageStage(s).getEffect();
            effect->getConstantColorComponents(&coverage, &coverageComponentFlags);
        }

        // Since the shader will multiply coverage and color, the only way the final A==1 is if
        // coverage and color both have A==1.
        return (kA_GrColorComponentFlag & validComponentFlags & coverageComponentFlags) &&
                0xFF == GrColorUnpackA(color) && 0xFF == GrColorUnpackA(coverage);

    }

    return (kA_GrColorComponentFlag & validComponentFlags) && 0xFF == GrColorUnpackA(color);
}

////////////////////////////////////////////////////////////////////////////////

bool GrDrawState::canIgnoreColorAttribute() const {
    if (fBlendOptFlags & kInvalid_BlendOptFlag) {
        this->getBlendOpts();
    }
    return SkToBool(fBlendOptFlags & (GrRODrawState::kEmitTransBlack_BlendOptFlag |
                                      GrRODrawState::kEmitCoverage_BlendOptFlag));
}

