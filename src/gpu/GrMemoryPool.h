/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMemoryPool_DEFINED
#define GrMemoryPool_DEFINED

#include "include/gpu/GrTypes.h"

#include <memory>  // std::unique_ptr
#include <cstddef> // max_align_t

#ifdef SK_DEBUG
#include "include/private/SkTHash.h"
#endif

/**
 * GrBlockAllocator provides low-level support for a block allocated arena with a dynamic tail that
 * tracks space reservations within each block. Its APIs provide the ability to reserve space,
 * resize reservations, and release reservations. It will always automatically create new blocks if
 * needed and destroy all remaining blocks when it is destructed. It assumes that anything allocated
 * within its blocks has its destructors called externally.
 *
 * GrBlockAllocator has minimal overhead, at 16 bytes per instance, 24 bytes per block, and 0 bytes
 * per reservation/allocation (plus any alignment padding). While a GrBlockAllocator can store more
 * than 2^31 bytes, each block is limited to 2^31 bytes (2GB, should be plenty...).
 */
class GrBlockAllocator {
public:
    enum class GrowthPolicy : int {
        kFixed,
        kLinear,
        kFibonacci,
        kExponential,
        kLast = kExponential
    };
    static constexpr int kGrowthPolicyCount = static_cast<int>(GrowthPolicy::kLast) + 1;

    // Smallest block size allocated on the heap (not the smallest reservation via allocate()).
    static constexpr int kMinAllocationSize = 1 << 10;
    // Largest block size allocated on the heap, reservable space will be slightly less.
    static constexpr int kMaxAllocationSize = std::numeric_limits<int32_t>::max();
    /**
     * Create a new block allocator that creates itself and its first, inline block in an allocation
     * of size 'preallocSize'. Subsequent heap-allocated blocks will also be of 'preallocSize'.
     */
    std::unique_ptr<GrBlockAllocator> Make(int preallocSize,
                                           GrowthPolicy policy = GrowthPolicy::kFixed) {
        return Prealloc<GrBlockAllocator>(preallocSize, policy, 0);
    }

    ~GrBlockAllocator();
    void operator delete(void* p) { ::operator delete(p); }

    /**
     * Reserve space that will hold 'size' bytes. Returns the pointer to the first byte, and it will
     * be aligned on 'align'. it is guaranteed that the consecutive 'size' bytes will not be
     * reserved for another allocation, unless the returned pointer is resized or released.
     *
     * This will automatically allocate a new block if there is not enough available space in the
     * current block.
     */
    void* allocate(int size, int align) { return this->allocate(size, align, 0); }

    /**
     * Return the maximum allocation size with the given alignment that can be made without
     * automatically adding a new block.
     */
    int avail(int align) { return this->avail(align, 0); }

    /**
     * Attempt to resize the allocation that starts at 'p' of 'currentSize' bytes to the
     * 'requestedSize'. If true is returned, the allocation was successfully resized to
     * 'requestedSize'. If false is returned, it remains its previous size.
     *
     * When 'requestedSize' is smaller than 'currentSize', this can be used to reclaim bytes for
     * subsequent allocation requests. If 'requestedSize' is 0 and true is returned, then 'p'
     * can no longer be used.
     */
    bool resize(void* p, int currentSize, int requestedSize) {
        return this->resize(this->currentBlock(), p, currentSize, requestedSize);
    }

    /**
     * Attempt to release the allocation 'p' of 'size' bytes. This is equivalent to resizing it to 0
     * bytes. Returns true if the allocator successfully reclaimed bytes.
     */
    bool release(void* p, int size) { return this->release(this->currentBlock(), p, size); }

    /**
     * Return the total number of allocated bytes, including the preallocation size and the
     * per-block overhead. This will always be >= preallocSize().
     */
    size_t size() const;

    /**
     * Return the total number of bytes that have been allocated by this GrBlockAllocator (e.g.
     * returned from 'allocate()' and not yet released). This also includes any per-block and
     * per-allocation overhead, since those bytes are "used". This will always be <= size().
     */
    size_t used() const;

