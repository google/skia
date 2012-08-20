/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
 
#define DEBUG_TEST 1
 
#include "Simplify.h"

namespace SimplifyFindNextTest {

#include "Simplify.cpp"

} // end of SimplifyFindNextTest namespace

#include "Intersection_Tests.h"

static const SimplifyFindNextTest::Segment* testCommon(
        int contourWinding, int spanWinding, int startIndex, int endIndex,
        SkTArray<SimplifyFindNextTest::Contour>& contours) {
    SkTDArray<SimplifyFindNextTest::Contour*> contourList;
    makeContourList(contours, contourList);
    addIntersectTs(contourList[0], contourList[0]);
    if (contours.count() > 1) {
        SkASSERT(contours.count() == 2);
        addIntersectTs(contourList[0], contourList[1]);
        addIntersectTs(contourList[1], contourList[1]);
    }
    fixOtherTIndex(contourList);
    SimplifyFindNextTest::Segment& segment = contours[0].debugSegments()[0];
    SkPoint pts[2];
    pts[0] = segment.xyAtT(&segment.span(endIndex));
    int nextStart = startIndex;
    int nextEnd = endIndex;
    SkTDArray<SimplifyFindNextTest::Span*> chaseArray;
    SimplifyFindNextTest::Segment* next = segment.findNextWinding(chaseArray,
            true, nextStart, nextEnd, contourWinding, spanWinding);
    pts[1] = next->xyAtT(&next->span(nextStart));
    SkASSERT(pts[0] == pts[1]);
    return next;
}

static void test(const SkPath& path) {
    SkTArray<SimplifyFindNextTest::Contour> contours;
    SimplifyFindNextTest::EdgeBuilder builder(path, contours);
    int contourWinding = 0;
    int spanWinding = 1;
    int start = 0;
    int end = 1;
    testCommon(contourWinding, spanWinding, start, end, contours);
}

static void test(const SkPath& path, int start, int end) {
    SkTArray<SimplifyFindNextTest::Contour> contours;
    SimplifyFindNextTest::EdgeBuilder builder(path, contours);
    int contourWinding = 0;
    int spanWinding = 1;
    testCommon(contourWinding, spanWinding, start, end, contours);
}

static void testLine1() {
    SkPath path;
    path.moveTo(2,0);
    path.lineTo(1,1);
    path.lineTo(0,0);
    path.close();
    test(path);
}

static void addInnerCWTriangle(SkPath& path) {
    path.moveTo(3,0);
    path.lineTo(4,1);
    path.lineTo(2,1);
    path.close();
}

#if DEBUG_UNUSED
static void addInnerCCWTriangle(SkPath& path) {
    path.moveTo(3,0);
    path.lineTo(2,1);
    path.lineTo(4,1);
    path.close();
}
#endif

static void addOuterCWTriangle(SkPath& path) {
    path.moveTo(3,0);
    path.lineTo(6,2);
    path.lineTo(0,2);
    path.close();
}

#if DEBUG_UNUSED
static void addOuterCCWTriangle(SkPath& path) {
    path.moveTo(3,0);
    path.lineTo(0,2);
    path.lineTo(6,2);
    path.close();
}
#endif

static void testLine2() {
    SkPath path;
    addInnerCWTriangle(path);
    addOuterCWTriangle(path);
    test(path, 0, 3);
}

static void testLine3() {
    SkPath path;
    addInnerCWTriangle(path);
    addOuterCWTriangle(path);
    test(path, 3, 0);
}

static void testLine4() {
    SkPath path;
    addInnerCWTriangle(path);
    addOuterCWTriangle(path);
    test(path, 3, 2);
}

static void (*tests[])() = {
    testLine1,
    testLine2,
    testLine3,
    testLine4,
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
