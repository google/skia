#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=81345f7619a072bb2b0cf59810fe86d0
REG_FIDDLE(RRect_getSimpleRadii, 256, 100, false, 0) {
void draw(SkCanvas* canvas) {
    auto drawDetails = [=](const SkRRect& rrect) {
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setTextSize(12);
        canvas->drawRRect(rrect, paint);
        SkVector corner = rrect.getSimpleRadii();
        std::string label = "corner: " + std::to_string(corner.fX).substr(0, 3) + ", " +
                        std::to_string(corner.fY).substr(0, 3);
        canvas->drawString(label.c_str(), 64, 90, paint);
        canvas->translate(128, 0);
    };
    SkRRect rrect = SkRRect::MakeRect({30, 10, 100, 60});
    drawDetails(rrect);
    rrect.setRectXY(rrect.getBounds(), 5, 8);
    drawDetails(rrect);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
