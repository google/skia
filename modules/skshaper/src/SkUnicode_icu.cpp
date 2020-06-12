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
using ICUUText = std::unique_ptr<UText, SkFunctionWrapper<decltype(utext_close), utext_close>>;
using ICUBreakIterator = std::unique_ptr<UBreakIterator, SkFunctionWrapper<decltype(ubrk_close), ubrk_close>>;

/** Replaces invalid utf-8 sequences with REPLACEMENT CHARACTER U+FFFD. */
static inline SkUnichar utf8_next(const char** ptr, const char* end) {
    SkUnichar val = SkUTF::NextUTF8(ptr, end);
    return val < 0 ? 0xFFFD : val;
}


namespace skia {

class SkUnicodeInput_icu : public SkUnicodeInput {

    SkSpan<const char> fText;
    Direction fTextDirection;
    UBreakType fBreakType;

    mutable std::unique_ptr<uint16_t[]> fUtf16;
    mutable int fUtf16Units;

    public:
    SkUnicodeInput_icu(SkSpan<const char> text, Direction dir)
        : fText(text), fTextDirection(dir), fUtf16(nullptr) { }
    SkUnicodeInput_icu(SkSpan<const char> text, UBreakType breakType)
        : fText(text), fBreakType(breakType), fUtf16(nullptr) { }

    const char* getUtf8() const override { return fText.data(); }
    int getUtf8Units() const override { return fText.size(); }

    const uint16_t* getUtf16() const override {
        if (fUtf16 == nullptr) {
            auto utf8 = fText.data();
            auto utf8Units = fText.size();
            fUtf16Units = SkUTF::UTF8ToUTF16(nullptr, 0, utf8, utf8Units);
            if (fUtf16Units < 0) {
                SkDEBUGF("Convert error: Invalid utf8 input");
                return fUtf16.get();
            }
            fUtf16 = std::unique_ptr<uint16_t[]>(new uint16_t[fUtf16Units]);
            SkDEBUGCODE(int dstLen =) SkUTF::UTF8ToUTF16(fUtf16.get(), fUtf16Units, utf8, utf8Units);
            SkASSERT(dstLen == fUtf16Units);
        }
        return fUtf16.get();
    }
    int getUtf16Units() const override {
        if (fUtf16 == nullptr) {
            getUtf16();
        }
        return fUtf16Units;
    }

    Direction getTextDirection() const override { return fTextDirection; }
    UBreakType getBreakType() const  override { return fBreakType; }
};

class SkUnicodeOutput_icu : public SkUnicodeOutput {
    UtfFormat fUtfFormat;
    std::vector<BidiRegion> fBidiRegions;
    std::vector<Position> fWords;
    std::vector<LineBreakBefore> fLineBreaks;
    std::vector<Position> fGraphemes;
    std::vector<Position> fWhitespaces;

public:
    explicit SkUnicodeOutput_icu(UtfFormat format) : fUtfFormat(format) { }
    UtfFormat getUtfFormat() const override { return fUtfFormat; }
    void setBidiRegions(std::vector<BidiRegion>&& regions) override { fBidiRegions = std::move(regions); }
    void addBidiRegion(Position start, Position end, BidiLevel level) override {
        fBidiRegions.emplace_back(start, end, level);
    }
    void setLineBreaks(std::vector<LineBreakBefore>&& breaks) override { }
    void addLineBreak(Position pos, LineBreakType lineBreakType) override {
        fLineBreaks.emplace_back(pos, lineBreakType);
    }
    void setWords(std::vector<Position>&& words) override { fWords = std::move(words); }
    void addWord(Position word) override {
        fWords.emplace_back(word);
    }
    void setGraphemes(std::vector<Position>&& graphemes) override { fGraphemes = std::move(graphemes); }
    void addGrapheme(Position grapheme) override {
        fGraphemes.emplace_back(grapheme);
    }
    void setWhitespaces(std::vector<Position>&& whitespaces) override { fWhitespaces = std::move(whitespaces); }
    void addWhitespace(Position whitespace) override {
        fWhitespaces.emplace_back(whitespace);
    }

    std::vector<BidiRegion> getBidiRegions() { return std::move(fBidiRegions); }
    std::vector<Position> getWords() { return std::move(fWords); }
    std::vector<LineBreakBefore> getLineBreaks() { return std::move(fLineBreaks); }
    std::vector<Position> getGraphemes() { return std::move(fGraphemes); }
    std::vector<Position> getWhitespaces() { return std::move(fWhitespaces); }
};

class SkUnicode_icu : public SkUnicode {
    ICUNorm fNorm;
    static UBreakIteratorType convertType(UBreakType type) {
        switch (type) {
            case UBreakType::kLines: return UBRK_LINE;
            case UBreakType::kGraphemes: return UBRK_CHARACTER;
            case UBreakType::kWords: return UBRK_WORD;
            default:
              return UBRK_COUNT;
        }
    }

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

