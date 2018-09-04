/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImage_Base.h"
#include "SkImageCacherator.h"

#include "SkBitmap.h"
#include "SkBitmapCache.h"
#include "SkData.h"
#include "SkImageGenerator.h"
#include "SkImagePriv.h"
#include "SkNextID.h"
#include "SkPixelRef.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrGpuResourcePriv.h"
#include "GrImageTextureMaker.h"
#include "GrResourceKey.h"
#include "GrProxyProvider.h"
#include "GrSamplerState.h"
#include "GrYUVProvider.h"
#include "SkGr.h"
#endif

// Ref-counted tuple(SkImageGenerator, SkMutex) which allows sharing one generator among N images
class SharedGenerator final : public SkNVRefCnt<SharedGenerator> {
public:
    static sk_sp<SharedGenerator> Make(std::unique_ptr<SkImageGenerator> gen) {
        return gen ? sk_sp<SharedGenerator>(new SharedGenerator(std::move(gen))) : nullptr;
    }

    // This is thread safe.  It is a const field set in the constructor.
    const SkImageInfo& getInfo() { return fGenerator->getInfo(); }

private:
    explicit SharedGenerator(std::unique_ptr<SkImageGenerator> gen)
            : fGenerator(std::move(gen)) {
        SkASSERT(fGenerator);
    }

    friend class ScopedGenerator;
    friend class SkImage_Lazy;

    std::unique_ptr<SkImageGenerator> fGenerator;
    SkMutex                           fMutex;
};

class SkImage_Lazy : public SkImage_Base, public SkImageCacherator {
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

    SkImageInfo onImageInfo() const override {
        return fInfo;
    }
    SkColorType onColorType() const override {
        return kUnknown_SkColorType;
    }
    SkAlphaType onAlphaType() const override {
        return fInfo.alphaType();
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
#endif
    sk_sp<SkData> onRefEncoded() const override;
    sk_sp<SkImage> onMakeSubset(const SkIRect&) const override;
    bool getROPixels(SkBitmap*, SkColorSpace* dstColorSpace, CachingHint) const override;
    bool onIsLazyGenerated() const override { return true; }
    sk_sp<SkImage> onMakeColorSpace(sk_sp<SkColorSpace>, SkColorType) const override;

    bool onIsValid(GrContext*) const override;

    SkImageCacherator* peekCacherator() const override {
        return const_cast<SkImage_Lazy*>(this);
    }

    // Only return true if the generate has already been cached.
    bool lockAsBitmapOnlyIfAlreadyCached(SkBitmap*) const;
    // Call the underlying generator directly
    bool directGeneratePixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRB,
                              int srcX, int srcY) const;

    // SkImageCacherator interface
#if SK_SUPPORT_GPU
    // Returns the texture proxy. If the cacherator is generating the texture and wants to cache it,
    // it should use the passed in key (if the key is valid).
    sk_sp<GrTextureProxy> lockTextureProxy(GrContext*,
                                           const GrUniqueKey& key,
                                           SkImage::CachingHint,
                                           bool willBeMipped,
                                           SkColorSpace* dstColorSpace,
                                           GrTextureMaker::AllowedTexGenType genType) override;

    // Returns the color space of the texture that would be returned if you called lockTexture.
    // Separate code path to allow querying of the color space for textures that cached (even
    // externally).
    sk_sp<SkColorSpace> getColorSpace(GrContext*, SkColorSpace* dstColorSpace) override;

    // TODO: Need to pass in dstColorSpace to fold into key here?
    void makeCacheKeyFromOrigKey(const GrUniqueKey& origKey, GrUniqueKey* cacheKey) override;
#endif

    SkImageInfo buildCacheInfo() const override;

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

