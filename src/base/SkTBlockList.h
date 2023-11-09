/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTBlockList_DEFINED
#define SkTBlockList_DEFINED

#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkBlockAllocator.h"

#include <algorithm>
#include <cstring>
#include <type_traits>
#include <utility>

// Forward declarations for the iterators used by SkTBlockList
using IndexFn = int (*)(const SkBlockAllocator::Block*);
using NextFn = int (*)(const SkBlockAllocator::Block*, int);
template<typename T, typename B> using ItemFn = T (*)(B*, int);
template <typename T, bool Forward, bool Const, IndexFn Start, IndexFn End, NextFn Next,
          ItemFn<T, typename std::conditional<Const, const SkBlockAllocator::Block,
                                                     SkBlockAllocator::Block>::type> Resolve>
class BlockIndexIterator;

/**
 * SkTBlockList manages dynamic storage for instances of T, reserving fixed blocks such that
 * allocation is amortized across every N instances. In this way it is a hybrid of an array-based
 * vector and a linked-list. T can be any type and non-trivial destructors are automatically
 * invoked when the SkTBlockList is destructed. The addresses of instances are guaranteed
 * not to move except when a list is concatenated to another.
 *
 * The collection supports storing a templated number of elements inline before heap-allocated
 * blocks are made to hold additional instances. By default, the heap blocks are sized to hold the
 * same number of items as the inline block. A common pattern is to have the inline size hold only
 * a small number of items for the common case and then allocate larger blocks when needed.
 *
 * If the size of a collection is N, and its block size is B, the complexity of the common
 * operations are:
 *  - push_back()/emplace_back(): O(1), with malloc O(B)
 *  - pop_back(): O(1), with free O(B)
 *  - front()/back(): O(1)
 *  - reset(): O(N) for non-trivial types, O(N/B) for trivial types
 *  - concat(): O(B)
 *  - random access: O(N/B)
 *  - iteration: O(1) at each step
 *
 * These characteristics make it well suited for allocating items in a LIFO ordering, or otherwise
 * acting as a stack, or simply using it as a typed allocator.
 */
template <typename T, int StartingItems = 1>
class SkTBlockList {
public:
    /**
     * Create an list that defaults to using StartingItems as heap increment and the
     * kFixed growth policy (e.g. all allocations will match StartingItems).
     */
    SkTBlockList() : SkTBlockList(SkBlockAllocator::GrowthPolicy::kFixed) {}

    /**
     * Create an list that defaults to using StartingItems as the heap increment, but with
     * the defined growth policy.
    */
    explicit SkTBlockList(SkBlockAllocator::GrowthPolicy policy)
            : SkTBlockList(StartingItems, policy) {}

    /**
     * Create an list.
     *
     * @param   itemsPerBlock   the number of items to allocate at once
     * @param   policy          the growth policy for subsequent blocks of items
     */
    explicit SkTBlockList(int itemsPerBlock,
                          SkBlockAllocator::GrowthPolicy policy =
                                  SkBlockAllocator::GrowthPolicy::kFixed)
            : fAllocator(policy,
                         SkBlockAllocator::BlockOverhead<alignof(T)>() + sizeof(T)*itemsPerBlock) {}

    ~SkTBlockList() { this->reset(); }

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
    void concat(SkTBlockList<T, SI>&& other);

    /**
     * Allocate, if needed, space to hold N more Ts before another malloc will occur.
     */
    void reserve(int n) {
        int avail = fAllocator->currentBlock()->template avail<alignof(T)>() / sizeof(T);
        if (n > avail) {
            int reserved = n - avail;
            // Don't consider existing bytes since we've already determined how to split the N items
            fAllocator->template reserve<alignof(T)>(
                    reserved * sizeof(T), SkBlockAllocator::kIgnoreExistingBytes_Flag);
        }
    }

