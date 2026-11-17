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
#include <vector>

using namespace skia::textlayout;
using namespace skia::textlayout::test;

UNIX_ONLY_TEST(SkParagraph_HeightOverrideParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    TestCanvas canvas("SkParagraph_HeightOverrideParagraph.png");
    const char* text = "01234満毎冠行来昼本可\nabcd\n満毎冠行来昼本可";
    const size_t len = strlen(text);

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    paragraph_style.setMaxLines(10);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setFontSize(20);
    text_style.setColor(SK_ColorBLACK);
    text_style.setHeight(3.6345f);
    text_style.setHeightOverride(true);
    builder.pushStyle(text_style);
    builder.addText(text, len);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(550);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, impl->runs().size() == 5);
    REPORTER_ASSERT(reporter, impl->styles().size() == 1);  // paragraph style does not count
    REPORTER_ASSERT(reporter, impl->styles()[0].fStyle.equals(text_style));

    paragraph->paint(canvas.get(), 0, 0);

    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setAntiAlias(true);
    paint.setStrokeWidth(1);

    // Tests for GetRectsForRange()
    RectHeightStyle rect_height_style = RectHeightStyle::kIncludeLineSpacingMiddle;
    RectWidthStyle rect_width_style = RectWidthStyle::kTight;
    paint.setColor(SK_ColorRED);
    std::vector<TextBox> boxes =
            paragraph->getRectsForRange(0, 0, rect_height_style, rect_width_style);
    canvas.drawRects(SK_ColorRED, boxes);
    REPORTER_ASSERT(reporter, boxes.size() == 0ull);

    boxes = paragraph->getRectsForRange(0, 40, rect_height_style, rect_width_style);
    canvas.drawRects(SK_ColorBLUE, boxes);
    REPORTER_ASSERT(reporter, boxes.size() == 3ull);
    if (boxes.size() >= 3) {
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[1].rect.left(), 0, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[1].rect.top(), 92.805f, EPSILON5));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[1].rect.right(), 43.843f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[1].rect.bottom(), 165.495f, EPSILON5));
    }
}

// Verify that baselineShift is applied once when heightOverride is enabled
DEF_TEST(SkParagraph_BaselineShiftWithHeightMultiplier, reporter) {
    auto fontCollection = sk_make_sp<FontCollection>();
    fontCollection->setDefaultFontManager(ToolUtils::TestFontMgr());

    const SkScalar kBaselineShift = -12.0f;
    for (bool halfLeading : {false, true}) {
        auto buildSingleRun = [&](SkScalar shift) {
            ParagraphStyle paragraphStyle;
            TextStyle textStyle;
            textStyle.setFontSize(50.0f);
            textStyle.setHeightOverride(true);
            textStyle.setHeight(2.0f);
            textStyle.setHalfLeading(halfLeading);
            textStyle.setBaselineShift(shift);

            ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());
            builder.pushStyle(textStyle);
            builder.addText("A");
            builder.pop();
            auto paragraph = builder.Build();
            paragraph->layout(500.0f);
            return paragraph;
        };

        auto unshifted = buildSingleRun(0.0f);
        auto shifted = buildSingleRun(kBaselineShift);
        auto* unshiftedImpl = static_cast<ParagraphImpl*>(unshifted.get());
        auto* shiftedImpl = static_cast<ParagraphImpl*>(shifted.get());

        REPORTER_ASSERT(reporter, unshiftedImpl->runs().size() == 1);
        REPORTER_ASSERT(reporter, shiftedImpl->runs().size() == 1);

        const Run& run0 = unshiftedImpl->runs()[0];
        const Run& runS = shiftedImpl->runs()[0];

        REPORTER_ASSERT(
                reporter,
                SkScalarNearlyEqual(
                        runS.correctAscent(), run0.correctAscent() + kBaselineShift, EPSILON100));
        REPORTER_ASSERT(
                reporter,
                SkScalarNearlyEqual(
                        runS.correctDescent(), run0.correctDescent() + kBaselineShift, EPSILON100));
    }
}

