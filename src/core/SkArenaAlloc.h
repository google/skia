/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkArenaAlloc_DEFINED
#define SkArenaAlloc_DEFINED

#include "include/private/SkTFitsIn.h"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <new>
#include <type_traits>
#include <utility>
#include <vector>

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
        : SkArenaAlloc(nullptr, 0, firstHeapAllocation)
    {}

    ~SkArenaAlloc();

    template <typename T, typename... Args>
    T* make(Args&&... args) {
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
        return new(objStart) T(std::forward<Args>(args)...);
    }

    template <typename T>
    T* makeArrayDefault(size_t count) {
        AssertRelease(SkTFitsIn<uint32_t>(count));
        uint32_t safeCount = ToU32(count);
        T* array = (T*)this->commonArrayAlloc<T>(safeCount);

        // If T is primitive then no initialization takes place.
        for (size_t i = 0; i < safeCount; i++) {
            new (&array[i]) T;
        }
        return array;
    }

    template <typename T>
    T* makeArray(size_t count) {
        AssertRelease(SkTFitsIn<uint32_t>(count));
        uint32_t safeCount = ToU32(count);
        T* array = (T*)this->commonArrayAlloc<T>(safeCount);

        // If T is primitive then the memory is initialized. For example, an array of chars will
        // be zeroed.
        for (size_t i = 0; i < safeCount; i++) {
            new (&array[i]) T();
        }
        return array;
    }

    // Only use makeBytesAlignedTo if none of the typed variants are impractical to use.
    void* makeBytesAlignedTo(size_t size, size_t align) {
        size_t allocated;
        return this->makeAtleastBytesAlignedTo(size, size, align, &allocated);
    }

    // Only use makeAtleastBytesAlignedTo when allocating a flexibly sized array, where it's
    // important to make effective use of the arena's blocks. If the current block has at least
    // 'minSize' bytes, it will allocate min(remaining, maxSize). If there is not enough room for
    // 'minSize' a new block will be made and a maxSize array will be returned. The size of the
    // returned array is reported in 'allocated'.
    void* makeAtleastBytesAlignedTo(size_t minSize, size_t maxSize, size_t align,
                                    size_t* allocated) {
        AssertRelease(SkTFitsIn<uint32_t>(minSize) && SkTFitsIn<uint32_t>(maxSize));
        // This is not quite alloc(minSize) + resize(maxSize) because if alloc(minSize) would
        // require an allocation, this makes sure to request maxSize (hypothetically, ensuring
        // minSize could still allocate a block that has enough room to resize up to max size, but
        // that is entirely dependent on the initial heap size and prior allocations.)
        auto objStart = this->allocAtleast(ToU32(minSize), ToU32(maxSize), ToU32(align), allocated);
        fCursor = objStart + *allocated;
        return objStart;
    }

    // Attempt resize the given allocation, where 'ptr' was previously returned by
    // makeBytesAlignedTo (and 'currentSize' equals the original input size), or was returned by
    // makeAtleastBytesAlignedTo (and 'currentSize' equals what had been allocated).
    //
    // When the allocation is the last in the arena, its reserved space will change from
    // 'currentSize' up to 'requestedSize' or the end of the current block. If 'requestedSize' is
    // less than 'currentSize', this essentially returns reserved space back to the arena. Thus,
    // this never triggers an allocation or moves data.
    //
    // Returns true if bytes were returned, at which point 'requestedSize' will be updated with the
    // actual new size reserved for 'ptr'. Returns false if 'ptr's reservation couldn't be changed.
    bool resize(void* ptr, size_t currentSize, size_t* requestedSize) {
        AssertRelease(SkTFitsIn<uint32_t>(currentSize) && SkTFitsIn<uint32_t>(*requestedSize));
        if (this->isLastAlloc(ptr, ToU32(currentSize))) {
            intptr_t remaining = static_cast<intptr_t>(fEnd - fCursor);
            // This can be negative when shrinking the space reservation. However, since it
            // starts from 'requestedSize', which is a uint, 'growth' will never cause fCursor to
            // move earlier than 'ptr'.
            intptr_t growth = static_cast<intptr_t>(*requestedSize) -
                              static_cast<intptr_t>(currentSize);
            if (growth > remaining) {
                growth = remaining;
            }
            AssertRelease(fCursor + growth <= fEnd && fCursor + growth >= ptr);
            fCursor += growth;
            *requestedSize = currentSize + growth;
            return true;
        } else {
            // Not the last, so no room to grow
            return false;
        }
    }

    // Return the POD to the arena so that it might be able to be re-purposed by subsequent
    // allocations. Returns true if recovered, false if the arena has already allocated beyond
    // 'ptr'. This is not inherently a bug, but can be used to identify inefficient use patterns.
    bool release(void* ptr, size_t size) {
        // Releasing the POD is equivalent to resizing it to 0 bytes
        size_t requested = 0;
        return this->resize(ptr, size, &requested);
    }

    // Destroy all allocated objects, free any heap allocations.
    void reset();

