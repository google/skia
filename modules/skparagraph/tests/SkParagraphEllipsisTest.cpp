/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkBitmap.h"
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
#include <limits>
#include <string>
#include <vector>

using namespace skia::textlayout;
using namespace skia::textlayout::test;

// Checked: NO DIFF
UNIX_ONLY_TEST(SkParagraph_Ellipsize, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    TestCanvas canvas("SkParagraph_Ellipsize.png");
    const char* text =
            "This is a very long sentence to test if the text will properly wrap "
            "around and go to the next line. Sometimes, short sentence. Longer "
            "sentences are okay too because they are nessecary. Very short. ";
    const size_t len = strlen(text);

    ParagraphStyle paragraph_style;
    paragraph_style.setMaxLines(1);
    std::u16string ellipsis = u"\u2026";
    paragraph_style.setEllipsis(ellipsis);
    std::u16string e = paragraph_style.getEllipsisUtf16();
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
    paragraph->paint(canvas.get(), 0, 0);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());

    // Check that the ellipsizer limited the text to one line and did not wrap to a second line.
    REPORTER_ASSERT(reporter, impl->lines().size() == 1);
    if (impl->lines().size() >= 1) {
        auto& line = impl->lines()[0];
        REPORTER_ASSERT(reporter, line.ellipsis() != nullptr);
        REPORTER_ASSERT(reporter, impl->runs().size() == 1);
    }
}

// A layout at another width reuses the shaped ellipsis of the run and style block, and gets
// the same result as a new paragraph. A new shaping of the paragraph shapes it again.
UNIX_ONLY_TEST(SkParagraph_EllipsisReusedAtOtherWidths, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    fontCollection->disableFontFallback();

    ParagraphStyle paragraphStyle;
    paragraphStyle.setMaxLines(1);
    paragraphStyle.setEllipsis(u"…");
    paragraphStyle.turnHintingOff();
    TextStyle first;
    first.setFontFamilies({SkString("Roboto")});
    first.setFontSize(14);
    const char* firstText = "Short text in a small font. ";
    const char* secondText = "More text in a large font that does not fit on the line.";
    SkScalar secondFontSize = 28;
    auto build = [&]() {
        ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());
        builder.pushStyle(first);
        builder.addText(firstText);
        builder.pop();
        TextStyle second = first;
        second.setFontSize(secondFontSize);
        builder.pushStyle(second);
        builder.addText(secondText);
        builder.pop();
        return builder.Build();
    };
    auto ellipsisOf = [](Paragraph* paragraph) {
        auto impl = static_cast<ParagraphImpl*>(paragraph);
        return impl->lines().size() == 1 ? impl->lines()[0].ellipsis() : nullptr;
    };
    auto sameAsNewParagraph = [&](Paragraph* paragraph, SkScalar width) {
        auto fresh = build();
        fresh->layout(width);
        const Run* a = ellipsisOf(paragraph);
        const Run* b = ellipsisOf(fresh.get());
        if (!a || !b) {
            return false;
        }
        auto ga = a->glyphs();
        auto gb = b->glyphs();
        return a->advance() == b->advance() && a->font() == b->font() &&
               std::equal(ga.begin(), ga.end(), gb.begin(), gb.end()) &&
               paragraph->getLongestLine() == fresh->getLongestLine();
    };

    auto paragraph = build();
    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    // The line ends in the small text.
    paragraph->layout(100);
    REPORTER_ASSERT(reporter, impl->shapedEllipsisCount() == 1);
    REPORTER_ASSERT(reporter, sameAsNewParagraph(paragraph.get(), 100));
    const SkScalar smallAdvance = ellipsisOf(paragraph.get())->advance().fX;
    paragraph->layout(110);
    REPORTER_ASSERT(reporter, impl->shapedEllipsisCount() == 1);
    REPORTER_ASSERT(reporter, sameAsNewParagraph(paragraph.get(), 110));

    // The line ends in the large text, which has its own run and style block.
    paragraph->layout(400);
    REPORTER_ASSERT(reporter, impl->shapedEllipsisCount() == 2);
    REPORTER_ASSERT(reporter, sameAsNewParagraph(paragraph.get(), 400));
    REPORTER_ASSERT(reporter, ellipsisOf(paragraph.get())->advance().fX > smallAdvance);
    paragraph->layout(100);
    REPORTER_ASSERT(reporter, impl->shapedEllipsisCount() == 2);
    REPORTER_ASSERT(reporter, sameAsNewParagraph(paragraph.get(), 100));

    // A change of the font size shapes the paragraph again, and the old ellipses are gone.
    paragraph->updateFontSize(0, impl->text().size(), 20);
    paragraph->layout(100);
    REPORTER_ASSERT(reporter, impl->shapedEllipsisCount() == 1);
    REPORTER_ASSERT(reporter, ellipsisOf(paragraph.get())->advance().fX > smallAdvance);

    // Blocks whose font sizes are nearly equal share one run, but each ellipsis uses the exact
    // size of its block.
    secondFontSize = 14 + 0.5f * SK_ScalarNearlyZero;
    paragraph = build();
    impl = static_cast<ParagraphImpl*>(paragraph.get());
    paragraph->layout(100);
    REPORTER_ASSERT(reporter, impl->runs().size() == 1);
    REPORTER_ASSERT(reporter, impl->shapedEllipsisCount() == 1);
    REPORTER_ASSERT(reporter, sameAsNewParagraph(paragraph.get(), 100));
    paragraph->layout(300);
    REPORTER_ASSERT(reporter, impl->shapedEllipsisCount() == 2);
    REPORTER_ASSERT(reporter, sameAsNewParagraph(paragraph.get(), 300));
    paragraph->layout(110);
    REPORTER_ASSERT(reporter, impl->shapedEllipsisCount() == 2);
    REPORTER_ASSERT(reporter, sameAsNewParagraph(paragraph.get(), 110));
}

