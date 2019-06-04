/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkOverdrawCanvas.h"
#include "include/core/SkPath.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRegion.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrContext.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/core/SkDevice.h"
#include "src/core/SkUtils.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrGpuResourcePriv.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/SkGpuDevice.h"
#include "src/image/SkImage_Base.h"
#include "src/image/SkImage_Gpu.h"
#include "src/image/SkSurface_Gpu.h"
#include "tests/Test.h"
#include "tests/TestUtils.h"

#include <functional>
#include <initializer_list>
#include <vector>

#include "tools/ToolUtils.h"

static void release_direct_surface_storage(void* pixels, void* context) {
    SkASSERT(pixels == context);
    sk_free(pixels);
}
static sk_sp<SkSurface> create_surface(SkAlphaType at = kPremul_SkAlphaType,
                                       SkImageInfo* requestedInfo = nullptr) {
    const SkImageInfo info = SkImageInfo::MakeN32(10, 10, at);
    if (requestedInfo) {
        *requestedInfo = info;
    }
    return SkSurface::MakeRaster(info);
}
static sk_sp<SkSurface> create_direct_surface(SkAlphaType at = kPremul_SkAlphaType,
                                              SkImageInfo* requestedInfo = nullptr) {
    const SkImageInfo info = SkImageInfo::MakeN32(10, 10, at);
    if (requestedInfo) {
        *requestedInfo = info;
    }
    const size_t rowBytes = info.minRowBytes();
    void* storage = sk_malloc_throw(info.computeByteSize(rowBytes));
    return SkSurface::MakeRasterDirectReleaseProc(info, storage, rowBytes,
                                                  release_direct_surface_storage,
                                                  storage);
}
static sk_sp<SkSurface> create_gpu_surface(GrContext* context, SkAlphaType at = kPremul_SkAlphaType,
                                           SkImageInfo* requestedInfo = nullptr) {
    const SkImageInfo info = SkImageInfo::MakeN32(10, 10, at);
    if (requestedInfo) {
        *requestedInfo = info;
    }
    return SkSurface::MakeRenderTarget(context, SkBudgeted::kNo, info);
}
static sk_sp<SkSurface> create_gpu_scratch_surface(GrContext* context,
                                                   SkAlphaType at = kPremul_SkAlphaType,
                                                   SkImageInfo* requestedInfo = nullptr) {
    const SkImageInfo info = SkImageInfo::MakeN32(10, 10, at);
    if (requestedInfo) {
        *requestedInfo = info;
    }
    return SkSurface::MakeRenderTarget(context, SkBudgeted::kYes, info);
}

DEF_TEST(SurfaceEmpty, reporter) {
    const SkImageInfo info = SkImageInfo::Make(0, 0, kN32_SkColorType, kPremul_SkAlphaType);
    REPORTER_ASSERT(reporter, nullptr == SkSurface::MakeRaster(info));
    REPORTER_ASSERT(reporter, nullptr == SkSurface::MakeRasterDirect(info, nullptr, 0));

}
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(SurfaceEmpty_Gpu, reporter, ctxInfo) {
    const SkImageInfo info = SkImageInfo::Make(0, 0, kN32_SkColorType, kPremul_SkAlphaType);
    REPORTER_ASSERT(reporter, nullptr ==
                    SkSurface::MakeRenderTarget(ctxInfo.grContext(), SkBudgeted::kNo, info));
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrContext_colorTypeSupportedAsSurface, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();

    for (int ct = 0; ct < kLastEnum_SkColorType; ++ct) {
        static constexpr int kSize = 10;

        SkColorType colorType = static_cast<SkColorType>(ct);
        auto info = SkImageInfo::Make(kSize, kSize, colorType, kOpaque_SkAlphaType, nullptr);
        bool can = context->colorTypeSupportedAsSurface(colorType);
        auto surf = SkSurface::MakeRenderTarget(context, SkBudgeted::kYes, info, 1, nullptr);
        REPORTER_ASSERT(reporter, can == SkToBool(surf), "ct: %d, can: %d, surf: %d",
                        colorType, can, SkToBool(surf));

        GrBackendTexture backendTex = context->createBackendTexture(
                kSize, kSize, colorType,
                SkColors::kTransparent, GrMipMapped::kNo, GrRenderable::kYes);
        surf = SkSurface::MakeFromBackendTexture(context, backendTex,
                                                 kTopLeft_GrSurfaceOrigin, 0, colorType, nullptr,
                                                 nullptr);
        REPORTER_ASSERT(reporter, can == SkToBool(surf), "ct: %d, can: %d, surf: %d",
                        colorType, can, SkToBool(surf));

        surf = SkSurface::MakeFromBackendTextureAsRenderTarget(context, backendTex,
                                                               kTopLeft_GrSurfaceOrigin, 1,
                                                               colorType, nullptr, nullptr);
        REPORTER_ASSERT(reporter, can == SkToBool(surf), "ct: %d, can: %d, surf: %d",
                        colorType, can, SkToBool(surf));

        surf.reset();
        context->flush();
        context->deleteBackendTexture(backendTex);

        static constexpr int kSampleCnt = 2;

        can = context->maxSurfaceSampleCountForColorType(colorType) >= kSampleCnt;
        surf = SkSurface::MakeRenderTarget(context, SkBudgeted::kYes, info, kSampleCnt, nullptr);
        REPORTER_ASSERT(reporter, can == SkToBool(surf), "ct: %d, can: %d, surf: %d",
                        colorType, can, SkToBool(surf));

        backendTex = context->createBackendTexture(kSize, kSize, colorType,
                                                   SkColors::kTransparent,
                                                   GrMipMapped::kNo, GrRenderable::kYes);
        surf = SkSurface::MakeFromBackendTexture(context, backendTex,
                                                 kTopLeft_GrSurfaceOrigin, kSampleCnt, colorType,
                                                 nullptr, nullptr);
        REPORTER_ASSERT(reporter, can == SkToBool(surf),
                        "colorTypeSupportedAsSurface:%d, surf:%d, ct:%d", can, SkToBool(surf),
                        colorType);
        // Ensure that the sample count stored on the resulting SkSurface is a valid value.
        if (surf) {
            auto* rtc = ((SkSurface_Gpu*)(surf.get()))->getDevice()->accessRenderTargetContext();
            int storedCnt = rtc->numStencilSamples();
            int allowedCnt = context->priv().caps()->getSampleCount(
                    storedCnt, rtc->asSurfaceProxy()->config());
            REPORTER_ASSERT(reporter, storedCnt == allowedCnt,
                            "Should store an allowed sample count (%d vs %d)", allowedCnt,
                            storedCnt);
        }

        surf = SkSurface::MakeFromBackendTextureAsRenderTarget(context, backendTex,
                                                               kTopLeft_GrSurfaceOrigin, kSampleCnt,
                                                               colorType, nullptr, nullptr);
        REPORTER_ASSERT(reporter, can == SkToBool(surf),
                        "colorTypeSupportedAsSurface:%d, surf:%d, ct:%d", can, SkToBool(surf),
                        colorType);
        if (surf) {
            auto* rtc = ((SkSurface_Gpu*)(surf.get()))->getDevice()->accessRenderTargetContext();
            int storedCnt = rtc->numStencilSamples();
            int allowedCnt = context->priv().caps()->getSampleCount(
                    storedCnt, rtc->asSurfaceProxy()->config());
            REPORTER_ASSERT(reporter, storedCnt == allowedCnt,
                            "Should store an allowed sample count (%d vs %d)", allowedCnt,
                            storedCnt);
        }

        surf.reset();
        context->flush();
        context->deleteBackendTexture(backendTex);

        auto* gpu = context->priv().getGpu();

        GrBackendRenderTarget backendRenderTarget = gpu->createTestingOnlyBackendRenderTarget(
                16, 16, SkColorTypeToGrColorType(colorType));
        can = context->colorTypeSupportedAsSurface(colorType);
        surf = SkSurface::MakeFromBackendRenderTarget(context, backendRenderTarget,
                                                      kTopLeft_GrSurfaceOrigin, colorType, nullptr,
                                                      nullptr);
        REPORTER_ASSERT(reporter, can == SkToBool(surf), "ct: %d, can: %d, surf: %d", colorType,
                        can, SkToBool(surf));
        surf.reset();
        context->flush();
        if (backendRenderTarget.isValid()) {
            gpu->deleteTestingOnlyBackendRenderTarget(backendRenderTarget);
        }
    }
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrContext_maxSurfaceSamplesForColorType, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();

    static constexpr int kSize = 10;

    for (int ct = 0; ct < kLastEnum_SkColorType; ++ct) {

        SkColorType colorType = static_cast<SkColorType>(ct);
        int max = context->maxSurfaceSampleCountForColorType(colorType);
        if (!max) {
            continue;
        }
        GrBackendTexture backendTex = context->createBackendTexture(
                kSize, kSize, colorType, SkColors::kTransparent,
                GrMipMapped::kNo, GrRenderable::kYes);
        if (!backendTex.isValid()) {
            continue;
        }
        SkScopeExit freeTex([&backendTex, context] {
            context->deleteBackendTexture(backendTex);
        });
        auto info = SkImageInfo::Make(kSize, kSize, colorType, kOpaque_SkAlphaType, nullptr);
        auto surf = SkSurface::MakeFromBackendTexture(context, backendTex,
                                                      kTopLeft_GrSurfaceOrigin, max,
                                                      colorType, nullptr, nullptr);
        REPORTER_ASSERT(reporter, surf);
        if (!surf) {
            continue;
        }
        int sampleCnt = ((SkSurface_Gpu*)(surf.get()))
                                ->getDevice()
                                ->accessRenderTargetContext()
                                ->numStencilSamples();
        REPORTER_ASSERT(reporter, sampleCnt == max, "Exected: %d, actual: %d", max, sampleCnt);
    }
}

