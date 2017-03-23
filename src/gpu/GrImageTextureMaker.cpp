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

static bool cacher_is_alpha_only(const SkImageCacherator& cacher) {
    return kAlpha_8_SkColorType == cacher.info().colorType();
}

GrImageTextureMaker::GrImageTextureMaker(GrContext* context, SkImageCacherator* cacher,
                                         const SkImage* client, SkImage::CachingHint chint)
    : INHERITED(context, cacher->info().width(), cacher->info().height(),
                cacher_is_alpha_only(*cacher))
    , fCacher(cacher)
    , fClient(client)
    , fCachingHint(chint) {
    if (client) {
        GrMakeKeyFromImageID(&fOriginalKey, client->uniqueID(),
                             SkIRect::MakeWH(this->width(), this->height()));
    }
}

sk_sp<GrTextureProxy> GrImageTextureMaker::refOriginalTextureProxy(bool willBeMipped,
                                                                   SkColorSpace* dstColorSpace) {
    return fCacher->lockTextureProxy(this->context(), fOriginalKey, fClient, fCachingHint,
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
    if (fClient) {
        as_IB(fClient)->notifyAddedToCache();
    }
}

SkAlphaType GrImageTextureMaker::alphaType() const {
    return fCacher->info().alphaType();
}
sk_sp<SkColorSpace> GrImageTextureMaker::getColorSpace(SkColorSpace* dstColorSpace) {
    return fCacher->getColorSpace(this->context(), dstColorSpace);
}
