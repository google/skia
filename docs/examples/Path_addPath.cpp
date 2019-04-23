#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=c416bddfe286628974e1c7f0fd66f3f4
REG_FIDDLE(Path_addPath, 256, 180, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setTextSize(128);
    paint.setFakeBoldText(true);
    SkPath dest, text;
    paint.getTextPath("O", 1, 50, 120, &text);
    for (int i = 0; i < 3; i++) {
        dest.addPath(text, i * 20, i * 20);
    }
    Simplify(dest, &dest);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(3);
    canvas->drawPath(dest, paint);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
