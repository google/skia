/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/effects/SkBlurImageFilter.h"
#include "include/effects/SkColorFilterImageFilter.h"
#include "include/core/SkPaint.h"

// This draws correctly if there's a small cyan rectangle above a much larger magenta rectangle.
// There should be no red around the cyan rectangle and no green within the magenta rectangle.

DEF_SIMPLE_GM(backdrop_imagefilter_croprect, canvas, 600, 500) {
    // CTM translates to (150, 150)
    SkPoint origin = SkPoint::Make(150.f, 150.f);
    // The save layer specified after the CTM has negative coordinates, but
    // means that (100, 100) to (500, 250) in device-space will be saved
    SkRect clip = SkRect::MakeXYWH(-50.f, -50.f, 400.f, 150.f);
    // The image-filter crop relative to the CTM, which will map to
    // (200, 160) to (400, 190) in device space, or (100, 600) to (300, 90) in
    // the layer's image space.
    SkRect cropInLocal = SkRect::MakeLTRB(50.f, 10.f, 250.f, 40.f);

    SkImageFilter::CropRect cropRect(cropInLocal);
    float matrix[20] = {-1.f, 0.f, 0.f, 0.f, 1.f,
                        0.f, -1.f, 0.f, 0.f, 1.f,
                        0.f, 0.f, -1.f, 0.f, 1.f,
                        0.f, 0.f, 0.f, 1.f, 0.f};
    sk_sp<SkColorFilter> colorFilter = SkColorFilters::Matrix(matrix);
    sk_sp<SkImageFilter> imageFilter = SkColorFilterImageFilter::Make(colorFilter, nullptr,
                                                                      &cropRect);

    SkPaint p;
    SkPaint l;
    l.setStyle(SkPaint::kStroke_Style);
    l.setStrokeWidth(0.f);

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
            canvas->saveLayer({nullptr, nullptr, imageFilter.get(), nullptr, nullptr,
                               SkCanvas::kInitWithPrevious_SaveLayerFlag});
        }

        // Restore the saved layer (either a main layer that was just drawn into and needs to be
        // filtered, or an "empty" layer initialized with the previously filtered backdrop)
        canvas->restore();

        // Move down
        canvas->restore();
        origin.fY += 150.f;
    }
}
