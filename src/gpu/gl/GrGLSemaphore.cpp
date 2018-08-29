/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLSemaphore.h"

#include "GrGLGpu.h"

GrGLSemaphore::GrGLSemaphore(GrGLGpu* gpu, bool isOwned)
        : INHERITED(gpu), fSync(0), fIsOwned(isOwned) {}

void GrGLSemaphore::onRelease() {
    if (fSync && fIsOwned) {
        static_cast<const GrGLGpu*>(this->getGpu())->deleteSync(fSync);
    }
    fSync = 0;
}

void GrGLSemaphore::onAbandon() { fSync = 0; }
