/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTAllocator_DEFINED
#define GrTAllocator_DEFINED

#include "src/gpu/GrBlockAllocator.h"

template <typename T, int StartingItems = 1>
class GrTAllocator {
public:
    /**
     * Create an allocator that defaults to using StartingItems as heap increment.
     */
    GrTAllocator()
            : fTotalCount(0)
            , fAllocator(GrBlockAllocator::GrowthPolicy::kFixed) {}

    /**
     * Create an allocator
     *
     * @param   itemsPerBlock   the number of items to allocate at once
     */
    explicit GrTAllocator(int itemsPerBlock)
            : fTotalCount(0)
            , fAllocator(GrBlockAllocator::GrowthPolicy::kFixed,
                         GrBlockAllocator::BlockOverhead<alignof(T)>() + sizeof(T)*itemsPerBlock) {}

    ~GrTAllocator() { this->reset(); }

    /**
     * Adds an item and returns it.
     *
     * @return the added item.
     */
    T& push_back() {
        return *new (this->pushItem()) T;
    }

    T& push_back(const T& t) {
        return *new (this->pushItem()) T(t);
    }

    T& push_back(T&& t) {
        return *new (this->pushItem()) T(std::move(t));
    }

    template <typename... Args>
    T& emplace_back(Args&&... args) {
        return *new (this->pushItem()) T(std::forward<Args>(args)...);
    }

    /**
     * Remove the last item, only call if count() != 0
     */
    void pop_back() {
        SkASSERT(fTotalCount > 0);

        GrBlockAllocator::Block* block = fAllocator->currentBlock();
        int newCount = block->metadata() - 1;

        // Run dtor for the popped item
        int releaseIndex;
        GetItemAndOffset(block, newCount, &releaseIndex)->~T();

        if (newCount == 0) {
            fAllocator->releaseBlock(block);
        } else {
            // Since this always follows LIFO, the block should always be able to release the memory
            SkAssertResult(block->release(releaseIndex, releaseIndex + sizeof(T)));
            block->setMetadata(newCount);
        }

        fTotalCount--;
    }

    /**
     * Removes all added items.
     */
    void reset() {
        // Invoke destructors in reverse order
        for (const auto* b : fAllocator->rblocks()) {
            int c = b->metadata();
            for (int i = c - 1; i >= 0; i--) {
                GetItem(b, i)->~T();
            }
        }

        fAllocator->reset();
        fTotalCount = 0;
    }

    /**
     * Returns the item count.
     */
    int count() const {
#ifdef SK_DEBUG
        // Confirm total count matches sum of block counts
        int count = 0;
        int blockCount = 0;
        for (const auto* b :fAllocator->blocks()) {
            count += b->metadata();
            blockCount++;
        }
        SkASSERT(count == fTotalCount);
#endif
        return fTotalCount;
    }

    /**
     * Is the count 0?
     */
    bool empty() const { return this->count() == 0; }

    /**
     * Access first item, only call if count() != 0
     */
    T& front() {
        // This assumes that the head block actually have room to store the first item.
        static_assert(StartingItems >= 1);
        SkASSERT(fTotalCount > 0);
        return *(GetItem(fAllocator->headBlock(), 0));
    }

    /**
     * Access first item, only call if count() != 0
     */
    const T& front() const {
        SkASSERT(fTotalCount > 0);
        return *(GetItem(fAllocator->headBlock(), 0));
    }

    /**
     * Access last item, only call if count() != 0
     */
    T& back() {
        SkASSERT(fTotalCount > 0);
        int blockCount = fAllocator->currentBlock()->metadata();
        return *(GetItem(fAllocator->currentBlock(), blockCount - 1));
    }

    /**
     * Access last item, only call if count() != 0
     */
    const T& back() const {
        SkASSERT(fTotalCount > 0);
        int blockCount = fAllocator->currentBlock()->metadata();
        return *(GetItem(fAllocator->currentBlock(), blockCount - 1));
    }

