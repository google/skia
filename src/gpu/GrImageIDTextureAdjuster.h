/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrImageIDTextureAdjuster_DEFINED
#define GrImageIDTextureAdjuster_DEFINED

#include "GrTextureParamsAdjuster.h"
#include "SkImage.h"

class SkBitmap;
class SkImage_Base;
class SkImageCacherator;

/** Implementation for texture-backed SkBitmaps. The bitmap must stay in scope and unmodified
    while this object exists. */
class GrBitmapTextureAdjuster : public GrTextureAdjuster {
public:
    explicit GrBitmapTextureAdjuster(const SkBitmap* bmp);

private:
    void makeCopyKey(const CopyParams& params, GrUniqueKey* copyKey) override;

    void didCacheCopy(const GrUniqueKey& copyKey) override;

    const SkBitmap* fBmp;

    typedef GrTextureAdjuster INHERITED;
};

/** Implementation for texture-backed SkImages. The image must stay in scope and unmodified while
    this object exists. */
class GrImageTextureAdjuster : public GrTextureAdjuster {
public:
    explicit GrImageTextureAdjuster(const SkImage_Base* img);

private:
    void makeCopyKey(const CopyParams& params, GrUniqueKey* copyKey) override;

    void didCacheCopy(const GrUniqueKey& copyKey) override;

    const SkImage_Base* fImageBase;

    typedef GrTextureAdjuster INHERITED;
};

/** This class manages the conversion of SW-backed bitmaps to GrTextures. If the input bitmap is
    non-volatile the texture is cached using a key created from the pixels' image id and the
    subset of the pixelref specified by the bitmap. */
class GrBitmapTextureMaker : public GrTextureMaker {
public:
    GrBitmapTextureMaker(GrContext* context, const SkBitmap& bitmap);

protected:
    GrTexture* refOriginalTexture(bool willBeMipped, SkSourceGammaTreatment) override;

    void makeCopyKey(const CopyParams& copyParams, GrUniqueKey* copyKey) override;

    void didCacheCopy(const GrUniqueKey& copyKey) override;

private:
    const SkBitmap  fBitmap;
    GrUniqueKey     fOriginalKey;

    typedef GrTextureMaker INHERITED;
};

/** This class manages the conversion of generator-backed images to GrTextures. If the caching hint
    is kAllow the image's ID is used for the cache key. */
class GrImageTextureMaker : public GrTextureMaker {
public:
    GrImageTextureMaker(GrContext* context, SkImageCacherator* cacher, const SkImage* client,
                        SkImage::CachingHint chint);

protected:
    // TODO: consider overriding this, for the case where the underlying generator might be
    //       able to efficiently produce a "stretched" texture natively (e.g. picture-backed)
    //          GrTexture* generateTextureForParams(const CopyParams&) override;

    GrTexture* refOriginalTexture(bool willBeMipped, SkSourceGammaTreatment) override;
    void makeCopyKey(const CopyParams& stretch, GrUniqueKey* paramsCopyKey) override;
    void didCacheCopy(const GrUniqueKey& copyKey) override;

private:
    SkImageCacherator*      fCacher;
    const SkImage*          fClient;
    GrUniqueKey             fOriginalKey;
    SkImage::CachingHint    fCachingHint;

    typedef GrTextureMaker INHERITED;
};

#endif
