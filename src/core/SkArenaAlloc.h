/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkArenaAlloc_DEFINED
#define SkArenaAlloc_DEFINED

#include "include/core/SkMath.h"
#include "include/core/SkTypes.h"
#include "include/private/SkTFitsIn.h"
#include "src/core/SkEnumerate.h"
#include "src/core/SkSpan.h"

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>
#include <vector>

// SkDestroyer and SkDestroyerPtr together provide a way to call the destructors of an object
// when SkDestroyerPtr goes out of scope. Notice, this system only calls the destructors, and
// does not delete the object.
template<typename T> struct SkDestroyer { void operator()(T* o) const { o->~T(); } };
template<typename T> using SkDestroyerPtr = std::unique_ptr<T, SkDestroyer<T>>;

// SkDestroyerSpan provides a span that calls the destructors on each item in the span. Notice,
// it does not delete the underlying array.
template<typename T>
class SkDestroyerSpan : public SkSpan<T> {
public:
    SkDestroyerSpan() : SkSpan<T>(nullptr, 0) {}
    SkDestroyerSpan(SkSpan<T> span) : SkSpan<T>{span} {}
    SkDestroyerSpan(const SkDestroyerSpan&) = delete;
    SkDestroyerSpan& operator=(const SkDestroyerSpan&) = delete;
    SkDestroyerSpan(SkDestroyerSpan&& that) : SkSpan<T>{that.release()} {}
    SkDestroyerSpan& operator=(SkDestroyerSpan&& that) {
        this->~SkDestroyerSpan();
        new (this) SkDestroyerSpan{that.release()};
        return *this;
    }
    ~SkDestroyerSpan() { for (auto& t : *this) { t.~T(); } }
    SkSpan<T> release() {
        SkSpan result{this->data(), this->size()};
        new (this) SkDestroyerSpan{};
        return result;
    }
};

// SkArena provides fast allocation where the user takes care of calling the destructors of the
// returned pointers, and SkArena takes care of deleting the storage. SkDestroyerPtr and
// SkDestroyer span are provided to help track what needs to be destructed.
class SkArena {
public:
    SkArena(char* block, size_t blockSize, size_t firstHeapAllocation);
    explicit SkArena(size_t firstHeapAllocation = 0);
    ~SkArena();

    template <typename T, typename... Args> T* makePOD(Args&&... args) {
        static_assert(std::is_trivially_destructible<T>::value, "This is not POD. Use make.");
        return this->template innerMake<T>(std::forward<Args>(args)...);
    }

    template <typename T, typename... Args> SkDestroyerPtr<T> make(Args&&... args) {
        static_assert(!std::is_trivially_destructible<T>::value, "This is POD. Use makePOD.");
        return SkDestroyerPtr<T>{this->template innerMake<T>(std::forward<Args>(args)...)};
    }

    template<typename T> T* makePODArray(size_t size) {
        static_assert(std::is_trivially_destructible<T>::value, "This is not POD. Use makeArray.");
        return this->template innerMakeArray<T>(size);
    }

    template<typename T> SkDestroyerSpan<T> makeArray(size_t size) {
        static_assert(!std::is_trivially_destructible<T>::value, "This is POD. Use makePODArray.");
        SkSpan<T> array{this->template innerMakeArray<T>(size), size};
        for (auto& e : array) {
            new (&e) T{};
        }
        return SkDestroyerSpan<T>{array};
    }

    template<typename T, typename I> SkDestroyerSpan<T> makeArray(size_t size, I&& initer) {
        static_assert(!std::is_trivially_destructible<T>::value, "This is POD. Use makePODArray.");
        SkSpan<T> array{this->template innerMakeArray<T>(size), size};
        for (size_t i = 0; i < array.size(); i++) {
            new (&array[i]) T(initer(i));
        }
        return SkDestroyerSpan<T>{array};
    }

