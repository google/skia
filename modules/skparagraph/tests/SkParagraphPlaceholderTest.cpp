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
#include "modules/skparagraph/src/TextLine.h"
#include "modules/skparagraph/tests/SkParagraphTestUtils.h"
#include "tests/Test.h"
#include "tools/fonts/FontToolUtils.h"

#include <string.h>
#include <string>
#include <vector>

using namespace skia::textlayout;
using namespace skia::textlayout::test;

UNIX_ONLY_TEST(SkParagraph_PlaceholderLogicalOrder, reporter) {
    // No external fonts are required: the paragraph contains only placeholders.
    // Regression test for https://github.com/flutter/flutter/issues/54400.
    for (auto direction : {TextDirection::kLtr, TextDirection::kRtl}) {
        ParagraphStyle style;
        style.setTextDirection(direction);
        ParagraphBuilderImpl builder(style, sk_make_sp<FontCollection>(), get_unicode());
        for (SkScalar width : {30, 50, 70}) {
            builder.addPlaceholder(PlaceholderStyle(
                    width, 20, PlaceholderAlignment::kBottom, TextBaseline::kAlphabetic, 0));
        }
        auto paragraph = builder.Build();
        // Re-layout the same paragraph to exercise both wrapping and cached shaping.
        for (SkScalar width : {500, 80, 500}) {
            paragraph->layout(width);
            auto boxes = paragraph->getRectsForPlaceholders();
            REPORTER_ASSERT(reporter, boxes.size() == 3);
            if (boxes.size() != 3) {
                continue;
            }
            for (size_t i = 0; i < boxes.size(); ++i) {
                REPORTER_ASSERT(reporter, boxes[i].rect.width() == 30 + 20 * i);
                REPORTER_ASSERT(reporter, boxes[i].rect.height() == 20);
                REPORTER_ASSERT(reporter, boxes[i].direction == direction);
                auto range = paragraph->getRectsForRange(
                        i, i + 1, RectHeightStyle::kTight, RectWidthStyle::kTight);
                REPORTER_ASSERT(reporter, range.size() == 1);
                if (range.size() == 1) {
                    REPORTER_ASSERT(reporter, range[0].rect == boxes[i].rect);
                }
            }
            if (direction == TextDirection::kRtl) {
                REPORTER_ASSERT(reporter, boxes[0].rect.left() == width - 30);
                REPORTER_ASSERT(reporter, boxes[1].rect.left() == width - 80);
                REPORTER_ASSERT(reporter, boxes[2].rect.left() == width - (width == 80 ? 70 : 150));
            } else {
                REPORTER_ASSERT(reporter, boxes[0].rect.left() == 0);
                REPORTER_ASSERT(reporter, boxes[1].rect.left() == 30);
                REPORTER_ASSERT(reporter, boxes[2].rect.left() == (width == 80 ? 0 : 80));
            }
            REPORTER_ASSERT(reporter, boxes[0].rect.top() == boxes[1].rect.top());
            if (width == 80) {
                REPORTER_ASSERT(reporter, boxes[2].rect.top() > boxes[1].rect.top());
            } else {
                REPORTER_ASSERT(reporter, boxes[2].rect.top() == boxes[1].rect.top());
            }
        }
    }
}

