/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTextUtils.h"

void SkTextUtils::DrawText(SkCanvas* canvas, const void* text, size_t size, SkScalar x, SkScalar y,
                            const SkPaint& origPaint, Align align) {
    int count = origPaint.countText(text, size);
    if (!count) {
        return;
    }

    SkPaint paint(origPaint);
    SkAutoSTArray<32, uint16_t> glyphStorage;
    const uint16_t* glyphs;

    if (paint.getTextEncoding() != SkPaint::kGlyphID_TextEncoding) {
        glyphStorage.reset(count);
        paint.textToGlyphs(text, size, glyphStorage.get());
        glyphs = glyphStorage.get();
        paint.setTextEncoding(SkPaint::kGlyphID_TextEncoding);
    } else {
        glyphs = static_cast<const uint16_t*>(text);
    }

    SkAutoSTArray<32, SkScalar> widthStorage(count);
    SkScalar* widths = widthStorage.get();
    paint.getTextWidths(glyphs, count * sizeof(uint16_t), widths);

    if (align != kLeft_Align) {
        SkScalar offset = 0;
        for (int i = 0; i < count; ++i) {
            offset += widths[i];
        }
        if (align == kCenter_Align) {
            offset *= 0.5f;
        }
        x -= offset;
    }

    // Turn widths into h-positions
    for (int i = 0; i < count; ++i) {
        SkScalar w = widths[i];
        widths[i] = x;
        x += w;
    }
    canvas->drawPosTextH(glyphs, count * sizeof(uint16_t), widths, y, paint);
}

