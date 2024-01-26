/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_VulkanSharedContext_DEFINED
#define skgpu_graphite_VulkanSharedContext_DEFINED

#include "src/gpu/graphite/SharedContext.h"

#include "include/gpu/vk/VulkanTypes.h"
#include "src/gpu/graphite/vk/VulkanCaps.h"

namespace skgpu {
struct VulkanBackendContext;
struct VulkanInterface;
class VulkanMemoryAllocator;
}

namespace skgpu::graphite {

struct ContextOptions;
class VulkanCaps;

class VulkanSharedContext final : public SharedContext {
public:
    static sk_sp<SharedContext> Make(const VulkanBackendContext&, const ContextOptions&);
    ~VulkanSharedContext() override;

    const VulkanCaps& vulkanCaps() const { return static_cast<const VulkanCaps&>(*this->caps()); }

    const skgpu::VulkanInterface* interface() const { return fInterface.get(); }

    skgpu::VulkanMemoryAllocator* memoryAllocator() const { return fMemoryAllocator.get(); }

    VkDevice device() const { return fDevice; }
    uint32_t  queueIndex() const { return fQueueIndex; }

    std::unique_ptr<ResourceProvider> makeResourceProvider(SingleOwner*,
                                                           uint32_t recorderID,
                                                           size_t resourceBudget) override;

    bool checkVkResult(VkResult result) const;

private:
    VulkanSharedContext(const VulkanBackendContext&,
                        sk_sp<const skgpu::VulkanInterface> interface,
                        sk_sp<skgpu::VulkanMemoryAllocator> memoryAllocator,
                        std::unique_ptr<const VulkanCaps> caps);

    sk_sp<const skgpu::VulkanInterface> fInterface;
    sk_sp<skgpu::VulkanMemoryAllocator> fMemoryAllocator;

    VkDevice fDevice;
    uint32_t fQueueIndex;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_VulkanSharedContext_DEFINED
