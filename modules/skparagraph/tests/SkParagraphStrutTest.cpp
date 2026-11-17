/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
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
#include "modules/skparagraph/src/TextLine.h"
#include "modules/skparagraph/tests/SkParagraphTestUtils.h"
#include "tests/Test.h"

#include <string.h>
#include <vector>

using namespace skia::textlayout;
using namespace skia::textlayout::test;

UNIX_ONLY_TEST(SkParagraph_StrutHalfLeadingSimple, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    const char* text = "A";
    const size_t len = strlen(text);

    TestCanvas canvas("SkParagraph_StrutHalfLeading.png");

    ParagraphStyle paragraph_style;
    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setFontSize(100.0f);
    text_style.setColor(SK_ColorBLACK);
    text_style.setLetterSpacing(0.0f);
    text_style.setWordSpacing(0.0f);
    text_style.setHeightOverride(true);
    text_style.setHeight(2.0f);
    text_style.setHalfLeading(true);

    StrutStyle strut_style;
    strut_style.setFontFamilies({SkString("Roboto")});
    strut_style.setFontSize(100.0f);
    strut_style.setHeightOverride(true);
    strut_style.setHeight(2.0f);
    strut_style.setHalfLeading(true);
    strut_style.setStrutEnabled(true);
    strut_style.setForceStrutHeight(true);

    paragraph_style.setStrutStyle(strut_style);
    paragraph_style.setTextStyle(text_style);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());

    builder.pushStyle(text_style);
    builder.addText(text);

    auto paragraph = builder.Build();
    paragraph->layout(550);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, impl->styles().size() == 1);  // paragraph style does not count

    paragraph->paint(canvas.get(), 0, 0);

    const RectWidthStyle rect_width_style = RectWidthStyle::kTight;
    std::vector<TextBox> boxes =
            paragraph->getRectsForRange(0, len, RectHeightStyle::kTight, rect_width_style);
    std::vector<TextBox> lineBoxes =
            paragraph->getRectsForRange(0, len, RectHeightStyle::kMax, rect_width_style);

    canvas.drawRects(SK_ColorBLUE, boxes);
    REPORTER_ASSERT(reporter, lineBoxes.size() == boxes.size());
    if (lineBoxes.size() >= 1 && boxes.size() >= 1) {
        // line spacing is distributed evenly over and under the text.
        REPORTER_ASSERT(reporter,
                        SkScalarNearlyEqual(lineBoxes[0].rect.bottom() - boxes[0].rect.bottom(),
                                            boxes[0].rect.top() - lineBoxes[0].rect.top()));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(lineBoxes[0].rect.top(), 0.0f));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(lineBoxes[0].rect.left(), 0.0f));

        std::vector<LineMetrics> lineMetrics;
        paragraph->getLineMetrics(lineMetrics);
        LineMetrics& firstLine = lineMetrics[0];
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(firstLine.fHeight, 200.0f));

        // Half leading does not move the text horizontally.
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 0, EPSILON100));
    }
}

