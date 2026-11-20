/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
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
#include "modules/skparagraph/src/TextLine.h"
#include "modules/skparagraph/tests/SkParagraphTestUtils.h"
#include "tests/Test.h"
#include "tools/Resources.h"
#include "tools/fonts/FontToolUtils.h"

#include <string.h>
#include <algorithm>
#include <array>
#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>

using namespace skia::textlayout;
using namespace skia::textlayout::test;

// Checked: DIFF (unclear)
UNIX_ONLY_TEST(SkParagraph_SpacingParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    TestCanvas canvas("SkParagraph_SpacingParagraph.png");
    ParagraphStyle paragraph_style;
    paragraph_style.setMaxLines(10);
    paragraph_style.setTextAlign(TextAlign::kLeft);
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setFontSize(50);
    text_style.setLetterSpacing(20);
    text_style.setWordSpacing(0);
    text_style.setColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText("H", 1);
    builder.pop();

    text_style.setLetterSpacing(10);
    text_style.setWordSpacing(0);
    builder.pushStyle(text_style);
    builder.addText("H", 1);
    builder.pop();

    text_style.setLetterSpacing(20);
    text_style.setWordSpacing(0);
    builder.pushStyle(text_style);
    builder.addText("H", 1);
    builder.pop();

    text_style.setLetterSpacing(0);
    text_style.setWordSpacing(0);
    builder.pushStyle(text_style);
    builder.addText("|", 1);
    builder.pop();

    const char* hSpace = "H ";
    const size_t len = strlen(hSpace);

    text_style.setLetterSpacing(0);
    text_style.setWordSpacing(20);
    builder.pushStyle(text_style);
    builder.addText(hSpace, len);
    builder.pop();

    text_style.setLetterSpacing(0);
    text_style.setWordSpacing(0);
    builder.pushStyle(text_style);
    builder.addText(hSpace, len);
    builder.pop();

    text_style.setLetterSpacing(0);
    text_style.setLetterSpacing(0);
    text_style.setWordSpacing(20);
    builder.pushStyle(text_style);
    builder.addText(hSpace, len);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(550);
    paragraph->paint(canvas.get(), 0, 0);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, impl->lines().size() == 1);
    size_t index = 0;
    impl->lines().begin()->scanStyles(
            StyleType::kLetterSpacing,
            [&](TextRange textRange, const TextStyle& style, const TextLine::ClipContext& context) {
                ++index;
                return true;
            });
    REPORTER_ASSERT(reporter, index == 4);
    index = 0;
    impl->lines().begin()->scanStyles(
            StyleType::kWordSpacing,
            [&](TextRange textRange, const TextStyle& style, const TextLine::ClipContext& context) {
                ++index;
                return true;
            });
    REPORTER_ASSERT(reporter, index == 4);
}

// Checked: disabled for TxtLib
UNIX_ONLY_TEST(SkParagraph_ArabicNoLetterSpacing, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    TestCanvas canvas("SkParagraph_NoSpacesArabicParagraph.png");
    const char* arabic = "سلام";
    const char* english = "Hello";

    ParagraphStyle paragraph_style;
    TextStyle text_style;
    text_style.setFontSize(10);
    text_style.setColor(SK_ColorBLACK);

    auto layout = [&](const char* familyName, const char* text, double letterSpacing) -> double {
        ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());
        text_style.setLetterSpacing(letterSpacing);
        text_style.setFontFamilies({SkString(familyName)});
        builder.pushStyle(text_style);
        builder.addText(text, strlen(text));
        builder.pop();

        auto paragraph = builder.Build();
        paragraph->layout(TestCanvasWidth);
        paragraph->paint(canvas.get(), 20, 0);
        canvas.get()->translate(0, paragraph->getHeight() + 20);
        return paragraph->getLongestLine();
    };

    {  // Letter spacing does not affect the layout for Arabic
        auto noLetterSpacing = layout("Katibeh", arabic, 0.0);
        auto withLetterSpacing = layout("Katibeh", arabic, 100.0);
        REPORTER_ASSERT(reporter,
                        SkScalarNearlyEqual(noLetterSpacing, withLetterSpacing, EPSILON1000));
    }
    {
        // Letter spacing affects English text (number of letter * letter spacing)
        auto noLetterSpacing = layout("Roboto", english, 0.0);
        auto withLetterSpacing = layout("Roboto", english, 100.0);
        REPORTER_ASSERT(reporter,
                        SkScalarNearlyEqual((withLetterSpacing - noLetterSpacing),
                                            100.0 * strlen(english),
                                            EPSILON1000));
    }
}

