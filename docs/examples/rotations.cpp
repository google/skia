// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(rotations, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    canvas->translate(128, 0);
    canvas->rotate(60);
    SkRect rect = SkRect::MakeXYWH(0, 0, 200, 100);

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(0xff4285F4);
    canvas->drawRect(rect, paint);

    canvas->rotate(SkIntToScalar(20));
    paint.setColor(0xffDB4437);
    canvas->drawRect(rect, paint);
}
}  // END FIDDLE
