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


#include "GrClip.h"
#include "GrTDArray.h"
#include "GrTBSearch.h"
#include "GrMatrix.h"
#include "GrRedBlackTree.h"

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

static void dump(const GrClip& clip, const char message[]) {
    GrPrintf("--- dump clip %s\n", message);
    GrClipIter iter(clip);
    while (!iter.isDone()) {
        GrIRect r;
        iter.getRect(&r);
        GrPrintf("--- [%d %d %d %d]\n", r.fLeft, r.fTop, r.fRight, r.fBottom);
        iter.next();
    }
}

static void test_clip() {
    GrClip  clip;
    GrAssert(clip.isEmpty());
    GrAssert(!clip.isRect());
    GrAssert(!clip.isComplex());
    GrAssert(clip.getBounds().equalsLTRB(0, 0, 0, 0));
    GrAssert(0 == clip.countRects());

    clip.setRect(GrIRect(10, 10, 10, 10));
    GrAssert(clip.isEmpty());
    GrAssert(!clip.isRect());
    GrAssert(!clip.isComplex());
    GrAssert(clip.getBounds().equalsLTRB(0, 0, 0, 0));
    GrAssert(0 == clip.countRects());
    dump(clip, "empty");

    clip.setRect(GrIRect(10, 10, 20, 20));
    GrAssert(!clip.isEmpty());
    GrAssert(clip.isRect());
    GrAssert(!clip.isComplex());
    GrAssert(clip.getBounds().equalsLTRB(10, 10, 20, 20));
    GrAssert(1 == clip.countRects());
    GrAssert(clip.getRects()[0] == clip.getBounds());
    dump(clip, "rect");

    clip.addRect(GrIRect(20, 20, 25, 25));
    GrAssert(!clip.isEmpty());
    GrAssert(!clip.isRect());
    GrAssert(clip.isComplex());
    GrAssert(clip.getBounds().equalsLTRB(10, 10, 25, 25));
    GrAssert(2 == clip.countRects());
    dump(clip, "complex");

    GrClip c1(clip);
    GrAssert(c1 == clip);
    GrClip c2;
    GrAssert(c2 != c1);
    c2 = clip;
    GrAssert(c2 == clip);

    clip.setEmpty();
    GrAssert(clip.isEmpty());
    GrAssert(!clip.isRect());
    GrAssert(!clip.isComplex());
    GrAssert(clip.getBounds().equalsLTRB(0, 0, 0, 0));

    GrAssert(c1 != clip);
    GrAssert(c2 != clip);
}

void gr_run_unittests() {
    test_tdarray();
    test_bsearch();
    test_clip();
    GrMatrix::UnitTest();
    GrRedBlackTree<int>::UnitTest();
}


