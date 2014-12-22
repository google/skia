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

bool GrDrawState::isEqual(const GrDrawState& that, bool explicitLocalCoords) const {
    if (this->getRenderTarget() != that.getRenderTarget() ||
        this->fColorStages.count() != that.fColorStages.count() ||
        this->fCoverageStages.count() != that.fCoverageStages.count() ||
        !this->fViewMatrix.cheapEqualTo(that.fViewMatrix) ||
        this->fFlagBits != that.fFlagBits ||
        this->fStencilSettings != that.fStencilSettings ||
        this->fDrawFace != that.fDrawFace) {
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
    fXPFactory.reset(SkRef(that.getXPFactory()));
    fColorStages = that.fColorStages;
    fCoverageStages = that.fCoverageStages;

    fColorProcInfoValid = that.fColorProcInfoValid;
    fCoverageProcInfoValid = that.fCoverageProcInfoValid;
    fColorCache = that.fColorCache;
    fCoverageCache = that.fCoverageCache;
    fColorPrimProc = that.fColorPrimProc;
    fCoveragePrimProc = that.fCoveragePrimProc;
    if (fColorProcInfoValid) {
        fColorProcInfo = that.fColorProcInfo;
    }
    if (fCoverageProcInfoValid) {
        fCoverageProcInfo = that.fCoverageProcInfo;
    }
    return *this;
}

void GrDrawState::onReset(const SkMatrix* initialViewMatrix) {
    SkASSERT(0 == fBlockEffectRemovalCnt || 0 == this->numFragmentStages());
    fRenderTarget.reset(NULL);

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

    fColorProcInfoValid = false;
    fCoverageProcInfoValid = false;

    fColorCache = GrColor_ILLEGAL;
    fCoverageCache = GrColor_ILLEGAL;

    fColorPrimProc = NULL;
    fCoveragePrimProc = NULL;
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
    SkASSERT(0 == fBlockEffectRemovalCnt || 0 == this->numFragmentStages());

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

    // Enable the clip bit
    this->enableState(GrDrawState::kClip_StateBit);

    this->setState(GrDrawState::kDither_StateBit, paint.isDither());
    this->setState(GrDrawState::kHWAntialias_StateBit, paint.isAntiAlias());

    fColorProcInfoValid = false;
    fCoverageProcInfoValid = false;

    fColorCache = GrColor_ILLEGAL;
    fCoverageCache = GrColor_ILLEGAL;

    fColorPrimProc = NULL;
    fCoveragePrimProc = NULL;
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
    return fXPFactory->canApplyCoverage(fColorProcInfo, fCoverageProcInfo);
}

//////////////////////////////////////////////////////////////////////////////s

bool GrDrawState::willEffectReadDstColor(const GrPrimitiveProcessor* pp) const {
    this->calcColorInvariantOutput(pp);
    this->calcCoverageInvariantOutput(pp);

    return fXPFactory->willReadDst(fColorProcInfo, fCoverageProcInfo);
}

void GrDrawState::AutoRestoreEffects::set(GrDrawState* ds) {
    if (fDrawState) {
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
        fColorEffectCnt = ds->numColorStages();
        fCoverageEffectCnt = ds->numCoverageStages();
        SkDEBUGCODE(++ds->fBlockEffectRemovalCnt;)
    }
}

////////////////////////////////////////////////////////////////////////////////

// Some blend modes allow folding a fractional coverage value into the color's alpha channel, while
// others will blend incorrectly.
bool GrDrawState::canTweakAlphaForCoverage() const {
    return fXPFactory->canTweakAlphaForCoverage();
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

bool GrDrawState::willBlendWithDst(const GrPrimitiveProcessor* pp) const {
    this->calcColorInvariantOutput(pp);
    this->calcCoverageInvariantOutput(pp);
    
    GrXPFactory::InvariantOutput output;
    fXPFactory->getInvariantOutput(fColorProcInfo, fCoverageProcInfo, &output);
    return output.fWillBlendWithDst;
}

void GrDrawState::calcColorInvariantOutput(const GrPrimitiveProcessor* pp) const {
    if (!fColorProcInfoValid || fColorPrimProc != pp) {
        fColorProcInfo.calcColorWithPrimProc(pp, fColorStages.begin(), this->numColorStages());
        fColorProcInfoValid = true;
        fColorPrimProc = pp;
    }
}

void GrDrawState::calcCoverageInvariantOutput(const GrPrimitiveProcessor* pp) const {
    if (!fCoverageProcInfoValid ||  fCoveragePrimProc != pp) {
        fCoverageProcInfo.calcCoverageWithPrimProc(pp, fCoverageStages.begin(),
                                                   this->numCoverageStages());
        fCoverageProcInfoValid = true;
        fCoveragePrimProc = pp;
    }
}

void GrDrawState::calcColorInvariantOutput(GrColor color) const {
    if (!fColorProcInfoValid || color != fColorCache) {
        GrColorComponentFlags flags = kRGBA_GrColorComponentFlags;
        fColorProcInfo.calcWithInitialValues(fColorStages.begin(), this->numColorStages(), color,
                                             flags, false);
        fColorProcInfoValid = true;
        fColorCache = color;
    }
}

void GrDrawState::calcCoverageInvariantOutput(GrColor coverage) const {
    if (!fCoverageProcInfoValid || coverage != fCoverageCache) {
        GrColorComponentFlags flags = kRGBA_GrColorComponentFlags;
        fCoverageProcInfo.calcWithInitialValues(fCoverageStages.begin(),
                                                this->numCoverageStages(), coverage, flags,
                                                true);
        fCoverageProcInfoValid = true;
        fCoverageCache = coverage;
    }
}
