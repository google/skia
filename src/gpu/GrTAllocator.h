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

// Forward declarations for the iterators used by GrTAllocator and GrTVAllocator.
using IndexFn = int (*)(const GrBlockAllocator::Block*);
using NextFn = int (*)(const GrBlockAllocator::Block*, int);
template<typename T, typename B>
using ItemFn = T (*)(B*, int);
template <typename T, bool Forward, bool Const, IndexFn Start, IndexFn End, NextFn Next,
          ItemFn<T, typename std::conditional<Const, const GrBlockAllocator::Block, GrBlockAllocator::Block>::type> Resolve>
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

        // Run dtor for the popped item (we still run this when trivial since the compiler should
        // skip the dtor, but we still need the computed release index).
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
        // Invoke destructors in reverse order if not trivially destructible
        if /* constexpr */ (!std::is_trivially_destructible<T>::value) {
            for (const auto* b : fAllocator->rblocks()) {
                int c = b->metadata();
                for (int i = c - 1; i >= 0; i--) {
                    GetItem(b, i).~T();
                }
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
        return GetItem(fAllocator->headBlock(), 0);
    }

    /**
     * Access first item, only call if count() != 0
     */
    const T& front() const {
        SkASSERT(fTotalCount > 0);
        return GetItem(fAllocator->headBlock(), 0);
    }

    /**
     * Access last item, only call if count() != 0
     */
    T& back() {
        SkASSERT(fTotalCount > 0);
        int blockCount = fAllocator->currentBlock()->metadata();
        return GetItem(fAllocator->currentBlock(), blockCount - 1);
    }

    /**
     * Access last item, only call if count() != 0
     */
    const T& back() const {
        SkASSERT(fTotalCount > 0);
        int blockCount = fAllocator->currentBlock()->metadata();
        return GetItem(fAllocator->currentBlock(), blockCount - 1);
    }

    /**
     * Access item by index. Not an operator[] since it should not be considered constant time.
     * Use for-range loops by calling items() instead to access all added items in order.
     */
    T& item(int i) {
        // Iterate over blocks until we find the one that contains i.
        SkASSERT(i >= 0 && i < fTotalCount);
        for (auto* b : fAllocator->blocks()) {
            int blockCount = b->metadata();
            if (i < blockCount) {
                return GetItem(b, i);
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

    static T& GetItem(GrBlockAllocator::Block* block, int index) {
        int offset;
        return *GetItemAndOffset(block, index, &offset);
    }
    static const T& GetItem(const GrBlockAllocator::Block* block, int index) {
        int offset;
        return *GetItemAndOffset(block, index, &offset);
    }
    static int StartIndex(const GrBlockAllocator::Block* b) {
        return 0;
    }
    static int EndIndex(const GrBlockAllocator::Block* b) {
        return b->metadata() - 1;
    }
    static int AdvanceIndex(const GrBlockAllocator::Block* b, int index) {
        return index + 1;
    }
    static int DecrementIndex(const GrBlockAllocator::Block* b, int index) {
        return index - 1;
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

public:
    using Iter   = BlockIndexIterator<T&,       true,  false, &StartIndex, &EndIndex,   &AdvanceIndex,   &GetItem>;
    using CIter  = BlockIndexIterator<const T&, true,  true,  &StartIndex, &EndIndex,   &AdvanceIndex,   &GetItem>;
    using RIter  = BlockIndexIterator<T&,       false, false, &EndIndex,   &StartIndex, &DecrementIndex, &GetItem>;
    using CRIter = BlockIndexIterator<const T&, false, true,  &EndIndex,   &StartIndex, &DecrementIndex, &GetItem>;

    /**
     * Iterate over all items in allocation order (oldest to newest) using a for-range loop:
     *
     *   for (auto&& T : this->items()) {}
     */
    Iter   items() { return Iter(fAllocator.allocator()); }
    CIter  items() const { return CIter(fAllocator.allocator()); }
    // Iterate from newest to oldest using a for-range loop.
    RIter  ritems() { return RIter(fAllocator.allocator()); }
    CRIter ritems() const { return CRITer(fAllocator.allocator()); }
};

/**
 * Similar to GrTAllocator, except this allocator considers an "item" to be one T instance and
 * a variable number of V instances. This is useful for storing items that have a consistent
 * descriptor and a variable-length payload, such as geometric coordinates. Due to the varying
 * size of items, this is not randomly accessible and only exposes items by iterators.
 */
template<typename T, typename V, int StartingItems = 1, int ExpectedVPerT = 1>
class GrTVAllocator {
public:
    using Item = std::tuple<T&, V*, int>;
    using CItem = std::tuple<const T&, const V*, int>;

    /**
     * Create an allocator that defaults to using its template parameters for the eap increment.
     */
    GrTVAllocator() : GrTVAllocator(StartingItems, ExpectedVPerT) {}

    /**
     * Create an allocator with the given heap allocation parameters for controlling the size
     * of blocks allocated after the inline starting storage has filled up.
     *
     * @param   itemsPerBlock   the number of items to allocate at once
     * @param   vPerT           the number of V instances per T in a heap-allocated item
     */
    GrTVAllocator(int itemsPerBlock, int vPerT)
            : fTotalCount(0)
            , fAllocator(GrBlockAllocator::GrowthPolicy::kFibonacci,
                         GrBlockAllocator::BlockOverhead<kAlign, sizeof(Header)>() +
                                itemsPerBlock * (kVOffset + vPerT * sizeof(V))) {}

    ~GrTVAllocator() { this->reset(); }

    // Add an item to the list, where T and 'numV' instances of V are default initialized.
    Item push_back(int numV) {
        auto [tp, vp] = this->pushItem(numV);
        T& t = *new (tp) T;
        DefaultInitV(vp, numV);
        return {t, vp, numV};
    }

    // Add an item to the list, where 't' is copied and 'numV' instances of V are default init'ed.
    Item push_back(const T& t, int numV) {
        auto [tp, vp] = this->pushItem(numV);
        T& t = *new (tp) T(t);
        DefaultInitV(vp, numV);
        return {t, vp, numV};
    }

    // Add an item to the list, where 't' is moved into the item and 'numV' instances of V are
    // default initialized.
    Item push_back(T&& t, int numV) {
        auto [tp, vp] = this->pushItem(numV);
        T& t = *new (tp) T(std::move(t));
        DefaultInitV(vp, numV);
        return {t, vp, numV};
    }

    // Add an item to the list, where both 't' and 'numV' elements of 'v' are copied into the item.
    Item push_back(const T& t, const V v[], int numV) {
        auto [tp, vp] = this->pushItem(numV);
        T& nt = *new (tp) T(t);
        if (std::is_trivially_copyable<V>::value) {
            memcpy(vp, v, numV * sizeof(V));
        } else {
            for (int i = 0; i < numV; ++i) {
                new (vp + i) V(v[i]);
            }
        }
        return {nt, vp, numV};
    }

    // Add an item to the list, initializing the T instance in-place with the constructor 'args'
    // and default initializing the 'numV' instances of V.
    template <typename... Args>
    Item emplace_back(int numV, Args&&... args) {
        auto [tp, vp] = this->pushItem(numV);
        T& t = *new (tp) T(std::forward(...args));
        DefaultInitV(vp, numV);
        return {t, vp, numV};
    }

    // Remove the last item added to the list, invoking the destructors of every V in reverse order
    // and then the destructor of T (assuming they are not trivially destructible).
    void pop_back() {
        SkASSERT(fTotalCount > 0);
        GrBlockAllocator::Block* b = fAllocator->currentBlock();

        int prev = DestroyItem(b, b->metadata(), /* release memory */ true);
        if (prev == 0) {
            fAllocator->releaseBlock(b);
        } else {
            b->setMetadata(prev);
        }
        fTotalCount--;
    }

    // Remove all items from the list, invoking their destructors in reverse order as needed, and
    // freeing all allocated memory.
    void reset() {
        // Invoke destructors in reverse order if not trivially destructible
        if /* constexpr */ (!std::is_trivially_destructible<T>::value ||
                            !std::is_trivially_destructible<V>::value) {
            for (auto* b : fAllocator->rblocks()) {
                int start = b->metadata();
                while(start > 0) {
                    start = DestroyItem(b, start, /* release memory */ false);
                }
            }
        }
        fAllocator->reset();
        fTotalCount = 0;
    }

    // Access the first allocated item of the list. Must not be empty.
    Item front() {
        SkASSERT(fTotalCount > 0);
        GrBlockAllocator::Block* b = fAllocator->headBlock();
        return GetItem(b, b->firstAlignedOffset<kAlign, sizeof(Header)>());
    }
    CItem front() const {
        SkASSERT(fTotalCount > 0);
        GrBlockAllocator::Block* b = fAllocator->headBlock();
        return GetCItem(b, b->firstAlignedOffset<kAlign, sizeof(Header)>());
    }

    // Access the last allocated item of the list. Must not be empty.
    Item back() {
        SkASSERT(fTotalCount > 0);
        GrBlockAllocator::Block* b = fAllocator->currentBlock();
        return GetItem(b, b->alignedOffset<kAlign, sizeof(Header)>(b->metadata()));
    }
    CItem back() const {
        SkASSERT(fTotalCount > 0);
        GrBlockAllocator::Block* b = fAllocator->currentBlock();
        return GetCItem(b, b->alignedOffset<kAlign, sizeof(Header)>(b->metadata()));
    }

    int count() const {
#ifdef SK_DEBUG
        int count = 0;
        for (auto item : this->items()) {
            (void) item;
            count++;
        }
        SkASSERT(count == fTotalCount);
#endif
        return fTotalCount;
    }

    bool empty() const { return fTotalCount == 0; }

    template <int SI, int EVpT>
    void concat(GrTVAllocator<T, V, SI, EVpT>* other) {
        // Compact items in other's starting blocks until the tail block cannot hold an entire block
        int numCompacted = 0;
        for (GrBlockAllocator::Block* src : other->fAllocator->blocks()) {
            if (src->metadata() == 0) {
                // Empty block, no copy needed
                continue;
            }
            int copied = this->copyItems(src, /* add block */ src == other->fAllocator->headBlock());
            if (!copied) {
                break;
            }

            numCompacted += copied;
            other->fAllocator->releaseBlock(src);
        }

        // The other allocator's head block must have been fully copied since we can't just
        // reattach it to this allocator's block list.
        SkASSERT(other->fAllocator->headBlock()->metadata() == 0);
        fAllocator->stealHeapBlocks(other->fAllocator.allocator());
        fTotalCount += (other->fTotalCount - numCompacted);
        other->fTotalCount = 0;
    }

    void reserve(int numItems, int expectedVPerT) {
        size_t required = numItems * (kVOffset + expectedVPerT * sizeof(V));
        size_t avail = this->avail();
        if (required > avail) {
            fAllocator->template reserve<kAlign, sizeof(Header)>(required - avail);
        }
    }

private:
    struct Header {
        // The number of V's appended after the T allocation
        int fLen;
        // The offset of the previous allocation's start for reverse iteration; 0 if this is the
        // first allocation in a block.
        int fPrev;
    };

    static constexpr int kVOffset = GrAlignTo(sizeof(T), alignof(V));
    static constexpr size_t kAlign = std::max({alignof(T), alignof(V), alignof(Header)});
    static constexpr size_t StartingSize =
            GrBlockAllocator::Overhead<kAlign, sizeof(Header)>() +
            StartingItems * (sizeof(T) + ExpectedVPerT * sizeof(V));

    std::pair<T*, V*> pushItem(int numV) {
        auto br = fAllocator->template allocate<kAlign, sizeof(Header)>(kVOffset + numV * sizeof(V));

        Header* h = static_cast<Header*>(br.fBlock->ptr(br.fAlignedOffset - sizeof(Header)));
        T* t = static_cast<T*>(br.fBlock->ptr(br.fAlignedOffset));
        V* v = static_cast<V*>(br.fBlock->ptr(br.fAlignedOffset + kVOffset));

        int lastStart = br.fBlock->metadata(); // automatically 0 if this block was just added
        h->fLen = numV;
        h->fPrev = lastStart;
        br.fBlock->setMetadata(br.fStart);

        fTotalCount++;
        return {t, v};
    }

    int avail() const {
        return fAllocator->currentBlock()->template avail<kAlign, sizeof(Header)>();
    }

    int copyItems(const GrBlockAllocator::Block* src, bool addBlock) {
        SkASSERT(src->metadata() > 0);

        int count = fTotalCount;
        int start = StartIndex(src);
        // EndIndex returns the start of the last item, we want the exclusive end
        int end = ItemLimit(src, EndIndex(src));
        if (!addBlock && this->avail() < (end - start)) {
            // If we can't add a block and we can't fit the entire 'src' into the current, then
            // end now so we don't split 'src' apart.
            return 0;
        }

        while(start < end) {
            auto [t, v, numV] = GetItem(src, start);
            int size = kVOffset + numV * sizeof(V);
            if (this->avail() < size) {
                SkASSERT(addBlock && size <= (end - start));
                fAllocator->template reserve<kAlign, sizeof(Header)>(end - start, /* grow */ false);
                addBlock = false;
            }
            this->push_back(t, v, numV);
            start = AdvanceIndex(src, start);
        }

        // How many items were copied from 'src'
        return fTotalCount - count;
    }

    static Item GetItem(GrBlockAllocator::Block* b, int offset) {
        const Header* h = static_cast<const Header*>(b->ptr(offset - sizeof(Header)));
        T& t = *static_cast<T*>(b->ptr(offset));
        V* v = static_cast<V*>(b->ptr(offset + kVOffset));
        return {t, v, h->fLen};
    }
    static CItem GetItem(const GrBlockAllocator::Block* b, int offset) {
        const Header* h = static_cast<const Header*>(b->ptr(offset - sizeof(Header)));
        const T& t = *static_cast<const T*>(b->ptr(offset));
        const V* v = static_cast<const V*>(b->ptr(offset + kVOffset));
        return {t, v, h->fLen};
    }

    static int ItemLimit(const GrBlockAllocator::Block* b, int index) {
        const Header* h = static_cast<const Header*>(b->ptr(index - sizeof(Header)));
        return index + kVOffset + h->fLen * sizeof(V);
    }
    static int StartIndex(const GrBlockAllocator::Block* b) {
        return b->firstAlignedOffset<kAlign, sizeof(Header)>();
    }
    static int EndIndex(const GrBlockAllocator::Block* b) {
        return b->alignedOffset<kAlign, sizeof(Header)>(b->metadata());
    }
    static int AdvanceIndex(const GrBlockAllocator::Block* b, int index) {
        return b->alignedOffset<kAlign, sizeof(Header)>(ItemLimit(b, index));
    }
    static int DecrementIndex(const GrBlockAllocator::Block* b, int index) {
        const Header* h = static_cast<const Header*>(b->ptr(index - sizeof(Header)));
        return b->alignedOffset<kAlign, sizeof(Header)>(h->fPrev);
    }

    static int DestroyItem(GrBlockAllocator::Block* b, int start, bool releaseMemory) {
        // Reconstruct T+nV's allocated byte range
        SkASSERT(start > 0);
        int offset = b->alignedOffset<kAlign, sizeof(Header)>(start);

        Header* h = static_cast<Header*>(b->ptr(offset - sizeof(Header)));
        int end = offset + kVOffset + h->fLen * sizeof(V);

        // Run destructors if needed for T and V
        if (!std::is_trivially_destructible<V>::value) {
            for (int i = h->fLen - 1; i >= 0; i--) {
                V* v = static_cast<V*>(b->ptr(offset + kVOffset + i * sizeof(V)));
                v->~V();
            }
        }
        if (!std::is_trivially_destructible<T>::value) {
            T* t = static_cast<T*>(b->ptr(offset));
            t->~T();
        }

        int prev = h->fPrev;
        if (releaseMemory) {
            // If we're releasing the memory, it should be done in LIFO order so these asserts pass.
            if (prev > 0) {
                SkAssertResult(b->release(start, end));
            }
        }

        return prev;
    }

    static void DefaultInitV(V* v, int numV) {
        if /*constexpr*/ (!std::is_trivially_default_constructible<V>::value) {
            for (int i = 0; i < numV; ++i) {
                new (v + i) V;
            }
        }
    }

    int fTotalCount;
    GrSBlockAllocator<StartingSize> fAllocator;

public:
    using Iter   = BlockIndexIterator<Item,  true,  false, &StartIndex, &EndIndex,   &AdvanceIndex,   &GetItem>;
    using CIter  = BlockIndexIterator<CItem, true,  true,  &StartIndex, &EndIndex,   &AdvanceIndex,   &GetItem>;
    using RIter  = BlockIndexIterator<Item,  false, false, &EndIndex,   &StartIndex, &DecrementIndex, &GetItem>;
    using CRIter = BlockIndexIterator<CItem, false, true,  &EndIndex,   &StartIndex, &DecrementIndex, &GetItem>;

    // Iterate over the items in the list from oldest to newest, via a for-range loop.
    Iter   items() { return Iter(fAllocator.allocator()); }
    CIter  items() const { return CIter(fAllocator.allocator()); }
    RIter  ritems() { return RIter(fAllocator.allocator()); }
    CRIter ritems() const { return CRIter(fAllocator.allocator()); }
};

template <typename T,
          bool Forward,
          bool Const,
          IndexFn Start,
          IndexFn End,
          NextFn Next,
          ItemFn<T, typename std::conditional<Const, const GrBlockAllocator::Block, GrBlockAllocator::Block>::type> Resolve>
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

        Item(BlockItem block)
                : fBlock(block) {
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
