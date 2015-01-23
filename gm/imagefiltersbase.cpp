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
    class Registrar {
    public:
        Registrar() {
            SkFlattenable::Register("FailImageFilter",
                                    FailImageFilter::CreateProc,
                                    FailImageFilter::GetFlattenableType());
        }
    };
    static FailImageFilter* Create() {
        return SkNEW(FailImageFilter);
    }

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(FailImageFilter)

protected:
    FailImageFilter() : INHERITED(0, NULL) {}

    virtual bool onFilterImage(Proxy*, const SkBitmap& src, const Context&,
                               SkBitmap* result, SkIPoint* offset) const SK_OVERRIDE {
        return false;
    }

private:
    typedef SkImageFilter INHERITED;
};

static FailImageFilter::Registrar gReg0;

SkFlattenable* FailImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 0);
    return FailImageFilter::Create();
}

#ifndef SK_IGNORE_TO_STRING
void FailImageFilter::toString(SkString* str) const {
    str->appendf("FailImageFilter: (");
    str->append(")");
}
#endif

class IdentityImageFilter : public SkImageFilter {
public:
    class Registrar {
    public:
        Registrar() {
            SkFlattenable::Register("IdentityImageFilter",
                                    IdentityImageFilter::CreateProc,
                                    IdentityImageFilter::GetFlattenableType());
        }
    };
    static IdentityImageFilter* Create(SkImageFilter* input = NULL) {
        return SkNEW_ARGS(IdentityImageFilter, (input));
    }

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(IdentityImageFilter)
protected:
    IdentityImageFilter(SkImageFilter* input) : INHERITED(1, &input) {}

    virtual bool onFilterImage(Proxy*, const SkBitmap& src, const Context&,
                               SkBitmap* result, SkIPoint* offset) const SK_OVERRIDE {
        *result = src;
        offset->set(0, 0);
        return true;
    }

private:
    typedef SkImageFilter INHERITED;
};

static IdentityImageFilter::Registrar gReg1;

SkFlattenable* IdentityImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);
    return IdentityImageFilter::Create(common.getInput(0));
}

#ifndef SK_IGNORE_TO_STRING
void IdentityImageFilter::toString(SkString* str) const {
    str->appendf("IdentityImageFilter: (");
    str->append(")");
}
#endif

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
    sk_tool_utils::set_portable_typeface(&paint);
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
    bm.allocN32Pixels(bounds.width(), bounds.height());
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
    bm.allocN32Pixels(bounds.width(), bounds.height());
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
            IdentityImageFilter::Create(),
            FailImageFilter::Create(),
            SkColorFilterImageFilter::Create(cf),
            SkBlurImageFilter::Create(12.0f, 0.0f),
            SkDropShadowImageFilter::Create(10.0f, 5.0f, 3.0f, 3.0f, SK_ColorBLUE,
                SkDropShadowImageFilter::kDrawShadowAndForeground_ShadowMode),
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
