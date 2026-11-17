/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkPaint.h"
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
#include <chrono>
#include <limits>
#include <string>
#include <vector>

using namespace skia::textlayout;
using namespace skia::textlayout::test;

UNIX_ONLY_TEST(SkParagraph_InlinePlaceholderGetRectsParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    TestCanvas canvas("SkParagraph_InlinePlaceholderGetRectsParagraph.png");
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    const char* text = "012 34";
    const size_t len = strlen(text);

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    paragraph_style.setMaxLines(14);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setColor(SK_ColorBLACK);
    text_style.setFontSize(26);
    text_style.setWordSpacing(5);
    text_style.setLetterSpacing(1);
    text_style.setDecoration(TextDecoration::kUnderline);
    text_style.setDecorationColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text, len);

    PlaceholderStyle placeholder1(
            50, 50, PlaceholderAlignment::kBaseline, TextBaseline::kAlphabetic, 50);
    PlaceholderStyle placeholder2(
            5, 20, PlaceholderAlignment::kBaseline, TextBaseline::kAlphabetic, 10);

    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder2);  // 8 + 1
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder2);  // 5 + 1
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder1);  // 8 + 0

    builder.addText(text, len);

    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder2);
    builder.addPlaceholder(placeholder2);  // 1 + 2
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder2);
    builder.addPlaceholder(placeholder2);  // 1 + 2

    builder.addText(text, len);
    builder.addText(text, len);
    builder.addText(text, len);
    builder.addText(text, len);
    builder.addText(text, len);
    builder.addText(text, len);
    builder.addText(text, len);
    builder.addText(text, len);
    builder.addText(text, len);
    builder.addText(text, len);
    builder.addText(text, len);  // 11

    builder.addPlaceholder(placeholder2);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder2);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder2);

    builder.addText(text, len);

    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);
    paragraph->paint(canvas.get(), 0, 0);

    RectHeightStyle rect_height_style = RectHeightStyle::kMax;
    RectWidthStyle rect_width_style = RectWidthStyle::kTight;

    auto boxes = paragraph->getRectsForPlaceholders();
    canvas.drawRects(SK_ColorRED, boxes);

    REPORTER_ASSERT(reporter, boxes.size() == 34);
    if (boxes.size() >= 34) {
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 90.921f, EPSILON50));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 0, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 140.921f, EPSILON50));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 50, EPSILON100));

        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[16].rect.left(), 800.921f, EPSILON20));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[16].rect.top(), 0, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[16].rect.right(), 850.921f, EPSILON20));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[16].rect.bottom(), 50, EPSILON100));

        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[33].rect.left(), 503.382f, EPSILON10));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[33].rect.top(), 160, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[33].rect.right(), 508.382f, EPSILON10));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[33].rect.bottom(), 180, EPSILON100));
    }

    boxes = paragraph->getRectsForRange(30, 50, rect_height_style, rect_width_style);
    canvas.drawRects(SK_ColorBLUE, boxes);

    REPORTER_ASSERT(reporter, boxes.size() == 8);
    if (boxes.size() >= 8) {
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 216.097f, EPSILON50));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 60, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 290.921f, EPSILON50));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 120, EPSILON100));

        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[1].rect.left(), 290.921f, EPSILON20));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[1].rect.top(), 60, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[1].rect.right(), 340.921f, EPSILON20));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[1].rect.bottom(), 120, EPSILON100));

        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[2].rect.left(), 340.921f, EPSILON50));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[2].rect.top(), 60, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[2].rect.right(), 345.921f, EPSILON50));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[2].rect.bottom(), 120, EPSILON100));
    }
}

// Checked: DIFF (2 boxes and each space is a word)
UNIX_ONLY_TEST(SkParagraph_ArabicRectsParagraph, reporter) {

    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    TestCanvas canvas("SkParagraph_ArabicRectsParagraph.png");
    const char* text = "بمباركة التقليدية قام عن. تصفح يد    ";
    const size_t len = strlen(text);

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    paragraph_style.setMaxLines(14);
    paragraph_style.setTextAlign(TextAlign::kRight);
    paragraph_style.setTextDirection(TextDirection::kRtl);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Noto Naskh Arabic")});
    text_style.setFontSize(26);
    text_style.setWordSpacing(5);
    text_style.setColor(SK_ColorBLACK);
    text_style.setDecoration(TextDecoration::kUnderline);
    text_style.setDecorationColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text, len);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth - 100);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, impl->runs().size() == 1);

    paragraph->paint(canvas.get(), 0, 0);

    RectHeightStyle rect_height_style = RectHeightStyle::kMax;
    RectWidthStyle rect_width_style = RectWidthStyle::kTight;
    std::vector<TextBox> boxes =
            paragraph->getRectsForRange(0, 100, rect_height_style, rect_width_style);
    canvas.drawRects(SK_ColorRED, boxes);

    REPORTER_ASSERT(reporter, boxes.size() == 1ull);
    if (boxes.size() >= 1) {
        REPORTER_ASSERT(reporter,
                        SkScalarNearlyEqual(
                                boxes[0].rect.left(), 538.120f, EPSILON100));  // DIFF: 510.09375
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), -0.280f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 900, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 44, EPSILON100));
    }
}

