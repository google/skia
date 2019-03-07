#if 0  // disabled
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=412afffdf4682baa503a4e2e99201967
REG_FIDDLE(Canvas_046, 256, 128, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    canvas->scale(4, 6);
    canvas->drawString("truth", 2, 10, paint);
    canvas->resetMatrix();
    canvas->scale(2.8f, 6);
    canvas->drawString("consequences", 2, 20, paint);
}
}  // END FIDDLE
#endif  // disabled
