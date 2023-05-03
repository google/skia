/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPixelRef.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkTypeface.h"
#include "include/effects/SkImageFilters.h"
#include "src/base/SkRandom.h"
#include "tools/ToolUtils.h"
#include "tools/timer/TimeUtils.h"

#include <utility>

#define WIDTH 500
#define HEIGHT 500

static void draw_content(SkCanvas* canvas, float maxTextSize, int count) {
    const char* str = "The quick brown fox jumped over the lazy dog.";
    SkRandom rand;
    SkFont      font(ToolUtils::create_portable_typeface());
    for (int i = 0; i < count; ++i) {
        int x = rand.nextULessThan(WIDTH);
        int y = rand.nextULessThan(HEIGHT);
        SkPaint paint;
        paint.setColor(ToolUtils::color_to_565(rand.nextBits(24) | 0xFF000000));
        font.setSize(rand.nextRangeScalar(0, maxTextSize));
        canvas->drawString(str, SkIntToScalar(x), SkIntToScalar(y), font, paint);
    }
}

DEF_SIMPLE_GM_BG(imagemagnifier, canvas, WIDTH, HEIGHT, SK_ColorBLACK) {
        SkPaint filterPaint;
        filterPaint.setImageFilter(
            SkImageFilters::Magnifier(
                SkRect::MakeXYWH(SkIntToScalar(100), SkIntToScalar(100),
                                 SkIntToScalar(WIDTH / 2),
                                 SkIntToScalar(HEIGHT / 2)),
                100, nullptr));
        canvas->saveLayer(nullptr, &filterPaint);
        draw_content(canvas, 300.f, 25);
        canvas->restore();
}

////////////////////////////////////////////////////////////////////////////////
#define WIDTH_HEIGHT 256

static sk_sp<SkImage> make_img() {
    SkBitmap bitmap;
    bitmap.allocN32Pixels(WIDTH_HEIGHT, WIDTH_HEIGHT);
    SkCanvas canvas(bitmap);

    canvas.clear(0x0);

    SkPaint paint;
    paint.setColor(SK_ColorBLUE);

    for (float pos = 0; pos < WIDTH_HEIGHT; pos += 16) {
        canvas.drawLine(0, pos, SkIntToScalar(WIDTH_HEIGHT), pos, paint);
        canvas.drawLine(pos, 0, pos, SkIntToScalar(WIDTH_HEIGHT), paint);
    }

    SkBitmap result;
    result.setInfo(SkImageInfo::MakeS32(WIDTH_HEIGHT, WIDTH_HEIGHT, kPremul_SkAlphaType));
    result.setPixelRef(sk_ref_sp(bitmap.pixelRef()), 0, 0);

    return result.asImage();
}

DEF_SIMPLE_GM_BG(imagemagnifier_cropped, canvas, WIDTH_HEIGHT, WIDTH_HEIGHT, SK_ColorBLACK) {

    sk_sp<SkImage> image(make_img());

    sk_sp<SkImageFilter> imageSource(SkImageFilters::Image(std::move(image)));

    SkRect srcRect = SkRect::MakeWH(SkIntToScalar(WIDTH_HEIGHT-32),
                                    SkIntToScalar(WIDTH_HEIGHT-32));
    srcRect.inset(64.0f, 64.0f);

    constexpr SkScalar kInset = 64.0f;

    // Crop out a 16 pixel ring around the result
    const SkIRect cropRect = SkIRect::MakeXYWH(16, 16, WIDTH_HEIGHT-32, WIDTH_HEIGHT-32);

    SkPaint filterPaint;
    filterPaint.setImageFilter(SkImageFilters::Magnifier(
            srcRect, kInset, std::move(imageSource),  &cropRect));

    canvas->saveLayer(nullptr, &filterPaint);
    canvas->restore();
}

class ImageMagnifierBounds : public skiagm::GM {
public:
    ImageMagnifierBounds() : fX(0.f), fY(0.f) {}

protected:
    SkString onShortName() override { return SkString("imagemagnifier_bounds"); }
    SkISize onISize() override { return SkISize::Make(768, 512); }

    bool onAnimate(double nanos) override {
        fX = TimeUtils::SineWave(nanos, 10.f, 0.f, -64.f, 128.f);
        fY = TimeUtils::SineWave(nanos, 10.f, 3.f, -64.f, 128.f);
        return true;
    }

