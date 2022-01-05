// Copyright 2019 Google LLC.
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkEncodedImageFormat.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkImageEncoder.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSpan.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "modules/skparagraph/include/DartTypes.h"
#include "modules/skparagraph/include/FontCollection.h"
#include "modules/skparagraph/include/Paragraph.h"
#include "modules/skparagraph/include/ParagraphCache.h"
#include "modules/skparagraph/include/ParagraphStyle.h"
#include "modules/skparagraph/include/TextShadow.h"
#include "modules/skparagraph/include/TextStyle.h"
#include "modules/skparagraph/include/TypefaceFontProvider.h"
#include "modules/skparagraph/src/ParagraphBuilderImpl.h"
#include "modules/skparagraph/src/ParagraphImpl.h"
#include "modules/skparagraph/src/Run.h"
#include "modules/skparagraph/src/TextLine.h"
#include "modules/skparagraph/utils/TestFontCollection.h"
#include "src/core/SkOSFile.h"
#include "src/utils/SkOSPath.h"
#include "src/utils/SkShaperJSONWriter.h"
#include "tests/Test.h"
#include "tools/Resources.h"

#include <string.h>
#include <algorithm>
#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>

struct GrContextOptions;

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

std::u16string mirror(const std::string& text) {
    std::u16string result;
    result += u"\u202E";
    for (auto i = text.size(); i > 0; --i) {
        result += text[i - 1];
    }
    //result += u"\u202C";
    return result;
}

std::u16string straight(const std::string& text) {
    std::u16string result;
    result += u"\u202D";
    for (auto ch : text) {
        result += ch;
    }
    return result;
}

class ResourceFontCollection : public FontCollection {
public:
    ResourceFontCollection(bool testOnly = false)
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
            // SkDebugf("Fonts not found, skipping all the tests\n");
            return;
        }
        // Only register fonts if we have to
        for (auto& font : fonts) {
            SkString file_path;
            file_path.printf("%s/%s", fResourceDir.c_str(), font.c_str());
            fFontProvider->registerTypeface(SkTypeface::MakeFromFile(file_path.c_str()));
        }

        if (testOnly) {
            this->setTestFontManager(std::move(fFontProvider));
        } else {
            this->setAssetFontManager(std::move(fFontProvider));
        }
        this->disableFontFallback();
    }

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

UNIX_ONLY_TEST(SkParagraph_SimpleParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;
    const char* text = "Hello World Text Dialog";
    const size_t len = strlen(text);

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setColor(SK_ColorBLACK);
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
                        [&index, reporter]
                        (TextRange textRange, const TextStyle& style, const TextLine::ClipContext& context) {
                            REPORTER_ASSERT(reporter, index == 0);
                            REPORTER_ASSERT(reporter, style.getColor() == SK_ColorBLACK);
                            ++index;
                        });
    }
}

UNIX_ONLY_TEST(SkParagraph_InlinePlaceholderParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    TestCanvas canvas("SkParagraph_InlinePlaceholderParagraph.png");
    if (!fontCollection->fontsFound()) return;

    const char* text = "012 34";
    const size_t len = strlen(text);

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
    builder.addText(text, len);

    PlaceholderStyle placeholder1(50, 50, PlaceholderAlignment::kBaseline, TextBaseline::kAlphabetic, 0);
    builder.addPlaceholder(placeholder1);
    builder.addText(text, len);
    builder.addPlaceholder(placeholder1);

    PlaceholderStyle placeholder2(5, 50, PlaceholderAlignment::kBaseline, TextBaseline::kAlphabetic, 50);
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

    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[1].rect.left(), 90.921f, EPSILON2));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[1].rect.top(), 50, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[1].rect.right(), 90.921f + 50, EPSILON2));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[1].rect.bottom(), 100, EPSILON100));

    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[3].rect.left(), 231.343f, EPSILON2));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[3].rect.top(), 50, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[3].rect.right(), 231.343f + 50, EPSILON2));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[3].rect.bottom(), 100, EPSILON100));

    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[4].rect.left(), 281.343f, EPSILON2));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[4].rect.top(), 0, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[4].rect.right(), 281.343f + 5, EPSILON2));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[4].rect.bottom(), 50, EPSILON100));

    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[6].rect.left(), 336.343f, EPSILON2));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[6].rect.top(), 0, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[6].rect.right(), 336.343f + 5, EPSILON2));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[6].rect.bottom(), 50, EPSILON100));
}

UNIX_ONLY_TEST(SkParagraph_InlinePlaceholderBaselineParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    TestCanvas canvas("SkParagraph_InlinePlaceholderBaselineParagraph.png");
    if (!fontCollection->fontsFound()) return;

    const char* text = "012 34";
    const size_t len = strlen(text);

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
    builder.addText(text, len);

    PlaceholderStyle placeholder(55, 50, PlaceholderAlignment::kBaseline, TextBaseline::kAlphabetic, 38.347f);
    builder.addPlaceholder(placeholder);
    builder.addText(text, len);

    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);
    paragraph->paint(canvas.get(), 0, 0);

    auto boxes = paragraph->getRectsForPlaceholders();
    canvas.drawRects(SK_ColorRED, boxes);

    REPORTER_ASSERT(reporter, boxes.size() == 1);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 90.921f, EPSILON2));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 0, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 90.921f + 55, EPSILON2));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 50, EPSILON100));

    RectHeightStyle rect_height_style = RectHeightStyle::kTight;
    RectWidthStyle rect_width_style = RectWidthStyle::kTight;

    boxes = paragraph->getRectsForRange(5, 6, rect_height_style, rect_width_style);
    canvas.drawRects(SK_ColorBLUE, boxes);

    REPORTER_ASSERT(reporter, boxes.size() == 1);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 75.324f, EPSILON2));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 14.226f, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 90.921f, EPSILON2));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 44.694f, EPSILON100));
}

UNIX_ONLY_TEST(SkParagraph_InlinePlaceholderAboveBaselineParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    TestCanvas canvas("SkParagraph_InlinePlaceholderAboveBaselineParagraph.png");
    if (!fontCollection->fontsFound()) return;

    const char* text = "012 34";
    const size_t len = strlen(text);

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
    builder.addText(text, len);

    PlaceholderStyle placeholder(55, 50, PlaceholderAlignment::kAboveBaseline, TextBaseline::kAlphabetic, 903129.129308f);
    builder.addPlaceholder(placeholder);
    builder.addText(text, len);

    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);
    paragraph->paint(canvas.get(), 0, 0);

    auto boxes = paragraph->getRectsForPlaceholders();
    canvas.drawRects(SK_ColorRED, boxes);

    REPORTER_ASSERT(reporter, boxes.size() == 1);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 90.921f, EPSILON2));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), -0.347f, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 90.921f + 55, EPSILON2));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 49.652f, EPSILON100));

    RectHeightStyle rect_height_style = RectHeightStyle::kTight;
    RectWidthStyle rect_width_style = RectWidthStyle::kTight;

    boxes = paragraph->getRectsForRange(5, 6, rect_height_style, rect_width_style);
    canvas.drawRects(SK_ColorBLUE, boxes);

    REPORTER_ASSERT(reporter, boxes.size() == 1);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 75.324f, EPSILON2));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 25.531f, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 90.921f, EPSILON2));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 56, EPSILON100));
}

UNIX_ONLY_TEST(SkParagraph_InlinePlaceholderBelowBaselineParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    TestCanvas canvas("SkParagraph_InlinePlaceholderBelowBaselineParagraph.png");
    if (!fontCollection->fontsFound()) return;

    const char* text = "012 34";
    const size_t len = strlen(text);

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
    builder.addText(text, len);

    PlaceholderStyle placeholder(55, 50, PlaceholderAlignment::kBelowBaseline, TextBaseline::kAlphabetic, 903129.129308f);
    builder.addPlaceholder(placeholder);
    builder.addText(text, len);

    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);
    paragraph->paint(canvas.get(), 0, 0);

    auto boxes = paragraph->getRectsForPlaceholders();
    canvas.drawRects(SK_ColorRED, boxes);

    REPORTER_ASSERT(reporter, boxes.size() == 1);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 90.921f, EPSILON2));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 24, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 90.921f + 55, EPSILON2));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 74, EPSILON100));

    RectHeightStyle rect_height_style = RectHeightStyle::kTight;
    RectWidthStyle rect_width_style = RectWidthStyle::kTight;

    boxes = paragraph->getRectsForRange(5, 6, rect_height_style, rect_width_style);
    canvas.drawRects(SK_ColorBLUE, boxes);

    REPORTER_ASSERT(reporter, boxes.size() == 1);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 75.324f, EPSILON2));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), -0.121f, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 90.921f, EPSILON2));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 30.347f, EPSILON100));
}

UNIX_ONLY_TEST(SkParagraph_InlinePlaceholderBottomParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    TestCanvas canvas("SkParagraph_InlinePlaceholderBottomParagraph.png");
    if (!fontCollection->fontsFound()) return;

    const char* text = "012 34";
    const size_t len = strlen(text);

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
    builder.addText(text, len);

    PlaceholderStyle placeholder(55, 50, PlaceholderAlignment::kBottom, TextBaseline::kAlphabetic, 0);
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
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 90.921f, EPSILON50));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 0, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 90.921f + 55, EPSILON50));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 50, EPSILON100));

    boxes = paragraph->getRectsForRange(0, 1, rect_height_style, rect_width_style);
    canvas.drawRects(SK_ColorBLUE, boxes);
    REPORTER_ASSERT(reporter, boxes.size() == 1);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 0.5f, EPSILON50));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 19.531f, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 16.097f, EPSILON50));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 50, EPSILON100));
}

UNIX_ONLY_TEST(SkParagraph_InlinePlaceholderTopParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    TestCanvas canvas("SkParagraph_InlinePlaceholderTopParagraph.png");
    if (!fontCollection->fontsFound()) return;

    const char* text = "012 34";
    const size_t len = strlen(text);

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
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 90.921f, EPSILON50));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 0, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 90.921f + 55, EPSILON50));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 50, EPSILON100));

    boxes = paragraph->getRectsForRange(0, 1, rect_height_style, rect_width_style);
    canvas.drawRects(SK_ColorBLUE, boxes);
    REPORTER_ASSERT(reporter, boxes.size() == 1);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 0.5f, EPSILON50));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 0, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 16.097f, EPSILON50));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 30.468f, EPSILON100));
}

UNIX_ONLY_TEST(SkParagraph_InlinePlaceholderMiddleParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    TestCanvas canvas("SkParagraph_InlinePlaceholderMiddleParagraph.png");
    if (!fontCollection->fontsFound()) return;

    const char* text = "012 34";
    const size_t len = strlen(text);

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
    builder.addText(text, len);

    PlaceholderStyle placeholder(55, 50, PlaceholderAlignment::kMiddle, TextBaseline::kAlphabetic, 0);
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
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 90.921f, EPSILON50));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 0, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 90.921f + 55, EPSILON50));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 50, EPSILON100));

    boxes = paragraph->getRectsForRange(5, 6, rect_height_style, rect_width_style);
    canvas.drawRects(SK_ColorBLUE, boxes);
    REPORTER_ASSERT(reporter, boxes.size() == 1);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 75.324f, EPSILON50));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 9.765f, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 90.921f, EPSILON50));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 40.234f, EPSILON100));
}

UNIX_ONLY_TEST(SkParagraph_InlinePlaceholderIdeographicBaselineParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    TestCanvas canvas("SkParagraph_InlinePlaceholderIdeographicBaselineParagraph.png");
    if (!fontCollection->fontsFound()) return;

    const char* text = "給能上目秘使";
    const size_t len = strlen(text);

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
    builder.addText(text, len);
    PlaceholderStyle placeholder(55, 50, PlaceholderAlignment::kBaseline, TextBaseline::kIdeographic, 38.347f);
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
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 162.5f, EPSILON50));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 0, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 162.5f + 55, EPSILON50));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 50, EPSILON100));

    boxes = paragraph->getRectsForRange(5, 6, rect_height_style, rect_width_style);
    canvas.drawRects(SK_ColorBLUE, boxes);
    REPORTER_ASSERT(reporter, boxes.size() == 1);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 135.5f, EPSILON50));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 4.703f, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 162.5f, EPSILON50));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 42.065f, EPSILON100));
}

UNIX_ONLY_TEST(SkParagraph_InlinePlaceholderBreakParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    TestCanvas canvas("SkParagraph_InlinePlaceholderBreakParagraph.png");
    if (!fontCollection->fontsFound()) return;

    const char* text = "012 34";
    const size_t len = strlen(text);

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
    builder.addText(text, len);

    PlaceholderStyle placeholder1(50, 50, PlaceholderAlignment::kBaseline, TextBaseline::kAlphabetic, 50);
    PlaceholderStyle placeholder2(25, 25, PlaceholderAlignment::kBaseline, TextBaseline::kAlphabetic, 12.5f);

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
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 31.695f, EPSILON50));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 218.531f, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 47.292f, EPSILON50));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 249, EPSILON100));

    boxes = paragraph->getRectsForPlaceholders();
    canvas.drawRects(SK_ColorRED, boxes);

    boxes = paragraph->getRectsForRange(4, 45, rect_height_style, rect_width_style);
    canvas.drawRects(SK_ColorBLUE, boxes);
    REPORTER_ASSERT(reporter, boxes.size() == 30);
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

