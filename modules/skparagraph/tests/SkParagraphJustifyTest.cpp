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
#include <memory>
#include <string>
#include <utility>
#include <vector>

using namespace skia::textlayout;
using namespace skia::textlayout::test;

UNIX_ONLY_TEST(SkParagraph_LeftAlignParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    TestCanvas canvas("SkParagraph_LeftAlignParagraph.png");
    const char* text =
            "This is a very long sentence to test if the text will properly wrap "
            "around and go to the next line. Sometimes, short sentence. Longer "
            "sentences are okay too because they are nessecary. Very short. "
            "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
            "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim "
            "veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea "
            "commodo consequat. Duis aute irure dolor in reprehenderit in voluptate "
            "velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint "
            "occaecat cupidatat non proident, sunt in culpa qui officia deserunt "
            "mollit anim id est laborum. "
            "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
            "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim "
            "veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea "
            "commodo consequat. Duis aute irure dolor in reprehenderit in voluptate "
            "velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint "
            "occaecat cupidatat non proident, sunt in culpa qui officia deserunt "
            "mollit anim id est laborum.";
    const size_t len = strlen(text);

    ParagraphStyle paragraph_style;
    paragraph_style.setMaxLines(14);
    paragraph_style.setTextAlign(TextAlign::kLeft);
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setFontSize(26);
    text_style.setLetterSpacing(1);
    text_style.setWordSpacing(5);
    text_style.setColor(SK_ColorBLACK);
    text_style.setHeight(1);
    text_style.setDecoration(TextDecoration::kUnderline);
    text_style.setDecorationColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text, len);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth - 100);
    paragraph->paint(canvas.get(), 0, 0);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());

    REPORTER_ASSERT(reporter, impl->text().size() == std::string{text}.length());
    REPORTER_ASSERT(reporter, impl->runs().size() == 1);
    REPORTER_ASSERT(reporter, impl->styles().size() == 1);
    REPORTER_ASSERT(reporter, impl->styles()[0].fStyle.equals(text_style));
    REPORTER_ASSERT(reporter, impl->lines().size() == paragraph_style.getMaxLines());

    double expected_y = 0;
    double epsilon = 0.01f;
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(impl->lines()[0].baseline(), 24.121f, epsilon));
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(impl->lines()[0].offset().fY, expected_y, epsilon));
    expected_y += 30;
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(impl->lines()[1].offset().fY, expected_y, epsilon));
    expected_y += 30;
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(impl->lines()[2].offset().fY, expected_y, epsilon));
    expected_y += 30;
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(impl->lines()[3].offset().fY, expected_y, epsilon));
    expected_y += 30 * 10;
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(impl->lines()[13].offset().fY, expected_y, epsilon));

    REPORTER_ASSERT(reporter,
                    paragraph_style.getTextAlign() == impl->paragraphStyle().getTextAlign());

    // Tests for GetGlyphPositionAtCoordinate()
    REPORTER_ASSERT(reporter, impl->getGlyphPositionAtCoordinate(0, 0).position == 0);
    REPORTER_ASSERT(reporter, impl->getGlyphPositionAtCoordinate(1, 1).position == 0);
    REPORTER_ASSERT(reporter, impl->getGlyphPositionAtCoordinate(1, 35).position == 68);
    REPORTER_ASSERT(reporter, impl->getGlyphPositionAtCoordinate(1, 70).position == 134);
    REPORTER_ASSERT(reporter, impl->getGlyphPositionAtCoordinate(2000, 35).position == 134);
}