    /**
     * Return the number of bytes allocated for block storage when the GrBlockAllocator was
     * instantiated. This includes per-block overhead of the head block and the size of
     * GrBlockAllocator itself.
     */
    int preallocSize() const { return this->head()->fSize + fHeadOffset; }

protected:
    /*
     * GrBlockAllocator provides more nuanced, protected APIs for extending allocators to access to
     * per-block and per-allocation metadata.
     *
     * In order to support higher-level constructs, GrBlockAllocator can track two types of
     * metadata: per-block and per-allocation. The allocator can reserve a fixed number of bytes at
     * the start of each block for metadata for the higher-level allocator. The size of this
     * metadata is provided as a constructor arg and will always be zero-initialized and aligned to
     * "void*" when a new block is made. Additionally, each call to 'allocate' can specify a
     * per-allocation number of bytes that are packed, *unaligned*, directly in front of the
     * allocation's returned pointer.
     *
     * These protected APIs are similar to the public allocate/avail/resize/release calls except
     * that they accept a per-allocation number of bytes for metadata, and have a version that
     * takes the block to operate on (not including allocations, which always come from the current
     * tail block).
     */

    static constexpr int kAssignedMarker = 0xCDCDCDCD;
    static constexpr int kFreedMarker    = 0xEFEFEFEF;

    // An opaque way to address the blocks managed by the GrBlockAllocator
    using Block = uintptr_t;
    using VisitBlockProc = int (Block, int); // int arg and return are sole captured data for proc.

    // sizeof(allocator) <= (preallocStart - this) <= 255
    GrBlockAllocator(void* preallocStart, int preallocSize, GrowthPolicy policy,
                     int blockMetadataBytes);

    /**
     * Helper function to allocate and initialize the allocator of type T, such that its size
     * plus the inline head block fill 'preallocSize'. T's constructor must be accessible to
     * GrBlockAllocator, and must have (void* preallocStart, int preallocSize) as its first two
     * arguments, followed by the remaining CtorArgs template values.
     */
    template <typename T, typename... CtorArgs>
    static std::unique_ptr<T> Prealloc(int preallocSize, CtorArgs... ctorArgs) {
        static constexpr size_t kPreallocOffset = GrSizeAlignUp(sizeof(T), alignof(BlockHeader));
        SkASSERT(GrSizeAlignUp(sizeof(T) + sizeof(BlockHeader), alignof(BlockHeader)) < kMinAllocationSize);
        if (preallocSize < kMinAllocationSize) {
            preallocSize = kMinAllocationSize;
        } else if (preallocSize > kMaxAllocationSize) {
            return nullptr; // FIXME or abort?
        }
        void* mem = operator new(preallocSize);
        void* preallocStart = static_cast<char*>(mem) + kPreallocOffset;
        return std::unique_ptr<T>(new (mem) T(SkTo<int>(preallocStart), preallocSize, std::forward<CtorArgs>(ctorArgs)...));
    }

    /**
     * Reserve space that will hold 'size' bytes. Returns the pointer to the first byte, and it will
     * be aligned on 'align'. it is guaranteed that the consecutive 'size' bytes will not be
     * reserved for another allocation, unless the returned pointer is resized or released.
     *
     * If 'allocMetadataBytes' is not zero, and 'p' is the returned pointer to the allocation of
     * 'size' bytes, then 'p - allocMetadataBytes' points to zero-initialized metadata for the
     * allocation. Since it is stored as a prefix to the allocation, the alloc can still be resized
     * later. The limitation is that 'p - allocMetadataBytes' has no guaranteed alignment, so memcpy
     * must be used to update it if it's storing aggregate data.
     *
     * This will automatically allocate a new block if there is not enough available space in the
     * current block.
     */
    void* allocate(int size, int align, int allocMetadataBytes);

    /**
     * Return the maximum allocation size with the given alignment and number of unaligned metadata
     * bytes that can be made without automatically adding a new block. The returned size does not
     * include 'allocMetadataBytes', but represents the size that would be usable by the allocation.
     */
    int avail(int align, int allocMetadataBytes) const {
        return this->avail(this->currentBlock(), align, allocMetadataBytes);
    }

    /**
     * Return the amount of free space at the end of the given block, respecting the given alignment
     * and number of per-alloc metadata bytes. Even if the block is not the current block, this
     * provides an upper bound on the amount that existing allocations associated with 'block' can
     * be resized up to.
     */
    int avail(Block block, int align, int allocMetadataBytes) const {
        const BlockHeader* blockPtr = BlockToHeader(block);
        return blockPtr->fSize - blockPtr->cursor(align, allocMetadataBytes);
    }

