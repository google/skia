// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=471e7aad0feaf9ec3a21757a317a64f5
REG_FIDDLE(RRect_empty_constructor, 256, 60, false, 0) {
void draw(SkCanvas* canvas) {
    SkRRect rrect;
    SkPaint p;
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(10);
    canvas->drawRRect(rrect, p);
    rrect.setRect({10, 10, 100, 50});
    canvas->drawRRect(rrect, p);
}
}  // END FIDDLE
