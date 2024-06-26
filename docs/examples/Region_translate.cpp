// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Region_translate, 256, 90, false, 0) {
void draw(SkCanvas* canvas) {
    SkRegion test;
    SkIRect rects[] = {{40, 20, 50, 30}, {70, 40, 80, 50}, { 60, 10, 70, 20}};
    test.setRects(rects, std::size(rects));
    SkPaint paint;
    for (auto color :  { SK_ColorRED, SK_ColorBLUE, SK_ColorGREEN, SK_ColorMAGENTA } ) {
        paint.setColor(color);
        canvas->drawRegion(test, paint);
        test.translate(10, 10);
    }
}
}  // END FIDDLE
