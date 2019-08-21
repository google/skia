// Copyright 2019 Google LLC.
#include "src/utils/SkOSPath.h"
#include <sstream>
#include "modules/skparagraph/include/TypefaceFontProvider.h"
#include "modules/skparagraph/src/ParagraphBuilderImpl.h"
#include "modules/skparagraph/src/ParagraphImpl.h"
#include "src/core/SkOSFile.h"
#include "src/utils/SkShaperJSONWriter.h"
#include "tests/CodecPriv.h"
#include "tests/Test.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

#define VeryLongCanvasWidth 1000000
#define TestCanvasWidth 1000
#define TestCanvasHeight 600

using namespace skia::textlayout;
namespace {

SkScalar EPSILON100 = 0.01f;
SkScalar EPSILON50 = 0.02f;
SkScalar EPSILON20 = 0.05f;
SkScalar EPSILON10 = 0.1f;
SkScalar EPSILON5 = 0.20f;
SkScalar EPSILON2 = 0.50f;

bool equal(const char* base, TextRange a, const char* b) {
    return std::strncmp(b, base + a.start, a.width()) == 0;
}
class TestFontCollection : public FontCollection {
public:
    TestFontCollection()
            : fFontsFound(false)
            , fResolvedFonts(0)
            , fResourceDir(GetResourcePath("fonts").c_str())
            , fFontProvider(sk_make_sp<TypefaceFontProvider>()) {
        std::vector<SkString> fonts;
        SkOSFile::Iter iter(fResourceDir.c_str());
        SkString path;
        while (iter.next(&path)) {
            if (path.endsWith("Roboto-Italic.ttf")) {
                fFontsFound = true;
            }
            fonts.emplace_back(path);
        }

        if (!fFontsFound) {
            return;
        }
        // Only register fonts if we have to
        for (auto& font : fonts) {
            SkString file_path;
            file_path.printf("%s/%s", fResourceDir.c_str(), font.c_str());
            fFontProvider->registerTypeface(SkTypeface::MakeFromFile(file_path.c_str()));
        }

        this->setTestFontManager(std::move(fFontProvider));
        this->disableFontFallback();

        if (!fFontsFound) SkDebugf("Fonts not found, skipping all the tests\n");
    }

    ~TestFontCollection() = default;

    size_t resolvedFonts() const { return fResolvedFonts; }

    // TODO: temp solution until we check in fonts
    bool fontsFound() const { return fFontsFound; }

private:
    bool fFontsFound;
    size_t fResolvedFonts;
    std::string fResourceDir;
    sk_sp<TypefaceFontProvider> fFontProvider;
};

class TestCanvas {
public:
    TestCanvas(const char* testName) : name(testName) {
        bits.allocN32Pixels(TestCanvasWidth, TestCanvasHeight);
        canvas = new SkCanvas(bits);
        canvas->clear(SK_ColorWHITE);
    }

    ~TestCanvas() {
        SkString tmpDir = skiatest::GetTmpDir();
        if (!tmpDir.isEmpty()) {
            SkString path = SkOSPath::Join(tmpDir.c_str(), name);
            SkFILEWStream file(path.c_str());
            if (!SkEncodeImage(&file, bits, SkEncodedImageFormat::kPNG, 100)) {
                SkDebugf("Cannot write a picture %s\n", name);
            }
        }
        delete canvas;
    }

    void drawRects(SkColor color, std::vector<TextBox>& result, bool fill = false) {

        SkPaint paint;
        if (!fill) {
            paint.setStyle(SkPaint::kStroke_Style);
            paint.setAntiAlias(true);
            paint.setStrokeWidth(1);
        }
        paint.setColor(color);
        for (auto& r : result) {
            canvas->drawRect(r.rect, paint);
        }
    }

    void drawLine(SkColor color, SkRect rect, bool vertical = true) {

        SkPaint paint;
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setAntiAlias(true);
        paint.setStrokeWidth(1);
        paint.setColor(color);
        if (vertical) {
            canvas->drawLine(rect.fLeft, rect.fTop, rect.fLeft, rect.fBottom, paint);
        } else {
            canvas->drawLine(rect.fLeft, rect.fTop, rect.fRight, rect.fTop, paint);
        }
    }

    void drawLines(SkColor color, std::vector<TextBox>& result) {

        for (auto& r : result) {
            drawLine(color, r.rect);
        }
    }

    SkCanvas* get() { return canvas; }
private:
    SkBitmap bits;
    SkCanvas* canvas;
    const char* name;
};

}  // namespace

// Checked: NO DIFF
DEF_TEST(SkParagraph_SimpleParagraph, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    const char* text = "Hello World Text Dialog";

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, impl->runs().size() == 1);
    REPORTER_ASSERT(reporter, impl->styles().size() == 1);  // paragraph style does not count
    REPORTER_ASSERT(reporter, impl->styles()[0].fStyle.equals(text_style));

    size_t index = 0;
    for (auto& line : impl->lines()) {
        line.scanStyles(StyleType::kDecorations,
                        [&index, reporter](TextRange text, TextStyle style, SkScalar) {
                            REPORTER_ASSERT(reporter, index == 0);
                            REPORTER_ASSERT(reporter, style.getColor() == SK_ColorBLACK);
                            ++index;
                            return true;
                        });
    }
}

// Checked: DIFF? (letter_spacing/2 before the first letter)
DEF_TEST(SkParagraph_InlinePlaceholderParagraph, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    TestCanvas canvas("SkParagraph_InlinePlaceholderParagraph.png");
    if (!fontCollection->fontsFound()) return;

    const char* text = "012 34";

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    paragraph_style.setMaxLines(14);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setColor(SK_ColorBLACK);
    text_style.setFontSize(26);
    text_style.setWordSpacing(5);
    text_style.setLetterSpacing(1);
    text_style.setDecoration(TextDecoration::kUnderline);
    text_style.setDecorationColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text);

    PlaceholderStyle placeholder1(50, 50, PlaceholderAlignment::kBaseline, TextBaseline::kAlphabetic, 0);
    builder.addPlaceholder(placeholder1);
    builder.addText(text);
    builder.addPlaceholder(placeholder1);

    PlaceholderStyle placeholder2(5, 50, PlaceholderAlignment::kBaseline, TextBaseline::kAlphabetic, 50);
    builder.addPlaceholder(placeholder2);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder2);
    builder.addText(text);
    builder.addPlaceholder(placeholder2);
    builder.addText(text);
    builder.addText(text);
    builder.addPlaceholder(placeholder2);
    builder.addPlaceholder(placeholder2);
    builder.addPlaceholder(placeholder2);
    builder.addPlaceholder(placeholder2);
    builder.addPlaceholder(placeholder2);
    builder.addPlaceholder(placeholder1);
    builder.addText(text);
    builder.addText(text);
    builder.addText(text);
    builder.addText(text);
    builder.addText(text);
    builder.addPlaceholder(placeholder2);
    builder.addPlaceholder(placeholder1);
    builder.addText(text);
    builder.addText(text);

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

    boxes = paragraph->GetRectsForPlaceholders();
    canvas.drawRects(SK_ColorRED, boxes);

    boxes = paragraph->getRectsForRange(4, 17, rect_height_style, rect_width_style);
    canvas.drawRects(SK_ColorBLUE, boxes);

    REPORTER_ASSERT(reporter, boxes.size() == 7);

    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[1].rect.left(), 90.921f - 0.5f, EPSILON2));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[1].rect.top(), 50, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[1].rect.right(), 90.921f + 50 - 0.5f, EPSILON2));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[1].rect.bottom(), 100, EPSILON100));

    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[3].rect.left(), 231.343f - 0.5f, EPSILON2));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[3].rect.top(), 50, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[3].rect.right(), 231.343f + 50 - 0.5f, EPSILON2));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[3].rect.bottom(), 100, EPSILON100));

    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[4].rect.left(), 281.343f - 0.5f, EPSILON2));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[4].rect.top(), 0, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[4].rect.right(), 281.343f + 5 - 0.5f, EPSILON2));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[4].rect.bottom(), 50, EPSILON100));

    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[6].rect.left(), 336.343f - 0.5f, EPSILON2));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[6].rect.top(), 0, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[6].rect.right(), 336.343f + 5 - 0.5f, EPSILON2));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[6].rect.bottom(), 50, EPSILON100));
}

// Checked: DIFF? (letter_spacing/2 before the first letter)
DEF_TEST(SkParagraph_InlinePlaceholderBaselineParagraph, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    TestCanvas canvas("SkParagraph_InlinePlaceholderBaselineParagraph.png");
    if (!fontCollection->fontsFound()) return;

    const char* text = "012 34";

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    paragraph_style.setMaxLines(14);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setColor(SK_ColorBLACK);
    text_style.setFontSize(26);
    text_style.setWordSpacing(5);
    text_style.setLetterSpacing(1);
    text_style.setDecoration(TextDecoration::kUnderline);
    text_style.setDecorationColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text);

    PlaceholderStyle placeholder(55, 50, PlaceholderAlignment::kBaseline, TextBaseline::kAlphabetic, 38.347f);
    builder.addPlaceholder(placeholder);
    builder.addText(text);

    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);
    paragraph->paint(canvas.get(), 0, 0);

    auto boxes = paragraph->GetRectsForPlaceholders();
    canvas.drawRects(SK_ColorRED, boxes);

    REPORTER_ASSERT(reporter, boxes.size() == 1);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 90.921f - 0.5f, EPSILON2));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 0, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 90.921f + 55 - 0.5f, EPSILON2));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 50, EPSILON100));

    RectHeightStyle rect_height_style = RectHeightStyle::kTight;
    RectWidthStyle rect_width_style = RectWidthStyle::kTight;

    boxes = paragraph->getRectsForRange(5, 6, rect_height_style, rect_width_style);
    canvas.drawRects(SK_ColorBLUE, boxes);

    REPORTER_ASSERT(reporter, boxes.size() == 1);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 75.324f - 0.5f, EPSILON2));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 14.226f, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 90.921f - 0.5f, EPSILON2));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 44.694f, EPSILON100));
}

// Checked: DIFF? (letter_spacing/2 before the first letter)
DEF_TEST(SkParagraph_InlinePlaceholderAboveBaselineParagraph, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    TestCanvas canvas("SkParagraph_InlinePlaceholderAboveBaselineParagraph.png");
    if (!fontCollection->fontsFound()) return;

    const char* text = "012 34";

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    paragraph_style.setMaxLines(14);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setColor(SK_ColorBLACK);
    text_style.setFontSize(26);
    text_style.setWordSpacing(5);
    text_style.setLetterSpacing(1);
    text_style.setDecoration(TextDecoration::kUnderline);
    text_style.setDecorationColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text);

    PlaceholderStyle placeholder(55, 50, PlaceholderAlignment::kAboveBaseline, TextBaseline::kAlphabetic, 903129.129308f);
    builder.addPlaceholder(placeholder);
    builder.addText(text);

    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);
    paragraph->paint(canvas.get(), 0, 0);

    auto boxes = paragraph->GetRectsForPlaceholders();
    canvas.drawRects(SK_ColorRED, boxes);

    REPORTER_ASSERT(reporter, boxes.size() == 1);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 90.921f - 0.5f, EPSILON2));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), -0.347f, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 90.921f + 55 - 0.5f, EPSILON2));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 49.652f, EPSILON100));

    RectHeightStyle rect_height_style = RectHeightStyle::kTight;
    RectWidthStyle rect_width_style = RectWidthStyle::kTight;

    boxes = paragraph->getRectsForRange(5, 6, rect_height_style, rect_width_style);
    canvas.drawRects(SK_ColorBLUE, boxes);

    REPORTER_ASSERT(reporter, boxes.size() == 1);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 75.324f - 0.5f, EPSILON2));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 25.531f, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 90.921f - 0.5f, EPSILON2));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 56, EPSILON100));
}

// Checked: DIFF? (letter_spacing/2 before the first letter)
DEF_TEST(SkParagraph_InlinePlaceholderBelowBaselineParagraph, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    TestCanvas canvas("SkParagraph_InlinePlaceholderBelowBaselineParagraph.png");
    if (!fontCollection->fontsFound()) return;

    const char* text = "012 34";

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    paragraph_style.setMaxLines(14);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setColor(SK_ColorBLACK);
    text_style.setFontSize(26);
    text_style.setWordSpacing(5);
    text_style.setLetterSpacing(1);
    text_style.setDecoration(TextDecoration::kUnderline);
    text_style.setDecorationColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text);

    PlaceholderStyle placeholder(55, 50, PlaceholderAlignment::kBelowBaseline, TextBaseline::kAlphabetic, 903129.129308f);
    builder.addPlaceholder(placeholder);
    builder.addText(text);

    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);
    paragraph->paint(canvas.get(), 0, 0);

    auto boxes = paragraph->GetRectsForPlaceholders();
    canvas.drawRects(SK_ColorRED, boxes);

    REPORTER_ASSERT(reporter, boxes.size() == 1);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 90.921f - 0.5f, EPSILON2));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 24, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 90.921f + 55 - 0.5f, EPSILON2));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 74, EPSILON100));

    RectHeightStyle rect_height_style = RectHeightStyle::kTight;
    RectWidthStyle rect_width_style = RectWidthStyle::kTight;

    boxes = paragraph->getRectsForRange(5, 6, rect_height_style, rect_width_style);
    canvas.drawRects(SK_ColorBLUE, boxes);

    REPORTER_ASSERT(reporter, boxes.size() == 1);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 75.324f - 0.5f, EPSILON2));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), -0.121f, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 90.921f - 0.5f, EPSILON2));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 30.347f, EPSILON100));
}

// Checked: DIFF? (letter_spacing/2 before the first letter)
DEF_TEST(SkParagraph_InlinePlaceholderBottomParagraph, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    TestCanvas canvas("SkParagraph_InlinePlaceholderBottomParagraph.png");
    if (!fontCollection->fontsFound()) return;

    const char* text = "012 34";

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    paragraph_style.setMaxLines(14);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setColor(SK_ColorBLACK);
    text_style.setFontSize(26);
    text_style.setWordSpacing(5);
    text_style.setLetterSpacing(1);
    text_style.setDecoration(TextDecoration::kUnderline);
    text_style.setDecorationColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text);

    PlaceholderStyle placeholder(55, 50, PlaceholderAlignment::kBottom, TextBaseline::kAlphabetic, 0);
    builder.addPlaceholder(placeholder);
    builder.addText(text);

    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);
    paragraph->paint(canvas.get(), 0, 0);

    RectHeightStyle rect_height_style = RectHeightStyle::kTight;
    RectWidthStyle rect_width_style = RectWidthStyle::kTight;

    auto boxes = paragraph->GetRectsForPlaceholders();
    canvas.drawRects(SK_ColorRED, boxes);
    REPORTER_ASSERT(reporter, boxes.size() == 1);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 90.921f - 0.5f, EPSILON50));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 0, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 90.921f + 55 - 0.5f, EPSILON50));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 50, EPSILON100));

    boxes = paragraph->getRectsForRange(0, 1, rect_height_style, rect_width_style);
    canvas.drawRects(SK_ColorBLUE, boxes);
    REPORTER_ASSERT(reporter, boxes.size() == 1);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 0.5f - 0.5f, EPSILON50));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 19.531f, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 16.097f - 0.5f, EPSILON50));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 50, EPSILON100));
}

