/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skshaper/include/SkShaper.h"

#ifdef SK_BUILD_FOR_MAC
#import <ApplicationServices/ApplicationServices.h>
#endif

#ifdef SK_BUILD_FOR_IOS
#include <CoreText/CoreText.h>
#include <CoreText/CTFontManager.h>
#include <CoreGraphics/CoreGraphics.h>
#include <CoreFoundation/CoreFoundation.h>
#endif

#include "include/ports/SkTypeface_mac.h"
#include "include/private/base/SkTemplates.h"
#include "src/base/SkUTF.h"
#include "src/core/SkFontPriv.h"
#include "src/utils/mac/SkCGBase.h"
#include "src/utils/mac/SkUniqueCFRef.h"

#include <vector>
#include <utility>

using namespace skia_private;

class SkShaper_CoreText : public SkShaper {
public:
    SkShaper_CoreText() {}
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

// CTFramesetter/CTFrame can do this, but require version 10.14
class LineBreakIter {
    CTTypesetterRef fTypesetter;
    double          fWidth;
    CFIndex         fStart;

public:
    LineBreakIter(CTTypesetterRef ts, SkScalar width) : fTypesetter(ts), fWidth(width) {
        fStart = 0;
    }

    SkUniqueCFRef<CTLineRef> nextLine() {
        CFRange stringRange {fStart, CTTypesetterSuggestLineBreak(fTypesetter, fStart, fWidth)};
        if (stringRange.length == 0) {
            return nullptr;
        }
        fStart += stringRange.length;
        return SkUniqueCFRef<CTLineRef>(CTTypesetterCreateLine(fTypesetter, stringRange));
    }
};

static void dict_add_double(CFMutableDictionaryRef d, const void* name, double value) {
    SkUniqueCFRef<CFNumberRef> number(
            CFNumberCreate(kCFAllocatorDefault, kCFNumberDoubleType, &value));
    CFDictionaryAddValue(d, name, number.get());
}

static SkUniqueCFRef<CTFontRef> create_ctfont_from_font(const SkFont& font) {
    auto typeface = font.getTypeface();
    auto ctfont = SkTypeface_GetCTFontRef(typeface);
    return SkUniqueCFRef<CTFontRef>(
            CTFontCreateCopyWithAttributes(ctfont, font.getSize(), nullptr, nullptr));
}

static SkFont run_to_font(CTRunRef run, const SkFont& orig) {
    CFDictionaryRef attr = CTRunGetAttributes(run);
    CTFontRef ct = (CTFontRef)CFDictionaryGetValue(attr, kCTFontAttributeName);
    if (!ct) {
        SkDebugf("no ctfont in Run Attributes\n");
        CFShow(attr);
        return orig;
    }
    // Do I need to add a local cache, or allow the caller to manage this lookup?
    SkFont font(orig);
    font.setTypeface(SkMakeTypefaceFromCTFont(ct));
    return font;
}

namespace {
class UTF16ToUTF8IndicesMap {
public:
    /** Builds a UTF-16 to UTF-8 indices map; the text is not retained
     * @return true if successful
     */
    bool setUTF8(const char* utf8, size_t size) {
        SkASSERT(utf8 != nullptr);

        if (!SkTFitsIn<int32_t>(size)) {
            SkDEBUGF("UTF16ToUTF8IndicesMap: text too long");
            return false;
        }

        auto utf16Size = SkUTF::UTF8ToUTF16(nullptr, 0, utf8, size);
        if (utf16Size < 0) {
            SkDEBUGF("UTF16ToUTF8IndicesMap: Invalid utf8 input");
            return false;
        }

        // utf16Size+1 to also store the size
        fUtf16ToUtf8Indices = std::vector<size_t>(utf16Size + 1);
        auto utf16 = fUtf16ToUtf8Indices.begin();
        auto utf8Begin = utf8, utf8End = utf8 + size;
        while (utf8Begin < utf8End) {
            *utf16 = utf8Begin - utf8;
            utf16 += SkUTF::ToUTF16(SkUTF::NextUTF8(&utf8Begin, utf8End), nullptr);
        }
        *utf16 = size;

        return true;
    }

    size_t mapIndex(size_t index) const {
        SkASSERT(index < fUtf16ToUtf8Indices.size());
        return fUtf16ToUtf8Indices[index];
    }

