/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkMatrix.h"
#include "SkMatrix22.h"
#include "SkPoint.h"
#include "SkScalar.h"

void SkComputeGivensRotation(const SkVector& h, SkMatrix* G) {
    const SkScalar& a = h.fX;
    const SkScalar& b = h.fY;
    SkScalar c, s;
    if (0 == b) {
        c = SkScalarCopySign(SK_Scalar1, a);
        s = 0;
        //r = SkScalarAbs(a);
    } else if (0 == a) {
        c = 0;
        s = -SkScalarCopySign(SK_Scalar1, b);
        //r = SkScalarAbs(b);
    } else if (SkScalarAbs(b) > SkScalarAbs(a)) {
        SkScalar t = a / b;
        SkScalar u = SkScalarCopySign(SkScalarSqrt(SK_Scalar1 + t*t), b);
        s = -SK_Scalar1 / u;
        c = -s * t;
        //r = b * u;
    } else {
        SkScalar t = b / a;
        SkScalar u = SkScalarCopySign(SkScalarSqrt(SK_Scalar1 + t*t), a);
        c = SK_Scalar1 / u;
        s = -c * t;
        //r = a * u;
    }

    G->setSinCos(s, c);
}
