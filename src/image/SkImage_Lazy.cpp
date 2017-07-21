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
#include "SkColorSpace_Base.h"
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
#include "GrResourceProvider.h"
#include "GrSamplerParams.h"
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
    SkAlphaType onAlphaType() const override {
        return fInfo.alphaType();
    }

    bool onReadPixels(const SkImageInfo&, void*, size_t, int srcX, int srcY,
                      CachingHint) const override;
#if SK_SUPPORT_GPU
    sk_sp<GrTextureProxy> asTextureProxyRef(GrContext*, const GrSamplerParams&,
                                            SkColorSpace*, sk_sp<SkColorSpace>*,
                                            SkScalar scaleAdjust[2]) const override;
#endif
    SkData* onRefEncoded() const override;
    sk_sp<SkImage> onMakeSubset(const SkIRect&) const override;
    bool getROPixels(SkBitmap*, SkColorSpace* dstColorSpace, CachingHint) const override;
    bool onIsLazyGenerated() const override { return true; }
    bool onCanLazyGenerateOnGPU() const override;
    sk_sp<SkImage> onMakeColorSpace(sk_sp<SkColorSpace>, SkColorType,
                                    SkTransferFunctionBehavior) const override;

    bool onIsValid(GrContext*) const override;

    SkImageCacherator* peekCacherator() const override {
        return const_cast<SkImage_Lazy*>(this);
    }

    // Only return true if the generate has already been cached.
    bool lockAsBitmapOnlyIfAlreadyCached(SkBitmap*, CachedFormat) const;
    // Call the underlying generator directly
    bool directGeneratePixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRB,
                              int srcX, int srcY, SkTransferFunctionBehavior behavior) const;

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
    void makeCacheKeyFromOrigKey(const GrUniqueKey& origKey, CachedFormat,
                                 GrUniqueKey* cacheKey) override;
#endif

    CachedFormat chooseCacheFormat(SkColorSpace* dstColorSpace,
                                   const GrCaps* = nullptr) const override;
    SkImageInfo buildCacheInfo(CachedFormat) const override;

private:
    class ScopedGenerator;

    /**
     *  On success (true), bitmap will point to the pixels for this generator. If this returns
     *  false, the bitmap will be reset to empty.
     */
    bool lockAsBitmap(SkBitmap*, SkImage::CachingHint, CachedFormat, const SkImageInfo&,
                      SkTransferFunctionBehavior) const;

    /**
     * Populates parameters to pass to the generator for reading pixels or generating a texture.
     * For image generators, legacy versus true color blending is indicated using a
     * SkTransferFunctionBehavior, and the target color space is specified on the SkImageInfo.
     * If generatorImageInfo has no color space set, set its color space to this SkImage's color
     * space, and return "ignore" behavior, indicating legacy mode. If generatorImageInfo has a
     * color space set, return "respect" behavior, indicating linear blending mode.
     */
    SkTransferFunctionBehavior getGeneratorBehaviorAndInfo(SkImageInfo* generatorImageInfo) const;

    sk_sp<SharedGenerator> fSharedGenerator;
    // Note that fInfo is not necessarily the info from the generator. It may be cropped by
    // onMakeSubset and its color space may be changed by onMakeColorSpace.
    const SkImageInfo      fInfo;
    const SkIPoint         fOrigin;

    struct IDRec {
        SkOnce      fOnce;
        uint32_t    fUniqueID;
    };
    mutable IDRec fIDRecs[kNumCachedFormats];

    uint32_t getUniqueID(CachedFormat) const;

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
    // We explicit set the legacy format slot, but leave the others uninitialized (via SkOnce)
    // and only resolove them to IDs as needed (by calling getUniqueID()).
    fIDRecs[kLegacy_CachedFormat].fOnce([this, validator] {
        fIDRecs[kLegacy_CachedFormat].fUniqueID = validator->fUniqueID;
    });
}

