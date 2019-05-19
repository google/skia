#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=0bcc0f86a2aefc899f3500503dce6968
REG_FIDDLE(PreMultiplyColor, 300, 128, false, 0) {
void draw(SkCanvas* canvas) {
    SkColor unpremultiplied = SkColorSetARGB(160, 128, 160, 192);
    SkPMColor premultiplied = SkPreMultiplyColor(unpremultiplied);
    canvas->drawString("Unpremultiplied:", 20, 20, SkPaint());
    std::string str = "alpha=" + std::to_string(SkColorGetA(unpremultiplied));
    str += " red=" + std::to_string(SkColorGetR(unpremultiplied));
    str += " green=" + std::to_string(SkColorGetG(unpremultiplied));
    str += " blue=" + std::to_string(SkColorGetB(unpremultiplied));
    canvas->drawString(str.c_str(), 20, 40, SkPaint());
    canvas->drawString("Premultiplied:", 20, 80, SkPaint());
    str = "alpha=" + std::to_string(SkColorGetA(premultiplied));
    str += " red=" + std::to_string(SkColorGetR(premultiplied));
    str += " green=" + std::to_string(SkColorGetG(premultiplied));
    str += " blue=" + std::to_string(SkColorGetB(premultiplied));
    canvas->drawString(str.c_str(), 20, 100, SkPaint());
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
