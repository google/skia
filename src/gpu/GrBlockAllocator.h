/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/GrTypesPriv.h"   // GrAlignTo
#include "include/private/SkNoncopyable.h"

#include <memory>                          // std::unique_ptr
#include <cstddef>                         // max_align_t

/**
 * GrBlockAllocator provides low-level support for a block allocated arena with a dynamic tail that
 * tracks space reservations within each block. Its APIs provide the ability to reserve space,
 * resize reservations, and release reservations. It will always automatically create new blocks if
 * needed and destroy all remaining blocks when it is destructed. It assumes that anything allocated
 * within its blocks has its destructors called externally.
 *
 * GrBlockAllocator can be used directly for pure byte storage, but can also be extended
 * (preferably as protected/private) to gain access to additional APIs that make implementing more
 * advanced arenas/pools easier.
 *
 * GrBlockAllocator has minimal overhead, at 16 bytes per instance, 32 bytes per block, and 0 bytes
 * per reservation/allocation (plus any alignment padding). While a GrBlockAllocator can store any
 * amount of bytes, each allocation is limited to 512MB to simplify integer overflow math (and given
 * our uses cases, the total size should probably never reach 512MB anyways).
 */
class GrBlockAllocator : SkNoncopyable {
public:
    enum class GrowthPolicy : int {
        kFixed,
        kLinear,
        kFibonacci,
        kExponential,
        kLast = kExponential
    };
    static constexpr int kGrowthPolicyCount = static_cast<int>(GrowthPolicy::kLast) + 1;

    // Largest size that can be requested from allocate()
    static constexpr int kMaxAllocationSize = 1 << 29;

    /**
     * Create a new block allocator that creates itself and its first, inline block in an allocation
     * of size 'preallocSize'. Subsequent heap-allocated blocks will be sized according to
     * 'preallocSize' and the growth policy.
     */
    std::unique_ptr<GrBlockAllocator> Make(size_t preallocSize,
                                           GrowthPolicy policy = GrowthPolicy::kFixed) {
        return Prealloc<GrBlockAllocator>(preallocSize, policy);
    }

    ~GrBlockAllocator() { this->reset(); }
    void operator delete(void* p) { ::operator delete(p); }

    /**
     * Reserve space that will hold 'size' bytes. Returns the pointer to the first byte, and it will
     * be aligned on 'align'. The returned pointer and 'size' can be used later to release or
     * resize the allocation.
     *
     * This will automatically allocate a new block if there is not enough available space in the
     * current block to provide 'size' bytes.
     */
    template <size_t kAlign>
    void* allocate(size_t size) {
        int offset = this->allocateInternal<kAlign>(CheckSize(size));
        return fTail->ptr(offset);
    }

    /**
     * Return the maximum allocation size with the given alignment that can be made without
     * triggering the creation of a new block.
     */
    template <size_t kAlign>
    size_t avail() { return this->avail<kAlign>(this->currentBlock()); }

    /**
     * Attempt to resize the allocation that starts at 'p' of 'currentSize' bytes to the
     * 'requestedSize'. If true is returned, the allocation was successfully resized to
     * 'requestedSize'. If false is returned, it remains its previous size.
     *
     * When 'requestedSize' is smaller than 'currentSize', this can be used to reclaim bytes for
     * subsequent allocation requests. If 'requestedSize' is 0 and true is returned, then 'p'
     * can no longer be used.
     */
    bool resize(void* p, size_t currentSize, size_t requestedSize) {
        Block b = this->currentBlock();
        int offset = CheckAlloc(b, p, currentSize);
        return offset >= 0 && this->resize(b, offset, (int) currentSize, requestedSize);
    }

    /**
     * Attempt to release the allocation 'p' of 'size' bytes. This is equivalent to resizing it to 0
     * bytes. Returns true if the allocator successfully reclaimed bytes.
     */
    bool release(void* p, size_t size) {
        Block b = this->currentBlock();
        int offset = CheckAlloc(b, p, size);
        return offset >= 0 && this->release(b, offset, (int) size);
     }

