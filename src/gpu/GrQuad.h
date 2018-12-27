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

// If an SkRect is transformed by this matrix, what class of quad is required to represent it. Since
// quadType() is only provided on Gr[Persp]Quad in debug builds, production code should use this
// to efficiently determine quad types.
GrQuadType GrQuadTypeForTransformedRect(const SkMatrix& matrix);

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

    /** Sets the quad to the rect as transformed by the matrix. */
    GrQuad(const SkRect&, const SkMatrix&);

    explicit GrQuad(const SkPoint pts[4])
            : fX{pts[0].fX, pts[1].fX, pts[2].fX, pts[3].fX}
            , fY{pts[0].fY, pts[1].fY, pts[2].fY, pts[3].fY} {}

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

    // True if anti-aliasing affects this quad. Requires quadType() == kRect_QuadType
    bool aaHasEffectOnRect() const;

#ifdef SK_DEBUG
    GrQuadType quadType() const;
#endif

private:
    template<int N>
    friend class GrQuadListBase;

    float fX[4];
    float fY[4];
};

class GrPerspQuad {
public:
    GrPerspQuad() = default;

    GrPerspQuad(const SkRect&, const SkMatrix&);

    GrPerspQuad& operator=(const GrPerspQuad&) = default;

    SkPoint3 point(int i) const { return {fX[i], fY[i], fW[i]}; }

    SkRect bounds() const {
        auto iw = this->iw4f();
        auto x = this->x4f() * iw;
        auto y = this->y4f() * iw;
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

    // True if anti-aliasing affects this quad. Requires quadType() == kRect_QuadType
    bool aaHasEffectOnRect() const;

#ifdef SK_DEBUG
    GrQuadType quadType() const;
#endif

private:
    template<int N>
    friend class GrQuadListBase;

    // Copy 4 values from each of the arrays into the quad's components
    GrPerspQuad(const float* xs, const float* ys, const float* ws);

    float fX[4];
    float fY[4];
    float fW[4];
};

// A dynamic list of (possibly) perspective quads that tracks the most general quad type of all
// added quads. It avoids storing the 3rd component if the quad type never becomes perspective.
// Use GrQuadList subclass when only storing quads. Use GrTQuadList subclass when storing quads
// and per-quad templated metadata (such as color or domain).
template<int META_BYTES>
class GrQuadListBase {
public:

    int count() const { return fXYs.count(); }

    GrQuadType quadType() const { return fType; }

    void reserve(int count, GrQuadType forType) {
        fXYs.reserve(count);
        if (forType == GrQuadType::kPerspective || fType == GrQuadType::kPerspective) {
            fWs.reserve(4 * count);
        }
    }

    // Append the quad to the list, possibly upgrading the list's type. The index of the added quad
    // is returned. Any metadata associated with the quad is undefined.
    int push_back(const GrQuad& quad, GrQuadType type) {
        // Make sure the incoming quad is not meant to be perspective (but it's okay if the list's
        // type is perspective since a 2D quad can always be upgraded).
        SkASSERT(type != GrQuadType::kPerspective);
        SkASSERT(quad.quadType() <= type);

        // Linking fails if kAllOnes is made a static member of GrQuadListBase so that it could
        // be shared with the same definition in operator[].
        static constexpr float kAllOnes[4] = {1.f, 1.f, 1.f, 1.f};
        return this->pushBackRaw(quad.fX, kAllOnes, type);
    }

    int push_back(const GrPerspQuad& quad, GrQuadType type) {
        SkASSERT(quad.quadType() <= type);
        // The Ws will automatically be 1s if the quad isn't truly perspective
        return this->pushBackRaw(quad.fX, quad.fW, type);
    }

    GrPerspQuad operator[] (int i) const {
        SkASSERT(i < this->count());
        SkASSERT(i >= 0);

        const XYAndMetadata& item = fXYs[i];
        if (fType == GrQuadType::kPerspective) {
            // Read the explicit ws
            return GrPerspQuad(item.fX, item.fY, fWs.begin() + 4 * i);
        } else {
            // Ws are implicitly 1s.
            static constexpr float kAllOnes[4] = {1.f, 1.f, 1.f, 1.f};
            return GrPerspQuad(item.fX, item.fY, kAllOnes);
        }
    }

protected:
    GrQuadListBase() : fType(GrQuadType::kRect) {}

