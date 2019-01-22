/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrQuad.h"

#include "GrTypesPriv.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Functions for identifying the quad type from its coordinates, which are kept debug-only since
// production code should rely on the matrix to derive the quad type more efficiently. These are
// useful in asserts that the quad type is as expected.
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef SK_DEBUG
// Allow some tolerance from floating point matrix transformations, but SkScalarNearlyEqual doesn't
// support comparing infinity, and coords_form_rect should return true for infinite edges
#define NEARLY_EQUAL(f1, f2) (f1 == f2 || SkScalarNearlyEqual(f1, f2, 1e-5f))
// Similarly, support infinite rectangles by looking at the sign of infinities
static bool dot_nearly_zero(const SkVector& e1, const SkVector& e2) {
    static constexpr auto dot = SkPoint::DotProduct;
    static constexpr auto sign = SkScalarSignAsScalar;

    SkScalar dotValue = dot(e1, e2);
    if (SkScalarIsNaN(dotValue)) {
        // Form vectors from the signs of infinities, and check their dot product
        dotValue = dot({sign(e1.fX), sign(e1.fY)}, {sign(e2.fX), sign(e2.fY)});
    }

    return SkScalarNearlyZero(dotValue, 1e-3f);
}

// This is not the most performance critical function; code using GrQuad should rely on the faster
// quad type from matrix path, so this will only be called as part of SkASSERT.
static bool coords_form_rect(const float xs[4], const float ys[4]) {
    return (NEARLY_EQUAL(xs[0], xs[1]) && NEARLY_EQUAL(xs[2], xs[3]) &&
            NEARLY_EQUAL(ys[0], ys[2]) && NEARLY_EQUAL(ys[1], ys[3])) ||
           (NEARLY_EQUAL(xs[0], xs[2]) && NEARLY_EQUAL(xs[1], xs[3]) &&
            NEARLY_EQUAL(ys[0], ys[1]) && NEARLY_EQUAL(ys[2], ys[3]));
}

static bool coords_rectilinear(const float xs[4], const float ys[4]) {
    SkVector e0{xs[1] - xs[0], ys[1] - ys[0]}; // connects to e1 and e2(repeat)
    SkVector e1{xs[3] - xs[1], ys[3] - ys[1]}; // connects to e0(repeat) and e3
    SkVector e2{xs[0] - xs[2], ys[0] - ys[2]}; // connects to e0 and e3(repeat)
    SkVector e3{xs[2] - xs[3], ys[2] - ys[3]}; // connects to e1(repeat) and e2

    e0.normalize();
    e1.normalize();
    e2.normalize();
    e3.normalize();

    return dot_nearly_zero(e0, e1) && dot_nearly_zero(e1, e3) &&
           dot_nearly_zero(e2, e0) && dot_nearly_zero(e3, e2);
}

GrQuadType GrQuad::quadType() const {
    // Since GrQuad applies any perspective information at construction time, there's only two
    // types to choose from.
    if (coords_form_rect(fX, fY)) {
        return GrQuadType::kRect;
    } else if (coords_rectilinear(fX, fY)) {
        return GrQuadType::kRectilinear;
    } else {
        return GrQuadType::kStandard;
    }
}

