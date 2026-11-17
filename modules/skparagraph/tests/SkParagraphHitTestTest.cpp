/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "modules/skparagraph/include/DartTypes.h"
#include "modules/skparagraph/include/FontCollection.h"
#include "modules/skparagraph/include/Paragraph.h"
#include "modules/skparagraph/include/ParagraphStyle.h"
#include "modules/skparagraph/include/TextStyle.h"
#include "modules/skparagraph/src/ParagraphBuilderImpl.h"
#include "modules/skparagraph/src/ParagraphImpl.h"
#include "modules/skparagraph/src/Run.h"
#include "modules/skparagraph/tests/SkParagraphTestUtils.h"
#include "tests/Test.h"

#include <string.h>
#include <string>
#include <vector>

using namespace skia::textlayout;
using namespace skia::textlayout::test;

UNIX_ONLY_TEST(SkParagraph_GetGlyphPositionAtCoordinateParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    TestCanvas canvas("SkParagraph_GetGlyphPositionAtCoordinateParagraph.png");
    const char* text =
            "12345 67890 12345 67890 12345 67890 12345 67890 12345 67890 12345 "
            "67890 12345";
    const size_t len = strlen(text);

    ParagraphStyle paragraphStyle;
    paragraphStyle.setTextAlign(TextAlign::kLeft);
    paragraphStyle.setMaxLines(10);
    paragraphStyle.turnHintingOff();
    TextStyle textStyle;
    textStyle.setFontFamilies({SkString("Roboto")});
    textStyle.setFontStyle(SkFontStyle(
            SkFontStyle::kNormal_Weight, SkFontStyle::kNormal_Width, SkFontStyle::kUpright_Slant));
    textStyle.setFontSize(50);
    textStyle.setLetterSpacing(1);
    textStyle.setWordSpacing(5);
    textStyle.setHeight(1);
    textStyle.setColor(SK_ColorBLACK);

    ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());
    builder.pushStyle(textStyle);
    builder.addText(text, len);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(550);
    paragraph->paint(canvas.get(), 0, 0);

    // Tests for getGlyphPositionAtCoordinate()
    // NOTE: resulting values can be a few off from their respective positions in
    // the original text because the final trailing whitespaces are sometimes not
    // drawn (namely, when using "justify" alignment) and therefore are not active
    // glyphs.
    REPORTER_ASSERT(reporter,
                    paragraph->getGlyphPositionAtCoordinate(-10000, -10000).position == 0);
    REPORTER_ASSERT(reporter, paragraph->getGlyphPositionAtCoordinate(-1, -1).position == 0);
    REPORTER_ASSERT(reporter, paragraph->getGlyphPositionAtCoordinate(0, 0).position == 0);
    REPORTER_ASSERT(reporter, paragraph->getGlyphPositionAtCoordinate(3, 3).position == 0);
    REPORTER_ASSERT(reporter, paragraph->getGlyphPositionAtCoordinate(35, 1).position == 1);
    REPORTER_ASSERT(reporter, paragraph->getGlyphPositionAtCoordinate(300, 2).position == 11);
    REPORTER_ASSERT(reporter, paragraph->getGlyphPositionAtCoordinate(301, 2.2f).position == 11);
    REPORTER_ASSERT(reporter, paragraph->getGlyphPositionAtCoordinate(302, 2.6f).position == 11);
    REPORTER_ASSERT(reporter, paragraph->getGlyphPositionAtCoordinate(301, 2.1f).position == 11);
    REPORTER_ASSERT(reporter, paragraph->getGlyphPositionAtCoordinate(100000, 20).position == 18);
    REPORTER_ASSERT(reporter, paragraph->getGlyphPositionAtCoordinate(450, 20).position == 16);
    REPORTER_ASSERT(reporter, paragraph->getGlyphPositionAtCoordinate(100000, 90).position == 36);
    REPORTER_ASSERT(reporter, paragraph->getGlyphPositionAtCoordinate(-100000, 90).position == 18);
    REPORTER_ASSERT(reporter, paragraph->getGlyphPositionAtCoordinate(20, -80).position == 1);
    REPORTER_ASSERT(reporter, paragraph->getGlyphPositionAtCoordinate(1, 90).position == 18);
    REPORTER_ASSERT(reporter, paragraph->getGlyphPositionAtCoordinate(1, 170).position == 36);
    REPORTER_ASSERT(reporter, paragraph->getGlyphPositionAtCoordinate(10000, 180).position == 72);
    REPORTER_ASSERT(reporter, paragraph->getGlyphPositionAtCoordinate(70, 180).position == 56);
    REPORTER_ASSERT(reporter, paragraph->getGlyphPositionAtCoordinate(1, 270).position == 72);
    REPORTER_ASSERT(reporter, paragraph->getGlyphPositionAtCoordinate(35, 90).position == 19);
    REPORTER_ASSERT(reporter, paragraph->getGlyphPositionAtCoordinate(10000, 10000).position == 77);
    REPORTER_ASSERT(reporter, paragraph->getGlyphPositionAtCoordinate(85, 10000).position == 75);
}

