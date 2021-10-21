/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVertexWriter_DEFINED
#define GrVertexWriter_DEFINED

#include "include/core/SkRect.h"
#include "include/private/SkNx.h"
#include "include/private/SkTemplates.h"
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
     * Specialized utilities for writing a four-vertices, with some data being replicated at each
     * vertex, and other data being the appropriate 2-components from an SkRect to construct a
     * triangle strip.
     *
     * - Four sets of data will be written
     *
     * - For any arguments where is_quad<Type>::value is true, a unique point will be written at
     *   each vertex. To make a custom type be emitted as a quad, declare:
     *
     *       template<> struct GrVertexWriter::is_quad<MyQuadClass> : std::true_type {};
     *
     *   and define:
     *
     *       MyQuadClass::writeVertex(int cornerIdx, GrVertexWriter&) const { ... }
     *
     * - For any arguments where is_quad<Type>::value is false, its value will be replicated at each
     *   vertex.
     */
    template <typename T>
    struct is_quad : std::false_type {};

    template <typename T>
    struct TriStrip {
        void writeVertex(int cornerIdx, GrVertexWriter& w) const {
            switch (cornerIdx) {
                case 0: w << l << t; return;
                case 1: w << l << b; return;
                case 2: w << r << t; return;
                case 3: w << r << b; return;
            }
            SkUNREACHABLE;
        }
        T l, t, r, b;
    };

    static TriStrip<float> TriStripFromRect(const SkRect& r) {
        return { r.fLeft, r.fTop, r.fRight, r.fBottom };
    }

    static TriStrip<uint16_t> TriStripFromUVs(const std::array<uint16_t, 4>& rect) {
        return { rect[0], rect[1], rect[2], rect[3] };
    }

    template <typename T>
    struct TriFan {
        void writeVertex(int cornerIdx, GrVertexWriter& w) const {
            switch (cornerIdx) {
                case 0: w << l << t; return;
                case 1: w << l << b; return;
                case 2: w << r << b; return;
                case 3: w << r << t; return;
            }
            SkUNREACHABLE;
        }
        T l, t, r, b;
    };

    static TriFan<float> TriFanFromRect(const SkRect& r) {
        return { r.fLeft, r.fTop, r.fRight, r.fBottom };
    }

    template <typename... Args>
    void writeQuad(const Args&... remainder) {
        this->writeQuadVertex<0>(remainder...);
        this->writeQuadVertex<1>(remainder...);
        this->writeQuadVertex<2>(remainder...);
        this->writeQuadVertex<3>(remainder...);
    }

private:
    template<typename T> friend GrVertexWriter& operator<<(GrVertexWriter&, const T&);

    template <int kCornerIdx, typename T, typename... Args>
    std::enable_if_t<!is_quad<T>::value, void> writeQuadVertex(const T& val,
                                                               const Args&... remainder) {
        *this << val;  // Non-quads duplicate their value.
        this->writeQuadVertex<kCornerIdx>(remainder...);
    }

    template <int kCornerIdx, typename Q, typename... Args>
    std::enable_if_t<is_quad<Q>::value, void> writeQuadVertex(const Q& quad,
                                                              const Args&... remainder) {
        quad.writeVertex(kCornerIdx, *this);  // Quads emit a different corner each time.
        this->writeQuadVertex<kCornerIdx>(remainder...);
    }

    template <int kCornerIdx>
    void writeQuadVertex() {}
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

template <typename T>
struct GrVertexWriter::is_quad<GrVertexWriter::TriStrip<T>> : std::true_type {};

template <typename T>
struct GrVertexWriter::is_quad<GrVertexWriter::TriFan<T>> : std::true_type {};

#endif
