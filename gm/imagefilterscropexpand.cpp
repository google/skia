/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkPoint3.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTileMode.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkImageFilters.h"

#include <utility>

namespace {

sk_sp<SkImage> make_checkerboard();
sk_sp<SkImage> make_gradient_circle(int width, int height);
void draw(SkCanvas*, sk_sp<SkImage>, const SkIRect&, sk_sp<SkImageFilter>);

}  // namespace

///////////////////////////////////////////////////////////////////////////////

DEF_SIMPLE_GM(imagefilterscropexpand, canvas, 730, 650) {
    SkIRect cropRect = SkIRect::MakeXYWH(10, 10, 44, 44);

    sk_sp<SkImage> gradientCircle(make_gradient_circle(64, 64));
    auto checkerboard = make_checkerboard();

    sk_sp<SkImageFilter> gradientCircleSource(SkImageFilters::Image(std::move(gradientCircle),
                                                                    SkFilterMode::kLinear));
    sk_sp<SkImageFilter> noopCropped(SkImageFilters::Offset(0, 0, nullptr, &cropRect));
    // This color matrix saturates the green component but only partly increases the opacity.
    // For the opaque checkerboard, the opacity boost doesn't matter but it does impact the
    // area outside the checkerboard.
    float matrix[20] = { 1, 0, 0, 0, 0,
                         0, 1, 0, 0, 1,
                         0, 0, 1, 0, 0,
                         0, 0, 0, 1, 32.0f/255 };
    sk_sp<SkColorFilter> cfAlphaTrans(SkColorFilters::Matrix(matrix));

    SkRect r = SkRect::MakeWH(SkIntToScalar(64), SkIntToScalar(64));
    SkScalar MARGIN = SkIntToScalar(12);

    SkPoint3 pointLocation = SkPoint3::Make(0, 0, SkIntToScalar(10));
    SkScalar kd = SkIntToScalar(2);
    SkScalar surfaceScale = SkIntToScalar(1);
    SkIRect bounds;
    r.roundOut(&bounds);

    SkPaint paint;
    canvas->translate(MARGIN, MARGIN);
    for (int outset = -15; outset <= 20; outset += 5) {
        canvas->save();
        SkIRect bigRect = cropRect;
        bigRect.outset(outset, outset);

        draw(canvas, checkerboard, bigRect,
             SkImageFilters::ColorFilter(cfAlphaTrans, noopCropped,  &bigRect));

        draw(canvas, checkerboard, bigRect,
             SkImageFilters::Blur(0.3f, 0.3f,  noopCropped, &bigRect));

        draw(canvas, checkerboard, bigRect,
             SkImageFilters::Blur(8.0f, 8.0f,  noopCropped, &bigRect));

        draw(canvas, checkerboard, bigRect,
             SkImageFilters::Dilate(2, 2, noopCropped, &bigRect));

        draw(canvas, checkerboard, bigRect,
             SkImageFilters::Erode(2, 2, noopCropped, &bigRect));

        draw(canvas, checkerboard, bigRect,
             SkImageFilters::DropShadow(10, 10, 3, 3, SK_ColorBLUE, noopCropped, &bigRect));

        draw(canvas, checkerboard, bigRect,
             SkImageFilters::DisplacementMap(SkColorChannel::kR, SkColorChannel::kR, 12,
                                             gradientCircleSource, noopCropped, &bigRect));

        draw(canvas, checkerboard, bigRect,
             SkImageFilters::Offset(SkIntToScalar(-8), SkIntToScalar(16), noopCropped, &bigRect));

        draw(canvas, checkerboard, bigRect,
             SkImageFilters::PointLitDiffuse(pointLocation, SK_ColorWHITE, surfaceScale, kd,
                                             noopCropped, &bigRect));

        canvas->restore();
        canvas->translate(0, SkIntToScalar(80));
    }
}

namespace {
    sk_sp<SkImage> make_checkerboard() {
        auto surf = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(64, 64));
        auto canvas = surf->getCanvas();
        canvas->clear(0xFFFF0000);
        SkPaint darkPaint;
        darkPaint.setColor(0xFF404040);
        SkPaint lightPaint;
        lightPaint.setColor(0xFFA0A0A0);
        for (int y = 8; y < 48; y += 16) {
            for (int x = 8; x < 48; x += 16) {
                canvas->save();
                canvas->translate(SkIntToScalar(x), SkIntToScalar(y));
                canvas->drawRect(SkRect::MakeXYWH(0, 0, 8, 8), darkPaint);
                canvas->drawRect(SkRect::MakeXYWH(8, 0, 8, 8), lightPaint);
                canvas->drawRect(SkRect::MakeXYWH(0, 8, 8, 8), lightPaint);
                canvas->drawRect(SkRect::MakeXYWH(8, 8, 8, 8), darkPaint);
                canvas->restore();
            }
        }
        return surf->makeImageSnapshot();
    }

    sk_sp<SkImage> make_gradient_circle(int width, int height) {
        SkScalar x = SkIntToScalar(width / 2);
        SkScalar y = SkIntToScalar(height / 2);
        SkScalar radius = std::min(x, y) * 0.8f;
        auto surface(SkSurfaces::Raster(SkImageInfo::MakeN32Premul(width, height)));
        SkCanvas* canvas = surface->getCanvas();
        canvas->clear(0x00000000);
        SkColor colors[2];
        colors[0] = SK_ColorWHITE;
        colors[1] = SK_ColorBLACK;
        SkPaint paint;
        paint.setShader(SkGradientShader::MakeRadial(SkPoint::Make(x, y), radius, colors, nullptr,
                                                     2, SkTileMode::kClamp));
        canvas->drawCircle(x, y, radius, paint);

        return surface->makeImageSnapshot();
    }

    void draw(SkCanvas* canvas, sk_sp<SkImage> image, const SkIRect& layerRect,
                     sk_sp<SkImageFilter> filter) {
          SkPaint paint;
          paint.setImageFilter(std::move(filter));

          // We don't pass 'layerRect' in as the saveLayer bounds hint because that is not the
          // pre-filtering local bounds (which would be image->dimensions()). We could clip before
          // the saveLayer but every 'filter' should have been constructed with a crop rect equal
          // to 'layerRect', so do an unclipped saveLayer to ensure that the crop rect itself is
          // what restricts the output of the filter.
          canvas->saveLayer(nullptr, &paint);
          canvas->drawImage(image, 0, 0);
          canvas->restore();

          SkPaint strokePaint;
          strokePaint.setColor(0xFFFF0000);
          strokePaint.setStyle(SkPaint::kStroke_Style);
          canvas->drawRect(SkRect::Make(layerRect), strokePaint);

          canvas->translate(SkIntToScalar(80), 0);
    }
}  // namespace
