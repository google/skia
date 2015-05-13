/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "PathOpsExtendedTest.h"
#include "PathOpsTestCommon.h"

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
        testPathOp(reporter, path, paths[index], ops[index], filename);
        Op(path, paths[index], ops[index], &path);
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

#include "SkPathOpsCubic.h"

static void loop1asQuad(skiatest::Reporter* reporter, const char* filename) {
    SkDCubic c1 = {{{0,1}, {1,5}, {-5.66666651f,3.33333349f}, {8.83333302f,2.33333349f}}};
    SkDCubic c2 = {{{1,5}, {-5.66666651f,3.33333349f}, {8.83333302f,2.33333349f}, {0,1}}};
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

#include "SkParsePath.h"

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

#include "SkGeometry.h"

static void complex_to_quads(const SkPoint pts[], SkPath* path) {
    SkScalar loopT;
    SkDCubic::CubicType dType;
    if (SkDCubic::ComplexBreak(pts, &loopT, &dType)) {
        SkPoint cubicPair[7]; 
        SkChopCubicAt(pts, cubicPair, loopT);
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
FAILED: d:\cygwin\puregit\tests\pathopsextendedtest.cpp:346	0 */
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

static void (*skipTest)(skiatest::Reporter* , const char* filename) = 0;
static void (*firstTest)(skiatest::Reporter* , const char* filename) = loops63i;
static void (*stopTest)(skiatest::Reporter* , const char* filename) = 0;

#define TEST(name) { name, #name }

static struct TestDesc tests[] = {
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

static void (*firstSubTest)(skiatest::Reporter* , const char* filename) = 0;

static bool runSubTests = false;
static bool runSubTestsFirst = true;
static bool runReverse = false;

DEF_TEST(PathOpsOp, reporter) {
#if DEBUG_SHOW_TEST_NAME
    strncpy(DEBUG_FILENAME_STRING, "", DEBUG_FILENAME_STRING_LENGTH);
#endif
    if (runSubTests && runSubTestsFirst) {
        RunTestSet(reporter, subTests, subTestCount, firstSubTest, NULL, stopTest, runReverse);
    }
    RunTestSet(reporter, tests, testCount, firstTest, skipTest, stopTest, runReverse);
    if (runSubTests && !runSubTestsFirst) {
        RunTestSet(reporter, subTests, subTestCount, firstSubTest, NULL, stopTest, runReverse);
    }
}

static void bufferOverflow(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0,0, 300,170141183460469231731687303715884105728.f);
    SkPath pathB;
    pathB.addRect(0,0, 300,16);
    testPathOp(reporter, path, pathB, kUnion_SkPathOp, filename);
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

    testPathOpCheck(reporter, path1, path2, kIntersect_SkPathOp, filename, FLAGS_runFail);
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

    testPathOpCheck(reporter, path1, path2, kUnion_SkPathOp, filename, FLAGS_runFail);
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
    testPathOpCheck(reporter, path1, path2, (SkPathOp) 2, filename, FLAGS_runFail);
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
    testPathOpCheck(reporter, path1, path2, (SkPathOp) 2, filename, FLAGS_runFail);
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
    testPathOpCheck(reporter, path1, path2, (SkPathOp) 2, filename, FLAGS_runFail);
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
    testPathFailOp(reporter, path1, path2, (SkPathOp) 2, filename);
}

static struct TestDesc failTests[] = {
    TEST(fuzz487a),
    TEST(fuzz433),
    TEST(fuzz1),
    TEST(fuzz714),
    TEST(fuzz487b),
    TEST(fuzz433b),
    TEST(bufferOverflow),
};

static const size_t failTestCount = SK_ARRAY_COUNT(failTests);

DEF_TEST(PathOpsFailOp, reporter) {
#if DEBUG_SHOW_TEST_NAME
    strncpy(DEBUG_FILENAME_STRING, "", DEBUG_FILENAME_STRING_LENGTH);
#endif
    RunTestSet(reporter, failTests, failTestCount, NULL, NULL, NULL, false);
}
