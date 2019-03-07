// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=2d991a231e49d1de13eeb2ba9b440e01
REG_FIDDLE(Surface_023, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    sk_sp<SkSurface> surf(SkSurface::MakeRasterN32Premul(64, 64));
    auto surfCanvas = surf->getCanvas();
    surfCanvas->clear(SK_ColorGREEN);
    SkPaint paint;
    surfCanvas->drawOval({2, 10, 58, 54}, paint);
    SkImageInfo info = SkImageInfo::Make(64, 64, kBGRA_8888_SkColorType, kPremul_SkAlphaType);
    SkBitmap bitmap;
    bitmap.setInfo(info);
    bitmap.allocPixels();
    for (int x : { 32, -32 } ) {
        for (int y : { 32, -32 } ) {
            surf->readPixels(bitmap, x, y);
        }
    }
    canvas->drawBitmap(bitmap, 0, 0);
}
}  // END FIDDLE
