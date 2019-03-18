#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=0e6fbb7773cd925b274552f4cd1abef2
REG_FIDDLE(Paint_setTypeface, 256, 64, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setTypeface(SkTypeface::MakeFromName("monospace", SkFontStyle()));
    canvas->drawString("hamburgerfons", 10, 30, paint);
    paint.setTypeface(nullptr);
    canvas->drawString("hamburgerfons", 10, 50, paint);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
