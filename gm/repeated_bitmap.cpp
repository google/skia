/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"
#include "Resources.h"

DEF_SIMPLE_GM(repeated_bitmap, canvas, 576, 576) {
    sk_tool_utils::draw_checkerboard(canvas, sk_tool_utils::color_to_565(0xFF999999),
            SK_ColorWHITE, 12);
    SkRect rect = SkRect::MakeLTRB(-4.25f, -4.25f, 4.25f, 4.25f);
    SkPaint paint;
    paint.setColor(0xFF333333);
    SkBitmap bm;
    if (GetResourceAsBitmap("randPixels.png", &bm)) {
        for (int j = 0; j < 4; ++j) {
            for (int i = 0; i < 4; ++i) {
                SkAutoCanvasRestore autoCanvasRestore(canvas, true);
                canvas->scale(12.0f, 12.0f);
                canvas->translate(6.0f + 12.0f * SkIntToScalar(i),
                                  6.0f + 12.0f * SkIntToScalar(j));
                canvas->rotate(18.0f * (i + 4 * j));
                canvas->drawRect(rect, paint);
                canvas->drawBitmap(bm, -4.0f, -4.0f);
            }
        }
    }
}

DEF_SIMPLE_GM(repeated_bitmap_jpg, canvas, 576, 576) {
    sk_tool_utils::draw_checkerboard(canvas, sk_tool_utils::color_to_565(0xFF999999),
            SK_ColorWHITE, 12);
    SkRect rect = SkRect::MakeLTRB(-68.0f, -68.0f, 68.0f, 68.0f);
    SkPaint paint;
    paint.setColor(0xFF333333);
    SkBitmap bm;
    if (GetResourceAsBitmap("color_wheel.jpg", &bm)) {
        for (int j = 0; j < 4; ++j) {
            for (int i = 0; i < 4; ++i) {
                SkAutoCanvasRestore autoCanvasRestore(canvas, true);
                canvas->translate(96.0f + 192.0f * SkIntToScalar(i),
                                  96.0f + 192.0f * SkIntToScalar(j));
                canvas->rotate(18.0f * (i + 4 * j));
                canvas->drawRect(rect, paint);
                canvas->drawBitmap(bm, -64.0f, -64.0f);
            }
        }
    }
}