// A shifted run must extend the line only by its shift, as without a height override
DEF_TEST(SkParagraph_BaselineShiftLineHeightWithHeightOverride, reporter) {
    auto fontCollection = sk_make_sp<FontCollection>();
    fontCollection->setDefaultFontManager(ToolUtils::TestFontMgr());
    fontCollection->getParagraphCache()->turnOn(false);

    const SkScalar kShift = -20.0f;
    auto heightWith = [&](bool heightOverride, bool halfLeading, SkScalar shift) {
        TextStyle base;
        base.setFontSize(50.0f);
        base.setHeightOverride(heightOverride);
        base.setHeight(1.0f);
        base.setHalfLeading(halfLeading);
        TextStyle shifted = base;
        shifted.setBaselineShift(shift);

        ParagraphStyle paragraphStyle;
        ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());
        builder.pushStyle(base);
        builder.addText("x");
        builder.pop();
        builder.pushStyle(shifted);
        builder.addText("x");
        builder.pop();
        auto paragraph = builder.Build();
        paragraph->layout(500.0f);
        return paragraph->getHeight();
    };

    for (bool heightOverride : {false, true}) {
        for (bool halfLeading : {false, true}) {
            const SkScalar growth = heightWith(heightOverride, halfLeading, kShift) -
                                    heightWith(heightOverride, halfLeading, 0.0f);
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(growth, -kShift, 1.0f),
                            "heightOverride=%d halfLeading=%d growth %g",
                            heightOverride,
                            halfLeading,
                            growth);
        }
    }
}

UNIX_ONLY_TEST(SkParagraph_BasicHalfLeading, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    const char* text = "01234満毎冠行来昼本可\nabcd\n満毎冠行来昼本可";
    const size_t len = strlen(text);

    TestCanvas canvas("SkParagraph_BasicHalfLeading.png");

    ParagraphStyle paragraph_style;
    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setFontSize(20.0f);
    text_style.setColor(SK_ColorBLACK);
    text_style.setLetterSpacing(0.0f);
    text_style.setWordSpacing(0.0f);
    text_style.setHeightOverride(true);
    text_style.setHeight(3.6345f);
    text_style.setHalfLeading(true);

    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());

    builder.pushStyle(text_style);
    builder.addText(text);

    auto paragraph = builder.Build();
    paragraph->layout(550);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, impl->styles().size() == 1);  // paragraph style does not count
    REPORTER_ASSERT(reporter, impl->styles()[0].fStyle.equals(text_style));

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
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[1].rect.right(), 43.843f, EPSILON100));
    }
}

