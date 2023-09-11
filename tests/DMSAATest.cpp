/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkSurface.h"
#include "include/core/SkSurfaceProps.h"
#include "include/core/SkTypes.h"
#include "include/core/SkVertices.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrContextOptions.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrTypes.h"
#include "include/gpu/ganesh/SkImageGanesh.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/private/SkColorData.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/core/SkBlendModePriv.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/ganesh/GrPaint.h"
#include "src/gpu/ganesh/GrPixmap.h"
#include "src/gpu/ganesh/GrXferProcessor.h"
#include "src/gpu/ganesh/SurfaceDrawContext.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"
#include "tools/gpu/ContextType.h"

#include <cstdint>
#include <cstring>
#include <memory>
#include <utility>

namespace {

static SkSurfaceProps kDMSAAProps(SkSurfaceProps::kDynamicMSAA_Flag, kUnknown_SkPixelGeometry);
static SkSurfaceProps kBasicProps(0, kUnknown_SkPixelGeometry);
constexpr static SkPMColor4f kTransYellow = {.5f,.5f,.0f,.5f};
constexpr static SkPMColor4f kTransCyan = {.0f,.5f,.5f,.5f};
constexpr static int kWidth=10, kHeight=10;

}

static void draw_paint_with_aa(skgpu::ganesh::SurfaceDrawContext* sdc,
                               const SkPMColor4f& color,
                               SkBlendMode blendMode) {
    GrPaint paint;
    paint.setColor4f(color);
    paint.setXPFactory(GrXPFactory::FromBlendMode(blendMode));
    sdc->drawRect(nullptr, std::move(paint), GrAA::kYes, SkMatrix::I(),
                  SkRect::MakeIWH(kWidth, kHeight), nullptr);
}

static void draw_paint_with_dmsaa(skgpu::ganesh::SurfaceDrawContext* sdc,
                                  const SkPMColor4f& color,
                                  SkBlendMode blendMode) {
    // drawVertices should always trigger dmsaa, but draw something non-rectangular just to be 100%
    // certain.
    static const SkPoint kVertices[3] = {{-.5f,-.5f}, {kWidth * 2.1f, 0}, {0, kHeight * 2.1f}};
    SkVertices::Builder builder(SkVertices::kTriangles_VertexMode, 3, 0, 0);
    memcpy(builder.positions(), kVertices, sizeof(kVertices));
    auto vertices = builder.detach();

    GrPaint paint;
    paint.setColor4f(color);
    paint.setXPFactory(GrXPFactory::FromBlendMode(blendMode));
    sdc->drawVertices(nullptr, std::move(paint), SkMatrix::I(), vertices);
}

static bool fuzzy_equals(const float a[4], const SkPMColor4f& b) {
    constexpr static float kTolerance = 2.5f / 256;
    for (int i = 0; i < 4; ++i) {
        if (!SkScalarNearlyEqual(a[i], b.vec()[i], kTolerance)) {
            return false;
        }
    }
    return true;
}

static void check_sdc_color(skiatest::Reporter* reporter,
                            skgpu::ganesh::SurfaceDrawContext* sdc,
                            GrDirectContext* ctx,
                            const SkPMColor4f& color) {
    auto info = SkImageInfo::Make(kWidth, kHeight, kRGBA_F32_SkColorType, kPremul_SkAlphaType);
    GrPixmap pixmap = GrPixmap::Allocate(info);
    sdc->readPixels(ctx, pixmap, {0, 0});
    auto pix = static_cast<const float*>(pixmap.addr());
    for (int y = 0; y < kHeight; ++y) {
        for (int x = 0; x < kWidth; ++x) {
            if (!fuzzy_equals(pix, color)) {
                ERRORF(reporter, "SDC color mismatch.\n"
                                 "Got      [%0.3f, %0.3f, %0.3f, %0.3f]\n"
                                 "Expected [%0.3f, %0.3f, %0.3f, %0.3f]",
                       pix[0], pix[1], pix[2], pix[3], color.fR, color.fG, color.fB, color.fA);
                return;
            }
            pix += 4;
        }
    }
}

