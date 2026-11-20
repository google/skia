/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSpan.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "modules/skparagraph/include/DartTypes.h"
#include "modules/skparagraph/include/FontCollection.h"
#include "modules/skparagraph/include/Paragraph.h"
#include "modules/skparagraph/include/ParagraphStyle.h"
#include "modules/skparagraph/include/TextShadow.h"
#include "modules/skparagraph/include/TextStyle.h"
#include "modules/skparagraph/src/ParagraphBuilderImpl.h"
#include "modules/skparagraph/src/ParagraphImpl.h"
#include "modules/skparagraph/src/Run.h"
#include "modules/skparagraph/src/TextLine.h"
#include "modules/skparagraph/tests/SkParagraphTestUtils.h"
#include "modules/skparagraph/tests/SkShaperJSONWriter.h"
#include "modules/skparagraph/utils/TestFontCollection.h"
#include "tests/Test.h"
#include "tools/Resources.h"
#include "tools/fonts/FontToolUtils.h"

#include <string.h>
#include <chrono>
#include <string>
#include <utility>

using namespace skia_private;

using namespace skia::textlayout;
using namespace skia::textlayout::test;

UNIX_ONLY_TEST(SkParagraph_SimpleParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    const char* text = "Hello World Text Dialog";
    const size_t len = strlen(text);

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text, len);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);
    REPORTER_ASSERT(reporter, paragraph->unresolvedGlyphs() == 0);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, impl->runs().size() == 1);
    REPORTER_ASSERT(reporter, impl->styles().size() == 1);  // paragraph style does not count
    REPORTER_ASSERT(reporter, impl->styles()[0].fStyle.equals(text_style));

    size_t index = 0;
    for (auto& line : impl->lines()) {
        line.scanStyles(StyleType::kDecorations,
                        [&index, reporter](TextRange textRange,
                                           const TextStyle& style,
                                           const TextLine::ClipContext& context) {
                            REPORTER_ASSERT(reporter, index == 0);
                            REPORTER_ASSERT(reporter, style.getColor() == SK_ColorBLACK);
                            ++index;
                        });
    }
}

UNIX_ONLY_TEST(SkParagraph_DefaultStyleParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    TestCanvas canvas("SkParagraph_DefaultStyleParagraph.png");
    const char* text = "No TextStyle! Uh Oh!";
    const size_t len = strlen(text);

    ParagraphStyle paragraph_style;
    TextStyle defaultStyle;
    defaultStyle.setFontFamilies({SkString("Roboto")});
    defaultStyle.setColor(SK_ColorBLACK);
    paragraph_style.setTextStyle(defaultStyle);
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());
    builder.addText(text, len);

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);
    paragraph->paint(canvas.get(), 10.0, 15.0);

    REPORTER_ASSERT(reporter, paragraph->unresolvedGlyphs() == 0);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());

    REPORTER_ASSERT(reporter, impl->runs().size() == 1);
    REPORTER_ASSERT(reporter, impl->styles().size() == 1);
    REPORTER_ASSERT(reporter, impl->lines().size() == 1);

    size_t index = 0;
    impl->lines()[0].scanStyles(
            StyleType::kAllAttributes,
            [&](TextRange textRange, const TextStyle& style, const TextLine::ClipContext& context) {
                REPORTER_ASSERT(reporter, style.equals(paragraph_style.getTextStyle()));
                REPORTER_ASSERT(reporter, equal(impl->text().data(), textRange, text));
                ++index;
                return true;
            });
    REPORTER_ASSERT(reporter, index == 1);
}