// Checked: DIFF? (letter_spacing/2 before the first letter)
DEF_TEST(SkParagraph_InlinePlaceholderTopParagraph, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    TestCanvas canvas("SkParagraph_InlinePlaceholderTopParagraph.png");
    if (!fontCollection->fontsFound()) return;

    const char* text = "012 34";

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    paragraph_style.setMaxLines(14);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setColor(SK_ColorBLACK);
    text_style.setFontSize(26);
    text_style.setWordSpacing(5);
    text_style.setLetterSpacing(1);
    text_style.setDecoration(TextDecoration::kUnderline);
    text_style.setDecorationColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text);

    PlaceholderStyle placeholder(55, 50, PlaceholderAlignment::kTop, TextBaseline::kAlphabetic, 0);
    builder.addPlaceholder(placeholder);
    builder.addText(text);

    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);
    paragraph->paint(canvas.get(), 0, 0);

    RectHeightStyle rect_height_style = RectHeightStyle::kTight;
    RectWidthStyle rect_width_style = RectWidthStyle::kTight;

    auto boxes = paragraph->GetRectsForPlaceholders();
    canvas.drawRects(SK_ColorRED, boxes);
    REPORTER_ASSERT(reporter, boxes.size() == 1);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 90.921f - 0.5f, EPSILON50));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 0, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 90.921f + 55 - 0.5f, EPSILON50));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 50, EPSILON100));

    boxes = paragraph->getRectsForRange(0, 1, rect_height_style, rect_width_style);
    canvas.drawRects(SK_ColorBLUE, boxes);
    REPORTER_ASSERT(reporter, boxes.size() == 1);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 0.5f - 0.5f, EPSILON50));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 0, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 16.097f - 0.5f, EPSILON50));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 30.468f, EPSILON100));
}

// Checked: DIFF? (letter_spacing/2 before the first letter)
DEF_TEST(SkParagraph_InlinePlaceholderMiddleParagraph, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    TestCanvas canvas("SkParagraph_InlinePlaceholderMiddleParagraph.png");
    if (!fontCollection->fontsFound()) return;

    const char* text = "012 34";

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    paragraph_style.setMaxLines(14);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setColor(SK_ColorBLACK);
    text_style.setFontSize(26);
    text_style.setWordSpacing(5);
    text_style.setLetterSpacing(1);
    text_style.setDecoration(TextDecoration::kUnderline);
    text_style.setDecorationColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text);

    PlaceholderStyle placeholder(55, 50, PlaceholderAlignment::kMiddle, TextBaseline::kAlphabetic, 0);
    builder.addPlaceholder(placeholder);
    builder.addText(text);

    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);
    paragraph->paint(canvas.get(), 0, 0);

    RectHeightStyle rect_height_style = RectHeightStyle::kTight;
    RectWidthStyle rect_width_style = RectWidthStyle::kTight;

    auto boxes = paragraph->GetRectsForPlaceholders();
    canvas.drawRects(SK_ColorRED, boxes);
    REPORTER_ASSERT(reporter, boxes.size() == 1);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 90.921f - 0.5f, EPSILON50));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 0, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 90.921f + 55 - 0.5f, EPSILON50));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 50, EPSILON100));

    boxes = paragraph->getRectsForRange(5, 6, rect_height_style, rect_width_style);
    canvas.drawRects(SK_ColorBLUE, boxes);
    REPORTER_ASSERT(reporter, boxes.size() == 1);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 75.324f - 0.5f, EPSILON50));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 9.765f, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 90.921f - 0.5f, EPSILON50));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 40.234f, EPSILON100));
}

// Checked: DIFF? (letter_spacing/2 before the first letter)
DEF_TEST(SkParagraph_InlinePlaceholderIdeographicBaselineParagraph, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    TestCanvas canvas("SkParagraph_InlinePlaceholderIdeographicBaselineParagraph.png");
    if (!fontCollection->fontsFound()) return;

    const char* text = "給能上目秘使";

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    paragraph_style.setMaxLines(14);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Source Han Serif CN")});
    text_style.setColor(SK_ColorBLACK);
    text_style.setFontSize(26);
    text_style.setWordSpacing(5);
    text_style.setLetterSpacing(1);
    text_style.setDecoration(TextDecoration::kUnderline);
    text_style.setDecorationColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text);
    PlaceholderStyle placeholder(55, 50, PlaceholderAlignment::kBaseline, TextBaseline::kIdeographic, 38.347f);
    builder.addPlaceholder(placeholder);
    builder.addText(text);

    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);
    paragraph->paint(canvas.get(), 0, 0);

    RectHeightStyle rect_height_style = RectHeightStyle::kTight;
    RectWidthStyle rect_width_style = RectWidthStyle::kTight;

    auto boxes = paragraph->GetRectsForPlaceholders();
    canvas.drawRects(SK_ColorRED, boxes);
    REPORTER_ASSERT(reporter, boxes.size() == 1);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 162.5f - 0.5f, EPSILON50));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 0, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 162.5f + 55 - 0.5f, EPSILON50));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 50, EPSILON100));

    boxes = paragraph->getRectsForRange(5, 6, rect_height_style, rect_width_style);
    canvas.drawRects(SK_ColorBLUE, boxes);
    REPORTER_ASSERT(reporter, boxes.size() == 1);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 135.5f - 0.5f, EPSILON50));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 4.703f, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 162.5f - 0.5f, EPSILON50));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 42.065f, EPSILON100));
}

// Checked: DIFF? (letter_spacing/2 before the first letter)
DEF_TEST(SkParagraph_InlinePlaceholderBreakParagraph, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    TestCanvas canvas("SkParagraph_InlinePlaceholderBreakParagraph.png");
    if (!fontCollection->fontsFound()) return;

    const char* text = "012 34";

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    paragraph_style.setMaxLines(14);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setColor(SK_ColorBLACK);
    text_style.setFontSize(26);
    text_style.setWordSpacing(5);
    text_style.setLetterSpacing(1);
    text_style.setDecoration(TextDecoration::kUnderline);
    text_style.setDecorationColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text);

    PlaceholderStyle placeholder1(50, 50, PlaceholderAlignment::kBaseline, TextBaseline::kAlphabetic, 50);
    PlaceholderStyle placeholder2(25, 25, PlaceholderAlignment::kBaseline, TextBaseline::kAlphabetic, 12.5f);

    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder2);
    builder.addPlaceholder(placeholder1);
    builder.addText(text);

    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder2); // 4 + 1
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder2); // 6 + 1
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder2); // 7 + 1

    builder.addPlaceholder(placeholder1);
    builder.addText(text);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder2);

    builder.addText(text);
    builder.addText(text);
    builder.addText(text);
    builder.addText(text);

    builder.addPlaceholder(placeholder2);
    builder.addPlaceholder(placeholder1);

    builder.addText(text);

    builder.addPlaceholder(placeholder2);

    builder.addText(text);
    builder.addText(text);
    builder.addText(text);
    builder.addText(text);
    builder.addText(text);
    builder.addText(text);
    builder.addText(text);
    builder.addText(text);
    builder.addText(text);
    builder.addText(text);
    builder.addText(text);
    builder.addText(text);
    builder.addText(text);
    builder.addText(text);
    builder.addText(text);
    builder.addText(text);
    builder.addText(text);
    builder.addText(text);
    builder.addText(text);

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
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 31.695f - 0.5f, EPSILON50));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 218.531f, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 47.292f - 0.5f, EPSILON50));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 249, EPSILON100));

    boxes = paragraph->GetRectsForPlaceholders();
    canvas.drawRects(SK_ColorRED, boxes);

    boxes = paragraph->getRectsForRange(4, 45, rect_height_style, rect_width_style);
    canvas.drawRects(SK_ColorBLUE, boxes);
    REPORTER_ASSERT(reporter, boxes.size() == 30);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 59.726f - 0.5f, EPSILON50));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 26.378f, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 90.921f - 0.5f, EPSILON50));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 56.847f, EPSILON100));

    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[11].rect.left(), 606.343f - 0.5f, EPSILON20));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[11].rect.top(), 38, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[11].rect.right(), 631.343f - 0.5f, EPSILON20));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[11].rect.bottom(), 63, EPSILON100));

    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[17].rect.left(), 0.5f - 0.5f, EPSILON50));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[17].rect.top(), 63.5f, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[17].rect.right(), 50.5f - 0.5f, EPSILON50));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[17].rect.bottom(), 113.5f, EPSILON100));
}

// Checked: DIFF? (letter_spacing/2 before the first letter)
DEF_TEST(SkParagraph_InlinePlaceholderGetRectsParagraph, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    TestCanvas canvas("SkParagraph_InlinePlaceholderGetRectsParagraph.png");
    if (!fontCollection->fontsFound()) return;

    const char* text = "012 34";

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    paragraph_style.setMaxLines(14);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setColor(SK_ColorBLACK);
    text_style.setFontSize(26);
    text_style.setWordSpacing(5);
    text_style.setLetterSpacing(1);
    text_style.setDecoration(TextDecoration::kUnderline);
    text_style.setDecorationColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text);

    PlaceholderStyle placeholder1(50, 50, PlaceholderAlignment::kBaseline, TextBaseline::kAlphabetic, 50);
    PlaceholderStyle placeholder2(5, 20, PlaceholderAlignment::kBaseline, TextBaseline::kAlphabetic, 10);

    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder2); // 8 + 1
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder2); // 5 + 1
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder1); // 8 + 0

    builder.addText(text);

    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder2);
    builder.addPlaceholder(placeholder2); // 1 + 2
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder2);
    builder.addPlaceholder(placeholder2); // 1 + 2

    builder.addText(text);
    builder.addText(text);
    builder.addText(text);
    builder.addText(text);
    builder.addText(text);
    builder.addText(text);
    builder.addText(text);
    builder.addText(text);
    builder.addText(text);
    builder.addText(text);
    builder.addText(text);  // 11

    builder.addPlaceholder(placeholder2);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder2);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder2);

    builder.addText(text);

    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);
    paragraph->paint(canvas.get(), 0, 0);

    RectHeightStyle rect_height_style = RectHeightStyle::kMax;
    RectWidthStyle rect_width_style = RectWidthStyle::kTight;

    auto boxes = paragraph->GetRectsForPlaceholders();
    canvas.drawRects(SK_ColorRED, boxes);

    REPORTER_ASSERT(reporter, boxes.size() == 34);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 90.921f - 0.5f, EPSILON50));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 0, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 140.921f - 0.5f, EPSILON50));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 50, EPSILON100));

    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[16].rect.left(), 800.921f - 0.5f, EPSILON20));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[16].rect.top(), 0, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[16].rect.right(), 850.921f - 0.5f, EPSILON20));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[16].rect.bottom(), 50, EPSILON100));

    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[33].rect.left(), 503.382f - 0.5f, EPSILON10));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[33].rect.top(), 160, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[33].rect.right(), 508.382f - 0.5f, EPSILON10));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[33].rect.bottom(), 180, EPSILON100));

    boxes = paragraph->getRectsForRange(30, 50, rect_height_style, rect_width_style);
    canvas.drawRects(SK_ColorBLUE, boxes);

    REPORTER_ASSERT(reporter, boxes.size() == 8);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 216.097f - 0.5f, EPSILON50));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 60, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 290.921f - 0.5f, EPSILON50));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 120, EPSILON100));

    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[1].rect.left(), 290.921f - 0.5f, EPSILON20));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[1].rect.top(), 60, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[1].rect.right(), 340.921f - 0.5f, EPSILON20));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[1].rect.bottom(), 120, EPSILON100));

    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[2].rect.left(), 340.921f - 0.5f, EPSILON50));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[2].rect.top(), 60, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[2].rect.right(), 345.921f - 0.5f, EPSILON50));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[2].rect.bottom(), 120, EPSILON100));
}

// Checked: NO DIFF
DEF_TEST(SkParagraph_SimpleRedParagraph, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    const char* text = "I am RED";

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setColor(SK_ColorRED);
    builder.pushStyle(text_style);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, impl->runs().size() == 1);
    REPORTER_ASSERT(reporter, impl->styles().size() == 1);  // paragraph style does not count
    REPORTER_ASSERT(reporter, impl->styles()[0].fStyle.equals(text_style));

    size_t index = 0;
    for (auto& line : impl->lines()) {
        line.scanStyles(StyleType::kDecorations,
                        [&index, reporter](TextRange text, TextStyle style, SkScalar) {
                            REPORTER_ASSERT(reporter, index == 0);
                            REPORTER_ASSERT(reporter, style.getColor() == SK_ColorRED);
                            ++index;
                            return true;
                        });
    }
}

// Checked: DIFF+ (Space between 1 & 2 style blocks)
DEF_TEST(SkParagraph_RainbowParagraph, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    TestCanvas canvas("SkParagraph_RainbowParagraph.png");
    if (!fontCollection->fontsFound()) return;
    const char* text1 = "Red Roboto"; // [0:10)
    const char* text2 = "big Greeen Default"; // [10:28)
    const char* text3 = "Defcolor Homemade Apple"; // [28:51)
    const char* text4 = "Small Blue Roboto"; // [51:68)
    const char* text41 = "Small Blue ";
    const char* text5 =
            "Continue Last Style With lots of words to check if it overlaps "
            "properly or not"; // [68:)
    const char* text42 =
            "Roboto"
            "Continue Last Style With lots of words to check if it overlaps "
            "properly or not";

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    paragraph_style.setTextAlign(TextAlign::kLeft);
    paragraph_style.setMaxLines(2);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style1;
    text_style1.setFontFamilies({SkString("Roboto")});

    text_style1.setColor(SK_ColorRED);
    builder.pushStyle(text_style1);
    builder.addText(text1);

    TextStyle text_style2;
    text_style2.setFontFamilies({SkString("Roboto")});
    text_style2.setFontSize(50);
    text_style2.setFontStyle(SkFontStyle(SkFontStyle::kMedium_Weight, SkFontStyle::kNormal_Width,
                                         SkFontStyle::kUpright_Slant));
    text_style2.setLetterSpacing(10);
    text_style2.setDecorationColor(SK_ColorBLACK);
    text_style2.setDecoration((TextDecoration)(
            TextDecoration::kUnderline | TextDecoration::kOverline | TextDecoration::kLineThrough));
    text_style2.setWordSpacing(30);
    text_style2.setColor(SK_ColorGREEN);
    builder.pushStyle(text_style2);
    builder.addText(text2);

    TextStyle text_style3;
    text_style3.setFontFamilies({SkString("Homemade Apple")});
    builder.pushStyle(text_style3);
    builder.addText(text3);

    TextStyle text_style4;
    text_style4.setFontFamilies({SkString("Roboto")});
    text_style4.setFontSize(14);
    text_style4.setDecorationColor(SK_ColorBLACK);
    text_style4.setDecoration((TextDecoration)(
            TextDecoration::kUnderline | TextDecoration::kOverline | TextDecoration::kLineThrough));
    text_style4.setColor(SK_ColorBLUE);
    builder.pushStyle(text_style4);
    builder.addText(text4);

    builder.addText(text5);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(1000);
    paragraph->paint(canvas.get(), 0, 0);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, impl->runs().size() == 4);
    REPORTER_ASSERT(reporter, impl->styles().size() == 4);
    REPORTER_ASSERT(reporter, impl->lines().size() == 2);

    auto rects = paragraph->getRectsForRange(0, impl->text().size(), RectHeightStyle::kMax, RectWidthStyle::kTight);
    canvas.drawRects(SK_ColorMAGENTA, rects);

    size_t index = 0;
    impl->lines()[0].scanStyles(
        StyleType::kAllAttributes, [&](TextRange text, TextStyle style, SkScalar) {
            switch (index) {
                case 0:
                    REPORTER_ASSERT(reporter, style.equals(text_style1));
                    REPORTER_ASSERT(reporter, equal(impl->text().begin(), text, text1));
                    break;
                case 1:
                    REPORTER_ASSERT(reporter, style.equals(text_style2));
                    REPORTER_ASSERT(reporter, equal(impl->text().begin(), text, text2));
                    break;
                case 2:
                    REPORTER_ASSERT(reporter, style.equals(text_style3));
                    REPORTER_ASSERT(reporter, equal(impl->text().begin(), text, text3));
                    break;
                case 3:
                    REPORTER_ASSERT(reporter, style.equals(text_style4));
                    REPORTER_ASSERT(reporter, equal(impl->text().begin(), text, text41));
                    break;
                default:
                    REPORTER_ASSERT(reporter, false);
                    break;
            }
            ++index;
            return true;
        });
    impl->lines()[1].scanStyles(
    StyleType::kAllAttributes, [&](TextRange text, TextStyle style, SkScalar) {
        switch (index) {
            case 4:
                REPORTER_ASSERT(reporter, style.equals(text_style4));
                REPORTER_ASSERT(reporter, equal(impl->text().begin(), text, text42));
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

// Checked: NO DIFF
DEF_TEST(SkParagraph_DefaultStyleParagraph, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_DefaultStyleParagraph.png");
    const char* text = "No TextStyle! Uh Oh!";

    ParagraphStyle paragraph_style;
    TextStyle defaultStyle;
    defaultStyle.setFontFamilies({SkString("Roboto")});
    paragraph_style.setTextStyle(defaultStyle);
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);
    builder.addText(text);

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);
    paragraph->paint(canvas.get(), 10.0, 15.0);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());

    REPORTER_ASSERT(reporter, impl->runs().size() == 1);
    REPORTER_ASSERT(reporter, impl->styles().size() == 1);
    REPORTER_ASSERT(reporter, impl->lines().size() == 1);

    size_t index = 0;
    impl->lines()[0].scanStyles(
            StyleType::kAllAttributes, [&](TextRange text1, TextStyle style, SkScalar) {
                REPORTER_ASSERT(reporter, style.equals(paragraph_style.getTextStyle()));
                REPORTER_ASSERT(reporter, equal(impl->text().begin(), text1, text));
                ++index;
                return true;
            });
    REPORTER_ASSERT(reporter, index == 1);
}