UNIX_ONLY_TEST(SkParagraph_Ellipsis, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    fontCollection->setDefaultFontManager(ToolUtils::TestFontMgr());
    TestCanvas canvas("SkParagraph_Ellipsis.png");

    const char* text =
            "This\n"
            "is a wrapping test. It should wrap at manual newlines, and if softWrap is true, also "
            "at spaces.";
    TextStyle text_style;
    text_style.setFontFamilies({SkString("Ahem")});
    text_style.setColor(SK_ColorBLACK);
    text_style.setFontSize(10);

    auto relayout = [&](size_t lines,
                        bool ellipsis,
                        SkScalar width,
                        SkScalar height,
                        SkScalar minWidth,
                        SkScalar maxWidth,
                        SkColor bg) {
        ParagraphStyle paragraph_style;
        SkPaint paint;
        paint.setColor(bg);
        text_style.setForegroundColor(paint);
        paragraph_style.setTextStyle(text_style);
        paragraph_style.setMaxLines(lines);
        if (ellipsis) {
            paragraph_style.setEllipsis(u"\u2026");
        }
        ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());
        builder.addText(text);
        auto paragraph = builder.Build();
        paragraph->layout(50);
        paragraph->paint(canvas.get(), 0, 0);
        canvas.get()->translate(50, paragraph->getHeight() + 10);
        auto result = paragraph->getRectsForRange(
                0, strlen(text), RectHeightStyle::kTight, RectWidthStyle::kTight);
        SkPaint background;
        background.setColor(SK_ColorRED);
        background.setStyle(SkPaint::kStroke_Style);
        background.setAntiAlias(true);
        background.setStrokeWidth(1);
        canvas.get()->drawRect(result.front().rect, background);

        SkASSERT(width == paragraph->getMaxWidth());
        SkASSERT(height == paragraph->getHeight());
        SkASSERT(nearlyEqual(minWidth, paragraph->getMinIntrinsicWidth(), EPSILON100));
        SkASSERT(nearlyEqual(maxWidth, paragraph->getMaxIntrinsicWidth(), EPSILON100));
    };

    SkPaint paint;
    paint.setColor(SK_ColorLTGRAY);
    canvas.get()->drawRect(SkRect::MakeXYWH(0, 0, 50, 500), paint);

    relayout(1, false, 50, 10, 950, 950, SK_ColorRED);
    relayout(3, false, 50, 30, 90, 950, SK_ColorBLUE);
    relayout(std::numeric_limits<size_t>::max(), false, 50, 200, 90, 950, SK_ColorGREEN);

    relayout(1, true, 50, 10, 950, 950, SK_ColorYELLOW);
    relayout(3, true, 50, 30, 90, 950, SK_ColorMAGENTA);
    relayout(std::numeric_limits<size_t>::max(), true, 50, 20, 950, 950, SK_ColorCYAN);

    relayout(1, false, 50, 10, 950, 950, SK_ColorRED);
    relayout(3, false, 50, 30, 90, 950, SK_ColorBLUE);
    relayout(std::numeric_limits<size_t>::max(), false, 50, 200, 90, 950, SK_ColorGREEN);
}

