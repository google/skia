/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrQuad.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Functions for identifying the quad type from its coordinates, which are kept debug-only since
// production code should rely on the matrix to derive the quad type more efficiently. These are
// useful in asserts that the quad type is as expected.
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef SK_DEBUG
// Allow some tolerance from floating point matrix transformations, but SkScalarNearlyEqual doesn't
// support comparing infinity, and coords_form_rect should return true for infinite edges
#define NEARLY_EQUAL(f1, f2) (f1 == f2 || SkScalarNearlyEqual(f1, f2, 1e-5f))

// This is not the most performance critical function; code using GrQuad should rely on the faster
// quad type from matrix path, so this will only be called as part of SkASSERT.
static bool coords_form_rect(const float xs[4], const float ys[4]) {
    return (NEARLY_EQUAL(xs[0], xs[1]) && NEARLY_EQUAL(xs[2], xs[3]) &&
            NEARLY_EQUAL(ys[0], ys[2]) && NEARLY_EQUAL(ys[1], ys[3])) ||
           (NEARLY_EQUAL(xs[0], xs[2]) && NEARLY_EQUAL(xs[1], xs[3]) &&
            NEARLY_EQUAL(ys[0], ys[1]) && NEARLY_EQUAL(ys[2], ys[3]));
}

GrQuadType GrQuad::quadType() const {
    // Since GrQuad applies any perspective information at construction time, there's only two
    // types to choose from.
    return coords_form_rect(fX, fY) ? GrQuadType::kRect_QuadType : GrQuadType::kStandard_QuadType;
}

GrQuadType GrPerspQuad::quadType() const {
    if (this->hasPerspective()) {
        return GrQuadType::kPerspective_QuadType;
    } else {
        // Rect or standard quad, can ignore w since they are all ones
        return coords_form_rect(fX, fY) ? GrQuadType::kRect_QuadType
                                        : GrQuadType::kStandard_QuadType;
    }
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////

static bool aa_affects_rect(float ql, float qt, float qr, float qb) {
    return !SkScalarIsInt(ql) || !SkScalarIsInt(qr) || !SkScalarIsInt(qt) || !SkScalarIsInt(qb);
}

GrQuadType GrQuadTypeForTransformedRect(const SkMatrix& matrix) {
    if (matrix.rectStaysRect()) {
        return GrQuadType::kRect_QuadType;
    } else if (matrix.hasPerspective()) {
        return GrQuadType::kPerspective_QuadType;
    } else {
        return GrQuadType::kStandard_QuadType;
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
    SkASSERT(this->quadType() == GrQuadType::kRect_QuadType);
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
        fIW[0] = fIW[1] = fIW[2] = fIW[3] = 1.f;
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
            w.invert().store(fIW);
        } else {
            fW[0] = fW[1] = fW[2] = fW[3] = 1.f;
            fIW[0] = fIW[1] = fIW[2] = fIW[3] = 1.f;
        }
    }
}

bool GrPerspQuad::aaHasEffectOnRect() const {
    SkASSERT(this->quadType() == GrQuadType::kRect_QuadType);
    // If rect, ws must all be 1s so no need to divide
    return aa_affects_rect(fX[0], fY[0], fX[3], fY[3]);
}
