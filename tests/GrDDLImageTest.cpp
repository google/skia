/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkImage.h"
#include "include/core/SkSurface.h"
#include "include/core/SkSurfaceCharacterization.h"
#include "tests/Test.h"

DEF_GPUTEST(GrDDLImage_MakeSubset, reporter, options) {
    sk_gpu_test::GrContextFactory factory(options);
    for (int ct = 0; ct < sk_gpu_test::GrContextFactory::kContextTypeCnt; ++ct) {
        auto contextType = static_cast<sk_gpu_test::GrContextFactory::ContextType>(ct);
        auto dContext = factory.get(contextType);
        if (!dContext) {
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
        auto rasterImg = bm.asImage();
        REPORTER_ASSERT(reporter, rasterImg->isValid(static_cast<GrRecordingContext*>(nullptr)));

        // raster + context:
        auto subImg1 = rasterImg->makeSubset(subsetBounds, dContext);
        REPORTER_ASSERT(reporter, subImg1->isValid(dContext));

        // raster + no context:
        auto subImg2 = rasterImg->makeSubset(subsetBounds, nullptr);
        REPORTER_ASSERT(reporter, subImg2->isValid(static_cast<GrRecordingContext*>(nullptr)));

        // Texture image:
        auto surf = SkSurface::MakeRenderTarget(dContext, SkBudgeted::kNo, ii);
        SkSurfaceCharacterization sc;
        REPORTER_ASSERT(reporter, surf->characterize(&sc));
        GrBackendTexture tex =
                dContext->createBackendTexture(ii.width(), ii.height(), ii.colorType(),
                                               GrMipmapped(sc.isMipMapped()), GrRenderable::kYes);
        auto gpuImage = SkImage::MakeFromTexture(dContext, tex, kTopLeft_GrSurfaceOrigin,
                                                 ii.colorType(), ii.alphaType(),
                                                 ii.refColorSpace());
        REPORTER_ASSERT(reporter, gpuImage->isValid(dContext));

        // gpu image + context:
        auto subImg5 = gpuImage->makeSubset(subsetBounds, dContext);
        REPORTER_ASSERT(reporter, subImg5->isValid(dContext));

        // gpu image + nullptr:
        REPORTER_ASSERT(reporter, !gpuImage->makeSubset(subsetBounds, nullptr));

        dContext->flush();
        dContext->submit(true);
        dContext->deleteBackendTexture(tex);
    }
}
