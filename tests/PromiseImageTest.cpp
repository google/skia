/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkShader.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/ganesh/GrBackendSurface.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/GrTypes.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/private/base/SkTArray.h"
#include "include/private/chromium/GrDeferredDisplayListRecorder.h"
#include "include/private/chromium/GrPromiseImageTexture.h"
#include "include/private/chromium/SkImageChromium.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrResourceProvider.h"
#include "src/gpu/ganesh/GrTexture.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"
#include "tools/gpu/ContextType.h"
#include "tools/gpu/FenceSync.h"
#include "tools/gpu/ManagedBackendTexture.h"

#include <cstddef>
#include <functional>
#include <utility>

using namespace skia_private;

struct GrContextOptions;

using namespace sk_gpu_test;

struct PromiseTextureChecker {
    // shared indicates whether the backend texture is used to fulfill more than one promise
    // image.
    explicit PromiseTextureChecker(const GrBackendTexture& tex,
                                   skiatest::Reporter* reporter,
                                   bool shared)
            : fTexture(GrPromiseImageTexture::Make(tex)), fReporter(reporter), fShared(shared) {}
    sk_sp<GrPromiseImageTexture> fTexture;
    skiatest::Reporter* fReporter;
    bool fShared;
    int fFulfillCount = 0;
    int fReleaseCount = 0;

    static sk_sp<GrPromiseImageTexture> Fulfill(void* self) {
        auto checker = static_cast<PromiseTextureChecker*>(self);
        checker->fFulfillCount++;
        return checker->fTexture;
    }
    static void Release(void* self) { static_cast<PromiseTextureChecker*>(self)->fReleaseCount++; }
};

enum class ReleaseBalanceExpectation {
    kBalanced,
    kAllUnbalanced,
    kUnknown,
    kUnbalancedByOne,
    kBalancedOrOffByOne,
};

static void check_fulfill_and_release_cnts(skiatest::Reporter* reporter,
                                           const PromiseTextureChecker& promiseChecker,
                                           int expectedFulfillCnt,
                                           ReleaseBalanceExpectation releaseBalanceExpecation) {
    REPORTER_ASSERT(reporter, promiseChecker.fFulfillCount == expectedFulfillCnt);
    if (!expectedFulfillCnt) {
        // Release and Done should only ever be called after Fulfill.
        REPORTER_ASSERT(reporter, !promiseChecker.fReleaseCount);
        return;
    }
    int releaseDiff = promiseChecker.fFulfillCount - promiseChecker.fReleaseCount;
    switch (releaseBalanceExpecation) {
        case ReleaseBalanceExpectation::kBalanced:
            REPORTER_ASSERT(reporter, !releaseDiff);
            break;
        case ReleaseBalanceExpectation::kAllUnbalanced:
            REPORTER_ASSERT(reporter, releaseDiff == promiseChecker.fFulfillCount);
            break;
        case ReleaseBalanceExpectation::kUnknown:
            REPORTER_ASSERT(reporter,
                            releaseDiff >= 0 && releaseDiff <= promiseChecker.fFulfillCount);
            break;
        case ReleaseBalanceExpectation::kUnbalancedByOne:
            REPORTER_ASSERT(reporter, releaseDiff == 1);
            break;
        case ReleaseBalanceExpectation::kBalancedOrOffByOne:
            REPORTER_ASSERT(reporter, releaseDiff == 0 || releaseDiff == 1);
            break;
    }
}

static void check_unfulfilled(const PromiseTextureChecker& promiseChecker,
                              skiatest::Reporter* reporter) {
    check_fulfill_and_release_cnts(reporter, promiseChecker, 0,
                                   ReleaseBalanceExpectation::kBalanced);
}

static void check_only_fulfilled(skiatest::Reporter* reporter,
                                 const PromiseTextureChecker& promiseChecker,
                                 int expectedFulfillCnt = 1) {
    check_fulfill_and_release_cnts(reporter, promiseChecker, expectedFulfillCnt,
                                   ReleaseBalanceExpectation::kAllUnbalanced);
}

static void check_all_flushed_but_not_synced(skiatest::Reporter* reporter,
                                             const PromiseTextureChecker& promiseChecker,
                                             GrBackendApi api,
                                             int expectedFulfillCnt = 1) {
    ReleaseBalanceExpectation releaseBalanceExpectation = ReleaseBalanceExpectation::kBalanced;
    // On Vulkan and D3D Done isn't guaranteed to be called until a sync has occurred.
    if (api == GrBackendApi::kVulkan || api == GrBackendApi::kDirect3D) {
        releaseBalanceExpectation = expectedFulfillCnt == 1
                                            ? ReleaseBalanceExpectation::kBalancedOrOffByOne
                                            : ReleaseBalanceExpectation::kUnknown;
    }
    check_fulfill_and_release_cnts(reporter, promiseChecker, expectedFulfillCnt,
                                   releaseBalanceExpectation);
}

