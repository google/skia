/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a GPU-backend specific test
#if SK_SUPPORT_GPU

#include "GrBinHashKey.h"

#include "Test.h"

DEF_TEST(GrBinHashKeyTest, reporter) {
    const char* testStringA_ = "abcdABCD";
    const char* testStringB_ = "abcdBBCD";
    const uint32_t* testStringA = reinterpret_cast<const uint32_t*>(testStringA_);
    const uint32_t* testStringB = reinterpret_cast<const uint32_t*>(testStringB_);
    enum {
        kDataLenUsedForKey = 8
    };

    GrBinHashKey<kDataLenUsedForKey> keyA;
    keyA.setKeyData(testStringA);
    // test copy constructor and comparison
    GrBinHashKey<kDataLenUsedForKey> keyA2(keyA);
    REPORTER_ASSERT(reporter, keyA == keyA2);
    REPORTER_ASSERT(reporter, keyA.getHash() == keyA2.getHash());
    // test re-init
    keyA2.setKeyData(testStringA);
    REPORTER_ASSERT(reporter, keyA == keyA2);
    REPORTER_ASSERT(reporter, keyA.getHash() == keyA2.getHash());
    // test sorting
    GrBinHashKey<kDataLenUsedForKey> keyB;
    keyB.setKeyData(testStringB);
    REPORTER_ASSERT(reporter, keyA < keyB);
    REPORTER_ASSERT(reporter, keyA.getHash() != keyB.getHash());
}

#endif
