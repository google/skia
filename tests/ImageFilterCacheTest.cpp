 /*
  * Copyright 2016 Google Inc.
  *
  * Use of this source code is governed by a BSD-style license that can be
  * found in the LICENSE file.
  */

#include "Test.h"

#include "SkBitmap.h"
#include "SkImage.h"
#include "SkImageFilter.h"
#include "SkImageFilterCache.h"
#include "SkMatrix.h"
#include "SkSpecialImage.h"

static const int kSmallerSize = 10;
static const int kPad = 3;
static const int kFullSize = kSmallerSize + 2 * kPad;

static SkBitmap create_bm() {
    SkBitmap bm;
    bm.allocN32Pixels(kFullSize, kFullSize, true);
    bm.eraseColor(SK_ColorTRANSPARENT);
    return bm;
}

// Ensure the cache can return a cached image
static void test_find_existing(skiatest::Reporter* reporter,
                               const sk_sp<SkSpecialImage>& image,
                               const sk_sp<SkSpecialImage>& subset) {
    static const size_t kCacheSize = 1000000;
    sk_sp<SkImageFilterCache> cache(SkImageFilterCache::Create(kCacheSize));

    SkIRect clip = SkIRect::MakeWH(100, 100);
    SkImageFilterCacheKey key1(0, SkMatrix::I(), clip, image->uniqueID(), image->subset());
    SkImageFilterCacheKey key2(0, SkMatrix::I(), clip, subset->uniqueID(), subset->subset());

    SkIPoint offset = SkIPoint::Make(3, 4);
    cache->set(key1, image.get(), offset);

    SkIPoint foundOffset;

    SkSpecialImage* foundImage = cache->get(key1, &foundOffset);
    REPORTER_ASSERT(reporter, foundImage);
    REPORTER_ASSERT(reporter, offset == foundOffset);

    REPORTER_ASSERT(reporter, !cache->get(key2, &foundOffset));
}

// If either id is different or the clip or the matrix are different the
// cached image won't be found. Even if it is caching the same bitmap.
static void test_dont_find_if_diff_key(skiatest::Reporter* reporter,
                                       const sk_sp<SkSpecialImage>& image,
                                       const sk_sp<SkSpecialImage>& subset) {
    static const size_t kCacheSize = 1000000;
    sk_sp<SkImageFilterCache> cache(SkImageFilterCache::Create(kCacheSize));

    SkIRect clip1 = SkIRect::MakeWH(100, 100);
    SkIRect clip2 = SkIRect::MakeWH(200, 200);
    SkImageFilterCacheKey key0(0, SkMatrix::I(), clip1, image->uniqueID(), image->subset());
    SkImageFilterCacheKey key1(1, SkMatrix::I(), clip1, image->uniqueID(), image->subset());
    SkImageFilterCacheKey key2(0, SkMatrix::MakeTrans(5, 5), clip1,
                                   image->uniqueID(), image->subset());
    SkImageFilterCacheKey key3(0, SkMatrix::I(), clip2, image->uniqueID(), image->subset());
    SkImageFilterCacheKey key4(0, SkMatrix::I(), clip1, subset->uniqueID(), subset->subset());

    SkIPoint offset = SkIPoint::Make(3, 4);
    cache->set(key0, image.get(), offset);

    SkIPoint foundOffset;
    REPORTER_ASSERT(reporter, !cache->get(key1, &foundOffset));
    REPORTER_ASSERT(reporter, !cache->get(key2, &foundOffset));
    REPORTER_ASSERT(reporter, !cache->get(key3, &foundOffset));
    REPORTER_ASSERT(reporter, !cache->get(key4, &foundOffset));
}

// Test purging when the max cache size is exceeded
static void test_internal_purge(skiatest::Reporter* reporter, const sk_sp<SkSpecialImage>& image) {
    SkASSERT(image->getSize());
    const size_t kCacheSize = image->getSize() + 10;
    sk_sp<SkImageFilterCache> cache(SkImageFilterCache::Create(kCacheSize));

    SkIRect clip = SkIRect::MakeWH(100, 100);
    SkImageFilterCacheKey key1(0, SkMatrix::I(), clip, image->uniqueID(), image->subset());
    SkImageFilterCacheKey key2(1, SkMatrix::I(), clip, image->uniqueID(), image->subset());

    SkIPoint offset = SkIPoint::Make(3, 4);
    cache->set(key1, image.get(), offset);

    SkIPoint foundOffset;

    REPORTER_ASSERT(reporter, cache->get(key1, &foundOffset));

    // This should knock the first one out of the cache
    cache->set(key2, image.get(), offset);

    REPORTER_ASSERT(reporter, cache->get(key2, &foundOffset));
    REPORTER_ASSERT(reporter, !cache->get(key1, &foundOffset));
}

