/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_VulkanSharedContext_DEFINED
#define skgpu_graphite_VulkanSharedContext_DEFINED

#include "include/private/base/SkMutex.h"
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

    VkPhysicalDevice physDevice() const { return fPhysDevice; }
    VkDevice device() const { return fDevice; }
    uint32_t  queueIndex() const { return fQueueIndex; }

    std::unique_ptr<ResourceProvider> makeResourceProvider(SingleOwner*,
                                                           uint32_t recorderID,
                                                           size_t resourceBudget) override;

    bool checkVkResult(VkResult result) const;

    bool isDeviceLost() const override {
        SkAutoMutexExclusive lock(fDeviceIsLostMutex);
        return fDeviceIsLost;
    }

private:
    VulkanSharedContext(const VulkanBackendContext&,
                        sk_sp<const skgpu::VulkanInterface>,
                        sk_sp<skgpu::VulkanMemoryAllocator>,
                        std::unique_ptr<const VulkanCaps>,
                        SkExecutor*,
                        SkSpan<sk_sp<SkRuntimeEffect>> userDefinedKnownRuntimeEffects);

    sk_sp<const skgpu::VulkanInterface> fInterface;
    sk_sp<skgpu::VulkanMemoryAllocator> fMemoryAllocator;

    VkPhysicalDevice fPhysDevice;
    VkDevice fDevice;
    uint32_t fQueueIndex;

    mutable SkMutex fDeviceIsLostMutex;
    // TODO(b/322207523): consider refactoring to remove the mutable keyword from fDeviceIsLost.
    mutable bool fDeviceIsLost SK_GUARDED_BY(fDeviceIsLostMutex) = false;
    skgpu::VulkanDeviceLostContext fDeviceLostContext;
    skgpu::VulkanDeviceLostProc fDeviceLostProc;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_VulkanSharedContext_DEFINED
