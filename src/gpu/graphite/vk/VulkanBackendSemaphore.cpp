/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkString.h"
#include "include/gpu/graphite/BackendSemaphore.h"
#include "include/private/gpu/vk/SkiaVulkan.h"
#include "src/gpu/graphite/BackendSemaphorePriv.h"

#include <cstdint>

namespace skgpu::graphite {

class VulkanBackendSemaphoreData final : public BackendSemaphoreData {
public:
    VulkanBackendSemaphoreData(VkSemaphore sem) : fVkSemaphore(sem) {}

#if defined(SK_DEBUG)
    skgpu::BackendApi type() const override { return skgpu::BackendApi::kVulkan; }
#endif

    VkSemaphore semaphore() const { return fVkSemaphore; }

private:
    VkSemaphore fVkSemaphore;

    void copyTo(AnyBackendSemaphoreData& dstData) const override {
        // Don't assert that dstData has a Vulkan type() because it could be
        // uninitialized and that assert would fail.
        dstData.emplace<VulkanBackendSemaphoreData>(fVkSemaphore);
    }
};

static const VulkanBackendSemaphoreData* get_and_cast_data(const BackendSemaphore& sem) {
    auto data = BackendSemaphorePriv::GetData(sem);
    SkASSERT(!data || data->type() == skgpu::BackendApi::kVulkan);
    return static_cast<const VulkanBackendSemaphoreData*>(data);
}

namespace BackendSemaphores {
BackendSemaphore MakeVulkan(VkSemaphore sem) {
    return BackendSemaphorePriv::Make(skgpu::BackendApi::kVulkan, VulkanBackendSemaphoreData(sem));
}

VkSemaphore GetVkSemaphore(const BackendSemaphore& sem) {
    if (!sem.isValid() || sem.backend() != skgpu::BackendApi::kVulkan) {
        return VK_NULL_HANDLE;
    }
    const VulkanBackendSemaphoreData* vkData = get_and_cast_data(sem);
    SkASSERT(vkData);
    return vkData->semaphore();
}

}  // namespace BackendSemaphores

}  // namespace skgpu::graphite
