/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrBackendSemaphore.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrTypes.h"
#include "include/gpu/ganesh/SkImageGanesh.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/private/base/SkTemplates.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"
#include "tools/gpu/ContextType.h"
#include "tools/gpu/TestContext.h"

#include <string>
#include <cstdint>
#include <initializer_list>

#ifdef SK_GL
#include "include/gpu/gl/GrGLFunctions.h"
#include "include/gpu/gl/GrGLInterface.h"
#include "include/gpu/gl/GrGLTypes.h"
#include "src/gpu/ganesh/gl/GrGLGpu.h"
#include "src/gpu/ganesh/gl/GrGLUtil.h"
#endif

#ifdef SK_VULKAN
#include "include/gpu/ganesh/vk/GrVkBackendSurface.h"
#include "src/gpu/ganesh/vk/GrVkCommandPool.h"
#include "src/gpu/ganesh/vk/GrVkGpu.h"
#include "src/gpu/ganesh/vk/GrVkUtil.h"

#include <vulkan/vulkan_core.h>

namespace skgpu { struct VulkanInterface; }

#ifdef VK_USE_PLATFORM_WIN32_KHR
// windows wants to define this as CreateSemaphoreA or CreateSemaphoreW
#undef CreateSemaphore
#endif
#endif

using namespace skia_private;

