#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=505e47b3e6474ebdecdc04c3c2af2c34
REG_FIDDLE(RRect_notequal_operator, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkRRect rrect1 = SkRRect::MakeRectXY({10, 20, 60, 220}, 50, 100);
    SkRRect rrect2 = SkRRect::MakeRectXY(rrect1.rect(), 50, 50);
    SkRRect rrect3 = SkRRect::MakeOval(rrect1.rect());
    canvas->drawRRect(rrect1, SkPaint());
    std::string str = "rrect1 " + std::string(rrect1 == rrect2 ? "=" : "!") + "= rrect2";
    canvas->drawString(str.c_str(), 10, 240, SkPaint());
    canvas->translate(70, 0);
    canvas->drawRRect(rrect2, SkPaint());
    canvas->translate(70, 0);
    canvas->drawRRect(rrect3, SkPaint());
    str = "rrect2 " + std::string(rrect2 == rrect3 ? "=" : "!") + "= rrect3";
    canvas->drawString(str.c_str(), -20, 240, SkPaint());
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
