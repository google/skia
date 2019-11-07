/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrClip.h"
#include "src/gpu/GrContextPriv.h"
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

typedef GrQuadAAFlags (*foo)(int i);

static void bulk_rect_create_test(skiatest::Reporter* reporter, GrContext* context,
                                  foo pfFoo, GrAAType overallAA, int requestedTotNumQuads,
                                  int expectedNumOpsInChain) {

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
        set[i].fAAFlags = pfFoo(i);
    }

    std::unique_ptr<GrDrawOp> op = GrTextureOp::MakeSet(context, set, requestedTotNumQuads,
                                                        GrSamplerState::Filter::kNearest,
                                                        GrTextureOp::Saturate::kYes,
                                                        overallAA,
                                                        SkCanvas::kStrict_SrcRectConstraint,
                                                        SkMatrix::I(), nullptr);

    SkASSERT(!op->prevInChain());

    int actualTotNumQuads = 0;
    int actualNumOpsInChain = 0;
    for (GrDrawOp* tmp = op.get(); tmp; tmp = (GrDrawOp*) tmp->nextInChain()) {
        ++actualNumOpsInChain;
        actualTotNumQuads += tmp->numQuads();
    }

    REPORTER_ASSERT(reporter, expectedNumOpsInChain == actualNumOpsInChain);
    REPORTER_ASSERT(reporter, requestedTotNumQuads == actualTotNumQuads);
    delete[] set;
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(BulkRect, reporter, ctxInfo) {

    auto constNone = [](int i) -> GrQuadAAFlags {
        return GrQuadAAFlags::kNone;
    };

    // This is the simple case where there is no AA at all.
    bulk_rect_create_test(reporter, ctxInfo.grContext(), constNone, GrAAType::kNone,
                          2*GrResourceProvider::MaxNumNonAAQuads(), 2);

}
