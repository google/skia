/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrContext.h"
#include "GrGpu.h"
#include "GrSemaphore.h"
#include "gl/GrGLTypes.h"

GrGLExternalTextureData::GrGLExternalTextureData(const GrGLTextureInfo& info,
                                                 sk_sp<GrSemaphore> semaphore)
        : fSemaphore(std::move(semaphore))
        , fInfo(info) {
    fSemaphore->resetGpu(nullptr);
}

void GrGLExternalTextureData::attachToContext(GrContext* context) {
    fSemaphore->resetGpu(context->getGpu());
    context->getGpu()->waitSemaphore(fSemaphore);
}
