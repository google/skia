/*
* Copyright 2015 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "GrVkMemory.h"

#include "GrVkGpu.h"
#include "GrVkUtil.h"
#include "vk/GrVkMemoryAllocator.h"

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

<<<<<<< HEAD   (ac7f23 SkQP: refatctor C++ bits.)
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
=======
    uint32_t typeIndex = 0;
    uint32_t heapIndex = 0;
    const VkPhysicalDeviceMemoryProperties& phDevMemProps = gpu->physicalDeviceMemoryProperties();
    const VkPhysicalDeviceProperties& phDevProps = gpu->physicalDeviceProperties();
    if (dynamic) {
        // try to get cached and ideally non-coherent memory first
        if (!get_valid_memory_type_index(phDevMemProps,
                                         memReqs.memoryTypeBits,
                                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                         VK_MEMORY_PROPERTY_HOST_CACHED_BIT,
                                         &typeIndex,
                                         &heapIndex)) {
            // some sort of host-visible memory type should always be available for dynamic buffers
            SkASSERT_RELEASE(get_valid_memory_type_index(phDevMemProps,
                                                         memReqs.memoryTypeBits,
                                                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                                                         &typeIndex,
                                                         &heapIndex));
        }

        VkMemoryPropertyFlags mpf = phDevMemProps.memoryTypes[typeIndex].propertyFlags;
        alloc->fFlags = mpf & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT ? 0x0
                                                                   : GrVkAlloc::kNoncoherent_Flag;
        if (SkToBool(alloc->fFlags & GrVkAlloc::kNoncoherent_Flag)) {
            SkASSERT(SkIsPow2(memReqs.alignment));
            SkASSERT(SkIsPow2(phDevProps.limits.nonCoherentAtomSize));
            memReqs.alignment = SkTMax(memReqs.alignment, phDevProps.limits.nonCoherentAtomSize);
        }
>>>>>>> BRANCH (3e3428 SkQP: Remove tests that use too much RAM)
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

const VkDeviceSize kMaxSmallImageSize = 16 * 1024;

bool GrVkMemory::AllocAndBindImageMemory(const GrVkGpu* gpu,
                                         VkImage image,
                                         bool linearTiling,
                                         GrVkAlloc* alloc) {
    SkASSERT(!linearTiling);
    GrVkMemoryAllocator* allocator = gpu->memoryAllocator();
    GrVkBackendMemory memory = 0;

    VkMemoryRequirements memReqs;
    GR_VK_CALL(gpu->vkInterface(), GetImageMemoryRequirements(gpu->device(), image, &memReqs));

<<<<<<< HEAD   (ac7f23 SkQP: refatctor C++ bits.)
    AllocationPropertyFlags propFlags;
    if (memReqs.size > kMaxSmallImageSize || gpu->vkCaps().shouldAlwaysUseDedicatedImageMemory()) {
        propFlags = AllocationPropertyFlags::kDedicatedAllocation;
=======
    uint32_t typeIndex = 0;
    uint32_t heapIndex = 0;
    GrVkHeap* heap;
    const VkPhysicalDeviceMemoryProperties& phDevMemProps = gpu->physicalDeviceMemoryProperties();
    const VkPhysicalDeviceProperties& phDevProps = gpu->physicalDeviceProperties();
    if (linearTiling) {
        VkMemoryPropertyFlags desiredMemProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
        if (!get_valid_memory_type_index(phDevMemProps,
                                         memReqs.memoryTypeBits,
                                         desiredMemProps,
                                         &typeIndex,
                                         &heapIndex)) {
            // some sort of host-visible memory type should always be available
            SkASSERT_RELEASE(get_valid_memory_type_index(phDevMemProps,
                                                         memReqs.memoryTypeBits,
                                                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                                                         &typeIndex,
                                                         &heapIndex));
        }
        heap = gpu->getHeap(GrVkGpu::kLinearImage_Heap);
        VkMemoryPropertyFlags mpf = phDevMemProps.memoryTypes[typeIndex].propertyFlags;
        alloc->fFlags = mpf & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT ? 0x0
                                                                   : GrVkAlloc::kNoncoherent_Flag;
        if (SkToBool(alloc->fFlags & GrVkAlloc::kNoncoherent_Flag)) {
            SkASSERT(SkIsPow2(memReqs.alignment));
            SkASSERT(SkIsPow2(phDevProps.limits.nonCoherentAtomSize));
            memReqs.alignment = SkTMax(memReqs.alignment, phDevProps.limits.nonCoherentAtomSize);
        }
>>>>>>> BRANCH (3e3428 SkQP: Remove tests that use too much RAM)
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

<<<<<<< HEAD   (ac7f23 SkQP: refatctor C++ bits.)
void* GrVkMemory::MapAlloc(const GrVkGpu* gpu, const GrVkAlloc& alloc) {
    SkASSERT(GrVkAlloc::kMappable_Flag & alloc.fFlags);
=======
VkPipelineStageFlags GrVkMemory::LayoutToPipelineStageFlags(const VkImageLayout layout) {
    if (VK_IMAGE_LAYOUT_GENERAL == layout) {
        return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    } else if (VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL == layout ||
               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL == layout) {
        return VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL == layout ||
               VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL == layout ||
               VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL == layout ||
               VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL == layout) {
        return VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
    } else if (VK_IMAGE_LAYOUT_PREINITIALIZED == layout) {
        return VK_PIPELINE_STAGE_HOST_BIT;
    }

    SkASSERT(VK_IMAGE_LAYOUT_UNDEFINED == layout);
    return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
}

VkAccessFlags GrVkMemory::LayoutToSrcAccessMask(const VkImageLayout layout) {
    // Currently we assume we will never being doing any explict shader writes (this doesn't include
    // color attachment or depth/stencil writes). So we will ignore the
    // VK_MEMORY_OUTPUT_SHADER_WRITE_BIT.

    // We can only directly access the host memory if we are in preinitialized or general layout,
    // and the image is linear.
    // TODO: Add check for linear here so we are not always adding host to general, and we should
    //       only be in preinitialized if we are linear
    VkAccessFlags flags = 0;;
    if (VK_IMAGE_LAYOUT_GENERAL == layout) {
        flags = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
                VK_ACCESS_TRANSFER_WRITE_BIT |
                VK_ACCESS_TRANSFER_READ_BIT |
                VK_ACCESS_SHADER_READ_BIT |
                VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_HOST_READ_BIT;
    } else if (VK_IMAGE_LAYOUT_PREINITIALIZED == layout) {
        flags = VK_ACCESS_HOST_WRITE_BIT;
    } else if (VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL == layout) {
        flags = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    } else if (VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL == layout) {
        flags = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    } else if (VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL == layout) {
        flags = VK_ACCESS_TRANSFER_WRITE_BIT;
    } else if (VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL == layout) {
        flags = VK_ACCESS_TRANSFER_READ_BIT;
    } else if (VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL == layout) {
        flags = VK_ACCESS_SHADER_READ_BIT;
    }
    return flags;
}

void GrVkMemory::FlushMappedAlloc(const GrVkGpu* gpu, const GrVkAlloc& alloc, VkDeviceSize offset,
                                  VkDeviceSize size) {
    if (alloc.fFlags & GrVkAlloc::kNoncoherent_Flag) {
#ifdef SK_DEBUG
        SkASSERT(offset >= alloc.fOffset);
        VkDeviceSize alignment = gpu->physicalDeviceProperties().limits.nonCoherentAtomSize;
        SkASSERT(0 == (offset & (alignment-1)));
        if (size != VK_WHOLE_SIZE) {
            SkASSERT(size > 0);
            SkASSERT(0 == (size & (alignment-1)) ||
                     (offset + size) == (alloc.fOffset + alloc.fSize));
            SkASSERT(offset + size <= alloc.fOffset + alloc.fSize);
        }
#endif

        VkMappedMemoryRange mappedMemoryRange;
        memset(&mappedMemoryRange, 0, sizeof(VkMappedMemoryRange));
        mappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        mappedMemoryRange.memory = alloc.fMemory;
        mappedMemoryRange.offset = offset;
        mappedMemoryRange.size = size;
        GR_VK_CALL(gpu->vkInterface(), FlushMappedMemoryRanges(gpu->device(),
                                                               1, &mappedMemoryRange));
    }
}

void GrVkMemory::InvalidateMappedAlloc(const GrVkGpu* gpu, const GrVkAlloc& alloc,
                                       VkDeviceSize offset, VkDeviceSize size) {
    if (alloc.fFlags & GrVkAlloc::kNoncoherent_Flag) {
#ifdef SK_DEBUG
        SkASSERT(offset >= alloc.fOffset);
        VkDeviceSize alignment = gpu->physicalDeviceProperties().limits.nonCoherentAtomSize;
        SkASSERT(0 == (offset & (alignment-1)));
        if (size != VK_WHOLE_SIZE) {
            SkASSERT(size > 0);
            SkASSERT(0 == (size & (alignment-1)) ||
                     (offset + size) == (alloc.fOffset + alloc.fSize));
            SkASSERT(offset + size <= alloc.fOffset + alloc.fSize);
        }
#endif

        VkMappedMemoryRange mappedMemoryRange;
        memset(&mappedMemoryRange, 0, sizeof(VkMappedMemoryRange));
        mappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        mappedMemoryRange.memory = alloc.fMemory;
        mappedMemoryRange.offset = offset;
        mappedMemoryRange.size = size;
        GR_VK_CALL(gpu->vkInterface(), InvalidateMappedMemoryRanges(gpu->device(),
                                                               1, &mappedMemoryRange));
    }
}

bool GrVkFreeListAlloc::alloc(VkDeviceSize requestedSize,
                              VkDeviceSize* allocOffset, VkDeviceSize* allocSize) {
    VkDeviceSize alignedSize = align_size(requestedSize, fAlignment);

    // find the smallest block big enough for our allocation
    FreeList::Iter iter = fFreeList.headIter();
    FreeList::Iter bestFitIter;
    VkDeviceSize   bestFitSize = fSize + 1;
    VkDeviceSize   secondLargestSize = 0;
    VkDeviceSize   secondLargestOffset = 0;
    while (iter.get()) {
        Block* block = iter.get();
        // need to adjust size to match desired alignment
        SkASSERT(align_size(block->fOffset, fAlignment) - block->fOffset == 0);
        if (block->fSize >= alignedSize && block->fSize < bestFitSize) {
            bestFitIter = iter;
            bestFitSize = block->fSize;
        }
        if (secondLargestSize < block->fSize && block->fOffset != fLargestBlockOffset) {
            secondLargestSize = block->fSize;
            secondLargestOffset = block->fOffset;
        }
        iter.next();
    }
    SkASSERT(secondLargestSize <= fLargestBlockSize);

    Block* bestFit = bestFitIter.get();
    if (bestFit) {
        SkASSERT(align_size(bestFit->fOffset, fAlignment) == bestFit->fOffset);
        *allocOffset = bestFit->fOffset;
        *allocSize = alignedSize;
        // adjust or remove current block
        VkDeviceSize originalBestFitOffset = bestFit->fOffset;
        if (bestFit->fSize > alignedSize) {
            bestFit->fOffset += alignedSize;
            bestFit->fSize -= alignedSize;
            if (fLargestBlockOffset == originalBestFitOffset) {
                if (bestFit->fSize >= secondLargestSize) {
                    fLargestBlockSize = bestFit->fSize;
                    fLargestBlockOffset = bestFit->fOffset;
                } else {
                    fLargestBlockSize = secondLargestSize;
                    fLargestBlockOffset = secondLargestOffset;
                }
            }
>>>>>>> BRANCH (3e3428 SkQP: Remove tests that use too much RAM)
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

<<<<<<< HEAD   (ac7f23 SkQP: refatctor C++ bits.)
=======
GrVkSubHeap::GrVkSubHeap(const GrVkGpu* gpu, uint32_t memoryTypeIndex, uint32_t heapIndex,
                         VkDeviceSize size, VkDeviceSize alignment)
    : INHERITED(size, alignment)
    , fGpu(gpu)
#ifdef SK_DEBUG
    , fHeapIndex(heapIndex)
#endif
    , fMemoryTypeIndex(memoryTypeIndex) {

    VkMemoryAllocateInfo allocInfo = {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,      // sType
        nullptr,                                     // pNext
        size,                                        // allocationSize
        memoryTypeIndex,                             // memoryTypeIndex
    };

    VkResult err = GR_VK_CALL(gpu->vkInterface(), AllocateMemory(gpu->device(),
                                                                 &allocInfo,
                                                                 nullptr,
                                                                 &fAlloc));
    if (VK_SUCCESS != err) {
        this->reset();
    }
#ifdef SK_DEBUG
    else {
        gHeapUsage[heapIndex] += size;
    }
#endif
}

GrVkSubHeap::~GrVkSubHeap() {
    const GrVkInterface* iface = fGpu->vkInterface();
    GR_VK_CALL(iface, FreeMemory(fGpu->device(), fAlloc, nullptr));
#ifdef SK_DEBUG
    gHeapUsage[fHeapIndex] -= fSize;
#endif
}

bool GrVkSubHeap::alloc(VkDeviceSize size, GrVkAlloc* alloc) {
    alloc->fMemory = fAlloc;
    return INHERITED::alloc(size, &alloc->fOffset, &alloc->fSize);
}

void GrVkSubHeap::free(const GrVkAlloc& alloc) {
    SkASSERT(alloc.fMemory == fAlloc);

    INHERITED::free(alloc.fOffset, alloc.fSize);
}

bool GrVkHeap::subAlloc(VkDeviceSize size, VkDeviceSize alignment,
                        uint32_t memoryTypeIndex, uint32_t heapIndex, GrVkAlloc* alloc) {
    VkDeviceSize alignedSize = align_size(size, alignment);

    // if requested is larger than our subheap allocation, just alloc directly
    if (alignedSize > fSubHeapSize) {
        VkMemoryAllocateInfo allocInfo = {
            VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,      // sType
            nullptr,                                     // pNext
            alignedSize,                                 // allocationSize
            memoryTypeIndex,                             // memoryTypeIndex
        };

        VkResult err = GR_VK_CALL(fGpu->vkInterface(), AllocateMemory(fGpu->device(),
                                                                      &allocInfo,
                                                                      nullptr,
                                                                      &alloc->fMemory));
        if (VK_SUCCESS != err) {
            return false;
        }
        alloc->fOffset = 0;
        alloc->fSize = alignedSize;
        alloc->fUsesSystemHeap = true;
#ifdef SK_DEBUG
        gHeapUsage[VK_MAX_MEMORY_HEAPS] += alignedSize;
#endif

        return true;
    }

    // first try to find a subheap that fits our allocation request
    int bestFitIndex = -1;
    VkDeviceSize bestFitSize = 0x7FFFFFFF;
    for (auto i = 0; i < fSubHeaps.count(); ++i) {
        if (fSubHeaps[i]->memoryTypeIndex() == memoryTypeIndex &&
            fSubHeaps[i]->alignment() == alignment) {
            VkDeviceSize heapSize = fSubHeaps[i]->largestBlockSize();
            if (heapSize >= alignedSize && heapSize < bestFitSize) {
                bestFitIndex = i;
                bestFitSize = heapSize;
            }
        }
    }

    if (bestFitIndex >= 0) {
        SkASSERT(fSubHeaps[bestFitIndex]->alignment() == alignment);
        if (fSubHeaps[bestFitIndex]->alloc(size, alloc)) {
            fUsedSize += alloc->fSize;
            return true;
        }
        return false;
    }

    // need to allocate a new subheap
    std::unique_ptr<GrVkSubHeap>& subHeap = fSubHeaps.push_back();
    subHeap.reset(new GrVkSubHeap(fGpu, memoryTypeIndex, heapIndex, fSubHeapSize, alignment));
    // try to recover from failed allocation by only allocating what we need
    if (subHeap->size() == 0) {
        VkDeviceSize alignedSize = align_size(size, alignment);
        subHeap.reset(new GrVkSubHeap(fGpu, memoryTypeIndex, heapIndex, alignedSize, alignment));
        if (subHeap->size() == 0) {
            return false;
        }
    }
    fAllocSize += fSubHeapSize;
    if (subHeap->alloc(size, alloc)) {
        fUsedSize += alloc->fSize;
        return true;
    }

    return false;
}

bool GrVkHeap::singleAlloc(VkDeviceSize size, VkDeviceSize alignment,
                           uint32_t memoryTypeIndex, uint32_t heapIndex, GrVkAlloc* alloc) {
    VkDeviceSize alignedSize = align_size(size, alignment);

    // first try to find an unallocated subheap that fits our allocation request
    int bestFitIndex = -1;
    VkDeviceSize bestFitSize = 0x7FFFFFFF;
    for (auto i = 0; i < fSubHeaps.count(); ++i) {
        if (fSubHeaps[i]->memoryTypeIndex() == memoryTypeIndex &&
            fSubHeaps[i]->alignment() == alignment &&
            fSubHeaps[i]->unallocated()) {
            VkDeviceSize heapSize = fSubHeaps[i]->size();
            if (heapSize >= alignedSize && heapSize < bestFitSize) {
                bestFitIndex = i;
                bestFitSize = heapSize;
            }
        }
    }

    if (bestFitIndex >= 0) {
        SkASSERT(fSubHeaps[bestFitIndex]->alignment() == alignment);
        if (fSubHeaps[bestFitIndex]->alloc(size, alloc)) {
            fUsedSize += alloc->fSize;
            return true;
        }
        return false;
    }

    // need to allocate a new subheap
    std::unique_ptr<GrVkSubHeap>& subHeap = fSubHeaps.push_back();
    subHeap.reset(new GrVkSubHeap(fGpu, memoryTypeIndex, heapIndex, alignedSize, alignment));
    fAllocSize += alignedSize;
    if (subHeap->alloc(size, alloc)) {
        fUsedSize += alloc->fSize;
        return true;
    }

    return false;
}

bool GrVkHeap::free(const GrVkAlloc& alloc) {
    // a size of 0 means we're using the system heap
    if (alloc.fUsesSystemHeap) {
        const GrVkInterface* iface = fGpu->vkInterface();
        GR_VK_CALL(iface, FreeMemory(fGpu->device(), alloc.fMemory, nullptr));
        return true;
    }

    for (auto i = 0; i < fSubHeaps.count(); ++i) {
        if (fSubHeaps[i]->memory() == alloc.fMemory) {
            fSubHeaps[i]->free(alloc);
            fUsedSize -= alloc.fSize;
            return true;
        }
    }

    return false;
}


>>>>>>> BRANCH (3e3428 SkQP: Remove tests that use too much RAM)
