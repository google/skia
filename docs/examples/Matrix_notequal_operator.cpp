// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=088ab41f877599f980a99523749b0afd
REG_FIDDLE(Matrix_notequal_operator, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    auto debugster = [](const char* prefix, const SkMatrix& a, const SkMatrix& b) -> void {
        SkDebugf("%s: a %c= b a.cheapEqualTo(b): %s\n", prefix,
                 a != b ? '!' : '=', a.cheapEqualTo(b) ? "true" : "false");
    };
    SkMatrix a, b;
    a.setAll(1, 0, 0,   0, 1, 0,  1, 0, 1);
    if (a.invert(&b)) {
        debugster("identity", a, b);
    }
}
}  // END FIDDLE
