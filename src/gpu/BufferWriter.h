/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_BufferWriter_DEFINED
#define skgpu_BufferWriter_DEFINED

#include "include/core/SkRect.h"
#include "include/private/SkColorData.h"
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

    template<typename T>
    struct ArrayDesc {
        const T* fArray;
        int fCount;
    };

    template <typename T>
    static ArrayDesc<T> Array(const T* array, int count) {
        return {array, count};
    }

    template<int kCount, typename T>
    struct RepeatDesc {
        const T& fVal;
    };

    template <int kCount, typename T>
    static RepeatDesc<kCount, T> Repeat(const T& val) {
        return {val};
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

    template <typename T>
    friend VertexWriter& operator<<(VertexWriter&, const T&);

    template <typename T>
    friend VertexWriter& operator<<(VertexWriter&, const ArrayDesc<T>&);
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

template <typename T>
inline VertexWriter& operator<<(VertexWriter& w, const VertexWriter::ArrayDesc<T>& array) {
    static_assert(std::is_pod<T>::value, "");
    memcpy(w.fPtr, array.fArray, array.fCount * sizeof(T));
    w = w.makeOffset(sizeof(T) * array.fCount);
    return w;
}

template <int kCount, typename T>
inline VertexWriter& operator<<(VertexWriter& w, const VertexWriter::RepeatDesc<kCount,T>& repeat) {
    for (int i = 0; i < kCount; ++i) {
        w << repeat.fVal;
    }
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

/**
 * VertexColor is a helper for writing colors to a vertex buffer. It outputs either four bytes or
 * or four float32 channels, depending on the wideColor parameter. Note that the GP needs to have
 * been constructed with the correct attribute type for colors, to match the usage here.
 */
class VertexColor {
public:
    VertexColor() = default;

    explicit VertexColor(const SkPMColor4f& color, bool wideColor) {
        this->set(color, wideColor);
    }

    void set(const SkPMColor4f& color, bool wideColor) {
        if (wideColor) {
            memcpy(fColor, color.vec(), sizeof(fColor));
        } else {
            fColor[0] = color.toBytes_RGBA();
        }
        fWideColor = wideColor;
    }

    size_t size() const { return fWideColor ? 16 : 4; }

private:
    template <typename T>
    friend VertexWriter& operator<<(VertexWriter&, const T&);

    uint32_t fColor[4];
    bool     fWideColor;
};

template <>
SK_MAYBE_UNUSED inline VertexWriter& operator<<(VertexWriter& w, const VertexColor& color) {
    w << color.fColor[0];
    if (color.fWideColor) {
        w << color.fColor[1]
          << color.fColor[2]
          << color.fColor[3];
    }
    return w;
}

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

inline IndexWriter& operator<<(IndexWriter& w, int val) { return (w << SkTo<uint16_t>(val)); }

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

#endif // skgpu_BufferWriter_DEFINED
