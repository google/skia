// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(redwhiteonblue, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    canvas->clear(SK_ColorBLUE);
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(SK_ColorWHITE);
    canvas->drawRect({10.5, 10.5, 120.5, 120.5}, paint);
    paint.setColor(SK_ColorRED);
    canvas->drawRect({120.5, 10.5, 230.5, 120.5}, paint);
}
}  // END FIDDLE
