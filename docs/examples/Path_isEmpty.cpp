// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=0b34e6d55d11586744adeb889d2a12f4
REG_FIDDLE(Path_isEmpty, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    auto debugster = [](const char* prefix, const SkPath& path) -> void {
        SkDebugf("%s path is %s" "empty\n", prefix, path.isEmpty() ? "" : "not ");
    };
    SkPath path;
    debugster("initial", path);
    path.moveTo(0, 0);
    debugster("after moveTo", path);
    path.rewind();
    debugster("after rewind", path);
    path.lineTo(0, 0);
    debugster("after lineTo", path);
    path.reset();
    debugster("after reset", path);
}
}  // END FIDDLE
