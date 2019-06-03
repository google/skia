/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrQuad_DEFINED
#define GrQuad_DEFINED

#include "include/core/SkMatrix.h"
#include "include/core/SkPoint.h"
#include "include/core/SkPoint3.h"
#include "include/private/SkVx.h"

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

// Resolve disagreements between the overall requested AA type and the per-edge quad AA flags.
// knownQuadType must have come from GrQuadTypeForTransformedRect with the matrix that created the
// provided quad. Both outAAType and outEdgeFlags will be updated.
template <typename Q>
void GrResolveAATypeForQuad(GrAAType requestedAAType, GrQuadAAFlags requestedEdgeFlags,
                            const Q& quad, GrAAType* outAAtype, GrQuadAAFlags* outEdgeFlags);

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
            , fY{rect.fTop, rect.fBottom, rect.fTop, rect.fBottom}
            , fType(GrQuadType::kRect) {}

    GrQuad(const skvx::Vec<4, float>& xs, const skvx::Vec<4, float>& ys, GrQuadType type)
            : fType(type) {
        xs.store(fX);
        ys.store(fY);
    }

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
        return {min(x), min(y), max(x), max(y)};
    }

    float x(int i) const { return fX[i]; }
    float y(int i) const { return fY[i]; }

    skvx::Vec<4, float> x4f() const { return skvx::Vec<4, float>::Load(fX); }
    skvx::Vec<4, float> y4f() const { return skvx::Vec<4, float>::Load(fY); }

    GrQuadType quadType() const { return fType; }

    // True if anti-aliasing affects this quad. Only valid when quadType == kRect_QuadType
    bool aaHasEffectOnRect() const;

private:
    template<typename T>
    friend class GrQuadListBase;

    float fX[4];
    float fY[4];

    GrQuadType fType;
};

class GrPerspQuad {
public:
    GrPerspQuad() = default;

    explicit GrPerspQuad(const SkRect& rect)
            : fX{rect.fLeft, rect.fLeft, rect.fRight, rect.fRight}
            , fY{rect.fTop, rect.fBottom, rect.fTop, rect.fBottom}
            , fW{1.f, 1.f, 1.f, 1.f}
            , fType(GrQuadType::kRect) {}

    GrPerspQuad(const skvx::Vec<4, float>& xs, const skvx::Vec<4, float>& ys,
                GrQuadType type)
            : fType(type) {
        SkASSERT(type != GrQuadType::kPerspective);
        xs.store(fX);
        ys.store(fY);
        fW[0] = fW[1] = fW[2] = fW[3] = 1.f;
    }

    GrPerspQuad(const skvx::Vec<4, float>& xs, const skvx::Vec<4, float>& ys,
                const skvx::Vec<4, float>& ws, GrQuadType type)
            : fType(type) {
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

    SkRect bounds() const {
        auto x = this->x4f();
        auto y = this->y4f();
        if (fType == GrQuadType::kPerspective) {
            auto iw = this->iw4f();
            x *= iw;
            y *= iw;
        }

        return {min(x), min(y), max(x), max(y)};
    }

    float x(int i) const { return fX[i]; }
    float y(int i) const { return fY[i]; }
    float w(int i) const { return fW[i]; }
    float iw(int i) const { return sk_ieee_float_divide(1.f, fW[i]); }

    skvx::Vec<4, float> x4f() const { return skvx::Vec<4, float>::Load(fX); }
    skvx::Vec<4, float> y4f() const { return skvx::Vec<4, float>::Load(fY); }
    skvx::Vec<4, float> w4f() const { return skvx::Vec<4, float>::Load(fW); }
    skvx::Vec<4, float> iw4f() const { return 1.f / this->w4f(); }

    GrQuadType quadType() const { return fType; }

    bool hasPerspective() const { return fType == GrQuadType::kPerspective; }

    // True if anti-aliasing affects this quad. Only valid when quadType == kRect_QuadType
    bool aaHasEffectOnRect() const;

private:
    template<typename T>
    friend class GrQuadListBase;

    // Copy 4 values from each of the arrays into the quad's components
    GrPerspQuad(const float xs[4], const float ys[4], const float ws[4], GrQuadType type);

    float fX[4];
    float fY[4];
    float fW[4];

    GrQuadType fType;
};

#endif