// Exercise the purgeByKeys and purge methods
static void test_explicit_purging(skiatest::Reporter* reporter,
                                  const sk_sp<SkSpecialImage>& image,
                                  const sk_sp<SkSpecialImage>& subset) {
    static const size_t kCacheSize = 1000000;
    sk_sp<SkImageFilterCache> cache(SkImageFilterCache::Create(kCacheSize));

    SkIRect clip = SkIRect::MakeWH(100, 100);
    SkImageFilterCacheKey key1(0, SkMatrix::I(), clip, image->uniqueID(), image->subset());
    SkImageFilterCacheKey key2(1, SkMatrix::I(), clip, subset->uniqueID(), image->subset());

    SkIPoint offset = SkIPoint::Make(3, 4);
    cache->set(key1, image.get(), offset);
    cache->set(key2, image.get(), offset);
    SkDEBUGCODE(REPORTER_ASSERT(reporter, 2 == cache->count());)

    SkIPoint foundOffset;

    REPORTER_ASSERT(reporter, cache->get(key1, &foundOffset));
    REPORTER_ASSERT(reporter, cache->get(key2, &foundOffset));

    cache->purgeByKeys(&key1, 1);
    SkDEBUGCODE(REPORTER_ASSERT(reporter, 1 == cache->count());)

    REPORTER_ASSERT(reporter, !cache->get(key1, &foundOffset));
    REPORTER_ASSERT(reporter, cache->get(key2, &foundOffset));

    cache->purge();
    SkDEBUGCODE(REPORTER_ASSERT(reporter, 0 == cache->count());)

    REPORTER_ASSERT(reporter, !cache->get(key1, &foundOffset));
    REPORTER_ASSERT(reporter, !cache->get(key2, &foundOffset));
}

DEF_TEST(ImageFilterCache_RasterBacked, reporter) {
    SkBitmap srcBM = create_bm();

    const SkIRect& full = SkIRect::MakeWH(kFullSize, kFullSize);

    sk_sp<SkSpecialImage> fullImg(SkSpecialImage::MakeFromRaster(full, srcBM));

    const SkIRect& subset = SkIRect::MakeXYWH(kPad, kPad, kSmallerSize, kSmallerSize);

    sk_sp<SkSpecialImage> subsetImg(SkSpecialImage::MakeFromRaster(subset, srcBM));

    test_find_existing(reporter, fullImg, subsetImg);
    test_dont_find_if_diff_key(reporter, fullImg, subsetImg);
    test_internal_purge(reporter, fullImg);
    test_explicit_purging(reporter, fullImg, subsetImg);
}


// Shared test code for both the raster and gpu-backed image cases
static void test_image_backed(skiatest::Reporter* reporter, const sk_sp<SkImage>& srcImage) {
    const SkIRect& full = SkIRect::MakeWH(kFullSize, kFullSize);

    sk_sp<SkSpecialImage> fullImg(
        SkSpecialImage::MakeFromImage(full, srcImage, SkDestinationSurfaceColorMode::kLegacy));

    const SkIRect& subset = SkIRect::MakeXYWH(kPad, kPad, kSmallerSize, kSmallerSize);

    sk_sp<SkSpecialImage> subsetImg(
        SkSpecialImage::MakeFromImage(subset, srcImage, SkDestinationSurfaceColorMode::kLegacy));

    test_find_existing(reporter, fullImg, subsetImg);
    test_dont_find_if_diff_key(reporter, fullImg, subsetImg);
    test_internal_purge(reporter, fullImg);
    test_explicit_purging(reporter, fullImg, subsetImg);
}

DEF_TEST(ImageFilterCache_ImageBackedRaster, reporter) {
    SkBitmap srcBM = create_bm();

    sk_sp<SkImage> srcImage(SkImage::MakeFromBitmap(srcBM));

    test_image_backed(reporter, srcImage);
}

#if SK_SUPPORT_GPU
#include "GrContext.h"

static GrTexture* create_texture(GrContext* context) {
    SkBitmap srcBM = create_bm();

    GrSurfaceDesc desc;
    desc.fConfig = kRGBA_8888_GrPixelConfig;
    desc.fFlags  = kNone_GrSurfaceFlags;
    desc.fWidth  = kFullSize;
    desc.fHeight = kFullSize;

    return context->textureProvider()->createTexture(desc, SkBudgeted::kNo, srcBM.getPixels(), 0);
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ImageFilterCache_ImageBackedGPU, reporter, ctxInfo) {
    sk_sp<GrTexture> srcTexture(create_texture(ctxInfo.grContext()));
    if (!srcTexture) {
        return;
    }

    GrBackendTextureDesc backendDesc;
    backendDesc.fConfig = kRGBA_8888_GrPixelConfig;
    backendDesc.fFlags = kNone_GrBackendTextureFlag;
    backendDesc.fWidth = kFullSize;
    backendDesc.fHeight = kFullSize;
    backendDesc.fSampleCnt = 0;
    backendDesc.fTextureHandle = srcTexture->getTextureHandle();
    sk_sp<SkImage> srcImage(SkImage::MakeFromTexture(ctxInfo.grContext(), backendDesc, kPremul_SkAlphaType));
    if (!srcImage) {
        return;
    }

    test_image_backed(reporter, srcImage);
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ImageFilterCache_GPUBacked, reporter, ctxInfo) {

    sk_sp<GrTexture> srcTexture(create_texture(ctxInfo.grContext()));
    if (!srcTexture) {
        return;
    }

    const SkIRect& full = SkIRect::MakeWH(kFullSize, kFullSize);

    sk_sp<SkSpecialImage> fullImg(SkSpecialImage::MakeFromGpu(full,
                                                              kNeedNewImageUniqueID_SpecialImage,
                                                              srcTexture, nullptr));

    const SkIRect& subset = SkIRect::MakeXYWH(kPad, kPad, kSmallerSize, kSmallerSize);

    sk_sp<SkSpecialImage> subsetImg(SkSpecialImage::MakeFromGpu(subset,
                                                                kNeedNewImageUniqueID_SpecialImage,
                                                                srcTexture, nullptr));

    test_find_existing(reporter, fullImg, subsetImg);
    test_dont_find_if_diff_key(reporter, fullImg, subsetImg);
    test_internal_purge(reporter, fullImg);
    test_explicit_purging(reporter, fullImg, subsetImg);
}
#endif
