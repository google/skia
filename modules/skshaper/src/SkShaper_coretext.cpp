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
#include <vector>

void SkShaper_CoreText::shape(const char* utf8, size_t utf8Bytes,
                              const SkFont& font,
                              bool /* leftToRight */,
                              SkScalar width,
                              RunHandler* handler) const {
    auto cgfloat_to_scalar = [](CGFloat x) {
        SkScalar s;
        if (sizeof(CGFloat) == sizeof(double)) {
            s = SkDoubleToScalar(x);
        } else {
            s = x;
        }
        return s;
    };

    CFStringRef textString = CFStringCreateWithBytes(nullptr, (const uint8_t*)utf8, utf8Bytes,
                                                     kCFStringEncodingUTF8, false);

    auto typeface = font.getTypefaceOrDefault();
    auto ctfont = SkTypeface_GetCTFontRef(typeface);
    auto ctfont2 = CTFontCreateCopyWithAttributes(ctfont, font.getSize(), nullptr, nullptr);

    CFMutableDictionaryRef attr =
            CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
                                      &kCFTypeDictionaryKeyCallBacks,
                                      &kCFTypeDictionaryValueCallBacks);
    CFDictionaryAddValue(attr, kCTFontAttributeName, ctfont2);

    CFAttributedStringRef attrString = CFAttributedStringCreate(nullptr, textString, attr);

    CTTypesetterRef typesetter = CTTypesetterCreateWithAttributedStringAndOptions(attrString,
                                                                                  nullptr);

    CTFramesetterRef framesetter = CTFramesetterCreateWithTypesetter(typesetter);

    CGFloat path_bottom = 100000;
    CGPathRef path = CGPathCreateWithRect({{0, 0}, {width, path_bottom}}, nullptr);
    CTFrameRef frame = CTFramesetterCreateFrame(framesetter, {0, 0}, path, nullptr);

    CFArrayRef line_array = CTFrameGetLines(frame);
    CFIndex count = CFArrayGetCount(line_array);
    std::vector<SkShaper::RunHandler::RunInfo> infos;
    for (CFIndex i = 0; i < count; ++i) {
        CTLineRef line = (CTLineRef)CFArrayGetValueAtIndex(line_array, i);

        CFArrayRef run_array = CTLineGetGlyphRuns(line);
        CFIndex runCount = CFArrayGetCount(run_array);
        if (runCount == 0) {
            continue;
        }
        handler->beginLine();
        infos.clear();
        for (CFIndex j = 0; j < runCount; ++j) {
            CTRunRef run = (CTRunRef)CFArrayGetValueAtIndex(run_array, j);
            CFIndex runGlyphs = CTRunGetGlyphCount(run);

            SkASSERT(sizeof(CGGlyph) == sizeof(uint16_t));

            CGSize advances[10000];
            CTRunGetAdvances(run, {0, runGlyphs}, advances);
            SkScalar adv = 0;
            for (CFIndex k = 0; k < runGlyphs; ++k) {
                adv += advances[k].width;
            }
            CFRange range = CTRunGetStringRange(run);

            infos.push_back({
                font,   // need resolved font
                0,      // need fBidiLevel
                {adv, 0}, // fAdvance;
                (size_t)runGlyphs,
                {(size_t)range.location, (size_t)range.length},
            });
            handler->runInfo(infos.back());
        }
        handler->commitRunInfo();

        // Now loop through again and fill in the buffers
        for (CFIndex j = 0; j < runCount; ++j) {
            const auto& info = infos[j];
            auto buffer = handler->runBuffer(info);

            CTRunRef run = (CTRunRef)CFArrayGetValueAtIndex(run_array, j);
            CFIndex runGlyphs = info.glyphCount;
            SkASSERT(CTRunGetGlyphCount(run) == (CFIndex)info.glyphCount);

            CTRunGetGlyphs(run, {0, runGlyphs}, buffer.glyphs);
            CGPoint positions[10000];
            CTRunGetPositions(run, {0, runGlyphs}, positions);
            CFIndex indices[10000];
            if (buffer.clusters) {
                CTRunGetStringIndices(run, {0, runGlyphs}, indices);
            }

            for (CFIndex k = 0; k < runGlyphs; ++k) {
                buffer.positions[k] = {
                    buffer.point.fX + cgfloat_to_scalar(positions[k].x),
                    buffer.point.fY,
                };
                if (buffer.offsets) {
                    buffer.offsets[k] = {0, 0}; // ???
                }
                if (buffer.clusters) {
                    buffer.clusters[k] = indices[k];
                }
            }
            handler->commitRunBuffer(info);
        }
        handler->commitLine();
    }
}

#else
std::unique_ptr<SkShaper> SkShaper::MakeCoreText() {
    return nullptr;
}
#endif