UNIX_ONLY_TEST(SkParagraph_GetRectsForRangeParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    TestCanvas canvas("SkParagraph_GetRectsForRangeParagraph.png");
    const char* text =
            "12345,  \"67890\" 12345 67890 12345 67890 12345 67890 12345 67890 12345 "
            "67890 12345";
    const size_t len = strlen(text);

    ParagraphStyle paragraphStyle;
    paragraphStyle.setTextAlign(TextAlign::kLeft);
    paragraphStyle.setMaxLines(10);
    paragraphStyle.turnHintingOff();
    TextStyle textStyle;
    textStyle.setFontFamilies({SkString("Roboto")});
    textStyle.setFontSize(50);
    textStyle.setColor(SK_ColorBLACK);
    textStyle.setFontStyle(SkFontStyle(
            SkFontStyle::kMedium_Weight, SkFontStyle::kNormal_Width, SkFontStyle::kUpright_Slant));

    ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());
    builder.pushStyle(textStyle);
    builder.addText(text, len);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(550);
    paragraph->paint(canvas.get(), 0, 0);

    RectHeightStyle heightStyle = RectHeightStyle::kMax;
    RectWidthStyle widthStyle = RectWidthStyle::kTight;

    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setAntiAlias(true);
    paint.setStrokeWidth(1);

    {
        auto result = paragraph->getRectsForRange(0, 0, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.empty());
    }
    {
        auto result = paragraph->getRectsForRange(0, 1, heightStyle, widthStyle);
        canvas.drawRects(SK_ColorRED, result);
        REPORTER_ASSERT(reporter, result.size() == 1);
        if (result.size() >= 1) {
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 0, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.top(), 0.40625f, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.right(), 28.417f, EPSILON100));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 59, EPSILON100));
        }
    }
    {
        auto result = paragraph->getRectsForRange(2, 8, heightStyle, widthStyle);
        canvas.drawRects(SK_ColorBLUE, result);
        REPORTER_ASSERT(reporter, result.size() == 1);
        if (result.size() >= 1) {
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.left(), 56.835f, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.top(), 0.40625f, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.right(), 177.97f, EPSILON100));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 59, EPSILON100));
        }
    }
    {
        auto result = paragraph->getRectsForRange(8, 21, heightStyle, widthStyle);
        canvas.drawRects(SK_ColorGREEN, result);
        REPORTER_ASSERT(reporter, result.size() == 1);
        if (result.size() >= 1) {
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.left(), 177.97f, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.top(), 0.40625f, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.right(), 507.031f, EPSILON100));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 59, EPSILON100));
        }
    }
    {
        auto result = paragraph->getRectsForRange(30, 100, heightStyle, widthStyle);
        canvas.drawRects(SK_ColorRED, result);
        REPORTER_ASSERT(reporter, result.size() == 4);
        if (result.size() >= 4) {
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.left(), 211.375f, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.top(), 59.40625f, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.right(), 463.623f, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.bottom(), 118, EPSILON100));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[3].rect.left(), 0, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[3].rect.top(), 236.406f, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[3].rect.right(), 142.089f, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[3].rect.bottom(), 295, EPSILON100));
        }
    }
    {
        auto result = paragraph->getRectsForRange(19, 22, heightStyle, widthStyle);
        canvas.drawRects(SK_ColorBLUE, result);
        REPORTER_ASSERT(reporter, result.size() == 1);
        if (result.size() >= 1) {
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.left(), 450.1875f, EPSILON20));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.top(), 0.40625f, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.right(), 519.47266f, EPSILON20));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 59, EPSILON100));
        }
    }
    {
        auto result = paragraph->getRectsForRange(21, 21, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.empty());
    }
}

