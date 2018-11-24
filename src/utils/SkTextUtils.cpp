/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTextUtils.h"
#include "SkPath.h"

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

void SkTextUtils::GetTextPath(const SkFont& font, const void* text, size_t length,
                              SkTextEncoding encoding, SkScalar x, SkScalar y, SkPath* path) {
    int count;
    SkAutoTArray<uint16_t> glyphsStorage;
    const uint16_t* glyphs;

    if (encoding == kGlyphID_SkTextEncoding) {
        count = length >> 1;
        glyphs = static_cast<const uint16_t*>(text);
    } else {
        count = font.countText(text, length, encoding);
        glyphsStorage.reset(count);
        font.textToGlyphs(text, length, encoding, glyphsStorage.get(), count);
        glyphs = glyphsStorage.get();
    }
    SkAutoTArray<SkScalar> advStorage(count);
    SkScalar* adv = advStorage.get();
    font.getWidths(glyphs, count, adv, nullptr);

    struct Rec {
        SkPath* fPath;
        SkScalar* fAdv;
        SkPoint fPos;
    } rec = {
        path, adv, { x, y }
    };

    path->reset();
    font.getPaths(glyphs, count, [](uint16_t, const SkPath* src, void* ctx) {
        Rec* rec = static_cast<Rec*>(ctx);
        if (src) {
            rec->fPath->addPath(*src, rec->fPos.fX, rec->fPos.fY);
        }
        rec->fPos.fX += *rec->fAdv++;
    }, &rec);
}