    bool analyzeBidi(const SkUnicodeInput& input, SkUnicodeOutput& output) override {

        // Convert utf8 into utf16 since ubidi only accepts utf16
        auto utf16 = input.getUtf16();
        auto utf16Units = input.getUtf16Units();

        // Create bidi iterator
        UErrorCode status = U_ZERO_ERROR;
        ICUBiDi bidi(ubidi_openSized(utf16Units, 0, &status));
        if (U_FAILURE(status)) {
            SkDEBUGF("Bidi error: %s", u_errorName(status));
            return false;
        }
        SkASSERT(bidi);
        uint8_t bidiLevel = (input.getTextDirection() == Direction::kLTR) ? UBIDI_LTR : UBIDI_RTL;
        // The required lifetime of utf16 isn't well documented.
        // It appears it isn't used after ubidi_setPara except through ubidi_getText.
        ubidi_setPara(bidi.get(), (const UChar*)utf16, utf16Units, bidiLevel, nullptr, &status);
        if (U_FAILURE(status)) {
            SkDEBUGF("Bidi error: %s", u_errorName(status));
            return false;
        }

        // Collect the results in utf8 format
        auto utf8 = input.getUtf8();
        auto start8 = utf8;
        auto end8 = utf8 + input.getUtf8Units();
        BidiLevel currentLevel = 0;

        Position pos8 = 0;
        Position pos16 = 0;
        Position end16 = ubidi_getLength(bidi.get());
        while (pos16 < end16) {
            auto level = ubidi_getLevelAt(bidi.get(), pos16);
            if (pos16 == 0) {
                currentLevel = level;
            } else if (level != currentLevel) {
                Position end = start8 - utf8;
                output.addBidiRegion(pos8, end, currentLevel);
                currentLevel = level;
                pos8 = end;
            }
            SkUnichar u = utf8_next(&start8, end8);
            pos16 += SkUTF::ToUTF16(u);
        }
        Position end = start8 - utf8;
        if (end != pos8) {
            output.addBidiRegion(pos8, end, currentLevel);
        }
        return true;
    }
    bool analyzeWords(const SkUnicodeInput& input, SkUnicodeOutput& output) override {
        // Convert utf8 into utf16
        UErrorCode status = U_ZERO_ERROR;

        UBreakIteratorType breakType = convertType(UBreakType::kWords);
        ICUBreakIterator iterator(ubrk_open(breakType, uloc_getDefault(), nullptr, 0, &status));
        if (U_FAILURE(status)) {
            SkDEBUGF("Break error: %s", u_errorName(status));
            return false;
        }
        SkASSERT(iterator);

        const UChar* utf16 = (UChar*)input.getUtf16();
        size_t utf16Units = input.getUtf16Units();

        UText sUtf16UText = UTEXT_INITIALIZER;
        ICUUText utf16UText(utext_openUChars(&sUtf16UText, utf16, utf16Units, &status));
        if (U_FAILURE(status)) {
            SkDEBUGF("Break error: %s", u_errorName(status));
            return false;
        }

        ubrk_setUText(iterator.get(), utf16UText.get(), &status);
        if (U_FAILURE(status)) {
            SkDEBUGF("Break error: %s", u_errorName(status));
            return false;
        }

        // Get the words
        int32_t pos = ubrk_first(iterator.get());
        while (pos != UBRK_DONE) {
            output.addWord(pos);
            pos = ubrk_next(iterator.get());
        }

        return true;
    }
    bool analyzeLineBreaks(const SkUnicodeInput& input, SkUnicodeOutput& output) override {

        // Create an iterator
        auto utf8 = input.getUtf8();
        auto utf8Units = input.getUtf8Units();

        UErrorCode status = U_ZERO_ERROR;
        UText sUtf8UText = UTEXT_INITIALIZER;
        ICUUText text(utext_openUTF8(&sUtf8UText, &utf8[0], utf8Units, &status));

        if (U_FAILURE(status)) {
            SkDEBUGF("Break error: %s", u_errorName(status));
            return false;
        }
        SkASSERT(text);

        ICUBreakIterator iterator(ubrk_open(convertType(UBreakType::kLines), uloc_getDefault(), nullptr, 0, &status));
        if (U_FAILURE(status)) {
            SkDEBUGF("Break error: %s", u_errorName(status));
        }

        ubrk_setUText(iterator.get(), text.get(), &status);
        if (U_FAILURE(status)) {
            SkDEBUGF("Break error: %s", u_errorName(status));
            return false;
        }

        auto pos = ubrk_first(iterator.get());
        while (pos != UBRK_DONE) {
            auto status = ubrk_getRuleStatus(iterator.get()) == UBRK_LINE_HARD
                          ? LineBreakType::kHardLineBreak
                          : LineBreakType::kSoftLineBreak;
            output.addLineBreak(pos, status);
            pos = ubrk_next(iterator.get());
        }

        return true;
    }
    bool analyzeGraphemes(const SkUnicodeInput& input, SkUnicodeOutput& output) override {

        // Create an iterator
        auto utf8 = input.getUtf8();
        auto utf8Units = input.getUtf8Units();

        UErrorCode status = U_ZERO_ERROR;
        UText sUtf8UText = UTEXT_INITIALIZER;
        ICUUText text(utext_openUTF8(&sUtf8UText, &utf8[0], utf8Units, &status));

        if (U_FAILURE(status)) {
            SkDEBUGF("Break error: %s", u_errorName(status));
            return false;
        }
        SkASSERT(text);

        ICUBreakIterator iterator(ubrk_open(convertType(UBreakType::kGraphemes), uloc_getDefault(), nullptr, 0, &status));
        if (U_FAILURE(status)) {
            SkDEBUGF("Break error: %s", u_errorName(status));
        }

        ubrk_setUText(iterator.get(), text.get(), &status);
        if (U_FAILURE(status)) {
            SkDEBUGF("Break error: %s", u_errorName(status));
            return false;
        }

        auto pos = ubrk_first(iterator.get());
        while (pos != UBRK_DONE) {
            output.addGrapheme(pos);
            pos = ubrk_next(iterator.get());
        }
        return true;
    }

