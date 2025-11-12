/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_BufferWriter_DEFINED
#define skgpu_BufferWriter_DEFINED

#include "include/core/SkImageInfo.h"
#include "include/core/SkRect.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkTemplates.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkRectMemcpy.h"
#include "src/core/SkColorData.h"
#include "src/core/SkConvertPixels.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <type_traits>
#include <utility>

namespace skgpu {

namespace graphite {
    class VelloRenderer;
}

struct BufferWriter {
public:
    // Marks a read-only position in the underlying buffer
    struct Mark {
    public:
        Mark() : Mark(nullptr) {}
        Mark(void* ptr, size_t offset = 0)
                : fMark(reinterpret_cast<uintptr_t>(ptr) + offset) {
            SkASSERT(ptr || offset == 0);
        }

        bool operator< (const Mark& o) const { return fMark <  o.fMark; }
        bool operator<=(const Mark& o) const { return fMark <= o.fMark; }
        bool operator==(const Mark& o) const { return fMark == o.fMark; }
        bool operator!=(const Mark& o) const { return fMark != o.fMark; }
        bool operator>=(const Mark& o) const { return fMark >= o.fMark; }
        bool operator> (const Mark& o) const { return fMark >  o.fMark; }

        ptrdiff_t operator-(const Mark& o) const { return fMark - o.fMark; }

        explicit operator bool() const { return *this != Mark(); }
    private:
        uintptr_t fMark;
    };

    BufferWriter() = default;
    BufferWriter(BufferWriter&& w) { *this = std::move(w); }

    BufferWriter(void* ptr, size_t size) : fPtr(ptr) {
        SkDEBUGCODE(fEnd = Mark(ptr, ptr ? size : 0);)
    }
    BufferWriter(void* ptr, Mark end = {}) : fPtr(ptr) {
        SkDEBUGCODE(fEnd = end;)
    }

    BufferWriter& operator=(const BufferWriter&) = delete;
    BufferWriter& operator=(BufferWriter&& that) {
        fPtr = that.fPtr;
        that.fPtr = nullptr;
        SkDEBUGCODE(fEnd = that.fEnd;)
        SkDEBUGCODE(that.fEnd = Mark();)
        return *this;
    }

    explicit operator bool() const { return fPtr != nullptr; }

    Mark mark(size_t offset=0) const {
        this->validate(offset);
        return Mark(fPtr, offset);
    }

    void zeroBytes(size_t bytes) {
        auto s = this->slice(bytes);
        memset(s.data(), 0, s.size_bytes());
    }

    void write(const void* src, size_t bytes) {
        auto s = this->slice(bytes);
        memcpy(s.data(), src, s.size_bytes());
    }

    template <typename T>
    void write(SkSpan<const T> data) { this->write(data.data(), data.size_bytes()); }
    template <typename T>
    void write(T data) { this->write(&data, sizeof(T)); }

protected:
    // For integration with Rust, to expose slice() directly
    friend class skgpu::graphite::VelloRenderer;

    // makeOffset effectively splits the current writer from {fPtr, fEnd} into {fPtr, p} and
    // a new writer {p, fEnd}. The same data range is accessible, but each byte can only be
    // set by a single writer. Automatically validates that there is enough bytes remaining in this
    // writer to do such a split.
    //
    // This splitting and validation means that providers of BufferWriters to callers can easily
    // and correctly track everything in a single BufferWriter field and use
    //    return std::exchange(fCurrWriter, fCurrWriter.makeOffset(requestedBytes));
    // This exposes the current writer position to the caller and sets the provider's new current
    // position to be just after the requested bytes.
    //
    // Templated so that it can create subclasses directly.
    template<typename W>
    W makeOffset(size_t offsetInBytes) const {
        this->validate(offsetInBytes);
        void* p = SkTAddOffset<void>(fPtr, offsetInBytes);
        Mark end{SkDEBUGCODE(fEnd)};
        SkDEBUGCODE(fEnd = Mark(p);)
        return W{p, end};
    }

    // The Writer's pointer is advanced by `bytes` just as though `write()` had been called. The
    // returned pointer should not be used for reading, and should only write to each index once, in
    // order, for optimal performance.
    SkSpan<uint8_t> slice(size_t bytes) {
        this->validate(bytes);
        SkSpan<uint8_t> slice{static_cast<uint8_t*>(fPtr), bytes};
        fPtr = SkTAddOffset<void>(fPtr, bytes);
        return slice;
    }

    void validate(size_t bytesToWrite) const {
        // If the buffer writer had an end marked, make sure we're not crossing it.
        // Ideally, all creators of BufferWriters mark the end, but a lot of legacy code is not set
        // up to easily do this.
        SkASSERT(fPtr || bytesToWrite == 0);
        SkASSERT(!fEnd || Mark(fPtr, bytesToWrite) <= fEnd);
    }

