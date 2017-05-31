/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrImageTextureMaker.h"

#include "GrContext.h"
#include "GrGpuResourcePriv.h"
#include "SkGr.h"
#include "SkImage_Base.h"
#include "SkImageCacherator.h"
#include "SkPixelRef.h"
#include "GrGpu.h"
#include "GrResourceProvider.h"

GrImageTextureMaker::GrImageTextureMaker(GrContext* context, const SkImage* client,
                                         SkImage::CachingHint chint)
        : INHERITED(context, client->width(), client->height(), client->isAlphaOnly())
        , fCacher(as_IB(client)->peekCacherator())
        , fClient(client)
        , fCachingHint(chint) {
    SkASSERT(fCacher);
    GrMakeKeyFromImageID(&fOriginalKey, client->uniqueID(),
                         SkIRect::MakeWH(this->width(), this->height()));
}

sk_sp<GrTextureProxy> GrImageTextureMaker::refOriginalTextureProxy(bool willBeMipped,
                                                                   SkColorSpace* dstColorSpace) {
    return fCacher->lockTextureProxy(this->context(), fOriginalKey, fCachingHint,
                                     willBeMipped, dstColorSpace);
}

void GrImageTextureMaker::makeCopyKey(const CopyParams& stretch, GrUniqueKey* paramsCopyKey,
                                      SkColorSpace* dstColorSpace) {
    if (fOriginalKey.isValid() && SkImage::kAllow_CachingHint == fCachingHint) {
        SkImageCacherator::CachedFormat cacheFormat =
            fCacher->chooseCacheFormat(dstColorSpace, this->context()->caps());
        GrUniqueKey cacheKey;
        fCacher->makeCacheKeyFromOrigKey(fOriginalKey, cacheFormat, &cacheKey);
        MakeCopyKeyFromOrigKey(cacheKey, stretch, paramsCopyKey);
    }
}

void GrImageTextureMaker::didCacheCopy(const GrUniqueKey& copyKey) {
    as_IB(fClient)->notifyAddedToCache();
}

SkAlphaType GrImageTextureMaker::alphaType() const {
    return fClient->alphaType();
}
sk_sp<SkColorSpace> GrImageTextureMaker::getColorSpace(SkColorSpace* dstColorSpace) {
    return fCacher->getColorSpace(this->context(), dstColorSpace);
}

sk_sp<GrTextureProxy> GrImageTextureMaker::refTextureProxyForParams(
                                                               const GrSamplerParams& params,
                                                               SkColorSpace* dstColorSpace,
                                                               sk_sp<SkColorSpace>* texColorSpace,
                                                               SkScalar scaleAdjust[2]) {
    GrContext*  context = this->context();
    CopyParams copyParams;
    bool willBeMipped = params.filterMode() == GrSamplerParams::kMipMap_FilterMode;

    if (!context->caps()->mipMapSupport()) {
        willBeMipped = false;
    }

    if (texColorSpace) {
        *texColorSpace = this->getColorSpace(dstColorSpace);
    }

    sk_sp<GrTextureProxy> original(this->refOriginalTextureProxy(willBeMipped, dstColorSpace));
    if (!original) {
        return nullptr;
    }

    if (!context->getGpu()->isACopyNeededForTextureParams(original.get(), params, &copyParams,
                                                          scaleAdjust)) {
        return original;
    }
    GrUniqueKey copyKey;
    this->makeCopyKey(copyParams, &copyKey, dstColorSpace);
    if (copyKey.isValid()) {
        sk_sp<GrTextureProxy> result(context->resourceProvider()->findProxyByUniqueKey(copyKey));
        if (result) {
            return result;
        }
    }

    sk_sp<GrTextureProxy> result(CopyOnGpu(context, std::move(original), nullptr, copyParams));
    if (!result) {
        return nullptr;
    }

    if (copyKey.isValid()) {
        context->resourceProvider()->assignUniqueKeyToProxy(copyKey, result.get());
        this->didCacheCopy(copyKey);
    }

    return result;
}
