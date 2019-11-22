/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrClip.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/ops/GrFillRectOp.h"
#include "src/gpu/ops/GrTextureOp.h"
#include "tests/Test.h"

static std::unique_ptr<GrRenderTargetContext> new_RTC(GrContext* context) {
    return context->priv().makeDeferredRenderTargetContext(SkBackingFit::kExact, 128, 128,
                                                           GrColorType::kRGBA_8888, nullptr);
}

sk_sp<GrSurfaceProxy> create_proxy(GrContext* context) {
    GrSurfaceDesc desc;
    desc.fConfig = kRGBA_8888_GrPixelConfig;
    desc.fWidth  = 128;
    desc.fHeight = 128;

    const GrBackendFormat format = context->priv().caps()->getDefaultBackendFormat(
                                                                           GrColorType::kRGBA_8888,
                                                                           GrRenderable::kYes);

    return context->priv().proxyProvider()->createProxy(
        format, desc, GrRenderable::kYes, 1, kTopLeft_GrSurfaceOrigin, GrMipMapped::kNo,
        SkBackingFit::kExact, SkBudgeted::kNo, GrProtected::kNo, GrInternalSurfaceFlags::kNone);
}

typedef GrQuadAAFlags (*PerQuadAAFunc)(int i);

typedef void (*BulkRectTest)(skiatest::Reporter* reporter, GrContext* context,
                             PerQuadAAFunc perQuadAA, GrAAType overallAA,
                             int requestedTotNumQuads, int expectedNumOps);

//-------------------------------------------------------------------------------------------------
static void bulk_fill_rect_create_test(skiatest::Reporter* reporter, GrContext* context,
                                       PerQuadAAFunc perQuadAA, GrAAType overallAA,
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
                                          int requestedTotNumQuads, int expectedNumOps) {

    std::unique_ptr<GrRenderTargetContext> rtc = new_RTC(context);

    sk_sp<GrSurfaceProxy> proxy = create_proxy(context);

    GrSurfaceProxyView proxyView(std::move(proxy), kTopLeft_GrSurfaceOrigin, GrSwizzle::RGBA());

    auto set = new GrRenderTargetContext::TextureSetEntry[requestedTotNumQuads];

    for (int i = 0; i < requestedTotNumQuads; ++i) {
        set[i].fProxyView = proxyView;
        set[i].fSrcColorType = GrColorType::kRGBA_8888;
        set[i].fSrcRect = SkRect::MakeWH(100.0f, 100.0f);
        set[i].fDstRect = SkRect::MakeWH(100.5f, 100.5f); // prevent the int non-AA optimization
        set[i].fDstClipQuad = nullptr;
        set[i].fPreViewMatrix = nullptr;
        set[i].fAlpha = 1.0f;
        set[i].fAAFlags = perQuadAA(i);
    }

    GrTextureOp::CreateTextureSetOps(rtc.get(), GrNoClip(), context, set, requestedTotNumQuads,
                                     GrSamplerState::Filter::kNearest,
                                     GrTextureOp::Saturate::kYes,
                                     overallAA,
                                     SkCanvas::kStrict_SrcRectConstraint,
                                     SkMatrix::I(), nullptr);

    GrOpsTask* opsTask = rtc->testingOnly_PeekLastOpsTask();
    int actualNumOps = opsTask->numOpChains();

    int actualTotNumQuads = 0;

    for (int i = 0; i < actualNumOps; ++i) {
        const GrOp* tmp = opsTask->getChain(i);
        REPORTER_ASSERT(reporter, tmp->classID() == GrTextureOp::ClassID());
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

    if (!context->priv().caps()->dynamicStateArrayGeometryProcessorTextureSupport()) {
        return;
    }

    // This is the simple case where there is no AA at all. We expect 2 non-AA clumps of quads.
    {
        auto noAA = [](int i) -> GrQuadAAFlags {
            return GrQuadAAFlags::kNone;
        };

        static const int kNumExpectedOps = 2;

        test(reporter, context, noAA, GrAAType::kNone,
             2*GrResourceProvider::MaxNumNonAAQuads(), kNumExpectedOps);
    }

    // This is the same as the above case except the overall AA is kCoverage. However, since
    // the per-quad AA is still none, all the quads should be downgraded to non-AA.
    {
        auto noAA = [](int i) -> GrQuadAAFlags {
            return GrQuadAAFlags::kNone;
        };

        static const int kNumExpectedOps = 2;

        test(reporter, context, noAA, GrAAType::kCoverage,
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

        test(reporter, context, alternateAA, GrAAType::kCoverage,
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

        test(reporter, context, runOfNonAA, GrAAType::kCoverage,
             2*GrResourceProvider::MaxNumAAQuads(), kNumExpectedOps);
    }
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(BulkFillRectTest, reporter, ctxInfo) {
    run_test(ctxInfo.grContext(), reporter, bulk_fill_rect_create_test);
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(BulkTextureRectTest, reporter, ctxInfo) {
    run_test(ctxInfo.grContext(), reporter, bulk_texture_rect_create_test);
}
