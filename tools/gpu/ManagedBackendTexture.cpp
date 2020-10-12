/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/gpu/ManagedBackendTexture.h"

namespace sk_gpu_test {

void ManagedBackendTexture::ReleaseProc(void* context) {
    static_cast<ManagedBackendTexture*>(context)->unref();
}

ManagedBackendTexture::~ManagedBackendTexture() {
    if (fDContext && fTexture.isValid()) {
        fDContext->deleteBackendTexture(fTexture);
    }
}

void* ManagedBackendTexture::releaseContext() {
    this->ref();
    return static_cast<void*>(this);
}

}  // namespace sk_gpu_test
