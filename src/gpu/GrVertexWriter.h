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
 *  GrVertexWriter vertices{target->makeVertexSpace(...)};
 *  vertices.write(A0, B0, C0, ...);
 *  vertices.write(A1, B1, C1, ...);
 *
 * Supports any number of arguments. Each argument must be POD (plain old data), or an array
 * thereof.
 */
struct GrVertexWriter {
    void* fPtr;

    template <typename T, typename... Args>
    void write(const T& val, const Args&... remainder) {
        static_assert(std::is_pod<T>::value, "");
        static_assert(alignof(T) == 4, "");
        memcpy(fPtr, &val, sizeof(T));
        fPtr = SkTAddOffset<void>(fPtr, sizeof(T));
        this->write(remainder...);
    }

    template <typename T, size_t N, typename... Args>
    void write(const T(&val)[N], const Args&... remainder) {
        static_assert(std::is_pod<T>::value, "");
        static_assert(alignof(T) == 4, "");
        memcpy(fPtr, val, N * sizeof(T));
        fPtr = SkTAddOffset<void>(fPtr, N * sizeof(T));
        this->write(remainder...);
    }

    void write() {}

    /**
     * Specialized utility for writing a four-vertices, with some data being replicated at each
     * vertex, and other data being the appropriate 2-components from an SkRect to construct a
     * triangle strip.
     *
     * writeQuad(A, B, C, ...) is similar to write(A, B, C, ...), except that:
     *
     * - Four sets of data will be written
     * - For any arguments of type TriStrip, a unique SkPoint will be written at each vertex,
     *   in this order: left-top, left-bottom, right-top, right-bottom.
     */
    struct TriStrip { const SkRect& fRect; };

    template <typename... Args>
    void writeQuad(const Args&... remainder) {
        this->writeQuadVert<0>(remainder...);
        this->writeQuadVert<1>(remainder...);
        this->writeQuadVert<2>(remainder...);
        this->writeQuadVert<3>(remainder...);
    }

private:
    template <int corner, typename T, typename... Args>
    void writeQuadVert(const T& val, const Args&... remainder) {
        this->writeQuadValue<corner>(val);
        this->writeQuadVert<corner>(remainder...);
    }

    template <int corner>
    void writeQuadVert() {}

    template <int corner, typename T>
    void writeQuadValue(const T& val) {
        this->write(val);
    }

    template <int corner>
    void writeQuadValue(const TriStrip& r) {
        switch (corner) {
            case 0: this->write(r.fRect.fLeft , r.fRect.fTop);    break;
            case 1: this->write(r.fRect.fLeft , r.fRect.fBottom); break;
            case 2: this->write(r.fRect.fRight, r.fRect.fTop);    break;
            case 3: this->write(r.fRect.fRight, r.fRect.fBottom); break;
        }
    }
};

#endif
