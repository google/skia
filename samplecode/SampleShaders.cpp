/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "DecodeFile.h"
#include "Sample.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkGraphics.h"
#include "SkPath.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkUTF.h"
#include "SkColorPriv.h"
#include "SkColorFilter.h"
#include "SkTime.h"
#include "SkTypeface.h"

static sk_sp<SkShader> make_bitmapfade(const SkBitmap& bm) {
    SkPoint pts[2];
    SkColor colors[2];

    pts[0].set(0, 0);
    pts[1].set(0, SkIntToScalar(bm.height()));
    colors[0] = SK_ColorBLACK;
    colors[1] = SkColorSetARGB(0, 0, 0, 0);
    auto shaderA = SkGradientShader::MakeLinear(pts, colors, nullptr, 2, SkTileMode::kClamp);

    auto shaderB = SkShader::MakeBitmapShader(bm, SkTileMode::kClamp, SkTileMode::kClamp);

    return SkShader::MakeComposeShader(std::move(shaderB), std::move(shaderA), SkBlendMode::kDstIn);
}

class ShaderView : public Sample {
public:
    sk_sp<SkShader> fShader;
    SkBitmap        fBitmap;

    ShaderView() {
        decode_file("/skimages/logo.gif", &fBitmap);

        SkPoint pts[2];
        SkColor colors[2];

        pts[0].set(0, 0);
        pts[1].set(SkIntToScalar(100), 0);
        colors[0] = SK_ColorRED;
        colors[1] = SK_ColorBLUE;
        auto shaderA = SkGradientShader::MakeLinear(pts, colors, nullptr, 2, SkTileMode::kClamp);

        pts[0].set(0, 0);
        pts[1].set(0, SkIntToScalar(100));
        colors[0] = SK_ColorBLACK;
        colors[1] = SkColorSetARGB(0x80, 0, 0, 0);
        auto shaderB = SkGradientShader::MakeLinear(pts, colors, nullptr, 2, SkTileMode::kClamp);

        fShader = SkShader::MakeComposeShader(std::move(shaderA), std::move(shaderB),
                                              SkBlendMode::kDstIn);
    }

protected:
    bool onQuery(Sample::Event* evt) override {
        if (Sample::TitleQ(*evt)) {
            Sample::TitleR(evt, "Shaders");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void onDrawContent(SkCanvas* canvas) override {
        canvas->drawBitmap(fBitmap, 0, 0);

        canvas->translate(20, 120);

        SkPaint paint;
        SkRect  r;

        paint.setColor(SK_ColorGREEN);
        canvas->drawRect(SkRect::MakeWH(100, 100), paint);
        paint.setShader(fShader);
        canvas->drawRect(SkRect::MakeWH(100, 100), paint);

        canvas->translate(SkIntToScalar(110), 0);

        int w = fBitmap.width();
        int h = fBitmap.height();
        w = 120;
        h = 80;
        r.set(0, 0, SkIntToScalar(w), SkIntToScalar(h));

        paint.setShader(nullptr);
        canvas->drawRect(r, paint);
        paint.setShader(make_bitmapfade(fBitmap));
        canvas->drawRect(r, paint);
    }

private:
    typedef Sample INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_SAMPLE( return new ShaderView(); )
