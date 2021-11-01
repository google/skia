/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBlockAllocator_DEFINED
#define SkBlockAllocator_DEFINED

#include "include/core/SkMath.h"
#include "include/core/SkTypes.h"
#include "include/private/SkMacros.h"
#include "include/private/SkNoncopyable.h"
#include "include/private/SkTo.h"
#include "src/core/SkASAN.h"

#include <memory>  // std::unique_ptr
#include <cstddef> // max_align_t
#include <limits> // numeric_limits
#include <algorithm> // max

/**
 * SkBlockAllocator provides low-level support for a block allocated arena with a dynamic tail that
 * tracks space reservations within each block. Its APIs provide the ability to reserve space,
 * resize reservations, and release reservations. It will automatically create new blocks if needed
 * and destroy all remaining blocks when it is destructed. It assumes that anything allocated within
 * its blocks has its destructors called externally. It is recommended that SkBlockAllocator is
 * wrapped by a higher-level allocator that uses the low-level APIs to implement a simpler,
 * purpose-focused API w/o having to worry as much about byte-level concerns.
 *
 * SkBlockAllocator has no limit to its total size, but each allocation is limited to 512MB (which
 * should be sufficient for Skia's use cases). This upper allocation limit allows all internal
 * operations to be performed using 'int' and avoid many overflow checks. Static asserts are used
 * to ensure that those operations would not overflow when using the largest possible values.
 *
 * Possible use modes:
 * 1. No upfront allocation, either on the stack or as a field
 *    SkBlockAllocator allocator(policy, heapAllocSize);
 *
 * 2. In-place new'd
 *    void* mem = operator new(totalSize);
 *    SkBlockAllocator* allocator = new (mem) SkBlockAllocator(policy, heapAllocSize,
 *                                                             totalSize- sizeof(SkBlockAllocator));
 *    delete allocator;
 *
 * 3. Use SkSBlockAllocator to increase the preallocation size
 *    SkSBlockAllocator<1024> allocator(policy, heapAllocSize);
 *    sizeof(allocator) == 1024;
 */
// TODO(michaelludwig) - While API is different, this shares similarities to SkArenaAlloc and
// SkFibBlockSizes, so we should work to integrate them.
class SkBlockAllocator final : SkNoncopyable {
public:
    // Largest size that can be requested from allocate(), chosen because it's the largest pow-2
    // that is less than int32_t::max()/2.
    inline static constexpr int kMaxAllocationSize = 1 << 29;

    enum class GrowthPolicy : int {
        kFixed,       // Next block size = N
        kLinear,      //   = #blocks * N
        kFibonacci,   //   = fibonacci(#blocks) * N
        kExponential, //   = 2^#blocks * N
        kLast = kExponential
    };
    inline static constexpr int kGrowthPolicyCount = static_cast<int>(GrowthPolicy::kLast) + 1;

    class Block;

    // Tuple representing a range of bytes, marking the unaligned start, the first aligned point
    // after any padding, and the upper limit depending on requested size.
    struct ByteRange {
        Block* fBlock;         // Owning block
        int    fStart;         // Inclusive byte lower limit of byte range
        int    fAlignedOffset; // >= start, matching alignment requirement (i.e. first real byte)
        int    fEnd;           // Exclusive upper limit of byte range
    };

    class Block final {
    public:
        ~Block();
        void operator delete(void* p) { ::operator delete(p); }

        // Return the maximum allocation size with the given alignment that can fit in this block.
        template <size_t Align = 1, size_t Padding = 0>
        int avail() const { return std::max(0, fSize - this->cursor<Align, Padding>()); }

        // Return the aligned offset of the first allocation, assuming it was made with the
        // specified Align, and Padding. The returned offset does not mean a valid allocation
        // starts at that offset, this is a utility function for classes built on top to manage
        // indexing into a block effectively.
        template <size_t Align = 1, size_t Padding = 0>
        int firstAlignedOffset() const { return this->alignedOffset<Align, Padding>(kDataStart); }

