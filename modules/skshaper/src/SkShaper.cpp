/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/private/SkTFitsIn.h"
#include "modules/skshaper/include/SkShaper.h"
#include "src/core/SkMakeUnique.h"
#include "src/core/SkTextBlobPriv.h"
#include "src/utils/SkUTF.h"

#include <limits.h>
#include <string.h>
#include <locale>
#include <string>
#include <utility>

std::unique_ptr<SkShaper> SkShaper::Make() {
#ifdef SK_SHAPER_HARFBUZZ_AVAILABLE
    std::unique_ptr<SkShaper> shaper = SkShaper::MakeShaperDrivenWrapper();
    if (shaper) {
        return shaper;
    }
#endif
    return SkShaper::MakePrimitive();
}

SkShaper::SkShaper() {}
SkShaper::~SkShaper() {}

/** Replaces invalid utf-8 sequences with REPLACEMENT CHARACTER U+FFFD. */
static inline SkUnichar utf8_next(const char** ptr, const char* end) {
    SkUnichar val = SkUTF::NextUTF8(ptr, end);
    return val < 0 ? 0xFFFD : val;
}

class FontMgrRunIterator final : public SkShaper::FontRunIterator {
public:
    FontMgrRunIterator(const char* utf8, size_t utf8Bytes, const SkFont& font,
                       sk_sp<SkFontMgr> fallbackMgr)
        : fCurrent(utf8), fBegin(utf8), fEnd(fCurrent + utf8Bytes)
        , fFallbackMgr(std::move(fallbackMgr))
        , fFont(font)
        , fFallbackFont(fFont)
        , fCurrentFont(nullptr)
    {
        fFont.setTypeface(font.refTypefaceOrDefault());
        fFallbackFont.setTypeface(nullptr);
    }
    void consume() override {
        SkASSERT(fCurrent < fEnd);
        SkUnichar u = utf8_next(&fCurrent, fEnd);
        // If the starting typeface can handle this character, use it.
        if (fFont.unicharToGlyph(u)) {
            fCurrentFont = &fFont;
        // If the current fallback can handle this character, use it.
        } else if (fFallbackFont.getTypeface() && fFallbackFont.unicharToGlyph(u)) {
            fCurrentFont = &fFallbackFont;
        // If not, try to find a fallback typeface
        } else {
            sk_sp<SkTypeface> candidate(fFallbackMgr->matchFamilyStyleCharacter(
                nullptr, fFont.getTypeface()->fontStyle(), nullptr, 0, u));
            if (candidate) {
                fFallbackFont.setTypeface(std::move(candidate));
                fCurrentFont = &fFallbackFont;
            } else {
                fCurrentFont = &fFont;
            }
        }

        while (fCurrent < fEnd) {
            const char* prev = fCurrent;
            u = utf8_next(&fCurrent, fEnd);

            // End run if not using initial typeface and initial typeface has this character.
            if (fCurrentFont->getTypeface() != fFont.getTypeface() && fFont.unicharToGlyph(u)) {
                fCurrent = prev;
                return;
            }

            // End run if current typeface does not have this character and some other font does.
            if (!fCurrentFont->unicharToGlyph(u)) {
                sk_sp<SkTypeface> candidate(fFallbackMgr->matchFamilyStyleCharacter(
                    nullptr, fFont.getTypeface()->fontStyle(), nullptr, 0, u));
                if (candidate) {
                    fCurrent = prev;
                    return;
                }
            }
        }
    }
    size_t endOfCurrentRun() const override {
        return fCurrent - fBegin;
    }
    bool atEnd() const override {
        return fCurrent == fEnd;
    }

    const SkFont& currentFont() const override {
        return *fCurrentFont;
    }

private:
    char const * fCurrent;
    char const * const fBegin;
    char const * const fEnd;
    sk_sp<SkFontMgr> fFallbackMgr;
    SkFont fFont;
    SkFont fFallbackFont;
    SkFont* fCurrentFont;
};

std::unique_ptr<SkShaper::FontRunIterator>
SkShaper::MakeFontMgrRunIterator(const char* utf8, size_t utf8Bytes,
                                 const SkFont& font, sk_sp<SkFontMgr> fallback)
{
    return skstd::make_unique<FontMgrRunIterator>(utf8, utf8Bytes, font, std::move(fallback));
}

std::unique_ptr<SkShaper::LanguageRunIterator>
SkShaper::MakeStdLanguageRunIterator(const char* utf8, size_t utf8Bytes) {
    return skstd::make_unique<TrivialLanguageRunIterator>(std::locale().name().c_str(), utf8Bytes);
}

void SkTextBlobBuilderRunHandler::beginLine() {
    fCurrentPosition = fOffset;
    fMaxRunAscent = 0;
    fMaxRunDescent = 0;
    fMaxRunLeading = 0;
}
void SkTextBlobBuilderRunHandler::runInfo(const RunInfo& info) {
    SkFontMetrics metrics;
    info.fFont.getMetrics(&metrics);
    fMaxRunAscent = SkTMin(fMaxRunAscent, metrics.fAscent);
    fMaxRunDescent = SkTMax(fMaxRunDescent, metrics.fDescent);
    fMaxRunLeading = SkTMax(fMaxRunLeading, metrics.fLeading);
}

void SkTextBlobBuilderRunHandler::commitRunInfo() {
    fCurrentPosition.fY -= fMaxRunAscent;
}

SkShaper::RunHandler::Buffer SkTextBlobBuilderRunHandler::runBuffer(const RunInfo& info) {
    int glyphCount = SkTFitsIn<int>(info.glyphCount) ? info.glyphCount : INT_MAX;
    int utf8RangeSize = SkTFitsIn<int>(info.utf8Range.size()) ? info.utf8Range.size() : INT_MAX;

    const auto& runBuffer = SkTextBlobBuilderPriv::AllocRunTextPos(&fBuilder, info.fFont, glyphCount,
                                                                   utf8RangeSize, SkString());
    if (runBuffer.utf8text && fUtf8Text) {
        memcpy(runBuffer.utf8text, fUtf8Text + info.utf8Range.begin(), utf8RangeSize);
    }
    fClusters = runBuffer.clusters;
    fGlyphCount = glyphCount;
    fClusterOffset = info.utf8Range.begin();

    return { runBuffer.glyphs,
             runBuffer.points(),
             nullptr,
             runBuffer.clusters,
             fCurrentPosition };
}

void SkTextBlobBuilderRunHandler::commitRunBuffer(const RunInfo& info) {
    SkASSERT(0 <= fClusterOffset);
    for (int i = 0; i < fGlyphCount; ++i) {
        SkASSERT(fClusters[i] >= (unsigned)fClusterOffset);
        fClusters[i] -= fClusterOffset;
    }
    fCurrentPosition += info.fAdvance;
}
void SkTextBlobBuilderRunHandler::commitLine() {
    fOffset += { 0, fMaxRunDescent + fMaxRunLeading - fMaxRunAscent };
}

sk_sp<SkTextBlob> SkTextBlobBuilderRunHandler::makeBlob() {
    return fBuilder.make();
}
