/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <unicode/brkiter.h>
#include <unicode/ubidi.h>
#include "SkParagraphImpl.h"
#include "include/core/SkBlurTypes.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkPictureRecorder.h"
#include "src/core/SkSpan.h"
#include "src/utils/SkUTF.h"

class SkFontIterator final : public SkShaper::FontRunIterator {
public:
    SkFontIterator(SkSpan<const char> utf8,
                   SkSpan<SkBlock> styles,
                   sk_sp<SkFontCollection> fonts,
                   bool hintingOn);

    void consume() override;

    size_t endOfCurrentRun() const override { return fCurrentChar - fText.begin(); }
    bool atEnd() const override { return fCurrentChar == fText.end(); }
    const SkFont& currentFont() const override { return fFont; }
    SkScalar lineHeight() const { return fLineHeight; }

private:
    struct Hash {
        uint32_t operator()(const std::pair<SkFont, SkScalar>& key) const {
            return SkTypeface::UniqueID(key.first.getTypeface()) +
                   SkScalarCeilToInt(key.first.getSize()) + SkScalarCeilToInt(key.second);
        }
    };

    void findAllFontsForAllStyledBlocks();

    void findAllFontsForStyledBlock(const SkTextStyle& style, SkSpan<const char> text);

    std::pair<SkFont, SkScalar> makeFont(sk_sp<SkTypeface> typeface, SkScalar size,
                                         SkScalar height);

    size_t resolveAllCharactersByFont(std::pair<SkFont, SkScalar> font);
    void addResolvedWhitespacesToMapping();

    SkUnichar firstUnresolved() {
        if (fUnresolved == 0) return 0;

        bool firstTry = fUnresolved == fCodepoints.size();
        auto index = firstTry ? 0 : fUnresolvedIndexes[0];
        return fCodepoints[index];
    }

    SkSpan<const char> fText;
    SkSpan<SkBlock> fStyles;
    const char* fCurrentChar;
    SkFont fFont;
    SkScalar fLineHeight;
    sk_sp<SkFontCollection> fFontCollection;
    SkTHashMap<const char*, std::pair<SkFont, SkScalar>> fFontMapping;
    SkTHashSet<std::pair<SkFont, SkScalar>, Hash> fResolvedFonts;
    bool fHintingOn;
    std::pair<SkFont, SkScalar> fFirstResolvedFont;

    SkTArray<SkUnichar> fCodepoints;
    SkTArray<const char*> fCharacters;
    SkTArray<size_t> fUnresolvedIndexes;
    SkTArray<SkUnichar> fUnresolvedCodepoints;
    SkTHashMap<size_t, std::pair<SkFont, SkScalar>> fWhitespaces;
    size_t fUnresolved;
};