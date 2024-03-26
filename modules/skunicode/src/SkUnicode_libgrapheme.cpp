/*
* Copyright 2022 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "modules/skunicode/include/SkUnicode_libgrapheme.h"

#include "include/core/SkSpan.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkTArray.h"
#include "modules/skunicode/include/SkUnicode.h"
#include "modules/skunicode/src/SkBidiFactory_icu_subset.h"
#include "modules/skunicode/src/SkUnicode_hardcoded.h"
#include "modules/skunicode/src/SkUnicode_icu_bidi.h"
#include "src/base/SkBitmaskEnum.h"

extern "C" {
#include <grapheme.h>
}
#include <array>
#include <memory>
#include <vector>
#include <unordered_map>

using namespace skia_private;

class SkUnicode_libgrapheme : public SkUnicodeHardCodedCharProperties {
public:
    SkUnicode_libgrapheme() { }

    ~SkUnicode_libgrapheme() override = default;

    // For SkShaper
    std::unique_ptr<SkBidiIterator> makeBidiIterator(const uint16_t text[], int count,
                                                     SkBidiIterator::Direction dir) override;
    std::unique_ptr<SkBidiIterator> makeBidiIterator(const char text[],
                                                     int count,
                                                     SkBidiIterator::Direction dir) override;
    std::unique_ptr<SkBreakIterator> makeBreakIterator(const char locale[],
                                                       BreakType breakType) override;
    std::unique_ptr<SkBreakIterator> makeBreakIterator(BreakType breakType) override;
    bool getBidiRegions(const char utf8[],
                        int utf8Units,
                        TextDirection dir,
                        std::vector<BidiRegion>* results) override {
        return fBidiFact->ExtractBidi(utf8, utf8Units, dir, results);
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
                              skia_private::TArray<SkUnicode::CodeUnitFlags, true>* results) override {
        results->clear();
        results->push_back_n(utf8Units + 1, CodeUnitFlags::kNoCodeUnitFlag);

        size_t lineBreak = 0;
        (*results)[lineBreak] |= CodeUnitFlags::kSoftLineBreakBefore;
        while (lineBreak < utf8Units) {
            lineBreak += grapheme_next_line_break_utf8(utf8 + lineBreak, utf8Units - lineBreak);
            // Check if the previous code unit is a hard break.
            auto codePoint = utf8[lineBreak - 1];
            (*results)[lineBreak] |= this->isHardBreak(codePoint)
                                    ? CodeUnitFlags::kHardLineBreakBefore
                                    : CodeUnitFlags::kSoftLineBreakBefore;
        }
        (*results)[utf8Units] |= CodeUnitFlags::kSoftLineBreakBefore;

        size_t graphemeBreak = 0;
        (*results)[graphemeBreak] |= CodeUnitFlags::kGraphemeStart;
        while (graphemeBreak < utf8Units) {
            graphemeBreak += grapheme_next_character_break_utf8(utf8 + graphemeBreak, utf8Units - graphemeBreak);
            (*results)[graphemeBreak] |= CodeUnitFlags::kGraphemeStart;
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
                          skia_private::TArray<SkUnicode::CodeUnitFlags, true>* results) override {
        SkASSERT(false);
        return false;
    }

    bool getUtf8To16Mapping(const char utf8[], int utf8Units, std::unordered_map<Position, Position>* results) {
        int utf16Units = 0;
        const char* ptr8 = utf8;
        const char* end8 = utf8 + utf8Units;
        while (ptr8 < end8) {
            results->emplace(ptr8 - utf8, utf16Units);
            SkUnichar uni = SkUTF::NextUTF8(&ptr8, end8);
            if (uni < 0) {
                return false;
            }

            uint16_t utf16[2];
            size_t count = SkUTF::ToUTF16(uni, utf16);
            if (count == 0) {
                return false;
            }
            utf16Units += count;
        }
        results->emplace(utf8Units, utf16Units);
        return true;
    }

    bool getWords(const char utf8[], int utf8Units, const char* locale, std::vector<Position>* results) override {
        std::unordered_map<Position, Position> mapping;
        if (!getUtf8To16Mapping(utf8, utf8Units, &mapping)) {
            return false;
        }
        size_t wordBreak = 0;
        while (wordBreak < utf8Units) {
            wordBreak += grapheme_next_word_break_utf8(utf8 + wordBreak, utf8Units - wordBreak);
            if (mapping.find(wordBreak) == mapping.end()) {
                return false;
            }
            results->emplace_back(mapping[wordBreak]);
        }
        return true;
    }

    bool getUtf8Words(const char utf8[],
                      int utf8Units,
                      const char* locale,
                      std::vector<Position>* results) override {
        // Let's consider sort line breaks, whitespaces and CJK codepoints instead
        std::vector<CodeUnitFlags> breaks(utf8Units + 1, CodeUnitFlags::kNoCodeUnitFlag);

        size_t lineBreak = 0;
        breaks[lineBreak] = CodeUnitFlags::kSoftLineBreakBefore;
        while (lineBreak < utf8Units) {
            lineBreak += grapheme_next_line_break_utf8(utf8 + lineBreak, utf8Units - lineBreak);
            breaks[lineBreak] = CodeUnitFlags::kSoftLineBreakBefore;
        }
        breaks[lineBreak] = CodeUnitFlags::kSoftLineBreakBefore;

        const char* current = utf8;
        const char* end = utf8 + utf8Units;
        while (current < end) {
            auto index = current - utf8;
            SkUnichar unichar = SkUTF::NextUTF8(&current, end);
            if (this->isWhitespace(unichar)) {
                breaks[index] = CodeUnitFlags::kPartOfWhiteSpaceBreak;
            } else if (this->isIdeographic(unichar)) {
                breaks[index] = CodeUnitFlags::kIdeographic;
            }
        }

        bool whitespaces = false;
        for (size_t i = 0; i < breaks.size(); ++i) {
            auto b = breaks[i];
            if (b == CodeUnitFlags::kSoftLineBreakBefore) {
                results->emplace_back(i);
                whitespaces = false;
            } else if (b == CodeUnitFlags::kIdeographic) {
                results->emplace_back(i);
                whitespaces = false;
            } else if (b == CodeUnitFlags::kPartOfWhiteSpaceBreak) {
                if (!whitespaces) {
                    results->emplace_back(i);
                }
                whitespaces = true;
            } else {
                whitespaces = false;
            }
        }

        return true;

        /*
        size_t wordBreak = 0;
        while (wordBreak < utf8Units) {
            wordBreak += grapheme_next_word_break_utf8(utf8 + wordBreak, utf8Units - wordBreak);
            results->emplace_back(wordBreak);
        }
        return true;
        */
    }

    SkString toUpper(const SkString& str) override {
        return this->toUpper(str, nullptr);
    }

    SkString toUpper(const SkString& str, const char* locale) override {
        SkString res(" ", str.size());
        grapheme_to_uppercase_utf8(str.data(), str.size(), res.data(), res.size());
        return res;
    }

    void reorderVisual(const BidiLevel runLevels[],
                       int levelsCount,
                       int32_t logicalFromVisual[]) override {
        fBidiFact->bidi_reorderVisual(runLevels, levelsCount, logicalFromVisual);
    }
