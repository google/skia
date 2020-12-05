/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkArenaAlloc_DEFINED
#define SkArenaAlloc_DEFINED

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

//#define UseOldSkArenaAlloc

// SkArena provides fast allocation where the user takes care of calling the destructors of the
// returned pointers, and SkArena takes care of deleting the storage. The unique_ptrs returned,
// are to assist in assuring the object's destructor is called.
// A note on zero length arrays: according to the standard a pointer must be returned, and it
// can't be a nullptr. SkArena allocates one item, but does not initialize it.
class SkArena {
public:
    SkArena(char* block, int blockSize, int firstHeapAllocation);
    explicit SkArena(int firstHeapAllocation = 0);
    ~SkArena();

    template <typename Ctor> auto makePOD(Ctor&& ctor) -> decltype(ctor(nullptr)) {
        using T = std::remove_pointer_t<decltype(ctor(nullptr))>;
        return this->innerMake<T, Ctor>(std::forward<Ctor>(ctor));
    }

    template <typename T, typename... Args> T* makePOD(Args&&... args) {
        static_assert(std::is_trivially_destructible<T>::value, "This is not POD. Use make.");
        return this->innerMake<T>([&](void* objStart) {
            return new(objStart) T(std::forward<Args>(args)...);
        });
    }

    struct Destroyer {
        template <typename T>
        void operator()(T* ptr) { ptr->~T(); }
    };

    template<typename Ctor>
    auto makeUnique(Ctor&& ctor)
            -> std::unique_ptr<std::remove_pointer_t<decltype(ctor(nullptr))>, Destroyer> {
        using T = std::remove_pointer_t<decltype(ctor(nullptr))>;
        static_assert(!std::is_trivially_destructible<T>::value, "This is POD. Use makePOD.");
        return std::unique_ptr<T, Destroyer>{this->innerMake<T>(std::forward<Ctor>(ctor))};
    }

    template <typename T, typename... Args>
    std::unique_ptr<T, Destroyer> makeUnique(Args&&... args) {
        return this->makeUnique([&](void* objStart) {
            return new(objStart) T(std::forward<Args>(args)...);
        });
    }

    template<typename T> T* makePODArray(int n) {
        static_assert(std::is_trivially_destructible<T>::value,
                "This is not POD. Use makeUniqueArray.");
        return this->template innerMakeArray<T>(n);
    }

    struct ArrayDestroyer {
        int n;
        template <typename T>
        void operator()(T* ptr) {
            for (int i = 0; i < n; i++) { ptr[i].~T(); }
        }
    };

    template<typename T>
    std::unique_ptr<T[], ArrayDestroyer> makeUniqueArray(int n) {
        static_assert(!std::is_trivially_destructible<T>::value, "This is POD. Use makePODArray.");
        T* array = this->template innerMakeArray<T>(n);
        for (int i = 0; i < n; i++) {
            new (&array[i]) T{};
        }
        return std::unique_ptr<T[], ArrayDestroyer>{array, ArrayDestroyer{n}};
    }

    template<typename T, typename I>
    std::unique_ptr<T[], ArrayDestroyer> makeUniqueArray(int n, I initializer) {
        static_assert(!std::is_trivially_destructible<T>::value, "This is POD. Use makePODArray.");
        T* array = this->template innerMakeArray<T>(n);
        for (int i = 0; i < n; i++) {
            new (&array[i]) T(initializer(i));
        }
        return std::unique_ptr<T[], ArrayDestroyer>{array, ArrayDestroyer{n}};
    }

    static constexpr int MinimumSizeWithOverhead(int requestedSize) {
        SkASSERT_RELEASE(requestedSize < kMaxByteSize);
        constexpr int kMallocRounding = kMaxAlignment - alignof(max_align_t);
        constexpr int k4K  = (1 << 12);
        constexpr int k32K = (1 << 15);

        auto alignUp = [](int size, int alignment) {return (size + (alignment - 1)) & -alignment;};

        // The minimumSize is the amount to allocate to assure a pointer with kMaxAlignment
        // alignment and at least size requiredSize + sizeof(Block);
        int minimumSize =
                alignUp(requestedSize + kMallocRounding + sizeof(Block), alignof(max_align_t));

        // If minimumSize is > 32k then round to a 4K boundary. The > 32K heuristic is from the
        // JEMalloc behavior.
        if (minimumSize >= k32K) {
            minimumSize = alignUp(minimumSize, k4K);
        }

        return minimumSize;
    }

    char* alignedBytes(int size, int alignment);

private:
    // 16 seems to be a good number for alignment. If a use case for larger alignments is found,
    // we can turn this into a template parameter.
    static constexpr int kMaxAlignment = 16;
    // The largest size that can be allocated. Includes maximum padding and fudge for the Block.
    // This should never overflow with the calculations done on the code.
    static constexpr int kMaxByteSize = std::numeric_limits<int>::max() - 2*kMaxAlignment - 32;

