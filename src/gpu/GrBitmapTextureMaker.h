/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBitmapTextureMaker_DEFINED
#define GrBitmapTextureMaker_DEFINED

#include "GrTextureParamsAdjuster.h"

/** This class manages the conversion of SW-backed bitmaps to GrTextures. If the input bitmap is
    non-volatile the texture is cached using a key created from the pixels' image id and the
    subset of the pixelref specified by the bitmap. */
class GrBitmapTextureMaker : public GrTextureMaker {
public:
    GrBitmapTextureMaker(GrContext* context, const SkBitmap& bitmap);

protected:
    GrTexture* refOriginalTexture(bool willBeMipped, SkDestinationSurfaceColorMode) override;

    void makeCopyKey(const CopyParams& copyParams, GrUniqueKey* copyKey,
                     SkDestinationSurfaceColorMode colorMode) override;

    void didCacheCopy(const GrUniqueKey& copyKey) override;

    SkAlphaType alphaType() const override;
    sk_sp<SkColorSpace> getColorSpace(SkDestinationSurfaceColorMode) override;

private:
    const SkBitmap  fBitmap;
    GrUniqueKey     fOriginalKey;

    typedef GrTextureMaker INHERITED;
};

#endif
