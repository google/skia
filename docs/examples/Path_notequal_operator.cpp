// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=bbbda1cc818d96c9c0d2a06c0c48902b
REG_FIDDLE(Path_notequal_operator, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    auto debugster = [](const char* prefix, const SkPath& a, const SkPath& b) -> void {
                SkDebugf("%s one %c= two\n", prefix, a != b ? '!' : '=');
    };
    SkPath one;
    SkPath two;
    debugster("empty", one, two);
    one.addRect({10, 20, 30, 40});
    two.addRect({10, 20, 30, 40});
    debugster("add rect", one, two);
    one.setConvexity(SkPath::kConcave_Convexity);
    debugster("setConvexity", one, two);
    SkDebugf("convexity %c=\n", one.getConvexity() == two.getConvexity() ? '=' : '!');
}
}  // END FIDDLE
