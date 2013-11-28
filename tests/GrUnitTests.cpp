
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Test.h"
#include "TestClassDef.h"

// This is a GPU-backend specific test
#if SK_SUPPORT_GPU
#include "GrBinHashKey.h"
#include "GrDrawTarget.h"
#include "SkMatrix.h"
#include "GrRedBlackTree.h"

// If we aren't inheriting these as #defines from elsewhere,
// clang demands they be declared before we #include the template
// that relies on them.
static bool LT(const int& elem, int value) {
    return elem < value;
}
static bool EQ(const int& elem, int value) {
    return elem == value;
}
#include "GrTBSearch.h"


DEF_TEST(GrUnitTests_bsearch, reporter) {
    const int array[] = {
        1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 22, 33, 44, 55, 66, 77, 88, 99
    };

    for (int n = 0; n < static_cast<int>(GR_ARRAY_COUNT(array)); ++n) {
        for (int i = 0; i < n; i++) {
            int index = GrTBSearch<int, int>(array, n, array[i]);
            REPORTER_ASSERT(reporter, index == (int) i);
            index = GrTBSearch<int, int>(array, n, -array[i]);
            REPORTER_ASSERT(reporter, index < 0);
        }
    }
}

DEF_TEST(GrUnitTests_binHashKey, reporter) {
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


DEF_TEST(GrUnitTests_redBlackTree, reporter) {
    // TODO(mtklein): unwrap this and use reporter.
    GrRedBlackTree<int>::UnitTest();
}

#endif