UNIX_ONLY_TEST(SkParagraph_StrutHalfLeadingMultiline, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    const char* text = "01234満毎冠行来昼本可\nabcd\n満毎冠行来昼本可";
    const size_t len = strlen(text);

    TestCanvas canvas("SkParagraph_StrutHalfLeading.png");

    ParagraphStyle paragraph_style;
    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setFontSize(20.0f);
    text_style.setColor(SK_ColorBLACK);
    text_style.setLetterSpacing(0.0f);
    text_style.setWordSpacing(0.0f);
    text_style.setHeightOverride(true);
    text_style.setHeight(3.0f);
    text_style.setHalfLeading(true);

    StrutStyle strut_style;
    strut_style.setFontFamilies({SkString("Roboto")});
    strut_style.setFontSize(20.0f);
    strut_style.setHeightOverride(true);
    strut_style.setHeight(3.0f);
    strut_style.setHalfLeading(true);
    strut_style.setStrutEnabled(true);
    strut_style.setForceStrutHeight(true);

    paragraph_style.setStrutStyle(strut_style);
    paragraph_style.setTextStyle(text_style);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());

    builder.pushStyle(text_style);
    builder.addText(text);

    auto paragraph = builder.Build();
    paragraph->layout(550);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, impl->styles().size() == 1);  // paragraph style does not count

    paragraph->paint(canvas.get(), 0, 0);

    const RectWidthStyle rect_width_style = RectWidthStyle::kTight;
    std::vector<TextBox> boxes =
            paragraph->getRectsForRange(0, len, RectHeightStyle::kTight, rect_width_style);
    std::vector<TextBox> lineBoxes =
            paragraph->getRectsForRange(0, len, RectHeightStyle::kMax, rect_width_style);

    canvas.drawRects(SK_ColorBLUE, boxes);
    REPORTER_ASSERT(reporter, boxes.size() == 3ull);
    REPORTER_ASSERT(reporter, lineBoxes.size() == boxes.size());
    if (boxes.size() >= 3) {
        const auto line_spacing1 = boxes[1].rect.top() - boxes[0].rect.bottom();
        const auto line_spacing2 = boxes[2].rect.top() - boxes[1].rect.bottom();

        // Uniform line spacing.
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(line_spacing1, line_spacing2));

        // line spacing is distributed evenly over and under the text.
        REPORTER_ASSERT(reporter,
                        SkScalarNearlyEqual(lineBoxes[0].rect.bottom() - boxes[0].rect.bottom(),
                                            boxes[0].rect.top() - lineBoxes[0].rect.top()));
        REPORTER_ASSERT(reporter,
                        SkScalarNearlyEqual(lineBoxes[1].rect.bottom() - boxes[1].rect.bottom(),
                                            boxes[1].rect.top() - lineBoxes[1].rect.top()));
        REPORTER_ASSERT(reporter,
                        SkScalarNearlyEqual(lineBoxes[2].rect.bottom() - boxes[2].rect.bottom(),
                                            boxes[2].rect.top() - lineBoxes[2].rect.top()));

        // Half leading does not move the text horizontally.
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[1].rect.left(), 0, EPSILON100));
    }
}

// Checked: NO DIFF
UNIX_ONLY_TEST(SkParagraph_StrutParagraph1, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    TestCanvas canvas("SkParagraph_StrutParagraph1.png");
    // The chinese extra height should be absorbed by the strut.
    const char* text = "01234満毎冠p来É本可\nabcd\n満毎É行p昼本可";
    const size_t len = strlen(text);

    ParagraphStyle paragraph_style;
    paragraph_style.setMaxLines(10);
    paragraph_style.setTextAlign(TextAlign::kLeft);
    paragraph_style.turnHintingOff();

    StrutStyle strut_style;
    strut_style.setStrutEnabled(true);
    strut_style.setFontFamilies({SkString("BlahFake"), SkString("Ahem")});
    strut_style.setFontSize(50);
    strut_style.setHeight(1.8f);
    strut_style.setHeightOverride(true);
    strut_style.setLeading(0.1f);
    paragraph_style.setStrutStyle(strut_style);

    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Ahem")});
    text_style.setFontSize(50);
    text_style.setFontStyle(SkFontStyle(
            SkFontStyle::kMedium_Weight, SkFontStyle::kNormal_Width, SkFontStyle::kUpright_Slant));
    text_style.setColor(SK_ColorBLACK);
    text_style.setHeight(0.5f);
    builder.pushStyle(text_style);
    builder.addText(text, len);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(550);
    paragraph->paint(canvas.get(), 0, 0);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, impl->lines().size() == 4);

    RectHeightStyle rect_height_style = RectHeightStyle::kTight;
    RectHeightStyle rect_height_max_style = RectHeightStyle::kMax;
    RectWidthStyle rect_width_style = RectWidthStyle::kTight;
    {
        auto boxes = paragraph->getRectsForRange(0, 0, rect_height_style, rect_width_style);
        REPORTER_ASSERT(reporter, boxes.empty());
    }
    {
        auto boxes = paragraph->getRectsForRange(0, 1, rect_height_style, rect_width_style);
        canvas.drawRects(SK_ColorRED, boxes);
        REPORTER_ASSERT(reporter, boxes.size() == 1);
        if (boxes.size() >= 1) {
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 0, EPSILON100));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 34.5f, EPSILON100));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 50, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(boxes[0].rect.bottom(), 84.5f, EPSILON100));
        }
    }
    {
        auto boxes = paragraph->getRectsForRange(0, 1, rect_height_max_style, rect_width_style);
        canvas.drawRects(SK_ColorRED, boxes);
        REPORTER_ASSERT(reporter, boxes.size() == 1);
        if (boxes.size() >= 1) {
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 0, EPSILON100));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 0, EPSILON100));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 50, EPSILON100));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 95, EPSILON100));
        }
    }
    {
        auto boxes = paragraph->getRectsForRange(6, 10, rect_height_style, rect_width_style);
        canvas.drawRects(SK_ColorRED, boxes);
        REPORTER_ASSERT(reporter, boxes.size() == 1);
        if (boxes.size() >= 1) {
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 300, EPSILON100));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 34.5f, EPSILON100));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 500, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(boxes[0].rect.bottom(), 84.5f, EPSILON100));
        }
    }
    {
        auto boxes = paragraph->getRectsForRange(6, 10, rect_height_max_style, rect_width_style);
        canvas.drawRects(SK_ColorRED, boxes);
        REPORTER_ASSERT(reporter, boxes.size() == 1);
        if (boxes.size() >= 1) {
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 300, EPSILON100));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 0, EPSILON100));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 500, EPSILON100));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 95, EPSILON100));
        }
    }
    {
        auto boxes = paragraph->getRectsForRange(14, 16, rect_height_max_style, rect_width_style);
        canvas.drawRects(SK_ColorRED, boxes);
        REPORTER_ASSERT(reporter, boxes.size() == 1);
        if (boxes.size() >= 1) {
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 0, EPSILON100));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 190, EPSILON100));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 100, EPSILON100));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 285, EPSILON100));
        }
    }
    {
        auto boxes = paragraph->getRectsForRange(20, 25, rect_height_max_style, rect_width_style);
        canvas.drawRects(SK_ColorRED, boxes);
        REPORTER_ASSERT(reporter, boxes.size() == 1);
        if (boxes.size() >= 1) {
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 50, EPSILON100));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 285, EPSILON100));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 300, EPSILON100));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 380, EPSILON100));
        }
    }
}

