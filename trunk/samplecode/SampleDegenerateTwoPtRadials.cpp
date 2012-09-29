
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "Sk64.h"
#include "SkGradientShader.h"

static void draw_gradient2(SkCanvas* canvas, const SkRect& rect, SkScalar delta) {
    SkColor colors[] = { SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE, SK_ColorMAGENTA };
    SkScalar pos[] = { 0, SkFloatToScalar(0.25f), SkFloatToScalar(0.75f), SK_Scalar1 };

    SkScalar l = rect.fLeft;
    SkScalar t = rect.fTop;
    SkScalar w = rect.width();
    SkScalar h = rect.height();

    SkASSERT(0 == SkScalarMod(w, SK_Scalar1 * 5));

    SkPoint c0 = { l + 2 * w / 5 + delta, t + h / 2 };
    SkPoint c1 = { l + 3 * w / 5, t + h / 2 };
    SkScalar r0 = w / 5;
    SkScalar r1 = 2 * w / 5;
    SkShader* s = SkGradientShader::CreateTwoPointConical(c0, r0, c1, r1, colors,
                                                         pos, SK_ARRAY_COUNT(pos),
                                                         SkShader::kClamp_TileMode);
    SkPaint paint;
    paint.setShader(s)->unref();

    canvas->drawRect(rect, paint);
}


class DegenerateTwoPtRadialsView : public SampleView {

public:
    DegenerateTwoPtRadialsView() {
        fTime = 0;
        this->setBGColor(0xFFDDDDDD);
    }

protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "DegenerateTwoPtRadials");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    virtual void onDrawContent(SkCanvas* canvas) {
        fTime += SampleCode::GetAnimSecondsDelta();
        SkScalar delta = fTime / 15.f;
        int intPart = SkScalarFloor(delta);
        delta = delta - SK_Scalar1 * intPart;
        if (intPart % 2) {
            delta = SK_Scalar1 - delta;
        }
        delta -= SK_ScalarHalf;
        static const int DELTA_SCALE = 500;
        delta /= DELTA_SCALE;

        SkScalar w = SK_Scalar1 * 500;
        SkScalar h = SK_Scalar1 * 500;
        SkScalar l = SK_Scalar1 * 100;
        SkScalar t = SK_Scalar1 * 100;
        draw_gradient2(canvas, SkRect::MakeXYWH(l, t, w, h), delta);
        char txt[512];
        sprintf(txt, "gap at \"tangent\" pt = %f", SkScalarToFloat(delta));
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(SK_ColorBLACK);
        canvas->drawText(txt, strlen(txt), l + w/2 + w*DELTA_SCALE*delta, t + h + SK_Scalar1 * 10, paint);
        this->inval(NULL);
    }

private:
    SkScalar           fTime;
    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new DegenerateTwoPtRadialsView; }
static SkViewRegister reg(MyFactory);
