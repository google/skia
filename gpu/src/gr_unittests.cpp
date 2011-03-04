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


#include "GrDrawTarget.h"
#include "GrTDArray.h"
#include "GrTBSearch.h"
#include "GrMatrix.h"
#include "GrRedBlackTree.h"
#include "GrPath.h"

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

static bool LT(const int& elem, int value) {
    return elem < value;
}
static bool EQ(const int& elem, int value) {
    return elem == value;
}

static void test_bsearch() {
    const int array[] = {
        1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 22, 33, 44, 55, 66, 77, 88, 99
    };

    for (size_t n = 0; n < GR_ARRAY_COUNT(array); n++) {
        for (size_t i = 0; i < n; i++) {
            int index = GrTBSearch<int, int>(array, n, array[i]);
            GrAssert(index == i);
            index = GrTBSearch<int, int>(array, n, -array[i]);
            GrAssert(index < 0);
        }
    }
}

void gr_run_unittests() {
    test_tdarray();
    test_bsearch();
    GrMatrix::UnitTest();
    GrRedBlackTree<int>::UnitTest();
    GrPath::ConvexUnitTest();
    GrDrawTarget::VertexLayoutUnitTest();
}


