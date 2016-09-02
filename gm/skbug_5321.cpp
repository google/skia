/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

// https://bugs.skia.org/5321
// two strings should draw the same.  PDF did not.
DEF_SIMPLE_GM(skbug_5321, canvas, 128, 128) {
    SkPaint paint;
    paint.setStyle(SkPaint::kFill_Style);
    paint.setTextSize(30);
    
    paint.setTextEncoding(SkPaint::kUTF8_TextEncoding);
    const char text[] = "x\314\200y";  // utf8(u"x\u0300y")
    SkScalar x = 20, y = 45;
    size_t byteLength = strlen(text);
    canvas->drawText(text, byteLength, x, y, paint);
    
    int glyph_count = paint.countText(text, byteLength);
    SkAutoTMalloc<SkScalar> widths(glyph_count);
    (void)paint.getTextWidths(text, byteLength, &widths[0]);
    for (int i = 0; i < glyph_count; ++i) {
        SkScalar w = widths[i];
        widths[i] = x;
        x += w;
    }
    y += paint.getFontMetrics(nullptr);
    canvas->drawPosTextH(text, byteLength, &widths[0], y, paint);
}
