/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrExternalTextureData.h"
#include "GrSemaphore.h"

GrExternalTextureData::GrExternalTextureData(sk_sp<GrSemaphore> semaphore)
        : fSemaphore(std::move(semaphore)) {
    // We're externalizing this texture, so detach the semaphore from the GPU. We'll re-attach it
    // when we import the texture on another context.
    fSemaphore->resetGpu(nullptr);
}

GrExternalTextureData::~GrExternalTextureData() {}

sk_sp<GrSemaphore> GrExternalTextureData::getSemaphoreRef() const {
    return fSemaphore;
}
