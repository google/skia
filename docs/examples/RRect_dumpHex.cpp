// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=c73f5e2644d949b859f05bd367883454
REG_FIDDLE(RRect_dumpHex, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkRRect rrect = SkRRect::MakeRect({6.f / 7, 2.f / 3, 6.f / 7, 2.f / 3});
    rrect.dumpHex();
    SkRect bounds = SkRect::MakeLTRB(SkBits2Float(0x3f5b6db7), /* 0.857143 */
                     SkBits2Float(0x3f2aaaab), /* 0.666667 */
                     SkBits2Float(0x3f5b6db7), /* 0.857143 */
                     SkBits2Float(0x3f2aaaab)  /* 0.666667 */);
    const SkPoint corners[] = {
        { SkBits2Float(0x00000000), SkBits2Float(0x00000000) }, /* 0.000000 0.000000 */
        { SkBits2Float(0x00000000), SkBits2Float(0x00000000) }, /* 0.000000 0.000000 */
        { SkBits2Float(0x00000000), SkBits2Float(0x00000000) }, /* 0.000000 0.000000 */
        { SkBits2Float(0x00000000), SkBits2Float(0x00000000) }, /* 0.000000 0.000000 */
    };
    SkRRect copy;
    copy.setRectRadii(bounds, corners);
    SkDebugf("rrect is " "%s" "equal to copy\n", rrect == copy ? "" : "not ");
}
}  // END FIDDLE
