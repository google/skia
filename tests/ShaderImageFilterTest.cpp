/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "TestClassDef.h"
#include "SkCanvas.h"
#include "SkShader.h"
#include "SkRectShaderImageFilter.h"
#include "SkGradientShader.h"

DEF_TEST(ShaderImageFilter, reporter) {
    int w = 10, h = 10;
    SkRect r = SkRect::MakeWH(SkIntToScalar(w), SkIntToScalar(h)); // Make small 10x10 gradient

    SkBitmap filterResult, shaderResult;

    filterResult.setConfig(SkBitmap::kARGB_8888_Config, w, h);
    filterResult.allocPixels();
    SkCanvas canvasFilter(filterResult);
    canvasFilter.clear(0x00000000);

    shaderResult.setConfig(SkBitmap::kARGB_8888_Config, w, h);
    shaderResult.allocPixels();
    SkCanvas canvasShader(shaderResult);
    canvasShader.clear(0x00000000);

    SkPoint center = SkPoint::Make(SkIntToScalar(5), SkIntToScalar(5));
    SkColor colors[] = {SK_ColorBLUE, SK_ColorRED, SK_ColorGREEN};
    SkScalar pos[] = {0, SK_ScalarHalf, SK_Scalar1};
    SkScalar radius = SkIntToScalar(5);

    // Test using the image filter
    {
        SkShader* s = SkGradientShader::CreateRadial(
            center, radius, colors, pos, SK_ARRAY_COUNT(colors), SkShader::kClamp_TileMode);
        SkPaint paint;
        SkImageFilter::CropRect cr(r);
        paint.setImageFilter(SkRectShaderImageFilter::Create(s, &cr))->unref();
        canvasFilter.drawRect(r, paint);
        s->unref();
    }

    // Test using the shader directly
    {
        SkShader* s = SkGradientShader::CreateRadial(
            center, radius, colors, pos, SK_ARRAY_COUNT(colors), SkShader::kClamp_TileMode);
        SkPaint paint;
        paint.setShader(s)->unref();
        canvasShader.drawRect(r, paint);
    }

    // Assert that both paths yielded the same result
    for (int y = 0; y < r.height(); ++y) {
        const SkPMColor* filterPtr = filterResult.getAddr32(0, y);
        const SkPMColor* shaderPtr = shaderResult.getAddr32(0, y);
        for (int x = 0; x < r.width(); ++x, ++filterPtr, ++shaderPtr) {
            REPORTER_ASSERT(reporter, *filterPtr == *shaderPtr);
        }
    }
}
