/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFixedAlloc_DEFINED
#define SkFixedAlloc_DEFINED

#include "SkTypes.h"
#include <new>
#include <utility>

// SkFixedAlloc allocates arbitrary 16-byte aligned objects out of a fixed sized external buffer,
// and destroys them when destroyed.

class SkFixedAlloc {
public:
    // We'll use this buffer to allocate objects until exhausted.
    SkFixedAlloc(void* ptr, size_t len);
    ~SkFixedAlloc() { this->reset(); }

    // Allocate a new T in the buffer if possible.  If not, returns nullptr.
    template <typename T, typename... Args>
    T* make(Args&&... args) {
        size_t len = SkAlign16(sizeof(T));
        if (alignof(T) > 16 || fUsed + len + 16 > fLimit) {
            return nullptr;
        }
        auto ptr = (T*)(fBytes+fUsed);
        new (ptr) T{std::forward<Args>(args)...};
        fUsed += len;
        this->writeFooter([](void* ptr) { ((T*)ptr)->~T(); }, len);
        return ptr;
    }

    // Destroy the last object allocated and free its space in the buffer.
    void undo();

    // Destroy all objects and free all space in the buffer.
    void reset();

private:
    void writeFooter(void ( *dtor)(void*), size_t  len);
    void  readFooter(void (**dtor)(void*), size_t* len) const;

    char* fBytes;
    size_t fUsed, fLimit;
};

#endif//SkFixedAlloc_DEFINED