UNIX_ONLY_TEST(SkParagraph_GetRectsForRangeTight, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    TestCanvas canvas("SkParagraph_GetRectsForRangeTight.png");
    const char* text =
            "(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)("
            "　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)("
            "　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)";
    const size_t len = strlen(text);
    /*
(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)
    S    S     S   S   S    S     S   S   S    S     S   S   S    S     S   S
 G  G G  G  G  G  GGG  G G  G  G  G  GGG  G G  G  G  G  GGG  G G  G  G  G  GGG
 W                  W                  W                  W
 (the same 4 groups repeat 5 times)

 */
    ParagraphStyle paragraphStyle;
    paragraphStyle.setTextAlign(TextAlign::kLeft);
    paragraphStyle.setMaxLines(10);
    paragraphStyle.turnHintingOff();
    TextStyle textStyle;
    textStyle.setFontFamilies({SkString("Noto Sans CJK JP")});
    textStyle.setFontSize(50);
    textStyle.setColor(SK_ColorBLACK);
    textStyle.setFontStyle(SkFontStyle(
            SkFontStyle::kMedium_Weight, SkFontStyle::kNormal_Width, SkFontStyle::kUpright_Slant));

    ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());
    builder.pushStyle(textStyle);
    builder.addText(text, len);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(550);
    paragraph->paint(canvas.get(), 0, 0);

    RectHeightStyle heightStyle = RectHeightStyle::kTight;
    RectWidthStyle widthStyle = RectWidthStyle::kTight;
    {
        auto result = paragraph->getRectsForRange(0, 0, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.empty());
    }
    {
        auto result = paragraph->getRectsForRange(0, 1, heightStyle, widthStyle);
        canvas.drawRects(SK_ColorRED, result);
        REPORTER_ASSERT(reporter, result.size() == 1);
        if (result.size() >= 1) {
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 0, EPSILON100));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 0, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.right(), 16.898f, EPSILON100));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 74, EPSILON100));
        }
    }
    {
        auto result = paragraph->getRectsForRange(2, 8, heightStyle, widthStyle);
        canvas.drawRects(SK_ColorBLUE, result);
        REPORTER_ASSERT(reporter, result.size() == 1);
        if (result.size() >= 1) {
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.left(), 66.899f, EPSILON100));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 0, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.right(), 264.099f, EPSILON100));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 74, EPSILON100));
        }
    }
    {
        auto result = paragraph->getRectsForRange(8, 21, heightStyle, widthStyle);
        canvas.drawRects(SK_ColorGREEN, result);
        REPORTER_ASSERT(reporter, result.size() == 2);
        if (result.size() >= 2) {
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.left(), 264.099f, EPSILON100));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 0, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.right(), 595.085f, EPSILON50));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 74, EPSILON100));
        }
    }
}

// This is the test I cannot accommodate
// Any text range gets a smallest glyph rectangle
DEF_TEST_DISABLED(SkParagraph_GetRectsForRangeIncludeCombiningCharacter, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    TestCanvas canvas("SkParagraph_GetRectsForRangeIncludeCombiningCharacter.png");
    const char* text = "ดีสวัสดีชาวโลกที่น่ารัก";
    const size_t len = strlen(text);

    ParagraphStyle paragraphStyle;
    paragraphStyle.setTextAlign(TextAlign::kLeft);
    paragraphStyle.setMaxLines(10);
    paragraphStyle.turnHintingOff();
    ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());

    TextStyle textStyle;
    textStyle.setFontFamilies({SkString("Roboto")});
    textStyle.setFontSize(50);
    textStyle.setLetterSpacing(1);
    textStyle.setWordSpacing(5);
    textStyle.setHeight(1);
    textStyle.setColor(SK_ColorBLACK);

    builder.pushStyle(textStyle);
    builder.addText(text, len);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth - 100);
    paragraph->paint(canvas.get(), 0, 0);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, impl->lines().size() == 1);

    RectHeightStyle heightStyle = RectHeightStyle::kTight;
    RectWidthStyle widthStyle = RectWidthStyle::kTight;
    {
        auto result = paragraph->getRectsForRange(0, 0, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.empty());
    }
    {
        auto first = paragraph->getRectsForRange(0, 1, heightStyle, widthStyle);
        auto second = paragraph->getRectsForRange(1, 2, heightStyle, widthStyle);
        auto last = paragraph->getRectsForRange(0, 2, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, first.size() == 0 && second.size() == 1 && last.size() == 1);
        REPORTER_ASSERT(reporter, second[0].rect == last[0].rect);
    }
    {
        auto first = paragraph->getRectsForRange(3, 4, heightStyle, widthStyle);
        auto second = paragraph->getRectsForRange(4, 5, heightStyle, widthStyle);
        auto last = paragraph->getRectsForRange(3, 5, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, first.size() == 0 && second.size() == 1 && last.size() == 1);
        REPORTER_ASSERT(reporter, second[0].rect == last[0].rect);
    }
    {
        auto first = paragraph->getRectsForRange(14, 15, heightStyle, widthStyle);
        auto second = paragraph->getRectsForRange(15, 16, heightStyle, widthStyle);
        auto third = paragraph->getRectsForRange(16, 17, heightStyle, widthStyle);
        auto last = paragraph->getRectsForRange(14, 17, heightStyle, widthStyle);
        REPORTER_ASSERT(
                reporter,
                first.size() == 0 && second.size() == 0 && third.size() == 1 && last.size() == 1);
        REPORTER_ASSERT(reporter, third[0].rect == last[0].rect);
    }
}