// Checked: NO DIFF
DEF_TEST(SkParagraph_BoldParagraph, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_BoldParagraph.png");
    const char* text = "This is Red max bold text!";

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setColor(SK_ColorRED);
    text_style.setFontSize(60);
    text_style.setLetterSpacing(0);
    text_style.setFontStyle(SkFontStyle(SkFontStyle::kBlack_Weight, SkFontStyle::kNormal_Width,
                                        SkFontStyle::kUpright_Slant));
    builder.pushStyle(text_style);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(VeryLongCanvasWidth);
    paragraph->paint(canvas.get(), 10.0, 60.0);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());

    REPORTER_ASSERT(reporter, impl->runs().size() == 1);
    REPORTER_ASSERT(reporter, impl->styles().size() == 1);
    REPORTER_ASSERT(reporter, impl->lines().size() == 1);

    size_t index = 0;
    impl->lines()[0].scanStyles(
            StyleType::kAllAttributes, [&](TextRange text1, TextStyle style, SkScalar) {
                REPORTER_ASSERT(reporter, style.equals(text_style));
                REPORTER_ASSERT(reporter, equal(impl->text().begin(), text1, text));
                ++index;
                return true;
            });
    REPORTER_ASSERT(reporter, index == 1);
}

// Checked: NO DIFF (line height rounding error)
DEF_TEST(SkParagraph_HeightOverrideParagraph, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_HeightOverrideParagraph.png");
    const char* text = "01234満毎冠行来昼本可\nabcd\n満毎冠行来昼本可";

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    paragraph_style.setMaxLines(10);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setFontSize(20);
    text_style.setColor(SK_ColorBLACK);
    text_style.setHeight(3.6345f);
    text_style.setHeightOverride(true);
    builder.pushStyle(text_style);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(550);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, impl->runs().size() == 3);
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
    std::vector<TextBox> boxes = paragraph->getRectsForRange(0, 0, rect_height_style, rect_width_style);
    canvas.drawRects(SK_ColorRED, boxes);
    REPORTER_ASSERT(reporter, boxes.size() == 0ull);

    boxes = paragraph->getRectsForRange(0, 40, rect_height_style, rect_width_style);
    canvas.drawRects(SK_ColorBLUE, boxes);
    REPORTER_ASSERT(reporter, boxes.size() == 3ull);

    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[1].rect.left(), 0, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[1].rect.top(), 92.805f, EPSILON5));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[1].rect.right(), 43.843f, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[1].rect.bottom(), 165.495f, EPSILON5));
}

// Checked: DIFF+
DEF_TEST(SkParagraph_LeftAlignParagraph, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
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

    ParagraphStyle paragraph_style;
    paragraph_style.setMaxLines(14);
    paragraph_style.setTextAlign(TextAlign::kLeft);
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

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
    builder.addText(text);
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

// Checked: NO DIFF
DEF_TEST(SkParagraph_RightAlignParagraph, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
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

    ParagraphStyle paragraph_style;
    paragraph_style.setMaxLines(14);
    paragraph_style.setTextAlign(TextAlign::kRight);
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

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
    builder.addText(text);
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

// Checked: NO DIFF
DEF_TEST(SkParagraph_CenterAlignParagraph, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
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

    ParagraphStyle paragraph_style;
    paragraph_style.setMaxLines(14);
    paragraph_style.setTextAlign(TextAlign::kCenter);
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

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
    builder.addText(text);
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

// Checked: NO DIFF
DEF_TEST(SkParagraph_JustifyAlignParagraph, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
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

    ParagraphStyle paragraph_style;
    paragraph_style.setMaxLines(14);
    paragraph_style.setTextAlign(TextAlign::kJustify);
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

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
    builder.addText(text);
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
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(impl->lines()[0].baseline(), 24.121f, EPSILON100));
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

    auto calculate = [](const TextLine& line) -> SkScalar {
        return TestCanvasWidth - 100 - (line.offset().fX + line.width());
    };

    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(calculate(impl->lines()[0]), 0, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(calculate(impl->lines()[1]), 0, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(calculate(impl->lines()[2]), 0, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(calculate(impl->lines()[3]), 0, EPSILON100));
    REPORTER_ASSERT(reporter, calculate(impl->lines()[13]) > 0);

    REPORTER_ASSERT(reporter,
                    paragraph_style.getTextAlign() == impl->paragraphStyle().getTextAlign());
}

// Checked: DIFF (ghost spaces as a separate box in TxtLib)
DEF_TEST(SkParagraph_JustifyRTL, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_JustifyRTL.png");
    const char* text =
            "אאא בּבּבּבּ אאאא בּבּ אאא בּבּבּ אאאאא בּבּבּבּ אאאא בּבּבּבּבּ "
            "אאאאא בּבּבּבּבּ אאאבּבּבּבּבּבּאאאאא בּבּבּבּבּבּאאאאאבּבּבּבּבּבּ אאאאא בּבּבּבּבּ "
            "אאאאא בּבּבּבּבּבּ אאאאא בּבּבּבּבּבּ אאאאא בּבּבּבּבּבּ אאאאא בּבּבּבּבּבּ אאאאא בּבּבּבּבּבּ";

    ParagraphStyle paragraph_style;
    paragraph_style.setMaxLines(14);
    paragraph_style.setTextAlign(TextAlign::kJustify);
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Ahem")});
    text_style.setFontSize(26);
    text_style.setColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth - 100);
    paragraph->paint(canvas.get(), 0, 0);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());

    auto calculate = [](const TextLine& line) -> SkScalar {
        return TestCanvasWidth - 100 - line.width();
    };
    for (auto& line : impl->lines()) {
        if (&line == &impl->lines().back()) {
            REPORTER_ASSERT(reporter, calculate(line) > EPSILON100);
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
    REPORTER_ASSERT(reporter, boxes.size() == 3); // DIFF

    boxes = paragraph->getRectsForRange(240, 250, rect_height_style, rect_width_style);
    canvas.drawRects(SK_ColorBLUE, boxes);
    REPORTER_ASSERT(reporter, boxes.size() == 1);

    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 588, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 130, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 640, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 156, EPSILON100));
}

// Checked: NO DIFF (some minor decoration differences, probably)
DEF_TEST(SkParagraph_DecorationsParagraph, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_DecorationsParagraph.png");
    ParagraphStyle paragraph_style;
    paragraph_style.setMaxLines(14);
    paragraph_style.setTextAlign(TextAlign::kLeft);
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setFontSize(26);
    text_style.setLetterSpacing(0);
    text_style.setWordSpacing(5);
    text_style.setColor(SK_ColorBLACK);
    text_style.setHeight(2);
    text_style.setDecoration(TextDecoration::kUnderline);
    text_style.setDecorationColor(SK_ColorBLACK);
    text_style.setDecoration((TextDecoration)(
            TextDecoration::kUnderline | TextDecoration::kOverline | TextDecoration::kLineThrough));
    text_style.setDecorationStyle(TextDecorationStyle::kSolid);
    text_style.setDecorationColor(SK_ColorBLACK);
    text_style.setDecorationThicknessMultiplier(2.0);
    builder.pushStyle(text_style);
    builder.addText("This text should be");

    text_style.setDecorationStyle(TextDecorationStyle::kDouble);
    text_style.setDecorationColor(SK_ColorBLUE);
    text_style.setDecorationThicknessMultiplier(1.0);
    builder.pushStyle(text_style);
    builder.addText(" decorated even when");

    text_style.setDecorationStyle(TextDecorationStyle::kDotted);
    text_style.setDecorationColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(" wrapped around to");

    text_style.setDecorationStyle(TextDecorationStyle::kDashed);
    text_style.setDecorationColor(SK_ColorBLACK);
    text_style.setDecorationThicknessMultiplier(3.0);
    builder.pushStyle(text_style);
    builder.addText(" the next line.");

    text_style.setDecorationStyle(TextDecorationStyle::kWavy);
    text_style.setDecorationColor(SK_ColorRED);
    text_style.setDecorationThicknessMultiplier(1.0);
    builder.pushStyle(text_style);
    builder.addText(" Otherwise, bad things happen.");
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth - 100);
    paragraph->paint(canvas.get(), 0, 0);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());

    size_t index = 0;
    for (auto& line : impl->lines()) {
        line.scanStyles(
                StyleType::kDecorations, [&index, reporter](TextRange, TextStyle style, SkScalar) {
                    auto decoration = (TextDecoration)(TextDecoration::kUnderline |
                                                       TextDecoration::kOverline |
                                                       TextDecoration::kLineThrough);
                    REPORTER_ASSERT(reporter, style.getDecorationType() == decoration);
                    switch (index) {
                        case 0:
                            REPORTER_ASSERT(reporter, style.getDecorationStyle() ==
                                                              TextDecorationStyle::kSolid);
                            REPORTER_ASSERT(reporter, style.getDecorationColor() == SK_ColorBLACK);
                            REPORTER_ASSERT(reporter,
                                            style.getDecorationThicknessMultiplier() == 2.0);
                            break;
                        case 1:  // The style appears on 2 lines so it has 2 pieces
                            REPORTER_ASSERT(reporter, style.getDecorationStyle() ==
                                                              TextDecorationStyle::kDouble);
                            REPORTER_ASSERT(reporter, style.getDecorationColor() == SK_ColorBLUE);
                            REPORTER_ASSERT(reporter,
                                            style.getDecorationThicknessMultiplier() == 1.0);
                            break;
                        case 2:
                            REPORTER_ASSERT(reporter, style.getDecorationStyle() ==
                                                              TextDecorationStyle::kDotted);
                            REPORTER_ASSERT(reporter, style.getDecorationColor() == SK_ColorBLACK);
                            REPORTER_ASSERT(reporter,
                                            style.getDecorationThicknessMultiplier() == 1.0);
                            break;
                        case 3:
                        case 4:
                            REPORTER_ASSERT(reporter, style.getDecorationStyle() ==
                                                              TextDecorationStyle::kDashed);
                            REPORTER_ASSERT(reporter, style.getDecorationColor() == SK_ColorBLACK);
                            REPORTER_ASSERT(reporter,
                                            style.getDecorationThicknessMultiplier() == 3.0);
                            break;
                        case 5:
                            REPORTER_ASSERT(reporter, style.getDecorationStyle() ==
                                                              TextDecorationStyle::kWavy);
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

DEF_TEST(SkParagraph_WavyDecorationParagraph, reporter) {
    SkDebugf("TODO: Fix decorations\n");
}

// Checked: NO DIFF
DEF_TEST(SkParagraph_ItalicsParagraph, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_ItalicsParagraph.png");
    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setFontSize(10);
    text_style.setColor(SK_ColorRED);
    builder.pushStyle(text_style);
    builder.addText("No italic ");

    text_style.setFontStyle(SkFontStyle(SkFontStyle::kNormal_Weight, SkFontStyle::kNormal_Width,
                                        SkFontStyle::kItalic_Slant));
    builder.pushStyle(text_style);
    builder.addText("Yes Italic ");
    builder.pop();
    builder.addText("No Italic again.");

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
        [&index, reporter](TextRange textRange, TextStyle style, SkScalar) {
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
DEF_TEST(SkParagraph_ChineseParagraph, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_ChineseParagraph.png");
    const char* text =
            "左線読設重説切後碁給能上目秘使約。満毎冠行来昼本可必図将発確年。今属場育"
            "図情闘陰野高備込制詩西校客。審対江置講今固残必託地集済決維駆年策。立得庭"
            "際輝求佐抗蒼提夜合逃表。注統天言件自謙雅載報紙喪。作画稿愛器灯女書利変探"
            "訃第金線朝開化建。子戦年帝励害表月幕株漠新期刊人秘。図的海力生禁挙保天戦"
            "聞条年所在口。";

    ParagraphStyle paragraph_style;
    paragraph_style.setMaxLines(14);
    paragraph_style.setTextAlign(TextAlign::kJustify);
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    auto decoration = (TextDecoration)(TextDecoration::kUnderline | TextDecoration::kOverline |
                                       TextDecoration::kLineThrough);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Source Han Serif CN")});
    text_style.setFontSize(35);
    text_style.setColor(SK_ColorBLACK);
    text_style.setLetterSpacing(2);
    text_style.setHeight(1);
    text_style.setDecoration(decoration);
    text_style.setDecorationColor(SK_ColorBLACK);
    text_style.setDecorationStyle(TextDecorationStyle::kSolid);
    builder.pushStyle(text_style);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth - 100);
    paragraph->paint(canvas.get(), 0, 0);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());

    REPORTER_ASSERT(reporter, impl->runs().size() == 1);
    REPORTER_ASSERT(reporter, impl->lines().size() == 7);
    REPORTER_ASSERT(reporter, impl->styles().size() == 1);
    REPORTER_ASSERT(reporter, impl->styles()[0].fStyle.equals(text_style));
}

// Checked: NO DIFF (disabled)
DEF_TEST(SkParagraph_ArabicParagraph, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_ArabicParagraph.png");
    const char* text =
            "من أسر وإعلان الخاصّة وهولندا،, عل قائمة الضغوط بالمطالبة تلك. الصفحة "
            "بمباركة التقليدية قام عن. تصفح";

    ParagraphStyle paragraph_style;
    paragraph_style.setMaxLines(14);
    paragraph_style.setTextAlign(TextAlign::kJustify);
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

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
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth - 100);
    paragraph->paint(canvas.get(), 0, 0);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());

    REPORTER_ASSERT(reporter, impl->runs().size() == 1);
    REPORTER_ASSERT(reporter, impl->lines().size() == 2);
    REPORTER_ASSERT(reporter, impl->styles().size() == 1);
    REPORTER_ASSERT(reporter, impl->styles()[0].fStyle.equals(text_style));
}

// Checked: DIFF (2 boxes and each space is a word)
DEF_TEST(SkParagraph_ArabicRectsParagraph, reporter) {

    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_ArabicRectsParagraph.png");
    const char* text = "بمباركة التقليدية قام عن. تصفح يد    ";

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    paragraph_style.setMaxLines(14);
    paragraph_style.setTextAlign(TextAlign::kRight);
    paragraph_style.setTextDirection(TextDirection::kRtl);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Noto Naskh Arabic")});
    text_style.setFontSize(26);
    text_style.setWordSpacing(5);
    text_style.setColor(SK_ColorBLACK);
    text_style.setDecoration(TextDecoration::kUnderline);
    text_style.setDecorationColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth - 100);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, impl->runs().size() == 1);

    paragraph->paint(canvas.get(), 0, 0);

    RectHeightStyle rect_height_style = RectHeightStyle::kMax;
    RectWidthStyle rect_width_style = RectWidthStyle::kTight;
    std::vector<TextBox> boxes = paragraph->getRectsForRange(0, 100, rect_height_style, rect_width_style);
    canvas.drawRects(SK_ColorRED, boxes);

    REPORTER_ASSERT(reporter, boxes.size() == 1ull);

    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 538.548f, EPSILON100));  // DIFF: 510.09375
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), -0.268f, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(),  900, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 44, EPSILON100));
}

