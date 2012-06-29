
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

namespace skiagm {

class ShaderGM : public GM {
public:
    ShaderGM() {
        SkPoint pts[2];
        SkColor colors[2];

        pts[0].set(0, 0);
        pts[1].set(SkIntToScalar(100), 0);
        colors[0] = SK_ColorRED;
        colors[1] = SK_ColorBLUE;
        SkShader* shaderA = SkGradientShader::CreateLinear(pts, colors, NULL, 2,
                                                           SkShader::kClamp_TileMode);

        pts[0].set(0, 0);
        pts[1].set(0, SkIntToScalar(100));
        colors[0] = SK_ColorBLACK;
        colors[1] = SkColorSetARGB(0x80, 0, 0, 0);
        SkShader* shaderB = SkGradientShader::CreateLinear(pts, colors, NULL, 2,
                                                           SkShader::kClamp_TileMode);

        SkXfermode* mode = SkXfermode::Create(SkXfermode::kDstIn_Mode);

        fShader = new SkComposeShader(shaderA, shaderB, mode);
        shaderA->unref();
        shaderB->unref();
        mode->unref();
    }

    virtual ~ShaderGM() {
        SkSafeUnref(fShader);
    }

protected:
    virtual SkString onShortName() SK_OVERRIDE {
        return SkString("composeshader");
    }

    virtual SkISize onISize() SK_OVERRIDE {
        return make_isize(640, 480);
    }

    virtual uint32_t onGetFlags() const SK_OVERRIDE {
        return this->INHERITED::onGetFlags() | GM::kSkipPDF_Flag;
    }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {

        SkPaint paint;
        SkRect  r;

        paint.setColor(SK_ColorGREEN);
        canvas->drawRectCoords(0, 0, SkIntToScalar(100), SkIntToScalar(100), paint);
        paint.setShader(fShader);
        canvas->drawRectCoords(0, 0, SkIntToScalar(100), SkIntToScalar(100), paint);
    }

private:
    SkShader*   fShader;
    typedef GM INHERITED ;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new ShaderGM; }
static GMRegistry reg(MyFactory);

} // namespace

