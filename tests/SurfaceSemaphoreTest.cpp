/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"

#if SK_SUPPORT_GPU
#include "GrContextFactory.h"
#include "GrTest.h"
#include "Test.h"

#include "GrBackendSemaphore.h"
#include "GrBackendSurface.h"
#include "SkCanvas.h"
#include "SkSurface.h"

#ifdef SK_VULKAN
#include "vk/GrVkTypes.h"
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
                                                         kPremul_SkAlphaType,
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
    childCanvas->readPixels(bitmap, 0, 0);

    check_pixels(reporter, bitmap);
}

void surface_semaphore_test(skiatest::Reporter* reporter,
                            const sk_gpu_test::ContextInfo& mainInfo,
                            const sk_gpu_test::ContextInfo& childInfo1,
                            const sk_gpu_test::ContextInfo& childInfo2) {
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

    mainSurface->flushAndSignalSemaphores(2, semaphores.get());

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

DEF_GPUTEST(SurfaceSemaphores, reporter, factory) {
#if defined(SK_BUILD_FOR_UNIX) || defined(SK_BUILD_FOR_WIN) || defined(SK_BUILD_FOR_MAC)
    static constexpr auto kNativeGLType = sk_gpu_test::GrContextFactory::kGL_ContextType;
#else
    static constexpr auto kNativeGLType = sk_gpu_test::GrContextFactory::kGLES_ContextType;
#endif

    for (int typeInt = 0; typeInt < sk_gpu_test::GrContextFactory::kContextTypeCnt; ++typeInt) {
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
        sk_gpu_test::ContextInfo ctxInfo = factory->getContextInfo(
                contextType, sk_gpu_test::GrContextFactory::ContextOverrides::kDisableNVPR);
        if (!sk_gpu_test::GrContextFactory::IsRenderingContext(contextType)) {
            continue;
        }
        skiatest::ReporterContext ctx(
                reporter, SkString(sk_gpu_test::GrContextFactory::ContextTypeName(contextType)));
        if (ctxInfo.grContext()) {
            sk_gpu_test::ContextInfo child1 = factory->getSharedContextInfo(ctxInfo.grContext(), 0);
            sk_gpu_test::ContextInfo child2 = factory->getSharedContextInfo(ctxInfo.grContext(), 1);
            if (!child1.grContext() || !child2.grContext()) {
                continue;
            }

            surface_semaphore_test(reporter, ctxInfo, child1, child2);
        }
    }
}

#endif
