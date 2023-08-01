/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBlurTypes.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "src/base/SkTLazy.h"
#include "src/core/SkCachedData.h"
#include "src/core/SkMask.h"
#include "src/core/SkMaskCache.h"
#include "src/core/SkResourceCache.h"
#include "tests/Test.h"

#include <cstring>

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
    SkTLazy<SkMask> lazyMask;

    SkCachedData* data = SkMaskCache::FindAndRef(sigma, style, rrect, &lazyMask, &cache);
    REPORTER_ASSERT(reporter, nullptr == data);
    REPORTER_ASSERT(reporter, !lazyMask.isValid());

    size_t size = 256;
    data = cache.newCachedData(size);
    memset(data->writable_data(), 0xff, size);
    SkMask mask(nullptr, SkIRect::MakeXYWH(0, 0, 100, 100), 100, SkMask::kBW_Format);
    SkMaskCache::Add(sigma, style, rrect, mask, data, &cache);
    check_data(reporter, data, 2, kInCache, kLocked);

    data->unref();
    check_data(reporter, data, 1, kInCache, kUnlocked);

    lazyMask.reset();
    data = SkMaskCache::FindAndRef(sigma, style, rrect, &lazyMask, &cache);
    REPORTER_ASSERT(reporter, data);
    REPORTER_ASSERT(reporter, data->size() == size);
    REPORTER_ASSERT(reporter, lazyMask->fBounds.top() == 0 && lazyMask->fBounds.bottom() == 100);
    REPORTER_ASSERT(reporter, data->data() == static_cast<const void*>(lazyMask->fImage));
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
    SkTLazy<SkMask> lazyMask;

    SkCachedData* data = SkMaskCache::FindAndRef(sigma, style, rects, 1, &lazyMask, &cache);
    REPORTER_ASSERT(reporter, nullptr == data);
    REPORTER_ASSERT(reporter, !lazyMask.isValid());

    size_t size = 256;
    data = cache.newCachedData(size);
    memset(data->writable_data(), 0xff, size);
    SkMask mask(nullptr, SkIRect::MakeXYWH(0, 0, 100, 100), 100, SkMask::kBW_Format);
    SkMaskCache::Add(sigma, style, rects, 1, mask, data, &cache);
    check_data(reporter, data, 2, kInCache, kLocked);

    data->unref();
    check_data(reporter, data, 1, kInCache, kUnlocked);

    lazyMask.reset();
    data = SkMaskCache::FindAndRef(sigma, style, rects, 1, &lazyMask, &cache);
    REPORTER_ASSERT(reporter, data);
    REPORTER_ASSERT(reporter, data->size() == size);
    REPORTER_ASSERT(reporter, lazyMask->fBounds.top() == 0 && lazyMask->fBounds.bottom() == 100);
    REPORTER_ASSERT(reporter, data->data() == static_cast<const void*>(lazyMask->fImage));
    check_data(reporter, data, 2, kInCache, kLocked);

    cache.purgeAll();
    check_data(reporter, data, 1, kNotInCache, kLocked);
    data->unref();
}
