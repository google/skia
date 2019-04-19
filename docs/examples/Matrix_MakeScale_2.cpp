// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=2956aeb50fa862cdb13995e1e56a4bc8
REG_FIDDLE(Matrix_MakeScale_2, 256, 256, false, 4) {
void draw(SkCanvas* canvas) {
    canvas->concat(SkMatrix::MakeScale(4));
    canvas->drawBitmap(source, 0, 0);
}
}  // END FIDDLE
