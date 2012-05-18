/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
 
#include "Simplify.h"

namespace SimplifyFindNextTest {

#include "Simplify.cpp"

} // end of SimplifyFindNextTest namespace

#include "Intersection_Tests.h"

static const SimplifyFindNextTest::Segment* testCommon(
        int start, int winding, int step,
        SkTArray<SimplifyFindNextTest::Contour>& contours,
        SimplifyFindNextTest::EdgeBuilder& builder, const SkPath& path) {
    SkTDArray<SimplifyFindNextTest::Contour*> contourList;
    SimplifyFindNextTest::Contour sentinel;
    sentinel.reset();
    makeContourList(contours, sentinel, contourList);
    addIntersectTs(contourList[0], contourList[0], -1);
    if (contours.count() > 1) {
        SkASSERT(contours.count() == 2);
        addIntersectTs(contourList[0], contourList[1], -1);
        addIntersectTs(contourList[1], contourList[1], -1);
    }
    fixOtherTIndex(contourList);
    SimplifyFindNextTest::Segment& segment = contours[0].fSegments[0];
    int spanIndex;
    SimplifyFindNextTest::Segment* next = segment.findNext(start, winding,
            step, spanIndex);
    SkASSERT(spanIndex == 1);
    return next;
}

static void test(const SkPath& path) {
    SkTArray<SimplifyFindNextTest::Contour> contours;
    SimplifyFindNextTest::EdgeBuilder builder(path, contours);
    int start = 0;
    int winding = 0;
    int step = 1;
    testCommon(start, winding, step, contours, builder, path);
}

static void testLine1() {
    SkPath path;
    path.moveTo(2,0);
    path.lineTo(1,1);
    path.lineTo(0,0);
    path.close();
    test(path);
}

static void (*tests[])() = {
    testLine1,
};

static const size_t testCount = sizeof(tests) / sizeof(tests[0]);

static void (*firstTest)() = 0;
static bool skipAll = false;

void SimplifyFindNext_Test() {
    if (skipAll) {
        return;
    }
    size_t index = 0;
    if (firstTest) {
        while (index < testCount && tests[index] != firstTest) {
            ++index;
        }
    }
    bool firstTestComplete = false;
    for ( ; index < testCount; ++index) {
        (*tests[index])();
        firstTestComplete = true;
    }
}
