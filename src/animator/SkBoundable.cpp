
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkBoundable.h"
#include "SkAnimateMaker.h"
#include "SkCanvas.h"

SkBoundable::SkBoundable() {
    clearBounds();
    fBounds.fTop = 0;
    fBounds.fRight = 0;
    fBounds.fBottom = 0;
}

void SkBoundable::clearBounder() {
    fBounds.fLeft = 0x7fff;
}

void SkBoundable::getBounds(SkRect* rect) {
    SkASSERT(rect);
    if (fBounds.fLeft == (int16_t)0x8000U) {
        INHERITED::getBounds(rect);
        return;
    }
    rect->fLeft = SkIntToScalar(fBounds.fLeft);
    rect->fTop = SkIntToScalar(fBounds.fTop);
    rect->fRight = SkIntToScalar(fBounds.fRight);
    rect->fBottom = SkIntToScalar(fBounds.fBottom);
}

void SkBoundable::enableBounder() {
    fBounds.fLeft = 0;
}


SkBoundableAuto::SkBoundableAuto(SkBoundable* boundable, 
        SkAnimateMaker& maker) : fBoundable(boundable), fMaker(maker) {
    if (fBoundable->hasBounds()) {
        fMaker.fCanvas->setBounder(&maker.fDisplayList);
        fMaker.fDisplayList.fBounds.setEmpty();
    }
}

SkBoundableAuto::~SkBoundableAuto() {
    if (fBoundable->hasBounds() == false)
        return;
    fMaker.fCanvas->setBounder(NULL);
    fBoundable->setBounds(fMaker.fDisplayList.fBounds);
}