    typedef SkImage_Base INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

SkImage_Lazy::Validator::Validator(sk_sp<SharedGenerator> gen, const SkIRect* subset,
                                   sk_sp<SkColorSpace> colorSpace)
        : fSharedGenerator(std::move(gen)) {
    if (!fSharedGenerator) {
        return;
    }

    // The following generator accessors are safe without acquiring the mutex (const getters).
    // TODO: refactor to use a ScopedGenerator instead, for clarity.
    const SkImageInfo& info = fSharedGenerator->fGenerator->getInfo();
    if (info.isEmpty()) {
        fSharedGenerator.reset();
        return;
    }

    fUniqueID = fSharedGenerator->fGenerator->uniqueID();
    const SkIRect bounds = SkIRect::MakeWH(info.width(), info.height());
    if (subset) {
        if (!bounds.contains(*subset)) {
            fSharedGenerator.reset();
            return;
        }
        if (*subset != bounds) {
            // we need a different uniqueID since we really are a subset of the raw generator
            fUniqueID = SkNextID::ImageID();
        }
    } else {
        subset = &bounds;
    }

    fInfo   = info.makeWH(subset->width(), subset->height());
    fOrigin = SkIPoint::Make(subset->x(), subset->y());
    if (colorSpace) {
        fInfo = fInfo.makeColorSpace(colorSpace);
        fUniqueID = SkNextID::ImageID();
    }
}

///////////////////////////////////////////////////////////////////////////////

// Helper for exclusive access to a shared generator.
class SkImage_Lazy::ScopedGenerator {
public:
    ScopedGenerator(const sk_sp<SharedGenerator>& gen)
      : fSharedGenerator(gen)
      , fAutoAquire(gen->fMutex) {}

    SkImageGenerator* operator->() const {
        fSharedGenerator->fMutex.assertHeld();
        return fSharedGenerator->fGenerator.get();
    }

