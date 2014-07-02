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

class GrAllocator : SkNoncopyable {
public:
    ~GrAllocator() { this->reset(); }

    /**
     * Create an allocator
     *
     * @param   itemSize        the size of each item to allocate
     * @param   itemsPerBlock   the number of items to allocate at once
     * @param   initialBlock    optional memory to use for the first block.
     *                          Must be at least itemSize*itemsPerBlock sized.
     *                          Caller is responsible for freeing this memory.
     */
    GrAllocator(size_t itemSize, int itemsPerBlock, void* initialBlock)
        : fItemSize(itemSize)
        , fItemsPerBlock(itemsPerBlock)
        , fOwnFirstBlock(NULL == initialBlock)
        , fCount(0)
        , fInsertionIndexInBlock(0) {
        SkASSERT(itemsPerBlock > 0);
        fBlockSize = fItemSize * fItemsPerBlock;
        if (fOwnFirstBlock) {
            // This force us to allocate a new block on push_back().
            fInsertionIndexInBlock = fItemsPerBlock;
        } else {
            fBlocks.push_back() = initialBlock;
            fInsertionIndexInBlock = 0;
        }
    }

    /**
     * Adds an item and returns pointer to it.
     *
     * @return pointer to the added item.
     */
    void* push_back() {
        // we always have at least one block
        if (fItemsPerBlock == fInsertionIndexInBlock) {
            fBlocks.push_back() = sk_malloc_throw(fBlockSize);
            fInsertionIndexInBlock = 0;
        }
        void* ret = (char*)fBlocks.back() + fItemSize * fInsertionIndexInBlock;
        ++fCount;
        ++fInsertionIndexInBlock;
        return ret;
    }

    /**
     * Removes all added items.
     */
    void reset() {
        int firstBlockToFree = fOwnFirstBlock ? 0 : 1;
        for (int i = firstBlockToFree; i < fBlocks.count(); ++i) {
            sk_free(fBlocks[i]);
        }
        if (fOwnFirstBlock) {
            fBlocks.reset();
            // This force us to allocate a new block on push_back().
            fInsertionIndexInBlock = fItemsPerBlock;
        } else {
            fBlocks.pop_back_n(fBlocks.count() - 1);
            fInsertionIndexInBlock = 0;
        }
        fCount = 0;
    }

    /**
     * Returns the item count.
     */
    int count() const {
        return fCount;
    }

    /**
     * Is the count 0?
     */
    bool empty() const { return 0 == fCount; }

    /**
     * Access last item, only call if count() != 0
     */
    void* back() {
        SkASSERT(fCount);
        SkASSERT(fInsertionIndexInBlock > 0);
        return (char*)(fBlocks.back()) + (fInsertionIndexInBlock - 1) * fItemSize;
    }

    /**
     * Access last item, only call if count() != 0
     */
    const void* back() const {
        SkASSERT(fCount);
        SkASSERT(fInsertionIndexInBlock > 0);
        return (const char*)(fBlocks.back()) + (fInsertionIndexInBlock - 1) * fItemSize;
    }


    /**
     * Iterates through the allocator. This is faster than using operator[] when walking linearly
     * through the allocator.
     */
    class Iter {
    public:
        /**
         * Initializes the iterator. next() must be called before get().
         */
        Iter(const GrAllocator* allocator)
            : fAllocator(allocator)
            , fBlockIndex(-1)
            , fIndexInBlock(allocator->fItemsPerBlock - 1)
            , fItemIndex(-1) {}

        /**
         * Advances the iterator. Iteration is finished when next() returns false.
         */
        bool next() {
            ++fIndexInBlock;
            ++fItemIndex;
            if (fIndexInBlock == fAllocator->fItemsPerBlock) {
                ++fBlockIndex;
                fIndexInBlock = 0;
            }
            return fItemIndex < fAllocator->fCount;
        }