// Checked DIFF+
DEF_TEST(SkParagraph_ArabicRectsLTRLeftAlignParagraph, reporter) {

    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_ArabicRectsLTRLeftAlignParagraph.png");
    const char* text = "Helloبمباركة التقليدية قام عن. تصفح يد ";

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    paragraph_style.setMaxLines(14);
    paragraph_style.setTextAlign(TextAlign::kLeft);
    paragraph_style.setTextDirection(TextDirection::kLtr);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Noto Naskh Arabic")});
    text_style.setFontSize(26);
    text_style.setWordSpacing(5);
    text_style.setColor(SK_ColorBLACK);
    text_style.setDecoration(TextDecoration::kUnderline);
    text_style.setDecorationColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth - 100);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, impl->runs().size() == 3);

    paragraph->paint(canvas.get(), 0, 0);

    RectHeightStyle rect_height_style = RectHeightStyle::kMax;
    RectWidthStyle rect_width_style = RectWidthStyle::kTight;
    std::vector<TextBox> boxes = paragraph->getRectsForRange(36, 40, rect_height_style, rect_width_style);
    canvas.drawRects(SK_ColorRED, boxes);

    REPORTER_ASSERT(reporter, boxes.size() == 2ull); // DIFF: 1
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 83.916f, EPSILON100));  // DIFF: 89.40625
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), -0.268f, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 110.155f, EPSILON100)); // DIFF: 121.87891
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 44, EPSILON100));

    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[1].rect.left(), 422.414f, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[1].rect.top(), -0.268f, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[1].rect.right(), 428.152f, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[1].rect.bottom(), 44, EPSILON100));
}

// Checked DIFF+
DEF_TEST(SkParagraph_ArabicRectsLTRRightAlignParagraph, reporter) {

    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_ArabicRectsLTRRightAlignParagraph.png");
    const char* text = "Helloبمباركة التقليدية قام عن. تصفح يد ";

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    paragraph_style.setMaxLines(14);
    paragraph_style.setTextAlign(TextAlign::kRight);
    paragraph_style.setTextDirection(TextDirection::kLtr);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Noto Naskh Arabic")});
    text_style.setFontSize(26);
    text_style.setWordSpacing(5);
    text_style.setColor(SK_ColorBLACK);
    text_style.setDecoration(TextDecoration::kUnderline);
    text_style.setDecorationColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text);
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

    REPORTER_ASSERT(reporter, boxes.size() == 2ull);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 561.501f, EPSILON100));         // DIFF
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), -0.268f, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 587.741f, EPSILON100));         // DIFF
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 44, EPSILON100));

    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[1].rect.left(), 900, EPSILON100));              // DIFF
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[1].rect.top(), -0.268f, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[1].rect.right(), 905.738f, EPSILON100));        // DIFF
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[1].rect.bottom(), 44, EPSILON100));
}

// Checked: NO DIFF
DEF_TEST(SkParagraph_GetGlyphPositionAtCoordinateParagraph, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_GetGlyphPositionAtCoordinateParagraph.png");
    const char* text =
            "12345 67890 12345 67890 12345 67890 12345 67890 12345 67890 12345 "
            "67890 12345";

    ParagraphStyle paragraphStyle;
    paragraphStyle.setTextAlign(TextAlign::kLeft);
    paragraphStyle.setMaxLines(10);
    paragraphStyle.turnHintingOff();
    TextStyle textStyle;
    textStyle.setFontFamilies({SkString("Roboto")});
    textStyle.setFontStyle(SkFontStyle(SkFontStyle::kNormal_Weight, SkFontStyle::kNormal_Width,
                                   SkFontStyle::kUpright_Slant));
    textStyle.setFontSize(50);
    textStyle.setLetterSpacing(1);
    textStyle.setWordSpacing(5);
    textStyle.setHeight(1);
    textStyle.setColor(SK_ColorBLACK);

    ParagraphBuilderImpl builder(paragraphStyle, fontCollection);
    builder.pushStyle(textStyle);
    builder.addText(text);
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
    REPORTER_ASSERT(reporter,
                    paragraph->getGlyphPositionAtCoordinate(300, 2).position == 11);
    REPORTER_ASSERT(reporter, paragraph->getGlyphPositionAtCoordinate(301, 2.2f).position == 11);
    REPORTER_ASSERT(reporter, paragraph->getGlyphPositionAtCoordinate(302, 2.6f).position == 11);
    REPORTER_ASSERT(reporter, paragraph->getGlyphPositionAtCoordinate(301, 2.1f).position == 11);
    REPORTER_ASSERT(reporter, paragraph->getGlyphPositionAtCoordinate(100000, 20).position == 18);
    REPORTER_ASSERT(reporter, paragraph->getGlyphPositionAtCoordinate(450, 20).position == 16);
    REPORTER_ASSERT(reporter, paragraph->getGlyphPositionAtCoordinate(100000, 90).position == 36);
    REPORTER_ASSERT(reporter, paragraph->getGlyphPositionAtCoordinate(-100000, 90).position == 18);
    REPORTER_ASSERT(reporter,
                    paragraph->getGlyphPositionAtCoordinate(20, -80).position == 1);
    REPORTER_ASSERT(reporter, paragraph->getGlyphPositionAtCoordinate(1, 90).position == 18);
    REPORTER_ASSERT(reporter, paragraph->getGlyphPositionAtCoordinate(1, 170).position == 36);
    REPORTER_ASSERT(reporter, paragraph->getGlyphPositionAtCoordinate(10000, 180).position == 72);
    REPORTER_ASSERT(reporter, paragraph->getGlyphPositionAtCoordinate(70, 180).position == 56);
    REPORTER_ASSERT(reporter, paragraph->getGlyphPositionAtCoordinate(1, 270).position == 72);
    REPORTER_ASSERT(reporter, paragraph->getGlyphPositionAtCoordinate(35, 90).position == 19);
    REPORTER_ASSERT(reporter, paragraph->getGlyphPositionAtCoordinate(10000, 10000).position == 77);
    REPORTER_ASSERT(reporter, paragraph->getGlyphPositionAtCoordinate(85, 10000).position == 75);
}

// Checked: NO DIFF
DEF_TEST(SkParagraph_GetRectsForRangeParagraph, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_GetRectsForRangeParagraph.png");
    const char* text =
            "12345,  \"67890\" 12345 67890 12345 67890 12345 67890 12345 67890 12345 "
            "67890 12345";

    ParagraphStyle paragraphStyle;
    paragraphStyle.setTextAlign(TextAlign::kLeft);
    paragraphStyle.setMaxLines(10);
    paragraphStyle.turnHintingOff();
    TextStyle textStyle;
    textStyle.setFontFamilies({SkString("Roboto")});
    textStyle.setFontSize(50);
    textStyle.setColor(SK_ColorBLACK);
    textStyle.setFontStyle(SkFontStyle(SkFontStyle::kMedium_Weight, SkFontStyle::kNormal_Width,
                                       SkFontStyle::kUpright_Slant));

    ParagraphBuilderImpl builder(paragraphStyle, fontCollection);
    builder.pushStyle(textStyle);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(550);
    paragraph->paint(canvas.get(), 0, 0);

    RectHeightStyle heightStyle = RectHeightStyle::kMax;
    RectWidthStyle widthStyle = RectWidthStyle::kTight;

    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setAntiAlias(true);
    paint.setStrokeWidth(1);

    {
        auto result = paragraph->getRectsForRange(0, 0, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.empty());
    }
    {
        auto result = paragraph->getRectsForRange(0, 1, heightStyle, widthStyle);
        canvas.drawRects(SK_ColorRED, result);
        REPORTER_ASSERT(reporter, result.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 0, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 0.40625f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 28.417f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 59, EPSILON100));
    }
    {
        auto result = paragraph->getRectsForRange(2, 8, heightStyle, widthStyle);
        canvas.drawRects(SK_ColorBLUE, result);
        REPORTER_ASSERT(reporter, result.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 56.835f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 0.40625f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 177.97f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 59, EPSILON100));
    }
    {
        auto result = paragraph->getRectsForRange(8, 21, heightStyle, widthStyle);
        canvas.drawRects(SK_ColorGREEN, result);
        REPORTER_ASSERT(reporter, result.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 177.97f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 0.40625f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 507.031f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 59, EPSILON100));
    }
    {
        auto result = paragraph->getRectsForRange(30, 100, heightStyle, widthStyle);
        canvas.drawRects(SK_ColorRED, result);
        REPORTER_ASSERT(reporter, result.size() == 4);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 211.375f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 59.40625f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 463.623f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 118, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[3].rect.left(), 0, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[3].rect.top(), 236.406f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[3].rect.right(), 142.089f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[3].rect.bottom(), 295, EPSILON100));
    }
    {
        auto result = paragraph->getRectsForRange(19, 22, heightStyle, widthStyle);
        canvas.drawRects(SK_ColorBLUE, result);
        REPORTER_ASSERT(reporter, result.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 450.1875f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 0.40625f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 519.47266f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 59, EPSILON100));
    }
    {
        auto result = paragraph->getRectsForRange(21, 21, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.empty());
    }
}

// Checked: NO DIFF
DEF_TEST(SkParagraph_GetRectsForRangeTight, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_GetRectsForRangeTight.png");
    const char* text =
            "(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)("
            "　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)("
            "　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)";

    ParagraphStyle paragraphStyle;
    paragraphStyle.setTextAlign(TextAlign::kLeft);
    paragraphStyle.setMaxLines(10);
    paragraphStyle.turnHintingOff();
    TextStyle textStyle;
    textStyle.setFontFamilies({SkString("Noto Sans CJK JP")});
    textStyle.setFontSize(50);
    textStyle.setColor(SK_ColorBLACK);
    textStyle.setFontStyle(SkFontStyle(SkFontStyle::kMedium_Weight, SkFontStyle::kNormal_Width,
                                       SkFontStyle::kUpright_Slant));

    ParagraphBuilderImpl builder(paragraphStyle, fontCollection);
    builder.pushStyle(textStyle);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(550);
    paragraph->paint(canvas.get(), 0, 0);

    RectHeightStyle heightStyle = RectHeightStyle::kTight;
    RectWidthStyle widthStyle = RectWidthStyle::kTight;
    {
        auto result = paragraph->getRectsForRange(0, 0, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.empty());
    }
    {
        auto result = paragraph->getRectsForRange(0, 1, heightStyle, widthStyle);
        canvas.drawRects(SK_ColorRED, result);
        REPORTER_ASSERT(reporter, result.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 0, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 0, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 16.898f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 74, EPSILON100));
    }
    {
        auto result = paragraph->getRectsForRange(2, 8, heightStyle, widthStyle);
        canvas.drawRects(SK_ColorBLUE, result);
        REPORTER_ASSERT(reporter, result.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 66.899f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 0, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 264.099f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 74, EPSILON100));
    }
    {
        auto result = paragraph->getRectsForRange(8, 21, heightStyle, widthStyle);
        canvas.drawRects(SK_ColorGREEN, result);
        REPORTER_ASSERT(reporter, result.size() == 2);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 264.099f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 0, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 595.085f, EPSILON50));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 74, EPSILON100));
    }
}

// Checked: DIFF+
DEF_TEST(SkParagraph_GetRectsForRangeIncludeLineSpacingMiddle, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_GetRectsForRangeIncludeLineSpacingMiddle.png");
    const char* text =
            "(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)("
            "　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)("
            "　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)";

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
    textStyle.setFontStyle(SkFontStyle(SkFontStyle::kMedium_Weight, SkFontStyle::kNormal_Width,
                                       SkFontStyle::kUpright_Slant));

    ParagraphBuilderImpl builder(paragraphStyle, fontCollection);
    builder.pushStyle(textStyle);
    builder.addText(text);
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
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 0, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 16.946615f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 17.4296889f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 88.473305f, EPSILON100));
    }
    {
        auto result = paragraph->getRectsForRange(2, 8, heightStyle, widthStyle);
        canvas.drawRects(SK_ColorBLUE, result);
        REPORTER_ASSERT(reporter, result.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 67.429688f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 16.946615f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 190.00781f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 88.473305f, EPSILON100));
    }
    {
        auto result = paragraph->getRectsForRange(8, 21, heightStyle, widthStyle);
        canvas.drawRects(SK_ColorGREEN, result);
        REPORTER_ASSERT(reporter, result.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 190.00781f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 16.946615f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 508.0625f, EPSILON50));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 88.473305f, EPSILON100));
    }
    {
        auto result = paragraph->getRectsForRange(30, 150, heightStyle, widthStyle);
        canvas.drawRects(SK_ColorRED, result);
        REPORTER_ASSERT(reporter, result.size() == 8);

        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 190.00781f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 88.473305f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 525.687f, EPSILON20));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 168.47331f, EPSILON100));

        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[1].rect.left(), 525.687f, EPSILON20));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[1].rect.top(), 88.473305f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[1].rect.right(), 570.02344f, EPSILON20));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[1].rect.bottom(), 168.47331f, EPSILON100));

        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[2].rect.left(), 0, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[2].rect.top(), 168.47331f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[2].rect.right(), 531.574f, EPSILON50));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[2].rect.bottom(), 248.47331f, EPSILON100));

        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[3].rect.left(), 531.574f, EPSILON50));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[3].rect.top(), 168.47331f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[3].rect.right(), 570.02344f, EPSILON20));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[3].rect.bottom(), 248.47331f, EPSILON100));

        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[4].rect.left(), 0, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[4].rect.top(), 248.47331f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[4].rect.right(), 570.02344f, EPSILON20));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[4].rect.bottom(), 328.47333f, EPSILON100));

        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[5].rect.left(), 0, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[5].rect.top(), 328.47333f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[5].rect.right(), 570.02344f, EPSILON20));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[5].rect.bottom(), 408.4733f, EPSILON100));
    }
    {
        auto result = paragraph->getRectsForRange(19, 22, heightStyle, widthStyle);
        canvas.drawRects(SK_ColorBLUE, result);
        REPORTER_ASSERT(reporter, result.size() == 2); // DIFF
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 463.72656f, EPSILON50));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 16.946615f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 530.23047f, EPSILON50));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 88.473305f, EPSILON100));

        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[1].rect.left(), 530.23047f, EPSILON50));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[1].rect.top(), 16.946615f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[1].rect.right(), 570.02344f, EPSILON20));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[1].rect.bottom(), 88.473305f, EPSILON100));
    }
    {
        auto result = paragraph->getRectsForRange(21, 21, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.empty());
    }
}

