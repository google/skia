/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a GPU-backend specific test. It relies on static intializers to work

#include "SkTypes.h"

#if SK_SUPPORT_GPU && defined(SK_VULKAN)

#include "GrContextFactory.h"
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

    GrVkGpu* gpu = static_cast<GrVkGpu*>(context->getGpu());

    GrBackendObject backendObj = gpu->createTestingOnlyBackendTexture(nullptr, kW, kH, kPixelConfig,
                                                                      false);
    const GrVkImageInfo* imageInfo = reinterpret_cast<const GrVkImageInfo*>(backendObj);

    GrBackendTexture backendTex = GrBackendTexture(kW, kH, *imageInfo);
    sk_sp<GrTexture> tex = gpu->wrapBackendTexture(backendTex,
                                                   kTopLeft_GrSurfaceOrigin,
                                                   kBorrow_GrWrapOwnership);
    REPORTER_ASSERT(reporter, tex);

    // image is null
    GrVkImageInfo backendCopy = *imageInfo;
    backendCopy.fImage = VK_NULL_HANDLE;
    backendTex = GrBackendTexture(kW, kH, backendCopy);
    tex = gpu->wrapBackendTexture(backendTex,
                                  kTopLeft_GrSurfaceOrigin,
                                  kBorrow_GrWrapOwnership);
    REPORTER_ASSERT(reporter, !tex);
    tex = gpu->wrapBackendTexture(backendTex,
                                  kTopLeft_GrSurfaceOrigin,
                                  kAdopt_GrWrapOwnership);
    REPORTER_ASSERT(reporter, !tex);

    // alloc is null
    backendCopy.fImage = imageInfo->fImage;
    backendCopy.fAlloc = { VK_NULL_HANDLE, 0, 0, 0 };
    backendTex = GrBackendTexture(kW, kH, backendCopy);
    tex = gpu->wrapBackendTexture(backendTex,
                                  kTopLeft_GrSurfaceOrigin,
                                  kBorrow_GrWrapOwnership);
    REPORTER_ASSERT(reporter, !tex);
    tex = gpu->wrapBackendTexture(backendTex,
                                  kTopLeft_GrSurfaceOrigin,
                                  kAdopt_GrWrapOwnership);
    REPORTER_ASSERT(reporter, !tex);
    // check adopt creation
    backendCopy.fAlloc = imageInfo->fAlloc;
    backendTex = GrBackendTexture(kW, kH, backendCopy);
    tex = gpu->wrapBackendTexture(backendTex,
                                  kTopLeft_GrSurfaceOrigin,
                                  kAdopt_GrWrapOwnership);

    REPORTER_ASSERT(reporter, tex);

    gpu->deleteTestingOnlyBackendTexture(backendObj, true);
}

void wrap_rt_test(skiatest::Reporter* reporter, GrContext* context) {
    GrVkGpu* gpu = static_cast<GrVkGpu*>(context->getGpu());

    GrBackendObject backendObj = gpu->createTestingOnlyBackendTexture(nullptr, kW, kH, kPixelConfig,
                                                                      true);
    const GrVkImageInfo* backendTex = reinterpret_cast<const GrVkImageInfo*>(backendObj);

    GrBackendRenderTarget backendRT(kW, kH, 0, 0, *backendTex);

    sk_sp<GrRenderTarget> rt = gpu->wrapBackendRenderTarget(backendRT, kTopLeft_GrSurfaceOrigin);
    REPORTER_ASSERT(reporter, rt);

    // image is null
    GrVkImageInfo backendCopy = *backendTex;
    backendCopy.fImage = VK_NULL_HANDLE;
    GrBackendRenderTarget backendRT2(kW, kH, 0, 0, backendCopy);
    rt = gpu->wrapBackendRenderTarget(backendRT2, kTopLeft_GrSurfaceOrigin);
    REPORTER_ASSERT(reporter, !rt);

    // alloc is null
    backendCopy.fImage = backendTex->fImage;
    backendCopy.fAlloc = { VK_NULL_HANDLE, 0, 0, 0 };
    // can wrap null alloc
    GrBackendRenderTarget backendRT3(kW, kH, 0, 0, backendCopy);
    rt = gpu->wrapBackendRenderTarget(backendRT3, kTopLeft_GrSurfaceOrigin);
    REPORTER_ASSERT(reporter, rt);

    // When we wrapBackendRenderTarget it is always borrowed, so we must make sure to free the
    // resource when we're done.
    gpu->deleteTestingOnlyBackendTexture(backendObj, false);
}

void wrap_trt_test(skiatest::Reporter* reporter, GrContext* context) {
    GrVkGpu* gpu = static_cast<GrVkGpu*>(context->getGpu());

    GrBackendObject backendObj = gpu->createTestingOnlyBackendTexture(nullptr, kW, kH, kPixelConfig,
                                                                      true);
    const GrVkImageInfo* imageInfo = reinterpret_cast<const GrVkImageInfo*>(backendObj);

    GrBackendTexture backendTex = GrBackendTexture(kW, kH, *imageInfo);
    sk_sp<GrTexture> tex = gpu->wrapRenderableBackendTexture(backendTex,
                                                             kTopLeft_GrSurfaceOrigin,
                                                             0,
                                                             kBorrow_GrWrapOwnership);
    REPORTER_ASSERT(reporter, tex);

    // image is null
    GrVkImageInfo backendCopy = *imageInfo;
    backendCopy.fImage = VK_NULL_HANDLE;
    backendTex = GrBackendTexture(kW, kH, backendCopy);
    tex = gpu->wrapRenderableBackendTexture(backendTex,
                                            kTopLeft_GrSurfaceOrigin,
                                            0,
                                            kBorrow_GrWrapOwnership);
    REPORTER_ASSERT(reporter, !tex);
    tex = gpu->wrapRenderableBackendTexture(backendTex,
                                            kTopLeft_GrSurfaceOrigin,
                                            0,
                                            kAdopt_GrWrapOwnership);
    REPORTER_ASSERT(reporter, !tex);

    // alloc is null
    backendCopy.fImage = imageInfo->fImage;
    backendCopy.fAlloc = { VK_NULL_HANDLE, 0, 0, 0 };
    backendTex = GrBackendTexture(kW, kH, backendCopy);
    tex = gpu->wrapRenderableBackendTexture(backendTex,
                                            kTopLeft_GrSurfaceOrigin,
                                            0,
                                            kBorrow_GrWrapOwnership);
    REPORTER_ASSERT(reporter, !tex);
    tex = gpu->wrapRenderableBackendTexture(backendTex,
                                            kTopLeft_GrSurfaceOrigin,
                                            0,
                                            kAdopt_GrWrapOwnership);
    REPORTER_ASSERT(reporter, !tex);

    // check adopt creation
    backendCopy.fAlloc = imageInfo->fAlloc;
    backendTex = GrBackendTexture(kW, kH, backendCopy);
    tex = gpu->wrapRenderableBackendTexture(backendTex,
                                            kTopLeft_GrSurfaceOrigin,
                                            0,
                                            kAdopt_GrWrapOwnership);
    REPORTER_ASSERT(reporter, tex);

    gpu->deleteTestingOnlyBackendTexture(backendObj, true);
}

DEF_GPUTEST_FOR_VULKAN_CONTEXT(VkWrapTests, reporter, ctxInfo) {
    wrap_tex_test(reporter, ctxInfo.grContext());
    wrap_rt_test(reporter, ctxInfo.grContext());
    wrap_trt_test(reporter, ctxInfo.grContext());
}

#endif
