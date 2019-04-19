#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=4fb2da4a3d9b14ca4ac24eefb0f5126a
REG_FIDDLE(RGBToHSV, 256, 256, false, 3) {
void draw(SkCanvas* canvas) {
    canvas->drawBitmap(source, 0, 0);
    SkPaint bgPaint;
    bgPaint.setColor(0xafffffff);
    canvas->drawRect({20, 30, 110, 90}, bgPaint);
    SkScalar hsv[3];
    SkColor c = source.getColor(226, 128);
    SkRGBToHSV(SkColorGetR(c), SkColorGetG(c), SkColorGetB(c), hsv);
    canvas->drawString(("h: " + std::to_string(hsv[0]).substr(0, 6)).c_str(), 27, 45, SkPaint());
    canvas->drawString(("s: " + std::to_string(hsv[1]).substr(0, 6)).c_str(), 27, 65, SkPaint());
    canvas->drawString(("v: " + std::to_string(hsv[2]).substr(0, 6)).c_str(), 27, 85, SkPaint());
    canvas->drawLine(110, 90, 226, 128, SkPaint());
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
