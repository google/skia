/*
* Copyright 2025 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/
#include "include/core/SkSpan.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTo.h"
#include "modules/skunicode/include/SkUnicode.h"
#include "modules/skunicode/include/SkUnicode_bidi.h"
#include "modules/skunicode/src/SkBidiFactory_icu_subset.h"
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

class SkUnicode_bidi : public SkUnicodeHardCodedCharProperties {
public:
    SkUnicode_bidi() {}
    ~SkUnicode_bidi() override = default;

    std::unique_ptr<SkBidiIterator> makeBidiIterator(const uint16_t text[], int count,
                                                     SkBidiIterator::Direction dir) override {
        return fBidiFact->MakeIterator(text, count, dir);
    }
    std::unique_ptr<SkBidiIterator> makeBidiIterator(const char text[],
                                                     int count,
                                                     SkBidiIterator::Direction dir) override {
        SkDEBUGF("Method 'makeBidiIterator' is not implemented\n");
        return nullptr;
    }
    std::unique_ptr<SkBreakIterator> makeBreakIterator(const char locale[],
                                                       BreakType breakType) override {
        SkDEBUGF("Method 'makeBreakIterator' is not implemented\n");
        return nullptr;
    }
    std::unique_ptr<SkBreakIterator> makeBreakIterator(BreakType breakType) override {
        SkDEBUGF("Method 'makeBreakIterator' is not implemented\n");
        return nullptr;
    }

    bool getBidiRegions(const char utf8[],
                        int utf8Units,
                        TextDirection dir,
                        std::vector<BidiRegion>* results) override {
        return fBidiFact->ExtractBidi(utf8, utf8Units, dir, results);
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
        SkDEBUGF("Method 'computeCodeUnitFlags' is not implemented\n");
        return false;
    }

    bool computeCodeUnitFlags(char16_t utf16[], int utf16Units, bool replaceTabs,
                          TArray<SkUnicode::CodeUnitFlags, true>* results) override {
        results->clear();
        results->push_back_n(utf16Units + 1, CodeUnitFlags::kNoCodeUnitFlag);
        for (auto i = 0; i < utf16Units; ++i) {
            auto unichar = utf16[i];
            if (this->isSpace(unichar)) {
                results->at(i) |= SkUnicode::kPartOfIntraWordBreak;
            }
            if (this->isWhitespace(unichar)) {
                results->at(i) |= SkUnicode::kPartOfWhiteSpaceBreak;
            }
            if (this->isControl(unichar)) {
                results->at(i) |= SkUnicode::kControl;
            }
            if (this->isIdeographic(unichar)) {
                results->at(i) |= SkUnicode::kIdeographic;
            }
        }
        return true;
    }

    bool getWords(const char utf8[], int utf8Units, const char* locale, std::vector<Position>* results) override {
        SkDEBUGF("Method 'getWords' is not implemented\n");
        return false;
    }

    SkString toUpper(const SkString& str) override {
        SkDEBUGF("Method 'toUpper' is not implemented\n");
        return SkString();
    }

    SkString toUpper(const SkString& str, const char* locale) override {
        SkDEBUGF("Method 'toUpper' is not implemented\n");
        return SkString();
    }

    void reorderVisual(const BidiLevel runLevels[],
                       int levelsCount,
                       int32_t logicalFromVisual[]) override {
        if (levelsCount == 0) {
            // To avoid an assert in unicode
            return;
        }
        SkASSERT(runLevels != nullptr);
        fBidiFact->bidi_reorderVisual(runLevels, levelsCount, logicalFromVisual);
    }

private:
    sk_sp<SkBidiFactory> fBidiFact = sk_make_sp<SkBidiSubsetFactory>();
};

namespace SkUnicodes::Bidi {
    sk_sp<SkUnicode> Make() {
        return sk_make_sp<SkUnicode_bidi>();
    }
}