    static constexpr ptrdiff_t MinimumSizeWithOverhead(size_t requestedSize) {
        constexpr ptrdiff_t kMallocRounding = kMaxAlignment - alignof(max_align_t);
        constexpr ptrdiff_t k4K = (1 << 12);
        constexpr ptrdiff_t k32K = (1 << 15);

        // The minimumSize is the amount to allocate to assure a pointer with kMaxAlignment
        // alignment and at least size requiredSize + sizeof(Block);
        ptrdiff_t minimumSize =
                AlignUp(
                        requestedSize + kMallocRounding + sizeof(Block), alignof(max_align_t));

        // If minimumSize is > 32k then round to a 4K bondary. The > 32K heuristic is from the
        // JEMalloc behavior.
        if (minimumSize >= k32K) {
            minimumSize = AlignUp(minimumSize, k4K);
        }

        return minimumSize;
    }

    char* alignedBytes(size_t size, size_t alignment);

private:
    static constexpr ptrdiff_t kMaxAlignment = 64;
    // The largest size that can be allocated. Includes maximum padding and fudge for the Block.
    // This should never overflow with the calculations done on the code.
    static constexpr ptrdiff_t kMaxByteSize =
            std::numeric_limits<int32_t>::max() - 2*kMaxAlignment - 32;

    class Block {
    public:
        Block();
        Block(char* bytes);
        Block(char* previous, char* startOfBlock);
        void store(char* storage);
        static void DeleteBlocks(char* bytes);

    private:
        // Points to the storage of the previous block.
        char*  fPrevious{nullptr};

        // If fStartOfBlock is nullptr then this is a block provided from outside, and not owned.
        char* fStartOfBlock{nullptr};
    };

    template <typename T, typename... Args> T* innerMake(Args&&... args) {
        static_assert(alignof(T) <= kMaxAlignment, "Alignment is too big for arena");
        static_assert(sizeof(T) < kMaxByteSize, "Size is too big for arena");
        constexpr ptrdiff_t size = SkTo<ptrdiff_t>(sizeof(T));
        constexpr ptrdiff_t alignment = SkTo<ptrdiff_t>(alignof(T));

        fCapacity = AlignDown(fCapacity, alignment);
        if (fCapacity < size) {
            this->needMoreBytes(size, alignment);
        }
        T* object = new (fBytes - fCapacity) T {std::forward<Args>(args)...};

        // Assure that we have capacity to satisfy the size request.
        SkASSERT(fCapacity >= size);
        fCapacity -= size;

        // Check alignment of object.
        SkASSERT(AlignDown((char*)object, alignment) == (char*)object);
        return object;
    }

    template <typename T> T* innerMakeArray(size_t size) {
        static_assert(alignof(T) <= kMaxAlignment, "Alignment is too big for arena");
        constexpr ptrdiff_t kMaxSize = kMaxByteSize / sizeof(T);
        SkASSERT_RELEASE(size < kMaxSize);
        return (T*)this->alignedBytes(sizeof(T) * size, alignof(T));
    }

    static constexpr char* CalculateStartingBytes(char* block, ptrdiff_t size);
    static constexpr ptrdiff_t CalculateStartingCapacity(char* block, char* bytes);

    static constexpr ptrdiff_t AlignDown(ptrdiff_t v, ptrdiff_t alignment) {
        // Make sure alignment is a power of 2.
        SkASSERT((alignment & (alignment - 1)) == 0);
        return v & -alignment;  // -alignment == ~(alignment - 1)
    }
    static constexpr ptrdiff_t AlignUp(ptrdiff_t v, ptrdiff_t alignment) {
        return AlignDown(v + alignment - 1, alignment);
    }
    static char* AlignDown(char* ptr, ptrdiff_t alignment);

    // Adjust fBytes and fCapacity to satisfy the size and alignment request.
    void needMoreBytes(ptrdiff_t size, ptrdiff_t alignment);

    // This points to the highest kMaxAlignment address in the allocated block. The address of
    // the current end of allocated data is given by fBytes - fCapacity.
    char* fBytes{nullptr};

    // The number of bytes remaining in this block.
    ptrdiff_t fCapacity{0};

