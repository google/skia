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

// Before GCC 5, is_trivially_copyable had a pre-standard name.
#if defined(__GLIBCXX__) && (__GLIBCXX__ < 20150801)
    namespace std {
        template <typename T>
        using is_trivially_copyable = has_trivial_copy_constructor<T>;
    }
#endif

// SkFixedAlloc allocates POD objects out of a fixed-size buffer.
class SkFixedAlloc {
public:
    SkFixedAlloc(void* ptr, size_t len);
    ~SkFixedAlloc() { this->reset(); }

    // Allocates space suitable for a T.  If we can't, returns nullptr.
    template <typename T>
    void* alloc() {
        static_assert(std::is_standard_layout   <T>::value
                   && std::is_trivially_copyable<T>::value, "");
        return this->alloc(sizeof(T), alignof(T));
    }

    template <typename T, typename... Args>
    T* make(Args&&... args) {
        if (auto ptr = this->alloc<T>()) {
            return new (ptr) T(std::forward<Args>(args)...);
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
    void* alloc(size_t, size_t);

    char* fBuffer;
    size_t fUsed, fLimit;
};

class SkFallbackAlloc {
public:
    explicit SkFallbackAlloc(SkFixedAlloc*);
    ~SkFallbackAlloc() { this->reset(); }

    template <typename T>
    void* alloc() {
        // Once we go heap we never go back to fixed.  This keeps undo() working.
        if (fHeapAllocs.empty()) {
            if (void* ptr = fFixedAlloc->alloc<T>()) {
                return ptr;
            }
        }
        void* ptr = sk_malloc_throw(sizeof(T));
        fHeapAllocs.push_back(ptr);
        return ptr;
    }

    template <typename T, typename... Args>
    T* make(Args&&... args) { return new (this->alloc<T>()) T(std::forward<Args>(args)...); }

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
