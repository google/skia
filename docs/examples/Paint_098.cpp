#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=d13d787c1e36f515319fc998411c1d91
REG_FIDDLE(Paint_098, 256, 128, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setTextSize(24);
    paint.setTextScaleX(.8f);
    canvas->drawString("narrow", 10, 20, paint);
    paint.setTextScaleX(1);
    canvas->drawString("normal", 10, 60, paint);
    paint.setTextScaleX(1.2f);
    canvas->drawString("wide", 10, 100, paint);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
