/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"

#if SK_SUPPORT_GPU
#include "GrContextPriv.h"
#include "GrContextFactory.h"
#include "GrTest.h"
#include "Test.h"

#include "GrBackendSemaphore.h"
#include "GrBackendSurface.h"
#include "SkCanvas.h"
#include "SkSurface.h"

#include "gl/GrGLGpu.h"
#include "gl/GrGLUtil.h"

#ifdef SK_VULKAN
#include "vk/GrVkGpu.h"
#include "vk/GrVkTypes.h"
#include "vk/GrVkUtil.h"

#ifdef VK_USE_PLATFORM_WIN32_KHR
// windows wants to define this as CreateSemaphoreA or CreateSemaphoreW
#undef CreateSemaphore
#endif
#endif

static const int MAIN_W = 8, MAIN_H = 16;
static const int CHILD_W = 16, CHILD_H = 16;

void check_pixels(skiatest::Reporter* reporter, const SkBitmap& bitmap) {
    const uint32_t* canvasPixels = static_cast<const uint32_t*>(bitmap.getPixels());

    bool failureFound = false;
    SkPMColor expectedPixel;
    for (int cy = 0; cy < CHILD_H && !failureFound; ++cy) {
        for (int cx = 0; cx < CHILD_W && !failureFound; ++cx) {
            SkPMColor canvasPixel = canvasPixels[cy * CHILD_W + cx];
            if (cy < CHILD_H / 2) {
                if (cx < CHILD_W / 2) {
                    expectedPixel = 0xFF0000FF; // Red
                } else {
                    expectedPixel = 0xFFFF0000; // Blue
                }
            } else {
                expectedPixel = 0xFF00FF00; // Green
            }
            if (expectedPixel != canvasPixel) {
                failureFound = true;
                ERRORF(reporter, "Wrong color at %d, %d. Got 0x%08x when we expected 0x%08x",
                       cx, cy, canvasPixel, expectedPixel);
            }
        }
    }
}

void draw_child(skiatest::Reporter* reporter,
                const sk_gpu_test::ContextInfo& childInfo,
                const GrBackendObject& backendImage,
                const GrBackendSemaphore& semaphore) {
    GrBackendTexture backendTexture = GrTest::CreateBackendTexture(childInfo.backend(),
                                                                   MAIN_W, MAIN_H,
                                                                   kRGBA_8888_GrPixelConfig,
                                                                   GrMipMapped::kNo,
                                                                   backendImage);

    childInfo.testContext()->makeCurrent();

    const SkImageInfo childII = SkImageInfo::Make(CHILD_W, CHILD_H, kRGBA_8888_SkColorType,
                                                  kPremul_SkAlphaType);

    GrContext* childCtx = childInfo.grContext();
    sk_sp<SkSurface> childSurface(SkSurface::MakeRenderTarget(childCtx, SkBudgeted::kNo,
                                                              childII, 0, kTopLeft_GrSurfaceOrigin,
                                                              nullptr));

    sk_sp<SkImage> childImage = SkImage::MakeFromTexture(childCtx,
                                                         backendTexture,
                                                         kTopLeft_GrSurfaceOrigin,
                                                         kRGBA_8888_SkColorType,
                                                         kPremul_SkAlphaType,
                                                         nullptr,
                                                         nullptr,
                                                         nullptr);

    SkCanvas* childCanvas = childSurface->getCanvas();
    childCanvas->clear(SK_ColorRED);

    childSurface->wait(1, &semaphore);

    childCanvas->drawImage(childImage, CHILD_W/2, 0);

    SkPaint paint;
    paint.setColor(SK_ColorGREEN);
    SkIRect rect = SkIRect::MakeLTRB(0, CHILD_H/2, CHILD_W, CHILD_H);
    childCanvas->drawIRect(rect, paint);

    // read pixels
    SkBitmap bitmap;
    bitmap.allocPixels(childII);
    childSurface->readPixels(bitmap, 0, 0);

    check_pixels(reporter, bitmap);
}