UNIX_ONLY_TEST(SkParagraph_RightAlignParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    TestCanvas canvas("SkParagraph_RightAlignParagraph.png");
    const char* text =
            "This is a very long sentence to test if the text will properly wrap "
            "around and go to the next line. Sometimes, short sentence. Longer "
            "sentences are okay too because they are nessecary. Very short. "
            "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
            "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim "
            "veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea "
            "commodo consequat. Duis aute irure dolor in reprehenderit in voluptate "
            "velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint "
            "occaecat cupidatat non proident, sunt in culpa qui officia deserunt "
            "mollit anim id est laborum. "
            "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
            "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim "
            "veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea "
            "commodo consequat. Duis aute irure dolor in reprehenderit in voluptate "
            "velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint "
            "occaecat cupidatat non proident, sunt in culpa qui officia deserunt "
            "mollit anim id est laborum.";
    const size_t len = strlen(text);

    ParagraphStyle paragraph_style;
    paragraph_style.setMaxLines(14);
    paragraph_style.setTextAlign(TextAlign::kRight);
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setFontSize(26);
    text_style.setLetterSpacing(1);
    text_style.setWordSpacing(5);
    text_style.setColor(SK_ColorBLACK);
    text_style.setHeight(1);
    text_style.setDecoration(TextDecoration::kUnderline);
    text_style.setDecorationColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text, len);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth - 100);

    paragraph->paint(canvas.get(), 0, 0);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());

    REPORTER_ASSERT(reporter, impl->runs().size() == 1);
    REPORTER_ASSERT(reporter, impl->styles().size() == 1);
    REPORTER_ASSERT(reporter, impl->styles()[0].fStyle.equals(text_style));
    REPORTER_ASSERT(reporter, impl->lines().size() == paragraph_style.getMaxLines());

    double expected_y = 0;
    double epsilon = 0.01f;
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(impl->lines()[0].baseline(), 24.121f, epsilon));
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(impl->lines()[0].offset().fY, expected_y, epsilon));
    expected_y += 30;
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(impl->lines()[1].offset().fY, expected_y, epsilon));
    expected_y += 30;
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(impl->lines()[2].offset().fY, expected_y, epsilon));
    expected_y += 30;
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(impl->lines()[3].offset().fY, expected_y, epsilon));
    expected_y += 30 * 10;
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(impl->lines()[13].offset().fY, expected_y, epsilon));

    auto calculate = [](const TextLine& line) -> SkScalar {
        return TestCanvasWidth - 100 - line.offset().fX - line.width();
    };

    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(calculate(impl->lines()[0]), 0, epsilon));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(calculate(impl->lines()[1]), 0, epsilon));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(calculate(impl->lines()[2]), 0, epsilon));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(calculate(impl->lines()[3]), 0, epsilon));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(calculate(impl->lines()[13]), 0, epsilon));

    REPORTER_ASSERT(reporter,
                    paragraph_style.getTextAlign() == impl->paragraphStyle().getTextAlign());
}

UNIX_ONLY_TEST(SkParagraph_CenterAlignParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    TestCanvas canvas("SkParagraph_CenterAlignParagraph.png");
    const char* text =
            "This is a very long sentence to test if the text will properly wrap "
            "around and go to the next line. Sometimes, short sentence. Longer "
            "sentences are okay too because they are nessecary. Very short. "
            "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
            "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim "
            "veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea "
            "commodo consequat. Duis aute irure dolor in reprehenderit in voluptate "
            "velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint "
            "occaecat cupidatat non proident, sunt in culpa qui officia deserunt "
            "mollit anim id est laborum. "
            "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
            "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim "
            "veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea "
            "commodo consequat. Duis aute irure dolor in reprehenderit in voluptate "
            "velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint "
            "occaecat cupidatat non proident, sunt in culpa qui officia deserunt "
            "mollit anim id est laborum.";
    const size_t len = strlen(text);

    ParagraphStyle paragraph_style;
    paragraph_style.setMaxLines(14);
    paragraph_style.setTextAlign(TextAlign::kCenter);
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setFontSize(26);
    text_style.setLetterSpacing(1);
    text_style.setWordSpacing(5);
    text_style.setColor(SK_ColorBLACK);
    text_style.setHeight(1);
    text_style.setDecoration(TextDecoration::kUnderline);
    text_style.setDecorationColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text, len);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth - 100);
    paragraph->paint(canvas.get(), 0, 0);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());

    REPORTER_ASSERT(reporter, impl->text().size() == std::string{text}.length());
    REPORTER_ASSERT(reporter, impl->runs().size() == 1);
    REPORTER_ASSERT(reporter, impl->styles().size() == 1);
    REPORTER_ASSERT(reporter, impl->styles()[0].fStyle.equals(text_style));
    REPORTER_ASSERT(reporter, impl->lines().size() == paragraph_style.getMaxLines());

    double expected_y = 0;
    double epsilon = 0.01f;
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(impl->lines()[0].baseline(), 24.121f, epsilon));
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(impl->lines()[0].offset().fY, expected_y, epsilon));
    expected_y += 30;
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(impl->lines()[1].offset().fY, expected_y, epsilon));
    expected_y += 30;
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(impl->lines()[2].offset().fY, expected_y, epsilon));
    expected_y += 30;
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(impl->lines()[3].offset().fY, expected_y, epsilon));
    expected_y += 30 * 10;
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(impl->lines()[13].offset().fY, expected_y, epsilon));

    auto calculate = [](const TextLine& line) -> SkScalar {
        return TestCanvasWidth - 100 - (line.offset().fX * 2 + line.width());
    };

    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(calculate(impl->lines()[0]), 0, epsilon));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(calculate(impl->lines()[1]), 0, epsilon));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(calculate(impl->lines()[2]), 0, epsilon));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(calculate(impl->lines()[3]), 0, epsilon));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(calculate(impl->lines()[13]), 0, epsilon));

    REPORTER_ASSERT(reporter,
                    paragraph_style.getTextAlign() == impl->paragraphStyle().getTextAlign());
}

