/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a GPU-backend specific test. It relies on static intializers to work

#include "SkTypes.h"

#if SK_SUPPORT_GPU && defined(SK_VULKAN)

#include "GrContextFactory.h"
#include "GrContextPriv.h"
#include "GrSurfaceProxy.h"
#include "GrTest.h"
#include "SkGr.h"
#include "Test.h"
#include "vk/GrVkGpu.h"

using sk_gpu_test::GrContextFactory;

void fill_pixel_data(int width, int height, GrColor* data) {

    // build red-green gradient
    for (int j = 0; j < height; ++j) {
        for (int i = 0; i < width; ++i) {
            unsigned int red = (unsigned int)(256.f*(i / (float)width));
            unsigned int green = (unsigned int)(256.f*(j / (float)height));
            data[i + j*width] = GrColorPackRGBA(red - (red>>8), green - (green>>8), 0xff, 0xff);
        }
    }
}

bool does_full_buffer_contain_correct_color(GrColor* srcBuffer,
                                            GrColor* dstBuffer,
                                            int width,
                                            int height) {
    GrColor* srcPtr = srcBuffer;
    GrColor* dstPtr = dstBuffer;
    for (int j = 0; j < height; ++j) {
        for (int i = 0; i < width; ++i) {
            if (srcPtr[i] != dstPtr[i]) {
                return false;
            }
        }
        srcPtr += width;
        dstPtr += width;
    }
    return true;
}

void basic_texture_test(skiatest::Reporter* reporter, GrContext* context, GrPixelConfig config,
                        bool renderTarget) {
    const int kWidth = 16;
    const int kHeight = 16;
    SkAutoTMalloc<GrColor> srcBuffer(kWidth*kHeight);
    SkAutoTMalloc<GrColor> dstBuffer(kWidth*kHeight);

    fill_pixel_data(kWidth, kHeight, srcBuffer.get());

    GrSurfaceDesc surfDesc;
    surfDesc.fFlags = renderTarget ? kRenderTarget_GrSurfaceFlag : kNone_GrSurfaceFlags;
    surfDesc.fOrigin = kTopLeft_GrSurfaceOrigin;
    surfDesc.fWidth = kWidth;
    surfDesc.fHeight = kHeight;
    surfDesc.fConfig = config;
    surfDesc.fSampleCnt = 0;

    SkColorType ct;
    SkAssertResult(GrPixelConfigToColorType(config, &ct));

    sk_sp<GrTextureProxy> proxy = GrSurfaceProxy::MakeDeferred(context->resourceProvider(),
                                                               surfDesc, SkBudgeted::kNo,
                                                               srcBuffer, 0);
    REPORTER_ASSERT(reporter, proxy);
    if (proxy) {
        sk_sp<GrSurfaceContext> sContext = context->contextPriv().makeWrappedSurfaceContext(
                                                                                proxy, nullptr);

        SkImageInfo dstInfo = SkImageInfo::Make(kWidth, kHeight, ct, kOpaque_SkAlphaType);

        bool result = sContext->readPixels(dstInfo, dstBuffer, 0, 0, 0);
        REPORTER_ASSERT(reporter, result);
        REPORTER_ASSERT(reporter, does_full_buffer_contain_correct_color(srcBuffer,
                                                                         dstBuffer,
                                                                         kWidth,
                                                                         kHeight));

        dstInfo = SkImageInfo::Make(10, 2, ct, kOpaque_SkAlphaType);
        result = sContext->writePixels(dstInfo, srcBuffer, 0, 2, 10);
        REPORTER_ASSERT(reporter, result);

        memset(dstBuffer, 0, kWidth*kHeight*sizeof(GrColor));

        result = sContext->readPixels(dstInfo, dstBuffer, 0, 2, 10);
        REPORTER_ASSERT(reporter, result);

        REPORTER_ASSERT(reporter, does_full_buffer_contain_correct_color(srcBuffer,
                                                                         dstBuffer,
                                                                         10,
                                                                         2));
    }

    surfDesc.fOrigin = kBottomLeft_GrSurfaceOrigin;

    proxy = GrSurfaceProxy::MakeDeferred(context->resourceProvider(),
                                         surfDesc, SkBudgeted::kNo,
                                         srcBuffer, 0);
    REPORTER_ASSERT(reporter, proxy);
    if (proxy) {
        sk_sp<GrSurfaceContext> sContext = context->contextPriv().makeWrappedSurfaceContext(
                                                                                proxy, nullptr);

        SkImageInfo dstInfo = SkImageInfo::Make(kWidth, kHeight, ct, kOpaque_SkAlphaType);

        bool result = sContext->readPixels(dstInfo, dstBuffer, 0, 0, 0);
        REPORTER_ASSERT(reporter, result);
        REPORTER_ASSERT(reporter, does_full_buffer_contain_correct_color(srcBuffer,
                                                                         dstBuffer,
                                                                         kWidth,
                                                                         kHeight));

        dstInfo = SkImageInfo::Make(4, 5, ct, kOpaque_SkAlphaType);
        result = sContext->writePixels(dstInfo, srcBuffer, 0, 5, 4);
        REPORTER_ASSERT(reporter, result);

        memset(dstBuffer, 0, kWidth*kHeight*sizeof(GrColor));

        result = sContext->readPixels(dstInfo, dstBuffer, 0, 5, 4);
        REPORTER_ASSERT(reporter, result);

        REPORTER_ASSERT(reporter, does_full_buffer_contain_correct_color(srcBuffer,
                                                                         dstBuffer,
                                                                         4,
                                                                         5));

    }
}

DEF_GPUTEST_FOR_VULKAN_CONTEXT(VkUploadPixelsTests, reporter, ctxInfo) {
    // RGBA
    basic_texture_test(reporter, ctxInfo.grContext(), kRGBA_8888_GrPixelConfig, false);
    basic_texture_test(reporter, ctxInfo.grContext(), kRGBA_8888_GrPixelConfig, true);

    // BGRA
    basic_texture_test(reporter, ctxInfo.grContext(), kBGRA_8888_GrPixelConfig, false);
    basic_texture_test(reporter, ctxInfo.grContext(), kBGRA_8888_GrPixelConfig, true);
}

#endif
