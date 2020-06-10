
/*
* Copyright 2020 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/
#include "include/private/SkTFitsIn.h"
#include "include/private/SkTemplates.h"
#include "modules/skshaper/include/SkUnicode.h"
#include "src/utils/SkUTF.h"
#include <unicode/ubidi.h>
#include <unicode/ubrk.h>
#include <unicode/unorm2.h>
#include <unicode/uscript.h>

#include <unicode/ustring.h>
#include <unicode/utext.h>
#include <unicode/utypes.h>
#include <vector>

using ICUBiDi = std::unique_ptr<UBiDi, SkFunctionWrapper<decltype(ubidi_close), ubidi_close>>;
using ICUNorm = std::unique_ptr<UNormalizer2, SkFunctionWrapper<decltype(unorm2_close), unorm2_close>>;
using ICUText = std::unique_ptr<UText, SkFunctionWrapper<decltype(utext_close), utext_close>>;
using ICUBreakIterator = std::unique_ptr<UBreakIterator, SkFunctionWrapper<decltype(ubrk_close), ubrk_close>>;

/** Replaces invalid utf-8 sequences with REPLACEMENT CHARACTER U+FFFD. */
static inline SkUnichar utf8_next(const char** ptr, const char* end) {
    SkUnichar val = SkUTF::NextUTF8(ptr, end);
    return val < 0 ? 0xFFFD : val;
}

class SkBidiIterator_icu : public SkBidiIterator {
    ICUBiDi fBidi;
public:
    explicit SkBidiIterator_icu(ICUBiDi bidi) : fBidi(std::move(bidi)) {}
    Position getLength() override { return ubidi_getLength(fBidi.get()); }
    Level getLevelAt(Position pos) override { return ubidi_getLevelAt(fBidi.get(), pos); }

