/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"


class TwoPtConicalView : public SampleView {
public:
    TwoPtConicalView() {}

protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "2PtConical");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    virtual void onDrawContent(SkCanvas* canvas) {
        canvas->translate(SkIntToScalar(10), SkIntToScalar(20));

        SkColor colors[] = { SK_ColorRED, SK_ColorBLUE };
        SkPoint c0 = { 0, 0 };
        SkScalar r0 = 100;
        SkPoint c1 = { 100, 100 };
        SkScalar r1 = 100;
        SkPaint paint;
        paint.setShader(SkGradientShader::MakeTwoPointConical(c0, r0, c1, r1, colors,
                                                             nullptr, 2,
                                                             SkShader::kClamp_TileMode));
        canvas->drawPaint(paint);
    }

private:
    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new TwoPtConicalView; }
static SkViewRegister reg(MyFactory);
