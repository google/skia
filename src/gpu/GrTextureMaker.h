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
    /**
     *  Returns a texture that is safe for use with the params. If the size of the returned texture
     *  does not match width()/height() then the contents of the original must be scaled to fit
     *  the texture. Additionally, the 'scaleAdjust' must be applied to the texture matrix
     *  in order to correct the absolute texture coordinates.
     *  Places the color space of the texture in (*texColorSpace).
     */
    sk_sp<GrTextureProxy> refTextureProxyForParams(const GrSamplerParams&,
                                                   SkColorSpace* dstColorSpace,
                                                   sk_sp<SkColorSpace>* texColorSpace,
                                                   SkScalar scaleAdjust[2]);

    sk_sp<GrFragmentProcessor> createFragmentProcessor(
                                const SkMatrix& textureMatrix,
                                const SkRect& constraintRect,
                                FilterConstraint filterConstraint,
                                bool coordsLimitedToConstraintRect,
                                const GrSamplerParams::FilterMode* filterOrNullForBicubic,
                                SkColorSpace* dstColorSpace) override;

protected:
    GrTextureMaker(GrContext* context, int width, int height, bool isAlphaOnly)
        : INHERITED(width, height, isAlphaOnly)
        , fContext(context) {}

    /**
     *  Return the maker's "original" texture. It is the responsibility of the maker to handle any
     *  caching of the original if desired.
     */
    virtual sk_sp<GrTextureProxy> refOriginalTextureProxy(bool willBeMipped,
                                                          SkColorSpace* dstColorSpace) = 0;

    /**
     *  Returns the color space of the maker's "original" texture, assuming it was retrieved with
     *  the same destination color space.
     */
    virtual sk_sp<SkColorSpace> getColorSpace(SkColorSpace* dstColorSpace) = 0;

    /**
     *  Return a new (uncached) texture that is the stretch of the maker's original.
     *
     *  The base-class handles general logic for this, and only needs access to the following
     *  method:
     *  - refOriginalTextureProxy()
     *
     *  Subclass may override this if they can handle creating the texture more directly than
     *  by copying.
     */
    virtual sk_sp<GrTextureProxy> generateTextureProxyForParams(const CopyParams&,
                                                                bool willBeMipped,
                                                                SkColorSpace* dstColorSpace);

    GrContext* context() const { return fContext; }

private:
    GrContext*  fContext;

    typedef GrTextureProducer INHERITED;
};

#endif