UNIX_ONLY_TEST(SkParagraph_PlaceholderLogicalOrderMixedDirections, reporter) {
    for (auto direction : {TextDirection::kLtr, TextDirection::kRtl}) {
        auto fonts = sk_make_sp<FontCollection>();
        fonts->setDefaultFontManager(ToolUtils::TestFontMgr());
        ParagraphStyle style;
        style.setTextDirection(direction);
        ParagraphBuilderImpl builder(style, fonts, get_unicode());
        auto add = [&builder](SkScalar width) {
            builder.addPlaceholder(PlaceholderStyle(
                    width, 20, PlaceholderAlignment::kBottom, TextBaseline::kAlphabetic, 0));
        };
        add(30);
        // Put two placeholders in an embedding opposite to the paragraph direction.
        builder.addText(direction == TextDirection::kRtl ? u"\u202a" : u"\u202b");
        add(50);
        add(70);
        builder.addText(u"\u202c");
        add(90);
        auto paragraph = builder.Build();
        paragraph->layout(500);
        auto boxes = paragraph->getRectsForPlaceholders();
        REPORTER_ASSERT(reporter, boxes.size() == 4);
        if (boxes.size() != 4) {
            continue;
        }
        const SkScalar ltrLeft[] = {0, 100, 30, 150};
        const SkScalar rtlLeft[] = {470, 350, 400, 260};
        const unsigned offsets[] = {0, 2, 3, 5};
        for (size_t i = 0; i < boxes.size(); ++i) {
            REPORTER_ASSERT(reporter,
                            boxes[i].rect.left() ==
                                    (direction == TextDirection::kRtl ? rtlLeft[i] : ltrLeft[i]));
            REPORTER_ASSERT(reporter, boxes[i].rect.width() == 30 + 20 * i);
            auto range = paragraph->getRectsForRange(
                    offsets[i], offsets[i] + 1, RectHeightStyle::kTight, RectWidthStyle::kTight);
            REPORTER_ASSERT(reporter, range.size() == 1);
            if (range.size() == 1) {
                REPORTER_ASSERT(reporter, range[0].rect == boxes[i].rect);
                REPORTER_ASSERT(reporter, range[0].direction == boxes[i].direction);
            }
            const auto& box = boxes[i];
            SkScalar startX = box.direction == TextDirection::kRtl
                                      ? box.rect.right() - box.rect.width() / 4
                                      : box.rect.left() + box.rect.width() / 4;
            auto position = paragraph->getGlyphPositionAtCoordinate(startX, box.rect.centerY());
            REPORTER_ASSERT(reporter,
                            position.position == SkToS32(offsets[i]),
                            "base=%d placeholder=%zu expected=%u actual=%d x=%g direction=%d",
                            static_cast<int>(direction),
                            i,
                            offsets[i],
                            position.position,
                            startX,
                            static_cast<int>(box.direction));
            SkScalar endX = box.rect.left() + box.rect.right() - startX;
            auto endPosition = paragraph->getGlyphPositionAtCoordinate(endX, box.rect.centerY());
            REPORTER_ASSERT(reporter, endPosition.position == SkToS32(offsets[i] + 1));
        }
    }
}

UNIX_ONLY_TEST(SkParagraph_PlaceholderLogicalOrderMaxLines, reporter) {
    ParagraphStyle style;
    style.setTextDirection(TextDirection::kRtl);
    style.setMaxLines(1);
    ParagraphBuilderImpl builder(style, sk_make_sp<FontCollection>(), get_unicode());
    for (SkScalar width : {30, 50, 70}) {
        builder.addPlaceholder(PlaceholderStyle(
                width, 20, PlaceholderAlignment::kBottom, TextBaseline::kAlphabetic, 0));
    }
    auto paragraph = builder.Build();
    paragraph->layout(80);
    auto boxes = paragraph->getRectsForPlaceholders();
    REPORTER_ASSERT(reporter, boxes.size() == 2);
    if (boxes.size() == 2) {
        REPORTER_ASSERT(reporter, boxes[0].rect.left() == 50);
        REPORTER_ASSERT(reporter, boxes[0].rect.width() == 30);
        REPORTER_ASSERT(reporter, boxes[1].rect.left() == 0);
        REPORTER_ASSERT(reporter, boxes[1].rect.width() == 50);
    }
}

