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
    if (auto img = GetResourceAsImage("images/randPixels.png")) {
        SkRect rect = SkRect::Make(img->bounds());
        rect.inset(0.5, 1.5);
        SkRect dst;
        SkMatrix::Scale(8, 8).mapRect(&dst, rect);
        canvas->drawImageRect(img, rect, dst, SkSamplingOptions(), nullptr,
                              SkCanvas::kStrict_SrcRectConstraint);
    }
}