DEF_GANESH_TEST_FOR_CONTEXTS(DMSAA_preserve_contents,
                             &skgpu::IsRenderingContext,
                             reporter,
                             ctxInfo,
                             nullptr,
                             CtsEnforcement::kApiLevel_T) {
    auto dContext = ctxInfo.directContext();
    auto sdc = skgpu::ganesh::SurfaceDrawContext::Make(dContext,
                                                       GrColorType::kRGBA_8888,
                                                       nullptr,
                                                       SkBackingFit::kApprox,
                                                       {kWidth, kHeight},
                                                       kDMSAAProps,
                                                       /*label=*/{});

    // Initialize the texture and dmsaa attachment with transparent.
    draw_paint_with_dmsaa(sdc.get(), SK_PMColor4fTRANSPARENT, SkBlendMode::kSrc);
    check_sdc_color(reporter, sdc.get(), dContext, SK_PMColor4fTRANSPARENT);

    // Clear the main texture to yellow.
    sdc->clear(kTransYellow);

    // Close the opsTask by doing a readback.
    check_sdc_color(reporter, sdc.get(), dContext, kTransYellow);

    // Now the DMSAA attachment is clear and the texture is yellow. Blend cyan into the DMSAA
    // attachment. This will fail if the yellow from the main texture doesn't get copied into the
    // DMSAA attachment before the renderPass.
    draw_paint_with_dmsaa(sdc.get(), kTransCyan, SkBlendMode::kSrcOver);
    SkPMColor4f dstColor = SkBlendMode_Apply(SkBlendMode::kSrcOver, kTransCyan, kTransYellow);

    check_sdc_color(reporter, sdc.get(), dContext, dstColor);
}

static void require_dst_reads(GrContextOptions* options) {
    options->fSuppressAdvancedBlendEquations = true;
    options->fSuppressFramebufferFetch = true;
}

DEF_GANESH_TEST_FOR_CONTEXTS(DMSAA_dst_read,
                             &skgpu::IsRenderingContext,
                             reporter,
                             ctxInfo,
                             require_dst_reads,
                             CtsEnforcement::kApiLevel_T) {
    auto dContext = ctxInfo.directContext();
    auto sdc = skgpu::ganesh::SurfaceDrawContext::Make(dContext,
                                                       GrColorType::kRGBA_8888,
                                                       nullptr,
                                                       SkBackingFit::kApprox,
                                                       {kWidth, kHeight},
                                                       kDMSAAProps,
                                                       /*label=*/{});

    // Initialize the texture and dmsaa attachment with transparent.
    draw_paint_with_dmsaa(sdc.get(), SK_PMColor4fTRANSPARENT, SkBlendMode::kSrc);
    check_sdc_color(reporter, sdc.get(), dContext, SK_PMColor4fTRANSPARENT);

    sdc->clear(SK_PMColor4fWHITE);
    SkPMColor4f dstColor = SK_PMColor4fWHITE;

    draw_paint_with_dmsaa(sdc.get(), kTransYellow, SkBlendMode::kDarken);
    dstColor = SkBlendMode_Apply(SkBlendMode::kDarken, kTransYellow, dstColor);

    draw_paint_with_dmsaa(sdc.get(), kTransCyan, SkBlendMode::kDarken);
    dstColor = SkBlendMode_Apply(SkBlendMode::kDarken, kTransCyan, dstColor);

    check_sdc_color(reporter, sdc.get(), dContext, dstColor);
}

