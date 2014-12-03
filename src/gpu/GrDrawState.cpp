/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDrawState.h"

#include "GrBlend.h"
#include "GrOptDrawState.h"
#include "GrPaint.h"
#include "GrProcOptInfo.h"

//////////////////////////////////////////////////////////////////////////////s

bool GrDrawState::isEqual(const GrDrawState& that) const {
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
        this->fStencilSettings != that.fStencilSettings ||
        this->fDrawFace != that.fDrawFace) {
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
        } else if (!this->getGeometryProcessor()->isEqual(*that.getGeometryProcessor())) {
            return false;
        }
    } else if (that.hasGeometryProcessor()) {
        return false;
    }

    for (int i = 0; i < this->numColorStages(); i++) {
        if (!GrFragmentStage::AreCompatible(this->getColorStage(i), that.getColorStage(i),
                                             explicitLocalCoords)) {
            return false;
        }
    }
    for (int i = 0; i < this->numCoverageStages(); i++) {
        if (!GrFragmentStage::AreCompatible(this->getCoverageStage(i), that.getCoverageStage(i),
                                             explicitLocalCoords)) {
            return false;
        }
    }

    return true;
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
    }
}

GrDrawState& GrDrawState::operator=(const GrDrawState& that) {
    fRenderTarget.reset(SkSafeRef(that.fRenderTarget.get()));
    fColor = that.fColor;
    fViewMatrix = that.fViewMatrix;
    fSrcBlend = that.fSrcBlend;
    fDstBlend = that.fDstBlend;
    fBlendConstant = that.fBlendConstant;
    fFlagBits = that.fFlagBits;
    fStencilSettings = that.fStencilSettings;
    fCoverage = that.fCoverage;
    fDrawFace = that.fDrawFace;
    fGeometryProcessor.reset(SkSafeRef(that.fGeometryProcessor.get()));
    fColorStages = that.fColorStages;
    fCoverageStages = that.fCoverageStages;

    fHints = that.fHints;

    fColorProcInfoValid = that.fColorProcInfoValid;
    fCoverageProcInfoValid = that.fCoverageProcInfoValid;
    if (fColorProcInfoValid) {
        fColorProcInfo = that.fColorProcInfo;
    }
    if (fCoverageProcInfoValid) {
        fCoverageProcInfo = that.fCoverageProcInfo;
    }
    return *this;
}

void GrDrawState::onReset(const SkMatrix* initialViewMatrix) {
    SkASSERT(0 == fBlockEffectRemovalCnt || 0 == this->numTotalStages());
    fRenderTarget.reset(NULL);

    fGeometryProcessor.reset(NULL);
    fColorStages.reset();
    fCoverageStages.reset();

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

    fColorProcInfoValid = false;
    fCoverageProcInfoValid = false;
}