static void test_canvas_peek(skiatest::Reporter* reporter,
                             sk_sp<SkSurface>& surface,
                             const SkImageInfo& requestInfo,
                             bool expectPeekSuccess) {
    const SkColor color = SK_ColorRED;
    const SkPMColor pmcolor = SkPreMultiplyColor(color);
    surface->getCanvas()->clear(color);

    SkPixmap pmap;
    bool success = surface->getCanvas()->peekPixels(&pmap);
    REPORTER_ASSERT(reporter, expectPeekSuccess == success);

    SkPixmap pmap2;
    const void* addr2 = surface->peekPixels(&pmap2) ? pmap2.addr() : nullptr;

    if (success) {
        REPORTER_ASSERT(reporter, requestInfo == pmap.info());
        REPORTER_ASSERT(reporter, requestInfo.minRowBytes() <= pmap.rowBytes());
        REPORTER_ASSERT(reporter, pmcolor == *pmap.addr32());

        REPORTER_ASSERT(reporter, pmap.addr() == pmap2.addr());
        REPORTER_ASSERT(reporter, pmap.info() == pmap2.info());
        REPORTER_ASSERT(reporter, pmap.rowBytes() == pmap2.rowBytes());
    } else {
        REPORTER_ASSERT(reporter, nullptr == addr2);
    }
}
DEF_TEST(SurfaceCanvasPeek, reporter) {
    for (auto& surface_func : { &create_surface, &create_direct_surface }) {
        SkImageInfo requestInfo;
        auto surface(surface_func(kPremul_SkAlphaType, &requestInfo));
        test_canvas_peek(reporter, surface, requestInfo, true);
    }
}
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(SurfaceCanvasPeek_Gpu, reporter, ctxInfo) {
    for (auto& surface_func : { &create_gpu_surface, &create_gpu_scratch_surface }) {
        SkImageInfo requestInfo;
        auto surface(surface_func(ctxInfo.grContext(), kPremul_SkAlphaType, &requestInfo));
        test_canvas_peek(reporter, surface, requestInfo, false);
    }
}

static void test_snapshot_alphatype(skiatest::Reporter* reporter, const sk_sp<SkSurface>& surface,
                                    SkAlphaType expectedAlphaType) {
    REPORTER_ASSERT(reporter, surface);
    if (surface) {
        sk_sp<SkImage> image(surface->makeImageSnapshot());
        REPORTER_ASSERT(reporter, image);
        if (image) {
            REPORTER_ASSERT(reporter, image->alphaType() == expectedAlphaType);
        }
    }
}
DEF_TEST(SurfaceSnapshotAlphaType, reporter) {
    for (auto& surface_func : { &create_surface, &create_direct_surface }) {
        for (auto& at: { kOpaque_SkAlphaType, kPremul_SkAlphaType, kUnpremul_SkAlphaType }) {
            auto surface(surface_func(at, nullptr));
            test_snapshot_alphatype(reporter, surface, at);
        }
    }
}
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(SurfaceSnapshotAlphaType_Gpu, reporter, ctxInfo) {
    for (auto& surface_func : { &create_gpu_surface, &create_gpu_scratch_surface }) {
        // GPU doesn't support creating unpremul surfaces, so only test opaque + premul
        for (auto& at : { kOpaque_SkAlphaType, kPremul_SkAlphaType }) {
            auto surface(surface_func(ctxInfo.grContext(), at, nullptr));
            test_snapshot_alphatype(reporter, surface, at);
        }
    }
}

