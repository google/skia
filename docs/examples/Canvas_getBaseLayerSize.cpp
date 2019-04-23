// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=374e245d91cd729eca48fd20e631fdf3
REG_FIDDLE(Canvas_getBaseLayerSize, 256, 256, true, 0) {
void draw(SkCanvas* ) {
    SkBitmap bitmap;
    bitmap.allocPixels(SkImageInfo::MakeN32Premul(20, 30));
    SkCanvas canvas(bitmap, SkSurfaceProps(0, kUnknown_SkPixelGeometry));
    canvas.clipRect(SkRect::MakeWH(10, 40));
    SkIRect clipDeviceBounds = canvas.getDeviceClipBounds();
    if (clipDeviceBounds.isEmpty()) {
        SkDebugf("Empty clip bounds is unexpected!\n");
    }
    SkDebugf("clip=%d,%d\n", clipDeviceBounds.width(), clipDeviceBounds.height());
    SkISize baseLayerSize = canvas.getBaseLayerSize();
    SkDebugf("size=%d,%d\n", baseLayerSize.width(), baseLayerSize.height());
}
}  // END FIDDLE
