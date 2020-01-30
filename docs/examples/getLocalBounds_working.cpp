// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(getLocalBounds_working, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkCanvas local(256, 256);
    canvas = &local;
    SkRect bounds = canvas->getLocalClipBounds();
    SkDebugf("left:%g  top:%g  right:%g  bottom:%g\n", bounds.fLeft, bounds.fTop, bounds.fRight,
             bounds.fBottom);
    SkPoint clipPoints[] = {{30, 130}, {120, 130}, {120, 230}};
    SkPath clipPath;
    clipPath.addPoly(clipPoints, SK_ARRAY_COUNT(clipPoints), true);
    canvas->clipPath(clipPath);
    bounds = canvas->getLocalClipBounds();
    SkDebugf("left:%g  top:%g  right:%g  bottom:%g\n", bounds.fLeft, bounds.fTop, bounds.fRight,
             bounds.fBottom);
}
}  // END FIDDLE