        // Convert an offset into this block's storage into a usable pointer.
        void* ptr(int offset) {
            SkASSERT(offset >= kDataStart && offset < fSize);
            return reinterpret_cast<char*>(this) + offset;
        }
        const void* ptr(int offset) const { return const_cast<Block*>(this)->ptr(offset); }

        // Every block has an extra 'int' for clients to use however they want. It will start
        // at 0 when a new block is made, or when the head block is reset.
        int metadata() const { return fMetadata; }
        void setMetadata(int value) { fMetadata = value; }

        /**
         * Release the byte range between offset 'start' (inclusive) and 'end' (exclusive). This
         * will return true if those bytes were successfully reclaimed, i.e. a subsequent allocation
         * request could occupy the space. Regardless of return value, the provided byte range that
         * [start, end) represents should not be used until it's re-allocated with allocate<...>().
         */
        inline bool release(int start, int end);

        /**
         * Resize a previously reserved byte range of offset 'start' (inclusive) to 'end'
         * (exclusive). 'deltaBytes' is the SIGNED change to length of the reservation.
         *
         * When negative this means the reservation is shrunk and the new length is (end - start -
         * |deltaBytes|). If this new length would be 0, the byte range can no longer be used (as if
         * it were released instead). Asserts that it would not shrink the reservation below 0.
         *
         * If 'deltaBytes' is positive, the allocator attempts to increase the length of the
         * reservation. If 'deltaBytes' is less than or equal to avail() and it was the last
         * allocation in the block, it can be resized. If there is not enough available bytes to
         * accommodate the increase in size, or another allocation is blocking the increase in size,
         * then false will be returned and the reserved byte range is unmodified.
         */
        inline bool resize(int start, int end, int deltaBytes);

    private:
        friend class SkBlockAllocator;

        Block(Block* prev, int allocationSize);

        // We poison the unallocated space in a Block to allow ASAN to catch invalid writes.
        void poisonRange(int start, int end) {
            sk_asan_poison_memory_region(reinterpret_cast<char*>(this) + start, end - start);
        }
        void unpoisonRange(int start, int end) {
            sk_asan_unpoison_memory_region(reinterpret_cast<char*>(this) + start, end - start);
        }

        // Get fCursor, but aligned such that ptr(rval) satisfies Align.
        template <size_t Align, size_t Padding>
        int cursor() const { return this->alignedOffset<Align, Padding>(fCursor); }

        template <size_t Align, size_t Padding>
        int alignedOffset(int offset) const;

        bool isScratch() const { return fCursor < 0; }
        void markAsScratch() {
            fCursor = -1;
            this->poisonRange(kDataStart, fSize);
        }

        SkDEBUGCODE(int fSentinel;) // known value to check for bad back pointers to blocks

        Block*          fNext;      // doubly-linked list of blocks
        Block*          fPrev;

        // Each block tracks its own cursor because as later blocks are released, an older block
        // may become the active tail again.
        int             fSize;      // includes the size of the BlockHeader and requested metadata
        int             fCursor;    // (this + fCursor) points to next available allocation
        int             fMetadata;

        // On release builds, a Block's other 2 pointers and 3 int fields leaves 4 bytes of padding
        // for 8 and 16 aligned systems. Currently this is only manipulated in the head block for
        // an allocator-level metadata and is explicitly not reset when the head block is "released"
        // Down the road we could instead choose to offer multiple metadata slots per block.
        int             fAllocatorMetadata;
    };

    // The size of the head block is determined by 'additionalPreallocBytes'. Subsequent heap blocks
    // are determined by 'policy' and 'blockIncrementBytes', although 'blockIncrementBytes' will be
    // aligned to std::max_align_t.
    //
    // When 'additionalPreallocBytes' > 0, the allocator assumes that many extra bytes immediately
    // after the allocator can be used by its inline head block. This is useful when the allocator
    // is in-place new'ed into a larger block of memory, but it should remain set to 0 if stack
    // allocated or if the class layout does not guarantee that space is present.
    SkBlockAllocator(GrowthPolicy policy, size_t blockIncrementBytes,
                     size_t additionalPreallocBytes = 0);

