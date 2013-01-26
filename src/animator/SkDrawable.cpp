
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkDrawable.h"

bool SkDrawable::doEvent(SkDisplayEvent::Kind , SkEventState* ) {
    return false;
}

bool SkDrawable::isDrawable() const {
    return true;
}

void SkDrawable::initialize() {
}

void SkDrawable::setSteps(int steps) {
}
