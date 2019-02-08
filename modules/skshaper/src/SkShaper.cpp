/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkShaper.h"
#include "SkShaperPriv.h"

#include "SkTextBlobPriv.h"
#include "SkFontMetrics.h"
#include "SkStream.h"
#include "SkTo.h"
#include "SkTypeface.h"

SkShaper::RunHandler::Buffer SkTextBlobBuilderRunHandler::newRunBuffer(const RunInfo&,
                                                                       const SkFont& font,
                                                                       int glyphCount,
                                                                       int textCount) {
    const auto& runBuffer = SkTextBlobBuilderPriv::AllocRunTextPos(&fBuilder, font, glyphCount,
                                                                   textCount, SkString());
    return { runBuffer.glyphs,
             runBuffer.points(),
             runBuffer.utf8text,
             runBuffer.clusters };
}

sk_sp<SkTextBlob> SkTextBlobBuilderRunHandler::makeBlob() {
    return fBuilder.make();
}

// This example only uses public API, so we don't use SkUTF8_NextUnichar.
unsigned utf8_lead_byte_to_count(const char* ptr) {
    uint8_t c = *(const uint8_t*)ptr;
    SkASSERT(c <= 0xF7);
    SkASSERT((c & 0xC0) != 0x80);
    return (((0xE5 << 24) >> ((unsigned)c >> 4 << 1)) & 3) + 1;
}

SkPoint ShapePrimitive(SkShaper::RunHandler* handler,
                       const SkFont& font,
                       const char* utf8text,
                       size_t textBytes,
                       bool leftToRight,
                       SkPoint point,
                       SkScalar width) {
    sk_ignore_unused_variable(leftToRight);
    sk_ignore_unused_variable(width);

    int glyphCount = font.countText(utf8text, textBytes, SkTextEncoding::kUTF8);
    if (glyphCount <= 0) {
        return point;
    }

    SkFontMetrics metrics;
    font.getMetrics(&metrics);
    point.fY -= metrics.fAscent;

    const SkShaper::RunHandler::RunInfo info = {
        { font.measureText(utf8text, textBytes, SkTextEncoding::kUTF8), 0 },
        metrics.fAscent,
        metrics.fDescent,
        metrics.fLeading,
    };
    const auto buffer = handler->newRunBuffer(info, font, glyphCount, textBytes);
    SkAssertResult(font.textToGlyphs(utf8text, textBytes, SkTextEncoding::kUTF8, buffer.glyphs,
                                     glyphCount) == glyphCount);
    font.getPos(buffer.glyphs, glyphCount, buffer.positions, point);

    if (buffer.utf8text) {
        memcpy(buffer.utf8text, utf8text, textBytes);
    }

    if (buffer.clusters) {
        const char* txtPtr = utf8text;
        for (int i = 0; i < glyphCount; ++i) {
            // Each charater maps to exactly one glyph via SkGlyphCache::unicharToGlyph().
            buffer.clusters[i] = SkToU32(txtPtr - utf8text);
            txtPtr += utf8_lead_byte_to_count(txtPtr);
            SkASSERT(txtPtr <= utf8text + textBytes);
        }
    }

    handler->commitLine();

    return point + SkVector::Make(0, metrics.fDescent + metrics.fLeading);
}