UNIX_ONLY_TEST(SkParagraph_InlinePlaceholderGetRectsParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    TestCanvas canvas("SkParagraph_InlinePlaceholderGetRectsParagraph.png");
    if (!fontCollection->fontsFound()) return;

    const char* text = "012 34";
    const size_t len = strlen(text);

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
    builder.addText(text, len);

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

    builder.addText(text, len);

    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder2);
    builder.addPlaceholder(placeholder2); // 1 + 2
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder2);
    builder.addPlaceholder(placeholder2); // 1 + 2

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
    builder.addText(text, len);  // 11

    builder.addPlaceholder(placeholder2);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder2);
    builder.addPlaceholder(placeholder1);
    builder.addPlaceholder(placeholder2);

    builder.addText(text, len);

    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);
    paragraph->paint(canvas.get(), 0, 0);

    RectHeightStyle rect_height_style = RectHeightStyle::kMax;
    RectWidthStyle rect_width_style = RectWidthStyle::kTight;

    auto boxes = paragraph->getRectsForPlaceholders();
    canvas.drawRects(SK_ColorRED, boxes);

    REPORTER_ASSERT(reporter, boxes.size() == 34);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 90.921f, EPSILON50));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 0, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 140.921f, EPSILON50));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 50, EPSILON100));

    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[16].rect.left(), 800.921f, EPSILON20));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[16].rect.top(), 0, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[16].rect.right(), 850.921f, EPSILON20));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[16].rect.bottom(), 50, EPSILON100));

    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[33].rect.left(), 503.382f, EPSILON10));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[33].rect.top(), 160, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[33].rect.right(), 508.382f, EPSILON10));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[33].rect.bottom(), 180, EPSILON100));

    boxes = paragraph->getRectsForRange(30, 50, rect_height_style, rect_width_style);
    canvas.drawRects(SK_ColorBLUE, boxes);

    REPORTER_ASSERT(reporter, boxes.size() == 8);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 216.097f, EPSILON50));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 60, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 290.921f, EPSILON50));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 120, EPSILON100));

    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[1].rect.left(), 290.921f, EPSILON20));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[1].rect.top(), 60, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[1].rect.right(), 340.921f, EPSILON20));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[1].rect.bottom(), 120, EPSILON100));

    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[2].rect.left(), 340.921f, EPSILON50));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[2].rect.top(), 60, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[2].rect.right(), 345.921f, EPSILON50));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[2].rect.bottom(), 120, EPSILON100));
}

UNIX_ONLY_TEST(SkParagraph_SimpleRedParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;
    const char* text = "I am RED";
    const size_t len = strlen(text);

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

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
            [reporter, &index](TextRange textRange, const TextStyle& style, const TextLine::ClipContext& context) {
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
    builder.addText(text1, strlen(text1));

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
    text_style4.setDecoration((TextDecoration)(
            TextDecoration::kUnderline | TextDecoration::kOverline | TextDecoration::kLineThrough));
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

    auto rects = paragraph->getRectsForRange(0, impl->text().size(), RectHeightStyle::kMax, RectWidthStyle::kTight);
    canvas.drawRects(SK_ColorMAGENTA, rects);

    size_t index = 0;
    impl->lines()[0].scanStyles(
        StyleType::kAllAttributes,
           [&](TextRange textRange, const TextStyle& style, const TextLine::ClipContext& context) {
            switch (index) {
                case 0:
                    REPORTER_ASSERT(reporter, style.equals(text_style1));
                    REPORTER_ASSERT(reporter, equal(impl->text().begin(), textRange, text1));
                    break;
                case 1:
                    REPORTER_ASSERT(reporter, style.equals(text_style2));
                    REPORTER_ASSERT(reporter, equal(impl->text().begin(), textRange, text2));
                    break;
                case 2:
                    REPORTER_ASSERT(reporter, style.equals(text_style3));
                    REPORTER_ASSERT(reporter, equal(impl->text().begin(), textRange, text3));
                    break;
                case 3:
                    REPORTER_ASSERT(reporter, style.equals(text_style4));
                    REPORTER_ASSERT(reporter, equal(impl->text().begin(), textRange, text41));
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
                REPORTER_ASSERT(reporter, equal(impl->text().begin(), textRange, text42));
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

UNIX_ONLY_TEST(SkParagraph_DefaultStyleParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_DefaultStyleParagraph.png");
    const char* text = "No TextStyle! Uh Oh!";
    const size_t len = strlen(text);

    ParagraphStyle paragraph_style;
    TextStyle defaultStyle;
    defaultStyle.setFontFamilies({SkString("Roboto")});
    paragraph_style.setTextStyle(defaultStyle);
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);
    builder.addText(text, len);

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);
    paragraph->paint(canvas.get(), 10.0, 15.0);

    REPORTER_ASSERT(reporter, paragraph->unresolvedGlyphs() == 0);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());

    REPORTER_ASSERT(reporter, impl->runs().size() == 1);
    REPORTER_ASSERT(reporter, impl->styles().size() == 1);
    REPORTER_ASSERT(reporter, impl->lines().size() == 1);

    size_t index = 0;
    impl->lines()[0].scanStyles(
            StyleType::kAllAttributes,
            [&](TextRange textRange, const TextStyle& style, const TextLine::ClipContext& context) {
                REPORTER_ASSERT(reporter, style.equals(paragraph_style.getTextStyle()));
                REPORTER_ASSERT(reporter, equal(impl->text().begin(), textRange, text));
                ++index;
                return true;
            });
    REPORTER_ASSERT(reporter, index == 1);
}

UNIX_ONLY_TEST(SkParagraph_BoldParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_BoldParagraph.png");
    const char* text = "This is Red max bold text!";
    const size_t len = strlen(text);

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
                REPORTER_ASSERT(reporter, equal(impl->text().begin(), textRange, text));
                ++index;
                return true;
            });
    REPORTER_ASSERT(reporter, index == 1);
}

UNIX_ONLY_TEST(SkParagraph_HeightOverrideParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_HeightOverrideParagraph.png");
    const char* text = "01234満毎冠行来昼本可\nabcd\n満毎冠行来昼本可";
    const size_t len = strlen(text);

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

UNIX_ONLY_TEST(SkParagraph_BasicHalfLeading, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();

    if (!fontCollection->fontsFound()) {
      return;
    }

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

    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    builder.pushStyle(text_style);
    builder.addText(text);

    auto paragraph = builder.Build();
    paragraph->layout(550);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, impl->styles().size() == 1);  // paragraph style does not count
    REPORTER_ASSERT(reporter, impl->styles()[0].fStyle.equals(text_style));

    paragraph->paint(canvas.get(), 0, 0);

    const RectWidthStyle rect_width_style = RectWidthStyle::kTight;
    std::vector<TextBox> boxes = paragraph->getRectsForRange(0, len, RectHeightStyle::kTight, rect_width_style);
    std::vector<TextBox> lineBoxes = paragraph->getRectsForRange(0, len, RectHeightStyle::kMax, rect_width_style);

    canvas.drawRects(SK_ColorBLUE, boxes);
    REPORTER_ASSERT(reporter, boxes.size() == 3ull);
    REPORTER_ASSERT(reporter, lineBoxes.size() == boxes.size());

    const auto line_spacing1 = boxes[1].rect.top() - boxes[0].rect.bottom();
    const auto line_spacing2 = boxes[2].rect.top() - boxes[1].rect.bottom();

    // Uniform line spacing.
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(line_spacing1, line_spacing2));

    // line spacing is distributed evenly over and under the text.
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(lineBoxes[0].rect.bottom() - boxes[0].rect.bottom(), boxes[0].rect.top() - lineBoxes[0].rect.top()));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(lineBoxes[1].rect.bottom() - boxes[1].rect.bottom(), boxes[1].rect.top() - lineBoxes[1].rect.top()));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(lineBoxes[2].rect.bottom() - boxes[2].rect.bottom(), boxes[2].rect.top() - lineBoxes[2].rect.top()));

    // Half leading does not move the text horizontally.
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[1].rect.left(), 0, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[1].rect.right(), 43.843f, EPSILON100));
}

UNIX_ONLY_TEST(SkParagraph_NearZeroHeightMixedDistribution, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();

    if (!fontCollection->fontsFound()) {
      return;
    }

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

    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

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

    std::vector<TextBox> boxes = paragraph->getRectsForRange(0, len, RectHeightStyle::kTight, rect_width_style);
    std::vector<TextBox> lineBoxes = paragraph->getRectsForRange(0, len, RectHeightStyle::kMax, rect_width_style);

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

    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(lineBoxes[0].rect.height(), line_bottom - line_top, EPSILON2));
    REPORTER_ASSERT(reporter, lineBoxes[0].rect.height() > 1);

    // Half leading does not move the text horizontally.
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 0, EPSILON100));
}

UNIX_ONLY_TEST(SkParagraph_StrutHalfLeading, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();

    if (!fontCollection->fontsFound()) {
      return;
    }

    const char* text = "01234満毎冠行来昼本可\nabcd\n満毎冠行来昼本可";
    const size_t len = strlen(text);

    TestCanvas canvas("SkParagraph_StrutHalfLeading.png");

    ParagraphStyle paragraph_style;
    // Tiny font and height multiplier to ensure the height is entirely decided
    // by the strut.
    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setFontSize(1.0f);
    text_style.setColor(SK_ColorBLACK);
    text_style.setLetterSpacing(0.0f);
    text_style.setWordSpacing(0.0f);
    text_style.setHeight(0.1f);

    StrutStyle strut_style;
    strut_style.setFontFamilies({SkString("Roboto")});
    strut_style.setFontSize(20.0f);
    strut_style.setHeight(3.6345f);
    strut_style.setHalfLeading(true);
    strut_style.setStrutEnabled(true);
    strut_style.setForceStrutHeight(true);

    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    builder.pushStyle(text_style);
    builder.addText(text);

    auto paragraph = builder.Build();
    paragraph->layout(550);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, impl->styles().size() == 1);  // paragraph style does not count

    paragraph->paint(canvas.get(), 0, 0);

    const RectWidthStyle rect_width_style = RectWidthStyle::kTight;
    std::vector<TextBox> boxes = paragraph->getRectsForRange(0, len, RectHeightStyle::kTight, rect_width_style);
    std::vector<TextBox> lineBoxes = paragraph->getRectsForRange(0, len, RectHeightStyle::kMax, rect_width_style);

    canvas.drawRects(SK_ColorBLUE, boxes);
    REPORTER_ASSERT(reporter, boxes.size() == 3ull);
    REPORTER_ASSERT(reporter, lineBoxes.size() == boxes.size());

    const auto line_spacing1 = boxes[1].rect.top() - boxes[0].rect.bottom();
    const auto line_spacing2 = boxes[2].rect.top() - boxes[1].rect.bottom();

    // Uniform line spacing.
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(line_spacing1, line_spacing2));

    // line spacing is distributed evenly over and under the text.
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(lineBoxes[0].rect.bottom() - boxes[0].rect.bottom(), boxes[0].rect.top() - lineBoxes[0].rect.top()));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(lineBoxes[1].rect.bottom() - boxes[1].rect.bottom(), boxes[1].rect.top() - lineBoxes[1].rect.top()));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(lineBoxes[2].rect.bottom() - boxes[2].rect.bottom(), boxes[2].rect.top() - lineBoxes[2].rect.top()));

    // Half leading does not move the text horizontally.
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[1].rect.left(), 0, EPSILON100));
}

UNIX_ONLY_TEST(SkParagraph_TrimLeadingDistribution, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();

    if (!fontCollection->fontsFound()) {
      return;
    }

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

    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    builder.pushStyle(text_style);
    builder.addText(text);

    auto paragraph = builder.Build();
    paragraph->layout(550);
    paragraph->paint(canvas.get(), 0, 0);

    const RectWidthStyle rect_width_style = RectWidthStyle::kTight;

    std::vector<TextBox> boxes = paragraph->getRectsForRange(0, len, RectHeightStyle::kTight, rect_width_style);
    std::vector<TextBox> lineBoxes = paragraph->getRectsForRange(0, len, RectHeightStyle::kMax, rect_width_style);

    canvas.drawRects(SK_ColorBLUE, boxes);
    REPORTER_ASSERT(reporter, boxes.size() == 3ull);
    REPORTER_ASSERT(reporter, lineBoxes.size() == boxes.size());

    const auto line_spacing1 = boxes[1].rect.top() - boxes[0].rect.bottom();
    const auto line_spacing2 = boxes[2].rect.top() - boxes[1].rect.bottom();

    // Uniform line spacing. The delta is introduced by the height rounding.
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(line_spacing1, line_spacing2, 1));

    // Trim the first line's top leading.
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(lineBoxes[0].rect.top(), boxes[0].rect.top()));
    // Trim the last line's bottom leading.
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(lineBoxes[2].rect.bottom(), boxes[2].rect.bottom()));

    const auto halfLeading =  lineBoxes[0].rect.bottom() - boxes[0].rect.bottom();
    // Large epsilon because of rounding.
    const auto epsilon = EPSILON10;
    // line spacing is distributed evenly over and under the text.
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[1].rect.top() - lineBoxes[1].rect.top(), halfLeading, epsilon));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(lineBoxes[1].rect.bottom() - boxes[1].rect.bottom(),  halfLeading));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[2].rect.top() - lineBoxes[2].rect.top(), halfLeading, epsilon));

    // Half leading does not move the text horizontally.
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[1].rect.left(), 0, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[1].rect.right(), 43.843f, EPSILON100));
}

UNIX_ONLY_TEST(SkParagraph_LeftAlignParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
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
    const size_t len = strlen(text);

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
    const size_t len = strlen(text);

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
    const size_t len = strlen(text);

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
    const size_t len = strlen(text);

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
        return line.offset().fX;
    };

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
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_JustifyRTL.png");
    const char* text =
            "אאא בּבּבּבּ אאאא בּבּ אאא בּבּבּ אאאאא בּבּבּבּ אאאא בּבּבּבּבּ "
            "אאאאא בּבּבּבּבּ אאאבּבּבּבּבּבּאאאאא בּבּבּבּבּבּאאאאאבּבּבּבּבּבּ אאאאא בּבּבּבּבּ "
            "אאאאא בּבּבּבּבּבּ אאאאא בּבּבּבּבּבּ אאאאא בּבּבּבּבּבּ אאאאא בּבּבּבּבּבּ אאאאא בּבּבּבּבּבּ";
    const size_t len = strlen(text);

    ParagraphStyle paragraph_style;
    paragraph_style.setMaxLines(14);
    paragraph_style.setTextAlign(TextAlign::kJustify);
    paragraph_style.setTextDirection(TextDirection::kRtl);
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

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
    REPORTER_ASSERT(reporter, boxes.size() == 3);

    boxes = paragraph->getRectsForRange(240, 250, rect_height_style, rect_width_style);
    canvas.drawRects(SK_ColorBLUE, boxes);
    REPORTER_ASSERT(reporter, boxes.size() == 1);

    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 588, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 130, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 640, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 156, EPSILON100));
}