UNIX_ONLY_TEST(SkParagraph_PlaceholderLogicalOrderLanguages, reporter) {
    auto fonts = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fonts)
    struct Sample {
        std::u16string word;
        TextDirection direction;
    };
    const Sample russian{u"мир", TextDirection::kLtr};
    const Sample arabic{u"نص", TextDirection::kRtl};
    const std::vector<std::vector<Sample>> samples = {{russian}, {arabic}, {russian, arabic}};
    for (const auto& sample : samples) {
        for (auto direction : {TextDirection::kLtr, TextDirection::kRtl}) {
            ParagraphStyle style;
            style.setTextDirection(direction);
            TextStyle textStyle;
            textStyle.setFontSize(10);
            textStyle.setFontStyle(
                    SkFontStyle(500, SkFontStyle::kNormal_Width, SkFontStyle::kUpright_Slant));
            // Reuse fonts already present in the skparagraph asset.
            textStyle.setFontFamilies({SkString("Roboto"), SkString("Noto Naskh Arabic")});
            style.setTextStyle(textStyle);
            ParagraphBuilderImpl builder(style, fonts, get_unicode());
            std::vector<unsigned> offsets;
            unsigned offset = 0;
            for (const auto& script : sample) {
                for (int i = 0; i < 3; ++i) {
                    auto text = script.word + u" ";
                    builder.addText(text);
                    offset += text.size();
                    if (i < 2) {
                        offsets.push_back(offset++);
                        builder.addPlaceholder(PlaceholderStyle(10 + 10 * offsets.size(),
                                                                20,
                                                                PlaceholderAlignment::kBottom,
                                                                TextBaseline::kAlphabetic,
                                                                0));
                    }
                }
            }
            auto paragraph = builder.Build();
            for (SkScalar width : {800, 80, 800}) {
                paragraph->layout(width);
                // The pinned font asset must cover all scripts, with no system fallback.
                REPORTER_ASSERT(reporter, paragraph->unresolvedGlyphs() == 0);
                auto boxes = paragraph->getRectsForPlaceholders();
                REPORTER_ASSERT(reporter, boxes.size() == offsets.size());
                if (boxes.size() != offsets.size()) {
                    continue;
                }
                for (size_t i = 0; i < boxes.size(); ++i) {
                    REPORTER_ASSERT(
                            reporter,
                            SkScalarNearlyEqual(boxes[i].rect.width(), 20 + 10 * i, EPSILON1000));
                    REPORTER_ASSERT(reporter, boxes[i].direction == sample[i / 2].direction);
                    auto range = paragraph->getRectsForRange(offsets[i],
                                                             offsets[i] + 1,
                                                             RectHeightStyle::kTight,
                                                             RectWidthStyle::kTight);
                    REPORTER_ASSERT(reporter, range.size() == 1);
                    if (range.size() == 1) {
                        REPORTER_ASSERT(reporter, range[0].rect == boxes[i].rect);
                    }
                    const auto& box = boxes[i];
                    SkScalar startX = box.direction == TextDirection::kRtl
                                              ? box.rect.right() - box.rect.width() / 4
                                              : box.rect.left() + box.rect.width() / 4;
                    auto start =
                            paragraph->getGlyphPositionAtCoordinate(startX, box.rect.centerY());
                    auto end = paragraph->getGlyphPositionAtCoordinate(
                            box.rect.left() + box.rect.right() - startX, box.rect.centerY());
                    REPORTER_ASSERT(reporter, start.position == SkToS32(offsets[i]));
                    REPORTER_ASSERT(reporter, end.position == SkToS32(offsets[i] + 1));
                    if (width == 800 && i % 2 == 0) {
                        REPORTER_ASSERT(reporter,
                                        (boxes[i].rect.left() < boxes[i + 1].rect.left()) ==
                                                (sample[i / 2].direction == TextDirection::kLtr));
                    }
                }
            }
        }
    }
}

