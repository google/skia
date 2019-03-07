// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=799096fdc1298aa815934a74e76570ca
REG_FIDDLE(Path_000, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPath path;
    path.lineTo(20, 20);
    path.quadTo(-10, -10, 30, 30);
    path.close();
    path.cubicTo(1, 2, 3, 4, 5, 6);
    path.conicTo(0, 0, 0, 0, 2);
    uint8_t verbs[7];
    int count = path.getVerbs(verbs, (int) SK_ARRAY_COUNT(verbs));
    const char* verbStr[] = { "Move", "Line", "Quad", "Conic", "Cubic", "Close" };
    SkDebugf("verb count: %d\nverbs: ", count);
    for (int i = 0; i < count; ++i) {
        SkDebugf("k%s_Verb ", verbStr[verbs[i]]);
    }
    SkDebugf("\n");
}
}  // END FIDDLE
