// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=39016b3cfc6bbabb09348a53822ce508
REG_FIDDLE(Matrix_094, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    auto debugster = [](const char* prefix, const SkMatrix& a, const SkMatrix& b) -> void {
        SkDebugf("%s: a %c= b a.cheapEqualTo(b): %s\n", prefix,
                 a == b ? '=' : '!', a.cheapEqualTo(b) ? "true" : "false");
    };
    SkMatrix a, b;
    a.setAll(1, 0, 0,   0, 1, 0,  0, 0, 1);
    b.setIdentity();
    debugster("identity", a, b);
    a.setAll(1, -0.0f, 0,   0, 1, 0,  0, 0, 1);
    debugster("neg zero", a, b);
    a.setAll(1, SK_ScalarNaN, 0,   0, 1, 0,  0, 0, 1);
    debugster(" one NaN", a, b);
    b.setAll(1, SK_ScalarNaN, 0,   0, 1, 0,  0, 0, 1);
    debugster("both NaN", a, b);
}
}  // END FIDDLE