UNIX_ONLY_TEST(SkParagraph_InlinePlaceholderParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    TestCanvas canvas("SkParagraph_InlinePlaceholderParagraph.png");
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
            50, 50, PlaceholderAlignment::kBaseline, TextBaseline::kAlphabetic, 0);
    builder.addPlaceholder(placeholder1);
    builder.addText(text, len);
    builder.addPlaceholder(placeholder1);

    PlaceholderStyle placeholder2(
            5, 50, PlaceholderAlignment::kBaseline, TextBaseline::kAlphabetic, 50);
    builder.addPlaceholder(placeholder2);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder2);
    builder.addText(text, len);
    builder.addPlaceholder(placeholder2);
    builder.addText(text, len);
    builder.addText(text, len);
    builder.addPlaceholder(placeholder2);
    builder.addPlaceholder(placeholder2);
    builder.addPlaceholder(placeholder2);
    builder.addPlaceholder(placeholder2);
    builder.addPlaceholder(placeholder2);
    builder.addPlaceholder(placeholder1);
    builder.addText(text, len);
    builder.addText(text, len);
    builder.addText(text, len);
    builder.addText(text, len);
    builder.addText(text, len);
    builder.addPlaceholder(placeholder2);
    builder.addPlaceholder(placeholder1);
    builder.addText(text, len);
    builder.addText(text, len);

    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);
    paragraph->paint(canvas.get(), 0, 0);

    RectHeightStyle rect_height_style = RectHeightStyle::kTight;
    RectWidthStyle rect_width_style = RectWidthStyle::kTight;

    auto boxes = paragraph->getRectsForRange(0, 3, rect_height_style, rect_width_style);
    canvas.drawRects(SK_ColorRED, boxes);
    REPORTER_ASSERT(reporter, boxes.size() == 1);

    boxes = paragraph->getRectsForRange(0, 3, rect_height_style, rect_width_style);
    canvas.drawRects(SK_ColorGREEN, boxes);
    REPORTER_ASSERT(reporter, boxes.size() == 1);

    boxes = paragraph->getRectsForPlaceholders();
    canvas.drawRects(SK_ColorRED, boxes);

    boxes = paragraph->getRectsForRange(4, 17, rect_height_style, rect_width_style);
    canvas.drawRects(SK_ColorBLUE, boxes);

    REPORTER_ASSERT(reporter, boxes.size() == 7);
    if (boxes.size() >= 7) {
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[1].rect.left(), 90.921f, EPSILON2));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[1].rect.top(), 50, EPSILON100));
        REPORTER_ASSERT(reporter,
                        SkScalarNearlyEqual(boxes[1].rect.right(), 90.921f + 50, EPSILON2));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[1].rect.bottom(), 100, EPSILON100));

        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[3].rect.left(), 231.343f, EPSILON2));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[3].rect.top(), 50, EPSILON100));
        REPORTER_ASSERT(reporter,
                        SkScalarNearlyEqual(boxes[3].rect.right(), 231.343f + 50, EPSILON2));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[3].rect.bottom(), 100, EPSILON100));

        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[4].rect.left(), 281.343f, EPSILON2));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[4].rect.top(), 0, EPSILON100));
        REPORTER_ASSERT(reporter,
                        SkScalarNearlyEqual(boxes[4].rect.right(), 281.343f + 5, EPSILON2));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[4].rect.bottom(), 50, EPSILON100));

        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[6].rect.left(), 336.343f, EPSILON2));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[6].rect.top(), 0, EPSILON100));
        REPORTER_ASSERT(reporter,
                        SkScalarNearlyEqual(boxes[6].rect.right(), 336.343f + 5, EPSILON2));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[6].rect.bottom(), 50, EPSILON100));
    }
}

UNIX_ONLY_TEST(SkParagraph_InlinePlaceholderBaselineParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    TestCanvas canvas("SkParagraph_InlinePlaceholderBaselineParagraph.png");
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

    PlaceholderStyle placeholder(
            55, 50, PlaceholderAlignment::kBaseline, TextBaseline::kAlphabetic, 38.347f);
    builder.addPlaceholder(placeholder);
    builder.addText(text, len);

    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);
    paragraph->paint(canvas.get(), 0, 0);

    auto boxes = paragraph->getRectsForPlaceholders();
    canvas.drawRects(SK_ColorRED, boxes);

    REPORTER_ASSERT(reporter, boxes.size() == 1);
    if (boxes.size() >= 1) {
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 90.921f, EPSILON2));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 0, EPSILON100));
        REPORTER_ASSERT(reporter,
                        SkScalarNearlyEqual(boxes[0].rect.right(), 90.921f + 55, EPSILON2));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 50, EPSILON100));
    }

    RectHeightStyle rect_height_style = RectHeightStyle::kTight;
    RectWidthStyle rect_width_style = RectWidthStyle::kTight;

    boxes = paragraph->getRectsForRange(5, 6, rect_height_style, rect_width_style);
    canvas.drawRects(SK_ColorBLUE, boxes);

    REPORTER_ASSERT(reporter, boxes.size() == 1);
    if (boxes.size() >= 1) {
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 75.324f, EPSILON2));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 14.226f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 90.921f, EPSILON2));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 44.694f, EPSILON100));
    }
}