    /**
     * Remove the last item, only call if count() != 0
     */
    void pop_back() {
        SkASSERT(this->count() > 0);

        SkBlockAllocator::Block* block = fAllocator->currentBlock();

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
        if constexpr (!std::is_trivially_destructible<T>::value) {
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
        return const_cast<SkTBlockList*>(this)->item(i);
    }

private:
    // Let other SkTBlockLists have access (only ever used when T and S are the same but you
    // cannot have partial specializations declared as a friend...)
    template<typename S, int N> friend class SkTBlockList;
    friend class TBlockListTestAccess;  // for fAllocator

    inline static constexpr size_t StartingSize =
            SkBlockAllocator::Overhead<alignof(T)>() + StartingItems * sizeof(T);

    static T& GetItem(SkBlockAllocator::Block* block, int index) {
        return *static_cast<T*>(block->ptr(index));
    }
    static const T& GetItem(const SkBlockAllocator::Block* block, int index) {
        return *static_cast<const T*>(block->ptr(index));
    }
    static int First(const SkBlockAllocator::Block* b) {
        return b->firstAlignedOffset<alignof(T)>();
    }
    static int Last(const SkBlockAllocator::Block* b) {
        return b->metadata();
    }
    static int Increment(const SkBlockAllocator::Block* b, int index) {
        return index + sizeof(T);
    }
    static int Decrement(const SkBlockAllocator::Block* b, int index) {
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

    // N represents the number of items, whereas SkSBlockAllocator takes total bytes, so must
    // account for the block allocator's size too.
    //
    // This class uses the SkBlockAllocator's metadata to track total count of items, and per-block
    // metadata to track the index of the last allocated item within each block.
    SkSBlockAllocator<StartingSize> fAllocator;

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
};

template <typename T, int SI1>
template <int SI2>
void SkTBlockList<T, SI1>::concat(SkTBlockList<T, SI2>&& other) {
    // Optimize the common case where the list to append only has a single item
    if (other.empty()) {
        return;
    } else if (other.count() == 1) {
        this->push_back(other.back());
        other.pop_back();
        return;
    }

    // Manually move all items in other's head block into this list; all heap blocks from 'other'
    // will be appended to the block linked list (no per-item moves needed then).
    int headItemCount = 0;
    SkBlockAllocator::Block* headBlock = other.fAllocator->headBlock();
    SkDEBUGCODE(int oldCount = this->count();)
    if (headBlock->metadata() > 0) {
        int headStart = First(headBlock);
        int headEnd = Last(headBlock) + sizeof(T); // exclusive
        headItemCount = (headEnd - headStart) / sizeof(T);
        int avail = fAllocator->currentBlock()->template avail<alignof(T)>() / sizeof(T);
        if (headItemCount > avail) {
            // Make sure there is extra room for the items beyond what's already avail. Use the
            // kIgnoreGrowthPolicy_Flag to make this reservation as tight as possible since
            // 'other's heap blocks will be appended after it and any extra space is wasted.
            fAllocator->template reserve<alignof(T)>((headItemCount - avail) * sizeof(T),
                                                     SkBlockAllocator::kIgnoreExistingBytes_Flag |
                                                     SkBlockAllocator::kIgnoreGrowthPolicy_Flag);
        }

        if constexpr (std::is_trivially_copy_constructible<T>::value) {
            // memcpy all items at once (or twice between current and reserved space).
            SkASSERT(std::is_trivially_destructible<T>::value);
            auto copy = [](SkBlockAllocator::Block* src, int start, SkBlockAllocator* dst, int n) {
                auto target = dst->template allocate<alignof(T)>(n * sizeof(T));
                memcpy(target.fBlock->ptr(target.fAlignedOffset), src->ptr(start), n * sizeof(T));
                target.fBlock->setMetadata(target.fAlignedOffset + (n - 1) * sizeof(T));
            };

            if (avail > 0) {
                // Copy 0 to avail items into existing tail block
                copy(headBlock, headStart, fAllocator.allocator(), std::min(headItemCount, avail));
            }
            if (headItemCount > avail) {
                // Copy (head count - avail) into the extra reserved space
                copy(headBlock, headStart + avail * sizeof(T),
                     fAllocator.allocator(), headItemCount - avail);
            }
            fAllocator->setMetadata(fAllocator->metadata() + headItemCount);
        } else {
            // Move every item over one at a time
            for (int i = headStart; i < headEnd; i += sizeof(T)) {
                T& toMove = GetItem(headBlock, i);
                this->push_back(std::move(toMove));
                // Anything of interest should have been moved, but run this since T isn't
                // a trusted type.
                toMove.~T(); // NOLINT(bugprone-use-after-move): calling dtor always allowed
            }
        }

        other.fAllocator->releaseBlock(headBlock);
    }

    // other's head block must have been fully copied since it cannot be stolen
    SkASSERT(other.fAllocator->headBlock()->metadata() == 0 &&
             fAllocator->metadata() == oldCount + headItemCount);
    fAllocator->stealHeapBlocks(other.fAllocator.allocator());
    fAllocator->setMetadata(fAllocator->metadata() +
                            (other.fAllocator->metadata() - headItemCount));
    other.fAllocator->setMetadata(0);
}

/**
 * BlockIndexIterator provides a reusable iterator template for collections built on top of a
 * SkBlockAllocator, where each item is of the same type, and the index to an item can be iterated
 * over in a known manner. It supports const and non-const, and forward and reverse, assuming it's
 * provided with proper functions for starting, ending, and advancing.
 */
template <typename T,    // The element type (including any modifiers)
          bool Forward,  // Are indices within a block increasing or decreasing with iteration?
          bool Const,    // Whether or not T is const
          IndexFn Start, // Returns the index of the first valid item in a block
          IndexFn End,   // Returns the index of the last valid item (so it is inclusive)
          NextFn Next,   // Returns the next index given the current index
          ItemFn<T, typename std::conditional<Const, const SkBlockAllocator::Block,
                                                     SkBlockAllocator::Block>::type> Resolve>
class BlockIndexIterator {
    using BlockIter = typename SkBlockAllocator::BlockIter<Forward, Const>;
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
