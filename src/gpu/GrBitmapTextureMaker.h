/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBitmapTextureMaker_DEFINED
#define GrBitmapTextureMaker_DEFINED

#include "include/core/SkBitmap.h"
#include "src/gpu/GrTextureMaker.h"

/** This class manages the conversion of SW-backed bitmaps to GrTextures. If the input bitmap is
    non-volatile the texture is cached using a key created from the pixels' image id and the
    subset of the pixelref specified by the bitmap. */
class GrBitmapTextureMaker : public GrTextureMaker {
public:
    GrBitmapTextureMaker(GrRecordingContext* context, const SkBitmap& bitmap,
                         bool useDecal = false);

protected:
    sk_sp<GrTextureProxy> refOriginalTextureProxy(bool willBeMipped,
                                                  AllowedTexGenType onlyIfFast) override;

    void makeCopyKey(const CopyParams& copyParams, GrUniqueKey* copyKey) override;
    void didCacheCopy(const GrUniqueKey& copyKey, uint32_t contextUniqueID) override;

    SkAlphaType alphaType() const override;
    SkColorSpace* colorSpace() const override;

private:
    const SkBitmap  fBitmap;
    GrUniqueKey     fOriginalKey;

    typedef GrTextureMaker INHERITED;
};

#endif
