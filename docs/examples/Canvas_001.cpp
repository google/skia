// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=87f55e62ec4c3535e1a5d0f1415b20c6
REG_FIDDLE(Canvas_MakeRasterDirectN32, 256, 256, true, 0) {
void draw(SkCanvas* ) {
    const int width = 3;
    const int height = 3;
    SkPMColor pixels[height][width];  // allocate a 3 by 3 Premultiplied bitmap on the stack
    // create a SkCanvas backed by a raster device, and delete it when the
    // function goes out of scope.
    std::unique_ptr<SkCanvas> canvas = SkCanvas::MakeRasterDirectN32(
            width,
            height,
            pixels[0],  // top-left of the bitmap
            sizeof(pixels[0]));  // byte width of the each row
    // write a Premultiplied value for white into all pixels in the bitmap
    canvas->clear(SK_ColorWHITE);
    SkPMColor pmWhite = pixels[0][0];  // the Premultiplied format may vary
    SkPaint paint;  // by default, draws black
    canvas->drawPoint(1, 1, paint);  // draw in the center
    canvas->flush();  // ensure that pixels is ready to be read
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            SkDebugf("%c", pixels[y][x] == pmWhite ? '-' : 'x');
        }
        SkDebugf("\n");
    }
}
}  // END FIDDLE
