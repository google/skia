/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "SkBitmapCache.h"
#include "SkCanvas.h"
#include "SkDiscardableMemoryPool.h"
#include "SkGraphics.h"
#include "SkResourceCache.h"
#include "SkSurface.h"

static const int kCanvasSize = 1;
static const int kBitmapSize = 16;
static const int kScale = 8;

static bool is_in_scaled_image_cache(const SkBitmap& orig,
                                     SkScalar xScale,
                                     SkScalar yScale) {
    SkBitmap scaled;
    float roundedImageWidth = SkScalarRoundToScalar(orig.width() * xScale);
    float roundedImageHeight = SkScalarRoundToScalar(orig.height() * yScale);
    return SkBitmapCache::Find(orig, roundedImageWidth, roundedImageHeight, &scaled);
}

// Draw a scaled bitmap, then return true if it has been cached.
static bool test_scaled_image_cache_usage() {
    SkAutoTUnref<SkSurface> surface(SkSurface::NewRasterN32Premul(kCanvasSize, kCanvasSize));
    SkCanvas* canvas = surface->getCanvas();
    SkBitmap bitmap;
    bitmap.allocN32Pixels(kBitmapSize, kBitmapSize);
    bitmap.eraseColor(0xFFFFFFFF);
    SkScalar xScale = SkIntToScalar(kScale);
    SkScalar yScale = xScale / 2;
    SkScalar xScaledSize = SkIntToScalar(kBitmapSize) * xScale;
    SkScalar yScaledSize = SkIntToScalar(kBitmapSize) * yScale;
    canvas->clipRect(SkRect::MakeLTRB(0, 0, xScaledSize, yScaledSize));
    SkPaint paint;
    paint.setFilterQuality(kHigh_SkFilterQuality);

    canvas->drawBitmapRect(bitmap,
                           SkRect::MakeLTRB(0, 0, xScaledSize, yScaledSize),
                           &paint);

    return is_in_scaled_image_cache(bitmap, xScale, yScale);
}

// http://crbug.com/389439
DEF_TEST(ResourceCache_SingleAllocationByteLimit, reporter) {
    size_t originalByteLimit = SkGraphics::GetResourceCacheTotalByteLimit();
    size_t originalAllocationLimit =
        SkGraphics::GetResourceCacheSingleAllocationByteLimit();

    size_t size = kBitmapSize * kScale * kBitmapSize * kScale
        * SkColorTypeBytesPerPixel(kN32_SkColorType);

    SkGraphics::SetResourceCacheTotalByteLimit(0);  // clear cache
    SkGraphics::SetResourceCacheTotalByteLimit(2 * size);
    SkGraphics::SetResourceCacheSingleAllocationByteLimit(0);  // No limit

    REPORTER_ASSERT(reporter, test_scaled_image_cache_usage());

    SkGraphics::SetResourceCacheTotalByteLimit(0);  // clear cache
    SkGraphics::SetResourceCacheTotalByteLimit(2 * size);
    SkGraphics::SetResourceCacheSingleAllocationByteLimit(size * 2);  // big enough

    REPORTER_ASSERT(reporter, test_scaled_image_cache_usage());

    SkGraphics::SetResourceCacheTotalByteLimit(0);  // clear cache
    SkGraphics::SetResourceCacheTotalByteLimit(2 * size);
    SkGraphics::SetResourceCacheSingleAllocationByteLimit(size / 2);  // too small

    REPORTER_ASSERT(reporter, !test_scaled_image_cache_usage());

    SkGraphics::SetResourceCacheSingleAllocationByteLimit(originalAllocationLimit);
    SkGraphics::SetResourceCacheTotalByteLimit(originalByteLimit);
}

////////////////////////////////////////////////////////////////////////////////////////

static void make_bitmap(SkBitmap* bitmap, const SkImageInfo& info, SkBitmap::Allocator* allocator) {
    if (allocator) {
        bitmap->setInfo(info);
        allocator->allocPixelRef(bitmap, 0);
    } else {
        bitmap->allocPixels(info);
    }
}

