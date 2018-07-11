/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextureAdjuster_DEFINED
#define GrTextureAdjuster_DEFINED

#include "GrTextureProducer.h"
#include "GrTextureProxy.h"
#include "SkTLazy.h"

/**
 * Base class for sources that start out as textures. Optionally allows for a content area subrect.
 * The intent is not to use content area for subrect rendering. Rather, the pixels outside the
 * content area have undefined values and shouldn't be read *regardless* of filtering mode or
 * the SkCanvas::SrcRectConstraint used for subrect draws.
 */
class GrTextureAdjuster : public GrTextureProducer {
public:
    std::unique_ptr<GrFragmentProcessor> createFragmentProcessor(
            const SkMatrix& textureMatrix,
            const SkRect& constraintRect,
            FilterConstraint,
            bool coordsLimitedToConstraintRect,
            const GrSamplerState::Filter* filterOrNullForBicubic,
            SkColorSpace* dstColorSpace) override;

    // We do not ref the texture nor the colorspace, so the caller must keep them in scope while
    // this Adjuster is alive.
    GrTextureAdjuster(GrContext*, sk_sp<GrTextureProxy>, SkAlphaType, uint32_t uniqueID,
                      SkColorSpace*);

protected:
    SkAlphaType alphaType() const override { return fAlphaType; }
    void makeCopyKey(const CopyParams& params, GrUniqueKey* copyKey,
                     SkColorSpace* dstColorSpace) override;
    void didCacheCopy(const GrUniqueKey& copyKey) override;

    GrTextureProxy* originalProxy() const { return fOriginal.get(); }
    sk_sp<GrTextureProxy> originalProxyRef() const { return fOriginal; }

private:
    sk_sp<GrTextureProxy> onRefTextureProxyForParams(const GrSamplerState&,
                                                     SkColorSpace* dstColorSpace,
                                                     sk_sp<SkColorSpace>* proxyColorSpace,
                                                     SkScalar scaleAdjust[2]) override;

    sk_sp<GrTextureProxy> refTextureProxyCopy(const CopyParams& copyParams, bool willBeMipped);

    GrContext*            fContext;
    sk_sp<GrTextureProxy> fOriginal;
    SkAlphaType           fAlphaType;
    SkColorSpace*         fColorSpace;
    uint32_t              fUniqueID;

    typedef GrTextureProducer INHERITED;
};

#endif