bool GrDrawState::setIdentityViewMatrix()  {
    if (this->numFragmentStages()) {
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
    fFlagBits = 0;
    fHints = 0;

    // Enable the clip bit
    this->enableState(GrDrawState::kClip_StateBit);

    this->setColor(paint.getColor());
    this->setState(GrDrawState::kDither_StateBit, paint.isDither());
    this->setState(GrDrawState::kHWAntialias_StateBit, paint.isAntiAlias());

    this->setBlendFunc(paint.getSrcBlendCoeff(), paint.getDstBlendCoeff());
    this->setCoverage(0xFF);
    fColorProcInfoValid = false;
    fCoverageProcInfoValid = false;
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
    BlendOpt opt = this->getBlendOpt(true, &srcCoeff, &dstCoeff);
    return GrDrawState::kNone_BlendOpt != opt ||
           (this->willEffectReadDstColor() &&
            kOne_GrBlendCoeff == srcCoeff && kZero_GrBlendCoeff == dstCoeff);
}

bool GrDrawState::hasSolidCoverage() const {
    // If we're drawing coverage directly then coverage is effectively treated as color.
    if (this->isCoverageDrawing()) {
        return true;
    }

    if (this->numCoverageStages() > 0) {
        return false;
    }

    this->calcCoverageInvariantOutput();
    return fCoverageProcInfo.isSolidWhite();
}

//////////////////////////////////////////////////////////////////////////////s

bool GrDrawState::willEffectReadDstColor() const {
    if (!this->isColorWriteDisabled()) {
        this->calcColorInvariantOutput();
        if (fColorProcInfo.readsDst()) {
            return true;
        }
    }
    this->calcCoverageInvariantOutput();
    return fCoverageProcInfo.readsDst();
}

void GrDrawState::AutoRestoreEffects::set(GrDrawState* ds) {
    if (fDrawState) {
        // See the big comment on the class definition about GPs.
        if (SK_InvalidUniqueID == fOriginalGPID) {
            fDrawState->fGeometryProcessor.reset(NULL);
        } else {
            SkASSERT(fDrawState->getGeometryProcessor()->getUniqueID() ==
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
            fDrawState->fColorProcInfoValid = false;
            fDrawState->fCoverageProcInfoValid = false;
        }
        SkDEBUGCODE(--fDrawState->fBlockEffectRemovalCnt;)
    }
    fDrawState = ds;
    if (NULL != ds) {
        SkASSERT(SK_InvalidUniqueID == fOriginalGPID);
        if (NULL != ds->getGeometryProcessor()) {
            fOriginalGPID = ds->getGeometryProcessor()->getUniqueID();
        }
        fColorEffectCnt = ds->numColorStages();
        fCoverageEffectCnt = ds->numCoverageStages();
        SkDEBUGCODE(++ds->fBlockEffectRemovalCnt;)
    }
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
    return kOne_GrBlendCoeff == fDstBlend ||
           kISA_GrBlendCoeff == fDstBlend ||
           kISC_GrBlendCoeff == fDstBlend ||
           this->isCoverageDrawing();
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
    if (0 == drawState->numFragmentStages()) {
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
    fSavedCoordChanges.reset(fDrawState->numFragmentStages());
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

////////////////////////////////////////////////////////////////////////////////

GrDrawState::~GrDrawState() {
    SkASSERT(0 == fBlockEffectRemovalCnt);
}

////////////////////////////////////////////////////////////////////////////////

GrDrawState::BlendOpt GrDrawState::getBlendOpt(bool forceCoverage,
                                               GrBlendCoeff* srcCoeff,
                                               GrBlendCoeff* dstCoeff) const {
    GrBlendCoeff bogusSrcCoeff, bogusDstCoeff;
    if (NULL == srcCoeff) {
        srcCoeff = &bogusSrcCoeff;
    }
    if (NULL == dstCoeff) {
        dstCoeff = &bogusDstCoeff;
    }

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
            return kEmitCoverage_BlendOpt;
        } else {
            *dstCoeff = kOne_GrBlendCoeff;
            return kSkipDraw_BlendOpt;
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
                return kEmitTransBlack_BlendOpt;
            }
        }
    } else if (this->isCoverageDrawing()) {
        // we have coverage but we aren't distinguishing it from alpha by request.
        return kCoverageAsAlpha_BlendOpt;
    } else {
        // check whether coverage can be safely rolled into alpha
        // of if we can skip color computation and just emit coverage
        if (this->canTweakAlphaForCoverage()) {
            return kCoverageAsAlpha_BlendOpt;
        }
        if (dstCoeffIsZero) {
            if (kZero_GrBlendCoeff == *srcCoeff) {
                // the source color is not included in the blend
                // the dst coeff is effectively zero so blend works out to:
                // (c)(0)D + (1-c)D = (1-c)D.
                *dstCoeff = kISA_GrBlendCoeff;
                return  kEmitCoverage_BlendOpt;
            } else if (srcAIsOne) {
                // the dst coeff is effectively zero so blend works out to:
                // cS + (c)(0)D + (1-c)D = cS + (1-c)D.
                // If Sa is 1 then we can replace Sa with c
                // and set dst coeff to 1-Sa.
                *dstCoeff = kISA_GrBlendCoeff;
                return  kCoverageAsAlpha_BlendOpt;
            }
        } else if (dstCoeffIsOne) {
            // the dst coeff is effectively one so blend works out to:
            // cS + (c)(1)D + (1-c)D = cS + D.
            *dstCoeff = kOne_GrBlendCoeff;
            return  kCoverageAsAlpha_BlendOpt;
        }
    }

    return kNone_BlendOpt;
}

bool GrDrawState::srcAlphaWillBeOne() const {
    this->calcColorInvariantOutput();
    if (this->isCoverageDrawing()) {
        this->calcCoverageInvariantOutput();
        return (fColorProcInfo.isOpaque() && fCoverageProcInfo.isOpaque());
    }
    return fColorProcInfo.isOpaque();
}

bool GrDrawState::willBlendWithDst() const {
    if (!this->hasSolidCoverage()) {
        return true;
    }

    if (this->willEffectReadDstColor()) {
        return true;
    }

    if (GrBlendCoeffRefsDst(this->getSrcBlendCoeff())) {
        return true;
    }

    GrBlendCoeff dstCoeff = this->getDstBlendCoeff();
    if (!(kZero_GrBlendCoeff == dstCoeff ||
         (kISA_GrBlendCoeff == dstCoeff && this->srcAlphaWillBeOne()))) {
        return true;
    }

    return false;
}

void GrDrawState::calcColorInvariantOutput() const {
    if (!fColorProcInfoValid) {
        GrColor color;
        GrColorComponentFlags flags;
        if (this->hasColorVertexAttribute()) {
            if (fHints & kVertexColorsAreOpaque_Hint) {
                flags = kA_GrColorComponentFlag;
                color = 0xFF << GrColor_SHIFT_A;
            } else {
                flags = static_cast<GrColorComponentFlags>(0);
                color = 0;
            }
        } else {
            flags = kRGBA_GrColorComponentFlags;
            color = this->getColor();
        }
        fColorProcInfo.calcWithInitialValues(fColorStages.begin(), this->numColorStages(),
                                             color, flags, false);
        fColorProcInfoValid = true;
    }
}

void GrDrawState::calcCoverageInvariantOutput() const {
    if (!fCoverageProcInfoValid) {
        GrColor color;
        GrColorComponentFlags flags;
        // Check if per-vertex or constant color may have partial alpha
        if (this->hasCoverageVertexAttribute()) {
            flags = static_cast<GrColorComponentFlags>(0);
            color = 0;
        } else {
            flags = kRGBA_GrColorComponentFlags;
            color = this->getCoverageColor();
        }
        fCoverageProcInfo.calcWithInitialValues(fCoverageStages.begin(), this->numCoverageStages(),
                                                color, flags, true, fGeometryProcessor.get());
        fCoverageProcInfoValid = true;
    }
}

