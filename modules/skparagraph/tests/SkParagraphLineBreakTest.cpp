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
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "modules/skparagraph/include/DartTypes.h"
#include "modules/skparagraph/include/FontCollection.h"
#include "modules/skparagraph/include/Paragraph.h"
#include "modules/skparagraph/include/ParagraphCache.h"
#include "modules/skparagraph/include/ParagraphStyle.h"
#include "modules/skparagraph/include/TextStyle.h"
#include "modules/skparagraph/src/ParagraphBuilderImpl.h"
#include "modules/skparagraph/src/ParagraphImpl.h"
#include "modules/skparagraph/src/Run.h"
#include "modules/skparagraph/src/TextLine.h"
#include "modules/skparagraph/tests/SkParagraphTestUtils.h"
#include "tests/Test.h"
#include "tools/fonts/FontToolUtils.h"

#include <string.h>
#include <algorithm>
#include <string>
#include <vector>

using namespace skia::textlayout;
using namespace skia::textlayout::test;

UNIX_ONLY_TEST(SkParagraph_Rounding_Off_LineBreaks, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    const char* text = "AAAAAAAAAA";
    const size_t len = strlen(text);

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    paragraph_style.setApplyRoundingHack(false);
    TextStyle text_style;
    text_style.setFontFamilies({SkString("Ahem")});
    text_style.setColor(SK_ColorBLACK);

    auto testFontSize = {1.5f, 10.0f / 3, 10.0f / 6, 10.0f};
    for (auto fontSize : testFontSize) {
        ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());
        text_style.setFontSize(fontSize);
        builder.pushStyle(text_style);
        builder.addText(text, len);
        builder.pop();

        auto paragraph = builder.Build();
        paragraph->layout(SK_ScalarInfinity);
        // Slightly wider than the max intrinsic width.
        REPORTER_ASSERT(reporter, paragraph->unresolvedGlyphs() == 0);
        paragraph->layout(paragraph->getMaxIntrinsicWidth());

        ParagraphImpl* impl = static_cast<ParagraphImpl*>(paragraph.get());

        REPORTER_ASSERT(reporter, impl->lines().size() == 1);
        auto& line = impl->lines()[0];

        const LineMetrics metrics = line.getMetrics();
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(metrics.fWidth, fontSize * len, EPSILON2));
    }
}

// A single cluster wider than the line also moves to the next line instead of joining the text
// before it; CoreText shapes a whole ZWJ emoji sequence into one such cluster
UNIX_ONLY_TEST(SkParagraph_TooWideClusterStartsNextLine, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    ParagraphStyle paragraphStyle;
    paragraphStyle.turnHintingOff();
    paragraphStyle.setApplyRoundingHack(false);
    ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());
    TextStyle textStyle;
    textStyle.setFontFamilies({SkString("Ahem")});
    textStyle.setFontSize(10);
    textStyle.setColor(SK_ColorBLACK);
    builder.pushStyle(textStyle);
    builder.addText("n");
    textStyle.setFontSize(30);
    builder.pushStyle(textStyle);
    builder.addText("\xE6\xBC\xA2");
    auto paragraph = builder.Build();
    paragraph->layout(20);
    auto lines = static_cast<ParagraphImpl*>(paragraph.get())->lines();
    REPORTER_ASSERT(reporter,
                    lines.size() == 2 && lines[1].text().start == 1,
                    "%zu lines, first ends at %zu",
                    lines.size(),
                    lines[0].text().end);
}

