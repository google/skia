/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrQuad_DEFINED
#define GrQuad_DEFINED

#include "SkMatrix.h"
#include "SkNx.h"
#include "SkPoint.h"
#include "SkPoint3.h"
#include "SkTArray.h"

enum class GrAAType : unsigned;
enum class GrQuadAAFlags;

// Rectangles transformed by matrices (view or local) can be classified in three ways:
//  1. Stays a rectangle - the matrix rectStaysRect() is true, or x(0) == x(1) && x(2) == x(3)
//     and y(0) == y(2) && y(1) == y(3). Or under mirrors, x(0) == x(2) && x(1) == x(3) and
//     y(0) == y(1) && y(2) == y(3).
//  2. Is rectilinear - the matrix does not have skew or perspective, but may rotate (unlike #1)
//  3. Is a quadrilateral - the matrix does not have perspective, but may rotate or skew, or
//     ws() == all ones.
//  4. Is a perspective quad - the matrix has perspective, subsuming all previous quad types.
enum class GrQuadType {
    kRect,
    kRectilinear,
    kStandard,
    kPerspective,
    kLast = kPerspective
};
static const int kGrQuadTypeCount = static_cast<int>(GrQuadType::kLast) + 1;

// If an SkRect is transformed by this matrix, what class of quad is required to represent it.
GrQuadType GrQuadTypeForTransformedRect(const SkMatrix& matrix);
// Perform minimal analysis of 'pts' (which are suitable for MakeFromSkQuad), and determine a
// quad type that will be as minimally general as possible.
GrQuadType GrQuadTypeForPoints(const SkPoint pts[4], const SkMatrix& matrix);

// Resolve disagreements between the overall requested AA type and the per-edge quad AA flags.
// knownQuadType must have come from GrQuadTypeForTransformedRect with the matrix that created the
// provided quad. Both outAAType and outEdgeFlags will be updated.
template <typename Q>
void GrResolveAATypeForQuad(GrAAType requestedAAType, GrQuadAAFlags requestedEdgeFlags,
                            const Q& quad, GrQuadType knownQuadType,
                            GrAAType* outAAtype, GrQuadAAFlags* outEdgeFlags);

/**
 * GrQuad is a collection of 4 points which can be used to represent an arbitrary quadrilateral. The
 * points make a triangle strip with CCW triangles (top-left, bottom-left, top-right, bottom-right).
 */
class GrQuad {
public:
    GrQuad() = default;

    GrQuad(const GrQuad& that) = default;

    explicit GrQuad(const SkRect& rect)
            : fX{rect.fLeft, rect.fLeft, rect.fRight, rect.fRight}
            , fY{rect.fTop, rect.fBottom, rect.fTop, rect.fBottom} {}

    GrQuad(const Sk4f& xs, const Sk4f& ys) {
        xs.store(fX);
        ys.store(fY);
    }

    explicit GrQuad(const SkPoint pts[4])
            : fX{pts[0].fX, pts[1].fX, pts[2].fX, pts[3].fX}
            , fY{pts[0].fY, pts[1].fY, pts[2].fY, pts[3].fY} {}

    /** Sets the quad to the rect as transformed by the matrix. */
    static GrQuad MakeFromRect(const SkRect&, const SkMatrix&);

    // Creates a GrQuad from the quadrilateral 'pts', transformed by the matrix. Unlike the explicit
    // constructor, the input points array is arranged as per SkRect::toQuad (top-left, top-right,
    // bottom-right, bottom-left). The returned instance's point order will still be CCW tri-strip
    // order.
    static GrQuad MakeFromSkQuad(const SkPoint pts[4], const SkMatrix&);

    GrQuad& operator=(const GrQuad& that) = default;

    SkPoint point(int i) const { return {fX[i], fY[i]}; }

    SkRect bounds() const {
        auto x = this->x4f(), y = this->y4f();
        return {x.min(), y.min(), x.max(), y.max()};
    }

    float x(int i) const { return fX[i]; }
    float y(int i) const { return fY[i]; }

    Sk4f x4f() const { return Sk4f::Load(fX); }
    Sk4f y4f() const { return Sk4f::Load(fY); }

    // True if anti-aliasing affects this quad. Only valid when quadType == kRect_QuadType
    bool aaHasEffectOnRect() const;

private:
    template<typename T>
    friend class GrQuadBuffer;

    float fX[4];
    float fY[4];
};

class GrPerspQuad {
public:
    GrPerspQuad() = default;

