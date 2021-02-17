/*
* Copyright 2015 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/gpu/vk/GrVkMemory.h"

#include "src/gpu/vk/GrVkGpu.h"
#include "src/gpu/vk/GrVkUtil.h"

using AllocationPropertyFlags = GrVkMemoryAllocator::AllocationPropertyFlags;
using BufferUsage = GrVkMemoryAllocator::BufferUsage;

bool GrVkMemory::AllocAndBindBufferMemory(GrVkGpu* gpu,
                                          VkBuffer buffer,
                                          BufferUsage usage,
                                          GrVkAlloc* alloc) {
    GrVkMemoryAllocator* allocator = gpu->memoryAllocator();
    GrVkBackendMemory memory = 0;

    AllocationPropertyFlags propFlags;
    bool shouldPersistentlyMapCpuToGpu = gpu->vkCaps().shouldPersistentlyMapCpuToGpuBuffers();
    if (usage == BufferUsage::kTransfersFromCpuToGpu ||
        (usage == BufferUsage::kCpuWritesGpuReads && shouldPersistentlyMapCpuToGpu)) {
        // In general it is always fine (and often better) to keep buffers always mapped that we are
        // writing to on the cpu.
        propFlags = AllocationPropertyFlags::kPersistentlyMapped;
    } else {
        propFlags = AllocationPropertyFlags::kNone;
    }

    VkResult result = allocator->allocateBufferMemory(buffer, usage, propFlags, &memory);
    if (!gpu->checkVkResult(result)) {
        return false;
    }
    allocator->getAllocInfo(memory, alloc);

    // Bind buffer
    VkResult err;
    GR_VK_CALL_RESULT(gpu, err, BindBufferMemory(gpu->device(), buffer, alloc->fMemory,
                                                 alloc->fOffset));
    if (err) {
        FreeBufferMemory(gpu, *alloc);
        return false;
    }

    return true;
}

void GrVkMemory::FreeBufferMemory(const GrVkGpu* gpu, const GrVkAlloc& alloc) {
    SkASSERT(alloc.fBackendMemory);
    GrVkMemoryAllocator* allocator = gpu->memoryAllocator();
    allocator->freeMemory(alloc.fBackendMemory);
}

bool GrVkMemory::AllocAndBindImageMemory(GrVkGpu* gpu,
                                         VkImage image,
                                         bool linearTiling,
                                         GrVkAlloc* alloc) {
    SkASSERT(!linearTiling);
    GrVkMemoryAllocator* allocator = gpu->memoryAllocator();
    GrVkBackendMemory memory = 0;

    VkMemoryRequirements memReqs;
    GR_VK_CALL(gpu->vkInterface(), GetImageMemoryRequirements(gpu->device(), image, &memReqs));

    AllocationPropertyFlags propFlags;
    // If we ever find that our allocator is not aggressive enough in using dedicated image
    // memory we can add a size check here to force the use of dedicate memory. However for now,
    // we let the allocators decide. The allocator can query the GPU for each image to see if the
    // GPU recommends or requires the use of dedicated memory.
    if (gpu->vkCaps().shouldAlwaysUseDedicatedImageMemory()) {
        propFlags = AllocationPropertyFlags::kDedicatedAllocation;
    } else {
        propFlags = AllocationPropertyFlags::kNone;
    }

    if (gpu->protectedContext()) {
        propFlags |= AllocationPropertyFlags::kProtected;
    }

    VkResult result = allocator->allocateImageMemory(image, propFlags, &memory);
    if (!gpu->checkVkResult(result)) {
        return false;
    }

    allocator->getAllocInfo(memory, alloc);

    // Bind buffer
    VkResult err;
    GR_VK_CALL_RESULT(gpu, err, BindImageMemory(gpu->device(), image, alloc->fMemory,
                                                alloc->fOffset));
    if (err) {
        FreeImageMemory(gpu, linearTiling, *alloc);
        return false;
    }

    return true;
}

void GrVkMemory::FreeImageMemory(const GrVkGpu* gpu, bool linearTiling,
                                 const GrVkAlloc& alloc) {
    SkASSERT(alloc.fBackendMemory);
    GrVkMemoryAllocator* allocator = gpu->memoryAllocator();
    allocator->freeMemory(alloc.fBackendMemory);
}

void* GrVkMemory::MapAlloc(GrVkGpu* gpu, const GrVkAlloc& alloc) {
    SkASSERT(GrVkAlloc::kMappable_Flag & alloc.fFlags);
    SkASSERT(alloc.fBackendMemory);
    GrVkMemoryAllocator* allocator = gpu->memoryAllocator();
    void* mapPtr;
    VkResult result = allocator->mapMemory(alloc.fBackendMemory, &mapPtr);
    if (!gpu->checkVkResult(result)) {
        return nullptr;
    }
    return mapPtr;
}

void GrVkMemory::UnmapAlloc(const GrVkGpu* gpu, const GrVkAlloc& alloc) {
    SkASSERT(alloc.fBackendMemory);
    GrVkMemoryAllocator* allocator = gpu->memoryAllocator();
    allocator->unmapMemory(alloc.fBackendMemory);
}

void GrVkMemory::GetNonCoherentMappedMemoryRange(const GrVkAlloc& alloc, VkDeviceSize offset,
                                                 VkDeviceSize size, VkDeviceSize alignment,
                                                 VkMappedMemoryRange* range) {
    SkASSERT(alloc.fFlags & GrVkAlloc::kNoncoherent_Flag);
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

    memset(range, 0, sizeof(VkMappedMemoryRange));
    range->sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    range->memory = alloc.fMemory;
    range->offset = offset;
    range->size = size;
}

void GrVkMemory::FlushMappedAlloc(GrVkGpu* gpu, const GrVkAlloc& alloc, VkDeviceSize offset,
                                  VkDeviceSize size) {
    if (alloc.fFlags & GrVkAlloc::kNoncoherent_Flag) {
        SkASSERT(offset == 0);
        SkASSERT(size <= alloc.fSize);
        SkASSERT(alloc.fBackendMemory);
        GrVkMemoryAllocator* allocator = gpu->memoryAllocator();
        VkResult result = allocator->flushMemory(alloc.fBackendMemory, offset, size);
        gpu->checkVkResult(result);
    }
}

void GrVkMemory::InvalidateMappedAlloc(GrVkGpu* gpu, const GrVkAlloc& alloc,
                                       VkDeviceSize offset, VkDeviceSize size) {
    if (alloc.fFlags & GrVkAlloc::kNoncoherent_Flag) {
        SkASSERT(offset == 0);
        SkASSERT(size <= alloc.fSize);
        SkASSERT(alloc.fBackendMemory);
        GrVkMemoryAllocator* allocator = gpu->memoryAllocator();
        VkResult result = allocator->invalidateMemory(alloc.fBackendMemory, offset, size);
        gpu->checkVkResult(result);
    }
}