// Checked: NO DIFF
UNIX_ONLY_TEST(SkParagraph_StrutParagraph2, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    TestCanvas canvas("SkParagraph_StrutParagraph2.png");
    // The chinese extra height should be absorbed by the strut.
    const char* text = "01234ABCDEFGH\nabcd\nABCDEFGH";
    const size_t len = strlen(text);

    ParagraphStyle paragraph_style;
    paragraph_style.setMaxLines(10);
    paragraph_style.setTextAlign(TextAlign::kLeft);
    paragraph_style.turnHintingOff();

    StrutStyle strut_style;

    strut_style.setStrutEnabled(true);
    strut_style.setFontFamilies({SkString("Ahem")});
    strut_style.setFontSize(50);
    strut_style.setHeight(1.6f);
    strut_style.setHeightOverride(true);
    paragraph_style.setStrutStyle(strut_style);

    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Ahem")});
    text_style.setFontSize(50);
    text_style.setFontStyle(SkFontStyle(
            SkFontStyle::kMedium_Weight, SkFontStyle::kNormal_Width, SkFontStyle::kUpright_Slant));
    text_style.setColor(SK_ColorBLACK);
    text_style.setHeight(1);
    builder.pushStyle(text_style);
    builder.addText(text, len);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(550);
    paragraph->paint(canvas.get(), 0, 0);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    // Font is not resolved and the first line does not fit
    REPORTER_ASSERT(reporter, impl->lines().size() == 4);

    RectHeightStyle rect_height_style = RectHeightStyle::kTight;
    RectHeightStyle rect_height_max_style = RectHeightStyle::kMax;
    RectWidthStyle rect_width_style = RectWidthStyle::kTight;
    {
        auto boxes = paragraph->getRectsForRange(0, 0, rect_height_style, rect_width_style);
        REPORTER_ASSERT(reporter, boxes.empty());
    }
    {
        auto boxes = paragraph->getRectsForRange(0, 1, rect_height_style, rect_width_style);
        canvas.drawRects(SK_ColorRED, boxes);
        REPORTER_ASSERT(reporter, boxes.size() == 1);
        if (boxes.size() >= 1) {
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 0, EPSILON100));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 24, EPSILON100));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 50, EPSILON100));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 74, EPSILON100));
        }
    }
    {
        auto boxes = paragraph->getRectsForRange(0, 1, rect_height_max_style, rect_width_style);
        canvas.drawRects(SK_ColorRED, boxes);
        REPORTER_ASSERT(reporter, boxes.size() == 1);
        if (boxes.size() >= 1) {
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 0, EPSILON100));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 0, EPSILON100));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 50, EPSILON100));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 80, EPSILON100));
        }
    }
    {
        auto boxes = paragraph->getRectsForRange(6, 10, rect_height_style, rect_width_style);
        canvas.drawRects(SK_ColorRED, boxes);
        REPORTER_ASSERT(reporter, boxes.size() == 1);
        if (boxes.size() >= 1) {
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 300, EPSILON100));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 24, EPSILON100));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 500, EPSILON100));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 74, EPSILON100));
        }
    }
    {
        auto boxes = paragraph->getRectsForRange(6, 10, rect_height_max_style, rect_width_style);
        canvas.drawRects(SK_ColorRED, boxes);
        REPORTER_ASSERT(reporter, boxes.size() == 1);
        if (boxes.size() >= 1) {
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 300, EPSILON100));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 0, EPSILON100));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 500, EPSILON100));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 80, EPSILON100));
        }
    }
    {
        auto boxes = paragraph->getRectsForRange(14, 16, rect_height_max_style, rect_width_style);
        canvas.drawRects(SK_ColorRED, boxes);
        REPORTER_ASSERT(reporter, boxes.size() == 1);
        if (boxes.size() >= 1) {
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 0, EPSILON100));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 160, EPSILON100));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 100, EPSILON100));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 240, EPSILON100));
        }
    }
    {
        auto boxes = paragraph->getRectsForRange(20, 25, rect_height_max_style, rect_width_style);
        canvas.drawRects(SK_ColorRED, boxes);
        REPORTER_ASSERT(reporter, boxes.size() == 1);
        if (boxes.size() >= 1) {
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 50, EPSILON100));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 240, EPSILON100));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 300, EPSILON100));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 320, EPSILON100));
        }
    }
}