// http://skbug.com/2894
DEF_TEST(BitmapCache_add_rect, reporter) {
    SkResourceCache::DiscardableFactory factory = SkResourceCache::GetDiscardableFactory();
    SkBitmap::Allocator* allocator = SkBitmapCache::GetAllocator();

    SkAutoTDelete<SkResourceCache> cache;
    if (factory) {
        cache.reset(SkNEW_ARGS(SkResourceCache, (factory)));
    } else {
        const size_t byteLimit = 100 * 1024;
        cache.reset(SkNEW_ARGS(SkResourceCache, (byteLimit)));
    }
    SkBitmap cachedBitmap;
    make_bitmap(&cachedBitmap, SkImageInfo::MakeN32Premul(5, 5), allocator);
    cachedBitmap.setImmutable();

    SkBitmap bm;
    SkIRect rect = SkIRect::MakeWH(5, 5);
    uint32_t cachedID = cachedBitmap.getGenerationID();
    SkPixelRef* cachedPR = cachedBitmap.pixelRef();

    // Wrong subset size
    REPORTER_ASSERT(reporter, !SkBitmapCache::Add(cachedPR, SkIRect::MakeWH(4, 6), cachedBitmap, cache));
    REPORTER_ASSERT(reporter, !SkBitmapCache::Find(cachedID, rect, &bm, cache));
    // Wrong offset value
    REPORTER_ASSERT(reporter, !SkBitmapCache::Add(cachedPR, SkIRect::MakeXYWH(-1, 0, 5, 5), cachedBitmap, cache));
    REPORTER_ASSERT(reporter, !SkBitmapCache::Find(cachedID, rect, &bm, cache));

    // Should not be in the cache
    REPORTER_ASSERT(reporter, !SkBitmapCache::Find(cachedID, rect, &bm, cache));

    REPORTER_ASSERT(reporter, SkBitmapCache::Add(cachedPR, rect, cachedBitmap, cache));
    // Should be in the cache, we just added it
    REPORTER_ASSERT(reporter, SkBitmapCache::Find(cachedID, rect, &bm, cache));
}

#include "SkMipMap.h"

enum LockedState {
    kNotLocked,
    kLocked,
};

enum CachedState {
    kNotInCache,
    kInCache,
};

static void check_data(skiatest::Reporter* reporter, const SkCachedData* data,
                       int refcnt, CachedState cacheState, LockedState lockedState) {
    REPORTER_ASSERT(reporter, data->testing_only_getRefCnt() == refcnt);
    REPORTER_ASSERT(reporter, data->testing_only_isInCache() == (kInCache == cacheState));
    bool isLocked = (data->data() != NULL);
    REPORTER_ASSERT(reporter, isLocked == (lockedState == kLocked));
}

static void test_mipmapcache(skiatest::Reporter* reporter, SkResourceCache* cache) {
    cache->purgeAll();

    SkBitmap src;
    src.allocN32Pixels(5, 5);
    src.setImmutable();

    const SkMipMap* mipmap = SkMipMapCache::FindAndRef(src, cache);
    REPORTER_ASSERT(reporter, NULL == mipmap);

    mipmap = SkMipMapCache::AddAndRef(src, cache);
    REPORTER_ASSERT(reporter, mipmap);

    {
        const SkMipMap* mm = SkMipMapCache::FindAndRef(src, cache);
        REPORTER_ASSERT(reporter, mm);
        REPORTER_ASSERT(reporter, mm == mipmap);
        mm->unref();
    }

    check_data(reporter, mipmap, 2, kInCache, kLocked);

    mipmap->unref();
    // tricky, since technically after this I'm no longer an owner, but since the cache is
    // local, I know it won't get purged behind my back
    check_data(reporter, mipmap, 1, kInCache, kNotLocked);

    // find us again
    mipmap = SkMipMapCache::FindAndRef(src, cache);
    check_data(reporter, mipmap, 2, kInCache, kLocked);

    cache->purgeAll();
    check_data(reporter, mipmap, 1, kNotInCache, kLocked);

    mipmap->unref();
}

static void test_mipmap_notify(skiatest::Reporter* reporter, SkResourceCache* cache) {
    const int N = 3;
    SkBitmap src[N];
    for (int i = 0; i < N; ++i) {
        src[i].allocN32Pixels(5, 5);
        src[i].setImmutable();
        SkMipMapCache::AddAndRef(src[i], cache)->unref();
    }

    for (int i = 0; i < N; ++i) {
        const SkMipMap* mipmap = SkMipMapCache::FindAndRef(src[i], cache);
        if (cache) {
            // if cache is null, we're working on the global cache, and other threads might purge
            // it, making this check fragile.
            REPORTER_ASSERT(reporter, mipmap);
        }
        SkSafeUnref(mipmap);

        src[i].reset(); // delete the underlying pixelref, which *should* remove us from the cache

        mipmap = SkMipMapCache::FindAndRef(src[i], cache);
        REPORTER_ASSERT(reporter, !mipmap);
    }
}

