/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBlockAllocator_DEFINED
#define GrBlockAllocator_DEFINED

#include "include/private/GrTypesPriv.h"
#include "include/private/SkNoncopyable.h"

#include <memory>  // std::unique_ptr
#include <cstddef> // max_align_t

/**
 * GrBlockAllocator provides low-level support for a block allocated arena with a dynamic tail that
 * tracks space reservations within each block. Its APIs provide the ability to reserve space,
 * resize reservations, and release reservations. It will automatically create new blocks if needed
 * and destroy all remaining blocks when it is destructed. It assumes that anything allocated within
 * its blocks has its destructors called externally. It is recommended that GrBlockAllocator is
 * wrapped by a higher-level allocator that uses the low-level APIs to implement a simpler,
 * purpose-focused API w/o having to worry as much about byte-level concerns.
 *
 * GrBlockAllocator has no limit to its total size, but each allocation is limited to 512MB (which
 * should be sufficient for Ganesh's use cases). This limit allows all internal math to be performed
 * using 'int' even when that is the same as int32_t, avoiding the overhead of unsigned operations
 * or checking for overflow.
 *
 * Possible use modes:
 * 1. No upfront allocation, either on the stack or as a field
 *    GrBlockAllocator fAllocator(policy, heapAllocSize);
 *
 * 2. In-place new'd
 *    void* mem = operator new(totalSize);
 *    GrBlockAllocator* allocator = new (mem) GrBlockAllocator(policy, heapAllocSize,
 *                                                             totalSize-sizeof(GrBlockAllocator));
 *    ~GrBlockAllocator(); // Don't forget or heap blocks will remain
 *    delete mem;
 *
 * 3. Using extra space of a wrapping type
 *    class Foo<size_t N> {
 *        GrBlockAllocator fAllocator{policy, heapSize, N};
 *        char             fStorage[N];
 *    };
 */
class GrBlockAllocator : SkNoncopyable {
public:
    // Largest size that can be requested from allocate(), chosen because it's the largest pow-2
    // that is less than int32_t::max()/2.
    static constexpr int kMaxAllocationSize = 1 << 29;

    enum class GrowthPolicy : int {
        kFixed,
        kLinear,
        kFibonacci,
        kExponential,
        kLast = kExponential
    };
    static constexpr int kGrowthPolicyCount = static_cast<int>(GrowthPolicy::kLast) + 1;

    // Tuple representing a range of bytes, marking the unaligned start, the first aligned point
    // after any padding, and the upper limit depending on requested size.
    class Block;
    struct ByteRange {
        Block* fBlock;         // Owning block
        int    fStart;         // Inclusive byte lower limit of byte range
        int    fAlignedOffset; // >= start, matching alignment requirement (i.e. first real byte)
        int    fEnd;           // Exclusive upper limit of byte range
    };

    // Aligning Block with max_align_t means the aligned cursor calculations can be based solely on
    // the cursor offset when the requested alignment is max_align_t or less. The full pointer only
    // needs to be taken into account when over-aligning.
    class alignas(alignof(std::max_align_t)) Block {
    public:
        ~Block();
        void operator delete(void* p) { ::operator delete(p); }

        // Return the maximum allocation size with the given alignment that can fit in this block.
        template <size_t Align = 1, size_t Padding = 0>
        int avail() const { return fSize - this->cursor<Align, Padding>(); }

        // Convert an offset into this block's storage into a usable pointer.
        void* ptr(int offset) {
            SkASSERT(offset >= kDataStart && offset < fSize);
            return reinterpret_cast<char*>(this) + offset;
        }
        const void* ptr(int offset) const { return const_cast<Block*>(this)->ptr(offset); }

        // Every block has an extra 4 bytes for subclasses to use however they want. It will start
        // at 0 when a new block is made, or when the head block is reset.
        int metadata() const { return fMetadata; }
        void setMetadata(int value) { fMetadata = value; }

        /**
         * Attempt to release the byte interval described by the Allocation tuple. This will succeed
         * if the byte range between 'start' and 'end' is at the end of reserved space of the Block.
         * Returns true if the memory was recovered by the allocator, at which point that byte range
         * should no longer be read/written until it's re-allocated with allocate<...>().
         */
        inline bool release(int start, int end);

