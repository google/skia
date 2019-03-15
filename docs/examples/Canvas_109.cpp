#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=95c6a7ef82993a8d2add676080e9438a
REG_FIDDLE(Canvas_109, 256, 40, false, 0) {
void draw(SkCanvas* canvas) {
    SkScalar xpos[] = { 20, 40, 80, 160 };
    SkPaint paint;
    canvas->drawPosTextH("XXXX", 4, xpos, 20, paint);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
