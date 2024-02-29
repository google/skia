/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkFont.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkPoint.h"
#include "include/core/SkScalar.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkTo.h"
#include "modules/skshaper/include/SkShaper.h"
#include "src/base/SkUTF.h"

#include <cstdint>
#include <cstring>
#include <memory>

class SkShaperPrimitive : public SkShaper {
public:
    SkShaperPrimitive() {}
private:
#if !defined(SK_DISABLE_LEGACY_SKSHAPER_FUNCTIONS)
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
#endif

    void shape(const char* utf8, size_t utf8Bytes,
               FontRunIterator&,
               BiDiRunIterator&,
               ScriptRunIterator&,
               LanguageRunIterator&,
               const Feature*, size_t featureSize,
               SkScalar width,
               RunHandler*) const override;
};

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
    const char* wordStart = text;
    bool prevWS = true;
    *trailing = 0;

    while (text < stop) {
        const char* prevText = text;
        SkUnichar uni = SkUTF::NextUTF8(&text, stop);
        accumulatedWidth += advance[glyphIndex++];
        bool currWS = is_breaking_whitespace(uni);

        if (!currWS && prevWS) {
            wordStart = prevText;
        }
        prevWS = currWS;

        if (width < accumulatedWidth) {
            bool consumeWhitespace = false;
            if (currWS) {
                // previous fit, put this and following whitespace in trailing
                if (prevText == start) {
                    // don't put this in trailing if it's the first thing
                    prevText = text;
                }
                consumeWhitespace = true;
            } else if (wordStart != start) {
                // backup to the last whitespace that fit
                text = wordStart;
            } else if (prevText > start) {
                // backup to just before the glyph that didn't fit
                text = prevText;
            } else {
                // let it overflow, put any following whitespace in trailing
                prevText = text;
                consumeWhitespace = true;
            }
            if (consumeWhitespace) {
                const char* next = text;
                while (next < stop && is_breaking_whitespace(SkUTF::NextUTF8(&next, stop))) {
                    text = next;
                }
                if (trailing) {
                    *trailing = text - prevText;
                }
            }
            break;
        }
    }

    return text - start;
}

#if !defined(SK_DISABLE_LEGACY_SKSHAPER_FUNCTIONS)
void SkShaperPrimitive::shape(const char* utf8,
                              size_t utf8Bytes,
                              FontRunIterator& font,
                              BiDiRunIterator& bidi,
                              ScriptRunIterator& script,
                              LanguageRunIterator& lang,
                              SkScalar width,
                              RunHandler* handler) const {
    return this->shape(utf8, utf8Bytes, font, bidi, script, lang, nullptr, 0, width, handler);
}

void SkShaperPrimitive::shape(const char* utf8,
                              size_t utf8Bytes,
                              const SkFont& font,
                              bool leftToRight,
                              SkScalar width,
                              RunHandler* handler) const {
    std::unique_ptr<FontRunIterator> fontRuns(
            MakeFontMgrRunIterator(utf8, utf8Bytes, font, nullptr));
    if (!fontRuns) {
        return;
    }
    // bidi, script, and lang are all unused so we can construct them with empty data.
    TrivialBiDiRunIterator bidi{0, 0};
    TrivialScriptRunIterator script{0, 0};
    TrivialLanguageRunIterator lang{nullptr, 0};
    return this->shape(utf8, utf8Bytes, *fontRuns, bidi, script, lang, nullptr, 0, width, handler);
}
#endif

void SkShaperPrimitive::shape(const char* utf8,
                              size_t utf8Bytes,
                              FontRunIterator& fontRuns,
                              BiDiRunIterator&,
                              ScriptRunIterator&,
                              LanguageRunIterator&,
                              const Feature*,
                              size_t,
                              SkScalar width,
                              RunHandler* handler) const {
    SkFont font;
    if (!fontRuns.atEnd()) {
        fontRuns.consume();
        font = fontRuns.currentFont();
    }
    SkASSERT(font.getTypeface());

    int glyphCount = font.countText(utf8, utf8Bytes, SkTextEncoding::kUTF8);
    if (glyphCount < 0) {
        return;
    }

    std::unique_ptr<SkGlyphID[]> glyphs(new SkGlyphID[glyphCount]);
    font.textToGlyphs(utf8, utf8Bytes, SkTextEncoding::kUTF8, glyphs.get(), glyphCount);

    std::unique_ptr<SkScalar[]> advances(new SkScalar[glyphCount]);
    font.getWidthsBounds(glyphs.get(), glyphCount, advances.get(), nullptr, nullptr);

    size_t glyphOffset = 0;
    size_t utf8Offset = 0;
    do {
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
        if (info.glyphCount) {
            handler->runInfo(info);
        }
        handler->commitRunInfo();
        if (info.glyphCount) {
            const auto buffer = handler->runBuffer(info);

            memcpy(buffer.glyphs, glyphs.get() + glyphOffset, info.glyphCount * sizeof(SkGlyphID));
            SkPoint position = buffer.point;
            for (size_t i = 0; i < info.glyphCount; ++i) {
                buffer.positions[i] = position;
                position.fX += advances[i + glyphOffset];
            }
            if (buffer.clusters) {
                const char* txtPtr = utf8;
                for (size_t i = 0; i < info.glyphCount; ++i) {
                    // Each character maps to exactly one glyph.
                    buffer.clusters[i] = SkToU32(txtPtr - utf8 + utf8Offset);
                    SkUTF::NextUTF8(&txtPtr, utf8 + utf8Bytes);
                }
            }
            handler->commitRunBuffer(info);
        }
        handler->commitLine();

        glyphOffset += SkUTF::CountUTF8(utf8, bytesConsumed);
        utf8Offset += bytesConsumed;
        utf8 += bytesConsumed;
        utf8Bytes -= bytesConsumed;
    } while (0 < utf8Bytes);
}

#if !defined(SK_DISABLE_LEGACY_SKSHAPER_FUNCTIONS)
std::unique_ptr<SkShaper> SkShaper::MakePrimitive() { return SkShapers::Primitive(); }
#endif

namespace SkShapers {
std::unique_ptr<SkShaper> Primitive() { return std::make_unique<SkShaperPrimitive>(); }
}  // namespace SkShapers
