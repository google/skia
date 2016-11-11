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
    template <typename T, typename... Args>
    T* make(Args&&... args) {
        auto aligned = ((uintptr_t)(fBuffer+fUsed) + alignof(T) - 1) & ~(alignof(T)-1);
        size_t skip = aligned - (uintptr_t)(fBuffer+fUsed);

        if (!SkTFitsIn<uint32_t>(skip)      ||
            !SkTFitsIn<uint32_t>(sizeof(T)) ||
            fUsed + skip + sizeof(T) + sizeof(Footer) > fLimit) {
            return nullptr;
        }

        // Skip ahead until our buffer is aligned for T.
        fUsed += skip;

        // Create the T.
        auto ptr = (T*)(fBuffer+fUsed);
        new (ptr) T{std::forward<Args>(args)...};
        fUsed += sizeof(T);

        // Stamp a footer after the T that we can use to clean it up.
        Footer footer = { [](void* ptr) { ((T*)ptr)->~T(); }, SkToU32(skip), SkToU32(sizeof(T)) };
        memcpy(fBuffer+fUsed, &footer, sizeof(Footer));
        fUsed += sizeof(Footer);

        return ptr;
    }

    // Destroys the last object allocated and frees its space in the buffer.
    void undo();

    // Destroys all objects and frees all space in the buffer.
    void reset();

private:
    struct Footer {
        void (*dtor)(void*);
        uint32_t skip, len;
    };

    char* fBuffer;
    size_t fUsed, fLimit;
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
        auto ptr = new T{std::forward<Args>(args)...};
        fHeapAllocs.push_back({[](void* ptr) { delete (T*)ptr; }, ptr});
        return ptr;
    }

    // Destroys the last object allocated and frees any space it used in the SkFixedAlloc.
    void undo();

    // Destroys all objects and frees all space in the SkFixedAlloc.
    void reset();

private:
    struct HeapAlloc {
        void (*deleter)(void*);
        void* ptr;
    };

    SkFixedAlloc*          fFixedAlloc;
    std::vector<HeapAlloc> fHeapAllocs;
};

#endif//SkFixedAlloc_DEFINED
