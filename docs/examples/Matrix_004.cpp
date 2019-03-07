// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=ba19b36df8cd78586f3dff54e2d4c093
REG_FIDDLE(Matrix_004, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    auto debugster = [](const char* prefix, const SkMatrix& matrix) -> void {
        SkString typeMask;
        typeMask += SkMatrix::kIdentity_Mask == matrix.getType() ? "kIdentity_Mask " : "";
        typeMask += SkMatrix::kTranslate_Mask & matrix.getType() ? "kTranslate_Mask " : "";
        typeMask += SkMatrix::kScale_Mask & matrix.getType() ? "kScale_Mask " : "";
        typeMask += SkMatrix::kAffine_Mask & matrix.getType() ? "kAffine_Mask " : "";
        typeMask += SkMatrix::kPerspective_Mask & matrix.getType() ? "kPerspective_Mask" : "";
        SkDebugf("after %s: %s\n", prefix, typeMask.c_str());
    };
SkMatrix matrix;
matrix.reset();
debugster("reset", matrix);
matrix.postTranslate(1, 0);
debugster("postTranslate", matrix);
matrix.postScale(2, 1);
debugster("postScale", matrix);
matrix.postRotate(45);
debugster("postScale", matrix);
SkPoint polys[][4] = {{{0, 0}, {0, 1}, {1, 1}, {1, 0}}, {{0, 0}, {0, 1}, {2, 1}, {1, 0}}};
matrix.setPolyToPoly(polys[0], polys[1], 4);
debugster("setPolyToPoly", matrix);
}
}  // END FIDDLE