UNIX_ONLY_TEST(SkParagraph_EllipsisGetRectForRange, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    TestCanvas canvas("SkParagraph_EllipsisGetRectForRange.png");
    const char* text =
            "This is a very long sentence to test if the text will properly wrap "
            "around and go to the next line. Sometimes, short sentence. Longer "
            "sentences are okay too because they are nessecary. Very short. ";
    const size_t len = strlen(text);

    ParagraphStyle paragraph_style;
    paragraph_style.setMaxLines(1);
    std::u16string ellipsis = u"\u2026";
    paragraph_style.setEllipsis(ellipsis);
    std::u16string e = paragraph_style.getEllipsisUtf16();
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
    paragraph->paint(canvas.get(), 0, 0);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());

    // Check that the ellipsizer limited the text to one line and did not wrap to a second line.
    REPORTER_ASSERT(reporter, impl->lines().size() == 1);

    auto boxes1 = impl->getRectsForRange(0, 2, RectHeightStyle::kTight, RectWidthStyle::kTight);
    REPORTER_ASSERT(reporter, boxes1.size() == 1);

    auto boxes2 = impl->getRectsForRange(0, 3, RectHeightStyle::kTight, RectWidthStyle::kTight);
    REPORTER_ASSERT(reporter, boxes2.size() == 1);

    canvas.drawRects(SK_ColorRED, boxes1);
    canvas.drawRects(SK_ColorRED, boxes2);
}

UNIX_ONLY_TEST(SkParagraph_lineMetricsWithEllipsis, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    fontCollection->setDefaultFontManager(ToolUtils::TestFontMgr());
    fontCollection->enableFontFallback();

    ParagraphStyle paragraph_style;
    paragraph_style.setMaxLines(1);
    std::u16string ellipsis = u"\u2026";
    paragraph_style.setEllipsis(ellipsis);

    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());
    builder.addText("hello");

    auto paragraph = builder.Build();
    paragraph->layout(1.);

    std::vector<LineMetrics> lm;
    paragraph->getLineMetrics(lm);
    REPORTER_ASSERT(reporter, lm.size() == 1);
}

// RTL Ellipsis
UNIX_ONLY_TEST(SkParagraph_RtlEllipsis1, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>(true);
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    TestCanvas canvas("SkParagraph_RtlEllipsis1.png");

    canvas.get()->drawColor(SK_ColorWHITE);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Noto Naskh Arabic"), SkString("Roboto")});
    text_style.setFontSize(100);
    text_style.setColor(SK_ColorBLACK);
    ParagraphStyle paragraph_style;
    paragraph_style.setTextStyle(text_style);
    paragraph_style.setTextDirection(TextDirection::kRtl);
    paragraph_style.setEllipsis(u"\u2026");
    paragraph_style.setTextAlign(TextAlign::kStart);
    paragraph_style.setMaxLines(1);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());
    builder.pushStyle(text_style);
    builder.addText(u"1  2  3  4  5  6  7  8  9");
    auto paragraph = builder.Build();
    paragraph->layout(474);
    paragraph->paint(canvas.get(), 0, 0);
    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, paragraph->lineNumber() == 1);
    auto& line = impl->lines()[0];
    bool first = true;
    line.iterateThroughVisualRuns(true,
                                  [&](const Run* run,
                                      SkScalar runOffsetInLine,
                                      TextRange textRange,
                                      SkScalar* runWidthInLine) {
                                      REPORTER_ASSERT(reporter, first == (run->isEllipsis()));
                                      first = false;
                                      return true;
                                  });
}

