/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkColor.h"
#include "SkResizeImageFilter.h"

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
              SkPaint::FilterLevel filterLevel) {
        SkRect dstRect;
        canvas->getTotalMatrix().mapRect(&dstRect, rect);
        canvas->save();
        SkScalar deviceScaleX = SkScalarDiv(deviceSize.width(), dstRect.width());
        SkScalar deviceScaleY = SkScalarDiv(deviceSize.height(), dstRect.height());
        canvas->translate(rect.x(), rect.y());
        canvas->scale(deviceScaleX, deviceScaleY);
        canvas->translate(-rect.x(), -rect.y());
        SkAutoTUnref<SkImageFilter> imageFilter(
            SkResizeImageFilter::Create(SkScalarInvert(deviceScaleX),
                                        SkScalarInvert(deviceScaleY),
                                        filterLevel));
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
        return make_isize(420, 100);
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
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new ResizeGM; }
static GMRegistry reg(MyFactory);

}