    ~SkBlockAllocator() { this->reset(); }
    void operator delete(void* p) { ::operator delete(p); }

    /**
     * Helper to calculate the minimum number of bytes needed for heap block size, under the
     * assumption that Align will be the requested alignment of the first call to allocate().
     * Ex. To store N instances of T in a heap block, the 'blockIncrementBytes' should be set to
     *   BlockOverhead<alignof(T)>() + N * sizeof(T) when making the SkBlockAllocator.
     */
    template<size_t Align = 1, size_t Padding = 0>
    static constexpr size_t BlockOverhead();

    /**
     * Helper to calculate the minimum number of bytes needed for a preallocation, under the
     * assumption that Align will be the requested alignment of the first call to allocate().
     * Ex. To preallocate a SkSBlockAllocator to hold N instances of T, its arge should be
     *   Overhead<alignof(T)>() + N * sizeof(T)
     */
    template<size_t Align = 1, size_t Padding = 0>
    static constexpr size_t Overhead();

    /**
     * Return the total number of bytes of the allocator, including its instance overhead, per-block
     * overhead and space used for allocations.
     */
    size_t totalSize() const;
    /**
     * Return the total number of bytes usable for allocations. This includes bytes that have
     * been reserved already by a call to allocate() and bytes that are still available. It is
     * totalSize() minus all allocator and block-level overhead.
     */
    size_t totalUsableSpace() const;
    /**
     * Return the total number of usable bytes that have been reserved by allocations. This will
     * be less than or equal to totalUsableSpace().
     */
    size_t totalSpaceInUse() const;

