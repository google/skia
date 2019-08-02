 /*
  * Copyright 2016 Google Inc.
  *
  * Use of this source code is governed by a BSD-style license that can be
  * found in the LICENSE file.
  */

#include "tests/Test.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkMatrix.h"
#include "include/effects/SkImageFilters.h"
#include "src/core/SkImageFilterCache.h"
#include "src/core/SkSpecialImage.h"

static const int kSmallerSize = 10;
static const int kPad = 3;
static const int kFullSize = kSmallerSize + 2 * kPad;

static SkBitmap create_bm() {
    SkBitmap bm;
    bm.allocN32Pixels(kFullSize, kFullSize, true);
    bm.eraseColor(SK_ColorTRANSPARENT);
    return bm;
}

static sk_sp<SkImageFilter> make_filter() {
    sk_sp<SkColorFilter> filter(SkColorFilters::Blend(SK_ColorBLUE,
                                                              SkBlendMode::kSrcIn));
    return SkImageFilters::ColorFilter(std::move(filter), nullptr, nullptr);
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
    auto filter = make_filter();
    cache->set(key1, image.get(), offset, filter.get());

    SkIPoint foundOffset;

    sk_sp<SkSpecialImage> foundImage = cache->get(key1, &foundOffset);
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
    auto filter = make_filter();
    cache->set(key0, image.get(), offset, filter.get());

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
    auto filter1 = make_filter();
    cache->set(key1, image.get(), offset, filter1.get());

    SkIPoint foundOffset;

    REPORTER_ASSERT(reporter, cache->get(key1, &foundOffset));

    // This should knock the first one out of the cache
    auto filter2 = make_filter();
    cache->set(key2, image.get(), offset, filter2.get());

    REPORTER_ASSERT(reporter, cache->get(key2, &foundOffset));
    REPORTER_ASSERT(reporter, !cache->get(key1, &foundOffset));
}

// Exercise the purgeByKey and purge methods
static void test_explicit_purging(skiatest::Reporter* reporter,
                                  const sk_sp<SkSpecialImage>& image,
                                  const sk_sp<SkSpecialImage>& subset) {
    static const size_t kCacheSize = 1000000;
    sk_sp<SkImageFilterCache> cache(SkImageFilterCache::Create(kCacheSize));

    SkIRect clip = SkIRect::MakeWH(100, 100);
    SkImageFilterCacheKey key1(0, SkMatrix::I(), clip, image->uniqueID(), image->subset());
    SkImageFilterCacheKey key2(1, SkMatrix::I(), clip, subset->uniqueID(), image->subset());

    SkIPoint offset = SkIPoint::Make(3, 4);
    auto filter1 = make_filter();
    auto filter2 = make_filter();
    cache->set(key1, image.get(), offset, filter1.get());
    cache->set(key2, image.get(), offset, filter2.get());
    SkDEBUGCODE(REPORTER_ASSERT(reporter, 2 == cache->count());)

    SkIPoint foundOffset;

    REPORTER_ASSERT(reporter, cache->get(key1, &foundOffset));
    REPORTER_ASSERT(reporter, cache->get(key2, &foundOffset));

    cache->purgeByImageFilter(filter1.get());
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
static void test_image_backed(skiatest::Reporter* reporter,
                              GrContext* context,
                              const sk_sp<SkImage>& srcImage) {
    const SkIRect& full = SkIRect::MakeWH(kFullSize, kFullSize);

    sk_sp<SkSpecialImage> fullImg(SkSpecialImage::MakeFromImage(context, full, srcImage));

    const SkIRect& subset = SkIRect::MakeXYWH(kPad, kPad, kSmallerSize, kSmallerSize);

    sk_sp<SkSpecialImage> subsetImg(SkSpecialImage::MakeFromImage(context, subset, srcImage));

    test_find_existing(reporter, fullImg, subsetImg);
    test_dont_find_if_diff_key(reporter, fullImg, subsetImg);
    test_internal_purge(reporter, fullImg);
    test_explicit_purging(reporter, fullImg, subsetImg);
}

DEF_TEST(ImageFilterCache_ImageBackedRaster, reporter) {
    SkBitmap srcBM = create_bm();

    sk_sp<SkImage> srcImage(SkImage::MakeFromBitmap(srcBM));

    test_image_backed(reporter, nullptr, srcImage);
}

#include "include/gpu/GrContext.h"
#include "include/gpu/GrTexture.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrSurfaceProxyPriv.h"
#include "src/gpu/GrTextureProxy.h"

static sk_sp<GrTextureProxy> create_proxy(GrProxyProvider* proxyProvider) {
    SkBitmap srcBM = create_bm();
    sk_sp<SkImage> srcImage(SkImage::MakeFromBitmap(srcBM));
    return proxyProvider->createTextureProxy(srcImage, GrRenderable::kNo, 1, SkBudgeted::kYes,
                                             SkBackingFit::kExact);
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ImageFilterCache_ImageBackedGPU, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();

    sk_sp<GrTextureProxy> srcProxy(create_proxy(context->priv().proxyProvider()));
    if (!srcProxy) {
        return;
    }

    if (!srcProxy->instantiate(context->priv().resourceProvider())) {
        return;
    }
    GrTexture* tex = srcProxy->peekTexture();

    GrBackendTexture backendTex = tex->getBackendTexture();

    GrSurfaceOrigin texOrigin = kTopLeft_GrSurfaceOrigin;
    sk_sp<SkImage> srcImage(SkImage::MakeFromTexture(context,
                                                     backendTex,
                                                     texOrigin,
                                                     kRGBA_8888_SkColorType,
                                                     kPremul_SkAlphaType, nullptr,
                                                     nullptr, nullptr));
    if (!srcImage) {
        return;
    }

    GrSurfaceOrigin readBackOrigin;
    GrBackendTexture readBackBackendTex = srcImage->getBackendTexture(false, &readBackOrigin);
    if (!GrBackendTexture::TestingOnly_Equals(readBackBackendTex, backendTex)) {
        ERRORF(reporter, "backend mismatch\n");
    }
    REPORTER_ASSERT(reporter, GrBackendTexture::TestingOnly_Equals(readBackBackendTex, backendTex));

    if (readBackOrigin != texOrigin) {
        ERRORF(reporter, "origin mismatch %d %d\n", readBackOrigin, texOrigin);
    }
    REPORTER_ASSERT(reporter, readBackOrigin == texOrigin);

    test_image_backed(reporter, context, srcImage);
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ImageFilterCache_GPUBacked, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();

    sk_sp<GrTextureProxy> srcProxy(create_proxy(context->priv().proxyProvider()));
    if (!srcProxy) {
        return;
    }

    const SkIRect& full = SkIRect::MakeWH(kFullSize, kFullSize);

    sk_sp<SkSpecialImage> fullImg(SkSpecialImage::MakeDeferredFromGpu(
                                                              context, full,
                                                              kNeedNewImageUniqueID_SpecialImage,
                                                              srcProxy, nullptr));

    const SkIRect& subset = SkIRect::MakeXYWH(kPad, kPad, kSmallerSize, kSmallerSize);

    sk_sp<SkSpecialImage> subsetImg(SkSpecialImage::MakeDeferredFromGpu(
                                                                context, subset,
                                                                kNeedNewImageUniqueID_SpecialImage,
                                                                srcProxy, nullptr));

    test_find_existing(reporter, fullImg, subsetImg);
    test_dont_find_if_diff_key(reporter, fullImg, subsetImg);
    test_internal_purge(reporter, fullImg);
    test_explicit_purging(reporter, fullImg, subsetImg);
}