    // We found allocating strictly doubling amounts of memory from the heap left too
    // much unused slop, particularly on Android.  Instead we'll follow a Fibonacci-like
    // progression that's simple to implement and grows with roughly a 1.6 exponent:
    //
    // To start,
    //    fNextHeapAlloc = fYetNextHeapAlloc = 1*fFirstHeapAllocationSize;
    //
    // And then when we do allocate, follow a Fibonacci f(n+2) = f(n+1) + f(n) rule:
    //    void* block = malloc(fNextHeapAlloc);
    //    std::swap(fNextHeapAlloc, fYetNextHeapAlloc)
    //    fYetNextHeapAlloc += fNextHeapAlloc;
    //
    // That makes the nth allocation fib(n) * fFirstHeapAllocationSize bytes.
    uint32_t fNextHeapAlloc,     // How many bytes minimum will we allocate next from the heap?
             fYetNextHeapAlloc;  // And then how many the next allocation after that?
};

// Helper for defining allocators with inline/reserved storage.
// For argument declarations, stick to the base type (SkArena).
// Note: Inheriting from the storage first means the storage will outlive the
// SkArenaAlloc, letting ~SkArenaAlloc read it as it calls destructors.
// (This is mostly only relevant for strict tools like MSAN.)
template <size_t InlineStorageSize>
class SkSTArena : private std::array<char,
        SkArena::MinimumSizeWithOverhead(InlineStorageSize)>, public SkArena {
public:
    explicit SkSTArena(size_t firstHeapAllocation =
    MinimumSizeWithOverhead(InlineStorageSize))
            : SkArena{this->data(), this->size(), firstHeapAllocation} {}
};

// SkArenaAlloc allocates object and destroys the allocated objects when destroyed. It's designed
// to minimize the number of underlying block allocations. SkArenaAlloc allocates first out of an
// (optional) user-provided block of memory, and when that's exhausted it allocates on the heap,
// starting with an allocation of firstHeapAllocation bytes.  If your data (plus a small overhead)
// fits in the user-provided block, SkArenaAlloc never uses the heap, and if it fits in
// firstHeapAllocation bytes, it'll use the heap only once. If 0 is specified for
// firstHeapAllocation, then blockSize is used unless that too is 0, then 1024 is used.
//
// Examples:
//
//   char block[mostCasesSize];
//   SkArenaAlloc arena(block, mostCasesSize);
//
// If mostCasesSize is too large for the stack, you can use the following pattern.
//
//   std::unique_ptr<char[]> block{new char[mostCasesSize]};
//   SkArenaAlloc arena(block.get(), mostCasesSize, almostAllCasesSize);
//
// If the program only sometimes allocates memory, use the following pattern.
//
//   SkArenaAlloc arena(nullptr, 0, almostAllCasesSize);
//
// The storage does not necessarily need to be on the stack. Embedding the storage in a class also
// works.
//
//   class Foo {
//       char storage[mostCasesSize];
//       SkArenaAlloc arena (storage, mostCasesSize);
//   };
//
// In addition, the system is optimized to handle POD data including arrays of PODs (where
// POD is really data with no destructors). For POD data it has zero overhead per item, and a
// typical per block overhead of 8 bytes. For non-POD objects there is a per item overhead of 4
// bytes. For arrays of non-POD objects there is a per array overhead of typically 8 bytes. There
// is an addition overhead when switching from POD data to non-POD data of typically 8 bytes.
//
// If additional blocks are needed they are increased exponentially. This strategy bounds the
// recursion of the RunDtorsOnBlock to be limited to O(log size-of-memory). Block size grow using
// the Fibonacci sequence which means that for 2^32 memory there are 48 allocations, and for 2^48
// there are 71 allocations.
class SkArenaAlloc {
    static constexpr ptrdiff_t kMaxAlignment = 1024;
    // The largest size that can be allocated. Includes maximum padding and fudge for footers.
    // This should never overflow with the calculations done on the code.
    static constexpr ptrdiff_t kMaxSize = std::numeric_limits<int32_t>::max() - kMaxAlignment - 32;
public:
    SkArenaAlloc(char* block, size_t blockSize, size_t firstHeapAllocation);

    explicit SkArenaAlloc(size_t firstHeapAllocation)
        : SkArenaAlloc(nullptr, 0, firstHeapAllocation) {}

    ~SkArenaAlloc();

