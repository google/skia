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
    SkImageInfo ii = SkImageInfo::Make(kFullSize, kFullSize, kRGBA_8888_SkColorType,
                                       kPremul_SkAlphaType);

    SkBitmap bm;
    bm.allocPixels(ii);
    bm.eraseColor(SK_ColorTRANSPARENT);
    bm.setImmutable();
    return bm;
}

static sk_sp<SkImageFilter> make_filter() {
    sk_sp<SkColorFilter> filter(SkColorFilters::Blend(SK_ColorBLUE, SkBlendMode::kSrcIn));
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
    cache->set(key1, filter.get(), skif::FilterResult(image, skif::LayerSpace<SkIPoint>(offset)));

    skif::FilterResult foundImage;
    REPORTER_ASSERT(reporter, cache->get(key1, &foundImage));
    REPORTER_ASSERT(reporter, offset == SkIPoint(foundImage.layerOrigin()));

    REPORTER_ASSERT(reporter, !cache->get(key2, &foundImage));
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
    SkImageFilterCacheKey key2(0, SkMatrix::Translate(5, 5), clip1,
                                   image->uniqueID(), image->subset());
    SkImageFilterCacheKey key3(0, SkMatrix::I(), clip2, image->uniqueID(), image->subset());
    SkImageFilterCacheKey key4(0, SkMatrix::I(), clip1, subset->uniqueID(), subset->subset());

    SkIPoint offset = SkIPoint::Make(3, 4);
    auto filter = make_filter();
    cache->set(key0, filter.get(), skif::FilterResult(image, skif::LayerSpace<SkIPoint>(offset)));

    skif::FilterResult foundImage;
    REPORTER_ASSERT(reporter, !cache->get(key1, &foundImage));
    REPORTER_ASSERT(reporter, !cache->get(key2, &foundImage));
    REPORTER_ASSERT(reporter, !cache->get(key3, &foundImage));
    REPORTER_ASSERT(reporter, !cache->get(key4, &foundImage));
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
    cache->set(key1, filter1.get(), skif::FilterResult(image, skif::LayerSpace<SkIPoint>(offset)));

    skif::FilterResult foundImage;
    REPORTER_ASSERT(reporter, cache->get(key1, &foundImage));

    // This should knock the first one out of the cache
    auto filter2 = make_filter();
    cache->set(key2, filter2.get(),
               skif::FilterResult(image, skif::LayerSpace<SkIPoint>(offset)));

    REPORTER_ASSERT(reporter, cache->get(key2, &foundImage));
    REPORTER_ASSERT(reporter, !cache->get(key1, &foundImage));
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
    cache->set(key1, filter1.get(),
               skif::FilterResult(image, skif::LayerSpace<SkIPoint>(offset)));
    cache->set(key2, filter2.get(),
               skif::FilterResult(image, skif::LayerSpace<SkIPoint>(offset)));
    SkDEBUGCODE(REPORTER_ASSERT(reporter, 2 == cache->count());)

    skif::FilterResult foundImage;
    REPORTER_ASSERT(reporter, cache->get(key1, &foundImage));
    REPORTER_ASSERT(reporter, cache->get(key2, &foundImage));

    cache->purgeByImageFilter(filter1.get());
    SkDEBUGCODE(REPORTER_ASSERT(reporter, 1 == cache->count());)

    REPORTER_ASSERT(reporter, !cache->get(key1, &foundImage));
    REPORTER_ASSERT(reporter, cache->get(key2, &foundImage));

    cache->purge();
    SkDEBUGCODE(REPORTER_ASSERT(reporter, 0 == cache->count());)

    REPORTER_ASSERT(reporter, !cache->get(key1, &foundImage));
    REPORTER_ASSERT(reporter, !cache->get(key2, &foundImage));
}

DEF_TEST(ImageFilterCache_RasterBacked, reporter) {
    SkBitmap srcBM = create_bm();

    const SkIRect& full = SkIRect::MakeWH(kFullSize, kFullSize);

    sk_sp<SkSpecialImage> fullImg(SkSpecialImage::MakeFromRaster(full, srcBM, SkSurfaceProps()));

    const SkIRect& subset = SkIRect::MakeXYWH(kPad, kPad, kSmallerSize, kSmallerSize);

    sk_sp<SkSpecialImage> subsetImg(SkSpecialImage::MakeFromRaster(subset, srcBM,
                                                                   SkSurfaceProps()));

    test_find_existing(reporter, fullImg, subsetImg);
    test_dont_find_if_diff_key(reporter, fullImg, subsetImg);
    test_internal_purge(reporter, fullImg);
    test_explicit_purging(reporter, fullImg, subsetImg);
}