DEF_GANESH_TEST_FOR_CONTEXTS(DMSAA_aa_dst_read_after_dmsaa,
                             &skgpu::IsRenderingContext,
                             reporter,
                             ctxInfo,
                             require_dst_reads,
                             CtsEnforcement::kApiLevel_T) {
    auto dContext = ctxInfo.directContext();
    auto sdc = skgpu::ganesh::SurfaceDrawContext::Make(dContext,
                                                       GrColorType::kRGBA_8888,
                                                       nullptr,
                                                       SkBackingFit::kApprox,
                                                       {kWidth, kHeight},
                                                       kDMSAAProps,
                                                       /*label=*/{});

    // Initialize the texture and dmsaa attachment with transparent.
    draw_paint_with_dmsaa(sdc.get(), SK_PMColor4fTRANSPARENT, SkBlendMode::kSrc);
    check_sdc_color(reporter, sdc.get(), dContext, SK_PMColor4fTRANSPARENT);

    sdc->clear(SK_PMColor4fWHITE);
    SkPMColor4f dstColor = SK_PMColor4fWHITE;

    draw_paint_with_dmsaa(sdc.get(), kTransYellow, SkBlendMode::kDarken);
    dstColor = SkBlendMode_Apply(SkBlendMode::kDarken, kTransYellow, dstColor);

    // Draw with aa after dmsaa. This should break up the render pass and issue a texture barrier.
    draw_paint_with_aa(sdc.get(), kTransCyan, SkBlendMode::kDarken);
    dstColor = SkBlendMode_Apply(SkBlendMode::kDarken, kTransCyan, dstColor);

    check_sdc_color(reporter, sdc.get(), dContext, dstColor);
}

DEF_GANESH_TEST_FOR_CONTEXTS(DMSAA_dst_read_with_existing_barrier,
                             &skgpu::IsRenderingContext,
                             reporter,
                             ctxInfo,
                             require_dst_reads,
                             CtsEnforcement::kApiLevel_T) {
    auto dContext = ctxInfo.directContext();
    auto sdc = skgpu::ganesh::SurfaceDrawContext::Make(dContext,
                                                       GrColorType::kRGBA_8888,
                                                       nullptr,
                                                       SkBackingFit::kApprox,
                                                       {kWidth, kHeight},
                                                       kDMSAAProps,
                                                       /*label=*/{});

    // Initialize the texture and dmsaa attachment with transparent.
    draw_paint_with_dmsaa(sdc.get(), SK_PMColor4fTRANSPARENT, SkBlendMode::kSrc);
    check_sdc_color(reporter, sdc.get(), dContext, SK_PMColor4fTRANSPARENT);

    sdc->clear(SK_PMColor4fWHITE);
    SkPMColor4f dstColor = SK_PMColor4fWHITE;

    // Blend to the texture (not the dmsaa attachment) with a dst read. This creates a texture
    // barrier.
    draw_paint_with_aa(sdc.get(), kTransYellow, SkBlendMode::kDarken);
    dstColor = SkBlendMode_Apply(SkBlendMode::kDarken, kTransYellow, dstColor);

    // Blend to the msaa attachment _without_ a dst read. This ensures we respect the prior texture
    // barrier by splitting the opsTask.
    draw_paint_with_dmsaa(sdc.get(), kTransCyan, SkBlendMode::kSrcOver);
    dstColor = SkBlendMode_Apply(SkBlendMode::kSrcOver, kTransCyan, dstColor);

    check_sdc_color(reporter, sdc.get(), dContext, dstColor);
}

