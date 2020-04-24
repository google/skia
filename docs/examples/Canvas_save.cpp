// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=e477dce358a9ba3b0aa1bf33b8a376de
REG_FIDDLE(Canvas_save, 256, 100, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    SkRect rect = { 0, 0, 25, 25 };
    canvas->drawRect(rect, paint);
    canvas->save();
    canvas->translate(50, 50);
    canvas->drawRect(rect, paint);
    canvas->restore();
    paint.setColor(SK_ColorRED);
    canvas->drawRect(rect, paint);
}
}  // END FIDDLE
