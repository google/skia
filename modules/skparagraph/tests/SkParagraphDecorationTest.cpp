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
#include "include/core/SkPath.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSpan.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "modules/skparagraph/include/DartTypes.h"
#include "modules/skparagraph/include/FontCollection.h"
#include "modules/skparagraph/include/Paragraph.h"
#include "modules/skparagraph/include/ParagraphPainter.h"
#include "modules/skparagraph/include/ParagraphStyle.h"
#include "modules/skparagraph/include/TextShadow.h"
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
#include <vector>

using namespace skia::textlayout;
using namespace skia::textlayout::test;

UNIX_ONLY_TEST(SkParagraph_SimpleRedParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    const char* text = "I am RED";
    const size_t len = strlen(text);

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setColor(SK_ColorRED);
    builder.pushStyle(text_style);
    builder.addText(text, len);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);
    REPORTER_ASSERT(reporter, paragraph->unresolvedGlyphs() == 0);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, impl->runs().size() == 1);
    REPORTER_ASSERT(reporter, impl->styles().size() == 1);  // paragraph style does not count
    REPORTER_ASSERT(reporter, impl->styles()[0].fStyle.equals(text_style));

    size_t index = 0;
    for (auto& line : impl->lines()) {
        line.scanStyles(StyleType::kDecorations,
                        [reporter, &index](TextRange textRange,
                                           const TextStyle& style,
                                           const TextLine::ClipContext& context) {
                            REPORTER_ASSERT(reporter, index == 0);
                            REPORTER_ASSERT(reporter, style.getColor() == SK_ColorRED);
                            ++index;
                            return true;
                        });
    }
}

// Checked: DIFF+ (Space between 1 & 2 style blocks)
UNIX_ONLY_TEST(SkParagraph_RainbowParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    TestCanvas canvas("SkParagraph_RainbowParagraph.png");
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    const char* text1 = "Red Roboto";               // [0:10)
    const char* text2 = "big Greeen Default";       // [10:28)
    const char* text3 = "Defcolor Homemade Apple";  // [28:51)
    const char* text4 = "Small Blue Roboto";        // [51:68)
    const char* text41 = "Small Blue ";
    const char* text5 =
            "Continue Last Style With lots of words to check if it overlaps "
            "properly or not";  // [68:)
    const char* text42 =
            "Roboto"
            "Continue Last Style With lots of words to check if it overlaps "
            "properly or not";

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    paragraph_style.setTextAlign(TextAlign::kLeft);
    paragraph_style.setMaxLines(2);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());

    TextStyle text_style1;
    text_style1.setFontFamilies({SkString("Roboto")});

    text_style1.setColor(SK_ColorRED);
    builder.pushStyle(text_style1);
    builder.addText(text1, strlen(text1));

    TextStyle text_style2;
    text_style2.setFontFamilies({SkString("Roboto")});
    text_style2.setFontSize(50);
    text_style2.setFontStyle(SkFontStyle(
            SkFontStyle::kMedium_Weight, SkFontStyle::kNormal_Width, SkFontStyle::kUpright_Slant));
    text_style2.setLetterSpacing(10);
    text_style2.setDecorationColor(SK_ColorBLACK);
    text_style2.setDecoration((TextDecoration)(TextDecoration::kUnderline |
                                               TextDecoration::kOverline |
                                               TextDecoration::kLineThrough));
    text_style2.setWordSpacing(30);
    text_style2.setColor(SK_ColorGREEN);
    builder.pushStyle(text_style2);
    builder.addText(text2, strlen(text2));

    TextStyle text_style3;
    text_style3.setFontFamilies({SkString("Homemade Apple")});
    text_style3.setColor(SK_ColorBLACK);
    builder.pushStyle(text_style3);
    builder.addText(text3, strlen(text3));

    TextStyle text_style4;
    text_style4.setFontFamilies({SkString("Roboto")});
    text_style4.setFontSize(14);
    text_style4.setDecorationColor(SK_ColorBLACK);
    text_style4.setDecoration((TextDecoration)(TextDecoration::kUnderline |
                                               TextDecoration::kOverline |
                                               TextDecoration::kLineThrough));
    text_style4.setColor(SK_ColorBLUE);
    builder.pushStyle(text_style4);
    builder.addText(text4, strlen(text4));

    builder.addText(text5, strlen(text5));
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(1000);
    paragraph->paint(canvas.get(), 0, 0);

    REPORTER_ASSERT(reporter, paragraph->unresolvedGlyphs() == 0);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, impl->runs().size() == 4);
    REPORTER_ASSERT(reporter, impl->styles().size() == 4);
    REPORTER_ASSERT(reporter, impl->lines().size() == 2);

    auto rects = paragraph->getRectsForRange(
            0, impl->text().size(), RectHeightStyle::kMax, RectWidthStyle::kTight);
    canvas.drawRects(SK_ColorMAGENTA, rects);

    size_t index = 0;
    impl->lines()[0].scanStyles(
            StyleType::kAllAttributes,
            [&](TextRange textRange, const TextStyle& style, const TextLine::ClipContext& context) {
                switch (index) {
                    case 0:
                        REPORTER_ASSERT(reporter, style.equals(text_style1));
                        REPORTER_ASSERT(reporter, equal(impl->text().data(), textRange, text1));
                        break;
                    case 1:
                        REPORTER_ASSERT(reporter, style.equals(text_style2));
                        REPORTER_ASSERT(reporter, equal(impl->text().data(), textRange, text2));
                        break;
                    case 2:
                        REPORTER_ASSERT(reporter, style.equals(text_style3));
                        REPORTER_ASSERT(reporter, equal(impl->text().data(), textRange, text3));
                        break;
                    case 3:
                        REPORTER_ASSERT(reporter, style.equals(text_style4));
                        REPORTER_ASSERT(reporter, equal(impl->text().data(), textRange, text41));
                        break;
                    default:
                        REPORTER_ASSERT(reporter, false);
                        break;
                }
                ++index;
                return true;
            });
    impl->lines()[1].scanStyles(
            StyleType::kAllAttributes,
            [&](TextRange textRange, const TextStyle& style, const TextLine::ClipContext& context) {
                switch (index) {
                    case 4:
                        REPORTER_ASSERT(reporter, style.equals(text_style4));
                        REPORTER_ASSERT(reporter, equal(impl->text().data(), textRange, text42));
                        break;
                    default:
                        REPORTER_ASSERT(reporter, false);
                        break;
                }
                ++index;
                return true;
            });
    REPORTER_ASSERT(reporter, index == 5);
}