        /**
         * Gets the current iterator value. Call next() at least once before calling. Don't call
         * after next() returns false.
         */
        void* get() const {
            SkASSERT(fItemIndex >= 0 && fItemIndex < fAllocator->fCount);
            return (char*) fAllocator->fBlocks[fBlockIndex] + fIndexInBlock * fAllocator->fItemSize;
        }

    private:
        const GrAllocator* fAllocator;
        int                fBlockIndex;
        int                fIndexInBlock;
        int                fItemIndex;
    };

    /**
     * Access item by index.
     */
    void* operator[] (int i) {
        SkASSERT(i >= 0 && i < fCount);
        return (char*)fBlocks[i / fItemsPerBlock] +
               fItemSize * (i % fItemsPerBlock);
    }

    /**
     * Access item by index.
     */
    const void* operator[] (int i) const {
        SkASSERT(i >= 0 && i < fCount);
        return (const char*)fBlocks[i / fItemsPerBlock] +
               fItemSize * (i % fItemsPerBlock);
    }

protected:
    /**
     * Set first block of memory to write into.  Must be called before any other methods.
     * This requires that you have passed NULL in the constructor.
     *
     * @param   initialBlock    optional memory to use for the first block.
     *                          Must be at least itemSize*itemsPerBlock sized.
     *                          Caller is responsible for freeing this memory.
     */
    void setInitialBlock(void* initialBlock) {
        SkASSERT(0 == fCount);
        SkASSERT(0 == fBlocks.count());
        SkASSERT(fItemsPerBlock == fInsertionIndexInBlock);
        fOwnFirstBlock = false;
        fBlocks.push_back() = initialBlock;
        fInsertionIndexInBlock = 0;
    }

    // For access to above function.
    template <typename T> friend class GrTAllocator;

private:
    static const int NUM_INIT_BLOCK_PTRS = 8;

    SkSTArray<NUM_INIT_BLOCK_PTRS, void*, true>   fBlocks;
    size_t                                        fBlockSize;
    size_t                                        fItemSize;
    int                                           fItemsPerBlock;
    bool                                          fOwnFirstBlock;
    int                                           fCount;
    int                                           fInsertionIndexInBlock;

    typedef SkNoncopyable INHERITED;
};

template <typename T>
class GrTAllocator : SkNoncopyable {
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
     * Removes all added items.
     */
    void reset() {
        int c = fAllocator.count();
        for (int i = 0; i < c; ++i) {
            ((T*)fAllocator[i])->~T();
        }
        fAllocator.reset();
    }

    /**
     * Returns the item count.
     */
    int count() const {
        return fAllocator.count();
    }

    /**
     * Is the count 0?
     */
    bool empty() const { return fAllocator.empty(); }

    /**
     * Access last item, only call if count() != 0
     */
    T& back() {
        return *(T*)fAllocator.back();
    }

    /**
     * Access last item, only call if count() != 0
     */
    const T& back() const {
        return *(const T*)fAllocator.back();
    }

    /**
     * Iterates through the allocator. This is faster than using operator[] when walking linearly
     * through the allocator.
     */
    class Iter {
    public:
        /**
         * Initializes the iterator. next() must be called before get() or ops * and ->.
         */
        Iter(const GrTAllocator* allocator) : fImpl(&allocator->fAllocator) {}

        /**
         * Advances the iterator. Iteration is finished when next() returns false.
         */
        bool next() { return fImpl.next(); }

        /**
         * Gets the current iterator value. Call next() at least once before calling. Don't call
         * after next() returns false.
         */
        T* get() const { return (T*) fImpl.get(); }

        /**
         * Convenience operators. Same rules for calling apply as get().
         */
        T& operator*() const { return *this->get(); }
        T* operator->() const { return this->get(); }

    private:
        GrAllocator::Iter fImpl;
    };

    /**
     * Access item by index.
     */
    T& operator[] (int i) {
        return *(T*)(fAllocator[i]);
    }

    /**
     * Access item by index.
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
