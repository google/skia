/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkFont.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkImageFilters.h"
#include "include/utils/SkTextUtils.h"
#include "tools/ToolUtils.h"

#include <utility>

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

    auto surf = SkSurface::MakeRasterN32Premul(bounds.width(), bounds.height());
    draw_path(surf->getCanvas(), r, nullptr);

    paint.setImageFilter(std::move(imf));
    canvas->save();
    canvas->clipRect(r);
    surf->draw(canvas, 0, 0, SkSamplingOptions(), &paint);
    canvas->restore();
}

///////////////////////////////////////////////////////////////////////////////

DEF_SIMPLE_GM(dropshadowimagefilter, canvas, 400, 656) {
    void (*drawProc[])(SkCanvas*, const SkRect&, sk_sp<SkImageFilter>) = {
        draw_bitmap, draw_path, draw_paint, draw_text
    };

    sk_sp<SkColorFilter> cf(SkColorFilters::Blend(SK_ColorMAGENTA, SkBlendMode::kSrcIn));
    sk_sp<SkImageFilter> cfif(SkImageFilters::ColorFilter(std::move(cf), nullptr));
    SkIRect cropRect = SkIRect::MakeXYWH(10, 10, 44, 44);
    SkIRect bogusRect = SkIRect::MakeXYWH(-100, -100, 10, 10);

    sk_sp<SkImageFilter> filters[] = {
        nullptr,
        SkImageFilters::DropShadow(7.0f, 0.0f, 0.0f, 3.0f, SK_ColorBLUE, nullptr),
        SkImageFilters::DropShadow(0.0f, 7.0f, 3.0f, 0.0f, SK_ColorBLUE, nullptr),
        SkImageFilters::DropShadow(7.0f, 7.0f, 3.0f, 3.0f, SK_ColorBLUE, nullptr),
        SkImageFilters::DropShadow(7.0f, 7.0f, 3.0f, 3.0f, SK_ColorBLUE, std::move(cfif)),
        SkImageFilters::DropShadow(7.0f, 7.0f, 3.0f, 3.0f, SK_ColorBLUE, nullptr, &cropRect),
        SkImageFilters::DropShadow(7.0f, 7.0f, 3.0f, 3.0f, SK_ColorBLUE, nullptr, &bogusRect),
        SkImageFilters::DropShadowOnly(7.0f, 7.0f, 3.0f, 3.0f, SK_ColorBLUE, nullptr),
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
