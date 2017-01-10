/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkBitmapCache.h"
#include "SkColorSpace_Base.h"
#include "SkConservativeInfo.h"
#include "SkImage_Base.h"
#include "SkImageCacherator.h"
#include "SkMallocPixelRef.h"
#include "SkNextID.h"
#include "SkPixelRef.h"
#include "SkResourceCache.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "GrGpuResourcePriv.h"
#include "GrImageTextureMaker.h"
#include "GrResourceKey.h"
#include "GrSamplerParams.h"
#include "GrYUVProvider.h"
#include "SkGr.h"
#include "SkGrPriv.h"
#endif

// Until we actually have codecs/etc. that can contain/support a GPU texture format
// skip this step, since for some generators, returning their encoded data as a SkData
// can be somewhat expensive, and this call doesn't indicate to the generator that we're
// only interested in GPU datas...
// see skbug.com/ 4971, 5128, ...
//#define SK_SUPPORT_COMPRESSED_TEXTURES_IN_CACHERATOR

// Helper for exclusive access to a shared generator.
class SkImageCacherator::ScopedGenerator {
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

SkImageCacherator::Validator::Validator(sk_sp<SharedGenerator> gen, const SkIRect* subset)
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

    // If the encoded data is in a strange color space (it's not an XYZ matrix space), we won't be
    // able to preserve the gamut of the encoded data when we decode it. Instead, we'll have to
    // decode to a known color space (linear sRGB is a good choice). But we need to adjust the
    // stored color space, because drawing code will ask the SkImage for its color space, which
    // will in turn ask the cacherator. If we return the A2B color space, then we will be unable to
    // construct a source-to-dest gamut transformation matrix.
    if (fInfo.colorSpace() &&
        SkColorSpace_Base::Type::kXYZ != as_CSB(fInfo.colorSpace())->type()) {
        fInfo = fInfo.makeColorSpace(SkColorSpace::MakeNamed(SkColorSpace::kSRGBLinear_Named));
    }
}

SkImageCacherator* SkImageCacherator::NewFromGenerator(SkImageGenerator* gen,
                                                       const SkIRect* subset) {
    Validator validator(SharedGenerator::Make(gen), subset);

    return validator ? new SkImageCacherator(&validator) : nullptr;
}

SkImageCacherator::SkImageCacherator(Validator* validator)
    : fSharedGenerator(std::move(validator->fSharedGenerator)) // we take ownership
    , fInfo(validator->fInfo)
    , fOrigin(validator->fOrigin)
{
    fUniqueIDs[SkConservativeInfo::kLegacy_Format] = validator->fUniqueID;
    for (int i = 1; i < SkConservativeInfo::kNumFormats; ++i) {
        // We lazily allocate IDs for non-default caching cases
        fUniqueIDs[i] = kNeedNewImageUniqueID;
    }
    SkASSERT(fSharedGenerator);
}

SkImageCacherator::~SkImageCacherator() {}

SkData* SkImageCacherator::refEncoded(GrContext* ctx) {
    ScopedGenerator generator(fSharedGenerator);
    return generator->refEncodedData(ctx);
}

static bool check_output_bitmap(const SkBitmap& bitmap, uint32_t expectedID) {
    SkASSERT(bitmap.getGenerationID() == expectedID);
    SkASSERT(bitmap.isImmutable());
    SkASSERT(bitmap.getPixels());
    return true;
}

// Note, this returns a new, mutable, bitmap, with a new genID.
// If you want the immutable bitmap with the same ID as our cacherator, call tryLockAsBitmap()
//
bool SkImageCacherator::generateBitmap(SkBitmap* bitmap, const SkImageInfo& decodeInfo) {
    SkBitmap::Allocator* allocator = SkResourceCache::GetAllocator();

    ScopedGenerator generator(fSharedGenerator);
    const SkImageInfo& genInfo = generator->getInfo();
    if (decodeInfo.dimensions() == genInfo.dimensions()) {
        SkASSERT(fOrigin.x() == 0 && fOrigin.y() == 0);
        // fast-case, no copy needed
        return generator->tryGenerateBitmap(bitmap, decodeInfo, allocator);
    } else {
        // need to handle subsetting, so we first generate the full size version, and then
        // "read" from it to get our subset. See https://bug.skia.org/4213

        SkBitmap full;
        if (!generator->tryGenerateBitmap(&full,
                                          decodeInfo.makeWH(genInfo.width(), genInfo.height()),
                                          allocator)) {
            return false;
        }
        if (!bitmap->tryAllocPixels(decodeInfo, nullptr, full.getColorTable())) {
            return false;
        }
        return full.readPixels(bitmap->info(), bitmap->getPixels(), bitmap->rowBytes(),
                               fOrigin.x(), fOrigin.y());
    }
}

