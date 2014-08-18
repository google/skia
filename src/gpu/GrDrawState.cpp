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

bool GrDrawState::State::HaveCompatibleState(const State& a, const State& b,
                                             bool explicitLocalCoords) {
    if (a.fColorStages.count() != b.fColorStages.count() ||
        a.fCoverageStages.count() != b.fCoverageStages.count() ||
        a.fSrcBlend != b.fSrcBlend ||
        a.fDstBlend != b.fDstBlend) {
        return false;
    }
    for (int i = 0; i < a.fColorStages.count(); i++) {
        if (!GrEffectStage::AreCompatible(a.fColorStages[i], b.fColorStages[i],
                                          explicitLocalCoords)) {
            return false;
        }
    }
    for (int i = 0; i < a.fCoverageStages.count(); i++) {
        if (!GrEffectStage::AreCompatible(a.fCoverageStages[i], b.fCoverageStages[i],
                                          explicitLocalCoords)) {
            return false;
        }
    }
    return true;
}
//////////////////////////////////////////////////////////////////////////////s
GrDrawState::CombinedState GrDrawState::CombineIfPossible(
    const GrDrawState& a, const GrDrawState& b, const GrDrawTargetCaps& caps) {

    bool usingVertexColors = a.hasColorVertexAttribute();
    if (!usingVertexColors && a.fColor != b.fColor) {
        return kIncompatible_CombinedState;
    }

    if (a.fRenderTarget.get() != b.fRenderTarget.get() ||
        !a.fViewMatrix.cheapEqualTo(b.fViewMatrix) ||
        a.fBlendConstant != b.fBlendConstant ||
        a.fFlagBits != b.fFlagBits ||
        a.fVACount != b.fVACount ||
        memcmp(a.fVAPtr, b.fVAPtr, a.fVACount * sizeof(GrVertexAttrib)) ||
        a.fStencilSettings != b.fStencilSettings ||
        a.fDrawFace != b.fDrawFace) {
        return kIncompatible_CombinedState;
    }

    bool usingVertexCoverage = a.hasCoverageVertexAttribute();
    if (!usingVertexCoverage && a.fCoverage != b.fCoverage) {
        return kIncompatible_CombinedState;
    }

    bool explicitLocalCoords = a.hasLocalCoordAttribute();

    SkASSERT(0 == memcmp(a.fFixedFunctionVertexAttribIndices,
                            b.fFixedFunctionVertexAttribIndices,
                            sizeof(a.fFixedFunctionVertexAttribIndices)));
    if (!State::HaveCompatibleState(a.fState, b.fState, explicitLocalCoords)) {
        return kIncompatible_CombinedState;
    }

    if (usingVertexColors) {
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
            fState.fColorStages[i].localCoordChange(preConcatMatrix);
        }
        for (int i = 0; i < this->numCoverageStages(); ++i) {
            fState.fCoverageStages[i].localCoordChange(preConcatMatrix);
        }
        this->invalidateBlendOptFlags();
    }
}

GrDrawState& GrDrawState::operator=(const GrDrawState& that) {
    SkASSERT(0 == fBlockEffectRemovalCnt || 0 == this->numTotalStages());
    this->setRenderTarget(that.fRenderTarget.get());
    fColor = that.fColor;
    fViewMatrix = that.fViewMatrix;
    fBlendConstant = that.fBlendConstant;
    fFlagBits = that.fFlagBits;
    fVACount = that.fVACount;
    fVAPtr = that.fVAPtr;
    fStencilSettings = that.fStencilSettings;
    fCoverage = that.fCoverage;
    fDrawFace = that.fDrawFace;
    fOptSrcBlend = that.fOptSrcBlend;
    fOptDstBlend = that.fOptDstBlend;
    fBlendOptFlags = that.fBlendOptFlags;

    fState = that.fState;
    fHints = that.fHints;

    memcpy(fFixedFunctionVertexAttribIndices,
            that.fFixedFunctionVertexAttribIndices,
            sizeof(fFixedFunctionVertexAttribIndices));
    return *this;
}

void GrDrawState::onReset(const SkMatrix* initialViewMatrix) {
    SkASSERT(0 == fBlockEffectRemovalCnt || 0 == this->numTotalStages());
    fState.reset();

    fRenderTarget.reset(NULL);

    this->setDefaultVertexAttribs();

    fColor = 0xffffffff;
    if (NULL == initialViewMatrix) {
        fViewMatrix.reset();
    } else {
        fViewMatrix = *initialViewMatrix;
    }
    fBlendConstant = 0x0;
    fFlagBits = 0x0;
    fStencilSettings.setDisabled();
    fCoverage = 0xffffffff;
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
            fState.fColorStages[s].localCoordChange(invVM);
        }
        for (int s = 0; s < this->numCoverageStages(); ++s) {
            fState.fCoverageStages[s].localCoordChange(invVM);
        }
    }
    fViewMatrix.reset();
    return true;
}

