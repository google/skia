/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTextUtils.h"

void SkTextUtils::DrawText(SkCanvas* canvas, const void* text, size_t size, SkScalar x, SkScalar y,
                            const SkPaint& paint, SkPaint::Align align) {
    int count = paint.countText(text, size);
    if (!count) {
        return;
    }

    SkAutoSTArray<32, uint16_t> storage(count);
    uint16_t* glyphs = storage.get();
    paint.textToGlyphs(text, size, glyphs);

    SkAutoSTArray<32, SkScalar> widthStorage(count);
    SkScalar* widths = widthStorage.get();
    paint.getTextWidths(glyphs, count * sizeof(uint16_t), widths);

    if (align != SkPaint::kLeft_Align) {
        SkScalar offset = 0;
        for (int i = 0; i < count; ++i) {
            offset += widths[i];
        }
        if (align == SkPaint::kCenter_Align) {
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
    canvas->drawPosTextH(glyphs, count, widths, y, paint);
}