void surface_semaphore_test(skiatest::Reporter* reporter,
                            const sk_gpu_test::ContextInfo& mainInfo,
                            const sk_gpu_test::ContextInfo& childInfo1,
                            const sk_gpu_test::ContextInfo& childInfo2,
                            bool flushContext) {
    GrContext* mainCtx = mainInfo.grContext();
    if (!mainCtx->caps()->fenceSyncSupport()) {
        return;
    }

    const SkImageInfo ii = SkImageInfo::Make(MAIN_W, MAIN_H, kRGBA_8888_SkColorType,
                                             kPremul_SkAlphaType);

    sk_sp<SkSurface> mainSurface(SkSurface::MakeRenderTarget(mainCtx, SkBudgeted::kNo,
                                                             ii, 0, kTopLeft_GrSurfaceOrigin,
                                                             nullptr));
    SkCanvas* mainCanvas = mainSurface->getCanvas();
    mainCanvas->clear(SK_ColorBLUE);

    SkAutoTArray<GrBackendSemaphore> semaphores(2);
#ifdef SK_VULKAN
    if (kVulkan_GrBackend == mainInfo.backend()) {
        // Initialize the secondary semaphore instead of having Ganesh create one internally
        GrVkGpu* gpu = static_cast<GrVkGpu*>(mainCtx->contextPriv().getGpu());
        const GrVkInterface* interface = gpu->vkInterface();
        VkDevice device = gpu->device();

        VkSemaphore vkSem;

        VkSemaphoreCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        GR_VK_CALL_ERRCHECK(interface, CreateSemaphore(device, &createInfo, nullptr, &vkSem));

        semaphores[1].initVulkan(vkSem);
    }
#endif

    if (flushContext) {
        mainCtx->flushAndSignalSemaphores(2, semaphores.get());
    } else {
        mainSurface->flushAndSignalSemaphores(2, semaphores.get());
    }

    sk_sp<SkImage> mainImage = mainSurface->makeImageSnapshot();
    GrBackendObject backendImage = mainImage->getTextureHandle(false);

    draw_child(reporter, childInfo1, backendImage, semaphores[0]);

#ifdef SK_VULKAN
    if (kVulkan_GrBackend == mainInfo.backend()) {
        // In Vulkan we need to make sure we are sending the correct VkImageLayout in with the
        // backendImage. After the first child draw the layout gets changed to SHADER_READ, so
        // we just manually set that here.
        GrVkImageInfo* vkInfo = (GrVkImageInfo*)backendImage;
        vkInfo->updateImageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }
#endif

    draw_child(reporter, childInfo2, backendImage, semaphores[1]);
}

