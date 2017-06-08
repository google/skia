/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBitmapTextureMaker_DEFINED
#define GrBitmapTextureMaker_DEFINED

#include "GrTextureMaker.h"

/** This class manages the conversion of SW-backed bitmaps to GrTextures. If the input bitmap is
    non-volatile the texture is cached using a key created from the pixels' image id and the
    subset of the pixelref specified by the bitmap. */
class GrBitmapTextureMaker : public GrTextureMaker {
public:
    GrBitmapTextureMaker(GrContext* context, const SkBitmap& bitmap);

protected:
    sk_sp<GrTextureProxy> refOriginalTextureProxy(bool willBeMipped,
                                                  SkColorSpace* dstColorSpace,
                                                  AllowedTexGenType onlyIfFast) override;

    void makeCopyKey(const CopyParams& copyParams, GrUniqueKey* copyKey,
                     SkColorSpace* dstColorSpace) override;

    void didCacheCopy(const GrUniqueKey& copyKey) override;

    SkAlphaType alphaType() const override;
    sk_sp<SkColorSpace> getColorSpace(SkColorSpace* dstColorSpace) override;

private:
    const SkBitmap  fBitmap;
    GrUniqueKey     fOriginalKey;

    typedef GrTextureMaker INHERITED;
};

#endif
