/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkSurface.h"

static sk_sp<SkSurface> make_surface(SkCanvas* root, int N, int padLeft, int padTop,
                                     int padRight, int padBottom) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(N + padLeft + padRight, N + padTop + padBottom);
    auto surface = root->makeSurface(info);
    if (!surface) {
        surface = SkSurface::MakeRaster(info);
    }

    return surface;
}

static sk_sp<SkImage> make_image(SkCanvas* root, int* xDivs, int* yDivs, int padLeft, int padTop,
                                 int padRight, int padBottom) {
    const int kCap = 28;
    const int kMid = 8;
    const int kSize = 2*kCap + 3*kMid;

    auto surface(make_surface(root, kSize, padLeft, padTop, padRight, padBottom));
    SkCanvas* canvas = surface->getCanvas();
    canvas->translate((float) padLeft, (float) padTop);

    SkRect r = SkRect::MakeWH(SkIntToScalar(kSize), SkIntToScalar(kSize));
    const SkScalar strokeWidth = SkIntToScalar(6);
    const SkScalar radius = SkIntToScalar(kCap) - strokeWidth/2;

    xDivs[0] = kCap + padLeft;
    yDivs[0] = kCap + padTop;
    xDivs[1] = kCap + kMid + padLeft;
    yDivs[1] = kCap + kMid + padTop;
    xDivs[2] = kCap + 2 * kMid + padLeft;
    yDivs[2] = kCap + 2 * kMid + padTop;
    xDivs[3] = kCap + 3 * kMid + padLeft;
    yDivs[3] = kCap + 3 * kMid + padTop;

    SkPaint paint;
    paint.setAntiAlias(true);

    paint.setColor(0xFFFFFF00);
    canvas->drawRoundRect(r, radius, radius, paint);

    r.setXYWH(SkIntToScalar(kCap), 0, SkIntToScalar(kMid), SkIntToScalar(kSize));
    paint.setColor(0x8800FF00);
    canvas->drawRect(r, paint);
    r.setXYWH(SkIntToScalar(kCap + kMid), 0, SkIntToScalar(kMid), SkIntToScalar(kSize));
    paint.setColor(0x880000FF);
    canvas->drawRect(r, paint);
    r.setXYWH(SkIntToScalar(kCap + 2*kMid), 0, SkIntToScalar(kMid), SkIntToScalar(kSize));
    paint.setColor(0x88FF00FF);
    canvas->drawRect(r, paint);

    r.setXYWH(0, SkIntToScalar(kCap), SkIntToScalar(kSize), SkIntToScalar(kMid));
    paint.setColor(0x8800FF00);
    canvas->drawRect(r, paint);
    r.setXYWH(0, SkIntToScalar(kCap + kMid), SkIntToScalar(kSize), SkIntToScalar(kMid));
    paint.setColor(0x880000FF);
    canvas->drawRect(r, paint);
    r.setXYWH(0, SkIntToScalar(kCap + 2*kMid), SkIntToScalar(kSize), SkIntToScalar(kMid));
    paint.setColor(0x88FF00FF);
    canvas->drawRect(r, paint);

    return surface->makeImageSnapshot();
}

static void image_to_bitmap(const SkImage* image, SkBitmap* bm) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(image->width(), image->height());
    bm->allocPixels(info);
    image->readPixels(info, bm->getPixels(), bm->rowBytes(), 0, 0);
}

/**
 *  This is similar to NinePatchStretchGM, but it also tests "ninepatch" images with more
 *  than nine patches.
 */
class LatticeGM : public skiagm::GM {
public:
    LatticeGM() {}

protected:
    SkString onShortName() override {
        return SkString("lattice");
    }

    SkISize onISize() override {
        return SkISize::Make(800, 800);
    }

    void onDrawHelper(SkCanvas* canvas, int padLeft, int padTop, int padRight, int padBottom) {
        canvas->save();

        int xDivs[5];
        int yDivs[5];
        xDivs[0] = padLeft;
        yDivs[0] = padTop;

        SkBitmap bitmap;
        sk_sp<SkImage> image = make_image(canvas, xDivs + 1, yDivs + 1, padLeft, padTop,
                                          padRight, padBottom);
        image_to_bitmap(image.get(), &bitmap);

        const SkSize size[] = {
            {  50,  50, }, // shrink in both axes
            {  50, 200, }, // shrink in X
            { 200,  50, }, // shrink in Y
            { 200, 200, },
        };

        canvas->drawImage(image, 10, 10, nullptr);

        SkScalar x = SkIntToScalar(100);
        SkScalar y = SkIntToScalar(100);

        SkCanvas::Lattice lattice;
        lattice.fXCount = 4;
        lattice.fXDivs = xDivs + 1;
        lattice.fYCount = 4;
        lattice.fYDivs = yDivs + 1;
        lattice.fFlags = nullptr;

        SkIRect bounds = SkIRect::MakeLTRB(padLeft, padTop,
                                           image->width() - padRight, image->height() - padBottom);
        lattice.fBounds = (bounds == SkIRect::MakeWH(image->width(), image->height())) ?
                nullptr : &bounds;

        for (int iy = 0; iy < 2; ++iy) {
            for (int ix = 0; ix < 2; ++ix) {
                int i = ix * 2 + iy;
                SkRect r = SkRect::MakeXYWH(x + ix * 60, y + iy * 60,
                                            size[i].width(), size[i].height());
                canvas->drawBitmapLattice(bitmap, lattice, r);
            }
        }

        // Include the degenerate first div.  While normally the first patch is "scalable",
        // this will mean that the first non-degenerate patch is "fixed".
        lattice.fXCount = 5;
        lattice.fXDivs = xDivs;
        lattice.fYCount = 5;
        lattice.fYDivs = yDivs;

        // Let's skip a few rects.
        SkCanvas::Lattice::Flags flags[36];
        sk_bzero(flags, 36 * sizeof(SkCanvas::Lattice::Flags));
        flags[4] = SkCanvas::Lattice::kTransparent_Flags;
        flags[9] = SkCanvas::Lattice::kTransparent_Flags;
        flags[12] = SkCanvas::Lattice::kTransparent_Flags;
        flags[19] = SkCanvas::Lattice::kTransparent_Flags;
        lattice.fFlags = flags;

        canvas->translate(400, 0);
        for (int iy = 0; iy < 2; ++iy) {
            for (int ix = 0; ix < 2; ++ix) {
                int i = ix * 2 + iy;
                SkRect r = SkRect::MakeXYWH(x + ix * 60, y + iy * 60,
                                            size[i].width(), size[i].height());
                canvas->drawImageLattice(image.get(), lattice, r);
            }
        }

        canvas->restore();
    }

    void onDraw(SkCanvas* canvas) override {
        this->onDrawHelper(canvas, 0, 0, 0, 0);
        canvas->translate(0.0f, 400.0f);
        this->onDrawHelper(canvas, 3, 7, 4, 11);
    }

private:
    typedef skiagm::GM INHERITED;
};
DEF_GM( return new LatticeGM; )