    /**
     * Return the total number of allocated bytes, including the preallocation size and the
     * per-block overhead. This will always be >= preallocSize().
     */
    size_t size() const;

    /**
     * Return the total number of bytes that have been allocated, e.g. the unavailable portion of
     * size(). This includes any per-block or per-allocation overhead, and unrecoverable space.
     * This will always be <= size().
     */
    size_t used() const;

    /**
     * Return the number of bytes allocated for block storage when the GrBlockAllocator was
     * instantiated. This includes per-block overhead of the head block and the size of
     * GrBlockAllocator itself.
     */
    size_t preallocSize() const { return this->head()->fSize + fHeadOffset; }

protected:
    // An opaque way to address the blocks managed by the GrBlockAllocator
    using Block = uintptr_t;

    // The start of the inline head block can be no more than 16k away from 'this'.
    GrBlockAllocator(void* headBlock, int blockSize, GrowthPolicy policy);

    /**
     * Helper function to allocate and initialize the allocator of type T, such that its size
     * plus the inline head block fill 'preallocSize'. T's constructor must be accessible to
     * GrBlockAllocator, and must have (void* headBlock, int preallocSize) as its first two
     * arguments, followed by the remaining CtorArgs template values.
     */
    template <typename T, typename... CtorArgs>
    static std::unique_ptr<T> Prealloc(size_t preallocSize, CtorArgs... ctorArgs);

    /**
     * Like the public allocate<kAlign>(size) except that it also automatically inlines a header of
     * type Meta (output in 'metadata'). The user-facing pointer for the allocation is returned.
     * The header will be initialized by calling the available constructor:
     *   new (mem) Meta(currentBlock, offset, size, ...)
     *
     * This provides the block, offset, and size of the entire allocation (header included) to the
     * subclass for their own tracking, if desired, as well as forwarding the additional arguments
     * to the header constructor. Subclasses are responsible for calling ~Meta if needed.
     */
    template <size_t kAlign, typename Meta, typename... MetaArgs>
    void* allocate(size_t size, Meta** metadata, MetaArgs... metaArgs);

    /**
     * Return the amount of free space at the end of the given block, respecting the given alignment
     * and number of per-alloc metadata bytes. Even if the block is not the current block, this
     * provides an upper bound on the amount that existing allocations associated with 'block' can
     * be resized up to.
     */
    template <size_t kAlign>
    int avail(Block block) const {
        const BlockHeader* blockPtr = BlockToHeader(block);
        return blockPtr->fSize - blockPtr->cursor<kAlign>();
    }

    /**
     * Attempt to resize the allocation described by ('block', 'offset', 'size') to the 'newSize'.
     * It is assumed that the 3-tuple was previously provided from a call to allocate(). Returns
     * true if the allocation was updated to the new size.
     */
    bool resize(Block block, int offset, int size, size_t newSize);

    /**
     * Attempt to release the allocation described by ('block', 'offset', 'size'). It is assumed the
     * 3-tuple was previously provided from a call to allocate(). Returns true if the memory was
     * recovered by the allocator.
     */
    bool release(Block block, int offset, int size);

    /**
     * Return a pointer to the start of the current block. If called immediately after an allocate()
     * it will match the block the allocation belonged to. In such a case, the allocation's pointer
     * difference to the current block fits in 32 bits. It may be remembered or reconstructed at
     * a later point to provide resize() or release().
     */
    Block currentBlock() const { return reinterpret_cast<Block>(fTail); }

    /**
     * Explicitly free an entire block, invalidating any remaining allocations from the block.
     * GrBlockAllocator will release blocks automatically when it is destroyed, but this
     * function can be used to reclaim memory over the lifetime of the allocator. The provided
     * 'block' pointer must have previously come from a call to currentBlock() or allocate().
     *
     * If 'block' represents the inline-allocated head block, its cursor and metadata are instead
     * reset to their defaults.
     */
    void releaseBlock(Block block);

    /**
     * Explicitly free all blocks (invalidating all allocations), and resets the head block to its
     * default state. Subclasses that expose a reset functionality should perform their own block
     * and per-allocation clean up prior to calling this function.
     */
    void reset();

