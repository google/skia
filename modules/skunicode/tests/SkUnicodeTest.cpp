
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
#include "src/core/SkBitmaskEnum.h"
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
#if defined(SK_UNICODE_APPLE_IMPLEMENTATION)
#include "modules/skunicode/include/SkUnicode_apple.h"
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

#ifdef SK_UNICODE_APPLE_IMPLEMENTATION
#define DEF_TEST_APPLE(name, reporter) \
    DEF_TEST(name##Apple, reporter) { name(reporter, SkUnicodes::Apple::Make()); }
#else
#define DEF_TEST_APPLE(name, reporter)
#endif

#define DEF_TEST_NOIMPL(name, reporter)

#define DEF_TEST_UNICODES(name, reporter) \
    static void name(skiatest::Reporter* reporter, sk_sp<SkUnicode> unicode); \
    DEF_TEST_ICU(name, reporter) \
    DEF_TEST_ICU4X(name, reporter) \
    DEF_TEST_LIBGRAPHEME(name, reporter) \
    DEF_TEST_APPLE(name, reporter) \
    DEF_TEST_NOIMPL(name, reporter) \
    void name(skiatest::Reporter* reporter, sk_sp<SkUnicode> unicode)

#define DEF_TEST_ICU_UNICODES(name, reporter) \
    static void name(skiatest::Reporter* reporter, sk_sp<SkUnicode> unicode); \
    DEF_TEST_ICU(name, reporter) \
    DEF_TEST_ICU4X(name, reporter) \
    DEF_TEST_APPLE(name, reporter) \
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

#if defined(SK_UNICODE_APPLE_IMPLEMENTATION)
DEF_TEST(SkUnicode_Apple_Native, reporter) {
    auto apple = SkUnicodes::Apple::Make();
    if (!apple) {
        REPORTER_ASSERT(reporter, apple);
        return;
    }
    std::u16string text = u"\U000f2008";
    auto utf8 = SkUnicode::convertUtf16ToUtf8(text.data(), text.size());
    skia_private::TArray<SkUnicode::CodeUnitFlags, true> results;
    apple->computeCodeUnitFlags(utf8.data(), utf8.size(), false, &results);
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

#if defined(SK_UNICODE_APPLE_IMPLEMENTATION)
DEF_TEST(SkUnicode_Apple_GetUtf8Words, reporter) {
    SkString text("1 22 333 4444 55555 666666 7777777");
    std::vector<SkUnicode::Position> expected = { 0, 1, 2, 4, 5, 8, 9, 13, 14, 19, 20, 26, 27, 34 };
    auto apple = SkUnicodes::Apple::Make();
    if (!apple) {
        REPORTER_ASSERT(reporter, apple);
        return;
    }
    std::vector<SkUnicode::Position> results;
    auto result = apple->getUtf8Words(text.data(), text.size(), "en", &results);
    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter, results.size() == expected.size());
    for (auto i = 0ul; i < results.size(); ++i) {
        REPORTER_ASSERT(reporter, results[i] == expected[i]);
    }

    SkString cyrillic("Привет мир");
    std::vector<SkUnicode::Position> cyrillicExpected = {0, 12, 13, 19};
    results.clear();
    result = apple->getUtf8Words(cyrillic.data(), cyrillic.size(), "ru", &results);
    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter, results.size() == cyrillicExpected.size());
    for (auto i = 0ul; i < results.size(); ++i) {
        REPORTER_ASSERT(reporter, results[i] == cyrillicExpected[i]);
    }

    SkString mixed("Hello مرحبا World بالعالم");
    std::vector<SkUnicode::Position> mixedExpected = {0, 5, 6, 16, 17, 22, 23, 37};
    results.clear();
    result = apple->getUtf8Words(mixed.data(), mixed.size(), nullptr, &results);
    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter, results.size() == mixedExpected.size());
    for (auto i = 0ul; i < results.size(); ++i) {
        REPORTER_ASSERT(reporter, results[i] == mixedExpected[i]);
    }
}

