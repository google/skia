/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkBitmapCache.h"
#include "SkColorSpace_Base.h"
#include "SkImage_Base.h"
#include "SkImageCacherator.h"
#include "SkMallocPixelRef.h"
#include "SkNextID.h"
#include "SkPixelRef.h"
#include "SkResourceCache.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrGpuResourcePriv.h"
#include "GrImageTextureMaker.h"
#include "GrResourceKey.h"
#include "GrResourceProvider.h"
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
        fInfo = fInfo.makeColorSpace(SkColorSpace::MakeSRGBLinear());
    }
}

SkImageCacherator* SkImageCacherator::NewFromGenerator(std::unique_ptr<SkImageGenerator> gen,
                                                       const SkIRect* subset) {
    Validator validator(SharedGenerator::Make(std::move(gen)), subset);

    return validator ? new SkImageCacherator(&validator) : nullptr;
}

SkImageCacherator::SkImageCacherator(Validator* validator)
    : fSharedGenerator(std::move(validator->fSharedGenerator)) // we take ownership
    , fInfo(validator->fInfo)
    , fOrigin(validator->fOrigin)
{
    fUniqueIDs[kLegacy_CachedFormat] = validator->fUniqueID;
    for (int i = 1; i < kNumCachedFormats; ++i) {
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

//////////////////////////////////////////////////////////////////////////////////////////////////

bool SkImageCacherator::lockAsBitmapOnlyIfAlreadyCached(SkBitmap* bitmap, CachedFormat format) {
    return kNeedNewImageUniqueID != fUniqueIDs[format] &&
        SkBitmapCache::Find(fUniqueIDs[format], bitmap) &&
        check_output_bitmap(*bitmap, fUniqueIDs[format]);
}

bool SkImageCacherator::tryLockAsBitmap(SkBitmap* bitmap, const SkImage* client,
                                        SkImage::CachingHint chint, CachedFormat format,
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

bool SkImageCacherator::lockAsBitmap(GrContext* context, SkBitmap* bitmap, const SkImage* client,
                                     SkColorSpace* dstColorSpace,
                                     SkImage::CachingHint chint) {
    CachedFormat format = this->chooseCacheFormat(dstColorSpace);
    SkImageInfo cacheInfo = this->buildCacheInfo(format);

    if (kNeedNewImageUniqueID == fUniqueIDs[format]) {
        fUniqueIDs[format] = SkNextID::ImageID();
    }

    if (this->tryLockAsBitmap(bitmap, client, chint, format, cacheInfo)) {
        return check_output_bitmap(*bitmap, fUniqueIDs[format]);
    }

#if SK_SUPPORT_GPU
    if (!context) {
        bitmap->reset();
        return false;
    }

    // Try to get a texture and read it back to raster (and then cache that with our ID)
    sk_sp<GrTextureProxy> proxy;

    {
        ScopedGenerator generator(fSharedGenerator);
        proxy = generator->generateTexture(context, cacheInfo, fOrigin);
    }
    if (!proxy) {
        bitmap->reset();
        return false;
    }

    if (!bitmap->tryAllocPixels(cacheInfo)) {
        bitmap->reset();
        return false;
    }

    sk_sp<GrSurfaceContext> sContext(context->contextPriv().makeWrappedSurfaceContext(
                                                    proxy,
                                                    fInfo.refColorSpace())); // src colorSpace
    if (!sContext) {
        bitmap->reset();
        return false;
    }

    if (!sContext->readPixels(bitmap->info(), bitmap->getPixels(), bitmap->rowBytes(), 0, 0)) {
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

// Abstraction of GrCaps that handles the cases where we don't have a caps pointer (because
// we're in raster mode), or where GPU support is entirely missing. In theory, we only need the
// chosen format to be texturable, but that lets us choose F16 on GLES implemenations where we
// won't be able to read the texture back. We'd like to ensure that SkImake::makeNonTextureImage
// works, so we require that the formats we choose are renderable (as a proxy for being readable).
struct CacheCaps {
    CacheCaps(const GrCaps* caps) : fCaps(caps) {}

#if SK_SUPPORT_GPU
    bool supportsHalfFloat() const {
        return !fCaps ||
            (fCaps->isConfigTexturable(kRGBA_half_GrPixelConfig) &&
             fCaps->isConfigRenderable(kRGBA_half_GrPixelConfig, false));
    }

    bool supportsSRGB() const {
        return !fCaps ||
            (fCaps->srgbSupport() && fCaps->isConfigTexturable(kSRGBA_8888_GrPixelConfig));
    }

    bool supportsSBGR() const {
        return !fCaps || fCaps->srgbSupport();
    }
#else
    bool supportsHalfFloat() const { return true; }
    bool supportsSRGB() const { return true; }
    bool supportsSBGR() const { return true; }
#endif

    const GrCaps* fCaps;
};

SkImageCacherator::CachedFormat SkImageCacherator::chooseCacheFormat(SkColorSpace* dstColorSpace,
                                                                     const GrCaps* grCaps) {
    SkColorSpace* cs = fInfo.colorSpace();
    if (!cs || !dstColorSpace) {
        return kLegacy_CachedFormat;
    }

    CacheCaps caps(grCaps);
    switch (fInfo.colorType()) {
        case kUnknown_SkColorType:
        case kAlpha_8_SkColorType:
        case kRGB_565_SkColorType:
        case kARGB_4444_SkColorType:
            // We don't support color space on these formats, so always decode in legacy mode:
            // TODO: Ask the codec to decode these to something else (at least sRGB 8888)?
            return kLegacy_CachedFormat;

        case kIndex_8_SkColorType:
            // We can't draw from indexed textures with a color space, so ask the codec to expand
            if (cs->gammaCloseToSRGB()) {
                if (caps.supportsSRGB()) {
                    return kSRGB8888_CachedFormat;
                } else if (caps.supportsHalfFloat()) {
                    return kLinearF16_CachedFormat;
                } else {
                    return kLegacy_CachedFormat;
                }
            } else {
                if (caps.supportsHalfFloat()) {
                    return kLinearF16_CachedFormat;
                } else if (caps.supportsSRGB()) {
                    return kSRGB8888_CachedFormat;
                } else {
                    return kLegacy_CachedFormat;
                }
            }

        case kGray_8_SkColorType:
            // TODO: What do we do with grayscale sources that have strange color spaces attached?
            // The codecs and color space xform don't handle this correctly (yet), so drop it on
            // the floor. (Also, inflating by a factor of 8 is going to be unfortunate).
            // As it is, we don't directly support sRGB grayscale, so ask the codec to convert
            // it for us. This bypasses some really sketchy code GrUploadPixmapToTexture.
            if (cs->gammaCloseToSRGB() && caps.supportsSRGB()) {
                return kSRGB8888_CachedFormat;
            } else {
                return kLegacy_CachedFormat;
            }

        case kRGBA_8888_SkColorType:
            if (cs->gammaCloseToSRGB()) {
                if (caps.supportsSRGB()) {
                    return kAsIs_CachedFormat;
                } else if (caps.supportsHalfFloat()) {
                    return kLinearF16_CachedFormat;
                } else {
                    return kLegacy_CachedFormat;
                }
            } else {
                if (caps.supportsHalfFloat()) {
                    return kLinearF16_CachedFormat;
                } else if (caps.supportsSRGB()) {
                    return kSRGB8888_CachedFormat;
                } else {
                    return kLegacy_CachedFormat;
                }
            }

        case kBGRA_8888_SkColorType:
            // Odd case. sBGRA isn't a real thing, so we may not have this texturable.
            if (caps.supportsSBGR()) {
                if (cs->gammaCloseToSRGB()) {
                    return kAsIs_CachedFormat;
                } else if (caps.supportsHalfFloat()) {
                    return kLinearF16_CachedFormat;
                } else if (caps.supportsSRGB()) {
                    return kSRGB8888_CachedFormat;
                } else {
                    // sBGRA support without sRGBA is highly unlikely (impossible?) Nevertheless.
                    return kLegacy_CachedFormat;
                }
            } else {
                if (cs->gammaCloseToSRGB()) {
                    if (caps.supportsSRGB()) {
                        return kSRGB8888_CachedFormat;
                    } else if (caps.supportsHalfFloat()) {
                        return kLinearF16_CachedFormat;
                    } else {
                        return kLegacy_CachedFormat;
                    }
                } else {
                    if (caps.supportsHalfFloat()) {
                        return kLinearF16_CachedFormat;
                    } else if (caps.supportsSRGB()) {
                        return kSRGB8888_CachedFormat;
                    } else {
                        return kLegacy_CachedFormat;
                    }
                }
            }

        case kRGBA_F16_SkColorType:
            if (!caps.supportsHalfFloat()) {
                if (caps.supportsSRGB()) {
                    return kSRGB8888_CachedFormat;
                } else {
                    return kLegacy_CachedFormat;
                }
            } else if (cs->gammaIsLinear()) {
                return kAsIs_CachedFormat;
            } else {
                return kLinearF16_CachedFormat;
            }
    }
    SkDEBUGFAIL("Unreachable");
    return kLegacy_CachedFormat;
}

SkImageInfo SkImageCacherator::buildCacheInfo(CachedFormat format) {
    switch (format) {
        case kLegacy_CachedFormat:
            return fInfo.makeColorSpace(nullptr);
        case kAsIs_CachedFormat:
            return fInfo;
        case kLinearF16_CachedFormat:
            return fInfo
                .makeColorType(kRGBA_F16_SkColorType)
                .makeColorSpace(as_CSB(fInfo.colorSpace())->makeLinearGamma());
        case kSRGB8888_CachedFormat:
            return fInfo
                .makeColorType(kRGBA_8888_SkColorType)
                .makeColorSpace(as_CSB(fInfo.colorSpace())->makeSRGBGamma());
        default:
            SkDEBUGFAIL("Invalid cached format");
            return fInfo;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU

void SkImageCacherator::makeCacheKeyFromOrigKey(const GrUniqueKey& origKey, CachedFormat format,
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
    return ctx->resourceProvider()->createTexture(desc, SkBudgeted::kYes, rawStart, 0);
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

static GrTexture* set_key_and_return(GrResourceProvider* resourceProvider,
                                     GrTexture* tex, const GrUniqueKey& key) {
    if (key.isValid()) {
        resourceProvider->assignUniqueKeyToTexture(key, tex);
    }
    return tex;
}

sk_sp<SkColorSpace> SkImageCacherator::getColorSpace(GrContext* ctx, SkColorSpace* dstColorSpace) {
    // TODO: This isn't always correct. Picture generator currently produces textures in N32,
    // and will (soon) emit them in an arbitrary (destination) space. We will need to stash that
    // information in/on the key so we can return the correct space in case #1 of lockTexture.
    CachedFormat format = this->chooseCacheFormat(dstColorSpace, ctx->caps());
    SkImageInfo cacheInfo = this->buildCacheInfo(format);
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
    CachedFormat format = this->chooseCacheFormat(dstColorSpace, ctx->caps());

    // Fold the cache format into our texture key
    GrUniqueKey key;
    this->makeCacheKeyFromOrigKey(origKey, format, &key);

    // 1. Check the cache for a pre-existing one
    if (key.isValid()) {
        if (GrTexture* tex = ctx->resourceProvider()->findAndRefTextureByUniqueKey(key)) {
            SK_HISTOGRAM_ENUMERATION("LockTexturePath", kPreExisting_LockTexturePath,
                                     kLockTexturePathCount);
            return tex;
        }
    }

    // The CachedFormat is both an index for which cache "slot" we'll use to store this particular
    // decoded variant of the encoded data, and also a recipe for how to transform the original
    // info to get the one that we're going to decode to.
    SkImageInfo cacheInfo = this->buildCacheInfo(format);

    // 2. Ask the generator to natively create one
    {
        ScopedGenerator generator(fSharedGenerator);
        if (sk_sp<GrTextureProxy> proxy = generator->generateTexture(ctx, cacheInfo, fOrigin)) {
            SK_HISTOGRAM_ENUMERATION("LockTexturePath", kNative_LockTexturePath,
                                     kLockTexturePathCount);
            GrTexture* tex2 = proxy->instantiate(ctx->resourceProvider());
            if (tex2) {
                return set_key_and_return(ctx->resourceProvider(), SkSafeRef(tex2), key);
            }
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
    if (!ctx->contextPriv().disableGpuYUVConversion()) {
        ScopedGenerator generator(fSharedGenerator);
        Generator_GrYUVProvider provider(generator);
        sk_sp<GrTexture> tex = provider.refAsTexture(ctx, desc, true);
        if (tex) {
            SK_HISTOGRAM_ENUMERATION("LockTexturePath", kYUV_LockTexturePath,
                                     kLockTexturePathCount);
            return set_key_and_return(ctx->resourceProvider(), tex.release(), key);
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
            return set_key_and_return(ctx->resourceProvider(), tex, key);
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
                                            const SkImage* client,
                                            SkScalar scaleAdjust[2],
                                            SkImage::CachingHint chint) {
    if (!ctx) {
        return nullptr;
    }

    return GrImageTextureMaker(ctx, this, client, chint).refTextureForParams(params, dstColorSpace,
                                                                             texColorSpace,
                                                                             scaleAdjust);
}

#else

GrTexture* SkImageCacherator::lockAsTexture(GrContext* ctx, const GrSamplerParams&,
                                            SkColorSpace* dstColorSpace,
                                            sk_sp<SkColorSpace>* texColorSpace,
                                            const SkImage* client,
                                            SkScalar scaleAdjust[2], SkImage::CachingHint) {
    return nullptr;
}

#endif
