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

#include <memory>

DEF_TEST(DiscardableMemoryPool, reporter) {
    sk_sp<SkDiscardableMemoryPool> pool(SkDiscardableMemoryPool::Make(1));
    pool->setRAMBudget(3);
    REPORTER_ASSERT(reporter, 0 == pool->getRAMUsed());

    std::unique_ptr<SkDiscardableMemory> dm1(pool->create(100));
    REPORTER_ASSERT(reporter, dm1->data() != nullptr);
    REPORTER_ASSERT(reporter, 100 == pool->getRAMUsed());
    dm1->unlock();
    REPORTER_ASSERT(reporter, 0 == pool->getRAMUsed());
    REPORTER_ASSERT(reporter, !dm1->lock());


    std::unique_ptr<SkDiscardableMemory> dm2(pool->create(200));
    REPORTER_ASSERT(reporter, 200 == pool->getRAMUsed());
    pool->setRAMBudget(400);
    dm2->unlock();
    REPORTER_ASSERT(reporter, 200 == pool->getRAMUsed());
    REPORTER_ASSERT(reporter, dm2->lock());
    dm2->unlock();
    pool->dumpPool();
    REPORTER_ASSERT(reporter, !dm2->lock());
    REPORTER_ASSERT(reporter, 0 == pool->getRAMUsed());
}
