// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=cd203a3f9c5fb68272f21f302dd54fbc
REG_FIDDLE(ImageInfo_036, 256, 144, false, 3) {
void draw(SkCanvas* canvas) {
    SkImageInfo canvasImageInfo = canvas->imageInfo();
    SkRect canvasBounds = SkRect::Make(canvasImageInfo.bounds());
    canvas->drawBitmapRect(source, source.bounds(), canvasBounds, nullptr);
    SkImageInfo insetImageInfo =
              canvasImageInfo.makeWH(canvasBounds.width() / 2, canvasBounds.height() / 2);
    SkBitmap inset;
    inset.allocPixels(insetImageInfo);
    SkCanvas offscreen(inset);
    offscreen.drawBitmapRect(source, source.bounds(), SkRect::Make(inset.bounds()), nullptr);
    canvas->drawBitmap(inset, canvasBounds.width() / 4, canvasBounds.height() / 4);
}
}  // END FIDDLE
