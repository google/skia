// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=31883f51bb357f2ac5990d88f8b82e02
REG_FIDDLE(Path_equal_operator, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    auto debugster = [](const char* prefix, const SkPath& a, const SkPath& b) -> void {
                SkDebugf("%s one %c= two\n", prefix, a == b ? '=' : '!');
    };
    SkPath one;
    SkPath two;
    debugster("empty", one, two);
    one.moveTo(0, 0);
    debugster("moveTo", one, two);
    one.rewind();
    debugster("rewind", one, two);
    one.moveTo(0, 0);
    one.reset();
    debugster("reset", one, two);
}
}  // END FIDDLE
