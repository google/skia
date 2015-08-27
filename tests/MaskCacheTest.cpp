/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCachedData.h"
#include "SkMaskCache.h"
#include "SkResourceCache.h"
#include "Test.h"

enum LockedState {
    kUnlocked,
    kLocked,
};

enum CachedState {
    kNotInCache,
    kInCache,
};

static void check_data(skiatest::Reporter* reporter, SkCachedData* data,
                       int refcnt, CachedState cacheState, LockedState lockedState) {
    REPORTER_ASSERT(reporter, data->testing_only_getRefCnt() == refcnt);
    REPORTER_ASSERT(reporter, data->testing_only_isInCache() == (kInCache == cacheState));
    bool isLocked = (data->data() != nullptr);
    REPORTER_ASSERT(reporter, isLocked == (lockedState == kLocked));
}

DEF_TEST(RRectMaskCache, reporter) {
    SkResourceCache cache(1024);

    SkScalar sigma = 0.8f;
    SkRect rect = SkRect::MakeWH(100, 100);
    SkRRect rrect;
    rrect.setRectXY(rect, 30, 30);
    SkBlurStyle style = kNormal_SkBlurStyle;
    SkBlurQuality quality = kLow_SkBlurQuality;
    SkMask mask;

    SkCachedData* data = SkMaskCache::FindAndRef(sigma, style, quality, rrect, &mask, &cache);
    REPORTER_ASSERT(reporter, nullptr == data);

    size_t size = 256;
    data = cache.newCachedData(size);
    memset(data->writable_data(), 0xff, size);
    mask.fBounds.setXYWH(0, 0, 100, 100);
    mask.fRowBytes = 100;
    mask.fFormat = SkMask::kBW_Format;
    SkMaskCache::Add(sigma, style, quality, rrect, mask, data, &cache);
    check_data(reporter, data, 2, kInCache, kLocked);

    data->unref();
    check_data(reporter, data, 1, kInCache, kUnlocked);

    sk_bzero(&mask, sizeof(mask));
    data = SkMaskCache::FindAndRef(sigma, style, quality, rrect, &mask, &cache);
    REPORTER_ASSERT(reporter, data);
    REPORTER_ASSERT(reporter, data->size() == size);
    REPORTER_ASSERT(reporter, mask.fBounds.top() == 0 && mask.fBounds.bottom() == 100);
    REPORTER_ASSERT(reporter, data->data() == (const void*)mask.fImage);
    check_data(reporter, data, 2, kInCache, kLocked);

    cache.purgeAll();
    check_data(reporter, data, 1, kNotInCache, kLocked);
    data->unref();
}

DEF_TEST(RectsMaskCache, reporter) {
    SkResourceCache cache(1024);

    SkScalar sigma = 0.8f;
    SkRect rect = SkRect::MakeWH(100, 100);
    SkRect rects[2] = {rect};
    SkBlurStyle style = kNormal_SkBlurStyle;
    SkBlurQuality quality = kLow_SkBlurQuality;
    SkMask mask;

    SkCachedData* data = SkMaskCache::FindAndRef(sigma, style, quality, rects, 1, &mask, &cache);
    REPORTER_ASSERT(reporter, nullptr == data);

    size_t size = 256;
    data = cache.newCachedData(size);
    memset(data->writable_data(), 0xff, size);
    mask.fBounds.setXYWH(0, 0, 100, 100);
    mask.fRowBytes = 100;
    mask.fFormat = SkMask::kBW_Format;
    SkMaskCache::Add(sigma, style, quality, rects, 1, mask, data, &cache);
    check_data(reporter, data, 2, kInCache, kLocked);

    data->unref();
    check_data(reporter, data, 1, kInCache, kUnlocked);

    sk_bzero(&mask, sizeof(mask));
    data = SkMaskCache::FindAndRef(sigma, style, quality, rects, 1, &mask, &cache);
    REPORTER_ASSERT(reporter, data);
    REPORTER_ASSERT(reporter, data->size() == size);
    REPORTER_ASSERT(reporter, mask.fBounds.top() == 0 && mask.fBounds.bottom() == 100);
    REPORTER_ASSERT(reporter, data->data() == (const void*)mask.fImage);
    check_data(reporter, data, 2, kInCache, kLocked);

    cache.purgeAll();
    check_data(reporter, data, 1, kNotInCache, kLocked);
    data->unref();
}
