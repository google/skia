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
#include "include/effects/SkBlurImageFilter.h"
#include "include/effects/SkColorFilterImageFilter.h"
#include "include/effects/SkDisplacementMapEffect.h"
#include "include/effects/SkDropShadowImageFilter.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkImageSource.h"
#include "include/effects/SkLightingImageFilter.h"
#include "include/effects/SkMorphologyImageFilter.h"
#include "include/effects/SkOffsetImageFilter.h"

#include <utility>

namespace {

void make_checkerboard(SkBitmap* bitmap);
sk_sp<SkImage> make_gradient_circle(int width, int height);
void draw(SkCanvas* canvas, const SkBitmap& bitmap, const SkRect& rect,
                 sk_sp<SkImageFilter> filter);

};

///////////////////////////////////////////////////////////////////////////////

DEF_SIMPLE_GM(imagefilterscropexpand, canvas, 730, 650) {
    SkImageFilter::CropRect cropRect(
        SkRect::Make(SkIRect::MakeXYWH(10, 10, 44, 44)),
        SkImageFilter::CropRect::kHasAll_CropEdge);

    sk_sp<SkImage> gradientCircle(make_gradient_circle(64, 64));
    SkBitmap checkerboard;
    make_checkerboard(&checkerboard);

    sk_sp<SkImageFilter> gradientCircleSource(SkImageSource::Make(std::move(gradientCircle)));
    sk_sp<SkImageFilter> noopCropped(SkOffsetImageFilter::Make(0, 0, nullptr, &cropRect));
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
        SkRect rect = cropRect.rect();
        rect.outset(SkIntToScalar(outset),
                    SkIntToScalar(outset));
        SkImageFilter::CropRect bigRect(rect, SkImageFilter::CropRect::kHasAll_CropEdge);

        draw(canvas, checkerboard, rect, SkColorFilterImageFilter::Make(cfAlphaTrans,
                                                                        noopCropped,
                                                                        &bigRect));

        draw(canvas, checkerboard, rect, SkBlurImageFilter::Make(0.3f, 0.3f,
                                                                 noopCropped,
                                                                 &bigRect));

        draw(canvas, checkerboard, rect, SkBlurImageFilter::Make(8.0f, 8.0f,
                                                                 noopCropped,
                                                                 &bigRect));

        draw(canvas, checkerboard, rect, SkDilateImageFilter::Make(2, 2,
                                                                   noopCropped,
                                                                   &bigRect));

        draw(canvas, checkerboard, rect, SkErodeImageFilter::Make(2, 2,
                                                                  noopCropped,
                                                                  &bigRect));

        draw(canvas, checkerboard, rect,
             SkDropShadowImageFilter::Make(
                                SkIntToScalar(10),
                                SkIntToScalar(10),
                                SkIntToScalar(3),
                                SkIntToScalar(3),
                                SK_ColorBLUE,
                                SkDropShadowImageFilter::kDrawShadowAndForeground_ShadowMode,
                                noopCropped,
                                &bigRect));

        draw(canvas, checkerboard, rect,
             SkDisplacementMapEffect::Make(SkDisplacementMapEffect::kR_ChannelSelectorType,
                                           SkDisplacementMapEffect::kR_ChannelSelectorType,
                                           SkIntToScalar(12),
                                           gradientCircleSource,
                                           noopCropped,
                                           &bigRect));

        draw(canvas, checkerboard, rect,
             SkOffsetImageFilter::Make(SkIntToScalar(-8), SkIntToScalar(16),
                                       noopCropped,
                                       &bigRect));

        draw(canvas, checkerboard, rect,
             SkLightingImageFilter::MakePointLitDiffuse(pointLocation,
                                                        SK_ColorWHITE,
                                                        surfaceScale,
                                                        kd,
                                                        noopCropped,
                                                        &bigRect));

        canvas->restore();
        canvas->translate(0, SkIntToScalar(80));
    }
}

namespace {
    void make_checkerboard(SkBitmap* bitmap) {
        bitmap->allocN32Pixels(64, 64);
        SkCanvas canvas(*bitmap);
        canvas.clear(0xFFFF0000);
        SkPaint darkPaint;
        darkPaint.setColor(0xFF404040);
        SkPaint lightPaint;
        lightPaint.setColor(0xFFA0A0A0);
        for (int y = 8; y < 48; y += 16) {
            for (int x = 8; x < 48; x += 16) {
                canvas.save();
                canvas.translate(SkIntToScalar(x), SkIntToScalar(y));
                canvas.drawRect(SkRect::MakeXYWH(0, 0, 8, 8), darkPaint);
                canvas.drawRect(SkRect::MakeXYWH(8, 0, 8, 8), lightPaint);
                canvas.drawRect(SkRect::MakeXYWH(0, 8, 8, 8), lightPaint);
                canvas.drawRect(SkRect::MakeXYWH(8, 8, 8, 8), darkPaint);
                canvas.restore();
            }
        }
    }

    sk_sp<SkImage> make_gradient_circle(int width, int height) {
        SkScalar x = SkIntToScalar(width / 2);
        SkScalar y = SkIntToScalar(height / 2);
        SkScalar radius = SkMinScalar(x, y) * 0.8f;
        auto surface(SkSurface::MakeRasterN32Premul(width, height));
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

    void draw(SkCanvas* canvas, const SkBitmap& bitmap, const SkRect& rect,
                     sk_sp<SkImageFilter> filter) {
        SkPaint paint;
        paint.setImageFilter(std::move(filter));
        canvas->saveLayer(&rect, &paint);
        canvas->drawBitmap(bitmap, 0, 0);
        canvas->restore();

        SkPaint strokePaint;
        strokePaint.setColor(0xFFFF0000);
        strokePaint.setStyle(SkPaint::kStroke_Style);
        canvas->drawRect(rect, strokePaint);

        canvas->translate(SkIntToScalar(80), 0);
    }
};