DEF_TEST(SkUnicode_Apple_GetSentences, reporter) {
    auto apple = SkUnicodes::Apple::Make();
    if (!apple) {
        REPORTER_ASSERT(reporter, apple);
        return;
    }
    SkString text("Hello world! Hello world? Hello world... Not a sentence end: 3.1415926");
    std::vector<SkUnicode::Position> expected = {0, 13, 26, 41, 70};
    std::vector<SkUnicode::Position> results;
    auto result = apple->getSentences(text.data(), text.size(), nullptr, &results);
    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter, results.size() == expected.size());
    for (auto i = 0ul; i < results.size(); ++i) {
        REPORTER_ASSERT(reporter, results[i] == expected[i]);
    }

    SkString cyrillic("Привет мир! Как дела? Все хорошо.");
    std::vector<SkUnicode::Position> cyrillicExpected = {0, 21, 38, 58};
    results.clear();
    result = apple->getSentences(cyrillic.data(), cyrillic.size(), "ru", &results);
    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter, results.size() == cyrillicExpected.size());
    for (auto i = 0ul; i < results.size(); ++i) {
        REPORTER_ASSERT(reporter, results[i] == cyrillicExpected[i]);
    }

    SkString mixed("Hello world! مرحبا بالعالم؟ How are you?");
    std::vector<SkUnicode::Position> mixedExpected = {0, 13, 41, 53};
    results.clear();
    result = apple->getSentences(mixed.data(), mixed.size(), nullptr, &results);
    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter, results.size() == mixedExpected.size());
    for (auto i = 0ul; i < results.size(); ++i) {
        REPORTER_ASSERT(reporter, results[i] == mixedExpected[i]);
    }
}