    void onDraw(SkCanvas* canvas) override {
        // Draw the magnifier two ways: backdrop filtered and then through a saveLayer with a
        // regular filter. Lastly draw the un-filtered input. Relevant bounds are displayed on
        // top of the rendering:
        //  - black = the lens bounding box
        //  - red   = the clipped inset lens bounds
        //  - blue  = the source of the undistorted magnified content
        auto drawBorder = [canvas](SkRect rect, SkColor color, float width, float inset = 0.f) {
            rect.inset(inset, inset);
            SkPaint paint;
            paint.setStyle(SkPaint::kStroke_Style);
            paint.setStrokeWidth(width);
            paint.setColor(color);
            canvas->drawRect(rect, paint);
        };

        // Logically there is a 'widgetBounds' that is the region of pixels to
        // be filled with magnified content. Pixels inside widgetBounds are
        // scaled up by a factor of 'zoomAmount', with a non linear distortion
        // applied to pixels up to 'inset' inside 'widgetBounds'. The specific
        // linearly scaled region is termed the 'srcRect' and is adjusted
        // dynamically if parts of 'widgetBounds' are offscreen.
        SkRect widgetBounds = {16.f, 24.f, 220.f, 248.f};
        widgetBounds.offset(fX, fY); // animating helps highlight magnifier behavior

        constexpr float kInset = 16.f;
        constexpr float kZoomAmount = 2.5f;

        // The available content for backdrops, which clips the widgetBounds as it animates.
        constexpr SkRect kOutBounds = {0.f, 0.f, 256.f, 256.f};

        // The legacy Magnifier filter only accepted the srcRect that was to be
        // magnified, and inferred magnification by filling the desired output
        // of the image filter. Besides violating the image filter contract for
        // naive users of magnifier, Chromium's use was carefully controlled
        // such that the output size was always the widget's size and srcRect
        // was derived from zoom amount.
        // The inputs to Magnifier are calculated based on Chromium's logic from:
        // https://docs.google.com/document/d/1pwARyNTMWYf0N2FW3aoX0E3C_8cha56M6iJ3_OmQbGQ/edit?usp=sharing&resourcekey=0-c51EedcRMdxuw03qDZtw4A
        // https://source.chromium.org/chromium/chromium/src/+/main:components/viz/service/display/skia_renderer.cc;drc=48340c1e35efad5fb0253025dcc36b3a9573e258;l=2900
        SkRect clippedWidget = widgetBounds;
        SkAssertResult(clippedWidget.intersect(kOutBounds));
        SkVector offset = {(widgetBounds.fRight  - clippedWidget.fRight) +
                           (widgetBounds.fLeft   - clippedWidget.fLeft),
                           (widgetBounds.fTop    - clippedWidget.fTop) +
                           (widgetBounds.fBottom - clippedWidget.fBottom)};

        // and https://source.chromium.org/chromium/chromium/src/+/main:cc/paint/render_surface_filters.cc;drc=48340c1e35efad5fb0253025dcc36b3a9573e258;l=220
        // NOTE: Assuming widgetBounds does not span both edges of outBounds and has a top-left
        // corner of (0,0), this is equivalent to unclippedCenter = widgetBounds.center(), which
        // appears to be the intent. The original Chromium logic is preserved.
        SkPoint unclippedCenter = {(clippedWidget.width()  + offset.fX) / 2.f,
                                   (clippedWidget.height() + offset.fY) / 2.f};

        SkPoint zoomCenter = {SkTPin(unclippedCenter.x(), 0.f, clippedWidget.width()),
                              SkTPin(unclippedCenter.y(), 0.f, clippedWidget.height())};

        SkRect srcRect = SkRect::MakeXYWH(zoomCenter.fX - zoomCenter.fX / kZoomAmount,
                                          zoomCenter.fY - zoomCenter.fY / kZoomAmount,
                                          clippedWidget.width() / kZoomAmount,
                                          clippedWidget.height() / kZoomAmount);

        sk_sp<SkImageFilter> magnifier =
                SkImageFilters::Magnifier(srcRect, kInset, nullptr, kOutBounds);

        // Draw once as a backdrop filter
        canvas->save();
            canvas->clipRect(kOutBounds);
            draw_content(canvas, 32.f, 350);

            canvas->save();
                canvas->clipRect(clippedWidget);
                canvas->saveLayer({nullptr, nullptr, magnifier.get(), 0});
                canvas->restore();
            canvas->restore();

            drawBorder(widgetBounds, SK_ColorBLACK, 6.f);
            drawBorder(clippedWidget, SK_ColorRED, 3.f, kInset);
        canvas->restore();

        // Draw once as a regular filter
        canvas->save();
            canvas->translate(256.f, 0.f);
            canvas->clipRect(kOutBounds);

            canvas->save();
                SkPaint paint;
                paint.setImageFilter(magnifier);
                canvas->clipRect(clippedWidget);
                canvas->saveLayer(nullptr, &paint);
                    draw_content(canvas, 32.f, 350);
                canvas->restore();
            canvas->restore();

            drawBorder(widgetBounds, SK_ColorBLACK, 6.f);
            drawBorder(clippedWidget, SK_ColorRED, 3.f, kInset);
        canvas->restore();

        // Draw once unfiltered
        canvas->save();
            canvas->translate(512.f, 0.f);
            canvas->clipRect(kOutBounds);
            draw_content(canvas, 32.f, 350);

            drawBorder(widgetBounds, SK_ColorBLACK, 6.f);

            // NOTE: To make srcRect appear in the right spot we add the widget's top-left corner
            // but the legacy implementation expected coordinates relative to the underlying
            // SkDevice in order to work correctly. We also apply the zoom-adjusted inset.
            srcRect.offset(clippedWidget.fLeft, clippedWidget.fTop);
            srcRect.inset(kInset / kZoomAmount, kInset / kZoomAmount);
            drawBorder(srcRect, SK_ColorBLUE, 3.f);
        canvas->restore();
    }

private:
    SkScalar fX;
    SkScalar fY;
};

DEF_GM(return new ImageMagnifierBounds(); )