    // The Block starts at the location pointed to by fEndByte.
    // Beware. Order is important here. The destructor for fPrevious must be called first because
    // the Block is embedded in fBlockStart. Destructors are run in reverse order.
    struct Block {
        Block(char* previous, char* startOfBlock);
        char* const fBlockStart;
        Block* const fPrevious;
    };

    // Note: fCapacity is the number of bytes remaining, but the are subtracted from fEndByte to
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

    template <typename T, typename Ctor> T* innerMake(Ctor&& ctor) {
        static_assert(alignof(T) <= kMaxAlignment, "Alignment is too big for arena");
        static_assert(sizeof(T) < kMaxByteSize, "Size is too big for arena");
        constexpr int size = SkTo<int>(sizeof(T));
        constexpr int alignment = SkTo<int>(alignof(T));
        return ctor(this->allocateBytes(size, alignment));
    }

    template <typename T> T* innerMakeArray(int n) {
        static_assert(alignof(T) <= kMaxAlignment, "Alignment is too big for arena");
        constexpr int kMaxN = kMaxByteSize / sizeof(T);
        SkASSERT_RELEASE(0 <= n && n < kMaxN);
        // Allocate at least one item.
        return (T*)this->alignedBytes(sizeof(T) * (n ? n : 1), alignof(T));
    }

    void setupBytesAndCapacity(char* bytes, int size);

    // Adjust fEndByte and fCapacity to satisfy the size and alignment request.
    void needMoreBytes(int size, int alignment);

    // This points to the highest kMaxAlignment address in the allocated block. The address of
    // the current end of allocated data is given by fEndByte - fCapacity. While the negative side
    // of this pointer are the bytes to be allocated. The positive side points to the Block for
    // this memory. So, it virtually has type std::unique_ptr<Block, Destroyer>.
    char* fEndByte{nullptr};

    // The number of bytes remaining in this block.
    int fCapacity{0};

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
    int fNextHeapAlloc{1024},  // How many bytes minimum will we allocate next from the heap?
        fYetNextHeapAlloc{1024};  // And then how many the next allocation after that?
};

// Helper for defining allocators with inline/reserved storage.
// For argument declarations, stick to the base type (SkArena).
// Note: Inheriting from the storage first means the storage will outlive the
// SkArenaAlloc, letting ~SkArenaAlloc read it as it calls destructors.
// (This is mostly only relevant for strict tools like MSAN.)
template <size_t InlineStorageSize>
class SkSTArena : private std::array<char, SkArena::MinimumSizeWithOverhead(InlineStorageSize)>
                , public SkArena {
public:
    explicit SkSTArena(int firstHeapAllocation = MinimumSizeWithOverhead(InlineStorageSize))
        : SkArena{this->data(), SkTo<int>(this->size()), firstHeapAllocation} {}
};

#if !defined(UseOldSkArenaAlloc)

class SkArenaAlloc : public SkArena {
public:
    SkArenaAlloc(char* block, size_t blockSize, size_t firstHeapAllocation)
        : SkArena(block, blockSize, firstHeapAllocation) {}

    explicit SkArenaAlloc(size_t firstHeapAllocation = 0)
        : SkArenaAlloc(nullptr, 0, firstHeapAllocation) {}

    template <typename Ctor>
    auto make(Ctor&& ctor) -> decltype(ctor(nullptr)) {
        using T = std::remove_pointer_t<decltype(ctor(nullptr))>;

        if constexpr (std::is_trivially_destructible<T>::value) {
            return this->makePOD(std::forward<Ctor>(ctor));
        } else {
            return this->adopt<T>(std::forward<Ctor>(ctor));
        }
    }

    template <typename T, typename... Args>
    T* make(Args&&... args) {
        return this->make([&](void* objStart) {
            return new(objStart) T(std::forward<Args>(args)...);
        });
    }

    template <typename T>
    T* makeArrayDefault(size_t count) {
        if constexpr (std::is_trivially_destructible<T>::value) {
            return this->makePODArray<T>(count);
        } else {
            return this->adoptArray<T>(count);
        }
    }

    template <typename T>
    T* makeArray(size_t count) {
        if constexpr (std::is_trivially_destructible<T>::value) {
            T* array = this->makePODArray<T>(count);
            for (size_t i = 0; i < count; i++) {
                new (&array[i]) T{};
            }
            return array;
        } else {
            return this->adoptArray<T>(count);
        }
    }

    template <typename T, typename Initializer>
    T* makeInitializedArray(size_t count, Initializer initializer) {
        if constexpr (std::is_trivially_destructible<T>::value) {
            T* array = this->makePODArray<T>(count);
            for (size_t i = 0; i < count; i++) {
                new (&array[i]) T(initializer(i));
            }
            return array;
        } else {
            return this->adoptArray<T>(count, initializer);
        }
    }

    void* makeBytesAlignedTo(size_t size, size_t align) {
        return this->alignedBytes(size, align);
    }

private:
    struct Owner;
    using NextHeader = std::unique_ptr<Owner, SkArena::Destroyer>;
    template<typename Specific>
    static void UseOwnerAction(Owner* header) { static_cast<Specific*>(header)->act(); }
    struct Owner {
        // Use a pointer from the subclass for resolving specific.
        template<typename Specific>
        Owner(NextHeader next, Specific*)
                : fAction{UseOwnerAction<Specific>}
                , fNext{std::move(next)} {}
        ~Owner() {
            fAction(this);
        }
        void(*fAction)(Owner*);
        NextHeader fNext;
    };