// Checked: NO DIFF+
DEF_TEST(SkParagraph_GetRectsForRangeIncludeLineSpacingTop, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_GetRectsForRangeIncludeLineSpacingTop.png");
    const char* text =
            "(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)("
            "　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)("
            "　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)";

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
    textStyle.setFontStyle(SkFontStyle(SkFontStyle::kMedium_Weight, SkFontStyle::kNormal_Width,
                                       SkFontStyle::kUpright_Slant));

    ParagraphBuilderImpl builder(paragraphStyle, fontCollection);
    builder.pushStyle(textStyle);
    builder.addText(text);
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
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 0, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 16.946615f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 17.4296889f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 80, EPSILON100));
    }
    {
        auto result = paragraph->getRectsForRange(2, 8, heightStyle, widthStyle);
        canvas.drawRects(SK_ColorBLUE, result);
        REPORTER_ASSERT(reporter, result.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 67.429688f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 16.946615f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 190.00781f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 80, EPSILON100));
    }
    {
        auto result = paragraph->getRectsForRange(8, 21, heightStyle, widthStyle);
        canvas.drawRects(SK_ColorGREEN, result);
        REPORTER_ASSERT(reporter, result.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 190.00781f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 16.946615f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 508.0625f, EPSILON50));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 80, EPSILON100));
    }
    {
        auto result = paragraph->getRectsForRange(30, 150, heightStyle, widthStyle);
        canvas.drawRects(SK_ColorMAGENTA, result);
        REPORTER_ASSERT(reporter, result.size() == 8);

        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 190.00781f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 80, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 525.687f, EPSILON20));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 160, EPSILON100));

        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[1].rect.left(), 525.687f, EPSILON20));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[1].rect.top(), 80, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[1].rect.right(), 570.02344f, EPSILON20));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[1].rect.bottom(), 160, EPSILON100));

        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[2].rect.left(), 0, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[2].rect.top(), 160, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[2].rect.right(), 531.574f, EPSILON20));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[2].rect.bottom(), 240, EPSILON100));

        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[3].rect.left(), 531.574f, EPSILON20));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[3].rect.top(), 160, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[3].rect.right(), 570.02344f, EPSILON20));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[3].rect.bottom(), 240, EPSILON100));

        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[4].rect.left(), 0, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[4].rect.top(), 240, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[4].rect.right(), 570.02344f, EPSILON20));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[4].rect.bottom(), 320, EPSILON100));

        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[5].rect.left(), 0, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[5].rect.top(), 320, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[5].rect.right(), 570.02344f, EPSILON20));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[5].rect.bottom(), 400, EPSILON100));
    }
    {
        auto result = paragraph->getRectsForRange(19, 22, heightStyle, widthStyle);
        canvas.drawRects(SK_ColorBLACK, result);
        REPORTER_ASSERT(reporter, result.size() == 2); // DIFF
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 463.72656f, EPSILON50));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 16.946615f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 530.23047f, EPSILON50));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 80, EPSILON100));

        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[1].rect.left(), 530.23047f, EPSILON50));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[1].rect.top(), 16.946615f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[1].rect.right(), 570.02344f, EPSILON20));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[1].rect.bottom(), 80, EPSILON100));
    }
    {
        auto result = paragraph->getRectsForRange(21, 21, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.empty());
    }
}

// Checked: NO DIFF+
DEF_TEST(SkParagraph_GetRectsForRangeIncludeLineSpacingBottom, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_GetRectsForRangeIncludeLineSpacingBottom.png");
    const char* text =
            "(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)("
            "　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)("
            "　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)";

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
    textStyle.setFontStyle(SkFontStyle(SkFontStyle::kMedium_Weight, SkFontStyle::kNormal_Width,
                                       SkFontStyle::kUpright_Slant));

    ParagraphBuilderImpl builder(paragraphStyle, fontCollection);
    builder.pushStyle(textStyle);
    builder.addText(text);
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
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 0, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 16.946f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 17.429f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 96.946f, EPSILON100));
    }
    {
        auto result = paragraph->getRectsForRange(2, 8, heightStyle, widthStyle);
        canvas.drawRects(SK_ColorBLUE, result);
        REPORTER_ASSERT(reporter, result.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 67.4298f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 16.946f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 190.007f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 96.946f, EPSILON100));
    }
    {
        auto result = paragraph->getRectsForRange(8, 21, heightStyle, widthStyle);
        canvas.drawRects(SK_ColorGREEN, result);
        REPORTER_ASSERT(reporter, result.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 190.007f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 16.946f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 508.062f, EPSILON50));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 96.946f, EPSILON100));
    }
    {
        auto result = paragraph->getRectsForRange(30, 150, heightStyle, widthStyle);
        canvas.drawRects(SK_ColorMAGENTA, result);
        REPORTER_ASSERT(reporter, result.size() == 8);

        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 190.007f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 96.946f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 525.687f, EPSILON20));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 176.946f, EPSILON100));

        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[1].rect.left(), 525.687f, EPSILON20));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[1].rect.top(), 96.946f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[1].rect.right(), 570.023f, EPSILON20));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[1].rect.bottom(), 176.946f, EPSILON100));

        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[2].rect.left(), 0, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[2].rect.top(), 176.946f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[2].rect.right(), 531.574f, EPSILON20));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[2].rect.bottom(), 256.946f, EPSILON100));

        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[3].rect.left(), 531.574f, EPSILON20));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[3].rect.top(), 176.946f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[3].rect.right(), 570.023f, EPSILON20));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[3].rect.bottom(), 256.946f, EPSILON100));

        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[4].rect.left(), 0, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[4].rect.top(), 256.946f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[4].rect.right(), 570.023f, EPSILON20));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[4].rect.bottom(), 336.946f, EPSILON100));

        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[5].rect.left(), 0, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[5].rect.top(), 336.946f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[5].rect.right(), 570.023f, EPSILON20));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[5].rect.bottom(), 416.946f, EPSILON100));
    }
    {
        auto result = paragraph->getRectsForRange(19, 22, heightStyle, widthStyle);
        canvas.drawRects(SK_ColorBLACK, result);
        REPORTER_ASSERT(reporter, result.size() == 2); // DIFF
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 463.726f, EPSILON50));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 16.946f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 530.230f, EPSILON50));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 96.946f, EPSILON100));

        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[1].rect.left(), 530.230f, EPSILON50));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[1].rect.top(), 16.946f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[1].rect.right(), 570.023f, EPSILON20));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[1].rect.bottom(), 96.946f, EPSILON100));
    }
    {
        auto result = paragraph->getRectsForRange(21, 21, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.empty());
    }
}

// Checked: NO DIFF
DEF_TEST(SkParagraph_GetRectsForRangeIncludeCombiningCharacter, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_GetRectsForRangeIncludeCombiningCharacter.png");
    const char* text = "ดีสวัสดีชาวโลกที่น่ารัก";
    ParagraphStyle paragraphStyle;
    paragraphStyle.setTextAlign(TextAlign::kLeft);
    paragraphStyle.setMaxLines(10);
    paragraphStyle.turnHintingOff();
    ParagraphBuilderImpl builder(paragraphStyle, fontCollection);

    TextStyle textStyle;
    textStyle.setFontFamilies({SkString("Roboto")});
    textStyle.setFontSize(50);
    textStyle.setLetterSpacing(1);
    textStyle.setWordSpacing(5);
    textStyle.setHeight(1);
    textStyle.setColor(SK_ColorBLACK);

    builder.pushStyle(textStyle);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth - 100);
    paragraph->paint(canvas.get(), 0, 0);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, impl->lines().size() == 1);

    RectHeightStyle heightStyle = RectHeightStyle::kTight;
    RectWidthStyle widthStyle = RectWidthStyle::kTight;
    {
        auto result = paragraph->getRectsForRange(0, 0, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.empty());
    }
    {
        auto first = paragraph->getRectsForRange(0, 1, heightStyle, widthStyle);
        auto second = paragraph->getRectsForRange(1, 2, heightStyle, widthStyle);
        auto last = paragraph->getRectsForRange(0, 2, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, first.size() == 0 && second.size() == 1 && last.size() == 1);
        REPORTER_ASSERT(reporter, second[0].rect == last[0].rect);
    }
    {
        auto first = paragraph->getRectsForRange(3, 4, heightStyle, widthStyle);
        auto second = paragraph->getRectsForRange(4, 5, heightStyle, widthStyle);
        auto last = paragraph->getRectsForRange(3, 5, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, first.size() == 0 && second.size() == 1 && last.size() == 1);
        REPORTER_ASSERT(reporter, second[0].rect == last[0].rect);
    }
    {
        auto first = paragraph->getRectsForRange(14, 15, heightStyle, widthStyle);
        auto second = paragraph->getRectsForRange(15, 16, heightStyle, widthStyle);
        auto third = paragraph->getRectsForRange(16, 17, heightStyle, widthStyle);
        auto last = paragraph->getRectsForRange(14, 17, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, first.size() == 0 && second.size() == 0 &&
                                  third.size() == 1 && last.size() == 1);
        REPORTER_ASSERT(reporter, third[0].rect == last[0].rect);
    }
}

// Checked: NO DIFF
DEF_TEST(SkParagraph_GetRectsForRangeCenterParagraph, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_GetRectsForRangeCenterParagraph.png");
    // Minikin uses a hard coded list of unicode characters that he treats as invisible - as spaces.
    // It's absolutely wrong - invisibility is a glyph attribute, not character/grapheme.
    // Any attempt to substitute one for another leads to errors
    // (for instance, some fonts can use these hard coded characters for something that is visible)
    const char* text = "01234  　 ";   // includes ideographic space and english space.

    ParagraphStyle paragraphStyle;
    paragraphStyle.setTextAlign(TextAlign::kCenter);
    paragraphStyle.setMaxLines(10);
    paragraphStyle.turnHintingOff();
    ParagraphBuilderImpl builder(paragraphStyle, fontCollection);

    TextStyle textStyle;
    textStyle.setFontFamilies({SkString("Roboto")});
    textStyle.setFontSize(50);
    textStyle.setHeight(1);
    textStyle.setColor(SK_ColorBLACK);
    textStyle.setFontStyle(SkFontStyle(SkFontStyle::kMedium_Weight, SkFontStyle::kNormal_Width,
                                       SkFontStyle::kUpright_Slant));

    builder.pushStyle(textStyle);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(550);
    paragraph->paint(canvas.get(), 0, 0);

    // Some of the formatting lazily done on paint
    RectHeightStyle heightStyle = RectHeightStyle::kMax;
    RectWidthStyle widthStyle = RectWidthStyle::kTight;
    {
        auto result = paragraph->getRectsForRange(0, 0, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.empty());
    }

    {
        auto result = paragraph->getRectsForRange(0, 1, heightStyle, widthStyle);
        canvas.drawRects(SK_ColorRED, result);
        REPORTER_ASSERT(reporter, result.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 203.955f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 0.40625f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 232.373f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 59, EPSILON100));
    }

    {
        auto result = paragraph->getRectsForRange(2, 4, heightStyle, widthStyle);
        canvas.drawRects(SK_ColorBLUE, result);
        REPORTER_ASSERT(reporter, result.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 260.791f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 0.40625f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 317.626f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 59, EPSILON100));
    }

    {
        auto result = paragraph->getRectsForRange(4, 5, heightStyle, widthStyle);
        canvas.drawRects(SK_ColorGREEN, result);
        REPORTER_ASSERT(reporter, result.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 317.626f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 0.40625f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 346.044f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 59, EPSILON100));
    }

    {
        auto result = paragraph->getRectsForRange(4, 6, heightStyle, widthStyle);
        canvas.drawRects(SK_ColorBLACK, result);
        REPORTER_ASSERT(reporter, result.size() == 1); // DIFF
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 317.626f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 0.40625f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 358.494f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 59, EPSILON100));
    }

    {
        auto result = paragraph->getRectsForRange(5, 6, heightStyle, widthStyle);
        canvas.drawRects(SK_ColorRED, result);
        REPORTER_ASSERT(reporter, result.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 346.044f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 0.40625f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 358.494f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 59, EPSILON100));
    }

    {
        auto result = paragraph->getRectsForRange(21, 21, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.empty());
    }
}

// Checked DIFF+
DEF_TEST(SkParagraph_GetRectsForRangeCenterParagraphNewlineCentered, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_GetRectsForRangeCenterParagraphNewlineCentered.png");
    const char* text = "01234\n";

    ParagraphStyle paragraphStyle;
    paragraphStyle.setTextAlign(TextAlign::kCenter);
    paragraphStyle.setMaxLines(10);
    paragraphStyle.turnHintingOff();
    ParagraphBuilderImpl builder(paragraphStyle, fontCollection);

    TextStyle textStyle;
    textStyle.setFontFamilies({SkString("Roboto")});
    textStyle.setFontSize(50);
    textStyle.setHeight(1);
    textStyle.setColor(SK_ColorBLACK);
    textStyle.setFontStyle(SkFontStyle(SkFontStyle::kMedium_Weight, SkFontStyle::kNormal_Width,
                                       SkFontStyle::kUpright_Slant));

    builder.pushStyle(textStyle);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(550);

    paragraph->paint(canvas.get(), 0, 0);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, impl->lines().size() == 2);

    RectHeightStyle heightStyle = RectHeightStyle::kMax;
    RectWidthStyle widthStyle = RectWidthStyle::kTight;
    {
        auto result = paragraph->getRectsForRange(0, 0, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.empty());
    }

    {
        auto result = paragraph->getRectsForRange(0, 1, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 203.955f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 0.40625f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 232.373f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 59, EPSILON100));
    }

    {
        auto result = paragraph->getRectsForRange(6, 7, heightStyle, widthStyle);
        canvas.drawRects(SK_ColorBLUE, result);
        REPORTER_ASSERT(reporter, result.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 275.0f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 59.406f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 275.0f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 118, EPSILON100));
    }
}

