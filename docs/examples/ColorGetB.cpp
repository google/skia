#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=9ee27675284faea375611dc88123a2c5
REG_FIDDLE(ColorGetB, 256, 256, false, 3) {
void draw(SkCanvas* canvas) {
    canvas->drawBitmap(source, 0, 0);
    SkPaint bgPaint;
    bgPaint.setColor(0xafffffff);
    canvas->drawRect({20, 50, 80, 70}, bgPaint);
    uint8_t blue = SkColorGetB(source.getColor(168, 170));
    canvas->drawString(std::to_string(blue).c_str(), 40, 65, SkPaint());
    canvas->drawLine(80, 70, 168, 170, SkPaint());
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