    explicit GrPerspQuad(const SkRect& rect)
            : fX{rect.fLeft, rect.fLeft, rect.fRight, rect.fRight}
            , fY{rect.fTop, rect.fBottom, rect.fTop, rect.fBottom}
            , fW{1.f, 1.f, 1.f, 1.f} {}

    GrPerspQuad(const Sk4f& xs, const Sk4f& ys) {
        xs.store(fX);
        ys.store(fY);
        fW[0] = fW[1] = fW[2] = fW[3] = 1.f;
    }

    GrPerspQuad(const Sk4f& xs, const Sk4f& ys, const Sk4f& ws) {
        xs.store(fX);
        ys.store(fY);
        ws.store(fW);
    }

    static GrPerspQuad MakeFromRect(const SkRect&, const SkMatrix&);

    // Creates a GrPerspQuad from the quadrilateral 'pts', transformed by the matrix. The input
    // points array is arranged as per SkRect::toQuad (top-left, top-right, bottom-right,
    // bottom-left). The returned instance's point order will still be CCW tri-strip order.
    static GrPerspQuad MakeFromSkQuad(const SkPoint pts[4], const SkMatrix&);

    GrPerspQuad& operator=(const GrPerspQuad&) = default;

    SkPoint3 point(int i) const { return {fX[i], fY[i], fW[i]}; }

    SkRect bounds(GrQuadType type) const {
        Sk4f x = this->x4f();
        Sk4f y = this->y4f();
        if (type == GrQuadType::kPerspective) {
            Sk4f iw = this->iw4f();
            x *= iw;
            y *= iw;
        }

        return {x.min(), y.min(), x.max(), y.max()};
    }

    float x(int i) const { return fX[i]; }
    float y(int i) const { return fY[i]; }
    float w(int i) const { return fW[i]; }
    float iw(int i) const { return sk_ieee_float_divide(1.f, fW[i]); }

    Sk4f x4f() const { return Sk4f::Load(fX); }
    Sk4f y4f() const { return Sk4f::Load(fY); }
    Sk4f w4f() const { return Sk4f::Load(fW); }
    Sk4f iw4f() const { return this->w4f().invert(); }

    bool hasPerspective() const { return (w4f() != Sk4f(1.f)).anyTrue(); }

    // True if anti-aliasing affects this quad. Only valid when quadType == kRect_QuadType
    bool aaHasEffectOnRect() const;

private:
    template<typename T>
    friend class GrQuadBuffer;

    float fX[4];
    float fY[4];
    float fW[4];
};

/**
 * GrQuadBuffer is an optimized collection for storing GrPerspQuads, i.e. for use in a GrOp prior to
 * tessellation into a GPU buffer. It only supports adding quadrilaterals, appending another buffer,
 * and iterating over the elements. This allows it to compactly represent each GrPerspQuad with as
 * little data as possible:
 *  - Each quad with type kRect requires 4 floats
 *  - Each quad with type kRectilinear or kStandard requires 8 floats
 *  - Each quad with type kPerspective requires 12 floats
 *
 * Each "element" in the buffer is a primary quadrilateral, templated metadata, and an optional
 * secondary quadrilateral (e.g. for explicit local coordinates associated with primary quad).
 * The buffer tracks the most complex primary and secondary quad type in the buffer, but each
 * element preserves its own types so storing mixed type quadrilaterals does not require the largest
 * storage size per-quad.
 */
template<typename T>
class GrQuadBuffer {
public:
    explicit GrQuadBuffer(int reserve = 1)
            : fElementCount(0)
            , fPrimaryQuadType(GrQuadType::kRect)
            , fSecondaryQuadType(GrQuadType::kRect)
            , fData(reserve * (sizeof(T)/sizeof(int32_t) + 13))
            , fInsertIndex(0)
            , fDataLimit(reserve * (sizeof(T)/sizeof(int32_t) + 13)) {}


    /** Copies all elements of 'that' into this buffer. */
    void concat(const GrQuadBuffer<T>& that) {
        if (that.fPrimaryQuadType > fPrimaryQuadType) {
            fPrimaryQuadType = that.fPrimaryQuadType;
        }
        if (that.fSecondaryQuadType > fSecondaryQuadType) {
            fSecondaryQuadType = that.fSecondaryQuadType;
        }

        fElementCount += that.fElementCount;
        if (that.fInsertIndex + fInsertIndex > fDataLimit) {
            fDataLimit = 2 * (fDataLimit + that.fDataLimit);
            fData.realloc(fDataLimit);
        }
        memcpy(fData.get() + fInsertIndex, that.fData.get(), sizeof(int32_t) * that.fInsertIndex);
        fInsertIndex += that.fInsertIndex;
    }

