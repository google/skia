/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorSpaceXformImageGenerator.h"

// We will cache and reuse the unique id for this generator.  A better solution is to just
// cache the actual SkImage object.  When this is possible in the Android framework, we should
// delete this code.
struct Entry {
    sk_sp<SkColorSpace> fColorSpace;
    uint32_t            fSrcId;
    uint32_t            fDstId;
};

static constexpr int kNumCachedEntries = 8;
static Entry gIdCache[kNumCachedEntries] = { nullptr, 0, 0 };
static int gNextIndex = 0;

uint32_t find(uint32_t srcId, SkColorSpace* colorSpace) {
    for (int i = 0; i < kNumCachedEntries; i++) {
        if (gIdCache[i].fColorSpace.get() == colorSpace && gIdCache[i].fSrcId == srcId) {
            return gIdCache[i].fDstId;
        }
    }

    return 0;
}

void set(uint32_t srcId, sk_sp<SkColorSpace> colorSpace, uint32_t dstId) {
    gIdCache[gNextIndex].fColorSpace = std::move(colorSpace);
    gIdCache[gNextIndex].fSrcId = srcId;
    gIdCache[gNextIndex].fDstId = dstId;
    gNextIndex = (gNextIndex + 1) % kNumCachedEntries;
}

std::mutex SkColorSpaceXformImageGenerator::fMutex;

std::unique_ptr<SkImageGenerator> SkColorSpaceXformImageGenerator::Make(
        const SkBitmap& src, sk_sp<SkColorSpace> dst, SkCopyPixelsMode mode) {
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

    fMutex.lock();
    uint32_t id = find(srcPtr->getGenerationID(), dst.get());
    fMutex.unlock();

    return std::unique_ptr<SkImageGenerator>(
            new SkColorSpaceXformImageGenerator(*srcPtr, std::move(dst), id));
}

SkColorSpaceXformImageGenerator::SkColorSpaceXformImageGenerator(const SkBitmap& src,
                                                                 sk_sp<SkColorSpace> dst,
                                                                 uint32_t id)
    : INHERITED(src.info().makeColorSpace(dst), id)
    , fSrc(src)
    , fDst(dst)
{
    if (kNeedNewImageUniqueID == id) {
        fMutex.lock();
        set(src.getGenerationID(), dst, this->uniqueID());
        fMutex.unlock();
    }
}

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
            SkBackingFit::kExact, fSrc.width(), fSrc.height(), kRGBA_8888_GrPixelConfig, nullptr);
    if (!renderTargetContext) {
        return nullptr;
    }

    GrPaint paint;
    paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
    paint.addColorTextureProcessor(ctx->resourceProvider(), proxy, nullptr,
            SkMatrix::MakeTrans(origin.fX, origin.fY));
    paint.addColorFragmentProcessor(std::move(xform));

    const SkRect rect = SkRect::MakeWH(info.width(), info.height());
    renderTargetContext->drawRect(GrNoClip(), std::move(paint), GrAA::kNo, SkMatrix::I(), rect);
    return sk_ref_sp(renderTargetContext->asTextureProxy());
}

#endif
