#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=4606ae1be792d6bc46d496432f050ee9
REG_FIDDLE(Device_Text, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkBitmap bitmap;
    bitmap.allocN32Pixels(24, 33);
    SkCanvas offscreen(bitmap);
    offscreen.clear(SK_ColorWHITE);
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setTextSize(20);
    for (bool lcd : { false, true }) {
        paint.setLCDRenderText(lcd);
        for (bool subpixel : { false, true }) {
            paint.setSubpixelText(subpixel);
            offscreen.drawString(",,,,", 0, 4, paint);
            offscreen.translate(0, 7);
        }
    }
    canvas->drawBitmap(bitmap, 4, 12);
    canvas->scale(9, 9);
    canvas->drawBitmap(bitmap, 4, -1);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