uint32_t SkImage_Lazy::getUniqueID(CachedFormat format) const {
    IDRec* rec = &fIDRecs[format];
    rec->fOnce([rec] {
        rec->fUniqueID = SkNextID::ImageID();
    });
    return rec->fUniqueID;
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

SkImageCacherator::CachedFormat SkImage_Lazy::chooseCacheFormat(SkColorSpace* dstColorSpace,
                                                                const GrCaps* grCaps) const {
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

        case kBGRA_8888_SkColorType:
            // Odd case. sBGRA isn't a real thing, so we may not have this texturable.
            if (caps.supportsSBGR()) {
                if (cs->gammaCloseToSRGB()) {
                    return kSBGR8888_CachedFormat;
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
            if (caps.supportsHalfFloat()) {
                return kLinearF16_CachedFormat;
            } else if (caps.supportsSRGB()) {
                return kSRGB8888_CachedFormat;
            } else {
                return kLegacy_CachedFormat;
            }
    }
    SkDEBUGFAIL("Unreachable");
    return kLegacy_CachedFormat;
}

SkImageInfo SkImage_Lazy::buildCacheInfo(CachedFormat format) const {
    switch (format) {
        case kLegacy_CachedFormat:
            return fInfo.makeColorSpace(nullptr);
        case kLinearF16_CachedFormat:
            return fInfo.makeColorType(kRGBA_F16_SkColorType)
                        .makeColorSpace(as_CSB(fInfo.colorSpace())->makeLinearGamma());
        case kSRGB8888_CachedFormat:
            // If the transfer function is nearly (but not exactly) sRGB, we don't want the codec
            // to bother trans-coding. It would be slow, and do more harm than good visually,
            // so we make sure to leave the colorspace as-is.
            if (fInfo.colorSpace()->gammaCloseToSRGB()) {
                return fInfo.makeColorType(kRGBA_8888_SkColorType);
            } else {
                return fInfo.makeColorType(kRGBA_8888_SkColorType)
                            .makeColorSpace(as_CSB(fInfo.colorSpace())->makeSRGBGamma());
            }
        case kSBGR8888_CachedFormat:
            // See note above about not-quite-sRGB transfer functions.
            if (fInfo.colorSpace()->gammaCloseToSRGB()) {
                return fInfo.makeColorType(kBGRA_8888_SkColorType);
            } else {
                return fInfo.makeColorType(kBGRA_8888_SkColorType)
                            .makeColorSpace(as_CSB(fInfo.colorSpace())->makeSRGBGamma());
            }
        default:
            SkDEBUGFAIL("Invalid cached format");
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
                                        int srcX, int srcY,
                                        SkTransferFunctionBehavior behavior) const {
    ScopedGenerator generator(fSharedGenerator);
    const SkImageInfo& genInfo = generator->getInfo();
    // Currently generators do not natively handle subsets, so check that first.
    if (srcX || srcY || genInfo.width() != info.width() || genInfo.height() != info.height()) {
        return false;
    }

    SkImageGenerator::Options opts;
    // TODO: This should respect the behavior argument.
    opts.fBehavior = SkTransferFunctionBehavior::kIgnore;
    return generator->getPixels(info, pixels, rb, &opts);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

bool SkImage_Lazy::lockAsBitmapOnlyIfAlreadyCached(SkBitmap* bitmap, CachedFormat format) const {
    uint32_t uniqueID = this->getUniqueID(format);
    return SkBitmapCache::Find(SkBitmapCacheDesc::Make(uniqueID,
                                                       fInfo.width(), fInfo.height()), bitmap) &&
           check_output_bitmap(*bitmap, uniqueID);
}

static bool generate_pixels(SkImageGenerator* gen, const SkPixmap& pmap, int originX, int originY,
                            SkTransferFunctionBehavior behavior) {
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

    SkImageGenerator::Options opts;
    opts.fBehavior = behavior;
    if (!gen->getPixels(dstPM->info(), dstPM->writable_addr(), dstPM->rowBytes(), &opts)) {
        return false;
    }

    if (srcR != dstR) {
        if (!full.readPixels(pmap, originX, originY)) {
            return false;
        }
    }
    return true;
}

bool SkImage_Lazy::lockAsBitmap(SkBitmap* bitmap, SkImage::CachingHint chint, CachedFormat format,
                                const SkImageInfo& info,
                                SkTransferFunctionBehavior behavior) const {
    if (this->lockAsBitmapOnlyIfAlreadyCached(bitmap, format)) {
        return true;
    }

    uint32_t uniqueID = this->getUniqueID(format);

    SkBitmap tmpBitmap;
    SkBitmapCache::RecPtr cacheRec;
    SkPixmap pmap;
    if (SkImage::kAllow_CachingHint == chint) {
        auto desc = SkBitmapCacheDesc::Make(uniqueID, info.width(), info.height());
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
    if (!generate_pixels(generator, pmap, fOrigin.x(), fOrigin.y(), behavior)) {
        return false;
    }

    if (cacheRec) {
        SkBitmapCache::Add(std::move(cacheRec), bitmap);
        SkASSERT(bitmap->getPixels());  // we're locked
        SkASSERT(bitmap->isImmutable());
        SkASSERT(bitmap->getGenerationID() == uniqueID);
        this->notifyAddedToCache();
    } else {
        *bitmap = tmpBitmap;
        bitmap->pixelRef()->setImmutableWithID(uniqueID);
    }

    check_output_bitmap(*bitmap, uniqueID);
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

bool SkImage_Lazy::onReadPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRB,
                                int srcX, int srcY, CachingHint chint) const {
    SkColorSpace* dstColorSpace = dstInfo.colorSpace();
    SkBitmap bm;
    if (kDisallow_CachingHint == chint) {
        CachedFormat cacheFormat = this->chooseCacheFormat(dstColorSpace);
        SkImageInfo genPixelsInfo = dstInfo;
        SkTransferFunctionBehavior behavior = getGeneratorBehaviorAndInfo(&genPixelsInfo);
        if (this->lockAsBitmapOnlyIfAlreadyCached(&bm, cacheFormat)) {
            return bm.readPixels(dstInfo, dstPixels, dstRB, srcX, srcY);
        } else {
            // Try passing the caller's buffer directly down to the generator. If this fails we
            // may still succeed in the general case, as the generator may prefer some other
            // config, which we could then convert via SkBitmap::readPixels.
            if (this->directGeneratePixels(genPixelsInfo, dstPixels, dstRB, srcX, srcY, behavior)) {
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

SkData* SkImage_Lazy::onRefEncoded() const {
    ScopedGenerator generator(fSharedGenerator);
    return generator->refEncodedData();
}

bool SkImage_Lazy::getROPixels(SkBitmap* bitmap, SkColorSpace* dstColorSpace,
                               CachingHint chint) const {
    CachedFormat cacheFormat = this->chooseCacheFormat(dstColorSpace);
    const SkImageInfo cacheInfo = this->buildCacheInfo(cacheFormat);
    SkImageInfo genPixelsInfo = cacheInfo;
    SkTransferFunctionBehavior behavior = getGeneratorBehaviorAndInfo(&genPixelsInfo);
    return this->lockAsBitmap(bitmap, chint, cacheFormat, genPixelsInfo, behavior);
}

bool SkImage_Lazy::onIsValid(GrContext* context) const {
    ScopedGenerator generator(fSharedGenerator);
    return generator->isValid(context);
}

bool SkImage_Lazy::onCanLazyGenerateOnGPU() const {
#if SK_SUPPORT_GPU
    ScopedGenerator generator(fSharedGenerator);
    return SkImageGenerator::TexGenType::kNone != generator->onCanGenerateTexture();
#else
    return false;
#endif
}

SkTransferFunctionBehavior SkImage_Lazy::getGeneratorBehaviorAndInfo(SkImageInfo* generatorImageInfo) const {
    if (generatorImageInfo->colorSpace()) {
        return SkTransferFunctionBehavior::kRespect;
    }
    // Only specify an output color space if color conversion can be done on the color type.
    switch (generatorImageInfo->colorType()) {
        case kRGBA_8888_SkColorType:
        case kBGRA_8888_SkColorType:
        case kRGBA_F16_SkColorType:
        case kRGB_565_SkColorType:
            *generatorImageInfo = generatorImageInfo->makeColorSpace(fInfo.refColorSpace());
            break;
        default:
            break;
    }
    return SkTransferFunctionBehavior::kIgnore;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU
sk_sp<GrTextureProxy> SkImage_Lazy::asTextureProxyRef(GrContext* context,
                                                      const GrSamplerParams& params,
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
                                              SkColorType targetColorType,
                                              SkTransferFunctionBehavior premulBehavior) const {
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

void SkImage_Lazy::makeCacheKeyFromOrigKey(const GrUniqueKey& origKey, CachedFormat format,
                                           GrUniqueKey* cacheKey) {
    SkASSERT(!cacheKey->isValid());
    if (origKey.isValid()) {
        static const GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();
        GrUniqueKey::Builder builder(cacheKey, origKey, kDomain, 1);
        builder[0] = format;
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

static void set_key_on_proxy(GrResourceProvider* resourceProvider,
                             GrTextureProxy* proxy, const GrUniqueKey& key) {
    if (key.isValid()) {
        resourceProvider->assignUniqueKeyToProxy(key, proxy);
    }
}

sk_sp<SkColorSpace> SkImage_Lazy::getColorSpace(GrContext* ctx, SkColorSpace* dstColorSpace) {
    // TODO: This isn't always correct. Picture generator currently produces textures in N32,
    // and will (soon) emit them in an arbitrary (destination) space. We will need to stash that
    // information in/on the key so we can return the correct space in case #1 of lockTexture.
    CachedFormat format = this->chooseCacheFormat(dstColorSpace, ctx->caps());
    SkImageInfo cacheInfo = this->buildCacheInfo(format);
    return sk_ref_sp(cacheInfo.colorSpace());
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

    // Determine which cached format we're going to use (which may involve decoding to a different
    // info than the generator provides).
    CachedFormat format = this->chooseCacheFormat(dstColorSpace, ctx->caps());

    // Fold the cache format into our texture key
    GrUniqueKey key;
    this->makeCacheKeyFromOrigKey(origKey, format, &key);

    // 1. Check the cache for a pre-existing one
    if (key.isValid()) {
        if (sk_sp<GrTextureProxy> proxy = ctx->resourceProvider()->findProxyByUniqueKey(key)) {
            SK_HISTOGRAM_ENUMERATION("LockTexturePath", kPreExisting_LockTexturePath,
                                     kLockTexturePathCount);
            return proxy;
        }
    }

    // The CachedFormat is both an index for which cache "slot" we'll use to store this particular
    // decoded variant of the encoded data, and also a recipe for how to transform the original
    // info to get the one that we're going to decode to.
    const SkImageInfo cacheInfo = this->buildCacheInfo(format);
    SkImageInfo genPixelsInfo = cacheInfo;
    SkTransferFunctionBehavior behavior = getGeneratorBehaviorAndInfo(&genPixelsInfo);

    // 2. Ask the generator to natively create one
    {
        ScopedGenerator generator(fSharedGenerator);
        if (GrTextureMaker::AllowedTexGenType::kCheap == genType &&
                SkImageGenerator::TexGenType::kCheap != generator->onCanGenerateTexture()) {
            return nullptr;
        }
        if (sk_sp<GrTextureProxy> proxy =
                    generator->generateTexture(ctx, genPixelsInfo, fOrigin, behavior)) {
            SK_HISTOGRAM_ENUMERATION("LockTexturePath", kNative_LockTexturePath,
                                     kLockTexturePathCount);
            set_key_on_proxy(ctx->resourceProvider(), proxy.get(), key);
            return proxy;
        }
    }

    // 3. Ask the generator to return YUV planes, which the GPU can convert
    if (!ctx->contextPriv().disableGpuYUVConversion()) {
        const GrSurfaceDesc desc = GrImageInfoToSurfaceDesc(cacheInfo, *ctx->caps());
        ScopedGenerator generator(fSharedGenerator);
        Generator_GrYUVProvider provider(generator);

        // The pixels in the texture will be in the generator's color space. If onMakeColorSpace
        // has been called then this will not match this image's color space. To correct this, apply
        // a color space conversion from the generator's color space to this image's color space.
        const SkColorSpace* generatorColorSpace =
                fSharedGenerator->fGenerator->getInfo().colorSpace();
        const SkColorSpace* thisColorSpace = fInfo.colorSpace();

        sk_sp<GrTextureProxy> proxy =
                provider.refAsTextureProxy(ctx, desc, true, generatorColorSpace, thisColorSpace);
        if (proxy) {
            SK_HISTOGRAM_ENUMERATION("LockTexturePath", kYUV_LockTexturePath,
                                     kLockTexturePathCount);
            set_key_on_proxy(ctx->resourceProvider(), proxy.get(), key);
            return proxy;
        }
    }

    // 4. Ask the generator to return RGB(A) data, which the GPU can convert
    SkBitmap bitmap;
    if (this->lockAsBitmap(&bitmap, chint, format, genPixelsInfo, behavior)) {
        sk_sp<GrTextureProxy> proxy;
        if (willBeMipped) {
            proxy = GrGenerateMipMapsAndUploadToTextureProxy(ctx, bitmap, dstColorSpace);
        }
        if (!proxy) {
            proxy = GrUploadBitmapToTextureProxy(ctx->resourceProvider(), bitmap, dstColorSpace);
        }
        if (proxy) {
            SK_HISTOGRAM_ENUMERATION("LockTexturePath", kRGBA_LockTexturePath,
                                     kLockTexturePathCount);
            set_key_on_proxy(ctx->resourceProvider(), proxy.get(), key);
            return proxy;
        }
    }
    SK_HISTOGRAM_ENUMERATION("LockTexturePath", kFailure_LockTexturePath,
                             kLockTexturePathCount);
    return nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif
