/*
 * Copyright 2016 Google Inc.
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

    GrBackendObject backendObj = gpu->createTestingOnlyBackendTexture(nullptr, kW, kH, kPixelConfig);
    const GrVkTextureInfo* backendTex = reinterpret_cast<const GrVkTextureInfo*>(backendObj);

    // check basic borrowed creation
    GrBackendTextureDesc desc;
    desc.fConfig = kPixelConfig;
    desc.fWidth = kW;
    desc.fHeight = kH;
    desc.fTextureHandle = backendObj;
    GrTexture* tex = gpu->wrapBackendTexture(desc, kBorrow_GrWrapOwnership);
    REPORTER_ASSERT(reporter, tex);
    tex->unref();

    // image is null
    GrVkTextureInfo backendCopy = *backendTex;
    backendCopy.fImage = VK_NULL_HANDLE;
    desc.fTextureHandle = (GrBackendObject) &backendCopy;
    tex = gpu->wrapBackendTexture(desc, kBorrow_GrWrapOwnership);
    REPORTER_ASSERT(reporter, !tex);
    tex = gpu->wrapBackendTexture(desc, kAdopt_GrWrapOwnership);
    REPORTER_ASSERT(reporter, !tex);

    // alloc is null
    backendCopy.fImage = backendTex->fImage;
    backendCopy.fAlloc = VK_NULL_HANDLE;
    tex = gpu->wrapBackendTexture(desc, kBorrow_GrWrapOwnership);
    REPORTER_ASSERT(reporter, !tex);
    tex = gpu->wrapBackendTexture(desc, kAdopt_GrWrapOwnership);
    REPORTER_ASSERT(reporter, !tex);

    // check adopt creation
    backendCopy.fAlloc = backendTex->fAlloc;
    tex = gpu->wrapBackendTexture(desc, kAdopt_GrWrapOwnership);
    REPORTER_ASSERT(reporter, tex);
    tex->unref();

    gpu->deleteTestingOnlyBackendTexture(backendObj, true);
}

void wrap_rt_test(skiatest::Reporter* reporter, GrContext* context) {
    GrVkGpu* gpu = static_cast<GrVkGpu*>(context->getGpu());

    GrBackendObject backendObj = gpu->createTestingOnlyBackendTexture(nullptr, kW, kH, kPixelConfig);
    const GrVkTextureInfo* backendTex = reinterpret_cast<const GrVkTextureInfo*>(backendObj);

    // check basic borrowed creation
    GrBackendRenderTargetDesc desc;
    desc.fWidth = kW;
    desc.fHeight = kH;
    desc.fConfig = kPixelConfig;
    desc.fOrigin = kTopLeft_GrSurfaceOrigin;
    desc.fSampleCnt = 0;
    desc.fStencilBits = 0;
    desc.fRenderTargetHandle = backendObj;
    GrRenderTarget* rt = gpu->wrapBackendRenderTarget(desc, kBorrow_GrWrapOwnership);
    REPORTER_ASSERT(reporter, rt);
    rt->unref();

    // image is null
    GrVkTextureInfo backendCopy = *backendTex;
    backendCopy.fImage = VK_NULL_HANDLE;
    desc.fRenderTargetHandle = (GrBackendObject)&backendCopy;
    rt = gpu->wrapBackendRenderTarget(desc, kBorrow_GrWrapOwnership);
    REPORTER_ASSERT(reporter, !rt);
    rt = gpu->wrapBackendRenderTarget(desc, kAdopt_GrWrapOwnership);
    REPORTER_ASSERT(reporter, !rt);

    // alloc is null
    backendCopy.fImage = backendTex->fImage;
    backendCopy.fAlloc = VK_NULL_HANDLE;
    // can wrap null alloc if borrowing
    rt = gpu->wrapBackendRenderTarget(desc, kBorrow_GrWrapOwnership);
    REPORTER_ASSERT(reporter, rt);
    // but not if adopting
    rt = gpu->wrapBackendRenderTarget(desc, kAdopt_GrWrapOwnership);
    REPORTER_ASSERT(reporter, !rt);

    // check adopt creation
    backendCopy.fAlloc = backendTex->fAlloc;
    rt = gpu->wrapBackendRenderTarget(desc, kAdopt_GrWrapOwnership);
    REPORTER_ASSERT(reporter, rt);
    rt->unref();

    gpu->deleteTestingOnlyBackendTexture(backendObj, true);
}

void wrap_trt_test(skiatest::Reporter* reporter, GrContext* context) {
    GrVkGpu* gpu = static_cast<GrVkGpu*>(context->getGpu());

    GrBackendObject backendObj = gpu->createTestingOnlyBackendTexture(nullptr, kW, kH, kPixelConfig);
    const GrVkTextureInfo* backendTex = reinterpret_cast<const GrVkTextureInfo*>(backendObj);

    // check basic borrowed creation
    GrBackendTextureDesc desc;
    desc.fFlags = kRenderTarget_GrBackendTextureFlag;
    desc.fConfig = kPixelConfig;
    desc.fWidth = kW;
    desc.fHeight = kH;
    desc.fTextureHandle = backendObj;
    GrTexture* tex = gpu->wrapBackendTexture(desc, kBorrow_GrWrapOwnership);
    REPORTER_ASSERT(reporter, tex);
    tex->unref();

    // image is null
    GrVkTextureInfo backendCopy = *backendTex;
    backendCopy.fImage = VK_NULL_HANDLE;
    desc.fTextureHandle = (GrBackendObject)&backendCopy;
    tex = gpu->wrapBackendTexture(desc, kBorrow_GrWrapOwnership);
    REPORTER_ASSERT(reporter, !tex);
    tex = gpu->wrapBackendTexture(desc, kAdopt_GrWrapOwnership);
    REPORTER_ASSERT(reporter, !tex);

    // alloc is null
    backendCopy.fImage = backendTex->fImage;
    backendCopy.fAlloc = VK_NULL_HANDLE;
    tex = gpu->wrapBackendTexture(desc, kBorrow_GrWrapOwnership);
    REPORTER_ASSERT(reporter, !tex);
    tex = gpu->wrapBackendTexture(desc, kAdopt_GrWrapOwnership);
    REPORTER_ASSERT(reporter, !tex);

    // check adopt creation
    backendCopy.fAlloc = backendTex->fAlloc;
    tex = gpu->wrapBackendTexture(desc, kAdopt_GrWrapOwnership);
    REPORTER_ASSERT(reporter, tex);
    tex->unref();

    gpu->deleteTestingOnlyBackendTexture(backendObj, true);
}

DEF_GPUTEST_FOR_VULKAN_CONTEXT(VkWrapTests, reporter, ctxInfo) {
    wrap_tex_test(reporter, ctxInfo.fGrContext);
    wrap_rt_test(reporter, ctxInfo.fGrContext);
    wrap_trt_test(reporter, ctxInfo.fGrContext);
}

#endif
