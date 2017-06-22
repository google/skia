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
#include "GrTest.h"
#include "Test.h"
#include "vk/GrVkGpu.h"

using sk_gpu_test::GrContextFactory;

bool does_full_buffer_contain_correct_color(GrColor* buffer,
                                            GrColor clearColor,
                                            GrPixelConfig config,
                                            int width,
                                            int height) {
    GrColor matchColor;
    if (kRGBA_8888_GrPixelConfig == config) {
        matchColor = clearColor;
    } else if (kBGRA_8888_GrPixelConfig) {
        // Hack to flip the R, B componets in the GrColor so that the comparrison will work below
        matchColor = GrColorPackRGBA(GrColorUnpackB(clearColor),
                                     GrColorUnpackG(clearColor),
                                     GrColorUnpackR(clearColor),
                                     GrColorUnpackA(clearColor));
    } else {
        // currently only supporting rgba_8888 and bgra_8888
        return false;
    }

    for (int j = 0; j < height; ++j) {
        for (int i = 0; i < width; ++i) {
            if (buffer[j * width + i] != matchColor) {
                return false;
            }
        }
    }
    return true;
}

void basic_clear_test(skiatest::Reporter* reporter, GrContext* context, GrPixelConfig config) {
#if 0
    GrVkGpu* gpu = static_cast<GrVkGpu*>(context->getGpu());
    SkAutoTMalloc<GrColor> buffer(25);

    GrSurfaceDesc surfDesc;
    surfDesc.fFlags = kRenderTarget_GrSurfaceFlag;
    surfDesc.fOrigin = kTopLeft_GrSurfaceOrigin;
    surfDesc.fWidth = 5;
    surfDesc.fHeight = 5;
    surfDesc.fConfig = config;
    surfDesc.fSampleCnt = 0;
    GrTexture* tex = gpu->createTexture(surfDesc, SkBudgeted::kNo);
    SkASSERT(tex);
    SkASSERT(tex->asRenderTarget());
    SkIRect rect = SkIRect::MakeWH(5, 5);

    gpu->clear(rect, GrColor_TRANSPARENT_BLACK, tex->asRenderTarget());

    gpu->readPixels(tex, 0, 0, 5, 5, config, (void*)buffer.get(), 0);

    REPORTER_ASSERT(reporter, does_full_buffer_contain_correct_color(buffer.get(),
                                                                     GrColor_TRANSPARENT_BLACK,
                                                                     config,
                                                                     5,
                                                                     5));

    gpu->clear(rect, GrColor_WHITE, tex->asRenderTarget());

    gpu->readPixels(tex, 0, 0, 5, 5, config, (void*)buffer.get(), 0);

    REPORTER_ASSERT(reporter, does_full_buffer_contain_correct_color(buffer.get(),
                                                                     GrColor_WHITE,
                                                                     config,
                                                                     5,
                                                                     5));

    GrColor myColor = GrColorPackRGBA(0xFF, 0x7F, 0x40, 0x20);

    gpu->clear(rect, myColor, tex->asRenderTarget());

    gpu->readPixels(tex, 0, 0, 5, 5, config, (void*)buffer.get(), 0);

    REPORTER_ASSERT(reporter, does_full_buffer_contain_correct_color(buffer.get(),
                                                                     myColor,
                                                                     config,
                                                                     5,
                                                                     5));
#endif
}