UNIX_ONLY_TEST(SkParagraph_BoldParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    TestCanvas canvas("SkParagraph_BoldParagraph.png");
    const char* text = "This is Red max bold text!";
    const size_t len = strlen(text);

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setColor(SK_ColorRED);
    text_style.setFontSize(60);
    text_style.setLetterSpacing(0);
    text_style.setFontStyle(SkFontStyle(
            SkFontStyle::kBlack_Weight, SkFontStyle::kNormal_Width, SkFontStyle::kUpright_Slant));
    builder.pushStyle(text_style);
    builder.addText(text, len);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(VeryLongCanvasWidth);
    paragraph->paint(canvas.get(), 10.0, 60.0);

    REPORTER_ASSERT(reporter, paragraph->unresolvedGlyphs() == 0);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());

    REPORTER_ASSERT(reporter, impl->runs().size() == 1);
    REPORTER_ASSERT(reporter, impl->styles().size() == 1);
    REPORTER_ASSERT(reporter, impl->lines().size() == 1);

    size_t index = 0;
    impl->lines()[0].scanStyles(
            StyleType::kAllAttributes,
            [&](TextRange textRange, const TextStyle& style, const TextLine::ClipContext& context) {
                REPORTER_ASSERT(reporter, style.equals(text_style));
                REPORTER_ASSERT(reporter, equal(impl->text().data(), textRange, text));
                ++index;
                return true;
            });
    REPORTER_ASSERT(reporter, index == 1);
}