    template <typename T, typename... Args>
    T* make(Args&&... args) {
        static_assert(sizeof(T) < kMaxSize, "Size is too big for arena");
        static_assert(alignof(T) <= kMaxAlignment, "Alignment is too big for arena");
        constexpr ptrdiff_t size = sizeof(T);
        constexpr ptrdiff_t alignment = alignof(T);
        constexpr ptrdiff_t mask = alignment - 1;
        char* objStart;
        if (std::is_trivially_destructible<T>::value) {
            objStart = this->allocObject(size, alignment, mask);

            fCursor = objStart + size;
        } else {
            objStart = this->allocObjectWithFooter(size + sizeof(Footer), alignment, mask);

            // Can never be UB because max value is alignof(T).
            uint32_t padding = ToU32(objStart - fCursor);

            // Advance to end of object to install footer.
            fCursor = objStart + size;
            FooterAction* releaser = [](char* objEnd) {
                char* objStart = objEnd - (sizeof(T) + sizeof(Footer));
                ((T*)objStart)->~T();
                return objStart;
            };
            this->installFooter(releaser, padding);
        }

        // Make sure everything fits.
        SkASSERT(fCursor <= fEnd);

        // Must be aligned.
        SkASSERT((reinterpret_cast<intptr_t>(objStart) & mask) == 0);

        // This must be last to make objects with nested use of this allocator work.
        return new(objStart) T(std::forward<Args>(args)...);
    }

    template <typename T>
    T* makeArrayDefault(size_t count) {
        T* array = this->allocUninitializedArray<T>(count);
        for (size_t i = 0; i < count; i++) {
            // Default initialization: if T is primitive then the value is left uninitialized.
            new (&array[i]) T;
        }
        return array;
    }

    template <typename T>
    T* makeArray(size_t count) {
        T* array = this->allocUninitializedArray<T>(count);
        for (size_t i = 0; i < count; i++) {
            // Value initialization: if T is primitive then the value is zero-initialized.
            new (&array[i]) T();
        }
        return array;
    }

    template <typename T, typename Initializer>
    T* makeInitializedArray(size_t count, Initializer initializer) {
        T* array = this->allocUninitializedArray<T>(count);
        for (size_t i = 0; i < count; i++) {
            new (&array[i]) T(initializer(i));
        }
        return array;
    }

    // Only use makeBytesAlignedTo if none of the typed variants are impractical to use.
    void* makeBytesAlignedTo(size_t size, size_t align) {
        AssertRelease(SkTFitsIn<int32_t>(size));

        // Make sure alignment is a power of 2.
        SkASSERT(SkIsPow2(align));
        auto objStart = this->allocObject(ToU32(size), ToU32(align), align - 1);
        fCursor = objStart + size;
        return objStart;
    }

private:
    static void AssertRelease(bool cond) { if (!cond) { ::abort(); } }
    static uint32_t ToU32(size_t v) {
        assert(SkTFitsIn<uint32_t>(v));
        return (uint32_t)v;
    }

    using FooterAction = char* (char*);
    struct Footer {
        uint8_t unaligned_action[sizeof(FooterAction*)];
        uint8_t padding;
    };

    static char* SkipPod(char* footerEnd);
    static void RunDtorsOnBlock(char* footerEnd);
    static char* NextBlock(char* footerEnd);

    template <typename T>
    void installRaw(const T& val) {
        memcpy(fCursor, &val, sizeof(val));
        fCursor += sizeof(val);
    }
    void installFooter(FooterAction* releaser, uint32_t padding);

    void ensureSpace(uint32_t size, uint32_t alignment);

    char* allocObject(ptrdiff_t size, ptrdiff_t alignment, ptrdiff_t mask) {
        ptrdiff_t alignedOffset = -reinterpret_cast<intptr_t>(fCursor) & mask;

        if (size > fEnd - fCursor - alignedOffset) {
            // If we are out of space. Get more making sure to account for alignment.
            this->ensureSpace(size + alignment - 1, alignment);
            alignedOffset = -reinterpret_cast<intptr_t>(fCursor) & mask;
        }

        return fCursor + alignedOffset;
    }

    char* allocObjectWithFooter(ptrdiff_t sizeIncludingFooter, ptrdiff_t alignment, ptrdiff_t mask);