GrQuadType GrPerspQuad::quadType() const {
    if (this->hasPerspective()) {
        return GrQuadType::kPerspective;
    } else {
        // Rect or standard quad, can ignore w since they are all ones
        if (coords_form_rect(fX, fY)) {
            return GrQuadType::kRect;
        } else if (coords_rectilinear(fX, fY)) {
            return GrQuadType::kRectilinear;
        } else {
            return GrQuadType::kStandard;
        }
    }
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////

static bool aa_affects_rect(float ql, float qt, float qr, float qb) {
    return !SkScalarIsInt(ql) || !SkScalarIsInt(qr) || !SkScalarIsInt(qt) || !SkScalarIsInt(qb);
}

template <typename Q>
void GrResolveAATypeForQuad(GrAAType requestedAAType, GrQuadAAFlags requestedEdgeFlags,
                            const Q& quad, GrQuadType knownType,
                            GrAAType* outAAType, GrQuadAAFlags* outEdgeFlags) {
    // Most cases will keep the requested types unchanged
    *outAAType = requestedAAType;
    *outEdgeFlags = requestedEdgeFlags;

    switch (requestedAAType) {
        // When aa type is coverage, disable AA if the edge configuration doesn't actually need it
        case GrAAType::kCoverage:
            if (requestedEdgeFlags == GrQuadAAFlags::kNone) {
                // Turn off anti-aliasing
                *outAAType = GrAAType::kNone;
            } else {
                // For coverage AA, if the quad is a rect and it lines up with pixel boundaries
                // then overall aa and per-edge aa can be completely disabled
                if (knownType == GrQuadType::kRect && !quad.aaHasEffectOnRect()) {
                    *outAAType = GrAAType::kNone;
                    *outEdgeFlags = GrQuadAAFlags::kNone;
                }
            }
            break;
        // For no or msaa anti aliasing, override the edge flags since edge flags only make sense
        // when coverage aa is being used.
        case GrAAType::kNone:
            *outEdgeFlags = GrQuadAAFlags::kNone;
            break;
        case GrAAType::kMSAA:
            *outEdgeFlags = GrQuadAAFlags::kAll;
            break;
        case GrAAType::kMixedSamples:
            SK_ABORT("Should not use mixed sample AA with edge AA flags");
            break;
    }
};

// Instantiate GrResolve... for GrQuad and GrPerspQuad
template void GrResolveAATypeForQuad(GrAAType, GrQuadAAFlags, const GrQuad&, GrQuadType,
                                     GrAAType*, GrQuadAAFlags*);
template void GrResolveAATypeForQuad(GrAAType, GrQuadAAFlags, const GrPerspQuad&, GrQuadType,
                                     GrAAType*, GrQuadAAFlags*);

GrQuadType GrQuadTypeForTransformedRect(const SkMatrix& matrix) {
    if (matrix.rectStaysRect()) {
        return GrQuadType::kRect;
    } else if (matrix.preservesRightAngles()) {
        return GrQuadType::kRectilinear;
    } else if (matrix.hasPerspective()) {
        return GrQuadType::kPerspective;
    } else {
        return GrQuadType::kStandard;
    }
}

GrQuad::GrQuad(const SkRect& rect, const SkMatrix& m) {
    SkMatrix::TypeMask tm = m.getType();
    if (tm <= (SkMatrix::kScale_Mask | SkMatrix::kTranslate_Mask)) {
        auto r = Sk4f::Load(&rect);
        const Sk4f t(m.getTranslateX(), m.getTranslateY(), m.getTranslateX(), m.getTranslateY());
        if (tm <= SkMatrix::kTranslate_Mask) {
            r += t;
        } else {
            const Sk4f s(m.getScaleX(), m.getScaleY(), m.getScaleX(), m.getScaleY());
            r = r * s + t;
        }
        SkNx_shuffle<0, 0, 2, 2>(r).store(fX);
        SkNx_shuffle<1, 3, 1, 3>(r).store(fY);
    } else {
        Sk4f rx(rect.fLeft, rect.fLeft, rect.fRight, rect.fRight);
        Sk4f ry(rect.fTop, rect.fBottom, rect.fTop, rect.fBottom);
        Sk4f sx(m.getScaleX());
        Sk4f kx(m.getSkewX());
        Sk4f tx(m.getTranslateX());
        Sk4f ky(m.getSkewY());
        Sk4f sy(m.getScaleY());
        Sk4f ty(m.getTranslateY());
        auto x = SkNx_fma(sx, rx, SkNx_fma(kx, ry, tx));
        auto y = SkNx_fma(ky, rx, SkNx_fma(sy, ry, ty));
        if (m.hasPerspective()) {
            Sk4f w0(m.getPerspX());
            Sk4f w1(m.getPerspY());
            Sk4f w2(m.get(SkMatrix::kMPersp2));
            auto iw = SkNx_fma(w0, rx, SkNx_fma(w1, ry, w2)).invert();
            x *= iw;
            y *= iw;
        }
        x.store(fX);
        y.store(fY);
    }
}

bool GrQuad::aaHasEffectOnRect() const {
    SkASSERT(this->quadType() == GrQuadType::kRect);
    return aa_affects_rect(fX[0], fY[0], fX[3], fY[3]);
}

GrPerspQuad::GrPerspQuad(const SkRect& rect, const SkMatrix& m) {
    SkMatrix::TypeMask tm = m.getType();
    if (tm <= (SkMatrix::kScale_Mask | SkMatrix::kTranslate_Mask)) {
        auto r = Sk4f::Load(&rect);
        const Sk4f t(m.getTranslateX(), m.getTranslateY(), m.getTranslateX(), m.getTranslateY());
        if (tm <= SkMatrix::kTranslate_Mask) {
            r += t;
        } else {
            const Sk4f s(m.getScaleX(), m.getScaleY(), m.getScaleX(), m.getScaleY());
            r = r * s + t;
        }
        SkNx_shuffle<0, 0, 2, 2>(r).store(fX);
        SkNx_shuffle<1, 3, 1, 3>(r).store(fY);
        fW[0] = fW[1] = fW[2] = fW[3] = 1.f;
    } else {
        Sk4f rx(rect.fLeft, rect.fLeft, rect.fRight, rect.fRight);
        Sk4f ry(rect.fTop, rect.fBottom, rect.fTop, rect.fBottom);
        Sk4f sx(m.getScaleX());
        Sk4f kx(m.getSkewX());
        Sk4f tx(m.getTranslateX());
        Sk4f ky(m.getSkewY());
        Sk4f sy(m.getScaleY());
        Sk4f ty(m.getTranslateY());
        SkNx_fma(sx, rx, SkNx_fma(kx, ry, tx)).store(fX);
        SkNx_fma(ky, rx, SkNx_fma(sy, ry, ty)).store(fY);
        if (m.hasPerspective()) {
            Sk4f w0(m.getPerspX());
            Sk4f w1(m.getPerspY());
            Sk4f w2(m.get(SkMatrix::kMPersp2));
            auto w = SkNx_fma(w0, rx, SkNx_fma(w1, ry, w2));
            w.store(fW);
        } else {
            fW[0] = fW[1] = fW[2] = fW[3] = 1.f;
        }
    }
}

// Private constructor used by GrQuadList to quickly fill in a quad's values from the channel arrays
GrPerspQuad::GrPerspQuad(const float* xs, const float* ys, const float* ws) {
    memcpy(fX, xs, 4 * sizeof(float));
    memcpy(fY, ys, 4 * sizeof(float));
    memcpy(fW, ws, 4 * sizeof(float));
}

bool GrPerspQuad::aaHasEffectOnRect() const {
    SkASSERT(this->quadType() == GrQuadType::kRect);
    // If rect, ws must all be 1s so no need to divide
    return aa_affects_rect(fX[0], fY[0], fX[3], fY[3]);
}
