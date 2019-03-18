// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=d7f4fdc8bc63ca8410ed166ecef0aef3
REG_FIDDLE(Region_equal1_operator, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    auto debugster = [](const char* prefix, const SkRegion& a, const SkRegion& b) -> void {
                SkDebugf("%s one %c= two\n", prefix, a == b ? '=' : '!');
    };
    SkRegion one;
    SkRegion two;
    debugster("empty", one, two);
    one.setRect({1, 2, 3, 4});
    debugster("set rect", one, two);
    one.setEmpty();
    debugster("set empty", one, two);
}
}  // END FIDDLE
