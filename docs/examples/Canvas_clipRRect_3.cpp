// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=f583114580b2176fe3e75b0994476a84
REG_FIDDLE(Canvas_clipRRect_3, 256, 128, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    auto oval = SkRRect::MakeRectXY({10, 20, 90, 100}, 9, 13);
    canvas->clipRRect(oval, true);
    canvas->drawCircle(70, 100, 60, paint);
}
}  // END FIDDLE
