/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkSize.h"
#include "include/core/SkSurfaceProps.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/gpu/GrTypes.h"
#include "include/private/SkColorData.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/Swizzle.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrColorSpaceXform.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrOpsTypes.h"
#include "src/gpu/ganesh/GrPaint.h"
#include "src/gpu/ganesh/GrProxyProvider.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrResourceProvider.h"
#include "src/gpu/ganesh/GrSamplerState.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"
#include "src/gpu/ganesh/GrXferProcessor.h"
#include "src/gpu/ganesh/SurfaceDrawContext.h"
#include "src/gpu/ganesh/geometry/GrQuad.h"
#include "src/gpu/ganesh/ops/FillRectOp.h"
#include "src/gpu/ganesh/ops/GrDrawOp.h"
#include "src/gpu/ganesh/ops/GrOp.h"
#include "src/gpu/ganesh/ops/OpsTask.h"
#include "src/gpu/ganesh/ops/TextureOp.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"

#include <cstdint>
#include <memory>
#include <utility>

struct GrContextOptions;

using namespace skgpu::ganesh;

static std::unique_ptr<skgpu::ganesh::SurfaceDrawContext> new_SDC(GrRecordingContext* rContext) {
    return skgpu::ganesh::SurfaceDrawContext::Make(rContext,
                                                   GrColorType::kRGBA_8888,
                                                   nullptr,
                                                   SkBackingFit::kExact,
                                                   {128, 128},
                                                   SkSurfaceProps(),
                                                   /*label=*/{});
}

static sk_sp<GrSurfaceProxy> create_proxy(GrRecordingContext* rContext) {
    static constexpr SkISize kDimensions = {128, 128};

    const GrBackendFormat format = rContext->priv().caps()->getDefaultBackendFormat(
                                                                           GrColorType::kRGBA_8888,
                                                                           GrRenderable::kYes);
    return rContext->priv().proxyProvider()->createProxy(format,
                                                         kDimensions,
                                                         GrRenderable::kYes,
                                                         1,
                                                         skgpu::Mipmapped::kNo,
                                                         SkBackingFit::kExact,
                                                         skgpu::Budgeted::kNo,
                                                         GrProtected::kNo,
                                                         /*label=*/"CreateSurfaceProxy",
                                                         GrInternalSurfaceFlags::kNone);
}

typedef GrQuadAAFlags (*PerQuadAAFunc)(int i);

typedef void (*BulkRectTest)(skiatest::Reporter*,
                             GrDirectContext*,
                             PerQuadAAFunc,
                             GrAAType overallAA,
                             SkBlendMode,
                             bool addOneByOne,
                             bool allUniqueProxies,
                             int requestedTotNumQuads,
                             int expectedNumOps);

//-------------------------------------------------------------------------------------------------
static void fillrectop_creation_test(skiatest::Reporter* reporter, GrDirectContext* dContext,
                                     PerQuadAAFunc perQuadAA, GrAAType overallAA,
                                     SkBlendMode blendMode, bool addOneByOne,
                                     bool allUniqueProxies,
                                     int requestedTotNumQuads, int expectedNumOps) {

    if (addOneByOne || allUniqueProxies) {
        return;
    }

    std::unique_ptr<skgpu::ganesh::SurfaceDrawContext> sdc = new_SDC(dContext);

    auto quads = new GrQuadSetEntry[requestedTotNumQuads];

    for (int i = 0; i < requestedTotNumQuads; ++i) {
        quads[i].fRect = SkRect::MakeWH(100.5f, 100.5f); // prevent the int non-AA optimization
        quads[i].fColor = SK_PMColor4fWHITE;
        quads[i].fLocalMatrix = SkMatrix::I();
        quads[i].fAAFlags = perQuadAA(i);
    }

    GrPaint paint;
    paint.setXPFactory(GrXPFactory::FromBlendMode(blendMode));

    skgpu::ganesh::FillRectOp::AddFillRectOps(sdc.get(),
                                              nullptr,
                                              dContext,
                                              std::move(paint),
                                              overallAA,
                                              SkMatrix::I(),
                                              quads,
                                              requestedTotNumQuads);

    auto opsTask = sdc->testingOnly_PeekLastOpsTask();
    int actualNumOps = opsTask->numOpChains();

    int actualTotNumQuads = 0;

    for (int i = 0; i < actualNumOps; ++i) {
        const GrOp* tmp = opsTask->getChain(i);
        REPORTER_ASSERT(reporter, tmp->classID() == skgpu::ganesh::FillRectOp::ClassID());
        REPORTER_ASSERT(reporter, tmp->isChainTail());
        actualTotNumQuads += ((const GrDrawOp*) tmp)->numQuads();
    }

    REPORTER_ASSERT(reporter, expectedNumOps == actualNumOps);
    REPORTER_ASSERT(reporter, requestedTotNumQuads == actualTotNumQuads);

    dContext->flushAndSubmit();

    delete[] quads;
}