UNIX_ONLY_TEST(SkParagraph_ArabicWithLetterSpacingWhitespacesAsChrome, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    TestCanvas canvas("SkParagraph_SpacedArabicParagraph.png");
    const char* arabic = "سلام";
    const char* spaced = "س لا م";
    ParagraphStyle paragraph_style;
    TextStyle text_style;
    text_style.setFontSize(20);
    text_style.setColor(SK_ColorBLACK);

    auto layout = [&](const char* familyName, const char* text, double letterSpacing) -> double {
        ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());
        text_style.setLetterSpacing(letterSpacing);
        text_style.setFontFamilies({SkString(familyName)});
        builder.pushStyle(text_style);
        builder.addText(text, strlen(text));
        builder.pop();

        auto paragraph = builder.Build();
        paragraph->layout(TestCanvasWidth);
        paragraph->paint(canvas.get(), 20, 0);
        canvas.get()->translate(0, paragraph->getHeight() + 20);
        return paragraph->getLongestLine();
    };
    {  // Letter spacing does not show for Arabic without whitespaces
        auto noLetterSpacing = layout("Katibeh", arabic, 0.0);
        auto withLetterSpacing = layout("Katibeh", arabic, 10.0);
        REPORTER_ASSERT(reporter,
                        SkScalarNearlyEqual(withLetterSpacing, noLetterSpacing, EPSILON100));
    }
    {  // Letter spacing shows for Arabic on whitespaces
        auto noLetterSpacing = layout("Katibeh", spaced, 0.0);
        auto withLetterSpacing = layout("Katibeh", spaced, 10.0);
        REPORTER_ASSERT(reporter,
                        SkScalarNearlyEqual(withLetterSpacing, noLetterSpacing, EPSILON100));
    }
}

UNIX_ONLY_TEST(SkParagraph_ArabicWithLetterSpacingWhitespacesAsCSS, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    TestCanvas canvas("SkParagraph_SpacedArabicParagraph.png");
    const char* arabic = "سلام";
    const char* spaced = "س لا م";
    ParagraphStyle paragraph_style;
    paragraph_style.setLetterSpacingByCSSSpec(true);
    TextStyle text_style;
    text_style.setFontSize(20);
    text_style.setColor(SK_ColorBLACK);

    auto layout = [&](const char* familyName, const char* text, double letterSpacing) -> double {
        ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());
        text_style.setLetterSpacing(letterSpacing);
        text_style.setFontFamilies({SkString(familyName)});
        builder.pushStyle(text_style);
        builder.addText(text, strlen(text));
        builder.pop();

        auto paragraph = builder.Build();
        paragraph->layout(TestCanvasWidth);
        paragraph->paint(canvas.get(), 20, 0);
        canvas.get()->translate(0, paragraph->getHeight() + 20);
        return paragraph->getLongestLine();
    };
    {  // Letter spacing does not show for Arabic without whitespaces
        auto noLetterSpacing = layout("Katibeh", arabic, 0.0);
        auto withLetterSpacing = layout("Katibeh", arabic, 10.0);
        REPORTER_ASSERT(reporter,
                        SkScalarNearlyEqual(withLetterSpacing, noLetterSpacing, EPSILON100));
    }
    {  // Letter spacing shows for Arabic on whitespaces
        auto noLetterSpacing = layout("Katibeh", spaced, 0.0);
        auto withLetterSpacing = layout("Katibeh", spaced, 10.0);
        REPORTER_ASSERT(reporter,
                        SkScalarNearlyEqual(withLetterSpacing, noLetterSpacing + 20.0, EPSILON100));
    }
}

