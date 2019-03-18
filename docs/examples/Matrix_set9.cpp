// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=ec5de0d23e5fe28ba7628625d1402e85
REG_FIDDLE(Matrix_set9, 256, 256, false, 4) {
void draw(SkCanvas* canvas) {
    SkMatrix m;
    SkScalar buffer[9] = {4, 0, 3,    0, 5, 4,     0, 0, 1};
    m.set9(buffer);
    canvas->concat(m);
    canvas->drawBitmap(source, 0, 0);
}
}  // END FIDDLE
