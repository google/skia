/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFixedAlloc_DEFINED
#define SkFixedAlloc_DEFINED

#include "SkRefCnt.h"
#include "SkTFitsIn.h"
#include "SkTypes.h"
#include <cstddef>
#include <new>
#include <type_traits>
#include <utility>
#include <vector>

// SkArenaAlloc allocates object and destroys the allocated objects when destroyed. It's designed
// to minimize the number of underlying block allocations. SkArenaAlloc allocates first out of an
// (optional) user-provided block of memory, and when that's exhausted it allocates on the heap,
// starting with an allocation of extraSize bytes.  If your data (plus a small overhead) fits in
// the user-provided block, SkArenaAlloc never uses the heap, and if it fits in extraSize bytes,
// it'll use the heap only once.  If you pass extraSize = 0, it allocates blocks for each call to
// make<T>.
//
// Examples:
//
//   char block[mostCasesSize];
//   SkArenaAlloc arena(block, almostAllCasesSize);
//
// If mostCasesSize is too large for the stack, you can use the following pattern.
//
//   std::unique_ptr<char[]> block{new char[mostCasesSize]};
//   SkArenaAlloc arena(block.get(), mostCasesSize, almostAllCasesSize);
//
// If the program only sometimes allocates memory, use the following.
//
//   SkArenaAlloc arena(nullptr, 0, almostAllCasesSize);
//
// The storage does not necessarily need to be on the stack. Embedding the storage in a class also
// works.
//
//   class Foo {
//       char storage[mostCasesSize];
//       SkArenaAlloc arena (storage, almostAllCasesSize);
//   };
//
// In addition, the system is optimized to handle POD data including arrays of PODs (where
// POD is really data with no destructors). For POD data it has zero overhead per item, and a
// typical block overhead of 8 bytes. For non-POD objects there is a per item overhead of 4 bytes.
// For arrays of non-POD objects there is a per array overhead of typically 8 bytes. There is an
// addition overhead when switching from POD data to non-POD data of typically 8 bytes.
//
// You can track memory use by adding SkArenaAlloc::kTrack as the last parameter to any constructor.
//
//   char storage[someNumber];
//   SkArenaAlloc alloc{storage, SkArenaAlloc::kTrack};
//
// This will print out a line for every destructor or reset call that has the total memory
// allocated, the total slop (the unused portion of a block), and the slop of the last block.
//
// If additional blocks are needed they are increased exponentially. This strategy bounds the
// recursion of the RunDtorsOnBlock to be limited to O(log size-of-memory). Block size grow using
// the Fibonacci sequence which means that for 2^32 memory there are 48 allocations, and for 2^48
// there are 71 allocations.
class SkArenaAlloc {
public:
    enum Tracking {kDontTrack, kTrack};
    SkArenaAlloc(char* block, size_t size, size_t, Tracking tracking = kDontTrack);

    SkArenaAlloc(size_t extraSize, Tracking tracking = kDontTrack)
        : SkArenaAlloc(nullptr, 0, extraSize, tracking)
    {}

    ~SkArenaAlloc();