    /**
     * Attempt to resize the allocation at 'p' of 'currentSize' bytes to 'requestedSize'. It is
     * assumed that 'p' was allocated in the block represented by the provided block pointer.
     * 'block' must have been previously returned by currentBlock() or addBlock(), and not been
     * manually released.
     */
    bool resize(Block block, void* p, int currentSize, int requestedSize);

    /**
     * Release the allocation 'ptr' of 'size' bytes back to the given block pointer. 'block' must
     * have been previously returned by currentBlock() or addBlock(), and not been manually
     * released.
     */
    bool release(Block block, void* ptr, int size) {
        return this->resize(block, ptr, size, 0) == 0;
    }

    /**
     * Helper to release the allocation 'ptr' of 'size' bytes, along with the specified number of
     * per-allocation bytes.
     */
    bool release(Block block, void* ptr, int size, int allocMetadataBytes) {
        // This recovers everything but the unknowable amount of alignment padding
        void* metaPtr = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(p) - allocMetadataBytes);
        return this->release(block, metaPtr, size + allocMetadataBytes);
    }

    /**
     * Return a pointer to the start of the current block. This pointer will have the same alignment
     * as std::max_align_t and represents the per-block, zero-initialized metadata reserved for
     * allocators that build off of GrBlockAllocator. The number of bytes available was determined
     * by 'metadataBytes' at construction. This will change only when a new block is allocated
     * (either automatically or explicitly with addBlock()).
     *
     * The pointer returned by currentBlockMetadata() after a call to allocate() will always
     * represent the block the allocation belonged to. This block pointer can then be externally
     * associated with the allocation and used to release or resize the allocation even after
     * additional blocks have been added to the GrBlockAllocator. The difference between any
     * allocation's pointer and its block metadata pointer will fit into 32 bits.
     */
    Block currentBlock() const { return reinterpret_cast<Block>(fTail); }

    /**
     * Add a new block to allocator. The new block will be sized to the max of the allocator's
     * growth policy, or the size required to fulfill and a call to 'allocate(minimumSize, align,
     * allocMetadataBytes)'. Returns the pointer for the new block, and all metadata bytes will have
     * been zeroed. currentBlock() will return this same pointer until a new block is added.
     */
    Block addBlock(int minimumSize, int align, int allocMetadataBytes);

    /**
     * Explicitly free an entire block (and any of the reserved allocations in that block). The
     * GrBlockAllocator only releases blocks automatically when it is finally destroyed. This
     * function can be used to reclaim memory over the lifetime of the allocator. The provided
     * 'block' pointer must have previously come from a call to currentBlock() or addBlock() and not
     * have been released already.
     *
     * This does nothing if 'block' represents the inline-allocated head block.
     */
    void releaseBlock(Block block);

    /**
     * In reverse order (tail to head), invoke 'dtor' on each allocated block and then delete the
     * heap allocated block after 'dtor' returns. 'dtor' is responsible for invoking any embedded
     * destructors for the allocations within the block, etc. but can be a no-op if the block only
     * holds POD. Subclasses can use this utility function in their destructor if more cleanup is
     * needed than just freeing the blocks.
     */
    void releaseAllBlocks(VisitBlockProc dtor);

    /**
     * Get a pointer to the mutable block metadata for 'block'.
     */
    template <typename T>
    static inline T* BlockMetadata(Block block) {
        static_assert(alignof(T) <= alignof(BlockHeader));
        SkASSERT(reinterpret_cast<BlockHeader*>(block)->fSentinel == kAssignedMarker);
        return reinterpret_cast<T*>(block + sizeof(BlockHeader));
    }

    /**
     * Get the per-allocation metadata from 'p', which must have been returned by 'allocate' with
     * sizeof(T) specified for 'allocMetadataBytes'.
     */
    template <typename T>
    static inline T GetMetadata(void* p) {
        void* metaPtr = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(p) - sizeof(T));
        T metadata;
        memcpy(&metadata, metaPtr, sizeof(T));
        return metadata;
    }

    /**
     * Set the per-allocation metadata values for 'p', which must have been returned by 'allocate'
     * with sizeof(T) specified for 'allocMetadataBytes'.
     */
    template <typename T>
    static inline void SetMetadata(void* p, const T& metadata) {
        void* metaPtr = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(p) - sizeof(T));
        memcpy(metaPtr, &metadata, sizeof(T));
    }

