/*
 * Copyright 2019 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "tools/Resources.h"
#include "tests/Test.h"
#include "src/core/SkSpan.h"
#include "src/utils/SkJSONWriter.h"
#include "src/utils/SkUTF.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "src/core/SkOSFile.h"
#include "src/core/SkSpan.h"
#include "src/utils/SkOSPath.h"
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
#include "src/utils/SkTextLayoutJSON.h"
using namespace skia::textlayout;
#define TestCanvasWidth 1000
#define TestCanvasHeight 600
namespace {
SkString getPath(const char* name) {
    SkString tmpDir = skiatest::GetTmpDir();
    SkString path = SkOSPath::Join(tmpDir.c_str(), name);
    return path;
}
class TestCanvas {
public:
    TestCanvas(const char* testName) : name(testName) {
        bits.allocN32Pixels(TestCanvasWidth, TestCanvasHeight);
        canvas = new SkCanvas(bits);
        canvas->clear(SK_ColorWHITE);
    }
    ~TestCanvas() {
        SkFILEWStream file(getPath(name).c_str());
        if (!SkEncodeImage(&file, bits, SkEncodedImageFormat::kPNG, 100)) {
            SkDebugf("Cannot write a picture %s\n", name);
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
}
DEF_TEST(SkTextLayoutJSONTest_1, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>(GetResourcePath("fonts").c_str(), false, true);
    if (!fontCollection->fontsFound()) return;
    ParagraphStyle paragraph_style;
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);
    TextStyle text_style;
    text_style.setColor(SK_ColorBLACK);
    SkPaint paint;
    paint.setColor(SK_ColorWHITE);
    text_style.setBackgroundColor(paint);
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setFontSize(20);
    builder.pushStyle(text_style);
    builder.addText("World domination is such an ugly phrase - I prefer to call it world optimisation");
    auto paragraph = builder.Build();
    paragraph->layout(500);
    TestCanvas canvas("1.png");
    paragraph->paint(canvas.get(), 0, 0);
    {
        SkFILEWStream stream(getPath("1.input.json").c_str());
        SkJSONWriter writer(&stream, SkJSONWriter::Mode::kPretty);
        SkTextLayoutJSON::writeInput(&writer, (ParagraphImpl*)paragraph.get());
        writer.flush();
    }
    {
        SkFILEWStream stream(getPath("1.output.json").c_str());
        SkJSONWriter writer(&stream, SkJSONWriter::Mode::kPretty);
        SkTextLayoutJSON::writeOutput(&writer, (ParagraphImpl*)paragraph.get());
        writer.flush();
    }
}