// Letter spacing is part of the cluster widths that line breaking and intrinsic widths add up,
// so a left aligned line must draw and report its glyphs inside that width, even when only the
// start of the line is letter spaced
DEF_TEST(SkParagraph_LetterSpacingStaysInsideLineWidth, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    fontCollection->disableFontFallback();

    TextStyle plain;
    plain.setFontFamilies({SkString("Roboto"), SkString("Noto Naskh Arabic")});
    plain.setFontSize(14);
    plain.setColor(SK_ColorBLACK);
    TextStyle spaced = plain;
    spaced.setLetterSpacing(30);
    TextStyle tight = plain;
    tight.setLetterSpacing(-4);

    using Segments = std::vector<std::pair<const TextStyle*, const char*>>;
    auto build = [&](size_t maxLines,
                     const Segments& segments,
                     int count,
                     TextDirection direction = TextDirection::kLtr) {
        ParagraphStyle paragraphStyle;
        paragraphStyle.setTextStyle(plain);
        paragraphStyle.setMaxLines(maxLines);
        paragraphStyle.setTextDirection(direction);
        // Right or centered lines replace the shift with the alignment offset
        paragraphStyle.setTextAlign(TextAlign::kLeft);
        ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());
        for (int i = 0; i < count; ++i) {
            for (auto& [style, text] : segments) {
                builder.pushStyle(*style);
                builder.addText(text);
                builder.pop();
            }
        }
        return builder.Build();
    };

    auto checkInside = [&](Paragraph* paragraph, SkScalar width, bool checkBoxes = true) {
        const SkScalar epsilon = 0.01f;
        SkScalar drawnLeft = 0;
        SkScalar drawnRight = 0;
        paragraph->visit([&](int, const Paragraph::VisitorInfo* info) {
            if (!info) {
                return;
            }
            std::vector<SkScalar> advances(SkToSizeT(info->count));
            info->font.getWidths({info->glyphs, SkToSizeT(info->count)}, advances);
            for (int i = 0; i < info->count; ++i) {
                SkScalar x = info->origin.fX + info->positions[i].fX;
                drawnLeft = std::min(drawnLeft, x);
                drawnRight = std::max(drawnRight, x + advances[i]);
            }
        });
        REPORTER_ASSERT(reporter, drawnLeft >= -epsilon, "%f", drawnLeft);
        REPORTER_ASSERT(reporter, drawnRight <= width + epsilon, "%f > %f", drawnRight, width);
        if (!checkBoxes) {
            return;
        }
        auto boxes = paragraph->getRectsForRange(
                0, 1000, RectHeightStyle::kTight, RectWidthStyle::kTight);
        REPORTER_ASSERT(reporter, !boxes.empty());
        for (auto& box : boxes) {
            REPORTER_ASSERT(reporter, box.rect.fLeft >= -epsilon, "%f", box.rect.fLeft);
            REPORTER_ASSERT(reporter,
                            box.rect.fRight <= width + epsilon,
                            "%f > %f",
                            box.rect.fRight,
                            width);
        }
    };

    // Lines end with plain text after letter spaced text; maxLines makes Flutter clip the text
    // at the max intrinsic width
    auto clipped = build(3, {{&spaced, "-1 12345"}, {&plain, "67890\n"}}, 5);
    clipped->layout(1000);
    REPORTER_ASSERT(reporter, clipped->didExceedMaxLines());
    checkInside(clipped.get(), clipped->getMaxIntrinsicWidth());
    checkInside(clipped.get(), clipped->getLongestLine());

    // A letter spaced start followed by a long plain word that wraps at the paragraph width
    auto wrapped =
            build(std::numeric_limits<size_t>::max(),
                  {{&spaced, "12345"}, {&plain, "678901234567890123456789012345678901234567890"}},
                  1);
    wrapped->layout(300);
    REPORTER_ASSERT(reporter, wrapped->lineNumber() > 1);
    checkInside(wrapped.get(), 300);

    // The line ends with negative letter spacing, which must not move the line left; the last
    // glyph still draws its whole advance, 4 px past its tight cell
    auto negative = build(1, {{&spaced, "12345"}, {&tight, "67890"}}, 1);
    negative->layout(1000);
    checkInside(negative.get(), negative->getMaxIntrinsicWidth() + 4);

    // The line ends with a cursive script, which gets no letter spacing
    auto cursive = build(1, {{&spaced, "12345 \u0645\u0631\u062D\u0628\u0627"}}, 1);
    cursive->layout(1000);
    checkInside(cursive.get(), cursive->getMaxIntrinsicWidth());

    // Glyphs of right to left runs are laid out left to right too, each with its spacing on the
    // right, so the visually rightmost cluster decides whether the shift fits
    const char* spacedHebrew = "\u05D0\u05D1\u05D2\u05D3\u05D4";
    const char* plainHebrew = "\u05D5\u05D6\u05D7\u05D8\u05D9";
    auto drawnLeft = [](Paragraph* paragraph) {
        SkScalar left = SK_ScalarMax;
        paragraph->visit([&](int, const Paragraph::VisitorInfo* info) {
            if (info) {
                left = std::min(left, info->origin.fX + info->positions[0].fX);
            }
        });
        return left;
    };

    // Right to left text with a letter spaced logical start: that start is the visually
    // rightmost cluster, so the line fits with the shift and keeps it. Only the ink is checked:
    // the box of a right to left cluster includes its spacing even at the visual line end
    auto rtl = build(1, {{&spaced, spacedHebrew}, {&plain, plainHebrew}}, 1, TextDirection::kRtl);
    rtl->layout(1000);
    checkInside(rtl.get(), rtl->getMaxIntrinsicWidth(), /*checkBoxes=*/false);
    REPORTER_ASSERT(
            reporter, SkScalarNearlyEqual(drawnLeft(rtl.get()), 15), "%f", drawnLeft(rtl.get()));

    // A right to left run at the end of a left to right line, letter spaced only at its logical
    // end: its visually rightmost cluster is plain
    auto mixed = build(1, {{&spaced, "12345"}, {&plain, plainHebrew}, {&spaced, spacedHebrew}}, 1);
    mixed->layout(1000);
    checkInside(mixed.get(), mixed->getMaxIntrinsicWidth());
}