struct GrContextOptions;

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
                const GrBackendTexture& backendTexture,
                const GrBackendSemaphore& semaphore) {

    childInfo.testContext()->makeCurrent();

    const SkImageInfo childII = SkImageInfo::Make(CHILD_W, CHILD_H, kRGBA_8888_SkColorType,
                                                  kPremul_SkAlphaType);

    auto childDContext = childInfo.directContext();
    sk_sp<SkSurface> childSurface(SkSurfaces::RenderTarget(
            childDContext, skgpu::Budgeted::kNo, childII, 0, kTopLeft_GrSurfaceOrigin, nullptr));

    sk_sp<SkImage> childImage = SkImages::BorrowTextureFrom(childDContext,
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

enum class FlushType { kSurface, kImage, kContext };

void surface_semaphore_test(skiatest::Reporter* reporter,
                            const sk_gpu_test::ContextInfo& mainInfo,
                            const sk_gpu_test::ContextInfo& childInfo1,
                            const sk_gpu_test::ContextInfo& childInfo2,
                            FlushType flushType) {
    auto mainCtx = mainInfo.directContext();
    if (!mainCtx->priv().caps()->semaphoreSupport()) {
        return;
    }

    const SkImageInfo ii = SkImageInfo::Make(MAIN_W, MAIN_H, kRGBA_8888_SkColorType,
                                             kPremul_SkAlphaType);

    sk_sp<SkSurface> mainSurface(SkSurfaces::RenderTarget(
            mainCtx, skgpu::Budgeted::kNo, ii, 0, kTopLeft_GrSurfaceOrigin, nullptr));
    SkCanvas* mainCanvas = mainSurface->getCanvas();
    auto blueSurface = mainSurface->makeSurface(ii);
    blueSurface->getCanvas()->clear(SK_ColorBLUE);
    auto blueImage = blueSurface->makeImageSnapshot();
    blueSurface.reset();
    mainCanvas->drawImage(blueImage, 0, 0);

    AutoTArray<GrBackendSemaphore> semaphores(2);
#ifdef SK_VULKAN
    if (GrBackendApi::kVulkan == mainInfo.backend()) {
        // Initialize the secondary semaphore instead of having Ganesh create one internally
        GrVkGpu* gpu = static_cast<GrVkGpu*>(mainCtx->priv().getGpu());
        VkDevice device = gpu->device();

        VkSemaphore vkSem;

        VkSemaphoreCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        GR_VK_CALL_ERRCHECK(gpu, CreateSemaphore(device, &createInfo, nullptr, &vkSem));

        semaphores[1].initVulkan(vkSem);
    }
#endif

    GrFlushInfo info;
    info.fNumSemaphores = 2;
    info.fSignalSemaphores = semaphores.get();
    switch (flushType) {
        case FlushType::kSurface:
            mainCtx->flush(mainSurface, SkSurfaces::BackendSurfaceAccess::kNoAccess, info);
            break;
        case FlushType::kImage:
            mainCtx->flush(blueImage, info);
            break;
        case FlushType::kContext:
            mainCtx->flush(info);
            break;
    }
    mainCtx->submit();

    GrBackendTexture backendTexture = SkSurfaces::GetBackendTexture(
            mainSurface.get(), SkSurface::BackendHandleAccess::kFlushRead);

    draw_child(reporter, childInfo1, backendTexture, semaphores[0]);

#ifdef SK_VULKAN
    if (GrBackendApi::kVulkan == mainInfo.backend()) {
        // In Vulkan we need to make sure we are sending the correct VkImageLayout in with the
        // backendImage. After the first child draw the layout gets changed to SHADER_READ, so
        // we just manually set that here.
        GrBackendTextures::SetVkImageLayout(&backendTexture,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }
#endif

    draw_child(reporter, childInfo2, backendTexture, semaphores[1]);
}

#ifdef SK_GL
DEF_GANESH_TEST(SurfaceSemaphores, reporter, options, CtsEnforcement::kApiLevel_T) {
#if defined(SK_BUILD_FOR_UNIX) || defined(SK_BUILD_FOR_WIN) || defined(SK_BUILD_FOR_MAC)
    static constexpr auto kNativeGLType = skgpu::ContextType::kGL;
#else
    static constexpr auto kNativeGLType = skgpu::ContextType::kGLES;
#endif

    for (int typeInt = 0; typeInt < skgpu::kContextTypeCount; ++typeInt) {
        for (auto flushType : {FlushType::kSurface, FlushType::kImage, FlushType::kContext}) {
            skgpu::ContextType contextType = static_cast<skgpu::ContextType>(typeInt);
            // Use "native" instead of explicitly trying OpenGL and OpenGL ES. Do not use GLES on
            // desktop since tests do not account for not fixing http://skbug.com/2809
            if (contextType == skgpu::ContextType::kGL ||
                contextType == skgpu::ContextType::kGLES) {
                if (contextType != kNativeGLType) {
                    continue;
                }
            }
            sk_gpu_test::GrContextFactory factory(options);
            sk_gpu_test::ContextInfo ctxInfo = factory.getContextInfo(contextType);
            if (!skgpu::IsRenderingContext(contextType)) {
                continue;
            }
            skiatest::ReporterContext ctx(reporter, SkString(skgpu::ContextTypeName(contextType)));
            if (ctxInfo.directContext()) {
                sk_gpu_test::ContextInfo child1 =
                        factory.getSharedContextInfo(ctxInfo.directContext(), 0);
                sk_gpu_test::ContextInfo child2 =
                        factory.getSharedContextInfo(ctxInfo.directContext(), 1);
                if (!child1.directContext() || !child2.directContext()) {
                    continue;
                }

                surface_semaphore_test(reporter, ctxInfo, child1, child2, flushType);
            }
        }
    }
}
#endif

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(EmptySurfaceSemaphoreTest,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    auto ctx = ctxInfo.directContext();
    if (!ctx->priv().caps()->semaphoreSupport()) {
        return;
    }

    const SkImageInfo ii = SkImageInfo::Make(MAIN_W, MAIN_H, kRGBA_8888_SkColorType,
                                             kPremul_SkAlphaType);

    sk_sp<SkSurface> mainSurface(SkSurfaces::RenderTarget(
            ctx, skgpu::Budgeted::kNo, ii, 0, kTopLeft_GrSurfaceOrigin, nullptr));

    // Flush surface once without semaphores to make sure there is no peneding IO for it.
    ctx->flushAndSubmit(mainSurface);

    GrBackendSemaphore semaphore;
    GrFlushInfo flushInfo;
    flushInfo.fNumSemaphores = 1;
    flushInfo.fSignalSemaphores = &semaphore;
    GrSemaphoresSubmitted submitted =
            ctx->flush(mainSurface, SkSurfaces::BackendSurfaceAccess::kNoAccess, flushInfo);
    REPORTER_ASSERT(reporter, GrSemaphoresSubmitted::kYes == submitted);
    ctx->submit();

#ifdef SK_GL
    if (GrBackendApi::kOpenGL == ctxInfo.backend()) {
        GrGLGpu* gpu = static_cast<GrGLGpu*>(ctx->priv().getGpu());
        const GrGLInterface* interface = gpu->glInterface();
        GrGLsync sync = semaphore.glSync();
        REPORTER_ASSERT(reporter, sync);
        bool result;
        GR_GL_CALL_RET(interface, result, IsSync(sync));
        REPORTER_ASSERT(reporter, result);
    }
#endif

#ifdef SK_VULKAN
    if (GrBackendApi::kVulkan == ctxInfo.backend()) {
        GrVkGpu* gpu = static_cast<GrVkGpu*>(ctx->priv().getGpu());
        const skgpu::VulkanInterface* interface = gpu->vkInterface();
        VkDevice device = gpu->device();
        VkQueue queue = gpu->queue();
        GrVkCommandPool* cmdPool = gpu->cmdPool();
        VkCommandBuffer cmdBuffer;

        // Create Command Buffer
        const VkCommandBufferAllocateInfo cmdInfo = {
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,   // sType
            nullptr,                                          // pNext
            cmdPool->vkCommandPool(),                         // commandPool
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

        GR_VK_CALL_ERRCHECK(gpu, BeginCommandBuffer(cmdBuffer, &cmdBufferBeginInfo));
        GR_VK_CALL_ERRCHECK(gpu, EndCommandBuffer(cmdBuffer));

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
        GR_VK_CALL_ERRCHECK(gpu, QueueSubmit(queue, 1, &submitInfo, fence));

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
