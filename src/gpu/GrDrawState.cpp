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
            *this->sampler(s) = paint.getColorSampler(i);
        }
    }

    this->setFirstCoverageStage(GrPaint::kFirstCoverageStage);

    for (int i = 0; i < GrPaint::kMaxCoverageStages; ++i) {
        int s = i + GrPaint::kFirstCoverageStage;
        if (paint.isCoverageStageEnabled(i)) {
            *this->sampler(s) = paint.getCoverageSampler(i);
        }
    }

    // disable all stages not accessible via the paint
    for (int s = GrPaint::kTotalStages; s < GrDrawState::kNumStages; ++s) {
        this->disableStage(s);
    }

    this->setColor(paint.getColor());

    this->setState(GrDrawState::kDither_StateBit, paint.isDither());
    this->setState(GrDrawState::kHWAntialias_StateBit, paint.isAntiAlias());

    if (paint.isColorMatrixEnabled()) {
        this->enableState(GrDrawState::kColorMatrix_StateBit);
        this->setColorMatrix(paint.getColorMatrix());
    } else {
        this->disableState(GrDrawState::kColorMatrix_StateBit);
    }
    this->setBlendFunc(paint.getSrcBlendCoeff(), paint.getDstBlendCoeff());
    this->setColorFilter(paint.getColorFilterColor(), paint.getColorFilterMode());
    this->setCoverage(paint.getCoverage());
}

////////////////////////////////////////////////////////////////////////////////

GrDrawState::AutoDeviceCoordDraw::AutoDeviceCoordDraw(GrDrawState* drawState,
                                                      uint32_t explicitCoordStageMask) {
    GrAssert(NULL != drawState);

    fDrawState = drawState;
    fViewMatrix = drawState->getViewMatrix();
    fRestoreMask = 0;
    GrMatrix invVM;
    bool inverted = false;

    for (int s = 0; s < GrDrawState::kNumStages; ++s) {
        if (!(explicitCoordStageMask & (1 << s)) && drawState->isStageEnabled(s)) {
            if (!inverted && !fViewMatrix.invert(&invVM)) {
                // sad trombone sound
                fDrawState = NULL;
                return;
            } else {
                inverted = true;
            }
            fRestoreMask |= (1 << s);
            GrSamplerState* sampler = drawState->sampler(s);
            fSamplerMatrices[s] = sampler->getMatrix();
            sampler->preConcatMatrix(invVM);
        }
    }
    drawState->viewMatrix()->reset();
}

GrDrawState::AutoDeviceCoordDraw::~AutoDeviceCoordDraw() {
    if (NULL != fDrawState) {
        fDrawState->setViewMatrix(fViewMatrix);
        for (int s = 0; s < GrDrawState::kNumStages; ++s) {
            if (fRestoreMask & (1 << s)) {
                *fDrawState->sampler(s)->matrix() = fSamplerMatrices[s];
            }
        }
    }
}
