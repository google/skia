 /*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDiscardableMemory.h"
#include "SkResourceCache.h"
#include "Test.h"

namespace {
static void* gGlobalAddress;
struct TestingKey : public SkResourceCache::Key {
    void*       fPtr;
    intptr_t    fValue;

    TestingKey(intptr_t value) : fPtr(&gGlobalAddress), fValue(value) {
        this->init(sizeof(fPtr) + sizeof(fValue));
    }
};
struct TestingRec : public SkResourceCache::Rec {
    TestingRec(const TestingKey& key, uint32_t value) : fKey(key), fValue(value) {}

    TestingKey  fKey;
    intptr_t    fValue;

    virtual const Key& getKey() const SK_OVERRIDE { return fKey; }
    virtual size_t bytesUsed() const SK_OVERRIDE { return sizeof(fKey) + sizeof(fValue); }
};
}

static const int COUNT = 10;
static const int DIM = 256;

static void test_cache(skiatest::Reporter* reporter, SkResourceCache& cache,
                       bool testPurge) {
    SkResourceCache::ID id;

    for (int i = 0; i < COUNT; ++i) {
        TestingKey key(i);

        const TestingRec* rec = (const TestingRec*)cache.findAndLock(key);
        REPORTER_ASSERT(reporter, NULL == rec);

        TestingRec* newRec = SkNEW_ARGS(TestingRec, (key, i));
        const TestingRec* addedRec = (const TestingRec*)cache.addAndLock(newRec);
        REPORTER_ASSERT(reporter, NULL != addedRec);

        const TestingRec* foundRec = (const TestingRec*)cache.findAndLock(key);
        REPORTER_ASSERT(reporter, foundRec == addedRec);
        REPORTER_ASSERT(reporter, foundRec->fValue == i);
        cache.unlock(foundRec);
        cache.unlock(addedRec);
    }

    if (testPurge) {
        // stress test, should trigger purges
        for (size_t i = 0; i < COUNT * 100; ++i) {
            TestingKey key(i);
            SkResourceCache::ID id = cache.addAndLock(SkNEW_ARGS(TestingRec, (key, i)));
            REPORTER_ASSERT(reporter, NULL != id);
            cache.unlock(id);
        }
    }

    // test the originals after all that purging
    for (int i = 0; i < COUNT; ++i) {
        id = cache.findAndLock(TestingKey(i));
        if (id) {
            cache.unlock(id);
        }
    }

    cache.setTotalByteLimit(0);
}

#include "SkDiscardableMemoryPool.h"

static SkDiscardableMemoryPool* gPool;
static SkDiscardableMemory* pool_factory(size_t bytes) {
    SkASSERT(gPool);
    return gPool->create(bytes);
}

DEF_TEST(ImageCache, reporter) {
    static const size_t defLimit = DIM * DIM * 4 * COUNT + 1024;    // 1K slop

    {
        SkResourceCache cache(defLimit);
        test_cache(reporter, cache, true);
    }
    {
        SkAutoTUnref<SkDiscardableMemoryPool> pool(
                SkDiscardableMemoryPool::Create(defLimit, NULL));
        gPool = pool.get();
        SkResourceCache cache(pool_factory);
        test_cache(reporter, cache, true);
    }
    {
        SkResourceCache cache(SkDiscardableMemory::Create);
        test_cache(reporter, cache, false);
    }
}

DEF_TEST(ImageCache_doubleAdd, r) {
    // Adding the same key twice should be safe.
    SkResourceCache cache(4096);

    TestingKey key(1);

    SkResourceCache::ID id1 = cache.addAndLock(SkNEW_ARGS(TestingRec, (key, 2)));
    SkResourceCache::ID id2 = cache.addAndLock(SkNEW_ARGS(TestingRec, (key, 3)));
    // We don't really care if id1 == id2 as long as unlocking both works.
    cache.unlock(id1);
    cache.unlock(id2);

    // Lookup can return either value.
    const TestingRec* rec = (const TestingRec*)cache.findAndLock(key);
    REPORTER_ASSERT(r, NULL != rec);
    REPORTER_ASSERT(r, 2 == rec->fValue || 3 == rec->fValue);
    cache.unlock(rec);
}
