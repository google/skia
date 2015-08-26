/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkBitmapCache.h"
#include "SkImageCacherator.h"
#include "SkMallocPixelRef.h"
#include "SkNextID.h"
#include "SkPixelRef.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "GrGpuResourcePriv.h"
#include "GrResourceKey.h"
#include "GrTextureAccess.h"
#include "SkGr.h"
#include "SkGrPriv.h"
#endif

SkImageCacherator* SkImageCacherator::NewFromGenerator(SkImageGenerator* gen,
                                                       const SkIRect* subset) {
    if (!gen) {
        return nullptr;
    }
    const SkImageInfo& info = gen->getInfo();
    if (info.isEmpty()) {
        return nullptr;
    }

    uint32_t uniqueID = gen->uniqueID();
    const SkIRect bounds = SkIRect::MakeWH(info.width(), info.height());
    if (subset) {
        if (!bounds.contains(*subset)) {
            return nullptr;
        }
        if (*subset != bounds) {
            // we need a different uniqueID since we really are a subset of the raw generator
            uniqueID = SkNextID::ImageID();
        }
    } else {
        subset = &bounds;
    }

    return new SkImageCacherator(gen, gen->getInfo().makeWH(subset->width(), subset->height()),
                                 SkIPoint::Make(subset->x(), subset->y()), uniqueID);
}

SkImageCacherator::SkImageCacherator(SkImageGenerator* gen, const SkImageInfo& info,
                                     const SkIPoint& origin, uint32_t uniqueID)
    : fNotThreadSafeGenerator(gen)
    , fInfo(info)
    , fOrigin(origin)
    , fUniqueID(uniqueID)
{}

SkData* SkImageCacherator::refEncoded() {
    ScopedGenerator generator(this);
    return generator->refEncodedData();
}

static bool check_output_bitmap(const SkBitmap& bitmap, uint32_t expectedID) {
    SkASSERT(bitmap.getGenerationID() == expectedID);
    SkASSERT(bitmap.isImmutable());
    SkASSERT(bitmap.getPixels());
    return true;
}

bool SkImageCacherator::generateBitmap(SkBitmap* bitmap) {
    const size_t rowBytes = fInfo.minRowBytes();
    if (!bitmap->tryAllocPixels(fInfo, rowBytes)) {
        return false;
    }
    SkASSERT(bitmap->rowBytes() == rowBytes);

    ScopedGenerator generator(this);
    const SkImageInfo& genInfo = generator->getInfo();
    if (fInfo.dimensions() == genInfo.dimensions()) {
        SkASSERT(fOrigin.x() == 0 && fOrigin.y() == 0);
        // fast-case, no copy needed
        if (!generator->getPixels(bitmap->info(), bitmap->getPixels(), rowBytes)) {
            bitmap->reset();
            return false;
        }
    } else {
        // need to handle subsetting
        SkBitmap full;
        if (!full.tryAllocPixels(genInfo)) {
            return false;
        }
        if (!generator->getPixels(full.info(), full.getPixels(), full.rowBytes())) {
            bitmap->reset();
            return false;
        }
        full.readPixels(bitmap->info(), bitmap->getPixels(), bitmap->rowBytes(),
                        fOrigin.x(), fOrigin.y());
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

bool SkImageCacherator::tryLockAsBitmap(SkBitmap* bitmap) {
    if (SkBitmapCache::Find(fUniqueID, bitmap)) {
        return check_output_bitmap(*bitmap, fUniqueID);
    }

    if (!this->generateBitmap(bitmap)) {
        return false;
    }

    bitmap->pixelRef()->setImmutableWithID(fUniqueID);
    SkBitmapCache::Add(fUniqueID, *bitmap);
    return true;
}

bool SkImageCacherator::lockAsBitmap(SkBitmap* bitmap) {
    if (this->tryLockAsBitmap(bitmap)) {
        return check_output_bitmap(*bitmap, fUniqueID);
    }

#if SK_SUPPORT_GPU
    // Try to get a texture and read it back to raster (and then cache that with our ID)
    SkAutoTUnref<GrTexture> tex;

    {
        ScopedGenerator generator(this);
        SkIRect subset = SkIRect::MakeXYWH(fOrigin.x(), fOrigin.y(), fInfo.width(), fInfo.height());
        tex.reset(generator->generateTexture(nullptr, kUntiled_SkImageUsageType, &subset));
    }
    if (!tex) {
        bitmap->reset();
        return false;
    }

    if (!bitmap->tryAllocPixels(fInfo)) {
        bitmap->reset();
        return false;
    }

    const uint32_t pixelOpsFlags = 0;
    if (!tex->readPixels(0, 0, bitmap->width(), bitmap->height(), SkImageInfo2GrPixelConfig(fInfo),
                         bitmap->getPixels(), bitmap->rowBytes(), pixelOpsFlags)) {
        bitmap->reset();
        return false;
    }

    bitmap->pixelRef()->setImmutableWithID(fUniqueID);
    SkBitmapCache::Add(fUniqueID, *bitmap);
    return check_output_bitmap(*bitmap, fUniqueID);
#else
    return false;
#endif
}

//////////////////////////////////////////////////////////////////////////////////////////////////

/*
 *  We have a 5 ways to try to return a texture (in sorted order)
 *
 *  1. Check the cache for a pre-existing one
 *  2. Ask the genreator to natively create one
 *  3. Ask the generator to return a compressed form that the GPU might support
 *  4. Ask the generator to return YUV planes, which the GPU can convert
 *  5. Ask the generator to return RGB(A) data, which the GPU can convert
 */
GrTexture* SkImageCacherator::lockAsTexture(GrContext* ctx, SkImageUsageType usage) {
#if SK_SUPPORT_GPU
    if (!ctx) {
        return nullptr;
    }

    GrUniqueKey key;
    GrMakeKeyFromImageID(&key, fUniqueID, fInfo.width(), fInfo.height(), SkIPoint::Make(0, 0),
                         *ctx->caps(), usage);

    // 1. Check the cache for a pre-existing one
    if (GrTexture* tex = ctx->textureProvider()->findAndRefTextureByUniqueKey(key)) {
        return tex;
    }

    // 2. Ask the genreator to natively create one
    {
        ScopedGenerator generator(this);
        SkIRect subset = SkIRect::MakeXYWH(fOrigin.x(), fOrigin.y(), fInfo.width(), fInfo.height());
        if (GrTexture* tex = generator->generateTexture(ctx, usage, &subset)) {
            tex->resourcePriv().setUniqueKey(key);
            return tex;
        }
    }

    // 3. Ask the generator to return a compressed form that the GPU might support
    // TODO

    // 4. Ask the generator to return YUV planes, which the GPU can convert
    // TODO


    // 5. Ask the generator to return RGB(A) data, which the GPU can convert
    SkBitmap bitmap;
    if (!this->generateBitmap(&bitmap)) {
        return nullptr;
    }
    return GrRefCachedBitmapTexture(ctx, bitmap, usage);
#else
    return nullptr;
#endif
}