    static std::unique_ptr<SkBidiIterator> makeBidiIterator(const uint16_t utf16[], int utf16Units, Direction dir) {
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

    // ICU bidi iterator works with utf16 but clients (Flutter for instance) may work with utf8
    // This method allows the clients not to think about all these details
    static std::unique_ptr<SkBidiIterator> makeBidiIterator(const char utf8[], int utf8Units, Direction dir) {
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

        return makeBidiIterator(utf16.get(), utf16Units, dir);
    }

    // This method returns the final results only: a list of bidi regions
    // (this is all SkParagraph really needs; SkShaper however uses the iterator itself)
    static std::vector<Region> getBidiRegions(const char utf8[], int utf8Units, Direction dir) {

        auto bidiIterator = makeBidiIterator(utf8, utf8Units, dir);
        std::vector<Region> bidiRegions;
        const char* start8 = utf8;
        const char* end8 = utf8 + utf8Units;
        SkBidiIterator::Level currentLevel = 0;

        Position pos8 = 0;
        Position pos16 = 0;
        Position end16 = bidiIterator->getLength();
        while (pos16 < end16) {
            auto level = bidiIterator->getLevelAt(pos16);
            if (pos16 == 0) {
                currentLevel = level;
            } else if (level != currentLevel) {
                auto end = start8 - utf8;
                bidiRegions.emplace_back(pos8, end, currentLevel);
                currentLevel = level;
                pos8 = end;
            }
            SkUnichar u = utf8_next(&start8, end8);
            pos16 += SkUTF::ToUTF16(u);
        }
        auto end = start8 - utf8;
        if (end != pos8) {
            bidiRegions.emplace_back(pos8, end, currentLevel);
        }
        return bidiRegions;
    }
};

void SkBidiIterator::ReorderVisual(const Level runLevels[], int levelsCount,
                                   int32_t logicalFromVisual[]) {
    ubidi_reorderVisual(runLevels, levelsCount, logicalFromVisual);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Not finished yet
class SkUBreakIterator_icu : public SkUBreakIterator {
    ICUBreakIterator fBreakIterator;
 public:
    explicit SkUBreakIterator_icu(ICUBreakIterator iter) : fBreakIterator(std::move(iter)) {}
    Position first() override { return ubrk_first(fBreakIterator.get()); }
    Position next() override { return ubrk_next(fBreakIterator.get()); };
    Position preceding(Position offset) override { return ubrk_preceding(fBreakIterator.get(), offset); }
    Position following(Position offset) override { return ubrk_following(fBreakIterator.get(), offset);}
    Status status() override { return ubrk_getRuleStatus(fBreakIterator.get()); }

    static UBreakIteratorType convertType(SkUBreakIterator::UBreakType type) {
        switch (type) {
            case kLine: return UBRK_LINE;
            case kGrapheme: return UBRK_CHARACTER;
            case kWord: return UBRK_WORD;
            default:
              return UBRK_COUNT;
        }
    }

    static std::unique_ptr<SkUBreakIterator> makeUtf8BreakIterator
        (const char utf8[], int utf8Units, SkUBreakIterator::UBreakType type) {
        UErrorCode status = U_ZERO_ERROR;
        UText sUtf8UText = UTEXT_INITIALIZER;
        ICUText text(utext_openUTF8(&sUtf8UText, &utf8[0], utf8Units, &status));

        if (U_FAILURE(status)) {
            SkDEBUGF("Break error: %s", u_errorName(status));
            return nullptr;
        }
        SkASSERT(text);

        ICUBreakIterator iterator(ubrk_open(convertType(type), "en", nullptr, 0, &status));
        if (U_FAILURE(status)) {
            SkDEBUGF("Break error: %s", u_errorName(status));
        }

        ubrk_setUText(iterator.get(), text.get(), &status);
        if (U_FAILURE(status)) {
            SkDEBUGF("Break error: %s", u_errorName(status));
            return nullptr;
        }

        return std::unique_ptr<SkUBreakIterator>(new SkUBreakIterator_icu(std::move(iterator)));
    }

    static std::unique_ptr<SkUBreakIterator> makeUtf16BreakIterator
        (const char utf8[], int utf8Units, SkUBreakIterator::UBreakType type) {

        UErrorCode status = U_ZERO_ERROR;
        ICUBreakIterator iterator(ubrk_open(UBRK_WORD, uloc_getDefault(), nullptr, 0, &status));
        if (U_FAILURE(status)) {
            SkDEBUGF("Break error: %s", u_errorName(status));
            return nullptr;
        }

        int32_t utf16Units;
        u_strFromUTF8(nullptr, 0, &utf16Units, utf8,utf8Units, &status);
        if (U_FAILURE(status)) {
            SkDEBUGF("Break error: %s", u_errorName(status));
            return nullptr;
        }

        status = U_ZERO_ERROR;
        std::unique_ptr<UChar[]> utf16(new UChar[utf16Units]);
        u_strFromUTF8(utf16.get(), utf16Units, nullptr, utf8, utf8Units, &status);
        if (U_FAILURE(status)) {
            SkDEBUGF("Break error: %s", u_errorName(status));
            return nullptr;
        }

        UText sUtf16UText = UTEXT_INITIALIZER;
        ICUText utf8UText(utext_openUChars(&sUtf16UText, utf16.get(), utf16Units, &status));
        if (U_FAILURE(status)) {
            SkDEBUGF("Break error: %s", u_errorName(status));
            return nullptr;
        }

        ubrk_setUText(iterator.get(), utf8UText.get(), &status);
        if (U_FAILURE(status)) {
            SkDEBUGF("Break error: %s", u_errorName(status));
            return nullptr;
        }

        return std::unique_ptr<SkUBreakIterator>(new SkUBreakIterator_icu(std::move(iterator)));
    }
};

///////////////////////////////////////////////////////////////////////////////////////////////////
class SkUnicode_icu : public SkUnicode {
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
        return SkBidiIterator_icu::makeBidiIterator(text, count, dir);
    }
    std::unique_ptr<SkBidiIterator> makeBidiIterator(const char text[], int count,
                                                     SkBidiIterator::Direction dir) override {
        return SkBidiIterator_icu::makeBidiIterator(text, count, dir);
    }
    std::vector<SkBidiIterator::Region> getBidiRegions(const char utf8[], int utf8Units, SkBidiIterator::Direction dir) override {
        return SkBidiIterator_icu::getBidiRegions(utf8, utf8Units, dir);
    }

    std::unique_ptr<SkUBreakIterator> makeUBreakIterator
        (const char text[], int count, SkUBreakIterator::UBreakType type, SkUBreakIterator::Encoding encoding) override {
        if (encoding == SkUBreakIterator::Encoding::kUtf8) {
            return SkUBreakIterator_icu::makeUtf8BreakIterator(text, count, type);
        } else{
            return SkUBreakIterator_icu::makeUtf16BreakIterator(text, count, type);
        }
    }
};

std::unique_ptr<SkUnicode> SkUnicode_Make() { return std::make_unique<SkUnicode_icu>(); }
