
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrBinHashKey.h"
#include "GrDrawTarget.h"
#include "SkMatrix.h"
#include "GrRedBlackTree.h"

// FIXME: needs to be in a header
void gr_run_unittests();

// If we aren't inheriting these as #defines from elsewhere,
// clang demands they be declared before we #include the template
// that relies on them.
#if GR_DEBUG
static bool LT(const int& elem, int value) {
    return elem < value;
}
static bool EQ(const int& elem, int value) {
    return elem == value;
}
#include "GrTBSearch.h"

static void test_bsearch() {
    const int array[] = {
        1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 22, 33, 44, 55, 66, 77, 88, 99
    };

    for (size_t n = 0; n < GR_ARRAY_COUNT(array); n++) {
        for (size_t i = 0; i < n; i++) {
            int index = GrTBSearch<int, int>(array, n, array[i]);
            GrAssert(index == (int) i);
            index = GrTBSearch<int, int>(array, n, -array[i]);
            GrAssert(index < 0);
        }
    }
}
#endif

// bogus empty class for GrBinHashKey
class BogusEntry {};

static void test_binHashKey()
{
    const char* testStringA_ = "abcdABCD";
    const char* testStringB_ = "abcdBBCD";
    const uint32_t* testStringA = reinterpret_cast<const uint32_t*>(testStringA_);
    const uint32_t* testStringB = reinterpret_cast<const uint32_t*>(testStringB_);
    enum {
        kDataLenUsedForKey = 8
    };

    GrTBinHashKey<BogusEntry, kDataLenUsedForKey> keyA;
    keyA.setKeyData(testStringA);
    // test copy constructor and comparison
    GrTBinHashKey<BogusEntry, kDataLenUsedForKey> keyA2(keyA);
    GrAssert(keyA.compare(keyA2) == 0);
    GrAssert(keyA.getHash() == keyA2.getHash());
    // test re-init
    keyA2.setKeyData(testStringA);
    GrAssert(keyA.compare(keyA2) == 0);
    GrAssert(keyA.getHash() == keyA2.getHash());
    // test sorting
    GrTBinHashKey<BogusEntry, kDataLenUsedForKey> keyB;
    keyB.setKeyData(testStringB);
    GrAssert(keyA.compare(keyB) < 0);
    GrAssert(keyA.getHash() != keyB.getHash());
}


void gr_run_unittests() {
    GR_DEBUGCODE(test_bsearch();)
    test_binHashKey();
    GrRedBlackTree<int>::UnitTest();
}
