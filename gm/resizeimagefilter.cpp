/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkImageFilters.h"

#include <utility>

namespace skiagm {

class ResizeGM : public GM {
public:
    ResizeGM() {
        this->setBGColor(0x00000000);
    }

protected:
    SkString getName() const override { return SkString("resizeimagefilter"); }

    void draw(SkCanvas* canvas,
              const SkRect& rect,
              const SkSize& deviceSize,
              const SkSamplingOptions& sampling,
              sk_sp<SkImageFilter> input) {
        SkRect dstRect;
        canvas->getLocalToDeviceAs3x3().mapRect(&dstRect, rect);
        canvas->save();
        SkScalar deviceScaleX = deviceSize.width() / dstRect.width();
        SkScalar deviceScaleY = deviceSize.height() / dstRect.height();
        canvas->translate(rect.x(), rect.y());
        canvas->scale(deviceScaleX, deviceScaleY);
        canvas->translate(-rect.x(), -rect.y());
        SkMatrix matrix;
        matrix.setScale(SkScalarInvert(deviceScaleX), SkScalarInvert(deviceScaleY));
        sk_sp<SkImageFilter> filter(SkImageFilters::MatrixTransform(matrix,
                                                                    sampling,
                                                                    std::move(input)));
        SkPaint filteredPaint;
        filteredPaint.setImageFilter(std::move(filter));
        canvas->saveLayer(&rect, &filteredPaint);
        SkPaint paint;
        paint.setColor(0xFF00FF00);
        SkRect ovalRect = rect;
        ovalRect.inset(SkIntToScalar(4), SkIntToScalar(4));
        canvas->drawOval(ovalRect, paint);
        canvas->restore(); // for saveLayer
        canvas->restore();
    }

    SkISize getISize() override { return SkISize::Make(630, 100); }

    void onDraw(SkCanvas* canvas) override {
        canvas->clear(SK_ColorBLACK);

        const SkSamplingOptions samplings[] = {
            SkSamplingOptions(),
            SkSamplingOptions(SkFilterMode::kLinear),
            SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kLinear),
            SkSamplingOptions(SkCubicResampler::Mitchell()),
            SkSamplingOptions::Aniso(16),
        };
        const SkRect srcRect = SkRect::MakeWH(96, 96);
        const SkSize deviceSize = SkSize::Make(16, 16);

        for (const auto& sampling : samplings) {
            this->draw(canvas, srcRect, deviceSize, sampling, nullptr);
            canvas->translate(srcRect.width() + SkIntToScalar(10), 0);
        }

        {
            sk_sp<SkSurface> surface(SkSurfaces::Raster(SkImageInfo::MakeN32Premul(16, 16)));
            SkCanvas* surfaceCanvas = surface->getCanvas();
            surfaceCanvas->clear(0x000000);
            {
                SkPaint paint;
                paint.setColor(0xFF00FF00);
                SkRect ovalRect = SkRect::MakeWH(16, 16);
                ovalRect.inset(SkIntToScalar(2)/3, SkIntToScalar(2)/3);
                surfaceCanvas->drawOval(ovalRect, paint);
            }
            sk_sp<SkImage> image(surface->makeImageSnapshot());
            SkRect inRect = SkRect::MakeXYWH(-4, -4, 20, 20);
            SkRect outRect = SkRect::MakeXYWH(-24, -24, 120, 120);
            sk_sp<SkImageFilter> source(
                SkImageFilters::Image(std::move(image), inRect, outRect,
                                      SkSamplingOptions({1/3.0f, 1/3.0f})));
            this->draw(canvas, srcRect, deviceSize, samplings[3], std::move(source));
        }
    }

private:
    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ResizeGM; )

}  // namespace skiagm
