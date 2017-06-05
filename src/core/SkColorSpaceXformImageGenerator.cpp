/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorSpaceXformImageGenerator.h"

std::unique_ptr<SkImageGenerator> SkColorSpaceXformImageGenerator::Make(
        const SkBitmap& src, sk_sp<SkColorSpace> dst, Id id) {
    if (!dst) {
        return nullptr;
    }

    return std::unique_ptr<SkImageGenerator>(
            new SkColorSpaceXformImageGenerator(src, std::move(dst), id));
}

SkColorSpaceXformImageGenerator::SkColorSpaceXformImageGenerator(const SkBitmap& src,
                                                                 sk_sp<SkColorSpace> dst, Id id)
    : INHERITED(src.info().makeColorSpace(dst),
               (Id::kDefault == id ? (uint32_t)kNeedNewImageUniqueID : src.getGenerationID()))
    , fSrc(src)
    , fDst(dst)
{}

bool SkColorSpaceXformImageGenerator::onGetPixels(const SkImageInfo& info, void* pixels,
                                                  size_t rowBytes, const Options& opts) {
    return fSrc.readPixels(info, pixels, rowBytes, 0, 0, opts.fBehavior);
}

#if SK_SUPPORT_GPU

#include "GrClip.h"
#include "GrContext.h"
#include "GrPaint.h"
#include "GrRenderTargetContext.h"
#include "GrTextureProxy.h"
#include "SkGr.h"
#include "effects/GrNonlinearColorSpaceXformEffect.h"

sk_sp<GrTextureProxy> SkColorSpaceXformImageGenerator::onGenerateTexture(GrContext* ctx,
                                                                         const SkImageInfo& info,
                                                                         const SkIPoint& origin) {
    // FIXME:
    // This always operates as if SkTranferFunctionBehavior is kIgnore.  Should we add
    // options so that caller can also request kRespect?

    SkASSERT(ctx);

    sk_sp<GrTextureProxy> proxy =
            GrUploadBitmapToTextureProxy(ctx->resourceProvider(), fSrc, nullptr);

    sk_sp<SkColorSpace> srcSpace =
            fSrc.colorSpace() ? sk_ref_sp(fSrc.colorSpace()) : SkColorSpace::MakeSRGB();
    auto xform = GrNonlinearColorSpaceXformEffect::Make(srcSpace.get(), fDst.get());
    if (!xform) {
        return nullptr;
    }

    sk_sp<GrRenderTargetContext> renderTargetContext = ctx->makeDeferredRenderTargetContext(
            SkBackingFit::kExact, info.width(), info.height(), kRGBA_8888_GrPixelConfig, nullptr);
    if (!renderTargetContext) {
        return nullptr;
    }

    GrPaint paint;
    paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
    paint.addColorTextureProcessor(ctx->resourceProvider(), proxy, nullptr, SkMatrix::I());
    paint.addColorFragmentProcessor(std::move(xform));

    const SkRect rect = SkRect::MakeXYWH(origin.fX, origin.fY, info.width(), info.height());
    renderTargetContext->drawRect(GrNoClip(), std::move(paint), GrAA::kNo, SkMatrix::I(), rect);
    return sk_ref_sp(renderTargetContext->asTextureProxy());
}

#endif