        /**
         * Similar to 'release()' except that the byte range reservation can be resized to something
         * other than just 0 bytes. If this represents the last reservation range, and there is room
         * in the block, this will update the reservation to be 'start' to 'start + newSize'.
         * 'newSize' can be smaller than the original allocation.
         *
         * Returns true if the allocation was updated to the new size. If 'newSize == 0' and true is
         * returned, then the allocation has been released and should not be used again.
         */
        inline bool resize(int start, int end, size_t newSize);

    private:
        friend class GrBlockAllocator;

        Block(Block* prev, int allocationSize, bool log);

        // Get fCursor, but aligned to such that ptr(rval) satisfies Align.
        template <size_t Align, size_t Padding>
        int cursor() const;

        SkDEBUGCODE(int fSentinel;) // known value to check for bad back pointers to blocks

        Block*          fNext;      // doubly-linked list of blocks
        Block*          fPrev;

        // Each block tracks its own cursor because as later blocks are released, an older block
        // may become the active tail again.
        int             fSize;      // includes the size of the BlockHeader and requested metadata
        int             fCursor;    // (this + fCursor) points to next available allocation
        int             fMetadata;
        bool fLog;
    };

    // The size of the head block is determined by 'additionalPreallocBytes'. Subsequent heap blocks
    // are determined by 'policy' and 'blockIncrementBytes', although 'blockIncrementBytes' will be
    // aligned to std::max_align_t.
    //
    // When 'additionalPreallocBytes' > 0, the allocator assumes that many extra bytes immediately
    // after the allocator can be used by its inline head block. This is useful when the allocator
    // is in-place new'ed into a larger block of memory, but it should remain set to 0 if stack
    // allocated or if the class layout does not guarantee that space is present.
    GrBlockAllocator(GrowthPolicy policy, size_t blockIncrementBytes,
                     size_t additionalPreallocBytes = 0, bool log = false);

    ~GrBlockAllocator() { this->reset(); }

    /**
     * Return the total number of bytes of the allocator, including its instance overhead, per-block
     * overhead and space used for allocations.
     */
    size_t totalSize() const;
    /**
     * Return the total number of bytes for usable for allocations. This includes bytes that have
     * been reserved already by a call to allocate() and bytes that are still available. It is
     * totalSize() minus all allocator and block-level overhead.
     */
    size_t totalUsableSpace() const;
    /**
     * Return the total number of usable bytes that have been reserved by allocations. This will
     * be less than or equal to totalUsableSpace().
     */
    size_t totalAllocatedSpace() const;

    /**
     * Return the total number of bytes that were pre-allocated for the GrBlockAllocator. This will
     * include 'additionalPreallocBytes' passed to the constructor, and represents what the total
     * size would become after a call to reset().
     */
    size_t preallocSize() const {
        // Don't double count fHead since its size is in sizeof(GrBlockAllocator) and fSize.
        return sizeof(GrBlockAllocator) + fHead.fSize - sizeof(Block);
    }
    /**
     * Return the usable size of the inline head block; this will be equal to
     * 'additionalPreallocBytes' plus any alignment padding that the system had to add to Block.
     * The returned value represents what could be allocated before a heap block is be created.
     */
    size_t preallocUsableSpace() const {
        return fHead.fSize - kDataStart;
    }

