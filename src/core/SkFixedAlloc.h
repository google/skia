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

    // Destroys the last object allocated and frees any space it used in the SkFixedAlloc.
    void undo();

    // Destroys all objects and frees all space in the SkFixedAlloc.
    void reset();

private:
    struct HeapAlloc {
        void (*deleter)(char*);
        char* ptr;
    };

    SkFixedAlloc*          fFixedAlloc;
    std::vector<HeapAlloc> fHeapAllocs;
};

class SkArenaAlloc {
public:
    SkArenaAlloc() { }
    SkArenaAlloc(char* b, size_t s)
    : fBase{b}
    , fDtorCursor{b}
    , fCursor{b}
    , fEnd{b + s}
    {
        this->installFooter(EndChain, 0);
    }

    template <typename T, typename... Args>
    T* make(Args&&... args) {
        auto mask = alignof(T) - 1;

        // Align fCursor for this allocation.
        char* objStart = (char*)((uintptr_t)(fCursor + mask) & ~mask);
        size_t overhead = 0;
        if (!std::is_trivially_destructible<T>::value) {
            overhead = fCursor == fDtorCursor ? sizeof(Footer) : sizeof(Footer) + sizeof(int32_t);
        }

        if (objStart + sizeof(T) + overhead > fEnd) {
            this->ensureSpace(sizeof(T));
            objStart = (char*)((uintptr_t)(fCursor + mask) & ~mask);
        }

        SkASSERT(objStart + sizeof(T) + overhead < fEnd);

        ptrdiff_t padding = objStart - fCursor;
        // Advance cursor to end of the object.
        fCursor = objStart + sizeof(T);

        if (!std::is_trivially_destructible<T>::value) {
            if (fCursor != fDtorCursor) {
                this->installSkipFooter(fCursor - fDtorCursor);
            }
            Releaser releaser = [](char* objEnd) {
                char* objStart = objEnd - (sizeof(T) + sizeof(Footer));
                ((T*)objStart)->~T();
                return objStart;
            };

            this->installFooter(releaser, padding);
        }

        return new(objStart) T(std::forward<Args>(args)...);
    };

private:
    using Footer = int32_t;
    using Releaser = char*(*)(char*);
    void ensureSpace(size_t size) {}

    void installFooter(Releaser releaser, ptrdiff_t padding) {
        ptrdiff_t releaserDiff = (char *)releaser - (char *)EndChain;

        if (padding >= 32
            || releaserDiff >= (1 << 26)
            || releaserDiff < -(1 << 26)) {
            // Ran out of space, or code not store info in the Footer.
            SkFAIL("Constraints are busted.");
        }

        Footer footer = (Footer)(SkLeftShift((int64_t)releaserDiff, 5) | padding);
        memcpy(fCursor, &footer, sizeof(Footer));
        fCursor += sizeof(Footer);
        fDtorCursor = fCursor;
    }

    void installSkipFooter(size_t size) {
        if ((int32_t)size == size) {
            int32_t smallSize = size;
            memmove(fCursor, &smallSize, sizeof(int32_t));
            fCursor += sizeof(int32_t);
            this->installFooter(NextNear, 0);
        } else {
            memmove(fCursor, &size, sizeof(size_t));
            fCursor += sizeof(size_t);
            this->installFooter(NextFar, 0);
        }
    }

    static char* EndChain(char*) { return nullptr; }

    static char* NextNear(char* end) {
        char* start = end - (sizeof(Footer) + sizeof(int32_t));
        int32_t distanceNext;
        memmove(&distanceNext, start, sizeof(int32_t));
        free(start);
        return start + distanceNext;
    }

    static char* NextFar(char* end) {
        char* start = end - (sizeof(Footer) + sizeof(ptrdiff_t));
        ptrdiff_t distanceNext;
        memmove(&distanceNext, start, sizeof(ptrdiff_t));
        free(start);
        return start + distanceNext;
    }


    char* fBase        {nullptr};
    char* fDtorCursor  {nullptr};
    char* fCursor      {nullptr};
    char* fEnd         {nullptr};

};

#endif//SkFixedAlloc_DEFINED
