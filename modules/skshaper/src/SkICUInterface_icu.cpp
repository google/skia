/*
* Copyright 2020 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "include/private/SkTemplates.h"
#include "modules/skshaper/include/SkICUInterface.h"

#include <unicode/ubidi.h>

using ICUBiDi = std::unique_ptr<UBiDi, SkFunctionWrapper<decltype(ubidi_close), ubidi_close>>;

class SkBIDIIterator_icu : public SkBIDIIterator {
    ICUBiDi fBidi;
public:
    SkBIDIIterator_icu(ICUBiDi bidi) : fBidi(std::move(bidi)) {}

    Position getLength() override { return ubidi_getLength(fBidi.get()); }
    Level getLevelAt(Position pos) override { return ubidi_getLevelAt(fBidi.get(), pos); }

    static std::unique_ptr<SkBIDIIterator> Make(const uint16_t utf16[], int utf16Units,
                                                Direction dir) {
        UErrorCode status = U_ZERO_ERROR;

        ICUBiDi bidi(ubidi_openSized(utf16Units, 0, &status));
        if (U_FAILURE(status)) {
            SkDEBUGF("Bidi error: %s", u_errorName(status));
            return nullptr;
        }
        SkASSERT(bidi);

        uint8_t bidiLevel = (dir == SkBIDIIterator::kLTR) ? UBIDI_LTR : UBIDI_RTL;
        // The required lifetime of utf16 isn't well documented.
        // It appears it isn't used after ubidi_setPara except through ubidi_getText.
        ubidi_setPara(bidi.get(), (const UChar*)utf16, utf16Units, bidiLevel, nullptr, &status);
        if (U_FAILURE(status)) {
            SkDEBUGF("Bidi error: %s", u_errorName(status));
            return nullptr;
        }

        return std::unique_ptr<SkBIDIIterator>(new SkBIDIIterator_icu(std::move(bidi)));
    }
};

std::unique_ptr<SkBIDIIterator> SkBIDIIterator::MakeICU(const uint16_t utf16[], int utf16Units,
                                                        Direction dir) {
    return SkBIDIIterator_icu::Make(utf16, utf16Units, dir);
}
