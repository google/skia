/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkShader.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkGradientShader.h"
#include "src/shaders/SkColorShader.h"
#include "tests/Test.h"
#include "tools/ToolUtils.h"

static void test_bitmap(skiatest::Reporter* reporter) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(2, 2);

    SkBitmap bmp;
    bmp.setInfo(info);

    // test 1: bitmap without pixel data
    auto shader = bmp.makeShader(SkSamplingOptions());
    REPORTER_ASSERT(reporter, shader);
    REPORTER_ASSERT(reporter, !shader->isOpaque());

    // From this point on, we have pixels
    bmp.allocPixels(info);

    // test 2: not opaque by default
    shader = bmp.makeShader(SkSamplingOptions());
    REPORTER_ASSERT(reporter, shader);
    REPORTER_ASSERT(reporter, !shader->isOpaque());

    // test 3: explicitly opaque
    bmp.setAlphaType(kOpaque_SkAlphaType);
    shader = bmp.makeShader(SkSamplingOptions());
    REPORTER_ASSERT(reporter, shader);
    REPORTER_ASSERT(reporter, shader->isOpaque());

    // test 4: explicitly not opaque
    bmp.setAlphaType(kPremul_SkAlphaType);
    shader = bmp.makeShader(SkSamplingOptions());
    REPORTER_ASSERT(reporter, shader);
    REPORTER_ASSERT(reporter, !shader->isOpaque());
}

static void test_gradient(skiatest::Reporter* reporter) {
    SkPoint pts[2];
    pts[0].iset(0, 0);
    pts[1].iset(1, 0);
    SkColor colors[2];
    SkScalar pos[2] = {SkIntToScalar(0), SkIntToScalar(1)};
    int count = 2;
    SkTileMode mode = SkTileMode::kClamp;

    // test 1: all opaque
    colors[0] = SkColorSetARGB(0xFF, 0, 0, 0);
    colors[1] = SkColorSetARGB(0xFF, 0, 0, 0);
    auto grad = SkGradientShader::MakeLinear(pts, colors, pos, count, mode);
    REPORTER_ASSERT(reporter, grad);
    REPORTER_ASSERT(reporter, grad->isOpaque());

    // test 2: all 0 alpha
    colors[0] = SkColorSetARGB(0, 0, 0, 0);
    colors[1] = SkColorSetARGB(0, 0, 0, 0);
    grad = SkGradientShader::MakeLinear(pts, colors, pos, count, mode);
    REPORTER_ASSERT(reporter, grad);
    REPORTER_ASSERT(reporter, !grad->isOpaque());

    // test 3: one opaque, one transparent
    colors[0] = SkColorSetARGB(0xFF, 0, 0, 0);
    colors[1] = SkColorSetARGB(0x40, 0, 0, 0);
    grad = SkGradientShader::MakeLinear(pts, colors, pos, count, mode);
    REPORTER_ASSERT(reporter, grad);
    REPORTER_ASSERT(reporter, !grad->isOpaque());

    // test 4: test 3, swapped
    colors[0] = SkColorSetARGB(0x40, 0, 0, 0);
    colors[1] = SkColorSetARGB(0xFF, 0, 0, 0);
    grad = SkGradientShader::MakeLinear(pts, colors, pos, count, mode);
    REPORTER_ASSERT(reporter, grad);
    REPORTER_ASSERT(reporter, !grad->isOpaque());
}

static void test_color(skiatest::Reporter* reporter) {
    SkColorShader colorShader1(SkColorSetARGB(0,0,0,0));
    REPORTER_ASSERT(reporter, !colorShader1.isOpaque());
    SkColorShader colorShader2(SkColorSetARGB(0xFF,0,0,0));
    REPORTER_ASSERT(reporter, colorShader2.isOpaque());
    SkColorShader colorShader3(SkColorSetARGB(0x7F,0,0,0));
    REPORTER_ASSERT(reporter, !colorShader3.isOpaque());
}

DEF_TEST(ShaderOpacity, reporter) {
    test_gradient(reporter);
    test_color(reporter);
    test_bitmap(reporter);
}

#ifdef SK_SUPPORT_LEGACY_IMPLICIT_FILTERQUALITY
DEF_TEST(image_shader_filtering, reporter) {
    auto make_checker_img = [](int w, int h) {
        auto info = SkImageInfo::Make(w, h, kRGBA_8888_SkColorType, kOpaque_SkAlphaType);
        auto surf = SkSurface::MakeRaster(info);
        ToolUtils::draw_checkerboard(surf->getCanvas(), SK_ColorRED, SK_ColorBLUE, 2);
        return surf->makeImageSnapshot();
    };

    auto img = make_checker_img(4, 4);

    const SkFilterQuality quals[] = {
        kNone_SkFilterQuality,
        kLow_SkFilterQuality,
        kMedium_SkFilterQuality,
        kHigh_SkFilterQuality,
    };
    const SkScalar scales[] = { 3.0f, 1.0f, 0.5f, 0.25f, 0.125f };

    auto make_img = [&](const SkPaint& paint) {
        auto info = SkImageInfo::Make(70, 70, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
        auto surf = SkSurface::MakeRaster(info);
        auto canvas = surf->getCanvas();
        canvas->scale(1.06f, 1.06f);    // nicely exaggerates noise when not filtering
        canvas->drawRect({0, 0, 64, 64}, paint);
        return surf->makeImageSnapshot();
    };

    SkPaint paint;
    for (auto q : quals) {
        for (auto scale : scales) {
            SkMatrix m = SkMatrix::Scale(scale, scale);
            paint.setFilterQuality(kNone_SkFilterQuality); // this setting should be ignored
            paint.setShader(img->makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat,
                                            SkSamplingOptions(q), &m));
            auto img0 = make_img(paint);

            paint.setFilterQuality(q);  // this should (still) be ignored
            auto img1 = make_img(paint);

            // make legacy form of shader, relying on the paint's filter-quality (q)
            paint.setShader(img->makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat, &m));
            auto img2 = make_img(paint);

            REPORTER_ASSERT(reporter, ToolUtils::equal_pixels(img0.get(), img1.get()));
            REPORTER_ASSERT(reporter, ToolUtils::equal_pixels(img0.get(), img2.get()));
        }
    }
}
#endif