UNIX_ONLY_TEST(SkParagraph_DecorationsParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    TestCanvas canvas("SkParagraph_DecorationsParagraph.png");
    const char* text1 = "This text should be";
    const char* text2 = " decorated even when";
    const char* text3 = " wrapped around to";
    const char* text4 = " the next line.";
    const char* text5 = " Otherwise, bad things happen.";

    ParagraphStyle paragraph_style;
    paragraph_style.setMaxLines(14);
    paragraph_style.setTextAlign(TextAlign::kLeft);
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setFontSize(26);
    text_style.setLetterSpacing(0);
    text_style.setWordSpacing(5);
    text_style.setColor(SK_ColorBLACK);
    text_style.setHeight(2);
    text_style.setDecoration(TextDecoration::kUnderline);
    text_style.setDecorationColor(SK_ColorBLACK);
    text_style.setDecoration((TextDecoration)(TextDecoration::kUnderline |
                                              TextDecoration::kOverline |
                                              TextDecoration::kLineThrough));
    text_style.setDecorationStyle(TextDecorationStyle::kSolid);
    text_style.setDecorationColor(SK_ColorBLACK);
    text_style.setDecorationThicknessMultiplier(2.0);
    builder.pushStyle(text_style);
    builder.addText(text1, strlen(text1));

    text_style.setDecorationStyle(TextDecorationStyle::kDouble);
    text_style.setDecorationColor(SK_ColorBLUE);
    text_style.setDecorationThicknessMultiplier(1.0);
    builder.pushStyle(text_style);
    builder.addText(text2, strlen(text2));

    text_style.setDecorationStyle(TextDecorationStyle::kDotted);
    text_style.setDecorationColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text3, strlen(text3));

    text_style.setDecorationStyle(TextDecorationStyle::kDashed);
    text_style.setDecorationColor(SK_ColorBLACK);
    text_style.setDecorationThicknessMultiplier(3.0);
    builder.pushStyle(text_style);
    builder.addText(text4, strlen(text4));

    text_style.setDecorationStyle(TextDecorationStyle::kWavy);
    text_style.setDecorationColor(SK_ColorRED);
    text_style.setDecorationThicknessMultiplier(1.0);
    builder.pushStyle(text_style);
    builder.addText(text5, strlen(text5));
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth - 100);
    paragraph->paint(canvas.get(), 0, 0);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());

    size_t index = 0;
    for (auto& line : impl->lines()) {
        line.scanStyles(
                StyleType::kDecorations,
                [&](TextRange textRange,
                    const TextStyle& style,
                    const TextLine::ClipContext& context) {
                    auto decoration = (TextDecoration)(TextDecoration::kUnderline |
                                                       TextDecoration::kOverline |
                                                       TextDecoration::kLineThrough);
                    REPORTER_ASSERT(reporter, style.getDecorationType() == decoration);
                    switch (index) {
                        case 0:
                            REPORTER_ASSERT(
                                    reporter,
                                    style.getDecorationStyle() == TextDecorationStyle::kSolid);
                            REPORTER_ASSERT(reporter, style.getDecorationColor() == SK_ColorBLACK);
                            REPORTER_ASSERT(reporter,
                                            style.getDecorationThicknessMultiplier() == 2.0);
                            break;
                        case 1:  // The style appears on 2 lines so it has 2 pieces
                            REPORTER_ASSERT(
                                    reporter,
                                    style.getDecorationStyle() == TextDecorationStyle::kDouble);
                            REPORTER_ASSERT(reporter, style.getDecorationColor() == SK_ColorBLUE);
                            REPORTER_ASSERT(reporter,
                                            style.getDecorationThicknessMultiplier() == 1.0);
                            break;
                        case 2:
                            REPORTER_ASSERT(
                                    reporter,
                                    style.getDecorationStyle() == TextDecorationStyle::kDotted);
                            REPORTER_ASSERT(reporter, style.getDecorationColor() == SK_ColorBLACK);
                            REPORTER_ASSERT(reporter,
                                            style.getDecorationThicknessMultiplier() == 1.0);
                            break;
                        case 3:
                        case 4:
                            REPORTER_ASSERT(
                                    reporter,
                                    style.getDecorationStyle() == TextDecorationStyle::kDashed);
                            REPORTER_ASSERT(reporter, style.getDecorationColor() == SK_ColorBLACK);
                            REPORTER_ASSERT(reporter,
                                            style.getDecorationThicknessMultiplier() == 3.0);
                            break;
                        case 5:
                            REPORTER_ASSERT(
                                    reporter,
                                    style.getDecorationStyle() == TextDecorationStyle::kWavy);
                            REPORTER_ASSERT(reporter, style.getDecorationColor() == SK_ColorRED);
                            REPORTER_ASSERT(reporter,
                                            style.getDecorationThicknessMultiplier() == 1.0);
                            break;
                        default:
                            REPORTER_ASSERT(reporter, false);
                            break;
                    }
                    ++index;
                    return true;
                });
    }
}