UNIX_ONLY_TEST(SkParagraph_ChineseParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    TestCanvas canvas("SkParagraph_ChineseParagraph.png");
    const char* text =
            "     左線読設重説切後碁給能上目秘使約。満毎冠行来昼本可必図将発確年。今属場育"
            "図情闘陰野高備込制詩西校客。審対江置講今固残必託地集済決維駆年策。立得庭"
            "際輝求佐抗蒼提夜合逃表。注統天言件自謙雅載報紙喪。作画稿愛器灯女書利変探"
            "訃第金線朝開化建。子戦年帝励害表月幕株漠新期刊人秘。図的海力生禁挙保天戦"
            "聞条年所在口。";
    const size_t len = strlen(text);

    ParagraphStyle paragraph_style;
    paragraph_style.setMaxLines(14);
    paragraph_style.setTextAlign(TextAlign::kJustify);
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());

    auto decoration = (TextDecoration)(TextDecoration::kUnderline | TextDecoration::kOverline |
                                       TextDecoration::kLineThrough);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Source Han Serif CN")});
    text_style.setFontSize(35);
    text_style.setColor(SK_ColorBLACK);
    text_style.setLetterSpacing(2);
    text_style.setHeight(1);
    text_style.setDecoration(decoration);
    text_style.setDecorationColor(SK_ColorBLACK);
    text_style.setDecorationStyle(TextDecorationStyle::kSolid);
    builder.pushStyle(text_style);
    builder.addText(text, len);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth - 100);
    paragraph->paint(canvas.get(), 0, 0);

    REPORTER_ASSERT(reporter, paragraph->unresolvedGlyphs() == 0);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());

    REPORTER_ASSERT(reporter, impl->runs().size() == 1);
    REPORTER_ASSERT(reporter, impl->lines().size() == 7);
    REPORTER_ASSERT(reporter, impl->styles().size() == 1);
    if (impl->styles().size() >= 1) {
        REPORTER_ASSERT(reporter, impl->styles()[0].fStyle.equals(text_style));
    }
}

// Checked: NO DIFF
UNIX_ONLY_TEST(SkParagraph_LongWordParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    TestCanvas canvas("SkParagraph_LongWordParagraph.png");
    const char* text =
            "A "
            "veryverylongwordtoseewherethiswillwraporifitwillatallandifitdoesthenthat"
            "wouldbeagoodthingbecausethebreakingisworking.";
    const size_t len = strlen(text);

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setColor(SK_ColorRED);
    text_style.setFontSize(31);
    text_style.setLetterSpacing(0);
    text_style.setWordSpacing(0);
    text_style.setColor(SK_ColorBLACK);
    text_style.setHeight(1);
    builder.pushStyle(text_style);
    builder.addText(text, len);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth / 2);
    paragraph->paint(canvas.get(), 0, 0);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, impl->text().size() == std::string{text}.length());
    REPORTER_ASSERT(reporter, impl->runs().size() == 1);
    REPORTER_ASSERT(reporter, impl->styles().size() == 1);
    if (impl->styles().size() >= 1) {
        REPORTER_ASSERT(reporter, impl->styles()[0].fStyle.equals(text_style));
    }
    REPORTER_ASSERT(reporter, impl->lines().size() == 4);
    if (impl->lines().size() >= 4) {
        REPORTER_ASSERT(reporter, impl->lines()[0].width() > TestCanvasWidth / 2 - 20);
        REPORTER_ASSERT(reporter, impl->lines()[1].width() > TestCanvasWidth / 2 - 20);
        REPORTER_ASSERT(reporter, impl->lines()[2].width() > TestCanvasWidth / 2 - 20);
    }
}