UNIX_ONLY_TEST(SkParagraph_InlinePlaceholderAboveBaselineParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    TestCanvas canvas("SkParagraph_InlinePlaceholderAboveBaselineParagraph.png");
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

    PlaceholderStyle placeholder(55,
                                 50,
                                 PlaceholderAlignment::kAboveBaseline,
                                 TextBaseline::kAlphabetic,
                                 903129.129308f);
    builder.addPlaceholder(placeholder);
    builder.addText(text, len);

    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);
    paragraph->paint(canvas.get(), 0, 0);

    auto boxes = paragraph->getRectsForPlaceholders();
    canvas.drawRects(SK_ColorRED, boxes);

    REPORTER_ASSERT(reporter, boxes.size() == 1);
    if (boxes.size() >= 1) {
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 90.921f, EPSILON2));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), -0.347f, EPSILON100));
        REPORTER_ASSERT(reporter,
                        SkScalarNearlyEqual(boxes[0].rect.right(), 90.921f + 55, EPSILON2));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 49.652f, EPSILON100));
    }

    RectHeightStyle rect_height_style = RectHeightStyle::kTight;
    RectWidthStyle rect_width_style = RectWidthStyle::kTight;

    boxes = paragraph->getRectsForRange(5, 6, rect_height_style, rect_width_style);
    canvas.drawRects(SK_ColorBLUE, boxes);

    REPORTER_ASSERT(reporter, boxes.size() == 1);
    if (boxes.size() >= 1) {
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 75.324f, EPSILON2));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 25.531f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 90.921f, EPSILON2));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 56, EPSILON100));
    }
}

UNIX_ONLY_TEST(SkParagraph_InlinePlaceholderBelowBaselineParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    TestCanvas canvas("SkParagraph_InlinePlaceholderBelowBaselineParagraph.png");
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

    PlaceholderStyle placeholder(55,
                                 50,
                                 PlaceholderAlignment::kBelowBaseline,
                                 TextBaseline::kAlphabetic,
                                 903129.129308f);
    builder.addPlaceholder(placeholder);
    builder.addText(text, len);

    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);
    paragraph->paint(canvas.get(), 0, 0);

    auto boxes = paragraph->getRectsForPlaceholders();
    canvas.drawRects(SK_ColorRED, boxes);

    REPORTER_ASSERT(reporter, boxes.size() == 1);
    if (boxes.size() >= 1) {
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 90.921f, EPSILON2));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 24, EPSILON100));
        REPORTER_ASSERT(reporter,
                        SkScalarNearlyEqual(boxes[0].rect.right(), 90.921f + 55, EPSILON2));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 74, EPSILON100));
    }

    RectHeightStyle rect_height_style = RectHeightStyle::kTight;
    RectWidthStyle rect_width_style = RectWidthStyle::kTight;

    boxes = paragraph->getRectsForRange(5, 6, rect_height_style, rect_width_style);
    canvas.drawRects(SK_ColorBLUE, boxes);

    REPORTER_ASSERT(reporter, boxes.size() == 1);
    if (boxes.size() >= 1) {
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 75.324f, EPSILON2));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), -0.121f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 90.921f, EPSILON2));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 30.347f, EPSILON100));
    }
}

UNIX_ONLY_TEST(SkParagraph_InlinePlaceholderBottomParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    TestCanvas canvas("SkParagraph_InlinePlaceholderBottomParagraph.png");
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

    PlaceholderStyle placeholder(
            55, 50, PlaceholderAlignment::kBottom, TextBaseline::kAlphabetic, 0);
    builder.addPlaceholder(placeholder);
    builder.addText(text, len);

    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);
    paragraph->paint(canvas.get(), 0, 0);

    RectHeightStyle rect_height_style = RectHeightStyle::kTight;
    RectWidthStyle rect_width_style = RectWidthStyle::kTight;

    auto boxes = paragraph->getRectsForPlaceholders();
    canvas.drawRects(SK_ColorRED, boxes);
    REPORTER_ASSERT(reporter, boxes.size() == 1);
    if (boxes.size() >= 1) {
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 90.921f, EPSILON50));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 0, EPSILON100));
        REPORTER_ASSERT(reporter,
                        SkScalarNearlyEqual(boxes[0].rect.right(), 90.921f + 55, EPSILON50));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 50, EPSILON100));
    }

    boxes = paragraph->getRectsForRange(0, 1, rect_height_style, rect_width_style);
    canvas.drawRects(SK_ColorBLUE, boxes);
    REPORTER_ASSERT(reporter, boxes.size() == 1);
    if (boxes.size() >= 1) {
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 0.5f, EPSILON50));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 19.531f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 16.097f, EPSILON50));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 50, EPSILON100));
    }
}

