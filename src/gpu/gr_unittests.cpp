
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#include "GrBinHashKey.h"
#include "GrDrawTarget.h"
#include "GrMatrix.h"
#include "GrPath.h"
#include "GrRedBlackTree.h"
#include "GrTDArray.h"

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

static void dump(const GrTDArray<int>& array) {
#if 0
    for (int i = 0; i < array.count(); i++) {
        printf(" %d", array[i]);
    }
    printf("\n");
#endif
}

static void test_tdarray() {
    GrTDArray<int> array;

    *array.append() = 0; dump(array);
    *array.append() = 2; dump(array);
    *array.append() = 4; dump(array);
    *array.append() = 6; dump(array);
    GrAssert(array.count() == 4);

    *array.insert(0) = -1; dump(array);
    *array.insert(2) = 1; dump(array);
    *array.insert(4) = 3; dump(array);
    *array.insert(7) = 7; dump(array);
    GrAssert(array.count() == 8);
    array.remove(3); dump(array);
    array.remove(0); dump(array);
    array.removeShuffle(4); dump(array);
    array.removeShuffle(1); dump(array);
    GrAssert(array.count() == 4);
}


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

    GrBinHashKey<BogusEntry, kDataLenUsedForKey> keyA;
    keyA.setKeyData(testStringA);
    // test copy constructor and comparison
    GrBinHashKey<BogusEntry, kDataLenUsedForKey> keyA2(keyA);
    GrAssert(keyA.compare(keyA2) == 0);
    GrAssert(keyA.getHash() == keyA2.getHash());
    // test re-init
    keyA2.setKeyData(testStringA);
    GrAssert(keyA.compare(keyA2) == 0);
    GrAssert(keyA.getHash() == keyA2.getHash());
    // test sorting
    GrBinHashKey<BogusEntry, kDataLenUsedForKey> keyB;
    keyB.setKeyData(testStringB);
    GrAssert(keyA.compare(keyB) < 0);
    GrAssert(keyA.getHash() != keyB.getHash());    
}

static void test_convex() {
#if 0
    GrPath testPath;
    GrPath::Iter testIter;
    
    GrPath pt;
    pt.moveTo(0, 0);
    pt.close();
    
    testIter.reset(pt);
    testPath.resetFromIter(&testIter);
    GrAssert(kConvex_ConvexHint == testPath.getConvexHint());
    
    GrPath line;
    line.moveTo(GrIntToScalar(12), GrIntToScalar(20));
    line.lineTo(GrIntToScalar(-12), GrIntToScalar(-20));
    line.close();
    
    testIter.reset(line);
    testPath.resetFromIter(&testIter);
    GrAssert(kConvex_ConvexHint == testPath.getConvexHint());
    
    GrPath triLeft;
    triLeft.moveTo(0, 0);
    triLeft.lineTo(1, 0);
    triLeft.lineTo(1, 1);
    triLeft.close();
    
    testIter.reset(triLeft);
    testPath.resetFromIter(&testIter);
    GrAssert(kConvex_ConvexHint == testPath.getConvexHint());
    
    GrPath triRight;
    triRight.moveTo(0, 0);
    triRight.lineTo(-1, 0);
    triRight.lineTo(1, 1);
    triRight.close();
    
    testIter.reset(triRight);
    testPath.resetFromIter(&testIter);
    GrAssert(kConvex_ConvexHint == testPath.getConvexHint());
    
    GrPath square;
    square.moveTo(0, 0);
    square.lineTo(1, 0);
    square.lineTo(1, 1);
    square.lineTo(0, 1);
    square.close();
    
    testIter.reset(square);
    testPath.resetFromIter(&testIter);
    GrAssert(kConvex_ConvexHint == testPath.getConvexHint());
    
    GrPath redundantSquare;
    square.moveTo(0, 0);
    square.lineTo(0, 0);
    square.lineTo(0, 0);
    square.lineTo(1, 0);
    square.lineTo(1, 0);
    square.lineTo(1, 0);
    square.lineTo(1, 1);
    square.lineTo(1, 1);
    square.lineTo(1, 1);
    square.lineTo(0, 1);
    square.lineTo(0, 1);
    square.lineTo(0, 1);
    square.close();
    
    testIter.reset(redundantSquare);
    testPath.resetFromIter(&testIter);
    GrAssert(kConvex_ConvexHint == testPath.getConvexHint());
    
    GrPath bowTie;
    bowTie.moveTo(0, 0);
    bowTie.lineTo(0, 0);
    bowTie.lineTo(0, 0);
    bowTie.lineTo(1, 1);
    bowTie.lineTo(1, 1);
    bowTie.lineTo(1, 1);
    bowTie.lineTo(1, 0);
    bowTie.lineTo(1, 0);
    bowTie.lineTo(1, 0);
    bowTie.lineTo(0, 1);
    bowTie.lineTo(0, 1);
    bowTie.lineTo(0, 1);
    bowTie.close();
    
    testIter.reset(bowTie);
    testPath.resetFromIter(&testIter);
    GrAssert(kConcave_ConvexHint == testPath.getConvexHint());
    
    GrPath spiral;
    spiral.moveTo(0, 0);
    spiral.lineTo(1, 0);
    spiral.lineTo(1, 1);
    spiral.lineTo(0, 1);
    spiral.lineTo(0,.5);
    spiral.lineTo(.5,.5);
    spiral.lineTo(.5,.75);
    spiral.close();
    
    testIter.reset(spiral);
    testPath.resetFromIter(&testIter);
    GrAssert(kConcave_ConvexHint == testPath.getConvexHint());
    
    GrPath dent;
    dent.moveTo(0, 0);
    dent.lineTo(1, 1);
    dent.lineTo(0, 1);
    dent.lineTo(-.5,2);
    dent.lineTo(-2, 1);
    dent.close();
    
    testIter.reset(dent);
    testPath.resetFromIter(&testIter);
    GrAssert(kConcave_ConvexHint == testPath.getConvexHint());
#endif
}

void gr_run_unittests() {
    test_tdarray();
    test_bsearch();
    test_binHashKey();
    test_convex();
    GrRedBlackTree<int>::UnitTest();
    GrDrawTarget::VertexLayoutUnitTest();
}