// Text with no break opportunity is broken inside the word, once on every line. Each line is as
// full as the width allows, and the word is measured the same at every width.
UNIX_ONLY_TEST(SkParagraph_UnbreakableTextParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setFontSize(20);
    text_style.setColor(SK_ColorBLACK);
    ParagraphStyle paragraph_style;
    paragraph_style.setTextStyle(text_style);
    auto layout = [&](const std::string& text, SkScalar width) {
        ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());
        builder.pushStyle(text_style);
        builder.addText(text.data(), text.size());
        auto paragraph = builder.Build();
        paragraph->layout(width);
        return paragraph;
    };
    auto repeat = [](const std::string& unit, size_t count) {
        std::string text;
        for (size_t i = 0; i < count; ++i) {
            text += unit;
        }
        return text;
    };
    // Checks that the lines cover the text in order and fit the width. All lines but the last
    // are full: the first cluster of the next line would not fit on them.
    auto checkLines = [&](skiatest::Reporter* reporter,
                          Paragraph* paragraph,
                          size_t size,
                          SkScalar width,
                          bool full) {
        auto impl = static_cast<ParagraphImpl*>(paragraph);
        auto lines = impl->lines();
        size_t start = 0;
        for (size_t i = 0; i < lines.size(); ++i) {
            auto& line = lines[i];
            REPORTER_ASSERT(reporter, line.text().start == start);
            start = line.text().end;
            REPORTER_ASSERT(reporter, line.width() <= width + 0.25f);
            if (full && i + 1 < lines.size()) {
                auto next = line.clustersWithSpaces().end;
                REPORTER_ASSERT(reporter, lines[i + 1].clustersWithSpaces().start == next);
                SkScalar nextWidth = impl->cluster(next).width();
                REPORTER_ASSERT(reporter, line.width() + nextWidth > width - 0.25f);
            }
        }
        REPORTER_ASSERT(reporter, start == size);
    };

    // Minified JSON has a few break opportunities, so only the lines are checked
    std::string json;
    for (int i = 0; i < 50; ++i) {
        json += "{\"id\":" + std::to_string(i) +
                ",\"name\":\"abc\",\"tags\":[\"x\",\"y\"],"
                "\"ok\":true},";
    }
    for (SkScalar width : {50.f, 300.f, 301.f}) {
        auto paragraph = layout(json, width);
        checkLines(reporter, paragraph.get(), json.size(), width, false);
    }
    const std::pair<const char*, std::string> kinds[] = {
            {"latin", repeat("abcdef", 500)},
            {"cjk full stops", repeat("\xE3\x80\x82", 3000)},
            {"exclamation marks", repeat("!", 3000)},
            {"digits", repeat("666", 1000)},
    };
    for (auto& [name, text] : kinds) {
        SkScalar minWidth = -1;
        for (SkScalar width : {50.f, 300.f, 301.f}) {
            auto paragraph = layout(text, width);
            auto impl = static_cast<ParagraphImpl*>(paragraph.get());
            // No break opportunity in the text, so the line breaks do not depend on the words
            auto clusters = impl->clusters();
            bool unbreakable = true;
            // (The last cluster is the end of the text)
            for (size_t i = 0; i + 2 < clusters.size(); ++i) {
                unbreakable &= !clusters[i].isSoftBreak(impl) && !clusters[i].isHardBreak();
            }
            REPORTER_ASSERT(reporter, unbreakable, "%s", name);
            checkLines(reporter, paragraph.get(), text.size(), width, true);
            REPORTER_ASSERT(reporter, impl->lines().size() > 1, "%s", name);
            // The word is as wide as the text
            if (minWidth < 0) {
                minWidth = paragraph->getMinIntrinsicWidth();
            }
            REPORTER_ASSERT(reporter, paragraph->getMinIntrinsicWidth() == minWidth, "%s", name);
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(
                                    minWidth, paragraph->getMaxIntrinsicWidth(), minWidth / 1000),
                            "%s",
                            name);
        }
    }

    // A soft hyphen ends each word in the text, and every word is too long for the line
    std::string softHyphens = repeat(std::string(900, 'a') + "\xC2\xAD", 3) + std::string(20, 'b');
    SkScalar minWidth = -1;
    for (SkScalar width : {50.f, 300.f, 301.f}) {
        auto paragraph = layout(softHyphens, width);
        checkLines(reporter, paragraph.get(), softHyphens.size(), width, false);
        if (minWidth < 0) {
            minWidth = paragraph->getMinIntrinsicWidth();
        }
        REPORTER_ASSERT(reporter, paragraph->getMinIntrinsicWidth() == minWidth);
    }
    auto word = layout(std::string(900, 'a'), SK_ScalarInfinity);
    REPORTER_ASSERT(reporter, minWidth >= word->getMaxIntrinsicWidth());

    // A long word after a long word is not measured as the first one
    std::string twoWords = std::string(600, 'a') + " " + std::string(2000, 'b');
    for (SkScalar width : {50.f, 300.f}) {
        auto two = layout(twoWords, width);
        checkLines(reporter, two.get(), twoWords.size(), width, false);
        SkScalar longer = layout(std::string(2000, 'b'), width)->getMinIntrinsicWidth();
        REPORTER_ASSERT(reporter,
                        SkScalarNearlyEqual(two->getMinIntrinsicWidth(), longer, 0.5f),
                        "%f %f",
                        two->getMinIntrinsicWidth(),
                        longer);
    }

    // The no-break space comes just where a line (the 55th) is full: that line ends after it,
    // and the rest of the word goes to the next line
    std::string nbsp = std::string(1485, 'a') + "\xC2\xA0" + std::string(20, 'b');
    auto paragraph = layout(nbsp, 300);
    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, impl->lines().size() >= 2);
    if (impl->lines().size() >= 2) {
        auto& last = impl->lines().back();
        REPORTER_ASSERT(reporter, last.text().start == 1487, "%zu", last.text().start);
        REPORTER_ASSERT(reporter, last.text().end == nbsp.size());
    }
}

