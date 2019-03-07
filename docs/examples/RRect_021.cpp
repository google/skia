// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=cf418af29cbab6243ac16aacd1217ffe
REG_FIDDLE(RRect_021, 256, 70, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    SkRRect rrect = SkRRect::MakeRectXY({30, 10, 100, 60}, 20, 20);
    canvas->drawRRect(rrect, paint);
    rrect.setOval(rrect.getBounds());
    paint.setColor(SK_ColorWHITE);
    paint.setBlendMode(SkBlendMode::kExclusion);
    canvas->drawRRect(rrect, paint);
}
}  // END FIDDLE
