/*
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPaint.h"
#include "include/core/SkFont.h"
#include "include/utils/SkTextUtils.h"
#include "include/xamarin/SkCompatPaint.h"

#include "src/core/SkFontPriv.h"
#include "src/core/SkStrike.h"
#include "src/core/SkStrikeCache.h"
#include "src/core/SkStrikeSpec.h"
#include "src/core/SkUtils.h"
#include "src/utils/SkUTF.h"

SkCompatPaint::SkCompatPaint()
    : fFont(SkFont())
    , fTextAlign(SkTextUtils::Align::kLeft_Align)
    , fTextEncoding(SkTextEncoding::kUTF8)
{
}

SkCompatPaint::SkCompatPaint(const SkCompatPaint& paint) = default;

SkCompatPaint::SkCompatPaint(const SkFont* font)
    : fFont(*font)
    , fTextAlign(SkTextUtils::Align::kLeft_Align)
    , fTextEncoding(SkTextEncoding::kUTF8)
{
}

SkCompatPaint::~SkCompatPaint() = default;

void SkCompatPaint::reset() {
    *this = SkCompatPaint();
}

SkFont* SkCompatPaint::makeFont() {
    return new SkFont(fFont);
}

SkFont* SkCompatPaint::getFont() {
    return &fFont;
}

void SkCompatPaint::setTextAlign(SkTextUtils::Align textAlign) {
    fTextAlign = textAlign;
}

SkTextUtils::Align SkCompatPaint::getTextAlign() const {
    return fTextAlign;
}

void SkCompatPaint::setTextEncoding(SkTextEncoding encoding) {
    fTextEncoding = encoding;
}

SkTextEncoding SkCompatPaint::getTextEncoding() const {
    return fTextEncoding;
}

size_t SkCompatPaint::breakText(const void* text, size_t length, SkScalar maxWidth, SkScalar* measuredWidth) const {
    if (0 == length || 0 >= maxWidth) {
        if (measuredWidth) {
            *measuredWidth = 0;
        }
        return 0;
    }
    if (0 == fFont.getSize()) {
        if (measuredWidth) {
            *measuredWidth = 0;
        }
        return length;
    }

    SkASSERT(text != nullptr);

    SkStrikeSpec strikeSpec = SkStrikeSpec::MakeCanonicalized(fFont, this);
    SkBulkGlyphMetrics metrics{strikeSpec};

    SkScalar scale = strikeSpec.strikeToSourceRatio();

    // adjust max instead of each glyph
    if (scale) {
        maxWidth /= scale;
    }

    SkScalar width = 0;

    const char* start = (const char*)text;
    const char* stop = start + length;
    while (start < stop) {
        const char* curr = start;

        // read the glyph and move the pointer
        SkGlyphID glyphID;
        if (fTextEncoding == SkTextEncoding::kGlyphID) {
            auto glyphs = (const uint16_t*)start;
            glyphID = *glyphs;
            glyphs++;
            start = (const char*)glyphs;
        } else {
            auto t = (const void*)start;
            auto unichar = SkUTFN_Next(fTextEncoding, &t, stop);
            start = (const char*)t;
            glyphID = fFont.unicharToGlyph(unichar);
        }

        auto glyph = metrics.glyph(glyphID);

        SkScalar x = glyph->advanceX();
        if ((width += x) > maxWidth) {
            width -= x;
            start = curr;
            break;
        }
    }

    if (measuredWidth) {
        if (scale) {
            width *= scale;
        }
        *measuredWidth = width;
    }

    // return the number of bytes measured
    return start - stop + length;
}