// Checked: NO DIFF
UNIX_ONLY_TEST(SkParagraph_StrutParagraph3, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    TestCanvas canvas("SkParagraph_StrutParagraph3.png");

    // The chinese extra height should be absorbed by the strut.
    const char* text = "01234満毎p行来昼本可\nabcd\n満毎冠行来昼本可";
    const size_t len = strlen(text);

    ParagraphStyle paragraph_style;
    paragraph_style.setMaxLines(10);
    paragraph_style.setTextAlign(TextAlign::kLeft);
    paragraph_style.turnHintingOff();

    StrutStyle strut_style;
    strut_style.setStrutEnabled(true);
    strut_style.setFontFamilies({SkString("Ahem")});
    strut_style.setFontSize(50);
    strut_style.setHeight(1.2f);
    strut_style.setHeightOverride(true);
    paragraph_style.setStrutStyle(strut_style);

    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Ahem")});
    text_style.setFontSize(50);
    text_style.setFontStyle(SkFontStyle(
            SkFontStyle::kMedium_Weight, SkFontStyle::kNormal_Width, SkFontStyle::kUpright_Slant));
    text_style.setColor(SK_ColorBLACK);
    text_style.setHeight(1);
    builder.pushStyle(text_style);
    builder.addText(text, len);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(550);
    paragraph->paint(canvas.get(), 0, 0);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    // Font is not resolved and the first line does not fit
    REPORTER_ASSERT(reporter, impl->lines().size() == 4);

    RectHeightStyle rect_height_style = RectHeightStyle::kTight;
    RectHeightStyle rect_height_max_style = RectHeightStyle::kMax;
    RectWidthStyle rect_width_style = RectWidthStyle::kTight;
    SkScalar epsilon = 0.001f;
    {
        auto boxes = paragraph->getRectsForRange(0, 0, rect_height_style, rect_width_style);
        REPORTER_ASSERT(reporter, boxes.empty());
    }
    {
        auto boxes = paragraph->getRectsForRange(0, 1, rect_height_style, rect_width_style);
        canvas.drawRects(SK_ColorRED, boxes);
        REPORTER_ASSERT(reporter, boxes.size() == 1);
        if (boxes.size() >= 1) {
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 0, epsilon));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 8, epsilon));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 50, epsilon));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 58, epsilon));
        }
    }
    {
        auto boxes = paragraph->getRectsForRange(0, 1, rect_height_max_style, rect_width_style);
        canvas.drawRects(SK_ColorRED, boxes);
        REPORTER_ASSERT(reporter, boxes.size() == 1);
        if (boxes.size() >= 1) {
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 0, epsilon));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 0, epsilon));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 50, epsilon));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 60, epsilon));
        }
    }
    {
        auto boxes = paragraph->getRectsForRange(6, 10, rect_height_style, rect_width_style);
        canvas.drawRects(SK_ColorRED, boxes);
        REPORTER_ASSERT(reporter, boxes.size() == 1);
        if (boxes.size() >= 1) {
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 300, epsilon));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 8, epsilon));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 500, epsilon));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 58, epsilon));
        }
    }
    {
        auto boxes = paragraph->getRectsForRange(6, 10, rect_height_max_style, rect_width_style);
        canvas.drawRects(SK_ColorRED, boxes);
        REPORTER_ASSERT(reporter, boxes.size() == 1);
        if (boxes.size() >= 1) {
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 300, epsilon));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 0, epsilon));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 500, epsilon));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 60, epsilon));
        }
    }
    {
        auto boxes = paragraph->getRectsForRange(14, 16, rect_height_max_style, rect_width_style);
        canvas.drawRects(SK_ColorRED, boxes);
        REPORTER_ASSERT(reporter, boxes.size() == 1);
        if (boxes.size() >= 1) {
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 0, epsilon));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 120, epsilon));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 100, epsilon));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 180, epsilon));
        }
    }
    {
        auto boxes = paragraph->getRectsForRange(20, 25, rect_height_max_style, rect_width_style);
        canvas.drawRects(SK_ColorRED, boxes);
        REPORTER_ASSERT(reporter, boxes.size() == 1);
        if (boxes.size() >= 1) {
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 50, epsilon));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 180, epsilon));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 300, epsilon));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 240, epsilon));
        }
    }
}

