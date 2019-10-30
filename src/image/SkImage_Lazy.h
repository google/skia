/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImage_Lazy_DEFINED
#define SkImage_Lazy_DEFINED

#include "include/private/SkMutex.h"
#include "src/image/SkImage_Base.h"

#if SK_SUPPORT_GPU
#include "src/gpu/GrTextureMaker.h"
#endif

class SharedGenerator;

class SkImage_Lazy : public SkImage_Base {
public:
    struct Validator {
        Validator(sk_sp<SharedGenerator>, const SkIRect* subset, const SkColorType* colorType,
                  sk_sp<SkColorSpace> colorSpace);

        operator bool() const { return fSharedGenerator.get(); }

        sk_sp<SharedGenerator> fSharedGenerator;
        SkImageInfo            fInfo;
        SkIPoint               fOrigin;
        sk_sp<SkColorSpace>    fColorSpace;
        uint32_t               fUniqueID;
    };

    SkImage_Lazy(Validator* validator);
    ~SkImage_Lazy() override;

    SkIRect onGetSubset() const override {
        return SkIRect::MakeXYWH(fOrigin.fX, fOrigin.fY, this->width(), this->height());
    }

    bool onReadPixels(const SkImageInfo&, void*, size_t, int srcX, int srcY,
                      CachingHint) const override;
#if SK_SUPPORT_GPU
    sk_sp<GrTextureProxy> asTextureProxyRef(GrRecordingContext*,
                                            const GrSamplerState&,
                                            SkScalar scaleAdjust[2]) const override;
    sk_sp<SkCachedData> getPlanes(SkYUVASizeInfo*, SkYUVAIndex[4],
                                  SkYUVColorSpace*, const void* planes[4]) override;
#endif
    sk_sp<SkData> onRefEncoded() const override;
    sk_sp<SkImage> onMakeSubset(GrRecordingContext*, const SkIRect&) const override;
    bool getROPixels(SkBitmap*, CachingHint) const override;
    bool onIsLazyGenerated() const override { return true; }
    sk_sp<SkImage> onMakeColorTypeAndColorSpace(GrRecordingContext*,
                                                SkColorType, sk_sp<SkColorSpace>) const override;
    sk_sp<SkImage> onReinterpretColorSpace(sk_sp<SkColorSpace>) const final;

    bool onIsValid(GrContext*) const override;

#if SK_SUPPORT_GPU
    // Returns the texture proxy. If we're going to generate and cache the texture, we should use
    // the passed in key (if the key is valid). If genType is AllowedTexGenType::kCheap and the
    // texture is not trivial to construct, returns nullptr.
    sk_sp<GrTextureProxy> lockTextureProxy(GrRecordingContext*,
                                           const GrUniqueKey& key,
                                           SkImage::CachingHint,
                                           bool willBeMipped,
                                           GrTextureMaker::AllowedTexGenType genType) const;

    void makeCacheKeyFromOrigKey(const GrUniqueKey& origKey, GrUniqueKey* cacheKey) const;
#endif

private:
    class ScopedGenerator;

    // Note that this->imageInfo() is not necessarily the info from the generator. It may be
    // cropped by onMakeSubset and its color type/space may be changed by
    // onMakeColorTypeAndColorSpace.
    sk_sp<SharedGenerator> fSharedGenerator;
    const SkIPoint         fOrigin;

    uint32_t fUniqueID;

    // Repeated calls to onMakeColorTypeAndColorSpace will result in a proliferation of unique IDs
    // and SkImage_Lazy instances. Cache the result of the last successful call.
    mutable SkMutex             fOnMakeColorTypeAndSpaceMutex;
    mutable sk_sp<SkImage>      fOnMakeColorTypeAndSpaceResult;

#if SK_SUPPORT_GPU
    // When the SkImage_Lazy goes away, we will iterate over all the unique keys we've used and
    // send messages to the GrContexts to say the unique keys are no longer valid. The GrContexts
    // can then release the resources, conntected with the those unique keys, from their caches.
    mutable SkTDArray<GrUniqueKeyInvalidatedMessage*> fUniqueKeyInvalidatedMessages;
#endif

    typedef SkImage_Base INHERITED;
};

#endif
