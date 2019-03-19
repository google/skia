// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=647312aacd946c8a6eabaca797140432
REG_FIDDLE(Path_copy_const_SkPath, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPath path;
    path.lineTo(20, 20);
    SkPath path2(path);
    path2.close();
    SkDebugf("path verbs: %d\n", path.countVerbs());
    SkDebugf("path2 verbs: %d\n", path2.countVerbs());
    path.reset();
    SkDebugf("after reset\n" "path verbs: %d\n", path.countVerbs());
    SkDebugf("path2 verbs: %d\n", path2.countVerbs());
}
}  // END FIDDLE