// Checked: NO DIFF
UNIX_ONLY_TEST(SkParagraph_StrutForceParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    TestCanvas canvas("SkParagraph_StrutForceParagraph.png");
    const char* text = "01234満毎冠行来昼本可\nabcd\n満毎冠行来昼本可";
    const size_t len = strlen(text);

    ParagraphStyle paragraph_style;
    paragraph_style.setMaxLines(10);
    paragraph_style.setTextAlign(TextAlign::kLeft);
    paragraph_style.turnHintingOff();

    StrutStyle strut_style;
    strut_style.setStrutEnabled(true);
    strut_style.setFontFamilies({SkString("Ahem")});
    strut_style.setFontSize(50);
    strut_style.setHeight(1.5f);
    strut_style.setHeightOverride(true);
    strut_style.setLeading(0.1f);
    strut_style.setForceStrutHeight(true);
    paragraph_style.setStrutStyle(strut_style);

    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Ahem")});
    text_style.setFontSize(50);
    text_style.setLetterSpacing(0);
    text_style.setColor(SK_ColorBLACK);
    text_style.setHeight(1);
    builder.pushStyle(text_style);
    builder.addText(text, len);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(550);
    paragraph->paint(canvas.get(), 0, 0);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    // Font is not resolved and the first line does not fit
    REPORTER_ASSERT(reporter, impl->lines().size() == 4);

    RectHeightStyle rect_height_style = RectHeightStyle::kTight;
    RectHeightStyle rect_height_max_style = RectHeightStyle::kMax;
    RectWidthStyle rect_width_style = RectWidthStyle::kTight;

    auto boxes1 = paragraph->getRectsForRange(0, 0, rect_height_style, rect_width_style);
    REPORTER_ASSERT(reporter, boxes1.empty());

    auto boxes2 = paragraph->getRectsForRange(0, 1, rect_height_style, rect_width_style);
    canvas.drawRects(SK_ColorRED, boxes2);
    REPORTER_ASSERT(reporter, boxes2.size() == 1);
    if (boxes2.size() >= 1) {
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes2[0].rect.left(), 0, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes2[0].rect.top(), 22.5f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes2[0].rect.right(), 50, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes2[0].rect.bottom(), 72.5f, EPSILON100));
    }

    auto boxes3 = paragraph->getRectsForRange(0, 1, rect_height_max_style, rect_width_style);
    canvas.drawRects(SK_ColorRED, boxes3);
    REPORTER_ASSERT(reporter, boxes3.size() == 1);
    if (boxes3.size() >= 1) {
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes3[0].rect.left(), 0, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes3[0].rect.top(), 0, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes3[0].rect.right(), 50, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes3[0].rect.bottom(), 80, EPSILON100));
    }

    auto boxes4 = paragraph->getRectsForRange(6, 10, rect_height_style, rect_width_style);
    canvas.drawRects(SK_ColorRED, boxes4);
    REPORTER_ASSERT(reporter, boxes4.size() == 1);
    if (boxes4.size() >= 1) {
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes4[0].rect.left(), 300, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes4[0].rect.top(), 22.5f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes4[0].rect.right(), 500, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes4[0].rect.bottom(), 72.5f, EPSILON100));
    }

    auto boxes5 = paragraph->getRectsForRange(6, 10, rect_height_max_style, rect_width_style);
    canvas.drawRects(SK_ColorRED, boxes5);
    REPORTER_ASSERT(reporter, boxes5.size() == 1);
    if (boxes5.size() >= 1) {
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes5[0].rect.left(), 300, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes5[0].rect.top(), 0, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes5[0].rect.right(), 500, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes5[0].rect.bottom(), 80, EPSILON100));
    }

    auto boxes6 = paragraph->getRectsForRange(14, 16, rect_height_max_style, rect_width_style);
    canvas.drawRects(SK_ColorRED, boxes6);
    REPORTER_ASSERT(reporter, boxes6.size() == 1);
    if (boxes6.size() >= 1) {
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes6[0].rect.left(), 0, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes6[0].rect.top(), 160, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes6[0].rect.right(), 100, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes6[0].rect.bottom(), 240, EPSILON100));
    }

    auto boxes7 = paragraph->getRectsForRange(20, 25, rect_height_max_style, rect_width_style);
    canvas.drawRects(SK_ColorRED, boxes7);
    REPORTER_ASSERT(reporter, boxes7.size() == 1);
    if (boxes7.size() >= 1) {
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes7[0].rect.left(), 50, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes7[0].rect.top(), 240, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes7[0].rect.right(), 300, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes7[0].rect.bottom(), 320, EPSILON100));
    }
}

