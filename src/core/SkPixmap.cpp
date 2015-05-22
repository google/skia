/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPixmap.h"

void SkAutoPixmapUnlock::reset(const SkPixmap& pm, void (*unlock)(void*), void* ctx) {
    SkASSERT(pm.addr() != NULL);

    this->unlock();
    fPixmap = pm;
    fUnlockProc = unlock;
    fUnlockContext = ctx;
    fIsLocked = true;
}