DEF_TEST(SkUnicode_Apple_BreakIterator_Utf8_Hostile, reporter) {
    auto apple = SkUnicodes::Apple::Make();
    REPORTER_ASSERT(reporter, apple);
    if (!apple) {
        return;
    }

    auto collectBreaks = [](SkBreakIterator* iter) {
        std::vector<SkBreakIterator::Position> breaks;
        for (auto pos = iter->first(); !iter->isDone(); pos = iter->next()) {
            breaks.push_back(pos);
        }
        return breaks;
    };

    {
        SkString text("Привет мир");
        auto wordIter = apple->makeBreakIterator("ru", SkUnicode::BreakType::kWords);
        REPORTER_ASSERT(reporter, wordIter);
        REPORTER_ASSERT(reporter, wordIter->setText(text.data(), text.size()));
        std::vector<SkBreakIterator::Position> expectedWords = {0, 12, 13, 19};
        REPORTER_ASSERT(reporter, collectBreaks(wordIter.get()) == expectedWords);

        auto graphemeIter = apple->makeBreakIterator("ru", SkUnicode::BreakType::kGraphemes);
        REPORTER_ASSERT(reporter, graphemeIter);
        REPORTER_ASSERT(reporter, graphemeIter->setText(text.data(), text.size()));
        std::vector<SkBreakIterator::Position> expectedGraphemes = {
            0, 2, 4, 6, 8, 10, 12, 13, 15, 17, 19
        };
        REPORTER_ASSERT(reporter, collectBreaks(graphemeIter.get()) == expectedGraphemes);
    }

    {
        SkString text("你好世界");
        auto graphemeIter = apple->makeBreakIterator("zh", SkUnicode::BreakType::kGraphemes);
        REPORTER_ASSERT(reporter, graphemeIter);
        REPORTER_ASSERT(reporter, graphemeIter->setText(text.data(), text.size()));
        std::vector<SkBreakIterator::Position> expectedGraphemes = {0, 3, 6, 9, 12};
        REPORTER_ASSERT(reporter, collectBreaks(graphemeIter.get()) == expectedGraphemes);
    }

    {
        SkString text("😀😃😄");
        auto graphemeIter = apple->makeBreakIterator(SkUnicode::BreakType::kGraphemes);
        REPORTER_ASSERT(reporter, graphemeIter);
        REPORTER_ASSERT(reporter, graphemeIter->setText(text.data(), text.size()));
        std::vector<SkBreakIterator::Position> expectedGraphemes = {0, 4, 8, 12};
        REPORTER_ASSERT(reporter, collectBreaks(graphemeIter.get()) == expectedGraphemes);
    }

    {
        // UAX #14 LF and U+2029 Paragraph Separator must produce kHardLineBreakBefore status
        SkString text("Привет\nмир\u2029конец");
        auto lineIter = apple->makeBreakIterator(SkUnicode::BreakType::kLines);
        REPORTER_ASSERT(reporter, lineIter);
        REPORTER_ASSERT(reporter, lineIter->setText(text.data(), text.size()));
        std::vector<SkBreakIterator::Position> breaks;
        std::vector<SkBreakIterator::Status> statuses;
        for (auto pos = lineIter->first(); !lineIter->isDone(); pos = lineIter->next()) {
            breaks.push_back(pos);
            statuses.push_back(lineIter->status());
        }
        std::vector<SkBreakIterator::Position> expectedLines = {0, 13, 22, 32};
        REPORTER_ASSERT(reporter, breaks == expectedLines);
        REPORTER_ASSERT(reporter, !(statuses[0] & SkUnicode::CodeUnitFlags::kHardLineBreakBefore));
        REPORTER_ASSERT(reporter, statuses[1] & SkUnicode::CodeUnitFlags::kHardLineBreakBefore);
        REPORTER_ASSERT(reporter, statuses[2] & SkUnicode::CodeUnitFlags::kHardLineBreakBefore);
        REPORTER_ASSERT(reporter, !(statuses[3] & SkUnicode::CodeUnitFlags::kHardLineBreakBefore));
    }

    {
        // UAX #29 GB3 coalesces CRLF into a single grapheme cluster; empty and CR-only inputs
        // must still emit valid boundary sentinels
        SkString crlf("a\r\nb");
        auto graphemeIter = apple->makeBreakIterator(SkUnicode::BreakType::kGraphemes);
        REPORTER_ASSERT(reporter, graphemeIter->setText(crlf.data(), crlf.size()));
        std::vector<SkBreakIterator::Position> expectedCrlf = {0, 1, 3, 4};
        REPORTER_ASSERT(reporter, collectBreaks(graphemeIter.get()) == expectedCrlf);

        TArray<SkUnicode::CodeUnitFlags, true> flags;
        REPORTER_ASSERT(reporter,
                        apple->computeCodeUnitFlags(crlf.data(), crlf.size(), false, &flags));
        REPORTER_ASSERT(reporter, flags[1] & SkUnicode::CodeUnitFlags::kGraphemeStart);
        REPORTER_ASSERT(reporter, !(flags[2] & SkUnicode::CodeUnitFlags::kGraphemeStart));
        REPORTER_ASSERT(reporter, flags[3] & SkUnicode::CodeUnitFlags::kGraphemeStart);

        auto lineIter = apple->makeBreakIterator(SkUnicode::BreakType::kLines);
        REPORTER_ASSERT(reporter, lineIter->setText("", 0));
        std::vector<SkBreakIterator::Position> expectedEmpty = {0};
        REPORTER_ASSERT(reporter, collectBreaks(lineIter.get()) == expectedEmpty);

        REPORTER_ASSERT(reporter, lineIter->setText("\r", 1));
        std::vector<SkBreakIterator::Position> expectedCrOnly = {0, 1};
        REPORTER_ASSERT(reporter, collectBreaks(lineIter.get()) == expectedCrOnly);

        REPORTER_ASSERT(reporter, apple->computeCodeUnitFlags(crlf.data(), 0, false, &flags));
        REPORTER_ASSERT(reporter, flags.size() == 1);
        REPORTER_ASSERT(reporter,
                        flags[0] == (SkUnicode::CodeUnitFlags::kSoftLineBreakBefore |
                                     SkUnicode::CodeUnitFlags::kGraphemeStart));
    }
}

DEF_TEST(SkUnicode_Apple_BreakIterator_Utf16_Hostile, reporter) {
    auto apple = SkUnicodes::Apple::Make();
    REPORTER_ASSERT(reporter, apple);
    if (!apple) {
        return;
    }

    auto collectBreaks = [](SkBreakIterator* iter) {
        std::vector<SkBreakIterator::Position> breaks;
        for (auto pos = iter->first(); !iter->isDone(); pos = iter->next()) {
            breaks.push_back(pos);
        }
        return breaks;
    };

    {
        std::u16string u16Text = u"Привет мир";
        auto wordIter = apple->makeBreakIterator("ru", SkUnicode::BreakType::kWords);
        REPORTER_ASSERT(reporter, wordIter);
        REPORTER_ASSERT(reporter, wordIter->setText(u16Text.data(), u16Text.size()));
        std::vector<SkBreakIterator::Position> expectedWords = {0, 6, 7, 10};
        REPORTER_ASSERT(reporter, collectBreaks(wordIter.get()) == expectedWords);

        auto graphemeIter = apple->makeBreakIterator("ru", SkUnicode::BreakType::kGraphemes);
        REPORTER_ASSERT(reporter, graphemeIter);
        REPORTER_ASSERT(reporter, graphemeIter->setText(u16Text.data(), u16Text.size()));
        std::vector<SkBreakIterator::Position> expectedGraphemes = {
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10
        };
        REPORTER_ASSERT(reporter, collectBreaks(graphemeIter.get()) == expectedGraphemes);
    }

    {
        std::u16string u16Text = u"😀😃😄";
        auto graphemeIter = apple->makeBreakIterator(SkUnicode::BreakType::kGraphemes);
        REPORTER_ASSERT(reporter, graphemeIter);
        REPORTER_ASSERT(reporter, graphemeIter->setText(u16Text.data(), u16Text.size()));
        std::vector<SkBreakIterator::Position> expectedGraphemes = {0, 2, 4, 6};
        REPORTER_ASSERT(reporter, collectBreaks(graphemeIter.get()) == expectedGraphemes);
    }

    {
        // UAX #14 LF and U+2029 Paragraph Separator must produce kHardLineBreakBefore in UTF-16
        std::u16string u16Text = u"Привет\nмир\u2029конец";
        auto lineIter = apple->makeBreakIterator(SkUnicode::BreakType::kLines);
        REPORTER_ASSERT(reporter, lineIter);
        REPORTER_ASSERT(reporter, lineIter->setText(u16Text.data(), u16Text.size()));
        std::vector<SkBreakIterator::Position> breaks;
        std::vector<SkBreakIterator::Status> statuses;
        for (auto pos = lineIter->first(); !lineIter->isDone(); pos = lineIter->next()) {
            breaks.push_back(pos);
            statuses.push_back(lineIter->status());
        }
        std::vector<SkBreakIterator::Position> expectedLines = {0, 7, 11, 16};
        REPORTER_ASSERT(reporter, breaks == expectedLines);
        REPORTER_ASSERT(reporter, !(statuses[0] & SkUnicode::CodeUnitFlags::kHardLineBreakBefore));
        REPORTER_ASSERT(reporter, statuses[1] & SkUnicode::CodeUnitFlags::kHardLineBreakBefore);
        REPORTER_ASSERT(reporter, statuses[2] & SkUnicode::CodeUnitFlags::kHardLineBreakBefore);
        REPORTER_ASSERT(reporter, !(statuses[3] & SkUnicode::CodeUnitFlags::kHardLineBreakBefore));
    }

    {
        // UAX #29 GB3 coalesces CRLF into a single grapheme cluster in UTF-16
        std::u16string crlf = u"a\r\nb";
        auto graphemeIter = apple->makeBreakIterator(SkUnicode::BreakType::kGraphemes);
        REPORTER_ASSERT(reporter, graphemeIter->setText(crlf.data(), crlf.size()));
        std::vector<SkBreakIterator::Position> expectedCrlf = {0, 1, 3, 4};
        REPORTER_ASSERT(reporter, collectBreaks(graphemeIter.get()) == expectedCrlf);

        TArray<SkUnicode::CodeUnitFlags, true> flags;
        REPORTER_ASSERT(reporter,
                        apple->computeCodeUnitFlags(crlf.data(), crlf.size(), false, &flags));
        REPORTER_ASSERT(reporter, flags[1] & SkUnicode::CodeUnitFlags::kGraphemeStart);
        REPORTER_ASSERT(reporter, !(flags[2] & SkUnicode::CodeUnitFlags::kGraphemeStart));
        REPORTER_ASSERT(reporter, flags[3] & SkUnicode::CodeUnitFlags::kGraphemeStart);

        auto lineIter = apple->makeBreakIterator(SkUnicode::BreakType::kLines);
        REPORTER_ASSERT(reporter, lineIter->setText(u"", 0));
        std::vector<SkBreakIterator::Position> expectedEmpty = {0};
        REPORTER_ASSERT(reporter, collectBreaks(lineIter.get()) == expectedEmpty);

        REPORTER_ASSERT(reporter, lineIter->setText(u"\r", 1));
        std::vector<SkBreakIterator::Position> expectedCrOnly = {0, 1};
        REPORTER_ASSERT(reporter, collectBreaks(lineIter.get()) == expectedCrOnly);

        REPORTER_ASSERT(reporter, apple->computeCodeUnitFlags(crlf.data(), 0, false, &flags));
        REPORTER_ASSERT(reporter, flags.size() == 1);
        REPORTER_ASSERT(reporter,
                        flags[0] == (SkUnicode::CodeUnitFlags::kSoftLineBreakBefore |
                                     SkUnicode::CodeUnitFlags::kGraphemeStart));
    }
}