    /**
     * Return the total number of bytes that were pre-allocated for the SkBlockAllocator. This will
     * include 'additionalPreallocBytes' passed to the constructor, and represents what the total
     * size would become after a call to reset().
     */
    size_t preallocSize() const {
        // Don't double count fHead's Block overhead in both sizeof(SkBlockAllocator) and fSize.
        return sizeof(SkBlockAllocator) + fHead.fSize - BaseHeadBlockSize();
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
     * Get the current value of the allocator-level metadata (a user-oriented slot). This is
     * separate from any block-level metadata, but can serve a similar purpose to compactly support
     * data collections on top of SkBlockAllocator.
     */
    int metadata() const { return fHead.fAllocatorMetadata; }

    /**
     * Set the current value of the allocator-level metadata.
     */
    void setMetadata(int value) { fHead.fAllocatorMetadata = value; }

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
     * the allocations. If the Padding bytes are used for some 'struct Meta', then
     * ptr(alignedOffset - sizeof(Meta)) can be safely used as a Meta* if Meta's alignment
     * requirements are less than or equal to the alignment specified in allocate<>. This can be
     * easily guaranteed by using the pattern:
     *
     *    allocate<max(UserAlign, alignof(Meta)), sizeof(Meta)>(userSize);
     *
     * This ensures that ptr(alignedOffset) will always satisfy UserAlign and
     * ptr(alignedOffset - sizeof(Meta)) will always satisfy alignof(Meta).  Alternatively, memcpy
     * can be used to read and write values between start and alignedOffset without worrying about
     * alignment requirements of the metadata.
     *
     * For over-aligned allocations, the alignedOffset (as an int) may not be a multiple of Align,
     * but the result of ptr(alignedOffset) will be a multiple of Align.
     */
    template <size_t Align, size_t Padding = 0>
    ByteRange allocate(size_t size);

    enum ReserveFlags : unsigned {
        // If provided to reserve(), the input 'size' will be rounded up to the next size determined
        // by the growth policy of the SkBlockAllocator. If not, 'size' will be aligned to max_align
        kIgnoreGrowthPolicy_Flag  = 0b01,
        // If provided to reserve(), the number of available bytes of the current block  will not
        // be used to satisfy the reservation (assuming the contiguous range was long enough to
        // begin with).
        kIgnoreExistingBytes_Flag = 0b10,

        kNo_ReserveFlags          = 0b00
    };

    /**
     * Ensure the block allocator has 'size' contiguous available bytes. After calling this
     * function, currentBlock()->avail<Align, Padding>() may still report less than 'size' if the
     * reserved space was added as a scratch block. This is done so that anything remaining in
     * the current block can still be used if a smaller-than-size allocation is requested. If 'size'
     * is requested by a subsequent allocation, the scratch block will automatically be activated
     * and the request will not itself trigger any malloc.
     *
     * The optional 'flags' controls how the input size is allocated; by default it will attempt
     * to use available contiguous bytes in the current block and will respect the growth policy
     * of the allocator.
     */
    template <size_t Align = 1, size_t Padding = 0>
    void reserve(size_t size, ReserveFlags flags = kNo_ReserveFlags);

    /**
     * Return a pointer to the start of the current block. This will never be null.
     */
    const Block* currentBlock() const { return fTail; }
    Block* currentBlock() { return fTail; }

    const Block* headBlock() const { return &fHead; }
    Block* headBlock() { return &fHead; }

    /**
     * Return the block that owns the allocated 'ptr'. Assuming that earlier, an allocation was
     * returned as {b, start, alignedOffset, end}, and 'p = b->ptr(alignedOffset)', then a call
     * to 'owningBlock<Align, Padding>(p, start) == b'.
     *
     * If calling code has already made a pointer to their metadata, i.e. 'm = p - Padding', then
     * 'owningBlock<Align, 0>(m, start)' will also return b, allowing you to recover the block from
     * the metadata pointer.
     *
     * If calling code has access to the original alignedOffset, this function should not be used
     * since the owning block is just 'p - alignedOffset', regardless of original Align or Padding.
     */
    template <size_t Align, size_t Padding = 0>
    Block* owningBlock(const void* ptr, int start);

    template <size_t Align, size_t Padding = 0>
    const Block* owningBlock(const void* ptr, int start) const {
        return const_cast<SkBlockAllocator*>(this)->owningBlock<Align, Padding>(ptr, start);
    }

    /**
     * Find the owning block of the allocated pointer, 'p'. Without any additional information this
     * is O(N) on the number of allocated blocks.
     */
    Block* findOwningBlock(const void* ptr);
    const Block* findOwningBlock(const void* ptr) const {
        return const_cast<SkBlockAllocator*>(this)->findOwningBlock(ptr);
    }

    /**
     * Explicitly free an entire block, invalidating any remaining allocations from the block.
     * SkBlockAllocator will release all alive blocks automatically when it is destroyed, but this
     * function can be used to reclaim memory over the lifetime of the allocator. The provided
     * 'block' pointer must have previously come from a call to currentBlock() or allocate().
     *
     * If 'block' represents the inline-allocated head block, its cursor and metadata are instead
     * reset to their defaults.
     *
     * If the block is not the head block, it may be kept as a scratch block to be reused for
     * subsequent allocation requests, instead of making an entirely new block. A scratch block is
     * not visible when iterating over blocks but is reported in the total size of the allocator.
     */
    void releaseBlock(Block* block);

    /**
     * Detach every heap-allocated block owned by 'other' and concatenate them to this allocator's
     * list of blocks. This memory is now managed by this allocator. Since this only transfers
     * ownership of a Block, and a Block itself does not move, any previous allocations remain
     * valid and associated with their original Block instances. SkBlockAllocator-level functions
     * that accept allocated pointers (e.g. findOwningBlock), must now use this allocator and not
     * 'other' for these allocations.
     *
     * The head block of 'other' cannot be stolen, so higher-level allocators and memory structures
     * must handle that data differently.
     */
    void stealHeapBlocks(SkBlockAllocator* other);

    /**
     * Explicitly free all blocks (invalidating all allocations), and resets the head block to its
     * default state. The allocator-level metadata is reset to 0 as well.
     */
    void reset();

    /**
     * Remove any reserved scratch space, either from calling reserve() or releaseBlock().
     */
    void resetScratchSpace();

    template <bool Forward, bool Const> class BlockIter;

    /**
     * Clients can iterate over all active Blocks in the SkBlockAllocator using for loops:
     *
     * Forward iteration from head to tail block (or non-const variant):
     *   for (const Block* b : this->blocks()) { }
     * Reverse iteration from tail to head block:
     *   for (const Block* b : this->rblocks()) { }
     *
     * It is safe to call releaseBlock() on the active block while looping.
     */
    inline BlockIter<true, false> blocks();
    inline BlockIter<true, true> blocks() const;
    inline BlockIter<false, false> rblocks();
    inline BlockIter<false, true> rblocks() const;

#ifdef SK_DEBUG
    inline static constexpr int kAssignedMarker = 0xBEEFFACE;
    inline static constexpr int kFreedMarker    = 0xCAFEBABE;

    void validate() const;
#endif

private:
    friend class BlockAllocatorTestAccess;
    friend class TBlockListTestAccess;

    inline static constexpr int kDataStart = sizeof(Block);
    #ifdef SK_FORCE_8_BYTE_ALIGNMENT
        // This is an issue for WASM builds using emscripten, which had std::max_align_t = 16, but
        // was returning pointers only aligned to 8 bytes.
        // https://github.com/emscripten-core/emscripten/issues/10072
        //
        // Setting this to 8 will let SkBlockAllocator properly correct for the pointer address if
        // a 16-byte aligned allocation is requested in wasm (unlikely since we don't use long
        // doubles).
        inline static constexpr size_t kAddressAlign = 8;
    #else
        // The alignment Block addresses will be at when created using operator new
        // (spec-compliant is pointers are aligned to max_align_t).
        inline static constexpr size_t kAddressAlign = alignof(std::max_align_t);
    #endif

    // Calculates the size of a new Block required to store a kMaxAllocationSize request for the
    // given alignment and padding bytes. Also represents maximum valid fCursor value in a Block.
    template<size_t Align, size_t Padding>
    static constexpr size_t MaxBlockSize();

    static constexpr int BaseHeadBlockSize() {
        return sizeof(SkBlockAllocator) - offsetof(SkBlockAllocator, fHead);
    }

    // Append a new block to the end of the block linked list, updating fTail. 'minSize' must
    // have enough room for sizeof(Block). 'maxSize' is the upper limit of fSize for the new block
    // that will preserve the static guarantees SkBlockAllocator makes.
    void addBlock(int minSize, int maxSize);

    int scratchBlockSize() const { return fHead.fPrev ? fHead.fPrev->fSize : 0; }

    Block* fTail; // All non-head blocks are heap allocated; tail will never be null.

    // All remaining state is packed into 64 bits to keep SkBlockAllocator at 16 bytes + head block
    // (on a 64-bit system).

    // Growth of the block size is controlled by four factors: BlockIncrement, N0 and N1, and a
    // policy defining how N0 is updated. When a new block is needed, we calculate N1' = N0 + N1.
    // Depending on the policy, N0' = N0 (no growth or linear growth), or N0' = N1 (Fibonacci), or
    // N0' = N1' (exponential). The size of the new block is N1' * BlockIncrement * MaxAlign,
    // after which fN0 and fN1 store N0' and N1' clamped into 23 bits. With current bit allocations,
    // N1' is limited to 2^24, and assuming MaxAlign=16, then BlockIncrement must be '2' in order to
    // eventually reach the hard 2^29 size limit of SkBlockAllocator.

    // Next heap block size = (fBlockIncrement * alignof(std::max_align_t) * (fN0 + fN1))
    uint64_t fBlockIncrement : 16;
    uint64_t fGrowthPolicy   : 2;  // GrowthPolicy
    uint64_t fN0             : 23; // = 1 for linear/exp.; = 0 for fixed/fibonacci, initially
    uint64_t fN1             : 23; // = 1 initially

    // Inline head block, must be at the end so that it can utilize any additional reserved space
    // from the initial allocation.
    // The head block's prev pointer may be non-null, which signifies a scratch block that may be
    // reused instead of allocating an entirely new block (this helps when allocate+release calls
    // bounce back and forth across the capacity of a block).
    alignas(kAddressAlign) Block fHead;

    static_assert(kGrowthPolicyCount <= 4);
};

// A wrapper around SkBlockAllocator that includes preallocated storage for the head block.
// N will be the preallocSize() reported by the allocator.
template<size_t N>
class SkSBlockAllocator : SkNoncopyable {
public:
    using GrowthPolicy = SkBlockAllocator::GrowthPolicy;