// Checked: NO DIFF
UNIX_ONLY_TEST(SkParagraph_GetRectsForRangeCenterParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    TestCanvas canvas("SkParagraph_GetRectsForRangeCenterParagraph.png");
    // Minikin uses a hard coded list of unicode characters that he treats as invisible - as spaces.
    // It's absolutely wrong - invisibility is a glyph attribute, not character/grapheme.
    // Any attempt to substitute one for another leads to errors
    // (for instance, some fonts can use these hard coded characters for something that is visible)
    const char* text = "01234  　 ";  // includes ideographic space and english space.
    const size_t len = strlen(text);

    ParagraphStyle paragraphStyle;
    paragraphStyle.setTextAlign(TextAlign::kCenter);
    paragraphStyle.setMaxLines(10);
    paragraphStyle.turnHintingOff();
    ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());

    TextStyle textStyle;
    textStyle.setFontFamilies({SkString("Roboto")});
    textStyle.setFontSize(50);
    textStyle.setHeight(1);
    textStyle.setColor(SK_ColorBLACK);
    textStyle.setFontStyle(SkFontStyle(
            SkFontStyle::kMedium_Weight, SkFontStyle::kNormal_Width, SkFontStyle::kUpright_Slant));

    builder.pushStyle(textStyle);
    builder.addText(text, len);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(550);
    paragraph->paint(canvas.get(), 0, 0);

    // Some of the formatting lazily done on paint
    RectHeightStyle heightStyle = RectHeightStyle::kMax;
    RectWidthStyle widthStyle = RectWidthStyle::kTight;
    {
        auto result = paragraph->getRectsForRange(0, 0, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.empty());
    }

    {
        auto result = paragraph->getRectsForRange(0, 1, heightStyle, widthStyle);
        canvas.drawRects(SK_ColorRED, result);
        REPORTER_ASSERT(reporter, result.size() == 1);
        if (result.size() >= 1) {
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.left(), 203.955f, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.top(), 0.40625f, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.right(), 232.373f, EPSILON100));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 59, EPSILON100));
        }
    }

    {
        auto result = paragraph->getRectsForRange(2, 4, heightStyle, widthStyle);
        canvas.drawRects(SK_ColorBLUE, result);
        REPORTER_ASSERT(reporter, result.size() == 1);
        if (result.size() >= 1) {
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.left(), 260.791f, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.top(), 0.40625f, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.right(), 317.626f, EPSILON100));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 59, EPSILON100));
        }
    }

    {
        auto result = paragraph->getRectsForRange(4, 5, heightStyle, widthStyle);
        canvas.drawRects(SK_ColorGREEN, result);
        REPORTER_ASSERT(reporter, result.size() == 1);
        if (result.size() >= 1) {
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.left(), 317.626f, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.top(), 0.40625f, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.right(), 346.044f, EPSILON100));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 59, EPSILON100));
        }
    }

    {
        auto result = paragraph->getRectsForRange(4, 6, heightStyle, widthStyle);
        canvas.drawRects(SK_ColorBLACK, result);
        REPORTER_ASSERT(reporter, result.size() == 1);  // DIFF
        if (result.size() >= 1) {
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.left(), 317.626f, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.top(), 0.40625f, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.right(), 358.494f, EPSILON100));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 59, EPSILON100));
        }
    }

    {
        auto result = paragraph->getRectsForRange(5, 6, heightStyle, widthStyle);
        canvas.drawRects(SK_ColorRED, result);
        REPORTER_ASSERT(reporter, result.size() == 1);
        if (result.size() >= 1) {
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.left(), 346.044f, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.top(), 0.40625f, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.right(), 358.494f, EPSILON100));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 59, EPSILON100));
        }
    }

    {
        auto result = paragraph->getRectsForRange(21, 21, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.empty());
    }
}

