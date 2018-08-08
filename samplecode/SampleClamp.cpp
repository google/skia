/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Sample.h"
#include "SkCanvas.h"
#include "SkGraphics.h"
#include "SkRandom.h"
#include "SkGradientShader.h"
#include "SkPicture.h"

static sk_sp<SkShader> make_linear() {
    SkPoint pts[] = { 0, 0, SK_Scalar1/500, SK_Scalar1/500 };
    SkColor colors[] = { SK_ColorRED, SK_ColorBLUE };
    return SkGradientShader::MakeLinear(pts, colors, nullptr, 2, SkShader::kClamp_TileMode);
}

class ClampView : public Sample {
    sk_sp<SkShader> fGrad;

public:
    ClampView() {
        fGrad = make_linear();
    }

protected:
    virtual bool onQuery(Sample::Event* evt) {
        if (Sample::TitleQ(*evt)) {
            Sample::TitleR(evt, "Clamp");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    virtual void onDrawContent(SkCanvas* canvas) {
        SkPaint paint;
        paint.setShader(fGrad);

//        canvas->translate(this->width()/2, this->height()/2);
        canvas->translate(64, 64);
        canvas->drawPaint(paint);

        SkPicture pic;
        SkCanvas* c = pic.beginRecording(100, 100, 0);
        SkCanvas::LayerIter layerIterator(c, false);
        layerIterator.next();
        layerIterator.done();
    }

private:
    typedef Sample INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static Sample* MyFactory() { return new ClampView; }
static SampleRegister reg(MyFactory);
