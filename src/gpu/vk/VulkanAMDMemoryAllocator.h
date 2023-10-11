/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_VulkanAMDMemoryAllocator_DEFINED
#define skgpu_VulkanAMDMemoryAllocator_DEFINED

#include "include/gpu/vk/VulkanMemoryAllocator.h"

namespace skgpu {

class VulkanExtensions;
struct VulkanInterface;

#ifndef SK_USE_VMA
class VulkanAMDMemoryAllocator {
public:
    static sk_sp<VulkanMemoryAllocator> Make(VkInstance instance,
                                             VkPhysicalDevice physicalDevice,
                                             VkDevice device,
                                             uint32_t physicalDeviceVersion,
                                             const VulkanExtensions* extensions,
                                             sk_sp<const VulkanInterface> interface,
                                             bool mustUseCoherentHostVisibleMemory,
                                             bool threadSafe);
};

#else

#include "VulkanMemoryAllocatorWrapper.h"  // NO_G3_REWRITE

class VulkanAMDMemoryAllocator : public VulkanMemoryAllocator {
public:
    static sk_sp<VulkanMemoryAllocator> Make(VkInstance instance,
                                             VkPhysicalDevice physicalDevice,
                                             VkDevice device,
                                             uint32_t physicalDeviceVersion,
                                             const VulkanExtensions* extensions,
                                             sk_sp<const VulkanInterface> interface,
                                             bool mustUseCoherentHostVisibleMemory,
                                             bool threadSafe);

    ~VulkanAMDMemoryAllocator() override;

    VkResult allocateImageMemory(VkImage image, uint32_t allocationPropertyFlags,
                                 skgpu::VulkanBackendMemory*) override;

    VkResult allocateBufferMemory(VkBuffer buffer,
                                  BufferUsage usage,
                                  uint32_t allocationPropertyFlags,
                                  skgpu::VulkanBackendMemory*) override;

    void freeMemory(const VulkanBackendMemory&) override;

    void getAllocInfo(const VulkanBackendMemory&, VulkanAlloc*) const override;

    VkResult mapMemory(const VulkanBackendMemory&, void** data) override;
    void unmapMemory(const VulkanBackendMemory&) override;

    VkResult flushMemory(const VulkanBackendMemory&, VkDeviceSize offset,
                         VkDeviceSize size) override;
    VkResult invalidateMemory(const VulkanBackendMemory&, VkDeviceSize offset,
                              VkDeviceSize size) override;

    std::pair<uint64_t, uint64_t> totalAllocatedAndUsedMemory() const override;

private:
    VulkanAMDMemoryAllocator(VmaAllocator allocator, sk_sp<const VulkanInterface> interface,
                             bool mustUseCoherentHostVisibleMemory);

    VmaAllocator fAllocator;

    // If a future version of the AMD allocator has helper functions for flushing and invalidating
    // memory, then we won't need to save the VulkanInterface here since we won't need to
    // make direct vulkan calls.
    sk_sp<const VulkanInterface> fInterface;

    // For host visible allocations do we require they are coherent or not. All devices are required
    // to support a host visible and coherent memory type. This is used to work around bugs for
    // devices that don't handle non coherent memory correctly.
    bool fMustUseCoherentHostVisibleMemory;
};

#endif // SK_USE_VMA

} // namespace skgpu

#endif // skgpu_VulkanAMDMemoryAllocator_DEFINED
