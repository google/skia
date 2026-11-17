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
#include "include/core/SkFontStyle.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
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
#include "modules/skparagraph/include/TypefaceFontProvider.h"
#include "modules/skparagraph/src/ParagraphBuilderImpl.h"
#include "modules/skparagraph/src/ParagraphImpl.h"
#include "modules/skparagraph/src/Run.h"
#include "modules/skparagraph/src/TextLine.h"
#include "modules/skparagraph/tests/SkParagraphTestUtils.h"
#include "tests/Test.h"
#include "tools/Resources.h"
#include "tools/fonts/FontToolUtils.h"

#include <string.h>
#include <string>
#include <utility>
#include <vector>

using namespace skia::textlayout;
using namespace skia::textlayout::test;

UNIX_ONLY_TEST(SkParagraph_LeadingSpaceRTL, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>(true);
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    TestCanvas canvas("SkParagraph_LeadingSpaceRTL.png");

    const char* text = " leading space";
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

    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setAntiAlias(true);
    paint.setStrokeWidth(1);

    // Tests for GetRectsForRange()
    RectHeightStyle rect_height_style = RectHeightStyle::kMax;
    RectWidthStyle rect_width_style = RectWidthStyle::kTight;
    paint.setColor(SK_ColorRED);
    auto boxes = paragraph->getRectsForRange(0, 100, rect_height_style, rect_width_style);
    for (size_t i = 0; i < boxes.size(); ++i) {
        canvas.get()->drawRect(boxes[i].rect, paint);
    }
    REPORTER_ASSERT(reporter, boxes.size() == 2ull);
}

// Checked: disabled for TxtLib
UNIX_ONLY_TEST(SkParagraph_ArabicParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    TestCanvas canvas("SkParagraph_ArabicParagraph.png");
    const char* text =
            "من أسر وإعلان الخاصّة وهولندا،, عل قائمة الضغوط بالمطالبة تلك. الصفحة "
            "بمباركة التقليدية قام عن. تصفح";
    const size_t len = strlen(text);

    ParagraphStyle paragraph_style;
    paragraph_style.setMaxLines(14);
    paragraph_style.setTextAlign(TextAlign::kJustify);
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());

    auto decoration = (TextDecoration)(TextDecoration::kUnderline | TextDecoration::kOverline |
                                       TextDecoration::kLineThrough);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Katibeh")});
    text_style.setFontSize(35);
    text_style.setColor(SK_ColorBLACK);
    text_style.setLetterSpacing(2);
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
    REPORTER_ASSERT(reporter, impl->lines().size() == 2);
    REPORTER_ASSERT(reporter, impl->styles().size() == 1);
    if (impl->styles().size() >= 1) {
        REPORTER_ASSERT(reporter, impl->styles()[0].fStyle.equals(text_style));
    }
}

// Checked DIFF+
// This test shows now 2 boxes for [36:40) range:
// [36:38) for arabic text and [38:39) for the last space
// that has default paragraph direction (LTR) and is placed at the end of the paragraph
UNIX_ONLY_TEST(SkParagraph_ArabicRectsLTRLeftAlignParagraph, reporter) {

    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    TestCanvas canvas("SkParagraph_ArabicRectsLTRLeftAlignParagraph.png");
    const char* text = "Helloبمباركة التقليدية قام عن. تصفح يد ";
    const size_t len = strlen(text);

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    paragraph_style.setMaxLines(14);
    paragraph_style.setTextAlign(TextAlign::kLeft);
    paragraph_style.setTextDirection(TextDirection::kLtr);
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
    REPORTER_ASSERT(reporter, impl->runs().size() == 3);

    paragraph->paint(canvas.get(), 0, 0);

    RectHeightStyle rect_height_style = RectHeightStyle::kMax;
    RectWidthStyle rect_width_style = RectWidthStyle::kTight;
    // There are 39 codepoints: [0:39); asking for [36:40) would give the same as for [36:39)
    std::vector<TextBox> boxes =
            paragraph->getRectsForRange(36, 40, rect_height_style, rect_width_style);
    canvas.drawRects(SK_ColorRED, boxes);

    REPORTER_ASSERT(reporter, boxes.size() == 2ull);
    if (boxes.size() >= 2) {
        REPORTER_ASSERT(
                reporter,
                SkScalarNearlyEqual(boxes[0].rect.left(), 65.65f, EPSILON100));  // DIFF: 89.40625
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), -0.27f, EPSILON100));
        REPORTER_ASSERT(
                reporter,
                SkScalarNearlyEqual(boxes[0].rect.right(), 86.89f, EPSILON100));  // DIFF: 121.87891
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 44.0f, EPSILON100));
    }
}

