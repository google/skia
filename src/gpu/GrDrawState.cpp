/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDrawState.h"

#include "GrOptDrawState.h"
#include "GrPaint.h"

//////////////////////////////////////////////////////////////////////////////s

GrOptDrawState* GrDrawState::createOptState(const GrDrawTargetCaps& caps) const {
    if (NULL == fCachedOptState || caps.getUniqueID() != fCachedCapsID) {
        GrBlendCoeff srcCoeff;
        GrBlendCoeff dstCoeff;
        BlendOptFlags blendFlags = this->getBlendOpts(false, &srcCoeff, &dstCoeff);
        fCachedOptState = SkNEW_ARGS(GrOptDrawState, (*this, blendFlags, srcCoeff, dstCoeff, caps));
        fCachedCapsID = caps.getUniqueID();
    } else {
#ifdef SK_DEBUG
        GrBlendCoeff srcCoeff;
        GrBlendCoeff dstCoeff;
        BlendOptFlags blendFlags = this->getBlendOpts(false, &srcCoeff, &dstCoeff);
        SkASSERT(GrOptDrawState(*this, blendFlags, srcCoeff, dstCoeff, caps) == *fCachedOptState);
#endif
    }
    fCachedOptState->ref();
    return fCachedOptState;
}

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

GrDrawState::GrDrawState(const GrDrawState& state, const SkMatrix& preConcatMatrix)
    : fCachedOptState(NULL) {
    SkDEBUGCODE(fBlockEffectRemovalCnt = 0;)
    *this = state;
    if (!preConcatMatrix.isIdentity()) {
        if (this->hasGeometryProcessor()) {
            fGeometryProcessor->localCoordChange(preConcatMatrix);
        }
        for (int i = 0; i < this->numColorStages(); ++i) {
            fColorStages[i].localCoordChange(preConcatMatrix);
        }
        for (int i = 0; i < this->numCoverageStages(); ++i) {
            fCoverageStages[i].localCoordChange(preConcatMatrix);
        }
        this->invalidateOptState();
    }
}

GrDrawState& GrDrawState::operator=(const GrDrawState& that) {
    SkASSERT(0 == fBlockEffectRemovalCnt || 0 == this->numTotalStages());
    SkASSERT(!that.fRenderTarget.ownsPendingIO());
    SkASSERT(!this->fRenderTarget.ownsPendingIO());
    this->setRenderTarget(that.getRenderTarget());
    fColor = that.fColor;
    fViewMatrix = that.fViewMatrix;
    fSrcBlend = that.fSrcBlend;
    fDstBlend = that.fDstBlend;
    fBlendConstant = that.fBlendConstant;
    fFlagBits = that.fFlagBits;
    fVACount = that.fVACount;
    fVAPtr = that.fVAPtr;
    fVAStride = that.fVAStride;
    fStencilSettings = that.fStencilSettings;
    fCoverage = that.fCoverage;
    fDrawFace = that.fDrawFace;
    if (that.hasGeometryProcessor()) {
        fGeometryProcessor.reset(SkNEW_ARGS(GrGeometryStage, (*that.fGeometryProcessor.get())));
    } else {
        fGeometryProcessor.reset(NULL);
    }
    fColorStages = that.fColorStages;
    fCoverageStages = that.fCoverageStages;

    fHints = that.fHints;

    SkRefCnt_SafeAssign(fCachedOptState, that.fCachedOptState);

    memcpy(fFixedFunctionVertexAttribIndices,
            that.fFixedFunctionVertexAttribIndices,
            sizeof(fFixedFunctionVertexAttribIndices));
    return *this;
}

void GrDrawState::onReset(const SkMatrix* initialViewMatrix) {
    SkASSERT(0 == fBlockEffectRemovalCnt || 0 == this->numTotalStages());
    SkASSERT(!fRenderTarget.ownsPendingIO());

    fGeometryProcessor.reset(NULL);
    fColorStages.reset();
    fCoverageStages.reset();

    fRenderTarget.reset();

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

    this->invalidateOptState();
}

