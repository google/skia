
/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkSpan.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "modules/skunicode/include/SkUnicode.h"
#include "src/base/SkBitmaskEnum.h"
#include "tests/Test.h"

#include <vector>

using namespace skia_private;

#ifdef SK_UNICODE_CLIENT_IMPLEMENTATION
UNIX_ONLY_TEST(SkUnicode_Client, reporter) {
    std::u16string text = u"\U000f2008";
    auto utf8 = SkUnicode::convertUtf16ToUtf8(text.data(), text.size());
    auto client = SkUnicode::MakeClientBasedUnicode
                  (SkSpan<char>(&utf8[0], utf8.size()), {}, {}, {});
    skia_private::TArray<SkUnicode::CodeUnitFlags, true> results;
    client->computeCodeUnitFlags(utf8.data(), utf8.size(), false, &results);

    for (auto flag : results) {
        REPORTER_ASSERT(reporter, !SkUnicode::hasPartOfWhiteSpaceBreakFlag(flag));
    }
}
#endif
#ifdef SK_UNICODE_ICU_IMPLEMENTATION
UNIX_ONLY_TEST(SkUnicode_Native, reporter) {
    std::u16string text = u"\U000f2008";
    auto utf8 = SkUnicode::convertUtf16ToUtf8(text.data(), text.size());
    auto icu = SkUnicode::Make();
    skia_private::TArray<SkUnicode::CodeUnitFlags, true> results;
    icu->computeCodeUnitFlags(utf8.data(), utf8.size(), false, &results);
    for (auto flag : results) {
        REPORTER_ASSERT(reporter, !SkUnicode::hasPartOfWhiteSpaceBreakFlag(flag));
    }
}
#endif

#ifdef SK_UNICODE_LIBGRAPHEME_IMPLEMENTATION
UNIX_ONLY_TEST(SkUnicode_GetUtf8Words, reporter) {
    SkString text("1 22 333 4444 55555 666666 7777777");
    std::vector<SkUnicode::Position> expected = { 0, 1, 2, 4, 5, 8, 9, 13, 14, 19, 20, 26, 27, 34 };
    auto libgrapheme = SkUnicode::MakeLibgraphemeBasedUnicode();
    std::vector<SkUnicode::Position> results;
    auto result = libgrapheme->getUtf8Words(text.data(), text.size(), "en", &results);
    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter, results.size() == expected.size());
    for (auto i = 0ul; i < results.size(); ++i) {
        REPORTER_ASSERT(reporter, results[i] == expected[i]);
    }
}
#endif

#ifdef SK_UNICODE_ICU_IMPLEMENTATION
UNIX_ONLY_TEST(SkUnicode_GetSentences, reporter) {
    SkString text("Hello world! Hello world? Hello world... Not a sentence end: 3.1415926");
    std::vector<SkUnicode::Position> expected = {0, 13, 26, 41, 70};
    auto icu = SkUnicode::Make();
    std::vector<SkUnicode::Position> results;
    auto result = icu->getSentences(text.data(), text.size(), nullptr, &results);
    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter, results.size() == expected.size());
    for (auto i = 0ul; i < results.size(); ++i) {
        REPORTER_ASSERT(reporter, results[i] == expected[i]);
    }
}
#endif

bool hasWordFlag(SkUnicode::CodeUnitFlags flags) {
    return (flags & SkUnicode::kWordBreak) == SkUnicode::kWordBreak;
}

UNIX_ONLY_TEST(SkUnicode_GetBidiRegionsLTR, reporter) {
    SkString text("1 22 333 4444 55555 666666 7777777");
    auto icu = SkUnicode::Make();
    std::vector<SkUnicode::BidiRegion> results;
    auto result = icu->getBidiRegions(text.data(),
                                      text.size(),
                                      SkUnicode::TextDirection::kLTR,
                                      &results);
    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter, results.size() == 1);
    REPORTER_ASSERT(reporter, results[0].start == 0 &&
                              results[0].end == text.size() &&
                              results[0].level == 0);
}
UNIX_ONLY_TEST(SkUnicode_GetBidiRegionsRTL, reporter) {
    SkString text("الهيمنة على العالم عبارة قبيحة ، أفضل أن أسميها تحسين العالم.");
    auto icu = SkUnicode::Make();
    std::vector<SkUnicode::BidiRegion> results;
    auto result = icu->getBidiRegions(text.data(),
                                      text.size(),
                                      SkUnicode::TextDirection::kRTL,
                                      &results);
    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter, results.size() == 1);
    REPORTER_ASSERT(reporter, results[0].start == 0 &&
                              results[0].end == text.size() &&
                              results[0].level == 1);
}

UNIX_ONLY_TEST(SkUnicode_GetBidiRegionsMix1, reporter) {
    // Spaces become Arabic (RTL) but numbers remain English (LTR)
    SkString text("1 22 333 4444 55555 666666 7777777");
    std::vector<SkUnicode::BidiRegion> expected = {
        {0, 1, 2},
        {1, 2, 1},
        {2, 4, 2},
        {4, 5, 1},
        {5, 8, 2},
        {8, 9, 1},
        {9, 13, 2},
        {13, 14, 1},
        {14, 19, 2},
        {19, 20, 1},
        {20, 26, 2},
        {26, 27, 1},
        {27, 34, 2},
    };
    auto icu = SkUnicode::Make();
    std::vector<SkUnicode::BidiRegion> results;
    auto result = icu->getBidiRegions(text.data(),
                                      text.size(),
                                      SkUnicode::TextDirection::kRTL,
                                      &results);
    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter, results.size() == expected.size());
    for (auto i = 0ul; i < results.size(); ++i) {
      REPORTER_ASSERT(reporter, results[i].start == expected[i].start &&
                                results[i].end == expected[i].end &&
                                results[i].level == expected[i].level);
    }
}

