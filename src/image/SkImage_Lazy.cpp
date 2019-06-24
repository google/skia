/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/image/SkImage_Lazy.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkData.h"
#include "include/core/SkImageGenerator.h"
#include "src/core/SkBitmapCache.h"
#include "src/core/SkCachedData.h"
#include "src/core/SkImagePriv.h"
#include "src/core/SkNextID.h"

#if SK_SUPPORT_GPU
#include "include/gpu/GrSamplerState.h"
#include "include/private/GrRecordingContext.h"
#include "include/private/GrResourceKey.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrGpuResourcePriv.h"
#include "src/gpu/GrImageTextureMaker.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrYUVProvider.h"
#include "src/gpu/SkGr.h"
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

///////////////////////////////////////////////////////////////////////////////

SkImage_Lazy::Validator::Validator(sk_sp<SharedGenerator> gen, const SkIRect* subset,
                                   const SkColorType* colorType, sk_sp<SkColorSpace> colorSpace)
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
    if (colorType || colorSpace) {
        if (colorType) {
            fInfo = fInfo.makeColorType(*colorType);
        }
        if (colorSpace) {
            fInfo = fInfo.makeColorSpace(colorSpace);
        }
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
    SkAutoMutexExclusive          fAutoAquire;
};

///////////////////////////////////////////////////////////////////////////////

SkImage_Lazy::SkImage_Lazy(Validator* validator)
        : INHERITED(validator->fInfo, validator->fUniqueID)
        , fSharedGenerator(std::move(validator->fSharedGenerator))
        , fOrigin(validator->fOrigin) {
    SkASSERT(fSharedGenerator);
    fUniqueID = validator->fUniqueID;
}

SkImage_Lazy::~SkImage_Lazy() {
#if SK_SUPPORT_GPU
    for (int i = 0; i < fUniqueKeyInvalidatedMessages.count(); ++i) {
        SkMessageBus<GrUniqueKeyInvalidatedMessage>::Post(*fUniqueKeyInvalidatedMessages[i]);
    }
    fUniqueKeyInvalidatedMessages.deleteAll();
#endif
}

//////////////////////////////////////////////////////////////////////////////////////////////////

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

bool SkImage_Lazy::getROPixels(SkBitmap* bitmap, SkImage::CachingHint chint) const {
    auto check_output_bitmap = [bitmap]() {
        SkASSERT(bitmap->isImmutable());
        SkASSERT(bitmap->getPixels());
        (void)bitmap;
    };

    auto desc = SkBitmapCacheDesc::Make(this);
    if (SkBitmapCache::Find(desc, bitmap)) {
        check_output_bitmap();
        return true;
    }

    if (SkImage::kAllow_CachingHint == chint) {
        SkPixmap pmap;
        SkBitmapCache::RecPtr cacheRec = SkBitmapCache::Alloc(desc, this->imageInfo(), &pmap);
        if (!cacheRec ||
            !generate_pixels(ScopedGenerator(fSharedGenerator), pmap,
                             fOrigin.x(), fOrigin.y())) {
            return false;
        }
        SkBitmapCache::Add(std::move(cacheRec), bitmap);
        this->notifyAddedToRasterCache();
    } else {
        if (!bitmap->tryAllocPixels(this->imageInfo()) ||
            !generate_pixels(ScopedGenerator(fSharedGenerator), bitmap->pixmap(), fOrigin.x(),
                             fOrigin.y())) {
            return false;
        }
        bitmap->setImmutable();
    }

    check_output_bitmap();
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

bool SkImage_Lazy::onReadPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRB,
                                int srcX, int srcY, CachingHint chint) const {
    SkBitmap bm;
    if (this->getROPixels(&bm, chint)) {
        return bm.readPixels(dstInfo, dstPixels, dstRB, srcX, srcY);
    }
    return false;
}

sk_sp<SkData> SkImage_Lazy::onRefEncoded() const {
    ScopedGenerator generator(fSharedGenerator);
    return generator->refEncodedData();
}

