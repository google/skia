/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkAMDMemoryAllocator_DEFINED
#define GrVkAMDMemoryAllocator_DEFINED

#include "include/gpu/vk/GrVkMemoryAllocator.h"

class GrVkCaps;

namespace skgpu {
class VulkanExtensions;
struct VulkanInterface;
}

#ifndef SK_USE_VMA
class GrVkAMDMemoryAllocator {
public:
    static sk_sp<GrVkMemoryAllocator> Make(VkInstance instance,
                                           VkPhysicalDevice physicalDevice,
                                           VkDevice device,
                                           uint32_t physicalDeviceVersion,
                                           const skgpu::VulkanExtensions* extensions,
                                           sk_sp<const skgpu::VulkanInterface> interface,
                                           const GrVkCaps* caps);
};

#else

#include "GrVulkanMemoryAllocator.h"

class GrVkAMDMemoryAllocator : public GrVkMemoryAllocator {
public:
    static sk_sp<GrVkMemoryAllocator> Make(VkInstance instance,
                                           VkPhysicalDevice physicalDevice,
                                           VkDevice device,
                                           uint32_t physicalDeviceVersion,
                                           const skgpu::VulkanExtensions* extensions,
                                           sk_sp<const skgpu::VulkanInterface> interface,
                                           const GrVkCaps* caps);

    ~GrVkAMDMemoryAllocator() override;

    VkResult allocateImageMemory(VkImage image, AllocationPropertyFlags flags,
                                 skgpu::VulkanBackendMemory*) override;

    VkResult allocateBufferMemory(VkBuffer buffer,
                                  BufferUsage usage,
                                  AllocationPropertyFlags flags,
                                  skgpu::VulkanBackendMemory*) override;

    void freeMemory(const skgpu::VulkanBackendMemory&) override;

    void getAllocInfo(const skgpu::VulkanBackendMemory&, skgpu::VulkanAlloc*) const override;

    VkResult mapMemory(const skgpu::VulkanBackendMemory&, void** data) override;
    void unmapMemory(const skgpu::VulkanBackendMemory&) override;

    VkResult flushMemory(const skgpu::VulkanBackendMemory&, VkDeviceSize offset,
                         VkDeviceSize size) override;
    VkResult invalidateMemory(const skgpu::VulkanBackendMemory&, VkDeviceSize offset,
                              VkDeviceSize size) override;

    uint64_t totalUsedMemory() const override;
    uint64_t totalAllocatedMemory() const override;

private:
    GrVkAMDMemoryAllocator(VmaAllocator allocator, sk_sp<const skgpu::VulkanInterface> interface,
                           bool mustUseCoherentHostVisibleMemory);

    VmaAllocator fAllocator;

    // If a future version of the AMD allocator has helper functions for flushing and invalidating
    // memory, then we won't need to save the skgpu::VulkanInterface here since we won't need to
    // make direct vulkan calls.
    sk_sp<const skgpu::VulkanInterface> fInterface;

    // For host visible allocations do we require they are coherent or not. All devices are required
    // to support a host visible and coherent memory type. This is used to work around bugs for
    // devices that don't handle non coherent memory correctly.
    bool fMustUseCoherentHostVisibleMemory;

    using INHERITED = GrVkMemoryAllocator;
};

#endif // SK_USE_VMA

#endif
