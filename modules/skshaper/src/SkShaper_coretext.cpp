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

#include "include/ports/SkTypeface_mac.h"
#include "src/core/SkArenaAlloc.h"

#include <vector>

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

template <typename T> class AutoCF {
    T fObj;
public:
    AutoCF(T obj) : fObj(obj) {}
    ~AutoCF() { CFRelease(fObj); }

    T get() const { return fObj; }
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

    CTLineRef nextLine() {
        CFIndex count = CTTypesetterSuggestLineBreak(fTypesetter, fStart, fWidth);
        if (count == 0) {
            return nullptr;
        }
        auto line = CTTypesetterCreateLine(fTypesetter, {fStart, count});
        fStart += count;
        return line;
    }
};

static void dict_add_double(CFMutableDictionaryRef d, const void* name, double value) {
    AutoCF<CFNumberRef> number = CFNumberCreate(kCFAllocatorDefault, kCFNumberDoubleType, &value);
    CFDictionaryAddValue(d, name, number.get());
}

static void dump(CFDictionaryRef d) {
    CFIndex count = CFDictionaryGetCount(d);
    std::vector<const void*> keys(count);
    std::vector<const void*> vals(count);

    CFDictionaryGetKeysAndValues(d, keys.data(), vals.data());

    for (CFIndex i = 0; i < count; ++i) {
        CFStringRef kstr = (CFStringRef)keys[i];
        const char* ckstr = CFStringGetCStringPtr(kstr, kCFStringEncodingUTF8);
        SkDebugf("dict[%d] %s %p\n", i, ckstr, vals[i]);
    }
}

static CTFontRef create_ctfont_from_font(const SkFont& font) {
    auto typeface = font.getTypefaceOrDefault();
    auto ctfont = SkTypeface_GetCTFontRef(typeface);
    return CTFontCreateCopyWithAttributes(ctfont, font.getSize(), nullptr, nullptr);
}

static SkFont run_to_font(CTRunRef run, const SkFont& orig) {
    CFDictionaryRef attr = CTRunGetAttributes(run);
    CTFontRef ct = (CTFontRef)CFDictionaryGetValue(attr, kCTFontAttributeName);
    if (!ct) {
        SkDebugf("no ctfont in Run Attributes\n");
        dump(attr);
        return orig;
    }
    // Do I need to add a local cache, or allow the caller to manage this lookup?
    SkFont font(orig);
    font.setTypeface(SkMakeTypefaceFromCTFont(ct));
    return font;
}

// kCTTrackingAttributeName not available until 10.12
const CFStringRef kCTTracking_AttributeName = CFSTR("CTTracking");

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

    AutoCF<CFStringRef> textString = CFStringCreateWithBytes(nullptr, (const uint8_t*)utf8, utf8Bytes,
                                                     kCFStringEncodingUTF8, false);

    AutoCF<CTFontRef> ctfont = create_ctfont_from_font(font);

    AutoCF<CFMutableDictionaryRef> attr =
            CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
                                      &kCFTypeDictionaryKeyCallBacks,
                                      &kCFTypeDictionaryValueCallBacks);
    CFDictionaryAddValue(attr.get(), kCTFontAttributeName, ctfont.get());
    if (false) {
        // trying to see what these affect
        dict_add_double(attr.get(), kCTTracking_AttributeName, 1);
        dict_add_double(attr.get(), kCTKernAttributeName, 0.0);
    }

    AutoCF<CFAttributedStringRef> attrString =
            CFAttributedStringCreate(nullptr, textString.get(), attr.get());

    AutoCF<CTTypesetterRef> typesetter =
            CTTypesetterCreateWithAttributedStringAndOptions(attrString.get(), nullptr);

    SkSTArenaAlloc<4096> arena;

    // We have to compute RunInfos in a loop, and then reuse them in a 2nd loop,
    // so we store them in an array (we reuse the array's storage for each line).
    std::vector<SkShaper::RunHandler::RunInfo> infos;

    LineBreakIter iter(typesetter.get(), width);
    while (CTLineRef line = iter.nextLine()) {
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

            CGSize* advances = arena.makeArrayDefault<CGSize>(runGlyphs);
            CTRunGetAdvances(run, {0, runGlyphs}, advances);
            SkScalar adv = 0;
            for (CFIndex k = 0; k < runGlyphs; ++k) {
                adv += advances[k].width;
            }
            arena.reset();

            CFRange range = CTRunGetStringRange(run);

            SkFont run_font = run_to_font(run, font);
            infos.push_back({
                run_font,
                0,      // need fBidiLevel
                {adv, 0},
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

            CGPoint* positions = arena.makeArrayDefault<CGPoint>(runGlyphs);
            CTRunGetPositions(run, {0, runGlyphs}, positions);
            CFIndex* indices = nullptr;
            if (buffer.clusters) {
                indices = arena.makeArrayDefault<CFIndex>(runGlyphs);
                CTRunGetStringIndices(run, {0, runGlyphs}, indices);
            }

            for (CFIndex k = 0; k < runGlyphs; ++k) {
                buffer.positions[k] = {
                    buffer.point.fX + cgfloat_to_scalar(positions[k].x),
                    buffer.point.fY,
                };
                if (buffer.offsets) {
                    buffer.offsets[k] = {0, 0}; // offset relative to the origin for this glyph
                }
                if (buffer.clusters) {
                    buffer.clusters[k] = indices[k];
                }
            }
            arena.reset();
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
