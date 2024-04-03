/*
* Copyright 2023 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/
#include "modules/skunicode/include/SkUnicode_icu4x.h"

#include "include/core/SkSpan.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTo.h"
#include "modules/skunicode/include/SkUnicode.h"
#include "modules/skunicode/src/SkUnicode_hardcoded.h"
#include "src/base/SkBitmaskEnum.h"
#include "src/base/SkUTF.h"

#include <ICU4XBidi.hpp>
#include <ICU4XCaseMapper.hpp>
#include <ICU4XCodePointMapData8.hpp>
#include <ICU4XCodePointSetData.hpp>
#include <ICU4XDataProvider.hpp>
#include <ICU4XGraphemeClusterSegmenter.hpp>
#include <ICU4XLineSegmenter.hpp>
#include <ICU4XWordSegmenter.hpp>

#include <algorithm>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

class SkUnicode_icu4x :  public SkUnicode {
public:
    SkUnicode_icu4x() {
        fLocale = ICU4XLocale::create_from_string("tr").ok().value();
        fDataProvider = ICU4XDataProvider::create_compiled();
        fCaseMapper = ICU4XCaseMapper::create(fDataProvider).ok().value();
        const auto general = ICU4XCodePointMapData8::load_general_category(fDataProvider).ok().value();
        fControls = general.get_set_for_value(/*Control*/15);
        fWhitespaces = general.get_set_for_value(/*SpaceSeparator*/12);
        fSpaces = general.get_set_for_value(/*SpaceSeparator*/12);
        // TODO: u_isSpace
        fBlanks = ICU4XCodePointSetData::load_blank(fDataProvider).ok().value();
        fEmoji = ICU4XCodePointSetData::load_emoji(fDataProvider).ok().value();
        fEmojiComponent = ICU4XCodePointSetData::load_emoji_component(fDataProvider).ok().value();
        fEmojiModifier = ICU4XCodePointSetData::load_emoji_modifier(fDataProvider).ok().value();
        fEmojiModifierBase = ICU4XCodePointSetData::load_emoji_modifier_base(fDataProvider).ok().value();
        fEmoji = ICU4XCodePointSetData::load_emoji(fDataProvider).ok().value();
        fRegionalIndicator = ICU4XCodePointSetData::load_regional_indicator(fDataProvider).ok().value();
        fIdeographic = ICU4XCodePointSetData::load_ideographic(fDataProvider).ok().value();
        fLineBreaks = ICU4XCodePointMapData8::load_line_break(fDataProvider).ok().value();
    }

    ~SkUnicode_icu4x() override = default;

    void reset();

    // SkUnicode properties
    bool isControl(SkUnichar utf8) override { return fControls.contains(utf8); }
    bool isWhitespace(SkUnichar utf8) override { return fWhitespaces.contains(utf8); }
    bool isSpace(SkUnichar utf8) override { return fBlanks.contains(utf8); }
    bool isHardBreak(SkUnichar utf8) override {
        auto value = fLineBreaks.get(utf8);
        return (value == /*MandatoryBreak*/6) ||
               (value == /*CarriageReturn*/10) ||
               (value == /*LineFeed*/17) ||
               (value == /*NextLine*/29);
    }
    bool isEmoji(SkUnichar utf8) override { return fEmoji.contains(utf8); }
    bool isEmojiComponent(SkUnichar utf8) override { return fEmojiComponent.contains(utf8); }
    bool isEmojiModifierBase(SkUnichar utf8) override { return fEmojiModifierBase.contains(utf8); }
    bool isEmojiModifier(SkUnichar utf8) override { return fEmojiModifier.contains(utf8); }
    bool isRegionalIndicator(SkUnichar utf8) override { return fRegionalIndicator.contains(utf8); }
    bool isIdeographic(SkUnichar utf8) override { return fIdeographic.contains(utf8); }

    // TODO: is there a check for tabulation
    bool isTabulation(SkUnichar utf8) override {
        return utf8 == '\t';
    }

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

        const auto bidi = ICU4XBidi::create(fDataProvider).ok().value();
        std::string_view string_view(utf8, utf8Units);
        auto info = bidi.for_text(string_view, dir == TextDirection::kLTR ? 0 : 1);
        auto currentLevel = info.level_at(0);
        size_t start = 0;

        for (size_t i = 1; i < info.size(); i++) {
            const auto level =  info.level_at(i);
            if (level != currentLevel) {
                (*results).emplace_back(start, i, currentLevel);
                currentLevel = level;
                start = i;
            }
        }
        (*results).emplace_back(start, info.size(), currentLevel);
        return true;
    }

    bool getBidiRegions(const uint16_t utf16[],
                        int utf16Units,
                        TextDirection dir,
                        std::vector<BidiRegion>* results) {
        auto utf8 = SkUnicode::convertUtf16ToUtf8((char16_t*)utf16, utf16Units);
        return this->getBidiRegions(utf8.data(), utf8.size(), dir, results);
    }

    bool computeCodeUnitFlags(char utf8[],
                              int utf8Units,
                              bool replaceTabs,
                              skia_private::TArray<SkUnicode::CodeUnitFlags, true>* results) override {
        results->clear();
        results->push_back_n(utf8Units + 1, CodeUnitFlags::kNoCodeUnitFlag);
        this->markLineBreaks(utf8, utf8Units, /*hardLineBreaks=*/false, results);
        this->markHardLineBreaksHack(utf8, utf8Units, results);
        this->markGraphemes(utf8, utf8Units, results);
        this->markCharacters(utf8, utf8Units, replaceTabs, results);
        return true;
    }

    bool computeCodeUnitFlags(char16_t utf16[], int utf16Units, bool replaceTabs,
                          skia_private::TArray<SkUnicode::CodeUnitFlags, true>* results) override {
        SkASSERT(false);
        return true;
    }

    bool getWords(const char utf8[],
                  int utf8Units,
                  const char* locale,
                  std::vector<Position>* results) override {
        auto utf16 = SkUnicode::convertUtf8ToUtf16(utf8, utf8Units);
        const diplomat::span<const uint16_t> span((uint16_t*)utf16.data(), utf16.size());
        const auto segmenter = ICU4XWordSegmenter::create_dictionary(fDataProvider).ok().value();
        auto iterator = segmenter.segment_utf16(span);
        while (true) {
            int32_t breakpoint = iterator.next();
            if (breakpoint == -1) {
                break;
            }
            results->emplace_back(breakpoint);
        }
        return true;
    }

    SkString toUpper(const SkString& str) override {
        return toUpper(str, "und");
    }

    SkString toUpper(const SkString& str, const char* localeStr) override {
        auto locale = ICU4XLocale::create_from_string(localeStr).ok().value();
        std::string std_string(str.data(), str.size());
        // TODO: upper case
        auto result = fCaseMapper.uppercase(std_string, locale).ok().value();
        return SkString(result.data(), result.size());
    }

    void reorderVisual(const BidiLevel runLevels[],
                       int levelsCount,
                       int32_t logicalFromVisual[]) override {

        const auto bidi = ICU4XBidi::create(fDataProvider).ok().value();
        const diplomat::span<const uint8_t> levels(&runLevels[0], levelsCount);
        auto map = bidi.reorder_visual(levels);
        SkASSERT(levelsCount == map.len());
        std::vector<int32_t> results;
        for (size_t i = 0; i < map.len(); i++) {
            auto level = map.get(i);
            logicalFromVisual[i] = SkToS32(level);
        }
    }

