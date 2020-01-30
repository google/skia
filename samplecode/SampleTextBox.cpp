/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "samplecode/Sample.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkPath.h"
#include "include/core/SkRegion.h"
#include "include/core/SkShader.h"
#include "include/core/SkStream.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTime.h"
#include "include/core/SkTypeface.h"
#include "include/effects/SkGradientShader.h"
#include "include/utils/SkRandom.h"
#include "modules/skshaper/include/SkShaper.h"
#include "src/core/SkOSFile.h"
#include "src/shaders/SkColorShader.h"
#include "src/utils/SkUTF.h"

typedef std::unique_ptr<SkShaper> (*ShaperFactory)();

static const char gText[] =
    "When in the Course of human events it becomes necessary for one people "
    "to dissolve the political bands which have connected them with another "
    "and to assume among the powers of the earth, the separate and equal "
    "station to which the Laws of Nature and of Nature's God entitle them, "
    "a decent respect to the opinions of mankind requires that they should "
    "declare the causes which impel them to the separation.";

class TextBoxView : public Sample {
    SkString fName;
public:
    TextBoxView(ShaperFactory fact, const char suffix[]) : fShaper(fact()) {
        fName.printf("TextBox_%s", suffix);
    }

protected:
    SkString name() override { return fName; }

    void drawTest(SkCanvas* canvas, SkScalar w, SkScalar h, SkColor fg, SkColor bg) {
        SkAutoCanvasRestore acr(canvas, true);

        canvas->clipRect(SkRect::MakeWH(w, h));
        canvas->drawColor(bg);

        SkScalar margin = 20;

        SkPaint paint;
        paint.setColor(fg);

        for (int i = 9; i < 24; i += 2) {
            SkTextBlobBuilderRunHandler builder(gText, { margin, margin });
            SkFont srcFont(nullptr, SkIntToScalar(i));
            srcFont.setEdging(SkFont::Edging::kSubpixelAntiAlias);
            srcFont.setSubpixel(true);

            const char* utf8 = gText;
            size_t utf8Bytes = sizeof(gText) - 1;

            std::unique_ptr<SkShaper::BiDiRunIterator> bidi(
                SkShaper::MakeBiDiRunIterator(utf8, utf8Bytes, 0xfe));
            if (!bidi) {
                return;
            }

            std::unique_ptr<SkShaper::LanguageRunIterator> language(
                SkShaper::MakeStdLanguageRunIterator(utf8, utf8Bytes));
            if (!language) {
                return;
            }

            SkFourByteTag undeterminedScript = SkSetFourByteTag('Z','y','y','y');
            std::unique_ptr<SkShaper::ScriptRunIterator> script(
                SkShaper::MakeScriptRunIterator(utf8, utf8Bytes, undeterminedScript));
            if (!script) {
                return;
            }

            std::unique_ptr<SkShaper::FontRunIterator> font(
                SkShaper::MakeFontMgrRunIterator(utf8, utf8Bytes, srcFont, SkFontMgr::RefDefault(),
                                                 "Arial", SkFontStyle::Bold(), &*language));
            if (!font) {
                return;
            }

            fShaper->shape(utf8, utf8Bytes, *font, *bidi, *script, *language, w - margin, &builder);
            canvas->drawTextBlob(builder.makeBlob(), 0, 0, paint);

            canvas->translate(0, builder.endPoint().y());
        }
    }

    void onDrawContent(SkCanvas* canvas) override {
        SkScalar width = this->width() / 3;
        drawTest(canvas, width, this->height(), SK_ColorBLACK, SK_ColorWHITE);
        canvas->translate(width, 0);
        drawTest(canvas, width, this->height(), SK_ColorWHITE, SK_ColorBLACK);
        canvas->translate(width, 0);
        drawTest(canvas, width, this->height()/2, SK_ColorGRAY, SK_ColorWHITE);
        canvas->translate(0, this->height()/2);
        drawTest(canvas, width, this->height()/2, SK_ColorGRAY, SK_ColorBLACK);
    }

private:
    std::unique_ptr<SkShaper> fShaper;
    typedef Sample INHERITED;
};

DEF_SAMPLE( return new TextBoxView([](){ return SkShaper::Make(); }, "default"); );
#ifdef SK_BUILD_FOR_MAC
DEF_SAMPLE( return new TextBoxView(SkShaper::MakeCoreText, "coretext"); );
#endif

class SampleShaper : public Sample {
public:
    SampleShaper() {}

protected:
    SkString name() override { return SkString("shaper"); }

    void drawTest(SkCanvas* canvas, const char str[], SkScalar size,
                  std::unique_ptr<SkShaper> shaper) {
        if (!shaper) return;

        SkTextBlobBuilderRunHandler builder(str, {0, 0});
        SkFont srcFont;
        srcFont.setSize(size);
        srcFont.setEdging(SkFont::Edging::kSubpixelAntiAlias);
        srcFont.setSubpixel(true);

        size_t len = strlen(str);

        std::unique_ptr<SkShaper::BiDiRunIterator> bidi(
            SkShaper::MakeBiDiRunIterator(str, len, 0xfe));
        if (!bidi) {
            return;
        }

        std::unique_ptr<SkShaper::LanguageRunIterator> language(
            SkShaper::MakeStdLanguageRunIterator(str, len));
        if (!language) {
            return;
        }

        SkFourByteTag undeterminedScript = SkSetFourByteTag('Z','y','y','y');
        std::unique_ptr<SkShaper::ScriptRunIterator> script(
            SkShaper::MakeScriptRunIterator(str, len, undeterminedScript));
        if (!script) {
            return;
        }

        std::unique_ptr<SkShaper::FontRunIterator> font(
            SkShaper::MakeFontMgrRunIterator(str, len, srcFont, SkFontMgr::RefDefault(),
                                             "Arial", SkFontStyle::Bold(), &*language));
        if (!font) {
            return;
        }

        shaper->shape(str, len, *font, *bidi, *script, *language, 2000, &builder);

        canvas->drawTextBlob(builder.makeBlob(), 0, 0, SkPaint());
    }

    void onDrawContent(SkCanvas* canvas) override {
        canvas->translate(10, 30);

        const char text[] = "world";

        for (SkScalar size = 30; size <= 30; size += 10) {
            this->drawTest(canvas, text, size, SkShaper::Make());
            canvas->translate(0, size + 5);
            this->drawTest(canvas, text, size, SkShaper::MakeCoreText());
            canvas->translate(0, size*2);
        }
    }

private:
    typedef Sample INHERITED;
};
DEF_SAMPLE( return new SampleShaper; );
