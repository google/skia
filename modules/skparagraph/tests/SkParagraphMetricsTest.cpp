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
#include <string>
#include <vector>

using namespace skia::textlayout;
using namespace skia::textlayout::test;

// TextStyle::getFontMetrics must report the same ascent and descent the layout gives the run
DEF_TEST(SkParagraph_TextStyleFontMetricsMatchRun, reporter) {
    auto fontCollection = sk_make_sp<FontCollection>();
    fontCollection->setDefaultFontManager(ToolUtils::TestFontMgr());
    // Every layout must be freshly shaped to compare against the style it was shaped with
    fontCollection->getParagraphCache()->turnOn(false);

    for (bool heightOverride : {false, true}) {
        for (bool halfLeading : {false, true}) {
            TextStyle textStyle;
            textStyle.setFontSize(50.0f);
            textStyle.setHeightOverride(heightOverride);
            textStyle.setHeight(2.0f);
            textStyle.setHalfLeading(halfLeading);

            ParagraphStyle paragraphStyle;
            ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());
            builder.pushStyle(textStyle);
            builder.addText("A");
            builder.pop();
            auto paragraph = builder.Build();
            paragraph->layout(500.0f);
            auto* impl = static_cast<ParagraphImpl*>(paragraph.get());
            REPORTER_ASSERT(reporter, impl->runs().size() == 1);
            if (impl->runs().size() != 1) {
                continue;
            }
            const Run& run = impl->runs()[0];

            textStyle.setTypeface(run.font().refTypeface());
            SkFontMetrics metrics;
            textStyle.getFontMetrics(&metrics);
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(metrics.fAscent, run.correctAscent(), EPSILON100),
                            "heightOverride=%d halfLeading=%d ascent %g != %g",
                            heightOverride,
                            halfLeading,
                            metrics.fAscent,
                            run.correctAscent());
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(metrics.fDescent, run.correctDescent(), EPSILON100),
                            "heightOverride=%d halfLeading=%d descent %g != %g",
                            heightOverride,
                            halfLeading,
                            metrics.fDescent,
                            run.correctDescent());
        }
    }
}

// Checked: NO DIFF
UNIX_ONLY_TEST(SkParagraph_BaselineParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    TestCanvas canvas("SkParagraph_BaselineParagraph.png");
    const char* text =
            "左線読設Byg後碁給能上目秘使約。満毎冠行来昼本可必図将発確年。今属場育"
            "図情闘陰野高備込制詩西校客。審対江置講今固残必託地集済決維駆年策。立得";
    const size_t len = strlen(text);

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    paragraph_style.setMaxLines(14);
    paragraph_style.setTextAlign(TextAlign::kJustify);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Source Han Serif CN")});
    text_style.setColor(SK_ColorBLACK);
    text_style.setFontSize(55);
    text_style.setLetterSpacing(2);
    text_style.setDecorationStyle(TextDecorationStyle::kSolid);
    text_style.setDecorationColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text, len);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth - 100);
    paragraph->paint(canvas.get(), 0, 0);

    SkRect rect1 = SkRect::MakeXYWH(0,
                                    paragraph->getIdeographicBaseline(),
                                    paragraph->getMaxWidth(),
                                    paragraph->getIdeographicBaseline());
    SkRect rect2 = SkRect::MakeXYWH(0,
                                    paragraph->getAlphabeticBaseline(),
                                    paragraph->getMaxWidth(),
                                    paragraph->getAlphabeticBaseline());
    canvas.drawLine(SK_ColorRED, rect1, false);
    canvas.drawLine(SK_ColorGREEN, rect2, false);

    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(paragraph->getIdeographicBaseline(), 79.035f, EPSILON100));
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(paragraph->getAlphabeticBaseline(), 63.305f, EPSILON100));
}

