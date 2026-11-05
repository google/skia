/*
 * Copyright 2020 Google LLC
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
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkSize.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/ganesh/GrBackendSurface.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/GrRecordingContext.h"
#include "include/gpu/ganesh/GrTypes.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/core/SkArenaAlloc.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/Swizzle.h"
#include "src/gpu/ganesh/GrAppliedClip.h"
#include "src/gpu/ganesh/GrCanvas.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrColorSpaceXform.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrOpsTypes.h"
#include "src/gpu/ganesh/GrProcessorSet.h"
#include "src/gpu/ganesh/GrProxyProvider.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrSamplerState.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"
#include "src/gpu/ganesh/SurfaceDrawContext.h"
#include "src/gpu/ganesh/geometry/GrQuad.h"
#include "src/gpu/ganesh/image/SkImage_Ganesh.h"
#include "src/gpu/ganesh/ops/GrOp.h"
#include "src/gpu/ganesh/ops/OpsTask.h"
#include "src/gpu/ganesh/ops/TextureOp.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"

#include <utility>

struct GrContextOptions;

using namespace skgpu::ganesh;

class OpsTaskTestingAccess {
public:
    typedef skgpu::ganesh::OpsTask::OpChain OpChain;
};

static void check_chain(OpsTaskTestingAccess::OpChain* chain, SkRect firstRect, SkRect lastRect,
                        int expectedNumOps) {
    int actualNumOps = 0;
    for (const auto& op : GrOp::ChainRange<>(chain->head())) {
        ++actualNumOps;

        if (actualNumOps == 1) {
            SkAssertResult(op.bounds() == firstRect.makeOutset(0.5f, 0.5f));
        } else if (actualNumOps == expectedNumOps) {
            SkAssertResult(op.bounds() == lastRect.makeOutset(0.5f, 0.5f));
        }
    }

    SkAssertResult(actualNumOps == expectedNumOps);
}

static sk_sp<GrSurfaceProxy> create_proxy(GrRecordingContext* rContext) {
    const GrCaps* caps = rContext->priv().caps();

    static constexpr SkISize kDimensions = {16, 16};

    const GrBackendFormat format = caps->getDefaultBackendFormat(GrColorType::kRGBA_8888,
                                                                 GrRenderable::kYes);
    return rContext->priv().proxyProvider()->createProxy(format,
                                                         kDimensions,
                                                         GrRenderable::kYes,
                                                         1,
                                                         skgpu::Mipmapped::kNo,
                                                         SkBackingFit::kExact,
                                                         skgpu::Budgeted::kNo,
                                                         GrProtected::kNo,
                                                         /*label=*/"TextureOpTest",
                                                         GrInternalSurfaceFlags::kNone);
}

static GrOp::Owner create_op(GrDirectContext* dContext, SkRect rect,
                             const GrSurfaceProxyView& proxyView, bool isAA) {
    DrawQuad quad;

    quad.fDevice = GrQuad::MakeFromRect(rect.makeOutset(0.5f, 0.5f),  SkMatrix::I());
    quad.fLocal = GrQuad(rect);
    quad.fEdgeFlags = isAA ? GrQuadAAFlags::kAll : GrQuadAAFlags::kNone;

    return TextureOp::Make(dContext,
                           proxyView,
                           kPremul_SkAlphaType,
                           nullptr,
                           GrSamplerState::Filter::kNearest,
                           GrSamplerState::MipmapMode::kNone,
                           {1.f, 1.f, 1.f, 1.f},
                           TextureOp::Saturate::kYes,
                           SkBlendMode::kSrcOver,
                           isAA ? GrAAType::kCoverage
                                : GrAAType::kNone,
                           &quad,
                           nullptr);
}

