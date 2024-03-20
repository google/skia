/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkTypes.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkTFitsIn.h"
#include "include/private/base/SkTemplates.h"
#include "modules/skunicode/src/SkUnicode_icu_bidi.h"
#include "src/base/SkUTF.h"
#include <unicode/umachine.h>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#ifdef SK_UNICODE_ICU_IMPLEMENTATION

#include "modules/skunicode/src/SkUnicode_icu.h"

const char* SkUnicode_IcuBidi::errorName(UErrorCode status) {
    return SkGetICULib()->f_u_errorName(status);
}

void SkUnicode_IcuBidi::bidi_close(UBiDi* bidi) {
    SkGetICULib()->f_ubidi_close(bidi);
}
UBiDiDirection SkUnicode_IcuBidi::bidi_getDirection(const UBiDi* bidi) {
    return SkGetICULib()->f_ubidi_getDirection(bidi);
}
SkBidiIterator::Position SkUnicode_IcuBidi::bidi_getLength(const UBiDi* bidi) {
    return SkGetICULib()->f_ubidi_getLength(bidi);
}
SkBidiIterator::Level SkUnicode_IcuBidi::bidi_getLevelAt(const UBiDi* bidi, int pos) {
    return SkGetICULib()->f_ubidi_getLevelAt(bidi, pos);
}
UBiDi* SkUnicode_IcuBidi::bidi_openSized(int32_t maxLength, int32_t maxRunCount, UErrorCode* pErrorCode) {
    return SkGetICULib()->f_ubidi_openSized(maxLength, maxRunCount, pErrorCode);
}
void SkUnicode_IcuBidi::bidi_setPara(UBiDi* bidi,
                         const UChar* text,
                         int32_t length,
                         UBiDiLevel paraLevel,
                         UBiDiLevel* embeddingLevels,
                         UErrorCode* status) {
    return SkGetICULib()->f_ubidi_setPara(bidi, text, length, paraLevel, embeddingLevels, status);
}
void SkUnicode_IcuBidi::bidi_reorderVisual(const SkUnicode::BidiLevel runLevels[],
                               int levelsCount,
                               int32_t logicalFromVisual[]) {
    SkGetICULib()->f_ubidi_reorderVisual(runLevels, levelsCount, logicalFromVisual);
}

#else  // SK_UNICODE_ICU_IMPLEMENTATION

const char* SkUnicode_IcuBidi::errorName(UErrorCode status) {
    return u_errorName_skia(status);
}
void SkUnicode_IcuBidi::bidi_close(UBiDi* bidi) {
    ubidi_close_skia(bidi);
}
UBiDiDirection SkUnicode_IcuBidi::bidi_getDirection(const UBiDi* bidi) {
    return ubidi_getDirection_skia(bidi);
}
SkBidiIterator::Position SkUnicode_IcuBidi::bidi_getLength(const UBiDi* bidi) {
    return ubidi_getLength_skia(bidi);
}
SkBidiIterator::Level SkUnicode_IcuBidi::bidi_getLevelAt(const UBiDi* bidi, int pos) {
    return ubidi_getLevelAt_skia(bidi, pos);
}
UBiDi* SkUnicode_IcuBidi::bidi_openSized(int32_t maxLength, int32_t maxRunCount, UErrorCode* pErrorCode) {
    return ubidi_openSized_skia(maxLength, maxRunCount, pErrorCode);
}
void SkUnicode_IcuBidi::bidi_setPara(UBiDi* bidi,
                         const UChar* text,
                         int32_t length,
                         UBiDiLevel paraLevel,
                         UBiDiLevel* embeddingLevels,
                         UErrorCode* status) {
    return ubidi_setPara_skia(bidi, text, length, paraLevel, embeddingLevels, status);
}
void SkUnicode_IcuBidi::bidi_reorderVisual(const SkUnicode::BidiLevel runLevels[],
                               int levelsCount,
                               int32_t logicalFromVisual[]) {
    ubidi_reorderVisual_skia(runLevels, levelsCount, logicalFromVisual);
}

#endif  // SK_UNICODE_ICU_IMPLEMENTATION

namespace {
using SkUnicodeBidi = std::unique_ptr<UBiDi, SkFunctionObject<SkUnicode_IcuBidi::bidi_close>>;

class SkBidiIterator_icu : public SkBidiIterator {
public:
    SkBidiIterator_icu(SkUnicodeBidi bidi) : fBidi(std::move(bidi)) {}

    Position getLength() override { return SkUnicode_IcuBidi::bidi_getLength(fBidi.get()); }

    Level getLevelAt(Position pos) override { return SkUnicode_IcuBidi::bidi_getLevelAt(fBidi.get(), pos); }

private:
    SkUnicodeBidi fBidi;
};
}  // namespace

