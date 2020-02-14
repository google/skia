/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/core/SkBlendModePriv.h"
#include "src/gpu/GrClip.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/ops/GrFillRectOp.h"
#include "src/gpu/ops/GrTextureOp.h"
#include "tests/Test.h"

static std::unique_ptr<GrRenderTargetContext> new_RTC(GrContext* context) {
    return GrRenderTargetContext::Make(
            context, GrColorType::kRGBA_8888, nullptr, SkBackingFit::kExact, {128, 128});
}

sk_sp<GrSurfaceProxy> create_proxy(GrContext* context) {
    static constexpr SkISize kDimensions = {128, 128};

    const GrBackendFormat format = context->priv().caps()->getDefaultBackendFormat(
                                                                           GrColorType::kRGBA_8888,
                                                                           GrRenderable::kYes);
    GrSwizzle swizzle = context->priv().caps()->getReadSwizzle(format, GrColorType::kRGBA_8888);

    return context->priv().proxyProvider()->createProxy(
            format, kDimensions, swizzle, GrRenderable::kYes, 1, GrMipMapped::kNo,
            SkBackingFit::kExact, SkBudgeted::kNo, GrProtected::kNo, GrInternalSurfaceFlags::kNone);
}

typedef GrQuadAAFlags (*PerQuadAAFunc)(int i);

typedef void (*BulkRectTest)(skiatest::Reporter* reporter, GrContext* context,
                             PerQuadAAFunc perQuadAA, GrAAType overallAA, SkBlendMode blendMode,
                             int requestedTotNumQuads, int expectedNumOps);

//-------------------------------------------------------------------------------------------------
static void bulk_fill_rect_create_test(skiatest::Reporter* reporter, GrContext* context,
                                       PerQuadAAFunc perQuadAA, GrAAType overallAA,
                                       SkBlendMode blendMode,
                                       int requestedTotNumQuads, int expectedNumOps) {

    std::unique_ptr<GrRenderTargetContext> rtc = new_RTC(context);

    auto quads = new GrRenderTargetContext::QuadSetEntry[requestedTotNumQuads];

    for (int i = 0; i < requestedTotNumQuads; ++i) {
        quads[i].fRect = SkRect::MakeWH(100.5f, 100.5f); // prevent the int non-AA optimization
        quads[i].fColor = SK_PMColor4fWHITE;
        quads[i].fLocalMatrix = SkMatrix::I();
        quads[i].fAAFlags = perQuadAA(i);
    }

    GrPaint paint;
    paint.setXPFactory(SkBlendMode_AsXPFactory(blendMode));
    GrFillRectOp::AddFillRectOps(rtc.get(), GrNoClip(), context, std::move(paint), overallAA,
                                 SkMatrix::I(), quads, requestedTotNumQuads);

    GrOpsTask* opsTask = rtc->testingOnly_PeekLastOpsTask();
    int actualNumOps = opsTask->numOpChains();

    int actualTotNumQuads = 0;

    for (int i = 0; i < actualNumOps; ++i) {
        const GrOp* tmp = opsTask->getChain(i);
        REPORTER_ASSERT(reporter, tmp->classID() == GrFillRectOp::ClassID());
        REPORTER_ASSERT(reporter, tmp->isChainTail());
        actualTotNumQuads += ((GrDrawOp*) tmp)->numQuads();
    }

    REPORTER_ASSERT(reporter, expectedNumOps == actualNumOps);
    REPORTER_ASSERT(reporter, requestedTotNumQuads == actualTotNumQuads);

    context->flush();

    delete[] quads;
}

