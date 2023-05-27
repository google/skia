/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "include/core/SkTypeface.h"
#include "include/ports/SkTypeface_fontations.h"
#include "tools/Resources.h"

namespace skiagm {

namespace {
const SkScalar kTextSizes[] = {12, 18, 30, 120};
const char kTestFontName[] = "fonts/Roboto-Regular.ttf";
const SkScalar kDumpFontSize = 20.0f;

// TODO(drott): Test these dumps is in a unit test instead of dumping them to GM surface.
void dumpToCanvas(SkCanvas* canvas, sk_sp<SkTypeface> typeface, SkString text) {
    canvas->drawSimpleText(text.c_str(),
                           text.size() - 1,
                           SkTextEncoding::kUTF8, 0, 0,
                           SkFont(typeface, kDumpFontSize),
                           SkPaint());
}

void dumpLocalizedStrings(SkCanvas* canvas, sk_sp<SkTypeface> typeface) {
    auto family_names = typeface->createFamilyNameIterator();
    SkTypeface::LocalizedString famName;
    SkString localizedName;
    while (family_names->next(&famName)) {
        localizedName.printf(
                "Name: %s Language: %s\n", famName.fString.c_str(), famName.fLanguage.c_str());
        dumpToCanvas(canvas, typeface, localizedName);
        canvas->translate(0, kDumpFontSize * 1.2);
    }
    family_names->unref();
}

void dumpGlyphCount(SkCanvas* canvas, sk_sp<SkTypeface> typeface) {
    SkString glyphCount;
    glyphCount.printf("Num glyphs: %d\n", typeface->countGlyphs());
    dumpToCanvas(canvas, typeface, glyphCount);
}

void dumpFamilyAndPostscriptName(SkCanvas* canvas, sk_sp<SkTypeface> typeface) {
    SkString name;
    typeface->getFamilyName(&name);
    SkString nameDump;
    nameDump.printf("Family name: %s\n", name.c_str());
    dumpToCanvas(canvas, typeface, nameDump);

    if (typeface->getPostScriptName(&name)) {
        canvas->translate(0, kDumpFontSize * 1.2);
        nameDump.printf("PS Name: %s\n", name.c_str());
        dumpToCanvas(canvas, typeface, nameDump);
    } else {
        canvas->translate(0, kDumpFontSize * 1.2);
        nameDump.printf("No Postscript name.");
        dumpToCanvas(canvas, typeface, nameDump);
    }
}

}  // namespace

class FontationsTypefaceGM : public GM {
public:
    FontationsTypefaceGM() { this->setBGColor(SK_ColorWHITE); }

protected:
    void onOnceBeforeDraw() override {
        fTypeface =
                SkTypeface_Make_Fontations(GetResourceAsStream(kTestFontName), SkFontArguments());
    }

    SkString onShortName() override { return SkString("typeface_fontations"); }

    SkISize onISize() override { return SkISize::Make(400, 200); }

    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        SkPaint paint;
        paint.setColor(SK_ColorBLACK);

        if (!fTypeface) {
            *errorMsg = "Unable to initialize typeface.";
            return DrawResult::kSkip;
        }

        SkFont font(fTypeface);
        const char32_t testText[] = U"xyz";
        size_t testTextBytesize = sizeof(testText) / sizeof(char32_t) * sizeof(char32_t);
        SkScalar x = 100;
        SkScalar y = 150;

        for (SkScalar textSize : kTextSizes) {
            font.setSize(textSize);
            y += font.getSpacing();

            /* Draw origin marker as a green dot. */
            paint.setColor(SK_ColorGREEN);
            canvas->drawRect(SkRect::MakeXYWH(x, y, 2, 2), paint);
            paint.setColor(SK_ColorBLACK);

            canvas->drawSimpleText(
                    testText, testTextBytesize, SkTextEncoding::kUTF32, x, y, font, paint);
        }

        canvas->translate(100, 470);
        dumpGlyphCount(canvas, fTypeface);
        canvas->translate(0, kDumpFontSize * 1.2);
        dumpLocalizedStrings(canvas, fTypeface);
        canvas->translate(0, kDumpFontSize * 1.2);
        dumpFamilyAndPostscriptName(canvas, fTypeface);

        return DrawResult::kOk;
    }

private:
    using INHERITED = GM;

    sk_sp<SkTypeface> fTypeface;
};

DEF_GM(return new FontationsTypefaceGM();)

}  // namespace skiagm