namespace {
// Records glyphs and decoration paths in absolute coordinates so the test can check where
// the drawn underline is interrupted against where the drawn glyphs actually are
class GapsRecordingPainter final : public ParagraphPainter {
public:
    struct Blob {
        sk_sp<SkTextBlob> fBlob;
        SkPoint fOrigin;
    };
    struct Path {
        SkPath fPath;
        SkPoint fOrigin;
        SkPaint fPaint;
    };
    std::vector<Blob> fBlobs;
    std::vector<Path> fPaths;
    std::vector<SkRect> fFilledRects;

    void drawTextBlob(const sk_sp<SkTextBlob>& blob,
                      SkScalar x,
                      SkScalar y,
                      const SkPaintOrID&) override {
        fBlobs.push_back({blob, fOrigin + SkPoint::Make(x, y)});
    }
    void drawTextShadow(const sk_sp<SkTextBlob>&, SkScalar, SkScalar, SkColor, SkScalar) override {}
    void drawRect(const SkRect&, const SkPaintOrID&) override {}
    void drawFilledRect(const SkRect& rect, const DecorationStyle&) override {
        fFilledRects.push_back(rect.makeOffset(fOrigin));
    }
    void drawPath(const SkPath& path, const DecorationStyle& decorStyle) override {
        fPaths.push_back({path, fOrigin, decorStyle.skPaint()});
    }
    void drawLine(SkScalar, SkScalar, SkScalar, SkScalar, const DecorationStyle&) override {}
    void clipRect(const SkRect&) override {}
    void translate(SkScalar dx, SkScalar dy) override { fOrigin += SkPoint::Make(dx, dy); }
    void save() override { fStack.push_back(fOrigin); }
    void restore() override {
        fOrigin = fStack.back();
        fStack.pop_back();
    }

    std::unique_ptr<Paragraph> paintUnderlineGaps(const char* text,
                                                  SkScalar baselineShift = 0.0f,
                                                  SkScalar height = 0.0f,
                                                  bool halfLeading = false) {
        auto fontCollection = sk_make_sp<FontCollection>();
        fontCollection->setDefaultFontManager(ToolUtils::TestFontMgr());
        ParagraphStyle paragraphStyle;
        TextStyle textStyle;
        textStyle.setFontSize(50.0f);
        textStyle.setColor(SK_ColorBLACK);
        textStyle.setDecoration(TextDecoration::kUnderline);
        textStyle.setDecorationMode(TextDecorationMode::kGaps);
        textStyle.setDecorationStyle(TextDecorationStyle::kSolid);
        textStyle.setBaselineShift(baselineShift);
        textStyle.setHeightOverride(height != 0);
        textStyle.setHeight(height);
        textStyle.setHalfLeading(halfLeading);

        ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());
        builder.pushStyle(textStyle);
        builder.addText(text);
        builder.pop();
        auto paragraph = builder.Build();
        paragraph->layout(1000.0f);
        paragraph->paint(this, 0, 0);
        return paragraph;
    }

private:
    SkPoint fOrigin = {0, 0};
    std::vector<SkPoint> fStack;
};
}  // namespace