// Checked DIFF+
UNIX_ONLY_TEST(SkParagraph_ArabicRectsLTRRightAlignParagraph, reporter) {

    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    TestCanvas canvas("SkParagraph_ArabicRectsLTRRightAlignParagraph.png");
    const char* text = "Helloبمباركة التقليدية قام عن. تصفح يد ";
    const size_t len = strlen(text);

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    paragraph_style.setMaxLines(14);
    paragraph_style.setTextAlign(TextAlign::kRight);
    paragraph_style.setTextDirection(TextDirection::kLtr);
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
    REPORTER_ASSERT(reporter, impl->runs().size() == 3);

    paragraph->paint(canvas.get(), 0, 0);

    RectHeightStyle rect_height_style = RectHeightStyle::kMax;
    RectWidthStyle rect_width_style = RectWidthStyle::kTight;
    std::vector<TextBox> boxes =
            paragraph->getRectsForRange(36, 40, rect_height_style, rect_width_style);
    canvas.drawRects(SK_ColorRED, boxes);

    REPORTER_ASSERT(reporter, boxes.size() == 2ull);  // DIFF
    if (boxes.size() >= 2) {
        REPORTER_ASSERT(reporter,
                        SkScalarNearlyEqual(boxes[0].rect.left(), 561.1f, EPSILON100));  // DIFF
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), -0.27f, EPSILON100));
        REPORTER_ASSERT(reporter,
                        SkScalarNearlyEqual(boxes[0].rect.right(), 582.34f, EPSILON100));  // DIFF
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 44, EPSILON100));
    }
}

UNIX_ONLY_TEST(SkParagraph_Bidi1, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    fontCollection->setDefaultFontManager(ToolUtils::TestFontMgr());
    fontCollection->enableFontFallback();
    TestCanvas canvas("SkParagraph_Bidi1.png");

    std::u16string abc = u"\u202Dabc";
    std::u16string DEF = u"\u202EDEF";
    std::u16string ghi = u"\u202Dghi";
    std::u16string JKL = u"\u202EJKL";
    std::u16string mno = u"\u202Dmno";

    std::u16string abcDEFghiJKLmno = u"\u202Dabc\u202EDEF\u202Dghi\u202EJKL\u202Dmno";

    ParagraphStyle paragraph_style;
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());

    TextStyle text_style;
    text_style.setFontFamilies({SkString("sans-serif")});
    text_style.setFontSize(40);

    text_style.setColor(SK_ColorCYAN);
    text_style.setFontStyle(SkFontStyle(
            SkFontStyle::kThin_Weight, SkFontStyle::kNormal_Width, SkFontStyle::kUpright_Slant));
    builder.pushStyle(text_style);
    builder.addText(abc);

    text_style.setColor(SK_ColorGREEN);
    text_style.setFontStyle(SkFontStyle(
            SkFontStyle::kLight_Weight, SkFontStyle::kNormal_Width, SkFontStyle::kUpright_Slant));
    builder.pushStyle(text_style);
    builder.addText(DEF);

    text_style.setColor(SK_ColorYELLOW);
    text_style.setFontStyle(SkFontStyle(
            SkFontStyle::kNormal_Weight, SkFontStyle::kNormal_Width, SkFontStyle::kUpright_Slant));
    builder.pushStyle(text_style);
    builder.addText(ghi);

    text_style.setColor(SK_ColorMAGENTA);
    text_style.setFontStyle(SkFontStyle(
            SkFontStyle::kMedium_Weight, SkFontStyle::kNormal_Width, SkFontStyle::kUpright_Slant));
    builder.pushStyle(text_style);
    builder.addText(JKL);

    text_style.setColor(SK_ColorBLUE);
    text_style.setFontStyle(SkFontStyle(
            SkFontStyle::kBlack_Weight, SkFontStyle::kNormal_Width, SkFontStyle::kUpright_Slant));
    builder.pushStyle(text_style);
    builder.addText(mno);

    auto paragraph = builder.Build();
    paragraph->layout(400);
    paragraph->paint(canvas.get(), 0, 0);
}

UNIX_ONLY_TEST(SkParagraph_Bidi2, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    fontCollection->setDefaultFontManager(ToolUtils::TestFontMgr());
    fontCollection->enableFontFallback();
    TestCanvas canvas("SkParagraph_Bidi2.png");

    std::u16string abcD = u"\u202Dabc\u202ED";
    std::u16string EFgh = u"EF\u202Dgh";
    std::u16string iJKLmno = u"i\u202EJKL\u202Dmno";

    std::u16string abcDEFghiJKLmno = u"\u202Dabc\u202EDEF\u202Dghi\u202EJKL\u202Dmno";

    ParagraphStyle paragraph_style;
    TextStyle text_style;
    text_style.setFontFamilies({SkString("sans-serif")});
    text_style.setFontSize(40);
    text_style.setColor(SK_ColorBLACK);
    {
        ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());
        builder.pushStyle(text_style);
        builder.addText(abcD);
        builder.pushStyle(text_style);
        builder.addText(EFgh);
        builder.pushStyle(text_style);
        builder.addText(iJKLmno);
        auto paragraph = builder.Build();
        paragraph->layout(360);
        paragraph->paint(canvas.get(), 0, 0);
    }
    canvas.get()->translate(0, 400);
    {
        ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());
        builder.pushStyle(text_style);
        builder.addText(abcDEFghiJKLmno);
        auto paragraph = builder.Build();
        paragraph->layout(360);
        paragraph->paint(canvas.get(), 0, 0);
    }
}

