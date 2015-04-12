/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkBlurMask.h"
#include "SkBlurMaskFilter.h"
#include "SkCanvas.h"
#include "SkDrawFilter.h"
#include "SkPaint.h"

/**
 * Initial test coverage for SkDrawFilter.
 * Draws two rectangles; if draw filters are broken, they will match.
 * If draw filters are working correctly, the first will be blue and blurred,
 * the second red and sharp.
 */

namespace {
class TestFilter : public SkDrawFilter {
public:
    bool filter(SkPaint* p, Type) override {
        p->setColor(SK_ColorRED);
        p->setMaskFilter(NULL);
        return true;
    }
};
}

class DrawFilterGM : public skiagm::GM {
    SkAutoTUnref<SkMaskFilter> fBlur;

protected:
    SkISize onISize() override {
        return SkISize::Make(320, 240);
    }

    SkString onShortName() override {
        return SkString("drawfilter");
    }

    void onOnceBeforeDraw() override {
        fBlur.reset(SkBlurMaskFilter::Create(kNormal_SkBlurStyle,
                    SkBlurMask::ConvertRadiusToSigma(10.0f),
                    kLow_SkBlurQuality));
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint p;
        p.setColor(SK_ColorBLUE);
        p.setMaskFilter(fBlur.get());
        SkRect r = { 20, 20, 100, 100 };
        canvas->setDrawFilter(NULL);
        canvas->drawRect(r, p);
        TestFilter redNoBlur;
        canvas->setDrawFilter(&redNoBlur);
        canvas->translate(120.0f, 40.0f);
        canvas->drawRect(r, p);

        // Must unset if the DrawFilter is from the stack to avoid refcount errors!
        canvas->setDrawFilter(NULL);
    }

private:
    typedef GM INHERITED;
};

static skiagm::GM* MyFactory(void*) { return new DrawFilterGM; }
static skiagm::GMRegistry reg(MyFactory);

