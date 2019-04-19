// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=8b26507690b71462f44642b911890bbf
REG_FIDDLE(Dither_a, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkBitmap bm16;
    bm16.allocPixels(SkImageInfo::Make(32, 32, kRGB_565_SkColorType, kOpaque_SkAlphaType));
    SkCanvas c16(bm16);
    SkPaint colorPaint;
    for (auto dither : { false, true } ) {
        colorPaint.setDither(dither);
        for (auto colors : { 0xFF333333, 0xFF666666, 0xFF999999, 0xFFCCCCCC } ) {
            for (auto mask : { 0xFFFF0000, 0xFF00FF00, 0xFF0000FF, 0xFFFFFFFF } ) {
                 colorPaint.setColor(colors & mask);
                 c16.drawRect({0, 0, 8, 4}, colorPaint);
                 c16.translate(8, 0);
            }
            c16.translate(-32, 4);
        }
    }
    canvas->scale(8, 8);
    canvas->drawBitmap(bm16, 0, 0);
}
}  // END FIDDLE