UNIX_ONLY_TEST(SkParagraph_FontResolutionInRTL, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>(true);
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    TestCanvas canvas("SkParagraph_FontResolutionInRTL.png");
    const char* text = " אאא בּבּבּבּ אאאא בּבּ אאא בּבּבּ אאאאא בּבּבּבּ אאאא בּבּבּבּבּ ";
    const size_t len = strlen(text);

    ParagraphStyle paragraph_style;
    paragraph_style.setMaxLines(14);
    paragraph_style.setTextAlign(TextAlign::kRight);
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
    paragraph->layout(TestCanvasWidth);
    paragraph->paint(canvas.get(), 0, 0);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, impl->runs().size() == (10 + 11));
}

UNIX_ONLY_TEST(SkParagraph_FontResolutionInLTR, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>(true);
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    TestCanvas canvas("SkParagraph_FontResolutionInLTR.png");
    auto text = u"abc \u01A2 \u01A2 def";

    ParagraphStyle paragraph_style;
    paragraph_style.setMaxLines(14);
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setFontSize(26);
    text_style.setColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);
    paragraph->paint(canvas.get(), 0, 0);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, impl->runs().size() == 5);
    if (impl->runs().size() >= 5) {
        REPORTER_ASSERT(reporter, impl->runs()[0].textRange().width() == 4);  // "abc "
        REPORTER_ASSERT(reporter, impl->runs()[1].textRange().width() == 2);  // "{unresolved}"
        REPORTER_ASSERT(reporter, impl->runs()[2].textRange().width() == 1);  // " "
        REPORTER_ASSERT(reporter, impl->runs()[3].textRange().width() == 2);  // "{unresolved}"
        REPORTER_ASSERT(reporter, impl->runs()[4].textRange().width() == 4);  // " def"
    }
}

UNIX_ONLY_TEST(SkParagraph_RTL_With_Styles, reporter) {

    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    TestCanvas canvas("SkParagraph_RTL_With_Styles.png");

    SkPaint whiteSpaces;
    whiteSpaces.setColor(SK_ColorLTGRAY);

    SkPaint breakingSpace;
    breakingSpace.setColor(SK_ColorYELLOW);

    SkPaint text;
    text.setColor(SK_ColorWHITE);

    const char* arabic = "قففغغغغقففغغغغقففغغغ";

    ParagraphStyle paragraph_style;
    paragraph_style.setTextAlign(TextAlign::kRight);
    TextStyle text_style;
    text_style.setColor(SK_ColorBLACK);
    text_style.setFontFamilies({SkString("Roboto")});

    paragraph_style.setTextDirection(TextDirection::kRtl);
    paragraph_style.setTextAlign(TextAlign::kRight);
    text_style.setFontSize(20);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());
    text_style.setBackgroundColor(whiteSpaces);
    builder.pushStyle(text_style);
    builder.addText("   ");
    text_style.setBackgroundColor(text);
    builder.pushStyle(text_style);
    builder.addText(arabic);

    auto paragraph = builder.Build();
    paragraph->layout(300);
    paragraph->paint(canvas.get(), 0, 0);
}

UNIX_ONLY_TEST(SkParagraph_RTLGlyphPositions, reporter) {

    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    TestCanvas canvas("SkParagraph_RTLGlyphPositions.png");
    ParagraphStyle paragraph_style;
    paragraph_style.setTextDirection(TextDirection::kRtl);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());
    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setFontSize(20);
    text_style.setColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText("אאאא");
    builder.pop();
    auto paragraph = builder.Build();
    paragraph->layout(500);
    paragraph->paint(canvas.get(), 0, 0);

    std::vector<std::pair<SkScalar, PositionWithAffinity>> checks = {
            std::make_pair(550, PositionWithAffinity(0, Affinity::kDownstream)),
            std::make_pair(500, PositionWithAffinity(0, Affinity::kDownstream)),
            std::make_pair(494, PositionWithAffinity(1, Affinity::kUpstream)),
            std::make_pair(488, PositionWithAffinity(1, Affinity::kDownstream)),
            std::make_pair(485, PositionWithAffinity(2, Affinity::kUpstream)),
            std::make_pair(480, PositionWithAffinity(2, Affinity::kDownstream)),
            std::make_pair(475, PositionWithAffinity(3, Affinity::kUpstream)),
            std::make_pair(471, PositionWithAffinity(3, Affinity::kDownstream)),
            std::make_pair(467, PositionWithAffinity(4, Affinity::kUpstream)),
            std::make_pair(0, PositionWithAffinity(4, Affinity::kUpstream)),
    };

    for (auto check : checks) {
        auto pos = paragraph->getGlyphPositionAtCoordinate(check.first, 0);
        REPORTER_ASSERT(reporter, pos.affinity == check.second.affinity);
        REPORTER_ASSERT(reporter, pos.position == check.second.position);
    }
}