// The time to break a word into lines grows with the length of the word, not with its square
UNIX_ONLY_TEST(SkParagraph_VeryLongWordParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setFontSize(20);
    text_style.setColor(SK_ColorBLACK);
    ParagraphStyle paragraph_style;
    paragraph_style.setTextStyle(text_style);
    std::string text(50000, 'a');
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());
    builder.pushStyle(text_style);
    builder.addText(text.data(), text.size());
    auto paragraph = builder.Build();
    // Shape and build clusters outside the timer so ASAN TArray poisoning is not measured
    paragraph->layout(SK_ScalarInfinity);
    auto start = std::chrono::steady_clock::now();
    paragraph->layout(60);
    auto ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start);
    // The layout takes about 30 ms, and about 3 s when every line walks to the end of the word
    REPORTER_ASSERT(reporter, ms.count() < 1000, "%f ms", ms.count());
    REPORTER_ASSERT(reporter, static_cast<ParagraphImpl*>(paragraph.get())->lines().size() > 1000);
}

// Checked: DIFF?
UNIX_ONLY_TEST(SkParagraph_KernScaleParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    TestCanvas canvas("SkParagraph_KernScaleParagraph.png");

    const char* text1 = "AVAVAWAH A0 V0 VA To The Lo";
    const char* text2 =
            " Dialog Text List lots of words to see "
            "if kerning works on a bigger set of characters AVAVAW";
    float scale = 3.0f;
    ParagraphStyle paragraph_style;
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());
    TextStyle text_style;
    text_style.setFontFamilies({SkString("Droid Serif")});
    text_style.setFontSize(100 / scale);
    text_style.setColor(SK_ColorBLACK);

    builder.pushStyle(text_style);
    builder.addText(text1, strlen(text1));
    builder.pushStyle(text_style);
    builder.addText("A", 1);
    builder.pushStyle(text_style);
    builder.addText("V", 1);
    text_style.setFontSize(14 / scale);
    builder.pushStyle(text_style);
    builder.addText(text2, strlen(text2));
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth / scale);
    canvas.get()->scale(scale, scale);
    paragraph->paint(canvas.get(), 0, 0);
    canvas.get()->scale(1, 1);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());

    // First and second lines must have the same width, the third one must be bigger
    REPORTER_ASSERT(reporter, impl->lines().size() == 3);
    if (impl->lines().size() >= 3) {
        REPORTER_ASSERT(reporter,
                        SkScalarNearlyEqual(impl->lines()[0].width(), 285.858f, EPSILON100));
        REPORTER_ASSERT(reporter,
                        SkScalarNearlyEqual(impl->lines()[1].width(), 329.709f, EPSILON100));
        REPORTER_ASSERT(reporter,
                        SkScalarNearlyEqual(impl->lines()[2].width(), 120.619f, EPSILON100));
        REPORTER_ASSERT(reporter,
                        SkScalarNearlyEqual(impl->lines()[0].height(), 39.00f, EPSILON100));
        REPORTER_ASSERT(reporter,
                        SkScalarNearlyEqual(impl->lines()[1].height(), 39.00f, EPSILON100));
        REPORTER_ASSERT(reporter,
                        SkScalarNearlyEqual(impl->lines()[2].height(), 05.00f, EPSILON100));
    }
}