    /**
     * Reserve space that will hold 'size' bytes. This will automatically allocate a new block if
     * there is not enough available space in the current block to provide 'size' bytes. The
     * returned ByteRange tuple specifies the Block owning the reserved memory, the full byte range,
     * and the aligned offset within that range to use for the user-facing pointer. The following
     * invariants hold:
     *
     *  1. block->ptr(alignedOffset) is aligned to Align
     *  2. end - alignedOffset == size
     *  3. Padding <= alignedOffset - start <= Padding + Align - 1
     *
     * Invariant #3, when Padding > 0, allows intermediate allocators to embed metadata along with
     * the allocations. Care must be taken when reading/writing these Padding bytes because
     * ptr(alignedOffset - Padding) will not necessarily match Align.

     * Assuming some 'struct Meta', allocate<max(UserAlign, alignof(Meta)), sizeof(Meta)>(userSize)
     * means that ptr(alignedOffset) will always satisfy UserAlign and
     * ptr(alignedOffset - sizeof(Meta)) will always satisfy alignof(Meta). Alternatively, memcpy
     * can be used to read and write values between start and alignedOffset without worrying about
     * alignment requirements of the metadata.
     *
     * For over-aligned allocations, the alignedOffset (as an int) may not be a multiple of Align,
     * but the result of ptr(alignedOffset) will be a multiple of Align.
     */
    template <size_t Align, size_t Padding = 0>
    ByteRange allocate(size_t size);

    /**
     * Return a pointer to the start of the current block. This will never be null.
     */
    const Block* currentBlock() const { return fTail; }
    Block* currentBlock() { return fTail; }

    /**
     * Return the block that owns the allocated 'ptr'. Assuming that earlier, an allocation was
     * returned as {b, start, alignedOffset, end}, and 'p = b->ptr(alignedOffset)', then a call
     * to 'owningBlock<Align, Padding>(p, start) == b'.
     *
     * If calling code has already made a pointer to their metadata, i.e. 'm = p - Padding', then
     * 'owningBlock<Align, 0>(m, start)' will also return b, allowing you to recover the block from
     * metadata.
     *
     * If calling code has access to the original alignedOffset, this function should not be used
     * since the owning block is just 'p - alignedOffset', regardless of original Align or Padding.
     */
    template <size_t Align, size_t Padding = 0>
    Block* owningBlock(void* ptr, int start);

    template <size_t Align, size_t Padding = 0>
    const Block* owningBlock(const void* ptr, int start) const {
        return const_cast<GrBlockAllocator*>(this)->owningBlock<Align, Padding>(ptr, start);
    }

    /**
     * Explicitly free an entire block, invalidating any remaining allocations from the block.
     * GrBlockAllocator will release all alive blocks automatically when it is destroyed, but this
     * function can be used to reclaim memory over the lifetime of the allocator. The provided
     * 'block' pointer must have previously come from a call to currentBlock() or allocate().
     *
     * If 'block' represents the inline-allocated head block, its cursor and metadata are instead
     * reset to their defaults.
     */
    void releaseBlock(Block* block);

    /**
     * Explicitly free all blocks (invalidating all allocations), and resets the head block to its
     * default state.
     */
    void reset();

    /**
     * Subclasses can iterate over all active Blocks in the GrBlockAllocator using for loops:
     * Forward iteration from head to tail block:
     *   for (const Block* b : Blocks(this)) { }
     * Reverse iteration from tail to head block:
     *   for (const Block* b : RBlocks(this)) { }
     */
    template <bool kForward> class BlockIter;
    using Blocks = BlockIter<true>;
    using RBlocks = BlockIter<false>;

#ifdef SK_DEBUG
    static constexpr int kAssignedMarker = 0xBEEFFACE;
    static constexpr int kFreedMarker    = 0xCAFEBABE;

    void validate() const;
#endif

private:
    // Smallest value of fCursor, this will automatically repurpose any alignment padding that
    // the compiler introduced if the first allocation is aligned less than max_align_t.
    static constexpr int kDataStart = offsetof(Block, fMetadata) + sizeof(int);
    static constexpr int kBlockIncrementUnits = alignof(std::max_align_t);

    // Calculates the size of a new Block required to store a kMaxAllocationSize request for the
    // given alignment and padding bytes. Also represents maximum valid fCursor value in a Block.
    static constexpr size_t MaxBlockSize(size_t align, size_t padding);

    // Append a new block to the end of the block linked list, updating fTail. 'minSize' must
    // have enough room for sizeof(Block). 'maxSize' is the upper limit of fSize for the new block
    // that will preserve the static guarantees GrBlockAllocator makes.
    void addBlock(int minSize, int maxSize);

    Block* fTail; // All non-head blocks are heap allocated; tail will never be null.