// Checked: DIFF (small in numbers)
UNIX_ONLY_TEST(SkParagraph_GetWordBoundaryParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    TestCanvas canvas("SkParagraph_GetWordBoundaryParagraph.png");
    const char* text =
            "12345  67890 12345 67890 12345 67890 12345 "
            "67890 12345 67890 12345 67890 12345";
    const size_t len = strlen(text);
    ParagraphStyle paragraphStyle;
    paragraphStyle.setTextAlign(TextAlign::kLeft);
    paragraphStyle.setMaxLines(10);
    paragraphStyle.turnHintingOff();
    TextStyle textStyle;
    textStyle.setFontFamilies({SkString("Roboto")});
    textStyle.setFontSize(52);
    textStyle.setLetterSpacing(1.19039f);
    textStyle.setWordSpacing(5);
    textStyle.setHeight(1.5);
    textStyle.setHeightOverride(true);
    textStyle.setColor(SK_ColorBLACK);

    ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());
    builder.pushStyle(textStyle);
    builder.addText(text, len);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(550);
    paragraph->paint(canvas.get(), 0, 0);

    REPORTER_ASSERT(reporter, paragraph->getWordBoundary(0) == SkRange<size_t>(0, 5));
    REPORTER_ASSERT(reporter, paragraph->getWordBoundary(1) == SkRange<size_t>(0, 5));
    REPORTER_ASSERT(reporter, paragraph->getWordBoundary(2) == SkRange<size_t>(0, 5));
    REPORTER_ASSERT(reporter, paragraph->getWordBoundary(3) == SkRange<size_t>(0, 5));
    REPORTER_ASSERT(reporter, paragraph->getWordBoundary(4) == SkRange<size_t>(0, 5));
    auto boxes = paragraph->getRectsForRange(5, 6, RectHeightStyle::kMax, RectWidthStyle::kTight);
    canvas.drawLines(SK_ColorRED, boxes);

    REPORTER_ASSERT(reporter, paragraph->getWordBoundary(5) == SkRange<size_t>(5, 7));
    boxes = paragraph->getRectsForRange(6, 7, RectHeightStyle::kMax, RectWidthStyle::kTight);
    canvas.drawLines(SK_ColorRED, boxes);

    REPORTER_ASSERT(reporter, paragraph->getWordBoundary(6) == SkRange<size_t>(5, 7));
    boxes = paragraph->getRectsForRange(7, 8, RectHeightStyle::kMax, RectWidthStyle::kTight);
    canvas.drawLines(SK_ColorRED, boxes);

    REPORTER_ASSERT(reporter, paragraph->getWordBoundary(7) == SkRange<size_t>(7, 12));
    REPORTER_ASSERT(reporter, paragraph->getWordBoundary(8) == SkRange<size_t>(7, 12));
    REPORTER_ASSERT(reporter, paragraph->getWordBoundary(9) == SkRange<size_t>(7, 12));
    REPORTER_ASSERT(reporter, paragraph->getWordBoundary(10) == SkRange<size_t>(7, 12));
    REPORTER_ASSERT(reporter, paragraph->getWordBoundary(11) == SkRange<size_t>(7, 12));
    REPORTER_ASSERT(reporter, paragraph->getWordBoundary(12) == SkRange<size_t>(12, 13));
    REPORTER_ASSERT(reporter, paragraph->getWordBoundary(13) == SkRange<size_t>(13, 18));
    REPORTER_ASSERT(reporter, paragraph->getWordBoundary(30) == SkRange<size_t>(30, 31));

    boxes = paragraph->getRectsForRange(12, 13, RectHeightStyle::kMax, RectWidthStyle::kTight);
    canvas.drawLines(SK_ColorRED, boxes);
    boxes = paragraph->getRectsForRange(13, 14, RectHeightStyle::kMax, RectWidthStyle::kTight);
    canvas.drawLines(SK_ColorRED, boxes);
    boxes = paragraph->getRectsForRange(18, 19, RectHeightStyle::kMax, RectWidthStyle::kTight);
    canvas.drawLines(SK_ColorRED, boxes);
    boxes = paragraph->getRectsForRange(19, 20, RectHeightStyle::kMax, RectWidthStyle::kTight);
    canvas.drawLines(SK_ColorRED, boxes);
    boxes = paragraph->getRectsForRange(24, 25, RectHeightStyle::kMax, RectWidthStyle::kTight);
    canvas.drawLines(SK_ColorRED, boxes);
    boxes = paragraph->getRectsForRange(25, 26, RectHeightStyle::kMax, RectWidthStyle::kTight);
    canvas.drawLines(SK_ColorRED, boxes);
    boxes = paragraph->getRectsForRange(30, 31, RectHeightStyle::kMax, RectWidthStyle::kTight);
    canvas.drawLines(SK_ColorRED, boxes);
    boxes = paragraph->getRectsForRange(31, 32, RectHeightStyle::kMax, RectWidthStyle::kTight);
    canvas.drawLines(SK_ColorRED, boxes);

    auto outLen = static_cast<ParagraphImpl*>(paragraph.get())->text().size();
    REPORTER_ASSERT(reporter,
                    paragraph->getWordBoundary(outLen - 1) == SkRange<size_t>(outLen - 5, outLen));
}

