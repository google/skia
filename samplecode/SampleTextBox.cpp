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
#include "include/effects/SkBlurMaskFilter.h"
#include "include/effects/SkGradientShader.h"
#include "include/utils/SkRandom.h"
#include "modules/skshaper/include/SkShaper.h"
#include "src/core/SkOSFile.h"
#include "src/shaders/SkColorShader.h"
#include "src/utils/SkUTF.h"

static const char gText[] = "第一条 人人生而自由,在尊严和权利上一律平等。他们赋有理性和良心,并应以兄弟关系的精神相对待。第二条 人人有资格享有本宣言所载的一切权利和自由,不分种族、肤色、性别、语言、宗教、政治或其他见解、国籍或社会出身、财产、出生或其他身分等任何区别。并且不得因一人所属的国家或领土的政治的、行政的或者国际的地位之不同而有所区别,无论该领土是独立领土、托管领土、非自治领土或者处于其他任何主权受限制的情况之下。";
class TextBoxView : public Sample {
public:
    TextBoxView() : fShaper(SkShaper::Make()) {}

protected:
    SkString name() override { return SkString("TextBox"); }

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

            const char* utf8 = gText;
            size_t utf8Bytes = sizeof(gText) - 1;

            std::unique_ptr<SkShaper::BiDiRunIterator> bidi(SkShaper::MakeIcuBiDiRunIterator(utf8, utf8Bytes, 0xfe));
            if (!bidi) {
                return;
            }

            std::unique_ptr<SkShaper::LanguageRunIterator> language(SkShaper::MakeStdLanguageRunIterator(utf8, utf8Bytes));
            if (!language) {
                return;
            }

            std::unique_ptr<SkShaper::ScriptRunIterator> script(SkShaper::MakeHbIcuScriptRunIterator(utf8, utf8Bytes));
            if (!script) {
                return;
            }

            std::unique_ptr<SkShaper::FontRunIterator> font(
                        SkShaper::MakeFontMgrRunIterator(utf8, utf8Bytes, srcFont, SkFontMgr::RefDefault(),
                                                         "Arial", SkFontStyle::Bold(), &*language
                        ));
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

//////////////////////////////////////////////////////////////////////////////

DEF_SAMPLE( return new TextBoxView(); )
