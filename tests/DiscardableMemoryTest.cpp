/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDiscardableMemory.h"

#include "Test.h"
#include "TestClassDef.h"

DEF_TEST(DiscardableMemory, reporter) {
    const char testString[] = "HELLO, WORLD!";
    const size_t len = sizeof(testString);
    SkAutoTDelete<SkDiscardableMemory> dm(SkDiscardableMemory::Create(len));
    REPORTER_ASSERT(reporter, dm.get() != NULL);
    if (NULL == dm.get()) {
        return;
    }
    void* ptr = dm->data();
    REPORTER_ASSERT(reporter, ptr != NULL);
    memcpy(ptr, testString, sizeof(testString));
    dm->unlock();
    bool success = dm->lock();
    REPORTER_ASSERT(reporter, success);
    if (!success) {
        return;
    }
    ptr = dm->data();
    REPORTER_ASSERT(reporter, 0 == memcmp(ptr, testString, len));
    dm->unlock();
}
