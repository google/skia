/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkColorFilter.h"

#include "SkColorFilterImageFilter.h"
#include "SkDropShadowImageFilter.h"

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
    paint.setColor(SK_ColorGREEN);
    paint.setImageFilter(imf);
    paint.setAntiAlias(true);
    canvas->save();
    canvas->clipRect(r);
    canvas->drawCircle(r.centerX(), r.centerY(), r.width()/3, paint);
    canvas->restore();
}

static void draw_text(SkCanvas* canvas, const SkRect& r, SkImageFilter* imf) {
    SkPaint paint;
    paint.setImageFilter(imf);
    paint.setColor(SK_ColorGREEN);
    paint.setAntiAlias(true);
    sk_tool_utils::set_portable_typeface(&paint);
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
    bm.allocN32Pixels(bounds.width(), bounds.height());
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
    bm.allocN32Pixels(bounds.width(), bounds.height());
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

class DropShadowImageFilterGM : public skiagm::GM {
public:
    DropShadowImageFilterGM () {}

protected:

    virtual SkString onShortName() {
        return SkString("dropshadowimagefilter");
    }

    virtual SkISize onISize() { return SkISize::Make(400, 656); }

    void draw_frame(SkCanvas* canvas, const SkRect& r) {
        SkPaint paint;
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setColor(SK_ColorRED);
        canvas->drawRect(r, paint);
    }

    virtual void onDraw(SkCanvas* canvas) {
        void (*drawProc[])(SkCanvas*, const SkRect&, SkImageFilter*) = {
            draw_sprite, draw_bitmap, draw_path, draw_paint, draw_text
        };

        SkAutoTUnref<SkColorFilter> cf(
            SkColorFilter::CreateModeFilter(SK_ColorMAGENTA, SkXfermode::kSrcIn_Mode));
        SkAutoTUnref<SkImageFilter> cfif(SkColorFilterImageFilter::Create(cf.get()));
        SkImageFilter::CropRect cropRect(SkRect::Make(SkIRect::MakeXYWH(10, 10, 44, 44)),
                                         SkImageFilter::CropRect::kHasAll_CropEdge);
        SkImageFilter::CropRect bogusRect(SkRect::Make(SkIRect::MakeXYWH(-100, -100, 10, 10)),
                                          SkImageFilter::CropRect::kHasAll_CropEdge);

        SkImageFilter* filters[] = {
            NULL,
            SkDropShadowImageFilter::Create(7.0f, 0.0f, 0.0f, 3.0f, SK_ColorBLUE,
                SkDropShadowImageFilter::kDrawShadowAndForeground_ShadowMode),
            SkDropShadowImageFilter::Create(0.0f, 7.0f, 3.0f, 0.0f, SK_ColorBLUE,
                SkDropShadowImageFilter::kDrawShadowAndForeground_ShadowMode),
            SkDropShadowImageFilter::Create(7.0f, 7.0f, 3.0f, 3.0f, SK_ColorBLUE,
                SkDropShadowImageFilter::kDrawShadowAndForeground_ShadowMode),
            SkDropShadowImageFilter::Create(7.0f, 7.0f, 3.0f, 3.0f, SK_ColorBLUE,
                SkDropShadowImageFilter::kDrawShadowAndForeground_ShadowMode, cfif, NULL),
            SkDropShadowImageFilter::Create(7.0f, 7.0f, 3.0f, 3.0f, SK_ColorBLUE,
                SkDropShadowImageFilter::kDrawShadowAndForeground_ShadowMode, NULL, &cropRect),
            SkDropShadowImageFilter::Create(7.0f, 7.0f, 3.0f, 3.0f, SK_ColorBLUE,
                SkDropShadowImageFilter::kDrawShadowAndForeground_ShadowMode, NULL, &bogusRect),
            SkDropShadowImageFilter::Create(7.0f, 7.0f, 3.0f, 3.0f, SK_ColorBLUE,
                SkDropShadowImageFilter::kDrawShadowOnly_ShadowMode),
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

static skiagm::GM* MyFactory(void*) { return new DropShadowImageFilterGM; }
static skiagm::GMRegistry reg(MyFactory);
