/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"

static SkShader* make_grad(SkScalar width) {
    SkColor colors[] = { SK_ColorBLACK, SK_ColorBLACK, 0 };
    SkScalar pos[] = { 0, SK_Scalar1 * 5 / 10, SK_Scalar1 };
    SkPoint pts[] = { { 0, 0 }, { width, 0 } };
    return SkGradientShader::CreateLinear(pts, colors, pos,
                                          SK_ARRAY_COUNT(colors),
                                          SkShader::kMirror_TileMode);
}

static SkShader* make_grad2(SkScalar width) {
    SkColor colors[] = { SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE };
    SkPoint pts[] = { { 0, 0 }, { width, 0 } };
    return SkGradientShader::CreateLinear(pts, colors, NULL,
                                          SK_ARRAY_COUNT(colors),
                                          SkShader::kMirror_TileMode);
}

namespace skiagm {
    
class GradTextGM : public GM {
public:
    GradTextGM () {}

protected:

    virtual SkString onShortName() {
        return SkString("gradtext");
    }

    virtual SkISize onISize() { return make_isize(640, 480); }

    static void draw_text(SkCanvas* canvas, const SkPaint& paint) {
        const char* text = "When in the course of human events";
        size_t len = strlen(text);
        canvas->drawText(text, len, 0, 0, paint);
    }

    static void draw_text3(SkCanvas* canvas, const SkPaint& paint) {
        SkPaint p(paint);

        p.setAntiAlias(false);
        draw_text(canvas, p);
        p.setAntiAlias(true);
        canvas->translate(0, SkIntToScalar(20));
        draw_text(canvas, p);
        p.setLCDRenderText(true);
        canvas->translate(0, SkIntToScalar(20));
        draw_text(canvas, p);
    }

    virtual void onDraw(SkCanvas* canvas) {
        canvas->translate(SkIntToScalar(20), SkIntToScalar(20));

        //  The blits special-case opaque and non-paque shaders, so test both

        SkPaint paint;
        paint.setTextSize(SkIntToScalar(16));
        paint.setShader(make_grad(SkIntToScalar(80)))->unref();

        draw_text3(canvas, paint);
        canvas->translate(0, SkIntToScalar(40));
        paint.setShader(make_grad2(SkIntToScalar(80)))->unref();
        draw_text3(canvas, paint);
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new GradTextGM; }
static GMRegistry reg(MyFactory);

}

