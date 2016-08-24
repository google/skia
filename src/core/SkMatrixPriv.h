/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMatrixPriv_DEFINE
#define SkMatrixPriv_DEFINE

#include "SkMatrix.h"
#include "SkNx.h"

class SkMatrixPriv {
public:
    /**
     *  Attempt to map the rect through the inverse of the matrix. If it is not invertible,
     *  then this returns false and dst is unchanged.
     */
    static bool SK_WARN_UNUSED_RESULT InverseMapRect(const SkMatrix& mx,
                                                     SkRect* dst, const SkRect& src) {
        if (mx.getType() <= SkMatrix::kTranslate_Mask) {
            SkScalar tx = mx.getTranslateX();
            SkScalar ty = mx.getTranslateY();
            Sk4f trans(tx, ty, tx, ty);
            (Sk4f::Load(&src.fLeft) - trans).store(&dst->fLeft);
            return true;
        }
        // Insert other special-cases here (e.g. scale+translate)
        
        // general case
        SkMatrix inverse;
        if (mx.invert(&inverse)) {
            inverse.mapRect(dst, src);
            return true;
        }
        return false;
    }

    static void MapPointsWithStride(const SkMatrix& mx, SkPoint pts[], size_t stride, int count) {
        SkASSERT(stride >= sizeof(SkPoint));
        SkASSERT(0 == stride % sizeof(SkScalar));

        SkMatrix::TypeMask tm = mx.getType();
        
        if (SkMatrix::kIdentity_Mask == tm) {
            return;
        }
        if (SkMatrix::kTranslate_Mask == tm) {
            const SkScalar tx = mx.getTranslateX();
            const SkScalar ty = mx.getTranslateY();
            Sk2s trans(tx, ty);
            for (int i = 0; i < count; ++i) {
                (Sk2s::Load(&pts->fX) + trans).store(&pts->fX);
                pts = (SkPoint*)((intptr_t)pts + stride);
            }
            return;
        }
        // Insert other special-cases here (e.g. scale+translate)
        
        // general case
        SkMatrix::MapXYProc proc = mx.getMapXYProc();
        for (int i = 0; i < count; ++i) {
            proc(mx, pts->fX, pts->fY, pts);
            pts = (SkPoint*)((intptr_t)pts + stride);
        }
    }

    static void SetMappedRectFan(const SkMatrix& mx, const SkRect& rect, SkPoint quad[4]) {
        SkMatrix::TypeMask tm = mx.getType();
        SkScalar l = rect.fLeft;
        SkScalar t = rect.fTop;
        SkScalar r = rect.fRight;
        SkScalar b = rect.fBottom;
        if (tm <= (SkMatrix::kScale_Mask | SkMatrix::kTranslate_Mask)) {
            const SkScalar tx = mx.getTranslateX();
            const SkScalar ty = mx.getTranslateY();
            if (tm <= SkMatrix::kTranslate_Mask) {
                l += tx;
                t += ty;
                r += tx;
                b += ty;
            } else {
                const SkScalar sx = mx.getScaleX();
                const SkScalar sy = mx.getScaleY();
                l = sx * l + tx;
                t = sy * t + ty;
                r = sx * r + tx;
                b = sy * b + ty;
            }
            quad[0].set(l, t);
            quad[1].set(l, b);
            quad[2].set(r, b);
            quad[3].set(r, t);
        } else {
            quad[0].setRectFan(l, t, r, b);
            mx.mapPoints(quad, quad, 4);
        }
    }
};

#endif
