/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/BackendSemaphore.h"

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
#ifdef SK_DAWN
        case BackendApi::kDawn:
            SK_ABORT("Unsupported Backend");
#endif
 #ifdef SK_METAL
        case BackendApi::kMetal:
            fMtlEvent = that.fMtlEvent;
            fMtlValue = that.fMtlValue;
            break;
#endif
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

#ifdef SK_METAL
BackendSemaphore::BackendSemaphore(CFTypeRef mtlEvent, uint64_t value)
        : fMtlEvent(mtlEvent)
        , fMtlValue(value) {}

CFTypeRef BackendSemaphore::getMtlEvent() const {
    if (this->isValid() && this->backend() == BackendApi::kMetal) {
        return fMtlEvent;
    }
    return nullptr;
}

uint64_t BackendSemaphore::getMtlValue() const {
    if (this->isValid() && this->backend() == BackendApi::kMetal) {
        return fMtlValue;
    }
    return 0;
}
#endif // SK_METAL

#ifdef SK_VULKAN
BackendSemaphore::BackendSemaphore(VkSemaphore semaphore)
        : fVkSemaphore(semaphore)
        , fIsValid(true)
        , fBackend(BackendApi::kVulkan) {}

VkSemaphore BackendSemaphore::getVkSemaphore() const {
    if (this->isValid() && this->backend() == BackendApi::kVulkan) {
        return fVkSemaphore;
    }
    return VK_NULL_HANDLE;
}
#endif

}  // End of namespace skgpu::graphite