UNIX_ONLY_TEST(SkParagraph_InlinePlaceholderTopParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    TestCanvas canvas("SkParagraph_InlinePlaceholderTopParagraph.png");
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

    PlaceholderStyle placeholder(55, 50, PlaceholderAlignment::kTop, TextBaseline::kAlphabetic, 0);
    builder.addPlaceholder(placeholder);
    builder.addText(text, len);

    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);
    paragraph->paint(canvas.get(), 0, 0);

    RectHeightStyle rect_height_style = RectHeightStyle::kTight;
    RectWidthStyle rect_width_style = RectWidthStyle::kTight;

    auto boxes = paragraph->getRectsForPlaceholders();
    canvas.drawRects(SK_ColorRED, boxes);
    REPORTER_ASSERT(reporter, boxes.size() == 1);
    if (boxes.size() >= 1) {
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 90.921f, EPSILON50));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 0, EPSILON100));
        REPORTER_ASSERT(reporter,
                        SkScalarNearlyEqual(boxes[0].rect.right(), 90.921f + 55, EPSILON50));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 50, EPSILON100));
    }

    boxes = paragraph->getRectsForRange(0, 1, rect_height_style, rect_width_style);
    canvas.drawRects(SK_ColorBLUE, boxes);
    REPORTER_ASSERT(reporter, boxes.size() == 1);
    if (boxes.size() >= 1) {
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 0.5f, EPSILON50));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 0, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 16.097f, EPSILON50));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 30.468f, EPSILON100));
    }
}

UNIX_ONLY_TEST(SkParagraph_InlinePlaceholderMiddleParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    TestCanvas canvas("SkParagraph_InlinePlaceholderMiddleParagraph.png");
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

    PlaceholderStyle placeholder(
            55, 50, PlaceholderAlignment::kMiddle, TextBaseline::kAlphabetic, 0);
    builder.addPlaceholder(placeholder);
    builder.addText(text, len);

    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);
    paragraph->paint(canvas.get(), 0, 0);

    RectHeightStyle rect_height_style = RectHeightStyle::kTight;
    RectWidthStyle rect_width_style = RectWidthStyle::kTight;

    auto boxes = paragraph->getRectsForPlaceholders();
    canvas.drawRects(SK_ColorRED, boxes);
    REPORTER_ASSERT(reporter, boxes.size() == 1);
    if (boxes.size() >= 1) {
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 90.921f, EPSILON50));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 0, EPSILON100));
        REPORTER_ASSERT(reporter,
                        SkScalarNearlyEqual(boxes[0].rect.right(), 90.921f + 55, EPSILON50));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 50, EPSILON100));
    }

    boxes = paragraph->getRectsForRange(5, 6, rect_height_style, rect_width_style);
    canvas.drawRects(SK_ColorBLUE, boxes);
    REPORTER_ASSERT(reporter, boxes.size() == 1);
    if (boxes.size() >= 1) {
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 75.324f, EPSILON50));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 9.765f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 90.921f, EPSILON50));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 40.234f, EPSILON100));
    }
}

UNIX_ONLY_TEST(SkParagraph_InlinePlaceholderIdeographicBaselineParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    TestCanvas canvas("SkParagraph_InlinePlaceholderIdeographicBaselineParagraph.png");
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    const char* text = "給能上目秘使";
    const size_t len = strlen(text);

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    paragraph_style.setMaxLines(14);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Source Han Serif CN")});
    text_style.setColor(SK_ColorBLACK);
    text_style.setFontSize(26);
    text_style.setWordSpacing(5);
    text_style.setLetterSpacing(1);
    text_style.setDecoration(TextDecoration::kUnderline);
    text_style.setDecorationColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text, len);
    PlaceholderStyle placeholder(
            55, 50, PlaceholderAlignment::kBaseline, TextBaseline::kIdeographic, 38.347f);
    builder.addPlaceholder(placeholder);
    builder.addText(text, len);

    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);
    paragraph->paint(canvas.get(), 0, 0);

    RectHeightStyle rect_height_style = RectHeightStyle::kTight;
    RectWidthStyle rect_width_style = RectWidthStyle::kTight;

    auto boxes = paragraph->getRectsForPlaceholders();
    canvas.drawRects(SK_ColorRED, boxes);
    REPORTER_ASSERT(reporter, boxes.size() == 1);
    if (boxes.size() >= 1) {
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 162.5f, EPSILON50));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 0, EPSILON100));
        REPORTER_ASSERT(reporter,
                        SkScalarNearlyEqual(boxes[0].rect.right(), 162.5f + 55, EPSILON50));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 50, EPSILON100));
    }

    boxes = paragraph->getRectsForRange(5, 6, rect_height_style, rect_width_style);
    canvas.drawRects(SK_ColorBLUE, boxes);
    REPORTER_ASSERT(reporter, boxes.size() == 1);
    if (boxes.size() >= 1) {
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 135.5f, EPSILON50));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 4.703f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 162.5f, EPSILON50));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 42.065f, EPSILON100));
    }
}

