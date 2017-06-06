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
class IdCache {
public:
    struct Entry {
        sk_sp<SkColorSpace> fColorSpace;
        uint32_t            fSrcId;
        uint32_t            fDstId;
    };

    static uint32_t Find(uint32_t srcId, SkColorSpace* colorSpace) {
        fMutex.lock();
        for (int i = 0; i < kNumCachedEntries; i++) {
            if (fIdCache[i].fColorSpace.get() == colorSpace && fIdCache[i].fSrcId == srcId) {
                uint32_t result = fIdCache[i].fDstId;
                fMutex.unlock();
                return result;
            }
        }

        fMutex.unlock();
        return 0;
    }

    static void Set(uint32_t srcId, sk_sp<SkColorSpace> colorSpace, uint32_t dstId) {
        fMutex.lock();
        fIdCache[fNextIndex].fColorSpace = std::move(colorSpace);
        fIdCache[fNextIndex].fSrcId = srcId;
        fIdCache[fNextIndex].fDstId = dstId;
        fNextIndex = (fNextIndex + 1) % kNumCachedEntries;
        fMutex.unlock();
    }

private:
    static std::mutex    fMutex;

    static constexpr int kNumCachedEntries = 8;
    static Entry         fIdCache[kNumCachedEntries];
    static int           fNextIndex;
};

IdCache::Entry IdCache::fIdCache[IdCache::kNumCachedEntries] = { nullptr, 0, 0 };
int IdCache::fNextIndex = 0;
std::mutex IdCache::fMutex;

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

    uint32_t id = IdCache::Find(srcPtr->getGenerationID(), dst.get());
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
        IdCache::Set(src.getGenerationID(), dst, this->uniqueID());
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
