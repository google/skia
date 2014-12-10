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
#include "GrXferProcessor.h"
#include "effects/GrPorterDuffXferProcessor.h"

bool GrDrawState::isEqual(const GrDrawState& that) const {
    if (this->getRenderTarget() != that.getRenderTarget() ||
        this->fColorStages.count() != that.fColorStages.count() ||
        this->fCoverageStages.count() != that.fCoverageStages.count() ||
        !this->fViewMatrix.cheapEqualTo(that.fViewMatrix) ||
        this->fFlagBits != that.fFlagBits ||
        this->fStencilSettings != that.fStencilSettings ||
        this->fDrawFace != that.fDrawFace) {
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

    if (!this->getXPFactory()->isEqual(*that.getXPFactory())) {
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
    fViewMatrix = that.fViewMatrix;
    fFlagBits = that.fFlagBits;
    fStencilSettings = that.fStencilSettings;
    fDrawFace = that.fDrawFace;
    fGeometryProcessor.reset(SkSafeRef(that.fGeometryProcessor.get()));
    fXPFactory.reset(SkRef(that.getXPFactory()));
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
    fXPFactory.reset(GrPorterDuffXPFactory::Create(SkXfermode::kSrc_Mode));
    fColorStages.reset();
    fCoverageStages.reset();

    if (NULL == initialViewMatrix) {
        fViewMatrix.reset();
    } else {
        fViewMatrix = *initialViewMatrix;
    }
    fFlagBits = 0x0;
    fStencilSettings.setDisabled();
    fDrawFace = kBoth_DrawFace;

    fHints = 0;

    fColorProcInfoValid = false;
    fCoverageProcInfoValid = false;

    fColorCache = GrColor_ILLEGAL;
    fCoverageCache = GrColor_ILLEGAL;
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

    fXPFactory.reset(SkRef(paint.getXPFactory()));

    this->setRenderTarget(rt);

    fViewMatrix = vm;

    // These have no equivalent in GrPaint, set them to defaults
    fDrawFace = kBoth_DrawFace;
    fStencilSettings.setDisabled();
    fFlagBits = 0;
    fHints = 0;

    // Enable the clip bit
    this->enableState(GrDrawState::kClip_StateBit);

    this->setState(GrDrawState::kDither_StateBit, paint.isDither());
    this->setState(GrDrawState::kHWAntialias_StateBit, paint.isAntiAlias());

    fColorProcInfoValid = false;
    fCoverageProcInfoValid = false;

    fColorCache = GrColor_ILLEGAL;
    fCoverageCache = GrColor_ILLEGAL;
}

////////////////////////////////////////////////////////////////////////////////

bool GrDrawState::canUseFracCoveragePrimProc(GrColor color, const GrDrawTargetCaps& caps) const {
    if (caps.dualSourceBlendingSupport()) {
        return true;
    }

    this->calcColorInvariantOutput(color);

    // The coverage isn't actually white, its unknown, but this will produce the same effect
    // TODO we want to cache the result of this call, but we can probably clean up the interface
    // so we don't have to pass in a seemingly known coverage
    this->calcCoverageInvariantOutput(GrColor_WHITE);
    return fXPFactory->canApplyCoverage(fColorProcInfo, fCoverageProcInfo,
                                        this->isCoverageDrawing(), this->isColorWriteDisabled());
}

bool GrDrawState::hasSolidCoverage(GrColor coverage) const {
    // If we're drawing coverage directly then coverage is effectively treated as color.
    if (this->isCoverageDrawing()) {
        return true;
    }

    if (this->numCoverageStages() > 0) {
        return false;
    }

    this->calcCoverageInvariantOutput(coverage);
    return fCoverageProcInfo.isSolidWhite();
}

//////////////////////////////////////////////////////////////////////////////s

bool GrDrawState::willEffectReadDstColor(GrColor color, GrColor coverage) const {
    this->calcColorInvariantOutput(color);
    this->calcCoverageInvariantOutput(coverage);
    // TODO: Remove need to create the XP here.
    //       Also once all custom blends are turned into XPs we can remove the need
    //       to check other stages since only xp's will be able to read dst
    SkAutoTUnref<GrXferProcessor> xferProcessor(fXPFactory->createXferProcessor(fColorProcInfo,
                                                                                fCoverageProcInfo));
    if (xferProcessor && xferProcessor->willReadDstColor()) {
        return true;
    }

    if (!this->isColorWriteDisabled()) {
        if (fColorProcInfo.readsDst()) {
            return true;
        }
    }
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
    return fXPFactory->canTweakAlphaForCoverage(this->isCoverageDrawing());
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

bool GrDrawState::srcAlphaWillBeOne(GrColor color, GrColor coverage) const {
    this->calcColorInvariantOutput(color);
    if (this->isCoverageDrawing()) {
        this->calcCoverageInvariantOutput(coverage);
        return (fColorProcInfo.isOpaque() && fCoverageProcInfo.isOpaque());
    }
    return fColorProcInfo.isOpaque();
}

bool GrDrawState::willBlendWithDst(GrColor color, GrColor coverage) const {
    this->calcColorInvariantOutput(color);
    this->calcCoverageInvariantOutput(coverage);
    return fXPFactory->willBlendWithDst(fColorProcInfo, fCoverageProcInfo,
                                        this->isCoverageDrawing(), this->isColorWriteDisabled());
}

void GrDrawState::calcColorInvariantOutput(GrColor color) const {
    if (!fColorProcInfoValid || color != fColorCache) {
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
        }
        fColorProcInfo.calcWithInitialValues(fColorStages.begin(), this->numColorStages(),
                                             color, flags, false);
        fColorProcInfoValid = true;
        fColorCache = color;
    }
}

void GrDrawState::calcCoverageInvariantOutput(GrColor coverage) const {
    if (!fCoverageProcInfoValid || coverage != fCoverageCache) {
        GrColorComponentFlags flags;
        // Check if per-vertex or constant color may have partial alpha
        if (this->hasCoverageVertexAttribute()) {
            flags = static_cast<GrColorComponentFlags>(0);
            coverage = 0;
        } else {
            flags = kRGBA_GrColorComponentFlags;
        }
        fCoverageProcInfo.calcWithInitialValues(fCoverageStages.begin(), this->numCoverageStages(),
                                                coverage, flags, true, fGeometryProcessor.get());
        fCoverageProcInfoValid = true;
        fCoverageCache = coverage;
    }
}
