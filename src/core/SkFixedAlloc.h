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
#include <cmath>
#include <new>
#include <utility>
#include <vector>

// max_align_t is needed to calculate the alignment for createWithIniterT when the T used is an
// abstract type. The complication with max_align_t is that it is defined differently for
// different builds.
namespace {
#if defined(SK_BUILD_FOR_WIN32) || defined(SK_BUILD_FOR_MAC)
// Use std::max_align_t for compiles that follow the standard.
#include <cstddef>
using SkStdMaxAlignT = std::max_align_t;
#else
// Ubuntu compiles don't have std::max_align_t defined, but MSVC does not define max_align_t.
    #include <stddef.h>
    using SkStdMaxAlignT = max_align_t;
#endif
}

// SkFixedAlloc allocates objects out of a fixed-size buffer and destroys them when destroyed.
// This code accounts for two alignments 8 and 16. If an object's alignment is less than 8, it is
// rounded up to 8. Therefore, there is only a single alignment adjustment for allocating a
// pointer. This single bit is encoded in the cleanup function that is used.
class SkFixedAlloc {
public:
    SkFixedAlloc(void* ptr, size_t len);
    ~SkFixedAlloc() { this->reset(); }

    // Allocates a new T in the buffer if possible.  If not, returns nullptr.
    template <typename T, typename... Args>
    T* make(Args&&... args) {
        size_t alignmentAdjust = 0;
        if (alignof(T) == 16 && (fUsed & 0xF) != 0) {
            alignmentAdjust = 8;
        }

        if (fUsed + alignmentAdjust + sizeof(T) + sizeof(Footer) > fLimit) {
            return nullptr;
        }

        // Skip ahead until our buffer is aligned for T.
        fUsed += alignmentAdjust;

        // Make space for T.
        auto ptr = (T*)(fBuffer+fUsed);
        fUsed += sizeof(T);

        Footer footer;
        // Stamp a footer after the T that we can use to clean it up.
        if (alignmentAdjust == 0) {
            footer = {
                [](char* ptr) -> size_t {
                    char* objStart = ptr - (sizeof(T) + sizeof(Footer));
                    ((T*)objStart)->~T();
                    return sizeof(T) + sizeof(Footer);
                }
            };
        } else {
            footer = {
                [](char* ptr) -> size_t {
                    char* objStart = ptr - (sizeof(T) + sizeof(Footer));
                    ((T*)objStart)->~T();
                    return sizeof(T) + sizeof(Footer) + 8;
                }
            };
        }

        memcpy(fBuffer+fUsed, &footer, sizeof(Footer));
        fUsed += sizeof(Footer);

        // Creating a T must be last for nesting to work.
        return new (ptr) T(std::forward<Args>(args)...);
    }

    // Destroys the last object allocated and frees its space in the buffer.
    void undo();

    // Destroys all objects and frees all space in the buffer.
    void reset();

private:
    struct Footer {
        size_t (*dtor)(char*);
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