UNIX_ONLY_TEST(SkParagraph_NearZeroHeightMixedDistribution, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    const char* text = "Cookies need love";
    const size_t len = strlen(text);

    TestCanvas canvas("SkParagraph_ZeroHeightHalfLeading.png");

    ParagraphStyle paragraph_style;
    paragraph_style.setTextHeightBehavior(TextHeightBehavior::kAll);
    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setFontSize(20.0f);
    text_style.setColor(SK_ColorBLACK);
    text_style.setLetterSpacing(0.0f);
    text_style.setWordSpacing(0.0f);
    text_style.setHeightOverride(true);
    text_style.setHeight(0.001f);

    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());

    // First run, half leading.
    text_style.setHalfLeading(true);
    builder.pushStyle(text_style);
    builder.addText(text);

    // Second run, no half leading.
    text_style.setHalfLeading(false);
    builder.pushStyle(text_style);
    builder.addText(text);

    auto paragraph = builder.Build();
    paragraph->layout(550);
    paragraph->paint(canvas.get(), 0, 0);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, impl->runs().size() == 2);
    REPORTER_ASSERT(reporter, impl->styles().size() == 2);  // paragraph style does not count
    REPORTER_ASSERT(reporter, impl->lines().size() == 1ull);

    const RectWidthStyle rect_width_style = RectWidthStyle::kTight;

    std::vector<TextBox> boxes =
            paragraph->getRectsForRange(0, len, RectHeightStyle::kTight, rect_width_style);
    std::vector<TextBox> lineBoxes =
            paragraph->getRectsForRange(0, len, RectHeightStyle::kMax, rect_width_style);

    canvas.drawRects(SK_ColorBLUE, boxes);
    REPORTER_ASSERT(reporter, boxes.size() == 1ull);
    REPORTER_ASSERT(reporter, lineBoxes.size() == boxes.size());

    // From font metrics.
    const auto metricsAscent = -18.5546875f;
    const auto metricsDescent = 4.8828125f;

    // As the height multiplier converges to 0 (but not 0 since 0 is used as a
    // magic value to indicate there's no height multiplier), the `Run`s top
    // edge and bottom edge will converge to a horizontal line:
    // - When half leading is used the vertical line is roughly the center of
    //   of the glyphs in the run ((fontMetrics.descent - fontMetrics.ascent) / 2)
    // - When half leading is disabled the line is the alphabetic baseline.

    // Expected values in baseline coordinate space:
    const auto run1_ascent = (metricsAscent + metricsDescent) / 2;
    const auto run1_descent = (metricsAscent + metricsDescent) / 2;
    const auto run2_ascent = 0.0f;
    const auto run2_descent = 0.0f;
    const auto line_top = std::min(run1_ascent, run2_ascent);
    const auto line_bottom = std::max(run1_descent, run2_descent);

    // Expected glyph height in linebox coordinate space:
    const auto glyphs_top = metricsAscent - line_top;
    const auto glyphs_bottom = metricsDescent - line_top;

    // kTight reports the glyphs' bounding box in the linebox's coordinate
    // space.
    const auto actual_glyphs_top = boxes[0].rect.top() - lineBoxes[0].rect.top();
    const auto actual_glyphs_bottom = boxes[0].rect.bottom() - lineBoxes[0].rect.top();

    // Use a relatively large epsilon since the heightMultiplier is not actually
    // 0.
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(glyphs_top, actual_glyphs_top, EPSILON20));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(glyphs_bottom, actual_glyphs_bottom, EPSILON20));

    REPORTER_ASSERT(
            reporter,
            SkScalarNearlyEqual(lineBoxes[0].rect.height(), line_bottom - line_top, EPSILON2));
    REPORTER_ASSERT(reporter, lineBoxes[0].rect.height() > 1);

    // Half leading does not move the text horizontally.
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 0, EPSILON100));
}

UNIX_ONLY_TEST(SkParagraph_TrimLeadingDistribution, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    const char* text = "01234満毎冠行来昼本可\nabcd\n満毎冠行来昼本可";
    const size_t len = strlen(text);

    TestCanvas canvas("SkParagraph_TrimHalfLeading.png");

    ParagraphStyle paragraph_style;
    paragraph_style.setTextHeightBehavior(TextHeightBehavior::kDisableAll);
    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setFontSize(20.0f);
    text_style.setColor(SK_ColorBLACK);
    text_style.setLetterSpacing(0.0f);
    text_style.setWordSpacing(0.0f);
    text_style.setHeightOverride(true);
    text_style.setHeight(3.6345f);
    text_style.setHalfLeading(true);

    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());

    builder.pushStyle(text_style);
    builder.addText(text);

    auto paragraph = builder.Build();
    paragraph->layout(550);
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

        // Uniform line spacing. The delta is introduced by the height rounding.
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(line_spacing1, line_spacing2, 1));

        // Trim the first line's top leading.
        REPORTER_ASSERT(reporter,
                        SkScalarNearlyEqual(lineBoxes[0].rect.top(), boxes[0].rect.top()));
        // Trim the last line's bottom leading.
        REPORTER_ASSERT(reporter,
                        SkScalarNearlyEqual(lineBoxes[2].rect.bottom(), boxes[2].rect.bottom()));

        const auto halfLeading = lineBoxes[0].rect.bottom() - boxes[0].rect.bottom();
        // Large epsilon because of rounding.
        const auto epsilon = EPSILON10;
        // line spacing is distributed evenly over and under the text.
        REPORTER_ASSERT(
                reporter,
                SkScalarNearlyEqual(
                        boxes[1].rect.top() - lineBoxes[1].rect.top(), halfLeading, epsilon));
        REPORTER_ASSERT(reporter,
                        SkScalarNearlyEqual(lineBoxes[1].rect.bottom() - boxes[1].rect.bottom(),
                                            halfLeading));
        REPORTER_ASSERT(
                reporter,
                SkScalarNearlyEqual(
                        boxes[2].rect.top() - lineBoxes[2].rect.top(), halfLeading, epsilon));

        // Half leading does not move the text horizontally.
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[1].rect.left(), 0, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[1].rect.right(), 43.843f, EPSILON100));
    }
}

