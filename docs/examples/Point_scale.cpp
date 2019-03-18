// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=972e4e230806281adb928e068bcd8551
REG_FIDDLE(Point_scale, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    SkPoint point = {40, -15}, scaled;
    SkPoint origin = {30, 110};
    for (auto scale : {1, 2, 3, 5}) {
        paint.setStrokeWidth(scale * 5);
        paint.setARGB(0x7f, 0x9f, 0xbf, 0x33 * scale);
        point.scale(scale, &scaled);
        canvas->drawLine(origin, origin + scaled, paint);
    }
}
}  // END FIDDLE