// Checked NO DIFF
DEF_TEST(SkParagraph_GetRectsForRangeCenterMultiLineParagraph, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_GetRectsForRangeCenterMultiLineParagraph.png");
    const char* text = "01234  　 \n0123　        "; // includes ideographic space and english space.

    ParagraphStyle paragraphStyle;
    paragraphStyle.setTextAlign(TextAlign::kCenter);
    paragraphStyle.setMaxLines(10);
    paragraphStyle.turnHintingOff();
    ParagraphBuilderImpl builder(paragraphStyle, fontCollection);

    TextStyle textStyle;
    textStyle.setFontFamilies({SkString("Roboto")});
    textStyle.setFontSize(50);
    textStyle.setHeight(1);
    textStyle.setColor(SK_ColorBLACK);
    textStyle.setFontStyle(SkFontStyle(SkFontStyle::kMedium_Weight, SkFontStyle::kNormal_Width,
                                       SkFontStyle::kUpright_Slant));

    builder.pushStyle(textStyle);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(550);

    paragraph->paint(canvas.get(), 0, 0);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());

    REPORTER_ASSERT(reporter, impl->lines().size() == 2);

    RectHeightStyle heightStyle = RectHeightStyle::kMax;
    RectWidthStyle widthStyle = RectWidthStyle::kTight;
    SkScalar epsilon = 0.01f;
    {
        auto result = paragraph->getRectsForRange(0, 0, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.empty());
    }
    {
        auto result = paragraph->getRectsForRange(0, 1, heightStyle, widthStyle);
        canvas.drawRects(SK_ColorRED, result);
        REPORTER_ASSERT(reporter, result.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 203.955f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 0.40625f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 232.373f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 59, epsilon));
    }
    {
        auto result = paragraph->getRectsForRange(2, 4, heightStyle, widthStyle);
        canvas.drawRects(SK_ColorBLUE, result);
        REPORTER_ASSERT(reporter, result.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 260.791f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 0.40625f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 317.626f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 59, epsilon));
    }
    {
        auto result = paragraph->getRectsForRange(4, 6, heightStyle, widthStyle);
        canvas.drawRects(SK_ColorGREEN, result);
        REPORTER_ASSERT(reporter, result.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 317.626f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 0.40625f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 358.494f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 59, epsilon));
    }
    {
        auto result = paragraph->getRectsForRange(5, 6, heightStyle, widthStyle);
        canvas.drawRects(SK_ColorYELLOW, result);
        REPORTER_ASSERT(reporter, result.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 346.044f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 0.40625f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 358.494f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 59, epsilon));
    }
    {
        auto result = paragraph->getRectsForRange(10, 12, heightStyle, widthStyle);
        canvas.drawRects(SK_ColorCYAN, result);
        REPORTER_ASSERT(reporter, result.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 218.164f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 59.40625f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 275, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 118, epsilon));
    }
    {
        auto result = paragraph->getRectsForRange(14, 18, heightStyle, widthStyle);
        canvas.drawRects(SK_ColorBLACK, result);
        REPORTER_ASSERT(reporter, result.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 331.835f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 59.40625f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 419.189f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 118, epsilon));
    }
    {
        auto result = paragraph->getRectsForRange(21, 21, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.empty());
    }
}

// Checked: DIFF (line height rounding error)
DEF_TEST(SkParagraph_GetRectsForRangeStrut, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_GetRectsForRangeStrut.png");
    const char* text = "Chinese 字典";

    StrutStyle strutStyle;
    strutStyle.setStrutEnabled(true);
    strutStyle.setFontFamilies({SkString("Roboto")});
    strutStyle.setFontSize(14.0);

    ParagraphStyle paragraphStyle;
    paragraphStyle.setStrutStyle(strutStyle);

    TextStyle textStyle;
    textStyle.setFontFamilies({SkString("Noto Sans CJK JP")});
    textStyle.setFontSize(20);
    textStyle.setColor(SK_ColorBLACK);

    ParagraphBuilderImpl builder(paragraphStyle, fontCollection);
    builder.pushStyle(textStyle);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(550);
    paragraph->paint(canvas.get(), 0, 0);

    {
        auto result = paragraph->getRectsForRange(0, 10, RectHeightStyle::kTight, RectWidthStyle::kMax);
        canvas.drawRects(SK_ColorGREEN, result);
        REPORTER_ASSERT(reporter, result.size() == 1);
    }

    {
        auto result = paragraph->getRectsForRange(0, 10, RectHeightStyle::kStrut, RectWidthStyle::kMax);
        canvas.drawRects(SK_ColorRED, result);
        REPORTER_ASSERT(reporter, result.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 0, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 10.611f, EPSILON2));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 118.605f, EPSILON50));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 27.017f, EPSILON2));
    }
}

// Checked: NO DIFF
DEF_TEST(SkParagraph_GetRectsForRangeStrutFallback, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_GetRectsForRangeStrutFallback.png");
    const char* text = "Chinese 字典";

    StrutStyle strutStyle;
    strutStyle.setStrutEnabled(false);

    ParagraphStyle paragraphStyle;
    paragraphStyle.setStrutStyle(strutStyle);

    TextStyle textStyle;
    textStyle.setFontFamilies({SkString("Noto Sans CJK JP")});
    textStyle.setFontSize(20);
    textStyle.setColor(SK_ColorBLACK);

    ParagraphBuilderImpl builder(paragraphStyle, fontCollection);
    builder.pushStyle(textStyle);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(550);
    paragraph->paint(canvas.get(), 0, 0);


    auto result1 = paragraph->getRectsForRange(0, 10, RectHeightStyle::kTight, RectWidthStyle::kMax);
    canvas.drawRects(SK_ColorGREEN, result1);
    REPORTER_ASSERT(reporter, result1.size() == 1);

    auto result2 = paragraph->getRectsForRange(0, 10, RectHeightStyle::kStrut, RectWidthStyle::kMax);
    canvas.drawRects(SK_ColorRED, result2);
    REPORTER_ASSERT(reporter, result2.size() == 1);

    REPORTER_ASSERT(reporter, result1[0].rect == result2[0].rect);
}

// Checked: DIFF (small in numbers)
DEF_TEST(SkParagraph_GetWordBoundaryParagraph, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_GetWordBoundaryParagraph.png");
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

    ParagraphBuilderImpl builder(paragraphStyle, fontCollection);
    builder.pushStyle(textStyle);
    builder.addText(
            "12345  67890 12345 67890 12345 67890 12345 67890 12345 67890 12345 67890 12345");
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

    auto len = static_cast<ParagraphImpl*>(paragraph.get())->text().size();
    REPORTER_ASSERT(reporter, paragraph->getWordBoundary(len - 1) == SkRange<size_t>(len - 5, len));
}

// Checked: DIFF (unclear)
DEF_TEST(SkParagraph_SpacingParagraph, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_SpacingParagraph.png");
    ParagraphStyle paragraph_style;
    paragraph_style.setMaxLines(10);
    paragraph_style.setTextAlign(TextAlign::kLeft);
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setFontSize(50);
    text_style.setLetterSpacing(20);
    text_style.setWordSpacing(0);
    text_style.setColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText("H");
    builder.pop();

    text_style.setLetterSpacing(10);
    text_style.setWordSpacing(0);
    builder.pushStyle(text_style);
    builder.addText("H");
    builder.pop();

    text_style.setLetterSpacing(20);
    text_style.setWordSpacing(0);
    builder.pushStyle(text_style);
    builder.addText("H");
    builder.pop();

    text_style.setLetterSpacing(0);
    text_style.setWordSpacing(0);
    builder.pushStyle(text_style);
    builder.addText("|");
    builder.pop();

    text_style.setLetterSpacing(0);
    text_style.setWordSpacing(20);
    builder.pushStyle(text_style);
    builder.addText("H ");
    builder.pop();

    text_style.setLetterSpacing(0);
    text_style.setWordSpacing(0);
    builder.pushStyle(text_style);
    builder.addText("H ");
    builder.pop();

    text_style.setLetterSpacing(0);
    text_style.setLetterSpacing(0);
    text_style.setWordSpacing(20);
    builder.pushStyle(text_style);
    builder.addText("H ");
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(550);
    paragraph->paint(canvas.get(), 0, 0);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, impl->lines().size() == 1);
    size_t index = 0;
    impl->lines().begin()->scanStyles(StyleType::kLetterSpacing,
                                      [&index](TextRange text, TextStyle style, SkScalar) {
                                          ++index;
                                          return true;
                                      });
    REPORTER_ASSERT(reporter, index == 4);
    index = 0;
    impl->lines().begin()->scanStyles(StyleType::kWordSpacing,
                                      [&index](TextRange text, TextStyle style, SkScalar) {
                                          ++index;
                                          return true;
                                      });
    REPORTER_ASSERT(reporter, index == 4);
}

// Checked: NO DIFF
DEF_TEST(SkParagraph_LongWordParagraph, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_LongWordParagraph.png");
    const char* text =
            "A "
            "veryverylongwordtoseewherethiswillwraporifitwillatallandifitdoesthenthat"
            "wouldbeagoodthingbecausethebreakingisworking.";

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setColor(SK_ColorRED);
    text_style.setFontSize(31);
    text_style.setLetterSpacing(0);
    text_style.setWordSpacing(0);
    text_style.setColor(SK_ColorBLACK);
    text_style.setHeight(1);
    builder.pushStyle(text_style);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth / 2);
    paragraph->paint(canvas.get(), 0, 0);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, impl->text().size() == std::string{text}.length());
    REPORTER_ASSERT(reporter, impl->runs().size() == 1);
    REPORTER_ASSERT(reporter, impl->styles().size() == 1);
    REPORTER_ASSERT(reporter, impl->styles()[0].fStyle.equals(text_style));
    REPORTER_ASSERT(reporter, impl->lines().size() == 4);

    REPORTER_ASSERT(reporter, impl->lines()[0].width() > TestCanvasWidth / 2 - 20);
    REPORTER_ASSERT(reporter, impl->lines()[1].width() > TestCanvasWidth / 2 - 20);
    REPORTER_ASSERT(reporter, impl->lines()[2].width() > TestCanvasWidth / 2 - 20);
}

// Checked: DIFF?
DEF_TEST(SkParagraph_KernScaleParagraph, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_KernScaleParagraph.png");

    const char* text1 = "AVAVAWAH A0 V0 VA To The Lo";
    const char* text2 = " Dialog Text List lots of words to see "
                        "if kerning works on a bigger set of characters AVAVAW";
    float scale = 3.0f;
    ParagraphStyle paragraph_style;
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);
    TextStyle text_style;
    text_style.setFontFamilies({SkString("Droid Serif")});
    text_style.setFontSize(100 / scale);
    text_style.setColor(SK_ColorBLACK);

    builder.pushStyle(text_style);
    builder.addText(text1);
    builder.pushStyle(text_style);
    builder.addText("A");
    builder.pushStyle(text_style);
    builder.addText("V");
    text_style.setFontSize(14 / scale);
    builder.pushStyle(text_style);
    builder.addText(text2);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth / scale);
    canvas.get()->scale(scale, scale);
    paragraph->paint(canvas.get(), 0, 0);
    canvas.get()->scale(1, 1);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());

    // First and second lines must have the same width, the third one must be bigger
    REPORTER_ASSERT(reporter, impl->lines().size() == 3);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(impl->lines()[0].width(), 285.858f, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(impl->lines()[1].width(), 329.709f, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(impl->lines()[2].width(), 120.619f, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(impl->lines()[0].height(), 39.00f, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(impl->lines()[1].height(), 39.00f, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(impl->lines()[2].height(), 05.00f, EPSILON100));
}

// Checked: DIFF+
DEF_TEST(SkParagraph_NewlineParagraph, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_NewlineParagraph.png");
    const char* text =
            "line1\nline2 test1 test2 test3 test4 test5 test6 test7\nline3\n\nline4 "
            "test1 test2 test3 test4";
    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setColor(SK_ColorRED);
    text_style.setFontSize(60);
    text_style.setColor(SK_ColorBLACK);
    text_style.setHeight(1);
    builder.pushStyle(text_style);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth - 300);
    paragraph->paint(canvas.get(), 0, 0);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    // Minikin does not count empty lines but SkParagraph does
    REPORTER_ASSERT(reporter, impl->lines().size() == 7);

    REPORTER_ASSERT(reporter, impl->lines()[0].offset().fY == 0);
    REPORTER_ASSERT(reporter, impl->lines()[1].offset().fY == 70);
    REPORTER_ASSERT(reporter, impl->lines()[2].offset().fY == 140);
    REPORTER_ASSERT(reporter, impl->lines()[3].offset().fY == 210);
    REPORTER_ASSERT(reporter, impl->lines()[4].offset().fY == 280);  // Empty line
    REPORTER_ASSERT(reporter, impl->lines()[5].offset().fY == 350);
    REPORTER_ASSERT(reporter, impl->lines()[6].offset().fY == 420);
}

// Checked: DIFF? (underline)
DEF_TEST(SkParagraph_EmojiParagraph, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_EmojiParagraph.png");
  const char* text =
      "😀😃😄😁😆😅😂🤣☺😇🙂😍😡😟😢😻👽💩👍👎🙏👌👋👄👁👦👼👨‍🚀👨‍🚒🙋‍♂️👳👨‍👨‍👧‍👧\
      💼👡👠☂🐶🐰🐻🐼🐷🐒🐵🐔🐧🐦🐋🐟🐡🕸🐌🐴🐊🐄🐪🐘🌸🌏🔥🌟🌚🌝💦💧\
      ❄🍕🍔🍟🥝🍱🕶🎩🏈⚽🚴‍♀️🎻🎼🎹🚨🚎🚐⚓🛳🚀🚁🏪🏢🖱⏰📱💾💉📉🛏🔑🔓\
      📁🗓📊❤💯🚫🔻♠♣🕓❗🏳🏁🏳️‍🌈🇮🇹🇱🇷🇺🇸🇬🇧🇨🇳🇧🇴";

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Noto Color Emoji")});
    text_style.setFontSize(50);
    text_style.setDecoration(TextDecoration::kUnderline);
    text_style.setColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);
    paragraph->paint(canvas.get(), 0, 0);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());

    REPORTER_ASSERT(reporter, impl->lines().size() == 8);
    for (auto& line : impl->lines()) {
        if (&line != impl->lines().end() - 1) {
            REPORTER_ASSERT(reporter, line.width() == 998.25f);
        } else {
            REPORTER_ASSERT(reporter, line.width() < 998.25f);
        }
        REPORTER_ASSERT(reporter, line.height() == 59);
    }
}

