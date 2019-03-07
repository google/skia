// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=33b029064e8d1928e42a587c953d0e4e
REG_FIDDLE(Color4f_004, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    uint8_t red = 77, green = 101, blue = 153, alpha = 43;
    SkColor argb = SkColorSetARGB(alpha, red, green, blue);
    SkColor4f color4f = SkColor4f::FromColor(argb);
    SkDebugf("red=%g green=%g blue=%g alpha=%g\n", color4f.fR, color4f.fG, color4f.fB, color4f.fA);
    SkColor fromColor4f = color4f.toSkColor();
    SkDebugf("red=%d green=%d blue=%d alpha=%d\n", SkColorGetR(fromColor4f),
             SkColorGetG(fromColor4f), SkColorGetB(fromColor4f), SkColorGetA(fromColor4f));
}
}  // END FIDDLE