// A space-less font drops a trailing U+200B, leaving a cluster without glyphs at the end of the
// run; per-cluster spacing (two styles or word spacing) must not move the end of the run twice
DEF_TEST(SkParagraph_SpacingTrailingClusterWithoutGlyphs, reporter) {
    auto typeface = ToolUtils::TestFontMgr()->makeFromStream(
            GetResourceAsStream("fonts/abc/abc.ttf"), {});
    if (!typeface) {
        return;
    }
    auto fontProvider = sk_make_sp<TypefaceFontProvider>();
    fontProvider->registerTypeface(typeface, SkString("AbcFont"));
    auto fontCollection = sk_make_sp<FontCollection>();
    fontCollection->setAssetFontManager(fontProvider);
    fontCollection->disableFontFallback();

    // The last piece gets another color, which forces the per-cluster spacing path
    auto layout = [&](const char* text, const char* lastPiece, SkScalar letterSpacing,
                      SkScalar wordSpacing, size_t rangeEnd = 2) {
        TextStyle style;
        style.setFontFamilies({SkString("AbcFont")});
        style.setFontSize(20.0f);
        style.setColor(SK_ColorBLACK);
        style.setLetterSpacing(letterSpacing);
        style.setWordSpacing(wordSpacing);
        ParagraphBuilderImpl builder(ParagraphStyle(), fontCollection, get_unicode());
        builder.pushStyle(style);
        builder.addText(text);
        if (lastPiece) {
            style.setColor(SK_ColorRED);
            builder.pushStyle(style);
            builder.addText(lastPiece);
            builder.pop();
        }
        builder.pop();
        auto paragraph = builder.Build();
        paragraph->layout(1000.0f);
        std::vector<LineMetrics> metrics;
        paragraph->getLineMetrics(metrics);
        return paragraph->getRectsForRange(
                0, rangeEnd, RectHeightStyle::kTight, RectWidthStyle::kTight);
    };

    for (SkScalar spacing : {3.0f, -2.0f}) {
        auto expected = layout("ab", "c", spacing, 0.0f);
        auto twoStyles = layout("ab", "\u200B", spacing, 0.0f);
        auto wordSpacing = layout("ab\u200B", nullptr, spacing, 1.0f);
        REPORTER_ASSERT(reporter,
                        expected.size() == 1 && twoStyles.size() == 1 && wordSpacing.size() == 1);
        if (expected.size() != 1 || twoStyles.size() != 1 || wordSpacing.size() != 1) {
            return;
        }
        REPORTER_ASSERT(reporter,
                        SkScalarNearlyEqual(
                                expected[0].rect.width(), twoStyles[0].rect.width(), EPSILON100),
                        "%f != %f", expected[0].rect.width(), twoStyles[0].rect.width());
        REPORTER_ASSERT(reporter,
                        SkScalarNearlyEqual(
                                expected[0].rect.width(), wordSpacing[0].rect.width(), EPSILON100),
                        "%f != %f", expected[0].rect.width(), wordSpacing[0].rect.width());
    }

    // Word spacing on a space followed by a dropped LRM in the same run must still extend the run
    auto expectedSpace = layout("ab c", nullptr, 0.0f, 10.0f, /*rangeEnd=*/3);
    auto spaceBeforeEmpty = layout("ab \u200E", nullptr, 0.0f, 10.0f, /*rangeEnd=*/3);
    REPORTER_ASSERT(reporter, expectedSpace.size() == 1 && spaceBeforeEmpty.size() == 1);
    if (expectedSpace.size() == 1 && spaceBeforeEmpty.size() == 1) {
        REPORTER_ASSERT(reporter,
                        SkScalarNearlyEqual(expectedSpace[0].rect.width(),
                                            spaceBeforeEmpty[0].rect.width(),
                                            EPSILON100),
                        "%f != %f",
                        expectedSpace[0].rect.width(),
                        spaceBeforeEmpty[0].rect.width());
    }
}

