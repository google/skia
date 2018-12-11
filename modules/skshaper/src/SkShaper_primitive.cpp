/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkShaper.h"

#include "SkFont.h"
#include "SkFontMetrics.h"
#include "SkStream.h"
#include "SkTArray.h"
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

SkPoint SkShaper::shape(LineHandler* handler,
                        const SkFont& srcFont,
                        const char* utf8text,
                        size_t textBytes,
                        bool leftToRight,
                        SkPoint point,
                        SkScalar width) const {
    sk_ignore_unused_variable(leftToRight);
    sk_ignore_unused_variable(width);

    SkFont font(srcFont);
    font.setTypeface(fImpl->fTypeface);
    int glyphCount = font.countText(utf8text, textBytes, SkTextEncoding::kUTF8);
    if (glyphCount <= 0) {
        return point;
    }

    static constexpr size_t kInlineStorage = 256;
    SkSTArray<kInlineStorage, SkGlyphID, true> glyphs;
    glyphs.reset(glyphCount);
    SkAssertResult(font.textToGlyphs(utf8text, textBytes, SkTextEncoding::kUTF8,
                                     glyphs.data(), glyphCount) == glyphCount);

    SkSTArray<kInlineStorage, SkPoint, true> pos;
    pos.reset(glyphCount);
    font.getPos(glyphs.data(), glyphCount, pos.data(), point);

    (*handler)(glyphs.data(), glyphCount, pos.data());

    SkFontMetrics metrics;
    font.getMetrics(&metrics);

    return { point.x(), point.y() - metrics.fAscent + metrics.fDescent + metrics.fLeading };
}
