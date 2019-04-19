#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=d6da38577f189eaa6d9df75f6c3ed252
REG_FIDDLE(ColorGetR, 256, 256, false, 3) {
void draw(SkCanvas* canvas) {
    canvas->drawBitmap(source, 0, 0);
    SkPaint bgPaint;
    bgPaint.setColor(0xafffffff);
    canvas->drawRect({20, 50, 80, 70}, bgPaint);
    uint8_t red = SkColorGetR(source.getColor(226, 128));
    canvas->drawString(std::to_string(red).c_str(), 40, 65, SkPaint());
    canvas->drawLine(80, 70, 226, 128, SkPaint());
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
