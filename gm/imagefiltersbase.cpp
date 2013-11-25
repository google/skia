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
#include "SkDropShadowImageFilter.h"
#include "SkTestImageFilters.h"

class FailImageFilter : public SkImageFilter {
public:
    FailImageFilter() : INHERITED(0) {}

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(FailImageFilter)
protected:
    virtual bool onFilterImage(Proxy*, const SkBitmap& src, const SkMatrix&,
                               SkBitmap* result, SkIPoint* offset) {
        return false;
    }

    FailImageFilter(SkFlattenableReadBuffer& buffer)
      : INHERITED(1, buffer) {}

private:
    typedef SkImageFilter INHERITED;
};

// register the filter with the flattenable registry
static SkFlattenable::Registrar gFailImageFilterReg("FailImageFilter",
                                                    FailImageFilter::CreateProc,
                                                    FailImageFilter::GetFlattenableType());

class IdentityImageFilter : public SkImageFilter {
public:
    IdentityImageFilter() : INHERITED(0) {}

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(IdentityImageFilter)
protected:
    virtual bool onFilterImage(Proxy*, const SkBitmap& src, const SkMatrix&,
                               SkBitmap* result, SkIPoint* offset) {
        *result = src;
        return true;
    }

    IdentityImageFilter(SkFlattenableReadBuffer& buffer)
      : INHERITED(1, buffer) {}

private:
    typedef SkImageFilter INHERITED;
};

// register the filter with the flattenable registry
static SkFlattenable::Registrar gIdentityImageFilterReg("IdentityImageFilter",
                                                        IdentityImageFilter::CreateProc,
                                                        IdentityImageFilter::GetFlattenableType());


///////////////////////////////////////////////////////////////////////////////

static void draw_paint(SkCanvas* canvas, const SkRect& r, SkImageFilter* imf) {
    SkPaint paint;
    paint.setImageFilter(imf);
    paint.setColor(SK_ColorGREEN);
    canvas->save();
    canvas->clipRect(r);
    canvas->drawPaint(paint);
    canvas->restore();
}

static void draw_line(SkCanvas* canvas, const SkRect& r, SkImageFilter* imf) {
    SkPaint paint;
    paint.setColor(SK_ColorBLUE);
    paint.setImageFilter(imf);
    paint.setStrokeWidth(r.width()/10);
    canvas->drawLine(r.fLeft, r.fTop, r.fRight, r.fBottom, paint);
}

static void draw_rect(SkCanvas* canvas, const SkRect& r, SkImageFilter* imf) {
    SkPaint paint;
    paint.setColor(SK_ColorYELLOW);
    paint.setImageFilter(imf);
    SkRect rr(r);
    rr.inset(r.width()/10, r.height()/10);
    canvas->drawRect(rr, paint);
}

static void draw_path(SkCanvas* canvas, const SkRect& r, SkImageFilter* imf) {
    SkPaint paint;
    paint.setColor(SK_ColorMAGENTA);
    paint.setImageFilter(imf);
    paint.setAntiAlias(true);
    canvas->drawCircle(r.centerX(), r.centerY(), r.width()*2/5, paint);
}

static void draw_text(SkCanvas* canvas, const SkRect& r, SkImageFilter* imf) {
    SkPaint paint;
    paint.setImageFilter(imf);
    paint.setColor(SK_ColorCYAN);
    paint.setAntiAlias(true);
    paint.setTextSize(r.height()/2);
    paint.setTextAlign(SkPaint::kCenter_Align);
    canvas->drawText("Text", 4, r.centerX(), r.centerY(), paint);
}

static void draw_bitmap(SkCanvas* canvas, const SkRect& r, SkImageFilter* imf) {
    SkPaint paint;
    paint.setImageFilter(imf);

    SkIRect bounds;
    r.roundOut(&bounds);

    SkBitmap bm;
    bm.setConfig(SkBitmap::kARGB_8888_Config, bounds.width(), bounds.height());
    bm.allocPixels();
    bm.eraseColor(SK_ColorTRANSPARENT);
    SkCanvas c(bm);
    draw_path(&c, r, NULL);

    canvas->drawBitmap(bm, 0, 0, &paint);
}

static void draw_sprite(SkCanvas* canvas, const SkRect& r, SkImageFilter* imf) {
    SkPaint paint;
    paint.setImageFilter(imf);

    SkIRect bounds;
    r.roundOut(&bounds);

    SkBitmap bm;
    bm.setConfig(SkBitmap::kARGB_8888_Config, bounds.width(), bounds.height());
    bm.allocPixels();
    bm.eraseColor(SK_ColorTRANSPARENT);
    SkCanvas c(bm);
    draw_path(&c, r, NULL);

    SkPoint loc = { r.fLeft, r.fTop };
    canvas->getTotalMatrix().mapPoints(&loc, 1);
    canvas->drawSprite(bm,
                       SkScalarRoundToInt(loc.fX), SkScalarRoundToInt(loc.fY),
                       &paint);
}

///////////////////////////////////////////////////////////////////////////////

class ImageFiltersBaseGM : public skiagm::GM {
public:
    ImageFiltersBaseGM () {}

protected:

    virtual SkString onShortName() {
        return SkString("imagefiltersbase");
    }

    virtual SkISize onISize() { return SkISize::Make(700, 500); }

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
            draw_paint,
            draw_line, draw_rect, draw_path, draw_text,
            draw_bitmap,
            draw_sprite
        };

        SkColorFilter* cf = SkColorFilter::CreateModeFilter(SK_ColorRED,
                                                     SkXfermode::kSrcIn_Mode);
        SkImageFilter* filters[] = {
            NULL,
            new IdentityImageFilter,
            new FailImageFilter,
            SkColorFilterImageFilter::Create(cf),
            new SkBlurImageFilter(12.0f, 0.0f),
            new SkDropShadowImageFilter(10.0f, 5.0f, 3.0f, SK_ColorBLUE),
        };
        cf->unref();

        SkRect r = SkRect::MakeWH(SkIntToScalar(64), SkIntToScalar(64));
        SkScalar MARGIN = SkIntToScalar(16);
        SkScalar DX = r.width() + MARGIN;
        SkScalar DY = r.height() + MARGIN;

        canvas->translate(MARGIN, MARGIN);
        for (size_t i = 0; i < SK_ARRAY_COUNT(drawProc); ++i) {
            canvas->save();
            for (size_t j = 0; j < SK_ARRAY_COUNT(filters); ++j) {
                drawProc[i](canvas, r, filters[j]);

                draw_frame(canvas, r);
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

static skiagm::GM* MyFactory(void*) { return new ImageFiltersBaseGM; }
static skiagm::GMRegistry reg(MyFactory);