// Underline gaps must hug the drawn glyphs regardless of baselineShift or height
DEF_TEST(SkParagraph_UnderlineGapsFollowDrawnGlyphs, reporter) {
    auto check = [&](SkScalar baselineShift, SkScalar height, bool halfLeading) {
        GapsRecordingPainter painter;
        painter.paintUnderlineGaps("gjpqy", baselineShift, height, halfLeading);
        REPORTER_ASSERT(reporter, painter.fBlobs.size() == 1);
        REPORTER_ASSERT(reporter, painter.fPaths.size() == 1);
        if (painter.fBlobs.size() != 1 || painter.fPaths.size() != 1) {
            return;
        }
        const auto& blob = painter.fBlobs[0];
        const auto& path = painter.fPaths[0];
        SkSpan<const SkPoint> points = path.fPath.points();
        // Without any gap the path holds only its two end points
        REPORTER_ASSERT(reporter, points.size() > 2);
        if (points.size() <= 2) {
            return;
        }

        // Decorations::calculateGaps() intersects glyphs with [top, top + thickness]
        // and keeps the underline one thickness (halo) away from every intersection
        const SkScalar thickness = path.fPaint.getStrokeWidth();
        const SkScalar top = path.fOrigin.fY + points[0].fY - blob.fOrigin.fY;
        const SkScalar bounds[2] = {top, top + thickness};
        int count = blob.fBlob->getIntercepts(bounds, nullptr, &path.fPaint);
        std::vector<SkScalar> intercepts(count);
        blob.fBlob->getIntercepts(bounds, intercepts.data(), &path.fPaint);
        REPORTER_ASSERT(reporter, count > 0);

        for (size_t i = 1; i + 1 < points.size(); ++i) {
            const SkScalar x = path.fOrigin.fX + points[i].fX - blob.fOrigin.fX;
            bool hugsGlyph = false;
            for (int j = 0; j < count; ++j) {
                const SkScalar edge = intercepts[j] + (j % 2 == 0 ? -thickness : thickness);
                hugsGlyph |= SkScalarNearlyEqual(x, edge, EPSILON100);
            }
            REPORTER_ASSERT(reporter,
                            hugsGlyph,
                            "shift=%g height=%g halfLeading=%d gap edge %g",
                            baselineShift,
                            height,
                            halfLeading,
                            x);
        }
    };
    check(0.0f, 0.0f, false);
    check(-12.0f, 0.0f, false);
    check(12.0f, 0.0f, false);
    check(0.0f, 2.0f, false);
    check(0.0f, 2.0f, true);
}

// An underline in gaps mode that no glyph crosses must still be drawn in full
DEF_TEST(SkParagraph_UnderlineGapsWithoutIntersections, reporter) {
    GapsRecordingPainter painter;
    auto paragraph = painter.paintUnderlineGaps("aaa");
    REPORTER_ASSERT(reporter, painter.fPaths.size() == 1);
    if (painter.fPaths.size() != 1) {
        return;
    }
    SkSpan<const SkPoint> points = painter.fPaths[0].fPath.points();
    REPORTER_ASSERT(reporter, points.size() == 2);
    if (points.size() != 2) {
        return;
    }
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(
                            points[1].fX - points[0].fX, paragraph->getLongestLine(), EPSILON100));
}

// Overline, underline, and line-through must shift vertically by exactly baselineShift,
// maintaining a constant offset from the drawn glyphs
DEF_TEST(SkParagraph_DecorationsFollowBaselineShift, reporter) {
    auto fontCollection = sk_make_sp<FontCollection>();
    fontCollection->setDefaultFontManager(ToolUtils::TestFontMgr());

    auto measure = [&](TextDecoration decoration, SkScalar baselineShift, SkScalar height) {
        ParagraphStyle paragraphStyle;
        TextStyle unshiftedStyle;
        unshiftedStyle.setFontSize(50.0f);
        unshiftedStyle.setHeightOverride(height != 0);
        unshiftedStyle.setHeight(height);

        TextStyle shiftedStyle = unshiftedStyle;
        shiftedStyle.setColor(SK_ColorBLACK);
        shiftedStyle.setDecoration(decoration);
        shiftedStyle.setDecorationStyle(TextDecorationStyle::kSolid);
        shiftedStyle.setBaselineShift(baselineShift);

        ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());
        builder.pushStyle(unshiftedStyle);
        builder.addText("Anchor ");
        builder.pushStyle(shiftedStyle);
        builder.addText("Shifted");
        builder.pop();
        builder.pop();
        auto paragraph = builder.Build();
        paragraph->layout(1000.0f);

        GapsRecordingPainter painter;
        paragraph->paint(&painter, 0, 0);
        REPORTER_ASSERT(reporter, painter.fBlobs.size() == 2);
        REPORTER_ASSERT(reporter, painter.fFilledRects.size() == 1);
        if (painter.fBlobs.size() != 2 || painter.fFilledRects.size() != 1) {
            return 0.0f;
        }
        return painter.fFilledRects[0].centerY() - painter.fBlobs[1].fOrigin.fY;
    };

    for (auto decoration :
         {TextDecoration::kUnderline, TextDecoration::kOverline, TextDecoration::kLineThrough}) {
        for (SkScalar height : {0.0f, 2.0f}) {
            const SkScalar baseOffset = measure(decoration, 0.0f, height);
            for (SkScalar shift : {-15.0f, 15.0f}) {
                const SkScalar shiftedOffset = measure(decoration, shift, height);
                REPORTER_ASSERT(reporter,
                                SkScalarNearlyEqual(shiftedOffset, baseOffset, 1.0f),
                                "decoration=%d height=%g shift=%g expected=%g got=%g",
                                static_cast<int>(decoration),
                                height,
                                shift,
                                baseOffset,
                                shiftedOffset);
            }
        }
    }
}

