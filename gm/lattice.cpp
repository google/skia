/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSurface.h"
#include "ToolUtils.h"
#include "gm.h"

static sk_sp<SkSurface> make_surface(SkCanvas* root, int N, int padLeft, int padTop,
                                     int padRight, int padBottom) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(N + padLeft + padRight, N + padTop + padBottom);
    return ToolUtils::makeSurface(root, info);
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
        lattice.fRectTypes = nullptr;
        lattice.fColors = nullptr;

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

        // Provide hints about 3 solid color rects. These colors match
        // what was already in the bitmap.
        int fixedColorX[3] = {2, 4, 1};
        int fixedColorY[3] = {1, 1, 2};
        SkColor fixedColor[3] = {SK_ColorBLACK, SK_ColorBLACK, SK_ColorBLACK};
        const SkImageInfo info = SkImageInfo::Make(1, 1, kBGRA_8888_SkColorType,
                                                   kUnpremul_SkAlphaType);
        for (int rectNum = 0; rectNum < 3; rectNum++) {
            int srcX = xDivs[fixedColorX[rectNum]-1];
            int srcY = yDivs[fixedColorY[rectNum]-1];
            image->readPixels(info, &fixedColor[rectNum], 4, srcX, srcY);
        }

        // Include the degenerate first div.  While normally the first patch is "scalable",
        // this will mean that the first non-degenerate patch is "fixed".
        lattice.fXCount = 5;
        lattice.fXDivs = xDivs;
        lattice.fYCount = 5;
        lattice.fYDivs = yDivs;

        // Let's skip a few rects.
        SkCanvas::Lattice::RectType flags[36];
        sk_bzero(flags, 36 * sizeof(SkCanvas::Lattice::RectType));
        flags[4] = SkCanvas::Lattice::kTransparent;
        flags[9] = SkCanvas::Lattice::kTransparent;
        flags[12] = SkCanvas::Lattice::kTransparent;
        flags[19] = SkCanvas::Lattice::kTransparent;
        for (int rectNum = 0; rectNum < 3; rectNum++) {
            flags[fixedColorY[rectNum]*6 + fixedColorX[rectNum]]
                   = SkCanvas::Lattice::kFixedColor;
        }
        lattice.fRectTypes = flags;

        SkColor colors[36];
        sk_bzero(colors, 36 * sizeof(SkColor));
        for (int rectNum = 0; rectNum < 3; rectNum++) {
            colors[fixedColorY[rectNum]*6 + fixedColorX[rectNum]]
                   = fixedColor[rectNum];
        }

        lattice.fColors = colors;

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


// LatticeGM2 exercises code paths that draw fixed color and 1x1 rectangles.
class LatticeGM2 : public skiagm::GM {
public:
    LatticeGM2() {}
    SkString onShortName() override {
        return SkString("lattice2");
    }

    SkISize onISize() override {
        return SkISize::Make(800, 800);
    }

    sk_sp<SkImage> makeImage(SkCanvas* root, int padLeft, int padTop, int padRight, int padBottom) {
        const int kSize = 80;
        auto surface(make_surface(root, kSize, padLeft, padTop, padRight, padBottom));
        SkCanvas* canvas = surface->getCanvas();
        SkPaint paint;
        paint.setAntiAlias(false);
        SkRect r;

        //first line
        r.setXYWH(0, 0, 4, 1);  //4x1 green rect
        paint.setColor(0xFF00FF00);
        canvas->drawRect(r, paint);

        r.setXYWH(4, 0, 1, 1); //1x1 blue pixel -> draws as rectangle
        paint.setColor(0xFF0000FF);
        canvas->drawRect(r, paint);

        r.setXYWH(5, 0, kSize-5, 1); //the rest of the line is red
        paint.setColor(0xFFFF0000);
        canvas->drawRect(r, paint);


        //second line -> draws as fixed color rectangles
        r.setXYWH(0, 1, 4, 1);  //4x1 red rect
        paint.setColor(0xFFFF0000);
        canvas->drawRect(r, paint);

        r.setXYWH(4, 1, 1, 1); //1x1 blue pixel with alpha
        paint.setColor(0x880000FF);
        canvas->drawRect(r, paint);

        r.setXYWH(5, 1, kSize-5, 1); //the rest of the line is green
        paint.setColor(0xFF00FF00);
        canvas->drawRect(r, paint);


        //third line - does not draw, because it is transparent
        r.setXYWH(0, 2, 4, kSize-2);  //4x78 green rect
        paint.setColor(0xFF00FF00);
        canvas->drawRect(r, paint);

        r.setXYWH(4, 2, 1, kSize-2); //1x78 red pixel with alpha
        paint.setColor(0x88FF0000);
        canvas->drawRect(r, paint);

        r.setXYWH(5, 2, kSize-5, kSize-2); //the rest of the image is blue
        paint.setColor(0xFF0000FF);
        canvas->drawRect(r, paint);

        return surface->makeImageSnapshot();
    }