// Checked DIFF+
UNIX_ONLY_TEST(SkParagraph_GetRectsForRangeCenterParagraphNewlineCentered, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    TestCanvas canvas("SkParagraph_GetRectsForRangeCenterParagraphNewlineCentered.png");
    const char* text = "01234\n";
    const size_t len = strlen(text);

    ParagraphStyle paragraphStyle;
    paragraphStyle.setTextAlign(TextAlign::kCenter);
    paragraphStyle.setMaxLines(10);
    paragraphStyle.turnHintingOff();
    ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());

    TextStyle textStyle;
    textStyle.setFontFamilies({SkString("Roboto")});
    textStyle.setFontSize(50);
    textStyle.setHeight(1);
    textStyle.setColor(SK_ColorBLACK);
    textStyle.setFontStyle(SkFontStyle(
            SkFontStyle::kMedium_Weight, SkFontStyle::kNormal_Width, SkFontStyle::kUpright_Slant));

    builder.pushStyle(textStyle);
    builder.addText(text, len);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(550);

    paragraph->paint(canvas.get(), 0, 0);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, impl->lines().size() == 2);

    RectHeightStyle heightStyle = RectHeightStyle::kMax;
    RectWidthStyle widthStyle = RectWidthStyle::kTight;
    {
        auto result = paragraph->getRectsForRange(0, 0, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.empty());
    }

    {
        auto result = paragraph->getRectsForRange(0, 1, heightStyle, widthStyle);
        canvas.drawRects(SK_ColorRED, result);
        REPORTER_ASSERT(reporter, result.size() == 1);
        if (result.size() >= 1) {
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.left(), 203.955f, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.top(), 0.40625f, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.right(), 232.373f, EPSILON100));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 59, EPSILON100));
        }
    }

    {
        auto result = paragraph->getRectsForRange(6, 7, heightStyle, widthStyle);
        canvas.drawRects(SK_ColorBLUE, result);
        REPORTER_ASSERT(reporter, result.size() == 1);
        if (result.size() >= 1) {
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.left(), 275.0f, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.top(), 59.406f, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.right(), 275.0f, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.bottom(), 118, EPSILON100));
        }
    }
}

// Checked NO DIFF
UNIX_ONLY_TEST(SkParagraph_GetRectsForRangeCenterMultiLineParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    TestCanvas canvas("SkParagraph_GetRectsForRangeCenterMultiLineParagraph.png");
    const char* text =
            "01234  　 \n0123　        ";  // includes ideographic space and english space.
    const size_t len = strlen(text);

    ParagraphStyle paragraphStyle;
    paragraphStyle.setTextAlign(TextAlign::kCenter);
    paragraphStyle.setMaxLines(10);
    paragraphStyle.turnHintingOff();
    ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());

    TextStyle textStyle;
    textStyle.setFontFamilies({SkString("Roboto")});
    textStyle.setFontSize(50);
    textStyle.setHeight(1);
    textStyle.setColor(SK_ColorBLACK);
    textStyle.setFontStyle(SkFontStyle(
            SkFontStyle::kMedium_Weight, SkFontStyle::kNormal_Width, SkFontStyle::kUpright_Slant));

    builder.pushStyle(textStyle);
    builder.addText(text, len);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(550);

    paragraph->paint(canvas.get(), 0, 0);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());

    REPORTER_ASSERT(reporter, impl->lines().size() == 2);

    RectHeightStyle heightStyle = RectHeightStyle::kMax;
    RectWidthStyle widthStyle = RectWidthStyle::kTight;
    SkScalar epsilon = 0.01f;
    {
        auto result = paragraph->getRectsForRange(0, 0, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.empty());
    }
    {
        auto result = paragraph->getRectsForRange(0, 1, heightStyle, widthStyle);
        canvas.drawRects(SK_ColorRED, result);
        REPORTER_ASSERT(reporter, result.size() == 1);
        if (result.size() >= 1) {
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.left(), 203.955f, epsilon));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 0.40625f, epsilon));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.right(), 232.373f, epsilon));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 59, epsilon));
        }
    }
    {
        auto result = paragraph->getRectsForRange(2, 4, heightStyle, widthStyle);
        canvas.drawRects(SK_ColorBLUE, result);
        REPORTER_ASSERT(reporter, result.size() == 1);
        if (result.size() >= 1) {
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.left(), 260.791f, epsilon));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 0.40625f, epsilon));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.right(), 317.626f, epsilon));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 59, epsilon));
        }
    }
    {
        auto result = paragraph->getRectsForRange(4, 6, heightStyle, widthStyle);
        canvas.drawRects(SK_ColorGREEN, result);
        REPORTER_ASSERT(reporter, result.size() == 1);
        if (result.size() >= 1) {
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.left(), 317.626f, epsilon));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 0.40625f, epsilon));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.right(), 358.494f, epsilon));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 59, epsilon));
        }
    }
    {
        auto result = paragraph->getRectsForRange(5, 6, heightStyle, widthStyle);
        canvas.drawRects(SK_ColorYELLOW, result);
        REPORTER_ASSERT(reporter, result.size() == 1);
        if (result.size() >= 1) {
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.left(), 346.044f, epsilon));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 0.40625f, epsilon));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.right(), 358.494f, epsilon));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 59, epsilon));
        }
    }
    {
        auto result = paragraph->getRectsForRange(10, 12, heightStyle, widthStyle);
        canvas.drawRects(SK_ColorCYAN, result);
        REPORTER_ASSERT(reporter, result.size() == 1);
        if (result.size() >= 1) {
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.left(), 218.164f, epsilon));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.top(), 59.40625f, epsilon));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 275, epsilon));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 118, epsilon));
        }
    }
    {
        auto result = paragraph->getRectsForRange(14, 18, heightStyle, widthStyle);
        canvas.drawRects(SK_ColorBLACK, result);
        REPORTER_ASSERT(reporter, result.size() == 1);
        if (result.size() >= 1) {
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.left(), 331.835f, epsilon));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.top(), 59.40625f, epsilon));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.right(), 419.189f, epsilon));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 118, epsilon));
        }
    }
    {
        auto result = paragraph->getRectsForRange(21, 21, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.empty());
    }
}

