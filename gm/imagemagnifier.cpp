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
#include "include/core/SkRRect.h"
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
                SkImageFilters::Magnifier(SkRect::MakeWH(WIDTH, HEIGHT), 2.f, 100.f,
                                          SkFilterMode::kLinear, nullptr));
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
    sk_sp<SkImageFilter> imageSource(SkImageFilters::Image(make_img(), SkFilterMode::kNearest));

    // Crop out a 16 pixel ring around the result
    const SkIRect cropRect = SkIRect::MakeXYWH(16, 16, WIDTH_HEIGHT-32, WIDTH_HEIGHT-32);

    SkPaint filterPaint;
    filterPaint.setImageFilter(SkImageFilters::Magnifier(
            SkRect::MakeWH(WIDTH_HEIGHT, WIDTH_HEIGHT),
            WIDTH_HEIGHT / (WIDTH_HEIGHT - 96.f), 64.f, {},
            std::move(imageSource),  &cropRect));

    canvas->saveLayer(nullptr, &filterPaint);
    canvas->restore();
}

class ImageMagnifierBounds : public skiagm::GM {
public:
    ImageMagnifierBounds() : fX(0.f), fY(0.f) {}

protected:
    SkString getName() const override { return SkString("imagemagnifier_bounds"); }
    SkISize getISize() override { return SkISize::Make(768, 512); }

    bool onAnimate(double nanos) override {
        fX = TimeUtils::SineWave(nanos, 10.f, 0.f, -200.f, 200.f);
        fY = TimeUtils::SineWave(nanos, 10.f, 3.f, -200.f, 200.f);
        return true;
    }

    void onDraw(SkCanvas* canvas) override {
        this->drawRow(canvas, 16.f); // fish eye distortion
        canvas->translate(0.f, 256.f);
        this->drawRow(canvas, 0.f);  // no distortion, just zoom
    }

private:

    void drawRow(SkCanvas* canvas, float inset) {
        // Draw the magnifier two ways: backdrop filtered and then through a saveLayer with a
        // regular filter. Lastly draw the un-filtered input. Relevant bounds are displayed on
        // top of the rendering:
        //  - black = the lens bounding box
        //  - red   = the clipped inset lens bounds
        //  - blue  = the source of the undistorted magnified content
        auto drawBorder = [canvas](SkRect rect, SkColor color,
                                   float width, float borderInset = 0.f) {
            SkPaint paint;
            paint.setStyle(SkPaint::kStroke_Style);
            paint.setStrokeWidth(width);
            paint.setColor(color);
            paint.setAntiAlias(true);

            // This draws the original rect (unrounded) when borderInset = 0
            rect.inset(borderInset, borderInset);
            canvas->drawRRect(SkRRect::MakeRectXY(rect, borderInset, borderInset), paint);
        };

        // Logically there is a 'widgetBounds' that is the region of pixels to
        // be filled with magnified content. Pixels inside widgetBounds are
        // scaled up by a factor of 'zoomAmount', with a non linear distortion
        // applied to pixels up to 'inset' inside 'widgetBounds'. The specific
        // linearly scaled region is termed the 'srcRect' and is adjusted
        // dynamically if parts of 'widgetBounds' are offscreen.
        SkRect widgetBounds = {16.f, 24.f, 220.f, 248.f};
        widgetBounds.offset(fX, fY); // animating helps highlight magnifier behavior

        constexpr float kZoomAmount = 2.5f;

        // The available content for backdrops, which clips the widgetBounds as it animates.
        constexpr SkRect kOutBounds = {0.f, 0.f, 256.f, 256.f};

        // The filter responds to any crop (explicit or from missing backdrop content). Compute
        // the corresponding clipped bounds and source bounds for visualization purposes.
        SkPoint zoomCenter = widgetBounds.center();
        SkRect clippedWidget = widgetBounds;
        SkAssertResult(clippedWidget.intersect(kOutBounds));
        zoomCenter = {SkTPin(zoomCenter.fX, clippedWidget.fLeft, clippedWidget.fRight),
                      SkTPin(zoomCenter.fY, clippedWidget.fTop, clippedWidget.fBottom)};
        zoomCenter = zoomCenter * (1.f - 1.f / kZoomAmount);
        SkRect srcRect = {clippedWidget.fLeft   / kZoomAmount + zoomCenter.fX,
                          clippedWidget.fTop    / kZoomAmount + zoomCenter.fY,
                          clippedWidget.fRight  / kZoomAmount + zoomCenter.fX,
                          clippedWidget.fBottom / kZoomAmount + zoomCenter.fY};

        // Internally, the magnifier filter performs equivalent calculations but responds to the
        // canvas matrix and available input automatically.
        sk_sp<SkImageFilter> magnifier =
                SkImageFilters::Magnifier(widgetBounds, kZoomAmount, inset,
                                          SkFilterMode::kLinear, nullptr, kOutBounds);

        // Draw once as a backdrop filter
        canvas->save();
            canvas->clipRect(kOutBounds);
            draw_content(canvas, 32.f, 350);
            canvas->saveLayer({nullptr, nullptr, magnifier.get(), 0});
            canvas->restore();

            drawBorder(widgetBounds, SK_ColorBLACK, 2.f);
            if (inset > 0.f) {
                drawBorder(clippedWidget, SK_ColorRED, 2.f, inset);
            }
        canvas->restore();

        // Draw once as a regular filter
        canvas->save();
            canvas->translate(256.f, 0.f);
            canvas->clipRect(kOutBounds);

            SkPaint paint;
            paint.setImageFilter(magnifier);
            canvas->saveLayer(nullptr, &paint);
                draw_content(canvas, 32.f, 350);
            canvas->restore();

            drawBorder(widgetBounds, SK_ColorBLACK, 2.f);
            if (inset > 0.f) {
                drawBorder(clippedWidget, SK_ColorRED, 2.f, inset);
            }
        canvas->restore();

        // Draw once unfiltered
        canvas->save();
            canvas->translate(512.f, 0.f);
            canvas->clipRect(kOutBounds);
            draw_content(canvas, 32.f, 350);

            drawBorder(widgetBounds, SK_ColorBLACK, 2.f);
            drawBorder(srcRect, SK_ColorBLUE, 2.f, inset / kZoomAmount);
        canvas->restore();
    }

private:
    SkScalar fX;
    SkScalar fY;
};

DEF_GM(return new ImageMagnifierBounds(); )
