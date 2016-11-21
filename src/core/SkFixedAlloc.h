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

        // TODO: combine both if statments when study is done.
        if (objStart + sizeof(T) + sizeof(Footer) > fEnd) {
            return nullptr;
        }

        if (padding >= 32
            || deleterDiff >= (1 << 26)
            || deleterDiff < -(1 << 26)) {
            // Can't encode padding or deleter function offset.
            SkDebugf("SkFixedAlloc - padding: %dt, deleteDiff: %dt\n", padding, deleterDiff);
            SkFAIL("Failed to allocate due to constraint.");
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

#endif//SkFixedAlloc_DEFINED
