/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "modules/skshaper/include/SkShaper_skunicode.h"

#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkTFitsIn.h"
#include "modules/skshaper/include/SkShaper.h"
#include "modules/skunicode/include/SkUnicode.h"
#include "src/base/SkUTF.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <utility>

using SkUnicodeBidi = std::unique_ptr<SkBidiIterator>;

/** Replaces invalid utf-8 sequences with REPLACEMENT CHARACTER U+FFFD. */
static inline SkUnichar utf8_next(const char** ptr, const char* end) {
    SkUnichar val = SkUTF::NextUTF8(ptr, end);
    return val < 0 ? 0xFFFD : val;
}

class SkUnicodeBidiRunIterator final : public SkShaper::BiDiRunIterator {
public:
    SkUnicodeBidiRunIterator(const char* utf8, const char* end, SkUnicodeBidi bidi)
        : fBidi(std::move(bidi))
        , fEndOfCurrentRun(utf8)
        , fBegin(utf8)
        , fEnd(end)
        , fUTF16LogicalPosition(0)
        , fLevel(SkBidiIterator::kLTR)
    {}

    void consume() override {
        SkASSERT(fUTF16LogicalPosition < fBidi->getLength());
        int32_t endPosition = fBidi->getLength();
        fLevel = fBidi->getLevelAt(fUTF16LogicalPosition);
        SkUnichar u = utf8_next(&fEndOfCurrentRun, fEnd);
        fUTF16LogicalPosition += SkUTF::ToUTF16(u);
        SkBidiIterator::Level level;
        while (fUTF16LogicalPosition < endPosition) {
            level = fBidi->getLevelAt(fUTF16LogicalPosition);
            if (level != fLevel) {
                break;
            }
            u = utf8_next(&fEndOfCurrentRun, fEnd);

            fUTF16LogicalPosition += SkUTF::ToUTF16(u);
        }
    }
    size_t endOfCurrentRun() const override {
        return fEndOfCurrentRun - fBegin;
    }
    bool atEnd() const override {
        return fUTF16LogicalPosition == fBidi->getLength();
    }
    SkBidiIterator::Level currentLevel() const override {
        return fLevel;
    }
private:
    SkUnicodeBidi fBidi;
    char const * fEndOfCurrentRun;
    char const * const fBegin;
    char const * const fEnd;
    int32_t fUTF16LogicalPosition;
    SkBidiIterator::Level fLevel;
};

#if !defined(SK_DISABLE_LEGACY_SKSHAPER_FUNCTIONS)
std::unique_ptr<SkShaper::BiDiRunIterator> SkShaper::MakeIcuBiDiRunIterator(const char* utf8,
                                                                            size_t utf8Bytes,
                                                                            uint8_t bidiLevel) {
    auto unicode = SkUnicode::Make();
    if (!unicode) {
        return nullptr;
    }
    return SkShapers::unicode::BidiRunIterator(unicode.get(), utf8, utf8Bytes, bidiLevel);
}
#endif

namespace SkShapers::unicode {
std::unique_ptr<SkShaper::BiDiRunIterator> BidiRunIterator(SkUnicode* unicode,
                                                           const char* utf8,
                                                           size_t utf8Bytes,
                                                           uint8_t bidiLevel) {
    if (!unicode) {
        return nullptr;
    }
    // ubidi only accepts utf16 (though internally it basically works on utf32 chars).
    // We want an ubidi_setPara(UBiDi*, UText*, UBiDiLevel, UBiDiLevel*, UErrorCode*);
    if (!SkTFitsIn<int32_t>(utf8Bytes)) {
        SkDEBUGF("Bidi error: text too long");
        return nullptr;
    }

    int32_t utf16Units = SkUTF::UTF8ToUTF16(nullptr, 0, utf8, utf8Bytes);
    if (utf16Units < 0) {
        SkDEBUGF("Invalid utf8 input\n");
        return nullptr;
    }

    std::unique_ptr<uint16_t[]> utf16(new uint16_t[utf16Units]);
    (void)SkUTF::UTF8ToUTF16(utf16.get(), utf16Units, utf8, utf8Bytes);

    auto bidiDir = (bidiLevel % 2 == 0) ? SkBidiIterator::kLTR : SkBidiIterator::kRTL;
    SkUnicodeBidi bidi = unicode->makeBidiIterator(utf16.get(), utf16Units, bidiDir);
    if (!bidi) {
        SkDEBUGF("Bidi error\n");
        return nullptr;
    }

    return std::make_unique<SkUnicodeBidiRunIterator>(utf8, utf8 + utf8Bytes, std::move(bidi));
}
}  // namespace SkShapers::unicode
