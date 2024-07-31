/*
* Copyright 2015 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef skgpu_VulkanMemory_DEFINED
#define skgpu_VulkanMemory_DEFINED

#include "include/gpu/vk/VulkanMemoryAllocator.h"
#include "include/private/gpu/vk/SkiaVulkan.h"

#include <functional>

namespace skgpu {
enum class Protected : bool;
struct VulkanAlloc;

namespace VulkanMemory {
    using CheckResult = bool(VkResult);

    bool AllocBufferMemory(VulkanMemoryAllocator*,
                           VkBuffer buffer,
                           skgpu::Protected isProtected,
                           skgpu::VulkanMemoryAllocator::BufferUsage,
                           bool shouldPersistentlyMapCpuToGpu,
                           const std::function<CheckResult>&,
                           VulkanAlloc* alloc);

    void FreeBufferMemory(VulkanMemoryAllocator*, const VulkanAlloc& alloc);

    bool AllocImageMemory(VulkanMemoryAllocator*,
                          VkImage image,
                          skgpu::Protected isProtected,
                          bool forceDedicatedMemory,
                          bool useLazyAllocation,
                          const std::function<CheckResult>&,
                          VulkanAlloc* alloc);

    void FreeImageMemory(VulkanMemoryAllocator*, const VulkanAlloc& alloc);

    // Maps the entire skgpu::VulkanAlloc and returns a pointer to the start of the allocation.
    // Underneath the hood, we may map more than the range of the skgpu::VulkanAlloc (e.g. the
    // entire VkDeviceMemory), but the pointer returned will always be to the start of the
    // skgpu::VulkanAlloc. The caller should also never assume more than the skgpu::VulkanAlloc
    // block has been mapped.
    void* MapAlloc(VulkanMemoryAllocator*,
                   const VulkanAlloc&,
                   const std::function<CheckResult>&);
    void UnmapAlloc(VulkanMemoryAllocator*, const VulkanAlloc& alloc);

    // For the Flush and Invalidate calls, the offset should be relative to the skgpu::VulkanAlloc.
    // Thus this will often be 0. The client does not need to make sure the offset and size are
    // aligned to the nonCoherentAtomSize, the internal calls will handle that.
    void FlushMappedAlloc(VulkanMemoryAllocator*,
                          const skgpu::VulkanAlloc&,
                          VkDeviceSize offset,
                          VkDeviceSize size,
                          const std::function<CheckResult>&);
    void InvalidateMappedAlloc(VulkanMemoryAllocator*,
                               const VulkanAlloc& alloc,
                               VkDeviceSize offset,
                               VkDeviceSize size,
                               const std::function<CheckResult>&);

    // Helper for aligning and setting VkMappedMemoryRange for flushing/invalidating noncoherent
    // memory.
    void GetNonCoherentMappedMemoryRange(const VulkanAlloc&,
                                         VkDeviceSize offset,
                                         VkDeviceSize size,
                                         VkDeviceSize alignment,
                                         VkMappedMemoryRange*);
}  // namespace VulkanMemory

}  // namespace skgpu

#endif