UNIX_ONLY_TEST(SkParagraph_JustifyAlignParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    TestCanvas canvas("SkParagraph_JustifyAlignParagraph.png");
    const char* text =
            "This is a very long sentence to test if the text will properly wrap "
            "around and go to the next line. Sometimes, short sentence. Longer "
            "sentences are okay too because they are nessecary. Very short. "
            "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
            "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim "
            "veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea "
            "commodo consequat. Duis aute irure dolor in reprehenderit in voluptate "
            "velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint "
            "occaecat cupidatat non proident, sunt in culpa qui officia deserunt "
            "mollit anim id est laborum. "
            "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
            "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim "
            "veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea "
            "commodo consequat. Duis aute irure dolor in reprehenderit in voluptate "
            "velit esse cillum dolore eu fugiat.";
    const size_t len = strlen(text);

    ParagraphStyle paragraph_style;
    paragraph_style.setMaxLines(14);
    paragraph_style.setTextAlign(TextAlign::kJustify);
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setFontSize(26);
    text_style.setLetterSpacing(0);
    text_style.setWordSpacing(5);
    text_style.setColor(SK_ColorBLACK);
    text_style.setHeight(1);
    text_style.setDecoration(TextDecoration::kUnderline);
    text_style.setDecorationColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text, len);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth - 100);
    paragraph->paint(canvas.get(), 0, 0);

    RectHeightStyle rect_height_style = RectHeightStyle::kMax;
    RectWidthStyle rect_width_style = RectWidthStyle::kTight;
    auto boxes = paragraph->getRectsForRange(0, 100, rect_height_style, rect_width_style);
    canvas.drawRects(SK_ColorRED, boxes);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());

    REPORTER_ASSERT(reporter, impl->text().size() == std::string{text}.length());
    REPORTER_ASSERT(reporter, impl->runs().size() == 1);
    REPORTER_ASSERT(reporter, impl->styles().size() == 1);
    REPORTER_ASSERT(reporter, impl->styles()[0].fStyle.equals(text_style));

    double expected_y = 0;
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(impl->lines()[0].baseline(), 24.121f, EPSILON100));
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(impl->lines()[0].offset().fY, expected_y, EPSILON100));
    expected_y += 30;
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(impl->lines()[1].offset().fY, expected_y, EPSILON100));
    expected_y += 30;
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(impl->lines()[2].offset().fY, expected_y, EPSILON100));
    expected_y += 30;
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(impl->lines()[3].offset().fY, expected_y, EPSILON100));
    expected_y += 30 * 9;
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(impl->lines()[12].offset().fY, expected_y, EPSILON100));

    auto calculate = [](const TextLine& line) -> SkScalar { return line.offset().fX; };

    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(calculate(impl->lines()[0]), 0, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(calculate(impl->lines()[1]), 0, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(calculate(impl->lines()[2]), 0, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(calculate(impl->lines()[3]), 0, EPSILON100));

    REPORTER_ASSERT(reporter,
                    paragraph_style.getTextAlign() == impl->paragraphStyle().getTextAlign());
}

// Checked: DIFF (ghost spaces as a separate box in TxtLib)
UNIX_ONLY_TEST(SkParagraph_JustifyRTL, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>(true);
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    TestCanvas canvas("SkParagraph_JustifyRTL.png");
    const char* text =
            "אאא בּבּבּבּ אאאא בּבּ אאא בּבּבּ אאאאא בּבּבּבּ אאאא בּבּבּבּבּ "
            "אאאאא בּבּבּבּבּ אאאבּבּבּבּבּבּאאאאא בּבּבּבּבּבּאאאאאבּבּבּבּבּבּ אאאאא בּבּבּבּבּ "
            "אאאאא בּבּבּבּבּבּ אאאאא בּבּבּבּבּבּ אאאאא בּבּבּבּבּבּ אאאאא בּבּבּבּבּבּ "
            "אאאאאבּבּבּבּבּבּאאאאאבּבּבּבּבּבּאאאאאבּבּבּבּבּבּ "
            "אאאאא בּבּבּבּבּבּ";
    const size_t len = strlen(text);

    ParagraphStyle paragraph_style;
    paragraph_style.setMaxLines(14);
    paragraph_style.setTextAlign(TextAlign::kJustify);
    paragraph_style.setTextDirection(TextDirection::kRtl);
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Ahem")});
    text_style.setFontSize(26);
    text_style.setColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text, len);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth - 100);
    paragraph->paint(canvas.get(), 0, 0);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());

    auto calculate = [](const TextLine& line) -> SkScalar {
        return TestCanvasWidth - 100 - line.width();
    };
    for (auto& line : impl->lines()) {
        if (&line == &impl->lines().back() || &line == &impl->lines()[impl->lines().size() - 2]) {
            // Second-last line will be also right-aligned because it is only one cluster
            REPORTER_ASSERT(reporter, calculate(line) > EPSILON100);
            REPORTER_ASSERT(reporter, line.offset().fX > EPSILON100);
        } else {
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(calculate(line), 0, EPSILON100));
        }
    }

    // Just make sure the the text is actually RTL
    for (auto& run : impl->runs()) {
        REPORTER_ASSERT(reporter, !run.leftToRight());
    }

    // Tests for GetRectsForRange()
    RectHeightStyle rect_height_style = RectHeightStyle::kMax;
    RectWidthStyle rect_width_style = RectWidthStyle::kTight;
    auto boxes = paragraph->getRectsForRange(0, 100, rect_height_style, rect_width_style);
    canvas.drawRects(SK_ColorRED, boxes);
    REPORTER_ASSERT(reporter, boxes.size() == 3);

    boxes = paragraph->getRectsForRange(226, 278, rect_height_style, rect_width_style);
    canvas.drawRects(SK_ColorYELLOW, boxes);
    REPORTER_ASSERT(reporter, boxes.size() == 1);

    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 16, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 130, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 900, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 156, EPSILON100));

    boxes = paragraph->getRectsForRange(292, 296, rect_height_style, rect_width_style);
    canvas.drawRects(SK_ColorBLUE, boxes);
    REPORTER_ASSERT(reporter, boxes.size() == 1);

    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 588, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 156, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 640, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 182, EPSILON100));
}

