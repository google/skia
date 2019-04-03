/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkPaintImageFilter.h"
#include "SkShader.h"
#include "Test.h"

static void test_unscaled(skiatest::Reporter* reporter) {
    int w = 10, h = 10;
    SkRect r = SkRect::MakeWH(SkIntToScalar(w), SkIntToScalar(h));

    SkBitmap filterResult, paintResult;

    filterResult.allocN32Pixels(w, h);
    SkCanvas canvasFilter(filterResult);
    canvasFilter.clear(0x00000000);

    paintResult.allocN32Pixels(w, h);
    SkCanvas canvasPaint(paintResult);
    canvasPaint.clear(0x00000000);

    SkPoint center = SkPoint::Make(SkIntToScalar(5), SkIntToScalar(5));
    SkColor colors[] = {SK_ColorBLUE, SK_ColorRED, SK_ColorGREEN};
    SkScalar pos[] = {0, SK_ScalarHalf, SK_Scalar1};
    SkScalar radius = SkIntToScalar(5);

    SkPaint gradientPaint;
    gradientPaint.setShader(SkGradientShader::MakeRadial(
        center, radius, colors, pos, SK_ARRAY_COUNT(colors), SkTileMode::kClamp));

    // Test using the image filter
    {
        SkPaint paint;
        SkImageFilter::CropRect cr(r);
        paint.setImageFilter(SkPaintImageFilter::Make(gradientPaint, &cr));
        canvasFilter.drawRect(r, paint);
    }

    // Test using the paint directly
    {
        canvasPaint.drawRect(r, gradientPaint);
    }

    // Assert that both paths yielded the same result
    for (int y = 0; y < r.height(); ++y) {
        const SkPMColor* filterPtr = filterResult.getAddr32(0, y);
        const SkPMColor* paintPtr = paintResult.getAddr32(0, y);
        for (int x = 0; x < r.width(); ++x, ++filterPtr, ++paintPtr) {
            REPORTER_ASSERT(reporter, *filterPtr == *paintPtr);
        }
    }
}

static void test_scaled(skiatest::Reporter* reporter) {
    int w = 10, h = 10;
    SkRect r = SkRect::MakeWH(SkIntToScalar(w), SkIntToScalar(h));

    SkBitmap filterResult, paintResult;

    filterResult.allocN32Pixels(w, h);
    SkCanvas canvasFilter(filterResult);
    canvasFilter.clear(0x00000000);

    paintResult.allocN32Pixels(w, h);
    SkCanvas canvasPaint(paintResult);
    canvasPaint.clear(0x00000000);

    SkPoint center = SkPoint::Make(SkIntToScalar(5), SkIntToScalar(5));
    SkColor colors[] = {SK_ColorBLUE, SK_ColorRED, SK_ColorGREEN};
    SkScalar pos[] = {0, SK_ScalarHalf, SK_Scalar1};
    SkScalar radius = SkIntToScalar(5);

    SkPaint gradientPaint;
    gradientPaint.setShader(SkGradientShader::MakeRadial(
        center, radius, colors, pos, SK_ARRAY_COUNT(colors), SkTileMode::kClamp));

    // Test using the image filter
    {
        SkPaint paint;
        SkImageFilter::CropRect cr(r);
        paint.setImageFilter(SkPaintImageFilter::Make(gradientPaint, &cr));
        canvasFilter.scale(SkIntToScalar(2), SkIntToScalar(2));
        canvasFilter.drawRect(r, paint);
    }

    // Test using the paint directly
    {
        canvasPaint.scale(SkIntToScalar(2), SkIntToScalar(2));
        canvasPaint.drawRect(r, gradientPaint);
    }

    // Assert that both paths yielded the same result
    for (int y = 0; y < r.height(); ++y) {
        const SkPMColor* filterPtr = filterResult.getAddr32(0, y);
        const SkPMColor* paintPtr = paintResult.getAddr32(0, y);
        for (int x = 0; x < r.width(); ++x, ++filterPtr, ++paintPtr) {
            REPORTER_ASSERT(reporter, *filterPtr == *paintPtr);
        }
    }
}

DEF_TEST(PaintImageFilter, reporter) {
    test_unscaled(reporter);
    test_scaled(reporter);
}
