/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImageCacherator_DEFINED
#define SkImageCacherator_DEFINED

#include "SkImage.h"
#include "SkImageInfo.h"

#if SK_SUPPORT_GPU
#include "GrTextureMaker.h"
#endif

class GrCaps;
class GrContext;
class GrTextureProxy;
class GrUniqueKey;
class SkColorSpace;

/*
 *  Interface used by GrImageTextureMaker to construct textures from instances of SkImage
 *  (specifically, SkImage_Lazy).
 */
class SkImageCacherator {
public:
    virtual ~SkImageCacherator() {}

    enum CachedFormat {
        kLegacy_CachedFormat,    // The format from the generator, with any color space stripped out
        kLinearF16_CachedFormat, // Half float RGBA with linear gamma
        kSRGB8888_CachedFormat,  // sRGB bytes
        kSBGR8888_CachedFormat,  // sRGB bytes, in BGR order

        kNumCachedFormats,
    };

    virtual CachedFormat chooseCacheFormat(SkColorSpace* dstColorSpace,
                                           const GrCaps* = nullptr) const = 0;
    virtual SkImageInfo buildCacheInfo(CachedFormat) const = 0;

#if SK_SUPPORT_GPU
    // Returns the texture proxy. If the cacherator is generating the texture and wants to cache it,
    // it should use the passed in key (if the key is valid).
    // If "genType" argument equals AllowedTexGenType::kCheap and the texture is not trivial to
    // construct then refOriginalTextureProxy should return nullptr (for example if texture is made
    // by drawing into a render target).
    virtual sk_sp<GrTextureProxy> lockTextureProxy(GrContext*, const GrUniqueKey& key,
                                                   SkImage::CachingHint, bool willBeMipped,
                                                   SkColorSpace* dstColorSpace,
                                                   GrTextureMaker::AllowedTexGenType genType) = 0;

    // Returns the color space of the texture that would be returned if you called lockTexture.
    // Separate code path to allow querying of the color space for textures that cached (even
    // externally).
    virtual sk_sp<SkColorSpace> getColorSpace(GrContext*, SkColorSpace* dstColorSpace) = 0;
    virtual void makeCacheKeyFromOrigKey(const GrUniqueKey& origKey, CachedFormat,
                                         GrUniqueKey* cacheKey) = 0;
#endif
};

#endif
