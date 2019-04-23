// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=fed43797f13796529cb6731385d6f8f3
REG_FIDDLE(Matrix_setScaleTranslate, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkMatrix matrix;
    matrix.setScaleTranslate(1, 2, 3, 4);
    matrix.dump();
}
}  // END FIDDLE
