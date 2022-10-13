/*
 * Copyright 2022 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_MutableTextureStateRef_DEFINED
#define skgpu_MutableTextureStateRef_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/MutableTextureState.h"

namespace skgpu {

class MutableTextureStateRef : public SkRefCnt {
public:
#ifdef SK_VULKAN
    MutableTextureStateRef(VkImageLayout layout, uint32_t queueFamilyIndex)
            : fState(layout, queueFamilyIndex) {}
#endif

    void set(const MutableTextureState& state) { fState = state; }

#ifdef SK_VULKAN
    VkImageLayout getImageLayout() const {
        SkASSERT(fState.fBackend == BackendApi::kVulkan);
        return fState.fVkState.getImageLayout();
    }

    void setImageLayout(VkImageLayout layout) {
        SkASSERT(fState.fBackend == BackendApi::kVulkan);
        fState.fVkState.setImageLayout(layout);
    }

    uint32_t getQueueFamilyIndex() const {
        SkASSERT(fState.fBackend == BackendApi::kVulkan);
        return fState.fVkState.getQueueFamilyIndex();
    }

    void setQueueFamilyIndex(uint32_t queueFamilyIndex) {
        SkASSERT(fState.fBackend == BackendApi::kVulkan);
        fState.fVkState.setQueueFamilyIndex(queueFamilyIndex);
    }

    const VulkanMutableTextureState& getVkMutableTextureState() {
        SkASSERT(fState.fBackend == BackendApi::kVulkan);
        return fState.fVkState;
    }
#endif


private:
    MutableTextureState fState;
};

} // namespace skgpu

#endif // skgpu_MutableTextureStateRef_DEFINED

