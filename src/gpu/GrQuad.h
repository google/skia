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
        auto x = this->x4f() * this->iw4f();
        auto y = this->y4f() * this->iw4f();
        return {x.min(), y.min(), x.max(), y.max()};
    }

    float x(int i) const { return fX[i]; }
    float y(int i) const { return fY[i]; }
    float w(int i) const { return fW[i]; }
    float iw(int i) const { return fIW[i]; }

    Sk4f x4f() const { return Sk4f::Load(fX); }
    Sk4f y4f() const { return Sk4f::Load(fY); }
    Sk4f w4f() const { return Sk4f::Load(fW); }
    Sk4f iw4f() const { return Sk4f::Load(fIW); }

    bool hasPerspective() const { return (w4f() != Sk4f(1.f)).anyTrue(); }

    // True if anti-aliasing affects this quad. Requires quadType() == kRect_QuadType
    bool aaHasEffectOnRect() const;

#ifdef SK_DEBUG
    GrQuadType quadType() const;
#endif

private:
    float fX[4];
    float fY[4];
    float fW[4];
    float fIW[4];  // 1/w
};

#endif
