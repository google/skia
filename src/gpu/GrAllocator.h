/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrAllocator_DEFINED
#define GrAllocator_DEFINED

#include "GrConfig.h"
#include "GrTypes.h"
#include "SkTArray.h"
#include "SkTypes.h"

class GrAllocator : public SkNoncopyable {
public:
    ~GrAllocator() {
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
    GrAllocator(size_t itemSize, int itemsPerBlock, void* initialBlock) :
            fItemSize(itemSize),
            fItemsPerBlock(itemsPerBlock),
            fOwnFirstBlock(NULL == initialBlock),
            fCount(0) {
        SkASSERT(itemsPerBlock > 0);
        fBlockSize = fItemSize * fItemsPerBlock;
        fBlocks.push_back() = initialBlock;
        SkDEBUGCODE(if (!fOwnFirstBlock) {*((char*)initialBlock+fBlockSize-1)='a';} );
    }

    /*
     * Set first block of memory to write into.  Must be called before any other methods.
     * This requires that you have passed NULL in the constructor.
     *
     * @param   initialBlock    optional memory to use for the first block.
     *                          Must be at least itemSize*itemsPerBlock sized.
     *                          Caller is responsible for freeing this memory.
     */
    void setInitialBlock(void* initialBlock) {
        SkASSERT(0 == fCount);
        SkASSERT(1 == fBlocks.count());
        SkASSERT(NULL == fBlocks.back());
        fOwnFirstBlock = false;
        fBlocks.back() = initialBlock;
    }

    /**
     * Adds an item and returns pointer to it.
     *
     * @return pointer to the added item.
     */
    void* push_back() {
        int indexInBlock = fCount % fItemsPerBlock;
        // we always have at least one block
        if (0 == indexInBlock) {
            if (0 != fCount) {
                fBlocks.push_back() = sk_malloc_throw(fBlockSize);
            } else if (fOwnFirstBlock) {
                fBlocks[0] = sk_malloc_throw(fBlockSize);
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
        int blockCount = GrMax((unsigned)1,
                               GrUIDivRoundUp(fCount, fItemsPerBlock));
        for (int i = 1; i < blockCount; ++i) {
            sk_free(fBlocks[i]);
        }
        if (fOwnFirstBlock) {
            sk_free(fBlocks[0]);
            fBlocks[0] = NULL;
        }
        fBlocks.pop_back_n(blockCount-1);
        fCount = 0;
    }

    /**
     * count of items
     */
    int count() const {
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
        SkASSERT(fCount);
        return (*this)[fCount-1];
    }

    /**
     * access last item, only call if count() != 0
     */
    const void* back() const {
        SkASSERT(fCount);
        return (*this)[fCount-1];
    }

    /**
     * access item by index.
     */
    void* operator[] (int i) {
        SkASSERT(i >= 0 && i < fCount);
        return (char*)fBlocks[i / fItemsPerBlock] +
               fItemSize * (i % fItemsPerBlock);
    }

    /**
     * access item by index.
     */
    const void* operator[] (int i) const {
        SkASSERT(i >= 0 && i < fCount);
        return (const char*)fBlocks[i / fItemsPerBlock] +
               fItemSize * (i % fItemsPerBlock);
    }

private:
    static const int NUM_INIT_BLOCK_PTRS = 8;

    SkSTArray<NUM_INIT_BLOCK_PTRS, void*>   fBlocks;
    size_t                                  fBlockSize;
    size_t                                  fItemSize;
    int                                     fItemsPerBlock;
    bool                                    fOwnFirstBlock;
    int                                     fCount;

    typedef SkNoncopyable INHERITED;
};

template <typename T>
class GrTAllocator : public SkNoncopyable {
public:
    virtual ~GrTAllocator() { this->reset(); };

    /**
     * Create an allocator
     *
     * @param   itemsPerBlock   the number of items to allocate at once
     */
    explicit GrTAllocator(int itemsPerBlock)
        : fAllocator(sizeof(T), itemsPerBlock, NULL) {}

    /**
     * Adds an item and returns it.
     *
     * @return the added item.
     */
    T& push_back() {
        void* item = fAllocator.push_back();
        SkASSERT(NULL != item);
        SkNEW_PLACEMENT(item, T);
        return *(T*)item;
    }

    T& push_back(const T& t) {
        void* item = fAllocator.push_back();
        SkASSERT(NULL != item);
        SkNEW_PLACEMENT_ARGS(item, T, (t));
        return *(T*)item;
    }

    /**
     * removes all added items
     */
    void reset() {
        int c = fAllocator.count();
        for (int i = 0; i < c; ++i) {
            ((T*)fAllocator[i])->~T();
        }
        fAllocator.reset();
    }

    /**
     * count of items
     */
    int count() const {
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
    T& operator[] (int i) {
        return *(T*)(fAllocator[i]);
    }

    /**
     * access item by index.
     */
    const T& operator[] (int i) const {
        return *(const T*)(fAllocator[i]);
    }

protected:
    /*
     * Set first block of memory to write into.  Must be called before any other methods.
     *
     * @param   initialBlock    optional memory to use for the first block.
     *                          Must be at least size(T)*itemsPerBlock sized.
     *                          Caller is responsible for freeing this memory.
     */
    void setInitialBlock(void* initialBlock) {
        fAllocator.setInitialBlock(initialBlock);
    }

private:
    GrAllocator fAllocator;
    typedef SkNoncopyable INHERITED;
};

template <int N, typename T> class GrSTAllocator : public GrTAllocator<T> {
private:
    typedef GrTAllocator<T> INHERITED;

public:
    GrSTAllocator() : INHERITED(N) {
        this->setInitialBlock(fStorage.get());
    }

private:
    SkAlignedSTStorage<N, T> fStorage;
};

#endif