// Collection with one resource font and no fallback, or nullptr when the font cannot be loaded
static sk_sp<FontCollection> single_font_collection(const char* resource, const char* family) {
    auto typeface = ToolUtils::TestFontMgr()->makeFromStream(GetResourceAsStream(resource), {});
    if (!typeface) {
        return nullptr;
    }
    auto fontProvider = sk_make_sp<TypefaceFontProvider>();
    fontProvider->registerTypeface(typeface, SkString(family));
    auto fontCollection = sk_make_sp<FontCollection>();
    fontCollection->setAssetFontManager(fontProvider);
    fontCollection->disableFontFallback();
    return fontCollection;
}

// A differently colored last byte splits the text into two styles, which forces the per-cluster
// letter spacing path instead of the single-style fast path
static std::unique_ptr<Paragraph> layout_letter_spaced(sk_sp<FontCollection> fontCollection,
                                                       const char* family,
                                                       const std::string& text,
                                                       SkScalar letterSpacing,
                                                       bool twoStyles = false,
                                                       TextDirection dir = TextDirection::kLtr) {
    ParagraphStyle paragraphStyle;
    paragraphStyle.setTextDirection(dir);
    TextStyle style;
    style.setFontFamilies({SkString(family)});
    style.setFontSize(20.0f);
    style.setColor(SK_ColorBLACK);
    style.setLetterSpacing(letterSpacing);
    ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());
    builder.pushStyle(style);
    const size_t split = twoStyles ? text.size() - 1 : text.size();
    builder.addText(text.c_str(), split);
    if (twoStyles) {
        style.setColor(SK_ColorRED);
        builder.pushStyle(style);
        builder.addText(text.c_str() + split, text.size() - split);
        builder.pop();
    }
    builder.pop();
    auto paragraph = builder.Build();
    paragraph->layout(1000.0f);
    return paragraph;
}

static constexpr std::array<const char*, 5> kLetterSpacingIgnorables = {
        "\u200B", "\u200C", "\u200D", "\u2060", "\uFEFF"};