UNIX_ONLY_TEST(SkParagraph_PositionInsideEmoji, reporter) {

    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    TestCanvas canvas("SkParagraph_PositionInsideEmoji.png");

    std::u16string text =
            u"\U0001f469\u200D\U0001f469\u200D\U0001f467\u200D\U0001f467\U0001f469\U0001f469"
            u"\U0001f467\U0001f467";

    ParagraphStyle paragraph_style;
    TextStyle text_style;
    text_style.setColor(SK_ColorBLACK);
    text_style.setFontFamilies({SkString("Noto Color Emoji")});
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());
    builder.pushStyle(text_style);
    builder.addText(text);

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);
    paragraph->paint(canvas.get(), 0, 0);

    // UTF8       UTF16
    // 4          [0:2)
    // 3 + 4      [2:5)
    // 3 + 4      [5:8)
    // 3 + 4      [8:11)
    // 4          [11:13)
    // 4          [13:15)
    // 4          [15:17)
    // 4          [17:19)

    auto family = paragraph->getRectsForRange(
            0, 11, RectHeightStyle::kTight, RectWidthStyle::kTight);  // 00.0000000 + 17.4699993
    auto face01 = paragraph->getRectsForRange(
            11, 13, RectHeightStyle::kTight, RectWidthStyle::kTight);  // 17.4699993 + 17.4699993
    auto face02 = paragraph->getRectsForRange(
            13, 15, RectHeightStyle::kTight, RectWidthStyle::kTight);  // 34.9399986 + 17.4699993
    auto face03 = paragraph->getRectsForRange(
            15, 17, RectHeightStyle::kTight, RectWidthStyle::kTight);  // 52.4099998 + 17.4699993
    auto face04 = paragraph->getRectsForRange(
            17, 19, RectHeightStyle::kTight, RectWidthStyle::kTight);  // 69.8799973 + 17.4699993

    int32_t words[] = {11, 13, 15, 17, 19, 21};
    auto j = 0;
    for (auto i : words) {
        auto rects =
                paragraph->getRectsForRange(j, i, RectHeightStyle::kTight, RectWidthStyle::kTight);
        if (rects.empty()) {
            continue;
        }
        auto X = rects[0].rect.centerX();
        auto Y = rects[0].rect.centerY();
        auto res1 = paragraph->getGlyphPositionAtCoordinate(X - 5, Y);
        auto res2 = paragraph->getGlyphPositionAtCoordinate(X + 5, Y);
        REPORTER_ASSERT(reporter, i == res2.position && res1.position == j);
        j = i;
    }
}

UNIX_ONLY_TEST(SkParagraph_Utf16Indexes, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    TestCanvas canvas("SkParagraph_Utf16Indexes.png");
    canvas.get()->translate(100, 100);

    TextStyle text_style;
    text_style.setColor(SK_ColorBLACK);
    text_style.setFontFamilies({SkString("Ahem")});
    text_style.setFontSize(10.0f);
    ParagraphStyle paragraph_style;
    paragraph_style.setTextStyle(text_style);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());
    builder.pushStyle(text_style);
    builder.addText("áéíóú\nxxxx");
    auto paragraph = builder.Build();
    paragraph->layout(60.0f);
    paragraph->paint(canvas.get(), 0, 0);
    std::vector<LineMetrics> lm;
    paragraph->getLineMetrics(lm);
    REPORTER_ASSERT(reporter, lm.size() == 2);
    if (lm.size() >= 2) {
        REPORTER_ASSERT(reporter,
                        lm[0].fEndExcludingWhitespaces == 05 && lm[0].fEndIndex == 05 &&
                                lm[0].fEndIncludingNewline == 06);
        REPORTER_ASSERT(reporter,
                        lm[1].fEndExcludingWhitespaces == 10 && lm[1].fEndIndex == 10 &&
                                lm[1].fEndIncludingNewline == 10);
    }
}