UNIX_ONLY_TEST(SkParagraph_JustifyRTLNewLine, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>(true);
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_JustifyRTLNewLine.png");
    const char* text =
            "אאא בּבּבּבּ אאאא\nבּבּ אאא בּבּבּ אאאאא בּבּבּבּ אאאא בּבּבּבּבּ "
            "אאאאא בּבּבּבּבּ אאאבּבּבּבּבּבּאאאאא בּבּבּבּבּבּאאאאאבּבּבּבּבּבּ אאאאא בּבּבּבּבּ "
            "אאאאא בּבּבּבּבּבּ אאאאא בּבּבּבּבּבּ אאאאא בּבּבּבּבּבּ אאאאא בּבּבּבּבּבּ אאאאא בּבּבּבּבּבּ";
    const size_t len = strlen(text);

    ParagraphStyle paragraph_style;
    paragraph_style.setMaxLines(14);
    paragraph_style.setTextAlign(TextAlign::kJustify);
    paragraph_style.setTextDirection(TextDirection::kRtl);
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

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
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 562, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 0, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 900, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 26, EPSILON100));

    paint.setColor(SK_ColorBLUE);
    boxes = paragraph->getRectsForRange(240, 250, rect_height_style, rect_width_style);
    for (size_t i = 0; i < boxes.size(); ++i) {
        canvas.get()->drawRect(boxes[i].rect, paint);
    }
    REPORTER_ASSERT(reporter, boxes.size() == 1ull);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 68, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 130, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 120, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 156, EPSILON100));

    // All lines should be justified to the width of the paragraph
    // except for #0 (new line) and #5 (the last one)
    for (auto& line : impl->lines()) {
        auto num = &line - impl->lines().data();
        if (num == 0 || num == 5) {
            REPORTER_ASSERT(reporter, line.width() < TestCanvasWidth - 100);
        } else {
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(line.width(), TestCanvasWidth - 100, EPSILON100),
                            "#%d: %f <= %f\n", num, line.width(), TestCanvasWidth - 100);
        }
    }
}

UNIX_ONLY_TEST(SkParagraph_LeadingSpaceRTL, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>(true);
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_LeadingSpaceRTL.png");

    const char* text = " leading space";
    const size_t len = strlen(text);

    ParagraphStyle paragraph_style;
    paragraph_style.setMaxLines(14);
    paragraph_style.setTextAlign(TextAlign::kJustify);
    paragraph_style.setTextDirection(TextDirection::kRtl);
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

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

UNIX_ONLY_TEST(SkParagraph_DecorationsParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;
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
            [&](TextRange textRange, const TextStyle& style, const TextLine::ClipContext& context) {
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

// TODO: Add test for wavy decorations.

UNIX_ONLY_TEST(SkParagraph_ItalicsParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_ItalicsParagraph.png");
    const char* text1 = "No italic ";
    const char* text2 = "Yes Italic ";
    const char* text3 = "No Italic again.";

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setFontSize(10);
    text_style.setColor(SK_ColorRED);
    builder.pushStyle(text_style);
    builder.addText(text1, strlen(text1));

    text_style.setFontStyle(SkFontStyle(SkFontStyle::kNormal_Weight, SkFontStyle::kNormal_Width,
                                        SkFontStyle::kItalic_Slant));
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

UNIX_ONLY_TEST(SkParagraph_ChineseParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_ChineseParagraph.png");
    const char* text =
            "左線読設重説切後碁給能上目秘使約。満毎冠行来昼本可必図将発確年。今属場育"
            "図情闘陰野高備込制詩西校客。審対江置講今固残必託地集済決維駆年策。立得庭"
            "際輝求佐抗蒼提夜合逃表。注統天言件自謙雅載報紙喪。作画稿愛器灯女書利変探"
            "訃第金線朝開化建。子戦年帝励害表月幕株漠新期刊人秘。図的海力生禁挙保天戦"
            "聞条年所在口。";
    const size_t len = strlen(text);

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
    builder.addText(text, len);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth - 100);
    paragraph->paint(canvas.get(), 0, 0);

    REPORTER_ASSERT(reporter, paragraph->unresolvedGlyphs() == 0);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());

    REPORTER_ASSERT(reporter, impl->runs().size() == 1);
    REPORTER_ASSERT(reporter, impl->lines().size() == 7);
    REPORTER_ASSERT(reporter, impl->styles().size() == 1);
    REPORTER_ASSERT(reporter, impl->styles()[0].fStyle.equals(text_style));
}

// Checked: disabled for TxtLib
UNIX_ONLY_TEST(SkParagraph_ArabicParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_ArabicParagraph.png");
    const char* text =
            "من أسر وإعلان الخاصّة وهولندا،, عل قائمة الضغوط بالمطالبة تلك. الصفحة "
            "بمباركة التقليدية قام عن. تصفح";
    const size_t len = strlen(text);

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
    REPORTER_ASSERT(reporter, impl->styles()[0].fStyle.equals(text_style));
}

// Checked: DIFF (2 boxes and each space is a word)
UNIX_ONLY_TEST(SkParagraph_ArabicRectsParagraph, reporter) {

    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_ArabicRectsParagraph.png");
    const char* text = "بمباركة التقليدية قام عن. تصفح يد    ";
    const size_t len = strlen(text);

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
    builder.addText(text, len);
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
// This test shows now 2 boxes for [36:40) range:
// [36:38) for arabic text and [38:39) for the last space
// that has default paragraph direction (LTR) and is placed at the end of the paragraph
UNIX_ONLY_TEST(SkParagraph_ArabicRectsLTRLeftAlignParagraph, reporter) {

    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_ArabicRectsLTRLeftAlignParagraph.png");
    const char* text = "Helloبمباركة التقليدية قام عن. تصفح يد ";
    const size_t len = strlen(text);

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
    std::vector<TextBox> boxes = paragraph->getRectsForRange(36, 40, rect_height_style, rect_width_style);
    canvas.drawRects(SK_ColorRED, boxes);

    REPORTER_ASSERT(reporter, boxes.size() == 2ull);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 83.92f, EPSILON100));  // DIFF: 89.40625
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), -0.27f, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 110.16f, EPSILON100)); // DIFF: 121.87891
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 44, EPSILON100));
}

// Checked DIFF+
UNIX_ONLY_TEST(SkParagraph_ArabicRectsLTRRightAlignParagraph, reporter) {

    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_ArabicRectsLTRRightAlignParagraph.png");
    const char* text = "Helloبمباركة التقليدية قام عن. تصفح يد ";
    const size_t len = strlen(text);

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

    REPORTER_ASSERT(reporter, boxes.size() == 2ull); // DIFF
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 561.5f, EPSILON100));         // DIFF
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), -0.27f, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 587.74f, EPSILON100));       // DIFF
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 44, EPSILON100));
}

UNIX_ONLY_TEST(SkParagraph_GetGlyphPositionAtCoordinateParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;
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
    textStyle.setFontStyle(SkFontStyle(SkFontStyle::kNormal_Weight, SkFontStyle::kNormal_Width,
                                   SkFontStyle::kUpright_Slant));
    textStyle.setFontSize(50);
    textStyle.setLetterSpacing(1);
    textStyle.setWordSpacing(5);
    textStyle.setHeight(1);
    textStyle.setColor(SK_ColorBLACK);

    ParagraphBuilderImpl builder(paragraphStyle, fontCollection);
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

UNIX_ONLY_TEST(SkParagraph_GetRectsForRangeParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_GetRectsForRangeParagraph.png");
    const char* text =
            "12345,  \"67890\" 12345 67890 12345 67890 12345 67890 12345 67890 12345 "
            "67890 12345";
    const size_t len = strlen(text);

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
    builder.addText(text, len);
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
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 450.1875f, EPSILON20));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 0.40625f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 519.47266f, EPSILON20));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 59, EPSILON100));
    }
    {
        auto result = paragraph->getRectsForRange(21, 21, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.empty());
    }
}

UNIX_ONLY_TEST(SkParagraph_GetRectsForRangeTight, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_GetRectsForRangeTight.png");
    const char* text =
            "(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)("
            "　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)("
            "　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)";
    const size_t len = strlen(text);
/*
(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)
    S    S     S   S   S    S     S   S   S    S     S   S   S    S     S   S   S    S     S   S   S    S     S   S   S    S     S   S   S    S     S   S   S    S     S   S   S    S     S   S   S    S     S   S   S    S     S   S   S    S     S   S   S    S     S   S   S    S     S   S   S    S     S   S   S    S     S   S   S    S     S   S   S    S     S   S   S    S     S   S
 G  G G  G  G  G  GGG  G G  G  G  G  GGG  G G  G  G  G  GGG  G G  G  G  G  GGG  G G  G  G  G  GGG  G G  G  G  G  GGG  G G  G  G  G  GGG  G G  G  G  G  GGG  G G  G  G  G  GGG  G G  G  G  G  GGG  G G  G  G  G  GGG  G G  G  G  G  GGG  G G  G  G  G  GGG  G G  G  G  G  GGG  G G  G  G  G  GGG  G G  G  G  G  GGG  G G  G  G  G  GGG  G G  G  G  G  GGG  G G  G  G  G  GGG  G G  G  G  G  GG
 W                  W                  W                  W                  W                  W                  W                  W                  W                  W                  W                  W                  W                  W                  W                  W                  W                  W                  W                  W

 */
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
    builder.addText(text, len);
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
UNIX_ONLY_TEST(SkParagraph_GetRectsForRangeIncludeLineSpacingMiddle, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;
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
    textStyle.setFontStyle(SkFontStyle(SkFontStyle::kMedium_Weight, SkFontStyle::kNormal_Width,
                                       SkFontStyle::kUpright_Slant));

    ParagraphBuilderImpl builder(paragraphStyle, fontCollection);
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
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 190.00781f, EPSILON20));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 16.946615f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 508.0625f, EPSILON20));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 88.473305f, EPSILON100));
    }
    {
        auto result = paragraph->getRectsForRange(30, 150, heightStyle, widthStyle);
        canvas.drawRects(SK_ColorRED, result);
        REPORTER_ASSERT(reporter, result.size() == 8);

        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 190.00781f, EPSILON20));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 88.473305f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 525.687f, EPSILON20));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 168.47331f, EPSILON100));

        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[1].rect.left(), 525.687f, EPSILON20));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[1].rect.top(), 88.473305f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[1].rect.right(), 570.02344f, EPSILON20));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[1].rect.bottom(), 168.47331f, EPSILON100));

        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[2].rect.left(), 0, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[2].rect.top(), 168.47331f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[2].rect.right(), 531.574f, EPSILON20));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[2].rect.bottom(), 248.47331f, EPSILON100));

        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[3].rect.left(), 531.574f, EPSILON20));
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
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 463.72656f, EPSILON20));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 16.946615f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 530.23047f, EPSILON20));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 88.473305f, EPSILON100));

        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[1].rect.left(), 530.23047f, EPSILON20));
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
UNIX_ONLY_TEST(SkParagraph_GetRectsForRangeIncludeLineSpacingTop, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;
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
    textStyle.setFontStyle(SkFontStyle(SkFontStyle::kMedium_Weight, SkFontStyle::kNormal_Width,
                                       SkFontStyle::kUpright_Slant));

    ParagraphBuilderImpl builder(paragraphStyle, fontCollection);
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
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 463.72656f, EPSILON20));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 16.946615f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 530.23047f, EPSILON20));
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
UNIX_ONLY_TEST(SkParagraph_GetRectsForRangeIncludeLineSpacingBottom, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;
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
    textStyle.setFontStyle(SkFontStyle(SkFontStyle::kMedium_Weight, SkFontStyle::kNormal_Width,
                                       SkFontStyle::kUpright_Slant));

    ParagraphBuilderImpl builder(paragraphStyle, fontCollection);
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

        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 190.007f, EPSILON20));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 96.946f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 525.687f, EPSILON20));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 176.946f, EPSILON100));

        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[1].rect.left(), 525.687f, EPSILON20));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[1].rect.top(), 96.946f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[1].rect.right(), 570.023f, EPSILON20));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[1].rect.bottom(), 176.946f, EPSILON100));

        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[2].rect.left(), 0, EPSILON20));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[2].rect.top(), 176.946f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[2].rect.right(), 531.574f, EPSILON20));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[2].rect.bottom(), 256.946f, EPSILON100));

        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[3].rect.left(), 531.574f, EPSILON20));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[3].rect.top(), 176.946f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[3].rect.right(), 570.023f, EPSILON20));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[3].rect.bottom(), 256.946f, EPSILON100));

        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[4].rect.left(), 0, EPSILON20));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[4].rect.top(), 256.946f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[4].rect.right(), 570.023f, EPSILON20));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[4].rect.bottom(), 336.946f, EPSILON100));

        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[5].rect.left(), 0, EPSILON20));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[5].rect.top(), 336.946f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[5].rect.right(), 570.023f, EPSILON20));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[5].rect.bottom(), 416.946f, EPSILON100));
    }
    {
        auto result = paragraph->getRectsForRange(19, 22, heightStyle, widthStyle);
        canvas.drawRects(SK_ColorBLACK, result);
        REPORTER_ASSERT(reporter, result.size() == 2); // DIFF
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 463.726f, EPSILON20));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 16.946f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 530.230f, EPSILON20));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 96.946f, EPSILON100));

        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[1].rect.left(), 530.230f, EPSILON20));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[1].rect.top(), 16.946f, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[1].rect.right(), 570.023f, EPSILON20));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[1].rect.bottom(), 96.946f, EPSILON100));
    }
    {
        auto result = paragraph->getRectsForRange(21, 21, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.empty());
    }
}

// This is the test I cannot accommodate
// Any text range gets a smallest glyph rectangle
DEF_TEST_DISABLED(SkParagraph_GetRectsForRangeIncludeCombiningCharacter, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_GetRectsForRangeIncludeCombiningCharacter.png");
    const char* text = "ดีสวัสดีชาวโลกที่น่ารัก";
    const size_t len = strlen(text);

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
    builder.addText(text, len);
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
        REPORTER_ASSERT(reporter, first.size() == 0 && second.size() == 0 && third.size() == 1 && last.size() == 1);
        REPORTER_ASSERT(reporter, third[0].rect == last[0].rect);
    }
}

