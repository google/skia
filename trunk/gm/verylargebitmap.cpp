/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkPath.h"

static void make_bm(SkBitmap* bm, int width, int height, SkColor color) {
    bm->setConfig(SkBitmap::kARGB_8888_Config, width, height);
    bm->allocPixels();
    bm->eraseColor(color);
    bm->setImmutable();
}

static void show_bm(SkCanvas* canvas, int width, int height, SkColor color) {
    SkBitmap bm;
    make_bm(&bm, width, height, color);

    SkPaint paint;
    SkRect r;
    SkIRect ir;

    paint.setStyle(SkPaint::kStroke_Style);

    ir.set(0, 0, 128, 128);
    r.set(ir);

    canvas->save();
    canvas->clipRect(r);
    canvas->drawBitmap(bm, 0, 0, NULL);
    canvas->restore();
    canvas->drawRect(r, paint);

    r.offset(SkIntToScalar(150), 0);
    // exercises extract bitmap, but not shader
    canvas->drawBitmapRect(bm, &ir, r, NULL);
    canvas->drawRect(r, paint);

    r.offset(SkIntToScalar(150), 0);
    // exercises bitmapshader
    canvas->drawBitmapRect(bm, NULL, r, NULL);
    canvas->drawRect(r, paint);
}

class VeryLargeBitmapGM : public skiagm::GM {
public:
    VeryLargeBitmapGM() {}

protected:
    virtual SkString onShortName() SK_OVERRIDE {
        return SkString("verylargebitmap");
    }

    virtual SkISize onISize() SK_OVERRIDE {
        return SkISize::Make(640, 480);
    }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        int veryBig = 100*1024; // 64K < size
        int big = 60*1024;      // 32K < size < 64K
        int small = 300;

        canvas->translate(SkIntToScalar(10), SkIntToScalar(10));
        show_bm(canvas, small, small, SK_ColorRED);
        canvas->translate(0, SkIntToScalar(150));

        show_bm(canvas, big, small, SK_ColorBLUE);
        canvas->translate(0, SkIntToScalar(150));

        // as of this writing, the raster code will fail to draw the scaled version
        // since it has a 64K limit on x,y coordinates... (but gpu should succeed)
        show_bm(canvas, veryBig, small, SK_ColorGREEN);
    }

private:
    typedef skiagm::GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

// This GM allocates more memory than Android devices are capable of fulfilling.
#ifndef SK_BUILD_FOR_ANDROID
static skiagm::GM* MyFactory(void*) { return new VeryLargeBitmapGM; }
static skiagm::GMRegistry reg(MyFactory);
#endif
