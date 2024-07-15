/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/BackendSemaphore.h"

#include "src/gpu/graphite/BackendSemaphorePriv.h"

namespace skgpu::graphite {

BackendSemaphore::BackendSemaphore() = default;

BackendSemaphore::~BackendSemaphore() = default;

BackendSemaphore::BackendSemaphore(const BackendSemaphore& that) {
    *this = that;
}

BackendSemaphore& BackendSemaphore::operator=(const BackendSemaphore& that) {
    if (!that.isValid()) {
        fIsValid = false;
        return *this;
    }
    SkASSERT(!this->isValid() || this->backend() == that.backend());
    fIsValid = true;
    fBackend = that.fBackend;

    switch (that.backend()) {
        case BackendApi::kDawn:
            SK_ABORT("Unsupported Backend");
        case BackendApi::kMetal:
            fSemaphoreData.reset();
            that.fSemaphoreData->copyTo(fSemaphoreData);
            break;
#ifdef SK_VULKAN
        case BackendApi::kVulkan:
            fVkSemaphore = that.fVkSemaphore;
            break;
#endif
        default:
            SK_ABORT("Unsupported Backend");
    }
    return *this;
}

#ifdef SK_VULKAN
BackendSemaphore::BackendSemaphore(VkSemaphore semaphore)
        : fBackend(BackendApi::kVulkan), fIsValid(true), fVkSemaphore(semaphore) {}

VkSemaphore BackendSemaphore::getVkSemaphore() const {
    if (this->isValid() && this->backend() == BackendApi::kVulkan) {
        return fVkSemaphore;
    }
    return VK_NULL_HANDLE;
}
#endif

BackendSemaphoreData::~BackendSemaphoreData(){};

}  // End of namespace skgpu::graphite