// This test is used to test for crbug.com/1241134. The bug appears on Adreno5xx devices with OS
// PQ3A. It does not repro on the earlier PPR1 version since the extend blend func extension was not
// present on the older driver.
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(DMSAA_dual_source_blend_disable,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    SkISize surfaceDims = {100, 100};
    SkISize texDims = {50, 50};
    auto context = ctxInfo.directContext();

    auto sourceTexture = context->createBackendTexture(texDims.width(),
                                                       texDims.height(),
                                                       kRGBA_8888_SkColorType,
                                                       SkColors::kBlue,
                                                       skgpu::Mipmapped::kNo,
                                                       GrRenderable::kYes,
                                                       GrProtected::kNo);

    auto sourceImage = SkImages::BorrowTextureFrom(context,
                                                   sourceTexture,
                                                   kTopLeft_GrSurfaceOrigin,
                                                   kRGBA_8888_SkColorType,
                                                   kPremul_SkAlphaType,
                                                   nullptr);

    auto texture1 = context->createBackendTexture(surfaceDims.width(),
                                                  surfaceDims.height(),
                                                  kRGBA_8888_SkColorType,
                                                  SkColors::kRed,
                                                  skgpu::Mipmapped::kNo,
                                                  GrRenderable::kYes,
                                                  GrProtected::kNo);

    auto texture2 = context->createBackendTexture(surfaceDims.width(),
                                                  surfaceDims.height(),
                                                  kRGBA_8888_SkColorType,
                                                  SkColors::kYellow,
                                                  skgpu::Mipmapped::kNo,
                                                  GrRenderable::kYes,
                                                  GrProtected::kNo);

    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc);

    SkRect srcRect = SkRect::MakeIWH(texDims.width(), texDims.height());
    SkRect dstRect = SkRect::MakeXYWH(texDims.width()/2, texDims.height()/2,
                                      texDims.width(), texDims.height());

    // First we do an image draw to a DMSAA surface with kSrc blend mode. This will trigger us to
    // use dual source blending if supported.
    // Note: The draw here doesn't actually use the dmsaa multisampled buffer. However, by using
    // a dmsaa surface it forces us to use the FillRRectOp instead of the normal FillQuad path. It
    // is unclear why, but using the FillRRectOp is required to repro the bug.
    {
        auto surface = SkSurfaces::WrapBackendTexture(context,
                                                      texture1,
                                                      kTopLeft_GrSurfaceOrigin,
                                                      1,
                                                      kRGBA_8888_SkColorType,
                                                      nullptr,
                                                      &kDMSAAProps);

        surface->getCanvas()->drawImageRect(sourceImage,
                                            srcRect,
                                            dstRect,
                                            SkSamplingOptions(),
                                            &paint,
                                            SkCanvas::kStrict_SrcRectConstraint);
        // Make sure there isn't any batching
        context->flushAndSubmit(surface.get(), GrSyncCpu::kNo);
    }

    // Next we do an image draw to a different surface that doesn't have the dmsaa flag. This will
    // trigger use to disable blending. However, when the bug is present the driver still seems to
    // try and use a "src2" blend value and ends up just writing the original dst color of yellow.
    {
        auto surface = SkSurfaces::WrapBackendTexture(context,
                                                      texture2,
                                                      kTopLeft_GrSurfaceOrigin,
                                                      1,
                                                      kRGBA_8888_SkColorType,
                                                      nullptr,
                                                      &kBasicProps);

        surface->getCanvas()->drawImageRect(sourceImage,
                                            srcRect,
                                            dstRect,
                                            SkSamplingOptions(),
                                            &paint,
                                            SkCanvas::kStrict_SrcRectConstraint);
        context->flushAndSubmit(surface.get(), GrSyncCpu::kNo);
    }

    {
        auto readImage = SkImages::BorrowTextureFrom(context,
                                                     texture2,
                                                     kTopLeft_GrSurfaceOrigin,
                                                     kRGBA_8888_SkColorType,
                                                     kPremul_SkAlphaType,
                                                     nullptr);
        SkImageInfo dstIInfo = SkImageInfo::Make(texDims.width(),
                                                 texDims.height(),
                                                 kRGBA_8888_SkColorType,
                                                 kPremul_SkAlphaType,
                                                 nullptr);

        SkBitmap bitmap;
        bitmap.allocPixels(dstIInfo);

        bool success = readImage->readPixels(context, bitmap.pixmap(), dstRect.fLeft, dstRect.fTop);
        if (!success) {
            ERRORF(reporter, "Failed to read pixels");
            return;
        }
        auto pix = static_cast<const uint32_t*>(bitmap.getAddr(0, 0));
        for (int x = 0; x < 50; ++x) {
            for (int y = 0; y < 50; ++y) {
                uint32_t pixColor = pix[x + y * 50];
                if (pixColor != 0xFFFF0000) {
                    ERRORF(reporter, "Didn't get a blue pixel at %d, %d. Got 0x%8X",
                           x, y, pixColor);
                    continue;
                }
            }
        }
    }
    sourceImage.reset();
    // Need to make sure the gpu is fully finished before deleting the textures
    context->flushAndSubmit(GrSyncCpu::kYes);
    context->deleteBackendTexture(sourceTexture);
    context->deleteBackendTexture(texture1);
    context->deleteBackendTexture(texture2);
}
