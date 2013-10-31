/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#if SK_SUPPORT_GPU
#include "SkGpuDevice.h"
#else
class GrContext;
#endif

static void make_bitmap(SkBitmap* bitmap, GrContext* ctx, SkIRect* center) {
    SkBaseDevice* dev;

    const int kFixed = 28;
    const int kStretchy = 8;
    const int kSize = 2*kFixed + kStretchy;

#if SK_SUPPORT_GPU
    if (ctx) {
        dev = new SkGpuDevice(ctx, SkBitmap::kARGB_8888_Config, kSize, kSize);
        *bitmap = dev->accessBitmap(false);
    } else
#endif
    {
        bitmap->setConfig(SkBitmap::kARGB_8888_Config, kSize, kSize);
        bitmap->allocPixels();
        dev = new SkBitmapDevice(*bitmap);
    }

    SkCanvas canvas(dev);
    dev->unref();
    canvas.clear(SK_ColorTRANSPARENT);

    SkRect r = SkRect::MakeWH(SkIntToScalar(kSize), SkIntToScalar(kSize));
    const SkScalar strokeWidth = SkIntToScalar(6);
    const SkScalar radius = SkIntToScalar(kFixed) - strokeWidth/2;

    center->setXYWH(kFixed, kFixed, kStretchy, kStretchy);

    SkPaint paint;
    paint.setAntiAlias(true);

    paint.setColor(0xFFFF0000);
    canvas.drawRoundRect(r, radius, radius, paint);
    r.setXYWH(SkIntToScalar(kFixed), 0, SkIntToScalar(kStretchy), SkIntToScalar(kSize));
    paint.setColor(0x8800FF00);
    canvas.drawRect(r, paint);
    r.setXYWH(0, SkIntToScalar(kFixed), SkIntToScalar(kSize), SkIntToScalar(kStretchy));
    paint.setColor(0x880000FF);
    canvas.drawRect(r, paint);
}

namespace skiagm {

class NinePatchStretchGM : public GM {
public:
    SkBitmap fBM;

    NinePatchStretchGM() {}

protected:
    virtual SkString onShortName() {
        return SkString("ninepatch-stretch");
    }

    virtual SkISize onISize() {
        return make_isize(400, 400);
    }

    virtual void onDraw(SkCanvas* canvas) {
        SkBitmap bm;
        SkIRect center;
        make_bitmap(&bm, NULL /*SampleCode::GetGr()*/, &center);

        // amount of bm that should not be stretched (unless we have to)
        const SkScalar fixed = SkIntToScalar(bm.width() - center.width());

        const SkTSize<SkScalar> size[] = {
            { fixed * 4 / 5, fixed * 4 / 5 },   // shrink in both axes
            { fixed * 4 / 5, fixed * 4 },       // shrink in X
            { fixed * 4,     fixed * 4 / 5 },   // shrink in Y
            { fixed * 4,     fixed * 4 }
        };

        canvas->drawBitmap(bm, SkIntToScalar(10), SkIntToScalar(10), NULL);

        SkScalar x = SkIntToScalar(100);
        SkScalar y = SkIntToScalar(100);

        SkPaint paint;
        paint.setFilterLevel(SkPaint::kLow_FilterLevel);

        for (int iy = 0; iy < 2; ++iy) {
            for (int ix = 0; ix < 2; ++ix) {
                int i = ix * 2 + iy;
                SkRect r = SkRect::MakeXYWH(x + ix * fixed, y + iy * fixed,
                                            size[i].width(), size[i].height());
                canvas->drawBitmapNine(bm, center, r, &paint);
            }
        }
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new NinePatchStretchGM; }
static GMRegistry reg(MyFactory);

}
