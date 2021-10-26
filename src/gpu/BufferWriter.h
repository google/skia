/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef BufferWriter_DEFINED
#define BufferWriter_DEFINED

#include "include/core/SkRect.h"
#include "include/private/SkNx.h"
#include "include/private/SkTemplates.h"
#include <type_traits>

namespace skgpu {

struct BufferWriter {
public:
    operator bool() const { return fPtr != nullptr; }

protected:
    BufferWriter() = default;
    BufferWriter(void* ptr) : fPtr(ptr) {}

    BufferWriter& operator=(const BufferWriter&) = delete;
    BufferWriter& operator=(BufferWriter&& that) {
        fPtr = that.fPtr;
        that.fPtr = nullptr;
        return *this;
    }

protected:
    void* fPtr;
};

/**
 * Helper for writing vertex data to a buffer. Usage:
 *  VertexWriter vertices{target->makeVertexSpace(...)};
 *  vertices << A0 << B0 << C0 << ...;
 *  vertices << A1 << B1 << C1 << ...;
 *
 * Each value must be POD (plain old data), or have a specialization of the "<<" operator.
 */
struct VertexWriter : public BufferWriter {
    inline constexpr static uint32_t kIEEE_32_infinity = 0x7f800000;

    VertexWriter() = default;
    VertexWriter(void* ptr) : BufferWriter(ptr) {}
    VertexWriter(const VertexWriter&) = delete;
    VertexWriter(VertexWriter&& that) { *this = std::move(that); }

    VertexWriter& operator=(const VertexWriter&) = delete;
    VertexWriter& operator=(VertexWriter&& that) {
        BufferWriter::operator=(std::move(that));
        return *this;
    }

    bool operator==(const VertexWriter& that) const { return fPtr == that.fPtr; }

    // TODO: Remove this call. We want all users of VertexWriter to have to go through the vertex
    // writer functions to write data. We do not want them to directly access fPtr and copy their
    // own data.
    void* ptr() const { return fPtr; }

    VertexWriter makeOffset(ptrdiff_t offsetInBytes) const {
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
     *       template<> struct VertexWriter::is_quad<MyQuadClass> : std::true_type {};
     *
     *   and define:
     *
     *       MyQuadClass::writeVertex(int cornerIdx, VertexWriter&) const { ... }
     *
     * - For any arguments where is_quad<Type>::value is false, its value will be replicated at each
     *   vertex.
     */
    template <typename T>
    struct is_quad : std::false_type {};

    template <typename T>
    struct TriStrip {
        void writeVertex(int cornerIdx, VertexWriter& w) const {
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
        void writeVertex(int cornerIdx, VertexWriter& w) const {
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

    template <typename T> friend VertexWriter& operator<<(VertexWriter& w, const T& val);
};

template <typename T>
inline VertexWriter& operator<<(VertexWriter& w, const T& val) {
    static_assert(std::is_pod<T>::value, "");
    memcpy(w.fPtr, &val, sizeof(T));
    w = w.makeOffset(sizeof(T));
    return w;
}

template <typename T>
inline VertexWriter& operator<<(VertexWriter& w, const VertexWriter::Conditional<T>& val) {
    static_assert(std::is_pod<T>::value, "");
    if (val.fCondition) {
        w << val.fValue;
    }
    return w;
}

template <typename T>
inline VertexWriter& operator<<(VertexWriter& w, const VertexWriter::Skip<T>& val) {
    w = w.makeOffset(sizeof(T));
    return w;
}

template <>
SK_MAYBE_UNUSED inline VertexWriter& operator<<(VertexWriter& w, const Sk4f& vector) {
    vector.store(w.fPtr);
    w = w.makeOffset(sizeof(vector));
    return w;
}

template <typename T>
struct VertexWriter::is_quad<VertexWriter::TriStrip<T>> : std::true_type {};

template <typename T>
struct VertexWriter::is_quad<VertexWriter::TriFan<T>> : std::true_type {};

///////////////////////////////////////////////////////////////////////////////////////////////////

struct IndexWriter : public BufferWriter {
    IndexWriter() = default;
    IndexWriter(void* ptr) : BufferWriter(ptr) {}
    IndexWriter(const IndexWriter&) = delete;
    IndexWriter(IndexWriter&& that) { *this = std::move(that); }

    IndexWriter& operator=(const IndexWriter&) = delete;
    IndexWriter& operator=(IndexWriter&& that) {
        BufferWriter::operator=(std::move(that));
        return *this;
    }

    IndexWriter makeAdvance(int numIndices) const {
        return {SkTAddOffset<void>(fPtr, numIndices * sizeof(uint16_t))};
    }

    void writeArray(const uint16_t* array, int count) {
        memcpy(fPtr, array, count * sizeof(uint16_t));
        fPtr = SkTAddOffset<void>(fPtr, count * sizeof(uint16_t));
    }

    friend IndexWriter& operator<<(IndexWriter& w, uint16_t val);
};

inline IndexWriter& operator<<(IndexWriter& w, uint16_t val) {
    memcpy(w.fPtr, &val, sizeof(uint16_t));
    w = w.makeAdvance(1);
    return w;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

struct UniformWriter : public BufferWriter {
    UniformWriter() = default;
    UniformWriter(void* ptr) : BufferWriter(ptr) {}
    UniformWriter(const UniformWriter&) = delete;
    UniformWriter(UniformWriter&& that) { *this = std::move(that); }

    UniformWriter& operator=(const UniformWriter&) = delete;
    UniformWriter& operator=(UniformWriter&& that) {
        BufferWriter::operator=(std::move(that));
        return *this;
    }

    void write(const void* src, size_t bytes) {
        memcpy(fPtr, src, bytes);
        fPtr = SkTAddOffset<void>(fPtr, bytes);
    }
};

}  // namespace skgpu

#endif // BufferWriter_DEFINED