UNIX_ONLY_TEST(SkParagraph_LTRGlyphPositionsForTrailingSpaces, reporter) {

    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    TestCanvas canvas("SkParagraph_LTRGlyphPositionsForTrailingSpaces.png");

    ParagraphStyle paragraph_style;
    TextStyle text_style;
    text_style.setFontFamilies({SkString("Ahem")});
    text_style.setFontSize(10);
    text_style.setColor(SK_ColorBLACK);

    auto test = [&](const char* text) {
        auto str = straight(text);
        ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());
        builder.pushStyle(text_style);
        builder.addText(str);
        builder.pop();
        SkPaint gray;
        gray.setColor(SK_ColorGRAY);
        auto paragraph = builder.Build();
        paragraph->layout(100);
        canvas.get()->translate(0, 20);
        canvas.get()->drawRect(
                SkRect::MakeXYWH(0, 0, paragraph->getMaxIntrinsicWidth(), paragraph->getHeight()),
                gray);
        paragraph->paint(canvas.get(), 0, 0);
        canvas.get()->translate(0, paragraph->getHeight());

        for (size_t i = 0; i < str.size(); ++i) {
            auto res = paragraph->getGlyphPositionAtCoordinate(i * 10, 2);
            // There is a hidden codepoint at the beginning (to make it symmetric to RTL)
            REPORTER_ASSERT(reporter, res.position == SkToInt(i) + (i > 0 ? 1 : 0));
            // The ending looks slightly different...
            REPORTER_ASSERT(
                    reporter,
                    res.affinity == (res.position == SkToInt(str.size()) ? Affinity::kUpstream
                                                                         : Affinity::kDownstream));
        }
    };

    test("    ");
    test("hello               ");
}

UNIX_ONLY_TEST(SkParagraph_RTLGlyphPositionsForTrailingSpaces, reporter) {

    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    TestCanvas canvas("SkParagraph_RTLGlyphPositionsForTrailingSpaces.png");

    ParagraphStyle paragraph_style;
    paragraph_style.setTextDirection(TextDirection::kRtl);
    paragraph_style.setTextAlign(TextAlign::kRight);
    TextStyle text_style;
    text_style.setFontFamilies({SkString("Ahem")});
    text_style.setFontSize(10);
    text_style.setColor(SK_ColorBLACK);
    canvas.get()->translate(200, 0);

    auto test = [&](const char* text, int whitespaces) {
        auto str = mirror(text);
        ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());
        builder.pushStyle(text_style);
        builder.addText(str);
        builder.pop();
        SkPaint gray;
        gray.setColor(SK_ColorGRAY);
        auto paragraph = builder.Build();
        paragraph->layout(100);
        canvas.get()->translate(0, 20);
        auto res = paragraph->getRectsForRange(
                0, str.size(), RectHeightStyle::kTight, RectWidthStyle::kTight);
        bool even = true;
        for (auto& r : res) {
            if (even) {
                gray.setColor(SK_ColorGRAY);
            } else {
                gray.setColor(SK_ColorLTGRAY);
            }
            even = !even;
            canvas.get()->drawRect(r.rect, gray);
        }
        gray.setColor(SK_ColorRED);
        canvas.get()->drawRect(SkRect::MakeXYWH(0, 0, 1, paragraph->getHeight()), gray);
        paragraph->paint(canvas.get(), 0, 0);
        canvas.get()->translate(0, paragraph->getHeight());

        for (int i = 0; i < SkToInt(str.size()); ++i) {
            // Additional 1.0f to make sure the offset is not too close to the
            // edge of glyphs.
            auto pointX = (whitespaces + i) * 10.0f + 1.0f;
            auto pos = paragraph->getGlyphPositionAtCoordinate(pointX, 2);
            // At the beginning there is a control codepoint that makes the string RTL
            REPORTER_ASSERT(
                    reporter,
                    (pos.position + i) ==
                            SkToInt(str.size()) - (pos.affinity == Affinity::kDownstream ? 1 : 0));
        }
    };

    test("    ", 6);
    test("               hello", -10);
}

