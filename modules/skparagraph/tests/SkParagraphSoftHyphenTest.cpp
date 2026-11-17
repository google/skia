/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
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
#include "modules/skparagraph/src/ParagraphBuilderImpl.h"
#include "modules/skparagraph/src/ParagraphImpl.h"
#include "modules/skparagraph/src/Run.h"
#include "modules/skparagraph/src/TextLine.h"
#include "modules/skparagraph/tests/SkParagraphTestUtils.h"
#include "tests/Test.h"

#include <string.h>
#include <algorithm>
#include <memory>
#include <string>
#include <utility>

using namespace skia::textlayout;
using namespace skia::textlayout::test;

// Soft hyphen (U+00AD) tests
// When a line breaks at a soft hyphen position, a visible hyphen should be rendered.
// When NOT at a line break, soft hyphens should remain invisible (zero-width).

UNIX_ONLY_TEST(SkParagraph_SoftHyphenAtLineBreak, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>(true);
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    // "inter\u00ADnational" — soft hyphen between "inter" and "national"
    // With a narrow width, it should break at the soft hyphen and render a visible hyphen
    const char* text =
            "inter\xC2\xAD"
            "national";

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    paragraph_style.setRenderSoftHyphens(true);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setFontSize(20);
    text_style.setColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text, strlen(text));
    builder.pop();

    auto paragraph = builder.Build();
    // Narrow width forces a break at the soft hyphen. We don't assert on the
    // exact line count because glyph advances vary across font versions; we
    // verify the feature works by checking the hyphen run's structural state.
    paragraph->layout(80);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, !impl->lines().empty());

    // Structural check: the first line should have a hyphen run attached.
    // This is the feature's output regardless of how many total lines the
    // text breaks into, which depends on font metrics.
    auto& firstLine = impl->lines()[0];
    REPORTER_ASSERT(reporter, firstLine.hyphen() != nullptr);
    if (firstLine.hyphen() == nullptr)
        return;
    // The hyphen run should have positive advance width
    REPORTER_ASSERT(reporter, firstLine.hyphen()->advance().fX > 0);
    // The line's total width should include the hyphen
    REPORTER_ASSERT(reporter, firstLine.width() > firstLine.widthWithoutEllipsis());
}

UNIX_ONLY_TEST(SkParagraph_SoftHyphenNoBreak, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>(true);
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    // Same text but with enough width to fit on one line — no hyphen should render
    const char* text =
            "inter\xC2\xAD"
            "national";

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    paragraph_style.setRenderSoftHyphens(true);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setFontSize(20);
    text_style.setColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text, strlen(text));
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);  // Wide enough for one line

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, impl->lines().size() == 1);

    // No hyphen should be rendered when soft hyphen is not at a line break
    auto& firstLine = impl->lines()[0];
    REPORTER_ASSERT(reporter, firstLine.hyphen() == nullptr);
}

// A line can end with a soft hyphen without breaking there. In that case no
// hyphen should be rendered.
static std::unique_ptr<Paragraph> layoutSoftHyphenText(sk_sp<ResourceFontCollection> fontCollection,
                                                       const char* text,
                                                       SkScalar width) {
    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    paragraph_style.setRenderSoftHyphens(true);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setFontSize(20);
    text_style.setColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text, strlen(text));
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(width);
    return paragraph;
}

static bool anyLineHasHyphen(Paragraph* paragraph) {
    auto impl = static_cast<ParagraphImpl*>(paragraph);
    for (auto& line : impl->lines()) {
        if (line.hyphen() != nullptr) {
            return true;
        }
    }
    return false;
}

UNIX_ONLY_TEST(SkParagraph_SoftHyphenAtEndOfParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>(true);
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    // The space keeps this off the single-run layout fast path, which never
    // creates a soft hyphen.
    auto paragraph = layoutSoftHyphenText(fontCollection, "an inter\xC2\xAD", TestCanvasWidth);
    REPORTER_ASSERT(reporter, static_cast<ParagraphImpl*>(paragraph.get())->lines().size() == 1);
    REPORTER_ASSERT(reporter, !anyLineHasHyphen(paragraph.get()));
}

UNIX_ONLY_TEST(SkParagraph_SoftHyphenBeforeSpace, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>(true);
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    // The line breaks at the space, not at the soft hyphen before it.
    auto paragraph = layoutSoftHyphenText(fontCollection,
                                          "inter\xC2\xAD"
                                          " national",
                                          80);
    REPORTER_ASSERT(reporter, static_cast<ParagraphImpl*>(paragraph.get())->lines().size() >= 2);
    REPORTER_ASSERT(reporter, !anyLineHasHyphen(paragraph.get()));
}

UNIX_ONLY_TEST(SkParagraph_SoftHyphenBeforeHardBreak, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>(true);
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    auto paragraph = layoutSoftHyphenText(fontCollection,
                                          "inter\xC2\xAD"
                                          "\nnational",
                                          TestCanvasWidth);
    REPORTER_ASSERT(reporter, static_cast<ParagraphImpl*>(paragraph.get())->lines().size() == 2);
    REPORTER_ASSERT(reporter, !anyLineHasHyphen(paragraph.get()));
}

UNIX_ONLY_TEST(SkParagraph_MultipleSoftHyphens, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>(true);
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    // "su\u00ADper\u00ADcali\u00ADfrag" — multiple soft hyphens
    // Only the one at the actual line break should render
    const char* text =
            "su\xC2\xAD"
            "per\xC2\xAD"
            "cali\xC2\xAD"
            "frag";

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    paragraph_style.setRenderSoftHyphens(true);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setFontSize(20);
    text_style.setColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text, strlen(text));
    builder.pop();

    auto paragraph = builder.Build();
    // Narrow width forces a break at one of the soft hyphens.
    paragraph->layout(80);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, !impl->lines().empty());

    // The first line should have a hyphen (it breaks at a soft hyphen).
    auto& firstLine = impl->lines()[0];
    REPORTER_ASSERT(reporter, firstLine.hyphen() != nullptr);
    if (firstLine.hyphen() == nullptr)
        return;
    REPORTER_ASSERT(reporter, firstLine.hyphen()->advance().fX > 0);
}

UNIX_ONLY_TEST(SkParagraph_SoftHyphenLineWidth, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>(true);
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    const char* text =
            "inter\xC2\xAD"
            "national";

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    paragraph_style.setRenderSoftHyphens(true);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setFontSize(20);
    text_style.setColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text, strlen(text));
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(80);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, !impl->lines().empty());

    auto& firstLine = impl->lines()[0];
    REPORTER_ASSERT(reporter, firstLine.hyphen() != nullptr);
    if (firstLine.hyphen() == nullptr)
        return;

    // Line width should equal text width + hyphen width
    SkScalar textWidth = firstLine.widthWithoutEllipsis();
    SkScalar hyphenWidth = firstLine.hyphen()->advance().fX;
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(firstLine.width(), textWidth + hyphenWidth, EPSILON100));
}

UNIX_ONLY_TEST(SkParagraph_SoftHyphenRTL, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>(true);
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    // Arabic text with soft hyphens: "كلمة\u00ADطويلة" (kalima-tawila = "long word")
    const char* text =
            "\xD9\x83\xD9\x84\xD9\x85\xD8\xA9"           // كلمة
            "\xC2\xAD"                                   // soft hyphen
            "\xD8\xB7\xD9\x88\xD9\x8A\xD9\x84\xD8\xA9";  // طويلة

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    paragraph_style.setRenderSoftHyphens(true);
    paragraph_style.setTextDirection(TextDirection::kRtl);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Noto Naskh Arabic")});
    text_style.setFontSize(20);
    text_style.setColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text, strlen(text));
    builder.pop();

    auto paragraph = builder.Build();
    // Arabic glyphs are wider per character; narrow width forces a break.
    paragraph->layout(40);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, !impl->lines().empty());

    auto& firstLine = impl->lines()[0];
    REPORTER_ASSERT(reporter, firstLine.hyphen() != nullptr);
    if (firstLine.hyphen() == nullptr)
        return;
    REPORTER_ASSERT(reporter, firstLine.hyphen()->advance().fX > 0);
    // Line width should include the hyphen
    REPORTER_ASSERT(reporter, firstLine.width() > firstLine.widthWithoutEllipsis());
}