UNIX_ONLY_TEST(SkParagraph_InlinePlaceholderBreakParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    TestCanvas canvas("SkParagraph_InlinePlaceholderBreakParagraph.png");
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
            25, 25, PlaceholderAlignment::kBaseline, TextBaseline::kAlphabetic, 12.5f);

    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder2);
    builder.addPlaceholder(placeholder1);
    builder.addText(text, len);

    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder2);  // 4 + 1
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder2);  // 6 + 1
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder2);  // 7 + 1

    builder.addPlaceholder(placeholder1);
    builder.addText(text, len);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder2);

    builder.addText(text, len);
    builder.addText(text, len);
    builder.addText(text, len);
    builder.addText(text, len);

    builder.addPlaceholder(placeholder2);
    builder.addPlaceholder(placeholder1);

    builder.addText(text, len);

    builder.addPlaceholder(placeholder2);

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
    builder.addText(text, len);
    builder.addText(text, len);
    builder.addText(text, len);
    builder.addText(text, len);
    builder.addText(text, len);
    builder.addText(text, len);
    builder.addText(text, len);
    builder.addText(text, len);
    builder.addText(text, len);

    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth - 100);
    paragraph->paint(canvas.get(), 0, 0);

    RectHeightStyle rect_height_style = RectHeightStyle::kTight;
    RectWidthStyle rect_width_style = RectWidthStyle::kTight;

    auto boxes = paragraph->getRectsForRange(0, 3, rect_height_style, rect_width_style);
    canvas.drawRects(SK_ColorRED, boxes);
    REPORTER_ASSERT(reporter, boxes.size() == 1);

    boxes = paragraph->getRectsForRange(175, 176, rect_height_style, rect_width_style);
    canvas.drawRects(SK_ColorGREEN, boxes);
    REPORTER_ASSERT(reporter, boxes.size() == 1);
    if (boxes.size() >= 1) {
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 31.695f, EPSILON50));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 218.531f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 47.292f, EPSILON50));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 249, EPSILON100));
    }

    boxes = paragraph->getRectsForPlaceholders();
    canvas.drawRects(SK_ColorRED, boxes);

    boxes = paragraph->getRectsForRange(4, 45, rect_height_style, rect_width_style);
    canvas.drawRects(SK_ColorBLUE, boxes);
    REPORTER_ASSERT(reporter, boxes.size() == 30);
    if (boxes.size() >= 30) {
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 59.726f, EPSILON50));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 26.378f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 90.921f, EPSILON50));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 56.847f, EPSILON100));

        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[11].rect.left(), 606.343f, EPSILON20));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[11].rect.top(), 38, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[11].rect.right(), 631.343f, EPSILON20));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[11].rect.bottom(), 63, EPSILON100));

        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[17].rect.left(), 0.5f, EPSILON50));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[17].rect.top(), 63.5f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[17].rect.right(), 50.5f, EPSILON50));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[17].rect.bottom(), 113.5f, EPSILON100));
    }
}

// This test does not produce an image
UNIX_ONLY_TEST(SkParagraph_PlaceholderOnly, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    ParagraphStyle paragraph_style;
    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setBackgroundColor(SkPaint(SkColors::kRed));
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());

    PlaceholderStyle placeholder(
            0, 0, PlaceholderAlignment::kBaseline, TextBaseline::kAlphabetic, 0);
    builder.addPlaceholder(placeholder);

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);
    auto result = paragraph->getRectsForPlaceholders();
    REPORTER_ASSERT(reporter, result.size() == 1);
}

