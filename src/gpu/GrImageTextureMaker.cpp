/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrImageTextureMaker.h"
#include "SkGr.h"
#include "SkImage_Lazy.h"

GrImageTextureMaker::GrImageTextureMaker(GrContext* context, const SkImage* client,
                                         SkImage::CachingHint chint)
        : INHERITED(context, client->width(), client->height(), client->isAlphaOnly())
        , fImage(static_cast<const SkImage_Lazy*>(client))
        , fCachingHint(chint) {
    SkASSERT(client->isLazyGenerated());
    GrMakeKeyFromImageID(&fOriginalKey, client->uniqueID(),
                         SkIRect::MakeWH(this->width(), this->height()));
}

sk_sp<GrTextureProxy> GrImageTextureMaker::refOriginalTextureProxy(bool willBeMipped,
                                                                   SkColorSpace* dstColorSpace,
                                                                   AllowedTexGenType onlyIfFast) {
    return fImage->lockTextureProxy(this->context(), fOriginalKey, fCachingHint,
                                    willBeMipped, dstColorSpace, onlyIfFast);
}

void GrImageTextureMaker::makeCopyKey(const CopyParams& stretch, GrUniqueKey* paramsCopyKey) {
    if (fOriginalKey.isValid() && SkImage::kAllow_CachingHint == fCachingHint) {
        GrUniqueKey cacheKey;
        fImage->makeCacheKeyFromOrigKey(fOriginalKey, &cacheKey);
        MakeCopyKeyFromOrigKey(cacheKey, stretch, paramsCopyKey);
    }
}

SkAlphaType GrImageTextureMaker::alphaType() const {
    return fImage->alphaType();
}
sk_sp<SkColorSpace> GrImageTextureMaker::getColorSpace(SkColorSpace* dstColorSpace) {
    return fImage->refColorSpace();
}