// Soft hyphen affix must be shaped at the bidi level of the run it follows, not
// a hardcoded LTR level. In an RTL paragraph the last cluster before the break
// is RTL, so the shaped hyphen run must report leftToRight() == false.
UNIX_ONLY_TEST(SkParagraph_SoftHyphenRTLBidiLevel, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>(true);
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    // Arabic "كلمة\u00ADطويلة" — same text as SkParagraph_SoftHyphenRTL.
    const char* text =
            "\xD9\x83\xD9\x84\xD9\x85\xD8\xA9"
            "\xC2\xAD"
            "\xD8\xB7\xD9\x88\xD9\x8A\xD9\x84\xD8\xA9";

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    paragraph_style.setRenderSoftHyphens(true);
    paragraph_style.setTextDirection(TextDirection::kRtl);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Noto Naskh Arabic")});
    text_style.setFontSize(20);
    text_style.setColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text, strlen(text));
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(40);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, !impl->lines().empty());

    auto& firstLine = impl->lines()[0];
    REPORTER_ASSERT(reporter, firstLine.hyphen() != nullptr);
    if (firstLine.hyphen() == nullptr)
        return;
    // The shaped hyphen run inherits the surrounding cluster's bidi level.
    REPORTER_ASSERT(reporter, !firstLine.hyphen()->leftToRight());
    REPORTER_ASSERT(reporter, (firstLine.hyphen()->bidiLevel() % 2) == 1);
}

// Regression guard: the default LTR case must still produce an LTR hyphen run.
UNIX_ONLY_TEST(SkParagraph_SoftHyphenLTRBidiLevel, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>(true);
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    const char* text =
            "inter\xC2\xAD"
            "national";

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    paragraph_style.setRenderSoftHyphens(true);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setFontSize(20);
    text_style.setColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text, strlen(text));
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(80);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, !impl->lines().empty());

    auto& firstLine = impl->lines()[0];
    REPORTER_ASSERT(reporter, firstLine.hyphen() != nullptr);
    if (firstLine.hyphen() == nullptr)
        return;
    REPORTER_ASSERT(reporter, firstLine.hyphen()->leftToRight());
    REPORTER_ASSERT(reporter, (firstLine.hyphen()->bidiLevel() % 2) == 0);
    REPORTER_ASSERT(reporter, firstLine.hyphen()->advance().fX > 0);
}

// Verify that with the default (setRenderSoftHyphens(false)), no visible hyphen
// is rendered even when the line break occurs at a soft hyphen position.
UNIX_ONLY_TEST(SkParagraph_SoftHyphenDefaultOff, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>(true);
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    const char* text =
            "inter\xC2\xAD"
            "national";

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    // Intentionally do NOT call setRenderSoftHyphens — default must be false.
    REPORTER_ASSERT(reporter, !paragraph_style.getRenderSoftHyphens());
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setFontSize(20);
    text_style.setColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text, strlen(text));
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(80);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, !impl->lines().empty());

    // No hyphen run should be created on any line when rendering is disabled.
    for (const auto& line : impl->lines()) {
        REPORTER_ASSERT(reporter, line.hyphen() == nullptr);
    }
    auto& firstLine = impl->lines()[0];
    REPORTER_ASSERT(reporter, firstLine.width() == firstLine.widthWithoutEllipsis());
}

// Returns the horizontal extent {left, right} of the rects that getRectsForRange returns on
// `line` for the UTF-16 range [start, end), or {0, 0} if none of them are on that line.
static std::pair<SkScalar, SkScalar> lineRectExtent(Paragraph* paragraph,
                                                    const TextLine& line,
                                                    size_t start,
                                                    size_t end) {
    auto boxes = paragraph->getRectsForRange(
            start, end, RectHeightStyle::kTight, RectWidthStyle::kTight);
    SkScalar lineY = line.offset().fY + line.height() / 2;
    SkScalar left = SK_ScalarMax, right = 0;
    for (const auto& box : boxes) {
        if (box.rect.fTop <= lineY && box.rect.fBottom >= lineY) {
            left = std::min(left, box.rect.fLeft);
            right = std::max(right, box.rect.fRight);
        }
    }
    return right == 0 ? std::make_pair(0.0f, 0.0f) : std::make_pair(left, right);
}

// The rendered soft hyphen is selected together with the soft hyphen it stands for:
// getRectsForRange covers the hyphen's pixels exactly when the range includes the
// soft hyphen.
UNIX_ONLY_TEST(SkParagraph_SoftHyphenGetRectsForRange, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>(true);
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    const char* text =
            "inter\xC2\xAD"
            "national";
    const size_t textLen = strlen(text);
    // getRectsForRange takes UTF-16 offsets: "inter" is 0-4, the soft hyphen 5, "national" 6-13.
    const size_t softHyphen = 5;
    const size_t textLen16 = 14;

    // A custom hyphen string longer than the soft hyphen (U+2010 is 3 bytes) must not make
    // ranges on the second line select the first line's hyphen.
    for (const char* hyphenString : {"-", "\xE2\x80\x90"}) {
        ParagraphStyle paragraph_style;
        paragraph_style.turnHintingOff();
        paragraph_style.setRenderSoftHyphens(true);
        paragraph_style.setSoftHyphenString(SkString(hyphenString));
        ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());

        TextStyle text_style;
        text_style.setFontFamilies({SkString("Roboto")});
        text_style.setFontSize(20);
        text_style.setColor(SK_ColorBLACK);
        builder.pushStyle(text_style);
        builder.addText(text, textLen);
        builder.pop();

        auto paragraph = builder.Build();
        // Narrow width forces a break at the soft hyphen. Verify via structural
        // checks (hyphen run on first line) rather than exact line counts, since
        // those depend on font metrics that vary across platforms.
        paragraph->layout(80);

        auto impl = static_cast<ParagraphImpl*>(paragraph.get());
        REPORTER_ASSERT(reporter, !impl->lines().empty());
        auto& firstLine = impl->lines()[0];
        REPORTER_ASSERT(reporter, firstLine.hyphen() != nullptr);
        if (firstLine.hyphen() == nullptr)
            return;
        SkScalar textOnlyWidth = firstLine.widthWithoutEllipsis();
        SkScalar lineWidth = firstLine.width();
        REPORTER_ASSERT(reporter, lineWidth > textOnlyWidth);  // hyphen contributes width
        SkScalar lineLeft = firstLine.offset().fX;

        // Ranges that include the soft hyphen cover the rendered hyphen too.
        REPORTER_ASSERT(
                reporter,
                SkScalarNearlyEqual(lineRectExtent(paragraph.get(), firstLine, 0, textLen16).second,
                                    lineLeft + lineWidth,
                                    EPSILON100));
        REPORTER_ASSERT(
                reporter,
                SkScalarNearlyEqual(
                        lineRectExtent(paragraph.get(), firstLine, softHyphen, softHyphen + 1)
                                .second,
                        lineLeft + lineWidth,
                        EPSILON100));
        // A range that stops before the soft hyphen doesn't.
        REPORTER_ASSERT(reporter,
                        lineRectExtent(paragraph.get(), firstLine, 0, softHyphen).second <=
                                lineLeft + textOnlyWidth + EPSILON100);
        // Nor does a range on the second line only.
        REPORTER_ASSERT(
                reporter,
                lineRectExtent(paragraph.get(), firstLine, softHyphen + 1, textLen16).second == 0);
    }
}