// This unit test exercises the crbug.com/1112259 case.
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(TextureOpTest, reporter, ctxInfo, CtsEnforcement::kNever) {
    GrDirectContext* dContext = ctxInfo.directContext();
    const GrCaps* caps = dContext->priv().caps();
    SkArenaAlloc arena{nullptr, 0, 1024};
    auto auditTrail = dContext->priv().auditTrail();

    if (!caps->dynamicStateArrayGeometryProcessorTextureSupport()) {
        // This test requires chaining
        return;
    }

    GrSurfaceProxyView proxyViewA(create_proxy(dContext),
                                  kTopLeft_GrSurfaceOrigin,
                                  skgpu::Swizzle::RGBA());
    GrSurfaceProxyView proxyViewB(create_proxy(dContext),
                                  kTopLeft_GrSurfaceOrigin,
                                  skgpu::Swizzle::RGBA());
    GrSurfaceProxyView proxyViewC(create_proxy(dContext),
                                  kTopLeft_GrSurfaceOrigin,
                                  skgpu::Swizzle::RGBA());

    static const SkRect kOpARect{  0,  0, 16, 16 };
    static const SkRect kOpBRect{ 32,  0, 48, 16 };
    static const SkRect kOpCRect{  0, 32, 16, 48 };
    static const SkRect kOpDRect{ 32, 32, 48, 48 };

    // opA & opB can chain together but can't merge bc they have different proxies
    GrOp::Owner opA = create_op(dContext, kOpARect, proxyViewA, false);
    GrOp::Owner opB = create_op(dContext, kOpBRect, proxyViewB, false);

    GrAppliedClip noClip = GrAppliedClip::Disabled();
    OpsTaskTestingAccess::OpChain chain1(std::move(opA), GrProcessorSet::EmptySetAnalysis(),
                                         &noClip, nullptr);
    chain1.appendOp(std::move(opB), GrProcessorSet::EmptySetAnalysis(), nullptr, &noClip, *caps,
                    &arena, dContext->priv().auditTrail());
    check_chain(&chain1, kOpARect, kOpBRect, 2);

    // opC & opD can also chain together but can't merge (bc, again, they have different
    // proxies). Note, however, that opA and opD do share a proxy so can be merged if opA's
    // anti-aliasing is upgraded to coverage.
    GrOp::Owner opC = create_op(dContext, kOpCRect, proxyViewC, true);
    GrOp::Owner opD = create_op(dContext, kOpDRect, proxyViewA, true);

    OpsTaskTestingAccess::OpChain chain2(std::move(opC), GrProcessorSet::EmptySetAnalysis(),
                                         &noClip, nullptr);
    chain2.appendOp(std::move(opD), GrProcessorSet::EmptySetAnalysis(), nullptr, &noClip, *caps,
                    &arena, auditTrail);
    check_chain(&chain2, kOpCRect, kOpDRect, 2);

    // opA and opD, now in separate chains, can merge when the two chains are combined while
    // opB and opC can still only chain.
    chain1.prependChain(&chain2, *caps, &arena, auditTrail);

    // We should end up with the chain
    //   opC - opD/opA - opB
    check_chain(&chain1, kOpCRect, kOpBRect, 3);

    chain1.deleteOps();
}

