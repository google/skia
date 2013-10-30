/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkPath.h"

static void make_bm(SkBitmap* bm, int width, int height, SkColor colors[2]) {
    bm->setConfig(SkBitmap::kARGB_8888_Config, width, height);
    bm->allocPixels();
    SkCanvas canvas(*bm);
    SkPoint center = {SkIntToScalar(width)/2, SkIntToScalar(height)/2};
    SkScalar radius = 40;
    SkShader* shader = SkGradientShader::CreateRadial(center, radius, colors, NULL, 2,
                                                      SkShader::kMirror_TileMode);
    SkPaint paint;
    paint.setShader(shader)->unref();
    paint.setXfermodeMode(SkXfermode::kSrc_Mode);
    canvas.drawPaint(paint);
    bm->setImmutable();
}

static void show_bm(SkCanvas* canvas, int width, int height, SkColor colors[2]) {
    SkBitmap bm;
    make_bm(&bm, width, height, colors);

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
        return SkISize::Make(500, 600);
    }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        int veryBig = 65*1024; // 64K < size
        int big = 33*1024;     // 32K < size < 64K
        // smaller than many max texture sizes, but large enough to gpu-tile for memory reasons.
        int medium = 5*1024;
        int small = 150;

        SkColor colors[2];

        canvas->translate(SkIntToScalar(10), SkIntToScalar(10));
        colors[0] = SK_ColorRED;
        colors[1] = SK_ColorGREEN;
        show_bm(canvas, small, small, colors);
        canvas->translate(0, SkIntToScalar(150));

        colors[0] = SK_ColorBLUE;
        colors[1] = SK_ColorMAGENTA;
        show_bm(canvas, big, small, colors);
        canvas->translate(0, SkIntToScalar(150));

        colors[0] = SK_ColorMAGENTA;
        colors[1] = SK_ColorYELLOW;
        show_bm(canvas, medium, medium, colors);
        canvas->translate(0, SkIntToScalar(150));

        colors[0] = SK_ColorGREEN;
        colors[1] = SK_ColorYELLOW;
        // as of this writing, the raster code will fail to draw the scaled version
        // since it has a 64K limit on x,y coordinates... (but gpu should succeed)
        show_bm(canvas, veryBig, small, colors);
    }

    virtual uint32_t onGetFlags() const {
#ifdef SK_BUILD_FOR_WIN32
        // The Windows bot runs out of memory in replay modes on this test in 32bit builds:
        // http://skbug.com/1756
        return kSkipPicture_Flag            |
               kSkipPipe_Flag               |
               kSkipPipeCrossProcess_Flag   |
               kSkipTiled_Flag              |
               kSkipScaledReplay_Flag;
#else
        return 0;
#endif
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
