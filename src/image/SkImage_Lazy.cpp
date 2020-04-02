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
#include "include/private/GrRecordingContext.h"
#include "include/private/GrResourceKey.h"
#include "src/gpu/GrBitmapTextureMaker.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrGpuResourcePriv.h"
#include "src/gpu/GrImageTextureMaker.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrSamplerState.h"
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

    fInfo   = info.makeDimensions(subset->size());
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
GrSurfaceProxyView SkImage_Lazy::refView(GrRecordingContext* context, GrMipMapped mipMapped) const {
    if (!context) {
        return {};
    }

    GrImageTextureMaker textureMaker(context, this, GrImageTexGenPolicy::kDraw);
    return textureMaker.view(mipMapped);
}
#endif

sk_sp<SkImage> SkImage_Lazy::onMakeSubset(GrRecordingContext* context,
                                          const SkIRect& subset) const {
    SkASSERT(this->bounds().contains(subset));
    SkASSERT(this->bounds() != subset);

    const SkIRect generatorSubset = subset.makeOffset(fOrigin);
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

sk_sp<SkImage> SkImage_Lazy::onReinterpretColorSpace(sk_sp<SkColorSpace> newCS) const {
    // TODO: The correct thing is to clone the generator, and modify its color space. That's hard,
    // because we don't have a clone method, and generator is public (and derived-from by clients).
    // So do the simple/inefficient thing here, and fallback to raster when this is called.

    // We allocate the bitmap with the new color space, then generate the image using the original.
    SkBitmap bitmap;
    if (bitmap.tryAllocPixels(this->imageInfo().makeColorSpace(std::move(newCS)))) {
        SkPixmap pixmap = bitmap.pixmap();
        pixmap.setColorSpace(this->refColorSpace());
        if (generate_pixels(ScopedGenerator(fSharedGenerator), pixmap, fOrigin.x(), fOrigin.y())) {
            bitmap.setImmutable();
            return SkImage::MakeFromBitmap(bitmap);
        }
    }
    return nullptr;
}

sk_sp<SkImage> SkImage::MakeFromGenerator(std::unique_ptr<SkImageGenerator> generator,
                                          const SkIRect* subset) {
    SkImage_Lazy::Validator
            validator(SharedGenerator::Make(std::move(generator)), subset, nullptr, nullptr);

    return validator ? sk_make_sp<SkImage_Lazy>(&validator) : nullptr;
}

sk_sp<SkImage> SkImage::DecodeToRaster(const void* encoded, size_t length, const SkIRect* subset) {
    // The generator will not outlive this function, so we can wrap the encoded data without copy
    auto gen = SkImageGenerator::MakeFromEncoded(SkData::MakeWithoutCopy(encoded, length));
    if (!gen) {
        return nullptr;
    }
    SkImageInfo info = gen->getInfo();
    if (info.isEmpty()) {
        return nullptr;
    }

    SkIPoint origin = {0, 0};
    if (subset) {
        if (!SkIRect::MakeWH(info.width(), info.height()).contains(*subset)) {
            return nullptr;
        }
        info = info.makeDimensions(subset->size());
        origin = {subset->x(), subset->y()};
    }

    size_t rb = info.minRowBytes();
    if (rb == 0) {
        return nullptr; // rb was too big
    }
    size_t size = info.computeByteSize(rb);
    if (size == SIZE_MAX) {
        return nullptr;
    }
    auto data = SkData::MakeUninitialized(size);

    SkPixmap pmap(info, data->writable_data(), rb);
    if (!generate_pixels(gen.get(), pmap, origin.x(), origin.y())) {
        return nullptr;
    }

    return SkImage::MakeRasterData(info, data, rb);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU

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
GrSurfaceProxyView SkImage_Lazy::lockTextureProxyView(GrRecordingContext* ctx,
                                                      GrImageTexGenPolicy texGenPolicy,
                                                      GrMipMapped mipMapped) const {
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

    GrUniqueKey key;
    if (texGenPolicy == GrImageTexGenPolicy::kDraw) {
        GrMakeKeyFromImageID(&key, this->uniqueID(), SkIRect::MakeSize(this->dimensions()));
    }

    const GrCaps* caps = ctx->priv().caps();
    GrProxyProvider* proxyProvider = ctx->priv().proxyProvider();

    auto installKey = [&](const GrSurfaceProxyView& view) {
        SkASSERT(view && view.asTextureProxy());
        if (key.isValid()) {
            auto listener = GrMakeUniqueKeyInvalidationListener(&key, ctx->priv().contextID());
            this->addUniqueIDListener(std::move(listener));
            proxyProvider->assignUniqueKeyToProxy(key, view.asTextureProxy());
        }
    };

    auto ct = this->colorTypeOfLockTextureProxy(caps);

    // 1. Check the cache for a pre-existing one.
    if (key.isValid()) {
        auto proxy = proxyProvider->findOrCreateProxyByUniqueKey(key);
        if (proxy) {
            SK_HISTOGRAM_ENUMERATION("LockTexturePath", kPreExisting_LockTexturePath,
                                     kLockTexturePathCount);
            GrSwizzle swizzle = caps->getReadSwizzle(proxy->backendFormat(), ct);
            GrSurfaceProxyView view(std::move(proxy), kTopLeft_GrSurfaceOrigin, swizzle);
            if (mipMapped == GrMipMapped::kNo ||
                view.asTextureProxy()->mipMapped() == GrMipMapped::kYes) {
                return view;
            } else {
                // We need a mipped proxy, but we found a cached proxy that wasn't mipped. Thus we
                // generate a new mipped surface and copy the original proxy into the base layer. We
                // will then let the gpu generate the rest of the mips.
                auto mippedView = GrCopyBaseMipMapToView(ctx, view);
                if (!mippedView) {
                    // We failed to make a mipped proxy with the base copied into it. This could
                    // have been from failure to make the proxy or failure to do the copy. Thus we
                    // will fall back to just using the non mipped proxy; See skbug.com/7094.
                    return view;
                }
                proxyProvider->removeUniqueKeyFromProxy(view.asTextureProxy());
                installKey(mippedView);
                return mippedView;
            }
        }
    }

    // 2. Ask the generator to natively create one.
    {
        ScopedGenerator generator(fSharedGenerator);
        if (auto view = generator->generateTexture(ctx, this->imageInfo(), fOrigin, mipMapped,
                                                   texGenPolicy)) {
            SK_HISTOGRAM_ENUMERATION("LockTexturePath", kNative_LockTexturePath,
                                     kLockTexturePathCount);
            installKey(view);
            return view;
        }
    }

    // 3. Ask the generator to return YUV planes, which the GPU can convert. If we will be mipping
    //    the texture we skip this step so the CPU generate non-planar MIP maps for us.
    if (mipMapped == GrMipMapped::kNo && !ctx->priv().options().fDisableGpuYUVConversion) {
        SkColorType colorType = this->colorType();

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
        SkBudgeted budgeted = texGenPolicy == GrImageTexGenPolicy::kNew_Uncached_Unbudgeted
                                      ? SkBudgeted::kNo
                                      : SkBudgeted::kYes;
        auto view = provider.refAsTextureProxyView(ctx, this->imageInfo().dimensions(),
                                                   SkColorTypeToGrColorType(colorType),
                                                   generatorColorSpace, thisColorSpace, budgeted);
        if (view) {
            SK_HISTOGRAM_ENUMERATION("LockTexturePath", kYUV_LockTexturePath,
                                     kLockTexturePathCount);
            installKey(view);
            return view;
        }
    }

    // 4. Ask the generator to return a bitmap, which the GPU can convert.
    auto hint = texGenPolicy == GrImageTexGenPolicy::kDraw ? CachingHint::kAllow_CachingHint
                                                           : CachingHint::kDisallow_CachingHint;
    if (SkBitmap bitmap; this->getROPixels(&bitmap, hint)) {
        // We always pass uncached here because we will cache it external to the maker based on
        // *our* cache policy. We're just using the maker to generate the texture.
        auto makerPolicy = texGenPolicy == GrImageTexGenPolicy::kNew_Uncached_Unbudgeted
                                   ? GrImageTexGenPolicy::kNew_Uncached_Unbudgeted
                                   : GrImageTexGenPolicy::kNew_Uncached_Budgeted;
        GrBitmapTextureMaker bitmapMaker(ctx, bitmap, makerPolicy);
        auto view = bitmapMaker.view(mipMapped);
        if (view) {
            installKey(view);
            SK_HISTOGRAM_ENUMERATION("LockTexturePath", kRGBA_LockTexturePath,
                                     kLockTexturePathCount);
            return view;
        }
    }

    SK_HISTOGRAM_ENUMERATION("LockTexturePath", kFailure_LockTexturePath, kLockTexturePathCount);
    return {};
}

GrColorType SkImage_Lazy::colorTypeOfLockTextureProxy(const GrCaps* caps) const {
    GrColorType ct = SkColorTypeToGrColorType(this->colorType());
    GrBackendFormat format = caps->getDefaultBackendFormat(ct, GrRenderable::kNo);
    if (!format.isValid()) {
        ct = GrColorType::kRGBA_8888;
    }
    return ct;
}

#if SK_SUPPORT_GPU
void SkImage_Lazy::addUniqueIDListener(sk_sp<SkIDChangeListener> listener) const {
    bool singleThreaded = this->unique();
    fUniqueIDListeners.add(std::move(listener), singleThreaded);
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkImage> SkImage::DecodeToTexture(GrContext* ctx, const void* encoded, size_t length,
                                        const SkIRect* subset) {
    // img will not survive this function, so we don't need to copy/own the encoded data,
    auto img = MakeFromEncoded(SkData::MakeWithoutCopy(encoded, length), subset);
    if (!img) {
        return nullptr;
    }
    return img->makeTextureImage(ctx);
}

#endif