bool GrDrawState::setIdentityViewMatrix()  {
    if (this->numTotalStages()) {
        SkMatrix invVM;
        if (!fViewMatrix.invert(&invVM)) {
            // sad trombone sound
            return false;
        }
        if (this->hasGeometryProcessor()) {
            fGeometryProcessor->localCoordChange(invVM);
        }
        for (int s = 0; s < this->numColorStages(); ++s) {
            fColorStages[s].localCoordChange(invVM);
        }
        for (int s = 0; s < this->numCoverageStages(); ++s) {
            fCoverageStages[s].localCoordChange(invVM);
        }
    }
    this->invalidateOptState();
    fViewMatrix.reset();
    return true;
}

void GrDrawState::setFromPaint(const GrPaint& paint, const SkMatrix& vm, GrRenderTarget* rt) {
    SkASSERT(0 == fBlockEffectRemovalCnt || 0 == this->numTotalStages());

    fGeometryProcessor.reset(NULL);
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
    this->invalidateOptState();
}

////////////////////////////////////////////////////////////////////////////////

static void validate_vertex_attribs(const GrVertexAttrib* attribs, int count, size_t stride) {
    // this works as long as we're 4 byte-aligned
#ifdef SK_DEBUG
    uint32_t overlapCheck = 0;
    SkASSERT(count <= GrRODrawState::kMaxVertexAttribCnt);
    for (int index = 0; index < count; ++index) {
        size_t attribSize = GrVertexAttribTypeSize(attribs[index].fType);
        size_t attribOffset = attribs[index].fOffset;
        SkASSERT(attribOffset + attribSize <= stride);
        size_t dwordCount = attribSize >> 2;
        uint32_t mask = (1 << dwordCount)-1;
        size_t offsetShift = attribOffset >> 2;
        SkASSERT(!(overlapCheck & (mask << offsetShift)));
        overlapCheck |= (mask << offsetShift);
    }
#endif
}

////////////////////////////////////////////////////////////////////////////////

void GrDrawState::internalSetVertexAttribs(const GrVertexAttrib* attribs, int count,
                                           size_t stride) {
    SkASSERT(count <= kMaxVertexAttribCnt);

    fVAPtr = attribs;
    fVACount = count;
    fVAStride = stride;
    validate_vertex_attribs(fVAPtr, fVACount, fVAStride);

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
    this->invalidateOptState();
    // Positions must be specified.
    SkASSERT(-1 != fFixedFunctionVertexAttribIndices[kPosition_GrVertexAttribBinding]);
}

////////////////////////////////////////////////////////////////////////////////

void GrDrawState::setDefaultVertexAttribs() {
    static const GrVertexAttrib kPositionAttrib =
        {kVec2f_GrVertexAttribType, 0, kPosition_GrVertexAttribBinding};

    fVAPtr = &kPositionAttrib;
    fVACount = 1;
    fVAStride = GrVertexAttribTypeSize(kVec2f_GrVertexAttribType);

    // set all the fixed function indices to -1 except position.
    memset(fFixedFunctionVertexAttribIndices,
           0xff,
           sizeof(fFixedFunctionVertexAttribIndices));
    fFixedFunctionVertexAttribIndices[kPosition_GrVertexAttribBinding] = 0;
    this->invalidateOptState();
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
    BlendOptFlags flag = this->getBlendOpts(true, &srcCoeff, &dstCoeff);
    return GrRODrawState::kNone_BlendOpt != flag ||
           (this->willEffectReadDstColor() &&
            kOne_GrBlendCoeff == srcCoeff && kZero_GrBlendCoeff == dstCoeff);
}

//////////////////////////////////////////////////////////////////////////////

GrDrawState::AutoVertexAttribRestore::AutoVertexAttribRestore(GrDrawState* drawState) {
    SkASSERT(drawState);
    fDrawState = drawState;
    fVAPtr = drawState->fVAPtr;
    fVACount = drawState->fVACount;
    fVAStride = drawState->fVAStride;
    fDrawState->setDefaultVertexAttribs();
}