// lookAhead must group fWords + (fClusters + cluster) like maxIntrinsicWidth, or the float sums
// differ in the last bit and the last word wraps. Which size hits that depends on the font
// backend's advances: 14 + 0.1 does on macOS, 16.5 + 0.22 on Linux FreeType
UNIX_ONLY_TEST(SkParagraph_MaxIntrinsicWidthKeepsOneLine, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    struct Case {
        SkScalar fontSize;
        SkScalar letterSpacing;
    };
    for (Case c : {Case{14, 0.1f}, Case{16.5f, 0.22f}}) {
        skiatest::ReporterContext context(
                reporter, SkStringPrintf("size %g, spacing %g", c.fontSize, c.letterSpacing));
        ParagraphStyle paragraphStyle;
        paragraphStyle.turnHintingOff();
        paragraphStyle.setApplyRoundingHack(false);
        ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());
        TextStyle textStyle;
        textStyle.setFontFamilies({SkString("Ahem")});
        textStyle.setFontSize(c.fontSize);
        textStyle.setLetterSpacing(c.letterSpacing);
        textStyle.setColor(SK_ColorBLACK);
        builder.pushStyle(textStyle);
        builder.addText("View lic");
        textStyle.setColor(SK_ColorRED);
        builder.pushStyle(textStyle);
        builder.addText("enses");
        auto paragraph = builder.Build();
        paragraph->layout(1000);
        SkScalar maxIntrinsicWidth = paragraph->getMaxIntrinsicWidth();
        paragraph->layout(maxIntrinsicWidth);
        REPORTER_ASSERT(reporter,
                        paragraph->lineNumber() == 1,
                        "%zu lines at width %.9g",
                        paragraph->lineNumber(),
                        maxIntrinsicWidth);
    }
}

// Checked: DIFF+
UNIX_ONLY_TEST(SkParagraph_NewlineParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    TestCanvas canvas("SkParagraph_NewlineParagraph.png");
    const char* text =
            "line1\nline2 test1 test2 test3 test4 test5 test6 test7\nline3\n\nline4 "
            "test1 test2 test3 test4";
    const size_t len = strlen(text);

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setColor(SK_ColorRED);
    text_style.setFontSize(60);
    text_style.setColor(SK_ColorBLACK);
    text_style.setHeight(1);
    builder.pushStyle(text_style);
    builder.addText(text, len);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth - 300);
    paragraph->paint(canvas.get(), 0, 0);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    // Minikin does not count empty lines but SkParagraph does
    REPORTER_ASSERT(reporter, impl->lines().size() == 7);

    if (impl->lines().size() >= 7) {
        REPORTER_ASSERT(reporter, impl->lines()[0].offset().fY == 0);
        REPORTER_ASSERT(reporter, impl->lines()[1].offset().fY == 70);
        REPORTER_ASSERT(reporter, impl->lines()[2].offset().fY == 140);
        REPORTER_ASSERT(reporter, impl->lines()[3].offset().fY == 210);
        REPORTER_ASSERT(reporter, impl->lines()[4].offset().fY == 280);  // Empty line
        REPORTER_ASSERT(reporter, impl->lines()[5].offset().fY == 350);
        REPORTER_ASSERT(reporter, impl->lines()[6].offset().fY == 420);
    }
}

UNIX_ONLY_TEST(SkParagraph_ParagraphWithLineBreak, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    fontCollection->setDefaultFontManager(ToolUtils::TestFontMgr());
    fontCollection->enableFontFallback();

    TestCanvas canvas("SkParagraph_ParagraphWithLineBreak.png");

    ParagraphStyle paragraph_style;
    TextStyle text_style;
    text_style.setFontSize(16);
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setColor(SK_ColorBLACK);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());
    builder.addText("abc\n\ndef");
    text_style.setColor(SK_ColorBLACK);

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);
    paragraph->paint(canvas.get(), 0, 0);

    // Select a position at the second (empty) line
    auto pos = paragraph->getGlyphPositionAtCoordinate(0, 21);
    REPORTER_ASSERT(reporter, pos.affinity == Affinity::kDownstream && pos.position == 4);
    auto rect = paragraph->getRectsForRange(4, 5, RectHeightStyle::kTight, RectWidthStyle::kTight);
    REPORTER_ASSERT(reporter, rect.size() == 1 && rect[0].rect.width() == 0);
}

// This test does not produce an image
UNIX_ONLY_TEST(SkParagraph_NewlineOnly, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    fontCollection->setDefaultFontManager(ToolUtils::TestFontMgr());

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Ahem")});
    text_style.setColor(SK_ColorBLACK);
    StrutStyle strut_style;
    strut_style.setStrutEnabled(false);
    ParagraphStyle paragraph_style;
    paragraph_style.setStrutStyle(strut_style);
    paragraph_style.setTextStyle(text_style);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());
    builder.addText("\n");
    auto paragraph = builder.Build();
    paragraph->layout(1000);
    REPORTER_ASSERT(reporter, paragraph->getHeight() == 28);
}

