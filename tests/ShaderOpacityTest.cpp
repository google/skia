/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorShader.h"
#include "SkGradientShader.h"
#include "SkShader.h"
#include "Test.h"

static void test_bitmap(skiatest::Reporter* reporter) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(2, 2);

    SkBitmap bmp;
    bmp.setInfo(info);

    // test 1: bitmap without pixel data
    auto shader = SkShader::MakeBitmapShader(bmp,
        SkShader::kClamp_TileMode, SkShader::kClamp_TileMode);
    REPORTER_ASSERT(reporter, shader);
    REPORTER_ASSERT(reporter, !shader->isOpaque());

    // From this point on, we have pixels
    bmp.allocPixels(info);

    // test 2: not opaque by default
    shader = SkShader::MakeBitmapShader(bmp,
        SkShader::kClamp_TileMode, SkShader::kClamp_TileMode);
    REPORTER_ASSERT(reporter, shader);
    REPORTER_ASSERT(reporter, !shader->isOpaque());

    // test 3: explicitly opaque
    bmp.setAlphaType(kOpaque_SkAlphaType);
    shader = SkShader::MakeBitmapShader(bmp,
        SkShader::kClamp_TileMode, SkShader::kClamp_TileMode);
    REPORTER_ASSERT(reporter, shader);
    REPORTER_ASSERT(reporter, shader->isOpaque());

    // test 4: explicitly not opaque
    bmp.setAlphaType(kPremul_SkAlphaType);
    shader = SkShader::MakeBitmapShader(bmp,
        SkShader::kClamp_TileMode, SkShader::kClamp_TileMode);
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
    SkShader::TileMode mode = SkShader::kClamp_TileMode;

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
