/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#include "VisualSKPBench.h"

VisualSKPBench::VisualSKPBench(const char* name, const SkPicture* pic)
    : fPic(SkRef(pic))
    , fName(name) {
    fUniqueName.printf("%s", name);
}

const char* VisualSKPBench::onGetName() {
    return fName.c_str();
}

const char* VisualSKPBench::onGetUniqueName() {
    return fUniqueName.c_str();
}

bool VisualSKPBench::isSuitableFor(Backend backend) {
    return backend != kNonRendering_Backend;
}

void VisualSKPBench::onDraw(const int loops, SkCanvas* canvas) {
    for (int i = 0; i < loops; i++) {
        canvas->drawPicture(fPic);
    }
}
