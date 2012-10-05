/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDrawState.h"

#include "GrPaint.h"

void GrDrawState::setFromPaint(const GrPaint& paint) {
    for (int i = 0; i < GrPaint::kMaxTextures; ++i) {
        int s = i + GrPaint::kFirstTextureStage;
        if (paint.isTextureStageEnabled(i)) {
            *this->sampler(s) = paint.getTextureSampler(i);
        }
    }

    this->setFirstCoverageStage(GrPaint::kFirstMaskStage);

    for (int i = 0; i < GrPaint::kMaxMasks; ++i) {
        int s = i + GrPaint::kFirstMaskStage;
        if (paint.isMaskStageEnabled(i)) {
            *this->sampler(s) = paint.getMaskSampler(i);
        }
    }

    // disable all stages not accessible via the paint
    for (int s = GrPaint::kTotalStages; s < GrDrawState::kNumStages; ++s) {
        this->disableStage(s);
    }

    this->setColor(paint.fColor);

    this->setState(GrDrawState::kDither_StateBit, paint.fDither);
    this->setState(GrDrawState::kHWAntialias_StateBit, paint.fAntiAlias);

    if (paint.fColorMatrixEnabled) {
        this->enableState(GrDrawState::kColorMatrix_StateBit);
        this->setColorMatrix(paint.fColorMatrix);
    } else {
        this->disableState(GrDrawState::kColorMatrix_StateBit);
    }
    this->setBlendFunc(paint.fSrcBlendCoeff, paint.fDstBlendCoeff);
    this->setColorFilter(paint.fColorFilterColor, paint.fColorFilterXfermode);
    this->setCoverage(paint.fCoverage);
}
