/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Test.h"
#include "SkCanvas.h"
#include "SkGraphics.h"
#include "SkBitmapCache.h"

static const int kCanvasSize = 1;
static const int kBitmapSize = 16;
static const int kScale = 8;

static bool is_in_scaled_image_cache(const SkBitmap& orig,
                                     SkScalar xScale,
                                     SkScalar yScale) {
    SkBitmap scaled;
    float roundedImageWidth = SkScalarRoundToScalar(orig.width() * xScale);
    float roundedImageHeight = SkScalarRoundToScalar(orig.height() * xScale);
    return SkBitmapCache::Find(orig, roundedImageWidth, roundedImageHeight, &scaled);
}

// Draw a scaled bitmap, then return true iff it has been cached.
static bool test_scaled_image_cache_useage() {
    SkAutoTUnref<SkCanvas> canvas(
            SkCanvas::NewRasterN32(kCanvasSize, kCanvasSize));
    SkBitmap bitmap;
    bitmap.allocN32Pixels(kBitmapSize, kBitmapSize);
    bitmap.eraseColor(0xFFFFFFFF);
    SkScalar scale = SkIntToScalar(kScale);
    SkScalar scaledSize = SkIntToScalar(kBitmapSize) * scale;
    canvas->clipRect(SkRect::MakeLTRB(0, 0, scaledSize, scaledSize));
    SkPaint paint;
    paint.setFilterLevel(SkPaint::kHigh_FilterLevel);

    canvas->drawBitmapRect(bitmap,
                           SkRect::MakeLTRB(0, 0, scaledSize, scaledSize),
                           &paint);

    return is_in_scaled_image_cache(bitmap, scale, scale);
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

    REPORTER_ASSERT(reporter, test_scaled_image_cache_useage());

    SkGraphics::SetResourceCacheTotalByteLimit(0);  // clear cache
    SkGraphics::SetResourceCacheTotalByteLimit(2 * size);
    SkGraphics::SetResourceCacheSingleAllocationByteLimit(size * 2);  // big enough

    REPORTER_ASSERT(reporter, test_scaled_image_cache_useage());

    SkGraphics::SetResourceCacheTotalByteLimit(0);  // clear cache
    SkGraphics::SetResourceCacheTotalByteLimit(2 * size);
    SkGraphics::SetResourceCacheSingleAllocationByteLimit(size / 2);  // too small

    REPORTER_ASSERT(reporter, !test_scaled_image_cache_useage());

    SkGraphics::SetResourceCacheSingleAllocationByteLimit(originalAllocationLimit);
    SkGraphics::SetResourceCacheTotalByteLimit(originalByteLimit);
}

static SkBitmap createAllocatedBitmap(const SkImageInfo& info) {
    SkBitmap cachedBitmap;
    cachedBitmap.setInfo(info);
    SkBitmap::Allocator* allocator = SkBitmapCache::GetAllocator();
    if (NULL != allocator) {
        allocator->allocPixelRef(&cachedBitmap, 0);
    } else {
        cachedBitmap.allocPixels();
    }

    return cachedBitmap;
}

// http://skbug.com/2894
DEF_TEST(BitmapCache_add_rect, reporter) {
    SkBitmap bm;
    SkIRect rect = SkIRect::MakeWH(5, 5);
    SkBitmap cachedBitmap = createAllocatedBitmap(SkImageInfo::MakeN32Premul(5, 5));
    cachedBitmap.setImmutable();

    // Wrong subset size
    REPORTER_ASSERT(reporter, ! SkBitmapCache::Add(cachedBitmap.getGenerationID(), SkIRect::MakeWH(4, 6), cachedBitmap));
    REPORTER_ASSERT(reporter, ! SkBitmapCache::Find(cachedBitmap.getGenerationID(), rect, &bm));
    // Wrong offset value
    REPORTER_ASSERT(reporter, ! SkBitmapCache::Add(cachedBitmap.getGenerationID(), SkIRect::MakeXYWH(-1, 0, 5, 5), cachedBitmap));
    REPORTER_ASSERT(reporter, ! SkBitmapCache::Find(cachedBitmap.getGenerationID(), rect, &bm));

    // Should not be in the cache
    REPORTER_ASSERT(reporter, !SkBitmapCache::Find(cachedBitmap.getGenerationID(), rect, &bm));

    REPORTER_ASSERT(reporter, SkBitmapCache::Add(cachedBitmap.getGenerationID(), rect, cachedBitmap));
    // Should be in the cache, we just added it
    REPORTER_ASSERT(reporter, SkBitmapCache::Find(cachedBitmap.getGenerationID(), rect, &bm));
}
