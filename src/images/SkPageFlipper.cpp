
/*
 * Copyright 2008 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkPageFlipper.h"

SkPageFlipper::SkPageFlipper() {
    fWidth = 0;
    fHeight = 0;
    fDirty0 = &fDirty0Storage;
    fDirty1 = &fDirty1Storage;

    fDirty0->setEmpty();
    fDirty1->setEmpty();
}

SkPageFlipper::SkPageFlipper(int width, int height) {
    fWidth = width;
    fHeight = height;
    fDirty0 = &fDirty0Storage;
    fDirty1 = &fDirty1Storage;

    fDirty0->setRect(0, 0, width, height);
    fDirty1->setEmpty();
}

void SkPageFlipper::resize(int width, int height) {
    fWidth = width;
    fHeight = height;

    // this is the opposite of the constructors
    fDirty1->setRect(0, 0, width, height);
    fDirty0->setEmpty();
}

void SkPageFlipper::inval() {
    fDirty1->setRect(0, 0, fWidth, fHeight);
}

void SkPageFlipper::inval(const SkIRect& rect) {
    SkIRect r;
    r.set(0, 0, fWidth, fHeight);
    if (r.intersect(rect)) {
        fDirty1->op(r, SkRegion::kUnion_Op);
    }
}

void SkPageFlipper::inval(const SkRegion& rgn) {
    SkRegion r;
    r.setRect(0, 0, fWidth, fHeight);
    if (r.op(rgn, SkRegion::kIntersect_Op)) {
        fDirty1->op(r, SkRegion::kUnion_Op);
    }
}

void SkPageFlipper::inval(const SkRect& rect, bool antialias) {
    SkIRect r;
    rect.round(&r);
    if (antialias) {
        r.inset(-1, -1);
    }
    this->inval(r);
}

const SkRegion& SkPageFlipper::update(SkRegion* copyBits) {
    // Copy over anything new from page0 that isn't dirty in page1
    copyBits->op(*fDirty0, *fDirty1, SkRegion::kDifference_Op);
    SkTSwap<SkRegion*>(fDirty0, fDirty1);
    fDirty1->setEmpty();
    return *fDirty0;
}
