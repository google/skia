/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrClip.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/ops/GrTextureOp.h"
#include "tests/Test.h"

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

typedef GrQuadAAFlags (*GimmeSomeAAFunc)(int i);

static void bulk_rect_create_test(skiatest::Reporter* reporter, GrContext* context,
                                  GimmeSomeAAFunc gimmeSomeAA, GrAAType overallAA,
                                  int requestedTotNumQuads, int expectedNumOpsInChain) {

    sk_sp<GrSurfaceProxy> proxy = create_proxy(context);

    GrSurfaceProxyView proxyView(std::move(proxy), kTopLeft_GrSurfaceOrigin, GrSwizzle::RGBA());

    auto set = new GrRenderTargetContext::TextureSetEntry[requestedTotNumQuads];

    for (int i = 0; i < requestedTotNumQuads; ++i) {
        set[i].fProxyView = proxyView;
        set[i].fSrcColorType = GrColorType::kRGBA_8888;
        set[i].fSrcRect = SkRect::MakeWH(100, 100);
        set[i].fDstRect = SkRect::MakeWH(100, 100);
        set[i].fDstClipQuad = nullptr;
        set[i].fPreViewMatrix = nullptr;
        set[i].fAlpha = 1.0f;
        set[i].fAAFlags = gimmeSomeAA(i);
    }

    std::unique_ptr<GrOp> op = GrTextureOp::MakeSet(context, set, requestedTotNumQuads,
                                                    GrSamplerState::Filter::kNearest,
                                                    GrTextureOp::Saturate::kYes,
                                                    overallAA,
                                                    SkCanvas::kStrict_SrcRectConstraint,
                                                    SkMatrix::I(), nullptr);

    SkASSERT(!op->prevInChain());

    int actualTotNumQuads = 0;
    int actualNumOpsInChain = 0;
    for (GrDrawOp* tmp = (GrDrawOp*) op.get(); tmp; tmp = (GrDrawOp*) tmp->nextInChain()) {
        ++actualNumOpsInChain;
        actualTotNumQuads += tmp->numQuads();
    }

    REPORTER_ASSERT(reporter, expectedNumOpsInChain == actualNumOpsInChain);
    REPORTER_ASSERT(reporter, requestedTotNumQuads == actualTotNumQuads);

    {
        GrOpMemoryPool* pool = context->priv().opMemoryPool();

        while (op) {
            std::unique_ptr<GrOp> next = op->cutChain();
            pool->release(std::move(op));
            op = std::move(next);
        }
    }

    delete[] set;
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(BulkRect, reporter, ctxInfo) {
    // This is the simple case where there is no AA at all. We expect 2 non-AA clumps of quads.
    {
        auto noAA = [](int i) -> GrQuadAAFlags {
            return GrQuadAAFlags::kNone;
        };

        static const int kNumExpectedClumps = 2;

        bulk_rect_create_test(reporter, ctxInfo.grContext(), noAA, GrAAType::kNone,
                              2*GrResourceProvider::MaxNumNonAAQuads(), kNumExpectedClumps);
    }

    // This is the same as the above case except the overall AA is kCoverage. However, since
    // the per-quad AA is still none, all the quads should be downgraded to non-AA.
    {
        auto noAA = [](int i) -> GrQuadAAFlags {
            return GrQuadAAFlags::kNone;
        };

        static const int kNumExpectedClumps = 2;

        bulk_rect_create_test(reporter, ctxInfo.grContext(), noAA, GrAAType::kCoverage,
                              2*GrResourceProvider::MaxNumNonAAQuads(), kNumExpectedClumps);
    }

    // This case has an overall AA of kCoverage but the per-quad AA alternates.
    // We should end up with several aa-sized clumps
    {
        auto alternateAA = [](int i) -> GrQuadAAFlags {
            return (i % 2) ? GrQuadAAFlags::kAll : GrQuadAAFlags::kNone;
        };

        int numExpectedClumps = 2*GrResourceProvider::MaxNumNonAAQuads() /
                                                 GrResourceProvider::MaxNumAAQuads();

        bulk_rect_create_test(reporter, ctxInfo.grContext(), alternateAA, GrAAType::kCoverage,
                              2*GrResourceProvider::MaxNumNonAAQuads(), numExpectedClumps);
    }

    // In this case we have a run of MaxNumAAQuads non-AA quads and then AA quads. This
    // exercises the case where we have a clump of quads that can't be upgraded to AA bc of
    // its size. We expect one clump of non-AA quads followed by one clump of AA quads.
    {
        auto runOfNonAA = [](int i) -> GrQuadAAFlags {
            return (i < GrResourceProvider::MaxNumAAQuads()-1) ? GrQuadAAFlags::kNone
                                                               : GrQuadAAFlags::kAll;
        };

        static const int kNumExpectedClumps = 2;

        bulk_rect_create_test(reporter, ctxInfo.grContext(), runOfNonAA, GrAAType::kCoverage,
                              2*GrResourceProvider::MaxNumAAQuads(), kNumExpectedClumps);
    }
}