private:
    friend class SkBreakIterator_icu4x;
    friend class SkBidiIterator_icu4x;

    bool markHardLineBreaksHack(char utf8[],
                                int utf8Units,
                                skia_private::TArray<SkUnicode::CodeUnitFlags, true>* results) {
        const char* end = utf8 + utf8Units;
        const char* ch = utf8;
        while (ch < end) {
            auto unichar = SkUTF::NextUTF8(&ch, end);
            if (this->isHardBreak(unichar)) {
                (*results)[ch - utf8] |= CodeUnitFlags::kHardLineBreakBefore;
            }
        }
        return true;
    }

    SkUnichar getChar32(const char* pointer, const char* end) {
        if (pointer < end) {
            return SkUTF::NextUTF8(&pointer, end);
        }
        return -1;
    }

    bool markLineBreaks(char utf8[],
                        int utf8Units,
                        bool hardLineBreaks,
                        skia_private::TArray<SkUnicode::CodeUnitFlags, true>* results) {
        if (utf8Units == 0) {
            return true;
        }
        // TODO: Remove hard line break hack and detect it here
        SkASSERT(!hardLineBreaks);
        const auto lineBreakingOptions = hardLineBreaks
                                              ? ICU4XLineBreakOptionsV1{ICU4XLineBreakStrictness::Strict, ICU4XLineBreakWordOption::Normal}
                                              : ICU4XLineBreakOptionsV1{ICU4XLineBreakStrictness::Loose, ICU4XLineBreakWordOption::Normal};
        const auto segmenter = ICU4XLineSegmenter::create_auto_with_options_v1(fDataProvider, lineBreakingOptions).ok().value();
        std::string_view string_view(utf8, utf8Units);
        auto iterator = segmenter.segment_utf8(string_view);

        while (true) {
            int32_t lineBreak = iterator.next();
            if (lineBreak == -1) {
                break;
            }
            if (hardLineBreaks) {
                (*results)[lineBreak] |= CodeUnitFlags::kHardLineBreakBefore;
            } else {
                (*results)[lineBreak] |= CodeUnitFlags::kSoftLineBreakBefore;
            }
        }
        if (!hardLineBreaks) {
            (*results)[0] |= CodeUnitFlags::kSoftLineBreakBefore;
            (*results)[utf8Units] |= CodeUnitFlags::kSoftLineBreakBefore;
        }
        return true;
    }

    bool markGraphemes(const char utf8[],
                       int utf8Units,
                       skia_private::TArray<SkUnicode::CodeUnitFlags, true>* results) {
        const auto segmenter = ICU4XGraphemeClusterSegmenter::create(fDataProvider).ok().value();
        std::string_view string_view(utf8, utf8Units);
        auto iterator = segmenter.segment_utf8(string_view);
        while (true) {
            int32_t graphemeStart = iterator.next();
            if (graphemeStart == -1) {
                break;
            }
            (*results)[graphemeStart] |= CodeUnitFlags::kGraphemeStart;
        }
        return true;
    }

    bool markCharacters(char utf8[],
                        int utf8Units,
                        bool replaceTabs,
                        skia_private::TArray<SkUnicode::CodeUnitFlags, true>* results) {
        const char* current = utf8;
        const char* end = utf8 + utf8Units;
        while (current < end) {
            auto before = current - utf8;
            SkUnichar unichar = SkUTF::NextUTF8(&current, end);
            if (unichar < 0) unichar = 0xFFFD;
            auto after = current - utf8;
            if (replaceTabs && SkUnicode_icu4x::isTabulation(unichar)) {
                results->at(before) |= SkUnicode::kTabulation;
                if (replaceTabs) {
                    unichar = ' ';
                    utf8[before] = ' ';
                }
            }
            for (auto i = before; i < after; ++i) {
                bool isHardBreak = this->isHardBreak(unichar);
                bool isSpace = this->isSpace(unichar) || isHardBreak;
                bool isWhitespace = this->isWhitespace(unichar) || isHardBreak;
                if (isSpace) {
                    results->at(i) |= SkUnicode::kPartOfIntraWordBreak;
                }
                if (isWhitespace) {
                    results->at(i) |= SkUnicode::kPartOfWhiteSpaceBreak;
                }
                if (this->isControl(unichar)) {
                    results->at(i) |= SkUnicode::kControl;
                }
            }
        }
        return true;
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

    std::shared_ptr<std::vector<SkUnicode::BidiRegion>> fRegions;
    ICU4XLocale fLocale;
    ICU4XDataProvider fDataProvider;
    ICU4XCaseMapper fCaseMapper;
    ICU4XCodePointSetData fWhitespaces;
    ICU4XCodePointSetData fSpaces;
    ICU4XCodePointSetData fBlanks;
    ICU4XCodePointSetData fEmoji;
    ICU4XCodePointSetData fEmojiComponent;
    ICU4XCodePointSetData fEmojiModifier;
    ICU4XCodePointSetData fEmojiModifierBase;
    ICU4XCodePointSetData fRegionalIndicator;
    ICU4XCodePointSetData fIdeographic;
    ICU4XCodePointSetData fControls;
    ICU4XCodePointMapData8 fLineBreaks;
};