    /** Appends a 'quad' of the given type (not verified) with 'metadata' and no secondary quad. */
    void push_back(const GrQuad& quad, GrQuadType type, T&& metadata) {
        this->appendHeader(type, GrQuadType::kRect, false, std::move(metadata));
        this->appendQuad(quad.fX, quad.fY, nullptr, type);
    }

    /** Appends a 'quad' of the given type (not verified) with 'metadata' and no secondary quad. */
    void push_back(const GrPerspQuad& quad, GrQuadType type, T&& metadata) {
        this->appendHeader(type, GrQuadType::kRect, false, std::move(metadata));
        this->appendQuad(quad.fX, quad.fY, quad.fW, type);
    }

    /** Appends a 'primary' and 'secondary' quad of the given types (not verified) with 'metadata'*/
    void push_back(const GrQuad& primary, GrQuadType primaryType, const GrQuad& secondary,
                   GrQuadType secondaryType, T&& metadata) {
        this->appendHeader(primaryType, secondaryType, true, std::move(metadata));
        this->appendQuad(primary.fX, primary.fY, nullptr, primaryType);
        this->appendQuad(secondary.fX, secondary.fY, nullptr, secondaryType);
    }

    /** Appends a 'primary' and 'secondary' quad of the given types (not verified) with 'metadata'*/
    void push_back(const GrPerspQuad& primary, GrQuadType primaryType, const GrPerspQuad& secondary,
                   GrQuadType secondaryType, T&& metadata) {
        this->appendHeader(primaryType, secondaryType, true, std::move(metadata));
        this->appendQuad(primary.fX, primary.fY, primary.fW, primaryType);
        this->appendQuad(secondary.fX, secondary.fY, secondary.fW, secondaryType);
    }

    /** The most general primary quad type stored in the buffer. */
    GrQuadType primaryQuadType() const { return fPrimaryQuadType; }

    /** The most general secondary quad type stored in the buffer. */
    GrQuadType secondaryQuadType() const { return fSecondaryQuadType; }

    /** The number of elements (primary quad, data, and optional secondary quad) in the buffer. */
    int count() const { return fElementCount; }

    /**
     * Iterator reports quads in the order they were added to its buffer. It is not allowed to
     * iterate over the buffer while adding quads.
     */
    template <bool is_const>
    class Iterator {
    public:
        // const-dependent types (non-const iterators allow metadata to be mutated)
        typedef typename std::conditional<is_const, const GrQuadBuffer*, GrQuadBuffer*>::type
                BufferPointer;
        typedef typename std::conditional<is_const, const T*, T*>::type MetadataPointer;

        /**
         * Advance the iterator to the next element. If this returns true, the quad coordinates
         * can be accessed by primaryQuad() and secondaryQuad() (assuming hasSecondaryQuad() returns
         * true). The metadata for the element can be read and modified via the metadata() pointer.
         */
        bool next() {
            SkASSERT(fBufferCount == fBuffer->count());
            if (fNextIndex >= fBuffer->fInsertIndex) {
                SkDEBUGCODE(fQuadsValid = false;)
                SkDEBUGCODE(fMetadataValid = false;)
                return false;
            }

            fNextIndex = fBuffer->reconstructElement(
                    fNextIndex, &fPrimary, &fSecondary, &fHasSecondary, &fMetadata);

            SkDEBUGCODE(fQuadsValid = true;)
            SkDEBUGCODE(fMetadataValid = true;)
            return true;
        }

        /** Advance the iterator to the next element, skipping the quad coordinate data, to only
         * iterate the metadata. Can call metadata() and hasSecondaryQuad() if this returns true.
         * Cannot ever call primaryQuad() or secondaryQuad() after this method. */
        bool nextMeta() {
            SkASSERT(fBufferCount == fBuffer->count());
            if (fNextIndex >= fBuffer->fInsertIndex) {
                SkDEBUGCODE(fQuadsValid = false;)
                SkDEBUGCODE(fMetadataValid = false;)
                return false;
            }

            fNextIndex = fBuffer->reconstructElement(
                    fNextIndex, nullptr, nullptr, &fHasSecondary, &fMetadata);

            SkDEBUGCODE(fQuadsValid = false;)
            SkDEBUGCODE(fMetadataValid = true;)
            return true;
        }

        /** Get primary quad coordinates from last successful call to next().*/
        const GrPerspQuad& primaryQuad() const {
            SkASSERT(fQuadsValid);
            return fPrimary;
        }

        /** Get secondary quad coordinates from last successful to next(), but requires
         * hasSecondaryQuad() to be true. */
        const GrPerspQuad& secondaryQuad() const {
            SkASSERT(fQuadsValid && fHasSecondary);
            return fSecondary;
        }