    template <typename T>
    T* allocUninitializedArray(size_t countZ) {
        AssertRelease(SkTFitsIn<uint32_t>(countZ));
        uint32_t count = ToU32(countZ);

        char* objStart;
        AssertRelease(count <= std::numeric_limits<uint32_t>::max() / sizeof(T));
        uint32_t arraySize = ToU32(count * sizeof(T));
        uint32_t alignment = ToU32(alignof(T));

        if (std::is_trivially_destructible<T>::value) {
            objStart = this->allocObject(arraySize, alignment, alignment - 1);
            fCursor = objStart + arraySize;
        } else {
            constexpr uint32_t overhead = sizeof(Footer) + sizeof(uint32_t);
            AssertRelease(arraySize <= std::numeric_limits<uint32_t>::max() - overhead);
            uint32_t totalSize = arraySize + overhead;
            objStart = this->allocObjectWithFooter(totalSize, alignment, alignment - 1);

            // Can never be UB because max value is alignof(T).
            uint32_t padding = ToU32(objStart - fCursor);

            // Advance to end of array to install footer.?
            fCursor = objStart + arraySize;
            this->installRaw(ToU32(count));
            this->installFooter(
                [](char* footerEnd) {
                    char* objEnd = footerEnd - (sizeof(Footer) + sizeof(uint32_t));
                    uint32_t count;
                    memmove(&count, objEnd, sizeof(uint32_t));
                    char* objStart = objEnd - count * sizeof(T);
                    T* array = (T*) objStart;
                    for (uint32_t i = 0; i < count; i++) {
                        array[i].~T();
                    }
                    return objStart;
                },
                padding);
        }

        return (T*)objStart;
    }

    char*          fDtorCursor;
    char*          fCursor;
    char*          fEnd;

    // We found allocating strictly doubling amounts of memory from the heap left too
    // much unused slop, particularly on Android.  Instead we'll follow a Fibonacci-like
    // progression that's simple to implement and grows with roughly a 1.6 exponent:
    //
    // To start,
    //    fNextHeapAlloc = fYetNextHeapAlloc = 1*fFirstHeapAllocationSize;
    //
    // And then when we do allocate, follow a Fibonacci f(n+2) = f(n+1) + f(n) rule:
    //    void* block = malloc(fNextHeapAlloc);
    //    std::swap(fNextHeapAlloc, fYetNextHeapAlloc)
    //    fYetNextHeapAlloc += fNextHeapAlloc;
    //
    // That makes the nth allocation fib(n) * fFirstHeapAllocationSize bytes.
    uint32_t fNextHeapAlloc,     // How many bytes minimum will we allocate next from the heap?
             fYetNextHeapAlloc;  // And then how many the next allocation after that?
};

class SkArenaAllocWithReset : public SkArenaAlloc {
public:
    SkArenaAllocWithReset(char* block, size_t blockSize, size_t firstHeapAllocation);

    explicit SkArenaAllocWithReset(size_t firstHeapAllocation)
            : SkArenaAllocWithReset(nullptr, 0, firstHeapAllocation) {}

    // Destroy all allocated objects, free any heap allocations.
    void reset();

private:
    char* const    fFirstBlock;
    const uint32_t fFirstSize;
    const uint32_t fFirstHeapAllocationSize;
};

// Helper for defining allocators with inline/reserved storage.
// For argument declarations, stick to the base type (SkArenaAlloc).
// Note: Inheriting from the storage first means the storage will outlive the
// SkArenaAlloc, letting ~SkArenaAlloc read it as it calls destructors.
// (This is mostly only relevant for strict tools like MSAN.)
template <size_t InlineStorageSize>
class SkSTArenaAlloc : private std::array<char, InlineStorageSize>, public SkArenaAlloc {
public:
    explicit SkSTArenaAlloc(size_t firstHeapAllocation = InlineStorageSize)
        : SkArenaAlloc{this->data(), this->size(), firstHeapAllocation} {}
};

template <size_t InlineStorageSize>
class SkSTArenaAllocWithReset
        : private std::array<char, InlineStorageSize>, public SkArenaAllocWithReset {
public:
    explicit SkSTArenaAllocWithReset(size_t firstHeapAllocation = InlineStorageSize)
            : SkArenaAllocWithReset{this->data(), this->size(), firstHeapAllocation} {}
};

#endif  // SkArenaAlloc_DEFINED