class SkBreakIterator_icu4x: public SkBreakIterator {
    Position fLastResult;
    Position fStart;
    Position fEnd;
public:
    SkBreakIterator_icu4x() { }
    Position first() override { SkASSERT(false); return -1; }
    Position current() override { SkASSERT(false); return -1; }
    Position next() override { SkASSERT(false); return -1; }
    Status status() override { SkASSERT(false); return -1; }
    bool isDone() override { SkASSERT(false); return false; }
    bool setText(const char utftext8[], int utf8Units) override { SkASSERT(false); return false; }
    bool setText(const char16_t utftext16[], int utf16Units) override { SkASSERT(false); return false; }
};

class SkBidiIterator_icu4x : public SkBidiIterator {
    std::shared_ptr<std::vector<SkUnicode::BidiRegion>> fRegions;
public:
    explicit SkBidiIterator_icu4x(std::shared_ptr<std::vector<SkUnicode::BidiRegion>> regions)
            : fRegions(regions) { }
    Position getLength() override { return fRegions->size(); }
    Level getLevelAt(Position pos) override {
        auto found = std::lower_bound(
                fRegions->begin(),
                fRegions->end(),
                SkUnicode::BidiRegion(pos, pos, 0),
                [](const SkUnicode::BidiRegion& a, const SkUnicode::BidiRegion& b) {
                    return a.start <= b.start && a.end <= b.end;
                });
        return found->level;
    }
};