    void* fPtr = nullptr;
    SkDEBUGCODE(mutable Mark fEnd = {};)
};

#define BUFFER_WRITER_OVERLOADS(Writer) \
    Writer() = default; \
    Writer(void* ptr, size_t size) : BufferWriter(ptr, size) {} \
    Writer(void* ptr, Mark end) : BufferWriter(ptr, end) {} \
    Writer(const Writer&) = delete; \
    Writer(Writer&& that) { *this = std::move(that); } \
    Writer(BufferWriter&& that) { *this = std::move(that); } \
    Writer& operator=(const Writer&) = delete; \
    Writer& operator=(Writer&& that) { \
        BufferWriter::operator=(std::move(that)); \
        return *this; \
    } \
    Writer& operator=(BufferWriter&& that) { \
        BufferWriter::operator=(std::move(that)); \
        return *this; \
    } \
    using BufferWriter::operator bool;

/**
 * Helper for writing vertex data to a buffer. Usage:
 *  VertexWriter vertices{target->makeVertexSpace(...)};
 *  vertices << A0 << B0 << C0 << ...;
 *  vertices << A1 << B1 << C1 << ...;
 *
 * Each value must be POD (plain old data), or have a specialization of the "<<" operator.
 */
struct VertexWriter : private BufferWriter {
    inline constexpr static uint32_t kIEEE_32_infinity = 0x7f800000;

    // DEPRECATED: Prefer specifying the size of the buffer being written to as well
    explicit VertexWriter(void* ptr) : BufferWriter(ptr, Mark()) {}

    BUFFER_WRITER_OVERLOADS(VertexWriter)

    using BufferWriter::mark;
    using BufferWriter::zeroBytes;

    VertexWriter makeOffset(size_t offsetInBytes) const {
        return this->BufferWriter::makeOffset<VertexWriter>(offsetInBytes);
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
    static_assert(std::is_trivially_copyable<T>::value, "");
    w.write(&val, sizeof(T));
    return w;
}

template <typename T>
inline VertexWriter& operator<<(VertexWriter& w, const VertexWriter::Conditional<T>& val) {
    static_assert(std::is_trivially_copyable<T>::value, "");
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
    static_assert(std::is_trivially_copyable<T>::value, "");
    w.write(SkSpan<const T>{array.fArray, (size_t)array.fCount});
    return w;
}

template <int kCount, typename T>
inline VertexWriter& operator<<(VertexWriter& w, const VertexWriter::RepeatDesc<kCount,T>& repeat) {
    for (int i = 0; i < kCount; ++i) {
        w << repeat.fVal;
    }
    return w;
}

// Allow r-value/temporary writers to be appended to
template <typename T>
inline VertexWriter& operator<<(VertexWriter&& w, const T& val) { return w << val; }

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
[[maybe_unused]] inline VertexWriter& operator<<(VertexWriter& w, const VertexColor& color) {
    w << color.fColor[0];
    if (color.fWideColor) {
        w << color.fColor[1]
          << color.fColor[2]
          << color.fColor[3];
    }
    return w;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

struct IndexWriter : private BufferWriter {
    BUFFER_WRITER_OVERLOADS(IndexWriter)

    IndexWriter makeOffset(int numIndices) const {
        return this->BufferWriter::makeOffset<IndexWriter>(numIndices * sizeof(uint16_t));
    }

    void writeArray(SkSpan<const uint16_t> indices) {
        this->write(indices);
    }

    friend IndexWriter& operator<<(IndexWriter& w, uint16_t val);
};

inline IndexWriter& operator<<(IndexWriter& w, uint16_t val) {
    w.write(&val, sizeof(uint16_t));
    return w;
}

inline IndexWriter& operator<<(IndexWriter& w, int val) { return (w << SkTo<uint16_t>(val)); }

///////////////////////////////////////////////////////////////////////////////////////////////////

struct TextureUploadWriter : private BufferWriter {
    BUFFER_WRITER_OVERLOADS(TextureUploadWriter)

    // TODO(michaelludwig): This API doesn't prevent the underlying buffer from being written
    // multiple times, which would be nice to do.

    // Writes a block of image data to the upload buffer, starting at `offset`. The source image is
    // `srcRowBytes` wide, and the written block is `dstRowBytes` wide and `rowCount` bytes tall.
    void write(size_t offset, const void* src, size_t srcRowBytes, size_t dstRowBytes,
               size_t trimRowBytes, int rowCount) {
        this->validate(offset + dstRowBytes * rowCount);
        void* dst = SkTAddOffset<void>(fPtr, offset);
        SkRectMemcpy(dst, dstRowBytes, src, srcRowBytes, trimRowBytes, rowCount);
    }

    void convertAndWrite(size_t offset,
                         const SkImageInfo& srcInfo, const void* src, size_t srcRowBytes,
                         const SkImageInfo& dstInfo, size_t dstRowBytes) {
        SkASSERT(srcInfo.width() == dstInfo.width() && srcInfo.height() == dstInfo.height());
        this->validate(offset + dstRowBytes * dstInfo.height());
        void* dst = SkTAddOffset<void>(fPtr, offset);
        SkAssertResult(SkConvertPixels(dstInfo, dst, dstRowBytes, srcInfo, src, srcRowBytes));
    }

    // Writes a block of image data to the upload buffer. It converts src data of RGB_888x
    // colorType into a 3 channel RGB_888 format.
    void writeRGBFromRGBx(size_t offset, const void* src, size_t srcRowBytes, size_t dstRowBytes,
                          int rowPixels, int rowCount) {
        this->validate(offset + dstRowBytes * rowCount);
        void* dst = SkTAddOffset<void>(fPtr, offset);
        auto* sRow = reinterpret_cast<const char*>(src);
        auto* dRow = reinterpret_cast<char*>(dst);

        for (int y = 0; y < rowCount; ++y) {
            for (int x = 0; x < rowPixels; ++x) {
                memcpy(dRow + 3*x, sRow+4*x, 3);
            }
            sRow += srcRowBytes;
            dRow += dstRowBytes;
        }
    }
};

}  // namespace skgpu

#endif // skgpu_BufferWriter_DEFINED