// Checked: DIFF (line height rounding error)
UNIX_ONLY_TEST(SkParagraph_GetRectsForRangeStrut, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    TestCanvas canvas("SkParagraph_GetRectsForRangeStrut.png");
    const char* text = "Chinese 字典";
    const size_t len = strlen(text);

    StrutStyle strutStyle;
    strutStyle.setStrutEnabled(true);
    strutStyle.setFontFamilies({SkString("Roboto")});
    strutStyle.setFontSize(14.0);

    ParagraphStyle paragraphStyle;
    paragraphStyle.setStrutStyle(strutStyle);

    TextStyle textStyle;
    textStyle.setFontFamilies({SkString("Noto Sans CJK JP")});
    textStyle.setFontSize(20);
    textStyle.setColor(SK_ColorBLACK);

    ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());
    builder.pushStyle(textStyle);
    builder.addText(text, len);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(550);
    paragraph->paint(canvas.get(), 0, 0);

    {
        auto result =
                paragraph->getRectsForRange(0, 10, RectHeightStyle::kTight, RectWidthStyle::kMax);
        canvas.drawRects(SK_ColorGREEN, result);
        REPORTER_ASSERT(reporter, result.size() == 1);
    }

    {
        auto result =
                paragraph->getRectsForRange(0, 10, RectHeightStyle::kStrut, RectWidthStyle::kMax);
        canvas.drawRects(SK_ColorRED, result);
        REPORTER_ASSERT(reporter, result.size() == 1);
        if (result.size() >= 1) {
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 0, EPSILON100));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 10.611f, EPSILON2));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.right(), 118.605f, EPSILON50));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.bottom(), 27.017f, EPSILON2));
        }
    }
}

UNIX_ONLY_TEST(SkParagraph_GetRectsForRangeStrutWithHeight, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    const char* text = "A";
    const size_t len = strlen(text);

    StrutStyle strutStyle;
    strutStyle.setStrutEnabled(true);
    strutStyle.setFontFamilies({SkString("Roboto")});
    strutStyle.setFontSize(14.0);
    strutStyle.setHeightOverride(true);
    strutStyle.setHeight(2.0);
    strutStyle.setLeading(3.0);

    ParagraphStyle paragraphStyle;
    paragraphStyle.setStrutStyle(strutStyle);

    TextStyle textStyle;
    textStyle.setFontFamilies({SkString("Roboto")});
    textStyle.setFontSize(10);
    textStyle.setColor(SK_ColorBLACK);

    ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());
    builder.pushStyle(textStyle);
    builder.addText(text, len);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(550);

    auto result = paragraph->getRectsForRange(0, 1, RectHeightStyle::kStrut, RectWidthStyle::kMax);
    REPORTER_ASSERT(reporter, result.size() == 1);
    if (result.size() >= 1) {
        // Half of the strut leading: 3.0 * 14.0 / 2
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 21.0, EPSILON100));
        // Strut height 2.0 * 14.0
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.height(), 28.0, EPSILON100));
    }
}

