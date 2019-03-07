// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=56dcd14f943aea6f7d7aafe0de7e6c25
REG_FIDDLE(Canvas_060, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPoint testPoints[] = {{30,  30}, {120,  30}, {120, 129} };
    SkPoint clipPoints[] = {{30, 130}, {120, 130}, {120, 230} };
    SkPath testPath, clipPath;
    testPath.addPoly(testPoints, SK_ARRAY_COUNT(testPoints), true);
    clipPath.addPoly(clipPoints, SK_ARRAY_COUNT(clipPoints), true);
    canvas->save();
    canvas->clipPath(clipPath);
    SkDebugf("quickReject %s\n", canvas->quickReject(testPath) ? "true" : "false");
    canvas->restore();
    canvas->rotate(10);
    canvas->clipPath(clipPath);
    SkDebugf("quickReject %s\n", canvas->quickReject(testPath) ? "true" : "false");
}
}  // END FIDDLE
