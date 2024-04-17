/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkFont.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkOverdrawCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkRegion.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkColorMatrix.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrTypes.h"
#include "include/gpu/ganesh/SkImageGanesh.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/private/SkColorData.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkFloatingPoint.h"
#include "include/private/base/SkMalloc.h"
#include "include/private/base/SkTo.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/core/SkCanvasPriv.h"
#include "src/gpu/ganesh/Device.h"
#include "src/gpu/ganesh/GrCanvas.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrColorInfo.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrGpu.h"
#include "src/gpu/ganesh/GrRenderTarget.h"
#include "src/gpu/ganesh/GrRenderTargetProxy.h"
#include "src/gpu/ganesh/GrResourceProvider.h"
#include "src/gpu/ganesh/GrTextureProxy.h"
#include "src/gpu/ganesh/SurfaceContext.h"
#include "src/gpu/ganesh/SurfaceFillContext.h"
#include "src/gpu/ganesh/image/GrImageUtils.h"
#include "src/gpu/ganesh/surface/SkSurface_Ganesh.h"
#include "src/image/SkImage_Base.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"
#include "tests/TestHarness.h"
#include "tools/RuntimeBlendUtils.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"
#include "tools/gpu/BackendSurfaceFactory.h"
#include "tools/gpu/ManagedBackendTexture.h"
#include "tools/gpu/ProxyUtils.h"

#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <limits>
#include <memory>
#include <utility>

class GrRecordingContext;
struct GrContextOptions;

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
    return SkSurfaces::Raster(info);
}
static sk_sp<SkSurface> create_direct_surface(SkAlphaType at = kPremul_SkAlphaType,
                                              SkImageInfo* requestedInfo = nullptr) {
    const SkImageInfo info = SkImageInfo::MakeN32(10, 10, at);
    if (requestedInfo) {
        *requestedInfo = info;
    }
    const size_t rowBytes = info.minRowBytes();
    void* storage = sk_malloc_throw(info.computeByteSize(rowBytes));
    return SkSurfaces::WrapPixels(info, storage, rowBytes, release_direct_surface_storage, storage);
}
static sk_sp<SkSurface> create_gpu_surface(GrRecordingContext* rContext,
                                           SkAlphaType at = kPremul_SkAlphaType,
                                           SkImageInfo* requestedInfo = nullptr) {
    const SkImageInfo info = SkImageInfo::MakeN32(10, 10, at);
    if (requestedInfo) {
        *requestedInfo = info;
    }
    return SkSurfaces::RenderTarget(rContext, skgpu::Budgeted::kNo, info);
}
static sk_sp<SkSurface> create_gpu_scratch_surface(GrRecordingContext* rContext,
                                                   SkAlphaType at = kPremul_SkAlphaType,
                                                   SkImageInfo* requestedInfo = nullptr) {
    const SkImageInfo info = SkImageInfo::MakeN32(10, 10, at);
    if (requestedInfo) {
        *requestedInfo = info;
    }
    return SkSurfaces::RenderTarget(rContext, skgpu::Budgeted::kYes, info);
}