    bool analyzeWhitespaces(const SkUnicodeInput& input, SkUnicodeOutput& output) override {
        const char* start = input.getUtf8();
        const char* end = start + input.getUtf8Units();
        const char* ch = start;
        while (ch < end) {
            auto index = ch - start;
            auto unichar = utf8_next(&ch, end);
            if (u_isWhitespace(unichar)) {
                auto ending = ch - start;
                for (auto k = index; k < ending; ++k) {
                  output.addWhitespace(k);
                }
            }
        }
        return true;
    }
    bool getBidiRegions(const char utf8[], int utf8Units, Direction dir, std::vector<BidiRegion>& results) override {
        SkUnicodeInput_icu input(SkSpan<const char>(utf8, utf8Units), dir);
        SkUnicodeOutput_icu output(UtfFormat::kUTF8);
        if (!analyzeBidi(input, output)) {
            return false;
        }
        results = output.getBidiRegions();
        return true;
    }
    bool getLineBreaks(const char utf8[], int utf8Units, std::vector<LineBreakBefore> & results) override {
        SkUnicodeInput_icu input(SkSpan<const char>(utf8, utf8Units), UBreakType::kLines);
        SkUnicodeOutput_icu output(UtfFormat::kUTF8);
        if (!analyzeLineBreaks(input, output)) {
            return false;
        }
        results = output.getLineBreaks();
        return true;
    }
    bool getWords(const char utf8[], int utf8Units, std::vector<Position> & results) override {
        SkUnicodeInput_icu input(SkSpan<const char>(utf8, utf8Units), UBreakType::kWords);
        SkUnicodeOutput_icu output(UtfFormat::kUTF8);
        if (!analyzeWords(input, output)) {
            return false;
        }
        results = output.getWords();
        return true;
    }
    bool getGraphemes(const char utf8[], int utf8Units, std::vector<Position> & results) override {
        SkUnicodeInput_icu input(SkSpan<const char>(utf8, utf8Units), UBreakType::kGraphemes);
        SkUnicodeOutput_icu output(UtfFormat::kUTF8);
        if (!analyzeGraphemes(input, output)) {
            return false;
        }
        results = output.getGraphemes();
        return true;
    }
    bool getWhitespaces(const char utf8[], int utf8Units, std::vector<Position> & results) override {
        SkUnicodeInput_icu input(SkSpan<const char>(utf8, utf8Units), UBreakType::kLines);
        SkUnicodeOutput_icu output(UtfFormat::kUTF8);
        if (!analyzeWhitespaces(input, output)) {
            return false;
        }
        results = output.getWhitespaces();
        return true;
    }
};

void SkUnicode::ReorderVisual(const BidiLevel runLevels[], int levelsCount, int32_t logicalFromVisual[]) {
    ubidi_reorderVisual(runLevels, levelsCount, logicalFromVisual);
}

std::unique_ptr<SkUnicode> SkUnicode_Make() { return std::make_unique<SkUnicode_icu>(); }

}


