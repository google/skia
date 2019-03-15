// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=024200960eb52fee1f471514607e6001
REG_FIDDLE(Region_045, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkRegion test;
    SkIRect rects[] = {{40, 20, 50, 30}, {70, 40, 80, 50}, { 60, 10, 70, 20}};
    test.setRects(rects, SK_ARRAY_COUNT(rects));
    SkPaint paint;
    for (auto color :  { SK_ColorRED, SK_ColorBLUE, SK_ColorGREEN, SK_ColorMAGENTA } ) {
        paint.setColor(color);
        canvas->drawRegion(test, paint);
        SkRegion second;
        test.translate(10, test.getBounds().fBottom, &second);
        test.op(second, SkRegion::kXOR_Op);
        test.translate(30, 0);
    }
}
}  // END FIDDLE
