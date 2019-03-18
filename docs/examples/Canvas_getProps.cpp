// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=0fbf2dedc2619bbfbf173c9e3bc1a508
REG_FIDDLE(Canvas_getProps, 256, 256, true, 0) {
void draw(SkCanvas* ) {
    SkBitmap bitmap;
    bitmap.allocN32Pixels(1, 1);
    SkCanvas canvas(bitmap, SkSurfaceProps(0, kRGB_V_SkPixelGeometry));
    SkSurfaceProps surfaceProps(0, kUnknown_SkPixelGeometry);
    SkDebugf("isRGB:%d\n", SkPixelGeometryIsRGB(surfaceProps.pixelGeometry()));
    if (!canvas.getProps(&surfaceProps)) {
        SkDebugf("getProps failed unexpectedly.\n");
    }
    SkDebugf("isRGB:%d\n", SkPixelGeometryIsRGB(surfaceProps.pixelGeometry()));
}
}  // END FIDDLE
