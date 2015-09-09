/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkMagnifierImageFilter.h"
#include "SkRandom.h"

#define WIDTH 500
#define HEIGHT 500

DEF_SIMPLE_GM_BG(imagemagnifier, canvas, WIDTH, HEIGHT, SK_ColorBLACK) {
        SkPaint filterPaint;
        filterPaint.setImageFilter(
            SkMagnifierImageFilter::Create(
                SkRect::MakeXYWH(SkIntToScalar(100), SkIntToScalar(100),
                                 SkIntToScalar(WIDTH / 2),
                                 SkIntToScalar(HEIGHT / 2)),
                100))->unref();
        canvas->saveLayer(nullptr, &filterPaint);
        const char* str = "The quick brown fox jumped over the lazy dog.";
        SkRandom rand;
        for (int i = 0; i < 25; ++i) {
            int x = rand.nextULessThan(WIDTH);
            int y = rand.nextULessThan(HEIGHT);
            SkPaint paint;
            sk_tool_utils::set_portable_typeface(&paint);
            paint.setColor(sk_tool_utils::color_to_565(rand.nextBits(24) | 0xFF000000));
            paint.setTextSize(rand.nextRangeScalar(0, 300));
            paint.setAntiAlias(true);
            canvas->drawText(str, strlen(str), SkIntToScalar(x),
                             SkIntToScalar(y), paint);
        }
        canvas->restore();
}
