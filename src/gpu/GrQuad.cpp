/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrQuad.h"

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