UNIX_ONLY_TEST(SkParagraph_RtlEllipsis2, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>(true);
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    TestCanvas canvas("SkParagraph_RtlEllipsis2.png");

    canvas.get()->drawColor(SK_ColorWHITE);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Noto Naskh Arabic"), SkString("Roboto")});
    text_style.setFontSize(100);
    text_style.setColor(SK_ColorBLACK);
    ParagraphStyle paragraph_style;
    paragraph_style.setTextStyle(text_style);
    paragraph_style.setTextDirection(TextDirection::kRtl);
    paragraph_style.setEllipsis(u"\u2026");
    paragraph_style.setTextAlign(TextAlign::kStart);
    paragraph_style.setMaxLines(2);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());
    builder.pushStyle(text_style);
    builder.addText(u"تظاهرات و تجمعات اعتراضی در سراسر کشور ۲۳ مهر");
    auto paragraph = builder.Build();
    paragraph->layout(474);
    paragraph->paint(canvas.get(), 0, 0);
    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, paragraph->lineNumber() == 2);
    auto& line = impl->lines()[1];
    bool first = true;
    line.iterateThroughVisualRuns(true,
                                  [&](const Run* run,
                                      SkScalar runOffsetInLine,
                                      TextRange textRange,
                                      SkScalar* runWidthInLine) {
                                      REPORTER_ASSERT(reporter, first == (run->isEllipsis()));
                                      first = false;
                                      return true;
                                  });
}

UNIX_ONLY_TEST(SkParagraph_getLineNumberAt_Ellipsis, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    fontCollection->setDefaultFontManager(ToolUtils::TestFontMgr());
    TestCanvas canvas("SkParagraph_Ellipsis.png");

    // The second line will be ellipsized. The 10th glyph ("0") will be replaced
    // by U+2026.
    const char* text =
            "This\n"           // [0, 5)
            "1234567890ABCD";  // [5, len)

    const size_t len = strlen(text);

    ParagraphStyle paragraph_style;
    paragraph_style.setEllipsis(u"\u2026");
    paragraph_style.setMaxLines(2);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Ahem")});
    text_style.setColor(SK_ColorBLACK);
    text_style.setFontSize(10);

    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());
    builder.pushStyle(text_style);
    builder.addText(text, len);
    builder.pop();
    auto paragraph = builder.Build();

    // Roughly 10 characters wide.
    paragraph->layout(100);

    REPORTER_ASSERT(reporter, paragraph->getLineNumberAt(0) == 0);
    REPORTER_ASSERT(reporter, paragraph->getLineNumberAt(4) == 0);
    REPORTER_ASSERT(reporter, paragraph->getLineNumberAt(5) == 1);
    REPORTER_ASSERT(reporter, paragraph->getLineNumberAt(len) == -1);
    REPORTER_ASSERT(reporter, paragraph->getLineNumberAt(len - 1) == -1);
    // "0" should be ellipsized away so the call return -1 instead of 1.
    REPORTER_ASSERT(reporter, paragraph->getLineNumberAt(14) == -1);
}