    template<typename T> struct ObjectOwner final : public Owner {
        template<typename Ctor> ObjectOwner(NextHeader next, Ctor&& ctor)
                : Owner{std::move(next), this} {
                    ctor(&storage);
                }
        void act() { this->get()->~T(); }
        T* get() {return (T*)&storage; }
        alignas(alignof(T)) char storage[sizeof(T)];
    };

    template<typename T, typename Ctor>
    T* adopt(Ctor&& ctor) {
        auto overCtor = [&] (void *ptr) {
            return new (ptr) ObjectOwner<T>(std::move(fOwnerList), std::forward<Ctor>(ctor));
        };
        auto owner = this->makeUnique(overCtor);
        T* result = owner->get();
        fOwnerList = std::move(owner);
        return result;
    }

    template<typename T> struct ArrayOwner final : public Owner {
        using Array = std::unique_ptr<T[], SkArena::ArrayDestroyer>;
        ArrayOwner(NextHeader next, Array array)
            // Pass this to deduce the subclass for Owner.
            : Owner{std::move(next), this}
            , fArray{std::move(array)} {}
        void act() { fArray.~unique_ptr(); }
        Array fArray;
    };

    template<typename T, typename... Args>
    T* adoptArray(int count, Args&&... args) {
        auto array = this->SkArena::makeUniqueArray<T>(count, std::forward<Args>(args)...);
        T* result = array.get();
        auto owner = this->makeUnique<ArrayOwner<T>>(std::move(fOwnerList), std::move(array));
        fOwnerList = std::move(owner);
        return result;
    }

    NextHeader fOwnerList{nullptr};
};

#else

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
public:
    SkArenaAlloc(char* block, size_t blockSize, size_t firstHeapAllocation);

    explicit SkArenaAlloc(size_t firstHeapAllocation)
        : SkArenaAlloc(nullptr, 0, firstHeapAllocation) {}

    ~SkArenaAlloc();

    template <typename Ctor>
    auto make(Ctor&& ctor) -> decltype(ctor(nullptr)) {
        using T = std::remove_pointer_t<decltype(ctor(nullptr))>;

        uint32_t size      = ToU32(sizeof(T));
        uint32_t alignment = ToU32(alignof(T));
        char* objStart;
        if (std::is_trivially_destructible<T>::value) {
            objStart = this->allocObject(size, alignment);
            fCursor = objStart + size;
        } else {
            objStart = this->allocObjectWithFooter(size + sizeof(Footer), alignment);
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

        // This must be last to make objects with nested use of this allocator work.
        return ctor(objStart);
    }

    template <typename T, typename... Args>
    T* make(Args&&... args) {
        return this->make([&](void* objStart) {
            return new(objStart) T(std::forward<Args>(args)...);
        });
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
        AssertRelease(SkTFitsIn<uint32_t>(size));
        auto objStart = this->allocObject(ToU32(size), ToU32(align));
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

    char* allocObject(uint32_t size, uint32_t alignment) {
        uintptr_t mask = alignment - 1;
        uintptr_t alignedOffset = (~reinterpret_cast<uintptr_t>(fCursor) + 1) & mask;
        uintptr_t totalSize = size + alignedOffset;
        AssertRelease(totalSize >= size);
        if (totalSize > static_cast<uintptr_t>(fEnd - fCursor)) {
            this->ensureSpace(size, alignment);
            alignedOffset = (~reinterpret_cast<uintptr_t>(fCursor) + 1) & mask;
        }

        char* object = fCursor + alignedOffset;

        SkASSERT((reinterpret_cast<uintptr_t>(object) & (alignment - 1)) == 0);
        SkASSERT(object + size <= fEnd);

        return object;
    }

    char* allocObjectWithFooter(uint32_t sizeIncludingFooter, uint32_t alignment);

    template <typename T>
    T* allocUninitializedArray(size_t countZ) {
        AssertRelease(SkTFitsIn<uint32_t>(countZ));
        uint32_t count = ToU32(countZ);

        char* objStart;
        AssertRelease(count <= std::numeric_limits<uint32_t>::max() / sizeof(T));
        uint32_t arraySize = ToU32(count * sizeof(T));
        uint32_t alignment = ToU32(alignof(T));

        if (std::is_trivially_destructible<T>::value) {
            objStart = this->allocObject(arraySize, alignment);
            fCursor = objStart + arraySize;
        } else {
            constexpr uint32_t overhead = sizeof(Footer) + sizeof(uint32_t);
            AssertRelease(arraySize <= std::numeric_limits<uint32_t>::max() - overhead);
            uint32_t totalSize = arraySize + overhead;
            objStart = this->allocObjectWithFooter(totalSize, alignment);

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
#endif  // Choose SkArenaAlloc implementation

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
