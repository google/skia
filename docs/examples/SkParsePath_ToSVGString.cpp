// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(SkParsePath_ToSVGString, 256, 256, true, 0) {
SkPath star() {
    const SkScalar R = 115.2f, C = 128.0f;
    SkPath path;
    path.moveTo(sk_float_round(C + R), sk_float_round(C));
    for (int i = 1; i < 8; ++i) {
        SkScalar a = 2.6927937f * i;
        path.lineTo(sk_float_round(C + R * cos(a)), sk_float_round(C + R * sin(a)));
    }
    return path;
}

void draw(SkCanvas* canvas) {
    SkDebugf("%s\n", SkParsePath::ToSVGString(star()).c_str());
}
}  // END FIDDLE
