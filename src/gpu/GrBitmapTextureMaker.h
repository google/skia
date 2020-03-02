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
class GrBitmapTextureMaker final : public GrTextureMaker {
public:
    enum class Cached { kNo, kYes };

    GrBitmapTextureMaker(GrRecordingContext* context, const SkBitmap& bitmap,
                         Cached cached = Cached::kNo, SkBackingFit = SkBackingFit::kExact);

private:
    GrSurfaceProxyView refOriginalTextureProxyView(GrMipMapped) override;

    const SkBitmap     fBitmap;
    const SkBackingFit fFit;
    GrUniqueKey        fKey;

    typedef GrTextureMaker INHERITED;
};

#endif
