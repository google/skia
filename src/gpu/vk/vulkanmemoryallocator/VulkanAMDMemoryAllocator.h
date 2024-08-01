/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_VulkanAMDMemoryAllocator_DEFINED
#define skgpu_VulkanAMDMemoryAllocator_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/vk/VulkanMemoryAllocator.h"
#include "include/gpu/vk/VulkanTypes.h"
#include "include/private/gpu/vk/SkiaVulkan.h"
#include "src/gpu/vk/vulkanmemoryallocator/VulkanMemoryAllocatorWrapper.h"

#include <cstdint>
#include <optional>
#include <utility>

namespace skgpu {

class VulkanExtensions;
enum class ThreadSafe : bool;
struct VulkanInterface;

class VulkanAMDMemoryAllocator : public VulkanMemoryAllocator {
public:
    static sk_sp<VulkanMemoryAllocator> Make(VkInstance instance,
                                             VkPhysicalDevice physicalDevice,
                                             VkDevice device,
                                             uint32_t physicalDeviceVersion,
                                             const VulkanExtensions* extensions,
                                             const VulkanInterface* interface,
                                             ThreadSafe,
                                             std::optional<VkDeviceSize> blockSize);

    ~VulkanAMDMemoryAllocator() override;

    VkResult allocateImageMemory(VkImage image,
                                 uint32_t allocationPropertyFlags,
                                 skgpu::VulkanBackendMemory*) override;

    VkResult allocateBufferMemory(VkBuffer buffer,
                                  BufferUsage usage,
                                  uint32_t allocationPropertyFlags,
                                  skgpu::VulkanBackendMemory*) override;

    void freeMemory(const VulkanBackendMemory&) override;

    void getAllocInfo(const VulkanBackendMemory&, VulkanAlloc*) const override;

    VkResult mapMemory(const VulkanBackendMemory&, void** data) override;
    void unmapMemory(const VulkanBackendMemory&) override;

    VkResult flushMemory(const VulkanBackendMemory&,
                         VkDeviceSize offset,
                         VkDeviceSize size) override;
    VkResult invalidateMemory(const VulkanBackendMemory&,
                              VkDeviceSize offset,
                              VkDeviceSize size) override;

    std::pair<uint64_t, uint64_t> totalAllocatedAndUsedMemory() const override;

private:
    VulkanAMDMemoryAllocator(VmaAllocator allocator);

    VmaAllocator fAllocator;
};

}  // namespace skgpu

#endif  // skgpu_VulkanAMDMemoryAllocator_DEFINED
