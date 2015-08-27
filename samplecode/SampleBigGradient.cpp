
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

static SkShader* make_grad(SkScalar w, SkScalar h) {
    SkColor colors[] = { 0xFF000000, 0xFF333333 };
    SkPoint pts[] = { { 0, 0 }, { w, h } };
    return SkGradientShader::CreateLinear(pts, colors, nullptr, 2,
                                          SkShader::kClamp_TileMode);
}

class BigGradientView : public SampleView {
public:
    BigGradientView() {}

protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "BigGradient");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    virtual void onDrawContent(SkCanvas* canvas) {
        SkRect r;
        r.set(0, 0, this->width(), this->height());
        SkPaint p;
        p.setShader(make_grad(this->width(), this->height()))->unref();
        canvas->drawRect(r, p);
    }

private:
    typedef SampleView INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new BigGradientView; }
static SkViewRegister reg(MyFactory);
