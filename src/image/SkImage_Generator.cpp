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
        : INHERITED(cache->info().width(), cache->info().height(), cache->uniqueID(), NULL)
        , fCache(cache) // take ownership
    {}

    bool onReadPixels(const SkImageInfo&, void*, size_t, int srcX, int srcY) const override;
    const void* onPeekPixels(SkImageInfo*, size_t* /*rowBytes*/) const override;
    SkData* onRefEncoded() const override;
    bool isOpaque() const override { return fCache->info().isOpaque(); }
    SkImage* onNewSubset(const SkIRect&) const override;
    bool getROPixels(SkBitmap*) const override;
    GrTexture* asTextureRef(GrContext*, SkImageUsageType) const override;

    SkShader* onNewShader(SkShader::TileMode,
                          SkShader::TileMode,
                          const SkMatrix* localMatrix) const override;

    bool onIsLazyGenerated() const override { return true; }

private:
    SkAutoTDelete<SkImageCacherator> fCache;

    typedef SkImage_Base INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

SkShader* SkImage_Generator::onNewShader(SkShader::TileMode tileX, SkShader::TileMode tileY,
                                         const SkMatrix* localMatrix) const {
    // TODO: need a native Shader that takes Cacherator (or this image) so we can natively return
    // textures as output from the shader.
    SkBitmap bm;
    if (this->getROPixels(&bm)) {
        return SkShader::CreateBitmapShader(bm, tileX, tileY, localMatrix);
    }
    return nullptr;
}

bool SkImage_Generator::onReadPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRB,
                                     int srcX, int srcY) const {
    SkBitmap bm;
    if (this->getROPixels(&bm)) {
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

bool SkImage_Generator::getROPixels(SkBitmap* bitmap) const {
    return fCache->lockAsBitmap(bitmap, this);
}

GrTexture* SkImage_Generator::asTextureRef(GrContext* ctx, SkImageUsageType usage) const {
    return fCache->lockAsTexture(ctx, usage, this);
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
