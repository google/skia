#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=b62c183dc435d1fc091111fb2f3c3f8e
REG_FIDDLE(RRect_isComplex, 256, 100, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setTextSize(16);
    SkVector radii[] = {{25, 30}, {40, 30}, {40, 30}, {20, 30}};
    SkRRect rrect;
    rrect.setRectRadii({30, 10, 100, 60}, radii);
    canvas->drawRRect(rrect, paint);
    canvas->drawString(rrect.isComplex() ? "complex" : "not complex", 64, 90, paint);
    radii[0].fX = 20;
    rrect.setRectRadii(rrect.getBounds(), radii);
    canvas->translate(128, 0);
    canvas->drawRRect(rrect, paint);
    canvas->drawString(rrect.isComplex() ? "complex" : "not complex", 64, 90, paint);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