// Checked: NO DIFF
UNIX_ONLY_TEST(SkParagraph_StrutDefaultParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    TestCanvas canvas("SkParagraph_StrutDefaultParagraph.png");

    const char* text = "01234満毎冠行来昼本可\nabcd\n満毎冠行来昼本可";
    const size_t len = strlen(text);

    ParagraphStyle paragraph_style;
    paragraph_style.setMaxLines(10);
    paragraph_style.setTextAlign(TextAlign::kLeft);
    paragraph_style.turnHintingOff();

    StrutStyle strut_style;
    strut_style.setStrutEnabled(true);
    strut_style.setFontFamilies({SkString("Ahem")});
    strut_style.setFontSize(50);
    strut_style.setHeight(1.5f);
    strut_style.setLeading(0.1f);
    strut_style.setForceStrutHeight(false);
    paragraph_style.setStrutStyle(strut_style);

    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Ahem")});
    text_style.setFontSize(20);
    text_style.setColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text, len);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(550);
    paragraph->paint(canvas.get(), 0, 0);

    RectHeightStyle rect_height_style = RectHeightStyle::kTight;
    RectHeightStyle rect_height_strut_style = RectHeightStyle::kStrut;
    RectWidthStyle rect_width_style = RectWidthStyle::kTight;
    {
        auto boxes = paragraph->getRectsForRange(0, 0, rect_height_style, rect_width_style);
        REPORTER_ASSERT(reporter, boxes.empty());
    }
    {
        auto boxes = paragraph->getRectsForRange(0, 1, rect_height_style, rect_width_style);
        canvas.drawRects(SK_ColorRED, boxes);
        REPORTER_ASSERT(reporter, boxes.size() == 1);
        if (boxes.size() >= 1) {
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 0, EPSILON100));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 26.5f, EPSILON100));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 20, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(boxes[0].rect.bottom(), 46.5f, EPSILON100));
        }
    }
    {
        auto boxes = paragraph->getRectsForRange(0, 2, rect_height_strut_style, rect_width_style);
        canvas.drawRects(SK_ColorRED, boxes);
        REPORTER_ASSERT(reporter, boxes.size() == 1);
        if (boxes.size() >= 1) {
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 0, EPSILON100));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 2.5f, EPSILON100));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 40, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(boxes[0].rect.bottom(), 52.5f, EPSILON100));
        }
    }
}