// Verify that computeEmptyMetrics reads halfLeading from textStyle rather than strutStyle
DEF_TEST(SkParagraph_EmptyMetricsHalfLeadingFromTextStyle, reporter) {
    auto fontCollection = sk_make_sp<FontCollection>();
    fontCollection->setDefaultFontManager(ToolUtils::TestFontMgr());

    auto buildEmpty = [&](bool textHalfLeading, bool strutHalfLeading, bool heightOverride) {
        ParagraphStyle paragraphStyle;
        StrutStyle strutStyle;
        strutStyle.setStrutEnabled(false);
        strutStyle.setHalfLeading(strutHalfLeading);
        paragraphStyle.setStrutStyle(strutStyle);

        TextStyle textStyle;
        textStyle.setFontSize(50.0f);
        textStyle.setHeightOverride(heightOverride);
        textStyle.setHeight(2.0f);
        textStyle.setHalfLeading(textHalfLeading);
        paragraphStyle.setTextStyle(textStyle);

        ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());
        builder.pushStyle(textStyle);
        builder.pop();
        auto paragraph = builder.Build();
        paragraph->layout(500.0f);
        return paragraph;
    };

    auto basePara = buildEmpty(false, false, false);
    auto* baseImpl = static_cast<ParagraphImpl*>(basePara.get());
    const auto baseMetrics = baseImpl->getEmptyMetrics();
    const SkScalar intrinsicHeight = baseMetrics.height();
    const SkScalar targetHeight = 100.0f;

    auto halfLeadingPara = buildEmpty(true, false, true);
    auto* halfLeadingImpl = static_cast<ParagraphImpl*>(halfLeadingPara.get());
    const auto halfMetrics = halfLeadingImpl->getEmptyMetrics();
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(halfMetrics.ascent(), baseMetrics.ascent(), EPSILON100));
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(halfMetrics.descent(), baseMetrics.descent(), EPSILON100));
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(halfMetrics.leading(),
                                        baseMetrics.leading() + targetHeight - intrinsicHeight,
                                        EPSILON100));

    auto scaledPara = buildEmpty(false, true, true);
    auto* scaledImpl = static_cast<ParagraphImpl*>(scaledPara.get());
    const auto scaledMetrics = scaledImpl->getEmptyMetrics();
    const SkScalar multiplier = targetHeight / intrinsicHeight;
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(
                            scaledMetrics.ascent(), baseMetrics.ascent() * multiplier, EPSILON100));
    REPORTER_ASSERT(
            reporter,
            SkScalarNearlyEqual(
                    scaledMetrics.descent(), baseMetrics.descent() * multiplier, EPSILON100));
}