private:
    static void AssertRelease(bool cond) { if (!cond) { ::abort(); } }
    static uint32_t ToU32(size_t v) {
        assert(SkTFitsIn<uint32_t>(v));
        return (uint32_t)v;
    }

    using Footer = int64_t;
    using FooterAction = char* (char*);

    static char* SkipPod(char* footerEnd);
    static void RunDtorsOnBlock(char* footerEnd);
    static char* NextBlock(char* footerEnd);

    void installFooter(FooterAction* releaser, uint32_t padding);
    void installUint32Footer(FooterAction* action, uint32_t value, uint32_t padding);
    void installPtrFooter(FooterAction* action, char* ptr, uint32_t padding);

    bool isLastAlloc(void* start, uint32_t size) const {
        // For trivially destructible types, or raw allocations, fCursor was set to
        // objStart + size, so if start + size still equals the cursor, then nothing else has
        // been allocated since.
        const char* expectedCursor = static_cast<const char*>(start) + size;
        return expectedCursor == fCursor;
    }

    void ensureSpace(uint32_t size, uint32_t alignment);

    char* allocObject(uint32_t size, uint32_t alignment) {
        size_t allocated;
        return this->allocAtleast(size, size, alignment, &allocated);
    }

    char* allocAtleast(uint32_t minSize, uint32_t maxSize, uint32_t alignment, size_t* allocated) {
        uintptr_t mask = alignment - 1;
        uintptr_t alignedOffset = (~reinterpret_cast<uintptr_t>(fCursor) + 1) & mask;
        // Start by trying to fit the max size in the current block
        uintptr_t totalMaxSize = maxSize + alignedOffset;
        AssertRelease(totalMaxSize >= maxSize);
        uintptr_t remaining = static_cast<uintptr_t>(fEnd - fCursor);
        if (totalMaxSize > remaining) {
            // Max allocation won't fit, try to use remaining in the cursor if >= minSize
            uintptr_t totalMinSize = minSize + alignedOffset;
            if (totalMinSize > remaining) {
                // Need a new block, request the max size
                this->ensureSpace(maxSize, alignment);
                alignedOffset = (~reinterpret_cast<uintptr_t>(fCursor) + 1) & mask;
                *allocated = maxSize;
            } else {
                // Fit into the rest of the block
                *allocated = static_cast<size_t>(remaining - alignedOffset);
            }
        } else {
            *allocated = maxSize;
        }

        AssertRelease(*allocated >= minSize && *allocated <= maxSize);
        AssertRelease(fCursor + alignedOffset + *allocated <= fEnd);
        return fCursor + alignedOffset;
    }

    char* allocObjectWithFooter(uint32_t sizeIncludingFooter, uint32_t alignment);

    template <typename T>
    char* commonArrayAlloc(uint32_t count) {
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
                ToU32(count),
                padding);
        }

        return objStart;
    }

    char*          fDtorCursor;
    char*          fCursor;
    char*          fEnd;
    char* const    fFirstBlock;
    const uint32_t fFirstSize;
    const uint32_t fFirstHeapAllocationSize;

    // Use the Fibonacci sequence as the growth factor for block size. The size of the block
    // allocated is fFib0 * fFirstHeapAllocationSize. Using 2 ^ n * fFirstHeapAllocationSize
    // had too much slop for Android.
    uint32_t       fFib0 {1}, fFib1 {1};
};

// Helper for defining allocators with inline/reserved storage.
// For argument declarations, stick to the base type (SkArenaAlloc).
template <size_t InlineStorageSize>
class SkSTArenaAlloc : public SkArenaAlloc {
public:
    explicit SkSTArenaAlloc(size_t firstHeapAllocation = InlineStorageSize)
        : INHERITED(fInlineStorage, InlineStorageSize, firstHeapAllocation) {}

private:
    char fInlineStorage[InlineStorageSize];

    using INHERITED = SkArenaAlloc;
};

#endif  // SkArenaAlloc_DEFINED