#ifdef SK_DEBUG
    int validate(VisitBlockProc visitor) const;
#endif

private:
    // By requiring max alignment of the block header, there is no additional padding that needs
    // to be accounted for when advancing to the start of the per-block allocation data.
    struct BlockHeader {
        BlockHeader(int size, int metadataBytes);

        ~BlockHeader();
        void operator delete(void* p) { ::operator delete(p); }

        // Return the cursor, aligned to 'align' with prefixed padding for 'allocMetadataBytes'.
        // Guaranteed to be between [0, fSize], although if alignment and metadata would cause it
        // to exceed fSize it will be clamped to fSize.
        int cursor(int align, int allocMetadataBytes) const;

#ifdef SK_DEBUG
        int32_t      fSentinel;  // known value to check for bad back pointers to blocks
#endif
        BlockHeader* fNext;      // doubly-linked list of blocks.
        BlockHeader* fPrev;

        // Each block tracks its own cursor because as later blocks are released, an older block
        // may become the active tail again. However, since block allocations are capped to uint32,
        // we can pack both the block size and block cursor into 32 bits each.
        int32_t       fSize;      // includes the size of the BlockHeader and requested metadata
        int32_t       fCursor;    // (this + fCursor) points to next available allocation
    };

    // The head block is always a fixed offset from 'this' since it is allocated inline.
    BlockHeader* head() {
        return reinterpret_cast<BlockHeader*>(reinterpret_cast<uintptr_t>(this) + fHeadOffset);
    }
    const BlockHeader* head() const { return const_cast<GrBlockAllocator*>(this)->head(); }

    static inline BlockHeader* BlockToHeader(Block block) {
        BlockHeader* b = reinterpret_cast<BlockHeader*>(block);
        SkASSERT(b->fSentinel == kAssignedMarker);
        return b;
    }

    BlockHeader* fTail; // All non-head blocks are heap allocated

    // All remaining state is packed into 64 bits to keep GrBlockAllocator at 16 bytes.

    // Byte offset from 'this' that points to the head BlockHeader that is inline allocated with the
    // GrBlockAllocator. At 8 bits, the prefix size of GrBlockAllocator (and subclass) is limited to
    // 255 bytes. Currently this is not a problem, but can be adjusted in the future if needed.
    uint64_t     fHeadOffset    : 8;
    // Number of extra bytes to include after the BlockHeader, rounded up to max_align_t
    // At 8 bits, limits block metadata size to 255 bytes, which is currently not a limitation.
    uint64_t     fMetadataBytes : 8;
    // Growth of the block size is controlled by two factors N0 and N1, as well as a rule that
    // controls how N0 is updated. When a new block is needed, we calculdate N1' = N0 + N1.
    // Depending on the growth policy, N0' = N0 (no growth or linear growth), or N0' = N1
    // (Fibonacci), or N0' = N1' (exponential). The size of the new block is N1' * preallocSize,
    // after which fN0 and fN1 store N0' and N1' clamped into 23 bits. With current bit allocations,
    // N1' is limited to 2^24, so 'preallocSize' must be at least 2^7 in order for block allocations
    // to eventually hit the hard 2^31 limit block size for GrBlockAllocator. preallocSize is
    // forced to be 2^10 by kMinAllocationSize, so the bit limits are fine.
    uint64_t     fGrowthPolicy  : 2;  // GrowthPolicy
    uint64_t     fN0            : 23; // = 1 for linear/exp.; = 0 for fixed/fibonacci, initially
    uint64_t     fN1            : 23; // = 1 initially

    // Confirm bit and struct sizings
    static_assert(kGrowthPolicyCount <= 4);
#ifndef SK_DEBUG
    static_assert(sizeof(BlockHeader) <= 24);
#endif
};

static_assert(sizeof(GrBlockAllocator) <= 16);

class GrArenaAlloc {
    // Allows dynamic alignment in its allocate calls
    // Provides templated versions to alloc typed objects
    // Does not expose any release/resize functions
    // Adds a variable amount of space per allocation that stores its destructor function pointer.
    //   - does not bother releasing it (although we could add that support if we give it a non-monotonic edge)
    //   - has no overhead for POD data that is added
    // Updates its destructor to run backwards through its destructor function pointers, but allows
    //   the BlockAllocator to just delete the linked list of blocks
};

