/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a GPU-backend specific test. It relies on static intializers to work

#include "SkTypes.h"

#if SK_SUPPORT_GPU && defined(SK_VULKAN)

#include "GrContextPriv.h"
#include "GrContextFactory.h"
#include "GrRenderTarget.h"
#include "GrTest.h"
#include "GrTexture.h"

#include "Test.h"
#include "vk/GrVkCaps.h"
#include "vk/GrVkGpu.h"
#include "vk/GrVkMemory.h"
#include "vk/GrVkTypes.h"

using sk_gpu_test::GrContextFactory;

const int kW = 1024;
const int kH = 1024;
const GrPixelConfig kPixelConfig = kRGBA_8888_GrPixelConfig;

void wrap_tex_test(skiatest::Reporter* reporter, GrContext* context) {

    GrVkGpu* gpu = static_cast<GrVkGpu*>(context->contextPriv().getGpu());

    GrBackendTexture origBackendTex = gpu->createTestingOnlyBackendTexture(nullptr, kW, kH,
                                                                           kPixelConfig, false,
                                                                           GrMipMapped::kNo);
    const GrVkImageInfo* imageInfo = origBackendTex.getVkImageInfo();

    sk_sp<GrTexture> tex = gpu->wrapBackendTexture(origBackendTex, kBorrow_GrWrapOwnership);
    REPORTER_ASSERT(reporter, tex);

    // image is null
    {
        GrVkImageInfo backendCopy = *imageInfo;
        backendCopy.fImage = VK_NULL_HANDLE;
        GrBackendTexture backendTex = GrBackendTexture(kW, kH, backendCopy);
        tex = gpu->wrapBackendTexture(backendTex, kBorrow_GrWrapOwnership);
        REPORTER_ASSERT(reporter, !tex);
        tex = gpu->wrapBackendTexture(backendTex, kAdopt_GrWrapOwnership);
        REPORTER_ASSERT(reporter, !tex);
    }

    // alloc is null
    {
        GrVkImageInfo backendCopy = *imageInfo;
        backendCopy.fAlloc = GrVkAlloc();
        GrBackendTexture backendTex = GrBackendTexture(kW, kH, backendCopy);
        tex = gpu->wrapBackendTexture(backendTex, kBorrow_GrWrapOwnership);
        REPORTER_ASSERT(reporter, !tex);
        tex = gpu->wrapBackendTexture(backendTex, kAdopt_GrWrapOwnership);
        REPORTER_ASSERT(reporter, !tex);
    }

    // check adopt creation
    {
        GrVkImageInfo backendCopy = *imageInfo;
        GrBackendTexture backendTex = GrBackendTexture(kW, kH, backendCopy);
        tex = gpu->wrapBackendTexture(backendTex, kAdopt_GrWrapOwnership);

        REPORTER_ASSERT(reporter, tex);
    }

    gpu->deleteTestingOnlyBackendTexture(&origBackendTex, true);
}

void wrap_rt_test(skiatest::Reporter* reporter, GrContext* context) {
    GrVkGpu* gpu = static_cast<GrVkGpu*>(context->contextPriv().getGpu());

    GrBackendTexture origBackendTex = gpu->createTestingOnlyBackendTexture(nullptr, kW, kH,
                                                                           kPixelConfig, true,
                                                                           GrMipMapped::kNo);
    const GrVkImageInfo* imageInfo = origBackendTex.getVkImageInfo();

    GrBackendRenderTarget origBackendRT(kW, kH, 1, 0, *imageInfo);

    sk_sp<GrRenderTarget> rt = gpu->wrapBackendRenderTarget(origBackendRT);
    REPORTER_ASSERT(reporter, rt);

    // image is null
    {
        GrVkImageInfo backendCopy = *imageInfo;
        backendCopy.fImage = VK_NULL_HANDLE;
        GrBackendRenderTarget backendRT(kW, kH, 1, 0, backendCopy);
        rt = gpu->wrapBackendRenderTarget(backendRT);
        REPORTER_ASSERT(reporter, !rt);
    }

    // alloc is null
    {
        GrVkImageInfo backendCopy = *imageInfo;
        backendCopy.fAlloc = GrVkAlloc();
        // can wrap null alloc
        GrBackendRenderTarget backendRT(kW, kH, 1, 0, backendCopy);
        rt = gpu->wrapBackendRenderTarget(backendRT);
        REPORTER_ASSERT(reporter, rt);
    }

    // When we wrapBackendRenderTarget it is always borrowed, so we must make sure to free the
    // resource when we're done.
    gpu->deleteTestingOnlyBackendTexture(&origBackendTex);
}

void wrap_trt_test(skiatest::Reporter* reporter, GrContext* context) {
    GrVkGpu* gpu = static_cast<GrVkGpu*>(context->contextPriv().getGpu());

    GrBackendTexture origBackendTex = gpu->createTestingOnlyBackendTexture(nullptr, kW, kH,
                                                                           kPixelConfig, true,
                                                                           GrMipMapped::kNo);
    const GrVkImageInfo* imageInfo = origBackendTex.getVkImageInfo();

    sk_sp<GrTexture> tex =
            gpu->wrapRenderableBackendTexture(origBackendTex, 1, kBorrow_GrWrapOwnership);
    REPORTER_ASSERT(reporter, tex);

    // image is null
    {
        GrVkImageInfo backendCopy = *imageInfo;
        backendCopy.fImage = VK_NULL_HANDLE;
        GrBackendTexture backendTex = GrBackendTexture(kW, kH, backendCopy);
        tex = gpu->wrapRenderableBackendTexture(backendTex, 1, kBorrow_GrWrapOwnership);
        REPORTER_ASSERT(reporter, !tex);
        tex = gpu->wrapRenderableBackendTexture(backendTex, 1, kAdopt_GrWrapOwnership);
        REPORTER_ASSERT(reporter, !tex);
    }

    // alloc is null
    {
        GrVkImageInfo backendCopy = *imageInfo;
        backendCopy.fAlloc = GrVkAlloc();
        GrBackendTexture backendTex = GrBackendTexture(kW, kH, backendCopy);
        tex = gpu->wrapRenderableBackendTexture(backendTex, 1, kBorrow_GrWrapOwnership);
        REPORTER_ASSERT(reporter, !tex);
        tex = gpu->wrapRenderableBackendTexture(backendTex, 1, kAdopt_GrWrapOwnership);
        REPORTER_ASSERT(reporter, !tex);
    }

    // check adopt creation
    {
        GrVkImageInfo backendCopy = *imageInfo;
        GrBackendTexture backendTex = GrBackendTexture(kW, kH, backendCopy);
        tex = gpu->wrapRenderableBackendTexture(backendTex, 1, kAdopt_GrWrapOwnership);
        REPORTER_ASSERT(reporter, tex);
    }

    gpu->deleteTestingOnlyBackendTexture(&origBackendTex, true);
}

DEF_GPUTEST_FOR_VULKAN_CONTEXT(VkWrapTests, reporter, ctxInfo) {
    wrap_tex_test(reporter, ctxInfo.grContext());
    wrap_rt_test(reporter, ctxInfo.grContext());
    wrap_trt_test(reporter, ctxInfo.grContext());
}

#endif