    /**
     * Use to recover the per-allocation metadata pointer assuming 'p' was returned by an earlier
     * call to allocate<kAlign, Meta>(...).
     */
    template <size_t kAlign, typename Meta>
    static Meta* Metadata(void* p);

    /**
     * All blocks expose a free 'int' that subclasses can use for any purpose. A new block always
     * starts with the value set to 0.
     */
    static int* BlockData(Block block) {
        BlockHeader* b = BlockToHeader(block);
        return &b->fMetadata;
    }

    /**
     * Subclasses can iterate over all active Blocks in the GrBlockAllocator using for loops:
     * Forward iteration from head to tail block:
     *   for (Block b : Blocks(this)) { }
     * Reverse iteration from tail to head block:
     *   for (Block b : RBlocks(this)) { }
     */
    template <bool kForward> class BlockIter;
    using Blocks = BlockIter<true>;
    using RBlocks = BlockIter<false>;

#ifdef SK_DEBUG
    static constexpr int kAssignedMarker = 0xCDCDCDCD;
    static constexpr int kFreedMarker    = 0xEFEFEFEF;

    void validate() const;
#endif

private:
    // This is a limitation of convenience to enable static assertions about numeric overflow.
    // It's unlikely any per-allocation metadata will be 128 bytes, the limit could be set higher.
    static constexpr int kMaxAllocMetadataSize = 128;

    struct BlockHeader {
        BlockHeader(int size);

        ~BlockHeader();
        void operator delete(void* p) { ::operator delete(p); }

        // Get fCursor, but aligned to kAlign
        template <size_t kAlign>
        int cursor() const;

        void* ptr(int offset) const {
            SkASSERT(offset >= (int) sizeof(BlockHeader) && offset < fSize);
            return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(this) + offset);
        }

#ifdef SK_DEBUG
        int          fSentinel;  // known value to check for bad back pointers to blocks
#endif

        BlockHeader* fNext;      // doubly-linked list of blocks.
        BlockHeader* fPrev;

        // Each block tracks its own cursor because as later blocks are released, an older block
        // may become the active tail again.
        int           fSize;   // includes the size of the BlockHeader and requested metadata
        int           fCursor; // (this + fCursor) points to next available allocation

        // Every block has an extra 4 bytes for subclasses to use however they want. It will start
        // at 0 when a new block is made, or when the head block is reset.
        int           fMetadata;
    };

    // Helper functions
    static constexpr size_t MaxBlockSize(size_t align);
    static inline BlockHeader* BlockToHeader(Block block);
    // Aborts if size is larger than allowable, safe allocation, otherwise returns it as an int
    static inline int CheckSize(size_t size);
    // Returns safe int offset from block to p if that is possible; returns negative if 'p' is
    // not within the block's range, or if the supposed allocation size exceeds the safe limit.
    static inline int CheckAlloc(Block block, void* p, size_t size);

    // Reserve space with the allocator, returning an aligned offset from the current block.
    template <size_t kAlign>
    int allocateInternal(int size);

    // Append a new block to the end of the block linked list, updating fTail. 'minimumSize' must
    // have enough room for sizeof(BlockHeader).
    void addBlock(int minimumSize, int maxSize);

     // The head block is always a fixed offset from 'this' since it is allocated inline.
    const BlockHeader* head() const { return const_cast<GrBlockAllocator*>(this)->head(); }
    BlockHeader* head() {
        return reinterpret_cast<BlockHeader*>(reinterpret_cast<uintptr_t>(this) + fHeadOffset);
    }

    BlockHeader* fTail; // All non-head blocks are heap allocated; tail will never be null.

    // All remaining state is packed into 64 bits to keep GrBlockAllocator at 16 bytes.

    // Byte offset from 'this' that points to the head BlockHeader that is inline allocated with the
    // GrBlockAllocator. At 16 bits, this shouldn't pose a limitation on subclass sizes.
    uint64_t     fHeadOffset   : 16;
    // Growth of the block size is controlled by two factors N0 and N1, as well as a rule that
    // controls how N0 is updated. When a new block is needed, we calculdate N1' = N0 + N1.
    // Depending on the growth policy, N0' = N0 (no growth or linear growth), or N0' = N1
    // (Fibonacci), or N0' = N1' (exponential). The size of the new block is N1' * preallocSize,
    // after which fN0 and fN1 store N0' and N1' clamped into 23 bits. With current bit allocations,
    // N1' is limited to 2^24, so 'preallocSize' must be at least 2^5 in order for block allocations
    // to eventually hit the hard 2^29 limit block size for GrBlockAllocator.
    uint64_t     fGrowthPolicy : 2;  // GrowthPolicy
    uint64_t     fN0           : 23; // = 1 for linear/exp.; = 0 for fixed/fibonacci, initially
    uint64_t     fN1           : 23; // = 1 initially

    static_assert(kGrowthPolicyCount <= 4);
};


