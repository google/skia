// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=85496614e90c66b020f8a70db8d06f4a
REG_FIDDLE(Canvas_062, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkCanvas local(256, 256);
    canvas = &local;
    SkRect bounds;
    SkDebugf("local bounds empty = %s\n", canvas->getLocalClipBounds(&bounds)
             ? "false" : "true");
    SkPath path;
    canvas->clipPath(path);
    SkDebugf("local bounds empty = %s\n", canvas->getLocalClipBounds(&bounds)
             ? "false" : "true");
}
}  // END FIDDLE