// In RTL text the rendered hyphen is at the left edge of its line. It is selected with its
// soft hyphen, and the text after it in visual order is placed after it.
UNIX_ONLY_TEST(SkParagraph_SoftHyphenGetRectsForRangeRtl, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>(true);
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    const char* text =
            "\xD9\x83\xD9\x84\xD9\x85\xD8\xA9"
            "\xC2\xAD"
            "\xD8\xB7\xD9\x88\xD9\x8A\xD9\x84\xD8\xA9";
    const size_t textLen = strlen(text);
    // getRectsForRange takes UTF-16 offsets: the first word is 0-3, the soft hyphen 4, the
    // second word 5-9.
    const size_t softHyphen = 4;
    const size_t textLen16 = 10;

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    paragraph_style.setRenderSoftHyphens(true);
    paragraph_style.setTextDirection(TextDirection::kRtl);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Noto Naskh Arabic")});
    text_style.setFontSize(20);
    text_style.setColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text, textLen);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(60);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, !impl->lines().empty());
    auto& firstLine = impl->lines()[0];
    REPORTER_ASSERT(reporter, firstLine.hyphen() != nullptr);
    if (firstLine.hyphen() == nullptr)
        return;
    SkScalar hyphenWidth = firstLine.width() - firstLine.widthWithoutEllipsis();
    REPORTER_ASSERT(reporter, hyphenWidth > 0);
    SkScalar lineLeft = firstLine.offset().fX;

    // The whole text covers the first line from the hyphen at its left edge.
    auto all = lineRectExtent(paragraph.get(), firstLine, 0, textLen16);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(all.first, lineLeft, EPSILON100));
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(all.second, lineLeft + firstLine.width(), EPSILON100));
    // The first word alone starts after the hyphen.
    auto word = lineRectExtent(paragraph.get(), firstLine, 0, softHyphen);
    REPORTER_ASSERT(reporter, word.first >= lineLeft + hyphenWidth - EPSILON100);
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(word.second, lineLeft + firstLine.width(), EPSILON100));
}

// Verifies that clicking inside the rendered hyphen's pixel region maps to the
// source text offset at the end of the line (i.e. just after the soft hyphen),
// not into the synthetic hyphen run itself.
UNIX_ONLY_TEST(SkParagraph_SoftHyphenGlyphPositionAtCoordinate, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>(true);
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    const char* text =
            "inter\xC2\xAD"
            "national";

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    paragraph_style.setRenderSoftHyphens(true);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setFontSize(20);
    text_style.setColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text, strlen(text));
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(80);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, !impl->lines().empty());
    auto& firstLine = impl->lines()[0];
    REPORTER_ASSERT(reporter, firstLine.hyphen() != nullptr);
    if (firstLine.hyphen() == nullptr)
        return;

    SkScalar lineY = firstLine.offset().fY + firstLine.height() / 2;
    SkScalar textRight = firstLine.offset().fX + firstLine.widthWithoutEllipsis();
    SkScalar lineRight = firstLine.offset().fX + firstLine.width();

    // Click inside the rendered hyphen pixel region (between text end and line end).
    auto inHyphen = paragraph->getGlyphPositionAtCoordinate((textRight + lineRight) / 2, lineY);
    // The position should land at the end of the first line's source text -- i.e.
    // somewhere within the soft-hyphen cluster. It must NOT be greater than the
    // start of the second line's "national" text (position 7 = after "inter\xC2\xAD").
    REPORTER_ASSERT(reporter, inHyphen.position <= 7);
    REPORTER_ASSERT(reporter, inHyphen.position >= 5);
}

// When the rendered hyphen uses a fallback font, the soft hyphen still gets exactly one box with
// the hyphen's width: the unrendered U+00AD cluster in the text run must not add a 0-width box,
// which merge() can't combine with the hyphen's (and getGlyphInfoAtUTF16Offset would report)
UNIX_ONLY_TEST(SkParagraph_SoftHyphenFallbackFontRect, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>(true);
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    const char* text = "inter\xC2\xADnational";
    const size_t softHyphen = 5;

    ParagraphStyle paragraphStyle;
    paragraphStyle.turnHintingOff();
    paragraphStyle.setRenderSoftHyphens(true);
    // U+0640 is absent from Roboto and falls back to Noto Naskh Arabic, preventing box merging
    paragraphStyle.setSoftHyphenString(SkString("\xD9\x80"));
    ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());

    TextStyle textStyle;
    textStyle.setFontFamilies({SkString("Roboto"), SkString("Noto Naskh Arabic")});
    textStyle.setFontSize(20);
    builder.pushStyle(textStyle);
    builder.addText(text, strlen(text));
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(80);

    auto boxes = paragraph->getRectsForRange(
            softHyphen, softHyphen + 1, RectHeightStyle::kTight, RectWidthStyle::kTight);
    REPORTER_ASSERT(reporter, boxes.size() == 1);
    if (!boxes.empty()) {
        REPORTER_ASSERT(reporter, boxes[0].rect.width() > 0);
    }

    Paragraph::GlyphInfo glyphInfo;
    REPORTER_ASSERT(reporter, paragraph->getGlyphInfoAtUTF16Offset(softHyphen, &glyphInfo));
    REPORTER_ASSERT(reporter, glyphInfo.fGraphemeLayoutBounds.width() > 0);
}

// With letter spacing, the soft hyphen's box starts where the line's text ends: the letterSpacing
// that addLetterSpacesEvenly() adds to the invisible U+00AD cluster in the text run must not merge
// into the synthetic hyphen's box and shift it left
UNIX_ONLY_TEST(SkParagraph_SoftHyphenLetterSpacingRect, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>(true);
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    const char* text = "inter\xC2\xADnational";
    const size_t softHyphen = 5;
    const size_t textLen16 = 14;

    ParagraphStyle paragraphStyle;
    paragraphStyle.turnHintingOff();
    paragraphStyle.setRenderSoftHyphens(true);
    ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());

    TextStyle textStyle;
    textStyle.setFontFamilies({SkString("Roboto")});
    textStyle.setFontSize(20);
    textStyle.setLetterSpacing(10);
    builder.pushStyle(textStyle);
    builder.addText(text, strlen(text));
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(140);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, !impl->lines().empty());
    auto& firstLine = impl->lines()[0];
    REPORTER_ASSERT(reporter, firstLine.hyphen() != nullptr);
    if (firstLine.hyphen() == nullptr) {
        return;
    }

    SkScalar lineLeft = firstLine.offset().fX;
    SkScalar textOnlyWidth = firstLine.widthWithoutEllipsis();
    SkScalar lineWidth = firstLine.width();

    auto allExtent = lineRectExtent(paragraph.get(), firstLine, 0, textLen16);
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(allExtent.second, lineLeft + lineWidth, EPSILON100));

    auto hyphenExtent = lineRectExtent(paragraph.get(), firstLine, softHyphen, softHyphen + 1);
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(hyphenExtent.first, lineLeft + textOnlyWidth, EPSILON100));
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(hyphenExtent.second, lineLeft + lineWidth, EPSILON100));
}

// In an RTL paragraph with an LTR word, the soft hyphen's box is just the synthetic hyphen at the
// left edge of the line: the U+00AD cluster in the LTR text run must not add a 0-width box at the
// right edge, which would stretch the soft hyphen's extent across the whole line
UNIX_ONLY_TEST(SkParagraph_SoftHyphenBidiRect, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>(true);
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    const char* text = "inter\xC2\xADnational";
    const size_t softHyphen = 5;

    ParagraphStyle paragraphStyle;
    paragraphStyle.turnHintingOff();
    paragraphStyle.setRenderSoftHyphens(true);
    paragraphStyle.setTextDirection(TextDirection::kRtl);
    ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());

    TextStyle textStyle;
    textStyle.setFontFamilies({SkString("Roboto")});
    textStyle.setFontSize(20);
    builder.pushStyle(textStyle);
    builder.addText(text, strlen(text));
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(80);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, !impl->lines().empty());
    auto& firstLine = impl->lines()[0];
    REPORTER_ASSERT(reporter, firstLine.hyphen() != nullptr);
    if (firstLine.hyphen() == nullptr) {
        return;
    }

    SkScalar hyphenWidth = firstLine.width() - firstLine.widthWithoutEllipsis();
    auto boxes = paragraph->getRectsForRange(
            softHyphen, softHyphen + 1, RectHeightStyle::kTight, RectWidthStyle::kTight);
    REPORTER_ASSERT(reporter, boxes.size() == 1);

    auto extent = lineRectExtent(paragraph.get(), firstLine, softHyphen, softHyphen + 1);
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(extent.second - extent.first, hyphenWidth, EPSILON100));
}