    SkSBlockAllocator() {
        new (fStorage) SkBlockAllocator(GrowthPolicy::kFixed, N, N - sizeof(SkBlockAllocator));
    }
    explicit SkSBlockAllocator(GrowthPolicy policy) {
        new (fStorage) SkBlockAllocator(policy, N, N - sizeof(SkBlockAllocator));
    }

    SkSBlockAllocator(GrowthPolicy policy, size_t blockIncrementBytes) {
        new (fStorage) SkBlockAllocator(policy, blockIncrementBytes, N - sizeof(SkBlockAllocator));
    }

    ~SkSBlockAllocator() {
        this->allocator()->~SkBlockAllocator();
    }

    SkBlockAllocator* operator->() { return this->allocator(); }
    const SkBlockAllocator* operator->() const { return this->allocator(); }

    SkBlockAllocator* allocator() { return reinterpret_cast<SkBlockAllocator*>(fStorage); }
    const SkBlockAllocator* allocator() const {
        return reinterpret_cast<const SkBlockAllocator*>(fStorage);
    }

private:
    static_assert(N >= sizeof(SkBlockAllocator));

    // Will be used to placement new the allocator
    alignas(SkBlockAllocator) char fStorage[N];
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// Template and inline implementations

SK_MAKE_BITFIELD_OPS(SkBlockAllocator::ReserveFlags)

template<size_t Align, size_t Padding>
constexpr size_t SkBlockAllocator::BlockOverhead() {
    static_assert(SkAlignTo(kDataStart + Padding, Align) >= sizeof(Block));
    return SkAlignTo(kDataStart + Padding, Align);
}

template<size_t Align, size_t Padding>
constexpr size_t SkBlockAllocator::Overhead() {
    // NOTE: On most platforms, SkBlockAllocator is packed; this is not the case on debug builds
    // due to extra fields, or on WASM due to 4byte pointers but 16byte max align.
    return std::max(sizeof(SkBlockAllocator),
                    offsetof(SkBlockAllocator, fHead) + BlockOverhead<Align, Padding>());
}

template<size_t Align, size_t Padding>
constexpr size_t SkBlockAllocator::MaxBlockSize() {
    // Without loss of generality, assumes 'align' will be the largest encountered alignment for the
    // allocator (if it's not, the largest align will be encountered by the compiler and pass/fail
    // the same set of static asserts).
    return BlockOverhead<Align, Padding>() + kMaxAllocationSize;
}

template<size_t Align, size_t Padding>
void SkBlockAllocator::reserve(size_t size, ReserveFlags flags) {
    if (size > kMaxAllocationSize) {
        SK_ABORT("Allocation too large (%zu bytes requested)", size);
    }
    int iSize = (int) size;
    if ((flags & kIgnoreExistingBytes_Flag) ||
        this->currentBlock()->avail<Align, Padding>() < iSize) {

        int blockSize = BlockOverhead<Align, Padding>() + iSize;
        int maxSize = (flags & kIgnoreGrowthPolicy_Flag) ? blockSize
                                                         : MaxBlockSize<Align, Padding>();
        SkASSERT((size_t) maxSize <= (MaxBlockSize<Align, Padding>()));

        SkDEBUGCODE(auto oldTail = fTail;)
        this->addBlock(blockSize, maxSize);
        SkASSERT(fTail != oldTail);
        // Releasing the just added block will move it into scratch space, allowing the original
        // tail's bytes to be used first before the scratch block is activated.
        this->releaseBlock(fTail);
    }
}

template <size_t Align, size_t Padding>
SkBlockAllocator::ByteRange SkBlockAllocator::allocate(size_t size) {
    // Amount of extra space for a new block to make sure the allocation can succeed.
    static constexpr int kBlockOverhead = (int) BlockOverhead<Align, Padding>();

    // Ensures 'offset' and 'end' calculations will be valid
    static_assert((kMaxAllocationSize + SkAlignTo(MaxBlockSize<Align, Padding>(), Align))
                        <= (size_t) std::numeric_limits<int32_t>::max());
    // Ensures size + blockOverhead + addBlock's alignment operations will be valid
    static_assert(kMaxAllocationSize + kBlockOverhead + ((1 << 12) - 1) // 4K align for large blocks
                        <= std::numeric_limits<int32_t>::max());

    if (size > kMaxAllocationSize) {
        SK_ABORT("Allocation too large (%zu bytes requested)", size);
    }

    int iSize = (int) size;
    int offset = fTail->cursor<Align, Padding>();
    int end = offset + iSize;
    if (end > fTail->fSize) {
        this->addBlock(iSize + kBlockOverhead, MaxBlockSize<Align, Padding>());
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

    fTail->unpoisonRange(offset - Padding, end);

    return {fTail, start, offset, end};
}

template <size_t Align, size_t Padding>
SkBlockAllocator::Block* SkBlockAllocator::owningBlock(const void* p, int start) {
    // 'p' was originally formed by aligning 'block + start + Padding', producing the inequality:
    //     block + start + Padding <= p <= block + start + Padding + Align-1
    // Rearranging this yields:
    //     block <= p - start - Padding <= block + Align-1
    // Masking these terms by ~(Align-1) reconstructs 'block' if the alignment of the block is
    // greater than or equal to Align (since block & ~(Align-1) == (block + Align-1) & ~(Align-1)
    // in that case). Overalignment does not reduce to inequality unfortunately.
    if /* constexpr */ (Align <= kAddressAlign) {
        Block* block = reinterpret_cast<Block*>(
                (reinterpret_cast<uintptr_t>(p) - start - Padding) & ~(Align - 1));
        SkASSERT(block->fSentinel == kAssignedMarker);
        return block;
    } else {
        // There's not a constant-time expression available to reconstruct the block from 'p',
        // but this is unlikely to happen frequently.
        return this->findOwningBlock(p);
    }
}

template <size_t Align, size_t Padding>
int SkBlockAllocator::Block::alignedOffset(int offset) const {
    static_assert(SkIsPow2(Align));
    // Aligning adds (Padding + Align - 1) as an intermediate step, so ensure that can't overflow
    static_assert(MaxBlockSize<Align, Padding>() + Padding + Align - 1
                        <= (size_t) std::numeric_limits<int32_t>::max());

    if /* constexpr */ (Align <= kAddressAlign) {
        // Same as SkAlignTo, but operates on ints instead of size_t
        return (offset + Padding + Align - 1) & ~(Align - 1);
    } else {
        // Must take into account that 'this' may be starting at a pointer that doesn't satisfy the
        // larger alignment request, so must align the entire pointer, not just offset
        uintptr_t blockPtr = reinterpret_cast<uintptr_t>(this);
        uintptr_t alignedPtr = (blockPtr + offset + Padding + Align - 1) & ~(Align - 1);
        SkASSERT(alignedPtr - blockPtr <= (uintptr_t) std::numeric_limits<int32_t>::max());
        return (int) (alignedPtr - blockPtr);
    }
}

bool SkBlockAllocator::Block::resize(int start, int end, int deltaBytes) {
    SkASSERT(fSentinel == kAssignedMarker);
    SkASSERT(start >= kDataStart && end <= fSize && start < end);

    if (deltaBytes > kMaxAllocationSize || deltaBytes < -kMaxAllocationSize) {
        // Cannot possibly satisfy the resize and could overflow subsequent math
        return false;
    }
    if (fCursor == end) {
        int nextCursor = end + deltaBytes;
        SkASSERT(nextCursor >= start);
        // We still check nextCursor >= start for release builds that wouldn't assert.
        if (nextCursor <= fSize && nextCursor >= start) {
            if (nextCursor < fCursor) {
                // The allocation got smaller; poison the space that can no longer be used.
                this->poisonRange(nextCursor + 1, end);
            } else {
                // The allocation got larger; unpoison the space that can now be used.
                this->unpoisonRange(end, nextCursor);
            }

            fCursor = nextCursor;
            return true;
        }
    }
    return false;
}

// NOTE: release is equivalent to resize(start, end, start - end), and the compiler can optimize
// most of the operations away, but it wasn't able to remove the unnecessary branch comparing the
// new cursor to the block size or old start, so release() gets a specialization.
bool SkBlockAllocator::Block::release(int start, int end) {
    SkASSERT(fSentinel == kAssignedMarker);
    SkASSERT(start >= kDataStart && end <= fSize && start < end);

    this->poisonRange(start, end);

    if (fCursor == end) {
        fCursor = start;
        return true;
    } else {
        return false;
    }
}

///////// Block iteration
template <bool Forward, bool Const>
class SkBlockAllocator::BlockIter {
private:
    using BlockT = typename std::conditional<Const, const Block, Block>::type;
    using AllocatorT =
            typename std::conditional<Const, const SkBlockAllocator, SkBlockAllocator>::type;

public:
    BlockIter(AllocatorT* allocator) : fAllocator(allocator) {}

    class Item {
    public:
        bool operator!=(const Item& other) const { return fBlock != other.fBlock; }

        BlockT* operator*() const { return fBlock; }

        Item& operator++() {
            this->advance(fNext);
            return *this;
        }

    private:
        friend BlockIter;

        Item(BlockT* block) { this->advance(block); }

        void advance(BlockT* block) {
            fBlock = block;
            fNext = block ? (Forward ? block->fNext : block->fPrev) : nullptr;
            if (!Forward && fNext && fNext->isScratch()) {
                // For reverse-iteration only, we need to stop at the head, not the scratch block
                // possibly stashed in head->prev.
                fNext = nullptr;
            }
            SkASSERT(!fNext || !fNext->isScratch());
        }

        BlockT* fBlock;
        // Cache this before operator++ so that fBlock can be released during iteration
        BlockT* fNext;
    };

    Item begin() const { return Item(Forward ? &fAllocator->fHead : fAllocator->fTail); }
    Item end() const { return Item(nullptr); }

private:
    AllocatorT* fAllocator;
};

SkBlockAllocator::BlockIter<true, false> SkBlockAllocator::blocks() {
    return BlockIter<true, false>(this);
}
SkBlockAllocator::BlockIter<true, true> SkBlockAllocator::blocks() const {
    return BlockIter<true, true>(this);
}
SkBlockAllocator::BlockIter<false, false> SkBlockAllocator::rblocks() {
    return BlockIter<false, false>(this);
}
SkBlockAllocator::BlockIter<false, true> SkBlockAllocator::rblocks() const {
    return BlockIter<false, true>(this);
}

#endif // SkBlockAllocator_DEFINED
