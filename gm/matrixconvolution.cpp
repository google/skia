/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"
#include "SkColor.h"
#include "SkGradientShader.h"
#include "SkMatrixConvolutionImageFilter.h"
#include "SkPixelRef.h"

namespace skiagm {

class MatrixConvolutionGM : public GM {
public:
    MatrixConvolutionGM(SkColor colorOne, SkColor colorTwo, const char* nameSuffix)
            : fNameSuffix(nameSuffix) {
        this->setBGColor(0x00000000);
        fColors[0] = colorOne;
        fColors[1] = colorTwo;
    }

protected:

    SkString onShortName() override {
        return SkStringPrintf("matrixconvolution%s", fNameSuffix);
    }

    void makeBitmap() {
        // Draw our bitmap in N32, so legacy devices get "premul" values they understand
        SkBitmap n32Bitmap;
        n32Bitmap.allocN32Pixels(80, 80);
        SkCanvas canvas(n32Bitmap);
        canvas.clear(0x00000000);
        SkPaint paint;
        paint.setAntiAlias(true);
        sk_tool_utils::set_portable_typeface(&paint);
        paint.setColor(0xFFFFFFFF);
        paint.setTextSize(SkIntToScalar(180));
        SkPoint pts[2] = { SkPoint::Make(0, 0),
                           SkPoint::Make(0, SkIntToScalar(80)) };
        SkScalar pos[2] = { 0, SkIntToScalar(80) };
        paint.setShader(SkGradientShader::MakeLinear(
            pts, fColors, pos, 2, SkShader::kClamp_TileMode));
        const char* str = "e";
        canvas.drawString(str, SkIntToScalar(-10), SkIntToScalar(80), paint);

        // ... tag the data as sRGB, so color-aware devices do gamut adjustment, etc...
        fBitmap.setInfo(SkImageInfo::MakeS32(80, 80, kPremul_SkAlphaType));
        fBitmap.setPixelRef(sk_ref_sp(n32Bitmap.pixelRef()), 0, 0);
    }

    SkISize onISize() override {
        return SkISize::Make(500, 300);
    }

    void draw(SkCanvas* canvas, int x, int y, const SkIPoint& kernelOffset,
              SkMatrixConvolutionImageFilter::TileMode tileMode, bool convolveAlpha,
              const SkImageFilter::CropRect* cropRect = nullptr) {
        SkScalar kernel[9] = {
            SkIntToScalar( 1), SkIntToScalar( 1), SkIntToScalar( 1),
            SkIntToScalar( 1), SkIntToScalar(-7), SkIntToScalar( 1),
            SkIntToScalar( 1), SkIntToScalar( 1), SkIntToScalar( 1),
        };
        SkISize kernelSize = SkISize::Make(3, 3);
        SkScalar gain = 0.3f, bias = SkIntToScalar(100);
        if (canvas->imageInfo().colorSpace()) {
            // TODO: Gain and bias are poorly specified (in the feConvolveMatrix SVG documentation,
            // there is obviously no mention of gamma or color spaces). Eventually, we need to
            // decide what to do with these (they generally have an extreme brightening effect).
            // For now, I'm modifying this GM to use values tuned to preserve luminance across the
            // range of input values (compared to the legacy math and values).
            //
            // It's impossible to match the results exactly, because legacy math produces a flat
            // response (when looking at sRGB encoded results), while gamma-correct math produces
            // a curve.
            gain = 0.25f;
            bias = 16.5f;
        }
        SkPaint paint;
        paint.setImageFilter(SkMatrixConvolutionImageFilter::Make(kernelSize,
                                                                  kernel,
                                                                  gain,
                                                                  bias,
                                                                  kernelOffset,
                                                                  tileMode,
                                                                  convolveAlpha,
                                                                  nullptr,
                                                                  cropRect));
        canvas->save();
        canvas->translate(SkIntToScalar(x), SkIntToScalar(y));
        canvas->clipRect(SkRect::MakeWH(SkIntToScalar(fBitmap.width()),
                                        SkIntToScalar(fBitmap.height())));
        canvas->drawBitmap(fBitmap, 0, 0, &paint);
        canvas->restore();
    }

    typedef SkMatrixConvolutionImageFilter MCIF;

    void onOnceBeforeDraw() override {
        this->makeBitmap();
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->clear(SK_ColorBLACK);
        SkIPoint kernelOffset = SkIPoint::Make(1, 0);
        SkImageFilter::CropRect rect(SkRect::Make(fBitmap.bounds()));
        for (int x = 10; x < 310; x += 100) {
            this->draw(canvas, x, 10, kernelOffset, MCIF::kClamp_TileMode, true, &rect);
            this->draw(canvas, x, 110, kernelOffset, MCIF::kClampToBlack_TileMode, true, &rect);
            this->draw(canvas, x, 210, kernelOffset, MCIF::kRepeat_TileMode, true, &rect);
            kernelOffset.fY++;
        }
        kernelOffset.fY = 1;
        SkImageFilter::CropRect smallRect(SkRect::MakeXYWH(10, 5, 60, 60));
        this->draw(canvas, 310, 10, kernelOffset, MCIF::kClamp_TileMode, true, &smallRect);
        this->draw(canvas, 310, 110, kernelOffset, MCIF::kClampToBlack_TileMode, true, &smallRect);
        this->draw(canvas, 310, 210, kernelOffset, MCIF::kRepeat_TileMode, true, &smallRect);

        this->draw(canvas, 410, 10, kernelOffset, MCIF::kClamp_TileMode, false, &rect);
        this->draw(canvas, 410, 110, kernelOffset, MCIF::kClampToBlack_TileMode, false, &rect);
        this->draw(canvas, 410, 210, kernelOffset, MCIF::kRepeat_TileMode, false, &rect);
    }

private:
    SkBitmap fBitmap;
    SkColor fColors[2];
    const char* fNameSuffix;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new MatrixConvolutionGM(0xFFFFFFFF, 0x40404040, "");)
DEF_GM(return new MatrixConvolutionGM(0xFFFF0000, 0xFF00FF00, "_color");)

}