// Fonts with a space glyph shape default-ignorables into zero-advance glyphs; the single-style
// fast path must not letter-space them (flutter#160251)
DEF_TEST(SkParagraph_LetterSpacingSkipsIgnorables_OneStyle, reporter) {
    auto fonts = single_font_collection("fonts/Roboto-Regular.ttf", "Roboto");
    if (!fonts) {
        return;
    }
    const SkScalar expected = layout_letter_spaced(fonts, "Roboto", "OneTwo", 3)->getLongestLine();
    for (const char* ignorable : kLetterSpacingIgnorables) {
        auto paragraph =
                layout_letter_spaced(fonts, "Roboto", std::string("One") + ignorable + "Two", 3);
        REPORTER_ASSERT(reporter,
                        SkScalarNearlyEqual(paragraph->getLongestLine(), expected, EPSILON100));
    }
}

// The per-cluster spacing path used for multi-style text must skip default-ignorables too
DEF_TEST(SkParagraph_LetterSpacingSkipsIgnorables_TwoStyles, reporter) {
    auto fonts = single_font_collection("fonts/Roboto-Regular.ttf", "Roboto");
    if (!fonts) {
        return;
    }
    const SkScalar expected =
            layout_letter_spaced(fonts, "Roboto", "OneTwo", 3, true)->getLongestLine();
    for (const char* ignorable : kLetterSpacingIgnorables) {
        auto paragraph = layout_letter_spaced(
                fonts, "Roboto", std::string("One") + ignorable + "Two", 3, true);
        REPORTER_ASSERT(reporter,
                        SkScalarNearlyEqual(paragraph->getLongestLine(), expected, EPSILON100));
    }
}

// A leading ignorable must not take the line's half letter spacing from the first letter
DEF_TEST(SkParagraph_LetterSpacingSkipsIgnorables_Leading, reporter) {
    auto fonts = single_font_collection("fonts/Roboto-Regular.ttf", "Roboto");
    if (!fonts) {
        return;
    }
    for (bool twoStyles : {false, true}) {
        auto plain = layout_letter_spaced(fonts, "Roboto", "One", 3, twoStyles);
        auto leading = layout_letter_spaced(fonts, "Roboto", "\u200BOne", 3, twoStyles);
        auto plainRects =
                plain->getRectsForRange(0, 1, RectHeightStyle::kTight, RectWidthStyle::kTight);
        auto leadingRects =
                leading->getRectsForRange(1, 2, RectHeightStyle::kTight, RectWidthStyle::kTight);
        REPORTER_ASSERT(reporter, plainRects.size() == 1 && leadingRects.size() == 1);
        if (plainRects.size() != 1 || leadingRects.size() != 1) {
            return;
        }
        REPORTER_ASSERT(reporter,
                        SkScalarNearlyEqual(
                                plainRects[0].rect.fLeft, leadingRects[0].rect.fLeft, EPSILON100));
        auto impl = static_cast<ParagraphImpl*>(leading.get());
        REPORTER_ASSERT(reporter,
                        impl->cluster(impl->clusterIndex(0)).isZeroWidthDefaultIgnorable());
        REPORTER_ASSERT(reporter,
                        !impl->cluster(impl->clusterIndex(3)).isZeroWidthDefaultIgnorable());
    }
}

// An invisible cluster at the visual end of a line has no letter spacing, so the line shift
// follows the last visible cluster, the same as without the invisible one
DEF_TEST(SkParagraph_LetterSpacingSkipsIgnorables_Trailing, reporter) {
    auto fonts = single_font_collection("fonts/Roboto-Regular.ttf", "Roboto");
    if (!fonts) {
        return;
    }
    for (SkScalar spacing : {3.0f, 10.0f}) {
        auto plain = layout_letter_spaced(fonts, "Roboto", "One", spacing);
        auto trailing = layout_letter_spaced(fonts, "Roboto", "One\u200B", spacing);
        auto plainRects =
                plain->getRectsForRange(0, 1, RectHeightStyle::kTight, RectWidthStyle::kTight);
        auto trailingRects =
                trailing->getRectsForRange(0, 1, RectHeightStyle::kTight, RectWidthStyle::kTight);
        REPORTER_ASSERT(reporter, plainRects.size() == 1 && trailingRects.size() == 1);
        if (plainRects.size() != 1 || trailingRects.size() != 1) {
            return;
        }
        REPORTER_ASSERT(reporter,
                        SkScalarNearlyEqual(plainRects[0].rect.fLeft,
                                            trailingRects[0].rect.fLeft,
                                            EPSILON100),
                        "%f != %f", plainRects[0].rect.fLeft, trailingRects[0].rect.fLeft);
    }
}

