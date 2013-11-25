/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkColor.h"
#include "SkMatrixConvolutionImageFilter.h"
#include "SkGradientShader.h"

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
        SkBitmapDevice device(fBitmap);
        SkCanvas canvas(&device);
        canvas.clear(0x00000000);
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(0xFFFFFFFF);
        paint.setTextSize(SkIntToScalar(180));
        SkPoint pts[2] = { SkPoint::Make(0, 0),
                           SkPoint::Make(0, SkIntToScalar(80)) };
        SkColor colors[2] = { 0xFFFFFFFF, 0x40404040 };
        SkScalar pos[2] = { 0, SkIntToScalar(80) };
        paint.setShader(SkGradientShader::CreateLinear(
            pts, colors, pos, 2, SkShader::kClamp_TileMode))->unref();
        const char* str = "e";
        canvas.drawText(str, strlen(str), SkIntToScalar(-10), SkIntToScalar(80), paint);
    }

    virtual SkISize onISize() {
        return make_isize(500, 300);
    }

    void draw(SkCanvas* canvas, int x, int y, const SkIPoint& target,
              SkMatrixConvolutionImageFilter::TileMode tileMode, bool convolveAlpha,
              const SkImageFilter::CropRect* cropRect = NULL) {
        SkScalar kernel[9] = {
            SkIntToScalar( 1), SkIntToScalar( 1), SkIntToScalar( 1),
            SkIntToScalar( 1), SkIntToScalar(-7), SkIntToScalar( 1),
            SkIntToScalar( 1), SkIntToScalar( 1), SkIntToScalar( 1),
        };
        SkISize kernelSize = SkISize::Make(3, 3);
        SkScalar gain = 0.3f, bias = SkIntToScalar(100);
        SkPaint paint;
        SkAutoTUnref<SkImageFilter> filter(
            SkNEW_ARGS(SkMatrixConvolutionImageFilter, (kernelSize,
                                                        kernel,
                                                        gain,
                                                        bias,
                                                        target,
                                                        tileMode,
                                                        convolveAlpha,
                                                        NULL,
                                                        cropRect)));
        paint.setImageFilter(filter);
        canvas->save();
        canvas->translate(SkIntToScalar(x), SkIntToScalar(y));
        canvas->clipRect(SkRect::MakeWH(SkIntToScalar(fBitmap.width()),
                                        SkIntToScalar(fBitmap.height())));
        canvas->drawBitmap(fBitmap, 0, 0, &paint);
        canvas->restore();
    }

    typedef SkMatrixConvolutionImageFilter MCIF;

    virtual void onDraw(SkCanvas* canvas) {
        if (!fInitialized) {
            make_bitmap();
            fInitialized = true;
        }
        canvas->clear(0x00000000);
        SkIPoint target = SkIPoint::Make(1, 0);
        for (int x = 10; x < 310; x += 100) {
            this->draw(canvas, x, 10, target, MCIF::kClamp_TileMode, true);
            this->draw(canvas, x, 110, target, MCIF::kClampToBlack_TileMode, true);
            this->draw(canvas, x, 210, target, MCIF::kRepeat_TileMode, true);
            target.fY++;
        }
        target.fY = 1;
        SkImageFilter::CropRect rect(SkRect::MakeXYWH(10, 5, 60, 60));
        this->draw(canvas, 310, 10, target, MCIF::kClamp_TileMode, true, &rect);
        this->draw(canvas, 310, 110, target, MCIF::kClampToBlack_TileMode, true, &rect);
        this->draw(canvas, 310, 210, target, MCIF::kRepeat_TileMode, true, &rect);

        this->draw(canvas, 410, 10, target, MCIF::kClamp_TileMode, false);
        this->draw(canvas, 410, 110, target, MCIF::kClampToBlack_TileMode, false);
        this->draw(canvas, 410, 210, target, MCIF::kRepeat_TileMode, false);
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
