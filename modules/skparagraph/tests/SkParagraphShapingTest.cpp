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
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "modules/skparagraph/include/DartTypes.h"
#include "modules/skparagraph/include/FontCollection.h"
#include "modules/skparagraph/include/Paragraph.h"
#include "modules/skparagraph/include/ParagraphCache.h"
#include "modules/skparagraph/include/ParagraphStyle.h"
#include "modules/skparagraph/include/TextStyle.h"
#include "modules/skparagraph/include/TypefaceFontProvider.h"
#include "modules/skparagraph/src/ParagraphBuilderImpl.h"
#include "modules/skparagraph/src/ParagraphImpl.h"
#include "modules/skparagraph/src/Run.h"
#include "modules/skparagraph/src/TextLine.h"
#include "modules/skparagraph/tests/SkParagraphTestUtils.h"
#include "modules/skparagraph/utils/TestFontCollection.h"
#include "modules/skunicode/include/SkUnicode.h"
#include "tests/Test.h"
#include "tools/Resources.h"
#include "tools/fonts/FontToolUtils.h"

#include <string.h>
#include <limits>
#include <string>
#include <utility>
#include <vector>

using namespace skia::textlayout;
using namespace skia::textlayout::test;

UNIX_ONLY_TEST(SkParagraph_FontFeaturesParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    TestCanvas canvas("SkParagraph_FontFeaturesParagraph.png");

    const char* text = "12ab\n";

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());

    TextStyle text_style;
    text_style.setFontStyle(SkFontStyle::Italic());  // Regular Roboto doesn't have font features
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setColor(SK_ColorBLACK);

    text_style.addFontFeature(SkString("tnum"), 1);
    builder.pushStyle(text_style);
    builder.addText(text);

    text_style.resetFontFeatures();
    text_style.addFontFeature(SkString("tnum"), 0);
    text_style.addFontFeature(SkString("pnum"), 1);
    builder.pushStyle(text_style);
    builder.addText(text);

    builder.pop();
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);

    paragraph->paint(canvas.get(), 10.0, 15.0);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, paragraph->lineNumber() == 3ull);
    if (paragraph->lineNumber() >= 3) {
        auto& tnum_line = impl->lines()[0];
        auto& pnum_line = impl->lines()[1];

        REPORTER_ASSERT(reporter, tnum_line.clusters().width() == 4ull);
        REPORTER_ASSERT(reporter, pnum_line.clusters().width() == 4ull);
        // Tabular numbers should have equal widths.
        REPORTER_ASSERT(reporter, impl->clusters()[0].width() == impl->clusters()[1].width());
        // Proportional numbers should have variable widths.
        REPORTER_ASSERT(reporter, impl->clusters()[5].width() != impl->clusters()[6].width());
        // Alphabetic characters should be unaffected.
        REPORTER_ASSERT(reporter, impl->clusters()[2].width() == impl->clusters()[7].width());
    }
}

// Not in Minikin
UNIX_ONLY_TEST(SkParagraph_WhitespacesInMultipleFonts, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    const char* text = "English English 字典 字典 😀😃😄 😀😃😄";
    const size_t len = strlen(text);

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());

    TextStyle text_style;
    text_style.setFontFamilies(
            {SkString("Roboto"), SkString("Noto Color Emoji"), SkString("Source Han Serif CN")});
    text_style.setFontSize(60);
    builder.pushStyle(text_style);
    builder.addText(text, len);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);

    REPORTER_ASSERT(reporter, paragraph->unresolvedGlyphs() == 0);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    for (size_t i = 0; i < impl->runs().size() - 1; ++i) {
        auto first = impl->runs()[i].textRange();
        auto next = impl->runs()[i + 1].textRange();
        REPORTER_ASSERT(reporter, first.end == next.start);
    }
}

// Adjacent styles that differ in any font setting must be shaped separately
DEF_TEST(SkParagraph_ShapingKeepsFontSettingsPerStyle, reporter) {
    auto fontCollection = sk_make_sp<FontCollection>();
    fontCollection->setDefaultFontManager(ToolUtils::TestFontMgr());
    fontCollection->getParagraphCache()->turnOn(false);

    auto widthOf = [&](const TextStyle& first, const TextStyle& second) {
        ParagraphStyle paragraphStyle;
        ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());
        builder.pushStyle(first);
        builder.addText("Hello ");
        builder.pop();
        builder.pushStyle(second);
        builder.addText("world");
        builder.pop();
        auto paragraph = builder.Build();
        paragraph->layout(1000.0f);
        return paragraph->getMaxIntrinsicWidth();
    };

    TextStyle subpixel;
    subpixel.setFontSize(30.0f);
    TextStyle whole = subpixel;
    whole.setSubpixel(false);
    REPORTER_ASSERT(reporter, widthOf(subpixel, subpixel) != widthOf(whole, whole));
    REPORTER_ASSERT(reporter, widthOf(whole, subpixel) != widthOf(whole, whole));

    TextStyle overridden = subpixel;
    overridden.setHeightOverride(true);
    overridden.setHeight(3.0f);
    TextStyle ignored = overridden;
    ignored.setHeightOverride(false);

    ParagraphStyle paragraphStyle;
    ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());
    builder.pushStyle(ignored);
    builder.addText("Hello ");
    builder.pop();
    builder.pushStyle(overridden);
    builder.addText("world");
    builder.pop();
    auto paragraph = builder.Build();
    paragraph->layout(1000.0f);
    auto* impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, impl->runs().size() == 2);
    if (impl->runs().size() == 2) {
        REPORTER_ASSERT(reporter, impl->runs()[0].heightMultiplier() == 0);
        REPORTER_ASSERT(reporter, impl->runs()[1].heightMultiplier() == 3.0f);
    }
}

