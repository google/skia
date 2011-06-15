/*
    Copyright 2010 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

         http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
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
    const char* testStringA = "abcdABCD";
    const char* testStringB = "abcdBBCD";
    enum {
        kDataLenUsedForKey = 8
    };

    typedef GrBinHashKey<BogusEntry, kDataLenUsedForKey> KeyType;

    KeyType keyA;
    int passCnt = 0;
    while (keyA.doPass()) {
        ++passCnt;
        keyA.keyData(reinterpret_cast<const uint32_t*>(testStringA), kDataLenUsedForKey);
    }
    GrAssert(passCnt == 1); //We expect the static allocation to suffice
    GrBinHashKey<BogusEntry, kDataLenUsedForKey-1> keyBust;
    passCnt = 0;
    while (keyBust.doPass()) {
        ++passCnt;
        // Exceed static storage by 1
        keyBust.keyData(reinterpret_cast<const uint32_t*>(testStringA), kDataLenUsedForKey);
    }
    GrAssert(passCnt == 2); //We expect dynamic allocation to be necessary
    GrAssert(keyA.getHash() == keyBust.getHash());

    // Test that adding keyData in chunks gives
    // the same hash as with one chunk
    KeyType keyA2;
    while (keyA2.doPass()) {
        keyA2.keyData(reinterpret_cast<const uint32_t*>(testStringA), 4);
        keyA2.keyData(&reinterpret_cast<const uint32_t*>(testStringA)[4], kDataLenUsedForKey-4);
    }
    GrAssert(keyA.getHash() == keyA2.getHash());

    KeyType keyB;
    while (keyB.doPass()){
        keyB.keyData(reinterpret_cast<const uint32_t*>(testStringB), kDataLenUsedForKey);
    }
    GrAssert(keyA.compare(keyB) < 0);
    GrAssert(keyA.compare(keyA2) == 0);

    //Test ownership tranfer and copying
    keyB.copyAndTakeOwnership(keyA);
    GrAssert(keyA.fIsValid == false);
    GrAssert(keyB.fIsValid);
    GrAssert(keyB.getHash() == keyA2.getHash());
    GrAssert(keyB.compare(keyA2) == 0);
    keyA.deepCopyFrom(keyB);
    GrAssert(keyA.fIsValid);
    GrAssert(keyB.fIsValid);
    GrAssert(keyA.getHash() == keyA2.getHash());
    GrAssert(keyA.compare(keyA2) == 0);

    //Test ownership tranfer and copying with key on heap
    GrBinHashKey<BogusEntry, kDataLenUsedForKey-1> keyBust2;
    keyBust2.deepCopyFrom(keyBust);
    GrAssert(keyBust.fIsValid);
    GrAssert(keyBust2.fIsValid);
    GrAssert(keyBust.getHash() == keyBust2.getHash());
    GrAssert(keyBust.compare(keyBust2) == 0);
    GrBinHashKey<BogusEntry, kDataLenUsedForKey-1> keyBust3;
    keyBust3.deepCopyFrom(keyBust);
    GrAssert(keyBust.fIsValid == false);
    GrAssert(keyBust3.fIsValid);
    GrAssert(keyBust3.getHash() == keyBust2.getHash());
    GrAssert(keyBust3.compare(keyBust2) == 0);
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