    template<bool Const>
    class Iterator {
        using BlockIter = typename GrBlockAllocator::BlockIter<true, Const>;
        using C = typename std::conditional<Const, const T, T>::type;
        using AllocatorT = typename std::conditional<Const, const GrTAllocator, GrTAllocator>::type;
    public:
        Iterator(AllocatorT* allocator) : fBlockIter(allocator->fAllocator.allocator()) {}

        class Item {
        public:
            bool operator!=(const Item& other) const {
                if (other.fBlock != fBlock) {
                    // Treat an empty head block the same as the end block
                    bool blockEmpty = !(*fBlock) || (*fBlock)->metadata() == 0;
                    bool otherEmpty = !(*other.fBlock) || (*other.fBlock)->metadata() == 0;
                    return !blockEmpty || !otherEmpty;
                } else {
                    // Same block, so differentiate by index into it (unless it's the "end" block
                    // in which case ignore index).
                    return SkToBool(*fBlock) && other.fIndex != fIndex;
                }
            }

            C& operator*() const {
                C* item = const_cast<C*>(static_cast<const C*>((*fBlock)->ptr(fIndex)));
                SkDEBUGCODE(int offset;)
                SkASSERT(item == GetItemAndOffset(*fBlock, fItem, &offset) && offset == fIndex);
                return *item;
            }

            Item& operator++() {
                const auto* block = *fBlock;
                fItem++;
                fIndex += sizeof(T);
                if (fItem >= block->metadata()) {
                    ++fBlock;
                    fItem = 0;
                    fIndex = StartingIndex(fBlock);
                }
                return *this;
            }

        private:
            friend Iterator;
            using BlockItem = typename BlockIter::Item;

            Item(BlockItem block) : fBlock(block), fItem(0), fIndex(StartingIndex(block)) {}

            static int StartingIndex(const BlockItem& block) {
                return *block ? (*block)->template firstAlignedOffset<alignof(T)>() : 0;
            }

            BlockItem fBlock;
            int       fItem;
            int       fIndex;
        };

        Item begin() const { return Item(fBlockIter.begin()); }
        Item end() const { return Item(fBlockIter.end()); }

    private:
        const BlockIter fBlockIter;
    };

    using Iter = Iterator<false>;
    using CIter = Iterator<true>;

    /**
     * Prefer using a for-range loop when iterating over all allocated items, vs. index access.
     */
    Iter items() { return Iter(this); }
    CIter items() const { return CIter(this); }

    /**
     * Access item by index. Not an operator[] since it should not be considered constant time.
     */
    T& item(int i) {
        // Iterate over blocks until we find the one that contains i.
        SkASSERT(i >= 0 && i < fTotalCount);
        for (const auto* b : fAllocator->blocks()) {
            int blockCount = b->metadata();
            if (i < blockCount) {
                return *GetItem(b, i);
            } else {
                i -= blockCount;
            }
        }
        SkUNREACHABLE;
    }
    const T& item(int i) const {
        return const_cast<GrTAllocator<T>*>(this)->item(i);
    }

private:
    static constexpr size_t StartingSize =
            GrBlockAllocator::Overhead<alignof(T)>() + StartingItems * sizeof(T);

    static T* GetItemAndOffset(const GrBlockAllocator::Block* block, int index, int* offset) {
        SkASSERT(index >= 0 && index < block->metadata());
        *offset = block->firstAlignedOffset<alignof(T)>() + index * sizeof(T);
        return const_cast<T*>(static_cast<const T*>(block->ptr(*offset)));
    }

    static T* GetItem(const GrBlockAllocator::Block* block, int index) {
        int offset;
        return GetItemAndOffset(block, index, &offset);
    }

    void* pushItem() {
        // 'template' required because fAllocator is a template, calling a template member
        auto br = fAllocator->template allocate<alignof(T)>(sizeof(T));
        br.fBlock->setMetadata(br.fBlock->metadata() + 1);
        fTotalCount++;
        return br.fBlock->ptr(br.fAlignedOffset);
    }

    // Each Block in the allocator tracks their count of items, but it's convenient to store
    // the sum of their counts as well.
    int fTotalCount;

    // N represents the number of items, whereas GrSBlockAllocator takes total bytes, so must
    // account for the block allocator's size too.
    GrSBlockAllocator<StartingSize> fAllocator;
};

#endif