static void test_backend_texture_access_copy_on_write(
    skiatest::Reporter* reporter, SkSurface* surface, SkSurface::BackendHandleAccess access) {
    GrBackendTexture tex1 = surface->getBackendTexture(access);
    sk_sp<SkImage> snap1(surface->makeImageSnapshot());

    GrBackendTexture tex2 = surface->getBackendTexture(access);
    sk_sp<SkImage> snap2(surface->makeImageSnapshot());

    // If the access mode triggers CoW, then the backend objects should reflect it.
    REPORTER_ASSERT(reporter, GrBackendTexture::TestingOnly_Equals(tex1, tex2) == (snap1 == snap2));
}

static void test_backend_rendertarget_access_copy_on_write(
    skiatest::Reporter* reporter, SkSurface* surface, SkSurface::BackendHandleAccess access) {
    GrBackendRenderTarget rt1 = surface->getBackendRenderTarget(access);
    sk_sp<SkImage> snap1(surface->makeImageSnapshot());

    GrBackendRenderTarget rt2 = surface->getBackendRenderTarget(access);
    sk_sp<SkImage> snap2(surface->makeImageSnapshot());

    // If the access mode triggers CoW, then the backend objects should reflect it.
    REPORTER_ASSERT(reporter, GrBackendRenderTarget::TestingOnly_Equals(rt1, rt2) ==
                              (snap1 == snap2));
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(SurfaceBackendSurfaceAccessCopyOnWrite_Gpu, reporter, ctxInfo) {
    const SkSurface::BackendHandleAccess accessModes[] = {
        SkSurface::kFlushRead_BackendHandleAccess,
        SkSurface::kFlushWrite_BackendHandleAccess,
        SkSurface::kDiscardWrite_BackendHandleAccess,
    };

    for (auto& surface_func : { &create_gpu_surface, &create_gpu_scratch_surface }) {
        for (auto& accessMode : accessModes) {
            {
                auto surface(surface_func(ctxInfo.grContext(), kPremul_SkAlphaType, nullptr));
                test_backend_texture_access_copy_on_write(reporter, surface.get(), accessMode);
            }
            {
                auto surface(surface_func(ctxInfo.grContext(), kPremul_SkAlphaType, nullptr));
                test_backend_rendertarget_access_copy_on_write(reporter, surface.get(), accessMode);
            }
        }
    }
}

template<typename Type, Type(SkSurface::*func)(SkSurface::BackendHandleAccess)>
static void test_backend_unique_id(skiatest::Reporter* reporter, SkSurface* surface) {
    sk_sp<SkImage> image0(surface->makeImageSnapshot());

    Type obj = (surface->*func)(SkSurface::kFlushRead_BackendHandleAccess);
    REPORTER_ASSERT(reporter, obj.isValid());
    sk_sp<SkImage> image1(surface->makeImageSnapshot());
    // just read access should not affect the snapshot
    REPORTER_ASSERT(reporter, image0->uniqueID() == image1->uniqueID());

    obj = (surface->*func)(SkSurface::kFlushWrite_BackendHandleAccess);
    REPORTER_ASSERT(reporter, obj.isValid());
    sk_sp<SkImage> image2(surface->makeImageSnapshot());
    // expect a new image, since we claimed we would write
    REPORTER_ASSERT(reporter, image0->uniqueID() != image2->uniqueID());

    obj = (surface->*func)(SkSurface::kDiscardWrite_BackendHandleAccess);
    REPORTER_ASSERT(reporter, obj.isValid());
    sk_sp<SkImage> image3(surface->makeImageSnapshot());
    // expect a new(er) image, since we claimed we would write
    REPORTER_ASSERT(reporter, image0->uniqueID() != image3->uniqueID());
    REPORTER_ASSERT(reporter, image2->uniqueID() != image3->uniqueID());
}

// No CPU test.
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(SurfaceBackendHandleAccessIDs_Gpu, reporter, ctxInfo) {
    for (auto& surface_func : { &create_gpu_surface, &create_gpu_scratch_surface }) {
        {
            auto surface(surface_func(ctxInfo.grContext(), kPremul_SkAlphaType, nullptr));
            test_backend_unique_id<GrBackendTexture, &SkSurface::getBackendTexture>(reporter,
                                                                                    surface.get());
        }
        {
            auto surface(surface_func(ctxInfo.grContext(), kPremul_SkAlphaType, nullptr));
            test_backend_unique_id<GrBackendRenderTarget, &SkSurface::getBackendRenderTarget>(
                                                                reporter, surface.get());
        }
    }
}

// Verify that the right canvas commands trigger a copy on write.
static void test_copy_on_write(skiatest::Reporter* reporter, SkSurface* surface) {
    SkCanvas* canvas = surface->getCanvas();

    const SkRect testRect =
        SkRect::MakeXYWH(SkIntToScalar(0), SkIntToScalar(0),
                         SkIntToScalar(4), SkIntToScalar(5));
    SkPath testPath;
    testPath.addRect(SkRect::MakeXYWH(SkIntToScalar(0), SkIntToScalar(0),
                                      SkIntToScalar(2), SkIntToScalar(1)));

    const SkIRect testIRect = SkIRect::MakeXYWH(0, 0, 2, 1);

    SkRegion testRegion;
    testRegion.setRect(testIRect);


    const SkColor testColor = 0x01020304;
    const SkPaint testPaint;
    const SkPoint testPoints[3] = {
        {SkIntToScalar(0), SkIntToScalar(0)},
        {SkIntToScalar(2), SkIntToScalar(1)},
        {SkIntToScalar(0), SkIntToScalar(2)}
    };
    const size_t testPointCount = 3;

    SkBitmap testBitmap;
    testBitmap.allocN32Pixels(10, 10);
    testBitmap.eraseColor(0);

    SkRRect testRRect;
    testRRect.setRectXY(testRect, SK_Scalar1, SK_Scalar1);

    SkString testText("Hello World");

#define EXPECT_COPY_ON_WRITE(command)                               \
    {                                                               \
        sk_sp<SkImage> imageBefore = surface->makeImageSnapshot();  \
        sk_sp<SkImage> aur_before(imageBefore);                     \
        canvas-> command ;                                          \
        sk_sp<SkImage> imageAfter = surface->makeImageSnapshot();   \
        sk_sp<SkImage> aur_after(imageAfter);                       \
        REPORTER_ASSERT(reporter, imageBefore != imageAfter);       \
    }

    EXPECT_COPY_ON_WRITE(clear(testColor))
    EXPECT_COPY_ON_WRITE(drawPaint(testPaint))
    EXPECT_COPY_ON_WRITE(drawPoints(SkCanvas::kPoints_PointMode, testPointCount, testPoints, \
        testPaint))
    EXPECT_COPY_ON_WRITE(drawOval(testRect, testPaint))
    EXPECT_COPY_ON_WRITE(drawRect(testRect, testPaint))
    EXPECT_COPY_ON_WRITE(drawRRect(testRRect, testPaint))
    EXPECT_COPY_ON_WRITE(drawPath(testPath, testPaint))
    EXPECT_COPY_ON_WRITE(drawBitmap(testBitmap, 0, 0))
    EXPECT_COPY_ON_WRITE(drawBitmapRect(testBitmap, testRect, nullptr))
    EXPECT_COPY_ON_WRITE(drawBitmapNine(testBitmap, testIRect, testRect, nullptr))
    EXPECT_COPY_ON_WRITE(drawString(testText, 0, 1, SkFont(), testPaint))
}
DEF_TEST(SurfaceCopyOnWrite, reporter) {
    test_copy_on_write(reporter, create_surface().get());
}
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(SurfaceCopyOnWrite_Gpu, reporter, ctxInfo) {
    for (auto& surface_func : { &create_gpu_surface, &create_gpu_scratch_surface }) {
        auto surface(surface_func(ctxInfo.grContext(), kPremul_SkAlphaType, nullptr));
        test_copy_on_write(reporter, surface.get());
    }
}

static void test_writable_after_snapshot_release(skiatest::Reporter* reporter,
                                                 SkSurface* surface) {
    // This test succeeds by not triggering an assertion.
    // The test verifies that the surface remains writable (usable) after
    // acquiring and releasing a snapshot without triggering a copy on write.
    SkCanvas* canvas = surface->getCanvas();
    canvas->clear(1);
    surface->makeImageSnapshot();  // Create and destroy SkImage
    canvas->clear(2);  // Must not assert internally
}
DEF_TEST(SurfaceWriteableAfterSnapshotRelease, reporter) {
    test_writable_after_snapshot_release(reporter, create_surface().get());
}
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(SurfaceWriteableAfterSnapshotRelease_Gpu, reporter, ctxInfo) {
    for (auto& surface_func : { &create_gpu_surface, &create_gpu_scratch_surface }) {
        auto surface(surface_func(ctxInfo.grContext(), kPremul_SkAlphaType, nullptr));
        test_writable_after_snapshot_release(reporter, surface.get());
    }
}

static void test_crbug263329(skiatest::Reporter* reporter,
                             SkSurface* surface1,
                             SkSurface* surface2) {
    // This is a regression test for crbug.com/263329
    // Bug was caused by onCopyOnWrite releasing the old surface texture
    // back to the scratch texture pool even though the texture is used
    // by and active SkImage_Gpu.
    SkCanvas* canvas1 = surface1->getCanvas();
    SkCanvas* canvas2 = surface2->getCanvas();
    canvas1->clear(1);
    sk_sp<SkImage> image1(surface1->makeImageSnapshot());
    // Trigger copy on write, new backing is a scratch texture
    canvas1->clear(2);
    sk_sp<SkImage> image2(surface1->makeImageSnapshot());
    // Trigger copy on write, old backing should not be returned to scratch
    // pool because it is held by image2
    canvas1->clear(3);

    canvas2->clear(4);
    sk_sp<SkImage> image3(surface2->makeImageSnapshot());
    // Trigger copy on write on surface2. The new backing store should not
    // be recycling a texture that is held by an existing image.
    canvas2->clear(5);
    sk_sp<SkImage> image4(surface2->makeImageSnapshot());
    REPORTER_ASSERT(reporter, as_IB(image4)->getTexture() != as_IB(image3)->getTexture());
    // The following assertion checks crbug.com/263329
    REPORTER_ASSERT(reporter, as_IB(image4)->getTexture() != as_IB(image2)->getTexture());
    REPORTER_ASSERT(reporter, as_IB(image4)->getTexture() != as_IB(image1)->getTexture());
    REPORTER_ASSERT(reporter, as_IB(image3)->getTexture() != as_IB(image2)->getTexture());
    REPORTER_ASSERT(reporter, as_IB(image3)->getTexture() != as_IB(image1)->getTexture());
    REPORTER_ASSERT(reporter, as_IB(image2)->getTexture() != as_IB(image1)->getTexture());
}
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(SurfaceCRBug263329_Gpu, reporter, ctxInfo) {
    for (auto& surface_func : { &create_gpu_surface, &create_gpu_scratch_surface }) {
        auto surface1(surface_func(ctxInfo.grContext(), kPremul_SkAlphaType, nullptr));
        auto surface2(surface_func(ctxInfo.grContext(), kPremul_SkAlphaType, nullptr));
        test_crbug263329(reporter, surface1.get(), surface2.get());
    }
}

DEF_TEST(SurfaceGetTexture, reporter) {
    auto surface(create_surface());
    sk_sp<SkImage> image(surface->makeImageSnapshot());
    REPORTER_ASSERT(reporter, !as_IB(image)->isTextureBacked());
    surface->notifyContentWillChange(SkSurface::kDiscard_ContentChangeMode);
    REPORTER_ASSERT(reporter, !as_IB(image)->isTextureBacked());
}
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(SurfacepeekTexture_Gpu, reporter, ctxInfo) {
    for (auto& surface_func : { &create_gpu_surface, &create_gpu_scratch_surface }) {
        auto surface(surface_func(ctxInfo.grContext(), kPremul_SkAlphaType, nullptr));
        sk_sp<SkImage> image(surface->makeImageSnapshot());

        REPORTER_ASSERT(reporter, as_IB(image)->isTextureBacked());
        GrBackendTexture backendTex = image->getBackendTexture(false);
        REPORTER_ASSERT(reporter, backendTex.isValid());
        surface->notifyContentWillChange(SkSurface::kDiscard_ContentChangeMode);
        REPORTER_ASSERT(reporter, as_IB(image)->isTextureBacked());
        GrBackendTexture backendTex2 = image->getBackendTexture(false);
        REPORTER_ASSERT(reporter, GrBackendTexture::TestingOnly_Equals(backendTex, backendTex2));
    }
}

static SkBudgeted is_budgeted(const sk_sp<SkSurface>& surf) {
    SkSurface_Gpu* gsurf = (SkSurface_Gpu*)surf.get();

    GrRenderTargetProxy* proxy = gsurf->getDevice()->accessRenderTargetContext()
                                                                        ->asRenderTargetProxy();
    return proxy->isBudgeted();
}

static SkBudgeted is_budgeted(SkImage* image) {
    return ((SkImage_Gpu*)image)->peekProxy()->isBudgeted();
}

static SkBudgeted is_budgeted(const sk_sp<SkImage> image) {
    return is_budgeted(image.get());
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(SurfaceBudget, reporter, ctxInfo) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(8,8);
    for (auto budgeted : { SkBudgeted::kNo, SkBudgeted::kYes }) {
        auto surface(SkSurface::MakeRenderTarget(ctxInfo.grContext(), budgeted, info));
        SkASSERT(surface);
        REPORTER_ASSERT(reporter, budgeted == is_budgeted(surface));

        sk_sp<SkImage> image(surface->makeImageSnapshot());

        // Initially the image shares a texture with the surface, and the
        // the budgets should always match.
        REPORTER_ASSERT(reporter, budgeted == is_budgeted(surface));
        REPORTER_ASSERT(reporter, budgeted == is_budgeted(image));

        // Now trigger copy-on-write
        surface->getCanvas()->clear(SK_ColorBLUE);

        // They don't share a texture anymore but the budgets should still match.
        REPORTER_ASSERT(reporter, budgeted == is_budgeted(surface));
        REPORTER_ASSERT(reporter, budgeted == is_budgeted(image));
    }
}

static void test_no_canvas1(skiatest::Reporter* reporter,
                            SkSurface* surface,
                            SkSurface::ContentChangeMode mode) {
    // Test passes by not asserting
    surface->notifyContentWillChange(mode);
}
static void test_no_canvas2(skiatest::Reporter* reporter,
                            SkSurface* surface,
                            SkSurface::ContentChangeMode mode) {
    // Verifies the robustness of SkSurface for handling use cases where calls
    // are made before a canvas is created.
    sk_sp<SkImage> image1 = surface->makeImageSnapshot();
    sk_sp<SkImage> aur_image1(image1);
    surface->notifyContentWillChange(mode);
    sk_sp<SkImage> image2 = surface->makeImageSnapshot();
    sk_sp<SkImage> aur_image2(image2);
    REPORTER_ASSERT(reporter, image1 != image2);
}
DEF_TEST(SurfaceNoCanvas, reporter) {
    SkSurface::ContentChangeMode modes[] =
            { SkSurface::kDiscard_ContentChangeMode, SkSurface::kRetain_ContentChangeMode};
    for (auto& test_func : { &test_no_canvas1, &test_no_canvas2 }) {
        for (auto& mode : modes) {
            test_func(reporter, create_surface().get(), mode);
        }
    }
}
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(SurfaceNoCanvas_Gpu, reporter, ctxInfo) {
    SkSurface::ContentChangeMode modes[] =
            { SkSurface::kDiscard_ContentChangeMode, SkSurface::kRetain_ContentChangeMode};
    for (auto& surface_func : { &create_gpu_surface, &create_gpu_scratch_surface }) {
        for (auto& test_func : { &test_no_canvas1, &test_no_canvas2 }) {
            for (auto& mode : modes) {
                auto surface(surface_func(ctxInfo.grContext(), kPremul_SkAlphaType, nullptr));
                test_func(reporter, surface.get(), mode);
            }
        }
    }
}

static void check_rowbytes_remain_consistent(SkSurface* surface, skiatest::Reporter* reporter) {
    SkPixmap surfacePM;
    REPORTER_ASSERT(reporter, surface->peekPixels(&surfacePM));

    sk_sp<SkImage> image(surface->makeImageSnapshot());
    SkPixmap pm;
    REPORTER_ASSERT(reporter, image->peekPixels(&pm));

    REPORTER_ASSERT(reporter, surfacePM.rowBytes() == pm.rowBytes());

    // trigger a copy-on-write
    surface->getCanvas()->drawPaint(SkPaint());
    sk_sp<SkImage> image2(surface->makeImageSnapshot());
    REPORTER_ASSERT(reporter, image->uniqueID() != image2->uniqueID());

    SkPixmap pm2;
    REPORTER_ASSERT(reporter, image2->peekPixels(&pm2));
    REPORTER_ASSERT(reporter, pm2.rowBytes() == pm.rowBytes());
}

DEF_TEST(surface_rowbytes, reporter) {
    const SkImageInfo info = SkImageInfo::MakeN32Premul(100, 100);

    auto surf0(SkSurface::MakeRaster(info));
    check_rowbytes_remain_consistent(surf0.get(), reporter);

    // specify a larger rowbytes
    auto surf1(SkSurface::MakeRaster(info, 500, nullptr));
    check_rowbytes_remain_consistent(surf1.get(), reporter);

    // Try some illegal rowByte values
    auto s = SkSurface::MakeRaster(info, 396, nullptr);    // needs to be at least 400
    REPORTER_ASSERT(reporter, nullptr == s);
    s = SkSurface::MakeRaster(info, std::numeric_limits<size_t>::max(), nullptr);
    REPORTER_ASSERT(reporter, nullptr == s);
}

DEF_TEST(surface_raster_zeroinitialized, reporter) {
    sk_sp<SkSurface> s(SkSurface::MakeRasterN32Premul(100, 100));
    SkPixmap pixmap;
    REPORTER_ASSERT(reporter, s->peekPixels(&pixmap));

    for (int i = 0; i < pixmap.info().width(); ++i) {
        for (int j = 0; j < pixmap.info().height(); ++j) {
            REPORTER_ASSERT(reporter, *pixmap.addr32(i, j) == 0);
        }
    }
}

static sk_sp<SkSurface> create_gpu_surface_backend_texture(
    GrContext* ctx, int sampleCnt, SkColor color, GrBackendTexture* outTexture) {

    // On Pixel and Pixel2XL's with Adreno 530 and 540s, setting width and height to 10s reliably
    // triggers what appears to be a driver race condition where the 10x10 surface from the
    // OverdrawSurface_gpu test is reused(?) for this surface created by the SurfacePartialDraw_gpu
    // test.
    //
    // Immediately after creation of this surface, readback shows the correct initial solid color.
    // However, sometime before content is rendered into the upper half of the surface, the driver
    // presumably cleans up the OverdrawSurface_gpu's memory which corrupts this color buffer. The
    // top half of the surface is fine after the partially-covering rectangle is drawn, but the
    // untouched bottom half contains random pixel values that trigger asserts in the
    // SurfacePartialDraw_gpu test for no longer matching the initial color. Running the
    // SurfacePartialDraw_gpu test without the OverdrawSurface_gpu test completes successfully.
    //
    // Requesting a much larger backend texture size seems to prevent it from reusing the same
    // memory and avoids the issue.
#if defined(SK_BUILD_FOR_SKQP)
    const int kWidth = 10;
    const int kHeight = 10;
#else
    const int kWidth = 100;
    const int kHeight = 100;
#endif

    SkImageInfo ii = SkImageInfo::Make(kWidth, kHeight, SkColorType::kRGBA_8888_SkColorType,
                                       kPremul_SkAlphaType);

    if (!create_backend_texture(ctx, outTexture, ii, GrMipMapped::kNo, color,
                                GrRenderable::kYes)) {
        return nullptr;
    }

    sk_sp<SkSurface> surface = SkSurface::MakeFromBackendTexture(ctx, *outTexture,
                                                                 kTopLeft_GrSurfaceOrigin, sampleCnt,
                                                                 kRGBA_8888_SkColorType,
                                                                 nullptr, nullptr);
    if (!surface) {
        delete_backend_texture(ctx, *outTexture);
        return nullptr;
    }
    return surface;
}

static sk_sp<SkSurface> create_gpu_surface_backend_texture_as_render_target(
    GrContext* ctx, int sampleCnt, SkColor color, GrBackendTexture* outTexture) {

    const int kWidth = 10;
    const int kHeight = 10;

    SkImageInfo ii = SkImageInfo::Make(kWidth, kHeight, SkColorType::kRGBA_8888_SkColorType,
                                       kPremul_SkAlphaType);

    if (!create_backend_texture(ctx, outTexture, ii, GrMipMapped::kNo, color,
                                GrRenderable::kYes)) {
        return nullptr;
    }

    sk_sp<SkSurface> surface = SkSurface::MakeFromBackendTextureAsRenderTarget(
            ctx, *outTexture, kTopLeft_GrSurfaceOrigin, sampleCnt, kRGBA_8888_SkColorType,
            nullptr, nullptr);

    if (!surface) {
        delete_backend_texture(ctx, *outTexture);
        return nullptr;
    }
    return surface;
}

static void test_surface_clear(skiatest::Reporter* reporter, sk_sp<SkSurface> surface,
                               std::function<sk_sp<GrSurfaceContext>(SkSurface*)> grSurfaceGetter,
                               uint32_t expectedValue) {
    if (!surface) {
        ERRORF(reporter, "Could not create GPU SkSurface.");
        return;
    }
    int w = surface->width();
    int h = surface->height();
    std::unique_ptr<uint32_t[]> pixels(new uint32_t[w * h]);
    sk_memset32(pixels.get(), ~expectedValue, w * h);

    sk_sp<GrSurfaceContext> grSurfaceContext(grSurfaceGetter(surface.get()));
    if (!grSurfaceContext) {
        ERRORF(reporter, "Could access render target of GPU SkSurface.");
        return;
    }
    surface.reset();

    SkImageInfo ii = SkImageInfo::Make(w, h, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    grSurfaceContext->readPixels(ii, pixels.get(), 0, 0, 0);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            uint32_t pixel = pixels.get()[y * w + x];
            if (pixel != expectedValue) {
                SkString msg;
                if (expectedValue) {
                    msg = "SkSurface should have left render target unmodified";
                } else {
                    msg = "SkSurface should have cleared the render target";
                }
                ERRORF(reporter,
                       "%s but read 0x%08x (instead of 0x%08x) at %x,%d", msg.c_str(), pixel,
                       expectedValue, x, y);
                return;
            }
        }
    }
}

