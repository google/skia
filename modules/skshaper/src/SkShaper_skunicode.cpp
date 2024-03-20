/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkRefCnt.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkTFitsIn.h"
#include "modules/skshaper/include/SkShaper.h"
#include "modules/skshaper/include/SkShaper_skunicode.h"
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

#if defined(SK_UNICODE_ICU_IMPLEMENTATION)
#include "modules/skunicode/include/SkUnicode_icu.h"
#endif

#if defined(SK_UNICODE_LIBGRAPHEME_IMPLEMENTATION)
#include "modules/skunicode/include/SkUnicode_libgrapheme.h"
#endif

#if defined(SK_UNICODE_ICU4X_IMPLEMENTATION)
#include "modules/skunicode/include/SkUnicode_icu4x.h"
#endif

sk_sp<SkUnicode> get_unicode() {
#if defined(SK_UNICODE_ICU_IMPLEMENTATION)
    if (auto unicode = SkUnicodes::ICU::Make()) {
        return unicode;
    }
#endif  // defined(SK_UNICODE_ICU_IMPLEMENTATION)
#if defined(SK_UNICODE_LIBGRAPHEME_IMPLEMENTATION)
    if (auto unicode = SkUnicodes::Libgrapheme::Make()) {
        return unicode;
    }
#endif
#if defined(SK_UNICODE_ICU4X_IMPLEMENTATION)
    if (auto unicode = SkUnicodes::ICU4X::Make()) {
        return unicode;
    }
#endif
    return nullptr;
}

std::unique_ptr<SkShaper::BiDiRunIterator> SkShaper::MakeIcuBiDiRunIterator(const char* utf8,
                                                                            size_t utf8Bytes,
                                                                            uint8_t bidiLevel) {
    static auto unicode = get_unicode();
    if (!unicode) {
        return nullptr;
    }
    return SkShapers::unicode::BidiRunIterator(unicode, utf8, utf8Bytes, bidiLevel);
}
#endif  //  !defined(SK_DISABLE_LEGACY_SKSHAPER_FUNCTIONS)

namespace SkShapers::unicode {
std::unique_ptr<SkShaper::BiDiRunIterator> BidiRunIterator(sk_sp<SkUnicode> unicode,
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
