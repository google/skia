/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a GPU-backend specific test
#if SK_SUPPORT_GPU

#include "GrMurmur3HashKey.h"
#include "GrBinHashKey.h"

#include "Test.h"

struct AlignedKey {
    uint32_t align;
    char key[8];
};

template<typename KeyType> static void TestHash(skiatest::Reporter* reporter) {
    AlignedKey a, b;
    memcpy(&a.key, "abcdABCD", 8);
    memcpy(&b.key, "abcdBBCD", 8);
    const uint32_t* testStringA = reinterpret_cast<const uint32_t*>(&a.key);
    const uint32_t* testStringB = reinterpret_cast<const uint32_t*>(&b.key);

    KeyType keyA;
    keyA.setKeyData(testStringA);
    // test copy constructor and comparison
    KeyType keyA2(keyA);
    REPORTER_ASSERT(reporter, keyA == keyA2);
    REPORTER_ASSERT(reporter, keyA.getHash() == keyA2.getHash());
    // test re-init
    keyA2.setKeyData(testStringA);
    REPORTER_ASSERT(reporter, keyA == keyA2);
    REPORTER_ASSERT(reporter, keyA.getHash() == keyA2.getHash());
    // test sorting
    KeyType keyB;
    keyB.setKeyData(testStringB);
    REPORTER_ASSERT(reporter, keyA.getHash() != keyB.getHash());
}


DEF_TEST(GrBinHashKey, reporter) {
    enum {
        kDataLenUsedForKey = 8
    };

    TestHash<GrBinHashKey<kDataLenUsedForKey> >(reporter);
    TestHash<GrMurmur3HashKey<kDataLenUsedForKey> >(reporter);
}

#endif
