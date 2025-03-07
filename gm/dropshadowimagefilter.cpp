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
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkImageFilters.h"
#include "include/utils/SkTextUtils.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"

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

    SkFont font(ToolUtils::DefaultPortableTypeface(), r.height() / 2);
    canvas->save();
    canvas->clipRect(r);
    SkTextUtils::DrawString(canvas, "Text", r.centerX(), r.centerY(), font, paint, SkTextUtils::kCenter_Align);
    canvas->restore();
}

static void draw_bitmap(SkCanvas* canvas, const SkRect& r, sk_sp<SkImageFilter> imf) {
    SkPaint paint;

    SkIRect bounds;
    r.roundOut(&bounds);

    auto surf = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(bounds.width(), bounds.height()));
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

    sk_sp<SkColorSpace> spinCS = SkColorSpace::MakeSRGB()->makeColorSpin();
    sk_sp<SkImageFilter> filters[] = {
            nullptr,
            SkImageFilters::DropShadow(7.0f, 0.0f, 0.0f, 3.0f, SK_ColorBLUE, nullptr),
            SkImageFilters::DropShadow(0.0f, 7.0f, 3.0f, 0.0f, SK_ColorBLUE, nullptr),
            SkImageFilters::DropShadow(7.0f, 7.0f, 3.0f, 3.0f, SK_ColorBLUE, nullptr),
            SkImageFilters::DropShadow(7.0f, 7.0f, 3.0f, 3.0f, SK_ColorBLUE, std::move(cfif)),
            SkImageFilters::DropShadow(
                    7.0f, 7.0f, 3.0f, 3.0f, SkColors::kGreen, spinCS, nullptr, &cropRect),
            SkImageFilters::DropShadow(7.0f, 7.0f, 3.0f, 3.0f, SK_ColorBLUE, nullptr, &bogusRect),
            SkImageFilters::DropShadowOnly(7.0f, 7.0f, 3.0f, 3.0f, SK_ColorBLUE, nullptr),
    };

    SkRect r = SkRect::MakeWH(SkIntToScalar(64), SkIntToScalar(64));
    SkScalar MARGIN = SkIntToScalar(16);
    SkScalar DX = r.width() + MARGIN;
    SkScalar DY = r.height() + MARGIN;

    canvas->translate(MARGIN, MARGIN);
    for (size_t j = 0; j < std::size(drawProc); ++j) {
        canvas->save();
        for (size_t i = 0; i < std::size(filters); ++i) {
            drawProc[j](canvas, r, filters[i]);
            canvas->translate(0, DY);
        }
        canvas->restore();
        canvas->translate(DX, 0);
    }
}

DEF_SIMPLE_GM(dropshadow_pseudopersp, canvas, 155, 155) {
    canvas->clear(SK_ColorLTGRAY);
    canvas->concat(SkM44{0.5f, 0.f,  0.f, -75.f,
                         0.f,  0.5f, 0.f, -30.f,
                         0.f,  0.f,  1.f,  0.f,
                         0.f,  0.f,  0.f,  1.f});
    // This 4x4 matrtix technically has perspective, but it only impacts the Z values and the
    // projection doesn't appear to have any distortion. However, the projected coordinates have
    // Z values very different from 0. When inversing the device bounds with an assumed Z=0, the
    // layer bounds end up empty. This GM ensures layer mapping calculations don't discard it.
    canvas->concat(SkM44{1360.f, 0.f,     275.4f,  294100.f,
                         0.f,    1360.f,  489.6f,  98344.f,
                         0.f,    0.f,    -0.51f,  -2180.67f,
                         0.f,    0.f,     0.51f,   2181.67f});

    SkRect layerBounds{42.5f, 42.5f, 457.5f, 457.5f};
    SkPaint layerPaint;
    layerPaint.setImageFilter(SkImageFilters::DropShadow(
            /*dx=*/30.f, /*dy=*/30.f,
            /*sigmaX=*/12.f, /*sigmaY=*/12.f,
            /*color=*/{0.14902f, 0.215686f, 0.329412f, 0.666667f},
            /*colorSpace=*/nullptr,
            /*input=*/nullptr));
    canvas->saveLayer(&layerBounds, &layerPaint);

    SkRRect rrect = SkRRect::MakeRectXY({-250.f, -250.f, 250.f, 250.f}, 45.f, 45.f);
    SkPaint rrectPaint;
    rrectPaint.setColor4f(SkColors::kWhite);
    rrectPaint.setAntiAlias(true);

    canvas->concat(SkM44{0.83f, 0.f,   0.f, 250.f,
                         0.f,   0.83f, 0.f, 250.f,
                         0.f,   0.f,   1.f, 0.f,
                         0.f,   0.f,   0.f, 1.f});
    canvas->drawRRect(rrect, rrectPaint);
    canvas->restore();

    canvas->concat(SkM44{0.83f, 0.f,   0.f, 250.f,
                         0.f,   0.83f, 0.f, 250.f,
                         0.f,   0.f,   1.f, 0.f,
                         0.f,   0.f,   0.f, 1.f});

    rrectPaint.setColor4f(SkColors::kBlack);
    rrectPaint.setStyle(SkPaint::kStroke_Style);
    canvas->drawRRect(rrect, rrectPaint);
}
