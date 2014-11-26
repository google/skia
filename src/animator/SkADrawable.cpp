
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkADrawable.h"

bool SkADrawable::doEvent(SkDisplayEvent::Kind , SkEventState* ) {
    return false;
}

bool SkADrawable::isDrawable() const {
    return true;
}

void SkADrawable::initialize() {
}

void SkADrawable::setSteps(int steps) {
}
