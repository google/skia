/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTAllocator_DEFINED
#define GrTAllocator_DEFINED

#include "src/gpu/GrBlockAllocator.h"

#include <type_traits>

// Forward declarations for the iterators used by GrTAllocator
using IndexFn = int (*)(const GrBlockAllocator::Block*);
using NextFn = int (*)(const GrBlockAllocator::Block*, int);
template<typename T, typename B> using ItemFn = T (*)(B*, int);
template <typename T, bool Forward, bool Const, IndexFn Start, IndexFn End, NextFn Next,
          ItemFn<T, typename std::conditional<Const, const GrBlockAllocator::Block,
                                                     GrBlockAllocator::Block>::type> Resolve>
class BlockIndexIterator;

/**
 * GrTAllocator manages dynamic storage for instances of T, reserving fixed blocks such that
 * allocation is amortized across every N instances. The optional StartingItems argument specifies
 * how many instances can be stored inline with the GrTAllocator.
 */
template <typename T, int StartingItems = 1>
class GrTAllocator {
public:
    /**
     * Create an allocator that defaults to using StartingItems as heap increment.
     */
    GrTAllocator() : GrTAllocator(StartingItems) {}

    /**
     * Create an allocator
     *
     * @param   itemsPerBlock   the number of items to allocate at once
     */
    explicit GrTAllocator(int itemsPerBlock)
            : fAllocator(GrBlockAllocator::GrowthPolicy::kFixed,
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
     * Move all items from 'other' to the end of this collection. When this returns, 'other' will
     * be empty. Items in 'other' may be moved as part of compacting the pre-allocated start of
     * 'other' into this list (using T's move constructor or memcpy if T is trivially copyable), but
     * this is O(StartingItems) and not O(N). All other items are concatenated in O(1).
     */
    template <int SI>
    void concat(GrTAllocator<T, SI>&& other);

    /**
     * Allocate, if needed, space to hold N more Ts before another malloc will occur.
     */
    void reserve(int n) {
        int avail = fAllocator->currentBlock()->template avail<alignof(T)>() / sizeof(T);
        if (n > avail) {
            int reserved = n - avail;
            if (reserved <= avail) {
                // Subtle: if reserved <= avail, the block allocator would think it already has
                // enough contiguous bytes for the reserve request.
                // "allocate" temporarily so that reserved will actually be allocated.
                auto temp = fAllocator->template allocate<alignof(T)>(avail * sizeof(T));
                fAllocator->template reserve<alignof(T)>(reserved * sizeof(T));
                SkAssertResult(temp.fBlock->release(temp.fStart, temp.fEnd));
            } else {
                fAllocator->template reserve<alignof(T)>(reserved * sizeof(T));
            }
        }
    }

    /**
     * Remove the last item, only call if count() != 0
     */
    void pop_back() {
        SkASSERT(this->count() > 0);

        GrBlockAllocator::Block* block = fAllocator->currentBlock();

        // Run dtor for the popped item
        int releaseIndex = Last(block);
        GetItem(block, releaseIndex).~T();

        if (releaseIndex == First(block)) {
            fAllocator->releaseBlock(block);
        } else {
            // Since this always follows LIFO, the block should always be able to release the memory
            SkAssertResult(block->release(releaseIndex, releaseIndex + sizeof(T)));
            block->setMetadata(Decrement(block, releaseIndex));
        }

        fAllocator->setMetadata(fAllocator->metadata() - 1);
    }

    /**
     * Removes all added items.
     */
    void reset() {
        // Invoke destructors in reverse order if not trivially destructible
        if /* constexpr */ (!std::is_trivially_destructible<T>::value) {
            for (T& t : this->ritems()) {
                t.~T();
            }
        }

        fAllocator->reset();
    }

    /**
     * Returns the item count.
     */
    int count() const {
#ifdef SK_DEBUG
        // Confirm total count matches sum of block counts
        int count = 0;
        for (const auto* b :fAllocator->blocks()) {
            if (b->metadata() == 0) {
                continue; // skip empty
            }
            count += (sizeof(T) + Last(b) - First(b)) / sizeof(T);
        }
        SkASSERT(count == fAllocator->metadata());
#endif
        return fAllocator->metadata();
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
        SkASSERT(this->count() > 0 && fAllocator->headBlock()->metadata() > 0);
        return GetItem(fAllocator->headBlock(), First(fAllocator->headBlock()));
    }
    const T& front() const {
        SkASSERT(this->count() > 0 && fAllocator->headBlock()->metadata() > 0);
        return GetItem(fAllocator->headBlock(), First(fAllocator->headBlock()));
    }

    /**
     * Access last item, only call if count() != 0
     */
    T& back() {
        SkASSERT(this->count() > 0 && fAllocator->currentBlock()->metadata() > 0);
        return GetItem(fAllocator->currentBlock(), Last(fAllocator->currentBlock()));
    }
    const T& back() const {
        SkASSERT(this->count() > 0 && fAllocator->currentBlock()->metadata() > 0);
        return GetItem(fAllocator->currentBlock(), Last(fAllocator->currentBlock()));
    }

    /**
     * Access item by index. Not an operator[] since it should not be considered constant time.
     * Use for-range loops by calling items() or ritems() instead to access all added items in order
     */
    T& item(int i) {
        SkASSERT(i >= 0 && i < this->count());

        // Iterate over blocks until we find the one that contains i.
        for (auto* b : fAllocator->blocks()) {
            if (b->metadata() == 0) {
                continue; // skip empty
            }

            int start = First(b);
            int end = Last(b) + sizeof(T); // exclusive
            int index = start + i * sizeof(T);
            if (index < end) {
                return GetItem(b, index);
            } else {
                i -= (end - start) / sizeof(T);
            }
        }
        SkUNREACHABLE;
    }
    const T& item(int i) const {
        return const_cast<GrTAllocator*>(this)->item(i);
    }

private:
    // Let other GrTAllocators have access (only ever used when T and S are the same but you cannot
    // have partial specializations declared as a friend...)
    template<typename S, int N> friend class GrTAllocator;

    static constexpr size_t StartingSize =
            GrBlockAllocator::Overhead<alignof(T)>() + StartingItems * sizeof(T);

    static T& GetItem(GrBlockAllocator::Block* block, int index) {
        return *static_cast<T*>(block->ptr(index));
    }
    static const T& GetItem(const GrBlockAllocator::Block* block, int index) {
        return *static_cast<const T*>(block->ptr(index));
    }
    static int First(const GrBlockAllocator::Block* b) {
        return b->firstAlignedOffset<alignof(T)>();
    }
    static int Last(const GrBlockAllocator::Block* b) {
        return b->metadata();
    }
    static int Increment(const GrBlockAllocator::Block* b, int index) {
        return index + sizeof(T);
    }
    static int Decrement(const GrBlockAllocator::Block* b, int index) {
        return index - sizeof(T);
    }

    void* pushItem() {
        // 'template' required because fAllocator is a template, calling a template member
        auto br = fAllocator->template allocate<alignof(T)>(sizeof(T));
        SkASSERT(br.fStart == br.fAlignedOffset ||
                 br.fAlignedOffset == First(fAllocator->currentBlock()));
        br.fBlock->setMetadata(br.fAlignedOffset);
        fAllocator->setMetadata(fAllocator->metadata() + 1);
        return br.fBlock->ptr(br.fAlignedOffset);
    }

    // N represents the number of items, whereas GrSBlockAllocator takes total bytes, so must
    // account for the block allocator's size too.
    //
    // This class uses the GrBlockAllocator's metadata to track total count of items, and per-block
    // metadata to track the index of the last allocated item within each block.
    GrSBlockAllocator<StartingSize> fAllocator;

public:
    using Iter   = BlockIndexIterator<T&,       true,  false, &First, &Last,  &Increment, &GetItem>;
    using CIter  = BlockIndexIterator<const T&, true,  true,  &First, &Last,  &Increment, &GetItem>;
    using RIter  = BlockIndexIterator<T&,       false, false, &Last,  &First, &Decrement, &GetItem>;
    using CRIter = BlockIndexIterator<const T&, false, true,  &Last,  &First, &Decrement, &GetItem>;

    /**
     * Iterate over all items in allocation order (oldest to newest) using a for-range loop:
     *
     *   for (auto&& T : this->items()) {}
     */
    Iter   items() { return Iter(fAllocator.allocator()); }
    CIter  items() const { return CIter(fAllocator.allocator()); }

    // Iterate from newest to oldest using a for-range loop.
    RIter  ritems() { return RIter(fAllocator.allocator()); }
    CRIter ritems() const { return CRIter(fAllocator.allocator()); }

#if GR_TEST_UTILS
    // For introspection
    const GrBlockAllocator* allocator() const { return fAllocator.allocator(); }
#endif
};

template <typename T, int SI1>
template <int SI2>
void GrTAllocator<T, SI1>::concat(GrTAllocator<T, SI2>&& other) {
    // Compact items at the front of 'other' (importantly, all of the items in its head block),
    // as well as any other blocks that happen to fit. Everything else will be stolen.
    int numCompacted = 0;
    for (GrBlockAllocator::Block* block : other.fAllocator->blocks()) {
        if (block->metadata() == 0) {
            continue; // skip empty
        }

        int blockStart = First(block);
        int blockEnd = Last(block) + sizeof(T); // exclusive
        int blockItemCount = (blockEnd - blockStart) / sizeof(T);
        int avail = fAllocator->currentBlock()->template avail<alignof(T)>() / sizeof(T);
        if (blockItemCount <= avail || block == other.fAllocator->headBlock()) {
            // Move items into this block
            this->reserve(blockItemCount); // no-op unless 'block' is the head
            if /*constexpr*/ (std::is_trivially_copyable<T>::value) {
                // memcpy all items at once (or twice between current and reserved space).
                SkASSERT(std::is_trivially_destructible<T>::value);
                int copyCount = std::min(blockItemCount, avail);
                auto firstCopy = fAllocator->template allocate<alignof(T)>(copyCount * sizeof(T));
                memcpy(firstCopy.fBlock->ptr(firstCopy.fAlignedOffset),
                        block->ptr(blockStart), copyCount * sizeof(T));
                firstCopy.fBlock->setMetadata(
                        firstCopy.fAlignedOffset + (copyCount - 1) * sizeof(T));

                // Finish copying the remaining items from other's head block, since they otherwise
                // won't be moved to this list with stealHeapBlocks later.
                if (blockItemCount > avail) {
                    int remaining = blockItemCount - avail;
                    auto secondCopy =
                            fAllocator->template allocate<alignof(T)>(remaining * sizeof(T));
                    memcpy(secondCopy.fBlock->ptr(secondCopy.fAlignedOffset),
                            block->ptr(blockStart + avail * sizeof(T)), remaining * sizeof(T));
                    secondCopy.fBlock->setMetadata(
                            secondCopy.fAlignedOffset + (remaining - 1) * sizeof(T));
                }
                fAllocator->setMetadata(fAllocator->metadata() + blockItemCount);
            } else {
                // Move every item over one at a time
                for (int i = blockStart; i < blockEnd; i += sizeof(T)) {
                    T& toMove = GetItem(block, i);
                    this->push_back(std::move(toMove));
                    // Anything of interest should have been moved, but run this since T isn't
                    // a trusted type.
                    toMove.~T(); // NOLINT(bugprone-use-after-move): calling dtor always allowed
                }
            }

            other.fAllocator->releaseBlock(block);
            numCompacted += blockItemCount;
        } else {
            // Do not compact a heap allocated block that won't fit in its entirety, it can
            // just be linked to the end of the block linked list.
            break;
        }
    }

    // other's head block must have been fully copied since it cannot be stolen
    SkASSERT(other.fAllocator->headBlock()->metadata() == 0);
    fAllocator->stealHeapBlocks(other.fAllocator.allocator());
    fAllocator->setMetadata(fAllocator->metadata() +
                            (other.fAllocator->metadata() - numCompacted));
    other.fAllocator->setMetadata(0);
}

/**
 * BlockIndexIterator provides a reusable iterator template for collections built on top of a
 * GrBlockAllocator, where each item is of the same type, and the index to an item can be iterated
 * over in a known manner. It supports const and non-const, and forward and reverse, assuming it's
 * provided with proper functions for starting, ending, and advancing.
 */
template <typename T,    // The element type (including any modifiers)
          bool Forward,  // Are indices within a block increasing or decreasing with iteration?
          bool Const,    // Whether or not T is const
          IndexFn Start, // Returns the index of the first valid item in a block
          IndexFn End,   // Returns the index of the last valid item (so it is inclusive)
          NextFn Next,   // Returns the next index given the current index
          ItemFn<T, typename std::conditional<Const, const GrBlockAllocator::Block,
                                                     GrBlockAllocator::Block>::type> Resolve>
class BlockIndexIterator {
    using BlockIter = typename GrBlockAllocator::BlockIter<Forward, Const>;
public:
    BlockIndexIterator(BlockIter iter) : fBlockIter(iter) {}

    class Item {
    public:
        bool operator!=(const Item& other) const {
            return other.fBlock != fBlock || (SkToBool(*fBlock) && other.fIndex != fIndex);
        }

        T operator*() const {
            SkASSERT(*fBlock);
            return Resolve(*fBlock, fIndex);
        }

        Item& operator++() {
            const auto* block = *fBlock;
            SkASSERT(block && block->metadata() > 0);
            SkASSERT((Forward && Next(block, fIndex) > fIndex) ||
                     (!Forward && Next(block, fIndex) < fIndex));
            fIndex = Next(block, fIndex);
            if ((Forward && fIndex > fEndIndex) || (!Forward && fIndex < fEndIndex)) {
                ++fBlock;
                this->setIndices();
            }
            return *this;
        }

    private:
        friend BlockIndexIterator;
        using BlockItem = typename BlockIter::Item;

        Item(BlockItem block) : fBlock(block) {
            this->setIndices();
        }

        void setIndices() {
            // Skip empty blocks
            while(*fBlock && (*fBlock)->metadata() == 0) {
                ++fBlock;
            }
            if (*fBlock) {
                fIndex = Start(*fBlock);
                fEndIndex = End(*fBlock);
            } else {
                fIndex = 0;
                fEndIndex = 0;
            }

            SkASSERT((Forward && fIndex <= fEndIndex) || (!Forward && fIndex >= fEndIndex));
        }

        BlockItem fBlock;
        int       fIndex;
        int       fEndIndex;
    };

    Item begin() const { return Item(fBlockIter.begin()); }
    Item end() const { return Item(fBlockIter.end()); }

private:
    BlockIter fBlockIter;
};

#endif
