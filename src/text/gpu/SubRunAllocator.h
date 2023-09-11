/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sktext_gpu_SubRunAllocator_DEFINED
#define sktext_gpu_SubRunAllocator_DEFINED

#include "include/core/SkSpan.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkMath.h"
#include "include/private/base/SkTLogic.h"
#include "include/private/base/SkTemplates.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkArenaAlloc.h"

#include <algorithm>
#include <array>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <memory>
#include <new>
#include <tuple>
#include <utility>

namespace sktext::gpu {

// BagOfBytes parcels out bytes with a given size and alignment.
class BagOfBytes {
public:
    BagOfBytes(char* block, size_t blockSize, size_t firstHeapAllocation);
    explicit BagOfBytes(size_t firstHeapAllocation = 0);
    BagOfBytes(const BagOfBytes&) = delete;
    BagOfBytes& operator=(const BagOfBytes&) = delete;
    BagOfBytes(BagOfBytes&& that)
            : fEndByte{std::exchange(that.fEndByte, nullptr)}
            , fCapacity{that.fCapacity}
            , fFibProgression{that.fFibProgression} {}
    BagOfBytes& operator=(BagOfBytes&& that) {
        this->~BagOfBytes();
        new (this) BagOfBytes{std::move(that)};
        return *this;
    }

    ~BagOfBytes();

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
        int minimumSize = SkToInt(AlignUp(requestedSize, minAlignment))
                          + blockSize
                          + maxAlignment - minAlignment;

        // If minimumSize is > 32k then round to a 4K boundary unless it is too close to the
        // maximum int. The > 32K heuristic is from the JEMalloc behavior.
        constexpr int k32K = (1 << 15);
        if (minimumSize >= k32K && minimumSize < std::numeric_limits<int>::max() - k4K) {
            minimumSize = SkToInt(AlignUp(minimumSize, k4K));
        }

        return minimumSize;
    }

    template <int size>
    using Storage = std::array<char, PlatformMinimumSizeWithOverhead(size, 1)>;

    // Returns true if n * sizeof(T) will fit in an allocation block.
    template <typename T>
    static bool WillCountFit(int n) {
        constexpr int kMaxN = kMaxByteSize / sizeof(T);
        return 0 <= n && n < kMaxN;
    }

    // Returns a pointer to memory suitable for holding n Ts.
    template <typename T> char* allocateBytesFor(int n = 1) {
        static_assert(alignof(T) <= kMaxAlignment, "Alignment is too big for arena");
        static_assert(sizeof(T) < kMaxByteSize, "Size is too big for arena");
        SkASSERT_RELEASE(WillCountFit<T>(n));

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
        static constexpr int kAllocationAlignment = alignof(std::max_align_t);
    #else
        static constexpr int kAllocationAlignment = 8;
    #endif

    static constexpr size_t AlignUp(int size, int alignment) {
        return (size + (alignment - 1)) & -alignment;
    }

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

template <typename T>
class SubRunInitializer {
public:
    SubRunInitializer(void* memory) : fMemory{memory} { SkASSERT(memory != nullptr); }
    ~SubRunInitializer() {
        ::operator delete(fMemory);
    }
    template <typename... Args>
    T* initialize(Args&&... args) {
        // Warn on more than one initialization.
        SkASSERT(fMemory != nullptr);
        return new (std::exchange(fMemory, nullptr)) T(std::forward<Args>(args)...);
    }

private:
    void* fMemory;
};

// GrSubRunAllocator provides fast allocation where the user takes care of calling the destructors
// of the returned pointers, and GrSubRunAllocator takes care of deleting the storage. The
// unique_ptrs returned, are to assist in assuring the object's destructor is called.
// A note on zero length arrays: according to the standard a pointer must be returned, and it
// can't be a nullptr. In such a case, SkArena allocates one byte, but does not initialize it.
class SubRunAllocator {
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

    SubRunAllocator(char* block, int blockSize, int firstHeapAllocation);
    explicit SubRunAllocator(int firstHeapAllocation = 0);
    SubRunAllocator(const SubRunAllocator&) = delete;
    SubRunAllocator& operator=(const SubRunAllocator&) = delete;
    SubRunAllocator(SubRunAllocator&&) = default;
    SubRunAllocator& operator=(SubRunAllocator&&) = default;

    template <typename T>
    static std::tuple<SubRunInitializer<T>, int, SubRunAllocator>
    AllocateClassMemoryAndArena(int allocSizeHint) {
        SkASSERT_RELEASE(allocSizeHint >= 0);
        // Round the size after the object the optimal amount.
        int extraSize = BagOfBytes::PlatformMinimumSizeWithOverhead(allocSizeHint, alignof(T));

        // Don't overflow or die.
        SkASSERT_RELEASE(INT_MAX - SkTo<int>(sizeof(T)) > extraSize);
        int totalMemorySize = sizeof(T) + extraSize;

        void* memory = ::operator new (totalMemorySize);
        SubRunAllocator alloc{SkTAddOffset<char>(memory, sizeof(T)), extraSize, extraSize/2};
        return {memory, totalMemorySize, std::move(alloc)};
    }

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

    template<typename T>
    SkSpan<T> makePODSpan(SkSpan<const T> s) {
        static_assert(HasNoDestructor<T>, "This is not POD. Use makeUniqueArray.");
        if (s.empty()) {
            return SkSpan<T>{};
        }

        T* result = this->makePODArray<T>(SkTo<int>(s.size()));
        memcpy(result, s.data(), s.size_bytes());
        return {result, s.size()};
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

    template<typename T, typename U, typename Map>
    std::unique_ptr<T[], ArrayDestroyer> makeUniqueArray(SkSpan<const U> src, Map map) {
        static_assert(!HasNoDestructor<T>, "This is POD. Use makePODArray.");
        int count = SkCount(src);
        T* array = reinterpret_cast<T*>(fAlloc.template allocateBytesFor<T>(src.size()));
        for (int i = 0; i < count; ++i) {
            new (&array[i]) T(map(src[i]));
        }
        return std::unique_ptr<T[], ArrayDestroyer>{array, ArrayDestroyer{count}};
    }

    void* alignedBytes(int size, int alignment);

private:
    BagOfBytes fAlloc;
};

// Helper for defining allocators with inline/reserved storage.
// For argument declarations, stick to the base type (SubRunAllocator).
// Note: Inheriting from the storage first means the storage will outlive the
// SubRunAllocator, letting ~SubRunAllocator read it as it calls destructors.
// (This is mostly only relevant for strict tools like MSAN.)
template <size_t InlineStorageSize, size_t InlineStorageAlignment>
class STSubRunAllocator : private std::array<char,
                                             BagOfBytes::PlatformMinimumSizeWithOverhead(
                                                     InlineStorageSize, InlineStorageAlignment)>,
                          public SubRunAllocator {
public:
    explicit STSubRunAllocator(size_t firstHeapAllocation = InlineStorageSize)
            : SubRunAllocator{this->data(), SkToInt(this->size()), SkToInt(firstHeapAllocation)} {}
};
}  // namespace sktext::gpu

#endif // sktext_gpu_SubRunAllocator_DEFINED