    operator SkImageGenerator*() const {
        fSharedGenerator->fMutex.assertHeld();
        return fSharedGenerator->fGenerator.get();
    }

private:
    const sk_sp<SharedGenerator>& fSharedGenerator;
    SkAutoExclusive               fAutoAquire;
};

///////////////////////////////////////////////////////////////////////////////

SkImage_Lazy::SkImage_Lazy(Validator* validator)
        : INHERITED(validator->fInfo.width(), validator->fInfo.height(), validator->fUniqueID)
        , fSharedGenerator(std::move(validator->fSharedGenerator))
        , fInfo(validator->fInfo)
        , fOrigin(validator->fOrigin) {
    SkASSERT(fSharedGenerator);
    fUniqueID = validator->fUniqueID;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

SkImageInfo SkImage_Lazy::buildCacheInfo() const {
    if (kGray_8_SkColorType == fInfo.colorType()) {
        return fInfo.makeColorSpace(nullptr);
    } else {
        return fInfo;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////

static bool check_output_bitmap(const SkBitmap& bitmap, uint32_t expectedID) {
    SkASSERT(bitmap.getGenerationID() == expectedID);
    SkASSERT(bitmap.isImmutable());
    SkASSERT(bitmap.getPixels());
    return true;
}

bool SkImage_Lazy::directGeneratePixels(const SkImageInfo& info, void* pixels, size_t rb,
                                        int srcX, int srcY) const {
    ScopedGenerator generator(fSharedGenerator);
    const SkImageInfo& genInfo = generator->getInfo();
    // Currently generators do not natively handle subsets, so check that first.
    if (srcX || srcY || genInfo.width() != info.width() || genInfo.height() != info.height()) {
        return false;
    }

    return generator->getPixels(info, pixels, rb);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

bool SkImage_Lazy::lockAsBitmapOnlyIfAlreadyCached(SkBitmap* bitmap) const {
    return SkBitmapCache::Find(SkBitmapCacheDesc::Make(fUniqueID,
                                                       fInfo.width(), fInfo.height()), bitmap) &&
           check_output_bitmap(*bitmap, fUniqueID);
}

static bool generate_pixels(SkImageGenerator* gen, const SkPixmap& pmap, int originX, int originY) {
    const int genW = gen->getInfo().width();
    const int genH = gen->getInfo().height();
    const SkIRect srcR = SkIRect::MakeWH(genW, genH);
    const SkIRect dstR = SkIRect::MakeXYWH(originX, originY, pmap.width(), pmap.height());
    if (!srcR.contains(dstR)) {
        return false;
    }

    // If they are requesting a subset, we have to have a temp allocation for full image, and
    // then copy the subset into their allocation
    SkBitmap full;
    SkPixmap fullPM;
    const SkPixmap* dstPM = &pmap;
    if (srcR != dstR) {
        if (!full.tryAllocPixels(pmap.info().makeWH(genW, genH))) {
            return false;
        }
        if (!full.peekPixels(&fullPM)) {
            return false;
        }
        dstPM = &fullPM;
    }

    if (!gen->getPixels(dstPM->info(), dstPM->writable_addr(), dstPM->rowBytes())) {
        return false;
    }

    if (srcR != dstR) {
        if (!full.readPixels(pmap, originX, originY)) {
            return false;
        }
    }
    return true;
}

bool SkImage_Lazy::lockAsBitmap(SkBitmap* bitmap, SkImage::CachingHint chint,
                                const SkImageInfo& info) const {
    // TODO: Verify dstColorSpace here
    if (this->lockAsBitmapOnlyIfAlreadyCached(bitmap)) {
        return true;
    }

    SkBitmap tmpBitmap;
    SkBitmapCache::RecPtr cacheRec;
    SkPixmap pmap;
    if (SkImage::kAllow_CachingHint == chint) {
        auto desc = SkBitmapCacheDesc::Make(fUniqueID, info.width(), info.height());
        cacheRec = SkBitmapCache::Alloc(desc, info, &pmap);
        if (!cacheRec) {
            return false;
        }
    } else {
        if (!tmpBitmap.tryAllocPixels(info)) {
            return false;
        }
        if (!tmpBitmap.peekPixels(&pmap)) {
            return false;
        }
    }

    ScopedGenerator generator(fSharedGenerator);
    if (!generate_pixels(generator, pmap, fOrigin.x(), fOrigin.y())) {
        return false;
    }

    if (cacheRec) {
        SkBitmapCache::Add(std::move(cacheRec), bitmap);
        SkASSERT(bitmap->getPixels());  // we're locked
        SkASSERT(bitmap->isImmutable());
        SkASSERT(bitmap->getGenerationID() == fUniqueID);
        this->notifyAddedToRasterCache();
    } else {
        *bitmap = tmpBitmap;
        bitmap->pixelRef()->setImmutableWithID(fUniqueID);
    }

    check_output_bitmap(*bitmap, fUniqueID);
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

bool SkImage_Lazy::onReadPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRB,
                                int srcX, int srcY, CachingHint chint) const {
    SkColorSpace* dstColorSpace = dstInfo.colorSpace();
    SkBitmap bm;
    if (kDisallow_CachingHint == chint) {
        if (this->lockAsBitmapOnlyIfAlreadyCached(&bm)) {
            return bm.readPixels(dstInfo, dstPixels, dstRB, srcX, srcY);
        } else {
            // Try passing the caller's buffer directly down to the generator. If this fails we
            // may still succeed in the general case, as the generator may prefer some other
            // config, which we could then convert via SkBitmap::readPixels.
            if (this->directGeneratePixels(dstInfo, dstPixels, dstRB, srcX, srcY)) {
                return true;
            }
            // else fall through
        }
    }

    if (this->getROPixels(&bm, dstColorSpace, chint)) {
        return bm.readPixels(dstInfo, dstPixels, dstRB, srcX, srcY);
    }
    return false;
}

sk_sp<SkData> SkImage_Lazy::onRefEncoded() const {
    ScopedGenerator generator(fSharedGenerator);
    return generator->refEncodedData();
}

bool SkImage_Lazy::getROPixels(SkBitmap* bitmap, SkColorSpace* dstColorSpace,
                               CachingHint chint) const {
    const SkImageInfo cacheInfo = this->buildCacheInfo();
    return this->lockAsBitmap(bitmap, chint, cacheInfo);
}

bool SkImage_Lazy::onIsValid(GrContext* context) const {
    ScopedGenerator generator(fSharedGenerator);
    return generator->isValid(context);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU
sk_sp<GrTextureProxy> SkImage_Lazy::asTextureProxyRef(GrContext* context,
                                                      const GrSamplerState& params,
                                                      SkColorSpace* dstColorSpace,
                                                      sk_sp<SkColorSpace>* texColorSpace,
                                                      SkScalar scaleAdjust[2]) const {
    if (!context) {
        return nullptr;
    }

    GrImageTextureMaker textureMaker(context, this, kAllow_CachingHint);
    return textureMaker.refTextureProxyForParams(params, dstColorSpace, texColorSpace, scaleAdjust);
}
#endif

sk_sp<SkImage> SkImage_Lazy::onMakeSubset(const SkIRect& subset) const {
    SkASSERT(fInfo.bounds().contains(subset));
    SkASSERT(fInfo.bounds() != subset);

    const SkIRect generatorSubset = subset.makeOffset(fOrigin.x(), fOrigin.y());
    Validator validator(fSharedGenerator, &generatorSubset, fInfo.refColorSpace());
    return validator ? sk_sp<SkImage>(new SkImage_Lazy(&validator)) : nullptr;
}

sk_sp<SkImage> SkImage_Lazy::onMakeColorSpace(sk_sp<SkColorSpace> target,
                                              SkColorType targetColorType) const {
    SkAutoExclusive autoAquire(fOnMakeColorSpaceMutex);
    if (target && fOnMakeColorSpaceTarget &&
        SkColorSpace::Equals(target.get(), fOnMakeColorSpaceTarget.get())) {
        return fOnMakeColorSpaceResult;
    }
    const SkIRect generatorSubset =
            SkIRect::MakeXYWH(fOrigin.x(), fOrigin.y(), fInfo.width(), fInfo.height());
    Validator validator(fSharedGenerator, &generatorSubset, target);
    sk_sp<SkImage> result = validator ? sk_sp<SkImage>(new SkImage_Lazy(&validator)) : nullptr;
    if (result) {
        fOnMakeColorSpaceTarget = target;
        fOnMakeColorSpaceResult = result;
    }
    return result;
}

sk_sp<SkImage> SkImage::MakeFromGenerator(std::unique_ptr<SkImageGenerator> generator,
                                          const SkIRect* subset) {
    SkImage_Lazy::Validator validator(SharedGenerator::Make(std::move(generator)), subset, nullptr);

    return validator ? sk_make_sp<SkImage_Lazy>(&validator) : nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

/**
 *  Implementation of SkImageCacherator interface, as needed by GrImageTextureMaker
 */

#if SK_SUPPORT_GPU

void SkImage_Lazy::makeCacheKeyFromOrigKey(const GrUniqueKey& origKey, GrUniqueKey* cacheKey) {
    // TODO: Take dstColorSpace, include hash in key
    SkASSERT(!cacheKey->isValid());
    if (origKey.isValid()) {
        static const GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();
        GrUniqueKey::Builder builder(cacheKey, origKey, kDomain, 0, "Image");
    }
}

class Generator_GrYUVProvider : public GrYUVProvider {
    SkImageGenerator* fGen;

public:
    Generator_GrYUVProvider(SkImageGenerator* gen) : fGen(gen) {}

    uint32_t onGetID() override { return fGen->uniqueID(); }
    bool onQueryYUV8(SkYUVSizeInfo* sizeInfo, SkYUVColorSpace* colorSpace) const override {
        return fGen->queryYUV8(sizeInfo, colorSpace);
    }
    bool onGetYUV8Planes(const SkYUVSizeInfo& sizeInfo, void* planes[3]) override {
        return fGen->getYUV8Planes(sizeInfo, planes);
    }
};

static void set_key_on_proxy(GrProxyProvider* proxyProvider,
                             GrTextureProxy* proxy, GrTextureProxy* originalProxy,
                             const GrUniqueKey& key) {
    if (key.isValid()) {
        SkASSERT(proxy->origin() == kTopLeft_GrSurfaceOrigin);
        if (originalProxy && originalProxy->getUniqueKey().isValid()) {
            SkASSERT(originalProxy->getUniqueKey() == key);
            SkASSERT(GrMipMapped::kYes == proxy->mipMapped() &&
                     GrMipMapped::kNo == originalProxy->mipMapped());
            // If we had an originalProxy with a valid key, that means there already is a proxy in
            // the cache which matches the key, but it does not have mip levels and we require them.
            // Thus we must remove the unique key from that proxy.
            proxyProvider->removeUniqueKeyFromProxy(key, originalProxy);
        }
        proxyProvider->assignUniqueKeyToProxy(key, proxy);
    }
}

sk_sp<SkColorSpace> SkImage_Lazy::getColorSpace(GrContext* ctx, SkColorSpace* dstColorSpace) {
    // TODO: Is this ever needed? Is the output of this function going to be:
    // return dstColorSpace ? fInfo.refColorSpace() : dstColorSpace;
    // Yes, except for gray?
    if (!dstColorSpace) {
        // In legacy mode, we do no modification to the image's color space or encoding.
        // Subsequent legacy drawing is likely to ignore the color space, but some clients
        // may want to know what space the image data is in, so return it.
        return fInfo.refColorSpace();
    } else {
        SkImageInfo cacheInfo = this->buildCacheInfo();
        return cacheInfo.refColorSpace();
    }
}

/*
 *  We have 4 ways to try to return a texture (in sorted order)
 *
 *  1. Check the cache for a pre-existing one
 *  2. Ask the generator to natively create one
 *  3. Ask the generator to return YUV planes, which the GPU can convert
 *  4. Ask the generator to return RGB(A) data, which the GPU can convert
 */
sk_sp<GrTextureProxy> SkImage_Lazy::lockTextureProxy(GrContext* ctx,
                                                     const GrUniqueKey& origKey,
                                                     SkImage::CachingHint chint,
                                                     bool willBeMipped,
                                                     SkColorSpace* dstColorSpace,
                                                     GrTextureMaker::AllowedTexGenType genType) {
    // Values representing the various texture lock paths we can take. Used for logging the path
    // taken to a histogram.
    enum LockTexturePath {
        kFailure_LockTexturePath,
        kPreExisting_LockTexturePath,
        kNative_LockTexturePath,
        kCompressed_LockTexturePath, // Deprecated
        kYUV_LockTexturePath,
        kRGBA_LockTexturePath,
    };

    enum { kLockTexturePathCount = kRGBA_LockTexturePath + 1 };

    // Build our texture key.
    // TODO: This needs to include the dstColorSpace.
    GrUniqueKey key;
    this->makeCacheKeyFromOrigKey(origKey, &key);

    GrProxyProvider* proxyProvider = ctx->contextPriv().proxyProvider();
    sk_sp<GrTextureProxy> proxy;

    // 1. Check the cache for a pre-existing one
    if (key.isValid()) {
        proxy = proxyProvider->findOrCreateProxyByUniqueKey(key, kTopLeft_GrSurfaceOrigin);
        if (proxy) {
            SK_HISTOGRAM_ENUMERATION("LockTexturePath", kPreExisting_LockTexturePath,
                                     kLockTexturePathCount);
            if (!willBeMipped || GrMipMapped::kYes == proxy->mipMapped()) {
                return proxy;
            }
        }
    }

    // What format are we going to ask the generator to create?
    // TODO: Based on the dstColorSpace?
    const SkImageInfo cacheInfo = this->buildCacheInfo();

    // 2. Ask the generator to natively create one
    if (!proxy) {
        ScopedGenerator generator(fSharedGenerator);
        if (GrTextureMaker::AllowedTexGenType::kCheap == genType &&
                SkImageGenerator::TexGenType::kCheap != generator->onCanGenerateTexture()) {
            return nullptr;
        }
        if ((proxy = generator->generateTexture(ctx, cacheInfo, fOrigin, willBeMipped))) {
            SK_HISTOGRAM_ENUMERATION("LockTexturePath", kNative_LockTexturePath,
                                     kLockTexturePathCount);
            set_key_on_proxy(proxyProvider, proxy.get(), nullptr, key);
            if (!willBeMipped || GrMipMapped::kYes == proxy->mipMapped()) {
                return proxy;
            }
        }
    }

    // 3. Ask the generator to return YUV planes, which the GPU can convert. If we will be mipping
    //    the texture we fall through here and have the CPU generate the mip maps for us.
    if (!proxy && !willBeMipped && !ctx->contextPriv().disableGpuYUVConversion()) {
        const GrSurfaceDesc desc = GrImageInfoToSurfaceDesc(cacheInfo);
        ScopedGenerator generator(fSharedGenerator);
        Generator_GrYUVProvider provider(generator);

        // The pixels in the texture will be in the generator's color space. If onMakeColorSpace
        // has been called then this will not match this image's color space. To correct this, apply
        // a color space conversion from the generator's color space to this image's color space.
        // Note that we can only do this conversion (on the GPU) if both color spaces are XYZ type.
        SkColorSpace* generatorColorSpace = fSharedGenerator->fGenerator->getInfo().colorSpace();
        SkColorSpace* thisColorSpace = fInfo.colorSpace();

        if ((!generatorColorSpace || generatorColorSpace->toXYZD50()) &&
             (!thisColorSpace || thisColorSpace->toXYZD50())) {
            // TODO: Update to create the mipped surface in the YUV generator and draw the base
            // layer directly into the mipped surface.
            proxy = provider.refAsTextureProxy(ctx, desc, generatorColorSpace, thisColorSpace);
            if (proxy) {
                SK_HISTOGRAM_ENUMERATION("LockTexturePath", kYUV_LockTexturePath,
                                         kLockTexturePathCount);
                set_key_on_proxy(proxyProvider, proxy.get(), nullptr, key);
                return proxy;
            }
        }
    }

    // 4. Ask the generator to return RGB(A) data, which the GPU can convert
    SkBitmap bitmap;
    if (!proxy && this->lockAsBitmap(&bitmap, chint, cacheInfo)) {
        if (willBeMipped) {
            proxy = proxyProvider->createMipMapProxyFromBitmap(bitmap);
        }
        if (!proxy) {
            proxy = GrUploadBitmapToTextureProxy(proxyProvider, bitmap);
        }
        if (proxy && (!willBeMipped || GrMipMapped::kYes == proxy->mipMapped())) {
            SK_HISTOGRAM_ENUMERATION("LockTexturePath", kRGBA_LockTexturePath,
                                     kLockTexturePathCount);
            set_key_on_proxy(proxyProvider, proxy.get(), nullptr, key);
            return proxy;
        }
    }

    if (proxy) {
        // We need a mipped proxy, but we either found a proxy earlier that wasn't mipped, generated
        // a native non mipped proxy, or generated a non-mipped yuv proxy. Thus we generate a new
        // mipped surface and copy the original proxy into the base layer. We will then let the gpu
        // generate the rest of the mips.
        SkASSERT(willBeMipped);
        SkASSERT(GrMipMapped::kNo == proxy->mipMapped());
        if (auto mippedProxy = GrCopyBaseMipMapToTextureProxy(ctx, proxy.get())) {
            set_key_on_proxy(proxyProvider, mippedProxy.get(), proxy.get(), key);
            return mippedProxy;
        }
        // We failed to make a mipped proxy with the base copied into it. This could have
        // been from failure to make the proxy or failure to do the copy. Thus we will fall
        // back to just using the non mipped proxy; See skbug.com/7094.
        return proxy;
    }

    SK_HISTOGRAM_ENUMERATION("LockTexturePath", kFailure_LockTexturePath,
                             kLockTexturePathCount);
    return nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif
