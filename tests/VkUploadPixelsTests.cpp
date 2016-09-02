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
                                            GrPixelConfig config,
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
                        bool renderTarget, bool linearTiling) {
    GrVkGpu* gpu = static_cast<GrVkGpu*>(context->getGpu());

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
        if (!caps->isConfigTexturableLinearly(config) ||
            (renderTarget && !caps->isConfigRenderableLinearly(config, false))) {
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
    surfDesc.fConfig = config;
    surfDesc.fSampleCnt = 0;
    GrTexture* tex0 = gpu->createTexture(surfDesc, SkBudgeted::kNo, srcBuffer, 0);
    if (tex0) {
        REPORTER_ASSERT(reporter, canCreate);
        gpu->readPixels(tex0, 0, 0, kWidth, kHeight, config, dstBuffer, 0);
        REPORTER_ASSERT(reporter, does_full_buffer_contain_correct_color(srcBuffer,
                                                                         dstBuffer,
                                                                         config,
                                                                         kWidth,
                                                                         kHeight));

        tex0->writePixels(2, 10, 10, 2, config, srcBuffer);
        memset(dstBuffer, 0, kWidth*kHeight*sizeof(GrColor));
        gpu->readPixels(tex0, 2, 10, 10, 2, config, dstBuffer, 0);
        REPORTER_ASSERT(reporter, does_full_buffer_contain_correct_color(srcBuffer,
                                                                         dstBuffer,
                                                                         config,
                                                                         10,
                                                                         2));

        tex0->unref();
    } else {
        REPORTER_ASSERT(reporter, !canCreate);
    }

    surfDesc.fOrigin = kBottomLeft_GrSurfaceOrigin;
    GrTexture* tex1 = gpu->createTexture(surfDesc, SkBudgeted::kNo, srcBuffer, 0);
    if (tex1) {
        REPORTER_ASSERT(reporter, canCreate);
        gpu->readPixels(tex1, 0, 0, kWidth, kHeight, config, dstBuffer, 0);
        REPORTER_ASSERT(reporter, does_full_buffer_contain_correct_color(srcBuffer,
                                                                         dstBuffer,
                                                                         config,
                                                                         kWidth,
                                                                         kHeight));

        tex1->writePixels(5, 4, 4, 5, config, srcBuffer);
        memset(dstBuffer, 0, kWidth*kHeight*sizeof(GrColor));
        gpu->readPixels(tex1, 5, 4, 4, 5, config, dstBuffer, 0);
        REPORTER_ASSERT(reporter, does_full_buffer_contain_correct_color(srcBuffer,
                                                                         dstBuffer,
                                                                         config,
                                                                         4,
                                                                         5));

        tex1->unref();
    } else {
        REPORTER_ASSERT(reporter, !canCreate);
    }
}

DEF_GPUTEST_FOR_VULKAN_CONTEXT(VkUploadPixelsTests, reporter, ctxInfo) {
    basic_texture_test(reporter, ctxInfo.grContext(), kRGBA_8888_GrPixelConfig, false, false);
    basic_texture_test(reporter, ctxInfo.grContext(), kRGBA_8888_GrPixelConfig, true, false);
    basic_texture_test(reporter, ctxInfo.grContext(), kRGBA_8888_GrPixelConfig, false, true);
    basic_texture_test(reporter, ctxInfo.grContext(), kRGBA_8888_GrPixelConfig, true, true);
    basic_texture_test(reporter, ctxInfo.grContext(), kBGRA_8888_GrPixelConfig, false, false);
    basic_texture_test(reporter, ctxInfo.grContext(), kBGRA_8888_GrPixelConfig, true, false);
    basic_texture_test(reporter, ctxInfo.grContext(), kBGRA_8888_GrPixelConfig, false, true);
    basic_texture_test(reporter, ctxInfo.grContext(), kBGRA_8888_GrPixelConfig, true, true);
}

#endif
