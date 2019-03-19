// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=4e5b9e53aa1b200fed3ee6596ca01f0e
REG_FIDDLE(Region_translate, 256, 90, false, 0) {
void draw(SkCanvas* canvas) {
    SkRegion test;
    SkIRect rects[] = {{40, 20, 50, 30}, {70, 40, 80, 50}, { 60, 10, 70, 20}};
    test.setRects(rects, SK_ARRAY_COUNT(rects));
    SkPaint paint;
    for (auto color :  { SK_ColorRED, SK_ColorBLUE, SK_ColorGREEN, SK_ColorMAGENTA } ) {
        paint.setColor(color);
        canvas->drawRegion(test, paint);
        test.translate(10, 10);
    }
}
}  // END FIDDLE
