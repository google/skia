// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=cd203a3f9c5fb68272f21f302dd54fbc
REG_FIDDLE(ImageInfo_makeWH, 256, 144, false, 3) {
void draw(SkCanvas* canvas) {
    SkImageInfo canvasImageInfo = canvas->imageInfo();
    SkRect canvasBounds = SkRect::Make(canvasImageInfo.bounds());
    canvas->drawImageRect(image, SkRect::Make(source.bounds()), canvasBounds, SkSamplingOptions(),
                          nullptr, SkCanvas::kStrict_SrcRectConstraint);
    SkImageInfo insetImageInfo =
              canvasImageInfo.makeWH(canvasBounds.width() / 2, canvasBounds.height() / 2);
    SkBitmap inset;
    inset.allocPixels(insetImageInfo);
    SkCanvas offscreen(inset);
    offscreen.drawImageRect(image, SkRect::Make(source.bounds()), SkRect::Make(inset.bounds()),
                            SkSamplingOptions(), nullptr, SkCanvas::kStrict_SrcRectConstraint);
    canvas->drawImage(inset.asImage(), canvasBounds.width() / 4, canvasBounds.height() / 4);
}
}  // END FIDDLE