// Checked: DIFF+
UNIX_ONLY_TEST(SkParagraph_GetRectsForRangeIncludeLineSpacingMiddle, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    TestCanvas canvas("SkParagraph_GetRectsForRangeIncludeLineSpacingMiddle.png");
    const char* text =
            "(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)("
            "　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)("
            "　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)";
    const size_t len = strlen(text);

    ParagraphStyle paragraphStyle;
    paragraphStyle.setTextAlign(TextAlign::kLeft);
    paragraphStyle.setMaxLines(10);
    paragraphStyle.turnHintingOff();
    TextStyle textStyle;
    textStyle.setFontFamilies({SkString("Roboto")});
    textStyle.setFontSize(50);
    textStyle.setHeight(1.6f);
    textStyle.setHeightOverride(true);
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

    RectHeightStyle heightStyle = RectHeightStyle::kIncludeLineSpacingMiddle;
    RectWidthStyle widthStyle = RectWidthStyle::kMax;
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
                            SkScalarNearlyEqual(result[0].rect.top(), 16.946615f, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.right(), 17.4296889f, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.bottom(), 88.473305f, EPSILON100));
        }
    }
    {
        auto result = paragraph->getRectsForRange(2, 8, heightStyle, widthStyle);
        canvas.drawRects(SK_ColorBLUE, result);
        REPORTER_ASSERT(reporter, result.size() == 1);
        if (result.size() >= 1) {
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.left(), 67.429688f, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.top(), 16.946615f, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.right(), 190.00781f, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.bottom(), 88.473305f, EPSILON100));
        }
    }
    {
        auto result = paragraph->getRectsForRange(8, 21, heightStyle, widthStyle);
        canvas.drawRects(SK_ColorGREEN, result);
        REPORTER_ASSERT(reporter, result.size() == 1);
        if (result.size() >= 1) {
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.left(), 190.00781f, EPSILON20));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.top(), 16.946615f, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.right(), 508.0625f, EPSILON20));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.bottom(), 88.473305f, EPSILON100));
        }
    }
    {
        auto result = paragraph->getRectsForRange(30, 150, heightStyle, widthStyle);
        canvas.drawRects(SK_ColorRED, result);
        REPORTER_ASSERT(reporter, result.size() == 8);
        if (result.size() >= 8) {
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.left(), 190.00781f, EPSILON20));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.top(), 88.473305f, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.right(), 525.687f, EPSILON20));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.bottom(), 168.47331f, EPSILON100));

            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[1].rect.left(), 525.687f, EPSILON20));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[1].rect.top(), 88.473305f, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[1].rect.right(), 570.02344f, EPSILON20));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[1].rect.bottom(), 168.47331f, EPSILON100));

            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[2].rect.left(), 0, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[2].rect.top(), 168.47331f, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[2].rect.right(), 531.574f, EPSILON20));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[2].rect.bottom(), 248.47331f, EPSILON100));

            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[3].rect.left(), 531.574f, EPSILON20));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[3].rect.top(), 168.47331f, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[3].rect.right(), 570.02344f, EPSILON20));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[3].rect.bottom(), 248.47331f, EPSILON100));

            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[4].rect.left(), 0, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[4].rect.top(), 248.47331f, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[4].rect.right(), 570.02344f, EPSILON20));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[4].rect.bottom(), 328.47333f, EPSILON100));

            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[5].rect.left(), 0, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[5].rect.top(), 328.47333f, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[5].rect.right(), 570.02344f, EPSILON20));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[5].rect.bottom(), 408.4733f, EPSILON100));
        }
    }
    {
        auto result = paragraph->getRectsForRange(19, 22, heightStyle, widthStyle);
        canvas.drawRects(SK_ColorBLUE, result);
        REPORTER_ASSERT(reporter, result.size() == 2);  // DIFF
        if (result.size() >= 2) {
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.left(), 463.72656f, EPSILON20));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.top(), 16.946615f, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.right(), 530.23047f, EPSILON20));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.bottom(), 88.473305f, EPSILON100));

            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[1].rect.left(), 530.23047f, EPSILON20));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[1].rect.top(), 16.946615f, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[1].rect.right(), 570.02344f, EPSILON20));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[1].rect.bottom(), 88.473305f, EPSILON100));
        }
    }
    {
        auto result = paragraph->getRectsForRange(21, 21, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.empty());
    }
}