static void check_all_done(skiatest::Reporter* reporter,
                           const PromiseTextureChecker& promiseChecker,
                           int expectedFulfillCnt = 1) {
    check_fulfill_and_release_cnts(reporter, promiseChecker, expectedFulfillCnt,
                                   ReleaseBalanceExpectation::kBalanced);
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(PromiseImageTest,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNever) {
    using namespace skgpu;
    const int kWidth = 10;
    const int kHeight = 10;

    auto ctx = ctxInfo.directContext();

    Protected isProtected = Protected(ctx->priv().caps()->supportsProtectedContent());

    GrBackendTexture backendTex = ctx->createBackendTexture(kWidth,
                                                            kHeight,
                                                            kRGBA_8888_SkColorType,
                                                            SkColors::kTransparent,
                                                            skgpu::Mipmapped::kNo,
                                                            GrRenderable::kYes,
                                                            isProtected);
    REPORTER_ASSERT(reporter, backendTex.isValid());

    GrBackendFormat backendFormat = backendTex.getBackendFormat();
    REPORTER_ASSERT(reporter, backendFormat.isValid());

    PromiseTextureChecker promiseChecker(backendTex, reporter, false);
    GrSurfaceOrigin texOrigin = kTopLeft_GrSurfaceOrigin;
    sk_sp<SkImage> refImg(SkImages::PromiseTextureFrom(ctx->threadSafeProxy(),
                                                       backendFormat,
                                                       {kWidth, kHeight},
                                                       skgpu::Mipmapped::kNo,
                                                       texOrigin,
                                                       kRGBA_8888_SkColorType,
                                                       kPremul_SkAlphaType,
                                                       nullptr,
                                                       PromiseTextureChecker::Fulfill,
                                                       PromiseTextureChecker::Release,
                                                       &promiseChecker));

    SkImageInfo info = SkImageInfo::MakeN32Premul(kWidth, kHeight);
    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(ctx, skgpu::Budgeted::kNo, info);
    SkCanvas* canvas = surface->getCanvas();

    canvas->drawImage(refImg, 0, 0);
    check_unfulfilled(promiseChecker, reporter);

    ctx->flushAndSubmit(surface.get(), GrSyncCpu::kNo);
    // We still own the image so we should not have called Release or Done.
    check_only_fulfilled(reporter, promiseChecker);

    ctx->submit(GrSyncCpu::kYes);
    check_only_fulfilled(reporter, promiseChecker);

    canvas->drawImage(refImg, 0, 0);
    canvas->drawImage(refImg, 0, 0);

    ctx->flushAndSubmit(surface.get(), GrSyncCpu::kYes);

    // Image should still be fulfilled from the first time we drew/flushed it.
    check_only_fulfilled(reporter, promiseChecker);

    canvas->drawImage(refImg, 0, 0);
    ctx->flushAndSubmit(surface.get(), GrSyncCpu::kNo);
    check_only_fulfilled(reporter, promiseChecker);

    canvas->drawImage(refImg, 0, 0);
    refImg.reset();
    // We no longer own the image but the last draw is still unflushed.
    check_only_fulfilled(reporter, promiseChecker);

    ctx->flushAndSubmit(surface.get(), GrSyncCpu::kNo);
    // Flushing should have called Release. Depending on the backend and timing it may have called
    // done.
    check_all_flushed_but_not_synced(reporter, promiseChecker, ctx->backend());
    ctx->submit(GrSyncCpu::kYes);
    // Now Done should definitely have been called.
    check_all_done(reporter, promiseChecker);

    ctx->deleteBackendTexture(backendTex);
}

DEF_GANESH_TEST(PromiseImageTextureShutdown, reporter, ctxInfo, CtsEnforcement::kNever) {
    const int kWidth = 10;
    const int kHeight = 10;

    // Different ways of killing contexts.
    using DeathFn = std::function<void(sk_gpu_test::GrContextFactory*, GrDirectContext*)>;
    DeathFn destroy = [](sk_gpu_test::GrContextFactory* factory, GrDirectContext*) {
        factory->destroyContexts();
    };
    DeathFn abandon = [](sk_gpu_test::GrContextFactory* factory, GrDirectContext* dContext) {
        dContext->abandonContext();
    };
    DeathFn releaseResourcesAndAbandon = [](sk_gpu_test::GrContextFactory* factory,
                                            GrDirectContext* dContext) {
        dContext->releaseResourcesAndAbandonContext();
    };

    for (int type = 0; type < skgpu::kContextTypeCount; ++type) {
        auto contextType = static_cast<skgpu::ContextType>(type);
        // These tests are difficult to get working with Vulkan. See skbug.com/40039997
        // and skbug.com/40039545
        // And Direct3D, for similar reasons.
        GrBackendApi api = skgpu::ganesh::ContextTypeBackend(contextType);
        if (api == GrBackendApi::kUnsupported || api == GrBackendApi::kVulkan ||
            api == GrBackendApi::kDirect3D) {
            continue;
        }
        DeathFn contextKillers[] = {destroy, abandon, releaseResourcesAndAbandon};
        for (const DeathFn& contextDeath : contextKillers) {
            sk_gpu_test::GrContextFactory factory;
            auto ctx = factory.get(contextType);
            if (!ctx) {
                continue;
            }

            auto mbet = sk_gpu_test::ManagedBackendTexture::MakeWithoutData(ctx,
                                                                            kWidth,
                                                                            kHeight,
                                                                            kAlpha_8_SkColorType,
                                                                            skgpu::Mipmapped::kNo,
                                                                            GrRenderable::kNo);
            if (!mbet) {
                ERRORF(reporter, "Could not create texture alpha texture.");
                continue;
            }

            SkImageInfo info = SkImageInfo::Make(kWidth, kHeight, kRGBA_8888_SkColorType,
                                                 kPremul_SkAlphaType);
            sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(ctx, skgpu::Budgeted::kNo, info);
            SkCanvas* canvas = surface->getCanvas();

            PromiseTextureChecker promiseChecker(mbet->texture(), reporter, false);
            sk_sp<SkImage> image(SkImages::PromiseTextureFrom(ctx->threadSafeProxy(),
                                                              mbet->texture().getBackendFormat(),
                                                              {kWidth, kHeight},
                                                              skgpu::Mipmapped::kNo,
                                                              kTopLeft_GrSurfaceOrigin,
                                                              kAlpha_8_SkColorType,
                                                              kPremul_SkAlphaType,
                                                              /*color space*/ nullptr,
                                                              PromiseTextureChecker::Fulfill,
                                                              PromiseTextureChecker::Release,
                                                              &promiseChecker));
            REPORTER_ASSERT(reporter, image);

            canvas->drawImage(image, 0, 0);
            image.reset();
            // If the surface still holds a ref to the context then the factory will not be able
            // to destroy the context (and instead will release-all-and-abandon).
            surface.reset();

            ctx->flushAndSubmit();
            contextDeath(&factory, ctx);

            check_all_done(reporter, promiseChecker);
        }
    }
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(PromiseImageTextureFullCache,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNever) {
    using namespace skgpu;

    const int kWidth = 10;
    const int kHeight = 10;

    auto dContext = ctxInfo.directContext();

    Protected isProtected = Protected(dContext->priv().caps()->supportsProtectedContent());

    GrBackendTexture backendTex = dContext->createBackendTexture(kWidth,
                                                                 kHeight,
                                                                 kAlpha_8_SkColorType,
                                                                 SkColors::kTransparent,
                                                                 skgpu::Mipmapped::kNo,
                                                                 GrRenderable::kNo,
                                                                 isProtected);
    REPORTER_ASSERT(reporter, backendTex.isValid());

    SkImageInfo info =
            SkImageInfo::Make(kWidth, kHeight, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kNo, info);
    SkCanvas* canvas = surface->getCanvas();

    PromiseTextureChecker promiseChecker(backendTex, reporter, false);
    sk_sp<SkImage> image(SkImages::PromiseTextureFrom(dContext->threadSafeProxy(),
                                                      backendTex.getBackendFormat(),
                                                      {kWidth, kHeight},
                                                      skgpu::Mipmapped::kNo,
                                                      kTopLeft_GrSurfaceOrigin,
                                                      kAlpha_8_SkColorType,
                                                      kPremul_SkAlphaType,
                                                      nullptr,
                                                      PromiseTextureChecker::Fulfill,
                                                      PromiseTextureChecker::Release,
                                                      &promiseChecker));
    REPORTER_ASSERT(reporter, image);

    // Make the cache full. This tests that we don't preemptively purge cached textures for
    // fulfillment due to cache pressure.
    static constexpr int kMaxBytes = 1;
    dContext->setResourceCacheLimit(kMaxBytes);
    TArray<sk_sp<GrTexture>> textures;
    for (int i = 0; i < 5; ++i) {
        auto format = dContext->priv().caps()->getDefaultBackendFormat(GrColorType::kRGBA_8888,
                                                                       GrRenderable::kNo);
        textures.emplace_back(dContext->priv().resourceProvider()->createTexture(
                {100, 100},
                format,
                GrTextureType::k2D,
                GrRenderable::kNo,
                1,
                skgpu::Mipmapped::kNo,
                skgpu::Budgeted::kYes,
                isProtected,
                /*label=*/"PromiseImageTextureFullCacheTest"));
        REPORTER_ASSERT(reporter, textures[i]);
    }

    size_t bytesUsed;

    dContext->getResourceCacheUsage(nullptr, &bytesUsed);
    REPORTER_ASSERT(reporter, bytesUsed > kMaxBytes);

    // Relying on the asserts in the promiseImageChecker to ensure that fulfills and releases are
    // properly ordered.
    canvas->drawImage(image, 0, 0);
    dContext->flushAndSubmit(surface.get(), GrSyncCpu::kNo);
    canvas->drawImage(image, 1, 0);
    dContext->flushAndSubmit(surface.get(), GrSyncCpu::kNo);
    canvas->drawImage(image, 2, 0);
    dContext->flushAndSubmit(surface.get(), GrSyncCpu::kNo);
    canvas->drawImage(image, 3, 0);
    dContext->flushAndSubmit(surface.get(), GrSyncCpu::kNo);
    canvas->drawImage(image, 4, 0);
    dContext->flushAndSubmit(surface.get(), GrSyncCpu::kNo);
    canvas->drawImage(image, 5, 0);
    dContext->flushAndSubmit(surface.get(), GrSyncCpu::kNo);
    // Must call these to ensure that all callbacks are performed before the checker is destroyed.
    image.reset();
    dContext->flushAndSubmit(GrSyncCpu::kYes);

    dContext->deleteBackendTexture(backendTex);
}

// Test case where promise image fulfill returns nullptr.
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(PromiseImageNullFulfill,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNever) {
    const int kWidth = 10;
    const int kHeight = 10;

    auto dContext = ctxInfo.directContext();

    GrBackendFormat backendFormat =
            dContext->defaultBackendFormat(kRGBA_8888_SkColorType, GrRenderable::kYes);
    if (!backendFormat.isValid()) {
        ERRORF(reporter, "No valid default kRGBA_8888 texture format.");
        return;
    }

    struct Counts {
        int fFulfillCount = 0;
        int fReleaseCount = 0;
    } counts;
    auto fulfill = [](SkImages::PromiseImageTextureContext ctx) {
        ++static_cast<Counts*>(ctx)->fFulfillCount;
        return sk_sp<GrPromiseImageTexture>();
    };
    auto release = [](SkImages::PromiseImageTextureContext ctx) {
        ++static_cast<Counts*>(ctx)->fReleaseCount;
    };
    GrSurfaceOrigin texOrigin = kTopLeft_GrSurfaceOrigin;
    sk_sp<SkImage> refImg(SkImages::PromiseTextureFrom(dContext->threadSafeProxy(),
                                                       backendFormat,
                                                       {kWidth, kHeight},
                                                       skgpu::Mipmapped::kNo,
                                                       texOrigin,
                                                       kRGBA_8888_SkColorType,
                                                       kPremul_SkAlphaType,
                                                       nullptr,
                                                       fulfill,
                                                       release,
                                                       &counts));

    SkImageInfo info = SkImageInfo::MakeN32Premul(kWidth, kHeight);
    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kNo, info);
    SkCanvas* canvas = surface->getCanvas();
    // Draw the image a few different ways.
    canvas->drawImage(refImg, 0, 0);
    SkPaint paint;
    paint.setColorFilter(SkColorFilters::LinearToSRGBGamma());
    canvas->drawImage(refImg, 0, 0, SkSamplingOptions(), &paint);
    auto shader = refImg->makeShader(SkSamplingOptions());
    REPORTER_ASSERT(reporter, shader);
    paint.setShader(std::move(shader));
    canvas->drawRect(SkRect::MakeWH(1,1), paint);
    paint.setShader(nullptr);
    refImg.reset();
    dContext->flushAndSubmit(surface.get(), GrSyncCpu::kNo);
    // We should only call each callback once and we should have made all the calls by this point.
    REPORTER_ASSERT(reporter, counts.fFulfillCount == 1);
    REPORTER_ASSERT(reporter, counts.fReleaseCount == 1);
}