// Checked: DIFF (line breaking)
UNIX_ONLY_TEST(SkParagraph_RepeatLayoutParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    TestCanvas canvas("SkParagraph_RepeatLayoutParagraph.png");
    const char* text =
            "Sentence to layout at diff widths to get diff line counts. short words "
            "short words short words short words short words short words short words "
            "short words short words short words short words short words short words "
            "end";
    const size_t len = strlen(text);

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setFontSize(31);
    text_style.setColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text, len);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(300);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    // Some of the formatting lazily done on paint
    REPORTER_ASSERT(reporter, impl->runs().size() == 1);
    REPORTER_ASSERT(reporter, impl->styles().size() == 1);
    REPORTER_ASSERT(reporter, impl->lines().size() == 12);

    paragraph->layout(600);
    paragraph->paint(canvas.get(), 0, 0);
    REPORTER_ASSERT(reporter, impl->runs().size() == 1);
    REPORTER_ASSERT(reporter, impl->styles().size() == 1);
    REPORTER_ASSERT(reporter, impl->lines().size() == 6);
}

// Disable until I sort out fonts
DEF_TEST_DISABLED(SkParagraph_JSON1, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    const char* text = "👨‍👩‍👧‍👦";
    const size_t len = strlen(text);

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Noto Color Emoji")});
    builder.pushStyle(text_style);
    builder.addText(text, len);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, impl->runs().size() == 1);
    auto& run = impl->runs().front();

    auto cluster = 0;
    SkShaperJSONWriter::VisualizeClusters(
            text,
            0,
            std::strlen(text),
            run.glyphs(),
            run.clusterIndexes(),
            [&](int codePointCount, SkSpan<const char> utf1to1, SkSpan<const SkGlyphID> glyph1to1) {
                if (cluster == 0) {
                    std::string toCheckUtf8{utf1to1.data(), utf1to1.size()};
                    SkASSERT(std::strcmp(text, utf1to1.data()) == 0);
                    SkASSERT(glyph1to1.size() == 1);
                    SkASSERT(*glyph1to1.begin() == 1611);
                }
                ++cluster;
            });
    REPORTER_ASSERT(reporter, cluster <= 2);
}

// Disable until I sort out fonts
DEF_TEST_DISABLED(SkParagraph_JSON2, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    const char* text = "p〠q";
    const size_t len = strlen(text);

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Noto Sans CJK JP")});
    text_style.setColor(SK_ColorBLACK);
    text_style.setFontSize(50);
    builder.pushStyle(text_style);
    builder.addText(text, len);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, impl->runs().size() == 1);

    auto cluster = 0;
    for (auto& run : impl->runs()) {
        SkShaperJSONWriter::VisualizeClusters(
                impl->text().data() + run.textRange().start,
                0,
                run.textRange().width(),
                run.glyphs(),
                run.clusterIndexes(),
                [&](int codePointCount,
                    SkSpan<const char> utf1to1,
                    SkSpan<const SkGlyphID> glyph1to1) {
                    if (cluster == 0) {
                        std::string toCheckUtf8{utf1to1.data(), utf1to1.size()};
                        SkASSERT(std::strcmp(text, utf1to1.data()) == 0);
                        SkASSERT(glyph1to1.size() == 3);
                    }
                    ++cluster;
                });
    }

    REPORTER_ASSERT(reporter, cluster <= 2);
}

