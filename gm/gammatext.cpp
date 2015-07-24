/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkPath.h"
#include "SkGradientShader.h"
#include "SkTypeface.h"

static SkShader* make_heatGradient(const SkPoint pts[2]) {
    const SkColor bw[] = { SK_ColorBLACK, SK_ColorWHITE };

    return SkGradientShader::CreateLinear(pts, bw, NULL,
                                          SK_ARRAY_COUNT(bw),
                                          SkShader::kClamp_TileMode);
}

static bool setFont(SkPaint* paint, const char name[]) {
    SkTypeface* tf = SkTypeface::CreateFromName(name, SkTypeface::kNormal);
    if (tf) {
        paint->setTypeface(tf)->unref();
        return true;
    }
    return false;
}

/**
   Test a set of clipping problems discovered while writing blitAntiRect,
   and test all the code paths through the clipping blitters.
   Each region should show as a blue center surrounded by a 2px green
   border, with no red.
*/

#define HEIGHT 480

class GammaTextGM : public skiagm::GM {
protected:
    SkString onShortName() override {
        SkString name("gammatext");
        name.append(sk_tool_utils::major_platform_os_name());
        return name;
    }

    SkISize onISize() override {
        return SkISize::Make(1024, HEIGHT);
    }

    static void drawGrad(SkCanvas* canvas) {
        SkPoint pts[] = { { 0, 0 }, { 0, SkIntToScalar(HEIGHT) } };
        SkShader* s = make_heatGradient(pts);

        canvas->clear(SK_ColorRED);
        SkPaint paint;
        paint.setShader(s)->unref();
        SkRect r = { 0, 0, SkIntToScalar(1024), SkIntToScalar(HEIGHT) };
        canvas->drawRect(r, paint);
    }

    void onDraw(SkCanvas* canvas) override {
        drawGrad(canvas);

        const SkColor fg[] = {
            0xFFFFFFFF,
            0xFFFFFF00, 0xFFFF00FF, 0xFF00FFFF,
            0xFFFF0000, 0xFF00FF00, 0xFF0000FF,
            0xFF000000,
        };

        const char* text = "Hamburgefons";
        size_t len = strlen(text);

        SkPaint paint;
        setFont(&paint, sk_tool_utils::platform_font_name("serif"));
        paint.setTextSize(SkIntToScalar(16));
        paint.setAntiAlias(true);
        paint.setLCDRenderText(true);

        SkScalar x = SkIntToScalar(10);
        for (size_t i = 0; i < SK_ARRAY_COUNT(fg); ++i) {
            paint.setColor(fg[i]);

            SkScalar y = SkIntToScalar(40);
            SkScalar stopy = SkIntToScalar(HEIGHT);
            while (y < stopy) {
                canvas->drawText(text, len, x, y, paint);
                y += paint.getTextSize() * 2;
            }
            x += SkIntToScalar(1024) / SK_ARRAY_COUNT(fg);
        }
    }

private:
    typedef skiagm::GM INHERITED;
};

DEF_GM( return new GammaTextGM; )

//////////////////////////////////////////////////////////////////////////////

static SkShader* make_gradient(SkColor c) {
    const SkPoint pts[] = { { 0, 0 }, { 240, 0 } };
    SkColor colors[2];
    colors[0] = c;
    colors[1] = SkColorSetA(c, 0);
    return SkGradientShader::CreateLinear(pts, colors, NULL, 2, SkShader::kClamp_TileMode);
}

static void set_face(SkPaint* paint) {
    SkTypeface* face = SkTypeface::CreateFromName("serif", SkTypeface::kItalic);
    SkSafeUnref(paint->setTypeface(face));
}

static void draw_pair(SkCanvas* canvas, SkPaint* paint, SkShader* shader) {
    const char text[] = "Now is the time for all good";
    const size_t len = strlen(text);
    
    paint->setShader(NULL);
    canvas->drawText(text, len, 10, 20, *paint);
    paint->setShader(SkShader::CreateColorShader(paint->getColor()))->unref();
    canvas->drawText(text, len, 10, 40, *paint);
    paint->setShader(shader);
    canvas->drawText(text, len, 10, 60, *paint);
}

class GammaShaderTextGM : public skiagm::GM {
    SkShader* fShaders[3];
    SkColor fColors[3];

public:
    GammaShaderTextGM() {
        const SkColor colors[] = { SK_ColorBLACK, SK_ColorRED, SK_ColorBLUE };
        for (size_t i = 0; i < SK_ARRAY_COUNT(fShaders); ++i) {
            fShaders[i] = NULL;
            fColors[i] = colors[i];
        }
    }

    ~GammaShaderTextGM() override {
        for (size_t i = 0; i < SK_ARRAY_COUNT(fShaders); ++i) {
            SkSafeUnref(fShaders[i]);
        }
    }

protected:
    SkString onShortName() override {
        return SkString("gammagradienttext");
    }
    
    SkISize onISize() override {
        return SkISize::Make(300, 300);
    }

    void onOnceBeforeDraw() override {
        for (size_t i = 0; i < SK_ARRAY_COUNT(fShaders); ++i) {
            fShaders[i] = make_gradient(fColors[i]);
        }
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setLCDRenderText(true);
        paint.setTextSize(18);
        set_face(&paint);

        for (size_t i = 0; i < SK_ARRAY_COUNT(fShaders); ++i) {
            paint.setColor(fColors[i]);
            draw_pair(canvas, &paint, fShaders[i]);
            canvas->translate(0, 80);
        }
    }
    
private:
    typedef skiagm::GM INHERITED;
};

DEF_GM( return new GammaShaderTextGM; )