DEF_GPUTEST_FOR_GL_RENDERING_CONTEXTS(SurfaceClear_Gpu, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();

    std::function<sk_sp<GrSurfaceContext>(SkSurface*)> grSurfaceContextGetters[] = {
        [] (SkSurface* s){
            return sk_ref_sp(s->getCanvas()->internal_private_accessTopLayerRenderTargetContext());
        },
        [context] (SkSurface* s){
            sk_sp<SkImage> i(s->makeImageSnapshot());
            SkImage_Gpu* gpuImage = (SkImage_Gpu *) as_IB(i);
            sk_sp<GrTextureProxy> proxy = gpuImage->asTextureProxyRef(context);
            return context->priv().makeWrappedSurfaceContext(std::move(proxy),
                                                             gpuImage->refColorSpace());
        }
    };

    for (auto grSurfaceGetter : grSurfaceContextGetters) {
        // Test that non-wrapped RTs are created clear.
        for (auto& surface_func : {&create_gpu_surface, &create_gpu_scratch_surface}) {
            auto surface = surface_func(context, kPremul_SkAlphaType, nullptr);
            test_surface_clear(reporter, surface, grSurfaceGetter, 0x0);
        }
        // Wrapped RTs are *not* supposed to clear (to allow client to partially update a surface).
        const SkColor kOrigColor = 0xABABABAB;
        for (auto& surfaceFunc : {&create_gpu_surface_backend_texture,
                                  &create_gpu_surface_backend_texture_as_render_target}) {
            GrBackendTexture backendTex;
            auto surface = surfaceFunc(context, 1, kOrigColor, &backendTex);
            test_surface_clear(reporter, surface, grSurfaceGetter, kOrigColor);
            surface.reset();
            context->deleteBackendTexture(backendTex);
        }
    }
}

