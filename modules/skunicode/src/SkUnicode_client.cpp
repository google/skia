                                           /*
* Copyright 2022 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/
#include "include/core/SkSpan.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTo.h"
#include "modules/skunicode/include/SkUnicode.h"
#include "modules/skunicode/src/SkUnicode_client.h"
#include "modules/skunicode/src/SkUnicode_hardcoded.h"
#include "modules/skunicode/src/SkUnicode_icu_bidi.h"
#include "src/base/SkBitmaskEnum.h"
#include "src/base/SkUTF.h"

#include <algorithm>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <array>
#include <unicode/ubidi.h>
#include <unicode/ubrk.h>
#include <unicode/uchar.h>
#include <unicode/uloc.h>
#include <unicode/uscript.h>
#include <unicode/ustring.h>
#include <unicode/utext.h>
#include <unicode/utypes.h>

using namespace skia_private;

class SkUnicode_client : public SkUnicodeHardCodedCharProperties {
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
                                           std::move(lineBreaks))) { }
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
        return SkUnicode_IcuBidi::ExtractBidi(utf8, utf8Units, dir, results);
    }

    bool getUtf8Words(const char utf8[],
                      int utf8Units,
                      const char* locale,
                      std::vector<Position>* results) override {
        SkDEBUGF("Method 'getUtf8Words' is not implemented\n");
        return false;
    }

    bool getSentences(const char utf8[],
                      int utf8Units,
                      const char* locale,
                      std::vector<SkUnicode::Position>* results) override {
        SkDEBUGF("Method 'getSentences' is not implemented\n");
        return false;
    }

    bool computeCodeUnitFlags(char utf8[],
                              int utf8Units,
                              bool replaceTabs,
                              TArray<SkUnicode::CodeUnitFlags, true>* results) override {
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
            if (replaceTabs && this->isTabulation(unichar)) {
                results->at(before) |= SkUnicode::kTabulation;
                if (replaceTabs) {
                    unichar = ' ';
                    utf8[before] = ' ';
                }
            }
            for (auto i = before; i < after; ++i) {
                if (this->isSpace(unichar)) {
                    results->at(i) |= SkUnicode::kPartOfIntraWordBreak;
                }
                if (this->isWhitespace(unichar)) {
                    results->at(i) |= SkUnicode::kPartOfWhiteSpaceBreak;
                }
                if (this->isControl(unichar)) {
                    results->at(i) |= SkUnicode::kControl;
                }
            }
        }
        return true;
    }

    bool computeCodeUnitFlags(char16_t utf16[], int utf16Units, bool replaceTabs,
                          TArray<SkUnicode::CodeUnitFlags, true>* results) override {
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
        return this->toUpper(str, nullptr);
    }

    SkString toUpper(const SkString& str, const char* locale) override {
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
    return SkUnicode_IcuBidi::MakeIterator(text, count, dir);
}
std::unique_ptr<SkBidiIterator> SkUnicode_client::makeBidiIterator(const char text[],
                                                 int count,
                                                 SkBidiIterator::Direction dir) {
    return SkUnicode_IcuBidi::MakeIterator(text, count, dir);
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
    return std::make_unique<SkUnicode_client>(text, std::move(words), std::move(graphemeBreaks),
                                              std::move(lineBreaks));
}