// Checked: NO DIFF
UNIX_ONLY_TEST(SkParagraph_GetRectsForRangeCenterParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_GetRectsForRangeCenterParagraph.png");
    // Minikin uses a hard coded list of unicode characters that he treats as invisible - as spaces.
    // It's absolutely wrong - invisibility is a glyph attribute, not character/grapheme.
    // Any attempt to substitute one for another leads to errors
    // (for instance, some fonts can use these hard coded characters for something that is visible)
    const char* text = "01234  　 ";   // includes ideographic space and english space.
    const size_t len = strlen(text);

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
    builder.addText(text, len);
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
UNIX_ONLY_TEST(SkParagraph_GetRectsForRangeCenterParagraphNewlineCentered, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_GetRectsForRangeCenterParagraphNewlineCentered.png");
    const char* text = "01234\n";
    const size_t len = strlen(text);

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
    builder.addText(text, len);
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
        canvas.drawRects(SK_ColorRED, result);
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
UNIX_ONLY_TEST(SkParagraph_GetRectsForRangeCenterMultiLineParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_GetRectsForRangeCenterMultiLineParagraph.png");
    const char* text = "01234  　 \n0123　        "; // includes ideographic space and english space.
    const size_t len = strlen(text);

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
    builder.addText(text, len);
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
UNIX_ONLY_TEST(SkParagraph_GetRectsForRangeStrut, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_GetRectsForRangeStrut.png");
    const char* text = "Chinese 字典";
    const size_t len = strlen(text);

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
    builder.addText(text, len);
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
UNIX_ONLY_TEST(SkParagraph_GetRectsForRangeStrutFallback, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_GetRectsForRangeStrutFallback.png");
    const char* text = "Chinese 字典";
    const size_t len = strlen(text);

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
    builder.addText(text, len);
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
UNIX_ONLY_TEST(SkParagraph_GetWordBoundaryParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_GetWordBoundaryParagraph.png");
    const char* text = "12345  67890 12345 67890 12345 67890 12345 "
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

    ParagraphBuilderImpl builder(paragraphStyle, fontCollection);
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
    REPORTER_ASSERT(reporter, paragraph->getWordBoundary(outLen - 1) == SkRange<size_t>(outLen - 5, outLen));
}

// Checked: DIFF (unclear)
UNIX_ONLY_TEST(SkParagraph_SpacingParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
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
    impl->lines().begin()->scanStyles(StyleType::kLetterSpacing,
       [&](TextRange textRange, const TextStyle& style, const TextLine::ClipContext& context) {
          ++index;
          return true;
        });
    REPORTER_ASSERT(reporter, index == 4);
    index = 0;
    impl->lines().begin()->scanStyles(StyleType::kWordSpacing,
        [&](TextRange textRange, const TextStyle& style, const TextLine::ClipContext& context) {
          ++index;
          return true;
        });
    REPORTER_ASSERT(reporter, index == 4);
}

// Checked: NO DIFF
UNIX_ONLY_TEST(SkParagraph_LongWordParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_LongWordParagraph.png");
    const char* text =
            "A "
            "veryverylongwordtoseewherethiswillwraporifitwillatallandifitdoesthenthat"
            "wouldbeagoodthingbecausethebreakingisworking.";
    const size_t len = strlen(text);

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
    builder.addText(text, len);
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
UNIX_ONLY_TEST(SkParagraph_KernScaleParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
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
    builder.addText(text1, strlen(text1));
    builder.pushStyle(text_style);
    builder.addText("A", 1);
    builder.pushStyle(text_style);
    builder.addText("V", 1);
    text_style.setFontSize(14 / scale);
    builder.pushStyle(text_style);
    builder.addText(text2, strlen(text2));
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
UNIX_ONLY_TEST(SkParagraph_NewlineParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_NewlineParagraph.png");
    const char* text =
            "line1\nline2 test1 test2 test3 test4 test5 test6 test7\nline3\n\nline4 "
            "test1 test2 test3 test4";
    const size_t len = strlen(text);

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
    builder.addText(text, len);
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
    REPORTER_ASSERT(reporter, impl->lines()[5].offset().fY == 296);
    REPORTER_ASSERT(reporter, impl->lines()[6].offset().fY == 366);
}

// TODO: Fix underline
UNIX_ONLY_TEST(SkParagraph_EmojiParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_EmojiParagraph.png");
  const char* text =
      "😀😃😄😁😆😅😂🤣☺😇🙂😍😡😟😢😻👽💩👍👎🙏👌👋👄👁👦👼👨‍🚀👨‍🚒🙋‍♂️👳👨‍👨‍👧‍👧\
      💼👡👠☂🐶🐰🐻🐼🐷🐒🐵🐔🐧🐦🐋🐟🐡🕸🐌🐴🐊🐄🐪🐘🌸🌏🔥🌟🌚🌝💦💧\
      ❄🍕🍔🍟🥝🍱🕶🎩🏈⚽🚴‍♀️🎻🎼🎹🚨🚎🚐⚓🛳🚀🚁🏪🏢🖱⏰📱💾💉📉🛏🔑🔓\
      📁🗓📊❤💯🚫🔻♠♣🕓❗🏳🏁🏳️‍🌈🇮🇹🇱🇷🇺🇸🇬🇧🇨🇳🇧🇴";
    const size_t len = strlen(text);

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Noto Color Emoji")});
    text_style.setFontSize(50);
    text_style.setDecoration(TextDecoration::kUnderline);
    text_style.setColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text, len);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);
    paragraph->paint(canvas.get(), 0, 0);

    REPORTER_ASSERT(reporter, paragraph->unresolvedGlyphs() == 0);

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
UNIX_ONLY_TEST(SkParagraph_EmojiMultiLineRectsParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_EmojiMultiLineRectsParagraph.png");
  const char* text =
      "👩‍👩‍👦👩‍👩‍👧‍👧🇺🇸👩‍👩‍👦👩‍👩‍👧‍👧i🇺🇸👩‍👩‍👦👩‍👩‍👧‍👧🇺🇸👩‍👩‍👦👩‍👩‍👧‍👧🇺🇸"
      "👩‍👩‍👦👩‍👩‍👧‍👧🇺🇸👩‍👩‍👦👩‍👩‍👧‍👧🇺🇸👩‍👩‍👦👩‍👩‍👧‍👧🇺🇸👩‍👩‍👦👩‍👩‍👧‍👧🇺🇸"
      "👩‍👩‍👦👩‍👩‍👧‍👧🇺🇸👩‍👩‍👦👩‍👩‍👧‍👧🇺🇸👩‍👩‍👦👩‍👩‍👧‍👧🇺🇸👩‍👩‍👦👩‍👩‍👧‍👧🇺🇸"
      "👩‍👩‍👦👩‍👩‍👧‍👧🇺🇸👩‍👩‍👦👩‍👩‍👧‍👧🇺🇸👩‍👩‍👦👩‍👩‍👧‍👧🇺🇸👩‍👩‍👦👩‍👩‍👧‍👧🇺🇸"
      "❄🍕🍔🍟🥝🍱🕶🎩🏈⚽🚴‍♀️🎻🎼🎹🚨🚎🚐⚓🛳🚀🚁🏪🏢🖱⏰📱💾💉📉🛏🔑🔓"
      "📁🗓📊❤💯🚫🔻♠♣🕓❗🏳🏁🏳️‍🌈🇮🇹🇱🇷🇺🇸🇬🇧🇨🇳🇧🇴";
    const size_t len = strlen(text);

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Noto Color Emoji")});
    text_style.setFontSize(50);
    text_style.setColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text, len);
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
    REPORTER_ASSERT(reporter, result.size() == 0);
    // We changed the selection algorithm and now the selection is empty
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

// Checked: DIFF (line breaking)
UNIX_ONLY_TEST(SkParagraph_RepeatLayoutParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_RepeatLayoutParagraph.png");
    const char* text =
            "Sentence to layout at diff widths to get diff line counts. short words "
            "short words short words short words short words short words short words "
            "short words short words short words short words short words short words "
            "end";
    const size_t len = strlen(text);

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setFontSize(31);
    text_style.setColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text, len);
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
UNIX_ONLY_TEST(SkParagraph_Ellipsize, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_Ellipsize.png");
    const char* text =
            "This is a very long sentence to test if the text will properly wrap "
            "around and go to the next line. Sometimes, short sentence. Longer "
            "sentences are okay too because they are nessecary. Very short. ";
    const size_t len = strlen(text);

    ParagraphStyle paragraph_style;
    paragraph_style.setMaxLines(1);
    std::u16string ellipsis = u"\u2026";
    paragraph_style.setEllipsis(ellipsis);
    std::u16string e = paragraph_style.getEllipsisUtf16();
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text, len);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);
    paragraph->paint(canvas.get(), 0, 0);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());

    // Check that the ellipsizer limited the text to one line and did not wrap to a second line.
    REPORTER_ASSERT(reporter, impl->lines().size() == 1);

    auto& line = impl->lines()[0];
    REPORTER_ASSERT(reporter, line.ellipsis() != nullptr);
    REPORTER_ASSERT(reporter, impl->runs().size() == 1);
}

// Checked: NO DIFF
UNIX_ONLY_TEST(SkParagraph_UnderlineShiftParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
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

    ParagraphBuilderImpl builder1(paragraph_style, fontCollection);
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
        auto r1 = paragraph->getRectsForRange(i, i + 1, RectHeightStyle::kMax, RectWidthStyle::kTight);
        auto r2 = paragraph1->getRectsForRange(i, i + 1, RectHeightStyle::kMax, RectWidthStyle::kTight);

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
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_SimpleShadow.png");
    const char* text = "Hello World Text Dialog";
    const size_t len = strlen(text);

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

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
           [&](TextRange textRange, const TextStyle& style, const TextLine::ClipContext& context) {
                REPORTER_ASSERT(reporter, index == 0 && style.equals(text_style));
                ++index;
                return true;
            });
    }
}

// Checked: NO DIFF
UNIX_ONLY_TEST(SkParagraph_ComplexShadow, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_ComplexShadow.png");
    const char* text = "Text Chunk ";
    const size_t len = strlen(text);

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

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
           [&](TextRange textRange, const TextStyle& style, const TextLine::ClipContext& context) {
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
UNIX_ONLY_TEST(SkParagraph_BaselineParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_BaselineParagraph.png");
    const char* text =
            "左線読設Byg後碁給能上目秘使約。満毎冠行来昼本可必図将発確年。今属場育"
            "図情闘陰野高備込制詩西校客。審対江置講今固残必託地集済決維駆年策。立得";
    const size_t len = strlen(text);

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
    builder.addText(text, len);
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

// Checked: NO DIFF (number of runs only)
UNIX_ONLY_TEST(SkParagraph_FontFallbackParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
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
    builder.addText(text1, strlen(text1));

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
    builder.addText(text2, strlen(text2));

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
    builder.addText(text3, strlen(text3));

    builder.pop();

    auto paragraph = builder.Build();
    REPORTER_ASSERT(reporter, paragraph->unresolvedGlyphs() == -1); // Not shaped yet
    paragraph->layout(TestCanvasWidth);
    paragraph->paint(canvas.get(), 10.0, 15.0);

    size_t spaceRun = 1;
    REPORTER_ASSERT(reporter, paragraph->unresolvedGlyphs() == 2); // From the text1 ("字典" - excluding the last space)

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());

    REPORTER_ASSERT(reporter, impl->runs().size() == 6 + spaceRun);

    // Font resolution in Skia produces 6 runs because 2 parts of "Roboto 字典 " have different
    // script (Minikin merges the first 2 into one because of unresolved)
    // [Apple + Unresolved + ' '] 0, 1, 2
    // [Apple + Noto] 3, 4
    // [Apple + Han] 5, 6
    auto robotoAdvance = impl->runs()[0].advance().fX +
                         impl->runs()[1].advance().fX;
    robotoAdvance += impl->runs()[2].advance().fX;

    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(robotoAdvance, 64.199f, EPSILON50));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(impl->runs()[2 + spaceRun].advance().fX, 139.125f, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(impl->runs()[3 + spaceRun].advance().fX, 27.999f, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(impl->runs()[4 + spaceRun].advance().fX, 62.248f, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(impl->runs()[5 + spaceRun].advance().fX, 27.999f, EPSILON100));

    // When a different font is resolved, then the metrics are different.
    REPORTER_ASSERT(reporter, impl->runs()[3 + spaceRun].correctAscent() != impl->runs()[5 + spaceRun].correctAscent());
    REPORTER_ASSERT(reporter, impl->runs()[3 + spaceRun].correctDescent() != impl->runs()[5 + spaceRun].correctDescent());
}

// Checked: NO DIFF
UNIX_ONLY_TEST(SkParagraph_StrutParagraph1, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_StrutParagraph1.png");
    // The chinese extra height should be absorbed by the strut.
    const char* text = "01234満毎冠p来É本可\nabcd\n満毎É行p昼本可";
    const size_t len = strlen(text);

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
    builder.addText(text, len);
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
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 0, EPSILON100));
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
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 0, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 500, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 95, EPSILON100));
    }
    {
        auto boxes = paragraph->getRectsForRange(14, 16, rect_height_max_style, rect_width_style);
        canvas.drawRects(SK_ColorRED, boxes);
        REPORTER_ASSERT(reporter, boxes.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 0, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 190, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 100, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 285, EPSILON100));
    }
    {
        auto boxes = paragraph->getRectsForRange(20, 25, rect_height_max_style, rect_width_style);
        canvas.drawRects(SK_ColorRED, boxes);
        REPORTER_ASSERT(reporter, boxes.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 50, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 285, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 300, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 380, EPSILON100));
    }
}

// Checked: NO DIFF
UNIX_ONLY_TEST(SkParagraph_StrutParagraph2, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_StrutParagraph2.png");
    // The chinese extra height should be absorbed by the strut.
    const char* text = "01234ABCDEFGH\nabcd\nABCDEFGH";
    const size_t len = strlen(text);

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
    builder.addText(text, len);
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
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 0, EPSILON100));
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
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 0, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 500, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 80, EPSILON100));
    }
    {
        auto boxes = paragraph->getRectsForRange(14, 16, rect_height_max_style, rect_width_style);
        canvas.drawRects(SK_ColorRED, boxes);
        REPORTER_ASSERT(reporter, boxes.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 0, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 160, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 100, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 240, EPSILON100));
    }
    {
        auto boxes = paragraph->getRectsForRange(20, 25, rect_height_max_style, rect_width_style);
        canvas.drawRects(SK_ColorRED, boxes);
        REPORTER_ASSERT(reporter, boxes.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 50, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 240, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 300, EPSILON100));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 320, EPSILON100));
    }
}

// Checked: NO DIFF
UNIX_ONLY_TEST(SkParagraph_StrutParagraph3, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_StrutParagraph3.png");

    // The chinese extra height should be absorbed by the strut.
    const char* text = "01234満毎p行来昼本可\nabcd\n満毎冠行来昼本可";
    const size_t len = strlen(text);

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
    builder.addText(text, len);
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
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 0, epsilon));
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
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 0, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 500, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 60, epsilon));
    }
    {
        auto boxes = paragraph->getRectsForRange(14, 16, rect_height_max_style, rect_width_style);
        canvas.drawRects(SK_ColorRED, boxes);
        REPORTER_ASSERT(reporter, boxes.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 0, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 120, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 100, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 180, epsilon));
    }
    {
        auto boxes = paragraph->getRectsForRange(20, 25, rect_height_max_style, rect_width_style);
        canvas.drawRects(SK_ColorRED, boxes);
        REPORTER_ASSERT(reporter, boxes.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 50, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 180, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 300, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 240, epsilon));
    }
}

