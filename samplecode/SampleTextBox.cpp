/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "samplecode/Sample.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkColor.h"
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


extern sk_sp<SkTypeface> makeMacSystemFont(float size);
  
class SampleShaper : public Sample {
public:
    SampleShaper() {}

protected:
    SkString name() override { return SkString("shaper"); }

    sk_sp<SkTextBlob> drawTest(SkCanvas* canvas, const char str[], SkScalar size,
                  std::unique_ptr<SkShaper> shaper, SkColor color) {
        if (!shaper) return nullptr;

        SkTextBlobBuilderRunHandler builder(str, {0, 0});
        sk_sp<SkTypeface> mac_system_font(makeMacSystemFont(size));
        SkFont srcFont(mac_system_font);
        srcFont.setSize(size);
        srcFont.setEdging(SkFont::Edging::kSubpixelAntiAlias);
        srcFont.setSubpixel(true);

        size_t len = strlen(str);

        std::unique_ptr<SkShaper::BiDiRunIterator> bidi(
            SkShaper::MakeBiDiRunIterator(str, len, 0xfe));
        if (!bidi) {
            return nullptr;
        }

        std::unique_ptr<SkShaper::LanguageRunIterator> language(
            SkShaper::MakeStdLanguageRunIterator(str, len));
        if (!language) {
            return nullptr;
        }

        SkFourByteTag undeterminedScript = SkSetFourByteTag('Z','y','y','y');
        std::unique_ptr<SkShaper::ScriptRunIterator> script(
            SkShaper::MakeScriptRunIterator(str, len, undeterminedScript));
        if (!script) {
            return nullptr;
        }

        std::unique_ptr<SkShaper::FontRunIterator> font(
            SkShaper::MakeFontMgrRunIterator(str, len, srcFont, SkFontMgr::RefDefault(),
                                             "Arial", SkFontStyle::Bold(), &*language));
        if (!font) {
            return nullptr;
        }

        shaper->shape(str, len, *font, *bidi, *script, *language, 2000, &builder);

        SkPaint paint;
        paint.setColor(color);
        sk_sp<SkTextBlob> paint_blob = builder.makeBlob();
        canvas->drawTextBlob(paint_blob, 0, 0, paint);
        return paint_blob;
    }

    void onDrawContent(SkCanvas* canvas) override {
        canvas->translate(10, 30);

        const char text[] = "YouWebTorrentVa";

        float trakTableFontSizes[] = {6.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0,
                                      16.0, 17.0, 20.0, 22.0, 28.0, 32.0, 36.0, 50.0, 64.0, 80.0, 100.0, 138.0 };

        for (SkScalar size : trakTableFontSizes) {
          sk_sp<SkTextBlob> coretext_blob = this->drawTest(canvas, text, size, SkShaper::MakeCoreText(), SkColorSetARGB(180, 0, 255, 0));
          sk_sp<SkTextBlob> harfbuzz_blob = this->drawTest(canvas, text, size, SkShaper::Make(), SkColorSetARGB(180, 255, 0, 0));
            canvas->translate(0, size * 1.2);


            float diff = coretext_blob->bounds().width() - harfbuzz_blob->bounds().width();
            float diff_font_units_per_character = diff / size * 2048 / 15.0f;
            printf("Blob width diff for font size: %03.2f: %.3f, diff per character in font units: %.3f\n", size, diff, diff_font_units_per_character);
        }
    }

private:
    typedef Sample INHERITED;
};
DEF_SAMPLE( return new SampleShaper; );
