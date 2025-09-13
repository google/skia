/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkPaint.h"
#include "include/effects/SkImageFilters.h"

typedef sk_sp<SkImageFilter> (*FilterFactory)(const SkIRect* crop);

static void draw_backdrop_filter_gm(SkCanvas* canvas, float outsetX, float outsetY,
                                    FilterFactory factory) {
    // CTM translates to (150, 150)
    SkPoint origin = SkPoint::Make(150.f, 150.f);
    // The save layer specified after the CTM has negative coordinates, but
    // means that (100, 100) to (500, 250) in device-space will be saved
    SkRect clip = SkRect::MakeXYWH(-50.f, -50.f, 400.f, 150.f);
    // The image-filter crop relative to the CTM, which will map to
    // (200, 160) to (400, 190) in device space, or (100, 600) to (300, 90) in
    // the layer's image space.
    SkRect cropInLocal = SkRect::MakeLTRB(50.f, 10.f, 250.f, 40.f);

    SkIRect cropRect = cropInLocal.makeOutset(outsetX, outsetY).roundOut();
    sk_sp<SkImageFilter> imageFilter = factory(&cropRect);

    SkPaint p;
    for (int i = 0; i < 2; ++i) {
        canvas->save();
        canvas->translate(origin.fX, origin.fY);

        canvas->clipRect(clip);

        if (i == 0) {
            // Primary save layer mode, so save layer before drawing the content
            SkPaint imfPaint;
            imfPaint.setImageFilter(imageFilter);
            canvas->saveLayer(nullptr, &imfPaint);
        } // else backdrop mode, so the content is drawn first

        // Fill the clip with one color (cyan for i == 0 (inverse = red), and
        // magenta for i == 1 (inverse = green))
        p.setColor(i == 0 ? SK_ColorCYAN : SK_ColorMAGENTA);
        canvas->drawPaint(p);

        // Then an inner rectangle with a color meant to be inverted by the image filter
        p.setColor(i == 0 ? SK_ColorRED : SK_ColorGREEN);
        canvas->drawRect(cropInLocal, p);

        if (i == 1) {
            // Backdrop mode, so save a layer using the image filter as the backdrop to filter
            // content on initialization.
            canvas->saveLayer(SkCanvas::SaveLayerRec(nullptr, nullptr, imageFilter.get(),
                                                     SkCanvas::kInitWithPrevious_SaveLayerFlag));
        }

        // Restore the saved layer (either a main layer that was just drawn into and needs to be
        // filtered, or an "empty" layer initialized with the previously filtered backdrop)
        canvas->restore();

        // Move down
        canvas->restore();
        origin.fY += 150.f;
    }
}

static sk_sp<SkImageFilter> make_invert_filter(const SkIRect* crop) {
    static const float matrix[20] = {-1.f, 0.f, 0.f, 0.f, 1.f,
                                      0.f, -1.f, 0.f, 0.f, 1.f,
                                      0.f, 0.f, -1.f, 0.f, 1.f,
                                      0.f, 0.f, 0.f, 1.f, 0.f};
    return SkImageFilters::ColorFilter(SkColorFilters::Matrix(matrix), nullptr, crop);
}

static sk_sp<SkImageFilter> make_blur_filter(const SkIRect* crop) {
    // Use different sigmas for x and y so rotated CTM is apparent
    return SkImageFilters::Blur(16.f, 4.f, nullptr, crop);
}

// This draws correctly if there's a small cyan rectangle above a much larger magenta rectangle.
// There should be no red around the cyan rectangle and no green within the magenta rectangle.
DEF_SIMPLE_GM(backdrop_imagefilter_croprect, canvas, 600, 500) {
    draw_backdrop_filter_gm(canvas, 0.f, 0.f, make_invert_filter);
}

// This draws correctly if there's a blurred red rectangle inside a cyan rectangle, above a blurred
// green rectangle inside a larger magenta rectangle. All rectangles and the blur direction are
// consistently rotated.
DEF_SIMPLE_GM(backdrop_imagefilter_croprect_rotated, canvas, 600, 500) {
    canvas->translate(140.f, -180.f);
    canvas->rotate(30.f);
    draw_backdrop_filter_gm(canvas, 32.f, 32.f, make_blur_filter);
}

// This draws correctly if there's a blurred red rectangle inside a cyan rectangle, above a blurred
// green rectangle inside a larger magenta rectangle. All rectangles and the blur direction are
// under consistent perspective.
// NOTE: Currently renders incorrectly, see skbug.com/40040358
DEF_SIMPLE_GM(backdrop_imagefilter_croprect_persp, canvas, 600, 500) {
    SkMatrix persp = SkMatrix::I();
    persp.setPerspY(0.001f);
    persp.setSkewX(8.f / 25.f);
    canvas->concat(persp);
    draw_backdrop_filter_gm(canvas, 32.f, 32.f, make_blur_filter);
}

// This draws correctly if there's a small cyan rectangle above a much larger magenta rectangle.
// There should be no red around the cyan rectangle and no green within the magenta rectangle, and
// everything should be 50% transparent.
DEF_SIMPLE_GM(backdrop_imagefilter_croprect_nested, canvas, 600, 500) {
    SkPaint p;
    p.setAlphaf(0.5f);
    // This ensures there is a non-root device on the stack with a non-zero origin.
    canvas->translate(15.f, 10.f);
    canvas->clipRect(SkRect::MakeWH(600.f, 500.f));

    canvas->saveLayer(nullptr, &p);
    draw_backdrop_filter_gm(canvas, 0.f, 0.f, make_invert_filter);
    canvas->restore();
}

DEF_SIMPLE_GM(backdrop_layer_tilemode, canvas, 512, 128) {
    auto drawBackdropTileMode = [canvas](SkTileMode backdropTileMode) {
        canvas->save();
            // Restrict the canvas before starting a new layer to control its size.
            canvas->clipRect(SkRect::MakeIWH(128, 128));
            // This layer will be the backdrop content, but without any additional effects, its size
            // will match the clip (128x128).
            canvas->saveLayer(nullptr, nullptr);
                // Fill the layer with high frequency content (stripes of red and white)
                for (int y = 0; y < 128; y += 8) {
                    SkPaint fill;
                    fill.setColor(y % 16 ? SK_ColorRED : SK_ColorWHITE);
                    canvas->drawRect(SkRect::MakeXYWH(0, y, 128, 8), fill);
                }
                // Perform a backdrop blur layer with the specified backdrop tile mode
                sk_sp<SkImageFilter> blur = SkImageFilters::Blur(32.f, 32.f, nullptr);
                canvas->saveLayer({nullptr, nullptr, blur.get(), backdropTileMode, nullptr, 0});
                canvas->restore();
            canvas->restore();
        canvas->restore();

        canvas->translate(128, 0);
    };

    drawBackdropTileMode(SkTileMode::kClamp);
    drawBackdropTileMode(SkTileMode::kDecal);
    drawBackdropTileMode(SkTileMode::kRepeat);
    drawBackdropTileMode(SkTileMode::kMirror);
}