UNIX_ONLY_TEST(SkParagraph_FontStyle, reporter) {
    TestCanvas canvas("SkParagraph_FontStyle.png");

    sk_sp<TestFontCollection> fontCollection =
            sk_make_sp<TestFontCollection>(GetResourcePath("fonts").c_str(), false, true);
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setColor(SK_ColorBLACK);
    text_style.setFontSize(20);
    SkFontStyle fs = SkFontStyle(SkFontStyle::Weight::kLight_Weight,
                                 SkFontStyle::Width::kNormal_Width,
                                 SkFontStyle::Slant::kUpright_Slant);
    text_style.setFontStyle(fs);
    ParagraphStyle paragraph_style;
    paragraph_style.setTextStyle(text_style);
    TextStyle boldItalic;
    boldItalic.setFontFamilies({SkString("Roboto")});
    boldItalic.setColor(SK_ColorRED);
    SkFontStyle bi = SkFontStyle(SkFontStyle::Weight::kBold_Weight,
                                 SkFontStyle::Width::kNormal_Width,
                                 SkFontStyle::Slant::kItalic_Slant);
    boldItalic.setFontStyle(bi);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());
    builder.addText("Default text\n");
    builder.pushStyle(boldItalic);
    builder.addText("Bold and Italic\n");
    builder.pop();
    builder.addText("back to normal");
    auto paragraph = builder.Build();
    paragraph->layout(250);
    paragraph->paint(canvas.get(), 0, 0);
}

UNIX_ONLY_TEST(SkParagraph_Shaping, reporter) {
    TestCanvas canvas("SkParagraph_Shaping.png");

    sk_sp<TestFontCollection> fontCollection =
            sk_make_sp<TestFontCollection>(GetResourcePath("fonts").c_str(), true);
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setColor(SK_ColorGRAY);
    text_style.setFontSize(14);
    SkFontStyle b = SkFontStyle(SkFontStyle::Weight::kNormal_Weight,
                                SkFontStyle::Width::kNormal_Width,
                                SkFontStyle::Slant::kUpright_Slant);
    text_style.setFontStyle(b);
    ParagraphStyle paragraph_style;
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());
    builder.pushStyle(text_style);
    builder.addText("Eat0 apple0 pies0 | Eat1 apple1 pies1 | Eat2 apple2 pies2");
    auto paragraph = builder.Build();
    paragraph->layout(380);
    paragraph->paint(canvas.get(), 0, 0);
}

