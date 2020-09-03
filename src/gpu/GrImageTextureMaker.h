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
    GrSurfaceProxyView refOriginalTextureProxyView(GrMipmapped) override;

    const SkImage_Lazy*     fImage;
    GrImageTexGenPolicy     fTexGenPolicy;

    using INHERITED = GrTextureMaker;
};

/** This class manages the conversion of generator-backed YUVA images to GrTextures. */
class GrYUVAImageTextureMaker final : public GrTextureMaker {
public:
    GrYUVAImageTextureMaker(GrRecordingContext* context, const SkImage* client);

    std::unique_ptr<GrFragmentProcessor> createFragmentProcessor(const SkMatrix& textureMatrix,
                                                                 const SkRect* subset,
                                                                 const SkRect* domain,
                                                                 GrSamplerState) override;

    std::unique_ptr<GrFragmentProcessor> createBicubicFragmentProcessor(
            const SkMatrix& textureMatrix,
            const SkRect* subset,
            const SkRect* domain,
            GrSamplerState::WrapMode wrapX,
            GrSamplerState::WrapMode wrapY,
            SkImage::CubicResampler) override;

    bool isPlanar() const override { return true; }

private:
    GrSurfaceProxyView refOriginalTextureProxyView(GrMipmapped) override;

    const SkImage_GpuYUVA*  fImage;

    using INHERITED = GrTextureMaker;
};

#endif
