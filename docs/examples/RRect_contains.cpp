#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=0bb057140e4119234bdd2e8dd2f0fa19
REG_FIDDLE(RRect_contains, 256, 110, false, 0) {
void draw(SkCanvas* canvas) {
    SkRect test = {10, 10, 110, 80};
    SkRRect rrect = SkRRect::MakeRect(test);
    SkRRect oval = SkRRect::MakeOval(test);
    test.inset(10, 10);
    SkPaint paint;
    paint.setAntiAlias(true);
    canvas->drawString(rrect.contains(test) ? "contains" : "does not contain", 55, 100, paint);
    canvas->drawString(oval.contains(test) ? "contains" : "does not contain", 185, 100, paint);
    paint.setStyle(SkPaint::kStroke_Style);
    canvas->drawRRect(rrect, paint);
    canvas->drawRect(test, paint);
    canvas->translate(120, 0);
    canvas->drawRRect(oval, paint);
    canvas->drawRect(test, paint);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
