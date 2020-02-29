/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextureMaker_DEFINED
#define GrTextureMaker_DEFINED

#include "src/gpu/GrTextureProducer.h"

/**
 * Base class for sources that start out as something other than a texture (encoded image,
 * picture, ...).
 */
class GrTextureMaker : public GrTextureProducer {
public:
    std::unique_ptr<GrFragmentProcessor> createFragmentProcessor(
            const SkMatrix& textureMatrix,
            const SkRect& constraintRect,
            FilterConstraint filterConstraint,
            bool coordsLimitedToConstraintRect,
            GrSamplerState::WrapMode wrapX,
            GrSamplerState::WrapMode wrapY,
            const GrSamplerState::Filter* filterOrNullForBicubic) override;

protected:
    GrTextureMaker(GrRecordingContext* context, const GrImageInfo& info)
            : INHERITED(context, info) {}

private:
    /**
     *  Return the maker's "original" texture. It is the responsibility of the maker to handle any
     *  caching of the original if desired.
     */
    virtual GrSurfaceProxyView refOriginalTextureProxyView(GrMipMapped) = 0;

    GrSurfaceProxyView onView(GrMipMapped) final;

    typedef GrTextureProducer INHERITED;
};

#endif
