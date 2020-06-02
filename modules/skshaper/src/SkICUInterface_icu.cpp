
/*
* Copyright 2020 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/
#include "include/private/SkTemplates.h"
#include "modules/skshaper/include/SkICUInterface.h"
#include "src/utils/SkUTF.h"
#include <unicode/ubidi.h>
#include <unicode/unorm2.h>
#include <unicode/uscript.h>
using ICUBiDi = std::unique_ptr<UBiDi, SkFunctionWrapper<decltype(ubidi_close), ubidi_close>>;
using ICUNorm = std::unique_ptr<UNormalizer2, SkFunctionWrapper<decltype(unorm2_close), unorm2_close>>;
class SkBidiIterator_icu : public SkBidiIterator {
    ICUBiDi fBidi;
public:
    SkBidiIterator_icu(ICUBiDi bidi) : fBidi(std::move(bidi)) {}
    Position getLength() override { return ubidi_getLength(fBidi.get()); }
    Level getLevelAt(Position pos) override { return ubidi_getLevelAt(fBidi.get(), pos); }
    static std::unique_ptr<SkBidiIterator> MakeICU(const uint16_t utf16[], int utf16Units, Direction dir) {
        UErrorCode status = U_ZERO_ERROR;
        ICUBiDi bidi(ubidi_openSized(utf16Units, 0, &status));
        if (U_FAILURE(status)) {
            SkDEBUGF("Bidi error: %s", u_errorName(status));
            return nullptr;
        }
        SkASSERT(bidi);
        uint8_t bidiLevel = (dir == SkBidiIterator::kLTR) ? UBIDI_LTR : UBIDI_RTL;
        // The required lifetime of utf16 isn't well documented.
        // It appears it isn't used after ubidi_setPara except through ubidi_getText.
        ubidi_setPara(bidi.get(), (const UChar*)utf16, utf16Units, bidiLevel, nullptr, &status);
        if (U_FAILURE(status)) {
            SkDEBUGF("Bidi error: %s", u_errorName(status));
            return nullptr;
        }
        return std::unique_ptr<SkBidiIterator>(new SkBidiIterator_icu(std::move(bidi)));
    }
};
void SkBidiIterator::ReorderVisual(const Level runLevels[], int levelsCount,
                                   int32_t logicalFromVisual[]) {
    ubidi_reorderVisual(runLevels, levelsCount, logicalFromVisual);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
class SkICUInterface_icu : public SkICUInterface {
    ICUNorm fNorm;
public:
    ScriptID unicharToScriptID(SkUnichar uni) override {
        UErrorCode status = U_ZERO_ERROR;
        UScriptCode scriptCode = uscript_getScript(uni, &status);
        if (U_FAILURE(status)) {
            return -1; // HB_SCRIPT_UNKNOWN;
        }
        return scriptCode; // hb_icu_script_to_script (scriptCode);
    }
    CombiningClass unicharToCombiningClass(SkUnichar uni) override {
        return u_getCombiningClass(uni);
    }
    GeneralCategory unicharToGeneralCategory(SkUnichar uni) override {
        switch (u_getIntPropertyValue(uni, UCHAR_GENERAL_CATEGORY)) {
            default: break;
        }
        return 0;
    }
    SkUnichar mirrorUnichar(SkUnichar uni) override {
        return u_charMirror(uni);
    }
    bool composeUnichars(SkUnichar a, SkUnichar b, SkUnichar* ab) override {
        SkUnichar ret = unorm2_composePair(fNorm.get(), a, b);
        if (ret < 0) {
            // *ab = 0 or some sentinel?
            return false;
        }
        *ab = ret;
        return true;
    }
    bool decomposeUnichar(SkUnichar ab, SkUnichar* a, SkUnichar* b) override {
        uint16_t storage[4];
        UErrorCode err = U_ZERO_ERROR;
        int count = unorm2_getRawDecomposition(fNorm.get(), ab, (UChar*)storage,
                                               SK_ARRAY_COUNT(storage), &err);
        if (U_FAILURE(err) || count < 0) {
            // *a = *b = 0 or some sentinel?
            return false;
        }
        count = SkUTF::CountUTF16(storage, count * sizeof(uint16_t));
        if (count <= 0 || count > 2) {
            return false;
        }
        const uint16_t* decomp = storage;
        const uint16_t* endDecomp = decomp + count;
        if (count == 1) {
            *a = SkUTF::NextUTF16(&decomp, endDecomp);
            *b = 0;
            return *a != ab;
        } else if (count == 2) {
            *a = SkUTF::NextUTF16(&decomp, endDecomp);
            *b = SkUTF::NextUTF16(&decomp, endDecomp);
        }
        return true;
    }
    std::unique_ptr<SkBidiIterator> makeBidiIterator(const uint16_t text[], int count,
                                                     SkBidiIterator::Direction dir) override {
        return SkBidiIterator_icu::MakeICU(text, count, dir);
    }
    std::unique_ptr<SkUBreakIterator> makeUBreakIterator() override {
        return nullptr;
    }
};

std::unique_ptr<SkICUInterface> SkICUInterface_MakeICU() {
    return std::make_unique<SkICUInterface_icu>();
}
