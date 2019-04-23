#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=756345484fd48ca0ea7b6cec350f73b8
REG_FIDDLE(PreMultiplyARGB, 300, 128, false, 0) {
void draw(SkCanvas* canvas) {
    SkPMColor premultiplied = SkPreMultiplyARGB(160, 128, 160, 192);
    canvas->drawString("Unpremultiplied:", 20, 20, SkPaint());
    canvas->drawString("alpha=160 red=128 green=160 blue=192", 20, 40, SkPaint());
    canvas->drawString("Premultiplied:", 20, 80, SkPaint());
    std::string str = "alpha=" + std::to_string(SkColorGetA(premultiplied));
    str += " red=" + std::to_string(SkColorGetR(premultiplied));
    str += " green=" + std::to_string(SkColorGetG(premultiplied));
    str += " blue=" + std::to_string(SkColorGetB(premultiplied));
    canvas->drawString(str.c_str(), 20, 100, SkPaint());
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