// Checked: DIFF+
DEF_TEST(SkParagraph_EmojiMultiLineRectsParagraph, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_EmojiMultiLineRectsParagraph.png");
  const char* text =
      "👩‍👩‍👦👩‍👩‍👧‍👧🇺🇸👩‍👩‍👦👩‍👩‍👧‍👧i🇺🇸👩‍👩‍👦👩‍👩‍👧‍👧🇺🇸👩‍👩‍👦👩‍👩‍👧‍👧🇺🇸"
      "👩‍👩‍👦👩‍👩‍👧‍👧🇺🇸👩‍👩‍👦👩‍👩‍👧‍👧🇺🇸👩‍👩‍👦👩‍👩‍👧‍👧🇺🇸👩‍👩‍👦👩‍👩‍👧‍👧🇺🇸"
      "👩‍👩‍👦👩‍👩‍👧‍👧🇺🇸👩‍👩‍👦👩‍👩‍👧‍👧🇺🇸👩‍👩‍👦👩‍👩‍👧‍👧🇺🇸👩‍👩‍👦👩‍👩‍👧‍👧🇺🇸"
      "👩‍👩‍👦👩‍👩‍👧‍👧🇺🇸👩‍👩‍👦👩‍👩‍👧‍👧🇺🇸👩‍👩‍👦👩‍👩‍👧‍👧🇺🇸👩‍👩‍👦👩‍👩‍👧‍👧🇺🇸"
      "❄🍕🍔🍟🥝🍱🕶🎩🏈⚽🚴‍♀️🎻🎼🎹🚨🚎🚐⚓🛳🚀🚁🏪🏢🖱⏰📱💾💉📉🛏🔑🔓"
      "📁🗓📊❤💯🚫🔻♠♣🕓❗🏳🏁🏳️‍🌈🇮🇹🇱🇷🇺🇸🇬🇧🇨🇳🇧🇴";

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Noto Color Emoji")});
    text_style.setFontSize(50);
    text_style.setColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth - 300);
    paragraph->paint(canvas.get(), 0, 0);

    RectHeightStyle rect_height_style = RectHeightStyle::kTight;
    RectWidthStyle rect_width_style = RectWidthStyle::kTight;

    auto result = paragraph->getRectsForRange(0, 0, rect_height_style, rect_width_style);
    REPORTER_ASSERT(reporter, result.size() == 0);

    result = paragraph->getRectsForRange(0, 119, rect_height_style, rect_width_style);
    REPORTER_ASSERT(reporter, result.size() == 2);
    canvas.drawRects(SK_ColorRED, result);

    result = paragraph->getRectsForRange(122, 132, rect_height_style, rect_width_style);
    REPORTER_ASSERT(reporter, result.size() == 1);
    canvas.drawRects(SK_ColorBLUE, result);

    auto pos = paragraph->getGlyphPositionAtCoordinate(610, 100).position;
    result = paragraph->getRectsForRange(0, pos, rect_height_style, rect_width_style);
    REPORTER_ASSERT(reporter, result.size() == 2);
    canvas.drawRects(SK_ColorGREEN, result);

    pos = paragraph->getGlyphPositionAtCoordinate(580, 100).position;
    result = paragraph->getRectsForRange(0, pos, rect_height_style, rect_width_style);
    REPORTER_ASSERT(reporter, result.size() == 2);
    canvas.drawRects(SK_ColorGREEN, result);

    pos = paragraph->getGlyphPositionAtCoordinate(560, 100).position;
    result = paragraph->getRectsForRange(0, pos, rect_height_style, rect_width_style);
    REPORTER_ASSERT(reporter, result.size() == 2);
    canvas.drawRects(SK_ColorGREEN, result);
}

DEF_TEST(SkParagraph_HyphenBreakParagraph, reporter) {
    SkDebugf("Hyphens are not implemented, and will not be implemented soon.\n");
}

// Checked: DIFF (line breaking)
DEF_TEST(SkParagraph_RepeatLayoutParagraph, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_RepeatLayoutParagraph.png");
    const char* text =
            "Sentence to layout at diff widths to get diff line counts. short words "
            "short words short words short words short words short words short words "
            "short words short words short words short words short words short words "
            "end";

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setFontSize(31);
    text_style.setColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(300);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    // Some of the formatting lazily done on paint
    REPORTER_ASSERT(reporter, impl->runs().size() == 1);
    REPORTER_ASSERT(reporter, impl->styles().size() == 1);
    REPORTER_ASSERT(reporter, impl->lines().size() == 12);

    paragraph->layout(600);
    paragraph->paint(canvas.get(), 0, 0);
    REPORTER_ASSERT(reporter, impl->runs().size() == 1);
    REPORTER_ASSERT(reporter, impl->styles().size() == 1);
    REPORTER_ASSERT(reporter, impl->lines().size() == 6);
}

// Checked: NO DIFF
DEF_TEST(SkParagraph_Ellipsize, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_Ellipsize.png");
    const char* text =
            "This is a very long sentence to test if the text will properly wrap "
            "around and go to the next line. Sometimes, short sentence. Longer "
            "sentences are okay too because they are nessecary. Very short. ";

    ParagraphStyle paragraph_style;
    paragraph_style.setMaxLines(1);
    paragraph_style.setEllipsis(u"\u2026");
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);
    paragraph->paint(canvas.get(), 0, 0);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());

    // Check that the ellipsizer limited the text to one line and did not wrap to a second line.
    REPORTER_ASSERT(reporter, impl->lines().size() == 1);

    auto& line = impl->lines()[0];
    REPORTER_ASSERT(reporter, line.ellipsis() != nullptr);
    size_t index = 0;
    line.scanRuns([&index, &line, reporter](Run* run, int32_t, size_t, TextRange, SkRect, SkScalar, bool) {
        ++index;
        if (index == 2) {
            REPORTER_ASSERT(reporter, run->textRange() == line.ellipsis()->textRange());
        }
        return true;
    });
    REPORTER_ASSERT(reporter, index == 2);
}

// Checked: NO DIFF
DEF_TEST(SkParagraph_UnderlineShiftParagraph, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_UnderlineShiftParagraph.png");
    const char* text1 = "fluttser ";
    const char* text2 = "mdje";
    const char* text3 = "fluttser mdje";

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    paragraph_style.setTextAlign(TextAlign::kLeft);
    paragraph_style.setMaxLines(2);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text1);
    text_style.setDecoration(TextDecoration::kUnderline);
    text_style.setDecorationColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text2);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);
    paragraph->paint(canvas.get(), 0, 0);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());

    ParagraphBuilderImpl builder1(paragraph_style, fontCollection);
    text_style.setDecoration(TextDecoration::kNoDecoration);
    builder1.pushStyle(text_style);
    builder1.addText(text3);
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
        auto r =
                paragraph->getRectsForRange(i, i + 1, RectHeightStyle::kMax, RectWidthStyle::kTight)
                        .front()
                        .rect;
        auto r1 =
                paragraph1
                        ->getRectsForRange(i, i + 1, RectHeightStyle::kMax, RectWidthStyle::kTight)
                        .front()
                        .rect;

        REPORTER_ASSERT(reporter, r.fLeft == r1.fLeft);
        REPORTER_ASSERT(reporter, r.fRight == r1.fRight);
    }
}

// Checked: NO DIFF
DEF_TEST(SkParagraph_SimpleShadow, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_SimpleShadow.png");
    const char* text = "Hello World Text Dialog";

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setColor(SK_ColorBLACK);
    text_style.addShadow(TextShadow(SK_ColorBLACK, SkPoint::Make(2.0f, 2.0f), 1.0));
    builder.pushStyle(text_style);
    builder.addText(text);

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);
    paragraph->paint(canvas.get(), 10.0, 15.0);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());

    REPORTER_ASSERT(reporter, impl->runs().size() == 1);
    REPORTER_ASSERT(reporter, impl->styles().size() == 1);
    size_t index = 0;
    for (auto& line : impl->lines()) {
        line.scanStyles(StyleType::kShadow,
            [&index, text_style, reporter](TextRange text, TextStyle style, SkScalar) {
                REPORTER_ASSERT(reporter, index == 0 && style.equals(text_style));
                ++index;
                return true;
            });
    }
}

// Checked: NO DIFF
DEF_TEST(SkParagraph_ComplexShadow, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_ComplexShadow.png");
    const char* text = "Text Chunk ";

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setColor(SK_ColorBLACK);
    text_style.addShadow(TextShadow(SK_ColorBLACK, SkPoint::Make(2.0f, 2.0f), 1.0f));
    builder.pushStyle(text_style);
    builder.addText(text);

    text_style.addShadow(TextShadow(SK_ColorRED, SkPoint::Make(2.0f, 2.0f), 5.0f));
    text_style.addShadow(TextShadow(SK_ColorGREEN, SkPoint::Make(10.0f, -5.0f), 3.0f));
    builder.pushStyle(text_style);
    builder.addText(text);
    builder.pop();

    builder.addText(text);

    text_style.addShadow(TextShadow(SK_ColorRED, SkPoint::Make(0.0f, 1.0f), 0.0f));
    builder.pushStyle(text_style);
    builder.addText(text);
    builder.pop();

    builder.addText(text);

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);
    paragraph->paint(canvas.get(), 10.0, 15.0);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());

    size_t index = 0;
    for (auto& line : impl->lines()) {
        line.scanStyles(StyleType::kShadow,
            [&index, text_style, reporter](TextRange text, TextStyle style, SkScalar) {
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

// Checked: NO DIFF
DEF_TEST(SkParagraph_BaselineParagraph, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_BaselineParagraph.png");
    const char* text =
            "左線読設Byg後碁給能上目秘使約。満毎冠行来昼本可必図将発確年。今属場育"
            "図情闘陰野高備込制詩西校客。審対江置講今固残必託地集済決維駆年策。立得";

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    paragraph_style.setMaxLines(14);
    paragraph_style.setTextAlign(TextAlign::kJustify);
    paragraph_style.setHeight(1.5);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Source Han Serif CN")});
    text_style.setColor(SK_ColorBLACK);
    text_style.setFontSize(55);
    text_style.setLetterSpacing(2);
    text_style.setDecorationStyle(TextDecorationStyle::kSolid);
    text_style.setDecorationColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth - 100);
    paragraph->paint(canvas.get(), 0, 0);

    SkRect rect1 = SkRect::MakeXYWH(0, paragraph->getIdeographicBaseline(),
                                       paragraph->getMaxWidth(),
                                       paragraph->getIdeographicBaseline());
    SkRect rect2 = SkRect::MakeXYWH(0, paragraph->getAlphabeticBaseline(),
                                       paragraph->getMaxWidth(),
                                       paragraph->getAlphabeticBaseline());
    canvas.drawLine(SK_ColorRED, rect1, false);
    canvas.drawLine(SK_ColorGREEN, rect2, false);

    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(paragraph->getIdeographicBaseline(), 79.035f, EPSILON100));
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(paragraph->getAlphabeticBaseline(), 63.305f, EPSILON100));
}

// Checked: NO DIFF
DEF_TEST(SkParagraph_FontFallbackParagraph, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_FontFallbackParagraph.png");

    const char* text1 = "Roboto 字典 ";         // Roboto + unresolved
    const char* text2 = "Homemade Apple 字典";  // Homemade Apple + Noto Sans...
    const char* text3 = "Chinese 字典";         // Homemade Apple + Source Han

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({
            SkString("Not a real font"),
            SkString("Also a fake font"),
            SkString("So fake it is obvious"),
            SkString("Next one should be a real font..."),
            SkString("Roboto"),
            SkString("another fake one in between"),
            SkString("Homemade Apple"),
    });
    text_style.setColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text1);

    text_style.setFontFamilies({
            SkString("Not a real font"),
            SkString("Also a fake font"),
            SkString("So fake it is obvious"),
            SkString("Homemade Apple"),
            SkString("Next one should be a real font..."),
            SkString("Roboto"),
            SkString("another fake one in between"),
            SkString("Noto Sans CJK JP"),
            SkString("Source Han Serif CN"),
    });
    builder.pushStyle(text_style);
    builder.addText(text2);

    text_style.setFontFamilies({
            SkString("Not a real font"),
            SkString("Also a fake font"),
            SkString("So fake it is obvious"),
            SkString("Homemade Apple"),
            SkString("Next one should be a real font..."),
            SkString("Roboto"),
            SkString("another fake one in between"),
            SkString("Source Han Serif CN"),
            SkString("Noto Sans CJK JP"),
    });
    builder.pushStyle(text_style);
    builder.addText(text3);

    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);
    paragraph->paint(canvas.get(), 10.0, 15.0);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());

    // Font resolution in Skia produces 6 runs because 2 parts of "Roboto 字典 " have different
    // script (Minikin merges the first 2 into one because of unresolved) [Apple + Unresolved ]
    // [Apple + Noto] [Apple + Han]
    REPORTER_ASSERT(reporter, impl->runs().size() == 6);

    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(impl->runs()[0].advance().fX, 48.330f, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(impl->runs()[1].advance().fX, 15.879f, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(impl->runs()[2].advance().fX, 139.125f, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(impl->runs()[3].advance().fX, 27.999f, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(impl->runs()[4].advance().fX, 62.248f, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(impl->runs()[5].advance().fX, 27.999f, EPSILON100));

    // When a different font is resolved, then the metrics are different.
    REPORTER_ASSERT(reporter, impl->runs()[1].correctAscent() != impl->runs()[3].correctAscent());
    REPORTER_ASSERT(reporter, impl->runs()[1].correctDescent() != impl->runs()[3].correctDescent());
    REPORTER_ASSERT(reporter, impl->runs()[3].correctAscent() != impl->runs()[5].correctAscent());
    REPORTER_ASSERT(reporter, impl->runs()[3].correctDescent() != impl->runs()[5].correctDescent());
    REPORTER_ASSERT(reporter, impl->runs()[1].correctAscent() != impl->runs()[5].correctAscent());
    REPORTER_ASSERT(reporter, impl->runs()[1].correctDescent() != impl->runs()[5].correctDescent());
}

// Checked: NO DIFF
DEF_TEST(SkParagraph_StrutParagraph1, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_StrutParagraph1.png");
    // The chinese extra height should be absorbed by the strut.
    const char* text = "01234満毎冠p来É本可\nabcd\n満毎É行p昼本可";

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

    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Ahem")});
    text_style.setFontSize(50);
    text_style.setFontStyle(SkFontStyle(SkFontStyle::kMedium_Weight, SkFontStyle::kNormal_Width, SkFontStyle::kUpright_Slant));
    text_style.setColor(SK_ColorBLACK);
    text_style.setHeight(0.5f);
    builder.pushStyle(text_style);
    builder.addText(text);
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
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 0, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 34.5f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 50, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 84.5f, EPSILON100));
    }
    {
        auto boxes = paragraph->getRectsForRange(0, 1, rect_height_max_style, rect_width_style);
        canvas.drawRects(SK_ColorRED, boxes);
        REPORTER_ASSERT(reporter, boxes.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 0, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 34.5f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 50, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 95, EPSILON100));
    }
    {
        auto boxes = paragraph->getRectsForRange(6, 10, rect_height_style, rect_width_style);
        canvas.drawRects(SK_ColorRED, boxes);
        REPORTER_ASSERT(reporter, boxes.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 300, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 34.5f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 500, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 84.5f, EPSILON100));
    }
    {
        auto boxes = paragraph->getRectsForRange(6, 10, rect_height_max_style, rect_width_style);
        canvas.drawRects(SK_ColorRED, boxes);
        REPORTER_ASSERT(reporter, boxes.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 300, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 34.5f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 500, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 95, EPSILON100));
    }
    {
        auto boxes = paragraph->getRectsForRange(14, 16, rect_height_max_style, rect_width_style);
        canvas.drawRects(SK_ColorRED, boxes);
        REPORTER_ASSERT(reporter, boxes.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 0, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 224.5f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 100, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 285, EPSILON100));
    }
    {
        auto boxes = paragraph->getRectsForRange(20, 25, rect_height_max_style, rect_width_style);
        canvas.drawRects(SK_ColorRED, boxes);
        REPORTER_ASSERT(reporter, boxes.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 50, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 319.5f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 300, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 380, EPSILON100));
    }
}

