/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkPath.h"
#include "SkPictureRecorder.h"
#include "SkTableColorFilter.h"
#include "SkColorFilterImageFilter.h"
#include "SkPictureImageFilter.h"

static const int kTestRectSize = 50;
static const int kDetectorGreenValue = 50;

// Below are few functions to install "detector" color filters. The filter is there to assert that
// the color value it sees is the expected. It will trigger only with kDetectorGreenValue, and
// turn that value into full green. The idea is that if an optimization incorrectly changes
// kDetectorGreenValue and then the incorrect value is observable by some part of the drawing
// pipeline, that pixel will remain empty.

static sk_sp<SkColorFilter> make_detector_color_filter() {
    uint8_t tableA[256] = { 0, };
    uint8_t tableR[256] = { 0, };
    uint8_t tableG[256] = { 0, };
    uint8_t tableB[256] = { 0, };
    tableA[255] = 255;
    tableG[kDetectorGreenValue] = 255;
    return SkTableColorFilter::MakeARGB(tableA, tableR, tableG, tableB);
}

// This detector detects that color filter phase of the pixel pipeline receives the correct value.
static void install_detector_color_filter(SkPaint* drawPaint) {
    drawPaint->setColorFilter(make_detector_color_filter());
}

// This detector detects that image filter phase of the pixel pipeline receives the correct value.
static void install_detector_image_filter(SkPaint* drawPaint) {
    sk_sp<SkColorFilter> colorFilter(make_detector_color_filter());
    sk_sp<SkImageFilter> imageFilter(
        SkColorFilterImageFilter::Make(std::move(colorFilter),
                                       sk_ref_sp(drawPaint->getImageFilter())));
    drawPaint->setImageFilter(std::move(imageFilter));
}

static void no_detector_install(SkPaint*) {
}

typedef void(*InstallDetectorFunc)(SkPaint*);


// Draws an pattern that can be optimized by alpha folding outer savelayer alpha value to
// inner draw. Since we know that folding will happen to the inner draw, install a detector
// to make sure that optimization does not change anything observable.
static void draw_save_layer_draw_rect_restore_sequence(SkCanvas* canvas, SkColor shapeColor,
                                                       InstallDetectorFunc installDetector) {
    SkRect targetRect(SkRect::MakeWH(SkIntToScalar(kTestRectSize), SkIntToScalar(kTestRectSize)));
    SkPaint layerPaint;
    layerPaint.setColor(SkColorSetARGB(128, 0, 0, 0));
    canvas->saveLayer(&targetRect, &layerPaint);
        SkPaint drawPaint;
        drawPaint.setColor(shapeColor);
        installDetector(&drawPaint);
        canvas->drawRect(targetRect, drawPaint);
    canvas->restore();
}

// Draws an pattern that can be optimized by alpha folding outer savelayer alpha value to
// inner draw. A variant where the draw is not uniform color.
static void draw_save_layer_draw_bitmap_restore_sequence(SkCanvas* canvas, SkColor shapeColor,
                                                         InstallDetectorFunc installDetector) {
    SkBitmap bitmap;
    bitmap.allocN32Pixels(kTestRectSize, kTestRectSize);
    bitmap.eraseColor(shapeColor);
    {
        // Make the bitmap non-uniform color, so that it can not be optimized as uniform drawRect.
        SkCanvas canvas(bitmap);
        SkPaint p;
        p.setColor(SK_ColorWHITE);
        SkASSERT(shapeColor != SK_ColorWHITE);
        canvas.drawRect(SkRect::MakeWH(SkIntToScalar(7), SkIntToScalar(7)), p);
        canvas.flush();
    }

    SkRect targetRect(SkRect::MakeWH(SkIntToScalar(kTestRectSize), SkIntToScalar(kTestRectSize)));
    SkPaint layerPaint;
    layerPaint.setColor(SkColorSetARGB(129, 0, 0, 0));
    canvas->saveLayer(&targetRect, &layerPaint);
        SkPaint drawPaint;
        installDetector(&drawPaint);
        canvas->drawBitmap(bitmap, SkIntToScalar(0), SkIntToScalar(0), &drawPaint);
    canvas->restore();
}

// Draws an pattern that can be optimized by alpha folding outer savelayer alpha value to
// inner savelayer. We know that alpha folding happens to inner savelayer, so add detector there.
static void draw_svg_opacity_and_filter_layer_sequence(SkCanvas* canvas, SkColor shapeColor,
                                                       InstallDetectorFunc installDetector) {

    SkRect targetRect(SkRect::MakeWH(SkIntToScalar(kTestRectSize), SkIntToScalar(kTestRectSize)));
    sk_sp<SkPicture> shape;
    {
        SkPictureRecorder recorder;
        SkCanvas* canvas = recorder.beginRecording(SkIntToScalar(kTestRectSize + 2),
                                                   SkIntToScalar(kTestRectSize + 2));
        SkPaint shapePaint;
        shapePaint.setColor(shapeColor);
        canvas->drawRect(targetRect, shapePaint);
        shape = recorder.finishRecordingAsPicture();
    }

    SkPaint layerPaint;
    layerPaint.setColor(SkColorSetARGB(130, 0, 0, 0));
    canvas->saveLayer(&targetRect, &layerPaint);
        canvas->save();
            canvas->clipRect(targetRect);
            SkPaint drawPaint;
            drawPaint.setImageFilter(SkPictureImageFilter::Make(shape));
            installDetector(&drawPaint);
            canvas->saveLayer(&targetRect, &drawPaint);
            canvas->restore();
        canvas->restore();
    canvas->restore();
}

