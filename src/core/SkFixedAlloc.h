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

#if defined(__GNUC__) && !defined(__clang__) && __GNUC__ < 5
    namespace std { using is_trivially_copyable = has_trivial_copy_constructor; }
#endif

// SkFixedAlloc allocates POD objects out of a fixed-size buffer.
class SkFixedAlloc {
public:
    SkFixedAlloc(void* ptr, size_t len);
    ~SkFixedAlloc() { this->reset(); }

    // Allocates space for a T in the buffer if possible.  If not, returns nullptr.
    template <typename T>
    void* alloc() {
        static_assert(std::is_standard_layout   <T>::value
                   && std::is_trivially_copyable<T>::value, "");

        auto aligned = ((uintptr_t)(fBuffer+fUsed) + alignof(T) - 1) & ~(alignof(T)-1);
        size_t skip = aligned - (uintptr_t)(fBuffer+fUsed);

        return (T*)this->alloc(skip, sizeof(T));
    }

    template <typename T, typename... Args>
    T* make(Args&&... args) {
        if (auto ptr = this->alloc<T>()) {
            new (ptr) T(std::forward<Args>(args)...);
            return (T*)ptr;
        }
        return nullptr;
    }

    template <typename T>
    T* copy(const T& src) { return this->make<T>(src); }

    // Frees the space of the last object allocated.
    void undo();

    // Frees all space in the buffer.
    void reset();

private:
    void* alloc(size_t skip, size_t len);

    char* fBuffer;
    size_t fUsed, fLimit;
};

class SkFallbackAlloc {
public:
    explicit SkFallbackAlloc(SkFixedAlloc*);
    ~SkFallbackAlloc() { this->reset(); }

    template <typename T>
    void* alloc() {
        if (fHeapAllocs.empty()) {
            if (auto ptr = fFixedAlloc->alloc<T>()) {
                return ptr;
            }
        }
        auto ptr = sk_malloc_throw(sizeof(T));
        fHeapAllocs.push_back(ptr);
        return ptr;
    }

    template <typename T, typename... Args>
    T* make(Args&&... args) {
        auto ptr = this->alloc<T>();
        new (ptr) T(std::forward<Args>(args)...);
        return (T*)ptr;
    }

    template <typename T>
    T* copy(const T& src) { return this->make<T>(src); }

    // Frees the last object allocated, including any space it used in the SkFixedAlloc.
    void undo();

    // Frees all objects and all space in the SkFixedAlloc.
    void reset();

private:
    SkFixedAlloc*      fFixedAlloc;
    std::vector<void*> fHeapAllocs;
};

#endif//SkFixedAlloc_DEFINED
