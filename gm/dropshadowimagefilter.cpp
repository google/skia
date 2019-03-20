/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorFilter.h"
#include "SkColorFilterImageFilter.h"
#include "SkDropShadowImageFilter.h"
#include "SkTextUtils.h"
#include "ToolUtils.h"
#include "gm.h"

///////////////////////////////////////////////////////////////////////////////

static void draw_paint(SkCanvas* canvas, const SkRect& r, sk_sp<SkImageFilter> imf) {
    SkPaint paint;
    paint.setImageFilter(std::move(imf));
    paint.setColor(SK_ColorBLACK);
    canvas->save();
    canvas->clipRect(r);
    canvas->drawPaint(paint);
    canvas->restore();
}

static void draw_path(SkCanvas* canvas, const SkRect& r, sk_sp<SkImageFilter> imf) {
    SkPaint paint;
    paint.setColor(SK_ColorGREEN);
    paint.setImageFilter(std::move(imf));
    paint.setAntiAlias(true);
    canvas->save();
    canvas->clipRect(r);
    canvas->drawCircle(r.centerX(), r.centerY(), r.width()/3, paint);
    canvas->restore();
}

static void draw_text(SkCanvas* canvas, const SkRect& r, sk_sp<SkImageFilter> imf) {
    SkPaint paint;
    paint.setImageFilter(std::move(imf));
    paint.setColor(SK_ColorGREEN);
    paint.setAntiAlias(true);

    SkFont font(ToolUtils::create_portable_typeface(), r.height() / 2);
    canvas->save();
    canvas->clipRect(r);
    SkTextUtils::DrawString(canvas, "Text", r.centerX(), r.centerY(), font, paint, SkTextUtils::kCenter_Align);
    canvas->restore();
}

static void draw_bitmap(SkCanvas* canvas, const SkRect& r, sk_sp<SkImageFilter> imf) {
    SkPaint paint;

    SkIRect bounds;
    r.roundOut(&bounds);

    SkBitmap bm;
    bm.allocN32Pixels(bounds.width(), bounds.height());
    bm.eraseColor(SK_ColorTRANSPARENT);
    SkCanvas c(bm);
    draw_path(&c, r, nullptr);

    paint.setImageFilter(std::move(imf));
    canvas->save();
    canvas->clipRect(r);
    canvas->drawBitmap(bm, 0, 0, &paint);
    canvas->restore();
}

///////////////////////////////////////////////////////////////////////////////

DEF_SIMPLE_GM(dropshadowimagefilter, canvas, 400, 656) {
    void (*drawProc[])(SkCanvas*, const SkRect&, sk_sp<SkImageFilter>) = {
        draw_bitmap, draw_path, draw_paint, draw_text
    };

    sk_sp<SkColorFilter> cf(SkColorFilter::MakeModeFilter(SK_ColorMAGENTA,
                                                          SkBlendMode::kSrcIn));
    sk_sp<SkImageFilter> cfif(SkColorFilterImageFilter::Make(std::move(cf), nullptr));
    SkImageFilter::CropRect cropRect(SkRect::Make(SkIRect::MakeXYWH(10, 10, 44, 44)),
                                     SkImageFilter::CropRect::kHasAll_CropEdge);
    SkImageFilter::CropRect bogusRect(SkRect::Make(SkIRect::MakeXYWH(-100, -100, 10, 10)),
                                      SkImageFilter::CropRect::kHasAll_CropEdge);

    sk_sp<SkImageFilter> filters[] = {
        nullptr,
        SkDropShadowImageFilter::Make(7.0f, 0.0f, 0.0f, 3.0f, SK_ColorBLUE,
            SkDropShadowImageFilter::kDrawShadowAndForeground_ShadowMode, nullptr),
        SkDropShadowImageFilter::Make(0.0f, 7.0f, 3.0f, 0.0f, SK_ColorBLUE,
            SkDropShadowImageFilter::kDrawShadowAndForeground_ShadowMode, nullptr),
        SkDropShadowImageFilter::Make(7.0f, 7.0f, 3.0f, 3.0f, SK_ColorBLUE,
            SkDropShadowImageFilter::kDrawShadowAndForeground_ShadowMode, nullptr),
        SkDropShadowImageFilter::Make(7.0f, 7.0f, 3.0f, 3.0f, SK_ColorBLUE,
            SkDropShadowImageFilter::kDrawShadowAndForeground_ShadowMode, std::move(cfif)),
        SkDropShadowImageFilter::Make(7.0f, 7.0f, 3.0f, 3.0f, SK_ColorBLUE,
            SkDropShadowImageFilter::kDrawShadowAndForeground_ShadowMode, nullptr, &cropRect),
        SkDropShadowImageFilter::Make(7.0f, 7.0f, 3.0f, 3.0f, SK_ColorBLUE,
            SkDropShadowImageFilter::kDrawShadowAndForeground_ShadowMode, nullptr, &bogusRect),
        SkDropShadowImageFilter::Make(7.0f, 7.0f, 3.0f, 3.0f, SK_ColorBLUE,
            SkDropShadowImageFilter::kDrawShadowOnly_ShadowMode, nullptr),
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
}