private:
    friend class SkBreakIterator_libgrapheme;

    sk_sp<SkBidiFactory> fBidiFact = sk_make_sp<SkBidiSubsetFactory>();
};

class SkBreakIterator_libgrapheme: public SkBreakIterator {
    SkUnicode_libgrapheme* fUnicode;
    std::vector<SkUnicode::LineBreakBefore> fLineBreaks;
    Position fLineBreakIndex;
    static constexpr const int kDone = -1;
public:
    explicit SkBreakIterator_libgrapheme(SkUnicode_libgrapheme* unicode) : fUnicode(unicode) { }
    Position first() override
      { return fLineBreaks[(fLineBreakIndex = 0)].pos; }
    Position current() override
      { return fLineBreaks[fLineBreakIndex].pos; }
    Position next() override
      { return fLineBreaks[++fLineBreakIndex].pos; }
    Status status() override {
        return fLineBreaks[fLineBreakIndex].breakType ==
                       SkUnicode::LineBreakType::kHardLineBreak
                       ? SkUnicode::CodeUnitFlags::kHardLineBreakBefore
                       : SkUnicode::CodeUnitFlags::kSoftLineBreakBefore;
    }
    bool isDone() override { return fLineBreaks[fLineBreakIndex].pos == kDone; }
    bool setText(const char utftext8[], int utf8Units) override {
        fLineBreaks.clear();
        size_t lineBreak = 0;
        // first() must always go to the beginning of the string.
        fLineBreaks.emplace_back(0, SkUnicode::LineBreakType::kHardLineBreak);
        for (size_t pos = 0; pos < utf8Units;) {
            pos += grapheme_next_line_break_utf8(utftext8 + pos, utf8Units - pos);
            auto codePoint = utftext8[pos];
            fLineBreaks.emplace_back(pos,
                                     fUnicode->isHardBreak(codePoint)
                                    ? SkUnicode::LineBreakType::kHardLineBreak
                                    : SkUnicode::LineBreakType::kSoftLineBreak);
        }
        // There is always an "end" which signals "done".
        fLineBreaks.emplace_back(kDone, SkUnicode::LineBreakType::kHardLineBreak);
        fLineBreakIndex = 0;
        return true;
    }
    bool setText(const char16_t utftext16[], int utf16Units) override {
        SkASSERT(false);
        return false;
    }
};

std::unique_ptr<SkBidiIterator> SkUnicode_libgrapheme::makeBidiIterator(const uint16_t text[], int count,
                                                 SkBidiIterator::Direction dir) {
    return fBidiFact->MakeIterator(text, count, dir);
}
std::unique_ptr<SkBidiIterator> SkUnicode_libgrapheme::makeBidiIterator(const char text[],
                                                 int count,
                                                 SkBidiIterator::Direction dir) {
    return fBidiFact->MakeIterator(text, count, dir);
}
std::unique_ptr<SkBreakIterator> SkUnicode_libgrapheme::makeBreakIterator(const char locale[],
                                                   BreakType breakType) {
    return std::make_unique<SkBreakIterator_libgrapheme>(this);
}
std::unique_ptr<SkBreakIterator> SkUnicode_libgrapheme::makeBreakIterator(BreakType breakType) {
    return std::make_unique<SkBreakIterator_libgrapheme>(this);
}

namespace SkUnicodes::Libgrapheme {
sk_sp<SkUnicode> Make() {
    return sk_make_sp<SkUnicode_libgrapheme>();
}
}
