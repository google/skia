/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkRect.h"
#include "tools/Resources.h"

// https://bug.skia.org/4374
DEF_SIMPLE_GM(draw_bitmap_rect_skbug4734, canvas, 64, 64) {
    SkBitmap source;
    if (GetResourceAsBitmap("images/randPixels.png", &source)) {
        SkRect rect = SkRect::Make(source.bounds());
        rect.inset(0.5, 1.5);
        SkRect dst;
        SkMatrix::MakeScale(8.0f).mapRect(&dst, rect);
        canvas->drawBitmapRect(source, rect, dst, nullptr);
    }
}
