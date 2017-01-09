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

// SkArenaAlloc allocates object and destroys the allocated object when destroyed. Its designed
// to manage the number of block allocations. Say you know the max memory usage for 999/1,000 times
// (max999) and the max memory usage for 999,999/1,000,000 times (max999999). You can use it in
// the following to do no mallocs 999/1,000 times and only one malloc call all other times except
// for the 1 time out of 1,000,000.
//
//   char block[max999];
//   SkArenaAlloc arena(block, max999, max999999 - max999);
//
// If max999 is too large for the stack, you can use the following pattern.
//
//   std::unique_ptr<char[]> block{new char[max999]};
//   SkArenaAlloc arena(block.get(), max999, max999999 - max999);
//
// If the program only sometimes allocates memory (max999 = 0), you can use this:
//
//   SkArenaAlloc arena(nullptr, 0, max999999);
//
// In addition, the system is optimized to handle POD data including arrays of PODs (where
// POD is really data with no destructors). For POD data it has zero overhead per item, and a
// typical block overhead of 8 bytes. For non-POD objects there is a per item overhead of 4 bytes.
// For arrays of non-POD objects there is a per array overhead of typically 8 bytes. There is an
// addition overhead when switching from POD data to non-POD data of typically 8 bytes.
class SkArenaAlloc {
public:
    SkArenaAlloc(char* block, size_t size, size_t extraSize = 0)
    : fDtorCursor{block}
    , fCursor{block}
    , fEnd{block + size}
    , fExtraSize{extraSize}
    {
        if (size < sizeof(Footer)) {
            fEnd = fCursor = fDtorCursor = nullptr;
        }

        if (fCursor != nullptr) {
            this->installFooter(EndChain, 0);
        }
    }

    ~SkArenaAlloc();