// Checked: NO DIFF
UNIX_ONLY_TEST(SkParagraph_StrutForceParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_StrutForceParagraph.png");
    const char* text = "01234満毎冠行来昼本可\nabcd\n満毎冠行来昼本可";
    const size_t len = strlen(text);

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
    builder.addText(text, len);
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
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes3[0].rect.top(), 0, EPSILON100));
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
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes5[0].rect.top(), 0, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes5[0].rect.right(), 500, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes5[0].rect.bottom(), 80, EPSILON100));

    auto boxes6 = paragraph->getRectsForRange(14, 16, rect_height_max_style, rect_width_style);
    canvas.drawRects(SK_ColorRED, boxes6);
    REPORTER_ASSERT(reporter, boxes6.size() == 1);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes6[0].rect.left(), 0, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes6[0].rect.top(), 160, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes6[0].rect.right(), 100, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes6[0].rect.bottom(), 240, EPSILON100));

    auto boxes7 = paragraph->getRectsForRange(20, 25, rect_height_max_style, rect_width_style);
    canvas.drawRects(SK_ColorRED, boxes7);
    REPORTER_ASSERT(reporter, boxes7.size() == 1);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes7[0].rect.left(), 50, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes7[0].rect.top(), 240, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes7[0].rect.right(), 300, EPSILON100));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes7[0].rect.bottom(), 320, EPSILON100));
}

// Checked: NO DIFF
UNIX_ONLY_TEST(SkParagraph_StrutDefaultParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_StrutDefaultParagraph.png");

    const char* text = "01234満毎冠行来昼本可\nabcd\n満毎冠行来昼本可";
    const size_t len = strlen(text);

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
    builder.addText(text, len);
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

UNIX_ONLY_TEST(SkParagraph_FontFeaturesParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_FontFeaturesParagraph.png");

    const char* text = "12ab\n";

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontStyle(SkFontStyle::Italic()); // Regular Roboto doesn't have font features
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

// Not in Minikin
UNIX_ONLY_TEST(SkParagraph_WhitespacesInMultipleFonts, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;
    const char* text = "English English 字典 字典 😀😃😄 😀😃😄";
    const size_t len = strlen(text);

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

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
        auto next  = impl->runs()[i + 1].textRange();
        REPORTER_ASSERT(reporter, first.end == next.start);
    }
}

// Disable until I sort out fonts
DEF_TEST_DISABLED(SkParagraph_JSON1, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;
    const char* text = "👨‍👩‍👧‍👦";
    const size_t len = strlen(text);

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Noto Color Emoji")});
    builder.pushStyle(text_style);
    builder.addText(text, len);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, impl->runs().size() == 1);
    auto& run = impl->runs().front();

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
    REPORTER_ASSERT(reporter, cluster <= 2);
}

// Disable until I sort out fonts
DEF_TEST_DISABLED(SkParagraph_JSON2, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;
    const char* text = "p〠q";
    const size_t len = strlen(text);

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Noto Sans CJK JP")});
    text_style.setColor(SK_ColorBLACK);
    text_style.setFontSize(50);
    builder.pushStyle(text_style);
    builder.addText(text, len);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, impl->runs().size() == 1);

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

    REPORTER_ASSERT(reporter, cluster <= 2);
}

UNIX_ONLY_TEST(SkParagraph_CacheText, reporter) {
    ParagraphCache cache;
    cache.turnOn(true);
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setColor(SK_ColorBLACK);

    auto test = [&](const char* text, int count, bool expectedToBeFound) {
        ParagraphBuilderImpl builder(paragraph_style, fontCollection);
        builder.pushStyle(text_style);
        builder.addText(text, strlen(text));
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

UNIX_ONLY_TEST(SkParagraph_CacheFonts, reporter) {
    ParagraphCache cache;
    cache.turnOn(true);
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();

    TextStyle text_style;
    text_style.setColor(SK_ColorBLACK);

    const char* text = "text";
    const size_t len = strlen(text);

    auto test = [&](int count, bool expectedToBeFound) {
        ParagraphBuilderImpl builder(paragraph_style, fontCollection);
        builder.pushStyle(text_style);
        builder.addText(text, len);
        builder.pop();
        auto paragraph = builder.Build();
        auto impl = static_cast<ParagraphImpl*>(paragraph.get());

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

UNIX_ONLY_TEST(SkParagraph_CacheFontRanges, reporter) {
    ParagraphCache cache;
    cache.turnOn(true);
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
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
        builder.addText(text1, strlen(text1));
        builder.pop();
        text_style.setFontFamilies({SkString(font2)});
        builder.pushStyle(text_style);
        builder.addText(text2, strlen(text2));
        builder.pop();
        auto paragraph = builder.Build();
        auto impl = static_cast<ParagraphImpl*>(paragraph.get());

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

UNIX_ONLY_TEST(SkParagraph_CacheStyles, reporter) {
    ParagraphCache cache;
    cache.turnOn(true);
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setColor(SK_ColorBLACK);

    const char* text = "text";
    const size_t len = strlen(text);

    auto test = [&](int count, bool expectedToBeFound) {
        ParagraphBuilderImpl builder(paragraph_style, fontCollection);
        builder.pushStyle(text_style);
        builder.addText(text, len);
        builder.pop();
        auto paragraph = builder.Build();
        auto impl = static_cast<ParagraphImpl*>(paragraph.get());

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

UNIX_ONLY_TEST(SkParagraph_EmptyParagraphWithLineBreak, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;
    fontCollection->setDefaultFontManager(SkFontMgr::RefDefault());
    fontCollection->enableFontFallback();

    TestCanvas canvas("SkParagraph_EmptyParagraphWithLineBreak.png");

    ParagraphStyle paragraph_style;
    TextStyle text_style;
    text_style.setFontSize(16);
    text_style.setFontFamilies({SkString("Roboto")});
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);
    builder.addText("abc\n\ndef");

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);
    paragraph->paint(canvas.get(), 0, 0);

    // Select a position at the second (empty) line
    auto pos = paragraph->getGlyphPositionAtCoordinate(0, 21);
    REPORTER_ASSERT(reporter, pos.affinity == Affinity::kDownstream && pos.position == 4);
    auto rect = paragraph->getRectsForRange(4, 5, RectHeightStyle::kTight, RectWidthStyle::kTight);
    REPORTER_ASSERT(reporter, rect.size() == 1 && rect[0].rect.width() == 0);
}

UNIX_ONLY_TEST(SkParagraph_NullInMiddleOfText, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;
    fontCollection->setDefaultFontManager(SkFontMgr::RefDefault());
    TestCanvas canvas("SkParagraph_NullInMiddleOfText.png");

    const SkString text("null terminator ->\u0000<- on purpose did you see it?");

    ParagraphStyle paragraph_style;
    TextStyle text_style;
    text_style.setFontSize(16);
    text_style.setFontFamilies({SkString("Roboto")});
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);
    builder.addText(text.c_str(), text.size());

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);
    paragraph->paint(canvas.get(), 0, 0);
}

UNIX_ONLY_TEST(SkParagraph_PlaceholderOnly, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_PlaceholderOnly.png");

    ParagraphStyle paragraph_style;
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    PlaceholderStyle placeholder(0, 0, PlaceholderAlignment::kBaseline, TextBaseline::kAlphabetic, 0);
    builder.addPlaceholder(placeholder);

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);
    auto result = paragraph->getRectsForPlaceholders();
    paragraph->paint(canvas.get(), 0, 0);
}

UNIX_ONLY_TEST(SkParagraph_Fallbacks, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;
    fontCollection->setDefaultFontManager(SkFontMgr::RefDefault(), "Arial");
    TestCanvas canvas("SkParagraph_Fallbacks.png");

    const char* multiScript = "A1!aÀàĀāƁƀḂⱠꜲꬰəͲἀἏЀЖԠꙐꙮՁخ‎ࡔࠇܦআਉઐଘஇఘಧൺඣᭆᯔᮯ᳇ꠈᜅᩌꪈ༇ꥄꡙꫤ᧰៘꧁꧂ᜰᨏᯤᢆᣭᗗꗃⵞ𐒎߷ጩꬤ𖠺‡₩℻Ⅷ↹⋇⏳ⓖ╋▒◛⚧⑆שׁ🅕㊼龜ポ䷤🂡\n";
    const size_t len = strlen(multiScript);

    const char* androidFonts[] = {
        "sans-serif",
        "sans-serif-condensed",
        "serif",
        "monospace",
        "serif-monospace",
        "casual",
        "cursive",
        "sans-serif-smallcaps",
    };

    for (auto& font : androidFonts) {

        ParagraphStyle paragraph_style;
        ParagraphBuilderImpl builder(paragraph_style, fontCollection);

        TextStyle text_style;
        text_style.setColor(SK_ColorBLACK);
        text_style.setLocale(SkString("en_US"));
        text_style.setFontSize(20);

        text_style.setFontFamilies({ SkString(font) });
        builder.pushStyle(text_style);
        builder.addText(multiScript, len);

        builder.pop();

        auto paragraph = builder.Build();
        paragraph->layout(TestCanvasWidth);
        paragraph->paint(canvas.get(), 0, 0);
        canvas.get()->translate(0, paragraph->getHeight() + 10);
    }
}

UNIX_ONLY_TEST(SkParagraph_Bidi1, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;
    fontCollection->setDefaultFontManager(SkFontMgr::RefDefault());
    fontCollection->enableFontFallback();
    TestCanvas canvas("SkParagraph_Bidi1.png");

    std::u16string abc = u"\u202Dabc";
    std::u16string DEF = u"\u202EDEF";
    std::u16string ghi = u"\u202Dghi";
    std::u16string JKL = u"\u202EJKL";
    std::u16string mno = u"\u202Dmno";

    std::u16string abcDEFghiJKLmno = u"\u202Dabc\u202EDEF\u202Dghi\u202EJKL\u202Dmno";

    ParagraphStyle paragraph_style;
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({ SkString("sans-serif")});
    text_style.setFontSize(40);

    text_style.setColor(SK_ColorCYAN);
    text_style.setFontStyle(SkFontStyle(SkFontStyle::kThin_Weight, SkFontStyle::kNormal_Width, SkFontStyle::kUpright_Slant));
    builder.pushStyle(text_style);
    builder.addText(abc);

    text_style.setColor(SK_ColorGREEN);
    text_style.setFontStyle(SkFontStyle(SkFontStyle::kLight_Weight, SkFontStyle::kNormal_Width, SkFontStyle::kUpright_Slant));
    builder.pushStyle(text_style);
    builder.addText(DEF);

    text_style.setColor(SK_ColorYELLOW);
    text_style.setFontStyle(SkFontStyle(SkFontStyle::kNormal_Weight, SkFontStyle::kNormal_Width, SkFontStyle::kUpright_Slant));
    builder.pushStyle(text_style);
    builder.addText(ghi);

    text_style.setColor(SK_ColorMAGENTA);
    text_style.setFontStyle(SkFontStyle(SkFontStyle::kMedium_Weight, SkFontStyle::kNormal_Width, SkFontStyle::kUpright_Slant));
    builder.pushStyle(text_style);
    builder.addText(JKL);

    text_style.setColor(SK_ColorBLUE);
    text_style.setFontStyle(SkFontStyle(SkFontStyle::kBlack_Weight, SkFontStyle::kNormal_Width, SkFontStyle::kUpright_Slant));
    builder.pushStyle(text_style);
    builder.addText(mno);

    auto paragraph = builder.Build();
    paragraph->layout(400);
    paragraph->paint(canvas.get(), 0, 0);
}

UNIX_ONLY_TEST(SkParagraph_Bidi2, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;
    fontCollection->setDefaultFontManager(SkFontMgr::RefDefault());
    fontCollection->enableFontFallback();
    TestCanvas canvas("SkParagraph_Bidi2.png");

    std::u16string abcD = u"\u202Dabc\u202ED";
    std::u16string EFgh = u"EF\u202Dgh";
    std::u16string iJKLmno = u"i\u202EJKL\u202Dmno";

    std::u16string abcDEFghiJKLmno = u"\u202Dabc\u202EDEF\u202Dghi\u202EJKL\u202Dmno";

    ParagraphStyle paragraph_style;
    TextStyle text_style;
    text_style.setFontFamilies({ SkString("sans-serif")});
    text_style.setFontSize(40);

    {
        ParagraphBuilderImpl builder(paragraph_style, fontCollection);
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
        ParagraphBuilderImpl builder(paragraph_style, fontCollection);
        builder.pushStyle(text_style);
        builder.addText(abcDEFghiJKLmno);
        auto paragraph = builder.Build();
        paragraph->layout(360);
        paragraph->paint(canvas.get(), 0, 0);
    }
}

UNIX_ONLY_TEST(SkParagraph_NewlineOnly, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;
    fontCollection->setDefaultFontManager(SkFontMgr::RefDefault());
    TestCanvas canvas("SkParagraph_Newline.png");

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Ahem")});
    text_style.setColor(SK_ColorBLACK);
    StrutStyle strut_style;
    strut_style.setStrutEnabled(false);
    ParagraphStyle paragraph_style;
    paragraph_style.setStrutStyle(strut_style);
    paragraph_style.setTextStyle(text_style);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);
    builder.addText("\n");
    auto paragraph = builder.Build();
    paragraph->layout(1000);
    REPORTER_ASSERT(reporter, paragraph->getHeight() == 28);
}

UNIX_ONLY_TEST(SkParagraph_FontResolutions, reporter) {
    TestCanvas canvas("SkParagraph_FontResolutions.png");

    sk_sp<TestFontCollection> fontCollection =
            sk_make_sp<TestFontCollection>(GetResourcePath("fonts").c_str(), false);
    if (!fontCollection->fontsFound()) return;

    if (!fontCollection->addFontFromFile("abc/abc.ttf", "abc")) {
        return;
    }
    if (!fontCollection->addFontFromFile("abc/abc+grave.ttf", "abc+grave")) {
        return;
    }
    if (!fontCollection->addFontFromFile("abc/abc_agrave.ttf", "abc_agrave")) {
        return;
    }

    TextStyle text_style;
    text_style.setFontFamilies({SkString("abc")});
    text_style.setFontSize(50);

    ParagraphStyle paragraph_style;
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    text_style.setFontFamilies({SkString("abc"), SkString("abc+grave")});
    text_style.setColor(SK_ColorBLUE);
    builder.pushStyle(text_style);
    builder.addText(u"a\u0300");
    text_style.setColor(SK_ColorMAGENTA);
    builder.pushStyle(text_style);
    builder.addText(u"à");

    text_style.setFontFamilies({SkString("abc"), SkString("abc_agrave")});

    text_style.setColor(SK_ColorRED);
    builder.pushStyle(text_style);
    builder.addText(u"a\u0300");
    text_style.setColor(SK_ColorGREEN);
    builder.pushStyle(text_style);
    builder.addText(u"à");

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, impl->runs().size() == 2);

    REPORTER_ASSERT(reporter, impl->runs().front().size() == 4);
    REPORTER_ASSERT(reporter, impl->runs().front().glyphs()[0] == impl->runs().front().glyphs()[2]);
    REPORTER_ASSERT(reporter, impl->runs().front().glyphs()[1] == impl->runs().front().glyphs()[3]);

    REPORTER_ASSERT(reporter, impl->runs().back().size() == 2);
    REPORTER_ASSERT(reporter, impl->runs().back().glyphs()[0] == impl->runs().back().glyphs()[1]);

    paragraph->paint(canvas.get(), 100, 100);
}