// Shared test code for both the raster and gpu-backed image cases
static void test_image_backed(skiatest::Reporter* reporter,
                              GrRecordingContext* rContext,
                              const sk_sp<SkImage>& srcImage) {
    const SkIRect& full = SkIRect::MakeWH(kFullSize, kFullSize);

    sk_sp<SkSpecialImage> fullImg(SkSpecialImage::MakeFromImage(rContext, full, srcImage,
                                                                SkSurfaceProps()));

    const SkIRect& subset = SkIRect::MakeXYWH(kPad, kPad, kSmallerSize, kSmallerSize);

    sk_sp<SkSpecialImage> subsetImg(SkSpecialImage::MakeFromImage(rContext, subset, srcImage,
                                                                  SkSurfaceProps()));

    test_find_existing(reporter, fullImg, subsetImg);
    test_dont_find_if_diff_key(reporter, fullImg, subsetImg);
    test_internal_purge(reporter, fullImg);
    test_explicit_purging(reporter, fullImg, subsetImg);
}

DEF_TEST(ImageFilterCache_ImageBackedRaster, reporter) {
    SkBitmap srcBM = create_bm();

    sk_sp<SkImage> srcImage(srcBM.asImage());

    test_image_backed(reporter, nullptr, srcImage);
}

#include "include/gpu/GrDirectContext.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrSurfaceProxyPriv.h"
#include "src/gpu/GrTexture.h"
#include "src/gpu/GrTextureProxy.h"
#include "src/gpu/SkGr.h"

static GrSurfaceProxyView create_proxy_view(GrRecordingContext* rContext) {
    SkBitmap srcBM = create_bm();
    return std::get<0>(GrMakeUncachedBitmapProxyView(rContext, srcBM));
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ImageFilterCache_ImageBackedGPU, reporter, ctxInfo) {
    auto dContext = ctxInfo.directContext();

    GrSurfaceProxyView srcView = create_proxy_view(dContext);
    if (!srcView.proxy()) {
        return;
    }

    if (!srcView.proxy()->instantiate(dContext->priv().resourceProvider())) {
        return;
    }
    GrTexture* tex = srcView.proxy()->peekTexture();

    GrBackendTexture backendTex = tex->getBackendTexture();

    GrSurfaceOrigin texOrigin = kTopLeft_GrSurfaceOrigin;
    sk_sp<SkImage> srcImage(SkImage::MakeFromTexture(dContext,
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

    test_image_backed(reporter, dContext, srcImage);
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ImageFilterCache_GPUBacked, reporter, ctxInfo) {
    auto dContext = ctxInfo.directContext();

    GrSurfaceProxyView srcView = create_proxy_view(dContext);
    if (!srcView.proxy()) {
        return;
    }

    const SkIRect& full = SkIRect::MakeWH(kFullSize, kFullSize);

    sk_sp<SkSpecialImage> fullImg(SkSpecialImage::MakeDeferredFromGpu(
                                                              dContext, full,
                                                              kNeedNewImageUniqueID_SpecialImage,
                                                              srcView,
                                                              GrColorType::kRGBA_8888, nullptr,
                                                              SkSurfaceProps()));

    const SkIRect& subset = SkIRect::MakeXYWH(kPad, kPad, kSmallerSize, kSmallerSize);

    sk_sp<SkSpecialImage> subsetImg(SkSpecialImage::MakeDeferredFromGpu(
                                                                dContext, subset,
                                                                kNeedNewImageUniqueID_SpecialImage,
                                                                std::move(srcView),
                                                                GrColorType::kRGBA_8888, nullptr,
                                                                SkSurfaceProps()));

    test_find_existing(reporter, fullImg, subsetImg);
    test_dont_find_if_diff_key(reporter, fullImg, subsetImg);
    test_internal_purge(reporter, fullImg);
    test_explicit_purging(reporter, fullImg, subsetImg);
}
