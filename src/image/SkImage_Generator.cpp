/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImage_Base.h"
#include "SkBitmap.h"
#include "SkData.h"
#include "SkImageCacherator.h"
#include "SkImagePriv.h"
#include "SkPixelRef.h"

class SkImage_Generator : public SkImage_Base {
public:
    SkImage_Generator(SkImageCacherator::Validator* validator)
        : INHERITED(validator->fInfo.width(), validator->fInfo.height(), validator->fUniqueID)
        , fCache(validator)
    {}

    virtual SkImageInfo onImageInfo() const override {
        return fCache.info();
    }
    SkAlphaType onAlphaType() const override {
        return fCache.info().alphaType();
    }

    bool onReadPixels(const SkImageInfo&, void*, size_t, int srcX, int srcY,
                      CachingHint) const override;
#if SK_SUPPORT_GPU
    sk_sp<GrTextureProxy> asTextureProxyRef(GrContext*, const GrSamplerParams&,
                                            SkColorSpace*, sk_sp<SkColorSpace>*,
                                            SkScalar scaleAdjust[2]) const override;
#endif
    SkImageCacherator* peekCacherator() const override { return &fCache; }
    SkData* onRefEncoded(GrContext*) const override;
    sk_sp<SkImage> onMakeSubset(const SkIRect&) const override;
    bool getROPixels(SkBitmap*, SkColorSpace* dstColorSpace, CachingHint) const override;
    bool onIsLazyGenerated() const override { return true; }
    sk_sp<SkImage> onMakeColorSpace(sk_sp<SkColorSpace>) const override;

private:
    mutable SkImageCacherator fCache;

    typedef SkImage_Base INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

bool SkImage_Generator::onReadPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRB,
                                     int srcX, int srcY, CachingHint chint) const {
    SkColorSpace* dstColorSpace = dstInfo.colorSpace();
    SkBitmap bm;
    if (kDisallow_CachingHint == chint) {
        SkImageCacherator::CachedFormat cacheFormat = fCache.chooseCacheFormat(dstColorSpace);
        if (fCache.lockAsBitmapOnlyIfAlreadyCached(&bm, cacheFormat)) {
            return bm.readPixels(dstInfo, dstPixels, dstRB, srcX, srcY);
        } else {
            // Try passing the caller's buffer directly down to the generator. If this fails we
            // may still succeed in the general case, as the generator may prefer some other
            // config, which we could then convert via SkBitmap::readPixels.
            if (fCache.directGeneratePixels(dstInfo, dstPixels, dstRB, srcX, srcY,
                                            SkTransferFunctionBehavior::kRespect)) {
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

SkData* SkImage_Generator::onRefEncoded(GrContext* ctx) const {
    return fCache.refEncoded(ctx);
}

bool SkImage_Generator::getROPixels(SkBitmap* bitmap, SkColorSpace* dstColorSpace,
                                    CachingHint chint) const {
    return fCache.lockAsBitmap(nullptr, bitmap, this, dstColorSpace, chint);
}

#if SK_SUPPORT_GPU
sk_sp<GrTextureProxy> SkImage_Generator::asTextureProxyRef(GrContext* context,
                                                           const GrSamplerParams& params,
                                                           SkColorSpace* dstColorSpace,
                                                           sk_sp<SkColorSpace>* texColorSpace,
                                                           SkScalar scaleAdjust[2]) const {
    return fCache.lockAsTextureProxy(context, params, dstColorSpace,
                                     texColorSpace, this, scaleAdjust);
}
#endif

sk_sp<SkImage> SkImage_Generator::onMakeSubset(const SkIRect& subset) const {
    SkASSERT(fCache.info().bounds().contains(subset));
    SkASSERT(fCache.info().bounds() != subset);

    const SkIRect generatorSubset = subset.makeOffset(fCache.fOrigin.x(), fCache.fOrigin.y());
    SkImageCacherator::Validator validator(fCache.fSharedGenerator, &generatorSubset);
    return validator ? sk_sp<SkImage>(new SkImage_Generator(&validator)) : nullptr;
}

sk_sp<SkImage> SkImage_Generator::onMakeColorSpace(sk_sp<SkColorSpace> target) const {
    SkBitmap dst;
    SkImageInfo dstInfo = fCache.info().makeColorSpace(target);
    if (kIndex_8_SkColorType == dstInfo.colorType() ||
        kGray_8_SkColorType == dstInfo.colorType() ||
        kRGB_565_SkColorType == dstInfo.colorType()) {
        dstInfo = dstInfo.makeColorType(kN32_SkColorType);
    }
    dst.allocPixels(dstInfo);

    // Use kIgnore for transfer function behavior.  This is used by the SkColorSpaceXformCanvas,
    // which wants to pre-xform the inputs but ignore the transfer function on blends.
    if (!fCache.directGeneratePixels(dstInfo, dst.getPixels(), dst.rowBytes(), 0, 0,
                                     SkTransferFunctionBehavior::kIgnore)) {
        return nullptr;
    }

    dst.setImmutable();
    return SkImage::MakeFromBitmap(dst);
}

sk_sp<SkImage> SkImage::MakeFromGenerator(std::unique_ptr<SkImageGenerator> generator,
                                          const SkIRect* subset) {
    SkImageCacherator::Validator validator(
                       SkImageCacherator::SharedGenerator::Make(std::move(generator)), subset);

    return validator ? sk_make_sp<SkImage_Generator>(&validator) : nullptr;
}
