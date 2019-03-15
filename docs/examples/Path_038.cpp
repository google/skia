// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=2ec66880966a6133ddd9331ce7323438
REG_FIDDLE(Path_038, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    auto debugster = [](const char* prefix, const SkPath& path, uint8_t* verbs, int max) -> void {
         int count = path.getVerbs(verbs, max);
         SkDebugf("%s verb count: %d  ", prefix, count);
         const char* verbStr[] = { "move", "line", "quad", "conic", "cubic", "close" };
         for (int i = 0; i < SkTMin(count, max) && verbs; ++i) {
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
