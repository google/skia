#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=bc931c9a6eb8ffe7ea8d3fb47e07a475
REG_FIDDLE(RRect_007, 256, 100, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setTextSize(16);
    SkRRect rrect = SkRRect::MakeRect({30, 10, 100, 60});
    canvas->drawRRect(rrect, paint);
    canvas->drawString(rrect.isRect() ? "rect" : "not rect", 64, 90, paint);
    SkVector radii[] = {{10, 10}, {0, 0}, {0, 0}, {0, 0}};
    rrect.setRectRadii(rrect.getBounds(), radii);
    canvas->translate(128, 0);
    canvas->drawRRect(rrect, paint);
    canvas->drawString(rrect.isRect() ? "rect" : "not rect", 64, 90, paint);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
