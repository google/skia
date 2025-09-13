/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCPURecorder.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/ganesh/GrBackendSurface.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/GrTypes.h"
#include "include/gpu/ganesh/SkImageGanesh.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/private/chromium/GrSurfaceCharacterization.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"
#include "tools/gpu/ContextType.h"

class GrRecordingContext;
struct GrContextOptions;

DEF_GANESH_TEST(GrDDLImage_MakeSubset, reporter, options, CtsEnforcement::kApiLevel_T) {
    using namespace skgpu;

    auto cpuRecorder = skcpu::Recorder::TODO();
    sk_gpu_test::GrContextFactory factory(options);
    for (int ct = 0; ct < skgpu::kContextTypeCount; ++ct) {
        auto contextType = static_cast<skgpu::ContextType>(ct);
        auto dContext = factory.get(contextType);
        if (!dContext) {
            continue;
        }

        Protected isProtected = Protected(dContext->priv().caps()->supportsProtectedContent());

        SkIRect subsetBounds = SkIRect::MakeLTRB(4,4,8,8);
        SkImageInfo ii = SkImageInfo::Make(16, 16, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

        // Raster image:
        SkBitmap bm;
        bm.setInfo(ii);
        bm.allocPixels();
        bm.eraseColor(SK_ColorBLACK);
        bm.setImmutable();
        auto rasterImg = bm.asImage();
        REPORTER_ASSERT(reporter, rasterImg->isValid(cpuRecorder));

        // raster + context:
        auto recorder = dContext->asRecorder();
        auto subImg1 = rasterImg->makeSubset(recorder, subsetBounds, {});
        REPORTER_ASSERT(reporter, subImg1->isValid(recorder));

        // raster + no context:
        auto subImg2 = rasterImg->makeSubset(cpuRecorder, subsetBounds, {});
        REPORTER_ASSERT(reporter, subImg2->isValid(cpuRecorder));

        // Texture image:
        auto surf = SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kNo, ii);
        GrSurfaceCharacterization sc;
        REPORTER_ASSERT(reporter, surf->characterize(&sc));
        GrBackendTexture tex = dContext->createBackendTexture(ii.width(),
                                                              ii.height(),
                                                              ii.colorType(),
                                                              skgpu::Mipmapped(sc.isMipMapped()),
                                                              GrRenderable::kYes,
                                                              isProtected);
        auto gpuImage = SkImages::BorrowTextureFrom(dContext,
                                                    tex,
                                                    kTopLeft_GrSurfaceOrigin,
                                                    ii.colorType(),
                                                    ii.alphaType(),
                                                    ii.refColorSpace());
        REPORTER_ASSERT(reporter, gpuImage->isValid(recorder));

        // gpu image + context:
        auto subImg5 = gpuImage->makeSubset(recorder, subsetBounds, {});
        REPORTER_ASSERT(reporter, subImg5->isValid(recorder));

        // gpu image + nullptr:
        REPORTER_ASSERT(reporter, !gpuImage->makeSubset(nullptr, subsetBounds, {}));

        dContext->flush();
        dContext->submit(GrSyncCpu::kYes);
        dContext->deleteBackendTexture(tex);
    }
}
