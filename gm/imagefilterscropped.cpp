/*
 * Copyright 2011 Google Inc.
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
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
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
    paint.setColor(SK_ColorMAGENTA);
    paint.setImageFilter(std::move(imf));
    paint.setAntiAlias(true);
    canvas->drawCircle(r.centerX(), r.centerY(), r.width()*2/5, paint);
}

static void draw_text(SkCanvas* canvas, const SkRect& r, sk_sp<SkImageFilter> imf) {
    SkPaint paint;
    paint.setImageFilter(std::move(imf));
    paint.setColor(SK_ColorGREEN);

    SkFont font(ToolUtils::create_portable_typeface(), r.height() / 2);
    SkTextUtils::DrawString(canvas, "Text", r.centerX(), r.centerY(), font, paint, SkTextUtils::kCenter_Align);
}

static void draw_bitmap(SkCanvas* canvas, const SkRect& r, sk_sp<SkImageFilter> imf) {
    SkPaint paint;

    SkIRect bounds;
    r.roundOut(&bounds);

    auto surf = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(bounds.width(), bounds.height()));
    draw_path(surf->getCanvas(), r, nullptr);

    paint.setImageFilter(std::move(imf));
    surf->draw(canvas, 0, 0, SkSamplingOptions(), &paint);
}

///////////////////////////////////////////////////////////////////////////////

class ImageFiltersCroppedGM : public skiagm::GM {
public:
    ImageFiltersCroppedGM () {}

protected:
    SkString getName() const override { return SkString("imagefilterscropped"); }

    SkISize getISize() override { return SkISize::Make(400, 960); }

    void make_checkerboard() {
        auto surf = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(80, 80));
        auto canvas = surf->getCanvas();
        SkPaint darkPaint;
        darkPaint.setColor(0xFF404040);
        SkPaint lightPaint;
        lightPaint.setColor(0xFFA0A0A0);
        for (int y = 0; y < 80; y += 16) {
            for (int x = 0; x < 80; x += 16) {
                canvas->save();
                canvas->translate(SkIntToScalar(x), SkIntToScalar(y));
                canvas->drawRect(SkRect::MakeXYWH(0, 0, 8, 8), darkPaint);
                canvas->drawRect(SkRect::MakeXYWH(8, 0, 8, 8), lightPaint);
                canvas->drawRect(SkRect::MakeXYWH(0, 8, 8, 8), lightPaint);
                canvas->drawRect(SkRect::MakeXYWH(8, 8, 8, 8), darkPaint);
                canvas->restore();
            }
        }
        fCheckerboard = surf->makeImageSnapshot();
    }

    void draw_frame(SkCanvas* canvas, const SkRect& r) {
        SkPaint paint;
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setColor(SK_ColorRED);
        canvas->drawRect(r, paint);
    }

    void onOnceBeforeDraw() override {
        make_checkerboard();
    }

    void onDraw(SkCanvas* canvas) override {
        void (*drawProc[])(SkCanvas*, const SkRect&, sk_sp<SkImageFilter>) = {
            draw_bitmap, draw_path, draw_paint, draw_text
        };

        sk_sp<SkColorFilter> cf(SkColorFilters::Blend(SK_ColorBLUE,
                                                              SkBlendMode::kSrcIn));
        SkIRect cropRect = SkIRect::MakeXYWH(10, 10, 44, 44);
        SkIRect bogusRect = SkIRect::MakeXYWH(-100, -100, 10, 10);

        sk_sp<SkImageFilter> offset(SkImageFilters::Offset(-10, -10, nullptr));

        sk_sp<SkImageFilter> cfOffset(SkImageFilters::ColorFilter(cf, std::move(offset)));

        // These are composed with an outer erode along the other axis, so don't add a cropRect to
        // them or it will interfere with the second filter evaluation.
        sk_sp<SkImageFilter> erodeX(SkImageFilters::Erode(8, 0, nullptr));
        sk_sp<SkImageFilter> erodeY(SkImageFilters::Erode(0, 8, nullptr));

        sk_sp<SkImageFilter> filters[] = {
            nullptr,
            SkImageFilters::ColorFilter(cf, nullptr, &cropRect),
            SkImageFilters::Blur(0.0f, 0.0f, nullptr, &cropRect),
            SkImageFilters::Blur(1.0f, 1.0f, nullptr, &cropRect),
            SkImageFilters::Blur(8.0f, 0.0f, nullptr, &cropRect),
            SkImageFilters::Blur(0.0f, 8.0f, nullptr, &cropRect),
            SkImageFilters::Blur(8.0f, 8.0f, nullptr, &cropRect),
            SkImageFilters::Erode(1, 1, nullptr, &cropRect),
            SkImageFilters::Erode(8, 0, std::move(erodeY), &cropRect),
            SkImageFilters::Erode(0, 8, std::move(erodeX), &cropRect),
            SkImageFilters::Erode(8, 8, nullptr, &cropRect),
            SkImageFilters::Merge(nullptr, std::move(cfOffset), &cropRect),
            SkImageFilters::Blur(8.0f, 8.0f, nullptr, &bogusRect),
            SkImageFilters::ColorFilter(cf, nullptr, &bogusRect),
        };

        SkRect r = SkRect::MakeWH(SkIntToScalar(64), SkIntToScalar(64));
        SkScalar MARGIN = SkIntToScalar(16);
        SkScalar DX = r.width() + MARGIN;
        SkScalar DY = r.height() + MARGIN;

        canvas->translate(MARGIN, MARGIN);
        for (size_t j = 0; j < std::size(drawProc); ++j) {
            canvas->save();
            for (size_t i = 0; i < std::size(filters); ++i) {
                SkPaint paint;
                canvas->drawImage(fCheckerboard, 0, 0);
                drawProc[j](canvas, r, filters[i]);
                canvas->translate(0, DY);
            }
            canvas->restore();
            canvas->translate(DX, 0);
        }
    }

private:
    sk_sp<SkImage> fCheckerboard;
    using INHERITED = GM;
};

///////////////////////////////////////////////////////////////////////////////

DEF_GM( return new ImageFiltersCroppedGM; )
