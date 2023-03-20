/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkColor.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkTileMode.h"
#include "include/effects/SkGradientShader.h"
#include "tests/Test.h"

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
    sk_sp<SkShader> colorShader1 = SkShaders::Color(SkColorSetARGB(0,0,0,0));
    REPORTER_ASSERT(reporter, !colorShader1->isOpaque());
    sk_sp<SkShader> colorShader2 = SkShaders::Color(SkColorSetARGB(0xFF,0,0,0));
    REPORTER_ASSERT(reporter, colorShader2->isOpaque());
    sk_sp<SkShader> colorShader3 = SkShaders::Color(SkColorSetARGB(0x7F,0,0,0));
    REPORTER_ASSERT(reporter, !colorShader3->isOpaque());
}

DEF_TEST(ShaderOpacity, reporter) {
    test_gradient(reporter);
    test_color(reporter);
    test_bitmap(reporter);
}
