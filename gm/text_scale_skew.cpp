/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

// http://bug.skia.org/7315
DEF_SIMPLE_GM(text_scale_skew, canvas, 256, 128) {
    SkPaint p;
    p.setTextSize(18.0f);
    p.setAntiAlias(true);
    p.setTextAlign(SkPaint::kCenter_Align);
    float y = 10.0f;
    for (float scale : { 0.5f, 0.71f, 1.0f, 1.41f, 2.0f }) {
        p.setTextScaleX(scale);
        y += p.getFontSpacing();
        float x = 50.0f;
        for (float skew : { -0.5f, 0.0f, 0.5f }) {
            p.setTextSkewX(skew);
            canvas->drawString("Skia", x, y, p);
            x += 78.0f;
        }
    }
}