UNIX_ONLY_TEST(SkParagraph_ItalicsParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    TestCanvas canvas("SkParagraph_ItalicsParagraph.png");
    const char* text1 = "No italic ";
    const char* text2 = "Yes Italic ";
    const char* text3 = "No Italic again.";

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setFontSize(10);
    text_style.setColor(SK_ColorRED);
    builder.pushStyle(text_style);
    builder.addText(text1, strlen(text1));

    text_style.setFontStyle(SkFontStyle(
            SkFontStyle::kNormal_Weight, SkFontStyle::kNormal_Width, SkFontStyle::kItalic_Slant));
    builder.pushStyle(text_style);
    builder.addText(text2, strlen(text2));
    builder.pop();
    builder.addText(text3, strlen(text3));

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);
    paragraph->paint(canvas.get(), 0, 0);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());

    REPORTER_ASSERT(reporter, impl->runs().size() == 3);
    REPORTER_ASSERT(reporter, impl->styles().size() == 3);
    REPORTER_ASSERT(reporter, impl->lines().size() == 1);
    auto& line = impl->lines()[0];
    size_t index = 0;
    line.scanStyles(
            StyleType::kForeground,
            [&](TextRange textRange, const TextStyle& style, const TextLine::ClipContext& context) {
                switch (index) {
                    case 0:
                        REPORTER_ASSERT(
                                reporter,
                                style.getFontStyle().slant() == SkFontStyle::kUpright_Slant);
                        break;
                    case 1:
                        REPORTER_ASSERT(reporter,
                                        style.getFontStyle().slant() == SkFontStyle::kItalic_Slant);
                        break;
                    case 2:
                        REPORTER_ASSERT(
                                reporter,
                                style.getFontStyle().slant() == SkFontStyle::kUpright_Slant);
                        break;
                    default:
                        REPORTER_ASSERT(reporter, false);
                        break;
                }
                ++index;
                return true;
            });
}

// Checked: NO DIFF
UNIX_ONLY_TEST(SkParagraph_UnderlineShiftParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    TestCanvas canvas("SkParagraph_UnderlineShiftParagraph.png");
    const char* text1 = "fluttser ";
    const char* text2 = "mdje";
    const char* text3 = "fluttser mdje";

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    paragraph_style.setTextAlign(TextAlign::kLeft);
    paragraph_style.setMaxLines(2);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text1, strlen(text1));
    text_style.setDecoration(TextDecoration::kUnderline);
    text_style.setDecorationColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text2, strlen(text2));
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);
    paragraph->paint(canvas.get(), 0, 0);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());

    ParagraphBuilderImpl builder1(paragraph_style, fontCollection, get_unicode());
    text_style.setDecoration(TextDecoration::kNoDecoration);
    builder1.pushStyle(text_style);
    builder1.addText(text3, strlen(text3));
    builder1.pop();

    auto paragraph1 = builder1.Build();
    paragraph1->layout(TestCanvasWidth);
    paragraph1->paint(canvas.get(), 0, 25);

    auto impl1 = static_cast<ParagraphImpl*>(paragraph1.get());

    REPORTER_ASSERT(reporter, impl->lines().size() == 1);
    REPORTER_ASSERT(reporter, impl1->lines().size() == 1);

    auto rect = paragraph->getRectsForRange(0, 12, RectHeightStyle::kMax, RectWidthStyle::kTight)
                        .front()
                        .rect;
    auto rect1 = paragraph1->getRectsForRange(0, 12, RectHeightStyle::kMax, RectWidthStyle::kTight)
                         .front()
                         .rect;
    REPORTER_ASSERT(reporter, rect.fLeft == rect1.fLeft);
    REPORTER_ASSERT(reporter, rect.fRight == rect1.fRight);

    for (size_t i = 0; i < 12; ++i) {
        // Not all ranges produce a rectangle ("fl" goes into one cluster so [0:1) is empty)
        auto r1 = paragraph->getRectsForRange(
                i, i + 1, RectHeightStyle::kMax, RectWidthStyle::kTight);
        auto r2 = paragraph1->getRectsForRange(
                i, i + 1, RectHeightStyle::kMax, RectWidthStyle::kTight);

        REPORTER_ASSERT(reporter, r1.size() == r2.size());
        if (!r1.empty() && !r2.empty()) {
            REPORTER_ASSERT(reporter, r1.front().rect.fLeft == r2.front().rect.fLeft);
            REPORTER_ASSERT(reporter, r1.front().rect.fRight == r2.front().rect.fRight);
        }
    }
}

