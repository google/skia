/*
* Copyright 2015 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrVkMemory_DEFINED
#define GrVkMemory_DEFINED

#include "include/gpu/vk/GrVkMemoryAllocator.h"
#include "include/gpu/vk/GrVkTypes.h"
#include "include/private/SkTArray.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"

class GrVkGpu;

namespace GrVkMemory {
    /**
    * Allocates vulkan device memory and binds it to the gpu's device for the given object.
    * Returns true if allocation succeeded.
    */
    bool AllocAndBindBufferMemory(GrVkGpu* gpu,
                                  VkBuffer buffer,
                                  GrVkMemoryAllocator::BufferUsage,
                                  skgpu::VulkanAlloc* alloc);
    void FreeBufferMemory(const GrVkGpu* gpu, const skgpu::VulkanAlloc& alloc);

    bool AllocAndBindImageMemory(GrVkGpu* gpu,
                                 VkImage image,
                                 GrMemoryless,
                                 skgpu::VulkanAlloc* alloc);
    void FreeImageMemory(const GrVkGpu* gpu, const skgpu::VulkanAlloc& alloc);

    // Maps the entire skgpu::VulkanAlloc and returns a pointer to the start of the allocation.
    // Underneath the hood, we may map more than the range of the skgpu::VulkanAlloc (e.g. the
    // entire VkDeviceMemory), but the pointer returned will always be to the start of the
    // skgpu::VulkanAlloc. The caller should also never assume more than the skgpu::VulkanAlloc
    // block has been mapped.
    void* MapAlloc(GrVkGpu* gpu, const skgpu::VulkanAlloc& alloc);
    void UnmapAlloc(const GrVkGpu* gpu, const skgpu::VulkanAlloc& alloc);

    // For the Flush and Invalidate calls, the offset should be relative to the skgpu::VulkanAlloc.
    // Thus this will often be 0. The client does not need to make sure the offset and size are
    // aligned to the nonCoherentAtomSize, the internal calls will handle that.
    void FlushMappedAlloc(GrVkGpu* gpu, const skgpu::VulkanAlloc& alloc, VkDeviceSize offset,
                          VkDeviceSize size);
    void InvalidateMappedAlloc(GrVkGpu* gpu, const skgpu::VulkanAlloc& alloc, VkDeviceSize offset,
                               VkDeviceSize size);

    // Helper for aligning and setting VkMappedMemoryRange for flushing/invalidating noncoherent
    // memory.
    void GetNonCoherentMappedMemoryRange(const skgpu::VulkanAlloc&,
                                         VkDeviceSize offset,
                                         VkDeviceSize size,
                                         VkDeviceSize alignment,
                                         VkMappedMemoryRange*);
}  // namespace GrVkMemory

#endif
