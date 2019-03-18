// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=3357caa9d8d810f200cbccb668182496
REG_FIDDLE(Region_notequal1_operator, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    auto debugster = [](const char* prefix, const SkRegion& a, const SkRegion& b) -> void {
                SkDebugf("%s one %c= two\n", prefix, a != b ? '!' : '=');
    };
    SkRegion one;
    SkRegion two;
    debugster("empty", one, two);
    one.setRect({1, 2, 3, 4});
    two.setRect({1, 2, 3, 3});
    debugster("set rect", one, two);
    two.op({1, 3, 3, 4}, SkRegion::kUnion_Op);
    debugster("union rect", one, two);
}
}  // END FIDDLE
