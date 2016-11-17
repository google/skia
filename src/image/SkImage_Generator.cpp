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

    bool onReadPixels(const SkImageInfo&, void*, size_t, int srcX, int srcY, CachingHint) const override;
    SkImageCacherator* peekCacherator() const override { return &fCache; }
    SkData* onRefEncoded(GrContext*) const override;
    sk_sp<SkImage> onMakeSubset(const SkIRect&) const override;
    bool getROPixels(SkBitmap*, CachingHint) const override;
    GrTexture* asTextureRef(GrContext*, const GrSamplerParams&,
                            SkDestinationSurfaceColorMode) const override;
    bool onIsLazyGenerated() const override { return true; }

private:
    mutable SkImageCacherator fCache;

    typedef SkImage_Base INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

bool SkImage_Generator::onReadPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRB,
                                     int srcX, int srcY, CachingHint chint) const {
    SkBitmap bm;
    if (kDisallow_CachingHint == chint) {
        if (fCache.lockAsBitmapOnlyIfAlreadyCached(&bm)) {
            return bm.readPixels(dstInfo, dstPixels, dstRB, srcX, srcY);
        } else {
            // Try passing the caller's buffer directly down to the generator. If this fails we
            // may still succeed in the general case, as the generator may prefer some other
            // config, which we could then convert via SkBitmap::readPixels.
            if (fCache.directGeneratePixels(dstInfo, dstPixels, dstRB, srcX, srcY)) {
                return true;
            }
            // else fall through
        }
    }

    if (this->getROPixels(&bm, chint)) {
        return bm.readPixels(dstInfo, dstPixels, dstRB, srcX, srcY);
    }
    return false;
}

SkData* SkImage_Generator::onRefEncoded(GrContext* ctx) const {
    return fCache.refEncoded(ctx);
}

bool SkImage_Generator::getROPixels(SkBitmap* bitmap, CachingHint chint) const {
    return fCache.lockAsBitmap(bitmap, this, chint);
}

GrTexture* SkImage_Generator::asTextureRef(GrContext* ctx, const GrSamplerParams& params,
                                           SkDestinationSurfaceColorMode colorMode) const {
    return fCache.lockAsTexture(ctx, params, colorMode, this);
}

sk_sp<SkImage> SkImage_Generator::onMakeSubset(const SkIRect& subset) const {
    SkASSERT(fCache.info().bounds().contains(subset));
    SkASSERT(fCache.info().bounds() != subset);

    const SkIRect generatorSubset = subset.makeOffset(fCache.fOrigin.x(), fCache.fOrigin.y());
    SkImageCacherator::Validator validator(fCache.fSharedGenerator, &generatorSubset);
    return validator ? sk_sp<SkImage>(new SkImage_Generator(&validator)) : nullptr;
}

sk_sp<SkImage> SkImage::MakeFromGenerator(SkImageGenerator* generator, const SkIRect* subset) {
    SkImageCacherator::Validator validator(SkImageCacherator::SharedGenerator::Make(generator),
                                           subset);

    return validator ? sk_make_sp<SkImage_Generator>(&validator) : nullptr;
}
