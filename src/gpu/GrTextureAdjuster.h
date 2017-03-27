/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextureAdjuster_DEFINED
#define GrTextureAdjuster_DEFINED

#include "GrTextureProducer.h"
#include "SkTLazy.h"

/**
 * Base class for sources that start out as textures. Optionally allows for a content area subrect.
 * The intent is not to use content area for subrect rendering. Rather, the pixels outside the
 * content area have undefined values and shouldn't be read *regardless* of filtering mode or
 * the SkCanvas::SrcRectConstraint used for subrect draws.
 */
class GrTextureAdjuster : public GrTextureProducer {
public:
    /** Makes the subset of the texture safe to use with the given texture parameters.
        outOffset will be the top-left corner of the subset if a copy is not made. Otherwise,
        the copy will be tight to the contents and outOffset will be (0, 0). If the copy's size
        does not match subset's dimensions then the contents are scaled to fit the copy.*/
    sk_sp<GrTextureProxy> refTextureProxySafeForParams(const GrSamplerParams&, SkIPoint* outOffset,
                                                       SkScalar scaleAdjust[2]);

    sk_sp<GrFragmentProcessor> createFragmentProcessor(
                                const SkMatrix& textureMatrix,
                                const SkRect& constraintRect,
                                FilterConstraint,
                                bool coordsLimitedToConstraintRect,
                                const GrSamplerParams::FilterMode* filterOrNullForBicubic,
                                SkColorSpace* dstColorSpace) override;

    // We do not ref the texture nor the colorspace, so the caller must keep them in scope while
    // this Adjuster is alive.
    GrTextureAdjuster(GrContext*, sk_sp<GrTextureProxy>, SkAlphaType, const SkIRect& area,
                      uint32_t uniqueID, SkColorSpace*);

protected:
    SkAlphaType alphaType() const override { return fAlphaType; }
    void makeCopyKey(const CopyParams& params, GrUniqueKey* copyKey,
                     SkColorSpace* dstColorSpace) override;
    void didCacheCopy(const GrUniqueKey& copyKey) override;

    GrTextureProxy* originalProxy() const { return fOriginal.get(); }
    sk_sp<GrTextureProxy> originalProxyRef() const { return fOriginal; }

    /** Returns the content area or null for the whole original texture */
    const SkIRect* contentAreaOrNull() { return fContentArea.getMaybeNull(); }

private:
    SkTLazy<SkIRect>      fContentArea;
    GrContext*            fContext;
    sk_sp<GrTextureProxy> fOriginal;
    SkAlphaType           fAlphaType;
    SkColorSpace*         fColorSpace;
    uint32_t              fUniqueID;

    sk_sp<GrTextureProxy> refTextureProxyCopy(const CopyParams &copyParams);

    typedef GrTextureProducer INHERITED;
};

#endif
