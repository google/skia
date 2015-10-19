/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkBlurImageFilter.h"
#include "SkRSXform.h"
#include "SkSurface.h"

static void make_bm(SkBitmap* bm) {
    bm->allocN32Pixels(100, 100);
    bm->eraseColor(SK_ColorBLUE);

    SkCanvas canvas(*bm);
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(SK_ColorRED);
    canvas.drawCircle(50, 50, 50, paint);
}

static void draw_2_bitmaps(SkCanvas* canvas, const SkBitmap& bm, bool doClip,
                           int dx, int dy, SkImageFilter* filter = nullptr) {
    SkAutoCanvasRestore acr(canvas, true);
    SkPaint paint;

    SkRect clipR = SkRect::MakeXYWH(SkIntToScalar(dx),
                                    SkIntToScalar(dy),
                                    SkIntToScalar(bm.width()),
                                    SkIntToScalar(bm.height()));

    paint.setImageFilter(filter);
    clipR.inset(5, 5);

    if (doClip) {
        canvas->save();
        canvas->clipRect(clipR);
    }
    canvas->drawSprite(bm, dx, dy, &paint);
    if (doClip) {
        canvas->restore();
    }

    canvas->translate(SkIntToScalar(bm.width() + 20), 0);

    if (doClip) {
        canvas->save();
        canvas->clipRect(clipR);
    }
    canvas->drawBitmap(bm, SkIntToScalar(dx), SkIntToScalar(dy), &paint);
    if (doClip) {
        canvas->restore();
    }
}

/**
 *  Compare output of drawSprite and drawBitmap (esp. clipping and imagefilters)
 */
class SpriteBitmapGM : public skiagm::GM {
public:
    SpriteBitmapGM() {}

protected:

    SkString onShortName() override {
        return SkString("spritebitmap");
    }

    SkISize onISize() override {
        return SkISize::Make(640, 480);
    }

    void onDraw(SkCanvas* canvas) override {
        SkBitmap bm;
        make_bm(&bm);

        int dx = 10;
        int dy = 10;

        SkScalar sigma = 8;
        SkAutoTUnref<SkImageFilter> filter(SkBlurImageFilter::Create(sigma, sigma));

        draw_2_bitmaps(canvas, bm, false, dx, dy);
        dy += bm.height() + 20;
        draw_2_bitmaps(canvas, bm, false, dx, dy, filter);
        dy += bm.height() + 20;
        draw_2_bitmaps(canvas, bm, true, dx, dy);
        dy += bm.height() + 20;
        draw_2_bitmaps(canvas, bm, true, dx, dy, filter);
    }

private:
    typedef GM INHERITED;
};
DEF_GM( return new SpriteBitmapGM; )

///////////////////////////////////////////////////////////////////////////////////////////////////

#include "SkColorFilterImageFilter.h"
#include "SkModeColorFilter.h"
#include "SkMorphologyImageFilter.h"
#include "SkOffsetImageFilter.h"

static SkImage* make_image(SkCanvas* rootCanvas) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(100, 100);
    SkAutoTUnref<SkSurface> surface(rootCanvas->newSurface(info));
    if (!surface) {
        surface.reset(SkSurface::NewRaster(info));
    }

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(SK_ColorRED);
    surface->getCanvas()->drawCircle(50, 50, 50, paint);
    return surface->newImageSnapshot();
}

static void show_image(SkCanvas* canvas, SkImage* image, const SkIPoint& offset) {
    SkScalar x = SkIntToScalar(offset.x());
    SkScalar y = SkIntToScalar(offset.y());

    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);

    SkRect r = SkRect::MakeIWH(image->width(), image->height());
    r.offset(x, y);
    // get on pixel-centers to make the hairline land on a numerical stable boundary
    r.outset(SK_ScalarHalf, SK_ScalarHalf);
    canvas->drawRect(r, paint);

    canvas->drawImage(image, x, y, nullptr);
}

typedef SkImageFilter* (*ImageFilterFactory)();

// +[]{...} did not work on windows (VS)
// (ImageFilterFactory)[]{...} did not work on linux (gcc)
// hence this cast function
template <typename T> ImageFilterFactory IFCCast(T arg) { return arg; }

// We expect that applying the filter will keep us in the same domain (raster or gpu)
static void check_same_domain(SkImage* a, SkImage* b) {
    SkASSERT(a->isTextureBacked() == b->isTextureBacked());
}

/**
 *  Compare output of drawSprite and drawBitmap (esp. clipping and imagefilters)
 */
class ApplyFilterGM : public skiagm::GM {
public:
    ApplyFilterGM() {}

protected:
    SkString onShortName() override {
        return SkString("apply-filter");
    }

    SkISize onISize() override {
        return SkISize::Make(780, 780);
    }

    void onDraw(SkCanvas* canvas) override {
        SkAutoTUnref<SkImage> image0(make_image(canvas));

        const ImageFilterFactory factories[] = {
            IFCCast([]{ return SkBlurImageFilter::Create(8, 8); }),
            IFCCast([]{ SkAutoTUnref<SkColorFilter> cf(SkModeColorFilter::Create(SK_ColorBLUE,
                                                                     SkXfermode::kSrcIn_Mode));
                 return SkColorFilterImageFilter::Create(cf);
            }),
            IFCCast([]{ return SkDilateImageFilter::Create(8, 8); }),
            IFCCast([]{ return SkErodeImageFilter::Create(8, 8); }),
            IFCCast([]{ return SkOffsetImageFilter::Create(8, 8); }),
        };

        const SkScalar spacer = image0->width() * 3.0f / 2;

        for (auto&& factory : factories) {
            SkAutoTUnref<SkImageFilter> filter(factory());

            SkIPoint offset1, offset2;
            SkAutoTUnref<SkImage> image1(image0->applyFilter(filter, &offset1, true));
            SkAutoTUnref<SkImage> image2(image0->applyFilter(filter, &offset2, false));

            check_same_domain(image0, image1);
            check_same_domain(image0, image2);

            canvas->save();
            canvas->translate(30, 30);
            show_image(canvas, image0, SkIPoint::Make(0, 0));   // original
            canvas->translate(spacer, 0);
            show_image(canvas, image1, offset1);                // snug
            canvas->translate(spacer, 0);
            show_image(canvas, image2, offset2);                // not snug

            // Try drawing the original w/ the filter, to see that it "draws" the same as
            // when we have manually applied the filter (above).
            {
                SkPaint paint;
                paint.setImageFilter(filter);

                SkBitmap bm;
                image0->asLegacyBitmap(&bm, SkImage::kRO_LegacyBitmapMode);
                SkPoint loc = { 0, 0 };
                canvas->translate(spacer, 0);
                canvas->getTotalMatrix().mapPoints(&loc, 1);
                canvas->drawSprite(bm, (int)loc.x(), (int)loc.y(), &paint); // like snug

                canvas->translate(spacer, 0);
                canvas->drawImage(image0, 0, 0, &paint);        // like not snug
            }
            canvas->restore();

            canvas->translate(0, spacer);
        }
    }

private:
    typedef GM INHERITED;
};
DEF_GM( return new ApplyFilterGM; )