    template <typename T, typename... Args>
    T* make(Args&&... args) {
        uint32_t size      = SkTo<uint32_t>(sizeof(T));
        uint32_t alignment = SkTo<uint32_t>(alignof(T));
        char* objStart;
        if (skstd::is_trivially_destructible<T>::value) {
            objStart = this->allocObject(size, alignment);
            fCursor = objStart + size;
        } else {
            objStart = this->allocObjectWithFooter(size + sizeof(Footer), alignment);
            // Can never be UB because max value is alignof(T).
            uint32_t padding = SkTo<uint32_t>(objStart - fCursor);

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
        return new(objStart) T(std::forward<Args>(args)...);
    }

    template <typename T, typename... Args>
    sk_sp<T> makeSkSp(Args&&... args) {
        SkASSERT(SkTFitsIn<uint32_t>(sizeof(T)));

        // The arena takes a ref for itself to account for the destructor. The sk_sp count can't
        // become zero or the sk_sp will try to call free on the pointer.
        return sk_sp<T>(SkRef(this->make<T>(std::forward<Args>(args)...)));
    }

    template <typename T>
    T* makeArrayDefault(size_t count) {
        uint32_t safeCount = SkTo<uint32_t>(count);
        T* array = (T*)this->commonArrayAlloc<T>(safeCount);

        // If T is primitive then no initialization takes place.
        for (size_t i = 0; i < safeCount; i++) {
            new (&array[i]) T;
        }
        return array;
    }

    template <typename T>
    T* makeArray(size_t count) {
        uint32_t safeCount = SkTo<uint32_t>(count);
        T* array = (T*)this->commonArrayAlloc<T>(safeCount);

        // If T is primitive then the memory is initialized. For example, an array of chars will
        // be zeroed.
        for (size_t i = 0; i < safeCount; i++) {
            new (&array[i]) T();
        }
        return array;
    }

    // Destroy all allocated objects, free any heap allocations.
    void reset();

private:
    using Footer = int64_t;
    using FooterAction = char* (char*);

    static char* SkipPod(char* footerEnd);
    static void RunDtorsOnBlock(char* footerEnd);
    static char* NextBlock(char* footerEnd);

    void installFooter(FooterAction* releaser, uint32_t padding);
    void installUint32Footer(FooterAction* action, uint32_t value, uint32_t padding);
    void installPtrFooter(FooterAction* action, char* ptr, uint32_t padding);

    void ensureSpace(uint32_t size, uint32_t alignment);

    char* allocObject(uint32_t size, uint32_t alignment) {
        uintptr_t mask = alignment - 1;
        char* objStart = (char*)((uintptr_t)(fCursor + mask) & ~mask);
        if ((ptrdiff_t)size > fEnd - objStart) {
            this->ensureSpace(size, alignment);
            objStart = (char*)((uintptr_t)(fCursor + mask) & ~mask);
        }
        return objStart;
    }

    char* allocObjectWithFooter(uint32_t sizeIncludingFooter, uint32_t alignment);

    template <typename T>
    char* commonArrayAlloc(uint32_t count) {
        char* objStart;
        SkASSERT_RELEASE(count <= std::numeric_limits<uint32_t>::max() / sizeof(T));
        uint32_t arraySize = SkTo<uint32_t>(count * sizeof(T));
        uint32_t alignment = SkTo<uint32_t>(alignof(T));

        if (skstd::is_trivially_destructible<T>::value) {
            objStart = this->allocObject(arraySize, alignment);
            fCursor = objStart + arraySize;
        } else {
            constexpr uint32_t overhead = sizeof(Footer) + sizeof(uint32_t);
            SkASSERT_RELEASE(arraySize <= std::numeric_limits<uint32_t>::max() - overhead);
            uint32_t totalSize = arraySize + overhead;
            objStart = this->allocObjectWithFooter(totalSize, alignment);

            // Can never be UB because max value is alignof(T).
            uint32_t padding = SkTo<uint32_t>(objStart - fCursor);

            // Advance to end of array to install footer.?
            fCursor = objStart + arraySize;
            this->installUint32Footer(
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
                SkTo<uint32_t>(count),
                padding);
        }

        return objStart;
    }

    char*          fDtorCursor;
    char*          fCursor;
    char*          fEnd;
    char* const    fFirstBlock;
    const uint32_t fFirstSize;
    const uint32_t fExtraSize;

    // Track some useful stats. Track stats if fTotalSlop is >= 0;
    uint32_t       fTotalAlloc { 0};
    int32_t        fTotalSlop  {-1};

    // Use the Fibonacci sequence as the growth factor for block size. The size of the block
    // allocated is fFib0 * fExtraSize. Using 2 ^ n * fExtraSize had too much slop for Android.
    uint32_t       fFib0 {1}, fFib1 {1};
};

// Helper for defining allocators with inline/reserved storage.
// For argument declarations, stick to the base type (SkArenaAlloc).
template <size_t InlineStorageSize>
class SkSTArenaAlloc : public SkArenaAlloc {
public:
    explicit SkSTArenaAlloc(size_t extraSize = InlineStorageSize, Tracking tracking = kDontTrack)
        : INHERITED(fInlineStorage, InlineStorageSize, extraSize, tracking) {}

private:
    char fInlineStorage[InlineStorageSize];

    using INHERITED = SkArenaAlloc;
};

#endif//SkFixedAlloc_DEFINED
