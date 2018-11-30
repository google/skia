/*
* Copyright 2015 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrVkMemory_DEFINED
#define GrVkMemory_DEFINED

#include "GrVkVulkan.h"

#include "GrVkBuffer.h"
#include "SkTArray.h"
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
                                  bool dynamic,
                                  GrVkAlloc* alloc);
    void FreeBufferMemory(const GrVkGpu* gpu, GrVkBuffer::Type type, const GrVkAlloc& alloc);

    bool AllocAndBindImageMemory(const GrVkGpu* gpu,
                                 VkImage image,
                                 bool linearTiling,
                                 GrVkAlloc* alloc);
    void FreeImageMemory(const GrVkGpu* gpu, bool linearTiling, const GrVkAlloc& alloc);

    // Maps the entire GrVkAlloc and returns a pointer to the start of the allocation. Underneath
    // the hood, we may map more than the range of the GrVkAlloc (e.g. the entire VkDeviceMemory),
    // but the pointer returned will always be to the start of the GrVkAlloc. The caller should also
    // never assume more than the GrVkAlloc block has been mapped.
    void* MapAlloc(const GrVkGpu* gpu, const GrVkAlloc& alloc);
    void UnmapAlloc(const GrVkGpu* gpu, const GrVkAlloc& alloc);

    // For the Flush and Invalidate calls, the offset should be relative to the GrVkAlloc. Thus this
    // will often be 0. The client does not need to make sure the offset and size are aligned to the
    // nonCoherentAtomSize, the internal calls will handle that.
    void FlushMappedAlloc(const GrVkGpu* gpu, const GrVkAlloc& alloc, VkDeviceSize offset,
                          VkDeviceSize size);
    void InvalidateMappedAlloc(const GrVkGpu* gpu, const GrVkAlloc& alloc, VkDeviceSize offset,
                               VkDeviceSize size);

<<<<<<< HEAD   (ac7f23 SkQP: refatctor C++ bits.)
    // Helper for aligning and setting VkMappedMemoryRange for flushing/invalidating noncoherent
    // memory.
    void GetNonCoherentMappedMemoryRange(const GrVkAlloc&, VkDeviceSize offset, VkDeviceSize size,
                                         VkDeviceSize alignment, VkMappedMemoryRange*);
=======
    void FlushMappedAlloc(const GrVkGpu* gpu, const GrVkAlloc& alloc, VkDeviceSize offset,
                          VkDeviceSize size);
    void InvalidateMappedAlloc(const GrVkGpu* gpu, const GrVkAlloc& alloc, VkDeviceSize offset,
                               VkDeviceSize size);
>>>>>>> BRANCH (3e3428 SkQP: Remove tests that use too much RAM)
}

<<<<<<< HEAD   (ac7f23 SkQP: refatctor C++ bits.)
=======
class GrVkFreeListAlloc {
public:
    GrVkFreeListAlloc(VkDeviceSize size, VkDeviceSize alignment)
        : fSize(size)
        , fAlignment(alignment)
        , fFreeSize(size)
        , fLargestBlockSize(size)
        , fLargestBlockOffset(0) {
        Block* block = fFreeList.addToTail();
        block->fOffset = 0;
        block->fSize = fSize;
    }
    ~GrVkFreeListAlloc() {
        this->reset();
    }

    VkDeviceSize size() const { return fSize; }
    VkDeviceSize alignment() const { return fAlignment; }
    VkDeviceSize freeSize() const { return fFreeSize; }
    VkDeviceSize largestBlockSize() const { return fLargestBlockSize; }

    bool unallocated() const { return fSize == fFreeSize; }

protected:
    bool alloc(VkDeviceSize requestedSize, VkDeviceSize* allocOffset, VkDeviceSize* allocSize);
    void free(VkDeviceSize allocOffset, VkDeviceSize allocSize);

    void reset() {
        fSize = 0;
        fAlignment = 0;
        fFreeSize = 0;
        fLargestBlockSize = 0;
        fFreeList.reset();
    }

    struct Block {
        VkDeviceSize fOffset;
        VkDeviceSize fSize;
    };
    typedef SkTLList<Block, 16> FreeList;

    VkDeviceSize   fSize;
    VkDeviceSize   fAlignment;
    VkDeviceSize   fFreeSize;
    VkDeviceSize   fLargestBlockSize;
    VkDeviceSize   fLargestBlockOffset;
    FreeList       fFreeList;
};

class GrVkSubHeap : public GrVkFreeListAlloc {
public:
    GrVkSubHeap(const GrVkGpu* gpu, uint32_t memoryTypeIndex, uint32_t heapIndex,
                VkDeviceSize size, VkDeviceSize alignment);
    ~GrVkSubHeap();

    uint32_t memoryTypeIndex() const { return fMemoryTypeIndex; }
    VkDeviceMemory memory() { return fAlloc; }

    bool alloc(VkDeviceSize requestedSize, GrVkAlloc* alloc);
    void free(const GrVkAlloc& alloc);

private:
    const GrVkGpu* fGpu;
#ifdef SK_DEBUG
    uint32_t       fHeapIndex;
#endif
    uint32_t       fMemoryTypeIndex;
    VkDeviceMemory fAlloc;

    typedef GrVkFreeListAlloc INHERITED;
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

    ~GrVkHeap() {}

    VkDeviceSize allocSize() const { return fAllocSize; }
    VkDeviceSize usedSize() const { return fUsedSize; }

    bool alloc(VkDeviceSize size, VkDeviceSize alignment, uint32_t memoryTypeIndex,
               uint32_t heapIndex, GrVkAlloc* alloc) {
        SkASSERT(size > 0);
        alloc->fUsesSystemHeap = false;
        return (*this.*fAllocFunc)(size, alignment, memoryTypeIndex, heapIndex, alloc);
    }
    bool free(const GrVkAlloc& alloc);

private:
    typedef bool (GrVkHeap::*AllocFunc)(VkDeviceSize size, VkDeviceSize alignment,
                                        uint32_t memoryTypeIndex, uint32_t heapIndex,
                                        GrVkAlloc* alloc);

    bool subAlloc(VkDeviceSize size, VkDeviceSize alignment,
                  uint32_t memoryTypeIndex, uint32_t heapIndex,
                  GrVkAlloc* alloc);
    bool singleAlloc(VkDeviceSize size, VkDeviceSize alignment,
                     uint32_t memoryTypeIndex, uint32_t heapIndex,
                     GrVkAlloc* alloc);

    const GrVkGpu*         fGpu;
    VkDeviceSize           fSubHeapSize;
    VkDeviceSize           fAllocSize;
    VkDeviceSize           fUsedSize;
    AllocFunc              fAllocFunc;
    SkTArray<std::unique_ptr<GrVkSubHeap>> fSubHeaps;
};
>>>>>>> BRANCH (3e3428 SkQP: Remove tests that use too much RAM)
#endif
