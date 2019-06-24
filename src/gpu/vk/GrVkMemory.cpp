/*
* Copyright 2015 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/gpu/vk/GrVkMemory.h"

#include "include/gpu/vk/GrVkMemoryAllocator.h"
#include "src/gpu/vk/GrVkGpu.h"
#include "src/gpu/vk/GrVkUtil.h"

using AllocationPropertyFlags = GrVkMemoryAllocator::AllocationPropertyFlags;
using BufferUsage = GrVkMemoryAllocator::BufferUsage;

static BufferUsage get_buffer_usage(GrVkBuffer::Type type, bool dynamic) {
    switch (type) {
        case GrVkBuffer::kVertex_Type: // fall through
        case GrVkBuffer::kIndex_Type: // fall through
        case GrVkBuffer::kTexel_Type:
            return dynamic ? BufferUsage::kCpuWritesGpuReads : BufferUsage::kGpuOnly;
        case GrVkBuffer::kUniform_Type:
            SkASSERT(dynamic);
            return BufferUsage::kCpuWritesGpuReads;
        case GrVkBuffer::kCopyRead_Type: // fall through
        case GrVkBuffer::kCopyWrite_Type:
            return BufferUsage::kCpuOnly;
    }
    SK_ABORT("Invalid GrVkBuffer::Type");
    return BufferUsage::kCpuOnly; // Just returning an arbitrary value.
}

bool GrVkMemory::AllocAndBindBufferMemory(const GrVkGpu* gpu,
                                          VkBuffer buffer,
                                          GrVkBuffer::Type type,
                                          bool dynamic,
                                          GrVkAlloc* alloc) {
    GrVkMemoryAllocator* allocator = gpu->memoryAllocator();
    GrVkBackendMemory memory = 0;

    GrVkMemoryAllocator::BufferUsage usage = get_buffer_usage(type, dynamic);

    AllocationPropertyFlags propFlags;
    if (usage == GrVkMemoryAllocator::BufferUsage::kCpuWritesGpuReads) {
        // In general it is always fine (and often better) to keep buffers always mapped.
        // TODO: According to AMDs guide for the VulkanMemoryAllocator they suggest there are two
        // cases when keeping it mapped can hurt. The first is when running on Win7 or Win8 (Win 10
        // is fine). In general, by the time Vulkan ships it is probably less likely to be running
        // on non Win10 or newer machines. The second use case is if running on an AMD card and you
        // are using the special GPU local and host mappable memory. However, in general we don't
        // pick this memory as we've found it slower than using the cached host visible memory. In
        // the future if we find the need to special case either of these two issues we can add
        // checks for them here.
        propFlags = AllocationPropertyFlags::kPersistentlyMapped;
    } else {
        propFlags = AllocationPropertyFlags::kNone;
    }

    if (!allocator->allocateMemoryForBuffer(buffer, usage, propFlags, &memory)) {
        return false;
    }
    allocator->getAllocInfo(memory, alloc);

    // Bind buffer
    VkResult err = GR_VK_CALL(gpu->vkInterface(), BindBufferMemory(gpu->device(), buffer,
                                                                   alloc->fMemory,
                                                                   alloc->fOffset));
    if (err) {
        FreeBufferMemory(gpu, type, *alloc);
        return false;
    }

    return true;
}

void GrVkMemory::FreeBufferMemory(const GrVkGpu* gpu, GrVkBuffer::Type type,
                                  const GrVkAlloc& alloc) {
    if (alloc.fBackendMemory) {
        GrVkMemoryAllocator* allocator = gpu->memoryAllocator();
        allocator->freeMemory(alloc.fBackendMemory);
    } else {
        GR_VK_CALL(gpu->vkInterface(), FreeMemory(gpu->device(), alloc.fMemory, nullptr));
    }
}

const VkDeviceSize kMaxSmallImageSize = 256 * 1024;

bool GrVkMemory::AllocAndBindImageMemory(const GrVkGpu* gpu,
                                         VkImage image,
                                         bool linearTiling,
                                         GrVkAlloc* alloc) {
    SkASSERT(!linearTiling);
    GrVkMemoryAllocator* allocator = gpu->memoryAllocator();
    GrVkBackendMemory memory = 0;

    VkMemoryRequirements memReqs;
    GR_VK_CALL(gpu->vkInterface(), GetImageMemoryRequirements(gpu->device(), image, &memReqs));

    AllocationPropertyFlags propFlags;
    if (gpu->protectedContext()) {
        propFlags = AllocationPropertyFlags::kProtected;
    } else if (memReqs.size > kMaxSmallImageSize ||
               gpu->vkCaps().shouldAlwaysUseDedicatedImageMemory()) {
        propFlags = AllocationPropertyFlags::kDedicatedAllocation;
    } else {
        propFlags = AllocationPropertyFlags::kNone;
    }

    if (!allocator->allocateMemoryForImage(image, propFlags, &memory)) {
        return false;
    }
    allocator->getAllocInfo(memory, alloc);

    // Bind buffer
    VkResult err = GR_VK_CALL(gpu->vkInterface(), BindImageMemory(gpu->device(), image,
                                                                  alloc->fMemory, alloc->fOffset));
    if (err) {
        FreeImageMemory(gpu, linearTiling, *alloc);
        return false;
    }

    return true;
}

void GrVkMemory::FreeImageMemory(const GrVkGpu* gpu, bool linearTiling,
                                 const GrVkAlloc& alloc) {
    if (alloc.fBackendMemory) {
        GrVkMemoryAllocator* allocator = gpu->memoryAllocator();
        allocator->freeMemory(alloc.fBackendMemory);
    } else {
        GR_VK_CALL(gpu->vkInterface(), FreeMemory(gpu->device(), alloc.fMemory, nullptr));
    }
}

void* GrVkMemory::MapAlloc(const GrVkGpu* gpu, const GrVkAlloc& alloc) {
    SkASSERT(GrVkAlloc::kMappable_Flag & alloc.fFlags);
#ifdef SK_DEBUG
    if (alloc.fFlags & GrVkAlloc::kNoncoherent_Flag) {
        VkDeviceSize alignment = gpu->physicalDeviceProperties().limits.nonCoherentAtomSize;
        SkASSERT(0 == (alloc.fOffset & (alignment-1)));
        SkASSERT(0 == (alloc.fSize & (alignment-1)));
    }
#endif
    if (alloc.fBackendMemory) {
        GrVkMemoryAllocator* allocator = gpu->memoryAllocator();
        return allocator->mapMemory(alloc.fBackendMemory);
    }

    void* mapPtr;
    VkResult err = GR_VK_CALL(gpu->vkInterface(), MapMemory(gpu->device(), alloc.fMemory,
                                                            alloc.fOffset,
                                                            alloc.fSize, 0, &mapPtr));
    if (err) {
        mapPtr = nullptr;
    }
    return mapPtr;
}

void GrVkMemory::UnmapAlloc(const GrVkGpu* gpu, const GrVkAlloc& alloc) {
    if (alloc.fBackendMemory) {
        GrVkMemoryAllocator* allocator = gpu->memoryAllocator();
        allocator->unmapMemory(alloc.fBackendMemory);
    } else {
        GR_VK_CALL(gpu->vkInterface(), UnmapMemory(gpu->device(), alloc.fMemory));
    }
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

void GrVkMemory::FlushMappedAlloc(const GrVkGpu* gpu, const GrVkAlloc& alloc, VkDeviceSize offset,
                                  VkDeviceSize size) {
    if (alloc.fFlags & GrVkAlloc::kNoncoherent_Flag) {
        SkASSERT(offset == 0);
        SkASSERT(size <= alloc.fSize);
        if (alloc.fBackendMemory) {
            GrVkMemoryAllocator* allocator = gpu->memoryAllocator();
            allocator->flushMappedMemory(alloc.fBackendMemory, offset, size);
        } else {
            VkDeviceSize alignment = gpu->physicalDeviceProperties().limits.nonCoherentAtomSize;
            VkMappedMemoryRange mappedMemoryRange;
            GrVkMemory::GetNonCoherentMappedMemoryRange(alloc, offset, size, alignment,
                                                        &mappedMemoryRange);
            GR_VK_CALL(gpu->vkInterface(), FlushMappedMemoryRanges(gpu->device(), 1,
                                                                   &mappedMemoryRange));
        }
    }
}

void GrVkMemory::InvalidateMappedAlloc(const GrVkGpu* gpu, const GrVkAlloc& alloc,
                                       VkDeviceSize offset, VkDeviceSize size) {
    if (alloc.fFlags & GrVkAlloc::kNoncoherent_Flag) {
        SkASSERT(offset == 0);
        SkASSERT(size <= alloc.fSize);
        if (alloc.fBackendMemory) {
            GrVkMemoryAllocator* allocator = gpu->memoryAllocator();
            allocator->invalidateMappedMemory(alloc.fBackendMemory, offset, size);
        } else {
            VkDeviceSize alignment = gpu->physicalDeviceProperties().limits.nonCoherentAtomSize;
            VkMappedMemoryRange mappedMemoryRange;
            GrVkMemory::GetNonCoherentMappedMemoryRange(alloc, offset, size, alignment,
                                                        &mappedMemoryRange);
            GR_VK_CALL(gpu->vkInterface(), InvalidateMappedMemoryRanges(gpu->device(), 1,
                                                                        &mappedMemoryRange));
        }
    }
}