    // All remaining state is packed into 64 bits to keep GrBlockAllocator at 16 bytes + head block.

    // Growth of the block size is controlled by four factors: BlockIncrement, N0 and N1, and a
    // policy defining how N0 is updated. When a new block is needed, we calculdate N1' = N0 + N1.
    // Depending on the policy, N0' = N0 (no growth or linear growth), or N0' = N1 (Fibonacci), or
    // N0' = N1' (exponential). The size of the new block is N1' * BlockIncrement * MaxAlign,
    // after which fN0 and fN1 store N0' and N1' clamped into 23 bits. With current bit allocations,
    // N1' is limited to 2^24, and assuming MaxAlign=16, then BlockIncrement must be '2' in order to
    // eventually reach the hard 2^29 size limit of GrBlockAllocator.

    // Next heap block size = (fBlockIncrement * alignof(std::max_align_t) * (fN0 + fN1))
    uint64_t fBlockIncrement : 16;
    uint64_t fGrowthPolicy   : 2;  // GrowthPolicy
    uint64_t fN0             : 23; // = 1 for linear/exp.; = 0 for fixed/fibonacci, initially
    uint64_t fN1             : 23; // = 1 initially
    bool fLog;
    // Inline head block, must be at the end so that it can utilize any additional reserved space
    // from the initial allocation.
    Block fHead;

    static_assert(kGrowthPolicyCount <= 4);
};


///////////////////////////////////////////////////////////////////////////////////////////////////
// Template and inline implementations
///////////////////////////////////////////////////////////////////////////////////////////////////

constexpr size_t GrBlockAllocator::MaxBlockSize(size_t align, size_t padding) {
    // Without loss of generality, assumes 'align' will be the largest encountered alignment for the
    // allocator (if it's not, the largest align will be encountered by the compiler and pass/fail
    // the same set of static asserts).
    return GrAlignTo(kDataStart + padding, align) + kMaxAllocationSize;
}

template <size_t Align, size_t Padding>
GrBlockAllocator::ByteRange GrBlockAllocator::allocate(size_t size) {
    // Amount of extra space for a new block to make sure the allocation can succeed.
    // This is the non-pessimistic version of GrAlignTo(kDataStart + kMaxPadding, Align) used in
    // MaxBlockSize(Align).
    static constexpr int kBlockOverhead =
            std::max(sizeof(Block), GrAlignTo(kDataStart + Padding, Align));

    // Ensures 'offset' and 'end' calculations will be valid
    static_assert((kMaxAllocationSize + GrAlignTo(MaxBlockSize(Align, Padding), Align))
                        <= std::numeric_limits<int32_t>::max());
    // Ensures size + blockOverhead + addBlock's alignment operations will be valid
    static_assert(kMaxAllocationSize + kBlockOverhead + ((1 << 12) - 1) // 4K align for large blocks
                        <= std::numeric_limits<int32_t>::max());

    if (size > kMaxAllocationSize) {
        SK_ABORT("Allocation too large");
    }

    int iSize = (int) size;
    int offset = fTail->cursor<Align, Padding>();
    int end = offset + iSize;
    if (end > fTail->fSize) {
        this->addBlock(iSize + kBlockOverhead, MaxBlockSize(Align, Padding));
        offset = fTail->cursor<Align, Padding>();
        end = offset + iSize;
    }

    // Check invariants
    SkASSERT(end <= fTail->fSize);
    SkASSERT(end - offset == iSize);
    SkASSERT(offset - fTail->fCursor >= (int) Padding &&
             offset - fTail->fCursor <= (int) (Padding + Align - 1));
    SkASSERT(reinterpret_cast<uintptr_t>(fTail->ptr(offset)) % Align == 0);

    int start = fTail->fCursor;
    fTail->fCursor = end;
    if (fLog)
        SkDebugf("allocate set cursor to %d\n", end);
    return {fTail, start, offset, end};
}