// Space-less fonts delete default-ignorables in HarfBuzz, leaving a cluster without glyphs at the
// end of the run that must not move the end of the run a second time
DEF_TEST(SkParagraph_LetterSpacingFontWithoutSpaceGlyph, reporter) {
    auto fonts = single_font_collection("fonts/abc/abc.ttf", "AbcFont");
    if (!fonts) {
        return;
    }
    for (SkScalar spacing : {3.0f, -2.0f}) {
        auto ab = layout_letter_spaced(fonts, "AbcFont", "ab", spacing);
        auto abIgn = layout_letter_spaced(fonts, "AbcFont", "ab\u200B", spacing);
        auto abRects = ab->getRectsForRange(0, 2, RectHeightStyle::kTight, RectWidthStyle::kTight);
        auto abIgnRects =
                abIgn->getRectsForRange(0, 2, RectHeightStyle::kTight, RectWidthStyle::kTight);
        REPORTER_ASSERT(reporter, abRects.size() == 1 && abIgnRects.size() == 1);
        if (abRects.size() != 1 || abIgnRects.size() != 1) {
            return;
        }
        REPORTER_ASSERT(reporter,
                        SkScalarNearlyEqual(
                                abRects[0].rect.width(), abIgnRects[0].rect.width(), EPSILON100));
        std::vector<LineMetrics> metrics;
        abIgn->getLineMetrics(metrics);
        REPORTER_ASSERT(reporter, metrics.size() == 1);

        auto rtlPlain =
                layout_letter_spaced(fonts, "AbcFont", "b", spacing, false, TextDirection::kRtl);
        auto rtlIgn = layout_letter_spaced(
                fonts, "AbcFont", "\u200Fb", spacing, false, TextDirection::kRtl);
        REPORTER_ASSERT(reporter,
                        SkScalarNearlyEqual(
                                rtlPlain->getLongestLine(), rtlIgn->getLongestLine(), EPSILON100));
        metrics.clear();
        rtlIgn->getLineMetrics(metrics);
        REPORTER_ASSERT(reporter, metrics.size() == 1);
    }
}

// Hangul fillers have Default_Ignorable_Code_Point=true in Unicode, yet shape into visible
// positive-advance blank syllables that must keep letter spacing and leading half-spacing
DEF_TEST(SkParagraph_LetterSpacingKeepsHangulFillers, reporter) {
    auto fonts = sk_make_sp<ResourceFontCollection>(true);
    if (!fonts->fontsFound()) {
        return;
    }
    constexpr const char* kFamily = "Noto Sans CJK JP";
    const SkScalar unspaced =
            layout_letter_spaced(fonts, kFamily, "\uAC00\u115F\u1160\uAC00", 0)->getLongestLine();
    const SkScalar spaced =
            layout_letter_spaced(fonts, kFamily, "\uAC00\u115F\u1160\uAC00", 4)->getLongestLine();
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(spaced, unspaced + 12.0f, EPSILON100));

    auto plain = layout_letter_spaced(fonts, kFamily, "\uAC00\uAC00", 4);
    auto leadingFiller = layout_letter_spaced(fonts, kFamily, "\u3164\uAC00", 4);
    auto plainRects =
            plain->getRectsForRange(0, 1, RectHeightStyle::kTight, RectWidthStyle::kTight);
    auto fillerRects =
            leadingFiller->getRectsForRange(0, 1, RectHeightStyle::kTight, RectWidthStyle::kTight);
    REPORTER_ASSERT(reporter, plainRects.size() == 1 && fillerRects.size() == 1);
    if (plainRects.size() != 1 || fillerRects.size() != 1) {
        return;
    }
    REPORTER_ASSERT(
            reporter,
            SkScalarNearlyEqual(plainRects[0].rect.fLeft, fillerRects[0].rect.fLeft, EPSILON100));
}
