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
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrColorSpaceXform.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
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
    SkImageInfo srcInfo = SkImageInfo::MakeN32Premul(8, 4);
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
    SkImageInfo dstInfo = SkImageInfo::MakeN32Premul(dstW, dstH);
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
    bm.allocN32Pixels(1, 1);
    REPORTER_ASSERT(reporter, dstSurface->readPixels(bm, dstW - 1, 0));
    REPORTER_ASSERT(reporter, bm.getColor(0, 0) == SK_ColorBLUE);

    // Test 1:1 integer draw (should hit fast path and safely sample BLUE at x = 3).
    auto dstSurface1to1 = SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kYes,
                                                   SkImageInfo::MakeN32Premul(4, 4));
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
    auto dstSurfaceFrac = SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kYes,
                                                   SkImageInfo::MakeN32Premul(4, 4));
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
    auto dstSurfaceMatchingFrac = SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kYes,
                                                           SkImageInfo::MakeN32Premul(4, 4));
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
    auto dstSurfaceFlipped = SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kYes,
                                                      SkImageInfo::MakeN32Premul(4, 4));
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
    auto dstSurfaceRot90 = SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kYes,
                                                    SkImageInfo::MakeN32Premul(4, 4));
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
    auto dstSurfaceRot270 = SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kYes,
                                                     SkImageInfo::MakeN32Premul(4, 4));
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


