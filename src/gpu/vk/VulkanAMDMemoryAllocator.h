/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_VulkanAMDMemoryAllocator_DEFINED
#define skgpu_VulkanAMDMemoryAllocator_DEFINED

#include "include/gpu/vk/VulkanMemoryAllocator.h"

#if defined(SK_USE_VMA)
#include "src/gpu/vk/vulkanmemoryallocator/VulkanMemoryAllocatorWrapper.h"
#endif

namespace skgpu {

class VulkanExtensions;
struct VulkanInterface;

enum class ThreadSafe : bool {
    kNo = false,
    kYes = true,
};

#if !defined(SK_USE_VMA)
class VulkanAMDMemoryAllocator {
public:
    static sk_sp<VulkanMemoryAllocator> Make(VkInstance instance,
                                             VkPhysicalDevice physicalDevice,
                                             VkDevice device,
                                             uint32_t physicalDeviceVersion,
                                             const VulkanExtensions* extensions,
                                             const VulkanInterface* interface,
                                             ThreadSafe);
};

#else

class VulkanAMDMemoryAllocator : public VulkanMemoryAllocator {
public:
    static sk_sp<VulkanMemoryAllocator> Make(VkInstance instance,
                                             VkPhysicalDevice physicalDevice,
                                             VkDevice device,
                                             uint32_t physicalDeviceVersion,
                                             const VulkanExtensions* extensions,
                                             const VulkanInterface* interface,
                                             ThreadSafe);

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
    VulkanAMDMemoryAllocator(VmaAllocator allocator);

    VmaAllocator fAllocator;
};

#endif // SK_USE_VMA

} // namespace skgpu

#endif // skgpu_VulkanAMDMemoryAllocator_DEFINED
