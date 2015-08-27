
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkGraphics.h"
#include "SkRandom.h"
#include "SkGradientShader.h"
#include "SkPicture.h"

static SkShader* make_linear() {
    SkPoint pts[] = { 0, 0, SK_Scalar1/500, SK_Scalar1/500 };
    SkColor colors[] = { SK_ColorRED, SK_ColorBLUE };
    return SkGradientShader::CreateLinear(pts, colors, nullptr, 2,
                                          SkShader::kClamp_TileMode);
}

class ClampView : public SampleView {
    SkShader*   fGrad;

public:
    ClampView() {
        fGrad = make_linear();
    }

    virtual ~ClampView() {
        fGrad->unref();
    }

protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "Clamp");
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
    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new ClampView; }
static SkViewRegister reg(MyFactory);
