/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Sample.h"

#include "SkBlurMaskFilter.h"
#include "SkCanvas.h"
#include "SkColorFilter.h"
#include "SkColorPriv.h"
#include "SkColorShader.h"
#include "SkGradientShader.h"
#include "SkGraphics.h"
#include "SkOSFile.h"
#include "SkPath.h"
#include "SkRandom.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkShaper.h"
#include "SkStream.h"
#include "SkTextBlob.h"
#include "SkTime.h"
#include "SkTypeface.h"
#include "SkUTF.h"

static const char gText[] =
    "When in the Course of human events it becomes necessary for one people "
    "to dissolve the political bands which have connected them with another "
    "and to assume among the powers of the earth, the separate and equal "
    "station to which the Laws of Nature and of Nature's God entitle them, "
    "a decent respect to the opinions of mankind requires that they should "
    "declare the causes which impel them to the separation.";

class TextBoxView : public Sample {
public:
    TextBoxView() : fShaper(SkShaper::Make()) {}

protected:
    bool onQuery(Sample::Event* evt) override {
        if (Sample::TitleQ(*evt)) {
            Sample::TitleR(evt, "TextBox");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void drawTest(SkCanvas* canvas, SkScalar w, SkScalar h, SkColor fg, SkColor bg) {
        SkAutoCanvasRestore acr(canvas, true);

        canvas->clipRect(SkRect::MakeWH(w, h));
        canvas->drawColor(bg);

        SkScalar margin = 20;

        SkPaint paint;
        paint.setColor(fg);

        for (int i = 9; i < 24; i += 2) {
            SkTextBlobBuilderRunHandler builder(gText);
            SkFont font(nullptr, SkIntToScalar(i));
            font.setEdging(SkFont::Edging::kSubpixelAntiAlias);

            SkPoint end = fShaper->shape(&builder, font, gText, strlen(gText), true,
                                         { margin, margin }, w - margin);
            canvas->drawTextBlob(builder.makeBlob(), 0, 0, paint);

            canvas->translate(0, end.y());
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
