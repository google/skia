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

static void bulk_rect_create_test(skiatest::Reporter* reporter, GrContext* context) {

    sk_sp<GrSurfaceProxy> proxy = create_proxy(context);

    GrSurfaceProxyView proxyView(std::move(proxy), kTopLeft_GrSurfaceOrigin, GrSwizzle::RGBA());

    static const int kCount = 256;
    GrRenderTargetContext::TextureSetEntry set[kCount];

    for (int i = 0; i < kCount; ++i) {
        set[i].fProxyView = proxyView;
        set[i].fSrcColorType = GrColorType::kRGBA_8888;
        set[i].fSrcRect = SkRect::MakeWH(100, 100);
        set[i].fDstRect = SkRect::MakeWH(100, 100);
        set[i].fDstClipQuad = nullptr;
        set[i].fPreViewMatrix = nullptr;
        set[i].fAlpha = 1.0f;
        set[i].fAAFlags = GrQuadAAFlags::kNone;
    }

    std::unique_ptr<GrDrawOp> op = GrTextureOp::MakeSet(context, set, kCount,
                                                        GrSamplerState::Filter::kNearest,
                                                        GrTextureOp::Saturate::kYes,
                                                        GrAAType::kNone,
                                                        SkCanvas::kStrict_SrcRectConstraint,
                                                        SkMatrix::I(), nullptr);

}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(BulkRect, reporter, ctxInfo) {
    bulk_rect_create_test(reporter, ctxInfo.grContext());

}
