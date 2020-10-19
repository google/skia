/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "skia.h"

static void draw(SkCanvas* canvas);
DEF_SIMPLE_GM(fiddle, canvas, 256, 256) { draw(canvas); }

// Paste your fiddle.skia.org code over this stub.
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setColor(SK_ColorBLUE);
    paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 35));
    SkRRect rrect = SkRRect::MakeRectXY(SkRect::MakeLTRB(SkBits2Float(0x430d33c9), /* 141.202286 */
      -133689.734375 * 1/10.f,
     SkBits2Float(0x45368a87), /* 2920.657959 */
     SkBits2Float(0x41cd4000)  /* 25.656250 */), 38.f, 38.f);
   canvas->drawRRect(rrect, paint);
}
