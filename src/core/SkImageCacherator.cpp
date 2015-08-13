/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkBitmapCache.h"
#include "SkImageCacherator.h"
#include "SkPixelRef.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "GrGpuResourcePriv.h"
#include "GrResourceKey.h"
#include "GrTextureAccess.h"
#include "SkGr.h"
#include "SkGrPriv.h"
#endif

SkImageCacherator* SkImageCacherator::NewFromGenerator(SkImageGenerator* gen) {
    if (!gen) {
        return nullptr;
    }
    return SkNEW_ARGS(SkImageCacherator, (gen));
}

SkImageCacherator::SkImageCacherator(SkImageGenerator* gen) : fGenerator(gen) {}

SkImageCacherator::~SkImageCacherator() {
    SkDELETE(fGenerator);
}

static bool check_output_bitmap(const SkBitmap& bitmap, uint32_t expectedID) {
    SkASSERT(bitmap.getGenerationID() == expectedID);
    SkASSERT(bitmap.isImmutable());
    SkASSERT(bitmap.getPixels());
    return true;
}

static bool generate_bitmap(SkImageGenerator* generator, SkBitmap* bitmap) {
    if (!bitmap->tryAllocPixels(generator->getInfo()) ||
        !generator->getPixels(bitmap->info(), bitmap->getPixels(), bitmap->rowBytes()))
    {
        bitmap->reset();
        return false;
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

bool SkImageCacherator::tryLockAsBitmap(SkBitmap* bitmap) {
    const uint32_t uniqueID = fGenerator->uniqueID();

    if (SkBitmapCache::Find(uniqueID, bitmap)) {
        return check_output_bitmap(*bitmap, uniqueID);
    }
    if (!generate_bitmap(fGenerator, bitmap)) {
        return false;
    }

    bitmap->pixelRef()->setImmutableWithID(uniqueID);
    SkBitmapCache::Add(uniqueID, *bitmap);
    return true;
}

bool SkImageCacherator::lockAsBitmap(SkBitmap* bitmap) {
    const uint32_t uniqueID = fGenerator->uniqueID();

    if (this->tryLockAsBitmap(bitmap)) {
        return check_output_bitmap(*bitmap, uniqueID);
    }

#if SK_SUPPORT_GPU
    // Try to get a texture and read it back to raster (and then cache that with our ID)

    SkAutoTUnref<GrTexture> tex(fGenerator->generateTexture(nullptr, kUntiled_SkImageUsageType));
    if (!tex) {
        bitmap->reset();
        return false;
    }

    const SkImageInfo& info = this->info();
    if (!bitmap->tryAllocPixels(info)) {
        bitmap->reset();
        return false;
    }

    const uint32_t pixelOpsFlags = 0;
    if (!tex->readPixels(0, 0, bitmap->width(), bitmap->height(), SkImageInfo2GrPixelConfig(info),
                         bitmap->getPixels(), bitmap->rowBytes(), pixelOpsFlags)) {
        bitmap->reset();
        return false;
    }

    bitmap->pixelRef()->setImmutableWithID(uniqueID);
    SkBitmapCache::Add(uniqueID, *bitmap);
    return check_output_bitmap(*bitmap, uniqueID);
#else
    return false;
#endif
}

//////////////////////////////////////////////////////////////////////////////////////////////////

GrTexture* SkImageCacherator::tryLockAsTexture(GrContext* ctx, SkImageUsageType usage) {
#if SK_SUPPORT_GPU
    const uint32_t uniqueID = fGenerator->uniqueID();
    const SkImageInfo& info = this->info();

    GrUniqueKey key;
    GrMakeKeyFromImageID(&key, uniqueID, info.width(), info.height(), SkIPoint::Make(0, 0),
                         *ctx->caps(), usage);
    GrTexture* tex = ctx->textureProvider()->findAndRefTextureByUniqueKey(key);
    if (tex) {
        return tex; // we got a cache hit!
    }

    tex = fGenerator->generateTexture(ctx, usage);
    if (tex) {
        tex->resourcePriv().setUniqueKey(key);
    }
    return tex;
#else
    return nullptr;
#endif
}

GrTexture* SkImageCacherator::lockAsTexture(GrContext* ctx, SkImageUsageType usage) {
#if SK_SUPPORT_GPU
    if (!ctx) {
        return nullptr;
    }
    if (GrTexture* tex = this->tryLockAsTexture(ctx, usage)) {
        return tex;
    }

    // Try to get a bitmap and then upload/cache it as a texture

    SkBitmap bitmap;
    if (!generate_bitmap(fGenerator, &bitmap)) {
        return nullptr;
    }
    return GrRefCachedBitmapTexture(ctx, bitmap, usage);
#else
    return nullptr;
#endif
}