// This test does not produce an image
UNIX_ONLY_TEST(SkParagraph_NullInMiddleOfText, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    fontCollection->setDefaultFontManager(ToolUtils::TestFontMgr());

    const SkString text("null terminator ->\u0000<- on purpose did you see it?");

    ParagraphStyle paragraph_style;
    TextStyle text_style;
    text_style.setColor(SK_ColorBLACK);
    text_style.setFontSize(16);
    text_style.setFontFamilies({SkString("Roboto")});
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());
    builder.addText(text.c_str(), text.size());

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);
    REPORTER_ASSERT(reporter, paragraph->getHeight() > 0);
}

UNIX_ONLY_TEST(SkParagraph_FormattingInfinity, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    fontCollection->setDefaultFontManager(ToolUtils::TestFontMgr());
    TestCanvas canvas("SkParagraph_FormattingInfinity.png");

    const char* text = "Some text\nAnother line";

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(SK_ColorBLACK);

    TextStyle textStyle;
    textStyle.setForegroundColor(paint);
    textStyle.setFontFamilies({SkString("Roboto")});
    ParagraphStyle paragraphStyle;
    paragraphStyle.setTextStyle(textStyle);

    auto draw = [&](const char* prefix, TextAlign textAlign, TextDirection textDirection) {
        paragraphStyle.setTextAlign(textAlign);
        paragraphStyle.setTextDirection(textDirection);
        ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());
        builder.addText(text);
        auto paragraph = builder.Build();
        paragraph->layout(SK_ScalarInfinity);
        paragraph->paint(canvas.get(), 0, 0);
        canvas.get()->translate(0, 100);
    };

    draw("left", TextAlign::kLeft, TextDirection::kLtr);
    draw("right", TextAlign::kRight, TextDirection::kLtr);
    draw("center", TextAlign::kCenter, TextDirection::kLtr);
    draw("justify LTR", TextAlign::kJustify, TextDirection::kLtr);
    draw("justify RTL", TextAlign::kJustify, TextDirection::kRtl);
}

UNIX_ONLY_TEST(SkParagraph_Infinity, reporter) {
    SkASSERT(nearlyEqual(1, SK_ScalarInfinity) == false);
    SkASSERT(nearlyEqual(1, SK_ScalarNegativeInfinity) == false);
    SkASSERT(nearlyEqual(1, SK_ScalarNaN) == false);

    SkASSERT(nearlyEqual(SK_ScalarInfinity, SK_ScalarInfinity) == true);
    SkASSERT(nearlyEqual(SK_ScalarInfinity, SK_ScalarNegativeInfinity) == false);
    SkASSERT(nearlyEqual(SK_ScalarInfinity, SK_ScalarNaN) == false);

    SkASSERT(nearlyEqual(SK_ScalarNegativeInfinity, SK_ScalarInfinity) == false);
    SkASSERT(nearlyEqual(SK_ScalarNegativeInfinity, SK_ScalarNegativeInfinity) == true);
    SkASSERT(nearlyEqual(SK_ScalarNegativeInfinity, SK_ScalarNaN) == false);

    SkASSERT(nearlyEqual(SK_ScalarNaN, SK_ScalarInfinity) == false);
    SkASSERT(nearlyEqual(SK_ScalarNaN, SK_ScalarNegativeInfinity) == false);
    // Style equality backs cache keys, so it must hold for a NaN value too
    SkASSERT(nearlyEqual(SK_ScalarNaN, SK_ScalarNaN) == true);
}

