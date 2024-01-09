/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrTypes.h"
#include "include/private/SkColorData.h"
#include "include/private/chromium/GrDeferredDisplayListRecorder.h"
#include "include/private/chromium/GrSurfaceCharacterization.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/ganesh/GrCanvas.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrDrawingManager.h"
#include "src/gpu/ganesh/GrImageInfo.h"
#include "src/gpu/ganesh/GrRenderTargetProxy.h"
#include "src/gpu/ganesh/GrRenderTask.h"
#include "src/gpu/ganesh/GrSamplerState.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"
#include "src/gpu/ganesh/GrTextureProxy.h"
#include "src/gpu/ganesh/SurfaceContext.h"
#include "src/gpu/ganesh/SurfaceFillContext.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"
#include "tests/TestUtils.h"
#include "tools/gpu/BackendSurfaceFactory.h"
#include "tools/gpu/ProxyUtils.h"

#include <functional>
#include <memory>

struct GrContextOptions;

DEF_GANESH_TEST_FOR_ALL_CONTEXTS(WrappedSurfaceCopyOnWrite,
                                 reporter,
                                 ctxInfo,
                                 CtsEnforcement::kApiLevel_T) {
    using namespace skgpu;

    GrDirectContext* dContext = ctxInfo.directContext();

    Protected isProtected = Protected(dContext->priv().caps()->supportsProtectedContent());

    auto makeDirectBackendSurface = [&]() {
        auto info = SkImageInfo::Make({10, 10}, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
        return sk_gpu_test::MakeBackendTextureSurface(dContext,
                                                      info,
                                                      kTopLeft_GrSurfaceOrigin,
                                                      /* sampleCnt= */ 1,
                                                      Mipmapped::kNo,
                                                      isProtected);
    };

    auto imageProxyID = [&](const sk_sp<SkImage>& img) {
        return sk_gpu_test::GetTextureImageProxy(img.get(), dContext)->uniqueID();
    };

    auto surfaceProxyID = [&](const sk_sp<SkSurface>& surf) {
        GrRenderTargetProxy* rtp = skgpu::ganesh::TopDeviceTargetProxy(surf->getCanvas());
        return rtp->uniqueID();
    };

    sk_sp<SkSurface> surf = makeDirectBackendSurface();
    surf->getCanvas()->clear(SkColor4f{1, 0, 0, 1});
    sk_sp<SkImage> img = surf->makeImageSnapshot();
    // Initially they share
    REPORTER_ASSERT(reporter, surfaceProxyID(surf) == imageProxyID(img));
    // Using the image on the direct context shouldn't affect sharing.
    sk_sp<SkSurface> surf2 = makeDirectBackendSurface();
    surf2->getCanvas()->drawImage(img, 0, 0);
    REPORTER_ASSERT(reporter, surfaceProxyID(surf) == imageProxyID(img));
    // Modifying the original surface should trigger using the copy proxy.
    surf->getCanvas()->clear({0, 0, 1, 1});
    REPORTER_ASSERT(reporter, surfaceProxyID(surf) != imageProxyID(img));
    // Image caching on surface should mean another snapshot gives us the same image.
    GrSurfaceProxy::UniqueID imageID = imageProxyID(img);
    img = surf->makeImageSnapshot();
    REPORTER_ASSERT(reporter, imageProxyID(img) != imageID);

    GrSurfaceCharacterization characterization;
    REPORTER_ASSERT(reporter, surf->characterize(&characterization));
    GrDeferredDisplayListRecorder recorder(characterization);

    // Using an image from a direct context on a recording context should trigger using the copy.
    surf = makeDirectBackendSurface();
    img = surf->makeImageSnapshot();
    REPORTER_ASSERT(reporter, surfaceProxyID(surf) == imageProxyID(img));
    recorder.getCanvas()->drawImage(img, 0, 0);
    REPORTER_ASSERT(reporter, surfaceProxyID(surf) != imageProxyID(img));

    // Same as above but if the surface goes out of scope first we keep using the original
    surf = makeDirectBackendSurface();
    img = surf->makeImageSnapshot();
    GrSurfaceProxy::UniqueID surfID = surfaceProxyID(surf);
    REPORTER_ASSERT(reporter, surfaceProxyID(surf) == imageProxyID(img));
    surf.reset();
    recorder.getCanvas()->drawImage(img, 0, 0);
    REPORTER_ASSERT(reporter, surfID == imageProxyID(img));
}

// Make sure GrCopyRenderTasks's skip actually skips the copy.
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(SkipCopyTaskTest,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNever) {
    GrDirectContext* dContext = ctxInfo.directContext();

    GrImageInfo info(GrColorType::kRGBA_8888,
                     kPremul_SkAlphaType,
                     /*color space*/ nullptr,
                     10, 10);

    auto dstSC = CreateSurfaceContext(dContext,
                                      info,
                                      SkBackingFit::kExact,
                                      kBottomLeft_GrSurfaceOrigin,
                                      GrRenderable::kYes);
    dstSC->asFillContext()->clear(SkPMColor4f{1, 0, 0, 1});

    auto srcSC = CreateSurfaceContext(dContext,
                                      info,
                                      SkBackingFit::kExact,
                                      kBottomLeft_GrSurfaceOrigin,
                                      GrRenderable::kYes);
    srcSC->asFillContext()->clear(SkPMColor4f{0, 0, 1, 1});

    sk_sp<GrRenderTask> task =
            dContext->priv().drawingManager()->newCopyRenderTask(dstSC->asSurfaceProxyRef(),
                                                                 SkIRect::MakeWH(10, 10),
                                                                 srcSC->asSurfaceProxyRef(),
                                                                 SkIRect::MakeWH(10, 10),
                                                                 GrSamplerState::Filter::kNearest,
                                                                 kTopLeft_GrSurfaceOrigin);

    if (!task) {
        ERRORF(reporter, "Couldn't make a copy task.");
        return;
    }

    task->makeSkippable();

    SkAutoPixmapStorage pixels;
    pixels.alloc(SkImageInfo::Make({10, 10}, kRGBA_8888_SkColorType, kPremul_SkAlphaType));
    dstSC->readPixels(dContext, pixels, {0, 0});
    float kTol[4] = {};
    std::function<ComparePixmapsErrorReporter> errorReporter(
            [&](int x, int y, const float diffs[4]) {
                ERRORF(reporter, "Expected {1, 0, 0, 1}. diff {%f, %f, %f, %f}",
                       diffs[0], diffs[1], diffs[2], diffs[3]);
            });
    CheckSolidPixels(SkColor4f{1, 0, 0, 1}, pixels, kTol, errorReporter);
}

#if defined(SK_GANESH)

// Make sure OpsTask are skippable
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(SkipOpsTaskTest, reporter, ctxInfo, CtsEnforcement::kNever) {
    GrDirectContext* dContext = ctxInfo.directContext();

    GrImageInfo ii(GrColorType::kRGBA_8888, kPremul_SkAlphaType, /*color space*/ nullptr, 10, 10);

    auto dst = dContext->priv().makeSFC(ii, /*label=*/{}, SkBackingFit::kExact);
    dst->clear(SkPMColor4f{1, 0, 0, 1});
    dContext->flush();

    dst->clear(SkPMColor4f{0, 0, 1, 1});
    sk_sp<GrRenderTask> task = dst->refRenderTask();

    // GrDrawingManager maintains an "active ops task" and doesn't like having it closed behind
    // its back. temp exists just to replace dst's ops task as the active one.
    auto temp = dContext->priv().makeSFC(ii, /*label=*/{}, SkBackingFit::kExact);
    temp->clear(SkPMColor4f{0, 0, 0, 0});

    GrSurfaceProxyView readView = dst->readSurfaceView();
    task->makeSkippable();

    SkAutoPixmapStorage pixels;
    pixels.alloc(SkImageInfo::Make({10, 10}, kRGBA_8888_SkColorType, kPremul_SkAlphaType));
    dst->readPixels(dContext, pixels, {0, 0});
    float kTol[4] = {};
    std::function<ComparePixmapsErrorReporter> errorReporter(
            [&](int x, int y, const float diffs[4]) {
                ERRORF(reporter, "Expected {1, 0, 0, 1}. diff {%f, %f, %f, %f}",
                       diffs[0], diffs[1], diffs[2], diffs[3]);
            });
    CheckSolidPixels(SkColor4f{1, 0, 0, 1}, pixels, kTol, errorReporter);
}
#endif // defined(SK_GANESH)
