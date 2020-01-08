/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"

DEF_SIMPLE_GM(bicubic, canvas, 300, 64) {
    canvas->clear(SK_ColorBLACK);

    SkBitmap bmp;
    bmp.allocN32Pixels(8, 1);
    bmp.eraseColor(0);
    *bmp.getAddr32(3, 0) = 0xFFFFFFFF;

    SkPaint paint;
    paint.setFilterQuality(kHigh_SkFilterQuality);

    for (int i = 0; i < 64; ++i) {
        float x = 1.0f + i/63.0f;
        float y = i;
        canvas->drawBitmapRect(bmp, SkRect::MakeXYWH(x, y, 512, 1), &paint);
    }
}
