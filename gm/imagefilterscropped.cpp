/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkColorFilter.h"
#include "SkColorPriv.h"
#include "SkShader.h"

#include "SkBlurImageFilter.h"
#include "SkColorFilterImageFilter.h"
#include "SkMergeImageFilter.h"
#include "SkOffsetImageFilter.h"
#include "SkTestImageFilters.h"

///////////////////////////////////////////////////////////////////////////////

static void draw_paint(SkCanvas* canvas, const SkRect& r, SkImageFilter* imf) {
    SkPaint paint;
    paint.setImageFilter(imf);
    paint.setColor(SK_ColorBLACK);
    canvas->save();
    canvas->clipRect(r);
    canvas->drawPaint(paint);
    canvas->restore();
}

static void draw_path(SkCanvas* canvas, const SkRect& r, SkImageFilter* imf) {
    SkPaint paint;
    paint.setColor(SK_ColorMAGENTA);
    paint.setImageFilter(imf);
    paint.setAntiAlias(true);
    canvas->save();
    canvas->clipRect(r);
    canvas->drawCircle(r.centerX(), r.centerY(), r.width()*2/5, paint);
    canvas->restore();
}

static void draw_text(SkCanvas* canvas, const SkRect& r, SkImageFilter* imf) {
    SkPaint paint;
    paint.setImageFilter(imf);
    paint.setColor(SK_ColorGREEN);
    paint.setAntiAlias(true);
    paint.setTextSize(r.height()/2);
    paint.setTextAlign(SkPaint::kCenter_Align);
    canvas->save();
    canvas->clipRect(r);
    canvas->drawText("Text", 4, r.centerX(), r.centerY(), paint);
    canvas->restore();
}

static void draw_bitmap(SkCanvas* canvas, const SkRect& r, SkImageFilter* imf) {
    SkPaint paint;

    SkIRect bounds;
    r.roundOut(&bounds);

    SkBitmap bm;
    bm.setConfig(SkBitmap::kARGB_8888_Config, bounds.width(), bounds.height());
    bm.allocPixels();
    bm.eraseColor(SK_ColorTRANSPARENT);
    SkCanvas c(bm);
    draw_path(&c, r, NULL);

    paint.setImageFilter(imf);
    canvas->save();
    canvas->clipRect(r);
    canvas->drawBitmap(bm, 0, 0, &paint);
    canvas->restore();
}

static void draw_sprite(SkCanvas* canvas, const SkRect& r, SkImageFilter* imf) {
    SkPaint paint;

    SkIRect bounds;
    r.roundOut(&bounds);

    SkBitmap bm;
    bm.setConfig(SkBitmap::kARGB_8888_Config, bounds.width(), bounds.height());
    bm.allocPixels();
    bm.eraseColor(SK_ColorRED);
    SkCanvas c(bm);

    SkIRect cropRect = SkIRect::MakeXYWH(10, 10, 44, 44);
    paint.setColor(SK_ColorGREEN);
    c.drawRect(SkRect::Make(cropRect), paint);

    paint.setImageFilter(imf);
    SkPoint loc = { r.fLeft, r.fTop };
    canvas->getTotalMatrix().mapPoints(&loc, 1);
    canvas->drawSprite(bm,
                       SkScalarRoundToInt(loc.fX), SkScalarRoundToInt(loc.fY),
                       &paint);
}

///////////////////////////////////////////////////////////////////////////////

class ImageFiltersCroppedGM : public skiagm::GM {
public:
    ImageFiltersCroppedGM () {}

protected:

    virtual SkString onShortName() {
        return SkString("imagefilterscropped");
    }

    virtual SkISize onISize() { return SkISize::Make(400, 640); }

    void draw_frame(SkCanvas* canvas, const SkRect& r) {
        SkPaint paint;
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setColor(SK_ColorRED);
        canvas->drawRect(r, paint);
    }

    virtual uint32_t onGetFlags() const {
        // Because of the use of drawSprite, this test is excluded
        // from scaled replay tests because drawSprite ignores the
        // reciprocal scale that is applied at record time, which is
        // the intended behavior of drawSprite.
        return kSkipScaledReplay_Flag;
    }

    virtual void onDraw(SkCanvas* canvas) {
        void (*drawProc[])(SkCanvas*, const SkRect&, SkImageFilter*) = {
            draw_sprite, draw_bitmap, draw_path, draw_paint, draw_text
        };

        SkAutoTUnref<SkColorFilter> cf(
            SkColorFilter::CreateModeFilter(SK_ColorBLUE, SkXfermode::kSrcIn_Mode));
        SkImageFilter::CropRect cropRect(SkRect::Make(SkIRect::MakeXYWH(10, 10, 44, 44)), SkImageFilter::CropRect::kHasAll_CropEdge);
        SkImageFilter::CropRect bogusRect(SkRect::Make(SkIRect::MakeXYWH(-100, -100, 10, 10)), SkImageFilter::CropRect::kHasAll_CropEdge);

        SkAutoTUnref<SkImageFilter> offset(new SkOffsetImageFilter(
            SkIntToScalar(-10), SkIntToScalar(-10)));

        SkAutoTUnref<SkImageFilter> cfOffset(SkColorFilterImageFilter::Create(cf.get(), offset.get()));

        SkImageFilter* filters[] = {
            NULL,
            SkColorFilterImageFilter::Create(cf.get(), NULL, &cropRect),
            new SkBlurImageFilter(1.0f, 1.0f, NULL, &cropRect),
            new SkBlurImageFilter(8.0f, 0.0f, NULL, &cropRect),
            new SkBlurImageFilter(0.0f, 8.0f, NULL, &cropRect),
            new SkBlurImageFilter(8.0f, 8.0f, NULL, &cropRect),
            new SkMergeImageFilter(NULL, cfOffset.get(), SkXfermode::kSrcOver_Mode, &cropRect),
            new SkBlurImageFilter(8.0f, 8.0f, NULL, &bogusRect),
            SkColorFilterImageFilter::Create(cf.get(), NULL, &bogusRect),
        };

        SkRect r = SkRect::MakeWH(SkIntToScalar(64), SkIntToScalar(64));
        SkScalar MARGIN = SkIntToScalar(16);
        SkScalar DX = r.width() + MARGIN;
        SkScalar DY = r.height() + MARGIN;

        canvas->translate(MARGIN, MARGIN);
        for (size_t j = 0; j < SK_ARRAY_COUNT(drawProc); ++j) {
            canvas->save();
            for (size_t i = 0; i < SK_ARRAY_COUNT(filters); ++i) {
                drawProc[j](canvas, r, filters[i]);
                canvas->translate(0, DY);
            }
            canvas->restore();
            canvas->translate(DX, 0);
        }

        for(size_t j = 0; j < SK_ARRAY_COUNT(filters); ++j) {
            SkSafeUnref(filters[j]);
        }
    }

private:
    typedef GM INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

static skiagm::GM* MyFactory(void*) { return new ImageFiltersCroppedGM; }
static skiagm::GMRegistry reg(MyFactory);
