 /*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkDiscardableMemory.h"
#include "src/core/SkResourceCache.h"
#include "tests/Test.h"

namespace {
static void* gGlobalAddress;
struct TestingKey : public SkResourceCache::Key {
    intptr_t    fValue;

    TestingKey(intptr_t value, uint64_t sharedID = 0) : fValue(value) {
        this->init(&gGlobalAddress, sharedID, sizeof(fValue));
    }
};
struct TestingRec : public SkResourceCache::Rec {
    TestingRec(const TestingKey& key, uint32_t value) : fKey(key), fValue(value) {}

    TestingKey  fKey;
    intptr_t    fValue;

    const Key& getKey() const override { return fKey; }
    size_t bytesUsed() const override { return sizeof(fKey) + sizeof(fValue); }
    const char* getCategory() const override { return "test_cache"; }
    SkDiscardableMemory* diagnostic_only_getDiscardable() const override { return nullptr; }

    static bool Visitor(const SkResourceCache::Rec& baseRec, void* context) {
        const TestingRec& rec = static_cast<const TestingRec&>(baseRec);
        intptr_t* result = (intptr_t*)context;

        *result = rec.fValue;
        return true;
    }
};
}

static const int COUNT = 10;
static const int DIM = 256;

static void test_cache(skiatest::Reporter* reporter, SkResourceCache& cache, bool testPurge) {
    for (int i = 0; i < COUNT; ++i) {
        TestingKey key(i);
        intptr_t value = -1;

        REPORTER_ASSERT(reporter, !cache.find(key, TestingRec::Visitor, &value));
        REPORTER_ASSERT(reporter, -1 == value);

        cache.add(new TestingRec(key, i));

        REPORTER_ASSERT(reporter, cache.find(key, TestingRec::Visitor, &value));
        REPORTER_ASSERT(reporter, i == value);
    }

    if (testPurge) {
        // stress test, should trigger purges
        for (int i = 0; i < COUNT * 100; ++i) {
            TestingKey key(i);
            cache.add(new TestingRec(key, i));
        }
    }

    // test the originals after all that purging
    for (int i = 0; i < COUNT; ++i) {
        intptr_t value;
        (void)cache.find(TestingKey(i), TestingRec::Visitor, &value);
    }

    cache.setTotalByteLimit(0);
}

static void test_cache_purge_shared_id(skiatest::Reporter* reporter, SkResourceCache& cache) {
    for (int i = 0; i < COUNT; ++i) {
        TestingKey key(i, i & 1);   // every other key will have a 1 for its sharedID
        cache.add(new TestingRec(key, i));
    }

    // Ensure that everyone is present
    for (int i = 0; i < COUNT; ++i) {
        TestingKey key(i, i & 1);   // every other key will have a 1 for its sharedID
        intptr_t value = -1;

        REPORTER_ASSERT(reporter, cache.find(key, TestingRec::Visitor, &value));
        REPORTER_ASSERT(reporter, value == i);
    }

    // Now purge the ones that had a non-zero sharedID (the odd-indexed ones)
    cache.purgeSharedID(1);

    // Ensure that only the even ones are still present
    for (int i = 0; i < COUNT; ++i) {
        TestingKey key(i, i & 1);   // every other key will have a 1 for its sharedID
        intptr_t value = -1;

        if (i & 1) {
            REPORTER_ASSERT(reporter, !cache.find(key, TestingRec::Visitor, &value));
        } else {
            REPORTER_ASSERT(reporter, cache.find(key, TestingRec::Visitor, &value));
            REPORTER_ASSERT(reporter, value == i);
        }
    }
}

#include "src/lazy/SkDiscardableMemoryPool.h"

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
        sk_sp<SkDiscardableMemoryPool> pool(SkDiscardableMemoryPool::Make(defLimit));
        gPool = pool.get();
        SkResourceCache cache(pool_factory);
        test_cache(reporter, cache, true);
    }
    {
        SkResourceCache cache(SkDiscardableMemory::Create);
        test_cache(reporter, cache, false);
    }
    {
        SkResourceCache cache(defLimit);
        test_cache_purge_shared_id(reporter, cache);
    }
}

DEF_TEST(ImageCache_doubleAdd, r) {
    // Adding the same key twice should be safe.
    SkResourceCache cache(4096);

    TestingKey key(1);

    cache.add(new TestingRec(key, 2));
    cache.add(new TestingRec(key, 3));

    // Lookup can return either value.
    intptr_t value = -1;
    REPORTER_ASSERT(r, cache.find(key, TestingRec::Visitor, &value));
    REPORTER_ASSERT(r, 2 == value || 3 == value);
}
