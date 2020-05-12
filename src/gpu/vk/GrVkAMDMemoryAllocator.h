/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkAMDMemoryAllocator_DEFINED
#define GrVkAMDMemoryAllocator_DEFINED

#include "include/gpu/vk/GrVkMemoryAllocator.h"

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
                                           sk_sp<const GrVkInterface> interface);
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
                                           sk_sp<const GrVkInterface> interface);

    ~GrVkAMDMemoryAllocator() override;

    bool allocateMemoryForImage(VkImage image, AllocationPropertyFlags flags,
                                GrVkBackendMemory*) override;

    bool allocateMemoryForBuffer(VkBuffer buffer, BufferUsage usage,
                                 AllocationPropertyFlags flags, GrVkBackendMemory*) override;

    void freeMemory(const GrVkBackendMemory&) override;

    void getAllocInfo(const GrVkBackendMemory&, GrVkAlloc*) const override;

    void* mapMemory(const GrVkBackendMemory&) override;
    void unmapMemory(const GrVkBackendMemory&) override;

    void flushMappedMemory(const GrVkBackendMemory&, VkDeviceSize offset,
                           VkDeviceSize size) override;
    void invalidateMappedMemory(const GrVkBackendMemory&, VkDeviceSize offset,
                                VkDeviceSize size) override;

    uint64_t totalUsedMemory() const override;
    uint64_t totalAllocatedMemory() const override;

private:
    GrVkAMDMemoryAllocator(VmaAllocator allocator, sk_sp<const GrVkInterface> interface);

    VmaAllocator fAllocator;

    // If a future version of the AMD allocator has helper functions for flushing and invalidating
    // memory, then we won't need to save the GrVkInterface here since we won't need to make direct
    // vulkan calls.
    sk_sp<const GrVkInterface> fInterface;

    typedef GrVkMemoryAllocator INHERITED;
};

#endif // SK_USE_VMA

#endif
