// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=824b5a3fcfd46a7e1c5f9e3c16e6bb39
REG_FIDDLE(Rect_069, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkRect rect = {6.f / 7, 2.f / 3, 26.f / 10, 42.f / 6};
 rect.dumpHex();
 SkRect copy = SkRect::MakeLTRB(SkBits2Float(0x3f5b6db7), /* 0.857143 */
                  SkBits2Float(0x3f2aaaab), /* 0.666667 */
                  SkBits2Float(0x40266666), /* 2.600000 */
                  SkBits2Float(0x40e00000)  /* 7.000000 */);
 SkDebugf("rect is " "%s" "equal to copy\n", rect == copy ? "" : "not ");
}
}  // END FIDDLE
