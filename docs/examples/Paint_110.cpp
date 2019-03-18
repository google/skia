#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=85436c71aab5410767fc688ab0573e09
REG_FIDDLE(Paint_countText, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    const uint8_t utf8[] = { 0x24, 0xC2, 0xA2, 0xE2, 0x82, 0xAC, 0xC2, 0xA5, 0xC2, 0xA3 };
    SkDebugf("count = %d\n", paint.countText(utf8, sizeof(utf8)));
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