static void test_surface_draw_partially(
    skiatest::Reporter* reporter, sk_sp<SkSurface> surface, uint32_t origColor) {
    const int kW = surface->width();
    const int kH = surface->height();
    SkPaint paint;
    const SkColor kRectColor = ~origColor | 0xFF000000;
    paint.setColor(kRectColor);
    surface->getCanvas()->drawRect(SkRect::MakeWH(SkIntToScalar(kW), SkIntToScalar(kH)/2),
                                   paint);
    std::unique_ptr<uint32_t[]> pixels(new uint32_t[kW * kH]);
    sk_memset32(pixels.get(), ~origColor, kW * kH);
    // Read back RGBA to avoid format conversions that may not be supported on all platforms.
    SkImageInfo readInfo = SkImageInfo::Make(kW, kH, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    SkAssertResult(surface->readPixels(readInfo, pixels.get(), kW * sizeof(uint32_t), 0, 0));
    bool stop = false;
    SkPMColor origColorPM = SkPackARGB_as_RGBA((origColor >> 24 & 0xFF),
                                               (origColor >>  0 & 0xFF),
                                               (origColor >>  8 & 0xFF),
                                               (origColor >> 16 & 0xFF));
    SkPMColor rectColorPM = SkPackARGB_as_RGBA((kRectColor >> 24 & 0xFF),
                                               (kRectColor >> 16 & 0xFF),
                                               (kRectColor >>  8 & 0xFF),
                                               (kRectColor >>  0 & 0xFF));
    for (int y = 0; y < kH/2 && !stop; ++y) {
       for (int x = 0; x < kW && !stop; ++x) {
            REPORTER_ASSERT(reporter, rectColorPM == pixels[x + y * kW]);
            if (rectColorPM != pixels[x + y * kW]) {
                stop = true;
            }
        }
    }
    stop = false;
    for (int y = kH/2; y < kH && !stop; ++y) {
        for (int x = 0; x < kW && !stop; ++x) {
            REPORTER_ASSERT(reporter, origColorPM == pixels[x + y * kW]);
            if (origColorPM != pixels[x + y * kW]) {
                stop = true;
            }
        }
    }
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(SurfacePartialDraw_Gpu, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();

    static const SkColor kOrigColor = 0xFFAABBCC;

    for (auto& surfaceFunc : {&create_gpu_surface_backend_texture,
                              &create_gpu_surface_backend_texture_as_render_target}) {
        // Validate that we can draw to the canvas and that the original texture color is
        // preserved in pixels that aren't rendered to via the surface.
        // This works only for non-multisampled case.
        GrBackendTexture backendTex;
        auto surface = surfaceFunc(context, 1, kOrigColor, &backendTex);
        if (surface) {
            test_surface_draw_partially(reporter, surface, kOrigColor);
            surface.reset();
            context->deleteBackendTexture(backendTex);
        }
    }
}

struct ReleaseChecker {
    ReleaseChecker() : fReleaseCount(0) {}
    int fReleaseCount;
    static void Release(void* self) {
        static_cast<ReleaseChecker*>(self)->fReleaseCount++;
    }
};


DEF_GPUTEST_FOR_RENDERING_CONTEXTS(SurfaceWrappedWithRelease_Gpu, reporter, ctxInfo) {
    const int kWidth = 10;
    const int kHeight = 10;

    GrContext* ctx = ctxInfo.grContext();
    GrGpu* gpu = ctx->priv().getGpu();

    for (bool useTexture : {false, true}) {
        GrBackendTexture backendTex;
        GrBackendRenderTarget backendRT;
        sk_sp<SkSurface> surface;

        ReleaseChecker releaseChecker;
        GrSurfaceOrigin texOrigin = kBottomLeft_GrSurfaceOrigin;

        if (useTexture) {
            SkImageInfo ii = SkImageInfo::Make(kWidth, kHeight, SkColorType::kRGBA_8888_SkColorType,
                                               kPremul_SkAlphaType);
            if (!create_backend_texture(ctx, &backendTex, ii, GrMipMapped::kNo, SK_ColorRED,
                                        GrRenderable::kYes)) {
                continue;
            }

            surface = SkSurface::MakeFromBackendTexture(ctx, backendTex, texOrigin, 1,
                                                        kRGBA_8888_SkColorType,
                                                        nullptr, nullptr,
                                                        ReleaseChecker::Release,
                                                        &releaseChecker);
        } else {
            backendRT = gpu->createTestingOnlyBackendRenderTarget(kWidth, kHeight,
                                                                  GrColorType::kRGBA_8888);
            if (!backendRT.isValid()) {
                continue;
            }
            surface = SkSurface::MakeFromBackendRenderTarget(ctx, backendRT, texOrigin,
                                                             kRGBA_8888_SkColorType,
                                                             nullptr, nullptr,
                                                             ReleaseChecker::Release,
                                                             &releaseChecker);
        }
        if (!surface) {
            ERRORF(reporter, "Failed to create surface");
            continue;
        }

        surface->getCanvas()->clear(SK_ColorRED);
        surface->flush();
        gpu->testingOnly_flushGpuAndSync();

        // Now exercise the release proc
        REPORTER_ASSERT(reporter, 0 == releaseChecker.fReleaseCount);
        surface.reset(nullptr); // force a release of the surface
        REPORTER_ASSERT(reporter, 1 == releaseChecker.fReleaseCount);

        if (useTexture) {
            delete_backend_texture(ctx, backendTex);
        } else {
            gpu->deleteTestingOnlyBackendRenderTarget(backendRT);
        }
    }
}

DEF_GPUTEST_FOR_GL_RENDERING_CONTEXTS(SurfaceAttachStencil_Gpu, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    const GrCaps* caps = context->priv().caps();

    if (caps->avoidStencilBuffers()) {
        return;
    }

    static const SkColor kOrigColor = 0xFFAABBCC;

    auto resourceProvider = context->priv().resourceProvider();

    for (auto& surfaceFunc : {&create_gpu_surface_backend_texture,
                              &create_gpu_surface_backend_texture_as_render_target}) {
        for (int sampleCnt : {1, 4, 8}) {
            GrBackendTexture backendTex;
            auto surface = surfaceFunc(context, sampleCnt, kOrigColor, &backendTex);

            if (!surface && sampleCnt > 1) {
                // Certain platforms don't support MSAA, skip these.
                continue;
            }

            // Validate that we can attach a stencil buffer to an SkSurface created by either of
            // our surface functions.
            GrRenderTarget* rt = surface->getCanvas()
                ->internal_private_accessTopLayerRenderTargetContext()->accessRenderTarget();
            REPORTER_ASSERT(reporter, resourceProvider->attachStencilAttachment(rt));
            context->deleteBackendTexture(backendTex);
        }
    }
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ReplaceSurfaceBackendTexture, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();

    for (int sampleCnt : {1, 2}) {
        GrBackendTexture backendTexture1;
        auto ii = SkImageInfo::Make(10, 10, kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr);
        if (!create_backend_texture(context, &backendTexture1, ii, GrMipMapped::kNo,
                                    SK_ColorTRANSPARENT, GrRenderable::kYes)) {
            continue;
        }
        SkScopeExit delete1(
                [context, &backendTexture1] { delete_backend_texture(context, backendTexture1); });
        GrBackendTexture backendTexture2;
        if (!create_backend_texture(context, &backendTexture2, ii, GrMipMapped::kNo,
                                    SK_ColorTRANSPARENT, GrRenderable::kYes)) {
            ERRORF(reporter, "Expected to be able to make second texture");
            continue;
        }
        SkScopeExit delete2(
                [context, &backendTexture2] { delete_backend_texture(context, backendTexture2); });
        auto ii2 = ii.makeWH(8, 8);
        GrBackendTexture backendTexture3;
        if (!create_backend_texture(context, &backendTexture3, ii2, GrMipMapped::kNo,
                                    SK_ColorTRANSPARENT, GrRenderable::kYes)) {
            ERRORF(reporter, "Couldn't create different sized texture.");
            continue;
        }
        SkScopeExit delete3(
                [context, &backendTexture3] { delete_backend_texture(context, backendTexture3); });

        auto surf = SkSurface::MakeFromBackendTexture(
                context, backendTexture1, kTopLeft_GrSurfaceOrigin, sampleCnt,
                kRGBA_8888_SkColorType, ii.refColorSpace(), nullptr);
        if (!surf) {
            continue;
        }
        surf->getCanvas()->clear(SK_ColorBLUE);
        // Change matrix, layer, and clip state before swapping out the backing texture.
        surf->getCanvas()->translate(5, 5);
        surf->getCanvas()->saveLayer(nullptr, nullptr);
        surf->getCanvas()->clipRect(SkRect::MakeXYWH(0, 0, 1, 1));
        // switch origin while we're at it.
        bool replaced = surf->replaceBackendTexture(backendTexture2, kBottomLeft_GrSurfaceOrigin);
        REPORTER_ASSERT(reporter, replaced);
        SkPaint paint;
        paint.setColor(SK_ColorRED);
        surf->getCanvas()->drawRect(SkRect::MakeWH(5, 5), paint);
        surf->getCanvas()->restore();

        // Check that the replacement texture got the right color values.
        SkAutoPixmapStorage pm;
        pm.alloc(ii);
        bool bad = !surf->readPixels(pm, 0, 0);
        REPORTER_ASSERT(reporter, !bad, "Could not read surface.");
        for (int y = 0; y < ii.height() && !bad; ++y) {
            for (int x = 0; x < ii.width() && !bad; ++x) {
                auto expected = (x == 5 && y == 5) ? 0xFF0000FF : 0xFFFF0000;
                auto found = *pm.addr32(x, y);
                if (found != expected) {
                    bad = true;
                    ERRORF(reporter, "Expected color 0x%08x, found color 0x%08x at %d, %d.",
                           expected, found, x, y);
                }
            }
        }
        // The original texture should still be all blue.
        surf = SkSurface::MakeFromBackendTexture(
                context, backendTexture1, kBottomLeft_GrSurfaceOrigin, sampleCnt,
                kRGBA_8888_SkColorType, ii.refColorSpace(), nullptr);
        if (!surf) {
            ERRORF(reporter, "Could not create second surface.");
            continue;
        }
        bad = !surf->readPixels(pm, 0, 0);
        REPORTER_ASSERT(reporter, !bad, "Could not read second surface.");
        for (int y = 0; y < ii.height() && !bad; ++y) {
            for (int x = 0; x < ii.width() && !bad; ++x) {
                auto expected = 0xFFFF0000;
                auto found = *pm.addr32(x, y);
                if (found != expected) {
                    bad = true;
                    ERRORF(reporter, "Expected color 0x%08x, found color 0x%08x at %d, %d.",
                           expected, found, x, y);
                }
            }
        }

        // Can't replace with the same texture
        REPORTER_ASSERT(reporter,
                        !surf->replaceBackendTexture(backendTexture1, kTopLeft_GrSurfaceOrigin));
        // Can't replace with invalid texture
        REPORTER_ASSERT(reporter, !surf->replaceBackendTexture({}, kTopLeft_GrSurfaceOrigin));
        // Can't replace with different size texture.
        REPORTER_ASSERT(reporter,
                        !surf->replaceBackendTexture(backendTexture3, kTopLeft_GrSurfaceOrigin));
        // Can't replace texture of non-wrapped SkSurface.
        surf = SkSurface::MakeRenderTarget(context, SkBudgeted::kYes, ii, sampleCnt, nullptr);
        REPORTER_ASSERT(reporter, surf);
        if (surf) {
            REPORTER_ASSERT(reporter, !surf->replaceBackendTexture(backendTexture1,
                                                                   kTopLeft_GrSurfaceOrigin));
        }
    }
}

static void test_overdraw_surface(skiatest::Reporter* r, SkSurface* surface) {
    SkOverdrawCanvas canvas(surface->getCanvas());
    canvas.drawPaint(SkPaint());
    sk_sp<SkImage> image = surface->makeImageSnapshot();

    SkBitmap bitmap;
    image->asLegacyBitmap(&bitmap);
    for (int y = 0; y < 10; y++) {
        for (int x = 0; x < 10; x++) {
            REPORTER_ASSERT(r, 1 == SkGetPackedA32(*bitmap.getAddr32(x, y)));
        }
    }
}

DEF_TEST(OverdrawSurface_Raster, r) {
    sk_sp<SkSurface> surface = create_surface();
    test_overdraw_surface(r, surface.get());
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(OverdrawSurface_Gpu, r, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    sk_sp<SkSurface> surface = create_gpu_surface(context);
    test_overdraw_surface(r, surface.get());
}

DEF_TEST(Surface_null, r) {
    REPORTER_ASSERT(r, SkSurface::MakeNull(0, 0) == nullptr);

    const int w = 37;
    const int h = 1000;
    auto surf = SkSurface::MakeNull(w, h);
    auto canvas = surf->getCanvas();

    canvas->drawPaint(SkPaint());   // should not crash, but don't expect anything to draw
    REPORTER_ASSERT(r, surf->makeImageSnapshot() == nullptr);
}

// assert: if a given imageinfo is valid for a surface, then it must be valid for an image
//         (so the snapshot can succeed)
DEF_TEST(surface_image_unity, reporter) {
    auto do_test = [reporter](const SkImageInfo& info) {
        size_t rowBytes = info.minRowBytes();
        auto surf = SkSurface::MakeRaster(info, rowBytes, nullptr);
        if (surf) {
            auto img = surf->makeImageSnapshot();
            if (!img && false) {    // change to true to document the differences
                SkDebugf("image failed: [%08X %08X] %14s %s\n",
                         info.width(),
                         info.height(),
                         ToolUtils::colortype_name(info.colorType()),
                         ToolUtils::alphatype_name(info.alphaType()));
                return;
            }
            REPORTER_ASSERT(reporter, img != nullptr);

            char dummyPixel = 0;    // just need a valid address (not a valid size)
            SkPixmap pmap = { info, &dummyPixel, rowBytes };
            img = SkImage::MakeFromRaster(pmap, nullptr, nullptr);
            REPORTER_ASSERT(reporter, img != nullptr);
        }
    };

    const int32_t sizes[] = { -1, 0, 1, 1 << 18 };
    for (int cti = 0; cti <= kLastEnum_SkColorType; ++cti) {
        SkColorType ct = static_cast<SkColorType>(cti);
        for (int ati = 0; ati <= kLastEnum_SkAlphaType; ++ati) {
            SkAlphaType at = static_cast<SkAlphaType>(ati);
            for (int32_t size : sizes) {
                do_test(SkImageInfo::Make(1, size, ct, at));
                do_test(SkImageInfo::Make(size, 1, ct, at));
            }
        }
    }
}
