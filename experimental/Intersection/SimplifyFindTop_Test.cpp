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
        SimplifyFindTopTest::EdgeBuilder& builder, const SkPath& path) {
    SkTDArray<SimplifyFindTopTest::Contour*> contourList;
    SimplifyFindTopTest::Contour sentinel;
    sentinel.reset();
    makeContourList(contours, sentinel, contourList);
    addIntersectTs(contourList[0], contourList[0], -1);
    if (contours.count() > 1) {
        SkASSERT(contours.count() == 2);
        addIntersectTs(contourList[0], contourList[1], -1);
        addIntersectTs(contourList[1], contourList[1], -1);
    }
    fixOtherTIndex(contourList);
    SimplifyFindTopTest::Segment* topStart = findTopContour(contourList,
            contourList.count());
    int index, direction;
    const SimplifyFindTopTest::Segment* topSegment = topStart->findTop(index,
            direction);
    SkASSERT(direction == 1);
    return topSegment;
}

static void test(const SkPath& path) {
    SkTArray<SimplifyFindTopTest::Contour> contours;
    SimplifyFindTopTest::EdgeBuilder builder(path, contours);
    testCommon(contours, builder, path);
}

static void test(const SkPath& path, SkScalar x1, SkScalar y1,
        SkScalar x2, SkScalar y2) {
    SkTArray<SimplifyFindTopTest::Contour> contours;
    SimplifyFindTopTest::EdgeBuilder builder(path, contours);
    const SimplifyFindTopTest::Segment* topSegment =
            testCommon(contours, builder, path);
    const SkPoint* pts = topSegment->pts();
    SkPoint top = pts[0];
    SkPoint bottom = pts[1];
    if (top.fY > bottom.fY) {
        SkTSwap<SkPoint>(top, bottom);
    }
    SkASSERT(top.fX == x1);
    SkASSERT(top.fY == y1);
    SkASSERT(bottom.fX == x2);
    SkASSERT(bottom.fY == y2);
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
    test(path, 3, 0, 0, 2);
}

static void testLine3() {
    SkPath path;
    addOuterCWTriangle(path);
    addInnerCWTriangle(path);
    test(path, 3, 0, 0, 2);
}

static void testLine4() {
    SkPath path;
    addInnerCCWTriangle(path);
    addOuterCWTriangle(path);
    test(path, 3, 0, 0, 2);
}

static void testLine5() {
    SkPath path;
    addOuterCWTriangle(path);
    addInnerCCWTriangle(path);
    test(path, 3, 0, 0, 2);
}

static void testLine6() {
    SkPath path;
    addInnerCWTriangle(path);
    addOuterCCWTriangle(path);
    test(path, 3, 0, 0, 2);
}

static void testLine7() {
    SkPath path;
    addOuterCCWTriangle(path);
    addInnerCWTriangle(path);
    test(path, 3, 0, 0, 2);
}

static void testLine8() {
    SkPath path;
    addInnerCCWTriangle(path);
    addOuterCCWTriangle(path);
    test(path, 3, 0, 0, 2);
}

static void testLine9() {
    SkPath path;
    addOuterCCWTriangle(path);
    addInnerCCWTriangle(path);
    test(path, 3, 0, 0, 2);
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
