/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImage_Base.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkData.h"
#include "SkImageCacherator.h"
#include "SkImagePriv.h"
#include "SkPixelRef.h"
#include "SkSurface.h"

class SkImage_Generator : public SkImage_Base {
public:
    SkImage_Generator(SkImageCacherator* cache)
        : INHERITED(cache->info().width(), cache->info().height(), cache->uniqueID())
        , fCache(cache) // take ownership
    {}

    bool onReadPixels(const SkImageInfo&, void*, size_t, int srcX, int srcY, CachingHint) const override;
    const void* onPeekPixels(SkImageInfo*, size_t* /*rowBytes*/) const override;
    SkData* onRefEncoded() const override;
    bool isOpaque() const override { return fCache->info().isOpaque(); }
    SkImage* onNewSubset(const SkIRect&) const override;
    bool getROPixels(SkBitmap*, CachingHint) const override;
    GrTexture* asTextureRef(GrContext*, const GrTextureParams&) const override;
    bool onIsLazyGenerated() const override { return true; }

private:
    SkAutoTDelete<SkImageCacherator> fCache;

    typedef SkImage_Base INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

bool SkImage_Generator::onReadPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRB,
                                     int srcX, int srcY, CachingHint chint) const {
    SkBitmap bm;
    if (kDisallow_CachingHint == chint) {
        if (fCache->lockAsBitmapOnlyIfAlreadyCached(&bm)) {
            return bm.readPixels(dstInfo, dstPixels, dstRB, srcX, srcY);
        } else {
            // Try passing the caller's buffer directly down to the generator. If this fails we
            // may still succeed in the general case, as the generator may prefer some other
            // config, which we could then convert via SkBitmap::readPixels.
            if (fCache->directGeneratePixels(dstInfo, dstPixels, dstRB, srcX, srcY)) {
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

const void* SkImage_Generator::onPeekPixels(SkImageInfo* infoPtr, size_t* rowBytesPtr) const {
    return NULL;
}

SkData* SkImage_Generator::onRefEncoded() const {
    return fCache->refEncoded();
}

bool SkImage_Generator::getROPixels(SkBitmap* bitmap, CachingHint chint) const {
    return fCache->lockAsBitmap(bitmap, this, chint);
}

GrTexture* SkImage_Generator::asTextureRef(GrContext* ctx, const GrTextureParams& params) const {
    return fCache->lockAsTexture(ctx, params, this);
}

SkImage* SkImage_Generator::onNewSubset(const SkIRect& subset) const {
    // TODO: make this lazy, by wrapping the subset inside a new generator or something
    // For now, we do effectively what we did before, make it a raster

    const SkImageInfo info = SkImageInfo::MakeN32(subset.width(), subset.height(),
                                      this->isOpaque() ? kOpaque_SkAlphaType : kPremul_SkAlphaType);
    SkAutoTUnref<SkSurface> surface(SkSurface::NewRaster(info));
    if (!surface) {
        return nullptr;
    }
    surface->getCanvas()->clear(0);
    surface->getCanvas()->drawImage(this, SkIntToScalar(-subset.x()), SkIntToScalar(-subset.y()),
                                    nullptr);
    return surface->newImageSnapshot();
}

SkImage* SkImage::NewFromGenerator(SkImageGenerator* generator, const SkIRect* subset) {
    SkImageCacherator* cache = SkImageCacherator::NewFromGenerator(generator, subset);
    if (!cache) {
        return nullptr;
    }
    return new SkImage_Generator(cache);
}
