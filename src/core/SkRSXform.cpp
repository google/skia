/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkRSXform.h"

void SkRSXform::toQuad(SkScalar width, SkScalar height, SkPoint quad[4]) const {
#if 0
    // This is the slow way, but it documents what we're doing
    quad[0].set(0, 0);
    quad[1].set(width, 0);
    quad[2].set(width, height);
    quad[3].set(0, height);
    SkMatrix m;
    m.setRSXform(*this).mapPoints({quad, 4});
#else
    const SkScalar m00 = fSCos;
    const SkScalar m01 = -fSSin;
    const SkScalar m02 = fTx;
    const SkScalar m10 = -m01;
    const SkScalar m11 = m00;
    const SkScalar m12 = fTy;

    quad[0].set(m02, m12);
    quad[1].set(m00 * width + m02, m10 * width + m12);
    quad[2].set(m00 * width + m01 * height + m02, m10 * width + m11 * height + m12);
    quad[3].set(m01 * height + m02, m11 * height + m12);
#endif
}

void SkRSXform::toTriStrip(SkScalar width, SkScalar height, SkPoint strip[4]) const {
    const SkScalar m00 = fSCos;
    const SkScalar m01 = -fSSin;
    const SkScalar m02 = fTx;
    const SkScalar m10 = -m01;
    const SkScalar m11 = m00;
    const SkScalar m12 = fTy;

    strip[0].set(m02, m12);
    strip[1].set(m01 * height + m02, m11 * height + m12);
    strip[2].set(m00 * width + m02, m10 * width + m12);
    strip[3].set(m00 * width + m01 * height + m02, m10 * width + m11 * height + m12);
}
