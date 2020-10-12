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
#include "include/gpu/GrDirectContext.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrRenderTarget.h"
#include "src/gpu/GrTexture.h"
#include "tools/gpu/GrContextFactory.h"

#include "include/gpu/vk/GrVkTypes.h"
#include "src/gpu/vk/GrVkCaps.h"
#include "src/gpu/vk/GrVkGpu.h"
#include "src/gpu/vk/GrVkMemory.h"
#include "tests/Test.h"
#include "tests/TestUtils.h"

using sk_gpu_test::GrContextFactory;

const int kW = 1024;
const int kH = 1024;
const SkColorType kColorType = SkColorType::kRGBA_8888_SkColorType;

void wrap_tex_test(skiatest::Reporter* reporter, GrDirectContext* dContext) {

    GrGpu* gpu = dContext->priv().getGpu();

    GrBackendTexture origBackendTex;
    CreateBackendTexture(dContext, &origBackendTex, kW, kH, kColorType, SkColors::kTransparent,
                         GrMipmapped::kNo, GrRenderable::kNo, GrProtected::kNo);

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
        tex = gpu->wrapBackendTexture(backendTex, kAdopt_GrWrapOwnership, GrWrapCacheable::kNo,
                                      kRead_GrIOType);

        REPORTER_ASSERT(reporter, tex);
    }

    // image has MSAA
    {
        GrVkImageInfo backendCopy = imageInfo;
        backendCopy.fSampleCount = 4;
        GrBackendTexture backendTex = GrBackendTexture(kW, kH, backendCopy);
        tex = gpu->wrapBackendTexture(backendTex, kBorrow_GrWrapOwnership, GrWrapCacheable::kNo,
                                      kRead_GrIOType);
        REPORTER_ASSERT(reporter, !tex);
        tex = gpu->wrapBackendTexture(backendTex, kAdopt_GrWrapOwnership, GrWrapCacheable::kNo,
                                      kRead_GrIOType);
        REPORTER_ASSERT(reporter, !tex);
    }
}

void wrap_rt_test(skiatest::Reporter* reporter, GrDirectContext* dContext) {
    GrGpu* gpu = dContext->priv().getGpu();

    GrBackendTexture origBackendTex;
    CreateBackendTexture(dContext, &origBackendTex, kW, kH, kColorType, SkColors::kTransparent,
                         GrMipmapped::kNo, GrRenderable::kYes, GrProtected::kNo);

    GrVkImageInfo imageInfo;
    SkAssertResult(origBackendTex.getVkImageInfo(&imageInfo));

    GrBackendRenderTarget origBackendRT(kW, kH, 1, imageInfo);

    sk_sp<GrRenderTarget> rt = gpu->wrapBackendRenderTarget(origBackendRT);
    REPORTER_ASSERT(reporter, rt);

    // image is null
    {
        GrVkImageInfo backendCopy = imageInfo;
        backendCopy.fImage = VK_NULL_HANDLE;
        GrBackendRenderTarget backendRT(kW, kH, 1, backendCopy);
        rt = gpu->wrapBackendRenderTarget(backendRT);
        REPORTER_ASSERT(reporter, !rt);
    }

    // alloc is null
    {
        GrVkImageInfo backendCopy = imageInfo;
        backendCopy.fAlloc = GrVkAlloc();
        // can wrap null alloc
        GrBackendRenderTarget backendRT(kW, kH, 1, backendCopy);
        rt = gpu->wrapBackendRenderTarget(backendRT);
        REPORTER_ASSERT(reporter, rt);
    }

    // Image has MSAA
    {
        GrColorType ct = SkColorTypeToGrColorType(kColorType);
        GrGpu* gpu = dContext->priv().getGpu();
        GrBackendRenderTarget backendRT =
                gpu->createTestingOnlyBackendRenderTarget({kW, kW}, ct, 4);
        if (backendRT.isValid()) {
            rt = gpu->wrapBackendRenderTarget(backendRT);
            REPORTER_ASSERT(reporter, rt);
            dContext->priv().getGpu()->deleteTestingOnlyBackendRenderTarget(backendRT);
        }
    }

    // When we wrapBackendRenderTarget it is always borrowed, so we must make sure to free the
    // resource when we're done.
    dContext->deleteBackendTexture(origBackendTex);
}

void wrap_trt_test(skiatest::Reporter* reporter, GrDirectContext* dContext) {
    GrGpu* gpu = dContext->priv().getGpu();

    GrBackendTexture origBackendTex;
    CreateBackendTexture(dContext, &origBackendTex, kW, kH, kColorType, SkColors::kTransparent,
                         GrMipmapped::kNo, GrRenderable::kYes, GrProtected::kNo);

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
        tex = gpu->wrapRenderableBackendTexture(backendTex, 1, kAdopt_GrWrapOwnership,
                                                GrWrapCacheable::kNo);
        REPORTER_ASSERT(reporter, tex);
    }

    // check rendering with MSAA
    {
        int maxSamples = dContext->priv().caps()->maxRenderTargetSampleCount(
                origBackendTex.getBackendFormat());
        bool shouldSucceed = maxSamples > 1;
        tex = gpu->wrapRenderableBackendTexture(origBackendTex, 2, kBorrow_GrWrapOwnership,
                                                GrWrapCacheable::kNo);
        REPORTER_ASSERT(reporter, SkToBool(tex) == shouldSucceed);
    }

    // Image has MSAA
    {
        GrVkImageInfo backendCopy = imageInfo;
        backendCopy.fSampleCount = 4;
        GrBackendTexture backendTex = GrBackendTexture(kW, kH, backendCopy);
        tex = gpu->wrapRenderableBackendTexture(backendTex, 1, kAdopt_GrWrapOwnership,
                                                GrWrapCacheable::kNo);
        REPORTER_ASSERT(reporter, !tex);
    }
}

DEF_GPUTEST_FOR_VULKAN_CONTEXT(VkWrapTests, reporter, ctxInfo) {
    auto dContext = ctxInfo.directContext();

    wrap_tex_test(reporter, dContext);
    wrap_rt_test(reporter, dContext);
    wrap_trt_test(reporter, dContext);
}

#endif
