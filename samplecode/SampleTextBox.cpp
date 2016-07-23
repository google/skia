/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SampleCode.h"
#include "SkView.h"
#include "SkBlurMaskFilter.h"
#include "SkCanvas.h"
#include "SkColorShader.h"
#include "SkGradientShader.h"
#include "SkGraphics.h"
#include "SkPath.h"
#include "SkRandom.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkUtils.h"
#include "SkXfermode.h"
#include "SkColorPriv.h"
#include "SkColorFilter.h"
#include "SkTime.h"
#include "SkTypeface.h"
#include "SkTextBox.h"
#include "SkOSFile.h"
#include "SkStream.h"
#include "SkKey.h"

extern void skia_set_text_gamma(float blackGamma, float whiteGamma);

#if defined(SK_BUILD_FOR_WIN) && defined(SK_FONTHOST_WIN_GDI)
extern SkTypeface* SkCreateTypefaceFromLOGFONT(const LOGFONT&);
#endif

static const char gText[] =
    "When in the Course of human events it becomes necessary for one people "
    "to dissolve the political bands which have connected them with another "
    "and to assume among the powers of the earth, the separate and equal "
    "station to which the Laws of Nature and of Nature's God entitle them, "
    "a decent respect to the opinions of mankind requires that they should "
    "declare the causes which impel them to the separation.";

class TextBoxView : public SampleView {
public:
    TextBoxView() {
#if defined(SK_BUILD_FOR_WIN) && defined(SK_FONTHOST_WIN_GDI)
        LOGFONT lf;
        sk_bzero(&lf, sizeof(lf));
        lf.lfHeight = 9;
        SkTypeface* tf0 = SkCreateTypefaceFromLOGFONT(lf);
        lf.lfHeight = 12;
        SkTypeface* tf1 = SkCreateTypefaceFromLOGFONT(lf);
        // we assert that different sizes should not affect which face we get
        SkASSERT(tf0 == tf1);
        tf0->unref();
        tf1->unref();
#endif
    }

protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt)  {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "TextBox");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void drawTest(SkCanvas* canvas, SkScalar w, SkScalar h, SkColor fg, SkColor bg) {
        SkAutoCanvasRestore acr(canvas, true);

        canvas->clipRect(SkRect::MakeWH(w, h));
        canvas->drawColor(bg);
        SkScalar margin = 20;
        SkTextBox tbox;
        tbox.setMode(SkTextBox::kLineBreak_Mode);
        tbox.setBox(margin, margin,
                    w - margin, h - margin);
        tbox.setSpacing(SkIntToScalar(3)/3, 0);

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setLCDRenderText(true);
        paint.setColor(fg);
        tbox.setText(gText, strlen(gText), paint);

        for (int i = 9; i < 24; i += 2) {
            paint.setTextSize(SkIntToScalar(i));
            tbox.draw(canvas);
            canvas->translate(0, tbox.getTextHeight() + paint.getFontSpacing());
        }
    }

    virtual void onDrawContent(SkCanvas* canvas) {
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
    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new TextBoxView; }
static SkViewRegister reg(MyFactory);