    void concatRaw(const GrQuadListBase<META_BYTES>& that) {
        this->upgradeType(that.fType);
        fXYs.push_back_n(that.fXYs.count(), that.fXYs.begin());
        if (fType == GrQuadType::kPerspective) {
            if (that.fType == GrQuadType::kPerspective) {
                // Copy the other's ws into the end of this list's data
                fWs.push_back_n(that.fWs.count(), that.fWs.begin());
            } else {
                // This list stores ws but the appended list had implicit 1s, so add explicit 1s to
                // fill out the total list
                fWs.push_back_n(4 * that.count(), 1.f);
            }
        }
    }

    // Returns pointer to the beginning of the metadata storage for the given quad. It is safe to
    // write META_BYTES into the returned pointer.
    const void* metadataRaw(int i) const {
        SkASSERT(i < this->count());
        SkASSERT(i >= 0);

        const XYAndMetadata& item = fXYs[i];
        return static_cast<const void*>(item.fMetadata);
    }

    void* metadataRaw(int i) {
        SkASSERT(i < this->count());
        SkASSERT(i >= 0);

        XYAndMetadata& item = fXYs[i];
        return static_cast<void*>(item.fMetadata);
    }

private:

    void upgradeType(GrQuadType type) {
        // Possibly upgrade the overall type tracked by the list
        if (type > fType) {
            fType = type;
            if (type == GrQuadType::kPerspective) {
                // All existing quads were 2D, so the ws array just needs to be filled with 1s
                fWs.push_back_n(4 * this->count(), 1.f);
            }
        }
    }

    int pushBackRaw(const float* xys, const float* ws, GrQuadType type) {
        this->upgradeType(type);
        XYAndMetadata& item = fXYs.push_back();
        // Copy 4 xs and 4 ys from the quad into the item struct, this relies on GrPerspQuad and
        // XYAndMetadata both having fX[4]; fY[4]; in the same order.
        memcpy(&item.fX, xys, 8 * sizeof(float));
        // Leave any metadata on item untouched
        if (fType == GrQuadType::kPerspective) {
            fWs.push_back_n(4, ws);
        }

        return this->count() - 1;
    }

    // Interleave xs, ys, and per-quad metadata so that all data for a single quad is together
    // (barring ws, which can be dropped entirely if the quad type allows it).
    struct XYAndMetadata {
        float fX[4];
        float fY[4];
        char fMetadata[META_BYTES];
    };
    static_assert(sizeof(XYAndMetadata) == 8 * sizeof(float) + META_BYTES, "Unexpected size");

    SkSTArray<1, XYAndMetadata, true> fXYs;
    // The w channel is kept separate so that it can remain empty when only dealing with 2D quads.
    SkTArray<float, true> fWs;

    GrQuadType fType;
};

// This list only stores the quad data itself.
class GrQuadList : public GrQuadListBase<0> {
private:
    typedef GrQuadListBase<0> INHERITED;

public:
    GrQuadList() : INHERITED() {}

    void concat(const GrQuadList& that) {
        this->concatRaw(that);
    }

    // Don't expose the metadata functions since this list only holds the quads themselves
};

// This variant of the list allows simple metadata to be stored per quad as well, such as color
// or texture domain.
template<typename T>
class GrTQuadList : public GrQuadListBase<sizeof(T)> {
private:
    typedef GrQuadListBase<sizeof(T)> INHERITED;

public:
    GrTQuadList() : INHERITED() {}

    void concat(const GrTQuadList<T>& that) {
        this->concatRaw(that);
    }

    // And provide access to the metadata per quad
    const T& metadata(int i) const {
        const T* ptr = static_cast<const T*>(this->metadataRaw(i));
        return *ptr;
    }

    T& metadata(int i) {
        T* ptr = static_cast<T*>(this->metadataRaw(i));
        return *ptr;
    }
};

#endif
