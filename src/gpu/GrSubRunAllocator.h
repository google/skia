/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSubRunAllocator_DEFINED
#define GrSubRunAllocator_DEFINED

#include "include/core/SkSpan.h"
#include "src/core/SkArenaAlloc.h"

#include <algorithm>
#include <memory>

// GrBagOfBytes parcels out bytes with a given size and alignment.
class GrBagOfBytes {
public:
    GrBagOfBytes(char* block, size_t blockSize, size_t firstHeapAllocation);
    explicit GrBagOfBytes(size_t firstHeapAllocation = 0);
    ~GrBagOfBytes();

    // Given a requestedSize round up to the smallest size that accounts for all the per block
    // overhead and alignment. It crashes if requestedSize is negative or too big.
    static constexpr int PlatformMinimumSizeWithOverhead(int requestedSize, int assumedAlignment) {
        return MinimumSizeWithOverhead(
                requestedSize, assumedAlignment, sizeof(Block), kMaxAlignment);
    }

    static constexpr int MinimumSizeWithOverhead(
            int requestedSize, int assumedAlignment, int blockSize, int maxAlignment) {
        SkASSERT_RELEASE(0 <= requestedSize && requestedSize < kMaxByteSize);
        SkASSERT_RELEASE(SkIsPow2(assumedAlignment) && SkIsPow2(maxAlignment));

        auto alignUp = [](int size, int alignment) {return (size + (alignment - 1)) & -alignment;};

        const int minAlignment = std::min(maxAlignment, assumedAlignment);
        // There are two cases, one easy and one subtle. The easy case is when minAlignment ==
        // maxAlignment. When that happens, the term maxAlignment - minAlignment is zero, and the
        // block will be placed at the proper alignment because alignUp is properly
        // aligned.
        // The subtle case is where minAlignment < maxAlignment. Because
        // minAlignment < maxAlignment, alignUp(requestedSize, minAlignment) + blockSize does not
        // guarantee that block can be placed at a maxAlignment address. Block can be placed at
        // maxAlignment/minAlignment different address to achieve alignment, so we need
        // to add memory to allow the block to be placed on a maxAlignment address.
        // For example, if assumedAlignment = 4 and maxAlignment = 16 then block can be placed at
        // the following address offsets at the end of minimumSize bytes.
        //   0 * minAlignment =  0
        //   1 * minAlignment =  4
        //   2 * minAlignment =  8
        //   3 * minAlignment = 12
        // Following this logic, the equation for the additional bytes is
        //   (maxAlignment/minAlignment - 1) * minAlignment
        //     = maxAlignment - minAlignment.
        int minimumSize = alignUp(requestedSize, minAlignment)
                          + blockSize
                          + maxAlignment - minAlignment;

        // If minimumSize is > 32k then round to a 4K boundary unless it is too close to the
        // maximum int. The > 32K heuristic is from the JEMalloc behavior.
        constexpr int k32K = (1 << 15);
        if (minimumSize >= k32K && minimumSize < std::numeric_limits<int>::max() - k4K) {
            minimumSize = alignUp(minimumSize, k4K);
        }

        return minimumSize;
    }

    template <int size>
    using Storage = std::array<char, PlatformMinimumSizeWithOverhead(size, 1)>;

    // Returns a pointer to memory suitable for holding n Ts.
    template <typename T> char* allocateBytesFor(int n = 1) {
        static_assert(alignof(T) <= kMaxAlignment, "Alignment is too big for arena");
        static_assert(sizeof(T) < kMaxByteSize, "Size is too big for arena");
        constexpr int kMaxN = kMaxByteSize / sizeof(T);
        SkASSERT_RELEASE(0 <= n && n < kMaxN);

        int size = n ? n * sizeof(T) : 1;
        return this->allocateBytes(size, alignof(T));
    }

    void* alignedBytes(int unsafeSize, int unsafeAlignment);

private:
    // The maximum alignment supported by GrBagOfBytes. 16 seems to be a good number for alignment.
    // If a use case for larger alignments is found, we can turn this into a template parameter.
    inline static constexpr int kMaxAlignment = std::max(16, (int)alignof(std::max_align_t));
    // The largest size that can be allocated. In larger sizes, the block is rounded up to 4K
    // chunks. Leave a 4K of slop.
    inline static constexpr int k4K = (1 << 12);
    // This should never overflow with the calculations done on the code.
    inline static constexpr int kMaxByteSize = std::numeric_limits<int>::max() - k4K;
    // The assumed alignment of new char[] given the platform.
    // There is a bug in Emscripten's allocator that make alignment different than max_align_t.
    // kAllocationAlignment accounts for this difference. For more information see:
    // https://github.com/emscripten-core/emscripten/issues/10072
    #if !defined(SK_FORCE_8_BYTE_ALIGNMENT)
        inline static constexpr int kAllocationAlignment = alignof(std::max_align_t);
    #else
        inline static constexpr int kAllocationAlignment = 8;
    #endif

