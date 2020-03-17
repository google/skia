/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrImageTextureMaker_DEFINED
#define GrImageTextureMaker_DEFINED

#include "include/core/SkImage.h"
#include "src/gpu/GrTextureMaker.h"
#include "src/gpu/SkGr.h"

class SkImage_Lazy;
class SkImage_GpuYUVA;

/** This class manages the conversion of generator-backed images to GrTextures. If the caching hint
    is kAllow the image's ID is used for the cache key. */
class GrImageTextureMaker final : public GrTextureMaker {
public:
    GrImageTextureMaker(GrRecordingContext*, const SkImage* client, GrImageTexGenPolicy);

private:
    GrSurfaceProxyView refOriginalTextureProxyView(GrMipMapped) override;

    const SkImage_Lazy*     fImage;
    GrImageTexGenPolicy     fTexGenPolicy;

    typedef GrTextureMaker INHERITED;
};

/** This class manages the conversion of generator-backed YUVA images to GrTextures. */
class GrYUVAImageTextureMaker final : public GrTextureMaker {
public:
    GrYUVAImageTextureMaker(GrContext* context, const SkImage* client);

    std::unique_ptr<GrFragmentProcessor> createFragmentProcessor(
            const SkMatrix& textureMatrix,
            const SkRect& constraintRect,
            FilterConstraint filterConstraint,
            bool coordsLimitedToConstraintRect,
            GrSamplerState::WrapMode wrapX,
            GrSamplerState::WrapMode wrapY,
            const GrSamplerState::Filter* filterOrNullForBicubic) override;

    bool isPlanar() const override { return true; }

private:
    GrSurfaceProxyView refOriginalTextureProxyView(GrMipMapped) override;

    const SkImage_GpuYUVA*  fImage;

    typedef GrTextureMaker INHERITED;
};

#endif