// Google logo is shown in one style (the first one)
UNIX_ONLY_TEST(SkParagraph_MultiStyle_Logo, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>(true);
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    TestCanvas canvas("SkParagraph_MultiStyle_Logo.png");

    canvas.get()->drawColor(SK_ColorWHITE);
    SkScalar width = TestCanvasWidth;
    SkScalar height = TestCanvasHeight / 2;

    SkAutoCanvasRestore acr(canvas.get(), true);
    canvas.get()->clipRect(SkRect::MakeWH(width, height));

    TextStyle style;
    style.setFontFamilies({SkString("Google Sans")});
    style.setFontSize(30);

    TextStyle style0(style);
    style0.setDecoration(TextDecoration::kUnderline);
    style0.setDecorationColor(SK_ColorBLACK);

    TextStyle style1(style);
    style1.setDecoration(TextDecoration::kOverline);
    style1.setDecorationColor(SK_ColorBLACK);

    ParagraphStyle paraStyle;
    paraStyle.setTextStyle(style);
    paraStyle.setMaxLines(std::numeric_limits<size_t>::max());

    const char* logo1 = "google_";
    const char* logo2 = "logo";
    const char* logo3 = "go";
    const char* logo4 = "ogle_logo";
    const char* logo5 = "google_lo";
    const char* logo6 = "go";

    ParagraphBuilderImpl builder(paraStyle, fontCollection, get_unicode());
    style0.setDecorationStyle(TextDecorationStyle::kDouble);
    style0.setForegroundColor(SkPaint(SkColors::kBlack));
    style0.setBackgroundColor(SkPaint(SkColors::kLtGray));
    builder.pushStyle(style0);
    builder.addText(logo1, strlen(logo1));
    style1.setDecorationStyle(TextDecorationStyle::kWavy);
    style1.setForegroundColor(SkPaint(SkColors::kBlue));
    style1.setBackgroundColor(SkPaint(SkColors::kYellow));
    builder.pushStyle(style1);
    builder.addText(logo2, strlen(logo2));
    builder.addText(" ", 1);

    style0.setDecorationStyle(TextDecorationStyle::kSolid);
    style0.setForegroundColor(SkPaint(SkColors::kBlue));
    style0.setBackgroundColor(SkPaint(SkColors::kWhite));
    builder.pushStyle(style0);
    builder.addText(logo3, strlen(logo3));
    style1.setDecorationStyle(TextDecorationStyle::kDotted);
    style1.setForegroundColor(SkPaint(SkColors::kBlack));
    style1.setBackgroundColor(SkPaint(SkColors::kMagenta));
    builder.pushStyle(style1);
    builder.addText(logo4, strlen(logo4));
    builder.addText(" ", 1);

    style0.setDecorationStyle(TextDecorationStyle::kDashed);
    style0.setForegroundColor(SkPaint(SkColors::kGreen));
    style0.setBackgroundColor(SkPaint(SkColors::kGray));
    builder.pushStyle(style0);
    builder.addText(logo5, strlen(logo5));
    style1.setDecorationStyle(TextDecorationStyle::kDouble);
    style1.setForegroundColor(SkPaint(SkColors::kBlue));
    style1.setBackgroundColor(SkPaint(SkColors::kCyan));
    builder.pushStyle(style1);
    builder.addText(logo6, strlen(logo6));

    auto paragraph = builder.Build();
    paragraph->layout(width - 40);
    paragraph->paint(canvas.get(), 20, 20);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, impl->lines().size() == 1);

    size_t index = 0;
    SkScalar left = 0.0f;
    impl->lines().data()->scanStyles(
            StyleType::kDecorations,
            [&index, &left, reporter](TextRange textRange,
                                      const TextStyle& style,
                                      const TextLine::ClipContext& context) {
                switch (index) {
                    case 0:
                        REPORTER_ASSERT(reporter, context.pos == 0 && context.size == 1);
                        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(context.clip.fLeft, left));
                        REPORTER_ASSERT(reporter,
                                        SkScalarNearlyEqual(context.clip.fRight, 63.558979f));
                        break;
                    case 1:
                        REPORTER_ASSERT(reporter, context.pos == 0 && context.size == 2);
                        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(context.clip.fLeft, left));
                        REPORTER_ASSERT(reporter,
                                        SkScalarNearlyEqual(context.clip.fRight, 105.479904f));
                        break;
                    case 2:
                        REPORTER_ASSERT(reporter, context.pos == 2 && context.size == 1);
                        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(context.clip.fLeft, left));
                        REPORTER_ASSERT(reporter,
                                        SkScalarNearlyEqual(context.clip.fRight, 126.334106f));
                        break;
                    case 3:
                        REPORTER_ASSERT(reporter, context.pos == 2 && context.size == 2);
                        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(context.clip.fLeft, left));
                        REPORTER_ASSERT(reporter,
                                        SkScalarNearlyEqual(context.clip.fRight, 210.959808f));
                        break;
                    case 4:
                        REPORTER_ASSERT(reporter, context.pos == 4 && context.size == 1);
                        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(context.clip.fLeft, left));
                        REPORTER_ASSERT(reporter,
                                        SkScalarNearlyEqual(context.clip.fRight, 288.625549f));
                        break;
                    case 5:
                        // No space at the end
                        REPORTER_ASSERT(reporter, context.pos == 4 && context.size == 1);
                        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(context.clip.fLeft, left));
                        REPORTER_ASSERT(reporter,
                                        SkScalarNearlyEqual(context.clip.fRight, 309.479736f));
                        break;
                    default:
                        REPORTER_ASSERT(reporter, false);
                        break;
                }
                left = context.clip.fRight;
                ++index;
            });
}

