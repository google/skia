#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=f6959ea422a7c6e98ddfad216a52c707
REG_FIDDLE(RRect_isSimple, 256, 100, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setTextSize(16);
    SkVector radii[] = {{40, 30}, {40, 30}, {40, 30}, {40, 30}};
    SkRRect rrect;
    rrect.setRectRadii({30, 10, 100, 60}, radii);
    canvas->drawRRect(rrect, paint);
    canvas->drawString(rrect.isSimple() ? "simple" : "not simple", 64, 90, paint);
    radii[0].fX = 35;
    rrect.setRectRadii(rrect.getBounds(), radii);
    canvas->translate(128, 0);
    canvas->drawRRect(rrect, paint);
    canvas->drawString(rrect.isSimple() ? "simple" : "not simple", 64, 90, paint);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
