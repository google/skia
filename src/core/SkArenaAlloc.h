/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFixedAlloc_DEFINED
#define SkFixedAlloc_DEFINED

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
class SkArenaAlloc {
public:
    SkArenaAlloc(char* block, size_t size, size_t extraSize = 0);

    template <size_t kSize>
    SkArenaAlloc(char (&block)[kSize], size_t extraSize = kSize)
        : SkArenaAlloc(block, kSize, extraSize)
    {}

    SkArenaAlloc(size_t extraSize)
        : SkArenaAlloc(nullptr, 0, extraSize)
    {}

    ~SkArenaAlloc();

    template <typename T, typename... Args>
    T* make(Args&&... args) {
        SkASSERT(SkTFitsIn<uint32_t>(sizeof(T)));
        char* objStart;
        if (skstd::is_trivially_destructible<T>::value) {
            objStart = this->allocObject(sizeof(T), alignof(T));
            fCursor = objStart + sizeof(T);
        } else {
            objStart = this->allocObjectWithFooter(sizeof(T) + sizeof(Footer), alignof(T));
            size_t padding = objStart - fCursor;

            // Advance to end of object to install footer.
            fCursor = objStart + sizeof(T);
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
        T* array = (T*)this->commonArrayAlloc<T>(count);

        // If T is primitive then no initialization takes place.
        for (size_t i = 0; i < count; i++) {
            new (&array[i]) T;
        }
        return array;
    }

    template <typename T>
    T* makeArray(size_t count) {
        T* array = (T*)this->commonArrayAlloc<T>(count);

        // If T is primitive then the memory is initialized. For example, an array of chars will
        // be zeroed.
        for (size_t i = 0; i < count; i++) {
            new (&array[i]) T();
        }
        return array;
    }

    // Destroy all allocated objects, free any heap allocations.
    void reset();

private:
    using Footer = int32_t;
    using FooterAction = char* (char*);

    static char* SkipPod(char* footerEnd);
    static void RunDtorsOnBlock(char* footerEnd);
    static char* NextBlock(char* footerEnd);

    void installFooter(FooterAction* releaser, ptrdiff_t padding);
    void installUint32Footer(FooterAction* action, uint32_t value, ptrdiff_t padding);
    void installPtrFooter(FooterAction* action, char* ptr, ptrdiff_t padding);

    void ensureSpace(size_t size, size_t alignment);

    char* allocObject(size_t size, size_t alignment);

    char* allocObjectWithFooter(size_t sizeIncludingFooter, size_t alignment);

    template <typename T>
    char* commonArrayAlloc(size_t count) {
        SkASSERT(SkTFitsIn<uint32_t>(count));
        char* objStart;
        size_t arraySize = count * sizeof(T);
        SkASSERT(SkTFitsIn<uint32_t>(arraySize));

        if (skstd::is_trivially_destructible<T>::value) {
            objStart = this->allocObject(arraySize, alignof(T));
            fCursor = objStart + arraySize;
        } else {
            size_t totalSize = arraySize + sizeof(Footer) + sizeof(uint32_t);
            objStart = this->allocObjectWithFooter(totalSize, alignof(T));
            size_t padding = objStart - fCursor;

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

    char*        fDtorCursor;
    char*        fCursor;
    char*        fEnd;
    char* const  fFirstBlock;
    const size_t fFirstSize;
    const size_t fExtraSize;
};

#endif//SkFixedAlloc_DEFINED