//-------------------------------------------------------------------------------------------------
static void bulk_texture_rect_create_test(skiatest::Reporter* reporter, GrContext* context,
                                          PerQuadAAFunc perQuadAA, GrAAType overallAA,
                                          SkBlendMode blendMode,
                                          int requestedTotNumQuads, int expectedNumOps) {

    std::unique_ptr<GrRenderTargetContext> rtc = new_RTC(context);

    sk_sp<GrSurfaceProxy> proxyA = create_proxy(context);
    sk_sp<GrSurfaceProxy> proxyB = create_proxy(context);
    GrSurfaceProxyView proxyViewA(std::move(proxyA), kTopLeft_GrSurfaceOrigin, GrSwizzle::RGBA());
    GrSurfaceProxyView proxyViewB(std::move(proxyB), kTopLeft_GrSurfaceOrigin, GrSwizzle::RGBA());

    auto set = new GrRenderTargetContext::TextureSetEntry[requestedTotNumQuads];

    for (int i = 0; i < requestedTotNumQuads; ++i) {
        // Alternate between two proxies to prevent op merging if the batch API was forced to submit
        // one op at a time (to work, this does require that all fDstRects overlap).
        set[i].fProxyView = i % 2 == 0 ? proxyViewA : proxyViewB;
        set[i].fSrcAlphaType = kPremul_SkAlphaType;
        set[i].fSrcRect = SkRect::MakeWH(100.0f, 100.0f);
        set[i].fDstRect = SkRect::MakeWH(100.5f, 100.5f); // prevent the int non-AA optimization
        set[i].fDstClipQuad = nullptr;
        set[i].fPreViewMatrix = nullptr;
        set[i].fAlpha = 1.0f;
        set[i].fAAFlags = perQuadAA(i);
    }

    GrTextureOp::AddTextureSetOps(rtc.get(), GrNoClip(), context, set, requestedTotNumQuads,
                                  requestedTotNumQuads, // We alternate so proxyCnt == cnt
                                  GrSamplerState::Filter::kNearest,
                                  GrTextureOp::Saturate::kYes,
                                  blendMode,
                                  overallAA,
                                  SkCanvas::kStrict_SrcRectConstraint,
                                  SkMatrix::I(), nullptr);

    GrOpsTask* opsTask = rtc->testingOnly_PeekLastOpsTask();
    int actualNumOps = opsTask->numOpChains();

    int actualTotNumQuads = 0;

    if (blendMode != SkBlendMode::kSrcOver ||
        !context->priv().caps()->dynamicStateArrayGeometryProcessorTextureSupport()) {
        // In either of these two cases, GrTextureOp creates one op per quad instead. Since
        // each entry alternates proxies but overlaps geometrically, this will prevent the ops
        // from being merged back into fewer ops.
        expectedNumOps = requestedTotNumQuads;
    }
    uint32_t expectedOpID = blendMode == SkBlendMode::kSrcOver ? GrTextureOp::ClassID()
                                                               : GrFillRectOp::ClassID();
    for (int i = 0; i < actualNumOps; ++i) {
        const GrOp* tmp = opsTask->getChain(i);
        REPORTER_ASSERT(reporter, tmp->classID() == expectedOpID);
        REPORTER_ASSERT(reporter, tmp->isChainTail());
        actualTotNumQuads += ((GrDrawOp*) tmp)->numQuads();
    }

    REPORTER_ASSERT(reporter, expectedNumOps == actualNumOps);
    REPORTER_ASSERT(reporter, requestedTotNumQuads == actualTotNumQuads);

    context->flush();

    delete[] set;
}

//-------------------------------------------------------------------------------------------------
static void run_test(GrContext* context, skiatest::Reporter* reporter, BulkRectTest test) {
    // This is the simple case where there is no AA at all. We expect 2 non-AA clumps of quads.
    {
        auto noAA = [](int i) -> GrQuadAAFlags {
            return GrQuadAAFlags::kNone;
        };

        static const int kNumExpectedOps = 2;

        test(reporter, context, noAA, GrAAType::kNone, SkBlendMode::kSrcOver,
             2*GrResourceProvider::MaxNumNonAAQuads(), kNumExpectedOps);
    }

    // This is the same as the above case except the overall AA is kCoverage. However, since
    // the per-quad AA is still none, all the quads should be downgraded to non-AA.
    {
        auto noAA = [](int i) -> GrQuadAAFlags {
            return GrQuadAAFlags::kNone;
        };

        static const int kNumExpectedOps = 2;

        test(reporter, context, noAA, GrAAType::kCoverage, SkBlendMode::kSrcOver,
             2*GrResourceProvider::MaxNumNonAAQuads(), kNumExpectedOps);
    }

    // This case has an overall AA of kCoverage but the per-quad AA alternates.
    // We should end up with several aa-sized clumps
    {
        auto alternateAA = [](int i) -> GrQuadAAFlags {
            return (i % 2) ? GrQuadAAFlags::kAll : GrQuadAAFlags::kNone;
        };

        int numExpectedOps = 2*GrResourceProvider::MaxNumNonAAQuads() /
                                                 GrResourceProvider::MaxNumAAQuads();

        test(reporter, context, alternateAA, GrAAType::kCoverage, SkBlendMode::kSrcOver,
             2*GrResourceProvider::MaxNumNonAAQuads(), numExpectedOps);
    }

    // In this case we have a run of MaxNumAAQuads non-AA quads and then AA quads. This
    // exercises the case where we have a clump of quads that can't be upgraded to AA bc of
    // its size. We expect one clump of non-AA quads followed by one clump of AA quads.
    {
        auto runOfNonAA = [](int i) -> GrQuadAAFlags {
            return (i < GrResourceProvider::MaxNumAAQuads()) ? GrQuadAAFlags::kNone
                                                             : GrQuadAAFlags::kAll;
        };

        static const int kNumExpectedOps = 2;

        test(reporter, context, runOfNonAA, GrAAType::kCoverage, SkBlendMode::kSrcOver,
             2*GrResourceProvider::MaxNumAAQuads(), kNumExpectedOps);
    }

    // In this case we use a blend mode other than src-over, which hits the GrFillRectOp fallback
    // code path for GrTextureOp. We pass in the expected results if batching was successful, to
    // that bulk_fill_rect_create_test batches on all modes; bulk_texture_rect_create_test is
    // responsible for revising its expectations.
    {
        auto fixedAA = [](int i) -> GrQuadAAFlags {
            return GrQuadAAFlags::kAll;
        };

        static const int kNumExpectedOps = 2;

        test(reporter, context, fixedAA, GrAAType::kCoverage, SkBlendMode::kSrcATop,
             2*GrResourceProvider::MaxNumAAQuads(), kNumExpectedOps);
    }
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(BulkFillRectTest, reporter, ctxInfo) {
    run_test(ctxInfo.grContext(), reporter, bulk_fill_rect_create_test);
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(BulkTextureRectTest, reporter, ctxInfo) {
    run_test(ctxInfo.grContext(), reporter, bulk_texture_rect_create_test);
}
