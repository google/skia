/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skshaper/include/SkShaper.h"

#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)

#ifdef SK_BUILD_FOR_MAC
#import <ApplicationServices/ApplicationServices.h>
#endif

#ifdef SK_BUILD_FOR_IOS
#include <CoreText/CoreText.h>
#include <CoreText/CTFontManager.h>
#include <CoreGraphics/CoreGraphics.h>
#include <CoreFoundation/CoreFoundation.h>
#endif

#include "include/core/SkFontMetrics.h"
#include "include/core/SkStream.h"
#include "include/core/SkTypeface.h"
#include "include/private/SkTo.h"
#include "src/utils/SkUTF.h"

class SkShaper_CoreText : public SkShaper {
public:
    SkShaper_CoreText() {}
private:
    void shape(const char* utf8, size_t utf8Bytes,
               const SkFont& srcFont,
               bool leftToRight,
               SkScalar width,
               RunHandler*) const override;

    void shape(const char* utf8, size_t utf8Bytes,
               FontRunIterator&,
               BiDiRunIterator&,
               ScriptRunIterator&,
               LanguageRunIterator&,
               SkScalar width,
               RunHandler*) const override;

    void shape(const char* utf8, size_t utf8Bytes,
               FontRunIterator&,
               BiDiRunIterator&,
               ScriptRunIterator&,
               LanguageRunIterator&,
               const Feature*, size_t featureSize,
               SkScalar width,
               RunHandler*) const override;
};

std::unique_ptr<SkShaper> SkShaper::MakeCoreText() {
    return std::make_unique<SkShaper_CoreText>();
}

static inline bool is_breaking_whitespace(SkUnichar c) {
    switch (c) {
        case 0x0020: // SPACE
        //case 0x00A0: // NO-BREAK SPACE
        case 0x1680: // OGHAM SPACE MARK
        case 0x180E: // MONGOLIAN VOWEL SEPARATOR
        case 0x2000: // EN QUAD
        case 0x2001: // EM QUAD
        case 0x2002: // EN SPACE (nut)
        case 0x2003: // EM SPACE (mutton)
        case 0x2004: // THREE-PER-EM SPACE (thick space)
        case 0x2005: // FOUR-PER-EM SPACE (mid space)
        case 0x2006: // SIX-PER-EM SPACE
        case 0x2007: // FIGURE SPACE
        case 0x2008: // PUNCTUATION SPACE
        case 0x2009: // THIN SPACE
        case 0x200A: // HAIR SPACE
        case 0x200B: // ZERO WIDTH SPACE
        case 0x202F: // NARROW NO-BREAK SPACE
        case 0x205F: // MEDIUM MATHEMATICAL SPACE
        case 0x3000: // IDEOGRAPHIC SPACE
        //case 0xFEFF: // ZERO WIDTH NO-BREAK SPACE
            return true;
        default:
            return false;
    }
}

static size_t linebreak(const char text[], const char stop[],
                        const SkFont& font, SkScalar width,
                        SkScalar* advance,
                        size_t* trailing)
{
    SkScalar accumulatedWidth = 0;
    int glyphIndex = 0;
    const char* start = text;
    const char* word_start = text;
    bool prevWS = true;
    *trailing = 0;

    while (text < stop) {
        const char* prevText = text;
        SkUnichar uni = SkUTF::NextUTF8(&text, stop);
        accumulatedWidth += advance[glyphIndex++];
        bool currWS = is_breaking_whitespace(uni);

        if (!currWS && prevWS) {
            word_start = prevText;
        }
        prevWS = currWS;

        if (width < accumulatedWidth) {
            if (currWS) {
                // eat the rest of the whitespace
                const char* next = text;
                while (next < stop && is_breaking_whitespace(SkUTF::NextUTF8(&next, stop))) {
                    text = next;
                }
                if (trailing) {
                    *trailing = text - prevText;
                }
            } else {
                // backup until a whitespace (or 1 char)
                if (word_start == start) {
                    if (prevText > start) {
                        text = prevText;
                    }
                } else {
                    text = word_start;
                }
            }
            break;
        }
    }

    return text - start;
}

void SkShaper_CoreText::shape(const char* utf8, size_t utf8Bytes,
                              FontRunIterator& font,
                              BiDiRunIterator& bidi,
                              ScriptRunIterator&,
                              LanguageRunIterator&,
                              SkScalar width,
                              RunHandler* handler) const
{
    SkFont skfont;
    if (!font.atEnd()) {
        font.consume();
        skfont = font.currentFont();
    } else {
        skfont.setTypeface(sk_ref_sp(skfont.getTypefaceOrDefault()));
    }
    SkASSERT(skfont.getTypeface());
    bool skbidi = 0;
    if (!bidi.atEnd()) {
        bidi.consume();
        skbidi = (bidi.currentLevel() % 2) == 0;
    }
    return this->shape(utf8, utf8Bytes, skfont, skbidi, width, handler);
}

