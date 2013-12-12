/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "TestClassDef.h"
#include "SkShader.h"
#include "SkGradientShader.h"
#include "SkColorShader.h"

static void test_bitmap(skiatest::Reporter* reporter) {
    SkBitmap bmp;
    bmp.setConfig(SkBitmap::kARGB_8888_Config, 2, 2);

    // test 1: bitmap without pixel data
    SkShader* shader = SkShader::CreateBitmapShader(bmp,
        SkShader::kClamp_TileMode, SkShader::kClamp_TileMode);
    REPORTER_ASSERT(reporter, shader);
    REPORTER_ASSERT(reporter, !shader->isOpaque());
    shader->unref();

    // From this point on, we have pixels
    bmp.allocPixels();

    // test 2: not opaque by default
    shader = SkShader::CreateBitmapShader(bmp,
        SkShader::kClamp_TileMode, SkShader::kClamp_TileMode);
    REPORTER_ASSERT(reporter, shader);
    REPORTER_ASSERT(reporter, !shader->isOpaque());
    shader->unref();

    // test 3: explicitly opaque
    bmp.setAlphaType(kOpaque_SkAlphaType);
    shader = SkShader::CreateBitmapShader(bmp,
        SkShader::kClamp_TileMode, SkShader::kClamp_TileMode);
    REPORTER_ASSERT(reporter, shader);
    REPORTER_ASSERT(reporter, shader->isOpaque());
    shader->unref();

    // test 4: explicitly not opaque
    bmp.setAlphaType(kPremul_SkAlphaType);
    shader = SkShader::CreateBitmapShader(bmp,
        SkShader::kClamp_TileMode, SkShader::kClamp_TileMode);
    REPORTER_ASSERT(reporter, shader);
    REPORTER_ASSERT(reporter, !shader->isOpaque());
    shader->unref();

}

static void test_gradient(skiatest::Reporter* reporter)
{
    SkPoint pts[2];
    pts[0].iset(0, 0);
    pts[1].iset(1, 0);
    SkColor colors[2];
    SkScalar pos[2] = {SkIntToScalar(0), SkIntToScalar(1)};
    int count = 2;
    SkShader::TileMode mode = SkShader::kClamp_TileMode;

    // test 1: all opaque
    colors[0] = SkColorSetARGB(0xFF, 0, 0, 0);
    colors[1] = SkColorSetARGB(0xFF, 0, 0, 0);
    SkShader* grad = SkGradientShader::CreateLinear(pts, colors, pos, count,
                                                    mode);
    REPORTER_ASSERT(reporter, grad);
    REPORTER_ASSERT(reporter, grad->isOpaque());
    grad->unref();

    // test 2: all 0 alpha
    colors[0] = SkColorSetARGB(0, 0, 0, 0);
    colors[1] = SkColorSetARGB(0, 0, 0, 0);
    grad = SkGradientShader::CreateLinear(pts, colors, pos, count, mode);
    REPORTER_ASSERT(reporter, grad);
    REPORTER_ASSERT(reporter, !grad->isOpaque());
    grad->unref();

    // test 3: one opaque, one transparent
    colors[0] = SkColorSetARGB(0xFF, 0, 0, 0);
    colors[1] = SkColorSetARGB(0x40, 0, 0, 0);
    grad = SkGradientShader::CreateLinear(pts, colors, pos, count, mode);
    REPORTER_ASSERT(reporter, grad);
    REPORTER_ASSERT(reporter, !grad->isOpaque());
    grad->unref();

    // test 4: test 3, swapped
    colors[0] = SkColorSetARGB(0x40, 0, 0, 0);
    colors[1] = SkColorSetARGB(0xFF, 0, 0, 0);
    grad = SkGradientShader::CreateLinear(pts, colors, pos, count, mode);
    REPORTER_ASSERT(reporter, grad);
    REPORTER_ASSERT(reporter, !grad->isOpaque());
    grad->unref();
}

static void test_color(skiatest::Reporter* reporter)
{
    SkColorShader colorShader1(SkColorSetARGB(0,0,0,0));
    REPORTER_ASSERT(reporter, !colorShader1.isOpaque());
    SkColorShader colorShader2(SkColorSetARGB(0xFF,0,0,0));
    REPORTER_ASSERT(reporter, colorShader2.isOpaque());
    SkColorShader colorShader3(SkColorSetARGB(0x7F,0,0,0));
    REPORTER_ASSERT(reporter, !colorShader3.isOpaque());

    // with inherrited color, shader must declare itself as opaque,
    // since lack of opacity will depend solely on the paint
    SkColorShader colorShader4;
    REPORTER_ASSERT(reporter, colorShader4.isOpaque());
}

DEF_TEST(ShaderOpacity, reporter) {
    test_gradient(reporter);
    test_color(reporter);
    test_bitmap(reporter);
}
