/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFilterQuality.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTileMode.h"

 /***
  *
  * This GM reproduces Skia bug 2904, in which a tiled bitmap shader was failing to draw correctly
  * when fractional image scaling was ignored by the high quality bitmap scaler.
  *
  ***/

namespace skiagm {

class TiledScaledBitmapGM : public GM {
public:

    TiledScaledBitmapGM() {
    }

protected:
    SkString onShortName() override {
        return SkString("tiledscaledbitmap");
    }

    SkISize onISize() override {
        return SkISize::Make(1016, 616);
    }

    static SkBitmap make_bm(int width, int height) {
        SkBitmap bm;
        bm.allocN32Pixels(width, height);
        bm.eraseColor(SK_ColorTRANSPARENT);
        SkCanvas canvas(bm);
        SkPaint paint;
        paint.setAntiAlias(true);
        canvas.drawCircle(width/2.f, height/2.f, width/4.f, paint);
        return bm;
    }

    void onOnceBeforeDraw() override {
        fBitmap = make_bm(360, 288);
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint paint;

        paint.setAntiAlias(true);
        paint.setFilterQuality(kHigh_SkFilterQuality);

        SkMatrix mat;
        mat.setScale(121.f/360.f, 93.f/288.f);
        mat.postTranslate(-72, -72);

        paint.setShader(fBitmap.makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat, &mat));
        canvas->drawRect({ 8, 8, 1008, 608 }, paint);
    }

private:
    SkBitmap fBitmap;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new TiledScaledBitmapGM;)
}