    template <typename T, typename... Args>
    T* make(Args&&... args) {
        char* objStart;
        if (std::is_trivially_destructible<T>::value) {
            objStart = this->allocObject(sizeof(T), alignof(T));
            fCursor = objStart + sizeof(T);
        } else {
            size_t totalSize = sizeof(T) + sizeof(Footer);
            objStart = this->allocObjectWithFooter(totalSize, alignof(T));
            size_t padding = objStart - fCursor;

            // Advance to end of object to install footer.
            fCursor = objStart + sizeof(T);
            FooterAction releaser = [](char* objEnd) {
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
        char* objStart = this->commonArrayAlloc<T>(count);

        // This must be last to make objects with nested use of this allocator work.
        // If T is primitive then no initialization takes place.
        return new(objStart) T[count];
    }

    template <typename T>
    T* makeArray(size_t count) {
        char* objStart = this->commonArrayAlloc<T>(count);

        // This must be last to make objects with nested use of this allocator work.
        // If T is primitive then the memory is initialized. So, an array of chars will be zeroed.
        return new(objStart) T[count]();
    }

    void reset();

private:
    using Footer = int32_t;
    using FooterAction = char*(*)(char*);

    void ensureSpace(size_t size, size_t alignment);

    void installFooter(FooterAction releaser, ptrdiff_t padding) {
        ptrdiff_t releaserDiff = (char *)releaser - (char *)EndChain;
        ptrdiff_t footerData = SkLeftShift((int64_t)releaserDiff, 5) | padding;
        if (padding >= 32 || !SkTFitsIn<int32_t>(footerData)) {
            // Footer data will not fit.
            SkFAIL("Constraints are busted.");
        }

        Footer footer = (Footer)(footerData);
        memmove(fCursor, &footer, sizeof(Footer));
        fCursor += sizeof(Footer);
        fDtorCursor = fCursor;
    }

    // N.B. Action is different than FooterAction. FooterAction expects the end of the Footer,
    // and returns the start of the object. An Action expects the end of the *Object* and returns
    // the start of the object.
    template<typename Action>
    void installIntFooter(ptrdiff_t size, ptrdiff_t padding) {
        if (SkTFitsIn<int32_t>(size)) {
            int32_t smallSize = static_cast<int32_t>(size);
            memmove(fCursor, &smallSize, sizeof(int32_t));
            fCursor += sizeof(int32_t);
            this->installFooter(
                [](char* footerEnd) {
                    Action action;
                    char* objEnd = footerEnd - (sizeof(Footer) + sizeof(int32_t));
                    int32_t data;
                    memmove(&data, objEnd, sizeof(int32_t));
                    return action(objEnd, data);
                },
                padding);
        } else {
            memmove(fCursor, &size, sizeof(ptrdiff_t));
            fCursor += sizeof(ptrdiff_t);
            this->installFooter(
                [](char* footerEnd) {
                    Action action;
                    char* objEnd = footerEnd - (sizeof(Footer) + sizeof(ptrdiff_t));
                    ptrdiff_t data;
                    memmove(&data, objEnd, sizeof(ptrdiff_t));
                    return action(objEnd, data);
                },
                padding);
        }
    }

    char* allocObject(size_t size, size_t alignment) {
        size_t mask = alignment - 1;
        char* objStart = (char*)((uintptr_t)(fCursor + mask) & ~mask);
        if (objStart + size > fEnd) {
            this->ensureSpace(size, alignment);
            objStart = (char*)((uintptr_t)(fCursor + mask) & ~mask);
        }
        return objStart;
    }

    // * sizeAndFooter - the memory for the footer in addition to the size for the object.
    // * alignment - alignment needed by the object.
    char* allocObjectWithFooter(size_t sizeAndFooter, size_t alignment) {
        size_t mask = alignment - 1;

    restart:
        size_t skipOverhead = 0;
        bool needsSkipFooter = fCursor != fDtorCursor;
        if (needsSkipFooter) {
            size_t skipSize = SkTFitsIn<int32_t>(fDtorCursor - fCursor)
                              ? sizeof(int32_t)
                              : sizeof(ptrdiff_t);
            skipOverhead = sizeof(Footer) + skipSize;
        }
        char* objStart = (char*)((uintptr_t)(fCursor + skipOverhead + mask) & ~mask);
        size_t totalSize = sizeAndFooter + skipOverhead;

        if (objStart + totalSize > fEnd) {
            this->ensureSpace(totalSize, alignment);
            goto restart;
        }

        SkASSERT(objStart + totalSize <= fEnd);

        if (needsSkipFooter) {
            this->installIntFooter<Skipper>(fDtorCursor - fCursor, 0);
        }

        return objStart;
    }

    template <typename T>
    char* commonArrayAlloc(size_t count) {
        char* objStart;
        size_t arraySize = count * sizeof(T);

        SkASSERT(arraySize > 0);

        if (std::is_trivially_destructible<T>::value) {
            objStart = this->allocObject(arraySize, alignof(T));
            fCursor = objStart + arraySize;
        } else {
            size_t countSize = SkTFitsIn<int32_t>(count) ? sizeof(int32_t) : sizeof(ptrdiff_t);
            size_t totalSize = arraySize + sizeof(Footer) + countSize;
            objStart = this->allocObjectWithFooter(totalSize, alignof(T));
            size_t padding = objStart - fCursor;

            // Advance to end of array to install footer.
            fCursor = objStart + arraySize;
            this->installIntFooter <ArrayReleaser<T>> (count, padding);
        }

        return objStart;
    }

    char* callFooterAction(char* end);

    static char* EndChain(char*) { return nullptr; }

    struct Skipper {
        char* operator()(char* objEnd, ptrdiff_t size) { return objEnd + size; }
    };

    struct NextBlock {
        char* operator()(char* objEnd, ptrdiff_t size) { delete [] objEnd; return objEnd + size; }
    };

    template<typename T>
    struct ArrayReleaser {
        char* operator()(char* objEnd, ptrdiff_t count) {
            char* objStart = objEnd - count * sizeof(T);
            T* array = (T*) objStart;
            for (int i = 0; i < count; i++) {
                array[i].~T();
            }
            return objStart;
        }
    };

    char*  fDtorCursor     {nullptr};
    char*  fCursor         {nullptr};
    char*  fEnd            {nullptr};
    size_t fExtraSize;
};

#endif//SkFixedAlloc_DEFINED
