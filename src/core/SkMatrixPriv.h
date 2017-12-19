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
#include "SkPointPriv.h"

class SkMatrixPriv {
public:
    enum {
        // writeTo/readFromMemory will never return a value larger than this
        kMaxFlattenSize = 9 * sizeof(SkScalar) + sizeof(uint32_t),
    };

    static size_t WriteToMemory(const SkMatrix& matrix, void* buffer) {
        return matrix.writeToMemory(buffer);
    }

    static size_t ReadFromMemory(SkMatrix* matrix, const void* buffer, size_t length) {
        return matrix->readFromMemory(buffer, length);
    }

    typedef SkMatrix::MapXYProc MapXYProc;
    typedef SkMatrix::MapPtsProc MapPtsProc;


    static MapPtsProc GetMapPtsProc(const SkMatrix& matrix) {
        return SkMatrix::GetMapPtsProc(matrix.getType());
    }

    static MapXYProc GetMapXYProc(const SkMatrix& matrix) {
        return SkMatrix::GetMapXYProc(matrix.getType());
    }

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

    static void MapHomogeneousPointsWithStride(const SkMatrix& mx, SkPoint3 dst[],
                                               const SkPoint3 src[], size_t stride, int count);

    static void SetMappedRectTriStrip(const SkMatrix& mx, const SkRect& rect, SkPoint quad[4]) {
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
           SkPointPriv::SetRectTriStrip(quad, l, t, r, b, sizeof(SkPoint));
        } else {
            SkPointPriv::SetRectTriStrip(quad, l, t, r, b, sizeof(SkPoint));
            mx.mapPoints(quad, quad, 4);
        }
    }
};

#endif
