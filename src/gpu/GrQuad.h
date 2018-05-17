/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrQuad_DEFINED
#define GrQuad_DEFINED

#include "SkPoint.h"
#include "SkMatrix.h"
#include "SkMatrixPriv.h"

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

    explicit GrQuad(const SkPoint pts[4])
            : fX{pts[0].fX, pts[1].fX, pts[2].fX, pts[3].fX}
            , fY{pts[0].fY, pts[1].fY, pts[2].fY, pts[3].fY} {}

    GrQuad& operator=(const GrQuad& that) = default;

    void setFromMappedRect(const SkRect& rect, const SkMatrix& mx) {
        SkMatrix::TypeMask tm = mx.getType();
        if (tm <= (SkMatrix::kScale_Mask | SkMatrix::kTranslate_Mask)) {
            auto r = Sk4f::Load(&rect);
            const Sk4f t(mx.getTranslateX(), mx.getTranslateY(), mx.getTranslateX(),
                         mx.getTranslateY());
            if (tm <= SkMatrix::kTranslate_Mask) {
                r += t;
            } else {
                const Sk4f s(mx.getScaleX(), mx.getScaleY(), mx.getScaleX(), mx.getScaleY());
                r = r * s + t;
            }
            SkNx_shuffle<0, 0, 2, 2>(r).store(fX);
            SkNx_shuffle<1, 3, 1, 3>(r).store(fY);
        } else if (!(tm & SkMatrix::kPerspective_Mask)) {
            Sk4f rx(rect.fLeft, rect.fLeft, rect.fRight, rect.fRight);
            Sk4f ry(rect.fTop, rect.fBottom, rect.fTop, rect.fBottom);
            Sk4f sx(mx.getScaleX());
            Sk4f kx(mx.getSkewX());
            Sk4f tx(mx.getTranslateX());
            Sk4f ky(mx.getSkewY());
            Sk4f sy(mx.getScaleY());
            Sk4f ty(mx.getTranslateY());
            (sx * rx + kx * ry + tx).store(fX);
            (ky * rx + sy * ry + ty).store(fY);
        } else {
            SkPoint pts[4];
            SkPointPriv::SetRectTriStrip(pts, rect.fLeft, rect.fTop, rect.fRight, rect.fBottom,
                                         sizeof(SkPoint));
            mx.mapPoints(pts, pts, 4);
            *this = GrQuad(pts);
        }
    }

    SkPoint point(int i) const { return {fX[i], fY[i]}; }

    SkRect bounds() const {
        auto x = this->x4f(), y = this->y4f();
        return {x.min(), y.min(), x.max(), y.max()};
    }

    float x(int i) const { return fX[i]; }
    float y(int i) const { return fY[i]; }

    Sk4f x4f() const { return Sk4f::Load(fX); }
    Sk4f y4f() const { return Sk4f::Load(fY); }

private:
    float fX[4];
    float fY[4];
};

#endif
