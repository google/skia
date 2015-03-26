/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#include "SkCanvas.h"
#include "SkComposeShader.h"
#include "SkGradientShader.h"
#include "SkGraphics.h"
#include "SkShader.h"
#include "SkString.h"
#include "SkXfermode.h"

static SkShader* make_shader(SkXfermode::Mode mode) {
    SkPoint pts[2];
    SkColor colors[2];

    pts[0].set(0, 0);
    pts[1].set(SkIntToScalar(100), 0);
    colors[0] = SK_ColorRED;
    colors[1] = SK_ColorBLUE;
    SkAutoTUnref<SkShader> shaderA(SkGradientShader::CreateLinear(pts, colors, NULL, 2,
                                                                  SkShader::kClamp_TileMode));

    pts[0].set(0, 0);
    pts[1].set(0, SkIntToScalar(100));
    colors[0] = SK_ColorBLACK;
    colors[1] = SkColorSetARGB(0x80, 0, 0, 0);
    SkAutoTUnref<SkShader> shaderB(SkGradientShader::CreateLinear(pts, colors, NULL, 2,
                                                                  SkShader::kClamp_TileMode));

    SkAutoTUnref<SkXfermode> xfer(SkXfermode::Create(mode));

    return SkNEW_ARGS(SkComposeShader, (shaderA, shaderB, xfer));
}

class ComposeShaderGM : public skiagm::GM {
public:
    ComposeShaderGM() {
        fShader = make_shader(SkXfermode::kDstIn_Mode);
    }

    virtual ~ComposeShaderGM() {
        SkSafeUnref(fShader);
    }

protected:
    SkString onShortName() override {
        return SkString("composeshader");
    }

    SkISize onISize() override {
        return SkISize::Make(120, 120);
    }

    void onDraw(SkCanvas* canvas) override {

        SkPaint paint;

        paint.setColor(SK_ColorGREEN);
        canvas->drawRectCoords(0, 0, SkIntToScalar(100), SkIntToScalar(100), paint);
        paint.setShader(fShader);
        canvas->drawRectCoords(0, 0, SkIntToScalar(100), SkIntToScalar(100), paint);
    }

protected:
    SkShader*   fShader;

private:
    typedef GM INHERITED ;
};

class ComposeShaderAlphaGM : public skiagm::GM {
public:
    ComposeShaderAlphaGM() {}

protected:
    SkString onShortName() override {
        return SkString("composeshader_alpha");
    }

    SkISize onISize() override {
        return SkISize::Make(220, 750);
    }

    void onDraw(SkCanvas* canvas) override {
        SkAutoTUnref<SkShader> shader0(make_shader(SkXfermode::kDstIn_Mode));
        SkAutoTUnref<SkShader> shader1(make_shader(SkXfermode::kSrcOver_Mode));
        SkShader* shaders[] = { shader0.get(), shader1.get() };

        SkPaint paint;
        paint.setColor(SK_ColorGREEN);

        const SkRect r = SkRect::MakeXYWH(5, 5, 100, 100);

        for (size_t y = 0; y < SK_ARRAY_COUNT(shaders); ++y) {
            SkShader* shader = shaders[y];
            canvas->save();
            for (int alpha = 0xFF; alpha > 0; alpha -= 0x28) {
                paint.setAlpha(0xFF);
                paint.setShader(NULL);
                canvas->drawRect(r, paint);

                paint.setAlpha(alpha);
                paint.setShader(shader);
                canvas->drawRect(r, paint);

                canvas->translate(r.width() + 5, 0);
            }
            canvas->restore();
            canvas->translate(0, r.height() + 5);
        }
    }
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return new ComposeShaderGM; )
DEF_GM( return new ComposeShaderAlphaGM; )
