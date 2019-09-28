// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=a7ac9c21bbabcdeeca00f72a61cd0f3e
REG_FIDDLE(Canvas_accessTopLayerPixels_b, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
  SkPaint paint;
  SkFont font(nullptr, 100);
  canvas->drawString("ABC", 20, 160, font, paint);
  SkRect layerBounds = SkRect::MakeXYWH(32, 32, 192, 192);
  canvas->saveLayerAlpha(&layerBounds, 128);
  canvas->clear(SK_ColorWHITE);
  canvas->drawString("DEF", 20, 160, font, paint);
  SkImageInfo imageInfo;
  size_t rowBytes;
  SkIPoint origin;
  uint32_t* access = (uint32_t*) canvas->accessTopLayerPixels(&imageInfo, &rowBytes, &origin);
  if (access) {
    int h = imageInfo.height();
    int v = imageInfo.width();
    int rowWords = rowBytes / sizeof(uint32_t);
    for (int y = 0; y < h; ++y) {
        int newY = (y - h / 2) * 2 + h / 2;
        if (newY < 0 || newY >= h) {
            continue;
        }
        for (int x = 0; x < v; ++x) {
            int newX = (x - v / 2) * 2 + v / 2;
            if (newX < 0 || newX >= v) {
                continue;
            }
            if (access[y * rowWords + x] == SK_ColorBLACK) {
                access[newY * rowWords + newX] = SK_ColorGRAY;
            }
        }
    }
  }
  canvas->restore();
}
}  // END FIDDLE