bool SkImageCacherator::directGeneratePixels(const SkImageInfo& info, void* pixels, size_t rb,
                                             int srcX, int srcY) {
    ScopedGenerator generator(fSharedGenerator);
    const SkImageInfo& genInfo = generator->getInfo();
    // Currently generators do not natively handle subsets, so check that first.
    if (srcX || srcY || genInfo.width() != info.width() || genInfo.height() != info.height()) {
        return false;
    }
    return generator->getPixels(info, pixels, rb);
}

bool SkImageCacherator::directAccessScaledImage(const SkRect& srcRect,
                                                const SkMatrix& totalMatrix,
                                                SkFilterQuality fq,
                                                SkImageGenerator::ScaledImageRec* rec) {
    return ScopedGenerator(fSharedGenerator)->accessScaledImage(srcRect, totalMatrix, fq, rec);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

bool SkImageCacherator::lockAsBitmapOnlyIfAlreadyCached(SkBitmap* bitmap,
                                                        SkConservativeInfo::Format format) {
    return kNeedNewImageUniqueID != fUniqueIDs[format] &&
        SkBitmapCache::Find(fUniqueIDs[format], bitmap) &&
        check_output_bitmap(*bitmap, fUniqueIDs[format]);
}

bool SkImageCacherator::tryLockAsBitmap(SkBitmap* bitmap, const SkImage* client,
                                        SkImage::CachingHint chint,
                                        SkConservativeInfo::Format format,
                                        const SkImageInfo& info) {
    if (this->lockAsBitmapOnlyIfAlreadyCached(bitmap, format)) {
        return true;
    }
    if (!this->generateBitmap(bitmap, info)) {
        return false;
    }

    if (kNeedNewImageUniqueID == fUniqueIDs[format]) {
        fUniqueIDs[format] = SkNextID::ImageID();
    }
    bitmap->pixelRef()->setImmutableWithID(fUniqueIDs[format]);
    if (SkImage::kAllow_CachingHint == chint) {
        SkBitmapCache::Add(fUniqueIDs[format], *bitmap);
        if (client) {
            as_IB(client)->notifyAddedToCache();
        }
    }
    return true;
}

bool SkImageCacherator::lockAsBitmap(SkBitmap* bitmap, const SkImage* client,
                                     SkColorSpace* dstColorSpace,
                                     SkImage::CachingHint chint) {
    SkConservativeInfo::Format format =
            SkConservativeInfo::ChooseFormat(fInfo, dstColorSpace, nullptr);
    SkImageInfo cacheInfo = SkConservativeInfo::Make(fInfo, format);

    if (kNeedNewImageUniqueID == fUniqueIDs[format]) {
        fUniqueIDs[format] = SkNextID::ImageID();
    }

    if (this->tryLockAsBitmap(bitmap, client, chint, format, cacheInfo)) {
        return check_output_bitmap(*bitmap, fUniqueIDs[format]);
    }

#if SK_SUPPORT_GPU
    // Try to get a texture and read it back to raster (and then cache that with our ID)
    sk_sp<GrTexture> tex;

    {
        ScopedGenerator generator(fSharedGenerator);
        tex.reset(generator->generateTexture(nullptr, cacheInfo, fOrigin));
    }
    if (!tex) {
        bitmap->reset();
        return false;
    }

    if (!bitmap->tryAllocPixels(cacheInfo)) {
        bitmap->reset();
        return false;
    }

    const uint32_t pixelOpsFlags = 0;
    if (!tex->readPixels(fInfo.colorSpace(), 0, 0, bitmap->width(), bitmap->height(),
                         SkImageInfo2GrPixelConfig(cacheInfo, *tex->getContext()->caps()),
                         cacheInfo.colorSpace(), bitmap->getPixels(), bitmap->rowBytes(),
                         pixelOpsFlags)) {
        bitmap->reset();
        return false;
    }

    bitmap->pixelRef()->setImmutableWithID(fUniqueIDs[format]);
    if (SkImage::kAllow_CachingHint == chint) {
        SkBitmapCache::Add(fUniqueIDs[format], *bitmap);
        if (client) {
            as_IB(client)->notifyAddedToCache();
        }
    }
    return check_output_bitmap(*bitmap, fUniqueIDs[format]);
#else
    return false;
#endif
}

//////////////////////////////////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU

void SkImageCacherator::makeCacheKeyFromOrigKey(const GrUniqueKey& origKey,
                                                SkConservativeInfo::Format format,
                                                GrUniqueKey* cacheKey) {
    SkASSERT(!cacheKey->isValid());
    if (origKey.isValid()) {
        static const GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();
        GrUniqueKey::Builder builder(cacheKey, origKey, kDomain, 1);
        builder[0] = format;
    }
}

#ifdef SK_SUPPORT_COMPRESSED_TEXTURES_IN_CACHERATOR
static GrTexture* load_compressed_into_texture(GrContext* ctx, SkData* data, GrSurfaceDesc desc) {
    const void* rawStart;
    GrPixelConfig config = GrIsCompressedTextureDataSupported(ctx, data, desc.fWidth, desc.fHeight,
                                                              &rawStart);
    if (kUnknown_GrPixelConfig == config) {
        return nullptr;
    }

    desc.fConfig = config;
    return ctx->textureProvider()->createTexture(desc, SkBudgeted::kYes, rawStart, 0);
}
#endif

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

static GrTexture* set_key_and_return(GrTexture* tex, const GrUniqueKey& key) {
    if (key.isValid()) {
        tex->resourcePriv().setUniqueKey(key);
    }
    return tex;
}

sk_sp<SkColorSpace> SkImageCacherator::getColorSpace(GrContext* ctx, SkColorSpace* dstColorSpace) {
    // TODO: This isn't always correct. Picture generator currently produces textures in N32,
    // and will (soon) emit them in an arbitrary (destination) space. We will need to stash that
    // information in/on the key so we can return the correct space in case #1 of lockTexture.
    SkImageInfo cacheInfo = SkConservativeInfo::Make(fInfo, dstColorSpace, ctx->caps());
    return sk_ref_sp(cacheInfo.colorSpace());
}

/*
 *  We have a 5 ways to try to return a texture (in sorted order)
 *
 *  1. Check the cache for a pre-existing one
 *  2. Ask the generator to natively create one
 *  3. Ask the generator to return a compressed form that the GPU might support
 *  4. Ask the generator to return YUV planes, which the GPU can convert
 *  5. Ask the generator to return RGB(A) data, which the GPU can convert
 */
GrTexture* SkImageCacherator::lockTexture(GrContext* ctx, const GrUniqueKey& origKey,
                                          const SkImage* client, SkImage::CachingHint chint,
                                          bool willBeMipped, SkColorSpace* dstColorSpace) {
    // Values representing the various texture lock paths we can take. Used for logging the path
    // taken to a histogram.
    enum LockTexturePath {
        kFailure_LockTexturePath,
        kPreExisting_LockTexturePath,
        kNative_LockTexturePath,
        kCompressed_LockTexturePath,
        kYUV_LockTexturePath,
        kRGBA_LockTexturePath,
    };

    enum { kLockTexturePathCount = kRGBA_LockTexturePath + 1 };

    // Determine which cached format we're going to use (which may involve decoding to a different
    // info than the generator provides).
    SkConservativeInfo::Format format =
            SkConservativeInfo::ChooseFormat(fInfo, dstColorSpace, ctx->caps());

    // Fold the cache format into our texture key
    GrUniqueKey key;
    this->makeCacheKeyFromOrigKey(origKey, format, &key);

    // 1. Check the cache for a pre-existing one
    if (key.isValid()) {
        if (GrTexture* tex = ctx->textureProvider()->findAndRefTextureByUniqueKey(key)) {
            SK_HISTOGRAM_ENUMERATION("LockTexturePath", kPreExisting_LockTexturePath,
                                     kLockTexturePathCount);
            return tex;
        }
    }

    // The CachedFormat is both an index for which cache "slot" we'll use to store this particular
    // decoded variant of the encoded data, and also a recipe for how to transform the original
    // info to get the one that we're going to decode to.
    SkImageInfo cacheInfo = SkConservativeInfo::Make(fInfo, format);

    // 2. Ask the generator to natively create one
    {
        ScopedGenerator generator(fSharedGenerator);
        if (GrTexture* tex = generator->generateTexture(ctx, cacheInfo, fOrigin)) {
            SK_HISTOGRAM_ENUMERATION("LockTexturePath", kNative_LockTexturePath,
                                     kLockTexturePathCount);
            return set_key_and_return(tex, key);
        }
    }

    const GrSurfaceDesc desc = GrImageInfoToSurfaceDesc(cacheInfo, *ctx->caps());

#ifdef SK_SUPPORT_COMPRESSED_TEXTURES_IN_CACHERATOR
    // 3. Ask the generator to return a compressed form that the GPU might support
    sk_sp<SkData> data(this->refEncoded(ctx));
    if (data) {
        GrTexture* tex = load_compressed_into_texture(ctx, data, desc);
        if (tex) {
            SK_HISTOGRAM_ENUMERATION("LockTexturePath", kCompressed_LockTexturePath,
                                     kLockTexturePathCount);
            return set_key_and_return(tex, key);
        }
    }
#endif

    // 4. Ask the generator to return YUV planes, which the GPU can convert
    {
        ScopedGenerator generator(fSharedGenerator);
        Generator_GrYUVProvider provider(generator);
        sk_sp<GrTexture> tex = provider.refAsTexture(ctx, desc, true);
        if (tex) {
            SK_HISTOGRAM_ENUMERATION("LockTexturePath", kYUV_LockTexturePath,
                                     kLockTexturePathCount);
            return set_key_and_return(tex.release(), key);
        }
    }

    // 5. Ask the generator to return RGB(A) data, which the GPU can convert
    SkBitmap bitmap;
    if (this->tryLockAsBitmap(&bitmap, client, chint, format, cacheInfo)) {
        GrTexture* tex = nullptr;
        if (willBeMipped) {
            tex = GrGenerateMipMapsAndUploadToTexture(ctx, bitmap, dstColorSpace);
        }
        if (!tex) {
            tex = GrUploadBitmapToTexture(ctx, bitmap);
        }
        if (tex) {
            SK_HISTOGRAM_ENUMERATION("LockTexturePath", kRGBA_LockTexturePath,
                                     kLockTexturePathCount);
            return set_key_and_return(tex, key);
        }
    }
    SK_HISTOGRAM_ENUMERATION("LockTexturePath", kFailure_LockTexturePath,
                             kLockTexturePathCount);
    return nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

GrTexture* SkImageCacherator::lockAsTexture(GrContext* ctx, const GrSamplerParams& params,
                                            SkColorSpace* dstColorSpace,
                                            sk_sp<SkColorSpace>* texColorSpace,
                                            const SkImage* client, SkImage::CachingHint chint) {
    if (!ctx) {
        return nullptr;
    }

    return GrImageTextureMaker(ctx, this, client, chint).refTextureForParams(params, dstColorSpace,
                                                                             texColorSpace);
}

#else

GrTexture* SkImageCacherator::lockAsTexture(GrContext* ctx, const GrSamplerParams&,
                                            SkColorSpace* dstColorSpace,
                                            sk_sp<SkColorSpace>* texColorSpace,
                                            const SkImage* client, SkImage::CachingHint) {
    return nullptr;
}

#endif
