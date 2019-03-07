// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=03b740ab94b9017800a52e30b5e7fee7
REG_FIDDLE(Path_026, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    auto debugster = [](const char* prefix, const SkPath& path) -> void {
        SkDebugf("%s last contour is %s" "closed\n", prefix,
                 path.isLastContourClosed() ? "" : "not ");
    };
    SkPath path;
    debugster("initial", path);
    path.close();
    debugster("after close", path);
    path.lineTo(0, 0);
    debugster("after lineTo", path);
    path.close();
    debugster("after close", path);
}
}  // END FIDDLE
