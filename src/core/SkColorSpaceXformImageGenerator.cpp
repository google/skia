/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorSpaceXformImageGenerator.h"

std::unique_ptr<SkImageGenerator> SkColorSpaceXformImageGenerator::Make(
        const SkBitmap& src, sk_sp<SkColorSpace> dst, SkCopyPixelsMode mode) {
    return SkColorSpaceXformImageGenerator::Make(src, dst, mode, kNeedNewImageUniqueID);
}

std::unique_ptr<SkImageGenerator> SkColorSpaceXformImageGenerator::Make(
        const SkBitmap& src, sk_sp<SkColorSpace> dst, SkCopyPixelsMode mode, uint32_t id) {
    if (!dst) {
        return nullptr;
    }

    const SkBitmap* srcPtr = &src;
    SkBitmap copy;
    if (kAlways_SkCopyPixelsMode == mode ||
            (kNever_SkCopyPixelsMode != mode && !src.isImmutable())) {
        if (!copy.tryAllocPixels(src.info())) {
            return nullptr;
        }

        SkAssertResult(src.readPixels(copy.info(), copy.getPixels(), copy.rowBytes(), 0, 0));
        copy.setImmutable();
        srcPtr = &copy;
    }

    return std::unique_ptr<SkImageGenerator>(
            new SkColorSpaceXformImageGenerator(*srcPtr, std::move(dst), id));
}

SkColorSpaceXformImageGenerator::SkColorSpaceXformImageGenerator(const SkBitmap& src,
                                                                 sk_sp<SkColorSpace> dst,
                                                                 uint32_t id)
    : INHERITED(src.info().makeColorSpace(dst), id)
    , fSrc(src)
    , fDst(std::move(dst))
{}

bool SkColorSpaceXformImageGenerator::onGetPixels(const SkImageInfo& info, void* pixels,
                                                  size_t rowBytes, const Options& opts) {
    SkImageInfo dstInfo = info;
    if (!info.colorSpace()) {
        dstInfo = dstInfo.makeColorSpace(fDst);
    }
    return fSrc.readPixels(dstInfo, pixels, rowBytes, 0, 0, opts.fBehavior);
}

#if SK_SUPPORT_GPU

#include "GrClip.h"
#include "GrContext.h"
#include "GrPaint.h"
#include "GrRenderTargetContext.h"
#include "GrTextureProxy.h"
#include "SkGr.h"
#include "effects/GrNonlinearColorSpaceXformEffect.h"

sk_sp<GrTextureProxy> SkColorSpaceXformImageGenerator::onGenerateTexture(
        GrContext* ctx, const SkImageInfo& info, const SkIPoint& origin,
        SkTransferFunctionBehavior) {
    // FIXME:
    // This always operates as if SkTranferFunctionBehavior is kIgnore.  Should we add
    // options so that caller can also request kRespect?

    SkASSERT(ctx);

    sk_sp<GrTextureProxy> proxy = GrUploadBitmapToTextureProxy(ctx->resourceProvider(),
                                                               fSrc, nullptr);

    if (!proxy) {
        return nullptr;
    }

    sk_sp<SkColorSpace> srcSpace =
            fSrc.colorSpace() ? sk_ref_sp(fSrc.colorSpace()) : SkColorSpace::MakeSRGB();
    auto xform = GrNonlinearColorSpaceXformEffect::Make(srcSpace.get(), fDst.get());
    if (!xform) {
        return nullptr;
    }

    sk_sp<GrRenderTargetContext> renderTargetContext = ctx->makeDeferredRenderTargetContext(
            SkBackingFit::kExact, fSrc.width(), fSrc.height(), kRGBA_8888_GrPixelConfig, nullptr);
    if (!renderTargetContext) {
        return nullptr;
    }

    GrPaint paint;
    paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
    paint.addColorTextureProcessor(std::move(proxy), nullptr,
                                   SkMatrix::MakeTrans(origin.fX, origin.fY));
    paint.addColorFragmentProcessor(std::move(xform));

    const SkRect rect = SkRect::MakeWH(info.width(), info.height());
    renderTargetContext->drawRect(GrNoClip(), std::move(paint), GrAA::kNo, SkMatrix::I(), rect);
    return sk_ref_sp(renderTargetContext->asTextureProxy());
}

#endif
