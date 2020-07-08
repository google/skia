/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkDeferredDisplayListRecorder.h"
#include "include/core/SkImage.h"
#include "include/core/SkPromiseImageTexture.h"
#include "include/core/SkSurface.h"
#include "tests/Test.h"

struct PromiseImageContext {
    GrDirectContext*             fDirect;
    sk_sp<SkPromiseImageTexture> fPromiseImageTexture;
};

static sk_sp<SkPromiseImageTexture> promise_img_fulfill(void* ctx) {
    auto prom = static_cast<PromiseImageContext*>(ctx);
    GrBackendTexture tex = prom->fDirect->createBackendTexture(16, 16, kRGBA_8888_SkColorType,
                                                               GrMipMapped::kNo,
                                                               GrRenderable::kYes);
    SkASSERT(tex.isValid());
    prom->fPromiseImageTexture = SkPromiseImageTexture::Make(tex);
    return prom->fPromiseImageTexture;
}

static void promise_img_release(void* ctx) {
    auto prom = static_cast<PromiseImageContext*>(ctx);
    if (prom->fPromiseImageTexture) {
        prom->fDirect->deleteBackendTexture(prom->fPromiseImageTexture->backendTexture());
        prom->fPromiseImageTexture.reset();
    }
}

static void promise_img_done(void* ctx) {
    auto prom = static_cast<PromiseImageContext*>(ctx);
    delete prom;
}

DEF_GPUTEST(GrDDLImage_MakeSubset, reporter, options) {
    // Test out each of the cases in the table in the docs for SkImage::makeSubset.

    sk_gpu_test::GrContextFactory factory(options);
    for (int ct = 0; ct < sk_gpu_test::GrContextFactory::kContextTypeCnt; ++ct) {
        auto contextType = static_cast<sk_gpu_test::GrContextFactory::ContextType>(ct);
        auto direct = factory.get(contextType);
        if (!direct) {
            continue;
        }
        SkIRect subsetBounds = SkIRect::MakeLTRB(4,4,8,8);
        SkImageInfo ii = SkImageInfo::Make(16, 16, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

        // Raster image:
        SkBitmap bm;
        bm.setInfo(ii);
        bm.allocPixels();
        bm.eraseColor(SK_ColorBLACK);
        bm.setImmutable();
        auto rasterImg = SkImage::MakeFromBitmap(bm);
        REPORTER_ASSERT(reporter, rasterImg->isValid(nullptr));

        // 1. raster + context:
        auto subImg1 = rasterImg->makeSubset(subsetBounds, direct);
        REPORTER_ASSERT(reporter, subImg1->isValid(direct));

        // 2. raster + no context:
        auto subImg2 = rasterImg->makeSubset(subsetBounds, nullptr);
        REPORTER_ASSERT(reporter, subImg2->isValid(nullptr));

        // Promise image:
        auto surf = SkSurface::MakeRenderTarget(direct, SkBudgeted::kNo, ii);
        SkSurfaceCharacterization sc;
        REPORTER_ASSERT(reporter, surf->characterize(&sc));
        SkDeferredDisplayListRecorder recorder(sc);
        auto promCtx = new PromiseImageContext{direct, nullptr};
        auto promiseImage = recorder.makePromiseTexture(
            sc.backendFormat(), ii.width(), ii.height(), GrMipMapped::kNo, kTopLeft_GrSurfaceOrigin,
            ii.colorType(), ii.alphaType(), ii.refColorSpace(), promise_img_fulfill,
            promise_img_release, promise_img_done, promCtx);
        REPORTER_ASSERT(reporter, promiseImage->isValid(direct));

        // 3. promise image + context:
        auto subImg3 = promiseImage->makeSubset(subsetBounds, direct);
        REPORTER_ASSERT(reporter, subImg3->isValid(direct));

        // 4. promise image + nullptr:
        auto subImg4 = promiseImage->makeSubset(subsetBounds, nullptr);
        REPORTER_ASSERT(reporter, !subImg4);

        // Texture image:
        GrBackendTexture tex = direct->createBackendTexture(sc);
        auto gpuImage = SkImage::MakeFromTexture(direct, tex, kTopLeft_GrSurfaceOrigin,
                                                 ii.colorType(), ii.alphaType(),
                                                 ii.refColorSpace());
        REPORTER_ASSERT(reporter, gpuImage->isValid(direct));

        // 5. gpu image + context:
        auto subImg5 = gpuImage->makeSubset(subsetBounds, direct);
        REPORTER_ASSERT(reporter, subImg5->isValid(direct));

        // 6. gpu image + nullptr:
        auto subImg6 = gpuImage->makeSubset(subsetBounds, nullptr);
        REPORTER_ASSERT(reporter, subImg6->isValid(direct));

        direct->flush();
        direct->submit(true);
        direct->deleteBackendTexture(tex);
    }
}
