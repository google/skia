#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=1080805c8449406a4e26d694bc56d2dc
REG_FIDDLE(RRect_type, 256, 100, false, 0) {
void draw(SkCanvas* canvas) {
    SkRRect rrect = SkRRect::MakeRect({10, 10, 100, 50});
    SkRRect rrect2(rrect);
    rrect2.inset(20, 20);
    SkPaint p;
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(10);
    std::string str("Type ");
    str += SkRRect::kEmpty_Type == rrect2.type() ? "=" : "!";
    str += "= SkRRect::kEmpty_Type";
    canvas->drawString(str.c_str(), 20, 80, SkPaint());
    canvas->drawRRect(rrect2, p);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