template <size_t Align, size_t Padding>
GrBlockAllocator::Block* GrBlockAllocator::owningBlock(void* p, int start) {
    // 'p' was originally formed by 'block + start + Padding + alignment', where alignment >= 0
    // and <= Align - 1. So p - start - Padding will be at most (Align - 1) greater than block,
    // which can be simply masked off.
    GrBlockAllocator::Block* block = reinterpret_cast<GrBlockAllocator::Block*>(
            (reinterpret_cast<uintptr_t>(p) - start - Padding) & ~(Align - 1));
    SkASSERT(block->fSentinel == kAssignedMarker);
    return block;
}

template <size_t Align, size_t Padding>
int GrBlockAllocator::Block::cursor() const {
    static_assert(SkIsPow2(Align));
    // Aligning adds (Padding + Align - 1) as an intermediate step, so ensure that can't overflow
    static_assert(MaxBlockSize(Align, Padding) + Padding + Align - 1
                        <= std::numeric_limits<int32_t>::max());

    if /* constexpr */ (Align <= alignof(std::max_align_t)) {
        // Same as GrAlignTo, but operates on ints instead of size_t
        return (fCursor + Padding + Align - 1) & ~(Align - 1);
    } else {
        // Must take into account that 'this' may be starting at a pointer that doesn't satisfy the
        // larger alignment request, so must align the entire pointer, not just offset
        uintptr_t blockPtr = reinterpret_cast<uintptr_t>(this);
        uintptr_t alignedPtr = (blockPtr + fCursor + Padding + Align - 1) & ~(Align - 1);
        SkASSERT(alignedPtr - blockPtr <= std::numeric_limits<int32_t>::max());
        return (int) (alignedPtr - blockPtr);
    }
}

bool GrBlockAllocator::Block::resize(int start, int end, size_t newSize) {
    SkASSERT(fSentinel == kAssignedMarker);
    SkASSERT(start >= kDataStart && end <= fSize && start < end);
    if (fLog)
        SkDebugf("Block resize, start = %d, end = %d, new size = %d\n", start, end, newSize);
    if (newSize > kMaxAllocationSize) {
        // Cannot possibly satisfy the resize and could overflow subsequent math
        return false;
    }
    if (fLog)
        SkDebugf(" - cursor = %d, cursor == end? %d\n", fCursor, fCursor == end);
    if (fCursor == end) {
        // The last allocation can be shifted assuming newSize fits in the size of the block
        int nextCursor = start + (int) newSize;
        if (fLog)
            SkDebugf(" - next cursor = %d, <= size(%d)? %d\n",
                nextCursor, fSize, nextCursor <= fSize);
        if (nextCursor <= fSize) {
            fCursor = nextCursor;
            if (fLog)
                SkDebugf("resize set cursor to %d\n", fCursor);
            return true;
        }
    }
    // Not enough room, or not the last allocation so couldn't grow the reserved space
    return false;
}

// NOTE: release is equivalent to resize(block, offset, size, 0), and the compiler can optimize
// most of the additions away, but it wasn't able to remove the unnecessary branch comparing the
// new cursor to the block size, so release() gets a specialization.
bool GrBlockAllocator::Block::release(int start, int end) {
    SkASSERT(fSentinel == kAssignedMarker);
    SkASSERT(start >= kDataStart && end <= fSize && start < end);
    if (fCursor == end) {
        fCursor = start;
        if (fLog)
            SkDebugf("release set cursor to %d\n", start);
        return true;
    } else {
        return false;
    }
}

///////// Block iteration
template <bool kForward>
class GrBlockAllocator::BlockIter {
public:
    BlockIter(const GrBlockAllocator* allocator) : fAllocator(allocator) {}

    class Item {
    public:
        bool operator!=(const Item& other) const { return fBlock != other.fBlock; }

        const Block* operator*() const { return fBlock; }

        Item& operator++() {
            fBlock = kForward ? fBlock->fNext : fBlock->fPrev;
            return *this;
        }

    private:
        friend BlockIter;

        Item(const Block* block) : fBlock(block) {}

        const Block* fBlock;
    };

    Item begin() const { return Item(kForward ? &fAllocator->fHead : fAllocator->fTail); }
    Item end() const { return Item(nullptr); }

private:
    const GrBlockAllocator* fAllocator;
};

#endif // GrBlockAllocator_DEFINED