// Checked: NO DIFF+
UNIX_ONLY_TEST(SkParagraph_GetRectsForRangeIncludeLineSpacingTop, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    TestCanvas canvas("SkParagraph_GetRectsForRangeIncludeLineSpacingTop.png");
    const char* text =
            "(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)("
            "　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)("
            "　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)";
    const size_t len = strlen(text);

    ParagraphStyle paragraphStyle;
    paragraphStyle.setTextAlign(TextAlign::kLeft);
    paragraphStyle.setMaxLines(10);
    paragraphStyle.turnHintingOff();
    TextStyle textStyle;
    textStyle.setFontFamilies({SkString("Roboto")});
    textStyle.setFontSize(50);
    textStyle.setHeight(1.6f);
    textStyle.setHeightOverride(true);
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

    RectHeightStyle heightStyle = RectHeightStyle::kIncludeLineSpacingTop;
    RectWidthStyle widthStyle = RectWidthStyle::kMax;
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
                            SkScalarNearlyEqual(result[0].rect.top(), 16.946615f, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.right(), 17.4296889f, EPSILON100));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 80, EPSILON100));
        }
    }
    {
        auto result = paragraph->getRectsForRange(2, 8, heightStyle, widthStyle);
        canvas.drawRects(SK_ColorBLUE, result);
        REPORTER_ASSERT(reporter, result.size() == 1);
        if (result.size() >= 1) {
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.left(), 67.429688f, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.top(), 16.946615f, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.right(), 190.00781f, EPSILON100));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 80, EPSILON100));
        }
    }
    {
        auto result = paragraph->getRectsForRange(8, 21, heightStyle, widthStyle);
        canvas.drawRects(SK_ColorGREEN, result);
        REPORTER_ASSERT(reporter, result.size() == 1);
        if (result.size() >= 1) {
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.left(), 190.00781f, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.top(), 16.946615f, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.right(), 508.0625f, EPSILON50));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 80, EPSILON100));
        }
    }
    {
        auto result = paragraph->getRectsForRange(30, 150, heightStyle, widthStyle);
        canvas.drawRects(SK_ColorMAGENTA, result);
        REPORTER_ASSERT(reporter, result.size() == 8);
        if (result.size() >= 8) {
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.left(), 190.00781f, EPSILON100));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 80, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.right(), 525.687f, EPSILON20));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.bottom(), 160, EPSILON100));

            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[1].rect.left(), 525.687f, EPSILON20));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[1].rect.top(), 80, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[1].rect.right(), 570.02344f, EPSILON20));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[1].rect.bottom(), 160, EPSILON100));

            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[2].rect.left(), 0, EPSILON100));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[2].rect.top(), 160, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[2].rect.right(), 531.574f, EPSILON20));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[2].rect.bottom(), 240, EPSILON100));

            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[3].rect.left(), 531.574f, EPSILON20));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[3].rect.top(), 160, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[3].rect.right(), 570.02344f, EPSILON20));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[3].rect.bottom(), 240, EPSILON100));

            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[4].rect.left(), 0, EPSILON100));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[4].rect.top(), 240, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[4].rect.right(), 570.02344f, EPSILON20));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[4].rect.bottom(), 320, EPSILON100));

            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[5].rect.left(), 0, EPSILON100));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[5].rect.top(), 320, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[5].rect.right(), 570.02344f, EPSILON20));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[5].rect.bottom(), 400, EPSILON100));
        }
    }
    {
        auto result = paragraph->getRectsForRange(19, 22, heightStyle, widthStyle);
        canvas.drawRects(SK_ColorBLACK, result);
        REPORTER_ASSERT(reporter, result.size() == 2);  // DIFF
        if (result.size() >= 2) {
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.left(), 463.72656f, EPSILON20));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.top(), 16.946615f, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.right(), 530.23047f, EPSILON20));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 80, EPSILON100));

            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[1].rect.left(), 530.23047f, EPSILON50));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[1].rect.top(), 16.946615f, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[1].rect.right(), 570.02344f, EPSILON20));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[1].rect.bottom(), 80, EPSILON100));
        }
    }
    {
        auto result = paragraph->getRectsForRange(21, 21, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.empty());
    }
}