// Custom ellipsis strings must be shaped in the surrounding bidi context. With
// a hardcoded LTR level, the trailing "!" of an Arabic custom ellipsis resolves
// LTR and splits the ellipsis into two bidi runs, which shapeEllipsis doesn't
// support (it expects a single run).
UNIX_ONLY_TEST(SkParagraph_EllipsisRTLCustomStringBidiLevel, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>(true);
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Noto Naskh Arabic"), SkString("Roboto")});
    text_style.setFontSize(40);
    text_style.setColor(SK_ColorBLACK);

    ParagraphStyle paragraph_style;
    paragraph_style.setTextStyle(text_style);
    paragraph_style.setTextDirection(TextDirection::kRtl);
    // Arabic word "النهاية" (nihaya = "the end") plus "!", used as a custom ellipsis.
    paragraph_style.setEllipsis(u"\u0627\u0644\u0646\u0647\u0627\u064A\u0629!");
    paragraph_style.setMaxLines(1);

    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());
    builder.pushStyle(text_style);
    builder.addText(
            u"\u062A\u0638\u0627\u0647\u0631\u0627\u062A \u0648 "
            u"\u062A\u062C\u0645\u0639\u0627\u062A");
    auto paragraph = builder.Build();
    // Layout width chosen so that (a) the paragraph text (about 270px wide)
    // exceeds it and truncation is forced, and (b) the custom Arabic ellipsis
    // comfortably fits — otherwise shapeEllipsis falls back to "ellipsis did
    // not fit" and line.ellipsis() is nullptr, which would mask the regression
    // being tested.
    paragraph->layout(200);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, paragraph->lineNumber() == 1);
    auto& line = impl->lines()[0];
    REPORTER_ASSERT(reporter, line.ellipsis() != nullptr);
    if (line.ellipsis() == nullptr)
        return;
    // Surrounded by RTL Arabic content; the ellipsis must be shaped as RTL.
    REPORTER_ASSERT(reporter, !line.ellipsis()->leftToRight());
    REPORTER_ASSERT(reporter, (line.ellipsis()->bidiLevel() % 2) == 1);
    REPORTER_ASSERT(reporter, line.ellipsis()->advance().fX > 0);
}

// Regression guard for the common path: default "…" (ON-class) ellipsis in an
// LTR paragraph must still produce an LTR ellipsis run.
UNIX_ONLY_TEST(SkParagraph_EllipsisLTRDefaultBidiLevel, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>(true);
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setFontSize(20);
    text_style.setColor(SK_ColorBLACK);

    ParagraphStyle paragraph_style;
    paragraph_style.setTextStyle(text_style);
    paragraph_style.setEllipsis(u"\u2026");
    paragraph_style.setMaxLines(1);

    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());
    builder.pushStyle(text_style);
    builder.addText("The quick brown fox jumps over the lazy dog");
    auto paragraph = builder.Build();
    paragraph->layout(100);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, paragraph->lineNumber() == 1);
    auto& line = impl->lines()[0];
    REPORTER_ASSERT(reporter, line.ellipsis() != nullptr);
    if (line.ellipsis() == nullptr)
        return;
    REPORTER_ASSERT(reporter, line.ellipsis()->leftToRight());
    REPORTER_ASSERT(reporter, (line.ellipsis()->bidiLevel() % 2) == 0);
    REPORTER_ASSERT(reporter, line.ellipsis()->advance().fX > 0);
}

// An LTR custom ellipsis that ends in a neutral character must stay a single
// run in RTL text. At the surrounding RTL level, the "…" in "Read more…" would
// resolve RTL and split the ellipsis into two bidi runs, so it's shaped LTR.
UNIX_ONLY_TEST(SkParagraph_EllipsisRTLLtrCustomStringSingleRun, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>(true);
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Noto Naskh Arabic"), SkString("Roboto")});
    text_style.setFontSize(40);
    text_style.setColor(SK_ColorBLACK);

    ParagraphStyle paragraph_style;
    paragraph_style.setTextStyle(text_style);
    paragraph_style.setTextDirection(TextDirection::kRtl);
    paragraph_style.setEllipsis(u"Read more\u2026");
    paragraph_style.setMaxLines(1);

    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());
    builder.pushStyle(text_style);
    builder.addText(
            u"\u062A\u0638\u0627\u0647\u0631\u0627\u062A \u0648 "
            u"\u062A\u062C\u0645\u0639\u0627\u062A "
            u"\u062A\u0638\u0627\u0647\u0631\u0627\u062A");
    auto paragraph = builder.Build();
    paragraph->layout(300);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, paragraph->lineNumber() == 1);
    auto& line = impl->lines()[0];
    REPORTER_ASSERT(reporter, line.ellipsis() != nullptr);
    if (line.ellipsis() == nullptr)
        return;
    REPORTER_ASSERT(reporter, line.ellipsis()->leftToRight());
    REPORTER_ASSERT(reporter, line.ellipsis()->advance().fX > 0);
}

