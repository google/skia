/*
 * Copyright 2023 Google, LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/gpu/MutableTextureState.h"

namespace skgpu {
#if defined(SK_VULKAN)
MutableTextureState::MutableTextureState(VkImageLayout layout, uint32_t queueFamilyIndex)
            : fVkState(layout, queueFamilyIndex)
            , fBackend(BackendApi::kVulkan)
            , fIsValid(true) {}
#endif

MutableTextureState::MutableTextureState(const MutableTextureState& that) {
    this->set(that);
}

MutableTextureState& MutableTextureState::operator=(const MutableTextureState& that) {
    if (this != &that) {
        this->set(that);
    }
    return *this;
}

void MutableTextureState::set(const MutableTextureState& that) {
    SkASSERT(!fIsValid || this->fBackend == that.fBackend);
    fIsValid = that.fIsValid;
    fBackend = that.fBackend;
    if (!fIsValid) {
        return;
    }
    switch (fBackend) {
        case BackendApi::kVulkan:
#ifdef SK_VULKAN
            fVkState = that.fVkState;
#endif
            break;
        default:
            SK_ABORT("Unknown BackendApi");
    }
}
}  // namespace skgpu
