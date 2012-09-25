/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkMatrixConvolutionImageFilter.h"

namespace skiagm {

class MatrixConvolutionGM : public GM {
public:
    MatrixConvolutionGM() : fInitialized(false) {
        this->setBGColor(0x00000000);
    }

protected:
    virtual SkString onShortName() {
        return SkString("matrixconvolution");
    }

    void make_bitmap() {
        fBitmap.setConfig(SkBitmap::kARGB_8888_Config, 80, 80);
        fBitmap.allocPixels();
        SkDevice device(fBitmap);
        SkCanvas canvas(&device);
        canvas.clear(0x00000000);
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(0xFFFFFFFF);
        paint.setTextSize(SkIntToScalar(180));
        const char* str = "e";
        canvas.drawText(str, strlen(str), SkIntToScalar(-10), SkIntToScalar(80), paint);
    }

    virtual SkISize onISize() {
        return make_isize(400, 300);
    }

    void draw(SkCanvas* canvas, int x, int y, const SkIPoint& target, SkMatrixConvolutionImageFilter::TileMode tileMode, bool convolveAlpha) {
        SkScalar kernel[9] = {
            SkIntToScalar( 1), SkIntToScalar( 1), SkIntToScalar( 1),
            SkIntToScalar( 1), SkIntToScalar(-7), SkIntToScalar( 1),
            SkIntToScalar( 1), SkIntToScalar( 1), SkIntToScalar( 1),
        };
        SkISize kernelSize = SkISize::Make(3, 3);
        SkScalar gain = SkFloatToScalar(0.3f), bias = SkIntToScalar(100);
        SkPaint paint;
        SkAutoTUnref<SkImageFilter> filter(SkNEW_ARGS(SkMatrixConvolutionImageFilter, (kernelSize, kernel, gain, bias, target, tileMode, convolveAlpha)));
        paint.setImageFilter(filter);
        canvas->drawSprite(fBitmap, x, y, &paint);
    }

    virtual void onDraw(SkCanvas* canvas) {
        if (!fInitialized) {
            make_bitmap();
            fInitialized = true;
        }
        canvas->clear(0x00000000);
        SkIPoint target = SkIPoint::Make(1, 1);
        for (target.fY = 0; target.fY < 3; ++target.fY) {
            int y = target.fY * 100 + 10;
            draw(canvas, 10, y, target, SkMatrixConvolutionImageFilter::kClamp_TileMode, true);
            draw(canvas, 110, y, target, SkMatrixConvolutionImageFilter::kClampToBlack_TileMode, true);
            draw(canvas, 210, y, target, SkMatrixConvolutionImageFilter::kRepeat_TileMode, true);
        }
        target.fY = 1;
        draw(canvas, 310, 10, target, SkMatrixConvolutionImageFilter::kClamp_TileMode, false);
    }

private:
    typedef GM INHERITED;
    SkBitmap fBitmap;
    bool fInitialized;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new MatrixConvolutionGM; }
static GMRegistry reg(MyFactory);

}
