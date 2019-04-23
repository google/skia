/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "tests/PathOpsDebug.h"
#include "tests/PathOpsExtendedTest.h"
#include "tests/PathOpsTestCommon.h"

class PathTest_Private {
public:
    PathTest_Private(SkPath* path)
        : fPath(path) {}

    void setPt(int index, SkScalar x, SkScalar y) {
        fPath->setPt(index, x, y);
    }

    SkPath* fPath;
};

static void path_edit(const SkPoint& from, const SkPoint& to, SkPath* path) {
    PathTest_Private testPath(path);
    for (int index = 0; index < path->countPoints(); ++index) {
        if (SkDPoint::ApproximatelyEqual(path->getPoint(index), from)) {
            testPath.setPt(index, to.fX, to.fY);
            return;
        }
    }
}

static void cubicOp1d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(0,2, 1,0, 1,0);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,1);
    pathB.cubicTo(0,1, 1,0, 2,0);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp2d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,2);
    path.cubicTo(0,1, 1,0, 1,0);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,1);
    pathB.cubicTo(0,1, 2,0, 1,0);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp3d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(2,3, 1,0, 1,0);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,1);
    pathB.cubicTo(0,1, 1,0, 3,2);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp5d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(0,2, 1,0, 2,0);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,1);
    pathB.cubicTo(0,2, 1,0, 2,0);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp6d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(0,6, 1,0, 3,0);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,1);
    pathB.cubicTo(0,3, 1,0, 6,0);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp7d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(3,4, 1,0, 3,0);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,1);
    pathB.cubicTo(0,3, 1,0, 4,3);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp8d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(0,5, 1,0, 4,0);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,1);
    pathB.cubicTo(0,4, 1,0, 5,0);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp9d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(1,6, 1,0, 2,1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,1);
    pathB.cubicTo(1,2, 1,0, 6,1);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void quadOp9d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.quadTo(1,6, 1.5f,1);
    path.quadTo(1.5f,0.5f, 2,1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,1);
    pathB.quadTo(1,2, 1.4f,1);
    pathB.quadTo(3,0.4f, 6,1);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void lineOp9d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.lineTo(1,6);
    path.lineTo(1.5f,1);
    path.lineTo(1.8f,0.8f);
    path.lineTo(2,1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,1);
    pathB.lineTo(1,2);
    pathB.lineTo(1.4f,1);
    pathB.lineTo(3,0.4f);
    pathB.lineTo(6,1);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp1i(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(1,2, 1,0, 2,1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,1);
    pathB.cubicTo(1,2, 1,0, 2,1);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void cubicOp10d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(1,3, 1,0, 4,1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,1);
    pathB.cubicTo(1,4, 1,0, 3,1);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp11d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(3,4, 1,0, 5,1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,1);
    pathB.cubicTo(1,5, 1,0, 4,3);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp12d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(1,6, 1,0, 1,0);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,1);
    pathB.cubicTo(0,1, 1,0, 6,1);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp13d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(4,5, 1,0, 5,3);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,1);
    pathB.cubicTo(3,5, 1,0, 5,4);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp14d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(0,2, 2,0, 2,1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,2);
    pathB.cubicTo(1,2, 1,0, 2,0);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp15d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(3,6, 2,0, 2,1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,2);
    pathB.cubicTo(1,2, 1,0, 6,3);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp16d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,2);
    path.cubicTo(0,1, 3,0, 1,0);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,3);
    pathB.cubicTo(0,1, 2,0, 1,0);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp17d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,2);
    path.cubicTo(0,2, 4,0, 2,1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,4);
    pathB.cubicTo(1,2, 2,0, 2,0);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp18d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(3,5, 2,0, 2,1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,2);
    pathB.cubicTo(1,2, 1,0, 5,3);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp19i(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,2);
    path.cubicTo(0,1, 2,1, 6,2);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1,2);
    pathB.cubicTo(2,6, 2,0, 1,0);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void cubicOp20d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(0,1, 6,0, 2,1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,6);
    pathB.cubicTo(1,2, 1,0, 1,0);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp21d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(0,1, 2,1, 6,5);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1,2);
    pathB.cubicTo(5,6, 1,0, 1,0);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp22d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(2,3, 3,0, 2,1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,3);
    pathB.cubicTo(1,2, 1,0, 3,2);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp23d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(1,2, 4,0, 2,1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,4);
    pathB.cubicTo(1,2, 1,0, 2,1);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp24d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(1,2, 2,0, 3,2);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,2);
    pathB.cubicTo(2,3, 1,0, 2,1);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void testIntersect1(skiatest::Reporter* reporter, const char* filename) {
    SkPath one, two;
    one.addRect(0, 0, 6, 6, SkPath::kCW_Direction);
    two.addRect(3, 3, 9, 9, SkPath::kCW_Direction);
    testPathOp(reporter, one, two, kIntersect_SkPathOp, filename);
}

static void testUnion1(skiatest::Reporter* reporter, const char* filename) {
    SkPath one, two;
    one.addRect(0, 0, 6, 6, SkPath::kCW_Direction);
    two.addRect(3, 3, 9, 9, SkPath::kCW_Direction);
    testPathOp(reporter, one, two, kUnion_SkPathOp, filename);
}

static void testDiff1(skiatest::Reporter* reporter, const char* filename) {
    SkPath one, two;
    one.addRect(0, 0, 6, 6, SkPath::kCW_Direction);
    two.addRect(3, 3, 9, 9, SkPath::kCW_Direction);
    testPathOp(reporter, one, two, kDifference_SkPathOp, filename);
}

static void testXor1(skiatest::Reporter* reporter, const char* filename) {
    SkPath one, two;
    one.addRect(0, 0, 6, 6, SkPath::kCW_Direction);
    two.addRect(3, 3, 9, 9, SkPath::kCW_Direction);
    testPathOp(reporter, one, two, kXOR_SkPathOp, filename);
}

static void testIntersect2(skiatest::Reporter* reporter, const char* filename) {
    SkPath one, two;
    one.addRect(0, 0, 6, 6, SkPath::kCW_Direction);
    two.addRect(0, 3, 9, 9, SkPath::kCW_Direction);
    testPathOp(reporter, one, two, kIntersect_SkPathOp, filename);
}

static void testUnion2(skiatest::Reporter* reporter, const char* filename) {
    SkPath one, two;
    one.addRect(0, 0, 6, 6, SkPath::kCW_Direction);
    two.addRect(0, 3, 9, 9, SkPath::kCW_Direction);
    testPathOp(reporter, one, two, kUnion_SkPathOp, filename);
}

static void testDiff2(skiatest::Reporter* reporter, const char* filename) {
    SkPath one, two;
    one.addRect(0, 0, 6, 6, SkPath::kCW_Direction);
    two.addRect(0, 3, 9, 9, SkPath::kCW_Direction);
    testPathOp(reporter, one, two, kDifference_SkPathOp, filename);
}

static void testXor2(skiatest::Reporter* reporter, const char* filename) {
    SkPath one, two;
    one.addRect(0, 0, 6, 6, SkPath::kCW_Direction);
    two.addRect(0, 3, 9, 9, SkPath::kCW_Direction);
    testPathOp(reporter, one, two, kXOR_SkPathOp, filename);
}

static void testOp1d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    path.addRect(0, 0, 2, 2, SkPath::kCW_Direction);
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    pathB.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void testOp2d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    path.addRect(0, 0, 2, 2, SkPath::kCW_Direction);
    pathB.setFillType(SkPath::kEvenOdd_FillType);
    pathB.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    pathB.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void testOp3d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    path.addRect(1, 1, 2, 2, SkPath::kCW_Direction);
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    pathB.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void testOp1u(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    path.addRect(0, 0, 3, 3, SkPath::kCW_Direction);
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    pathB.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    testPathOp(reporter, path, pathB, kUnion_SkPathOp, filename);
}

static void testOp4d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    path.addRect(2, 2, 4, 4, SkPath::kCW_Direction);
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    pathB.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void testOp5d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 2, 2, SkPath::kCW_Direction);
    path.addRect(0, 0, 3, 3, SkPath::kCW_Direction);
    pathB.setFillType(SkPath::kEvenOdd_FillType);
    pathB.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    pathB.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void testOp6d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    path.addRect(0, 0, 3, 3, SkPath::kCW_Direction);
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    pathB.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void testOp7d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 2, 2, SkPath::kCW_Direction);
    path.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    pathB.setFillType(SkPath::kEvenOdd_FillType);
    pathB.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    pathB.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void testOp2u(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 2, 2, SkPath::kCW_Direction);
    path.addRect(0, 0, 2, 2, SkPath::kCW_Direction);
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.addRect(0, 0, 3, 3, SkPath::kCW_Direction);
    pathB.addRect(1, 1, 2, 2, SkPath::kCW_Direction);
    testPathOp(reporter, path, pathB, kUnion_SkPathOp, filename);
}

static void testOp8d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.addRect(0, 0, 640, 480);
    pathB.moveTo(577330, 1971.72f);
    pathB.cubicTo(10.7082f, -116.596f, 262.057f, 45.6468f, 294.694f, 1.96237f);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}
static void cubicOp25i(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(2,4, 5,0, 3,2);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,5);
    pathB.cubicTo(2,3, 1,0, 4,2);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void cubicOp26d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(3,4, 4,0, 3,2);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,4);
    pathB.cubicTo(2,3, 1,0, 4,3);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp27d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(3,6, 1,0, 5,2);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,1);
    pathB.cubicTo(2,5, 1,0, 6,3);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp28u(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(1,4, 6,0, 3,2);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,6);
    pathB.cubicTo(2,3, 1,0, 4,1);
    pathB.close();
    testPathOp(reporter, path, pathB, kUnion_SkPathOp, filename);
}

static void cubicOp29d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(2,5, 6,0, 4,2);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,6);
    pathB.cubicTo(2,4, 1,0, 5,2);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp30d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(2,5, 6,0, 5,3);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,6);
    pathB.cubicTo(3,5, 1,0, 5,2);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp31d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,2);
    path.cubicTo(0,3, 2,1, 4,0);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1,2);
    pathB.cubicTo(0,4, 2,0, 3,0);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp31u(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,2);
    path.cubicTo(0,3, 2,1, 4,0);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1,2);
    pathB.cubicTo(0,4, 2,0, 3,0);
    pathB.close();
    testPathOp(reporter, path, pathB, kUnion_SkPathOp, filename);
}

static void cubicOp31x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,2);
    path.cubicTo(0,3, 2,1, 4,0);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1,2);
    pathB.cubicTo(0,4, 2,0, 3,0);
    pathB.close();
    testPathOp(reporter, path, pathB, kXOR_SkPathOp, filename);
}

static void cubicOp32d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(1,2, 6,0, 3,1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,6);
    pathB.cubicTo(1,3, 1,0, 2,1);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp33i(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(1,2, 6,0, 3,1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,6);
    pathB.cubicTo(1,3, 1,0, 2,1);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void cubicOp34d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(3,5, 2,1, 3,1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1,2);
    pathB.cubicTo(1,3, 1,0, 5,3);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp35d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(1,5, 2,1, 4,0);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1,2);
    pathB.cubicTo(0,4, 1,0, 5,1);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp36u(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(1,6, 2,0, 5,1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,2);
    pathB.cubicTo(1,5, 1,0, 6,1);
    pathB.close();
    testPathOp(reporter, path, pathB, kUnion_SkPathOp, filename);
}

static void cubicOp37d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(2,6, 6,1, 4,3);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1,6);
    pathB.cubicTo(3,4, 1,0, 6,2);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp38d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(0,6, 3,2, 4,1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(2,3);
    pathB.cubicTo(1,4, 1,0, 6,0);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp39d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(2,3, 5,1, 4,3);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1,5);
    pathB.cubicTo(3,4, 1,0, 3,2);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp40d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(1,5, 3,2, 4,2);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(2,3);
    pathB.cubicTo(2,4, 1,0, 5,1);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp41i(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(2,6, 4,3, 6,4);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(3,4);
    pathB.cubicTo(4,6, 1,0, 6,2);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void cubicOp42d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(1,2, 6,5, 5,4);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(5,6);
    pathB.cubicTo(4,5, 1,0, 2,1);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp43d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,2);
    path.cubicTo(1,2, 4,0, 3,1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,4);
    pathB.cubicTo(1,3, 2,0, 2,1);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp44d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,2);
    path.cubicTo(3,6, 4,0, 3,2);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,4);
    pathB.cubicTo(2,3, 2,0, 6,3);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp45d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,2);
    path.cubicTo(2,4, 4,0, 3,2);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,4);
    pathB.cubicTo(2,3, 2,0, 4,2);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp46d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,2);
    path.cubicTo(3,5, 5,0, 4,2);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,5);
    pathB.cubicTo(2,4, 2,0, 5,3);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp47d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(1,6, 6,2, 5,4);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(2,6);
    pathB.cubicTo(4,5, 1,0, 6,1);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp48d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,2);
    path.cubicTo(2,3, 5,1, 3,2);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1,5);
    pathB.cubicTo(2,3, 2,0, 3,2);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp49d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,2);
    path.cubicTo(1,5, 3,2, 4,1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(2,3);
    pathB.cubicTo(1,4, 2,0, 5,1);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp50d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,3);
    path.cubicTo(1,6, 5,0, 5,1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,5);
    pathB.cubicTo(1,5, 3,0, 6,1);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp51d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,3);
    path.cubicTo(1,2, 4,1, 6,0);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1,4);
    pathB.cubicTo(0,6, 3,0, 2,1);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp52d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,2);
    path.cubicTo(1,2, 5,4, 4,3);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(4,5);
    pathB.cubicTo(3,4, 2,0, 2,1);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp53d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,3);
    path.cubicTo(1,2, 5,3, 2,1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(3,5);
    pathB.cubicTo(1,2, 3,0, 2,1);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp54d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,4);
    path.cubicTo(1,3, 5,4, 4,2);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(4,5);
    pathB.cubicTo(2,4, 4,0, 3,1);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp55d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,5);
    path.cubicTo(1,3, 3,2, 5,0);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(2,3);
    pathB.cubicTo(0,5, 5,0, 3,1);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp56d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(2,6, 5,0, 2,1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,5);
    pathB.cubicTo(1,2, 1,0, 6,2);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp57d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,5);
    path.cubicTo(0,5, 5,4, 6,4);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(4,5);
    pathB.cubicTo(4,6, 5,0, 5,0);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp58d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,5);
    path.cubicTo(3,4, 6,5, 5,3);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(5,6);
    pathB.cubicTo(3,5, 5,0, 4,3);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp59d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(5,6, 4,0, 4,1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,4);
    pathB.cubicTo(1,4, 1,0, 6,5);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp60d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,2);
    path.cubicTo(4,6, 6,0, 5,2);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,6);
    pathB.cubicTo(2,5, 2,0, 6,4);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp61d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(1,2);
    path.cubicTo(0,5, 3,2, 6,1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(2,3);
    pathB.cubicTo(1,6, 2,1, 5,0);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp62d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(1,3);
    path.cubicTo(5,6, 5,3, 5,4);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(3,5);
    pathB.cubicTo(4,5, 3,1, 6,5);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp63d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(2,3);
    path.cubicTo(0,4, 3,2, 5,3);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(2,3);
    pathB.cubicTo(3,5, 3,2, 4,0);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp64d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.moveTo(0,1);
    path.cubicTo(0,1, 1,0, 3,0);
    path.lineTo(0,1);
    path.close();
    pathB.moveTo(0,1);
    pathB.cubicTo(0,3, 1,0, 1,0);
    pathB.lineTo(0,1);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp65d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.moveTo(0,1);
    path.cubicTo(1,5, 1,0, 1,0);
    path.lineTo(0,1);
    path.close();
    pathB.moveTo(0,1);
    pathB.cubicTo(0,1, 1,0, 5,1);
    pathB.lineTo(0,1);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void rectOp1d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.moveTo(0,1);
    path.cubicTo(0,1, 1,0, 3,0);
    path.lineTo(0,1);
    path.close();
    pathB.moveTo(0,1);
    pathB.cubicTo(0,3, 1,0, 1,0);
    pathB.lineTo(0,1);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp66u(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(2,6, 4,2, 5,3);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(2,4);
    pathB.cubicTo(3,5, 1,0, 6,2);
    pathB.close();
    testPathOp(reporter, path, pathB, kUnion_SkPathOp, filename);
}

static void cubicOp67u(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.moveTo(3,5);
    path.cubicTo(1,6, 5,0, 3,1);
    path.lineTo(3,5);
    path.close();
    pathB.moveTo(0,5);
    pathB.cubicTo(1,3, 5,3, 6,1);
    pathB.lineTo(0,5);
    pathB.close();
    testPathOp(reporter, path, pathB, kUnion_SkPathOp, filename);
}

static void cubicOp68u(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.moveTo(0,5);
    path.cubicTo(4,5, 4,1, 5,0);
    path.close();
    pathB.moveTo(1,4);
    pathB.cubicTo(0,5, 5,0, 5,4);
    pathB.close();
    testPathOp(reporter, path, pathB, kUnion_SkPathOp, filename);
}

static void cubicOp69d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.moveTo(1,3);
    path.cubicTo(0,1, 3,1, 2,0);
    path.close();
    pathB.moveTo(1,3);
    pathB.cubicTo(0,2, 3,1, 1,0);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

SkPathOp ops[] = {
    kUnion_SkPathOp,
    kXOR_SkPathOp,
    kReverseDifference_SkPathOp,
    kXOR_SkPathOp,
    kReverseDifference_SkPathOp,
};

static void rRect1(skiatest::Reporter* reporter, const char* filename) {
    SkScalar xA = 0.65f;
    SkScalar xB = 10.65f;
    SkScalar xC = 20.65f;
    SkScalar xD = 30.65f;
    SkScalar xE = 40.65f;
    SkScalar xF = 50.65f;

    SkScalar yA = 0.65f;
    SkScalar yB = 10.65f;
    SkScalar yC = 20.65f;
    SkScalar yD = 30.65f;
    SkScalar yE = 40.65f;
    SkScalar yF = 50.65f;
    SkPath paths[5];
    SkRect rects[5];
    rects[0].set(xB, yB, xE, yE);
    paths[0].addRoundRect(rects[0], SkIntToScalar(5), SkIntToScalar(5));  // red
    rects[1].set(xA, yA, xD, yD);
    paths[1].addRoundRect(rects[1], SkIntToScalar(5), SkIntToScalar(5));  // green
    rects[2].set(xC, yA, xF, yD);
    paths[2].addRoundRect(rects[2], SkIntToScalar(5), SkIntToScalar(5));  // blue
    rects[3].set(xA, yC, xD, yF);
    paths[3].addRoundRect(rects[3], SkIntToScalar(5), SkIntToScalar(5));  // yellow
    rects[4].set(xC, yC, xF, yF);
    paths[4].addRoundRect(rects[4], SkIntToScalar(5), SkIntToScalar(5));  // cyan
    SkPath path;
    path.setFillType(SkPath::kInverseEvenOdd_FillType);
    for (int index = 0; index < 5; ++index) {
        SkString uniqueName;
        uniqueName.printf("%s%d", filename, index);
        testPathOp(reporter, path, paths[index], ops[index], uniqueName.c_str());
        REPORTER_ASSERT(reporter, Op(path, paths[index], ops[index], &path));
    }
}

static void skp1(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(189,7);
    path.cubicTo(189,5.34314585f, 190.34314f,4, 192,4);
    path.lineTo(243,4);
    path.cubicTo(244.65686f,4, 246,5.34314585f, 246,7);
    path.lineTo(246,21);
    path.cubicTo(246,22.6568546f, 244.65686f,24, 243,24);
    path.lineTo(192,24);
    path.cubicTo(190.34314f,24, 189,22.6568546f, 189,21);
    path.lineTo(189,7);
    path.close();
    path.moveTo(191,8);
    path.cubicTo(191,6.89543009f, 191.895432f,6, 193,6);
    path.lineTo(242,6);
    path.cubicTo(243.104568f,6, 244,6.89543009f, 244,8);
    path.lineTo(244,20);
    path.cubicTo(244,21.1045704f, 243.104568f,22, 242,22);
    path.lineTo(193,22);
    path.cubicTo(191.895432f,22, 191,21.1045704f, 191,20);
    path.lineTo(191,8);
    path.close();
    SkPath pathB;
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(189,4);
    pathB.lineTo(199,14);
    pathB.lineTo(236,14);
    pathB.lineTo(246,4);
    pathB.lineTo(189,4);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void skp2(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(253.000000f, 11757.0000f);
    path.lineTo(253.000000f, 222.000000f);
    path.lineTo(823.000000f, 222.000000f);
    path.lineTo(823.000000f, 11757.0000f);
    path.lineTo(253.000000f, 11757.0000f);
    path.close();
    SkPath pathB;
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(258.000000f, 1028.00000f);
    pathB.lineTo(258.000000f, 1027.00000f);
    pathB.lineTo(823.000000f, 1027.00000f);
    pathB.lineTo(823.000000f, 1028.00000f);
    pathB.lineTo(258.000000f, 1028.00000f);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void skp3(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(717.000000f, 507.000000f);
    path.lineTo(717.000000f, 425.000000f);
    path.lineTo(973.000000f, 425.000000f);
    path.lineTo(973.000000f, 507.000000f);
    path.quadTo(973.000000f, 508.242645f, 972.121582f, 509.121613f);
    path.quadTo(971.242615f, 510.000000f, 970.000000f, 510.000000f);
    path.lineTo(720.000000f, 510.000000f);
    path.quadTo(718.757385f, 510.000000f, 717.878418f, 509.121613f);
    path.quadTo(717.000000f, 508.242645f, 717.000000f, 507.000000f);
    path.close();
    path.moveTo(719.000000f, 426.000000f);
    path.lineTo(971.000000f, 426.000000f);
    path.lineTo(971.000000f, 506.000000f);
    path.cubicTo(971.000000f, 507.104584f, 970.104553f, 508.000000f, 969.000000f, 508.000000f);
    path.lineTo(721.000000f, 508.000000f);
    path.cubicTo(719.895447f, 508.000000f, 719.000000f, 507.104584f, 719.000000f, 506.000000f);
    path.lineTo(719.000000f, 426.000000f);
    path.close();
    SkPath pathB;
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(717.000000f, 510.000000f);
    pathB.lineTo(760.000000f, 467.000000f);
    pathB.lineTo(930.000000f, 467.000000f);
    pathB.lineTo(973.000000f, 510.000000f);
    pathB.lineTo(717.000000f, 510.000000f);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void skp4(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(230.756805f, 591.756775f);
    path.quadTo(232.514725f, 590.000000f, 235.000000f, 590.000000f);
    path.lineTo(300.000000f, 590.000000f);
    path.quadTo(302.485291f, 590.000000f, 304.243195f, 591.756775f);
    path.quadTo(306.000000f, 593.514709f, 306.000000f, 596.000000f);
    path.lineTo(306.000000f, 617.000000f);
    path.lineTo(229.000000f, 617.000000f);
    path.lineTo(229.000000f, 596.000000f);
    path.quadTo(229.000000f, 593.514709f, 230.756805f, 591.756775f);
    path.close();
    path.moveTo(231.000000f, 597.000000f);
    path.cubicTo(231.000000f, 594.238586f, 233.238571f, 592.000000f, 236.000000f, 592.000000f);
    path.lineTo(299.000000f, 592.000000f);
    path.cubicTo(301.761414f, 592.000000f, 304.000000f, 594.238586f, 304.000000f, 597.000000f);
    path.lineTo(304.000000f, 616.000000f);
    path.lineTo(231.000000f, 616.000000f);
    path.lineTo(231.000000f, 597.000000f);
    path.close();
    SkPath pathB;
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(306.000000f, 590.000000f);
    pathB.lineTo(292.000000f, 604.000000f);
    pathB.lineTo(305.000000f, 617.000000f);
    pathB.lineTo(306.000000f, 617.000000f);
    pathB.lineTo(306.000000f, 590.000000f);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void skp5(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(18.0000000f, 226.000000f);
    path.quadTo(14.6862917f, 226.000000f, 12.3423996f, 228.342407f);
    path.quadTo(10.0000000f, 230.686295f, 10.0000000f, 234.000000f);
    path.lineTo(10.0000000f, 253.000000f);
    path.lineTo(1247.00000f, 253.000000f);
    path.lineTo(1247.00000f, 234.000000f);
    path.quadTo(1247.00000f, 230.686295f, 1244.65759f, 228.342407f);
    path.quadTo(1242.31372f, 226.000000f, 1239.00000f, 226.000000f);
    path.lineTo(18.0000000f, 226.000000f);
    path.close();
    SkPath pathB;
    pathB.setFillType(SkPath::kInverseWinding_FillType);
    pathB.moveTo(18.0000000f, 226.000000f);
    pathB.lineTo(1239.00000f, 226.000000f);
    pathB.cubicTo(1243.41833f, 226.000000f, 1247.00000f, 229.581726f, 1247.00000f, 234.000000f);
    pathB.lineTo(1247.00000f, 252.000000f);
    pathB.lineTo(10.0000000f, 252.000000f);
    pathB.lineTo(10.0000000f, 234.000000f);
    pathB.cubicTo(10.0000000f, 229.581726f, 13.5817204f, 226.000000f, 18.0000000f, 226.000000f);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void cubicOp70d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(0,5, 4,0, 5,0);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,4);
    pathB.cubicTo(0,5, 1,0, 5,0);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp71d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(0,5, 4,1, 6,4);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1,4);
    pathB.cubicTo(4,6, 1,0, 5,0);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp72i(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(0,5, 5,2, 5,4);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(2,5);
    pathB.cubicTo(4,5, 1,0, 5,0);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void cubicOp73d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(3,4, 4,0, 6,4);
    path.lineTo(0,1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,4);
    pathB.cubicTo(4,6, 1,0, 4,3);
    pathB.lineTo(0,4);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp74d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(1,5, 5,1, 5,1);
    path.lineTo(0,1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1,5);
    pathB.cubicTo(1,5, 1,0, 5,1);
    pathB.lineTo(1,5);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp75d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(0,4, 5,1, 6,4);
    path.lineTo(0,1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1,5);
    pathB.cubicTo(4,6, 1,0, 4,0);
    pathB.lineTo(1,5);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp76u(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(0,2, 2,0, 5,3);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,2);
    pathB.cubicTo(3,5, 1,0, 2,0);
    pathB.close();
    testPathOp(reporter, path, pathB, kUnion_SkPathOp, filename);
}

static void cubicOp77i(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(0,1);
    path.cubicTo(1,3, 2,0, 3,2);
    path.lineTo(0,1);
    path.close();
    pathB.setFillType(SkPath::kEvenOdd_FillType);
    pathB.moveTo(0,2);
    pathB.cubicTo(2,3, 1,0, 3,1);
    pathB.lineTo(0,2);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void cubicOp78u(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(1,6);
    path.cubicTo(1,6, 5,0, 6,1);
    path.lineTo(1,6);
    path.close();
    pathB.setFillType(SkPath::kEvenOdd_FillType);
    pathB.moveTo(0,5);
    pathB.cubicTo(1,6, 6,1, 6,1);
    pathB.lineTo(0,5);
    pathB.close();
    testPathOp(reporter, path, pathB, kUnion_SkPathOp, filename);
}

static void cubicOp79u(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(1,3, 1,0, 6,4);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,1);
    pathB.cubicTo(4,6, 1,0, 3,1);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void cubicOp80i(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(2,3, 2,1, 4,3);
    path.lineTo(0,1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1,2);
    pathB.cubicTo(3,4, 1,0, 3,2);
    pathB.lineTo(1,2);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void cubicOp81d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(4,6, 4,3, 5,4);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(3,4);
    pathB.cubicTo(4,5, 1,0, 6,4);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp82i(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(0,1);
    path.cubicTo(2,3, 5,2, 3,0);
    path.lineTo(0,1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(2,5);
    pathB.cubicTo(0,3, 1,0, 3,2);
    pathB.lineTo(2,5);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void cubicOp83i(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(0,3, 2,1, 4,1);
    path.lineTo(0,1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1,2);
    pathB.cubicTo(1,4, 1,0, 3,0);
    pathB.lineTo(1,2);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void cubicOp84d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,4);
    path.cubicTo(2,3, 6,3, 3,2);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(3,6);
    pathB.cubicTo(2,3, 4,0, 3,2);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void skpClip1(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(1126.17114f, 877.171204f);
    path.quadTo(1127.34314f, 876.000000f, 1129.00000f, 876.000000f);
    path.lineTo(1243.00000f, 876.000000f);
    path.quadTo(1244.65686f, 876.000000f, 1245.82886f, 877.171204f);
    path.quadTo(1247.00000f, 878.343140f, 1247.00000f, 880.000000f);
    path.lineTo(1247.00000f, 907.000000f);
    path.lineTo(1246.00000f, 907.000000f);
    path.lineTo(1246.00000f, 880.000000f);
    path.cubicTo(1246.00000f, 878.343140f, 1244.65686f, 877.000000f, 1243.00000f, 877.000000f);
    path.lineTo(1129.00000f, 877.000000f);
    path.cubicTo(1127.34314f, 877.000000f, 1126.00000f, 878.343140f, 1126.00000f, 880.000000f);
    path.lineTo(1126.00000f, 907.000000f);
    path.lineTo(1125.00000f, 907.000000f);
    path.lineTo(1125.00000f, 880.000000f);
    path.quadTo(1125.00000f, 878.343140f, 1126.17114f, 877.171204f);
    path.close();
    SkPath pathB;
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1247.00000f, 876.000000f);
    pathB.lineTo(1231.00000f, 892.000000f);
    pathB.lineTo(1246.00000f, 907.000000f);
    pathB.lineTo(1247.00000f, 907.000000f);
    pathB.lineTo(1247.00000f, 876.000000f);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void skpClip2(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(134.000000f, 11414.0000f);
    path.cubicTo(131.990234f, 11414.0000f, 130.326660f, 11415.4824f, 130.042755f, 11417.4131f);
    path.cubicTo(130.233124f, 11418.3193f, 131.037079f, 11419.0000f, 132.000000f, 11419.0000f);
    path.lineTo(806.000000f, 11419.0000f);
    path.cubicTo(806.962891f, 11419.0000f, 807.766907f, 11418.3193f, 807.957275f, 11417.4131f);
    path.cubicTo(807.673401f, 11415.4824f, 806.009766f, 11414.0000f, 804.000000f, 11414.0000f);
    path.lineTo(134.000000f, 11414.0000f);
    path.close();
    SkPath pathB;
    pathB.setFillType(SkPath::kInverseWinding_FillType);
    pathB.moveTo(132.000000f, 11415.0000f);
    pathB.lineTo(806.000000f, 11415.0000f);
    pathB.cubicTo(807.104553f, 11415.0000f, 808.000000f, 11415.4473f, 808.000000f, 11416.0000f);
    pathB.lineTo(808.000000f, 11417.0000f);
    pathB.cubicTo(808.000000f, 11418.1045f, 807.104553f, 11419.0000f, 806.000000f, 11419.0000f);
    pathB.lineTo(132.000000f, 11419.0000f);
    pathB.cubicTo(130.895432f, 11419.0000f, 130.000000f, 11418.1045f, 130.000000f, 11417.0000f);
    pathB.lineTo(130.000000f, 11416.0000f);
    pathB.cubicTo(130.000000f, 11415.4473f, 130.895432f, 11415.0000f, 132.000000f, 11415.0000f);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void skp96prezzi1(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(157.464005f, 670.463989f);
    path.quadTo(158.928925f, 669.000000f, 161.000000f, 669.000000f);
    path.lineTo(248.000000f, 669.000000f);
    path.quadTo(250.071075f, 669.000000f, 251.535995f, 670.463989f);
    path.quadTo(253.000000f, 671.928955f, 253.000000f, 674.000000f);
    path.lineTo(253.000000f, 706.000000f);
    path.lineTo(251.000000f, 706.000000f);
    path.lineTo(251.000000f, 675.000000f);
    path.cubicTo(251.000000f, 672.790833f, 249.209137f, 671.000000f, 247.000000f, 671.000000f);
    path.lineTo(162.000000f, 671.000000f);
    path.cubicTo(159.790863f, 671.000000f, 158.000000f, 672.790833f, 158.000000f, 675.000000f);
    path.lineTo(158.000000f, 706.000000f);
    path.lineTo(156.000000f, 706.000000f);
    path.lineTo(156.000000f, 674.000000f);
    path.quadTo(156.000000f, 671.928955f, 157.464005f, 670.463989f);
    path.close();
    SkPath pathB;
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(156.000000f, 669.000000f);
    pathB.lineTo(178.500000f, 691.500000f);
    pathB.lineTo(230.500000f, 691.500000f);
    pathB.lineTo(253.000000f, 669.000000f);
    pathB.lineTo(156.000000f, 669.000000f);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void skpancestry_com1(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(161.000000f, 925.000000f);
    path.cubicTo(159.874390f, 925.000000f, 158.835663f, 925.371948f, 158.000000f, 925.999634f);
    path.lineTo(158.000000f, 926.000000f);
    path.lineTo(1108.00000f, 926.000000f);
    path.lineTo(1108.00000f, 925.999634f);
    path.cubicTo(1107.16443f, 925.371948f, 1106.12561f, 925.000000f, 1105.00000f, 925.000000f);
    path.lineTo(161.000000f, 925.000000f);
    path.close();
    SkPath pathB;
    pathB.setFillType(SkPath::kEvenOdd_FillType);
    pathB.moveTo(161.000000f, 926.000000f);
    pathB.lineTo(1105.00000f, 926.000000f);
    pathB.cubicTo(1107.20911f, 926.000000f, 1109.00000f, 927.790833f, 1109.00000f, 930.000000f);
    pathB.lineTo(1109.00000f, 956.000000f);
    pathB.cubicTo(1109.00000f, 958.209167f, 1107.20911f, 960.000000f, 1105.00000f, 960.000000f);
    pathB.lineTo(161.000000f, 960.000000f);
    pathB.cubicTo(158.790863f, 960.000000f, 157.000000f, 958.209167f, 157.000000f, 956.000000f);
    pathB.lineTo(157.000000f, 930.000000f);
    pathB.cubicTo(157.000000f, 927.790833f, 158.790863f, 926.000000f, 161.000000f, 926.000000f);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void skpeldorado_com_ua1(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(286.695129f, 291.000000f);
    path.lineTo(229.304855f, 561.000000f);
    path.lineTo(979.304871f, 561.000000f);
    path.lineTo(1036.69507f, 291.000000f);
    path.lineTo(286.695129f, 291.000000f);
    path.close();
    SkPath pathB;
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1006.69513f, 291.000000f);
    pathB.cubicTo(1023.26367f, 291.000000f, 1033.84021f, 304.431458f, 1030.31836f, 321.000000f);
    pathB.lineTo(985.681519f, 531.000000f);
    pathB.cubicTo(982.159790f, 547.568542f, 965.873413f, 561.000000f, 949.304871f, 561.000000f);
    pathB.lineTo(259.304871f, 561.000000f);
    pathB.cubicTo(242.736313f, 561.000000f, 232.159805f, 547.568542f, 235.681549f, 531.000000f);
    pathB.lineTo(280.318420f, 321.000000f);
    pathB.cubicTo(283.840179f, 304.431458f, 300.126587f, 291.000000f, 316.695129f, 291.000000f);
    pathB.lineTo(1006.69513f, 291.000000f);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void skpbyte_com1(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(968.000000f, 14.0000000f);
    path.cubicTo(965.238586f, 14.0000000f, 963.000000f, 16.2385769f, 963.000000f, 19.0000000f);
    path.lineTo(963.000000f, 32.0000000f);
    path.cubicTo(963.000000f, 34.7614250f, 965.238586f, 37.0000000f, 968.000000f, 37.0000000f);
    path.lineTo(1034.00000f, 37.0000000f);
    path.cubicTo(1036.76147f, 37.0000000f, 1039.00000f, 34.7614250f, 1039.00000f, 32.0000000f);
    path.lineTo(1039.00000f, 19.0000000f);
    path.cubicTo(1039.00000f, 16.2385769f, 1036.76147f, 14.0000000f, 1034.00000f, 14.0000000f);
    path.lineTo(968.000000f, 14.0000000f);
    path.close();
    SkPath pathB;
    pathB.setFillType(SkPath::kInverseWinding_FillType);
    pathB.moveTo(968.000000f, 14.0000000f);
    pathB.lineTo(1034.00000f, 14.0000000f);
    pathB.cubicTo(1036.76147f, 14.0000000f, 1039.00000f, 16.2385750f, 1039.00000f, 19.0000000f);
    pathB.lineTo(1039.00000f, 32.0000000f);
    pathB.cubicTo(1039.00000f, 34.2091408f, 1036.76147f, 36.0000000f, 1034.00000f, 36.0000000f);
    pathB.lineTo(968.000000f, 36.0000000f);
    pathB.cubicTo(965.238586f, 36.0000000f, 963.000000f, 34.2091408f, 963.000000f, 32.0000000f);
    pathB.lineTo(963.000000f, 19.0000000f);
    pathB.cubicTo(963.000000f, 16.2385750f, 965.238586f, 14.0000000f, 968.000000f, 14.0000000f);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void skphealth_com76(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(708.099182f, 7.09919119f);
    path.lineTo(708.099182f, 7.09920025f);
    path.quadTo(704.000000f, 11.2010098f, 704.000000f, 17.0000000f);
    path.lineTo(704.000000f, 33.0000000f);
    path.lineTo(705.000000f, 33.0000000f);
    path.lineTo(705.000000f, 17.0000000f);
    path.cubicTo(705.000000f, 13.4101496f, 706.455078f, 10.1601505f, 708.807617f, 7.80761385f);
    path.lineTo(708.099182f, 7.09919119f);
    path.close();
    SkPath pathB;
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(704.000000f, 3.00000000f);
    pathB.lineTo(704.000000f, 33.0000000f);
    pathB.lineTo(705.000000f, 33.0000000f);
    pathB.lineTo(719.500000f, 3.00000000f);
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void skpahrefs_com88(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(1099.82886f, 7.17117119f);
    path.lineTo(1099.12134f, 7.87867832f);
    path.cubicTo(1099.66418f, 8.42157173f, 1100.00000f, 9.17157173f, 1100.00000f, 10.0000000f);
    path.lineTo(1100.00000f, 28.0000000f);
    path.cubicTo(1100.00000f, 29.6568546f, 1098.65686f, 31.0000000f, 1097.00000f, 31.0000000f);
    path.lineTo(1088.00000f, 31.0000000f);
    path.lineTo(1088.00000f, 32.0000000f);
    path.lineTo(1097.00000f, 32.0000000f);
    path.quadTo(1098.65686f, 32.0000000f, 1099.82886f, 30.8288002f);
    path.quadTo(1101.00000f, 29.6568546f, 1101.00000f, 28.0000000f);
    path.lineTo(1101.00000f, 10.0000000f);
    path.quadTo(1101.00000f, 8.34314537f, 1099.82886f, 7.17119980f);
    path.lineTo(1099.82886f, 7.17117119f);
    path.close();
    SkPath pathB;
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1101.00000f, 6.00000000f);
    pathB.lineTo(1088.00000f, 6.00000000f);
    pathB.lineTo(1088.00000f, 19.0000000f);
    pathB.lineTo(1101.00000f, 32.0000000f);
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void skpahrefs_com29(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(1037.17114f, 7.17119980f);
    path.quadTo(1038.34314f, 6.00000000f, 1040.00000f, 6.00000000f);
    path.lineTo(1074.00000f, 6.00000000f);
    path.lineTo(1074.00000f, 32.0000000f);
    path.lineTo(1040.00000f, 32.0000000f);
    path.quadTo(1038.34314f, 32.0000000f, 1037.17114f, 30.8288002f);
    path.quadTo(1036.00000f, 29.6568546f, 1036.00000f, 28.0000000f);
    path.lineTo(1036.00000f, 10.0000000f);
    path.quadTo(1036.00000f, 8.34314537f, 1037.17114f, 7.17119980f);
    path.close();
    path.moveTo(1037.00000f, 10.0000000f);
    path.cubicTo(1037.00000f, 8.34314537f, 1038.34314f, 7.00000000f, 1040.00000f, 7.00000000f);
    path.lineTo(1073.00000f, 7.00000000f);
    path.lineTo(1073.00000f, 31.0000000f);
    path.lineTo(1040.00000f, 31.0000000f);
    path.cubicTo(1038.34314f, 31.0000000f, 1037.00000f, 29.6568546f, 1037.00000f, 28.0000000f);
    path.lineTo(1037.00000f, 10.0000000f);
    path.close();
    SkPath pathB;
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1036.00000f, 32.0000000f);
    pathB.lineTo(1049.00000f, 19.0000000f);
    pathB.lineTo(1073.00000f, 31.0000000f);
    pathB.lineTo(1074.00000f, 32.0000000f);
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void cubicOp85d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(1,6, 1,0, 6,2);
    path.close();
    SkPath pathB;
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,1);
    pathB.cubicTo(2,6, 1,0, 6,1);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void skpkkiste_to98(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(96, 122);
    path.cubicTo(94.6192932f, 122, 93.3692932f, 122.559647f, 92.4644699f, 123.46447f);
    path.lineTo(94.1715698f, 125.17157f);
    path.cubicTo(94.8954315f, 124.447708f, 95.8954315f, 124, 97, 124);
    path.lineTo(257, 124);
    path.cubicTo(258.104553f, 124, 259.104584f, 124.447708f, 259.82843f, 125.17157f);
    path.lineTo(261.535522f, 123.46447f);
    path.cubicTo(260.630707f, 122.559647f, 259.380707f, 122, 258, 122);
    path.lineTo(96, 122);
    path.close();
    SkPath pathB;
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(258, 122);
    pathB.cubicTo(260.761414f, 122, 263, 124.238579f, 263, 127);
    pathB.lineTo(263, 284);
    pathB.cubicTo(263, 286.761414f, 260.761414f, 289, 258, 289);
    pathB.lineTo(96, 289);
    pathB.cubicTo(93.2385788f, 289, 91, 286.761414f, 91, 284);
    pathB.lineTo(91, 127);
    pathB.cubicTo(91, 124.238579f, 93.2385788f, 122, 96, 122);
    pathB.lineTo(258, 122);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void issue1417(skiatest::Reporter* reporter, const char* filename) {
    SkPath path1;
    path1.moveTo(122.58908843994140625f, 82.2836456298828125f);
    path1.quadTo(129.8215789794921875f, 80, 138, 80);
    path1.quadTo(147.15692138671875f, 80, 155.1280364990234375f, 82.86279296875f);
    path1.lineTo(161.1764678955078125f, 100);
    path1.lineTo(161.1764678955078125f, 100);
    path1.lineTo(115.29412078857421875f, 100);
    path1.lineTo(115.29412078857421875f, 100);
    path1.lineTo(122.58908843994140625f, 82.2836456298828125f);
    path1.lineTo(122.58908843994140625f, 82.2836456298828125f);
    path1.close();
    path1.moveTo(98.68194580078125f, 140.343841552734375f);
    path1.lineTo(115.29412078857421875f, 100);
    path1.lineTo(115.29412078857421875f, 100);
    path1.lineTo(97.9337615966796875f, 100);
    path1.lineTo(97.9337615966796875f, 100);
    path1.quadTo(88, 112.94264984130859375f, 88, 130);
    path1.quadTo(88, 131.544830322265625f, 88.08148956298828125f, 133.0560302734375f);
    path1.lineTo(98.68194580078125f, 140.343841552734375f);
    path1.lineTo(98.68194580078125f, 140.343841552734375f);
    path1.close();
    path1.moveTo(136.969696044921875f, 166.6666717529296875f);
    path1.lineTo(98.68194580078125f, 140.343841552734375f);
    path1.lineTo(98.68194580078125f, 140.343841552734375f);
    path1.lineTo(93.45894622802734375f, 153.02825927734375f);
    path1.lineTo(93.45894622802734375f, 153.02825927734375f);
    path1.quadTo(96.94116973876953125f, 159.65185546875f, 102.64466094970703125f, 165.3553466796875f);
    path1.quadTo(110.7924652099609375f, 173.503143310546875f, 120.8179779052734375f, 177.1177825927734375f);
    path1.lineTo(136.969696044921875f, 166.6666717529296875f);
    path1.lineTo(136.969696044921875f, 166.6666717529296875f);
    path1.close();
    path1.moveTo(175.8309783935546875f, 141.5211334228515625f);
    path1.lineTo(136.969696044921875f, 166.6666717529296875f);
    path1.lineTo(136.969696044921875f, 166.6666717529296875f);
    path1.lineTo(153.15728759765625f, 177.7956390380859375f);
    path1.lineTo(153.15728759765625f, 177.7956390380859375f);
    path1.quadTo(164.392425537109375f, 174.318267822265625f, 173.3553466796875f, 165.3553466796875f);
    path1.quadTo(177.805816650390625f, 160.9048614501953125f, 180.90380859375f, 155.8941650390625f);
    path1.lineTo(175.8309783935546875f, 141.5211334228515625f);
    path1.lineTo(175.8309783935546875f, 141.5211334228515625f);
    path1.close();
    path1.moveTo(175.8309783935546875f, 141.5211334228515625f);
    path1.lineTo(187.8782806396484375f, 133.7258148193359375f);
    path1.lineTo(187.8782806396484375f, 133.7258148193359375f);
    path1.quadTo(188, 131.8880615234375f, 188, 130);
    path1.quadTo(188, 112.942657470703125f, 178.0662384033203125f, 100);
    path1.lineTo(161.1764678955078125f, 100);
    path1.lineTo(161.1764678955078125f, 100);
    path1.lineTo(175.8309783935546875f, 141.5211334228515625f);
    path1.lineTo(175.8309783935546875f, 141.5211334228515625f);
    path1.close();

    SkPath path2;
    path2.moveTo(174.117645263671875f, 100);
    path2.lineTo(161.1764678955078125f, 100);
    path2.lineTo(161.1764678955078125f, 100);
    path2.lineTo(155.1280364990234375f, 82.86279296875f);
    path2.lineTo(155.1280364990234375f, 82.86279296875f);
    path2.quadTo(153.14971923828125f, 82.15229034423828125f, 151.098419189453125f, 81.618133544921875f);
    path2.lineTo(143.5294189453125f, 100);
    path2.lineTo(143.5294189453125f, 100);
    path2.lineTo(161.1764678955078125f, 100);
    path2.lineTo(161.1764678955078125f, 100);
    path2.lineTo(168.23529052734375f, 120);
    path2.lineTo(168.23529052734375f, 120);
    path2.lineTo(181.1764678955078125f, 120);
    path2.lineTo(181.1764678955078125f, 120);
    path2.lineTo(186.3661956787109375f, 134.7042236328125f);
    path2.lineTo(186.3661956787109375f, 134.7042236328125f);
    path2.lineTo(187.8782806396484375f, 133.7258148193359375f);
    path2.lineTo(187.8782806396484375f, 133.7258148193359375f);
    path2.quadTo(188, 131.8880615234375f, 188, 130);
    path2.quadTo(188, 124.80947113037109375f, 187.080169677734375f, 120);
    path2.lineTo(181.1764678955078125f, 120);
    path2.lineTo(181.1764678955078125f, 120);
    path2.lineTo(174.117645263671875f, 100);
    path2.lineTo(174.117645263671875f, 100);
    path2.close();
    path2.moveTo(88.91983795166015625f, 120);
    path2.lineTo(107.0588226318359375f, 120);
    path2.lineTo(107.0588226318359375f, 120);
    path2.lineTo(98.68194580078125f, 140.343841552734375f);
    path2.lineTo(98.68194580078125f, 140.343841552734375f);
    path2.lineTo(88.08148956298828125f, 133.0560302734375f);
    path2.lineTo(88.08148956298828125f, 133.0560302734375f);
    path2.quadTo(88, 131.544830322265625f, 88, 130);
    path2.quadTo(88, 124.80951690673828125f, 88.91983795166015625f, 120);
    path2.close();
    path2.moveTo(96.67621612548828125f, 145.21490478515625f);
    path2.lineTo(98.68194580078125f, 140.343841552734375f);
    path2.lineTo(98.68194580078125f, 140.343841552734375f);
    path2.lineTo(120.68767547607421875f, 155.4727783203125f);
    path2.lineTo(120.68767547607421875f, 155.4727783203125f);
    path2.lineTo(118.68194580078125f, 160.343841552734375f);
    path2.lineTo(118.68194580078125f, 160.343841552734375f);
    path2.lineTo(96.67621612548828125f, 145.21490478515625f);
    path2.lineTo(96.67621612548828125f, 145.21490478515625f);
    path2.close();
    path2.moveTo(113.232177734375f, 173.5789947509765625f);
    path2.quadTo(116.8802642822265625f, 175.69805908203125f, 120.8179779052734375f, 177.1177825927734375f);
    path2.lineTo(132.2864990234375f, 169.6969757080078125f);
    path2.lineTo(132.2864990234375f, 169.6969757080078125f);
    path2.lineTo(118.68194580078125f, 160.343841552734375f);
    path2.lineTo(118.68194580078125f, 160.343841552734375f);
    path2.lineTo(113.232177734375f, 173.5789947509765625f);
    path2.lineTo(113.232177734375f, 173.5789947509765625f);
    path2.close();
    // FIXME : difficult data, circle back later
    testPathOp(reporter, path1, path2, kUnion_SkPathOp, filename);
}

static void issue1418(skiatest::Reporter* reporter, const char* filename) {
    SkPath path1;
    path1.moveTo(0, 0);
    path1.lineTo(1, 0);
    path1.lineTo(1, 0);
    path1.lineTo(1, 1);
    path1.lineTo(1, 1);
    path1.lineTo(0, 1);
    path1.lineTo(0, 1);
    path1.lineTo(0, 0);
    path1.lineTo(0, 0);
    path1.close();

    SkPath path2;
    path2.moveTo(0.64644664525985717773f, -0.35355341434478759766f);
    path2.quadTo(0.79289329051971435547f, -0.50000005960464477539f, 1.0000001192092895508f, -0.50000005960464477539f);
    path2.quadTo(1.2071068286895751953f, -0.50000005960464477539f, 1.3535535335540771484f, -0.35355341434478759766f);
    path2.quadTo(1.5000001192092895508f, -0.20710679888725280762f, 1.5000001192092895508f, 0);
    path2.quadTo(1.5000001192092895508f, 0.20710679888725280762f, 1.3535535335540771484f, 0.35355341434478759766f);
    path2.quadTo(1.2071068286895751953f, 0.50000005960464477539f, 1.0000001192092895508f, 0.50000005960464477539f);
    path2.quadTo(0.79289329051971435547f, 0.50000005960464477539f, 0.64644664525985717773f, 0.35355341434478759766f);
    path2.quadTo(0.50000005960464477539f, 0.20710679888725280762f, 0.50000005960464477539f, 0);
    path2.quadTo(0.50000005960464477539f, -0.20710679888725280762f, 0.64644664525985717773f, -0.35355341434478759766f);
    testPathOp(reporter, path1, path2, kIntersect_SkPathOp, filename);
}

static void cubicOp85i(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(3, 4);
    path.cubicTo(1, 5, 4, 3, 6, 4);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(3, 4);
    pathB.cubicTo(4, 6, 4, 3, 5, 1);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void issue1418b(skiatest::Reporter* reporter, const char* filename) {
    SkPath path1;
    path1.moveTo(0, 0);
    path1.lineTo(1, 0);
    path1.lineTo(1, 1);
    path1.lineTo(0, 1);
    path1.lineTo(0, 0);
    path1.close();
    path1.setFillType(SkPath::kWinding_FillType);
    SkPath path2;
    path2.moveTo(0.646446645f, -0.353553414f);
    path2.quadTo(0.792893291f, -0.50000006f, 1.00000012f, -0.50000006f);
    path2.quadTo(1.20710683f, -0.50000006f, 1.35355353f, -0.353553414f);
    path2.quadTo(1.50000012f, -0.207106799f, 1.50000012f, 0);
    path2.quadTo(1.50000012f, 0.207106799f, 1.35355353f, 0.353553414f);
    path2.quadTo(1.20710683f, 0.50000006f, 1.00000012f, 0.50000006f);
    path2.quadTo(0.792893291f, 0.50000006f, 0.646446645f, 0.353553414f);
    path2.quadTo(0.50000006f, 0.207106799f, 0.50000006f, 0);
    path2.quadTo(0.50000006f, -0.207106799f, 0.646446645f, -0.353553414f);
    path2.close();
    path2.moveTo(1.00000012f, 0.50000006f);
    path2.lineTo(1.00000012f, 1.00000012f);
    path2.lineTo(0.50000006f, 1.00000012f);
    path2.quadTo(0.50000006f, 0.792893291f, 0.646446645f, 0.646446645f);
    path2.quadTo(0.792893291f, 0.50000006f, 1.00000012f, 0.50000006f);
    path2.close();
    path2.setFillType(SkPath::kEvenOdd_FillType);
    testPathOp(reporter, path1, path2, kIntersect_SkPathOp, filename);
}

static void rectOp1i(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    path.addRect(2, 2, 4, 4, SkPath::kCW_Direction);
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    pathB.addRect(0, 0, 2, 2, SkPath::kCW_Direction);
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void rectOp2i(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    path.addRect(0, 0, 3, 3, SkPath::kCW_Direction);
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.addRect(0, 0, 2, 2, SkPath::kCW_Direction);
    pathB.addRect(0, 0, 2, 2, SkPath::kCW_Direction);
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void rectOp3x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(0, 0);
    path.lineTo(3, 0);
    path.lineTo(3, 3);
    path.lineTo(0, 3);
    path.close();
    path.moveTo(2, 2);
    path.lineTo(3, 2);
    path.lineTo(3, 3);
    path.lineTo(2, 3);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1, 1);
    pathB.lineTo(3, 1);
    pathB.lineTo(3, 3);
    pathB.lineTo(1, 3);
    pathB.close();
    pathB.moveTo(2, 2);
    pathB.lineTo(3, 2);
    pathB.lineTo(3, 3);
    pathB.lineTo(2, 3);
    pathB.close();
    testPathOp(reporter, path, pathB, kXOR_SkPathOp, filename);
}

static void issue1435(skiatest::Reporter* reporter, const char* filename) {
    SkPath path1;
    path1.moveTo(160, 60);
    path1.lineTo(220, 230);
    path1.lineTo(60, 120);
    path1.lineTo(260, 120);
    path1.lineTo(90, 230);
    path1.lineTo(160, 60);
    path1.close();
    path1.setFillType(SkPath::kEvenOdd_FillType);

    SkPath path2;
    path2.moveTo(142.589081f, 102.283646f);
    path2.quadTo(149.821579f, 100, 158, 100);
    path2.quadTo(167.156921f, 100, 175.128036f, 102.862793f);
    path2.lineTo(181.176468f, 120);
    path2.lineTo(135.294128f, 120);
    path2.lineTo(142.589081f, 102.283646f);
    path2.close();
    path2.moveTo(118.681946f, 160.343842f);
    path2.lineTo(135.294128f, 120);
    path2.lineTo(117.933762f, 120);
    path2.quadTo(108, 132.942657f, 108, 150);
    path2.quadTo(108, 151.54483f, 108.08149f, 153.05603f);
    path2.lineTo(118.681946f, 160.343842f);
    path2.close();
    path2.moveTo(156.969696f, 186.666672f);
    path2.lineTo(118.681946f, 160.343842f);
    path2.lineTo(113.458946f, 173.028259f);
    path2.quadTo(116.94117f, 179.651855f, 122.644661f, 185.355347f);
    path2.quadTo(130.792465f, 193.503143f, 140.817978f, 197.117783f);
    path2.lineTo(156.969696f, 186.666672f);
    path2.close();
    path2.moveTo(195.830978f, 161.521133f);
    path2.lineTo(156.969696f, 186.666672f);
    path2.lineTo(173.157288f, 197.795639f);
    path2.quadTo(184.392426f, 194.318268f, 193.355347f, 185.355347f);
    path2.quadTo(197.805817f, 180.904861f, 200.903809f, 175.894165f);
    path2.lineTo(195.830978f, 161.521133f);
    path2.close();
    path2.moveTo(195.830978f, 161.521133f);
    path2.lineTo(207.878281f, 153.725815f);
    path2.quadTo(208, 151.888062f, 208, 150);
    path2.quadTo(208, 132.942657f, 198.066238f, 120);
    path2.lineTo(181.176468f, 120);
    path2.lineTo(195.830978f, 161.521133f);
    path2.close();
    path2.setFillType(SkPath::kEvenOdd_FillType);
    testPathOp(reporter, path1, path2, kIntersect_SkPathOp, filename);
}

static void skpkkiste_to716(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(1173, 284);
    path.cubicTo(1173, 285.125824f, 1173.37207f, 286.164734f, 1174, 287.000488f);
    path.lineTo(1174, 123.999496f);
    path.cubicTo(1173.37207f, 124.835243f, 1173, 125.874168f, 1173, 127);
    path.lineTo(1173, 284);
    path.close();
    SkPath pathB;
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1340, 122);
    pathB.cubicTo(1342.76147f, 122, 1345, 124.238579f, 1345, 127);
    pathB.lineTo(1345, 284);
    pathB.cubicTo(1345, 286.761414f, 1342.76147f, 289, 1340, 289);
    pathB.lineTo(1178, 289);
    pathB.cubicTo(1175.23853f, 289, 1173, 286.761414f, 1173, 284);
    pathB.lineTo(1173, 127);
    pathB.cubicTo(1173, 124.238579f, 1175.23853f, 122, 1178, 122);
    pathB.lineTo(1340, 122);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void loopEdge1(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(0,0);
    path.lineTo(3,0);
    path.lineTo(3,2);
    path.lineTo(1,2);
    path.lineTo(1,1);
    path.lineTo(2,1);
    path.lineTo(2,3);
    path.lineTo(0,3);
    path.close();
    SkPath pathB;
    pathB.setFillType(SkPath::kEvenOdd_FillType);
    pathB.moveTo(1,2);
    pathB.lineTo(2,2);
    pathB.lineTo(2,4);
    pathB.lineTo(1,4);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void loopEdge2(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(0,0);
    path.lineTo(3,0);
    path.lineTo(3,2);
    path.lineTo(1,2);
    path.lineTo(1,1);
    path.lineTo(2,1);
    path.lineTo(2,3);
    path.lineTo(0,3);
    path.close();
    SkPath pathB;
    pathB.setFillType(SkPath::kEvenOdd_FillType);
    pathB.moveTo(1 - 1e-6f,2);
    pathB.lineTo(2 - 1e-6f,2);
    pathB.lineTo(2 - 1e-6f,4);
    pathB.lineTo(1 - 1e-6f,4);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void cubicOp86i(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0, 4);
    path.cubicTo(3, 4, 6, 2, 5, 2);
    path.close();
    pathB.setFillType(SkPath::kEvenOdd_FillType);
    pathB.moveTo(2, 6);
    pathB.cubicTo(2, 5, 4, 0, 4, 3);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void cubicOp87u(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(0,2, 2,0, 6,4);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,2);
    pathB.cubicTo(4,6, 1,0, 2,0);
    pathB.close();
    markTestFlakyForPathKit();
    testPathOp(reporter, path, pathB, kUnion_SkPathOp, filename);
}

static void cubicOp88u(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(2,5, 5,0, 6,4);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,5);
    pathB.cubicTo(4,6, 1,0, 5,2);
    pathB.close();
    testPathOp(reporter, path, pathB, kUnion_SkPathOp, filename);
}

static void cubicOp89u(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0, 3);
    path.cubicTo(1, 6, 5, 0, 6, 3);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0, 5);
    pathB.cubicTo(3, 6, 3, 0, 6, 1);
    pathB.close();
    testPathOp(reporter, path, pathB, kUnion_SkPathOp, filename);
}

static void cubicOp90u(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(0, 5);
    path.cubicTo(1, 2, 5, 2, 4, 1);
    path.close();
    pathB.setFillType(SkPath::kEvenOdd_FillType);
    pathB.moveTo(2, 5);
    pathB.cubicTo(1, 4, 5, 0, 2, 1);
    pathB.close();
    testPathOp(reporter, path, pathB, kUnion_SkPathOp, filename);
}

static void cubicOp91u(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(1, 6);
    path.cubicTo(0, 3, 6, 3, 5, 0);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(3, 6);
    pathB.cubicTo(0, 5, 6, 1, 3, 0);
    pathB.close();
    testPathOp(reporter, path, pathB, kUnion_SkPathOp, filename);
}

static void skpaaalgarve_org53(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
   path.moveTo(-1.24344979e-014f, 348);
    path.lineTo(258, 348);
    path.lineTo(258, 322);
    path.quadTo(258, 317.857849f, 255.072006f, 314.928009f);
    path.quadTo(252.142136f, 312, 248, 312);
    path.lineTo(1.77635684e-015f, 312);
    path.lineTo(-1.24344979e-014f, 348);
    path.close();
    SkPath pathB;
    pathB.setFillType(SkPath::kWinding_FillType);
   pathB.moveTo(0, 312);
    pathB.lineTo(258, 312);
    pathB.lineTo(258, 348);
    pathB.lineTo(0, 348);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void skpabcspark_ca103(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(1.99840144e-015f, 494);
    path.lineTo(97, 494);
    path.quadTo(100.313705f, 494, 102.6576f, 491.657593f);
    path.quadTo(105, 489.313721f, 105, 486);
    path.lineTo(105, 425);
    path.quadTo(105, 421.686279f, 102.6576f, 419.342407f);
    path.quadTo(100.313705f, 417, 97, 417);
    path.lineTo(2.22044605e-016f, 417);
    path.lineTo(1.99840144e-015f, 494);
    path.close();
    SkPath pathB;
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0, 417);
    pathB.lineTo(105, 417);
    pathB.lineTo(105, 494);
    pathB.lineTo(0, 494);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void skpacesoftech_com47(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(670.537415f, 285);
    path.lineTo(670.387451f, 285);
    path.lineTo(596.315186f, 314.850708f);
    path.lineTo(626.19696f, 389);
    path.lineTo(626.346863f, 389);
    path.lineTo(700.419189f, 359.149261f);
    path.lineTo(670.537415f, 285);
    path.close();
    SkPath pathB;
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(663.318542f, 374.100616f);
    pathB.quadTo(647.950989f, 380.293671f, 632.705322f, 373.806305f);
    pathB.quadTo(617.459595f, 367.318909f, 611.266541f, 351.951355f);
    pathB.quadTo(605.073486f, 336.58374f, 611.560913f, 321.338074f);
    pathB.quadTo(618.048279f, 306.092407f, 633.415833f, 299.899353f);
    pathB.quadTo(648.783447f, 293.706299f, 664.029114f, 300.193665f);
    pathB.quadTo(679.27478f, 306.68103f, 685.467834f, 322.048645f);
    pathB.quadTo(691.660889f, 337.416199f, 685.173523f, 352.661896f);
    pathB.quadTo(678.686157f, 367.907562f, 663.318542f, 374.100616f);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void skpact_com43(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(1.45716772e-016f, 924.336121f);
    path.lineTo(-1.11022302e-016f, 920);
    path.lineTo(6, 920);
    path.lineTo(6, 926);
    path.lineTo(1.66389287f, 926);
    path.quadTo(1.18842196f, 925.674561f, 0.756800175f, 925.243225f);
    path.quadTo(0.325406998f, 924.811523f, 1.45716772e-016f, 924.336121f);
    path.close();
    path.moveTo(1, 921);
    path.lineTo(5, 921);
    path.lineTo(5, 925);
    path.cubicTo(2.79086018f, 925, 1, 923.209167f, 1, 921);
    path.close();
    SkPath pathB;
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(-1, 920);
    pathB.lineTo(0, 920);
    pathB.lineTo(3, 927);
    pathB.lineTo(-1, 927);
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void skpadbox_lt8(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(320.097229f, 628.573669f);
    path.lineTo(610.227173f, 85.7786865f);
    path.lineTo(946.652588f, 265.601807f);
    path.lineTo(656.522644f, 808.39679f);
    path.lineTo(320.097229f, 628.573669f);
    path.close();
    SkPath pathB;
    pathB.setFillType(SkPath::kInverseWinding_FillType);
    pathB.moveTo(333.866608f, 623.496155f);
    pathB.lineTo(613.368042f, 100.585754f);
    pathB.cubicTo(613.685303f, 99.9921265f, 614.423767f, 99.7681885f, 615.017395f, 100.085449f);
    pathB.lineTo(932.633057f, 269.854553f);
    pathB.cubicTo(933.226685f, 270.171875f, 933.450623f, 270.910278f, 933.133301f, 271.503906f);
    pathB.lineTo(653.631897f, 794.414307f);
    pathB.cubicTo(653.314636f, 795.007935f, 652.576172f, 795.231934f, 651.982544f, 794.914612f);
    pathB.lineTo(334.366943f, 625.145508f);
    pathB.cubicTo(333.773315f, 624.828247f, 333.549286f, 624.089783f, 333.866608f, 623.496155f);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void skpadindex_de4(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(0, 926);
    path.lineTo(0, 0);
    path.lineTo(1280, 0);
    path.lineTo(1280, 926);
    path.lineTo(0, 926);
    path.close();
    SkPath pathB;
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0, 312);
    pathB.lineTo(8.20486257e-015f, 178);
    pathB.lineTo(49, 178);
    pathB.lineTo(49, 312);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void skpadithya_putr4_blogspot_com551(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(205.605804f, 142.334625f);
    path.lineTo(254.665359f, 85.6058044f);
    path.lineTo(311.394196f, 134.665359f);
    path.lineTo(262.334625f, 191.39418f);
    path.lineTo(205.605804f, 142.334625f);
    path.close();
    SkPath pathB;
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(283.407959f, 110.462646f);
    pathB.cubicTo(298.864319f, 123.829437f, 300.558258f, 147.195221f, 287.191467f, 162.651581f);
    pathB.lineTo(286.537354f, 163.407959f);
    pathB.cubicTo(273.170563f, 178.864334f, 249.804779f, 180.558258f, 234.348419f, 167.191467f);
    pathB.lineTo(233.592026f, 166.537338f);
    pathB.cubicTo(218.135666f, 153.170547f, 216.441727f, 129.804779f, 229.808517f, 114.348412f);
    pathB.lineTo(230.462646f, 113.592026f);
    pathB.cubicTo(243.829437f, 98.1356659f, 267.195221f, 96.4417267f, 282.651581f, 109.808517f);
    pathB.lineTo(283.407959f, 110.462646f);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void skpadspert_de11(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(-4.4408921e-016f, 682.5f);
    path.lineTo(30.5f, 682.5f);
    path.cubicTo(32.709137f, 682.5f, 34.5f, 680.709167f, 34.5f, 678.5f);
    path.lineTo(34.5f, 486.5f);
    path.cubicTo(34.5f, 484.290863f, 32.709137f, 482.5f, 30.5f, 482.5f);
    path.lineTo(0, 482.5f);
    path.lineTo(-4.4408921e-016f, 682.5f);
    path.close();
    SkPath pathB;
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0, 482);
    pathB.lineTo(35, 482);
    pathB.lineTo(35, 683);
    pathB.lineTo(0, 683);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void skpaiaigames_com870(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(324.071075f, 845.071045f);
    path.cubicTo(324.405151f, 844.737f, 324.715668f, 844.379395f, 325, 844.000977f);
    path.lineTo(325, 842.127197f);
    path.cubicTo(324.571411f, 842.956238f, 324.017761f, 843.710144f, 323.363953f, 844.363953f);
    path.lineTo(324.071075f, 845.071045f);
    path.close();
    path.moveTo(323.363953f, 714.636047f);
    path.lineTo(324.071075f, 713.928955f);
    path.cubicTo(324.405151f, 714.263f, 324.715668f, 714.620605f, 325, 714.999023f);
    path.lineTo(325, 716.872803f);
    path.cubicTo(324.571411f, 716.043762f, 324.017761f, 715.289856f, 323.363953f, 714.636047f);
    path.close();
    SkPath pathB;
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(317, 711);
    pathB.cubicTo(322.522858f, 711, 327, 715.477173f, 327, 721);
    pathB.lineTo(327, 838);
    pathB.cubicTo(327, 843.522827f, 322.522858f, 848, 317, 848);
    pathB.lineTo(155, 848);
    pathB.cubicTo(149.477158f, 848, 145, 843.522827f, 145, 838);
    pathB.lineTo(145, 721);
    pathB.cubicTo(145, 715.477173f, 149.477158f, 711, 155, 711);
    pathB.lineTo(317, 711);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void cubicOp92i(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0, 1);
    path.cubicTo(2, 6, 4, 1, 5, 4);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1, 4);
    pathB.cubicTo(4, 5, 1, 0, 6, 2);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void cubicOp93d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0, 1);
    path.cubicTo(1, 6, 4, 1, 4, 3);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1, 4);
    pathB.cubicTo(3, 4, 1, 0, 6, 1);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp94u(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(0, 3);
    path.cubicTo(2, 3, 5, 0, 5, 3);
    path.close();
    pathB.setFillType(SkPath::kEvenOdd_FillType);
    pathB.moveTo(0, 5);
    pathB.cubicTo(3, 5, 3, 0, 3, 2);
    pathB.close();
    testPathOp(reporter, path, pathB, kUnion_SkPathOp, filename);
}

static void skpadbox_lt15(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(333.292084f, 624.570984f);
    path.lineTo(614.229797f, 98.9735107f);
    path.lineTo(933.457764f, 269.604431f);
    path.lineTo(652.52002f, 795.201904f);
    path.lineTo(333.292084f, 624.570984f);
    path.close();
    SkPath pathB;
     pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(613.368042f, 100.585754f);
    pathB.cubicTo(613.685303f, 99.9921265f, 614.423767f, 99.7681885f, 615.017395f, 100.085449f);
    pathB.lineTo(932.633057f, 269.854553f);
    pathB.cubicTo(933.226685f, 270.171875f, 933.450623f, 270.910278f, 933.133301f, 271.503906f);
    pathB.lineTo(653.631897f, 794.414307f);
    pathB.cubicTo(653.314636f, 795.007935f, 652.576172f, 795.231934f, 651.982544f, 794.914612f);
    pathB.lineTo(334.366943f, 625.145508f);
    pathB.cubicTo(333.773315f, 624.828247f, 333.549286f, 624.089783f, 333.866608f, 623.496155f);
    pathB.lineTo(613.368042f, 100.585754f);
     pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void skpadoption_org196(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(802, 367);
    path.lineTo(802, 324);
    path.lineTo(956, 324);
    path.lineTo(956, 371);
    path.quadTo(956, 373.071075f, 954.536011f, 374.536011f);
    path.quadTo(953.071045f, 376, 951, 376);
    path.lineTo(811, 376);
    path.cubicTo(806.029419f, 376, 802, 371.970551f, 802, 367);
    path.close();
    SkPath pathB;
    pathB.setFillType(SkPath::kInverseWinding_FillType);
    pathB.moveTo(803, 326);
    pathB.lineTo(955, 326);
    pathB.lineTo(955, 370);
    pathB.cubicTo(955, 372.761414f, 952.761414f, 375, 950, 375);
    pathB.lineTo(808, 375);
    pathB.cubicTo(805.238586f, 375, 803, 372.761414f, 803, 370);
    pathB.lineTo(803, 326);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void skpadspert_net23(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(-2.220446e-018f, 483.5f);
    path.lineTo(0, 482.5f);
    path.lineTo(30.5f, 482.5f);
    path.cubicTo(32.709137f, 482.5f, 34.5f, 484.290863f, 34.5f, 486.5f);
    path.lineTo(34.5f, 678.5f);
    path.cubicTo(34.5f, 680.709167f, 32.709137f, 682.5f, 30.5f, 682.5f);
    path.lineTo(-4.4408921e-016f, 682.5f);
    path.lineTo(-4.41868766e-016f, 681.5f);
    path.lineTo(30.5f, 681.5f);
    path.cubicTo(32.1568565f, 681.5f, 33.5f, 680.15686f, 33.5f, 678.5f);
    path.lineTo(33.5f, 486.5f);
    path.cubicTo(33.5f, 484.84314f, 32.1568565f, 483.5f, 30.5f, 483.5f);
    path.lineTo(-2.220446e-018f, 483.5f);
    path.close();
    SkPath pathB;
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0, 482);
    pathB.lineTo(35, 482);
    pathB.lineTo(35, 683);
    pathB.lineTo(0, 683);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void skpadventistmission_org572(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(1182.00037f, 926);
    path.cubicTo(1181.08813f, 924.785583f, 1179.63586f, 924, 1178, 924);
    path.lineTo(938, 924);
    path.cubicTo(936.364197f, 924, 934.911865f, 924.785583f, 933.999634f, 926);
    path.lineTo(1182.00037f, 926);
    path.close();
    SkPath pathB;
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(934, 924);
    pathB.lineTo(1182, 924);
    pathB.lineTo(1182, 926);
    pathB.lineTo(934, 926);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void skpagentxsites_com55(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(925, 27);
    path.cubicTo(924.447693f, 27, 924, 27.4477158f, 924, 28);
    path.lineTo(924, 55);
    path.cubicTo(924, 55.5522842f, 924.447693f, 56, 925, 56);
    path.lineTo(1103, 56);
    path.cubicTo(1103.55225f, 56, 1104, 55.5522842f, 1104, 55);
    path.lineTo(1104, 28);
    path.cubicTo(1104, 27.4477158f, 1103.55225f, 27, 1103, 27);
    path.lineTo(925, 27);
    path.close();
    SkPath pathB;
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1103, 27);
    pathB.cubicTo(1104.10461f, 27, 1105, 27.8954315f, 1105, 29);
    pathB.lineTo(1105, 54);
    pathB.cubicTo(1105, 55.1045685f, 1104.10461f, 56, 1103, 56);
    pathB.lineTo(926, 56);
    pathB.cubicTo(924.895447f, 56, 924, 55.1045685f, 924, 54);
    pathB.lineTo(924, 29);
    pathB.cubicTo(924, 27.8954315f, 924.895447f, 27, 926, 27);
    pathB.lineTo(1103, 27);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void skpbakosoft_com10(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(190, 170);
    path.cubicTo(178.9543f, 170, 170, 178.9543f, 170, 190);
    path.cubicTo(170, 201.0457f, 178.9543f, 210, 190, 210);
    path.lineTo(370, 210);
    path.cubicTo(381.045685f, 210, 390, 201.0457f, 390, 190);
    path.cubicTo(390, 178.9543f, 381.045685f, 170, 370, 170);
    path.lineTo(190, 170);
    path.close();
    SkPath pathB;
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(210, 190);
    pathB.quadTo(210, 198.284271f, 204.142136f, 204.142136f);
    pathB.quadTo(198.284271f, 210, 190, 210);
    pathB.quadTo(181.715729f, 210, 175.857864f, 204.142136f);
    pathB.quadTo(170, 198.284271f, 170, 190);
    pathB.quadTo(170, 181.715729f, 175.857864f, 175.857864f);
    pathB.quadTo(181.715729f, 170, 190, 170);
    pathB.quadTo(198.284271f, 170, 204.142136f, 175.857864f);
    pathB.quadTo(210, 181.715729f, 210, 190);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void skpbambootheme_com12(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(47.8780937f, 58);
    path.lineTo(0, 58);
    path.lineTo(-8.65973959e-015f, 96.9914017f);
    path.quadTo(20.0654926f, 96.6451874f, 34.3553391f, 82.3553391f);
    path.quadTo(44.9466133f, 71.764061f, 47.8780937f, 58);
    path.close();
    SkPath pathB;
    pathB.setFillType(SkPath::kEvenOdd_FillType);
    pathB.moveTo(-1, -3);
    pathB.lineTo(-1, -3);
    pathB.cubicTo(26.6142502f, -3, 49, 19.3857498f, 49, 47);
    pathB.lineTo(49, 47);
    pathB.cubicTo(49, 74.6142502f, 26.6142502f, 97, -1, 97);
    pathB.lineTo(-1, 97);
    pathB.cubicTo(-28.6142502f, 97, -51, 74.6142502f, -51, 47);
    pathB.lineTo(-51, 47);
    pathB.cubicTo(-51, 19.3857498f, -28.6142502f, -3, -1, -3);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void skpakmmos_ru100(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(693.000488f, 926);
    path.cubicTo(692.164734f, 925.37207f, 691.125793f, 925, 690, 925);
    path.lineTo(578, 925);
    path.cubicTo(576.874207f, 925, 575.835266f, 925.37207f, 574.999512f, 926);
    path.lineTo(693.000488f, 926);
    path.close();
    SkPath pathB;
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(575, 925);
    pathB.lineTo(693, 925);
    pathB.lineTo(693, 926);
    pathB.lineTo(575, 926);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void skpcarpetplanet_ru22(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(195, 785);
    path.cubicTo(124.307556f, 785, 67, 841.859863f, 67, 912);
    path.lineTo(67, 913);
    path.cubicTo(67, 917.388916f, 67.2243805f, 921.725769f, 67.662384f, 926);
    path.lineTo(322, 926);
    path.lineTo(322, 896.048035f);
    path.cubicTo(314.09201f, 833.437622f, 260.247131f, 785, 195, 785);
    path.close();
    SkPath pathB;
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(195, 785);
    pathB.cubicTo(265.140167f, 785, 322, 842.307556f, 322, 913);
    pathB.cubicTo(322, 983.692444f, 265.140167f, 1041, 195, 1041);
    pathB.lineTo(194, 1041);
    pathB.cubicTo(123.85984f, 1041, 67, 983.692444f, 67, 913);
    pathB.cubicTo(67, 842.307556f, 123.85984f, 785, 194, 785);
    pathB.lineTo(195, 785);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void skpcarrot_is24(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(945, 597);
    path.quadTo(913.93396f, 597, 891.96698f, 618.96698f);
    path.quadTo(870, 640.93396f, 870, 672);
    path.quadTo(870, 703.06604f, 891.96698f, 725.03302f);
    path.quadTo(913.93396f, 747, 945, 747);
    path.quadTo(976.06604f, 747, 998.03302f, 725.03302f);
    path.quadTo(1020, 703.06604f, 1020, 672);
    path.quadTo(1020, 640.93396f, 998.03302f, 618.96698f);
    path.quadTo(976.06604f, 597, 945, 597);
    path.close();
    SkPath pathB;
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(945.080994f, 597.161987f);
    pathB.cubicTo(903.659973f, 597.161987f, 870.080994f, 630.73999f, 870.080994f, 672.161987f);
    pathB.cubicTo(870.080994f, 676.096008f, 870.387024f, 679.957031f, 870.971008f, 683.726013f);
    pathB.cubicTo(876.53302f, 719.656006f, 907.593994f, 747.161987f, 945.080994f, 747.161987f);
    pathB.cubicTo(982.567993f, 747.161987f, 1013.62903f, 719.656006f, 1019.19104f, 683.726013f);
    pathB.cubicTo(1019.77502f, 679.955017f, 1020.08099f, 676.094971f, 1020.08099f, 672.161987f);
    pathB.cubicTo(1020.08002f, 630.73999f, 986.502014f, 597.161987f, 945.080994f, 597.161987f);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void skpbangalorenest_com4(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(0, 926);
    path.lineTo(0, 0);
    path.lineTo(1265, 0);
    path.lineTo(1265, 926);
    path.lineTo(0, 926);
    path.close();
    SkPath pathB;
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0, 290);
    pathB.lineTo(-2.64514972e-014f, 146);
    pathB.lineTo(30, 146);
    pathB.lineTo(30, 290);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void skpbenzoteh_ru152(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(883, 23);
    path.lineTo(883, 0);
    path.lineTo(1122.5f, 0);
    path.lineTo(1122.5f, 25.2136822f);
    path.quadTo(1122.14441f, 25.9271851f, 1121.53601f, 26.5359993f);
    path.quadTo(1120.07104f, 28, 1118, 28);
    path.lineTo(888, 28);
    path.quadTo(885.928955f, 28, 884.463989f, 26.5359993f);
    path.quadTo(883, 25.0710678f, 883, 23);
    path.close();
    SkPath pathB;
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(883, 0);
    pathB.lineTo(1123, 0);
    pathB.lineTo(1123, 23);
    pathB.quadTo(1123, 25.0710678f, 1121.53601f, 26.5359993f);
    pathB.quadTo(1120.07104f, 28, 1118, 28);
    pathB.lineTo(888, 28);
    pathB.quadTo(885.928955f, 28, 884.463989f, 26.5359993f);
    pathB.quadTo(883, 25.0710678f, 883, 23);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void skpbestred_ru37(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(883, 23);
    path.lineTo(883, 0);
    path.lineTo(1122.5f, 0);
    path.lineTo(1122.5f, 25.2136822f);
    path.quadTo(1122.14441f, 25.9271851f, 1121.53601f, 26.5359993f);
    path.quadTo(1120.07104f, 28, 1118, 28);
    path.lineTo(888, 28);
    path.quadTo(885.928955f, 28, 884.463989f, 26.5359993f);
    path.quadTo(883, 25.0710678f, 883, 23);
    path.close();
    SkPath pathB;
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(883, 0);
    pathB.lineTo(1123, 0);
    pathB.lineTo(1123, 23);
    pathB.quadTo(1123, 25.0710678f, 1121.53601f, 26.5359993f);
    pathB.quadTo(1120.07104f, 28, 1118, 28);
    pathB.lineTo(888, 28);
    pathB.quadTo(885.928955f, 28, 884.463989f, 26.5359993f);
    pathB.quadTo(883, 25.0710678f, 883, 23);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void skpbingoentertainment_net189(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(896, 745.38678f);
    path.lineTo(896, 873.38678f);
    path.lineTo(922.567993f, 876.683716f);
    path.lineTo(922.567993f, 748.683716f);
    path.lineTo(896, 745.38678f);
    path.close();
    SkPath pathB;
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(899.200928f, 745.783997f);
    pathB.cubicTo(897.119385f, 745.525696f, 895.432007f, 752.031982f, 895.432007f, 760.316284f);
    pathB.lineTo(895.432007f, 858.316284f);
    pathB.cubicTo(895.432007f, 866.600586f, 897.119385f, 873.525696f, 899.200928f, 873.783997f);
    pathB.lineTo(918.799133f, 876.216003f);
    pathB.cubicTo(920.880615f, 876.474304f, 922.567993f, 869.968018f, 922.567993f, 861.683716f);
    pathB.lineTo(922.567993f, 763.683716f);
    pathB.cubicTo(922.567993f, 755.399414f, 920.880615f, 748.474304f, 918.799133f, 748.216003f);
    pathB.lineTo(899.200928f, 745.783997f);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void skpcarrefour_ro62(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(1104, 453);
    path.lineTo(399, 453);
    path.lineTo(399, 657);
    path.cubicTo(399, 661.970581f, 403.029449f, 666, 408, 666);
    path.lineTo(1095, 666);
    path.cubicTo(1099.97058f, 666, 1104, 661.970581f, 1104, 657);
    path.lineTo(1104, 453);
    path.close();
    SkPath pathB;
    pathB.setFillType(SkPath::kInverseWinding_FillType);
    pathB.moveTo(400, 453);
    pathB.lineTo(1103, 453);
    pathB.lineTo(1103, 666);
    pathB.lineTo(406, 666);
    pathB.cubicTo(402.686279f, 666, 400, 663.313721f, 400, 660);
    pathB.lineTo(400, 453);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void skpcaffelavazzait_com_ua21(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(883, 23);
    path.lineTo(883, 0);
    path.lineTo(1122.5f, 0);
    path.lineTo(1122.5f, 25.2136822f);
    path.quadTo(1122.14441f, 25.9271851f, 1121.53601f, 26.5359993f);
    path.quadTo(1120.07104f, 28, 1118, 28);
    path.lineTo(888, 28);
    path.quadTo(885.928955f, 28, 884.463989f, 26.5359993f);
    path.quadTo(883, 25.0710678f, 883, 23);
    path.close();
    SkPath pathB;
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(883, 0);
    pathB.lineTo(1123, 0);
    pathB.lineTo(1123, 23);
    pathB.quadTo(1123, 25.0710678f, 1121.53601f, 26.5359993f);
    pathB.quadTo(1120.07104f, 28, 1118, 28);
    pathB.lineTo(888, 28);
    pathB.quadTo(885.928955f, 28, 884.463989f, 26.5359993f);
    pathB.quadTo(883, 25.0710678f, 883, 23);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void skpcamcorder_kz21(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(883, 23);
    path.lineTo(883, 0);
    path.lineTo(1122.5f, 0);
    path.lineTo(1122.5f, 25.2136822f);
    path.quadTo(1122.14441f, 25.9271851f, 1121.53601f, 26.5359993f);
    path.quadTo(1120.07104f, 28, 1118, 28);
    path.lineTo(888, 28);
    path.quadTo(885.928955f, 28, 884.463989f, 26.5359993f);
    path.quadTo(883, 25.0710678f, 883, 23);
    path.close();
    SkPath pathB;
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(883, 0);
    pathB.lineTo(1123, 0);
    pathB.lineTo(1123, 23);
    pathB.quadTo(1123, 25.0710678f, 1121.53601f, 26.5359993f);
    pathB.quadTo(1120.07104f, 28, 1118, 28);
    pathB.lineTo(888, 28);
    pathB.quadTo(885.928955f, 28, 884.463989f, 26.5359993f);
    pathB.quadTo(883, 25.0710678f, 883, 23);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void skpcavablar_net563(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(160.000488f, 918);
    path.cubicTo(159.164749f, 917.37207f, 158.125824f, 917, 157, 917);
    path.lineTo(94, 917);
    path.cubicTo(92.874176f, 917, 91.8352661f, 917.37207f, 90.9995193f, 918);
    path.lineTo(160.000488f, 918);
    path.close();
    SkPath pathB;
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(91, 917);
    pathB.lineTo(160, 917);
    pathB.lineTo(160, 918);
    pathB.lineTo(91, 918);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void skpinsomnia_gr72(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(1138, 231);
    path.lineTo(1137, 243.625748f);
    path.lineTo(1137, 926);
    path.lineTo(1139, 926);
    path.lineTo(1139, 231);
    path.lineTo(1138, 231);
    path.close();
    SkPath pathB;
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1139, 231);
    pathB.lineTo(1138, 231);
    pathB.lineTo(633, 6101);
    pathB.lineTo(1139, 6607);
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void cubicOp95u(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(0, 2);
    path.cubicTo(2, 3, 5, 1, 3, 2);
    path.close();
    pathB.setFillType(SkPath::kEvenOdd_FillType);
    pathB.moveTo(1, 5);
    pathB.cubicTo(2, 3, 2, 0, 3, 2);
    pathB.close();
    testPathOp(reporter, path, pathB, kUnion_SkPathOp, filename);
}

static void cubicOp96d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(1, 6);
    path.cubicTo(0, 3, 6, 3, 5, 0);
    path.close();
    pathB.setFillType(SkPath::kEvenOdd_FillType);
    pathB.moveTo(3, 6);
    pathB.cubicTo(0, 5, 6, 1, 3, 0);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp97x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(0, 2);
    path.cubicTo(0, 6, 2, 1, 2, 1);
    path.close();
    pathB.setFillType(SkPath::kEvenOdd_FillType);
    pathB.moveTo(1, 2);
    pathB.cubicTo(1, 2, 2, 0, 6, 0);
    pathB.close();
    testPathOp(reporter, path, pathB, kXOR_SkPathOp, filename);
}

static void cubicOp98x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(0, 3);
    path.cubicTo(3, 6, 4, 1, 6, 3);
    path.close();
    pathB.setFillType(SkPath::kEvenOdd_FillType);
    pathB.moveTo(1, 4);
    pathB.cubicTo(3, 6, 3, 0, 6, 3);
    pathB.close();
    testPathOp(reporter, path, pathB, kXOR_SkPathOp, filename);
}

static void cubicOp99(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(3,6);
    path.cubicTo(0,3, 6,5, 5,4);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(5,6);
    pathB.cubicTo(4,5, 6,3, 3,0);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void cubicOp100(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(0,2, 2,1, 4,2);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1,2);
    pathB.cubicTo(2,4, 1,0, 2,0);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp101(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0, 1);
    path.cubicTo(2, 3, 2, 1, 5, 3);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1, 2);
    pathB.cubicTo(3, 5, 1, 0, 3, 2);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void cubicOp102(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(1,2, 1,0, 3,0);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,1);
    pathB.cubicTo(0,3, 1,0, 2,1);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void cubicOp103(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(1,5, 2,0, 2,1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,2);
    pathB.cubicTo(1,2, 1,0, 5,1);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp104(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(0,6, 4,0, 6,1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,4);
    pathB.cubicTo(1,6, 1,0, 6,0);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp105(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(0,4, 6,5, 2,0);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(5,6);
    pathB.cubicTo(0,2, 1,0, 4,0);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp106(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0, 1);
    path.cubicTo(4, 6, 2, 1, 2, 0);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1, 2);
    pathB.cubicTo(0, 2, 1, 0, 6, 4);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp107(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0, 1);
    path.cubicTo(4, 6, 2, 1, 2, 0);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1, 2);
    pathB.cubicTo(0, 2, 1, 0, 6, 4);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void cubicOp108(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0, 1);
    path.cubicTo(4, 6, 2, 1, 2, 0);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1, 2);
    pathB.cubicTo(0, 2, 1, 0, 6, 4);
    pathB.close();
    testPathOp(reporter, path, pathB, kUnion_SkPathOp, filename);
}

static void cubicOp109(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(4,5, 6,3, 5,4);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(3,6);
    pathB.cubicTo(4,5, 1,0, 5,4);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp110(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    path.addRect(0, 0, 4, 4, SkPath::kCW_Direction);
    pathB.setFillType(SkPath::kEvenOdd_FillType);
    pathB.addRect(0, 0, 2, 2, SkPath::kCW_Direction);
    pathB.addRect(0, 0, 2, 2, SkPath::kCW_Direction);
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void cubicOp111(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(1,4);
    path.cubicTo(0,5, 4,1, 3,1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1,4);
    pathB.cubicTo(1,3, 4,1, 5,0);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void xOp1u(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(1, 4);
    path.cubicTo(4, 5, 3, 2, 6, 3);
    path.close();
    pathB.setFillType(SkPath::kEvenOdd_FillType);
    pathB.moveTo(2, 3);
    pathB.cubicTo(3, 6, 4, 1, 5, 4);
    pathB.close();
    testPathOp(reporter, path, pathB, kUnion_SkPathOp, filename);
}

static void xOp1i(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(1, 4);
    path.cubicTo(1, 5, 6, 0, 5, 1);
    path.close();
    pathB.setFillType(SkPath::kEvenOdd_FillType);
    pathB.moveTo(0, 6);
    pathB.cubicTo(1, 5, 4, 1, 5, 1);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void xOp2i(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(1, 5);
    path.cubicTo(0, 4, 3, 2, 6, 1);
    path.close();
    pathB.setFillType(SkPath::kEvenOdd_FillType);
    pathB.moveTo(2, 3);
    pathB.cubicTo(1, 6, 5, 1, 4, 0);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void xOp3i(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(1,4);
    path.cubicTo(0,5, 4,1, 3,1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1,4);
    pathB.cubicTo(1,3, 4,1, 5,0);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void findFirst1(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(1,6, 5,0, 2,1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,5);
    pathB.cubicTo(1,2, 1,0, 6,1);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp112(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(2,4);
    path.cubicTo(2,3, 6,4, 1,0);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(4,6);
    pathB.cubicTo(0,1, 4,2, 3,2);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp113(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.moveTo(2,4);
    path.cubicTo(3,5, 2.33333325f,4.33333349f, 3.83333325f,3.83333349f);
    path.close();
    pathB.moveTo(3,5);
    pathB.cubicTo(2.33333325f,4.33333349f, 3.83333325f,3.83333349f, 2,4);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp114(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0, 1);
    path.cubicTo(1, 3, -1, 2, 3.5f, 1.33333337f);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1, 3);
    pathB.cubicTo(-1, 2, 3.5f, 1.33333337f, 0, 1);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void cubicOp114asQuad(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0, 1);
    path.cubicTo(1, 3, -1, 2, 3.5f, 1.33333337f);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1, 3);
    pathB.cubicTo(-1, 2, 3.5f, 1.33333337f, 0, 1);
    pathB.close();
    SkPath qPath, qPathB;
    CubicPathToQuads(path, &qPath);
    CubicPathToQuads(pathB, &qPathB);
    testPathOp(reporter, qPath, qPathB, kIntersect_SkPathOp, filename);
}

static void quadOp10i(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.moveTo(0, 0);
    path.quadTo(1, 8, 3, 5);
    path.lineTo(8, 1);
    path.close();
    pathB.moveTo(0, 0);
    pathB.quadTo(8, 1, 4, 8);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void kari1(skiatest::Reporter* reporter, const char* filename) {
    SkPath path1;
    path1.moveTo(39.9375, -5.8359375);
    path1.lineTo(40.625, -5.7890625);
    path1.lineTo(37.7109375, 1.3515625);
    path1.lineTo(37.203125, 0.9609375);
    path1.close();

    SkPath path2;
    path2.moveTo(37.52734375f, -1.44140625f);
    path2.cubicTo(37.8736991882324f, -1.69921875f, 38.1640625f, -2.140625f, 38.3984375f, -2.765625f);
    path2.lineTo(38.640625f, -2.609375f);
    path2.cubicTo(38.53125f, -1.89583337306976f, 38.0664443969727f, -0.154893040657043f, 38.0664443969727f, -0.154893040657043f);
    path2.cubicTo(38.0664443969727f, -0.154893040657043f, 37.1809883117676f, -1.18359375f, 37.52734375, -1.44140625f);
    path2.close();

    testPathOp(reporter, path1, path2, kDifference_SkPathOp, filename);
}

static void issue2504(skiatest::Reporter* reporter, const char* filename) {
    SkPath path1;
    path1.moveTo(34.2421875, -5.976562976837158203125);
    path1.lineTo(35.453121185302734375, 0);
    path1.lineTo(31.9375, 0);
    path1.close();

    SkPath path2;
    path2.moveTo(36.71843719482421875, 0.8886508941650390625);
    path2.cubicTo(36.71843719482421875, 0.8886508941650390625,
                  35.123386383056640625, 0.554015457630157470703125,
                  34.511409759521484375, -0.1152553558349609375);
    path2.cubicTo(33.899425506591796875, -0.7845261096954345703125,
                  34.53484344482421875, -5.6777553558349609375,
                  34.53484344482421875, -5.6777553558349609375);
    path2.close();
    testPathOp(reporter, path1, path2, kUnion_SkPathOp, filename);
}

static void issue2540(skiatest::Reporter* reporter, const char* filename) {
    SkPath path1;
    path1.moveTo(26.5054988861083984375, 85.73960113525390625);
    path1.cubicTo(84.19739532470703125, 17.77140045166015625, 16.93920135498046875, 101.86199951171875, 12.631000518798828125, 105.24700164794921875);
    path1.cubicTo(11.0819997787475585937500000, 106.46399688720703125, 11.5260000228881835937500000, 104.464996337890625, 11.5260000228881835937500000, 104.464996337890625);
    path1.lineTo(23.1654987335205078125, 89.72879791259765625);
    path1.cubicTo(23.1654987335205078125, 89.72879791259765625, -10.1713008880615234375, 119.9160003662109375, -17.1620006561279296875, 120.8249969482421875);
    path1.cubicTo(-19.1149997711181640625, 121.07900238037109375, -18.0380001068115234375, 119.79299163818359375, -18.0380001068115234375, 119.79299163818359375);
    path1.cubicTo(-18.0380001068115234375, 119.79299163818359375, 14.22100067138671875, 90.60700225830078125, 26.5054988861083984375, 85.73960113525390625);
    path1.close();

    SkPath path2;
    path2.moveTo(-25.077999114990234375, 124.9120025634765625);
    path2.cubicTo(-25.077999114990234375, 124.9120025634765625, -25.9509983062744140625, 125.95400238037109375, -24.368999481201171875, 125.7480010986328125);
    path2.cubicTo(-16.06999969482421875, 124.66899871826171875, 1.2680000066757202148437500, 91.23999786376953125, 37.264003753662109375, 95.35400390625);
    path2.cubicTo(37.264003753662109375, 95.35400390625, 11.3710002899169921875, 83.7339935302734375, -25.077999114990234375, 124.9120025634765625);
    path2.close();
    testPathOp(reporter, path1, path2, kUnion_SkPathOp, filename);
}

static void rects1(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(1, 1);
    path.lineTo(0, 1);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(6, 0);
    path.lineTo(6, 6);
    path.lineTo(0, 6);
    path.close();
    pathB.setFillType(SkPath::kEvenOdd_FillType);
    pathB.moveTo(0, 0);
    pathB.lineTo(1, 0);
    pathB.lineTo(1, 1);
    pathB.lineTo(0, 1);
    pathB.close();
    pathB.moveTo(0, 0);
    pathB.lineTo(2, 0);
    pathB.lineTo(2, 2);
    pathB.lineTo(0, 2);
    pathB.close();
    testPathOp(reporter, path, pathB, kUnion_SkPathOp, filename);
}

static void rects2(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(0, 0);
    path.lineTo(4, 0);
    path.lineTo(4, 4);
    path.lineTo(0, 4);
    path.close();
    path.moveTo(3, 3);
    path.lineTo(4, 3);
    path.lineTo(4, 4);
    path.lineTo(3, 4);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(3, 3);
    pathB.lineTo(6, 3);
    pathB.lineTo(6, 6);
    pathB.lineTo(3, 6);
    pathB.close();
    pathB.moveTo(3, 3);
    pathB.lineTo(4, 3);
    pathB.lineTo(4, 4);
    pathB.lineTo(3, 4);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void rects3(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    path.addRect(0, 0, 4, 4, SkPath::kCW_Direction);
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.addRect(0, 0, 2, 2, SkPath::kCW_Direction);
    pathB.addRect(0, 0, 2, 2, SkPath::kCW_Direction);
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void rects4(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    path.addRect(0, 0, 2, 2, SkPath::kCW_Direction);
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.addRect(0, 0, 2, 2, SkPath::kCW_Direction);
    pathB.addRect(0, 0, 3, 3, SkPath::kCW_Direction);
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void issue2753(skiatest::Reporter* reporter, const char* filename) {
    SkPath path1;
    path1.moveTo(142.701f, 110.568f);
    path1.lineTo(142.957f, 100);
    path1.lineTo(153.835f, 100);
    path1.lineTo(154.592f, 108.188f);
    path1.cubicTo(154.592f, 108.188f, 153.173f, 108.483f, 152.83f, 109.412f);
    path1.cubicTo(152.83f, 109.412f, 142.701f, 110.568f, 142.701f, 110.568f);
    path1.close();

    SkPath path2;
    path2.moveTo(39, 124.001f);
    path2.cubicTo(39, 124.001f, 50.6f, 117.001f, 50.6f, 117.001f);
    path2.cubicTo(50.6f, 117.001f, 164.601f, 85.2f, 188.201f, 117.601f);
    path2.cubicTo(188.201f, 117.601f, 174.801f, 93, 39, 124.001f);
    path2.close();

    testPathOp(reporter, path1, path2, kUnion_SkPathOp, filename);
}

static void issue2808(skiatest::Reporter* reporter, const char* filename) {
    SkPath path1, path2;

    path1.moveTo(509.20300293f, 385.601989746f);
    path1.quadTo(509.20300293f, 415.68838501f, 487.928710938f, 436.96270752f);
    path1.quadTo(466.654388428f, 458.236999512f, 436.567993164f, 458.236999512f);
    path1.quadTo(406.4815979f, 458.236999512f, 385.207275391f, 436.96270752f);
    path1.quadTo(363.932983398f, 415.68838501f, 363.932983398f, 385.601989746f);
    path1.quadTo(363.932983398f, 355.515594482f, 385.207275391f, 334.241271973f);
    path1.quadTo(406.4815979f, 312.96697998f, 436.567993164f, 312.96697998f);
    path1.quadTo(466.654388428f, 312.96697998f, 487.928710938f, 334.241271973f);
    path1.quadTo(509.20300293f, 355.515594482f, 509.20300293f, 385.601989746f);
    path1.close();

    path2.moveTo(449.033996582f, 290.87298584f);
    path2.quadTo(449.033996582f, 301.028259277f, 441.853149414f, 308.209106445f);
    path2.quadTo(434.672271729f, 315.389984131f, 424.516998291f, 315.389984131f);
    path2.quadTo(414.361724854f, 315.389984131f, 407.180847168f, 308.209106445f);
    path2.quadTo(400, 301.028259277f, 400, 290.87298584f);
    path2.quadTo(400, 280.717712402f, 407.180847168f, 273.536865234f);
    path2.quadTo(414.361724854f, 266.355987549f, 424.516998291f, 266.355987549f);
    path2.quadTo(434.672271729f, 266.355987549f, 441.853149414f, 273.536865234f);
    path2.quadTo(449.033996582f, 280.717712402f, 449.033996582f, 290.87298584f);
    path2.close();

    testPathOp(reporter, path1, path2, kUnion_SkPathOp, filename);
}

static void cubicOp115(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(3,4, 2,1, 5,3);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1,2);
    pathB.cubicTo(3,5, 1,0, 4,3);
    pathB.close();
    SkPath path2(path);
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void testRect1(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, path2;
    path.addRect(0, 0, 60, 60, SkPath::kCCW_Direction);
    path.addRect(30, 20, 50, 50, SkPath::kCCW_Direction);
    path.addRect(24, 20, 36, 30, SkPath::kCCW_Direction);
//    path.addRect(32, 24, 36, 41, SkPath::kCCW_Direction);
    testPathOp(reporter, path, path2, kUnion_SkPathOp, filename);
}

static void testRect2(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    path.addRect(4, 4, 5, 5, SkPath::kCW_Direction);
    pathB.setFillType(SkPath::kEvenOdd_FillType);
    pathB.addRect(0, 0, 2, 2, SkPath::kCW_Direction);
    pathB.addRect(0, 0, 6, 6, SkPath::kCW_Direction);
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp116(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(4,6, 2,0, 2,0);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,2);
    pathB.cubicTo(0,2, 1,0, 6,4);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp117(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(4,5, 6,0, 1,0);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,6);
    pathB.cubicTo(0,1, 1,0, 5,4);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp118(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(4,6, 5,1, 6,2);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1,5);
    pathB.cubicTo(2,6, 1,0, 6,4);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void loop1(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.moveTo(0,1);
    path.cubicTo(1,5, -5.66666651f,3.33333349f, 8.83333302f,2.33333349f);
    path.close();
    pathB.moveTo(1,5);
    pathB.cubicTo(-5.66666651f,3.33333349f, 8.83333302f,2.33333349f, 0,1);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

#include "src/pathops/SkPathOpsCubic.h"

static void loop1asQuad(skiatest::Reporter* reporter, const char* filename) {
    CubicPts cubic1 = {{{0,1}, {1,5}, {-5.66666651f,3.33333349f}, {8.83333302f,2.33333349f}}};
    CubicPts cubic2 = {{{1,5}, {-5.66666651f,3.33333349f}, {8.83333302f,2.33333349f}, {0,1}}};
    SkDCubic c1, c2;
    c1.debugSet(cubic1.fPts);
    c2.debugSet(cubic2.fPts);
    double c1InflectionTs[2], c2InflectionTs[2];
    SkDEBUGCODE(int c1InfTCount =) c1.findInflections(c1InflectionTs);
    SkASSERT(c1InfTCount == 2);
    SkDEBUGCODE(int c2InfTCount =) c2.findInflections(c2InflectionTs);
    SkASSERT(c2InfTCount == 1);
    SkASSERT(c1InflectionTs[0] > c1InflectionTs[1]);
    SkDCubicPair c1pair = c1.chopAt(c1InflectionTs[0]);
    SkDCubicPair c1apair = c1pair.first().chopAt(c1InflectionTs[1]);
    SkDCubicPair c2pair = c2.chopAt(c2InflectionTs[0]);
    SkDQuad q1[2] = { c1pair.first().toQuad(), c1pair.second().toQuad() };
    SkDQuad q1a[2] = { c1apair.first().toQuad(), c1apair.second().toQuad() };
    SkDQuad q2[2] = { c2pair.first().toQuad(), c2pair.second().toQuad() };
    SkPath path, pathB;
    path.moveTo(q1a[0].fPts[0].asSkPoint());
    path.quadTo(q1a[0].fPts[1].asSkPoint(), q1a[0].fPts[2].asSkPoint());
    path.quadTo(q1a[1].fPts[1].asSkPoint(), q1a[1].fPts[2].asSkPoint());
    path.quadTo(q1[1].fPts[1].asSkPoint(), q1[1].fPts[2].asSkPoint());
    path.close();
    pathB.moveTo(q2[0].fPts[0].asSkPoint());
    pathB.quadTo(q2[0].fPts[1].asSkPoint(), q2[0].fPts[2].asSkPoint());
    pathB.quadTo(q2[1].fPts[1].asSkPoint(), q2[1].fPts[2].asSkPoint());
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void loop2(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.moveTo(0,1);
    path.cubicTo(3,4, 3.f,4.f, 4.5f,1.5f);
    path.close();
    pathB.moveTo(3,4);
    pathB.cubicTo(3.f,4.f, 4.5f,1.5f, 0,1);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void loop3(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.moveTo(0,1);
    path.cubicTo(3,5, -3.66666651f,0, 10.5f,-1.66666651f);
    path.close();
    pathB.moveTo(3,5);
    pathB.cubicTo(-3.66666651f,0, 10.5f,-1.66666651f, 0,1);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void loop4(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.moveTo(0,5);
    path.cubicTo(1,5, 1,4, 0.833333313f,3);
    path.close();
    pathB.moveTo(1,5);
    pathB.cubicTo(1,4, 0.833333313f,3, 0,5);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

#include "include/utils/SkParsePath.h"

static void issue3517(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;

    const char str[] = "M31.35 57.75L31.35 57.75C31.9 57.7486 32.45 57.7948 33 57.7413C33.55 57.6878 34.1 57.5014 34.65 57.4291C35.2 57.3569 35.75 57.3223 36.3 57.3079C36.85 57.2935 37.4 57.3143 37.95 57.3428C38.5 57.3712 39.05 57.4112 39.6 57.4786C40.15 57.546 40.7 57.7029 41.25 57.7472C41.8 57.7916 42.35 57.7962 42.9 57.7445C43.45 57.6928 44 57.5345 44.55 57.4373C45.1 57.34 45.65 57.2115 46.2 57.1611C46.75 57.1107 47.3 57.1371 47.85 57.1349C48.4 57.1327 48.95 57.144 49.5 57.1478C50.05 57.1516 50.6 57.1553 51.15 57.1579C51.7 57.1605 52.25 57.1601 52.8 57.1634C53.35 57.1667 53.9 57.1731 54.45 57.1776C55 57.182 55.55 57.1916 56.1 57.19C56.65 57.1884 57.2 57.178 57.75 57.168C58.3 57.158 58.85 57.1355 59.4 57.1299C59.95 57.1243 60.5 57.1338 61.05 57.1345C61.6 57.1352 62.15 57.124 62.7 57.134C63.25 57.1441 63.8 57.1731 64.35 57.195C64.9 57.2169 65.45 57.2532 66 57.2655C66.55 57.2778 67.1 57.2647 67.65 57.2687C68.2 57.2728 68.75 57.267 69.3 57.2896C69.85 57.3122 70.4 57.371 70.95 57.4044C71.5 57.4377 72.05 57.4668 72.6 57.4896C73.15 57.5123 73.7 57.545 74.25 57.5408C74.8 57.5365 75.35 57.5068 75.9 57.4641C76.45 57.4213 77 57.3244 77.55 57.2842C78.1 57.244 78.65 57.2163 79.2 57.2228C79.75 57.2293 80.3 57.29 80.85 57.3232C81.4 57.3563 81.95 57.396 82.5 57.4219C83.05 57.4478 83.6 57.4637 84.15 57.4787C84.7 57.4937 85.25 57.5011 85.8 57.5121C86.35 57.523 86.9 57.5411 87.45 57.5444C88 57.5477 88.55 57.5663 89.1 57.5318C89.65 57.4972 90.2 57.3126 90.75 57.337C91.3 57.3613 91.85 57.6088 92.4 57.6776C92.95 57.7465 93.5 57.7379 94.05 57.75C94.6 57.7621 95.15 57.75 95.7 57.75L95.7 57.75L31.35 57.75Z";
    SkParsePath::FromSVGString(str, &path);

    const char strB[] = "M31.35 57.75L31.35 57.75C31.9 57.7514 32.45 57.7052 33 57.7587C33.55 57.8122 34.1 57.9986 34.65 58.0709C35.2 58.1431 35.75 58.1777 36.3 58.1921C36.85 58.2065 37.4 58.1857 37.95 58.1572C38.5 58.1288 39.05 58.0888 39.6 58.0214C40.15 57.954 40.7 57.7971 41.25 57.7528C41.8 57.7084 42.35 57.7038 42.9 57.7555C43.45 57.8072 44 57.9655 44.55 58.0627C45.1 58.16 45.65 58.2885 46.2 58.3389C46.75 58.3893 47.3 58.3629 47.85 58.3651C48.4 58.3673 48.95 58.356 49.5 58.3522C50.05 58.3484 50.6 58.3447 51.15 58.3421C51.7 58.3395 52.25 58.3399 52.8 58.3366C53.35 58.3333 53.9 58.3269 54.45 58.3224C55 58.318 55.55 58.3084 56.1 58.31C56.65 58.3116 57.2 58.322 57.75 58.332C58.3 58.342 58.85 58.3645 59.4 58.3701C59.95 58.3757 60.5 58.3662 61.05 58.3655C61.6 58.3648 62.15 58.376 62.7 58.366C63.25 58.3559 63.8 58.3269 64.35 58.305C64.9 58.2831 65.45 58.2468 66 58.2345C66.55 58.2222 67.1 58.2353 67.65 58.2313C68.2 58.2272 68.75 58.233 69.3 58.2104C69.85 58.1878 70.4 58.129 70.95 58.0956C71.5 58.0623 72.05 58.0332 72.6 58.0104C73.15 57.9877 73.7 57.955 74.25 57.9592C74.8 57.9635 75.35 57.9932 75.9 58.0359C76.45 58.0787 77 58.1756 77.55 58.2158C78.1 58.256 78.65 58.2837 79.2 58.2772C79.75 58.2707 80.3 58.21 80.85 58.1768C81.4 58.1437 81.95 58.104 82.5 58.0781C83.05 58.0522 83.6 58.0363 84.15 58.0213C84.7 58.0063 85.25 57.9989 85.8 57.9879C86.35 57.977 86.9 57.9589 87.45 57.9556C88 57.9523 88.55 57.9337 89.1 57.9682C89.65 58.0028 90.2 58.1874 90.75 58.163C91.3 58.1387 91.85 57.8912 92.4 57.8224C92.95 57.7535 93.5 57.7621 94.05 57.75C94.6 57.7379 95.15 57.75 95.7 57.75L95.7 57.75L31.35 57.75Z";
    SkParsePath::FromSVGString(strB, &pathB);
    testPathOp(reporter, path, pathB, kUnion_SkPathOp, filename);
}

static void cubicOp119(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(3,5, 2,1, 3,1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1,2);
    pathB.cubicTo(1,3, 1,0, 5,3);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void cubicOp120(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(2,4, 2,1, 4,0);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1,2);
    pathB.cubicTo(0,4, 1,0, 4,2);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp121(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(3,4, 3,2, 4,3);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(2,3);
    pathB.cubicTo(3,4, 1,0, 4,3);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

// FIXME : haven't debugged this failure yet
static void cubicOp122(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(3,5, 4,1, 4,0);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1,4);
    pathB.cubicTo(0,4, 1,0, 5,3);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp123(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(1,5, 2,0, 6,0);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,2);
    pathB.cubicTo(0,6, 1,0, 5,1);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void loop5(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.moveTo(0,2);
    path.cubicTo(1,2, 1,1.66666663f, 0.833333313f,1.33333325f);
    path.close();
    pathB.moveTo(1,2);
    pathB.cubicTo(1,1.66666663f, 0.833333313f,1.33333325f, 0,2);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void loop6(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.moveTo(0,1);
    path.cubicTo(1,3, -1.66666675f,1.66666663f, 4.16666651f,1.00000012f);
    path.close();
    pathB.moveTo(1,3);
    pathB.cubicTo(-1.66666675f,1.66666663f, 4.16666651f,1.00000012f, 0,1);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void cubicOp124(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(1,5, 6,0, 3,0);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,6);
    pathB.cubicTo(0,3, 1,0, 5,1);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp125(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(3,6, 3,1, 6,2);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1,3);
    pathB.cubicTo(2,6, 1,0, 6,3);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp126(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(0,3, 6,0, 2,1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,6);
    pathB.cubicTo(1,2, 1,0, 3,0);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp127(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(1,5, 6,0, 3,0);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,6);
    pathB.cubicTo(0,3, 1,0, 5,1);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void cubicOp128(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(0,3, 3,2, 5,2);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(2,3);
    pathB.cubicTo(2,5, 1,0, 3,0);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp129(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(5,6);
    path.cubicTo(3,4, 2,0, 2,1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,2);
    pathB.cubicTo(1,2, 6,5, 4,3);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp130(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(5,6);
    path.cubicTo(4,6, 3,0, 2,1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,3);
    pathB.cubicTo(1,2, 6,5, 6,4);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

#include "src/core/SkGeometry.h"

static void complex_to_quads(const SkPoint pts[], SkPath* path) {
    SkScalar loopT[3];
    if (SkDCubic::ComplexBreak(pts, loopT)) {
        SkPoint cubicPair[7];
        SkChopCubicAt(pts, cubicPair, loopT[0]);
        SkDCubic c1, c2;
        c1.set(cubicPair);
        c2.set(&cubicPair[3]);
        SkDQuad q1 = c1.toQuad();
        SkDQuad q2 = c2.toQuad();
        path->quadTo(q1[1].asSkPoint(), q1[2].asSkPoint());
        path->quadTo(q2[1].asSkPoint(), q2[2].asSkPoint());
    } else {
        path->cubicTo(pts[1], pts[2], pts[3]);
    }
}

static void cubicOp130a(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(5,6);
    SkPoint pts[] = { {5,6}, {4,6}, {3,0}, {2,1} };
    complex_to_quads(pts, &path);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,3);
    SkPoint pts2[] = { {0,3}, {1,2}, {6,5}, {6,4} };
    complex_to_quads(pts2, &path);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp131(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(3,4, 3,0, 6,2);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,3);
    pathB.cubicTo(2,6, 1,0, 4,3);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void circlesOp1(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.addCircle(0, 1, 2, SkPath::kCCW_Direction);
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.addCircle(0, 1, 1, SkPath::kCW_Direction);
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void circlesOp2(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.addCircle(0, 1, 4, SkPath::kCCW_Direction);
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.addCircle(0, 4, 3, SkPath::kCW_Direction);
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void rRect1x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(20.65f, 5.65f);
    path.conicTo(20.65f, 1.13612f, 25.1404f, 0.65f, 0.888488f);
    path.lineTo(25.65f, 0.65f);
    path.lineTo(26.1596f, 0.67604f);
    path.conicTo(30.65f, 1.13612f, 30.65f, 5.65f, 0.888488f);
    path.lineTo(30.65f, 25.65f);
    path.conicTo(30.65f, 20.65f, 25.65f, 20.65f, 0.707107f);
    path.lineTo(20.65f, 20.65f);
    path.lineTo(20.65f, 5.65f);
    path.close();
    path.moveTo(20.65f, 20.65f);
    path.lineTo(5.65f, 20.65f);
    path.conicTo(0.65f, 20.65f, 0.65f, 25.65f, 0.707107f);
    path.lineTo(0.65f, 45.65f);
    path.conicTo(0.65f, 50.65f, 5.65f, 50.65f, 0.707107f);
    path.lineTo(25.65f, 50.65f);
    path.conicTo(30.65f, 50.65f, 30.65f, 45.65f, 0.707107f);
    path.lineTo(30.65f, 25.65f);
    path.conicTo(30.65f, 30.65f, 25.65f, 30.65f, 0.707107f);
    path.conicTo(20.65f, 30.65f, 20.65f, 25.65f, 0.707107f);
    path.lineTo(20.65f, 20.65f);
    path.close();
    SkPath path1(path);

    path.reset();
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(20.65f, 45.65f);
    path.lineTo(20.65f, 25.65f);
    path.conicTo(20.65f, 20.65f, 25.65f, 20.65f, 0.707107f);
    path.lineTo(45.65f, 20.65f);
    path.conicTo(50.65f, 20.65f, 50.65f, 25.65f, 0.707107f);
    path.lineTo(50.65f, 45.65f);
    path.conicTo(50.65f, 50.65f, 45.65f, 50.65f, 0.707107f);
    path.lineTo(25.65f, 50.65f);
    path.conicTo(20.65f, 50.65f, 20.65f, 45.65f, 0.707107f);
    path.close();
    SkPath path2(path);

    testPathOp(reporter, path1, path2, kDifference_SkPathOp, filename);
}

static void loop7(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.moveTo(0,1);
    path.cubicTo(3,4, -1,0, 8.5f,-2.5f);
    path.close();
    pathB.moveTo(3,4);
    pathB.cubicTo(-1,0, 8.5f,-2.5f, 0,1);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void rects5(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.addRect(5, 5, 6, 6, SkPath::kCW_Direction);
    path.addRect(5, 5, 6, 6, SkPath::kCW_Direction);
    pathB.setFillType(SkPath::kEvenOdd_FillType);
    pathB.addRect(0, 0, 6, 6, SkPath::kCW_Direction);
    pathB.addRect(5, 5, 6, 6, SkPath::kCW_Direction);
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void loop8(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.moveTo(0,1);
    path.cubicTo(1,4, -3.83333325f,0.166666627f, 6,-1);
    path.close();
    pathB.moveTo(1,4);
    pathB.cubicTo(-3.83333325f,0.166666627f, 6,-1, 0,1);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void loop9(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.moveTo(0,1);
    path.cubicTo(1,3, -2.5f,0, 3.33333325f,-0.666666627f);
    path.close();
    pathB.moveTo(1,3);
    pathB.cubicTo(-2.5f,0, 3.33333325f,-0.666666627f, 0,1);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void circlesOp3(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.addCircle(0, 1, 2, SkPath::kCCW_Direction);
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.addCircle(3, 5, 3, SkPath::kCW_Direction);
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void loop10(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.moveTo(5,6);
    path.cubicTo(1,2, 1,2, -3.66666651f,13.333334f);
    path.close();
    pathB.moveTo(1,2);
    pathB.cubicTo(1,2, -3.66666651f,13.333334f, 5,6);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void loop11(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.moveTo(0,1);
    path.cubicTo(1,3, -1.83333349f,1.33333337f, 4,-1);
    path.close();
    pathB.moveTo(1,3);
    pathB.cubicTo(-1.83333349f,1.33333337f, 4,-1, 0,1);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void cubicOp132(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(5,6);
    path.cubicTo(3,4, 3,0, 3,2);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,3);
    pathB.cubicTo(2,3, 6,5, 4,3);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void loop12(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.moveTo(1,2);
    path.cubicTo(0,6, -3.16666675f,3.66666675f, 6.33333349f,3.33333349f);
    path.close();
    pathB.moveTo(0,6);
    pathB.cubicTo(-3.16666675f,3.66666675f, 6.33333349f,3.33333349f, 1,2);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void cubicOp133(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(5,6);
    path.cubicTo(5,6, 5,0, 4,1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,5);
    pathB.cubicTo(1,4, 6,5, 6,5);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp134(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(5,6);
    path.cubicTo(5,6, 6,0, 3,1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,6);
    pathB.cubicTo(1,3, 6,5, 6,5);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp135(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(5,6);
    path.cubicTo(5,6, 6,0, 4,1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,6);
    pathB.cubicTo(1,4, 6,5, 6,5);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp136(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(5,6);
    path.cubicTo(5,6, 5,0, 3,1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,5);
    pathB.cubicTo(1,3, 6,5, 6,5);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp136a(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(5,6);
    path.quadTo(5,0, 3,1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,5);
    pathB.cubicTo(1,3, 6,5, 6,5);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubics137(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0, 5);
    path.cubicTo(3, 6, 1, 0, 3, 2);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0, 1);
    pathB.cubicTo(2, 3, 5, 0, 6, 3);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubics138(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0, 5);
    path.cubicTo(3, 6, 1, 0, 4, 2);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0, 1);
    pathB.cubicTo(2, 4, 5, 0, 6, 3);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

// three curves intersect successfully nearby -- the angle only gets 2 of the 3 pts
static void cubicOp139(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,2);
    path.cubicTo(0,4, 3,1, 5,1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1,3);
    pathB.cubicTo(1,5, 2,0, 4,0);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp140(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,2);
    path.cubicTo(1,2, 5,4, 3,2);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(4,5);
    pathB.cubicTo(2,3, 2,0, 2,1);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp141(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,2);
    path.cubicTo(1,2, 6,4, 3,2);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(4,6);
    pathB.cubicTo(2,3, 2,0, 2,1);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void quadRect1(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.moveTo(6,15);
    path.quadTo(16,0, 8,4);
    path.quadTo(2,7, 12,12);
    path.close();
    pathB.addRect(4,11, 13,16);
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void quadRect2(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.moveTo(5,12);
    path.quadTo(15,7, 9,4);
    path.quadTo(1,0, 11,15);
    path.close();
    pathB.addRect(4,11, 13,16);
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void quadRect3(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.moveTo(12,12);
    path.quadTo(2,7, 8,4);
    path.quadTo(16,0, 6,15);
    path.close();
    pathB.addRect(4,11, 13,16);
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void quadRect4(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.moveTo(11,15);
    path.quadTo(1,0, 9,4);
    path.quadTo(15,7, 5,12);
    path.close();
    pathB.addRect(4,11, 13,16);
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void quadRect5(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.moveTo(11,13);
    path.quadTo(4,4, 8,4);
    path.quadTo(12,4, 5,13);
    path.close();
    pathB.addRect(4,11, 13,16);
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void quadRect6(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.moveTo(5,13);
    path.quadTo(12,4, 8,4);
    path.quadTo(4,4, 11,13);
    path.close();
    pathB.addRect(4,11, 13,16);
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void loops4i(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0, 3);
    path.cubicTo(0, 2, 0, 2, -1.66666663f, 2.16666675f);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0, 2);
    pathB.cubicTo(0, 2, -1.66666663f, 2.16666675f, 0, 3);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void loops5i(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(1, 2);
    path.cubicTo(0, 2, 0, 2, 0.166666672f, 2.66666675f);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0, 2);
    pathB.cubicTo(0, 2, 0.166666672f, 2.66666675f, 1, 2);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void cubicOp142(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(5,6);
    path.cubicTo(2,5, 2,1, 1,0);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1,2);
    pathB.cubicTo(0,1, 6,5, 5,2);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubics6d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(3, 5);
    path.cubicTo(1, 5, 4, 2, 4, 0);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(2, 4);
    pathB.cubicTo(0, 4, 5, 3, 5, 1);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubics7d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(2, 6);
    path.cubicTo(2, 4, 5, 1, 3, 1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1, 5);
    pathB.cubicTo(1, 3, 6, 2, 4, 2);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubics8d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(2, 5);
    path.cubicTo(2, 4, 5, 1, 3, 2);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1, 5);
    pathB.cubicTo(2, 3, 5, 2, 4, 2);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubics9d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(2, 4);
    path.cubicTo(2, 6, 3, 1, 5, 1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1, 3);
    pathB.cubicTo(1, 5, 4, 2, 6, 2);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubics10u(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(2, 4);
    path.cubicTo(1, 6, 4, 1, 5, 1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1, 4);
    pathB.cubicTo(1, 5, 4, 2, 6, 1);
    pathB.close();
    testPathOp(reporter, path, pathB, kUnion_SkPathOp, filename);
}

static void cubics11i(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(2, 4);
    path.cubicTo(2, 5, 3, 2, 5, 1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(2, 3);
    pathB.cubicTo(1, 5, 4, 2, 5, 2);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void cubics12d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(2, 4);
    path.cubicTo(0, 4, 5, 3, 5, 1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(3, 5);
    pathB.cubicTo(1, 5, 4, 2, 4, 0);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubics13d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(2, 3);
    path.cubicTo(1, 5, 4, 2, 5, 2);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(2, 4);
    pathB.cubicTo(2, 5, 3, 2, 5, 1);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubics14d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(2, 3);
    path.cubicTo(0, 4, 3, 1, 3, 0);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1, 3);
    pathB.cubicTo(0, 3, 3, 2, 4, 0);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubics15d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(1, 5);
    path.cubicTo(3, 5, 4, 0, 4, 2);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0, 4);
    pathB.cubicTo(2, 4, 5, 1, 5, 3);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubics16i(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(1, 5);
    path.cubicTo(2, 5, 5, 0, 4, 2);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0, 5);
    pathB.cubicTo(2, 4, 5, 1, 5, 2);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void cubics17d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(1, 5);
    path.cubicTo(3, 4, 4, 1, 4, 2);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1, 4);
    pathB.cubicTo(2, 4, 5, 1, 4, 3);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubics18d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(1, 5);
    path.cubicTo(1, 3, 4, 0, 2, 0);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0, 4);
    pathB.cubicTo(0, 2, 5, 1, 3, 1);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubics19d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(1, 5);
    path.cubicTo(2, 3, 5, 2, 4, 2);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(2, 5);
    pathB.cubicTo(2, 4, 5, 1, 3, 2);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubicOp157(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(1,5);
    path.cubicTo(1,3, 6,2, 4,2);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(2,6);
    pathB.cubicTo(2,4, 5,1, 3,1);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubics20d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(1, 2);
    path.cubicTo(0, 3, 6, 0, 3, 2);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0, 6);
    pathB.cubicTo(2, 3, 2, 1, 3, 0);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void loops20i(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(1, 2);
    path.cubicTo(0, 2, 0.833333313f, 2, 1, 3.66666651f);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0, 2);
    pathB.cubicTo(0.833333313f, 2, 1, 3.66666651f, 1, 2);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void loops21i(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(1, 2);
    path.cubicTo(0, 2, 0.833333313f, 2, 1, 4);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0, 2);
    pathB.cubicTo(0.833333313f, 2, 1, 4, 1, 2);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void loops22i(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(1, 3);
    path.cubicTo(0, 3, 0.833333313f, 3, 1, 4.66666651f);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0, 3);
    pathB.cubicTo(0.833333313f, 3, 1, 4.66666651f, 1, 3);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void loops23i(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(1, 5);
    path.cubicTo(0, 1, 6.16666698f, 5.66666698f, -5.66666651f, 6.66666651f);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0, 1);
    pathB.cubicTo(6.16666698f, 5.66666698f, -5.66666651f, 6.66666651f, 1, 5);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void loops24i(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(1, 2);
    path.cubicTo(0, 2, 0.833333313f, 2, 1, 3);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0, 2);
    pathB.cubicTo(0.833333313f, 2, 1, 3, 1, 2);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void loops25i(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(1, 5);
    path.cubicTo(0, 5, 0.833333313f, 5, 1, 7);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0, 5);
    pathB.cubicTo(0.833333313f, 5, 1, 7, 1, 5);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void loops26i(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(1, 6);
    path.cubicTo(0, 2, 6.16666698f, 6.66666698f, -5.66666651f, 7.66666651f);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0, 2);
    pathB.cubicTo(6.16666698f, 6.66666698f, -5.66666651f, 7.66666651f, 1, 6);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void loops27i(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(1, 3);
    path.cubicTo(0, 3, 0.833333313f, 3, 1, 4.33333349f);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0, 3);
    pathB.cubicTo(0.833333313f, 3, 1, 4.33333349f, 1, 3);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void loops28i(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(2, 3);
    path.cubicTo(1, 3, 1.83333337f, 3, 2, 4.66666651f);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1, 3);
    pathB.cubicTo(1.83333337f, 3, 2, 4.66666651f, 2, 3);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void loops29i(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(2, 4);
    path.cubicTo(0, 4, 1.66666663f, 4, 2, 7.33333302f);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0, 4);
    pathB.cubicTo(1.66666663f, 4, 2, 7.33333302f, 2, 4);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void loops30i(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(2, 4);
    path.cubicTo(0, 4, 1.66666663f, 4, 2, 8);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0, 4);
    pathB.cubicTo(1.66666663f, 4, 2, 8, 2, 4);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void loops31i(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(2, 5);
    path.cubicTo(1, 5, 1.83333337f, 5, 2, 6.66666651f);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1, 5);
    pathB.cubicTo(1.83333337f, 5, 2, 6.66666651f, 2, 5);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void loops32i(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(2, 6);
    path.cubicTo(1, 6, 1.83333337f, 6, 2, 8);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1, 6);
    pathB.cubicTo(1.83333337f, 6, 2, 8, 2, 6);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void loops33i(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(2, 6);
    path.cubicTo(1, 2, 7.16666698f, 6.66666698f, -4.66666651f, 7.66666651f);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1, 2);
    pathB.cubicTo(7.16666698f, 6.66666698f, -4.66666651f, 7.66666651f, 2, 6);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void loops33iMod(skiatest::Reporter* reporter, const char* filename) {
    SkPoint pts[] = {{2, 6}, {1, 2}, {7.16666698f, 6.66666698f}, {-4.66666651f, 7.66666651f},
                     {1, 2}, {7.16666698f, 6.66666698f}, {-4.66666651f, 7.66666651f}, {2, 6}};
    bool up = false;
    float offset = 0.0380172729f;
    float step = 7.62939453e-006f;
    bool lastResult = true;
 //   for (int i = 0; i < 30; ++i) {
        SkString name(filename);
 //       name.appendS32(i);
 //       if (i > 0) {
 //           SkDebugf("\n\n<div id=\"%s\">\n", name.c_str());
 //       }
        pts[5].fY = 6.66666698f + offset;
        SkPath path, pathB;
        path.setFillType(SkPath::kWinding_FillType);
        path.moveTo(pts[0]);
        path.cubicTo(pts[1], pts[2], pts[3]);
        path.close();
        pathB.setFillType(SkPath::kWinding_FillType);
        pathB.moveTo(pts[4]);
        pathB.cubicTo(pts[5], pts[6], pts[7]);
        pathB.close();
        bool result = testPathOp(reporter, path, pathB, kIntersect_SkPathOp, name.c_str());
        if (lastResult != result) {
            up = !up;
        }
        step /= 2;
        offset += up ? step : -step;
        lastResult = result;
 //   }
}


static void loops33iAsQuads(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(2, 6);
    path.cubicTo(1, 2, 7.16666698f, 6.66666698f, -4.66666651f, 7.66666651f);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1, 2);
    pathB.cubicTo(7.16666698f, 6.66666698f, -4.66666651f, 7.66666651f, 2, 6);
    pathB.close();
    SkPath qPath, qPathB;
    CubicPathToQuads(path, &qPath);
    CubicPathToQuads(pathB, &qPathB);
    testPathOp(reporter, qPath, qPathB, kIntersect_SkPathOp, filename);
}

static void loops34i(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(3, 4);
    path.cubicTo(0, 4, 2.5f, 4, 3, 9);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0, 4);
    pathB.cubicTo(2.5f, 4, 3, 9, 3, 4);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void loops35i(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(3, 4);
    path.cubicTo(0, 4, 2.5f, 4, 3, 10);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0, 4);
    pathB.cubicTo(2.5f, 4, 3, 10, 3, 4);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void loops36i(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(3, 4);
    path.cubicTo(1, 4, 2.66666675f, 4, 3, 8);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1, 4);
    pathB.cubicTo(2.66666675f, 4, 3, 8, 3, 4);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void loops37i(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(2, 4);
    path.cubicTo(1, 4, 1.83333337f, 4, 2, 5.33333349f);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1, 4);
    pathB.cubicTo(1.83333337f, 4, 2, 5.33333349f, 2, 4);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void loops38i(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(3, 4);
    path.cubicTo(2, 4, 2.83333325f, 4, 3, 6);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(2, 4);
    pathB.cubicTo(2.83333325f, 4, 3, 6, 3, 4);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void loops39i(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(3, 5);
    path.cubicTo(0, 5, 2.5f, 5, 3, 10);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0, 5);
    pathB.cubicTo(2.5f, 5, 3, 10, 3, 5);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void loops40i(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(3, 5);
    path.cubicTo(0, 5, 2.5f, 5, 3, 11);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0, 5);
    pathB.cubicTo(2.5f, 5, 3, 11, 3, 5);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void loops40iAsQuads(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(3, 5);
    path.cubicTo(0, 5, 2.5f, 5, 3, 11);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0, 5);
    pathB.cubicTo(2.5f, 5, 3, 11, 3, 5);
    pathB.close();
    SkPath qPath, qPathB;
    CubicPathToQuads(path, &qPath);
    CubicPathToQuads(pathB, &qPathB);
    testPathOp(reporter, qPath, qPathB, kIntersect_SkPathOp, filename);
}

static void loops44i(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(1, 5);
    path.cubicTo(0, 1, 7.33333302f, 5.33333349f, -7, 7);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0, 1);
    pathB.cubicTo(7.33333302f, 5.33333349f, -7, 7, 1, 5);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void loops45i(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(1, 6);
    path.cubicTo(0, 2, 7.33333302f, 6.33333302f, -7, 8);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0, 2);
    pathB.cubicTo(7.33333302f, 6.33333302f, -7, 8, 1, 6);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void loops46i(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(2, 6);
    path.cubicTo(1, 2, 8.33333302f, 6.33333302f, -6, 8);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1, 2);
    pathB.cubicTo(8.33333302f, 6.33333302f, -6, 8, 2, 6);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

/*
FAILED: d:\cygwin\puregit\tests\pathopsextendedtest.cpp:346    0 */
static void loops47i(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(2, 4);
    path.cubicTo(0, 1, 6, 5.83333302f, -4, 8);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0, 1);
    pathB.cubicTo(6, 5.83333302f, -4, 8, 2, 4);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void loops48i(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(2, 6);
    path.cubicTo(0, 1, 9.33333302f, 6.83333302f, -8.33333302f, 9.16666603f);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0, 1);
    pathB.cubicTo(9.33333302f, 6.83333302f, -8.33333302f, 9.16666603f, 2, 6);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void loops49i(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0, 2);
    path.cubicTo(1, 4, -0.166666687f, 2.66666675f, 1.66666675f, 2);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1, 4);
    pathB.cubicTo(-0.166666687f, 2.66666675f, 1.66666675f, 2, 0, 2);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void loops50i(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0, 3);
    path.cubicTo(1, 5, -0.166666687f, 3.66666675f, 1.66666675f, 3);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1, 5);
    pathB.cubicTo(-0.166666687f, 3.66666675f, 1.66666675f, 3, 0, 3);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void loops51i(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(1, 2);
    path.cubicTo(2, 4, 0.833333313f, 2.66666675f, 2.66666675f, 2);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(2, 4);
    pathB.cubicTo(0.833333313f, 2.66666675f, 2.66666675f, 2, 1, 2);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void loops52i(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(1, 3);
    path.cubicTo(2, 5, 0.833333313f, 3.66666675f, 2.66666675f, 3);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(2, 5);
    pathB.cubicTo(0.833333313f, 3.66666675f, 2.66666675f, 3, 1, 3);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void loops53i(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(2, 3);
    path.cubicTo(3, 5, 1.83333325f, 3.66666675f, 3.66666651f, 3);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(3, 5);
    pathB.cubicTo(1.83333325f, 3.66666675f, 3.66666651f, 3, 2, 3);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void loops54i(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0, 2);
    path.cubicTo(1, 4, 0, 3, 1.66666675f, 2);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1, 4);
    pathB.cubicTo(0, 3, 1.66666675f, 2, 0, 2);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void loops55i(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0, 3);
    path.cubicTo(1, 5, 0, 4, 1.66666675f, 3);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1, 5);
    pathB.cubicTo(0, 4, 1.66666675f, 3, 0, 3);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void loops56i(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(1, 2);
    path.cubicTo(2, 4, 0.99999994f, 3, 2.66666675f, 2);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(2, 4);
    pathB.cubicTo(0.99999994f, 3, 2.66666675f, 2, 1, 2);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void loops57i(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(1, 3);
    path.cubicTo(2, 5, 0.99999994f, 4, 2.66666675f, 3);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(2, 5);
    pathB.cubicTo(0.99999994f, 4, 2.66666675f, 3, 1, 3);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void loops58i(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(2, 3);
    path.cubicTo(3, 5, 2, 4, 3.66666651f, 3);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(3, 5);
    pathB.cubicTo(2, 4, 3.66666651f, 3, 2, 3);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void loops58iAsQuads(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(2, 3);
    path.cubicTo(3, 5, 2, 4, 3.66666651f, 3);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(3, 5);
    pathB.cubicTo(2, 4, 3.66666651f, 3, 2, 3);
    pathB.close();
    SkPath qPath, qPathB;
    CubicPathToQuads(path, &qPath);
    CubicPathToQuads(pathB, &qPathB);
//    SkPoint from = {2.61714339f,1.90228665f};
//    SkPoint to = {2.617045833359139f,1.9013528935803314f};
//    path_edit(from, to, &qPathB);
    testPathOp(reporter, qPath, qPathB, kIntersect_SkPathOp, filename);
}

static void loops59i(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0, 6);
    path.cubicTo(1, 2, 7.33333302f, 1.66666663f, -7.5f, 2);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1, 2);
    pathB.cubicTo(7.33333302f, 1.66666663f, -7.5f, 2, 0, 6);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void loops59iasQuads(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0, 6);
    path.cubicTo(1, 2, 7.33333302f, 1.66666663f, -7.5f, 2);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1, 2);
    pathB.cubicTo(7.33333302f, 1.66666663f, -7.5f, 2, 0, 6);
    pathB.close();
    SkPath qPath, qPathB;
    CubicPathToQuads(path, &qPath);
    CubicPathToQuads(pathB, &qPathB);
    SkPoint from = {2.61714339f,1.90228665f};
    SkPoint to = {2.617045833359139f,1.9013528935803314f};
    path_edit(from, to, &qPathB);
    testPathOp(reporter, qPath, qPathB, kIntersect_SkPathOp, filename);
}

static void cubics41d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0, 1);
    path.cubicTo(1, 4, 3, 0, 3, 1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0, 3);
    pathB.cubicTo(1, 3, 1, 0, 4, 1);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

void loops61i(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0, 1);
    path.cubicTo(1, 5, -6.33333302f, 0.666666627f, 8, -1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1, 5);
    pathB.cubicTo(-6.33333302f, 0.666666627f, 8, -1, 0, 1);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void loops62i(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0, 2);
    path.cubicTo(1, 6, -6.33333302f, 1.66666663f, 8, 0);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1, 6);
    pathB.cubicTo(-6.33333302f, 1.66666663f, 8, 0, 0, 2);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void loops63i(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0, 1);
    path.cubicTo(2, 4, -4, -0.833333254f, 6, -3);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(2, 4);
    pathB.cubicTo(-4, -0.833333254f, 6, -3, 0, 1);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void cubics44d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(3, 4);
    path.cubicTo(2, 5, 3, 1, 6, 2);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1, 3);
    pathB.cubicTo(2, 6, 4, 3, 5, 2);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubics45u(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(1, 3);
    path.cubicTo(2, 6, 4, 3, 5, 2);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(3, 4);
    pathB.cubicTo(2, 5, 3, 1, 6, 2);
    pathB.close();
    testPathOp(reporter, path, pathB, kUnion_SkPathOp, filename);
}

static void fuzz38(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.moveTo(100.34f, 303.312f);
    path.lineTo(-1e+08, 303.312f);
    path.lineTo(102, 310.156f);
    path.lineTo(100.34f, 310.156f);
    path.lineTo(100.34f, 303.312f);
    path.close();
    testPathOpCheck(reporter, path, pathB, kUnion_SkPathOp, filename, true);
}

// we currently don't produce meaningful intersections when a path has extremely large segments
// intersecting relatively small ones. This bug was reported as a fuzzer bug and wasn't expected
// to produce meaningful results
static void crbug_526025(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x43b40000), SkBits2Float(0xcf000000));  // 360, -2.14748e+09f
path.cubicTo(SkBits2Float(0x4e0d628f), SkBits2Float(0xceffffff), SkBits2Float(0x4e800003), SkBits2Float(0xcec6b143), SkBits2Float(0x4e800002), SkBits2Float(0xce7ffffc));  // 5.93012e+08f, -2.14748e+09f, 1.07374e+09f, -1.66675e+09f, 1.07374e+09f, -1.07374e+09f
path.cubicTo(SkBits2Float(0x4e800002), SkBits2Float(0xcde53aee), SkBits2Float(0x4e0d6292), SkBits2Float(0xc307820e), SkBits2Float(0x44627d00), SkBits2Float(0x437ffff2));  // 1.07374e+09f, -4.80731e+08f, 5.93012e+08f, -135.508f, 905.953f, 256
path.lineTo(SkBits2Float(0x444bf3bc), SkBits2Float(0x4460537e));  // 815.808f, 897.305f
path.lineTo(SkBits2Float(0x43553abd), SkBits2Float(0x440f3cbd));  // 213.229f, 572.949f
path.lineTo(SkBits2Float(0x42000000), SkBits2Float(0x41800000));  // 32, 16
path.lineTo(SkBits2Float(0x42c80000), SkBits2Float(0x44000000));  // 100, 512
path.lineTo(SkBits2Float(0x43553abd), SkBits2Float(0x440f3cbd));  // 213.229f, 572.949f
path.lineTo(SkBits2Float(0x43b40000), SkBits2Float(0x44800000));  // 360, 1024
path.lineTo(SkBits2Float(0x43b40000), SkBits2Float(0x45816000));  // 360, 4140

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42fe0000), SkBits2Float(0x43a08000));  // 127, 321
path.lineTo(SkBits2Float(0x45d5c000), SkBits2Float(0x43870000));  // 6840, 270
path.lineTo(SkBits2Float(0xd0a00000), SkBits2Float(0x4cbebc20));  // -2.14748e+10f, 1e+08
path.lineTo(SkBits2Float(0x451f7000), SkBits2Float(0x42800000));  // 2551, 64
path.lineTo(SkBits2Float(0x42fe0000), SkBits2Float(0x43a08000));  // 127, 321
path.close();

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 2, filename);
}

static void fuzzX_392(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
path.moveTo(SkBits2Float(0x41e80000), SkBits2Float(0x43bde212));  // 29, 379.766f
path.lineTo(SkBits2Float(0x41e80000), SkBits2Float(0x43bdc7ef));  // 29, 379.562f
path.conicTo(SkBits2Float(0x42a5861e), SkBits2Float(0x43c61f86), SkBits2Float(0x430b0610), SkBits2Float(0x43c61f86), SkBits2Float(0x3f7d23f3));  // 82.7619f, 396.246f, 139.024f, 396.246f, 0.98883f
path.conicTo(SkBits2Float(0x42a58e20), SkBits2Float(0x43c61f86), SkBits2Float(0x41e80000), SkBits2Float(0x43bde212), SkBits2Float(0x3f7d2cf5));  // 82.7776f, 396.246f, 29, 379.766f, 0.988967f
path.close();

    SkPath path1(path);
    path.setFillType(SkPath::kWinding_FillType);
path.moveTo(SkBits2Float(0xc36c7bd8), SkBits2Float(0xc3a31d72));  // -236.484f, -326.23f
path.lineTo(SkBits2Float(0xc367a4ae), SkBits2Float(0xc3a31d72));  // -231.643f, -326.23f
path.lineTo(SkBits2Float(0x430b0610), SkBits2Float(0x43c61f86));  // 139.024f, 396.246f
path.lineTo(SkBits2Float(0xc36c7bd8), SkBits2Float(0x43c61f86));  // -236.484f, 396.246f

    SkPath path2(path);
    testPathOp(reporter, path1, path2, kIntersect_SkPathOp, filename);
}

static void dean2(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x3f2b74b3), SkBits2Float(0x4154a02b)); // 0.669749f, 13.2891f
path.cubicTo(SkBits2Float(0x3f2b74b3), SkBits2Float(0x4154a02b), SkBits2Float(0x41531912), SkBits2Float(0x3f130322), SkBits2Float(0x4154a02b), SkBits2Float(0x3f2b74b3)); // 0.669749f, 13.2891f, 13.1936f, 0.574267f, 13.2891f, 0.669749f
path.cubicTo(SkBits2Float(0x414a835a), SkBits2Float(0x3ec07ba6), SkBits2Float(0x413fcc0d), SkBits2Float(0x3e193319), SkBits2Float(0x4134a02b), SkBits2Float(0x00000000)); // 12.6571f, 0.375943f, 11.9873f, 0.149609f, 11.2891f, 0
path.lineTo(SkBits2Float(0x3f2b74b3), SkBits2Float(0x4154a02b)); // 0.669749f, 13.2891f
path.close();
    SkPath path1(path);

    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x3f2b74b3), SkBits2Float(0x4154a02b)); // 0.669749f, 13.2891f
path.cubicTo(SkBits2Float(0x3f2b74b3), SkBits2Float(0x4154a02b), SkBits2Float(0x41531912), SkBits2Float(0x3f130322), SkBits2Float(0x4154a02b), SkBits2Float(0x3f2b74b3)); // 0.669749f, 13.2891f, 13.1936f, 0.574267f, 13.2891f, 0.669749f
path.lineTo(SkBits2Float(0x417ab74b), SkBits2Float(0x4154a02b)); // 15.6697f, 13.2891f
path.lineTo(SkBits2Float(0x3f2b74b3), SkBits2Float(0x4154a02b)); // 0.669749f, 13.2891f
path.close();
    SkPath path2(path);
    testPathOp(reporter, path1, path2, kIntersect_SkPathOp, filename);
}

static void cubics_d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0, 1);
    path.cubicTo(3, 5, 1, 0, 3, 0);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0, 1);
    pathB.cubicTo(0, 3, 1, 0, 5, 3);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubics_d2(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0, 1);
    path.cubicTo(2, 5, 2, 0, 2, 1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0, 2);
    pathB.cubicTo(1, 2, 1, 0, 5, 2);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void loops_i1(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(2, 3);
    path.cubicTo(0, 4, -0.333333343f, 4.66666651f, 3, 5.83333349f);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0, 4);
    pathB.cubicTo(-0.333333343f, 4.66666651f, 3, 5.83333349f, 2, 3);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void loops_i2(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(2, 4);
    path.cubicTo(0, 5, -0.333333343f, 5.66666651f, 3, 6.83333302f);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0, 5);
    pathB.cubicTo(-0.333333343f, 5.66666651f, 3, 6.83333302f, 2, 4);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void loops_i3(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(2, 5);
    path.cubicTo(0, 6, -0.333333343f, 6.66666651f, 3, 7.83333302f);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0, 6);
    pathB.cubicTo(-0.333333343f, 6.66666651f, 3, 7.83333302f, 2, 5);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void loops_i4(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(3, 4);
    path.cubicTo(1, 5, 0.666666627f, 5.66666651f, 4, 6.83333302f);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1, 5);
    pathB.cubicTo(0.666666627f, 5.66666651f, 4, 6.83333302f, 3, 4);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void loops_i5(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(3, 5);
    path.cubicTo(1, 6, 0.666666627f, 6.66666651f, 4, 7.83333302f);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1, 6);
    pathB.cubicTo(0.666666627f, 6.66666651f, 4, 7.83333302f, 3, 5);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void loops_i6(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(4, 5);
    path.cubicTo(2, 6, 1.66666663f, 6.66666651f, 5, 7.83333302f);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(2, 6);
    pathB.cubicTo(1.66666663f, 6.66666651f, 5, 7.83333302f, 4, 5);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void cubics_d3(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(3, 4);
    path.cubicTo(0, 6, 6, 1, 4, 2);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1, 6);
    pathB.cubicTo(2, 4, 4, 3, 6, 0);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void cubics_o(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(1, 4);
    path.cubicTo(2, 6, 5, 0, 5, 3);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0, 5);
    pathB.cubicTo(3, 5, 4, 1, 6, 2);
    pathB.close();
    testPathOp(reporter, path, pathB, kXOR_SkPathOp, filename);
}

static void cubicOp158(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(2,4, 2,0, 2,0);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,2);
    pathB.cubicTo(0,2, 1,0, 4,2);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void loop17(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.moveTo(1, 2);
    path.cubicTo(0, 3, -0.333333343f, 3.33333325f, 0.833333373f, 3.5f);
    path.close();
    pathB.moveTo(0, 3);
    pathB.cubicTo(-0.333333343f, 3.33333325f, 0.833333373f, 3.5f, 1, 2);
    pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void circlesOp4(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.addCircle(0, 1, 5, SkPath::kCW_Direction);
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.addCircle(0, 1, 0, SkPath::kCW_Direction);
    testPathOp(reporter, path, pathB, kDifference_SkPathOp, filename);
}

static void bug5240(skiatest::Reporter* reporter, const char* filename) {
 SkPath path;
path.moveTo(815, 82);
path.cubicTo(814.4794311523438f, 82.7868881225586f, 814.5330810546875f,
82.6266555786133f, 814.5291137695312f, 82.6252212524414f);
path.cubicTo(814.5229492187500f, 82.6230010986328f, 814.3790283203125f,
83.0008087158203f, 813.8533935546875f, 82.7072601318359f);
path.close();
    testPathOp(reporter, path, path, kUnion_SkPathOp, filename);
}

static void android1(skiatest::Reporter* reporter, const char* filename) {
 SkPath path, pathB;
path.moveTo(SkBits2Float(0xc0a00000), SkBits2Float(0x00000000));  // -5, 0
path.lineTo(SkBits2Float(0x44866000), SkBits2Float(0x00000000));  // 1075, 0
path.lineTo(SkBits2Float(0x44866000), SkBits2Float(0x43720000));  // 1075, 242
path.lineTo(SkBits2Float(0xc0a00000), SkBits2Float(0x43720000));  // -5, 242
path.lineTo(SkBits2Float(0xc0a00000), SkBits2Float(0x00000000));  // -5, 0
path.close();
pathB.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
pathB.lineTo(SkBits2Float(0x44870000), SkBits2Float(0x00000000));  // 1080, 0
pathB.lineTo(SkBits2Float(0x44870000), SkBits2Float(0x43720000));  // 1080, 242
pathB.lineTo(SkBits2Float(0x00000000), SkBits2Float(0x43720000));  // 0, 242
pathB.lineTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
pathB.close();
    testPathOp(reporter, path, pathB, kIntersect_SkPathOp, filename);
}

static void seanbug(skiatest::Reporter* reporter, const char* filename) {
   SkPath path;
   path.setFillType(SkPath::kEvenOdd_FillType);
   path.moveTo(SkBits2Float(0x45b56000), SkBits2Float(0x45bca000));  // 5804, 6036
   path.lineTo(SkBits2Float(0x45b55f0a), SkBits2Float(0x45bc9fc0));  // 5803.88f, 6035.97f
   path.lineTo(SkBits2Float(0x45b55e15), SkBits2Float(0x45bc9f7b));  // 5803.76f, 6035.94f
   path.lineTo(SkBits2Float(0x45b55d1f), SkBits2Float(0x45bc9f32));  // 5803.64f, 6035.9f
   path.lineTo(SkBits2Float(0x45b55c29), SkBits2Float(0x45bc9ee3));  // 5803.52f, 6035.86f
   path.lineTo(SkBits2Float(0x45b55b34), SkBits2Float(0x45bc9e90));  // 5803.4f, 6035.82f
   path.lineTo(SkBits2Float(0x45b55a3f), SkBits2Float(0x45bc9e38));  // 5803.28f, 6035.78f
   path.lineTo(SkBits2Float(0x45b5594a), SkBits2Float(0x45bc9ddc));  // 5803.16f, 6035.73f
   path.lineTo(SkBits2Float(0x45b55856), SkBits2Float(0x45bc9d7a));  // 5803.04f, 6035.68f
   path.lineTo(SkBits2Float(0x45b55762), SkBits2Float(0x45bc9d14));  // 5802.92f, 6035.63f
   path.lineTo(SkBits2Float(0x45b5566f), SkBits2Float(0x45bc9caa));  // 5802.8f, 6035.58f
   path.lineTo(SkBits2Float(0x45b5557c), SkBits2Float(0x45bc9c3b));  // 5802.69f, 6035.53f
   path.lineTo(SkBits2Float(0x45b55489), SkBits2Float(0x45bc9bc7));  // 5802.57f, 6035.47f
   path.lineTo(SkBits2Float(0x45b55397), SkBits2Float(0x45bc9b4f));  // 5802.45f, 6035.41f
   path.lineTo(SkBits2Float(0x45b552a6), SkBits2Float(0x45bc9ad3));  // 5802.33f, 6035.35f
   path.lineTo(SkBits2Float(0x45b551b5), SkBits2Float(0x45bc9a52));  // 5802.21f, 6035.29f
   path.lineTo(SkBits2Float(0x45b550c5), SkBits2Float(0x45bc99cd));  // 5802.1f, 6035.23f
   path.lineTo(SkBits2Float(0x45b54fd6), SkBits2Float(0x45bc9943));  // 5801.98f, 6035.16f
   path.lineTo(SkBits2Float(0x45b54ee8), SkBits2Float(0x45bc98b6));  // 5801.86f, 6035.09f
   path.lineTo(SkBits2Float(0x45b54dfb), SkBits2Float(0x45bc9824));  // 5801.75f, 6035.02f
   path.lineTo(SkBits2Float(0x45b54d0e), SkBits2Float(0x45bc978d));  // 5801.63f, 6034.94f
   path.lineTo(SkBits2Float(0x45b54c23), SkBits2Float(0x45bc96f3));  // 5801.52f, 6034.87f
   path.lineTo(SkBits2Float(0x45b54b39), SkBits2Float(0x45bc9654));  // 5801.4f, 6034.79f
   path.lineTo(SkBits2Float(0x45b54a4f), SkBits2Float(0x45bc95b2));  // 5801.29f, 6034.71f
   path.lineTo(SkBits2Float(0x45b54967), SkBits2Float(0x45bc950b));  // 5801.18f, 6034.63f
   path.lineTo(SkBits2Float(0x45b54880), SkBits2Float(0x45bc9460));  // 5801.06f, 6034.55f
   path.lineTo(SkBits2Float(0x45b5479a), SkBits2Float(0x45bc93b1));  // 5800.95f, 6034.46f
   path.lineTo(SkBits2Float(0x45b546b6), SkBits2Float(0x45bc92fe));  // 5800.84f, 6034.37f
   path.lineTo(SkBits2Float(0x45b545d3), SkBits2Float(0x45bc9248));  // 5800.73f, 6034.29f
   path.lineTo(SkBits2Float(0x45b544f1), SkBits2Float(0x45bc918d));  // 5800.62f, 6034.19f
   path.lineTo(SkBits2Float(0x45b54410), SkBits2Float(0x45bc90cf));  // 5800.51f, 6034.1f
   path.lineTo(SkBits2Float(0x45b54331), SkBits2Float(0x45bc900d));  // 5800.4f, 6034.01f
   path.lineTo(SkBits2Float(0x45b54254), SkBits2Float(0x45bc8f47));  // 5800.29f, 6033.91f
   path.lineTo(SkBits2Float(0x45b54178), SkBits2Float(0x45bc8e7d));  // 5800.18f, 6033.81f
   path.lineTo(SkBits2Float(0x45b5409e), SkBits2Float(0x45bc8db0));  // 5800.08f, 6033.71f
   path.lineTo(SkBits2Float(0x45b53fc6), SkBits2Float(0x45bc8cde));  // 5799.97f, 6033.61f
   path.lineTo(SkBits2Float(0x45b53eef), SkBits2Float(0x45bc8c0a));  // 5799.87f, 6033.5f
   path.lineTo(SkBits2Float(0x45b53e1a), SkBits2Float(0x45bc8b31));  // 5799.76f, 6033.4f
   path.lineTo(SkBits2Float(0x45b53d47), SkBits2Float(0x45bc8a56));  // 5799.66f, 6033.29f
   path.lineTo(SkBits2Float(0x45b53c75), SkBits2Float(0x45bc8976));  // 5799.56f, 6033.18f
   path.lineTo(SkBits2Float(0x45b53ba6), SkBits2Float(0x45bc8893));  // 5799.46f, 6033.07f
   path.lineTo(SkBits2Float(0x45b53ad8), SkBits2Float(0x45bc87ad));  // 5799.36f, 6032.96f
   path.lineTo(SkBits2Float(0x45b53a0d), SkBits2Float(0x45bc86c4));  // 5799.26f, 6032.85f
   path.lineTo(SkBits2Float(0x45b53944), SkBits2Float(0x45bc85d6));  // 5799.16f, 6032.73f
   path.lineTo(SkBits2Float(0x45b5387c), SkBits2Float(0x45bc84e6));  // 5799.06f, 6032.61f
   path.lineTo(SkBits2Float(0x45b537b7), SkBits2Float(0x45bc83f2));  // 5798.96f, 6032.49f
   path.lineTo(SkBits2Float(0x45b536f4), SkBits2Float(0x45bc82fc));  // 5798.87f, 6032.37f
   path.lineTo(SkBits2Float(0x45b53634), SkBits2Float(0x45bc8201));  // 5798.78f, 6032.25f
   path.lineTo(SkBits2Float(0x45b53575), SkBits2Float(0x45bc8104));  // 5798.68f, 6032.13f
   path.lineTo(SkBits2Float(0x45b534ba), SkBits2Float(0x45bc8004));  // 5798.59f, 6032
   path.lineTo(SkBits2Float(0x45b53400), SkBits2Float(0x45bc7f00));  // 5798.5f, 6031.88f
   path.lineTo(SkBits2Float(0x45b53349), SkBits2Float(0x45bc7df9));  // 5798.41f, 6031.75f
   path.lineTo(SkBits2Float(0x45b53294), SkBits2Float(0x45bc7cf0));  // 5798.32f, 6031.62f
   path.lineTo(SkBits2Float(0x45b531e2), SkBits2Float(0x45bc7be3));  // 5798.24f, 6031.49f
   path.lineTo(SkBits2Float(0x45b53133), SkBits2Float(0x45bc7ad3));  // 5798.15f, 6031.35f
   path.lineTo(SkBits2Float(0x45b53086), SkBits2Float(0x45bc79c1));  // 5798.07f, 6031.22f
   path.lineTo(SkBits2Float(0x45b52fdc), SkBits2Float(0x45bc78ab));  // 5797.98f, 6031.08f
   path.lineTo(SkBits2Float(0x45b52f35), SkBits2Float(0x45bc7793));  // 5797.9f, 6030.95f
   path.lineTo(SkBits2Float(0x45b52e90), SkBits2Float(0x45bc7678));  // 5797.82f, 6030.81f
   path.lineTo(SkBits2Float(0x45b52def), SkBits2Float(0x45bc755a));  // 5797.74f, 6030.67f
   path.lineTo(SkBits2Float(0x45b52d50), SkBits2Float(0x45bc7439));  // 5797.66f, 6030.53f
   path.lineTo(SkBits2Float(0x45b52cb4), SkBits2Float(0x45bc7316));  // 5797.59f, 6030.39f
   path.lineTo(SkBits2Float(0x45b52c1b), SkBits2Float(0x45bc71f0));  // 5797.51f, 6030.24f
   path.lineTo(SkBits2Float(0x45b52b86), SkBits2Float(0x45bc70c7));  // 5797.44f, 6030.1f
   path.lineTo(SkBits2Float(0x45b52af3), SkBits2Float(0x45bc6f9c));  // 5797.37f, 6029.95f
   path.lineTo(SkBits2Float(0x45b52a63), SkBits2Float(0x45bc6e6e));  // 5797.3f, 6029.8f
   path.lineTo(SkBits2Float(0x45b529d7), SkBits2Float(0x45bc6d3e));  // 5797.23f, 6029.66f
   path.lineTo(SkBits2Float(0x45b5294e), SkBits2Float(0x45bc6c0b));  // 5797.16f, 6029.51f
   path.lineTo(SkBits2Float(0x45b528c8), SkBits2Float(0x45bc6ad6));  // 5797.1f, 6029.35f
   path.lineTo(SkBits2Float(0x45b52846), SkBits2Float(0x45bc699e));  // 5797.03f, 6029.2f
   path.lineTo(SkBits2Float(0x45b527c7), SkBits2Float(0x45bc6864));  // 5796.97f, 6029.05f
   path.lineTo(SkBits2Float(0x45b5274b), SkBits2Float(0x45bc6728));  // 5796.91f, 6028.89f
   path.lineTo(SkBits2Float(0x45b526d3), SkBits2Float(0x45bc65e9));  // 5796.85f, 6028.74f
   path.lineTo(SkBits2Float(0x45b5265e), SkBits2Float(0x45bc64a8));  // 5796.8f, 6028.58f
   path.lineTo(SkBits2Float(0x45b52600), SkBits2Float(0x45bc639b));  // 5796.75f, 6028.45f
   path.lineTo(SkBits2Float(0x45b52600), SkBits2Float(0x45bab032));  // 5796.75f, 5974.02f
   path.lineTo(SkBits2Float(0x45b52611), SkBits2Float(0x45baaffd));  // 5796.76f, 5974
   path.lineTo(SkBits2Float(0x45b52687), SkBits2Float(0x45baae9d));  // 5796.82f, 5973.83f
   path.lineTo(SkBits2Float(0x45b52700), SkBits2Float(0x45baad40));  // 5796.88f, 5973.66f
   path.lineTo(SkBits2Float(0x45b5277d), SkBits2Float(0x45baabe7));  // 5796.94f, 5973.49f
   path.lineTo(SkBits2Float(0x45b527fe), SkBits2Float(0x45baaa91));  // 5797, 5973.32f
   path.lineTo(SkBits2Float(0x45b52883), SkBits2Float(0x45baa93f));  // 5797.06f, 5973.16f
   path.lineTo(SkBits2Float(0x45b5290b), SkBits2Float(0x45baa7f1));  // 5797.13f, 5972.99f
   path.lineTo(SkBits2Float(0x45b52998), SkBits2Float(0x45baa6a6));  // 5797.2f, 5972.83f
   path.lineTo(SkBits2Float(0x45b52a28), SkBits2Float(0x45baa55f));  // 5797.27f, 5972.67f
   path.lineTo(SkBits2Float(0x45b52abb), SkBits2Float(0x45baa41c));  // 5797.34f, 5972.51f
   path.lineTo(SkBits2Float(0x45b52b52), SkBits2Float(0x45baa2dc));  // 5797.42f, 5972.36f
   path.lineTo(SkBits2Float(0x45b52bed), SkBits2Float(0x45baa1a0));  // 5797.49f, 5972.2f
   path.lineTo(SkBits2Float(0x45b52c8c), SkBits2Float(0x45baa068));  // 5797.57f, 5972.05f
   path.lineTo(SkBits2Float(0x45b52d2e), SkBits2Float(0x45ba9f34));  // 5797.65f, 5971.9f
   path.lineTo(SkBits2Float(0x45b52dd3), SkBits2Float(0x45ba9e04));  // 5797.73f, 5971.75f
   path.lineTo(SkBits2Float(0x45b52e7c), SkBits2Float(0x45ba9cd8));  // 5797.81f, 5971.61f
   path.lineTo(SkBits2Float(0x45b52f28), SkBits2Float(0x45ba9baf));  // 5797.89f, 5971.46f
   path.lineTo(SkBits2Float(0x45b52fd8), SkBits2Float(0x45ba9a8b));  // 5797.98f, 5971.32f
   path.lineTo(SkBits2Float(0x45b5308b), SkBits2Float(0x45ba996b));  // 5798.07f, 5971.18f
   path.lineTo(SkBits2Float(0x45b53141), SkBits2Float(0x45ba984f));  // 5798.16f, 5971.04f
   path.lineTo(SkBits2Float(0x45b531fa), SkBits2Float(0x45ba9736));  // 5798.25f, 5970.9f
   path.lineTo(SkBits2Float(0x45b532b7), SkBits2Float(0x45ba9623));  // 5798.34f, 5970.77f
   path.lineTo(SkBits2Float(0x45b53377), SkBits2Float(0x45ba9513));  // 5798.43f, 5970.63f
   path.lineTo(SkBits2Float(0x45b5343a), SkBits2Float(0x45ba9407));  // 5798.53f, 5970.5f
   path.lineTo(SkBits2Float(0x45b53500), SkBits2Float(0x45ba9300));  // 5798.63f, 5970.38f
   path.lineTo(SkBits2Float(0x45b535c9), SkBits2Float(0x45ba91fd));  // 5798.72f, 5970.25f
   path.lineTo(SkBits2Float(0x45b53695), SkBits2Float(0x45ba90fe));  // 5798.82f, 5970.12f
   path.lineTo(SkBits2Float(0x45b53765), SkBits2Float(0x45ba9004));  // 5798.92f, 5970
   path.lineTo(SkBits2Float(0x45b53837), SkBits2Float(0x45ba8f0e));  // 5799.03f, 5969.88f
   path.lineTo(SkBits2Float(0x45b5390c), SkBits2Float(0x45ba8e1d));  // 5799.13f, 5969.76f
   path.lineTo(SkBits2Float(0x45b539e4), SkBits2Float(0x45ba8d30));  // 5799.24f, 5969.65f
   path.lineTo(SkBits2Float(0x45b53abf), SkBits2Float(0x45ba8c48));  // 5799.34f, 5969.54f
   path.lineTo(SkBits2Float(0x45b53b9d), SkBits2Float(0x45ba8b64));  // 5799.45f, 5969.42f
   path.lineTo(SkBits2Float(0x45b53c7d), SkBits2Float(0x45ba8a85));  // 5799.56f, 5969.31f
   path.lineTo(SkBits2Float(0x45b53d60), SkBits2Float(0x45ba89aa));  // 5799.67f, 5969.21f
   path.lineTo(SkBits2Float(0x45b53e46), SkBits2Float(0x45ba88d4));  // 5799.78f, 5969.1f
   path.lineTo(SkBits2Float(0x45b53f2f), SkBits2Float(0x45ba8803));  // 5799.9f, 5969
   path.lineTo(SkBits2Float(0x45b5401a), SkBits2Float(0x45ba8736));  // 5800.01f, 5968.9f
   path.lineTo(SkBits2Float(0x45b54108), SkBits2Float(0x45ba866f));  // 5800.13f, 5968.8f
   path.lineTo(SkBits2Float(0x45b541f8), SkBits2Float(0x45ba85ac));  // 5800.25f, 5968.71f
   path.lineTo(SkBits2Float(0x45b542eb), SkBits2Float(0x45ba84ee));  // 5800.36f, 5968.62f
   path.lineTo(SkBits2Float(0x45b543e0), SkBits2Float(0x45ba8435));  // 5800.48f, 5968.53f
   path.lineTo(SkBits2Float(0x45b544d8), SkBits2Float(0x45ba8380));  // 5800.61f, 5968.44f
   path.lineTo(SkBits2Float(0x45b545d2), SkBits2Float(0x45ba82d1));  // 5800.73f, 5968.35f
   path.lineTo(SkBits2Float(0x45b546cf), SkBits2Float(0x45ba8227));  // 5800.85f, 5968.27f
   path.lineTo(SkBits2Float(0x45b547ce), SkBits2Float(0x45ba8182));  // 5800.98f, 5968.19f
   path.lineTo(SkBits2Float(0x45b548cf), SkBits2Float(0x45ba80e2));  // 5801.1f, 5968.11f
   path.lineTo(SkBits2Float(0x45b549d2), SkBits2Float(0x45ba8047));  // 5801.23f, 5968.03f
   path.lineTo(SkBits2Float(0x45b54ad8), SkBits2Float(0x45ba7fb1));  // 5801.36f, 5967.96f
   path.lineTo(SkBits2Float(0x45b54be0), SkBits2Float(0x45ba7f20));  // 5801.48f, 5967.89f
   path.lineTo(SkBits2Float(0x45b54cea), SkBits2Float(0x45ba7e95));  // 5801.61f, 5967.82f
   path.lineTo(SkBits2Float(0x45b54df6), SkBits2Float(0x45ba7e0e));  // 5801.75f, 5967.76f
   path.lineTo(SkBits2Float(0x45b54f04), SkBits2Float(0x45ba7d8d));  // 5801.88f, 5967.69f
   path.lineTo(SkBits2Float(0x45b55015), SkBits2Float(0x45ba7d12));  // 5802.01f, 5967.63f
   path.lineTo(SkBits2Float(0x45b55127), SkBits2Float(0x45ba7c9c));  // 5802.14f, 5967.58f
   path.lineTo(SkBits2Float(0x45b551b5), SkBits2Float(0x45ba7c62));  // 5802.21f, 5967.55f
   path.lineTo(SkBits2Float(0x45c7b29a), SkBits2Float(0x45ba7c62));  // 6390.33f, 5967.55f
   path.lineTo(SkBits2Float(0x45c7b2f2), SkBits2Float(0x45ba7c8b));  // 6390.37f, 5967.57f
   path.lineTo(SkBits2Float(0x45c7b3dd), SkBits2Float(0x45ba7cff));  // 6390.48f, 5967.62f
   path.lineTo(SkBits2Float(0x45c7b4c7), SkBits2Float(0x45ba7d78));  // 6390.6f, 5967.68f
   path.lineTo(SkBits2Float(0x45c7b5b1), SkBits2Float(0x45ba7df5));  // 6390.71f, 5967.74f
   path.lineTo(SkBits2Float(0x45c7b699), SkBits2Float(0x45ba7e78));  // 6390.82f, 5967.81f
   path.lineTo(SkBits2Float(0x45c7b780), SkBits2Float(0x45ba7f00));  // 6390.94f, 5967.88f
   path.lineTo(SkBits2Float(0x45c7b866), SkBits2Float(0x45ba7f8d));  // 6391.05f, 5967.94f
   path.lineTo(SkBits2Float(0x45c7b94a), SkBits2Float(0x45ba801e));  // 6391.16f, 5968.01f
   path.lineTo(SkBits2Float(0x45c7ba2d), SkBits2Float(0x45ba80b5));  // 6391.27f, 5968.09f
   path.lineTo(SkBits2Float(0x45c7bb0f), SkBits2Float(0x45ba8150));  // 6391.38f, 5968.16f
   path.lineTo(SkBits2Float(0x45c7bbf0), SkBits2Float(0x45ba81f0));  // 6391.49f, 5968.24f
   path.lineTo(SkBits2Float(0x45c7bccf), SkBits2Float(0x45ba8294));  // 6391.6f, 5968.32f
   path.lineTo(SkBits2Float(0x45c7bdac), SkBits2Float(0x45ba833d));  // 6391.71f, 5968.4f
   path.lineTo(SkBits2Float(0x45c7be88), SkBits2Float(0x45ba83eb));  // 6391.82f, 5968.49f
   path.lineTo(SkBits2Float(0x45c7bf62), SkBits2Float(0x45ba849d));  // 6391.92f, 5968.58f
   path.lineTo(SkBits2Float(0x45c7c03a), SkBits2Float(0x45ba8554));  // 6392.03f, 5968.67f
   path.lineTo(SkBits2Float(0x45c7c111), SkBits2Float(0x45ba860f));  // 6392.13f, 5968.76f
   path.lineTo(SkBits2Float(0x45c7c1e6), SkBits2Float(0x45ba86cf));  // 6392.24f, 5968.85f
   path.lineTo(SkBits2Float(0x45c7c2b9), SkBits2Float(0x45ba8792));  // 6392.34f, 5968.95f
   path.lineTo(SkBits2Float(0x45c7c38b), SkBits2Float(0x45ba885b));  // 6392.44f, 5969.04f
   path.lineTo(SkBits2Float(0x45c7c45a), SkBits2Float(0x45ba8927));  // 6392.54f, 5969.14f
   path.lineTo(SkBits2Float(0x45c7c528), SkBits2Float(0x45ba89f7));  // 6392.64f, 5969.25f
   path.lineTo(SkBits2Float(0x45c7c5f3), SkBits2Float(0x45ba8acc));  // 6392.74f, 5969.35f
   path.lineTo(SkBits2Float(0x45c7c6bc), SkBits2Float(0x45ba8ba5));  // 6392.84f, 5969.46f
   path.lineTo(SkBits2Float(0x45c7c784), SkBits2Float(0x45ba8c82));  // 6392.94f, 5969.56f
   path.lineTo(SkBits2Float(0x45c7c849), SkBits2Float(0x45ba8d62));  // 6393.04f, 5969.67f
   path.lineTo(SkBits2Float(0x45c7c90c), SkBits2Float(0x45ba8e47));  // 6393.13f, 5969.78f
   path.lineTo(SkBits2Float(0x45c7c9cc), SkBits2Float(0x45ba8f30));  // 6393.22f, 5969.9f
   path.lineTo(SkBits2Float(0x45c7ca8b), SkBits2Float(0x45ba901c));  // 6393.32f, 5970.01f
   path.lineTo(SkBits2Float(0x45c7cb46), SkBits2Float(0x45ba910c));  // 6393.41f, 5970.13f
   path.lineTo(SkBits2Float(0x45c7cc00), SkBits2Float(0x45ba9200));  // 6393.5f, 5970.25f
   path.lineTo(SkBits2Float(0x45c7ccb7), SkBits2Float(0x45ba92f8));  // 6393.59f, 5970.37f
   path.lineTo(SkBits2Float(0x45c7cd6c), SkBits2Float(0x45ba93f3));  // 6393.68f, 5970.49f
   path.lineTo(SkBits2Float(0x45c7ce1e), SkBits2Float(0x45ba94f2));  // 6393.76f, 5970.62f
   path.lineTo(SkBits2Float(0x45c7cecd), SkBits2Float(0x45ba95f4));  // 6393.85f, 5970.74f
   path.lineTo(SkBits2Float(0x45c7cf7a), SkBits2Float(0x45ba96fa));  // 6393.93f, 5970.87f
   path.lineTo(SkBits2Float(0x45c7d024), SkBits2Float(0x45ba9803));  // 6394.02f, 5971
   path.lineTo(SkBits2Float(0x45c7d0cb), SkBits2Float(0x45ba9910));  // 6394.1f, 5971.13f
   path.lineTo(SkBits2Float(0x45c7d170), SkBits2Float(0x45ba9a20));  // 6394.18f, 5971.27f
   path.lineTo(SkBits2Float(0x45c7d211), SkBits2Float(0x45ba9b33));  // 6394.26f, 5971.4f
   path.lineTo(SkBits2Float(0x45c7d2b0), SkBits2Float(0x45ba9c4a));  // 6394.34f, 5971.54f
   path.lineTo(SkBits2Float(0x45c7d34c), SkBits2Float(0x45ba9d63));  // 6394.41f, 5971.67f
   path.lineTo(SkBits2Float(0x45c7d3e5), SkBits2Float(0x45ba9e80));  // 6394.49f, 5971.81f
   path.lineTo(SkBits2Float(0x45c7d47a), SkBits2Float(0x45ba9fa0));  // 6394.56f, 5971.95f
   path.lineTo(SkBits2Float(0x45c7d50d), SkBits2Float(0x45baa0c3));  // 6394.63f, 5972.1f
   path.lineTo(SkBits2Float(0x45c7d59d), SkBits2Float(0x45baa1e9));  // 6394.7f, 5972.24f
   path.lineTo(SkBits2Float(0x45c7d629), SkBits2Float(0x45baa312));  // 6394.77f, 5972.38f
   path.lineTo(SkBits2Float(0x45c7d6b2), SkBits2Float(0x45baa43e));  // 6394.84f, 5972.53f
   path.lineTo(SkBits2Float(0x45c7d738), SkBits2Float(0x45baa56d));  // 6394.9f, 5972.68f
   path.lineTo(SkBits2Float(0x45c7d7ba), SkBits2Float(0x45baa69f));  // 6394.97f, 5972.83f
   path.lineTo(SkBits2Float(0x45c7d839), SkBits2Float(0x45baa7d3));  // 6395.03f, 5972.98f
   path.lineTo(SkBits2Float(0x45c7d8b5), SkBits2Float(0x45baa90a));  // 6395.09f, 5973.13f
   path.lineTo(SkBits2Float(0x45c7d92d), SkBits2Float(0x45baaa44));  // 6395.15f, 5973.28f
   path.lineTo(SkBits2Float(0x45c7d9a2), SkBits2Float(0x45baab80));  // 6395.2f, 5973.44f
   path.lineTo(SkBits2Float(0x45c7da13), SkBits2Float(0x45baacbf));  // 6395.26f, 5973.59f
   path.lineTo(SkBits2Float(0x45c7da80), SkBits2Float(0x45baae00));  // 6395.31f, 5973.75f
   path.lineTo(SkBits2Float(0x45c7daea), SkBits2Float(0x45baaf44));  // 6395.36f, 5973.91f
   path.lineTo(SkBits2Float(0x45c7db50), SkBits2Float(0x45bab08a));  // 6395.41f, 5974.07f
   path.lineTo(SkBits2Float(0x45c7dbb2), SkBits2Float(0x45bab1d3));  // 6395.46f, 5974.23f
   path.lineTo(SkBits2Float(0x45c7dc10), SkBits2Float(0x45bab31d));  // 6395.51f, 5974.39f
   path.lineTo(SkBits2Float(0x45c7dc6a), SkBits2Float(0x45bab46a));  // 6395.55f, 5974.55f
   path.lineTo(SkBits2Float(0x45c7dc6b), SkBits2Float(0x45bc5fbe));  // 6395.55f, 6027.97f
   path.lineTo(SkBits2Float(0x45c7dc10), SkBits2Float(0x45bc60e7));  // 6395.51f, 6028.11f
   path.lineTo(SkBits2Float(0x45c7dbb2), SkBits2Float(0x45bc620f));  // 6395.46f, 6028.26f
   path.lineTo(SkBits2Float(0x45c7db50), SkBits2Float(0x45bc6336));  // 6395.41f, 6028.4f
   path.lineTo(SkBits2Float(0x45c7daea), SkBits2Float(0x45bc645c));  // 6395.36f, 6028.54f
   path.lineTo(SkBits2Float(0x45c7da80), SkBits2Float(0x45bc6580));  // 6395.31f, 6028.69f
   path.lineTo(SkBits2Float(0x45c7da13), SkBits2Float(0x45bc66a3));  // 6395.26f, 6028.83f
   path.lineTo(SkBits2Float(0x45c7d9a2), SkBits2Float(0x45bc67c5));  // 6395.2f, 6028.97f
   path.lineTo(SkBits2Float(0x45c7d92d), SkBits2Float(0x45bc68e6));  // 6395.15f, 6029.11f
   path.lineTo(SkBits2Float(0x45c7d8b5), SkBits2Float(0x45bc6a05));  // 6395.09f, 6029.25f
   path.lineTo(SkBits2Float(0x45c7d839), SkBits2Float(0x45bc6b23));  // 6395.03f, 6029.39f
   path.lineTo(SkBits2Float(0x45c7d7ba), SkBits2Float(0x45bc6c3f));  // 6394.97f, 6029.53f
   path.lineTo(SkBits2Float(0x45c7d738), SkBits2Float(0x45bc6d5a));  // 6394.9f, 6029.67f
   path.lineTo(SkBits2Float(0x45c7d6b2), SkBits2Float(0x45bc6e73));  // 6394.84f, 6029.81f
   path.lineTo(SkBits2Float(0x45c7d629), SkBits2Float(0x45bc6f8b));  // 6394.77f, 6029.94f
   path.lineTo(SkBits2Float(0x45c7d59d), SkBits2Float(0x45bc70a1));  // 6394.7f, 6030.08f
   path.lineTo(SkBits2Float(0x45c7d50d), SkBits2Float(0x45bc71b5));  // 6394.63f, 6030.21f
   path.lineTo(SkBits2Float(0x45c7d47a), SkBits2Float(0x45bc72c7));  // 6394.56f, 6030.35f
   path.lineTo(SkBits2Float(0x45c7d3e5), SkBits2Float(0x45bc73d8));  // 6394.49f, 6030.48f
   path.lineTo(SkBits2Float(0x45c7d34c), SkBits2Float(0x45bc74e7));  // 6394.41f, 6030.61f
   path.lineTo(SkBits2Float(0x45c7d2b0), SkBits2Float(0x45bc75f4));  // 6394.34f, 6030.74f
   path.lineTo(SkBits2Float(0x45c7d211), SkBits2Float(0x45bc76ff));  // 6394.26f, 6030.87f
   path.lineTo(SkBits2Float(0x45c7d170), SkBits2Float(0x45bc7807));  // 6394.18f, 6031
   path.lineTo(SkBits2Float(0x45c7d0cb), SkBits2Float(0x45bc790e));  // 6394.1f, 6031.13f
   path.lineTo(SkBits2Float(0x45c7d024), SkBits2Float(0x45bc7a13));  // 6394.02f, 6031.26f
   path.lineTo(SkBits2Float(0x45c7cf7a), SkBits2Float(0x45bc7b16));  // 6393.93f, 6031.39f
   path.lineTo(SkBits2Float(0x45c7cecd), SkBits2Float(0x45bc7c16));  // 6393.85f, 6031.51f
   path.lineTo(SkBits2Float(0x45c7ce1e), SkBits2Float(0x45bc7d14));  // 6393.76f, 6031.63f
   path.lineTo(SkBits2Float(0x45c7cd6c), SkBits2Float(0x45bc7e10));  // 6393.68f, 6031.76f
   path.lineTo(SkBits2Float(0x45c7ccb7), SkBits2Float(0x45bc7f09));  // 6393.59f, 6031.88f
   path.lineTo(SkBits2Float(0x45c7cc00), SkBits2Float(0x45bc8000));  // 6393.5f, 6032
   path.lineTo(SkBits2Float(0x45c7cb46), SkBits2Float(0x45bc80f5));  // 6393.41f, 6032.12f
   path.lineTo(SkBits2Float(0x45c7ca8b), SkBits2Float(0x45bc81e7));  // 6393.32f, 6032.24f
   path.lineTo(SkBits2Float(0x45c7c9cc), SkBits2Float(0x45bc82d6));  // 6393.22f, 6032.35f
   path.lineTo(SkBits2Float(0x45c7c90c), SkBits2Float(0x45bc83c3));  // 6393.13f, 6032.47f
   path.lineTo(SkBits2Float(0x45c7c849), SkBits2Float(0x45bc84ad));  // 6393.04f, 6032.58f
   path.lineTo(SkBits2Float(0x45c7c784), SkBits2Float(0x45bc8595));  // 6392.94f, 6032.7f
   path.lineTo(SkBits2Float(0x45c7c6bc), SkBits2Float(0x45bc8679));  // 6392.84f, 6032.81f
   path.lineTo(SkBits2Float(0x45c7c5f3), SkBits2Float(0x45bc875b));  // 6392.74f, 6032.92f
   path.lineTo(SkBits2Float(0x45c7c528), SkBits2Float(0x45bc883a));  // 6392.64f, 6033.03f
   path.lineTo(SkBits2Float(0x45c7c45a), SkBits2Float(0x45bc8917));  // 6392.54f, 6033.14f
   path.lineTo(SkBits2Float(0x45c7c38b), SkBits2Float(0x45bc89f0));  // 6392.44f, 6033.24f
   path.lineTo(SkBits2Float(0x45c7c2b9), SkBits2Float(0x45bc8ac6));  // 6392.34f, 6033.35f
   path.lineTo(SkBits2Float(0x45c7c1e6), SkBits2Float(0x45bc8b99));  // 6392.24f, 6033.45f
   path.lineTo(SkBits2Float(0x45c7c111), SkBits2Float(0x45bc8c69));  // 6392.13f, 6033.55f
   path.lineTo(SkBits2Float(0x45c7c03a), SkBits2Float(0x45bc8d36));  // 6392.03f, 6033.65f
   path.lineTo(SkBits2Float(0x45c7bf62), SkBits2Float(0x45bc8e00));  // 6391.92f, 6033.75f
   path.lineTo(SkBits2Float(0x45c7be88), SkBits2Float(0x45bc8ec7));  // 6391.82f, 6033.85f
   path.lineTo(SkBits2Float(0x45c7bdac), SkBits2Float(0x45bc8f8a));  // 6391.71f, 6033.94f
   path.lineTo(SkBits2Float(0x45c7bccf), SkBits2Float(0x45bc904a));  // 6391.6f, 6034.04f
   path.lineTo(SkBits2Float(0x45c7bbf0), SkBits2Float(0x45bc9106));  // 6391.49f, 6034.13f
   path.lineTo(SkBits2Float(0x45c7bb0f), SkBits2Float(0x45bc91bf));  // 6391.38f, 6034.22f
   path.lineTo(SkBits2Float(0x45c7ba2d), SkBits2Float(0x45bc9275));  // 6391.27f, 6034.31f
   path.lineTo(SkBits2Float(0x45c7b94a), SkBits2Float(0x45bc9327));  // 6391.16f, 6034.39f
   path.lineTo(SkBits2Float(0x45c7b866), SkBits2Float(0x45bc93d5));  // 6391.05f, 6034.48f
   path.lineTo(SkBits2Float(0x45c7b780), SkBits2Float(0x45bc9480));  // 6390.94f, 6034.56f
   path.lineTo(SkBits2Float(0x45c7b699), SkBits2Float(0x45bc9527));  // 6390.82f, 6034.64f
   path.lineTo(SkBits2Float(0x45c7b5b1), SkBits2Float(0x45bc95ca));  // 6390.71f, 6034.72f
   path.lineTo(SkBits2Float(0x45c7b4c8), SkBits2Float(0x45bc966a));  // 6390.6f, 6034.8f
   path.lineTo(SkBits2Float(0x45c7b3dd), SkBits2Float(0x45bc9706));  // 6390.48f, 6034.88f
   path.lineTo(SkBits2Float(0x45c7b2f2), SkBits2Float(0x45bc979e));  // 6390.37f, 6034.95f
   path.lineTo(SkBits2Float(0x45c7b205), SkBits2Float(0x45bc9832));  // 6390.25f, 6035.02f
   path.lineTo(SkBits2Float(0x45c7b118), SkBits2Float(0x45bc98c2));  // 6390.14f, 6035.09f
   path.lineTo(SkBits2Float(0x45c7b02a), SkBits2Float(0x45bc994e));  // 6390.02f, 6035.16f
   path.lineTo(SkBits2Float(0x45c7af3b), SkBits2Float(0x45bc99d5));  // 6389.9f, 6035.23f
   path.lineTo(SkBits2Float(0x45c7ae4b), SkBits2Float(0x45bc9a59));  // 6389.79f, 6035.29f
   path.lineTo(SkBits2Float(0x45c7ad5a), SkBits2Float(0x45bc9ad9));  // 6389.67f, 6035.36f
   path.lineTo(SkBits2Float(0x45c7ac69), SkBits2Float(0x45bc9b54));  // 6389.55f, 6035.42f
   path.lineTo(SkBits2Float(0x45c7ab77), SkBits2Float(0x45bc9bcb));  // 6389.43f, 6035.47f
   path.lineTo(SkBits2Float(0x45c7aa84), SkBits2Float(0x45bc9c3e));  // 6389.31f, 6035.53f
   path.lineTo(SkBits2Float(0x45c7a991), SkBits2Float(0x45bc9cac));  // 6389.2f, 6035.58f
   path.lineTo(SkBits2Float(0x45c7a89e), SkBits2Float(0x45bc9d16));  // 6389.08f, 6035.64f
   path.lineTo(SkBits2Float(0x45c7a7aa), SkBits2Float(0x45bc9d7b));  // 6388.96f, 6035.69f
   path.lineTo(SkBits2Float(0x45c7a6b6), SkBits2Float(0x45bc9ddc));  // 6388.84f, 6035.73f
   path.lineTo(SkBits2Float(0x45c7a5c1), SkBits2Float(0x45bc9e39));  // 6388.72f, 6035.78f
   path.lineTo(SkBits2Float(0x45c7a4cc), SkBits2Float(0x45bc9e90));  // 6388.6f, 6035.82f
   path.lineTo(SkBits2Float(0x45c7a3d7), SkBits2Float(0x45bc9ee3));  // 6388.48f, 6035.86f
   path.lineTo(SkBits2Float(0x45c7a2e1), SkBits2Float(0x45bc9f32));  // 6388.36f, 6035.9f
   path.lineTo(SkBits2Float(0x45c7a1eb), SkBits2Float(0x45bc9f7b));  // 6388.24f, 6035.94f
   path.lineTo(SkBits2Float(0x45c7a0f6), SkBits2Float(0x45bc9fc0));  // 6388.12f, 6035.97f
   path.lineTo(SkBits2Float(0x45c7a000), SkBits2Float(0x45bca000));  // 6388, 6036
   path.lineTo(SkBits2Float(0x45b56000), SkBits2Float(0x45bca000));  // 5804, 6036
   path.close();

   SkPath path2;
   path2.setFillType(SkPath::kWinding_FillType);
   path2.moveTo(SkBits2Float(0x45b52600), SkBits2Float(0x45ba7c62));  // 5796.75f, 5967.55f
   path2.lineTo(SkBits2Float(0x45c7dc6b), SkBits2Float(0x45ba7c62));  // 6395.55f, 5967.55f
   path2.lineTo(SkBits2Float(0x45c7dc6b), SkBits2Float(0x45bca239));  // 6395.55f, 6036.28f
   path2.lineTo(SkBits2Float(0x45b52600), SkBits2Float(0x45bca239));  // 5796.75f, 6036.28f
   path2.lineTo(SkBits2Float(0x45b52600), SkBits2Float(0x45ba7c62));  // 5796.75f, 5967.55f
   path2.close();

   SkPath result_path;
    testPathOp(reporter, path, path2, kIntersect_SkPathOp, filename);
}

static void halbug(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, path2;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(SkRect{278.653992f, 155.747406f, 580.15918f, 593.602051f});
    path2.setFillType(SkPath::kWinding_FillType);
    path2.addRect(SkRect{278.657715f, 155.747314f, 580.238281f, 594.114014f});
    testPathOp(reporter, path, path2, kIntersect_SkPathOp, filename);
}

static void testRect1_u(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0, 0);
    path.lineTo(0, 60);
    path.lineTo(60, 60);
    path.lineTo(60, 0);
    path.close();
    path.moveTo(30, 20);
    path.lineTo(30, 50);
    path.lineTo(50, 50);
    path.lineTo(50, 20);
    path.close();
    path.moveTo(24, 20);
    path.lineTo(24, 30);
    path.lineTo(36, 30);
    path.lineTo(36, 20);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    testPathOp(reporter, path, pathB, kUnion_SkPathOp, filename);
}

static void filinmangust14(skiatest::Reporter* reporter, const char* filename) {
SkPath path, path1;
path.setFillType(SkPath::kWinding_FillType);
        path.moveTo(SkBits2Float(0x440bc02c), SkBits2Float(0x4409c000));  // 559.003f, 551
        path.lineTo(SkBits2Float(0x440bc02c), SkBits2Float(0x440e8000));  // 559.003f, 570
        path.lineTo(SkBits2Float(0x440bbfda), SkBits2Float(0x440e8000));  // 558.998f, 570
        path.lineTo(SkBits2Float(0x440bbfda), SkBits2Float(0x4409c000));  // 558.998f, 551
        path.lineTo(SkBits2Float(0x440bc02c), SkBits2Float(0x4409c000));  // 559.003f, 551
        path.close();
path1 = path;
path.reset();
        path.setFillType(SkPath::kWinding_FillType);
        path.moveTo(SkBits2Float(0x45582000), SkBits2Float(0x45be9805));  // 3458, 6099
        path.lineTo(SkBits2Float(0x4554b667), SkBits2Float(0x45be9805));  // 3403.4f, 6099
        path.lineTo(SkBits2Float(0x4554b667), SkBits2Float(0x45be97fb));  // 3403.4f, 6099
        path.lineTo(SkBits2Float(0x45582000), SkBits2Float(0x45be97fb));  // 3458, 6099
        path.lineTo(SkBits2Float(0x45582000), SkBits2Float(0x45be9805));  // 3458, 6099
        path.close();
        path.moveTo(SkBits2Float(0x43b60000), SkBits2Float(0x443dffd7));  // 364, 759.997f
        path.lineTo(SkBits2Float(0x4554b667), SkBits2Float(0x443dffd7));  // 3403.4f, 759.997f
        path.lineTo(SkBits2Float(0x4554b667), SkBits2Float(0x443e0029));  // 3403.4f, 760.003f
        path.lineTo(SkBits2Float(0x43b60000), SkBits2Float(0x443e0029));  // 364, 760.003f
        path.lineTo(SkBits2Float(0x43b60000), SkBits2Float(0x443dffd7));  // 364, 759.997f
        path.close();
        path.moveTo(SkBits2Float(0x4554b65d), SkBits2Float(0x45be9800));  // 3403.4f, 6099
        path.lineTo(SkBits2Float(0x4554b65d), SkBits2Float(0x443e0000));  // 3403.4f, 760
        path.lineTo(SkBits2Float(0x4554b671), SkBits2Float(0x443e0000));  // 3403.4f, 760
        path.lineTo(SkBits2Float(0x4554b671), SkBits2Float(0x45be9800));  // 3403.4f, 6099
        path.lineTo(SkBits2Float(0x4554b65d), SkBits2Float(0x45be9800));  // 3403.4f, 6099
        path.close();
        path.moveTo(SkBits2Float(0x449f4000), SkBits2Float(0x43bdffae));  // 1274, 379.997f
        path.lineTo(SkBits2Float(0x4554b667), SkBits2Float(0x43bdffae));  // 3403.4f, 379.997f
        path.lineTo(SkBits2Float(0x4554b667), SkBits2Float(0x43be0052));  // 3403.4f, 380.003f
        path.lineTo(SkBits2Float(0x449f4000), SkBits2Float(0x43be0052));  // 1274, 380.003f
        path.lineTo(SkBits2Float(0x449f4000), SkBits2Float(0x43bdffae));  // 1274, 379.997f
        path.close();
        path.moveTo(SkBits2Float(0x4554b65d), SkBits2Float(0x443e0000));  // 3403.4f, 760
        path.lineTo(SkBits2Float(0x4554b65d), SkBits2Float(0x43be0000));  // 3403.4f, 380
        path.lineTo(SkBits2Float(0x4554b671), SkBits2Float(0x43be0000));  // 3403.4f, 380
        path.lineTo(SkBits2Float(0x4554b671), SkBits2Float(0x443e0000));  // 3403.4f, 760
        path.lineTo(SkBits2Float(0x4554b65d), SkBits2Float(0x443e0000));  // 3403.4f, 760
        path.close();
            testPathOp(reporter, path1, path, kUnion_SkPathOp, filename);
}

static void grshapearcs1(skiatest::Reporter* reporter, const char* filename) {
SkPath path, path1;
path.setFillType(SkPath::kWinding_FillType);
path.moveTo(25.0098f, 23.1973f);
path.lineTo(25.5689f, 22.3682f);
path.conicTo(26.1281f, 21.5392f, 26.9572f, 22.0984f, 0.707107f);
path.conicTo(27.7862f, 22.6576f, 27.227f, 23.4866f, 0.707107f);
path.lineTo(26.6678f, 24.3156f);
path.conicTo(26.1086f, 25.1447f, 25.2796f, 24.5855f, 0.707107f);
path.conicTo(24.4506f, 24.0263f, 25.0098f, 23.1973f, 0.707107f);
path.close();
path.moveTo(26.6873f, 20.7101f);
path.lineTo(27.2465f, 19.8811f);
path.conicTo(27.8057f, 19.0521f, 28.6348f, 19.6113f, 0.707107f);
path.conicTo(29.4638f, 20.1704f, 28.9046f, 20.9995f, 0.707107f);
path.lineTo(28.3454f, 21.8285f);
path.conicTo(27.7862f, 22.6576f, 26.9572f, 22.0984f, 0.707107f);
path.conicTo(26.1281f, 21.5392f, 26.6873f, 20.7101f, 0.707107f);
path.close();
path.moveTo(28.3649f, 18.223f);
path.lineTo(28.9241f, 17.394f);
path.conicTo(29.4833f, 16.565f, 30.3123f, 17.1241f, 0.707107f);
path.conicTo(31.1414f, 17.6833f, 30.5822f, 18.5124f, 0.707107f);
path.lineTo(30.023f, 19.3414f);
path.conicTo(29.4638f, 20.1704f, 28.6348f, 19.6113f, 0.707107f);
path.conicTo(27.8057f, 19.0521f, 28.3649f, 18.223f, 0.707107f);
path.close();
path.moveTo(30.0425f, 15.7359f);
path.lineTo(30.6017f, 14.9069f);
path.conicTo(31.1609f, 14.0778f, 31.9899f, 14.637f, 0.707107f);
path.conicTo(32.8189f, 15.1962f, 32.2598f, 16.0253f, 0.707107f);
path.lineTo(31.7006f, 16.8543f);
path.conicTo(31.1414f, 17.6833f, 30.3123f, 17.1241f, 0.707107f);
path.conicTo(29.4833f, 16.565f, 30.0425f, 15.7359f, 0.707107f);
path.close();
path.moveTo(31.7201f, 13.2488f);
path.lineTo(32.2793f, 12.4198f);
path.conicTo(32.8385f, 11.5907f, 33.6675f, 12.1499f, 0.707107f);
path.conicTo(34.4965f, 12.7091f, 33.9373f, 13.5381f, 0.707107f);
path.lineTo(33.3781f, 14.3672f);
path.conicTo(32.8189f, 15.1962f, 31.9899f, 14.637f, 0.707107f);
path.conicTo(31.1609f, 14.0778f, 31.7201f, 13.2488f, 0.707107f);
path.close();
path.moveTo(33.3976f, 10.7617f);
path.lineTo(33.9568f, 9.93265f);
path.conicTo(34.516f, 9.10361f, 35.3451f, 9.6628f, 0.707107f);
path.conicTo(36.1741f, 10.222f, 35.6149f, 11.051f, 0.707107f);
path.lineTo(35.0557f, 11.8801f);
path.conicTo(34.4965f, 12.7091f, 33.6675f, 12.1499f, 0.707107f);
path.conicTo(32.8385f, 11.5907f, 33.3976f, 10.7617f, 0.707107f);
path.close();
path.moveTo(35.0752f, 8.27457f);
path.lineTo(35.6344f, 7.44554f);
path.conicTo(36.1936f, 6.6165f, 37.0226f, 7.17569f, 0.707107f);
path.conicTo(37.8517f, 7.73488f, 37.2925f, 8.56392f, 0.707107f);
path.lineTo(36.7333f, 9.39296f);
path.conicTo(36.1741f, 10.222f, 35.3451f, 9.6628f, 0.707107f);
path.conicTo(34.516f, 9.10361f, 35.0752f, 8.27457f, 0.707107f);
path.close();
path.moveTo(36.7528f, 5.78746f);
path.lineTo(37.312f, 4.95842f);
path.conicTo(37.8712f, 4.12939f, 38.7002f, 4.68858f, 0.707107f);
path.conicTo(39.5293f, 5.24777f, 38.9701f, 6.07681f, 0.707107f);
path.lineTo(38.4109f, 6.90585f);
path.conicTo(37.8517f, 7.73488f, 37.0226f, 7.17569f, 0.707107f);
path.conicTo(36.1936f, 6.6165f, 36.7528f, 5.78746f, 0.707107f);
path.close();
path.moveTo(39.9447f, 3.72429f);
path.quadTo(40.3524f, 4.01069f, 40.7489f, 4.31248f);
path.conicTo(41.5445f, 4.9182f, 40.9388f, 5.71387f, 0.707107f);
path.conicTo(40.3331f, 6.50955f, 39.5374f, 5.90383f, 0.707107f);
path.quadTo(39.1714f, 5.62521f, 38.7951f, 5.36088f);
path.conicTo(37.9768f, 4.78608f, 38.5516f, 3.96779f, 0.707107f);
path.conicTo(39.1264f, 3.14949f, 39.9447f, 3.72429f, 0.707107f);
path.close();
path.moveTo(42.3194f, 5.60826f);
path.quadTo(42.707f, 5.95446f, 43.0804f, 6.31583f);
path.conicTo(43.7991f, 7.01122f, 43.1037f, 7.72985f, 0.707107f);
path.conicTo(42.4083f, 8.44848f, 41.6896f, 7.75308f, 0.707107f);
path.quadTo(41.3448f, 7.41944f, 40.9871f, 7.09992f);
path.conicTo(40.2413f, 6.43379f, 40.9074f, 5.68796f, 0.707107f);
path.conicTo(41.5735f, 4.94212f, 42.3194f, 5.60826f, 0.707107f);
path.close();
path.moveTo(44.5406f, 7.84871f);
path.quadTo(44.8959f, 8.25352f, 45.2341f, 8.67266f);
path.conicTo(45.862f, 9.4509f, 45.0838f, 10.0789f, 0.707107f);
path.conicTo(44.3056f, 10.7068f, 43.6776f, 9.9286f, 0.707107f);
path.quadTo(43.3654f, 9.54174f, 43.0374f, 9.16805f);
path.conicTo(42.3778f, 8.41649f, 43.1293f, 7.75682f, 0.707107f);
path.conicTo(43.8809f, 7.09715f, 44.5406f, 7.84871f, 0.707107f);
path.close();
path.moveTo(46.528f, 10.4211f);
path.quadTo(46.815f, 10.8449f, 47.0851f, 11.2796f);
path.conicTo(47.6128f, 12.129f, 46.7633f, 12.6567f, 0.707107f);
path.conicTo(45.9139f, 13.1844f, 45.3862f, 12.335f, 0.707107f);
path.quadTo(45.1369f, 11.9337f, 44.872f, 11.5426f);
path.conicTo(44.3113f, 10.7146f, 45.1393f, 10.1538f, 0.707107f);
path.conicTo(45.9673f, 9.5931f, 46.528f, 10.4211f, 0.707107f);
path.close();
path.moveTo(48.1056f, 13.0782f);
path.quadTo(48.3449f, 13.542f, 48.5654f, 14.015f);
path.conicTo(48.9879f, 14.9213f, 48.0816f, 15.3438f, 0.707107f);
path.conicTo(47.1752f, 15.7663f, 46.7527f, 14.86f, 0.707107f);
path.quadTo(46.5492f, 14.4234f, 46.3283f, 13.9953f);
path.conicTo(45.8698f, 13.1066f, 46.7584f, 12.6481f, 0.707107f);
path.conicTo(47.6471f, 12.1895f, 48.1056f, 13.0782f, 0.707107f);
path.close();
path.moveTo(49.3755f, 15.9538f);
path.quadTo(49.5594f, 16.4493f, 49.7229f, 16.9516f);
path.conicTo(50.0325f, 17.9025f, 49.0816f, 18.2121f, 0.707107f);
path.conicTo(48.1307f, 18.5216f, 47.8212f, 17.5707f, 0.707107f);
path.quadTo(47.6702f, 17.1069f, 47.5005f, 16.6497f);
path.conicTo(47.1526f, 15.7122f, 48.0901f, 15.3642f, 0.707107f);
path.conicTo(49.0276f, 15.0163f, 49.3755f, 15.9538f, 0.707107f);
path.close();
path.moveTo(50.2964f, 18.9923f);
path.quadTo(50.4191f, 19.5089f, 50.5206f, 20.0302f);
path.conicTo(50.7117f, 21.0117f, 49.7302f, 21.2029f, 0.707107f);
path.conicTo(48.7486f, 21.394f, 48.5575f, 20.4125f, 0.707107f);
path.quadTo(48.4638f, 19.9313f, 48.3505f, 19.4544f);
path.conicTo(48.1194f, 18.4815f, 49.0924f, 18.2504f, 0.707107f);
path.conicTo(50.0653f, 18.0193f, 50.2964f, 18.9923f, 0.707107f);
path.close();
path.moveTo(50.8373f, 22.0956f);
path.quadTo(50.8955f, 22.6138f, 50.933f, 23.1341f);
path.conicTo(51.0047f, 24.1315f, 50.0073f, 24.2033f, 0.707107f);
path.conicTo(49.0099f, 24.275f, 48.9381f, 23.2776f, 0.707107f);
path.quadTo(48.9036f, 22.7975f, 48.8498f, 22.3191f);
path.conicTo(48.7381f, 21.3253f, 49.7318f, 21.2136f, 0.707107f);
path.conicTo(50.7255f, 21.1019f, 50.8373f, 22.0956f, 0.707107f);
path.close();
path.moveTo(50.9992f, 25.2099f);
path.quadTo(50.9949f, 25.7358f, 50.9694f, 26.2608f);
path.conicTo(50.9209f, 27.2596f, 49.9221f, 27.2111f, 0.707107f);
path.conicTo(48.9233f, 27.1626f, 48.9718f, 26.1638f, 0.707107f);
path.quadTo(48.9953f, 25.679f, 48.9992f, 25.1938f);
path.conicTo(49.0073f, 24.1938f, 50.0073f, 24.2019f, 0.707107f);
path.conicTo(51.0072f, 24.21f, 50.9992f, 25.2099f, 0.707107f);
path.close();
path.moveTo(50.7839f, 28.3454f);
path.quadTo(50.7172f, 28.8596f, 50.63f, 29.3708f);
path.conicTo(50.4619f, 30.3565f, 49.4761f, 30.1884f, 0.707107f);
path.conicTo(48.4903f, 30.0203f, 48.6584f, 29.0346f, 0.707107f);
path.quadTo(48.7389f, 28.5627f, 48.8005f, 28.088f);
path.conicTo(48.9292f, 27.0963f, 49.9209f, 27.225f, 0.707107f);
path.conicTo(50.9126f, 27.3537f, 50.7839f, 28.3454f, 0.707107f);
path.close();
path.moveTo(50.1906f, 31.437f);
path.quadTo(50.0558f, 31.9646f, 49.899f, 32.4861f);
path.conicTo(49.611f, 33.4438f, 48.6534f, 33.1558f, 0.707107f);
path.conicTo(47.6957f, 32.8679f, 47.9837f, 31.9103f, 0.707107f);
path.quadTo(48.1284f, 31.4289f, 48.2528f, 30.9418f);
path.conicTo(48.5004f, 29.9729f, 49.4693f, 30.2205f, 0.707107f);
path.conicTo(50.4382f, 30.4681f, 50.1906f, 31.437f, 0.707107f);
path.close();
path.moveTo(49.1978f, 34.5114f);
path.quadTo(49.0051f, 35.0016f, 48.7927f, 35.4837f);
path.conicTo(48.3895f, 36.3988f, 47.4744f, 35.9956f, 0.707107f);
path.conicTo(46.5593f, 35.5923f, 46.9625f, 34.6772f, 0.707107f);
path.quadTo(47.1586f, 34.2323f, 47.3364f, 33.7797f);
path.conicTo(47.7023f, 32.849f, 48.6329f, 33.2149f, 0.707107f);
path.conicTo(49.5636f, 33.5807f, 49.1978f, 34.5114f, 0.707107f);
path.close();
path.moveTo(47.8852f, 37.3397f);
path.quadTo(47.6449f, 37.7853f, 47.3876f, 38.2211f);
path.conicTo(46.879f, 39.0821f, 46.018f, 38.5736f, 0.707107f);
path.conicTo(45.1569f, 38.0651f, 45.6655f, 37.204f, 0.707107f);
path.quadTo(45.903f, 36.8018f, 46.1248f, 36.3906f);
path.conicTo(46.5993f, 35.5103f, 47.4796f, 35.9849f, 0.707107f);
path.conicTo(48.3598f, 36.4595f, 47.8852f, 37.3397f, 0.707107f);
path.close();
path.moveTo(46.3154f, 39.8881f);
path.quadTo(46.0303f, 40.2962f, 45.7299f, 40.693f);
path.conicTo(45.1264f, 41.4903f, 44.3291f, 40.8867f, 0.707107f);
path.conicTo(43.5318f, 40.2831f, 44.1353f, 39.4858f, 0.707107f);
path.quadTo(44.4126f, 39.1195f, 44.6757f, 38.7428f);
path.conicTo(45.2483f, 37.923f, 46.0682f, 38.4956f, 0.707107f);
path.conicTo(46.888f, 39.0682f, 46.3154f, 39.8881f, 0.707107f);
path.close();
path.moveTo(44.4398f, 42.2654f);
path.quadTo(44.095f, 42.6536f, 43.7349f, 43.0278f);
path.conicTo(43.0415f, 43.7484f, 42.321f, 43.055f, 0.707107f);
path.conicTo(41.6004f, 42.3616f, 42.2938f, 41.641f, 0.707107f);
path.quadTo(42.6261f, 41.2957f, 42.9444f, 40.9374f);
path.conicTo(43.6084f, 40.1897f, 44.3561f, 40.8537f, 0.707107f);
path.conicTo(45.1038f, 41.5177f, 44.4398f, 42.2654f, 0.707107f);
path.close();
path.moveTo(42.2075f, 44.4911f);
path.quadTo(41.804f, 44.8473f, 41.3862f, 45.1865f);
path.conicTo(40.6098f, 45.8167f, 39.9795f, 45.0403f, 0.707107f);
path.conicTo(39.3493f, 44.2639f, 40.1257f, 43.6336f, 0.707107f);
path.quadTo(40.5114f, 43.3205f, 40.8838f, 42.9918f);
path.conicTo(41.6335f, 42.3299f, 42.2953f, 43.0796f, 0.707107f);
path.conicTo(42.9572f, 43.8292f, 42.2075f, 44.4911f, 0.707107f);
path.close();
path.moveTo(39.6379f, 46.488f);
path.quadTo(39.2151f, 46.776f, 38.7814f, 47.0471f);
path.conicTo(37.9334f, 47.5771f, 37.4034f, 46.7292f, 0.707107f);
path.conicTo(36.8733f, 45.8812f, 37.7213f, 45.3511f, 0.707107f);
path.quadTo(38.1217f, 45.1009f, 38.5119f, 44.835f);
path.conicTo(39.3383f, 44.2721f, 39.9013f, 45.0985f, 0.707107f);
path.conicTo(40.4643f, 45.925f, 39.6379f, 46.488f, 0.707107f);
path.close();
path.moveTo(36.9864f, 48.0722f);
path.quadTo(36.5234f, 48.3127f, 36.0513f, 48.5344f);
path.conicTo(35.1461f, 48.9595f, 34.7211f, 48.0543f, 0.707107f);
path.conicTo(34.296f, 47.1491f, 35.2012f, 46.7241f, 0.707107f);
path.quadTo(35.6371f, 46.5194f, 36.0644f, 46.2974f);
path.conicTo(36.9518f, 45.8364f, 37.4128f, 46.7238f, 0.707107f);
path.conicTo(37.8738f, 47.6112f, 36.9864f, 48.0722f, 0.707107f);
path.close();
path.moveTo(34.1153f, 49.3498f);
path.quadTo(33.6206f, 49.535f, 33.1187f, 49.6999f);
path.conicTo(32.1687f, 50.0122f, 31.8565f, 49.0622f, 0.707107f);
path.conicTo(31.5442f, 48.1122f, 32.4942f, 47.7999f, 0.707107f);
path.quadTo(32.9575f, 47.6477f, 33.4141f, 47.4767f);
path.conicTo(34.3507f, 47.1261f, 34.7012f, 48.0627f, 0.707107f);
path.conicTo(35.0518f, 48.9992f, 34.1153f, 49.3498f, 0.707107f);
path.close();
path.moveTo(31.08f, 50.2791f);
path.quadTo(30.5637f, 50.4033f, 30.0427f, 50.5063f);
path.conicTo(29.0617f, 50.7002f, 28.8678f, 49.7192f, 0.707107f);
path.conicTo(28.6738f, 48.7382f, 29.6548f, 48.5443f, 0.707107f);
path.quadTo(30.1357f, 48.4492f, 30.6122f, 48.3346f);
path.conicTo(31.5845f, 48.1007f, 31.8184f, 49.073f, 0.707107f);
path.conicTo(32.0522f, 50.0453f, 31.08f, 50.2791f, 0.707107f);
path.close();
path.moveTo(27.9769f, 50.829f);
path.quadTo(27.4588f, 50.8887f, 26.9386f, 50.9276f);
path.conicTo(25.9414f, 51.0022f, 25.8668f, 50.005f, 0.707107f);
path.conicTo(25.7923f, 49.0078f, 26.7895f, 48.9332f, 0.707107f);
path.quadTo(27.2696f, 48.8973f, 27.7479f, 48.8422f);
path.conicTo(28.7413f, 48.7277f, 28.8558f, 49.7211f, 0.707107f);
path.conicTo(28.9703f, 50.7145f, 27.9769f, 50.829f, 0.707107f);
path.close();
path.moveTo(24.8625f, 50.9996f);
path.quadTo(24.3373f, 50.9969f, 23.8128f, 50.9729f);
path.conicTo(22.8138f, 50.9272f, 22.8595f, 49.9283f, 0.707107f);
path.conicTo(22.9051f, 48.9293f, 23.9041f, 48.975f, 0.707107f);
path.quadTo(24.3884f, 48.9971f, 24.8731f, 48.9997f);
path.conicTo(25.8731f, 49.005f, 25.8678f, 50.005f, 0.707107f);
path.conicTo(25.8624f, 51.0049f, 24.8625f, 50.9996f, 0.707107f);
path.close();
path.moveTo(21.7268f, 50.7931f);
path.quadTo(21.2121f, 50.7278f, 20.7005f, 50.642f);
path.conicTo(19.7143f, 50.4767f, 19.8796f, 49.4905f, 0.707107f);
path.conicTo(20.045f, 48.5042f, 21.0312f, 48.6696f, 0.707107f);
path.quadTo(21.5036f, 48.7488f, 21.9786f, 48.8091f);
path.conicTo(22.9707f, 48.9349f, 22.8448f, 49.927f, 0.707107f);
path.conicTo(22.7189f, 50.919f, 21.7268f, 50.7931f, 0.707107f);
path.close();
path.moveTo(18.6372f, 50.2094f);
path.quadTo(18.1089f, 50.0761f, 17.5865f, 49.9207f);
path.conicTo(16.628f, 49.6356f, 16.9132f, 48.6771f, 0.707107f);
path.conicTo(17.1983f, 47.7186f, 18.1568f, 48.0037f, 0.707107f);
path.quadTo(18.639f, 48.1472f, 19.1267f, 48.2702f);
path.conicTo(20.0963f, 48.515f, 19.8516f, 49.4846f, 0.707107f);
path.conicTo(19.6068f, 50.4542f, 18.6372f, 50.2094f, 0.707107f);
path.close();
path.moveTo(15.5577f, 49.2248f);
path.quadTo(15.0665f, 49.0334f, 14.5834f, 48.8222f);
path.conicTo(13.6672f, 48.4215f, 14.0678f, 47.5053f, 0.707107f);
path.conicTo(14.4684f, 46.589f, 15.3847f, 46.9897f, 0.707107f);
path.quadTo(15.8306f, 47.1846f, 16.284f, 47.3614f);
path.conicTo(17.2158f, 47.7246f, 16.8526f, 48.6563f, 0.707107f);
path.conicTo(16.4894f, 49.588f, 15.5577f, 49.2248f, 0.707107f);
path.close();
path.moveTo(12.7231f, 47.9189f);
path.quadTo(12.2765f, 47.6797f, 11.8395f, 47.4233f);
path.conicTo(10.9771f, 46.9171f, 11.4833f, 46.0547f, 0.707107f);
path.conicTo(11.9894f, 45.1922f, 12.8519f, 45.6984f, 0.707107f);
path.quadTo(13.2552f, 45.9351f, 13.6675f, 46.156f);
path.conicTo(14.549f, 46.6282f, 14.0768f, 47.5096f, 0.707107f);
path.conicTo(13.6046f, 48.3911f, 12.7231f, 47.9189f, 0.707107f);
path.close();
path.moveTo(10.1686f, 46.3548f);
path.quadTo(9.76024f, 46.0712f, 9.363f, 45.7722f);
path.conicTo(8.56406f, 45.1708f, 9.16549f, 44.3718f, 0.707107f);
path.conicTo(9.76691f, 43.5729f, 10.5658f, 44.1743f, 0.707107f);
path.quadTo(10.9325f, 44.4504f, 11.3095f, 44.7122f);
path.conicTo(12.1308f, 45.2826f, 11.5604f, 46.1039f, 0.707107f);
path.conicTo(10.9899f, 46.9253f, 10.1686f, 46.3548f, 0.707107f);
path.close();
path.moveTo(7.78853f, 44.4876f);
path.quadTo(7.39972f, 44.1442f, 7.02492f, 43.7855f);
path.conicTo(6.3024f, 43.0942f, 6.99374f, 42.3717f, 0.707107f);
path.conicTo(7.68509f, 41.6492f, 8.40761f, 42.3405f, 0.707107f);
path.quadTo(8.7536f, 42.6715f, 9.11249f, 42.9885f);
path.conicTo(9.86201f, 43.6505f, 9.20003f, 44.4f, 0.707107f);
path.conicTo(8.53805f, 45.1496f, 7.78853f, 44.4876f, 0.707107f);
path.close();
path.moveTo(5.55855f, 42.2635f);
path.quadTo(5.20148f, 41.8614f, 4.86131f, 41.4449f);
path.conicTo(4.22883f, 40.6703f, 5.0034f, 40.0378f, 0.707107f);
path.conicTo(5.77797f, 39.4053f, 6.41046f, 40.1799f, 0.707107f);
path.quadTo(6.72443f, 40.5644f, 7.05403f, 40.9356f);
path.conicTo(7.71802f, 41.6833f, 6.97028f, 42.3473f, 0.707107f);
path.conicTo(6.22254f, 43.0113f, 5.55855f, 42.2635f, 0.707107f);
path.close();
path.moveTo(3.55261f, 39.6973f);
path.quadTo(3.26341f, 39.2752f, 2.99107f, 38.8422f);
path.conicTo(2.45867f, 37.9957f, 3.30517f, 37.4633f, 0.707107f);
path.conicTo(4.15167f, 36.9309f, 4.68406f, 37.7774f, 0.707107f);
path.quadTo(4.93548f, 38.1772f, 5.20241f, 38.5667f);
path.conicTo(5.76769f, 39.3916f, 4.94279f, 39.9569f, 0.707107f);
path.conicTo(4.11789f, 40.5222f, 3.55261f, 39.6973f, 0.707107f);
path.close();
path.moveTo(1.96145f, 37.0509f);
path.quadTo(1.71975f, 36.5889f, 1.49677f, 36.1175f);
path.conicTo(1.06917f, 35.2135f, 1.97315f, 34.7859f, 0.707107f);
path.conicTo(2.87712f, 34.3583f, 3.30471f, 35.2623f, 0.707107f);
path.quadTo(3.51053f, 35.6974f, 3.73364f, 36.1239f);
path.conicTo(4.19714f, 37.01f, 3.31105f, 37.4735f, 0.707107f);
path.conicTo(2.42495f, 37.937f, 1.96145f, 37.0509f, 0.707107f);
path.close();
path.moveTo(0.676191f, 34.1844f);
path.quadTo(0.489621f, 33.6902f, 0.323275f, 33.189f);
path.conicTo(0.00831527f, 32.2399f, 0.95742f, 31.9249f, 0.707107f);
path.conicTo(1.90653f, 31.6099f, 2.22149f, 32.559f, 0.707107f);
path.quadTo(2.37504f, 33.0218f, 2.54726f, 33.4779f);
path.conicTo(2.9005f, 34.4134f, 1.96497f, 34.7666f, 0.707107f);
path.conicTo(1.02943f, 35.1199f, 0.676191f, 34.1844f, 0.707107f);
path.close();
path.moveTo(-0.261658f, 31.1521f);
path.quadTo(-0.387304f, 30.6362f, -0.491779f, 30.1156f);
path.conicTo(-0.68853f, 29.1351f, 0.291923f, 28.9384f, 0.707107f);
path.conicTo(1.27238f, 28.7416f, 1.46913f, 29.7221f, 0.707107f);
path.quadTo(1.56557f, 30.2026f, 1.68155f, 30.6789f);
path.conicTo(1.91817f, 31.6505f, 0.946565f, 31.8871f, 0.707107f);
path.conicTo(-0.0250367f, 32.1237f, -0.261658f, 31.1521f, 0.707107f);
path.close();
path.moveTo(-0.820549f, 28.0495f);
path.quadTo(-0.881733f, 27.5314f, -0.922089f, 27.0113f);
path.conicTo(-0.999449f, 26.0143f, -0.00244591f, 25.9369f, 0.707107f);
path.conicTo(0.994557f, 25.8596f, 1.07192f, 26.8566f, 0.707107f);
path.quadTo(1.10917f, 27.3367f, 1.16565f, 27.8149f);
path.conicTo(1.28293f, 28.808f, 0.289834f, 28.9253f, 0.707107f);
path.conicTo(-0.703265f, 29.0426f, -0.820549f, 28.0495f, 0.707107f);
path.close();
path.moveTo(-0.999918f, 24.9349f);
path.quadTo(-0.998605f, 24.4104f, -0.976138f, 23.8863f);
path.conicTo(-0.933305f, 22.8873f, 0.0657772f, 22.9301f, 0.707107f);
path.conicTo(1.06486f, 22.9729f, 1.02203f, 23.972f, 0.707107f);
path.quadTo(1.00129f, 24.4557f, 1.00008f, 24.9399f);
path.conicTo(0.997572f, 25.9399f, -0.0024244f, 25.9374f, 0.707107f);
path.conicTo(-1.00242f, 25.9349f, -0.999918f, 24.9349f, 0.707107f);
path.close();
path.moveTo(-0.802212f, 21.7991f);
path.quadTo(-0.738311f, 21.284f, -0.653903f, 20.7719f);
path.conicTo(-0.491283f, 19.7852f, 0.495406f, 19.9478f, 0.707107f);
path.conicTo(1.48209f, 20.1104f, 1.31948f, 21.0971f, 0.707107f);
path.quadTo(1.24156f, 21.5698f, 1.18257f, 22.0453f);
path.conicTo(1.05946f, 23.0377f, 0.0670681f, 22.9146f, 0.707107f);
path.conicTo(-0.925325f, 22.7915f, -0.802212f, 21.7991f, 0.707107f);
path.close();
path.moveTo(-0.228066f, 18.7115f);
path.quadTo(-0.096172f, 18.1824f, 0.0577899f, 17.6593f);
path.conicTo(0.340124f, 16.7f, 1.29944f, 16.9823f, 0.707107f);
path.conicTo(2.25876f, 17.2646f, 1.97642f, 18.2239f, 0.707107f);
path.quadTo(1.8343f, 18.7068f, 1.71255f, 19.1953f);
path.conicTo(1.47069f, 20.1656f, 0.50038f, 19.9237f, 0.707107f);
path.conicTo(-0.46993f, 19.6819f, -0.228066f, 18.7115f, 0.707107f);
path.close();
path.moveTo(0.74831f, 15.6269f);
path.quadTo(0.938539f, 15.1347f, 1.14857f, 14.6506f);
path.conicTo(1.54662f, 13.7333f, 2.46398f, 14.1313f, 0.707107f);
path.conicTo(3.38135f, 14.5294f, 2.9833f, 15.4467f, 0.707107f);
path.quadTo(2.78942f, 15.8936f, 2.61382f, 16.3479f);
path.conicTo(2.25331f, 17.2806f, 1.32056f, 16.9201f, 0.707107f);
path.conicTo(0.387801f, 16.5596f, 0.74831f, 15.6269f, 0.707107f);
path.close();
path.moveTo(2.04744f, 12.7861f);
path.quadTo(2.28569f, 12.3384f, 2.5412f, 11.9003f);
path.conicTo(3.04504f, 11.0365f, 3.90884f, 11.5403f, 0.707107f);
path.conicTo(4.77264f, 12.0442f, 4.26881f, 12.908f, 0.707107f);
path.quadTo(4.03293f, 13.3123f, 3.81302f, 13.7256f);
path.conicTo(3.34325f, 14.6084f, 2.46046f, 14.1386f, 0.707107f);
path.conicTo(1.57767f, 13.6689f, 2.04744f, 12.7861f, 0.707107f);
path.close();
path.moveTo(3.60589f, 10.2253f);
path.quadTo(3.88812f, 9.81661f, 4.18576f, 9.419f);
path.conicTo(4.78503f, 8.61845f, 5.58558f, 9.21772f, 0.707107f);
path.conicTo(6.38613f, 9.81699f, 5.78686f, 10.6175f, 0.707107f);
path.quadTo(5.51211f, 10.9846f, 5.25159f, 11.3618f);
path.conicTo(4.68333f, 12.1847f, 3.86048f, 11.6164f, 0.707107f);
path.conicTo(3.03763f, 11.0481f, 3.60589f, 10.2253f, 0.707107f);
path.close();
path.moveTo(5.46482f, 7.84259f);
path.quadTo(5.80682f, 7.4532f, 6.16407f, 7.07773f);
path.conicTo(6.85339f, 6.35327f, 7.57785f, 7.04259f, 0.707107f);
path.conicTo(8.30231f, 7.73191f, 7.61299f, 8.45636f, 0.707107f);
path.quadTo(7.28322f, 8.80295f, 6.96752f, 9.16239f);
path.conicTo(6.30762f, 9.91375f, 5.55627f, 9.25385f, 0.707107f);
path.conicTo(4.80492f, 8.59395f, 5.46482f, 7.84259f, 0.707107f);
path.close();
path.moveTo(7.68062f, 5.60827f);
path.quadTo(8.08142f, 5.25031f, 8.49666f, 4.90921f);
path.conicTo(9.26938f, 4.27447f, 9.90412f, 5.04719f, 0.707107f);
path.conicTo(10.5389f, 5.81992f, 9.76614f, 6.45466f, 0.707107f);
path.quadTo(9.38285f, 6.76951f, 9.01289f, 7.09994f);
path.conicTo(8.26705f, 7.76607f, 7.60092f, 7.02024f, 0.707107f);
path.conicTo(6.93479f, 6.2744f, 7.68062f, 5.60827f, 0.707107f);
path.close();
path.moveTo(10.2392f, 3.59627f);
path.quadTo(10.6626f, 3.30433f, 11.0971f, 3.02935f);
path.conicTo(11.9421f, 2.49463f, 12.4768f, 3.33965f, 0.707107f);
path.conicTo(13.0116f, 4.18467f, 12.1666f, 4.7194f, 0.707107f);
path.quadTo(11.7654f, 4.97322f, 11.3747f, 5.24271f);
path.conicTo(10.5515f, 5.81043f, 9.98373f, 4.98721f, 0.707107f);
path.conicTo(9.41601f, 4.16399f, 10.2392f, 3.59627f, 0.707107f);
path.close();
path.moveTo(12.8847f, 1.99524f);
path.quadTo(13.3459f, 1.75234f, 13.8165f, 1.52812f);
path.conicTo(14.7193f, 1.09799f, 15.1494f, 2.00075f, 0.707107f);
path.conicTo(15.5795f, 2.90352f, 14.6768f, 3.33365f, 0.707107f);
path.quadTo(14.2424f, 3.54063f, 13.8166f, 3.76484f);
path.conicTo(12.9318f, 4.23081f, 12.4658f, 3.34601f, 0.707107f);
path.conicTo(11.9999f, 2.46122f, 12.8847f, 1.99524f, 0.707107f);
path.close();
path.moveTo(15.7467f, 0.702339f);
path.quadTo(16.2402f, 0.514409f, 16.7409f, 0.346672f);
path.conicTo(17.6891f, 0.029011f, 18.0067f, 0.977215f, 0.707107f);
path.conicTo(18.3244f, 1.92542f, 17.3762f, 2.24308f, 0.707107f);
path.quadTo(16.914f, 2.39792f, 16.4585f, 2.57139f);
path.conicTo(15.524f, 2.92729f, 15.1681f, 1.99276f, 0.707107f);
path.conicTo(14.8122f, 1.05824f, 15.7467f, 0.702339f, 0.707107f);
path.close();
path.moveTo(18.7758f, -0.24399f);
path.quadTo(19.2913f, -0.371107f, 19.8116f, -0.477061f);
path.conicTo(20.7915f, -0.676608f, 20.9911f, 0.303281f, 0.707107f);
path.conicTo(21.1906f, 1.28317f, 20.2107f, 1.48272f, 0.707107f);
path.quadTo(19.7304f, 1.58052f, 19.2546f, 1.69785f);
path.conicTo(18.2836f, 1.93725f, 18.0443f, 0.966329f, 0.707107f);
path.conicTo(17.8049f, -0.00459272f, 18.7758f, -0.24399f, 0.707107f);
path.close();
path.moveTo(21.878f, -0.811882f);
path.quadTo(22.396f, -0.874528f, 22.916f, -0.916348f);
path.conicTo(23.9128f, -0.996504f, 23.993f, 0.000278629f, 0.707107f);
path.conicTo(24.0731f, 0.997061f, 23.0764f, 1.07722f, 0.707107f);
path.quadTo(22.5963f, 1.11582f, 22.1182f, 1.17365f);
path.conicTo(21.1254f, 1.29372f, 21.0053f, 0.300958f, 0.707107f);
path.conicTo(20.8853f, -0.691807f, 21.878f, -0.811882f, 0.707107f);
path.close();
path.moveTo(24.9926f, -0.999999f);
path.quadTo(25.5166f, -1.00015f, 26.0401f, -0.979188f);
path.conicTo(27.0393f, -0.939179f, 26.9992f, 0.0600199f, 0.707107f);
path.conicTo(26.9592f, 1.05922f, 25.96f, 1.01921f, 0.707107f);
path.quadTo(25.4768f, 0.999863f, 24.9932f, 1);
path.conicTo(23.9932f, 1.00029f, 23.9929f, 0.000287339f, 0.707107f);
path.conicTo(23.9926f, -0.999713f, 24.9926f, -0.999999f, 0.707107f);
path.close();
path.moveTo(28.1286f, -0.811081f);
path.quadTo(28.6441f, -0.748593f, 29.1567f, -0.665572f);
path.conicTo(30.1439f, -0.505698f, 29.984f, 0.48144f, 0.707107f);
path.conicTo(29.8241f, 1.46858f, 28.837f, 1.3087f, 0.707107f);
path.quadTo(28.3638f, 1.23207f, 27.8879f, 1.17439f);
path.conicTo(26.8952f, 1.05406f, 27.0155f, 0.0613233f, 0.707107f);
path.conicTo(27.1359f, -0.931411f, 28.1286f, -0.811081f, 0.707107f);
path.close();
path.moveTo(31.214f, -0.246499f);
path.quadTo(31.7439f, -0.116076f, 32.2679f, 0.0364622f);
path.conicTo(33.228f, 0.315996f, 32.9485f, 1.27613f, 0.707107f);
path.conicTo(32.6689f, 2.23627f, 31.7088f, 1.95673f, 0.707107f);
path.quadTo(31.2252f, 1.81593f, 30.736f, 1.69554f);
path.conicTo(29.765f, 1.45654f, 30.004f, 0.48552f, 0.707107f);
path.conicTo(30.243f, -0.485499f, 31.214f, -0.246499f, 0.707107f);
path.close();
path.moveTo(34.3038f, 0.721629f);
path.quadTo(34.797f, 0.910612f, 35.282f, 1.11946f);
path.conicTo(36.2005f, 1.51493f, 35.805f, 2.43341f, 0.707107f);
path.conicTo(35.4096f, 3.35189f, 34.4911f, 2.95642f, 0.707107f);
path.quadTo(34.0434f, 2.76365f, 33.5881f, 2.5892f);
path.conicTo(32.6543f, 2.23137f, 33.0122f, 1.29758f, 0.707107f);
path.conicTo(33.37f, 0.363796f, 34.3038f, 0.721629f, 0.707107f);
path.close();
path.moveTo(37.1508f, 2.01396f);
path.quadTo(37.5996f, 2.2512f, 38.0388f, 2.50578f);
path.conicTo(38.904f, 3.00727f, 38.4025f, 3.87244f, 0.707107f);
path.conicTo(37.901f, 4.7376f, 37.0358f, 4.23612f, 0.707107f);
path.quadTo(36.6304f, 4.00111f, 36.2161f, 3.78211f);
path.conicTo(35.332f, 3.31476f, 35.7994f, 2.43069f, 0.707107f);
path.conicTo(36.2667f, 1.54661f, 37.1508f, 2.01396f, 0.707107f);
path.close();
path.moveTo(39.718f, 3.56681f);
path.quadTo(40.1269f, 3.84765f, 40.5249f, 4.14392f);
path.conicTo(41.3271f, 4.74104f, 40.73f, 5.54319f, 0.707107f);
path.conicTo(40.1329f, 6.34535f, 39.3307f, 5.74823f, 0.707107f);
path.quadTo(38.9634f, 5.47478f, 38.5858f, 5.21552f);
path.conicTo(37.7615f, 4.64945f, 38.3275f, 3.82509f, 0.707107f);
path.conicTo(38.8936f, 3.00074f, 39.718f, 3.56681f, 0.707107f);
path.close();
path.moveTo(42.1033f, 5.41741f);
path.quadTo(42.4933f, 5.75802f, 42.8694f, 6.11388f);
path.conicTo(43.5958f, 6.80115f, 42.9085f, 7.52755f, 0.707107f);
path.conicTo(42.2212f, 8.25394f, 41.4948f, 7.56667f, 0.707107f);
path.quadTo(41.1476f, 7.23817f, 40.7876f, 6.92375f);
path.conicTo(40.0345f, 6.26593f, 40.6923f, 5.51275f, 0.707107f);
path.conicTo(41.3501f, 4.75958f, 42.1033f, 5.41741f, 0.707107f);
path.close();
path.moveTo(44.3419f, 7.62498f);
path.quadTo(44.7007f, 8.02444f, 45.0428f, 8.43835f);
path.conicTo(45.6797f, 9.20922f, 44.9089f, 9.84622f, 0.707107f);
path.conicTo(44.138f, 10.4832f, 43.501f, 9.71234f, 0.707107f);
path.quadTo(43.1852f, 9.3302f, 42.854f, 8.96151f);
path.conicTo(42.1858f, 8.21759f, 42.9297f, 7.54932f, 0.707107f);
path.conicTo(43.6736f, 6.88106f, 44.3419f, 7.62498f, 0.707107f);
path.close();
path.moveTo(46.3599f, 10.1759f);
path.quadTo(46.6546f, 10.6005f, 46.9322f, 11.0366f);
path.conicTo(47.4693f, 11.8801f, 46.6257f, 12.4172f, 0.707107f);
path.conicTo(45.7822f, 12.9542f, 45.2451f, 12.1107f, 0.707107f);
path.quadTo(44.9889f, 11.7082f, 44.7168f, 11.3162f);
path.conicTo(44.1467f, 10.4947f, 44.9682f, 9.92452f, 0.707107f);
path.conicTo(45.7897f, 9.35435f, 46.3599f, 10.1759f, 0.707107f);
path.close();
path.moveTo(47.9708f, 12.8204f);
path.quadTo(48.2149f, 13.2808f, 48.4403f, 13.7506f);
path.conicTo(48.873f, 14.6521f, 47.9715f, 15.0848f, 0.707107f);
path.conicTo(47.0699f, 15.5174f, 46.6372f, 14.6159f, 0.707107f);
path.quadTo(46.4291f, 14.1822f, 46.2038f, 13.7573f);
path.conicTo(45.7354f, 12.8738f, 46.6188f, 12.4054f, 0.707107f);
path.conicTo(47.5023f, 11.9369f, 47.9708f, 12.8204f, 0.707107f);
path.close();
path.moveTo(49.2713f, 15.6778f);
path.quadTo(49.4606f, 16.1706f, 49.6297f, 16.6708f);
path.conicTo(49.9501f, 17.6181f, 49.0028f, 17.9384f, 0.707107f);
path.conicTo(48.0555f, 18.2588f, 47.7351f, 17.3115f, 0.707107f);
path.quadTo(47.5791f, 16.8499f, 47.4043f, 16.3949f);
path.conicTo(47.0458f, 15.4614f, 47.9793f, 15.1029f, 0.707107f);
path.conicTo(48.9128f, 14.7443f, 49.2713f, 15.6778f, 0.707107f);
path.close();
path.moveTo(50.2261f, 18.7037f);
path.quadTo(50.3547f, 19.2188f, 50.4621f, 19.7388f);
path.conicTo(50.6645f, 20.7182f, 49.6852f, 20.9205f, 0.707107f);
path.conicTo(48.7059f, 21.1229f, 48.5035f, 20.1436f, 0.707107f);
path.quadTo(48.4043f, 19.6636f, 48.2856f, 19.1881f);
path.conicTo(48.0435f, 18.2178f, 49.0137f, 17.9757f, 0.707107f);
path.conicTo(49.984f, 17.7335f, 50.2261f, 18.7037f, 0.707107f);
path.close();
path.moveTo(50.803f, 21.8055f);
path.quadTo(50.8671f, 22.3234f, 50.9104f, 22.8434f);
path.conicTo(50.9934f, 23.8399f, 49.9968f, 23.9229f, 0.707107f);
path.conicTo(49.0002f, 24.0058f, 48.9173f, 23.0093f, 0.707107f);
path.quadTo(48.8773f, 22.5293f, 48.8182f, 22.0513f);
path.conicTo(48.6953f, 21.0588f, 49.6877f, 20.936f, 0.707107f);
path.conicTo(50.6801f, 20.8131f, 50.803f, 21.8055f, 0.707107f);
path.close();
path.moveTo(50.9999f, 24.9202f);
path.quadTo(51.0015f, 25.4434f, 50.982f, 25.9664f);
path.conicTo(50.9449f, 26.9657f, 49.9456f, 26.9286f, 0.707107f);
path.conicTo(48.9463f, 26.8914f, 48.9834f, 25.8921f, 0.707107f);
path.quadTo(49.0014f, 25.4094f, 48.9999f, 24.9263f);
path.conicTo(48.9968f, 23.9263f, 49.9968f, 23.9232f, 0.707107f);
path.conicTo(50.9968f, 23.9202f, 50.9999f, 24.9202f, 0.707107f);
path.close();
path.moveTo(50.8198f, 28.0562f);
path.quadTo(50.7587f, 28.5721f, 50.677f, 29.0852f);
path.conicTo(50.5199f, 30.0728f, 49.5323f, 29.9157f, 0.707107f);
path.conicTo(48.5448f, 29.7586f, 48.7019f, 28.771f, 0.707107f);
path.quadTo(48.7772f, 28.2974f, 48.8336f, 27.8211f);
path.conicTo(48.9512f, 26.8281f, 49.9442f, 26.9456f, 0.707107f);
path.conicTo(50.9373f, 27.0632f, 50.8198f, 28.0562f, 0.707107f);
path.close();
path.moveTo(50.2647f, 31.1395f);
path.quadTo(50.1358f, 31.6701f, 49.9847f, 32.1949f);
path.conicTo(49.7079f, 33.1558f, 48.747f, 32.8791f, 0.707107f);
path.conicTo(47.786f, 32.6024f, 48.0628f, 31.6414f, 0.707107f);
path.quadTo(48.2022f, 31.1571f, 48.3213f, 30.6672f);
path.conicTo(48.5574f, 29.6955f, 49.5291f, 29.9317f, 0.707107f);
path.conicTo(50.5009f, 30.1678f, 50.2647f, 31.1395f, 0.707107f);
path.close();
path.moveTo(49.3049f, 34.2343f);
path.quadTo(49.1171f, 34.7285f, 48.9095f, 35.2145f);
path.conicTo(48.5166f, 36.1341f, 47.597f, 35.7412f, 0.707107f);
path.conicTo(46.6774f, 35.3483f, 47.0703f, 34.4288f, 0.707107f);
path.quadTo(47.262f, 33.9801f, 47.4353f, 33.524f);
path.conicTo(47.7904f, 32.5892f, 48.7252f, 32.9444f, 0.707107f);
path.conicTo(49.66f, 33.2995f, 49.3049f, 34.2343f, 0.707107f);
path.close();
path.moveTo(48.0194f, 37.0875f);
path.quadTo(47.7831f, 37.5374f, 47.5295f, 37.9777f);
path.conicTo(47.0304f, 38.8443f, 46.1638f, 38.3451f, 0.707107f);
path.conicTo(45.2973f, 37.846f, 45.7965f, 36.9795f, 0.707107f);
path.quadTo(46.0306f, 36.5729f, 46.2487f, 36.1577f);
path.conicTo(46.7136f, 35.2723f, 47.5989f, 35.7372f, 0.707107f);
path.conicTo(48.4843f, 36.2021f, 48.0194f, 37.0875f, 0.707107f);
path.close();
path.moveTo(46.4721f, 39.6612f);
path.quadTo(46.1926f, 40.0705f, 45.8977f, 40.4688f);
path.conicTo(45.3028f, 41.2726f, 44.499f, 40.6776f, 0.707107f);
path.conicTo(43.6953f, 40.0827f, 44.2902f, 39.2789f, 0.707107f);
path.quadTo(44.5624f, 38.9112f, 44.8204f, 38.5334f);
path.conicTo(45.3843f, 37.7075f, 46.2101f, 38.2714f, 0.707107f);
path.conicTo(47.036f, 38.8353f, 46.4721f, 39.6612f, 0.707107f);
path.close();
path.moveTo(44.6298f, 42.0491f);
path.quadTo(44.2906f, 42.4396f, 43.9361f, 42.8164f);
path.conicTo(43.2509f, 43.5447f, 42.5226f, 42.8595f, 0.707107f);
path.conicTo(41.7942f, 42.1742f, 42.4795f, 41.4459f, 0.707107f);
path.quadTo(42.8067f, 41.0981f, 43.1198f, 40.7376f);
path.conicTo(43.7756f, 39.9826f, 44.5306f, 40.6383f, 0.707107f);
path.conicTo(45.2856f, 41.2941f, 44.6298f, 42.0491f, 0.707107f);
path.close();
path.moveTo(42.4305f, 44.2919f);
path.quadTo(42.0324f, 44.6516f, 41.6198f, 44.9946f);
path.conicTo(40.8507f, 45.6338f, 40.2115f, 44.8648f, 0.707107f);
path.conicTo(39.5723f, 44.0958f, 40.3413f, 43.4566f, 0.707107f);
path.quadTo(40.7222f, 43.1399f, 41.0897f, 42.8079f);
path.conicTo(41.8317f, 42.1375f, 42.5021f, 42.8795f, 0.707107f);
path.conicTo(43.1725f, 43.6215f, 42.4305f, 44.2919f, 0.707107f);
path.close();
path.moveTo(39.8873f, 46.3159f);
path.quadTo(39.4613f, 46.6134f, 39.0238f, 46.8936f);
path.conicTo(38.1818f, 47.433f, 37.6424f, 46.5909f, 0.707107f);
path.conicTo(37.103f, 45.7489f, 37.9451f, 45.2095f, 0.707107f);
path.quadTo(38.3489f, 44.9508f, 38.7421f, 44.6763f);
path.conicTo(39.5619f, 44.1037f, 40.1345f, 44.9235f, 0.707107f);
path.conicTo(40.7071f, 45.7434f, 39.8873f, 46.3159f, 0.707107f);
path.close();
path.moveTo(37.2437f, 47.9367f);
path.quadTo(36.7842f, 48.182f, 36.3153f, 48.4086f);
path.conicTo(35.415f, 48.8439f, 34.9797f, 47.9435f, 0.707107f);
path.conicTo(34.5445f, 47.0432f, 35.4449f, 46.608f, 0.707107f);
path.quadTo(35.8778f, 46.3987f, 36.3019f, 46.1723f);
path.conicTo(37.1841f, 45.7014f, 37.655f, 46.5836f, 0.707107f);
path.conicTo(38.1259f, 47.4658f, 37.2437f, 47.9367f, 0.707107f);
path.close();
path.moveTo(34.3909f, 49.2448f);
path.quadTo(33.8988f, 49.4354f, 33.3992f, 49.606f);
path.conicTo(32.4528f, 49.929f, 32.1298f, 48.9826f, 0.707107f);
path.conicTo(31.8068f, 48.0362f, 32.7532f, 47.7132f, 0.707107f);
path.quadTo(33.2142f, 47.5558f, 33.6685f, 47.3798f);
path.conicTo(34.601f, 47.0186f, 34.9622f, 47.9511f, 0.707107f);
path.conicTo(35.3234f, 48.8836f, 34.3909f, 49.2448f, 0.707107f);
path.close();
path.moveTo(31.3682f, 50.208f);
path.quadTo(30.8535f, 50.3381f, 30.3338f, 50.447f);
path.conicTo(29.3551f, 50.6521f, 29.15f, 49.6734f, 0.707107f);
path.conicTo(28.9448f, 48.6947f, 29.9236f, 48.4895f, 0.707107f);
path.quadTo(30.4033f, 48.389f, 30.8784f, 48.269f);
path.conicTo(31.8479f, 48.024f, 32.0929f, 48.9936f, 0.707107f);
path.conicTo(32.3378f, 49.9631f, 31.3682f, 50.208f, 0.707107f);
path.close();
path.moveTo(28.2669f, 50.7939f);
path.quadTo(27.7491f, 50.8595f, 27.2292f, 50.9043f);
path.conicTo(26.2329f, 50.99f, 26.1472f, 49.9937f, 0.707107f);
path.conicTo(26.0615f, 48.9973f, 27.0578f, 48.9116f, 0.707107f);
path.quadTo(27.5378f, 48.8703f, 28.0156f, 48.8098f);
path.conicTo(29.0077f, 48.6841f, 29.1334f, 49.6762f, 0.707107f);
path.conicTo(29.259f, 50.6683f, 28.2669f, 50.7939f, 0.707107f);
path.close();
path.moveTo(25.1523f, 50.9996f);
path.quadTo(24.6297f, 51.0026f, 24.1072f, 50.9847f);
path.conicTo(23.1078f, 50.9503f, 23.1422f, 49.9509f, 0.707107f);
path.conicTo(23.1765f, 48.9515f, 24.1759f, 48.9858f, 0.707107f);
path.quadTo(24.658f, 49.0024f, 25.1406f, 48.9996f);
path.conicTo(26.1406f, 48.9937f, 26.1464f, 49.9937f, 0.707107f);
path.conicTo(26.1523f, 50.9937f, 25.1523f, 50.9996f, 0.707107f);
path.close();
path.moveTo(22.0162f, 50.8282f);
path.quadTo(21.4999f, 50.7686f, 20.9863f, 50.6883f);
path.conicTo(19.9983f, 50.5339f, 20.1527f, 49.5459f, 0.707107f);
path.conicTo(20.307f, 48.5579f, 21.295f, 48.7123f, 0.707107f);
path.quadTo(21.7691f, 48.7864f, 22.2457f, 48.8414f);
path.conicTo(23.2391f, 48.9562f, 23.1243f, 49.9496f, 0.707107f);
path.conicTo(23.0096f, 50.943f, 22.0162f, 50.8282f, 0.707107f);
path.close();
path.moveTo(18.9351f, 50.2827f);
path.quadTo(18.4037f, 50.1553f, 17.8782f, 50.0056f);
path.conicTo(16.9164f, 49.7317f, 17.1904f, 48.7699f, 0.707107f);
path.conicTo(17.4643f, 47.8082f, 18.426f, 48.0821f, 0.707107f);
path.quadTo(18.9112f, 48.2203f, 19.4016f, 48.3379f);
path.conicTo(20.374f, 48.5712f, 20.1408f, 49.5436f, 0.707107f);
path.conicTo(19.9075f, 50.516f, 18.9351f, 50.2827f, 0.707107f);
path.close();
path.moveTo(15.8352f, 49.3312f);
path.quadTo(15.3403f, 49.1448f, 14.8531f, 48.9383f);
path.conicTo(13.9324f, 48.548f, 14.3227f, 47.6273f, 0.707107f);
path.conicTo(14.713f, 46.7066f, 15.6337f, 47.0969f, 0.707107f);
path.quadTo(16.0832f, 47.2874f, 16.5402f, 47.4596f);
path.conicTo(17.476f, 47.812f, 17.1235f, 48.7479f, 0.707107f);
path.conicTo(16.771f, 49.6837f, 15.8352f, 49.3312f, 0.707107f);
path.close();
path.moveTo(12.9759f, 48.0526f);
path.quadTo(12.5249f, 47.8173f, 12.0835f, 47.5647f);
path.conicTo(11.2156f, 47.0679f, 11.7124f, 46.2f, 0.707107f);
path.conicTo(12.2092f, 45.3321f, 13.0771f, 45.8289f, 0.707107f);
path.quadTo(13.4846f, 46.0622f, 13.9009f, 46.2793f);
path.conicTo(14.7875f, 46.7418f, 14.325f, 47.6284f, 0.707107f);
path.conicTo(13.8626f, 48.5151f, 12.9759f, 48.0526f, 0.707107f);
path.close();
path.moveTo(10.3957f, 46.5108f);
path.quadTo(9.9861f, 46.2327f, 9.58733f, 45.9392f);
path.conicTo(8.78198f, 45.3464f, 9.37478f, 44.541f, 0.707107f);
path.conicTo(9.96757f, 43.7357f, 10.7729f, 44.3285f, 0.707107f);
path.quadTo(11.141f, 44.5994f, 11.5191f, 44.8561f);
path.conicTo(12.3464f, 45.4178f, 11.7847f, 46.2451f, 0.707107f);
path.conicTo(11.223f, 47.0725f, 10.3957f, 46.5108f, 0.707107f);
path.close();
path.moveTo(8.00525f, 44.6769f);
path.quadTo(7.6141f, 44.339f, 7.23672f, 43.9859f);
path.conicTo(6.50649f, 43.3027f, 7.18969f, 42.5725f, 0.707107f);
path.conicTo(7.87289f, 41.8423f, 8.60312f, 42.5255f, 0.707107f);
path.quadTo(8.95149f, 42.8514f, 9.31254f, 43.1632f);
path.conicTo(10.0693f, 43.8169f, 9.4157f, 44.5737f, 0.707107f);
path.conicTo(8.76206f, 45.3305f, 8.00525f, 44.6769f, 0.707107f);
path.close();
path.moveTo(5.75818f, 42.4858f);
path.quadTo(5.39763f, 42.089f, 5.05371f, 41.6777f);
path.conicTo(4.41226f, 40.9105f, 5.17942f, 40.2691f, 0.707107f);
path.conicTo(5.94658f, 39.6276f, 6.58804f, 40.3948f, 0.707107f);
path.quadTo(6.90548f, 40.7744f, 7.23832f, 41.1407f);
path.conicTo(7.91085f, 41.8808f, 7.17078f, 42.5533f, 0.707107f);
path.conicTo(6.43071f, 43.2258f, 5.75818f, 42.4858f, 0.707107f);
path.close();
path.moveTo(3.72821f, 39.9503f);
path.quadTo(3.42794f, 39.523f, 3.1451f, 39.0842f);
path.conicTo(2.6034f, 38.2436f, 3.44397f, 37.7019f, 0.707107f);
path.conicTo(4.28454f, 37.1602f, 4.82624f, 38.0008f, 0.707107f);
path.quadTo(5.08734f, 38.4059f, 5.3645f, 38.8003f);
path.conicTo(5.93951f, 39.6184f, 5.12137f, 40.1934f, 0.707107f);
path.conicTo(4.30322f, 40.7684f, 3.72821f, 39.9503f, 0.707107f);
path.close();
path.moveTo(2.09762f, 37.3078f);
path.quadTo(1.85114f, 36.8491f, 1.62324f, 36.381f);
path.conicTo(1.18551f, 35.4819f, 2.08461f, 35.0442f, 0.707107f);
path.conicTo(2.98372f, 34.6064f, 3.42145f, 35.5055f, 0.707107f);
path.quadTo(3.63184f, 35.9377f, 3.85934f, 36.361f);
path.conicTo(4.33272f, 37.2419f, 3.45185f, 37.7153f, 0.707107f);
path.conicTo(2.57099f, 38.1886f, 2.09762f, 37.3078f, 0.707107f);
path.close();
path.moveTo(0.781912f, 34.4596f);
path.quadTo(0.589924f, 33.9681f, 0.418029f, 33.4692f);
path.conicTo(0.0922952f, 32.5237f, 1.03776f, 32.198f, 0.707107f);
path.conicTo(1.98322f, 31.8722f, 2.30895f, 32.8177f, 0.707107f);
path.quadTo(2.46761f, 33.2782f, 2.64484f, 33.7319f);
path.conicTo(3.00867f, 34.6634f, 2.07721f, 35.0272f, 0.707107f);
path.conicTo(1.14575f, 35.3911f, 0.781912f, 34.4596f, 0.707107f);
path.close();
path.moveTo(-0.189761f, 31.4402f);
path.quadTo(-0.321263f, 30.9258f, -0.431662f, 30.4065f);
path.conicTo(-0.639608f, 29.4284f, 0.338532f, 29.2205f, 0.707107f);
path.conicTo(1.31667f, 29.0125f, 1.52462f, 29.9906f, 0.707107f);
path.quadTo(1.62653f, 30.47f, 1.74791f, 30.9448f);
path.conicTo(1.99561f, 31.9136f, 1.02677f, 32.1613f, 0.707107f);
path.conicTo(0.0579369f, 32.409f, -0.189761f, 31.4402f, 0.707107f);
path.close();
path.moveTo(-0.784658f, 28.3394f);
path.quadTo(-0.851693f, 27.8218f, -0.897902f, 27.3019f);
path.conicTo(-0.986437f, 26.3058f, 0.00963629f, 26.2173f, 0.707107f);
path.conicTo(1.00571f, 26.1288f, 1.09424f, 27.1248f, 0.707107f);
path.quadTo(1.1369f, 27.6047f, 1.19878f, 28.0825f);
path.conicTo(1.32721f, 29.0742f, 0.335496f, 29.2027f, 0.707107f);
path.conicTo(-0.656222f, 29.3311f, -0.784658f, 28.3394f, 0.707107f);
path.close();
path.moveTo(-0.999031f, 25.2248f);
path.quadTo(-1.00354f, 24.7027f, -0.987098f, 24.1809f);
path.conicTo(-0.955596f, 23.1814f, 0.0439078f, 23.2129f, 0.707107f);
path.conicTo(1.04341f, 23.2444f, 1.01191f, 24.2439f, 0.707107f);
path.quadTo(0.996728f, 24.7256f, 1.00089f, 25.2075f);
path.conicTo(1.00954f, 26.2075f, 0.00957754f, 26.2161f, 0.707107f);
path.conicTo(-0.990385f, 26.2248f, -0.999031f, 25.2248f, 0.707107f);
path.close();
path.moveTo(-0.836492f, 22.0887f);
path.quadTo(-0.778263f, 21.5719f, -0.699419f, 21.0579f);
path.conicTo(-0.5478f, 20.0695f, 0.440639f, 20.2211f, 0.707107f);
path.conicTo(1.42908f, 20.3727f, 1.27746f, 21.3612f, 0.707107f);
path.quadTo(1.20468f, 21.8356f, 1.15093f, 22.3126f);
path.conicTo(1.03896f, 23.3063f, 0.0452449f, 23.1944f, 0.707107f);
path.conicTo(-0.948466f, 23.0824f, -0.836492f, 22.0887f, 0.707107f);
path.close();
path.moveTo(-0.300548f, 19.0098f);
path.quadTo(-0.174573f, 18.4777f, -0.0263361f, 17.9514f);
path.conicTo(0.244762f, 16.9889f, 1.20731f, 17.26f, 0.707107f);
path.conicTo(2.16987f, 17.5311f, 1.89877f, 18.4936f, 0.707107f);
path.quadTo(1.76193f, 18.9794f, 1.64565f, 19.4706f);
path.conicTo(1.41526f, 20.4437f, 0.442159f, 20.2133f, 0.707107f);
path.conicTo(-0.530939f, 19.9829f, -0.300548f, 19.0098f, 0.707107f);
path.close();
path.moveTo(0.642658f, 15.9049f);
path.quadTo(0.827861f, 15.409f, 1.0331f, 14.9209f);
path.conicTo(1.42076f, 13.9991f, 2.34256f, 14.3868f, 0.707107f);
path.conicTo(3.26437f, 14.7744f, 2.87671f, 15.6962f, 0.707107f);
path.quadTo(2.68726f, 16.1467f, 2.5163f, 16.6046f);
path.conicTo(2.16648f, 17.5414f, 1.22967f, 17.1916f, 0.707107f);
path.conicTo(0.292846f, 16.8418f, 0.642658f, 15.9049f, 0.707107f);
path.close();
path.moveTo(1.91434f, 13.0395f);
path.quadTo(2.14856f, 12.5875f, 2.40031f, 12.1449f);
path.conicTo(2.89473f, 11.2757f, 3.76395f, 11.7701f, 0.707107f);
path.conicTo(4.63317f, 12.2645f, 4.13875f, 13.1337f, 0.707107f);
path.quadTo(3.90637f, 13.5423f, 3.69016f, 13.9596f);
path.conicTo(3.23014f, 14.8475f, 2.34223f, 14.3875f, 0.707107f);
path.conicTo(1.45432f, 13.9275f, 1.91434f, 13.0395f, 0.707107f);
path.close();
path.moveTo(3.45073f, 10.4525f);
path.quadTo(3.72744f, 10.0426f, 4.01954f, 9.64356f);
path.conicTo(4.61017f, 8.83661f, 5.41711f, 9.42725f, 0.707107f);
path.conicTo(6.22405f, 10.0179f, 5.63342f, 10.8248f, 0.707107f);
path.quadTo(5.36379f, 11.1932f, 5.10836f, 11.5716f);
path.conicTo(4.54884f, 12.4004f, 3.72003f, 11.8409f, 0.707107f);
path.conicTo(2.89121f, 11.2813f, 3.45073f, 10.4525f, 0.707107f);
path.close();
path.moveTo(5.2763f, 8.05964f);
path.quadTo(5.61273f, 7.66793f, 5.96445f, 7.2899f);
path.conicTo(6.6456f, 6.55776f, 7.37774f, 7.23892f, 0.707107f);
path.conicTo(8.10988f, 7.92008f, 7.42872f, 8.65221f, 0.707107f);
path.quadTo(7.10407f, 9.00116f, 6.79351f, 9.36274f);
path.conicTo(6.14196f, 10.1213f, 5.38336f, 9.46979f, 0.707107f);
path.conicTo(4.62475f, 8.81824f, 5.2763f, 8.05964f, 0.707107f);
path.close();
path.moveTo(7.45913f, 5.80839f);
path.quadTo(7.85457f, 5.44696f, 8.26455f, 5.10214f);
path.conicTo(9.02985f, 4.45847f, 9.67352f, 5.22377f, 0.707107f);
path.conicTo(10.3172f, 5.98907f, 9.5519f, 6.63274f, 0.707107f);
path.quadTo(9.17345f, 6.95105f, 8.80843f, 7.28467f);
path.conicTo(8.07029f, 7.95931f, 7.39564f, 7.22117f, 0.707107f);
path.conicTo(6.72099f, 6.48303f, 7.45913f, 5.80839f, 0.707107f);
path.close();
path.moveTo(9.98688f, 3.77251f);
path.quadTo(10.4153f, 3.46948f, 10.8557f, 3.18397f);
path.conicTo(11.6948f, 2.63996f, 12.2388f, 3.47904f, 0.707107f);
path.conicTo(12.7828f, 4.31812f, 11.9437f, 4.86213f, 0.707107f);
path.quadTo(11.5373f, 5.12566f, 11.1417f, 5.40539f);
path.conicTo(10.3253f, 5.98282f, 9.74787f, 5.16638f, 0.707107f);
path.conicTo(9.17044f, 4.34994f, 9.98688f, 3.77251f, 0.707107f);
path.close();
path.moveTo(12.6283f, 2.13208f);
path.quadTo(13.0861f, 1.88442f, 13.5534f, 1.65529f);
path.conicTo(14.4513f, 1.21504f, 14.8915f, 2.11291f, 0.707107f);
path.conicTo(15.3318f, 3.01078f, 14.4339f, 3.45104f, 0.707107f);
path.quadTo(14.0025f, 3.66255f, 13.58f, 3.89115f);
path.conicTo(12.7005f, 4.36698f, 12.2246f, 3.48744f, 0.707107f);
path.conicTo(11.7488f, 2.60791f, 12.6283f, 2.13208f, 0.707107f);
path.close();
path.moveTo(15.4718f, 0.808815f);
path.quadTo(15.9627f, 0.615476f, 16.461f, 0.442208f);
path.conicTo(17.4055f, 0.113784f, 17.7339f, 1.05831f, 0.707107f);
path.conicTo(18.0624f, 2.00284f, 17.1178f, 2.33127f, 0.707107f);
path.quadTo(16.6578f, 2.49121f, 16.2047f, 2.66968f);
path.conicTo(15.2743f, 3.03614f, 14.9078f, 2.10571f, 0.707107f);
path.conicTo(14.5414f, 1.17528f, 15.4718f, 0.808815f, 0.707107f);
path.close();
path.moveTo(18.4879f, -0.171272f);
path.quadTo(19.0019f, -0.304236f, 19.5208f, -0.416111f);
path.conicTo(20.4984f, -0.62685f, 20.7091f, 0.350692f, 0.707107f);
path.conicTo(20.9198f, 1.32823f, 19.9423f, 1.53897f, 0.707107f);
path.quadTo(19.4633f, 1.64224f, 18.9889f, 1.76498f);
path.conicTo(18.0207f, 2.01544f, 17.7703f, 1.04732f, 0.707107f);
path.conicTo(17.5198f, 0.0791926f, 18.4879f, -0.171272f, 0.707107f);
path.close();
path.moveTo(21.5882f, -0.77517f);
path.quadTo(22.1056f, -0.843665f, 22.6254f, -0.891339f);
path.conicTo(23.6212f, -0.982672f, 23.7126f, 0.0131486f, 0.707107f);
path.conicTo(23.8039f, 1.00897f, 22.8081f, 1.1003f, 0.707107f);
path.quadTo(22.3283f, 1.14431f, 21.8506f, 1.20754f);
path.conicTo(20.8592f, 1.33876f, 20.728f, 0.347405f, 0.707107f);
path.conicTo(20.5968f, -0.643948f, 21.5882f, -0.77517f, 0.707107f);
path.close();
path.moveTo(24.7026f, -0.998301f);
path.quadTo(25.2241f, -1.00426f, 25.7453f, -0.989316f);
path.conicTo(26.7449f, -0.960651f, 26.7162f, 0.0389383f, 0.707107f);
path.conicTo(26.6876f, 1.03853f, 25.688f, 1.00986f, 0.707107f);
path.quadTo(25.2068f, 0.996064f, 24.7255f, 1.00157f);
path.conicTo(23.7256f, 1.013f, 23.7141f, 0.0130688f, 0.707107f);
path.conicTo(23.7027f, -0.986866f, 24.7026f, -0.998301f, 0.707107f);
path.close();
path.moveTo(27.8388f, -0.844563f);
path.quadTo(28.3559f, -0.787759f, 28.8704f, -0.710314f);
path.conicTo(29.8592f, -0.561454f, 29.7104f, 0.427404f, 0.707107f);
path.conicTo(29.5615f, 1.41626f, 28.5726f, 1.2674f, 0.707107f);
path.quadTo(28.0978f, 1.19591f, 27.6204f, 1.14348f);
path.conicTo(26.6264f, 1.0343f, 26.7356f, 0.0402742f, 0.707107f);
path.conicTo(26.8447f, -0.953747f, 27.8388f, -0.844563f, 0.707107f);
path.close();
path.moveTo(30.9153f, -0.318153f);
path.quadTo(31.4481f, -0.193671f, 31.9752f, -0.046875f);
path.conicTo(32.9386f, 0.221405f, 32.6703f, 1.18475f, 0.707107f);
path.conicTo(32.402f, 2.14809f, 31.4387f, 1.87981f, 0.707107f);
path.quadTo(30.9521f, 1.74431f, 30.4603f, 1.6294f);
path.conicTo(29.4865f, 1.40189f, 29.714f, 0.428111f, 0.707107f);
path.conicTo(29.9416f, -0.545664f, 30.9153f, -0.318153f, 0.707107f);
path.close();
path.moveTo(34.0252f, 0.616677f);
path.quadTo(34.5221f, 0.800609f, 35.0111f, 1.00465f);
path.conicTo(35.934f, 1.3897f, 35.549f, 2.31259f, 0.707107f);
path.conicTo(35.1639f, 3.23549f, 34.241f, 2.85044f, 0.707107f);
path.quadTo(33.7896f, 2.66211f, 33.3309f, 2.49232f);
path.conicTo(32.3931f, 2.1452f, 32.7402f, 1.20738f, 0.707107f);
path.conicTo(33.0873f, 0.269559f, 34.0252f, 0.616677f, 0.707107f);
path.close();
path.moveTo(36.8967f, 1.88141f);
path.quadTo(37.3499f, 2.11462f, 37.7936f, 2.3654f);
path.conicTo(38.6641f, 2.85746f, 38.1721f, 3.72802f, 0.707107f);
path.conicTo(37.68f, 4.59858f, 36.8094f, 4.10652f, 0.707107f);
path.quadTo(36.3999f, 3.87504f, 35.9815f, 3.65976f);
path.conicTo(35.0924f, 3.2022f, 35.5499f, 2.31302f, 0.707107f);
path.conicTo(36.0075f, 1.42384f, 36.8967f, 1.88141f, 0.707107f);
path.close();
path.moveTo(39.4914f, 3.413f);
path.lineTo(39.5381f, 3.44439f);
path.quadTo(39.9244f, 3.70494f, 40.3002f, 3.97845f);
path.conicTo(41.1087f, 4.56692f, 40.5202f, 5.37544f, 0.707107f);
path.conicTo(39.9317f, 6.18396f, 39.1232f, 5.59549f, 0.707107f);
path.quadTo(38.7763f, 5.34298f, 38.4215f, 5.10371f);
path.lineTo(38.3749f, 5.07232f);
path.conicTo(37.5452f, 4.51406f, 38.1035f, 3.68439f, 0.707107f);
path.conicTo(38.6618f, 2.85473f, 39.4914f, 3.413f, 0.707107f);
path.close();
path.moveTo(41.8859f, 5.22965f);
path.quadTo(42.2782f, 5.56471f, 42.6568f, 5.91499f);
path.conicTo(43.3908f, 6.5941f, 42.7117f, 7.32814f, 0.707107f);
path.conicTo(42.0326f, 8.06218f, 41.2986f, 7.38308f, 0.707107f);
path.quadTo(40.949f, 7.05968f, 40.587f, 6.75043f);
path.conicTo(39.8266f, 6.10097f, 40.476f, 5.34058f, 0.707107f);
path.conicTo(41.1255f, 4.58018f, 41.8859f, 5.22965f, 0.707107f);
path.close();
path.moveTo(44.1413f, 7.40421f);
path.quadTo(44.5035f, 7.79829f, 44.8493f, 8.20695f);
path.conicTo(45.4952f, 8.97038f, 44.7317f, 9.61627f, 0.707107f);
path.conicTo(43.9683f, 10.2622f, 43.3224f, 9.49874f, 0.707107f);
path.quadTo(43.0033f, 9.1215f, 42.6689f, 8.75773f);
path.conicTo(41.9921f, 8.02152f, 42.7283f, 7.34476f, 0.707107f);
path.conicTo(43.4645f, 6.668f, 44.1413f, 7.40421f, 0.707107f);
path.close();
path.moveTo(46.183f, 9.9242f);
path.quadTo(46.4888f, 10.3539f, 46.777f, 10.7957f);
path.conicTo(47.3233f, 11.6332f, 46.4857f, 12.1796f, 0.707107f);
path.conicTo(45.6482f, 12.7259f, 45.1018f, 11.8883f, 0.707107f);
path.quadTo(44.8358f, 11.4805f, 44.5535f, 11.0839f);
path.conicTo(43.9737f, 10.2691f, 44.7884f, 9.6893f, 0.707107f);
path.conicTo(45.6032f, 9.10947f, 46.183f, 9.9242f, 0.707107f);
path.close();
path.moveTo(47.8333f, 12.5645f);
path.quadTo(48.0821f, 13.0214f, 48.3125f, 13.4879f);
path.conicTo(48.7552f, 14.3845f, 47.8586f, 14.8273f, 0.707107f);
path.conicTo(46.962f, 15.2701f, 46.5192f, 14.3734f, 0.707107f);
path.quadTo(46.3065f, 13.9428f, 46.0769f, 13.5211f);
path.conicTo(45.5986f, 12.6429f, 46.4768f, 12.1646f, 0.707107f);
path.conicTo(47.355f, 11.6863f, 47.8333f, 12.5645f, 0.707107f);
path.close();
path.moveTo(49.1641f, 15.4033f);
path.quadTo(49.3588f, 15.8935f, 49.5334f, 16.3912f);
path.conicTo(49.8645f, 17.3348f, 48.9209f, 17.6659f, 0.707107f);
path.conicTo(47.9773f, 17.997f, 47.6462f, 17.0534f, 0.707107f);
path.quadTo(47.485f, 16.5939f, 47.3053f, 16.1415f);
path.conicTo(46.9362f, 15.2121f, 47.8656f, 14.843f, 0.707107f);
path.conicTo(48.795f, 14.4739f, 49.1641f, 15.4033f, 0.707107f);
path.close();
path.moveTo(50.1526f, 18.4161f);
path.quadTo(50.287f, 18.9296f, 50.4003f, 19.4482f);
path.conicTo(50.6139f, 20.4252f, 49.6369f, 20.6387f, 0.707107f);
path.conicTo(48.66f, 20.8522f, 48.4465f, 19.8753f, 0.707107f);
path.quadTo(48.3419f, 19.3966f, 48.2178f, 18.9225f);
path.conicTo(47.9645f, 17.9551f, 48.9319f, 17.7019f, 0.707107f);
path.conicTo(49.8993f, 17.4487f, 50.1526f, 18.4161f, 0.707107f);
path.close();
path.moveTo(50.7655f, 21.5157f);
path.quadTo(50.8354f, 22.033f, 50.8846f, 22.5528f);
path.conicTo(50.9787f, 23.5483f, 49.9831f, 23.6425f, 0.707107f);
path.conicTo(48.9876f, 23.7366f, 48.8935f, 22.741f, 0.707107f);
path.quadTo(48.8481f, 22.2613f, 48.7835f, 21.7837f);
path.conicTo(48.6495f, 20.7928f, 49.6405f, 20.6587f, 0.707107f);
path.conicTo(50.6315f, 20.5247f, 50.7655f, 21.5157f, 0.707107f);
path.close();
path.moveTo(50.9974f, 24.6301f);
path.quadTo(51.0048f, 25.1509f, 50.9913f, 25.6715f);
path.conicTo(50.9655f, 26.6712f, 49.9658f, 26.6454f, 0.707107f);
path.conicTo(48.9662f, 26.6196f, 48.992f, 25.6199f, 0.707107f);
path.quadTo(49.0044f, 25.1393f, 48.9976f, 24.6585f);
path.conicTo(48.9834f, 23.6586f, 49.9833f, 23.6444f, 0.707107f);
path.conicTo(50.9832f, 23.6302f, 50.9974f, 24.6301f, 0.707107f);
path.close();
path.moveTo(50.8524f, 27.7662f);
path.quadTo(50.7971f, 28.2837f, 50.721f, 28.7986f);
path.conicTo(50.5749f, 29.7879f, 49.5856f, 29.6418f, 0.707107f);
path.conicTo(48.5963f, 29.4957f, 48.7425f, 28.5064f, 0.707107f);
path.quadTo(48.8127f, 28.0311f, 48.8638f, 27.5534f);
path.conicTo(48.9702f, 26.5591f, 49.9645f, 26.6655f, 0.707107f);
path.conicTo(50.9588f, 26.7718f, 50.8524f, 27.7662f, 0.707107f);
path.close();
path.moveTo(50.3355f, 30.8404f);
path.quadTo(50.2125f, 31.3739f, 50.0672f, 31.9018f);
path.conicTo(49.8018f, 32.8659f, 48.8376f, 32.6005f, 0.707107f);
path.conicTo(47.8735f, 32.335f, 48.139f, 31.3709f, 0.707107f);
path.quadTo(48.2731f, 30.8836f, 48.3867f, 30.3912f);
path.conicTo(48.6113f, 29.4167f, 49.5857f, 29.6413f, 0.707107f);
path.conicTo(50.5602f, 29.866f, 50.3355f, 30.8404f, 0.707107f);
path.close();
path.moveTo(49.4091f, 33.9552f);
path.quadTo(49.2264f, 34.4531f, 49.0236f, 34.9431f);
path.conicTo(48.6412f, 35.8671f, 47.7172f, 35.4846f, 0.707107f);
path.conicTo(46.7932f, 35.1022f, 47.1757f, 34.1782f, 0.707107f);
path.quadTo(47.3629f, 33.7259f, 47.5315f, 33.2663f);
path.conicTo(47.8759f, 32.3275f, 48.8147f, 32.672f, 0.707107f);
path.conicTo(49.7535f, 33.0164f, 49.4091f, 33.9552f, 0.707107f);
path.close();
path.moveTo(48.1514f, 36.8328f);
path.quadTo(47.9191f, 37.2871f, 47.6694f, 37.7318f);
path.conicTo(47.1797f, 38.6038f, 46.3078f, 38.1141f, 0.707107f);
path.conicTo(45.4359f, 37.6244f, 45.9256f, 36.7525f, 0.707107f);
path.quadTo(46.1562f, 36.3418f, 46.3705f, 35.9226f);
path.conicTo(46.8256f, 35.0321f, 47.716f, 35.4872f, 0.707107f);
path.conicTo(48.6065f, 35.9423f, 48.1514f, 36.8328f, 0.707107f);
path.close();
path.moveTo(46.6245f, 39.4354f);
path.lineTo(46.5563f, 39.537f);
path.quadTo(46.3146f, 39.8955f, 46.0624f, 40.2438f);
path.conicTo(45.4761f, 41.0539f, 44.666f, 40.4676f, 0.707107f);
path.conicTo(43.8559f, 39.8813f, 44.4422f, 39.0712f, 0.707107f);
path.quadTo(44.6749f, 38.7498f, 44.8955f, 38.4226f);
path.lineTo(44.9637f, 38.3211f);
path.conicTo(45.5209f, 37.4907f, 46.3513f, 38.0479f, 0.707107f);
path.conicTo(47.1817f, 38.605f, 46.6245f, 39.4354f, 0.707107f);
path.close();
path.moveTo(44.8168f, 41.8314f);
path.quadTo(44.4832f, 42.2241f, 44.1342f, 42.6034f);
path.conicTo(43.4572f, 43.3394f, 42.7212f, 42.6623f, 0.707107f);
path.conicTo(41.9853f, 41.9853f, 42.6623f, 41.2494f, 0.707107f);
path.quadTo(42.9845f, 40.8992f, 43.2924f, 40.5366f);
path.conicTo(43.9398f, 39.7745f, 44.702f, 40.4218f, 0.707107f);
path.conicTo(45.4642f, 41.0692f, 44.8168f, 41.8314f, 0.707107f);
path.close();
path.moveTo(42.6505f, 44.0908f);
path.quadTo(42.2577f, 44.454f, 41.8504f, 44.8006f);
path.conicTo(41.0888f, 45.4487f, 40.4408f, 44.6871f, 0.707107f);
path.conicTo(39.7927f, 43.9256f, 40.5542f, 43.2775f, 0.707107f);
path.quadTo(40.9302f, 42.9575f, 41.2928f, 42.6223f);
path.conicTo(42.027f, 41.9434f, 42.7059f, 42.6777f, 0.707107f);
path.conicTo(43.3848f, 43.412f, 42.6505f, 44.0908f, 0.707107f);
path.close();
path.moveTo(40.1383f, 46.1384f);
path.quadTo(39.7073f, 46.4471f, 39.2641f, 46.7378f);
path.conicTo(38.4281f, 47.2865f, 37.8795f, 46.4504f, 0.707107f);
path.conicTo(37.3308f, 45.6143f, 38.1669f, 45.0657f, 0.707107f);
path.quadTo(38.576f, 44.7972f, 38.9738f, 44.5124f);
path.conicTo(39.7868f, 43.9301f, 40.369f, 44.7432f, 0.707107f);
path.conicTo(40.9513f, 45.5562f, 40.1383f, 46.1384f, 0.707107f);
path.close();
path.moveTo(37.4991f, 47.7985f);
path.quadTo(37.0431f, 48.0485f, 36.5775f, 48.2801f);
path.conicTo(35.6821f, 48.7254f, 35.2368f, 47.83f, 0.707107f);
path.conicTo(34.7915f, 46.9346f, 35.6869f, 46.4893f, 0.707107f);
path.quadTo(36.1167f, 46.2755f, 36.5376f, 46.0448f);
path.conicTo(37.4145f, 45.5641f, 37.8952f, 46.4409f, 0.707107f);
path.conicTo(38.376f, 47.3178f, 37.4991f, 47.7985f, 0.707107f);
path.close();
path.moveTo(34.6651f, 49.1368f);
path.quadTo(34.1756f, 49.3328f, 33.6785f, 49.5089f);
path.conicTo(32.7358f, 49.8427f, 32.402f, 48.9f, 0.707107f);
path.conicTo(32.0682f, 47.9574f, 33.0109f, 47.6236f, 0.707107f);
path.quadTo(33.4697f, 47.4611f, 33.9216f, 47.2801f);
path.conicTo(34.85f, 46.9084f, 35.2217f, 47.8368f, 0.707107f);
path.conicTo(35.5934f, 48.7651f, 34.6651f, 49.1368f, 0.707107f);
path.close();
path.moveTo(31.6557f, 50.1337f);
path.quadTo(31.1425f, 50.2696f, 30.6243f, 50.3844f);
path.conicTo(29.648f, 50.6007f, 29.4317f, 49.6244f, 0.707107f);
path.conicTo(29.2153f, 48.6481f, 30.1917f, 48.4317f, 0.707107f);
path.quadTo(30.6701f, 48.3257f, 31.1437f, 48.2003f);
path.conicTo(32.1104f, 47.9443f, 32.3664f, 48.911f, 0.707107f);
path.conicTo(32.6223f, 49.8777f, 31.6557f, 50.1337f, 0.707107f);
path.close();
path.moveTo(28.5567f, 50.7556f);
path.quadTo(28.0395f, 50.827f, 27.5198f, 50.8776f);
path.conicTo(26.5245f, 50.9745f, 26.4276f, 49.9792f, 0.707107f);
path.conicTo(26.3307f, 48.9839f, 27.326f, 48.887f, 0.707107f);
path.quadTo(27.8056f, 48.8403f, 28.2831f, 48.7744f);
path.conicTo(29.2737f, 48.6376f, 29.4105f, 49.6282f, 0.707107f);
path.conicTo(29.5473f, 50.6188f, 28.5567f, 50.7556f, 0.707107f);
path.close();
path.moveTo(25.4424f, 50.9962f);
path.quadTo(24.9222f, 51.0051f, 24.4022f, 50.9931f);
path.conicTo(23.4025f, 50.9701f, 23.4255f, 49.9704f, 0.707107f);
path.conicTo(23.4485f, 48.9707f, 24.4482f, 48.9937f, 0.707107f);
path.quadTo(24.9283f, 49.0047f, 25.4084f, 48.9965f);
path.conicTo(26.4083f, 48.9795f, 26.4253f, 49.9794f, 0.707107f);
path.conicTo(26.4423f, 50.9792f, 25.4424f, 50.9962f, 0.707107f);
path.close();
path.moveTo(22.3065f, 50.8601f);
path.quadTo(21.7885f, 50.8062f, 21.2732f, 50.7315f);
path.conicTo(20.2835f, 50.5882f, 20.4268f, 49.5985f, 0.707107f);
path.conicTo(20.5702f, 48.6088f, 21.5599f, 48.7522f, 0.707107f);
path.quadTo(22.0355f, 48.8211f, 22.5136f, 48.8709f);
path.conicTo(23.5083f, 48.9745f, 23.4047f, 49.9691f, 0.707107f);
path.conicTo(23.3011f, 50.9637f, 22.3065f, 50.8601f, 0.707107f);
path.close();
path.moveTo(19.2346f, 50.3527f);
path.quadTo(18.7003f, 50.2312f, 18.1717f, 50.0873f);
path.conicTo(17.2068f, 49.8247f, 17.4694f, 48.8598f, 0.707107f);
path.conicTo(17.732f, 47.8949f, 18.6969f, 48.1575f, 0.707107f);
path.quadTo(19.185f, 48.2904f, 19.6781f, 48.4025f);
path.conicTo(20.6532f, 48.6243f, 20.4314f, 49.5994f, 0.707107f);
path.conicTo(20.2097f, 50.5745f, 19.2346f, 50.3527f, 0.707107f);
path.close();
path.moveTo(16.1149f, 49.4347f);
path.quadTo(15.6161f, 49.2533f, 15.1251f, 49.0517f);
path.conicTo(14.2f, 48.6719f, 14.5798f, 47.7469f, 0.707107f);
path.conicTo(14.9596f, 46.8218f, 15.8847f, 47.2016f, 0.707107f);
path.quadTo(16.3379f, 47.3877f, 16.7984f, 47.5551f);
path.conicTo(17.7382f, 47.8969f, 17.3964f, 48.8366f, 0.707107f);
path.conicTo(17.0547f, 49.7764f, 16.1149f, 49.4347f, 0.707107f);
path.close();
path.moveTo(13.2313f, 48.184f);
path.quadTo(12.776f, 47.9529f, 12.33f, 47.704f);
path.conicTo(11.4568f, 47.2167f, 11.9441f, 46.3434f, 0.707107f);
path.conicTo(12.4314f, 45.4702f, 13.3046f, 45.9575f, 0.707107f);
path.quadTo(13.7162f, 46.1872f, 14.1365f, 46.4006f);
path.conicTo(15.0282f, 46.8532f, 14.5756f, 47.7449f, 0.707107f);
path.conicTo(14.123f, 48.6366f, 13.2313f, 48.184f, 0.707107f);
path.close();
path.moveTo(10.6208f, 46.6619f);
path.lineTo(10.4641f, 46.5571f);
path.quadTo(10.1333f, 46.334f, 9.81253f, 46.1031f);
path.conicTo(9.00087f, 45.519f, 9.585f, 44.7073f, 0.707107f);
path.conicTo(10.1691f, 43.8957f, 10.9808f, 44.4798f, 0.707107f);
path.quadTo(11.2769f, 44.6929f, 11.5763f, 44.8948f);
path.lineTo(11.7329f, 44.9996f);
path.conicTo(12.564f, 45.5557f, 12.008f, 46.3868f, 0.707107f);
path.conicTo(11.4519f, 47.2179f, 10.6208f, 46.6619f, 0.707107f);
path.close();
path.moveTo(8.22326f, 44.8631f);
path.quadTo(7.82986f, 44.5308f, 7.44999f, 44.1833f);
path.conicTo(6.71217f, 43.5082f, 7.38718f, 42.7704f, 0.707107f);
path.conicTo(8.06219f, 42.0326f, 8.8f, 42.7076f, 0.707107f);
path.quadTo(9.15066f, 43.0284f, 9.51375f, 43.3351f);
path.conicTo(10.2777f, 43.9804f, 9.63248f, 44.7443f, 0.707107f);
path.conicTo(8.98724f, 45.5083f, 8.22326f, 44.8631f, 0.707107f);
path.close();
path.moveTo(5.95972f, 42.705f);
path.quadTo(5.59577f, 42.3136f, 5.24823f, 41.9076f);
path.conicTo(4.59793f, 41.148f, 5.3576f, 40.4977f, 0.707107f);
path.conicTo(6.11728f, 39.8473f, 6.76758f, 40.607f, 0.707107f);
path.quadTo(7.08843f, 40.9818f, 7.42436f, 41.3431f);
path.conicTo(8.10532f, 42.0754f, 7.373f, 42.7564f, 0.707107f);
path.conicTo(6.64068f, 43.4373f, 5.95972f, 42.705f, 0.707107f);
path.close();
path.moveTo(3.90635f, 40.2006f);
path.quadTo(3.59492f, 39.7684f, 3.30147f, 39.3239f);
path.conicTo(2.75055f, 38.4893f, 3.58511f, 37.9384f, 0.707107f);
path.conicTo(4.41967f, 37.3875f, 4.97059f, 38.222f, 0.707107f);
path.quadTo(5.24148f, 38.6324f, 5.52894f, 39.0313f);
path.conicTo(6.11358f, 39.8426f, 5.30228f, 40.4272f, 0.707107f);
path.conicTo(4.49099f, 41.0119f, 3.90635f, 40.2006f, 0.707107f);
path.close();
path.moveTo(2.23643f, 37.5626f);
path.quadTo(1.98525f, 37.1075f, 1.75248f, 36.6427f);
path.conicTo(1.30469f, 35.7486f, 2.19883f, 35.3008f, 0.707107f);
path.conicTo(3.09296f, 34.853f, 3.54076f, 35.7471f, 0.707107f);
path.quadTo(3.75563f, 36.1762f, 3.98747f, 36.5963f);
path.conicTo(4.47065f, 37.4718f, 3.59513f, 37.955f, 0.707107f);
path.conicTo(2.71961f, 38.4382f, 2.23643f, 37.5626f, 0.707107f);
path.close();
path.moveTo(0.890647f, 34.7334f);
path.quadTo(0.69328f, 34.2445f, 0.515902f, 33.7481f);
path.conicTo(0.179435f, 32.8064f, 1.12113f, 32.4699f, 0.707107f);
path.conicTo(2.06282f, 32.1335f, 2.39929f, 33.0752f, 0.707107f);
path.quadTo(2.56303f, 33.5334f, 2.74521f, 33.9847f);
path.conicTo(3.11957f, 34.912f, 2.19229f, 35.2863f, 0.707107f);
path.conicTo(1.26501f, 35.6607f, 0.890647f, 34.7334f, 0.707107f);
path.close();
path.moveTo(-0.114587f, 31.7274f);
path.quadTo(-0.251922f, 31.2147f, -0.368218f, 30.6968f);
path.conicTo(-0.587327f, 29.7211f, 0.388373f, 29.502f, 0.707107f);
path.conicTo(1.36407f, 29.2829f, 1.58318f, 30.2586f, 0.707107f);
path.quadTo(1.69053f, 30.7366f, 1.8173f, 31.2099f);
path.conicTo(2.07605f, 32.1758f, 1.1101f, 32.4346f, 0.707107f);
path.conicTo(0.144159f, 32.6933f, -0.114587f, 31.7274f, 0.707107f);
path.close();
path.moveTo(-0.745485f, 28.6291f);
path.quadTo(-0.818367f, 28.112f, -0.870432f, 27.5925f);
path.conicTo(-0.970142f, 26.5974f, 0.0248742f, 26.4977f, 0.707107f);
path.conicTo(1.01989f, 26.398f, 1.1196f, 27.393f, 0.707107f);
path.quadTo(1.16766f, 27.8726f, 1.23494f, 28.3499f);
path.conicTo(1.37452f, 29.3401f, 0.384305f, 29.4797f, 0.707107f);
path.conicTo(-0.605905f, 29.6193f, -0.745485f, 28.6291f, 0.707107f);
path.close();
path.moveTo(-0.994901f, 25.515f);
path.quadTo(-1.00519f, 24.9955f, -0.994722f, 24.4761f);
path.conicTo(-0.97457f, 23.4763f, 0.0252273f, 23.4964f, 0.707107f);
path.conicTo(1.02502f, 23.5166f, 1.00487f, 24.5164f, 0.707107f);
path.quadTo(0.995207f, 24.9959f, 1.00471f, 25.4754f);
path.conicTo(1.02451f, 26.4752f, 0.0247103f, 26.495f, 0.707107f);
path.conicTo(-0.975093f, 26.5148f, -0.994901f, 25.515f, 0.707107f);
path.close();
path.moveTo(-0.867571f, 22.3792f);
path.quadTo(-0.81506f, 21.8609f, -0.741825f, 21.3451f);
path.conicTo(-0.60125f, 20.355f, 0.38882f, 20.4956f, 0.707107f);
path.conicTo(1.37889f, 20.6361f, 1.23831f, 21.6262f, 0.707107f);
path.quadTo(1.17071f, 22.1023f, 1.12224f, 22.5807f);
path.conicTo(1.02144f, 23.5757f, 0.026537f, 23.4749f, 0.707107f);
path.conicTo(-0.96837f, 23.3741f, -0.867571f, 22.3792f, 0.707107f);
path.close();
path.moveTo(-0.369678f, 19.3097f);
path.quadTo(-0.249693f, 18.7748f, -0.107265f, 18.2453f);
path.conicTo(0.152529f, 17.2797f, 1.11819f, 17.5395f, 0.707107f);
path.conicTo(2.08386f, 17.7993f, 1.82406f, 18.7649f, 0.707107f);
path.quadTo(1.69259f, 19.2536f, 1.58184f, 19.7474f);
path.conicTo(1.36298f, 20.7232f, 0.387221f, 20.5043f, 0.707107f);
path.conicTo(-0.588536f, 20.2855f, -0.369678f, 19.3097f, 0.707107f);
path.close();
path.moveTo(0.539863f, 16.1851f);
path.quadTo(0.719962f, 15.6854f, 0.920307f, 15.1934f);
path.conicTo(1.29748f, 14.2673f, 2.22362f, 14.6445f, 0.707107f);
path.conicTo(3.14976f, 15.0216f, 2.7726f, 15.9478f, 0.707107f);
path.quadTo(2.58765f, 16.4019f, 2.42141f, 16.8632f);
path.conicTo(2.08237f, 17.804f, 1.1416f, 17.4649f, 0.707107f);
path.conicTo(0.200823f, 17.1259f, 0.539863f, 16.1851f, 0.707107f);
path.close();
path.moveTo(1.78353f, 13.2955f);
path.quadTo(2.01364f, 12.8391f, 2.26151f, 12.392f);
path.conicTo(2.74643f, 11.5175f, 3.62099f, 12.0024f, 0.707107f);
path.conicTo(4.49555f, 12.4873f, 4.01063f, 13.3618f, 0.707107f);
path.quadTo(3.78183f, 13.7745f, 3.56941f, 14.1958f);
path.conicTo(3.11923f, 15.0888f, 2.22629f, 14.6386f, 0.707107f);
path.conicTo(1.33336f, 14.1884f, 1.78353f, 13.2955f, 0.707107f);
path.close();
path.moveTo(3.30083f, 10.6771f);
path.lineTo(3.44218f, 10.4652f);
path.quadTo(3.6466f, 10.1621f, 3.85641f, 9.86895f);
path.conicTo(4.43837f, 9.05574f, 5.25159f, 9.6377f, 0.707107f);
path.conicTo(6.0648f, 10.2197f, 5.48284f, 11.0329f, 0.707107f);
path.quadTo(5.28917f, 11.3035f, 5.10592f, 11.5752f);
path.lineTo(4.96457f, 11.787f);
path.conicTo(4.4096f, 12.6189f, 3.57773f, 12.0639f, 0.707107f);
path.conicTo(2.74586f, 11.509f, 3.30083f, 10.6771f, 0.707107f);
path.close();
path.moveTo(5.0909f, 8.27793f);
path.quadTo(5.42174f, 7.88403f, 5.76791f, 7.50353f);
path.conicTo(6.44085f, 6.76383f, 7.18054f, 7.43678f, 0.707107f);
path.conicTo(7.92024f, 8.10972f, 7.24729f, 8.84942f, 0.707107f);
path.quadTo(6.92775f, 9.20065f, 6.62237f, 9.56424f);
path.conicTo(5.97921f, 10.33f, 5.21348f, 9.68682f, 0.707107f);
path.conicTo(4.44774f, 9.04367f, 5.0909f, 8.27793f, 0.707107f);
path.close();
path.moveTo(7.24064f, 6.0104f);
path.quadTo(7.63069f, 5.64561f, 8.03537f, 5.29717f);
path.conicTo(8.79318f, 4.64469f, 9.44566f, 5.40249f, 0.707107f);
path.conicTo(10.0981f, 6.16029f, 9.34034f, 6.81278f, 0.707107f);
path.quadTo(8.96678f, 7.13442f, 8.60675f, 7.47113f);
path.conicTo(7.87638f, 8.15419f, 7.19332f, 7.42382f, 0.707107f);
path.conicTo(6.51027f, 6.69345f, 7.24064f, 6.0104f, 0.707107f);
path.close();
path.moveTo(9.73726f, 3.95128f);
path.quadTo(10.1706f, 3.63704f, 10.6165f, 3.34092f);
path.conicTo(11.4496f, 2.78771f, 12.0028f, 3.62075f, 0.707107f);
path.conicTo(12.556f, 4.4538f, 11.7229f, 5.007f, 0.707107f);
path.quadTo(11.3113f, 5.28035f, 10.9113f, 5.57041f);
path.conicTo(10.1018f, 6.15744f, 9.51472f, 5.34787f, 0.707107f);
path.conicTo(8.92769f, 4.53831f, 9.73726f, 3.95128f, 0.707107f);
path.close();
path.moveTo(12.374f, 2.27153f);
path.quadTo(12.8282f, 2.01921f, 13.2921f, 1.78522f);
path.conicTo(14.185f, 1.33492f, 14.6353f, 2.22779f, 0.707107f);
path.conicTo(15.0856f, 3.12067f, 14.1927f, 3.57097f, 0.707107f);
path.quadTo(13.7645f, 3.78696f, 13.3452f, 4.01988f);
path.conicTo(12.471f, 4.5055f, 11.9854f, 3.63132f, 0.707107f);
path.conicTo(11.4998f, 2.75715f, 12.374f, 2.27153f, 0.707107f);
path.close();
path.moveTo(15.1984f, 0.918296f);
path.quadTo(15.6866f, 0.719602f, 16.1824f, 0.540851f);
path.conicTo(17.1231f, 0.20171f, 17.4623f, 1.14245f, 0.707107f);
path.conicTo(17.8014f, 2.08318f, 16.8607f, 2.42232f, 0.707107f);
path.quadTo(16.403f, 2.58733f, 15.9524f, 2.77074f);
path.conicTo(15.0261f, 3.14772f, 14.6492f, 2.2215f, 0.707107f);
path.conicTo(14.2722f, 1.29528f, 15.1984f, 0.918296f, 0.707107f);
path.close();
path.moveTo(18.201f, -0.0952874f);
path.quadTo(18.7132f, -0.234075f, 19.2308f, -0.351842f);
path.conicTo(20.2058f, -0.573734f, 20.4277f, 0.401338f, 0.707107f);
path.conicTo(20.6496f, 1.37641f, 19.6745f, 1.5983f, 0.707107f);
path.quadTo(19.1968f, 1.70701f, 18.724f, 1.83512f);
path.conicTo(17.7588f, 2.09662f, 17.4973f, 1.13142f, 0.707107f);
path.conicTo(17.2358f, 0.166216f, 18.201f, -0.0952874f, 0.707107f);
path.close();
path.moveTo(21.2986f, -0.73518f);
path.quadTo(21.8155f, -0.809526f, 22.3349f, -0.863052f);
path.conicTo(23.3297f, -0.965552f, 23.4322f, 0.029181f, 0.707107f);
path.conicTo(23.5347f, 1.02391f, 22.5399f, 1.12641f, 0.707107f);
path.quadTo(22.0604f, 1.17582f, 21.5833f, 1.24445f);
path.conicTo(20.5935f, 1.38681f, 20.4511f, 0.397f, 0.707107f);
path.conicTo(20.3088f, -0.592814f, 21.2986f, -0.73518f, 0.707107f);
path.close();
path.moveTo(24.4124f, -0.993361f);
path.quadTo(24.9312f, -1.00509f, 25.4501f, -0.996107f);
path.conicTo(26.4499f, -0.978799f, 26.4326f, 0.0210512f, 0.707107f);
path.conicTo(26.4153f, 1.0209f, 25.4155f, 1.00359f, 0.707107f);
path.quadTo(24.9365f, 0.995302f, 24.4576f, 1.00613f);
path.conicTo(23.4578f, 1.02873f, 23.4352f, 0.0289853f, 0.707107f);
path.conicTo(23.4126f, -0.970759f, 24.4124f, -0.993361f, 0.707107f);
path.close();
path.moveTo(27.5481f, -0.87484f);
path.quadTo(28.0668f, -0.823762f, 28.583f, -0.75194f);
path.conicTo(29.5734f, -0.614138f, 29.4356f, 0.376322f, 0.707107f);
path.conicTo(29.2978f, 1.36678f, 28.3074f, 1.22898f, 0.707107f);
path.quadTo(27.8309f, 1.16268f, 27.3521f, 1.11553f);
path.conicTo(26.3569f, 1.01753f, 26.4549f, 0.0223428f, 0.707107f);
path.conicTo(26.5529f, -0.972843f, 27.5481f, -0.87484f, 0.707107f);
path.close();
path.moveTo(30.6151f, -0.386432f);
path.quadTo(31.1507f, -0.267954f, 31.6809f, -0.126991f);
path.conicTo(32.6473f, 0.129965f, 32.3904f, 1.09639f, 0.707107f);
path.conicTo(32.1334f, 2.06281f, 31.167f, 1.80585f, 0.707107f);
path.quadTo(30.6776f, 1.67574f, 30.1832f, 1.56637f);
path.conicTo(29.2068f, 1.35041f, 29.4227f, 0.374005f, 0.707107f);
path.conicTo(29.6387f, -0.602396f, 30.6151f, -0.386432f, 0.707107f);
path.close();
path.moveTo(33.7445f, 0.514616f);
path.quadTo(34.2452f, 0.693421f, 34.7381f, 0.892536f);
path.conicTo(35.6653f, 1.26708f, 35.2908f, 2.19429f, 0.707107f);
path.conicTo(34.9162f, 3.1215f, 33.989f, 2.74696f, 0.707107f);
path.quadTo(33.534f, 2.56316f, 33.0718f, 2.3981f);
path.conicTo(32.1301f, 2.06177f, 32.4664f, 1.12003f, 0.707107f);
path.conicTo(32.8027f, 0.178285f, 33.7445f, 0.514616f, 0.707107f);
path.close();
path.moveTo(36.6402f, 1.7512f);
path.quadTo(37.0977f, 1.98026f, 37.5458f, 2.22715f);
path.conicTo(38.4217f, 2.70968f, 37.9392f, 3.58556f, 0.707107f);
path.conicTo(37.4566f, 4.46144f, 36.5808f, 3.97891f, 0.707107f);
path.quadTo(36.1671f, 3.75102f, 35.7448f, 3.53956f);
path.conicTo(34.8506f, 3.09185f, 35.2983f, 2.19767f, 0.707107f);
path.conicTo(35.746f, 1.30349f, 36.6402f, 1.7512f, 0.707107f);
path.close();
path.moveTo(39.2611f, 3.26012f);
path.quadTo(39.4005f, 3.35159f, 39.539f, 3.44501f);
path.quadTo(39.8091f, 3.62717f, 40.0746f, 3.81611f);
path.conicTo(40.8893f, 4.3959f, 40.3096f, 5.21067f, 0.707107f);
path.conicTo(39.7298f, 6.02543f, 38.915f, 5.44564f, 0.707107f);
path.quadTo(38.67f, 5.2713f, 38.4206f, 5.10309f);
path.quadTo(38.293f, 5.017f, 38.164f, 4.9324f);
path.conicTo(37.3279f, 4.38388f, 37.8764f, 3.54775f, 0.707107f);
path.conicTo(38.4249f, 2.71161f, 39.2611f, 3.26012f, 0.707107f);
path.close();
path.moveTo(41.6673f, 5.04503f);
path.quadTo(42.0618f, 5.37449f, 42.4428f, 5.71927f);
path.conicTo(43.1844f, 6.39015f, 42.5135f, 7.13171f, 0.707107f);
path.conicTo(41.8426f, 7.87327f, 41.1011f, 7.20239f, 0.707107f);
path.quadTo(40.7493f, 6.88414f, 40.3852f, 6.58004f);
path.conicTo(39.6177f, 5.93899f, 40.2588f, 5.17149f, 0.707107f);
path.conicTo(40.8998f, 4.40399f, 41.6673f, 5.04503f, 0.707107f);
path.close();
path.moveTo(43.9388f, 7.1865f);
path.quadTo(44.3044f, 7.57519f, 44.6538f, 7.97856f);
path.conicTo(45.3084f, 8.73448f, 44.5525f, 9.38914f, 0.707107f);
path.conicTo(43.7966f, 10.0438f, 43.1419f, 9.28789f, 0.707107f);
path.quadTo(42.8195f, 8.91555f, 42.482f, 8.55677f);
path.conicTo(41.7969f, 7.82836f, 42.5253f, 7.14322f, 0.707107f);
path.conicTo(43.2537f, 6.45808f, 43.9388f, 7.1865f, 0.707107f);
path.close();
path.moveTo(46.0036f, 9.6753f);
path.quadTo(46.3207f, 10.1098f, 46.6195f, 10.5571f);
path.conicTo(47.175f, 11.3886f, 46.3435f, 11.9441f, 0.707107f);
path.conicTo(45.5119f, 12.4996f, 44.9564f, 11.6681f, 0.707107f);
path.quadTo(44.6806f, 11.2552f, 44.388f, 10.8541f);
path.conicTo(43.7986f, 10.0463f, 44.6064f, 9.45688f, 0.707107f);
path.conicTo(45.4142f, 8.86747f, 46.0036f, 9.6753f, 0.707107f);
path.close();
path.moveTo(47.6932f, 12.3107f);
path.quadTo(47.9467f, 12.764f, 48.1819f, 13.2271f);
path.conicTo(48.6347f, 14.1187f, 47.7431f, 14.5715f, 0.707107f);
path.conicTo(46.8514f, 15.0243f, 46.3986f, 14.1327f, 0.707107f);
path.quadTo(46.1816f, 13.7053f, 45.9476f, 13.2868f);
path.conicTo(45.4595f, 12.414f, 46.3323f, 11.9259f, 0.707107f);
path.conicTo(47.2051f, 11.4379f, 47.6932f, 12.3107f, 0.707107f);
path.close();
path.moveTo(49.0539f, 15.1303f);
path.quadTo(49.2539f, 15.6178f, 49.434f, 16.113f);
path.conicTo(49.7758f, 17.0527f, 48.836f, 17.3946f, 0.707107f);
path.conicTo(47.8963f, 17.7364f, 47.5545f, 16.7966f, 0.707107f);
path.quadTo(47.3882f, 16.3395f, 47.2036f, 15.8895f);
path.conicTo(46.824f, 14.9643f, 47.7491f, 14.5847f, 0.707107f);
path.conicTo(48.6743f, 14.2051f, 49.0539f, 15.1303f, 0.707107f);
path.close();
path.moveTo(50.0758f, 18.1294f);
path.quadTo(50.216f, 18.6412f, 50.3352f, 19.1584f);
path.conicTo(50.5599f, 20.1328f, 49.5855f, 20.3575f, 0.707107f);
path.conicTo(48.6111f, 20.5821f, 48.3864f, 19.6077f, 0.707107f);
path.quadTo(48.2763f, 19.1304f, 48.1469f, 18.6579f);
path.conicTo(47.8826f, 17.6935f, 48.8471f, 17.4292f, 0.707107f);
path.conicTo(49.8115f, 17.165f, 50.0758f, 18.1294f, 0.707107f);
path.close();
path.moveTo(50.7247f, 21.2262f);
path.quadTo(50.8005f, 21.743f, 50.8555f, 22.2623f);
path.conicTo(50.9607f, 23.2568f, 49.9663f, 23.3621f, 0.707107f);
path.conicTo(48.9719f, 23.4673f, 48.8666f, 22.4729f, 0.707107f);
path.quadTo(48.8158f, 21.9935f, 48.7458f, 21.5165f);
path.conicTo(48.6007f, 20.5271f, 49.5901f, 20.382f, 0.707107f);
path.conicTo(50.5795f, 20.2368f, 50.7247f, 21.2262f, 0.707107f);
path.close();
path.moveTo(50.9916f, 24.3398f);
path.quadTo(51.0048f, 24.858f, 50.9973f, 25.3762f);
path.conicTo(50.9828f, 26.3761f, 49.9829f, 26.3616f, 0.707107f);
path.conicTo(48.983f, 26.3472f, 48.9975f, 25.3473f, 0.707107f);
path.quadTo(49.0044f, 24.8687f, 48.9923f, 24.3906f);
path.conicTo(48.9669f, 23.3909f, 49.9665f, 23.3655f, 0.707107f);
path.conicTo(50.9662f, 23.3401f, 50.9916f, 24.3398f, 0.707107f);
path.close();
path.moveTo(50.8819f, 27.4753f);
path.quadTo(50.8323f, 27.9943f, 50.7618f, 28.511f);
path.conicTo(50.6268f, 29.5018f, 49.636f, 29.3668f, 0.707107f);
path.conicTo(48.6451f, 29.2317f, 48.7802f, 28.2409f, 0.707107f);
path.quadTo(48.8452f, 27.7641f, 48.891f, 27.2849f);
path.conicTo(48.9862f, 26.2894f, 49.9816f, 26.3846f, 0.707107f);
path.conicTo(50.9771f, 26.4798f, 50.8819f, 27.4753f, 0.707107f);
path.close();
path.moveTo(50.4023f, 30.5429f);
path.quadTo(50.2856f, 31.0775f, 50.1465f, 31.607f);
path.conicTo(49.8924f, 32.5742f, 48.9252f, 32.3201f, 0.707107f);
path.conicTo(47.9581f, 32.066f, 48.2122f, 31.0988f, 0.707107f);
path.quadTo(48.3405f, 30.6102f, 48.4483f, 30.1165f);
path.conicTo(48.6614f, 29.1395f, 49.6385f, 29.3527f, 0.707107f);
path.conicTo(50.6155f, 29.5659f, 50.4023f, 30.5429f, 0.707107f);
path.close();
path.moveTo(49.5104f, 33.674f);
path.quadTo(49.3329f, 34.1756f, 49.1351f, 34.6695f);
path.conicTo(48.7632f, 35.5977f, 47.8349f, 35.2258f, 0.707107f);
path.conicTo(46.9066f, 34.854f, 47.2785f, 33.9257f, 0.707107f);
path.quadTo(47.4612f, 33.4697f, 47.625f, 33.0067f);
path.conicTo(47.9587f, 32.064f, 48.9014f, 32.3977f, 0.707107f);
path.conicTo(49.8441f, 32.7313f, 49.5104f, 33.674f, 0.707107f);
path.close();
path.moveTo(48.281f, 36.5756f);
path.quadTo(48.053f, 37.0342f, 47.8071f, 37.4835f);
path.conicTo(47.3269f, 38.3607f, 46.4497f, 37.8805f, 0.707107f);
path.conicTo(45.5725f, 37.4004f, 46.0527f, 36.5232f, 0.707107f);
path.quadTo(46.2797f, 36.1085f, 46.4901f, 35.6852f);
path.conicTo(46.9353f, 34.7898f, 47.8307f, 35.235f, 0.707107f);
path.conicTo(48.7262f, 35.6802f, 48.281f, 36.5756f, 0.707107f);
path.close();
path.moveTo(46.7777f, 39.2033f);
path.quadTo(46.6677f, 39.3719f, 46.555f, 39.539f);
path.quadTo(46.3865f, 39.7888f, 46.2121f, 40.0349f);
path.conicTo(45.6338f, 40.8507f, 44.818f, 40.2724f, 0.707107f);
path.conicTo(44.0021f, 39.6942f, 44.5804f, 38.8783f, 0.707107f);
path.quadTo(44.7413f, 38.6513f, 44.8969f, 38.4206f);
path.quadTo(45.0008f, 38.2665f, 45.1025f, 38.1107f);
path.conicTo(45.6488f, 37.2731f, 46.4864f, 37.8194f, 0.707107f);
path.conicTo(47.324f, 38.3657f, 46.7777f, 39.2033f, 0.707107f);
path.close();
path.moveTo(44.9527f, 41.6701f);
path.quadTo(44.6177f, 42.0709f, 44.267f, 42.458f);
path.conicTo(43.5955f, 43.1991f, 42.8545f, 42.5276f, 0.707107f);
path.conicTo(42.1135f, 41.8561f, 42.7849f, 41.1151f, 0.707107f);
path.quadTo(43.1087f, 40.7578f, 43.4178f, 40.3878f);
path.conicTo(44.059f, 39.6203f, 44.8264f, 40.2615f, 0.707107f);
path.conicTo(45.5938f, 40.9027f, 44.9527f, 41.6701f, 0.707107f);
path.close();
path.moveTo(42.7884f, 43.9624f);
path.quadTo(42.4083f, 44.319f, 42.014f, 44.6602f);
path.conicTo(41.2578f, 45.3146f, 40.6034f, 44.5585f, 0.707107f);
path.conicTo(39.949f, 43.8023f, 40.7052f, 43.1479f, 0.707107f);
path.quadTo(41.0691f, 42.833f, 41.4201f, 42.5037f);
path.conicTo(42.1494f, 41.8196f, 42.8336f, 42.5489f, 0.707107f);
path.conicTo(43.5178f, 43.2782f, 42.7884f, 43.9624f, 0.707107f);
path.close();
path.moveTo(40.3892f, 45.9564f);
path.quadTo(39.9683f, 46.2655f, 39.5354f, 46.5574f);
path.conicTo(38.7062f, 47.1165f, 38.1472f, 46.2873f, 0.707107f);
path.conicTo(37.5881f, 45.4582f, 38.4173f, 44.8992f, 0.707107f);
path.quadTo(38.8169f, 44.6297f, 39.2054f, 44.3444f);
path.conicTo(40.0114f, 43.7525f, 40.6033f, 44.5585f, 0.707107f);
path.conicTo(41.1952f, 45.3645f, 40.3892f, 45.9564f, 0.707107f);
path.close();
path.moveTo(37.7543f, 47.6568f);
path.quadTo(37.2977f, 47.9138f, 36.8312f, 48.1522f);
path.conicTo(35.9407f, 48.6072f, 35.4857f, 47.7167f, 0.707107f);
path.conicTo(35.0306f, 46.8263f, 35.9211f, 46.3712f, 0.707107f);
path.quadTo(36.3518f, 46.1511f, 36.7732f, 45.9139f);
path.conicTo(37.6446f, 45.4234f, 38.1351f, 46.2948f, 0.707107f);
path.conicTo(38.6257f, 47.1662f, 37.7543f, 47.6568f, 0.707107f);
path.close();
path.moveTo(34.9311f, 49.0286f);
path.quadTo(34.4488f, 49.2279f, 33.9589f, 49.4077f);
path.conicTo(33.0202f, 49.7523f, 32.6756f, 48.8136f, 0.707107f);
path.conicTo(32.331f, 47.8748f, 33.2698f, 47.5302f, 0.707107f);
path.quadTo(33.722f, 47.3642f, 34.1672f, 47.1802f);
path.conicTo(35.0914f, 46.7983f, 35.4733f, 47.7224f, 0.707107f);
path.conicTo(35.8553f, 48.6466f, 34.9311f, 49.0286f, 0.707107f);
path.close();
path.moveTo(31.9824f, 50.0449f);
path.quadTo(31.4774f, 50.1857f, 30.9668f, 50.3061f);
path.conicTo(29.9935f, 50.5355f, 29.764f, 49.5622f, 0.707107f);
path.conicTo(29.5346f, 48.5889f, 30.5079f, 48.3594f, 0.707107f);
path.quadTo(30.9789f, 48.2484f, 31.4453f, 48.1184f);
path.conicTo(32.4086f, 47.8498f, 32.6771f, 48.8131f, 0.707107f);
path.conicTo(32.9457f, 49.7763f, 31.9824f, 50.0449f, 0.707107f);
path.close();
path.moveTo(28.899f, 50.706f);
path.quadTo(28.3834f, 50.7842f, 27.8652f, 50.8416f);
path.conicTo(26.8713f, 50.9518f, 26.7611f, 49.9579f, 0.707107f);
path.conicTo(26.6509f, 48.964f, 27.6448f, 48.8538f, 0.707107f);
path.quadTo(28.1231f, 48.8008f, 28.599f, 48.7286f);
path.conicTo(29.5877f, 48.5786f, 29.7377f, 49.5673f, 0.707107f);
path.conicTo(29.8877f, 50.556f, 28.899f, 50.706f, 0.707107f);
path.close();
path.moveTo(25.8106f, 50.9874f);
path.quadTo(25.6321f, 50.9929f, 25.4537f, 50.996f);
path.conicTo(24.4539f, 51.0135f, 24.4365f, 50.0136f, 0.707115f);
path.lineTo(24.4251f, 49.3638f);
path.conicTo(24.4077f, 48.364f, 25.4075f, 48.3465f, 0.707107f);
path.conicTo(26.4073f, 48.3291f, 26.4248f, 49.3289f, 0.707107f);
path.lineTo(26.4361f, 49.9787f);
path.lineTo(25.4363f, 49.9962f);
path.lineTo(25.4189f, 48.9963f);
path.quadTo(25.5836f, 48.9935f, 25.7482f, 48.9883f);
path.conicTo(26.7477f, 48.9571f, 26.7789f, 49.9567f, 0.707107f);
path.conicTo(26.8101f, 50.9562f, 25.8106f, 50.9874f, 0.707107f);
path.close();
path.moveTo(24.3902f, 47.3641f);
path.lineTo(24.3728f, 46.3643f);
path.conicTo(24.3553f, 45.3645f, 25.3551f, 45.347f, 0.707107f);
path.conicTo(26.355f, 45.3295f, 26.3724f, 46.3294f, 0.707107f);
path.lineTo(26.3899f, 47.3292f);
path.conicTo(26.4074f, 48.3291f, 25.4075f, 48.3465f, 0.707107f);
path.conicTo(24.4077f, 48.364f, 24.3902f, 47.3641f, 0.707107f);
path.close();
path.moveTo(24.3378f, 44.3646f);
path.lineTo(24.3204f, 43.3648f);
path.conicTo(24.3029f, 42.3649f, 25.3028f, 42.3475f, 0.707107f);
path.conicTo(26.3026f, 42.33f, 26.3201f, 43.3298f, 0.707107f);
path.lineTo(26.3375f, 44.3297f);
path.conicTo(26.355f, 45.3295f, 25.3551f, 45.347f, 0.707107f);
path.conicTo(24.3553f, 45.3645f, 24.3378f, 44.3646f, 0.707107f);
path.close();
path.moveTo(24.2855f, 41.3651f);
path.lineTo(24.268f, 40.3652f);
path.conicTo(24.2506f, 39.3654f, 25.2504f, 39.3479f, 0.707107f);
path.conicTo(26.2503f, 39.3305f, 26.2677f, 40.3303f, 0.707107f);
path.lineTo(26.2852f, 41.3302f);
path.conicTo(26.3026f, 42.33f, 25.3028f, 42.3475f, 0.707107f);
path.conicTo(24.3029f, 42.3649f, 24.2855f, 41.3651f, 0.707107f);
path.close();
path.moveTo(24.2331f, 38.3655f);
path.lineTo(24.2157f, 37.3657f);
path.conicTo(24.1982f, 36.3658f, 25.1981f, 36.3484f, 0.707107f);
path.conicTo(26.1979f, 36.3309f, 26.2154f, 37.3308f, 0.707107f);
path.lineTo(26.2328f, 38.3306f);
path.conicTo(26.2503f, 39.3305f, 25.2504f, 39.3479f, 0.707107f);
path.conicTo(24.2506f, 39.3654f, 24.2331f, 38.3655f, 0.707107f);
path.close();
path.moveTo(24.1808f, 35.366f);
path.lineTo(24.1633f, 34.3661f);
path.conicTo(24.1459f, 33.3663f, 25.1457f, 33.3488f, 0.707107f);
path.conicTo(26.1456f, 33.3314f, 26.163f, 34.3312f, 0.707107f);
path.lineTo(26.1805f, 35.3311f);
path.conicTo(26.1979f, 36.3309f, 25.1981f, 36.3484f, 0.707107f);
path.conicTo(24.1982f, 36.3658f, 24.1808f, 35.366f, 0.707107f);
path.close();
path.moveTo(24.1284f, 32.3664f);
path.lineTo(24.111f, 31.3666f);
path.conicTo(24.0935f, 30.3667f, 25.0934f, 30.3493f, 0.707107f);
path.conicTo(26.0932f, 30.3318f, 26.1107f, 31.3317f, 0.707107f);
path.lineTo(26.1281f, 32.3315f);
path.conicTo(26.1456f, 33.3314f, 25.1457f, 33.3488f, 0.707107f);
path.conicTo(24.1459f, 33.3663f, 24.1284f, 32.3664f, 0.707107f);
path.close();
path.moveTo(24.0761f, 29.3669f);
path.lineTo(24.0586f, 28.367f);
path.conicTo(24.0412f, 27.3672f, 25.041f, 27.3497f, 0.707107f);
path.conicTo(26.0409f, 27.3323f, 26.0583f, 28.3321f, 0.707107f);
path.lineTo(26.0758f, 29.332f);
path.conicTo(26.0932f, 30.3318f, 25.0934f, 30.3493f, 0.707107f);
path.conicTo(24.0935f, 30.3667f, 24.0761f, 29.3669f, 0.707107f);
path.close();
path.moveTo(24.0237f, 26.3673f);
path.lineTo(24.0063f, 25.3675f);
path.conicTo(23.9888f, 24.3676f, 24.9887f, 24.3502f, 0.707107f);
path.conicTo(25.9885f, 24.3327f, 26.006f, 25.3326f, 0.707107f);
path.lineTo(26.0234f, 26.3324f);
path.conicTo(26.0409f, 27.3323f, 25.041f, 27.3497f, 0.707107f);
path.conicTo(24.0412f, 27.3672f, 24.0237f, 26.3673f, 0.707107f);
path.close();
path1 = path;
path.reset();
path.setFillType(SkPath::kWinding_FillType);
path.moveTo(25.0098f, 23.1973f);
path.lineTo(25.5689f, 22.3682f);
path.conicTo(26.1281f, 21.5392f, 26.9572f, 22.0984f, 0.707107f);
path.conicTo(27.7862f, 22.6576f, 27.227f, 23.4866f, 0.707107f);
path.lineTo(26.6678f, 24.3156f);
path.conicTo(26.1086f, 25.1447f, 25.2796f, 24.5855f, 0.707107f);
path.conicTo(24.4506f, 24.0263f, 25.0098f, 23.1973f, 0.707107f);
path.close();
path.moveTo(26.6873f, 20.7101f);
path.lineTo(27.2465f, 19.8811f);
path.conicTo(27.8057f, 19.0521f, 28.6348f, 19.6113f, 0.707107f);
path.conicTo(29.4638f, 20.1704f, 28.9046f, 20.9995f, 0.707107f);
path.lineTo(28.3454f, 21.8285f);
path.conicTo(27.7862f, 22.6576f, 26.9572f, 22.0984f, 0.707107f);
path.conicTo(26.1281f, 21.5392f, 26.6873f, 20.7101f, 0.707107f);
path.close();
path.moveTo(28.3649f, 18.223f);
path.lineTo(28.9241f, 17.394f);
path.conicTo(29.4833f, 16.565f, 30.3123f, 17.1241f, 0.707107f);
path.conicTo(31.1414f, 17.6833f, 30.5822f, 18.5124f, 0.707107f);
path.lineTo(30.023f, 19.3414f);
path.conicTo(29.4638f, 20.1704f, 28.6348f, 19.6113f, 0.707107f);
path.conicTo(27.8057f, 19.0521f, 28.3649f, 18.223f, 0.707107f);
path.close();
path.moveTo(30.0425f, 15.7359f);
path.lineTo(30.6017f, 14.9069f);
path.conicTo(31.1609f, 14.0778f, 31.9899f, 14.637f, 0.707107f);
path.conicTo(32.8189f, 15.1962f, 32.2598f, 16.0253f, 0.707107f);
path.lineTo(31.7006f, 16.8543f);
path.conicTo(31.1414f, 17.6833f, 30.3123f, 17.1241f, 0.707107f);
path.conicTo(29.4833f, 16.565f, 30.0425f, 15.7359f, 0.707107f);
path.close();
path.moveTo(31.7201f, 13.2488f);
path.lineTo(32.2793f, 12.4198f);
path.conicTo(32.8385f, 11.5907f, 33.6675f, 12.1499f, 0.707107f);
path.conicTo(34.4965f, 12.7091f, 33.9373f, 13.5381f, 0.707107f);
path.lineTo(33.3781f, 14.3672f);
path.conicTo(32.8189f, 15.1962f, 31.9899f, 14.637f, 0.707107f);
path.conicTo(31.1609f, 14.0778f, 31.7201f, 13.2488f, 0.707107f);
path.close();
path.moveTo(33.3976f, 10.7617f);
path.lineTo(33.9568f, 9.93265f);
path.conicTo(34.516f, 9.10361f, 35.3451f, 9.6628f, 0.707107f);
path.conicTo(36.1741f, 10.222f, 35.6149f, 11.051f, 0.707107f);
path.lineTo(35.0557f, 11.8801f);
path.conicTo(34.4965f, 12.7091f, 33.6675f, 12.1499f, 0.707107f);
path.conicTo(32.8385f, 11.5907f, 33.3976f, 10.7617f, 0.707107f);
path.close();
path.moveTo(35.0752f, 8.27457f);
path.lineTo(35.6344f, 7.44554f);
path.conicTo(36.1936f, 6.6165f, 37.0226f, 7.17569f, 0.707107f);
path.conicTo(37.8517f, 7.73488f, 37.2925f, 8.56392f, 0.707107f);
path.lineTo(36.7333f, 9.39296f);
path.conicTo(36.1741f, 10.222f, 35.3451f, 9.6628f, 0.707107f);
path.conicTo(34.516f, 9.10361f, 35.0752f, 8.27457f, 0.707107f);
path.close();
path.moveTo(36.7528f, 5.78746f);
path.lineTo(37.312f, 4.95842f);
path.conicTo(37.8712f, 4.12939f, 38.7002f, 4.68858f, 0.707107f);
path.conicTo(39.5293f, 5.24777f, 38.9701f, 6.07681f, 0.707107f);
path.lineTo(38.4109f, 6.90585f);
path.conicTo(37.8517f, 7.73488f, 37.0226f, 7.17569f, 0.707107f);
path.conicTo(36.1936f, 6.6165f, 36.7528f, 5.78746f, 0.707107f);
path.close();
path.moveTo(39.9447f, 3.72429f);
path.quadTo(40.3524f, 4.01069f, 40.7489f, 4.31248f);
path.conicTo(41.5445f, 4.9182f, 40.9388f, 5.71387f, 0.707107f);
path.conicTo(40.3331f, 6.50955f, 39.5374f, 5.90383f, 0.707107f);
path.quadTo(39.1714f, 5.62521f, 38.7951f, 5.36088f);
path.conicTo(37.9768f, 4.78608f, 38.5516f, 3.96779f, 0.707107f);
path.conicTo(39.1264f, 3.14949f, 39.9447f, 3.72429f, 0.707107f);
path.close();
path.moveTo(42.3194f, 5.60826f);
path.quadTo(42.707f, 5.95446f, 43.0804f, 6.31583f);
path.conicTo(43.7991f, 7.01122f, 43.1037f, 7.72985f, 0.707107f);
path.conicTo(42.4083f, 8.44848f, 41.6896f, 7.75308f, 0.707107f);
path.quadTo(41.3448f, 7.41944f, 40.9871f, 7.09992f);
path.conicTo(40.2413f, 6.43379f, 40.9074f, 5.68796f, 0.707107f);
path.conicTo(41.5735f, 4.94212f, 42.3194f, 5.60826f, 0.707107f);
path.close();
path.moveTo(44.5406f, 7.84871f);
path.quadTo(44.8959f, 8.25352f, 45.2341f, 8.67266f);
path.conicTo(45.862f, 9.4509f, 45.0838f, 10.0789f, 0.707107f);
path.conicTo(44.3056f, 10.7068f, 43.6776f, 9.9286f, 0.707107f);
path.quadTo(43.3654f, 9.54174f, 43.0374f, 9.16805f);
path.conicTo(42.3778f, 8.41649f, 43.1293f, 7.75682f, 0.707107f);
path.conicTo(43.8809f, 7.09715f, 44.5406f, 7.84871f, 0.707107f);
path.close();
path.moveTo(46.528f, 10.4211f);
path.quadTo(46.815f, 10.8449f, 47.0851f, 11.2796f);
path.conicTo(47.6128f, 12.129f, 46.7633f, 12.6567f, 0.707107f);
path.conicTo(45.9139f, 13.1844f, 45.3862f, 12.335f, 0.707107f);
path.quadTo(45.1369f, 11.9337f, 44.872f, 11.5426f);
path.conicTo(44.3113f, 10.7146f, 45.1393f, 10.1538f, 0.707107f);
path.conicTo(45.9673f, 9.5931f, 46.528f, 10.4211f, 0.707107f);
path.close();
path.moveTo(48.1056f, 13.0782f);
path.quadTo(48.3449f, 13.542f, 48.5654f, 14.015f);
path.conicTo(48.9879f, 14.9213f, 48.0816f, 15.3438f, 0.707107f);
path.conicTo(47.1752f, 15.7663f, 46.7527f, 14.86f, 0.707107f);
path.quadTo(46.5492f, 14.4234f, 46.3283f, 13.9953f);
path.conicTo(45.8698f, 13.1066f, 46.7584f, 12.6481f, 0.707107f);
path.conicTo(47.6471f, 12.1895f, 48.1056f, 13.0782f, 0.707107f);
path.close();
path.moveTo(49.3755f, 15.9538f);
path.quadTo(49.5594f, 16.4493f, 49.7229f, 16.9516f);
path.conicTo(50.0325f, 17.9025f, 49.0816f, 18.2121f, 0.707107f);
path.conicTo(48.1307f, 18.5216f, 47.8212f, 17.5707f, 0.707107f);
path.quadTo(47.6702f, 17.1069f, 47.5005f, 16.6497f);
path.conicTo(47.1526f, 15.7122f, 48.0901f, 15.3642f, 0.707107f);
path.conicTo(49.0276f, 15.0163f, 49.3755f, 15.9538f, 0.707107f);
path.close();
path.moveTo(50.2964f, 18.9923f);
path.quadTo(50.4191f, 19.5089f, 50.5206f, 20.0302f);
path.conicTo(50.7117f, 21.0117f, 49.7302f, 21.2029f, 0.707107f);
path.conicTo(48.7486f, 21.394f, 48.5575f, 20.4125f, 0.707107f);
path.quadTo(48.4638f, 19.9313f, 48.3505f, 19.4544f);
path.conicTo(48.1194f, 18.4815f, 49.0924f, 18.2504f, 0.707107f);
path.conicTo(50.0653f, 18.0193f, 50.2964f, 18.9923f, 0.707107f);
path.close();
path.moveTo(50.8373f, 22.0956f);
path.quadTo(50.8955f, 22.6138f, 50.933f, 23.1341f);
path.conicTo(51.0047f, 24.1315f, 50.0073f, 24.2033f, 0.707107f);
path.conicTo(49.0099f, 24.275f, 48.9381f, 23.2776f, 0.707107f);
path.quadTo(48.9036f, 22.7975f, 48.8498f, 22.3191f);
path.conicTo(48.7381f, 21.3253f, 49.7318f, 21.2136f, 0.707107f);
path.conicTo(50.7255f, 21.1019f, 50.8373f, 22.0956f, 0.707107f);
path.close();
path.moveTo(50.9992f, 25.2099f);
path.quadTo(50.9949f, 25.7358f, 50.9694f, 26.2608f);
path.conicTo(50.9209f, 27.2596f, 49.9221f, 27.2111f, 0.707107f);
path.conicTo(48.9233f, 27.1626f, 48.9718f, 26.1638f, 0.707107f);
path.quadTo(48.9953f, 25.679f, 48.9992f, 25.1938f);
path.conicTo(49.0073f, 24.1938f, 50.0073f, 24.2019f, 0.707107f);
path.conicTo(51.0072f, 24.21f, 50.9992f, 25.2099f, 0.707107f);
path.close();
path.moveTo(50.7839f, 28.3454f);
path.quadTo(50.7172f, 28.8596f, 50.63f, 29.3708f);
path.conicTo(50.4619f, 30.3565f, 49.4761f, 30.1884f, 0.707107f);
path.conicTo(48.4903f, 30.0203f, 48.6584f, 29.0346f, 0.707107f);
path.quadTo(48.7389f, 28.5627f, 48.8005f, 28.088f);
path.conicTo(48.9292f, 27.0963f, 49.9209f, 27.225f, 0.707107f);
path.conicTo(50.9126f, 27.3537f, 50.7839f, 28.3454f, 0.707107f);
path.close();
path.moveTo(50.1906f, 31.437f);
path.quadTo(50.0558f, 31.9646f, 49.899f, 32.4861f);
path.conicTo(49.611f, 33.4438f, 48.6534f, 33.1558f, 0.707107f);
path.conicTo(47.6957f, 32.8679f, 47.9837f, 31.9103f, 0.707107f);
path.quadTo(48.1284f, 31.4289f, 48.2528f, 30.9418f);
path.conicTo(48.5004f, 29.9729f, 49.4693f, 30.2205f, 0.707107f);
path.conicTo(50.4382f, 30.4681f, 50.1906f, 31.437f, 0.707107f);
path.close();
path.moveTo(49.1978f, 34.5114f);
path.quadTo(49.0051f, 35.0016f, 48.7927f, 35.4837f);
path.conicTo(48.3895f, 36.3988f, 47.4744f, 35.9956f, 0.707107f);
path.conicTo(46.5593f, 35.5923f, 46.9625f, 34.6772f, 0.707107f);
path.quadTo(47.1586f, 34.2323f, 47.3364f, 33.7797f);
path.conicTo(47.7023f, 32.849f, 48.6329f, 33.2149f, 0.707107f);
path.conicTo(49.5636f, 33.5807f, 49.1978f, 34.5114f, 0.707107f);
path.close();
path.moveTo(47.8852f, 37.3397f);
path.quadTo(47.6449f, 37.7853f, 47.3876f, 38.2211f);
path.conicTo(46.879f, 39.0821f, 46.018f, 38.5736f, 0.707107f);
path.conicTo(45.1569f, 38.0651f, 45.6655f, 37.204f, 0.707107f);
path.quadTo(45.903f, 36.8018f, 46.1248f, 36.3906f);
path.conicTo(46.5993f, 35.5103f, 47.4796f, 35.9849f, 0.707107f);
path.conicTo(48.3598f, 36.4595f, 47.8852f, 37.3397f, 0.707107f);
path.close();
path.moveTo(46.3154f, 39.8881f);
path.quadTo(46.0303f, 40.2962f, 45.7299f, 40.693f);
path.conicTo(45.1264f, 41.4903f, 44.3291f, 40.8867f, 0.707107f);
path.conicTo(43.5318f, 40.2831f, 44.1353f, 39.4858f, 0.707107f);
path.quadTo(44.4126f, 39.1195f, 44.6757f, 38.7428f);
path.conicTo(45.2483f, 37.923f, 46.0682f, 38.4956f, 0.707107f);
path.conicTo(46.888f, 39.0682f, 46.3154f, 39.8881f, 0.707107f);
path.close();
path.moveTo(44.4398f, 42.2654f);
path.quadTo(44.095f, 42.6536f, 43.7349f, 43.0278f);
path.conicTo(43.0415f, 43.7484f, 42.321f, 43.055f, 0.707107f);
path.conicTo(41.6004f, 42.3616f, 42.2938f, 41.641f, 0.707107f);
path.quadTo(42.6261f, 41.2957f, 42.9444f, 40.9374f);
path.conicTo(43.6084f, 40.1897f, 44.3561f, 40.8537f, 0.707107f);
path.conicTo(45.1038f, 41.5177f, 44.4398f, 42.2654f, 0.707107f);
path.close();
path.moveTo(42.2075f, 44.4911f);
path.quadTo(41.804f, 44.8473f, 41.3862f, 45.1865f);
path.conicTo(40.6098f, 45.8167f, 39.9795f, 45.0403f, 0.707107f);
path.conicTo(39.3493f, 44.2639f, 40.1257f, 43.6336f, 0.707107f);
path.quadTo(40.5114f, 43.3205f, 40.8838f, 42.9918f);
path.conicTo(41.6335f, 42.3299f, 42.2953f, 43.0796f, 0.707107f);
path.conicTo(42.9572f, 43.8292f, 42.2075f, 44.4911f, 0.707107f);
path.close();
path.moveTo(39.6379f, 46.488f);
path.quadTo(39.2151f, 46.776f, 38.7814f, 47.0471f);
path.conicTo(37.9334f, 47.5771f, 37.4034f, 46.7292f, 0.707107f);
path.conicTo(36.8733f, 45.8812f, 37.7213f, 45.3511f, 0.707107f);
path.quadTo(38.1217f, 45.1009f, 38.5119f, 44.835f);
path.conicTo(39.3383f, 44.2721f, 39.9013f, 45.0985f, 0.707107f);
path.conicTo(40.4643f, 45.925f, 39.6379f, 46.488f, 0.707107f);
path.close();
path.moveTo(36.9864f, 48.0722f);
path.quadTo(36.5234f, 48.3127f, 36.0513f, 48.5344f);
path.conicTo(35.1461f, 48.9595f, 34.7211f, 48.0543f, 0.707107f);
path.conicTo(34.296f, 47.1491f, 35.2012f, 46.7241f, 0.707107f);
path.quadTo(35.6371f, 46.5194f, 36.0644f, 46.2974f);
path.conicTo(36.9518f, 45.8364f, 37.4128f, 46.7238f, 0.707107f);
path.conicTo(37.8738f, 47.6112f, 36.9864f, 48.0722f, 0.707107f);
path.close();
path.moveTo(34.1153f, 49.3498f);
path.quadTo(33.6206f, 49.535f, 33.1187f, 49.6999f);
path.conicTo(32.1687f, 50.0122f, 31.8565f, 49.0622f, 0.707107f);
path.conicTo(31.5442f, 48.1122f, 32.4942f, 47.7999f, 0.707107f);
path.quadTo(32.9575f, 47.6477f, 33.4141f, 47.4767f);
path.conicTo(34.3507f, 47.1261f, 34.7012f, 48.0627f, 0.707107f);
path.conicTo(35.0518f, 48.9992f, 34.1153f, 49.3498f, 0.707107f);
path.close();
path.moveTo(31.08f, 50.2791f);
path.quadTo(30.5637f, 50.4033f, 30.0427f, 50.5063f);
path.conicTo(29.0617f, 50.7002f, 28.8678f, 49.7192f, 0.707107f);
path.conicTo(28.6738f, 48.7382f, 29.6548f, 48.5443f, 0.707107f);
path.quadTo(30.1357f, 48.4492f, 30.6122f, 48.3346f);
path.conicTo(31.5845f, 48.1007f, 31.8184f, 49.073f, 0.707107f);
path.conicTo(32.0522f, 50.0453f, 31.08f, 50.2791f, 0.707107f);
path.close();
path.moveTo(27.9769f, 50.829f);
path.quadTo(27.4588f, 50.8887f, 26.9386f, 50.9276f);
path.conicTo(25.9414f, 51.0022f, 25.8668f, 50.005f, 0.707107f);
path.conicTo(25.7923f, 49.0078f, 26.7895f, 48.9332f, 0.707107f);
path.quadTo(27.2696f, 48.8973f, 27.7479f, 48.8422f);
path.conicTo(28.7413f, 48.7277f, 28.8558f, 49.7211f, 0.707107f);
path.conicTo(28.9703f, 50.7145f, 27.9769f, 50.829f, 0.707107f);
path.close();
path.moveTo(24.8625f, 50.9996f);
path.quadTo(24.3373f, 50.9969f, 23.8128f, 50.9729f);
path.conicTo(22.8138f, 50.9272f, 22.8595f, 49.9283f, 0.707107f);
path.conicTo(22.9051f, 48.9293f, 23.9041f, 48.975f, 0.707107f);
path.quadTo(24.3884f, 48.9971f, 24.8731f, 48.9997f);
path.conicTo(25.8731f, 49.005f, 25.8678f, 50.005f, 0.707107f);
path.conicTo(25.8624f, 51.0049f, 24.8625f, 50.9996f, 0.707107f);
path.close();
path.moveTo(21.7268f, 50.7931f);
path.quadTo(21.2121f, 50.7278f, 20.7005f, 50.642f);
path.conicTo(19.7143f, 50.4767f, 19.8796f, 49.4905f, 0.707107f);
path.conicTo(20.045f, 48.5042f, 21.0312f, 48.6696f, 0.707107f);
path.quadTo(21.5036f, 48.7488f, 21.9786f, 48.8091f);
path.conicTo(22.9707f, 48.9349f, 22.8448f, 49.927f, 0.707107f);
path.conicTo(22.7189f, 50.919f, 21.7268f, 50.7931f, 0.707107f);
path.close();
path.moveTo(18.6372f, 50.2094f);
path.quadTo(18.1089f, 50.0761f, 17.5865f, 49.9207f);
path.conicTo(16.628f, 49.6356f, 16.9132f, 48.6771f, 0.707107f);
path.conicTo(17.1983f, 47.7186f, 18.1568f, 48.0037f, 0.707107f);
path.quadTo(18.639f, 48.1472f, 19.1267f, 48.2702f);
path.conicTo(20.0963f, 48.515f, 19.8516f, 49.4846f, 0.707107f);
path.conicTo(19.6068f, 50.4542f, 18.6372f, 50.2094f, 0.707107f);
path.close();
path.moveTo(15.5577f, 49.2248f);
path.quadTo(15.0665f, 49.0334f, 14.5834f, 48.8222f);
path.conicTo(13.6672f, 48.4215f, 14.0678f, 47.5053f, 0.707107f);
path.conicTo(14.4684f, 46.589f, 15.3847f, 46.9897f, 0.707107f);
path.quadTo(15.8306f, 47.1846f, 16.284f, 47.3614f);
path.conicTo(17.2158f, 47.7246f, 16.8526f, 48.6563f, 0.707107f);
path.conicTo(16.4894f, 49.588f, 15.5577f, 49.2248f, 0.707107f);
path.close();
path.moveTo(12.7231f, 47.9189f);
path.quadTo(12.2765f, 47.6797f, 11.8395f, 47.4233f);
path.conicTo(10.9771f, 46.9171f, 11.4833f, 46.0547f, 0.707107f);
path.conicTo(11.9894f, 45.1922f, 12.8519f, 45.6984f, 0.707107f);
path.quadTo(13.2552f, 45.9351f, 13.6675f, 46.156f);
path.conicTo(14.549f, 46.6282f, 14.0768f, 47.5096f, 0.707107f);
path.conicTo(13.6046f, 48.3911f, 12.7231f, 47.9189f, 0.707107f);
path.close();
path.moveTo(10.1686f, 46.3548f);
path.quadTo(9.76024f, 46.0712f, 9.363f, 45.7722f);
path.conicTo(8.56406f, 45.1708f, 9.16549f, 44.3718f, 0.707107f);
path.conicTo(9.76691f, 43.5729f, 10.5658f, 44.1743f, 0.707107f);
path.quadTo(10.9325f, 44.4504f, 11.3095f, 44.7122f);
path.conicTo(12.1308f, 45.2826f, 11.5604f, 46.1039f, 0.707107f);
path.conicTo(10.9899f, 46.9253f, 10.1686f, 46.3548f, 0.707107f);
path.close();
path.moveTo(7.78853f, 44.4876f);
path.quadTo(7.39972f, 44.1442f, 7.02492f, 43.7855f);
path.conicTo(6.3024f, 43.0942f, 6.99374f, 42.3717f, 0.707107f);
path.conicTo(7.68509f, 41.6492f, 8.40761f, 42.3405f, 0.707107f);
path.quadTo(8.7536f, 42.6715f, 9.11249f, 42.9885f);
path.conicTo(9.86201f, 43.6505f, 9.20003f, 44.4f, 0.707107f);
path.conicTo(8.53805f, 45.1496f, 7.78853f, 44.4876f, 0.707107f);
path.close();
path.moveTo(5.55855f, 42.2635f);
path.quadTo(5.20148f, 41.8614f, 4.86131f, 41.4449f);
path.conicTo(4.22883f, 40.6703f, 5.0034f, 40.0378f, 0.707107f);
path.conicTo(5.77797f, 39.4053f, 6.41046f, 40.1799f, 0.707107f);
path.quadTo(6.72443f, 40.5644f, 7.05403f, 40.9356f);
path.conicTo(7.71802f, 41.6833f, 6.97028f, 42.3473f, 0.707107f);
path.conicTo(6.22254f, 43.0113f, 5.55855f, 42.2635f, 0.707107f);
path.close();
path.moveTo(3.55261f, 39.6973f);
path.quadTo(3.26341f, 39.2752f, 2.99107f, 38.8422f);
path.conicTo(2.45867f, 37.9957f, 3.30517f, 37.4633f, 0.707107f);
path.conicTo(4.15167f, 36.9309f, 4.68406f, 37.7774f, 0.707107f);
path.quadTo(4.93548f, 38.1772f, 5.20241f, 38.5667f);
path.conicTo(5.76769f, 39.3916f, 4.94279f, 39.9569f, 0.707107f);
path.conicTo(4.11789f, 40.5222f, 3.55261f, 39.6973f, 0.707107f);
path.close();
path.moveTo(1.96145f, 37.0509f);
path.quadTo(1.71975f, 36.5889f, 1.49677f, 36.1175f);
path.conicTo(1.06917f, 35.2135f, 1.97315f, 34.7859f, 0.707107f);
path.conicTo(2.87712f, 34.3583f, 3.30471f, 35.2623f, 0.707107f);
path.quadTo(3.51053f, 35.6974f, 3.73364f, 36.1239f);
path.conicTo(4.19714f, 37.01f, 3.31105f, 37.4735f, 0.707107f);
path.conicTo(2.42495f, 37.937f, 1.96145f, 37.0509f, 0.707107f);
path.close();
path.moveTo(0.676191f, 34.1844f);
path.quadTo(0.489621f, 33.6902f, 0.323275f, 33.189f);
path.conicTo(0.00831527f, 32.2399f, 0.95742f, 31.9249f, 0.707107f);
path.conicTo(1.90653f, 31.6099f, 2.22149f, 32.559f, 0.707107f);
path.quadTo(2.37504f, 33.0218f, 2.54726f, 33.4779f);
path.conicTo(2.9005f, 34.4134f, 1.96497f, 34.7666f, 0.707107f);
path.conicTo(1.02943f, 35.1199f, 0.676191f, 34.1844f, 0.707107f);
path.close();
path.moveTo(-0.261658f, 31.1521f);
path.quadTo(-0.387304f, 30.6362f, -0.491779f, 30.1156f);
path.conicTo(-0.68853f, 29.1351f, 0.291923f, 28.9384f, 0.707107f);
path.conicTo(1.27238f, 28.7416f, 1.46913f, 29.7221f, 0.707107f);
path.quadTo(1.56557f, 30.2026f, 1.68155f, 30.6789f);
path.conicTo(1.91817f, 31.6505f, 0.946565f, 31.8871f, 0.707107f);
path.conicTo(-0.0250367f, 32.1237f, -0.261658f, 31.1521f, 0.707107f);
path.close();
path.moveTo(-0.820549f, 28.0495f);
path.quadTo(-0.881733f, 27.5314f, -0.922089f, 27.0113f);
path.conicTo(-0.999449f, 26.0143f, -0.00244591f, 25.9369f, 0.707107f);
path.conicTo(0.994557f, 25.8596f, 1.07192f, 26.8566f, 0.707107f);
path.quadTo(1.10917f, 27.3367f, 1.16565f, 27.8149f);
path.conicTo(1.28293f, 28.808f, 0.289834f, 28.9253f, 0.707107f);
path.conicTo(-0.703265f, 29.0426f, -0.820549f, 28.0495f, 0.707107f);
path.close();
path.moveTo(-0.999918f, 24.9349f);
path.quadTo(-0.998605f, 24.4104f, -0.976138f, 23.8863f);
path.conicTo(-0.933305f, 22.8873f, 0.0657772f, 22.9301f, 0.707107f);
path.conicTo(1.06486f, 22.9729f, 1.02203f, 23.972f, 0.707107f);
path.quadTo(1.00129f, 24.4557f, 1.00008f, 24.9399f);
path.conicTo(0.997572f, 25.9399f, -0.0024244f, 25.9374f, 0.707107f);
path.conicTo(-1.00242f, 25.9349f, -0.999918f, 24.9349f, 0.707107f);
path.close();
path.moveTo(-0.802212f, 21.7991f);
path.quadTo(-0.738311f, 21.284f, -0.653903f, 20.7719f);
path.conicTo(-0.491283f, 19.7852f, 0.495406f, 19.9478f, 0.707107f);
path.conicTo(1.48209f, 20.1104f, 1.31948f, 21.0971f, 0.707107f);
path.quadTo(1.24156f, 21.5698f, 1.18257f, 22.0453f);
path.conicTo(1.05946f, 23.0377f, 0.0670681f, 22.9146f, 0.707107f);
path.conicTo(-0.925325f, 22.7915f, -0.802212f, 21.7991f, 0.707107f);
path.close();
path.moveTo(-0.228066f, 18.7115f);
path.quadTo(-0.096172f, 18.1824f, 0.0577899f, 17.6593f);
path.conicTo(0.340124f, 16.7f, 1.29944f, 16.9823f, 0.707107f);
path.conicTo(2.25876f, 17.2646f, 1.97642f, 18.2239f, 0.707107f);
path.quadTo(1.8343f, 18.7068f, 1.71255f, 19.1953f);
path.conicTo(1.47069f, 20.1656f, 0.50038f, 19.9237f, 0.707107f);
path.conicTo(-0.46993f, 19.6819f, -0.228066f, 18.7115f, 0.707107f);
path.close();
path.moveTo(0.74831f, 15.6269f);
path.quadTo(0.938539f, 15.1347f, 1.14857f, 14.6506f);
path.conicTo(1.54662f, 13.7333f, 2.46398f, 14.1313f, 0.707107f);
path.conicTo(3.38135f, 14.5294f, 2.9833f, 15.4467f, 0.707107f);
path.quadTo(2.78942f, 15.8936f, 2.61382f, 16.3479f);
path.conicTo(2.25331f, 17.2806f, 1.32056f, 16.9201f, 0.707107f);
path.conicTo(0.387801f, 16.5596f, 0.74831f, 15.6269f, 0.707107f);
path.close();
path.moveTo(2.04744f, 12.7861f);
path.quadTo(2.28569f, 12.3384f, 2.5412f, 11.9003f);
path.conicTo(3.04504f, 11.0365f, 3.90884f, 11.5403f, 0.707107f);
path.conicTo(4.77264f, 12.0442f, 4.26881f, 12.908f, 0.707107f);
path.quadTo(4.03293f, 13.3123f, 3.81302f, 13.7256f);
path.conicTo(3.34325f, 14.6084f, 2.46046f, 14.1386f, 0.707107f);
path.conicTo(1.57767f, 13.6689f, 2.04744f, 12.7861f, 0.707107f);
path.close();
path.moveTo(3.60589f, 10.2253f);
path.quadTo(3.88812f, 9.81661f, 4.18576f, 9.419f);
path.conicTo(4.78503f, 8.61845f, 5.58558f, 9.21772f, 0.707107f);
path.conicTo(6.38613f, 9.81699f, 5.78686f, 10.6175f, 0.707107f);
path.quadTo(5.51211f, 10.9846f, 5.25159f, 11.3618f);
path.conicTo(4.68333f, 12.1847f, 3.86048f, 11.6164f, 0.707107f);
path.conicTo(3.03763f, 11.0481f, 3.60589f, 10.2253f, 0.707107f);
path.close();
path.moveTo(5.46482f, 7.84259f);
path.quadTo(5.80682f, 7.4532f, 6.16407f, 7.07773f);
path.conicTo(6.85339f, 6.35327f, 7.57785f, 7.04259f, 0.707107f);
path.conicTo(8.30231f, 7.73191f, 7.61299f, 8.45636f, 0.707107f);
path.quadTo(7.28322f, 8.80295f, 6.96752f, 9.16239f);
path.conicTo(6.30762f, 9.91375f, 5.55627f, 9.25385f, 0.707107f);
path.conicTo(4.80492f, 8.59395f, 5.46482f, 7.84259f, 0.707107f);
path.close();
path.moveTo(7.68062f, 5.60827f);
path.quadTo(8.08142f, 5.25031f, 8.49666f, 4.90921f);
path.conicTo(9.26938f, 4.27447f, 9.90412f, 5.04719f, 0.707107f);
path.conicTo(10.5389f, 5.81992f, 9.76614f, 6.45466f, 0.707107f);
path.quadTo(9.38285f, 6.76951f, 9.01289f, 7.09994f);
path.conicTo(8.26705f, 7.76607f, 7.60092f, 7.02024f, 0.707107f);
path.conicTo(6.93479f, 6.2744f, 7.68062f, 5.60827f, 0.707107f);
path.close();
path.moveTo(10.2392f, 3.59627f);
path.quadTo(10.6626f, 3.30433f, 11.0971f, 3.02935f);
path.conicTo(11.9421f, 2.49463f, 12.4768f, 3.33965f, 0.707107f);
path.conicTo(13.0116f, 4.18467f, 12.1666f, 4.7194f, 0.707107f);
path.quadTo(11.7654f, 4.97322f, 11.3747f, 5.24271f);
path.conicTo(10.5515f, 5.81043f, 9.98373f, 4.98721f, 0.707107f);
path.conicTo(9.41601f, 4.16399f, 10.2392f, 3.59627f, 0.707107f);
path.close();
path.moveTo(12.8847f, 1.99524f);
path.quadTo(13.3459f, 1.75234f, 13.8165f, 1.52812f);
path.conicTo(14.7193f, 1.09799f, 15.1494f, 2.00075f, 0.707107f);
path.conicTo(15.5795f, 2.90352f, 14.6768f, 3.33365f, 0.707107f);
path.quadTo(14.2424f, 3.54063f, 13.8166f, 3.76484f);
path.conicTo(12.9318f, 4.23081f, 12.4658f, 3.34601f, 0.707107f);
path.conicTo(11.9999f, 2.46122f, 12.8847f, 1.99524f, 0.707107f);
path.close();
path.moveTo(15.7467f, 0.702339f);
path.quadTo(16.2402f, 0.514409f, 16.7409f, 0.346672f);
path.conicTo(17.6891f, 0.029011f, 18.0067f, 0.977215f, 0.707107f);
path.conicTo(18.3244f, 1.92542f, 17.3762f, 2.24308f, 0.707107f);
path.quadTo(16.914f, 2.39792f, 16.4585f, 2.57139f);
path.conicTo(15.524f, 2.92729f, 15.1681f, 1.99276f, 0.707107f);
path.conicTo(14.8122f, 1.05824f, 15.7467f, 0.702339f, 0.707107f);
path.close();
path.moveTo(18.7758f, -0.24399f);
path.quadTo(19.2913f, -0.371107f, 19.8116f, -0.477061f);
path.conicTo(20.7915f, -0.676608f, 20.9911f, 0.303281f, 0.707107f);
path.conicTo(21.1906f, 1.28317f, 20.2107f, 1.48272f, 0.707107f);
path.quadTo(19.7304f, 1.58052f, 19.2546f, 1.69785f);
path.conicTo(18.2836f, 1.93725f, 18.0443f, 0.966329f, 0.707107f);
path.conicTo(17.8049f, -0.00459272f, 18.7758f, -0.24399f, 0.707107f);
path.close();
path.moveTo(21.878f, -0.811882f);
path.quadTo(22.396f, -0.874528f, 22.916f, -0.916348f);
path.conicTo(23.9128f, -0.996504f, 23.993f, 0.000278629f, 0.707107f);
path.conicTo(24.0731f, 0.997061f, 23.0764f, 1.07722f, 0.707107f);
path.quadTo(22.5963f, 1.11582f, 22.1182f, 1.17365f);
path.conicTo(21.1254f, 1.29372f, 21.0053f, 0.300958f, 0.707107f);
path.conicTo(20.8853f, -0.691807f, 21.878f, -0.811882f, 0.707107f);
path.close();
path.moveTo(24.9926f, -0.999999f);
path.quadTo(25.5166f, -1.00015f, 26.0401f, -0.979188f);
path.conicTo(27.0393f, -0.939179f, 26.9992f, 0.0600199f, 0.707107f);
path.conicTo(26.9592f, 1.05922f, 25.96f, 1.01921f, 0.707107f);
path.quadTo(25.4768f, 0.999863f, 24.9932f, 1);
path.conicTo(23.9932f, 1.00029f, 23.9929f, 0.000287339f, 0.707107f);
path.conicTo(23.9926f, -0.999713f, 24.9926f, -0.999999f, 0.707107f);
path.close();
path.moveTo(28.1286f, -0.811081f);
path.quadTo(28.6441f, -0.748593f, 29.1567f, -0.665572f);
path.conicTo(30.1439f, -0.505698f, 29.984f, 0.48144f, 0.707107f);
path.conicTo(29.8241f, 1.46858f, 28.837f, 1.3087f, 0.707107f);
path.quadTo(28.3638f, 1.23207f, 27.8879f, 1.17439f);
path.conicTo(26.8952f, 1.05406f, 27.0155f, 0.0613233f, 0.707107f);
path.conicTo(27.1359f, -0.931411f, 28.1286f, -0.811081f, 0.707107f);
path.close();
path.moveTo(31.214f, -0.246499f);
path.quadTo(31.7439f, -0.116076f, 32.2679f, 0.0364622f);
path.conicTo(33.228f, 0.315996f, 32.9485f, 1.27613f, 0.707107f);
path.conicTo(32.6689f, 2.23627f, 31.7088f, 1.95673f, 0.707107f);
path.quadTo(31.2252f, 1.81593f, 30.736f, 1.69554f);
path.conicTo(29.765f, 1.45654f, 30.004f, 0.48552f, 0.707107f);
path.conicTo(30.243f, -0.485499f, 31.214f, -0.246499f, 0.707107f);
path.close();
path.moveTo(34.3038f, 0.721629f);
path.quadTo(34.797f, 0.910612f, 35.282f, 1.11946f);
path.conicTo(36.2005f, 1.51493f, 35.805f, 2.43341f, 0.707107f);
path.conicTo(35.4096f, 3.35189f, 34.4911f, 2.95642f, 0.707107f);
path.quadTo(34.0434f, 2.76365f, 33.5881f, 2.5892f);
path.conicTo(32.6543f, 2.23137f, 33.0122f, 1.29758f, 0.707107f);
path.conicTo(33.37f, 0.363796f, 34.3038f, 0.721629f, 0.707107f);
path.close();
path.moveTo(37.1508f, 2.01396f);
path.quadTo(37.5996f, 2.2512f, 38.0388f, 2.50578f);
path.conicTo(38.904f, 3.00727f, 38.4025f, 3.87244f, 0.707107f);
path.conicTo(37.901f, 4.7376f, 37.0358f, 4.23612f, 0.707107f);
path.quadTo(36.6304f, 4.00111f, 36.2161f, 3.78211f);
path.conicTo(35.332f, 3.31476f, 35.7994f, 2.43069f, 0.707107f);
path.conicTo(36.2667f, 1.54661f, 37.1508f, 2.01396f, 0.707107f);
path.close();
path.moveTo(39.718f, 3.56681f);
path.quadTo(40.1269f, 3.84765f, 40.5249f, 4.14392f);
path.conicTo(41.3271f, 4.74104f, 40.73f, 5.54319f, 0.707107f);
path.conicTo(40.1329f, 6.34535f, 39.3307f, 5.74823f, 0.707107f);
path.quadTo(38.9634f, 5.47478f, 38.5858f, 5.21552f);
path.conicTo(37.7615f, 4.64945f, 38.3275f, 3.82509f, 0.707107f);
path.conicTo(38.8936f, 3.00074f, 39.718f, 3.56681f, 0.707107f);
path.close();
path.moveTo(42.1033f, 5.41741f);
path.quadTo(42.4933f, 5.75802f, 42.8694f, 6.11388f);
path.conicTo(43.5958f, 6.80115f, 42.9085f, 7.52755f, 0.707107f);
path.conicTo(42.2212f, 8.25394f, 41.4948f, 7.56667f, 0.707107f);
path.quadTo(41.1476f, 7.23817f, 40.7876f, 6.92375f);
path.conicTo(40.0345f, 6.26593f, 40.6923f, 5.51275f, 0.707107f);
path.conicTo(41.3501f, 4.75958f, 42.1033f, 5.41741f, 0.707107f);
path.close();
path.moveTo(44.3419f, 7.62498f);
path.quadTo(44.7007f, 8.02444f, 45.0428f, 8.43835f);
path.conicTo(45.6797f, 9.20922f, 44.9089f, 9.84622f, 0.707107f);
path.conicTo(44.138f, 10.4832f, 43.501f, 9.71234f, 0.707107f);
path.quadTo(43.1852f, 9.3302f, 42.854f, 8.96151f);
path.conicTo(42.1858f, 8.21759f, 42.9297f, 7.54932f, 0.707107f);
path.conicTo(43.6736f, 6.88106f, 44.3419f, 7.62498f, 0.707107f);
path.close();
path.moveTo(46.3599f, 10.1759f);
path.quadTo(46.6546f, 10.6005f, 46.9322f, 11.0366f);
path.conicTo(47.4693f, 11.8801f, 46.6257f, 12.4172f, 0.707107f);
path.conicTo(45.7822f, 12.9542f, 45.2451f, 12.1107f, 0.707107f);
path.quadTo(44.9889f, 11.7082f, 44.7168f, 11.3162f);
path.conicTo(44.1467f, 10.4947f, 44.9682f, 9.92452f, 0.707107f);
path.conicTo(45.7897f, 9.35435f, 46.3599f, 10.1759f, 0.707107f);
path.close();
path.moveTo(47.9708f, 12.8204f);
path.quadTo(48.2149f, 13.2808f, 48.4403f, 13.7506f);
path.conicTo(48.873f, 14.6521f, 47.9715f, 15.0848f, 0.707107f);
path.conicTo(47.0699f, 15.5174f, 46.6372f, 14.6159f, 0.707107f);
path.quadTo(46.4291f, 14.1822f, 46.2038f, 13.7573f);
path.conicTo(45.7354f, 12.8738f, 46.6188f, 12.4054f, 0.707107f);
path.conicTo(47.5023f, 11.9369f, 47.9708f, 12.8204f, 0.707107f);
path.close();
path.moveTo(49.2713f, 15.6778f);
path.quadTo(49.4606f, 16.1706f, 49.6297f, 16.6708f);
path.conicTo(49.9501f, 17.6181f, 49.0028f, 17.9384f, 0.707107f);
path.conicTo(48.0555f, 18.2588f, 47.7351f, 17.3115f, 0.707107f);
path.quadTo(47.5791f, 16.8499f, 47.4043f, 16.3949f);
path.conicTo(47.0458f, 15.4614f, 47.9793f, 15.1029f, 0.707107f);
path.conicTo(48.9128f, 14.7443f, 49.2713f, 15.6778f, 0.707107f);
path.close();
path.moveTo(50.2261f, 18.7037f);
path.quadTo(50.3547f, 19.2188f, 50.4621f, 19.7388f);
path.conicTo(50.6645f, 20.7182f, 49.6852f, 20.9205f, 0.707107f);
path.conicTo(48.7059f, 21.1229f, 48.5035f, 20.1436f, 0.707107f);
path.quadTo(48.4043f, 19.6636f, 48.2856f, 19.1881f);
path.conicTo(48.0435f, 18.2178f, 49.0137f, 17.9757f, 0.707107f);
path.conicTo(49.984f, 17.7335f, 50.2261f, 18.7037f, 0.707107f);
path.close();
path.moveTo(50.803f, 21.8055f);
path.quadTo(50.8671f, 22.3234f, 50.9104f, 22.8434f);
path.conicTo(50.9934f, 23.8399f, 49.9968f, 23.9229f, 0.707107f);
path.conicTo(49.0002f, 24.0058f, 48.9173f, 23.0093f, 0.707107f);
path.quadTo(48.8773f, 22.5293f, 48.8182f, 22.0513f);
path.conicTo(48.6953f, 21.0588f, 49.6877f, 20.936f, 0.707107f);
path.conicTo(50.6801f, 20.8131f, 50.803f, 21.8055f, 0.707107f);
path.close();
path.moveTo(50.9999f, 24.9202f);
path.quadTo(51.0015f, 25.4434f, 50.982f, 25.9664f);
path.conicTo(50.9449f, 26.9657f, 49.9456f, 26.9286f, 0.707107f);
path.conicTo(48.9463f, 26.8914f, 48.9834f, 25.8921f, 0.707107f);
path.quadTo(49.0014f, 25.4094f, 48.9999f, 24.9263f);
path.conicTo(48.9968f, 23.9263f, 49.9968f, 23.9232f, 0.707107f);
path.conicTo(50.9968f, 23.9202f, 50.9999f, 24.9202f, 0.707107f);
path.close();
path.moveTo(50.8198f, 28.0562f);
path.quadTo(50.7587f, 28.5721f, 50.677f, 29.0852f);
path.conicTo(50.5199f, 30.0728f, 49.5323f, 29.9157f, 0.707107f);
path.conicTo(48.5448f, 29.7586f, 48.7019f, 28.771f, 0.707107f);
path.quadTo(48.7772f, 28.2974f, 48.8336f, 27.8211f);
path.conicTo(48.9512f, 26.8281f, 49.9442f, 26.9456f, 0.707107f);
path.conicTo(50.9373f, 27.0632f, 50.8198f, 28.0562f, 0.707107f);
path.close();
path.moveTo(50.2647f, 31.1395f);
path.quadTo(50.1358f, 31.6701f, 49.9847f, 32.1949f);
path.conicTo(49.7079f, 33.1558f, 48.747f, 32.8791f, 0.707107f);
path.conicTo(47.786f, 32.6024f, 48.0628f, 31.6414f, 0.707107f);
path.quadTo(48.2022f, 31.1571f, 48.3213f, 30.6672f);
path.conicTo(48.5574f, 29.6955f, 49.5291f, 29.9317f, 0.707107f);
path.conicTo(50.5009f, 30.1678f, 50.2647f, 31.1395f, 0.707107f);
path.close();
path.moveTo(49.3049f, 34.2343f);
path.quadTo(49.1171f, 34.7285f, 48.9095f, 35.2145f);
path.conicTo(48.5166f, 36.1341f, 47.597f, 35.7412f, 0.707107f);
path.conicTo(46.6774f, 35.3483f, 47.0703f, 34.4288f, 0.707107f);
path.quadTo(47.262f, 33.9801f, 47.4353f, 33.524f);
path.conicTo(47.7904f, 32.5892f, 48.7252f, 32.9444f, 0.707107f);
path.conicTo(49.66f, 33.2995f, 49.3049f, 34.2343f, 0.707107f);
path.close();
path.moveTo(48.0194f, 37.0875f);
path.quadTo(47.7831f, 37.5374f, 47.5295f, 37.9777f);
path.conicTo(47.0304f, 38.8443f, 46.1638f, 38.3451f, 0.707107f);
path.conicTo(45.2973f, 37.846f, 45.7965f, 36.9795f, 0.707107f);
path.quadTo(46.0306f, 36.5729f, 46.2487f, 36.1577f);
path.conicTo(46.7136f, 35.2723f, 47.5989f, 35.7372f, 0.707107f);
path.conicTo(48.4843f, 36.2021f, 48.0194f, 37.0875f, 0.707107f);
path.close();
path.moveTo(46.4721f, 39.6612f);
path.quadTo(46.1926f, 40.0705f, 45.8977f, 40.4688f);
path.conicTo(45.3028f, 41.2726f, 44.499f, 40.6776f, 0.707107f);
path.conicTo(43.6953f, 40.0827f, 44.2902f, 39.2789f, 0.707107f);
path.quadTo(44.5624f, 38.9112f, 44.8204f, 38.5334f);
path.conicTo(45.3843f, 37.7075f, 46.2101f, 38.2714f, 0.707107f);
path.conicTo(47.036f, 38.8353f, 46.4721f, 39.6612f, 0.707107f);
path.close();
path.moveTo(44.6298f, 42.0491f);
path.quadTo(44.2906f, 42.4396f, 43.9361f, 42.8164f);
path.conicTo(43.2509f, 43.5447f, 42.5226f, 42.8595f, 0.707107f);
path.conicTo(41.7942f, 42.1742f, 42.4795f, 41.4459f, 0.707107f);
path.quadTo(42.8067f, 41.0981f, 43.1198f, 40.7376f);
path.conicTo(43.7756f, 39.9826f, 44.5306f, 40.6383f, 0.707107f);
path.conicTo(45.2856f, 41.2941f, 44.6298f, 42.0491f, 0.707107f);
path.close();
path.moveTo(42.4305f, 44.2919f);
path.quadTo(42.0324f, 44.6516f, 41.6198f, 44.9946f);
path.conicTo(40.8507f, 45.6338f, 40.2115f, 44.8648f, 0.707107f);
path.conicTo(39.5723f, 44.0958f, 40.3413f, 43.4566f, 0.707107f);
path.quadTo(40.7222f, 43.1399f, 41.0897f, 42.8079f);
path.conicTo(41.8317f, 42.1375f, 42.5021f, 42.8795f, 0.707107f);
path.conicTo(43.1725f, 43.6215f, 42.4305f, 44.2919f, 0.707107f);
path.close();
path.moveTo(39.8873f, 46.3159f);
path.quadTo(39.4613f, 46.6134f, 39.0238f, 46.8936f);
path.conicTo(38.1818f, 47.433f, 37.6424f, 46.5909f, 0.707107f);
path.conicTo(37.103f, 45.7489f, 37.9451f, 45.2095f, 0.707107f);
path.quadTo(38.3489f, 44.9508f, 38.7421f, 44.6763f);
path.conicTo(39.5619f, 44.1037f, 40.1345f, 44.9235f, 0.707107f);
path.conicTo(40.7071f, 45.7434f, 39.8873f, 46.3159f, 0.707107f);
path.close();
path.moveTo(37.2437f, 47.9367f);
path.quadTo(36.7842f, 48.182f, 36.3153f, 48.4086f);
path.conicTo(35.415f, 48.8439f, 34.9797f, 47.9435f, 0.707107f);
path.conicTo(34.5445f, 47.0432f, 35.4449f, 46.608f, 0.707107f);
path.quadTo(35.8778f, 46.3987f, 36.3019f, 46.1723f);
path.conicTo(37.1841f, 45.7014f, 37.655f, 46.5836f, 0.707107f);
path.conicTo(38.1259f, 47.4658f, 37.2437f, 47.9367f, 0.707107f);
path.close();
path.moveTo(34.3909f, 49.2448f);
path.quadTo(33.8988f, 49.4354f, 33.3992f, 49.606f);
path.conicTo(32.4528f, 49.929f, 32.1298f, 48.9826f, 0.707107f);
path.conicTo(31.8068f, 48.0362f, 32.7532f, 47.7132f, 0.707107f);
path.quadTo(33.2142f, 47.5558f, 33.6685f, 47.3798f);
path.conicTo(34.601f, 47.0186f, 34.9622f, 47.9511f, 0.707107f);
path.conicTo(35.3234f, 48.8836f, 34.3909f, 49.2448f, 0.707107f);
path.close();
path.moveTo(31.3682f, 50.208f);
path.quadTo(30.8535f, 50.3381f, 30.3338f, 50.447f);
path.conicTo(29.3551f, 50.6521f, 29.15f, 49.6734f, 0.707107f);
path.conicTo(28.9448f, 48.6947f, 29.9236f, 48.4895f, 0.707107f);
path.quadTo(30.4033f, 48.389f, 30.8784f, 48.269f);
path.conicTo(31.8479f, 48.024f, 32.0929f, 48.9936f, 0.707107f);
path.conicTo(32.3378f, 49.9631f, 31.3682f, 50.208f, 0.707107f);
path.close();
path.moveTo(28.2669f, 50.7939f);
path.quadTo(27.7491f, 50.8595f, 27.2292f, 50.9043f);
path.conicTo(26.2329f, 50.99f, 26.1472f, 49.9937f, 0.707107f);
path.conicTo(26.0615f, 48.9973f, 27.0578f, 48.9116f, 0.707107f);
path.quadTo(27.5378f, 48.8703f, 28.0156f, 48.8098f);
path.conicTo(29.0077f, 48.6841f, 29.1334f, 49.6762f, 0.707107f);
path.conicTo(29.259f, 50.6683f, 28.2669f, 50.7939f, 0.707107f);
path.close();
path.moveTo(25.1523f, 50.9996f);
path.quadTo(24.6297f, 51.0026f, 24.1072f, 50.9847f);
path.conicTo(23.1078f, 50.9503f, 23.1422f, 49.9509f, 0.707107f);
path.conicTo(23.1765f, 48.9515f, 24.1759f, 48.9858f, 0.707107f);
path.quadTo(24.658f, 49.0024f, 25.1406f, 48.9996f);
path.conicTo(26.1406f, 48.9937f, 26.1464f, 49.9937f, 0.707107f);
path.conicTo(26.1523f, 50.9937f, 25.1523f, 50.9996f, 0.707107f);
path.close();
path.moveTo(22.0162f, 50.8282f);
path.quadTo(21.4999f, 50.7686f, 20.9863f, 50.6883f);
path.conicTo(19.9983f, 50.5339f, 20.1527f, 49.5459f, 0.707107f);
path.conicTo(20.307f, 48.5579f, 21.295f, 48.7123f, 0.707107f);
path.quadTo(21.7691f, 48.7864f, 22.2457f, 48.8414f);
path.conicTo(23.2391f, 48.9562f, 23.1243f, 49.9496f, 0.707107f);
path.conicTo(23.0096f, 50.943f, 22.0162f, 50.8282f, 0.707107f);
path.close();
path.moveTo(18.9351f, 50.2827f);
path.quadTo(18.4037f, 50.1553f, 17.8782f, 50.0056f);
path.conicTo(16.9164f, 49.7317f, 17.1904f, 48.7699f, 0.707107f);
path.conicTo(17.4643f, 47.8082f, 18.426f, 48.0821f, 0.707107f);
path.quadTo(18.9112f, 48.2203f, 19.4016f, 48.3379f);
path.conicTo(20.374f, 48.5712f, 20.1408f, 49.5436f, 0.707107f);
path.conicTo(19.9075f, 50.516f, 18.9351f, 50.2827f, 0.707107f);
path.close();
path.moveTo(15.8352f, 49.3312f);
path.quadTo(15.3403f, 49.1448f, 14.8531f, 48.9383f);
path.conicTo(13.9324f, 48.548f, 14.3227f, 47.6273f, 0.707107f);
path.conicTo(14.713f, 46.7066f, 15.6337f, 47.0969f, 0.707107f);
path.quadTo(16.0832f, 47.2874f, 16.5402f, 47.4596f);
path.conicTo(17.476f, 47.812f, 17.1235f, 48.7479f, 0.707107f);
path.conicTo(16.771f, 49.6837f, 15.8352f, 49.3312f, 0.707107f);
path.close();
path.moveTo(12.9759f, 48.0526f);
path.quadTo(12.5249f, 47.8173f, 12.0835f, 47.5647f);
path.conicTo(11.2156f, 47.0679f, 11.7124f, 46.2f, 0.707107f);
path.conicTo(12.2092f, 45.3321f, 13.0771f, 45.8289f, 0.707107f);
path.quadTo(13.4846f, 46.0622f, 13.9009f, 46.2793f);
path.conicTo(14.7875f, 46.7418f, 14.325f, 47.6284f, 0.707107f);
path.conicTo(13.8626f, 48.5151f, 12.9759f, 48.0526f, 0.707107f);
path.close();
path.moveTo(10.3957f, 46.5108f);
path.quadTo(9.9861f, 46.2327f, 9.58733f, 45.9392f);
path.conicTo(8.78198f, 45.3464f, 9.37478f, 44.541f, 0.707107f);
path.conicTo(9.96757f, 43.7357f, 10.7729f, 44.3285f, 0.707107f);
path.quadTo(11.141f, 44.5994f, 11.5191f, 44.8561f);
path.conicTo(12.3464f, 45.4178f, 11.7847f, 46.2451f, 0.707107f);
path.conicTo(11.223f, 47.0725f, 10.3957f, 46.5108f, 0.707107f);
path.close();
path.moveTo(8.00525f, 44.6769f);
path.quadTo(7.6141f, 44.339f, 7.23672f, 43.9859f);
path.conicTo(6.50649f, 43.3027f, 7.18969f, 42.5725f, 0.707107f);
path.conicTo(7.87289f, 41.8423f, 8.60312f, 42.5255f, 0.707107f);
path.quadTo(8.95149f, 42.8514f, 9.31254f, 43.1632f);
path.conicTo(10.0693f, 43.8169f, 9.4157f, 44.5737f, 0.707107f);
path.conicTo(8.76206f, 45.3305f, 8.00525f, 44.6769f, 0.707107f);
path.close();
path.moveTo(5.75818f, 42.4858f);
path.quadTo(5.39763f, 42.089f, 5.05371f, 41.6777f);
path.conicTo(4.41226f, 40.9105f, 5.17942f, 40.2691f, 0.707107f);
path.conicTo(5.94658f, 39.6276f, 6.58804f, 40.3948f, 0.707107f);
path.quadTo(6.90548f, 40.7744f, 7.23832f, 41.1407f);
path.conicTo(7.91085f, 41.8808f, 7.17078f, 42.5533f, 0.707107f);
path.conicTo(6.43071f, 43.2258f, 5.75818f, 42.4858f, 0.707107f);
path.close();
path.moveTo(3.72821f, 39.9503f);
path.quadTo(3.42794f, 39.523f, 3.1451f, 39.0842f);
path.conicTo(2.6034f, 38.2436f, 3.44397f, 37.7019f, 0.707107f);
path.conicTo(4.28454f, 37.1602f, 4.82624f, 38.0008f, 0.707107f);
path.quadTo(5.08734f, 38.4059f, 5.3645f, 38.8003f);
path.conicTo(5.93951f, 39.6184f, 5.12137f, 40.1934f, 0.707107f);
path.conicTo(4.30322f, 40.7684f, 3.72821f, 39.9503f, 0.707107f);
path.close();
path.moveTo(2.09762f, 37.3078f);
path.quadTo(1.85114f, 36.8491f, 1.62324f, 36.381f);
path.conicTo(1.18551f, 35.4819f, 2.08461f, 35.0442f, 0.707107f);
path.conicTo(2.98372f, 34.6064f, 3.42145f, 35.5055f, 0.707107f);
path.quadTo(3.63184f, 35.9377f, 3.85934f, 36.361f);
path.conicTo(4.33272f, 37.2419f, 3.45185f, 37.7153f, 0.707107f);
path.conicTo(2.57099f, 38.1886f, 2.09762f, 37.3078f, 0.707107f);
path.close();
path.moveTo(0.781912f, 34.4596f);
path.quadTo(0.589924f, 33.9681f, 0.418029f, 33.4692f);
path.conicTo(0.0922952f, 32.5237f, 1.03776f, 32.198f, 0.707107f);
path.conicTo(1.98322f, 31.8722f, 2.30895f, 32.8177f, 0.707107f);
path.quadTo(2.46761f, 33.2782f, 2.64484f, 33.7319f);
path.conicTo(3.00867f, 34.6634f, 2.07721f, 35.0272f, 0.707107f);
path.conicTo(1.14575f, 35.3911f, 0.781912f, 34.4596f, 0.707107f);
path.close();
path.moveTo(-0.189761f, 31.4402f);
path.quadTo(-0.321263f, 30.9258f, -0.431662f, 30.4065f);
path.conicTo(-0.639608f, 29.4284f, 0.338532f, 29.2205f, 0.707107f);
path.conicTo(1.31667f, 29.0125f, 1.52462f, 29.9906f, 0.707107f);
path.quadTo(1.62653f, 30.47f, 1.74791f, 30.9448f);
path.conicTo(1.99561f, 31.9136f, 1.02677f, 32.1613f, 0.707107f);
path.conicTo(0.0579369f, 32.409f, -0.189761f, 31.4402f, 0.707107f);
path.close();
path.moveTo(-0.784658f, 28.3394f);
path.quadTo(-0.851693f, 27.8218f, -0.897902f, 27.3019f);
path.conicTo(-0.986437f, 26.3058f, 0.00963629f, 26.2173f, 0.707107f);
path.conicTo(1.00571f, 26.1288f, 1.09424f, 27.1248f, 0.707107f);
path.quadTo(1.1369f, 27.6047f, 1.19878f, 28.0825f);
path.conicTo(1.32721f, 29.0742f, 0.335496f, 29.2027f, 0.707107f);
path.conicTo(-0.656222f, 29.3311f, -0.784658f, 28.3394f, 0.707107f);
path.close();
path.moveTo(-0.999031f, 25.2248f);
path.quadTo(-1.00354f, 24.7027f, -0.987098f, 24.1809f);
path.conicTo(-0.955596f, 23.1814f, 0.0439078f, 23.2129f, 0.707107f);
path.conicTo(1.04341f, 23.2444f, 1.01191f, 24.2439f, 0.707107f);
path.quadTo(0.996728f, 24.7256f, 1.00089f, 25.2075f);
path.conicTo(1.00954f, 26.2075f, 0.00957754f, 26.2161f, 0.707107f);
path.conicTo(-0.990385f, 26.2248f, -0.999031f, 25.2248f, 0.707107f);
path.close();
path.moveTo(-0.836492f, 22.0887f);
path.quadTo(-0.778263f, 21.5719f, -0.699419f, 21.0579f);
path.conicTo(-0.5478f, 20.0695f, 0.440639f, 20.2211f, 0.707107f);
path.conicTo(1.42908f, 20.3727f, 1.27746f, 21.3612f, 0.707107f);
path.quadTo(1.20468f, 21.8356f, 1.15093f, 22.3126f);
path.conicTo(1.03896f, 23.3063f, 0.0452449f, 23.1944f, 0.707107f);
path.conicTo(-0.948466f, 23.0824f, -0.836492f, 22.0887f, 0.707107f);
path.close();
path.moveTo(-0.300548f, 19.0098f);
path.quadTo(-0.174573f, 18.4777f, -0.0263361f, 17.9514f);
path.conicTo(0.244762f, 16.9889f, 1.20731f, 17.26f, 0.707107f);
path.conicTo(2.16987f, 17.5311f, 1.89877f, 18.4936f, 0.707107f);
path.quadTo(1.76193f, 18.9794f, 1.64565f, 19.4706f);
path.conicTo(1.41526f, 20.4437f, 0.442159f, 20.2133f, 0.707107f);
path.conicTo(-0.530939f, 19.9829f, -0.300548f, 19.0098f, 0.707107f);
path.close();
path.moveTo(0.642658f, 15.9049f);
path.quadTo(0.827861f, 15.409f, 1.0331f, 14.9209f);
path.conicTo(1.42076f, 13.9991f, 2.34256f, 14.3868f, 0.707107f);
path.conicTo(3.26437f, 14.7744f, 2.87671f, 15.6962f, 0.707107f);
path.quadTo(2.68726f, 16.1467f, 2.5163f, 16.6046f);
path.conicTo(2.16648f, 17.5414f, 1.22967f, 17.1916f, 0.707107f);
path.conicTo(0.292846f, 16.8418f, 0.642658f, 15.9049f, 0.707107f);
path.close();
path.moveTo(1.91434f, 13.0395f);
path.quadTo(2.14856f, 12.5875f, 2.40031f, 12.1449f);
path.conicTo(2.89473f, 11.2757f, 3.76395f, 11.7701f, 0.707107f);
path.conicTo(4.63317f, 12.2645f, 4.13875f, 13.1337f, 0.707107f);
path.quadTo(3.90637f, 13.5423f, 3.69016f, 13.9596f);
path.conicTo(3.23014f, 14.8475f, 2.34223f, 14.3875f, 0.707107f);
path.conicTo(1.45432f, 13.9275f, 1.91434f, 13.0395f, 0.707107f);
path.close();
path.moveTo(3.45073f, 10.4525f);
path.quadTo(3.72744f, 10.0426f, 4.01954f, 9.64356f);
path.conicTo(4.61017f, 8.83661f, 5.41711f, 9.42725f, 0.707107f);
path.conicTo(6.22405f, 10.0179f, 5.63342f, 10.8248f, 0.707107f);
path.quadTo(5.36379f, 11.1932f, 5.10836f, 11.5716f);
path.conicTo(4.54884f, 12.4004f, 3.72003f, 11.8409f, 0.707107f);
path.conicTo(2.89121f, 11.2813f, 3.45073f, 10.4525f, 0.707107f);
path.close();
path.moveTo(5.2763f, 8.05964f);
path.quadTo(5.61273f, 7.66793f, 5.96445f, 7.2899f);
path.conicTo(6.6456f, 6.55776f, 7.37774f, 7.23892f, 0.707107f);
path.conicTo(8.10988f, 7.92008f, 7.42872f, 8.65221f, 0.707107f);
path.quadTo(7.10407f, 9.00116f, 6.79351f, 9.36274f);
path.conicTo(6.14196f, 10.1213f, 5.38336f, 9.46979f, 0.707107f);
path.conicTo(4.62475f, 8.81824f, 5.2763f, 8.05964f, 0.707107f);
path.close();
path.moveTo(7.45913f, 5.80839f);
path.quadTo(7.85457f, 5.44696f, 8.26455f, 5.10214f);
path.conicTo(9.02985f, 4.45847f, 9.67352f, 5.22377f, 0.707107f);
path.conicTo(10.3172f, 5.98907f, 9.5519f, 6.63274f, 0.707107f);
path.quadTo(9.17345f, 6.95105f, 8.80843f, 7.28467f);
path.conicTo(8.07029f, 7.95931f, 7.39564f, 7.22117f, 0.707107f);
path.conicTo(6.72099f, 6.48303f, 7.45913f, 5.80839f, 0.707107f);
path.close();
path.moveTo(9.98688f, 3.77251f);
path.quadTo(10.4153f, 3.46948f, 10.8557f, 3.18397f);
path.conicTo(11.6948f, 2.63996f, 12.2388f, 3.47904f, 0.707107f);
path.conicTo(12.7828f, 4.31812f, 11.9437f, 4.86213f, 0.707107f);
path.quadTo(11.5373f, 5.12566f, 11.1417f, 5.40539f);
path.conicTo(10.3253f, 5.98282f, 9.74787f, 5.16638f, 0.707107f);
path.conicTo(9.17044f, 4.34994f, 9.98688f, 3.77251f, 0.707107f);
path.close();
path.moveTo(12.6283f, 2.13208f);
path.quadTo(13.0861f, 1.88442f, 13.5534f, 1.65529f);
path.conicTo(14.4513f, 1.21504f, 14.8915f, 2.11291f, 0.707107f);
path.conicTo(15.3318f, 3.01078f, 14.4339f, 3.45104f, 0.707107f);
path.quadTo(14.0025f, 3.66255f, 13.58f, 3.89115f);
path.conicTo(12.7005f, 4.36698f, 12.2246f, 3.48744f, 0.707107f);
path.conicTo(11.7488f, 2.60791f, 12.6283f, 2.13208f, 0.707107f);
path.close();
path.moveTo(15.4718f, 0.808815f);
path.quadTo(15.9627f, 0.615476f, 16.461f, 0.442208f);
path.conicTo(17.4055f, 0.113784f, 17.7339f, 1.05831f, 0.707107f);
path.conicTo(18.0624f, 2.00284f, 17.1178f, 2.33127f, 0.707107f);
path.quadTo(16.6578f, 2.49121f, 16.2047f, 2.66968f);
path.conicTo(15.2743f, 3.03614f, 14.9078f, 2.10571f, 0.707107f);
path.conicTo(14.5414f, 1.17528f, 15.4718f, 0.808815f, 0.707107f);
path.close();
path.moveTo(18.4879f, -0.171272f);
path.quadTo(19.0019f, -0.304236f, 19.5208f, -0.416111f);
path.conicTo(20.4984f, -0.62685f, 20.7091f, 0.350692f, 0.707107f);
path.conicTo(20.9198f, 1.32823f, 19.9423f, 1.53897f, 0.707107f);
path.quadTo(19.4633f, 1.64224f, 18.9889f, 1.76498f);
path.conicTo(18.0207f, 2.01544f, 17.7703f, 1.04732f, 0.707107f);
path.conicTo(17.5198f, 0.0791926f, 18.4879f, -0.171272f, 0.707107f);
path.close();
path.moveTo(21.5882f, -0.77517f);
path.quadTo(22.1056f, -0.843665f, 22.6254f, -0.891339f);
path.conicTo(23.6212f, -0.982672f, 23.7126f, 0.0131486f, 0.707107f);
path.conicTo(23.8039f, 1.00897f, 22.8081f, 1.1003f, 0.707107f);
path.quadTo(22.3283f, 1.14431f, 21.8506f, 1.20754f);
path.conicTo(20.8592f, 1.33876f, 20.728f, 0.347405f, 0.707107f);
path.conicTo(20.5968f, -0.643948f, 21.5882f, -0.77517f, 0.707107f);
path.close();
path.moveTo(24.7026f, -0.998301f);
path.quadTo(25.2241f, -1.00426f, 25.7453f, -0.989316f);
path.conicTo(26.7449f, -0.960651f, 26.7162f, 0.0389383f, 0.707107f);
path.conicTo(26.6876f, 1.03853f, 25.688f, 1.00986f, 0.707107f);
path.quadTo(25.2068f, 0.996064f, 24.7255f, 1.00157f);
path.conicTo(23.7256f, 1.013f, 23.7141f, 0.0130688f, 0.707107f);
path.conicTo(23.7027f, -0.986866f, 24.7026f, -0.998301f, 0.707107f);
path.close();
path.moveTo(27.8388f, -0.844563f);
path.quadTo(28.3559f, -0.787759f, 28.8704f, -0.710314f);
path.conicTo(29.8592f, -0.561454f, 29.7104f, 0.427404f, 0.707107f);
path.conicTo(29.5615f, 1.41626f, 28.5726f, 1.2674f, 0.707107f);
path.quadTo(28.0978f, 1.19591f, 27.6204f, 1.14348f);
path.conicTo(26.6264f, 1.0343f, 26.7356f, 0.0402742f, 0.707107f);
path.conicTo(26.8447f, -0.953747f, 27.8388f, -0.844563f, 0.707107f);
path.close();
path.moveTo(30.9153f, -0.318153f);
path.quadTo(31.4481f, -0.193671f, 31.9752f, -0.046875f);
path.conicTo(32.9386f, 0.221405f, 32.6703f, 1.18475f, 0.707107f);
path.conicTo(32.402f, 2.14809f, 31.4387f, 1.87981f, 0.707107f);
path.quadTo(30.9521f, 1.74431f, 30.4603f, 1.6294f);
path.conicTo(29.4865f, 1.40189f, 29.714f, 0.428111f, 0.707107f);
path.conicTo(29.9416f, -0.545664f, 30.9153f, -0.318153f, 0.707107f);
path.close();
path.moveTo(34.0252f, 0.616677f);
path.quadTo(34.5221f, 0.800609f, 35.0111f, 1.00465f);
path.conicTo(35.934f, 1.3897f, 35.549f, 2.31259f, 0.707107f);
path.conicTo(35.1639f, 3.23549f, 34.241f, 2.85044f, 0.707107f);
path.quadTo(33.7896f, 2.66211f, 33.3309f, 2.49232f);
path.conicTo(32.3931f, 2.1452f, 32.7402f, 1.20738f, 0.707107f);
path.conicTo(33.0873f, 0.269559f, 34.0252f, 0.616677f, 0.707107f);
path.close();
path.moveTo(36.8967f, 1.88141f);
path.quadTo(37.3499f, 2.11462f, 37.7936f, 2.3654f);
path.conicTo(38.6641f, 2.85746f, 38.1721f, 3.72802f, 0.707107f);
path.conicTo(37.68f, 4.59858f, 36.8094f, 4.10652f, 0.707107f);
path.quadTo(36.3999f, 3.87504f, 35.9815f, 3.65976f);
path.conicTo(35.0924f, 3.2022f, 35.5499f, 2.31302f, 0.707107f);
path.conicTo(36.0075f, 1.42384f, 36.8967f, 1.88141f, 0.707107f);
path.close();
path.moveTo(39.4914f, 3.413f);
path.lineTo(39.5381f, 3.44439f);
path.quadTo(39.9244f, 3.70494f, 40.3002f, 3.97845f);
path.conicTo(41.1087f, 4.56692f, 40.5202f, 5.37544f, 0.707107f);
path.conicTo(39.9317f, 6.18396f, 39.1232f, 5.59549f, 0.707107f);
path.quadTo(38.7763f, 5.34298f, 38.4215f, 5.10371f);
path.lineTo(38.3749f, 5.07232f);
path.conicTo(37.5452f, 4.51406f, 38.1035f, 3.68439f, 0.707107f);
path.conicTo(38.6618f, 2.85473f, 39.4914f, 3.413f, 0.707107f);
path.close();
path.moveTo(41.8859f, 5.22965f);
path.quadTo(42.2782f, 5.56471f, 42.6568f, 5.91499f);
path.conicTo(43.3908f, 6.5941f, 42.7117f, 7.32814f, 0.707107f);
path.conicTo(42.0326f, 8.06218f, 41.2986f, 7.38308f, 0.707107f);
path.quadTo(40.949f, 7.05968f, 40.587f, 6.75043f);
path.conicTo(39.8266f, 6.10097f, 40.476f, 5.34058f, 0.707107f);
path.conicTo(41.1255f, 4.58018f, 41.8859f, 5.22965f, 0.707107f);
path.close();
path.moveTo(44.1413f, 7.40421f);
path.quadTo(44.5035f, 7.79829f, 44.8493f, 8.20695f);
path.conicTo(45.4952f, 8.97038f, 44.7317f, 9.61627f, 0.707107f);
path.conicTo(43.9683f, 10.2622f, 43.3224f, 9.49874f, 0.707107f);
path.quadTo(43.0033f, 9.1215f, 42.6689f, 8.75773f);
path.conicTo(41.9921f, 8.02152f, 42.7283f, 7.34476f, 0.707107f);
path.conicTo(43.4645f, 6.668f, 44.1413f, 7.40421f, 0.707107f);
path.close();
path.moveTo(46.183f, 9.9242f);
path.quadTo(46.4888f, 10.3539f, 46.777f, 10.7957f);
path.conicTo(47.3233f, 11.6332f, 46.4857f, 12.1796f, 0.707107f);
path.conicTo(45.6482f, 12.7259f, 45.1018f, 11.8883f, 0.707107f);
path.quadTo(44.8358f, 11.4805f, 44.5535f, 11.0839f);
path.conicTo(43.9737f, 10.2691f, 44.7884f, 9.6893f, 0.707107f);
path.conicTo(45.6032f, 9.10947f, 46.183f, 9.9242f, 0.707107f);
path.close();
path.moveTo(47.8333f, 12.5645f);
path.quadTo(48.0821f, 13.0214f, 48.3125f, 13.4879f);
path.conicTo(48.7552f, 14.3845f, 47.8586f, 14.8273f, 0.707107f);
path.conicTo(46.962f, 15.2701f, 46.5192f, 14.3734f, 0.707107f);
path.quadTo(46.3065f, 13.9428f, 46.0769f, 13.5211f);
path.conicTo(45.5986f, 12.6429f, 46.4768f, 12.1646f, 0.707107f);
path.conicTo(47.355f, 11.6863f, 47.8333f, 12.5645f, 0.707107f);
path.close();
path.moveTo(49.1641f, 15.4033f);
path.quadTo(49.3588f, 15.8935f, 49.5334f, 16.3912f);
path.conicTo(49.8645f, 17.3348f, 48.9209f, 17.6659f, 0.707107f);
path.conicTo(47.9773f, 17.997f, 47.6462f, 17.0534f, 0.707107f);
path.quadTo(47.485f, 16.5939f, 47.3053f, 16.1415f);
path.conicTo(46.9362f, 15.2121f, 47.8656f, 14.843f, 0.707107f);
path.conicTo(48.795f, 14.4739f, 49.1641f, 15.4033f, 0.707107f);
path.close();
path.moveTo(50.1526f, 18.4161f);
path.quadTo(50.287f, 18.9296f, 50.4003f, 19.4482f);
path.conicTo(50.6139f, 20.4252f, 49.6369f, 20.6387f, 0.707107f);
path.conicTo(48.66f, 20.8522f, 48.4465f, 19.8753f, 0.707107f);
path.quadTo(48.3419f, 19.3966f, 48.2178f, 18.9225f);
path.conicTo(47.9645f, 17.9551f, 48.9319f, 17.7019f, 0.707107f);
path.conicTo(49.8993f, 17.4487f, 50.1526f, 18.4161f, 0.707107f);
path.close();
path.moveTo(50.7655f, 21.5157f);
path.quadTo(50.8354f, 22.033f, 50.8846f, 22.5528f);
path.conicTo(50.9787f, 23.5483f, 49.9831f, 23.6425f, 0.707107f);
path.conicTo(48.9876f, 23.7366f, 48.8935f, 22.741f, 0.707107f);
path.quadTo(48.8481f, 22.2613f, 48.7835f, 21.7837f);
path.conicTo(48.6495f, 20.7928f, 49.6405f, 20.6587f, 0.707107f);
path.conicTo(50.6315f, 20.5247f, 50.7655f, 21.5157f, 0.707107f);
path.close();
path.moveTo(50.9974f, 24.6301f);
path.quadTo(51.0048f, 25.1509f, 50.9913f, 25.6715f);
path.conicTo(50.9655f, 26.6712f, 49.9658f, 26.6454f, 0.707107f);
path.conicTo(48.9662f, 26.6196f, 48.992f, 25.6199f, 0.707107f);
path.quadTo(49.0044f, 25.1393f, 48.9976f, 24.6585f);
path.conicTo(48.9834f, 23.6586f, 49.9833f, 23.6444f, 0.707107f);
path.conicTo(50.9832f, 23.6302f, 50.9974f, 24.6301f, 0.707107f);
path.close();
path.moveTo(50.8524f, 27.7662f);
path.quadTo(50.7971f, 28.2837f, 50.721f, 28.7986f);
path.conicTo(50.5749f, 29.7879f, 49.5856f, 29.6418f, 0.707107f);
path.conicTo(48.5963f, 29.4957f, 48.7425f, 28.5064f, 0.707107f);
path.quadTo(48.8127f, 28.0311f, 48.8638f, 27.5534f);
path.conicTo(48.9702f, 26.5591f, 49.9645f, 26.6655f, 0.707107f);
path.conicTo(50.9588f, 26.7718f, 50.8524f, 27.7662f, 0.707107f);
path.close();
path.moveTo(50.3355f, 30.8404f);
path.quadTo(50.2125f, 31.3739f, 50.0672f, 31.9018f);
path.conicTo(49.8018f, 32.8659f, 48.8376f, 32.6005f, 0.707107f);
path.conicTo(47.8735f, 32.335f, 48.139f, 31.3709f, 0.707107f);
path.quadTo(48.2731f, 30.8836f, 48.3867f, 30.3912f);
path.conicTo(48.6113f, 29.4167f, 49.5857f, 29.6413f, 0.707107f);
path.conicTo(50.5602f, 29.866f, 50.3355f, 30.8404f, 0.707107f);
path.close();
path.moveTo(49.4091f, 33.9552f);
path.quadTo(49.2264f, 34.4531f, 49.0236f, 34.9431f);
path.conicTo(48.6412f, 35.8671f, 47.7172f, 35.4846f, 0.707107f);
path.conicTo(46.7932f, 35.1022f, 47.1757f, 34.1782f, 0.707107f);
path.quadTo(47.3629f, 33.7259f, 47.5315f, 33.2663f);
path.conicTo(47.8759f, 32.3275f, 48.8147f, 32.672f, 0.707107f);
path.conicTo(49.7535f, 33.0164f, 49.4091f, 33.9552f, 0.707107f);
path.close();
path.moveTo(48.1514f, 36.8328f);
path.quadTo(47.9191f, 37.2871f, 47.6694f, 37.7318f);
path.conicTo(47.1797f, 38.6038f, 46.3078f, 38.1141f, 0.707107f);
path.conicTo(45.4359f, 37.6244f, 45.9256f, 36.7525f, 0.707107f);
path.quadTo(46.1562f, 36.3418f, 46.3705f, 35.9226f);
path.conicTo(46.8256f, 35.0321f, 47.716f, 35.4872f, 0.707107f);
path.conicTo(48.6065f, 35.9423f, 48.1514f, 36.8328f, 0.707107f);
path.close();
path.moveTo(46.6245f, 39.4354f);
path.lineTo(46.5563f, 39.537f);
path.quadTo(46.3146f, 39.8955f, 46.0624f, 40.2438f);
path.conicTo(45.4761f, 41.0539f, 44.666f, 40.4676f, 0.707107f);
path.conicTo(43.8559f, 39.8813f, 44.4422f, 39.0712f, 0.707107f);
path.quadTo(44.6749f, 38.7498f, 44.8955f, 38.4226f);
path.lineTo(44.9637f, 38.3211f);
path.conicTo(45.5209f, 37.4907f, 46.3513f, 38.0479f, 0.707107f);
path.conicTo(47.1817f, 38.605f, 46.6245f, 39.4354f, 0.707107f);
path.close();
path.moveTo(44.8168f, 41.8314f);
path.quadTo(44.4832f, 42.2241f, 44.1342f, 42.6034f);
path.conicTo(43.4572f, 43.3394f, 42.7212f, 42.6623f, 0.707107f);
path.conicTo(41.9853f, 41.9853f, 42.6623f, 41.2494f, 0.707107f);
path.quadTo(42.9845f, 40.8992f, 43.2924f, 40.5366f);
path.conicTo(43.9398f, 39.7745f, 44.702f, 40.4218f, 0.707107f);
path.conicTo(45.4642f, 41.0692f, 44.8168f, 41.8314f, 0.707107f);
path.close();
path.moveTo(42.6505f, 44.0908f);
path.quadTo(42.2577f, 44.454f, 41.8504f, 44.8006f);
path.conicTo(41.0888f, 45.4487f, 40.4408f, 44.6871f, 0.707107f);
path.conicTo(39.7927f, 43.9256f, 40.5542f, 43.2775f, 0.707107f);
path.quadTo(40.9302f, 42.9575f, 41.2928f, 42.6223f);
path.conicTo(42.027f, 41.9434f, 42.7059f, 42.6777f, 0.707107f);
path.conicTo(43.3848f, 43.412f, 42.6505f, 44.0908f, 0.707107f);
path.close();
path.moveTo(40.1383f, 46.1384f);
path.quadTo(39.7073f, 46.4471f, 39.2641f, 46.7378f);
path.conicTo(38.4281f, 47.2865f, 37.8795f, 46.4504f, 0.707107f);
path.conicTo(37.3308f, 45.6143f, 38.1669f, 45.0657f, 0.707107f);
path.quadTo(38.576f, 44.7972f, 38.9738f, 44.5124f);
path.conicTo(39.7868f, 43.9301f, 40.369f, 44.7432f, 0.707107f);
path.conicTo(40.9513f, 45.5562f, 40.1383f, 46.1384f, 0.707107f);
path.close();
path.moveTo(37.4991f, 47.7985f);
path.quadTo(37.0431f, 48.0485f, 36.5775f, 48.2801f);
path.conicTo(35.6821f, 48.7254f, 35.2368f, 47.83f, 0.707107f);
path.conicTo(34.7915f, 46.9346f, 35.6869f, 46.4893f, 0.707107f);
path.quadTo(36.1167f, 46.2755f, 36.5376f, 46.0448f);
path.conicTo(37.4145f, 45.5641f, 37.8952f, 46.4409f, 0.707107f);
path.conicTo(38.376f, 47.3178f, 37.4991f, 47.7985f, 0.707107f);
path.close();
path.moveTo(34.6651f, 49.1368f);
path.quadTo(34.1756f, 49.3328f, 33.6785f, 49.5089f);
path.conicTo(32.7358f, 49.8427f, 32.402f, 48.9f, 0.707107f);
path.conicTo(32.0682f, 47.9574f, 33.0109f, 47.6236f, 0.707107f);
path.quadTo(33.4697f, 47.4611f, 33.9216f, 47.2801f);
path.conicTo(34.85f, 46.9084f, 35.2217f, 47.8368f, 0.707107f);
path.conicTo(35.5934f, 48.7651f, 34.6651f, 49.1368f, 0.707107f);
path.close();
path.moveTo(31.6557f, 50.1337f);
path.quadTo(31.1425f, 50.2696f, 30.6243f, 50.3844f);
path.conicTo(29.648f, 50.6007f, 29.4317f, 49.6244f, 0.707107f);
path.conicTo(29.2153f, 48.6481f, 30.1917f, 48.4317f, 0.707107f);
path.quadTo(30.6701f, 48.3257f, 31.1437f, 48.2003f);
path.conicTo(32.1104f, 47.9443f, 32.3664f, 48.911f, 0.707107f);
path.conicTo(32.6223f, 49.8777f, 31.6557f, 50.1337f, 0.707107f);
path.close();
path.moveTo(28.5567f, 50.7556f);
path.quadTo(28.0395f, 50.827f, 27.5198f, 50.8776f);
path.conicTo(26.5245f, 50.9745f, 26.4276f, 49.9792f, 0.707107f);
path.conicTo(26.3307f, 48.9839f, 27.326f, 48.887f, 0.707107f);
path.quadTo(27.8056f, 48.8403f, 28.2831f, 48.7744f);
path.conicTo(29.2737f, 48.6376f, 29.4105f, 49.6282f, 0.707107f);
path.conicTo(29.5473f, 50.6188f, 28.5567f, 50.7556f, 0.707107f);
path.close();
path.moveTo(25.4424f, 50.9962f);
path.quadTo(24.9222f, 51.0051f, 24.4022f, 50.9931f);
path.conicTo(23.4025f, 50.9701f, 23.4255f, 49.9704f, 0.707107f);
path.conicTo(23.4485f, 48.9707f, 24.4482f, 48.9937f, 0.707107f);
path.quadTo(24.9283f, 49.0047f, 25.4084f, 48.9965f);
path.conicTo(26.4083f, 48.9795f, 26.4253f, 49.9794f, 0.707107f);
path.conicTo(26.4423f, 50.9792f, 25.4424f, 50.9962f, 0.707107f);
path.close();
path.moveTo(22.3065f, 50.8601f);
path.quadTo(21.7885f, 50.8062f, 21.2732f, 50.7315f);
path.conicTo(20.2835f, 50.5882f, 20.4268f, 49.5985f, 0.707107f);
path.conicTo(20.5702f, 48.6088f, 21.5599f, 48.7522f, 0.707107f);
path.quadTo(22.0355f, 48.8211f, 22.5136f, 48.8709f);
path.conicTo(23.5083f, 48.9745f, 23.4047f, 49.9691f, 0.707107f);
path.conicTo(23.3011f, 50.9637f, 22.3065f, 50.8601f, 0.707107f);
path.close();
path.moveTo(19.2346f, 50.3527f);
path.quadTo(18.7003f, 50.2312f, 18.1717f, 50.0873f);
path.conicTo(17.2068f, 49.8247f, 17.4694f, 48.8598f, 0.707107f);
path.conicTo(17.732f, 47.8949f, 18.6969f, 48.1575f, 0.707107f);
path.quadTo(19.185f, 48.2904f, 19.6781f, 48.4025f);
path.conicTo(20.6532f, 48.6243f, 20.4314f, 49.5994f, 0.707107f);
path.conicTo(20.2097f, 50.5745f, 19.2346f, 50.3527f, 0.707107f);
path.close();
path.moveTo(16.1149f, 49.4347f);
path.quadTo(15.6161f, 49.2533f, 15.1251f, 49.0517f);
path.conicTo(14.2f, 48.6719f, 14.5798f, 47.7469f, 0.707107f);
path.conicTo(14.9596f, 46.8218f, 15.8847f, 47.2016f, 0.707107f);
path.quadTo(16.3379f, 47.3877f, 16.7984f, 47.5551f);
path.conicTo(17.7382f, 47.8969f, 17.3964f, 48.8366f, 0.707107f);
path.conicTo(17.0547f, 49.7764f, 16.1149f, 49.4347f, 0.707107f);
path.close();
path.moveTo(13.2313f, 48.184f);
path.quadTo(12.776f, 47.9529f, 12.33f, 47.704f);
path.conicTo(11.4568f, 47.2167f, 11.9441f, 46.3434f, 0.707107f);
path.conicTo(12.4314f, 45.4702f, 13.3046f, 45.9575f, 0.707107f);
path.quadTo(13.7162f, 46.1872f, 14.1365f, 46.4006f);
path.conicTo(15.0282f, 46.8532f, 14.5756f, 47.7449f, 0.707107f);
path.conicTo(14.123f, 48.6366f, 13.2313f, 48.184f, 0.707107f);
path.close();
path.moveTo(10.6208f, 46.6619f);
path.lineTo(10.4641f, 46.5571f);
path.quadTo(10.1333f, 46.334f, 9.81253f, 46.1031f);
path.conicTo(9.00087f, 45.519f, 9.585f, 44.7073f, 0.707107f);
path.conicTo(10.1691f, 43.8957f, 10.9808f, 44.4798f, 0.707107f);
path.quadTo(11.2769f, 44.6929f, 11.5763f, 44.8948f);
path.lineTo(11.7329f, 44.9996f);
path.conicTo(12.564f, 45.5557f, 12.008f, 46.3868f, 0.707107f);
path.conicTo(11.4519f, 47.2179f, 10.6208f, 46.6619f, 0.707107f);
path.close();
path.moveTo(8.22326f, 44.8631f);
path.quadTo(7.82986f, 44.5308f, 7.44999f, 44.1833f);
path.conicTo(6.71217f, 43.5082f, 7.38718f, 42.7704f, 0.707107f);
path.conicTo(8.06219f, 42.0326f, 8.8f, 42.7076f, 0.707107f);
path.quadTo(9.15066f, 43.0284f, 9.51375f, 43.3351f);
path.conicTo(10.2777f, 43.9804f, 9.63248f, 44.7443f, 0.707107f);
path.conicTo(8.98724f, 45.5083f, 8.22326f, 44.8631f, 0.707107f);
path.close();
path.moveTo(5.95972f, 42.705f);
path.quadTo(5.59577f, 42.3136f, 5.24823f, 41.9076f);
path.conicTo(4.59793f, 41.148f, 5.3576f, 40.4977f, 0.707107f);
path.conicTo(6.11728f, 39.8473f, 6.76758f, 40.607f, 0.707107f);
path.quadTo(7.08843f, 40.9818f, 7.42436f, 41.3431f);
path.conicTo(8.10532f, 42.0754f, 7.373f, 42.7564f, 0.707107f);
path.conicTo(6.64068f, 43.4373f, 5.95972f, 42.705f, 0.707107f);
path.close();
path.moveTo(3.90635f, 40.2006f);
path.quadTo(3.59492f, 39.7684f, 3.30147f, 39.3239f);
path.conicTo(2.75055f, 38.4893f, 3.58511f, 37.9384f, 0.707107f);
path.conicTo(4.41967f, 37.3875f, 4.97059f, 38.222f, 0.707107f);
path.quadTo(5.24148f, 38.6324f, 5.52894f, 39.0313f);
path.conicTo(6.11358f, 39.8426f, 5.30228f, 40.4272f, 0.707107f);
path.conicTo(4.49099f, 41.0119f, 3.90635f, 40.2006f, 0.707107f);
path.close();
path.moveTo(2.23643f, 37.5626f);
path.quadTo(1.98525f, 37.1075f, 1.75248f, 36.6427f);
path.conicTo(1.30469f, 35.7486f, 2.19883f, 35.3008f, 0.707107f);
path.conicTo(3.09296f, 34.853f, 3.54076f, 35.7471f, 0.707107f);
path.quadTo(3.75563f, 36.1762f, 3.98747f, 36.5963f);
path.conicTo(4.47065f, 37.4718f, 3.59513f, 37.955f, 0.707107f);
path.conicTo(2.71961f, 38.4382f, 2.23643f, 37.5626f, 0.707107f);
path.close();
path.moveTo(0.890647f, 34.7334f);
path.quadTo(0.69328f, 34.2445f, 0.515902f, 33.7481f);
path.conicTo(0.179435f, 32.8064f, 1.12113f, 32.4699f, 0.707107f);
path.conicTo(2.06282f, 32.1335f, 2.39929f, 33.0752f, 0.707107f);
path.quadTo(2.56303f, 33.5334f, 2.74521f, 33.9847f);
path.conicTo(3.11957f, 34.912f, 2.19229f, 35.2863f, 0.707107f);
path.conicTo(1.26501f, 35.6607f, 0.890647f, 34.7334f, 0.707107f);
path.close();
path.moveTo(-0.114587f, 31.7274f);
path.quadTo(-0.251922f, 31.2147f, -0.368218f, 30.6968f);
path.conicTo(-0.587327f, 29.7211f, 0.388373f, 29.502f, 0.707107f);
path.conicTo(1.36407f, 29.2829f, 1.58318f, 30.2586f, 0.707107f);
path.quadTo(1.69053f, 30.7366f, 1.8173f, 31.2099f);
path.conicTo(2.07605f, 32.1758f, 1.1101f, 32.4346f, 0.707107f);
path.conicTo(0.144159f, 32.6933f, -0.114587f, 31.7274f, 0.707107f);
path.close();
path.moveTo(-0.745485f, 28.6291f);
path.quadTo(-0.818367f, 28.112f, -0.870432f, 27.5925f);
path.conicTo(-0.970142f, 26.5974f, 0.0248742f, 26.4977f, 0.707107f);
path.conicTo(1.01989f, 26.398f, 1.1196f, 27.393f, 0.707107f);
path.quadTo(1.16766f, 27.8726f, 1.23494f, 28.3499f);
path.conicTo(1.37452f, 29.3401f, 0.384305f, 29.4797f, 0.707107f);
path.conicTo(-0.605905f, 29.6193f, -0.745485f, 28.6291f, 0.707107f);
path.close();
path.moveTo(-0.994901f, 25.515f);
path.quadTo(-1.00519f, 24.9955f, -0.994722f, 24.4761f);
path.conicTo(-0.97457f, 23.4763f, 0.0252273f, 23.4964f, 0.707107f);
path.conicTo(1.02502f, 23.5166f, 1.00487f, 24.5164f, 0.707107f);
path.quadTo(0.995207f, 24.9959f, 1.00471f, 25.4754f);
path.conicTo(1.02451f, 26.4752f, 0.0247103f, 26.495f, 0.707107f);
path.conicTo(-0.975093f, 26.5148f, -0.994901f, 25.515f, 0.707107f);
path.close();
path.moveTo(-0.867571f, 22.3792f);
path.quadTo(-0.81506f, 21.8609f, -0.741825f, 21.3451f);
path.conicTo(-0.60125f, 20.355f, 0.38882f, 20.4956f, 0.707107f);
path.conicTo(1.37889f, 20.6361f, 1.23831f, 21.6262f, 0.707107f);
path.quadTo(1.17071f, 22.1023f, 1.12224f, 22.5807f);
path.conicTo(1.02144f, 23.5757f, 0.026537f, 23.4749f, 0.707107f);
path.conicTo(-0.96837f, 23.3741f, -0.867571f, 22.3792f, 0.707107f);
path.close();
path.moveTo(-0.369678f, 19.3097f);
path.quadTo(-0.249693f, 18.7748f, -0.107265f, 18.2453f);
path.conicTo(0.152529f, 17.2797f, 1.11819f, 17.5395f, 0.707107f);
path.conicTo(2.08386f, 17.7993f, 1.82406f, 18.7649f, 0.707107f);
path.quadTo(1.69259f, 19.2536f, 1.58184f, 19.7474f);
path.conicTo(1.36298f, 20.7232f, 0.387221f, 20.5043f, 0.707107f);
path.conicTo(-0.588536f, 20.2855f, -0.369678f, 19.3097f, 0.707107f);
path.close();
path.moveTo(0.539863f, 16.1851f);
path.quadTo(0.719962f, 15.6854f, 0.920307f, 15.1934f);
path.conicTo(1.29748f, 14.2673f, 2.22362f, 14.6445f, 0.707107f);
path.conicTo(3.14976f, 15.0216f, 2.7726f, 15.9478f, 0.707107f);
path.quadTo(2.58765f, 16.4019f, 2.42141f, 16.8632f);
path.conicTo(2.08237f, 17.804f, 1.1416f, 17.4649f, 0.707107f);
path.conicTo(0.200823f, 17.1259f, 0.539863f, 16.1851f, 0.707107f);
path.close();
path.moveTo(1.78353f, 13.2955f);
path.quadTo(2.01364f, 12.8391f, 2.26151f, 12.392f);
path.conicTo(2.74643f, 11.5175f, 3.62099f, 12.0024f, 0.707107f);
path.conicTo(4.49555f, 12.4873f, 4.01063f, 13.3618f, 0.707107f);
path.quadTo(3.78183f, 13.7745f, 3.56941f, 14.1958f);
path.conicTo(3.11923f, 15.0888f, 2.22629f, 14.6386f, 0.707107f);
path.conicTo(1.33336f, 14.1884f, 1.78353f, 13.2955f, 0.707107f);
path.close();
path.moveTo(3.30083f, 10.6771f);
path.lineTo(3.44218f, 10.4652f);
path.quadTo(3.6466f, 10.1621f, 3.85641f, 9.86895f);
path.conicTo(4.43837f, 9.05574f, 5.25159f, 9.6377f, 0.707107f);
path.conicTo(6.0648f, 10.2197f, 5.48284f, 11.0329f, 0.707107f);
path.quadTo(5.28917f, 11.3035f, 5.10592f, 11.5752f);
path.lineTo(4.96457f, 11.787f);
path.conicTo(4.4096f, 12.6189f, 3.57773f, 12.0639f, 0.707107f);
path.conicTo(2.74586f, 11.509f, 3.30083f, 10.6771f, 0.707107f);
path.close();
path.moveTo(5.0909f, 8.27793f);
path.quadTo(5.42174f, 7.88403f, 5.76791f, 7.50353f);
path.conicTo(6.44085f, 6.76383f, 7.18054f, 7.43678f, 0.707107f);
path.conicTo(7.92024f, 8.10972f, 7.24729f, 8.84942f, 0.707107f);
path.quadTo(6.92775f, 9.20065f, 6.62237f, 9.56424f);
path.conicTo(5.97921f, 10.33f, 5.21348f, 9.68682f, 0.707107f);
path.conicTo(4.44774f, 9.04367f, 5.0909f, 8.27793f, 0.707107f);
path.close();
path.moveTo(7.24064f, 6.0104f);
path.quadTo(7.63069f, 5.64561f, 8.03537f, 5.29717f);
path.conicTo(8.79318f, 4.64469f, 9.44566f, 5.40249f, 0.707107f);
path.conicTo(10.0981f, 6.16029f, 9.34034f, 6.81278f, 0.707107f);
path.quadTo(8.96678f, 7.13442f, 8.60675f, 7.47113f);
path.conicTo(7.87638f, 8.15419f, 7.19332f, 7.42382f, 0.707107f);
path.conicTo(6.51027f, 6.69345f, 7.24064f, 6.0104f, 0.707107f);
path.close();
path.moveTo(9.73726f, 3.95128f);
path.quadTo(10.1706f, 3.63704f, 10.6165f, 3.34092f);
path.conicTo(11.4496f, 2.78771f, 12.0028f, 3.62075f, 0.707107f);
path.conicTo(12.556f, 4.4538f, 11.7229f, 5.007f, 0.707107f);
path.quadTo(11.3113f, 5.28035f, 10.9113f, 5.57041f);
path.conicTo(10.1018f, 6.15744f, 9.51472f, 5.34787f, 0.707107f);
path.conicTo(8.92769f, 4.53831f, 9.73726f, 3.95128f, 0.707107f);
path.close();
path.moveTo(12.374f, 2.27153f);
path.quadTo(12.8282f, 2.01921f, 13.2921f, 1.78522f);
path.conicTo(14.185f, 1.33492f, 14.6353f, 2.22779f, 0.707107f);
path.conicTo(15.0856f, 3.12067f, 14.1927f, 3.57097f, 0.707107f);
path.quadTo(13.7645f, 3.78696f, 13.3452f, 4.01988f);
path.conicTo(12.471f, 4.5055f, 11.9854f, 3.63132f, 0.707107f);
path.conicTo(11.4998f, 2.75715f, 12.374f, 2.27153f, 0.707107f);
path.close();
path.moveTo(15.1984f, 0.918296f);
path.quadTo(15.6866f, 0.719602f, 16.1824f, 0.540851f);
path.conicTo(17.1231f, 0.20171f, 17.4623f, 1.14245f, 0.707107f);
path.conicTo(17.8014f, 2.08318f, 16.8607f, 2.42232f, 0.707107f);
path.quadTo(16.403f, 2.58733f, 15.9524f, 2.77074f);
path.conicTo(15.0261f, 3.14772f, 14.6492f, 2.2215f, 0.707107f);
path.conicTo(14.2722f, 1.29528f, 15.1984f, 0.918296f, 0.707107f);
path.close();
path.moveTo(18.201f, -0.0952874f);
path.quadTo(18.7132f, -0.234075f, 19.2308f, -0.351842f);
path.conicTo(20.2058f, -0.573734f, 20.4277f, 0.401338f, 0.707107f);
path.conicTo(20.6496f, 1.37641f, 19.6745f, 1.5983f, 0.707107f);
path.quadTo(19.1968f, 1.70701f, 18.724f, 1.83512f);
path.conicTo(17.7588f, 2.09662f, 17.4973f, 1.13142f, 0.707107f);
path.conicTo(17.2358f, 0.166216f, 18.201f, -0.0952874f, 0.707107f);
path.close();
path.moveTo(21.2986f, -0.73518f);
path.quadTo(21.8155f, -0.809526f, 22.3349f, -0.863052f);
path.conicTo(23.3297f, -0.965552f, 23.4322f, 0.029181f, 0.707107f);
path.conicTo(23.5347f, 1.02391f, 22.5399f, 1.12641f, 0.707107f);
path.quadTo(22.0604f, 1.17582f, 21.5833f, 1.24445f);
path.conicTo(20.5935f, 1.38681f, 20.4511f, 0.397f, 0.707107f);
path.conicTo(20.3088f, -0.592814f, 21.2986f, -0.73518f, 0.707107f);
path.close();
path.moveTo(24.4124f, -0.993361f);
path.quadTo(24.9312f, -1.00509f, 25.4501f, -0.996107f);
path.conicTo(26.4499f, -0.978799f, 26.4326f, 0.0210512f, 0.707107f);
path.conicTo(26.4153f, 1.0209f, 25.4155f, 1.00359f, 0.707107f);
path.quadTo(24.9365f, 0.995302f, 24.4576f, 1.00613f);
path.conicTo(23.4578f, 1.02873f, 23.4352f, 0.0289853f, 0.707107f);
path.conicTo(23.4126f, -0.970759f, 24.4124f, -0.993361f, 0.707107f);
path.close();
path.moveTo(27.5481f, -0.87484f);
path.quadTo(28.0668f, -0.823762f, 28.583f, -0.75194f);
path.conicTo(29.5734f, -0.614138f, 29.4356f, 0.376322f, 0.707107f);
path.conicTo(29.2978f, 1.36678f, 28.3074f, 1.22898f, 0.707107f);
path.quadTo(27.8309f, 1.16268f, 27.3521f, 1.11553f);
path.conicTo(26.3569f, 1.01753f, 26.4549f, 0.0223428f, 0.707107f);
path.conicTo(26.5529f, -0.972843f, 27.5481f, -0.87484f, 0.707107f);
path.close();
path.moveTo(30.6151f, -0.386432f);
path.quadTo(31.1507f, -0.267954f, 31.6809f, -0.126991f);
path.conicTo(32.6473f, 0.129965f, 32.3904f, 1.09639f, 0.707107f);
path.conicTo(32.1334f, 2.06281f, 31.167f, 1.80585f, 0.707107f);
path.quadTo(30.6776f, 1.67574f, 30.1832f, 1.56637f);
path.conicTo(29.2068f, 1.35041f, 29.4227f, 0.374005f, 0.707107f);
path.conicTo(29.6387f, -0.602396f, 30.6151f, -0.386432f, 0.707107f);
path.close();
path.moveTo(33.7445f, 0.514616f);
path.quadTo(34.2452f, 0.693421f, 34.7381f, 0.892536f);
path.conicTo(35.6653f, 1.26708f, 35.2908f, 2.19429f, 0.707107f);
path.conicTo(34.9162f, 3.1215f, 33.989f, 2.74696f, 0.707107f);
path.quadTo(33.534f, 2.56316f, 33.0718f, 2.3981f);
path.conicTo(32.1301f, 2.06177f, 32.4664f, 1.12003f, 0.707107f);
path.conicTo(32.8027f, 0.178285f, 33.7445f, 0.514616f, 0.707107f);
path.close();
path.moveTo(36.6402f, 1.7512f);
path.quadTo(37.0977f, 1.98026f, 37.5458f, 2.22715f);
path.conicTo(38.4217f, 2.70968f, 37.9392f, 3.58556f, 0.707107f);
path.conicTo(37.4566f, 4.46144f, 36.5808f, 3.97891f, 0.707107f);
path.quadTo(36.1671f, 3.75102f, 35.7448f, 3.53956f);
path.conicTo(34.8506f, 3.09185f, 35.2983f, 2.19767f, 0.707107f);
path.conicTo(35.746f, 1.30349f, 36.6402f, 1.7512f, 0.707107f);
path.close();
path.moveTo(39.2611f, 3.26012f);
path.quadTo(39.4005f, 3.35159f, 39.539f, 3.44501f);
path.quadTo(39.8091f, 3.62717f, 40.0746f, 3.81611f);
path.conicTo(40.8893f, 4.3959f, 40.3096f, 5.21067f, 0.707107f);
path.conicTo(39.7298f, 6.02543f, 38.915f, 5.44564f, 0.707107f);
path.quadTo(38.67f, 5.2713f, 38.4206f, 5.10309f);
path.quadTo(38.293f, 5.017f, 38.164f, 4.9324f);
path.conicTo(37.3279f, 4.38388f, 37.8764f, 3.54775f, 0.707107f);
path.conicTo(38.4249f, 2.71161f, 39.2611f, 3.26012f, 0.707107f);
path.close();
path.moveTo(41.6673f, 5.04503f);
path.quadTo(42.0618f, 5.37449f, 42.4428f, 5.71927f);
path.conicTo(43.1844f, 6.39015f, 42.5135f, 7.13171f, 0.707107f);
path.conicTo(41.8426f, 7.87327f, 41.1011f, 7.20239f, 0.707107f);
path.quadTo(40.7493f, 6.88414f, 40.3852f, 6.58004f);
path.conicTo(39.6177f, 5.93899f, 40.2588f, 5.17149f, 0.707107f);
path.conicTo(40.8998f, 4.40399f, 41.6673f, 5.04503f, 0.707107f);
path.close();
path.moveTo(43.9388f, 7.1865f);
path.quadTo(44.3044f, 7.57519f, 44.6538f, 7.97856f);
path.conicTo(45.3084f, 8.73448f, 44.5525f, 9.38914f, 0.707107f);
path.conicTo(43.7966f, 10.0438f, 43.1419f, 9.28789f, 0.707107f);
path.quadTo(42.8195f, 8.91555f, 42.482f, 8.55677f);
path.conicTo(41.7969f, 7.82836f, 42.5253f, 7.14322f, 0.707107f);
path.conicTo(43.2537f, 6.45808f, 43.9388f, 7.1865f, 0.707107f);
path.close();
path.moveTo(46.0036f, 9.6753f);
path.quadTo(46.3207f, 10.1098f, 46.6195f, 10.5571f);
path.conicTo(47.175f, 11.3886f, 46.3435f, 11.9441f, 0.707107f);
path.conicTo(45.5119f, 12.4996f, 44.9564f, 11.6681f, 0.707107f);
path.quadTo(44.6806f, 11.2552f, 44.388f, 10.8541f);
path.conicTo(43.7986f, 10.0463f, 44.6064f, 9.45688f, 0.707107f);
path.conicTo(45.4142f, 8.86747f, 46.0036f, 9.6753f, 0.707107f);
path.close();
path.moveTo(47.6932f, 12.3107f);
path.quadTo(47.9467f, 12.764f, 48.1819f, 13.2271f);
path.conicTo(48.6347f, 14.1187f, 47.7431f, 14.5715f, 0.707107f);
path.conicTo(46.8514f, 15.0243f, 46.3986f, 14.1327f, 0.707107f);
path.quadTo(46.1816f, 13.7053f, 45.9476f, 13.2868f);
path.conicTo(45.4595f, 12.414f, 46.3323f, 11.9259f, 0.707107f);
path.conicTo(47.2051f, 11.4379f, 47.6932f, 12.3107f, 0.707107f);
path.close();
path.moveTo(49.0539f, 15.1303f);
path.quadTo(49.2539f, 15.6178f, 49.434f, 16.113f);
path.conicTo(49.7758f, 17.0527f, 48.836f, 17.3946f, 0.707107f);
path.conicTo(47.8963f, 17.7364f, 47.5545f, 16.7966f, 0.707107f);
path.quadTo(47.3882f, 16.3395f, 47.2036f, 15.8895f);
path.conicTo(46.824f, 14.9643f, 47.7491f, 14.5847f, 0.707107f);
path.conicTo(48.6743f, 14.2051f, 49.0539f, 15.1303f, 0.707107f);
path.close();
path.moveTo(50.0758f, 18.1294f);
path.quadTo(50.216f, 18.6412f, 50.3352f, 19.1584f);
path.conicTo(50.5599f, 20.1328f, 49.5855f, 20.3575f, 0.707107f);
path.conicTo(48.6111f, 20.5821f, 48.3864f, 19.6077f, 0.707107f);
path.quadTo(48.2763f, 19.1304f, 48.1469f, 18.6579f);
path.conicTo(47.8826f, 17.6935f, 48.8471f, 17.4292f, 0.707107f);
path.conicTo(49.8115f, 17.165f, 50.0758f, 18.1294f, 0.707107f);
path.close();
path.moveTo(50.7247f, 21.2262f);
path.quadTo(50.8005f, 21.743f, 50.8555f, 22.2623f);
path.conicTo(50.9607f, 23.2568f, 49.9663f, 23.3621f, 0.707107f);
path.conicTo(48.9719f, 23.4673f, 48.8666f, 22.4729f, 0.707107f);
path.quadTo(48.8158f, 21.9935f, 48.7458f, 21.5165f);
path.conicTo(48.6007f, 20.5271f, 49.5901f, 20.382f, 0.707107f);
path.conicTo(50.5795f, 20.2368f, 50.7247f, 21.2262f, 0.707107f);
path.close();
path.moveTo(50.9916f, 24.3398f);
path.quadTo(51.0048f, 24.858f, 50.9973f, 25.3762f);
path.conicTo(50.9828f, 26.3761f, 49.9829f, 26.3616f, 0.707107f);
path.conicTo(48.983f, 26.3472f, 48.9975f, 25.3473f, 0.707107f);
path.quadTo(49.0044f, 24.8687f, 48.9923f, 24.3906f);
path.conicTo(48.9669f, 23.3909f, 49.9665f, 23.3655f, 0.707107f);
path.conicTo(50.9662f, 23.3401f, 50.9916f, 24.3398f, 0.707107f);
path.close();
path.moveTo(50.8819f, 27.4753f);
path.quadTo(50.8323f, 27.9943f, 50.7618f, 28.511f);
path.conicTo(50.6268f, 29.5018f, 49.636f, 29.3668f, 0.707107f);
path.conicTo(48.6451f, 29.2317f, 48.7802f, 28.2409f, 0.707107f);
path.quadTo(48.8452f, 27.7641f, 48.891f, 27.2849f);
path.conicTo(48.9862f, 26.2894f, 49.9816f, 26.3846f, 0.707107f);
path.conicTo(50.9771f, 26.4798f, 50.8819f, 27.4753f, 0.707107f);
path.close();
path.moveTo(50.4023f, 30.5429f);
path.quadTo(50.2856f, 31.0775f, 50.1465f, 31.607f);
path.conicTo(49.8924f, 32.5742f, 48.9252f, 32.3201f, 0.707107f);
path.conicTo(47.9581f, 32.066f, 48.2122f, 31.0988f, 0.707107f);
path.quadTo(48.3405f, 30.6102f, 48.4483f, 30.1165f);
path.conicTo(48.6614f, 29.1395f, 49.6385f, 29.3527f, 0.707107f);
path.conicTo(50.6155f, 29.5659f, 50.4023f, 30.5429f, 0.707107f);
path.close();
path.moveTo(49.5104f, 33.674f);
path.quadTo(49.3329f, 34.1756f, 49.1351f, 34.6695f);
path.conicTo(48.7632f, 35.5977f, 47.8349f, 35.2258f, 0.707107f);
path.conicTo(46.9066f, 34.854f, 47.2785f, 33.9257f, 0.707107f);
path.quadTo(47.4612f, 33.4697f, 47.625f, 33.0067f);
path.conicTo(47.9587f, 32.064f, 48.9014f, 32.3977f, 0.707107f);
path.conicTo(49.8441f, 32.7313f, 49.5104f, 33.674f, 0.707107f);
path.close();
path.moveTo(48.281f, 36.5756f);
path.quadTo(48.053f, 37.0342f, 47.8071f, 37.4835f);
path.conicTo(47.3269f, 38.3607f, 46.4497f, 37.8805f, 0.707107f);
path.conicTo(45.5725f, 37.4004f, 46.0527f, 36.5232f, 0.707107f);
path.quadTo(46.2797f, 36.1085f, 46.4901f, 35.6852f);
path.conicTo(46.9353f, 34.7898f, 47.8307f, 35.235f, 0.707107f);
path.conicTo(48.7262f, 35.6802f, 48.281f, 36.5756f, 0.707107f);
path.close();
path.moveTo(46.7777f, 39.2033f);
path.quadTo(46.6677f, 39.3719f, 46.555f, 39.539f);
path.quadTo(46.3865f, 39.7888f, 46.2121f, 40.0349f);
path.conicTo(45.6338f, 40.8507f, 44.818f, 40.2724f, 0.707107f);
path.conicTo(44.0021f, 39.6942f, 44.5804f, 38.8783f, 0.707107f);
path.quadTo(44.7413f, 38.6513f, 44.8969f, 38.4206f);
path.quadTo(45.0008f, 38.2665f, 45.1025f, 38.1107f);
path.conicTo(45.6488f, 37.2731f, 46.4864f, 37.8194f, 0.707107f);
path.conicTo(47.324f, 38.3657f, 46.7777f, 39.2033f, 0.707107f);
path.close();
path.moveTo(44.9527f, 41.6701f);
path.quadTo(44.6177f, 42.0709f, 44.267f, 42.458f);
path.conicTo(43.5955f, 43.1991f, 42.8545f, 42.5276f, 0.707107f);
path.conicTo(42.1135f, 41.8561f, 42.7849f, 41.1151f, 0.707107f);
path.quadTo(43.1087f, 40.7578f, 43.4178f, 40.3878f);
path.conicTo(44.059f, 39.6203f, 44.8264f, 40.2615f, 0.707107f);
path.conicTo(45.5938f, 40.9027f, 44.9527f, 41.6701f, 0.707107f);
path.close();
path.moveTo(42.7884f, 43.9624f);
path.quadTo(42.4083f, 44.319f, 42.014f, 44.6602f);
path.conicTo(41.2578f, 45.3146f, 40.6034f, 44.5585f, 0.707107f);
path.conicTo(39.949f, 43.8023f, 40.7052f, 43.1479f, 0.707107f);
path.quadTo(41.0691f, 42.833f, 41.4201f, 42.5037f);
path.conicTo(42.1494f, 41.8196f, 42.8336f, 42.5489f, 0.707107f);
path.conicTo(43.5178f, 43.2782f, 42.7884f, 43.9624f, 0.707107f);
path.close();
path.moveTo(40.3892f, 45.9564f);
path.quadTo(39.9683f, 46.2655f, 39.5354f, 46.5574f);
path.conicTo(38.7062f, 47.1165f, 38.1472f, 46.2873f, 0.707107f);
path.conicTo(37.5881f, 45.4582f, 38.4173f, 44.8992f, 0.707107f);
path.quadTo(38.8169f, 44.6297f, 39.2054f, 44.3444f);
path.conicTo(40.0114f, 43.7525f, 40.6033f, 44.5585f, 0.707107f);
path.conicTo(41.1952f, 45.3645f, 40.3892f, 45.9564f, 0.707107f);
path.close();
path.moveTo(37.7543f, 47.6568f);
path.quadTo(37.2977f, 47.9138f, 36.8312f, 48.1522f);
path.conicTo(35.9407f, 48.6072f, 35.4857f, 47.7167f, 0.707107f);
path.conicTo(35.0306f, 46.8263f, 35.9211f, 46.3712f, 0.707107f);
path.quadTo(36.3518f, 46.1511f, 36.7732f, 45.9139f);
path.conicTo(37.6446f, 45.4234f, 38.1351f, 46.2948f, 0.707107f);
path.conicTo(38.6257f, 47.1662f, 37.7543f, 47.6568f, 0.707107f);
path.close();
path.moveTo(34.9311f, 49.0286f);
path.quadTo(34.4488f, 49.2279f, 33.9589f, 49.4077f);
path.conicTo(33.0202f, 49.7523f, 32.6756f, 48.8136f, 0.707107f);
path.conicTo(32.331f, 47.8748f, 33.2698f, 47.5302f, 0.707107f);
path.quadTo(33.722f, 47.3642f, 34.1672f, 47.1802f);
path.conicTo(35.0914f, 46.7983f, 35.4733f, 47.7224f, 0.707107f);
path.conicTo(35.8553f, 48.6466f, 34.9311f, 49.0286f, 0.707107f);
path.close();
path.moveTo(31.9824f, 50.0449f);
path.quadTo(31.4774f, 50.1857f, 30.9668f, 50.3061f);
path.conicTo(29.9935f, 50.5355f, 29.764f, 49.5622f, 0.707107f);
path.conicTo(29.5346f, 48.5889f, 30.5079f, 48.3594f, 0.707107f);
path.quadTo(30.9789f, 48.2484f, 31.4453f, 48.1184f);
path.conicTo(32.4086f, 47.8498f, 32.6771f, 48.8131f, 0.707107f);
path.conicTo(32.9457f, 49.7763f, 31.9824f, 50.0449f, 0.707107f);
path.close();
path.moveTo(28.899f, 50.706f);
path.quadTo(28.3834f, 50.7842f, 27.8652f, 50.8416f);
path.conicTo(26.8713f, 50.9518f, 26.7611f, 49.9579f, 0.707107f);
path.conicTo(26.6509f, 48.964f, 27.6448f, 48.8538f, 0.707107f);
path.quadTo(28.1231f, 48.8008f, 28.599f, 48.7286f);
path.conicTo(29.5877f, 48.5786f, 29.7377f, 49.5673f, 0.707107f);
path.conicTo(29.8877f, 50.556f, 28.899f, 50.706f, 0.707107f);
path.close();
path.moveTo(25.8106f, 50.9874f);
path.quadTo(25.6321f, 50.9929f, 25.4537f, 50.996f);
path.conicTo(24.4539f, 51.0135f, 24.4365f, 50.0136f, 0.707115f);
path.lineTo(24.4251f, 49.3638f);
path.conicTo(24.4077f, 48.364f, 25.4075f, 48.3465f, 0.707107f);
path.conicTo(26.4073f, 48.3291f, 26.4248f, 49.3289f, 0.707107f);
path.lineTo(26.4361f, 49.9787f);
path.lineTo(25.4363f, 49.9962f);
path.lineTo(25.4189f, 48.9963f);
path.quadTo(25.5836f, 48.9935f, 25.7482f, 48.9883f);
path.conicTo(26.7477f, 48.9571f, 26.7789f, 49.9567f, 0.707107f);
path.conicTo(26.8101f, 50.9562f, 25.8106f, 50.9874f, 0.707107f);
path.close();
path.moveTo(24.3902f, 47.3641f);
path.lineTo(24.3728f, 46.3643f);
path.conicTo(24.3553f, 45.3645f, 25.3551f, 45.347f, 0.707107f);
path.conicTo(26.355f, 45.3295f, 26.3724f, 46.3294f, 0.707107f);
path.lineTo(26.3899f, 47.3292f);
path.conicTo(26.4074f, 48.3291f, 25.4075f, 48.3465f, 0.707107f);
path.conicTo(24.4077f, 48.364f, 24.3902f, 47.3641f, 0.707107f);
path.close();
path.moveTo(24.3378f, 44.3646f);
path.lineTo(24.3204f, 43.3648f);
path.conicTo(24.3029f, 42.3649f, 25.3028f, 42.3475f, 0.707107f);
path.conicTo(26.3026f, 42.33f, 26.3201f, 43.3298f, 0.707107f);
path.lineTo(26.3375f, 44.3297f);
path.conicTo(26.355f, 45.3295f, 25.3551f, 45.347f, 0.707107f);
path.conicTo(24.3553f, 45.3645f, 24.3378f, 44.3646f, 0.707107f);
path.close();
path.moveTo(24.2855f, 41.3651f);
path.lineTo(24.268f, 40.3652f);
path.conicTo(24.2506f, 39.3654f, 25.2504f, 39.3479f, 0.707107f);
path.conicTo(26.2503f, 39.3305f, 26.2677f, 40.3303f, 0.707107f);
path.lineTo(26.2852f, 41.3302f);
path.conicTo(26.3026f, 42.33f, 25.3028f, 42.3475f, 0.707107f);
path.conicTo(24.3029f, 42.3649f, 24.2855f, 41.3651f, 0.707107f);
path.close();
path.moveTo(24.2331f, 38.3655f);
path.lineTo(24.2157f, 37.3657f);
path.conicTo(24.1982f, 36.3658f, 25.1981f, 36.3484f, 0.707107f);
path.conicTo(26.1979f, 36.3309f, 26.2154f, 37.3308f, 0.707107f);
path.lineTo(26.2328f, 38.3306f);
path.conicTo(26.2503f, 39.3305f, 25.2504f, 39.3479f, 0.707107f);
path.conicTo(24.2506f, 39.3654f, 24.2331f, 38.3655f, 0.707107f);
path.close();
path.moveTo(24.1808f, 35.366f);
path.lineTo(24.1633f, 34.3661f);
path.conicTo(24.1459f, 33.3663f, 25.1457f, 33.3488f, 0.707107f);
path.conicTo(26.1456f, 33.3314f, 26.163f, 34.3312f, 0.707107f);
path.lineTo(26.1805f, 35.3311f);
path.conicTo(26.1979f, 36.3309f, 25.1981f, 36.3484f, 0.707107f);
path.conicTo(24.1982f, 36.3658f, 24.1808f, 35.366f, 0.707107f);
path.close();
path.moveTo(24.1284f, 32.3664f);
path.lineTo(24.111f, 31.3666f);
path.conicTo(24.0935f, 30.3667f, 25.0934f, 30.3493f, 0.707107f);
path.conicTo(26.0932f, 30.3318f, 26.1107f, 31.3317f, 0.707107f);
path.lineTo(26.1281f, 32.3315f);
path.conicTo(26.1456f, 33.3314f, 25.1457f, 33.3488f, 0.707107f);
path.conicTo(24.1459f, 33.3663f, 24.1284f, 32.3664f, 0.707107f);
path.close();
path.moveTo(24.0761f, 29.3669f);
path.lineTo(24.0586f, 28.367f);
path.conicTo(24.0412f, 27.3672f, 25.041f, 27.3497f, 0.707107f);
path.conicTo(26.0409f, 27.3323f, 26.0583f, 28.3321f, 0.707107f);
path.lineTo(26.0758f, 29.332f);
path.conicTo(26.0932f, 30.3318f, 25.0934f, 30.3493f, 0.707107f);
path.conicTo(24.0935f, 30.3667f, 24.0761f, 29.3669f, 0.707107f);
path.close();
path.moveTo(24.0237f, 26.3673f);
path.lineTo(24.0063f, 25.3675f);
path.conicTo(23.9888f, 24.3676f, 24.9887f, 24.3502f, 0.707107f);
path.conicTo(25.9885f, 24.3327f, 26.006f, 25.3326f, 0.707107f);
path.lineTo(26.0234f, 26.3324f);
path.conicTo(26.0409f, 27.3323f, 25.041f, 27.3497f, 0.707107f);
path.conicTo(24.0412f, 27.3672f, 24.0237f, 26.3673f, 0.707107f);
path.close();
    testPathOpFail(reporter, path, path1, kXOR_SkPathOp, filename);
}

static void op_1(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.setFillType(SkPath::kWinding_FillType);
path.moveTo(SkBits2Float(0x15e80300), SkBits2Float(0x400004dc));  // 9.37088e-26f, 2.0003f
path.quadTo(SkBits2Float(0xe56c206c), SkBits2Float(0x646c5f40), SkBits2Float(0x6c80885e), SkBits2Float(0xb4bc576c));  // -6.96923e+22f, 1.74412e+22f, 1.24309e+27f, -3.50813e-07f

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.setFillType(SkPath::kWinding_FillType);
path.moveTo(SkBits2Float(0x1b000010), SkBits2Float(0x6e5a5a1b));  // 1.05879e-22f, 1.68942e+28f
path.quadTo(SkBits2Float(0xef646464), SkBits2Float(0xefefefef), SkBits2Float(0x000000ef), SkBits2Float(0x1bb4bc00));  // -7.06839e+28f, -1.48514e+29f, 3.3491e-43f, 2.99e-22f

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 2, filename);
}


static void op_2(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.setFillType(SkPath::kEvenOdd_FillType);
path.moveTo(SkBits2Float(0xeee3ef57), SkBits2Float(0xef6300f8));  // -3.52712e+28f, -7.02543e+28f
path.quadTo(SkBits2Float(0xeeee9c6e), SkBits2Float(0xef609993), SkBits2Float(0x00000000), SkBits2Float(0x6e5a5a1b));  // -3.69233e+28f, -6.95103e+28f, 0, 1.68942e+28f
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.quadTo(SkBits2Float(0xe56c206c), SkBits2Float(0x646c5f40), SkBits2Float(0x6c80885e), SkBits2Float(0x00000000));  // -6.96923e+22f, 1.74412e+22f, 1.24309e+27f, 0
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.quadTo(SkBits2Float(0xeeda2c5a), SkBits2Float(0xef6533a7), SkBits2Float(0xeee3ef57), SkBits2Float(0xef6300f8));  // -3.37607e+28f, -7.09345e+28f, -3.52712e+28f, -7.02543e+28f
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.setFillType(SkPath::kWinding_FillType);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.lineTo(SkBits2Float(0x1b1b1b00), SkBits2Float(0x1b5a5a1b));  // 1.283e-22f, 1.80617e-22f

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 0, filename);
}


static void op_3(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.setFillType(SkPath::kEvenOdd_FillType);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x6e5a5a1b));  // 0, 1.68942e+28f
path.quadTo(SkBits2Float(0xeeee9c6e), SkBits2Float(0xef609993), SkBits2Float(0xeee3ef57), SkBits2Float(0xef6300f8));  // -3.69233e+28f, -6.95103e+28f, -3.52712e+28f, -7.02543e+28f
path.quadTo(SkBits2Float(0xeeda2c5a), SkBits2Float(0xef6533a7), SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // -3.37607e+28f, -7.09345e+28f, 0, 0
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0x6e5a5a1b));  // 0, 1.68942e+28f
path.close();
path.moveTo(SkBits2Float(0x6c80885e), SkBits2Float(0x00000000));  // 1.24309e+27f, 0
path.quadTo(SkBits2Float(0xe56c206c), SkBits2Float(0x646c5f40), SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // -6.96923e+22f, 1.74412e+22f, 0, 0
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.lineTo(SkBits2Float(0x6c80885e), SkBits2Float(0x00000000));  // 1.24309e+27f, 0
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.setFillType(SkPath::kWinding_FillType);

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 0, filename);
}

static void op_4(skiatest::Reporter* reporter, const char* filename) {
   SkPath patha, pathb;

   patha.setFillType(SkPath::kEvenOdd_FillType);
   patha.moveTo(SkBits2Float(0x40d7ea90), SkBits2Float(0x3fa58930));  // 6.74738f, 1.29325f
   patha.lineTo(SkBits2Float(0x40ad3d93), SkBits2Float(0x3fa58930));  // 5.41377f, 1.29325f
   patha.lineTo(SkBits2Float(0x40ad3d93), SkBits2Float(0x3edba819));  // 5.41377f, 0.429017f
   patha.lineTo(SkBits2Float(0x40fc41e0), SkBits2Float(0x3edba819));  // 7.88304f, 0.429017f
   patha.lineTo(SkBits2Float(0x40fc41e0), SkBits2Float(0x3f3b7c94));  // 7.88304f, 0.73237f
   patha.lineTo(SkBits2Float(0x40d7ea90), SkBits2Float(0x3f3b7c94));  // 6.74738f, 0.73237f
   patha.lineTo(SkBits2Float(0x40d7ea90), SkBits2Float(0x3fa58930));  // 6.74738f, 1.29325f
   patha.close();

   pathb.setFillType(SkPath::kEvenOdd_FillType);
   pathb.moveTo(SkBits2Float(0x40d7ea89), SkBits2Float(0x409a721d));  // 6.74738f, 4.82643f
   pathb.lineTo(SkBits2Float(0x411a9d73), SkBits2Float(0x409a721d));  // 9.66344f, 4.82643f
   pathb.lineTo(SkBits2Float(0x411a9d73), SkBits2Float(0x3f3b7c9a));  // 9.66344f, 0.73237f
   pathb.lineTo(SkBits2Float(0x40d7ea89), SkBits2Float(0x3f3b7c9a));  // 6.74738f, 0.73237f
   pathb.lineTo(SkBits2Float(0x40d7ea89), SkBits2Float(0x409a721d));  // 6.74738f, 4.82643f
   pathb.close();
    testPathOp(reporter, patha, pathb, kDifference_SkPathOp, filename);
}

static void bug8228(skiatest::Reporter* reporter, const char* filename) {
    SkPath path1;
    path1.moveTo(SkBits2Float(0x41fd5557), SkBits2Float(0x4292aaab));
    path1.lineTo(SkBits2Float(0x41fd5557), SkBits2Float(0x41555556));
    path1.conicTo(SkBits2Float(0x41fd5557), SkBits2Float(0x41200002), SkBits2Float(0x420c0000), SkBits2Float(0x41200002), SkBits2Float(0x3f3504f3));
    path1.lineTo(SkBits2Float(0x426071c7), SkBits2Float(0x41200002));
    path1.conicTo(SkBits2Float(0x426dc71d), SkBits2Float(0x41200002), SkBits2Float(0x426dc71d), SkBits2Float(0x41555556), SkBits2Float(0x3f3504f3));
    path1.lineTo(SkBits2Float(0x426dc71d), SkBits2Float(0x4292aaab));
    path1.conicTo(SkBits2Float(0x426dc71d), SkBits2Float(0x42995555), SkBits2Float(0x426071c7), SkBits2Float(0x42995555), SkBits2Float(0x3f3504f3));
    path1.lineTo(SkBits2Float(0x420c0000), SkBits2Float(0x42995555));
    path1.conicTo(SkBits2Float(0x41fd5557), SkBits2Float(0x42995555), SkBits2Float(0x41fd5557), SkBits2Float(0x4292aaab), SkBits2Float(0x3f3504f3));
    path1.close();

    SkPath path2;
    path2.moveTo(SkBits2Float(0x41200000), SkBits2Float(0x41200000));
    path2.lineTo(SkBits2Float(0x41eb2366), SkBits2Float(0x41200000));
    path2.conicTo(SkBits2Float(0x41e9d2b6), SkBits2Float(0x4127bdec), SkBits2Float(0x41e9d2b6), SkBits2Float(0x412feb1c), SkBits2Float(0x3f7c9333));
    path2.lineTo(SkBits2Float(0x41e9d2b6), SkBits2Float(0x42855349));
    path2.conicTo(SkBits2Float(0x41e9d2b6), SkBits2Float(0x428b82b9), SkBits2Float(0x4201483b), SkBits2Float(0x428b82b9), SkBits2Float(0x3f3504f3));
    path2.lineTo(SkBits2Float(0x424fa11f), SkBits2Float(0x428b82b9));
    path2.conicTo(SkBits2Float(0x425bffff), SkBits2Float(0x428b82b9), SkBits2Float(0x425bffff), SkBits2Float(0x42855349), SkBits2Float(0x3f3504f3));
    path2.lineTo(SkBits2Float(0x425bffff), SkBits2Float(0x412feb1c));
    path2.conicTo(SkBits2Float(0x425bffff), SkBits2Float(0x4127bdec), SkBits2Float(0x425b57a7), SkBits2Float(0x41200000), SkBits2Float(0x3f7c9333));
    path2.lineTo(SkBits2Float(0x4282f24d), SkBits2Float(0x41200000));
    path2.conicTo(SkBits2Float(0x42829e21), SkBits2Float(0x4127bdec), SkBits2Float(0x42829e21), SkBits2Float(0x412feb1c), SkBits2Float(0x3f7c9333));
    path2.lineTo(SkBits2Float(0x42829e21), SkBits2Float(0x42855349));
    path2.conicTo(SkBits2Float(0x42829e21), SkBits2Float(0x428b82b9), SkBits2Float(0x4288cd91), SkBits2Float(0x428b82b9), SkBits2Float(0x3f3504f3));
    path2.lineTo(SkBits2Float(0x42affa03), SkBits2Float(0x428b82b9));
    path2.conicTo(SkBits2Float(0x42b62973), SkBits2Float(0x428b82b9), SkBits2Float(0x42b62973), SkBits2Float(0x42855349), SkBits2Float(0x3f3504f3));
    path2.lineTo(SkBits2Float(0x42b62973), SkBits2Float(0x412feb1c));
    path2.conicTo(SkBits2Float(0x42b62973), SkBits2Float(0x4127bdec), SkBits2Float(0x42b5d547), SkBits2Float(0x41200000), SkBits2Float(0x3f7c9333));
    path2.lineTo(SkBits2Float(0x42dc0000), SkBits2Float(0x41200000));
    path2.lineTo(SkBits2Float(0x42dc0000), SkBits2Float(0x42dc0000));
    path2.lineTo(SkBits2Float(0x41200000), SkBits2Float(0x42dc0000));
    path2.lineTo(SkBits2Float(0x41200000), SkBits2Float(0x41200000));
    path2.close();
    testPathOp(reporter, path1, path2, kIntersect_SkPathOp, filename);
}

static void bug8380(skiatest::Reporter* reporter, const char* filename) {
SkPath path, path2;
path.setFillType(SkPath::kEvenOdd_FillType);
path.moveTo(SkBits2Float(0xa6800000), SkBits2Float(0x43b0f22d));  // -8.88178e-16f, 353.892f
path.lineTo(SkBits2Float(0x42fc0000), SkBits2Float(0x4116566d));  // 126, 9.3961f
path.cubicTo(SkBits2Float(0x42fb439d), SkBits2Float(0x4114bbc7), SkBits2Float(0x42fa3ed7), SkBits2Float(0x411565bd), SkBits2Float(0x42f934d2), SkBits2Float(0x4116131e));  // 125.632f, 9.29584f, 125.123f, 9.33734f, 124.603f, 9.37967f
path.cubicTo(SkBits2Float(0x42f84915), SkBits2Float(0x4116acc3), SkBits2Float(0x42f75939), SkBits2Float(0x41174918), SkBits2Float(0x42f693f8), SkBits2Float(0x4116566d));  // 124.143f, 9.41718f, 123.674f, 9.45535f, 123.289f, 9.3961f
path.lineTo(SkBits2Float(0x42ec3cee), SkBits2Float(0x410127bb));  // 118.119f, 8.0722f
path.lineTo(SkBits2Float(0x4102c0ec), SkBits2Float(0x42d06d0e));  // 8.1721f, 104.213f
path.lineTo(SkBits2Float(0xa6000000), SkBits2Float(0x4381a63d));  // -4.44089e-16f, 259.299f
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0x43b0f22d));  // 0, 353.892f
path.lineTo(SkBits2Float(0xa6800000), SkBits2Float(0x43b0f22d));  // -8.88178e-16f, 353.892f
path.close();
path2.setFillType(SkPath::kEvenOdd_FillType);
path2.moveTo(SkBits2Float(0x4102c0ec), SkBits2Float(0x42d06d0e));  // 8.1721f, 104.213f
path2.lineTo(SkBits2Float(0xc0ba5a1d), SkBits2Float(0x43b8e831));  // -5.8235f, 369.814f
path2.lineTo(SkBits2Float(0x42fc0000), SkBits2Float(0x411656d6));  // 126, 9.3962f
path2.cubicTo(SkBits2Float(0x42fa9cac), SkBits2Float(0x41134fdf), SkBits2Float(0x42f837cf), SkBits2Float(0x41185aee), SkBits2Float(0x42f693f8), SkBits2Float(0x411656d6));  // 125.306f, 9.207f, 124.109f, 9.5222f, 123.289f, 9.3962f
path2.lineTo(SkBits2Float(0x42ec3cee), SkBits2Float(0x410127bb));  // 118.119f, 8.0722f
path2.lineTo(SkBits2Float(0x4102c0ec), SkBits2Float(0x42d06d0e));  // 8.1721f, 104.213f
path2.close();
    testPathOp(reporter, path, path2, kIntersect_SkPathOp, filename);
}

static void (*skipTest)(skiatest::Reporter* , const char* filename) = 0;
static void (*firstTest)(skiatest::Reporter* , const char* filename) = 0;
static void (*stopTest)(skiatest::Reporter* , const char* filename) = 0;

#define TEST(name) { name, #name }

static struct TestDesc tests[] = {
    TEST(bug8380),
    TEST(crbug_526025),
    TEST(bug8228),
    TEST(op_4),
    TEST(op_1),
    TEST(op_2),
    TEST(op_3),
    TEST(grshapearcs1),
    TEST(filinmangust14),
    TEST(testRect1_u),
    TEST(halbug),
    TEST(seanbug),
    TEST(android1),
    TEST(bug5240),
    TEST(circlesOp4),
    TEST(loop17),
    TEST(cubicOp158),
    TEST(loops_i1),
    TEST(loops_i2),
    TEST(loops_i3),
    TEST(loops_i4),
    TEST(loops_i5),
    TEST(loops_i6),
    TEST(cubics_d3),
    TEST(cubics_o),
    TEST(cubics_d2),
    TEST(cubics_d),
    TEST(dean2),
    TEST(fuzzX_392),
    TEST(fuzz38),
    TEST(cubics44d),
    TEST(cubics45u),
    TEST(loops61i),
    TEST(loops62i),
    TEST(loops63i),
    TEST(loops58iAsQuads),
    TEST(cubics41d),
    TEST(loops59iasQuads),
    TEST(loops59i),
    TEST(loops44i),
    TEST(loops45i),
    TEST(loops46i),
    TEST(loops47i),
    TEST(loops48i),
    TEST(loops49i),
    TEST(loops50i),
    TEST(loops51i),
    TEST(loops52i),
    TEST(loops53i),
    TEST(loops54i),
    TEST(loops55i),
    TEST(loops56i),
    TEST(loops57i),
    TEST(loops58i),
    TEST(loops33iMod),
    TEST(loops33iAsQuads),
    TEST(loops33i),
    TEST(loops40i),
    TEST(loops40iAsQuads),
    TEST(loops39i),
    TEST(loops38i),
    TEST(loops37i),
    TEST(loops36i),
    TEST(loops35i),
    TEST(loops34i),
    TEST(loops32i),
    TEST(loops31i),
    TEST(loops30i),
    TEST(loops29i),
    TEST(loops28i),
    TEST(loops27i),
    TEST(loops26i),
    TEST(loops25i),
    TEST(loops24i),
    TEST(loops23i),
    TEST(loops22i),
    TEST(loops21i),
    TEST(loops20i),
    TEST(cubics20d),
    TEST(cubics6d),
    TEST(cubics7d),
    TEST(cubics8d),
    TEST(cubics9d),
    TEST(cubics10u),
    TEST(cubics11i),
    TEST(cubics12d),
    TEST(cubics13d),
    TEST(cubics14d),
    TEST(cubics15d),
    TEST(cubics16i),
    TEST(cubics17d),
    TEST(cubics18d),
    TEST(cubics19d),
    TEST(cubicOp157),
    TEST(cubicOp142),
    TEST(loops4i),
    TEST(quadRect1),
    TEST(quadRect2),
    TEST(quadRect3),
    TEST(quadRect4),
    TEST(quadRect5),
    TEST(quadRect6),
    TEST(cubicOp141),
    TEST(cubicOp58d),
    TEST(loops5i),
    TEST(cubicOp140),
    TEST(cubicOp139),
    TEST(cubics138),
    TEST(cubics137),
    TEST(cubicOp136a),
    TEST(cubicOp136),
    TEST(cubicOp135),
    TEST(cubicOp134),
    TEST(cubicOp133),
    TEST(loop12),
    TEST(cubicOp132),
    TEST(loop11),
    TEST(loop10),
    TEST(circlesOp3),
    TEST(loop9),
    TEST(loop8),
    TEST(rects5),
    TEST(loop7),
    TEST(cubicOp130a),
    TEST(rRect1x),
    TEST(circlesOp2),
    TEST(circlesOp1),
    TEST(cubicOp131),
    TEST(cubicOp130),
    TEST(cubicOp129),
    TEST(cubicOp128),
    TEST(cubicOp127),
    TEST(cubicOp126),
    TEST(cubicOp125),
    TEST(cubicOp124),
    TEST(loop6),
    TEST(loop5),
    TEST(cubicOp123),
    TEST(cubicOp122),
    TEST(cubicOp121),
    TEST(cubicOp120),
    TEST(cubicOp119),
    TEST(loop4),
    TEST(loop3),
    TEST(loop2),
    TEST(loop1asQuad),
    TEST(loop1),
    TEST(issue3517),
    TEST(cubicOp118),
    TEST(cubicOp117),
    TEST(cubicOp116),
    TEST(testRect2),
    TEST(testRect1),
    TEST(cubicOp115),
    TEST(issue2753),
    TEST(cubicOp114),
    TEST(issue2808),
    TEST(cubicOp114asQuad),
    TEST(rects4),
    TEST(rects3),
    TEST(rects2),
    TEST(rects1),
    TEST(issue2540),
    TEST(issue2504),
    TEST(kari1),
    TEST(quadOp10i),
    TEST(cubicOp113),
    TEST(skpcarrot_is24),
    TEST(issue1417),
    TEST(cubicOp112),
    TEST(skpadspert_net23),
    TEST(skpadspert_de11),
    TEST(findFirst1),
    TEST(xOp2i),
    TEST(xOp3i),
    TEST(xOp1u),
    TEST(xOp1i),
    TEST(cubicOp111),
    TEST(cubicOp110),
    TEST(cubicOp109),
    TEST(cubicOp108),
    TEST(cubicOp107),
    TEST(cubicOp106),
    TEST(cubicOp105),
    TEST(cubicOp104),
    TEST(cubicOp103),
    TEST(cubicOp102),
    TEST(cubicOp101),
    TEST(cubicOp100),
    TEST(cubicOp99),
    TEST(issue1435),
    TEST(cubicOp98x),
    TEST(cubicOp97x),
    TEST(skpcarpetplanet_ru22),
    TEST(cubicOp96d),
    TEST(cubicOp95u),
    TEST(skpadbox_lt15),
    TEST(skpagentxsites_com55),
    TEST(skpadventistmission_org572),
    TEST(skpadoption_org196),
    TEST(skpbambootheme_com12),
    TEST(skpbakosoft_com10),
    TEST(skpakmmos_ru100),
    TEST(skpbangalorenest_com4),
    TEST(skpbingoentertainment_net189),
    TEST(skpbestred_ru37),
    TEST(skpbenzoteh_ru152),
    TEST(skpcamcorder_kz21),
    TEST(skpcaffelavazzait_com_ua21),
    TEST(skpcarrefour_ro62),
    TEST(skpcavablar_net563),
    TEST(skpinsomnia_gr72),
    TEST(skpadbox_lt8),
    TEST(skpact_com43),
    TEST(skpacesoftech_com47),
    TEST(skpabcspark_ca103),
    TEST(cubicOp94u),
    TEST(cubicOp93d),
    TEST(cubicOp92i),
    TEST(skpadithya_putr4_blogspot_com551),
    TEST(skpadindex_de4),
    TEST(skpaiaigames_com870),
    TEST(skpaaalgarve_org53),
    TEST(skpkkiste_to716),
    TEST(cubicOp91u),
    TEST(cubicOp90u),
    TEST(cubicOp89u),
    TEST(cubicOp88u),
    TEST(cubicOp87u),
    TEST(cubicOp86i),
    TEST(loopEdge2),
    TEST(loopEdge1),
    TEST(rectOp3x),
    TEST(rectOp2i),
    TEST(rectOp1i),
    TEST(issue1418b),
    TEST(cubicOp85i),
    TEST(issue1418),
    TEST(skpkkiste_to98),
    TEST(skpahrefs_com29),
    TEST(cubicOp85d),
    TEST(skpahrefs_com88),
    TEST(skphealth_com76),
    TEST(skpancestry_com1),
    TEST(skpbyte_com1),
    TEST(skpeldorado_com_ua1),
    TEST(skp96prezzi1),
    TEST(skpClip2),
    TEST(skpClip1),
    TEST(cubicOp84d),
    TEST(cubicOp83i),
    TEST(cubicOp82i),
    TEST(cubicOp81d),
    TEST(cubicOp80i),
    TEST(cubicOp79u),
    TEST(cubicOp78u),
    TEST(cubicOp77i),
    TEST(cubicOp76u),
    TEST(cubicOp75d),
    TEST(cubicOp74d),
    TEST(cubicOp73d),
    TEST(cubicOp72i),
    TEST(cubicOp71d),
    TEST(skp5),
    TEST(skp4),
    TEST(skp3),
    TEST(skp2),
    TEST(skp1),
    TEST(rRect1),
    TEST(cubicOp70d),
    TEST(cubicOp69d),
    TEST(cubicOp68u),
    TEST(cubicOp67u),
    TEST(cubicOp66u),
    TEST(rectOp1d),
    TEST(cubicOp65d),
    TEST(cubicOp64d),
    TEST(cubicOp63d),
    TEST(cubicOp62d),
    TEST(cubicOp61d),
    TEST(cubicOp60d),
    TEST(cubicOp59d),
    TEST(cubicOp57d),
    TEST(cubicOp56d),
    TEST(cubicOp55d),
    TEST(cubicOp54d),
    TEST(cubicOp53d),
    TEST(cubicOp52d),
    TEST(cubicOp51d),
    TEST(cubicOp50d),
    TEST(cubicOp49d),
    TEST(cubicOp48d),
    TEST(cubicOp47d),
    TEST(cubicOp46d),
    TEST(cubicOp45d),
    TEST(cubicOp44d),
    TEST(cubicOp43d),
    TEST(cubicOp42d),
    TEST(cubicOp41i),
    TEST(cubicOp40d),
    TEST(cubicOp39d),
    TEST(cubicOp38d),
    TEST(cubicOp37d),
    TEST(cubicOp36u),
    TEST(cubicOp35d),
    TEST(cubicOp34d),
    TEST(cubicOp33i),
    TEST(cubicOp32d),
    TEST(cubicOp31d),
    TEST(cubicOp31x),
    TEST(cubicOp31u),
    TEST(cubicOp30d),
    TEST(cubicOp29d),
    TEST(cubicOp28u),
    TEST(cubicOp27d),
    TEST(cubicOp26d),
    TEST(cubicOp25i),
    TEST(testOp8d),
    TEST(testDiff1),
    TEST(testIntersect1),
    TEST(testUnion1),
    TEST(testXor1),
    TEST(testDiff2),
    TEST(testIntersect2),
    TEST(testUnion2),
    TEST(testXor2),
    TEST(testOp1d),
    TEST(testOp2d),
    TEST(testOp3d),
    TEST(testOp1u),
    TEST(testOp4d),
    TEST(testOp5d),
    TEST(testOp6d),
    TEST(testOp7d),
    TEST(testOp2u),

    TEST(cubicOp24d),
    TEST(cubicOp23d),
    TEST(cubicOp22d),
    TEST(cubicOp21d),
    TEST(cubicOp20d),
    TEST(cubicOp19i),
    TEST(cubicOp18d),
    TEST(cubicOp17d),
    TEST(cubicOp16d),
    TEST(cubicOp15d),
    TEST(cubicOp14d),
    TEST(cubicOp13d),
    TEST(cubicOp12d),
    TEST(cubicOp11d),
    TEST(cubicOp10d),
    TEST(cubicOp1i),
    TEST(cubicOp9d),
    TEST(quadOp9d),
    TEST(lineOp9d),
    TEST(cubicOp8d),
    TEST(cubicOp7d),
    TEST(cubicOp6d),
    TEST(cubicOp5d),
    TEST(cubicOp3d),
    TEST(cubicOp2d),
    TEST(cubicOp1d),
};

static const size_t testCount = SK_ARRAY_COUNT(tests);

static struct TestDesc subTests[] = {
    TEST(loops47i),
    TEST(loops61i),
    TEST(loops62i),
    TEST(issue3517),
};

static const size_t subTestCount = SK_ARRAY_COUNT(subTests);

static void (*firstSubTest)(skiatest::Reporter* , const char* filename) = nullptr;

static bool runSubTests = false;
static bool runSubTestsFirst = true;
static bool runReverse = false;

DEF_TEST(PathOpsOp, reporter) {
#if DEBUG_SHOW_TEST_NAME
    strncpy(DEBUG_FILENAME_STRING, "", DEBUG_FILENAME_STRING_LENGTH);
#endif
    if (runSubTests && runSubTestsFirst) {
        RunTestSet(reporter, subTests, subTestCount, firstSubTest, nullptr, stopTest, runReverse);
    }
    RunTestSet(reporter, tests, testCount, firstTest, skipTest, stopTest, runReverse);
    if (runSubTests && !runSubTestsFirst) {
        RunTestSet(reporter, subTests, subTestCount, firstSubTest, nullptr, stopTest, runReverse);
    }
}

static void fuzz767834(skiatest::Reporter* reporter, const char* filename) {
    SkPath one;
    SkPath two;
one.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0

one.conicTo(SkBits2Float(0x02807252), SkBits2Float(0xee23000a), SkBits2Float(0x00000000), SkBits2Float(0x0fe00008), SkBits2Float(0x52526831));  // 1.88735e-37f, -1.26115e+28f, 0, 2.20881e-29f, 2.25923e+11f

one.cubicTo(SkBits2Float(0x474d475a), SkBits2Float(0x72727252), SkBits2Float(0x72267272), SkBits2Float(0x535202ff), SkBits2Float(0x53535353), SkBits2Float(0x58943353));  // 52551.4f, 4.80215e+30f, 3.29682e+30f, 9.01993e+11f, 9.07636e+11f, 1.30359e+15f

one.quadTo(SkBits2Float(0x52727272), SkBits2Float(0x52595252), SkBits2Float(0x8e460900), SkBits2Float(0x7272db72));  // 2.60326e+11f, 2.33347e+11f, -2.44097e-30f, 4.81028e+30f

one.lineTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0

one.close();

one.moveTo(SkBits2Float(0x72000400), SkBits2Float(0x72727272));  // 2.53561e+30f, 4.80216e+30f

one.quadTo(SkBits2Float(0x60727272), SkBits2Float(0x72727272), SkBits2Float(0x2a527272), SkBits2Float(0x72525252));  // 6.98806e+19f, 4.80216e+30f, 1.86915e-13f, 4.16585e+30f

one.cubicTo(SkBits2Float(0x72727251), SkBits2Float(0x52617272), SkBits2Float(0x46032352), SkBits2Float(0x7272728e), SkBits2Float(0x5c527272), SkBits2Float(0x72726552));  // 4.80215e+30f, 2.42072e+11f, 8392.83f, 4.80217e+30f, 2.36942e+17f, 4.80114e+30f

one.cubicTo(SkBits2Float(0x2b7280ff), SkBits2Float(0x7240ffff), SkBits2Float(0x72724960), SkBits2Float(0x52008072), SkBits2Float(0x72725230), SkBits2Float(0x5f727272));  // 8.61547e-13f, 3.82276e+30f, 4.79898e+30f, 1.37978e+11f, 4.79966e+30f, 1.74702e+19f

one.lineTo(SkBits2Float(0x72000400), SkBits2Float(0x72727272));  // 2.53561e+30f, 4.80216e+30f

one.close();

one.moveTo(SkBits2Float(0x8e524603), SkBits2Float(0x72727272));  // -2.59182e-30f, 4.80216e+30f

one.close();

one.moveTo(SkBits2Float(0x8e524603), SkBits2Float(0x72727272));  // -2.59182e-30f, 4.80216e+30f

one.quadTo(SkBits2Float(0x72725d72), SkBits2Float(0x52008072), SkBits2Float(0x00016552), SkBits2Float(0x72724000));  // 4.80053e+30f, 1.37978e+11f, 1.28182e-40f, 4.79826e+30f

one.quadTo(SkBits2Float(0x00807272), SkBits2Float(0x392a5b25), SkBits2Float(0x72685768), SkBits2Float(0x000000ff));  // 1.1796e-38f, 0.000162464f, 4.602e+30f, 3.57331e-43f

one.moveTo(SkBits2Float(0xe0e060e0), SkBits2Float(0x728f5740));  // -1.29345e+20f, 5.67831e+30f

one.quadTo(SkBits2Float(0x72727272), SkBits2Float(0xd2008072), SkBits2Float(0x8e460900), SkBits2Float(0x72727072));  // 4.80216e+30f, -1.37978e+11f, -2.44097e-30f, 4.802e+30f

one.cubicTo(SkBits2Float(0xe0e060e0), SkBits2Float(0x58943303), SkBits2Float(0x72727272), SkBits2Float(0x59525252), SkBits2Float(0x00090052), SkBits2Float(0x72000000));  // -1.29345e+20f, 1.30357e+15f, 4.80216e+30f, 3.70002e+15f, 8.26634e-40f, 2.5353e+30f

one.quadTo(SkBits2Float(0x005252ec), SkBits2Float(0x72000400), SkBits2Float(0x72727272), SkBits2Float(0x72727272));  // 7.56026e-39f, 2.53561e+30f, 4.80216e+30f, 4.80216e+30f

one.lineTo(SkBits2Float(0xe0e060e0), SkBits2Float(0x728f5740));  // -1.29345e+20f, 5.67831e+30f

one.close();

one.moveTo(SkBits2Float(0xe0e060e0), SkBits2Float(0x728f5740));  // -1.29345e+20f, 5.67831e+30f

one.quadTo(SkBits2Float(0x72727272), SkBits2Float(0x522a5272), SkBits2Float(0x20725252), SkBits2Float(0x72727251));  // 4.80216e+30f, 1.82882e+11f, 2.05254e-19f, 4.80215e+30f

one.quadTo(SkBits2Float(0x72727272), SkBits2Float(0x59525252), SkBits2Float(0x46090052), SkBits2Float(0x72db728e));  // 4.80216e+30f, 3.70002e+15f, 8768.08f, 8.69321e+30f

one.quadTo(SkBits2Float(0x005252ec), SkBits2Float(0x72000400), SkBits2Float(0x72727272), SkBits2Float(0x72727272));  // 7.56026e-39f, 2.53561e+30f, 4.80216e+30f, 4.80216e+30f

one.lineTo(SkBits2Float(0xe0e060e0), SkBits2Float(0x728f5740));  // -1.29345e+20f, 5.67831e+30f

one.close();

one.moveTo(SkBits2Float(0xe0e060e0), SkBits2Float(0x728f5740));  // -1.29345e+20f, 5.67831e+30f

one.quadTo(SkBits2Float(0x72727272), SkBits2Float(0x522a5272), SkBits2Float(0x20725252), SkBits2Float(0x72727251));  // 4.80216e+30f, 1.82882e+11f, 2.05254e-19f, 4.80215e+30f

one.quadTo(SkBits2Float(0x52526172), SkBits2Float(0x8e460323), SkBits2Float(0x72727272), SkBits2Float(0x525c5272));  // 2.25894e+11f, -2.44069e-30f, 4.80216e+30f, 2.36569e+11f

one.conicTo(SkBits2Float(0xff727272), SkBits2Float(0xff2b549b), SkBits2Float(0x607240ff), SkBits2Float(0x72727249), SkBits2Float(0x30520080));  // -3.22267e+38f, -2.27737e+38f, 6.98249e+19f, 4.80215e+30f, 7.63983e-10f

one.lineTo(SkBits2Float(0xe0e060e0), SkBits2Float(0x728f5740));  // -1.29345e+20f, 5.67831e+30f

one.close();

one.moveTo(SkBits2Float(0xe0e060e0), SkBits2Float(0x728f5740));  // -1.29345e+20f, 5.67831e+30f

one.quadTo(SkBits2Float(0x72727272), SkBits2Float(0x0052525f), SkBits2Float(0x8e524603), SkBits2Float(0x72727272));  // 4.80216e+30f, 7.56006e-39f, -2.59182e-30f, 4.80216e+30f

one.lineTo(SkBits2Float(0xe0e060e0), SkBits2Float(0x728f5740));  // -1.29345e+20f, 5.67831e+30f

one.close();

one.moveTo(SkBits2Float(0xe0e060e0), SkBits2Float(0x728f5740));  // -1.29345e+20f, 5.67831e+30f

one.quadTo(SkBits2Float(0x72725d72), SkBits2Float(0x52008072), SkBits2Float(0x00016552), SkBits2Float(0x72724000));  // 4.80053e+30f, 1.37978e+11f, 1.28182e-40f, 4.79826e+30f

one.quadTo(SkBits2Float(0x00807272), SkBits2Float(0x392a5b25), SkBits2Float(0x72685768), SkBits2Float(0x000000ff));  // 1.1796e-38f, 0.000162464f, 4.602e+30f, 3.57331e-43f

one.moveTo(SkBits2Float(0xe0e060e0), SkBits2Float(0x728f5740));  // -1.29345e+20f, 5.67831e+30f

one.quadTo(SkBits2Float(0x72727272), SkBits2Float(0xd2008072), SkBits2Float(0x8e460900), SkBits2Float(0x72727072));  // 4.80216e+30f, -1.37978e+11f, -2.44097e-30f, 4.802e+30f

one.cubicTo(SkBits2Float(0xe0e060e0), SkBits2Float(0x58943303), SkBits2Float(0x72727272), SkBits2Float(0x59525252), SkBits2Float(0x46090052), SkBits2Float(0x72db728e));  // -1.29345e+20f, 1.30357e+15f, 4.80216e+30f, 3.70002e+15f, 8768.08f, 8.69321e+30f

one.quadTo(SkBits2Float(0x005252ec), SkBits2Float(0x72000400), SkBits2Float(0x72727272), SkBits2Float(0x72727272));  // 7.56026e-39f, 2.53561e+30f, 4.80216e+30f, 4.80216e+30f

one.lineTo(SkBits2Float(0xe0e060e0), SkBits2Float(0x728f5740));  // -1.29345e+20f, 5.67831e+30f

one.close();

one.moveTo(SkBits2Float(0xe0e060e0), SkBits2Float(0x728f5740));  // -1.29345e+20f, 5.67831e+30f

one.quadTo(SkBits2Float(0x72727272), SkBits2Float(0x522a5272), SkBits2Float(0x20725252), SkBits2Float(0x72727251));  // 4.80216e+30f, 1.82882e+11f, 2.05254e-19f, 4.80215e+30f

    testPathOpFuzz(reporter, two, one, kIntersect_SkPathOp, filename);
}

static void fuzz535151(skiatest::Reporter* reporter, const char* filename) {
    SkPath one;
    one.setFillType(SkPath::kWinding_FillType);
    SkPath two;
    two.setFillType(SkPath::kWinding_FillType);
    two.moveTo(0, 0);
    two.lineTo(0, 50);
    two.lineTo(4.29497e+09f, 50);
    SkPath dummy;
    testPathOpFuzz(reporter, one, two, kIntersect_SkPathOp, filename);
}

static void bufferOverflow(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0,0, 300,170141183460469231731687303715884105728.f);
    SkPath pathB;
    pathB.addRect(0,0, 300,16);
    testPathOpFuzz(reporter, path, pathB, kUnion_SkPathOp, filename);
}

// m 100,0 60,170 -160,-110 200,0 -170,11000000000 z
static void fuzz433(skiatest::Reporter* reporter, const char* filename) {
    SkPath path1, path2;
    path1.moveTo(100,0);
    path1.lineTo(60,170);
    path1.lineTo(-160,-110);
    path1.lineTo(200,0);
    path1.lineTo(-170,11000000000.0f);
    path1.close();

    path2.moveTo(100 + 20,0 + 20);
    path2.lineTo(60 + 20,170 + 20);
    path2.lineTo(-160 + 20,-110 + 20);
    path2.lineTo(200 + 20,0 + 20);
    path2.lineTo(-170 + 20,11000000000.0f + 20);
    path2.close();

    testPathOpFuzz(reporter, path1, path2, kIntersect_SkPathOp, filename);
}

static void fuzz433b(skiatest::Reporter* reporter, const char* filename) {
    SkPath path1, path2;
    path1.setFillType(SkPath::kEvenOdd_FillType);
    path1.moveTo(140, 40);
    path1.lineTo(200, 210);
    path1.lineTo(40, 100);
    path1.lineTo(240, 100);
    path1.lineTo(70, 1.1e+10f);
    path1.lineTo(140, 40);
    path1.close();

    path1.setFillType(SkPath::kWinding_FillType);
    path2.moveTo(190, 60);
    path2.lineTo(250, 230);
    path2.lineTo(90, 120);
    path2.lineTo(290, 120);
    path2.lineTo(120, 1.1e+10f);
    path2.lineTo(190, 60);
    path2.close();

    testPathOpFuzz(reporter, path1, path2, kUnion_SkPathOp, filename);
}

static void fuzz487a(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x432c8000), SkBits2Float(0x42c00000));
path.lineTo(SkBits2Float(0x4309999a), SkBits2Float(0x42c00000));
path.cubicTo(SkBits2Float(0x4309999a), SkBits2Float(0x429a6666), SkBits2Float(0x42f9999a), SkBits2Float(0x4275999a), SkBits2Float(0x42d70001), SkBits2Float(0x42633333));
path.lineTo(SkBits2Float(0x42e90001), SkBits2Float(0x41b8cccc));
path.cubicTo(SkBits2Float(0x42dc6667), SkBits2Float(0x41ab3332), SkBits2Float(0x42cf3334), SkBits2Float(0x41a3ffff), SkBits2Float(0x42c20001), SkBits2Float(0x41a3ffff));
path.lineTo(SkBits2Float(0x42c20001), SkBits2Float(0x425d999a));
path.lineTo(SkBits2Float(0x42c20001), SkBits2Float(0x425d999a));
path.cubicTo(SkBits2Float(0x429c6668), SkBits2Float(0x425d999a), SkBits2Float(0x4279999c), SkBits2Float(0x42886667), SkBits2Float(0x42673335), SkBits2Float(0x42ab0000));
path.lineTo(SkBits2Float(0x41c0ccd0), SkBits2Float(0x42990000));
path.cubicTo(SkBits2Float(0x41b33336), SkBits2Float(0x42a5999a), SkBits2Float(0x41ac0003), SkBits2Float(0x42b2cccd), SkBits2Float(0x41ac0003), SkBits2Float(0x42c00000));
path.lineTo(SkBits2Float(0x4261999c), SkBits2Float(0x42c00000));
path.lineTo(SkBits2Float(0x4261999c), SkBits2Float(0x42c00000));
path.cubicTo(SkBits2Float(0x4261999c), SkBits2Float(0x434d3333), SkBits2Float(0x4364e667), SkBits2Float(0x4346b333), SkBits2Float(0x4364e667), SkBits2Float(0x43400000));
path.lineTo(SkBits2Float(0x432c8000), SkBits2Float(0x42c00000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x432c8000), SkBits2Float(0x42c00000));
path.lineTo(SkBits2Float(0x4309999a), SkBits2Float(0x42c00000));
path.cubicTo(SkBits2Float(0x4309999a), SkBits2Float(0x42a20000), SkBits2Float(0x43016667), SkBits2Float(0x4287cccd), SkBits2Float(0x42ea999a), SkBits2Float(0x4273999a));
path.lineTo(SkBits2Float(0x4306cccd), SkBits2Float(0x41f5999a));
path.cubicTo(SkBits2Float(0x42f76667), SkBits2Float(0x41c26667), SkBits2Float(0x42dd999a), SkBits2Float(0x41a4cccd), SkBits2Float(0x42c23334), SkBits2Float(0x41a4cccd));
path.lineTo(SkBits2Float(0x42c23334), SkBits2Float(0x425e0000));
path.cubicTo(SkBits2Float(0x42a43334), SkBits2Float(0x425e0000), SkBits2Float(0x428a0001), SkBits2Float(0x427ecccd), SkBits2Float(0x42780002), SkBits2Float(0x4297999a));
path.lineTo(SkBits2Float(0x41fccccd), SkBits2Float(0x42693333));
path.cubicTo(SkBits2Float(0x41c9999a), SkBits2Float(0x428acccd), SkBits2Float(0x41ac0000), SkBits2Float(0x42a4999a), SkBits2Float(0x41ac0000), SkBits2Float(0x42c00000));
path.lineTo(SkBits2Float(0x4261999a), SkBits2Float(0x42c00000));
path.cubicTo(SkBits2Float(0x4261999a), SkBits2Float(0x42de0000), SkBits2Float(0x42813333), SkBits2Float(0x42f83333), SkBits2Float(0x42996666), SkBits2Float(0x4303199a));
path.cubicTo(SkBits2Float(0x4272cccc), SkBits2Float(0x4303199a), SkBits2Float(0x423d3332), SkBits2Float(0x430de667), SkBits2Float(0x422d9999), SkBits2Float(0x431cb334));
path.lineTo(SkBits2Float(0x7086a1dc), SkBits2Float(0x42eecccd));
path.lineTo(SkBits2Float(0x41eb3333), SkBits2Float(0xc12ccccd));
path.lineTo(SkBits2Float(0x42053333), SkBits2Float(0xc1cccccd));
path.lineTo(SkBits2Float(0x42780000), SkBits2Float(0xc18f3334));
path.cubicTo(SkBits2Float(0x43206666), SkBits2Float(0x43134ccd), SkBits2Float(0x43213333), SkBits2Float(0x430db333), SkBits2Float(0x43213333), SkBits2Float(0x43080000));
path.lineTo(SkBits2Float(0x432c8000), SkBits2Float(0x42c00000));
path.close();

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 2, filename);
}

static void fuzz487b(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x432c8000), SkBits2Float(0x42c00000));
path.lineTo(SkBits2Float(0x4309999a), SkBits2Float(0x42c00000));
path.cubicTo(SkBits2Float(0x4309999a), SkBits2Float(0x429a6666), SkBits2Float(0x42f9999a), SkBits2Float(0x4275999a), SkBits2Float(0x42d70001), SkBits2Float(0x42633333));
path.lineTo(SkBits2Float(0x42e90001), SkBits2Float(0x41b8cccc));
path.cubicTo(SkBits2Float(0x42dc6667), SkBits2Float(0x41ab3332), SkBits2Float(0x42cf3334), SkBits2Float(0x41a3ffff), SkBits2Float(0x42c20001), SkBits2Float(0x41a3ffff));
path.lineTo(SkBits2Float(0x42c20001), SkBits2Float(0x425d999a));
path.lineTo(SkBits2Float(0x42c20001), SkBits2Float(0x425d999a));
path.cubicTo(SkBits2Float(0x429c6668), SkBits2Float(0x425d999a), SkBits2Float(0x4279999c), SkBits2Float(0x42886667), SkBits2Float(0x42673335), SkBits2Float(0x42ab0000));
path.lineTo(SkBits2Float(0x41c0ccd0), SkBits2Float(0x42990000));
path.cubicTo(SkBits2Float(0x41b33336), SkBits2Float(0x42a5999a), SkBits2Float(0x41ac0003), SkBits2Float(0x42b2cccd), SkBits2Float(0x41ac0003), SkBits2Float(0x42c00000));
path.lineTo(SkBits2Float(0x4261999c), SkBits2Float(0x42c00000));
path.lineTo(SkBits2Float(0x4261999c), SkBits2Float(0x42c00000));
path.cubicTo(SkBits2Float(0x4261999c), SkBits2Float(0x434d3333), SkBits2Float(0x4364e667), SkBits2Float(0x4346b333), SkBits2Float(0x4364e667), SkBits2Float(0x43400000));
path.lineTo(SkBits2Float(0x432c8000), SkBits2Float(0x42c00000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x432c8000), SkBits2Float(0x42c00000));
path.lineTo(SkBits2Float(0x4309999a), SkBits2Float(0x42c00000));
path.cubicTo(SkBits2Float(0x4309999a), SkBits2Float(0x42a20000), SkBits2Float(0x43016667), SkBits2Float(0x4287cccd), SkBits2Float(0x42ea999a), SkBits2Float(0x4273999a));
path.lineTo(SkBits2Float(0x4306cccd), SkBits2Float(0x41f5999a));
path.cubicTo(SkBits2Float(0x42f76667), SkBits2Float(0x41c26667), SkBits2Float(0x42dd999a), SkBits2Float(0x41a4cccd), SkBits2Float(0x42c23334), SkBits2Float(0x41a4cccd));
path.lineTo(SkBits2Float(0x42c23334), SkBits2Float(0x425e0000));
path.cubicTo(SkBits2Float(0x42a43334), SkBits2Float(0x425e0000), SkBits2Float(0x428a0001), SkBits2Float(0x427ecccd), SkBits2Float(0x42780002), SkBits2Float(0x4297999a));
path.lineTo(SkBits2Float(0x41fccccd), SkBits2Float(0x42693333));
path.cubicTo(SkBits2Float(0x41c9999a), SkBits2Float(0x428acccd), SkBits2Float(0x41ac0000), SkBits2Float(0x42a4999a), SkBits2Float(0x41ac0000), SkBits2Float(0x42c00000));
path.lineTo(SkBits2Float(0x4261999a), SkBits2Float(0x42c00000));
path.cubicTo(SkBits2Float(0x4261999a), SkBits2Float(0x42de0000), SkBits2Float(0x42813333), SkBits2Float(0x42f83333), SkBits2Float(0x42996666), SkBits2Float(0x4303199a));
path.cubicTo(SkBits2Float(0x4272cccc), SkBits2Float(0x4303199a), SkBits2Float(0x423d3332), SkBits2Float(0x430de667), SkBits2Float(0x422d9999), SkBits2Float(0x431cb334));
path.lineTo(SkBits2Float(0x7086a1dc), SkBits2Float(0x42eecccd));
path.lineTo(SkBits2Float(0x41eb3333), SkBits2Float(0xc12ccccd));
path.lineTo(SkBits2Float(0x42053333), SkBits2Float(0xc1cccccd));
path.lineTo(SkBits2Float(0x42780000), SkBits2Float(0xc18f3334));
path.cubicTo(SkBits2Float(0x43206666), SkBits2Float(0x43134ccd), SkBits2Float(0x43213333), SkBits2Float(0x430db333), SkBits2Float(0x43213333), SkBits2Float(0x43080000));
path.lineTo(SkBits2Float(0x432c8000), SkBits2Float(0x42c00000));
path.close();

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 2, filename);
}

static void fuzz714(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x430c0000), SkBits2Float(0x42200000));
path.lineTo(SkBits2Float(0x43480000), SkBits2Float(0x43520000));
path.lineTo(SkBits2Float(0x42200000), SkBits2Float(0x42c80000));
path.lineTo(SkBits2Float(0x64969569), SkBits2Float(0x42c80000));  // 2.22222e+022f
path.lineTo(SkBits2Float(0x64969569), SkBits2Float(0x43520000));  // 2.22222e+022f
path.lineTo(SkBits2Float(0x430c0000), SkBits2Float(0x42200000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43200000), SkBits2Float(0x42700000));
path.lineTo(SkBits2Float(0x435c0000), SkBits2Float(0x43660000));
path.lineTo(SkBits2Float(0x42700000), SkBits2Float(0x42f00000));
path.lineTo(SkBits2Float(0x64969569), SkBits2Float(0x42f00000));  // 2.22222e+022f
path.lineTo(SkBits2Float(0x64969569), SkBits2Float(0x43660000));  // 2.22222e+022f
path.lineTo(SkBits2Float(0x43200000), SkBits2Float(0x42700000));
path.close();

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 2, filename);
}

static void fuzz1(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x7f800000), SkBits2Float(0x7f800000));
path.quadTo(SkBits2Float(0x7f800000), SkBits2Float(0x7f800000), SkBits2Float(0x7f800000), SkBits2Float(0x7f800000));
path.quadTo(SkBits2Float(0x7f800000), SkBits2Float(0x7f800000), SkBits2Float(0x7f800000), SkBits2Float(0x7f800000));
path.quadTo(SkBits2Float(0xffc00000), SkBits2Float(0x7f800000), SkBits2Float(0xffc00000), SkBits2Float(0x7f800000));
path.quadTo(SkBits2Float(0xff000001), SkBits2Float(0x7f800000), SkBits2Float(0xff000001), SkBits2Float(0x7f800000));
path.quadTo(SkBits2Float(0xff000001), SkBits2Float(0xffc00000), SkBits2Float(0xffc00000), SkBits2Float(0xffc00000));
path.quadTo(SkBits2Float(0xffc00000), SkBits2Float(0xff000001), SkBits2Float(0x7f800000), SkBits2Float(0xff000001));
path.quadTo(SkBits2Float(0x7f800000), SkBits2Float(0xff000001), SkBits2Float(0x7f800000), SkBits2Float(0xffc00000));
path.quadTo(SkBits2Float(0x7f800000), SkBits2Float(0xffc00000), SkBits2Float(0x7f800000), SkBits2Float(0x7f800000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 2, filename);
}


static void fuzz753_91(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42910000), SkBits2Float(0x00000000));  // 72.5f, 0
path.lineTo(SkBits2Float(0x42166668), SkBits2Float(0x00000000));  // 37.6f, 0
path.cubicTo(SkBits2Float(0x42166668), SkBits2Float(0xc1966668), SkBits2Float(0x41c66668), SkBits2Float(0xc20a6666), SkBits2Float(0x40f00010), SkBits2Float(0xc21ccccd));  // 37.6f, -18.8f, 24.8f, -34.6f, 7.50001f, -39.2f
path.lineTo(SkBits2Float(0x41840004), SkBits2Float(0xc291cccd));  // 16.5f, -72.9f
path.lineTo(SkBits2Float(0x42fb6668), SkBits2Float(0x42c73334));  // 125.7f, 99.6f
path.lineTo(SkBits2Float(0x43646668), SkBits2Float(0x43880ccd));  // 228.4f, 272.1f

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x428bf702), SkBits2Float(0xcf223cbf));  // 69.9824f, -2.72189e+09f
path.lineTo(SkBits2Float(0x42112d68), SkBits2Float(0xcf223cbf));  // 36.2943f, -2.72189e+09f
path.cubicTo(SkBits2Float(0x4220d9fc), SkBits2Float(0xcf223cc0), SkBits2Float(0x420ee118), SkBits2Float(0xcf223cc0), SkBits2Float(0x41cef2f8), SkBits2Float(0xcf223cc0));  // 40.2129f, -2.72189e+09f, 35.7198f, -2.72189e+09f, 25.8686f, -2.72189e+09f
path.lineTo(SkBits2Float(0x424a99e0), SkBits2Float(0xcf223cc0));  // 50.6503f, -2.72189e+09f
path.cubicTo(SkBits2Float(0x42266e32), SkBits2Float(0xcf223cc0), SkBits2Float(0x41f0fa20), SkBits2Float(0xcf223cc0), SkBits2Float(0x41872ed4), SkBits2Float(0xcf223cc0));  // 41.6076f, -2.72189e+09f, 30.1221f, -2.72189e+09f, 16.8979f, -2.72189e+09f
path.lineTo(SkBits2Float(0x40f8fbe0), SkBits2Float(0xcf223cc0));  // 7.78075f, -2.72189e+09f

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 2, filename);
}

static void bug597926_0(skiatest::Reporter* reporter, const char* filename) {
SkPath path;
path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43b38000), SkBits2Float(0x433e0000));  // 359, 190
path.lineTo(SkBits2Float(0x40c00000), SkBits2Float(0x449ce000));  // 6, 1255
path.cubicTo(SkBits2Float(0x438c0000), SkBits2Float(0x4497a000), SkBits2Float(0x43e40000), SkBits2Float(0x44750000), SkBits2Float(0x41000000), SkBits2Float(0x44aa2000));  // 280, 1213, 456, 980, 8, 1361
path.moveTo(SkBits2Float(0x43290000), SkBits2Float(0x4431c000));  // 169, 711
path.lineTo(SkBits2Float(0xd987d6ba), SkBits2Float(0xd93d0ad4));  // -4.7794e+15f, -3.32567e+15f
path.conicTo(SkBits2Float(0x43cc8000), SkBits2Float(0x445b8000), SkBits2Float(0xd888b096), SkBits2Float(0xd9a1ebfa), SkBits2Float(0x3ebcb199));  // 409, 878, -1.20234e+15f, -5.69712e+15f, 0.368542f
path.cubicTo(SkBits2Float(0x43c00000), SkBits2Float(0x443a8000), SkBits2Float(0x42380000), SkBits2Float(0x4421c000), SkBits2Float(0x42500000), SkBits2Float(0x448ca000));  // 384, 746, 46, 647, 52, 1125
path.quadTo(SkBits2Float(0x43948000), SkBits2Float(0x42ac0000), SkBits2Float(0x43880000), SkBits2Float(0x4487e000));  // 297, 86, 272, 1087
SkPath path1(path);
path.reset();
path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0xc51d735c), SkBits2Float(0xc49db029));  // -2519.21f, -1261.51f
path.cubicTo(SkBits2Float(0xc51d1dbd), SkBits2Float(0xc49d7a3f), SkBits2Float(0xc51c524a), SkBits2Float(0xc49d1610), SkBits2Float(0xc51d1a96), SkBits2Float(0xc49d86a6));  // -2513.86f, -1259.82f, -2501.14f, -1256.69f, -2513.66f, -1260.21f
path.cubicTo(SkBits2Float(0xc51cd471), SkBits2Float(0xc49d54d0), SkBits2Float(0xc51c2e51), SkBits2Float(0xc49d0081), SkBits2Float(0xc51d197b), SkBits2Float(0xc49d7927));  // -2509.28f, -1258.65f, -2498.89f, -1256.02f, -2513.59f, -1259.79f
path.quadTo(SkBits2Float(0xc51bf7eb), SkBits2Float(0xc49cf010), SkBits2Float(0xc51ba866), SkBits2Float(0xc49cb9e6));  // -2495.49f, -1255.5f, -2490.52f, -1253.81f
path.cubicTo(SkBits2Float(0xc51bac0d), SkBits2Float(0xc49cc50e), SkBits2Float(0xc51c29eb), SkBits2Float(0xc49cfb01), SkBits2Float(0xc51c5bca), SkBits2Float(0xc49d1fa6));  // -2490.75f, -1254.16f, -2498.62f, -1255.84f, -2501.74f, -1256.99f
SkPath path2(path);
testPathOpFuzz(reporter, path1, path2, (SkPathOp) 1, filename);
}

static void fuzz1450_0(skiatest::Reporter* reporter, const char* filename) {
SkPath path;
path.moveTo(SkBits2Float(0x43b40000), SkBits2Float(0xcf000000));  // 360, -2.14748e+09f
path.conicTo(SkBits2Float(0x4e800002), SkBits2Float(0xcf000000), SkBits2Float(0x4e800002), SkBits2Float(0xce7ffffe), SkBits2Float(0x3f3504f4));  // 1.07374e+09f, -2.14748e+09f, 1.07374e+09f, -1.07374e+09f, 0.707107f
path.conicTo(SkBits2Float(0x4e800002), SkBits2Float(0x43800001), SkBits2Float(0x43348000), SkBits2Float(0x43800001), SkBits2Float(0x3f3504f4));  // 1.07374e+09f, 256, 180.5f, 256, 0.707107f
SkPath path1(path);
path.reset();
path.moveTo(SkBits2Float(0x43b40000), SkBits2Float(0x45816000));  // 360, 4140
path.conicTo(SkBits2Float(0x43b40005), SkBits2Float(0x458a945d), SkBits2Float(0x45610000), SkBits2Float(0x458a945d), SkBits2Float(0x3f3504f3));  // 360, 4434.55f, 3600, 4434.55f, 0.707107f
path.conicTo(SkBits2Float(0x45d5bfff), SkBits2Float(0x458a945d), SkBits2Float(0x45d5bfff), SkBits2Float(0x45816000), SkBits2Float(0x3f3504f3));  // 6840, 4434.55f, 6840, 4140, 0.707107f
path.lineTo(SkBits2Float(0x42c80000), SkBits2Float(0x44000000));  // 100, 512
path.lineTo(SkBits2Float(0x42000000), SkBits2Float(0x41800000));  // 32, 16
path.lineTo(SkBits2Float(0x43b40000), SkBits2Float(0x44800000));  // 360, 1024
path.lineTo(SkBits2Float(0x43b40000), SkBits2Float(0x45816000));  // 360, 4140
path.close();
SkPath path2(path);
testPathOpFuzz(reporter, path1, path2, kUnion_SkPathOp, filename);
}

static void fuzz1450_1(skiatest::Reporter* reporter, const char* filename) {
SkPath path;
path.setFillType(SkPath::kEvenOdd_FillType);
path.moveTo(SkBits2Float(0x4e800002), SkBits2Float(0xce7ffffe));  // 1.07374e+09f, -1.07374e+09f
path.conicTo(SkBits2Float(0x4e800002), SkBits2Float(0xcf000000), SkBits2Float(0x43b40000), SkBits2Float(0xcf000000), SkBits2Float(0x3f3504f4));  // 1.07374e+09f, -2.14748e+09f, 360, -2.14748e+09f, 0.707107f
path.lineTo(SkBits2Float(0x43348000), SkBits2Float(0x43800001));  // 180.5f, 256
path.lineTo(SkBits2Float(0x42000000), SkBits2Float(0x41800000));  // 32, 16
path.lineTo(SkBits2Float(0x42c80000), SkBits2Float(0x44000000));  // 100, 512
path.lineTo(SkBits2Float(0x43553abd), SkBits2Float(0x440f3cbd));  // 213.229f, 572.949f
path.lineTo(SkBits2Float(0x43b40000), SkBits2Float(0x44800000));  // 360, 1024
path.lineTo(SkBits2Float(0x43b40000), SkBits2Float(0x45816000));  // 360, 4140
path.conicTo(SkBits2Float(0x43b40005), SkBits2Float(0x458a945d), SkBits2Float(0x45610000), SkBits2Float(0x458a945d), SkBits2Float(0x3f3504f3));  // 360, 4434.55f, 3600, 4434.55f, 0.707107f
path.conicTo(SkBits2Float(0x45d5bfff), SkBits2Float(0x458a945d), SkBits2Float(0x45d5bfff), SkBits2Float(0x45816000), SkBits2Float(0x3f3504f3));  // 6840, 4434.55f, 6840, 4140, 0.707107f
path.lineTo(SkBits2Float(0x43553abd), SkBits2Float(0x440f3cbd));  // 213.229f, 572.949f
path.lineTo(SkBits2Float(0x43348000), SkBits2Float(0x43800001));  // 180.5f, 256
path.conicTo(SkBits2Float(0x4e800002), SkBits2Float(0x43800001), SkBits2Float(0x4e800002), SkBits2Float(0xce7ffffe), SkBits2Float(0x3f3504f4));  // 1.07374e+09f, 256, 1.07374e+09f, -1.07374e+09f, 0.707107f
path.close();
SkPath path1(path);
path.reset();
path.moveTo(SkBits2Float(0x42fe0000), SkBits2Float(0x43a08000));  // 127, 321
path.lineTo(SkBits2Float(0x45d5c000), SkBits2Float(0x43870000));  // 6840, 270
path.lineTo(SkBits2Float(0xd0a00000), SkBits2Float(0x4cbebc20));  // -2.14748e+10f, 1e+08
path.lineTo(SkBits2Float(0x451f7000), SkBits2Float(0x42800000));  // 2551, 64
path.lineTo(SkBits2Float(0x42fe0000), SkBits2Float(0x43a08000));  // 127, 321
path.close();
SkPath path2(path);
testPathOpFuzz(reporter, path1, path2, kUnion_SkPathOp, filename);
}

static void fuzz763_9(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.conicTo(SkBits2Float(0x2a8c555b), SkBits2Float(0x081f2a21), SkBits2Float(0x7bc00321), SkBits2Float(0xed7a6a4b), SkBits2Float(0x1f212a8c));  // 2.49282e-13f, 4.78968e-34f, 1.99397e+36f, -4.84373e+27f, 3.41283e-20f
path.lineTo(SkBits2Float(0x7bc00321), SkBits2Float(0xed7a6a4b));  // 1.99397e+36f, -4.84373e+27f
path.lineTo(SkBits2Float(0x282a3a21), SkBits2Float(0x3a21df28));  // 9.4495e-15f, 0.000617492f
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.close();
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.quadTo(SkBits2Float(0x8a284f9a), SkBits2Float(0x3ac23ab3), SkBits2Float(0x1d2a2928), SkBits2Float(0x63962be6));  // -8.10388e-33f, 0.00148185f, 2.25206e-21f, 5.54035e+21f
path.moveTo(SkBits2Float(0x29272a81), SkBits2Float(0x2ab03a55));  // 3.71183e-14f, 3.13044e-13f
path.quadTo(SkBits2Float(0x2720213b), SkBits2Float(0x3a214729), SkBits2Float(0xdf28282a), SkBits2Float(0x8a2f2121));  // 2.22225e-15f, 0.000615227f, -1.2117e+19f, -8.43217e-33f
path.quadTo(SkBits2Float(0x373b3a27), SkBits2Float(0x201fc4c1), SkBits2Float(0x27576c2a), SkBits2Float(0x5921c25d));  // 1.11596e-05f, 1.35329e-19f, 2.98959e-15f, 2.8457e+15f
path.quadTo(SkBits2Float(0x2720213b), SkBits2Float(0x3a214729), SkBits2Float(0xdf28282a), SkBits2Float(0x3a8a3a21));  // 2.22225e-15f, 0.000615227f, -1.2117e+19f, 0.00105459f
path.cubicTo(SkBits2Float(0x373b3ac5), SkBits2Float(0x201fc422), SkBits2Float(0x523a702a), SkBits2Float(0x27576c51), SkBits2Float(0x5921c25d), SkBits2Float(0x51523a70));  // 1.11598e-05f, 1.35327e-19f, 2.00186e+11f, 2.9896e-15f, 2.8457e+15f, 5.64327e+10f
path.quadTo(SkBits2Float(0xd912102a), SkBits2Float(0x284f9a28), SkBits2Float(0xb38a1f30), SkBits2Float(0x3a3ac23a));  // -2.56957e+15f, 1.15242e-14f, -6.4318e-08f, 0.000712428f
path.lineTo(SkBits2Float(0xc809272a), SkBits2Float(0x29b02829));  // -140445, 7.82294e-14f

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 1, filename);
}


static void fuzz763_4(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.lineTo(SkBits2Float(0x555b3a2d), SkBits2Float(0x2a212a8c));  // 1.50652e+13f, 1.43144e-13f
path.conicTo(SkBits2Float(0xc0032108), SkBits2Float(0x7a6a4b7b), SkBits2Float(0x212a8ced), SkBits2Float(0x0321081f), SkBits2Float(0x6a3a7bc0));  // -2.04889f, 3.04132e+35f, 5.77848e-19f, 4.7323e-37f, 5.63611e+25f
path.conicTo(SkBits2Float(0x3a2147ed), SkBits2Float(0xdf28282a), SkBits2Float(0x3a8a3a21), SkBits2Float(0x8a284f9a), SkBits2Float(0x3ac2b33a));  // 0.000615238f, -1.2117e+19f, 0.00105459f, -8.10388e-33f, 0.00148544f
path.cubicTo(SkBits2Float(0x1d2a2928), SkBits2Float(0x63962be6), SkBits2Float(0x295b2d2a), SkBits2Float(0x68295b2d), SkBits2Float(0x2d296855), SkBits2Float(0x2a8c275b));  // 2.25206e-21f, 5.54035e+21f, 4.86669e-14f, 3.19905e+24f, 9.6297e-12f, 2.48963e-13f
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.close();
path.moveTo(SkBits2Float(0x55685b1f), SkBits2Float(0x5b2d2968));  // 1.59674e+13f, 4.87407e+16f
path.lineTo(SkBits2Float(0x2a212a8c), SkBits2Float(0x2a21081f));  // 1.43144e-13f, 1.43025e-13f
path.conicTo(SkBits2Float(0xde6a4b7b), SkBits2Float(0x2a8ced7a), SkBits2Float(0x21081f21), SkBits2Float(0x3a7bc003), SkBits2Float(0x47ed7a6a));  // -4.22068e+18f, 2.50338e-13f, 4.61198e-19f, 0.00096035f, 121589
path.lineTo(SkBits2Float(0x55685b1f), SkBits2Float(0x5b2d2968));  // 1.59674e+13f, 4.87407e+16f
path.close();
path.moveTo(SkBits2Float(0x55685b1f), SkBits2Float(0x5b2d2968));  // 1.59674e+13f, 4.87407e+16f
path.quadTo(SkBits2Float(0xdf28282a), SkBits2Float(0x3a8a3a21), SkBits2Float(0x8a284f9a), SkBits2Float(0x3ac23ab3));  // -1.2117e+19f, 0.00105459f, -8.10388e-33f, 0.00148185f
path.lineTo(SkBits2Float(0x2928088c), SkBits2Float(0x2be61d2a));  // 3.73109e-14f, 1.63506e-12f
path.conicTo(SkBits2Float(0x2a812a63), SkBits2Float(0x2d292a27), SkBits2Float(0x5568295b), SkBits2Float(0x5b2d2968), SkBits2Float(0x552d6829));  // 2.29444e-13f, 9.6159e-12f, 1.5954e+13f, 4.87407e+16f, 1.19164e+13f
path.conicTo(SkBits2Float(0x395b2d5b), SkBits2Float(0x68552768), SkBits2Float(0x555b2df0), SkBits2Float(0x1f722a8c), SkBits2Float(0x082a212a));  // 0.000209024f, 4.02636e+24f, 1.50619e+13f, 5.12807e-20f, 5.11965e-34f
path.lineTo(SkBits2Float(0x55685b1f), SkBits2Float(0x5b2d2968));  // 1.59674e+13f, 4.87407e+16f
path.close();
path.moveTo(SkBits2Float(0x212a8c55), SkBits2Float(0x21081f2a));  // 5.7784e-19f, 4.61198e-19f
path.conicTo(SkBits2Float(0x6a4b7bc0), SkBits2Float(0x2147ed7a), SkBits2Float(0x28282a3a), SkBits2Float(0x21df212a), SkBits2Float(0x033a8a3a));  // 6.14991e+25f, 6.77381e-19f, 9.33503e-15f, 1.51198e-18f, 5.48192e-37f

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 1, filename);
}

static void fuzz763_3(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.lineTo(SkBits2Float(0x555b292d), SkBits2Float(0x2a212a8c));  // 1.50606e+13f, 1.43144e-13f
path.conicTo(SkBits2Float(0xc0032108), SkBits2Float(0x7a6a4b7b), SkBits2Float(0x212a8ced), SkBits2Float(0x295b2d1f), SkBits2Float(0x29685568));  // -2.04889f, 3.04132e+35f, 5.77848e-19f, 4.86669e-14f, 5.15884e-14f
path.conicTo(SkBits2Float(0x8c28295b), SkBits2Float(0x1f21212a), SkBits2Float(0xc0032a08), SkBits2Float(0x7a6a4b7b), SkBits2Float(0x212a8ced));  // -1.29547e-31f, 3.41205e-20f, -2.04944f, 3.04132e+35f, 5.77848e-19f
path.moveTo(SkBits2Float(0x25682929), SkBits2Float(0x212a8c5b));  // 2.01367e-16f, 5.7784e-19f
path.moveTo(SkBits2Float(0x0321081f), SkBits2Float(0x6a4a7bc0));  // 4.7323e-37f, 6.11969e+25f
path.conicTo(SkBits2Float(0x032108ed), SkBits2Float(0x283a7bc0), SkBits2Float(0x47ed7a6a), SkBits2Float(0x282a3a21), SkBits2Float(0x3a21ff28));  // 4.73239e-37f, 1.03519e-14f, 121589, 9.4495e-15f, 0.000617968f
path.quadTo(SkBits2Float(0x8a284f9a), SkBits2Float(0x3ac23ab3), SkBits2Float(0x2a292827), SkBits2Float(0x962be61d));  // -8.10388e-33f, 0.00148185f, 1.50241e-13f, -1.38859e-25f
path.lineTo(SkBits2Float(0x295b2d2a), SkBits2Float(0x2d296868));  // 4.86669e-14f, 9.62972e-12f
path.moveTo(SkBits2Float(0x212a8c55), SkBits2Float(0x21081f2a));  // 5.7784e-19f, 4.61198e-19f
path.conicTo(SkBits2Float(0x6a4b7bc0), SkBits2Float(0x898ced7a), SkBits2Float(0x21081f21), SkBits2Float(0x3a7bc003), SkBits2Float(0x47ed7a6a));  // 6.14991e+25f, -3.39271e-33f, 4.61198e-19f, 0.00096035f, 121589
path.lineTo(SkBits2Float(0x212a8c55), SkBits2Float(0x21081f2a));  // 5.7784e-19f, 4.61198e-19f
path.close();
path.moveTo(SkBits2Float(0x212a8c55), SkBits2Float(0x21081f2a));  // 5.7784e-19f, 4.61198e-19f
path.quadTo(SkBits2Float(0xdf28282a), SkBits2Float(0x3a8a3a21), SkBits2Float(0xb38a281a), SkBits2Float(0x29283ac2));  // -1.2117e+19f, 0.00105459f, -6.43342e-08f, 3.73545e-14f
path.moveTo(SkBits2Float(0x962be61d), SkBits2Float(0x432a2927));  // -1.38859e-25f, 170.161f
path.conicTo(SkBits2Float(0x3a2a552a), SkBits2Float(0x3b1e2ab0), SkBits2Float(0x29272021), SkBits2Float(0x3b3ac527), SkBits2Float(0x1fc42236));  // 0.000649768f, 0.00241343f, 3.71093e-14f, 0.00284989f, 8.30658e-20f
path.cubicTo(SkBits2Float(0x27576c2a), SkBits2Float(0x5921c25d), SkBits2Float(0x51503a70), SkBits2Float(0x12102a10), SkBits2Float(0x633a28d9), SkBits2Float(0x29c80927));  // 2.98959e-15f, 2.8457e+15f, 5.58959e+10f, 4.54902e-28f, 3.43404e+21f, 8.88337e-14f
path.lineTo(SkBits2Float(0x272927b0), SkBits2Float(0x5b392929));  // 2.3475e-15f, 5.21181e+16f
path.moveTo(SkBits2Float(0x3a1127b4), SkBits2Float(0x2921ee3b));  // 0.000553723f, 3.59558e-14f
path.cubicTo(SkBits2Float(0x5e215d3b), SkBits2Float(0x7828ee3a), SkBits2Float(0x8e28b03b), SkBits2Float(0x50783be8), SkBits2Float(0x9e0b8a3a), SkBits2Float(0x555b2d68));  // 2.90688e+18f, 1.37053e+34f, -2.07925e-30f, 1.66587e+10f, -7.38718e-21f, 1.50618e+13f
path.moveTo(SkBits2Float(0x21081f3f), SkBits2Float(0x9fd4e62a));  // 4.61199e-19f, -9.01663e-20f
path.cubicTo(SkBits2Float(0x3a293a2a), SkBits2Float(0x0e3bf0c5), SkBits2Float(0x3b29d42a), SkBits2Float(0x0f217265), SkBits2Float(0x2d5d2921), SkBits2Float(0x5568295b));  // 0.000645551f, 2.31655e-30f, 0.00259138f, 7.95994e-30f, 1.25715e-11f, 1.5954e+13f

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 1, filename);
}

static void fuzz763_5(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x5b292d55), SkBits2Float(0x2a2a8c55));  // 4.76191e+16f, 1.51477e-13f
path.conicTo(SkBits2Float(0xc0032108), SkBits2Float(0x7a6a4b79), SkBits2Float(0x212a8ced), SkBits2Float(0x0321081f), SkBits2Float(0x6a3a7bc0));  // -2.04889f, 3.04132e+35f, 5.77848e-19f, 4.7323e-37f, 5.63611e+25f
path.conicTo(SkBits2Float(0x3a2147ed), SkBits2Float(0xdf28282a), SkBits2Float(0x3a8a3a21), SkBits2Float(0x8a284f9a), SkBits2Float(0x3ac23ab3));  // 0.000615238f, -1.2117e+19f, 0.00105459f, -8.10388e-33f, 0.00148185f
path.cubicTo(SkBits2Float(0xe62a2928), SkBits2Float(0x2a63962b), SkBits2Float(0x68295b2d), SkBits2Float(0x2d296855), SkBits2Float(0x2a8c555b), SkBits2Float(0x001f2a21));  // -2.0089e+23f, 2.02138e-13f, 3.19905e+24f, 9.6297e-12f, 2.49282e-13f, 2.86201e-39f
path.lineTo(SkBits2Float(0x5b292d55), SkBits2Float(0x2a2a8c55));  // 4.76191e+16f, 1.51477e-13f
path.close();
path.moveTo(SkBits2Float(0x5b292d55), SkBits2Float(0x2a2a8c55));  // 4.76191e+16f, 1.51477e-13f
path.conicTo(SkBits2Float(0x6a4b7bc0), SkBits2Float(0x2a8ced7a), SkBits2Float(0x21081f21), SkBits2Float(0x3a7bc003), SkBits2Float(0x47ed7a6a));  // 6.14991e+25f, 2.50338e-13f, 4.61198e-19f, 0.00096035f, 121589
path.lineTo(SkBits2Float(0x5b292d55), SkBits2Float(0x2a2a8c55));  // 4.76191e+16f, 1.51477e-13f
path.close();
path.moveTo(SkBits2Float(0x5b292d55), SkBits2Float(0x2a2a8c55));  // 4.76191e+16f, 1.51477e-13f
path.quadTo(SkBits2Float(0xdf28282a), SkBits2Float(0x3a8a3b21), SkBits2Float(0x28ee4f9a), SkBits2Float(0x68293b78));  // -1.2117e+19f, 0.00105462f, 2.64578e-14f, 3.19671e+24f
path.lineTo(SkBits2Float(0x5b2d2968), SkBits2Float(0x5b2d8c55));  // 4.87407e+16f, 4.88495e+16f

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 4, filename);
}

static void fuzz763_2(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.lineTo(SkBits2Float(0x555b292d), SkBits2Float(0x2a212a8c));  // 1.50606e+13f, 1.43144e-13f
path.conicTo(SkBits2Float(0xc0032108), SkBits2Float(0x7a6a4b7b), SkBits2Float(0x212a8ced), SkBits2Float(0x0321081f), SkBits2Float(0x6a3a7bc0));  // -2.04889f, 3.04132e+35f, 5.77848e-19f, 4.7323e-37f, 5.63611e+25f
path.lineTo(SkBits2Float(0x081f2ad7), SkBits2Float(0x7bc00321));  // 4.78977e-34f, 1.99397e+36f
path.moveTo(SkBits2Float(0x2a3a2147), SkBits2Float(0xdf212828));  // 1.65317e-13f, -1.16126e+19f
path.quadTo(SkBits2Float(0x4f1a3a8a), SkBits2Float(0x3ab38a28), SkBits2Float(0x29283ac2), SkBits2Float(0x962be62a));  // 2.58753e+09f, 0.00136978f, 3.73545e-14f, -1.38859e-25f
path.cubicTo(SkBits2Float(0x272a812a), SkBits2Float(0x3a2a5529), SkBits2Float(0x3b1e2ab0), SkBits2Float(0x29272021), SkBits2Float(0x3b3ac527), SkBits2Float(0x1fc42237));  // 2.36623e-15f, 0.000649768f, 0.00241343f, 3.71093e-14f, 0.00284989f, 8.30658e-20f
path.cubicTo(SkBits2Float(0x27576c2a), SkBits2Float(0x5921c25d), SkBits2Float(0x51523a70), SkBits2Float(0x12102a10), SkBits2Float(0x633a28d9), SkBits2Float(0x29c80927));  // 2.98959e-15f, 2.8457e+15f, 5.64327e+10f, 4.54902e-28f, 3.43404e+21f, 8.88337e-14f
path.lineTo(SkBits2Float(0x29292727), SkBits2Float(0x21475b3b));  // 3.75595e-14f, 6.75446e-19f
path.quadTo(SkBits2Float(0xdf28282a), SkBits2Float(0x3a8a3a21), SkBits2Float(0x8a284f9a), SkBits2Float(0x3ac23ab3));  // -1.2117e+19f, 0.00105459f, -8.10388e-33f, 0.00148185f
path.cubicTo(SkBits2Float(0x682d2928), SkBits2Float(0x555b6829), SkBits2Float(0x555b292d), SkBits2Float(0x2a212a8c), SkBits2Float(0x0321081f), SkBits2Float(0x6a4b7bc0));  // 3.27091e+24f, 1.50775e+13f, 1.50606e+13f, 1.43144e-13f, 4.7323e-37f, 6.14991e+25f
path.conicTo(SkBits2Float(0x295b2ded), SkBits2Float(0x29685568), SkBits2Float(0x8c555b2d), SkBits2Float(0xe61d2a2a), SkBits2Float(0x2a63962b));  // 4.86676e-14f, 5.15884e-14f, -1.64364e-31f, -1.85547e+23f, 2.02138e-13f
path.conicTo(SkBits2Float(0x5568295b), SkBits2Float(0x5b2d2968), SkBits2Float(0x212a8c55), SkBits2Float(0x21081f2a), SkBits2Float(0x4b7bc003));  // 1.5954e+13f, 4.87407e+16f, 5.7784e-19f, 4.61198e-19f, 1.64987e+07f
path.lineTo(SkBits2Float(0x2a8ced7a), SkBits2Float(0x21081f21));  // 2.50338e-13f, 4.61198e-19f
path.conicTo(SkBits2Float(0x6a3a7bc0), SkBits2Float(0x2147ed7a), SkBits2Float(0x28282a3a), SkBits2Float(0x8a3a21df), SkBits2Float(0x27b42a3a));  // 5.63611e+25f, 6.77381e-19f, 9.33503e-15f, -8.96194e-33f, 5.00058e-15f
path.conicTo(SkBits2Float(0x2921217d), SkBits2Float(0x5e3a3b35), SkBits2Float(0x7828ee3a), SkBits2Float(0x8e28b03b), SkBits2Float(0x783be82a));  // 3.57782e-14f, 3.35484e+18f, 1.37053e+34f, -2.07925e-30f, 1.52448e+34f
path.conicTo(SkBits2Float(0x8e0b8a3a), SkBits2Float(0x279fd4e6), SkBits2Float(0x7a293a2a), SkBits2Float(0x2a0ef0c5), SkBits2Float(0x653b29d4));  // -1.71996e-30f, 4.43622e-15f, 2.19669e+35f, 1.26957e-13f, 5.52409e+22f
path.quadTo(SkBits2Float(0x29210f21), SkBits2Float(0x282a085d), SkBits2Float(0xc2ab2127), SkBits2Float(0xa6800028));  // 3.57623e-14f, 9.43871e-15f, -85.5648f, -8.88183e-16f
path.lineTo(SkBits2Float(0x2a3a2147), SkBits2Float(0xdf212828));  // 1.65317e-13f, -1.16126e+19f
path.close();
path.moveTo(SkBits2Float(0x2a3a2147), SkBits2Float(0xdf212828));  // 1.65317e-13f, -1.16126e+19f
path.quadTo(SkBits2Float(0x216a2770), SkBits2Float(0x2ab73b28), SkBits2Float(0x4b28f427), SkBits2Float(0x283b5b28));  // 7.93345e-19f, 3.25484e-13f, 1.10726e+07f, 1.04004e-14f
path.lineTo(SkBits2Float(0x2a3a2147), SkBits2Float(0xdf212828));  // 1.65317e-13f, -1.16126e+19f
path.close();
path.moveTo(SkBits2Float(0x2a3a2147), SkBits2Float(0xdf212828));  // 1.65317e-13f, -1.16126e+19f
path.conicTo(SkBits2Float(0xf86d273b), SkBits2Float(0x27e523e3), SkBits2Float(0x2927e0f5), SkBits2Float(0x2ac0e729), SkBits2Float(0x6b492128));  // -1.92402e+34f, 6.35992e-15f, 3.72766e-14f, 3.42665e-13f, 2.43151e+26f
path.cubicTo(SkBits2Float(0x2f273927), SkBits2Float(0xa83a2c21), SkBits2Float(0xd7122121), SkBits2Float(0x21212921), SkBits2Float(0x3be3db3a), SkBits2Float(0xa9deb63b));  // 1.52089e-10f, -1.03346e-14f, -1.60671e+14f, 5.46034e-19f, 0.00695362f, -9.89039e-14f

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 1, filename);
}

// crbug.com/626164
static void fuzz763_1c(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
    path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
    path.cubicTo(SkBits2Float(0x1931204a), SkBits2Float(0x2ba1a14a), SkBits2Float(0x4a4a08ff), SkBits2Float(0x4a4a08ff), SkBits2Float(0x4a4a4a34), SkBits2Float(0x4a4a4a4a));  // 9.15721e-24f, 1.14845e-12f, 3.31014e+06f, 3.31014e+06f, 3.31432e+06f, 3.31432e+06f
    path.moveTo(SkBits2Float(0x000010a1), SkBits2Float(0x19312000));  // 5.96533e-42f, 9.15715e-24f
    path.cubicTo(SkBits2Float(0x4a4a4a4a), SkBits2Float(0x4a4a4a4a), SkBits2Float(0xa14a4a4a), SkBits2Float(0x08ff2ba1), SkBits2Float(0x08ff4a4a), SkBits2Float(0x4a344a4a));  // 3.31432e+06f, 3.31432e+06f, -6.85386e-19f, 1.53575e-33f, 1.53647e-33f, 2.95387e+06f
    path.cubicTo(SkBits2Float(0x4a4a4a4a), SkBits2Float(0x4a4a4a4a), SkBits2Float(0x2ba1a14a), SkBits2Float(0x4e4a08ff), SkBits2Float(0x4a4a4a4a), SkBits2Float(0xa1a181ff));  // 3.31432e+06f, 3.31432e+06f, 1.14845e-12f, 8.47397e+08f, 3.31432e+06f, -1.09442e-18f

    SkPath path2(path);
    SkPath dummy;
    testPathOpFuzz(reporter, path1, path2, (SkPathOp)4, filename);
}

// crbug.com/626186
static void fuzz763_1b(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
    path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
    path.cubicTo(SkBits2Float(0x0000ff07), SkBits2Float(0xf9f9ff00), SkBits2Float(0xfe0ef9f4), SkBits2Float(0xd9b105fb), SkBits2Float(0x000000f9), SkBits2Float(0xfe11f901));  // 9.14866e-41f, -1.62257e+35f, -4.75121e+37f, -6.22846e+15f, 3.48923e-43f, -4.85077e+37f
    path.lineTo(SkBits2Float(0xda1905ed), SkBits2Float(0x3c05fbfb));  // -1.0768e+16f, 0.00817775f
    path.cubicTo(SkBits2Float(0x3c3c3c3c), SkBits2Float(0x3c3c3c3c), SkBits2Float(0x253c7f00), SkBits2Float(0xfa00d3fa), SkBits2Float(0x250025fe), SkBits2Float(0x00000006));  // 0.011489f, 0.011489f, 1.63494e-16f, -1.67228e+35f, 1.11151e-16f, 8.40779e-45f

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
    path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
    path.quadTo(SkBits2Float(0x3c3c3c3c), SkBits2Float(0xfa253c3c), SkBits2Float(0xfefa00d3), SkBits2Float(0x25fad9df));  // 0.011489f, -2.14488e+35f, -1.66156e+38f, 4.35157e-16f
    path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
    path.close();
    path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
    path.lineTo(SkBits2Float(0x8dfefa00), SkBits2Float(0xf0f9fad9));  // -1.57141e-30f, -6.1892e+29f
    path.cubicTo(SkBits2Float(0x20fe58f9), SkBits2Float(0x0525fbed), SkBits2Float(0x1905ffff), SkBits2Float(0x01f9f9f9), SkBits2Float(0xfbfe0ef9), SkBits2Float(0xfb212fff));  // 4.30882e-19f, 7.80453e-36f, 6.92764e-24f, 9.18268e-38f, -2.63829e+36f, -8.36933e+35f

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp)2, filename);
}

static void fuzz763_1a(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
    path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
    path.cubicTo(SkBits2Float(0x154be880), SkBits2Float(0x80000640), SkBits2Float(0x5559a419), SkBits2Float(0x59d55928), SkBits2Float(0x80045959), SkBits2Float(0x40154be8));  // 4.11789e-26f, -2.24208e-42f, 1.49562e+13f, 7.50652e+15f, -3.99394e-40f, 2.33276f

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
    path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
    path.quadTo(SkBits2Float(0x5559a419), SkBits2Float(0x59d55928), SkBits2Float(0xbd595959), SkBits2Float(0x3f3f3f09));  // 1.49562e+13f, 7.50652e+15f, -0.0530637f, 0.747056f
    path.moveTo(SkBits2Float(0x3f3f3f3f), SkBits2Float(0x3f3f3f3f));  // 0.747059f, 0.747059f
    path.moveTo(SkBits2Float(0x3f3f3f3f), SkBits2Float(0xff3f3f3f));  // 0.747059f, -2.54211e+38f
    path.lineTo(SkBits2Float(0x09090909), SkBits2Float(0x3038d509));  // 1.6495e-33f, 6.72416e-10f
    path.conicTo(SkBits2Float(0x5947ffff), SkBits2Float(0x40e88004), SkBits2Float(0x00002059), SkBits2Float(0x28555900), SkBits2Float(0x5959d559));  // 3.51844e+15f, 7.26563f, 1.16042e-41f, 1.18432e-14f, 3.83217e+15f
    path.lineTo(SkBits2Float(0x3f3f3f3f), SkBits2Float(0xff3f3f3f));  // 0.747059f, -2.54211e+38f
    path.close();
    path.moveTo(SkBits2Float(0x3f3f3f3f), SkBits2Float(0xff3f3f3f));  // 0.747059f, -2.54211e+38f
    path.lineTo(SkBits2Float(0x38d57f4b), SkBits2Float(0x59597f4b));  // 0.000101803f, 3.82625e+15f
    path.lineTo(SkBits2Float(0x3f3f3f3f), SkBits2Float(0xff3f3f3f));  // 0.747059f, -2.54211e+38f
    path.close();
    path.moveTo(SkBits2Float(0x384700ff), SkBits2Float(0x0108804b));  // 4.74462e-05f, 2.50713e-38f

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp)0, filename);
}

// crbug.com/627780
static void fuzz763_3a(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.lineTo(SkBits2Float(0x555b292d), SkBits2Float(0x2a212a8c));  // 1.50606e+13f, 1.43144e-13f
path.conicTo(SkBits2Float(0xc0032108), SkBits2Float(0x7a6a4b7b), SkBits2Float(0x212a8ced), SkBits2Float(0x0321081f), SkBits2Float(0x6a3a7bc0));  // -2.04889f, 3.04132e+35f, 5.77848e-19f, 4.7323e-37f, 5.63611e+25f
path.conicTo(SkBits2Float(0x3a2147ed), SkBits2Float(0xdf28282a), SkBits2Float(0x3a8a3a21), SkBits2Float(0x8a284f9a), SkBits2Float(0x3ac23ab3));  // 0.000615238f, -1.2117e+19f, 0.00105459f, -8.10388e-33f, 0.00148185f
path.cubicTo(SkBits2Float(0x1d2a2928), SkBits2Float(0x63962be6), SkBits2Float(0x272a812a), SkBits2Float(0x295b2d29), SkBits2Float(0x2a685568), SkBits2Float(0x68295b2d));  // 2.25206e-21f, 5.54035e+21f, 2.36623e-15f, 4.86669e-14f, 2.06354e-13f, 3.19905e+24f
path.conicTo(SkBits2Float(0x2a8c555b), SkBits2Float(0x081f2a21), SkBits2Float(0x7bc00321), SkBits2Float(0x7a6a4b77), SkBits2Float(0x3a214726));  // 2.49282e-13f, 4.78968e-34f, 1.99397e+36f, 3.04132e+35f, 0.000615226f
path.moveTo(SkBits2Float(0x8adf2028), SkBits2Float(0x3a219a3a));  // -2.14862e-32f, 0.000616464f
path.quadTo(SkBits2Float(0x3ab38e28), SkBits2Float(0x29283ac2), SkBits2Float(0x2be61d2a), SkBits2Float(0x812a4396));  // 0.0013699f, 3.73545e-14f, 1.63506e-12f, -3.12726e-38f

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 1, filename);
}

// crbug.com/627689
static void fuzz763_5a(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x38bd8610), SkBits2Float(0x00000000));  // 9.03719e-05f, 0
path.conicTo(SkBits2Float(0x4183d871), SkBits2Float(0x41fea321), SkBits2Float(0xb700ff00), SkBits2Float(0x4240b8b8), SkBits2Float(0x3b058283));  // 16.4807f, 31.8297f, -7.68877e-06f, 48.1804f, 0.0020372f
path.lineTo(SkBits2Float(0x3a3a3ab8), SkBits2Float(0xb8b8b8b8));  // 0.000710409f, -8.80821e-05f
path.conicTo(SkBits2Float(0x3a455ec8), SkBits2Float(0xb8b8b8b3), SkBits2Float(0x38b2418d), SkBits2Float(0xb730d014), SkBits2Float(0x3f7ffff3));  // 0.000752908f, -8.80821e-05f, 8.49991e-05f, -1.05389e-05f, 0.999999f
path.quadTo(SkBits2Float(0x3a51246a), SkBits2Float(0xb6da45a3), SkBits2Float(0x38bc5c3c), SkBits2Float(0x00000000));  // 0.000797814f, -6.50501e-06f, 8.98172e-05f, 0
path.lineTo(SkBits2Float(0x3a3a3ab8), SkBits2Float(0xb8b8b8b8));  // 0.000710409f, -8.80821e-05f
path.quadTo(SkBits2Float(0x39a32d2d), SkBits2Float(0x00000000), SkBits2Float(0xb8a13a00), SkBits2Float(0x00000000));  // 0.000311234f, 0, -7.68788e-05f, 0
path.lineTo(SkBits2Float(0x3a3a3ab8), SkBits2Float(0xb8b8b8b8));  // 0.000710409f, -8.80821e-05f
path.quadTo(SkBits2Float(0x39ba814c), SkBits2Float(0xb838fed2), SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0.00035573f, -4.41063e-05f, 0, 0
path.lineTo(SkBits2Float(0x38bd8610), SkBits2Float(0x00000000));  // 9.03719e-05f, 0
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 4, filename);
}

// crbug.com/627401
static void fuzz763_2a(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.quadTo(SkBits2Float(0x3e484500), SkBits2Float(0x164f3a30), SkBits2Float(0x49484801), SkBits2Float(0x7d0100c8));  // 0.195576f, 1.67397e-25f, 820352, 1.07172e+37f
path.conicTo(SkBits2Float(0xff7f36fd), SkBits2Float(0x3e647d01), SkBits2Float(0x0c00f430), SkBits2Float(0x486b6448), SkBits2Float(0x00484848));  // -3.39239e+38f, 0.223133f, 9.93424e-32f, 241041, 6.63809e-39f
path.lineTo(SkBits2Float(0x4f4f557d), SkBits2Float(0x48480112));  // 3.47849e+09f, 204804
path.lineTo(SkBits2Float(0xf40c01ff), SkBits2Float(0x45008000));  // -4.43702e+31f, 2056
path.moveTo(SkBits2Float(0x4bfffa00), SkBits2Float(0x7d4ac859));  // 3.35514e+07f, 1.68465e+37f
path.conicTo(SkBits2Float(0x7d014f3e), SkBits2Float(0x00f4ff01), SkBits2Float(0x6b64480c), SkBits2Float(0x48484848), SkBits2Float(0x557d0100));  // 1.07426e+37f, 2.24993e-38f, 2.75975e+26f, 205089, 1.73863e+13f

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 0, filename);
}

// crbug.com/627761
static void fuzz763_2b(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x5b292d55), SkBits2Float(0x212a8c55));  // 4.76191e+16f, 5.7784e-19f
path.moveTo(SkBits2Float(0x3b21081f), SkBits2Float(0x4b7bc003));  // 0.00245715f, 1.64987e+07f
path.lineTo(SkBits2Float(0x2a8ced7a), SkBits2Float(0x21081f21));  // 2.50338e-13f, 4.61198e-19f
path.conicTo(SkBits2Float(0x6a3a7bc0), SkBits2Float(0x4721ed7a), SkBits2Float(0x282a3a21), SkBits2Float(0x3a21df28), SkBits2Float(0x4f9a3a8a));  // 5.63611e+25f, 41453.5f, 9.4495e-15f, 0.000617492f, 5.17506e+09f
path.lineTo(SkBits2Float(0x3b21081f), SkBits2Float(0x4b7bc003));  // 0.00245715f, 1.64987e+07f
path.close();
path.moveTo(SkBits2Float(0x3b21081f), SkBits2Float(0x4b7bc003));  // 0.00245715f, 1.64987e+07f
path.cubicTo(SkBits2Float(0x273ac23a), SkBits2Float(0x1d2a2928), SkBits2Float(0x63962be6), SkBits2Float(0x272a812a), SkBits2Float(0x295b2d29), SkBits2Float(0x29685568));  // 2.5918e-15f, 2.25206e-21f, 5.54035e+21f, 2.36623e-15f, 4.86669e-14f, 5.15884e-14f
path.lineTo(SkBits2Float(0x081f2a21), SkBits2Float(0x7bc00321));  // 4.78968e-34f, 1.99397e+36f
path.lineTo(SkBits2Float(0x282a3a21), SkBits2Float(0x3a21df28));  // 9.4495e-15f, 0.000617492f
path.lineTo(SkBits2Float(0x3b21081f), SkBits2Float(0x4b7bc003));  // 0.00245715f, 1.64987e+07f
path.close();
path.moveTo(SkBits2Float(0x3b21081f), SkBits2Float(0x4b7bc003));  // 0.00245715f, 1.64987e+07f
path.quadTo(SkBits2Float(0x8a4fc29a), SkBits2Float(0x3ab3283a), SkBits2Float(0x1d2a2928), SkBits2Float(0x43962be6));  // -1.00033e-32f, 0.00136686f, 2.25206e-21f, 300.343f
path.moveTo(SkBits2Float(0x5b2d2a81), SkBits2Float(0x29276829));  // 4.87419e+16f, 3.71718e-14f
path.conicTo(SkBits2Float(0x1e2ab03a), SkBits2Float(0x2920213b), SkBits2Float(0x3b3ac527), SkBits2Float(0xc422333b), SkBits2Float(0x6c2a9f1f));  // 9.03617e-21f, 3.5556e-14f, 0.00284989f, -648.8f, 8.25075e+26f
path.quadTo(SkBits2Float(0xc25d2757), SkBits2Float(0x3a705921), SkBits2Float(0x2a105152), SkBits2Float(0x28d91210));  // -55.2884f, 0.000916855f, 1.2818e-13f, 2.40997e-14f
path.quadTo(SkBits2Float(0x68295b2d), SkBits2Float(0x2d296855), SkBits2Float(0x2a8c555b), SkBits2Float(0x081f2a21));  // 3.19905e+24f, 9.6297e-12f, 2.49282e-13f, 4.78968e-34f
path.lineTo(SkBits2Float(0x5b2d2a81), SkBits2Float(0x29276829));  // 4.87419e+16f, 3.71718e-14f
path.close();
path.moveTo(SkBits2Float(0x5b2d2a81), SkBits2Float(0x29276829));  // 4.87419e+16f, 3.71718e-14f
path.conicTo(SkBits2Float(0x6a4b7bc0), SkBits2Float(0x2a8ced7a), SkBits2Float(0x21081f21), SkBits2Float(0xcb7bc003), SkBits2Float(0x47ed7a6a));  // 6.14991e+25f, 2.50338e-13f, 4.61198e-19f, -1.64987e+07f, 121589
path.lineTo(SkBits2Float(0x5b2d2a81), SkBits2Float(0x29276829));  // 4.87419e+16f, 3.71718e-14f
path.close();
path.moveTo(SkBits2Float(0x5b2d2a81), SkBits2Float(0x29276829));  // 4.87419e+16f, 3.71718e-14f
path.quadTo(SkBits2Float(0xdf28282a), SkBits2Float(0x2d8a3a21), SkBits2Float(0x5b682b68), SkBits2Float(0x5b292d55));  // -1.2117e+19f, 1.57146e-11f, 6.53499e+16f, 4.76191e+16f
path.lineTo(SkBits2Float(0x2a212a8c), SkBits2Float(0x0321081f));  // 1.43144e-13f, 4.7323e-37f
path.conicTo(SkBits2Float(0x7a6a4b7b), SkBits2Float(0x212a8ced), SkBits2Float(0x0321081f), SkBits2Float(0x6a3a7bc0), SkBits2Float(0x3a21477a));  // 3.04132e+35f, 5.77848e-19f, 4.7323e-37f, 5.63611e+25f, 0.000615231f
path.moveTo(SkBits2Float(0x21df2828), SkBits2Float(0x9a3a8a3a));  // 1.51217e-18f, -3.85756e-23f
path.quadTo(SkBits2Float(0x3ab38a28), SkBits2Float(0x28273ac2), SkBits2Float(0xe61d2a29), SkBits2Float(0x2a63962b));  // 0.00136978f, 9.2831e-15f, -1.85547e+23f, 2.02138e-13f
path.conicTo(SkBits2Float(0x2d29272a), SkBits2Float(0x5568295b), SkBits2Float(0x5b2d2968), SkBits2Float(0x5b2d6829), SkBits2Float(0x212a8c55));  // 9.61523e-12f, 1.5954e+13f, 4.87407e+16f, 4.88097e+16f, 5.7784e-19f
path.moveTo(SkBits2Float(0x0321081f), SkBits2Float(0x6a4b7bc0));  // 4.7323e-37f, 6.14991e+25f
path.conicTo(SkBits2Float(0x3a2147ed), SkBits2Float(0xdf28282a), SkBits2Float(0x3a8a3a21), SkBits2Float(0x8a284f9a), SkBits2Float(0x3ac23ab3));  // 0.000615238f, -1.2117e+19f, 0.00105459f, -8.10388e-33f, 0.00148185f
path.lineTo(SkBits2Float(0x0321081f), SkBits2Float(0x6a4b7bc0));  // 4.7323e-37f, 6.14991e+25f
path.close();

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 4, filename);
}

static void fuzz763_2c(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);

path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x36344a4a));  // 0, 2.68653e-06f
path.cubicTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000), SkBits2Float(0x364a4a4a), SkBits2Float(0x364a4a4a), SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0, 3.01436e-06f, 3.01436e-06f, 0, 0
path.lineTo(SkBits2Float(0x364a4a4a), SkBits2Float(0x00000000));  // 3.01436e-06f, 0
path.cubicTo(SkBits2Float(0x364a30f0), SkBits2Float(0x344ac7fb), SkBits2Float(0x3656d432), SkBits2Float(0x34cabb48), SkBits2Float(0x367031a9), SkBits2Float(0x351802f1));  // 3.01288e-06f, 1.88855e-07f, 3.2012e-06f, 3.77617e-07f, 3.57917e-06f, 5.66287e-07f
path.cubicTo(SkBits2Float(0x36a7b150), SkBits2Float(0x35ab09db), SkBits2Float(0x371874ed), SkBits2Float(0x3604f2c7), SkBits2Float(0x3784e0c7), SkBits2Float(0x36344a51));  // 4.99763e-06f, 1.27434e-06f, 9.08713e-06f, 1.98108e-06f, 1.58403e-05f, 2.68653e-06f
path.cubicTo(SkBits2Float(0x3743dc9a), SkBits2Float(0x36344a4f), SkBits2Float(0x36fbef33), SkBits2Float(0x36344a4e), SkBits2Float(0x36604a35), SkBits2Float(0x36344a4c));  // 1.16743e-05f, 2.68653e-06f, 7.50823e-06f, 2.68653e-06f, 3.34218e-06f, 2.68653e-06f
path.cubicTo(SkBits2Float(0x36531715), SkBits2Float(0x36344a4c), SkBits2Float(0x3645e3f5), SkBits2Float(0x36344a4b), SkBits2Float(0x3638b0d4), SkBits2Float(0x36344a4b));  // 3.14549e-06f, 2.68653e-06f, 2.9488e-06f, 2.68653e-06f, 2.75211e-06f, 2.68653e-06f
path.cubicTo(SkBits2Float(0x35f64120), SkBits2Float(0x36344a4b), SkBits2Float(0x35764124), SkBits2Float(0x36344a4a), SkBits2Float(0x00000000), SkBits2Float(0x36344a4a));  // 1.83474e-06f, 2.68653e-06f, 9.17369e-07f, 2.68653e-06f, 0, 2.68653e-06f
path.close();
    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.cubicTo(SkBits2Float(0x1931204a), SkBits2Float(0x2ba1a14a), SkBits2Float(0x4a4a08ff), SkBits2Float(0x4a4a08ff), SkBits2Float(0x4a4a4a34), SkBits2Float(0x4a4a4a4a));  // 9.15721e-24f, 1.14845e-12f, 3.31014e+06f, 3.31014e+06f, 3.31432e+06f, 3.31432e+06f
path.moveTo(SkBits2Float(0x000010a1), SkBits2Float(0x19312000));  // 5.96533e-42f, 9.15715e-24f
path.cubicTo(SkBits2Float(0x4a4a4a4a), SkBits2Float(0x4a4a4a4a), SkBits2Float(0xa14a4a4a), SkBits2Float(0x08ff2ba1), SkBits2Float(0x08ff4a4a), SkBits2Float(0x4a344a4a));  // 3.31432e+06f, 3.31432e+06f, -6.85386e-19f, 1.53575e-33f, 1.53647e-33f, 2.95387e+06f
path.cubicTo(SkBits2Float(0x544a4a4a), SkBits2Float(0x4a4a4a4a), SkBits2Float(0x2ba1a14a), SkBits2Float(0x4e4a08ff), SkBits2Float(0x4a4a4a4a), SkBits2Float(0xa1a181ff));  // 3.47532e+12f, 3.31432e+06f, 1.14845e-12f, 8.47397e+08f, 3.31432e+06f, -1.09442e-18f
    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, kReverseDifference_SkPathOp, filename);
}

static void fuzz763_6(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x6a2a291f));  // 0, 5.14279e+25f
path.cubicTo(SkBits2Float(0x68295b2d), SkBits2Float(0x00000000), SkBits2Float(0x00000000), SkBits2Float(0x00000000), SkBits2Float(0x00000000), SkBits2Float(0x68556829));  // 3.19905e+24f, 0, 0, 0, 0, 4.03114e+24f
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0x68555b2a));  // 0, 4.03018e+24f
path.cubicTo(SkBits2Float(0x00000000), SkBits2Float(0x67d55b2a), SkBits2Float(0x67296a4b), SkBits2Float(0x67555b2a), SkBits2Float(0x677e1f70), SkBits2Float(0x66d55b2a));  // 0, 2.01509e+24f, 8.00041e+23f, 1.00755e+24f, 1.20006e+24f, 5.03773e+23f
path.cubicTo(SkBits2Float(0x678f0684), SkBits2Float(0x6684f008), SkBits2Float(0x6798f8ea), SkBits2Float(0x6625a942), SkBits2Float(0x67961914), SkBits2Float(0x65ce709a));  // 1.35084e+24f, 3.1389e+23f, 1.44478e+24f, 1.95578e+23f, 1.41764e+24f, 1.21861e+23f
path.cubicTo(SkBits2Float(0x679174f7), SkBits2Float(0x63199132), SkBits2Float(0x6756c79f), SkBits2Float(0x606478de), SkBits2Float(0x65682bcf), SkBits2Float(0x00000000));  // 1.3738e+24f, 2.83281e+21f, 1.01427e+24f, 6.58526e+19f, 6.85248e+22f, 0
path.conicTo(SkBits2Float(0x68295b02), SkBits2Float(0x60f7f28b), SkBits2Float(0x00000000), SkBits2Float(0x6a2a291f), SkBits2Float(0x42784f5a));  // 3.19903e+24f, 1.42932e+20f, 0, 5.14279e+25f, 62.0775f
path.close();
path.moveTo(SkBits2Float(0x654d6d10), SkBits2Float(0x00000000));  // 6.06311e+22f, 0
path.lineTo(SkBits2Float(0x6a4b7bc0), SkBits2Float(0x00000000));  // 6.14991e+25f, 0
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0x6a4b7bc0));  // 0, 6.14991e+25f
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x3ac23a55), SkBits2Float(0x2a292827));  // 0.00148184f, 1.50241e-13f
path.lineTo(SkBits2Float(0x63962be6), SkBits2Float(0x272a812a));  // 5.54035e+21f, 2.36623e-15f

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 0, filename);
}

static void fuzz763_7(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x68556829), SkBits2Float(0x555b2d29));  // 4.03114e+24f, 1.50617e+13f
path.moveTo(SkBits2Float(0x0f2a312a), SkBits2Float(0xc0032108));  // 8.39112e-30f, -2.04889f
path.cubicTo(SkBits2Float(0x68392d55), SkBits2Float(0xf05b684b), SkBits2Float(0x8c55272d), SkBits2Float(0x212a1f2a), SkBits2Float(0x0321082a), SkBits2Float(0x6a4b7bc0));  // 3.4979e+24f, -2.71613e+29f, -1.64207e-31f, 5.76395e-19f, 4.7323e-37f, 6.14991e+25f
path.conicTo(SkBits2Float(0x212a8ced), SkBits2Float(0x0321081f), SkBits2Float(0x6a4b7bc0), SkBits2Float(0x2829ed84), SkBits2Float(0x2d555b2d));  // 5.77848e-19f, 4.7323e-37f, 6.14991e+25f, 9.43289e-15f, 1.21279e-11f
path.moveTo(SkBits2Float(0x68345b2d), SkBits2Float(0xf0682955));  // 3.40683e+24f, -2.87402e+29f
path.conicTo(SkBits2Float(0x212a1f5b), SkBits2Float(0xef2a8c55), SkBits2Float(0x295b2d2a), SkBits2Float(0x08685568), SkBits2Float(0x7bc00321));  // 5.76397e-19f, -5.27821e+28f, 4.86669e-14f, 6.99154e-34f, 1.99397e+36f
path.lineTo(SkBits2Float(0x68345b2d), SkBits2Float(0xf0682955));  // 3.40683e+24f, -2.87402e+29f
path.close();
path.moveTo(SkBits2Float(0x68345b2d), SkBits2Float(0xf0682955));  // 3.40683e+24f, -2.87402e+29f
path.lineTo(SkBits2Float(0x5b2c6829), SkBits2Float(0x212a8c55));  // 4.85282e+16f, 5.7784e-19f
path.moveTo(SkBits2Float(0x0321081f), SkBits2Float(0x6a4b7bc0));  // 4.7323e-37f, 6.14991e+25f
path.lineTo(SkBits2Float(0x3a8a3adf), SkBits2Float(0x8a281a4f));  // 0.00105461f, -8.09385e-33f
path.quadTo(SkBits2Float(0x1d2a2928), SkBits2Float(0x43962be6), SkBits2Float(0x272a812a), SkBits2Float(0x3a2a5529));  // 2.25206e-21f, 300.343f, 2.36623e-15f, 0.000649768f
path.lineTo(SkBits2Float(0x213b1e2a), SkBits2Float(0x27292720));  // 6.3398e-19f, 2.34747e-15f
path.conicTo(SkBits2Float(0xba1f203a), SkBits2Float(0xc422c538), SkBits2Float(0x215d5927), SkBits2Float(0x70ec2ac2), SkBits2Float(0x2a51523a));  // -0.000607017f, -651.082f, 7.49957e-19f, 5.84721e+29f, 1.85915e-13f
path.quadTo(SkBits2Float(0x633ad912), SkBits2Float(0x29c80927), SkBits2Float(0x272927b0), SkBits2Float(0x683a5b2d));  // 3.44674e+21f, 8.88337e-14f, 2.3475e-15f, 3.52017e+24f
path.lineTo(SkBits2Float(0x295b2d68), SkBits2Float(0x29685568));  // 4.86672e-14f, 5.15884e-14f
path.conicTo(SkBits2Float(0xaa8c555b), SkBits2Float(0x081f2a21), SkBits2Float(0x5b2d0321), SkBits2Float(0x68556829), SkBits2Float(0x2a552d29));  // -2.49282e-13f, 4.78968e-34f, 4.86986e+16f, 4.03114e+24f, 1.89339e-13f
path.cubicTo(SkBits2Float(0x21295b2d), SkBits2Float(0x2a688c5b), SkBits2Float(0x68295b2d), SkBits2Float(0x2d296855), SkBits2Float(0x8c08555b), SkBits2Float(0x2a2a29ca));  // 5.73801e-19f, 2.06544e-13f, 3.19905e+24f, 9.6297e-12f, -1.05027e-31f, 1.51135e-13f
path.quadTo(SkBits2Float(0x68295b21), SkBits2Float(0x2d296855), SkBits2Float(0x2a8c555b), SkBits2Float(0x081f2a21));  // 3.19904e+24f, 9.6297e-12f, 2.49282e-13f, 4.78968e-34f
path.lineTo(SkBits2Float(0x0321081f), SkBits2Float(0x6a4b7bc0));  // 4.7323e-37f, 6.14991e+25f
path.close();
path.moveTo(SkBits2Float(0x0321081f), SkBits2Float(0x6a4b7bc0));  // 4.7323e-37f, 6.14991e+25f
path.conicTo(SkBits2Float(0x6a4b7bc0), SkBits2Float(0x5b2d6829), SkBits2Float(0x212a8c55), SkBits2Float(0xed7aba1f), SkBits2Float(0x2a212a8c));  // 6.14991e+25f, 4.88097e+16f, 5.7784e-19f, -4.84977e+27f, 1.43144e-13f
path.moveTo(SkBits2Float(0x2d212d08), SkBits2Float(0x5568295b));  // 9.16179e-12f, 1.5954e+13f
path.moveTo(SkBits2Float(0x5529685b), SkBits2Float(0x11295b68));  // 1.16416e+13f, 1.33599e-28f
path.conicTo(SkBits2Float(0x5b782968), SkBits2Float(0x3a292d55), SkBits2Float(0x2a8c555b), SkBits2Float(0x68295a2d), SkBits2Float(0x2d296855));  // 6.98513e+16f, 0.000645359f, 2.49282e-13f, 3.19897e+24f, 9.6297e-12f
path.moveTo(SkBits2Float(0x555b8c55), SkBits2Float(0x21682929));  // 1.50872e+13f, 7.86591e-19f
path.moveTo(SkBits2Float(0x0321081f), SkBits2Float(0x6a4b7bc0));  // 4.7323e-37f, 6.14991e+25f
path.conicTo(SkBits2Float(0xac2d8ced), SkBits2Float(0x5b682968), SkBits2Float(0x5b292d55), SkBits2Float(0x212a8c55), SkBits2Float(0x081f282a));  // -2.4663e-12f, 6.53477e+16f, 4.76191e+16f, 5.7784e-19f, 4.78945e-34f
path.lineTo(SkBits2Float(0x0321081f), SkBits2Float(0x6a4b7bc0));  // 4.7323e-37f, 6.14991e+25f
path.close();
path.moveTo(SkBits2Float(0x0321081f), SkBits2Float(0x6a4b7bc0));  // 4.7323e-37f, 6.14991e+25f
path.conicTo(SkBits2Float(0x6a4b7bc0), SkBits2Float(0x2a8ced7a), SkBits2Float(0x03081f21), SkBits2Float(0x6a3a7bc0), SkBits2Float(0x2147ed7a));  // 6.14991e+25f, 2.50338e-13f, 4.00025e-37f, 5.63611e+25f, 6.77381e-19f
path.lineTo(SkBits2Float(0x0321081f), SkBits2Float(0x6a4b7bc0));  // 4.7323e-37f, 6.14991e+25f
path.close();
path.moveTo(SkBits2Float(0x0321081f), SkBits2Float(0x6a4b7bc0));  // 4.7323e-37f, 6.14991e+25f
path.quadTo(SkBits2Float(0x2d28282a), SkBits2Float(0x5568295b), SkBits2Float(0x3a21df68), SkBits2Float(0x4f9a3a8a));  // 9.55861e-12f, 1.5954e+13f, 0.000617495f, 5.17506e+09f
path.lineTo(SkBits2Float(0x0321081f), SkBits2Float(0x6a4b7bc0));  // 4.7323e-37f, 6.14991e+25f
path.close();
path.moveTo(SkBits2Float(0x0321081f), SkBits2Float(0x6a4b7bc0));  // 4.7323e-37f, 6.14991e+25f
path.cubicTo(SkBits2Float(0x5568c23a), SkBits2Float(0x5b2d2968), SkBits2Float(0x212a8c55), SkBits2Float(0x21081f2a), SkBits2Float(0x3a7bc003), SkBits2Float(0x294b2827));  // 1.59951e+13f, 4.87407e+16f, 5.7784e-19f, 4.61198e-19f, 0.00096035f, 4.51099e-14f

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 0, filename);
}

static void kfuzz2(skiatest::Reporter* reporter, const char* filename) {
    SkPath path1;
    SkPath path;
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xfafadbfa));  // 0, -6.51268e+35f
path.close();
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xfafadbfa));  // 0, -6.51268e+35f
path.cubicTo(SkBits2Float(0xe3000000), SkBits2Float(0xf19e92c7), SkBits2Float(0xf17febcb), SkBits2Float(0xff7febcb), SkBits2Float(0x60600100), SkBits2Float(0x0100ff60));  // -2.36118e+21f, -1.57043e+30f, -1.26726e+30f, -3.40177e+38f, 6.45647e+19f, 2.36931e-38f
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xfafadbfa));  // 0, -6.51268e+35f
path.close();
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0xfafadbfa));  // 0, -6.51268e+35f
path.lineTo(SkBits2Float(0x60601a1d), SkBits2Float(0x60606060));  // 6.4593e+19f, 6.46721e+19f
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xfafadbfa));  // 0, -6.51268e+35f
path.close();
path.moveTo(SkBits2Float(0xe5e2f300), SkBits2Float(0xee244a40));  // -1.33967e+23f, -1.27113e+28f
path.moveTo(SkBits2Float(0xba98ffee), SkBits2Float(0xfafafa1a));  // -0.0011673f, -6.51573e+35f
path.close();
path.moveTo(SkBits2Float(0xba98ffee), SkBits2Float(0xfafafa1a));  // -0.0011673f, -6.51573e+35f
path.lineTo(SkBits2Float(0xfafafafa), SkBits2Float(0xe30000fa));  // -6.51582e+35f, -2.36125e+21f
path.conicTo(SkBits2Float(0x92e592e5), SkBits2Float(0xfafafafb), SkBits2Float(0xc4fa0000), SkBits2Float(0x6060fafa), SkBits2Float(0x60606060));  // -1.44881e-27f, -6.51582e+35f, -2000, 6.48462e+19f, 6.46721e+19f
path.lineTo(SkBits2Float(0xba98ffee), SkBits2Float(0xfafafa1a));  // -0.0011673f, -6.51573e+35f
path.close();
path.moveTo(SkBits2Float(0xba98ffee), SkBits2Float(0xfafafa1a));  // -0.0011673f, -6.51573e+35f
path.cubicTo(SkBits2Float(0xe3000000), SkBits2Float(0xf19e92c7), SkBits2Float(0xf17febcb), SkBits2Float(0xff7febcb), SkBits2Float(0xfafafa00), SkBits2Float(0xfafafafa));  // -2.36118e+21f, -1.57043e+30f, -1.26726e+30f, -3.40177e+38f, -6.51572e+35f, -6.51582e+35f
path.lineTo(SkBits2Float(0xba98ffee), SkBits2Float(0xfafafa1a));  // -0.0011673f, -6.51573e+35f
path.close();
path.moveTo(SkBits2Float(0xba98ffee), SkBits2Float(0xfafafa1a));  // -0.0011673f, -6.51573e+35f
path.cubicTo(SkBits2Float(0xe3000000), SkBits2Float(0xe39e92c7), SkBits2Float(0xf17febcb), SkBits2Float(0xff7febcb), SkBits2Float(0xeed0ee9a), SkBits2Float(0x9a98ffca));  // -2.36118e+21f, -5.85032e+21f, -1.26726e+30f, -3.40177e+38f, -3.23307e+28f, -6.3279e-23f
path.lineTo(SkBits2Float(0xba98ffee), SkBits2Float(0xfafafa1a));  // -0.0011673f, -6.51573e+35f
path.close();
SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, kXOR_SkPathOp, filename);
}

static void fuzz763_10(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x68556829));  // 0, 4.03114e+24f
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.quadTo(SkBits2Float(0x6a4b7bc0), SkBits2Float(0x00000000), SkBits2Float(0x00000000), SkBits2Float(0x6a4b7bc4));  // 6.14991e+25f, 0, 0, 6.14991e+25f
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0x68556829));  // 0, 4.03114e+24f
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.lineTo(SkBits2Float(0x5b2d2968), SkBits2Float(0x2a8c8f55));  // 4.87407e+16f, 2.49685e-13f
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.close();
path.moveTo(SkBits2Float(0xc021211f), SkBits2Float(0x6a4b7b03));  // -2.51765f, 6.14982e+25f
path.conicTo(SkBits2Float(0x682d2fed), SkBits2Float(0x755b6829), SkBits2Float(0x5b292d2b), SkBits2Float(0xc92a8c55), SkBits2Float(0x081f2a21));  // 3.27141e+24f, 2.78131e+32f, 4.76189e+16f, -698565, 4.78968e-34f
path.lineTo(SkBits2Float(0xc021211f), SkBits2Float(0x6a4b7b03));  // -2.51765f, 6.14982e+25f
path.close();
path.moveTo(SkBits2Float(0xc021211f), SkBits2Float(0x6a4b7b03));  // -2.51765f, 6.14982e+25f
path.conicTo(SkBits2Float(0x6a4b7bc0), SkBits2Float(0x2a8ced7a), SkBits2Float(0x21081f21), SkBits2Float(0x3a7bc003), SkBits2Float(0x47ed7a29));  // 6.14991e+25f, 2.50338e-13f, 4.61198e-19f, 0.00096035f, 121588
path.lineTo(SkBits2Float(0xc021211f), SkBits2Float(0x6a4b7b03));  // -2.51765f, 6.14982e+25f
path.close();
path.moveTo(SkBits2Float(0xc021211f), SkBits2Float(0x6a4b7b03));  // -2.51765f, 6.14982e+25f
path.quadTo(SkBits2Float(0x6829682d), SkBits2Float(0x292d555b), SkBits2Float(0x2a8c555b), SkBits2Float(0x081f2a29));  // 3.20001e+24f, 3.84878e-14f, 2.49282e-13f, 4.78969e-34f
path.conicTo(SkBits2Float(0x6a497b19), SkBits2Float(0x218ced7a), SkBits2Float(0x0321081f), SkBits2Float(0x6a3a7bc0), SkBits2Float(0x47ed3a7a));  // 6.08939e+25f, 9.54963e-19f, 4.7323e-37f, 5.63611e+25f, 121461
path.lineTo(SkBits2Float(0xc021211f), SkBits2Float(0x6a4b7b03));  // -2.51765f, 6.14982e+25f
path.close();
path.moveTo(SkBits2Float(0xc021211f), SkBits2Float(0x6a4b7b03));  // -2.51765f, 6.14982e+25f
path.quadTo(SkBits2Float(0x282a282a), SkBits2Float(0x8a3a21df), SkBits2Float(0x2728282a), SkBits2Float(0x8a3a2129));  // 9.4456e-15f, -8.96194e-33f, 2.33365e-15f, -8.96181e-33f
path.quadTo(SkBits2Float(0x8a284f9a), SkBits2Float(0x3a3ac2b3), SkBits2Float(0x2a292827), SkBits2Float(0x962be61d));  // -8.10388e-33f, 0.000712435f, 1.50241e-13f, -1.38859e-25f
path.lineTo(SkBits2Float(0x272a802a), SkBits2Float(0x2a8c2d29));  // 2.36617e-15f, 2.49003e-13f
path.lineTo(SkBits2Float(0xc021211f), SkBits2Float(0x6a4b7b03));  // -2.51765f, 6.14982e+25f
path.close();
path.moveTo(SkBits2Float(0x4f9a3a29), SkBits2Float(0x3ab38a28));  // 5.17501e+09f, 0.00136978f
path.quadTo(SkBits2Float(0xc368305b), SkBits2Float(0x5b296855), SkBits2Float(0x2d8c5568), SkBits2Float(0x1f2a2172));  // -232.189f, 4.7684e+16f, 1.59541e-11f, 3.60266e-20f
path.lineTo(SkBits2Float(0x29c00321), SkBits2Float(0x5b4b7b13));  // 8.52706e-14f, 5.72747e+16f

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 4, filename);
}

static void fuzz763_11(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x68556829), SkBits2Float(0x555b2d29));  // 4.03114e+24f, 1.50617e+13f
path.moveTo(SkBits2Float(0x2a0f312a), SkBits2Float(0xc0032108));  // 1.2718e-13f, -2.04889f
path.cubicTo(SkBits2Float(0x68392d55), SkBits2Float(0xf05b684b), SkBits2Float(0x8c55272d), SkBits2Float(0x212a1f2a), SkBits2Float(0x0321082a), SkBits2Float(0x6a4b7bc0));  // 3.4979e+24f, -2.71613e+29f, -1.64207e-31f, 5.76395e-19f, 4.7323e-37f, 6.14991e+25f
path.conicTo(SkBits2Float(0x212a8ced), SkBits2Float(0x0321081f), SkBits2Float(0x6a4b7b21), SkBits2Float(0x2829ed84), SkBits2Float(0x2d555b2d));  // 5.77848e-19f, 4.7323e-37f, 6.14984e+25f, 9.43289e-15f, 1.21279e-11f
path.moveTo(SkBits2Float(0x68385b2d), SkBits2Float(0x70682955));  // 3.48239e+24f, 2.87402e+29f
path.conicTo(SkBits2Float(0x212a1f5b), SkBits2Float(0xef2a8c55), SkBits2Float(0x295b2d2a), SkBits2Float(0x08685568), SkBits2Float(0x7bc00321));  // 5.76397e-19f, -5.27821e+28f, 4.86669e-14f, 6.99154e-34f, 1.99397e+36f
path.lineTo(SkBits2Float(0x68385b2d), SkBits2Float(0x70682955));  // 3.48239e+24f, 2.87402e+29f
path.close();
path.moveTo(SkBits2Float(0x68385b2d), SkBits2Float(0x70682955));  // 3.48239e+24f, 2.87402e+29f
path.lineTo(SkBits2Float(0x5b2c6829), SkBits2Float(0x212a8c55));  // 4.85282e+16f, 5.7784e-19f
path.moveTo(SkBits2Float(0x0321081f), SkBits2Float(0x6a4b7bc0));  // 4.7323e-37f, 6.14991e+25f
path.lineTo(SkBits2Float(0x3a8a3adf), SkBits2Float(0x8a281a4f));  // 0.00105461f, -8.09385e-33f
path.quadTo(SkBits2Float(0x1d2a2928), SkBits2Float(0x43962be6), SkBits2Float(0x2a812a3b), SkBits2Float(0x2a552927));  // 2.25206e-21f, 300.343f, 2.29443e-13f, 1.89325e-13f
path.quadTo(SkBits2Float(0x3b1e2ab0), SkBits2Float(0x29272021), SkBits2Float(0x203a3b27), SkBits2Float(0x22c5381f));  // 0.00241343f, 3.71093e-14f, 1.57744e-19f, 5.34564e-18f
path.moveTo(SkBits2Float(0x5d27ec2a), SkBits2Float(0x705921c2));  // 7.56256e+17f, 2.68796e+29f
path.quadTo(SkBits2Float(0x102a5152), SkBits2Float(0x5b2dd912), SkBits2Float(0x68556829), SkBits2Float(0x555b2d29));  // 3.35892e-29f, 4.89338e+16f, 4.03114e+24f, 1.50617e+13f
path.moveTo(SkBits2Float(0x1f2a312a), SkBits2Float(0xc0032127));  // 3.60396e-20f, -2.0489f
path.cubicTo(SkBits2Float(0x68392d55), SkBits2Float(0x2a8c684b), SkBits2Float(0xf05b272d), SkBits2Float(0x2a1f1555), SkBits2Float(0x21082a21), SkBits2Float(0x6a4b7b03));  // 3.4979e+24f, 2.49414e-13f, -2.71298e+29f, 1.41294e-13f, 4.61343e-19f, 6.14982e+25f
path.conicTo(SkBits2Float(0x212a8ced), SkBits2Float(0x0321081f), SkBits2Float(0x6a4b7bc0), SkBits2Float(0x2829ed84), SkBits2Float(0x2d555b2d));  // 5.77848e-19f, 4.7323e-37f, 6.14991e+25f, 9.43289e-15f, 1.21279e-11f
path.moveTo(SkBits2Float(0x2a395b2d), SkBits2Float(0xf0682955));  // 1.64629e-13f, -2.87402e+29f
path.conicTo(SkBits2Float(0x212a1f5b), SkBits2Float(0xef2a8c55), SkBits2Float(0x295b2d2a), SkBits2Float(0x68210368), SkBits2Float(0x7bc05508));  // 5.76397e-19f, -5.27821e+28f, 4.86669e-14f, 3.04146e+24f, 1.99729e+36f
path.lineTo(SkBits2Float(0x2a395b2d), SkBits2Float(0xf0682955));  // 1.64629e-13f, -2.87402e+29f
path.close();
path.moveTo(SkBits2Float(0x2a395b2d), SkBits2Float(0xf0682955));  // 1.64629e-13f, -2.87402e+29f
path.lineTo(SkBits2Float(0x5b2c6829), SkBits2Float(0x2a21211f));  // 4.85282e+16f, 1.43112e-13f
path.lineTo(SkBits2Float(0x03552a8c), SkBits2Float(0x6a4f7b28));  // 6.26439e-37f, 6.27073e+25f
path.conicTo(SkBits2Float(0x2347ed93), SkBits2Float(0x282a3a21), SkBits2Float(0x3adf2128), SkBits2Float(0x4f1a3a8a), SkBits2Float(0x3ab38a28));  // 1.08381e-17f, 9.4495e-15f, 0.00170234f, 2.58753e+09f, 0.00136978f
path.lineTo(SkBits2Float(0x2a395b2d), SkBits2Float(0xf0682955));  // 1.64629e-13f, -2.87402e+29f
path.close();
path.moveTo(SkBits2Float(0x2a395b2d), SkBits2Float(0xf0682955));  // 1.64629e-13f, -2.87402e+29f
path.quadTo(SkBits2Float(0x1d2a2928), SkBits2Float(0x43962be6), SkBits2Float(0x262a812a), SkBits2Float(0x3a2a5529));  // 2.25206e-21f, 300.343f, 5.91556e-16f, 0.000649768f
path.lineTo(SkBits2Float(0x213b1e2a), SkBits2Float(0x27292720));  // 6.3398e-19f, 2.34747e-15f
path.conicTo(SkBits2Float(0x371f203a), SkBits2Float(0xc52a22c4), SkBits2Float(0xc25d27ec), SkBits2Float(0x3a705921), SkBits2Float(0x5210513a));  // 9.48464e-06f, -2722.17f, -55.289f, 0.000916855f, 1.5496e+11f
path.cubicTo(SkBits2Float(0x63102ad9), SkBits2Float(0x29c80927), SkBits2Float(0x633a27b0), SkBits2Float(0x2909c827), SkBits2Float(0x272927b1), SkBits2Float(0x3a685b2d));  // 2.65942e+21f, 8.88337e-14f, 3.43395e+21f, 3.05937e-14f, 2.3475e-15f, 0.000886368f
path.moveTo(SkBits2Float(0x682d6829), SkBits2Float(0x29685555));  // 3.27556e+24f, 5.15884e-14f
path.conicTo(SkBits2Float(0xaa8c555b), SkBits2Float(0x081f2a21), SkBits2Float(0x5b2d0321), SkBits2Float(0x68556829), SkBits2Float(0x5b2d2729));  // -2.49282e-13f, 4.78968e-34f, 4.86986e+16f, 4.03114e+24f, 4.87382e+16f
path.quadTo(SkBits2Float(0x2d685568), SkBits2Float(0x5568295b), SkBits2Float(0x2a552d29), SkBits2Float(0x295b2d27));  // 1.32066e-11f, 1.5954e+13f, 1.89339e-13f, 4.86669e-14f
path.lineTo(SkBits2Float(0x682d6829), SkBits2Float(0x29685555));  // 3.27556e+24f, 5.15884e-14f
path.close();

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 0, filename);
}

static void fuzz763_12(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0x6a29082a));  // 0, 5.10868e+25f
path.conicTo(SkBits2Float(0x6a295ac3), SkBits2Float(0x61bb988e), SkBits2Float(0x6829682d), SkBits2Float(0x5f3ba76a), SkBits2Float(0x42730a87));  // 5.11843e+25f, 4.32567e+20f, 3.20001e+24f, 1.35219e+19f, 60.7603f
path.conicTo(SkBits2Float(0x67aedf99), SkBits2Float(0x00000000), SkBits2Float(0x00000000), SkBits2Float(0x00000000), SkBits2Float(0x3f801112));  // 1.65163e+24f, 0, 0, 0, 1.00052f
path.close();
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.conicTo(SkBits2Float(0x6a4b7bc0), SkBits2Float(0x00000000), SkBits2Float(0x00000000), SkBits2Float(0x68556829), SkBits2Float(0x555b2d29));  // 6.14991e+25f, 0, 0, 4.03114e+24f, 1.50617e+13f
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0x68555b2a));  // 0, 4.03018e+24f
path.cubicTo(SkBits2Float(0x00000000), SkBits2Float(0x67d55b2a), SkBits2Float(0x67296a4b), SkBits2Float(0x67555b2a), SkBits2Float(0x677e1f70), SkBits2Float(0x66d55b2a));  // 0, 2.01509e+24f, 8.00041e+23f, 1.00755e+24f, 1.20006e+24f, 5.03773e+23f
path.cubicTo(SkBits2Float(0x678f0684), SkBits2Float(0x6684f008), SkBits2Float(0x6798f8ea), SkBits2Float(0x6625a942), SkBits2Float(0x67961914), SkBits2Float(0x65ce709a));  // 1.35084e+24f, 3.1389e+23f, 1.44478e+24f, 1.95578e+23f, 1.41764e+24f, 1.21861e+23f
path.cubicTo(SkBits2Float(0x679158b0), SkBits2Float(0x00000000), SkBits2Float(0x67531e34), SkBits2Float(0x00000000), SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 1.37276e+24f, 0, 9.96976e+23f, 0, 0, 0
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.conicTo(SkBits2Float(0x21081f21), SkBits2Float(0x4b7bc003), SkBits2Float(0xed237a6a), SkBits2Float(0x2d682967), SkBits2Float(0x2a8c555b));  // 4.61198e-19f, 1.64987e+07f, -3.16213e+27f, 1.31969e-11f, 2.49282e-13f
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.close();
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.lineTo(SkBits2Float(0x3a6821df), SkBits2Float(0x2a8c3a8a));  // 0.000885514f, 2.49096e-13f
path.moveTo(SkBits2Float(0x29272a1d), SkBits2Float(0xb03a2a55));  // 3.7118e-14f, -6.77266e-10f
path.moveTo(SkBits2Float(0x20213b1e), SkBits2Float(0xc5272927));  // 1.36568e-19f, -2674.57f
path.quadTo(SkBits2Float(0xc422373b), SkBits2Float(0xec2a201f), SkBits2Float(0x21c25d27), SkBits2Float(0x523a7059));  // -648.863f, -8.22676e+26f, 1.31706e-18f, 2.00187e+11f
path.cubicTo(SkBits2Float(0x12102a10), SkBits2Float(0xe73a28d9), SkBits2Float(0xc8092763), SkBits2Float(0x2927b029), SkBits2Float(0x295b2d27), SkBits2Float(0x2d685568));  // 4.54902e-28f, -8.79114e+23f, -140446, 3.72342e-14f, 4.86669e-14f, 1.32066e-11f
path.moveTo(SkBits2Float(0x68556809), SkBits2Float(0x555b2d29));  // 4.03113e+24f, 1.50617e+13f
path.moveTo(SkBits2Float(0x1f2a212a), SkBits2Float(0x2d032108));  // 3.60263e-20f, 7.45382e-12f
path.moveTo(SkBits2Float(0x68556829), SkBits2Float(0x2a552d29));  // 4.03114e+24f, 1.89339e-13f
path.cubicTo(SkBits2Float(0x21295b2d), SkBits2Float(0x2a528c5b), SkBits2Float(0x284f5b2d), SkBits2Float(0x218aa621), SkBits2Float(0x3f2d2db3), SkBits2Float(0x68293a2a));  // 5.73801e-19f, 1.87004e-13f, 1.15106e-14f, 9.39522e-19f, 0.676479f, 3.19661e+24f

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 0, filename);
}

static void fuzz763_13(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x212a8c55), SkBits2Float(0x21081f2a));  // 5.7784e-19f, 4.61198e-19f
path.conicTo(SkBits2Float(0x6a4b7bc0), SkBits2Float(0x4793ed7a), SkBits2Float(0x282a3a21), SkBits2Float(0x3adf2128), SkBits2Float(0x4f1a3a8a));  // 6.14991e+25f, 75739, 9.4495e-15f, 0.00170234f, 2.58753e+09f
path.lineTo(SkBits2Float(0x212a8c55), SkBits2Float(0x21081f2a));  // 5.7784e-19f, 4.61198e-19f
path.close();
path.moveTo(SkBits2Float(0x212a8c55), SkBits2Float(0x21081f2a));  // 5.7784e-19f, 4.61198e-19f
path.cubicTo(SkBits2Float(0x3ac2213a), SkBits2Float(0x1d2a2928), SkBits2Float(0x43962be6), SkBits2Float(0x272a8128), SkBits2Float(0x3a2a5529), SkBits2Float(0x3b1e2ab0));  // 0.00148109f, 2.25206e-21f, 300.343f, 2.36623e-15f, 0.000649768f, 0.00241343f
path.lineTo(SkBits2Float(0x212a8c55), SkBits2Float(0x21081f2a));  // 5.7784e-19f, 4.61198e-19f
path.close();
path.moveTo(SkBits2Float(0x212a8c55), SkBits2Float(0x21081f2a));  // 5.7784e-19f, 4.61198e-19f
path.cubicTo(SkBits2Float(0x3b272927), SkBits2Float(0x381f203a), SkBits2Float(0x2ac422c5), SkBits2Float(0xc25d27ec), SkBits2Float(0x3a705921), SkBits2Float(0x2a105152));  // 0.00255067f, 3.79386e-05f, 3.48407e-13f, -55.289f, 0.000916855f, 1.2818e-13f
path.quadTo(SkBits2Float(0x633ad912), SkBits2Float(0x29c80927), SkBits2Float(0x272927b0), SkBits2Float(0x68295b2d));  // 3.44674e+21f, 8.88337e-14f, 2.3475e-15f, 3.19905e+24f
path.lineTo(SkBits2Float(0x295b2d68), SkBits2Float(0x29685568));  // 4.86672e-14f, 5.15884e-14f
path.conicTo(SkBits2Float(0xaa8c555b), SkBits2Float(0x081f2a21), SkBits2Float(0x5b2d0321), SkBits2Float(0x68556829), SkBits2Float(0x2a552d29));  // -2.49282e-13f, 4.78968e-34f, 4.86986e+16f, 4.03114e+24f, 1.89339e-13f
path.cubicTo(SkBits2Float(0x21295b2d), SkBits2Float(0x2a688c5b), SkBits2Float(0x6829292d), SkBits2Float(0x2d296855), SkBits2Float(0x8c08555b), SkBits2Float(0x2a2a291f));  // 5.73801e-19f, 2.06544e-13f, 3.19536e+24f, 9.6297e-12f, -1.05027e-31f, 1.51133e-13f
path.conicTo(SkBits2Float(0x68295b21), SkBits2Float(0x2d296855), SkBits2Float(0x2a8c555b), SkBits2Float(0x081f2a21), SkBits2Float(0x7bc00321));  // 3.19904e+24f, 9.6297e-12f, 2.49282e-13f, 4.78968e-34f, 1.99397e+36f
path.lineTo(SkBits2Float(0x212a8c55), SkBits2Float(0x21081f2a));  // 5.7784e-19f, 4.61198e-19f
path.close();
path.moveTo(SkBits2Float(0x212a8c55), SkBits2Float(0x21081f2a));  // 5.7784e-19f, 4.61198e-19f
path.lineTo(SkBits2Float(0x5b2d6829), SkBits2Float(0x212a8c55));  // 4.88097e+16f, 5.7784e-19f
path.conicTo(SkBits2Float(0x8ced7aba), SkBits2Float(0x3f2a212a), SkBits2Float(0x2d212d08), SkBits2Float(0x5568295b), SkBits2Float(0x29685b2d));  // -3.65895e-31f, 0.664569f, 9.16179e-12f, 1.5954e+13f, 5.15934e-14f
path.lineTo(SkBits2Float(0x68295b68), SkBits2Float(0x2d296855));  // 3.19906e+24f, 9.6297e-12f
path.moveTo(SkBits2Float(0x212a8c55), SkBits2Float(0x21081f2a));  // 5.7784e-19f, 4.61198e-19f
path.conicTo(SkBits2Float(0x6a4b7bc0), SkBits2Float(0x2a8ced7a), SkBits2Float(0x21081f21), SkBits2Float(0x6aba7b03), SkBits2Float(0x2147ed7a));  // 6.14991e+25f, 2.50338e-13f, 4.61198e-19f, 1.12721e+26f, 6.77381e-19f
path.quadTo(SkBits2Float(0x6028282a), SkBits2Float(0x68292ddf), SkBits2Float(0x5b2d555b), SkBits2Float(0x68556829));  // 4.84679e+19f, 3.1957e+24f, 4.8789e+16f, 4.03114e+24f

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 4, filename);
}

static void fuzz763_14(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x68556829), SkBits2Float(0xf45b2d29));  // 4.03114e+24f, -6.94598e+31f
path.moveTo(SkBits2Float(0x1f2a302a), SkBits2Float(0xc8032108));  // 3.60387e-20f, -134276
path.cubicTo(SkBits2Float(0x68392d55), SkBits2Float(0xf0db684b), SkBits2Float(0x8c55272d), SkBits2Float(0x212a292a), SkBits2Float(0x302a5b25), SkBits2Float(0xf0685568));  // 3.4979e+24f, -5.43226e+29f, -1.64207e-31f, 5.76527e-19f, 6.19752e-10f, -2.87615e+29f

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 0, filename);
}

static void fuzz763_15(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x5b292d55), SkBits2Float(0x212a8c55));  // 4.76191e+16f, 5.7784e-19f
path.moveTo(SkBits2Float(0x0321081f), SkBits2Float(0x6a6b7bc4));  // 4.7323e-37f, 7.11705e+25f
path.conicTo(SkBits2Float(0x212a8ced), SkBits2Float(0x0321081f), SkBits2Float(0x2c6829c0), SkBits2Float(0x2a8c555b), SkBits2Float(0x081f2a29));  // 5.77848e-19f, 4.7323e-37f, 3.29924e-12f, 2.49282e-13f, 4.78969e-34f
path.lineTo(SkBits2Float(0x0321081f), SkBits2Float(0x6a6b7bc4));  // 4.7323e-37f, 7.11705e+25f
path.close();
path.moveTo(SkBits2Float(0x0321081f), SkBits2Float(0x6a6b7bc4));  // 4.7323e-37f, 7.11705e+25f
path.conicTo(SkBits2Float(0x6a4b7bc0), SkBits2Float(0x4793ed7a), SkBits2Float(0x282a3a21), SkBits2Float(0xdf218a28), SkBits2Float(0x4f1a3a3a));  // 6.14991e+25f, 75739, 9.4495e-15f, -1.16402e+19f, 2.58751e+09f
path.quadTo(SkBits2Float(0x3ab38a28), SkBits2Float(0x283ac221), SkBits2Float(0xe6432a29), SkBits2Float(0x2a96812b));  // 0.00136978f, 1.03672e-14f, -2.3041e+23f, 2.6735e-13f
path.lineTo(SkBits2Float(0x5529272a), SkBits2Float(0x1eb03a2a));  // 1.16241e+13f, 1.86588e-20f
path.conicTo(SkBits2Float(0x2a272021), SkBits2Float(0x3ac52729), SkBits2Float(0xc422313b), SkBits2Float(0xec2a201f), SkBits2Float(0x21c25d27));  // 1.48437e-13f, 0.00150416f, -648.769f, -8.22676e+26f, 1.31706e-18f
path.lineTo(SkBits2Float(0x0321081f), SkBits2Float(0x6a6b7bc4));  // 4.7323e-37f, 7.11705e+25f
path.close();
path.moveTo(SkBits2Float(0x1051523a), SkBits2Float(0xd912102a));  // 4.12813e-29f, -2.56957e+15f
path.close();
path.moveTo(SkBits2Float(0x1051523a), SkBits2Float(0xd912102a));  // 4.12813e-29f, -2.56957e+15f
path.quadTo(SkBits2Float(0xc82763e7), SkBits2Float(0x2927b029), SkBits2Float(0x295b2d27), SkBits2Float(0x2d685568));  // -171408, 3.72342e-14f, 4.86669e-14f, 1.32066e-11f
path.moveTo(SkBits2Float(0x68556809), SkBits2Float(0x8c555b2d));  // 4.03113e+24f, -1.64364e-31f
path.moveTo(SkBits2Float(0x081f2a21), SkBits2Float(0x252d0321));  // 4.78968e-34f, 1.50064e-16f
path.moveTo(SkBits2Float(0x5568392a), SkBits2Float(0x5b2df068));  // 1.59583e+13f, 4.89595e+16f
path.quadTo(SkBits2Float(0x2a1f2a8c), SkBits2Float(0x21482a21), SkBits2Float(0x4b7bc003), SkBits2Float(0x8ced3a6a));  // 1.41368e-13f, 6.78184e-19f, 1.64987e+07f, -3.65508e-31f
path.moveTo(SkBits2Float(0x21481f21), SkBits2Float(0x4b7bc003));  // 6.78038e-19f, 1.64987e+07f
path.conicTo(SkBits2Float(0x6829ed27), SkBits2Float(0x2d155b2d), SkBits2Float(0x5568295b), SkBits2Float(0x5b2d2968), SkBits2Float(0x2a8c8f55));  // 3.20982e+24f, 8.48991e-12f, 1.5954e+13f, 4.87407e+16f, 2.49685e-13f
path.lineTo(SkBits2Float(0x21481f21), SkBits2Float(0x4b7bc003));  // 6.78038e-19f, 1.64987e+07f
path.close();
path.moveTo(SkBits2Float(0xc021211f), SkBits2Float(0x6a4b7b03));  // -2.51765f, 6.14982e+25f
path.conicTo(SkBits2Float(0x682d2fed), SkBits2Float(0x755b6829), SkBits2Float(0x5b292d2b), SkBits2Float(0xc92a8c55), SkBits2Float(0x081f2a21));  // 3.27141e+24f, 2.78131e+32f, 4.76189e+16f, -698565, 4.78968e-34f
path.lineTo(SkBits2Float(0xc021211f), SkBits2Float(0x6a4b7b03));  // -2.51765f, 6.14982e+25f
path.close();
path.moveTo(SkBits2Float(0xc021211f), SkBits2Float(0x6a4b7b03));  // -2.51765f, 6.14982e+25f
path.conicTo(SkBits2Float(0x6a4b7bc0), SkBits2Float(0x212aed7a), SkBits2Float(0x0321081f), SkBits2Float(0x293a7bc0), SkBits2Float(0x2147ed7a));  // 6.14991e+25f, 5.79125e-19f, 4.7323e-37f, 4.14076e-14f, 6.77381e-19f
path.quadTo(SkBits2Float(0x6829682d), SkBits2Float(0x292d555b), SkBits2Float(0x292a8c55), SkBits2Float(0x21081f2a));  // 3.20001e+24f, 3.84878e-14f, 3.78693e-14f, 4.61198e-19f
path.conicTo(SkBits2Float(0x6a4b7bc0), SkBits2Float(0x218ced7a), SkBits2Float(0x0321081f), SkBits2Float(0x6a3a7bc0), SkBits2Float(0x47ed3a7a));  // 6.14991e+25f, 9.54963e-19f, 4.7323e-37f, 5.63611e+25f, 121461
path.lineTo(SkBits2Float(0xc021211f), SkBits2Float(0x6a4b7b03));  // -2.51765f, 6.14982e+25f
path.close();
path.moveTo(SkBits2Float(0xc021211f), SkBits2Float(0x6a4b7b03));  // -2.51765f, 6.14982e+25f
path.quadTo(SkBits2Float(0x282a282a), SkBits2Float(0x8a3a21df), SkBits2Float(0x2728282a), SkBits2Float(0x8a3a21df));  // 9.4456e-15f, -8.96194e-33f, 2.33365e-15f, -8.96194e-33f
path.quadTo(SkBits2Float(0x8a284f9a), SkBits2Float(0x3a3ac2b3), SkBits2Float(0x2a292827), SkBits2Float(0x962be61d));  // -8.10388e-33f, 0.000712435f, 1.50241e-13f, -1.38859e-25f
path.lineTo(SkBits2Float(0x272a802a), SkBits2Float(0x2a8c2d29));  // 2.36617e-15f, 2.49003e-13f
path.lineTo(SkBits2Float(0xc021211f), SkBits2Float(0x6a4b7b03));  // -2.51765f, 6.14982e+25f
path.close();
path.moveTo(SkBits2Float(0x4f9a3a29), SkBits2Float(0x3ab38a28));  // 5.17501e+09f, 0.00136978f
path.quadTo(SkBits2Float(0xc368305b), SkBits2Float(0x5b296855), SkBits2Float(0x2d8c5568), SkBits2Float(0x1f2a2172));  // -232.189f, 4.7684e+16f, 1.59541e-11f, 3.60266e-20f
path.lineTo(SkBits2Float(0x29c00321), SkBits2Float(0x5b4b7b13));  // 8.52706e-14f, 5.72747e+16f

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 4, filename);
}

static void fuzz763_16(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x68556829), SkBits2Float(0x555b2d29));  // 4.03114e+24f, 1.50617e+13f
path.moveTo(SkBits2Float(0x1f2a312a), SkBits2Float(0xc0032108));  // 3.60396e-20f, -2.04889f
path.cubicTo(SkBits2Float(0x68372d55), SkBits2Float(0xf05b684b), SkBits2Float(0x8c552775), SkBits2Float(0x212a292a), SkBits2Float(0x0321082a), SkBits2Float(0x6a4b7bc0));  // 3.46012e+24f, -2.71613e+29f, -1.64208e-31f, 5.76527e-19f, 4.7323e-37f, 6.14991e+25f
path.conicTo(SkBits2Float(0x212a8ced), SkBits2Float(0x0321081f), SkBits2Float(0x6a4b7bc0), SkBits2Float(0x2829ed84), SkBits2Float(0x69555b2d));  // 5.77848e-19f, 4.7323e-37f, 6.14991e+25f, 9.43289e-15f, 1.61207e+25f
path.moveTo(SkBits2Float(0x68315b2d), SkBits2Float(0xf0682955));  // 3.35016e+24f, -2.87402e+29f
path.conicTo(SkBits2Float(0x212a1f5b), SkBits2Float(0x8cef552a), SkBits2Float(0x295b2d2a), SkBits2Float(0x68210368), SkBits2Float(0x7bc05508));  // 5.76397e-19f, -3.6875e-31f, 4.86669e-14f, 3.04146e+24f, 1.99729e+36f
path.lineTo(SkBits2Float(0x68315b2d), SkBits2Float(0xf0682955));  // 3.35016e+24f, -2.87402e+29f
path.close();
path.moveTo(SkBits2Float(0x68315b2d), SkBits2Float(0xf0682955));  // 3.35016e+24f, -2.87402e+29f
path.lineTo(SkBits2Float(0x5b2c6829), SkBits2Float(0x212a8c55));  // 4.85282e+16f, 5.7784e-19f
path.moveTo(SkBits2Float(0x0321081f), SkBits2Float(0x6a4b7bc0));  // 4.7323e-37f, 6.14991e+25f
path.conicTo(SkBits2Float(0x68385b2d), SkBits2Float(0x555bf055), SkBits2Float(0x2a1f2a8c), SkBits2Float(0x03212a21), SkBits2Float(0x5a4b7bc0));  // 3.48239e+24f, 1.51141e+13f, 1.41368e-13f, 4.7362e-37f, 1.43189e+16f
path.conicTo(SkBits2Float(0xc08c2aed), SkBits2Float(0x211f2108), SkBits2Float(0x6a4b7b03), SkBits2Float(0x6829ed27), SkBits2Float(0x2d555b2d));  // -4.38024f, 5.3915e-19f, 6.14982e+25f, 3.20982e+24f, 1.21279e-11f
path.moveTo(SkBits2Float(0x68315b2d), SkBits2Float(0xf0685527));  // 3.35016e+24f, -2.87614e+29f
path.conicTo(SkBits2Float(0x2a8c555b), SkBits2Float(0x6e2a1f72), SkBits2Float(0x0321082a), SkBits2Float(0x6a4b7bc0), SkBits2Float(0x4793ed7a));  // 2.49282e-13f, 1.31626e+28f, 4.7323e-37f, 6.14991e+25f, 75739
path.lineTo(SkBits2Float(0x68315b2d), SkBits2Float(0xf0685527));  // 3.35016e+24f, -2.87614e+29f
path.close();
path.moveTo(SkBits2Float(0x68315b2d), SkBits2Float(0xf0685527));  // 3.35016e+24f, -2.87614e+29f
path.quadTo(SkBits2Float(0x2128282a), SkBits2Float(0x3a8a3adf), SkBits2Float(0x8a284f1a), SkBits2Float(0x2c213ab3));  // 5.69738e-19f, 0.00105461f, -8.10378e-33f, 2.29121e-12f
path.lineTo(SkBits2Float(0x68315b2d), SkBits2Float(0xf0685527));  // 3.35016e+24f, -2.87614e+29f
path.close();
path.moveTo(SkBits2Float(0x68315b2d), SkBits2Float(0xf0685527));  // 3.35016e+24f, -2.87614e+29f
path.quadTo(SkBits2Float(0x1d2a2928), SkBits2Float(0x43962be6), SkBits2Float(0x3a2a812a), SkBits2Float(0x2a8ced29));  // 2.25206e-21f, 300.343f, 0.000650423f, 2.50336e-13f
path.lineTo(SkBits2Float(0x68315b2d), SkBits2Float(0xf0685527));  // 3.35016e+24f, -2.87614e+29f
path.close();
path.moveTo(SkBits2Float(0x68315b2d), SkBits2Float(0xf0685527));  // 3.35016e+24f, -2.87614e+29f
path.conicTo(SkBits2Float(0x03210831), SkBits2Float(0x6a4b7bc0), SkBits2Float(0x681aed27), SkBits2Float(0x55555b2d), SkBits2Float(0x1e2a3a2a));  // 4.73231e-37f, 6.14991e+25f, 2.92648e+24f, 1.46617e+13f, 9.01175e-21f
path.conicTo(SkBits2Float(0x27202140), SkBits2Float(0x3a3b2769), SkBits2Float(0xc4371f20), SkBits2Float(0xecc52a22), SkBits2Float(0x21512727));  // 2.22225e-15f, 0.000713936f, -732.486f, -1.90686e+27f, 7.08638e-19f
path.lineTo(SkBits2Float(0x68315b2d), SkBits2Float(0xf0685527));  // 3.35016e+24f, -2.87614e+29f
path.close();
path.moveTo(SkBits2Float(0x6829523a), SkBits2Float(0x2d555b2d));  // 3.19839e+24f, 1.21279e-11f
path.moveTo(SkBits2Float(0x68556829), SkBits2Float(0x555b2d29));  // 4.03114e+24f, 1.50617e+13f
path.moveTo(SkBits2Float(0x1f2a322a), SkBits2Float(0xc0032108));  // 3.60404e-20f, -2.04889f
path.cubicTo(SkBits2Float(0x68572d55), SkBits2Float(0xf05bd24b), SkBits2Float(0x8c55272d), SkBits2Float(0x212a292a), SkBits2Float(0x0321082a), SkBits2Float(0xed4b7bc0));  // 4.06458e+24f, -2.72126e+29f, -1.64207e-31f, 5.76527e-19f, 4.7323e-37f, -3.93594e+27f
path.conicTo(SkBits2Float(0x212a8c6a), SkBits2Float(0x0329081f), SkBits2Float(0x6a4b7bc0), SkBits2Float(0x2829ed84), SkBits2Float(0x2d555b2d));  // 5.77841e-19f, 4.9674e-37f, 6.14991e+25f, 9.43289e-15f, 1.21279e-11f
path.moveTo(SkBits2Float(0x68305b2d), SkBits2Float(0xf0682955));  // 3.33127e+24f, -2.87402e+29f
path.conicTo(SkBits2Float(0x212a1f5b), SkBits2Float(0x8cef552a), SkBits2Float(0x295b2d2a), SkBits2Float(0x68210368), SkBits2Float(0x7bc05508));  // 5.76397e-19f, -3.6875e-31f, 4.86669e-14f, 3.04146e+24f, 1.99729e+36f
path.lineTo(SkBits2Float(0x68305b2d), SkBits2Float(0xf0682955));  // 3.33127e+24f, -2.87402e+29f
path.close();
path.moveTo(SkBits2Float(0x68305b2d), SkBits2Float(0xf0682955));  // 3.33127e+24f, -2.87402e+29f
path.lineTo(SkBits2Float(0x555b6829), SkBits2Float(0x6c212a8c));  // 1.50775e+13f, 7.79352e+26f
path.conicTo(SkBits2Float(0x084b0321), SkBits2Float(0x6ac07b2a), SkBits2Float(0x395b2d7a), SkBits2Float(0x5bf05568), SkBits2Float(0x212a3a8c));  // 6.10918e-34f, 1.16348e+26f, 0.000209024f, 1.35296e+17f, 5.76757e-19f
path.lineTo(SkBits2Float(0x8c558c55), SkBits2Float(0x212a1f2a));  // -1.64512e-31f, 5.76395e-19f

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 0, filename);
}

static void fuzz763_17(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x68556829), SkBits2Float(0x555b2d29));  // 4.03114e+24f, 1.50617e+13f
path.moveTo(SkBits2Float(0x1f2a312a), SkBits2Float(0xc0032108));  // 3.60396e-20f, -2.04889f
path.cubicTo(SkBits2Float(0x68392d55), SkBits2Float(0xf05b684b), SkBits2Float(0x8c55272d), SkBits2Float(0x212a292a), SkBits2Float(0x0321082a), SkBits2Float(0x6a4b7bc0));  // 3.4979e+24f, -2.71613e+29f, -1.64207e-31f, 5.76527e-19f, 4.7323e-37f, 6.14991e+25f
path.conicTo(SkBits2Float(0x212a8ced), SkBits2Float(0x0321081f), SkBits2Float(0x6a4b7bc0), SkBits2Float(0x2829ed84), SkBits2Float(0x69555b2d));  // 5.77848e-19f, 4.7323e-37f, 6.14991e+25f, 9.43289e-15f, 1.61207e+25f
path.moveTo(SkBits2Float(0x6835282d), SkBits2Float(0xf0682955));  // 3.42196e+24f, -2.87402e+29f
path.conicTo(SkBits2Float(0x212a1f5b), SkBits2Float(0x2aef552a), SkBits2Float(0x68295b2d), SkBits2Float(0x08682103), SkBits2Float(0x4b7bc055));  // 5.76397e-19f, 4.2514e-13f, 3.19905e+24f, 6.98538e-34f, 1.64988e+07f
path.lineTo(SkBits2Float(0x5b2c6829), SkBits2Float(0x212a8c55));  // 4.85282e+16f, 5.7784e-19f
path.moveTo(SkBits2Float(0x0321081f), SkBits2Float(0x6a4b7bc0));  // 4.7323e-37f, 6.14991e+25f
path.conicTo(SkBits2Float(0x68395b2d), SkBits2Float(0x555bf055), SkBits2Float(0x2a1f2a8c), SkBits2Float(0x03212a21), SkBits2Float(0x5a4b7bc0));  // 3.50128e+24f, 1.51141e+13f, 1.41368e-13f, 4.7362e-37f, 1.43189e+16f
path.conicTo(SkBits2Float(0xc08c2aed), SkBits2Float(0x211f2108), SkBits2Float(0x6a4b7b03), SkBits2Float(0x6829ed27), SkBits2Float(0x2d555b2d));  // -4.38024f, 5.3915e-19f, 6.14982e+25f, 3.20982e+24f, 1.21279e-11f
path.moveTo(SkBits2Float(0x68305b2d), SkBits2Float(0xf0685527));  // 3.33127e+24f, -2.87614e+29f
path.conicTo(SkBits2Float(0x2a8c555b), SkBits2Float(0x212a1f72), SkBits2Float(0x0321082a), SkBits2Float(0x6a4b7bc0), SkBits2Float(0x254793ed));  // 2.49282e-13f, 5.76399e-19f, 4.7323e-37f, 6.14991e+25f, 1.73106e-16f
path.quadTo(SkBits2Float(0x2128282a), SkBits2Float(0x3a8a3adf), SkBits2Float(0x8a284f1a), SkBits2Float(0xc2213ab3));  // 5.69738e-19f, 0.00105461f, -8.10378e-33f, -40.3073f
path.quadTo(SkBits2Float(0x1d2a2928), SkBits2Float(0x43962be6), SkBits2Float(0x3a2a812a), SkBits2Float(0x2a8ced29));  // 2.25206e-21f, 300.343f, 0.000650423f, 2.50336e-13f
path.lineTo(SkBits2Float(0x68305b2d), SkBits2Float(0xf0685527));  // 3.33127e+24f, -2.87614e+29f
path.close();
path.moveTo(SkBits2Float(0x68305b2d), SkBits2Float(0xf0685527));  // 3.33127e+24f, -2.87614e+29f
path.conicTo(SkBits2Float(0x03210831), SkBits2Float(0x6a4b7bc0), SkBits2Float(0x6829ed27), SkBits2Float(0x55555b2d), SkBits2Float(0x1e2a3a2a));  // 4.73231e-37f, 6.14991e+25f, 3.20982e+24f, 1.46617e+13f, 9.01175e-21f
path.conicTo(SkBits2Float(0x27202140), SkBits2Float(0x3a3b2729), SkBits2Float(0xc4371f20), SkBits2Float(0x16c52a22), SkBits2Float(0x515d27ec));  // 2.22225e-15f, 0.000713932f, -732.486f, 3.18537e-25f, 5.93661e+10f
path.lineTo(SkBits2Float(0x68305b2d), SkBits2Float(0xf0685527));  // 3.33127e+24f, -2.87614e+29f
path.close();
path.moveTo(SkBits2Float(0x6829523a), SkBits2Float(0x2d555b2d));  // 3.19839e+24f, 1.21279e-11f
path.moveTo(SkBits2Float(0x68556829), SkBits2Float(0x555b2d29));  // 4.03114e+24f, 1.50617e+13f
path.moveTo(SkBits2Float(0x1f2a312a), SkBits2Float(0xc0032108));  // 3.60396e-20f, -2.04889f
path.cubicTo(SkBits2Float(0x68572d55), SkBits2Float(0xf05b684b), SkBits2Float(0x8c55272d), SkBits2Float(0x212a292a), SkBits2Float(0x0321082a), SkBits2Float(0x6a4b7bc0));  // 4.06458e+24f, -2.71613e+29f, -1.64207e-31f, 5.76527e-19f, 4.7323e-37f, 6.14991e+25f
path.conicTo(SkBits2Float(0x212a8ced), SkBits2Float(0x0321081f), SkBits2Float(0x6a4b7bc0), SkBits2Float(0x2829ed84), SkBits2Float(0x2d555b2d));  // 5.77848e-19f, 4.7323e-37f, 6.14991e+25f, 9.43289e-15f, 1.21279e-11f
path.moveTo(SkBits2Float(0x68395b2d), SkBits2Float(0xf0682955));  // 3.50128e+24f, -2.87402e+29f
path.lineTo(SkBits2Float(0x2a8c555b), SkBits2Float(0x2a212a1f));  // 2.49282e-13f, 1.43143e-13f
path.lineTo(SkBits2Float(0x68395b2d), SkBits2Float(0xf0682955));  // 3.50128e+24f, -2.87402e+29f
path.close();
path.moveTo(SkBits2Float(0x68395b2d), SkBits2Float(0xf0682955));  // 3.50128e+24f, -2.87402e+29f
path.lineTo(SkBits2Float(0x8c2aed7a), SkBits2Float(0x2a1f08c0));  // -1.31678e-31f, 1.41251e-13f
path.lineTo(SkBits2Float(0x68395b2d), SkBits2Float(0xf0682955));  // 3.50128e+24f, -2.87402e+29f
path.close();
path.moveTo(SkBits2Float(0x2a8cef55), SkBits2Float(0x68295b2d));  // 2.50351e-13f, 3.19905e+24f
path.conicTo(SkBits2Float(0x55086821), SkBits2Float(0x6a4b7bc0), SkBits2Float(0x5b2c6829), SkBits2Float(0x21218c55), SkBits2Float(0x2a6c1f03));  // 9.3738e+12f, 6.14991e+25f, 4.85282e+16f, 5.47346e-19f, 2.09718e-13f
path.lineTo(SkBits2Float(0x2a8cef55), SkBits2Float(0x68295b2d));  // 2.50351e-13f, 3.19905e+24f
path.close();
path.moveTo(SkBits2Float(0x2a8cef55), SkBits2Float(0x68295b2d));  // 2.50351e-13f, 3.19905e+24f
path.lineTo(SkBits2Float(0x6ac07b2a), SkBits2Float(0x395b2d7a));  // 1.16348e+26f, 0.000209024f

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 0, filename);
}

static void fuzz763_18(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x68556829), SkBits2Float(0x555b2d29));  // 4.03114e+24f, 1.50617e+13f
path.moveTo(SkBits2Float(0x1f2a312a), SkBits2Float(0xc0032108));  // 3.60396e-20f, -2.04889f
path.cubicTo(SkBits2Float(0x68392d55), SkBits2Float(0xf05b684b), SkBits2Float(0x8c55272d), SkBits2Float(0x212a292a), SkBits2Float(0x0321082a), SkBits2Float(0x6a4b7bc0));  // 3.4979e+24f, -2.71613e+29f, -1.64207e-31f, 5.76527e-19f, 4.7323e-37f, 6.14991e+25f
path.conicTo(SkBits2Float(0x212a8ced), SkBits2Float(0x0321081f), SkBits2Float(0x6a4b7bc0), SkBits2Float(0x2829ed84), SkBits2Float(0x69555b2d));  // 5.77848e-19f, 4.7323e-37f, 6.14991e+25f, 9.43289e-15f, 1.61207e+25f
path.moveTo(SkBits2Float(0x6835282d), SkBits2Float(0xf0682955));  // 3.42196e+24f, -2.87402e+29f
path.conicTo(SkBits2Float(0x212a1f5b), SkBits2Float(0x2aef552a), SkBits2Float(0x68295b2d), SkBits2Float(0x08682103), SkBits2Float(0x4b7bc055));  // 5.76397e-19f, 4.2514e-13f, 3.19905e+24f, 6.98538e-34f, 1.64988e+07f
path.lineTo(SkBits2Float(0x5b2c6829), SkBits2Float(0x212a8c55));  // 4.85282e+16f, 5.7784e-19f
path.moveTo(SkBits2Float(0x0321081f), SkBits2Float(0x6a4b7bc0));  // 4.7323e-37f, 6.14991e+25f
path.conicTo(SkBits2Float(0x68395b2d), SkBits2Float(0x555bf055), SkBits2Float(0x2a1f2a8c), SkBits2Float(0x03212a21), SkBits2Float(0x5a4b7bc0));  // 3.50128e+24f, 1.51141e+13f, 1.41368e-13f, 4.7362e-37f, 1.43189e+16f
path.conicTo(SkBits2Float(0xc08c2aed), SkBits2Float(0x211f2108), SkBits2Float(0x6a4b7b03), SkBits2Float(0x6829ed27), SkBits2Float(0x2d555b2d));  // -4.38024f, 5.3915e-19f, 6.14982e+25f, 3.20982e+24f, 1.21279e-11f
path.moveTo(SkBits2Float(0x68305b2d), SkBits2Float(0xf0685527));  // 3.33127e+24f, -2.87614e+29f
path.conicTo(SkBits2Float(0x2a8c555b), SkBits2Float(0x212a1f72), SkBits2Float(0x0321082a), SkBits2Float(0x6a4b7bc0), SkBits2Float(0x254793ed));  // 2.49282e-13f, 5.76399e-19f, 4.7323e-37f, 6.14991e+25f, 1.73106e-16f
path.quadTo(SkBits2Float(0x2128282a), SkBits2Float(0x3a8a3adf), SkBits2Float(0x8a284f1a), SkBits2Float(0xc2213ab3));  // 5.69738e-19f, 0.00105461f, -8.10378e-33f, -40.3073f
path.quadTo(SkBits2Float(0x1d2a2928), SkBits2Float(0x43962be6), SkBits2Float(0x3a2a812a), SkBits2Float(0x2a8ced29));  // 2.25206e-21f, 300.343f, 0.000650423f, 2.50336e-13f
path.lineTo(SkBits2Float(0x68305b2d), SkBits2Float(0xf0685527));  // 3.33127e+24f, -2.87614e+29f
path.close();
path.moveTo(SkBits2Float(0x68305b2d), SkBits2Float(0xf0685527));  // 3.33127e+24f, -2.87614e+29f
path.conicTo(SkBits2Float(0x03210831), SkBits2Float(0x6a4b7bc0), SkBits2Float(0x6829ed27), SkBits2Float(0x55555b2d), SkBits2Float(0x1e2a3a2a));  // 4.73231e-37f, 6.14991e+25f, 3.20982e+24f, 1.46617e+13f, 9.01175e-21f
path.conicTo(SkBits2Float(0x27202140), SkBits2Float(0x3a3b2729), SkBits2Float(0xc4371f20), SkBits2Float(0x16c52a22), SkBits2Float(0x515d27ec));  // 2.22225e-15f, 0.000713932f, -732.486f, 3.18537e-25f, 5.93661e+10f
path.lineTo(SkBits2Float(0x68305b2d), SkBits2Float(0xf0685527));  // 3.33127e+24f, -2.87614e+29f
path.close();
path.moveTo(SkBits2Float(0x6829523a), SkBits2Float(0x2d555b2d));  // 3.19839e+24f, 1.21279e-11f
path.moveTo(SkBits2Float(0x68556829), SkBits2Float(0x555b2d29));  // 4.03114e+24f, 1.50617e+13f
path.moveTo(SkBits2Float(0x1f2a312a), SkBits2Float(0xc0032108));  // 3.60396e-20f, -2.04889f
path.cubicTo(SkBits2Float(0x68572d55), SkBits2Float(0xf05b684b), SkBits2Float(0x8c55272d), SkBits2Float(0x212a292a), SkBits2Float(0x0321082a), SkBits2Float(0x6a4b7bc0));  // 4.06458e+24f, -2.71613e+29f, -1.64207e-31f, 5.76527e-19f, 4.7323e-37f, 6.14991e+25f
path.conicTo(SkBits2Float(0x212a8ced), SkBits2Float(0x0321081f), SkBits2Float(0x6a4b7bc0), SkBits2Float(0x2829ed84), SkBits2Float(0x2d555b2d));  // 5.77848e-19f, 4.7323e-37f, 6.14991e+25f, 9.43289e-15f, 1.21279e-11f
path.moveTo(SkBits2Float(0x68395b2d), SkBits2Float(0xf0682955));  // 3.50128e+24f, -2.87402e+29f
path.lineTo(SkBits2Float(0x2a8c555b), SkBits2Float(0x2a212a1f));  // 2.49282e-13f, 1.43143e-13f
path.lineTo(SkBits2Float(0x68395b2d), SkBits2Float(0xf0682955));  // 3.50128e+24f, -2.87402e+29f
path.close();
path.moveTo(SkBits2Float(0x68395b2d), SkBits2Float(0xf0682955));  // 3.50128e+24f, -2.87402e+29f
path.lineTo(SkBits2Float(0x8c2aed7a), SkBits2Float(0x2a1f08c0));  // -1.31678e-31f, 1.41251e-13f

path.moveTo(SkBits2Float(0x6829523a), SkBits2Float(0x2d555b2d));  // 3.19839e+24f, 1.21279e-11f
path.moveTo(SkBits2Float(0x68556829), SkBits2Float(0x555b2d29));  // 4.03114e+24f, 1.50617e+13f
path.moveTo(SkBits2Float(0x1f2a312a), SkBits2Float(0xc0032108));  // 3.60396e-20f, -2.04889f
path.cubicTo(SkBits2Float(0x68572d55), SkBits2Float(0xf05b684b), SkBits2Float(0x8c55272d), SkBits2Float(0x212a292a), SkBits2Float(0x0321082a), SkBits2Float(0x6a4b7bc0));  // 4.06458e+24f, -2.71613e+29f, -1.64207e-31f, 5.76527e-19f, 4.7323e-37f, 6.14991e+25f
path.conicTo(SkBits2Float(0x2a8c54ed), SkBits2Float(0x21081f21), SkBits2Float(0x4b7bc003), SkBits2Float(0x29ed846a), SkBits2Float(0x555b2d28));  // 2.49279e-13f, 4.61198e-19f, 1.64987e+07f, 1.05479e-13f, 1.50617e+13f
path.conicTo(SkBits2Float(0x68392d5b), SkBits2Float(0xf0682955), SkBits2Float(0x2a1f5b2d), SkBits2Float(0xef552a21), SkBits2Float(0x5b2d2a8c));  // 3.4979e+24f, -2.87402e+29f, 1.41537e-13f, -6.59712e+28f, 4.8742e+16f

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 0, filename);
}

static void fuzz763_19(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x21081f21), SkBits2Float(0x4b7bc003));  // 4.61198e-19f, 1.64987e+07f
path.lineTo(SkBits2Float(0x2829ed84), SkBits2Float(0x69555b2d));  // 9.43289e-15f, 1.61207e+25f
path.moveTo(SkBits2Float(0x68305b2d), SkBits2Float(0xf0682955));  // 3.33127e+24f, -2.87402e+29f
path.conicTo(SkBits2Float(0x212a1f5b), SkBits2Float(0x2aef552a), SkBits2Float(0x68295b2d), SkBits2Float(0x08682103), SkBits2Float(0x4b7bc055));  // 5.76397e-19f, 4.2514e-13f, 3.19905e+24f, 6.98538e-34f, 1.64988e+07f
path.moveTo(SkBits2Float(0x5b2c6829), SkBits2Float(0x212a8c55));  // 4.85282e+16f, 5.7784e-19f
path.moveTo(SkBits2Float(0x0321081f), SkBits2Float(0x6a4b7bc0));  // 4.7323e-37f, 6.14991e+25f
path.conicTo(SkBits2Float(0x68395b2d), SkBits2Float(0x8c5bf055), SkBits2Float(0x2a1f2a55), SkBits2Float(0x03212a21), SkBits2Float(0x5a4b7bc0));  // 3.50128e+24f, -1.69435e-31f, 1.41367e-13f, 4.7362e-37f, 1.43189e+16f
path.conicTo(SkBits2Float(0xc08c2aed), SkBits2Float(0x211f2108), SkBits2Float(0x6a4b7b03), SkBits2Float(0x6829ed27), SkBits2Float(0x2d555b2d));  // -4.38024f, 5.3915e-19f, 6.14982e+25f, 3.20982e+24f, 1.21279e-11f
path.moveTo(SkBits2Float(0x68315b2d), SkBits2Float(0xf0685527));  // 3.35016e+24f, -2.87614e+29f
path.conicTo(SkBits2Float(0x2a8c555b), SkBits2Float(0x212a1f72), SkBits2Float(0x0321082a), SkBits2Float(0x6a4b7bc0), SkBits2Float(0x2547937a));  // 2.49282e-13f, 5.76399e-19f, 4.7323e-37f, 6.14991e+25f, 1.73105e-16f
path.quadTo(SkBits2Float(0x2128282a), SkBits2Float(0x3a8a3adf), SkBits2Float(0x8a284f1a), SkBits2Float(0xc2213ab3));  // 5.69738e-19f, 0.00105461f, -8.10378e-33f, -40.3073f
path.quadTo(SkBits2Float(0x1d2a2928), SkBits2Float(0x43962be6), SkBits2Float(0x3a2a812a), SkBits2Float(0x2a8ced29));  // 2.25206e-21f, 300.343f, 0.000650423f, 2.50336e-13f
path.lineTo(SkBits2Float(0x68315b2d), SkBits2Float(0xf0685527));  // 3.35016e+24f, -2.87614e+29f
path.close();
path.moveTo(SkBits2Float(0x68315b2d), SkBits2Float(0xf0685527));  // 3.35016e+24f, -2.87614e+29f
path.conicTo(SkBits2Float(0x03210831), SkBits2Float(0x6a4b7bc0), SkBits2Float(0x6829ed27), SkBits2Float(0x55555b2d), SkBits2Float(0x1e2a3a2a));  // 4.73231e-37f, 6.14991e+25f, 3.20982e+24f, 1.46617e+13f, 9.01175e-21f
path.conicTo(SkBits2Float(0x27202140), SkBits2Float(0x3a3b2729), SkBits2Float(0xc4371f20), SkBits2Float(0xecc52a22), SkBits2Float(0x21515d27));  // 2.22225e-15f, 0.000713932f, -732.486f, -1.90686e+27f, 7.09352e-19f
path.lineTo(SkBits2Float(0x68315b2d), SkBits2Float(0xf0685527));  // 3.35016e+24f, -2.87614e+29f
path.close();
path.moveTo(SkBits2Float(0x6829523a), SkBits2Float(0x2d555b2d));  // 3.19839e+24f, 1.21279e-11f
path.moveTo(SkBits2Float(0x68556829), SkBits2Float(0x555b2d29));  // 4.03114e+24f, 1.50617e+13f
path.moveTo(SkBits2Float(0x1f2a312a), SkBits2Float(0xc0032108));  // 3.60396e-20f, -2.04889f
path.cubicTo(SkBits2Float(0x68572d55), SkBits2Float(0xf05b684b), SkBits2Float(0x8c55272d), SkBits2Float(0x212a292a), SkBits2Float(0x0321082a), SkBits2Float(0x6a4b7bc0));  // 4.06458e+24f, -2.71613e+29f, -1.64207e-31f, 5.76527e-19f, 4.7323e-37f, 6.14991e+25f
path.conicTo(SkBits2Float(0x212a8ced), SkBits2Float(0x0321081f), SkBits2Float(0x6a4b7bc0), SkBits2Float(0x2829ed84), SkBits2Float(0x2d555b2d));  // 5.77848e-19f, 4.7323e-37f, 6.14991e+25f, 9.43289e-15f, 1.21279e-11f
path.moveTo(SkBits2Float(0x68395b2d), SkBits2Float(0xf0682955));  // 3.50128e+24f, -2.87402e+29f
path.conicTo(SkBits2Float(0x212a1f5b), SkBits2Float(0x8cef552a), SkBits2Float(0x295b2d2a), SkBits2Float(0x68210368), SkBits2Float(0x7bc05508));  // 5.76397e-19f, -3.6875e-31f, 4.86669e-14f, 3.04146e+24f, 1.99729e+36f
path.lineTo(SkBits2Float(0x68395b2d), SkBits2Float(0xf0682955));  // 3.50128e+24f, -2.87402e+29f
path.close();
path.moveTo(SkBits2Float(0x68395b2d), SkBits2Float(0xf0682955));  // 3.50128e+24f, -2.87402e+29f
path.lineTo(SkBits2Float(0x555b2c29), SkBits2Float(0x6c212a8c));  // 1.50614e+13f, 7.79352e+26f
path.conicTo(SkBits2Float(0x084b0321), SkBits2Float(0x6ac07b2a), SkBits2Float(0x395b2d7a), SkBits2Float(0xf05b5568), SkBits2Float(0x212a3a8c));  // 6.10918e-34f, 1.16348e+26f, 0.000209024f, -2.71522e+29f, 5.76757e-19f
path.conicTo(SkBits2Float(0x290321d9), SkBits2Float(0x555b2d68), SkBits2Float(0x2a8c558c), SkBits2Float(0x2abe2a1f), SkBits2Float(0x7bc00321));  // 2.91172e-14f, 1.50618e+13f, 2.49284e-13f, 3.378e-13f, 1.99397e+36f
path.lineTo(SkBits2Float(0x68395b2d), SkBits2Float(0xf0682955));  // 3.50128e+24f, -2.87402e+29f
path.close();
path.moveTo(SkBits2Float(0x68395b2d), SkBits2Float(0xf0682955));  // 3.50128e+24f, -2.87402e+29f
path.lineTo(SkBits2Float(0x8c2aed7a), SkBits2Float(0x1f2128c0));  // -1.31678e-31f, 3.41268e-20f
path.lineTo(SkBits2Float(0x68395b2d), SkBits2Float(0xf0682955));  // 3.50128e+24f, -2.87402e+29f
path.close();

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 0, filename);
}

static void fuzz763_20(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x68556829), SkBits2Float(0x555b2d29));  // 4.03114e+24f, 1.50617e+13f
path.moveTo(SkBits2Float(0x1f2a312a), SkBits2Float(0xc0032108));  // 3.60396e-20f, -2.04889f
path.cubicTo(SkBits2Float(0x68392d55), SkBits2Float(0xf05b684b), SkBits2Float(0x8c55272d), SkBits2Float(0x212a292a), SkBits2Float(0x0321082a), SkBits2Float(0x6a4b7bc0));  // 3.4979e+24f, -2.71613e+29f, -1.64207e-31f, 5.76527e-19f, 4.7323e-37f, 6.14991e+25f
path.conicTo(SkBits2Float(0x212a8ced), SkBits2Float(0x0321081f), SkBits2Float(0x6a4b7bc0), SkBits2Float(0x2829ed84), SkBits2Float(0x69555b2d));  // 5.77848e-19f, 4.7323e-37f, 6.14991e+25f, 9.43289e-15f, 1.61207e+25f
path.moveTo(SkBits2Float(0x68305b2d), SkBits2Float(0xf0682955));  // 3.33127e+24f, -2.87402e+29f
path.conicTo(SkBits2Float(0x212a1f5b), SkBits2Float(0x2a8c552a), SkBits2Float(0x68295b2d), SkBits2Float(0x08682103), SkBits2Float(0x4b7bc055));  // 5.76397e-19f, 2.49281e-13f, 3.19905e+24f, 6.98538e-34f, 1.64988e+07f
path.lineTo(SkBits2Float(0x5b2c6829), SkBits2Float(0x212a8c55));  // 4.85282e+16f, 5.7784e-19f
path.moveTo(SkBits2Float(0x0321081f), SkBits2Float(0x6a4b7bc0));  // 4.7323e-37f, 6.14991e+25f
path.conicTo(SkBits2Float(0x68395b2d), SkBits2Float(0x555bf055), SkBits2Float(0x2a1f2a8c), SkBits2Float(0x03212a21), SkBits2Float(0x5a4b7bc0));  // 3.50128e+24f, 1.51141e+13f, 1.41368e-13f, 4.7362e-37f, 1.43189e+16f
path.conicTo(SkBits2Float(0xc08c2aed), SkBits2Float(0x211f2108), SkBits2Float(0x6a4b7b03), SkBits2Float(0x6829ed27), SkBits2Float(0x2d555b2d));  // -4.38024f, 5.3915e-19f, 6.14982e+25f, 3.20982e+24f, 1.21279e-11f
path.moveTo(SkBits2Float(0x68305b2d), SkBits2Float(0xf0685527));  // 3.33127e+24f, -2.87614e+29f
path.conicTo(SkBits2Float(0x2a8c555b), SkBits2Float(0x6e2a1f72), SkBits2Float(0x0321182a), SkBits2Float(0x6a4b7bc0), SkBits2Float(0x4793ed7a));  // 2.49282e-13f, 1.31626e+28f, 4.73414e-37f, 6.14991e+25f, 75739
path.lineTo(SkBits2Float(0x68305b2d), SkBits2Float(0xf0685527));  // 3.33127e+24f, -2.87614e+29f
path.close();
path.moveTo(SkBits2Float(0x68305b2d), SkBits2Float(0xf0685527));  // 3.33127e+24f, -2.87614e+29f
path.quadTo(SkBits2Float(0x2128282a), SkBits2Float(0x3a8a3adf), SkBits2Float(0x8a284f1a), SkBits2Float(0x2c213ab3));  // 5.69738e-19f, 0.00105461f, -8.10378e-33f, 2.29121e-12f
path.lineTo(SkBits2Float(0x68305b2d), SkBits2Float(0xf0685527));  // 3.33127e+24f, -2.87614e+29f
path.close();
path.moveTo(SkBits2Float(0x68305b2d), SkBits2Float(0xf0685527));  // 3.33127e+24f, -2.87614e+29f
path.quadTo(SkBits2Float(0x1d2a2928), SkBits2Float(0x43962be6), SkBits2Float(0x3a2a812a), SkBits2Float(0x2a8ced29));  // 2.25206e-21f, 300.343f, 0.000650423f, 2.50336e-13f
path.lineTo(SkBits2Float(0x68305b2d), SkBits2Float(0xf0685527));  // 3.33127e+24f, -2.87614e+29f
path.close();
path.moveTo(SkBits2Float(0x68305b2d), SkBits2Float(0xf0685527));  // 3.33127e+24f, -2.87614e+29f
path.conicTo(SkBits2Float(0x03210831), SkBits2Float(0x6a4b7bc0), SkBits2Float(0x6829ed27), SkBits2Float(0x55555b2d), SkBits2Float(0x1e2a3a2a));  // 4.73231e-37f, 6.14991e+25f, 3.20982e+24f, 1.46617e+13f, 9.01175e-21f
path.conicTo(SkBits2Float(0x27202140), SkBits2Float(0x3a3b2769), SkBits2Float(0xc4371f20), SkBits2Float(0xecc52a22), SkBits2Float(0x51282727));  // 2.22225e-15f, 0.000713936f, -732.486f, -1.90686e+27f, 4.51382e+10f
path.lineTo(SkBits2Float(0x68305b2d), SkBits2Float(0xf0685527));  // 3.33127e+24f, -2.87614e+29f
path.close();
path.moveTo(SkBits2Float(0x6829523a), SkBits2Float(0x2d555b2d));  // 3.19839e+24f, 1.21279e-11f
path.moveTo(SkBits2Float(0x68556829), SkBits2Float(0x8c555b2d));  // 4.03114e+24f, -1.64364e-31f
path.moveTo(SkBits2Float(0x081f2a31), SkBits2Float(0xc0032921));  // 4.78969e-34f, -2.04939f
path.cubicTo(SkBits2Float(0x68572d55), SkBits2Float(0xf05bd24b), SkBits2Float(0x8c55272d), SkBits2Float(0x212a292a), SkBits2Float(0x0321082a), SkBits2Float(0xed4b7bc0));  // 4.06458e+24f, -2.72126e+29f, -1.64207e-31f, 5.76527e-19f, 4.7323e-37f, -3.93594e+27f
path.conicTo(SkBits2Float(0x212a8c6a), SkBits2Float(0x4329081f), SkBits2Float(0x6a4b7bc0), SkBits2Float(0x2829ed84), SkBits2Float(0x5b2d2d55));  // 5.77841e-19f, 169.032f, 6.14991e+25f, 9.43289e-15f, 4.8745e+16f
path.moveTo(SkBits2Float(0x68395b2d), SkBits2Float(0xf0682955));  // 3.50128e+24f, -2.87402e+29f
path.conicTo(SkBits2Float(0x212a1f5b), SkBits2Float(0x8cef552a), SkBits2Float(0x295b2d2a), SkBits2Float(0x3a210368), SkBits2Float(0x7bc05508));  // 5.76397e-19f, -3.6875e-31f, 4.86669e-14f, 0.000614217f, 1.99729e+36f
path.lineTo(SkBits2Float(0x68395b2d), SkBits2Float(0xf0682955));  // 3.50128e+24f, -2.87402e+29f
path.close();
path.moveTo(SkBits2Float(0x68395b2d), SkBits2Float(0xf0682955));  // 3.50128e+24f, -2.87402e+29f
path.lineTo(SkBits2Float(0x555b6829), SkBits2Float(0x6c212a8c));  // 1.50775e+13f, 7.79352e+26f
path.lineTo(SkBits2Float(0x5b2d7a6a), SkBits2Float(0xf0556830));  // 4.88298e+16f, -2.64185e+29f
path.lineTo(SkBits2Float(0x68395b2d), SkBits2Float(0xf0682955));  // 3.50128e+24f, -2.87402e+29f
path.close();
path.moveTo(SkBits2Float(0x68395b2d), SkBits2Float(0xf0682955));  // 3.50128e+24f, -2.87402e+29f
path.conicTo(SkBits2Float(0x0321d90a), SkBits2Float(0x555b2d68), SkBits2Float(0x2a8c558c), SkBits2Float(0x212a2a1f), SkBits2Float(0x4b7bc003));  // 4.75628e-37f, 1.50618e+13f, 2.49284e-13f, 5.7654e-19f, 1.64987e+07f
path.lineTo(SkBits2Float(0x8c2aed7a), SkBits2Float(0x212128c0));  // -1.31678e-31f, 5.46029e-19f
path.lineTo(SkBits2Float(0x68395b2d), SkBits2Float(0xf0682955));  // 3.50128e+24f, -2.87402e+29f
path.close();

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 0, filename);
}

static void fuzz763_21(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x6828c6f9), SkBits2Float(0x6614dc9e));  // 3.18811e+24f, 1.75745e+23f
path.cubicTo(SkBits2Float(0x68303469), SkBits2Float(0x661f92fc), SkBits2Float(0x6837d3c3), SkBits2Float(0x662b0eb2), SkBits2Float(0x683fa268), SkBits2Float(0x663759e1));  // 3.32841e+24f, 1.88392e+23f, 3.4724e+24f, 2.01949e+23f, 3.61987e+24f, 2.16463e+23f
path.cubicTo(SkBits2Float(0x68c4391f), SkBits2Float(0x672c5c9f), SkBits2Float(0x688b20ab), SkBits2Float(0x6804b825), SkBits2Float(0x681ddb5e), SkBits2Float(0x6838dc00));  // 7.4131e+24f, 8.13956e+23f, 5.25609e+24f, 2.507e+24f, 2.98183e+24f, 3.49189e+24f
path.lineTo(SkBits2Float(0x6828c6f9), SkBits2Float(0x6614dc9e));  // 3.18811e+24f, 1.75745e+23f
path.close();
path.moveTo(SkBits2Float(0x68226c73), SkBits2Float(0x660bd15e));  // 3.0681e+24f, 1.65068e+23f
path.cubicTo(SkBits2Float(0x6823b0e1), SkBits2Float(0x660d990f), SkBits2Float(0x6824f6d5), SkBits2Float(0x660f668c), SkBits2Float(0x68263e4e), SkBits2Float(0x66113632));  // 3.09203e+24f, 1.67169e+23f, 3.11609e+24f, 1.69298e+23f, 3.14025e+24f, 1.71436e+23f
path.cubicTo(SkBits2Float(0x682715e4), SkBits2Float(0x6612676d), SkBits2Float(0x6827ee22), SkBits2Float(0x66139997), SkBits2Float(0x6828c709), SkBits2Float(0x6614cba5));  // 3.15616e+24f, 1.72843e+23f, 3.17211e+24f, 1.74255e+23f, 3.18812e+24f, 1.75667e+23f
path.lineTo(SkBits2Float(0x6828d720), SkBits2Float(0x6604a1a2));  // 3.1893e+24f, 1.56583e+23f
path.cubicTo(SkBits2Float(0x68270421), SkBits2Float(0x6601102c), SkBits2Float(0x68252b97), SkBits2Float(0x65fb1edd), SkBits2Float(0x68234ce5), SkBits2Float(0x65f4367f));  // 3.15485e+24f, 1.52371e+23f, 3.11998e+24f, 1.48235e+23f, 3.08466e+24f, 1.44158e+23f
path.conicTo(SkBits2Float(0x6822e012), SkBits2Float(0x6602acc5), SkBits2Float(0x68226c73), SkBits2Float(0x660bd15e), SkBits2Float(0x3f7ffa04));  // 3.07663e+24f, 1.54274e+23f, 3.0681e+24f, 1.65068e+23f, 0.999909f
path.close();
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x6a2a291f));  // 0, 5.14279e+25f
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0x68555b2a));  // 0, 4.03018e+24f
path.cubicTo(SkBits2Float(0x00000000), SkBits2Float(0x68617414), SkBits2Float(0x66af1c42), SkBits2Float(0x68624f96), SkBits2Float(0x6757755b), SkBits2Float(0x685b93f2));  // 0, 4.25869e+24f, 4.13468e+23f, 4.27489e+24f, 1.01747e+24f, 4.14771e+24f
path.cubicTo(SkBits2Float(0x67a63a84), SkBits2Float(0x68fe1c37), SkBits2Float(0x67c05eed), SkBits2Float(0x69930962), SkBits2Float(0x00000000), SkBits2Float(0x6a2a291f));  // 1.56998e+24f, 9.60001e+24f, 1.81689e+24f, 2.22196e+25f, 0, 5.14279e+25f
path.close();
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x6a2a291f));  // 0, 5.14279e+25f
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0x6a4b7bc4));  // 0, 6.14991e+25f
path.cubicTo(SkBits2Float(0x6a2c8798), SkBits2Float(0x68f7a144), SkBits2Float(0x6951f5ea), SkBits2Float(0x6796ad55), SkBits2Float(0x683fa268), SkBits2Float(0x663759e1));  // 5.21439e+25f, 9.35519e+24f, 1.58642e+25f, 1.4231e+24f, 3.61987e+24f, 2.16463e+23f
path.cubicTo(SkBits2Float(0x683871e3), SkBits2Float(0x66253b4f), SkBits2Float(0x6830da01), SkBits2Float(0x66144d3e), SkBits2Float(0x6828d720), SkBits2Float(0x6604a1a2));  // 3.48407e+24f, 1.95071e+23f, 3.34063e+24f, 1.75084e+23f, 3.1893e+24f, 1.56583e+23f
path.conicTo(SkBits2Float(0x68295b21), SkBits2Float(0x00000000), SkBits2Float(0x00000000), SkBits2Float(0x00000000), SkBits2Float(0x492bb324));  // 3.19904e+24f, 0, 0, 0, 703282
path.cubicTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000), SkBits2Float(0x677b84f0), SkBits2Float(0x00000000), SkBits2Float(0x68226c73), SkBits2Float(0x660bd15e));  // 0, 0, 1.18777e+24f, 0, 3.0681e+24f, 1.65068e+23f
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0x68156829));  // 0, 2.82222e+24f
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0x68555b2a));  // 0, 4.03018e+24f
path.lineTo(SkBits2Float(0x673918f3), SkBits2Float(0x681b0f5f));  // 8.74098e+23f, 2.929e+24f
path.lineTo(SkBits2Float(0x67391759), SkBits2Float(0x681b0fae));  // 8.74068e+23f, 2.92902e+24f
path.cubicTo(SkBits2Float(0x674384e7), SkBits2Float(0x682e2068), SkBits2Float(0x674db698), SkBits2Float(0x6843893b), SkBits2Float(0x6757755b), SkBits2Float(0x685b93f2));  // 9.23313e+23f, 3.28916e+24f, 9.71453e+23f, 3.69357e+24f, 1.01747e+24f, 4.14771e+24f
path.cubicTo(SkBits2Float(0x67a63484), SkBits2Float(0x68556bdd), SkBits2Float(0x67f18c5f), SkBits2Float(0x6848eb25), SkBits2Float(0x681ddb5e), SkBits2Float(0x6838dc00));  // 1.56976e+24f, 4.03142e+24f, 2.28136e+24f, 3.79524e+24f, 2.98183e+24f, 3.49189e+24f
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0x6a2a291f));  // 0, 5.14279e+25f
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 1, filename);
}

static void fuzz763_22(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x68295b2d));  // 0, 3.19905e+24f
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.lineTo(SkBits2Float(0x6a3a7bc0), SkBits2Float(0x00000000));  // 5.63611e+25f, 0
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0x6a034b21));  // 0, 3.9681e+25f
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0x68295b2d));  // 0, 3.19905e+24f
path.close();
path.moveTo(SkBits2Float(0x6617da56), SkBits2Float(0x00000000));  // 1.79276e+23f, 0
path.conicTo(SkBits2Float(0x5e704d09), SkBits2Float(0x5e3a4dfd), SkBits2Float(0x00000000), SkBits2Float(0x65eb62ef), SkBits2Float(0x430fa5e6));  // 4.32888e+18f, 3.35617e+18f, 0, 1.38948e+23f, 143.648f
path.conicTo(SkBits2Float(0x5e798b32), SkBits2Float(0x627a95c0), SkBits2Float(0x61f5014c), SkBits2Float(0x61fba0fd), SkBits2Float(0x40f8a1a1));  // 4.49538e+18f, 1.15562e+21f, 5.64943e+20f, 5.80217e+20f, 7.76973f
path.conicTo(SkBits2Float(0x62743d2d), SkBits2Float(0x5e49b862), SkBits2Float(0x6617da56), SkBits2Float(0x00000000), SkBits2Float(0x410ef54c));  // 1.12635e+21f, 3.63387e+18f, 1.79276e+23f, 0, 8.93489f
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.quadTo(SkBits2Float(0x4f9a3a8a), SkBits2Float(0xc28a0d28), SkBits2Float(0x273a3ab3), SkBits2Float(0x8b2a2928));  // 5.17506e+09f, -69.0257f, 2.58445e-15f, -3.27718e-32f
path.lineTo(SkBits2Float(0x63283ae6), SkBits2Float(0x27282a81));  // 3.1033e+21f, 2.33377e-15f

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 3, filename);
}

static void fuzz763_23(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x68556829), SkBits2Float(0x555b2d29));  // 4.03114e+24f, 1.50617e+13f
path.moveTo(SkBits2Float(0x1f2a312a), SkBits2Float(0xc0032108));  // 3.60396e-20f, -2.04889f
path.cubicTo(SkBits2Float(0x68392d55), SkBits2Float(0xf05b684b), SkBits2Float(0x8c55272d), SkBits2Float(0x212a292a), SkBits2Float(0x03210c2a), SkBits2Float(0x6a4b7bc0));  // 3.4979e+24f, -2.71613e+29f, -1.64207e-31f, 5.76527e-19f, 4.73276e-37f, 6.14991e+25f
path.conicTo(SkBits2Float(0x212a8ced), SkBits2Float(0x0321081f), SkBits2Float(0x6a4b7bc0), SkBits2Float(0x2829ed84), SkBits2Float(0x69555b2d));  // 5.77848e-19f, 4.7323e-37f, 6.14991e+25f, 9.43289e-15f, 1.61207e+25f
path.moveTo(SkBits2Float(0x68305b2d), SkBits2Float(0xf0682955));  // 3.33127e+24f, -2.87402e+29f
path.conicTo(SkBits2Float(0x212a1f5b), SkBits2Float(0x2aef552a), SkBits2Float(0x29295b2d), SkBits2Float(0x68210368), SkBits2Float(0x7bc05508));  // 5.76397e-19f, 4.2514e-13f, 3.76046e-14f, 3.04146e+24f, 1.99729e+36f
path.lineTo(SkBits2Float(0x68305b2d), SkBits2Float(0xf0682955));  // 3.33127e+24f, -2.87402e+29f
path.close();
path.moveTo(SkBits2Float(0x5b2c6829), SkBits2Float(0x212a8c55));  // 4.85282e+16f, 5.7784e-19f
path.moveTo(SkBits2Float(0x0321081f), SkBits2Float(0x6a4b7bc0));  // 4.7323e-37f, 6.14991e+25f
path.conicTo(SkBits2Float(0x68395b2d), SkBits2Float(0x8c5bf055), SkBits2Float(0x2a1f2a55), SkBits2Float(0x03212a21), SkBits2Float(0x5a4b7bc0));  // 3.50128e+24f, -1.69435e-31f, 1.41367e-13f, 4.7362e-37f, 1.43189e+16f
path.conicTo(SkBits2Float(0xc08c2aed), SkBits2Float(0x211f2108), SkBits2Float(0x6a4b7b03), SkBits2Float(0x6829ed27), SkBits2Float(0x2d555b2d));  // -4.38024f, 5.3915e-19f, 6.14982e+25f, 3.20982e+24f, 1.21279e-11f
path.moveTo(SkBits2Float(0x68315b2d), SkBits2Float(0xf0685527));  // 3.35016e+24f, -2.87614e+29f
path.conicTo(SkBits2Float(0x2a8c555b), SkBits2Float(0x08211f72), SkBits2Float(0x032a2a21), SkBits2Float(0x6a4b7bc0), SkBits2Float(0x2547937a));  // 2.49282e-13f, 4.84861e-34f, 5.00069e-37f, 6.14991e+25f, 1.73105e-16f
path.quadTo(SkBits2Float(0x2128282a), SkBits2Float(0x3a8a3adf), SkBits2Float(0x8a284f1a), SkBits2Float(0xc2213ab3));  // 5.69738e-19f, 0.00105461f, -8.10378e-33f, -40.3073f
path.quadTo(SkBits2Float(0x1d2a2928), SkBits2Float(0x43962be6), SkBits2Float(0x3a2a812a), SkBits2Float(0x2a8ced29));  // 2.25206e-21f, 300.343f, 0.000650423f, 2.50336e-13f
path.lineTo(SkBits2Float(0x68315b2d), SkBits2Float(0xf0685527));  // 3.35016e+24f, -2.87614e+29f
path.close();
path.moveTo(SkBits2Float(0x68315b2d), SkBits2Float(0xf0685527));  // 3.35016e+24f, -2.87614e+29f
path.conicTo(SkBits2Float(0x03210831), SkBits2Float(0x6a4b7bc0), SkBits2Float(0x6829ed27), SkBits2Float(0x55555b2d), SkBits2Float(0x1e2a3a2a));  // 4.73231e-37f, 6.14991e+25f, 3.20982e+24f, 1.46617e+13f, 9.01175e-21f
path.conicTo(SkBits2Float(0x27202140), SkBits2Float(0x3a3b2729), SkBits2Float(0xc4371f20), SkBits2Float(0xecc52a22), SkBits2Float(0x21515d27));  // 2.22225e-15f, 0.000713932f, -732.486f, -1.90686e+27f, 7.09352e-19f
path.lineTo(SkBits2Float(0x68315b2d), SkBits2Float(0xf0685527));  // 3.35016e+24f, -2.87614e+29f
path.close();
path.moveTo(SkBits2Float(0x6829523a), SkBits2Float(0x2d555b2d));  // 3.19839e+24f, 1.21279e-11f
path.moveTo(SkBits2Float(0x68556829), SkBits2Float(0x555b2d29));  // 4.03114e+24f, 1.50617e+13f
path.moveTo(SkBits2Float(0x1f2a312a), SkBits2Float(0xc0032108));  // 3.60396e-20f, -2.04889f
path.cubicTo(SkBits2Float(0x68572d55), SkBits2Float(0xf05b684b), SkBits2Float(0x8c55272d), SkBits2Float(0x212a292a), SkBits2Float(0x0321082a), SkBits2Float(0x6a4b7bc0));  // 4.06458e+24f, -2.71613e+29f, -1.64207e-31f, 5.76527e-19f, 4.7323e-37f, 6.14991e+25f
path.conicTo(SkBits2Float(0x2a8c54ed), SkBits2Float(0x21081f21), SkBits2Float(0x4b7bc003), SkBits2Float(0x29ed846a), SkBits2Float(0x555b2d28));  // 2.49279e-13f, 4.61198e-19f, 1.64987e+07f, 1.05479e-13f, 1.50617e+13f
path.conicTo(SkBits2Float(0x68392d5b), SkBits2Float(0xf0682955), SkBits2Float(0x2a1f5b2d), SkBits2Float(0xef552a21), SkBits2Float(0x5b2d2a8c));  // 3.4979e+24f, -2.87402e+29f, 1.41537e-13f, -6.59712e+28f, 4.8742e+16f

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 0, filename);
}

static void fuzz763_24(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0xededed02), SkBits2Float(0xedededed));  // -9.20431e+27f, -9.20445e+27f
path.close();
path.moveTo(SkBits2Float(0xededed02), SkBits2Float(0xedededed));  // -9.20431e+27f, -9.20445e+27f
path.quadTo(SkBits2Float(0x9fb9c16e), SkBits2Float(0x27737375), SkBits2Float(0xb7c5ff00), SkBits2Float(0x00ff9908));  // -7.86706e-20f, 3.37856e-15f, -2.3603e-05f, 2.34729e-38f
path.moveTo(SkBits2Float(0x73737300), SkBits2Float(0x73735273));  // 1.9288e+31f, 1.9278e+31f
path.cubicTo(SkBits2Float(0x1616ecec), SkBits2Float(0x2c321616), SkBits2Float(0x3516c616), SkBits2Float(0x6e161616), SkBits2Float(0x4c416033), SkBits2Float(0xf6000000));  // 1.21917e-25f, 2.53076e-12f, 5.61676e-07f, 1.16124e+28f, 5.06923e+07f, -6.49037e+32f
path.quadTo(SkBits2Float(0x04007f41), SkBits2Float(0xecececec), SkBits2Float(0xecececec), SkBits2Float(0xecec41ec));  // 1.51048e-36f, -2.2914e+27f, -2.2914e+27f, -2.28494e+27f
path.lineTo(SkBits2Float(0x73737300), SkBits2Float(0x73735273));  // 1.9288e+31f, 1.9278e+31f
path.close();
path.moveTo(SkBits2Float(0x73737300), SkBits2Float(0x73735273));  // 1.9288e+31f, 1.9278e+31f
path.quadTo(SkBits2Float(0x000000ec), SkBits2Float(0xececcc00), SkBits2Float(0x48ececec), SkBits2Float(0x0278806e));  // 3.30706e-43f, -2.29016e+27f, 485223, 1.8257e-37f
path.lineTo(SkBits2Float(0x72ececec), SkBits2Float(0xecec02ec));  // 9.38559e+30f, -2.28256e+27f
path.quadTo(SkBits2Float(0xec04007f), SkBits2Float(0xecececec), SkBits2Float(0xecececec), SkBits2Float(0xecec0400));  // -6.38322e+26f, -2.2914e+27f, -2.2914e+27f, -2.2826e+27f
path.lineTo(SkBits2Float(0x73737300), SkBits2Float(0x73735273));  // 1.9288e+31f, 1.9278e+31f
path.close();
path.moveTo(SkBits2Float(0x73737300), SkBits2Float(0x73735273));  // 1.9288e+31f, 1.9278e+31f
path.quadTo(SkBits2Float(0x000040ec), SkBits2Float(0x3a333300), SkBits2Float(0xecec3333), SkBits2Float(0xececdbec));  // 2.32896e-41f, 0.000683591f, -2.28439e+27f, -2.29076e+27f
path.lineTo(SkBits2Float(0x3300007f), SkBits2Float(0x33d83333));  // 2.98028e-08f, 1.00676e-07f
path.lineTo(SkBits2Float(0x73737300), SkBits2Float(0x73735273));  // 1.9288e+31f, 1.9278e+31f
path.close();
path.moveTo(SkBits2Float(0x73737300), SkBits2Float(0x73735273));  // 1.9288e+31f, 1.9278e+31f
path.quadTo(SkBits2Float(0x9e9ea900), SkBits2Float(0x33ececec), SkBits2Float(0xececec33), SkBits2Float(0xec336e6e));  // -1.67988e-20f, 1.10327e-07f, -2.29138e+27f, -8.67677e+26f
path.lineTo(SkBits2Float(0x73737300), SkBits2Float(0x73735273));  // 1.9288e+31f, 1.9278e+31f
path.close();
path.moveTo(SkBits2Float(0x73737300), SkBits2Float(0x73735273));  // 1.9288e+31f, 1.9278e+31f
path.lineTo(SkBits2Float(0xedededed), SkBits2Float(0xedededed));  // -9.20445e+27f, -9.20445e+27f
path.lineTo(SkBits2Float(0xecececec), SkBits2Float(0xecececec));  // -2.2914e+27f, -2.2914e+27f
path.lineTo(SkBits2Float(0x73737300), SkBits2Float(0x73735273));  // 1.9288e+31f, 1.9278e+31f
path.close();
path.moveTo(SkBits2Float(0x73737300), SkBits2Float(0x73735273));  // 1.9288e+31f, 1.9278e+31f
path.lineTo(SkBits2Float(0x01003300), SkBits2Float(0x33d83333));  // 2.35465e-38f, 1.00676e-07f
path.quadTo(SkBits2Float(0xecec3333), SkBits2Float(0x04eeedec), SkBits2Float(0xe0e0e0e0), SkBits2Float(0x9ee0e0e0));  // -2.28439e+27f, 5.6172e-36f, -1.29634e+20f, -2.38099e-20f
path.lineTo(SkBits2Float(0x73737300), SkBits2Float(0x73735273));  // 1.9288e+31f, 1.9278e+31f
path.close();
path.moveTo(SkBits2Float(0x73737300), SkBits2Float(0x73735273));  // 1.9288e+31f, 1.9278e+31f
path.cubicTo(SkBits2Float(0x299e9e9e), SkBits2Float(0xecececec), SkBits2Float(0xececb6ec), SkBits2Float(0xf0ececec), SkBits2Float(0x0000ecec), SkBits2Float(0x9ebe6e6e));  // 7.04413e-14f, -2.2914e+27f, -2.28936e+27f, -5.86599e+29f, 8.49916e-41f, -2.01627e-20f
path.cubicTo(SkBits2Float(0x9e9e9e9e), SkBits2Float(0xe8009e9e), SkBits2Float(0x9e9e9e9e), SkBits2Float(0xecec9e9e), SkBits2Float(0xec3333ec), SkBits2Float(0xececf0ec));  // -1.67945e-20f, -2.42956e+24f, -1.67945e-20f, -2.28844e+27f, -8.66572e+26f, -2.29155e+27f

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 2, filename);
}

static void fuzz763_25(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x6a4b7bc4));  // 0, 6.14991e+25f
path.conicTo(SkBits2Float(0x653140d9), SkBits2Float(0x6a4b4f74), SkBits2Float(0x65906630), SkBits2Float(0x6a25a070), SkBits2Float(0x3f6728a2));  // 5.23159e+22f, 6.14468e+25f, 8.52382e+22f, 5.00576e+25f, 0.902964f
path.cubicTo(SkBits2Float(0x68295bc5), SkBits2Float(0x00000000), SkBits2Float(0x682958ff), SkBits2Float(0x00000000), SkBits2Float(0x68286829), SkBits2Float(0x00000000));  // 3.19909e+24f, 0, 3.19889e+24f, 0, 3.18112e+24f, 0
path.lineTo(SkBits2Float(0x68555b29), SkBits2Float(0x00000000));  // 4.03018e+24f, 0
path.conicTo(SkBits2Float(0x00000000), SkBits2Float(0x682d2927), SkBits2Float(0x00000000), SkBits2Float(0x00000000), SkBits2Float(0x6829686f));  // 0, 3.27091e+24f, 0, 0, 3.20003e+24f
path.lineTo(SkBits2Float(0xdf218a28), SkBits2Float(0x00000000));  // -1.16402e+19f, 0
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0x6a4b7bc4));  // 0, 6.14991e+25f
path.close();
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.conicTo(SkBits2Float(0x6642c40c), SkBits2Float(0x00000000), SkBits2Float(0x65906630), SkBits2Float(0x6a25a070), SkBits2Float(0x3edcd74d));  // 2.29939e+23f, 0, 8.52382e+22f, 5.00576e+25f, 0.43133f
path.conicTo(SkBits2Float(0x68295afa), SkBits2Float(0x00000000), SkBits2Float(0x00000000), SkBits2Float(0x00000000), SkBits2Float(0x4277a57b));  // 3.19903e+24f, 0, 0, 0, 61.9116f
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 4, filename);
}


static void fuzz763_26(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x68556829), SkBits2Float(0x555b2d29));  // 4.03114e+24f, 1.50617e+13f
path.moveTo(SkBits2Float(0x1f2a312a), SkBits2Float(0xc003210a));  // 3.60396e-20f, -2.04889f
path.cubicTo(SkBits2Float(0x68372d55), SkBits2Float(0xf05b684b), SkBits2Float(0x8c55272d), SkBits2Float(0x212a292a), SkBits2Float(0x0321082a), SkBits2Float(0x6a4b7bc0));  // 3.46012e+24f, -2.71613e+29f, -1.64207e-31f, 5.76527e-19f, 4.7323e-37f, 6.14991e+25f
path.conicTo(SkBits2Float(0x212a8ced), SkBits2Float(0x0321081f), SkBits2Float(0x6a4b7bc0), SkBits2Float(0x2829ed84), SkBits2Float(0x69555b2d));  // 5.77848e-19f, 4.7323e-37f, 6.14991e+25f, 9.43289e-15f, 1.61207e+25f
path.moveTo(SkBits2Float(0x68315b2d), SkBits2Float(0xf0682955));  // 3.35016e+24f, -2.87402e+29f
path.conicTo(SkBits2Float(0x212a1f5b), SkBits2Float(0x8cef552a), SkBits2Float(0x295b2d2a), SkBits2Float(0x68210368), SkBits2Float(0x7bc05508));  // 5.76397e-19f, -3.6875e-31f, 4.86669e-14f, 3.04146e+24f, 1.99729e+36f
path.lineTo(SkBits2Float(0x68315b2d), SkBits2Float(0xf0682955));  // 3.35016e+24f, -2.87402e+29f
path.close();
path.moveTo(SkBits2Float(0x68315b2d), SkBits2Float(0xf0682955));  // 3.35016e+24f, -2.87402e+29f
path.lineTo(SkBits2Float(0x5b2c6829), SkBits2Float(0x212a8c55));  // 4.85282e+16f, 5.7784e-19f
path.moveTo(SkBits2Float(0x0321081f), SkBits2Float(0x6a4b7bc0));  // 4.7323e-37f, 6.14991e+25f
path.conicTo(SkBits2Float(0x68385b2d), SkBits2Float(0x555bf055), SkBits2Float(0x2a1f2a8c), SkBits2Float(0x03212121), SkBits2Float(0x5a4b7bc0));  // 3.48239e+24f, 1.51141e+13f, 1.41368e-13f, 4.73517e-37f, 1.43189e+16f
path.conicTo(SkBits2Float(0xc08c2aed), SkBits2Float(0x211f2108), SkBits2Float(0x6a4b7b03), SkBits2Float(0x6829ed27), SkBits2Float(0x2d555b2d));  // -4.38024f, 5.3915e-19f, 6.14982e+25f, 3.20982e+24f, 1.21279e-11f
path.moveTo(SkBits2Float(0x68355b2d), SkBits2Float(0xf0685527));  // 3.42572e+24f, -2.87614e+29f
path.conicTo(SkBits2Float(0x2a8c555b), SkBits2Float(0x6e2a1f72), SkBits2Float(0x0321082a), SkBits2Float(0x6a4b7bc0), SkBits2Float(0x4793ed7a));  // 2.49282e-13f, 1.31626e+28f, 4.7323e-37f, 6.14991e+25f, 75739
path.lineTo(SkBits2Float(0x68355b2d), SkBits2Float(0xf0685527));  // 3.42572e+24f, -2.87614e+29f
path.close();
path.moveTo(SkBits2Float(0x68355b2d), SkBits2Float(0xf0685527));  // 3.42572e+24f, -2.87614e+29f
path.quadTo(SkBits2Float(0x2128282a), SkBits2Float(0x3a8a3adf), SkBits2Float(0x8a284f1a), SkBits2Float(0x2c213ab3));  // 5.69738e-19f, 0.00105461f, -8.10378e-33f, 2.29121e-12f
path.lineTo(SkBits2Float(0x68355b2d), SkBits2Float(0xf0685527));  // 3.42572e+24f, -2.87614e+29f
path.close();
path.moveTo(SkBits2Float(0x68355b2d), SkBits2Float(0xf0685527));  // 3.42572e+24f, -2.87614e+29f
path.quadTo(SkBits2Float(0x1d2a2928), SkBits2Float(0x43962be6), SkBits2Float(0x3a2a812a), SkBits2Float(0x2127ed29));  // 2.25206e-21f, 300.343f, 0.000650423f, 5.68957e-19f
path.conicTo(SkBits2Float(0x03210831), SkBits2Float(0x6a4b7bc0), SkBits2Float(0x6829ed27), SkBits2Float(0x55555b2d), SkBits2Float(0x1e2a3a2a));  // 4.73231e-37f, 6.14991e+25f, 3.20982e+24f, 1.46617e+13f, 9.01175e-21f
path.conicTo(SkBits2Float(0x27202140), SkBits2Float(0x3a3b2769), SkBits2Float(0xc4371f20), SkBits2Float(0xecc52a22), SkBits2Float(0x21512727));  // 2.22225e-15f, 0.000713936f, -732.486f, -1.90686e+27f, 7.08638e-19f
path.lineTo(SkBits2Float(0x68355b2d), SkBits2Float(0xf0685527));  // 3.42572e+24f, -2.87614e+29f
path.close();
path.moveTo(SkBits2Float(0x6829523a), SkBits2Float(0x2d555b2d));  // 3.19839e+24f, 1.21279e-11f
path.moveTo(SkBits2Float(0x68556829), SkBits2Float(0x5b2d5529));  // 4.03114e+24f, 4.87888e+16f
path.moveTo(SkBits2Float(0x1f2a322a), SkBits2Float(0xc0032108));  // 3.60404e-20f, -2.04889f
path.cubicTo(SkBits2Float(0x68572d55), SkBits2Float(0xf05bd24b), SkBits2Float(0x8c55272d), SkBits2Float(0x212a292a), SkBits2Float(0x0321082a), SkBits2Float(0xed4b7bc0));  // 4.06458e+24f, -2.72126e+29f, -1.64207e-31f, 5.76527e-19f, 4.7323e-37f, -3.93594e+27f
path.conicTo(SkBits2Float(0x212a8c6a), SkBits2Float(0x0329081f), SkBits2Float(0x6a4b7bc0), SkBits2Float(0x2829ed84), SkBits2Float(0x2d555b2d));  // 5.77841e-19f, 4.9674e-37f, 6.14991e+25f, 9.43289e-15f, 1.21279e-11f
path.moveTo(SkBits2Float(0x68385b2d), SkBits2Float(0xf0682955));  // 3.48239e+24f, -2.87402e+29f
path.conicTo(SkBits2Float(0x212a1f5b), SkBits2Float(0x8cef552a), SkBits2Float(0x295b2d2a), SkBits2Float(0x68210368), SkBits2Float(0x7bc05508));  // 5.76397e-19f, -3.6875e-31f, 4.86669e-14f, 3.04146e+24f, 1.99729e+36f
path.lineTo(SkBits2Float(0x68385b2d), SkBits2Float(0xf0682955));  // 3.48239e+24f, -2.87402e+29f
path.close();
path.moveTo(SkBits2Float(0x68385b2d), SkBits2Float(0xf0682955));  // 3.48239e+24f, -2.87402e+29f
path.lineTo(SkBits2Float(0x555b1b29), SkBits2Float(0x6c212a8c));  // 1.50569e+13f, 7.79352e+26f
path.conicTo(SkBits2Float(0x084b0321), SkBits2Float(0x6ac07b2a), SkBits2Float(0x395b2d7a), SkBits2Float(0x8c5bf055), SkBits2Float(0x1f212a3a));  // 6.10918e-34f, 1.16348e+26f, 0.000209024f, -1.69435e-31f, 3.4128e-20f
path.conicTo(SkBits2Float(0x290321d9), SkBits2Float(0x555b2d68), SkBits2Float(0x2a8c558c), SkBits2Float(0x2a212a1f), SkBits2Float(0x7bc00321));  // 2.91172e-14f, 1.50618e+13f, 2.49284e-13f, 1.43143e-13f, 1.99397e+36f
path.lineTo(SkBits2Float(0x68385b2d), SkBits2Float(0xf0682955));  // 3.48239e+24f, -2.87402e+29f
path.close();
path.moveTo(SkBits2Float(0x68385b2d), SkBits2Float(0xf0682955));  // 3.48239e+24f, -2.87402e+29f
path.lineTo(SkBits2Float(0x8c2aed7a), SkBits2Float(0x1f2128c0));  // -1.31678e-31f, 3.41268e-20f
path.lineTo(SkBits2Float(0x68385b2d), SkBits2Float(0xf0682955));  // 3.48239e+24f, -2.87402e+29f
path.close();

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 0, filename);
}

static void fuzz763_28(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x68556829), SkBits2Float(0x555b2d29));  // 4.03114e+24f, 1.50617e+13f
path.moveTo(SkBits2Float(0x1f2a312a), SkBits2Float(0xc0032108));  // 3.60396e-20f, -2.04889f
path.cubicTo(SkBits2Float(0x68302d55), SkBits2Float(0xf05b684b), SkBits2Float(0x8c55272d), SkBits2Float(0x212a1f2a), SkBits2Float(0x0321082a), SkBits2Float(0x6aa37bc0));  // 3.32789e+24f, -2.71613e+29f, -1.64207e-31f, 5.76395e-19f, 4.7323e-37f, 9.88197e+25f
path.conicTo(SkBits2Float(0x212a8ced), SkBits2Float(0x0321081f), SkBits2Float(0x6a4b7bc0), SkBits2Float(0x2d28ed84), SkBits2Float(0x5b2d2955));  // 5.77848e-19f, 4.7323e-37f, 6.14991e+25f, 9.60243e-12f, 4.87406e+16f
path.moveTo(SkBits2Float(0x6c395b2d), SkBits2Float(0xf0682955));  // 8.96327e+26f, -2.87402e+29f
path.conicTo(SkBits2Float(0x212a1f5b), SkBits2Float(0x2aef8c55), SkBits2Float(0x68295b2d), SkBits2Float(0x21086855), SkBits2Float(0x4b7bc003));  // 5.76397e-19f, 4.25523e-13f, 3.19905e+24f, 4.62167e-19f, 1.64987e+07f
path.lineTo(SkBits2Float(0x5b2c6829), SkBits2Float(0x212a8c55));  // 4.85282e+16f, 5.7784e-19f
path.moveTo(SkBits2Float(0x0321081f), SkBits2Float(0x6a4b7bc0));  // 4.7323e-37f, 6.14991e+25f
path.lineTo(SkBits2Float(0x8a283a28), SkBits2Float(0x284f1a3a));  // -8.09984e-33f, 1.14965e-14f
path.quadTo(SkBits2Float(0x1d2a2928), SkBits2Float(0x43962be6), SkBits2Float(0x272a812a), SkBits2Float(0x3a2a5529));  // 2.25206e-21f, 300.343f, 2.36623e-15f, 0.000649768f
path.lineTo(SkBits2Float(0x213b1e2a), SkBits2Float(0x27292720));  // 6.3398e-19f, 2.34747e-15f
path.conicTo(SkBits2Float(0x381f203a), SkBits2Float(0x2ac422c5), SkBits2Float(0xc25d27ec), SkBits2Float(0x3a705921), SkBits2Float(0x2a105152));  // 3.79386e-05f, 3.48407e-13f, -55.289f, 0.000916855f, 1.2818e-13f
path.quadTo(SkBits2Float(0x633ad912), SkBits2Float(0x29c80927), SkBits2Float(0x272927b0), SkBits2Float(0x683a5b2d));  // 3.44674e+21f, 8.88337e-14f, 2.3475e-15f, 3.52017e+24f
path.lineTo(SkBits2Float(0x295b2d68), SkBits2Float(0x29685568));  // 4.86672e-14f, 5.15884e-14f
path.conicTo(SkBits2Float(0xaa8c555b), SkBits2Float(0x081f2a21), SkBits2Float(0x5b2d0321), SkBits2Float(0x68556829), SkBits2Float(0x2a552d29));  // -2.49282e-13f, 4.78968e-34f, 4.86986e+16f, 4.03114e+24f, 1.89339e-13f
path.cubicTo(SkBits2Float(0x21295b2d), SkBits2Float(0x2a688c5b), SkBits2Float(0x68295b2d), SkBits2Float(0x2d296855), SkBits2Float(0x8c08555b), SkBits2Float(0x2a2a29ca));  // 5.73801e-19f, 2.06544e-13f, 3.19905e+24f, 9.6297e-12f, -1.05027e-31f, 1.51135e-13f
path.quadTo(SkBits2Float(0x68295b21), SkBits2Float(0x2d296855), SkBits2Float(0x2a8c555b), SkBits2Float(0x081f2a21));  // 3.19904e+24f, 9.6297e-12f, 2.49282e-13f, 4.78968e-34f
path.lineTo(SkBits2Float(0x0321081f), SkBits2Float(0x6a4b7bc0));  // 4.7323e-37f, 6.14991e+25f
path.close();
path.moveTo(SkBits2Float(0x0321081f), SkBits2Float(0x6a4b7bc0));  // 4.7323e-37f, 6.14991e+25f
path.conicTo(SkBits2Float(0x6a4b7bc0), SkBits2Float(0x5b2d6829), SkBits2Float(0x1f212a55), SkBits2Float(0x8ced7aba), SkBits2Float(0x3f2a212a));  // 6.14991e+25f, 4.88097e+16f, 3.41281e-20f, -3.65895e-31f, 0.664569f
path.lineTo(SkBits2Float(0x5b2d212d), SkBits2Float(0x2d556829));  // 4.87316e+16f, 1.21308e-11f
path.moveTo(SkBits2Float(0x68552968), SkBits2Float(0x5568295b));  // 4.02651e+24f, 1.5954e+13f
path.moveTo(SkBits2Float(0x5b2d2968), SkBits2Float(0x212a8c55));  // 4.87407e+16f, 5.7784e-19f
path.moveTo(SkBits2Float(0x0321081f), SkBits2Float(0x6a4b7bc0));  // 4.7323e-37f, 6.14991e+25f
path.conicTo(SkBits2Float(0x212a8ced), SkBits2Float(0x0321081f), SkBits2Float(0x6a3a7bc0), SkBits2Float(0x2147ed7a), SkBits2Float(0x28282a3a));  // 5.77848e-19f, 4.7323e-37f, 5.63611e+25f, 6.77381e-19f, 9.33503e-15f

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 0, filename);
}

static void fuzz763_27(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.quadTo(SkBits2Float(0x30309ab8), SkBits2Float(0x305b3030), SkBits2Float(0x00f53030), SkBits2Float(0x3a3a0000));  // 6.42483e-10f, 7.97402e-10f, 2.2517e-38f, 0.000709534f
path.quadTo(SkBits2Float(0xb8b8d5b8), SkBits2Float(0x0b0b0b03), SkBits2Float(0x0b0b0b0b), SkBits2Float(0x3a3a0b0b));  // -8.81361e-05f, 2.67787e-32f, 2.67787e-32f, 0.000709698f
path.quadTo(SkBits2Float(0xb8b8b8b8), SkBits2Float(0x0b1203b8), SkBits2Float(0x0b0b0b0b), SkBits2Float(0x3a3a2110));  // -8.80821e-05f, 2.81214e-32f, 2.67787e-32f, 0.000710026f

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 4, filename);
}

static void fuzz763_29(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0x743e0000));  // 0, 6.02134e+31f
path.cubicTo(SkBits2Float(0x74083cf1), SkBits2Float(0x74536e73), SkBits2Float(0x742ac4e4), SkBits2Float(0x7415f5be), SkBits2Float(0x7433ee3c), SkBits2Float(0x7405a69a));  // 4.31756e+31f, 6.70053e+31f, 5.41189e+31f, 4.75242e+31f, 5.70223e+31f, 4.23556e+31f
path.quadTo(SkBits2Float(0x74360ca0), SkBits2Float(0x7401e10c), SkBits2Float(0x7436a382), SkBits2Float(0x7401cc18));  // 5.76937e+31f, 4.11603e+31f, 5.78805e+31f, 4.11344e+31f
path.cubicTo(SkBits2Float(0x74374a91), SkBits2Float(0x7401ef19), SkBits2Float(0x74375c84), SkBits2Float(0x7404d9b9), SkBits2Float(0x7437868f), SkBits2Float(0x740bae8a));  // 5.80873e+31f, 4.11777e+31f, 5.81095e+31f, 4.2102e+31f, 5.81616e+31f, 4.42669e+31f
path.cubicTo(SkBits2Float(0x7437d6c1), SkBits2Float(0x7418b629), SkBits2Float(0x74387e9b), SkBits2Float(0x7433fbc5), SkBits2Float(0x743e2ff7), SkBits2Float(0x74655fa2));  // 5.82609e+31f, 4.83962e+31f, 5.84687e+31f, 5.7039e+31f, 6.02728e+31f, 7.26914e+31f
path.cubicTo(SkBits2Float(0x741ada75), SkBits2Float(0x74745717), SkBits2Float(0x73c106b4), SkBits2Float(0x74744e64), SkBits2Float(0x00000000), SkBits2Float(0x74744006));  // 4.9075e+31f, 7.74345e+31f, 3.05862e+31f, 7.74237e+31f, 0, 7.74059e+31f
path.cubicTo(SkBits2Float(0x00000000), SkBits2Float(0x74746c7c), SkBits2Float(0x74244dce), SkBits2Float(0x7474733e), SkBits2Float(0x74400000), SkBits2Float(0x74747445));  // 0, 7.7461e+31f, 5.207e+31f, 7.74693e+31f, 6.08472e+31f, 7.74706e+31f
path.cubicTo(SkBits2Float(0x743f5854), SkBits2Float(0x746f3659), SkBits2Float(0x743ebe05), SkBits2Float(0x746a3017), SkBits2Float(0x743e2ff7), SkBits2Float(0x74655fa2));  // 6.06397e+31f, 7.58094e+31f, 6.04486e+31f, 7.42171e+31f, 6.02728e+31f, 7.26914e+31f
path.cubicTo(SkBits2Float(0x7447a582), SkBits2Float(0x74615dee), SkBits2Float(0x744f74f6), SkBits2Float(0x745c4903), SkBits2Float(0x7455e7e6), SkBits2Float(0x7455d751));  // 6.32705e+31f, 7.14216e+31f, 6.57457e+31f, 6.98112e+31f, 6.77895e+31f, 6.77689e+31f
path.cubicTo(SkBits2Float(0x74747474), SkBits2Float(0x743750a4), SkBits2Float(0x74747474), SkBits2Float(0x73f46f0d), SkBits2Float(0x74747474), SkBits2Float(0x00000000));  // 7.74708e+31f, 5.80948e+31f, 7.74708e+31f, 3.87321e+31f, 7.74708e+31f, 0
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.lineTo(SkBits2Float(0xf0682955), SkBits2Float(0x211f5b2d));  // -2.87402e+29f, 5.3992e-19f
path.moveTo(SkBits2Float(0x2d2aff2d), SkBits2Float(0x74747474));  // 9.72004e-12f, 7.74708e+31f
path.cubicTo(SkBits2Float(0x7474748e), SkBits2Float(0x74747490), SkBits2Float(0x8c722174), SkBits2Float(0x181f0080), SkBits2Float(0x74c0e520), SkBits2Float(0x747d7463));  // 7.7471e+31f, 7.7471e+31f, -1.86531e-31f, 2.05505e-24f, 1.22262e+32f, 8.0323e+31f
path.cubicTo(SkBits2Float(0x7b005e4b), SkBits2Float(0xdf3a6a3a), SkBits2Float(0x2a3a2848), SkBits2Float(0x2d2d7821), SkBits2Float(0x8c55212d), SkBits2Float(0x2d2d2d24));  // 6.66526e+35f, -1.34326e+19f, 1.65341e-13f, 9.86059e-12f, -1.64189e-31f, 9.84393e-12f
path.conicTo(SkBits2Float(0xde28804c), SkBits2Float(0x28e03721), SkBits2Float(0x3329df28), SkBits2Float(0x2d291515), SkBits2Float(0x0568295b));  // -3.03545e+18f, 2.48929e-14f, 3.95513e-08f, 9.61122e-12f, 1.09162e-35f
path.conicTo(SkBits2Float(0x556a2d21), SkBits2Float(0x21088c2a), SkBits2Float(0x3a333303), SkBits2Float(0x5b293a8a), SkBits2Float(0x6855683b));  // 1.60925e+13f, 4.62641e-19f, 0.000683591f, 4.76336e+16f, 4.03115e+24f

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 0, filename);
}

static void fuzz763_30(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x1f2108c0), SkBits2Float(0x4b7b0321));  // 3.41003e-20f, 1.64503e+07f
path.lineTo(SkBits2Float(0x6829ed27), SkBits2Float(0x2d555b2d));  // 3.20982e+24f, 1.21279e-11f
path.moveTo(SkBits2Float(0x68305b2d), SkBits2Float(0xf0685527));  // 3.33127e+24f, -2.87614e+29f
path.conicTo(SkBits2Float(0x2a8c555b), SkBits2Float(0x6e2a1f72), SkBits2Float(0x0321082a), SkBits2Float(0x2a4b7bc0), SkBits2Float(0x68295b2d));  // 2.49282e-13f, 1.31626e+28f, 4.7323e-37f, 1.8073e-13f, 3.19905e+24f
path.lineTo(SkBits2Float(0x5b2d2968), SkBits2Float(0x212a8c55));  // 4.87407e+16f, 5.7784e-19f
path.moveTo(SkBits2Float(0x0321081f), SkBits2Float(0x4b7b28c0));  // 4.7323e-37f, 1.646e+07f
path.lineTo(SkBits2Float(0x2a8ced7a), SkBits2Float(0x2d081f21));  // 2.50338e-13f, 7.73762e-12f
path.moveTo(SkBits2Float(0x68556829), SkBits2Float(0x555b2d29));  // 4.03114e+24f, 1.50617e+13f
path.moveTo(SkBits2Float(0x1f2a312a), SkBits2Float(0xc0032108));  // 3.60396e-20f, -2.04889f
path.cubicTo(SkBits2Float(0x69392d55), SkBits2Float(0x2d5b684b), SkBits2Float(0x8c5527f0), SkBits2Float(0x212a1f2a), SkBits2Float(0x0321082a), SkBits2Float(0x6a4b7bc0));  // 1.39916e+25f, 1.24719e-11f, -1.64209e-31f, 5.76395e-19f, 4.7323e-37f, 6.14991e+25f
path.conicTo(SkBits2Float(0x212a8ced), SkBits2Float(0xed7a6a1f), SkBits2Float(0x3a214793), SkBits2Float(0x3328282a), SkBits2Float(0x3a8a3adf));  // 5.77848e-19f, -4.84372e+27f, 0.000615233f, 3.91521e-08f, 0.00105461f
path.conicTo(SkBits2Float(0x4be80304), SkBits2Float(0xdcdcdc15), SkBits2Float(0xdcdcdcdc), SkBits2Float(0x71dcdcdc), SkBits2Float(0x6c107164));  // 3.04102e+07f, -4.97332e+17f, -4.97339e+17f, 2.18732e+30f, 6.98483e+26f
path.conicTo(SkBits2Float(0x6c0f1d6c), SkBits2Float(0x8e406c6e), SkBits2Float(0x6c6c0200), SkBits2Float(0x6c6ce46c), SkBits2Float(0x6c6c6c6c));  // 6.92061e+26f, -2.3718e-30f, 1.14126e+27f, 1.14554e+27f, 1.14327e+27f
path.lineTo(SkBits2Float(0x1f2a312a), SkBits2Float(0xc0032108));  // 3.60396e-20f, -2.04889f
path.close();
path.moveTo(SkBits2Float(0x1f2a312a), SkBits2Float(0xc0032108));  // 3.60396e-20f, -2.04889f
path.quadTo(SkBits2Float(0x3ab38a28), SkBits2Float(0x3ac22c21), SkBits2Float(0x6c401057), SkBits2Float(0x6d6d6b64));  // 0.00136978f, 0.00148142f, 9.28764e+26f, 4.59236e+27f
path.cubicTo(SkBits2Float(0x6d6d6d6d), SkBits2Float(0x6d6d6d6d), SkBits2Float(0x286d6d6d), SkBits2Float(0x081d2a29), SkBits2Float(0x6d690321), SkBits2Float(0x6b6b026d));  // 4.59251e+27f, 4.59251e+27f, 1.31799e-14f, 4.7295e-34f, 4.50711e+27f, 2.84109e+26f

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 2, filename);
}

static void fuzz763_31(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0xd72a8c55), SkBits2Float(0x61081f2a));  // -1.8752e+14f, 1.56938e+20f
path.conicTo(SkBits2Float(0x6a4b7bc0), SkBits2Float(0x4793ed7a), SkBits2Float(0x282a3a21), SkBits2Float(0xdf3a2128), SkBits2Float(0x471ac575));  // 6.14991e+25f, 75739, 9.4495e-15f, -1.3412e+19f, 39621.5f
path.lineTo(SkBits2Float(0x28404040), SkBits2Float(0x552a298a));  // 1.06721e-14f, 1.16935e+13f
path.moveTo(SkBits2Float(0x212c685b), SkBits2Float(0x21081f2a));  // 5.8414e-19f, 4.61198e-19f
path.conicTo(SkBits2Float(0x6a4b7bc0), SkBits2Float(0x80ed7a3a), SkBits2Float(0x2a3a2147), SkBits2Float(0xdf212828), SkBits2Float(0x4f1a3a3a));  // 6.14991e+25f, -2.18089e-38f, 1.65317e-13f, -1.16126e+19f, 2.58751e+09f
path.lineTo(SkBits2Float(0x212c685b), SkBits2Float(0x21081f2a));  // 5.8414e-19f, 4.61198e-19f
path.close();
path.moveTo(SkBits2Float(0x212c685b), SkBits2Float(0x21081f2a));  // 5.8414e-19f, 4.61198e-19f
path.cubicTo(SkBits2Float(0x3ac2213a), SkBits2Float(0x432a2928), SkBits2Float(0x96812be6), SkBits2Float(0x272a1d2a), SkBits2Float(0x3a2a3529), SkBits2Float(0x3b1e2ab0));  // 0.00148109f, 170.161f, -2.08688e-25f, 2.3608e-15f, 0.000649291f, 0.00241343f
path.lineTo(SkBits2Float(0x212c685b), SkBits2Float(0x21081f2a));  // 5.8414e-19f, 4.61198e-19f
path.close();
path.moveTo(SkBits2Float(0x212c685b), SkBits2Float(0x21081f2a));  // 5.8414e-19f, 4.61198e-19f
path.cubicTo(SkBits2Float(0xc5272927), SkBits2Float(0x22383b39), SkBits2Float(0x1051523a), SkBits2Float(0x2927b029), SkBits2Float(0x685b2d27), SkBits2Float(0x5b2d6855));  // -2674.57f, 2.4968e-18f, 4.12813e-29f, 3.72342e-14f, 4.14012e+24f, 4.88099e+16f

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 4, filename);
}

static void fuzz763_33(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x72c185d5), SkBits2Float(0x72c184e8));  // 7.66623e+30f, 7.66608e+30f
path.quadTo(SkBits2Float(0x724341bf), SkBits2Float(0x72433fc4), SkBits2Float(0x6d757575), SkBits2Float(0x6d6d6d6d));  // 3.86746e+30f, 3.86731e+30f, 4.74786e+27f, 4.59251e+27f
path.cubicTo(SkBits2Float(0x6d18b5e5), SkBits2Float(0x6d6d6d6d), SkBits2Float(0x6cbe03bd), SkBits2Float(0x6d4b455b), SkBits2Float(0x6c6c69d8), SkBits2Float(0x6d20df31));  // 2.95385e+27f, 4.59251e+27f, 1.83771e+27f, 3.93183e+27f, 1.14323e+27f, 3.11171e+27f
path.conicTo(SkBits2Float(0x6c6c8b72), SkBits2Float(0x00000000), SkBits2Float(0x6c6c6c6c), SkBits2Float(0x00000000), SkBits2Float(0x400812df));  // 1.14386e+27f, 0, 1.14327e+27f, 0, 2.12615f
path.quadTo(SkBits2Float(0x72432acb), SkBits2Float(0x72432295), SkBits2Float(0x72c185d5), SkBits2Float(0x72c184e8));  // 3.86568e+30f, 3.86505e+30f, 7.66623e+30f, 7.66608e+30f
path.close();
path.moveTo(SkBits2Float(0x72c185d5), SkBits2Float(0x72c184e8));  // 7.66623e+30f, 7.66608e+30f
path.cubicTo(SkBits2Float(0x74f97d76), SkBits2Float(0x74f97d90), SkBits2Float(0x75381628), SkBits2Float(0x7538182c), SkBits2Float(0x7538153b), SkBits2Float(0x75381835));  // 1.58133e+32f, 1.58133e+32f, 2.33357e+32f, 2.33367e+32f, 2.33353e+32f, 2.33368e+32f
path.cubicTo(SkBits2Float(0x7538144e), SkBits2Float(0x7538183f), SkBits2Float(0x74f9760f), SkBits2Float(0x74f97ddd), SkBits2Float(0x72c185d5), SkBits2Float(0x72c184e8));  // 2.33348e+32f, 2.33368e+32f, 1.58115e+32f, 1.58134e+32f, 7.66623e+30f, 7.66608e+30f
path.close();
path.moveTo(SkBits2Float(0x6c6c69d8), SkBits2Float(0x6d20df31));  // 1.14323e+27f, 3.11171e+27f
path.conicTo(SkBits2Float(0x6c6c55ae), SkBits2Float(0x6d80b520), SkBits2Float(0x6c6c1071), SkBits2Float(0x6e0f1d6c), SkBits2Float(0x3f96e656));  // 1.14284e+27f, 4.97913e+27f, 1.14154e+27f, 1.1073e+28f, 1.1789f
path.lineTo(SkBits2Float(0x6a674231), SkBits2Float(0x6c0c3394));  // 6.98936e+25f, 6.77973e+26f
path.cubicTo(SkBits2Float(0x6b12c63f), SkBits2Float(0x6c881439), SkBits2Float(0x6bba4ae5), SkBits2Float(0x6ced1e23), SkBits2Float(0x6c6c69d8), SkBits2Float(0x6d20df31));  // 1.77439e+26f, 1.31608e+27f, 4.50428e+26f, 2.29326e+27f, 1.14323e+27f, 3.11171e+27f
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.lineTo(SkBits2Float(0x6c6b6ba7), SkBits2Float(0x886b6b6b));  // 1.13842e+27f, -7.0844e-34f
path.quadTo(SkBits2Float(0x0000206b), SkBits2Float(0x6d6d6d6d), SkBits2Float(0x6d6d6d6d), SkBits2Float(0x6d6d6d6d));  // 1.16294e-41f, 4.59251e+27f, 4.59251e+27f, 4.59251e+27f
path.conicTo(SkBits2Float(0x3e3e3e3e), SkBits2Float(0xafbcad20), SkBits2Float(0x78787878), SkBits2Float(0x78787829), SkBits2Float(0x78787878));  // 0.185784f, -3.432e-10f, 2.01583e+34f, 2.01582e+34f, 2.01583e+34f
path.lineTo(SkBits2Float(0x78787878), SkBits2Float(0x95066b78));  // 2.01583e+34f, -2.71459e-26f
path.lineTo(SkBits2Float(0x6c6b6ba7), SkBits2Float(0x886b6b6b));  // 1.13842e+27f, -7.0844e-34f
path.quadTo(SkBits2Float(0x0000206b), SkBits2Float(0x6d6d6d6d), SkBits2Float(0x6d6d6d6d), SkBits2Float(0x6d6d6d6d));  // 1.16294e-41f, 4.59251e+27f, 4.59251e+27f, 4.59251e+27f
path.conicTo(SkBits2Float(0x3e3e3e3e), SkBits2Float(0xafbcad20), SkBits2Float(0x78787878), SkBits2Float(0x78787829), SkBits2Float(0x78787878));  // 0.185784f, -3.432e-10f, 2.01583e+34f, 2.01582e+34f, 2.01583e+34f
path.lineTo(SkBits2Float(0x8787878f), SkBits2Float(0x87878787));  // -2.03922e-34f, -2.03922e-34f
path.lineTo(SkBits2Float(0x78787878), SkBits2Float(0x78787878));  // 2.01583e+34f, 2.01583e+34f
path.lineTo(SkBits2Float(0x78787878), SkBits2Float(0x78787878));  // 2.01583e+34f, 2.01583e+34f
path.lineTo(SkBits2Float(0x6c105778), SkBits2Float(0x6d406b64));  // 6.97994e+26f, 3.72193e+27f
path.cubicTo(SkBits2Float(0x7575756d), SkBits2Float(0x75757575), SkBits2Float(0x75757575), SkBits2Float(0x75757575), SkBits2Float(0x6d6d7575), SkBits2Float(0x6d6d6d6d));  // 3.11156e+32f, 3.11156e+32f, 3.11156e+32f, 3.11156e+32f, 4.59312e+27f, 4.59251e+27f
path.cubicTo(SkBits2Float(0x6d696d6d), SkBits2Float(0x026d6d6d), SkBits2Float(0x80bc6b6b), SkBits2Float(0xaebcdfd0), SkBits2Float(0x7878bcac), SkBits2Float(0x78787878));  // 4.51514e+27f, 1.74434e-37f, -1.73036e-38f, -8.58901e-11f, 2.01799e+34f, 2.01583e+34f
path.lineTo(SkBits2Float(0x78787878), SkBits2Float(0x78787878));  // 2.01583e+34f, 2.01583e+34f
path.lineTo(SkBits2Float(0x78787878), SkBits2Float(0x78787878));  // 2.01583e+34f, 2.01583e+34f
path.lineTo(SkBits2Float(0x78787878), SkBits2Float(0x78787878));  // 2.01583e+34f, 2.01583e+34f
path.lineTo(SkBits2Float(0xb4bcacbc), SkBits2Float(0xbcadbcbc));  // -3.51434e-07f, -0.0212082f
path.moveTo(SkBits2Float(0xa03aacbc), SkBits2Float(0x757575a0));  // -1.5812e-19f, 3.11157e+32f
path.close();

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 4, filename);
}

static void fuzz763_32(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.cubicTo(SkBits2Float(0xdedcdcdc), SkBits2Float(0xdcdcdcdc), SkBits2Float(0xdcdcdcdc), SkBits2Float(0xdcdcdcdc), SkBits2Float(0x55dcdcdc), SkBits2Float(0x29407d7f));  // -7.95742e+18f, -4.97339e+17f, -4.97339e+17f, -4.97339e+17f, 3.03551e+13f, 4.27414e-14f
path.cubicTo(SkBits2Float(0x7b93ed4b), SkBits2Float(0x29521472), SkBits2Float(0xdfc83c28), SkBits2Float(0x1a3a834e), SkBits2Float(0x6855e84f), SkBits2Float(0xf2f22a80));  // 1.53616e+36f, 4.66471e-14f, -2.88569e+19f, 3.857e-23f, 4.0406e+24f, -9.59318e+30f
path.moveTo(SkBits2Float(0xe0f2f210), SkBits2Float(0xc3f2eef2));  // -1.40049e+20f, -485.867f
path.cubicTo(SkBits2Float(0x108ced7a), SkBits2Float(0x7bc00308), SkBits2Float(0x287a6a3a), SkBits2Float(0x242847ed), SkBits2Float(0x2bcb302a), SkBits2Float(0xf21003e8));  // 5.55862e-29f, 1.99396e+36f, 1.39008e-14f, 3.64901e-17f, 1.44374e-12f, -2.85252e+30f
path.moveTo(SkBits2Float(0x556c0010), SkBits2Float(0x002a8768));  // 1.62178e+13f, 3.90567e-39f
path.quadTo(SkBits2Float(0xf2f22021), SkBits2Float(0xf2f2f56e), SkBits2Float(0xf2f2f2f2), SkBits2Float(0xf22040d9));  // -9.59158e+30f, -9.62459e+30f, -9.6242e+30f, -3.17414e+30f
path.lineTo(SkBits2Float(0xc013f2f2), SkBits2Float(0x0000294d));  // -2.3117f, 1.48159e-41f

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 0, filename);
}

static void fuzz763_34(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x63a95a6c), SkBits2Float(0x6cc8e7e2));  // 6.24803e+21f, 1.94304e+27f
path.quadTo(SkBits2Float(0x63690f37), SkBits2Float(0x6d0a3d9b), SkBits2Float(0x00000000), SkBits2Float(0x6d3e3e3e));  // 4.29919e+21f, 2.67396e+27f, 0, 3.67984e+27f
path.conicTo(SkBits2Float(0x6b9253fc), SkBits2Float(0x6c956a8b), SkBits2Float(0x6c6ac798), SkBits2Float(0x692a5d27), SkBits2Float(0x3e56eb72));  // 3.538e+26f, 1.44506e+27f, 1.13532e+27f, 1.28723e+25f, 0.209883f
path.lineTo(SkBits2Float(0x6c6c586c), SkBits2Float(0x00000000));  // 1.1429e+27f, 0
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.conicTo(SkBits2Float(0x6c8c6c6c), SkBits2Float(0x00000000), SkBits2Float(0x00000000), SkBits2Float(0x6cc8e82a), SkBits2Float(0x5b684b68));  // 1.35809e+27f, 0, 0, 1.94305e+27f, 6.53851e+16f
path.lineTo(SkBits2Float(0x63a95a6c), SkBits2Float(0x6cc8e7e2));  // 6.24803e+21f, 1.94304e+27f
path.close();
path.moveTo(SkBits2Float(0x63a95a6c), SkBits2Float(0x6cc8e7e2));  // 6.24803e+21f, 1.94304e+27f
path.quadTo(SkBits2Float(0x641ae35f), SkBits2Float(0x00000000), SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 1.14287e+22f, 0, 0, 0
path.lineTo(SkBits2Float(0x6c6c586c), SkBits2Float(0x00000000));  // 1.1429e+27f, 0
path.conicTo(SkBits2Float(0x6c6ba1fc), SkBits2Float(0x688c9eb1), SkBits2Float(0x6c6ac798), SkBits2Float(0x692a5d27), SkBits2Float(0x3f7fec32));  // 1.13945e+27f, 5.31247e+24f, 1.13532e+27f, 1.28723e+25f, 0.999698f
path.lineTo(SkBits2Float(0x63a95a6c), SkBits2Float(0x6cc8e7e2));  // 6.24803e+21f, 1.94304e+27f
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x6c3e3e3e), SkBits2Float(0x586c79ff));  // 9.19959e+26f, 1.04003e+15f
path.quadTo(SkBits2Float(0x6c6c4a6c), SkBits2Float(0x6c6c6c6c), SkBits2Float(0xc83e6c6c), SkBits2Float(0x3e313e3e));  // 1.14263e+27f, 1.14327e+27f, -194994, 0.173089f

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 2, filename);
}

static void fuzz763_36(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x68556829), SkBits2Float(0x555b2d29));  // 4.03114e+24f, 1.50617e+13f
path.moveTo(SkBits2Float(0x1f2a312a), SkBits2Float(0xc0032108));  // 3.60396e-20f, -2.04889f
path.cubicTo(SkBits2Float(0x68392d55), SkBits2Float(0xf05b684b), SkBits2Float(0x8c55272d), SkBits2Float(0x212a292a), SkBits2Float(0x0321082a), SkBits2Float(0x6a4b7bc0));  // 3.4979e+24f, -2.71613e+29f, -1.64207e-31f, 5.76527e-19f, 4.7323e-37f, 6.14991e+25f
path.conicTo(SkBits2Float(0x212a8ced), SkBits2Float(0x0321081f), SkBits2Float(0x6a4b7bc0), SkBits2Float(0x2829ed84), SkBits2Float(0x2d555b2d));  // 5.77848e-19f, 4.7323e-37f, 6.14991e+25f, 9.43289e-15f, 1.21279e-11f
path.moveTo(SkBits2Float(0xe8355b2d), SkBits2Float(0xf0682955));  // -3.42572e+24f, -2.87402e+29f
path.conicTo(SkBits2Float(0x212a1f5b), SkBits2Float(0x8cef552a), SkBits2Float(0x295b2d2a), SkBits2Float(0x68210368), SkBits2Float(0x7bc05508));  // 5.76397e-19f, -3.6875e-31f, 4.86669e-14f, 3.04146e+24f, 1.99729e+36f
path.lineTo(SkBits2Float(0xe8355b2d), SkBits2Float(0xf0682955));  // -3.42572e+24f, -2.87402e+29f
path.close();
path.moveTo(SkBits2Float(0xe8355b2d), SkBits2Float(0xf0682955));  // -3.42572e+24f, -2.87402e+29f
path.lineTo(SkBits2Float(0x5b2c6829), SkBits2Float(0x212a8c55));  // 4.85282e+16f, 5.7784e-19f
path.conicTo(SkBits2Float(0x212a081f), SkBits2Float(0x4b7bc003), SkBits2Float(0x5b2d7a6a), SkBits2Float(0xf0556839), SkBits2Float(0x2a8c555b));  // 5.7609e-19f, 1.64987e+07f, 4.88298e+16f, -2.64185e+29f, 2.49282e-13f
path.conicTo(SkBits2Float(0xf42a212a), SkBits2Float(0x4b7bc003), SkBits2Float(0x2aed7a39), SkBits2Float(0x2108c08c), SkBits2Float(0x7b03211f));  // -5.39162e+31f, 1.64987e+07f, 4.21845e-13f, 4.63334e-19f, 6.80863e+35f
path.lineTo(SkBits2Float(0xe8355b2d), SkBits2Float(0xf0682955));  // -3.42572e+24f, -2.87402e+29f
path.close();
path.moveTo(SkBits2Float(0xe8355b2d), SkBits2Float(0xf0682955));  // -3.42572e+24f, -2.87402e+29f
path.lineTo(SkBits2Float(0x6829ed27), SkBits2Float(0x2d555b2d));  // 3.20982e+24f, 1.21279e-11f
path.moveTo(SkBits2Float(0x68275b2d), SkBits2Float(0xf0685527));  // 3.16127e+24f, -2.87614e+29f
path.conicTo(SkBits2Float(0x2a8c555b), SkBits2Float(0x212a1f72), SkBits2Float(0x03210807), SkBits2Float(0x6a4b7b28), SkBits2Float(0x4793ed7a));  // 2.49282e-13f, 5.76399e-19f, 4.73229e-37f, 6.14984e+25f, 75739
path.lineTo(SkBits2Float(0x68275b2d), SkBits2Float(0xf0685527));  // 3.16127e+24f, -2.87614e+29f
path.close();
path.moveTo(SkBits2Float(0x68275b2d), SkBits2Float(0xf0685527));  // 3.16127e+24f, -2.87614e+29f
path.quadTo(SkBits2Float(0x282a282a), SkBits2Float(0x8a3adf21), SkBits2Float(0x284f1a3a), SkBits2Float(0x213ab38a));  // 9.4456e-15f, -8.99754e-33f, 1.14965e-14f, 6.32569e-19f
path.lineTo(SkBits2Float(0x68275b2d), SkBits2Float(0xf0685527));  // 3.16127e+24f, -2.87614e+29f
path.close();
path.moveTo(SkBits2Float(0x68275b2d), SkBits2Float(0xf0685527));  // 3.16127e+24f, -2.87614e+29f
path.quadTo(SkBits2Float(0x1d2a2928), SkBits2Float(0x43962be6), SkBits2Float(0x3a20002a), SkBits2Float(0x2a8ced29));  // 2.25206e-21f, 300.343f, 0.000610354f, 2.50336e-13f
path.lineTo(SkBits2Float(0x68275b2d), SkBits2Float(0xf0685527));  // 3.16127e+24f, -2.87614e+29f
path.close();
path.moveTo(SkBits2Float(0x68275b2d), SkBits2Float(0xf0685527));  // 3.16127e+24f, -2.87614e+29f
path.conicTo(SkBits2Float(0xed210830), SkBits2Float(0xc04b6a03), SkBits2Float(0x68297b27), SkBits2Float(0x55555b2d), SkBits2Float(0x2ab03a2a));  // -3.11481e+27f, -3.17835f, 3.20141e+24f, 1.46617e+13f, 3.13042e-13f
path.quadTo(SkBits2Float(0x2720213b), SkBits2Float(0x3a3b2729), SkBits2Float(0xc4341f20), SkBits2Float(0xecc52a22));  // 2.22225e-15f, 0.000713932f, -720.486f, -1.90686e+27f
path.cubicTo(SkBits2Float(0x5921c25d), SkBits2Float(0x29523a70), SkBits2Float(0x555b2d68), SkBits2Float(0x1f212a8c), SkBits2Float(0x0321d90a), SkBits2Float(0x5b2d6829));  // 2.8457e+15f, 4.66801e-14f, 1.50618e+13f, 3.41283e-20f, 4.75628e-37f, 4.88097e+16f
path.lineTo(SkBits2Float(0x1f2a2a8c), SkBits2Float(0x03210821));  // 3.60341e-20f, 4.7323e-37f
path.lineTo(SkBits2Float(0x68275b2d), SkBits2Float(0xf0685527));  // 3.16127e+24f, -2.87614e+29f
path.close();
path.moveTo(SkBits2Float(0x68275b2d), SkBits2Float(0xf0685527));  // 3.16127e+24f, -2.87614e+29f
path.conicTo(SkBits2Float(0x2eed6a7a), SkBits2Float(0x282a3a21), SkBits2Float(0x3a21df28), SkBits2Float(0x4f1a3a8a), SkBits2Float(0x3ab38a28));  // 1.07964e-10f, 9.4495e-15f, 0.000617492f, 2.58753e+09f, 0.00136978f
path.lineTo(SkBits2Float(0x68275b2d), SkBits2Float(0xf0685527));  // 3.16127e+24f, -2.87614e+29f
path.close();
path.moveTo(SkBits2Float(0x68275b2d), SkBits2Float(0xf0685527));  // 3.16127e+24f, -2.87614e+29f
path.quadTo(SkBits2Float(0xe61d2a28), SkBits2Float(0x2a43962b), SkBits2Float(0x29272a81), SkBits2Float(0x2bb02a55));  // -1.85547e+23f, 1.73716e-13f, 3.71183e-14f, 1.25173e-12f
path.quadTo(SkBits2Float(0x2720213b), SkBits2Float(0x3ac52729), SkBits2Float(0xc4223b32), SkBits2Float(0x6c2a201f));  // 2.22225e-15f, 0.00150416f, -648.925f, 8.22676e+26f

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 0, filename);
}

static void fuzz763_35(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x2aed2a8c), SkBits2Float(0x03210a1f));  // 4.21292e-13f, 4.73253e-37f
path.conicTo(SkBits2Float(0x0000007b), SkBits2Float(0x7474747f), SkBits2Float(0x74747474), SkBits2Float(0x747474c4), SkBits2Float(0x74747474));  // 1.7236e-43f, 7.74709e+31f, 7.74708e+31f, 7.74712e+31f, 7.74708e+31f
path.quadTo(SkBits2Float(0x74747474), SkBits2Float(0x74747474), SkBits2Float(0x20437474), SkBits2Float(0x43a52b02));  // 7.74708e+31f, 7.74708e+31f, 1.65557e-19f, 330.336f
path.moveTo(SkBits2Float(0x3a214781), SkBits2Float(0x2128282a));  // 0.000615232f, 5.69738e-19f
path.lineTo(SkBits2Float(0x4b7bd603), SkBits2Float(0x6cf33b6a));  // 1.65043e+07f, 2.3524e+27f
path.conicTo(SkBits2Float(0x35778caa), SkBits2Float(0x0000002a), SkBits2Float(0x74742164), SkBits2Float(0x2a3a7474), SkBits2Float(0x4cc22157));  // 9.22194e-07f, 5.88545e-44f, 7.7368e+31f, 1.65605e-13f, 1.0178e+08f
path.cubicTo(SkBits2Float(0x21479321), SkBits2Float(0x23434cc2), SkBits2Float(0x3a214793), SkBits2Float(0x2128282a), SkBits2Float(0x323adf81), SkBits2Float(0x77291a3a));  // 6.76185e-19f, 1.05872e-17f, 0.000615233f, 5.69738e-19f, 1.08774e-08f, 3.42981e+33f
path.conicTo(SkBits2Float(0x0000002a), SkBits2Float(0x7474743e), SkBits2Float(0x74747474), SkBits2Float(0x74746474), SkBits2Float(0x74747474));  // 5.88545e-44f, 7.74706e+31f, 7.74708e+31f, 7.7451e+31f, 7.74708e+31f
path.cubicTo(SkBits2Float(0x21e7fc06), SkBits2Float(0x2a212a59), SkBits2Float(0x0321081f), SkBits2Float(0x00002a35), SkBits2Float(0x74744000), SkBits2Float(0x2974e874));  // 1.57199e-18f, 1.43144e-13f, 4.7323e-37f, 1.5141e-41f, 7.74059e+31f, 5.43805e-14f
path.cubicTo(SkBits2Float(0x74647474), SkBits2Float(0x74747474), SkBits2Float(0x12ec7474), SkBits2Float(0x4cc22147), SkBits2Float(0x47932343), SkBits2Float(0x282a3a21));  // 7.24002e+31f, 7.74708e+31f, 1.49224e-27f, 1.0178e+08f, 75334.5f, 9.4495e-15f
path.lineTo(SkBits2Float(0x3a214781), SkBits2Float(0x2128282a));  // 0.000615232f, 5.69738e-19f
path.close();
path.moveTo(SkBits2Float(0x3a214781), SkBits2Float(0x2128282a));  // 0.000615232f, 5.69738e-19f
path.conicTo(SkBits2Float(0x3a323adf), SkBits2Float(0x4977291a), SkBits2Float(0x0000002a), SkBits2Float(0x7474743e), SkBits2Float(0x74747474));  // 0.000679893f, 1.01237e+06f, 5.88545e-44f, 7.74706e+31f, 7.74708e+31f
path.cubicTo(SkBits2Float(0x74747464), SkBits2Float(0x74747474), SkBits2Float(0x21e7fc06), SkBits2Float(0x2a212a59), SkBits2Float(0x0321081f), SkBits2Float(0x00002a35));  // 7.74708e+31f, 7.74708e+31f, 1.57199e-18f, 1.43144e-13f, 4.7323e-37f, 1.5141e-41f
path.moveTo(SkBits2Float(0x74747440), SkBits2Float(0x742974e8));  // 7.74706e+31f, 5.3703e+31f
path.cubicTo(SkBits2Float(0x74746474), SkBits2Float(0x74747474), SkBits2Float(0xd912ec74), SkBits2Float(0x553a3728), SkBits2Float(0x29202a8c), SkBits2Float(0x5555201b));  // 7.7451e+31f, 7.74708e+31f, -2.58471e+15f, 1.27966e+13f, 3.5564e-14f, 1.46459e+13f
path.moveTo(SkBits2Float(0x31292768), SkBits2Float(0x212d2aff));  // 2.46151e-09f, 5.86716e-19f
path.quadTo(SkBits2Float(0x2128282a), SkBits2Float(0x323adf81), SkBits2Float(0x77291a3a), SkBits2Float(0x00002a49));  // 5.69738e-19f, 1.08774e-08f, 3.42981e+33f, 1.51691e-41f
path.moveTo(SkBits2Float(0x7474743e), SkBits2Float(0x74747474));  // 7.74706e+31f, 7.74708e+31f
path.cubicTo(SkBits2Float(0x74747464), SkBits2Float(0x74747474), SkBits2Float(0x21e7fc06), SkBits2Float(0x2a212a59), SkBits2Float(0x0321081f), SkBits2Float(0x00002a35));  // 7.74708e+31f, 7.74708e+31f, 1.57199e-18f, 1.43144e-13f, 4.7323e-37f, 1.5141e-41f
path.moveTo(SkBits2Float(0x74747440), SkBits2Float(0x74747474));  // 7.74706e+31f, 7.74708e+31f
path.cubicTo(SkBits2Float(0x74747464), SkBits2Float(0x74747474), SkBits2Float(0x43747474), SkBits2Float(0xa52b0220), SkBits2Float(0x47812a43), SkBits2Float(0x282a3a21));  // 7.74708e+31f, 7.74708e+31f, 244.455f, -1.48326e-16f, 66132.5f, 9.4495e-15f
path.lineTo(SkBits2Float(0x74747440), SkBits2Float(0x74747474));  // 7.74706e+31f, 7.74708e+31f
path.close();
path.moveTo(SkBits2Float(0x74747440), SkBits2Float(0x74747474));  // 7.74706e+31f, 7.74708e+31f
path.conicTo(SkBits2Float(0x3a323adf), SkBits2Float(0x19433b1a), SkBits2Float(0x5921e7fc), SkBits2Float(0x1f2a212a), SkBits2Float(0x35032108));  // 0.000679893f, 1.00932e-23f, 2.84828e+15f, 3.60263e-20f, 4.88494e-07f

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 4, filename);
}

static void fuzz763_37(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x5568392a), SkBits2Float(0x5b2d3368));  // 1.59583e+13f, 4.87517e+16f
path.conicTo(SkBits2Float(0x5b2d555b), SkBits2Float(0x68275b2d), SkBits2Float(0x21685527), SkBits2Float(0x0321082a), SkBits2Float(0x6ab485c0));  // 4.8789e+16f, 3.16127e+24f, 7.87174e-19f, 4.7323e-37f, 1.09119e+26f
path.conicTo(SkBits2Float(0x212a8ced), SkBits2Float(0x0321081f), SkBits2Float(0x6a4b7bc0), SkBits2Float(0x2829ed84), SkBits2Float(0x5b2d2d55));  // 5.77848e-19f, 4.7323e-37f, 6.14991e+25f, 9.43289e-15f, 4.8745e+16f
path.moveTo(SkBits2Float(0x6839552d), SkBits2Float(0xf0683b5b));  // 3.50084e+24f, -2.87489e+29f
path.conicTo(SkBits2Float(0x212a1f5b), SkBits2Float(0x2a8cef2a), SkBits2Float(0x682d2953), SkBits2Float(0xee682103), SkBits2Float(0x4b7bc055));  // 5.76397e-19f, 2.50349e-13f, 3.27093e+24f, -1.79601e+28f, 1.64988e+07f
path.lineTo(SkBits2Float(0x5b2c6829), SkBits2Float(0x212a8c55));  // 4.85282e+16f, 5.7784e-19f
path.conicTo(SkBits2Float(0x4b03213b), SkBits2Float(0xc07b2a08), SkBits2Float(0x5b2d7a6a), SkBits2Float(0xf0556830), SkBits2Float(0x2a8c555b));  // 8.59372e+06f, -3.92444f, 4.88298e+16f, -2.64185e+29f, 2.49282e-13f
path.conicTo(SkBits2Float(0x0321212a), SkBits2Float(0x4b7bd2c0), SkBits2Float(0xed7ac039), SkBits2Float(0x2f218c08), SkBits2Float(0x1f037b2a));  // 4.73517e-37f, 1.65035e+07f, -4.85023e+27f, 1.46926e-10f, 2.78422e-20f
path.lineTo(SkBits2Float(0x6839552d), SkBits2Float(0xf0683b5b));  // 3.50084e+24f, -2.87489e+29f
path.close();
path.moveTo(SkBits2Float(0x6839552d), SkBits2Float(0xf0683b5b));  // 3.50084e+24f, -2.87489e+29f
path.lineTo(SkBits2Float(0x6829ed27), SkBits2Float(0x2d555b2d));  // 3.20982e+24f, 1.21279e-11f
path.moveTo(SkBits2Float(0x68275b2d), SkBits2Float(0xf0685527));  // 3.16127e+24f, -2.87614e+29f
path.conicTo(SkBits2Float(0x721f2a5b), SkBits2Float(0x212a8c55), SkBits2Float(0x0321082a), SkBits2Float(0x6a4b7b28), SkBits2Float(0x4793ed7a));  // 3.1526e+30f, 5.7784e-19f, 4.7323e-37f, 6.14984e+25f, 75739
path.lineTo(SkBits2Float(0x68275b2d), SkBits2Float(0xf0685527));  // 3.16127e+24f, -2.87614e+29f
path.close();
path.moveTo(SkBits2Float(0x68275b2d), SkBits2Float(0xf0685527));  // 3.16127e+24f, -2.87614e+29f
path.quadTo(SkBits2Float(0x28282a2a), SkBits2Float(0x2c682921), SkBits2Float(0x8c555bf6), SkBits2Float(0x6d03de30));  // 9.33502e-15f, 3.2992e-12f, -1.64366e-31f, 2.5507e+27f
path.cubicTo(SkBits2Float(0x68392d55), SkBits2Float(0xf05b684b), SkBits2Float(0x8c55272d), SkBits2Float(0x212a292a), SkBits2Float(0x0321082a), SkBits2Float(0x081f2a21));  // 3.4979e+24f, -2.71613e+29f, -1.64207e-31f, 5.76527e-19f, 4.7323e-37f, 4.78968e-34f
path.lineTo(SkBits2Float(0x68275b2d), SkBits2Float(0xf0685527));  // 3.16127e+24f, -2.87614e+29f
path.close();
path.moveTo(SkBits2Float(0x68275b2d), SkBits2Float(0xf0685527));  // 3.16127e+24f, -2.87614e+29f
path.conicTo(SkBits2Float(0x6a4b7bc0), SkBits2Float(0xdf93ed7a), SkBits2Float(0x1a3a803a), SkBits2Float(0xb38a294f), SkBits2Float(0x3ac2213a));  // 6.14991e+25f, -2.13186e+19f, 3.85675e-23f, -6.43364e-08f, 0.00148109f
path.lineTo(SkBits2Float(0x68275b2d), SkBits2Float(0xf0685527));  // 3.16127e+24f, -2.87614e+29f
path.close();
path.moveTo(SkBits2Float(0x68275b2d), SkBits2Float(0xf0685527));  // 3.16127e+24f, -2.87614e+29f
path.conicTo(SkBits2Float(0xe62b291d), SkBits2Float(0x2a812a43), SkBits2Float(0x8ced093a), SkBits2Float(0xb38a5c5c), SkBits2Float(0x3ac2213a));  // -2.02071e+23f, 2.29443e-13f, -3.65212e-31f, -6.44293e-08f, 0.00148109f
path.lineTo(SkBits2Float(0x68275b2d), SkBits2Float(0xf0685527));  // 3.16127e+24f, -2.87614e+29f
path.close();
path.moveTo(SkBits2Float(0x68275b2d), SkBits2Float(0xf0685527));  // 3.16127e+24f, -2.87614e+29f
path.lineTo(SkBits2Float(0x8ced293a), SkBits2Float(0x5c5c5c5c));  // -3.65404e-31f, 2.48104e+17f
path.moveTo(SkBits2Float(0x21081f21), SkBits2Float(0x4b7bc003));  // 4.61198e-19f, 1.64987e+07f
path.lineTo(SkBits2Float(0x2829ed84), SkBits2Float(0x5b2d2d55));  // 9.43289e-15f, 4.8745e+16f
path.moveTo(SkBits2Float(0x6839552d), SkBits2Float(0xf0683b5a));  // 3.50084e+24f, -2.87489e+29f
path.lineTo(SkBits2Float(0x682d2952), SkBits2Float(0xee682103));  // 3.27093e+24f, -1.79601e+28f
path.lineTo(SkBits2Float(0x5b2c6829), SkBits2Float(0x2a3b0355));  // 4.85282e+16f, 1.66101e-13f
path.lineTo(SkBits2Float(0x6839552d), SkBits2Float(0xf0683b5a));  // 3.50084e+24f, -2.87489e+29f
path.close();
path.moveTo(SkBits2Float(0x6839552d), SkBits2Float(0xf0683b5a));  // 3.50084e+24f, -2.87489e+29f
path.conicTo(SkBits2Float(0x084b218c), SkBits2Float(0x6ac07b2a), SkBits2Float(0x395b2d7a), SkBits2Float(0x5bf05568), SkBits2Float(0x1f2a8c55));  // 6.11275e-34f, 1.16348e+26f, 0.000209024f, 1.35296e+17f, 3.6115e-20f

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.cubicTo(SkBits2Float(0xbcb4bcac), SkBits2Float(0x000029ff), SkBits2Float(0x010000bc), SkBits2Float(0x00bcbc00), SkBits2Float(0xbebcbcbc), SkBits2Float(0xb6aebcae));  // -0.0220626f, 1.50654e-41f, 2.35104e-38f, 1.73325e-38f, -0.368627f, -5.20757e-06f

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 4, filename);
}

static void fuzz763_38(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.conicTo(SkBits2Float(0x5b682968), SkBits2Float(0x5b292d11), SkBits2Float(0x212a8c55), SkBits2Float(0x555b2d2d), SkBits2Float(0x52525268));  // 6.53477e+16f, 4.76188e+16f, 5.7784e-19f, 1.50617e+13f, 2.25831e+11f
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.close();
path.moveTo(SkBits2Float(0xa5252600), SkBits2Float(0x52b4adad));  // -1.43243e-16f, 3.88004e+11f
path.close();
path.moveTo(SkBits2Float(0xa5252600), SkBits2Float(0x52b4adad));  // -1.43243e-16f, 3.88004e+11f
path.quadTo(SkBits2Float(0x72727270), SkBits2Float(0x52525272), SkBits2Float(0x2ac05252), SkBits2Float(0x727fb721));  // 4.80216e+30f, 2.25832e+11f, 3.41632e-13f, 5.06496e+30f
path.lineTo(SkBits2Float(0x73727322), SkBits2Float(0x555b2d29));  // 1.92088e+31f, 1.50617e+13f
path.lineTo(SkBits2Float(0xab2a212e), SkBits2Float(0x7a27872a));  // -6.04422e-13f, 2.17464e+35f
path.moveTo(SkBits2Float(0x25fffefb), SkBits2Float(0x7bc00321));  // 4.44082e-16f, 1.99397e+36f
path.quadTo(SkBits2Float(0x52524852), SkBits2Float(0x72525228), SkBits2Float(0x72727272), SkBits2Float(0x3a727272));  // 2.25789e+11f, 4.16584e+30f, 4.80216e+30f, 0.000924862f
path.lineTo(SkBits2Float(0x25fffefb), SkBits2Float(0x7bc00321));  // 4.44082e-16f, 1.99397e+36f
path.close();
path.moveTo(SkBits2Float(0x25fffefb), SkBits2Float(0x7bc00321));  // 4.44082e-16f, 1.99397e+36f
path.quadTo(SkBits2Float(0x2a292827), SkBits2Float(0x962b0080), SkBits2Float(0x5252752a), SkBits2Float(0x72725252));  // 1.50241e-13f, -1.38134e-25f, 2.25977e+11f, 4.79967e+30f
path.quadTo(SkBits2Float(0x72725252), SkBits2Float(0x52525272), SkBits2Float(0x72525252), SkBits2Float(0x72727272));  // 4.79967e+30f, 2.25832e+11f, 4.16585e+30f, 4.80216e+30f
path.quadTo(SkBits2Float(0x72727255), SkBits2Float(0xda000072), SkBits2Float(0x52525ada), SkBits2Float(0x52525252));  // 4.80215e+30f, -9.00732e+15f, 2.25867e+11f, 2.25831e+11f
path.quadTo(SkBits2Float(0x72727272), SkBits2Float(0x52525272), SkBits2Float(0x72525248), SkBits2Float(0x72727272));  // 4.80216e+30f, 2.25832e+11f, 4.16584e+30f, 4.80216e+30f
path.quadTo(SkBits2Float(0x72727255), SkBits2Float(0xda007b72), SkBits2Float(0x52525ada), SkBits2Float(0x52525252));  // 4.80215e+30f, -9.04113e+15f, 2.25867e+11f, 2.25831e+11f
path.quadTo(SkBits2Float(0x86727272), SkBits2Float(0x5252528d), SkBits2Float(0x72525252), SkBits2Float(0x72727227));  // -4.55992e-35f, 2.25832e+11f, 4.16585e+30f, 4.80214e+30f
path.quadTo(SkBits2Float(0x72727272), SkBits2Float(0x29217272), SkBits2Float(0xc003211c), SkBits2Float(0x556a4b7b));  // 4.80216e+30f, 3.58484e-14f, -2.0489f, 1.61006e+13f
path.moveTo(SkBits2Float(0x72557272), SkBits2Float(0x00727272));  // 4.22775e+30f, 1.05103e-38f
path.moveTo(SkBits2Float(0x5a61dada), SkBits2Float(0x52525252));  // 1.58931e+16f, 2.25831e+11f
path.close();
path.moveTo(SkBits2Float(0x5a61dada), SkBits2Float(0x52525252));  // 1.58931e+16f, 2.25831e+11f
path.quadTo(SkBits2Float(0x72727272), SkBits2Float(0x3a727272), SkBits2Float(0x28273ac2), SkBits2Float(0x00802a29));  // 4.80216e+30f, 0.000924862f, 9.2831e-15f, 1.17701e-38f
path.lineTo(SkBits2Float(0x52752a96), SkBits2Float(0x72525252));  // 2.63245e+11f, 4.16585e+30f
path.quadTo(SkBits2Float(0x72525272), SkBits2Float(0x52527272), SkBits2Float(0x52525252), SkBits2Float(0x72727272));  // 4.16586e+30f, 2.25966e+11f, 2.25831e+11f, 4.80216e+30f
path.quadTo(SkBits2Float(0x72725572), SkBits2Float(0x00007272), SkBits2Float(0x525adada), SkBits2Float(0x52525252));  // 4.79991e+30f, 4.10552e-41f, 2.34994e+11f, 2.25831e+11f
path.lineTo(SkBits2Float(0x5a61dada), SkBits2Float(0x52525252));  // 1.58931e+16f, 2.25831e+11f
path.close();
path.moveTo(SkBits2Float(0x5a61dada), SkBits2Float(0x52525252));  // 1.58931e+16f, 2.25831e+11f
path.quadTo(SkBits2Float(0x72727272), SkBits2Float(0x52525272), SkBits2Float(0x72525248), SkBits2Float(0x72727272));  // 4.80216e+30f, 2.25832e+11f, 4.16584e+30f, 4.80216e+30f
path.quadTo(SkBits2Float(0x72727255), SkBits2Float(0xda007b72), SkBits2Float(0x52525ada), SkBits2Float(0x72525252));  // 4.80215e+30f, -9.04113e+15f, 2.25867e+11f, 4.16585e+30f
path.quadTo(SkBits2Float(0x72727272), SkBits2Float(0x72727252), SkBits2Float(0xda007b72), SkBits2Float(0x52525ada));  // 4.80216e+30f, 4.80215e+30f, -9.04113e+15f, 2.25867e+11f
path.lineTo(SkBits2Float(0x5a61dada), SkBits2Float(0x52525252));  // 1.58931e+16f, 2.25831e+11f
path.close();
path.moveTo(SkBits2Float(0x5a61dada), SkBits2Float(0x52525252));  // 1.58931e+16f, 2.25831e+11f
path.quadTo(SkBits2Float(0x86727272), SkBits2Float(0x5252528d), SkBits2Float(0x72525252), SkBits2Float(0x72727227));  // -4.55992e-35f, 2.25832e+11f, 4.16585e+30f, 4.80214e+30f
path.quadTo(SkBits2Float(0x72727272), SkBits2Float(0x29217272), SkBits2Float(0xc003211c), SkBits2Float(0x556a4b7b));  // 4.80216e+30f, 3.58484e-14f, -2.0489f, 1.61006e+13f
path.moveTo(SkBits2Float(0x72557272), SkBits2Float(0x00727272));  // 4.22775e+30f, 1.05103e-38f
path.moveTo(SkBits2Float(0x525adada), SkBits2Float(0x52525252));  // 2.34994e+11f, 2.25831e+11f
path.close();
path.moveTo(SkBits2Float(0xa5252600), SkBits2Float(0x52b4adad));  // -1.43243e-16f, 3.88004e+11f
path.close();
path.moveTo(SkBits2Float(0xa5252600), SkBits2Float(0x52b4adad));  // -1.43243e-16f, 3.88004e+11f
path.quadTo(SkBits2Float(0x72727270), SkBits2Float(0x52525272), SkBits2Float(0x72525252), SkBits2Float(0x72727272));  // 4.80216e+30f, 2.25832e+11f, 4.16585e+30f, 4.80216e+30f
path.quadTo(SkBits2Float(0x72727255), SkBits2Float(0xda007b72), SkBits2Float(0x52525ada), SkBits2Float(0x52525252));  // 4.80215e+30f, -9.04113e+15f, 2.25867e+11f, 2.25831e+11f
path.quadTo(SkBits2Float(0x52525272), SkBits2Float(0x3b3b0052), SkBits2Float(0x5b2d553a), SkBits2Float(0x68556829));  // 2.25832e+11f, 0.00285341f, 4.87889e+16f, 4.03114e+24f

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x52528c55), SkBits2Float(0x29215252));  // 2.26074e+11f, 3.58206e-14f

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 0, filename);
}

static void fuzz763_41(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.quadTo(SkBits2Float(0x7a057c72), SkBits2Float(0x72727272), SkBits2Float(0x725b5e72), SkBits2Float(0x055f0089));  // 1.73275e+35f, 4.80216e+30f, 4.34505e+30f, 1.04855e-35f
path.quadTo(SkBits2Float(0x00057272), SkBits2Float(0x72ff0000), SkBits2Float(0xba405e72), SkBits2Float(0x031b0074));  // 5.00233e-40f, 1.01016e+31f, -0.000733829f, 4.55509e-37f
path.lineTo(SkBits2Float(0x664af700), SkBits2Float(0x56397d39));  // 2.39619e+23f, 5.09869e+13f
path.quadTo(SkBits2Float(0x7a057273), SkBits2Float(0x057300e4), SkBits2Float(0x257c0c9f), SkBits2Float(0x72400006));  // 1.73224e+35f, 1.1426e-35f, 2.18618e-16f, 3.80295e+30f
path.quadTo(SkBits2Float(0xba5b5e72), SkBits2Float(0x030000ff), SkBits2Float(0x74ba00e8), SkBits2Float(0xe8ec4000));  // -0.000836826f, 3.7617e-37f, 1.17894e+32f, -8.92527e+24f
path.moveTo(SkBits2Float(0x39724aff), SkBits2Float(0x7200397d));  // 0.000231069f, 2.53975e+30f
path.quadTo(SkBits2Float(0x827a0572), SkBits2Float(0x08727272), SkBits2Float(0x08080808), SkBits2Float(0x08080808));  // -1.83687e-37f, 7.29588e-34f, 4.09355e-34f, 4.09355e-34f
path.lineTo(SkBits2Float(0x08080808), SkBits2Float(0x08080808));  // 4.09355e-34f, 4.09355e-34f
path.lineTo(SkBits2Float(0x08080808), SkBits2Float(0x08080808));  // 4.09355e-34f, 4.09355e-34f
path.conicTo(SkBits2Float(0x72728c08), SkBits2Float(0x5b5e7272), SkBits2Float(0x000074ba), SkBits2Float(0x03f8e300), SkBits2Float(0x5aff00e8));  // 4.80414e+30f, 6.26133e+16f, 4.18736e-41f, 1.46282e-36f, 3.58886e+16f
path.quadTo(SkBits2Float(0x00800039), SkBits2Float(0x72100039), SkBits2Float(0x727a0572), SkBits2Float(0x7a727272));  // 1.1755e-38f, 2.85223e+30f, 4.95218e+30f, 3.14714e+35f
path.lineTo(SkBits2Float(0x7272727a), SkBits2Float(0xdb5e6472));  // 4.80216e+30f, -6.25979e+16f
path.moveTo(SkBits2Float(0x440039fc), SkBits2Float(0x0000f647));  // 512.906f, 8.83477e-41f
path.lineTo(SkBits2Float(0x666d0100), SkBits2Float(0x726efe62));  // 2.79805e+23f, 4.73376e+30f
path.lineTo(SkBits2Float(0x440039fc), SkBits2Float(0x0000f647));  // 512.906f, 8.83477e-41f
path.close();
path.moveTo(SkBits2Float(0x440039fc), SkBits2Float(0x0000f647));  // 512.906f, 8.83477e-41f
path.conicTo(SkBits2Float(0x72727272), SkBits2Float(0xf3db5e64), SkBits2Float(0x475afc16), SkBits2Float(0x170100ad), SkBits2Float(0x01008000));  // 4.80216e+30f, -3.47604e+31f, 56060.1f, 4.1683e-25f, 2.36017e-38f
path.quadTo(SkBits2Float(0x72057272), SkBits2Float(0x8c7a3472), SkBits2Float(0x72727272), SkBits2Float(0x00f6475e));  // 2.64319e+30f, -1.92751e-31f, 4.80216e+30f, 2.26171e-38f
path.moveTo(SkBits2Float(0x6d106d43), SkBits2Float(0x6efe6266));  // 2.79362e+27f, 3.93641e+28f
path.quadTo(SkBits2Float(0x72727a05), SkBits2Float(0xba5b7272), SkBits2Float(0x03000074), SkBits2Float(0x5aff00e8));  // 4.80274e+30f, -0.000837124f, 3.76163e-37f, 3.58886e+16f
path.quadTo(SkBits2Float(0x00da0039), SkBits2Float(0x72100039), SkBits2Float(0x727a0572), SkBits2Float(0x7a727272));  // 2.00202e-38f, 2.85223e+30f, 4.95218e+30f, 3.14714e+35f
path.lineTo(SkBits2Float(0x7272727a), SkBits2Float(0xdb5e6472));  // 4.80216e+30f, -6.25979e+16f
path.lineTo(SkBits2Float(0xfc5b97fc), SkBits2Float(0x47440039));  // -4.56078e+36f, 50176.2f
path.lineTo(SkBits2Float(0x00710000), SkBits2Float(0x62766d01));  // 1.03774e-38f, 1.13644e+21f
path.quadTo(SkBits2Float(0x7a05726e), SkBits2Float(0x72727272), SkBits2Float(0xf3db5e64), SkBits2Float(0x4a5afc16));  // 1.73224e+35f, 4.80216e+30f, -3.47604e+31f, 3.58785e+06f

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 4, filename);
}

static void fuzz763_40(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x10190004), SkBits2Float(0x7272727a));  // 3.01739e-29f, 4.80216e+30f
path.quadTo(SkBits2Float(0xf3db5e64), SkBits2Float(0x5b97fc16), SkBits2Float(0x000039fc), SkBits2Float(0x01008000));  // -3.47604e+31f, 8.55598e+16f, 2.08009e-41f, 2.36017e-38f
path.quadTo(SkBits2Float(0x7a057272), SkBits2Float(0x72727272), SkBits2Float(0x725b5e72), SkBits2Float(0x41720089));  // 1.73224e+35f, 4.80216e+30f, 4.34505e+30f, 15.1251f
path.lineTo(SkBits2Float(0x63636363), SkBits2Float(0x63606363));  // 4.19457e+21f, 4.13923e+21f
path.lineTo(SkBits2Float(0x01000000), SkBits2Float(0x10010004));  // 2.35099e-38f, 2.54408e-29f
path.conicTo(SkBits2Float(0x72727272), SkBits2Float(0xf3db5e64), SkBits2Float(0x4a5afc16), SkBits2Float(0x0000d07d), SkBits2Float(0x01008000));  // 4.80216e+30f, -3.47604e+31f, 3.58785e+06f, 7.47915e-41f, 2.36017e-38f
path.quadTo(SkBits2Float(0x7a057272), SkBits2Float(0x72727272), SkBits2Float(0x725b5e72), SkBits2Float(0x63720089));  // 1.73224e+35f, 4.80216e+30f, 4.34505e+30f, 4.46415e+21f
path.lineTo(SkBits2Float(0x63636363), SkBits2Float(0x63606363));  // 4.19457e+21f, 4.13923e+21f
path.lineTo(SkBits2Float(0x72000000), SkBits2Float(0x5b5e72b4));  // 2.5353e+30f, 6.26136e+16f
path.quadTo(SkBits2Float(0x05720089), SkBits2Float(0x05727272), SkBits2Float(0x7272727a), SkBits2Float(0x5b5e7272));  // 1.13789e-35f, 1.13998e-35f, 4.80216e+30f, 6.26133e+16f
path.cubicTo(SkBits2Float(0x03000074), SkBits2Float(0x4aff00e8), SkBits2Float(0x397d3972), SkBits2Float(0x01727200), SkBits2Float(0x72727a00), SkBits2Float(0x5e8d7272));  // 3.76163e-37f, 8.35596e+06f, 0.000241494f, 4.45302e-38f, 4.80274e+30f, 5.09617e+18f
path.moveTo(SkBits2Float(0x72008972), SkBits2Float(0x458fe705));  // 2.54594e+30f, 4604.88f
path.quadTo(SkBits2Float(0x7a057272), SkBits2Float(0xe8727272), SkBits2Float(0xba5b5e03), SkBits2Float(0x03000074));  // 1.73224e+35f, -4.5797e+24f, -0.00083682f, 3.76163e-37f
path.lineTo(SkBits2Float(0xf3dbff00), SkBits2Float(0x00397d16));  // -3.48598e+31f, 5.2795e-39f
path.cubicTo(SkBits2Float(0x7a101900), SkBits2Float(0x72727272), SkBits2Float(0xf3db5e64), SkBits2Float(0x0197fc16), SkBits2Float(0x200c2010), SkBits2Float(0x20203620));  // 1.87049e+35f, 4.80216e+30f, -3.47604e+31f, 5.58304e-38f, 1.18691e-19f, 1.35704e-19f

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 2, filename);
}

static void fuzz763_39(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.quadTo(SkBits2Float(0x7a057c72), SkBits2Float(0x72727272), SkBits2Float(0x725b5e72), SkBits2Float(0x055f0089));  // 1.73275e+35f, 4.80216e+30f, 4.34505e+30f, 1.04855e-35f
path.quadTo(SkBits2Float(0x7a057272), SkBits2Float(0x72727272), SkBits2Float(0xba405e72), SkBits2Float(0x03000074));  // 1.73224e+35f, 4.80216e+30f, -0.000733829f, 3.76163e-37f
path.lineTo(SkBits2Float(0x664aff00), SkBits2Float(0x56397d39));  // 2.39655e+23f, 5.09869e+13f
path.quadTo(SkBits2Float(0x7a057273), SkBits2Float(0x057300ff), SkBits2Float(0x257c0c9f), SkBits2Float(0x72787257));  // 1.73224e+35f, 1.1426e-35f, 2.18618e-16f, 4.92099e+30f
path.quadTo(SkBits2Float(0xba5b5e72), SkBits2Float(0x03000093), SkBits2Float(0x74ba00e8), SkBits2Float(0xe8ecff00));  // -0.000836826f, 3.76165e-37f, 1.17894e+32f, -8.95346e+24f
path.moveTo(SkBits2Float(0x39724aff), SkBits2Float(0x7200397d));  // 0.000231069f, 2.53975e+30f
path.quadTo(SkBits2Float(0x827a0572), SkBits2Float(0x72727272), SkBits2Float(0x724adf00), SkBits2Float(0x00397d39));  // -1.83687e-37f, 4.80216e+30f, 4.01828e+30f, 5.27954e-39f
path.quadTo(SkBits2Float(0x7a057272), SkBits2Float(0x16f3abab), SkBits2Float(0xfc5b97fc), SkBits2Float(0x47440039));  // 1.73224e+35f, 3.93671e-25f, -4.56078e+36f, 50176.2f
path.lineTo(SkBits2Float(0x00710000), SkBits2Float(0x62767201));  // 1.03774e-38f, 1.13653e+21f
path.quadTo(SkBits2Float(0x7a05726e), SkBits2Float(0x72727272), SkBits2Float(0xf3db5e64), SkBits2Float(0x4a5afc16));  // 1.73224e+35f, 4.80216e+30f, -3.47604e+31f, 3.58785e+06f

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 4, filename);
}


static void fuzz763_42(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.quadTo(SkBits2Float(0x7a057272), SkBits2Float(0x72727272), SkBits2Float(0x725b5e72), SkBits2Float(0x05720089));  // 1.73224e+35f, 4.80216e+30f, 4.34505e+30f, 1.13789e-35f
path.quadTo(SkBits2Float(0x7a057272), SkBits2Float(0x72727272), SkBits2Float(0xba405e72), SkBits2Float(0x03000074));  // 1.73224e+35f, 4.80216e+30f, -0.000733829f, 3.76163e-37f
path.lineTo(SkBits2Float(0x724aff00), SkBits2Float(0x56397d39));  // 4.02075e+30f, 5.09869e+13f
path.quadTo(SkBits2Float(0x7a057272), SkBits2Float(0xfa8d00ff), SkBits2Float(0x25727272), SkBits2Float(0x7272727a));  // 1.73224e+35f, -3.66067e+35f, 2.10289e-16f, 4.80216e+30f
path.quadTo(SkBits2Float(0xba5b5e72), SkBits2Float(0x03000093), SkBits2Float(0x74ba00e8), SkBits2Float(0xe8e0ff00));  // -0.000836826f, 3.76165e-37f, 1.17894e+32f, -8.50011e+24f
path.conicTo(SkBits2Float(0x39724aff), SkBits2Float(0x7200397d), SkBits2Float(0x7a057272), SkBits2Float(0x72727272), SkBits2Float(0x4aff0072));  // 0.000231069f, 2.53975e+30f, 1.73224e+35f, 4.80216e+30f, 8.3559e+06f
path.quadTo(SkBits2Float(0x00397d39), SkBits2Float(0x05727272), SkBits2Float(0x7272727a), SkBits2Float(0x385e7272));  // 5.27954e-39f, 1.13998e-35f, 4.80216e+30f, 5.30355e-05f
path.quadTo(SkBits2Float(0x057200ff), SkBits2Float(0x25727272), SkBits2Float(0x7272727a), SkBits2Float(0x5b5e7272));  // 1.1379e-35f, 2.10289e-16f, 4.80216e+30f, 6.26133e+16f
path.cubicTo(SkBits2Float(0x03000074), SkBits2Float(0x4aff00e8), SkBits2Float(0x397d3972), SkBits2Float(0x01000400), SkBits2Float(0x72727a10), SkBits2Float(0x5e647272));  // 3.76163e-37f, 8.35596e+06f, 0.000241494f, 2.35128e-38f, 4.80275e+30f, 4.11534e+18f
path.quadTo(SkBits2Float(0x2b2d16f3), SkBits2Float(0x0039fc4d), SkBits2Float(0x68800000), SkBits2Float(0x0100fafa));  // 6.14938e-13f, 5.32513e-39f, 4.8357e+24f, 2.369e-38f
path.quadTo(SkBits2Float(0x7a057272), SkBits2Float(0x72727272), SkBits2Float(0x725b5e72), SkBits2Float(0x63720089));  // 1.73224e+35f, 4.80216e+30f, 4.34505e+30f, 4.46415e+21f
path.lineTo(SkBits2Float(0x63636363), SkBits2Float(0x63606363));  // 4.19457e+21f, 4.13923e+21f
path.lineTo(SkBits2Float(0x72720000), SkBits2Float(0xff725b5e));  // 4.7933e+30f, -3.22148e+38f
path.moveTo(SkBits2Float(0x72720572), SkBits2Float(0x5b5e2572));  // 4.79373e+30f, 6.25286e+16f
path.quadTo(SkBits2Float(0x05720089), SkBits2Float(0x25727272), SkBits2Float(0x72728c7a), SkBits2Float(0x5b5e7272));  // 1.13789e-35f, 2.10289e-16f, 4.80417e+30f, 6.26133e+16f
path.cubicTo(SkBits2Float(0x03000074), SkBits2Float(0x4aff00e8), SkBits2Float(0x397d3972), SkBits2Float(0x01000400), SkBits2Float(0x72727a10), SkBits2Float(0x5e827272));  // 3.76163e-37f, 8.35596e+06f, 0.000241494f, 2.35128e-38f, 4.80275e+30f, 4.69985e+18f
path.quadTo(SkBits2Float(0x97fc16f3), SkBits2Float(0x0039fc5b), SkBits2Float(0x00f6472e), SkBits2Float(0x01008000));  // -1.62909e-24f, 5.32515e-39f, 2.26171e-38f, 2.36017e-38f
path.quadTo(SkBits2Float(0x7a057272), SkBits2Float(0x72727272), SkBits2Float(0xf3db5e64), SkBits2Float(0x4a5afc16));  // 1.73224e+35f, 4.80216e+30f, -3.47604e+31f, 3.58785e+06f

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 4, filename);
}

static void fuzz763_43(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x5c386c3a), SkBits2Float(0x4e691a3e));  // 2.07642e+17f, 9.77703e+08f
path.cubicTo(SkBits2Float(0x6f69f9f5), SkBits2Float(0x18ff8791), SkBits2Float(0x2492263c), SkBits2Float(0xbc6fdb48), SkBits2Float(0xc2f82107), SkBits2Float(0x729a18e1));  // 7.24122e+28f, 6.60528e-24f, 6.33822e-17f, -0.0146397f, -124.065f, 6.10442e+30f
path.cubicTo(SkBits2Float(0x07d729d1), SkBits2Float(0xdea6db48), SkBits2Float(0xcd1dfb88), SkBits2Float(0x90826769), SkBits2Float(0x1c20e5a4), SkBits2Float(0xa4c3ba9b));  // 3.23742e-34f, -6.01164e+18f, -1.65657e+08f, -5.14353e-29f, 5.32364e-22f, -8.48839e-17f
path.moveTo(SkBits2Float(0xcc2084b7), SkBits2Float(0x19f68bdb));  // -4.20789e+07f, 2.54923e-23f
path.close();
path.moveTo(SkBits2Float(0xcc2084b7), SkBits2Float(0x19f68bdb));  // -4.20789e+07f, 2.54923e-23f
path.cubicTo(SkBits2Float(0xdeea1d6e), SkBits2Float(0xc7774804), SkBits2Float(0x27cf0dcf), SkBits2Float(0x6ae8b99f), SkBits2Float(0x24ac3260), SkBits2Float(0x062fa93c));  // -8.43488e+18f, -63304, 5.7469e-15f, 1.40674e+26f, 7.46784e-17f, 3.30382e-35f
path.lineTo(SkBits2Float(0x438a0b9c), SkBits2Float(0x60a1d2c8));  // 276.091f, 9.32848e+19f
path.quadTo(SkBits2Float(0xe13fb902), SkBits2Float(0x07ee536f), SkBits2Float(0x971d8ac1), SkBits2Float(0x2f9f174b));  // -2.21041e+20f, 3.58593e-34f, -5.09046e-25f, 2.89385e-10f
path.lineTo(SkBits2Float(0x0f2cf5d8), SkBits2Float(0xe271654c));  // 8.5276e-30f, -1.11324e+21f
path.lineTo(SkBits2Float(0xe6cf24d2), SkBits2Float(0xd9537742));  // -4.89105e+23f, -3.72015e+15f
path.cubicTo(SkBits2Float(0x1aaaee04), SkBits2Float(0x9e3b804c), SkBits2Float(0x84cba87d), SkBits2Float(0x4e0e8ccc), SkBits2Float(0x2aec611a), SkBits2Float(0x7ae4b639));  // 7.06949e-23f, -9.92623e-21f, -4.78798e-36f, 5.97898e+08f, 4.19894e-13f, 5.9377e+35f
path.conicTo(SkBits2Float(0x73357921), SkBits2Float(0x6f163021), SkBits2Float(0x70ea542c), SkBits2Float(0xe008f404), SkBits2Float(0x1f6c5e52));  // 1.43778e+31f, 4.64809e+28f, 5.8017e+29f, -3.94741e+19f, 5.0053e-20f
path.lineTo(SkBits2Float(0xda45ad4e), SkBits2Float(0xedce4a04));  // -1.39103e+16f, -7.98042e+27f
path.lineTo(SkBits2Float(0xac0e45da), SkBits2Float(0x8f632841));  // -2.02182e-12f, -1.11997e-29f
path.lineTo(SkBits2Float(0xcc2084b7), SkBits2Float(0x19f68bdb));  // -4.20789e+07f, 2.54923e-23f
path.close();
path.moveTo(SkBits2Float(0xcc2084b7), SkBits2Float(0x19f68bdb));  // -4.20789e+07f, 2.54923e-23f
path.quadTo(SkBits2Float(0xf35c4ad5), SkBits2Float(0x0692f251), SkBits2Float(0x69632126), SkBits2Float(0xb927af67));  // -1.74534e+31f, 5.52751e-35f, 1.71614e+25f, -0.000159917f
path.moveTo(SkBits2Float(0x6534bff9), SkBits2Float(0x434a9986));  // 5.3348e+22f, 202.6f
path.quadTo(SkBits2Float(0x37c603e5), SkBits2Float(0xa0683953), SkBits2Float(0x751915e4), SkBits2Float(0x831c911a));  // 2.36053e-05f, -1.96701e-19f, 1.94059e+32f, -4.60108e-37f
path.cubicTo(SkBits2Float(0xba4f10f1), SkBits2Float(0x5a7571df), SkBits2Float(0x4ec67459), SkBits2Float(0x33c58827), SkBits2Float(0x10b78ccb), SkBits2Float(0xedbd2748));  // -0.000789895f, 1.72716e+16f, 1.66476e+09f, 9.19829e-08f, 7.23977e-29f, -7.31752e+27f
path.cubicTo(SkBits2Float(0x6d06f06a), SkBits2Float(0xe30465cf), SkBits2Float(0xc5458fe7), SkBits2Float(0xca488dc4), SkBits2Float(0x38f9021c), SkBits2Float(0x3e8d58db));  // 2.6101e+27f, -2.44231e+21f, -3160.99f, -3.28587e+06f, 0.000118736f, 0.276069f

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 4, filename);
}

static void fuzz763_44(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x7c223bab), SkBits2Float(0x7cf35966));  // 3.36945e+36f, 1.01083e+37f
path.quadTo(SkBits2Float(0x00000000), SkBits2Float(0x7ccaca6d), SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 8.4236e+36f, 0, 0
path.lineTo(SkBits2Float(0x7d7d7d7d), SkBits2Float(0x00000000));  // 2.10591e+37f, 0
path.quadTo(SkBits2Float(0x7ccacab0), SkBits2Float(0x7d1817f4), SkBits2Float(0x7c223bab), SkBits2Float(0x7cf35966));  // 8.42364e+36f, 1.26354e+37f, 3.36945e+36f, 1.01083e+37f
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x109d0000), SkBits2Float(0xff7bc000));  // 6.19256e-29f, -3.34633e+38f
path.conicTo(SkBits2Float(0x979797ed), SkBits2Float(0x3a214797), SkBits2Float(0x28aa217a), SkBits2Float(0x01007272), SkBits2Float(0x00000072));  // -9.7965e-25f, 0.000615233f, 1.88883e-14f, 2.3592e-38f, 1.59748e-43f
path.quadTo(SkBits2Float(0x72728302), SkBits2Float(0x8b727272), SkBits2Float(0x72727272), SkBits2Float(0xc00308f6));  // 4.80344e+30f, -4.66936e-32f, 4.80216e+30f, -2.04742f
path.conicTo(SkBits2Float(0x7f52753a), SkBits2Float(0x8072ffff), SkBits2Float(0x67af2103), SkBits2Float(0x7d2a6847), SkBits2Float(0x7d7d7d7d));  // 2.79747e+38f, -1.05611e-38f, 1.65405e+24f, 1.41569e+37f, 2.10591e+37f

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 3, filename);
}

static void fuzz763_45(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.cubicTo(SkBits2Float(0x30303030), SkBits2Float(0x30303030), SkBits2Float(0x30303030), SkBits2Float(0x7a303030), SkBits2Float(0x7a303030), SkBits2Float(0x30303030));  // 6.40969e-10f, 6.40969e-10f, 6.40969e-10f, 2.28705e+35f, 2.28705e+35f, 6.40969e-10f
path.conicTo(SkBits2Float(0x30303030), SkBits2Float(0x74303030), SkBits2Float(0x74303030), SkBits2Float(0x30303030), SkBits2Float(0x74303030));  // 6.40969e-10f, 5.58363e+31f, 5.58363e+31f, 6.40969e-10f, 5.58363e+31f
path.conicTo(SkBits2Float(0x30303030), SkBits2Float(0x30303030), SkBits2Float(0x30303030), SkBits2Float(0x30303030), SkBits2Float(0x30303030));  // 6.40969e-10f, 6.40969e-10f, 6.40969e-10f, 6.40969e-10f, 6.40969e-10f
path.moveTo(SkBits2Float(0x30303030), SkBits2Float(0x30303030));  // 6.40969e-10f, 6.40969e-10f
path.moveTo(SkBits2Float(0x77773030), SkBits2Float(0x30303030));  // 5.01357e+33f, 6.40969e-10f
path.conicTo(SkBits2Float(0x30303030), SkBits2Float(0x30303030), SkBits2Float(0x7a743030), SkBits2Float(0x74303030), SkBits2Float(0x30303030));  // 6.40969e-10f, 6.40969e-10f, 3.16974e+35f, 5.58363e+31f, 6.40969e-10f
path.lineTo(SkBits2Float(0x77773030), SkBits2Float(0x30303030));  // 5.01357e+33f, 6.40969e-10f
path.close();
path.moveTo(SkBits2Float(0x77773030), SkBits2Float(0x30303030));  // 5.01357e+33f, 6.40969e-10f
path.lineTo(SkBits2Float(0x7f303030), SkBits2Float(0x7a303030));  // 2.34194e+38f, 2.28705e+35f
path.conicTo(SkBits2Float(0x77303030), SkBits2Float(0x30303030), SkBits2Float(0x30303030), SkBits2Float(0xf9303030), SkBits2Float(0x7a303030));  // 3.57352e+33f, 6.40969e-10f, 6.40969e-10f, -5.71764e+34f, 2.28705e+35f
path.conicTo(SkBits2Float(0x30303030), SkBits2Float(0x30303030), SkBits2Float(0x30303030), SkBits2Float(0x30303030), SkBits2Float(0x30303030));  // 6.40969e-10f, 6.40969e-10f, 6.40969e-10f, 6.40969e-10f, 6.40969e-10f
path.quadTo(SkBits2Float(0x30303030), SkBits2Float(0x30303030), SkBits2Float(0x30303030), SkBits2Float(0x30303030));  // 6.40969e-10f, 6.40969e-10f, 6.40969e-10f, 6.40969e-10f
path.quadTo(SkBits2Float(0x30303030), SkBits2Float(0x30303030), SkBits2Float(0x30303030), SkBits2Float(0x30303030));  // 6.40969e-10f, 6.40969e-10f, 6.40969e-10f, 6.40969e-10f
path.conicTo(SkBits2Float(0x30303030), SkBits2Float(0x30303030), SkBits2Float(0x30303030), SkBits2Float(0x30303030), SkBits2Float(0x30303030));  // 6.40969e-10f, 6.40969e-10f, 6.40969e-10f, 6.40969e-10f, 6.40969e-10f
path.conicTo(SkBits2Float(0x30303030), SkBits2Float(0x30303030), SkBits2Float(0x30303030), SkBits2Float(0x7a303030), SkBits2Float(0x30303030));  // 6.40969e-10f, 6.40969e-10f, 6.40969e-10f, 2.28705e+35f, 6.40969e-10f
path.cubicTo(SkBits2Float(0x30303030), SkBits2Float(0x30303030), SkBits2Float(0x30303030), SkBits2Float(0x30303030), SkBits2Float(0x7a303030), SkBits2Float(0x30303030));  // 6.40969e-10f, 6.40969e-10f, 6.40969e-10f, 6.40969e-10f, 2.28705e+35f, 6.40969e-10f
path.conicTo(SkBits2Float(0x30303030), SkBits2Float(0x30303030), SkBits2Float(0x30303030), SkBits2Float(0x30303030), SkBits2Float(0x30303030));  // 6.40969e-10f, 6.40969e-10f, 6.40969e-10f, 6.40969e-10f, 6.40969e-10f
path.moveTo(SkBits2Float(0x77303030), SkBits2Float(0xff303030));  // 3.57352e+33f, -2.34194e+38f
path.conicTo(SkBits2Float(0x30303030), SkBits2Float(0x30303030), SkBits2Float(0x7f773030), SkBits2Float(0x7a7a3030), SkBits2Float(0x7a303030));  // 6.40969e-10f, 6.40969e-10f, 3.2857e+38f, 3.24763e+35f, 2.28705e+35f
path.quadTo(SkBits2Float(0x30303030), SkBits2Float(0x30303030), SkBits2Float(0x77303030), SkBits2Float(0x30303030));  // 6.40969e-10f, 6.40969e-10f, 3.57352e+33f, 6.40969e-10f
path.conicTo(SkBits2Float(0x30303030), SkBits2Float(0x30303030), SkBits2Float(0x7b303030), SkBits2Float(0x73303030), SkBits2Float(0x30303030));  // 6.40969e-10f, 6.40969e-10f, 9.14822e+35f, 1.39591e+31f, 6.40969e-10f
path.quadTo(SkBits2Float(0x30303030), SkBits2Float(0x30303030), SkBits2Float(0x30303030), SkBits2Float(0x7a7a3030));  // 6.40969e-10f, 6.40969e-10f, 6.40969e-10f, 3.24763e+35f
    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 3, filename);
}

static void fuzz763_46(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
    path.conicTo(SkBits2Float(0x44444444), SkBits2Float(0x44444444), SkBits2Float(0x44263030), SkBits2Float(0x44304430), SkBits2Float(0x4c444430));  // 785.067f, 785.067f, 664.753f, 705.065f, 5.145e+07f
path.moveTo(SkBits2Float(0x44444444), SkBits2Float(0x44444444));  // 785.067f, 785.067f
path.cubicTo(SkBits2Float(0x30303030), SkBits2Float(0x44444444), SkBits2Float(0x30303030), SkBits2Float(0x44444444), SkBits2Float(0x44444444), SkBits2Float(0x4444444c));  // 6.40969e-10f, 785.067f, 6.40969e-10f, 785.067f, 785.067f, 785.067f
    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 3, filename);
}

static void fuzz763_47(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.cubicTo(SkBits2Float(0x7272728e), SkBits2Float(0x52527272), SkBits2Float(0x2d555252), SkBits2Float(0x68556829), SkBits2Float(0x555b2d29), SkBits2Float(0x2a212a8c));  // 4.80217e+30f, 2.25966e+11f, 1.21259e-11f, 4.03114e+24f, 1.50617e+13f, 1.43144e-13f
path.conicTo(SkBits2Float(0x00296808), SkBits2Float(0x00000002), SkBits2Float(0x52525252), SkBits2Float(0x72007272), SkBits2Float(0x52527272));  // 3.80257e-39f, 2.8026e-45f, 2.25831e+11f, 2.54416e+30f, 2.25966e+11f
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.close();
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.lineTo(SkBits2Float(0x2a212a8c), SkBits2Float(0x7272081f));  // 1.43144e-13f, 4.79393e+30f
path.quadTo(SkBits2Float(0x72727272), SkBits2Float(0x5974fa80), SkBits2Float(0x00747474), SkBits2Float(0x59585264));  // 4.80216e+30f, 4.30971e+15f, 1.06947e-38f, 3.80557e+15f
path.cubicTo(SkBits2Float(0x64007474), SkBits2Float(0x088c5852), SkBits2Float(0x80808021), SkBits2Float(0x8c808080), SkBits2Float(0x80802108), SkBits2Float(0x80808080));  // 9.4783e+21f, 8.44671e-34f, -1.18009e-38f, -1.97989e-31f, -1.17668e-38f, -1.1801e-38f
path.quadTo(SkBits2Float(0x80807d80), SkBits2Float(0x80808080), SkBits2Float(0xff7f0000), SkBits2Float(0x80808080));  // -1.18e-38f, -1.1801e-38f, -3.38953e+38f, -1.1801e-38f
path.quadTo(SkBits2Float(0x80808080), SkBits2Float(0x80808080), SkBits2Float(0xed842b00), SkBits2Float(0x7252ff6d));  // -1.1801e-38f, -1.1801e-38f, -5.113e+27f, 4.17924e+30f
path.quadTo(SkBits2Float(0x72577200), SkBits2Float(0x55525352), SkBits2Float(0x2a212a8c), SkBits2Float(0x7272081f));  // 4.26733e+30f, 1.44535e+13f, 1.43144e-13f, 4.79393e+30f
path.quadTo(SkBits2Float(0x72727272), SkBits2Float(0x6f740080), SkBits2Float(0x8c556874), SkBits2Float(0x2982ffff));  // 4.80216e+30f, 7.55149e+28f, -1.64404e-31f, 5.81757e-14f

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 2, filename);
}

static void fuzz763_48(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.lineTo(SkBits2Float(0xed0081bc), SkBits2Float(0x1b2d8040));  // -2.48568e+27f, 1.43517e-22f
path.moveTo(SkBits2Float(0x74747403), SkBits2Float(0x29747474));  // 7.74703e+31f, 5.42799e-14f
path.close();
path.moveTo(SkBits2Float(0x74747403), SkBits2Float(0x29747474));  // 7.74703e+31f, 5.42799e-14f
path.conicTo(SkBits2Float(0x662d5576), SkBits2Float(0x2d804066), SkBits2Float(0x8068291b), SkBits2Float(0x740315ff), SkBits2Float(0x74747474));  // 2.04636e+23f, 1.45805e-11f, -9.56564e-39f, 4.15428e+31f, 7.74708e+31f
path.cubicTo(SkBits2Float(0x762d0529), SkBits2Float(0x72525252), SkBits2Float(0x007b7272), SkBits2Float(0x525adada), SkBits2Float(0x52525252), SkBits2Float(0x52727252));  // 8.77316e+32f, 4.16585e+30f, 1.13368e-38f, 2.34994e+11f, 2.25831e+11f, 2.60325e+11f
path.lineTo(SkBits2Float(0x74747403), SkBits2Float(0x29747474));  // 7.74703e+31f, 5.42799e-14f
path.close();
path.moveTo(SkBits2Float(0xa5252620), SkBits2Float(0x52b4adad));  // -1.43244e-16f, 3.88004e+11f
path.close();
path.moveTo(SkBits2Float(0xa5252620), SkBits2Float(0x52b4adad));  // -1.43244e-16f, 3.88004e+11f
path.quadTo(SkBits2Float(0x72727270), SkBits2Float(0x52524872), SkBits2Float(0x72525252), SkBits2Float(0x72727272));  // 4.80216e+30f, 2.2579e+11f, 4.16585e+30f, 4.80216e+30f
path.quadTo(SkBits2Float(0x72727255), SkBits2Float(0x80406666), SkBits2Float(0x68291b2d), SkBits2Float(0x0315ff80));  // 4.80215e+30f, -5.91421e-39f, 3.19432e+24f, 4.40805e-37f
path.cubicTo(SkBits2Float(0x74747474), SkBits2Float(0x7b722974), SkBits2Float(0x5adada00), SkBits2Float(0x52525252), SkBits2Float(0x72720052), SkBits2Float(0x72727272));  // 7.74708e+31f, 1.25738e+36f, 3.08006e+16f, 2.25831e+11f, 4.79333e+30f, 4.80216e+30f
path.lineTo(SkBits2Float(0xa5252620), SkBits2Float(0x52b4adad));  // -1.43244e-16f, 3.88004e+11f
path.close();
path.moveTo(SkBits2Float(0xa5252620), SkBits2Float(0x52b4adad));  // -1.43244e-16f, 3.88004e+11f
path.quadTo(SkBits2Float(0x72727227), SkBits2Float(0x72727272), SkBits2Float(0x74727272), SkBits2Float(0x55747421));  // 4.80214e+30f, 4.80216e+30f, 7.68345e+31f, 1.67987e+13f
path.lineTo(SkBits2Float(0xa5252620), SkBits2Float(0x52b4adad));  // -1.43244e-16f, 3.88004e+11f
path.close();
path.moveTo(SkBits2Float(0x724b0000), SkBits2Float(0x00725f72));  // 4.02083e+30f, 1.05035e-38f
path.lineTo(SkBits2Float(0x52525252), SkBits2Float(0x72725252));  // 2.25831e+11f, 4.79967e+30f
path.quadTo(SkBits2Float(0x26727272), SkBits2Float(0x0303a525), SkBits2Float(0x52005c03), SkBits2Float(0x72525252));  // 8.41157e-16f, 3.8687e-37f, 1.37825e+11f, 4.16585e+30f
path.quadTo(SkBits2Float(0x72727272), SkBits2Float(0x1ff07255), SkBits2Float(0x2a8c5572), SkBits2Float(0x21082a21));  // 4.80216e+30f, 1.01833e-19f, 2.49283e-13f, 4.61343e-19f
path.lineTo(SkBits2Float(0x2a2a3a21), SkBits2Float(0x29212828));  // 1.51192e-13f, 3.5784e-14f

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 4, filename);
}

static void fuzz763_49(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.conicTo(SkBits2Float(0x30303030), SkBits2Float(0x78303030), SkBits2Float(0x78787881), SkBits2Float(0x78787878), SkBits2Float(0x30303030));  // 6.40969e-10f, 1.42941e+34f, 2.01583e+34f, 2.01583e+34f, 6.40969e-10f
path.lineTo(SkBits2Float(0x78787878), SkBits2Float(0x78787878));  // 2.01583e+34f, 2.01583e+34f
path.lineTo(SkBits2Float(0x78787878), SkBits2Float(0x78787878));  // 2.01583e+34f, 2.01583e+34f
path.lineTo(SkBits2Float(0x78787878), SkBits2Float(0x78787878));  // 2.01583e+34f, 2.01583e+34f
path.quadTo(SkBits2Float(0x30303030), SkBits2Float(0x78787878), SkBits2Float(0x78787878), SkBits2Float(0x78787878));  // 6.40969e-10f, 2.01583e+34f, 2.01583e+34f, 2.01583e+34f
path.lineTo(SkBits2Float(0x30303030), SkBits2Float(0x30303030));  // 6.40969e-10f, 6.40969e-10f
path.lineTo(SkBits2Float(0x30303030), SkBits2Float(0x30303030));  // 6.40969e-10f, 6.40969e-10f
path.lineTo(SkBits2Float(0x30303030), SkBits2Float(0x30303030));  // 6.40969e-10f, 6.40969e-10f
path.lineTo(SkBits2Float(0x30303030), SkBits2Float(0x30303030));  // 6.40969e-10f, 6.40969e-10f
path.lineTo(SkBits2Float(0x30303030), SkBits2Float(0x30303030));  // 6.40969e-10f, 6.40969e-10f
path.lineTo(SkBits2Float(0x30303030), SkBits2Float(0x30303030));  // 6.40969e-10f, 6.40969e-10f
path.lineTo(SkBits2Float(0x30303030), SkBits2Float(0x30303030));  // 6.40969e-10f, 6.40969e-10f
path.lineTo(SkBits2Float(0x30303030), SkBits2Float(0x30303030));  // 6.40969e-10f, 6.40969e-10f
path.lineTo(SkBits2Float(0x30303030), SkBits2Float(0x30303030));  // 6.40969e-10f, 6.40969e-10f
path.lineTo(SkBits2Float(0x30303030), SkBits2Float(0x30303030));  // 6.40969e-10f, 6.40969e-10f
path.lineTo(SkBits2Float(0x30303030), SkBits2Float(0x30303030));  // 6.40969e-10f, 6.40969e-10f
path.lineTo(SkBits2Float(0x30303030), SkBits2Float(0x30303030));  // 6.40969e-10f, 6.40969e-10f
path.lineTo(SkBits2Float(0x30303030), SkBits2Float(0x30303030));  // 6.40969e-10f, 6.40969e-10f
path.lineTo(SkBits2Float(0x78787878), SkBits2Float(0x7878788d));  // 2.01583e+34f, 2.01584e+34f
path.lineTo(SkBits2Float(0x78787878), SkBits2Float(0x30303030));  // 2.01583e+34f, 6.40969e-10f

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 3, filename);
}

static void fuzz763_50(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x70621ede), SkBits2Float(0x00000000));  // 2.79924e+29f, 0
path.conicTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000), SkBits2Float(0x00000000), SkBits2Float(0x74fc5b97), SkBits2Float(0x7d458fe4));  // 0, 0, 0, 1.59951e+32f, 1.64128e+37f
path.lineTo(SkBits2Float(0xefea1ffe), SkBits2Float(0x00000000));  // -1.44916e+29f, 0
path.lineTo(SkBits2Float(0x70621ede), SkBits2Float(0x00000000));  // 2.79924e+29f, 0
path.close();
path.moveTo(SkBits2Float(0xefea1ffe), SkBits2Float(0x00000000));  // -1.44916e+29f, 0
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.lineTo(SkBits2Float(0xefea1ffe), SkBits2Float(0x00000000));  // -1.44916e+29f, 0
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 3, filename);
}

static void fuzz763_51(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.quadTo(SkBits2Float(0x868b5aae), SkBits2Float(0x626c45ab), SkBits2Float(0xefea1ffe), SkBits2Float(0x0029fc76));  // -5.24192e-35f, 1.08961e+21f, -1.44916e+29f, 3.85582e-39f
path.moveTo(SkBits2Float(0xfacbff01), SkBits2Float(0x56fc5b97));  // -5.29604e+35f, 1.38735e+14f
path.cubicTo(SkBits2Float(0x7d4559c9), SkBits2Float(0xad801c39), SkBits2Float(0xfbe2091a), SkBits2Float(0x7268e394), SkBits2Float(0x7c800079), SkBits2Float(0xa1d75590));  // 1.63953e+37f, -1.45644e-11f, -2.34729e+36f, 4.61284e+30f, 5.31699e+36f, -1.45916e-18f

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 2, filename);
}

static void fuzz763_52(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.quadTo(SkBits2Float(0x29ff4bae), SkBits2Float(0xa1d75590), SkBits2Float(0x9fd6f6c3), SkBits2Float(0x70621ede));  // 1.13374e-13f, -1.45916e-18f, -9.10408e-20f, 2.79924e+29f
path.quadTo(SkBits2Float(0x57a839d3), SkBits2Float(0x1a80d34b), SkBits2Float(0x0147a31b), SkBits2Float(0xff7fffff));  // 3.69933e+14f, 5.32809e-23f, 3.66675e-38f, -3.40282e+38f
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.close();
path.moveTo(SkBits2Float(0x1ab8e97c), SkBits2Float(0x94fbe3ef));  // 7.64778e-23f, -2.54344e-26f
path.conicTo(SkBits2Float(0x75757568), SkBits2Float(0x7575755e), SkBits2Float(0x75757575), SkBits2Float(0x75757575), SkBits2Float(0x75756575));  // 3.11156e+32f, 3.11156e+32f, 3.11156e+32f, 3.11156e+32f, 3.11077e+32f
path.lineTo(SkBits2Float(0x1ab8e97c), SkBits2Float(0x94fbe3ef));  // 7.64778e-23f, -2.54344e-26f
path.close();
path.moveTo(SkBits2Float(0x1ab8e97c), SkBits2Float(0x94fbe3ef));  // 7.64778e-23f, -2.54344e-26f
path.conicTo(SkBits2Float(0x75757575), SkBits2Float(0x75757575), SkBits2Float(0x75757575), SkBits2Float(0x75917575), SkBits2Float(0x75757575));  // 3.11156e+32f, 3.11156e+32f, 3.11156e+32f, 3.68782e+32f, 3.11156e+32f
path.lineTo(SkBits2Float(0x1ab8e97c), SkBits2Float(0x94fbe3ef));  // 7.64778e-23f, -2.54344e-26f
path.close();
path.moveTo(SkBits2Float(0x1ab8e97c), SkBits2Float(0x94fbe3ef));  // 7.64778e-23f, -2.54344e-26f
path.conicTo(SkBits2Float(0x75757575), SkBits2Float(0x7575758f), SkBits2Float(0x7f757575), SkBits2Float(0x75757575), SkBits2Float(0x75757575));  // 3.11156e+32f, 3.11157e+32f, 3.26271e+38f, 3.11156e+32f, 3.11156e+32f
path.lineTo(SkBits2Float(0x1ab8e97c), SkBits2Float(0x94fbe3ef));  // 7.64778e-23f, -2.54344e-26f
path.close();

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 2, filename);
}

static void fuzz763_53(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x7644b829), SkBits2Float(0x00000000));  // 9.97486e+32f, 0
path.lineTo(SkBits2Float(0x74fc5b97), SkBits2Float(0x77df944a));  // 1.59951e+32f, 9.06945e+33f
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xf8fbe3ff));  // 0, -4.08716e+34f
path.lineTo(SkBits2Float(0x7644b829), SkBits2Float(0x00000000));  // 9.97486e+32f, 0
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.quadTo(SkBits2Float(0x45ab86ae), SkBits2Float(0xd6d6626c), SkBits2Float(0xd6d6d6d6), SkBits2Float(0x7644d6d6));  // 5488.83f, -1.17859e+14f, -1.18109e+14f, 9.98093e+32f
path.moveTo(SkBits2Float(0xd6d6d6d6), SkBits2Float(0xd6d6d6d6));  // -1.18109e+14f, -1.18109e+14f
path.cubicTo(SkBits2Float(0xd6d6d6d6), SkBits2Float(0x64fed6d6), SkBits2Float(0x7644ef40), SkBits2Float(0x290877fc), SkBits2Float(0x447644b8), SkBits2Float(0x80fafc76));  // -1.18109e+14f, 3.76076e+22f, 9.98577e+32f, 3.03021e-14f, 985.074f, -2.30494e-38f
path.conicTo(SkBits2Float(0x87808080), SkBits2Float(0x764400ae), SkBits2Float(0x764400fc), SkBits2Float(0x450080fc), SkBits2Float(0x3636366c));  // -1.93348e-34f, 9.93852e+32f, 9.93858e+32f, 2056.06f, 2.71518e-06f
path.lineTo(SkBits2Float(0xd6d6d6d6), SkBits2Float(0xd6d6d6d6));  // -1.18109e+14f, -1.18109e+14f
path.close();
path.moveTo(SkBits2Float(0xef08a412), SkBits2Float(0x5aaeff7f));  // -4.22883e+28f, 2.46288e+16f
path.conicTo(SkBits2Float(0x7644626c), SkBits2Float(0x088912fc), SkBits2Float(0xae8744ef), SkBits2Float(0x76571f5a), SkBits2Float(0x45ab86fc));  // 9.95788e+32f, 8.24985e-34f, -6.15133e-11f, 1.0908e+33f, 5488.87f
path.conicTo(SkBits2Float(0x4064fe62), SkBits2Float(0x290877ef), SkBits2Float(0x780080b8), SkBits2Float(0x553c7644), SkBits2Float(0x644eae87));  // 3.57803f, 3.03021e-14f, 1.04254e+34f, 1.2951e+13f, 1.52504e+22f
path.lineTo(SkBits2Float(0xef08a412), SkBits2Float(0x5aaeff7f));  // -4.22883e+28f, 2.46288e+16f
path.close();

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 0, filename);
}

// hangs 654939
static void fuzz763_54(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.conicTo(SkBits2Float(0x5b682968), SkBits2Float(0xb3b32d11), SkBits2Float(0xb3b3b3b3), SkBits2Float(0x5b29b3b3), SkBits2Float(0x212a8c55));  // 6.53477e+16f, -8.34353e-08f, -8.36802e-08f, 4.77669e+16f, 5.7784e-19f
path.conicTo(SkBits2Float(0x68555b2d), SkBits2Float(0x28296869), SkBits2Float(0x5b252a08), SkBits2Float(0x5d68392a), SkBits2Float(0x29282780));  // 4.03018e+24f, 9.40402e-15f, 4.64896e+16f, 1.04584e+18f, 3.73378e-14f
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.close();
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.cubicTo(SkBits2Float(0x52727272), SkBits2Float(0x72727252), SkBits2Float(0x525252c7), SkBits2Float(0x72725252), SkBits2Float(0x72727272), SkBits2Float(0x72727255));  // 2.60326e+11f, 4.80215e+30f, 2.25833e+11f, 4.79967e+30f, 4.80216e+30f, 4.80215e+30f
path.quadTo(SkBits2Float(0xd7da0000), SkBits2Float(0x5252525a), SkBits2Float(0x72525252), SkBits2Float(0x72727272));  // -4.79387e+14f, 2.25831e+11f, 4.16585e+30f, 4.80216e+30f
path.quadTo(SkBits2Float(0x48525252), SkBits2Float(0x72725252), SkBits2Float(0x72727272), SkBits2Float(0x72727255));  // 215369, 4.79967e+30f, 4.80216e+30f, 4.80215e+30f
path.quadTo(SkBits2Float(0xdada007b), SkBits2Float(0x5252525a), SkBits2Float(0x72675252), SkBits2Float(0x72727272));  // -3.0681e+16f, 2.25831e+11f, 4.5818e+30f, 4.80216e+30f
path.quadTo(SkBits2Float(0x52525252), SkBits2Float(0x27725252), SkBits2Float(0x72727272), SkBits2Float(0x72727272));  // 2.25831e+11f, 3.36289e-15f, 4.80216e+30f, 4.80216e+30f
path.quadTo(SkBits2Float(0x1c292172), SkBits2Float(0x7bc00321), SkBits2Float(0x9aaaaaaa), SkBits2Float(0x8c556a4b));  // 5.59606e-22f, 1.99397e+36f, -7.05861e-23f, -1.64409e-31f
path.quadTo(SkBits2Float(0x72725572), SkBits2Float(0x00007272), SkBits2Float(0x525adada), SkBits2Float(0x52525252));  // 4.79991e+30f, 4.10552e-41f, 2.34994e+11f, 2.25831e+11f
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.close();
path.moveTo(SkBits2Float(0xa5252600), SkBits2Float(0x52b4adad));  // -1.43243e-16f, 3.88004e+11f
path.close();
path.moveTo(SkBits2Float(0xa5252600), SkBits2Float(0x52b4adad));  // -1.43243e-16f, 3.88004e+11f
path.quadTo(SkBits2Float(0x72725570), SkBits2Float(0x52525272), SkBits2Float(0x72525252), SkBits2Float(0x72727272));  // 4.79991e+30f, 2.25832e+11f, 4.16585e+30f, 4.80216e+30f
path.quadTo(SkBits2Float(0x72727255), SkBits2Float(0x555bb672), SkBits2Float(0x29686968), SkBits2Float(0x252a081f));  // 4.80215e+30f, 1.50985e+13f, 5.16058e-14f, 1.47479e-16f
path.moveTo(SkBits2Float(0x5d68392a), SkBits2Float(0x01002780));  // 1.04584e+18f, 2.35382e-38f
path.moveTo(SkBits2Float(0x72727200), SkBits2Float(0x72725252));  // 4.80212e+30f, 4.79967e+30f
path.quadTo(SkBits2Float(0x5adada00), SkBits2Float(0xa5252652), SkBits2Float(0x727272ad), SkBits2Float(0xda007b72));  // 3.08006e+16f, -1.43245e-16f, 4.80218e+30f, -9.04113e+15f
path.lineTo(SkBits2Float(0x5252525a), SkBits2Float(0x72525252));  // 2.25831e+11f, 4.16585e+30f
path.quadTo(SkBits2Float(0x72727272), SkBits2Float(0x52525252), SkBits2Float(0x27725252), SkBits2Float(0x72727272));  // 4.80216e+30f, 2.25831e+11f, 3.36289e-15f, 4.80216e+30f
path.quadTo(SkBits2Float(0x72727272), SkBits2Float(0x74217472), SkBits2Float(0x005b5574), SkBits2Float(0x72680000));  // 4.80216e+30f, 5.11671e+31f, 8.38768e-39f, 4.59523e+30f
path.quadTo(SkBits2Float(0x72727272), SkBits2Float(0x52525252), SkBits2Float(0x007b7272), SkBits2Float(0x525adada));  // 4.80216e+30f, 2.25831e+11f, 1.13368e-38f, 2.34994e+11f
path.lineTo(SkBits2Float(0x72727200), SkBits2Float(0x72725252));  // 4.80212e+30f, 4.79967e+30f
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 4, filename);
}


// afl crash
static void fuzz763_55(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x55415500));  // 0, 1.32857e+13f
path.lineTo(SkBits2Float(0x55555568), SkBits2Float(0x55555555));  // 1.46602e+13f, 1.46602e+13f
path.lineTo(SkBits2Float(0x98989898), SkBits2Float(0x55989898));  // -3.94452e-24f, 2.09726e+13f
path.lineTo(SkBits2Float(0xf6f65555), SkBits2Float(0x101006f6));  // -2.49812e+33f, 2.84044e-29f
path.quadTo(SkBits2Float(0xdca33f10), SkBits2Float(0xf6f6f6f6), SkBits2Float(0xf621f6f6), SkBits2Float(0xf70ff6f6));  // -3.67598e+17f, -2.50452e+33f, -8.21259e+32f, -2.91995e+33f
path.lineTo(SkBits2Float(0x9400f6f6), SkBits2Float(0x10530000));  // -6.51105e-27f, 4.16124e-29f
path.quadTo(SkBits2Float(0x0f101010), SkBits2Float(0x00101010), SkBits2Float(0xf610f720), SkBits2Float(0xf6f6f6f6));  // 7.10284e-30f, 1.47513e-39f, -7.35062e+32f, -2.50452e+33f
path.lineTo(SkBits2Float(0x105352f6), SkBits2Float(0x1cf6ff10));  // 4.16763e-29f, 1.63448e-21f
path.lineTo(SkBits2Float(0xf6f6220a), SkBits2Float(0x003700f6));  // -2.49608e+33f, 5.0513e-39f
path.cubicTo(SkBits2Float(0x0000001e), SkBits2Float(0x00fff4f6), SkBits2Float(0xff101064), SkBits2Float(0xf6b6ac7f), SkBits2Float(0xf6f629f6), SkBits2Float(0x10f6f6f6));  // 4.2039e-44f, 2.35059e-38f, -1.91494e+38f, -1.85253e+33f, -2.4964e+33f, 9.74104e-29f
path.quadTo(SkBits2Float(0x10101007), SkBits2Float(0x10f7fd10), SkBits2Float(0xf6f6f6f6), SkBits2Float(0xf6f645e0));  // 2.84113e-29f, 9.78142e-29f, -2.50452e+33f, -2.4975e+33f
path.lineTo(SkBits2Float(0xed9ef6f6), SkBits2Float(0x53535353));  // -6.14965e+27f, 9.07636e+11f
path.lineTo(SkBits2Float(0x53006cf6), SkBits2Float(0x53295353));  // 5.51584e+11f, 7.27247e+11f
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0x55415500));  // 0, 1.32857e+13f
path.close();
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x55415500));  // 0, 1.32857e+13f
path.lineTo(SkBits2Float(0xf6f6f6f6), SkBits2Float(0x5353d9f6));  // -2.50452e+33f, 9.09895e+11f

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 3, filename);
}

// 656149
static void fuzz763_56(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.conicTo(SkBits2Float(0x5b682968), SkBits2Float(0xb3b32d11), SkBits2Float(0xb3b3b3b3), SkBits2Float(0x5b29b3b3), SkBits2Float(0x72725255));  // 6.53477e+16f, -8.34353e-08f, -8.36802e-08f, 4.77669e+16f, 4.79967e+30f
path.quadTo(SkBits2Float(0x525252c7), SkBits2Float(0x72725252), SkBits2Float(0x72727272), SkBits2Float(0x72727255));  // 2.25833e+11f, 4.79967e+30f, 4.80216e+30f, 4.80215e+30f
path.quadTo(SkBits2Float(0xd7da0000), SkBits2Float(0x5adada00), SkBits2Float(0x52525252), SkBits2Float(0x00005252));  // -4.79387e+14f, 3.08006e+16f, 2.25831e+11f, 2.9531e-41f
path.conicTo(SkBits2Float(0xadada525), SkBits2Float(0x52525ab4), SkBits2Float(0x52525252), SkBits2Float(0x72727272), SkBits2Float(0x52527272));  // -1.97412e-11f, 2.25866e+11f, 2.25831e+11f, 4.80216e+30f, 2.25966e+11f
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.close();
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.quadTo(SkBits2Float(0x72725252), SkBits2Float(0x72727272), SkBits2Float(0x72727255), SkBits2Float(0xda007b72));  // 4.79967e+30f, 4.80216e+30f, 4.80215e+30f, -9.04113e+15f
path.lineTo(SkBits2Float(0x5252525a), SkBits2Float(0x72525252));  // 2.25831e+11f, 4.16585e+30f
path.quadTo(SkBits2Float(0x72727272), SkBits2Float(0x52525252), SkBits2Float(0x27725252), SkBits2Float(0x72727272));  // 4.80216e+30f, 2.25831e+11f, 3.36289e-15f, 4.80216e+30f
path.lineTo(SkBits2Float(0x7bc00321), SkBits2Float(0x9aaaaaaa));  // 1.99397e+36f, -7.05861e-23f
path.quadTo(SkBits2Float(0x72725572), SkBits2Float(0x00007272), SkBits2Float(0x525adada), SkBits2Float(0x52525252));  // 4.79991e+30f, 4.10552e-41f, 2.34994e+11f, 2.25831e+11f
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.close();
path.moveTo(SkBits2Float(0xa5252600), SkBits2Float(0x52b4adad));  // -1.43243e-16f, 3.88004e+11f
path.close();
path.moveTo(SkBits2Float(0xa5252600), SkBits2Float(0x52b4adad));  // -1.43243e-16f, 3.88004e+11f
path.quadTo(SkBits2Float(0x72727270), SkBits2Float(0x52525272), SkBits2Float(0x72525252), SkBits2Float(0x72727272));  // 4.80216e+30f, 2.25832e+11f, 4.16585e+30f, 4.80216e+30f
path.quadTo(SkBits2Float(0x72727255), SkBits2Float(0xda007b72), SkBits2Float(0x26525ada), SkBits2Float(0x72ada525));  // 4.80215e+30f, -9.04113e+15f, 7.29815e-16f, 6.87879e+30f
path.quadTo(SkBits2Float(0x007b7272), SkBits2Float(0x525adada), SkBits2Float(0x52525252), SkBits2Float(0x72727252));  // 1.13368e-38f, 2.34994e+11f, 2.25831e+11f, 4.80215e+30f
path.quadTo(SkBits2Float(0x52527272), SkBits2Float(0x52525252), SkBits2Float(0x72722772), SkBits2Float(0x72727272));  // 2.25966e+11f, 2.25831e+11f, 4.79636e+30f, 4.80216e+30f
path.quadTo(SkBits2Float(0x74727272), SkBits2Float(0x55747421), SkBits2Float(0x0000005b), SkBits2Float(0x72727268));  // 7.68345e+31f, 1.67987e+13f, 1.27518e-43f, 4.80216e+30f
path.quadTo(SkBits2Float(0x52527272), SkBits2Float(0x52525252), SkBits2Float(0x72727272), SkBits2Float(0x72557272));  // 2.25966e+11f, 2.25831e+11f, 4.80216e+30f, 4.22775e+30f
path.quadTo(SkBits2Float(0x5adada72), SkBits2Float(0x52525252), SkBits2Float(0x72725252), SkBits2Float(0x72727272));  // 3.08009e+16f, 2.25831e+11f, 4.79967e+30f, 4.80216e+30f

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 0, filename);
}

static void fuzz763_57(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x68546829), SkBits2Float(0x555b2d29));  // 4.01225e+24f, 1.50617e+13f
path.moveTo(SkBits2Float(0x1f2a322a), SkBits2Float(0x4b7b2108));  // 3.60404e-20f, 1.6458e+07f
path.lineTo(SkBits2Float(0x2829ed84), SkBits2Float(0x5b2d2d55));  // 9.43289e-15f, 4.8745e+16f
path.moveTo(SkBits2Float(0x6838552d), SkBits2Float(0xf0684f5b));  // 3.48195e+24f, -2.87586e+29f
path.conicTo(SkBits2Float(0x212a1f5b), SkBits2Float(0x2a8cef2a), SkBits2Float(0x682d2953), SkBits2Float(0xce682103), SkBits2Float(0x4b7bc055));  // 5.76397e-19f, 2.50349e-13f, 3.27093e+24f, -9.73619e+08f, 1.64988e+07f
path.lineTo(SkBits2Float(0x5b2c6829), SkBits2Float(0x3b2a8c55));  // 4.85282e+16f, 0.00260236f
path.lineTo(SkBits2Float(0x6838552d), SkBits2Float(0xf0684f5b));  // 3.48195e+24f, -2.87586e+29f
path.close();
path.moveTo(SkBits2Float(0x6838552d), SkBits2Float(0xf0684f5b));  // 3.48195e+24f, -2.87586e+29f
path.conicTo(SkBits2Float(0xd2c00321), SkBits2Float(0xc0394b7b), SkBits2Float(0x8c08ed7a), SkBits2Float(0x211f2f2a), SkBits2Float(0x704b7b03));  // -4.12343e+11f, -2.89523f, -1.05485e-31f, 5.39337e-19f, 2.51897e+29f
path.cubicTo(SkBits2Float(0x2d6829ed), SkBits2Float(0x5b2d555b), SkBits2Float(0x68275b2d), SkBits2Float(0x21685527), SkBits2Float(0x0321082a), SkBits2Float(0x6a4b7bc0));  // 1.3197e-11f, 4.8789e+16f, 3.16127e+24f, 7.87174e-19f, 4.7323e-37f, 6.14991e+25f
path.conicTo(SkBits2Float(0x212a8ced), SkBits2Float(0x0321081f), SkBits2Float(0x6a4b7bc0), SkBits2Float(0x2829ed84), SkBits2Float(0x5b2d2d55));  // 5.77848e-19f, 4.7323e-37f, 6.14991e+25f, 9.43289e-15f, 4.8745e+16f
path.moveTo(SkBits2Float(0x6839552d), SkBits2Float(0xf0683b5b));  // 3.50084e+24f, -2.87489e+29f
path.conicTo(SkBits2Float(0x212a1f5b), SkBits2Float(0x228cef2a), SkBits2Float(0x682d2953), SkBits2Float(0xee682103), SkBits2Float(0x287bc055));  // 5.76397e-19f, 3.82003e-18f, 3.27093e+24f, -1.79601e+28f, 1.3975e-14f
path.lineTo(SkBits2Float(0x5b2c6829), SkBits2Float(0x212a8c55));  // 4.85282e+16f, 5.7784e-19f
path.conicTo(SkBits2Float(0x4b03213b), SkBits2Float(0xc07b2a08), SkBits2Float(0x5b2d7a6a), SkBits2Float(0xf0556830), SkBits2Float(0x2a8c555b));  // 8.59372e+06f, -3.92444f, 4.88298e+16f, -2.64185e+29f, 2.49282e-13f
path.conicTo(SkBits2Float(0x0321212a), SkBits2Float(0x4b7bd2c0), SkBits2Float(0xed7ac039), SkBits2Float(0x2f2a8c08), SkBits2Float(0x7b03211f));  // 4.73517e-37f, 1.65035e+07f, -4.85023e+27f, 1.55112e-10f, 6.80863e+35f
path.lineTo(SkBits2Float(0x6839552d), SkBits2Float(0xf0683b5b));  // 3.50084e+24f, -2.87489e+29f
path.close();
path.moveTo(SkBits2Float(0x6839552d), SkBits2Float(0xf0683b5b));  // 3.50084e+24f, -2.87489e+29f
path.lineTo(SkBits2Float(0x6829ed27), SkBits2Float(0x2d555b2d));  // 3.20982e+24f, 1.21279e-11f
path.moveTo(SkBits2Float(0x68275b2d), SkBits2Float(0xf0685527));  // 3.16127e+24f, -2.87614e+29f
path.conicTo(SkBits2Float(0x721f2a5b), SkBits2Float(0x212a8c55), SkBits2Float(0x0321082a), SkBits2Float(0x6a4b7b28), SkBits2Float(0x4797ed7a));  // 3.1526e+30f, 5.7784e-19f, 4.7323e-37f, 6.14984e+25f, 77787
path.lineTo(SkBits2Float(0x68275b2d), SkBits2Float(0xf0685527));  // 3.16127e+24f, -2.87614e+29f
path.close();
path.moveTo(SkBits2Float(0x68275b2d), SkBits2Float(0xf0685527));  // 3.16127e+24f, -2.87614e+29f
path.quadTo(SkBits2Float(0x2828102a), SkBits2Float(0x2c682921), SkBits2Float(0x8c555bf6), SkBits2Float(0x6d03de30));  // 9.32938e-15f, 3.2992e-12f, -1.64366e-31f, 2.5507e+27f
path.cubicTo(SkBits2Float(0x683f2d55), SkBits2Float(0xf05b684b), SkBits2Float(0x8c55272d), SkBits2Float(0x212a292a), SkBits2Float(0x0321082a), SkBits2Float(0x211f2a21));  // 3.61123e+24f, -2.71613e+29f, -1.64207e-31f, 5.76527e-19f, 4.7323e-37f, 5.39271e-19f
path.lineTo(SkBits2Float(0x3a803adf), SkBits2Float(0x8a294f1a));  // 0.000978317f, -8.15193e-33f
path.quadTo(SkBits2Float(0x291d9628), SkBits2Float(0x2a43e62b), SkBits2Float(0x093a2a81), SkBits2Float(0x5c5c8ced));  // 3.49912e-14f, 1.73993e-13f, 2.24089e-33f, 2.48318e+17f
path.lineTo(SkBits2Float(0x68275b2d), SkBits2Float(0xf0685527));  // 3.16127e+24f, -2.87614e+29f
path.close();
path.moveTo(SkBits2Float(0x68275b2d), SkBits2Float(0xf0685527));  // 3.16127e+24f, -2.87614e+29f
path.cubicTo(SkBits2Float(0x3ac2213a), SkBits2Float(0x291d9628), SkBits2Float(0x2a43e62b), SkBits2Float(0x293a2a81), SkBits2Float(0x5c5c8ced), SkBits2Float(0x5c5c6e5c));  // 0.00148109f, 3.49912e-14f, 1.73993e-13f, 4.13372e-14f, 2.48318e+17f, 2.48183e+17f
path.lineTo(SkBits2Float(0x1f212a8c), SkBits2Float(0xc0032108));  // 3.41283e-20f, -2.04889f
path.lineTo(SkBits2Float(0xed847b4b), SkBits2Float(0x2d552829));  // -5.12513e+27f, 1.21166e-11f
path.conicTo(SkBits2Float(0x552d5b5b), SkBits2Float(0x3b5a6839), SkBits2Float(0x5b2df068), SkBits2Float(0x2a212a1f), SkBits2Float(0x532a8cef));  // 1.1913e+13f, 0.00333263f, 4.89595e+16f, 1.43143e-13f, 7.32509e+11f

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 0, filename);
}

static void fuzzhang_1(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.cubicTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000), SkBits2Float(0x668ece09), SkBits2Float(0x00000000), SkBits2Float(0x6751c81a), SkBits2Float(0x61c4b0fb));  // 0, 0, 3.37188e+23f, 0, 9.90666e+23f, 4.53539e+20f
path.conicTo(SkBits2Float(0x66f837a9), SkBits2Float(0x00000000), SkBits2Float(0x00000000), SkBits2Float(0x00000000), SkBits2Float(0x3f823406));  // 5.86087e+23f, 0, 0, 0, 1.01721f
path.close();
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.quadTo(SkBits2Float(0x675b1bfe), SkBits2Float(0x00000000), SkBits2Float(0x67d76c42), SkBits2Float(0x6292c469));  // 1.03471e+24f, 0, 2.03461e+24f, 1.35369e+21f
path.cubicTo(SkBits2Float(0x6a16df68), SkBits2Float(0x651a2f15), SkBits2Float(0x6c1e7f31), SkBits2Float(0x67a1f9b4), SkBits2Float(0x00000000), SkBits2Float(0x6a2a291f));  // 4.55985e+25f, 4.55071e+22f, 7.66444e+26f, 1.52981e+24f, 0, 5.14279e+25f
path.conicTo(SkBits2Float(0x680dcb75), SkBits2Float(0x68dd898d), SkBits2Float(0x681a434a), SkBits2Float(0x6871046b), SkBits2Float(0x3fea0440));  // 2.67843e+24f, 8.36944e+24f, 2.91394e+24f, 4.55269e+24f, 1.82825f
path.quadTo(SkBits2Float(0x679e1b26), SkBits2Float(0x687703c4), SkBits2Float(0x00000000), SkBits2Float(0x687d2968));  // 1.49327e+24f, 4.66598e+24f, 0, 4.78209e+24f
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.cubicTo(SkBits2Float(0x535353ec), SkBits2Float(0x98989898), SkBits2Float(0x98989898), SkBits2Float(0xf207f36e), SkBits2Float(0xf3f2f2f2), SkBits2Float(0xed3a9781));  // 9.07646e+11f, -3.94452e-24f, -3.94452e-24f, -2.69278e+30f, -3.84968e+31f, -3.60921e+27f
path.quadTo(SkBits2Float(0xf8f8c0ed), SkBits2Float(0xf8f8f8f8), SkBits2Float(0x9f9f9f9f), SkBits2Float(0x3014149f));  // -4.03626e+34f, -4.03981e+34f, -6.76032e-20f, 5.38714e-10f

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 0, filename);
}

static void release_13(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.setFillType(SkPath::kEvenOdd_FillType);
path.moveTo(SkBits2Float(0xd4438848), SkBits2Float(0xd488cf64));  // -3.35922e+12f, -4.70076e+12f
path.lineTo(SkBits2Float(0xd43a056e), SkBits2Float(0xd4851696));  // -3.19582e+12f, -4.57288e+12f
path.quadTo(SkBits2Float(0xd3d48e79), SkBits2Float(0xd49fb136), SkBits2Float(0x00000000), SkBits2Float(0xd4d4d4d4));  // -1.82585e+12f, -5.48698e+12f, 0, -7.31283e+12f
path.quadTo(SkBits2Float(0xd3d06670), SkBits2Float(0xd4a0bb38), SkBits2Float(0xd41d628f), SkBits2Float(0xd472c531));  // -1.79014e+12f, -5.52269e+12f, -2.70385e+12f, -4.17076e+12f
path.lineTo(SkBits2Float(0xd43a0559), SkBits2Float(0xd485168e));  // -3.19581e+12f, -4.57287e+12f
path.lineTo(SkBits2Float(0xd446958b), SkBits2Float(0xd4810278));  // -3.41165e+12f, -4.43274e+12f
path.lineTo(SkBits2Float(0xd443884a), SkBits2Float(0xd488cf65));  // -3.35922e+12f, -4.70076e+12f
path.quadTo(SkBits2Float(0xd47efa09), SkBits2Float(0xd49fd72a), SkBits2Float(0xd4a63f0f), SkBits2Float(0xd4b83ab3));  // -4.38047e+12f, -5.49208e+12f, -5.71218e+12f, -6.33007e+12f
path.lineTo(SkBits2Float(0xd497ca70), SkBits2Float(0xd4c4d4ae));  // -5.21549e+12f, -6.76305e+12f
path.lineTo(SkBits2Float(0xd459d4d4), SkBits2Float(0xd4c4d4d4));  // -3.74231e+12f, -6.76307e+12f
path.lineTo(SkBits2Float(0xd440daf9), SkBits2Float(0xd4c632d3));  // -3.31323e+12f, -6.81005e+12f
path.lineTo(SkBits2Float(0xd4438848), SkBits2Float(0xd488cf64));  // -3.35922e+12f, -4.70076e+12f
path.close();
path.moveTo(SkBits2Float(0xd4767560), SkBits2Float(0xd4d1ca84));  // -4.23412e+12f, -7.20837e+12f
path.lineTo(SkBits2Float(0xd4422174), SkBits2Float(0xd4d02069));  // -3.33514e+12f, -7.15118e+12f
path.lineTo(SkBits2Float(0xd440daa3), SkBits2Float(0xd4c632d9));  // -3.31321e+12f, -6.81005e+12f
path.lineTo(SkBits2Float(0xd41017bc), SkBits2Float(0xd4cb99b6));  // -2.47549e+12f, -6.99566e+12f
path.lineTo(SkBits2Float(0xd442213b), SkBits2Float(0xd4d02067));  // -3.33512e+12f, -7.15117e+12f
path.lineTo(SkBits2Float(0xd442d4d4), SkBits2Float(0xd4d4d4d4));  // -3.34718e+12f, -7.31283e+12f
path.lineTo(SkBits2Float(0xd4767560), SkBits2Float(0xd4d1ca84));  // -4.23412e+12f, -7.20837e+12f
path.close();
path.moveTo(SkBits2Float(0xd46c7a11), SkBits2Float(0xd46c7a2e));  // -4.06264e+12f, -4.06265e+12f
path.lineTo(SkBits2Float(0xd484e02c), SkBits2Float(0xd45fafcd));  // -4.56557e+12f, -3.84291e+12f
path.lineTo(SkBits2Float(0xd462c867), SkBits2Float(0xd45655f7));  // -3.8961e+12f, -3.68226e+12f
path.lineTo(SkBits2Float(0xd45ac463), SkBits2Float(0xd45ac505));  // -3.75839e+12f, -3.75843e+12f
path.lineTo(SkBits2Float(0xd43d2fa9), SkBits2Float(0xd43d2fb5));  // -3.25019e+12f, -3.2502e+12f
path.lineTo(SkBits2Float(0xd41d6287), SkBits2Float(0xd472c52a));  // -2.70385e+12f, -4.17076e+12f
path.quadTo(SkBits2Float(0x00000000), SkBits2Float(0xd3db1b95), SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, -1.88212e+12f, 0, 0
path.quadTo(SkBits2Float(0xd4b7efac), SkBits2Float(0x00000000), SkBits2Float(0xd4d0e88f), SkBits2Float(0xd40b8b46));  // -6.32e+12f, 0, -7.17804e+12f, -2.39735e+12f
path.lineTo(SkBits2Float(0xd4d4d4d4), SkBits2Float(0x00000000));  // -7.31283e+12f, 0
path.lineTo(SkBits2Float(0xdcdc154b), SkBits2Float(0x00000000));  // -4.95583e+17f, 0
path.lineTo(SkBits2Float(0xd4d4d4d4), SkBits2Float(0xd4c4d477));  // -7.31283e+12f, -6.76303e+12f
path.lineTo(SkBits2Float(0xd4d4d4d4), SkBits2Float(0xd4d4d442));  // -7.31283e+12f, -7.31275e+12f
path.lineTo(SkBits2Float(0xd4d4a691), SkBits2Float(0xd4d4d442));  // -7.30662e+12f, -7.31275e+12f
path.lineTo(SkBits2Float(0xd454d4d4), SkBits2Float(0xd4d4aa30));  // -3.65641e+12f, -7.30711e+12f
path.lineTo(SkBits2Float(0xd4bd9def), SkBits2Float(0xd4d43df0));  // -6.51519e+12f, -7.29258e+12f
path.lineTo(SkBits2Float(0xd4767560), SkBits2Float(0xd4d1ca84));  // -4.23412e+12f, -7.20837e+12f
path.lineTo(SkBits2Float(0xd497ca70), SkBits2Float(0xd4c4d4ae));  // -5.21549e+12f, -6.76305e+12f
path.lineTo(SkBits2Float(0xd4bab953), SkBits2Float(0xd4c4d48e));  // -6.41579e+12f, -6.76304e+12f
path.lineTo(SkBits2Float(0xd4a63f0f), SkBits2Float(0xd4b83ab3));  // -5.71218e+12f, -6.33007e+12f
path.lineTo(SkBits2Float(0xd4ae61eb), SkBits2Float(0xd4ae61f4));  // -5.99174e+12f, -5.99174e+12f
path.lineTo(SkBits2Float(0xd46c7a11), SkBits2Float(0xd46c7a2e));  // -4.06264e+12f, -4.06265e+12f
path.close();
path.moveTo(SkBits2Float(0xd46c7a11), SkBits2Float(0xd46c7a2e));  // -4.06264e+12f, -4.06265e+12f
path.lineTo(SkBits2Float(0xd446965c), SkBits2Float(0xd4810237));  // -3.4117e+12f, -4.4327e+12f
path.lineTo(SkBits2Float(0xd45ac549), SkBits2Float(0xd45ac55f));  // -3.75845e+12f, -3.75846e+12f
path.lineTo(SkBits2Float(0xd46c7a11), SkBits2Float(0xd46c7a2e));  // -4.06264e+12f, -4.06265e+12f
path.close();
path.moveTo(SkBits2Float(0xd4b46028), SkBits2Float(0xd41e572a));  // -6.19766e+12f, -2.72027e+12f
path.lineTo(SkBits2Float(0xd4cde20a), SkBits2Float(0xd434bb57));  // -7.07408e+12f, -3.10495e+12f
path.lineTo(SkBits2Float(0xd4c75ffe), SkBits2Float(0xd46f215d));  // -6.85047e+12f, -4.10823e+12f
path.lineTo(SkBits2Float(0xd4b46028), SkBits2Float(0xd41e572a));  // -6.19766e+12f, -2.72027e+12f
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.setFillType(SkPath::kWinding_FillType);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.quadTo(SkBits2Float(0x00000000), SkBits2Float(0xa5a50000), SkBits2Float(0xd4d4a5a5), SkBits2Float(0xd4d4d4d4));  // 0, -2.86229e-16f, -7.3065e+12f, -7.31283e+12f
path.quadTo(SkBits2Float(0xd4d4d4d4), SkBits2Float(0xd4d4d4d4), SkBits2Float(0xd4cfd4d4), SkBits2Float(0xd4d41dd4));  // -7.31283e+12f, -7.31283e+12f, -7.14103e+12f, -7.28827e+12f
path.quadTo(SkBits2Float(0xd4d4d4d4), SkBits2Float(0xd4d432d4), SkBits2Float(0xd4d4d4d4), SkBits2Float(0xd4a5a5d4));  // -7.31283e+12f, -7.29109e+12f, -7.31283e+12f, -5.69161e+12f
path.quadTo(SkBits2Float(0xd4d4d4d4), SkBits2Float(0xd4d4d4d4), SkBits2Float(0xd4d4d4d4), SkBits2Float(0x00000000));  // -7.31283e+12f, -7.31283e+12f, -7.31283e+12f, 0
path.moveTo(SkBits2Float(0xa5a5a500), SkBits2Float(0xd4d4d4a5));  // -2.87347e-16f, -7.31281e+12f
path.quadTo(SkBits2Float(0xd4d4d4d4), SkBits2Float(0x2ad4d4d4), SkBits2Float(0xd4d4d4d4), SkBits2Float(0xd4cfd4d4));  // -7.31283e+12f, 3.78064e-13f, -7.31283e+12f, -7.14103e+12f
path.quadTo(SkBits2Float(0xd4d4d4d4), SkBits2Float(0xd4d4d4d4), SkBits2Float(0xd4d4d4d4), SkBits2Float(0xd4d4d4d4));  // -7.31283e+12f, -7.31283e+12f, -7.31283e+12f, -7.31283e+12f
path.quadTo(SkBits2Float(0xd4d40000), SkBits2Float(0xd4d4d4d4), SkBits2Float(0xd4d4d4d4), SkBits2Float(0xd4d4d4d4));  // -7.28426e+12f, -7.31283e+12f, -7.31283e+12f, -7.31283e+12f

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 2, filename);
}

static void fuzzhang_2(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.setFillType(SkPath::kWinding_FillType);
path.moveTo(SkBits2Float(0x5568392a), SkBits2Float(0x72837268));  // 1.59583e+13f, 5.20715e+30f
path.quadTo(SkBits2Float(0xe0e02972), SkBits2Float(0xe0e060e0), SkBits2Float(0x728e4603), SkBits2Float(0x72727272));  // -1.29221e+20f, -1.29345e+20f, 5.63603e+30f, 4.80216e+30f
path.lineTo(SkBits2Float(0x5568392a), SkBits2Float(0x72837268));  // 1.59583e+13f, 5.20715e+30f
path.close();
path.moveTo(SkBits2Float(0x5568392a), SkBits2Float(0x72837268));  // 1.59583e+13f, 5.20715e+30f
path.quadTo(SkBits2Float(0x68720052), SkBits2Float(0x52527372), SkBits2Float(0x00527252), SkBits2Float(0x728e4601));  // 4.57127e+24f, 2.2597e+11f, 7.57152e-39f, 5.63603e+30f
path.quadTo(SkBits2Float(0x52ec7272), SkBits2Float(0x6265527f), SkBits2Float(0x8e460152), SkBits2Float(0x72ff8072));  // 5.07766e+11f, 1.05756e+21f, -2.4406e-30f, 1.01215e+31f
path.lineTo(SkBits2Float(0x5568392a), SkBits2Float(0x72837268));  // 1.59583e+13f, 5.20715e+30f
path.close();
path.moveTo(SkBits2Float(0x5568392a), SkBits2Float(0x72837268));  // 1.59583e+13f, 5.20715e+30f
path.lineTo(SkBits2Float(0x52626552), SkBits2Float(0x72727272));  // 2.43091e+11f, 4.80216e+30f
path.quadTo(SkBits2Float(0x72727272), SkBits2Float(0x62727272), SkBits2Float(0x39393939), SkBits2Float(0x728bc739));  // 4.80216e+30f, 1.11809e+21f, 0.000176643f, 5.53719e+30f
path.cubicTo(SkBits2Float(0x72728092), SkBits2Float(0x72727260), SkBits2Float(0x4d727272), SkBits2Float(0x5252522a), SkBits2Float(0x72735252), SkBits2Float(0x72707272));  // 4.80325e+30f, 4.80215e+30f, 2.54224e+08f, 2.2583e+11f, 4.81948e+30f, 4.76254e+30f
path.quadTo(SkBits2Float(0x72727272), SkBits2Float(0x56727272), SkBits2Float(0x72720152), SkBits2Float(0x72727270));  // 4.80216e+30f, 6.66433e+13f, 4.79341e+30f, 4.80216e+30f
path.quadTo(SkBits2Float(0x52526172), SkBits2Float(0x8e460300), SkBits2Float(0x72727272), SkBits2Float(0x52525272));  // 2.25894e+11f, -2.44068e-30f, 4.80216e+30f, 2.25832e+11f
path.conicTo(SkBits2Float(0xb5727272), SkBits2Float(0x7f2b727f), SkBits2Float(0x607272ff), SkBits2Float(0x72727276), SkBits2Float(0x2a527272));  // -9.03186e-07f, 2.27892e+38f, 6.98812e+19f, 4.80216e+30f, 1.86915e-13f
path.lineTo(SkBits2Float(0x5568392a), SkBits2Float(0x72837268));  // 1.59583e+13f, 5.20715e+30f
path.close();
path.moveTo(SkBits2Float(0x5568392a), SkBits2Float(0x72837268));  // 1.59583e+13f, 5.20715e+30f
path.lineTo(SkBits2Float(0x72727272), SkBits2Float(0x52525f72));  // 4.80216e+30f, 2.25886e+11f
path.lineTo(SkBits2Float(0x5568392a), SkBits2Float(0x72837268));  // 1.59583e+13f, 5.20715e+30f
path.close();
path.moveTo(SkBits2Float(0x5568392a), SkBits2Float(0x72837268));  // 1.59583e+13f, 5.20715e+30f
path.quadTo(SkBits2Float(0x52727272), SkBits2Float(0x64655252), SkBits2Float(0x72c1c152), SkBits2Float(0x72727272));  // 2.60326e+11f, 1.69209e+22f, 7.67543e+30f, 4.80216e+30f

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.setFillType(SkPath::kWinding_FillType);

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 1, filename);
}

static void fuzzhang_3(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.setFillType(SkPath::kWinding_FillType);

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.setFillType(SkPath::kWinding_FillType);
path.moveTo(SkBits2Float(0x46090052), SkBits2Float(0x7270726c));  // 8768.08f, 4.76254e+30f
path.moveTo(SkBits2Float(0xe0437272), SkBits2Float(0x03e0e060));  // -5.63338e+19f, 1.32171e-36f
path.close();
path.moveTo(SkBits2Float(0xe0437272), SkBits2Float(0x03e0e060));  // -5.63338e+19f, 1.32171e-36f
path.lineTo(SkBits2Float(0x77727272), SkBits2Float(0x52520072));  // 4.91741e+33f, 2.25488e+11f
path.lineTo(SkBits2Float(0x46090052), SkBits2Float(0x727272ce));  // 8768.08f, 4.80219e+30f
path.quadTo(SkBits2Float(0x725252ec), SkBits2Float(0x72727272), SkBits2Float(0x72727272), SkBits2Float(0x39393962));  // 4.16589e+30f, 4.80216e+30f, 4.80216e+30f, 0.000176644f
path.lineTo(SkBits2Float(0x6c460900), SkBits2Float(0x72727072));  // 9.57639e+26f, 4.802e+30f
path.cubicTo(SkBits2Float(0xe0e060e0), SkBits2Float(0x72943603), SkBits2Float(0x72777272), SkBits2Float(0x5c525200), SkBits2Float(0x46090052), SkBits2Float(0x727272ce));  // -1.29345e+20f, 5.87124e+30f, 4.90119e+30f, 2.368e+17f, 8768.08f, 4.80219e+30f
path.quadTo(SkBits2Float(0x725252ec), SkBits2Float(0x72727272), SkBits2Float(0x72727272), SkBits2Float(0x39393962));  // 4.16589e+30f, 4.80216e+30f, 4.80216e+30f, 0.000176644f
path.lineTo(SkBits2Float(0x0052ca00), SkBits2Float(0x728e4603));  // 7.60297e-39f, 5.63603e+30f
path.quadTo(SkBits2Float(0xff727272), SkBits2Float(0x52527272), SkBits2Float(0x39392072), SkBits2Float(0xe0393939));  // -3.22267e+38f, 2.25966e+11f, 0.000176551f, -5.3387e+19f
path.lineTo(SkBits2Float(0xe0437272), SkBits2Float(0x03e0e060));  // -5.63338e+19f, 1.32171e-36f
path.close();
path.moveTo(SkBits2Float(0xe0437272), SkBits2Float(0x03e0e060));  // -5.63338e+19f, 1.32171e-36f
path.cubicTo(SkBits2Float(0xdada7272), SkBits2Float(0x2dff7272), SkBits2Float(0x767272f0), SkBits2Float(0x72727272), SkBits2Float(0x21727f72), SkBits2Float(0x0b210929));  // -3.07437e+16f, 2.9041e-11f, 1.22936e+33f, 4.80216e+30f, 8.21615e-19f, 3.10144e-32f
path.cubicTo(SkBits2Float(0xd6d6d6d6), SkBits2Float(0x72a5d6d6), SkBits2Float(0x72553872), SkBits2Float(0xdada7072), SkBits2Float(0x5252525a), SkBits2Float(0x72727252));  // -1.18109e+14f, 6.56957e+30f, 4.22327e+30f, -3.07426e+16f, 2.25831e+11f, 4.80215e+30f
path.quadTo(SkBits2Float(0x72725572), SkBits2Float(0xdada0072), SkBits2Float(0x52524b5a), SkBits2Float(0x72528000));  // 4.79991e+30f, -3.0681e+16f, 2.25802e+11f, 4.16938e+30f
path.quadTo(SkBits2Float(0x72727272), SkBits2Float(0xca005252), SkBits2Float(0x46030052), SkBits2Float(0x7272728e));  // 4.80216e+30f, -2.10242e+06f, 8384.08f, 4.80217e+30f
path.quadTo(SkBits2Float(0x7272ff72), SkBits2Float(0x20725252), SkBits2Float(0x39393939), SkBits2Float(0xd76ee039));  // 4.81307e+30f, 2.05254e-19f, 0.000176643f, -2.62647e+14f
path.cubicTo(SkBits2Float(0xdada7272), SkBits2Float(0x2dff7272), SkBits2Float(0x767272f0), SkBits2Float(0x72727272), SkBits2Float(0x21727f72), SkBits2Float(0x0b210929));  // -3.07437e+16f, 2.9041e-11f, 1.22936e+33f, 4.80216e+30f, 8.21615e-19f, 3.10144e-32f
path.cubicTo(SkBits2Float(0xd6d6d6d6), SkBits2Float(0x72a5d6d6), SkBits2Float(0x72553872), SkBits2Float(0xdada7072), SkBits2Float(0x5252525a), SkBits2Float(0x72727252));  // -1.18109e+14f, 6.56957e+30f, 4.22327e+30f, -3.07426e+16f, 2.25831e+11f, 4.80215e+30f
path.quadTo(SkBits2Float(0x72725572), SkBits2Float(0xdada0072), SkBits2Float(0x52524b5a), SkBits2Float(0x72528000));  // 4.79991e+30f, -3.0681e+16f, 2.25802e+11f, 4.16938e+30f
path.quadTo(SkBits2Float(0x72727272), SkBits2Float(0x52525252), SkBits2Float(0x27725252), SkBits2Float(0x72727272));  // 4.80216e+30f, 2.25831e+11f, 3.36289e-15f, 4.80216e+30f
path.quadTo(SkBits2Float(0x72667254), SkBits2Float(0x00000040), SkBits2Float(0x00a70155), SkBits2Float(0x726800ff));  // 4.56447e+30f, 8.96831e-44f, 1.5337e-38f, 4.59531e+30f
path.quadTo(SkBits2Float(0x7b727272), SkBits2Float(0xad000c52), SkBits2Float(0x1c10adad), SkBits2Float(0x72728d8a));  // 1.25886e+36f, -7.27869e-12f, 4.78701e-22f, 4.80425e+30f
path.quadTo(SkBits2Float(0xff056546), SkBits2Float(0x727205ff), SkBits2Float(0x524b5aff), SkBits2Float(0x64005252));  // -1.77313e+38f, 4.79377e+30f, 2.18351e+11f, 9.46846e+21f
path.quadTo(SkBits2Float(0x72524872), SkBits2Float(0xdada7272), SkBits2Float(0x5252525a), SkBits2Float(0x72727252));  // 4.16508e+30f, -3.07437e+16f, 2.25831e+11f, 4.80215e+30f
path.quadTo(SkBits2Float(0x72724172), SkBits2Float(0xdad10072), SkBits2Float(0x52524b5a), SkBits2Float(0x725b8000));  // 4.79837e+30f, -2.94144e+16f, 2.25802e+11f, 4.34765e+30f
path.quadTo(SkBits2Float(0x72727272), SkBits2Float(0x52525252), SkBits2Float(0x27725252), SkBits2Float(0x72727272));  // 4.80216e+30f, 2.25831e+11f, 3.36289e-15f, 4.80216e+30f
path.quadTo(SkBits2Float(0x72728372), SkBits2Float(0x00000040), SkBits2Float(0xf6a70147), SkBits2Float(0xc2c2c256));  // 4.80347e+30f, 8.96831e-44f, -1.69363e+33f, -97.3796f
path.lineTo(SkBits2Float(0xe0437272), SkBits2Float(0x03e0e060));  // -5.63338e+19f, 1.32171e-36f
path.close();
path.moveTo(SkBits2Float(0x7a787a7a), SkBits2Float(0x7a3a7a7a));  // 3.22543e+35f, 2.42063e+35f
path.lineTo(SkBits2Float(0x8f4603e0), SkBits2Float(0x72727272));  // -9.7629e-30f, 4.80216e+30f
path.quadTo(SkBits2Float(0x00807272), SkBits2Float(0x46090052), SkBits2Float(0x7270726c), SkBits2Float(0x60e04372));  // 1.1796e-38f, 8768.08f, 4.76254e+30f, 1.29279e+20f
path.moveTo(SkBits2Float(0x943603e0), SkBits2Float(0x77727272));  // -9.18942e-27f, 4.91741e+33f
path.quadTo(SkBits2Float(0x5c525200), SkBits2Float(0x46090052), SkBits2Float(0x727272ce), SkBits2Float(0x5252ec72));  // 2.368e+17f, 8768.08f, 4.80219e+30f, 2.26478e+11f

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 3, filename);
}

static void fuzz754434_1(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.setFillType(SkPath::kWinding_FillType);

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.setFillType(SkPath::kWinding_FillType);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.cubicTo(SkBits2Float(0x535e5372), SkBits2Float(0x53536153), SkBits2Float(0x79530f53), SkBits2Float(0x101b6c88), SkBits2Float(0x5353735e), SkBits2Float(0x006df653));  // 9.54883e+11f, 9.07871e+11f, 6.84928e+34f, 3.0652e-29f, 9.08174e+11f, 1.00984e-38f
path.cubicTo(SkBits2Float(0xf26df46d), SkBits2Float(0xf6f6f6f6), SkBits2Float(0x5656f666), SkBits2Float(0x5a565656), SkBits2Float(0x00000056), SkBits2Float(0xf66e5600));  // -4.71318e+30f, -2.50452e+33f, 5.90884e+13f, 1.50826e+16f, 1.20512e-43f, -1.20851e+33f
path.lineTo(SkBits2Float(0xff00ff56), SkBits2Float(0x00faf6f6));  // -1.71467e+38f, 2.30475e-38f
path.moveTo(SkBits2Float(0x60576bfa), SkBits2Float(0x006df653));  // 6.20911e+19f, 1.00984e-38f
path.cubicTo(SkBits2Float(0xf26df46d), SkBits2Float(0xf653f6f6), SkBits2Float(0x563ef666), SkBits2Float(0x56565656), SkBits2Float(0x65565656), SkBits2Float(0xf6765656));  // -4.71318e+30f, -1.07479e+33f, 5.24914e+13f, 5.89166e+13f, 6.32612e+22f, -1.24908e+33f

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 3, filename);
}

static void fuzz754434_2(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.setFillType(SkPath::kEvenOdd_FillType);
path.moveTo(SkBits2Float(0xff00ff56), SkBits2Float(0x00000000));  // -1.71467e+38f, 0
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xf66e5600));  // 0, -1.20851e+33f
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xf629168b));  // 0, -8.57378e+32f
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.lineTo(SkBits2Float(0xff00ff56), SkBits2Float(0x00000000));  // -1.71467e+38f, 0
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.setFillType(SkPath::kWinding_FillType);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.lineTo(SkBits2Float(0x03e8f6f6), SkBits2Float(0xf7060000));  // 1.36924e-36f, -2.71784e+33f
path.lineTo(SkBits2Float(0x4ff6f6f6), SkBits2Float(0x3e3e3e2a));  // 8.28676e+09f, 0.185784f
path.conicTo(SkBits2Float(0x6c8879ff), SkBits2Float(0x08761b1b), SkBits2Float(0x7066662d), SkBits2Float(0x70707070), SkBits2Float(0x70707070));  // 1.31992e+27f, 7.40598e-34f, 2.8522e+29f, 2.97649e+29f, 2.97649e+29f

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 2, filename);
}

static void fuzz754434_3(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
path.setFillType(SkPath::kWinding_FillType);

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.setFillType(SkPath::kWinding_FillType);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.cubicTo(SkBits2Float(0x535e5372), SkBits2Float(0x53536153), SkBits2Float(0x79530f53), SkBits2Float(0x101b6c88), SkBits2Float(0x5353735e), SkBits2Float(0x006df653));  // 9.54883e+11f, 9.07871e+11f, 6.84928e+34f, 3.0652e-29f, 9.08174e+11f, 1.00984e-38f
path.cubicTo(SkBits2Float(0xf26df46d), SkBits2Float(0xf6f6f6f6), SkBits2Float(0x5656f666), SkBits2Float(0x5a565656), SkBits2Float(0x00000056), SkBits2Float(0xf66e5600));  // -4.71318e+30f, -2.50452e+33f, 5.90884e+13f, 1.50826e+16f, 1.20512e-43f, -1.20851e+33f
path.lineTo(SkBits2Float(0xff00ff56), SkBits2Float(0x00faf6f6));  // -1.71467e+38f, 2.30475e-38f
path.moveTo(SkBits2Float(0x60576bfa), SkBits2Float(0x006df653));  // 6.20911e+19f, 1.00984e-38f
path.cubicTo(SkBits2Float(0xf26df46d), SkBits2Float(0xf653f6f6), SkBits2Float(0x563ef666), SkBits2Float(0x56565656), SkBits2Float(0x65565656), SkBits2Float(0xf6765656));  // -4.71318e+30f, -1.07479e+33f, 5.24914e+13f, 5.89166e+13f, 6.32612e+22f, -1.24908e+33f

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 3, filename);
}

static void fuzz754434_4(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.setFillType(SkPath::kEvenOdd_FillType);
path.moveTo(SkBits2Float(0xff00ff56), SkBits2Float(0x00000000));  // -1.71467e+38f, 0
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xf66e5600));  // 0, -1.20851e+33f
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0xf629168b));  // 0, -8.57378e+32f
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.lineTo(SkBits2Float(0xff00ff56), SkBits2Float(0x00000000));  // -1.71467e+38f, 0
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.setFillType(SkPath::kWinding_FillType);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.lineTo(SkBits2Float(0x03e8f6f6), SkBits2Float(0xf7060000));  // 1.36924e-36f, -2.71784e+33f
path.lineTo(SkBits2Float(0x4ff6f6f6), SkBits2Float(0x3e3e3e2a));  // 8.28676e+09f, 0.185784f
path.conicTo(SkBits2Float(0x6c8879ff), SkBits2Float(0x08761b1b), SkBits2Float(0x7066662d), SkBits2Float(0x70707070), SkBits2Float(0x70707070));  // 1.31992e+27f, 7.40598e-34f, 2.8522e+29f, 2.97649e+29f, 2.97649e+29f

    SkPath path2(path);
    testPathOpFuzz(reporter, path1, path2, (SkPathOp) 2, filename);
}

static struct TestDesc failTests[] = {
    TEST(fuzz767834),
    TEST(fuzz754434_1),
    TEST(fuzz754434_2),
    TEST(fuzz754434_3),
    TEST(fuzz754434_4),
    TEST(fuzzhang_3),
    TEST(fuzzhang_2),
    TEST(release_13),
    TEST(fuzzhang_1),
    TEST(fuzz763_57),
    TEST(fuzz763_56),
    TEST(fuzz763_55),
    TEST(fuzz763_54),
    TEST(fuzz763_53),
    TEST(fuzz763_52),
    TEST(fuzz763_51),
    TEST(fuzz763_50),
    TEST(fuzz763_49),
    TEST(fuzz763_48),
    TEST(fuzz763_47),
    TEST(fuzz763_46),
    TEST(fuzz763_45),
    TEST(fuzz763_44),
    TEST(fuzz763_43),
    TEST(fuzz763_42),
    TEST(fuzz763_41),
    TEST(fuzz763_40),
    TEST(fuzz763_39),
    TEST(fuzz763_38),
    TEST(fuzz763_37),
    TEST(fuzz763_36),
    TEST(fuzz763_35),
    TEST(fuzz763_34),
    TEST(fuzz763_33),
    TEST(fuzz763_32),
    TEST(fuzz763_31),
    TEST(fuzz763_30),
    TEST(fuzz763_29),
    TEST(fuzz763_28),
    TEST(fuzz763_27),
    TEST(fuzz763_26),
    TEST(fuzz763_25),
    TEST(fuzz763_24),
    TEST(fuzz763_23),
    TEST(fuzz763_22),
    TEST(fuzz763_21),
    TEST(fuzz763_20),
    TEST(fuzz763_19),
    TEST(fuzz763_18),
    TEST(fuzz763_17),
    TEST(fuzz763_16),
    TEST(fuzz763_15),
    TEST(fuzz763_14),
    TEST(fuzz763_13),
    TEST(fuzz763_12),
    TEST(fuzz763_11),
    TEST(fuzz763_10),
    TEST(kfuzz2),
    TEST(fuzz763_7),
    TEST(fuzz763_6),
    TEST(fuzz763_2c),
    TEST(fuzz763_2b),
    TEST(fuzz763_2a),
    TEST(fuzz763_5a),
    TEST(fuzz763_3a),
    TEST(fuzz763_1a),
    TEST(fuzz763_1b),
    TEST(fuzz763_1c),
    TEST(fuzz763_2),
    TEST(fuzz763_5),
    TEST(fuzz763_3),
    TEST(fuzz763_4),
    TEST(fuzz763_9),
    TEST(fuzz1450_1),
    TEST(fuzz1450_0),
    TEST(bug597926_0),
    TEST(fuzz535151),
    TEST(fuzz753_91),
    TEST(fuzz714),
    TEST(fuzz487a),
    TEST(fuzz433),
    TEST(fuzz1),
    TEST(fuzz487b),
    TEST(fuzz433b),
    TEST(bufferOverflow),
};

static const size_t failTestCount = SK_ARRAY_COUNT(failTests);

DEF_TEST(PathOpsFailOp, reporter) {
#if DEBUG_SHOW_TEST_NAME
    strncpy(DEBUG_FILENAME_STRING, "", DEBUG_FILENAME_STRING_LENGTH);
#endif
    RunTestSet(reporter, failTests, failTestCount, nullptr, nullptr, nullptr, false);
}

static struct TestDesc repTests[] = {
    TEST(fuzz763_5a),
};

DEF_TEST(PathOpsRepOp, reporter) {
    if (PathOpsDebug::gJson) {
        return;
    }
  for (int index = 0; index < 1; ++index)
    RunTestSet(reporter, repTests, SK_ARRAY_COUNT(repTests), nullptr, nullptr, nullptr, false);
}
