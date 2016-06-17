/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkBlurImageFilter.h"
#include "SkRandom.h"

#define WIDTH 500
#define HEIGHT 500

void imageblurgm_draw(SkScalar fSigmaX, SkScalar fSigmaY, SkCanvas* canvas) {
        SkPaint paint;
        paint.setImageFilter(SkBlurImageFilter::Make(fSigmaX, fSigmaY, nullptr));
        canvas->saveLayer(nullptr, &paint);
        const char* str = "The quick brown fox jumped over the lazy dog.";

        SkRandom rand;
        SkPaint textPaint;
        textPaint.setAntiAlias(true);
        sk_tool_utils::set_portable_typeface(&textPaint);
        for (int i = 0; i < 25; ++i) {
            int x = rand.nextULessThan(WIDTH);
            int y = rand.nextULessThan(HEIGHT);
            textPaint.setColor(sk_tool_utils::color_to_565(rand.nextBits(24) | 0xFF000000));
            textPaint.setTextSize(rand.nextRangeScalar(0, 300));
            canvas->drawText(str, strlen(str), SkIntToScalar(x),
                             SkIntToScalar(y), textPaint);
        }
        canvas->restore();
}
DEF_SIMPLE_GM_BG(imageblur,       canvas, WIDTH, HEIGHT, SK_ColorBLACK) {
    imageblurgm_draw(24.0f, 0.0f, canvas);
}
DEF_SIMPLE_GM_BG(imageblur_large, canvas, WIDTH, HEIGHT, SK_ColorBLACK) {
    imageblurgm_draw(80.0f, 80.0f, canvas);
}