    std::pair<size_t, size_t> mapRange(size_t start, size_t size) const {
        auto utf8Start = mapIndex(start);
        return {utf8Start, mapIndex(start + size) - utf8Start};
    }
private:
    std::vector<size_t> fUtf16ToUtf8Indices;
};
} // namespace

// kCTTrackingAttributeName not available until 10.12
const CFStringRef kCTTracking_AttributeName = CFSTR("CTTracking");

#if !defined(SK_DISABLE_LEGACY_SKSHAPER_FUNCTIONS)
void SkShaper_CoreText::shape(const char* utf8,
                              size_t utf8Bytes,
                              FontRunIterator& font,
                              BiDiRunIterator& bidi,
                              ScriptRunIterator& script,
                              LanguageRunIterator& lang,
                              SkScalar width,
                              RunHandler* handler) const {
    return this->shape(utf8, utf8Bytes, font, bidi, script, lang, nullptr, 0, width, handler);
}

void SkShaper_CoreText::shape(const char* utf8,
                              size_t utf8Bytes,
                              const SkFont& font,
                              bool,
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

void SkShaper_CoreText::shape(const char* utf8,
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

    SkUniqueCFRef<CFStringRef> textString(
            CFStringCreateWithBytes(kCFAllocatorDefault, (const uint8_t*)utf8, utf8Bytes,
                                    kCFStringEncodingUTF8, false));

    UTF16ToUTF8IndicesMap utf8IndicesMap;
    if (!utf8IndicesMap.setUTF8(utf8, utf8Bytes)) {
        return;
    }

    SkUniqueCFRef<CTFontRef> ctfont = create_ctfont_from_font(font);

    SkUniqueCFRef<CFMutableDictionaryRef> attr(
            CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
                                      &kCFTypeDictionaryKeyCallBacks,
                                      &kCFTypeDictionaryValueCallBacks));
    CFDictionaryAddValue(attr.get(), kCTFontAttributeName, ctfont.get());
    if (false) {
        // trying to see what these affect
        dict_add_double(attr.get(), kCTTracking_AttributeName, 1);
        dict_add_double(attr.get(), kCTKernAttributeName, 0.0);
    }

    SkUniqueCFRef<CFAttributedStringRef> attrString(
            CFAttributedStringCreate(kCFAllocatorDefault, textString.get(), attr.get()));

    SkUniqueCFRef<CTTypesetterRef> typesetter(
            CTTypesetterCreateWithAttributedString(attrString.get()));

    // We have to compute RunInfos in a loop, and then reuse them in a 2nd loop,
    // so we store them in an array (we reuse the array's storage for each line).
    std::vector<SkFont> fontStorage;
    std::vector<SkShaper::RunHandler::RunInfo> infos;

    LineBreakIter iter(typesetter.get(), width);
    while (SkUniqueCFRef<CTLineRef> line = iter.nextLine()) {
        CFArrayRef run_array = CTLineGetGlyphRuns(line.get());
        CFIndex runCount = CFArrayGetCount(run_array);
        if (runCount == 0) {
            continue;
        }
        handler->beginLine();
        fontStorage.clear();
        fontStorage.reserve(runCount); // ensure the refs won't get invalidated
        infos.clear();
        for (CFIndex j = 0; j < runCount; ++j) {
            CTRunRef run = (CTRunRef)CFArrayGetValueAtIndex(run_array, j);
            CFIndex runGlyphs = CTRunGetGlyphCount(run);

            SkASSERT(sizeof(CGGlyph) == sizeof(uint16_t));

            AutoSTArray<4096, CGSize> advances(runGlyphs);
            CTRunGetAdvances(run, {0, runGlyphs}, advances.data());
            SkScalar adv = 0;
            for (CFIndex k = 0; k < runGlyphs; ++k) {
                adv += advances[k].width;
            }

            CFRange cfRange = CTRunGetStringRange(run);
            auto range = utf8IndicesMap.mapRange(cfRange.location, cfRange.length);

            fontStorage.push_back(run_to_font(run, font));
            infos.push_back({
                fontStorage.back(), // info just stores a ref to the font
                0,                  // need fBidiLevel
                {adv, 0},
                (size_t)runGlyphs,
                {range.first, range.second},
            });
            handler->runInfo(infos.back());
        }
        handler->commitRunInfo();

        // Now loop through again and fill in the buffers
        SkScalar lineAdvance = 0;
        for (CFIndex j = 0; j < runCount; ++j) {
            const auto& info = infos[j];
            auto buffer = handler->runBuffer(info);

            CTRunRef run = (CTRunRef)CFArrayGetValueAtIndex(run_array, j);
            CFIndex runGlyphs = info.glyphCount;
            SkASSERT(CTRunGetGlyphCount(run) == (CFIndex)info.glyphCount);

            CTRunGetGlyphs(run, {0, runGlyphs}, buffer.glyphs);

            AutoSTArray<4096, CGPoint> positions(runGlyphs);
            CTRunGetPositions(run, {0, runGlyphs}, positions.data());
            AutoSTArray<4096, CFIndex> indices;
            if (buffer.clusters) {
                indices.reset(runGlyphs);
                CTRunGetStringIndices(run, {0, runGlyphs}, indices.data());
            }

            for (CFIndex k = 0; k < runGlyphs; ++k) {
                buffer.positions[k] = {
                    buffer.point.fX + SkScalarFromCGFloat(positions[k].x) - lineAdvance,
                    buffer.point.fY,
                };
                if (buffer.offsets) {
                    buffer.offsets[k] = {0, 0}; // offset relative to the origin for this glyph
                }
                if (buffer.clusters) {
                    buffer.clusters[k] = utf8IndicesMap.mapIndex(indices[k]);
                }
            }
            handler->commitRunBuffer(info);
            lineAdvance += info.fAdvance.fX;
        }
        handler->commitLine();
    }
}

namespace SkShapers::CT {
std::unique_ptr<SkShaper> CoreText() { return std::make_unique<SkShaper_CoreText>(); }
}  // namespace SkShapers::CT