// Checked: NO DIFF+
UNIX_ONLY_TEST(SkParagraph_GetRectsForRangeIncludeLineSpacingBottom, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    TestCanvas canvas("SkParagraph_GetRectsForRangeIncludeLineSpacingBottom.png");
    const char* text =
            "(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)("
            "　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)("
            "　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)";
    const size_t len = strlen(text);

    ParagraphStyle paragraphStyle;
    paragraphStyle.setTextAlign(TextAlign::kLeft);
    paragraphStyle.setMaxLines(10);
    paragraphStyle.turnHintingOff();
    TextStyle textStyle;
    textStyle.setFontFamilies({SkString("Roboto")});
    textStyle.setFontSize(50);
    textStyle.setHeight(1.6f);
    textStyle.setHeightOverride(true);
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

    RectHeightStyle heightStyle = RectHeightStyle::kIncludeLineSpacingBottom;
    RectWidthStyle widthStyle = RectWidthStyle::kMax;
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
                            SkScalarNearlyEqual(result[0].rect.top(), 16.946f, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.right(), 17.429f, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.bottom(), 96.946f, EPSILON100));
        }
    }
    {
        auto result = paragraph->getRectsForRange(2, 8, heightStyle, widthStyle);
        canvas.drawRects(SK_ColorBLUE, result);
        REPORTER_ASSERT(reporter, result.size() == 1);
        if (result.size() >= 1) {
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.left(), 67.4298f, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.top(), 16.946f, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.right(), 190.007f, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.bottom(), 96.946f, EPSILON100));
        }
    }
    {
        auto result = paragraph->getRectsForRange(8, 21, heightStyle, widthStyle);
        canvas.drawRects(SK_ColorGREEN, result);
        REPORTER_ASSERT(reporter, result.size() == 1);
        if (result.size() >= 1) {
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.left(), 190.007f, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.top(), 16.946f, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.right(), 508.062f, EPSILON50));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.bottom(), 96.946f, EPSILON100));
        }
    }
    {
        auto result = paragraph->getRectsForRange(30, 150, heightStyle, widthStyle);
        canvas.drawRects(SK_ColorMAGENTA, result);
        REPORTER_ASSERT(reporter, result.size() == 8);
        if (result.size() >= 8) {
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.left(), 190.007f, EPSILON20));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.top(), 96.946f, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.right(), 525.687f, EPSILON20));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.bottom(), 176.946f, EPSILON100));

            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[1].rect.left(), 525.687f, EPSILON20));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[1].rect.top(), 96.946f, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[1].rect.right(), 570.023f, EPSILON20));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[1].rect.bottom(), 176.946f, EPSILON100));

            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[2].rect.left(), 0, EPSILON20));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[2].rect.top(), 176.946f, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[2].rect.right(), 531.574f, EPSILON20));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[2].rect.bottom(), 256.946f, EPSILON100));

            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[3].rect.left(), 531.574f, EPSILON20));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[3].rect.top(), 176.946f, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[3].rect.right(), 570.023f, EPSILON20));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[3].rect.bottom(), 256.946f, EPSILON100));

            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[4].rect.left(), 0, EPSILON20));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[4].rect.top(), 256.946f, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[4].rect.right(), 570.023f, EPSILON20));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[4].rect.bottom(), 336.946f, EPSILON100));

            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[5].rect.left(), 0, EPSILON20));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[5].rect.top(), 336.946f, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[5].rect.right(), 570.023f, EPSILON20));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[5].rect.bottom(), 416.946f, EPSILON100));
        }
    }
    {
        auto result = paragraph->getRectsForRange(19, 22, heightStyle, widthStyle);
        canvas.drawRects(SK_ColorBLACK, result);
        REPORTER_ASSERT(reporter, result.size() == 2);  // DIFF
        if (result.size() >= 2) {
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.left(), 463.726f, EPSILON20));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.top(), 16.946f, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.right(), 530.230f, EPSILON20));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[0].rect.bottom(), 96.946f, EPSILON100));

            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[1].rect.left(), 530.230f, EPSILON20));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[1].rect.top(), 16.946f, EPSILON100));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[1].rect.right(), 570.023f, EPSILON20));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(result[1].rect.bottom(), 96.946f, EPSILON100));
        }
    }
    {
        auto result = paragraph->getRectsForRange(21, 21, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.empty());
    }
}

// ParagraphStyle::setHeight() is ignored by layout, so it must not split ParagraphCache entries
DEF_TEST(SkParagraph_ParagraphStyleHeightIgnored, reporter) {
    auto fontCollection = sk_make_sp<FontCollection>();
    fontCollection->setDefaultFontManager(ToolUtils::TestFontMgr());

    bool found = false;
    auto layout = [&](SkScalar height) {
        ParagraphStyle paragraphStyle;
        paragraphStyle.setHeight(height);
        TextStyle textStyle;
        textStyle.setFontSize(30.0f);
        ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());
        builder.pushStyle(textStyle);
        builder.addText("Hello world");
        builder.pop();
        auto paragraph = builder.Build();
        found = false;
        fontCollection->getParagraphCache()->setChecker(
                [&found](ParagraphImpl*, const char* event, bool) {
                    found |= strcmp(event, "foundParagraph") == 0;
                });
        paragraph->layout(1000.0f);
        fontCollection->getParagraphCache()->setChecker([](ParagraphImpl*, const char*, bool) {});
        return paragraph;
    };

    auto single = layout(1.0f);
    auto triple = layout(3.0f);
    REPORTER_ASSERT(reporter, found);
    REPORTER_ASSERT(reporter, triple->getHeight() == single->getHeight());
}