///////////////////////////////////////////////////////////////////////////////////////////////////
// Template and inline implementations
///////////////////////////////////////////////////////////////////////////////////////////////////

///////// Static helper functions
constexpr size_t GrBlockAllocator::MaxBlockSize(size_t align) {
    // WLOG assumes 'align' was the largest encountered alignment for the allocator.
    return GrAlignTo(sizeof(BlockHeader), align) +   // The block overhead
           GrAlignTo(kMaxAllocMetadataSize, align) + // The max overhead for allocate<Meta...>
           kMaxAllocationSize;                       // The max input size allowed by CheckSize()
}

GrBlockAllocator::BlockHeader* GrBlockAllocator::BlockToHeader(Block block) {
    BlockHeader* b = reinterpret_cast<BlockHeader*>(block);
    SkASSERT(b->fSentinel == kAssignedMarker);
    return b;
}

int GrBlockAllocator::CheckSize(size_t size) {
    if (size > kMaxAllocationSize) {
        SK_ABORT("Allocation too large");
    } else {
        return (int) size;
    }
}

int GrBlockAllocator::CheckAlloc(Block block, void* p, size_t size) {
    BlockHeader* bp = BlockToHeader(block);
    uintptr_t ptr = reinterpret_cast<uintptr_t>(p);
    uintptr_t blockSize = bp->fSize;
    if (size > blockSize || ptr < block) {
        // Pointer cannot belong to the block
        return -1;
    }
    uintptr_t offset = ptr - block;
    if (offset >= blockSize) {
        // Definitely also cannot belong to the block
        return -1;
    }
    return (int) offset;
}

template <typename T, typename... CtorArgs>
std::unique_ptr<T> GrBlockAllocator::Prealloc(size_t preallocSize, CtorArgs... ctorArgs) {
    // Make sure it's feasibly possible to allocate a GrBlockAllocator subclass in this manner
    static constexpr int kPadding = GrAlignTo(sizeof(T), alignof(BlockHeader));
    static constexpr size_t kMinSize = kPadding + sizeof(BlockHeader);
    static_assert(kMinSize <= MaxBlockSize(alignof(BlockHeader)));
    static_assert(kPadding <= std::numeric_limits<uint16_t>::max());

    // Clamp 'preallocSize' to allowable range, at which point it can be cast ot an int.
    preallocSize = std::clamp(preallocSize, kMinSize, MaxBlockSize(alignof(BlockHeader)));

    void* mem = operator new(preallocSize);
    // NOTE: technically not aligned yet, this is the earliest possible start of the head block and
    // the GrBlockAllocator will align as appropriate and update 'preallocSize'.
    void* headBlock = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(mem) + kPadding);
    return std::unique_ptr<T>(new (mem) T(headBlock, (int) preallocSize - kPadding,
                                          std::forward<CtorArgs>(ctorArgs)...));
}

///////// Allocation and cursor management
template <size_t kAlign, typename Meta>
Meta* GrBlockAllocator::Metadata(void* p) {
    // If 'p' came from allocate<> it will be aligned to both the requested alignment and Meta.
    SkASSERT(reinterpret_cast<uintptr_t>(p) % alignof(Meta) == 0 &&
             reinterpret_cast<uintptr_t>(p) % kAlign == 0);
    static constexpr int kPadding = GrAlignTo(sizeof(Meta), kAlign);
    return reinterpret_cast<Meta*>(reinterpret_cast<uintptr_t>(p) - kPadding);
}