UNIX_ONLY_TEST(SkParagraph_StrutTopLine, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    TestCanvas canvas("SkParagraph_StrutTopLine.png");

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Ahem")});
    text_style.setFontSize(10);
    SkPaint black;
    black.setColor(SK_ColorBLACK);
    text_style.setForegroundColor(black);

    ParagraphStyle paragraph_style;
    paragraph_style.setTextStyle(text_style);
    paragraph_style.setTextDirection(TextDirection::kLtr);
    StrutStyle strut_style;
    strut_style.setStrutEnabled(true);
    strut_style.setFontFamilies({SkString("Ahem")});
    strut_style.setFontSize(16);
    strut_style.setHeight(4.0f);
    strut_style.setHeightOverride(true);
    strut_style.setLeading(-1.0f);
    strut_style.setForceStrutHeight(true);
    paragraph_style.setStrutStyle(strut_style);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());

    builder.pushStyle(text_style);
    builder.addText(u"Atwater Peel Sherbrooke Bonaventure\nhi\nwasssup!");

    auto paragraph = builder.Build();
    paragraph->layout(797);
    paragraph->paint(canvas.get(), 0, 0);
    auto boxes = paragraph->getRectsForRange(
            0, 60, RectHeightStyle::kIncludeLineSpacingTop, RectWidthStyle::kMax);
    REPORTER_ASSERT(reporter, boxes.size() == 4);
    if (boxes.size() >= 4) {
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.fTop, 38.4f));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.fBottom, 64.0f));

        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[1].rect.fTop, 64.0f));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[1].rect.fBottom, 128.0f));

        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[2].rect.fTop, 64.0f));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[2].rect.fBottom, 128.0f));

        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[3].rect.fTop, 128.0f));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[3].rect.fBottom, 192.0f));
    }
}

// This test does not produce an image
UNIX_ONLY_TEST(SkParagraph_StrutAndTextBehavior, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    const char* text = "";
    const size_t len = strlen(text);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Ahem")});
    text_style.setFontSize(16.0);
    text_style.setColor(SK_ColorBLACK);
    StrutStyle strut_style;
    strut_style.setStrutEnabled(true);
    strut_style.setForceStrutHeight(true);
    strut_style.setHeight(1.5);
    strut_style.setHeightOverride(true);
    strut_style.setFontFamilies({SkString("Ahem")});
    strut_style.setFontSize(16.0);
    ParagraphStyle paragraph_style;
    paragraph_style.setStrutStyle(strut_style);
    paragraph_style.setTextStyle(text_style);

    auto layout = [&](TextHeightBehavior tb) {
        paragraph_style.setTextHeightBehavior(tb);
        ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());
        builder.pushStyle(text_style);
        builder.addText(text, len);
        auto paragraph = builder.Build();
        paragraph->layout(SK_ScalarInfinity);
        return paragraph->getHeight();
    };

    auto height1 = layout(TextHeightBehavior::kDisableAll);
    auto height2 = layout(TextHeightBehavior::kAll);

    // Regardless of TextHeightBehavior strut sets the line height
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(height1, 24.0f));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(height2, 24.0f));
}
