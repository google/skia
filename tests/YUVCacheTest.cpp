/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkYUVAInfo.h"
#include "include/core/SkYUVAPixmaps.h"
#include "src/core/SkCachedData.h"
#include "src/core/SkResourceCache.h"
#include "src/core/SkYUVPlanesCache.h"
#include "tests/Test.h"

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

DEF_TEST(YUVPlanesCache, reporter) {
    SkResourceCache cache(1024);

    SkYUVAInfo yuvaInfo({5, 5},
                        SkYUVAInfo::PlaneConfig::kY_U_V,
                        SkYUVAInfo::Subsampling::k420,
                        kRec601_Limited_SkYUVColorSpace);
    SkYUVAPixmapInfo yuvaPixmapInfo(yuvaInfo,
                                    SkYUVAPixmapInfo::DataType::kUnorm8,
                                    /*rowBytes[]*/ nullptr);
    SkYUVAPixmaps yuvaPixmaps;
    const uint32_t genID = 12345678;

    SkCachedData* data = SkYUVPlanesCache::FindAndRef(genID, &yuvaPixmaps, &cache);
    REPORTER_ASSERT(reporter, !data);

    size_t size = yuvaPixmapInfo.computeTotalBytes();
    data = cache.newCachedData(size);
    memset(data->writable_data(), 0xff, size);

    SkPixmap pmaps[SkYUVAInfo::kMaxPlanes];
    yuvaPixmapInfo.initPixmapsFromSingleAllocation(data->writable_data(), pmaps);
    yuvaPixmaps = SkYUVAPixmaps::FromExternalPixmaps(yuvaInfo, pmaps);

    SkYUVPlanesCache::Add(genID, data, yuvaPixmaps, &cache);
    check_data(reporter, data, 2, kInCache, kLocked);

    data->unref();
    check_data(reporter, data, 1, kInCache, kUnlocked);

    SkYUVAPixmaps yuvaPixmapsRead;
    data = SkYUVPlanesCache::FindAndRef(genID, &yuvaPixmapsRead, &cache);

    REPORTER_ASSERT(reporter, data);
    REPORTER_ASSERT(reporter, data->size() == size);
    REPORTER_ASSERT(reporter, yuvaPixmapsRead.yuvaInfo() == yuvaPixmaps.yuvaInfo());

    for (int i = 0; i < yuvaPixmaps.numPlanes(); ++i) {
        REPORTER_ASSERT(reporter, yuvaPixmaps.plane(i).info() == yuvaPixmapsRead.plane(i).info());
        REPORTER_ASSERT(reporter, yuvaPixmaps.plane(i).addr() == yuvaPixmapsRead.plane(i).addr());
        REPORTER_ASSERT(reporter, yuvaPixmaps.plane(i).rowBytes() ==
                                  yuvaPixmapsRead.plane(i).rowBytes());
    }

    check_data(reporter, data, 2, kInCache, kLocked);

    cache.purgeAll();
    check_data(reporter, data, 1, kNotInCache, kLocked);
    data->unref();
}
