#if 0  // disabled
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=bcf5baea1c66a957d5ffd7b54bbbfeff
REG_FIDDLE(Canvas_042, 256, 192, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setTextSize(96);
    canvas->drawString("A1", 130, 100, paint);
    canvas->rotate(180, 130, 100);
    canvas->drawString("A1", 130, 100, paint);
}
}  // END FIDDLE
#endif  // disabled
