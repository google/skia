// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=edc5fd18d961f7607d2bcbf7f7d427e5
REG_FIDDLE(Color4f_005, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    float red = 0.07, green = 0.13, blue = 0.32, alpha = 0.17;
    SkColor4f color4f = { red, green, blue, alpha };
    SkColor argb = color4f.toSkColor();
    SkDebugf("red=%d green=%d blue=%d alpha=%d\n", SkColorGetR(argb),
             SkColorGetG(argb), SkColorGetB(argb), SkColorGetA(argb));
    SkColor4f fromSkColor = SkColor4f::FromColor(argb);
    SkDebugf("red=%g green=%g blue=%g alpha=%g\n", fromSkColor.fR, fromSkColor.fG,
                                                   fromSkColor.fB, fromSkColor.fA);
}
}  // END FIDDLE
