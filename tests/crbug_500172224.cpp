/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkGradient.h"
#include "include/gpu/ganesh/SkImageGanesh.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "src/gpu/ganesh/GrResourceProvider.h"
#include "tests/Test.h"

namespace {

sk_sp<SkImage> make_gpu_src_img(GrDirectContext* dContext, float srcSize) {
    SkImageInfo ii = SkImageInfo::Make(srcSize, srcSize,
                                       kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    SkBitmap srcBM;
    srcBM.allocPixels(ii);

    SkPaint p;
    const SkPoint pts[] = { { 0, 0 }, { srcSize, srcSize } };
    const SkColor4f colors[] = { SkColors::kWhite, SkColors::kBlue };
    p.setShader(SkShaders::LinearGradient(pts, {{colors, {}, SkTileMode::kClamp}, {}}));

    SkCanvas c(srcBM);
    c.drawRect(SkRect::MakeWH(srcSize, srcSize), p);

    sk_sp<SkImage> rasterImage = SkImages::RasterFromBitmap(srcBM);
    return SkImages::TextureFromImage(dContext, rasterImage);
}

// This test will trigger an assert if the max quads/TextureOp limit overflows
void run_test(GrDirectContext* dContext, uint32_t numXSteps, uint32_t numYSteps, bool withAA) {
    constexpr uint32_t kRectSize = 40;
    constexpr float kHalfRectSize = kRectSize / 2.0f;

    sk_sp<SkImage> srcImg = make_gpu_src_img(dContext, kRectSize);

    SkImageInfo ii = SkImageInfo::Make(numXSteps*kRectSize, numYSteps*kRectSize,
                                       kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    auto surface(SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kNo, ii));

    SkCanvas* canvas = surface->getCanvas();
    canvas->clear(SK_ColorBLACK);

    SkCanvas::ImageSetEntry* entries = new SkCanvas::ImageSetEntry[numXSteps*numYSteps];
    SkMatrix* preViewMatrices = new SkMatrix[numXSteps*numYSteps];

    SkMatrix perspMatrix;
    perspMatrix.setPerspX(0.1f);
    perspMatrix.setPerspY(0.1f);

    for (uint32_t y = 0; y < numYSteps; ++y) {
        for (uint32_t x = 0; x < numXSteps; x++) {
            uint32_t entry = y*numXSteps+x;

            // These matrices are arranged so every quad will be split due to
            // perspective
            preViewMatrices[entry].setTranslate(x*kRectSize+kHalfRectSize,
                                                y*kRectSize+kHalfRectSize);

            preViewMatrices[entry].preConcat(perspMatrix);

            SkRect destRect = SkRect::MakeXYWH(-kHalfRectSize, -kHalfRectSize,
                                               kRectSize, kRectSize);

            entries[entry].fImage = srcImg;
            entries[entry].fSrcRect = SkRect::MakeLTRB(0, 0, kRectSize, kRectSize);
            entries[entry].fDstRect = destRect;
            entries[entry].fMatrixIndex = entry;
            entries[entry].fAlpha = 1.f;
            entries[entry].fAAFlags = withAA ? SkCanvas::kAll_QuadAAFlags
                                             : SkCanvas::kNone_QuadAAFlags;
            entries[entry].fHasClip = false;
        }
    }

    canvas->experimental_DrawEdgeAAImageSet(
        entries,
        numXSteps*numYSteps,
        /* dstClips= */ nullptr,
        preViewMatrices,
        SkSamplingOptions());

    dContext->flushAndSubmit(surface.get(), GrSyncCpu::kYes);

    delete [] preViewMatrices;
    delete [] entries;
}

} // anonymous namespace

// This checks that the perspective quad splitting doesn't overflow for AA draws
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(crbug_500172224_aa_double,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNever) {
    constexpr uint32_t kNumXSteps = 32;
    constexpr uint32_t kNumYSteps = 32;
    SkASSERT(kNumXSteps*kNumYSteps == 2*GrResourceProvider::MaxNumAAQuads());
    run_test(ctxInfo.directContext(), kNumXSteps, kNumYSteps, /* withAA= */ true);
}

// This checks that the perspective quad splitting doesn't overflow for non-AA draws
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(crbug_500172224_non_aa_double,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNever) {
    constexpr uint32_t kNumXSteps = 128;
    constexpr uint32_t kNumYSteps = 64;
    SkASSERT(kNumXSteps*kNumYSteps == 2*GrResourceProvider::MaxNumNonAAQuads());
    run_test(ctxInfo.directContext(), kNumXSteps, kNumYSteps, /* withAA= */ false);
}

// This tests the AA perspective fast-path blocking in TextureOp::AddTextureSetOps.
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(crbug_500172224_aa,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNever) {
    constexpr uint32_t kNumXSteps = 32;
    constexpr uint32_t kNumYSteps = 16;
    SkASSERT(kNumXSteps*kNumYSteps == GrResourceProvider::MaxNumAAQuads());
    run_test(ctxInfo.directContext(), kNumXSteps, kNumYSteps, /* withAA= */ true);
}

