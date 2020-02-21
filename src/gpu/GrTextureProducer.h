/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextureProducer_DEFINED
#define GrTextureProducer_DEFINED

#include "include/core/SkImageInfo.h"
#include "include/private/GrResourceKey.h"
#include "include/private/SkNoncopyable.h"
#include "src/gpu/GrImageInfo.h"
#include "src/gpu/GrSamplerState.h"

class GrFragmentProcessor;
class GrRecordingContext;
class GrTexture;
class GrTextureProxy;
class SkColorSpace;
class SkMatrix;
struct SkRect;

/**
 * Different GPUs and API extensions have different requirements with respect to what texture
 * sampling parameters may be used with textures of various types. This class facilitates making
 * texture compatible with a given GrSamplerState. There are two immediate subclasses defined
 * below. One is a base class for sources that are inherently texture-backed (e.g. a texture-backed
 * SkImage). It supports subsetting the original texture. The other is for use cases where the
 * source can generate a texture that represents some content (e.g. cpu pixels, SkPicture, ...).
 */
class GrTextureProducer : public SkNoncopyable {
public:
    enum FilterConstraint {
        kYes_FilterConstraint,
        kNo_FilterConstraint,
    };

    /**
     * Helper for creating a fragment processor to sample the texture with a given filtering mode.
     * It attempts to avoid making texture copies or using domains whenever possible.
     *
     * @param textureMatrix                    Matrix used to access the texture. It is applied to
     *                                         the local coords. The post-transformed coords should
     *                                         be in texel units (rather than normalized) with
     *                                         respect to this Producer's bounds (width()/height()).
     * @param constraintRect                   A rect that represents the area of the texture to be
     *                                         sampled. It must be contained in the Producer's
     *                                         bounds as defined by width()/height().
     * @param filterConstriant                 Indicates whether filtering is limited to
     *                                         constraintRect.
     * @param coordsLimitedToConstraintRect    Is it known that textureMatrix*localCoords is bound
     *                                         by the portion of the texture indicated by
     *                                         constraintRect (without consideration of filter
     *                                         width, just the raw coords).
     * @param filterOrNullForBicubic           If non-null indicates the filter mode. If null means
     *                                         use bicubic filtering.
     **/
    virtual std::unique_ptr<GrFragmentProcessor> createFragmentProcessor(
            const SkMatrix& textureMatrix,
            const SkRect& constraintRect,
            FilterConstraint filterConstraint,
            bool coordsLimitedToConstraintRect,
            const GrSamplerState::Filter* filterOrNullForBicubic) = 0;

    /**
     *  Returns a texture that is safe for use with the params.
     */
    GrSurfaceProxyView viewForParams(GrSamplerState);

    GrSurfaceProxyView viewForParams(const GrSamplerState::Filter* filterOrNullForBicubic);

    /**
     * Returns a texture. If willNeedMips is true then the returned texture is guaranteed to have
     * allocated mip map levels. This can be a performance win if future draws with the texture
     * require mip maps.
     */
    // TODO: Once we remove support for npot textures, we should add a flag for must support repeat
    // wrap mode. To support that flag now would require us to support scaleAdjust array like in
    // refTextureProxyForParams, however the current public API that uses this call does not expose
    // that array.
    std::pair<GrSurfaceProxyView, GrColorType> view(GrMipMapped willNeedMips);

    virtual ~GrTextureProducer() {}

    int width() const { return fImageInfo.width(); }
    int height() const { return fImageInfo.height(); }
    SkISize dimensions() const { return fImageInfo.dimensions(); }
    SkAlphaType alphaType() const { return fImageInfo.alphaType(); }
    SkColorSpace* colorSpace() const { return fImageInfo.colorSpace(); }
    bool isAlphaOnly() const { return GrColorTypeIsAlphaOnly(fImageInfo.colorType()); }
    bool domainNeedsDecal() const { return fDomainNeedsDecal; }
    // If the "texture" samples multiple images that have different resolutions (e.g. YUV420)
    virtual bool hasMixedResolutions() const { return false; }

protected:
    friend class GrTextureProducer_TestAccess;

    GrTextureProducer(GrRecordingContext* context,
                      const GrImageInfo& imageInfo,
                      bool domainNeedsDecal)
            : fContext(context), fImageInfo(imageInfo), fDomainNeedsDecal(domainNeedsDecal) {}

    GrColorType colorType() const { return fImageInfo.colorType(); }

    /** Helper for creating a key for a copy from an original key. */
    static void MakeMipMappedKeyFromOriginalKey(const GrUniqueKey& origKey,
                                                GrUniqueKey* mipMappedKey) {
        SkASSERT(!mipMappedKey->isValid());
        if (origKey.isValid()) {
            static const GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();
            GrUniqueKey::Builder builder(mipMappedKey, origKey, kDomain, 0);
        }
    }

    /**
     * If we need to make a copy with MIP maps the producer is asked to return a key that identifies
     * the original conteny + the addition of MIP map levels. If the producer does not want to cache
     * the copy it can simply leave the key uninitialized.
     */
    virtual void makeMipMappedKey(GrUniqueKey* mipMappedKey) = 0;

    /**
     *  If a stretched version of the texture is generated, it may be cached (assuming that
     *  makeMipMappedKey() returns true). In that case, the maker is notified in case it
     *  wants to note that for when the maker is destroyed.
     */
    virtual void didCacheMipMappedCopy(const GrUniqueKey& mipMappedKey,
                                       uint32_t contextUniqueID) = 0;

    enum DomainMode {
        kNoDomain_DomainMode,
        kDomain_DomainMode,
        kTightCopy_DomainMode
    };

    static DomainMode DetermineDomainMode(const SkRect& constraintRect,
                                          FilterConstraint filterConstraint,
                                          bool coordsLimitedToConstraintRect,
                                          GrSurfaceProxy*,
                                          const GrSamplerState::Filter* filterModeOrNullForBicubic,
                                          SkRect* domainRect);

    std::unique_ptr<GrFragmentProcessor> createFragmentProcessorForDomainAndFilter(
            GrSurfaceProxyView view,
            const SkMatrix& textureMatrix,
            DomainMode,
            const SkRect& domain,
            const GrSamplerState::Filter* filterOrNullForBicubic);

    GrRecordingContext* context() const { return fContext; }

private:
    virtual GrSurfaceProxyView onRefTextureProxyViewForParams(GrSamplerState,
                                                              bool willBeMipped) = 0;

    GrRecordingContext* fContext;
    const GrImageInfo fImageInfo;
    // If true, any domain effect uses kDecal instead of kClamp, and sampler filter uses
    // kClampToBorder instead of kClamp.
    const bool  fDomainNeedsDecal;

    typedef SkNoncopyable INHERITED;
};

#endif
