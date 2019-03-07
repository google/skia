// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=c6c5b40cad7c3a839fdf576b380391a6
REG_FIDDLE(Rect_021, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    auto debugster = [](const SkRect& test) -> void {
        SkRect negZero = {-0.0f, -0.0f, 2, 2};
        SkDebugf("{%g, %g, %g, %g} %c= {%g, %g, %g, %g} %s numerically equal\n",
                 test.fLeft, test.fTop, test.fRight, test.fBottom,
                 negZero.fLeft, negZero.fTop, negZero.fRight, negZero.fBottom,
                 test == negZero ? '=' : '!',
                 test.fLeft == negZero.fLeft && test.fTop == negZero.fTop &&
                 test.fRight == negZero.fRight && test.fBottom == negZero.fBottom ?
                 "and are" : "yet are not");
    };
    SkRect tests[] = {{0, 0, 2, 2}, {-0, -0, 2, 2}, {0.0f, 0.0f, 2, 2}};
    SkDebugf("tests are %s" "equal\n", tests[0] == tests[1] && tests[1] == tests[2] ? "" : "not ");
    for (auto rect : tests) {
        debugster(rect);
    }
}
}  // END FIDDLE
