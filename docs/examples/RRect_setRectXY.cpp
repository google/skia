// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=6ac569e40fb68c758319e85428b9ae95
REG_FIDDLE(RRect_setRectXY, 256, 70, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    SkRRect rrect = SkRRect::MakeRectXY({30, 10, 100, 60}, 20, 20);
    canvas->drawRRect(rrect, paint);
    rrect.setRectXY(rrect.getBounds(), 5, 5);
    paint.setColor(SK_ColorWHITE);
    paint.setBlendMode(SkBlendMode::kExclusion);
    canvas->drawRRect(rrect, paint);
}
}  // END FIDDLE
