/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextureMaker_DEFINED
#define GrTextureMaker_DEFINED

#include "GrTextureProducer.h"

/**
 * Base class for sources that start out as something other than a texture (encoded image,
 * picture, ...).
 */
class GrTextureMaker : public GrTextureProducer {
public:
    enum class AllowedTexGenType : bool { kCheap, kAny };

    std::unique_ptr<GrFragmentProcessor> createFragmentProcessor(
            const SkMatrix& textureMatrix,
            const SkRect& constraintRect,
            FilterConstraint filterConstraint,
            bool coordsLimitedToConstraintRect,
            const GrSamplerState::Filter* filterOrNullForBicubic) override;

protected:
    GrTextureMaker(GrRecordingContext* context, int width, int height, bool isAlphaOnly,
                   bool domainNeedsLocal)
        : INHERITED(context, width, height, isAlphaOnly, domainNeedsLocal) {}

    /**
     *  Return the maker's "original" texture. It is the responsibility of the maker to handle any
     *  caching of the original if desired.
     *  If "genType" argument equals AllowedTexGenType::kCheap and the texture is not trivial to
     *  construct then refOriginalTextureProxy should return nullptr (for example if texture is made
     *  by drawing into a render target).
     */
    virtual sk_sp<GrTextureProxy> refOriginalTextureProxy(bool willBeMipped,
                                                          AllowedTexGenType genType) = 0;

private:
    sk_sp<GrTextureProxy> onRefTextureProxyForParams(const GrSamplerState&,
                                                     bool willBeMipped,
                                                     SkScalar scaleAdjust[2]) override;

    typedef GrTextureProducer INHERITED;
};

#endif