// Checked: NO DIFF
UNIX_ONLY_TEST(SkParagraph_SimpleShadow, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    TestCanvas canvas("SkParagraph_SimpleShadow.png");
    const char* text = "Hello World Text Dialog";
    const size_t len = strlen(text);

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setColor(SK_ColorBLACK);
    text_style.addShadow(TextShadow(SK_ColorBLACK, SkPoint::Make(2.0f, 2.0f), 1.0));
    builder.pushStyle(text_style);
    builder.addText(text, len);

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);
    paragraph->paint(canvas.get(), 10.0, 15.0);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());

    REPORTER_ASSERT(reporter, impl->runs().size() == 1);
    REPORTER_ASSERT(reporter, impl->styles().size() == 1);
    size_t index = 0;
    for (auto& line : impl->lines()) {
        line.scanStyles(StyleType::kShadow,
                        [&](TextRange textRange,
                            const TextStyle& style,
                            const TextLine::ClipContext& context) {
                            REPORTER_ASSERT(reporter, index == 0 && style.equals(text_style));
                            ++index;
                            return true;
                        });
    }
}

// Checked: NO DIFF
UNIX_ONLY_TEST(SkParagraph_ComplexShadow, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    TestCanvas canvas("SkParagraph_ComplexShadow.png");
    const char* text = "Text Chunk ";
    const size_t len = strlen(text);

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setColor(SK_ColorBLACK);
    text_style.addShadow(TextShadow(SK_ColorBLACK, SkPoint::Make(2.0f, 2.0f), 1.0f));
    builder.pushStyle(text_style);
    builder.addText(text, len);

    text_style.addShadow(TextShadow(SK_ColorRED, SkPoint::Make(2.0f, 2.0f), 5.0f));
    text_style.addShadow(TextShadow(SK_ColorGREEN, SkPoint::Make(10.0f, -5.0f), 3.0f));
    builder.pushStyle(text_style);
    builder.addText(text, len);
    builder.pop();

    builder.addText(text, len);

    text_style.addShadow(TextShadow(SK_ColorRED, SkPoint::Make(0.0f, 1.0f), 0.0f));
    builder.pushStyle(text_style);
    builder.addText(text, len);
    builder.pop();

    builder.addText(text, len);

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);
    paragraph->paint(canvas.get(), 10.0, 15.0);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());

    size_t index = 0;
    for (auto& line : impl->lines()) {
        line.scanStyles(StyleType::kShadow,
                        [&](TextRange textRange,
                            const TextStyle& style,
                            const TextLine::ClipContext& context) {
                            ++index;
                            switch (index) {
                                case 1:
                                    REPORTER_ASSERT(reporter, style.getShadowNumber() == 1);
                                    break;
                                case 2:
                                    REPORTER_ASSERT(reporter, style.getShadowNumber() == 3);
                                    break;
                                case 3:
                                    REPORTER_ASSERT(reporter, style.getShadowNumber() == 1);
                                    break;
                                case 4:
                                    REPORTER_ASSERT(reporter, style.getShadowNumber() == 4);
                                    REPORTER_ASSERT(reporter, style.equals(text_style));
                                    break;
                                case 5:
                                    REPORTER_ASSERT(reporter, style.getShadowNumber() == 1);
                                    break;
                                default:
                                    REPORTER_ASSERT(reporter, false);
                            }
                            return true;
                        });
    }
}
