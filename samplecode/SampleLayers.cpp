
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkBlurMaskFilter.h"
#include "SkCamera.h"
#include "SkColorFilter.h"
#include "SkColorPriv.h"
#include "SkDevice.h"
#include "SkGradientShader.h"
#include "SkImageDecoder.h"
#include "SkInterpolator.h"
#include "SkMaskFilter.h"
#include "SkPath.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkTime.h"
#include "SkTypeface.h"
#include "SkUtils.h"
#include "SkKey.h"
#include "SkXfermode.h"
#include "SkDrawFilter.h"

static void make_paint(SkPaint* paint, const SkMatrix& localMatrix) {
    SkColor colors[] = { 0, SK_ColorWHITE };
    SkPoint pts[] = { { 0, 0 }, { 0, SK_Scalar1*20 } };
    SkShader* s = SkGradientShader::CreateLinear(pts, colors, NULL, 2, SkShader::kClamp_TileMode,
                                                 0, &localMatrix);

    paint->setShader(s)->unref();
    paint->setXfermodeMode(SkXfermode::kDstIn_Mode);
}

static void dump_layers(const char label[], SkCanvas* canvas) {
    SkDebugf("Dump Layers(%s)\n", label);

    SkCanvas::LayerIter iter(canvas, true);
    int index = 0;
    while (!iter.done()) {
        SkImageInfo info = iter.device()->imageInfo();
        const SkIRect& clip = iter.clip().getBounds();
        SkDebugf("Layer[%d] bitmap [%d %d] X=%d Y=%d clip=[%d %d %d %d] alpha=%d\n", index++,
                 info.width(), info.height(), iter.x(), iter.y(),
                 clip.fLeft, clip.fTop, clip.fRight, clip.fBottom,
                 iter.paint().getAlpha());
        iter.next();
    }
}

// test drawing with strips of fading gradient above and below
static void test_fade(SkCanvas* canvas) {
    SkAutoCanvasRestore ar(canvas, true);

    SkRect r;

    SkPaint p;
    p.setAlpha(0x88);

    SkAutoCanvasRestore ar2(canvas, false);

    // create the layers

    r.set(0, 0, SkIntToScalar(100), SkIntToScalar(100));
    canvas->clipRect(r);

    r.fBottom = SkIntToScalar(20);
    canvas->saveLayer(&r, NULL);

    r.fTop = SkIntToScalar(80);
    r.fBottom = SkIntToScalar(100);
    canvas->saveLayer(&r, NULL);

    // now draw the "content"

    if (true) {
        r.set(0, 0, SkIntToScalar(100), SkIntToScalar(100));

        canvas->saveLayerAlpha(&r, 0x80);

        SkPaint p;
        p.setColor(SK_ColorRED);
        p.setAntiAlias(true);
        canvas->drawOval(r, p);

        dump_layers("inside layer alpha", canvas);

        canvas->restore();
    } else {
        r.set(0, 0, SkIntToScalar(100), SkIntToScalar(100));

        SkPaint p;
        p.setColor(SK_ColorRED);
        p.setAntiAlias(true);
        canvas->drawOval(r, p);
    }

//    return;

    dump_layers("outside layer alpha", canvas);

    // now apply an effect
    SkMatrix m;
    m.setScale(SK_Scalar1, -SK_Scalar1);
    m.postTranslate(0, SkIntToScalar(100));

    SkPaint paint;
    make_paint(&paint, m);
    r.set(0, 0, SkIntToScalar(100), SkIntToScalar(20));
//    SkDebugf("--------- draw top grad\n");
    canvas->drawRect(r, paint);

    r.fTop = SkIntToScalar(80);
    r.fBottom = SkIntToScalar(100);
//    SkDebugf("--------- draw bot grad\n");
    canvas->drawRect(r, paint);
}

class RedFilter : public SkDrawFilter {
public:
    bool filter(SkPaint* p, SkDrawFilter::Type) override {
        fColor = p->getColor();
        if (fColor == SK_ColorRED) {
            p->setColor(SK_ColorGREEN);
        }
        return true;
    }

private:
    SkColor fColor;
};

class LayersView : public SkView {
public:
    LayersView() {}

protected:
    // overrides from SkEventSink
    bool onQuery(SkEvent* evt) override {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "Layers");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void drawBG(SkCanvas* canvas) {
        canvas->drawColor(SK_ColorGRAY);
    }

    void onDraw(SkCanvas* canvas) override {
        this->drawBG(canvas);

        if (true) {
            SkRect r;
            r.set(SkIntToScalar(0), SkIntToScalar(0),
                  SkIntToScalar(220), SkIntToScalar(120));
            SkPaint p;
            canvas->saveLayer(&r, &p);
            canvas->drawColor(0xFFFF0000);
            p.setAlpha(0);  // or 0
            p.setXfermodeMode(SkXfermode::kSrc_Mode);
            canvas->drawOval(r, p);
            canvas->restore();
            return;
        }

        if (false) {
            SkRect r;
            r.set(SkIntToScalar(0), SkIntToScalar(0),
                  SkIntToScalar(220), SkIntToScalar(120));
            SkPaint p;
            p.setAlpha(0x88);
            p.setAntiAlias(true);

            if (true) {
                canvas->saveLayer(&r, &p);
                p.setColor(0xFFFF0000);
                canvas->drawOval(r, p);
                canvas->restore();
            }

            p.setColor(0xFF0000FF);
            r.offset(SkIntToScalar(20), SkIntToScalar(50));
            canvas->drawOval(r, p);
        }

        if (false) {
            SkPaint p;
            p.setAlpha(0x88);
            p.setAntiAlias(true);

            canvas->translate(SkIntToScalar(300), 0);

            SkRect r;
            r.set(SkIntToScalar(0), SkIntToScalar(0),
                  SkIntToScalar(220), SkIntToScalar(60));

            canvas->saveLayer(&r, &p);

            r.set(SkIntToScalar(0), SkIntToScalar(0),
                  SkIntToScalar(220), SkIntToScalar(120));
            p.setColor(SK_ColorBLUE);
            canvas->drawOval(r, p);
            canvas->restore();
            return;
        }

        test_fade(canvas);
    }

    SkView::Click* onFindClickHandler(SkScalar x, SkScalar y, unsigned modi) override {
        this->inval(NULL);

        return this->INHERITED::onFindClickHandler(x, y, modi);
    }

    bool onClick(Click* click) override {
        return this->INHERITED::onClick(click);
    }

    virtual bool handleKey(SkKey) {
        this->inval(NULL);
        return true;
    }

private:
    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new LayersView; }
static SkViewRegister reg(MyFactory);