// In RTL, the hyphen is visited first, at the left edge of the line, so
// getGlyphPositionAtCoordinate must count its width: otherwise the hit-test offsets of all the
// runs after it are shifted left by hyphenWidth
UNIX_ONLY_TEST(SkParagraph_SoftHyphenGlyphPositionAtCoordinateRtl, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>(true);
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    const char* text =
            "\xD9\x83\xD9\x84\xD9\x85\xD8\xA9"
            "\xC2\xAD"
            "\xD8\xB7\xD9\x88\xD9\x8A\xD9\x84\xD8\xA9";

    ParagraphStyle paragraphStyle;
    paragraphStyle.turnHintingOff();
    paragraphStyle.setRenderSoftHyphens(true);
    paragraphStyle.setTextDirection(TextDirection::kRtl);
    ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());

    TextStyle textStyle;
    textStyle.setFontFamilies({SkString("Noto Naskh Arabic")});
    textStyle.setFontSize(20);
    builder.pushStyle(textStyle);
    builder.addText(text, strlen(text));
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(60);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, !impl->lines().empty());
    auto& firstLine = impl->lines()[0];
    REPORTER_ASSERT(reporter, firstLine.hyphen() != nullptr);
    if (firstLine.hyphen() == nullptr) {
        return;
    }

    SkScalar lineY = firstLine.offset().fY + firstLine.height() / 2;
    // The first RTL letter 'ك' (index 0) is at the right edge of the line [51.70, 60.00];
    // clicking near the right edge (x = 59) must map to the start of the word (0, kDownstream)
    SkScalar rightEdge = firstLine.offset().fX + firstLine.width();
    auto posAtRight = paragraph->getGlyphPositionAtCoordinate(rightEdge - 1.0f, lineY);
    REPORTER_ASSERT(reporter, posAtRight.position == 0);
    REPORTER_ASSERT(reporter, posAtRight.affinity == Affinity::kDownstream);

    // Every character's center from getRectsForRange must hit-test back to that same character
    for (size_t i = 0; i < 4; ++i) {
        auto charBoxes = paragraph->getRectsForRange(
                i, i + 1, RectHeightStyle::kTight, RectWidthStyle::kTight);
        REPORTER_ASSERT(reporter, !charBoxes.empty());
        if (!charBoxes.empty()) {
            SkScalar centerX = charBoxes[0].rect.centerX();
            Paragraph::GlyphInfo glyphInfo;
            REPORTER_ASSERT(reporter,
                            paragraph->getClosestUTF16GlyphInfoAt(centerX, lineY, &glyphInfo));
            REPORTER_ASSERT(reporter, glyphInfo.fGraphemeClusterTextRange.start == i);
        }
    }
}

// Verifies that getWordBoundary is unaffected by soft hyphen rendering: word
// boundaries are computed from source text only.
UNIX_ONLY_TEST(SkParagraph_SoftHyphenWordBoundary, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>(true);
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    const char* text =
            "inter\xC2\xAD"
            "national";

    auto buildParagraph = [&](bool renderHyphens) {
        ParagraphStyle paragraph_style;
        paragraph_style.turnHintingOff();
        paragraph_style.setRenderSoftHyphens(renderHyphens);
        ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());

        TextStyle text_style;
        text_style.setFontFamilies({SkString("Roboto")});
        text_style.setFontSize(20);
        text_style.setColor(SK_ColorBLACK);
        builder.pushStyle(text_style);
        builder.addText(text, strlen(text));
        builder.pop();
        auto paragraph = builder.Build();
        paragraph->layout(80);
        return paragraph;
    };

    auto withHyphen = buildParagraph(true);
    auto withoutHyphen = buildParagraph(false);

    // Word boundaries must be identical regardless of soft hyphen rendering.
    for (unsigned i = 0; i < strlen(text); ++i) {
        auto a = withHyphen->getWordBoundary(i);
        auto b = withoutHyphen->getWordBoundary(i);
        REPORTER_ASSERT(reporter, a == b);
    }
}

// Verifies that ParagraphStyle::setSoftHyphenString overrides the default "-"
// glyph rendered at soft hyphen line breaks, and that an empty string disables
// the rendered glyph entirely.
UNIX_ONLY_TEST(SkParagraph_SoftHyphenCustomString, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>(true);
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    const char* text =
            "inter\xC2\xAD"
            "national";

    auto build = [&](const SkString& hyphenStr) {
        ParagraphStyle paragraph_style;
        paragraph_style.turnHintingOff();
        paragraph_style.setRenderSoftHyphens(true);
        paragraph_style.setSoftHyphenString(hyphenStr);
        ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());
        TextStyle text_style;
        text_style.setFontFamilies({SkString("Roboto")});
        text_style.setFontSize(20);
        text_style.setColor(SK_ColorBLACK);
        builder.pushStyle(text_style);
        builder.addText(text, strlen(text));
        builder.pop();
        auto paragraph = builder.Build();
        // Wide enough for "national", so the only line break is at the soft hyphen.
        paragraph->layout(80);
        return paragraph;
    };

    // The default hyphen string is "-".
    REPORTER_ASSERT(reporter, ParagraphStyle().getSoftHyphenString() == SkString("-"));

    // The std::u16string overload stores the string as UTF-8 (U+2010 HYPHEN here).
    ParagraphStyle utf16Style;
    utf16Style.setSoftHyphenString(std::u16string(u"\u2010"));
    REPORTER_ASSERT(reporter, utf16Style.getSoftHyphenString() == SkString("\xE2\x80\x90"));

    auto pDefault = build(SkString("-"));
    auto pCustom = build(SkString("="));

    auto* implDefault = static_cast<ParagraphImpl*>(pDefault.get());
    auto* implCustom = static_cast<ParagraphImpl*>(pCustom.get());
    REPORTER_ASSERT(reporter, implDefault->lines().size() == 2);
    REPORTER_ASSERT(reporter, implCustom->lines().size() == 2);

    auto& firstDefault = implDefault->lines()[0];
    auto& firstCustom = implCustom->lines()[0];
    REPORTER_ASSERT(reporter, firstDefault.hyphen() != nullptr);
    REPORTER_ASSERT(reporter, firstCustom.hyphen() != nullptr);
    REPORTER_ASSERT(reporter, firstCustom.hyphen()->advance().fX > 0);
    // "=" and "-" have different advances, so this shows the custom string was used.
    REPORTER_ASSERT(reporter,
                    firstCustom.hyphen()->advance().fX != firstDefault.hyphen()->advance().fX);

    // Empty string disables the rendered glyph (no fHyphen even at the break).
    auto pEmpty = build(SkString(""));
    auto* implEmpty = static_cast<ParagraphImpl*>(pEmpty.get());
    REPORTER_ASSERT(reporter, implEmpty->lines().size() == 2);
    REPORTER_ASSERT(reporter, implEmpty->lines()[0].hyphen() == nullptr);
}

// Returns `text` with each '~' replaced by a soft hyphen (U+00AD).
static std::string withSoftHyphens(const char* text) {
    std::string result;
    for (const char* c = text; *c != '\0'; ++c) {
        if (*c == '~') {
            result += "\xC2\xAD";
        } else {
            result += *c;
        }
    }
    return result;
}

