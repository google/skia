/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a GPU-backend specific test. It relies on static intializers to work

#include "include/core/SkTypes.h"

#include "include/gpu/GrDirectContext.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrImageInfo.h"
#include "src/gpu/GrSurfaceContext.h"
#include "src/gpu/GrSurfaceProxy.h"
#include "src/gpu/SkGr.h"
#include "tests/Test.h"
#include "tests/TestUtils.h"
#include "tools/gpu/GrContextFactory.h"
#include "tools/gpu/ProxyUtils.h"

using sk_gpu_test::GrContextFactory;

void basic_texture_test(skiatest::Reporter* reporter, GrDirectContext* dContext, SkColorType ct,
                        GrRenderable renderable) {
    const int kWidth = 16;
    const int kHeight = 16;
    SkAutoTMalloc<GrColor> srcBuffer(kWidth*kHeight);
    SkAutoTMalloc<GrColor> dstBuffer(kWidth*kHeight);

    FillPixelData(kWidth, kHeight, srcBuffer.get());

    auto info = SkImageInfo::Make({kWidth, kHeight}, ct, kPremul_SkAlphaType, nullptr);
    auto view = sk_gpu_test::MakeTextureProxyViewFromData(dContext,
                                                          renderable,
                                                          kTopLeft_GrSurfaceOrigin,
                                                          info,
                                                          srcBuffer,
                                                          /*row bytes*/ 0);
    REPORTER_ASSERT(reporter, view);
    if (view) {
        auto sContext = GrSurfaceContext::Make(dContext, std::move(view), info.colorInfo());

        SkImageInfo dstInfo = SkImageInfo::Make(kWidth, kHeight, ct, kPremul_SkAlphaType);

        bool result = sContext->readPixels(dContext, dstInfo, dstBuffer, 0, {0, 0});
        REPORTER_ASSERT(reporter, result);
        REPORTER_ASSERT(reporter,
                        DoesFullBufferContainCorrectColor(srcBuffer, dstBuffer, kWidth, kHeight));

        dstInfo = SkImageInfo::Make(10, 2, ct, kPremul_SkAlphaType);
        result = sContext->writePixels(dContext, dstInfo, srcBuffer, 0, {2, 10});
        REPORTER_ASSERT(reporter, result);

        memset(dstBuffer, 0, kWidth*kHeight*sizeof(GrColor));

        result = sContext->readPixels(dContext, dstInfo, dstBuffer, 0, {2, 10});
        REPORTER_ASSERT(reporter, result);

        REPORTER_ASSERT(reporter, DoesFullBufferContainCorrectColor(srcBuffer, dstBuffer, 10, 2));
    }

    view = sk_gpu_test::MakeTextureProxyViewFromData(dContext,
                                                     renderable,
                                                     kBottomLeft_GrSurfaceOrigin,
                                                     info,
                                                     srcBuffer,
                                                     0);
    REPORTER_ASSERT(reporter, view);
    if (view) {
        auto sContext = GrSurfaceContext::Make(dContext, std::move(view), info.colorInfo());

        SkImageInfo dstInfo = SkImageInfo::Make(kWidth, kHeight, ct, kPremul_SkAlphaType);

        bool result = sContext->readPixels(dContext, dstInfo, dstBuffer, 0, {0, 0});
        REPORTER_ASSERT(reporter, result);
        REPORTER_ASSERT(reporter,
                        DoesFullBufferContainCorrectColor(srcBuffer, dstBuffer, kWidth, kHeight));

        dstInfo = SkImageInfo::Make(4, 5, ct, kPremul_SkAlphaType);
        result = sContext->writePixels(dContext, dstInfo, srcBuffer, 0, {5, 4});
        REPORTER_ASSERT(reporter, result);

        memset(dstBuffer, 0, kWidth*kHeight*sizeof(GrColor));

        result = sContext->readPixels(dContext, dstInfo, dstBuffer, 0, {5, 4});
        REPORTER_ASSERT(reporter, result);

        REPORTER_ASSERT(reporter, DoesFullBufferContainCorrectColor(srcBuffer, dstBuffer, 4, 5));
    }
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrUploadPixelsTests, reporter, ctxInfo) {
    auto direct = ctxInfo.directContext();

    // RGBA
    basic_texture_test(reporter, direct, kRGBA_8888_SkColorType, GrRenderable::kNo);
    basic_texture_test(reporter, direct, kRGBA_8888_SkColorType, GrRenderable::kYes);

    // BGRA
    basic_texture_test(reporter, direct, kBGRA_8888_SkColorType, GrRenderable::kNo);
    basic_texture_test(reporter, direct, kBGRA_8888_SkColorType, GrRenderable::kYes);
}
