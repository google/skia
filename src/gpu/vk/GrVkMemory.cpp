/*
* Copyright 2015 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "GrVkMemory.h"

#include "GrVkGpu.h"
#include "GrVkUtil.h"

static bool get_valid_memory_type_index(VkPhysicalDeviceMemoryProperties physDevMemProps,
                                        uint32_t typeBits,
                                        VkMemoryPropertyFlags requestedMemFlags,
                                        uint32_t* typeIndex) {
    uint32_t checkBit = 1;
    for (uint32_t i = 0; i < 32; ++i) {
        if (typeBits & checkBit) {
            uint32_t supportedFlags = physDevMemProps.memoryTypes[i].propertyFlags &
                                      requestedMemFlags;
            if (supportedFlags == requestedMemFlags) {
                *typeIndex = i;
                return true;
            }
        }
        checkBit <<= 1;
    }
    return false;
}

static GrVkGpu::Heap buffer_type_to_heap(GrVkBuffer::Type type) {
    const GrVkGpu::Heap kBufferToHeap[]{
        GrVkGpu::kVertexBuffer_Heap,
        GrVkGpu::kIndexBuffer_Heap,
        GrVkGpu::kUniformBuffer_Heap,
        GrVkGpu::kCopyReadBuffer_Heap,
        GrVkGpu::kCopyWriteBuffer_Heap,
    };
    GR_STATIC_ASSERT(0 == GrVkBuffer::kVertex_Type);
    GR_STATIC_ASSERT(1 == GrVkBuffer::kIndex_Type);
    GR_STATIC_ASSERT(2 == GrVkBuffer::kUniform_Type);
    GR_STATIC_ASSERT(3 == GrVkBuffer::kCopyRead_Type);
    GR_STATIC_ASSERT(4 == GrVkBuffer::kCopyWrite_Type);

    return kBufferToHeap[type];
}

bool GrVkMemory::AllocAndBindBufferMemory(const GrVkGpu* gpu,
                                          VkBuffer buffer,
                                          GrVkBuffer::Type type,
                                          GrVkAlloc* alloc) {
    const GrVkInterface* iface = gpu->vkInterface();
    VkDevice device = gpu->device();

    VkMemoryRequirements memReqs;
    GR_VK_CALL(iface, GetBufferMemoryRequirements(device, buffer, &memReqs));

    VkMemoryPropertyFlags desiredMemProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
                                            VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
    uint32_t typeIndex = 0;
    if (!get_valid_memory_type_index(gpu->physicalDeviceMemoryProperties(),
                                     memReqs.memoryTypeBits,
                                     desiredMemProps,
                                     &typeIndex)) {
        // this memory type should always be available
        SkASSERT_RELEASE(get_valid_memory_type_index(gpu->physicalDeviceMemoryProperties(),
                                                     memReqs.memoryTypeBits,
                                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                                     &typeIndex));
    }

    GrVkHeap* heap = gpu->getHeap(buffer_type_to_heap(type));

    if (!heap->alloc(memReqs.size, memReqs.alignment, typeIndex, alloc)) {
        SkDebugf("Failed to alloc buffer\n");
        return false;
    }

    // Bind Memory to device
    VkResult err = GR_VK_CALL(iface, BindBufferMemory(device, buffer, 
                                                      alloc->fMemory, alloc->fOffset));
    if (err) {
        SkASSERT_RELEASE(heap->free(*alloc));
        return false;
    }

    return true;
}

void GrVkMemory::FreeBufferMemory(const GrVkGpu* gpu, GrVkBuffer::Type type,
                                  const GrVkAlloc& alloc) {

    GrVkHeap* heap = gpu->getHeap(buffer_type_to_heap(type));
    SkASSERT_RELEASE(heap->free(alloc));
}

// for debugging
static uint64_t gTotalImageMemory = 0;
static uint64_t gTotalImageMemoryFullPage = 0;

const VkDeviceSize kMaxSmallImageSize = 16 * 1024;
const VkDeviceSize kMinVulkanPageSize = 16 * 1024;

static VkDeviceSize align_size(VkDeviceSize size, VkDeviceSize alignment) {
    return (size + alignment - 1) & ~(alignment - 1);
}

bool GrVkMemory::AllocAndBindImageMemory(const GrVkGpu* gpu,
                                         VkImage image,
                                         bool linearTiling,
                                         GrVkAlloc* alloc) {
    const GrVkInterface* iface = gpu->vkInterface();
    VkDevice device = gpu->device();

    VkMemoryRequirements memReqs;
    GR_VK_CALL(iface, GetImageMemoryRequirements(device, image, &memReqs));

    uint32_t typeIndex = 0;
    GrVkHeap* heap;
    if (linearTiling) {
        VkMemoryPropertyFlags desiredMemProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
                                                VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
        if (!get_valid_memory_type_index(gpu->physicalDeviceMemoryProperties(),
                                         memReqs.memoryTypeBits,
                                         desiredMemProps,
                                         &typeIndex)) {
            // this memory type should always be available
            SkASSERT_RELEASE(get_valid_memory_type_index(gpu->physicalDeviceMemoryProperties(),
                                                         memReqs.memoryTypeBits,
                                                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                                         &typeIndex));
        }
        heap = gpu->getHeap(GrVkGpu::kLinearImage_Heap);
    } else {
        // this memory type should always be available
        SkASSERT_RELEASE(get_valid_memory_type_index(gpu->physicalDeviceMemoryProperties(),
                                                     memReqs.memoryTypeBits,
                                                     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                                     &typeIndex));
        if (memReqs.size <= kMaxSmallImageSize) {
            heap = gpu->getHeap(GrVkGpu::kSmallOptimalImage_Heap);
        } else {
            heap = gpu->getHeap(GrVkGpu::kOptimalImage_Heap);
        }
    }

    if (!heap->alloc(memReqs.size, memReqs.alignment, typeIndex, alloc)) {
        SkDebugf("Failed to alloc image\n");
        return false;
    }

    // Bind Memory to device
    VkResult err = GR_VK_CALL(iface, BindImageMemory(device, image,
                              alloc->fMemory, alloc->fOffset));
    if (err) {
        SkASSERT_RELEASE(heap->free(*alloc));
        return false;
    }

    gTotalImageMemory += alloc->fSize;

    VkDeviceSize pageAlignedSize = align_size(alloc->fSize, kMinVulkanPageSize);
    gTotalImageMemoryFullPage += pageAlignedSize;

    return true;
}

void GrVkMemory::FreeImageMemory(const GrVkGpu* gpu, bool linearTiling,
                                 const GrVkAlloc& alloc) {
    GrVkHeap* heap;
    if (linearTiling) {
        heap = gpu->getHeap(GrVkGpu::kLinearImage_Heap);
    } else if (alloc.fSize <= kMaxSmallImageSize) {
        heap = gpu->getHeap(GrVkGpu::kSmallOptimalImage_Heap);
    } else {
        heap = gpu->getHeap(GrVkGpu::kOptimalImage_Heap);
    }
    if (!heap->free(alloc)) {
        // must be an adopted allocation
        GR_VK_CALL(gpu->vkInterface(), FreeMemory(gpu->device(), alloc.fMemory, nullptr));
    } else {
        gTotalImageMemory -= alloc.fSize;
        VkDeviceSize pageAlignedSize = align_size(alloc.fSize, kMinVulkanPageSize);
        gTotalImageMemoryFullPage -= pageAlignedSize;
    }
}

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
        return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
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

GrVkSubHeap::GrVkSubHeap(const GrVkGpu* gpu, uint32_t memoryTypeIndex, 
                         VkDeviceSize size, VkDeviceSize alignment)
    : fGpu(gpu)
    , fMemoryTypeIndex(memoryTypeIndex) {

    VkMemoryAllocateInfo allocInfo = {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,      // sType
        NULL,                                        // pNext
        size,                                        // allocationSize
        memoryTypeIndex,                             // memoryTypeIndex
    };

    VkResult err = GR_VK_CALL(gpu->vkInterface(), AllocateMemory(gpu->device(),
                                                                 &allocInfo,
                                                                 nullptr,
                                                                 &fAlloc));

    if (VK_SUCCESS == err) {
        fSize = size;
        fAlignment = alignment;
        fFreeSize = size;
        fLargestBlockSize = size;
        fLargestBlockOffset = 0;

        Block* block = fFreeList.addToTail();
        block->fOffset = 0;
        block->fSize = fSize;
    } else {
        fSize = 0;
        fAlignment = 0;
        fFreeSize = 0;
        fLargestBlockSize = 0;
    }
}

GrVkSubHeap::~GrVkSubHeap() {
    const GrVkInterface* iface = fGpu->vkInterface();
    GR_VK_CALL(iface, FreeMemory(fGpu->device(), fAlloc, nullptr));

    fFreeList.reset();
}

bool GrVkSubHeap::alloc(VkDeviceSize size, GrVkAlloc* alloc) {
    VkDeviceSize alignedSize = align_size(size, fAlignment);

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
        alloc->fMemory = fAlloc;
        SkASSERT(align_size(bestFit->fOffset, fAlignment) == bestFit->fOffset);
        alloc->fOffset = bestFit->fOffset;
        alloc->fSize = alignedSize;
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
#ifdef SK_DEBUG
            VkDeviceSize largestSize = 0;
            iter = fFreeList.headIter();
            while (iter.get()) {
                Block* block = iter.get();
                if (largestSize < block->fSize) {
                    largestSize = block->fSize;
                }
                iter.next();
            }
            SkASSERT(largestSize == fLargestBlockSize)
#endif
        } else {
            SkASSERT(bestFit->fSize == alignedSize);
            if (fLargestBlockOffset == originalBestFitOffset) {
                fLargestBlockSize = secondLargestSize;
                fLargestBlockOffset = secondLargestOffset;
            }
            fFreeList.remove(bestFit);
#ifdef SK_DEBUG
            VkDeviceSize largestSize = 0;
            iter = fFreeList.headIter();
            while (iter.get()) {
                Block* block = iter.get();
                if (largestSize < block->fSize) {
                    largestSize = block->fSize;
                }
                iter.next();
            }
            SkASSERT(largestSize == fLargestBlockSize);
#endif
        }
        fFreeSize -= alignedSize;
        SkASSERT(alloc->fSize > 0);

        return true;
    }
    
    SkDebugf("Can't allocate %d bytes, %d bytes available, largest free block %d\n", alignedSize, fFreeSize, fLargestBlockSize);

    return false;
}


void GrVkSubHeap::free(const GrVkAlloc& alloc) {
    SkASSERT(alloc.fMemory == fAlloc);

    // find the block right after this allocation
    FreeList::Iter iter = fFreeList.headIter();
    FreeList::Iter prev;
    while (iter.get() && iter.get()->fOffset < alloc.fOffset) {
        prev = iter;
        iter.next();
    } 
    // we have four cases:
    // we exactly follow the previous one
    Block* block;
    if (prev.get() && prev.get()->fOffset + prev.get()->fSize == alloc.fOffset) {
        block = prev.get();
        block->fSize += alloc.fSize;
        if (block->fOffset == fLargestBlockOffset) {
            fLargestBlockSize = block->fSize;
        }
        // and additionally we may exactly precede the next one
        if (iter.get() && iter.get()->fOffset == alloc.fOffset + alloc.fSize) {
            block->fSize += iter.get()->fSize;
            if (iter.get()->fOffset == fLargestBlockOffset) {
                fLargestBlockOffset = block->fOffset;
                fLargestBlockSize = block->fSize;
            }
            fFreeList.remove(iter.get());
        }
    // or we only exactly proceed the next one
    } else if (iter.get() && iter.get()->fOffset == alloc.fOffset + alloc.fSize) {
        block = iter.get();
        block->fSize += alloc.fSize;
        if (block->fOffset == fLargestBlockOffset) {
            fLargestBlockOffset = alloc.fOffset;
            fLargestBlockSize = block->fSize;
        }
        block->fOffset = alloc.fOffset;
    // or we fall somewhere in between, with gaps
    } else {
        block = fFreeList.addBefore(iter);
        block->fOffset = alloc.fOffset;
        block->fSize = alloc.fSize;
    }
    fFreeSize += alloc.fSize;
    if (block->fSize > fLargestBlockSize) {
        fLargestBlockSize = block->fSize;
        fLargestBlockOffset = block->fOffset;
    }

#ifdef SK_DEBUG
    VkDeviceSize   largestSize = 0;
    iter = fFreeList.headIter();
    while (iter.get()) {
        Block* block = iter.get();
        if (largestSize < block->fSize) {
            largestSize = block->fSize;
        }
        iter.next();
    }
    SkASSERT(fLargestBlockSize == largestSize);
#endif
}

GrVkHeap::~GrVkHeap() {
}

bool GrVkHeap::subAlloc(VkDeviceSize size, VkDeviceSize alignment, 
                        uint32_t memoryTypeIndex, GrVkAlloc* alloc) {
    VkDeviceSize alignedSize = align_size(size, alignment);

    // if requested is larger than our subheap allocation, just alloc directly
    if (alignedSize > fSubHeapSize) {
        VkMemoryAllocateInfo allocInfo = {
            VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,      // sType
            NULL,                                        // pNext
            size,                                        // allocationSize
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
        alloc->fSize = 0;    // hint that this is not a subheap allocation
        
        return true;
    }

    // first try to find a subheap that fits our allocation request
    int bestFitIndex = -1;
    VkDeviceSize bestFitSize = 0x7FFFFFFF;
    for (auto i = 0; i < fSubHeaps.count(); ++i) {
        if (fSubHeaps[i]->memoryTypeIndex() == memoryTypeIndex) {
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
    SkAutoTDelete<GrVkSubHeap>& subHeap = fSubHeaps.push_back();
    subHeap.reset(new GrVkSubHeap(fGpu, memoryTypeIndex, fSubHeapSize, alignment));
    // try to recover from failed allocation by only allocating what we need
    if (subHeap->size() == 0) {
        VkDeviceSize alignedSize = align_size(size, alignment);
        subHeap.reset(new GrVkSubHeap(fGpu, memoryTypeIndex, alignedSize, alignment));
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
                           uint32_t memoryTypeIndex, GrVkAlloc* alloc) {
    VkDeviceSize alignedSize = align_size(size, alignment);

    // first try to find an unallocated subheap that fits our allocation request
    int bestFitIndex = -1;
    VkDeviceSize bestFitSize = 0x7FFFFFFF;
    for (auto i = 0; i < fSubHeaps.count(); ++i) {
        if (fSubHeaps[i]->memoryTypeIndex() == memoryTypeIndex && fSubHeaps[i]->unallocated()) {
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
    SkAutoTDelete<GrVkSubHeap>& subHeap = fSubHeaps.push_back();
    subHeap.reset(new GrVkSubHeap(fGpu, memoryTypeIndex, alignedSize, alignment));
    fAllocSize += alignedSize;
    if (subHeap->alloc(size, alloc)) {
        fUsedSize += alloc->fSize;
        return true;
    }

    return false;
}

bool GrVkHeap::free(const GrVkAlloc& alloc) {
    // a size of 0 means we're using the system heap
    if (0 == alloc.fSize) {
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


