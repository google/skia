/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a GPU-backend specific test. It relies on static intializers to work

#include "include/core/SkTypes.h"

#if defined(SK_VULKAN)

#include "include/gpu/vk/GrVkVulkan.h"

#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrRenderTarget.h"
#include "include/gpu/GrTexture.h"
#include "src/gpu/GrContextPriv.h"
#include "tools/gpu/GrContextFactory.h"

#include "include/gpu/vk/GrVkTypes.h"
#include "src/gpu/vk/GrVkCaps.h"
#include "src/gpu/vk/GrVkGpu.h"
#include "src/gpu/vk/GrVkMemory.h"
#include "tests/Test.h"

using sk_gpu_test::GrContextFactory;

const int kW = 1024;
const int kH = 1024;
const GrPixelConfig kPixelConfig = kRGBA_8888_GrPixelConfig;
const SkColorType kColorType = SkColorType::kRGBA_8888_SkColorType;

void wrap_tex_test(skiatest::Reporter* reporter, GrContext* context) {

    GrGpu* gpu = context->priv().getGpu();

    GrBackendTexture origBackendTex = context->createBackendTexture(kW, kH,
                                                                    kColorType,
                                                                    SkColors::kTransparent,
                                                                    GrMipMapped::kNo,
                                                                    GrRenderable::kNo,
                                                                    GrProtected::kNo);
    GrVkImageInfo imageInfo;
    SkAssertResult(origBackendTex.getVkImageInfo(&imageInfo));

    sk_sp<GrTexture> tex = gpu->wrapBackendTexture(origBackendTex, kBorrow_GrWrapOwnership,
                                                   GrWrapCacheable::kNo, kRead_GrIOType);
    REPORTER_ASSERT(reporter, tex);

    // image is null
    {
        GrVkImageInfo backendCopy = imageInfo;
        backendCopy.fImage = VK_NULL_HANDLE;
        GrBackendTexture backendTex = GrBackendTexture(kW, kH, backendCopy);
        backendTex.setPixelConfig(kPixelConfig);
        tex = gpu->wrapBackendTexture(backendTex, kBorrow_GrWrapOwnership, GrWrapCacheable::kNo,
                                      kRead_GrIOType);
        REPORTER_ASSERT(reporter, !tex);
        tex = gpu->wrapBackendTexture(backendTex, kAdopt_GrWrapOwnership, GrWrapCacheable::kNo,
                                      kRead_GrIOType);
        REPORTER_ASSERT(reporter, !tex);
    }

    // alloc is null
    {
        GrVkImageInfo backendCopy = imageInfo;
        backendCopy.fAlloc = GrVkAlloc();
        GrBackendTexture backendTex = GrBackendTexture(kW, kH, backendCopy);
        backendTex.setPixelConfig(kPixelConfig);
        tex = gpu->wrapBackendTexture(backendTex, kBorrow_GrWrapOwnership, GrWrapCacheable::kNo,
                                      kRead_GrIOType);
        REPORTER_ASSERT(reporter, tex);
        tex = gpu->wrapBackendTexture(backendTex, kAdopt_GrWrapOwnership, GrWrapCacheable::kNo,
                                      kRead_GrIOType);
        REPORTER_ASSERT(reporter, !tex);
    }

    // check adopt creation
    {
        GrVkImageInfo backendCopy = imageInfo;
        GrBackendTexture backendTex = GrBackendTexture(kW, kH, backendCopy);
        backendTex.setPixelConfig(kPixelConfig);
        tex = gpu->wrapBackendTexture(backendTex, kAdopt_GrWrapOwnership, GrWrapCacheable::kNo,
                                      kRead_GrIOType);

        REPORTER_ASSERT(reporter, tex);
    }
}

