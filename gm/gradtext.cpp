/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"

// test shader w/ transparency
static SkShader* make_grad(SkScalar width) {
    SkColor colors[] = { SK_ColorRED, 0x0000FF00, SK_ColorBLUE };
    SkPoint pts[] = { { 0, 0 }, { width, 0 } };
    return SkGradientShader::CreateLinear(pts, colors, NULL,
                                          SK_ARRAY_COUNT(colors),
                                          SkShader::kMirror_TileMode);
}

// test opaque shader
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

    virtual SkISize onISize() { return make_isize(500, 480); }

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
        canvas->translate(0, paint.getTextSize() * 4/3);
        draw_text(canvas, p);
        p.setLCDRenderText(true);
        canvas->translate(0, paint.getTextSize() * 4/3);
        draw_text(canvas, p);
    }

    virtual void onDraw(SkCanvas* canvas) {
        SkPaint paint;
        paint.setTextSize(SkIntToScalar(26));

        const SkISize& size = this->getISize();
        SkRect r = SkRect::MakeWH(SkIntToScalar(size.width()),
                                  SkIntToScalar(size.height()) / 2);
        canvas->drawRect(r, paint);

        canvas->translate(SkIntToScalar(20), paint.getTextSize());

        for (int i = 0; i < 2; ++i) {
            paint.setShader(make_grad(SkIntToScalar(80)))->unref();
            draw_text3(canvas, paint);

            canvas->translate(0, paint.getTextSize() * 2);

            paint.setShader(make_grad2(SkIntToScalar(80)))->unref();
            draw_text3(canvas, paint);
            
            canvas->translate(0, paint.getTextSize() * 2);
        }
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new GradTextGM; }
static GMRegistry reg(MyFactory);

}

