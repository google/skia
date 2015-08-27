
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
#include "SkGraphics.h"
#include "SkImageDecoder.h"
#include "SkPath.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkUtils.h"
#include "SkXfermode.h"
#include "SkComposeShader.h"
#include "SkColorPriv.h"
#include "SkColorFilter.h"
#include "SkTime.h"
#include "SkTypeface.h"

static SkShader* make_bitmapfade(const SkBitmap& bm)
{
    SkPoint pts[2];
    SkColor colors[2];

    pts[0].set(0, 0);
    pts[1].set(0, SkIntToScalar(bm.height()));
    colors[0] = SK_ColorBLACK;
    colors[1] = SkColorSetARGB(0, 0, 0, 0);
    SkShader* shaderA = SkGradientShader::CreateLinear(pts, colors, nullptr, 2, SkShader::kClamp_TileMode);

    SkShader* shaderB = SkShader::CreateBitmapShader(bm,
                        SkShader::kClamp_TileMode, SkShader::kClamp_TileMode);

    SkXfermode* mode = SkXfermode::Create(SkXfermode::kDstIn_Mode);

    SkShader* shader = new SkComposeShader(shaderB, shaderA, mode);
    shaderA->unref();
    shaderB->unref();
    mode->unref();

    return shader;
}

class ShaderView : public SampleView {
public:
    SkShader*   fShader;
    SkBitmap    fBitmap;

    ShaderView() {
        SkImageDecoder::DecodeFile("/skimages/logo.gif", &fBitmap);

        SkPoint pts[2];
        SkColor colors[2];

        pts[0].set(0, 0);
        pts[1].set(SkIntToScalar(100), 0);
        colors[0] = SK_ColorRED;
        colors[1] = SK_ColorBLUE;
        SkShader* shaderA = SkGradientShader::CreateLinear(pts, colors, nullptr, 2, SkShader::kClamp_TileMode);

        pts[0].set(0, 0);
        pts[1].set(0, SkIntToScalar(100));
        colors[0] = SK_ColorBLACK;
        colors[1] = SkColorSetARGB(0x80, 0, 0, 0);
        SkShader* shaderB = SkGradientShader::CreateLinear(pts, colors, nullptr, 2, SkShader::kClamp_TileMode);

        SkXfermode* mode = SkXfermode::Create(SkXfermode::kDstIn_Mode);

        fShader = new SkComposeShader(shaderA, shaderB, mode);
        shaderA->unref();
        shaderB->unref();
        mode->unref();
    }
    virtual ~ShaderView() {
        SkSafeUnref(fShader);
    }

protected:
    // overrides from SkEventSink
    bool onQuery(SkEvent* evt) override {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "Shaders");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void onDrawContent(SkCanvas* canvas) override {
        canvas->drawBitmap(fBitmap, 0, 0);

        canvas->translate(SkIntToScalar(20), SkIntToScalar(120));

        SkPaint paint;
        SkRect  r;

        paint.setColor(SK_ColorGREEN);
        canvas->drawRectCoords(0, 0, SkIntToScalar(100), SkIntToScalar(100), paint);
        paint.setShader(fShader);
        canvas->drawRectCoords(0, 0, SkIntToScalar(100), SkIntToScalar(100), paint);

        canvas->translate(SkIntToScalar(110), 0);

        int w = fBitmap.width();
        int h = fBitmap.height();
        w = 120;
        h = 80;
        r.set(0, 0, SkIntToScalar(w), SkIntToScalar(h));

        paint.setShader(nullptr);
        canvas->drawRect(r, paint);
        paint.setShader(make_bitmapfade(fBitmap))->unref();
        canvas->drawRect(r, paint);
    }

    SkView::Click* onFindClickHandler(SkScalar x, SkScalar y, unsigned modi) override {
        this->inval(nullptr);
        return this->INHERITED::onFindClickHandler(x, y, modi);
    }

    bool onClick(Click* click)  override {
        return this->INHERITED::onClick(click);
    }

private:
    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new ShaderView; }
static SkViewRegister reg(MyFactory);
