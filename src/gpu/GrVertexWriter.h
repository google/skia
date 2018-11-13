/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVertexWriter_DEFINED
#define GrVertexWriter_DEFINED

#include "SkTemplates.h"
#include <type_traits>

/**
 * Helper for writing vertex data to a buffer. Usage:
 *  GrVertexWriter vertices(target->makeVertexSpace(...));
 *  vertices.write(A0, B0, C0, ...);
 *  vertices.write(A1, B1, C1, ...);
 *
 * Supports any number of arguments. Each argument must be POD (plain old data), or an array
 * thereof.
 */
struct GrVertexWriter {
    GrVertexWriter(void* ptr) : fPtr(ptr) {}

    template <typename T, typename... Args>
    inline void write(const T& val, const Args&... remainder) {
        static_assert(std::is_pod<T>::value, "");
        static_assert(alignof(T) == 4, "");
        memcpy(fPtr, &val, sizeof(T));
        fPtr = SkTAddOffset<void>(fPtr, sizeof(T));
        this->write(remainder...);
    }

    template <typename T, size_t N, typename... Args>
    inline void write(const T(&val)[N], const Args&... remainder) {
        static_assert(std::is_pod<T>::value, "");
        static_assert(alignof(T) == 4, "");
        memcpy(fPtr, val, N * sizeof(T));
        fPtr = SkTAddOffset<void>(fPtr, N * sizeof(T));
        this->write(remainder...);
    }

    inline void write() {}

    void* fPtr;
};

#endif
