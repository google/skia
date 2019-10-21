/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a GPU-backend specific test. It relies on static intializers to work

#include "include/core/SkTypes.h"

#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrImageInfo.h"
#include "src/gpu/GrSurfaceProxy.h"
#include "src/gpu/SkGr.h"
#include "tests/Test.h"
#include "tests/TestUtils.h"
#include "tools/gpu/GrContextFactory.h"
#include "tools/gpu/ProxyUtils.h"

using sk_gpu_test::GrContextFactory;

void basic_texture_test(skiatest::Reporter* reporter, GrContext* context, SkColorType ct,
                        GrRenderable renderable) {
    const int kWidth = 16;
    const int kHeight = 16;
    SkAutoTMalloc<GrColor> srcBuffer(kWidth*kHeight);
    SkAutoTMalloc<GrColor> dstBuffer(kWidth*kHeight);

    fill_pixel_data(kWidth, kHeight, srcBuffer.get());

    auto grCT = SkColorTypeToGrColorType(ct);
    auto proxy = sk_gpu_test::MakeTextureProxyFromData(
            context, renderable, kTopLeft_GrSurfaceOrigin,
            {grCT, kPremul_SkAlphaType, nullptr, kWidth, kHeight}, srcBuffer, 0);
    REPORTER_ASSERT(reporter, proxy);
    if (proxy) {
        auto sContext = context->priv().makeWrappedSurfaceContext(
                proxy, SkColorTypeToGrColorType(ct), kPremul_SkAlphaType);

        SkImageInfo dstInfo = SkImageInfo::Make(kWidth, kHeight, ct, kPremul_SkAlphaType);

        bool result = sContext->readPixels(dstInfo, dstBuffer, 0, {0, 0});
        REPORTER_ASSERT(reporter, result);
        REPORTER_ASSERT(reporter, does_full_buffer_contain_correct_color(srcBuffer,
                                                                         dstBuffer,
                                                                         kWidth,
                                                                         kHeight));

        dstInfo = SkImageInfo::Make(10, 2, ct, kPremul_SkAlphaType);
        result = sContext->writePixels(dstInfo, srcBuffer, 0, {2, 10});
        REPORTER_ASSERT(reporter, result);

        memset(dstBuffer, 0, kWidth*kHeight*sizeof(GrColor));

        result = sContext->readPixels(dstInfo, dstBuffer, 0, {2, 10});
        REPORTER_ASSERT(reporter, result);

        REPORTER_ASSERT(reporter, does_full_buffer_contain_correct_color(srcBuffer,
                                                                         dstBuffer,
                                                                         10,
                                                                         2));
    }

    proxy = sk_gpu_test::MakeTextureProxyFromData(
            context, renderable, kBottomLeft_GrSurfaceOrigin,
            {grCT, kPremul_SkAlphaType, nullptr, kWidth, kHeight}, srcBuffer, 0);
    REPORTER_ASSERT(reporter, proxy);
    if (proxy) {
        auto sContext = context->priv().makeWrappedSurfaceContext(
                proxy, SkColorTypeToGrColorType(ct), kPremul_SkAlphaType);

        SkImageInfo dstInfo = SkImageInfo::Make(kWidth, kHeight, ct, kPremul_SkAlphaType);

        bool result = sContext->readPixels(dstInfo, dstBuffer, 0, {0, 0});
        REPORTER_ASSERT(reporter, result);
        REPORTER_ASSERT(reporter, does_full_buffer_contain_correct_color(srcBuffer,
                                                                         dstBuffer,
                                                                         kWidth,
                                                                         kHeight));

        dstInfo = SkImageInfo::Make(4, 5, ct, kPremul_SkAlphaType);
        result = sContext->writePixels(dstInfo, srcBuffer, 0, {5, 4});
        REPORTER_ASSERT(reporter, result);

        memset(dstBuffer, 0, kWidth*kHeight*sizeof(GrColor));

        result = sContext->readPixels(dstInfo, dstBuffer, 0, {5, 4});
        REPORTER_ASSERT(reporter, result);

        REPORTER_ASSERT(reporter, does_full_buffer_contain_correct_color(srcBuffer,
                                                                         dstBuffer,
                                                                         4,
                                                                         5));

    }
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrUploadPixelsTests, reporter, ctxInfo) {
    // RGBA
    basic_texture_test(reporter, ctxInfo.grContext(), kRGBA_8888_SkColorType, GrRenderable::kNo);
    basic_texture_test(reporter, ctxInfo.grContext(), kRGBA_8888_SkColorType, GrRenderable::kYes);

    // BGRA
    basic_texture_test(reporter, ctxInfo.grContext(), kBGRA_8888_SkColorType, GrRenderable::kNo);
    basic_texture_test(reporter, ctxInfo.grContext(), kBGRA_8888_SkColorType, GrRenderable::kYes);
}
