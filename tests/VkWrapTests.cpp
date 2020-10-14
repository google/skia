/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a GPU-backend specific test. It relies on static intializers to work

#include "include/core/SkTypes.h"

#if defined(SK_VULKAN)

#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/vk/GrVkTypes.h"
#include "include/gpu/vk/GrVkVulkan.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrRenderTarget.h"
#include "src/gpu/GrTexture.h"
#include "src/gpu/vk/GrVkCaps.h"
#include "src/gpu/vk/GrVkGpu.h"
#include "src/gpu/vk/GrVkMemory.h"
#include "tests/Test.h"
#include "tools/gpu/GrContextFactory.h"
#include "tools/gpu/ManagedBackendTexture.h"

using sk_gpu_test::GrContextFactory;

const int kW = 1024;
const int kH = 1024;
const SkColorType kColorType = SkColorType::kRGBA_8888_SkColorType;

void wrap_tex_test(skiatest::Reporter* reporter, GrDirectContext* dContext) {
    GrGpu* gpu = dContext->priv().getGpu();

    auto mbet = sk_gpu_test::ManagedBackendTexture::MakeWithoutData(
            dContext, kW, kH, kRGBA_8888_SkColorType, GrMipmapped::kNo, GrRenderable::kNo);
    if (!mbet) {
        ERRORF(reporter, "Could not create backend texture.");
        return;
    }

    GrBackendTexture origBackendTex = mbet->texture();

    GrVkImageInfo imageInfo;
    SkAssertResult(origBackendTex.getVkImageInfo(&imageInfo));

    {
        sk_sp<GrTexture> tex = gpu->wrapBackendTexture(origBackendTex, kBorrow_GrWrapOwnership,
                                                       GrWrapCacheable::kNo, kRead_GrIOType);
        REPORTER_ASSERT(reporter, tex);
    }

    // image is null
    {
        GrVkImageInfo backendCopy = imageInfo;
        backendCopy.fImage = VK_NULL_HANDLE;
        GrBackendTexture backendTex = GrBackendTexture(kW, kH, backendCopy);
        sk_sp<GrTexture> tex = gpu->wrapBackendTexture(
                backendTex, kBorrow_GrWrapOwnership, GrWrapCacheable::kNo, kRead_GrIOType);
        REPORTER_ASSERT(reporter, !tex);
        tex = gpu->wrapBackendTexture(
                backendTex, kAdopt_GrWrapOwnership, GrWrapCacheable::kNo, kRead_GrIOType);
        REPORTER_ASSERT(reporter, !tex);
    }

    // alloc is null
    {
        GrVkImageInfo backendCopy = imageInfo;
        backendCopy.fAlloc = GrVkAlloc();
        GrBackendTexture backendTex = GrBackendTexture(kW, kH, backendCopy);
        sk_sp<GrTexture> tex = gpu->wrapBackendTexture(
                backendTex, kBorrow_GrWrapOwnership, GrWrapCacheable::kNo, kRead_GrIOType);
        REPORTER_ASSERT(reporter, tex);
        tex = gpu->wrapBackendTexture(
                backendTex, kAdopt_GrWrapOwnership, GrWrapCacheable::kNo, kRead_GrIOType);
        REPORTER_ASSERT(reporter, !tex);
    }

    // check adopt creation
    {
        GrVkImageInfo backendCopy = imageInfo;
        GrBackendTexture backendTex = GrBackendTexture(kW, kH, backendCopy);
        sk_sp<GrTexture> tex = gpu->wrapBackendTexture(
                backendTex, kAdopt_GrWrapOwnership, GrWrapCacheable::kNo, kRead_GrIOType);

        REPORTER_ASSERT(reporter, tex);
        if (tex) {
            mbet->wasAdopted();
        }
    }
}

void wrap_rt_test(skiatest::Reporter* reporter, GrDirectContext* dContext) {
    GrGpu* gpu = dContext->priv().getGpu();
    GrColorType ct = SkColorTypeToGrColorType(kColorType);

    for (int sampleCnt : {1, 4}) {
        GrBackendFormat format = gpu->caps()->getDefaultBackendFormat(ct, GrRenderable::kYes);
        if (sampleCnt > gpu->caps()->maxRenderTargetSampleCount(format)) {
            continue;
        }

        GrBackendRenderTarget origBackendRT =
                gpu->createTestingOnlyBackendRenderTarget({kW, kH}, ct, sampleCnt);
        if (!origBackendRT.isValid()) {
            ERRORF(reporter, "Could not create backend render target.");
        }

        GrVkImageInfo imageInfo;
        REPORTER_ASSERT(reporter, origBackendRT.getVkImageInfo(&imageInfo));

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

        gpu->deleteTestingOnlyBackendRenderTarget(origBackendRT);
    }
}

void wrap_trt_test(skiatest::Reporter* reporter, GrDirectContext* dContext) {
    GrGpu* gpu = dContext->priv().getGpu();

    auto mbet = sk_gpu_test::ManagedBackendTexture::MakeWithoutData(
            dContext, kW, kH, kRGBA_8888_SkColorType, GrMipmapped::kNo, GrRenderable::kYes);
    if (!mbet) {
        ERRORF(reporter, "Could not create renderable backend texture.");
        return;
    }
    GrBackendTexture origBackendTex = mbet->texture();

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

    // check rendering with MSAA
    {
        int maxSamples = dContext->priv().caps()->maxRenderTargetSampleCount(
                origBackendTex.getBackendFormat());
        bool shouldSucceed = maxSamples > 1;
        tex = gpu->wrapRenderableBackendTexture(origBackendTex, 2, kBorrow_GrWrapOwnership,
                                                GrWrapCacheable::kNo);
        REPORTER_ASSERT(reporter, SkToBool(tex) == shouldSucceed);
    }

    // check adopt creation
    {
        GrVkImageInfo backendCopy = imageInfo;
        GrBackendTexture backendTex = GrBackendTexture(kW, kH, backendCopy);
        tex = gpu->wrapRenderableBackendTexture(backendTex, 1, kAdopt_GrWrapOwnership,
                                                GrWrapCacheable::kNo);
        REPORTER_ASSERT(reporter, tex);
        if (tex) {
            mbet->wasAdopted();
        }
    }
}

DEF_GPUTEST_FOR_VULKAN_CONTEXT(VkWrapTests, reporter, ctxInfo) {
    auto dContext = ctxInfo.directContext();

    wrap_tex_test(reporter, dContext);
    wrap_rt_test(reporter, dContext);
    wrap_trt_test(reporter, dContext);
}

#endif
