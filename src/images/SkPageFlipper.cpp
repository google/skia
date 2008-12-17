/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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


