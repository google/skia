/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkRefCnt.h"
#include "src/core/SkDiscardableMemory.h"
#include "src/lazy/SkDiscardableMemoryPool.h"
#include "tests/Test.h"

#include <cstring>
#include <memory>

namespace {
constexpr char kTestString[] = "HELLO, WORLD!";
constexpr size_t kTestStringLength = sizeof(kTestString);
}

static void test_dm(skiatest::Reporter* reporter,
                    SkDiscardableMemory* dm,
                    bool assertRelock) {
    REPORTER_ASSERT(reporter, dm);
    if (!dm) {
        return;
    }
    void* ptr = dm->data();
    REPORTER_ASSERT(reporter, ptr);
    if (!ptr) {
        return;
    }
    memcpy(ptr, kTestString, sizeof(kTestString));
    dm->unlock();
    bool relockSuccess = dm->lock();
    if (assertRelock) {
        REPORTER_ASSERT(reporter, relockSuccess);
    }
    if (!relockSuccess) {
        return;
    }
    ptr = dm->data();
    REPORTER_ASSERT(reporter, ptr);
    if (!ptr) {
        return;
    }
    REPORTER_ASSERT(reporter, 0 == memcmp(ptr, kTestString, kTestStringLength));
    dm->unlock();
}

DEF_TEST(DiscardableMemory_global, reporter) {
    std::unique_ptr<SkDiscardableMemory> dm(SkDiscardableMemory::Create(kTestStringLength));
    // lock() test is allowed to fail, since other threads could be
    // using global pool.
    test_dm(reporter, dm.get(), false);
}

DEF_TEST(DiscardableMemory_nonglobal, reporter) {
    sk_sp<SkDiscardableMemoryPool> pool(
        SkDiscardableMemoryPool::Make(1024));
    std::unique_ptr<SkDiscardableMemory> dm(pool->create(kTestStringLength));
    test_dm(reporter, dm.get(), true);
}