// Checked: NO DIFF
DEF_TEST(SkParagraph_StrutParagraph2, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    TestCanvas canvas("SkParagraph_StrutParagraph2.png");
    if (!fontCollection->fontsFound()) return;
    // The chinese extra height should be absorbed by the strut.
    const char* text = "01234ABCDEFGH\nabcd\nABCDEFGH";

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

    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Ahem")});
    text_style.setFontSize(50);
    text_style.setFontStyle(SkFontStyle(SkFontStyle::kMedium_Weight, SkFontStyle::kNormal_Width,
    SkFontStyle::kUpright_Slant));
    text_style.setColor(SK_ColorBLACK);
    text_style.setHeight(1);
    builder.pushStyle(text_style);
    builder.addText(text);
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
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 0, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 24, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 50, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 74, EPSILON100));
    }
    {
        auto boxes = paragraph->getRectsForRange(0, 1, rect_height_max_style, rect_width_style);
        canvas.drawRects(SK_ColorRED, boxes);
        REPORTER_ASSERT(reporter, boxes.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 0, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 24, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 50, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 80, EPSILON100));
    }
    {
        auto boxes = paragraph->getRectsForRange(6, 10, rect_height_style, rect_width_style);
        canvas.drawRects(SK_ColorRED, boxes);
        REPORTER_ASSERT(reporter, boxes.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 300, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 24, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 500, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 74, EPSILON100));
    }
    {
        auto boxes = paragraph->getRectsForRange(6, 10, rect_height_max_style, rect_width_style);
        canvas.drawRects(SK_ColorRED, boxes);
        REPORTER_ASSERT(reporter, boxes.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 300, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 24, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 500, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 80, EPSILON100));
    }
    {
        auto boxes = paragraph->getRectsForRange(14, 16, rect_height_max_style, rect_width_style);
        canvas.drawRects(SK_ColorRED, boxes);
        REPORTER_ASSERT(reporter, boxes.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 0, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 184, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 100, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 240, EPSILON100));
    }
    {
        auto boxes = paragraph->getRectsForRange(20, 25, rect_height_max_style, rect_width_style);
        canvas.drawRects(SK_ColorRED, boxes);
        REPORTER_ASSERT(reporter, boxes.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 50, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 264, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 300, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 320, EPSILON100));
    }
}

// Checked: NO DIFF
DEF_TEST(SkParagraph_StrutParagraph3, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_StrutParagraph3.png");

    // The chinese extra height should be absorbed by the strut.
    const char* text = "01234満毎p行来昼本可\nabcd\n満毎冠行来昼本可";

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

    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Ahem")});
    text_style.setFontSize(50);
    text_style.setFontStyle(SkFontStyle(SkFontStyle::kMedium_Weight, SkFontStyle::kNormal_Width,
    SkFontStyle::kUpright_Slant));
    text_style.setColor(SK_ColorBLACK);
    text_style.setHeight(1);
    builder.pushStyle(text_style);
    builder.addText(text);
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
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 0, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 8, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 50, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 58, epsilon));
    }
    {
        auto boxes = paragraph->getRectsForRange(0, 1, rect_height_max_style, rect_width_style);
        canvas.drawRects(SK_ColorRED, boxes);
        REPORTER_ASSERT(reporter, boxes.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 0, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 8, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 50, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 60, epsilon));
    }
    {
        auto boxes = paragraph->getRectsForRange(6, 10, rect_height_style, rect_width_style);
        canvas.drawRects(SK_ColorRED, boxes);
        REPORTER_ASSERT(reporter, boxes.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 300, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 8, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 500, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 58, epsilon));
    }
    {
        auto boxes = paragraph->getRectsForRange(6, 10, rect_height_max_style, rect_width_style);
        canvas.drawRects(SK_ColorRED, boxes);
        REPORTER_ASSERT(reporter, boxes.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 300, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 8, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 500, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 60, epsilon));
    }
    {
        auto boxes = paragraph->getRectsForRange(14, 16, rect_height_max_style, rect_width_style);
        canvas.drawRects(SK_ColorRED, boxes);
        REPORTER_ASSERT(reporter, boxes.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 0, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 128, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 100, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 180, epsilon));
    }
    {
        auto boxes = paragraph->getRectsForRange(20, 25, rect_height_max_style, rect_width_style);
        canvas.drawRects(SK_ColorRED, boxes);
        REPORTER_ASSERT(reporter, boxes.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 50, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 188, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 300, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 240, epsilon));
    }
}

// Checked: NO DIFF
DEF_TEST(SkParagraph_StrutForceParagraph, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_StrutForceParagraph.png");
  const char* text = "01234満毎冠行来昼本可\nabcd\n満毎冠行来昼本可";

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

    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Ahem")});
    text_style.setFontSize(50);
    text_style.setLetterSpacing(0);
    text_style.setColor(SK_ColorBLACK);
    text_style.setHeight(1);
    builder.pushStyle(text_style);
    builder.addText(text);
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
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes2[0].rect.left(), 0, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes2[0].rect.top(), 22.5f, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes2[0].rect.right(), 50, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes2[0].rect.bottom(), 72.5f, EPSILON100));

    auto boxes3 = paragraph->getRectsForRange(0, 1, rect_height_max_style, rect_width_style);
    canvas.drawRects(SK_ColorRED, boxes3);
    REPORTER_ASSERT(reporter, boxes3.size() == 1);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes3[0].rect.left(), 0, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes3[0].rect.top(), 22.5f, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes3[0].rect.right(), 50, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes3[0].rect.bottom(), 80, EPSILON100));

    auto boxes4 = paragraph->getRectsForRange(6, 10, rect_height_style, rect_width_style);
    canvas.drawRects(SK_ColorRED, boxes4);
    REPORTER_ASSERT(reporter, boxes4.size() == 1);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes4[0].rect.left(), 300, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes4[0].rect.top(), 22.5f, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes4[0].rect.right(), 500, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes4[0].rect.bottom(), 72.5f, EPSILON100));

    auto boxes5 = paragraph->getRectsForRange(6, 10, rect_height_max_style, rect_width_style);
    canvas.drawRects(SK_ColorRED, boxes5);
    REPORTER_ASSERT(reporter, boxes5.size() == 1);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes5[0].rect.left(), 300, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes5[0].rect.top(), 22.5f, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes5[0].rect.right(), 500, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes5[0].rect.bottom(), 80, EPSILON100));

    auto boxes6 = paragraph->getRectsForRange(14, 16, rect_height_max_style, rect_width_style);
    canvas.drawRects(SK_ColorRED, boxes6);
    REPORTER_ASSERT(reporter, boxes6.size() == 1);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes6[0].rect.left(), 0, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes6[0].rect.top(), 182.5f, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes6[0].rect.right(), 100, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes6[0].rect.bottom(), 240, EPSILON100));

    auto boxes7 = paragraph->getRectsForRange(20, 25, rect_height_max_style, rect_width_style);
    canvas.drawRects(SK_ColorRED, boxes7);
    REPORTER_ASSERT(reporter, boxes7.size() == 1);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes7[0].rect.left(), 50, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes7[0].rect.top(), 262.5f, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes7[0].rect.right(), 300, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes7[0].rect.bottom(), 320, EPSILON100));
}

// Checked: NO DIFF
DEF_TEST(SkParagraph_StrutDefaultParagraph, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_StrutDefaultParagraph.png");

    const char* text = "01234満毎冠行来昼本可\nabcd\n満毎冠行来昼本可";

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

    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Ahem")});
    text_style.setFontSize(20);
    text_style.setColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text);
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
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 0, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 26.5f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 20, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 46.5f, EPSILON100));
    }
    {
        auto boxes = paragraph->getRectsForRange(0, 2, rect_height_strut_style, rect_width_style);
        canvas.drawRects(SK_ColorRED, boxes);
        REPORTER_ASSERT(reporter, boxes.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 0, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 2.5f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 40, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 52.5f, EPSILON100));
    }
}

// TODO: Implement font features
DEF_TEST(SkParagraph_FontFeaturesParagraph, reporter) {
    SkDebugf("TODO: Font features\n");
}

// Not in Minikin
DEF_TEST(SkParagraph_WhitespacesInMultipleFonts, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    const char* text = "English English 字典 字典 😀😃😄 😀😃😄";
    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies(
            {SkString("Roboto"), SkString("Noto Color Emoji"), SkString("Source Han Serif CN")});
    text_style.setFontSize(60);
    builder.pushStyle(text_style);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);

    SkDEBUGCODE(auto impl = static_cast<ParagraphImpl*>(paragraph.get());)
            SkASSERT(impl->runs().size() == 3);
    SkASSERT(impl->runs()[0].textRange().end == impl->runs()[1].textRange().start);
    SkASSERT(impl->runs()[1].textRange().end == impl->runs()[2].textRange().start);
}

DEF_TEST(SkParagraph_JSON1, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    const char* text = "👨‍👩‍👧‍👦";

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Noto Color Emoji")});
    builder.pushStyle(text_style);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, impl->runs().size() == 1);
    auto run = impl->runs().front();

    auto cluster = 0;
    SkShaperJSONWriter::VisualizeClusters(
            text, 0, std::strlen(text), run.glyphs(), run.clusterIndexes(),
            [&](int codePointCount, SkSpan<const char> utf1to1, SkSpan<const SkGlyphID> glyph1to1) {
                if (cluster == 0) {
                    std::string toCheckUtf8{utf1to1.data(), utf1to1.size()};
                    SkASSERT(std::strcmp(text, utf1to1.data()) == 0);
                    SkASSERT(glyph1to1.size() == 1);
                    SkASSERT(*glyph1to1.begin() == 1611);
                }
                ++cluster;
            });
    SkASSERT(cluster <= 2);
}

DEF_TEST(SkParagraph_JSON2, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    const char* text = "p〠q";

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Noto Sans CJK JP")});
    text_style.setColor(SK_ColorBLACK);
    text_style.setFontSize(50);
    builder.pushStyle(text_style);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, impl->runs().size() == 1);
    auto run = impl->runs().front();

    auto cluster = 0;
    for (auto& run : impl->runs()) {
        SkShaperJSONWriter::VisualizeClusters(
                impl->text().begin() + run.textRange().start, 0, run.textRange().width(),
                run.glyphs(), run.clusterIndexes(),
                [&](int codePointCount, SkSpan<const char> utf1to1,
                    SkSpan<const SkGlyphID> glyph1to1) {
                    if (cluster == 0) {
                        std::string toCheckUtf8{utf1to1.data(), utf1to1.size()};
                        SkASSERT(std::strcmp(text, utf1to1.data()) == 0);
                        SkASSERT(glyph1to1.size() == 3);
                    }
                    ++cluster;
                });
    }

    SkASSERT(cluster <= 2);
}

DEF_TEST(SkParagraph_CacheText, reporter) {
    ParagraphCache cache;
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setColor(SK_ColorBLACK);

    auto test = [&](const char* text, int count, bool expectedToBeFound) {
        ParagraphBuilderImpl builder(paragraph_style, fontCollection);
        builder.pushStyle(text_style);
        builder.addText(text);
        builder.pop();
        auto paragraph = builder.Build();

        auto impl = static_cast<ParagraphImpl*>(paragraph.get());
        REPORTER_ASSERT(reporter, count == cache.count());
        auto found = cache.findParagraph(impl);
        REPORTER_ASSERT(reporter, found == expectedToBeFound);
        auto added = cache.updateParagraph(impl);
        REPORTER_ASSERT(reporter, added != expectedToBeFound);
    };

    test("text1", 0, false);
    test("text1", 1, true);
    test("text2", 1, false);
    test("text2", 2, true);
    test("text3", 2, false);
}

DEF_TEST(SkParagraph_CacheFonts, reporter) {
    ParagraphCache cache;
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();

    TextStyle text_style;
    text_style.setColor(SK_ColorBLACK);

    auto test = [&](int count, bool expectedToBeFound) {
        ParagraphBuilderImpl builder(paragraph_style, fontCollection);
        builder.pushStyle(text_style);
        builder.addText("text");
        builder.pop();
        auto paragraph = builder.Build();
        auto impl = static_cast<ParagraphImpl*>(paragraph.get());

        impl->getResolver().findAllFontsForAllStyledBlocks(impl);

        REPORTER_ASSERT(reporter, count == cache.count());
        auto found = cache.findParagraph(impl);
        REPORTER_ASSERT(reporter, found == expectedToBeFound);
        auto added = cache.updateParagraph(impl);
        REPORTER_ASSERT(reporter, added != expectedToBeFound);
    };

    text_style.setFontFamilies({SkString("Roboto")});
    test(0, false);
    test(1, true);
    text_style.setFontFamilies({SkString("Homemade Apple")});
    test(1, false);
    test(2, true);
    text_style.setFontFamilies({SkString("Noto Color Emoji")});
    test(2, false);
}

DEF_TEST(SkParagraph_CacheFontRanges, reporter) {
    ParagraphCache cache;
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setColor(SK_ColorBLACK);

    auto test = [&](const char* text1,
                    const char* text2,
                    const char* font1,
                    const char* font2,
                    int count,
                    bool expectedToBeFound) {
        ParagraphBuilderImpl builder(paragraph_style, fontCollection);
        text_style.setFontFamilies({SkString(font1)});
        builder.pushStyle(text_style);
        builder.addText(text1);
        builder.pop();
        text_style.setFontFamilies({SkString(font2)});
        builder.pushStyle(text_style);
        builder.addText(text2);
        builder.pop();
        auto paragraph = builder.Build();
        auto impl = static_cast<ParagraphImpl*>(paragraph.get());

        impl->getResolver().findAllFontsForAllStyledBlocks(impl);

        REPORTER_ASSERT(reporter, count == cache.count());
        auto found = cache.findParagraph(impl);
        REPORTER_ASSERT(reporter, found == expectedToBeFound);
        auto added = cache.updateParagraph(impl);
        REPORTER_ASSERT(reporter, added != expectedToBeFound);
    };

    test("text", "", "Roboto", "Homemade Apple", 0, false);
    test("t", "ext", "Roboto", "Homemade Apple", 1, false);
    test("te", "xt", "Roboto", "Homemade Apple", 2, false);
    test("tex", "t", "Roboto", "Homemade Apple", 3, false);
    test("text", "", "Roboto", "Homemade Apple", 4, true);
}

DEF_TEST(SkParagraph_CacheStyles, reporter) {
    ParagraphCache cache;
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setColor(SK_ColorBLACK);

    auto test = [&](int count, bool expectedToBeFound) {
        ParagraphBuilderImpl builder(paragraph_style, fontCollection);
        builder.pushStyle(text_style);
        builder.addText("text");
        builder.pop();
        auto paragraph = builder.Build();
        auto impl = static_cast<ParagraphImpl*>(paragraph.get());

        impl->getResolver().findAllFontsForAllStyledBlocks(impl);

        REPORTER_ASSERT(reporter, count == cache.count());
        auto found = cache.findParagraph(impl);
        REPORTER_ASSERT(reporter, found == expectedToBeFound);
        auto added = cache.updateParagraph(impl);
        REPORTER_ASSERT(reporter, added != expectedToBeFound);
    };

    test(0, false);
    test(1, true);
    text_style.setLetterSpacing(10);
    test(1, false);
    test(2, true);
    text_style.setWordSpacing(10);
    test(2, false);
}

DEF_TEST(SkParagraph_EmptyParagraph, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_EmptyParagraph.png");

    ParagraphStyle paragraph_style;
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setColor(SK_ColorBLACK);
    //builder.pushStyle(text_style);
    builder.addText("");
    //builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);
    paragraph->paint(canvas.get(), 0, 0);
}

DEF_TEST(SkParagraph_PlaceholderOnly, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_PlaceholderOnly.png");

    ParagraphStyle paragraph_style;
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    PlaceholderStyle placeholder(50, 50, PlaceholderAlignment::kBaseline, TextBaseline::kAlphabetic, 0);
    builder.addPlaceholder(placeholder);

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);
    paragraph->paint(canvas.get(), 0, 0);
}