UNIX_ONLY_TEST(SkParagraph_GlyphPositionsInEmptyLines, reporter) {

    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    TestCanvas canvas("SkParagraph_GlyphPositionsInEmptyLines.png");
    ParagraphStyle paragraph_style;
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());
    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setFontSize(20);
    text_style.setColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText("A\n\n");
    builder.pop();
    auto paragraph = builder.Build();
    paragraph->layout(300);
    paragraph->paint(canvas.get(), 0, 0);

    auto res1 = paragraph->getGlyphPositionAtCoordinate(paragraph->getMinIntrinsicWidth(), 1);
    REPORTER_ASSERT(reporter, res1.position == 1 && res1.affinity == Affinity::kUpstream);

    auto res2 = paragraph->getGlyphPositionAtCoordinate(0, paragraph->getHeight() * 0.5);
    REPORTER_ASSERT(reporter, res2.position == 2 && res2.affinity == Affinity::kDownstream);

    auto res3 = paragraph->getGlyphPositionAtCoordinate(0, paragraph->getHeight() - 1);
    REPORTER_ASSERT(reporter, res3.position == 3 && res3.affinity == Affinity::kDownstream);
}

UNIX_ONLY_TEST(SkParagraph_RTLGlyphPositionsInEmptyLines, reporter) {

    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    TestCanvas canvas("SkParagraph_RTLGlyphPositionsInEmptyLines.png");

    ParagraphStyle paragraph_style;
    paragraph_style.setTextDirection(TextDirection::kRtl);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());
    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setFontSize(20);
    text_style.setColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    //builder.addText("בבבב\n\nאאאא");
    builder.addText("בבבב\n\nאאאא");
    builder.pop();
    auto paragraph = builder.Build();
    paragraph->layout(500);
    paragraph->paint(canvas.get(), 0, 0);

    auto height = paragraph->getHeight();
    auto res1 = paragraph->getGlyphPositionAtCoordinate(0, 0);
    REPORTER_ASSERT(reporter, res1.position == 4 && res1.affinity == Affinity::kUpstream);
    auto res2 = paragraph->getGlyphPositionAtCoordinate(0, height / 2);
    REPORTER_ASSERT(reporter, res2.position == 5 && res2.affinity == Affinity::kDownstream);
    auto res3 = paragraph->getGlyphPositionAtCoordinate(0, height);
    REPORTER_ASSERT(reporter, res3.position == 10 && res3.affinity == Affinity::kUpstream);
}

UNIX_ONLY_TEST(SkParagraph_LTRLineMetricsDoesNotIncludeNewLine, reporter) {

    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    TestCanvas canvas("SkParagraph_LTRLineMetricsDoesNotIncludeNewLine.png");

    ParagraphStyle paragraph_style;
    paragraph_style.setTextDirection(TextDirection::kRtl);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());
    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setFontSize(20);
    text_style.setColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText("one two\n\nthree four\nwith spaces     \n    \n______________________");
    builder.pop();
    auto paragraph = builder.Build();
    paragraph->layout(190);
    paragraph->paint(canvas.get(), 0, 0);

    std::vector<std::tuple<size_t, size_t, size_t, size_t>> expected = {
            {0, 7, 7, 8},      // one two\n
            {8, 8, 8, 9},      // \n
            {9, 19, 19, 20},   // three four\n
            {20, 31, 36, 37},  // with spaces    \n
            {37, 37, 41, 42},  //      { just spaces }\n
            {42, 63, 63, 63},  // _____________________
            {63, 64, 64, 64},  // _
    };
    std::vector<LineMetrics> metrics;
    paragraph->getLineMetrics(metrics);
    for (auto& metric : metrics) {
        auto result = expected[metric.fLineNumber];
        REPORTER_ASSERT(reporter, metric.fStartIndex == std::get<0>(result));
        REPORTER_ASSERT(reporter, metric.fEndExcludingWhitespaces == std::get<1>(result));
        REPORTER_ASSERT(reporter, metric.fEndIndex == std::get<2>(result));
        REPORTER_ASSERT(reporter, metric.fEndIncludingNewline == std::get<3>(result));
    }
}

