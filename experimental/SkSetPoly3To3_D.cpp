
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkMatrix.h"

// FIXME: needs to be in a header
bool SkSetPoly3To3_D(SkMatrix* matrix, const SkPoint src[3], const SkPoint dst[3]);

typedef int64_t SkDScalar;

static SkScalar SkDScalar_toScalar(SkDScalar value) {
    SkDScalar result = (value + (1 << 15)) >> 16;
    SkDEBUGCODE(int top = static_cast<int>(result >> 31);)
    SkASSERT(top == 0 || top == -1);
    return (SkScalar)result;
}

static SkDScalar SkDScalar_setMul(SkScalar a, SkScalar b) {
    return (SkDScalar) ((SkDScalar) a * (SkDScalar) b);
}

static void computeOuterProduct(SkMatrix* matrix,
                                const SkPoint pts0[3], const SkPoint& ave0,
                                const SkPoint pts1[3], const SkPoint& ave1) {
    SkDScalar tmp[4];
    sk_bzero(tmp, sizeof(tmp));

    for (int i = 0; i < 3; i++) {
        SkScalar x0 = pts0[i].fX - ave0.fX;
        SkScalar y0 = pts0[i].fY - ave0.fY;
        SkScalar x1 = pts1[i].fX - ave1.fX;
        SkScalar y1 = pts1[i].fY - ave1.fY;
        tmp[0] += SkDScalar_setMul(x0, x1);
        tmp[1] += SkDScalar_setMul(x0, y1);
        tmp[2] += SkDScalar_setMul(y0, x1);
        tmp[3] += SkDScalar_setMul(y0, y1);
    }
    matrix->reset();
    matrix->setScaleX(SkDScalar_toScalar(tmp[0]));
    matrix->setSkewY( SkDScalar_toScalar(tmp[1]));
    matrix->setSkewX( SkDScalar_toScalar(tmp[2]));
    matrix->setScaleY(SkDScalar_toScalar(tmp[3]));
}

static SkScalar dot(SkScalar ax, SkScalar ay, SkScalar bx, SkScalar by) {
    return SkDScalar_toScalar(SkDScalar_setMul(ax, bx) +
                              SkDScalar_setMul(ay, by));
}

bool SkSetPoly3To3_D(SkMatrix* matrix, const SkPoint src[3], const SkPoint dst[3]) {
    const SkPoint& srcAve = src[0];
    const SkPoint& dstAve = dst[0];

    SkMatrix srcOP, dstOP;

    computeOuterProduct(&srcOP, src, srcAve, src, srcAve);

    if (!srcOP.invert(&srcOP)) {
        return false;
    }

    computeOuterProduct(&dstOP, src, srcAve, dst, dstAve);

    matrix->setConcat(dstOP, srcOP);
    matrix->setTranslateX(dstAve.fX - dot(srcAve.fX, srcAve.fY,
                                    matrix->getScaleX(), matrix->getSkewX()));
    matrix->setTranslateY(dstAve.fY - dot(srcAve.fX, srcAve.fY,
                                    matrix->getSkewY(), matrix->getScaleY()));
    return true;
}