// Ligature FFI should allow painting and querying by codepoints
UNIX_ONLY_TEST(SkParagraph_MultiStyle_FFI, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>(true);
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    TestCanvas canvas("SkParagraph_MultiStyle_FFI.png");

    canvas.get()->drawColor(SK_ColorWHITE);

    ParagraphStyle paragraph_style;
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());
    TextStyle text_style;
    text_style.setColor(SK_ColorBLACK);
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setFontSize(60);
    text_style.setBackgroundColor(SkPaint(SkColors::kGray));
    builder.pushStyle(text_style);
    builder.addText("f");
    text_style.setBackgroundColor(SkPaint(SkColors::kYellow));
    builder.pushStyle(text_style);
    builder.addText("f");
    text_style.setBackgroundColor(SkPaint(SkColors::kLtGray));
    builder.pushStyle(text_style);
    builder.addText("i");
    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);
    paragraph->paint(canvas.get(), 0, 0);
    auto width = paragraph->getLongestLine();
    auto height = paragraph->getHeight();

    auto f1Pos = paragraph->getGlyphPositionAtCoordinate(width / 6 - 5, height / 2);
    auto f2Pos = paragraph->getGlyphPositionAtCoordinate(width / 2 - 5, height / 2);
    auto iPos = paragraph->getGlyphPositionAtCoordinate(width * 5 / 6 - 5, height / 2);

    // Positions are aligned with graphemes (no pointing inside ffi grapheme)
    REPORTER_ASSERT(reporter, f1Pos.position == 0 && f1Pos.affinity == Affinity::kDownstream);
    REPORTER_ASSERT(reporter, f2Pos.position == 1 && f2Pos.affinity == Affinity::kDownstream);
    REPORTER_ASSERT(reporter, iPos.position == 2 && iPos.affinity == Affinity::kDownstream);

    // Bounding boxes show the extact position (inside ffi grapheme)
    auto f1 = paragraph->getRectsForRange(0, 1, RectHeightStyle::kTight, RectWidthStyle::kTight);
    REPORTER_ASSERT(reporter, f1.size() == 1);
    REPORTER_ASSERT(reporter, f1[0].direction == TextDirection::kLtr);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(f1[0].rect.fLeft, 0.000000f, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(f1[0].rect.fRight, 18.969999f, EPSILON100));

    auto f2 = paragraph->getRectsForRange(1, 2, RectHeightStyle::kTight, RectWidthStyle::kTight);
    REPORTER_ASSERT(reporter, f2.size() == 1);
    REPORTER_ASSERT(reporter, f2[0].direction == TextDirection::kLtr);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(f2[0].rect.fLeft, 18.969999f, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(f2[0].rect.fRight, 37.950001f, EPSILON100));

    auto fi = paragraph->getRectsForRange(2, 3, RectHeightStyle::kTight, RectWidthStyle::kTight);
    REPORTER_ASSERT(reporter, fi.size() == 1);
    REPORTER_ASSERT(reporter, fi[0].direction == TextDirection::kLtr);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(fi[0].rect.fLeft, 37.950001f, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(fi[0].rect.fRight, 51.209999f, EPSILON100));
}

// Multiple code points/single glyph emoji family should be treated as a single glyph
UNIX_ONLY_TEST(SkParagraph_MultiStyle_EmojiFamily, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>(true);
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    TestCanvas canvas("SkParagraph_MultiStyle_EmojiFamily.png");

    canvas.get()->drawColor(SK_ColorWHITE);

    ParagraphStyle paragraph_style;
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());
    TextStyle text_style;
    text_style.setColor(SK_ColorBLACK);
    text_style.setFontFamilies({SkString("Noto Color Emoji")});
    text_style.setFontSize(40);
    builder.pushStyle(text_style);
    builder.addText(u"\U0001F468\u200D\U0001F469\u200D\U0001F467\u200D\U0001F466");
    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setAntiAlias(true);
    paint.setStrokeWidth(1);
    paint.setColor(SK_ColorLTGRAY);
    canvas.get()->drawRect(
            SkRect::MakeXYWH(0, 0, paragraph->getLongestLine(), paragraph->getHeight()), paint);
    paragraph->paint(canvas.get(), 0, 0);
    auto width = paragraph->getLongestLine();
    auto height = paragraph->getHeight();

    auto pos00 = paragraph->getGlyphPositionAtCoordinate(width / 4, height / 4);
    auto pos10 = paragraph->getGlyphPositionAtCoordinate(width * 3 / 4, height / 2);
    auto pos01 = paragraph->getGlyphPositionAtCoordinate(width / 4, height / 2);
    auto pos11 = paragraph->getGlyphPositionAtCoordinate(width * 3 / 4, height * 3 / 4);

    // Positioning is aligned with the closest grapheme edge (left or right)
    REPORTER_ASSERT(reporter, pos00.position == 0 && pos00.affinity == Affinity::kDownstream);
    REPORTER_ASSERT(reporter, pos01.position == 0 && pos01.affinity == Affinity::kDownstream);
    REPORTER_ASSERT(reporter, pos10.position == 11 && pos10.affinity == Affinity::kUpstream);
    REPORTER_ASSERT(reporter, pos11.position == 11 && pos11.affinity == Affinity::kUpstream);

    // Bounding boxes around a part of a grapheme are empty
    auto f1 = paragraph->getRectsForRange(0, 2, RectHeightStyle::kTight, RectWidthStyle::kTight);
    REPORTER_ASSERT(reporter, f1.size() == 0);

    auto f2 = paragraph->getRectsForRange(4, 6, RectHeightStyle::kTight, RectWidthStyle::kTight);
    REPORTER_ASSERT(reporter, f2.size() == 0);

    auto f3 = paragraph->getRectsForRange(8, 10, RectHeightStyle::kTight, RectWidthStyle::kTight);
    REPORTER_ASSERT(reporter, f3.size() == 0);

    auto f4 = paragraph->getRectsForRange(8, 10, RectHeightStyle::kTight, RectWidthStyle::kTight);
    REPORTER_ASSERT(reporter, f4.size() == 0);
}

