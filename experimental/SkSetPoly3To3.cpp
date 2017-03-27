
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkMatrix.h"
#include "SkMalloc.h"

// FIXME: needs to be in a header
bool SkSetPoly3To3(SkMatrix* matrix, const SkPoint src[3], const SkPoint dst[3]);

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

static SkScalar dot(SkScalar ax, SkScalar ay, SkScalar bx, SkScalar by) {
    return ax * bx + ay * by;
}

bool SkSetPoly3To3(SkMatrix* matrix, const SkPoint src[3], const SkPoint dst[3]) {
    const SkPoint& srcAve = src[0];
    const SkPoint& dstAve = dst[0];

    SkScalar srcOP[4], dstOP[4];

    computeOuterProduct(srcOP, src, srcAve, src, srcAve);
    computeOuterProduct(dstOP, src, srcAve, dst, dstAve);

    SkScalar det = srcOP[0] * srcOP[3] - srcOP[1] * srcOP[2];

    // need SkScalarNearlyZeroSquared for this (to match Chrome's fix)
    if (SkScalarNearlyZero(det)) {
        return false;
    }

    SkScalar invDet = SkScalarInvert(det);

    // now compute invDet * [srcOP]T * [dstOP]

    // scale and transpose
    const SkScalar srcOP0 =  srcOP[3] * invDet;
    const SkScalar srcOP1 = -srcOP[1] * invDet;
    const SkScalar srcOP2 = -srcOP[2] * invDet;
    const SkScalar srcOP3 =  srcOP[0] * invDet;

    matrix->reset();
    matrix->setScaleX(dot(srcOP0, srcOP1, dstOP[0], dstOP[2]));
    matrix->setSkewX( dot(srcOP2, srcOP3, dstOP[0], dstOP[2]));
    matrix->setSkewY (dot(srcOP0, srcOP1, dstOP[1], dstOP[3]));
    matrix->setScaleY(dot(srcOP2, srcOP3, dstOP[1], dstOP[3]));
    matrix->setTranslateX(dstAve.fX - dot(srcAve.fX, srcAve.fY,
                                    matrix->getScaleX(), matrix->getSkewX()));
    matrix->setTranslateY(dstAve.fY - dot(srcAve.fX, srcAve.fY,
                                    matrix->getSkewY(), matrix->getScaleY()));
    return true;
}
