/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkTypeface.h"

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

static SkShader* make_chrome_solid() {
    SkColor colors[] = { SK_ColorGREEN, SK_ColorGREEN };
    SkPoint pts[] = { { 0, 0 }, { 1, 0 } };
    return SkGradientShader::CreateLinear(pts, colors, NULL, 2,
                                          SkShader::kClamp_TileMode);
}

namespace skiagm {

// Replicate chrome layout test - clipped pathed gradient-shaded text
class ChromeGradTextGM1 : public GM {
public:
    ChromeGradTextGM1() { }
protected:

    virtual SkString onShortName() { return SkString("chrome_gradtext1"); }
    virtual SkISize onISize() { return SkISize::Make(500, 480); }
    virtual void onDraw(SkCanvas* canvas) {
        SkPaint paint;
        sk_tool_utils::set_portable_typeface(&paint);
        SkRect r = SkRect::MakeWH(SkIntToScalar(100), SkIntToScalar(100));

        canvas->clipRect(r);

        paint.setColor(SK_ColorRED);
        canvas->drawRect(r, paint);

        // Minimal repro doesn't require AA, LCD, or a nondefault typeface
        paint.setShader(make_chrome_solid())->unref();
        paint.setTextSize(SkIntToScalar(500));

        canvas->drawText("I", 1, 0, 100, paint);
    }
private:
    typedef GM INHERITED;
};


// Replicate chrome layout test - switching between solid & gradient text
class ChromeGradTextGM2 : public GM {
public:
    ChromeGradTextGM2() { }
protected:

    virtual SkString onShortName() { return SkString("chrome_gradtext2"); }
    virtual SkISize onISize() { return SkISize::Make(500, 480); }
    virtual void onDraw(SkCanvas* canvas) {
        SkPaint paint;
        sk_tool_utils::set_portable_typeface(&paint);

        paint.setStyle(SkPaint::kFill_Style);
        canvas->drawText("Normal Fill Text", 16, 0, 50, paint);
        paint.setStyle(SkPaint::kStroke_Style);
        canvas->drawText("Normal Stroke Text", 18, 0, 100, paint);

        // Minimal repro doesn't require AA, LCD, or a nondefault typeface
        paint.setShader(make_chrome_solid())->unref();

        paint.setStyle(SkPaint::kFill_Style);
        canvas->drawText("Gradient Fill Text", 18, 0, 150, paint);
        paint.setStyle(SkPaint::kStroke_Style);
        canvas->drawText("Gradient Stroke Text", 20, 0, 200, paint);
    }
private:
    typedef GM INHERITED;
};



class GradTextGM : public GM {
public:
    GradTextGM () {}

protected:
    SkString onShortName() override {
        return SkString("gradtext");
    }

    SkISize onISize() override { return SkISize::Make(500, 480); }

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

    void onDraw(SkCanvas* canvas) override {
        SkPaint paint;
        sk_tool_utils::set_portable_typeface(&paint);
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
static GM* CMyFactory(void*) { return new ChromeGradTextGM1; }
static GM* CMyFactory2(void*) { return new ChromeGradTextGM2; }

static GMRegistry reg(MyFactory);
static GMRegistry Creg(CMyFactory);
static GMRegistry Creg2(CMyFactory2);
}
