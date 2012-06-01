/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Simplify.h"

namespace SimplifyFindTopTest {

#include "Simplify.cpp"

} // end of SimplifyFindTopTest namespace

#include "Intersection_Tests.h"

static const SimplifyFindTopTest::Segment* testCommon(
        SkTArray<SimplifyFindTopTest::Contour>& contours,
        int& index, int& end) {
    SkTDArray<SimplifyFindTopTest::Contour*> contourList;
    makeContourList(contours, contourList);
    addIntersectTs(contourList[0], contourList[0]);
    if (contours.count() > 1) {
        SkASSERT(contours.count() == 2);
        addIntersectTs(contourList[0], contourList[1]);
        addIntersectTs(contourList[1], contourList[1]);
    }
    fixOtherTIndex(contourList);
    SimplifyFindTopTest::Segment* topStart = findTopContour(contourList,
            contourList.count());
    const SimplifyFindTopTest::Segment* topSegment = topStart->findTop(index,
            end);
    return topSegment;
}

static void test(const SkPath& path) {
    SkTArray<SimplifyFindTopTest::Contour> contours;
    SimplifyFindTopTest::EdgeBuilder builder(path, contours);
    int index, end;
    testCommon(contours, index, end);
    SkASSERT(index + 1 == end);
}

static void test(const SkPath& path, SkScalar x1, SkScalar y1,
        SkScalar x2, SkScalar y2) {
    SkTArray<SimplifyFindTopTest::Contour> contours;
    SimplifyFindTopTest::EdgeBuilder builder(path, contours);
    int index, end;
    const SimplifyFindTopTest::Segment* topSegment =
            testCommon(contours, index, end);
    SkPoint pts[2];
    double firstT = topSegment->t(index);
    pts[0] = topSegment->xyAtT(&topSegment->span(index));
    int direction = index < end ? 1 : -1;
    do {
        index += direction;
        double nextT = topSegment->t(index);
        if (nextT == firstT) {
            continue;
        }
        pts[1] = topSegment->xyAtT(&topSegment->span(index));
        if (pts[0] != pts[1]) {
            break;
        }
    } while (true);
    SkASSERT(pts[0].fX == x1);
    SkASSERT(pts[0].fY == y1);
    SkASSERT(pts[1].fX == x2);
    SkASSERT(pts[1].fY == y2);
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

static void addInnerCCWTriangle(SkPath& path) {
    path.moveTo(3,0);
    path.lineTo(2,1);
    path.lineTo(4,1);
    path.close();
}

static void addOuterCWTriangle(SkPath& path) {
    path.moveTo(3,0);
    path.lineTo(6,2);
    path.lineTo(0,2);
    path.close();
}

static void addOuterCCWTriangle(SkPath& path) {
    path.moveTo(3,0);
    path.lineTo(0,2);
    path.lineTo(6,2);
    path.close();
}

static void testLine2() {
    SkPath path;
    addInnerCWTriangle(path);
    addOuterCWTriangle(path);
    test(path, 0, 2, 3, 0);
}

static void testLine3() {
    SkPath path;
    addOuterCWTriangle(path);
    addInnerCWTriangle(path);
    test(path, 0, 2, 3, 0);
}

static void testLine4() {
    SkPath path;
    addInnerCCWTriangle(path);
    addOuterCWTriangle(path);
    test(path, 0, 2, 3, 0);
}

static void testLine5() {
    SkPath path;
    addOuterCWTriangle(path);
    addInnerCCWTriangle(path);
    test(path, 0, 2, 3, 0);
}

static void testLine6() {
    SkPath path;
    addInnerCWTriangle(path);
    addOuterCCWTriangle(path);
    test(path, 0, 2, 3, 0);
}

static void testLine7() {
    SkPath path;
    addOuterCCWTriangle(path);
    addInnerCWTriangle(path);
    test(path, 0, 2, 3, 0);
}

static void testLine8() {
    SkPath path;
    addInnerCCWTriangle(path);
    addOuterCCWTriangle(path);
    test(path, 0, 2, 3, 0);
}

static void testLine9() {
    SkPath path;
    addOuterCCWTriangle(path);
    addInnerCCWTriangle(path);
    test(path, 0, 2, 3, 0);
}

static void testQuads() {
    SkPath path;
    path.moveTo(2,0);
    path.quadTo(1,1, 0,0);
    path.close();
    test(path);
}

static void testCubics() {
    SkPath path;
    path.moveTo(2,0);
    path.cubicTo(2,3, 1,1, 0,0);
    path.close();
    test(path);
}

static void (*tests[])() = {
    testLine1,
    testLine2,
    testLine3,
    testLine4,
    testLine5,
    testLine6,
    testLine7,
    testLine8,
    testLine9,
    testQuads,
    testCubics
};

static const size_t testCount = sizeof(tests) / sizeof(tests[0]);

static void (*firstTest)() = 0;
static bool skipAll = false;

void SimplifyFindTop_Test() {
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