static bool has_empty_typeface(SkFont f) {
    SkTypeface* face = f.getTypeface();
    if (!face) {
        return true;  // Should be impossible, but just in case...
    }
    return face->countGlyphs() == 0 && face->getBounds().isEmpty();
}

UNIX_ONLY_TEST(SkParagraph_TextEditingFunctionality, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    TestCanvas canvas("SkParagraph_TextEditingFunctionality.png");
    const char* text =
            "This is a very long sentence to test if the text will properly wrap "
            "around and go to the next line. Sometimes, short sentence. Longer "
            "sentences are okay too because they are nessecary. Very short. "
            "This is a very long sentence to test if the text will properly wrap "
            "around and go to the next line. Sometimes, short sentence. Longer "
            "sentences are okay too because they are nessecary. Very short. ";

    const size_t len = strlen(text);

    ParagraphStyle paragraph_style;
    paragraph_style.setEllipsis(u"\u2026");
    paragraph_style.setMaxLines(3);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());
    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setFontSize(20);
    text_style.setColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text, len);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);
    paragraph->paint(canvas.get(), 0, 0);

    auto lineNumber = paragraph->getLineNumberAt(0);
    REPORTER_ASSERT(reporter, lineNumber == 0);
    lineNumber = paragraph->getLineNumberAt(len / 2);
    REPORTER_ASSERT(reporter, lineNumber == 1);
    lineNumber = paragraph->getLineNumberAt(len - 1);
    REPORTER_ASSERT(reporter, lineNumber == -1);
    lineNumber = paragraph->getLineNumberAt(len);
    REPORTER_ASSERT(reporter, lineNumber == -1);
    lineNumber = paragraph->getLineNumberAt(len + 10);
    REPORTER_ASSERT(reporter, lineNumber == -1);

    LineMetrics lineMetrics;
    auto foundMetrics = paragraph->getLineMetricsAt(0, &lineMetrics);
    REPORTER_ASSERT(reporter, foundMetrics && lineMetrics.fLineNumber == 0);
    foundMetrics = paragraph->getLineMetricsAt(1, &lineMetrics);
    REPORTER_ASSERT(reporter, foundMetrics && lineMetrics.fLineNumber == 1);
    foundMetrics = paragraph->getLineMetricsAt(3, &lineMetrics);
    REPORTER_ASSERT(reporter, !foundMetrics);
    foundMetrics = paragraph->getLineMetricsAt(10, &lineMetrics);
    REPORTER_ASSERT(reporter, !foundMetrics);

    std::vector<LineMetrics> metrics;
    paragraph->getLineMetrics(metrics);
    auto actualText = paragraph->getActualTextRange(0, false);
    REPORTER_ASSERT(reporter, actualText.end == metrics[0].fEndExcludingWhitespaces);
    actualText = paragraph->getActualTextRange(1, false);
    REPORTER_ASSERT(reporter, actualText.end == metrics[1].fEndExcludingWhitespaces);
    actualText = paragraph->getActualTextRange(2, false);
    REPORTER_ASSERT(reporter, actualText.end == metrics[2].fEndExcludingWhitespaces);

    Paragraph::GlyphClusterInfo glyphInfo;
    auto foundCluster = paragraph->getGlyphClusterAt(0, &glyphInfo);
    REPORTER_ASSERT(reporter, foundCluster && glyphInfo.fClusterTextRange.start == 0);
    foundCluster = paragraph->getGlyphClusterAt(len / 2, &glyphInfo);
    REPORTER_ASSERT(reporter, foundCluster && glyphInfo.fClusterTextRange.start == len / 2);
    foundCluster = paragraph->getGlyphClusterAt(len, &glyphInfo);
    REPORTER_ASSERT(reporter, !foundCluster);

    auto foundClosest = paragraph->getClosestGlyphClusterAt(0, 10, &glyphInfo);
    REPORTER_ASSERT(reporter,
                    foundClosest && glyphInfo.fClusterTextRange.start == 0 &&
                            glyphInfo.fClusterTextRange.end == 1);
    foundClosest = paragraph->getClosestGlyphClusterAt(TestCanvasWidth / 2, 20, &glyphInfo);
    REPORTER_ASSERT(reporter,
                    foundClosest && glyphInfo.fClusterTextRange.start == 61 &&
                            glyphInfo.fClusterTextRange.end == 62);
    foundClosest = paragraph->getClosestGlyphClusterAt(TestCanvasWidth + 10, 30, &glyphInfo);

    REPORTER_ASSERT(reporter,
                    foundClosest && glyphInfo.fClusterTextRange.start == 228 &&
                            glyphInfo.fClusterTextRange.end == 229);

    auto font = paragraph->getFontAt(10);
    REPORTER_ASSERT(reporter, !has_empty_typeface(font));
    SkString fontFamily;
    font.getTypeface()->getFamilyName(&fontFamily);
    REPORTER_ASSERT(reporter, fontFamily.equals("Roboto"));

    auto fonts = paragraph->getFonts();
    REPORTER_ASSERT(reporter, fonts.size() == 1);
    REPORTER_ASSERT(reporter, fonts[0].fTextRange.start == 0 && fonts[0].fTextRange.end == len);
    REPORTER_ASSERT(reporter, !has_empty_typeface(fonts[0].fFont));
    font.getTypeface()->getFamilyName(&fontFamily);
    REPORTER_ASSERT(reporter, fontFamily.equals("Roboto"));
}