UNIX_ONLY_TEST(SkParagraph_RTLLineMetricsDoesNotIncludeNewLine, reporter) {

    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    TestCanvas canvas("SkParagraph_RTLLineMetricsDoesNotIncludeNewLine.png");
    canvas.get()->translate(100, 100);

    ParagraphStyle paragraph_style;
    paragraph_style.setTextDirection(TextDirection::kRtl);
    paragraph_style.setTextAlign(TextAlign::kRight);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());
    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setFontSize(20);
    text_style.setColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(
            mirror("______________________\none two\n\nthree four\nwith spaces     \n    "));
    builder.pop();
    auto paragraph = builder.Build();
    paragraph->layout(190);
    paragraph->paint(canvas.get(), 0, 0);
    //auto impl = static_cast<ParagraphImpl*>(paragraph.get());

    SkPaint gray;
    gray.setColor(SK_ColorGRAY);
    gray.setStyle(SkPaint::kStroke_Style);
    gray.setAntiAlias(true);
    gray.setStrokeWidth(1);
    canvas.get()->drawRect(SkRect::MakeXYWH(0, 0, paragraph->getMaxWidth(), paragraph->getHeight()),
                           gray);

    SkPaint red;
    red.setColor(SK_ColorRED);
    red.setStyle(SkPaint::kStroke_Style);
    red.setAntiAlias(true);
    red.setStrokeWidth(1);

    SkPaint blue;
    blue.setColor(SK_ColorRED);
    blue.setStyle(SkPaint::kStroke_Style);
    blue.setAntiAlias(true);
    blue.setStrokeWidth(1);

    auto boxes =
            paragraph->getRectsForRange(0, 100, RectHeightStyle::kTight, RectWidthStyle::kTight);
    bool even = false;
    for (auto& box : boxes) {
        canvas.get()->drawRect(box.rect, even ? red : blue);
        even = !even;
    }

    // RTL codepoint u"\u202E" messes everything up
    // (adds one invisible codepoint to the first line
    // and shift all the indexes by 1 right)
    std::vector<std::tuple<int, int, int, int>> expected = {
            {0,
             1,
             5,
             6},  //      { just spaces; the end of the text considered as a new line in libtxt?!? }
            {6, 22, 22, 23},   // with spaces    \n
            {23, 33, 33, 34},  // three four\n
            {34, 34, 34, 35},  // \n
            {35, 42, 42, 43},  // one two\n
            {43, 64, 64, 64},  // _____________________
            {64, 65, 65, 65}   // _
    };

    std::vector<LineMetrics> metrics;
    paragraph->getLineMetrics(metrics);
    for (auto& metric : metrics) {
        auto result = expected[metric.fLineNumber];
        REPORTER_ASSERT(reporter, metric.fStartIndex == SkToU32(std::get<0>(result)));
        REPORTER_ASSERT(reporter, metric.fEndExcludingWhitespaces == SkToU32(std::get<1>(result)));
        REPORTER_ASSERT(reporter, metric.fEndIndex == SkToU32(std::get<2>(result)));
        REPORTER_ASSERT(reporter, metric.fEndIncludingNewline == SkToU32(std::get<3>(result)));
    }
}