UNIX_ONLY_TEST(SkUnicode_GetBidiRegionsMix2, reporter) {
    // Few Russian/English words (ЛТР) in the mix
    SkString text("World ЛТР Domination هي عبارة قبيحة ، أفضل أن أسميها World ЛТР Optimization.");
    std::vector<SkUnicode::BidiRegion> expected = {
        { 0, 24, 0},
        { 24, 80, 1},
        { 80, 107, 0},
    };
    auto icu = SkUnicode::Make();
    std::vector<SkUnicode::BidiRegion> results;
    auto result = icu->getBidiRegions(text.data(),
                                      text.size(),
                                      SkUnicode::TextDirection::kLTR,
                                      &results);
    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter, results.size() == expected.size());
    for (auto i = 0ul; i < results.size(); ++i) {
        REPORTER_ASSERT(reporter, results[i].start == expected[i].start &&
                                  results[i].end == expected[i].end &&
                                  results[i].level == expected[i].level);
    }
}

#ifndef SK_UNICODE_ICU4X_IMPLEMENTATION
UNIX_ONLY_TEST(SkUnicode_ToUpper, reporter) {
    SkString lower("abcdefghijklmnopqrstuvwxyz");
    SkString upper("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    auto icu = SkUnicode::Make();
    auto icu_result1 = icu->toUpper(lower);
    REPORTER_ASSERT(reporter, icu_result1.equals(upper));
    auto icu_result2 = icu->toUpper(upper);
    REPORTER_ASSERT(reporter, icu_result2.equals(upper));
}
#endif

UNIX_ONLY_TEST(SkUnicode_ComputeCodeUnitFlags, reporter) {
    //SkString text("World domination is such an ugly phrase - I prefer to call it world optimisation");
    SkString text("1\n22 333 4444 55555 666666 7777777");
    // 4 8 13 19 24
    auto icu = SkUnicode::Make();
    TArray<SkUnicode::CodeUnitFlags> results;
    auto result = icu->computeCodeUnitFlags(text.data(),
                                            text.size(),
                                            /*replaceTabs=*/true,
                                            &results);
    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter, results.size() == SkToS16(text.size() + 1));
    for (auto i = 0; i < results.size(); ++i) {
        auto flags = results[i];
        auto expected = SkUnicode::CodeUnitFlags::kGraphemeStart;
        if (i == 1) {
            expected |= SkUnicode::CodeUnitFlags::kControl;
        }
        if (i == 2) {
            expected |= SkUnicode::CodeUnitFlags::kHardLineBreakBefore;
        }
        if (i == 1 || i == 4 || i == 8 || i == 13 || i == 19 || i == 26) {
            expected |= SkUnicode::CodeUnitFlags::kPartOfWhiteSpaceBreak;
            expected |= SkUnicode::CodeUnitFlags::kPartOfIntraWordBreak;
        }
        if (i == 0 || i == 2 || i == 5 || i == 9 || i == 14 || i == 20
                                                 || i == 27 || i == 34) {
            expected |= SkUnicode::CodeUnitFlags::kSoftLineBreakBefore;
        }
        REPORTER_ASSERT(reporter, flags == expected);
    }
}

UNIX_ONLY_TEST(SkUnicode_ReorderVisual, reporter) {
    auto icu = SkUnicode::Make();
    auto reorder = [&](std::vector<SkUnicode::BidiLevel> levels,
                       std::vector<int32_t> expected) {
            std::vector<int32_t> logicalOrder(levels.size());
            icu->reorderVisual(levels.data(), levels.size(), logicalOrder.data());
            for (auto i = 0ul; i < levels.size(); ++i) {
                REPORTER_ASSERT(reporter, expected[i] == logicalOrder[i]);
            }
        };
    reorder({}, {});
    reorder({0}, {0});
    reorder({1}, {0});
    reorder({0, 1, 0, 1}, {0, 1, 2, 3});
}

#ifdef SK_UNICODE_ICU_IMPLEMENTATION
UNIX_ONLY_TEST(SkUnicode_Emoji, reporter) {
    std::u32string emojis(U"😄😁😆😅😂🤣");
    std::u32string not_emojis(U"満毎行昼本可");
    auto icu = SkUnicode::Make();
    for (auto e : emojis) {
        REPORTER_ASSERT(reporter, icu->isEmoji(e));
    }
    for (auto n: not_emojis) {
        REPORTER_ASSERT(reporter, !icu->isEmoji(n));
    }
}
#endif

UNIX_ONLY_TEST(SkUnicode_Ideographic, reporter) {
    std::u32string ideographic(U"満毎行昼本可");
    std::u32string not_ideographic(U"😄😁😆😅😂🤣");
    auto icu = SkUnicode::Make();
    for (auto i : ideographic) {
        REPORTER_ASSERT(reporter, icu->isIdeographic(i));
    }
    for (auto n: not_ideographic) {
        REPORTER_ASSERT(reporter, !icu->isIdeographic(n));
    }
}