// Zalgo text should be painted into multi styles but queried as a single glyph
UNIX_ONLY_TEST(SkParagraph_MultiStyle_Zalgo, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    NEED_SYSTEM_FONTS(fontCollection)

    TestCanvas canvas("SkParagraph_MultiStyle_Zalgo.png");

    canvas.get()->drawColor(SK_ColorWHITE);

    std::u16string text16 =
            u">S\u035B\u036D\u030B\u0346\u0308\u0314\u0307\u0357\u030D\u0351\u030E\u036A\u036E"
            u"\u0367\u0363\u033D\u036B\u0363\u0301\u036C\u0300\u034C\u0351\u0342\u0357\u0352\u030D"
            u"\u0314\u0304\u0367\u030F\u031A\u0309\u030C\u030A\u030A\u033F\u0300\u030C\u0303\u0304"
            u"\u0350\u0313\u031A\u0313\u0363\u0321\u035C\u035D\u035F\u0360\u0335\u034F\u0321\u0327"
            u"\u0338\u035C\u0335\u0363\u0337\u0321\u0347\u031C\u0318\u033B\u033A\u0318\u031F\u031D"
            u"\u0359\u032C\u0318\u0329\u0347\u032D\u033C\u0325\u0316\u0324\u0326\u034Ek\u0302"
            u"\u0368\u0309\u036A\u0312\u034B\u035B\u0300\u030D\u034A\u0367\u033F\u0305\u0346\u0313"
            u"\u0314\u0314\u036C\u0307\u0311\u033F\u0369\u0357\u036E\u030E\u034C\u033F\u0304\u0349"
            u"\u0329\u0318\u035A\u031C\u0339\u0317\u0317\u034D\u0324\u0325\u0331\u0349\u0333\u0355"
            u"\u0345\u0356\u0324\u0332\u0323\u035A\u032E\u031E\u032C\u0332\u034D\u0354\u032F\u033B"
            u"\u032E\u031E\u032D\u0348\u0317\u032B\u0353P\u031A\u0351\u0350\u030E\u0303\u0312"
            u"\u0350\u034B\u0334\u031B\u035E\u0358\u0321\u0335\u0321\u032A\u0359\u033C\u0323\u031F"
            u"\u0329\u032D\u032B\u0331\u0359\u032C\u0354\u0349\u034D\u0318\u0320\u0349\u0326\u031D"
            u"\u0318\u0325\u031F\u0317\u0356\u032B\u0324\u0355\u0319\u032C\u0326\u034D\u0331\u0316"
            u"\u032E\u0331a\u0313\u033F\u0346\u030E\u034B\u0313\u0366\u0344\u0366\u030C\u0301"
            u"\u0304\u0357\u030C\u0313\u034C\u035C\u0336\u035C\u0321\u0336\u035D\u0315\u0335\u0335"
            u"\u0335\u035F\u0336\u0336\u0327\u0322\u0361\u0362\u0322\u033A\u0354\u0323\u0356\u032D"
            u"\u033A\u034D\u0324\u035A\u0331\u031C\u0330\u0325\u0355\u032C\u0325\u0332\u031E\u0325"
            u"\u0318\u0347\u035A\u033A\u0330\u035A\u032A\u033A\u0354\u0324\u034Dr\u0301\u0361"
            u"\u0315\u034E\u031D\u0355\u0349\u033B\u034E\u0324\u0345\u0345\u032D\u035A\u0317\u0333"
            u"\u0316\u0319\u0318\u035A\u0345\u032B\u0356\u0353\u035A\u0349\u0354\u0348\u031F\u0330"
            u"\u031F\u032C\u0317\u0353\u031F\u035A\u0331a\u033D\u036E\u0369\u0305\u0352\u031A"
            u"\u036B\u0357\u0342\u030E\u0366\u0344\u0343\u0338\u035C\u0361\u0322\u031B\u0358\u031B"
            u"\u0362\u0336\u0363\u0337\u035C\u0322\u035C\u0321\u0335\u0336\u0345\u031C\u032E\u0326"
            u"\u031C\u0325\u031C\u032F\u0319\u0353\u0354\u033C\u0317\u033Bg\u0304\u0368\u0368"
            u"\u030A\u0352\u0369\u036D\u0364\u030D\u0305\u0303\u036A\u030B\u030F\u0313\u030D\u031A"
            u"\u030B\u0357\u030B\u0368\u030F\u033D\u0344\u0314\u0300\u030B\u0309\u036B\u0305\u0302"
            u"\u036D\u036B\u030F\u0352\u034B\u0365\u0322\u031B\u035C\u0355\u0317\u0356\u0316\u0324"
            u"\u0326\u0318\u0354r\u0300\u0302\u0312\u036A\u034C\u033F\u034C\u0305\u035B\u0368"
            u"\u036A\u0352\u030D\u0365\u0309\u0364\u030C\u033F\u0306\u0301\u036D\u0346\u0303\u0312"
            u"\u0364\u035B\u030A\u0367\u033D\u035D\u0360\u0322\u0358\u0327\u0327\u0336\u0325\u0324"
            u"\u033Ca\u0351\u0351\u0313\u0351\u036E\u0303\u0342\u0304\u035B\u0301\u0344\u034B"
            u"\u0302\u034C\u033D\u0304\u0352\u0314\u0301\u0307\u0368\u0367\u036D\u0350\u0366\u031A"
            u"\u030B\u0368\u030D\u0366\u030D\u030B\u0346\u0314\u0367\u0351\u034B\u034C\u0308\u0343"
            u"\u035B\u035B\u0327\u0322\u0327\u0362\u035C\u035C\u0322\u034F\u0322\u031B\u0334\u0327"
            u"\u0333\u034D\u0339\u035A\u0330\u0339\u033B\u0354p\u0346\u0366\u031A\u0363\u034A"
            u"\u033D\u0314\u0342\u0309\u0363\u0314\u0363\u030C\u030C\u0309\u0303\u030B\u0302\u0352"
            u"\u036B\u0304\u030E\u0310\u0357\u0309\u030C\u0303\u033D\u033D\u0328\u0341\u0358\u0340"
            u"\u0321\u035C\u035F\u0363\u0331\u0349h\u035C\u035D\u032D\u032E\u0318\u0317\u0354"
            u"\u031C\u032F\u0354\u0348\u032F\u033A\u0354\u0317\u0323\u032D\u035A\u0331\u0330\u0319"
            u"\u033C\u0339\u035A\u0323\u033B\u0325\u0332\u032E\u034D\u0324<";
    SkString text = SkUnicode::convertUtf16ToUtf8(text16);
    auto K = text.find("k");
    auto P = text.find("P");
    auto h = text.find("h");
    ParagraphStyle paragraph_style;
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());
    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setFontSize(20);
    text_style.setColor(SK_ColorRED);
    builder.pushStyle(text_style);
    builder.addText(text.data(), K + 3);
    text_style.setColor(SK_ColorBLUE);
    text_style.setBackgroundColor(SkPaint(SkColors::kYellow));
    builder.pushStyle(text_style);
    builder.addText(text.data() + K + 3, P - K - 3 + 6);
    text_style.setColor(SK_ColorGREEN);
    builder.pushStyle(text_style);
    builder.addText(text.data() + P + 6, h - P - 6);
    text_style.setColor(SK_ColorBLACK);
    text_style.setBackgroundColor(SkPaint(SkColors::kLtGray));
    builder.pushStyle(text_style);
    builder.addText(text.data() + h, text.size() - h);
    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);
    paragraph->paint(canvas.get(), 0, paragraph->getHeight() / 2);
    //auto width = paragraph->getLongestLine();
    auto height = paragraph->getHeight();

    auto resSK = paragraph->getRectsForRange(0, K, RectHeightStyle::kTight, RectWidthStyle::kTight);
    REPORTER_ASSERT(reporter, resSK.size() != 0);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(resSK[0].rect.width(), 10.45f, EPSILON100));

    auto resKP = paragraph->getRectsForRange(K, P, RectHeightStyle::kTight, RectWidthStyle::kTight);
    REPORTER_ASSERT(reporter, resKP.size() != 0);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(resKP[0].rect.width(), 11.22f));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(resKP[0].rect.width(), 11.22f, EPSILON100));

    auto resPh = paragraph->getRectsForRange(P, h, RectHeightStyle::kTight, RectWidthStyle::kTight);
    REPORTER_ASSERT(reporter, resPh.size() != 0);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(resPh[0].rect.width(), 67.26f, EPSILON20));

    auto posK = paragraph->getGlyphPositionAtCoordinate(resSK.back().rect.fRight, height / 2);
    auto posP = paragraph->getGlyphPositionAtCoordinate(resKP.back().rect.fRight, height / 2);
    auto posH = paragraph->getGlyphPositionAtCoordinate(resPh.back().rect.fRight, height / 2);
    REPORTER_ASSERT(reporter, posK.position == 148 && posP.position == 264 && posH.position == 572);
}