UNIX_ONLY_TEST(SkParagraph_LineEnd, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    TestCanvas canvas("SkParagraph_LineEnd.png");
    canvas.get()->translate(100, 100);

    TextStyle text_style;
    text_style.setColor(SK_ColorBLACK);
    text_style.setFontFamilies({SkString("Ahem")});
    text_style.setFontSize(10.0f);
    ParagraphStyle paragraph_style;
    paragraph_style.setTextStyle(text_style);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());
    builder.pushStyle(text_style);
    builder.addText("Hello ");
    builder.addText("hello   ");
    builder.addText("hello\n");
    builder.addText("hello   \n");
    builder.addText("world");

    auto paragraph = builder.Build();
    paragraph->layout(60.0f);
    paragraph->paint(canvas.get(), 0, 0);

    std::vector<LineMetrics> lm;
    paragraph->getLineMetrics(lm);
    REPORTER_ASSERT(reporter, lm.size() >= 4);
    if (lm.size() >= 4) {
        REPORTER_ASSERT(reporter,
                        lm[0].fEndExcludingWhitespaces == 05 && lm[0].fEndIndex == 06 &&
                                lm[0].fEndIncludingNewline == 06);
        REPORTER_ASSERT(reporter,
                        lm[1].fEndExcludingWhitespaces == 11 && lm[1].fEndIndex == 14 &&
                                lm[1].fEndIncludingNewline == 14);
        REPORTER_ASSERT(reporter,
                        lm[2].fEndExcludingWhitespaces == 19 && lm[2].fEndIndex == 19 &&
                                lm[2].fEndIncludingNewline == 20);
        REPORTER_ASSERT(reporter,
                        lm[3].fEndExcludingWhitespaces == 25 && lm[3].fEndIndex == 28 &&
                                lm[3].fEndIncludingNewline == 29);
    }
}

UNIX_ONLY_TEST(SkParagraph_TabSubstitution, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>(true);
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    TestCanvas canvas("SkParagraph_TabSubstitution.png");

    ParagraphStyle paragraph_style;
    paragraph_style.setReplaceTabCharacters(true);

    TextStyle text_style;
    text_style.setColor(SK_ColorBLACK);
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setFontSize(100);

    ParagraphBuilderImpl builder1(paragraph_style, fontCollection, get_unicode());
    builder1.pushStyle(text_style);
    builder1.addText("There is a tab>\t<right here");
    auto paragraph1 = builder1.Build();
    paragraph1->layout(TestCanvasWidth);
    paragraph1->paint(canvas.get(), 0, 0);

    paragraph_style.setReplaceTabCharacters(false);
    ParagraphBuilderImpl builder2(paragraph_style, fontCollection, get_unicode());
    builder2.pushStyle(text_style);
    builder2.addText("There is a tab>\t<right here");
    auto paragraph2 = builder2.Build();
    paragraph2->layout(TestCanvasWidth);
    paragraph2->paint(canvas.get(), 0, 0);

    // Second paragraph has an unresolved \t (glyph == 0)
    REPORTER_ASSERT(reporter, ((ParagraphImpl*)paragraph1.get())->runs()[0].glyphs()[15] != 0);
    REPORTER_ASSERT(reporter, ((ParagraphImpl*)paragraph2.get())->runs()[0].glyphs()[15] == 0);
    // Notice, that the cache didn't work for the second paragraph - as it should not
    REPORTER_ASSERT(reporter, 2 == fontCollection->getParagraphCache()->count());
}

UNIX_ONLY_TEST(SkParagraph_EndWithLineSeparator, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    const char* text = "A text ending with line separator.\u2028";
    const size_t len = strlen(text);

    ParagraphStyle paragraph_style;

    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());
    TextStyle textStyle;
    textStyle.setFontFamilies({SkString("Roboto")});
    builder.pushStyle(textStyle);
    builder.addText(text, len);

    auto paragraph = builder.Build();
    paragraph->layout(SK_ScalarMax);

    int visitedCount = 0;
    paragraph->visit([&visitedCount, reporter](int lineNumber, const Paragraph::VisitorInfo* info) {
        visitedCount++;
        if (lineNumber == 1) {
            // Visitor for second line created from line separator should only be called
            // for 'end of line', denoted by 'info' being nullptr
            REPORTER_ASSERT(reporter, info == nullptr);
        }
    });
    REPORTER_ASSERT(reporter, visitedCount == 3, "visitedCount: %d", visitedCount);
}

