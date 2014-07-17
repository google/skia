/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Test.h"
#include "SkGraphics.h"
#include "SkCanvas.h"

static const int kCanvasSize = 1;
static const int kBitmapSize = 16;
static const int kScale = 8;

static size_t test_scaled_image_cache_useage() {
    SkAutoTUnref<SkCanvas> canvas(
            SkCanvas::NewRasterN32(kCanvasSize, kCanvasSize));
    SkBitmap bitmap;
    SkAssertResult(bitmap.allocN32Pixels(kBitmapSize, kBitmapSize));
    SkScalar scaledSize = SkIntToScalar(kScale * kBitmapSize);
    canvas->clipRect(SkRect::MakeLTRB(0, 0, scaledSize, scaledSize));
    SkPaint paint;
    paint.setFilterLevel(SkPaint::kHigh_FilterLevel);
    size_t bytesUsed = SkGraphics::GetImageCacheBytesUsed();
    canvas->drawBitmapRect(bitmap,
                           SkRect::MakeLTRB(0, 0, scaledSize, scaledSize),
                           &paint);
    return SkGraphics::GetImageCacheBytesUsed() - bytesUsed;
}

// http://crbug.com/389439
DEF_TEST(ScaledImageCache_SingleAllocationByteLimit, reporter) {
    size_t originalByteLimit = SkGraphics::GetImageCacheByteLimit();
    size_t originalAllocationLimit =
        SkGraphics::GetImageCacheSingleAllocationByteLimit();

    size_t size = kBitmapSize * kScale * kBitmapSize * kScale
        * SkColorTypeBytesPerPixel(kN32_SkColorType);

    SkGraphics::SetImageCacheByteLimit(0); // clear cache
    SkGraphics::SetImageCacheByteLimit(2 * size);
    SkGraphics::SetImageCacheSingleAllocationByteLimit(0);

    REPORTER_ASSERT(reporter, size == test_scaled_image_cache_useage());

    SkGraphics::SetImageCacheByteLimit(0); // clear cache
    SkGraphics::SetImageCacheByteLimit(2 * size);
    SkGraphics::SetImageCacheSingleAllocationByteLimit(size * 2);

    REPORTER_ASSERT(reporter, size == test_scaled_image_cache_useage());

    SkGraphics::SetImageCacheByteLimit(0); // clear cache
    SkGraphics::SetImageCacheByteLimit(2 * size);
    SkGraphics::SetImageCacheSingleAllocationByteLimit(size / 2);

    REPORTER_ASSERT(reporter, 0 == test_scaled_image_cache_useage());

    SkGraphics::SetImageCacheSingleAllocationByteLimit(originalAllocationLimit);
    SkGraphics::SetImageCacheByteLimit(originalByteLimit);
}
