/*
* Copyright 2015 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/vk/VulkanMemoryAllocator.h"
#include "include/gpu/vk/VulkanTypes.h"
#include "src/gpu/vk/VulkanMemory.h"

#include <cstdint>
#include <cstring>

namespace skgpu {

using BufferUsage = VulkanMemoryAllocator::BufferUsage;

bool VulkanMemory::AllocBufferMemory(VulkanMemoryAllocator* allocator,
                                     VkBuffer buffer,
                                     skgpu::Protected isProtected,
                                     BufferUsage usage,
                                     bool shouldPersistentlyMapCpuToGpu,
                                     const std::function<CheckResult>& checkResult,
                                     VulkanAlloc* alloc) {
    VulkanBackendMemory memory = 0;
    uint32_t propFlags;
    if (usage == BufferUsage::kTransfersFromCpuToGpu ||
        (usage == BufferUsage::kCpuWritesGpuReads && shouldPersistentlyMapCpuToGpu)) {
        // In general it is always fine (and often better) to keep buffers always mapped that we are
        // writing to on the cpu.
        propFlags = VulkanMemoryAllocator::kPersistentlyMapped_AllocationPropertyFlag;
    } else {
        propFlags = VulkanMemoryAllocator::kNone_AllocationPropertyFlag;
    }

    if (isProtected == Protected::kYes) {
        propFlags = propFlags | VulkanMemoryAllocator::kProtected_AllocationPropertyFlag;
    }

    VkResult result = allocator->allocateBufferMemory(buffer, usage, propFlags, &memory);
    if (!checkResult(result)) {
        return false;
    }
    allocator->getAllocInfo(memory, alloc);
    return true;
}

void VulkanMemory::FreeBufferMemory(VulkanMemoryAllocator* allocator, const VulkanAlloc& alloc) {
    SkASSERT(alloc.fBackendMemory);
    allocator->freeMemory(alloc.fBackendMemory);
}

bool VulkanMemory::AllocImageMemory(VulkanMemoryAllocator* allocator,
                                    VkImage image,
                                    Protected isProtected,
                                    bool forceDedicatedMemory,
                                    bool useLazyAllocation,
                                    const std::function<CheckResult>& checkResult,
                                    VulkanAlloc* alloc) {
    VulkanBackendMemory memory = 0;

    uint32_t propFlags;
    // If we ever find that our allocator is not aggressive enough in using dedicated image
    // memory we can add a size check here to force the use of dedicate memory. However for now,
    // we let the allocators decide. The allocator can query the GPU for each image to see if the
    // GPU recommends or requires the use of dedicated memory.
    if (forceDedicatedMemory) {
        propFlags = VulkanMemoryAllocator::kDedicatedAllocation_AllocationPropertyFlag;
    } else {
        propFlags = VulkanMemoryAllocator::kNone_AllocationPropertyFlag;
    }

    if (isProtected == Protected::kYes) {
        propFlags = propFlags | VulkanMemoryAllocator::kProtected_AllocationPropertyFlag;
    }

    if (useLazyAllocation) {
        propFlags = propFlags | VulkanMemoryAllocator::kLazyAllocation_AllocationPropertyFlag;
    }

    VkResult result = allocator->allocateImageMemory(image, propFlags, &memory);
    if (!checkResult(result)) {
        return false;
    }

    allocator->getAllocInfo(memory, alloc);
    return true;
}

void VulkanMemory::FreeImageMemory(VulkanMemoryAllocator* allocator,
                                   const VulkanAlloc& alloc) {
    SkASSERT(alloc.fBackendMemory);
    allocator->freeMemory(alloc.fBackendMemory);
}

void* VulkanMemory::MapAlloc(VulkanMemoryAllocator* allocator,
                             const VulkanAlloc& alloc,
                             const std::function<CheckResult>& checkResult) {
    SkASSERT(VulkanAlloc::kMappable_Flag & alloc.fFlags);
    SkASSERT(alloc.fBackendMemory);
    void* mapPtr;
    VkResult result = allocator->mapMemory(alloc.fBackendMemory, &mapPtr);
    if (!checkResult(result)) {
        return nullptr;
    }
    return mapPtr;
}

void VulkanMemory::UnmapAlloc(VulkanMemoryAllocator* allocator,
                              const VulkanAlloc& alloc) {
    SkASSERT(alloc.fBackendMemory);
    allocator->unmapMemory(alloc.fBackendMemory);
}

void VulkanMemory::GetNonCoherentMappedMemoryRange(const VulkanAlloc& alloc,
                                                   VkDeviceSize offset,
                                                   VkDeviceSize size,
                                                   VkDeviceSize alignment,
                                                   VkMappedMemoryRange* range) {
    SkASSERT(alloc.fFlags & VulkanAlloc::kNoncoherent_Flag);
    offset = offset + alloc.fOffset;
    VkDeviceSize offsetDiff = offset & (alignment -1);
    offset = offset - offsetDiff;
    size = (size + alignment - 1) & ~(alignment - 1);
#ifdef SK_DEBUG
    SkASSERT(offset >= alloc.fOffset);
    SkASSERT(offset + size <= alloc.fOffset + alloc.fSize);
    SkASSERT(0 == (offset & (alignment-1)));
    SkASSERT(size > 0);
    SkASSERT(0 == (size & (alignment-1)));
#endif

    std::memset(range, 0, sizeof(VkMappedMemoryRange));
    range->sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    range->memory = alloc.fMemory;
    range->offset = offset;
    range->size = size;
}

void VulkanMemory::FlushMappedAlloc(VulkanMemoryAllocator* allocator,
                                    const VulkanAlloc& alloc,
                                    VkDeviceSize offset,
                                    VkDeviceSize size,
                                    const std::function<CheckResult>& checkResult) {
    if (alloc.fFlags & VulkanAlloc::kNoncoherent_Flag) {
        SkASSERT(offset == 0);
        SkASSERT(size <= alloc.fSize);
        SkASSERT(alloc.fBackendMemory);
        VkResult result = allocator->flushMemory(alloc.fBackendMemory, offset, size);
        checkResult(result);
    }
}

void VulkanMemory::InvalidateMappedAlloc(VulkanMemoryAllocator* allocator,
                                         const VulkanAlloc& alloc,
                                         VkDeviceSize offset,
                                         VkDeviceSize size,
                                         const std::function<CheckResult>& checkResult) {
    if (alloc.fFlags & VulkanAlloc::kNoncoherent_Flag) {
        SkASSERT(offset == 0);
        SkASSERT(size <= alloc.fSize);
        SkASSERT(alloc.fBackendMemory);
        VkResult result = allocator->invalidateMemory(alloc.fBackendMemory, offset, size);
        checkResult(result);
    }
}

}  // namespace skgpu