void SkShaper_CoreText::shape(const char* utf8, size_t utf8Bytes,
                              FontRunIterator& font,
                              BiDiRunIterator& bidi,
                              ScriptRunIterator&,
                              LanguageRunIterator&,
                              const Feature*, size_t,
                              SkScalar width,
                              RunHandler* handler) const {
    font.consume();
    SkASSERT(font.currentFont().getTypeface());
    bidi.consume();
    return this->shape(utf8, utf8Bytes, font.currentFont(), (bidi.currentLevel() % 2) == 0,
                       width, handler);
}

#include "include/ports/SkTypeface_mac.h"

void SkShaper_CoreText::shape(const char* utf8, size_t utf8Bytes,
                              const SkFont& font,
                              bool /* leftToRight */,
                              SkScalar width,
                              RunHandler* handler) const {
    CFStringRef textString = CFStringCreateWithBytes(nullptr, (const uint8_t*)utf8, utf8Bytes,
                                                     kCFStringEncodingUTF8, false);

//    auto typeface = font.getTypefaceOrDefault();
//    auto ctfont = SkTypeface_GetCTFontRef(typeface);

    CFAttributedStringRef attrString = CFAttributedStringCreate(nullptr, textString, nullptr);

    CTTypesetterRef typesetter = CTTypesetterCreateWithAttributedStringAndOptions(attrString,
                                                                                  nullptr);

    CTFramesetterRef framesetter = CTFramesetterCreateWithTypesetter(typesetter);

    CGPathRef path = CGPathCreateWithRect({{0, 0}, {width, 1000000}}, nullptr);
    CTFrameRef frame = CTFramesetterCreateFrame(framesetter, {0, 0}, path, nullptr);

    CFArrayRef line_array = CTFrameGetLines(frame);
    CFIndex count = CFArrayGetCount(line_array);
    for (CFIndex i = 0; i < count; ++i) {
        CGPoint origin;
        CTFrameGetLineOrigins(frame, {i, 1}, &origin);

        CTLineRef line = (CTLineRef)CFArrayGetValueAtIndex(line_array, i);
        int lineGlyphs = CTLineGetGlyphCount(line);
        SkDebugf("Line:%d glyphs:%d [%g %g]\n", (int)i, lineGlyphs, origin.x, origin.y);

        CFArrayRef run_array = CTLineGetGlyphRuns(line);
        CFIndex runCount = CFArrayGetCount(run_array);
        for (CFIndex j = 0; j < runCount; ++j) {
            CTRunRef run = (CTRunRef)CFArrayGetValueAtIndex(run_array, j);
            CFIndex runGlyphs = CTRunGetGlyphCount(run);

            SkASSERT(sizeof(CGGlyph) == sizeof(uint16_t));

            uint16_t storage[10000];
            CTRunGetGlyphs(run, {0, runGlyphs}, storage);
            for (CFIndex k = 0; k < runGlyphs; ++k) {
                SkDebugf(" %d", storage[k]);
            }
            SkDebugf("\n");
        }

    }

#if 0
    size_t glyphOffset = 0;
    size_t utf8Offset = 0;
    while (0 < utf8Bytes) {
        size_t bytesCollapsed;
        size_t bytesConsumed = linebreak(utf8, utf8 + utf8Bytes, font, width,
                                         advances.get() + glyphOffset, &bytesCollapsed);
        size_t bytesVisible = bytesConsumed - bytesCollapsed;

        size_t numGlyphs = SkUTF::CountUTF8(utf8, bytesVisible);
        const RunHandler::RunInfo info = {
            font,
            0,
            { font.measureText(utf8, bytesVisible, SkTextEncoding::kUTF8), 0 },
            numGlyphs,
            RunHandler::Range(utf8Offset, bytesVisible)
        };
        handler->beginLine();
        handler->runInfo(info);
        handler->commitRunInfo();
        const auto buffer = handler->runBuffer(info);

        memcpy(buffer.glyphs, glyphs.get() + glyphOffset, numGlyphs * sizeof(SkGlyphID));
        SkPoint position = buffer.point;
        for (size_t i = 0; i < numGlyphs; ++i) {
            buffer.positions[i] = position;
            position.fX += advances[i + glyphOffset];
        }
        if (buffer.clusters) {
            const char* txtPtr = utf8;
            for (size_t i = 0; i < numGlyphs; ++i) {
                // Each character maps to exactly one glyph.
                buffer.clusters[i] = SkToU32(txtPtr - utf8 + utf8Offset);
                SkUTF::NextUTF8(&txtPtr, utf8 + utf8Bytes);
            }
        }
        handler->commitRunBuffer(info);
        handler->commitLine();

        glyphOffset += SkUTF::CountUTF8(utf8, bytesConsumed);
        utf8Offset += bytesConsumed;
        utf8 += bytesConsumed;
        utf8Bytes -= bytesConsumed;
    }
#endif
}

#else
std::unique_ptr<SkShaper> SkShaper::MakeCoreText() {
    return nullptr;
}
#endif