UNIX_ONLY_TEST(SkParagraph_FontStyle, reporter) {
    TestCanvas canvas("SkParagraph_FontStyle.png");

    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>(GetResourcePath("fonts").c_str(), false, true);
    if (!fontCollection->fontsFound()) return;

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setColor(SK_ColorBLACK);
    text_style.setFontSize(20);
    SkFontStyle fs = SkFontStyle(
        SkFontStyle::Weight::kLight_Weight,
        SkFontStyle::Width::kNormal_Width,
        SkFontStyle::Slant::kUpright_Slant
    );
    text_style.setFontStyle(fs);
    ParagraphStyle paragraph_style;
    paragraph_style.setTextStyle(text_style);
    TextStyle boldItalic;
    boldItalic.setFontFamilies({SkString("Roboto")});
    boldItalic.setColor(SK_ColorRED);
    SkFontStyle bi = SkFontStyle(
        SkFontStyle::Weight::kBold_Weight,
        SkFontStyle::Width::kNormal_Width,
        SkFontStyle::Slant::kItalic_Slant
    );
    boldItalic.setFontStyle(bi);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);
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

    auto dir = "/usr/local/google/home/jlavrova/Sources/flutter/engine/src/out/host_debug_unopt_x86/gen/flutter/third_party/txt/assets";
    sk_sp<TestFontCollection> fontCollection =
            sk_make_sp<TestFontCollection>(dir, /*GetResourcePath("fonts").c_str(), */ false);
    if (!fontCollection->fontsFound()) return;


    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setColor(SK_ColorGRAY);
    text_style.setFontSize(14);
    SkFontStyle b = SkFontStyle(
        SkFontStyle::Weight::kNormal_Weight,
        SkFontStyle::Width::kNormal_Width,
        SkFontStyle::Slant::kUpright_Slant
    );
    text_style.setFontStyle(b);
    ParagraphStyle paragraph_style;
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);
    builder.pushStyle(text_style);
    builder.addText("Eat0 apple0 pies0 | Eat1 apple1 pies1 | Eat2 apple2 pies2");
    auto paragraph = builder.Build();
    paragraph->layout(380);
    paragraph->paint(canvas.get(), 0, 0);
}

UNIX_ONLY_TEST(SkParagraph_Ellipsis, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;
    fontCollection->setDefaultFontManager(SkFontMgr::RefDefault());
    TestCanvas canvas("SkParagraph_Ellipsis.png");

    const char* text = "This\n"
                       "is a wrapping test. It should wrap at manual newlines, and if softWrap is true, also at spaces.";
    TextStyle text_style;
    text_style.setFontFamilies({SkString("Ahem")});
    text_style.setColor(SK_ColorBLACK);
    text_style.setFontSize(10);

    auto relayout = [&](size_t lines, bool ellipsis,
            SkScalar width, SkScalar height, SkScalar minWidth, SkScalar maxWidth, SkColor bg) {
        ParagraphStyle paragraph_style;
        SkPaint paint;
        paint.setColor(bg);
        text_style.setForegroundColor(paint);
        paragraph_style.setTextStyle(text_style);
        paragraph_style.setMaxLines(lines);
        if (ellipsis) {
            paragraph_style.setEllipsis(u"\u2026");
        }
        ParagraphBuilderImpl builder(paragraph_style, fontCollection);
        builder.addText(text);
        auto paragraph = builder.Build();
        paragraph->layout(50);
        paragraph->paint(canvas.get(), 0, 0);
        canvas.get()->translate(50, paragraph->getHeight() + 10);
        auto result = paragraph->getRectsForRange(0, strlen(text), RectHeightStyle::kTight, RectWidthStyle::kTight);
        SkPaint background;
        background.setColor(SK_ColorRED);
        background.setStyle(SkPaint::kStroke_Style);
        background.setAntiAlias(true);
        background.setStrokeWidth(1);
        canvas.get()->drawRect(result.front().rect, background);

        SkASSERT(width == paragraph->getMaxWidth());
        SkASSERT(height == paragraph->getHeight());
        SkASSERT(minWidth == paragraph->getMinIntrinsicWidth());
        SkASSERT(maxWidth == paragraph->getMaxIntrinsicWidth());
    };

    SkPaint paint;
    paint.setColor(SK_ColorLTGRAY);
    canvas.get()->drawRect(SkRect::MakeXYWH(0, 0, 50, 500), paint);

    relayout(1, false, 50, 10, 950, 950, SK_ColorRED);
    relayout(3, false, 50, 30,  90, 950, SK_ColorBLUE);
    relayout(std::numeric_limits<size_t>::max(), false, 50, 200,  90, 950, SK_ColorGREEN);

    relayout(1, true, 50, 10, 950, 950, SK_ColorYELLOW);
    relayout(3, true, 50, 30,  90, 950, SK_ColorMAGENTA);
    relayout(std::numeric_limits<size_t>::max(), true, 50, 20,  950, 950, SK_ColorCYAN);

    relayout(1, false, 50, 10, 950, 950, SK_ColorRED);
    relayout(3, false, 50, 30,  90, 950, SK_ColorBLUE);
    relayout(std::numeric_limits<size_t>::max(), false, 50, 200,  90, 950, SK_ColorGREEN);
}

UNIX_ONLY_TEST(SkParagraph_MemoryLeak, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;
    fontCollection->setDefaultFontManager(SkFontMgr::RefDefault());

    std::string text;
    for (size_t i = 0; i < 10; i++)
	{
		SkPaint paint;
		paint.setAntiAlias(true);
		paint.setColor(SK_ColorBLACK);

		TextStyle textStyle;
		textStyle.setForegroundColor(paint);
		textStyle.setFontFamilies({ SkString("Roboto") });

		ParagraphStyle paragraphStyle;
		paragraphStyle.setTextStyle(textStyle);

		ParagraphBuilderImpl builder(paragraphStyle, fontCollection);
		text += "Text ";
		builder.addText(text.c_str());

		auto paragraph = builder.Build();
		paragraph->layout(100);

		//used to add a delay so I can monitor memory usage
		//std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
};

UNIX_ONLY_TEST(SkParagraph_FormattingInfinity, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;
    fontCollection->setDefaultFontManager(SkFontMgr::RefDefault());
    TestCanvas canvas("SkParagraph_FormattingInfinity.png");

    const char* text = "Some text\nAnother line";

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(SK_ColorBLACK);

    TextStyle textStyle;
    textStyle.setForegroundColor(paint);
    textStyle.setFontFamilies({ SkString("Roboto") });
    ParagraphStyle paragraphStyle;
    paragraphStyle.setTextStyle(textStyle);

    auto draw = [&](const char* prefix, TextAlign textAlign) {
        paragraphStyle.setTextAlign(textAlign);
        ParagraphBuilderImpl builder(paragraphStyle, fontCollection);
        builder.addText(text);
        auto paragraph = builder.Build();
        paragraph->layout(SK_ScalarInfinity);
        paragraph->paint(canvas.get(), 0, 0);
        canvas.get()->translate(0, 100);
    };

    draw("left", TextAlign::kLeft);
    draw("right", TextAlign::kRight);
    draw("center", TextAlign::kCenter);
    draw("justify", TextAlign::kJustify);
};

UNIX_ONLY_TEST(SkParagraph_Infinity, reporter) {
    SkASSERT(nearlyEqual(1, SK_ScalarInfinity) == false);
    SkASSERT(nearlyEqual(1, SK_ScalarNegativeInfinity) == false);
    SkASSERT(nearlyEqual(1, SK_ScalarNaN) == false);

    SkASSERT(nearlyEqual(SK_ScalarInfinity, SK_ScalarInfinity) == true);
    SkASSERT(nearlyEqual(SK_ScalarInfinity, SK_ScalarNegativeInfinity) == false);
    SkASSERT(nearlyEqual(SK_ScalarInfinity, SK_ScalarNaN) == false);

    SkASSERT(nearlyEqual(SK_ScalarNegativeInfinity, SK_ScalarInfinity) == false);
    SkASSERT(nearlyEqual(SK_ScalarNegativeInfinity, SK_ScalarNegativeInfinity) == true);
    SkASSERT(nearlyEqual(SK_ScalarNegativeInfinity, SK_ScalarNaN) == false);

    SkASSERT(nearlyEqual(SK_ScalarNaN, SK_ScalarInfinity) == false);
    SkASSERT(nearlyEqual(SK_ScalarNaN, SK_ScalarNegativeInfinity) == false);
    SkASSERT(nearlyEqual(SK_ScalarNaN, SK_ScalarNaN) == false);
};

UNIX_ONLY_TEST(SkParagraph_LineMetrics, reporter) {

    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;

    TestCanvas canvas("SkParagraph_LineMetrics.png");

    const char* text = "One line of text\n";
    const size_t len = strlen(text);

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setColor(SK_ColorBLACK);

    text_style.setFontSize(8);
    builder.pushStyle(text_style);
    builder.addText(text, len);
    builder.pop();

    text_style.setFontSize(12);
    builder.pushStyle(text_style);
    builder.addText(text, len);
    builder.pop();

    text_style.setFontSize(18);
    builder.pushStyle(text_style);
    builder.addText(text, len);
    builder.pop();

    text_style.setFontSize(30);
    builder.pushStyle(text_style);
    builder.addText(text, len - 1); // Skip the last \n
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);

    std::vector<LineMetrics> metrics;
    paragraph->getLineMetrics(metrics);

    SkDEBUGCODE(auto impl = static_cast<ParagraphImpl*>(paragraph.get());)
    SkASSERT(metrics.size() == impl->lines().size());
    for (size_t i = 0; i < metrics.size(); ++i) {
        SkDEBUGCODE(auto& line = impl->lines()[i];)
        SkDEBUGCODE(auto baseline = metrics[i].fBaseline;)
        SkDEBUGCODE(auto top = line.offset().fY;)
        SkDEBUGCODE(auto bottom = line.offset().fY + line.height();)
        SkASSERT( baseline > top && baseline <= bottom);
    }

    paragraph->paint(canvas.get(), 0, 0);
    auto rects = paragraph->getRectsForRange(0, len * 4, RectHeightStyle::kMax, RectWidthStyle::kTight);

    SkPaint red;
    red.setColor(SK_ColorRED);
    red.setStyle(SkPaint::kStroke_Style);
    red.setAntiAlias(true);
    red.setStrokeWidth(1);

    for (auto& rect : rects) {
        canvas.get()->drawRect(rect.rect, red);
    }

    SkPaint green;
    green.setColor(SK_ColorGREEN);
    green.setStyle(SkPaint::kStroke_Style);
    green.setAntiAlias(true);
    green.setStrokeWidth(1);
    for (auto& metric : metrics) {
        auto x0 = 0.0;
        auto x1 = metric.fWidth;
        auto y = metric.fBaseline;
        canvas.get()->drawLine(x0, y, x1, y, green);
    }
};

UNIX_ONLY_TEST(SkParagraph_PlaceholderHeightInf, reporter) {
    TestCanvas canvas("SkParagraph_PlaceholderHeightInf.png");

    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;

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
    paragraph_style.setDrawOptions(DrawOptions::kRecord);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);
    builder.pushStyle(text_style);
    builder.addText("Limited by budget");
    builder.addPlaceholder(placeholder_style);
    auto paragraph = builder.Build();
    paragraph->layout(SK_ScalarInfinity);
    paragraph->paint(canvas.get(), 0, 0);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, SkScalarIsFinite(impl->getPicture()->cullRect().height()));
    REPORTER_ASSERT(reporter, SkScalarIsFinite(impl->getPicture()->cullRect().width()));
}

UNIX_ONLY_TEST(SkParagraph_LineMetricsTextAlign, reporter) {

    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;

    TestCanvas canvas("SkParagraph_LineMetricsTextAlign.png");

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setColor(SK_ColorBLACK);

    auto layout = [&](TextAlign text_align) -> std::pair<SkScalar, SkScalar> {
        paragraph_style.setTextAlign(text_align);
        ParagraphBuilderImpl builder(paragraph_style, fontCollection);
        builder.pushStyle(text_style);
        builder.addText("Some text that takes more than 200 px");
        auto paragraph = builder.Build();
        paragraph->layout(200);

        std::vector<LineMetrics> metrics;
        paragraph->getLineMetrics(metrics);
        REPORTER_ASSERT(reporter, metrics.size() > 0);
        return { metrics[0].fLeft, metrics[0].fWidth };
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
    REPORTER_ASSERT(reporter, width[3] > width[0]); // delta == 0
}

UNIX_ONLY_TEST(SkParagraph_FontResolutionInRTL, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>(true);
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_FontResolutionInRTL.png");
    const char* text = " אאא בּבּבּבּ אאאא בּבּ אאא בּבּבּ אאאאא בּבּבּבּ אאאא בּבּבּבּבּ ";
    const size_t len = strlen(text);

    ParagraphStyle paragraph_style;
    paragraph_style.setMaxLines(14);
    paragraph_style.setTextAlign(TextAlign::kRight);
    paragraph_style.setTextDirection(TextDirection::kRtl);
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

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
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_FontResolutionInLTR.png");
    auto text = u"abc \u01A2 \u01A2 def";

    ParagraphStyle paragraph_style;
    paragraph_style.setMaxLines(14);
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

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
    REPORTER_ASSERT(reporter, impl->runs()[0].textRange().width() == 4); // "abc "
    REPORTER_ASSERT(reporter, impl->runs()[1].textRange().width() == 2); // "{unresolved}"
    REPORTER_ASSERT(reporter, impl->runs()[2].textRange().width() == 1); // " "
    REPORTER_ASSERT(reporter, impl->runs()[3].textRange().width() == 2); // "{unresolved}"
    REPORTER_ASSERT(reporter, impl->runs()[4].textRange().width() == 4); // " def"
}

UNIX_ONLY_TEST(SkParagraph_Intrinsic, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;
    SkString text(std::string(3000, 'a'));

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Google Sans")});
    text_style.setFontSize(12.0f);
    text_style.setColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text.c_str());

    auto paragraph = builder.Build();
    paragraph->layout(300000.0f);
    REPORTER_ASSERT(reporter, paragraph->getMinIntrinsicWidth() <= paragraph->getMaxIntrinsicWidth());
}

