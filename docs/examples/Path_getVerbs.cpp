// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Path_getVerbs, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    auto debugster = [](const char* prefix, const SkPath& path, uint8_t* verbs, int max) -> void {
        int count = path.getVerbs({verbs, max});
         SkDebugf("%s verb count: %d  ", prefix, count);
         const char* verbStr[] = { "move", "line", "quad", "conic", "cubic", "close" };
         for (int i = 0; i < std::min(count, max) && verbs; ++i) {
             SkDebugf("%s ", verbStr[verbs[i]]);
         }
         SkDebugf("\n");
    };
    SkPath path;
    path.lineTo(20, 20);
    path.lineTo(-10, -10);
    uint8_t verbs[3];
    debugster("no verbs",  path, nullptr, 0);
    debugster("zero max",  path, verbs, 0);
    debugster("too small",  path, verbs, 2);
    debugster("just right",  path, verbs, path.countVerbs());
}
}  // END FIDDLE
