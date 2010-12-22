/*
    Copyright 2010 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

         http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */


#ifndef GrAllocator_DEFINED
#define GrAllocator_DEFINED

#include "GrConfig.h"
#include "GrTArray.h"

class GrAllocator {
public:
    virtual ~GrAllocator() {
        reset();
    }

    /**
     * Create an allocator
     *
     * @param   itemSize        the size of each item to allocate
     * @param   itemsPerBlock   the number of items to allocate at once
     * @param   initialBlock    optional memory to use for the first block.
     *                          Must be at least itemSize*itemsPerBlock sized.
     *                          Caller is responsible for freeing this memory.
     */
    GrAllocator(size_t itemSize, uint32_t itemsPerBlock, void* initialBlock) :
            fBlocks(fBlockInitialStorage, NUM_INIT_BLOCK_PTRS),
            fItemSize(itemSize),
            fItemsPerBlock(itemsPerBlock),
            fOwnFirstBlock(NULL == initialBlock),
            fCount(0) {
        fBlockSize = fItemSize * fItemsPerBlock;
        fBlocks.push_back() = initialBlock;
        GR_DEBUGCODE(if (!fOwnFirstBlock) {*((char*)initialBlock+fBlockSize-1)='a';} );
    }

    /**
     * Adds an item and returns pointer to it.
     *
     * @return pointer to the added item.
     */
    void* push_back() {        
        uint32_t indexInBlock = fCount % fItemsPerBlock;
        // we always have at least one block
        if (0 == indexInBlock) {
            if (0 != fCount) {
                fBlocks.push_back() = GrMalloc(fBlockSize);
            } else if (fOwnFirstBlock) {
                fBlocks[0] = GrMalloc(fBlockSize);
            }
        }        
        void* ret = (char*)fBlocks[fCount/fItemsPerBlock] + 
                    fItemSize * indexInBlock;
        ++fCount;
        return ret;
    }

    /**
     * removes all added items
     */
    void reset() {        
        uint32_t blockCount = GrMax((unsigned)1, 
                                    GrUIDivRoundUp(fCount, fItemsPerBlock));
        for (uint32_t i = 1; i < blockCount; ++i) {
            GrFree(fBlocks[i]);
        }
        if (fOwnFirstBlock) {
            GrFree(fBlocks[0]);
            fBlocks[0] = NULL;
        }
        fBlocks.pop_back_n(blockCount-1);
        fCount = 0;
    }

    /**
     * count of items
     */
    uint32_t count() const {
        return fCount;
    }
    
    /**
     * is the count 0
     */
    bool empty() const { return fCount == 0; }
    
    /**
     * access last item, only call if count() != 0
     */
    void* back() {
        GrAssert(fCount);
        return (*this)[fCount-1];
    }
    
    /**
     * access last item, only call if count() != 0
     */
    const void* back() const {
        GrAssert(fCount);
        return (*this)[fCount-1];
    }
    
    /**
     * access item by index.
     */    
    void* operator[] (uint32_t i) {
        GrAssert(i < fCount);
        return (char*)fBlocks[i / fItemsPerBlock] + 
               fItemSize * (i % fItemsPerBlock);
    }

    /**
     * access item by index.
     */  
    const void* operator[] (uint32_t i) const {
        GrAssert(i < fCount);
        return (const char*)fBlocks[i / fItemsPerBlock] + 
               fItemSize * (i % fItemsPerBlock);
    }

private:
    static const uint32_t NUM_INIT_BLOCK_PTRS = 8;
    
    GrTArray<void*> fBlocks;
    size_t          fBlockSize;    
    char            fBlockInitialStorage[NUM_INIT_BLOCK_PTRS*sizeof(void*)];    
    size_t          fItemSize;
    uint32_t        fItemsPerBlock;
    bool            fOwnFirstBlock;
    uint32_t        fCount;    
};

template <typename T>
class GrTAllocator {
private:
    GrAllocator fAllocator;
    
public:
    virtual ~GrTAllocator() {};
    
    /**
     * Create an allocator
     *
     * @param   itemsPerBlock   the number of items to allocate at once
     * @param   initialBlock    optional memory to use for the first block.
     *                          Must be at least size(T)*itemsPerBlock sized.
     *                          Caller is responsible for freeing this memory.
     */
    GrTAllocator(uint32_t itemsPerBlock, void* initialBlock) :
        fAllocator(sizeof(T), itemsPerBlock, initialBlock)
    {}
    
    /**
     * Adds an item and returns it.
     *
     * @return the added item.
     */
    T& push_back() {
        void* item = fAllocator.push_back();
        GrAssert(NULL != item);
        new (item) T;
        return *(T*)item;
    }
    
    /**
     * removes all added items
     */
    void reset() {
        uint32_t c = fAllocator.count();
        for (uint32_t i = 0; i < c; ++i) {
            ((T*)fAllocator[i])->~T();
        }
        fAllocator.reset();
    }
    
    /**
     * count of items
     */
    uint32_t count() const {
        return fAllocator.count();
    }
    
    /**
     * is the count 0
     */
    bool empty() const { return fAllocator.empty(); }
    
    /**
     * access last item, only call if count() != 0
     */
    T& back() {
        return *(T*)fAllocator.back();
    }

    /**
     * access last item, only call if count() != 0
     */
    const T& back() const {
        return *(const T*)fAllocator.back();
    }

    /**
     * access item by index.
     */  
    T& operator[] (uint32_t i) {
        return *(T*)(fAllocator[i]);
    }
    
    /**
     * access item by index.
     */
    const T& operator[] (uint32_t i) const {
        return *(const T*)(fAllocator[i]);
    }    
};

#endif