//-------------------------------------------------------------------------------------------------
static void textureop_creation_test(skiatest::Reporter* reporter, GrDirectContext* dContext,
                                    PerQuadAAFunc perQuadAA, GrAAType overallAA,
                                    SkBlendMode blendMode, bool addOneByOne,
                                    bool allUniqueProxies,
                                    int requestedTotNumQuads, int expectedNumOps) {
    std::unique_ptr<skgpu::ganesh::SurfaceDrawContext> sdc = new_SDC(dContext);

    GrSurfaceProxyView proxyViewA, proxyViewB;

    if (!allUniqueProxies) {
        sk_sp<GrSurfaceProxy> proxyA = create_proxy(dContext);
        sk_sp<GrSurfaceProxy> proxyB = create_proxy(dContext);
        proxyViewA = GrSurfaceProxyView(std::move(proxyA),
                                        kTopLeft_GrSurfaceOrigin,
                                        skgpu::Swizzle::RGBA());
        proxyViewB = GrSurfaceProxyView(std::move(proxyB),
                                        kTopLeft_GrSurfaceOrigin,
                                        skgpu::Swizzle::RGBA());
    }

    auto set = new GrTextureSetEntry[requestedTotNumQuads];

    for (int i = 0; i < requestedTotNumQuads; ++i) {
        if (!allUniqueProxies) {
            // Alternate between two proxies to prevent op merging if the batch API was forced to
            // submit one op at a time (to work, this does require that all fDstRects overlap).
            set[i].fProxyView = i % 2 == 0 ? proxyViewA : proxyViewB;
        } else {
            // Each op gets its own proxy to force chaining only
            sk_sp<GrSurfaceProxy> proxyA = create_proxy(dContext);
            set[i].fProxyView = GrSurfaceProxyView(std::move(proxyA),
                                                   kTopLeft_GrSurfaceOrigin,
                                                   skgpu::Swizzle::RGBA());
        }

        set[i].fSrcAlphaType = kPremul_SkAlphaType;
        set[i].fSrcRect = SkRect::MakeWH(100.0f, 100.0f);
        set[i].fDstRect = SkRect::MakeWH(100.5f, 100.5f); // prevent the int non-AA optimization
        set[i].fDstClipQuad = nullptr;
        set[i].fPreViewMatrix = nullptr;
        set[i].fColor = {1.f, 1.f, 1.f, 1.f};
        set[i].fAAFlags = perQuadAA(i);
    }

    if (addOneByOne) {
        for (int i = 0; i < requestedTotNumQuads; ++i) {
            DrawQuad quad;

            quad.fDevice = GrQuad::MakeFromRect(set[i].fDstRect,  SkMatrix::I());
            quad.fLocal = GrQuad(set[i].fSrcRect);
            quad.fEdgeFlags = set[i].fAAFlags;

            GrOp::Owner op = TextureOp::Make(dContext,
                                             set[i].fProxyView,
                                             set[i].fSrcAlphaType,
                                             nullptr,
                                             GrSamplerState::Filter::kNearest,
                                             GrSamplerState::MipmapMode::kNone,
                                             set[i].fColor,
                                             TextureOp::Saturate::kYes,
                                             blendMode,
                                             overallAA,
                                             &quad,
                                             nullptr);
            sdc->addDrawOp(nullptr, std::move(op));
        }
    } else {
        TextureOp::AddTextureSetOps(sdc.get(),
                                    nullptr,
                                    dContext,
                                    set,
                                    requestedTotNumQuads,
                                    requestedTotNumQuads,  // We alternate so proxyCnt == cnt
                                    GrSamplerState::Filter::kNearest,
                                    GrSamplerState::MipmapMode::kNone,
                                    TextureOp::Saturate::kYes,
                                    blendMode,
                                    overallAA,
                                    SkCanvas::kStrict_SrcRectConstraint,
                                    SkMatrix::I(),
                                    nullptr);
    }

    auto opsTask = sdc->testingOnly_PeekLastOpsTask();
    int actualNumOps = opsTask->numOpChains();

    int actualTotNumQuads = 0;

    if (blendMode != SkBlendMode::kSrcOver ||
        !dContext->priv().caps()->dynamicStateArrayGeometryProcessorTextureSupport()) {
        // In either of these two cases, TextureOp creates one op per quad instead. Since
        // each entry alternates proxies but overlaps geometrically, this will prevent the ops
        // from being merged back into fewer ops.
        expectedNumOps = requestedTotNumQuads;
    }
    uint32_t expectedOpID = blendMode == SkBlendMode::kSrcOver
                                    ? TextureOp::ClassID()
                                    : skgpu::ganesh::FillRectOp::ClassID();
    for (int i = 0; i < actualNumOps; ++i) {
        const GrOp* tmp = opsTask->getChain(i);
        REPORTER_ASSERT(reporter, allUniqueProxies || tmp->isChainTail());
        while (tmp) {
            REPORTER_ASSERT(reporter, tmp->classID() == expectedOpID);
            actualTotNumQuads += ((const GrDrawOp*) tmp)->numQuads();
            tmp = tmp->nextInChain();
        }
    }

    REPORTER_ASSERT(reporter, expectedNumOps == actualNumOps);
    REPORTER_ASSERT(reporter, requestedTotNumQuads == actualTotNumQuads);

    dContext->flushAndSubmit();

    delete[] set;
}

