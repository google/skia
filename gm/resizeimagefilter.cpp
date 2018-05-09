/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkColor.h"
#include "SkImageSource.h"
#include "SkRefCnt.h"
#include "SkSurface.h"

namespace skiagm {

class ResizeGM : public GM {
public:
    ResizeGM() {
        this->setBGColor(0x00000000);
    }

protected:
    SkString onShortName() override {
        return SkString("resizeimagefilter");
    }

    void draw(SkCanvas* canvas,
              const SkRect& rect,
              const SkSize& deviceSize,
              SkFilterQuality filterQuality,
              sk_sp<SkImageFilter> input) {
        SkRect dstRect;
        canvas->getTotalMatrix().mapRect(&dstRect, rect);
        canvas->save();
        SkScalar deviceScaleX = deviceSize.width() / dstRect.width();
        SkScalar deviceScaleY = deviceSize.height() / dstRect.height();
        canvas->translate(rect.x(), rect.y());
        canvas->scale(deviceScaleX, deviceScaleY);
        canvas->translate(-rect.x(), -rect.y());
        SkMatrix matrix;
        matrix.setScale(SkScalarInvert(deviceScaleX), SkScalarInvert(deviceScaleY));
        sk_sp<SkImageFilter> filter(SkImageFilter::MakeMatrixFilter(matrix,
                                                                    filterQuality,
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

    SkISize onISize() override {
        return SkISize::Make(520, 100);
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->clear(SK_ColorBLACK);

        SkRect srcRect = SkRect::MakeWH(96, 96);

        SkSize deviceSize = SkSize::Make(16, 16);
        this->draw(canvas,
                   srcRect,
                   deviceSize,
                   kNone_SkFilterQuality,
                   nullptr);

        canvas->translate(srcRect.width() + SkIntToScalar(10), 0);
        this->draw(canvas,
                   srcRect,
                   deviceSize,
                   kLow_SkFilterQuality,
                   nullptr);

        canvas->translate(srcRect.width() + SkIntToScalar(10), 0);
        this->draw(canvas,
                   srcRect,
                   deviceSize,
                   kMedium_SkFilterQuality,
                   nullptr);

        canvas->translate(srcRect.width() + SkIntToScalar(10), 0);
        this->draw(canvas,
                   srcRect,
                   deviceSize,
                   kHigh_SkFilterQuality,
                   nullptr);

        sk_sp<SkSurface> surface(SkSurface::MakeRasterN32Premul(16, 16));
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
            SkImageSource::Make(std::move(image), inRect, outRect, kHigh_SkFilterQuality));
        canvas->translate(srcRect.width() + SkIntToScalar(10), 0);
        this->draw(canvas,
                   srcRect,
                   deviceSize,
                   kHigh_SkFilterQuality,
                   std::move(source));
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ResizeGM; )

}