UNIX_ONLY_TEST(SkParagraph_SimpleParagraphReset, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    const char* text = "Hello World Text Dialog";
    const size_t len = strlen(text);

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());

    for (int iteration = 0; iteration < 2; iteration += 1) {
        builder.Reset();
        REPORTER_ASSERT(reporter, builder.peekStyle().equals(paragraph_style.getTextStyle()));

        TextStyle text_style;
        text_style.setFontFamilies({SkString("Roboto")});
        text_style.setColor(SK_ColorBLACK);
        builder.pushStyle(text_style);
        builder.addText(text, len);
        builder.pop();

        auto paragraph = builder.Build();
        paragraph->layout(TestCanvasWidth);
        REPORTER_ASSERT(reporter, paragraph->unresolvedGlyphs() == 0);

        auto impl = static_cast<ParagraphImpl*>(paragraph.get());
        REPORTER_ASSERT(reporter, impl->runs().size() == 1);
        REPORTER_ASSERT(reporter, impl->styles().size() == 1);  // paragraph style does not count
        REPORTER_ASSERT(reporter, impl->styles()[0].fStyle.equals(text_style));

        size_t index = 0;
        for (auto& line : impl->lines()) {
            line.scanStyles(StyleType::kDecorations,
                            [&index, reporter](TextRange textRange,
                                               const TextStyle& style,
                                               const TextLine::ClipContext& context) {
                                REPORTER_ASSERT(reporter, index == 0);
                                REPORTER_ASSERT(reporter, style.getColor() == SK_ColorBLACK);
                                ++index;
                            });
        }
    }
}

// Regression test for https://github.com/flutter/flutter/issues/141080
DEF_TEST(SkParagraph_DiacriticWithMultipleStyles, reporter) {
    sk_sp<TestFontCollection> fontCollection =
            sk_make_sp<TestFontCollection>(GetResourcePath("fonts").c_str(), false);
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    REPORTER_ASSERT(reporter, fontCollection->addFontFromFile("abc/abc+grave.ttf", "abc+grave"));

    auto checkTwoStyles = [&](const char* family,
                              const char* leftText,
                              const char* rightText,
                              size_t expectedRunGlyphs,
                              size_t expectedPos0,
                              size_t expectedSize0,
                              size_t expectedPos1,
                              size_t expectedSize1) {
        TextStyle style1;
        style1.setFontFamilies({SkString(family)});
        style1.setFontSize(40);
        style1.setColor(SK_ColorRED);

        TextStyle style2 = style1;
        style2.setColor(SK_ColorBLUE);

        ParagraphStyle paragraphStyle;
        ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());
        builder.pushStyle(style1);
        builder.addText(leftText, strlen(leftText));
        builder.pop();
        builder.pushStyle(style2);
        builder.addText(rightText, strlen(rightText));
        builder.pop();

        auto paragraph = builder.Build();
        paragraph->layout(1000);
        REPORTER_ASSERT(reporter, paragraph->unresolvedGlyphs() == 0);

        auto* impl = static_cast<ParagraphImpl*>(paragraph.get());
        REPORTER_ASSERT(reporter, impl->runs().size() == 1);
        REPORTER_ASSERT(reporter, impl->runs()[0].size() == expectedRunGlyphs);
        REPORTER_ASSERT(reporter, impl->lines().size() == 1);

        int index = 0;
        SkScalar totalWidth = 0;
        SkScalar shifts[2] = {0, 0};
        impl->lines()[0].scanStyles(
                StyleType::kForeground,
                [&](TextRange, const TextStyle&, const TextLine::ClipContext& context) {
                    REPORTER_ASSERT(reporter, !context.clippingNeeded);
                    if (index == 0) {
                        REPORTER_ASSERT(
                                reporter,
                                context.pos == expectedPos0 && context.size == expectedSize0);
                    } else if (index == 1) {
                        REPORTER_ASSERT(reporter, context.size == expectedSize1);
                        if (expectedSize1 > 0) {
                            REPORTER_ASSERT(reporter, context.pos == expectedPos1);
                        }
                    }
                    if (index < 2) {
                        shifts[index] = context.fTextShift;
                    }
                    totalWidth += context.clip.width();
                    ++index;
                });
        REPORTER_ASSERT(reporter, index == 2);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(totalWidth, impl->lines()[0].width()));
        if (expectedRunGlyphs == 2 && expectedSize0 == 1 && expectedSize1 == 1) {
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(shifts[0], shifts[1]));
        }
    };

    // Precomposed 1-glyph cluster ("A" + U+0301 -> "Á")
    checkTwoStyles("Roboto", "A", "\xCC\x81", 1, 0, 1, 0, 0);
    // Multi-cluster style spans split across a precomposed cluster ("abA" + U+0301 + "CD")
    checkTwoStyles("Roboto",
                   "abA",
                   "\xCC\x81"
                   "CD",
                   5,
                   0,
                   3,
                   3,
                   2);
    // Multi-glyph 1:1 cluster in LTR ("a" + U+0300 -> 'a' + gravecomb in 1 cluster)
    checkTwoStyles("abc+grave", "a", "\xCC\x80", 2, 0, 1, 1, 1);
}

