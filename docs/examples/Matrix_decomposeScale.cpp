// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=139b874da0a3ede1f3df88119085c0aa
REG_FIDDLE(Matrix_decomposeScale, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkMatrix matrix;
    matrix.setRotate(90 * SK_Scalar1);
    matrix.postScale(1.f / 4, 1.f / 2);
    matrix.dump();
    SkSize scale = {SK_ScalarNaN, SK_ScalarNaN};
    SkMatrix remaining;
    remaining.reset();
    bool success = matrix.decomposeScale(&scale, &remaining);
    SkDebugf("success: %s  ", success ? "true" : "false");
    SkDebugf("scale: %g, %g\n", scale.width(), scale.height());
    remaining.dump();
    SkMatrix scaleMatrix = SkMatrix::MakeScale(scale.width(), scale.height());
    SkMatrix combined = SkMatrix::Concat(scaleMatrix, remaining);
    combined.dump();
}
}  // END FIDDLE
