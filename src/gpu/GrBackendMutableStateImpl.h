/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBackendMutableStateImpl_DEFINED
#define GrBackendMutableStateImpl_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/GrBackendMutableState.h"

class GrBackendMutableStateImpl : public GrBackendMutableState, public SkRefCnt {
public:
#ifdef SK_VULKAN
    GrBackendMutableStateImpl(VkImageLayout layout, uint32_t queueFamilyIndex)
            : GrBackendMutableState(layout, queueFamilyIndex) {}

    GrBackendMutableStateImpl(GrVkSharedImageInfo sharedInfo)
            : GrBackendMutableState(sharedInfo.getImageLayout(),
                                    sharedInfo.getQueueFamilyIndex()) {}

    VkImageLayout getImageLayout() const {
        SkASSERT(fBackend == GrBackend::kVulkan);
        return fVkState.getImageLayout();
    }

    void setImageLayout(VkImageLayout layout) {
        SkASSERT(fBackend == GrBackend::kVulkan);
        fVkState.setImageLayout(layout);
    }

    uint32_t getQueueFamilyIndex() const {
        SkASSERT(fBackend == GrBackend::kVulkan);
        return fVkState.getQueueFamilyIndex();
    }

    void setQueueFamilyIndex(uint32_t queueFamilyIndex) {
        SkASSERT(fBackend == GrBackend::kVulkan);
        fVkState.setQueueFamilyIndex(queueFamilyIndex);
    }

    const GrVkSharedImageInfo& getVkSharedImageInfo() {
        SkASSERT(fBackend == GrBackend::kVulkan);
        return fVkState;
    }
#endif
};

#endif