void sub_clear_test(skiatest::Reporter* reporter, GrContext* context, GrPixelConfig config) {
#if 0
    const int width = 10;
    const int height = 10;
    const int subWidth = width/2;
    const int subHeight = height/2;
    GrVkGpu* gpu = static_cast<GrVkGpu*>(context->getGpu());
    SkAutoTMalloc<GrColor> buffer(width * height);
    SkAutoTMalloc<GrColor> subBuffer(subWidth * subHeight);

    GrSurfaceDesc surfDesc;
    surfDesc.fFlags = kRenderTarget_GrSurfaceFlag;
    surfDesc.fOrigin = kTopLeft_GrSurfaceOrigin;
    surfDesc.fWidth = width;
    surfDesc.fHeight = height;
    surfDesc.fConfig = config;
    surfDesc.fSampleCnt = 0;
    GrTexture* tex = gpu->createTexture(surfDesc, SkBudgeted::kNo);
    SkASSERT(tex);
    SkASSERT(tex->asRenderTarget());

    SkIRect fullRect = SkIRect::MakeWH(10, 10);
    gpu->clear(fullRect, GrColor_TRANSPARENT_BLACK, tex->asRenderTarget());

    gpu->readPixels(tex, 0, 0, width, height, config, (void*)buffer.get(), 0);

    REPORTER_ASSERT(reporter, does_full_buffer_contain_correct_color(buffer.get(),
                                                                     GrColor_TRANSPARENT_BLACK,
                                                                     config,
                                                                     width,
                                                                     height));
    SkIRect rect;
    rect = SkIRect::MakeXYWH(0, 0, subWidth, subHeight);
    gpu->clear(rect, GrColor_WHITE, tex->asRenderTarget());
    rect = SkIRect::MakeXYWH(subWidth, 0, subWidth, subHeight);
    gpu->clear(rect, GrColor_WHITE, tex->asRenderTarget());
    rect = SkIRect::MakeXYWH(0, subHeight, subWidth, subHeight);
    gpu->clear(rect, GrColor_WHITE, tex->asRenderTarget());

    // Should fail since bottom right sub area has not been cleared to white
    gpu->readPixels(tex, 0, 0, width, height, config, (void*)buffer.get(), 0);
    REPORTER_ASSERT(reporter, !does_full_buffer_contain_correct_color(buffer.get(),
                                                                      GrColor_WHITE,
                                                                      config,
                                                                      width,
                                                                      height));

    rect = SkIRect::MakeXYWH(subWidth, subHeight, subWidth, subHeight);
    gpu->clear(rect, GrColor_WHITE, tex->asRenderTarget());

    gpu->readPixels(tex, 0, 0, width, height, config, (void*)buffer.get(), 0);
    REPORTER_ASSERT(reporter, does_full_buffer_contain_correct_color(buffer.get(),
                                                                     GrColor_WHITE,
                                                                     config,
                                                                     width,
                                                                     height));

    // Try different colors and that each sub area has correct color
    GrColor subColor1 = GrColorPackRGBA(0xFF, 0x00, 0x00, 0xFF);
    GrColor subColor2 = GrColorPackRGBA(0x00, 0xFF, 0x00, 0xFF);
    GrColor subColor3 = GrColorPackRGBA(0x00, 0x00, 0xFF, 0xFF);
    GrColor subColor4 = GrColorPackRGBA(0xFF, 0xFF, 0x00, 0xFF);

    rect = SkIRect::MakeXYWH(0, 0, subWidth, subHeight);
    gpu->clear(rect, subColor1, tex->asRenderTarget());
    rect = SkIRect::MakeXYWH(subWidth, 0, subWidth, subHeight);
    gpu->clear(rect, subColor2, tex->asRenderTarget());
    rect = SkIRect::MakeXYWH(0, subHeight, subWidth, subHeight);
    gpu->clear(rect, subColor3, tex->asRenderTarget());
    rect = SkIRect::MakeXYWH(subWidth, subHeight, subWidth, subHeight);
    gpu->clear(rect, subColor4, tex->asRenderTarget());

    gpu->readPixels(tex, 0, 0, subWidth, subHeight, config, (void*)subBuffer.get(), 0);
    REPORTER_ASSERT(reporter, does_full_buffer_contain_correct_color(subBuffer.get(),
                                                                     subColor1,
                                                                     config,
                                                                     subWidth,
                                                                     subHeight));
    gpu->readPixels(tex, subWidth, 0, subWidth, subHeight, config, (void*)subBuffer.get(), 0);
    REPORTER_ASSERT(reporter, does_full_buffer_contain_correct_color(subBuffer.get(),
                                                                     subColor2,
                                                                     config,
                                                                     subWidth,
                                                                     subHeight));
    gpu->readPixels(tex, 0, subHeight, subWidth, subHeight, config, (void*)subBuffer.get(), 0);
    REPORTER_ASSERT(reporter, does_full_buffer_contain_correct_color(subBuffer.get(),
                                                                     subColor3,
                                                                     config,
                                                                     subWidth,
                                                                     subHeight));
    gpu->readPixels(tex, subWidth, subHeight, subWidth, subHeight,
                    config, (void*)subBuffer.get(), 0);
    REPORTER_ASSERT(reporter, does_full_buffer_contain_correct_color(subBuffer.get(),
                                                                     subColor4,
                                                                     config,
                                                                     subWidth,
                                                                     subHeight));
#endif
}

DEF_GPUTEST_FOR_VULKAN_CONTEXT(VkClearTests, reporter, ctxInfo) {
    basic_clear_test(reporter, ctxInfo.grContext(), kRGBA_8888_GrPixelConfig);
    basic_clear_test(reporter, ctxInfo.grContext(), kBGRA_8888_GrPixelConfig);
    sub_clear_test(reporter, ctxInfo.grContext(), kRGBA_8888_GrPixelConfig);
    sub_clear_test(reporter, ctxInfo.grContext(), kBGRA_8888_GrPixelConfig);
}

#endif
