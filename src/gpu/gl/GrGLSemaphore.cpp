/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLSemaphore.h"

#include "GrGLGpu.h"

GrGLSemaphore::GrGLSemaphore(const GrGLGpu* gpu, bool isOwned)
    : INHERITED(gpu), fSync(0), fIsOwned(isOwned) {
}

GrGLSemaphore::~GrGLSemaphore() {
    if (fIsOwned && fGpu) {
        static_cast<const GrGLGpu*>(fGpu)->deleteSync(fSync);
    }
}
