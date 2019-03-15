// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=7ff17718111df6d6f95381d8a8f1b389
REG_FIDDLE(Matrix_000, 256, 256, false, 4) {
void draw(SkCanvas* canvas) {
    canvas->concat(SkMatrix::MakeScale(4, 3));
    canvas->drawBitmap(source, 0, 0);
}
}  // END FIDDLE
