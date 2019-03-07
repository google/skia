// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=2431ebc7e7d1e91e6d9daafd0f7a478f
REG_FIDDLE(Bitmap_032, 256, 160, false, 3) {
void draw(SkCanvas* canvas) {
    SkRect bounds;
    source.getBounds(&bounds);
    bounds.offset(100, 100);
    SkPaint paint;
    paint.setColor(SK_ColorGRAY);
    canvas->scale(.25f, .25f);
    canvas->drawRect(bounds, paint);
    canvas->drawBitmap(source, 40, 40);
}
}  // END FIDDLE
