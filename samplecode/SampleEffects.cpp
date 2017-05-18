/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SampleCode.h"
#include "SkBlurMask.h"
#include "SkBlurMaskFilter.h"
#include "SkCanvas.h"
#include "SkColorMatrixFilter.h"
#include "SkDiscretePathEffect.h"
#include "SkGradientShader.h"
#include "SkPaint.h"
#include "SkView.h"


//#define COLOR 0xFFFF8844
#define COLOR 0xFF888888

static void paint_proc0(SkPaint*) {
}

static void paint_proc1(SkPaint* paint) {
    paint->setMaskFilter(SkBlurMaskFilter::Make(
                                kNormal_SkBlurStyle,
                                SkBlurMask::ConvertRadiusToSigma(2)));
}

static void paint_proc2(SkPaint* paint) {
#ifdef SK_SUPPORT_LEGACY_EMBOSSMASKFILTER
    SkScalar dir[3] = { 1, 1, 1};
    paint->setMaskFilter(
            SkBlurMaskFilter::MakeEmboss(SkBlurMask::ConvertRadiusToSigma(1), dir, 0.1f, 0.05f));
#endif
}

static void paint_proc3(SkPaint* paint) {
    SkColor colors[] = { SK_ColorRED, COLOR, SK_ColorBLUE };
    SkPoint pts[] = { { 3, 0 }, { 7, 5 } };
    paint->setShader(SkGradientShader::MakeLinear(pts, colors, nullptr, SK_ARRAY_COUNT(colors),
                                                  SkShader::kMirror_TileMode));
}

static void paint_proc5(SkPaint* paint) {
    paint_proc3(paint);
    paint_proc2(paint);
}

typedef void (*PaintProc)(SkPaint*);
const PaintProc gPaintProcs[] = {
    paint_proc0,
    paint_proc1,
    paint_proc2,
    paint_proc3,
    paint_proc5,
};

///////////////////////////////////////////////////////////////////////////////

class EffectsView : public SampleView {
public:
    SkPath fPath;
    SkPaint fPaint[SK_ARRAY_COUNT(gPaintProcs)];

    EffectsView() {
        size_t i;
        const float pts[] = {
            0, 0,
            10, 0,
            10, 5,
            20, -5,
            10, -15,
            10, -10,
            0, -10
        };
        fPath.moveTo(pts[0], pts[1]);
        for (i = 2; i < SK_ARRAY_COUNT(pts); i += 2) {
            fPath.lineTo(pts[i], pts[i+1]);
        }

        for (i = 0; i < SK_ARRAY_COUNT(gPaintProcs); i++) {
            fPaint[i].setAntiAlias(true);
            fPaint[i].setColor(COLOR);
            gPaintProcs[i](&fPaint[i]);
        }

        SkColorMatrix cm;
        cm.setRotate(SkColorMatrix::kG_Axis, 180);
        cm.setIdentity();

        this->setBGColor(0xFFDDDDDD);
    }

protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "Effects");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    virtual void onDrawContent(SkCanvas* canvas) {
        canvas->scale(3, 3);
        canvas->translate(10, 30);
        for (size_t i = 0; i < SK_ARRAY_COUNT(fPaint); i++) {
            canvas->drawPath(fPath, fPaint[i]);
            canvas->translate(32, 0);
        }
    }

private:
    typedef SampleView INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new EffectsView; }
static SkViewRegister reg(MyFactory);
