/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrImageTextureMaker_DEFINED
#define GrImageTextureMaker_DEFINED

#include "GrTextureMaker.h"
#include "SkImage.h"

class SkImageCacherator;

/** This class manages the conversion of generator-backed images to GrTextures. If the caching hint
    is kAllow the image's ID is used for the cache key. */
class GrImageTextureMaker : public GrTextureMaker {
public:
    GrImageTextureMaker(GrContext* context, const SkImage* client, SkImage::CachingHint chint);

protected:
    // TODO: consider overriding this, for the case where the underlying generator might be
    //       able to efficiently produce a "stretched" texture natively (e.g. picture-backed)
    //          GrTexture* generateTextureForParams(const CopyParams&) override;
    sk_sp<GrTextureProxy> refOriginalTextureProxy(bool willBeMipped,
                                                  SkColorSpace* dstColorSpace,
                                                  AllowedTexGenType onlyIfFast) override;

    void makeCopyKey(const CopyParams& stretch, GrUniqueKey* paramsCopyKey,
                     SkColorSpace* dstColorSpace) override;
    void didCacheCopy(const GrUniqueKey& copyKey) override;

    SkAlphaType alphaType() const override;
    sk_sp<SkColorSpace> getColorSpace(SkColorSpace* dstColorSpace) override;

private:
    SkImageCacherator*      fCacher;
    const SkImage*          fClient;
    GrUniqueKey             fOriginalKey;
    SkImage::CachingHint    fCachingHint;

    typedef GrTextureMaker INHERITED;
};

#endif