UNIX_ONLY_TEST(SkParagraph_JustifyRTLNewLine, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>(true);
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    TestCanvas canvas("SkParagraph_JustifyRTLNewLine.png");
    const char* text =
            "אאא בּבּבּבּ אאאא\nבּבּ אאא בּבּבּ אאאאא בּבּבּבּ אאאא בּבּבּבּבּ "
            "אאאאא בּבּבּבּבּ אאאבּבּבּבּבּבּאאאאא בּבּבּבּבּבּאאאאאבּבּבּבּבּבּ אאאאא בּבּבּבּבּ "
            "אאאאא בּבּבּבּבּבּ אאאאא בּבּבּבּבּבּ אאאאא בּבּבּבּבּבּ אאאאא בּבּבּבּבּבּ אאאאא "
            "בּבּבּבּבּבּ";
    const size_t len = strlen(text);

    ParagraphStyle paragraph_style;
    paragraph_style.setMaxLines(14);
    paragraph_style.setTextAlign(TextAlign::kJustify);
    paragraph_style.setTextDirection(TextDirection::kRtl);
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Ahem")});
    text_style.setFontSize(26);
    text_style.setColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text, len);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth - 100);
    paragraph->paint(canvas.get(), 0, 0);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());

    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setAntiAlias(true);
    paint.setStrokeWidth(1);

    // Tests for GetRectsForRange()
    RectHeightStyle rect_height_style = RectHeightStyle::kMax;
    RectWidthStyle rect_width_style = RectWidthStyle::kTight;
    paint.setColor(SK_ColorRED);
    auto boxes = paragraph->getRectsForRange(0, 30, rect_height_style, rect_width_style);
    for (size_t i = 0; i < boxes.size(); ++i) {
        canvas.get()->drawRect(boxes[i].rect, paint);
    }
    REPORTER_ASSERT(reporter, boxes.size() == 2ull);
    if (boxes.size() >= 2) {
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 562, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 0, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 900, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 26, EPSILON100));
    }

    paint.setColor(SK_ColorBLUE);
    boxes = paragraph->getRectsForRange(240, 250, rect_height_style, rect_width_style);
    for (size_t i = 0; i < boxes.size(); ++i) {
        canvas.get()->drawRect(boxes[i].rect, paint);
    }
    REPORTER_ASSERT(reporter, boxes.size() == 1ull);
    if (boxes.size() >= 1) {
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 68, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 130, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 120, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 156, EPSILON100));
    }

    // All lines should be justified to the width of the paragraph
    // except for #0 (new line) and #5 (the last one)
    for (auto& line : impl->lines()) {
        ptrdiff_t num = &line - impl->lines().data();
        if (num == 0 || num == 5) {
            REPORTER_ASSERT(reporter, line.width() < TestCanvasWidth - 100);
        } else {
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(line.width(), TestCanvasWidth - 100, EPSILON100),
                            "#%zd: %f <= %d\n",
                            num,
                            line.width(),
                            TestCanvasWidth - 100);
        }
    }
}

