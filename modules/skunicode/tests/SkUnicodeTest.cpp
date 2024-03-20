
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
#include "src/base/SkBitmaskEnum.h"
#include "tests/Test.h"

#include "modules/skunicode/include/SkUnicode.h"

#if defined(SK_UNICODE_ICU_IMPLEMENTATION)
#include "modules/skunicode/include/SkUnicode_icu.h"
#endif
#if defined(SK_UNICODE_LIBGRAPHEME_IMPLEMENTATION)
#include "modules/skunicode/include/SkUnicode_libgrapheme.h"
#endif
#if defined(SK_UNICODE_ICU4X_IMPLEMENTATION)
#include "modules/skunicode/include/SkUnicode_icu4x.h"
#endif
#if defined(SK_UNICODE_CLIENT_IMPLEMENTATION)
#include "modules/skunicode/include/SkUnicode_client.h"
#endif

#include <vector>

#ifdef SK_UNICODE_ICU_IMPLEMENTATION
#define DEF_TEST_ICU(name, reporter) \
    DEF_TEST(name##ICU, reporter) { name(reporter, SkUnicodes::ICU::Make()); }
#else
#define DEF_TEST_ICU(name, reporter)
#endif

#ifdef SK_UNICODE_ICU4X_IMPLEMENTATION
#define DEF_TEST_ICU4X(name, reporter) \
    DEF_TEST(name##ICU4X, reporter) { name(reporter, SkUnicodes::ICU4X::Make()); }
#else
#define DEF_TEST_ICU4X(name, reporter)
#endif

#ifdef SK_UNICODE_LIBGRAPHEME_IMPLEMENTATION
#define DEF_TEST_LIBGRAPHEME(name, reporter) \
    DEF_TEST(name##LIBGRAPHEME, reporter) { name(reporter, SkUnicodes::Libgrapheme::Make()); }
#else
#define DEF_TEST_LIBGRAPHEME(name, reporter)
#endif

#define DEF_TEST_NOIMPL(name, reporter)

#define DEF_TEST_UNICODES(name, reporter) \
    static void name(skiatest::Reporter* reporter, sk_sp<SkUnicode> unicode); \
    DEF_TEST_ICU(name, reporter) \
    DEF_TEST_ICU4X(name, reporter) \
    DEF_TEST_LIBGRAPHEME(name, reporter) \
    DEF_TEST_NOIMPL(name, reporter) \
    void name(skiatest::Reporter* reporter, sk_sp<SkUnicode> unicode)

#define DEF_TEST_ICU_UNICODES(name, reporter) \
    static void name(skiatest::Reporter* reporter, sk_sp<SkUnicode> unicode); \
    DEF_TEST_ICU(name, reporter) \
    DEF_TEST_ICU4X(name, reporter) \
    DEF_TEST_NOIMPL(name, reporter) \
    void name(skiatest::Reporter* reporter, sk_sp<SkUnicode> unicode)

using namespace skia_private;

#ifdef SK_UNICODE_CLIENT_IMPLEMENTATION
UNIX_ONLY_TEST(SkUnicode_Client, reporter) {
    std::u16string text = u"\U000f2008";
    auto utf8 = SkUnicode::convertUtf16ToUtf8(text.data(), text.size());
    auto client = SkUnicodes::Client::Make
                  (SkSpan<char>(&utf8[0], utf8.size()), {}, {}, {});
    skia_private::TArray<SkUnicode::CodeUnitFlags, true> results;
    client->computeCodeUnitFlags(utf8.data(), utf8.size(), false, &results);

    for (auto flag : results) {
        REPORTER_ASSERT(reporter, !SkUnicode::hasPartOfWhiteSpaceBreakFlag(flag));
    }
}
#endif

#if defined(SK_UNICODE_ICU_IMPLEMENTATION)
UNIX_ONLY_TEST(SkUnicode_Compiled_Native, reporter) {
    auto icu = SkUnicodes::ICU::Make();
    if (!icu) {
        REPORTER_ASSERT(reporter, icu);
        return;
    }
    std::u16string text = u"\U000f2008";
    auto utf8 = SkUnicode::convertUtf16ToUtf8(text.data(), text.size());
    skia_private::TArray<SkUnicode::CodeUnitFlags, true> results;
    icu->computeCodeUnitFlags(utf8.data(), utf8.size(), false, &results);
    for (auto flag : results) {
        REPORTER_ASSERT(reporter, !SkUnicode::hasPartOfWhiteSpaceBreakFlag(flag));
    }
}
#endif

#if defined(SK_UNICODE_LIBGRAPHEME_IMPLEMENTATION)
UNIX_ONLY_TEST(SkUnicode_GetUtf8Words, reporter) {
    SkString text("1 22 333 4444 55555 666666 7777777");
    std::vector<SkUnicode::Position> expected = { 0, 1, 2, 4, 5, 8, 9, 13, 14, 19, 20, 26, 27, 34 };
    auto libgrapheme = SkUnicodes::Libgrapheme::Make();
    std::vector<SkUnicode::Position> results;
    auto result = libgrapheme->getUtf8Words(text.data(), text.size(), "en", &results);
    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter, results.size() == expected.size());
    for (auto i = 0ul; i < results.size(); ++i) {
        REPORTER_ASSERT(reporter, results[i] == expected[i]);
    }
}
#endif

#if defined(SK_UNICODE_ICU_IMPLEMENTATION)
UNIX_ONLY_TEST(SkUnicode_Compiled_GetSentences, reporter) {
    auto icu = SkUnicodes::ICU::Make();
    if (!icu) {
        REPORTER_ASSERT(reporter, icu);
        return;
    }
    SkString text("Hello world! Hello world? Hello world... Not a sentence end: 3.1415926");
    std::vector<SkUnicode::Position> expected = {0, 13, 26, 41, 70};
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

// On Windows libgrapheme produces different results
DEF_TEST_ICU_UNICODES(SkUnicode_GetBidiRegionsLTR, reporter) {
    if (!unicode) {
        return;
    }
    SkString text("1 22 333 4444 55555 666666 7777777");
    std::vector<SkUnicode::BidiRegion> results;
    auto result = unicode->getBidiRegions(text.data(),
                                          text.size(),
                                          SkUnicode::TextDirection::kLTR,
                                          &results);
    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter, results.size() == 1);
    REPORTER_ASSERT(reporter, results[0].start == 0 &&
                              results[0].end == text.size() &&
                              results[0].level == 0);
}

DEF_TEST_ICU_UNICODES(SkUnicode_GetBidiRegionsRTL, reporter) {
    if (!unicode) {
        return;
    }
    SkString text("الهيمنة على العالم عبارة قبيحة ، أفضل أن أسميها تحسين العالم.");
    std::vector<SkUnicode::BidiRegion> results;
    auto result = unicode->getBidiRegions(text.data(),
                                          text.size(),
                                          SkUnicode::TextDirection::kRTL,
                                          &results);
    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter, results.size() == 1);
    REPORTER_ASSERT(reporter, results[0].start == 0 &&
                              results[0].end == text.size() &&
                              results[0].level == 1);
}

DEF_TEST_ICU_UNICODES(SkUnicode_GetBidiRegionsMix1, reporter) {
    if (!unicode) {
        return;
    }
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
    std::vector<SkUnicode::BidiRegion> results;
    auto result = unicode->getBidiRegions(text.data(),
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

DEF_TEST_ICU_UNICODES(SkUnicode_GetBidiRegionsMix2, reporter) {
    if (!unicode) {
      return;
    }
    // Few Russian/English words (ЛТР) in the mix
    SkString text("World ЛТР Domination هي عبارة قبيحة ، أفضل أن أسميها World ЛТР Optimization.");
    std::vector<SkUnicode::BidiRegion> expected = {
        { 0, 24, 0},
        { 24, 80, 1},
        { 80, 107, 0},
    };
    std::vector<SkUnicode::BidiRegion> results;
    auto result = unicode->getBidiRegions(text.data(),
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

// Currently, libgrapheme uses different default rules and produces slightly
// different results; it does not matter for text shaping
DEF_TEST_ICU_UNICODES(SkUnicode_ToUpper, reporter) {
    if (!unicode) {
        return;
    }
    SkString lower("abcdefghijklmnopqrstuvwxyz");
    SkString upper("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    auto icu_result1 = unicode->toUpper(lower);
    REPORTER_ASSERT(reporter, icu_result1.equals(upper));
    auto icu_result2 = unicode->toUpper(upper);
    REPORTER_ASSERT(reporter, icu_result2.equals(upper));
}

DEF_TEST_ICU_UNICODES(SkUnicode_ComputeCodeUnitFlags, reporter) {
    if (!unicode) {
        return;
    }
    //SkString text("World domination is such an ugly phrase - I prefer to call it world optimisation");
    SkString text("1\n22 333 4444 55555 666666 7777777");
    // 4 8 13 19 24
    TArray<SkUnicode::CodeUnitFlags> results;
    auto result = unicode->computeCodeUnitFlags(text.data(),
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

DEF_TEST_UNICODES(SkUnicode_ReorderVisual, reporter) {
    if (!unicode) {
        return;
    }
    auto reorder = [&](std::vector<SkUnicode::BidiLevel> levels,
                       std::vector<int32_t> expected) {
            std::vector<int32_t> logicalOrder(levels.size());
            unicode->reorderVisual(levels.data(), levels.size(), logicalOrder.data());
            for (auto i = 0ul; i < levels.size(); ++i) {
                REPORTER_ASSERT(reporter, expected[i] == logicalOrder[i]);
            }
        };
    reorder({}, {});
    reorder({0}, {0});
    reorder({1}, {0});
    reorder({0, 1, 0, 1}, {0, 1, 2, 3});
}

[[maybe_unused]] static void SkUnicode_Emoji(SkUnicode* icu, skiatest::Reporter* reporter) {
    std::u32string emojis(U"😄😁😆😅😂🤣");
    std::u32string not_emojis(U"満毎行昼本可");
    for (auto e : emojis) {
        REPORTER_ASSERT(reporter, icu->isEmoji(e));
    }
    for (auto n: not_emojis) {
        REPORTER_ASSERT(reporter, !icu->isEmoji(n));
    }
}

#ifdef SK_UNICODE_ICU_IMPLEMENTATION
UNIX_ONLY_TEST(SkUnicode_Compiled_Emoji, reporter) {
    auto icu = SkUnicodes::ICU::Make();
    if (!icu) {
        REPORTER_ASSERT(reporter, icu);
        return;
    }
    SkUnicode_Emoji(icu.get(), reporter);
}
#endif

#ifdef SK_UNICODE_ICU4X_IMPLEMENTATION
UNIX_ONLY_TEST(SkUnicode_ICU4X_Emoji, reporter) {
    auto icu = SkUnicodes::ICU4X::Make();
    if (!icu) {
        REPORTER_ASSERT(reporter, icu);
        return;
    }
    SkUnicode_Emoji(icu.get(), reporter);
}
#endif

[[maybe_unused]] static void SkUnicode_Ideographic(SkUnicode* icu, skiatest::Reporter* reporter) {
    std::u32string ideographic(U"満毎行昼本可");
    std::u32string not_ideographic(U"😄😁😆😅😂🤣");
    for (auto i : ideographic) {
        REPORTER_ASSERT(reporter, icu->isIdeographic(i));
    }
    for (auto n: not_ideographic) {
        REPORTER_ASSERT(reporter, !icu->isIdeographic(n));
    }
}

#ifdef SK_UNICODE_ICU_IMPLEMENTATION
UNIX_ONLY_TEST(SkUnicode_Compiled_Ideographic, reporter) {
    auto icu = SkUnicodes::ICU::Make();
    if (!icu) {
        REPORTER_ASSERT(reporter, icu);
        return;
    }
    SkUnicode_Ideographic(icu.get(), reporter);
}
#endif

#ifdef SK_UNICODE_ICU4X_IMPLEMENTATION
UNIX_ONLY_TEST(SkUnicode_ICU4X_Ideographic, reporter) {
    auto icu = SkUnicodes::ICU4X::Make();
    if (!icu) {
        REPORTER_ASSERT(reporter, icu);
        return;
    }
    SkUnicode_Ideographic(icu.get(), reporter);
}
#endif