DEF_TEST_DISABLED(SkParagraph_PlaceholderHeightInf, reporter) {
    TestCanvas canvas("SkParagraph_PlaceholderHeightInf.png");

    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Ahem")});
    text_style.setColor(SK_ColorBLACK);
    text_style.setFontSize(14);

    PlaceholderStyle placeholder_style;
    placeholder_style.fWidth = 16.0f;
    placeholder_style.fHeight = SK_ScalarInfinity;
    placeholder_style.fAlignment = PlaceholderAlignment::kBottom;
    placeholder_style.fBaseline = TextBaseline::kAlphabetic;
    placeholder_style.fBaselineOffset = SK_ScalarInfinity;

    ParagraphStyle paragraph_style;
    //paragraph_style.setDrawOptions(DrawOptions::kRecord);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());
    builder.pushStyle(text_style);
    builder.addText("Limited by budget");
    builder.addPlaceholder(placeholder_style);
    auto paragraph = builder.Build();
    paragraph->layout(SK_ScalarInfinity);
    paragraph->paint(canvas.get(), 0, 0);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, SkIsFinite(impl->getPicture()->cullRect().height()));
    REPORTER_ASSERT(reporter, SkIsFinite(impl->getPicture()->cullRect().width()));
}

UNIX_ONLY_TEST(SkParagraph_PlaceholderWidth, reporter) {

    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    TestCanvas canvas("SkParagraph_PlaceholderWidth.png");

    const char* text = "1 23 456 7890";  // 13 * 50 = 650

    ParagraphStyle paragraph_style;
    TextStyle text_style;
    text_style.setColor(SK_ColorBLACK);
    text_style.setFontSize(50);
    text_style.setFontFamilies({SkString("Ahem")});
    PlaceholderStyle placeholder(
            300, 50, PlaceholderAlignment::kBaseline, TextBaseline::kAlphabetic, 0);

    auto draw = [&](bool withPlaceholder) {
        ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());
        builder.pushStyle(text_style);
        builder.addText(text);
        if (withPlaceholder) {
            SkPaint red;
            red.setColor(SK_ColorRED);
            text_style.setBackgroundColor(red);
            builder.pushStyle(text_style);
            builder.addPlaceholder(placeholder);
        }
        builder.addText(text);

        auto paragraph = builder.Build();
        paragraph->layout(950);
        paragraph->paint(canvas.get(), 0, 0);
        canvas.get()->translate(0, paragraph->getHeight());
        return paragraph->getMinIntrinsicWidth();
    };

    auto len1 = draw(true);
    auto len2 = draw(false);

    // placeholder: 300 "78901": 250
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(len1, 300.0f, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(len2, 250.0f, EPSILON100));
}

UNIX_ONLY_TEST(SkParagraph_PlaceholderPosition, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    TestCanvas canvas("SkParagraph_PlaceholderPosition.png");
    canvas.get()->translate(100, 100);

    TextStyle text_style;
    text_style.setColor(SK_ColorBLACK);
    text_style.setFontFamilies({SkString("Ahem")});
    text_style.setFontSize(10.0f);
    ParagraphStyle paragraph_style;
    paragraph_style.setTextStyle(text_style);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());
    builder.pushStyle(text_style);
    builder.addText("abcd");

    PlaceholderStyle placeholder_style;
    placeholder_style.fHeight = 10;
    placeholder_style.fWidth = 10;
    placeholder_style.fBaseline = TextBaseline::kAlphabetic;
    placeholder_style.fAlignment = PlaceholderAlignment::kBottom;
    builder.addPlaceholder(placeholder_style);

    auto paragraph = builder.Build();
    paragraph->layout(500);
    paragraph->paint(canvas.get(), 0, 0);

    auto result = paragraph->getGlyphPositionAtCoordinate(41.0f, 0.0f);
    REPORTER_ASSERT(reporter, result.position == 4 && result.affinity == Affinity::kDownstream);
}

UNIX_ONLY_TEST(SkParagraph_SingleDummyPlaceholder, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    const char* text = "Single dummy placeholder";
    const size_t len = strlen(text);

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text, len);

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, impl->placeholders().size() == 1);

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
