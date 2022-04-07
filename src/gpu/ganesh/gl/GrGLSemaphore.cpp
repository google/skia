/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/gl/GrGLSemaphore.h"

#include "src/gpu/ganesh/gl/GrGLGpu.h"

GrGLSemaphore::GrGLSemaphore(GrGLGpu* gpu, bool isOwned)
        : fGpu(gpu), fSync(nullptr), fIsOwned(isOwned) {
}

GrGLSemaphore::~GrGLSemaphore() {
    if (fSync && fIsOwned) {
        fGpu->deleteSync(fSync);
    }
}
