// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(draw_text_fails, 256, 46, false, 0) {
void draw(SkCanvas* canvas) {
    canvas->drawString("ABCDE ⒶⒷⒸⒹⒺ 𝓐𝓑𝓒𝓓𝓔", 18, 32, SkFont(nullptr, 12), SkPaint());
}
}  // END FIDDLE
