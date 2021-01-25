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
class GrVkExtensions;
struct GrVkInterface;

#ifndef SK_USE_VMA
class GrVkAMDMemoryAllocator {
public:
    static sk_sp<GrVkMemoryAllocator> Make(VkInstance instance,
                                           VkPhysicalDevice physicalDevice,
                                           VkDevice device,
                                           uint32_t physicalDeviceVersion,
                                           const GrVkExtensions* extensions,
                                           sk_sp<const GrVkInterface> interface,
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
                                           const GrVkExtensions* extensions,
                                           sk_sp<const GrVkInterface> interface,
                                           const GrVkCaps* caps);

    ~GrVkAMDMemoryAllocator() override;

    VkResult allocateImageMemory(VkImage image, AllocationPropertyFlags flags,
                                 GrVkBackendMemory*) override;

    VkResult allocateBufferMemory(VkBuffer buffer, BufferUsage usage,
                                  AllocationPropertyFlags flags, GrVkBackendMemory*) override;

    void freeMemory(const GrVkBackendMemory&) override;

    void getAllocInfo(const GrVkBackendMemory&, GrVkAlloc*) const override;

    VkResult mapMemory(const GrVkBackendMemory&, void** data) override;
    void unmapMemory(const GrVkBackendMemory&) override;

    VkResult flushMemory(const GrVkBackendMemory&, VkDeviceSize offset, VkDeviceSize size) override;
    VkResult invalidateMemory(const GrVkBackendMemory&, VkDeviceSize offset,
                              VkDeviceSize size) override;

    uint64_t totalUsedMemory() const override;
    uint64_t totalAllocatedMemory() const override;

private:
    GrVkAMDMemoryAllocator(VmaAllocator allocator, sk_sp<const GrVkInterface> interface,
                           bool mustUseCoherentHostVisibleMemory);

    VmaAllocator fAllocator;

    // If a future version of the AMD allocator has helper functions for flushing and invalidating
    // memory, then we won't need to save the GrVkInterface here since we won't need to make direct
    // vulkan calls.
    sk_sp<const GrVkInterface> fInterface;

    // For host visible allocations do we require they are coherent or not. All devices are required
    // to support a host visible and coherent memory type. This is used to work around bugs for
    // devices that don't handle non coherent memory correctly.
    bool fMustUseCoherentHostVisibleMemory;

    using INHERITED = GrVkMemoryAllocator;
};

#endif // SK_USE_VMA

#endif
