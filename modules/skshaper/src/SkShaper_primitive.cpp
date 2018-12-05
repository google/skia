/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkShaper.h"

#include "SkStream.h"
#include "SkTextBlob.h"
#include "SkTo.h"
#include "SkTypeface.h"

struct SkShaper::Impl {
    sk_sp<SkTypeface> fTypeface;
};

SkShaper::SkShaper(sk_sp<SkTypeface> tf) : fImpl(new Impl) {
    fImpl->fTypeface = tf ? std::move(tf) : SkTypeface::MakeDefault();
}

SkShaper::~SkShaper() {}

bool SkShaper::good() const { return true; }

// This example only uses public API, so we don't use SkUTF8_NextUnichar.
unsigned utf8_lead_byte_to_count(const char* ptr) {
    uint8_t c = *(const uint8_t*)ptr;
    SkASSERT(c <= 0xF7);
    SkASSERT((c & 0xC0) != 0x80);
    return (((0xE5 << 24) >> ((unsigned)c >> 4 << 1)) & 3) + 1;
}

SkPoint SkShaper::shape(SkTextBlobBuilder* builder,
                        const SkFont& srcFont,
                        const char* utf8text,
                        size_t textBytes,
                        bool leftToRight,
                        SkPoint point,
                        SkScalar width) const {
    sk_ignore_unused_variable(leftToRight);

    SkFont font(srcFont);
    font.setTypeface(fImpl->fTypeface);
    int glyphCount = font.countText(utf8text, textBytes, SkTextEncoding::kUTF8);
    if (glyphCount <= 0) {
        return point;
    }
    SkRect bounds;
    SkFontMetrics metrics;
    font.getMetrics(&metrics);
    point.fY -= metrics.fAscent;
    (void)font.measureText(utf8text, textBytes, SkTextEncoding::kUTF8, &bounds);
    const SkTextBlobBuilder::RunBuffer& runBuffer =
        builder->allocRunTextPosH(font, glyphCount, point.y(), textBytes, SkString(), &bounds);
    memcpy(runBuffer.utf8text, utf8text, textBytes);
    const char* txtPtr = utf8text;
    for (int i = 0; i < glyphCount; ++i) {
        // Each charater maps to exactly one glyph via SkGlyphCache::unicharToGlyph().
        runBuffer.clusters[i] = SkToU32(txtPtr - utf8text);
        txtPtr += utf8_lead_byte_to_count(txtPtr);
        SkASSERT(txtPtr <= utf8text + textBytes);
    }
    (void)font.textToGlyphs(utf8text, textBytes, SkTextEncoding::kUTF8,
                            runBuffer.glyphs, glyphCount);
    // replace with getPos()?
    (void)font.getWidths(runBuffer.glyphs, glyphCount, runBuffer.pos);
    SkScalar x = point.x();
    for (int i = 0; i < glyphCount; ++i) {
        SkScalar w = runBuffer.pos[i];
        runBuffer.pos[i] = x;
        x += w;
    }
    point.fY += metrics.fDescent + metrics.fLeading;

    return point;
}
