                                           /*
* Copyright 2022 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "include/core/SkSpan.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/private/SkBitmaskEnum.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTo.h"
#include "modules/skunicode/include/SkUnicode.h"
#include "modules/skunicode/src/SkUnicode_client.h"
#include "modules/skunicode/src/SkUnicode_icu_bidi.h"
#include "src/base/SkUTF.h"

#include <algorithm>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <unicode/ubidi.h>
#include <unicode/ubrk.h>
#include <unicode/uchar.h>
#include <unicode/uloc.h>
#include <unicode/uscript.h>
#include <unicode/ustring.h>
#include <unicode/utext.h>
#include <unicode/utypes.h>


#ifndef SK_UNICODE_ICU_IMPLEMENTATION
const char* SkUnicode_IcuBidi::errorName(UErrorCode status) {
    return u_errorName(status);
}
void SkUnicode_IcuBidi::bidi_close(UBiDi* bidi) {
    ubidi_close(bidi);
}
UBiDiDirection SkUnicode_IcuBidi::bidi_getDirection(const UBiDi* bidi) {
    return ubidi_getDirection(bidi);
}
SkBidiIterator::Position SkUnicode_IcuBidi::bidi_getLength(const UBiDi* bidi) {
    return ubidi_getLength(bidi);
}
SkBidiIterator::Level SkUnicode_IcuBidi::bidi_getLevelAt(const UBiDi* bidi, int pos) {
    return ubidi_getLevelAt(bidi, pos);
}
UBiDi* SkUnicode_IcuBidi::bidi_openSized(int32_t maxLength, int32_t maxRunCount, UErrorCode* pErrorCode) {
    return ubidi_openSized(maxLength, maxRunCount, pErrorCode);
}
void SkUnicode_IcuBidi::bidi_setPara(UBiDi* bidi,
                         const UChar* text,
                         int32_t length,
                         UBiDiLevel paraLevel,
                         UBiDiLevel* embeddingLevels,
                         UErrorCode* status) {
    return ubidi_setPara(bidi, text, length, paraLevel, embeddingLevels, status);
}
void SkUnicode_IcuBidi::bidi_reorderVisual(const SkUnicode::BidiLevel runLevels[],
                               int levelsCount,
                               int32_t logicalFromVisual[]) {
    ubidi_reorderVisual(runLevels, levelsCount, logicalFromVisual);
}
#endif

class SkUnicode_client : public SkUnicode {
public:
    struct Data {
        SkSpan<const char> fText8;
        SkSpan<const char16_t> fText16;
        std::vector<Position> fWords;
        std::vector<SkUnicode::Position> fGraphemeBreaks;
        std::vector<SkUnicode::LineBreakBefore> fLineBreaks;
        Data(SkSpan<char> text,
             std::vector<SkUnicode::Position> words,
             std::vector<SkUnicode::Position> graphemeBreaks,
             std::vector<SkUnicode::LineBreakBefore> lineBreaks)
            : fText8(text)
            , fText16(SkSpan<const char16_t>(nullptr, 0))
            , fWords(std::move(words))
            , fGraphemeBreaks(std::move(graphemeBreaks))
            , fLineBreaks(std::move(lineBreaks)) {
        }

        void reset() {
            fText8 = SkSpan<const char>(nullptr, 0);
            fText16 = SkSpan<const char16_t>(nullptr, 0);
            fGraphemeBreaks.clear();
            fLineBreaks.clear();
        }
    };
    SkUnicode_client() = delete;
    SkUnicode_client(SkSpan<char> text,
                     std::vector<SkUnicode::Position> words,
                     std::vector<SkUnicode::Position> graphemeBreaks,
                     std::vector<SkUnicode::LineBreakBefore> lineBreaks)
            : fData(std::make_shared<Data>(text,
                                           std::move(words),
                                           std::move(graphemeBreaks),
                                           std::move(lineBreaks))) {}
    SkUnicode_client(const SkUnicode_client* origin)
            : fData(origin->fData) {}


    std::unique_ptr<SkUnicode> copy() override {
        return std::make_unique<SkUnicode_client>(this);
    }

    ~SkUnicode_client() override = default;

    void reset() { fData->reset(); }
    // For SkShaper
    std::unique_ptr<SkBidiIterator> makeBidiIterator(const uint16_t text[], int count,
                                                     SkBidiIterator::Direction dir) override;
    std::unique_ptr<SkBidiIterator> makeBidiIterator(const char text[],
                                                     int count,
                                                     SkBidiIterator::Direction dir) override;
    std::unique_ptr<SkBreakIterator> makeBreakIterator(const char locale[],
                                                       BreakType breakType) override;
    std::unique_ptr<SkBreakIterator> makeBreakIterator(BreakType breakType) override;
    // For SkParagraph
    bool getBidiRegions(const char utf8[],
                        int utf8Units,
                        TextDirection dir,
                        std::vector<BidiRegion>* results) override {
        return SkUnicode::extractBidi(utf8, utf8Units, dir, results);
    }

    // TODO: Take if from the Client or hard code here?
    static bool isControl(SkUnichar utf8) {
        return (utf8 < ' ') || (utf8 >= 0x7f && utf8 <= 0x9f) ||
               (utf8 >= 0x200D && utf8 <= 0x200F) ||
               (utf8 >= 0x202A && utf8 <= 0x202E);
    }

    static bool isWhitespace(SkUnichar unichar) {
        std::u16string whitespaces =
       u"\u0009" // character tabulation
        "\u000A" // line feed
        "\u000B" // line tabulation
        "\u000C" // form feed
        "\u000D" // carriage return
        "\u0020" // space
      //"\u0085" // next line
      //"\u00A0" // no-break space
        "\u1680" // ogham space mark
        "\u2000" // en quad
        "\u2001" // em quad
        "\u2002" // en space
        "\u2003" // em space
        "\u2004" // three-per-em space
        "\u2005" // four-per-em space
        "\u2006" // six-per-em space
      //"\u2007" // figure space
        "\u2008" // punctuation space
        "\u2009" // thin space
        "\u200A" // hair space
        "\u2028" // line separator
        "\u2029" // paragraph separator
      //"\u202F" // narrow no-break space
        "\u205F" // medium mathematical space
        "\u3000";// ideographic space
        return whitespaces.find(unichar) != std::u16string::npos;
    }

    static bool isSpace(SkUnichar unichar) {
        std::u16string spaces =
       u"\u0009" // character tabulation
        "\u000A" // line feed
        "\u000B" // line tabulation
        "\u000C" // form feed
        "\u000D" // carriage return
        "\u0020" // space
        "\u0085" // next line
        "\u00A0" // no-break space
        "\u1680" // ogham space mark
        "\u2000" // en quad
        "\u2001" // em quad
        "\u2002" // en space
        "\u2003" // em space
        "\u2004" // three-per-em space
        "\u2005" // four-per-em space
        "\u2006" // six-per-em space
        "\u2007" // figure space
        "\u2008" // punctuation space
        "\u2009" // thin space
        "\u200A" // hair space
        "\u2028" // line separator
        "\u2029" // paragraph separator
        "\u202F" // narrow no-break space
        "\u205F" // medium mathematical space
        "\u3000"; // ideographic space
        return spaces.find(unichar) != std::u16string::npos;
    }

    static bool isTabulation(SkUnichar utf8) {
        return utf8 == '\t';
    }

    static bool isHardBreak(SkUnichar utf8) {
        return utf8 == '\n';
    }

    bool computeCodeUnitFlags(char utf8[],
                              int utf8Units,
                              bool replaceTabs,
                              SkTArray<SkUnicode::CodeUnitFlags, true>* results) override {
        results->clear();
        results->push_back_n(utf8Units + 1, CodeUnitFlags::kNoCodeUnitFlag);
        for (auto& lineBreak : fData->fLineBreaks) {
            (*results)[lineBreak.pos] |=
                lineBreak.breakType == LineBreakType::kHardLineBreak
                    ? CodeUnitFlags::kHardLineBreakBefore
                    : CodeUnitFlags::kSoftLineBreakBefore;
        }
        for (auto& grapheme : fData->fGraphemeBreaks) {
            (*results)[grapheme] |= CodeUnitFlags::kGraphemeStart;
        }
        const char* current = utf8;
        const char* end = utf8 + utf8Units;
        while (current < end) {
            auto before = current - utf8;
            SkUnichar unichar = SkUTF::NextUTF8(&current, end);
            if (unichar < 0) unichar = 0xFFFD;
            auto after = current - utf8;
            if (replaceTabs && SkUnicode_client::isTabulation(unichar)) {
                results->at(before) |= SkUnicode::kTabulation;
                if (replaceTabs) {
                    unichar = ' ';
                    utf8[before] = ' ';
                }
            }
            for (auto i = before; i < after; ++i) {
                if (SkUnicode_client::isSpace(unichar)) {
                    results->at(i) |= SkUnicode::kPartOfIntraWordBreak;
                }
                if (SkUnicode_client::isWhitespace(unichar)) {
                    results->at(i) |= SkUnicode::kPartOfWhiteSpaceBreak;
                }
                if (SkUnicode_client::isControl(unichar)) {
                    results->at(i) |= SkUnicode::kControl;
                }
            }
        }
        return true;
    }

    bool computeCodeUnitFlags(char16_t utf16[], int utf16Units, bool replaceTabs,
                          SkTArray<SkUnicode::CodeUnitFlags, true>* results) override {
        results->clear();
        results->push_back_n(utf16Units + 1, CodeUnitFlags::kNoCodeUnitFlag);
        for (auto& lineBreak : fData->fLineBreaks) {
            (*results)[lineBreak.pos] |=
                lineBreak.breakType == LineBreakType::kHardLineBreak
                    ? CodeUnitFlags::kHardLineBreakBefore
                    : CodeUnitFlags::kSoftLineBreakBefore;
        }
        for (auto& grapheme : fData->fGraphemeBreaks) {
            (*results)[grapheme] |= CodeUnitFlags::kGraphemeStart;
        }
        return true;
    }

    bool getWords(const char utf8[], int utf8Units, const char* locale, std::vector<Position>* results) override {
        *results = fData->fWords;
        return true;
    }

    SkString toUpper(const SkString& str) override {
        SkASSERT(false);
        return SkString(fData->fText8.data(), fData->fText8.size());
    }

    void reorderVisual(const BidiLevel runLevels[],
                       int levelsCount,
                       int32_t logicalFromVisual[]) override {
        SkUnicode_IcuBidi::bidi_reorderVisual(runLevels, levelsCount, logicalFromVisual);
    }
private:
    friend class SkBreakIterator_client;

    std::shared_ptr<Data> fData;
};

class SkBreakIterator_client: public SkBreakIterator {
    std::shared_ptr<SkUnicode_client::Data> fData;
    Position fLastResult;
    Position fStart;
    Position fEnd;
public:
    explicit SkBreakIterator_client(std::shared_ptr<SkUnicode_client::Data> data) : fData(data) { }
    Position first() override
      { return fData->fLineBreaks[fStart + (fLastResult = 0)].pos; }
    Position current() override
      { return fData->fLineBreaks[fStart + fLastResult].pos; }
    Position next() override
      { return fData->fLineBreaks[fStart + fLastResult + 1].pos; }
    Status status() override {
        return fData->fLineBreaks[fStart + fLastResult].breakType ==
                       SkUnicode::LineBreakType::kHardLineBreak
                       ? SkUnicode::CodeUnitFlags::kHardLineBreakBefore
                       : SkUnicode::CodeUnitFlags::kSoftLineBreakBefore;
    }
    bool isDone() override { return fStart + fLastResult == fEnd; }
    bool setText(const char utftext8[], int utf8Units) override {
        SkASSERT(utftext8 >= fData->fText8.data() &&
                 utf8Units <= SkToS16(fData->fText8.size()));
        fStart = utftext8 - fData->fText8.data();
        fEnd = fStart + utf8Units;
        fLastResult = 0;
        return true;
    }
    bool setText(const char16_t utftext16[], int utf16Units) override {
        SkASSERT(utftext16 >= fData->fText16.data() &&
                 utf16Units <= SkToS16(fData->fText16.size()));
        fStart = utftext16 - fData->fText16.data();
        fEnd = fStart + utf16Units;
        fLastResult = 0;
        return true;
    }
};
std::unique_ptr<SkBidiIterator> SkUnicode_client::makeBidiIterator(const uint16_t text[], int count,
                                                 SkBidiIterator::Direction dir) {
    return SkUnicode::makeBidiIterator(text, count, dir);
}
std::unique_ptr<SkBidiIterator> SkUnicode_client::makeBidiIterator(const char text[],
                                                 int count,
                                                 SkBidiIterator::Direction dir) {
    return SkUnicode::makeBidiIterator(text, count, dir);
}
std::unique_ptr<SkBreakIterator> SkUnicode_client::makeBreakIterator(const char locale[],
                                                   BreakType breakType) {
    return std::make_unique<SkBreakIterator_client>(fData);
}
std::unique_ptr<SkBreakIterator> SkUnicode_client::makeBreakIterator(BreakType breakType) {
    return std::make_unique<SkBreakIterator_client>(fData);
}

std::unique_ptr<SkUnicode> SkUnicode::MakeClientBasedUnicode(
        SkSpan<char> text,
        std::vector<SkUnicode::Position> words,
        std::vector<SkUnicode::Position> graphemeBreaks,
        std::vector<SkUnicode::LineBreakBefore> lineBreaks) {
    return std::make_unique<SkUnicode_client>(text, words, graphemeBreaks, lineBreaks);
}
