/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypeface.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkImageFilters.h"
#include "src/gpu/effects/GrMatrixConvolutionEffect.h"
#include "tools/ToolUtils.h"

#include <vector>

namespace skiagm {

enum KernelFixture {
    kBasic_KernelFixture,
    kLarge_KernelFixture
};

class MatrixConvolutionGM : public GM {
public:
    MatrixConvolutionGM(SkColor colorOne, SkColor colorTwo, KernelFixture kernelFixture, const char* nameSuffix)
            : fNameSuffix(nameSuffix),
              fKernelFixture(kernelFixture) {
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
        auto surf = SkSurface::MakeRasterN32Premul(80, 80);
        SkPaint paint;
        paint.setColor(0xFFFFFFFF);
        SkPoint pts[2] = { {0, 0},
                           {0, 80.0f} };
        SkScalar pos[2] = { 0, 80.0f };
        paint.setShader(SkGradientShader::MakeLinear(
            pts, fColors, pos, 2, SkTileMode::kClamp));
        SkFont font(ToolUtils::create_portable_typeface(), 180.0f);
        surf->getCanvas()->drawString("e", -10.0f, 80.0f, font, paint);
        fImage = surf->makeImageSnapshot();
    }

    SkISize onISize() override {
        return SkISize::Make(500, 300);
    }

    sk_sp<SkImageFilter> makeFilter(const SkIPoint &kernelOffset, SkTileMode tileMode,
                                    bool convolveAlpha, const SkIRect *cropRect = nullptr) {
        switch (fKernelFixture) {
            case kBasic_KernelFixture: {
                // All 1s except center value, which is -7 (sum of 1).
                std::vector<SkScalar> kernel(9, SkIntToScalar(1));
                kernel[4] = SkIntToScalar(-7);
                return SkImageFilters::MatrixConvolution({3,3}, kernel.data(), /* gain */ 0.3f, /* bias */ SkIntToScalar(100), kernelOffset, tileMode, convolveAlpha, nullptr, cropRect);
            }
            case kLarge_KernelFixture: {
                static_assert(49 > GrMatrixConvolutionEffect::kMaxUniformSize);
                // All 1s except center value, which is -47 (sum of 1).
                std::vector<SkScalar> kernel(49, SkIntToScalar(1));
                kernel[24] = SkIntToScalar(-47);
                return SkImageFilters::MatrixConvolution({7,7}, kernel.data(), /* gain */ 0.3f, /* bias */ SkIntToScalar(100), kernelOffset, tileMode, convolveAlpha, nullptr, cropRect);
            }
            default:
                return nullptr;
        }
    }

    void draw(SkCanvas* canvas, int x, int y, const SkIPoint& kernelOffset,
              SkTileMode tileMode, bool convolveAlpha,
              const SkIRect* cropRect = nullptr) {
        SkPaint paint;
        paint.setImageFilter(this->makeFilter(kernelOffset, tileMode, convolveAlpha, cropRect));
        canvas->save();
        canvas->translate(SkIntToScalar(x), SkIntToScalar(y));
        const SkRect layerBounds = SkRect::Make(fImage->bounds());
        canvas->clipRect(layerBounds);
        // This GM is, in part, intended to display the wrapping behavior of the
        // matrix image filter. The only (rational) way to achieve that for repeat mode
        // is to create a tight layer.
        canvas->saveLayer(layerBounds, &paint);
            canvas->drawImage(fImage, 0, 0);
        canvas->restore();
        canvas->restore();
    }

    void onOnceBeforeDraw() override {
        this->makeBitmap();
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->clear(SK_ColorBLACK);
        SkIPoint kernelOffset = SkIPoint::Make(1, 0);
        SkIRect rect = fImage->bounds();
        for (int x = 10; x < 310; x += 100) {
            this->draw(canvas, x, 10, kernelOffset, SkTileMode::kClamp, true, &rect);
            this->draw(canvas, x, 110, kernelOffset, SkTileMode::kDecal, true, &rect);
            this->draw(canvas, x, 210, kernelOffset, SkTileMode::kRepeat, true, &rect);
            kernelOffset.fY++;
        }
        kernelOffset.fY = 1;
        SkIRect smallRect = SkIRect::MakeXYWH(10, 5, 60, 60);
        this->draw(canvas, 310, 10, kernelOffset, SkTileMode::kClamp, true, &smallRect);
        this->draw(canvas, 310, 110, kernelOffset, SkTileMode::kDecal, true, &smallRect);
        this->draw(canvas, 310, 210, kernelOffset, SkTileMode::kRepeat, true, &smallRect);

        this->draw(canvas, 410, 10, kernelOffset, SkTileMode::kClamp, false, &rect);
        this->draw(canvas, 410, 110, kernelOffset, SkTileMode::kDecal, false, &rect);
        this->draw(canvas, 410, 210, kernelOffset, SkTileMode::kRepeat, false, &rect);
    }

private:
    sk_sp<SkImage> fImage;
    SkColor fColors[2];
    const char* fNameSuffix;
    KernelFixture fKernelFixture;

    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new MatrixConvolutionGM(0xFFFFFFFF, 0x40404040, KernelFixture::kBasic_KernelFixture, "");)
DEF_GM(return new MatrixConvolutionGM(0xFFFF0000, 0xFF00FF00, KernelFixture::kBasic_KernelFixture, "_color");)
DEF_GM(return new MatrixConvolutionGM(0xFFFFFFFF, 0x40404040, KernelFixture::kLarge_KernelFixture, "_big");)
DEF_GM(return new MatrixConvolutionGM(0xFFFF0000, 0xFF00FF00, KernelFixture::kLarge_KernelFixture, "_big_color");)

}  // namespace skiagm
