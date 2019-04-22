#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=535d38b2c019299d915170f7b03d5fea
REG_FIDDLE(ColorGetG, 256, 256, false, 3) {
void draw(SkCanvas* canvas) {
    canvas->drawBitmap(source, 0, 0);
    SkPaint bgPaint;
    bgPaint.setColor(0xafffffff);
    canvas->drawRect({20, 50, 80, 70}, bgPaint);
    uint8_t green = SkColorGetG(source.getColor(57, 192));
    canvas->drawString(std::to_string(green).c_str(), 40, 65, SkPaint());
    canvas->drawLine(80, 70, 57, 192, SkPaint());
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