// Likewise, an RTL custom ellipsis that ends in a neutral character must stay a
// single run in LTR text, so "النهاية!" is shaped RTL.
UNIX_ONLY_TEST(SkParagraph_EllipsisLTRRtlCustomStringSingleRun, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>(true);
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto"), SkString("Noto Naskh Arabic")});
    text_style.setFontSize(40);
    text_style.setColor(SK_ColorBLACK);

    ParagraphStyle paragraph_style;
    paragraph_style.setTextStyle(text_style);
    paragraph_style.setEllipsis(u"\u0627\u0644\u0646\u0647\u0627\u064A\u0629!");
    paragraph_style.setMaxLines(1);

    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());
    builder.pushStyle(text_style);
    builder.addText("The quick brown fox jumps over the lazy dog");
    auto paragraph = builder.Build();
    paragraph->layout(300);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, paragraph->lineNumber() == 1);
    auto& line = impl->lines()[0];
    REPORTER_ASSERT(reporter, line.ellipsis() != nullptr);
    if (line.ellipsis() == nullptr)
        return;
    REPORTER_ASSERT(reporter, !line.ellipsis()->leftToRight());
    REPORTER_ASSERT(reporter, line.ellipsis()->advance().fX > 0);
}

DEF_TEST(SkParagraph_EllipsisAfterHardLineBreak, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    auto build = [&](const char* text,
                     size_t maxLines,
                     const char* family = "Roboto",
                     TextDirection dir = TextDirection::kLtr,
                     SkScalar width = 1000) {
        ParagraphStyle paragraphStyle;
        paragraphStyle.turnHintingOff();
        if (maxLines > 0) {
            paragraphStyle.setMaxLines(maxLines);
        }
        paragraphStyle.setEllipsis(u"\u2026");
        paragraphStyle.setTextDirection(dir);
        TextStyle textStyle;
        textStyle.setFontFamilies({SkString(family)});
        textStyle.setFontSize(20);
        paragraphStyle.setTextStyle(textStyle);
        ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());
        builder.addText(text);
        auto paragraph = builder.Build();
        paragraph->layout(width);
        // Painting runs the debug checks of the line geometry
        SkCanvas canvas(SkIsFinite(width) ? SkScalarCeilToInt(width) : 1000, 1000);
        paragraph->paint(&canvas, 0, 0);
        return paragraph;
    };
    // Returns the index of the only line with the ellipsis, or -1 if there is none
    auto ellipsisLine = [&](Paragraph* paragraph, size_t expectedLines) {
        auto* impl = static_cast<ParagraphImpl*>(paragraph);
        REPORTER_ASSERT(reporter, impl->lines().size() == expectedLines);
        int found = -1;
        for (size_t i = 0; i < impl->lines().size(); ++i) {
            if (impl->lines()[i].ellipsis() != nullptr) {
                REPORTER_ASSERT(reporter, found == -1);
                found = SkToInt(i);
            }
        }
        return found;
    };
    auto check = [&](const char* text, size_t maxLines, size_t expectedLines) {
        return ellipsisLine(build(text, maxLines).get(), expectedLines);
    };

    // Nothing is hidden: a single final line break, or no line limit at all
    REPORTER_ASSERT(reporter, check("AAA\n", 1, 1) == -1);
    REPORTER_ASSERT(reporter, check("AAA\nBBB", 0, 2) == -1);
    REPORTER_ASSERT(reporter, check("AAA\nBBB", 2, 2) == -1);
    REPORTER_ASSERT(
            reporter,
            ellipsisLine(build("AAA\n", 1, "Roboto", TextDirection::kLtr, SK_ScalarInfinity).get(),
                         1) == -1);
#ifdef SK_IGNORE_ELLIPSIZE_TRUNCATED_LINES
    REPORTER_ASSERT(reporter, check("AAA\n\n", 1, 1) == -1);
    REPORTER_ASSERT(reporter, check("AAA\n   ", 1, 1) == -1);
    REPORTER_ASSERT(reporter, check("AAA\nBBB", 1, 1) == -1);
    REPORTER_ASSERT(reporter, check("AAA\n\nBBB", 2, 2) == -1);
    REPORTER_ASSERT(reporter, check("\n\n", 1, 1) == -1);
    REPORTER_ASSERT(
            reporter,
            ellipsisLine(
                    build("AAA\nBBB", 1, "Roboto", TextDirection::kLtr, SK_ScalarInfinity).get(),
                    1) == -1);
