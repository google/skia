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

static sk_sp<SkPromiseImageTexture> promise_img_fulfill(void* ctx) {
    return nullptr;
}

static void promise_img_release(void* ctx) {
}

static void promise_img_done(void* ctx) {
}

DEF_GPUTEST(GrDDLImage_MakeSubset_CrossCtx, reporter, options) {
    // Passed in:  |       raster      |   gpu w/ recording  |   gpu w/ direct
    // -----------------------------------------------------------------------
    // w/ context  |       gpu         |        gpu          |       gpu
    // nullptr     |       raster      |        fail         |       fail

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
        bm.setImmutable();
        auto rasterImg = SkImage::MakeFromBitmap(bm);
        REPORTER_ASSERT(reporter, rasterImg->isValid(nullptr));

        // 1. raster + context:
        auto subImg1 = rasterImg->makeSubset(subsetBounds, direct);
        REPORTER_ASSERT(reporter, subImg1->isValid(direct));

        // 2. raster + no context:
        auto subImg2 = rasterImg->makeSubset(subsetBounds, nullptr);
        REPORTER_ASSERT(reporter, subImg2->isValid(nullptr));

        // Recording ctx image:
        auto surf = SkSurface::MakeRenderTarget(direct, SkBudgeted::kNo, ii);
        SkSurfaceCharacterization sc;
        REPORTER_ASSERT(reporter, surf->characterize(&sc));
        SkDeferredDisplayListRecorder recorder(sc);
        auto promiseImage = recorder.makePromiseTexture(
            sc.backendFormat(), ii.width(), ii.height(), GrMipMapped::kNo, kTopLeft_GrSurfaceOrigin,
            ii.colorType(), ii.alphaType(), ii.refColorSpace(), promise_img_fulfill,
            promise_img_release, promise_img_done, nullptr);
        REPORTER_ASSERT(reporter, promiseImage->isValid(direct));

        // 3. recording ctx image + context:
        auto subImg3 = promiseImage->makeSubset(subsetBounds, direct);
        REPORTER_ASSERT(reporter, subImg3->isValid(direct));

        // 4. recording ctx image + nullptr:
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
        REPORTER_ASSERT(reporter, !subImg6);
    }
}