/**
 * Allocates memory in blocks and parcels out space in the blocks for allocation requests. All
 * heap-allocated blocks will It is optimized for allocate / release speed over memory efficiency.
 * The interface is designed to be used to implement operator new and delete overrides. All
 * allocations are expected to be released before the pool's destructor is called. Allocations will
 * be aligned to sizeof(std::max_align_t).
 *
 * All allocated objects must be released back to the memory pool before it can be destroyed.
 */
class GrMemoryPool : protected GrBlockAllocator {
public:
    // Guaranteed alignment of pointer returned by allocate().
    static constexpr size_t kAlignment = alignof(std::max_align_t);

    using GrBlockAllocator::kMinAllocationSize;

    /**
     * Prealloc size is the amount of space to allocate at pool creation time and keep around until
     * pool destruction.
     *
     * The size is what the pool will end up allocating from the system, and portions of the
     * allocated memory are used for internal bookkeeping. If preallocSize < minPreallocSize it will
     * be increased to accommodate.
     */
    static std::unique_ptr<GrMemoryPool> Make(size_t preallocSize) {
        return GrBlockAllocator::Prealloc<GrMemoryPool>((uint)(preallocSize));
    }

    ~GrMemoryPool();
    void operator delete(void* p) { ::operator delete(p); }

    /**
     * Allocates memory. The memory must be freed with release() before the GrMemoryPool is deleted.
     */
    void* allocate(size_t size);

    /**
     * p must have been returned by allocate().
     */
    void release(void* p);

    /**
     * Returns true if there are no unreleased allocations.
     */
    bool isEmpty() const {
        // If size is the same as preallocSize, there aren't any heap blocks, so currentBlock()
        // is the inline head block.
        return 0 >= this->size() &&
               0 == *(BlockMetadata<LiveCount>(this->currentBlock()));
    }

    /**
     * Returns the total allocated size of the GrMemoryPool minus any preallocated amount
     * (does not include the preallocSize for historic reasons).
     */
    size_t size() const { return this->GrBlockAllocator::size() - this->preallocSize(); }

    /**
     * Returns the preallocated size of the GrMemoryPool
     */
    using GrBlockAllocator::preallocSize;

private:
    friend class GrBlockAllocator; // For private constructor
    friend class GrOpMemoryPool;   // ""

    // GrMemoryPool adds 8 bytes of per-allocation overhead so that it can always identify the
    // block an allocation came from and the size of the allocation, which is required for it to
    // implement on-the-fly block reclaiming.
    struct AllocHeader {
#ifdef SK_DEBUG
        int32_t fSentinel;      // known value to check for memory stomping (e.g., (CD)*)
        int32_t fID;            // ID that can be used to track down leaks by clients.
#endif
        int32_t fOffset;        // offset from block ptr to the allocated ptr
        int32_t fSize;          // size of the allocation
    };

    // Track live reservations in each block. Since block metadata is zero-initialized for a new
    // block by GrBlockAllocator, it starts with the expected value.
    using LiveCount = int32_t;

    GrMemoryPool(void* preallocStart, int preallocSize);

#ifdef SK_DEBUG
    void validate() const;

    SkTHashSet<int> fAllocatedIDs;
    int             fAllocationCount;
#endif
};

class GrOp;

/**
 * A specialized variant on GrMemoryPool that provides convenience functions for allocating and
 * releasing GrOps.
 */
class GrOpMemoryPool : private GrMemoryPool {
public:
    static std::unique_ptr<GrOpMemoryPool> Make(size_t preallocSize) {
        return GrBlockAllocator::Prealloc<GrOpMemoryPool>((int)(preallocSize));
    }

    void operator delete(void* p) { ::operator delete(p); }

    template <typename Op, typename... OpArgs>
    std::unique_ptr<Op> allocate(OpArgs&&... opArgs) {
        auto mem = this->allocate(sizeof(Op));
        return std::unique_ptr<Op>(new (mem) Op(std::forward<OpArgs>(opArgs)...));
    }

    void release(std::unique_ptr<GrOp> op);

    using GrMemoryPool::allocate;
    using GrMemoryPool::isEmpty;

private:
    friend class GrBlockAllocator; // For private constructor

    GrOpMemoryPool(void* preallocStart, int preallocSize)
            : GrMemoryPool(preallocStart, preallocSize) {}
};

#endif