// Lays out `text`, with '~' for each soft hyphen, in Roboto 20 with rendered soft hyphens at
// `width`. The rounding hack is off (as in Flutter) so that line widths can be compared exactly.
static std::unique_ptr<Paragraph> layoutWithSoftHyphens(
        sk_sp<ResourceFontCollection> fontCollection,
        const char* text,
        SkScalar width,
        const std::function<void(ParagraphStyle&)>& configure = nullptr) {
    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    paragraph_style.setApplyRoundingHack(false);
    paragraph_style.setRenderSoftHyphens(true);
    if (configure) {
        configure(paragraph_style);
    }
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());
    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setFontSize(20);
    text_style.setColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    std::string utf8 = withSoftHyphens(text);
    builder.addText(utf8.c_str(), utf8.size());
    builder.pop();
    auto paragraph = builder.Build();
    paragraph->layout(width);
    return paragraph;
}

// The width of `text` on a single line.
static SkScalar singleLineWidth(sk_sp<ResourceFontCollection> fontCollection, const char* text) {
    return layoutWithSoftHyphens(fontCollection, text, 10000)->getMaxIntrinsicWidth();
}

// Each line's text and whether it ends in a rendered hyphen, e.g. "xy|abcd-|efgh".
static SkString describeLines(Paragraph* paragraph) {
    auto impl = static_cast<ParagraphImpl*>(paragraph);
    SkString result;
    for (auto& line : impl->lines()) {
        if (!result.isEmpty()) {
            result.append("|");
        }
        auto range = line.trimmedText();
        for (size_t i = range.start; i < range.end; ++i) {
            char c = impl->text()[i];
            // Leave out the soft hyphen's two UTF-8 bytes.
            if ((c & 0x80) == 0) {
                result.append(&c, 1);
            }
        }
        if (line.hyphen() != nullptr) {
            result.append("-");
        }
    }
    return result;
}

// Checks describeLines(paragraph) against `expected`, reporting the actual lines if they differ.
static void checkLines(skiatest::Reporter* reporter, Paragraph* paragraph, const char* expected) {
    SkString lines = describeLines(paragraph);
    REPORTER_ASSERT(reporter, lines.equals(expected), "lines: %s", lines.c_str());
}

// A soft hyphen break also draws a hyphen, so it only fits if the hyphen does too. If it
// doesn't, the line breaks at the earlier space instead, like browsers do.
UNIX_ONLY_TEST(SkParagraph_SoftHyphenBreakReservesHyphenWidth, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>(true);
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    SkScalar hyphen = singleLineWidth(fontCollection, "-");
    SkScalar width = singleLineWidth(fontCollection, "xy abcd") + hyphen / 2;
    auto paragraph = layoutWithSoftHyphens(fontCollection, "xy abcd~efgh", width);
    checkLines(reporter, paragraph.get(), "xy|abcd-|efgh");
    REPORTER_ASSERT(reporter, paragraph->getLongestLine() <= width);
}

// Likewise, an earlier soft hyphen is used when the later one doesn't fit with its hyphen.
UNIX_ONLY_TEST(SkParagraph_SoftHyphenBreakFallsBackToEarlierSoftHyphen, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>(true);
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    SkScalar hyphen = singleLineWidth(fontCollection, "-");
    SkScalar width = singleLineWidth(fontCollection, "abcd") + hyphen / 2;
    auto paragraph = layoutWithSoftHyphens(fontCollection, "ab~cd~efgh", width);
    checkLines(reporter, paragraph.get(), "ab-|cd-|efgh");
    REPORTER_ASSERT(reporter, paragraph->getLongestLine() <= width);
}

// When several soft hyphen breaks don't fit with their hyphens, the line ends at the last break
// that does: here, the space.
UNIX_ONLY_TEST(SkParagraph_SoftHyphenBreakFallsBackPastSoftHyphensThatDontFit, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>(true);
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    SkScalar hyphen = singleLineWidth(fontCollection, "-");
    SkScalar width = singleLineWidth(fontCollection, "x ab") + hyphen / 2;
    auto paragraph = layoutWithSoftHyphens(fontCollection, "x ab~cd~ef", width);
    SkString lines = describeLines(paragraph.get());
    REPORTER_ASSERT(reporter, lines.startsWith("x|ab-|"), "lines: %s", lines.c_str());
    REPORTER_ASSERT(reporter, paragraph->getLongestLine() <= width);
}

// The room reserved is the width of the hyphen string that's drawn, not of "-".
UNIX_ONLY_TEST(SkParagraph_SoftHyphenBreakReservesCustomHyphenWidth, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>(true);
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    SkScalar hyphen = singleLineWidth(fontCollection, "-");
    SkScalar width = singleLineWidth(fontCollection, "xy abcd") + hyphen * 1.5f;
    const char* text = "xy abcd~efgh";
    auto paragraph = layoutWithSoftHyphens(fontCollection, text, width);
    checkLines(reporter, paragraph.get(), "xy abcd-|efgh");
    paragraph = layoutWithSoftHyphens(fontCollection, text, width, [](ParagraphStyle& style) {
        style.setSoftHyphenString(SkString("--"));
    });
    checkLines(reporter, paragraph.get(), "xy|abcd-|efgh");
    REPORTER_ASSERT(reporter, paragraph->getLongestLine() <= width);
}

// A line that gets an ellipsis draws no hyphen, so it breaks the same whether or not soft
// hyphens are rendered.
UNIX_ONLY_TEST(SkParagraph_SoftHyphenBreakOnEllipsisLineReservesNoRoom, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>(true);
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    SkScalar hyphen = singleLineWidth(fontCollection, "-");
    SkScalar width = singleLineWidth(fontCollection, "xy abcd") + hyphen / 2;
    const char* text = "xy abcd~efgh ijkl";
    auto lines = [&](bool renderSoftHyphens) {
        auto paragraph = layoutWithSoftHyphens(
                fontCollection, text, width, [renderSoftHyphens](ParagraphStyle& style) {
                    style.setRenderSoftHyphens(renderSoftHyphens);
                    style.setMaxLines(1);
                    style.setEllipsis(SkString("..."));
                });
        auto impl = static_cast<ParagraphImpl*>(paragraph.get());
        REPORTER_ASSERT(reporter, impl->lines().size() == 1);
        REPORTER_ASSERT(reporter, impl->lines()[0].ellipsis() != nullptr);
        REPORTER_ASSERT(reporter, impl->lines()[0].hyphen() == nullptr);
        return impl->lines()[0].trimmedText();
    };
    REPORTER_ASSERT(reporter, lines(true) == lines(false));
}

// A placeholder that doesn't fit ends the line after the word before it, so an earlier soft
// hyphen that wouldn't fit with its hyphen doesn't matter.
UNIX_ONLY_TEST(SkParagraph_SoftHyphenBreakBeforePlaceholderNeedsNoRoom, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>(true);
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    const char* text = "xy abcd~i";
    SkScalar width = singleLineWidth(fontCollection, text) + 0.3f;
    SkScalar hyphenated = singleLineWidth(fontCollection, "xy abcd-");
    REPORTER_ASSERT(reporter, hyphenated > width + 0.3f, "%g vs %g", hyphenated, width);

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    paragraph_style.setApplyRoundingHack(false);
    paragraph_style.setRenderSoftHyphens(true);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());
    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setFontSize(20);
    text_style.setColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    std::string utf8 = withSoftHyphens(text);
    builder.addText(utf8.c_str(), utf8.size());
    builder.addPlaceholder(PlaceholderStyle(
            20, 20, PlaceholderAlignment::kBaseline, TextBaseline::kAlphabetic, 0));
    builder.pop();
    auto paragraph = builder.Build();
    paragraph->layout(width);
    // The second line holds only the placeholder.
    checkLines(reporter, paragraph.get(), "xy abcdi|");
}

// With no earlier break opportunity on the line, the soft hyphen break is still used and the
// hyphen overflows, rather than breaking inside the word (also like browsers).
UNIX_ONLY_TEST(SkParagraph_SoftHyphenOnlyBreakMayOverflow, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>(true);
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    SkScalar hyphen = singleLineWidth(fontCollection, "-");
    SkScalar width = singleLineWidth(fontCollection, "abcd") + hyphen / 2;
    auto paragraph = layoutWithSoftHyphens(fontCollection, "abcd~efgh", width);
    checkLines(reporter, paragraph.get(), "abcd-|efgh");
    REPORTER_ASSERT(reporter, paragraph->getLongestLine() > width);
}