std::unique_ptr<SkBidiIterator> SkUnicode_IcuBidi::MakeIterator(const uint16_t utf16[],
                                                                int utf16Units,
                                                                SkBidiIterator::Direction dir) {
    UErrorCode status = U_ZERO_ERROR;
    SkUnicodeBidi bidi(SkUnicode_IcuBidi::bidi_openSized(utf16Units, 0, &status));
    if (U_FAILURE(status)) {
        SkDEBUGF("Bidi error: %s", SkUnicode_IcuBidi::errorName(status));
        return nullptr;
    }
    SkASSERT(bidi);
    uint8_t bidiLevel = (dir == SkBidiIterator::kLTR) ? UBIDI_LTR : UBIDI_RTL;
    // The required lifetime of utf16 isn't well documented.
    // It appears it isn't used after ubidi_setPara except through ubidi_getText.
    SkUnicode_IcuBidi::bidi_setPara(bidi.get(), (const UChar*)utf16, utf16Units, bidiLevel, nullptr, &status);
    if (U_FAILURE(status)) {
        SkDEBUGF("Bidi error: %s", SkUnicode_IcuBidi::errorName(status));
        return nullptr;
    }
    return std::unique_ptr<SkBidiIterator>(new SkBidiIterator_icu(std::move(bidi)));
}

std::unique_ptr<SkBidiIterator> SkUnicode_IcuBidi::MakeIterator(const char utf8[],
                                                                int utf8Units,
                                                                SkBidiIterator::Direction dir) {
    // Convert utf8 into utf16 since ubidi only accepts utf16
    if (!SkTFitsIn<int32_t>(utf8Units)) {
        SkDEBUGF("Bidi error: text too long");
        return nullptr;
    }

    // Getting the length like this seems to always set U_BUFFER_OVERFLOW_ERROR
    int utf16Units = SkUTF::UTF8ToUTF16(nullptr, 0, utf8, utf8Units);
    if (utf16Units < 0) {
        SkDEBUGF("Bidi error: Invalid utf8 input");
        return nullptr;
    }
    std::unique_ptr<uint16_t[]> utf16(new uint16_t[utf16Units]);
    SkDEBUGCODE(int dstLen =) SkUTF::UTF8ToUTF16(utf16.get(), utf16Units, utf8, utf8Units);
    SkASSERT(dstLen == utf16Units);

    return MakeIterator(utf16.get(), utf16Units, dir);
}

/** Replaces invalid utf-8 sequences with REPLACEMENT CHARACTER U+FFFD. */
static inline SkUnichar utf8_next(const char** ptr, const char* end) {
    SkUnichar val = SkUTF::NextUTF8(ptr, end);
    return val < 0 ? 0xFFFD : val;
}

bool SkUnicode_IcuBidi::ExtractBidi(const char utf8[],
                                    int utf8Units,
                                    SkUnicode::TextDirection dir,
                                    std::vector<SkUnicode::BidiRegion>* bidiRegions) {
    // Convert to UTF16 since for now bidi iterator only operates on utf16
    auto utf16 = SkUnicode::convertUtf8ToUtf16(utf8, utf8Units);

    // Create bidi iterator
    UErrorCode status = U_ZERO_ERROR;
    SkUnicodeBidi bidi(SkUnicode_IcuBidi::bidi_openSized(utf16.size(), 0, &status));
    if (U_FAILURE(status)) {
        SkDEBUGF("Bidi error: %s", SkUnicode_IcuBidi::errorName(status));
        return false;
    }
    SkASSERT(bidi);
    uint8_t bidiLevel = (dir == SkUnicode::TextDirection::kLTR) ? UBIDI_LTR : UBIDI_RTL;
    // The required lifetime of utf16 isn't well documented.
    // It appears it isn't used after ubidi_setPara except through ubidi_getText.
    SkUnicode_IcuBidi::bidi_setPara(bidi.get(), (const UChar*)utf16.c_str(), utf16.size(), bidiLevel, nullptr,
                        &status);
    if (U_FAILURE(status)) {
        SkDEBUGF("Bidi error: %s", SkUnicode_IcuBidi::errorName(status));
        return false;
    }

    // Iterate through bidi regions and the result positions into utf8
    const char* start8 = utf8;
    const char* end8 = utf8 + utf8Units;
    SkUnicode::BidiLevel currentLevel = 0;

    SkUnicode::Position pos8 = 0;
    SkUnicode::Position pos16 = 0;
    SkUnicode::Position end16 = SkUnicode_IcuBidi::bidi_getLength(bidi.get());

    if (end16 == 0) {
        return true;
    }
    if (SkUnicode_IcuBidi::bidi_getDirection(bidi.get()) != UBIDI_MIXED) {
        // The entire paragraph is unidirectional.
        bidiRegions->emplace_back(0, utf8Units, SkUnicode_IcuBidi::bidi_getLevelAt(bidi.get(), 0));
        return true;
    }

    while (pos16 < end16) {
        auto level = SkUnicode_IcuBidi::bidi_getLevelAt(bidi.get(), pos16);
        if (pos16 == 0) {
            currentLevel = level;
        } else if (level != currentLevel) {
            SkUnicode::Position end = start8 - utf8;
            bidiRegions->emplace_back(pos8, end, currentLevel);
            currentLevel = level;
            pos8 = end;
        }
        SkUnichar u = utf8_next(&start8, end8);
        pos16 += SkUTF::ToUTF16(u);
    }
    SkUnicode::Position end = start8 - utf8;
    if (end != pos8) {
        bidiRegions->emplace_back(pos8, end, currentLevel);
    }
    return true;
}