    void onDrawHelper(SkCanvas* canvas, int padLeft, int padTop, int padRight, int padBottom,
                      SkPaint& paint) {
        int xDivs[2] = {4, 5};
        int yDivs[2] = {1, 2};

        canvas->save();

        sk_sp<SkImage> image = makeImage(canvas, padLeft, padTop, padRight, padBottom);

        canvas->drawImage(image, 10, 10, nullptr);

        SkCanvas::Lattice lattice;
        lattice.fXCount = 2;
        lattice.fXDivs = xDivs;
        lattice.fYCount = 2;
        lattice.fYDivs = yDivs;
        lattice.fBounds = nullptr;

        SkCanvas::Lattice::RectType flags[9];
        sk_bzero(flags, 9 * sizeof(SkCanvas::Lattice::RectType));
        flags[3] = SkCanvas::Lattice::kFixedColor;
        flags[4] = SkCanvas::Lattice::kFixedColor;
        flags[5] = SkCanvas::Lattice::kFixedColor;

        flags[6] = SkCanvas::Lattice::kTransparent;
        flags[7] = SkCanvas::Lattice::kTransparent;
        flags[8] = SkCanvas::Lattice::kTransparent;
        lattice.fRectTypes = flags;

        SkColor colors[9] = {SK_ColorBLACK, SK_ColorBLACK, SK_ColorBLACK,
                             0xFFFF0000, 0x880000FF, 0xFF00FF00,
                             SK_ColorBLACK, SK_ColorBLACK, SK_ColorBLACK};
        lattice.fColors = colors;
        paint.setColor(0xFFFFFFFF);
        canvas->drawImageLattice(image.get(), lattice,
                                 SkRect::MakeXYWH(100, 100, 200, 200), &paint);

        //draw the same content with alpha
        canvas->translate(400, 0);
        paint.setColor(0x80000FFF);
        canvas->drawImageLattice(image.get(), lattice,
                                 SkRect::MakeXYWH(100, 100, 200, 200), &paint);

        canvas->restore();
    }

    void onDraw(SkCanvas* canvas) override {

        //draw a rectangle in the background with transparent pixels
        SkPaint paint;
        paint.setColor(0x7F123456);
        paint.setBlendMode(SkBlendMode::kSrc);
        canvas->drawRect( SkRect::MakeXYWH(300, 0, 300, 800), paint);

        //draw image lattice with kSrcOver blending
        paint.setBlendMode(SkBlendMode::kSrcOver);
        this->onDrawHelper(canvas, 0, 0, 0, 0, paint);

        //draw image lattice with kSrcATop blending
        canvas->translate(0.0f, 400.0f);
        paint.setBlendMode(SkBlendMode::kSrcATop);
        this->onDrawHelper(canvas, 0, 0, 0, 0, paint);
    }

private:
    typedef skiagm::GM INHERITED;
};
DEF_GM( return new LatticeGM2; )

// Code paths that incorporate the paint color when drawing the lattice (using an alpha image)
DEF_SIMPLE_GM_BG(lattice_alpha, canvas, 120, 120, SK_ColorWHITE) {
    auto surface = ToolUtils::makeSurface(canvas, SkImageInfo::MakeA8(100, 100));
    surface->getCanvas()->clear(0);
    surface->getCanvas()->drawCircle(50, 50, 50, SkPaint());
    auto image = surface->makeImageSnapshot();

    int divs[] = { 20, 40, 60, 80 };

    SkCanvas::Lattice lattice;
    lattice.fXCount = 4;
    lattice.fXDivs = divs;
    lattice.fYCount = 4;
    lattice.fYDivs = divs;
    lattice.fRectTypes = nullptr;
    lattice.fColors = nullptr;
    lattice.fBounds = nullptr;

    SkPaint paint;
    paint.setColor(SK_ColorMAGENTA);
    canvas->drawImageLattice(image.get(), lattice, SkRect::MakeWH(120, 120), &paint);
}