UNIX_ONLY_TEST(SkParagraph_LineMetricsTextAlign, reporter) {

    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    TestCanvas canvas("SkParagraph_LineMetricsTextAlign.png");

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setColor(SK_ColorBLACK);

    auto layout = [&](TextAlign text_align) -> std::pair<SkScalar, SkScalar> {
        paragraph_style.setTextAlign(text_align);
        ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());
        builder.pushStyle(text_style);
        builder.addText("Some text that takes more than 200 px");
        auto paragraph = builder.Build();
        paragraph->layout(200);
        paragraph->paint(canvas.get(), 0, 0);
        canvas.get()->translate(0, paragraph->getHeight());
        std::vector<LineMetrics> metrics;
        paragraph->getLineMetrics(metrics);
        REPORTER_ASSERT(reporter, metrics.size() > 0);
        return {metrics[0].fLeft, metrics[0].fWidth};
    };

    SkScalar left[4];
    SkScalar width[4];
    std::tie(left[0], width[0]) = layout(TextAlign::kLeft);
    std::tie(left[1], width[1]) = layout(TextAlign::kCenter);
    std::tie(left[2], width[2]) = layout(TextAlign::kRight);
    std::tie(left[3], width[3]) = layout(TextAlign::kJustify);

    // delta = line_width - text_width
    REPORTER_ASSERT(reporter, left[0] == 0);        // Starts from 0
    REPORTER_ASSERT(reporter, left[1] > left[0]);   // Starts from delta / 2
    REPORTER_ASSERT(reporter, left[2] > left[1]);   // Starts from delta
    REPORTER_ASSERT(reporter, left[3] == left[0]);  // Starts from 0
    REPORTER_ASSERT(reporter, width[1] == width[0]);
    REPORTER_ASSERT(reporter, width[2] == width[0]);
    REPORTER_ASSERT(reporter, width[3] > width[0]);  // delta == 0
}

