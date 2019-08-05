/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkShader.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkImageFilters.h"
#include "tests/Test.h"

static void test_unscaled(skiatest::Reporter* reporter) {
    static const int kWidth = 10;
    static const int kHeight = 10;

    SkIRect ir = SkIRect::MakeWH(kWidth, kHeight);

    SkBitmap filterResult, paintResult;

    filterResult.allocN32Pixels(kWidth, kHeight);
    SkCanvas canvasFilter(filterResult);
    canvasFilter.clear(0x00000000);

    paintResult.allocN32Pixels(kWidth, kHeight);
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
        paint.setImageFilter(SkImageFilters::Paint(gradientPaint, &ir));
        canvasFilter.drawRect(SkRect::Make(ir), paint);
    }

    // Test using the paint directly
    {
        canvasPaint.drawRect(SkRect::Make(ir), gradientPaint);
    }

    // Assert that both paths yielded the same result
    for (int y = 0; y < kHeight; ++y) {
        const SkPMColor* filterPtr = filterResult.getAddr32(0, y);
        const SkPMColor* paintPtr = paintResult.getAddr32(0, y);
        for (int x = 0; x < kWidth; ++x, ++filterPtr, ++paintPtr) {
            REPORTER_ASSERT(reporter, *filterPtr == *paintPtr);
        }
    }
}

static void test_scaled(skiatest::Reporter* reporter) {
    static const int kWidth = 10;
    static const int kHeight = 10;

    SkIRect ir = SkIRect::MakeWH(kWidth, kHeight);

    SkBitmap filterResult, paintResult;

    filterResult.allocN32Pixels(kWidth, kHeight);
    SkCanvas canvasFilter(filterResult);
    canvasFilter.clear(0x00000000);

    paintResult.allocN32Pixels(kWidth, kHeight);
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
        paint.setImageFilter(SkImageFilters::Paint(gradientPaint, &ir));
        canvasFilter.scale(SkIntToScalar(2), SkIntToScalar(2));
        canvasFilter.drawRect(SkRect::Make(ir), paint);
    }

    // Test using the paint directly
    {
        canvasPaint.scale(SkIntToScalar(2), SkIntToScalar(2));
        canvasPaint.drawRect(SkRect::Make(ir), gradientPaint);
    }

    // Assert that both paths yielded the same result
    for (int y = 0; y < kHeight; ++y) {
        const SkPMColor* filterPtr = filterResult.getAddr32(0, y);
        const SkPMColor* paintPtr = paintResult.getAddr32(0, y);
        for (int x = 0; x < kWidth; ++x, ++filterPtr, ++paintPtr) {
            REPORTER_ASSERT(reporter, *filterPtr == *paintPtr);
        }
    }
}

DEF_TEST(PaintImageFilter, reporter) {
    test_unscaled(reporter);
    test_scaled(reporter);
}