// ParagraphCache compares styles by the effective height the shaper uses
DEF_TEST(SkParagraph_EqualsByFontsUsesEffectiveHeight, reporter) {
    TextStyle overridden;
    overridden.setHeightOverride(true);
    overridden.setHeight(2.0f);

    TextStyle ignored = overridden;
    ignored.setHeightOverride(false);
    REPORTER_ASSERT(reporter, !overridden.equalsByFonts(ignored));

    TextStyle otherIgnored = ignored;
    otherIgnored.setHeight(3.0f);
    REPORTER_ASSERT(reporter, ignored.equalsByFonts(otherIgnored));
}

UNIX_ONLY_TEST(SkParagraph_HeightCalculations, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    TestCanvas canvas("SkParagraph_HeightCalculations.png");

    auto draw = [&](TextHeightBehavior hb, const char* text, SkScalar height) {
        ParagraphStyle paragraph_style;
        paragraph_style.setTextHeightBehavior(hb);

        ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());
        TextStyle text_style;
        text_style.setFontFamilies({SkString("Roboto")});
        text_style.setFontSize(14.0f);
        text_style.setHeight(5.0f);
        text_style.setHeightOverride(true);
        text_style.setColor(SK_ColorBLACK);
        builder.pushStyle(text_style);
        builder.addText(text);

        auto paragraph = builder.Build();
        paragraph->layout(500);
        paragraph->paint(canvas.get(), 0, 0);
        canvas.get()->translate(0, paragraph->getHeight());
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(paragraph->getHeight(), height));
    };

    draw(TextHeightBehavior::kAll, "Hello\nLine 2\nLine 3", 210);
    draw(TextHeightBehavior::kDisableAll, "Hello\nLine 2\nLine 3", 157);
    draw(TextHeightBehavior::kDisableFirstAscent, "Hello", 28);
}

UNIX_ONLY_TEST(SkParagraph_SingleLineHeight1, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    TestCanvas canvas("SkParagraph_SingleLineHeight1.png");

    auto paint = [&](const char* text) {
        ParagraphStyle paragraph_style;
        paragraph_style.setTextHeightBehavior(TextHeightBehavior::kDisableAll);
        paragraph_style.setMaxLines(1);
        ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());
        TextStyle text_style;
        text_style.setColor(SK_ColorBLACK);
        text_style.setFontFamilies({SkString("Ahem")});
        text_style.setFontSize(14);
        text_style.setHeight(2);
        text_style.setHeightOverride(true);
        builder.pushStyle(text_style);
        builder.addText(text);
        auto paragraph = builder.Build();
        paragraph->layout(80);
        paragraph->paint(canvas.get(), 0, 0);
        REPORTER_ASSERT(reporter, paragraph->getHeight() == 14.0f);
    };

    paint("Loooooooooooooooooooooooooooooooooooong text");
    paint("");
}

UNIX_ONLY_TEST(SkParagraph_SingleLineHeight2, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    TestCanvas canvas("SkParagraph_SingleLineHeight2.png");

    auto paint = [&](const char* text) {
        ParagraphStyle paragraph_style;
        paragraph_style.setMaxLines(1);
        ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());
        TextStyle text_style;
        text_style.setColor(SK_ColorBLACK);
        text_style.setFontFamilies({SkString("Ahem")});
        text_style.setFontSize(14);
        text_style.setHeight(2);
        text_style.setHeightOverride(true);
        builder.pushStyle(text_style);
        builder.addText(text);
        auto paragraph = builder.Build();
        paragraph->layout(80);
        paragraph->paint(canvas.get(), 0, 0);
        REPORTER_ASSERT(reporter, paragraph->getHeight() == 28.0f);
    };

    paint("Loooooooooooooooooooooooooooooooooooong text");
    paint("");
}