// Regression test for b/301804265: hit-testing justified lines with multi-grapheme clusters
DEF_TEST(SkParagraph_JustificationHitTest, reporter) {
    auto fontCollection = sk_make_sp<FontCollection>();
    fontCollection->setDefaultFontManager(ToolUtils::TestFontMgr());

    ParagraphStyle paragraphStyle;
    paragraphStyle.setTextDirection(TextDirection::kLtr);
    paragraphStyle.setTextAlign(TextAlign::kJustify);
    TextStyle style;
    style.setFontSize(10.0f);
    paragraphStyle.setTextStyle(style);

    ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());
    builder.pushStyle(style);
    builder.addText("A ");
    builder.addText("B\u200B");
    builder.addText("CCCCCCCCCC");
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(100.0f);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    Paragraph::GlyphInfo glyphInfo;
    for (SkScalar x = 0.0f; x <= 100.0f; x += 5.0f) {
        REPORTER_ASSERT(reporter, impl->getClosestUTF16GlyphInfoAt(x, 5.0f, &glyphInfo));
    }
}

// Lays out the text in one style, or in two styles if the second text is given
static std::unique_ptr<Paragraph> layoutJustified(sk_sp<ResourceFontCollection> fontCollection,
                                                  const char* family,
                                                  SkScalar fontSize,
                                                  TextDirection direction,
                                                  SkScalar width,
                                                  const char* text,
                                                  const char* secondText = nullptr) {
    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    paragraph_style.setApplyRoundingHack(false);
    paragraph_style.setTextAlign(TextAlign::kJustify);
    paragraph_style.setTextDirection(direction);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());
    TextStyle text_style;
    text_style.setFontFamilies({SkString(family), SkString("Roboto")});
    text_style.setFontSize(fontSize);
    text_style.setColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text);
    if (secondText != nullptr) {
        text_style.setFontSize(fontSize * 1.5f);
        builder.pushStyle(text_style);
        builder.addText(secondText);
        builder.pop();
    }
    builder.pop();
    auto paragraph = builder.Build();
    paragraph->layout(width);
    return paragraph;
}

// A hit inside the box of a code unit lands on that code unit: near the start of the box (in the
// direction of its text) it returns the code unit downstream, near the end the next one upstream
static void checkHitsInsideBoxes(skiatest::Reporter* reporter, Paragraph* paragraph) {
    auto impl = static_cast<ParagraphImpl*>(paragraph);
    impl->ensureUTF16Mapping();
    auto length = SkToS32(impl->getUTF16Index(impl->text().size()));
    for (int32_t i = 0; i < length; ++i) {
        auto boxes = paragraph->getRectsForRange(
                i, i + 1, RectHeightStyle::kTight, RectWidthStyle::kTight);
        for (auto& box : boxes) {
            if (box.rect.width() < 1) {
                continue;
            }
            SkScalar y = box.rect.centerY();
            SkScalar quarter = box.rect.width() / 4;
            auto left = paragraph->getGlyphPositionAtCoordinate(box.rect.fLeft + quarter, y);
            auto right = paragraph->getGlyphPositionAtCoordinate(box.rect.fRight - quarter, y);
            bool ltr = box.direction == TextDirection::kLtr;
            auto start = ltr ? left : right;
            auto end = ltr ? right : left;
            REPORTER_ASSERT(reporter,
                            start.position == i && start.affinity == Affinity::kDownstream,
                            "box of %d [%g, %g): start hit %d %s",
                            i,
                            box.rect.fLeft,
                            box.rect.fRight,
                            start.position,
                            start.affinity == Affinity::kUpstream ? "upstream" : "downstream");
            REPORTER_ASSERT(reporter,
                            end.position == i + 1 && end.affinity == Affinity::kUpstream,
                            "box of %d [%g, %g): end hit %d %s",
                            i,
                            box.rect.fLeft,
                            box.rect.fRight,
                            end.position,
                            end.affinity == Affinity::kUpstream ? "upstream" : "downstream");
        }
    }
}

// The text from https://github.com/flutter/flutter/issues/155062, where the box of the last
// ideograph on a line ends at the line width
UNIX_ONLY_TEST(SkParagraph_JustifiedIdeographicHitsInsideBoxes, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>(true);
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    auto paragraph = layoutJustified(
            fontCollection,
            "Source Han Serif CN",
            15,
            TextDirection::kLtr,
            300,
            "等一下我先下去，你跟在我后面，二伢子你带个土耗子殿后，三伢子你就别下去了，"
            "四个人都下去，想退都来不及退，你就拉着土耗子的尾巴，我们在里面一吆喝你就把东西拉出来。”");
    REPORTER_ASSERT(reporter, paragraph->lineNumber() > 1);
    checkHitsInsideBoxes(reporter, paragraph.get());
}