// Draws two columns of rectangles. The test is correct when:
//  - Left and right columns always identical
//  - First 3 rows are green, with a white dent in the middle row
//  - Next 6 rows are green, with a grey dent in the middle row
//    (the grey dent is from the color filter removing everything but the "good" green, see below)
//  - Last 6 rows are grey
DEF_SIMPLE_GM(recordopts, canvas, (kTestRectSize+1)*2, (kTestRectSize+1)*15) {
    canvas->clear(SK_ColorTRANSPARENT);

    typedef void (*TestVariantSequence)(SkCanvas*, SkColor, InstallDetectorFunc);
    TestVariantSequence funcs[] = {
        draw_save_layer_draw_rect_restore_sequence,
        draw_save_layer_draw_bitmap_restore_sequence,
        draw_svg_opacity_and_filter_layer_sequence,
    };

    // Draw layer-related sequences that can be optimized by folding the opacity layer alpha to
    // the inner draw operation. This tries to trigger the optimization, and relies on gm diffs
    // to keep the color value correct over time.

    // Draws two green rects side by side: one is without the optimization, the other is with
    // the optimization applied.

    SkColor shapeColor = SkColorSetARGB(255, 0, 255, 0);
    for (size_t k = 0; k < SK_ARRAY_COUNT(funcs); ++k) {
        canvas->save();

        TestVariantSequence drawTestSequence = funcs[k];
        drawTestSequence(canvas, shapeColor, no_detector_install);
        canvas->flush();
        canvas->translate(SkIntToScalar(kTestRectSize) + SkIntToScalar(1), SkIntToScalar(0));
        {
            SkPictureRecorder recorder;
            drawTestSequence(recorder.beginRecording(SkIntToScalar(kTestRectSize),
                                                     SkIntToScalar(kTestRectSize)),
                             shapeColor, no_detector_install);
            recorder.finishRecordingAsPicture()->playback(canvas);
            canvas->flush();
        }
        canvas->restore();
        canvas->translate(SkIntToScalar(0), SkIntToScalar(kTestRectSize) + SkIntToScalar(1));
    }

    // Draw the same layer related sequences, but manipulate the sequences so that the result is
    // incorrect if the alpha is folded or folded incorrectly. These test the observable state
    // throughout the pixel pipeline, and thus may turn off the optimizations (this is why we
    // trigger the optimizations above).

    // Draws two green rects side by side: one is without the optimization, the other is with
    // the possibility that optimization is applied.
    // At the end, draws the same patterns in translucent black. This tests that the detectors
    // work, eg. that if the value the detector sees is wrong, the resulting image shows this.
    SkColor shapeColors[] = {
        SkColorSetARGB(255, 0, kDetectorGreenValue, 0),
        SkColorSetARGB(255, 0, (kDetectorGreenValue + 1), 0) // This tests that detectors work.
    };

    InstallDetectorFunc detectorInstallFuncs[] = {
        install_detector_image_filter,
        install_detector_color_filter
    };

    for (size_t i = 0; i < SK_ARRAY_COUNT(shapeColors); ++i) {
        shapeColor = shapeColors[i];
        for (size_t j = 0; j < SK_ARRAY_COUNT(detectorInstallFuncs); ++j) {
            InstallDetectorFunc detectorInstallFunc = detectorInstallFuncs[j];
            for (size_t k = 0; k < SK_ARRAY_COUNT(funcs); ++k) {
                TestVariantSequence drawTestSequence = funcs[k];
                canvas->save();
                drawTestSequence(canvas, shapeColor, detectorInstallFunc);
                canvas->flush();
                canvas->translate(SkIntToScalar(kTestRectSize) + SkIntToScalar(1), SkIntToScalar(0));
                {
                    SkPictureRecorder recorder;
                    drawTestSequence(recorder.beginRecording(SkIntToScalar(kTestRectSize),
                                                             SkIntToScalar(kTestRectSize)),
                                     shapeColor, detectorInstallFunc);
                    recorder.finishRecordingAsPicture()->playback(canvas);
                    canvas->flush();
                }

                canvas->restore();
                canvas->translate(SkIntToScalar(0), SkIntToScalar(kTestRectSize) + SkIntToScalar(1));
            }

        }
    }
}