static void test_bitmap_notify(skiatest::Reporter* reporter, SkResourceCache* cache) {
    const SkIRect subset = SkIRect::MakeWH(5, 5);
    const int N = 3;
    SkBitmap src[N], dst[N];
    for (int i = 0; i < N; ++i) {
        src[i].allocN32Pixels(5, 5);
        src[i].setImmutable();
        dst[i].allocN32Pixels(5, 5);
        dst[i].setImmutable();
        SkBitmapCache::Add(src[i].pixelRef(), subset, dst[i], cache);
    }

    for (int i = 0; i < N; ++i) {
        const uint32_t genID = src[i].getGenerationID();
        SkBitmap result;
        bool found = SkBitmapCache::Find(genID, subset, &result, cache);
        if (cache) {
            // if cache is null, we're working on the global cache, and other threads might purge
            // it, making this check fragile.
            REPORTER_ASSERT(reporter, found);
        }

        src[i].reset(); // delete the underlying pixelref, which *should* remove us from the cache

        found = SkBitmapCache::Find(genID, subset, &result, cache);
        REPORTER_ASSERT(reporter, !found);
    }
}

DEF_TEST(BitmapCache_discarded_bitmap, reporter) {
    SkResourceCache::DiscardableFactory factory = SkResourceCache::GetDiscardableFactory();
    SkBitmap::Allocator* allocator = SkBitmapCache::GetAllocator();
    
    SkAutoTDelete<SkResourceCache> cache;
    if (factory) {
        cache.reset(SkNEW_ARGS(SkResourceCache, (factory)));
    } else {
        const size_t byteLimit = 100 * 1024;
        cache.reset(SkNEW_ARGS(SkResourceCache, (byteLimit)));
    }
    SkBitmap cachedBitmap;
    make_bitmap(&cachedBitmap, SkImageInfo::MakeN32Premul(5, 5), allocator);
    cachedBitmap.setImmutable();
    cachedBitmap.unlockPixels();

    SkBitmap bm;
    SkIRect rect = SkIRect::MakeWH(5, 5);

    // Add a bitmap to the cache.
    REPORTER_ASSERT(reporter, SkBitmapCache::Add(cachedBitmap.pixelRef(), rect, cachedBitmap, cache));
    REPORTER_ASSERT(reporter, SkBitmapCache::Find(cachedBitmap.getGenerationID(), rect, &bm, cache));

    // Finding more than once works fine.
    REPORTER_ASSERT(reporter, SkBitmapCache::Find(cachedBitmap.getGenerationID(), rect, &bm, cache));
    bm.unlockPixels();

    // Drop the pixels in the bitmap.
    if (factory) {
        REPORTER_ASSERT(reporter, SkGetGlobalDiscardableMemoryPool()->getRAMUsed() > 0);
        SkGetGlobalDiscardableMemoryPool()->dumpPool();
        REPORTER_ASSERT(reporter, SkGetGlobalDiscardableMemoryPool()->getRAMUsed() == 0);

        // The bitmap is not in the cache since it has been dropped.
        REPORTER_ASSERT(reporter, !SkBitmapCache::Find(cachedBitmap.getGenerationID(), rect, &bm, cache));
    }

    make_bitmap(&cachedBitmap, SkImageInfo::MakeN32Premul(5, 5), allocator);
    cachedBitmap.setImmutable();
    cachedBitmap.unlockPixels();

    // We can add the bitmap back to the cache and find it again.
    REPORTER_ASSERT(reporter, SkBitmapCache::Add(cachedBitmap.pixelRef(), rect, cachedBitmap, cache));
    REPORTER_ASSERT(reporter, SkBitmapCache::Find(cachedBitmap.getGenerationID(), rect, &bm, cache));

    test_mipmapcache(reporter, cache);
    test_bitmap_notify(reporter, cache);
    test_mipmap_notify(reporter, cache);
}