template <size_t kAlign, typename Meta, typename... MetaArgs>
void*  GrBlockAllocator::allocate(size_t size, Meta** metadata, MetaArgs... metaArgs) {
    static constexpr int kMaxAlign = std::max(kAlign, alignof(Meta));
    // Will pad sizeof(Meta) up if kAlign > alignof(Meta), but if kkMaxAlign is alignof(Meta)
    // then this will be sizeof(Meta) since that will still produce a kAlign'ed user pointer.
    static constexpr int kPadding = GrAlignTo(sizeof(Meta), kAlign);

    // Ensures we don't violate assumptions in the other numeric guards
    static_assert(sizeof(Meta) <= kMaxAllocMetadataSize);
    // Ensures our padded size calculation won't overflow
    static_assert(kMaxAllocationSize + kPadding <= std::numeric_limits<int32_t>::max());

    int iSize = CheckSize(size) + kPadding;
    int offset = this->allocateInternal<kMaxAlign>(iSize);
    *metadata = new (fTail->ptr(offset)) Meta(this->currentBlock(), offset, iSize,
                                              std::forward<MetaArgs>(metaArgs)...);
    SkASSERT((offset + kPadding) % kAlign == 0);
    return fTail->ptr(offset + kPadding);
}

template <size_t kAlign>
int GrBlockAllocator::allocateInternal(int size) {
    // These asserts, plus the static asserts in cursor<> and allocator<> ensure none of this
    // math will overflow
    static_assert(MaxBlockSize(kAlign) <= std::numeric_limits<int32_t>::max());
    static constexpr int kMaxInputSize =
            kMaxAllocationSize + GrAlignTo(kMaxAllocMetadataSize, kAlign);
    static constexpr int kMaxAlignedCursor = GrAlignTo(MaxBlockSize(kAlign), kAlign);
    // Max intermediate value when determining if there's room left in the current block
    static_assert(kMaxAlignedCursor + kMaxInputSize <= std::numeric_limits<int32_t>::max());

    SkASSERT(size > 0 && size <= kMaxInputSize);

    int alignedCursor = fTail->cursor<kAlign>();
    if (alignedCursor + size > fTail->fSize) {
        static constexpr int kPadding = GrAlignTo(sizeof(BlockHeader), kAlign);
        this->addBlock(kPadding + size, MaxBlockSize(kAlign));
        alignedCursor = fTail->cursor<kAlign>();
    }
    SkASSERT(fTail->fSize - alignedCursor >= size);
    fTail->fCursor = alignedCursor + size;
    return alignedCursor;
}

template <size_t kAlign>
int GrBlockAllocator::BlockHeader::cursor() const {
    static_assert(SkIsPow2(kAlign) && kAlign > 0);
    // Aligning adds kAlign - 1 as an intermediate step, so ensure that can't overflow
    static_assert(MaxBlockSize(kAlign) + kAlign - 1 <= std::numeric_limits<int32_t>::max());
    // Same as GrAlignTo, but operates on ints instead of size_t
    return (fCursor + kAlign - 1) & ~(kAlign - 1);
}

///////// Block iteration
template <bool kForward>
class GrBlockAllocator::BlockIter {
public:
    BlockIter(const GrBlockAllocator* allocator) : fAllocator(allocator) {}

    class Item {
    public:
        bool operator!=(const Item& other) const { return fBlock != other.fBlock; }

        Block operator*() const { return reinterpret_cast<Block>(fBlock); }

        Item& operator++() {
            fBlock = kForward ? fBlock->fNext : fBlock->fPrev;
            return *this;
        }

    private:
        friend BlockIter;

        Item(const BlockHeader* block) : fBlock(block) {}

        const BlockHeader* fBlock;
    };

    Item begin() const { return Item(kForward ? fAllocator->head() : fAllocator->fTail); }
    Item end() const { return Item(nullptr); }

private:
    const GrBlockAllocator* fAllocator;
};