//-------------------------------------------------------------------------------------------------
static void run_test(GrDirectContext* dContext, skiatest::Reporter* reporter, BulkRectTest test) {

    // This is the simple case where there is no AA at all. We expect 2 non-AA clumps of quads.
    {
        auto noAA = [](int i) -> GrQuadAAFlags {
            return GrQuadAAFlags::kNone;
        };

        static const int kNumExpectedOps = 2;

        test(reporter, dContext, noAA, GrAAType::kNone, SkBlendMode::kSrcOver,
             false, false, 2*GrResourceProvider::MaxNumNonAAQuads(), kNumExpectedOps);
    }

    // This is the same as the above case except the overall AA is kCoverage. However, since
    // the per-quad AA is still none, all the quads should be downgraded to non-AA.
    {
        auto noAA = [](int i) -> GrQuadAAFlags {
            return GrQuadAAFlags::kNone;
        };

        static const int kNumExpectedOps = 2;

        test(reporter, dContext, noAA, GrAAType::kCoverage, SkBlendMode::kSrcOver,
             false, false, 2*GrResourceProvider::MaxNumNonAAQuads(), kNumExpectedOps);
    }

    // This case has an overall AA of kCoverage but the per-quad AA alternates.
    // We should end up with several aa-sized clumps
    {
        auto alternateAA = [](int i) -> GrQuadAAFlags {
            return (i % 2) ? GrQuadAAFlags::kAll : GrQuadAAFlags::kNone;
        };

        int numExpectedOps = 2*GrResourceProvider::MaxNumNonAAQuads() /
                                                 GrResourceProvider::MaxNumAAQuads();

        test(reporter, dContext, alternateAA, GrAAType::kCoverage, SkBlendMode::kSrcOver,
             false, false, 2*GrResourceProvider::MaxNumNonAAQuads(), numExpectedOps);
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

        test(reporter, dContext, runOfNonAA, GrAAType::kCoverage, SkBlendMode::kSrcOver,
             false, false, 2*GrResourceProvider::MaxNumAAQuads(), kNumExpectedOps);
    }

    // In this case we use a blend mode other than src-over, which hits the FillRectOp fallback
    // code path for TextureOp. We pass in the expected results if batching was successful, to
    // that bulk_fill_rect_create_test batches on all modes; bulk_texture_rect_create_test is
    // responsible for revising its expectations.
    {
        auto fixedAA = [](int i) -> GrQuadAAFlags {
            return GrQuadAAFlags::kAll;
        };

        static const int kNumExpectedOps = 2;

        test(reporter, dContext, fixedAA, GrAAType::kCoverage, SkBlendMode::kSrcATop,
             false, false, 2*GrResourceProvider::MaxNumAAQuads(), kNumExpectedOps);
    }

    // This repros crbug.com/1108475, where we create 1024 non-AA texture ops w/ one coverage-AA
    // texture op in the middle. Because each op has its own texture, all the texture ops
    // get chained together so the quad count can exceed the AA maximum.
    {
        auto onlyOneAA = [](int i) -> GrQuadAAFlags {
            return i == 256 ? GrQuadAAFlags::kAll : GrQuadAAFlags::kNone;
        };

        static const int kNumExpectedOps = 3;

        test(reporter, dContext, onlyOneAA, GrAAType::kCoverage, SkBlendMode::kSrcOver,
             true, true, 1024, kNumExpectedOps);
    }

    // This repros a problem related to crbug.com/1108475. In this case, the bulk creation
    // method had no way to break up the set of texture ops at the AA quad limit.
    {
        auto onlyOneAA = [](int i) -> GrQuadAAFlags {
            return i == 256 ? GrQuadAAFlags::kAll : GrQuadAAFlags::kNone;
        };

        static const int kNumExpectedOps = 2;

        test(reporter, dContext, onlyOneAA, GrAAType::kCoverage, SkBlendMode::kSrcOver,
             false, true, 1024, kNumExpectedOps);
    }

}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(BulkFillRectTest,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    run_test(ctxInfo.directContext(), reporter, fillrectop_creation_test);
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(BulkTextureRectTest,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    run_test(ctxInfo.directContext(), reporter, textureop_creation_test);
}