DEF_TEST(SkParagraph_LigatureWithMultipleStyles, reporter) {
    sk_sp<TestFontCollection> fontCollection =
            sk_make_sp<TestFontCollection>(GetResourcePath("fonts").c_str(), false);
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    auto checkLigatureSplit = [&](const char* leftText,
                                  const char* rightText,
                                  size_t expectedSize0,
                                  size_t expectedSize1,
                                  SkScalar* outLeftWidth) {
        TextStyle style1;
        style1.setFontFamilies({SkString("Roboto")});
        style1.setFontSize(60);
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
        REPORTER_ASSERT(reporter, impl->lines().size() == 1);

        int index = 0;
        SkScalar totalWidth = 0;
        impl->lines()[0].scanStyles(
                StyleType::kForeground,
                [&](TextRange, const TextStyle&, const TextLine::ClipContext& context) {
                    REPORTER_ASSERT(reporter, context.pos == 0);
                    REPORTER_ASSERT(reporter, context.clippingNeeded);
                    if (index == 0) {
                        REPORTER_ASSERT(reporter, context.size == expectedSize0);
                        if (outLeftWidth) {
                            *outLeftWidth = context.clip.width();
                        }
                    } else if (index == 1) {
                        REPORTER_ASSERT(reporter, context.size == expectedSize1);
                    }
                    totalWidth += context.clip.width();
                    ++index;
                });
        REPORTER_ASSERT(reporter, index == 2);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(totalWidth, impl->lines()[0].width()));
        return impl->lines()[0].width();
    };

    SkScalar ffWidth = 0;
    SkScalar ffiTotal = checkLigatureSplit("ff", "i", 1, 1, &ffWidth);
    // Proportional split gives "ff" more than 2/3 of the ligature width because 'f' > 'i'
    REPORTER_ASSERT(reporter, ffWidth > ffiTotal * 2.0f / 3.0f + 1.0f);

    // Multi-cluster style range starting inside a ligature ("f" + "fiab")
    checkLigatureSplit("f", "fiab", 1, 3, nullptr);
}

