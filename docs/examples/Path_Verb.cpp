// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Path_Verb, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPath path = SkPathBuilder()
                  .lineTo(20, 20)
                  .quadTo(-10, -10, 30, 30)
                  .close()
                  .cubicTo(1, 2, 3, 4, 5, 6)
                  .conicTo(0, 0, 0, 0, 2)
                  .detach();
    uint8_t verbs[7];
    int count = path.getVerbs(verbs);
    const char* verbStr[] = { "Move", "Line", "Quad", "Conic", "Cubic", "Close" };
    SkDebugf("verb count: %d\nverbs: ", count);
    for (int i = 0; i < count; ++i) {
        SkDebugf("k%s_Verb ", verbStr[verbs[i]]);
    }
    SkDebugf("\n");
}
}  // END FIDDLE
