/*
* Copyright 2015 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "include/core/SkMatrix.h"
#include "src/gpu/glsl/GrGLSLUtil.h"

template<> void GrGLSLGetMatrix<3>(float* dest, const SkMatrix& src) {
    // Col 0
    dest[0] = SkScalarToFloat(src[SkMatrix::kMScaleX]);
    dest[1] = SkScalarToFloat(src[SkMatrix::kMSkewY]);
    dest[2] = SkScalarToFloat(src[SkMatrix::kMPersp0]);

    // Col 1
    dest[3] = SkScalarToFloat(src[SkMatrix::kMSkewX]);
    dest[4] = SkScalarToFloat(src[SkMatrix::kMScaleY]);
    dest[5] = SkScalarToFloat(src[SkMatrix::kMPersp1]);

    // Col 2
    dest[6] = SkScalarToFloat(src[SkMatrix::kMTransX]);
    dest[7] = SkScalarToFloat(src[SkMatrix::kMTransY]);
    dest[8] = SkScalarToFloat(src[SkMatrix::kMPersp2]);
}

template<> void GrGLSLGetMatrix<4>(float* dest, const SkMatrix& src) {
    // Col 0
    dest[0] = SkScalarToFloat(src[SkMatrix::kMScaleX]);
    dest[1] = SkScalarToFloat(src[SkMatrix::kMSkewY]);
    dest[2] = 0;
    dest[3] = SkScalarToFloat(src[SkMatrix::kMPersp0]);

    // Col 1
    dest[4] = SkScalarToFloat(src[SkMatrix::kMSkewX]);
    dest[5] = SkScalarToFloat(src[SkMatrix::kMScaleY]);
    dest[6] = 0;
    dest[7] = SkScalarToFloat(src[SkMatrix::kMPersp1]);

    // Col 2
    dest[8] = 0;
    dest[9] = 0;
    dest[10] = 1;
    dest[11] = 0;

    // Col 3
    dest[12] = SkScalarToFloat(src[SkMatrix::kMTransX]);
    dest[13] = SkScalarToFloat(src[SkMatrix::kMTransY]);
    dest[14] = 0;
    dest[15] = SkScalarToFloat(src[SkMatrix::kMPersp2]);
}