// The hyphen only counts if the line ends at the soft hyphen. Here the line would overflow if it
// ended there, but the text after the soft hyphen is narrower than the hyphen, so the line can
// continue past it to the next space.
UNIX_ONLY_TEST(SkParagraph_SoftHyphenBreakNotTakenNeedsNoRoom, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>(true);
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    SkScalar width = singleLineWidth(fontCollection, "xy abcd~i") + 0.3f;
    SkScalar hyphenated = singleLineWidth(fontCollection, "xy abcd-");
    REPORTER_ASSERT(reporter, hyphenated > width + 0.3f, "%g vs %g", hyphenated, width);
    auto paragraph = layoutWithSoftHyphens(fontCollection, "xy abcd~i jklm", width);
    checkLines(reporter, paragraph.get(), "xy abcdi|jklm");
}

// A justified line that breaks at a soft hyphen spreads its text so that the text and the drawn
// hyphen together fill the line, in either paragraph direction.
UNIX_ONLY_TEST(SkParagraph_SoftHyphenJustifiedLineFitsHyphen, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>(true);
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    SkScalar width = singleLineWidth(fontCollection, "xy abcd-") + 15;
    for (TextDirection direction : {TextDirection::kLtr, TextDirection::kRtl}) {
        auto paragraph = layoutWithSoftHyphens(
                fontCollection, "xy abcd~efgh", width, [direction](ParagraphStyle& style) {
                    style.setTextAlign(TextAlign::kJustify);
                    style.setTextDirection(direction);
                });
        checkLines(reporter, paragraph.get(), "xy abcd-|efgh");
        auto& line = static_cast<ParagraphImpl*>(paragraph.get())->lines()[0];
        auto extent = lineRectExtent(paragraph.get(), line, 0, 12);
        REPORTER_ASSERT(
                reporter, SkScalarNearlyEqual(extent.first, 0, EPSILON100), "%g", extent.first);
        REPORTER_ASSERT(reporter,
                        SkScalarNearlyEqual(extent.second, width, EPSILON100),
                        "%g vs %g",
                        extent.second,
                        width);
    }
}

// A justified RTL line with no spaces to spread is right aligned, and its hyphen stays in the line.
UNIX_ONLY_TEST(SkParagraph_SoftHyphenJustifiedRtlLineWithoutSpacesFitsHyphen, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>(true);
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    SkScalar width = singleLineWidth(fontCollection, "abcd-") + 15;
    auto paragraph =
            layoutWithSoftHyphens(fontCollection, "abcd~efgh", width, [](ParagraphStyle& style) {
                style.setTextAlign(TextAlign::kJustify);
                style.setTextDirection(TextDirection::kRtl);
            });
    checkLines(reporter, paragraph.get(), "abcd-|efgh");
    auto& line = static_cast<ParagraphImpl*>(paragraph.get())->lines()[0];
    auto extent = lineRectExtent(paragraph.get(), line, 0, 9);
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(extent.first, width - line.width(), EPSILON100),
                    "%g vs %g",
                    extent.first,
                    width - line.width());
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(extent.second, width, EPSILON100),
                    "%g vs %g",
                    extent.second,
                    width);
}

// An ideographic line breaking at a soft hyphen counts justification patches only between
// visible characters, not between an ideograph and an adjacent soft hyphen
UNIX_ONLY_TEST(SkParagraph_SoftHyphenJustifiedIdeographicFitsHyphen, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>(true);
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    auto layoutCJK = [&](const char* text, SkScalar width, TextDirection direction) {
        ParagraphStyle paragraph_style;
        paragraph_style.turnHintingOff();
        paragraph_style.setApplyRoundingHack(false);
        paragraph_style.setRenderSoftHyphens(true);
        paragraph_style.setTextAlign(TextAlign::kJustify);
        paragraph_style.setTextDirection(direction);
        ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());
        TextStyle text_style;
        text_style.setFontFamilies({SkString("Source Han Serif CN"), SkString("Roboto")});
        text_style.setFontSize(20);
        text_style.setColor(SK_ColorBLACK);
        builder.pushStyle(text_style);
        std::string utf8 = withSoftHyphens(text);
        builder.addText(utf8.c_str(), utf8.size());
        builder.pop();
        auto paragraph = builder.Build();
        paragraph->layout(width);
        return paragraph;
    };

    // "漢字~語文" breaks after "漢字~": only 1 justification patch exists (between 漢 and 字), so
    // the second ideograph 字 must reach the start of the rendered hyphen at (cjkWidth - hyphen)
    SkScalar hyphenWidth = layoutCJK("-", 10000, TextDirection::kLtr)->getMaxIntrinsicWidth();
    SkScalar cjkWidth = layoutCJK("漢字-", 10000, TextDirection::kLtr)->getMaxIntrinsicWidth() + 6;
    auto paragraph = layoutCJK("漢字~語文", cjkWidth, TextDirection::kLtr);
    auto* impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, impl->lines().size() == 2);
    REPORTER_ASSERT(reporter, impl->lines()[0].hyphen() != nullptr);
    auto secondChar = lineRectExtent(paragraph.get(), impl->lines()[0], 1, 2);
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(secondChar.second, cjkWidth - hyphenWidth, EPSILON100),
                    "%g vs %g",
                    secondChar.second,
                    cjkWidth - hyphenWidth);
    auto extent = lineRectExtent(paragraph.get(), impl->lines()[0], 0, 3);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(extent.first, 0, EPSILON100), "%g", extent.first);
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(extent.second, cjkWidth, EPSILON100),
                    "%g vs %g",
                    extent.second,
                    cjkWidth);

    // A single ideograph before a soft hyphen break ("漢~字") has 0 inter-character patches to
    // stretch, so its width must remain its unstretched advance plus the hyphen
    SkScalar oneCharPlusHyphen =
            layoutCJK("漢-", 10000, TextDirection::kLtr)->getMaxIntrinsicWidth();
    SkScalar oneWidth = oneCharPlusHyphen + 6;
    auto single = layoutCJK("漢~字", oneWidth, TextDirection::kLtr);
    auto* singleImpl = static_cast<ParagraphImpl*>(single.get());
    REPORTER_ASSERT(reporter, singleImpl->lines().size() == 2);
    REPORTER_ASSERT(reporter, singleImpl->lines()[0].hyphen() != nullptr);
    REPORTER_ASSERT(
            reporter,
            SkScalarNearlyEqual(singleImpl->lines()[0].width(), oneCharPlusHyphen, EPSILON100),
            "%g vs %g",
            singleImpl->lines()[0].width(),
            oneCharPlusHyphen);

    // An unbroken soft hyphen between two ideographs ("漢~字語~文言") must not insert a second
    // justification patch between 漢 and 字: each character's justified box must match the same
    // line without the unbroken soft hyphen ("漢字語~文言")
    SkScalar threeWidth =
            layoutCJK("漢字語-", 10000, TextDirection::kLtr)->getMaxIntrinsicWidth() + 6;
    auto unbroken = layoutCJK("漢~字語~文言", threeWidth, TextDirection::kLtr);
    auto* unbrokenImpl = static_cast<ParagraphImpl*>(unbroken.get());
    auto reference = layoutCJK("漢字語~文言", threeWidth, TextDirection::kLtr);
    auto* referenceImpl = static_cast<ParagraphImpl*>(reference.get());
    REPORTER_ASSERT(reporter, unbrokenImpl->lines().size() == 2);
    REPORTER_ASSERT(reporter, unbrokenImpl->lines()[0].hyphen() != nullptr);
    auto char1 = lineRectExtent(unbroken.get(), unbrokenImpl->lines()[0], 2, 3);
    auto refChar1 = lineRectExtent(reference.get(), referenceImpl->lines()[0], 1, 2);
    auto char2 = lineRectExtent(unbroken.get(), unbrokenImpl->lines()[0], 3, 4);
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(char1.first, refChar1.first, EPSILON100),
                    "%g vs %g",
                    char1.first,
                    refChar1.first);
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(char1.second, refChar1.second, EPSILON100),
                    "%g vs %g",
                    char1.second,
                    refChar1.second);
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(char2.second, threeWidth - hyphenWidth, EPSILON100),
                    "%g vs %g",
                    char2.second,
                    threeWidth - hyphenWidth);
}

