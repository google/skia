/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a GPU-backend specific test. It relies on static intializers to work

#include "SkTypes.h"

#if SK_SUPPORT_GPU && SK_ALLOW_STATIC_GLOBAL_INITIALIZERS && defined(SK_VULKAN)

#include "GrContextFactory.h"
#include "GrContextPriv.h"
#include "GrSurfaceProxy.h"
#include "GrTest.h"
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
                SkDebugf("src %x dst %x at %d,%d", i, j);
                return false;
            }
        }
        srcPtr += width;
        dstPtr += width;
    }
    return true;
}

void basic_texture_test(skiatest::Reporter* reporter, GrContext* context,
                        bool renderTarget, bool linearTiling) {
    GrVkGpu* gpu = static_cast<GrVkGpu*>(context->getGpu());

    const GrPixelConfig kConfig = kRGBA_8888_GrPixelConfig;
    const int kWidth = 16;
    const int kHeight = 16;
    SkAutoTMalloc<GrColor> srcBuffer(kWidth*kHeight);
    SkAutoTMalloc<GrColor> dstBuffer(kWidth*kHeight);

    fill_pixel_data(kWidth, kHeight, srcBuffer.get());

    const GrVkCaps* caps = reinterpret_cast<const GrVkCaps*>(context->caps());

    bool canCreate = true;
    // the expectation is that the given config is texturable/renderable with optimal tiling
    // but may not be with linear tiling
    if (linearTiling) {
        if (!caps->isConfigTexturableLinearly(kConfig) ||
            (renderTarget && !caps->isConfigRenderableLinearly(kConfig, false))) {
            canCreate = false;
        }
    }

    GrSurfaceDesc surfDesc;
    surfDesc.fFlags = renderTarget ? kRenderTarget_GrSurfaceFlag : kNone_GrSurfaceFlags;
    if (linearTiling) {
        surfDesc.fFlags |= kZeroCopy_GrSurfaceFlag;
    }
    surfDesc.fOrigin = kTopLeft_GrSurfaceOrigin;
    surfDesc.fWidth = kWidth;
    surfDesc.fHeight = kHeight;
    surfDesc.fConfig = kConfig;
    surfDesc.fSampleCnt = 0;

    sk_sp<GrTextureProxy> proxy0 = GrSurfaceProxy::MakeDeferred(context->resourceProvider(),
                                                                surfDesc, SkBudgeted::kNo,
                                                                srcBuffer, 0);

    if (proxy0) {
        REPORTER_ASSERT(reporter, canCreate);

        sk_sp<GrSurfaceContext> sContext = context->contextPriv().makeWrappedSurfaceContext(
                                                                                proxy0, nullptr);

        SkImageInfo dstInfo = SkImageInfo::Make(kWidth, kHeight,
                                                kRGBA_8888_SkColorType, kOpaque_SkAlphaType);

        bool result = sContext->readPixels(dstInfo, dstBuffer, 0, 0, 0);
        REPORTER_ASSERT(reporter, result);

        REPORTER_ASSERT(reporter, does_full_buffer_contain_correct_color(srcBuffer,
                                                                         dstBuffer,
                                                                         kWidth,
                                                                         kHeight));

        dstInfo = SkImageInfo::Make(10, 2, kRGBA_8888_SkColorType, kOpaque_SkAlphaType);
        result = sContext->writePixels(dstInfo, srcBuffer, 0, 2, 10);
        REPORTER_ASSERT(reporter, result);

        memset(dstBuffer, 0, kWidth*kHeight*sizeof(GrColor));

        result = sContext->readPixels(dstInfo, dstBuffer, 0, 2, 10);
        REPORTER_ASSERT(reporter, result);

        REPORTER_ASSERT(reporter, does_full_buffer_contain_correct_color(srcBuffer,
                                                                         dstBuffer,
                                                                         10,
                                                                         2));
    } else {
        REPORTER_ASSERT(reporter, !canCreate);
    }

    surfDesc.fOrigin = kBottomLeft_GrSurfaceOrigin;

    sk_sp<GrTextureProxy> proxy1 = GrSurfaceProxy::MakeDeferred(context->resourceProvider(),
                                                                surfDesc, SkBudgeted::kNo,
                                                                srcBuffer, 0);
    if (proxy1) {
        REPORTER_ASSERT(reporter, canCreate);

        sk_sp<GrSurfaceContext> sContext = context->contextPriv().makeWrappedSurfaceContext(
                                                                                proxy0, nullptr);

        SkImageInfo dstInfo = SkImageInfo::Make(kWidth, kHeight,
                                                kRGBA_8888_SkColorType, kOpaque_SkAlphaType);

        bool result = sContext->readPixels(dstInfo, dstBuffer, 0, 0, 0);
        REPORTER_ASSERT(reporter, result);
        REPORTER_ASSERT(reporter, does_full_buffer_contain_correct_color(srcBuffer,
                                                                         dstBuffer,
                                                                         kWidth,
                                                                         kHeight));

        dstInfo = SkImageInfo::Make(4, 5, kRGBA_8888_SkColorType, kOpaque_SkAlphaType);
        result = sContext->writePixels(dstInfo, srcBuffer, 0, 5, 4);
        REPORTER_ASSERT(reporter, result);

        memset(dstBuffer, 0, kWidth*kHeight*sizeof(GrColor));

        result = sContext->readPixels(dstInfo, dstBuffer, 0, 5, 4);
        REPORTER_ASSERT(reporter, result);

        REPORTER_ASSERT(reporter, does_full_buffer_contain_correct_color(srcBuffer,
                                                                         dstBuffer,
                                                                         4,
                                                                         5));

    } else {
        REPORTER_ASSERT(reporter, !canCreate);
    }
}

DEF_GPUTEST_FOR_VULKAN_CONTEXT(VkUploadPixelsTests, reporter, ctxInfo) {
    basic_texture_test(reporter, ctxInfo.grContext(), false, false);
    basic_texture_test(reporter, ctxInfo.grContext(), true, false);
    basic_texture_test(reporter, ctxInfo.grContext(), false, true);
    basic_texture_test(reporter, ctxInfo.grContext(), true, true);
    basic_texture_test(reporter, ctxInfo.grContext(), false, false);
    basic_texture_test(reporter, ctxInfo.grContext(), true, false);
    basic_texture_test(reporter, ctxInfo.grContext(), false, true);
    basic_texture_test(reporter, ctxInfo.grContext(), true, true);
}

#endif
