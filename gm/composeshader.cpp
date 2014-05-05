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

class ComposeShaderGM : public skiagm::GM {
public:
    ComposeShaderGM() {
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

    virtual ~ComposeShaderGM() {
        SkSafeUnref(fShader);
    }

protected:
    virtual uint32_t onGetFlags() const SK_OVERRIDE {
        return kSkipTiled_Flag;
    }

    virtual SkString onShortName() SK_OVERRIDE {
        return SkString("composeshader");
    }

    virtual SkISize onISize() SK_OVERRIDE {
        return SkISize::Make(120, 120);
    }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {

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

class ComposeShaderAlphaGM : public ComposeShaderGM {
public:
    ComposeShaderAlphaGM() {}

protected:
    virtual SkString onShortName() SK_OVERRIDE {
        return SkString("composeshader_alpha");
    }
    
    virtual SkISize onISize() SK_OVERRIDE {
        return SkISize::Make(120, 1024);
    }
    
    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        SkPaint paint;
        paint.setColor(SK_ColorGREEN);
        
        const SkRect r = SkRect::MakeXYWH(5, 5, 100, 100);
        for (int alpha = 0xFF; alpha > 0; alpha -= 0x20) {
            paint.setAlpha(0xFF);
            paint.setShader(NULL);
            canvas->drawRect(r, paint);
            
            paint.setAlpha(alpha);
            paint.setShader(fShader);
            canvas->drawRect(r, paint);
            
            canvas->translate(r.width() + 5, 0);
        }
    }
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return new ComposeShaderGM; )
DEF_GM( return new ComposeShaderAlphaGM; )