UNIX_ONLY_TEST(SkParagraph_RTLFollowedByLTR, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    TestCanvas canvas("SkParagraph_RTLFollowedByLTR.png");
    canvas.get()->translate(100, 100);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Ahem")});
    text_style.setFontSize(10);
    text_style.setColor(SK_ColorBLACK);

    ParagraphStyle paragraph_style;
    paragraph_style.setTextStyle(text_style);
    paragraph_style.setTextDirection(TextDirection::kLtr);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());
    builder.pushStyle(text_style);
    builder.addText(u"\u05D0\u05D0\u05D0ABC");
    auto paragraph = builder.Build();
    paragraph->layout(100);
    paragraph->paint(canvas.get(), 0, 0);

    auto boxes = paragraph->getRectsForRange(
            0, paragraph->getMaxWidth(), RectHeightStyle::kTight, RectWidthStyle::kTight);
    REPORTER_ASSERT(reporter, boxes.size() == 2);
    REPORTER_ASSERT(
            reporter,
            boxes[0].direction == TextDirection::kRtl && boxes[1].direction == TextDirection::kLtr);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.fLeft, 0.0f));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.fRight, boxes[1].rect.fLeft));
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(boxes[1].rect.fRight, paragraph->getMaxIntrinsicWidth()));

    std::vector<std::pair<SkScalar, PositionWithAffinity>> checks = {
            std::make_pair(-10, PositionWithAffinity(3, Affinity::kUpstream)),
            std::make_pair(0, PositionWithAffinity(3, Affinity::kUpstream)),
            std::make_pair(5, PositionWithAffinity(2, Affinity::kDownstream)),
            std::make_pair(10, PositionWithAffinity(2, Affinity::kUpstream)),
            std::make_pair(15, PositionWithAffinity(1, Affinity::kDownstream)),
            std::make_pair(20, PositionWithAffinity(1, Affinity::kUpstream)),
            std::make_pair(25, PositionWithAffinity(0, Affinity::kDownstream)),
            std::make_pair(30, PositionWithAffinity(3, Affinity::kDownstream)),
            std::make_pair(35, PositionWithAffinity(4, Affinity::kUpstream)),
            std::make_pair(40, PositionWithAffinity(4, Affinity::kDownstream)),
            std::make_pair(45, PositionWithAffinity(5, Affinity::kUpstream)),
            std::make_pair(50, PositionWithAffinity(5, Affinity::kDownstream)),
            std::make_pair(55, PositionWithAffinity(6, Affinity::kUpstream)),
            std::make_pair(60, PositionWithAffinity(6, Affinity::kUpstream)),
    };

    for (auto check : checks) {
        auto pos = paragraph->getGlyphPositionAtCoordinate(check.first, 0);
        REPORTER_ASSERT(reporter, pos.affinity == check.second.affinity);
        REPORTER_ASSERT(reporter, pos.position == check.second.position);
    }
}

UNIX_ONLY_TEST(SkParagraph_NonMonotonicGlyphsLTR, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    NEED_SYSTEM_FONTS(fontCollection)

    TestCanvas canvas("SkParagraph_NonMonotonicGlyphsLTR.png");
    std::u16string text =
            u"\u0068\u0301\u0350\u0312\u0357\u030C\u0369\u0305\u036C\u0304\u0310\u033F\u0366\u0350"
            u" ";
    /*
            u"\u0343\u0364\u0369\u0311\u0309\u030E\u0365\u031B\u0340\u0337\u0335\u035E\u0334\u0328"
            u"\u0360\u0360\u0315\u035F\u0340\u0340\u0362\u0360\u0322\u031B\u031B\u0337\u0340\u031E"
            u"\u031F\u032A\u0331\u0345\u032F\u0332\u032E\u0333\u0353\u0320\u0345\u031C\u031F\u033C"
            u"\u0325\u0355\u032C\u0325\u033Aa\u0307\u0312\u034B\u0308\u0312\u0346\u0313\u0346\u0304"
            u"\u0307\u0344\u0305\u0342\u0368\u0346\u036A\u035B\u030F\u0365\u0307\u0340\u0328\u0322"
            u"\u0361\u0489\u034F\u0328\u0334\u035F\u0335\u0362\u0489\u0360\u0358\u035E\u0360\u035D"
            u"\u0341\u0337\u0337\u032E\u0326\u032D\u0359\u0318\u033C\u032F\u0333\u035A\u034D\u0319"
            u"\u031C\u0353\u033C\u0345\u0359\u0331\u033B\u0331\u033C";
            */

    TextStyle text_style;
    text_style.setFontSize(14);
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setColor(SK_ColorBLACK);

    ParagraphStyle paragraph_style;
    paragraph_style.setTextStyle(text_style);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());

    builder.pushStyle(text_style);
    builder.addText(text);
    auto paragraph = builder.Build();
    paragraph->layout(SK_ScalarInfinity);

    paragraph->layout(paragraph->getMinIntrinsicWidth() + 1);
    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, impl->runs().size() > 1);  // It's not the simple case
    bool hasNonMonotonicPlacement = false;
    for (auto& run : impl->runs()) {
        for (auto& offset : run.offsets()) {
            if (offset.fX < 0) {
                hasNonMonotonicPlacement = true;
            }
        }
        if (hasNonMonotonicPlacement) {
            break;
        }
    }
    REPORTER_ASSERT(reporter, hasNonMonotonicPlacement);  // There are non-monotonic placement
    REPORTER_ASSERT(reporter, impl->lineNumber() == 1);   // But it's still one line
    paragraph->paint(canvas.get(), 0, 0);
}

