/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "modules/skunicode/src/SkUnicode_icu_bidi.h"

#include "include/core/SkTypes.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkTFitsIn.h"
#include "src/base/SkUTF.h"

#include <unicode/ubidi.h>
#include <unicode/umachine.h>
#include <unicode/utypes.h>

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace {
using SkUnicodeBidi = std::unique_ptr<UBiDi, SkBidiFactory::BidiCloseCallback>;

class SkBidiIterator_icu : public SkBidiIterator {
public:
    SkBidiIterator_icu(SkUnicodeBidi bidi, sk_sp<SkBidiFactory> fact)
            : fBidi(std::move(bidi)), fBidiFact(std::move(fact)) {}

    Position getLength() override { return fBidiFact->bidi_getLength(fBidi.get()); }

    Level getLevelAt(Position pos) override { return fBidiFact->bidi_getLevelAt(fBidi.get(), pos); }

private:
    SkUnicodeBidi fBidi;
    sk_sp<SkBidiFactory> fBidiFact;
};
}  // namespace

std::unique_ptr<SkBidiIterator> SkBidiFactory::MakeIterator(const uint16_t utf16[],
                                                            int utf16Units,
                                                            SkBidiIterator::Direction dir) const {
    UErrorCode status = U_ZERO_ERROR;
    SkUnicodeBidi bidi(this->bidi_openSized(utf16Units, 0, &status), this->bidi_close_callback());
    if (U_FAILURE(status)) {
        SkDEBUGF("Bidi error: %s", this->errorName(status));
        return nullptr;
    }
    SkASSERT(bidi);
    uint8_t bidiLevel = (dir == SkBidiIterator::kLTR) ? UBIDI_LTR : UBIDI_RTL;
    // The required lifetime of utf16 isn't well documented.
    // It appears it isn't used after ubidi_setPara except through ubidi_getText.
    this->bidi_setPara(bidi.get(), (const UChar*)utf16, utf16Units, bidiLevel, nullptr, &status);
    if (U_FAILURE(status)) {
        SkDEBUGF("Bidi error: %s", this->errorName(status));
        return nullptr;
    }
    return std::unique_ptr<SkBidiIterator>(
            new SkBidiIterator_icu(std::move(bidi), sk_ref_sp(this)));
}

std::unique_ptr<SkBidiIterator> SkBidiFactory::MakeIterator(const char utf8[],
                                                            int utf8Units,
                                                            SkBidiIterator::Direction dir) const {
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

bool SkBidiFactory::ExtractBidi(const char utf8[],
                                int utf8Units,
                                SkUnicode::TextDirection dir,
                                std::vector<SkUnicode::BidiRegion>* bidiRegions) const {
    // Convert to UTF16 since for now bidi iterator only operates on utf16
    auto utf16 = SkUnicode::convertUtf8ToUtf16(utf8, utf8Units);

    // Create bidi iterator
    UErrorCode status = U_ZERO_ERROR;
    SkUnicodeBidi bidi(this->bidi_openSized(utf16.size(), 0, &status), this->bidi_close_callback());
    if (U_FAILURE(status)) {
        SkDEBUGF("Bidi error: %s", this->errorName(status));
        return false;
    }
    SkASSERT(bidi);
    uint8_t bidiLevel = (dir == SkUnicode::TextDirection::kLTR) ? UBIDI_LTR : UBIDI_RTL;
    // The required lifetime of utf16 isn't well documented.
    // It appears it isn't used after ubidi_setPara except through ubidi_getText.
    this->bidi_setPara(
            bidi.get(), (const UChar*)utf16.c_str(), utf16.size(), bidiLevel, nullptr, &status);
    if (U_FAILURE(status)) {
        SkDEBUGF("Bidi error: %s", this->errorName(status));
        return false;
    }

    // Iterate through bidi regions and the result positions into utf8
    const char* start8 = utf8;
    const char* end8 = utf8 + utf8Units;
    SkUnicode::BidiLevel currentLevel = 0;

    SkUnicode::Position pos8 = 0;
    SkUnicode::Position pos16 = 0;
    SkUnicode::Position end16 = this->bidi_getLength(bidi.get());

    if (end16 == 0) {
        return true;
    }
    if (this->bidi_getDirection(bidi.get()) != UBIDI_MIXED) {
        // The entire paragraph is unidirectional.
        bidiRegions->emplace_back(0, utf8Units, this->bidi_getLevelAt(bidi.get(), 0));
        return true;
    }

    while (pos16 < end16) {
        auto level = this->bidi_getLevelAt(bidi.get(), pos16);
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
