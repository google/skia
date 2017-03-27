
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkMatrix.h"
#include "SkMalloc.h"

// FIXME: needs to be in a header
bool SkSetPoly3To3_A(SkMatrix* matrix, const SkPoint src[3], const SkPoint dst[3]);

typedef double SkDScalar;

static SkScalar SkDScalar_toScalar(SkDScalar value) {
    return static_cast<float>(value);
}
static SkScalar divide(SkDScalar numer, SkDScalar denom) {
    return static_cast<float>(numer / denom);
}

static SkDScalar SkDScalar_setMul(SkScalar a, SkScalar b) {
    return (SkDScalar) ((SkDScalar) a * b);
}

static void computeOuterProduct(SkScalar op[4],
                                const SkPoint pts0[3], const SkPoint& ave0,
                                const SkPoint pts1[3], const SkPoint& ave1) {
    sk_bzero(op, 4 * sizeof(op[0]));
    for (int i = 0; i < 3; i++) {
        SkScalar x0 = pts0[i].fX - ave0.fX;
        SkScalar y0 = pts0[i].fY - ave0.fY;
        SkScalar x1 = pts1[i].fX - ave1.fX;
        SkScalar y1 = pts1[i].fY - ave1.fY;
        op[0] += x0 * x1;
        op[1] += x0 * y1;
        op[2] += y0 * x1;
        op[3] += y0 * y1;
    }
}

static SkDScalar ddot(SkScalar ax, SkScalar ay, SkScalar bx, SkScalar by) {
    return SkDScalar_setMul(ax, bx) + SkDScalar_setMul(ay, by);
}

static SkScalar dot(SkScalar ax, SkScalar ay, SkScalar bx, SkScalar by) {
    return SkDScalar_toScalar(ddot(ax, ay, bx, by));
}

bool SkSetPoly3To3_A(SkMatrix* matrix, const SkPoint src[3], const SkPoint dst[3]) {
    const SkPoint& srcAve = src[0];
    const SkPoint& dstAve = dst[0];

    SkScalar srcOP[4], dstOP[4];

    computeOuterProduct(srcOP, src, srcAve, src, srcAve);
    computeOuterProduct(dstOP, src, srcAve, dst, dstAve);

    SkDScalar det = SkDScalar_setMul(srcOP[0], srcOP[3]) -
                    SkDScalar_setMul(srcOP[1], srcOP[2]);

    SkDScalar M[4];

    const SkScalar srcOP0 = srcOP[3];
    const SkScalar srcOP1 = -srcOP[1];
    const SkScalar srcOP2 = -srcOP[2];
    const SkScalar srcOP3 = srcOP[0];

    M[0] = ddot(srcOP0, srcOP1, dstOP[0], dstOP[2]);
    M[1] = ddot(srcOP2, srcOP3, dstOP[0], dstOP[2]);
    M[2] = ddot(srcOP0, srcOP1, dstOP[1], dstOP[3]);
    M[3] = ddot(srcOP2, srcOP3, dstOP[1], dstOP[3]);

    matrix->reset();
    matrix->setScaleX(divide(M[0], det));
    matrix->setSkewX( divide(M[1], det));
    matrix->setSkewY (divide(M[2], det));
    matrix->setScaleY(divide(M[3], det));
    matrix->setTranslateX(dstAve.fX - dot(srcAve.fX, srcAve.fY,
                                    matrix->getScaleX(), matrix->getSkewX()));
    matrix->setTranslateY(dstAve.fY - dot(srcAve.fX, srcAve.fY,
                                    matrix->getSkewY(), matrix->getScaleY()));
    return true;
}