        /** True if the iterated element came with a secondary quadrilateral. */
        bool hasSecondaryQuad() const {
            SkASSERT(fMetadataValid);
            return fHasSecondary;
        }

        /** Get the pointer to the metadata of the last iterated element, from  next() or
         * nextMeta().*/
        MetadataPointer metadata() const {
            SkASSERT(fMetadataValid);
            return fMetadata;
        }

    private:
        friend class GrQuadBuffer<T>;

        explicit Iterator(BufferPointer buffer) : fBuffer(buffer), fNextIndex(0) {
            SkDEBUGCODE(fBufferCount = fBuffer->count();)
            SkDEBUGCODE(fQuadsValid = false;)
            SkDEBUGCODE(fMetadataValid = false;)
        }

        BufferPointer          fBuffer;
        int                    fNextIndex;
        // Hold on to the results of the last next() call so callers don't need to always declare
        // the same block of local variables.
        GrPerspQuad            fPrimary;
        GrPerspQuad            fSecondary;
        MetadataPointer        fMetadata;
        bool                   fHasSecondary;

        // Since buffers are only added to, and existing elements are immutable, if the buffer's
        // element count is greater than this, the iterator has been invalidated.
        SkDEBUGCODE(int fBufferCount;)
        SkDEBUGCODE(bool fQuadsValid;)
        SkDEBUGCODE(bool fMetadataValid;)
    };

    /** Get a new iterator over every current quad in the buffer. */
    Iterator<false> iter() { return Iterator<false>(this); }
    Iterator<true> iter() const { return Iterator<true>(this); }

private:
    struct ElementHeader {
        unsigned fPrimaryQuadType    : 2;
        unsigned fSecondaryQuadType  : 2;
        bool     fHasSecondaryQuad   : 1;
    };
    static constexpr int kMetadataSize = sizeof(T) / sizeof(int32_t);

    static_assert(kGrQuadTypeCount <= 4, "QuadType cannot fit into 2 bits");
    static_assert(sizeof(ElementHeader) == sizeof(int32_t), "ElementHeader must fit in int32_t");
    static_assert(alignof(T) == alignof(int32_t), "Metadata must align on int32_t");

    int             fElementCount;
    GrQuadType      fPrimaryQuadType;
    GrQuadType      fSecondaryQuadType;
    // SkTArray<int32_t, /* MEM_MOVE */ true> fData;
    SkAutoSTMalloc<24, int32_t> fData;
    int fInsertIndex;
    int fDataLimit;

    int elementSize(GrQuadType type) const {
        switch(type) {
            case GrQuadType::kRect: return 4;
            case GrQuadType::kPerspective: return 12;
            default: return 8;
        }
    }

    int elementSize(GrQuadType primaryType, GrQuadType secondaryType, bool hasSecondary) const {
        return 1 + kMetadataSize + this->elementSize(primaryType) + (hasSecondary ? this->elementSize(secondaryType) : 0);
    }

    int32_t* push_back(int num) {
        SkASSERT(fInsertIndex + num <= fDataLimit);
        int32_t* ptr = fData.get() + fInsertIndex;
        fInsertIndex += num;
        return ptr;
    }

    void appendHeader(GrQuadType primaryType, GrQuadType secondaryType, bool hasSecondary,
                      T&& metadata) {
        int requiredSize = this->elementSize(primaryType, secondaryType, hasSecondary);
        if (fInsertIndex + requiredSize > fDataLimit) {
            fDataLimit = 2 * (fInsertIndex + requiredSize);
            fData.realloc(fDataLimit);
        }

        ElementHeader* header = reinterpret_cast<ElementHeader*>(this->push_back(1));
        header->fPrimaryQuadType = static_cast<unsigned>(primaryType);
        header->fSecondaryQuadType = static_cast<unsigned>(secondaryType);
        header->fHasSecondaryQuad = hasSecondary;

        T* metaStorage = reinterpret_cast<T*>(this->push_back(kMetadataSize));
        *metaStorage = std::move(metadata);

        fElementCount++;
        if (primaryType > fPrimaryQuadType) {
            fPrimaryQuadType = primaryType;
        }
        if (secondaryType > fSecondaryQuadType) {
            fSecondaryQuadType = secondaryType;
        }
    }

    void append4f(const float data[4]) {
        int32_t* storage = this->push_back(4);
        memcpy(storage, data, 4 * sizeof(float));
    }