bool SkImage_Lazy::onIsValid(GrContext* context) const {
    ScopedGenerator generator(fSharedGenerator);
    return generator->isValid(context);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU
sk_sp<GrTextureProxy> SkImage_Lazy::asTextureProxyRef(GrRecordingContext* context,
                                                      const GrSamplerState& params,
                                                      SkScalar scaleAdjust[2]) const {
    if (!context) {
        return nullptr;
    }

    GrImageTextureMaker textureMaker(context, this, kAllow_CachingHint);
    return textureMaker.refTextureProxyForParams(params, scaleAdjust);
}
#endif

sk_sp<SkImage> SkImage_Lazy::onMakeSubset(GrRecordingContext* context,
                                          const SkIRect& subset) const {
    SkASSERT(this->bounds().contains(subset));
    SkASSERT(this->bounds() != subset);

    const SkIRect generatorSubset = subset.makeOffset(fOrigin.x(), fOrigin.y());
    const SkColorType colorType = this->colorType();
    Validator validator(fSharedGenerator, &generatorSubset, &colorType, this->refColorSpace());
    return validator ? sk_sp<SkImage>(new SkImage_Lazy(&validator)) : nullptr;
}

sk_sp<SkImage> SkImage_Lazy::onMakeColorTypeAndColorSpace(GrRecordingContext*,
                                                          SkColorType targetCT,
                                                          sk_sp<SkColorSpace> targetCS) const {
    SkAutoMutexExclusive autoAquire(fOnMakeColorTypeAndSpaceMutex);
    if (fOnMakeColorTypeAndSpaceResult &&
        targetCT == fOnMakeColorTypeAndSpaceResult->colorType() &&
        SkColorSpace::Equals(targetCS.get(), fOnMakeColorTypeAndSpaceResult->colorSpace())) {
        return fOnMakeColorTypeAndSpaceResult;
    }
    const SkIRect generatorSubset =
            SkIRect::MakeXYWH(fOrigin.x(), fOrigin.y(), this->width(), this->height());
    Validator validator(fSharedGenerator, &generatorSubset, &targetCT, targetCS);
    sk_sp<SkImage> result = validator ? sk_sp<SkImage>(new SkImage_Lazy(&validator)) : nullptr;
    if (result) {
        fOnMakeColorTypeAndSpaceResult = result;
    }
    return result;
}

sk_sp<SkImage> SkImage::MakeFromGenerator(std::unique_ptr<SkImageGenerator> generator,
                                          const SkIRect* subset) {
    SkImage_Lazy::Validator
            validator(SharedGenerator::Make(std::move(generator)), subset, nullptr, nullptr);

    return validator ? sk_make_sp<SkImage_Lazy>(&validator) : nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU

void SkImage_Lazy::makeCacheKeyFromOrigKey(const GrUniqueKey& origKey,
                                           GrUniqueKey* cacheKey) const {
    SkASSERT(!cacheKey->isValid());
    if (origKey.isValid()) {
        static const GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();
        GrUniqueKey::Builder builder(cacheKey, origKey, kDomain, 0, "Image");
    }
}

class Generator_GrYUVProvider : public GrYUVProvider {
public:
    Generator_GrYUVProvider(SkImageGenerator* gen) : fGen(gen) {}

private:
    uint32_t onGetID() const override { return fGen->uniqueID(); }
    bool onQueryYUVA8(SkYUVASizeInfo* sizeInfo,
                      SkYUVAIndex yuvaIndices[SkYUVAIndex::kIndexCount],
                      SkYUVColorSpace* colorSpace) const override {
        return fGen->queryYUVA8(sizeInfo, yuvaIndices, colorSpace);
    }
    bool onGetYUVA8Planes(const SkYUVASizeInfo& sizeInfo,
                          const SkYUVAIndex yuvaIndices[SkYUVAIndex::kIndexCount],
                          void* planes[]) override {
        return fGen->getYUVA8Planes(sizeInfo, yuvaIndices, planes);
    }

    SkImageGenerator* fGen;

    typedef GrYUVProvider INHERITED;
};

static void set_key_on_proxy(GrProxyProvider* proxyProvider,
                             GrTextureProxy* proxy, GrTextureProxy* originalProxy,
                             const GrUniqueKey& key) {
    if (key.isValid()) {
        if (originalProxy && originalProxy->getUniqueKey().isValid()) {
            SkASSERT(originalProxy->getUniqueKey() == key);
            SkASSERT(GrMipMapped::kYes == proxy->mipMapped() &&
                     GrMipMapped::kNo == originalProxy->mipMapped());
            // If we had an originalProxy with a valid key, that means there already is a proxy in
            // the cache which matches the key, but it does not have mip levels and we require them.
            // Thus we must remove the unique key from that proxy.
            SkASSERT(originalProxy->getUniqueKey() == key);
            proxyProvider->removeUniqueKeyFromProxy(originalProxy);
        }
        proxyProvider->assignUniqueKeyToProxy(key, proxy);
    }
}

sk_sp<SkCachedData> SkImage_Lazy::getPlanes(SkYUVASizeInfo* yuvaSizeInfo,
                                            SkYUVAIndex yuvaIndices[SkYUVAIndex::kIndexCount],
                                            SkYUVColorSpace* yuvColorSpace,
                                            const void* planes[SkYUVASizeInfo::kMaxCount]) {
    ScopedGenerator generator(fSharedGenerator);
    Generator_GrYUVProvider provider(generator);

    sk_sp<SkCachedData> data = provider.getPlanes(yuvaSizeInfo, yuvaIndices, yuvColorSpace, planes);
    if (!data) {
        return nullptr;
    }

    return data;
}


/*
 *  We have 4 ways to try to return a texture (in sorted order)
 *
 *  1. Check the cache for a pre-existing one
 *  2. Ask the generator to natively create one
 *  3. Ask the generator to return YUV planes, which the GPU can convert
 *  4. Ask the generator to return RGB(A) data, which the GPU can convert
 */
sk_sp<GrTextureProxy> SkImage_Lazy::lockTextureProxy(
        GrRecordingContext* ctx,
        const GrUniqueKey& origKey,
        SkImage::CachingHint chint,
        bool willBeMipped,
        GrTextureMaker::AllowedTexGenType genType) const {
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
    // Even though some proxies created here may have a specific origin and use that origin, we do
    // not include that in the key. Since SkImages are meant to be immutable, a given SkImage will
    // always have an associated proxy that is always one origin or the other. It never can change
    // origins. Thus we don't need to include that info in the key iteself.
    GrUniqueKey key;
    this->makeCacheKeyFromOrigKey(origKey, &key);

    GrProxyProvider* proxyProvider = ctx->priv().proxyProvider();
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

    // 2. Ask the generator to natively create one
    if (!proxy) {
        ScopedGenerator generator(fSharedGenerator);
        if (GrTextureMaker::AllowedTexGenType::kCheap == genType &&
                SkImageGenerator::TexGenType::kCheap != generator->onCanGenerateTexture()) {
            return nullptr;
        }
        if ((proxy = generator->generateTexture(ctx, this->imageInfo(), fOrigin, willBeMipped))) {
            SK_HISTOGRAM_ENUMERATION("LockTexturePath", kNative_LockTexturePath,
                                     kLockTexturePathCount);
            set_key_on_proxy(proxyProvider, proxy.get(), nullptr, key);
            if (!willBeMipped || GrMipMapped::kYes == proxy->mipMapped()) {
                *fUniqueKeyInvalidatedMessages.append() =
                        new GrUniqueKeyInvalidatedMessage(key, ctx->priv().contextID());
                return proxy;
            }
        }
    }

    // 3. Ask the generator to return YUV planes, which the GPU can convert. If we will be mipping
    //    the texture we fall through here and have the CPU generate the mip maps for us.
    if (!proxy && !willBeMipped && !ctx->priv().options().fDisableGpuYUVConversion) {
        const GrSurfaceDesc desc = GrImageInfoToSurfaceDesc(this->imageInfo());

        SkColorType colorType = this->colorType();
        GrBackendFormat format =
                ctx->priv().caps()->getBackendFormatFromColorType(colorType);

        ScopedGenerator generator(fSharedGenerator);
        Generator_GrYUVProvider provider(generator);

        // The pixels in the texture will be in the generator's color space.
        // If onMakeColorTypeAndColorSpace has been called then this will not match this image's
        // color space. To correct this, apply a color space conversion from the generator's color
        // space to this image's color space.
        SkColorSpace* generatorColorSpace = fSharedGenerator->fGenerator->getInfo().colorSpace();
        SkColorSpace* thisColorSpace = this->colorSpace();

        // TODO: Update to create the mipped surface in the YUV generator and draw the base
        // layer directly into the mipped surface.
        proxy = provider.refAsTextureProxy(ctx, format, desc, SkColorTypeToGrColorType(colorType),
                                           generatorColorSpace, thisColorSpace);
        if (proxy) {
            SK_HISTOGRAM_ENUMERATION("LockTexturePath", kYUV_LockTexturePath,
                                     kLockTexturePathCount);
            set_key_on_proxy(proxyProvider, proxy.get(), nullptr, key);
            *fUniqueKeyInvalidatedMessages.append() =
                    new GrUniqueKeyInvalidatedMessage(key, ctx->priv().contextID());
            return proxy;
        }
    }

    // 4. Ask the generator to return RGB(A) data, which the GPU can convert
    SkBitmap bitmap;
    if (!proxy && this->getROPixels(&bitmap, chint)) {
        proxy = proxyProvider->createProxyFromBitmap(bitmap, willBeMipped ? GrMipMapped::kYes
                                                                          : GrMipMapped::kNo);
        if (proxy && (!willBeMipped || GrMipMapped::kYes == proxy->mipMapped())) {
            SK_HISTOGRAM_ENUMERATION("LockTexturePath", kRGBA_LockTexturePath,
                                     kLockTexturePathCount);
            set_key_on_proxy(proxyProvider, proxy.get(), nullptr, key);
            *fUniqueKeyInvalidatedMessages.append() =
                    new GrUniqueKeyInvalidatedMessage(key, ctx->priv().contextID());
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
        *fUniqueKeyInvalidatedMessages.append() =
                new GrUniqueKeyInvalidatedMessage(key, ctx->priv().contextID());
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