#endif

[[maybe_unused]] static void SkUnicode_CharProperties(SkUnicode* unicode,
                                                      skiatest::Reporter* reporter) {
    // Cc, Cf, Zl, and Zp control/format ranges vs Variation Selectors Supplement (U+E0100..U+E01EF)
    REPORTER_ASSERT(reporter, unicode->isControl(0x00));
    REPORTER_ASSERT(reporter, unicode->isControl(0x1F));
    REPORTER_ASSERT(reporter, unicode->isControl(0x7F));
    REPORTER_ASSERT(reporter, unicode->isControl(0x9F));
    REPORTER_ASSERT(reporter, unicode->isControl(0x00AD));
    REPORTER_ASSERT(reporter, unicode->isControl(0x0600));
    REPORTER_ASSERT(reporter, unicode->isControl(0x061C));
    REPORTER_ASSERT(reporter, unicode->isControl(0x180E));
    REPORTER_ASSERT(reporter, unicode->isControl(0x200B));
    REPORTER_ASSERT(reporter, unicode->isControl(0x200D));
    REPORTER_ASSERT(reporter, unicode->isControl(0x2028));
    REPORTER_ASSERT(reporter, unicode->isControl(0x2029));
    REPORTER_ASSERT(reporter, unicode->isControl(0x202E));
    REPORTER_ASSERT(reporter, unicode->isControl(0x2060));
    REPORTER_ASSERT(reporter, unicode->isControl(0x2066));
    REPORTER_ASSERT(reporter, unicode->isControl(0xFEFF));
    REPORTER_ASSERT(reporter, unicode->isControl(0xFFF9));
    REPORTER_ASSERT(reporter, unicode->isControl(0xE0001));
    REPORTER_ASSERT(reporter, unicode->isControl(0xE0020));
    REPORTER_ASSERT(reporter, unicode->isControl(0xE007F));
    REPORTER_ASSERT(reporter, !unicode->isControl(0xE0100));
    REPORTER_ASSERT(reporter, !unicode->isControl(0xE0101));
    REPORTER_ASSERT(reporter, !unicode->isControl(0xE0120));
    REPORTER_ASSERT(reporter, !unicode->isControl('A'));
    REPORTER_ASSERT(reporter, !unicode->isControl(' '));

    // ASCII information separators (FS..US) and ZWSP exclusion in isSpace/isWhitespace
    REPORTER_ASSERT(reporter, unicode->isSpace(' '));
    REPORTER_ASSERT(reporter, unicode->isSpace('\t'));
    REPORTER_ASSERT(reporter, unicode->isSpace('\n'));
    REPORTER_ASSERT(reporter, unicode->isSpace(0x001C));
    REPORTER_ASSERT(reporter, unicode->isSpace(0x001F));
    REPORTER_ASSERT(reporter, unicode->isSpace(0x00A0));
    REPORTER_ASSERT(reporter, unicode->isSpace(0x202F));
    REPORTER_ASSERT(reporter, unicode->isSpace(0x3000));
    REPORTER_ASSERT(reporter, !unicode->isSpace(0x200B));
    REPORTER_ASSERT(reporter, !unicode->isSpace('A'));

    REPORTER_ASSERT(reporter, unicode->isWhitespace(' '));
    REPORTER_ASSERT(reporter, unicode->isWhitespace('\t'));
    REPORTER_ASSERT(reporter, unicode->isWhitespace('\n'));
    REPORTER_ASSERT(reporter, unicode->isWhitespace(0x001C));
    REPORTER_ASSERT(reporter, unicode->isWhitespace(0x001F));
    REPORTER_ASSERT(reporter, unicode->isWhitespace(0x3000));
    REPORTER_ASSERT(reporter, !unicode->isWhitespace(0x00A0));
    REPORTER_ASSERT(reporter, !unicode->isWhitespace(0x2007));
    REPORTER_ASSERT(reporter, !unicode->isWhitespace(0x200B));
    REPORTER_ASSERT(reporter, !unicode->isWhitespace(0x202F));
    REPORTER_ASSERT(reporter, !unicode->isWhitespace(0x0085));
    REPORTER_ASSERT(reporter, !unicode->isWhitespace('A'));

    // UAX #14 LF/BK mandatory breaks vs CR and NEL
    REPORTER_ASSERT(reporter, unicode->isHardBreak('\n'));
    REPORTER_ASSERT(reporter, unicode->isHardBreak('\v'));
    REPORTER_ASSERT(reporter, unicode->isHardBreak('\f'));
    REPORTER_ASSERT(reporter, unicode->isHardBreak(0x2028));
    REPORTER_ASSERT(reporter, unicode->isHardBreak(0x2029));
    REPORTER_ASSERT(reporter, !unicode->isHardBreak('\r'));
    REPORTER_ASSERT(reporter, !unicode->isHardBreak(0x0085));
    REPORTER_ASSERT(reporter, !unicode->isHardBreak(' '));
    REPORTER_ASSERT(reporter, !unicode->isHardBreak('A'));

    // Unicode Default_Ignorable_Code_Point property
    REPORTER_ASSERT(reporter, unicode->isDefaultIgnorable(0x00AD));
    REPORTER_ASSERT(reporter, unicode->isDefaultIgnorable(0x034F));
    REPORTER_ASSERT(reporter, unicode->isDefaultIgnorable(0x061C));
    REPORTER_ASSERT(reporter, unicode->isDefaultIgnorable(0x115F));
    REPORTER_ASSERT(reporter, unicode->isDefaultIgnorable(0x1160));
    REPORTER_ASSERT(reporter, unicode->isDefaultIgnorable(0x17B4));
    REPORTER_ASSERT(reporter, unicode->isDefaultIgnorable(0x17B5));
    REPORTER_ASSERT(reporter, unicode->isDefaultIgnorable(0x180B));
    REPORTER_ASSERT(reporter, unicode->isDefaultIgnorable(0x180E));
    REPORTER_ASSERT(reporter, unicode->isDefaultIgnorable(0x180F));
    REPORTER_ASSERT(reporter, unicode->isDefaultIgnorable(0x200B));
    REPORTER_ASSERT(reporter, unicode->isDefaultIgnorable(0x200C));
    REPORTER_ASSERT(reporter, unicode->isDefaultIgnorable(0x200D));
    REPORTER_ASSERT(reporter, unicode->isDefaultIgnorable(0x200E));
    REPORTER_ASSERT(reporter, unicode->isDefaultIgnorable(0x200F));
    REPORTER_ASSERT(reporter, unicode->isDefaultIgnorable(0x202A));
    REPORTER_ASSERT(reporter, unicode->isDefaultIgnorable(0x202E));
    REPORTER_ASSERT(reporter, unicode->isDefaultIgnorable(0x2060));
    REPORTER_ASSERT(reporter, unicode->isDefaultIgnorable(0x2066));
    REPORTER_ASSERT(reporter, unicode->isDefaultIgnorable(0x2069));
    REPORTER_ASSERT(reporter, unicode->isDefaultIgnorable(0x206F));
    REPORTER_ASSERT(reporter, unicode->isDefaultIgnorable(0x3164));
    REPORTER_ASSERT(reporter, unicode->isDefaultIgnorable(0xFE00));
    REPORTER_ASSERT(reporter, unicode->isDefaultIgnorable(0xFE0F));
    REPORTER_ASSERT(reporter, unicode->isDefaultIgnorable(0xFEFF));
    REPORTER_ASSERT(reporter, unicode->isDefaultIgnorable(0xFFA0));
    REPORTER_ASSERT(reporter, unicode->isDefaultIgnorable(0xFFF0));
    REPORTER_ASSERT(reporter, unicode->isDefaultIgnorable(0xFFF8));
    REPORTER_ASSERT(reporter, unicode->isDefaultIgnorable(0x1BCA0));
    REPORTER_ASSERT(reporter, unicode->isDefaultIgnorable(0x1D173));
    REPORTER_ASSERT(reporter, unicode->isDefaultIgnorable(0xE0000));
    REPORTER_ASSERT(reporter, unicode->isDefaultIgnorable(0xE0001));
    REPORTER_ASSERT(reporter, unicode->isDefaultIgnorable(0xE0FFF));
    REPORTER_ASSERT(reporter, !unicode->isDefaultIgnorable('A'));
    REPORTER_ASSERT(reporter, !unicode->isDefaultIgnorable(' '));
    REPORTER_ASSERT(reporter, !unicode->isDefaultIgnorable('\n'));
    REPORTER_ASSERT(reporter, !unicode->isDefaultIgnorable(0x00A0));
    REPORTER_ASSERT(reporter, !unicode->isDefaultIgnorable(0x200A));
    REPORTER_ASSERT(reporter, !unicode->isDefaultIgnorable(0x2010));
}