UNIX_ONLY_TEST(SkParagraph_NonMonotonicGlyphsRTL, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    NEED_SYSTEM_FONTS(fontCollection)

    TestCanvas canvas("SkParagraph_NonMonotonicGlyphsRTL.png");
    const char* text = "ٱلْرَّحْمَـانُ";
    const size_t len = strlen(text);

    TextStyle text_style;
    text_style.setFontSize(14);
    text_style.setColor(SK_ColorBLACK);

    ParagraphStyle paragraph_style;
    paragraph_style.setTextStyle(text_style);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());

    builder.pushStyle(text_style);
    builder.addText(text, len);
    auto paragraph = builder.Build();
    paragraph->layout(SK_ScalarInfinity);

    paragraph->layout(paragraph->getMinIntrinsicWidth() + 1);
    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    bool hasNonMonotonicPlacement = false;
    for (auto& run : impl->runs()) {
        for (auto& offset : run.offsets()) {
            if (offset.fX < 0) {
                hasNonMonotonicPlacement = true;
            }
        }
        if (hasNonMonotonicPlacement) {
            break;
        }
    }
    REPORTER_ASSERT(reporter, impl->lineNumber() == 1);  // But it's still one line
    paragraph->paint(canvas.get(), 0, 0);
}

// Arabic Ligature case should be painted into multi styles but queried as a single glyph
UNIX_ONLY_TEST(SkParagraph_MultiStyle_Arabic, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>(true);
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    TestCanvas canvas("SkParagraph_MultiStyle_Arabic.png");

    canvas.get()->drawColor(SK_ColorWHITE);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Noto Naskh Arabic")});
    text_style.setFontSize(50);
    text_style.setColor(SK_ColorBLACK);
    ParagraphStyle paragraph_style;
    paragraph_style.setTextStyle(text_style);
    paragraph_style.setTextDirection(TextDirection::kRtl);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());
    text_style.setColor(SK_ColorBLUE);
    builder.pushStyle(text_style);
    builder.addText("ك");
    text_style.setColor(SK_ColorRED);
    builder.pushStyle(text_style);
    builder.addText("ِّ");
    text_style.setColor(SK_ColorBLUE);
    builder.pushStyle(text_style);
    builder.addText("ـ");
    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);
    paragraph->paint(canvas.get(), 0, 0);

    auto width = paragraph->getLongestLine();
    auto height = paragraph->getHeight();

    // Positioning is align with the grapheme
    auto f1Pos =
            paragraph->getGlyphPositionAtCoordinate(TestCanvasWidth - width * 5 / 6, height / 2);
    auto f2Pos = paragraph->getGlyphPositionAtCoordinate(TestCanvasWidth - width / 3, height / 2);
    auto iPos = paragraph->getGlyphPositionAtCoordinate(TestCanvasWidth - width / 6, height / 2);
    REPORTER_ASSERT(reporter, f1Pos.position == 4 && f1Pos.affinity == Affinity::kUpstream);
    REPORTER_ASSERT(reporter, f2Pos.position == 1 && f2Pos.affinity == Affinity::kUpstream);
    REPORTER_ASSERT(reporter, iPos.position == 0 && iPos.affinity == Affinity::kDownstream);

    // Bounding boxes around a part of a grapheme are empty
    auto f1 = paragraph->getRectsForRange(0, 1, RectHeightStyle::kTight, RectWidthStyle::kTight);
    REPORTER_ASSERT(reporter, f1.size() == 0);

    auto f2 = paragraph->getRectsForRange(1, 2, RectHeightStyle::kTight, RectWidthStyle::kTight);
    REPORTER_ASSERT(reporter, f2.size() == 0);

    auto fi = paragraph->getRectsForRange(2, 3, RectHeightStyle::kTight, RectWidthStyle::kTight);
    REPORTER_ASSERT(reporter, fi.size() == 0);
}