UNIX_ONLY_TEST(SkParagraph_LineMetrics, reporter) {

    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    TestCanvas canvas("SkParagraph_LineMetrics.png");

    const char* text = "One line of text\n";
    const size_t len = strlen(text);

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setColor(SK_ColorBLACK);

    text_style.setFontSize(8);
    builder.pushStyle(text_style);
    builder.addText(text, len);
    builder.pop();

    text_style.setFontSize(12);
    builder.pushStyle(text_style);
    builder.addText(text, len);
    builder.pop();

    text_style.setFontSize(18);
    builder.pushStyle(text_style);
    builder.addText(text, len);
    builder.pop();

    text_style.setFontSize(30);
    builder.pushStyle(text_style);
    builder.addText(text, len - 1);  // Skip the last \n
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);

    std::vector<LineMetrics> metrics;
    paragraph->getLineMetrics(metrics);

    SkDEBUGCODE(auto impl = static_cast<ParagraphImpl*>(paragraph.get());)
    SkASSERT(metrics.size() == impl->lines().size());
    for (size_t i = 0; i < metrics.size(); ++i) {
        SkDEBUGCODE(auto& line = impl->lines()[i];)
        SkDEBUGCODE(auto baseline = metrics[i].fBaseline;)
        SkDEBUGCODE(auto top = line.offset().fY;)
        SkDEBUGCODE(auto bottom = line.offset().fY + line.height();)
        SkASSERT(baseline > top && baseline <= bottom);
    }

    paragraph->paint(canvas.get(), 0, 0);
    auto rects =
            paragraph->getRectsForRange(0, len * 4, RectHeightStyle::kMax, RectWidthStyle::kTight);

    SkPaint red;
    red.setColor(SK_ColorRED);
    red.setStyle(SkPaint::kStroke_Style);
    red.setAntiAlias(true);
    red.setStrokeWidth(1);

    for (auto& rect : rects) {
        canvas.get()->drawRect(rect.rect, red);
    }

    SkPaint green;
    green.setColor(SK_ColorGREEN);
    green.setStyle(SkPaint::kStroke_Style);
    green.setAntiAlias(true);
    green.setStrokeWidth(1);
    for (auto& metric : metrics) {
        auto x0 = 0.0;
        auto x1 = metric.fWidth;
        auto y = metric.fBaseline;
        canvas.get()->drawLine(x0, y, x1, y, green);
    }
}

UNIX_ONLY_TEST(SkParagraph_Intrinsic, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    SkString text(std::string(3000, 'a'));

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Google Sans")});
    text_style.setFontSize(12.0f);
    text_style.setColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text.c_str());

    auto paragraph = builder.Build();
    paragraph->layout(300000.0f);
    REPORTER_ASSERT(reporter,
                    paragraph->getMinIntrinsicWidth() <= paragraph->getMaxIntrinsicWidth());
}

UNIX_ONLY_TEST(SkParagraph_DifferentFontsTopLine, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    TestCanvas canvas("SkParagraph_DifferentFontsTopLine.png");

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Ahem")});
    text_style.setFontSize(10);
    SkPaint black;
    black.setColor(SK_ColorBLACK);
    text_style.setForegroundColor(black);

    ParagraphStyle paragraph_style;
    paragraph_style.setTextStyle(text_style);
    paragraph_style.setTextDirection(TextDirection::kLtr);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());

    text_style.setFontSize(30.0);
    builder.pushStyle(text_style);
    builder.addText(u"Atwater Peel ");
    text_style.setFontSize(15.0);
    builder.pushStyle(text_style);
    builder.addText(u"Sherbrooke Bonaventure ");
    text_style.setFontSize(10.0);
    builder.pushStyle(text_style);
    builder.addText(u"hi wassup!");

    auto paragraph = builder.Build();
    paragraph->layout(797);
    paragraph->paint(canvas.get(), 0, 0);
    auto boxes = paragraph->getRectsForRange(
            0, 60, RectHeightStyle::kIncludeLineSpacingTop, RectWidthStyle::kMax);
    REPORTER_ASSERT(reporter, boxes.size() == 4);
    if (boxes.size() >= 4) {
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.fTop, 00.0f));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.fBottom, 30.0f));

        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[1].rect.fTop, 00.0f));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[1].rect.fBottom, 30.0f));

        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[2].rect.fTop, 00.0f));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[2].rect.fBottom, 30.0f));

        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[3].rect.fTop, 30.0f));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[3].rect.fBottom, 40.0f));
    }
}

UNIX_ONLY_TEST(SkParagraph_lineMetricsAfterUpdate, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    fontCollection->setDefaultFontManager(ToolUtils::TestFontMgr());
    fontCollection->enableFontFallback();

    auto text = std::u16string(u"hello world");

    ParagraphStyle paragraph_style;

    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());
    builder.addText(text);

    auto paragraph = builder.Build();
    paragraph->layout(200.);

    std::vector<LineMetrics> lm;
    paragraph->getLineMetrics(lm);
    REPORTER_ASSERT(reporter, lm.size() == 1, "size: %zu", lm.size());

    paragraph->updateFontSize(0, text.size(), 42);
    paragraph->layout(200.);
    paragraph->getLineMetrics(lm);
    REPORTER_ASSERT(reporter, lm.size() == 2, "size: %zu", lm.size());
}