// Uses DEF_TEST rather than UNIX_ONLY_TEST so Windows ASAN bots exercise STArray<4, ...>
// reallocation of TextStyle, Block, and Placeholder when growing past inline capacity 4
DEF_TEST(SkParagraph_InlineCapacityGrowthRelocation, reporter) {
    auto fontCollection = sk_make_sp<FontCollection>();
    fontCollection->setDefaultFontManager(ToolUtils::TestFontMgr());
    fontCollection->enableFontFallback();

    ParagraphStyle paragraphStyle;
    ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());

    for (int i = 0; i < 6; ++i) {
        TextStyle style;
        style.setFontSize(14.0f + i);
        style.setFontFamilies({SkString("sans-serif")});
        style.addFontFeature(SkString("liga"), i % 2);
        style.addShadow(TextShadow(SK_ColorBLACK, SkPoint::Make(1.0f, 1.0f), 2.0f));
        if (i % 2 == 1) {
            SkFontArguments::VariationPosition::Coordinate coord = {
                    SkSetFourByteTag('w', 'g', 'h', 't'), 400.0f + i * 50.0f};
            SkFontArguments fontArgs;
            fontArgs.setVariationDesignPosition({&coord, 1});
            style.setFontArguments(fontArgs);
        }
        builder.pushStyle(style);
        builder.addText("word ");
        if (i < 5) {
            builder.addPlaceholder(PlaceholderStyle(10.0f,
                                                    12.0f,
                                                    PlaceholderAlignment::kBaseline,
                                                    TextBaseline::kAlphabetic,
                                                    0.0f));
        }
    }
    for (int i = 0; i < 6; ++i) {
        builder.pop();
    }

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);

    auto rects = paragraph->getRectsForPlaceholders();
    REPORTER_ASSERT(reporter, rects.size() == 5);

    auto* impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, impl->styles().size() == 11);
    REPORTER_ASSERT(reporter, !impl->styles()[0].fStyle.getFontArguments().has_value());
    REPORTER_ASSERT(reporter, !impl->styles()[1].fStyle.getFontArguments().has_value());
    REPORTER_ASSERT(reporter, impl->styles()[2].fStyle.getFontArguments().has_value());
    REPORTER_ASSERT(reporter, impl->styles()[2].fStyle.getFontFeatureNumber() == 1);
    REPORTER_ASSERT(reporter, impl->styles()[2].fStyle.getShadowNumber() == 1);
}