void wrap_rt_test(skiatest::Reporter* reporter, GrContext* context) {
    GrGpu* gpu = context->priv().getGpu();

    GrBackendTexture origBackendTex = context->createBackendTexture(kW, kH,
                                                                    kColorType,
                                                                    SkColors::kTransparent,
                                                                    GrMipMapped::kNo,
                                                                    GrRenderable::kYes,
                                                                    GrProtected::kNo);

    GrVkImageInfo imageInfo;
    SkAssertResult(origBackendTex.getVkImageInfo(&imageInfo));

    GrBackendRenderTarget origBackendRT(kW, kH, 1, 0, imageInfo);
    origBackendRT.setPixelConfig(kPixelConfig);

    sk_sp<GrRenderTarget> rt = gpu->wrapBackendRenderTarget(origBackendRT);
    REPORTER_ASSERT(reporter, rt);

    // image is null
    {
        GrVkImageInfo backendCopy = imageInfo;
        backendCopy.fImage = VK_NULL_HANDLE;
        GrBackendRenderTarget backendRT(kW, kH, 1, 0, backendCopy);
        backendRT.setPixelConfig(kPixelConfig);
        rt = gpu->wrapBackendRenderTarget(backendRT);
        REPORTER_ASSERT(reporter, !rt);
    }

    // alloc is null
    {
        GrVkImageInfo backendCopy = imageInfo;
        backendCopy.fAlloc = GrVkAlloc();
        // can wrap null alloc
        GrBackendRenderTarget backendRT(kW, kH, 1, 0, backendCopy);
        backendRT.setPixelConfig(kPixelConfig);
        rt = gpu->wrapBackendRenderTarget(backendRT);
        REPORTER_ASSERT(reporter, rt);
    }

    // When we wrapBackendRenderTarget it is always borrowed, so we must make sure to free the
    // resource when we're done.
    context->deleteBackendTexture(origBackendTex);
}

void wrap_trt_test(skiatest::Reporter* reporter, GrContext* context) {
    GrGpu* gpu = context->priv().getGpu();

    GrBackendTexture origBackendTex = context->createBackendTexture(kW, kH,
                                                                    kColorType,
                                                                    SkColors::kTransparent,
                                                                    GrMipMapped::kNo,
                                                                    GrRenderable::kYes,
                                                                    GrProtected::kNo);
    GrVkImageInfo imageInfo;
    SkAssertResult(origBackendTex.getVkImageInfo(&imageInfo));

    sk_sp<GrTexture> tex = gpu->wrapRenderableBackendTexture(
            origBackendTex, 1, kBorrow_GrWrapOwnership, GrWrapCacheable::kNo);
    REPORTER_ASSERT(reporter, tex);

    // image is null
    {
        GrVkImageInfo backendCopy = imageInfo;
        backendCopy.fImage = VK_NULL_HANDLE;
        GrBackendTexture backendTex = GrBackendTexture(kW, kH, backendCopy);
        backendTex.setPixelConfig(kPixelConfig);
        tex = gpu->wrapRenderableBackendTexture(backendTex, 1, kBorrow_GrWrapOwnership,
                                                GrWrapCacheable::kNo);
        REPORTER_ASSERT(reporter, !tex);
        tex = gpu->wrapRenderableBackendTexture(backendTex, 1, kAdopt_GrWrapOwnership,
                                                GrWrapCacheable::kNo);
        REPORTER_ASSERT(reporter, !tex);
    }

    // alloc is null
    {
        GrVkImageInfo backendCopy = imageInfo;
        backendCopy.fAlloc = GrVkAlloc();
        GrBackendTexture backendTex = GrBackendTexture(kW, kH, backendCopy);
        backendTex.setPixelConfig(kPixelConfig);
        tex = gpu->wrapRenderableBackendTexture(backendTex, 1, kBorrow_GrWrapOwnership,
                                                GrWrapCacheable::kNo);
        REPORTER_ASSERT(reporter, tex);
        tex = gpu->wrapRenderableBackendTexture(backendTex, 1, kAdopt_GrWrapOwnership,
                                                GrWrapCacheable::kNo);
        REPORTER_ASSERT(reporter, !tex);
    }

    // check adopt creation
    {
        GrVkImageInfo backendCopy = imageInfo;
        GrBackendTexture backendTex = GrBackendTexture(kW, kH, backendCopy);
        backendTex.setPixelConfig(kPixelConfig);
        tex = gpu->wrapRenderableBackendTexture(backendTex, 1, kAdopt_GrWrapOwnership,
                                                GrWrapCacheable::kNo);
        REPORTER_ASSERT(reporter, tex);
    }
}

DEF_GPUTEST_FOR_VULKAN_CONTEXT(VkWrapTests, reporter, ctxInfo) {
    wrap_tex_test(reporter, ctxInfo.grContext());
    wrap_rt_test(reporter, ctxInfo.grContext());
    wrap_trt_test(reporter, ctxInfo.grContext());
}

#endif
