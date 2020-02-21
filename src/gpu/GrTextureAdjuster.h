/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextureAdjuster_DEFINED
#define GrTextureAdjuster_DEFINED

#include "src/core/SkTLazy.h"
#include "src/gpu/GrTextureProducer.h"
#include "src/gpu/GrTextureProxy.h"

class GrRecordingContext;

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
            const GrSamplerState::Filter* filterOrNullForBicubic) override;

    GrTextureAdjuster(GrRecordingContext*, GrSurfaceProxyView, const GrColorInfo&,
                      uint32_t uniqueID, bool useDecal = false);

protected:
    void makeCopyKey(const CopyParams& params, GrUniqueKey* copyKey) override;
    void didCacheCopy(const GrUniqueKey& copyKey, uint32_t contextUniqueID) override;

    const GrSurfaceProxyView& originalProxyView() const { return fOriginal; }
    GrSurfaceProxyView originalProxyViewRef() const { return fOriginal; }

private:
    GrSurfaceProxyView onRefTextureProxyViewForParams(GrSamplerState, bool willBeMipped,
                                                      SkScalar scaleAdjust[2]) override;

    GrSurfaceProxyView copy(const CopyParams& copyParams, bool willBeMipped, bool copyOnlyForMips);

    GrSurfaceProxyView fOriginal;
    uint32_t fUniqueID;

    typedef GrTextureProducer INHERITED;
};

#endif
