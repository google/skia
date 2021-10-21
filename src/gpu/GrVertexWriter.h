/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVertexWriter_DEFINED
#define GrVertexWriter_DEFINED

#include "include/private/SkNx.h"
#include "include/private/SkTemplates.h"
#include "src/gpu/geometry/GrQuad.h"
#include <type_traits>

/**
 * Helper for writing vertex data to a buffer. Usage:
 *  GrVertexWriter vertices{target->makeVertexSpace(...)};
 *  vertices << A0 << B0 << C0 << ...;
 *  vertices << A1 << B1 << C1 << ...;
 *
 * Each value must be POD (plain old data), or have a specialization of the "<<" operator.
 */
struct GrVertexWriter {
    inline constexpr static uint32_t kIEEE_32_infinity = 0x7f800000;

    void* fPtr;

    GrVertexWriter() = default;
    GrVertexWriter(void* ptr) : fPtr(ptr) {}
    GrVertexWriter(const GrVertexWriter&) = delete;
    GrVertexWriter(GrVertexWriter&& that) { *this = std::move(that); }

    GrVertexWriter& operator=(const GrVertexWriter&) = delete;
    GrVertexWriter& operator=(GrVertexWriter&& that) {
        fPtr = that.fPtr;
        that.fPtr = nullptr;
        return *this;
    }

    bool operator==(const GrVertexWriter& that) const { return fPtr == that.fPtr; }
    operator bool() const { return fPtr != nullptr; }

    GrVertexWriter makeOffset(ptrdiff_t offsetInBytes) const {
        return {SkTAddOffset<void>(fPtr, offsetInBytes)};
    }

    template <typename T>
    struct Conditional {
        bool fCondition;
        T fValue;
    };

    template <typename T>
    static Conditional<T> If(bool condition, const T& value) {
        return {condition, value};
    }

    template <typename T>
    struct Skip {};

    template <typename T>
    void writeArray(const T* array, int count) {
        static_assert(std::is_pod<T>::value, "");
        memcpy(fPtr, array, count * sizeof(T));
        fPtr = SkTAddOffset<void>(fPtr, count * sizeof(T));
    }

    template <typename T>
    void fill(const T& val, int repeatCount) {
        for (int i = 0; i < repeatCount; ++i) {
            *this << val;
        }
    }

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
    template <typename T>
    struct TriStrip { T l, t, r, b; };

    static TriStrip<float> TriStripFromRect(const SkRect& r) {
        return { r.fLeft, r.fTop, r.fRight, r.fBottom };
    }

    static TriStrip<uint16_t> TriStripFromUVs(const std::array<uint16_t, 4>& rect) {
        return { rect[0], rect[1], rect[2], rect[3] };
    }

    template <typename T>
    struct TriFan { T l, t, r, b; };

    static TriFan<float> TriFanFromRect(const SkRect& r) {
        return { r.fLeft, r.fTop, r.fRight, r.fBottom };
    }

    template <typename... Args>
    void writeQuad(const Args&... remainder) {
        this->writeQuadVert<0>(remainder...);
        this->writeQuadVert<1>(remainder...);
        this->writeQuadVert<2>(remainder...);
        this->writeQuadVert<3>(remainder...);
    }

private:
    template<typename T> friend GrVertexWriter& operator<<(GrVertexWriter&, const T&);

    template <int corner, typename T, typename... Args>
    void writeQuadVert(const T& val, const Args&... remainder) {
        this->writeQuadValue<corner>(val);
        this->writeQuadVert<corner>(remainder...);
    }

    template <int corner>
    void writeQuadVert() {}

    template <int corner, typename T>
    void writeQuadValue(const T& val) {
        *this << val;
    }

    template <int corner, typename T>
    void writeQuadValue(const TriStrip<T>& r) {
        switch (corner) {
            case 0: *this << r.l << r.t; break;
            case 1: *this << r.l << r.b; break;
            case 2: *this << r.r << r.t; break;
            case 3: *this << r.r << r.b; break;
        }
    }

    template <int corner, typename T>
    void writeQuadValue(const TriFan<T>& r) {
        switch (corner) {
            case 0: *this << r.l << r.t; break;
            case 1: *this << r.l << r.b; break;
            case 2: *this << r.r << r.b; break;
            case 3: *this << r.r << r.t; break;
        }
    }

    template <int corner>
    void writeQuadValue(const GrQuad& q) {
        *this << q.point(corner);
    }
};

template <typename T>
inline GrVertexWriter& operator<<(GrVertexWriter& w, const T& val) {
    static_assert(std::is_pod<T>::value, "");
    memcpy(w.fPtr, &val, sizeof(T));
    w = w.makeOffset(sizeof(T));
    return w;
}

template <typename T>
inline GrVertexWriter& operator<<(GrVertexWriter& w, const GrVertexWriter::Conditional<T>& val) {
    static_assert(std::is_pod<T>::value, "");
    if (val.fCondition) {
        w << val.fValue;
    }
    return w;
}

template <typename T>
inline GrVertexWriter& operator<<(GrVertexWriter& w, const GrVertexWriter::Skip<T>& val) {
    w = w.makeOffset(sizeof(T));
    return w;
}

template <>
SK_MAYBE_UNUSED inline GrVertexWriter& operator<<(GrVertexWriter& w, const Sk4f& vector) {
    vector.store(w.fPtr);
    w = w.makeOffset(sizeof(vector));
    return w;
}

#endif