// Laid out at its min intrinsic width, text with soft hyphens doesn't overflow: as in CSS
// min-content, the min intrinsic width includes the hyphen drawn at a soft hyphen break.
UNIX_ONLY_TEST(SkParagraph_SoftHyphenMinIntrinsicWidthIncludesHyphen, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>(true);
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    const char* text = "xy abcd~ef";
    SkScalar minWidth = layoutWithSoftHyphens(fontCollection, text, 1000)->getMinIntrinsicWidth();
    auto paragraph = layoutWithSoftHyphens(fontCollection, text, minWidth);
    checkLines(reporter, paragraph.get(), "xy|abcd-|ef");
    REPORTER_ASSERT(reporter,
                    paragraph->getLongestLine() <= minWidth + EPSILON100,
                    "%g > %g",
                    paragraph->getLongestLine(),
                    minWidth);
}

// The hyphen counts with the width of the string that's drawn, and only where one is drawn.
UNIX_ONLY_TEST(SkParagraph_SoftHyphenMinIntrinsicWidthCountsDrawnHyphen, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>(true);
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    auto minWidth = [&](const char* text, bool render, const char* hyphen) {
        auto configure = [&](ParagraphStyle& style) {
            style.setRenderSoftHyphens(render);
            style.setSoftHyphenString(SkString(hyphen));
        };
        return layoutWithSoftHyphens(fontCollection, text, 1000, configure)->getMinIntrinsicWidth();
    };
    // "abcd" is the widest word, so the hidden case is its width.
    SkScalar hidden = minWidth("xy abcd~ef", false, "-");
    for (const char* hyphen : {"-", "--"}) {
        SkScalar extra = minWidth("xy abcd~ef", true, hyphen) - hidden;
        SkScalar hyphenWidth = singleLineWidth(fontCollection, hyphen);
        REPORTER_ASSERT(reporter,
                        SkScalarNearlyEqual(extra, hyphenWidth, EPSILON100),
                        "%s: %g vs %g",
                        hyphen,
                        extra,
                        hyphenWidth);
    }
    // A soft hyphen at the end of the text never breaks a line, so no hyphen is drawn for it.
    SkScalar atEnd = minWidth("xy abcd~", true, "-");
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(atEnd, minWidth("xy abcd~", false, "-")));
}

// The min intrinsic width doesn't depend on the layout width: a word too long for the line still
// counts with its hyphen.
UNIX_ONLY_TEST(SkParagraph_SoftHyphenMinIntrinsicWidthOfTooLongWord, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>(true);
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    const char* text = "xy abcdefghij~k";
    SkScalar wide = layoutWithSoftHyphens(fontCollection, text, 1000)->getMinIntrinsicWidth();
    SkScalar narrow = layoutWithSoftHyphens(fontCollection, text, 30)->getMinIntrinsicWidth();
    auto hide = [](ParagraphStyle& style) { style.setRenderSoftHyphens(false); };
    SkScalar hidden = layoutWithSoftHyphens(fontCollection, text, 30, hide)->getMinIntrinsicWidth();
    REPORTER_ASSERT(
            reporter, SkScalarNearlyEqual(narrow, wide, EPSILON100), "%g vs %g", narrow, wide);
    REPORTER_ASSERT(reporter, narrow > hidden + 1, "%g vs %g", narrow, hidden);
}

// Verifies that letter-spaced words ending at a soft hyphen include U+00AD's letterSpacing in
// nextWordLength when the word is too long or breaks directly on the U+00AD cluster
UNIX_ONLY_TEST(SkParagraph_SoftHyphenMinIntrinsicWidthLetterSpacing, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>(true);
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    auto buildAndLayout = [&](SkScalar width) {
        ParagraphStyle paragraphStyle;
        paragraphStyle.turnHintingOff();
        paragraphStyle.setApplyRoundingHack(false);
        paragraphStyle.setRenderSoftHyphens(true);
        ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());

        TextStyle textStyle;
        textStyle.setFontFamilies({SkString("Roboto")});
        textStyle.setFontSize(20);
        textStyle.setLetterSpacing(10);
        builder.pushStyle(textStyle);
        std::string utf8 = withSoftHyphens("xy abcdefghij~k");
        builder.addText(utf8.c_str(), utf8.size());
        builder.pop();

        auto paragraph = builder.Build();
        paragraph->layout(width);
        return paragraph;
    };

    SkScalar wide = buildAndLayout(1000)->getMinIntrinsicWidth();
    SkScalar narrow = buildAndLayout(30)->getMinIntrinsicWidth();
    REPORTER_ASSERT(
            reporter, SkScalarNearlyEqual(narrow, wide, EPSILON100), "%g vs %g", narrow, wide);

    // Also test breaking right on the U+00AD cluster itself when letterSpacing > 0 gives it width:
    // "xy abcd" fits, "xy abcd~" exceeds maxWidth by U+00AD's letterSpacing
    ParagraphStyle ps;
    ps.turnHintingOff();
    ps.setApplyRoundingHack(false);
    TextStyle ts;
    ts.setFontFamilies({SkString("Roboto")});
    ts.setFontSize(20);
    ts.setLetterSpacing(10);

    ParagraphBuilderImpl b2(ps, fontCollection, get_unicode());
    b2.pushStyle(ts);
    b2.addText("xy abcd", 7);
    b2.pop();
    auto p2 = b2.Build();
    p2->layout(1000);
    SkScalar xyAbcdWidth = p2->getMaxIntrinsicWidth();

    ps.setRenderSoftHyphens(true);
    auto buildShort = [&]() {
        ParagraphBuilderImpl b(ps, fontCollection, get_unicode());
        b.pushStyle(ts);
        std::string shortUtf8 = withSoftHyphens("xy abcd~ef");
        b.addText(shortUtf8.c_str(), shortUtf8.size());
        b.pop();
        return b.Build();
    };
    auto pWide = buildShort();
    pWide->layout(1000);
    SkScalar expectedMin = pWide->getMinIntrinsicWidth();

    auto pOnShy = buildShort();
    pOnShy->layout(xyAbcdWidth + 5);
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(pOnShy->getMinIntrinsicWidth(), expectedMin, EPSILON100),
                    "%g vs %g",
                    pOnShy->getMinIntrinsicWidth(),
                    expectedMin);
}

// Verifies that fMinIntrinsicWidth counts the hyphenated word width when nextWordLength <= maxWidth
// in lookAhead and in the post-maxLines remainder loop
UNIX_ONLY_TEST(SkParagraph_SoftHyphenMinIntrinsicWidthMaxLines, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>(true);
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    const char* text = "xy za abcd~i";
    SkScalar expectedMin =
            layoutWithSoftHyphens(fontCollection, text, 1000)->getMinIntrinsicWidth();
    SkScalar abcdWidth = singleLineWidth(fontCollection, "abcd");

    // Layout at width between width("abcd") and width("abcd-") with maxLines=2:
    // Line 1 breaks after "xy" and Line 2 breaks after "za" because "za abcd" exceeds maxWidth,
    // while in lookAhead's further loop nextWordLength ("abcd") <= maxWidth, and the post-maxLines
    // remainder loop scans "abcd~i" without splitting at the soft hyphen or adding hyphenWidth
    SkScalar actualMin =
            layoutWithSoftHyphens(fontCollection, text, abcdWidth + 1, [](ParagraphStyle& style) {
                style.setMaxLines(2);
            })->getMinIntrinsicWidth();
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(actualMin, expectedMin, EPSILON100),
                    "%g vs %g",
                    actualMin,
                    expectedMin);

    // Also verify that a word split at a non-breaking space (U+00A0) in lookAhead is not
    // inflated to the unsplit "aa\u00A0bbbb" width in fMinIntrinsicWidth
    SkScalar xxxxAaWidth = singleLineWidth(fontCollection, "xxxx aa");
    SkScalar unsplitNbspWidth = singleLineWidth(fontCollection,
                                                "aa\xC2\xA0"
                                                "bbbb");
    auto nbspParagraph = layoutWithSoftHyphens(fontCollection,
                                               "xxxx aa\xC2\xA0"
                                               "bbbb",
                                               xxxxAaWidth + 2);
    REPORTER_ASSERT(reporter,
                    nbspParagraph->getMinIntrinsicWidth() < unsplitNbspWidth - 1,
                    "%g vs %g",
                    nbspParagraph->getMinIntrinsicWidth(),
                    unsplitNbspWidth);
}

