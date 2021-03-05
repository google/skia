/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkDeferredDisplayListRecorder.h"
#include "include/core/SkImageInfo.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrTypes.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/core/SkCanvasPriv.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrSurfaceDrawContext.h"
#include "src/gpu/GrSurfaceProxy.h"
#include "tests/Test.h"
#include "tests/TestUtils.h"
#include "tools/gpu/BackendSurfaceFactory.h"
#include "tools/gpu/ProxyUtils.h"

DEF_GPUTEST_FOR_ALL_CONTEXTS(WrappedSurfaceCopyOnWrite, reporter, ctxInfo) {
    GrDirectContext* dContext = ctxInfo.directContext();

    auto makeDirectBackendSurface = [&]() {
        auto info = SkImageInfo::Make({10, 10}, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
        return sk_gpu_test::MakeBackendTextureSurface(dContext,
                                                      info,
                                                      kTopLeft_GrSurfaceOrigin,
                                                      /*sample count*/ 1);
    };

    auto imageProxyID = [&](const sk_sp<SkImage>& img) {
        return sk_gpu_test::GetTextureImageProxy(img.get(), dContext)->uniqueID();
    };

    auto surfaceProxyID = [&](const sk_sp<SkSurface>& surf) {
        GrSurfaceDrawContext* sdc = SkCanvasPriv::TopDeviceSurfaceDrawContext(surf->getCanvas());
        return sdc->asSurfaceProxy()->uniqueID();
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

    SkSurfaceCharacterization characterization;
    REPORTER_ASSERT(reporter, surf->characterize(&characterization));
    SkDeferredDisplayListRecorder recorder(characterization);

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
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(SkipCopyTaskTest, reporter, ctxInfo) {
    GrDirectContext* dContext = ctxInfo.directContext();

    auto dst = GrSurfaceDrawContext::Make(dContext,
                                          GrColorType::kRGBA_8888,
                                          /*color space*/ nullptr,
                                          SkBackingFit::kExact,
                                          {10, 10});
    dst->clear(SkPMColor4f{1, 0, 0, 1});

    auto src = GrSurfaceDrawContext::Make(dContext,
                                          GrColorType::kRGBA_8888,
                                          /*color space*/ nullptr,
                                          SkBackingFit::kExact,
                                          {10, 10});
    src->clear(SkPMColor4f{0, 0, 1, 1});

    sk_sp<GrRenderTask> task =
            dContext->priv().drawingManager()->newCopyRenderTask(src->asSurfaceProxyRef(),
                                                                 SkIRect::MakeWH(10, 10),
                                                                 dst->asSurfaceProxyRef(),
                                                                 {0, 0},
                                                                 kTopLeft_GrSurfaceOrigin);

    if (!task) {
        ERRORF(reporter, "Couldn't make a copy task.");
        return;
    }

    task->canSkip();

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

// Make sure GrOpsTask are skippable
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(SkipOpsTaskTest, reporter, ctxInfo) {
    GrDirectContext* dContext = ctxInfo.directContext();

    auto dst = GrSurfaceDrawContext::Make(dContext,
                                          GrColorType::kRGBA_8888,
                                          /*color space*/ nullptr,
                                          SkBackingFit::kExact,
                                          {10, 10});
    dst->clear(SkPMColor4f{1, 0, 0, 1});
    dContext->flush();

    dst->clear(SkPMColor4f{0, 0, 1, 1});
    sk_sp<GrRenderTask> task = sk_ref_sp(dst->getOpsTask());

    // GrDrawingManager maintains an "active ops task" and doesn't like having it closed behind
    // its back. temp exists just to replace dst's ops task as the active one.
    auto temp = GrSurfaceDrawContext::Make(dContext,
                                           GrColorType::kRGBA_8888,
                                           /*color space*/ nullptr,
                                           SkBackingFit::kExact,
                                           {10, 10});
    temp->clear(SkPMColor4f{0, 0, 0, 0});

    GrSurfaceProxyView readView = dst->readSurfaceView();
    task->canSkip();

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