    // The Block starts at the location pointed to by fEndByte.
    // Beware. Order is important here. The destructor for fPrevious must be called first because
    // the Block is embedded in fBlockStart. Destructors are run in reverse order.
    struct Block {
        Block(char* previous, char* startOfBlock);
        // The start of the originally allocated bytes. This is the thing that must be deleted.
        char* const fBlockStart;
        Block* const fPrevious;
    };

    // Note: fCapacity is the number of bytes remaining, and is subtracted from fEndByte to
    // generate the location of the object.
    char* allocateBytes(int size, int alignment) {
        fCapacity = fCapacity & -alignment;
        if (fCapacity < size) {
            this->needMoreBytes(size, alignment);
        }
        char* const ptr = fEndByte - fCapacity;
        SkASSERT(((intptr_t)ptr & (alignment - 1)) == 0);
        SkASSERT(fCapacity >= size);
        fCapacity -= size;
        return ptr;
    }

    // Adjust fEndByte and fCapacity give a new block starting at bytes with size.
    void setupBytesAndCapacity(char* bytes, int size);

    // Adjust fEndByte and fCapacity to satisfy the size and alignment request.
    void needMoreBytes(int size, int alignment);

    // This points to the highest kMaxAlignment address in the allocated block. The address of
    // the current end of allocated data is given by fEndByte - fCapacity. While the negative side
    // of this pointer are the bytes to be allocated. The positive side points to the Block for
    // this memory. In other words, the pointer to the Block structure for these bytes is
    // reinterpret_cast<Block*>(fEndByte).
    char* fEndByte{nullptr};

    // The number of bytes remaining in this block.
    int fCapacity{0};

    SkFibBlockSizes<kMaxByteSize> fFibProgression;
};

// GrSubRunAllocator provides fast allocation where the user takes care of calling the destructors
// of the returned pointers, and GrSubRunAllocator takes care of deleting the storage. The
// unique_ptrs returned, are to assist in assuring the object's destructor is called.
// A note on zero length arrays: according to the standard a pointer must be returned, and it
// can't be a nullptr. In such a case, SkArena allocates one byte, but does not initialize it.
class GrSubRunAllocator {
public:
    struct Destroyer {
        template <typename T>
        void operator()(T* ptr) { ptr->~T(); }
    };

    struct ArrayDestroyer {
        int n;
        template <typename T>
        void operator()(T* ptr) {
            for (int i = 0; i < n; i++) { ptr[i].~T(); }
        }
    };

    template<class T>
    inline static constexpr bool HasNoDestructor = std::is_trivially_destructible<T>::value;

    GrSubRunAllocator(char* block, int blockSize, int firstHeapAllocation);
    explicit GrSubRunAllocator(int firstHeapAllocation = 0);

    template <typename T, typename... Args> T* makePOD(Args&&... args) {
        static_assert(HasNoDestructor<T>, "This is not POD. Use makeUnique.");
        char* bytes = fAlloc.template allocateBytesFor<T>();
        return new (bytes) T(std::forward<Args>(args)...);
    }

    template <typename T, typename... Args>
    std::unique_ptr<T, Destroyer> makeUnique(Args&&... args) {
        static_assert(!HasNoDestructor<T>, "This is POD. Use makePOD.");
        char* bytes = fAlloc.template allocateBytesFor<T>();
        return std::unique_ptr<T, Destroyer>{new (bytes) T(std::forward<Args>(args)...)};
    }

    template<typename T> T* makePODArray(int n) {
        static_assert(HasNoDestructor<T>, "This is not POD. Use makeUniqueArray.");
        return reinterpret_cast<T*>(fAlloc.template allocateBytesFor<T>(n));
    }

    template<typename T, typename Src, typename Map>
    SkSpan<T> makePODArray(const Src& src, Map map) {
        static_assert(HasNoDestructor<T>, "This is not POD. Use makeUniqueArray.");
        int size = SkTo<int>(src.size());
        T* result = this->template makePODArray<T>(size);
        for (int i = 0; i < size; i++) {
            new (&result[i]) T(map(src[i]));
        }
        return {result, src.size()};
    }

    template<typename T>
    std::unique_ptr<T[], ArrayDestroyer> makeUniqueArray(int n) {
        static_assert(!HasNoDestructor<T>, "This is POD. Use makePODArray.");
        T* array = reinterpret_cast<T*>(fAlloc.template allocateBytesFor<T>(n));
        for (int i = 0; i < n; i++) {
            new (&array[i]) T{};
        }
        return std::unique_ptr<T[], ArrayDestroyer>{array, ArrayDestroyer{n}};
    }

    template<typename T, typename I>
    std::unique_ptr<T[], ArrayDestroyer> makeUniqueArray(int n, I initializer) {
        static_assert(!HasNoDestructor<T>, "This is POD. Use makePODArray.");
        T* array = reinterpret_cast<T*>(fAlloc.template allocateBytesFor<T>(n));
        for (int i = 0; i < n; i++) {
            new (&array[i]) T(initializer(i));
        }
        return std::unique_ptr<T[], ArrayDestroyer>{array, ArrayDestroyer{n}};
    }

    void* alignedBytes(int size, int alignment);

private:
    GrBagOfBytes fAlloc;
};
#endif // GrSubRunAllocator_DEFINED
