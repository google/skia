/*
* Copyright 2015 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrVkMemory_DEFINED
#define GrVkMemory_DEFINED

#include "GrVkBuffer.h"
#include "SkTArray.h"
#include "SkTLList.h"
#include "vk/GrVkDefines.h"
#include "vk/GrVkTypes.h"

class GrVkGpu;

namespace GrVkMemory {
    /**
    * Allocates vulkan device memory and binds it to the gpu's device for the given object.
    * Returns true if allocation succeeded.
    */
    bool AllocAndBindBufferMemory(const GrVkGpu* gpu,
                                  VkBuffer buffer,
                                  GrVkBuffer::Type type,
                                  GrVkAlloc* alloc);
    void FreeBufferMemory(const GrVkGpu* gpu, GrVkBuffer::Type type, const GrVkAlloc& alloc);

    bool AllocAndBindImageMemory(const GrVkGpu* gpu,
                                 VkImage image,
                                 bool linearTiling,
                                 GrVkAlloc* alloc);
    void FreeImageMemory(const GrVkGpu* gpu, bool linearTiling, const GrVkAlloc& alloc);

    VkPipelineStageFlags LayoutToPipelineStageFlags(const VkImageLayout layout);

    VkAccessFlags LayoutToSrcAccessMask(const VkImageLayout layout);
}

class GrVkSubHeap {
public:
    GrVkSubHeap(const GrVkGpu* gpu, uint32_t memoryTypeIndex, 
                VkDeviceSize size, VkDeviceSize alignment);
    ~GrVkSubHeap();

    uint32_t  memoryTypeIndex() const { return fMemoryTypeIndex;  }
    VkDeviceSize size() const { return fSize; }
    VkDeviceSize alignment() const { return fAlignment; }
    VkDeviceSize freeSize() const { return fFreeSize; }
    VkDeviceSize largestBlockSize() const { return fLargestBlockSize; }
    VkDeviceMemory memory() { return fAlloc; }

    bool unallocated() const { return fSize == fFreeSize; }

    bool alloc(VkDeviceSize size, GrVkAlloc* alloc);
    void free(const GrVkAlloc& alloc);

private:
    struct Block {
        VkDeviceSize fOffset;
        VkDeviceSize fSize;
    };
    typedef SkTLList<Block, 16> FreeList;

    const GrVkGpu* fGpu;
    uint32_t       fMemoryTypeIndex;
    VkDeviceSize   fSize;
    VkDeviceSize   fAlignment;
    VkDeviceSize   fFreeSize;
    VkDeviceSize   fLargestBlockSize;
    VkDeviceSize   fLargestBlockOffset;
    VkDeviceMemory fAlloc;
    FreeList       fFreeList;
};

class GrVkHeap {
public:
    enum Strategy {
        kSubAlloc_Strategy,       // alloc large subheaps and suballoc within them
        kSingleAlloc_Strategy     // alloc/recycle an individual subheap per object
    };

    GrVkHeap(const GrVkGpu* gpu, Strategy strategy, VkDeviceSize subHeapSize)
        : fGpu(gpu)
        , fSubHeapSize(subHeapSize)
        , fAllocSize(0)
        , fUsedSize(0) {
        if (strategy == kSubAlloc_Strategy) {
            fAllocFunc = &GrVkHeap::subAlloc;
        } else {
            fAllocFunc = &GrVkHeap::singleAlloc;
        }
    }

    ~GrVkHeap();

    VkDeviceSize allocSize() const { return fAllocSize; }
    VkDeviceSize usedSize() const { return fUsedSize; }

    bool alloc(VkDeviceSize size, VkDeviceSize alignment, uint32_t memoryTypeIndex, 
               GrVkAlloc* alloc) {
        SkASSERT(size > 0);
        return (*this.*fAllocFunc)(size, alignment, memoryTypeIndex, alloc);
    }
    bool free(const GrVkAlloc& alloc);

private:
    typedef bool (GrVkHeap::*AllocFunc)(VkDeviceSize size, VkDeviceSize alignment, 
                                        uint32_t memoryTypeIndex, GrVkAlloc* alloc);

    bool subAlloc(VkDeviceSize size, VkDeviceSize alignment, 
                  uint32_t memoryTypeIndex, GrVkAlloc* alloc);
    bool singleAlloc(VkDeviceSize size, VkDeviceSize alignment,
                     uint32_t memoryTypeIndex, GrVkAlloc* alloc);

    const GrVkGpu*         fGpu;
    VkDeviceSize           fSubHeapSize;
    VkDeviceSize           fAllocSize;
    VkDeviceSize           fUsedSize;
    AllocFunc              fAllocFunc;
    SkTArray<SkAutoTDelete<GrVkSubHeap>> fSubHeaps;
};
#endif
