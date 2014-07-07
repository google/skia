/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkBitmapDevice.h"
#include "SkBitmapSource.h"
#include "SkColor.h"
#include "SkMatrixImageFilter.h"
#include "SkRefCnt.h"

namespace skiagm {

class ResizeGM : public GM {
public:
    ResizeGM() {
        this->setBGColor(0x00000000);
    }

protected:
    virtual SkString onShortName() {
        return SkString("resizeimagefilter");
    }

    void draw(SkCanvas* canvas,
              const SkRect& rect,
              const SkSize& deviceSize,
              SkPaint::FilterLevel filterLevel,
              SkImageFilter* input = NULL) {
        SkRect dstRect;
        canvas->getTotalMatrix().mapRect(&dstRect, rect);
        canvas->save();
        SkScalar deviceScaleX = SkScalarDiv(deviceSize.width(), dstRect.width());
        SkScalar deviceScaleY = SkScalarDiv(deviceSize.height(), dstRect.height());
        canvas->translate(rect.x(), rect.y());
        canvas->scale(deviceScaleX, deviceScaleY);
        canvas->translate(-rect.x(), -rect.y());
        SkMatrix matrix;
        matrix.setScale(SkScalarInvert(deviceScaleX),
                        SkScalarInvert(deviceScaleY));
        SkAutoTUnref<SkImageFilter> imageFilter(
            SkMatrixImageFilter::Create(matrix, filterLevel, input));
        SkPaint filteredPaint;
        filteredPaint.setImageFilter(imageFilter.get());
        canvas->saveLayer(&rect, &filteredPaint);
        SkPaint paint;
        paint.setColor(0xFF00FF00);
        SkRect ovalRect = rect;
        ovalRect.inset(SkIntToScalar(4), SkIntToScalar(4));
        canvas->drawOval(ovalRect, paint);
        canvas->restore(); // for saveLayer
        canvas->restore();
    }

    virtual SkISize onISize() {
        return SkISize::Make(520, 100);
    }

    virtual void onDraw(SkCanvas* canvas) {
        canvas->clear(0x00000000);

        SkRect srcRect = SkRect::MakeWH(96, 96);

        SkSize deviceSize = SkSize::Make(16, 16);
        draw(canvas,
             srcRect,
             deviceSize,
             SkPaint::kNone_FilterLevel);

        canvas->translate(srcRect.width() + SkIntToScalar(10), 0);
        draw(canvas,
             srcRect,
             deviceSize,
             SkPaint::kLow_FilterLevel);

        canvas->translate(srcRect.width() + SkIntToScalar(10), 0);
        draw(canvas,
             srcRect,
             deviceSize,
             SkPaint::kMedium_FilterLevel);

        canvas->translate(srcRect.width() + SkIntToScalar(10), 0);
        draw(canvas,
             srcRect,
             deviceSize,
             SkPaint::kHigh_FilterLevel);

        SkBitmap bitmap;
        bitmap.allocN32Pixels(16, 16);
        bitmap.eraseARGB(0x00, 0x00, 0x00, 0x00);
        {
            SkBitmapDevice bitmapDevice(bitmap);
            SkCanvas bitmapCanvas(&bitmapDevice);
            SkPaint paint;
            paint.setColor(0xFF00FF00);
            SkRect ovalRect = SkRect::MakeWH(16, 16);
            ovalRect.inset(SkScalarDiv(2.0f, 3.0f), SkScalarDiv(2.0f, 3.0f));
            bitmapCanvas.drawOval(ovalRect, paint);
        }
        SkRect inRect = SkRect::MakeXYWH(-4, -4, 20, 20);
        SkRect outRect = SkRect::MakeXYWH(-24, -24, 120, 120);
        SkAutoTUnref<SkBitmapSource> source(SkBitmapSource::Create(bitmap, inRect, outRect));
        canvas->translate(srcRect.width() + SkIntToScalar(10), 0);
        draw(canvas,
             srcRect,
             deviceSize,
             SkPaint::kHigh_FilterLevel,
             source.get());
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new ResizeGM; }
static GMRegistry reg(MyFactory);

}