// The trailing space of each justified line, and a space where the style changes
UNIX_ONLY_TEST(SkParagraph_JustifiedSpacesHitsInsideBoxes, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>(true);
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    auto paragraph = layoutJustified(fontCollection,
                                     "Roboto",
                                     15,
                                     TextDirection::kLtr,
                                     300,
                                     "The quick brown fox jumps over the lazy dog and ",
                                     "keeps running across the wide green field until night");
    REPORTER_ASSERT(reporter, paragraph->lineNumber() > 1);
    checkHitsInsideBoxes(reporter, paragraph.get());
}

// Mixed RTL and LTR runs in justified RTL lines
UNIX_ONLY_TEST(SkParagraph_JustifiedRtlMixedHitsInsideBoxes, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>(true);
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    for (SkScalar width : {110.0f, 140.0f, 180.0f, 300.0f}) {
        auto paragraph =
                layoutJustified(fontCollection,
                                "Noto Naskh Arabic",
                                15,
                                TextDirection::kRtl,
                                width,
                                "Hello world دار ورد زاد and more english words دور رواد here ok");
        REPORTER_ASSERT(reporter, paragraph->lineNumber() > 1);
        checkHitsInsideBoxes(reporter, paragraph.get());
    }
}

// Each painted glyph starts inside the box of its code unit, and every box stays in the line
static void checkBoxesMatchGlyphs(skiatest::Reporter* reporter,
                                  Paragraph* paragraph,
                                  SkScalar width) {
    auto impl = static_cast<ParagraphImpl*>(paragraph);
    impl->ensureUTF16Mapping();
    size_t glyphs = 0;
    paragraph->visit([&](int, const Paragraph::VisitorInfo* info) {
        if (info == nullptr) {
            return;
        }
        for (int g = 0; g < info->count; ++g) {
            auto i = impl->getUTF16Index(info->utf8Starts[g]);
            auto boxes = paragraph->getRectsForRange(
                    i, i + 1, RectHeightStyle::kTight, RectWidthStyle::kTight);
            if (boxes.empty()) {
                continue;
            }
            ++glyphs;
            SkScalar x = info->origin.fX + info->positions[g].fX;
            auto& box = boxes.front().rect;
            REPORTER_ASSERT(reporter,
                            box.fLeft - EPSILON100 <= x && x <= box.fRight + EPSILON100,
                            "glyph of %zu at %g, box [%g, %g)",
                            i,
                            x,
                            box.fLeft,
                            box.fRight);
            REPORTER_ASSERT(reporter,
                            box.fLeft > -EPSILON100 && box.fRight < width + EPSILON100,
                            "box of %zu [%g, %g) outside [0, %g)",
                            i,
                            box.fLeft,
                            box.fRight,
                            width);
        }
    });
    REPORTER_ASSERT(reporter, glyphs > 0);
}

// An RTL run at the end of a justified LTR line moves with the rest of the line, including the
// trailing space inside it that the boxes of the run are measured from
UNIX_ONLY_TEST(SkParagraph_JustifiedLtrLineEndingInRtlRunBoxesMatchGlyphs, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>(true);
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    // At these widths a line ends inside an Arabic run
    for (SkScalar width : {110.0f, 140.0f, 180.0f}) {
        auto paragraph =
                layoutJustified(fontCollection,
                                "Noto Naskh Arabic",
                                15,
                                TextDirection::kLtr,
                                width,
                                "Hello world دار ورد زاد and more english words دور رواد here ok");
        REPORTER_ASSERT(reporter, paragraph->lineNumber() > 1);
        checkBoxesMatchGlyphs(reporter, paragraph.get(), width);
    }
}

