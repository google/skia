/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "include/private/SkMalloc.h"
#include "src/core/SkCachedData.h"
#include "src/lazy/SkDiscardableMemoryPool.h"
#include "tests/Test.h"

#include <cstring>

class SkDiscardableMemory;

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
    REPORTER_ASSERT(reporter, data->testing_only_isLocked() == (lockedState == kLocked));
}

static SkCachedData* make_data(size_t size, SkDiscardableMemoryPool* pool) {
    if (pool) {
        SkDiscardableMemory* dm = pool->create(size);
        // the pool "can" return null, but it shouldn't in these controlled conditions
        SkASSERT_RELEASE(dm);
        return new SkCachedData(size, dm);
    } else {
        return new SkCachedData(sk_malloc_throw(size), size);
    }
}

// returns with the data locked by client and cache
static SkCachedData* test_locking(skiatest::Reporter* reporter,
                                  size_t size, SkDiscardableMemoryPool* pool) {
    SkCachedData* data = make_data(size, pool);

    memset(data->writable_data(), 0x80, size);  // just to use writable_data()

    check_data(reporter, data, 1, kNotInCache, kLocked);

    data->ref();
    check_data(reporter, data, 2, kNotInCache, kLocked);
    data->unref();
    check_data(reporter, data, 1, kNotInCache, kLocked);

    data->attachToCacheAndRef();
    check_data(reporter, data, 2, kInCache, kLocked);

    data->unref();
    check_data(reporter, data, 1, kInCache, kUnlocked);

    data->ref();
    check_data(reporter, data, 2, kInCache, kLocked);

    return data;
}

/*
 *  SkCachedData behaves differently (regarding its locked/unlocked state) depending on
 *  when it is in the cache or not. Being in the cache is signaled by calling attachToCacheAndRef()
 *  instead of ref(). (and balanced by detachFromCacheAndUnref).
 *
 *  Thus, among other things, we test the end-of-life behavior when the client is the last owner
 *  and when the cache is.
 */
DEF_TEST(CachedData, reporter) {
    sk_sp<SkDiscardableMemoryPool> pool(SkDiscardableMemoryPool::Make(1000));

    for (int useDiscardable = 0; useDiscardable <= 1; ++useDiscardable) {
        const size_t size = 100;

        // test with client as last owner
        SkCachedData* data = test_locking(reporter, size, useDiscardable ? pool.get() : nullptr);
        check_data(reporter, data, 2, kInCache, kLocked);
        data->detachFromCacheAndUnref();
        check_data(reporter, data, 1, kNotInCache, kLocked);
        data->unref();

        // test with cache as last owner
        data = test_locking(reporter, size, useDiscardable ? pool.get() : nullptr);
        check_data(reporter, data, 2, kInCache, kLocked);
        data->unref();
        check_data(reporter, data, 1, kInCache, kUnlocked);
        data->detachFromCacheAndUnref();
    }
}