void GrDrawState::setFromPaint(const GrPaint& paint, const SkMatrix& vm, GrRenderTarget* rt) {
    SkASSERT(0 == fBlockEffectRemovalCnt || 0 == this->numTotalStages());

    fState.reset();

    for (int i = 0; i < paint.numColorStages(); ++i) {
        fState.fColorStages.push_back(paint.getColorStage(i));
    }

    for (int i = 0; i < paint.numCoverageStages(); ++i) {
        fState.fCoverageStages.push_back(paint.getCoverageStage(i));
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
    SkASSERT(count <= GrDrawState::kMaxVertexAttribCnt);
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

size_t GrDrawState::getVertexSize() const {
    return vertex_size(fVAPtr, fVACount);
}

////////////////////////////////////////////////////////////////////////////////

void GrDrawState::setVertexAttribs(const GrVertexAttrib* attribs, int count) {
    SkASSERT(count <= kMaxVertexAttribCnt);

    fVAPtr = attribs;
    fVACount = count;

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

    // set all the fixed function indices to -1 except position.
    memset(fFixedFunctionVertexAttribIndices,
           0xff,
           sizeof(fFixedFunctionVertexAttribIndices));
    fFixedFunctionVertexAttribIndices[kPosition_GrVertexAttribBinding] = 0;
    this->invalidateBlendOptFlags();
}

////////////////////////////////////////////////////////////////////////////////

bool GrDrawState::validateVertexAttribs() const {
    // check consistency of effects and attributes
    GrSLType slTypes[kMaxVertexAttribCnt];
    for (int i = 0; i < kMaxVertexAttribCnt; ++i) {
        slTypes[i] = static_cast<GrSLType>(-1);
    }
    int totalStages = this->numTotalStages();
    for (int s = 0; s < totalStages; ++s) {
        int covIdx = s - this->numColorStages();
        const GrEffectStage& stage = covIdx < 0 ? this->getColorStage(s) :
                                                  this->getCoverageStage(covIdx);
        const GrEffect* effect = stage.getEffect();
        SkASSERT(NULL != effect);
        // make sure that any attribute indices have the correct binding type, that the attrib
        // type and effect's shader lang type are compatible, and that attributes shared by
        // multiple effects use the same shader lang type.
        const int* attributeIndices = stage.getVertexAttribIndices();
        int numAttributes = stage.getVertexAttribIndexCount();
        for (int i = 0; i < numAttributes; ++i) {
            int attribIndex = attributeIndices[i];
            if (attribIndex >= fVACount ||
                kEffect_GrVertexAttribBinding != fVAPtr[attribIndex].fBinding) {
                return false;
            }

            GrSLType effectSLType = effect->vertexAttribType(i);
            GrVertexAttribType attribType = fVAPtr[attribIndex].fType;
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

    return true;
}

bool GrDrawState::willEffectReadDstColor() const {
    if (!this->isColorWriteDisabled()) {
        for (int s = 0; s < this->numColorStages(); ++s) {
            if (this->getColorStage(s).getEffect()->willReadDstColor()) {
                return true;
            }
        }
    }
    for (int s = 0; s < this->numCoverageStages(); ++s) {
        if (this->getCoverageStage(s).getEffect()->willReadDstColor()) {
            return true;
        }
    }
    return false;
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
    GrDrawState::BlendOptFlags flag = this->getBlendOpts(true, &srcCoeff, &dstCoeff);
    return GrDrawState::kNone_BlendOpt != flag ||
           (this->willEffectReadDstColor() &&
            kOne_GrBlendCoeff == srcCoeff && kZero_GrBlendCoeff == dstCoeff);
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
        coverage = fCoverage;
        validComponentFlags = kRGBA_GrColorComponentFlags;
    }

    // Run through the coverage stages and see if the coverage will be all ones at the end.
    for (int s = 0; s < this->numCoverageStages(); ++s) {
        const GrEffect* effect = this->getCoverageStage(s).getEffect();
        effect->getConstantColorComponents(&coverage, &validComponentFlags);
    }
    return (kRGBA_GrColorComponentFlags == validComponentFlags) && (0xffffffff == coverage);
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
    return kOne_GrBlendCoeff == fState.fDstBlend ||
           kISA_GrBlendCoeff == fState.fDstBlend ||
           kISC_GrBlendCoeff == fState.fDstBlend ||
           this->isCoverageDrawing();
}

GrDrawState::BlendOptFlags GrDrawState::getBlendOpts(bool forceCoverage,
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

GrDrawState::BlendOptFlags GrDrawState::calcBlendOpts(bool forceCoverage,
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

bool GrDrawState::canIgnoreColorAttribute() const {
    if (fBlendOptFlags & kInvalid_BlendOptFlag) {
        this->getBlendOpts();
    }
    return SkToBool(fBlendOptFlags & (GrDrawState::kEmitTransBlack_BlendOptFlag |
                                      GrDrawState::kEmitCoverage_BlendOptFlag));
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
        fDrawState->fState.fColorStages.pop_back_n(m);

        int n = fDrawState->numCoverageStages() - fCoverageEffectCnt;
        SkASSERT(n >= 0);
        fDrawState->fState.fCoverageStages.pop_back_n(n);
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

void GrDrawState::AutoViewMatrixRestore::restore() {
    if (NULL != fDrawState) {
        SkDEBUGCODE(--fDrawState->fBlockEffectRemovalCnt;)
        fDrawState->fViewMatrix = fViewMatrix;
        SkASSERT(fDrawState->numColorStages() >= fNumColorStages);
        int numCoverageStages = fSavedCoordChanges.count() - fNumColorStages;
        SkASSERT(fDrawState->numCoverageStages() >= numCoverageStages);

        int i = 0;
        for (int s = 0; s < fNumColorStages; ++s, ++i) {
            fDrawState->fState.fColorStages[s].restoreCoordChange(fSavedCoordChanges[i]);
        }
        for (int s = 0; s < numCoverageStages; ++s, ++i) {
            fDrawState->fState.fCoverageStages[s].restoreCoordChange(fSavedCoordChanges[i]);
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
        fDrawState->fState.fColorStages[s].localCoordChange(coordChangeMatrix);
    }

    int numCoverageStages = fDrawState->numCoverageStages();
    for (int s = 0; s < numCoverageStages; ++s, ++i) {
        fDrawState->getCoverageStage(s).saveCoordChange(&fSavedCoordChanges[i]);
        fDrawState->fState.fCoverageStages[s].localCoordChange(coordChangeMatrix);
    }
}
