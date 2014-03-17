/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTObjectPool.h"
#include "SkTObjectPool.h"
#include "Test.h"

class PoolEntry {
public:
private:
    SK_DECLARE_INTERNAL_SLIST_INTERFACE(PoolEntry);
};

static const int kNumItemsPerBlock = 3;
typedef SkTObjectPool<PoolEntry, kNumItemsPerBlock> ObjectPoolType;

static bool verifyPool(skiatest::Reporter* reporter,
                       const ObjectPoolType& pool,
                       const char* stage,
                       int available, int blocks) {
    if (available != pool.available()) {
        ERRORF(reporter, "%s - Pool available is %d not %d",
               stage, pool.available(), available);
        return false;
    }
    if (blocks != pool.blocks()) {
        ERRORF(reporter, "%s - Pool blocks is %d not %d",
               stage, pool.blocks(), blocks);
        return false;
    }
    return true;
}

static const int kNumToAcquire = kNumItemsPerBlock * 5;
static void testObjectPool(skiatest::Reporter* reporter) {
    ObjectPoolType pool;
    SkTInternalSList<PoolEntry> used;
    verifyPool(reporter, pool, "empty", 0, 0);
    for (int index = 0; index < kNumToAcquire; ++index) {
        used.push(pool.acquire());
        int blocks = (index / kNumItemsPerBlock) + 1;
        int available = (blocks * kNumItemsPerBlock) - (index + 1);
        if (!verifyPool(reporter, pool, "acquire", available, blocks)) {
            return;
        }
    }
    int available = pool.available();
    int blocks = pool.blocks();
    for (int index = 0; index < kNumToAcquire / 2; ++index) {
        pool.release(used.pop());
        ++available;
        if (!verifyPool(reporter, pool, "release", available, blocks)) {
            return;
        }
    }
    available += used.getCount();
    pool.releaseAll(&used);
    REPORTER_ASSERT(reporter, used.isEmpty());
    verifyPool(reporter, pool, "releaseAll", available, blocks);
}

DEF_TEST(ObjectPool, reporter) {
    testObjectPool(reporter);
}