UNIX_ONLY_TEST(SkParagraph_NoCache1, reporter) {

    ParagraphCache cache;
    cache.turnOn(true);

    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>(true);
    if (!fontCollection->fontsFound()) return;
    TestCanvas canvas("SkParagraph_NoCache1.png");
    // Long arabic text with english spaces
    const char* text =
            "من أسر وإعلان الخاصّة وهولندا،, عل قائمة الضغوط بالمطالبة تلك. الصفحة "
            "من أسر وإعلان الخاصّة وهولندا،, عل قائمة الضغوط بالمطالبة تلك. الصفحة "
            "من أسر وإعلان الخاصّة وهولندا،, عل قائمة الضغوط بالمطالبة تلك. الصفحة "
            "من أسر وإعلان الخاصّة وهولندا،, عل قائمة الضغوط بالمطالبة تلك. الصفحة "
            "من أسر وإعلان الخاصّة وهولندا،, عل قائمة الضغوط بالمطالبة تلك. الصفحة "
            "عل بمباركة التقليدية قام عن. تصفح";

    SkString str;

    ParagraphStyle paragraph_style;
    paragraph_style.setTextDirection(TextDirection::kLtr);
    TextStyle text_style;
    text_style.setFontFamilies({SkString("Ahem")});
    text_style.setFontSize(14);
    text_style.setColor(SK_ColorBLACK);


    auto test = [&](const char* test, const char* text, bool editing) {
        ParagraphBuilderImpl builder(paragraph_style, fontCollection);
        //SkDebugf("test %s:\n", test);
        builder.pushStyle(text_style);
        builder.addText(text);
        builder.pop();

        auto cache = fontCollection->getParagraphCache();
        auto countBefore = cache->count();
        auto paragraph = builder.Build();
        paragraph->layout(TestCanvasWidth);
        auto countAfter = cache->count();
        //paragraph->paint(canvas.get(), 0, 0);

        if (test == nullptr) {
            return;
        }

        REPORTER_ASSERT(reporter, (countBefore == countAfter) == editing);
    };

    str.append(text);
    test("Long arabic text", str.c_str(), false);

    str.append("عل");
    test("+2 character at the end", str.c_str(), true);

    str = SkString(text);
    test("-2 characters from the end", str.c_str(), true);

    str.insert(0, "عل");
    test("+2 character at the start", str.c_str(), true);

    test("-2 characters from the start", text, true);

    // Make sure that different strings are not flagged as editing
    test("different strings", "0123456789 0123456789 0123456789 0123456789 0123456789", false);
}

UNIX_ONLY_TEST(SkParagraph_HeightCalculations, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;

    TestCanvas canvas("SkParagraph_HeightCalculations.png");

    auto draw = [&](TextHeightBehavior hb, const char* text, SkScalar height) {
        ParagraphStyle paragraph_style;
        paragraph_style.setTextHeightBehavior(hb);

        ParagraphBuilderImpl builder(paragraph_style, fontCollection);
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

UNIX_ONLY_TEST(SkParagraph_RTL_With_Styles, reporter) {

    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;

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
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);
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

UNIX_ONLY_TEST(SkParagraph_PositionInsideEmoji, reporter) {

    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;

    TestCanvas canvas("SkParagraph_PositionInsideEmoji.png");

    std::u16string text = u"\U0001f469\u200D\U0001f469\u200D\U0001f467\u200D\U0001f467\U0001f469\U0001f469\U0001f467\U0001f467";

    ParagraphStyle paragraph_style;
    TextStyle text_style;
    text_style.setColor(SK_ColorBLACK);
    text_style.setFontFamilies({SkString("Noto Color Emoji")});
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);
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

    auto family = paragraph->getRectsForRange(0, 11, RectHeightStyle::kTight, RectWidthStyle::kTight);  // 00.0000000 + 17.4699993
    auto face01 = paragraph->getRectsForRange(11, 13, RectHeightStyle::kTight, RectWidthStyle::kTight); // 17.4699993 + 17.4699993
    auto face02 = paragraph->getRectsForRange(13, 15, RectHeightStyle::kTight, RectWidthStyle::kTight); // 34.9399986 + 17.4699993
    auto face03 = paragraph->getRectsForRange(15, 17, RectHeightStyle::kTight, RectWidthStyle::kTight); // 52.4099998 + 17.4699993
    auto face04 = paragraph->getRectsForRange(17, 19, RectHeightStyle::kTight, RectWidthStyle::kTight); // 69.8799973 + 17.4699993

    int32_t words[] = { 11, 13, 15, 17, 19, 21};
    auto j = 0;
    for (auto i :  words) {
        auto rects = paragraph->getRectsForRange(j, i, RectHeightStyle::kTight, RectWidthStyle::kTight);
        if (rects.empty()) {
            continue;
        }
        auto X = rects[0].rect.centerX();
        auto Y = rects[0].rect.centerY();
        auto res1 = paragraph->getGlyphPositionAtCoordinate(X - 5, Y);
        //SkDebugf("[%d:%d) @%f,%f: %d %s\n", j, i, X - 5, Y, res1.position, res1.affinity == Affinity::kDownstream ? "D" : "U");
        auto res2 = paragraph->getGlyphPositionAtCoordinate(X + 5, Y);
        //SkDebugf("[%d:%d) @%f,%f: %d %s\n\n", j, i, X + 5, Y, res2.position, res2.affinity == Affinity::kDownstream ? "D" : "U");
        REPORTER_ASSERT(reporter, i == res2.position && res1.position == j);
        j = i;
    }
}

UNIX_ONLY_TEST(SkParagraph_SingleLineHeight1, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;

    TestCanvas canvas("SkParagraph_SingleLineHeight1.png");

    auto paint = [&](const char* text) {
        ParagraphStyle paragraph_style;
        paragraph_style.setTextHeightBehavior(TextHeightBehavior::kDisableAll);
        paragraph_style.setMaxLines(1);
        ParagraphBuilderImpl builder(paragraph_style, fontCollection);
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
    if (!fontCollection->fontsFound()) return;

    TestCanvas canvas("SkParagraph_SingleLineHeight2.png");

    auto paint = [&](const char* text) {
        ParagraphStyle paragraph_style;
        paragraph_style.setMaxLines(1);
        ParagraphBuilderImpl builder(paragraph_style, fontCollection);
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

UNIX_ONLY_TEST(SkParagraph_PlaceholderWidth, reporter) {

    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;

    TestCanvas canvas("SkParagraph_PlaceholderWidth.png");

    const char* text = "1 23 456 7890"; // 13 * 50 = 650

    ParagraphStyle paragraph_style;
    TextStyle text_style;
    text_style.setColor(SK_ColorBLACK);
    text_style.setFontSize(50);
    text_style.setFontFamilies({SkString("Ahem")});
    PlaceholderStyle placeholder(300, 50, PlaceholderAlignment::kBaseline, TextBaseline::kAlphabetic, 0);

    auto draw = [&](bool withPlaceholder) {
        ParagraphBuilderImpl builder(paragraph_style, fontCollection);
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
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(len1, 300.0f));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(len2, 250.0f));
}

UNIX_ONLY_TEST(SkParagraph_GlyphPositionsInEmptyLines, reporter) {

    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;

    TestCanvas canvas("SkParagraph_GlyphPositionsInEmptyLines");
    ParagraphStyle paragraph_style;
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);
    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto") });
    text_style.setFontSize(20);
    text_style.setColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText("A\n\n");
    builder.pop();
    auto paragraph = builder.Build();
    paragraph->layout(300);
    paragraph->paint(canvas.get(), 0, 0);

    auto res1 = paragraph->
        getGlyphPositionAtCoordinate(paragraph->getMinIntrinsicWidth(),1);
    REPORTER_ASSERT(reporter, res1.position == 1 && res1.affinity == Affinity::kUpstream);

    auto res2 = paragraph->
        getGlyphPositionAtCoordinate(0,paragraph->getHeight() * 0.5);
    REPORTER_ASSERT(reporter, res2.position == 2 && res2.affinity == Affinity::kDownstream);

    auto res3 = paragraph->
        getGlyphPositionAtCoordinate(0,paragraph->getHeight() - 1);
    REPORTER_ASSERT(reporter, res3.position == 3 && res3.affinity == Affinity::kDownstream);
}

UNIX_ONLY_TEST(SkParagraph_RTLGlyphPositions, reporter) {

    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;

    TestCanvas canvas("SkParagraph_RTLGlyphPositions");
    ParagraphStyle paragraph_style;
    paragraph_style.setTextDirection(TextDirection::kRtl);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);
    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto") });
    text_style.setFontSize(20);
    text_style.setColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText("אאאא");
    builder.pop();
    auto paragraph = builder.Build();
    paragraph->layout(500);
    paragraph->paint(canvas.get(), 0, 0);

    auto res1 = paragraph->getGlyphPositionAtCoordinate(0, 0);
    REPORTER_ASSERT(reporter, res1.position == 3 && res1.affinity == Affinity::kDownstream);
/*
    auto width = paragraph->getMinIntrinsicWidth();
    auto letter = width / 4;
    for (size_t i = 0; i < 4; i++) {
        auto left = 500 - letter * (4 - i) + letter * 0.25;
        auto right = left + letter * 0.5;
        auto res1 = paragraph->getGlyphPositionAtCoordinate(left, 1);
        auto res2 = paragraph->getGlyphPositionAtCoordinate(right, 1);

        SkDebugf("%d: %f %d%s %f %d%s\n", i,
           left, res1.position, res1.affinity == Affinity::kUpstream ? "U" : "D",
           right, res2.position, res2.affinity == Affinity::kUpstream ? "U" : "D");
    }
*/
    auto res2 = paragraph->getGlyphPositionAtCoordinate(500, 1);
    REPORTER_ASSERT(reporter, res2.position == 0 && res2.affinity == Affinity::kDownstream);
//    SkDebugf("edges: %f %d%s %f %d%s\n",
//           0.0f, res1.position, res1.affinity == Affinity::kUpstream ? "U" : "D",
//           500.0f, res2.position, res2.affinity == Affinity::kUpstream ? "U" : "D");
}

UNIX_ONLY_TEST(SkParagraph_RTLGlyphPositionsInEmptyLines, reporter) {

    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;

    TestCanvas canvas("SkParagraph_RTLGlyphPositionsInEmptyLines");

    ParagraphStyle paragraph_style;
    paragraph_style.setTextDirection(TextDirection::kRtl);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);
    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto") });
    text_style.setFontSize(20);
    text_style.setColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    //builder.addText("בבבב\n\nאאאא");
    builder.addText("בבבב\n\nאאאא");
    builder.pop();
    auto paragraph = builder.Build();
    paragraph->layout(500);
    paragraph->paint(canvas.get(), 0, 0);

    auto height = paragraph->getHeight();
    auto res1 = paragraph->getGlyphPositionAtCoordinate(0, 0);
    REPORTER_ASSERT(reporter, res1.position == 3 && res1.affinity == Affinity::kDownstream);
    auto res2 = paragraph->getGlyphPositionAtCoordinate(0, height / 2);
    REPORTER_ASSERT(reporter, res2.position == 5 && res2.affinity == Affinity::kDownstream);
    auto res3 = paragraph->getGlyphPositionAtCoordinate(0, height);
    REPORTER_ASSERT(reporter, res3.position == 9 && res3.affinity == Affinity::kDownstream);
}

UNIX_ONLY_TEST(SkParagraph_LTRGlyphPositionsForTrailingSpaces, reporter) {

    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;

    TestCanvas canvas("SkParagraph_LTRGlyphPositionsForTrailingSpaces");

    ParagraphStyle paragraph_style;
    TextStyle text_style;
    text_style.setFontFamilies({SkString("Ahem") });
    text_style.setFontSize(10);
    text_style.setColor(SK_ColorBLACK);

    auto test = [&](const char* text) {
        auto str = straight(text);
        ParagraphBuilderImpl builder(paragraph_style, fontCollection);
        builder.pushStyle(text_style);
        builder.addText(str);
        builder.pop();
        SkPaint gray; gray.setColor(SK_ColorGRAY);
        auto paragraph = builder.Build();
        paragraph->layout(100);
        canvas.get()->translate(0, 20);
        canvas.get()->drawRect(SkRect::MakeXYWH(0, 0, paragraph->getMaxIntrinsicWidth(), paragraph->getHeight()), gray);
        paragraph->paint(canvas.get(), 0, 0);
        canvas.get()->translate(0, paragraph->getHeight());

        for (size_t i = 0; i < str.size(); ++i) {
            auto res = paragraph->getGlyphPositionAtCoordinate(i * 10, 2);
            //SkDebugf("@%f[%d]: %d %s\n", i * 10.0f, i, res.position, res.affinity == Affinity::kDownstream ? "D" : "U");
            // There is a hidden codepoint at the beginning (to make it symmetric to RTL)
            REPORTER_ASSERT(reporter, res.position == SkToInt(i) + (i > 0 ? 1 : 0));
            // The ending looks slightly different...
            REPORTER_ASSERT(reporter, res.affinity == (res.position == SkToInt(str.size()) ? Affinity::kUpstream : Affinity::kDownstream));
        }
    };

    test("    ");
    test("hello               ");
}