// Verifies that layout(0), which places 1 cluster per line and tracks the max single-cluster
// width in nextWordLength, does not inflate fMinIntrinsicWidth by nextWordHyphenWidth
UNIX_ONLY_TEST(SkParagraph_SoftHyphenMinIntrinsicWidthZeroLayoutWidth, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>(true);
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    const char* text = "xy abcd~ef";
    SkScalar zero = layoutWithSoftHyphens(fontCollection, text, 0)->getMinIntrinsicWidth();
    auto hide = [](ParagraphStyle& style) { style.setRenderSoftHyphens(false); };
    SkScalar hiddenZero =
            layoutWithSoftHyphens(fontCollection, text, 0, hide)->getMinIntrinsicWidth();
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(zero, hiddenZero, EPSILON100),
                    "%g vs %g",
                    zero,
                    hiddenZero);
}

// Verifies that softHyphenWidth shapes with font fallback when the word's font lacks the hyphen
// glyph, matching the hyphen run shaped by TextLine::createSoftHyphen
UNIX_ONLY_TEST(SkParagraph_SoftHyphenFallbackFontWidth, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>(true);
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    auto buildWithFallbackHyphen = [&](SkScalar width) {
        ParagraphStyle paragraphStyle;
        paragraphStyle.turnHintingOff();
        paragraphStyle.setApplyRoundingHack(false);
        paragraphStyle.setRenderSoftHyphens(true);
        // U+4E00 is absent from Roboto and falls back to Source Han Serif CN in shapeEllipsis,
        // whose fullwidth glyph (20px) is wider than Roboto's .notdef glyph
        paragraphStyle.setSoftHyphenString(SkString("\xE4\xB8\x80"));
        ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());

        TextStyle textStyle;
        textStyle.setFontFamilies({SkString("Roboto"), SkString("Source Han Serif CN")});
        textStyle.setFontSize(20);
        builder.pushStyle(textStyle);
        std::string utf8 = withSoftHyphens("xy abcd~efgh");
        builder.addText(utf8.c_str(), utf8.size());
        builder.pop();

        auto paragraph = builder.Build();
        paragraph->layout(width);
        return paragraph;
    };

    SkScalar minWidth = buildWithFallbackHyphen(1000)->getMinIntrinsicWidth();
    auto atMin = buildWithFallbackHyphen(minWidth);
    checkLines(reporter, atMin.get(), "xy|abcd-|efgh");
    REPORTER_ASSERT(reporter,
                    atMin->getLongestLine() <= minWidth + EPSILON100,
                    "%g > %g",
                    atMin->getLongestLine(),
                    minWidth);
}

// A hit on the rendered hyphen is at the end of its line, upstream, as at the end of any other
// line. Downstream would put it at the start of the next line, and getClosestUTF16GlyphInfoAt
// would return the next line's first glyph instead of the soft hyphen.
UNIX_ONLY_TEST(SkParagraph_SoftHyphenHitIsUpstream, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>(true);
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    const size_t softHyphen = 5;
    auto paragraph = layoutWithSoftHyphens(fontCollection, "inter~national", 80);
    checkLines(reporter, paragraph.get(), "inter-|national");
    auto& line = static_cast<ParagraphImpl*>(paragraph.get())->lines()[0];
    SkScalar x = line.offset().fX + (line.widthWithoutEllipsis() + line.width()) / 2;
    SkScalar y = line.offset().fY + line.height() / 2;

    auto position = paragraph->getGlyphPositionAtCoordinate(x, y);
    REPORTER_ASSERT(reporter, position.position == softHyphen + 1, "%d", position.position);
    REPORTER_ASSERT(reporter, position.affinity == Affinity::kUpstream);

    Paragraph::GlyphInfo glyphInfo;
    REPORTER_ASSERT(reporter, paragraph->getClosestUTF16GlyphInfoAt(x, y, &glyphInfo));
    REPORTER_ASSERT(reporter,
                    glyphInfo.fGraphemeClusterTextRange == TextRange(softHyphen, softHyphen + 1),
                    "[%zu, %zu)",
                    glyphInfo.fGraphemeClusterTextRange.start,
                    glyphInfo.fGraphemeClusterTextRange.end);
}

// Likewise for a hit on the ellipsis, which is at the end of the truncated text. Downstream there
// is past the line's text, so getClosestUTF16GlyphInfoAt found no glyph at all.
UNIX_ONLY_TEST(SkParagraph_EllipsisHitIsUpstream, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>(true);
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    SkScalar width = singleLineWidth(fontCollection, "abc...") + 1;
    auto paragraph =
            layoutWithSoftHyphens(fontCollection, "abcdef", width, [](ParagraphStyle& style) {
                style.setMaxLines(1);
                style.setEllipsis(SkString("..."));
            });
    auto& line = static_cast<ParagraphImpl*>(paragraph.get())->lines()[0];
    REPORTER_ASSERT(reporter, line.ellipsis() != nullptr);
    REPORTER_ASSERT(reporter, line.trimmedText() == TextRange(0, 3));
    SkScalar x = line.offset().fX + (line.widthWithoutEllipsis() + line.width()) / 2;
    SkScalar y = line.offset().fY + line.height() / 2;

    auto position = paragraph->getGlyphPositionAtCoordinate(x, y);
    REPORTER_ASSERT(reporter, position.position == 3, "%d", position.position);
    REPORTER_ASSERT(reporter, position.affinity == Affinity::kUpstream);

    Paragraph::GlyphInfo glyphInfo;
    REPORTER_ASSERT(reporter, paragraph->getClosestUTF16GlyphInfoAt(x, y, &glyphInfo));
    REPORTER_ASSERT(reporter,
                    glyphInfo.fGraphemeClusterTextRange == TextRange(2, 3),
                    "[%zu, %zu)",
                    glyphInfo.fGraphemeClusterTextRange.start,
                    glyphInfo.fGraphemeClusterTextRange.end);
}

// Only a soft hyphen's own width changes how a too-long word is measured. Other soft breaks, such
// as the end of the text, still end the word before their cluster, so a word too long for the
// line at the end of the text moves to the next line before it's split, as it always has
// (Flutter's DropdownMenu tests depend on this). With Ahem, every glyph is 10px wide, so each
// 56px line holds 5 glyphs
UNIX_ONLY_TEST(SkParagraph_TooLongWordAtEndOfTextBreaksAsBefore, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    for (bool renderSoftHyphens : {false, true}) {
        ParagraphStyle paragraphStyle;
        paragraphStyle.turnHintingOff();
        paragraphStyle.setApplyRoundingHack(false);
        paragraphStyle.setRenderSoftHyphens(renderSoftHyphens);
        ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());
        TextStyle textStyle;
        textStyle.setFontFamilies({SkString("Ahem")});
        textStyle.setFontSize(10);
        textStyle.setColor(SK_ColorBLACK);
        builder.pushStyle(textStyle);
        const char* text = "n twelve";
        builder.addText(text, strlen(text));
        builder.pop();
        auto paragraph = builder.Build();
        paragraph->layout(56);
        checkLines(reporter, paragraph.get(), "n|twelv|e");
    }
}