UNIX_ONLY_TEST(SkParagraph_API_USES_UTF16, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    // Each of these characters consists of 3 UTF-8 code points, or 1 UTF-16
    // code point.
    const char* text = "一丁丂七";
    const size_t len = strlen(text);

    ParagraphStyle paragraph_style;
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());
    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setFontSize(20);
    text_style.setColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text, len);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);

    REPORTER_ASSERT(reporter, !has_empty_typeface(paragraph->getFontAtUTF16Offset(0)));
    REPORTER_ASSERT(reporter, has_empty_typeface(paragraph->getFontAtUTF16Offset(4)));

    REPORTER_ASSERT(reporter, paragraph->getGlyphInfoAtUTF16Offset(0, nullptr));
    REPORTER_ASSERT(reporter, !paragraph->getGlyphInfoAtUTF16Offset(4, nullptr));
    // Verify the output also uses UTF-16
    Paragraph::GlyphInfo glyphInfo;
    REPORTER_ASSERT(reporter, paragraph->getGlyphInfoAtUTF16Offset(3, &glyphInfo));
    REPORTER_ASSERT(reporter, glyphInfo.fGraphemeClusterTextRange.start == 3);
    REPORTER_ASSERT(reporter, glyphInfo.fGraphemeClusterTextRange.end == 4);

    REPORTER_ASSERT(reporter, paragraph->getLineNumberAtUTF16Offset(0) == 0);
    REPORTER_ASSERT(reporter, paragraph->getLineNumberAtUTF16Offset(4) == -1);

    // The last logical character ("七") is considered hit.
    REPORTER_ASSERT(reporter, paragraph->getClosestUTF16GlyphInfoAt(999.0f, 999.0f, &glyphInfo));
    REPORTER_ASSERT(reporter, glyphInfo.fGraphemeClusterTextRange.start == 3);
    REPORTER_ASSERT(reporter, glyphInfo.fGraphemeClusterTextRange.end == 4);

    // The first logical character ("一") is considered hit.
    REPORTER_ASSERT(reporter, paragraph->getClosestUTF16GlyphInfoAt(-999.0f, 0.0f, &glyphInfo));
    REPORTER_ASSERT(reporter, glyphInfo.fGraphemeClusterTextRange.start == 0);
    REPORTER_ASSERT(reporter, glyphInfo.fGraphemeClusterTextRange.end == 1);
}

UNIX_ONLY_TEST(SkParagraph_Empty_Paragraph_Metrics, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    const char* text = "";

    ParagraphStyle paragraph_style;
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());
    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setFontSize(20);
    text_style.setColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text, 0);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);

    REPORTER_ASSERT(reporter, has_empty_typeface(paragraph->getFontAt(0)));
    REPORTER_ASSERT(reporter, !paragraph->getGlyphClusterAt(0, nullptr));
    REPORTER_ASSERT(reporter, paragraph->getLineNumberAt(0) == -1);
    REPORTER_ASSERT(reporter, !paragraph->getClosestGlyphClusterAt(10.0, 5.0, nullptr));
}
