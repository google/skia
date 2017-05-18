/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkSurface.h"

static sk_sp<SkSurface> make_surface(SkCanvas* root, int N) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(N, N);
    auto surface = root->makeSurface(info);
    if (!surface) {
        surface = SkSurface::MakeRaster(info);
    }
    return surface;
}

static sk_sp<SkImage> make_image(SkCanvas* root, SkIRect* center) {
    const int kFixed = 28;
    const int kStretchy = 8;
    const int kSize = 2*kFixed + kStretchy;

    auto surface(make_surface(root, kSize));
    SkCanvas* canvas = surface->getCanvas();

    SkRect r = SkRect::MakeWH(SkIntToScalar(kSize), SkIntToScalar(kSize));
    const SkScalar strokeWidth = SkIntToScalar(6);
    const SkScalar radius = SkIntToScalar(kFixed) - strokeWidth/2;

    center->setXYWH(kFixed, kFixed, kStretchy, kStretchy);

    SkPaint paint;
    paint.setAntiAlias(true);

    paint.setColor(0xFFFF0000);
    canvas->drawRoundRect(r, radius, radius, paint);
    r.setXYWH(SkIntToScalar(kFixed), 0, SkIntToScalar(kStretchy), SkIntToScalar(kSize));
    paint.setColor(0x8800FF00);
    canvas->drawRect(r, paint);
    r.setXYWH(0, SkIntToScalar(kFixed), SkIntToScalar(kSize), SkIntToScalar(kStretchy));
    paint.setColor(0x880000FF);
    canvas->drawRect(r, paint);

    return surface->makeImageSnapshot();
}

static void image_to_bitmap(const SkImage* image, SkBitmap* bm) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(image->width(), image->height());
    bm->allocPixels(info);
    image->readPixels(info, bm->getPixels(), bm->rowBytes(), 0, 0);
}

class NinePatchStretchGM : public skiagm::GM {
public:
    sk_sp<SkImage>  fImage;
    SkBitmap        fBitmap;
    SkIRect         fCenter;

    NinePatchStretchGM() {}

protected:
    SkString onShortName() override {
        return SkString("ninepatch-stretch");
    }

    SkISize onISize() override {
        return SkISize::Make(760, 800);
    }

    void onDraw(SkCanvas* canvas) override {
        if (nullptr == fBitmap.pixelRef()) {
            fImage = make_image(canvas, &fCenter);
            image_to_bitmap(fImage.get(), &fBitmap);
        }

        // amount of bm that should not be stretched (unless we have to)
        const SkScalar fixed = SkIntToScalar(fBitmap.width() - fCenter.width());

        const SkSize size[] = {
            { fixed * 4 / 5, fixed * 4 / 5 },   // shrink in both axes
            { fixed * 4 / 5, fixed * 4 },       // shrink in X
            { fixed * 4,     fixed * 4 / 5 },   // shrink in Y
            { fixed * 4,     fixed * 4 }
        };

        canvas->drawBitmap(fBitmap, 10, 10, nullptr);

        SkScalar x = SkIntToScalar(100);
        SkScalar y = SkIntToScalar(100);

        SkPaint paint;
        for (int filter = 0; filter < 2; filter++) {
            paint.setFilterQuality(filter == 0 ? kLow_SkFilterQuality : kNone_SkFilterQuality);
            canvas->translate(0, filter * SkIntToScalar(400));
            for (int iy = 0; iy < 2; ++iy) {
                for (int ix = 0; ix < 2; ++ix) {
                    int i = ix * 2 + iy;
                    SkRect r = SkRect::MakeXYWH(x + ix * fixed, y + iy * fixed,
                                                size[i].width(), size[i].height());
                    canvas->drawBitmapNine(fBitmap, fCenter, r, &paint);
                    canvas->drawImageNine(fImage.get(), fCenter, r.makeOffset(360, 0), &paint);
                }
            }
        }
    }

private:
    typedef skiagm::GM INHERITED;
};
DEF_GM( return new NinePatchStretchGM; )