#if defined(SK_UNICODE_ICU_IMPLEMENTATION)
// Match other SkUnicode_Compiled_* tests: Windows bots do not bundle icudtl.dat in the test cwd
UNIX_ONLY_TEST(SkUnicode_Compiled_CharProperties, reporter) {
    auto icu = SkUnicodes::ICU::Make();
    REPORTER_ASSERT(reporter, icu);
    if (!icu) {
        return;
    }
    SkUnicode_CharProperties(icu.get(), reporter);
}
#endif

#if defined(SK_UNICODE_APPLE_IMPLEMENTATION)
DEF_TEST(SkUnicode_Apple_CharProperties, reporter) {
    auto apple = SkUnicodes::Apple::Make();
    REPORTER_ASSERT(reporter, apple);
    if (!apple) {
        return;
    }
    SkUnicode_CharProperties(apple.get(), reporter);

    // Inclusive ideographic block upper bounds in SkUnicodeHardCodedCharProperties
    REPORTER_ASSERT(reporter, apple->isIdeographic(0x11FF));
    REPORTER_ASSERT(reporter, apple->isIdeographic(0xA4CF));
    REPORTER_ASSERT(reporter, apple->isIdeographic(0xD7AF));
    REPORTER_ASSERT(reporter, apple->isIdeographic(0x2FFFF));
    REPORTER_ASSERT(reporter, !apple->isIdeographic(0x1200));
    REPORTER_ASSERT(reporter, !apple->isIdeographic(0x30000));
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

#ifdef SK_UNICODE_APPLE_IMPLEMENTATION
DEF_TEST(SkUnicode_Apple_Emoji, reporter) {
    auto apple = SkUnicodes::Apple::Make();
    if (!apple) {
        REPORTER_ASSERT(reporter, apple);
        return;
    }
    SkUnicode_Emoji(apple.get(), reporter);
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

#ifdef SK_UNICODE_APPLE_IMPLEMENTATION
DEF_TEST(SkUnicode_Apple_Ideographic, reporter) {
    auto apple = SkUnicodes::Apple::Make();
    if (!apple) {
        REPORTER_ASSERT(reporter, apple);
        return;
    }
    SkUnicode_Ideographic(apple.get(), reporter);
}
#endif