// Each style block resolves its text with several fonts. The runs of each block in a
// many-block paragraph must be the same as the runs of that block shaped alone.
DEF_TEST(SkParagraph_ManyStyleBlocksWithSeveralFonts, reporter) {
    sk_sp<SkFontMgr> mgr = ToolUtils::TestFontMgr();
    auto abcTypeface = mgr->makeFromStream(GetResourceAsStream("fonts/abc/abc.ttf"), {});
    auto robotoTypeface = mgr->makeFromStream(GetResourceAsStream("fonts/Roboto-Regular.ttf"), {});
    if (!abcTypeface || !robotoTypeface) {
        return;
    }

    auto fontCollection = sk_make_sp<FontCollection>();
    auto fontProvider = sk_make_sp<TypefaceFontProvider>();
    fontProvider->registerTypeface(abcTypeface, SkString("Abc"));
    fontProvider->registerTypeface(robotoTypeface, SkString("Roboto"));
    fontCollection->setAssetFontManager(fontProvider);
    fontCollection->getParagraphCache()->turnOn(false);

    // "ab" and "c" resolve with Abc, "xy " with Roboto, U+E123 with no font
    const std::string blockText = "abxy c\xEE\x84\xA3";
    auto styleOf = [](size_t i) {
        TextStyle style;
        style.setFontFamilies({SkString("Abc"), SkString("Roboto")});
        // A different font size makes each block a separate shaping block
        style.setFontSize(i % 2 ? 21.0f : 20.0f);
        return style;
    };
    struct Piece {
        TextRange text;
        std::vector<SkGlyphID> glyphs;
        SkTypefaceID typeface;
    };
    auto piecesOf = [](ParagraphImpl* impl) {
        std::vector<Piece> pieces;
        for (auto& run : impl->runs()) {
            pieces.push_back({run.textRange(),
                              std::vector<SkGlyphID>(run.glyphs().begin(), run.glyphs().end()),
                              run.font().getTypeface()->uniqueID()});
        }
        return pieces;
    };

    const size_t kBlocks = 64;
    ParagraphStyle paragraphStyle;
    ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());
    for (size_t i = 0; i < kBlocks; ++i) {
        builder.pushStyle(styleOf(i));
        builder.addText(blockText.data(), blockText.size());
        builder.pop();
    }
    auto paragraph = builder.Build();
    paragraph->layout(10000.0f);
    auto* impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, paragraph->unresolvedGlyphs() == (int32_t)kBlocks);

    std::vector<Piece> expected[2];
    for (size_t i = 0; i < 2; ++i) {
        ParagraphBuilderImpl single(paragraphStyle, fontCollection, get_unicode());
        single.pushStyle(styleOf(i));
        single.addText(blockText.data(), blockText.size());
        single.pop();
        auto singleParagraph = single.Build();
        singleParagraph->layout(10000.0f);
        expected[i] = piecesOf(static_cast<ParagraphImpl*>(singleParagraph.get()));
        // Abc, Roboto, Abc and the unresolved piece
        REPORTER_ASSERT(reporter, expected[i].size() >= 3);
    }

    std::vector<Piece> actual = piecesOf(impl);
    REPORTER_ASSERT(reporter, actual.size() == kBlocks * expected[0].size());
    if (actual.size() != kBlocks * expected[0].size()) {
        return;
    }
    size_t index = 0;
    for (size_t i = 0; i < kBlocks; ++i) {
        size_t blockStart = i * blockText.size();
        for (const Piece& piece : expected[i % 2]) {
            const Piece& run = actual[index++];
            REPORTER_ASSERT(reporter, run.text.start == piece.text.start + blockStart);
            REPORTER_ASSERT(reporter, run.text.end == piece.text.end + blockStart);
            REPORTER_ASSERT(reporter, run.glyphs == piece.glyphs);
            REPORTER_ASSERT(reporter, run.typeface == piece.typeface);
        }
    }
}