// Fonts missing U+0020 (abc.ttf) combined with BiDi default-ignorable controls
// produce both 0-glyph runs and merged clusters when shaped by HarfBuzz
UNIX_ONLY_TEST(SkParagraph_ReproNoSpaceBidiCrash, reporter) {
    sk_sp<SkFontMgr> mgr = ToolUtils::TestFontMgr();
    auto noSpaceTypeface = mgr->makeFromStream(GetResourceAsStream("fonts/abc/abc.ttf"), {});
    auto robotoTypeface = mgr->makeFromStream(GetResourceAsStream("fonts/Roboto-Regular.ttf"), {});
    if (!noSpaceTypeface || !robotoTypeface) {
        return;
    }

    auto checkParagraph = [&](const char* text, bool enableFallback, bool expectAllResolved) {
        auto fontCollection = sk_make_sp<FontCollection>();
        auto fontProvider = sk_make_sp<TypefaceFontProvider>();
        fontProvider->registerTypeface(noSpaceTypeface, SkString("NoSpaceFont"));
        fontProvider->registerTypeface(robotoTypeface, SkString("Roboto"));
        fontCollection->setAssetFontManager(fontProvider);
        fontCollection->setDefaultFontManager(mgr);
        if (enableFallback) {
            fontCollection->enableFontFallback();
        } else {
            fontCollection->disableFontFallback();
        }

        ParagraphStyle paragraphStyle;
        paragraphStyle.setTextDirection(TextDirection::kRtl);
        ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());

        TextStyle style;
        style.setFontFamilies({SkString("NoSpaceFont"), SkString("Roboto")});
        style.setFontSize(20.0f);
        builder.pushStyle(style);
        size_t len = strlen(text);
        builder.addText(text, len);
        builder.pop();

        auto paragraph = builder.Build();
        paragraph->layout(300.0f);

        auto* impl = static_cast<ParagraphImpl*>(paragraph.get());
        REPORTER_ASSERT(reporter, impl->lines().size() == 1);

        SkBitmap bm;
        bm.allocN32Pixels(300, 100);
        SkCanvas canvas(bm);
        paragraph->paint(&canvas, 0, 0);

        auto rects = paragraph->getRectsForRange(
                0, len, RectHeightStyle::kTight, RectWidthStyle::kTight);
        REPORTER_ASSERT(reporter, !rects.empty());

        paragraph->getGlyphPositionAtCoordinate(10.0f, 10.0f);
        for (size_t i = 0; i < len; ++i) {
            Paragraph::GlyphClusterInfo clusterInfo;
            REPORTER_ASSERT(reporter, paragraph->getGlyphClusterAt(i, &clusterInfo));
            if (impl->lines().front().width() > 0) {
                REPORTER_ASSERT(reporter,
                                paragraph->getClosestGlyphClusterAt(10.0f, 10.0f, &clusterInfo));
            }
        }

        int visitedRuns = 0;
        paragraph->visit([&](int, const Paragraph::VisitorInfo* info) {
            if (info) {
                visitedRuns++;
                if (expectAllResolved && info->font.getTypeface() == robotoTypeface.get()) {
                    for (int i = 0; i < info->count; ++i) {
                        REPORTER_ASSERT(reporter, info->glyphs[i] != 0);
                    }
                }
            }
        });
        int extendedVisitedRuns = 0;
        paragraph->extendedVisit([&](int, const Paragraph::ExtendedVisitorInfo* info) {
            if (info) {
                extendedVisitedRuns++;
            }
        });
        REPORTER_ASSERT(reporter, visitedRuns == extendedVisitedRuns);

        SkPath path;
        paragraph->getPath(0, &path);

        auto unresolved = impl->unresolvedCodepoints();
        REPORTER_ASSERT(reporter, unresolved.find(0x200E) == unresolved.end());
        REPORTER_ASSERT(reporter, unresolved.find(0x2068) == unresolved.end());
        REPORTER_ASSERT(reporter, unresolved.find(0x2069) == unresolved.end());
        REPORTER_ASSERT(reporter, unresolved.find(0x0020) == unresolved.end());
        if (expectAllResolved) {
            REPORTER_ASSERT(reporter, impl->unresolvedGlyphs() == 0);
        }
        return paragraph;
    };

    const char* flutterReproText = "أنت \u200e\u2068@wren\u2069 هنا";
    auto p1 = checkParagraph(flutterReproText, true, false);
    REPORTER_ASSERT(reporter, p1->getFontAt(21).getTypeface() == robotoTypeface.get());
    auto p2 = checkParagraph(flutterReproText, false, false);
    REPORTER_ASSERT(reporter, p2->getFontAt(21).getTypeface() == robotoTypeface.get());

    checkParagraph("\u200e\u2068abc\u2069 a", true, true);
    checkParagraph("\u200e", true, true);
    checkParagraph("\u200e\u2068", false, true);
}