// A line-final hard break has no visible width, so in an RTL paragraph (where its glyph is the
// visual left end of the last run) boxes and hit-test positions must match the text without it
DEF_TEST(SkParagraph_RTLHardBreakHasNoWidth, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    auto build = [&](const std::string& text, TextDirection dir) {
        ParagraphStyle paragraphStyle;
        paragraphStyle.setTextDirection(dir);
        TextStyle textStyle;
        textStyle.setFontFamilies({SkString("Ahem")});
        textStyle.setFontSize(14);
        paragraphStyle.setTextStyle(textStyle);
        ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());
        builder.addText(text.c_str(), text.size());
        auto paragraph = builder.Build();
        paragraph->layout(300);
        return paragraph;
    };
    auto sameBoxes = [&](const char* label,
                         Paragraph* expected,
                         Paragraph* actual,
                         unsigned start,
                         unsigned end) {
        auto e = expected->getRectsForRange(
                start, end, RectHeightStyle::kTight, RectWidthStyle::kTight);
        auto a = actual->getRectsForRange(
                start, end, RectHeightStyle::kTight, RectWidthStyle::kTight);
        REPORTER_ASSERT(reporter,
                        !e.empty() && e.size() == a.size(),
                        "%s [%u:%u): %zu != %zu",
                        label,
                        start,
                        end,
                        e.size(),
                        a.size());
        for (size_t i = 0; i < std::min(e.size(), a.size()); ++i) {
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(e[i].rect.fLeft, a[i].rect.fLeft) &&
                                    SkScalarNearlyEqual(e[i].rect.fRight, a[i].rect.fRight),
                            "%s [%u:%u) #%zu: [%g:%g) != [%g:%g)",
                            label,
                            start,
                            end,
                            i,
                            e[i].rect.fLeft,
                            e[i].rect.fRight,
                            a[i].rect.fLeft,
                            a[i].rect.fRight);
        }
    };

    // Mixed BiDi: an LTR run followed by an RTL run that owns the hard break. In CRLF only LF is
    // the hard break; CR stays trailing whitespace, as it already is in LTR
    struct Case {
        const char* label;
        const char* text;
        const char* hardBreak;
    };
    for (auto [label, text, hardBreak] : {Case{"LF", "aaa bbb.", "\n"},
                                          Case{"CRLF", "aaa bbb.\r", "\n"},
                                          Case{"PS", "aaa bbb.", "\xE2\x80\xA9"}}) {
        auto mixed = build(text, TextDirection::kRtl);
        auto broken = build(std::string(text) + hardBreak, TextDirection::kRtl);
        sameBoxes(label, mixed.get(), broken.get(), 0, 3);
        sameBoxes(label, mixed.get(), broken.get(), 4, 7);
        sameBoxes(label, mixed.get(), broken.get(), 7, 8);
        sameBoxes(label, mixed.get(), broken.get(), 0, 8);

        // dx left of the line maps to the logical end of the line, before the hard break
        for (SkScalar dx :
             {0.0f, 100.0f, 190.0f, 195.0f, 201.0f, 203.0f, 230.0f, 260.0f, 299.0f, 310.0f}) {
            auto e = mixed->getGlyphPositionAtCoordinate(dx, 7);
            auto a = broken->getGlyphPositionAtCoordinate(dx, 7);
            REPORTER_ASSERT(reporter,
                            e.position == a.position && e.affinity == a.affinity,
                            "%s @%g: %d%c != %d%c",
                            label,
                            dx,
                            e.position,
                            e.affinity == kUpstream ? 'U' : 'D',
                            a.position,
                            a.affinity == kUpstream ? 'U' : 'D');
        }
    }

    // Pure RTL: the hard break shares the run with the text
    auto hebrew = build("\u05D0\u05D1\u05D2", TextDirection::kRtl);
    auto hebrewBroken = build("\u05D0\u05D1\u05D2\n", TextDirection::kRtl);
    sameBoxes("hebrew", hebrew.get(), hebrewBroken.get(), 0, 3);
    sameBoxes("hebrew", hebrew.get(), hebrewBroken.get(), 1, 2);

    // LTR keeps excluding the hard break at the visual right end
    auto ltr = build("aaa bbb.", TextDirection::kLtr);
    auto ltrBroken = build("aaa bbb.\n", TextDirection::kLtr);
    sameBoxes("ltr", ltr.get(), ltrBroken.get(), 0, 8);
}
