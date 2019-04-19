// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=a45cdd46ef2fe0df62d84d41713e82e2
REG_FIDDLE(RRect_offset, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    SkRRect rrect = SkRRect::MakeRectXY({100, 20, 140, 220}, 50, 100);
    for (int index = 0; index < 25; ++index) {
       canvas->drawRRect(rrect, paint);
       rrect.offset(3, 3);
    }
}
}  // END FIDDLE