// Ensure BiDi control characters and dropped glyphs do not leave gaps in code unit cluster mapping
DEF_TEST(SkParagraph_BidiControlGaps, reporter) {
    auto fontCollection = sk_make_sp<FontCollection>();
    fontCollection->setDefaultFontManager(ToolUtils::TestFontMgr());

    auto checkGaps = [&](const char* text) {
        ParagraphStyle paragraphStyle;
        ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());
        TextStyle style;
        style.setFontSize(14.0f);
        builder.pushStyle(style);
        builder.addText(text);
        builder.pop();

        auto paragraph = builder.Build();
        paragraph->layout(1000.0f);

        auto impl = static_cast<ParagraphImpl*>(paragraph.get());
        for (size_t i = 0; i <= impl->text().size(); ++i) {
            REPORTER_ASSERT(reporter, impl->clusterIndex(i) != EMPTY_INDEX);
        }

        SkBitmap bitmap;
        bitmap.allocN32Pixels(1000, 100);
        SkCanvas canvas(bitmap);
        paragraph->paint(&canvas, 0, 0);
    };

    checkGaps("hello world\u202B\u202E BOY HOW DO\u202C you \u202E DO SIR\u202C\u202C good bye");
    checkGaps("\u200e");
    checkGaps("A\u200e");
    checkGaps("A\u202c");
    checkGaps("\u202a\u202c");
    checkGaps("\u200Ehello");
    checkGaps("\u202Dhello\u202C");
    checkGaps("\u2066hello\u2069");
}

// Verify that a BiDi isolate starting a styled block with a space-less primary font
// preserves a 0-width cluster for the isolate and does not trap during style measurement
DEF_TEST(SkParagraph_BidiIsolateStyledBlockNoSpaceFont, reporter) {
    sk_sp<SkFontMgr> mgr = ToolUtils::TestFontMgr();
    auto noSpaceTypeface = mgr->makeFromStream(GetResourceAsStream("fonts/abc/abc.ttf"), {});
    auto robotoTypeface = mgr->makeFromStream(GetResourceAsStream("fonts/Roboto-Regular.ttf"), {});
    if (!noSpaceTypeface || !robotoTypeface) {
        return;
    }

    auto fontCollection = sk_make_sp<FontCollection>();
    auto fontProvider = sk_make_sp<TypefaceFontProvider>();
    fontProvider->registerTypeface(noSpaceTypeface, SkString("NoSpaceFont"));
    fontProvider->registerTypeface(robotoTypeface, SkString("Roboto"));
    fontCollection->setAssetFontManager(fontProvider);
    fontCollection->setDefaultFontManager(mgr);
    fontCollection->enableFontFallback();

    ParagraphStyle paragraphStyle;
    paragraphStyle.setTextDirection(TextDirection::kRtl);
    ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());

    TextStyle baseStyle;
    baseStyle.setFontFamilies({SkString("NoSpaceFont"), SkString("Roboto")});
    baseStyle.setFontSize(20.0f);
    SkPaint fg;
    fg.setColor(SK_ColorBLACK);
    baseStyle.setForegroundPaint(fg);

    TextStyle boldStyle = baseStyle;
    boldStyle.setFontStyle(SkFontStyle::Bold());

    builder.pushStyle(baseStyle);
    builder.addText("أنت ");
    builder.pushStyle(boldStyle);
    builder.addText("\u206680\u00A0\u2013\u00A083\u2069 abc");
    builder.pop();
    builder.addText(" هنا");
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(300.0f);

    auto* impl = static_cast<ParagraphImpl*>(paragraph.get());
    for (size_t i = 0; i <= impl->text().size(); ++i) {
        REPORTER_ASSERT(reporter, impl->clusterIndex(i) != EMPTY_INDEX);
    }

    // U+2066 at the start of the bold block ([7:10)) gets RTL bidiLevel 1 while "80..."
    // gets LTR bidiLevel 2, producing a 0-glyph run in NoSpaceFont that must map to [7:10)
    const auto& isolateCluster = impl->cluster(impl->clusterIndex(7));
    REPORTER_ASSERT(reporter, isolateCluster.textRange() == TextRange(7, 10));
    REPORTER_ASSERT(reporter, isolateCluster.size() == 0);
    REPORTER_ASSERT(reporter, isolateCluster.width() == 0.0f);

    // Space after U+2069 (offset 24) must fall back to Roboto while "abc" (offset 25) uses abc.ttf
    REPORTER_ASSERT(reporter, paragraph->getFontAt(24).getTypeface() == robotoTypeface.get());
    REPORTER_ASSERT(reporter, paragraph->getFontAt(25).getTypeface() == noSpaceTypeface.get());

    SkBitmap bm;
    bm.allocN32Pixels(300, 100);
    SkCanvas canvas(bm);
    paragraph->paint(&canvas, 0, 0);

    auto rects = paragraph->getRectsForRange(
            0, impl->text().size(), RectHeightStyle::kTight, RectWidthStyle::kTight);
    REPORTER_ASSERT(reporter, !rects.empty());
}
