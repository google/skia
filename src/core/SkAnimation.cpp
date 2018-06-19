/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAnimation.h"

SkAnimation::SkAnimation()
    : fBones() {
}

SkAnimation::~SkAnimation() {
}

void SkAnimation::setBones(SkMatrix* bones, int numBones) {
    // Don't do anything if numBones is invalid.
    if (numBones < 0) {
        return;
    }

    // Limit the number of bones to 100.
    if (numBones > 100) {
        numBones = 100;
    }

    // Set the data.
    fBones.assign(bones, bones + numBones);
}