// Trailing spaces in two runs at a line end must not make an invalid cluster range in justify
DEF_TEST(SkParagraph_JustifyTrailingSpacesInTwoRuns, reporter) {
    auto fontCollection = sk_make_sp<FontCollection>();
    fontCollection->setDefaultFontManager(ToolUtils::TestFontMgr());
    ParagraphStyle paragraphStyle;
    paragraphStyle.setTextAlign(TextAlign::kJustify);
    ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());
    TextStyle textStyle;
    textStyle.setFontSize(20);
    builder.pushStyle(textStyle);
    builder.addText("aaa bbb ");
    // A different font size puts the second space in its own run
    textStyle.setFontSize(10);
    builder.pushStyle(textStyle);
    builder.addText(" ");
    builder.pop();
    builder.addText("ccc");
    auto paragraph = builder.Build();
    paragraph->layout(SK_ScalarInfinity);
    auto boxes = paragraph->getRectsForRange(0, 7, RectHeightStyle::kTight, RectWidthStyle::kTight);
    REPORTER_ASSERT(reporter, boxes.size() == 1);
    if (boxes.empty()) {
        return;
    }
    SkScalar width = SkScalarCeilToScalar(boxes.back().rect.fRight) + 1;

    // The first line is "aaa bbb" and the two spaces are its trailing whitespaces
    paragraph->layout(width);
    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, impl->lines().size() == 2);
    if (impl->lines().size() != 2) {
        return;
    }
    auto& line = impl->lines()[0];
    REPORTER_ASSERT(reporter, line.clusters().end + 2 == line.clustersWithSpaces().end);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(line.width(), width));
}

// Justify must skip runs that the ellipsis removed from a line
DEF_TEST(SkParagraph_JustifyEllipsisRemovesRuns, reporter) {
    auto fontCollection = sk_make_sp<FontCollection>();
    fontCollection->setDefaultFontManager(ToolUtils::TestFontMgr());
    auto build = [&](bool ellipsis) {
        ParagraphStyle paragraphStyle;
        paragraphStyle.setTextAlign(TextAlign::kJustify);
        if (ellipsis) {
            paragraphStyle.setMaxLines(1);
            paragraphStyle.setEllipsis(u"\u2026");
        }
        ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());
        TextStyle textStyle;
        // The ellipsis in this large font is wider than the line
        textStyle.setFontSize(100);
        builder.pushStyle(textStyle);
        builder.addText("i");
        textStyle.setFontSize(10);
        builder.pushStyle(textStyle);
        builder.addText("j kkkk");
        return builder.Build();
    };
    auto paragraph = build(false);
    paragraph->layout(SK_ScalarInfinity);
    auto boxes = paragraph->getRectsForRange(0, 2, RectHeightStyle::kTight, RectWidthStyle::kTight);
    REPORTER_ASSERT(reporter, !boxes.empty());
    if (boxes.empty()) {
        return;
    }

    // "ij" fits on the line, but no ellipsis fits after any part of it
    paragraph = build(true);
    paragraph->layout(SkScalarCeilToScalar(boxes.back().rect.fRight) + 1);
    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, impl->lines().size() == 1);
    REPORTER_ASSERT(reporter, paragraph->didExceedMaxLines());
}

// Justification fills the lines to the full width, also when the paragraph is laid out again
DEF_TEST(SkParagraph_JustifyFillsLines, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    fontCollection->disableFontFallback();

    TextStyle style;
    style.setFontFamilies({SkString("Roboto")});
    style.setFontSize(20);
    style.setColor(SK_ColorBLACK);
    ParagraphStyle paragraphStyle;
    paragraphStyle.setTextStyle(style);
    paragraphStyle.setTextAlign(TextAlign::kJustify);
    ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());
    builder.pushStyle(style);
    builder.addText("The quick brown fox jumps over the lazy dog and runs far away");
    auto paragraph = builder.Build();
    auto impl = static_cast<ParagraphImpl*>(paragraph.get());

    for (SkScalar width : {200.0f, 260.0f}) {
        paragraph->layout(width);
        REPORTER_ASSERT(reporter, impl->lines().size() > 1);
        // The last line is not justified
        for (size_t i = 0; i + 1 < impl->lines().size(); ++i) {
            auto range = impl->lines()[i].trimmedText();
            auto boxes = paragraph->getRectsForRange(
                    range.start, range.end, RectHeightStyle::kTight, RectWidthStyle::kTight);
            REPORTER_ASSERT(reporter, !boxes.empty());
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(boxes.back().rect.fRight, width, 0.01f),
                            "line %zu: %f != %f",
                            i,
                            boxes.back().rect.fRight,
                            width);
        }
    }
}
