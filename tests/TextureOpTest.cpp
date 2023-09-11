/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkSize.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/gpu/GrTypes.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/base/SkArenaAlloc.h"
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
#include "src/gpu/ganesh/geometry/GrQuad.h"
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