UNIX_ONLY_TEST(SkParagraph_GetRectsForRangeStrutWithHeightAndHalfLeading, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    const char* text = "A";
    const size_t len = strlen(text);

    StrutStyle strutStyle;
    strutStyle.setStrutEnabled(true);
    strutStyle.setFontFamilies({SkString("Roboto")});
    strutStyle.setFontSize(14.0);
    strutStyle.setHeightOverride(true);
    strutStyle.setHeight(2.0);
    strutStyle.setLeading(3.0);
    strutStyle.setHalfLeading(true);

    ParagraphStyle paragraphStyle;
    paragraphStyle.setStrutStyle(strutStyle);

    TextStyle textStyle;
    textStyle.setFontFamilies({SkString("Roboto")});
    textStyle.setFontSize(10);
    textStyle.setColor(SK_ColorBLACK);

    ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());
    builder.pushStyle(textStyle);
    builder.addText(text, len);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(550);

    // Produces the same results as halfLeading = false.
    auto result = paragraph->getRectsForRange(0, 1, RectHeightStyle::kStrut, RectWidthStyle::kMax);
    REPORTER_ASSERT(reporter, result.size() == 1);
    if (result.size() >= 1) {
        // Half of the strut leading: 3.0 * 14.0 / 2
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 21.0, EPSILON100));
        // Strut height 2.0 * 14.0
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.height(), 28.0, EPSILON100));
    }
}

UNIX_ONLY_TEST(SkParagraph_GetRectsForRangeStrutFallback, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    TestCanvas canvas("SkParagraph_GetRectsForRangeStrutFallback.png");
    const char* text = "Chinese 字典";
    const size_t len = strlen(text);

    StrutStyle strutStyle;
    strutStyle.setStrutEnabled(false);

    ParagraphStyle paragraphStyle;
    paragraphStyle.setStrutStyle(strutStyle);

    TextStyle textStyle;
    textStyle.setFontFamilies({SkString("Noto Sans CJK JP")});
    textStyle.setFontSize(20);
    textStyle.setColor(SK_ColorBLACK);

    ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());
    builder.pushStyle(textStyle);
    builder.addText(text, len);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(550);
    paragraph->paint(canvas.get(), 0, 0);

    auto result1 =
            paragraph->getRectsForRange(0, 10, RectHeightStyle::kTight, RectWidthStyle::kMax);
    canvas.drawRects(SK_ColorGREEN, result1);
    REPORTER_ASSERT(reporter, result1.size() == 1);

    auto result2 =
            paragraph->getRectsForRange(0, 10, RectHeightStyle::kStrut, RectWidthStyle::kMax);
    canvas.drawRects(SK_ColorRED, result2);
    REPORTER_ASSERT(reporter, result2.size() == 1);

    if (result1.size() >= 1 && result2.size() >= 1) {
        REPORTER_ASSERT(reporter, result1[0].rect == result2[0].rect);
    }
}

// getRectsForRange over the whole text must stay linear in the number of lines when the rounding
// hack is on: an 8x longer text must not take ~64x longer. Serial, so other tests do not skew
// the timing, and with the portable test fonts, so it runs on every bot
DEF_SERIAL_TEST(SkParagraph_GetRectsForRangeLinearInLines, reporter) {
    auto fontCollection = sk_make_sp<FontCollection>();
    fontCollection->setDefaultFontManager(ToolUtils::TestFontMgr());
    TextStyle text_style;
    text_style.setFontSize(20);
    text_style.setColor(SK_ColorBLACK);
    ParagraphStyle paragraph_style;
    paragraph_style.setTextStyle(text_style);
    paragraph_style.setApplyRoundingHack(true);

    auto timeRects = [&](int lineCount, size_t* boxCount) {
        std::string text;
        for (int i = 0; i < lineCount; ++i) {
            text += "line\n";
        }
        ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());
        builder.pushStyle(text_style);
        builder.addText(text.data(), text.size());
        auto paragraph = builder.Build();
        paragraph->layout(1000);
        double best = std::numeric_limits<double>::max();
        // The best of several runs filters out scheduler noise on shared bots
        for (int run = 0; run < 3; ++run) {
            auto start = std::chrono::steady_clock::now();
            auto boxes = paragraph->getRectsForRange(
                    0, text.size(), RectHeightStyle::kTight, RectWidthStyle::kTight);
            std::chrono::duration<double, std::milli> ms = std::chrono::steady_clock::now() - start;
            best = std::min(best, ms.count());
            *boxCount = boxes.size();
        }
        return best;
    };

    size_t smallBoxes = 0;
    size_t largeBoxes = 0;
    double small = timeRects(1000, &smallBoxes);
    double large = timeRects(8000, &largeBoxes);
    REPORTER_ASSERT(
            reporter, smallBoxes >= 1000 && largeBoxes >= 8000, "%zu %zu", smallBoxes, largeBoxes);
    // Linear work gives a ratio of ~8 and rounding all earlier boxes on every line gives ~50;
    // the threshold leaves a 3x margin on both sides for noisy bots
    REPORTER_ASSERT(reporter, large < 24 * small, "%f ms vs %f ms", large, small);
}

