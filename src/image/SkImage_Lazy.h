/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImage_Lazy_DEFINED
#define SkImage_Lazy_DEFINED

#include "SkImage_Base.h"
#include "SkMutex.h"

#if SK_SUPPORT_GPU
#include "GrTextureMaker.h"
#endif

class SharedGenerator;

class SkImage_Lazy : public SkImage_Base {
public:
    struct Validator {
        Validator(sk_sp<SharedGenerator>, const SkIRect* subset, sk_sp<SkColorSpace> colorSpace);

        operator bool() const { return fSharedGenerator.get(); }

        sk_sp<SharedGenerator> fSharedGenerator;
        SkImageInfo            fInfo;
        SkIPoint               fOrigin;
        sk_sp<SkColorSpace>    fColorSpace;
        uint32_t               fUniqueID;
    };

    SkImage_Lazy(Validator* validator);
    ~SkImage_Lazy() override;

    SkImageInfo onImageInfo() const override {
        return fInfo;
    }

    SkIRect onGetSubset() const override {
        return SkIRect::MakeXYWH(fOrigin.fX, fOrigin.fY, fInfo.width(), fInfo.height());
    }

    bool onReadPixels(const SkImageInfo&, void*, size_t, int srcX, int srcY,
                      CachingHint) const override;
#if SK_SUPPORT_GPU
    sk_sp<GrTextureProxy> asTextureProxyRef(GrContext*,
                                            const GrSamplerState&, SkColorSpace*,
                                            sk_sp<SkColorSpace>*,
                                            SkScalar scaleAdjust[2]) const override;
    sk_sp<SkCachedData> getPlanes(SkYUVSizeInfo*, SkYUVColorSpace*, const void* planes[3]) override;
#endif
    sk_sp<SkData> onRefEncoded() const override;
    sk_sp<SkImage> onMakeSubset(const SkIRect&) const override;
    bool getROPixels(SkBitmap*, SkColorSpace* dstColorSpace, CachingHint) const override;
    bool onIsLazyGenerated() const override { return true; }
    sk_sp<SkImage> onMakeColorSpace(sk_sp<SkColorSpace>) const override;

    bool onIsValid(GrContext*) const override;

    // Only return true if the generate has already been cached, in a format that matches the info.
    bool lockAsBitmapOnlyIfAlreadyCached(SkBitmap*, const SkImageInfo&) const;
    // Call the underlying generator directly
    bool directGeneratePixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRB,
                              int srcX, int srcY) const;

#if SK_SUPPORT_GPU
    // Returns the texture proxy. If we're going to generate and cache the texture, we should use
    // the passed in key (if the key is valid). If genType is AllowedTexGenType::kCheap and the
    // texture is not trivial to construct, returns nullptr.
    sk_sp<GrTextureProxy> lockTextureProxy(GrContext*,
                                           const GrUniqueKey& key,
                                           SkImage::CachingHint,
                                           bool willBeMipped,
                                           SkColorSpace* dstColorSpace,
                                           GrTextureMaker::AllowedTexGenType genType) const;

    // TODO: Need to pass in dstColorSpace to fold into key here?
    void makeCacheKeyFromOrigKey(const GrUniqueKey& origKey, GrUniqueKey* cacheKey) const;
#endif

private:
    class ScopedGenerator;

    /**
     *  On success (true), bitmap will point to the pixels for this generator. If this returns
     *  false, the bitmap will be reset to empty.
     *  TODO: Pass in dstColorSpace to ensure bitmap is compatible?
     */
    bool lockAsBitmap(SkBitmap*, SkImage::CachingHint, const SkImageInfo&) const;

    sk_sp<SharedGenerator> fSharedGenerator;
    // Note that fInfo is not necessarily the info from the generator. It may be cropped by
    // onMakeSubset and its color space may be changed by onMakeColorSpace.
    const SkImageInfo      fInfo;
    const SkIPoint         fOrigin;

    uint32_t fUniqueID;

    // Repeated calls to onMakeColorSpace will result in a proliferation of unique IDs and
    // SkImage_Lazy instances. Cache the result of the last successful onMakeColorSpace call.
    mutable SkMutex             fOnMakeColorSpaceMutex;
    mutable sk_sp<SkColorSpace> fOnMakeColorSpaceTarget;
    mutable sk_sp<SkImage>      fOnMakeColorSpaceResult;

#if SK_SUPPORT_GPU
    // When the SkImage_Lazy goes away, we will iterate over all the unique keys we've used and
    // send messages to the GrContexts to say the unique keys are no longer valid. The GrContexts
    // can then release the resources, conntected with the those unique keys, from their caches.
    mutable SkTDArray<GrUniqueKeyInvalidatedMessage*> fUniqueKeyInvalidatedMessages;
#endif

    typedef SkImage_Base INHERITED;
};

#endif