DEF_GPUTEST(SurfaceSemaphores, reporter, options) {
#if defined(SK_BUILD_FOR_UNIX) || defined(SK_BUILD_FOR_WIN) || defined(SK_BUILD_FOR_MAC)
    static constexpr auto kNativeGLType = sk_gpu_test::GrContextFactory::kGL_ContextType;
#else
    static constexpr auto kNativeGLType = sk_gpu_test::GrContextFactory::kGLES_ContextType;
#endif

    for (int typeInt = 0; typeInt < sk_gpu_test::GrContextFactory::kContextTypeCnt; ++typeInt) {
        for (auto flushContext : { false, true }) {
            sk_gpu_test::GrContextFactory::ContextType contextType =
                    (sk_gpu_test::GrContextFactory::ContextType) typeInt;
            // Use "native" instead of explicitly trying OpenGL and OpenGL ES. Do not use GLES on
            // desktop since tests do not account for not fixing http://skbug.com/2809
            if (contextType == sk_gpu_test::GrContextFactory::kGL_ContextType ||
                contextType == sk_gpu_test::GrContextFactory::kGLES_ContextType) {
                if (contextType != kNativeGLType) {
                    continue;
                }
            }
            sk_gpu_test::GrContextFactory factory(options);
            sk_gpu_test::ContextInfo ctxInfo = factory.getContextInfo(
                    contextType, sk_gpu_test::GrContextFactory::ContextOverrides::kDisableNVPR);
            if (!sk_gpu_test::GrContextFactory::IsRenderingContext(contextType)) {
                continue;
            }
            skiatest::ReporterContext ctx(
                   reporter, SkString(sk_gpu_test::GrContextFactory::ContextTypeName(contextType)));
            if (ctxInfo.grContext()) {
                sk_gpu_test::ContextInfo child1 =
                        factory.getSharedContextInfo(ctxInfo.grContext(), 0);
                sk_gpu_test::ContextInfo child2 =
                        factory.getSharedContextInfo(ctxInfo.grContext(), 1);
                if (!child1.grContext() || !child2.grContext()) {
                    continue;
                }

                surface_semaphore_test(reporter, ctxInfo, child1, child2, flushContext);
            }
        }
    }
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(EmptySurfaceSemaphoreTest, reporter, ctxInfo) {
    GrContext* ctx = ctxInfo.grContext();
    if (!ctx->caps()->fenceSyncSupport()) {
        return;
    }

    const SkImageInfo ii = SkImageInfo::Make(MAIN_W, MAIN_H, kRGBA_8888_SkColorType,
                                             kPremul_SkAlphaType);

    sk_sp<SkSurface> mainSurface(SkSurface::MakeRenderTarget(ctx, SkBudgeted::kNo,
                                                             ii, 0, kTopLeft_GrSurfaceOrigin,
                                                             nullptr));

    // Flush surface once without semaphores to make sure there is no peneding IO for it.
    mainSurface->flush();

    GrBackendSemaphore semaphore;
    GrSemaphoresSubmitted submitted = mainSurface->flushAndSignalSemaphores(1, &semaphore);
    REPORTER_ASSERT(reporter, GrSemaphoresSubmitted::kYes == submitted);

    if (kOpenGL_GrBackend == ctxInfo.backend()) {
        GrGLGpu* gpu = static_cast<GrGLGpu*>(ctx->contextPriv().getGpu());
        const GrGLInterface* interface = gpu->glInterface();
        GrGLsync sync = semaphore.glSync();
        REPORTER_ASSERT(reporter, sync);
        bool result;
        GR_GL_CALL_RET(interface, result, IsSync(sync));
        REPORTER_ASSERT(reporter, result);
    }

#ifdef SK_VULKAN
    if (kVulkan_GrBackend == ctxInfo.backend()) {
        GrVkGpu* gpu = static_cast<GrVkGpu*>(ctx->contextPriv().getGpu());
        const GrVkInterface* interface = gpu->vkInterface();
        VkDevice device = gpu->device();
        VkQueue queue = gpu->queue();
        VkCommandPool cmdPool = gpu->cmdPool();
        VkCommandBuffer cmdBuffer;

        // Create Command Buffer
        const VkCommandBufferAllocateInfo cmdInfo = {
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,   // sType
            nullptr,                                          // pNext
            cmdPool,                                          // commandPool
            VK_COMMAND_BUFFER_LEVEL_PRIMARY,                  // level
            1                                                 // bufferCount
        };

        VkResult err = GR_VK_CALL(interface, AllocateCommandBuffers(device, &cmdInfo, &cmdBuffer));
        if (err) {
            return;
        }

        VkCommandBufferBeginInfo cmdBufferBeginInfo;
        memset(&cmdBufferBeginInfo, 0, sizeof(VkCommandBufferBeginInfo));
        cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        cmdBufferBeginInfo.pNext = nullptr;
        cmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        cmdBufferBeginInfo.pInheritanceInfo = nullptr;

        GR_VK_CALL_ERRCHECK(interface, BeginCommandBuffer(cmdBuffer, &cmdBufferBeginInfo));
        GR_VK_CALL_ERRCHECK(interface, EndCommandBuffer(cmdBuffer));

        VkFenceCreateInfo fenceInfo;
        VkFence fence;

        memset(&fenceInfo, 0, sizeof(VkFenceCreateInfo));
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        err = GR_VK_CALL(interface, CreateFence(device, &fenceInfo, nullptr, &fence));
        SkASSERT(!err);

        VkPipelineStageFlags waitStages = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        VkSubmitInfo submitInfo;
        memset(&submitInfo, 0, sizeof(VkSubmitInfo));
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.pNext = nullptr;
        submitInfo.waitSemaphoreCount = 1;
        VkSemaphore vkSem = semaphore.vkSemaphore();
        submitInfo.pWaitSemaphores = &vkSem;
        submitInfo.pWaitDstStageMask = &waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cmdBuffer;
        submitInfo.signalSemaphoreCount = 0;
        submitInfo.pSignalSemaphores = nullptr;
        GR_VK_CALL_ERRCHECK(interface, QueueSubmit(queue, 1, &submitInfo, fence));

        err = GR_VK_CALL(interface, WaitForFences(device, 1, &fence, true, 3000000000));

        REPORTER_ASSERT(reporter, err != VK_TIMEOUT);

        GR_VK_CALL(interface, DestroyFence(device, fence, nullptr));
        GR_VK_CALL(interface, DestroySemaphore(device, vkSem, nullptr));
        // If the above test fails the wait semaphore will never be signaled which can cause the
        // device to hang when tearing down (even if just tearing down GL). So we Fail here to
        // kill things.
        if (err == VK_TIMEOUT) {
            SK_ABORT("Waiting on semaphore indefinitely");
        }
    }
#endif
}

#endif