//////////////////////////////////////////////////////////////////////////////s

void GrDrawState::AutoRestoreEffects::set(GrDrawState* ds) {
    if (fDrawState) {
        // See the big comment on the class definition about GPs.
        if (SK_InvalidUniqueID == fOriginalGPID) {
            fDrawState->fGeometryProcessor.reset(NULL);
        } else {
            SkASSERT(fDrawState->getGeometryProcessor()->getProcessor()->getUniqueID() ==
                     fOriginalGPID);
            fOriginalGPID = SK_InvalidUniqueID;
        }

        int m = fDrawState->numColorStages() - fColorEffectCnt;
        SkASSERT(m >= 0);
        fDrawState->fColorStages.pop_back_n(m);

        int n = fDrawState->numCoverageStages() - fCoverageEffectCnt;
        SkASSERT(n >= 0);
        fDrawState->fCoverageStages.pop_back_n(n);
        if (m + n > 0) {
            fDrawState->invalidateOptState();
        }
        SkDEBUGCODE(--fDrawState->fBlockEffectRemovalCnt;)
    }
    fDrawState = ds;
    if (NULL != ds) {
        SkASSERT(SK_InvalidUniqueID == fOriginalGPID);
        if (NULL != ds->getGeometryProcessor()) {
            fOriginalGPID = ds->getGeometryProcessor()->getProcessor()->getUniqueID();
        }
        fColorEffectCnt = ds->numColorStages();
        fCoverageEffectCnt = ds->numCoverageStages();
        SkDEBUGCODE(++ds->fBlockEffectRemovalCnt;)
    }
}

////////////////////////////////////////////////////////////////////////////////

void GrDrawState::AutoViewMatrixRestore::restore() {
    if (fDrawState) {
        SkDEBUGCODE(--fDrawState->fBlockEffectRemovalCnt;)
        fDrawState->fViewMatrix = fViewMatrix;
        SkASSERT(fDrawState->numColorStages() >= fNumColorStages);
        int numCoverageStages = fSavedCoordChanges.count() - fNumColorStages;
        SkASSERT(fDrawState->numCoverageStages() >= numCoverageStages);

        int i = 0;
        if (fHasGeometryProcessor) {
            SkASSERT(fDrawState->hasGeometryProcessor());
            fDrawState->fGeometryProcessor->restoreCoordChange(fSavedCoordChanges[i++]);
        }
        for (int s = 0; s < fNumColorStages; ++s, ++i) {
            fDrawState->fColorStages[s].restoreCoordChange(fSavedCoordChanges[i]);
        }
        for (int s = 0; s < numCoverageStages; ++s, ++i) {
            fDrawState->fCoverageStages[s].restoreCoordChange(fSavedCoordChanges[i]);
        }
        fDrawState->invalidateOptState();
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
    drawState->invalidateOptState();
}

bool GrDrawState::AutoViewMatrixRestore::setIdentity(GrDrawState* drawState) {
    this->restore();

    if (NULL == drawState) {
        return false;
    }

    if (drawState->getViewMatrix().isIdentity()) {
        return true;
    }

    drawState->invalidateOptState();
    fViewMatrix = drawState->getViewMatrix();
    if (0 == drawState->numTotalStages()) {
        drawState->fViewMatrix.reset();
        fDrawState = drawState;
        fHasGeometryProcessor = false;
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

    fHasGeometryProcessor = false;
    if (fDrawState->hasGeometryProcessor()) {
        fDrawState->fGeometryProcessor->saveCoordChange(&fSavedCoordChanges[i++]);
        fDrawState->fGeometryProcessor->localCoordChange(coordChangeMatrix);
        fHasGeometryProcessor = true;
    }

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

////////////////////////////////////////////////////////////////////////////////

void GrDrawState::invalidateOptState() const {
    SkSafeSetNull(fCachedOptState);
}

////////////////////////////////////////////////////////////////////////////////

GrDrawState::~GrDrawState() {
    SkSafeUnref(fCachedOptState);
    SkASSERT(0 == fBlockEffectRemovalCnt);
}