UNIX_ONLY_TEST(SkParagraph_RTLGlyphPositionsForTrailingSpaces, reporter) {

    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;

    TestCanvas canvas("SkParagraph_RTLGlyphPositionsForTrailingSpaces");

    ParagraphStyle paragraph_style;
    paragraph_style.setTextDirection(TextDirection::kRtl);
    paragraph_style.setTextAlign(TextAlign::kRight);
    TextStyle text_style;
    text_style.setFontFamilies({SkString("Ahem") });
    text_style.setFontSize(10);
    text_style.setColor(SK_ColorBLACK);
    canvas.get()->translate(200, 0);

    auto test = [&](const char* text, int whitespaces) {
        auto str = mirror(text);
        ParagraphBuilderImpl builder(paragraph_style, fontCollection);
        builder.pushStyle(text_style);
        builder.addText(str);
        builder.pop();
        SkPaint gray; gray.setColor(SK_ColorGRAY);
        auto paragraph = builder.Build();
        paragraph->layout(100);
        canvas.get()->translate(0, 20);
        auto res = paragraph->getRectsForRange(0, str.size(), RectHeightStyle::kTight, RectWidthStyle::kTight);
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
            auto pointX = (whitespaces + i) * 10.0f;
            auto pos = paragraph->getGlyphPositionAtCoordinate(pointX, 2);
            //SkDebugf("@%f[%d]: %d %s\n", pointX, i, pos.position, pos.affinity == Affinity::kDownstream ? "D" : "U");
            // At the beginning there is a control codepoint that makes the string RTL
            REPORTER_ASSERT(reporter, (pos.position + i) == SkToInt(str.size()) - (pos.affinity == Affinity::kDownstream ? 1 : 0));
        }
    };

    test("    ", 6);
    test("               hello", -10);
}

UNIX_ONLY_TEST(SkParagraph_LTRLineMetricsDoesNotIncludeNewLine, reporter) {

    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;

    TestCanvas canvas("SkParagraph_LTRLineMetricsDoesNotIncludeNewLine");

    ParagraphStyle paragraph_style;
    paragraph_style.setTextDirection(TextDirection::kRtl);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);
    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto") });
    text_style.setFontSize(20);
    text_style.setColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText("one two\n\nthree four\nwith spaces     \n    \n______________________");
    builder.pop();
    auto paragraph = builder.Build();
    paragraph->layout(190);
    paragraph->paint(canvas.get(), 0, 0);

    std::vector<std::tuple<size_t, size_t, size_t, size_t>> expected = {
            { 0, 7, 7, 8 },             // one two\n
            { 8, 8, 8, 9 },             // \n
            { 9, 19, 19, 20 },          // three four\n
            { 20, 31, 36, 37 },         // with spaces    \n
            { 37, 37, 41, 42 },         //      { just spaces }\n
            { 42, 63, 63, 63 },         // _____________________
            { 63, 64, 64, 64 },         // _
    };
    std::vector<LineMetrics> metrics;
    paragraph->getLineMetrics(metrics);
    for (auto& metric : metrics) {
        //SkDebugf("Line[%d:%d <= %d <=%d)\n", metric.fStartIndex, metric.fEndExcludingWhitespaces, metric.fEndIndex, metric.fEndIncludingNewline);
        auto result = expected[metric.fLineNumber];
        REPORTER_ASSERT(reporter, metric.fStartIndex ==std::get<0>(result));
        REPORTER_ASSERT(reporter, metric.fEndExcludingWhitespaces == std::get<1>(result));
        REPORTER_ASSERT(reporter, metric.fEndIndex == std::get<2>(result));
        REPORTER_ASSERT(reporter, metric.fEndIncludingNewline == std::get<3>(result));
    }
}

UNIX_ONLY_TEST(SkParagraph_RTLLineMetricsDoesNotIncludeNewLine, reporter) {

    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;

    TestCanvas canvas("SkParagraph_RTLLineMetricsDoesNotIncludeNewLine");
    canvas.get()->translate(100, 100);

    ParagraphStyle paragraph_style;
    paragraph_style.setTextDirection(TextDirection::kRtl);
    paragraph_style.setTextAlign(TextAlign::kRight);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);
    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto") });
    text_style.setFontSize(20);
    text_style.setColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(mirror("______________________\none two\n\nthree four\nwith spaces     \n    "));
    builder.pop();
    auto paragraph = builder.Build();
    paragraph->layout(190);
    paragraph->paint(canvas.get(), 0, 0);
    //auto impl = static_cast<ParagraphImpl*>(paragraph.get());

    SkPaint gray;
    gray.setColor(SK_ColorGRAY);
    gray.setStyle(SkPaint::kStroke_Style);
    gray.setAntiAlias(true);
    gray.setStrokeWidth(1);
    canvas.get()->drawRect(SkRect::MakeXYWH(0, 0, paragraph->getMaxWidth(), paragraph->getHeight()), gray);

    SkPaint red;
    red.setColor(SK_ColorRED);
    red.setStyle(SkPaint::kStroke_Style);
    red.setAntiAlias(true);
    red.setStrokeWidth(1);

    SkPaint blue;
    blue.setColor(SK_ColorRED);
    blue.setStyle(SkPaint::kStroke_Style);
    blue.setAntiAlias(true);
    blue.setStrokeWidth(1);

    auto boxes = paragraph->getRectsForRange(0, 100, RectHeightStyle::kTight, RectWidthStyle::kTight);
    bool even = false;
    for (auto& box : boxes) {
        canvas.get()->drawRect(box.rect, even ? red : blue);
        even = !even;
    }

    // RTL codepoint u"\u202E" messes everything up
    // (adds one invisible codepoint to the first line
    // and shift all the indexes by 1 right)
    std::vector<std::tuple<int, int, int, int>> expected = {
            { 0, 1, 5, 6 },                 //      { just spaces; the end of the text considered as a new line in libtxt?!? }
            { 6, 22, 22, 23  },             // with spaces    \n
            { 23, 33, 33, 34 },             // three four\n
            { 34, 34, 34, 35 },             // \n
            { 35, 42, 42, 43 },             // one two\n
            { 43, 64, 64, 64 },             // _____________________
            { 64, 65, 65, 65 }              // _
    };

    std::vector<LineMetrics> metrics;
    paragraph->getLineMetrics(metrics);
    for (auto& metric : metrics) {
        //SkDebugf("Line[%d:%d <= %d <=%d]\n", metric.fStartIndex, metric.fEndExcludingWhitespaces, metric.fEndIndex, metric.fEndIncludingNewline);
        auto result = expected[metric.fLineNumber];
        REPORTER_ASSERT(reporter, metric.fStartIndex == SkToU32(std::get<0>(result)));
        REPORTER_ASSERT(reporter, metric.fEndExcludingWhitespaces == SkToU32(std::get<1>(result)));
        REPORTER_ASSERT(reporter, metric.fEndIndex == SkToU32(std::get<2>(result)));
        REPORTER_ASSERT(reporter, metric.fEndIncludingNewline == SkToU32(std::get<3>(result)));
    }
}

UNIX_ONLY_TEST(SkParagraph_PlaceholderPosition, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;

    TestCanvas canvas("SkParagraph_PlaceholderPosition.png");
    canvas.get()->translate(100, 100);

    TextStyle text_style;
    text_style.setColor(SK_ColorBLACK);
    text_style.setFontFamilies({SkString("Ahem")});
    text_style.setFontSize(10.0f);
    ParagraphStyle paragraph_style;
    paragraph_style.setTextStyle(text_style);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);
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

UNIX_ONLY_TEST(SkParagraph_LineEnd, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;

    TestCanvas canvas("SkParagraph_LineEnd.png");
    canvas.get()->translate(100, 100);

    TextStyle text_style;
    text_style.setColor(SK_ColorBLACK);
    text_style.setFontFamilies({SkString("Ahem")});
    text_style.setFontSize(10.0f);
    ParagraphStyle paragraph_style;
    paragraph_style.setTextStyle(text_style);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);
    builder.pushStyle(text_style);
    builder.addText("Hello ");
    builder.addText("hello   ");
    builder.addText("hello\n");
    builder.addText("hello   \n");
    builder.addText("world");

    auto paragraph = builder.Build();
    paragraph->layout(60.0f);
    paragraph->paint(canvas.get(), 0, 0);

    std::vector<LineMetrics> lm;
    paragraph->getLineMetrics(lm);
    /*
    for (auto& lm : lm) {
        SkDebugf("%d %d %d\n", (int)lm.fEndExcludingWhitespaces, (int)lm.fEndIndex, (int)lm.fEndIncludingNewline);
    }
    */
    REPORTER_ASSERT(reporter, lm[0].fEndExcludingWhitespaces == 05 && lm[0].fEndIndex == 06 && lm[0].fEndIncludingNewline == 06);
    REPORTER_ASSERT(reporter, lm[1].fEndExcludingWhitespaces == 11 && lm[1].fEndIndex == 14 && lm[1].fEndIncludingNewline == 14);
    REPORTER_ASSERT(reporter, lm[2].fEndExcludingWhitespaces == 19 && lm[2].fEndIndex == 19 && lm[2].fEndIncludingNewline == 20);
    REPORTER_ASSERT(reporter, lm[3].fEndExcludingWhitespaces == 25 && lm[3].fEndIndex == 28 && lm[3].fEndIncludingNewline == 29);
}

UNIX_ONLY_TEST(SkParagraph_Utf16Indexes, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;

    TestCanvas canvas("SkParagraph_Utf16Indexes.png");
    canvas.get()->translate(100, 100);

    TextStyle text_style;
    text_style.setColor(SK_ColorBLACK);
    text_style.setFontFamilies({SkString("Ahem")});
    text_style.setFontSize(10.0f);
    ParagraphStyle paragraph_style;
    paragraph_style.setTextStyle(text_style);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);
    builder.pushStyle(text_style);
    builder.addText("áéíóú\nxxxx");
    auto paragraph = builder.Build();
    paragraph->layout(60.0f);
    paragraph->paint(canvas.get(), 0, 0);
    std::vector<LineMetrics> lm;
    paragraph->getLineMetrics(lm);
    //for (auto& lm : lm) {
    //    SkDebugf("%d %d %d\n", (int)lm.fEndExcludingWhitespaces, (int)lm.fEndIndex, (int)lm.fEndIncludingNewline);
    //}
    REPORTER_ASSERT(reporter, lm[0].fEndExcludingWhitespaces == 05 && lm[0].fEndIndex == 05 && lm[0].fEndIncludingNewline == 06);
    REPORTER_ASSERT(reporter, lm[1].fEndExcludingWhitespaces == 10 && lm[1].fEndIndex == 10 && lm[1].fEndIncludingNewline == 10);
}

UNIX_ONLY_TEST(SkParagraph_RTLFollowedByLTR, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;

    TestCanvas canvas("SkParagraph_RTLFollowedByLTR.png");
    canvas.get()->translate(100, 100);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Ahem")});
    text_style.setFontSize(10);

    ParagraphStyle paragraph_style;
    paragraph_style.setTextStyle(text_style);
    paragraph_style.setTextDirection(TextDirection::kLtr);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);
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

    std::vector<SkScalar> widths = {-10, 0, 10, 20, 30, 40, 50, 60};
    std::vector<int> positions = {-2, -2, 2, 1, -3, -4, -5, 6};

    size_t index = 0;
    for (auto w : widths) {
        auto pos = paragraph->getGlyphPositionAtCoordinate(w, 0);
        auto res = positions[index];
        REPORTER_ASSERT(reporter,
                        pos.affinity == (res < 0 ? Affinity::kDownstream : Affinity::kUpstream));
        REPORTER_ASSERT(reporter, pos.position == std::abs(res));
        ++index;
    }
}

UNIX_ONLY_TEST(SkParagraph_StrutTopLine, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;

    TestCanvas canvas("SkParagraph_StrutTopLine.png");

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Ahem")});
    text_style.setFontSize(10);
    SkPaint black;
    black.setColor(SK_ColorBLACK);
    text_style.setForegroundColor(black);

    ParagraphStyle paragraph_style;
    paragraph_style.setTextStyle(text_style);
    paragraph_style.setTextDirection(TextDirection::kLtr);
    StrutStyle strut_style;
    strut_style.setStrutEnabled(true);
    strut_style.setFontFamilies({SkString("Ahem")});
    strut_style.setFontSize(16);
    strut_style.setHeight(4.0f);
    strut_style.setHeightOverride(true);
    strut_style.setLeading(-1.0f);
    strut_style.setForceStrutHeight(true);
    paragraph_style.setStrutStyle(strut_style);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    builder.pushStyle(text_style);
    builder.addText(u"Atwater Peel Sherbrooke Bonaventure\nhi\nwasssup!");

    auto paragraph = builder.Build();
    paragraph->layout(797);
    paragraph->paint(canvas.get(), 0, 0);
    auto boxes = paragraph->getRectsForRange(0, 60, RectHeightStyle::kIncludeLineSpacingTop, RectWidthStyle::kMax);
    REPORTER_ASSERT(reporter, boxes.size() == 4);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.fTop, 38.4f));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.fBottom, 64.0f));

    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[1].rect.fTop, 64.0f));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[1].rect.fBottom, 128.0f));

    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[2].rect.fTop, 64.0f));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[2].rect.fBottom, 128.0f));

    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[3].rect.fTop, 128.0f));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[3].rect.fBottom, 192.0f));
}

UNIX_ONLY_TEST(SkParagraph_DifferentFontsTopLine, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    if (!fontCollection->fontsFound()) return;

    TestCanvas canvas("SkParagraph_DifferentFontsTopLine.png");

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Ahem")});
    text_style.setFontSize(10);
    SkPaint black;
    black.setColor(SK_ColorBLACK);
    text_style.setForegroundColor(black);

    ParagraphStyle paragraph_style;
    paragraph_style.setTextStyle(text_style);
    paragraph_style.setTextDirection(TextDirection::kLtr);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    text_style.setFontSize(30.0);
    builder.pushStyle(text_style);
    builder.addText(u"Atwater Peel ");
    text_style.setFontSize(15.0);
    builder.pushStyle(text_style);
    builder.addText(u"Sherbrooke Bonaventure ");
    text_style.setFontSize(10.0);
    builder.pushStyle(text_style);
    builder.addText(u"hi wassup!");

    auto paragraph = builder.Build();
    paragraph->layout(797);
    paragraph->paint(canvas.get(), 0, 0);
    auto boxes = paragraph->getRectsForRange(0, 60, RectHeightStyle::kIncludeLineSpacingTop, RectWidthStyle::kMax);
    REPORTER_ASSERT(reporter, boxes.size() == 4);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.fTop, 00.0f));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.fBottom, 30.0f));

    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[1].rect.fTop, 00.0f));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[1].rect.fBottom, 30.0f));

    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[2].rect.fTop, 00.0f));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[2].rect.fBottom, 30.0f));

    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[3].rect.fTop, 30.0f));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[3].rect.fBottom, 40.0f));
}