#else
    // Text after the last visible hard line break is hidden, even if it is blank
    REPORTER_ASSERT(reporter, check("AAA\n\n", 1, 1) == 0);
    REPORTER_ASSERT(reporter, check("AAA\n   ", 1, 1) == 0);
    REPORTER_ASSERT(reporter, check("AAA\nBBB", 1, 1) == 0);
    REPORTER_ASSERT(reporter, check("AAA\n\nBBB", 2, 2) == 1);
    REPORTER_ASSERT(reporter, check("\n\n", 1, 1) == 0);
    auto inf = build("AAA\nBBB", 1, "Roboto", TextDirection::kLtr, SK_ScalarInfinity);
    REPORTER_ASSERT(reporter, inf->didExceedMaxLines());
    REPORTER_ASSERT(reporter, ellipsisLine(inf.get(), 1) == 0);
    REPORTER_ASSERT(
            reporter,
            ellipsisLine(
                    build("\xD8\xA8\xD8\xAA\n\xD8\xA8", 1, "Katibeh", TextDirection::kRtl).get(),
                    1) == 0);

    // As on Android and the web, the spaces before the line break stay in front of the
    // ellipsis while they fit; the line break itself never stays
    auto plain = build("AAAAAA", 1);
    SkScalar plainWidth = static_cast<ParagraphImpl*>(plain.get())->lines()[0].width();
    auto unlimited = build("AAAAAA      \nB", 0);
    auto& unlimitedLine = static_cast<ParagraphImpl*>(unlimited.get())->lines()[0];
    SkScalar spacedWidth = unlimitedLine.width() + unlimitedLine.spacesWidth();
    SkScalar spaceWidth = (spacedWidth - plainWidth) / 6;
    REPORTER_ASSERT(reporter, spaceWidth > 0);
    auto spaces = build("AAAAAA      \nB", 1);
    REPORTER_ASSERT(reporter, ellipsisLine(spaces.get(), 1) == 0);
    auto* impl = static_cast<ParagraphImpl*>(spaces.get());
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(impl->lines()[0].widthWithoutEllipsis(), spacedWidth));
    std::vector<LineMetrics> metrics;
    spaces->getLineMetrics(metrics);
    REPORTER_ASSERT(reporter, metrics[0].fEndExcludingWhitespaces == 12);
    REPORTER_ASSERT(reporter, metrics[0].fEndIndex == 12);
    REPORTER_ASSERT(reporter, impl->lines()[0].spacesWidth() == 0);
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(impl->widthWithTrailingSpaces(),
                                        impl->lines()[0].widthWithoutEllipsis()));
    SkScalar ellipsisWidth = impl->lines()[0].ellipsis()->advance().fX;

    // Spaces that do not fit with the ellipsis go one by one, before any glyph
    SkScalar twoSpaces = plainWidth + 2.5f * spaceWidth + ellipsisWidth;
    auto trimmed = build("AAAAAA      \nB", 1, "Roboto", TextDirection::kLtr, twoSpaces);
    REPORTER_ASSERT(reporter, ellipsisLine(trimmed.get(), 1) == 0);
    trimmed->getLineMetrics(metrics);
    REPORTER_ASSERT(reporter, metrics[0].fEndIndex == 8, "%zu", metrics[0].fEndIndex);
    REPORTER_ASSERT(reporter,
                    static_cast<ParagraphImpl*>(trimmed.get())->lines()[0].width() <= twoSpaces);

    // A blank line keeps its spaces as well, and gets the ellipsis alone if not one space fits
    auto blank = build("   \nB", 1);
    REPORTER_ASSERT(reporter, ellipsisLine(blank.get(), 1) == 0);
    blank->getLineMetrics(metrics);
    REPORTER_ASSERT(reporter, metrics[0].fEndIndex == 3, "%zu", metrics[0].fEndIndex);
    auto lone =
            build("   \nB", 1, "Roboto", TextDirection::kLtr, ellipsisWidth + 0.5f * spaceWidth);
    REPORTER_ASSERT(reporter, ellipsisLine(lone.get(), 1) == 0);
    auto* loneImpl = static_cast<ParagraphImpl*>(lone.get());
    REPORTER_ASSERT(reporter, loneImpl->lines()[0].widthWithoutEllipsis() == 0);

    // A visible text that fits alone but not with the ellipsis loses its last clusters
    SkScalar narrow = plainWidth + 1;
    auto cut = build("AAAAAA\nB", 1, "Roboto", TextDirection::kLtr, narrow);
    REPORTER_ASSERT(reporter, ellipsisLine(cut.get(), 1) == 0);
    auto* cutImpl = static_cast<ParagraphImpl*>(cut.get());
    REPORTER_ASSERT(reporter, cutImpl->lines()[0].widthWithoutEllipsis() < plainWidth);
    REPORTER_ASSERT(reporter, cutImpl->lines()[0].width() <= narrow);
    // A visible text that does not fit even one cluster with the ellipsis gets it alone
    auto alone = build("W\nX", 1, "Roboto", TextDirection::kLtr, ellipsisWidth + 1);
    REPORTER_ASSERT(reporter, ellipsisLine(alone.get(), 1) == 0);

    // The CR of a CRLF line break is part of the line break, not a space before it
    auto crlf = build("AAA  \r\nB", 1);
    REPORTER_ASSERT(reporter, ellipsisLine(crlf.get(), 1) == 0);
    crlf->getLineMetrics(metrics);
    REPORTER_ASSERT(reporter, metrics[0].fEndIndex == 5, "%zu", metrics[0].fEndIndex);

    // The ellipsis of an empty line takes the style of its line break, background included
    TextStyle small;
    small.setFontFamilies({SkString("Roboto")});
    small.setFontSize(20);
    TextStyle large = small;
    large.setFontSize(40);
    large.setBackgroundColor(SkPaint(SkColors::kRed));
    ParagraphStyle paragraphStyle;
    paragraphStyle.setMaxLines(2);
    paragraphStyle.setEllipsis(u"\u2026");
    ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());
    builder.pushStyle(small);
    builder.addText("AAA\n");
    builder.pushStyle(large);
    builder.addText("\nBBB");
    auto styled = builder.Build();
    styled->layout(1000);
    SkBitmap bitmap;
    bitmap.allocN32Pixels(1000, 200);
    SkCanvas canvas(bitmap);
    canvas.clear(SK_ColorWHITE);
    styled->paint(&canvas, 0, 0);
    REPORTER_ASSERT(reporter, ellipsisLine(styled.get(), 2) == 1);
    auto* styledImpl = static_cast<ParagraphImpl*>(styled.get());
    REPORTER_ASSERT(reporter, styledImpl->lines()[1].ellipsis()->font().getSize() == 40);
    std::vector<LineMetrics> styledMetrics;
    styled->getLineMetrics(styledMetrics);
    int lineTop = SkScalarCeilToInt(styledMetrics[1].fBaseline - styledMetrics[1].fAscent);
    REPORTER_ASSERT(reporter, bitmap.getColor(2, lineTop + 2) == SK_ColorRED);
    // A hit on the empty line stays on it, as on any empty line
    auto hit = styled->getGlyphPositionAtCoordinate(1, styledMetrics[1].fBaseline);
    REPORTER_ASSERT(reporter, hit.position == 4 && hit.affinity == Affinity::kDownstream);

    // Same for an RTL empty line, on both sides of the ellipsis
    auto rtl = build("\xD8\xA8\xD8\xAA\n\n\xD8\xA8", 2, "Katibeh", TextDirection::kRtl);
    REPORTER_ASSERT(reporter, ellipsisLine(rtl.get(), 2) == 1);
    rtl->getLineMetrics(metrics);
    for (SkScalar dx : {1.0f, 999.0f, 1001.0f}) {
        auto rtlHit = rtl->getGlyphPositionAtCoordinate(dx, metrics[1].fBaseline);
        REPORTER_ASSERT(reporter,
                        rtlHit.position == 3 && rtlHit.affinity == kDownstream,
                        "%g: %d",
                        dx,
                        rtlHit.position);
    }
#endif
}