    void appendQuad(const float xs[], const float ys[], const float ws[], GrQuadType type) {
        SkASSERT(xs && ys);
        if (type == GrQuadType::kRect) {
            // Select LTRB from the xs and ys
            float ltrb[4] = {xs[0], ys[0], xs[2], ys[1]};
            this->append4f(ltrb);
        } else {
            // Always push back the xs and the ys
            this->append4f(xs);
            this->append4f(ys);
            if (type == GrQuadType::kPerspective) {
                SkASSERT(ws);
                this->append4f(ws);
            }
        }
    }

    int reconstructElement(int index, GrPerspQuad* primary, GrPerspQuad* secondary,
                           bool* hasSecondary, T** metadata) {
        // Always expects metadata and hasSecondary, quad pointers are optional to support
        // metadata-only iteration
        SkASSERT(metadata && hasSecondary);

        SkASSERT(index < fDataLimit);
        const ElementHeader& header = reinterpret_cast<const ElementHeader&>(fData[index++]);

        SkASSERT(index + kMetadataSize <= fDataLimit);
        // If we have an output pointer for the metadata, update it to the element's data
        *metadata = reinterpret_cast<T*>(fData.get() + index);
        index += kMetadataSize;

        index = reconstructQuads(index, header, primary, secondary, hasSecondary);
        return index;
    }

    int reconstructElement(int index, GrPerspQuad* primary, GrPerspQuad* secondary,
                           bool* hasSecondary, const T** metadata) const {
        // Always expects metadata and hasSecondary, quad pointers are optional to support
        // metadata-only iteration
        SkASSERT(metadata && hasSecondary);
        SkASSERT(index < fDataLimit);
        const ElementHeader& header = reinterpret_cast<const ElementHeader&>(fData[index++]);

        SkASSERT(index + kMetadataSize <= fDataLimit);
        // If we have an output pointer for the metadata, update it to the element's data
        *metadata = reinterpret_cast<const T*>(fData.get() + index);
        index += kMetadataSize;

        return reconstructQuads(index, header, primary, secondary, hasSecondary);
    }

    int reconstructQuads(int index, const ElementHeader& header, GrPerspQuad* primary,
                         GrPerspQuad* secondary, bool* hasSecondary) const {
        // Read primary quad (always provided)
        index = this->reconstructQuad(
                index, static_cast<GrQuadType>(header.fPrimaryQuadType), primary);

        if (header.fHasSecondaryQuad) {
            // Read secondary quad
            index = this->reconstructQuad(
                    index, static_cast<GrQuadType>(header.fSecondaryQuadType), secondary);
            *hasSecondary = true;
        } else {
            *hasSecondary = false;
        }

        return index;
    }

    int reconstructQuad(int index, GrQuadType type, GrPerspQuad* quad) const {
        if (type == GrQuadType::kRect) {
            SkASSERT(index + 4 <= fDataLimit);
            // Rectangle stored as LTRB so inflate xs: LLRR, ys: TBTB, ws: 1111
            if (quad) {
                const float* ltrb = reinterpret_cast<const float*>(fData.get() + index);
                quad->fX[0] = ltrb[0];
                quad->fX[1] = ltrb[0];
                quad->fX[2] = ltrb[2];
                quad->fX[3] = ltrb[2];
                quad->fY[0] = ltrb[1];
                quad->fY[1] = ltrb[3];
                quad->fY[2] = ltrb[1];
                quad->fY[3] = ltrb[3];
                quad->fW[0] = 1.f;
                quad->fW[1] = 1.f;
                quad->fW[2] = 1.f;
                quad->fW[3] = 1.f;
            }
            index += 4;
        } else {
            // Read back 4 xs and 4 ys
            SkASSERT(index + 8 <= fDataLimit);
            if (quad) {
                const float* xs = reinterpret_cast<const float*>(fData.get() + index);
                const float* ys = reinterpret_cast<const float*>(fData.get() + index + 4);
                memcpy(quad->fX, xs, 4 * sizeof(float));
                memcpy(quad->fY, ys, 4 * sizeof(float));
            }
            index += 8;

            if (type == GrQuadType::kPerspective) {
                // Also read back 4 ws
                SkASSERT(index + 4 <= fDataLimit);
                if (quad) {
                    const float* ws = reinterpret_cast<const float*>(fData.get() + index);
                    memcpy(quad->fW, ws, 4 * sizeof(float));
                }
                index += 4;
            } else if (quad) {
                // Store 1.s in the ws
                quad->fW[0] = 1.f;
                quad->fW[1] = 1.f;
                quad->fW[2] = 1.f;
                quad->fW[3] = 1.f;
            }
        }
        return index;
    }
};

#endif