DEF_TEST(SurfaceEmpty, reporter) {
    const SkImageInfo info = SkImageInfo::Make(0, 0, kN32_SkColorType, kPremul_SkAlphaType);
    REPORTER_ASSERT(reporter, nullptr == SkSurfaces::Raster(info));
    REPORTER_ASSERT(reporter, nullptr == SkSurfaces::WrapPixels(info, nullptr, 0));
}
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(SurfaceEmpty_Gpu,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    const SkImageInfo info = SkImageInfo::Make(0, 0, kN32_SkColorType, kPremul_SkAlphaType);
    REPORTER_ASSERT(reporter,
                    nullptr == SkSurfaces::RenderTarget(
                                       ctxInfo.directContext(), skgpu::Budgeted::kNo, info));
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(GrContext_colorTypeSupportedAsSurface,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    using namespace skgpu;

    auto context = ctxInfo.directContext();

    Protected isProtected = Protected(context->priv().caps()->supportsProtectedContent());

    for (int ct = 0; ct < kLastEnum_SkColorType; ++ct) {
        static constexpr int kSize = 10;

        SkColorType colorType = static_cast<SkColorType>(ct);
        auto info = SkImageInfo::Make(kSize, kSize, colorType, kOpaque_SkAlphaType, nullptr);

        {
            bool can = context->colorTypeSupportedAsSurface(colorType);
            auto surf = SkSurfaces::RenderTarget(context, Budgeted::kYes, info, 1, nullptr);
            REPORTER_ASSERT(reporter, can == SkToBool(surf), "ct: %d, can: %d, surf: %d",
                            colorType, can, SkToBool(surf));

            surf = sk_gpu_test::MakeBackendTextureSurface(context,
                                                          {kSize, kSize},
                                                          kTopLeft_GrSurfaceOrigin,
                                                          /*sample cnt*/ 1,
                                                          colorType,
                                                          /* colorSpace= */ nullptr,
                                                          Mipmapped::kNo,
                                                          isProtected);
            REPORTER_ASSERT(reporter, can == SkToBool(surf), "ct: %d, can: %d, surf: %d",
                            colorType, can, SkToBool(surf));
        }

        // The MSAA test only makes sense if the colorType is renderable to begin with.
        if (context->colorTypeSupportedAsSurface(colorType)) {
            static constexpr int kSampleCnt = 2;

            bool can = context->maxSurfaceSampleCountForColorType(colorType) >= kSampleCnt;
            auto surf = SkSurfaces::RenderTarget(
                    context, Budgeted::kYes, info, kSampleCnt, nullptr);
            REPORTER_ASSERT(reporter, can == SkToBool(surf), "ct: %d, can: %d, surf: %d",
                            colorType, can, SkToBool(surf));

            surf = sk_gpu_test::MakeBackendTextureSurface(
                    context, {kSize, kSize}, kTopLeft_GrSurfaceOrigin, kSampleCnt, colorType,
                    /* colorSpace= */ nullptr, Mipmapped::kNo, isProtected);
            REPORTER_ASSERT(reporter, can == SkToBool(surf),
                            "colorTypeSupportedAsSurface:%d, surf:%d, ct:%d", can, SkToBool(surf),
                            colorType);
            // Ensure that the sample count stored on the resulting SkSurface is a valid value.
            if (surf) {
                auto rtp = skgpu::ganesh::TopDeviceTargetProxy(surf->getCanvas());
                int storedCnt = rtp->numSamples();
                const GrBackendFormat& format = rtp->backendFormat();
                int allowedCnt =
                        context->priv().caps()->getRenderTargetSampleCount(storedCnt, format);
                REPORTER_ASSERT(reporter, storedCnt == allowedCnt,
                                "Should store an allowed sample count (%d vs %d)", allowedCnt,
                                storedCnt);
            }
        }

        for (int sampleCnt : {1, 2}) {
            auto surf = sk_gpu_test::MakeBackendRenderTargetSurface(context,
                                                                    {16, 16},
                                                                    kTopLeft_GrSurfaceOrigin,
                                                                    sampleCnt,
                                                                    colorType,
                                                                    /* colorSpace= */ nullptr,
                                                                    isProtected);
            bool can = context->colorTypeSupportedAsSurface(colorType) &&
                       context->maxSurfaceSampleCountForColorType(colorType) >= sampleCnt;
            if (!surf && can && colorType == kBGRA_8888_SkColorType && sampleCnt > 1 &&
                context->backend() == GrBackendApi::kOpenGL) {
                // This is an execeptional case. On iOS GLES we support MSAA BGRA for internally-
                // created render targets by using a MSAA RGBA8 renderbuffer that resolves to a
                // BGRA8 texture. However, the GL_APPLE_texture_format_BGRA8888 extension does not
                // allow creation of BGRA8 renderbuffers and we don't support multisampled textures.
                // So this is expected to fail. As of 10/5/2020 it actually seems to work to create
                // a MSAA BGRA8 renderbuffer (at least in the simulator) but we don't want to rely
                // on this undocumented behavior.
                continue;
            }
            REPORTER_ASSERT(reporter, can == SkToBool(surf), "ct: %d, sc: %d, can: %d, surf: %d",
                            colorType, sampleCnt, can, SkToBool(surf));
            if (surf) {
                auto rtp = skgpu::ganesh::TopDeviceTargetProxy(surf->getCanvas());
                int storedCnt = rtp->numSamples();
                const GrBackendFormat& backendFormat = rtp->backendFormat();
                int allowedCnt = context->priv().caps()->getRenderTargetSampleCount(storedCnt,
                                                                                    backendFormat);
                REPORTER_ASSERT(reporter, storedCnt == allowedCnt,
                                "Should store an allowed sample count (%d vs %d)", allowedCnt,
                                storedCnt);
            }
        }
    }
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(GrContext_maxSurfaceSamplesForColorType,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    using namespace skgpu;

    auto context = ctxInfo.directContext();

    Protected isProtected = Protected(context->priv().caps()->supportsProtectedContent());

    static constexpr int kSize = 10;

    for (int ct = 0; ct < kLastEnum_SkColorType; ++ct) {

        SkColorType colorType = static_cast<SkColorType>(ct);
        int maxSampleCnt = context->maxSurfaceSampleCountForColorType(colorType);
        if (!maxSampleCnt) {
            continue;
        }
        if (!context->colorTypeSupportedAsSurface(colorType)) {
            continue;
        }

        auto info = SkImageInfo::Make(kSize, kSize, colorType, kOpaque_SkAlphaType, nullptr);
        auto surf = sk_gpu_test::MakeBackendTextureSurface(
                context, info, kTopLeft_GrSurfaceOrigin, maxSampleCnt, Mipmapped::kNo, isProtected);
        if (!surf) {
            ERRORF(reporter, "Could not make surface of color type %d.", colorType);
            continue;
        }
        int sampleCnt = ((SkSurface_Ganesh*)(surf.get()))->getDevice()->targetProxy()->numSamples();
        REPORTER_ASSERT(reporter, sampleCnt == maxSampleCnt, "Exected: %d, actual: %d",
                        maxSampleCnt, sampleCnt);
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
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(SurfaceCanvasPeek_Gpu,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    for (auto& surface_func : { &create_gpu_surface, &create_gpu_scratch_surface }) {
        SkImageInfo requestInfo;
        auto surface(surface_func(ctxInfo.directContext(), kPremul_SkAlphaType, &requestInfo));
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
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(SurfaceSnapshotAlphaType_Gpu,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    for (auto& surface_func : { &create_gpu_surface, &create_gpu_scratch_surface }) {
        // GPU doesn't support creating unpremul surfaces, so only test opaque + premul
        for (auto& at : { kOpaque_SkAlphaType, kPremul_SkAlphaType }) {
            auto surface(surface_func(ctxInfo.directContext(), at, nullptr));
            test_snapshot_alphatype(reporter, surface, at);
        }
    }
}

static void test_backend_texture_access_copy_on_write(
    skiatest::Reporter* reporter, SkSurface* surface, SkSurface::BackendHandleAccess access) {
    GrBackendTexture tex1 = SkSurfaces::GetBackendTexture(surface, access);
    sk_sp<SkImage> snap1(surface->makeImageSnapshot());

    GrBackendTexture tex2 = SkSurfaces::GetBackendTexture(surface, access);
    sk_sp<SkImage> snap2(surface->makeImageSnapshot());

    // If the access mode triggers CoW, then the backend objects should reflect it.
    REPORTER_ASSERT(reporter, GrBackendTexture::TestingOnly_Equals(tex1, tex2) == (snap1 == snap2));
}

static void test_backend_rendertarget_access_copy_on_write(
    skiatest::Reporter* reporter, SkSurface* surface, SkSurface::BackendHandleAccess access) {
    GrBackendRenderTarget rt1 = SkSurfaces::GetBackendRenderTarget(surface, access);
    sk_sp<SkImage> snap1(surface->makeImageSnapshot());

    GrBackendRenderTarget rt2 = SkSurfaces::GetBackendRenderTarget(surface, access);
    sk_sp<SkImage> snap2(surface->makeImageSnapshot());

    // If the access mode triggers CoW, then the backend objects should reflect it.
    REPORTER_ASSERT(reporter, GrBackendRenderTarget::TestingOnly_Equals(rt1, rt2) ==
                              (snap1 == snap2));
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(SurfaceBackendSurfaceAccessCopyOnWrite_Gpu,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    const SkSurfaces::BackendHandleAccess accessModes[] = {
            SkSurfaces::BackendHandleAccess::kFlushRead,
            SkSurfaces::BackendHandleAccess::kFlushWrite,
            SkSurfaces::BackendHandleAccess::kDiscardWrite,
    };

    for (auto& surface_func : { &create_gpu_surface, &create_gpu_scratch_surface }) {
        for (auto& accessMode : accessModes) {
            {
                auto surface(surface_func(ctxInfo.directContext(), kPremul_SkAlphaType, nullptr));
                test_backend_texture_access_copy_on_write(reporter, surface.get(), accessMode);
            }
            {
                auto surface(surface_func(ctxInfo.directContext(), kPremul_SkAlphaType, nullptr));
                test_backend_rendertarget_access_copy_on_write(reporter, surface.get(), accessMode);
            }
        }
    }
}

template <typename Type, Type (*func)(SkSurface*, SkSurface::BackendHandleAccess)>
static void test_backend_unique_id(skiatest::Reporter* reporter, SkSurface* surface) {
    sk_sp<SkImage> image0(surface->makeImageSnapshot());

    Type obj = func(surface, SkSurfaces::BackendHandleAccess::kFlushRead);
    REPORTER_ASSERT(reporter, obj.isValid());
    sk_sp<SkImage> image1(surface->makeImageSnapshot());
    // just read access should not affect the snapshot
    REPORTER_ASSERT(reporter, image0->uniqueID() == image1->uniqueID());

    obj = func(surface, SkSurfaces::BackendHandleAccess::kFlushWrite);
    REPORTER_ASSERT(reporter, obj.isValid());
    sk_sp<SkImage> image2(surface->makeImageSnapshot());
    // expect a new image, since we claimed we would write
    REPORTER_ASSERT(reporter, image0->uniqueID() != image2->uniqueID());

    obj = func(surface, SkSurfaces::BackendHandleAccess::kDiscardWrite);
    REPORTER_ASSERT(reporter, obj.isValid());
    sk_sp<SkImage> image3(surface->makeImageSnapshot());
    // expect a new(er) image, since we claimed we would write
    REPORTER_ASSERT(reporter, image0->uniqueID() != image3->uniqueID());
    REPORTER_ASSERT(reporter, image2->uniqueID() != image3->uniqueID());
}

// No CPU test.
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(SurfaceBackendHandleAccessIDs_Gpu,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    for (auto& surface_func : { &create_gpu_surface, &create_gpu_scratch_surface }) {
        {
            auto surface(surface_func(ctxInfo.directContext(), kPremul_SkAlphaType, nullptr));
            test_backend_unique_id<GrBackendTexture, &SkSurfaces::GetBackendTexture>(reporter,
                                                                                     surface.get());
        }
        {
            auto surface(surface_func(ctxInfo.directContext(), kPremul_SkAlphaType, nullptr));
            test_backend_unique_id<GrBackendRenderTarget, &SkSurfaces::GetBackendRenderTarget>(
                    reporter, surface.get());
        }
    }
}

// No CPU test.
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(SurfaceAbandonPostFlush_Gpu,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    auto direct = ctxInfo.directContext();
    sk_sp<SkSurface> surface = create_gpu_surface(direct, kPremul_SkAlphaType, nullptr);
    if (!surface) {
        return;
    }
    // This flush can put command buffer refs on the GrGpuResource for the surface.
    direct->flush(surface.get());
    direct->abandonContext();
    // We pass the test if we don't hit any asserts or crashes when the ref on the surface goes away
    // after we abanonded the context. One thing specifically this checks is to make sure we're
    // correctly handling the mix of normal refs and command buffer refs, and correctly deleting
    // the object at the right time.
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(SurfaceBackendAccessAbandoned_Gpu,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    auto dContext = ctxInfo.directContext();
    sk_sp<SkSurface> surface = create_gpu_surface(dContext, kPremul_SkAlphaType, nullptr);
    if (!surface) {
        return;
    }

    GrBackendRenderTarget beRT = SkSurfaces::GetBackendRenderTarget(
            surface.get(), SkSurfaces::BackendHandleAccess::kFlushRead);
    REPORTER_ASSERT(reporter, beRT.isValid());
    GrBackendTexture beTex = SkSurfaces::GetBackendTexture(
            surface.get(), SkSurfaces::BackendHandleAccess::kFlushRead);
    REPORTER_ASSERT(reporter, beTex.isValid());

    dContext->flush(surface.get());
    dContext->abandonContext();

    // After abandoning the context none of the backend surfaces should be valid.
    beRT = SkSurfaces::GetBackendRenderTarget(surface.get(),
                                              SkSurfaces::BackendHandleAccess::kFlushRead);
    REPORTER_ASSERT(reporter, !beRT.isValid());
    beTex = SkSurfaces::GetBackendTexture(surface.get(),
                                          SkSurfaces::BackendHandleAccess::kFlushRead);
    REPORTER_ASSERT(reporter, !beTex.isValid());
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
        sk_sp<SkImage> aur_before(imageBefore);  /*NOLINT*/         \
        canvas-> command ;                                          \
        sk_sp<SkImage> imageAfter = surface->makeImageSnapshot();   \
        sk_sp<SkImage> aur_after(imageAfter);  /*NOLINT*/           \
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
    EXPECT_COPY_ON_WRITE(drawImage(testBitmap.asImage(), 0, 0))
    EXPECT_COPY_ON_WRITE(drawImageRect(testBitmap.asImage(), testRect, SkSamplingOptions()))
    EXPECT_COPY_ON_WRITE(drawString(testText, 0, 1, ToolUtils::DefaultPortableFont(), testPaint))
}
DEF_TEST(SurfaceCopyOnWrite, reporter) {
    test_copy_on_write(reporter, create_surface().get());
}
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(SurfaceCopyOnWrite_Gpu,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    for (auto& surface_func : { &create_gpu_surface, &create_gpu_scratch_surface }) {
        auto surface(surface_func(ctxInfo.directContext(), kPremul_SkAlphaType, nullptr));
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
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(SurfaceWriteableAfterSnapshotRelease_Gpu,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    for (auto& surface_func : { &create_gpu_surface, &create_gpu_scratch_surface }) {
        auto surface(surface_func(ctxInfo.directContext(), kPremul_SkAlphaType, nullptr));
        test_writable_after_snapshot_release(reporter, surface.get());
    }
}

static void test_crbug263329(skiatest::Reporter* reporter,
                             SkSurface* surface1,
                             SkSurface* surface2) {
    // This is a regression test for crbug.com/263329
    // Bug was caused by onCopyOnWrite releasing the old surface texture
    // back to the scratch texture pool even though the texture is used
    // by and active SkImage_Ganesh.
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

    auto imageProxy = [ctx = surface1->recordingContext()](SkImage* img) {
        GrTextureProxy* proxy = sk_gpu_test::GetTextureImageProxy(img, ctx);
        SkASSERT(proxy);
        return proxy;
    };

    REPORTER_ASSERT(reporter, imageProxy(image4.get()) != imageProxy(image3.get()));
    // The following assertion checks crbug.com/263329
    REPORTER_ASSERT(reporter, imageProxy(image4.get()) != imageProxy(image2.get()));
    REPORTER_ASSERT(reporter, imageProxy(image4.get()) != imageProxy(image1.get()));
    REPORTER_ASSERT(reporter, imageProxy(image3.get()) != imageProxy(image2.get()));
    REPORTER_ASSERT(reporter, imageProxy(image3.get()) != imageProxy(image1.get()));
    REPORTER_ASSERT(reporter, imageProxy(image2.get()) != imageProxy(image1.get()));
}
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(SurfaceCRBug263329_Gpu,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    for (auto& surface_func : { &create_gpu_surface, &create_gpu_scratch_surface }) {
        auto surface1(surface_func(ctxInfo.directContext(), kPremul_SkAlphaType, nullptr));
        auto surface2(surface_func(ctxInfo.directContext(), kPremul_SkAlphaType, nullptr));
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
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(SurfacepeekTexture_Gpu,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    for (auto& surface_func : { &create_gpu_surface, &create_gpu_scratch_surface }) {
        auto surface(surface_func(ctxInfo.directContext(), kPremul_SkAlphaType, nullptr));
        sk_sp<SkImage> image(surface->makeImageSnapshot());

        REPORTER_ASSERT(reporter, as_IB(image)->isTextureBacked());
        GrBackendTexture backendTex;
        bool ok = SkImages::GetBackendTextureFromImage(image, &backendTex, false);
        REPORTER_ASSERT(reporter, ok);
        REPORTER_ASSERT(reporter, backendTex.isValid());
        surface->notifyContentWillChange(SkSurface::kDiscard_ContentChangeMode);
        REPORTER_ASSERT(reporter, as_IB(image)->isTextureBacked());
        GrBackendTexture backendTex2;
        ok = SkImages::GetBackendTextureFromImage(image, &backendTex2, false);
        REPORTER_ASSERT(reporter, ok);
        REPORTER_ASSERT(reporter, GrBackendTexture::TestingOnly_Equals(backendTex, backendTex2));
    }
}

static skgpu::Budgeted is_budgeted(const sk_sp<SkSurface>& surf) {
    SkSurface_Ganesh* gsurf = (SkSurface_Ganesh*)surf.get();

    GrRenderTargetProxy* proxy = gsurf->getDevice()->targetProxy();
    return proxy->isBudgeted();
}

static skgpu::Budgeted is_budgeted(SkImage* image, GrRecordingContext* rc) {
    return sk_gpu_test::GetTextureImageProxy(image, rc)->isBudgeted();
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(SurfaceBudget,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(8,8);
    GrDirectContext* dContext = ctxInfo.directContext();
    for (auto budgeted : {skgpu::Budgeted::kNo, skgpu::Budgeted::kYes}) {
        auto surface(SkSurfaces::RenderTarget(dContext, budgeted, info));
        SkASSERT(surface);
        REPORTER_ASSERT(reporter, budgeted == is_budgeted(surface));

        sk_sp<SkImage> image(surface->makeImageSnapshot());

        // Initially the image shares a texture with the surface, and the
        // the budgets should always match.
        REPORTER_ASSERT(reporter, budgeted == is_budgeted(surface));
        REPORTER_ASSERT(reporter, budgeted == is_budgeted(image.get(), dContext));

        // Now trigger copy-on-write
        surface->getCanvas()->clear(SK_ColorBLUE);

        // They don't share a texture anymore but the budgets should still match.
        REPORTER_ASSERT(reporter, budgeted == is_budgeted(surface));
        REPORTER_ASSERT(reporter, budgeted == is_budgeted(image.get(), dContext));
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
    sk_sp<SkImage> aur_image1(image1);  // NOLINT(performance-unnecessary-copy-initialization)
    surface->notifyContentWillChange(mode);
    sk_sp<SkImage> image2 = surface->makeImageSnapshot();
    sk_sp<SkImage> aur_image2(image2);  // NOLINT(performance-unnecessary-copy-initialization)
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
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(SurfaceNoCanvas_Gpu,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    SkSurface::ContentChangeMode modes[] =
            { SkSurface::kDiscard_ContentChangeMode, SkSurface::kRetain_ContentChangeMode};
    for (auto& surface_func : { &create_gpu_surface, &create_gpu_scratch_surface }) {
        for (auto& test_func : { &test_no_canvas1, &test_no_canvas2 }) {
            for (auto& mode : modes) {
                auto surface(surface_func(ctxInfo.directContext(), kPremul_SkAlphaType, nullptr));
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

    auto surf0(SkSurfaces::Raster(info));
    check_rowbytes_remain_consistent(surf0.get(), reporter);

    // specify a larger rowbytes
    auto surf1(SkSurfaces::Raster(info, 500, nullptr));
    check_rowbytes_remain_consistent(surf1.get(), reporter);

    // Try some illegal rowByte values
    auto s = SkSurfaces::Raster(info, 396, nullptr);  // needs to be at least 400
    REPORTER_ASSERT(reporter, nullptr == s);
    s = SkSurfaces::Raster(info, std::numeric_limits<size_t>::max(), nullptr);
    REPORTER_ASSERT(reporter, nullptr == s);
}

DEF_TEST(surface_raster_zeroinitialized, reporter) {
    sk_sp<SkSurface> s(SkSurfaces::Raster(SkImageInfo::MakeN32Premul(100, 100)));
    SkPixmap pixmap;
    REPORTER_ASSERT(reporter, s->peekPixels(&pixmap));

    for (int i = 0; i < pixmap.info().width(); ++i) {
        for (int j = 0; j < pixmap.info().height(); ++j) {
            REPORTER_ASSERT(reporter, *pixmap.addr32(i, j) == 0);
        }
    }
}

static sk_sp<SkSurface> create_gpu_surface_backend_texture(GrDirectContext* dContext,
                                                           int sampleCnt,
                                                           const SkColor4f& color) {
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
    const SkISize kSize = CurrentTestHarnessIsSkQP() ? SkISize{10, 10} : SkISize{100, 100};

    auto surf = sk_gpu_test::MakeBackendTextureSurface(dContext,
                                                       kSize,
                                                       kTopLeft_GrSurfaceOrigin,
                                                       sampleCnt,
                                                       kRGBA_8888_SkColorType);
    if (!surf) {
        return nullptr;
    }
    surf->getCanvas()->clear(color);
    return surf;
}

static bool supports_readpixels(const GrCaps* caps, SkSurface* surface) {
    auto surfaceGpu = static_cast<SkSurface_Ganesh*>(surface);
    GrRenderTarget* rt = surfaceGpu->getDevice()->targetProxy()->peekRenderTarget();
    if (!rt) {
        return false;
    }
    return caps->surfaceSupportsReadPixels(rt) == GrCaps::SurfaceReadPixelsSupport::kSupported;
}

static sk_sp<SkSurface> create_gpu_surface_backend_render_target(GrDirectContext* dContext,
                                                                 int sampleCnt,
                                                                 const SkColor4f& color) {
    const int kWidth = 10;
    const int kHeight = 10;

    auto surf = sk_gpu_test::MakeBackendRenderTargetSurface(dContext,
                                                            {kWidth, kHeight},
                                                            kTopLeft_GrSurfaceOrigin,
                                                            sampleCnt,
                                                            kRGBA_8888_SkColorType);
    if (!surf) {
        return nullptr;
    }
    surf->getCanvas()->clear(color);
    return surf;
}

static void test_surface_context_clear(skiatest::Reporter* reporter,
                                       GrDirectContext* dContext,
                                       skgpu::ganesh::SurfaceContext* surfaceContext,
                                       uint32_t expectedValue) {
    int w = surfaceContext->width();
    int h = surfaceContext->height();

    SkImageInfo ii = SkImageInfo::Make(w, h, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    SkAutoPixmapStorage readback;
    readback.alloc(ii);

    readback.erase(~expectedValue);
    surfaceContext->readPixels(dContext, readback, {0, 0});
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            uint32_t pixel = readback.addr32()[y * w + x];
            if (pixel != expectedValue) {
                SkString msg;
                if (expectedValue) {
                    msg = "SkSurface should have left render target unmodified";
                } else {
                    msg = "SkSurface should have cleared the render target";
                }
                ERRORF(reporter,
                       "%s but read 0x%08X (instead of 0x%08X) at %d,%d", msg.c_str(), pixel,
                       expectedValue, x, y);
                return;
            }
        }
    }
}

DEF_GANESH_TEST_FOR_GL_CONTEXT(SurfaceClear_Gpu, reporter, ctxInfo, CtsEnforcement::kApiLevel_T) {
    auto dContext = ctxInfo.directContext();
    // Snaps an image from a surface and then makes a SurfaceContext from the image's texture.
    auto makeImageSurfaceContext = [dContext](SkSurface* surface) {
        sk_sp<SkImage> i(surface->makeImageSnapshot());
        auto [view, ct] = skgpu::ganesh::AsView(dContext, i, skgpu::Mipmapped::kNo);
        GrColorInfo colorInfo(ct, i->alphaType(), i->refColorSpace());
        return dContext->priv().makeSC(view, std::move(colorInfo));
    };

    // Test that non-wrapped RTs are created clear.
    for (auto& surface_func : {&create_gpu_surface, &create_gpu_scratch_surface}) {
        auto surface = surface_func(dContext, kPremul_SkAlphaType, nullptr);
        if (!surface) {
            ERRORF(reporter, "Could not create GPU SkSurface.");
            return;
        }
        auto sfc = skgpu::ganesh::TopDeviceSurfaceFillContext(surface->getCanvas());
        if (!sfc) {
            ERRORF(reporter, "Could access surface context of GPU SkSurface.");
            return;
        }
        test_surface_context_clear(reporter, dContext, sfc, 0x0);
        auto imageSurfaceCtx = makeImageSurfaceContext(surface.get());
        test_surface_context_clear(reporter, dContext, imageSurfaceCtx.get(), 0x0);
    }

    // Wrapped RTs are *not* supposed to clear (to allow client to partially update a surface).
    const SkColor4f kOrigColor{.67f, .67f, .67f, 1};
    for (auto& surfaceFunc :
         {&create_gpu_surface_backend_texture, &create_gpu_surface_backend_render_target}) {
        auto surface = surfaceFunc(dContext, 1, kOrigColor);
        if (!surface) {
            ERRORF(reporter, "Could not create GPU SkSurface.");
            return;
        }
        auto sfc = skgpu::ganesh::TopDeviceSurfaceFillContext(surface->getCanvas());
        if (!sfc) {
            ERRORF(reporter, "Could access surface context of GPU SkSurface.");
            return;
        }
        test_surface_context_clear(reporter, dContext, sfc, kOrigColor.toSkColor());
        auto imageSurfaceCtx = makeImageSurfaceContext(surface.get());
        test_surface_context_clear(reporter, dContext, imageSurfaceCtx.get(),
                                   kOrigColor.toSkColor());
    }
}

static void test_surface_draw_partially(
    skiatest::Reporter* reporter, sk_sp<SkSurface> surface, SkColor origColor) {
    const int kW = surface->width();
    const int kH = surface->height();
    SkPaint paint;
    const SkColor kRectColor = ~origColor | 0xFF000000;
    paint.setColor(kRectColor);
    surface->getCanvas()->drawRect(SkRect::MakeIWH(kW, kH/2), paint);

    // Read back RGBA to avoid format conversions that may not be supported on all platforms.
    SkImageInfo readInfo = SkImageInfo::Make(kW, kH, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    SkAutoPixmapStorage readback;
    readback.alloc(readInfo);

    readback.erase(~origColor);
    REPORTER_ASSERT(reporter, surface->readPixels(readback.info(), readback.writable_addr(),
                                                  readback.rowBytes(), 0, 0));
    bool stop = false;

    SkPMColor origColorPM = SkPackARGB_as_RGBA(SkColorGetA(origColor),
                                               SkColorGetR(origColor),
                                               SkColorGetG(origColor),
                                               SkColorGetB(origColor));
    SkPMColor rectColorPM = SkPackARGB_as_RGBA(SkColorGetA(kRectColor),
                                               SkColorGetR(kRectColor),
                                               SkColorGetG(kRectColor),
                                               SkColorGetB(kRectColor));

    for (int y = 0; y < kH/2 && !stop; ++y) {
       for (int x = 0; x < kW && !stop; ++x) {
            REPORTER_ASSERT(reporter, rectColorPM == readback.addr32()[x + y * kW]);
            if (rectColorPM != readback.addr32()[x + y * kW]) {
                SkDebugf("--- got [%x] expected [%x], x = %d, y = %d\n",
                         readback.addr32()[x + y * kW], rectColorPM, x, y);
                stop = true;
            }
        }
    }
    stop = false;
    for (int y = kH/2; y < kH && !stop; ++y) {
        for (int x = 0; x < kW && !stop; ++x) {
            REPORTER_ASSERT(reporter, origColorPM == readback.addr32()[x + y * kW]);
            if (origColorPM != readback.addr32()[x + y * kW]) {
                SkDebugf("--- got [%x] expected [%x], x = %d, y = %d\n",
                         readback.addr32()[x + y * kW], origColorPM, x, y);
                stop = true;
            }
        }
    }
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(SurfacePartialDraw_Gpu,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    auto context = ctxInfo.directContext();

    static const SkColor4f kOrigColor { 0.667f, 0.733f, 0.8f, 1 };

    for (auto& surfaceFunc :
         {&create_gpu_surface_backend_texture, &create_gpu_surface_backend_render_target}) {
        // Validate that we can draw to the canvas and that the original texture color is
        // preserved in pixels that aren't rendered to via the surface.
        // This works only for non-multisampled case.
        auto surface = surfaceFunc(context, 1, kOrigColor);
        if (surface && supports_readpixels(context->priv().caps(), surface.get())) {
            test_surface_draw_partially(reporter, surface, kOrigColor.toSkColor());
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

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(SurfaceWrappedWithRelease_Gpu,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    const int kWidth = 10;
    const int kHeight = 10;

    auto ctx = ctxInfo.directContext();
    GrGpu* gpu = ctx->priv().getGpu();

    for (bool useTexture : {false, true}) {
        sk_sp<sk_gpu_test::ManagedBackendTexture> mbet;
        GrBackendRenderTarget backendRT;
        sk_sp<SkSurface> surface;

        ReleaseChecker releaseChecker;
        GrSurfaceOrigin texOrigin = kBottomLeft_GrSurfaceOrigin;

        if (useTexture) {
            SkImageInfo ii = SkImageInfo::Make(kWidth, kHeight, SkColorType::kRGBA_8888_SkColorType,
                                               kPremul_SkAlphaType);
            mbet = sk_gpu_test::ManagedBackendTexture::MakeFromInfo(
                    ctx, ii, skgpu::Mipmapped::kNo, GrRenderable::kYes);
            if (!mbet) {
                continue;
            }

            surface = SkSurfaces::WrapBackendTexture(
                    ctx,
                    mbet->texture(),
                    texOrigin,
                    /*sample count*/ 1,
                    kRGBA_8888_SkColorType,
                    /*color space*/ nullptr,
                    /*surface props*/ nullptr,
                    sk_gpu_test::ManagedBackendTexture::ReleaseProc,
                    mbet->releaseContext(ReleaseChecker::Release, &releaseChecker));
        } else {
            backendRT = gpu->createTestingOnlyBackendRenderTarget({kWidth, kHeight},
                                                                  GrColorType::kRGBA_8888);
            if (!backendRT.isValid()) {
                continue;
            }
            surface = SkSurfaces::WrapBackendRenderTarget(ctx,
                                                          backendRT,
                                                          texOrigin,
                                                          kRGBA_8888_SkColorType,
                                                          nullptr,
                                                          nullptr,
                                                          ReleaseChecker::Release,
                                                          &releaseChecker);
        }
        if (!surface) {
            ERRORF(reporter, "Failed to create surface");
            continue;
        }

        surface->getCanvas()->clear(SK_ColorRED);
        ctx->flush(surface.get());
        ctx->submit(GrSyncCpu::kYes);

        // Now exercise the release proc
        REPORTER_ASSERT(reporter, 0 == releaseChecker.fReleaseCount);
        surface.reset(nullptr); // force a release of the surface
        REPORTER_ASSERT(reporter, 1 == releaseChecker.fReleaseCount);

        if (!useTexture) {
            gpu->deleteTestingOnlyBackendRenderTarget(backendRT);
        }
    }
}

DEF_GANESH_TEST_FOR_GL_CONTEXT(SurfaceAttachStencil_Gpu,
                               reporter,
                               ctxInfo,
                               CtsEnforcement::kApiLevel_T) {
    auto context = ctxInfo.directContext();
    const GrCaps* caps = context->priv().caps();

    if (caps->avoidStencilBuffers()) {
        return;
    }

    static const SkColor4f kOrigColor { 0.667f, 0.733f, 0.8f, 1 };

    auto resourceProvider = context->priv().resourceProvider();

    for (auto& surfaceFunc :
         {&create_gpu_surface_backend_texture, &create_gpu_surface_backend_render_target}) {
        for (int sampleCnt : {1, 4, 8}) {
            auto surface = surfaceFunc(context, sampleCnt, kOrigColor);

            if (!surface && sampleCnt > 1) {
                // Certain platforms don't support MSAA, skip these.
                continue;
            }

            // Validate that we can attach a stencil buffer to an SkSurface created by either of
            // our surface functions.
            auto rtp = skgpu::ganesh::TopDeviceTargetProxy(surface->getCanvas());
            GrRenderTarget* rt = rtp->peekRenderTarget();
            REPORTER_ASSERT(reporter,
                            resourceProvider->attachStencilAttachment(rt, rt->numSamples() > 1));
        }
    }
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(ReplaceSurfaceBackendTexture,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    auto context = ctxInfo.directContext();

    for (int sampleCnt : {1, 2}) {
        auto ii = SkImageInfo::Make(10, 10, kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr);
        auto mbet1 = sk_gpu_test::ManagedBackendTexture::MakeFromInfo(
                context, ii, skgpu::Mipmapped::kNo, GrRenderable::kYes);
        if (!mbet1) {
            continue;
        }
        auto mbet2 = sk_gpu_test::ManagedBackendTexture::MakeFromInfo(
                context, ii, skgpu::Mipmapped::kNo, GrRenderable::kYes);
        if (!mbet2) {
            ERRORF(reporter, "Expected to be able to make second texture");
            continue;
        }
        auto ii2 = ii.makeWH(8, 8);
        auto mbet3 = sk_gpu_test::ManagedBackendTexture::MakeFromInfo(
                context, ii2, skgpu::Mipmapped::kNo, GrRenderable::kYes);
        GrBackendTexture backendTexture3;
        if (!mbet3) {
            ERRORF(reporter, "Couldn't create different sized texture.");
            continue;
        }

        auto surf = SkSurfaces::WrapBackendTexture(context,
                                                   mbet1->texture(),
                                                   kTopLeft_GrSurfaceOrigin,
                                                   sampleCnt,
                                                   kRGBA_8888_SkColorType,
                                                   ii.refColorSpace(),
                                                   nullptr);
        if (!surf) {
            continue;
        }
        surf->getCanvas()->clear(SK_ColorBLUE);
        // Change matrix, layer, and clip state before swapping out the backing texture.
        surf->getCanvas()->translate(5, 5);
        surf->getCanvas()->saveLayer(nullptr, nullptr);
        surf->getCanvas()->clipRect(SkRect::MakeXYWH(0, 0, 1, 1));
        // switch origin while we're at it.
        bool replaced = surf->replaceBackendTexture(mbet2->texture(), kBottomLeft_GrSurfaceOrigin);
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
        surf = SkSurfaces::WrapBackendTexture(context,
                                              mbet1->texture(),
                                              kBottomLeft_GrSurfaceOrigin,
                                              sampleCnt,
                                              kRGBA_8888_SkColorType,
                                              ii.refColorSpace(),
                                              nullptr);
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
                        !surf->replaceBackendTexture(mbet1->texture(), kTopLeft_GrSurfaceOrigin));
        // Can't replace with invalid texture
        REPORTER_ASSERT(reporter, !surf->replaceBackendTexture({}, kTopLeft_GrSurfaceOrigin));
        // Can't replace with different size texture.
        REPORTER_ASSERT(reporter,
                        !surf->replaceBackendTexture(mbet3->texture(), kTopLeft_GrSurfaceOrigin));
        // Can't replace texture of non-wrapped SkSurface.
        surf = SkSurfaces::RenderTarget(context, skgpu::Budgeted::kYes, ii, sampleCnt, nullptr);
        REPORTER_ASSERT(reporter, surf);
        if (surf) {
            REPORTER_ASSERT(reporter, !surf->replaceBackendTexture(mbet1->texture(),
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

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(OverdrawSurface_Gpu,
                                       r,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    auto context = ctxInfo.directContext();
    sk_sp<SkSurface> surface = create_gpu_surface(context);
    test_overdraw_surface(r, surface.get());
}

DEF_TEST(Surface_null, r) {
    REPORTER_ASSERT(r, SkSurfaces::Null(0, 0) == nullptr);

    const int w = 37;
    const int h = 1000;
    auto surf = SkSurfaces::Null(w, h);
    auto canvas = surf->getCanvas();

    canvas->drawPaint(SkPaint());   // should not crash, but don't expect anything to draw
    REPORTER_ASSERT(r, surf->makeImageSnapshot() == nullptr);
}

// assert: if a given imageinfo is valid for a surface, then it must be valid for an image
//         (so the snapshot can succeed)
DEF_TEST(surface_image_unity, reporter) {
    auto do_test = [reporter](const SkImageInfo& info) {
        size_t rowBytes = info.minRowBytes();
        auto surf = SkSurfaces::Raster(info, rowBytes, nullptr);
        if (surf) {
            auto img = surf->makeImageSnapshot();
            if ((false)) { // change to true to document the differences
                if (!img) {
                    SkDebugf("image failed: [%08X %08X] %14s %s\n",
                             (uint32_t)info.width(),
                             (uint32_t)info.height(),
                             ToolUtils::colortype_name(info.colorType()),
                             ToolUtils::alphatype_name(info.alphaType()));
                    return;
                }
            }
            REPORTER_ASSERT(reporter, img != nullptr);

            char tempPixel = 0;    // just need a valid address (not a valid size)
            SkPixmap pmap = { info, &tempPixel, rowBytes };
            img = SkImages::RasterFromPixmap(pmap, nullptr, nullptr);
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