// Test that TextureOp does not overshoot a strict subset edge during nearest-neighbor
// magnification sampling (crbug.com/576937680). Due to limited sub-texel precision
// in hardware texture units (8-bit for D3D11, 4-bit for Vulkan), round-to-nearest snaps
// coordinates close to the boundary to the adjacent texel unless clamped in the shader.
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(TextureOpNearestSubsetOvershootTest,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNever) {
    GrDirectContext* dContext = ctxInfo.directContext();

    // Create a source surface where the left half [0, 0, 4, 4] is BLUE and the right
    // half [4, 0, 8, 4] is RED.
    SkImageInfo srcInfo =
            SkImageInfo::Make(8, 4, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    auto srcSurface = SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kYes, srcInfo);
    if (!srcSurface) {
        return;
    }
    srcSurface->getCanvas()->clear(SK_ColorRED);
    SkPaint bluePaint;
    bluePaint.setColor(SK_ColorBLUE);
    bluePaint.setBlendMode(SkBlendMode::kSrc);
    srcSurface->getCanvas()->drawRect(SkRect::MakeWH(4, 4), bluePaint);

    sk_sp<SkImage> srcImage = srcSurface->makeImageSnapshot();

    // Create a destination surface with width 2048 (512x magnification of width 4,
    // which exceeds 8-bit precision: 2048 / 4 = 512 > 256).
    const int dstW = 2048;
    const int dstH = 4;
    SkImageInfo dstInfo =
            SkImageInfo::Make(dstW, dstH, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    auto dstSurface = SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kYes, dstInfo);
    if (!dstSurface) {
        return;
    }
    dstSurface->getCanvas()->clear(SK_ColorTRANSPARENT);

    // Draw the subset [0, 0, 4, 4] with strict constraint, nearest filtering, and no AA.
    SkPaint paint;
    paint.setAntiAlias(false);
    dstSurface->getCanvas()->drawImageRect(
            srcImage,
            SkRect::MakeWH(4, 4),
            SkRect::MakeWH(dstW, dstH),
            SkSamplingOptions(SkFilterMode::kNearest),
            &paint,
            SkCanvas::kStrict_SrcRectConstraint);

    // Read back the rightmost column (x = dstW - 1).
    // If the subset clamp was improperly ignored, the sampler snaps to texel 4 (RED).
    // With proper clamping, it samples texel 3 (BLUE).
    SkBitmap bm;
    bm.allocPixels(SkImageInfo::Make(1, 1, kRGBA_8888_SkColorType, kPremul_SkAlphaType));
    REPORTER_ASSERT(reporter, dstSurface->readPixels(bm, dstW - 1, 0));
    REPORTER_ASSERT(reporter, bm.getColor(0, 0) == SK_ColorBLUE);

    const SkImageInfo dst1to1Info =
            SkImageInfo::Make(4, 4, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    // Test 1:1 integer draw (should hit fast path and safely sample BLUE at x = 3).
    auto dstSurface1to1 = SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kYes, dst1to1Info);
    if (dstSurface1to1) {
        dstSurface1to1->getCanvas()->clear(SK_ColorTRANSPARENT);
        dstSurface1to1->getCanvas()->drawImageRect(
                srcImage,
                SkRect::MakeWH(4, 4),
                SkRect::MakeWH(4, 4),
                SkSamplingOptions(SkFilterMode::kNearest),
                &paint,
                SkCanvas::kStrict_SrcRectConstraint);
        REPORTER_ASSERT(reporter, dstSurface1to1->readPixels(bm, 3, 0));
        REPORTER_ASSERT(reporter, bm.getColor(0, 0) == SK_ColorBLUE);
    }

    // Test 1:1 draw with non-integer relative translation placing pixel center close to
    // subset edge. The subset clamp must be retained to prevent sampling outside the subset.
    auto dstSurfaceFrac = SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kYes, dst1to1Info);
    if (dstSurfaceFrac) {
        dstSurfaceFrac->getCanvas()->clear(SK_ColorTRANSPARENT);
        dstSurfaceFrac->getCanvas()->drawImageRect(
                srcImage,
                SkRect::MakeXYWH(4, 0, 4, 4),
                SkRect::MakeXYWH(0.5001f, 0.f, 4.f, 4.f),
                SkSamplingOptions(SkFilterMode::kNearest),
                &paint,
                SkCanvas::kStrict_SrcRectConstraint);
        for (int x = 0; x < 4; ++x) {
            REPORTER_ASSERT(reporter, dstSurfaceFrac->readPixels(bm, x, 0));
            REPORTER_ASSERT(reporter, bm.getColor(0, 0) == SK_ColorRED);
        }
    }

    // Test 1:1 draw where both src and dst have matching fractional offsets (integer relative
    // translation, e.g. dst.left - src.left == 0). Fast path is safe and samples BLUE.
    auto dstSurfaceMatchingFrac =
            SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kYes, dst1to1Info);
    if (dstSurfaceMatchingFrac) {
        dstSurfaceMatchingFrac->getCanvas()->clear(SK_ColorTRANSPARENT);
        dstSurfaceMatchingFrac->getCanvas()->drawImageRect(
                srcImage,
                SkRect::MakeXYWH(0.25f, 0.f, 3.f, 4.f),
                SkRect::MakeXYWH(0.25f, 0.f, 3.f, 4.f),
                SkSamplingOptions(SkFilterMode::kNearest),
                &paint,
                SkCanvas::kStrict_SrcRectConstraint);
        for (int x = 0; x < 3; ++x) {
            REPORTER_ASSERT(reporter, dstSurfaceMatchingFrac->readPixels(bm, x, 0));
            REPORTER_ASSERT(reporter, bm.getColor(0, 0) == SK_ColorBLUE);
        }
    }

    // Test 1:1 draw with a horizontal flip (e.g. canvas scale(-1, 1)).
    // Fast path is safe and samples BLUE across all 4 pixels.
    auto dstSurfaceFlipped =
            SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kYes, dst1to1Info);
    if (dstSurfaceFlipped) {
        dstSurfaceFlipped->getCanvas()->clear(SK_ColorTRANSPARENT);
        dstSurfaceFlipped->getCanvas()->save();
        dstSurfaceFlipped->getCanvas()->translate(4.f, 0.f);
        dstSurfaceFlipped->getCanvas()->scale(-1.f, 1.f);
        dstSurfaceFlipped->getCanvas()->drawImageRect(
                srcImage,
                SkRect::MakeWH(4, 4),
                SkRect::MakeWH(4, 4),
                SkSamplingOptions(SkFilterMode::kNearest),
                &paint,
                SkCanvas::kStrict_SrcRectConstraint);
        dstSurfaceFlipped->getCanvas()->restore();
        for (int x = 0; x < 4; ++x) {
            REPORTER_ASSERT(reporter, dstSurfaceFlipped->readPixels(bm, x, 0));
            REPORTER_ASSERT(reporter, bm.getColor(0, 0) == SK_ColorBLUE);
        }
    }

    // Test 1:1 draw with a 90-degree rotation.
    // Fast path is safe and samples BLUE across all 4 pixels.
    auto dstSurfaceRot90 =
            SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kYes, dst1to1Info);
    if (dstSurfaceRot90) {
        dstSurfaceRot90->getCanvas()->clear(SK_ColorTRANSPARENT);
        dstSurfaceRot90->getCanvas()->save();
        dstSurfaceRot90->getCanvas()->rotate(90.f);
        dstSurfaceRot90->getCanvas()->translate(0.f, -4.f);
        dstSurfaceRot90->getCanvas()->drawImageRect(
                srcImage,
                SkRect::MakeWH(4, 4),
                SkRect::MakeWH(4, 4),
                SkSamplingOptions(SkFilterMode::kNearest),
                &paint,
                SkCanvas::kStrict_SrcRectConstraint);
        dstSurfaceRot90->getCanvas()->restore();
        for (int y = 0; y < 4; ++y) {
            for (int x = 0; x < 4; ++x) {
                REPORTER_ASSERT(reporter, dstSurfaceRot90->readPixels(bm, x, y));
                REPORTER_ASSERT(reporter, bm.getColor(0, 0) == SK_ColorBLUE);
            }
        }
    }

    // Test 1:1 draw with a 270-degree rotation.
    // Fast path is safe and samples BLUE across all 4 pixels.
    auto dstSurfaceRot270 =
            SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kYes, dst1to1Info);
    if (dstSurfaceRot270) {
        dstSurfaceRot270->getCanvas()->clear(SK_ColorTRANSPARENT);
        dstSurfaceRot270->getCanvas()->save();
        dstSurfaceRot270->getCanvas()->rotate(270.f);
        dstSurfaceRot270->getCanvas()->translate(-4.f, 0.f);
        dstSurfaceRot270->getCanvas()->drawImageRect(
                srcImage,
                SkRect::MakeWH(4, 4),
                SkRect::MakeWH(4, 4),
                SkSamplingOptions(SkFilterMode::kNearest),
                &paint,
                SkCanvas::kStrict_SrcRectConstraint);
        dstSurfaceRot270->getCanvas()->restore();
        for (int y = 0; y < 4; ++y) {
            for (int x = 0; x < 4; ++x) {
                REPORTER_ASSERT(reporter, dstSurfaceRot270->readPixels(bm, x, y));
                REPORTER_ASSERT(reporter, bm.getColor(0, 0) == SK_ColorBLUE);
            }
        }
    }

    // Test approx-fit proxy with kFast constraint is upgraded to kStrict and does not overshoot.
    auto approxSDC = skgpu::ganesh::SurfaceDrawContext::Make(
            dContext,
            GrColorType::kRGBA_8888,
            nullptr,
            SkBackingFit::kApprox,
            SkISize::Make(8, 4),
            SkSurfaceProps(),
            "ApproxSDC",
            1,
            skgpu::Mipmapped::kNo,
            skgpu::Protected::kNo,
            kTopLeft_GrSurfaceOrigin);
    if (approxSDC) {
        approxSDC->clear(SkPMColor4f{1.f, 0.f, 0.f, 1.f});
        GrPaint blueGrPaint;
        blueGrPaint.setColor4f(SkPMColor4f{0.f, 0.f, 1.f, 1.f});
        approxSDC->drawRect(nullptr, std::move(blueGrPaint), GrAA::kNo, SkMatrix::I(),
                            SkRect::MakeWH(4, 4));

        sk_sp<SkImage> approxImage = sk_make_sp<SkImage_Ganesh>(
                sk_ref_sp(dContext),
                kNeedNewImageUniqueID,
                approxSDC->readSurfaceView(),
                srcInfo.colorInfo());

        auto dstSurfaceApprox = SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kYes, dstInfo);
        if (dstSurfaceApprox) {
            dstSurfaceApprox->getCanvas()->clear(SK_ColorTRANSPARENT);
            dstSurfaceApprox->getCanvas()->drawImageRect(
                    approxImage,
                    SkRect::MakeWH(4, 4),
                    SkRect::MakeWH(dstW, dstH),
                    SkSamplingOptions(SkFilterMode::kNearest),
                    &paint,
                    SkCanvas::kFast_SrcRectConstraint);
            REPORTER_ASSERT(reporter, dstSurfaceApprox->readPixels(bm, dstW - 1, 0));
            REPORTER_ASSERT(reporter, bm.getColor(0, 0) == SK_ColorBLUE);
        }
    }
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(TextureOpCoverageAASubsetClampTest,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNever) {
    GrDirectContext* dContext = ctxInfo.directContext();

    auto checkNoRedPixels = [&](SkSurface* dstSurface, const char* label) {
        SkBitmap bm;
        bm.allocPixels(dstSurface->imageInfo());
        REPORTER_ASSERT(reporter, dstSurface->readPixels(bm, 0, 0));
        int badPixels = 0;
        int firstX = -1, firstY = -1;
        SkColor firstColor = 0;
        for (int y = 0; y < bm.height(); ++y) {
            for (int x = 0; x < bm.width(); ++x) {
                SkColor c = bm.getColor(x, y);
                if (SkColorGetR(c) != 0) {
                    if (badPixels == 0) {
                        firstX = x;
                        firstY = y;
                        firstColor = c;
                    }
                    ++badPixels;
                }
            }
        }
        REPORTER_ASSERT(reporter,
                        badPixels == 0,
                        "%s: %d pixels had red > 0 (first at (%d, %d) = 0x%08x)",
                        label,
                        badPixels,
                        firstX,
                        firstY,
                        firstColor);
    };

    auto srcSurface = SkSurfaces::RenderTarget(
            dContext,
            skgpu::Budgeted::kYes,
            SkImageInfo::Make(128, 64, kRGBA_8888_SkColorType, kPremul_SkAlphaType));
    auto dstSurface = SkSurfaces::RenderTarget(
            dContext,
            skgpu::Budgeted::kYes,
            SkImageInfo::Make(16, 32, kRGBA_8888_SkColorType, kPremul_SkAlphaType));
    if (!srcSurface || !dstSurface) {
        return;
    }

    SkPaint bluePaint;
    bluePaint.setColor(SK_ColorBLUE);
    bluePaint.setBlendMode(SkBlendMode::kSrc);
    srcSurface->getCanvas()->clear(SK_ColorRED);
    srcSurface->getCanvas()->drawRect(SkRect::MakeWH(64, 64), bluePaint);
    sk_sp<SkImage> img = srcSurface->makeImageSnapshot();

    SkPaint aaPaint;
    aaPaint.setAntiAlias(true);

    // 1. TextureOpImpl path: crop all four edges with an AA clip so the local quad is > 0.5 texels
    // inside the [0, 0, 64, 64] subset, while ~35x minification in x causes the 0.5px coverage-AA
    // fringe to extrapolate ~18 texels past the subset edge unless clamped.
    {
        SkCanvas* canvas = dstSurface->getCanvas();
        canvas->clear(SK_ColorBLACK);
        canvas->save();
        canvas->clipRect(SkRect::MakeLTRB(2.6f, 2.f, 4.3f, 30.f),
                         SkClipOp::kIntersect,
                         /*doAntiAlias=*/true);
        canvas->drawImageRect(img,
                              SkRect::MakeWH(64, 64),
                              SkRect::MakeLTRB(2.58f, 1.f, 4.32f, 31.f),
                              SkSamplingOptions(SkFilterMode::kLinear),
                              &aaPaint,
                              SkCanvas::kStrict_SrcRectConstraint);
        canvas->restore();
        checkNoRedPixels(dstSurface.get(), "TextureOpImpl strict AA subset");
    }

    // 2. FillRectOp fallback in TextureOp::Make (non-kSrcOver blend mode): isolates both
    // safe_to_ignore_subset_rect() and the aaType == kNone domain guard on GrTextureEffect.
    {
        dstSurface->getCanvas()->clear(SK_ColorBLACK);
        auto* srcSDC = skgpu::ganesh::TopDeviceSurfaceDrawContext(srcSurface->getCanvas());
        auto* dstSDC = skgpu::ganesh::TopDeviceSurfaceDrawContext(dstSurface->getCanvas());
        const SkRect subset = SkRect::MakeWH(64, 64);
        SkPoint srcQuad[4], dstQuad[4];
        SkRect::MakeLTRB(2.f, 2.f, 62.f, 62.f).toQuad(srcQuad);
        SkRect::MakeLTRB(2.58f, 1.f, 4.32f, 31.f).toQuad(dstQuad);
        dstSDC->drawTextureQuad(nullptr,
                                srcSDC->readSurfaceView(),
                                GrColorType::kRGBA_8888,
                                kPremul_SkAlphaType,
                                GrSamplerState::Filter::kLinear,
                                GrSamplerState::MipmapMode::kNone,
                                SkBlendMode::kPlus,
                                {1.f, 1.f, 1.f, 1.f},
                                srcQuad,
                                dstQuad,
                                GrQuadAAFlags::kAll,
                                &subset,
                                SkMatrix::I(),
                                nullptr);
        checkNoRedPixels(dstSurface.get(), "TextureOp::Make FillRectOp fallback");
    }

    // 3. Device::drawEdgeAAImage() general FP path: non-AA paint with a color filter (forcing the
    // FP path), cropped by an AA clip with nearest filtering and fractional src edges.
    {
        SkCanvas* canvas = dstSurface->getCanvas();
        canvas->clear(SK_ColorBLACK);
        canvas->save();
        SkPaint cfPaint;
        cfPaint.setColorFilter(SkColorFilters::LinearToSRGBGamma());
        canvas->clipRect(SkRect::MakeLTRB(2.6f, 2.f, 4.3f, 30.f),
                         SkClipOp::kIntersect,
                         /*doAntiAlias=*/true);
        canvas->drawImageRect(img,
                              SkRect::MakeLTRB(0.6f, 0.6f, 63.4f, 63.4f),
                              SkRect::MakeLTRB(2.58f, 1.f, 4.32f, 31.f),
                              SkSamplingOptions(SkFilterMode::kNearest),
                              &cfPaint,
                              SkCanvas::kStrict_SrcRectConstraint);
        canvas->restore();
        checkNoRedPixels(dstSurface.get(), "drawEdgeAAImage general FP path");
    }

    // 4. Non-exact (SkBackingFit::kApprox) proxy paths with kFast_SrcRectConstraint, AA, and
    // minification: clear a 65x64 kApprox surface (backed by 128x64) to red (which clears the full
    // backing store via a load-op clear), then fill [0, 0, 65, 64] with blue using two sub-rects so
    // attemptQuadOptimization does not promote a full-bounds draw into a full-backing-store clear.
    {
        auto approxSDC = skgpu::ganesh::SurfaceDrawContext::Make(
                dContext,
                GrColorType::kRGBA_8888,
                nullptr,
                SkBackingFit::kApprox,
                SkISize::Make(65, 64),
                SkSurfaceProps(),
                "ApproxMinSDC",
                1,
                skgpu::Mipmapped::kNo,
                skgpu::Protected::kNo,
                kTopLeft_GrSurfaceOrigin);
        if (approxSDC) {
            REPORTER_ASSERT(reporter, !approxSDC->asSurfaceProxy()->isFunctionallyExact());
            approxSDC->clear(SkPMColor4f{1.f, 0.f, 0.f, 1.f});
            auto fillBlue = [&](const SkRect& r) {
                GrPaint p;
                p.setColor4f({0.f, 0.f, 1.f, 1.f});
                approxSDC->drawRect(nullptr, std::move(p), GrAA::kNo, SkMatrix::I(), r);
            };
            fillBlue(SkRect::MakeLTRB(0.f, 0.f, 64.f, 64.f));
            fillBlue(SkRect::MakeLTRB(64.f, 0.f, 65.f, 64.f));

            // Use a srcRect inset by > 0.5 texels inside the 65x64 logical bounds so
            // draw_texture() does not upgrade kFast to kStrict on srcRect.
            const SkRect srcRect = SkRect::MakeLTRB(2.f, 2.f, 63.f, 62.f);
            const SkRect dstRect = SkRect::MakeLTRB(2.58f, 1.f, 4.32f, 31.f);

            SkSurfaceProps dmsaaProps(SkSurfaceProps::kDynamicMSAA_Flag, kUnknown_SkPixelGeometry);
            auto dmsaaDstSurface = SkSurfaces::RenderTarget(
                    dContext,
                    skgpu::Budgeted::kYes,
                    SkImageInfo::Make(16, 32, kRGBA_8888_SkColorType, kPremul_SkAlphaType),
                    /*sampleCount=*/1,
                    kTopLeft_GrSurfaceOrigin,
                    &dmsaaProps);

            for (SkSurface* target : {dstSurface.get(), dmsaaDstSurface.get()}) {
                if (!target) {
                    continue;
                }
                const bool isDMSAA = (target == dmsaaDstSurface.get());
                auto* targetSDC = skgpu::ganesh::TopDeviceSurfaceDrawContext(target->getCanvas());

                // (a) SurfaceDrawContext::drawTexture with kFast (tests TextureOp::Make
                // proxy-bounds subset and the DMSAA branch of SurfaceDrawContext::drawTexture).
                target->getCanvas()->clear(SK_ColorBLACK);
                targetSDC->drawTexture(nullptr,
                                       approxSDC->readSurfaceView(),
                                       kPremul_SkAlphaType,
                                       GrSamplerState::Filter::kLinear,
                                       GrSamplerState::MipmapMode::kNone,
                                       SkBlendMode::kSrcOver,
                                       {1.f, 1.f, 1.f, 1.f},
                                       srcRect,
                                       dstRect,
                                       GrQuadAAFlags::kAll,
                                       SkCanvas::kFast_SrcRectConstraint,
                                       SkMatrix::I(),
                                       nullptr);
                checkNoRedPixels(target,
                                 isDMSAA ? "drawTexture kApprox with kFast (DMSAA)"
                                         : "drawTexture kApprox with kFast");

                // (b) SurfaceDrawContext::drawTextureSet with kFast (tests TextureOpImpl batch
                // constructor proxy-bounds subset).
                target->getCanvas()->clear(SK_ColorBLACK);
                GrTextureSetEntry entry{approxSDC->readSurfaceView(),
                                        kPremul_SkAlphaType,
                                        srcRect,
                                        dstRect,
                                        /*fDstClipQuad=*/nullptr,
                                        /*fPreViewMatrix=*/nullptr,
                                        {1.f, 1.f, 1.f, 1.f},
                                        GrQuadAAFlags::kAll};
                targetSDC->drawTextureSet(nullptr,
                                          &entry,
                                          /*cnt=*/1,
                                          /*proxyRunCnt=*/1,
                                          GrSamplerState::Filter::kLinear,
                                          GrSamplerState::MipmapMode::kNone,
                                          SkBlendMode::kSrcOver,
                                          SkCanvas::kFast_SrcRectConstraint,
                                          SkMatrix::I(),
                                          nullptr,
                                          /*setMayHavePersp=*/false);
                checkNoRedPixels(target,
                                 isDMSAA ? "drawTextureSet kApprox with kFast (DMSAA)"
                                         : "drawTextureSet kApprox with kFast");
            }
        }
    }

    // 5. DMSAA destination surface with a non-AA clipRect and QuadAAFlags::kNone on the general FP
    // path of Device::drawEdgeAAImage(): ensures !sdc->alwaysAntialias() is checked before
    // Device::isClipAntiAliased() (which asserts !alwaysAntialias() on non-AA clip elements).
    {
        SkSurfaceProps dmsaaProps(SkSurfaceProps::kDynamicMSAA_Flag, kUnknown_SkPixelGeometry);
        auto dmsaaDst = SkSurfaces::RenderTarget(
                dContext,
                skgpu::Budgeted::kYes,
                SkImageInfo::Make(16, 32, kRGBA_8888_SkColorType, kPremul_SkAlphaType),
                /*sampleCount=*/1,
                kTopLeft_GrSurfaceOrigin,
                &dmsaaProps);
        if (dmsaaDst) {
            SkCanvas* canvas = dmsaaDst->getCanvas();
            canvas->clear(SK_ColorBLACK);
            canvas->save();
            // Must be smaller than the device: a device-sized clip is discarded by ClipStack,
            // leaving no non-AA element for isClipAntiAliased() to visit.
            canvas->clipRect(
                    SkRect::MakeLTRB(1, 1, 15, 31), SkClipOp::kIntersect, /*doAntiAlias=*/false);
            SkPaint cfPaint;
            cfPaint.setColorFilter(SkColorFilters::LinearToSRGBGamma());
            SkCanvas::ImageSetEntry setEntry(img,
                                             SkRect::MakeLTRB(2.f, 2.f, 62.f, 62.f),
                                             SkRect::MakeLTRB(2.58f, 1.f, 4.32f, 31.f),
                                             /*alpha=*/1.f,
                                             SkCanvas::kNone_QuadAAFlags);
            canvas->experimental_DrawEdgeAAImageSet(&setEntry,
                                                    /*cnt=*/1,
                                                    /*dstClips=*/nullptr,
                                                    /*preViewMatrices=*/nullptr,
                                                    SkSamplingOptions(SkFilterMode::kLinear),
                                                    &cfPaint,
                                                    SkCanvas::kStrict_SrcRectConstraint);
            canvas->restore();
            checkNoRedPixels(dmsaaDst.get(), "DMSAA non-AA clipRect with DrawEdgeAAImageSet");
        }
    }
}


