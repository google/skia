/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef VkTestMemoryAllocator_DEFINED
#define VkTestMemoryAllocator_DEFINED

#include "include/gpu/vk/VulkanMemoryAllocator.h"

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc++98-compat-extra-semi"
#endif

#include <vk_mem_alloc.h>

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

namespace skgpu {
class VulkanExtensions;
struct VulkanInterface;
}  // namespace skgpu

namespace sk_gpu_test {

// A test-only Vulkan memory allocator. Based on
// https://skia.googlesource.com/skia/+/c3fbd20fbc7d3662dd31a0e4139226d2951d1ae4/src/gpu/vk/VulkanAMDMemoryAllocator.h.
class VkTestMemoryAllocator : public skgpu::VulkanMemoryAllocator {
public:
    static sk_sp<VulkanMemoryAllocator> Make(VkInstance instance,
                                             VkPhysicalDevice physicalDevice,
                                             VkDevice device,
                                             uint32_t physicalDeviceVersion,
                                             const skgpu::VulkanExtensions* extensions,
                                             const skgpu::VulkanInterface* interface);

    ~VkTestMemoryAllocator() override;

    VkResult allocateImageMemory(VkImage image,
                                 uint32_t allocationPropertyFlags,
                                 skgpu::VulkanBackendMemory*) override;

    VkResult allocateBufferMemory(VkBuffer buffer,
                                  BufferUsage usage,
                                  uint32_t allocationPropertyFlags,
                                  skgpu::VulkanBackendMemory*) override;

    void freeMemory(const skgpu::VulkanBackendMemory&) override;

    void getAllocInfo(const skgpu::VulkanBackendMemory&, skgpu::VulkanAlloc*) const override;

    VkResult mapMemory(const skgpu::VulkanBackendMemory&, void** data) override;
    void unmapMemory(const skgpu::VulkanBackendMemory&) override;

    VkResult flushMemory(const skgpu::VulkanBackendMemory&,
                         VkDeviceSize offset,
                         VkDeviceSize size) override;
    VkResult invalidateMemory(const skgpu::VulkanBackendMemory&,
                              VkDeviceSize offset,
                              VkDeviceSize size) override;

    std::pair<uint64_t, uint64_t> totalAllocatedAndUsedMemory() const override;

private:
    VkTestMemoryAllocator(VmaAllocator allocator);

    VmaAllocator fAllocator;
};

}  // namespace sk_gpu_test

#endif  // VkTestMemoryAllocator_DEFINED
