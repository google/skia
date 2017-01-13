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

// SkFixedAlloc allocates objects out of a fixed-size buffer and destroys them when destroyed.
class SkFixedAlloc {
public:
    SkFixedAlloc(void* ptr, size_t len);
    ~SkFixedAlloc() { this->reset(); }

    // Allocates a new T in the buffer if possible.  If not, returns nullptr.
    // Assumptions:
    // * max alignment value is 32 - if alignment is greater than 32, the allocation is best effort.
    // * footer is 32 bits - 5 bits of alignment and 27 bits of deleter difference from Base.
    // * deleter difference - the difference D is -2^26 <= D < 2^26.
    template <typename T, typename... Args>
    T* make(Args&&... args) {
        auto mask = alignof(T) - 1;

        // Align fCursor for this allocation.
        char* objStart = (char*)((uintptr_t)(fCursor + mask) & ~mask);
        ptrdiff_t padding = objStart - fCursor;
        Releaser releaser = [](char* objEnd) {
            char* objStart = objEnd - (sizeof(T) + sizeof(Footer));
            ((T*)objStart)->~T();
            return objStart;
        };

        ptrdiff_t deleterDiff = (char*)releaser - (char*)Base;

        if (objStart + sizeof(T) + sizeof(Footer) > fEnd
            || padding >= 32
            || deleterDiff >= (1 << 26)
            || deleterDiff < -(1 << 26)) {
            // Ran out of space, or code not store info in the Footer.
            return nullptr;
        }

        // Advance cursor to end of the object.
        fCursor = objStart + sizeof(T);

        Footer footer = (Footer)(SkLeftShift((int64_t)deleterDiff, 5) | padding);
        memcpy(fCursor, &footer, sizeof(Footer));
        fCursor += sizeof(Footer);

        return new (objStart) T(std::forward<Args>(args)...);
    }

    // Destroys the last object allocated and frees its space in the buffer.
    void undo();

    // Destroys all objects and frees all space in the buffer.
    void reset();

private:
    using Footer  = int32_t;
    using Releaser = char*(*)(char*);

    // A function pointer to use for offsets of releasers.
    static void Base();

    char* const fStorage;
    char*       fCursor;
    char* const fEnd;
};

class SkFallbackAlloc {
public:
    explicit SkFallbackAlloc(SkFixedAlloc*);
    ~SkFallbackAlloc() { this->reset(); }

    // Allocates a new T with the SkFixedAlloc if possible.  If not, uses the heap.
    template <typename T, typename... Args>
    T* make(Args&&... args) {
        // Once we go heap we never go back to fixed.  This keeps destructor ordering sane.
        if (fHeapAllocs.empty()) {
            if (T* ptr = fFixedAlloc->make<T>(std::forward<Args>(args)...)) {
                return ptr;
            }
        }

        char* ptr = new char[sizeof(T)];
        fHeapAllocs.push_back({[](char* ptr) { ((T*)ptr)->~T(); delete [] ptr; }, ptr});
        return new (ptr) T(std::forward<Args>(args)...);
    }


    // Destroys all objects and frees all space in the SkFixedAlloc.
    void reset();

private:
    // Destroys the last object allocated and frees any space it used in the SkFixedAlloc.
    void undo();

    struct HeapAlloc {
        void (*deleter)(char*);
        char* ptr;
    };

    SkFixedAlloc*          fFixedAlloc;
    std::vector<HeapAlloc> fHeapAllocs;
};

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
    SkArenaAlloc(char (&block)[kSize], size_t extraSize = 0)
        : SkArenaAlloc(block, kSize, extraSize)
    {}

    ~SkArenaAlloc();

    template <typename T, typename... Args>
    T* make(Args&&... args) {
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

    void installFooter(FooterAction* releaser, ptrdiff_t padding);

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
                    char* objEnd = footerEnd - (sizeof(Footer) + sizeof(int32_t));
                    int32_t data;
                    memmove(&data, objEnd, sizeof(int32_t));
                    return Action()(objEnd, data);
                },
                padding);
        } else {
            memmove(fCursor, &size, sizeof(ptrdiff_t));
            fCursor += sizeof(ptrdiff_t);
            this->installFooter(
                [](char* footerEnd) {
                    char* objEnd = footerEnd - (sizeof(Footer) + sizeof(ptrdiff_t));
                    ptrdiff_t data;
                    memmove(&data, objEnd, sizeof(ptrdiff_t));
                    return Action()(objEnd, data);
                },
                padding);
        }
    }

    void ensureSpace(size_t size, size_t alignment);

    char* allocObject(size_t size, size_t alignment);

    char* allocObjectWithFooter(size_t sizeIncludingFooter, size_t alignment);

    template <typename T>
    char* commonArrayAlloc(size_t count) {
        char* objStart;
        size_t arraySize = count * sizeof(T);

        SkASSERT(arraySize > 0);

        if (skstd::is_trivially_destructible<T>::value) {
            objStart = this->allocObject(arraySize, alignof(T));
            fCursor = objStart + arraySize;
        } else {
            size_t countSize = SkTFitsIn<int32_t>(count) ? sizeof(int32_t) : sizeof(ptrdiff_t);
            size_t totalSize = arraySize + sizeof(Footer) + countSize;
            objStart = this->allocObjectWithFooter(totalSize, alignof(T));
            size_t padding = objStart - fCursor;

            // Advance to end of array to install footer.?
            fCursor = objStart + arraySize;
            this->installIntFooter<ArrayDestructor<T>> (count, padding);
        }

        return objStart;
    }

    char* callFooterAction(char* end);

    static char* EndChain(char*);

    template<typename T>
    struct ArrayDestructor {
        char* operator()(char* objEnd, ptrdiff_t count) {
            char* objStart = objEnd - count * sizeof(T);
            T* array = (T*) objStart;
            for (int i = 0; i < count; i++) {
                array[i].~T();
            }
            return objStart;
        }
    };

    char*  fDtorCursor;
    char*  fCursor;
    char*  fEnd;
    size_t fExtraSize;
};

#endif//SkFixedAlloc_DEFINED
