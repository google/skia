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
#include "src/gpu/GrColorInfo.h"
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
    struct CopyParams {
        GrSamplerState::Filter fFilter;
        int fWidth;
        int fHeight;
    };

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
     *
     * If the size of the returned texture does not match width()/height() then the contents of the
     * original may have been scaled to fit the texture or the original may have been copied into
     * a subrect of the copy. 'scaleAdjust' must be  applied to the normalized texture coordinates
     * in order to correct for the latter case.
     *
     * If the GrSamplerState is known to clamp and use kNearest or kBilerp filter mode then the
     * proxy will always be unscaled and nullptr can be passed for scaleAdjust. There is a weird
     * contract that if scaleAdjust is not null it must be initialized to {1, 1} before calling
     * this method. (TODO: Fix this and make this function always initialize scaleAdjust).
     */
    sk_sp<GrTextureProxy> refTextureProxyForParams(const GrSamplerState&,
                                                   SkScalar scaleAdjust[2]);

    sk_sp<GrTextureProxy> refTextureProxyForParams(
            const GrSamplerState::Filter* filterOrNullForBicubic, SkScalar scaleAdjust[2]);

    /**
     * Returns a texture. If willNeedMips is true then the returned texture is guaranteed to have
     * allocated mip map levels. This can be a performance win if future draws with the texture
     * require mip maps.
     */
    // TODO: Once we remove support for npot textures, we should add a flag for must support repeat
    // wrap mode. To support that flag now would require us to support scaleAdjust array like in
    // refTextureProxyForParams, however the current public API that uses this call does not expose
    // that array.
    sk_sp<GrTextureProxy> refTextureProxy(GrMipMapped willNeedMips);

    virtual ~GrTextureProducer() {}

    int width() const { return fWidth; }
    int height() const { return fHeight; }
    const GrColorInfo& colorInfo() const { return fColorInfo; }
    GrColorType colorType() const { return fColorInfo.colorType(); }
    SkAlphaType alphaType() const { return fColorInfo.alphaType(); }
    SkColorSpace* colorSpace() const { return fColorInfo.colorSpace(); }
    bool isAlphaOnly() const { return GrColorTypeIsAlphaOnly(fColorInfo.colorType()); }
    bool domainNeedsDecal() const { return fDomainNeedsDecal; }
    // If the "texture" samples multiple images that have different resolutions (e.g. YUV420)
    virtual bool hasMixedResolutions() const { return false; }

protected:
    friend class GrTextureProducer_TestAccess;

    GrTextureProducer(GrRecordingContext* context, int width, int height,
                      const GrColorInfo& colorInfo, bool domainNeedsDecal)
            : fContext(context)
            , fWidth(width)
            , fHeight(height)
            , fColorInfo(colorInfo)
            , fDomainNeedsDecal(domainNeedsDecal) {}

    /** Helper for creating a key for a copy from an original key. */
    static void MakeCopyKeyFromOrigKey(const GrUniqueKey& origKey,
                                       const CopyParams& copyParams,
                                       GrUniqueKey* copyKey) {
        SkASSERT(!copyKey->isValid());
        if (origKey.isValid()) {
            static const GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();
            GrUniqueKey::Builder builder(copyKey, origKey, kDomain, 3);
            builder[0] = static_cast<uint32_t>(copyParams.fFilter);
            builder[1] = copyParams.fWidth;
            builder[2] = copyParams.fHeight;
        }
    }

    /**
    *  If we need to make a copy in order to be compatible with GrTextureParams producer is asked to
    *  return a key that identifies its original content + the CopyParms parameter. If the producer
    *  does not want to cache the stretched version (e.g. the producer is volatile), this should
    *  simply return without initializing the copyKey. If the texture generated by this producer
    *  depends on the destination color space, then that information should also be incorporated
    *  in the key.
    */
    virtual void makeCopyKey(const CopyParams&, GrUniqueKey* copyKey) = 0;

    /**
    *  If a stretched version of the texture is generated, it may be cached (assuming that
    *  makeCopyKey() returns true). In that case, the maker is notified in case it
    *  wants to note that for when the maker is destroyed.
    */
    virtual void didCacheCopy(const GrUniqueKey& copyKey, uint32_t contextUniqueID) = 0;

    enum DomainMode {
        kNoDomain_DomainMode,
        kDomain_DomainMode,
        kTightCopy_DomainMode
    };

    // This can draw to accomplish the copy, thus the recording context is needed
    static sk_sp<GrTextureProxy> CopyOnGpu(GrRecordingContext*,
                                           sk_sp<GrTextureProxy> inputProxy,
                                           GrColorType,
                                           const CopyParams& copyParams,
                                           bool dstWillRequireMipMaps);

    static DomainMode DetermineDomainMode(const SkRect& constraintRect,
                                          FilterConstraint filterConstraint,
                                          bool coordsLimitedToConstraintRect,
                                          GrTextureProxy*,
                                          const GrSamplerState::Filter* filterModeOrNullForBicubic,
                                          SkRect* domainRect);

    std::unique_ptr<GrFragmentProcessor> createFragmentProcessorForDomainAndFilter(
            sk_sp<GrTextureProxy> proxy,
            const SkMatrix& textureMatrix,
            DomainMode,
            const SkRect& domain,
            const GrSamplerState::Filter* filterOrNullForBicubic);

    GrRecordingContext* context() const { return fContext; }

private:
    virtual sk_sp<GrTextureProxy> onRefTextureProxyForParams(const GrSamplerState&,
                                                             bool willBeMipped,
                                                             SkScalar scaleAdjust[2]) = 0;

    GrRecordingContext* fContext;
    const int fWidth;
    const int fHeight;
    const GrColorInfo fColorInfo;
    // If true, any domain effect uses kDecal instead of kClamp, and sampler filter uses
    // kClampToBorder instead of kClamp.
    const bool  fDomainNeedsDecal;

    typedef SkNoncopyable INHERITED;
};

#endif