std::unique_ptr<SkBidiIterator> SkUnicode_icu4x::makeBidiIterator(const uint16_t text[], int count,
                                                 SkBidiIterator::Direction dir) {
    if (fRegions) {
        fRegions->clear();
    } else {
        fRegions = std::make_shared<std::vector<SkUnicode::BidiRegion>>();
    }

    if (this->getBidiRegions(text, count, dir == SkBidiIterator::Direction::kLTR ? TextDirection::kLTR : TextDirection::kRTL, fRegions.get())) {
        return std::make_unique<SkBidiIterator_icu4x>(fRegions);
    } else {
        return nullptr;
    }
}

std::unique_ptr<SkBidiIterator> SkUnicode_icu4x::makeBidiIterator(const char text[],
                                                 int count,
                                                 SkBidiIterator::Direction dir) {
    if (fRegions) {
        fRegions->clear();
    } else {
        fRegions = std::make_shared<std::vector<SkUnicode::BidiRegion>>();
    }
    if (this->getBidiRegions(text, count, dir == SkBidiIterator::Direction::kLTR ? TextDirection::kLTR : TextDirection::kRTL, fRegions.get())) {
        return std::make_unique<SkBidiIterator_icu4x>(fRegions);
    } else {
        return nullptr;
    }
}

std::unique_ptr<SkBreakIterator> SkUnicode_icu4x::makeBreakIterator(const char locale[],
                                                   BreakType breakType) {
    SkASSERT(false); return nullptr;
}

std::unique_ptr<SkBreakIterator> SkUnicode_icu4x::makeBreakIterator(BreakType breakType) {
    SkASSERT(false); return nullptr;
}

namespace SkUnicodes::ICU4X {
sk_sp<SkUnicode> Make() {
    return sk_make_sp<SkUnicode_icu4x>();
}
}