// Checked: DIFF+
UNIX_ONLY_TEST(SkParagraph_EmojiMultiLineRectsParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    TestCanvas canvas("SkParagraph_EmojiMultiLineRectsParagraph.png");
    const char* text =
            "👩‍👩‍👦👩‍👩‍👧‍👧🇺🇸👩‍👩‍👦👩‍👩‍👧‍👧i🇺🇸👩‍👩‍👦👩‍👩‍👧‍👧🇺🇸👩‍👩‍👦👩‍👩‍👧‍👧🇺🇸"
            "👩‍👩‍👦👩‍👩‍👧‍👧🇺🇸👩‍👩‍👦👩‍👩‍👧‍👧🇺🇸👩‍👩‍👦👩‍👩‍👧‍👧🇺🇸👩‍👩‍👦👩‍👩‍👧‍👧🇺🇸"
            "👩‍👩‍👦👩‍👩‍👧‍👧🇺🇸👩‍👩‍👦👩‍👩‍👧‍👧🇺🇸👩‍👩‍👦👩‍👩‍👧‍👧🇺🇸👩‍👩‍👦👩‍👩‍👧‍👧🇺🇸"
            "👩‍👩‍👦👩‍👩‍👧‍👧🇺🇸👩‍👩‍👦👩‍👩‍👧‍👧🇺🇸👩‍👩‍👦👩‍👩‍👧‍👧🇺🇸👩‍👩‍👦👩‍👩‍👧‍👧🇺🇸"
            "❄🍕🍔🍟🥝🍱🕶🎩🏈⚽🚴‍♀️🎻🎼🎹🚨🚎🚐⚓🛳🚀🚁🏪🏢🖱⏰📱💾💉📉🛏🔑🔓"
            "📁🗓📊❤💯🚫🔻♠♣🕓❗🏳🏁🏳️‍🌈🇮🇹🇱🇷🇺🇸🇬🇧🇨🇳🇧🇴";
    const size_t len = strlen(text);

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Noto Color Emoji")});
    text_style.setFontSize(50);
    text_style.setColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text, len);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth - 300);
    paragraph->paint(canvas.get(), 0, 0);

    RectHeightStyle rect_height_style = RectHeightStyle::kTight;
    RectWidthStyle rect_width_style = RectWidthStyle::kTight;

    auto result = paragraph->getRectsForRange(0, 0, rect_height_style, rect_width_style);
    REPORTER_ASSERT(reporter, result.size() == 0);

    result = paragraph->getRectsForRange(0, 119, rect_height_style, rect_width_style);
    REPORTER_ASSERT(reporter, result.size() == 2);
    canvas.drawRects(SK_ColorRED, result);

    result = paragraph->getRectsForRange(122, 132, rect_height_style, rect_width_style);
    REPORTER_ASSERT(reporter, result.size() == 0);
    // We changed the selection algorithm and now the selection is empty
    canvas.drawRects(SK_ColorBLUE, result);

    auto pos = paragraph->getGlyphPositionAtCoordinate(610, 100).position;
    result = paragraph->getRectsForRange(0, pos, rect_height_style, rect_width_style);
    REPORTER_ASSERT(reporter, result.size() == 2);
    canvas.drawRects(SK_ColorGREEN, result);

    pos = paragraph->getGlyphPositionAtCoordinate(580, 100).position;
    result = paragraph->getRectsForRange(0, pos, rect_height_style, rect_width_style);
    REPORTER_ASSERT(reporter, result.size() == 2);
    canvas.drawRects(SK_ColorGREEN, result);

    pos = paragraph->getGlyphPositionAtCoordinate(560, 100).position;
    result = paragraph->getRectsForRange(0, pos, rect_height_style, rect_width_style);
    REPORTER_ASSERT(reporter, result.size() == 2);
    canvas.drawRects(SK_ColorGREEN, result);
}
