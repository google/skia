// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=3902859150b0f0c4aeb1f25d00434baa
REG_FIDDLE(Matrix_equal_operator, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    auto debugster = [](const char* prefix, const SkMatrix& a, const SkMatrix& b) -> void {
        SkDebugf("%s: a %c= b a.cheapEqualTo(b): %s\n", prefix,
                 a == b ? '=' : '!', a.cheapEqualTo(b) ? "true" : "false");
    };
    SkMatrix a, b;
    a.setAll(1, 0, 0,   0, 1, 0,  0, 0, 1);
    b.setScale(2, 4);
    b.postScale(0.5f, 0.25f);
    debugster("identity", a, b);
}
}  // END FIDDLE