// When an unresolved combining mark attaches to the non-initial codepoint of a multi-char
// ligature, clusteredText must expand both the glyph range and text range to the whole ligature
DEF_TEST(SkParagraph_LigatureWithUnresolvedMark, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>(true);
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    fontCollection->disableFontFallback();

    auto buildAndLayout = [&](const char* text,
                              std::vector<SkString> families,
                              TextDirection dir = TextDirection::kLtr) {
        ParagraphStyle paragraphStyle;
        paragraphStyle.setTextDirection(dir);
        TextStyle textStyle;
        textStyle.setFontFamilies(std::move(families));
        textStyle.setFontSize(20);
        textStyle.setColor(SK_ColorBLACK);
        paragraphStyle.setTextStyle(textStyle);

        ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());
        builder.pushStyle(textStyle);
        builder.addText(text, strlen(text));
        builder.pop();
        auto paragraph = builder.Build();
        paragraph->layout(1000);
        return paragraph;
    };

    // Run-start LTR ligature + missing U+064E must keep the unresolved text range at [0, 4)
    {
        auto p = buildAndLayout("fi\xD9\x8E", {SkString("Roboto")});
        auto* impl = static_cast<ParagraphImpl*>(p.get());
        REPORTER_ASSERT(reporter, impl->runs().size() == 1);
        if (impl->runs().size() == 1) {
            REPORTER_ASSERT(reporter, impl->run(0).textRange() == TextRange(0, 4));
        }
    }

    // LTR mid-run ligature + missing U+064E without fallback: cluster indexes stay inside run
    {
        auto p = buildAndLayout("afi\xD9\x8E", {SkString("Roboto")});
        auto* impl = static_cast<ParagraphImpl*>(p.get());
        REPORTER_ASSERT(reporter, !impl->runs().empty());
        for (const auto& run : impl->runs()) {
            REPORTER_ASSERT(reporter, run.size() > 0);
            for (size_t i = 0; i <= run.size(); ++i) {
                size_t ci = run.globalClusterIndex(i);
                REPORTER_ASSERT(reporter, ci >= run.textRange().start && ci <= run.textRange().end);
            }
        }
    }

    // LTR mid-run ligature + U+064E resolved by secondary family must not drop 'f'
    {
        auto p = buildAndLayout("afi\xD9\x8E", {SkString("Roboto"), SkString("Noto Naskh Arabic")});
        auto refFi = buildAndLayout("fi\xD9\x8E", {SkString("Noto Naskh Arabic")});
        auto* impl = static_cast<ParagraphImpl*>(p.get());
        REPORTER_ASSERT(reporter, p->unresolvedGlyphs() == 0);
        REPORTER_ASSERT(reporter, impl->runs().size() == 2);
        if (impl->runs().size() == 2) {
            REPORTER_ASSERT(reporter, impl->run(0).textRange() == TextRange(0, 1));
            REPORTER_ASSERT(reporter, impl->run(1).textRange() == TextRange(1, 5));
            REPORTER_ASSERT(
                    reporter,
                    SkScalarNearlyEqual(
                            impl->run(1).advance().fX, refFi->getMaxIntrinsicWidth(), EPSILON100));
        }
    }

    // RTL lam-alef ligature + U+0657 missing in Katibeh must not duplicate alef in fallback
    {
        auto p = buildAndLayout("\xD8\xA8\xD9\x84\xD8\xA7\xD9\x97",
                                {SkString("Katibeh"), SkString("Noto Naskh Arabic")},
                                TextDirection::kRtl);
        auto refLamAlef = buildAndLayout(
                "\xD9\x84\xD8\xA7\xD9\x97", {SkString("Noto Naskh Arabic")}, TextDirection::kRtl);
        auto* impl = static_cast<ParagraphImpl*>(p.get());
        REPORTER_ASSERT(reporter, p->unresolvedGlyphs() == 0);
        REPORTER_ASSERT(reporter, impl->runs().size() == 2);
        if (impl->runs().size() == 2) {
            REPORTER_ASSERT(reporter, impl->run(0).textRange() == TextRange(0, 2));
            REPORTER_ASSERT(reporter, impl->run(1).textRange() == TextRange(2, 8));
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(impl->run(1).advance().fX,
                                                refLamAlef->getMaxIntrinsicWidth(),
                                                EPSILON100));
        }
    }
}
