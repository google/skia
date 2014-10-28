/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "PathOpsExtendedTest.h"
#include "PathOpsTestCommon.h"

#define TEST(name) { name, #name }

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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
}

static void testIntersect1(skiatest::Reporter* reporter, const char* filename) {
    SkPath one, two;
    one.addRect(0, 0, 6, 6, SkPath::kCW_Direction);
    two.addRect(3, 3, 9, 9, SkPath::kCW_Direction);
    testPathOp(reporter, one, two, kIntersect_PathOp, filename);
}

static void testUnion1(skiatest::Reporter* reporter, const char* filename) {
    SkPath one, two;
    one.addRect(0, 0, 6, 6, SkPath::kCW_Direction);
    two.addRect(3, 3, 9, 9, SkPath::kCW_Direction);
    testPathOp(reporter, one, two, kUnion_PathOp, filename);
}

static void testDiff1(skiatest::Reporter* reporter, const char* filename) {
    SkPath one, two;
    one.addRect(0, 0, 6, 6, SkPath::kCW_Direction);
    two.addRect(3, 3, 9, 9, SkPath::kCW_Direction);
    testPathOp(reporter, one, two, kDifference_PathOp, filename);
}

static void testXor1(skiatest::Reporter* reporter, const char* filename) {
    SkPath one, two;
    one.addRect(0, 0, 6, 6, SkPath::kCW_Direction);
    two.addRect(3, 3, 9, 9, SkPath::kCW_Direction);
    testPathOp(reporter, one, two, kXOR_PathOp, filename);
}

static void testIntersect2(skiatest::Reporter* reporter, const char* filename) {
    SkPath one, two;
    one.addRect(0, 0, 6, 6, SkPath::kCW_Direction);
    two.addRect(0, 3, 9, 9, SkPath::kCW_Direction);
    testPathOp(reporter, one, two, kIntersect_PathOp, filename);
}

static void testUnion2(skiatest::Reporter* reporter, const char* filename) {
    SkPath one, two;
    one.addRect(0, 0, 6, 6, SkPath::kCW_Direction);
    two.addRect(0, 3, 9, 9, SkPath::kCW_Direction);
    testPathOp(reporter, one, two, kUnion_PathOp, filename);
}

static void testDiff2(skiatest::Reporter* reporter, const char* filename) {
    SkPath one, two;
    one.addRect(0, 0, 6, 6, SkPath::kCW_Direction);
    two.addRect(0, 3, 9, 9, SkPath::kCW_Direction);
    testPathOp(reporter, one, two, kDifference_PathOp, filename);
}

static void testXor2(skiatest::Reporter* reporter, const char* filename) {
    SkPath one, two;
    one.addRect(0, 0, 6, 6, SkPath::kCW_Direction);
    two.addRect(0, 3, 9, 9, SkPath::kCW_Direction);
    testPathOp(reporter, one, two, kXOR_PathOp, filename);
}

static void testOp1d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    path.addRect(0, 0, 2, 2, SkPath::kCW_Direction);
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    pathB.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
}

static void testOp2d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    path.addRect(0, 0, 2, 2, SkPath::kCW_Direction);
    pathB.setFillType(SkPath::kEvenOdd_FillType);
    pathB.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    pathB.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
}

static void testOp3d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    path.addRect(1, 1, 2, 2, SkPath::kCW_Direction);
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    pathB.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
}

static void testOp1u(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    path.addRect(0, 0, 3, 3, SkPath::kCW_Direction);
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    pathB.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    testPathOp(reporter, path, pathB, kUnion_PathOp, filename);
}

static void testOp4d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    path.addRect(2, 2, 4, 4, SkPath::kCW_Direction);
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    pathB.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
}

static void testOp5d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 2, 2, SkPath::kCW_Direction);
    path.addRect(0, 0, 3, 3, SkPath::kCW_Direction);
    pathB.setFillType(SkPath::kEvenOdd_FillType);
    pathB.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    pathB.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
}

static void testOp6d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    path.addRect(0, 0, 3, 3, SkPath::kCW_Direction);
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    pathB.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
}

static void testOp7d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 2, 2, SkPath::kCW_Direction);
    path.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    pathB.setFillType(SkPath::kEvenOdd_FillType);
    pathB.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    pathB.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
}

static void testOp2u(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 2, 2, SkPath::kCW_Direction);
    path.addRect(0, 0, 2, 2, SkPath::kCW_Direction);
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.addRect(0, 0, 3, 3, SkPath::kCW_Direction);
    pathB.addRect(1, 1, 2, 2, SkPath::kCW_Direction);
    testPathOp(reporter, path, pathB, kUnion_PathOp, filename);
}

static void testOp8d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.addRect(0, 0, 640, 480);
    pathB.moveTo(577330, 1971.72f);
    pathB.cubicTo(10.7082f, -116.596f, 262.057f, 45.6468f, 294.694f, 1.96237f);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kUnion_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kUnion_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kXOR_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kUnion_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
}

// this fails to detect a cubic/cubic intersection
// the slight overlap is missed when the cubics are approximated by quadratics
// and the subsequent line/cubic intersection also (correctly) misses the intersection
// if the line/cubic was a matching line/approx.quadratic then the missing intersection
// could have been detected
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kUnion_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kUnion_PathOp, filename);
}

static void cubicOp68u(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.moveTo(0,5);
    path.cubicTo(4,5, 4,1, 5,0);
    path.close();
    pathB.moveTo(1,4);
    pathB.cubicTo(0,5, 5,0, 5,4);
    pathB.close();
    testPathOp(reporter, path, pathB, kUnion_PathOp, filename);
}

static void cubicOp69d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.moveTo(1,3);
    path.cubicTo(0,1, 3,1, 2,0);
    path.close();
    pathB.moveTo(1,3);
    pathB.cubicTo(0,2, 3,1, 1,0);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
}

SkPathOp ops[] = {
    kUnion_PathOp,
    kXOR_PathOp,
    kReverseDifference_PathOp,
    kXOR_PathOp,
    kReverseDifference_PathOp,
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
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kUnion_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kUnion_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
}

// this fails because the pair of nearly coincident cubics intersect at the ends
// but the line connected to one of the cubics at the same point does not intersect
// the other
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
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
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

    testPathOp(reporter, path1, path2, kUnion_PathOp, filename);
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
    testPathOp(reporter, path1, path2, kIntersect_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
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
    testPathOp(reporter, path1, path2, kIntersect_PathOp, filename);
}

static void rectOp1i(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    path.addRect(2, 2, 4, 4, SkPath::kCW_Direction);
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    pathB.addRect(0, 0, 2, 2, SkPath::kCW_Direction);
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
}

static void rectOp2i(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    path.addRect(0, 0, 3, 3, SkPath::kCW_Direction);
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.addRect(0, 0, 2, 2, SkPath::kCW_Direction);
    pathB.addRect(0, 0, 2, 2, SkPath::kCW_Direction);
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kXOR_PathOp, filename);
}

// this fails to generate two interior line segments 
// an earlier pathops succeeded, but still failed to generate one interior line segment
// (but was saved by assemble, which works around a single line missing segment)
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
    testPathOp(reporter, path1, path2, kIntersect_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kUnion_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kUnion_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kUnion_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kUnion_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kUnion_PathOp, filename);
}

static void skpaaalgarve_org53(skiatest::Reporter* reporter, const char* filename) {  //  add t cancel
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
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
}

static void skpabcspark_ca103(skiatest::Reporter* reporter, const char* filename) {  //  add t cancel
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
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
}

static void skpacesoftech_com47(skiatest::Reporter* reporter, const char* filename) {  // partial coincidence
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
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
}

static void skpact_com43(skiatest::Reporter* reporter, const char* filename) {  // bridge op
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
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
}

static void skpadbox_lt8(skiatest::Reporter* reporter, const char* filename) {  // zero span
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
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
}

static void skpadindex_de4(skiatest::Reporter* reporter, const char* filename) {  // find chase op
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
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
}

static void skpadithya_putr4_blogspot_com551(skiatest::Reporter* reporter, const char* filename) { // calc common
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
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
}

static void skpadspert_de11(skiatest::Reporter* reporter, const char* filename) {  // mark and chase winding
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
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
}

static void skpaiaigames_com870(skiatest::Reporter* reporter, const char* filename) {  // cubic/cubic intersect
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
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kUnion_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
}

// this fails because cubic/quad misses an intersection (failure is isolated in c/q int test)
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
    testPathOpCheck(reporter, path, pathB, kIntersect_PathOp, filename, FLAGS_runFail);
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
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kUnion_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kXOR_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kXOR_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kUnion_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
}

static void cubicOp110(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    path.addRect(0, 0, 4, 4, SkPath::kCW_Direction);
    pathB.setFillType(SkPath::kEvenOdd_FillType);
    pathB.addRect(0, 0, 2, 2, SkPath::kCW_Direction);
    pathB.addRect(0, 0, 2, 2, SkPath::kCW_Direction);
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kUnion_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
}

// triggers addSimpleAngle with non-zero argument
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
}

static void cubicOp113(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.moveTo(2,4);
    path.cubicTo(3,5, 2.33333325f,4.33333349f, 3.83333325f,3.83333349f);
    path.close();
    pathB.moveTo(3,5);
    pathB.cubicTo(2.33333325f,4.33333349f, 3.83333325f,3.83333349f, 2,4);
    pathB.close();
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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
    testPathOpCheck(reporter, path, pathB, kIntersect_PathOp, filename, FLAGS_runFail);
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
    testPathOp(reporter, qPath, qPathB, kIntersect_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kIntersect_PathOp, filename);
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

    testPathOp(reporter, path1, path2, kDifference_PathOp, filename);
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
    testPathOp(reporter, path1, path2, kUnion_PathOp, filename);
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
    testPathOp(reporter, path1, path2, kUnion_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kUnion_PathOp, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
}

static void rects3(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    path.addRect(0, 0, 4, 4, SkPath::kCW_Direction);
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.addRect(0, 0, 2, 2, SkPath::kCW_Direction);
    pathB.addRect(0, 0, 2, 2, SkPath::kCW_Direction);
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
}

static void rects4(skiatest::Reporter* reporter, const char* filename) {
    SkPath path, pathB;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    path.addRect(0, 0, 2, 2, SkPath::kCW_Direction);
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.addRect(0, 0, 2, 2, SkPath::kCW_Direction);
    pathB.addRect(0, 0, 3, 3, SkPath::kCW_Direction);
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
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

    testPathOpCheck(reporter, path1, path2, kUnion_PathOp, filename, FLAGS_runFail);
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

    testPathOp(reporter, path1, path2, kUnion_PathOp, filename);
}

static void fuzz763_3084(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0xc19e6455), SkBits2Float(0xc19e6455));
path.quadTo(SkBits2Float(0xc1399153), SkBits2Float(0xc1e00000), SkBits2Float(0x00000000), SkBits2Float(0xc1e00000));
path.quadTo(SkBits2Float(0x41399153), SkBits2Float(0xc1e00000), SkBits2Float(0x419e6455), SkBits2Float(0xc19e6455));
path.quadTo(SkBits2Float(0x41e00000), SkBits2Float(0xc1399153), SkBits2Float(0x41e00000), SkBits2Float(0x00000000));
path.quadTo(SkBits2Float(0x41e00000), SkBits2Float(0x41399153), SkBits2Float(0x419e6455), SkBits2Float(0x419e6455));
path.quadTo(SkBits2Float(0x415b75ce), SkBits2Float(0x41cf0dc3), SkBits2Float(0x40b878fc), SkBits2Float(0x41db9f74));
path.quadTo(SkBits2Float(0x41000000), SkBits2Float(0x41ee1ba4), SkBits2Float(0x41000000), SkBits2Float(0x42040000));
path.quadTo(SkBits2Float(0x41000000), SkBits2Float(0x4211413d), SkBits2Float(0x40b504f3), SkBits2Float(0x421aa09e));
path.quadTo(SkBits2Float(0x405413cd), SkBits2Float(0x42240000), SkBits2Float(0x00000000), SkBits2Float(0x42240000));
path.quadTo(SkBits2Float(0xc05413cd), SkBits2Float(0x42240000), SkBits2Float(0xc0b504f3), SkBits2Float(0x421aa09e));
path.quadTo(SkBits2Float(0xc1000000), SkBits2Float(0x4211413d), SkBits2Float(0xc1000000), SkBits2Float(0x42040000));
path.quadTo(SkBits2Float(0xc1000000), SkBits2Float(0x41ee1ba4), SkBits2Float(0xc0b878fc), SkBits2Float(0x41db9f74));
path.quadTo(SkBits2Float(0xc15b75ce), SkBits2Float(0x41cf0dc3), SkBits2Float(0xc19e6455), SkBits2Float(0x419e6455));
path.quadTo(SkBits2Float(0xc1e00000), SkBits2Float(0x41399153), SkBits2Float(0xc1e00000), SkBits2Float(0x00000000));
path.quadTo(SkBits2Float(0xc1e00000), SkBits2Float(0xc1399153), SkBits2Float(0xc19e6455), SkBits2Float(0xc19e6455));
path.close();
path.moveTo(SkBits2Float(0x421d76c6), SkBits2Float(0x414d1957));
path.quadTo(SkBits2Float(0x4229fd05), SkBits2Float(0x413bbdcc), SkBits2Float(0x4235e9b0), SkBits2Float(0x4152e45d));
path.quadTo(SkBits2Float(0x4241d65c), SkBits2Float(0x416a0aee), SkBits2Float(0x42462d3e), SkBits2Float(0x418e11f4));
path.quadTo(SkBits2Float(0x424a8421), SkBits2Float(0x41a71e71), SkBits2Float(0x4244ba7d), SkBits2Float(0x41bef7c6));
path.quadTo(SkBits2Float(0x423ef0da), SkBits2Float(0x41d6d11e), SkBits2Float(0x42326a9b), SkBits2Float(0x41df7ee4));
path.quadTo(SkBits2Float(0x42273b3e), SkBits2Float(0x41e73f0f), SkBits2Float(0x421c865e), SkBits2Float(0x41ded7e1));
path.quadTo(SkBits2Float(0x42240000), SkBits2Float(0x41f0534a), SkBits2Float(0x42240000), SkBits2Float(0x42040000));
path.quadTo(SkBits2Float(0x42240000), SkBits2Float(0x4211413d), SkBits2Float(0x421aa09e), SkBits2Float(0x421aa09e));
path.quadTo(SkBits2Float(0x4211413d), SkBits2Float(0x42240000), SkBits2Float(0x42040000), SkBits2Float(0x42240000));
path.quadTo(SkBits2Float(0x41ed7d86), SkBits2Float(0x42240000), SkBits2Float(0x41dabec3), SkBits2Float(0x421aa09e));
path.quadTo(SkBits2Float(0x41c80000), SkBits2Float(0x4211413d), SkBits2Float(0x41c80000), SkBits2Float(0x42040000));
path.quadTo(SkBits2Float(0x41c80000), SkBits2Float(0x41ed7d86), SkBits2Float(0x41dabec3), SkBits2Float(0x41dabec3));
path.quadTo(SkBits2Float(0x41ed7d86), SkBits2Float(0x41c80000), SkBits2Float(0x42040000), SkBits2Float(0x41c80000));
path.quadTo(SkBits2Float(0x4209f7d0), SkBits2Float(0x41c80000), SkBits2Float(0x420f2625), SkBits2Float(0x41cbccd7));
path.quadTo(SkBits2Float(0x420ba850), SkBits2Float(0x41c340da), SkBits2Float(0x4209b422), SkBits2Float(0x41b7f99d));
path.quadTo(SkBits2Float(0x42055d40), SkBits2Float(0x419eed20), SkBits2Float(0x420b26e4), SkBits2Float(0x418713c8));
path.quadTo(SkBits2Float(0x4210f088), SkBits2Float(0x415e74e2), SkBits2Float(0x421d76c6), SkBits2Float(0x414d1957));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42240000), SkBits2Float(0x42040000));
path.quadTo(SkBits2Float(0x42240000), SkBits2Float(0x4211413d), SkBits2Float(0x421aa09e), SkBits2Float(0x421aa09e));
path.quadTo(SkBits2Float(0x4211413d), SkBits2Float(0x42240000), SkBits2Float(0x42040000), SkBits2Float(0x42240000));
path.quadTo(SkBits2Float(0x41ed7d86), SkBits2Float(0x42240000), SkBits2Float(0x41dabec3), SkBits2Float(0x421aa09e));
path.quadTo(SkBits2Float(0x41c80000), SkBits2Float(0x4211413d), SkBits2Float(0x41c80000), SkBits2Float(0x42040000));
path.quadTo(SkBits2Float(0x41c80000), SkBits2Float(0x41ed7d86), SkBits2Float(0x41dabec3), SkBits2Float(0x41dabec3));
path.quadTo(SkBits2Float(0x41ed7d86), SkBits2Float(0x41c80000), SkBits2Float(0x42040000), SkBits2Float(0x41c80000));
path.quadTo(SkBits2Float(0x4211413d), SkBits2Float(0x41c80000), SkBits2Float(0x421aa09e), SkBits2Float(0x41dabec3));
path.quadTo(SkBits2Float(0x42240000), SkBits2Float(0x41ed7d86), SkBits2Float(0x42240000), SkBits2Float(0x42040000));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}

static void fuzz763_1823(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0xc19e6455), SkBits2Float(0xc19e6455));
path.quadTo(SkBits2Float(0xc1399153), SkBits2Float(0xc1e00000), SkBits2Float(0x00000000), SkBits2Float(0xc1e00000));
path.quadTo(SkBits2Float(0x41399153), SkBits2Float(0xc1e00000), SkBits2Float(0x419e6455), SkBits2Float(0xc19e6455));
path.quadTo(SkBits2Float(0x41e00000), SkBits2Float(0xc1399153), SkBits2Float(0x41e00000), SkBits2Float(0x00000000));
path.quadTo(SkBits2Float(0x41e00000), SkBits2Float(0x41399153), SkBits2Float(0x419e6455), SkBits2Float(0x419e6455));
path.quadTo(SkBits2Float(0x415b75ce), SkBits2Float(0x41cf0dc3), SkBits2Float(0x40b878fc), SkBits2Float(0x41db9f74));
path.quadTo(SkBits2Float(0x41000000), SkBits2Float(0x41ee1ba4), SkBits2Float(0x41000000), SkBits2Float(0x42040000));
path.quadTo(SkBits2Float(0x41000000), SkBits2Float(0x4211413d), SkBits2Float(0x40b504f3), SkBits2Float(0x421aa09e));
path.quadTo(SkBits2Float(0x405eb92c), SkBits2Float(0x422355aa), SkBits2Float(0x3eee625a), SkBits2Float(0x4223f3e8));
path.quadTo(SkBits2Float(0x3f238500), SkBits2Float(0x4224bba7), SkBits2Float(0x3f4dcc60), SkBits2Float(0x4225921a));
path.quadTo(SkBits2Float(0x4036c5c0), SkBits2Float(0x422ffa87), SkBits2Float(0x401de138), SkBits2Float(0x423d244e));
path.quadTo(SkBits2Float(0x4004fcb0), SkBits2Float(0x424a4e17), SkBits2Float(0xbf0628a0), SkBits2Float(0x42528342));
path.quadTo(SkBits2Float(0xc04810f8), SkBits2Float(0x425ab86c), SkBits2Float(0xc0cd56bc), SkBits2Float(0x42592a22));
path.quadTo(SkBits2Float(0xc11b5280), SkBits2Float(0x42579bda), SkBits2Float(0xc13c272a), SkBits2Float(0x424d336e));
path.quadTo(SkBits2Float(0xc15cfbd4), SkBits2Float(0x4242cb00), SkBits2Float(0xc156c2ae), SkBits2Float(0x4235a138));
path.quadTo(SkBits2Float(0xc150898c), SkBits2Float(0x42287770), SkBits2Float(0xc126e7d8), SkBits2Float(0x42204246));
path.quadTo(SkBits2Float(0xc1066ae4), SkBits2Float(0x4219da96), SkBits2Float(0xc0be6f82), SkBits2Float(0x42196502));
path.quadTo(SkBits2Float(0xc1000000), SkBits2Float(0x42106507), SkBits2Float(0xc1000000), SkBits2Float(0x42040000));
path.quadTo(SkBits2Float(0xc1000000), SkBits2Float(0x41ee1ba4), SkBits2Float(0xc0b878fc), SkBits2Float(0x41db9f74));
path.quadTo(SkBits2Float(0xc15b75ce), SkBits2Float(0x41cf0dc3), SkBits2Float(0xc19e6455), SkBits2Float(0x419e6455));
path.quadTo(SkBits2Float(0xc1e00000), SkBits2Float(0x41399153), SkBits2Float(0xc1e00000), SkBits2Float(0x00000000));
path.quadTo(SkBits2Float(0xc1e00000), SkBits2Float(0xc1399153), SkBits2Float(0xc19e6455), SkBits2Float(0xc19e6455));
path.close();
path.moveTo(SkBits2Float(0x41dabec3), SkBits2Float(0x41dabec3));
path.quadTo(SkBits2Float(0x41ed7d86), SkBits2Float(0x41c80000), SkBits2Float(0x42040000), SkBits2Float(0x41c80000));
path.quadTo(SkBits2Float(0x4211413d), SkBits2Float(0x41c80000), SkBits2Float(0x421aa09e), SkBits2Float(0x41dabec3));
path.quadTo(SkBits2Float(0x42240000), SkBits2Float(0x41ed7d86), SkBits2Float(0x42240000), SkBits2Float(0x42040000));
path.quadTo(SkBits2Float(0x42240000), SkBits2Float(0x4211413d), SkBits2Float(0x421aa09e), SkBits2Float(0x421aa09e));
path.quadTo(SkBits2Float(0x4211413d), SkBits2Float(0x42240000), SkBits2Float(0x42040000), SkBits2Float(0x42240000));
path.quadTo(SkBits2Float(0x41ed7d86), SkBits2Float(0x42240000), SkBits2Float(0x41dabec3), SkBits2Float(0x421aa09e));
path.quadTo(SkBits2Float(0x41c80000), SkBits2Float(0x4211413d), SkBits2Float(0x41c80000), SkBits2Float(0x42040000));
path.quadTo(SkBits2Float(0x41c80000), SkBits2Float(0x41ed7d86), SkBits2Float(0x41dabec3), SkBits2Float(0x41dabec3));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x41000000), SkBits2Float(0x42040000));
path.quadTo(SkBits2Float(0x41000000), SkBits2Float(0x4211413d), SkBits2Float(0x40b504f3), SkBits2Float(0x421aa09e));
path.quadTo(SkBits2Float(0x405413cd), SkBits2Float(0x42240000), SkBits2Float(0x00000000), SkBits2Float(0x42240000));
path.quadTo(SkBits2Float(0xc05413cd), SkBits2Float(0x42240000), SkBits2Float(0xc0b504f3), SkBits2Float(0x421aa09e));
path.quadTo(SkBits2Float(0xc1000000), SkBits2Float(0x4211413d), SkBits2Float(0xc1000000), SkBits2Float(0x42040000));
path.quadTo(SkBits2Float(0xc1000000), SkBits2Float(0x41ed7d86), SkBits2Float(0xc0b504f3), SkBits2Float(0x41dabec3));
path.quadTo(SkBits2Float(0xc05413cd), SkBits2Float(0x41c80000), SkBits2Float(0x00000000), SkBits2Float(0x41c80000));
path.quadTo(SkBits2Float(0x405413cd), SkBits2Float(0x41c80000), SkBits2Float(0x40b504f3), SkBits2Float(0x41dabec3));
path.quadTo(SkBits2Float(0x41000000), SkBits2Float(0x41ed7d86), SkBits2Float(0x41000000), SkBits2Float(0x42040000));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}

static void fuzz763_378(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x41013776), SkBits2Float(0xc25007a8));
path.quadTo(SkBits2Float(0x412f219e), SkBits2Float(0xc256a86f), SkBits2Float(0x41625842), SkBits2Float(0xc2533a60));
path.quadTo(SkBits2Float(0x418ac776), SkBits2Float(0xc24fcc52), SkBits2Float(0x41980904), SkBits2Float(0xc24451c8));
path.quadTo(SkBits2Float(0x41a54a92), SkBits2Float(0xc238d73e), SkBits2Float(0x419e6e72), SkBits2Float(0xc22c0994));
path.quadTo(SkBits2Float(0x41979256), SkBits2Float(0xc21f3bea), SkBits2Float(0x41809d42), SkBits2Float(0xc2189b23));
path.quadTo(SkBits2Float(0x4153505c), SkBits2Float(0xc211fa5c), SkBits2Float(0x412019b5), SkBits2Float(0xc215686b));
path.quadTo(SkBits2Float(0x40d9c61e), SkBits2Float(0xc218d67a), SkBits2Float(0x40a4bfe8), SkBits2Float(0xc2245104));
path.quadTo(SkBits2Float(0x405f7360), SkBits2Float(0xc22fcb8e), SkBits2Float(0x408b2a24), SkBits2Float(0xc23c9937));
path.quadTo(SkBits2Float(0x40a69a9c), SkBits2Float(0xc24966e1), SkBits2Float(0x41013776), SkBits2Float(0xc25007a8));
path.close();
path.moveTo(SkBits2Float(0xc21aa3d0), SkBits2Float(0xc21a9d6c));
path.quadTo(SkBits2Float(0xc21144a0), SkBits2Float(0xc223fd00), SkBits2Float(0xc2040363), SkBits2Float(0xc223fd46));
path.quadTo(SkBits2Float(0xc1ed844d), SkBits2Float(0xc223fd8c), SkBits2Float(0xc1dac526), SkBits2Float(0xc21a9e5c));
path.quadTo(SkBits2Float(0xc1c80600), SkBits2Float(0xc2113f2c), SkBits2Float(0xc1c80574), SkBits2Float(0xc203fdef));
path.quadTo(SkBits2Float(0xc1c804e8), SkBits2Float(0xc1ed7964), SkBits2Float(0xc1dac348), SkBits2Float(0xc1daba3e));
path.quadTo(SkBits2Float(0xc1ed81a8), SkBits2Float(0xc1c7fb18), SkBits2Float(0xc2040211), SkBits2Float(0xc1c7fa8c));
path.quadTo(SkBits2Float(0xc211434e), SkBits2Float(0xc1c7fa00), SkBits2Float(0xc21aa2e0), SkBits2Float(0xc1dab860));
path.quadTo(SkBits2Float(0xc2240274), SkBits2Float(0xc1ed76bf), SkBits2Float(0xc22402ba), SkBits2Float(0xc203fc9d));
path.quadTo(SkBits2Float(0xc2240300), SkBits2Float(0xc2113dda), SkBits2Float(0xc21aa3d0), SkBits2Float(0xc21a9d6c));
path.close();
path.moveTo(SkBits2Float(0xc19e6455), SkBits2Float(0xc19e6455));
path.quadTo(SkBits2Float(0xc1399153), SkBits2Float(0xc1e00000), SkBits2Float(0x00000000), SkBits2Float(0xc1e00000));
path.quadTo(SkBits2Float(0x41399153), SkBits2Float(0xc1e00000), SkBits2Float(0x419e6455), SkBits2Float(0xc19e6455));
path.quadTo(SkBits2Float(0x41e00000), SkBits2Float(0xc1399153), SkBits2Float(0x41e00000), SkBits2Float(0x00000000));
path.quadTo(SkBits2Float(0x41e00000), SkBits2Float(0x41399153), SkBits2Float(0x419e6455), SkBits2Float(0x419e6455));
path.quadTo(SkBits2Float(0x415b75ce), SkBits2Float(0x41cf0dc3), SkBits2Float(0x40b878fc), SkBits2Float(0x41db9f74));
path.quadTo(SkBits2Float(0x41000000), SkBits2Float(0x41ee1ba4), SkBits2Float(0x41000000), SkBits2Float(0x42040000));
path.quadTo(SkBits2Float(0x41000000), SkBits2Float(0x4211413d), SkBits2Float(0x40b504f3), SkBits2Float(0x421aa09e));
path.quadTo(SkBits2Float(0x405413cd), SkBits2Float(0x42240000), SkBits2Float(0x00000000), SkBits2Float(0x42240000));
path.quadTo(SkBits2Float(0xc05413cd), SkBits2Float(0x42240000), SkBits2Float(0xc0b504f3), SkBits2Float(0x421aa09e));
path.quadTo(SkBits2Float(0xc1000000), SkBits2Float(0x4211413d), SkBits2Float(0xc1000000), SkBits2Float(0x42040000));
path.quadTo(SkBits2Float(0xc1000000), SkBits2Float(0x41ee1ba4), SkBits2Float(0xc0b878fc), SkBits2Float(0x41db9f74));
path.quadTo(SkBits2Float(0xc15b75ce), SkBits2Float(0x41cf0dc3), SkBits2Float(0xc19e6455), SkBits2Float(0x419e6455));
path.quadTo(SkBits2Float(0xc1e00000), SkBits2Float(0x41399153), SkBits2Float(0xc1e00000), SkBits2Float(0x00000000));
path.quadTo(SkBits2Float(0xc1e00000), SkBits2Float(0xc1399153), SkBits2Float(0xc19e6455), SkBits2Float(0xc19e6455));
path.close();
path.moveTo(SkBits2Float(0xc23c98fa), SkBits2Float(0x408b3eec));
path.quadTo(SkBits2Float(0xc22fcb5c), SkBits2Float(0x405f9a18), SkBits2Float(0xc22450bb), SkBits2Float(0x40a4d200));
path.quadTo(SkBits2Float(0xc218d61a), SkBits2Float(0x40d9d6f4), SkBits2Float(0xc21567dd), SkBits2Float(0x412021ef));
path.quadTo(SkBits2Float(0xc211f9a2), SkBits2Float(0x41535866), SkBits2Float(0xc2189a40), SkBits2Float(0x4180a176));
path.quadTo(SkBits2Float(0xc21f3ade), SkBits2Float(0x419796b9), SkBits2Float(0xc22c087c), SkBits2Float(0x419e7330));
path.quadTo(SkBits2Float(0xc238d61a), SkBits2Float(0x41a54fa9), SkBits2Float(0xc24450bb), SkBits2Float(0x41980e6c));
path.quadTo(SkBits2Float(0xc24fcaa2), SkBits2Float(0x418ace05), SkBits2Float(0xc2533929), SkBits2Float(0x41626a5d));
path.lineTo(SkBits2Float(0xc25338d1), SkBits2Float(0x41626f8c));
path.quadTo(SkBits2Float(0xc24fca68), SkBits2Float(0x418ad2e8), SkBits2Float(0xc2444fb0), SkBits2Float(0x419813d4));
path.quadTo(SkBits2Float(0xc238d4f6), SkBits2Float(0x41a554c0), SkBits2Float(0xc22c0765), SkBits2Float(0x419e77ee));
path.quadTo(SkBits2Float(0xc21f39d4), SkBits2Float(0x41979b1c), SkBits2Float(0xc218995e), SkBits2Float(0x4180a5aa));
path.quadTo(SkBits2Float(0xc211f8e8), SkBits2Float(0x41536071), SkBits2Float(0xc2156751), SkBits2Float(0x41202a2d));
path.quadTo(SkBits2Float(0xc2156774), SkBits2Float(0x41202819), SkBits2Float(0xc2156798), SkBits2Float(0x41202604));
path.quadTo(SkBits2Float(0xc2156d3e), SkBits2Float(0x411fd1b7), SkBits2Float(0xc2157321), SkBits2Float(0x411f7b6e));
path.quadTo(SkBits2Float(0xc218e910), SkBits2Float(0x40d986da), SkBits2Float(0xc2245097), SkBits2Float(0x40a4daf8));
path.quadTo(SkBits2Float(0xc22fcb44), SkBits2Float(0x405fad48), SkBits2Float(0xc23c98dc), SkBits2Float(0x408b493c));
path.quadTo(SkBits2Float(0xc2496673), SkBits2Float(0x40a6bbcc), SkBits2Float(0xc25006fe), SkBits2Float(0x4101489a));
path.quadTo(SkBits2Float(0xc256a729), SkBits2Float(0x412f30b9), SkBits2Float(0xc2533998), SkBits2Float(0x416263e8));
path.quadTo(SkBits2Float(0xc256a7d4), SkBits2Float(0x412f2d72), SkBits2Float(0xc2500736), SkBits2Float(0x410142ec));
path.quadTo(SkBits2Float(0xc2496698), SkBits2Float(0x40a6b0cc), SkBits2Float(0xc23c98fa), SkBits2Float(0x408b3eec));
path.close();
path.moveTo(SkBits2Float(0xc2533937), SkBits2Float(0x41626995));
path.quadTo(SkBits2Float(0xc2533968), SkBits2Float(0x416266bf), SkBits2Float(0xc2533998), SkBits2Float(0x416263e8));
path.lineTo(SkBits2Float(0x41dac1c6), SkBits2Float(0x41dabbc0));
path.quadTo(SkBits2Float(0x41dac044), SkBits2Float(0x41dabd41), SkBits2Float(0x41dabec3), SkBits2Float(0x41dabec3));
path.quadTo(SkBits2Float(0x41c80000), SkBits2Float(0x41ed7d86), SkBits2Float(0x41c80000), SkBits2Float(0x42040000));
path.quadTo(SkBits2Float(0x41c80000), SkBits2Float(0x4211413d), SkBits2Float(0x41dabec3), SkBits2Float(0x421aa09e));
path.quadTo(SkBits2Float(0x41ed7d86), SkBits2Float(0x42240000), SkBits2Float(0x42040000), SkBits2Float(0x42240000));
path.quadTo(SkBits2Float(0x4211413d), SkBits2Float(0x42240000), SkBits2Float(0x421aa09e), SkBits2Float(0x421aa09e));
path.quadTo(SkBits2Float(0x421aa1af), SkBits2Float(0x421a9f8e), SkBits2Float(0x421aa2bf), SkBits2Float(0x421a9e7d));
path.quadTo(SkBits2Float(0x42240200), SkBits2Float(0x42113efb), SkBits2Float(0x422401d1), SkBits2Float(0x4203fdbe));
path.quadTo(SkBits2Float(0x422401a3), SkBits2Float(0x41ed7902), SkBits2Float(0x421aa220), SkBits2Float(0x41daba81));
path.quadTo(SkBits2Float(0x4211429e), SkBits2Float(0x41c7fc00), SkBits2Float(0x42040161), SkBits2Float(0x41c7fc5d));
path.quadTo(SkBits2Float(0x41ed8047), SkBits2Float(0x41c7fcbb), SkBits2Float(0x41dac1c6), SkBits2Float(0x41dabbc0));
path.lineTo(SkBits2Float(0xc2533937), SkBits2Float(0x41626995));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0xc2444fb0), SkBits2Float(0x419813d4));
path.quadTo(SkBits2Float(0xc24fca68), SkBits2Float(0x418ad2e8), SkBits2Float(0xc25338d1), SkBits2Float(0x41626f8c));
path.quadTo(SkBits2Float(0xc256a73a), SkBits2Float(0x412f3944), SkBits2Float(0xc25006c4), SkBits2Float(0x41014e62));
path.quadTo(SkBits2Float(0xc249664e), SkBits2Float(0x40a6c6fc), SkBits2Float(0xc23c98bd), SkBits2Float(0x408b53b8));
path.quadTo(SkBits2Float(0xc22fcb2b), SkBits2Float(0x405fc0d8), SkBits2Float(0xc2245073), SkBits2Float(0x40a4e41c));
path.quadTo(SkBits2Float(0xc218d5ba), SkBits2Float(0x40d9e7cc), SkBits2Float(0xc2156751), SkBits2Float(0x41202a2d));
path.quadTo(SkBits2Float(0xc211f8e8), SkBits2Float(0x41536071), SkBits2Float(0xc218995e), SkBits2Float(0x4180a5aa));
path.quadTo(SkBits2Float(0xc21f39d4), SkBits2Float(0x41979b1c), SkBits2Float(0xc22c0765), SkBits2Float(0x419e77ee));
path.quadTo(SkBits2Float(0xc238d4f6), SkBits2Float(0x41a554c0), SkBits2Float(0xc2444fb0), SkBits2Float(0x419813d4));
path.close();
    SkPath path2(path);
    testPathOpCheck(reporter, path1, path2, (SkPathOp) 2, filename, FLAGS_runFail);
}

static void fuzz763_378b(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(-47.1494f, 4.35143f);
path.quadTo(-39.8075f, 18.9486f, -43.0083f, 19.8062f);
path.quadTo(-50.35f, 5.21042f, -52.0068f, 8.08022f);
path.quadTo(-53.6632f, 10.9494f, -52.8062f, 14.1494f);
path.quadTo(-53.6639f, 10.9486f, -52.007f, 8.07884f);
path.quadTo(-50.3502f, 5.20908f, -47.1494f, 4.35143f);
path.close();
    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0xc2444fb0), SkBits2Float(0x419813d4));
path.quadTo(SkBits2Float(0xc24fca68), SkBits2Float(0x418ad2e8), SkBits2Float(0xc25338d1), SkBits2Float(0x41626f8c));
path.quadTo(SkBits2Float(0xc256a73a), SkBits2Float(0x412f3944), SkBits2Float(0xc25006c4), SkBits2Float(0x41014e62));
path.quadTo(SkBits2Float(0xc21f39d4), SkBits2Float(0x41979b1c), SkBits2Float(0xc22c0765), SkBits2Float(0x419e77ee));
path.quadTo(SkBits2Float(0xc238d4f6), SkBits2Float(0x41a554c0), SkBits2Float(0xc2444fb0), SkBits2Float(0x419813d4));
path.close();
    SkPath path2(path);
    testPathOpCheck(reporter, path1, path2, (SkPathOp) 2, filename, FLAGS_runFail);
}

static void fuzz763_378c(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
    path.moveTo(-47.1494f, 4.35143f);
    path.quadTo(-46.208f, 20.6664f, -43.0072f, 19.8086f);
    path.quadTo(-39.8065f, 18.9507f, -38.1498f, 16.0809f);
    path.quadTo(-36.4931f, 13.211f, -37.3509f, 10.0103f);
    path.quadTo(-37.351f, 10.0098f, -37.3512f, 10.0093f);
    path.close();
    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
    path.moveTo(-49.0778f, 19.0097f);
    path.quadTo(-38.2087f, 6.80955f, -37.3509f, 10.0103f);
    path.quadTo(-36.4931f, 13.211f, -38.1498f, 16.0809f);
    path.quadTo(-39.8065f, 18.9507f, -43.0072f, 19.8086f);
    path.close();
    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}

static void fuzz763_378d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(-47.1494f, 4.35143f);
path.quadTo(-38.2091f, 6.80749f, -37.3514f, 10.0083f);  // required
path.quadTo(-36.4938f, 13.2091f, -38.1506f, 16.0788f);  // required
path.close();
    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(-49.0778f, 19.0097f);
path.quadTo(-38.2087f, 6.80955f, -37.3509f, 10.0103f);
path.quadTo(-36.4931f, 13.211f, -38.1498f, 16.0809f);
path.close();
    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}

static void fuzz763_558(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x41c95d06), SkBits2Float(0xc238e312));
path.quadTo(SkBits2Float(0x41e37302), SkBits2Float(0xc23b3f66), SkBits2Float(0x41f93bb2), SkBits2Float(0xc233b1b3));
path.quadTo(SkBits2Float(0x42025d9e), SkBits2Float(0xc22fb50a), SkBits2Float(0x4205bcea), SkBits2Float(0xc22a30db));
path.quadTo(SkBits2Float(0x420be531), SkBits2Float(0xc22837fe), SkBits2Float(0x421120f1), SkBits2Float(0xc2239353));
path.quadTo(SkBits2Float(0x421b0b2d), SkBits2Float(0xc21ac757), SkBits2Float(0x421bd594), SkBits2Float(0xc20d8c25));
path.quadTo(SkBits2Float(0x421c9ffc), SkBits2Float(0xc20050f2), SkBits2Float(0x4213d3fe), SkBits2Float(0xc1eccd6f));
path.quadTo(SkBits2Float(0x420b0802), SkBits2Float(0xc1d8f8fa), SkBits2Float(0x41fb99a0), SkBits2Float(0xc1d7642b));
path.quadTo(SkBits2Float(0x41e1233b), SkBits2Float(0xc1d5cf5c), SkBits2Float(0x41cd4ec5), SkBits2Float(0xc1e76755));
path.quadTo(SkBits2Float(0x41c5ef3d), SkBits2Float(0xc1edf201), SkBits2Float(0x41c11591), SkBits2Float(0xc1f5b68f));
path.quadTo(SkBits2Float(0x41b863c9), SkBits2Float(0xc1f896c5), SkBits2Float(0x41b04a41), SkBits2Float(0xc1fe34bf));
path.quadTo(SkBits2Float(0x419a8190), SkBits2Float(0xc206a812), SkBits2Float(0x4195c8e8), SkBits2Float(0xc213b310));
path.quadTo(SkBits2Float(0x41911040), SkBits2Float(0xc220be0e), SkBits2Float(0x41a02ba6), SkBits2Float(0xc22ba266));
path.quadTo(SkBits2Float(0x41af470a), SkBits2Float(0xc23686bf), SkBits2Float(0x41c95d06), SkBits2Float(0xc238e312));
path.close();
path.moveTo(SkBits2Float(0xc2169738), SkBits2Float(0xc2131d1b));
path.quadTo(SkBits2Float(0xc2096e21), SkBits2Float(0xc214b131), SkBits2Float(0xc1fe042e), SkBits2Float(0xc20c809d));
path.quadTo(SkBits2Float(0xc1e92c1a), SkBits2Float(0xc204500a), SkBits2Float(0xc1e603ef), SkBits2Float(0xc1ee4de5));
path.quadTo(SkBits2Float(0xc1e2dbc3), SkBits2Float(0xc1d3fbb6), SkBits2Float(0xc1f33ce9), SkBits2Float(0xc1bf23a3));
path.quadTo(SkBits2Float(0xc201cf08), SkBits2Float(0xc1aa4b8f), SkBits2Float(0xc20ef820), SkBits2Float(0xc1a72363));
path.quadTo(SkBits2Float(0xc21c2138), SkBits2Float(0xc1a3fb38), SkBits2Float(0xc2268d41), SkBits2Float(0xc1b45c5e));
path.quadTo(SkBits2Float(0xc230f94b), SkBits2Float(0xc1c4bd85), SkBits2Float(0xc2328d61), SkBits2Float(0xc1df0fb4));
path.quadTo(SkBits2Float(0xc2342177), SkBits2Float(0xc1f961e4), SkBits2Float(0xc22bf0e3), SkBits2Float(0xc2071cfb));
path.quadTo(SkBits2Float(0xc223c050), SkBits2Float(0xc2118905), SkBits2Float(0xc2169738), SkBits2Float(0xc2131d1b));
path.close();
path.moveTo(SkBits2Float(0xc19e6455), SkBits2Float(0xc19e6455));
path.quadTo(SkBits2Float(0xc1399153), SkBits2Float(0xc1e00000), SkBits2Float(0x00000000), SkBits2Float(0xc1e00000));
path.quadTo(SkBits2Float(0x41399153), SkBits2Float(0xc1e00000), SkBits2Float(0x419e6455), SkBits2Float(0xc19e6455));
path.quadTo(SkBits2Float(0x41e00000), SkBits2Float(0xc1399153), SkBits2Float(0x41e00000), SkBits2Float(0x00000000));
path.quadTo(SkBits2Float(0x41e00000), SkBits2Float(0x41399153), SkBits2Float(0x419e6455), SkBits2Float(0x419e6455));
path.quadTo(SkBits2Float(0x415b75ce), SkBits2Float(0x41cf0dc3), SkBits2Float(0x40b878fc), SkBits2Float(0x41db9f74));
path.quadTo(SkBits2Float(0x41000000), SkBits2Float(0x41ee1ba4), SkBits2Float(0x41000000), SkBits2Float(0x42040000));
path.quadTo(SkBits2Float(0x41000000), SkBits2Float(0x4211413d), SkBits2Float(0x40b504f3), SkBits2Float(0x421aa09e));
path.quadTo(SkBits2Float(0x405413cd), SkBits2Float(0x42240000), SkBits2Float(0x00000000), SkBits2Float(0x42240000));
path.quadTo(SkBits2Float(0xbe8f799b), SkBits2Float(0x42240000), SkBits2Float(0xbf0db675), SkBits2Float(0x4223eed6));
path.quadTo(SkBits2Float(0xc060c2a3), SkBits2Float(0x42233513), SkBits2Float(0xc0b504f3), SkBits2Float(0x421aa09e));
path.lineTo(SkBits2Float(0xc0c24f68), SkBits2Float(0x4218d9ff));
path.quadTo(SkBits2Float(0xc1000000), SkBits2Float(0x421005d8), SkBits2Float(0xc1000000), SkBits2Float(0x42040000));
path.quadTo(SkBits2Float(0xc1000000), SkBits2Float(0x41ee1ba4), SkBits2Float(0xc0b878fc), SkBits2Float(0x41db9f74));
path.quadTo(SkBits2Float(0xc15b75ce), SkBits2Float(0x41cf0dc3), SkBits2Float(0xc19e6455), SkBits2Float(0x419e6455));
path.quadTo(SkBits2Float(0xc1e00000), SkBits2Float(0x41399153), SkBits2Float(0xc1e00000), SkBits2Float(0x00000000));
path.quadTo(SkBits2Float(0xc1e00000), SkBits2Float(0xc1399153), SkBits2Float(0xc19e6455), SkBits2Float(0xc19e6455));
path.close();
path.moveTo(SkBits2Float(0x42215fd9), SkBits2Float(0xc1c64bb4));
path.quadTo(SkBits2Float(0x422dad8e), SkBits2Float(0xc1d0284f), SkBits2Float(0x4239dd52), SkBits2Float(0xc1c5bb1d));
path.quadTo(SkBits2Float(0x42460d14), SkBits2Float(0xc1bb4dea), SkBits2Float(0x424afb61), SkBits2Float(0xc1a2b282));
path.quadTo(SkBits2Float(0x424fe9af), SkBits2Float(0xc18a1717), SkBits2Float(0x424ab316), SkBits2Float(0xc1636f22));
path.quadTo(SkBits2Float(0x42457c7c), SkBits2Float(0xc132b016), SkBits2Float(0x42392ec8), SkBits2Float(0xc11ef6e3));
path.quadTo(SkBits2Float(0x422ce113), SkBits2Float(0xc10b3dad), SkBits2Float(0x4220b150), SkBits2Float(0xc1201812));
path.quadTo(SkBits2Float(0x4214818d), SkBits2Float(0xc134f276), SkBits2Float(0x420f9340), SkBits2Float(0xc1662949));
path.quadTo(SkBits2Float(0x420aa4f2), SkBits2Float(0xc18bb00e), SkBits2Float(0x420fdb8c), SkBits2Float(0xc1a40f94));
path.quadTo(SkBits2Float(0x42151225), SkBits2Float(0xc1bc6f1a), SkBits2Float(0x42215fd9), SkBits2Float(0xc1c64bb4));
path.close();
path.moveTo(SkBits2Float(0xc23c98fa), SkBits2Float(0x408b3eec));
path.quadTo(SkBits2Float(0xc22fcb5c), SkBits2Float(0x405f9a18), SkBits2Float(0xc22450bb), SkBits2Float(0x40a4d200));
path.quadTo(SkBits2Float(0xc218d61a), SkBits2Float(0x40d9d6f4), SkBits2Float(0xc21567dd), SkBits2Float(0x412021ef));
path.quadTo(SkBits2Float(0xc211f9a2), SkBits2Float(0x41535866), SkBits2Float(0xc2189a40), SkBits2Float(0x4180a176));
path.quadTo(SkBits2Float(0xc21f3ade), SkBits2Float(0x419796b9), SkBits2Float(0xc22c087c), SkBits2Float(0x419e7330));
path.quadTo(SkBits2Float(0xc238d61a), SkBits2Float(0x41a54fa9), SkBits2Float(0xc24450bb), SkBits2Float(0x41980e6c));
path.quadTo(SkBits2Float(0xc24fcb5c), SkBits2Float(0x418acd2f), SkBits2Float(0xc2533998), SkBits2Float(0x416263e8));
path.quadTo(SkBits2Float(0xc256a7d4), SkBits2Float(0x412f2d72), SkBits2Float(0xc2500736), SkBits2Float(0x410142ec));
path.quadTo(SkBits2Float(0xc2496698), SkBits2Float(0x40a6b0cc), SkBits2Float(0xc23c98fa), SkBits2Float(0x408b3eec));
path.close();
path.moveTo(SkBits2Float(0x41dabec3), SkBits2Float(0x41dabec3));
path.quadTo(SkBits2Float(0x41ed7d86), SkBits2Float(0x41c80000), SkBits2Float(0x42040000), SkBits2Float(0x41c80000));
path.quadTo(SkBits2Float(0x4211413d), SkBits2Float(0x41c80000), SkBits2Float(0x421aa09e), SkBits2Float(0x41dabec3));
path.quadTo(SkBits2Float(0x42240000), SkBits2Float(0x41ed7d86), SkBits2Float(0x42240000), SkBits2Float(0x42040000));
path.quadTo(SkBits2Float(0x42240000), SkBits2Float(0x4211413d), SkBits2Float(0x421aa09e), SkBits2Float(0x421aa09e));
path.quadTo(SkBits2Float(0x4211413d), SkBits2Float(0x42240000), SkBits2Float(0x42040000), SkBits2Float(0x42240000));
path.quadTo(SkBits2Float(0x41ed7d86), SkBits2Float(0x42240000), SkBits2Float(0x41dabec3), SkBits2Float(0x421aa09e));
path.quadTo(SkBits2Float(0x41c80000), SkBits2Float(0x4211413d), SkBits2Float(0x41c80000), SkBits2Float(0x42040000));
path.quadTo(SkBits2Float(0x41c80000), SkBits2Float(0x41ed7d86), SkBits2Float(0x41dabec3), SkBits2Float(0x41dabec3));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0xbfe9fe20), SkBits2Float(0x42526568));
path.quadTo(SkBits2Float(0xc08f5cf4), SkBits2Float(0x425a57e7), SkBits2Float(0xc0f853f0), SkBits2Float(0x4258763b));
path.quadTo(SkBits2Float(0xc130a57c), SkBits2Float(0x42569490), SkBits2Float(0xc1506f76), SkBits2Float(0x424bf8e3));
path.quadTo(SkBits2Float(0xc1703970), SkBits2Float(0x42415d36), SkBits2Float(0xc168b2c0), SkBits2Float(0x42343e56));
path.quadTo(SkBits2Float(0xc1612c17), SkBits2Float(0x42271f76), SkBits2Float(0xc136bd61), SkBits2Float(0x421f2cf7));
path.quadTo(SkBits2Float(0xc10c4ead), SkBits2Float(0x42173a78), SkBits2Float(0xc0afa654), SkBits2Float(0x42191c24));
path.quadTo(SkBits2Float(0xc00d5ea8), SkBits2Float(0x421afdcf), SkBits2Float(0xbe636c00), SkBits2Float(0x4225997c));
path.quadTo(SkBits2Float(0x3fe1e250), SkBits2Float(0x42303529), SkBits2Float(0x3fa5acf0), SkBits2Float(0x423d5409));
path.quadTo(SkBits2Float(0x3f52ef00), SkBits2Float(0x424a72ea), SkBits2Float(0xbfe9fe20), SkBits2Float(0x42526568));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}

static void fuzz763_378a(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x40f4c1fc), SkBits2Float(0xc25049c6));
path.quadTo(SkBits2Float(0x41281306), SkBits2Float(0xc25702a0), SkBits2Float(0x415b6610), SkBits2Float(0xc253af82));
path.quadTo(SkBits2Float(0x41875c90), SkBits2Float(0xc2505c66), SkBits2Float(0x4194ce44), SkBits2Float(0xc244efe4));
path.quadTo(SkBits2Float(0x41a23ff8), SkBits2Float(0xc2398363), SkBits2Float(0x419b99bc), SkBits2Float(0xc22caea0));
path.quadTo(SkBits2Float(0x4194f385), SkBits2Float(0xc21fd9dc), SkBits2Float(0x417c3502), SkBits2Float(0xc2192102));
path.quadTo(SkBits2Float(0x414e82fc), SkBits2Float(0xc2126828), SkBits2Float(0x411b2fef), SkBits2Float(0xc215bb45));
path.quadTo(SkBits2Float(0x40cfb9c4), SkBits2Float(0xc2190e62), SkBits2Float(0x4099f2f4), SkBits2Float(0xc2247ae4));
path.quadTo(SkBits2Float(0x40485848), SkBits2Float(0xc22fe766), SkBits2Float(0x407d8a18), SkBits2Float(0xc23cbc28));
path.quadTo(SkBits2Float(0x40995df0), SkBits2Float(0xc24990ec), SkBits2Float(0x40f4c1fc), SkBits2Float(0xc25049c6));
path.close();
path.moveTo(SkBits2Float(0xc20605f2), SkBits2Float(0xc22259cd));
path.quadTo(SkBits2Float(0xc1f189ee), SkBits2Float(0xc22283df), SkBits2Float(0xc1de900b), SkBits2Float(0xc219426c));
path.quadTo(SkBits2Float(0xc1cb9626), SkBits2Float(0xc21000fa), SkBits2Float(0xc1cb4202), SkBits2Float(0xc202c000));
path.quadTo(SkBits2Float(0xc1caeddd), SkBits2Float(0xc1eafe0b), SkBits2Float(0xc1dd70c3), SkBits2Float(0xc1d80427));
path.quadTo(SkBits2Float(0xc1eff3a7), SkBits2Float(0xc1c50a43), SkBits2Float(0xc2053ace), SkBits2Float(0xc1c4b61e));
path.quadTo(SkBits2Float(0xc2127bc8), SkBits2Float(0xc1c461fa), SkBits2Float(0xc21bf8ba), SkBits2Float(0xc1d6e4df));
path.quadTo(SkBits2Float(0xc22575ad), SkBits2Float(0xc1e967c4), SkBits2Float(0xc2259fbf), SkBits2Float(0xc201f4dc));
path.quadTo(SkBits2Float(0xc225c9d1), SkBits2Float(0xc20f35d6), SkBits2Float(0xc21c885e), SkBits2Float(0xc218b2c8));
path.quadTo(SkBits2Float(0xc21346ec), SkBits2Float(0xc2222fbb), SkBits2Float(0xc20605f2), SkBits2Float(0xc22259cd));
path.close();
path.moveTo(SkBits2Float(0xc19e6455), SkBits2Float(0xc19e6455));
path.quadTo(SkBits2Float(0xc1399153), SkBits2Float(0xc1e00000), SkBits2Float(0x00000000), SkBits2Float(0xc1e00000));
path.quadTo(SkBits2Float(0x41399153), SkBits2Float(0xc1e00000), SkBits2Float(0x419e6455), SkBits2Float(0xc19e6455));
path.quadTo(SkBits2Float(0x41e00000), SkBits2Float(0xc1399153), SkBits2Float(0x41e00000), SkBits2Float(0x00000000));
path.quadTo(SkBits2Float(0x41e00000), SkBits2Float(0x41399153), SkBits2Float(0x419e6455), SkBits2Float(0x419e6455));
path.quadTo(SkBits2Float(0x415b75ce), SkBits2Float(0x41cf0dc3), SkBits2Float(0x40b878fc), SkBits2Float(0x41db9f74));
path.quadTo(SkBits2Float(0x41000000), SkBits2Float(0x41ee1ba4), SkBits2Float(0x41000000), SkBits2Float(0x42040000));
path.quadTo(SkBits2Float(0x41000000), SkBits2Float(0x4211413d), SkBits2Float(0x40b504f3), SkBits2Float(0x421aa09e));
path.quadTo(SkBits2Float(0x405413cd), SkBits2Float(0x42240000), SkBits2Float(0x00000000), SkBits2Float(0x42240000));
path.quadTo(SkBits2Float(0xc05413cd), SkBits2Float(0x42240000), SkBits2Float(0xc0b504f3), SkBits2Float(0x421aa09e));
path.quadTo(SkBits2Float(0xc1000000), SkBits2Float(0x4211413d), SkBits2Float(0xc1000000), SkBits2Float(0x42040000));
path.quadTo(SkBits2Float(0xc1000000), SkBits2Float(0x41ee1ba4), SkBits2Float(0xc0b878fc), SkBits2Float(0x41db9f74));
path.quadTo(SkBits2Float(0xc15b75ce), SkBits2Float(0x41cf0dc3), SkBits2Float(0xc19e6455), SkBits2Float(0x419e6455));
path.quadTo(SkBits2Float(0xc1e00000), SkBits2Float(0x41399153), SkBits2Float(0xc1e00000), SkBits2Float(0x00000000));
path.quadTo(SkBits2Float(0xc1e00000), SkBits2Float(0xc1399153), SkBits2Float(0xc19e6455), SkBits2Float(0xc19e6455));
path.close();
path.moveTo(SkBits2Float(0xc23c98fa), SkBits2Float(0x408b3eec));
path.quadTo(SkBits2Float(0xc22fcb5c), SkBits2Float(0x405f9a18), SkBits2Float(0xc22450bb), SkBits2Float(0x40a4d200));
path.quadTo(SkBits2Float(0xc218d61a), SkBits2Float(0x40d9d6f4), SkBits2Float(0xc21567dd), SkBits2Float(0x412021ef));
path.quadTo(SkBits2Float(0xc2155d3d), SkBits2Float(0x4120c08f), SkBits2Float(0xc2155303), SkBits2Float(0x41215e9f));
path.quadTo(SkBits2Float(0xc21547f6), SkBits2Float(0x4121fb98), SkBits2Float(0xc2153d2f), SkBits2Float(0x412299db));
path.quadTo(SkBits2Float(0xc2153265), SkBits2Float(0x41233845), SkBits2Float(0xc21527fc), SkBits2Float(0x4123d684));
path.quadTo(SkBits2Float(0xc2151cc4), SkBits2Float(0x41247361), SkBits2Float(0xc21511d9), SkBits2Float(0x41251125));
path.quadTo(SkBits2Float(0xc211888d), SkBits2Float(0x41582a1e), SkBits2Float(0xc21810d2), SkBits2Float(0x4183262b));
path.quadTo(SkBits2Float(0xc21e9918), SkBits2Float(0x419a3747), SkBits2Float(0xc22b5f56), SkBits2Float(0x41a149de));
path.quadTo(SkBits2Float(0xc2382594), SkBits2Float(0x41a85c76), SkBits2Float(0xc243ae22), SkBits2Float(0x419b4beb));
path.quadTo(SkBits2Float(0xc24f36b0), SkBits2Float(0x418e3b60), SkBits2Float(0xc252bffc), SkBits2Float(0x41695dc8));
path.quadTo(SkBits2Float(0xc252cf70), SkBits2Float(0x41687e79), SkBits2Float(0xc252de2e), SkBits2Float(0x41679ef4));
path.quadTo(SkBits2Float(0xc252ee09), SkBits2Float(0x4166c0c4), SkBits2Float(0xc252fd41), SkBits2Float(0x4165e14c));
path.quadTo(SkBits2Float(0xc2530c80), SkBits2Float(0x41650165), SkBits2Float(0xc2531afd), SkBits2Float(0x416421f9));
path.quadTo(SkBits2Float(0xc2532a97), SkBits2Float(0x416343e9), SkBits2Float(0xc2533998), SkBits2Float(0x416263e8));
path.quadTo(SkBits2Float(0xc256a7d4), SkBits2Float(0x412f2d72), SkBits2Float(0xc2500736), SkBits2Float(0x410142ec));
path.quadTo(SkBits2Float(0xc2496698), SkBits2Float(0x40a6b0cc), SkBits2Float(0xc23c98fa), SkBits2Float(0x408b3eec));
path.close();
path.moveTo(SkBits2Float(0x4204d274), SkBits2Float(0x41c5cf9d));
path.quadTo(SkBits2Float(0x42121393), SkBits2Float(0x41c59784), SkBits2Float(0x421b86b4), SkBits2Float(0x41d82e73));
path.quadTo(SkBits2Float(0x4224f9d6), SkBits2Float(0x41eac561), SkBits2Float(0x422515e3), SkBits2Float(0x4202a3d1));
path.quadTo(SkBits2Float(0x422531ef), SkBits2Float(0x420fe4f0), SkBits2Float(0x421be677), SkBits2Float(0x42195811));
path.quadTo(SkBits2Float(0x421b94ff), SkBits2Float(0x4219aae4), SkBits2Float(0x421b423a), SkBits2Float(0x4219fb06));
path.lineTo(SkBits2Float(0x41dabec3), SkBits2Float(0x421aa09e));
path.quadTo(SkBits2Float(0x41c80000), SkBits2Float(0x4211413d), SkBits2Float(0x41c80000), SkBits2Float(0x42040000));
path.quadTo(SkBits2Float(0x41c80000), SkBits2Float(0x41ed7d86), SkBits2Float(0x41dabec3), SkBits2Float(0x41dabec3));
path.quadTo(SkBits2Float(0x41db32ee), SkBits2Float(0x41da4a98), SkBits2Float(0x41dba82d), SkBits2Float(0x41d9d952));
path.quadTo(SkBits2Float(0x41dc1880), SkBits2Float(0x41d9631e), SkBits2Float(0x41dc8bb9), SkBits2Float(0x41d8edf9));
path.quadTo(SkBits2Float(0x41ef22a8), SkBits2Float(0x41c607b5), SkBits2Float(0x4204d274), SkBits2Float(0x41c5cf9d));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0xc243ae22), SkBits2Float(0x419b4beb));
path.quadTo(SkBits2Float(0xc24f36b0), SkBits2Float(0x418e3b60), SkBits2Float(0xc252bffc), SkBits2Float(0x41695dc8));
path.quadTo(SkBits2Float(0xc2564948), SkBits2Float(0x413644ce), SkBits2Float(0xc24fc102), SkBits2Float(0x41082296));
path.quadTo(SkBits2Float(0xc24938bd), SkBits2Float(0x40b400bc), SkBits2Float(0xc23c727f), SkBits2Float(0x4097b660));
path.quadTo(SkBits2Float(0xc22fac40), SkBits2Float(0x4076d800), SkBits2Float(0xc22423b2), SkBits2Float(0x40afae2c));
path.quadTo(SkBits2Float(0xc2189b24), SkBits2Float(0x40e3f058), SkBits2Float(0xc21511d9), SkBits2Float(0x41251125));
path.quadTo(SkBits2Float(0xc211888d), SkBits2Float(0x41582a1e), SkBits2Float(0xc21810d2), SkBits2Float(0x4183262b));
path.quadTo(SkBits2Float(0xc21e9918), SkBits2Float(0x419a3747), SkBits2Float(0xc22b5f56), SkBits2Float(0x41a149de));
path.quadTo(SkBits2Float(0xc2382594), SkBits2Float(0x41a85c76), SkBits2Float(0xc243ae22), SkBits2Float(0x419b4beb));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}


static void fuzz763_378a_1(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0xc23c98fa), SkBits2Float(0x408b3eec));
path.quadTo(SkBits2Float(0xc22fcb5c), SkBits2Float(0x405f9a18), SkBits2Float(0xc22450bb), SkBits2Float(0x40a4d200));
path.quadTo(SkBits2Float(0xc218d61a), SkBits2Float(0x40d9d6f4), SkBits2Float(0xc21567dd), SkBits2Float(0x412021ef));
path.quadTo(SkBits2Float(0xc2155d3d), SkBits2Float(0x4120c08f), SkBits2Float(0xc2155303), SkBits2Float(0x41215e9f));
path.quadTo(SkBits2Float(0xc21547f6), SkBits2Float(0x4121fb98), SkBits2Float(0xc2153d2f), SkBits2Float(0x412299db));
path.quadTo(SkBits2Float(0xc2153265), SkBits2Float(0x41233845), SkBits2Float(0xc21527fc), SkBits2Float(0x4123d684));
path.quadTo(SkBits2Float(0xc2151cc4), SkBits2Float(0x41247361), SkBits2Float(0xc21511d9), SkBits2Float(0x41251125));
path.quadTo(SkBits2Float(0xc211888d), SkBits2Float(0x41582a1e), SkBits2Float(0xc21810d2), SkBits2Float(0x4183262b));
path.quadTo(SkBits2Float(0xc21e9918), SkBits2Float(0x419a3747), SkBits2Float(0xc22b5f56), SkBits2Float(0x41a149de));
path.quadTo(SkBits2Float(0xc2382594), SkBits2Float(0x41a85c76), SkBits2Float(0xc243ae22), SkBits2Float(0x419b4beb));
path.quadTo(SkBits2Float(0xc24f36b0), SkBits2Float(0x418e3b60), SkBits2Float(0xc252bffc), SkBits2Float(0x41695dc8));
path.quadTo(SkBits2Float(0xc252cf70), SkBits2Float(0x41687e79), SkBits2Float(0xc252de2e), SkBits2Float(0x41679ef4));
path.quadTo(SkBits2Float(0xc252ee09), SkBits2Float(0x4166c0c4), SkBits2Float(0xc252fd41), SkBits2Float(0x4165e14c));
path.quadTo(SkBits2Float(0xc2530c80), SkBits2Float(0x41650165), SkBits2Float(0xc2531afd), SkBits2Float(0x416421f9));
path.quadTo(SkBits2Float(0xc2532a97), SkBits2Float(0x416343e9), SkBits2Float(0xc2533998), SkBits2Float(0x416263e8));
path.quadTo(SkBits2Float(0xc256a7d4), SkBits2Float(0x412f2d72), SkBits2Float(0xc2500736), SkBits2Float(0x410142ec));
path.quadTo(SkBits2Float(0xc2496698), SkBits2Float(0x40a6b0cc), SkBits2Float(0xc23c98fa), SkBits2Float(0x408b3eec));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0xc243ae22), SkBits2Float(0x419b4beb));
path.quadTo(SkBits2Float(0xc24f36b0), SkBits2Float(0x418e3b60), SkBits2Float(0xc252bffc), SkBits2Float(0x41695dc8));
path.quadTo(SkBits2Float(0xc2564948), SkBits2Float(0x413644ce), SkBits2Float(0xc24fc102), SkBits2Float(0x41082296));
path.quadTo(SkBits2Float(0xc24938bd), SkBits2Float(0x40b400bc), SkBits2Float(0xc23c727f), SkBits2Float(0x4097b660));
path.quadTo(SkBits2Float(0xc22fac40), SkBits2Float(0x4076d800), SkBits2Float(0xc22423b2), SkBits2Float(0x40afae2c));
path.quadTo(SkBits2Float(0xc2189b24), SkBits2Float(0x40e3f058), SkBits2Float(0xc21511d9), SkBits2Float(0x41251125));
path.quadTo(SkBits2Float(0xc211888d), SkBits2Float(0x41582a1e), SkBits2Float(0xc21810d2), SkBits2Float(0x4183262b));
path.quadTo(SkBits2Float(0xc21e9918), SkBits2Float(0x419a3747), SkBits2Float(0xc22b5f56), SkBits2Float(0x41a149de));
path.quadTo(SkBits2Float(0xc2382594), SkBits2Float(0x41a85c76), SkBits2Float(0xc243ae22), SkBits2Float(0x419b4beb));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
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
    testPathOp(reporter, path, pathB, kDifference_PathOp, filename);
}

static void fuzz763_8712(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x40dce520), SkBits2Float(0xc250b45c));
path.quadTo(SkBits2Float(0x411bc0ec), SkBits2Float(0xc25796e0), SkBits2Float(0x414f4352), SkBits2Float(0xc25472d6));
path.quadTo(SkBits2Float(0x418162dd), SkBits2Float(0xc2514ece), SkBits2Float(0x418f27e4), SkBits2Float(0xc245fb37));
path.quadTo(SkBits2Float(0x419cecea), SkBits2Float(0xc23aa7a0), SkBits2Float(0x4196a4d8), SkBits2Float(0xc22dc706));
path.quadTo(SkBits2Float(0x41905cc8), SkBits2Float(0xc220e66c), SkBits2Float(0x41736b34), SkBits2Float(0xc21a03e9));
path.quadTo(SkBits2Float(0x41461cda), SkBits2Float(0xc2132166), SkBits2Float(0x41129a71), SkBits2Float(0xc216456f));
path.quadTo(SkBits2Float(0x40be3010), SkBits2Float(0xc2196978), SkBits2Float(0x40871bf8), SkBits2Float(0xc224bd0e));
path.quadTo(SkBits2Float(0x40200fb8), SkBits2Float(0xc23010a5), SkBits2Float(0x40525050), SkBits2Float(0xc23cf13e));
path.quadTo(SkBits2Float(0x4082486c), SkBits2Float(0xc249d1d9), SkBits2Float(0x40dce520), SkBits2Float(0xc250b45c));
path.close();
path.moveTo(SkBits2Float(0xc19e6455), SkBits2Float(0xc19e6455));
path.quadTo(SkBits2Float(0xc1399153), SkBits2Float(0xc1e00000), SkBits2Float(0x00000000), SkBits2Float(0xc1e00000));
path.quadTo(SkBits2Float(0x41399153), SkBits2Float(0xc1e00000), SkBits2Float(0x419e6455), SkBits2Float(0xc19e6455));
path.quadTo(SkBits2Float(0x41e00000), SkBits2Float(0xc1399153), SkBits2Float(0x41e00000), SkBits2Float(0x00000000));
path.quadTo(SkBits2Float(0x41e00000), SkBits2Float(0x41399153), SkBits2Float(0x419e6455), SkBits2Float(0x419e6455));
path.quadTo(SkBits2Float(0x415b75ce), SkBits2Float(0x41cf0dc3), SkBits2Float(0x40b878fc), SkBits2Float(0x41db9f74));
path.quadTo(SkBits2Float(0x41000000), SkBits2Float(0x41ee1ba4), SkBits2Float(0x41000000), SkBits2Float(0x42040000));
path.quadTo(SkBits2Float(0x41000000), SkBits2Float(0x4211413d), SkBits2Float(0x40b504f3), SkBits2Float(0x421aa09e));
path.quadTo(SkBits2Float(0x405413cd), SkBits2Float(0x42240000), SkBits2Float(0x00000000), SkBits2Float(0x42240000));
path.quadTo(SkBits2Float(0xc05413cd), SkBits2Float(0x42240000), SkBits2Float(0xc0b504f3), SkBits2Float(0x421aa09e));
path.quadTo(SkBits2Float(0xc1000000), SkBits2Float(0x4211413d), SkBits2Float(0xc1000000), SkBits2Float(0x42040000));
path.quadTo(SkBits2Float(0xc1000000), SkBits2Float(0x41ee1ba4), SkBits2Float(0xc0b878fc), SkBits2Float(0x41db9f74));
path.quadTo(SkBits2Float(0xc15b75ce), SkBits2Float(0x41cf0dc3), SkBits2Float(0xc19e6455), SkBits2Float(0x419e6455));
path.quadTo(SkBits2Float(0xc1e00000), SkBits2Float(0x41399153), SkBits2Float(0xc1e00000), SkBits2Float(0x00000000));
path.quadTo(SkBits2Float(0xc1e00000), SkBits2Float(0xc1399153), SkBits2Float(0xc19e6455), SkBits2Float(0xc19e6455));
path.close();
path.moveTo(SkBits2Float(0x422b20ca), SkBits2Float(0xc1a252a8));
path.quadTo(SkBits2Float(0x4237e448), SkBits2Float(0xc1a97900), SkBits2Float(0x424371e0), SkBits2Float(0xc19c7a4f));
path.quadTo(SkBits2Float(0x424eff77), SkBits2Float(0xc18f7b9e), SkBits2Float(0x425292a1), SkBits2Float(0xc16be93c));
path.quadTo(SkBits2Float(0x425625cd), SkBits2Float(0xc138db44), SkBits2Float(0x424fa674), SkBits2Float(0xc10aa4e6));
path.quadTo(SkBits2Float(0x4249271c), SkBits2Float(0xc0b8dd14), SkBits2Float(0x423c639c), SkBits2Float(0xc09c43bc));
path.quadTo(SkBits2Float(0x422fa01e), SkBits2Float(0xc07f54c8), SkBits2Float(0x42241287), SkBits2Float(0xc0b3a528));
path.quadTo(SkBits2Float(0x421884f0), SkBits2Float(0xc0e79fee), SkBits2Float(0x4214f1c4), SkBits2Float(0xc126ddf2));
path.quadTo(SkBits2Float(0x42115e99), SkBits2Float(0xc159ebed), SkBits2Float(0x4217ddf2), SkBits2Float(0xc1841124));
path.quadTo(SkBits2Float(0x421e5d4a), SkBits2Float(0xc19b2c54), SkBits2Float(0x422b20ca), SkBits2Float(0xc1a252a8));
path.close();
path.moveTo(SkBits2Float(0xc23c98fa), SkBits2Float(0x408b3eec));
path.quadTo(SkBits2Float(0xc22fcb5c), SkBits2Float(0x405f9a18), SkBits2Float(0xc22450bb), SkBits2Float(0x40a4d200));
path.quadTo(SkBits2Float(0xc218d61a), SkBits2Float(0x40d9d6f4), SkBits2Float(0xc21567dd), SkBits2Float(0x412021ef));
path.quadTo(SkBits2Float(0xc2152d73), SkBits2Float(0x412389fb), SkBits2Float(0xc214fe6a), SkBits2Float(0x4126ec3a));
path.quadTo(SkBits2Float(0xc214b621), SkBits2Float(0x412a3217), SkBits2Float(0xc21476d0), SkBits2Float(0x412d948d));
path.quadTo(SkBits2Float(0xc210bed3), SkBits2Float(0x41607862), SkBits2Float(0xc2171cb0), SkBits2Float(0x41877c8b));
path.quadTo(SkBits2Float(0xc21d7a8c), SkBits2Float(0x419ebce4), SkBits2Float(0xc22a3381), SkBits2Float(0x41a62cde));
path.quadTo(SkBits2Float(0xc236ec77), SkBits2Float(0x41ad9cd6), SkBits2Float(0xc2428ca4), SkBits2Float(0x41a0e11e));
path.quadTo(SkBits2Float(0xc24e2cd0), SkBits2Float(0x41942565), SkBits2Float(0xc251e4cc), SkBits2Float(0x417566f6));
path.quadTo(SkBits2Float(0xc2523ea2), SkBits2Float(0x41709990), SkBits2Float(0xc252817b), SkBits2Float(0x416bd61f));
path.quadTo(SkBits2Float(0xc252e6b7), SkBits2Float(0x41673927), SkBits2Float(0xc2533998), SkBits2Float(0x416263e8));
path.quadTo(SkBits2Float(0xc256a7d4), SkBits2Float(0x412f2d72), SkBits2Float(0xc2500736), SkBits2Float(0x410142ec));
path.quadTo(SkBits2Float(0xc2496698), SkBits2Float(0x40a6b0cc), SkBits2Float(0xc23c98fa), SkBits2Float(0x408b3eec));
path.close();
path.moveTo(SkBits2Float(0x42074f3a), SkBits2Float(0x41bef2d8));
path.quadTo(SkBits2Float(0x42148e85), SkBits2Float(0x41be0d1d), SkBits2Float(0x421e3dbf), SkBits2Float(0x41d026ae));
path.quadTo(SkBits2Float(0x4227ecfa), SkBits2Float(0x41e24040), SkBits2Float(0x42285fd8), SkBits2Float(0x41fcbed6));
path.quadTo(SkBits2Float(0x4228d2b5), SkBits2Float(0x420b9eb6), SkBits2Float(0x421fc5ec), SkBits2Float(0x42154df0));
path.quadTo(SkBits2Float(0x421f5958), SkBits2Float(0x4215c221), SkBits2Float(0x421eea62), SkBits2Float(0x42163126));
path.quadTo(SkBits2Float(0x421e81d1), SkBits2Float(0x4216a62c), SkBits2Float(0x421e13f4), SkBits2Float(0x4217191c));
path.quadTo(SkBits2Float(0x421d36b1), SkBits2Float(0x42180097), SkBits2Float(0x421c5020), SkBits2Float(0x4218d2e4));
path.quadTo(SkBits2Float(0x421bb44d), SkBits2Float(0x421985ae), SkBits2Float(0x421b0c17), SkBits2Float(0x421a3367));
path.lineTo(SkBits2Float(0x421aa09e), SkBits2Float(0x421aa09e));
path.quadTo(SkBits2Float(0x4211413d), SkBits2Float(0x42240000), SkBits2Float(0x42040000), SkBits2Float(0x42240000));
path.quadTo(SkBits2Float(0x41ed7d86), SkBits2Float(0x42240000), SkBits2Float(0x41dabec3), SkBits2Float(0x421aa09e));
path.quadTo(SkBits2Float(0x41c80000), SkBits2Float(0x4211413d), SkBits2Float(0x41c80000), SkBits2Float(0x42040000));
path.quadTo(SkBits2Float(0x41c80000), SkBits2Float(0x41ed7d86), SkBits2Float(0x41dabec3), SkBits2Float(0x41dabec3));
path.quadTo(SkBits2Float(0x41dbfdf8), SkBits2Float(0x41d97f8e), SkBits2Float(0x41dd45f8), SkBits2Float(0x41d85595));
path.quadTo(SkBits2Float(0x41de6877), SkBits2Float(0x41d706ef), SkBits2Float(0x41dfa063), SkBits2Float(0x41d5c09b));
path.quadTo(SkBits2Float(0x41e03b86), SkBits2Float(0x41d51e4d), SkBits2Float(0x41e0d904), SkBits2Float(0x41d48124));
path.quadTo(SkBits2Float(0x41e16d06), SkBits2Float(0x41d3db0f), SkBits2Float(0x41e2064d), SkBits2Float(0x41d33709));
path.quadTo(SkBits2Float(0x41f41fdf), SkBits2Float(0x41bfd894), SkBits2Float(0x42074f3a), SkBits2Float(0x41bef2d8));
path.close();
    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0xc2428ca4), SkBits2Float(0x41a0e11e));
path.quadTo(SkBits2Float(0xc24e2cd0), SkBits2Float(0x41942565), SkBits2Float(0xc251e4cc), SkBits2Float(0x417566f6));
path.quadTo(SkBits2Float(0xc2559cca), SkBits2Float(0x4142831e), SkBits2Float(0xc24f3eed), SkBits2Float(0x4114026c));
path.quadTo(SkBits2Float(0xc248e111), SkBits2Float(0x40cb0370), SkBits2Float(0xc23c281b), SkBits2Float(0x40ad4390));
path.quadTo(SkBits2Float(0xc22f6f26), SkBits2Float(0x408f83a8), SkBits2Float(0xc223cefa), SkBits2Float(0x40c2728a));
path.quadTo(SkBits2Float(0xc2182ecc), SkBits2Float(0x40f5616e), SkBits2Float(0xc21476d0), SkBits2Float(0x412d948d));
path.quadTo(SkBits2Float(0xc210bed3), SkBits2Float(0x41607862), SkBits2Float(0xc2171cb0), SkBits2Float(0x41877c8b));
path.quadTo(SkBits2Float(0xc21d7a8c), SkBits2Float(0x419ebce4), SkBits2Float(0xc22a3381), SkBits2Float(0x41a62cde));
path.quadTo(SkBits2Float(0xc236ec77), SkBits2Float(0x41ad9cd6), SkBits2Float(0xc2428ca4), SkBits2Float(0x41a0e11e));
path.close();

    SkPath path2(path);
    testPathOpCheck(reporter, path1, path2, (SkPathOp) 2, filename, FLAGS_runFail);
}

static void fuzz763_8712a(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0xc23c98fa), SkBits2Float(0x408b3eec));
path.quadTo(SkBits2Float(0xc22fcb5c), SkBits2Float(0x405f9a18), SkBits2Float(0xc22450bb), SkBits2Float(0x40a4d200));
path.quadTo(SkBits2Float(0xc218d61a), SkBits2Float(0x40d9d6f4), SkBits2Float(0xc21567dd), SkBits2Float(0x412021ef));
path.quadTo(SkBits2Float(0xc2152d73), SkBits2Float(0x412389fb), SkBits2Float(0xc214fe6a), SkBits2Float(0x4126ec3a));
path.quadTo(SkBits2Float(0xc214b621), SkBits2Float(0x412a3217), SkBits2Float(0xc21476d0), SkBits2Float(0x412d948d));
path.quadTo(SkBits2Float(0xc210bed3), SkBits2Float(0x41607862), SkBits2Float(0xc2171cb0), SkBits2Float(0x41877c8b));
path.quadTo(SkBits2Float(0xc21d7a8c), SkBits2Float(0x419ebce4), SkBits2Float(0xc22a3381), SkBits2Float(0x41a62cde));
path.quadTo(SkBits2Float(0xc236ec77), SkBits2Float(0x41ad9cd6), SkBits2Float(0xc2428ca4), SkBits2Float(0x41a0e11e));
path.quadTo(SkBits2Float(0xc24e2cd0), SkBits2Float(0x41942565), SkBits2Float(0xc251e4cc), SkBits2Float(0x417566f6));
path.quadTo(SkBits2Float(0xc2523ea2), SkBits2Float(0x41709990), SkBits2Float(0xc252817b), SkBits2Float(0x416bd61f));
path.quadTo(SkBits2Float(0xc252e6b7), SkBits2Float(0x41673927), SkBits2Float(0xc2533998), SkBits2Float(0x416263e8));
path.quadTo(SkBits2Float(0xc256a7d4), SkBits2Float(0x412f2d72), SkBits2Float(0xc2500736), SkBits2Float(0x410142ec));
path.quadTo(SkBits2Float(0xc2496698), SkBits2Float(0x40a6b0cc), SkBits2Float(0xc23c98fa), SkBits2Float(0x408b3eec));
path.close();
    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0xc2428ca4), SkBits2Float(0x41a0e11e));
path.quadTo(SkBits2Float(0xc24e2cd0), SkBits2Float(0x41942565), SkBits2Float(0xc251e4cc), SkBits2Float(0x417566f6));
path.quadTo(SkBits2Float(0xc2559cca), SkBits2Float(0x4142831e), SkBits2Float(0xc24f3eed), SkBits2Float(0x4114026c));
path.quadTo(SkBits2Float(0xc248e111), SkBits2Float(0x40cb0370), SkBits2Float(0xc23c281b), SkBits2Float(0x40ad4390));
path.quadTo(SkBits2Float(0xc22f6f26), SkBits2Float(0x408f83a8), SkBits2Float(0xc223cefa), SkBits2Float(0x40c2728a));
path.quadTo(SkBits2Float(0xc2182ecc), SkBits2Float(0x40f5616e), SkBits2Float(0xc21476d0), SkBits2Float(0x412d948d));
path.quadTo(SkBits2Float(0xc210bed3), SkBits2Float(0x41607862), SkBits2Float(0xc2171cb0), SkBits2Float(0x41877c8b));
path.quadTo(SkBits2Float(0xc21d7a8c), SkBits2Float(0x419ebce4), SkBits2Float(0xc22a3381), SkBits2Float(0x41a62cde));
path.quadTo(SkBits2Float(0xc236ec77), SkBits2Float(0x41ad9cd6), SkBits2Float(0xc2428ca4), SkBits2Float(0x41a0e11e));
path.close();

    SkPath path2(path);
    testPathOpCheck(reporter, path1, path2, (SkPathOp) 2, filename, FLAGS_runFail);
}

static void fuzz763_4014(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x4126977e), SkBits2Float(0xc24e5cc8));
path.quadTo(SkBits2Float(0x4155a79e), SkBits2Float(0xc2547762), SkBits2Float(0x41841952), SkBits2Float(0xc250767b));
path.quadTo(SkBits2Float(0x419d5ed4), SkBits2Float(0xc24c7594), SkBits2Float(0x41a99408), SkBits2Float(0xc240b18c));
path.quadTo(SkBits2Float(0x41b5c93d), SkBits2Float(0xc234ed84), SkBits2Float(0x41adc770), SkBits2Float(0xc2284ac3));
path.quadTo(SkBits2Float(0x41a5c5a2), SkBits2Float(0xc21ba802), SkBits2Float(0x418e3d92), SkBits2Float(0xc2158d68));
path.quadTo(SkBits2Float(0x416d6b02), SkBits2Float(0xc20f72ce), SkBits2Float(0x413adfff), SkBits2Float(0xc21373b4));
path.quadTo(SkBits2Float(0x410854fa), SkBits2Float(0xc217749a), SkBits2Float(0x40dfd522), SkBits2Float(0xc22338a3));
path.quadTo(SkBits2Float(0x40af0050), SkBits2Float(0xc22efcab), SkBits2Float(0x40cf0788), SkBits2Float(0xc23b9f6c));
path.quadTo(SkBits2Float(0x40ef0eb8), SkBits2Float(0xc248422e), SkBits2Float(0x4126977e), SkBits2Float(0xc24e5cc8));
path.close();
path.moveTo(SkBits2Float(0xc19e6455), SkBits2Float(0xc19e6455));
path.quadTo(SkBits2Float(0xc1399153), SkBits2Float(0xc1e00000), SkBits2Float(0x00000000), SkBits2Float(0xc1e00000));
path.quadTo(SkBits2Float(0x41399153), SkBits2Float(0xc1e00000), SkBits2Float(0x419e6455), SkBits2Float(0xc19e6455));
path.quadTo(SkBits2Float(0x41e00000), SkBits2Float(0xc1399153), SkBits2Float(0x41e00000), SkBits2Float(0x00000000));
path.quadTo(SkBits2Float(0x41e00000), SkBits2Float(0x41399153), SkBits2Float(0x419e6455), SkBits2Float(0x419e6455));
path.quadTo(SkBits2Float(0x415b75ce), SkBits2Float(0x41cf0dc3), SkBits2Float(0x40b878fc), SkBits2Float(0x41db9f74));
path.quadTo(SkBits2Float(0x41000000), SkBits2Float(0x41ee1ba4), SkBits2Float(0x41000000), SkBits2Float(0x42040000));
path.quadTo(SkBits2Float(0x41000000), SkBits2Float(0x4211413d), SkBits2Float(0x40b504f3), SkBits2Float(0x421aa09e));
path.quadTo(SkBits2Float(0x405413cd), SkBits2Float(0x42240000), SkBits2Float(0x00000000), SkBits2Float(0x42240000));
path.quadTo(SkBits2Float(0xc05413cd), SkBits2Float(0x42240000), SkBits2Float(0xc0b504f3), SkBits2Float(0x421aa09e));
path.quadTo(SkBits2Float(0xc1000000), SkBits2Float(0x4211413d), SkBits2Float(0xc1000000), SkBits2Float(0x42040000));
path.quadTo(SkBits2Float(0xc1000000), SkBits2Float(0x41ee1ba4), SkBits2Float(0xc0b878fc), SkBits2Float(0x41db9f74));
path.quadTo(SkBits2Float(0xc15b75ce), SkBits2Float(0x41cf0dc3), SkBits2Float(0xc19e6455), SkBits2Float(0x419e6455));
path.quadTo(SkBits2Float(0xc1e00000), SkBits2Float(0x41399153), SkBits2Float(0xc1e00000), SkBits2Float(0x00000000));
path.quadTo(SkBits2Float(0xc1e00000), SkBits2Float(0xc1399153), SkBits2Float(0xc19e6455), SkBits2Float(0xc19e6455));
path.close();
path.moveTo(SkBits2Float(0x422dc5fa), SkBits2Float(0xc196a9b4));
path.quadTo(SkBits2Float(0x423aa688), SkBits2Float(0xc19cf222), SkBits2Float(0x4245fa38), SkBits2Float(0xc18f2d6c));
path.quadTo(SkBits2Float(0x42514de7), SkBits2Float(0xc18168b7), SkBits2Float(0x4254721e), SkBits2Float(0xc14f4f32));
path.quadTo(SkBits2Float(0x42579654), SkBits2Float(0xc11bccf8), SkBits2Float(0x4250b3fa), SkBits2Float(0xc0dcfc74));
path.quadTo(SkBits2Float(0x4249d19f), SkBits2Float(0xc0825efc), SkBits2Float(0x423cf110), SkBits2Float(0xc0527a88));
path.quadTo(SkBits2Float(0x42301082), SkBits2Float(0xc0203718), SkBits2Float(0x4224bcd2), SkBits2Float(0xc0872e60));
path.quadTo(SkBits2Float(0x42196923), SkBits2Float(0xc0be4136), SkBits2Float(0x421644ec), SkBits2Float(0xc112a2d8));
path.quadTo(SkBits2Float(0x421320b5), SkBits2Float(0xc1462514), SkBits2Float(0x421a0310), SkBits2Float(0xc17373d0));
path.quadTo(SkBits2Float(0x4220e56a), SkBits2Float(0xc1906147), SkBits2Float(0x422dc5fa), SkBits2Float(0xc196a9b4));
path.close();
path.moveTo(SkBits2Float(0xc23d30a0), SkBits2Float(0x400e5c28));
path.quadTo(SkBits2Float(0xc2303ecc), SkBits2Float(0x3fc17f10), SkBits2Float(0xc2251387), SkBits2Float(0x4052f2a8));
path.quadTo(SkBits2Float(0xc219e842), SkBits2Float(0x40a292e2), SkBits2Float(0xc2170e78), SkBits2Float(0x410510c3));
path.quadTo(SkBits2Float(0xc216ac8b), SkBits2Float(0x410c0373), SkBits2Float(0xc21678a8), SkBits2Float(0x4112d552));
path.quadTo(SkBits2Float(0xc215ddb7), SkBits2Float(0x411942a8), SkBits2Float(0xc21567dd), SkBits2Float(0x412021ef));
path.quadTo(SkBits2Float(0xc211f9a2), SkBits2Float(0x41535866), SkBits2Float(0xc2189a40), SkBits2Float(0x4180a176));
path.quadTo(SkBits2Float(0xc21f3ade), SkBits2Float(0x419796b9), SkBits2Float(0xc22c087c), SkBits2Float(0x419e7330));
path.quadTo(SkBits2Float(0xc238d61a), SkBits2Float(0x41a54fa9), SkBits2Float(0xc24450bb), SkBits2Float(0x41980e6c));
path.quadTo(SkBits2Float(0xc24fcb5c), SkBits2Float(0x418acd2f), SkBits2Float(0xc2533998), SkBits2Float(0x416263e8));
path.quadTo(SkBits2Float(0xc253e12c), SkBits2Float(0x41589e3d), SkBits2Float(0xc2542b01), SkBits2Float(0x414f09d7));
path.quadTo(SkBits2Float(0xc25503cd), SkBits2Float(0x414600f6), SkBits2Float(0xc2558f0e), SkBits2Float(0x413c1fa8));
path.quadTo(SkBits2Float(0xc25868d8), SkBits2Float(0x41085856), SkBits2Float(0xc25145a6), SkBits2Float(0x40b75684));
path.quadTo(SkBits2Float(0xc24a2274), SkBits2Float(0x403bf8b8), SkBits2Float(0xc23d30a0), SkBits2Float(0x400e5c28));
path.close();
path.moveTo(SkBits2Float(0x41dabec3), SkBits2Float(0x41dabec3));
path.quadTo(SkBits2Float(0x41ed7d86), SkBits2Float(0x41c80000), SkBits2Float(0x42040000), SkBits2Float(0x41c80000));
path.quadTo(SkBits2Float(0x4211413d), SkBits2Float(0x41c80000), SkBits2Float(0x421aa09e), SkBits2Float(0x41dabec3));
path.quadTo(SkBits2Float(0x42240000), SkBits2Float(0x41ed7d86), SkBits2Float(0x42240000), SkBits2Float(0x42040000));
path.quadTo(SkBits2Float(0x42240000), SkBits2Float(0x4211413d), SkBits2Float(0x421aa09e), SkBits2Float(0x421aa09e));
path.quadTo(SkBits2Float(0x4218d6c4), SkBits2Float(0x421c6a78), SkBits2Float(0x4216e8ba), SkBits2Float(0x421ddcf4));
path.quadTo(SkBits2Float(0x42156061), SkBits2Float(0x421fb9cf), SkBits2Float(0x42138263), SkBits2Float(0x42216e93));
path.quadTo(SkBits2Float(0x42129692), SkBits2Float(0x4222460e), SkBits2Float(0x4211a2ed), SkBits2Float(0x422307e2));
path.quadTo(SkBits2Float(0x4210c6f1), SkBits2Float(0x4223e438), SkBits2Float(0x420fd652), SkBits2Float(0x4224b658));
path.quadTo(SkBits2Float(0x4205da74), SkBits2Float(0x422d6e4b), SkBits2Float(0x41f141e8), SkBits2Float(0x422c893f));
path.quadTo(SkBits2Float(0x41d6cee9), SkBits2Float(0x422ba432), SkBits2Float(0x41c55f04), SkBits2Float(0x4221a853));
path.quadTo(SkBits2Float(0x41b3ef1f), SkBits2Float(0x4217ac75), SkBits2Float(0x41b5b938), SkBits2Float(0x420a72f5));
path.quadTo(SkBits2Float(0x41b78350), SkBits2Float(0x41fa72eb), SkBits2Float(0x41cb7b0e), SkBits2Float(0x41e90306));
path.quadTo(SkBits2Float(0x41ccce3f), SkBits2Float(0x41e7dad2), SkBits2Float(0x41ce28c1), SkBits2Float(0x41e6c848));
path.quadTo(SkBits2Float(0x41cf607c), SkBits2Float(0x41e58ed0), SkBits2Float(0x41d0aced), SkBits2Float(0x41e45f0b));
path.quadTo(SkBits2Float(0x41d34d34), SkBits2Float(0x41e1f8bf), SkBits2Float(0x41d60d52), SkBits2Float(0x41dfea66));
path.quadTo(SkBits2Float(0x41d83ad5), SkBits2Float(0x41dd42b1), SkBits2Float(0x41dabec3), SkBits2Float(0x41dabec3));
path.close();
    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0xc2478a00), SkBits2Float(0x418639e0));
path.quadTo(SkBits2Float(0xc252b546), SkBits2Float(0x416fe6f8), SkBits2Float(0xc2558f0e), SkBits2Float(0x413c1fa8));
path.quadTo(SkBits2Float(0xc25868d8), SkBits2Float(0x41085856), SkBits2Float(0xc25145a6), SkBits2Float(0x40b75684));
path.quadTo(SkBits2Float(0xc24a2274), SkBits2Float(0x403bf8b8), SkBits2Float(0xc23d30a0), SkBits2Float(0x400e5c28));
path.quadTo(SkBits2Float(0xc2303ecc), SkBits2Float(0x3fc17f10), SkBits2Float(0xc2251387), SkBits2Float(0x4052f2a8));
path.quadTo(SkBits2Float(0xc219e842), SkBits2Float(0x40a292e2), SkBits2Float(0xc2170e78), SkBits2Float(0x410510c3));
path.quadTo(SkBits2Float(0xc21434af), SkBits2Float(0x4138d815), SkBits2Float(0xc21b57e0), SkBits2Float(0x41658529));
path.quadTo(SkBits2Float(0xc2227b12), SkBits2Float(0x4189191e), SkBits2Float(0xc22f6ce6), SkBits2Float(0x418eccb0));
path.quadTo(SkBits2Float(0xc23c5ebc), SkBits2Float(0x41948044), SkBits2Float(0xc2478a00), SkBits2Float(0x418639e0));
path.close();

    SkPath path2(path);
    testPathOpCheck(reporter, path1, path2, (SkPathOp) 2, filename, FLAGS_runFail);
}

static void fuzz763_4014a(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0xc23d30a0), SkBits2Float(0x400e5c28));  // -47.2975f, 2.22437f
path.quadTo(SkBits2Float(0xc2303ecc), SkBits2Float(0x3fc17f10), SkBits2Float(0xc2251387), SkBits2Float(0x4052f2a8));  // -44.0613f, 1.51169f, -41.2691f, 3.29606f
path.quadTo(SkBits2Float(0xc219e842), SkBits2Float(0x40a292e2), SkBits2Float(0xc2170e78), SkBits2Float(0x410510c3));  // -38.4768f, 5.08043f, -37.7641f, 8.31659f
path.quadTo(SkBits2Float(0xc216ac8b), SkBits2Float(0x410c0373), SkBits2Float(0xc21678a8), SkBits2Float(0x4112d552));  // -37.6685f, 8.75084f, -37.6178f, 9.17708f
path.quadTo(SkBits2Float(0xc215ddb7), SkBits2Float(0x411942a8), SkBits2Float(0xc21567dd), SkBits2Float(0x412021ef));  // -37.4665f, 9.57877f, -37.3514f, 10.0083f
path.quadTo(SkBits2Float(0xc211f9a2), SkBits2Float(0x41535866), SkBits2Float(0xc2189a40), SkBits2Float(0x4180a176));  // -36.4938f, 13.2091f, -38.1506f, 16.0788f
path.quadTo(SkBits2Float(0xc21f3ade), SkBits2Float(0x419796b9), SkBits2Float(0xc22c087c), SkBits2Float(0x419e7330));  // -39.8075f, 18.9486f, -43.0083f, 19.8062f
path.quadTo(SkBits2Float(0xc238d61a), SkBits2Float(0x41a54fa9), SkBits2Float(0xc24450bb), SkBits2Float(0x41980e6c));  // -46.2091f, 20.6639f, -49.0788f, 19.007f
path.quadTo(SkBits2Float(0xc24fcb5c), SkBits2Float(0x418acd2f), SkBits2Float(0xc2533998), SkBits2Float(0x416263e8));  // -51.9486f, 17.3502f, -52.8062f, 14.1494f
path.quadTo(SkBits2Float(0xc253e12c), SkBits2Float(0x41589e3d), SkBits2Float(0xc2542b01), SkBits2Float(0x414f09d7));  // -52.9699f, 13.5386f, -53.042f, 12.9399f
path.quadTo(SkBits2Float(0xc25503cd), SkBits2Float(0x414600f6), SkBits2Float(0xc2558f0e), SkBits2Float(0x413c1fa8));  // -53.2537f, 12.3752f, -53.3897f, 11.7577f
path.quadTo(SkBits2Float(0xc25868d8), SkBits2Float(0x41085856), SkBits2Float(0xc25145a6), SkBits2Float(0x40b75684));  // -54.1024f, 8.52157f, -52.318f, 5.72931f
path.quadTo(SkBits2Float(0xc24a2274), SkBits2Float(0x403bf8b8), SkBits2Float(0xc23d30a0), SkBits2Float(0x400e5c28));  // -50.5336f, 2.93706f, -47.2975f, 2.22437f
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0xc2478a00), SkBits2Float(0x418639e0));  // -49.8848f, 16.7783f
path.quadTo(SkBits2Float(0xc252b546), SkBits2Float(0x416fe6f8), SkBits2Float(0xc2558f0e), SkBits2Float(0x413c1fa8));  // -52.677f, 14.9939f, -53.3897f, 11.7577f
path.quadTo(SkBits2Float(0xc25868d8), SkBits2Float(0x41085856), SkBits2Float(0xc25145a6), SkBits2Float(0x40b75684));  // -54.1024f, 8.52157f, -52.318f, 5.72931f
path.quadTo(SkBits2Float(0xc24a2274), SkBits2Float(0x403bf8b8), SkBits2Float(0xc23d30a0), SkBits2Float(0x400e5c28));  // -50.5336f, 2.93706f, -47.2975f, 2.22437f
path.quadTo(SkBits2Float(0xc2303ecc), SkBits2Float(0x3fc17f10), SkBits2Float(0xc2251387), SkBits2Float(0x4052f2a8));  // -44.0613f, 1.51169f, -41.2691f, 3.29606f
path.quadTo(SkBits2Float(0xc219e842), SkBits2Float(0x40a292e2), SkBits2Float(0xc2170e78), SkBits2Float(0x410510c3));  // -38.4768f, 5.08043f, -37.7641f, 8.31659f
path.quadTo(SkBits2Float(0xc21434af), SkBits2Float(0x4138d815), SkBits2Float(0xc21b57e0), SkBits2Float(0x41658529));  // -37.0514f, 11.5528f, -38.8358f, 14.345f
path.quadTo(SkBits2Float(0xc2227b12), SkBits2Float(0x4189191e), SkBits2Float(0xc22f6ce6), SkBits2Float(0x418eccb0));  // -40.6202f, 17.1373f, -43.8563f, 17.8499f
path.quadTo(SkBits2Float(0xc23c5ebc), SkBits2Float(0x41948044), SkBits2Float(0xc2478a00), SkBits2Float(0x418639e0));  // -47.0925f, 18.5626f, -49.8848f, 16.7783f
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}

static void fuzz763_1404(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x419b2e3e), SkBits2Float(0xc243b405));
path.quadTo(SkBits2Float(0x41b4811d), SkBits2Float(0xc2479f9a), SkBits2Float(0x41cbf476), SkBits2Float(0xc2417131));
path.quadTo(SkBits2Float(0x41e19882), SkBits2Float(0xc23bbceb), SkBits2Float(0x41e9f15f), SkBits2Float(0xc23083cb));
path.quadTo(SkBits2Float(0x4200ef06), SkBits2Float(0xc2310b91), SkBits2Float(0x420a6762), SkBits2Float(0xc2294dac));
path.quadTo(SkBits2Float(0x4214aa80), SkBits2Float(0xc220ea0a), SkBits2Float(0x4215fd8a), SkBits2Float(0xc213b9c8));
path.quadTo(SkBits2Float(0x42175094), SkBits2Float(0xc2068986), SkBits2Float(0x420eecf0), SkBits2Float(0xc1f88cd3));
path.quadTo(SkBits2Float(0x4206894d), SkBits2Float(0xc1e4069a), SkBits2Float(0x41f2b216), SkBits2Float(0xc1e16085));
path.quadTo(SkBits2Float(0x41d85192), SkBits2Float(0xc1deba71), SkBits2Float(0x41c3cb5a), SkBits2Float(0xc1ef81b8));
path.quadTo(SkBits2Float(0x41b61bc0), SkBits2Float(0xc1fab1e0), SkBits2Float(0x41b05ee8), SkBits2Float(0xc2051350));
path.quadTo(SkBits2Float(0x419fe690), SkBits2Float(0xc204b4a5), SkBits2Float(0x4190436e), SkBits2Float(0xc208d3d1));
path.quadTo(SkBits2Float(0x4171a027), SkBits2Float(0xc20f0238), SkBits2Float(0x4161f1d7), SkBits2Float(0xc21baba8));
path.quadTo(SkBits2Float(0x41524386), SkBits2Float(0xc2285517), SkBits2Float(0x416afd23), SkBits2Float(0xc2340ec3));
path.quadTo(SkBits2Float(0x4181db5f), SkBits2Float(0xc23fc871), SkBits2Float(0x419b2e3e), SkBits2Float(0xc243b405));
path.close();
path.moveTo(SkBits2Float(0xc221f910), SkBits2Float(0xc2067acc));
path.quadTo(SkBits2Float(0xc214fb93), SkBits2Float(0xc2091d7b), SkBits2Float(0xc209ef07), SkBits2Float(0xc201cb14));
path.quadTo(SkBits2Float(0xc1fdc4f6), SkBits2Float(0xc1f4f15c), SkBits2Float(0xc1f87f97), SkBits2Float(0xc1daf662));
path.quadTo(SkBits2Float(0xc1f33a38), SkBits2Float(0xc1c0fb68), SkBits2Float(0xc200ef83), SkBits2Float(0xc1aae250));
path.quadTo(SkBits2Float(0xc20841e9), SkBits2Float(0xc194c938), SkBits2Float(0xc2153f65), SkBits2Float(0xc18f83d9));
path.quadTo(SkBits2Float(0xc2223ce2), SkBits2Float(0xc18a3e7a), SkBits2Float(0xc22d496e), SkBits2Float(0xc198e348));
path.quadTo(SkBits2Float(0xc23855fb), SkBits2Float(0xc1a78814), SkBits2Float(0xc23af8aa), SkBits2Float(0xc1c1830c));
path.quadTo(SkBits2Float(0xc23d9b5a), SkBits2Float(0xc1db7e06), SkBits2Float(0xc23648f3), SkBits2Float(0xc1f1971e));
path.quadTo(SkBits2Float(0xc22ef68d), SkBits2Float(0xc203d81c), SkBits2Float(0xc221f910), SkBits2Float(0xc2067acc));
path.close();
path.moveTo(SkBits2Float(0x4218d883), SkBits2Float(0xc1dfb2a2));
path.quadTo(SkBits2Float(0x4224b610), SkBits2Float(0xc1eb836c), SkBits2Float(0x4231475d), SkBits2Float(0xc1e31687));
path.quadTo(SkBits2Float(0x423dd8aa), SkBits2Float(0xc1daa9a1), SkBits2Float(0x4243c10e), SkBits2Float(0xc1c2ee88));
path.quadTo(SkBits2Float(0x4249a974), SkBits2Float(0xc1ab336d), SkBits2Float(0x42457300), SkBits2Float(0xc19210d4));
path.quadTo(SkBits2Float(0x42413c8e), SkBits2Float(0xc171dc76), SkBits2Float(0x42355f01), SkBits2Float(0xc15a3ae1));
path.quadTo(SkBits2Float(0x42298174), SkBits2Float(0xc142994c), SkBits2Float(0x421cf027), SkBits2Float(0xc1537318));
path.quadTo(SkBits2Float(0x42105edb), SkBits2Float(0xc1644ce3), SkBits2Float(0x420a7675), SkBits2Float(0xc189e18c));
path.quadTo(SkBits2Float(0x42048e10), SkBits2Float(0xc1a19ca6), SkBits2Float(0x4208c483), SkBits2Float(0xc1babf40));
path.quadTo(SkBits2Float(0x420cfaf6), SkBits2Float(0xc1d3e1d8), SkBits2Float(0x4218d883), SkBits2Float(0xc1dfb2a2));
path.close();
path.moveTo(SkBits2Float(0xc19e6455), SkBits2Float(0xc19e6455));
path.quadTo(SkBits2Float(0xc1399153), SkBits2Float(0xc1e00000), SkBits2Float(0x00000000), SkBits2Float(0xc1e00000));
path.quadTo(SkBits2Float(0x41399153), SkBits2Float(0xc1e00000), SkBits2Float(0x419e6455), SkBits2Float(0xc19e6455));
path.quadTo(SkBits2Float(0x41e00000), SkBits2Float(0xc1399153), SkBits2Float(0x41e00000), SkBits2Float(0x00000000));
path.quadTo(SkBits2Float(0x41e00000), SkBits2Float(0x41399153), SkBits2Float(0x419e6455), SkBits2Float(0x419e6455));
path.quadTo(SkBits2Float(0x415b75ce), SkBits2Float(0x41cf0dc3), SkBits2Float(0x40b878fc), SkBits2Float(0x41db9f74));
path.quadTo(SkBits2Float(0x41000000), SkBits2Float(0x41ee1ba4), SkBits2Float(0x41000000), SkBits2Float(0x42040000));
path.quadTo(SkBits2Float(0x41000000), SkBits2Float(0x4211413d), SkBits2Float(0x40b504f3), SkBits2Float(0x421aa09e));
path.quadTo(SkBits2Float(0x4087d82a), SkBits2Float(0x42204637), SkBits2Float(0x401ecaaa), SkBits2Float(0x422284dc));
path.quadTo(SkBits2Float(0x4033f0a5), SkBits2Float(0x4223bc28), SkBits2Float(0x4047ae10), SkBits2Float(0x4225218a));
path.quadTo(SkBits2Float(0x40aa0f54), SkBits2Float(0x422f1027), SkBits2Float(0x40a38748), SkBits2Float(0x423c4af2));
path.quadTo(SkBits2Float(0x409cff44), SkBits2Float(0x424985be), SkBits2Float(0x401b14b8), SkBits2Float(0x42524cc7));
path.quadTo(SkBits2Float(0xbd754800), SkBits2Float(0x425b13d0), SkBits2Float(0xc05781d0), SkBits2Float(0x425a42cf));
path.quadTo(SkBits2Float(0xc0d59744), SkBits2Float(0x425971cf), SkBits2Float(0xc10de7c8), SkBits2Float(0x424f8332));
path.quadTo(SkBits2Float(0xc13103ee), SkBits2Float(0x42459494), SkBits2Float(0xc12dbfea), SkBits2Float(0x423859c9));
path.quadTo(SkBits2Float(0xc12a7be8), SkBits2Float(0x422b1efe), SkBits2Float(0xc102c172), SkBits2Float(0x422257f4));
path.quadTo(SkBits2Float(0xc0dbff18), SkBits2Float(0x421dc1e9), SkBits2Float(0xc0ab47a0), SkBits2Float(0x421bca58));
path.quadTo(SkBits2Float(0xc0ad79af), SkBits2Float(0x421b8a4a), SkBits2Float(0xc0afa610), SkBits2Float(0x421b4830));
path.quadTo(SkBits2Float(0xc0b25ad5), SkBits2Float(0x421af5e2), SkBits2Float(0xc0b504f3), SkBits2Float(0x421aa09e));
path.quadTo(SkBits2Float(0xc1000000), SkBits2Float(0x4211413d), SkBits2Float(0xc1000000), SkBits2Float(0x42040000));
path.quadTo(SkBits2Float(0xc1000000), SkBits2Float(0x41ee1ba4), SkBits2Float(0xc0b878fc), SkBits2Float(0x41db9f74));
path.quadTo(SkBits2Float(0xc15b75ce), SkBits2Float(0x41cf0dc3), SkBits2Float(0xc19e6455), SkBits2Float(0x419e6455));
path.quadTo(SkBits2Float(0xc1e00000), SkBits2Float(0x41399153), SkBits2Float(0xc1e00000), SkBits2Float(0x00000000));
path.quadTo(SkBits2Float(0xc1e00000), SkBits2Float(0xc1399153), SkBits2Float(0xc19e6455), SkBits2Float(0xc19e6455));
path.close();
path.moveTo(SkBits2Float(0xc23c98fa), SkBits2Float(0x408b3eec));
path.quadTo(SkBits2Float(0xc22fcb5c), SkBits2Float(0x405f9a18), SkBits2Float(0xc22450bb), SkBits2Float(0x40a4d200));
path.quadTo(SkBits2Float(0xc218d61a), SkBits2Float(0x40d9d6f4), SkBits2Float(0xc21567dd), SkBits2Float(0x412021ef));
path.quadTo(SkBits2Float(0xc211f9a2), SkBits2Float(0x41535866), SkBits2Float(0xc2189a40), SkBits2Float(0x4180a176));
path.quadTo(SkBits2Float(0xc21f3ade), SkBits2Float(0x419796b9), SkBits2Float(0xc22c087c), SkBits2Float(0x419e7330));
path.quadTo(SkBits2Float(0xc238d61a), SkBits2Float(0x41a54fa9), SkBits2Float(0xc24450bb), SkBits2Float(0x41980e6c));
path.quadTo(SkBits2Float(0xc24fcb5c), SkBits2Float(0x418acd2f), SkBits2Float(0xc2533998), SkBits2Float(0x416263e8));
path.quadTo(SkBits2Float(0xc256a7d4), SkBits2Float(0x412f2d72), SkBits2Float(0xc2500736), SkBits2Float(0x410142ec));
path.quadTo(SkBits2Float(0xc2496698), SkBits2Float(0x40a6b0cc), SkBits2Float(0xc23c98fa), SkBits2Float(0x408b3eec));
path.close();
path.moveTo(SkBits2Float(0x41dabec3), SkBits2Float(0x41dabec3));
path.quadTo(SkBits2Float(0x41ed7d86), SkBits2Float(0x41c80000), SkBits2Float(0x42040000), SkBits2Float(0x41c80000));
path.quadTo(SkBits2Float(0x4211413d), SkBits2Float(0x41c80000), SkBits2Float(0x421aa09e), SkBits2Float(0x41dabec3));
path.quadTo(SkBits2Float(0x42240000), SkBits2Float(0x41ed7d86), SkBits2Float(0x42240000), SkBits2Float(0x42040000));
path.quadTo(SkBits2Float(0x42240000), SkBits2Float(0x4211413d), SkBits2Float(0x421aa09e), SkBits2Float(0x421aa09e));
path.quadTo(SkBits2Float(0x4211413d), SkBits2Float(0x42240000), SkBits2Float(0x42040000), SkBits2Float(0x42240000));
path.quadTo(SkBits2Float(0x41ed7d86), SkBits2Float(0x42240000), SkBits2Float(0x41dabec3), SkBits2Float(0x421aa09e));
path.quadTo(SkBits2Float(0x41c80000), SkBits2Float(0x4211413d), SkBits2Float(0x41c80000), SkBits2Float(0x42040000));
path.quadTo(SkBits2Float(0x41c80000), SkBits2Float(0x41ed7d86), SkBits2Float(0x41dabec3), SkBits2Float(0x41dabec3));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x401b14b8), SkBits2Float(0x42524cc7));
path.quadTo(SkBits2Float(0xbd754800), SkBits2Float(0x425b13d0), SkBits2Float(0xc05781d0), SkBits2Float(0x425a42cf));
path.quadTo(SkBits2Float(0xc0d59744), SkBits2Float(0x425971cf), SkBits2Float(0xc10de7c8), SkBits2Float(0x424f8332));
path.quadTo(SkBits2Float(0xc13103ee), SkBits2Float(0x42459494), SkBits2Float(0xc12dbfea), SkBits2Float(0x423859c9));
path.quadTo(SkBits2Float(0xc12a7be8), SkBits2Float(0x422b1efe), SkBits2Float(0xc102c172), SkBits2Float(0x422257f4));
path.quadTo(SkBits2Float(0xc0b60dfc), SkBits2Float(0x421990ea), SkBits2Float(0xc0186f48), SkBits2Float(0x421a61ec));
path.quadTo(SkBits2Float(0x3f6cf5e0), SkBits2Float(0x421b32ec), SkBits2Float(0x4047ae10), SkBits2Float(0x4225218a));
path.quadTo(SkBits2Float(0x40aa0f54), SkBits2Float(0x422f1027), SkBits2Float(0x40a38748), SkBits2Float(0x423c4af2));
path.quadTo(SkBits2Float(0x409cff44), SkBits2Float(0x424985be), SkBits2Float(0x401b14b8), SkBits2Float(0x42524cc7));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}

static void fuzz763_4713(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x40f7bc68), SkBits2Float(0xc2503bb0));
path.quadTo(SkBits2Float(0x41299c84), SkBits2Float(0xc256ef4e), SkBits2Float(0x415ce976), SkBits2Float(0xc2539652));
path.quadTo(SkBits2Float(0x41881b33), SkBits2Float(0xc2503d58), SkBits2Float(0x41958271), SkBits2Float(0xc244cdc4));
path.quadTo(SkBits2Float(0x41a2e9af), SkBits2Float(0xc2395e30), SkBits2Float(0x419c37b8), SkBits2Float(0xc22c8af3));
path.quadTo(SkBits2Float(0x419585c2), SkBits2Float(0xc21fb7b7), SkBits2Float(0x417d4d34), SkBits2Float(0xc2190418));
path.quadTo(SkBits2Float(0x414f8ee4), SkBits2Float(0xc2125079), SkBits2Float(0x411c41f2), SkBits2Float(0xc215a974));
path.quadTo(SkBits2Float(0x40d1ea00), SkBits2Float(0xc2190270), SkBits2Float(0x409c4d08), SkBits2Float(0xc2247204));
path.quadTo(SkBits2Float(0x404d6020), SkBits2Float(0xc22fe198), SkBits2Float(0x408177f0), SkBits2Float(0xc23cb4d4));
path.quadTo(SkBits2Float(0x409c3fc8), SkBits2Float(0xc2498810), SkBits2Float(0x40f7bc68), SkBits2Float(0xc2503bb0));
path.close();
path.moveTo(SkBits2Float(0xc20487d4), SkBits2Float(0xc2239250));
path.quadTo(SkBits2Float(0xc1ee8d37), SkBits2Float(0xc2239d4e), SkBits2Float(0xc1dbbeef), SkBits2Float(0xc21a45b5));
path.quadTo(SkBits2Float(0xc1c8f0a7), SkBits2Float(0xc210ee1d), SkBits2Float(0xc1c8daab), SkBits2Float(0xc203ace5));
path.quadTo(SkBits2Float(0xc1c8c4af), SkBits2Float(0xc1ecd758), SkBits2Float(0xc1db73e0), SkBits2Float(0xc1da0910));
path.quadTo(SkBits2Float(0xc1ee2310), SkBits2Float(0xc1c73ac7), SkBits2Float(0xc20452c1), SkBits2Float(0xc1c724cb));
path.quadTo(SkBits2Float(0xc21193f9), SkBits2Float(0xc1c70ecf), SkBits2Float(0xc21afb1d), SkBits2Float(0xc1d9be01));
path.quadTo(SkBits2Float(0xc2246242), SkBits2Float(0xc1ec6d31), SkBits2Float(0xc2246d40), SkBits2Float(0xc20377d2));
path.quadTo(SkBits2Float(0xc224783e), SkBits2Float(0xc210b90a), SkBits2Float(0xc21b20a5), SkBits2Float(0xc21a202d));
path.quadTo(SkBits2Float(0xc211c90c), SkBits2Float(0xc2238752), SkBits2Float(0xc20487d4), SkBits2Float(0xc2239250));
path.close();
path.moveTo(SkBits2Float(0xc19e6455), SkBits2Float(0xc19e6455));
path.quadTo(SkBits2Float(0xc1399153), SkBits2Float(0xc1e00000), SkBits2Float(0x00000000), SkBits2Float(0xc1e00000));
path.quadTo(SkBits2Float(0x41399153), SkBits2Float(0xc1e00000), SkBits2Float(0x419e6455), SkBits2Float(0xc19e6455));
path.quadTo(SkBits2Float(0x41e00000), SkBits2Float(0xc1399153), SkBits2Float(0x41e00000), SkBits2Float(0x00000000));
path.quadTo(SkBits2Float(0x41e00000), SkBits2Float(0x41399153), SkBits2Float(0x419e6455), SkBits2Float(0x419e6455));
path.quadTo(SkBits2Float(0x415b75ce), SkBits2Float(0x41cf0dc3), SkBits2Float(0x40b878fc), SkBits2Float(0x41db9f74));
path.quadTo(SkBits2Float(0x41000000), SkBits2Float(0x41ee1ba4), SkBits2Float(0x41000000), SkBits2Float(0x42040000));
path.quadTo(SkBits2Float(0x41000000), SkBits2Float(0x4211413d), SkBits2Float(0x40b504f3), SkBits2Float(0x421aa09e));
path.quadTo(SkBits2Float(0x405413cd), SkBits2Float(0x42240000), SkBits2Float(0x00000000), SkBits2Float(0x42240000));
path.quadTo(SkBits2Float(0xc05413cd), SkBits2Float(0x42240000), SkBits2Float(0xc0b504f3), SkBits2Float(0x421aa09e));
path.quadTo(SkBits2Float(0xc1000000), SkBits2Float(0x4211413d), SkBits2Float(0xc1000000), SkBits2Float(0x42040000));
path.quadTo(SkBits2Float(0xc1000000), SkBits2Float(0x41ee1ba4), SkBits2Float(0xc0b878fc), SkBits2Float(0x41db9f74));
path.quadTo(SkBits2Float(0xc15b75ce), SkBits2Float(0x41cf0dc3), SkBits2Float(0xc19e6455), SkBits2Float(0x419e6455));
path.quadTo(SkBits2Float(0xc1e00000), SkBits2Float(0x41399153), SkBits2Float(0xc1e00000), SkBits2Float(0x00000000));
path.quadTo(SkBits2Float(0xc1e00000), SkBits2Float(0xc1399153), SkBits2Float(0xc19e6455), SkBits2Float(0xc19e6455));
path.close();
path.moveTo(SkBits2Float(0xc23c7b18), SkBits2Float(0x40950470));
path.quadTo(SkBits2Float(0xc22fb33d), SkBits2Float(0x4071d1b0), SkBits2Float(0xc2242dae), SkBits2Float(0x40ad5534));
path.quadTo(SkBits2Float(0xc218a81f), SkBits2Float(0x40e1c194), SkBits2Float(0xc21524ab), SkBits2Float(0x41240037));
path.quadTo(SkBits2Float(0xc211a138), SkBits2Float(0x41571fa5), SkBits2Float(0xc2182ec4), SkBits2Float(0x41829af0));
path.quadTo(SkBits2Float(0xc21ebc50), SkBits2Float(0x4199a610), SkBits2Float(0xc22b842b), SkBits2Float(0x41a0acf4));
path.quadTo(SkBits2Float(0xc2384c07), SkBits2Float(0x41a7b3dc), SkBits2Float(0xc243d196), SkBits2Float(0x419a98c4));
path.quadTo(SkBits2Float(0xc24f5726), SkBits2Float(0x418d7dad), SkBits2Float(0xc252da98), SkBits2Float(0x4167dbea));
path.quadTo(SkBits2Float(0xc2565e0c), SkBits2Float(0x4134bc7e), SkBits2Float(0xc24fd080), SkBits2Float(0x4106a640));
path.quadTo(SkBits2Float(0xc24942f4), SkBits2Float(0x40b12008), SkBits2Float(0xc23c7b18), SkBits2Float(0x40950470));
path.close();
path.moveTo(SkBits2Float(0x4204f72e), SkBits2Float(0x41c56cd2));
path.quadTo(SkBits2Float(0x42123842), SkBits2Float(0x41c52adf), SkBits2Float(0x421baed7), SkBits2Float(0x41d7bac6));
path.quadTo(SkBits2Float(0x4225256d), SkBits2Float(0x41ea4aad), SkBits2Float(0x42254667), SkBits2Float(0x4202666b));
path.quadTo(SkBits2Float(0x42256760), SkBits2Float(0x420fa77f), SkBits2Float(0x421c1f6c), SkBits2Float(0x42191e14));
path.quadTo(SkBits2Float(0x421bff97), SkBits2Float(0x42193e89), SkBits2Float(0x421bdf6b), SkBits2Float(0x42195eb8));
path.quadTo(SkBits2Float(0x421bbff6), SkBits2Float(0x42197f32), SkBits2Float(0x421ba03b), SkBits2Float(0x42199f57));
path.quadTo(SkBits2Float(0x421b605e), SkBits2Float(0x4219e00a), SkBits2Float(0x421b1fa8), SkBits2Float(0x421a1f22));
path.quadTo(SkBits2Float(0x421ae0f1), SkBits2Float(0x421a604b), SkBits2Float(0x421aa09e), SkBits2Float(0x421aa09e));
path.quadTo(SkBits2Float(0x4211413d), SkBits2Float(0x42240000), SkBits2Float(0x42040000), SkBits2Float(0x42240000));
path.quadTo(SkBits2Float(0x41ed7d86), SkBits2Float(0x42240000), SkBits2Float(0x41dabec3), SkBits2Float(0x421aa09e));
path.quadTo(SkBits2Float(0x41c80000), SkBits2Float(0x4211413d), SkBits2Float(0x41c80000), SkBits2Float(0x42040000));
path.quadTo(SkBits2Float(0x41c80000), SkBits2Float(0x41ed7d86), SkBits2Float(0x41dabec3), SkBits2Float(0x41dabec3));
path.quadTo(SkBits2Float(0x41db19b1), SkBits2Float(0x41da63d5), SkBits2Float(0x41db755b), SkBits2Float(0x41da0a9b));
path.quadTo(SkBits2Float(0x41dbce01), SkBits2Float(0x41d9ae59), SkBits2Float(0x41dc285e), SkBits2Float(0x41d952ce));
path.quadTo(SkBits2Float(0x41dc55b6), SkBits2Float(0x41d924df), SkBits2Float(0x41dc82cd), SkBits2Float(0x41d8f7cd));
path.quadTo(SkBits2Float(0x41dcaf1e), SkBits2Float(0x41d8ca01), SkBits2Float(0x41dcdc4c), SkBits2Float(0x41d89bf0));
path.quadTo(SkBits2Float(0x41ef6c33), SkBits2Float(0x41c5aec5), SkBits2Float(0x4204f72e), SkBits2Float(0x41c56cd2));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x42240000), SkBits2Float(0x42040000));
path.quadTo(SkBits2Float(0x42240000), SkBits2Float(0x4211413d), SkBits2Float(0x421aa09e), SkBits2Float(0x421aa09e));
path.quadTo(SkBits2Float(0x4211413d), SkBits2Float(0x42240000), SkBits2Float(0x42040000), SkBits2Float(0x42240000));
path.quadTo(SkBits2Float(0x41ed7d86), SkBits2Float(0x42240000), SkBits2Float(0x41dabec3), SkBits2Float(0x421aa09e));
path.quadTo(SkBits2Float(0x41c80000), SkBits2Float(0x4211413d), SkBits2Float(0x41c80000), SkBits2Float(0x42040000));
path.quadTo(SkBits2Float(0x41c80000), SkBits2Float(0x41ed7d86), SkBits2Float(0x41dabec3), SkBits2Float(0x41dabec3));
path.quadTo(SkBits2Float(0x41ed7d86), SkBits2Float(0x41c80000), SkBits2Float(0x42040000), SkBits2Float(0x41c80000));
path.quadTo(SkBits2Float(0x4211413d), SkBits2Float(0x41c80000), SkBits2Float(0x421aa09e), SkBits2Float(0x41dabec3));
path.quadTo(SkBits2Float(0x42240000), SkBits2Float(0x41ed7d86), SkBits2Float(0x42240000), SkBits2Float(0x42040000));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}

static void fuzz763_24588(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x413a5194), SkBits2Float(0xc24d4e33));  // 11.6449f, -51.3264f
path.quadTo(SkBits2Float(0x4169f3fc), SkBits2Float(0xc2532032), SkBits2Float(0x418e0c8b), SkBits2Float(0xc24ed218));  // 14.6221f, -52.7814f, 17.7561f, -51.7052f
path.quadTo(SkBits2Float(0x41a71f17), SkBits2Float(0xc24a83ff), SkBits2Float(0x41b2c316), SkBits2Float(0xc23e9b65));  // 20.8902f, -50.6289f, 22.3453f, -47.6518f
path.quadTo(SkBits2Float(0x41be6714), SkBits2Float(0xc232b2cb), SkBits2Float(0x41b5cae0), SkBits2Float(0xc2262985));  // 23.8003f, -44.6746f, 22.7241f, -41.5405f
path.quadTo(SkBits2Float(0x41ad2ead), SkBits2Float(0xc219a03f), SkBits2Float(0x41955d79), SkBits2Float(0xc213ce40));  // 21.6478f, -38.4065f, 18.6706f, -36.9514f
path.quadTo(SkBits2Float(0x417b188a), SkBits2Float(0xc20dfc40), SkBits2Float(0x4148f373), SkBits2Float(0xc2124a5a));  // 15.6935f, -35.4963f, 12.5594f, -36.5726f
path.quadTo(SkBits2Float(0x4116ce5a), SkBits2Float(0xc2169874), SkBits2Float(0x40ff0cba), SkBits2Float(0xc222810e));  // 9.42538f, -37.6489f, 7.9703f, -40.626f
path.quadTo(SkBits2Float(0x40d07cc0), SkBits2Float(0xc22e69a8), SkBits2Float(0x40f2ed90), SkBits2Float(0xc23af2ee));  // 6.51523f, -43.6032f, 7.5915f, -46.7372f
path.quadTo(SkBits2Float(0x410aaf2c), SkBits2Float(0xc2477c34), SkBits2Float(0x413a5194), SkBits2Float(0xc24d4e33));  // 8.66777f, -49.8713f, 11.6449f, -51.3264f
path.close();
path.moveTo(SkBits2Float(0xc19e6455), SkBits2Float(0xc19e6455));  // -19.799f, -19.799f
path.quadTo(SkBits2Float(0xc1399153), SkBits2Float(0xc1e00000), SkBits2Float(0x00000000), SkBits2Float(0xc1e00000));  // -11.598f, -28, 0, -28
path.quadTo(SkBits2Float(0x41399153), SkBits2Float(0xc1e00000), SkBits2Float(0x419e6455), SkBits2Float(0xc19e6455));  // 11.598f, -28, 19.799f, -19.799f
path.quadTo(SkBits2Float(0x41e00000), SkBits2Float(0xc1399153), SkBits2Float(0x41e00000), SkBits2Float(0x00000000));  // 28, -11.598f, 28, 0
path.quadTo(SkBits2Float(0x41e00000), SkBits2Float(0x41399153), SkBits2Float(0x419e6455), SkBits2Float(0x419e6455));  // 28, 11.598f, 19.799f, 19.799f
path.quadTo(SkBits2Float(0x415b75ce), SkBits2Float(0x41cf0dc3), SkBits2Float(0x40b878fc), SkBits2Float(0x41db9f74));  // 13.7163f, 25.8817f, 5.76477f, 27.4529f
path.quadTo(SkBits2Float(0x41000000), SkBits2Float(0x41ee1ba4), SkBits2Float(0x41000000), SkBits2Float(0x42040000));  // 8, 29.7635f, 8, 33
path.quadTo(SkBits2Float(0x41000000), SkBits2Float(0x4211413d), SkBits2Float(0x40b504f3), SkBits2Float(0x421aa09e));  // 8, 36.3137f, 5.65685f, 38.6569f
path.quadTo(SkBits2Float(0x405413cd), SkBits2Float(0x42240000), SkBits2Float(0x00000000), SkBits2Float(0x42240000));  // 3.31371f, 41, 0, 41
path.quadTo(SkBits2Float(0xc05413cd), SkBits2Float(0x42240000), SkBits2Float(0xc0b504f3), SkBits2Float(0x421aa09e));  // -3.31371f, 41, -5.65685f, 38.6569f
path.quadTo(SkBits2Float(0xc1000000), SkBits2Float(0x4211413d), SkBits2Float(0xc1000000), SkBits2Float(0x42040000));  // -8, 36.3137f, -8, 33
path.quadTo(SkBits2Float(0xc1000000), SkBits2Float(0x41ee1ba4), SkBits2Float(0xc0b878fc), SkBits2Float(0x41db9f74));  // -8, 29.7635f, -5.76477f, 27.4529f
path.quadTo(SkBits2Float(0xc15b75ce), SkBits2Float(0x41cf0dc3), SkBits2Float(0xc19e6455), SkBits2Float(0x419e6455));  // -13.7163f, 25.8817f, -19.799f, 19.799f
path.quadTo(SkBits2Float(0xc1e00000), SkBits2Float(0x41399153), SkBits2Float(0xc1e00000), SkBits2Float(0x00000000));  // -28, 11.598f, -28, 0
path.quadTo(SkBits2Float(0xc1e00000), SkBits2Float(0xc1399153), SkBits2Float(0xc19e6455), SkBits2Float(0xc19e6455));  // -28, -11.598f, -19.799f, -19.799f
path.close();
path.moveTo(SkBits2Float(0xc23d594a), SkBits2Float(0x3f8b9aa0));  // -47.3372f, 1.09066f
path.quadTo(SkBits2Float(0xc23056ee), SkBits2Float(0x3ee95200), SkBits2Float(0xc2255841), SkBits2Float(0x40139cf0));  // -44.0849f, 0.455704f, -41.3362f, 2.30645f
path.quadTo(SkBits2Float(0xc21a5994), SkBits2Float(0x408507d0), SkBits2Float(0xc217cf63), SkBits2Float(0x40ed1ab6));  // -38.5875f, 4.1572f, -37.9525f, 7.40951f
path.quadTo(SkBits2Float(0xc21747fe), SkBits2Float(0x41016369), SkBits2Float(0xc2172ef9), SkBits2Float(0x410bdff7));  // -37.8203f, 8.08677f, -37.7959f, 8.74218f
path.quadTo(SkBits2Float(0xc2161ebf), SkBits2Float(0x411577cf), SkBits2Float(0xc21567dd), SkBits2Float(0x412021ef));  // -37.53f, 9.34175f, -37.3514f, 10.0083f
path.quadTo(SkBits2Float(0xc211f9a2), SkBits2Float(0x41535866), SkBits2Float(0xc2189a40), SkBits2Float(0x4180a176));  // -36.4938f, 13.2091f, -38.1506f, 16.0788f
path.quadTo(SkBits2Float(0xc21f3ade), SkBits2Float(0x419796b9), SkBits2Float(0xc22c087c), SkBits2Float(0x419e7330));  // -39.8075f, 18.9486f, -43.0083f, 19.8062f
path.quadTo(SkBits2Float(0xc238d61a), SkBits2Float(0x41a54fa9), SkBits2Float(0xc24450bb), SkBits2Float(0x41980e6c));  // -46.2091f, 20.6639f, -49.0788f, 19.007f
path.quadTo(SkBits2Float(0xc24fcb5c), SkBits2Float(0x418acd2f), SkBits2Float(0xc2533998), SkBits2Float(0x416263e8));  // -51.9486f, 17.3502f, -52.8062f, 14.1494f
path.quadTo(SkBits2Float(0xc2543df8), SkBits2Float(0x415334fe), SkBits2Float(0xc2546005), SkBits2Float(0x41447d40));  // -53.0605f, 13.2004f, -53.0938f, 12.2806f
path.quadTo(SkBits2Float(0xc255df09), SkBits2Float(0x41370862), SkBits2Float(0xc2569fcc), SkBits2Float(0x41279af0));  // -53.4678f, 11.4395f, -53.6561f, 10.4753f
path.quadTo(SkBits2Float(0xc25929fe), SkBits2Float(0x40e722fc), SkBits2Float(0xc251c2d2), SkBits2Float(0x408f2d94));  // -54.291f, 7.22302f, -52.4403f, 4.47431f
path.quadTo(SkBits2Float(0xc24a5ba8), SkBits2Float(0x3fdce0a0), SkBits2Float(0xc23d594a), SkBits2Float(0x3f8b9aa0));  // -50.5895f, 1.72561f, -47.3372f, 1.09066f
path.close();
path.moveTo(SkBits2Float(0xc18b14a2), SkBits2Float(0x42164b25));  // -17.3851f, 37.5734f
path.quadTo(SkBits2Float(0xc1675bab), SkBits2Float(0x421010e4), SkBits2Float(0xc134a62c), SkBits2Float(0x4213efa5));  // -14.4599f, 36.0165f, -11.2906f, 36.984f
path.quadTo(SkBits2Float(0xc101f0aa), SkBits2Float(0x4217ce66), SkBits2Float(0xc0d20f46), SkBits2Float(0x422381cc));  // -8.12126f, 37.9516f, -6.56436f, 40.8768f
path.quadTo(SkBits2Float(0xc0a03d38), SkBits2Float(0x422f3532), SkBits2Float(0xc0bf3344), SkBits2Float(0x423be292));  // -5.00747f, 43.8019f, -5.97501f, 46.9713f
path.quadTo(SkBits2Float(0xc0de294c), SkBits2Float(0x42488ff2), SkBits2Float(0xc11de23e), SkBits2Float(0x424eca34));  // -6.94254f, 50.1406f, -9.86773f, 51.6975f
path.quadTo(SkBits2Float(0xc14cafd4), SkBits2Float(0x42550476), SkBits2Float(0xc17f6556), SkBits2Float(0x425125b4));  // -12.7929f, 53.2544f, -15.9622f, 52.2868f
path.quadTo(SkBits2Float(0xc1990d6c), SkBits2Float(0x424d46f3), SkBits2Float(0xc1a581f0), SkBits2Float(0x4241938e));  // -19.1316f, 51.3193f, -20.6884f, 48.3941f
path.quadTo(SkBits2Float(0xc1b1f673), SkBits2Float(0x4235e028), SkBits2Float(0xc1aa38f0), SkBits2Float(0x422932c8));  // -22.2453f, 45.4689f, -21.2778f, 42.2996f
path.quadTo(SkBits2Float(0xc1a27b6c), SkBits2Float(0x421c8567), SkBits2Float(0xc18b14a2), SkBits2Float(0x42164b25));  // -20.3103f, 39.1303f, -17.3851f, 37.5734f
path.close();
path.moveTo(SkBits2Float(0x41dabec3), SkBits2Float(0x41dabec3));  // 27.3431f, 27.3431f
path.quadTo(SkBits2Float(0x41ed7d86), SkBits2Float(0x41c80000), SkBits2Float(0x42040000), SkBits2Float(0x41c80000));  // 29.6863f, 25, 33, 25
path.quadTo(SkBits2Float(0x4211413d), SkBits2Float(0x41c80000), SkBits2Float(0x421aa09e), SkBits2Float(0x41dabec3));  // 36.3137f, 25, 38.6569f, 27.3431f
path.quadTo(SkBits2Float(0x42240000), SkBits2Float(0x41ed7d86), SkBits2Float(0x42240000), SkBits2Float(0x42040000));  // 41, 29.6863f, 41, 33
path.quadTo(SkBits2Float(0x42240000), SkBits2Float(0x4211413d), SkBits2Float(0x421aa09e), SkBits2Float(0x421aa09e));  // 41, 36.3137f, 38.6569f, 38.6569f
path.quadTo(SkBits2Float(0x4217d943), SkBits2Float(0x421d67f9), SkBits2Float(0x4214ba8d), SkBits2Float(0x421f5c6e));  // 37.9622f, 39.3515f, 37.1822f, 39.8403f
path.quadTo(SkBits2Float(0x42129039), SkBits2Float(0x422256bd), SkBits2Float(0x420f9986), SkBits2Float(0x4224eb5c));  // 36.6408f, 40.5847f, 35.8999f, 41.2298f
path.quadTo(SkBits2Float(0x420e25cf), SkBits2Float(0x42262f05), SkBits2Float(0x420ca0a4), SkBits2Float(0x42273ec0));  // 35.5369f, 41.5459f, 35.1569f, 41.8113f
path.quadTo(SkBits2Float(0x420b5228), SkBits2Float(0x42288f80), SkBits2Float(0x4209d382), SkBits2Float(0x4229c624));  // 34.8302f, 42.1401f, 34.4566f, 42.4435f
path.quadTo(SkBits2Float(0x41ff1232), SkBits2Float(0x423220d2), SkBits2Float(0x41e4b406), SkBits2Float(0x4230c247));  // 31.8839f, 44.5321f, 28.5879f, 44.1897f
path.quadTo(SkBits2Float(0x41ca55dc), SkBits2Float(0x422f63bd), SkBits2Float(0x41b9a084), SkBits2Float(0x42251952));  // 25.2919f, 43.8474f, 23.2034f, 41.2747f
path.quadTo(SkBits2Float(0x41a8eb2b), SkBits2Float(0x421acee9), SkBits2Float(0x41aba840), SkBits2Float(0x420d9fd3));  // 21.1148f, 38.7021f, 21.4572f, 35.4061f
path.quadTo(SkBits2Float(0x41ae6555), SkBits2Float(0x420070be), SkBits2Float(0x41c2fa28), SkBits2Float(0x41f02c24));  // 21.7995f, 32.1101f, 24.3721f, 30.0216f
path.quadTo(SkBits2Float(0x41c514db), SkBits2Float(0x41ee76d0), SkBits2Float(0x41c73f0f), SkBits2Float(0x41ecf584));  // 24.6352f, 29.808f, 24.9058f, 29.6199f
path.quadTo(SkBits2Float(0x41c919c1), SkBits2Float(0x41eb15ab), SkBits2Float(0x41cb250b), SkBits2Float(0x41e94e07));  // 25.1376f, 29.3856f, 25.3931f, 29.1631f
path.quadTo(SkBits2Float(0x41cf4ed6), SkBits2Float(0x41e5ae04), SkBits2Float(0x41d3c048), SkBits2Float(0x41e2e387));  // 25.9135f, 28.71f, 26.4689f, 28.3611f
path.quadTo(SkBits2Float(0x41d4d649), SkBits2Float(0x41e1661f), SkBits2Float(0x41d605fb), SkBits2Float(0x41dff35b));  // 26.6046f, 28.1749f, 26.7529f, 27.9938f
path.quadTo(SkBits2Float(0x41d7094e), SkBits2Float(0x41deb6c2), SkBits2Float(0x41d81f4d), SkBits2Float(0x41dd81fd));  // 26.8795f, 27.8392f, 27.0153f, 27.6885f
path.lineTo(SkBits2Float(0x41d81f53), SkBits2Float(0x41dd81f7));  // 27.0153f, 27.6885f
path.quadTo(SkBits2Float(0x41d96269), SkBits2Float(0x41dc1b1d), SkBits2Float(0x41dabec3), SkBits2Float(0x41dabec3));  // 27.1731f, 27.5132f, 27.3431f, 27.3431f
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x421a8288), SkBits2Float(0x420efdef));  // 38.6275f, 35.748f
path.quadTo(SkBits2Float(0x4219989b), SkBits2Float(0x421c3719), SkBits2Float(0x420f9986), SkBits2Float(0x4224eb5c));  // 38.399f, 39.0538f, 35.8999f, 41.2298f
path.quadTo(SkBits2Float(0x42059a71), SkBits2Float(0x422d9f9f), SkBits2Float(0x41f0c28e), SkBits2Float(0x422cb5b2));  // 33.4008f, 43.4059f, 30.095f, 43.1774f
path.quadTo(SkBits2Float(0x41d65038), SkBits2Float(0x422bcbc5), SkBits2Float(0x41c4e7b3), SkBits2Float(0x4221ccb0));  // 26.7892f, 42.949f, 24.6131f, 40.4499f
path.quadTo(SkBits2Float(0x41b37f2c), SkBits2Float(0x4217cd9b), SkBits2Float(0x41b55306), SkBits2Float(0x420a9471));  // 22.4371f, 37.9508f, 22.6655f, 34.645f
path.quadTo(SkBits2Float(0x41b726e0), SkBits2Float(0x41fab68c), SkBits2Float(0x41cb250b), SkBits2Float(0x41e94e07));  // 22.894f, 31.3391f, 25.3931f, 29.1631f
path.quadTo(SkBits2Float(0x41df2336), SkBits2Float(0x41d7e580), SkBits2Float(0x41f9958b), SkBits2Float(0x41d9b95a));  // 27.8922f, 26.9871f, 31.198f, 27.2155f
path.quadTo(SkBits2Float(0x420a03ef), SkBits2Float(0x41db8d34), SkBits2Float(0x4212b832), SkBits2Float(0x41ef8b5f));  // 34.5038f, 27.4439f, 36.6799f, 29.9431f
path.quadTo(SkBits2Float(0x421b6c75), SkBits2Float(0x4201c4c5), SkBits2Float(0x421a8288), SkBits2Float(0x420efdef));  // 38.8559f, 32.4422f, 38.6275f, 35.748f
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}

// SkDQuadIntersection.cpp:594: failed assertion "way_roughly_zero(fT[0][index])
static void fuzz763_20016(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x41e88c66), SkBits2Float(0xc22f800b));  // 29.0686f, -43.875f
path.quadTo(SkBits2Float(0x420178e8), SkBits2Float(0xc230b9b9), SkBits2Float(0x420babd1), SkBits2Float(0xc228426b));  // 32.3681f, -44.1814f, 34.9178f, -42.0649f
path.quadTo(SkBits2Float(0x4215debb), SkBits2Float(0xc21fcb1e), SkBits2Float(0x42171869), SkBits2Float(0xc2129869));  // 37.4675f, -39.9484f, 37.7738f, -36.6488f
path.quadTo(SkBits2Float(0x42185217), SkBits2Float(0xc20565b3), SkBits2Float(0x420fdac9), SkBits2Float(0xc1f66594));  // 38.0802f, -33.3493f, 35.9637f, -30.7996f
path.quadTo(SkBits2Float(0x4207637c), SkBits2Float(0xc1e1ffc1), SkBits2Float(0x41f4618e), SkBits2Float(0xc1df8c65));  // 33.8472f, -28.2499f, 30.5476f, -27.9436f
path.quadTo(SkBits2Float(0x41d9fc22), SkBits2Float(0xc1dd190a), SkBits2Float(0x41c59650), SkBits2Float(0xc1ee07a4));  // 27.2481f, -27.6372f, 24.6984f, -29.7537f
path.quadTo(SkBits2Float(0x41b1307c), SkBits2Float(0xc1fef63e), SkBits2Float(0x41aebd21), SkBits2Float(0xc20cadd5));  // 22.1487f, -31.8702f, 21.8423f, -35.1698f
path.quadTo(SkBits2Float(0x41ac49c5), SkBits2Float(0xc219e08a), SkBits2Float(0x41bd3860), SkBits2Float(0xc2241373));  // 21.536f, -38.4693f, 23.6525f, -41.019f
path.quadTo(SkBits2Float(0x41ce26fa), SkBits2Float(0xc22e465d), SkBits2Float(0x41e88c66), SkBits2Float(0xc22f800b));  // 25.769f, -43.5687f, 29.0686f, -43.875f
path.close();
path.moveTo(SkBits2Float(0xc19e6455), SkBits2Float(0xc19e6455));  // -19.799f, -19.799f
path.quadTo(SkBits2Float(0xc1399153), SkBits2Float(0xc1e00000), SkBits2Float(0x00000000), SkBits2Float(0xc1e00000));  // -11.598f, -28, 0, -28
path.quadTo(SkBits2Float(0x41399153), SkBits2Float(0xc1e00000), SkBits2Float(0x419e6455), SkBits2Float(0xc19e6455));  // 11.598f, -28, 19.799f, -19.799f
path.quadTo(SkBits2Float(0x41e00000), SkBits2Float(0xc1399153), SkBits2Float(0x41e00000), SkBits2Float(0x00000000));  // 28, -11.598f, 28, 0
path.quadTo(SkBits2Float(0x41e00000), SkBits2Float(0x41399153), SkBits2Float(0x419e6455), SkBits2Float(0x419e6455));  // 28, 11.598f, 19.799f, 19.799f
path.quadTo(SkBits2Float(0x415b75ce), SkBits2Float(0x41cf0dc3), SkBits2Float(0x40b878fc), SkBits2Float(0x41db9f74));  // 13.7163f, 25.8817f, 5.76477f, 27.4529f
path.quadTo(SkBits2Float(0x41000000), SkBits2Float(0x41ee1ba4), SkBits2Float(0x41000000), SkBits2Float(0x42040000));  // 8, 29.7635f, 8, 33
path.quadTo(SkBits2Float(0x41000000), SkBits2Float(0x42100ef9), SkBits2Float(0x40c1f194), SkBits2Float(0x4218e765));  // 8, 36.0146f, 6.06074f, 38.226f
path.quadTo(SkBits2Float(0x4100db87), SkBits2Float(0x42186e1c), SkBits2Float(0x411cd246), SkBits2Float(0x421bbc51));  // 8.0536f, 38.1075f, 9.80134f, 38.9339f
path.quadTo(SkBits2Float(0x41326be0), SkBits2Float(0x42146b95), SkBits2Float(0x4156b15b), SkBits2Float(0x421110b0));  // 11.1513f, 37.1051f, 13.4183f, 36.2663f
path.quadTo(SkBits2Float(0x41843577), SkBits2Float(0x420c7739), SkBits2Float(0x419c4b3e), SkBits2Float(0x421200ec));  // 16.5261f, 35.1164f, 19.5367f, 36.5009f
path.quadTo(SkBits2Float(0x41b46104), SkBits2Float(0x42178a9f), SkBits2Float(0x41bd93f2), SkBits2Float(0x4223f904));  // 22.5474f, 37.8854f, 23.6972f, 40.9932f
path.quadTo(SkBits2Float(0x41c6c6e0), SkBits2Float(0x42306768), SkBits2Float(0x41bbb37b), SkBits2Float(0x423c724c));  // 24.8471f, 44.101f, 23.4626f, 47.1116f
path.quadTo(SkBits2Float(0x41b0a015), SkBits2Float(0x42487d2f), SkBits2Float(0x4197c34c), SkBits2Float(0x424d16a6));  // 22.0782f, 50.1222f, 18.9704f, 51.2721f
path.quadTo(SkBits2Float(0x417dcd04), SkBits2Float(0x4251b01e), SkBits2Float(0x414da178), SkBits2Float(0x424c266a));  // 15.8626f, 52.422f, 12.8519f, 51.0375f
path.quadTo(SkBits2Float(0x414d992c), SkBits2Float(0x424c2576), SkBits2Float(0x414d90e0), SkBits2Float(0x424c2481));  // 12.8499f, 51.0366f, 12.8479f, 51.0356f
path.quadTo(SkBits2Float(0x414d8b5f), SkBits2Float(0x424c2655), SkBits2Float(0x414d85dc), SkBits2Float(0x424c2828));  // 12.8465f, 51.0374f, 12.8452f, 51.0392f
path.quadTo(SkBits2Float(0x412d952c), SkBits2Float(0x4256bc8e), SkBits2Float(0x40f225c0), SkBits2Float(0x4258923c));  // 10.8489f, 53.6841f, 7.56711f, 54.1428f
path.quadTo(SkBits2Float(0x4089212c), SkBits2Float(0x425a67eb), SkBits2Float(0x3fd1f7e0), SkBits2Float(0x42526bbf));  // 4.2853f, 54.6015f, 1.64038f, 52.6052f
path.quadTo(SkBits2Float(0xbf8094f0), SkBits2Float(0x424a6f94), SkBits2Float(0xbfbb4ab0), SkBits2Float(0x423d4f00));  // -1.00455f, 50.609f, -1.46322f, 47.3271f
path.quadTo(SkBits2Float(0xbff60080), SkBits2Float(0x42302e6e), SkBits2Float(0x3d985000), SkBits2Float(0x42259a07));  // -1.92189f, 44.0453f, 0.0743713f, 41.4004f
path.quadTo(SkBits2Float(0x3e6fb042), SkBits2Float(0x4224c15b), SkBits2Float(0x3ecdd2ea), SkBits2Float(0x4223f703));  // 0.234071f, 41.1888f, 0.402f, 40.9912f
path.quadTo(SkBits2Float(0x3e4fb040), SkBits2Float(0x42240000), SkBits2Float(0x00000000), SkBits2Float(0x42240000));  // 0.202821f, 41, 0, 41
path.quadTo(SkBits2Float(0xc05413cd), SkBits2Float(0x42240000), SkBits2Float(0xc0b504f3), SkBits2Float(0x421aa09e));  // -3.31371f, 41, -5.65685f, 38.6569f
path.quadTo(SkBits2Float(0xc1000000), SkBits2Float(0x4211413d), SkBits2Float(0xc1000000), SkBits2Float(0x42040000));  // -8, 36.3137f, -8, 33
path.quadTo(SkBits2Float(0xc1000000), SkBits2Float(0x41ee1ba4), SkBits2Float(0xc0b878fc), SkBits2Float(0x41db9f74));  // -8, 29.7635f, -5.76477f, 27.4529f
path.quadTo(SkBits2Float(0xc15b75ce), SkBits2Float(0x41cf0dc3), SkBits2Float(0xc19e6455), SkBits2Float(0x419e6455));  // -13.7163f, 25.8817f, -19.799f, 19.799f
path.quadTo(SkBits2Float(0xc1e00000), SkBits2Float(0x41399153), SkBits2Float(0xc1e00000), SkBits2Float(0x00000000));  // -28, 11.598f, -28, 0
path.quadTo(SkBits2Float(0xc1e00000), SkBits2Float(0xc1399153), SkBits2Float(0xc19e6455), SkBits2Float(0xc19e6455));  // -28, -11.598f, -19.799f, -19.799f
path.close();
path.moveTo(SkBits2Float(0xc24a80c4), SkBits2Float(0xc16637fc));  // -50.6257f, -14.3887f
path.quadTo(SkBits2Float(0xc23faf8f), SkBits2Float(0xc1826e03), SkBits2Float(0xc2329eca), SkBits2Float(0xc17bee18));  // -47.9214f, -16.3037f, -44.6551f, -15.7456f
path.quadTo(SkBits2Float(0xc2258e06), SkBits2Float(0xc173002c), SkBits2Float(0xc21de504), SkBits2Float(0xc147bb58));  // -41.3887f, -15.1875f, -39.4736f, -12.4832f
path.quadTo(SkBits2Float(0xc2163c02), SkBits2Float(0xc11c7684), SkBits2Float(0xc218777c), SkBits2Float(0xc0d066e4));  // -37.5586f, -9.77893f, -38.1167f, -6.51256f
path.quadTo(SkBits2Float(0xc21ab2f8), SkBits2Float(0xc04fc188), SkBits2Float(0xc225842d), SkBits2Float(0xbfaa62c0));  // -38.6748f, -3.24619f, -41.3791f, -1.33114f
path.quadTo(SkBits2Float(0xc2305562), SkBits2Float(0x3f157b20), SkBits2Float(0xc23d6626), SkBits2Float(0x3cd38800));  // -44.0834f, 0.58391f, -47.3498f, 0.0258217f
path.quadTo(SkBits2Float(0xc24a76ea), SkBits2Float(0xbf084280), SkBits2Float(0xc2521fed), SkBits2Float(0xc04f23f0));  // -50.6161f, -0.532265f, -52.5312f, -3.23657f
path.quadTo(SkBits2Float(0xc259c8f0), SkBits2Float(0xc0be1ba0), SkBits2Float(0xc2578d74), SkBits2Float(0xc11350e2));  // -54.4462f, -5.94087f, -53.8881f, -9.20725f
path.quadTo(SkBits2Float(0xc25551f9), SkBits2Float(0xc14793f2), SkBits2Float(0xc24a80c4), SkBits2Float(0xc16637fc));  // -53.3301f, -12.4736f, -50.6257f, -14.3887f
path.close();
path.moveTo(SkBits2Float(0xc23c98fa), SkBits2Float(0x408b3eec));  // -47.1494f, 4.35143f
path.quadTo(SkBits2Float(0xc22fcb5c), SkBits2Float(0x405f9a18), SkBits2Float(0xc22450bb), SkBits2Float(0x40a4d200));  // -43.9486f, 3.49378f, -41.0788f, 5.15063f
path.quadTo(SkBits2Float(0xc218d61a), SkBits2Float(0x40d9d6f4), SkBits2Float(0xc21567dd), SkBits2Float(0x412021ef));  // -38.2091f, 6.80749f, -37.3514f, 10.0083f
path.quadTo(SkBits2Float(0xc211f9a2), SkBits2Float(0x41535866), SkBits2Float(0xc2189a40), SkBits2Float(0x4180a176));  // -36.4938f, 13.2091f, -38.1506f, 16.0788f
path.quadTo(SkBits2Float(0xc21f3ade), SkBits2Float(0x419796b9), SkBits2Float(0xc22c087c), SkBits2Float(0x419e7330));  // -39.8075f, 18.9486f, -43.0083f, 19.8062f
path.quadTo(SkBits2Float(0xc238d61a), SkBits2Float(0x41a54fa9), SkBits2Float(0xc24450bb), SkBits2Float(0x41980e6c));  // -46.2091f, 20.6639f, -49.0788f, 19.007f
path.quadTo(SkBits2Float(0xc24fcb5c), SkBits2Float(0x418acd2f), SkBits2Float(0xc2533998), SkBits2Float(0x416263e8));  // -51.9486f, 17.3502f, -52.8062f, 14.1494f
path.quadTo(SkBits2Float(0xc256a7d4), SkBits2Float(0x412f2d72), SkBits2Float(0xc2500736), SkBits2Float(0x410142ec));  // -53.6639f, 10.9486f, -52.007f, 8.07884f
path.quadTo(SkBits2Float(0xc2496698), SkBits2Float(0x40a6b0cc), SkBits2Float(0xc23c98fa), SkBits2Float(0x408b3eec));  // -50.3502f, 5.20908f, -47.1494f, 4.35143f
path.close();
path.moveTo(SkBits2Float(0x41dabec3), SkBits2Float(0x41dabec3));  // 27.3431f, 27.3431f
path.quadTo(SkBits2Float(0x41ed7d86), SkBits2Float(0x41c80000), SkBits2Float(0x42040000), SkBits2Float(0x41c80000));  // 29.6863f, 25, 33, 25
path.quadTo(SkBits2Float(0x4211413d), SkBits2Float(0x41c80000), SkBits2Float(0x421aa09e), SkBits2Float(0x41dabec3));  // 36.3137f, 25, 38.6569f, 27.3431f
path.quadTo(SkBits2Float(0x42240000), SkBits2Float(0x41ed7d86), SkBits2Float(0x42240000), SkBits2Float(0x42040000));  // 41, 29.6863f, 41, 33
path.quadTo(SkBits2Float(0x42240000), SkBits2Float(0x4211413d), SkBits2Float(0x421aa09e), SkBits2Float(0x421aa09e));  // 41, 36.3137f, 38.6569f, 38.6569f
path.quadTo(SkBits2Float(0x4211413d), SkBits2Float(0x42240000), SkBits2Float(0x42040000), SkBits2Float(0x42240000));  // 36.3137f, 41, 33, 41
path.quadTo(SkBits2Float(0x41ed7d86), SkBits2Float(0x42240000), SkBits2Float(0x41dabec3), SkBits2Float(0x421aa09e));  // 29.6863f, 41, 27.3431f, 38.6569f
path.quadTo(SkBits2Float(0x41c80000), SkBits2Float(0x4211413d), SkBits2Float(0x41c80000), SkBits2Float(0x42040000));  // 25, 36.3137f, 25, 33
path.quadTo(SkBits2Float(0x41c80000), SkBits2Float(0x41ed7d86), SkBits2Float(0x41dabec3), SkBits2Float(0x41dabec3));  // 25, 29.6863f, 27.3431f, 27.3431f
path.close();
path.moveTo(SkBits2Float(0xc1beec1e), SkBits2Float(0x4207519a));  // -23.8653f, 33.8297f
path.quadTo(SkBits2Float(0xc1a5a92e), SkBits2Float(0x42034ca6), SkBits2Float(0xc18e1d36), SkBits2Float(0x42096379));  // -20.7076f, 32.8249f, -17.7643f, 34.3471f
path.quadTo(SkBits2Float(0xc16d2279), SkBits2Float(0x420f7a4d), SkBits2Float(0xc15d0ea8), SkBits2Float(0x421c1bc5));  // -14.8209f, 35.8694f, -13.8161f, 39.0271f
path.quadTo(SkBits2Float(0xc14cfad8), SkBits2Float(0x4228bd3d), SkBits2Float(0xc1655627), SkBits2Float(0x42348339));  // -12.8112f, 42.1848f, -14.3335f, 45.1281f
path.quadTo(SkBits2Float(0xc17db174), SkBits2Float(0x42404936), SkBits2Float(0xc1981baa), SkBits2Float(0x42444e2a));  // -15.8558f, 48.0715f, -19.0135f, 49.0763f
path.quadTo(SkBits2Float(0xc1b15e9a), SkBits2Float(0x4248531e), SkBits2Float(0xc1c8ea94), SkBits2Float(0x42423c4a));  // -22.1712f, 50.0812f, -25.1145f, 48.5589f
path.quadTo(SkBits2Float(0xc1e0768c), SkBits2Float(0x423c2577), SkBits2Float(0xc1e88074), SkBits2Float(0x422f83ff));  // -28.0579f, 47.0366f, -29.0627f, 43.8789f
path.quadTo(SkBits2Float(0xc1f08a5c), SkBits2Float(0x4222e287), SkBits2Float(0xc1e45cb6), SkBits2Float(0x42171c8a));  // -30.0676f, 40.7212f, -28.5453f, 37.7779f
path.quadTo(SkBits2Float(0xc1d82f0e), SkBits2Float(0x420b568e), SkBits2Float(0xc1beec1e), SkBits2Float(0x4207519a));  // -27.023f, 34.8345f, -23.8653f, 33.8297f
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x41bbb37b), SkBits2Float(0x423c724c));  // 23.4626f, 47.1116f
path.quadTo(SkBits2Float(0x41b0a015), SkBits2Float(0x42487d2f), SkBits2Float(0x4197c34c), SkBits2Float(0x424d16a6));  // 22.0782f, 50.1222f, 18.9704f, 51.2721f
path.quadTo(SkBits2Float(0x417dcd04), SkBits2Float(0x4251b01e), SkBits2Float(0x414da178), SkBits2Float(0x424c266a));  // 15.8626f, 52.422f, 12.8519f, 51.0375f
path.quadTo(SkBits2Float(0x411d75ea), SkBits2Float(0x42469cb8), SkBits2Float(0x410b100e), SkBits2Float(0x423a2e53));  // 9.84129f, 49.653f, 8.69142f, 46.5452f
path.quadTo(SkBits2Float(0x40f15460), SkBits2Float(0x422dbfee), SkBits2Float(0x410ed0fc), SkBits2Float(0x4221b50b));  // 7.54155f, 43.4374f, 8.92602f, 40.4268f
path.quadTo(SkBits2Float(0x4124f7c7), SkBits2Float(0x4215aa28), SkBits2Float(0x4156b15b), SkBits2Float(0x421110b0));  // 10.3105f, 37.4162f, 13.4183f, 36.2663f
path.quadTo(SkBits2Float(0x41843577), SkBits2Float(0x420c7739), SkBits2Float(0x419c4b3e), SkBits2Float(0x421200ec));  // 16.5261f, 35.1164f, 19.5367f, 36.5009f
path.quadTo(SkBits2Float(0x41b46104), SkBits2Float(0x42178a9f), SkBits2Float(0x41bd93f2), SkBits2Float(0x4223f904));  // 22.5474f, 37.8854f, 23.6972f, 40.9932f
path.quadTo(SkBits2Float(0x41c6c6e0), SkBits2Float(0x42306768), SkBits2Float(0x41bbb37b), SkBits2Float(0x423c724c));  // 24.8471f, 44.101f, 23.4626f, 47.1116f
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}

//SkOpSegment.cpp:3475: failed assertion "firstAngle"
static void fuzz763_17370(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x41fb8980), SkBits2Float(0xc20d9cf4));  // 31.4421f, -35.4033f
path.quadTo(SkBits2Float(0x42081e43), SkBits2Float(0xc215e4e6), SkBits2Float(0x42154ac6), SkBits2Float(0xc2146e6f));  // 34.0296f, -37.4735f, 37.323f, -37.1078f
path.quadTo(SkBits2Float(0x4222774a), SkBits2Float(0xc212f7f8), SkBits2Float(0x422abf3a), SkBits2Float(0xc2089e76));  // 40.6165f, -36.7422f, 42.6867f, -34.1547f
path.quadTo(SkBits2Float(0x4233072c), SkBits2Float(0xc1fc89e5), SkBits2Float(0x423190b5), SkBits2Float(0xc1e230df));  // 44.757f, -31.5673f, 44.3913f, -28.2739f
path.quadTo(SkBits2Float(0x42301a3e), SkBits2Float(0xc1c7d7d8), SkBits2Float(0x4225c0bc), SkBits2Float(0xc1b747f6));  // 44.0256f, -24.9804f, 41.4382f, -22.9101f
path.quadTo(SkBits2Float(0x421b6738), SkBits2Float(0xc1a6b815), SkBits2Float(0x420e3ab6), SkBits2Float(0xc1a9a502));  // 38.8508f, -20.8399f, 35.5573f, -21.2056f
path.quadTo(SkBits2Float(0x42010e32), SkBits2Float(0xc1ac91ef), SkBits2Float(0x41f18c82), SkBits2Float(0xc1c144f4));  // 32.2639f, -21.5713f, 30.1936f, -24.1587f
path.quadTo(SkBits2Float(0x41e0fca1), SkBits2Float(0xc1d5f7fa), SkBits2Float(0x41e3e98e), SkBits2Float(0xc1f05101));  // 28.1234f, -26.7461f, 28.489f, -30.0396f
path.quadTo(SkBits2Float(0x41e6d67b), SkBits2Float(0xc2055504), SkBits2Float(0x41fb8980), SkBits2Float(0xc20d9cf4));  // 28.8547f, -33.333f, 31.4421f, -35.4033f
path.close();
path.moveTo(SkBits2Float(0xc19e6455), SkBits2Float(0xc19e6455));  // -19.799f, -19.799f
path.quadTo(SkBits2Float(0xc1399153), SkBits2Float(0xc1e00000), SkBits2Float(0x00000000), SkBits2Float(0xc1e00000));  // -11.598f, -28, 0, -28
path.quadTo(SkBits2Float(0x41399153), SkBits2Float(0xc1e00000), SkBits2Float(0x419e6455), SkBits2Float(0xc19e6455));  // 11.598f, -28, 19.799f, -19.799f
path.quadTo(SkBits2Float(0x41e00000), SkBits2Float(0xc1399153), SkBits2Float(0x41e00000), SkBits2Float(0x00000000));  // 28, -11.598f, 28, 0
path.quadTo(SkBits2Float(0x41e00000), SkBits2Float(0x41399153), SkBits2Float(0x419e6455), SkBits2Float(0x419e6455));  // 28, 11.598f, 19.799f, 19.799f
path.quadTo(SkBits2Float(0x415b75ce), SkBits2Float(0x41cf0dc3), SkBits2Float(0x40b878fc), SkBits2Float(0x41db9f74));  // 13.7163f, 25.8817f, 5.76477f, 27.4529f
path.quadTo(SkBits2Float(0x41000000), SkBits2Float(0x41ee1ba4), SkBits2Float(0x41000000), SkBits2Float(0x42040000));  // 8, 29.7635f, 8, 33
path.quadTo(SkBits2Float(0x41000000), SkBits2Float(0x420ff6ba), SkBits2Float(0x40c2ea2a), SkBits2Float(0x4218c3c6));  // 8, 35.9909f, 6.09108f, 38.1912f
path.quadTo(SkBits2Float(0x41135b8a), SkBits2Float(0x42173bf5), SkBits2Float(0x413c53fb), SkBits2Float(0x421ec49c));  // 9.20985f, 37.8086f, 11.7705f, 39.692f
path.quadTo(SkBits2Float(0x416709c9), SkBits2Float(0x42269f2a), SkBits2Float(0x416f0674), SkBits2Float(0x4233b9ad));  // 14.4399f, 41.6554f, 14.9391f, 44.9313f
path.quadTo(SkBits2Float(0x41770320), SkBits2Float(0x4240d431), SkBits2Float(0x415798ee), SkBits2Float(0x424b81a4));  // 15.4383f, 48.2072f, 13.4748f, 50.8766f
path.quadTo(SkBits2Float(0x41382eba), SkBits2Float(0x42562f18), SkBits2Float(0x4103c4ac), SkBits2Float(0x42582e42));  // 11.5114f, 53.546f, 8.23552f, 54.0452f
path.quadTo(SkBits2Float(0x409eb53c), SkBits2Float(0x425a2d6e), SkBits2Float(0x40129340), SkBits2Float(0x425252e0));  // 4.95962f, 54.5444f, 2.29024f, 52.5809f
path.quadTo(SkBits2Float(0x3ee54581), SkBits2Float(0x424ce72c), SkBits2Float(0xbeb8b807), SkBits2Float(0x4244fb36));  // 0.447796f, 51.2258f, -0.360779f, 49.2453f
path.quadTo(SkBits2Float(0xbf99615c), SkBits2Float(0x424cdad2), SkBits2Float(0xc043dd58), SkBits2Float(0x42522abc));  // -1.19828f, 51.2137f, -3.06038f, 52.5417f
path.quadTo(SkBits2Float(0xc0b84398), SkBits2Float(0x4259dd06), SkBits2Float(0xc1106c72), SkBits2Float(0x4257acc2));  // -5.75825f, 54.4658f, -9.02648f, 53.9187f
path.quadTo(SkBits2Float(0xc144b71a), SkBits2Float(0x42557c80), SkBits2Float(0xc163803e), SkBits2Float(0x424ab1e2));  // -12.2947f, 53.3716f, -14.2188f, 50.6737f
path.quadTo(SkBits2Float(0xc18124b1), SkBits2Float(0x423fe745), SkBits2Float(0xc1798856), SkBits2Float(0x4232d49b));  // -16.1429f, 47.9758f, -15.5958f, 44.7076f
path.quadTo(SkBits2Float(0xc170c74c), SkBits2Float(0x4225c1f0), SkBits2Float(0xc1459cd6), SkBits2Float(0x421e0fa8));  // -15.0487f, 41.4394f, -12.3508f, 39.5153f
path.quadTo(SkBits2Float(0xc11a7260), SkBits2Float(0x42165d5e), SkBits2Float(0xc0cc4f70), SkBits2Float(0x42188da2));  // -9.65292f, 37.5912f, -6.3847f, 38.1383f
path.quadTo(SkBits2Float(0xc0c78c19), SkBits2Float(0x4218a726), SkBits2Float(0xc0c2e01e), SkBits2Float(0x4218c538));  // -6.23585f, 38.1632f, -6.08986f, 38.1926f
path.quadTo(SkBits2Float(0xc1000000), SkBits2Float(0x420ff7b5), SkBits2Float(0xc1000000), SkBits2Float(0x42040000));  // -8, 35.9919f, -8, 33
path.quadTo(SkBits2Float(0xc1000000), SkBits2Float(0x41ee1ba4), SkBits2Float(0xc0b878fc), SkBits2Float(0x41db9f74));  // -8, 29.7635f, -5.76477f, 27.4529f
path.quadTo(SkBits2Float(0xc15b75ce), SkBits2Float(0x41cf0dc3), SkBits2Float(0xc19e6455), SkBits2Float(0x419e6455));  // -13.7163f, 25.8817f, -19.799f, 19.799f
path.quadTo(SkBits2Float(0xc1e00000), SkBits2Float(0x41399153), SkBits2Float(0xc1e00000), SkBits2Float(0x00000000));  // -28, 11.598f, -28, 0
path.quadTo(SkBits2Float(0xc1e00000), SkBits2Float(0xc1399153), SkBits2Float(0xc19e6455), SkBits2Float(0xc19e6455));  // -28, -11.598f, -19.799f, -19.799f
path.close();
path.moveTo(SkBits2Float(0xc23ab9e7), SkBits2Float(0xc1c274dd));  // -46.6815f, -24.3071f
path.quadTo(SkBits2Float(0xc22e95fe), SkBits2Float(0xc1cd18ca), SkBits2Float(0xc2223d50), SkBits2Float(0xc1c373a5));  // -43.6465f, -25.6371f, -40.5599f, -24.4315f
path.quadTo(SkBits2Float(0xc215e4a2), SkBits2Float(0xc1b9ce80), SkBits2Float(0xc21092ac), SkBits2Float(0xc1a186ac));  // -37.4733f, -23.2258f, -36.1432f, -20.1908f
path.quadTo(SkBits2Float(0xc20b40b5), SkBits2Float(0xc1893ed9), SkBits2Float(0xc2101348), SkBits2Float(0xc1611afc));  // -34.8132f, -17.1557f, -36.0188f, -14.0691f
path.quadTo(SkBits2Float(0xc214e5da), SkBits2Float(0xc12fb844), SkBits2Float(0xc22109c4), SkBits2Float(0xc11a706a));  // -37.2245f, -10.9825f, -40.2595f, -9.65244f
path.quadTo(SkBits2Float(0xc22d2dae), SkBits2Float(0xc1052890), SkBits2Float(0xc239865c), SkBits2Float(0xc11872dd));  // -43.2946f, -8.3224f, -46.3812f, -9.52804f
path.quadTo(SkBits2Float(0xc245df09), SkBits2Float(0xc12bbd26), SkBits2Float(0xc24b3100), SkBits2Float(0xc15c4ccc));  // -49.4678f, -10.7337f, -50.7979f, -13.7687f
path.quadTo(SkBits2Float(0xc25082f6), SkBits2Float(0xc1866e3a), SkBits2Float(0xc24bb063), SkBits2Float(0xc19f1f96));  // -52.1279f, -16.8038f, -50.9223f, -19.8904f
path.quadTo(SkBits2Float(0xc246ddd1), SkBits2Float(0xc1b7d0f0), SkBits2Float(0xc23ab9e7), SkBits2Float(0xc1c274dd));  // -49.7166f, -22.977f, -46.6815f, -24.3071f
path.close();
path.moveTo(SkBits2Float(0xc23c98fa), SkBits2Float(0x408b3eec));  // -47.1494f, 4.35143f
path.quadTo(SkBits2Float(0xc22fcb5c), SkBits2Float(0x405f9a18), SkBits2Float(0xc22450bb), SkBits2Float(0x40a4d200));  // -43.9486f, 3.49378f, -41.0788f, 5.15063f
path.quadTo(SkBits2Float(0xc218d61a), SkBits2Float(0x40d9d6f4), SkBits2Float(0xc21567dd), SkBits2Float(0x412021ef));  // -38.2091f, 6.80749f, -37.3514f, 10.0083f
path.quadTo(SkBits2Float(0xc211f9a2), SkBits2Float(0x41535866), SkBits2Float(0xc2189a40), SkBits2Float(0x4180a176));  // -36.4938f, 13.2091f, -38.1506f, 16.0788f
path.quadTo(SkBits2Float(0xc21f3ade), SkBits2Float(0x419796b9), SkBits2Float(0xc22c087c), SkBits2Float(0x419e7330));  // -39.8075f, 18.9486f, -43.0083f, 19.8062f
path.quadTo(SkBits2Float(0xc238d61a), SkBits2Float(0x41a54fa9), SkBits2Float(0xc24450bb), SkBits2Float(0x41980e6c));  // -46.2091f, 20.6639f, -49.0788f, 19.007f
path.quadTo(SkBits2Float(0xc24fcb5c), SkBits2Float(0x418acd2f), SkBits2Float(0xc2533998), SkBits2Float(0x416263e8));  // -51.9486f, 17.3502f, -52.8062f, 14.1494f
path.quadTo(SkBits2Float(0xc256a7d4), SkBits2Float(0x412f2d72), SkBits2Float(0xc2500736), SkBits2Float(0x410142ec));  // -53.6639f, 10.9486f, -52.007f, 8.07884f
path.quadTo(SkBits2Float(0xc2496698), SkBits2Float(0x40a6b0cc), SkBits2Float(0xc23c98fa), SkBits2Float(0x408b3eec));  // -50.3502f, 5.20908f, -47.1494f, 4.35143f
path.close();
path.moveTo(SkBits2Float(0x41dabec3), SkBits2Float(0x41dabec3));  // 27.3431f, 27.3431f
path.quadTo(SkBits2Float(0x41ed7d86), SkBits2Float(0x41c80000), SkBits2Float(0x42040000), SkBits2Float(0x41c80000));  // 29.6863f, 25, 33, 25
path.quadTo(SkBits2Float(0x4211413d), SkBits2Float(0x41c80000), SkBits2Float(0x421aa09e), SkBits2Float(0x41dabec3));  // 36.3137f, 25, 38.6569f, 27.3431f
path.quadTo(SkBits2Float(0x42240000), SkBits2Float(0x41ed7d86), SkBits2Float(0x42240000), SkBits2Float(0x42040000));  // 41, 29.6863f, 41, 33
path.quadTo(SkBits2Float(0x42240000), SkBits2Float(0x4211413d), SkBits2Float(0x421aa09e), SkBits2Float(0x421aa09e));  // 41, 36.3137f, 38.6569f, 38.6569f
path.quadTo(SkBits2Float(0x4211413d), SkBits2Float(0x42240000), SkBits2Float(0x42040000), SkBits2Float(0x42240000));  // 36.3137f, 41, 33, 41
path.quadTo(SkBits2Float(0x41ed7d86), SkBits2Float(0x42240000), SkBits2Float(0x41dabec3), SkBits2Float(0x421aa09e));  // 29.6863f, 41, 27.3431f, 38.6569f
path.quadTo(SkBits2Float(0x41c80000), SkBits2Float(0x4211413d), SkBits2Float(0x41c80000), SkBits2Float(0x42040000));  // 25, 36.3137f, 25, 33
path.quadTo(SkBits2Float(0x41c80000), SkBits2Float(0x41ed7d86), SkBits2Float(0x41dabec3), SkBits2Float(0x41dabec3));  // 25, 29.6863f, 27.3431f, 27.3431f
path.close();
path.moveTo(SkBits2Float(0xc1d961c1), SkBits2Float(0x41f9e1da));  // -27.1727f, 31.2353f
path.quadTo(SkBits2Float(0xc1bf6f78), SkBits2Float(0x41f47252), SkBits2Float(0xc1a93eb6), SkBits2Float(0x42017995));  // -23.9294f, 30.5558f, -21.1556f, 32.3687f
path.quadTo(SkBits2Float(0xc1930df4), SkBits2Float(0x4208ba01), SkBits2Float(0xc18d9e6c), SkBits2Float(0x4215b325));  // -18.3818f, 34.1816f, -17.7024f, 37.4249f
path.quadTo(SkBits2Float(0xc1882ee5), SkBits2Float(0x4222ac49), SkBits2Float(0xc196afbd), SkBits2Float(0x422dc4aa));  // -17.0229f, 40.6682f, -18.8358f, 43.4421f
path.quadTo(SkBits2Float(0xc1a53094), SkBits2Float(0x4238dd0c), SkBits2Float(0xc1bf22dd), SkBits2Float(0x423b94cf));  // -20.6487f, 46.2159f, -23.892f, 46.8953f
path.quadTo(SkBits2Float(0xc1d91525), SkBits2Float(0x423e4c93), SkBits2Float(0xc1ef45e7), SkBits2Float(0x42370c27));  // -27.1353f, 47.5748f, -29.9091f, 45.7619f
path.quadTo(SkBits2Float(0xc202bb55), SkBits2Float(0x422fcbbc), SkBits2Float(0xc2057319), SkBits2Float(0x4222d298));  // -32.6829f, 43.949f, -33.3624f, 40.7057f
path.quadTo(SkBits2Float(0xc2082adc), SkBits2Float(0x4215d973), SkBits2Float(0xc200ea70), SkBits2Float(0x420ac112));  // -34.0419f, 37.4624f, -32.2289f, 34.6885f
path.quadTo(SkBits2Float(0xc1f35409), SkBits2Float(0x41ff5161), SkBits2Float(0xc1d961c1), SkBits2Float(0x41f9e1da));  // -30.416f, 31.9147f, -27.1727f, 31.2353f
path.close();
path.moveTo(SkBits2Float(0xbfccf162), SkBits2Float(0x42236913));  // -1.60112f, 40.8526f
path.quadTo(SkBits2Float(0xbf54c171), SkBits2Float(0x42240000), SkBits2Float(0x00000000), SkBits2Float(0x42240000));  // -0.831077f, 41, 0, 41
path.quadTo(SkBits2Float(0x3dcda9e6), SkBits2Float(0x42240000), SkBits2Float(0x3e4cbe2e), SkBits2Float(0x4223fdcc));  // 0.100422f, 41, 0.199944f, 40.9978f
path.quadTo(SkBits2Float(0x3f12dfd9), SkBits2Float(0x4223f586), SkBits2Float(0x3f6f571b), SkBits2Float(0x4223ce2c));  // 0.573728f, 40.9898f, 0.934923f, 40.9513f
path.quadTo(SkBits2Float(0x3f4168dc), SkBits2Float(0x4224a9bb), SkBits2Float(0x3f15fde0), SkBits2Float(0x422595d9));  // 0.755506f, 41.1658f, 0.585905f, 41.3963f
path.quadTo(SkBits2Float(0x3d27275b), SkBits2Float(0x42288cb9), SkBits2Float(0xbea10331), SkBits2Float(0x422bb375));  // 0.040809f, 42.1374f, -0.314477f, 42.9253f
path.quadTo(SkBits2Float(0xbf287eab), SkBits2Float(0x4228877a), SkBits2Float(0xbf989f50), SkBits2Float(0x42258882));  // -0.658183f, 42.1323f, -1.19236f, 41.3833f
path.quadTo(SkBits2Float(0xbfb1e0f9), SkBits2Float(0x42246d34), SkBits2Float(0xbfccf162), SkBits2Float(0x42236913));  // -1.38968f, 41.1066f, -1.60112f, 40.8526f
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x415798ee), SkBits2Float(0x424b81a4));  // 13.4748f, 50.8766f
path.quadTo(SkBits2Float(0x41382eba), SkBits2Float(0x42562f18), SkBits2Float(0x4103c4ac), SkBits2Float(0x42582e42));  // 11.5114f, 53.546f, 8.23552f, 54.0452f
path.quadTo(SkBits2Float(0x409eb53c), SkBits2Float(0x425a2d6e), SkBits2Float(0x40129340), SkBits2Float(0x425252e0));  // 4.95962f, 54.5444f, 2.29024f, 52.5809f
path.quadTo(SkBits2Float(0xbec21f80), SkBits2Float(0x424a7854), SkBits2Float(0xbf60da80), SkBits2Float(0x423d5dd0));  // -0.379147f, 50.6175f, -0.878334f, 47.3416f
path.quadTo(SkBits2Float(0xbfb052b0), SkBits2Float(0x4230434d), SkBits2Float(0x3f15fde0), SkBits2Float(0x422595d9));  // -1.37752f, 44.0657f, 0.585905f, 41.3963f
path.quadTo(SkBits2Float(0x40232840), SkBits2Float(0x421ae866), SkBits2Float(0x40ba6840), SkBits2Float(0x4218e93b));  // 2.54933f, 38.727f, 5.82523f, 38.2278f
path.quadTo(SkBits2Float(0x41119e2f), SkBits2Float(0x4216ea10), SkBits2Float(0x413c53fb), SkBits2Float(0x421ec49c));  // 9.10112f, 37.7286f, 11.7705f, 39.692f
path.quadTo(SkBits2Float(0x416709c9), SkBits2Float(0x42269f2a), SkBits2Float(0x416f0674), SkBits2Float(0x4233b9ad));  // 14.4399f, 41.6554f, 14.9391f, 44.9313f
path.quadTo(SkBits2Float(0x41770320), SkBits2Float(0x4240d431), SkBits2Float(0x415798ee), SkBits2Float(0x424b81a4));  // 15.4383f, 48.2072f, 13.4748f, 50.8766f
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}

// SkDQuadIntersection.cpp:598: failed assertion "way_roughly_equal(fT[0][index], 1)"
static void fuzz763_35322(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x41042400), SkBits2Float(0xc24fea42));  // 8.25879f, -51.9788f
path.quadTo(SkBits2Float(0x413225f2), SkBits2Float(0xc25680b3), SkBits2Float(0x4165502a), SkBits2Float(0xc2530720));  // 11.1343f, -53.6257f, 14.3321f, -52.757f
path.quadTo(SkBits2Float(0x418c3d32), SkBits2Float(0xc24f8d8e), SkBits2Float(0x41996a13), SkBits2Float(0xc2440d11));  // 17.5299f, -51.8882f, 19.1768f, -49.0128f
path.quadTo(SkBits2Float(0x41a696f3), SkBits2Float(0xc2388c95), SkBits2Float(0x419fa3cc), SkBits2Float(0xc22bc206));  // 20.8237f, -46.1373f, 19.955f, -42.9395f
path.quadTo(SkBits2Float(0x4198b0a8), SkBits2Float(0xc21ef778), SkBits2Float(0x4181afaf), SkBits2Float(0xc2186108));  // 19.0863f, -39.7417f, 16.2108f, -38.0948f
path.quadTo(SkBits2Float(0x41555d6e), SkBits2Float(0xc211ca98), SkBits2Float(0x41223335), SkBits2Float(0xc215442b));  // 13.3353f, -36.4478f, 10.1375f, -37.3166f
path.quadTo(SkBits2Float(0x40de11f8), SkBits2Float(0xc218bdbe), SkBits2Float(0x40a95e78), SkBits2Float(0xc2243e3a));  // 6.93969f, -38.1853f, 5.29278f, -41.0608f
path.quadTo(SkBits2Float(0x406955e8), SkBits2Float(0xc22fbeb6), SkBits2Float(0x4090778c), SkBits2Float(0xc23c8944));  // 3.64587f, -43.9362f, 4.51459f, -47.134f
path.quadTo(SkBits2Float(0x40ac4420), SkBits2Float(0xc24953d2), SkBits2Float(0x41042400), SkBits2Float(0xc24fea42));  // 5.38332f, -50.3319f, 8.25879f, -51.9788f
path.close();
path.moveTo(SkBits2Float(0x41cb7543), SkBits2Float(0xc2385013));  // 25.4323f, -46.0782f
path.quadTo(SkBits2Float(0x41e591fd), SkBits2Float(0xc23a9972), SkBits2Float(0x41fb44a5), SkBits2Float(0xc232fbf5));  // 28.6963f, -46.6498f, 31.4085f, -44.7461f
path.quadTo(SkBits2Float(0x42087ba8), SkBits2Float(0xc22b5e79), SkBits2Float(0x420ac507), SkBits2Float(0xc21e501c));  // 34.1208f, -42.8423f, 34.6924f, -39.5782f
path.quadTo(SkBits2Float(0x420d0e67), SkBits2Float(0xc21141bf), SkBits2Float(0x420570ea), SkBits2Float(0xc206686a));  // 35.2641f, -36.3142f, 33.3603f, -33.602f
path.quadTo(SkBits2Float(0x41fba6dc), SkBits2Float(0xc1f71e2c), SkBits2Float(0x41e18a22), SkBits2Float(0xc1f28b6c));  // 31.4565f, -30.8897f, 28.1924f, -30.3181f
path.quadTo(SkBits2Float(0x41c76d67), SkBits2Float(0xc1edf8ad), SkBits2Float(0x41b1babe), SkBits2Float(0xc1fd33a6));  // 24.9284f, -29.7464f, 22.2162f, -31.6502f
path.quadTo(SkBits2Float(0x419c0815), SkBits2Float(0xc2063750), SkBits2Float(0x41977556), SkBits2Float(0xc21345ad));  // 19.5039f, -33.554f, 18.9323f, -36.818f
path.quadTo(SkBits2Float(0x4192e296), SkBits2Float(0xc220540a), SkBits2Float(0x41a21d8f), SkBits2Float(0xc22b2d5e));  // 18.3606f, -40.0821f, 20.2644f, -42.7943f
path.quadTo(SkBits2Float(0x41b15888), SkBits2Float(0xc23606b3), SkBits2Float(0x41cb7543), SkBits2Float(0xc2385013));  // 22.1682f, -45.5065f, 25.4323f, -46.0782f
path.close();
path.moveTo(SkBits2Float(0x4206de71), SkBits2Float(0xc204f99f));  // 33.7172f, -33.2438f
path.quadTo(SkBits2Float(0x4211be80), SkBits2Float(0xc20c8d7c), SkBits2Float(0x421ecad2), SkBits2Float(0xc20a388c));  // 36.436f, -35.1382f, 39.6981f, -34.5552f
path.quadTo(SkBits2Float(0x422bd724), SkBits2Float(0xc207e39b), SkBits2Float(0x42336b00), SkBits2Float(0xc1fa0718));  // 42.9601f, -33.9723f, 44.8545f, -31.2535f
path.quadTo(SkBits2Float(0x423afedd), SkBits2Float(0xc1e446f9), SkBits2Float(0x4238a9ec), SkBits2Float(0xc1ca2e57));  // 46.7489f, -28.5347f, 46.1659f, -25.2726f
path.quadTo(SkBits2Float(0x423654fc), SkBits2Float(0xc1b015b3), SkBits2Float(0x422b74ed), SkBits2Float(0xc1a0edfa));  // 45.583f, -22.0106f, 42.8642f, -20.1162f
path.quadTo(SkBits2Float(0x422094de), SkBits2Float(0xc191c640), SkBits2Float(0x4213888c), SkBits2Float(0xc1967021));  // 40.1454f, -18.2218f, 36.8833f, -18.8048f
path.quadTo(SkBits2Float(0x42067c3a), SkBits2Float(0xc19b1a02), SkBits2Float(0x41fdd0bc), SkBits2Float(0xc1b0da20));  // 33.6213f, -19.3877f, 31.7269f, -22.1065f
path.quadTo(SkBits2Float(0x41eea902), SkBits2Float(0xc1c69a3f), SkBits2Float(0x41f352e3), SkBits2Float(0xc1e0b2e3));  // 29.8325f, -24.8253f, 30.4155f, -28.0873f
path.quadTo(SkBits2Float(0x41f7fcc4), SkBits2Float(0xc1facb85), SkBits2Float(0x4206de71), SkBits2Float(0xc204f99f));  // 30.9984f, -31.3494f, 33.7172f, -33.2438f
path.close();
path.moveTo(SkBits2Float(0xc19e6455), SkBits2Float(0xc19e6455));  // -19.799f, -19.799f
path.quadTo(SkBits2Float(0xc1399153), SkBits2Float(0xc1e00000), SkBits2Float(0x00000000), SkBits2Float(0xc1e00000));  // -11.598f, -28, 0, -28
path.quadTo(SkBits2Float(0x41399153), SkBits2Float(0xc1e00000), SkBits2Float(0x419e6455), SkBits2Float(0xc19e6455));  // 11.598f, -28, 19.799f, -19.799f
path.quadTo(SkBits2Float(0x41e00000), SkBits2Float(0xc1399153), SkBits2Float(0x41e00000), SkBits2Float(0x00000000));  // 28, -11.598f, 28, 0
path.quadTo(SkBits2Float(0x41e00000), SkBits2Float(0x41399153), SkBits2Float(0x419e6455), SkBits2Float(0x419e6455));  // 28, 11.598f, 19.799f, 19.799f
path.quadTo(SkBits2Float(0x415b75ce), SkBits2Float(0x41cf0dc3), SkBits2Float(0x40b878fc), SkBits2Float(0x41db9f74));  // 13.7163f, 25.8817f, 5.76477f, 27.4529f
path.quadTo(SkBits2Float(0x41000000), SkBits2Float(0x41ee1ba4), SkBits2Float(0x41000000), SkBits2Float(0x42040000));  // 8, 29.7635f, 8, 33
path.quadTo(SkBits2Float(0x41000000), SkBits2Float(0x4211413d), SkBits2Float(0x40b504f3), SkBits2Float(0x421aa09e));  // 8, 36.3137f, 5.65685f, 38.6569f
path.quadTo(SkBits2Float(0x40b4fa00), SkBits2Float(0x421aa1fd), SkBits2Float(0x40b4ef0c), SkBits2Float(0x421aa35b));  // 5.65552f, 38.6582f, 5.65418f, 38.6595f
path.quadTo(SkBits2Float(0x40f11f68), SkBits2Float(0x421c342c), SkBits2Float(0x4111ddac), SkBits2Float(0x42218978));  // 7.53508f, 39.0509f, 9.11662f, 40.3842f
path.quadTo(SkBits2Float(0x413a6700), SkBits2Float(0x422a1497), SkBits2Float(0x413ee6c0), SkBits2Float(0x42374996));  // 11.6501f, 42.5201f, 11.9313f, 45.8219f
path.quadTo(SkBits2Float(0x41436686), SkBits2Float(0x42447e96), SkBits2Float(0x41213a06), SkBits2Float(0x424ea0eb));  // 12.2125f, 49.1236f, 10.0767f, 51.6571f
path.quadTo(SkBits2Float(0x40fe1b10), SkBits2Float(0x4258c340), SkBits2Float(0x40947314), SkBits2Float(0x4259e330));  // 7.9408f, 54.1907f, 4.63905f, 54.4719f
path.quadTo(SkBits2Float(0x3fab2c60), SkBits2Float(0x425b0321), SkBits2Float(0xbf991e40), SkBits2Float(0x42527802));  // 1.33729f, 54.7531f, -1.19624f, 52.6172f
path.quadTo(SkBits2Float(0xc06eb470), SkBits2Float(0x4249ece2), SkBits2Float(0xc08059b8), SkBits2Float(0x423cb7e2));  // -3.72976f, 50.4813f, -4.01095f, 47.1796f
path.quadTo(SkBits2Float(0xc0895940), SkBits2Float(0x422f82e3), SkBits2Float(0xc00a0088), SkBits2Float(0x4225608e));  // -4.29214f, 43.8778f, -2.15628f, 41.3443f
path.quadTo(SkBits2Float(0xbff725c6), SkBits2Float(0x42244eb9), SkBits2Float(0xbfd8a182), SkBits2Float(0x4223569a));  // -1.93084f, 41.0769f, -1.69243f, 40.8346f
path.quadTo(SkBits2Float(0xc07bec59), SkBits2Float(0x42218277), SkBits2Float(0xc0b504f3), SkBits2Float(0x421aa09e));  // -3.9363f, 40.3774f, -5.65685f, 38.6569f
path.quadTo(SkBits2Float(0xc1000000), SkBits2Float(0x4211413d), SkBits2Float(0xc1000000), SkBits2Float(0x42040000));  // -8, 36.3137f, -8, 33
path.quadTo(SkBits2Float(0xc1000000), SkBits2Float(0x41ee1ba4), SkBits2Float(0xc0b878fc), SkBits2Float(0x41db9f74));  // -8, 29.7635f, -5.76477f, 27.4529f
path.quadTo(SkBits2Float(0xc15b75ce), SkBits2Float(0x41cf0dc3), SkBits2Float(0xc19e6455), SkBits2Float(0x419e6455));  // -13.7163f, 25.8817f, -19.799f, 19.799f
path.quadTo(SkBits2Float(0xc1e00000), SkBits2Float(0x41399153), SkBits2Float(0xc1e00000), SkBits2Float(0x00000000));  // -28, 11.598f, -28, 0
path.quadTo(SkBits2Float(0xc1e00000), SkBits2Float(0xc1399153), SkBits2Float(0xc19e6455), SkBits2Float(0xc19e6455));  // -28, -11.598f, -19.799f, -19.799f
path.close();
path.moveTo(SkBits2Float(0xc233e0fb), SkBits2Float(0xc1dac1ac));  // -44.9697f, -27.3446f
path.quadTo(SkBits2Float(0xc22769b6), SkBits2Float(0xc1e3c410), SkBits2Float(0xc21b69b6), SkBits2Float(0xc1d881ca));  // -41.8532f, -28.4707f, -38.8532f, -27.0634f
path.quadTo(SkBits2Float(0xc20f69b6), SkBits2Float(0xc1cd3f84), SkBits2Float(0xc20ae884), SkBits2Float(0xc1b450fa));  // -35.8532f, -25.656f, -34.7271f, -22.5395f
path.quadTo(SkBits2Float(0xc2066752), SkBits2Float(0xc19b6270), SkBits2Float(0xc20c0875), SkBits2Float(0xc1836270));  // -33.6009f, -19.4231f, -35.0083f, -16.4231f
path.quadTo(SkBits2Float(0xc211a998), SkBits2Float(0xc156c4e1), SkBits2Float(0xc21e20dd), SkBits2Float(0xc144c018));  // -36.4156f, -13.4231f, -39.5321f, -12.2969f
path.quadTo(SkBits2Float(0xc22a9822), SkBits2Float(0xc132bb50), SkBits2Float(0xc2369821), SkBits2Float(0xc1493fdc));  // -42.6486f, -11.1707f, -45.6486f, -12.5781f
path.quadTo(SkBits2Float(0xc2429822), SkBits2Float(0xc15fc467), SkBits2Float(0xc2471954), SkBits2Float(0xc188d0be));  // -48.6486f, -13.9854f, -49.7747f, -17.1019f
path.quadTo(SkBits2Float(0xc24b9a86), SkBits2Float(0xc1a1bf48), SkBits2Float(0xc245f962), SkBits2Float(0xc1b9bf47));  // -50.9009f, -20.2184f, -49.4935f, -23.2184f
path.quadTo(SkBits2Float(0xc2405840), SkBits2Float(0xc1d1bf48), SkBits2Float(0xc233e0fb), SkBits2Float(0xc1dac1ac));  // -48.0862f, -26.2184f, -44.9697f, -27.3446f
path.close();
path.moveTo(SkBits2Float(0xc23c98fa), SkBits2Float(0x408b3eec));  // -47.1494f, 4.35143f
path.quadTo(SkBits2Float(0xc22fcb5c), SkBits2Float(0x405f9a18), SkBits2Float(0xc22450bb), SkBits2Float(0x40a4d200));  // -43.9486f, 3.49378f, -41.0788f, 5.15063f
path.quadTo(SkBits2Float(0xc218d61a), SkBits2Float(0x40d9d6f4), SkBits2Float(0xc21567dd), SkBits2Float(0x412021ef));  // -38.2091f, 6.80749f, -37.3514f, 10.0083f
path.quadTo(SkBits2Float(0xc211f9a2), SkBits2Float(0x41535866), SkBits2Float(0xc2189a40), SkBits2Float(0x4180a176));  // -36.4938f, 13.2091f, -38.1506f, 16.0788f
path.quadTo(SkBits2Float(0xc21f3ade), SkBits2Float(0x419796b9), SkBits2Float(0xc22c087c), SkBits2Float(0x419e7330));  // -39.8075f, 18.9486f, -43.0083f, 19.8062f
path.quadTo(SkBits2Float(0xc238d61a), SkBits2Float(0x41a54fa9), SkBits2Float(0xc24450bb), SkBits2Float(0x41980e6c));  // -46.2091f, 20.6639f, -49.0788f, 19.007f
path.quadTo(SkBits2Float(0xc24fcb5c), SkBits2Float(0x418acd2f), SkBits2Float(0xc2533998), SkBits2Float(0x416263e8));  // -51.9486f, 17.3502f, -52.8062f, 14.1494f
path.quadTo(SkBits2Float(0xc256a7d4), SkBits2Float(0x412f2d72), SkBits2Float(0xc2500736), SkBits2Float(0x410142ec));  // -53.6639f, 10.9486f, -52.007f, 8.07884f
path.quadTo(SkBits2Float(0xc2496698), SkBits2Float(0x40a6b0cc), SkBits2Float(0xc23c98fa), SkBits2Float(0x408b3eec));  // -50.3502f, 5.20908f, -47.1494f, 4.35143f
path.close();
path.moveTo(SkBits2Float(0x41dabec3), SkBits2Float(0x41dabec3));  // 27.3431f, 27.3431f
path.quadTo(SkBits2Float(0x41ed7d86), SkBits2Float(0x41c80000), SkBits2Float(0x42040000), SkBits2Float(0x41c80000));  // 29.6863f, 25, 33, 25
path.quadTo(SkBits2Float(0x4211413d), SkBits2Float(0x41c80000), SkBits2Float(0x421aa09e), SkBits2Float(0x41dabec3));  // 36.3137f, 25, 38.6569f, 27.3431f
path.quadTo(SkBits2Float(0x42240000), SkBits2Float(0x41ed7d86), SkBits2Float(0x42240000), SkBits2Float(0x42040000));  // 41, 29.6863f, 41, 33
path.quadTo(SkBits2Float(0x42240000), SkBits2Float(0x4211413d), SkBits2Float(0x421aa09e), SkBits2Float(0x421aa09e));  // 41, 36.3137f, 38.6569f, 38.6569f
path.quadTo(SkBits2Float(0x4211413d), SkBits2Float(0x42240000), SkBits2Float(0x42040000), SkBits2Float(0x42240000));  // 36.3137f, 41, 33, 41
path.quadTo(SkBits2Float(0x41ed7d86), SkBits2Float(0x42240000), SkBits2Float(0x41dabec3), SkBits2Float(0x421aa09e));  // 29.6863f, 41, 27.3431f, 38.6569f
path.quadTo(SkBits2Float(0x41c80000), SkBits2Float(0x4211413d), SkBits2Float(0x41c80000), SkBits2Float(0x42040000));  // 25, 36.3137f, 25, 33
path.quadTo(SkBits2Float(0x41c80000), SkBits2Float(0x41ed7d86), SkBits2Float(0x41dabec3), SkBits2Float(0x41dabec3));  // 25, 29.6863f, 27.3431f, 27.3431f
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x41213a06), SkBits2Float(0x424ea0eb));  // 10.0767f, 51.6571f
path.quadTo(SkBits2Float(0x40fe1b10), SkBits2Float(0x4258c340), SkBits2Float(0x40947314), SkBits2Float(0x4259e330));  // 7.9408f, 54.1907f, 4.63905f, 54.4719f
path.quadTo(SkBits2Float(0x3fab2c60), SkBits2Float(0x425b0321), SkBits2Float(0xbf991e40), SkBits2Float(0x42527802));  // 1.33729f, 54.7531f, -1.19624f, 52.6172f
path.quadTo(SkBits2Float(0xc06eb470), SkBits2Float(0x4249ece2), SkBits2Float(0xc08059b8), SkBits2Float(0x423cb7e2));  // -3.72976f, 50.4813f, -4.01095f, 47.1796f
path.quadTo(SkBits2Float(0xc0895940), SkBits2Float(0x422f82e3), SkBits2Float(0xc00a0088), SkBits2Float(0x4225608e));  // -4.29214f, 43.8778f, -2.15628f, 41.3443f
path.quadTo(SkBits2Float(0xbca74400), SkBits2Float(0x421b3e39), SkBits2Float(0x40520168), SkBits2Float(0x421a1e48));  // -0.0204182f, 38.8108f, 3.28134f, 38.5296f
path.quadTo(SkBits2Float(0x40d2a8b0), SkBits2Float(0x4218fe58), SkBits2Float(0x4111ddac), SkBits2Float(0x42218978));  // 6.58309f, 38.2484f, 9.11662f, 40.3842f
path.quadTo(SkBits2Float(0x413a6700), SkBits2Float(0x422a1497), SkBits2Float(0x413ee6c0), SkBits2Float(0x42374996));  // 11.6501f, 42.5201f, 11.9313f, 45.8219f
path.quadTo(SkBits2Float(0x41436686), SkBits2Float(0x42447e96), SkBits2Float(0x41213a06), SkBits2Float(0x424ea0eb));  // 12.2125f, 49.1236f, 10.0767f, 51.6571f
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}

// SkPathOpsOp.cpp:52: failed assertion "angle != firstAngle || !loop"
static void fuzz763_849020(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x411e4374), SkBits2Float(0xc24ec58f));  // 9.89147f, -51.6929f
path.quadTo(SkBits2Float(0x414d13fa), SkBits2Float(0xc254fe70), SkBits2Float(0x417fc7a6), SkBits2Float(0xc2511e30));  // 12.8174f, -53.2485f, 15.9862f, -52.2795f
path.quadTo(SkBits2Float(0x41993dab), SkBits2Float(0xc24d3df4), SkBits2Float(0x41a5af6e), SkBits2Float(0xc24189d2));  // 19.1551f, -51.3105f, 20.7107f, -48.3846f
path.quadTo(SkBits2Float(0x41b22132), SkBits2Float(0xc235d5b1), SkBits2Float(0x41aa60b2), SkBits2Float(0xc22928c4));  // 22.2662f, -45.4587f, 21.2972f, -42.2898f
path.quadTo(SkBits2Float(0x41a2a038), SkBits2Float(0xc21c7bda), SkBits2Float(0x418b37f4), SkBits2Float(0xc21642f8));  // 20.3282f, -39.1209f, 17.4023f, -37.5654f
path.quadTo(SkBits2Float(0x41679f65), SkBits2Float(0xc2100a16), SkBits2Float(0x4134ebb5), SkBits2Float(0xc213ea55));  // 14.4764f, -36.0098f, 11.3075f, -36.9788f
path.quadTo(SkBits2Float(0x41023808), SkBits2Float(0xc217ca94), SkBits2Float(0x40d2a902), SkBits2Float(0xc2237eb5));  // 8.13868f, -37.9478f, 6.58313f, -40.8737f
path.quadTo(SkBits2Float(0x40a0e1f4), SkBits2Float(0xc22f32d6), SkBits2Float(0x40bfe3ec), SkBits2Float(0xc23bdfc1));  // 5.02758f, -43.7996f, 5.99657f, -46.9685f
path.quadTo(SkBits2Float(0x40dee5e0), SkBits2Float(0xc2488cad), SkBits2Float(0x411e4374), SkBits2Float(0xc24ec58f));  // 6.96556f, -50.1374f, 9.89147f, -51.6929f
path.close();
path.moveTo(SkBits2Float(0xc19e6455), SkBits2Float(0xc19e6455));  // -19.799f, -19.799f
path.quadTo(SkBits2Float(0xc1399153), SkBits2Float(0xc1e00000), SkBits2Float(0x00000000), SkBits2Float(0xc1e00000));  // -11.598f, -28, 0, -28
path.quadTo(SkBits2Float(0x41399153), SkBits2Float(0xc1e00000), SkBits2Float(0x419e6455), SkBits2Float(0xc19e6455));  // 11.598f, -28, 19.799f, -19.799f
path.quadTo(SkBits2Float(0x41e00000), SkBits2Float(0xc1399153), SkBits2Float(0x41e00000), SkBits2Float(0x00000000));  // 28, -11.598f, 28, 0
path.quadTo(SkBits2Float(0x41e00000), SkBits2Float(0x41399153), SkBits2Float(0x419e6455), SkBits2Float(0x419e6455));  // 28, 11.598f, 19.799f, 19.799f
path.quadTo(SkBits2Float(0x415b75ce), SkBits2Float(0x41cf0dc3), SkBits2Float(0x40b878fc), SkBits2Float(0x41db9f74));  // 13.7163f, 25.8817f, 5.76477f, 27.4529f
path.quadTo(SkBits2Float(0x41000000), SkBits2Float(0x41ee1ba4), SkBits2Float(0x41000000), SkBits2Float(0x42040000));  // 8, 29.7635f, 8, 33
path.quadTo(SkBits2Float(0x41000000), SkBits2Float(0x4211413d), SkBits2Float(0x40b504f3), SkBits2Float(0x421aa09e));  // 8, 36.3137f, 5.65685f, 38.6569f
path.quadTo(SkBits2Float(0x405413cd), SkBits2Float(0x42240000), SkBits2Float(0x00000000), SkBits2Float(0x42240000));  // 3.31371f, 41, 0, 41
path.quadTo(SkBits2Float(0xc05413cd), SkBits2Float(0x42240000), SkBits2Float(0xc0b504f3), SkBits2Float(0x421aa09e));  // -3.31371f, 41, -5.65685f, 38.6569f
path.quadTo(SkBits2Float(0xc1000000), SkBits2Float(0x4211413d), SkBits2Float(0xc1000000), SkBits2Float(0x42040000));  // -8, 36.3137f, -8, 33
path.quadTo(SkBits2Float(0xc1000000), SkBits2Float(0x41ee1ba4), SkBits2Float(0xc0b878fc), SkBits2Float(0x41db9f74));  // -8, 29.7635f, -5.76477f, 27.4529f
path.quadTo(SkBits2Float(0xc15b75ce), SkBits2Float(0x41cf0dc3), SkBits2Float(0xc19e6455), SkBits2Float(0x419e6455));  // -13.7163f, 25.8817f, -19.799f, 19.799f
path.quadTo(SkBits2Float(0xc1e00000), SkBits2Float(0x41399153), SkBits2Float(0xc1e00000), SkBits2Float(0x00000000));  // -28, 11.598f, -28, 0
path.quadTo(SkBits2Float(0xc1e00000), SkBits2Float(0xc1399153), SkBits2Float(0xc19e6455), SkBits2Float(0xc19e6455));  // -28, -11.598f, -19.799f, -19.799f
path.close();
path.moveTo(SkBits2Float(0xc23d1735), SkBits2Float(0x402cdce8));  // -47.2727f, 2.70098f
path.quadTo(SkBits2Float(0xc2302ce7), SkBits2Float(0x3ffa54f0), SkBits2Float(0xc224ef5c), SkBits2Float(0x406d8f00));  // -44.0438f, 1.95572f, -41.2337f, 3.71185f
path.quadTo(SkBits2Float(0xc219b1d2), SkBits2Float(0x40aef9c4), SkBits2Float(0xc216b6ab), SkBits2Float(0x410b261b));  // -38.4237f, 5.46799f, -37.6784f, 8.6968f
path.quadTo(SkBits2Float(0xc2166795), SkBits2Float(0x411080a8), SkBits2Float(0xc2163402), SkBits2Float(0x4115c8a0));  // -37.6012f, 9.03141f, -37.5508f, 9.36148f
path.quadTo(SkBits2Float(0xc215c2e1), SkBits2Float(0x411ad33f), SkBits2Float(0xc21567dd), SkBits2Float(0x412021ef));  // -37.4403f, 9.67657f, -37.3514f, 10.0083f
path.quadTo(SkBits2Float(0xc211f9a2), SkBits2Float(0x41535866), SkBits2Float(0xc2189a40), SkBits2Float(0x4180a176));  // -36.4938f, 13.2091f, -38.1506f, 16.0788f
path.quadTo(SkBits2Float(0xc21f3ade), SkBits2Float(0x419796b9), SkBits2Float(0xc22c087c), SkBits2Float(0x419e7330));  // -39.8075f, 18.9486f, -43.0083f, 19.8062f
path.quadTo(SkBits2Float(0xc238d61a), SkBits2Float(0x41a54fa9), SkBits2Float(0xc24450bb), SkBits2Float(0x41980e6c));  // -46.2091f, 20.6639f, -49.0788f, 19.007f
path.quadTo(SkBits2Float(0xc24fcb5c), SkBits2Float(0x418acd2f), SkBits2Float(0xc2533998), SkBits2Float(0x416263e8));  // -51.9486f, 17.3502f, -52.8062f, 14.1494f
path.quadTo(SkBits2Float(0xc253bae4), SkBits2Float(0x415ad9be), SkBits2Float(0xc2540461), SkBits2Float(0x41536cea));  // -52.9325f, 13.6782f, -53.0043f, 13.2141f
path.quadTo(SkBits2Float(0xc254a293), SkBits2Float(0x414c5480), SkBits2Float(0xc25512ee), SkBits2Float(0x4144b962));  // -53.1588f, 12.7706f, -53.2685f, 12.2953f
path.quadTo(SkBits2Float(0xc2580e14), SkBits2Float(0x41111028), SkBits2Float(0xc25107cc), SkBits2Float(0x40c833fc));  // -54.0137f, 9.06644f, -52.2576f, 6.25635f
path.quadTo(SkBits2Float(0xc24a0183), SkBits2Float(0x405c8f50), SkBits2Float(0xc23d1735), SkBits2Float(0x402cdce8));  // -50.5015f, 3.44625f, -47.2727f, 2.70098f
path.close();
path.moveTo(SkBits2Float(0x41dabec3), SkBits2Float(0x41dabec3));  // 27.3431f, 27.3431f
path.quadTo(SkBits2Float(0x41ed7d86), SkBits2Float(0x41c80000), SkBits2Float(0x42040000), SkBits2Float(0x41c80000));  // 29.6863f, 25, 33, 25
path.quadTo(SkBits2Float(0x4211413d), SkBits2Float(0x41c80000), SkBits2Float(0x421aa09e), SkBits2Float(0x41dabec3));  // 36.3137f, 25, 38.6569f, 27.3431f
path.quadTo(SkBits2Float(0x42240000), SkBits2Float(0x41ed7d86), SkBits2Float(0x42240000), SkBits2Float(0x42040000));  // 41, 29.6863f, 41, 33
path.quadTo(SkBits2Float(0x42240000), SkBits2Float(0x4211413d), SkBits2Float(0x421aa09e), SkBits2Float(0x421aa09e));  // 41, 36.3137f, 38.6569f, 38.6569f
path.quadTo(SkBits2Float(0x421a60e3), SkBits2Float(0x421ae059), SkBits2Float(0x421a205f), SkBits2Float(0x421b1e77));  // 38.5946f, 38.7191f, 38.5316f, 38.7798f
path.lineTo(SkBits2Float(0x421a2049), SkBits2Float(0x421b1e8d));  // 38.5315f, 38.7798f
path.quadTo(SkBits2Float(0x4218fba8), SkBits2Float(0x421c384c), SkBits2Float(0x4217c88e), SkBits2Float(0x421d2f21));  // 38.2458f, 39.055f, 37.9459f, 39.296f
path.quadTo(SkBits2Float(0x42168e68), SkBits2Float(0x421e9b27), SkBits2Float(0x42152104), SkBits2Float(0x421fefda));  // 37.6391f, 39.6515f, 37.2822f, 39.9842f
path.quadTo(SkBits2Float(0x42146c3b), SkBits2Float(0x4220986c), SkBits2Float(0x4213b29d), SkBits2Float(0x42213416));  // 37.1057f, 40.1488f, 36.9244f, 40.3009f
path.quadTo(SkBits2Float(0x42130756), SkBits2Float(0x4221df6b), SkBits2Float(0x42124f9e), SkBits2Float(0x422284d0));  // 36.7572f, 40.4682f, 36.5778f, 40.6297f
path.quadTo(SkBits2Float(0x420875c9), SkBits2Float(0x422b6326), SkBits2Float(0x41f6726d), SkBits2Float(0x422ab150));  // 34.115f, 42.8468f, 30.8059f, 42.6732f
path.quadTo(SkBits2Float(0x41dbf947), SkBits2Float(0x4229ff79), SkBits2Float(0x41ca3c9c), SkBits2Float(0x422025a4));  // 27.4967f, 42.4995f, 25.2796f, 40.0368f
path.quadTo(SkBits2Float(0x41b87ff1), SkBits2Float(0x42164bcf), SkBits2Float(0x41b9e39e), SkBits2Float(0x42090f3c));  // 23.0625f, 37.574f, 23.2361f, 34.2649f
path.quadTo(SkBits2Float(0x41bb474a), SkBits2Float(0x41f7a551), SkBits2Float(0x41cefaf4), SkBits2Float(0x41e5e8a6));  // 23.4098f, 30.9557f, 25.8725f, 28.7386f
path.quadTo(SkBits2Float(0x41cffe18), SkBits2Float(0x41e4ff5a), SkBits2Float(0x41d105e6), SkBits2Float(0x41e422e9));  // 25.9991f, 28.6247f, 26.1279f, 28.517f
path.quadTo(SkBits2Float(0x41d1f880), SkBits2Float(0x41e32f61), SkBits2Float(0x41d2f77e), SkBits2Float(0x41e2419e));  // 26.2463f, 28.3981f, 26.3708f, 28.282f
path.quadTo(SkBits2Float(0x41d4f9d5), SkBits2Float(0x41e06208), SkBits2Float(0x41d70fba), SkBits2Float(0x41deb6ad));  // 26.622f, 28.0479f, 26.8827f, 27.8392f
path.quadTo(SkBits2Float(0x41d8cd7c), SkBits2Float(0x41dcb00a), SkBits2Float(0x41dabec3), SkBits2Float(0x41dabec3));  // 27.1003f, 27.586f, 27.3431f, 27.3431f
path.close();
path.moveTo(SkBits2Float(0xc185f1f2), SkBits2Float(0x42177488));  // -16.7431f, 37.8638f
path.quadTo(SkBits2Float(0xc15d848f), SkBits2Float(0x42110787), SkBits2Float(0xc12a8d7f), SkBits2Float(0x4214aee9));  // -13.8449f, 36.2574f, -10.6595f, 37.1708f
path.quadTo(SkBits2Float(0xc0ef2cde), SkBits2Float(0x4218564b), SkBits2Float(0xc0bbc4da), SkBits2Float(0x4223ee21));  // -7.47423f, 38.0843f, -5.86778f, 40.9825f
path.quadTo(SkBits2Float(0xc0885cd4), SkBits2Float(0x422f85f6), SkBits2Float(0xc0a597e8), SkBits2Float(0x423c43ba));  // -4.26133f, 43.8808f, -5.17479f, 47.0661f
path.quadTo(SkBits2Float(0xc0c2d2f4), SkBits2Float(0x4249017e), SkBits2Float(0xc10fc8d0), SkBits2Float(0x424f6e7e));  // -6.08825f, 50.2515f, -8.98653f, 51.8579f
path.quadTo(SkBits2Float(0xc13e2826), SkBits2Float(0x4255db7f), SkBits2Float(0xc1711f36), SkBits2Float(0x4252341c));  // -11.8848f, 53.4644f, -15.0701f, 52.5509f
path.quadTo(SkBits2Float(0xc1920b22), SkBits2Float(0x424e8cbc), SkBits2Float(0xc19ee524), SkBits2Float(0x4242f4e6));  // -18.2554f, 51.6374f, -19.8619f, 48.7392f
path.quadTo(SkBits2Float(0xc1abbf24), SkBits2Float(0x42375d10), SkBits2Float(0xc1a47060), SkBits2Float(0x422a9f4c));  // -21.4683f, 45.8409f, -20.5549f, 42.6556f
path.quadTo(SkBits2Float(0xc19d219e), SkBits2Float(0x421de188), SkBits2Float(0xc185f1f2), SkBits2Float(0x42177488));  // -19.6414f, 39.4702f, -16.7431f, 37.8638f
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x421f4961), SkBits2Float(0x4209a6a0));  // 39.8217f, 34.4127f
path.quadTo(SkBits2Float(0x421ed2ca), SkBits2Float(0x4216e5cb), SkBits2Float(0x42152104), SkBits2Float(0x421fefda));  // 39.7058f, 37.7244f, 37.2822f, 39.9842f
path.quadTo(SkBits2Float(0x420b6f41), SkBits2Float(0x4228f9ea), SkBits2Float(0x41fc602b), SkBits2Float(0x42288353));  // 34.8586f, 42.2441f, 31.547f, 42.1282f
path.quadTo(SkBits2Float(0x41e1e1d7), SkBits2Float(0x42280cbd), SkBits2Float(0x41cfcdb9), SkBits2Float(0x421e5af7));  // 28.2353f, 42.0124f, 25.9755f, 39.5888f
path.quadTo(SkBits2Float(0x41bdb999), SkBits2Float(0x4214a933), SkBits2Float(0x41bea6c6), SkBits2Float(0x42076a08));  // 23.7156f, 37.1652f, 23.8314f, 33.8535f
path.quadTo(SkBits2Float(0x41bf93f3), SkBits2Float(0x41f455bd), SkBits2Float(0x41d2f77e), SkBits2Float(0x41e2419e));  // 23.9472f, 30.5419f, 26.3708f, 28.282f
path.quadTo(SkBits2Float(0x41e65b07), SkBits2Float(0x41d02d7f), SkBits2Float(0x42006cae), SkBits2Float(0x41d11aac));  // 28.7944f, 26.0222f, 32.1061f, 26.138f
path.quadTo(SkBits2Float(0x420dabd9), SkBits2Float(0x41d207d9), SkBits2Float(0x4216b5e7), SkBits2Float(0x41e56b63));  // 35.4178f, 26.2538f, 37.6776f, 28.6774f
path.quadTo(SkBits2Float(0x421fbff7), SkBits2Float(0x41f8ceed), SkBits2Float(0x421f4961), SkBits2Float(0x4209a6a0));  // 39.9375f, 31.101f, 39.8217f, 34.4127f
path.close();

    SkPath path2(path);
    testPathOpCheck(reporter, path1, path2, (SkPathOp) 2, filename, FLAGS_runFail);
}

static void fuzz763_1597464(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x4101092a), SkBits2Float(0xc2500973));  // 8.06474f, -52.0092f
path.quadTo(SkBits2Float(0x412ef1d8), SkBits2Float(0xc256aade), SkBits2Float(0x41622940), SkBits2Float(0xc2533d84));  // 10.934f, -53.6669f, 14.1351f, -52.8101f
path.quadTo(SkBits2Float(0x418ab055), SkBits2Float(0xc24fd02d), SkBits2Float(0x4197f32a), SkBits2Float(0xc2445602));  // 17.3361f, -51.9533f, 18.9937f, -49.084f
path.quadTo(SkBits2Float(0x41a535ff), SkBits2Float(0xc238dbd6), SkBits2Float(0x419e5b4c), SkBits2Float(0xc22c0dfb));  // 20.6514f, -46.2147f, 19.7946f, -43.0137f
path.quadTo(SkBits2Float(0x4197809e), SkBits2Float(0xc21f4021), SkBits2Float(0x41808c46), SkBits2Float(0xc2189eb6));  // 18.9378f, -39.8126f, 16.0685f, -38.155f
path.quadTo(SkBits2Float(0x41532fdd), SkBits2Float(0xc211fd4c), SkBits2Float(0x411ff875), SkBits2Float(0xc2156aa5));  // 13.1992f, -36.4974f, 9.99816f, -37.3541f
path.quadTo(SkBits2Float(0x40d98218), SkBits2Float(0xc218d7fd), SkBits2Float(0x40a476c4), SkBits2Float(0xc2245229));  // 6.79713f, -38.2109f, 5.1395f, -41.0802f
path.quadTo(SkBits2Float(0x405ed6e0), SkBits2Float(0xc22fcc54), SkBits2Float(0x408ad638), SkBits2Float(0xc23c9a2e));  // 3.48186f, -43.9495f, 4.33865f, -47.1506f
path.quadTo(SkBits2Float(0x40a640f4), SkBits2Float(0xc2496808), SkBits2Float(0x4101092a), SkBits2Float(0xc2500973));  // 5.19543f, -50.3516f, 8.06474f, -52.0092f
path.close();
path.moveTo(SkBits2Float(0xc21ab0b3), SkBits2Float(0xc21a9087));  // -38.6726f, -38.6411f
path.quadTo(SkBits2Float(0xc211524c), SkBits2Float(0xc223f0e2), SkBits2Float(0xc204110f), SkBits2Float(0xc223f243));  // -36.3304f, -40.9852f, -33.0167f, -40.9866f
path.quadTo(SkBits2Float(0xc1ed9fa5), SkBits2Float(0xc223f3a4), SkBits2Float(0xc1dadeef), SkBits2Float(0xc21a953c));  // -29.703f, -40.9879f, -27.3589f, -38.6457f
path.quadTo(SkBits2Float(0xc1c81e38), SkBits2Float(0xc21136d5), SkBits2Float(0xc1c81b76), SkBits2Float(0xc203f598));  // -25.0148f, -36.3035f, -25.0134f, -32.9898f
path.quadTo(SkBits2Float(0xc1c818b4), SkBits2Float(0xc1ed68b6), SkBits2Float(0xc1dad584), SkBits2Float(0xc1daa800));  // -25.0121f, -29.6761f, -27.3543f, -27.332f
path.quadTo(SkBits2Float(0xc1ed9254), SkBits2Float(0xc1c7e74a), SkBits2Float(0xc2040a66), SkBits2Float(0xc1c7e488));  // -29.6964f, -24.9879f, -33.0102f, -24.9866f
path.quadTo(SkBits2Float(0xc2114ba3), SkBits2Float(0xc1c7e1c6), SkBits2Float(0xc21aabfe), SkBits2Float(0xc1da9e95));  // -36.3239f, -24.9852f, -38.668f, -27.3274f
path.quadTo(SkBits2Float(0xc2240c5a), SkBits2Float(0xc1ed5b65), SkBits2Float(0xc2240dbb), SkBits2Float(0xc203eeef));  // -41.0121f, -29.6696f, -41.0134f, -32.9833f
path.quadTo(SkBits2Float(0xc2240f1c), SkBits2Float(0xc211302c), SkBits2Float(0xc21ab0b3), SkBits2Float(0xc21a9087));  // -41.0148f, -36.297f, -38.6726f, -38.6411f
path.close();
path.moveTo(SkBits2Float(0xc19e6455), SkBits2Float(0xc19e6455));  // -19.799f, -19.799f
path.quadTo(SkBits2Float(0xc1399153), SkBits2Float(0xc1e00000), SkBits2Float(0x00000000), SkBits2Float(0xc1e00000));  // -11.598f, -28, 0, -28
path.quadTo(SkBits2Float(0x41399153), SkBits2Float(0xc1e00000), SkBits2Float(0x419e6455), SkBits2Float(0xc19e6455));  // 11.598f, -28, 19.799f, -19.799f
path.quadTo(SkBits2Float(0x41e00000), SkBits2Float(0xc1399153), SkBits2Float(0x41e00000), SkBits2Float(0x00000000));  // 28, -11.598f, 28, 0
path.quadTo(SkBits2Float(0x41e00000), SkBits2Float(0x41399153), SkBits2Float(0x419e6455), SkBits2Float(0x419e6455));  // 28, 11.598f, 19.799f, 19.799f
path.quadTo(SkBits2Float(0x415b75ce), SkBits2Float(0x41cf0dc3), SkBits2Float(0x40b878fc), SkBits2Float(0x41db9f74));  // 13.7163f, 25.8817f, 5.76477f, 27.4529f
path.quadTo(SkBits2Float(0x41000000), SkBits2Float(0x41ee1ba4), SkBits2Float(0x41000000), SkBits2Float(0x42040000));  // 8, 29.7635f, 8, 33
path.quadTo(SkBits2Float(0x41000000), SkBits2Float(0x4211413d), SkBits2Float(0x40b504f3), SkBits2Float(0x421aa09e));  // 8, 36.3137f, 5.65685f, 38.6569f
path.quadTo(SkBits2Float(0x405413cd), SkBits2Float(0x42240000), SkBits2Float(0x00000000), SkBits2Float(0x42240000));  // 3.31371f, 41, 0, 41
path.quadTo(SkBits2Float(0xc05413cd), SkBits2Float(0x42240000), SkBits2Float(0xc0b504f3), SkBits2Float(0x421aa09e));  // -3.31371f, 41, -5.65685f, 38.6569f
path.quadTo(SkBits2Float(0xc1000000), SkBits2Float(0x4211413d), SkBits2Float(0xc1000000), SkBits2Float(0x42040000));  // -8, 36.3137f, -8, 33
path.quadTo(SkBits2Float(0xc1000000), SkBits2Float(0x41ee1ba4), SkBits2Float(0xc0b878fc), SkBits2Float(0x41db9f74));  // -8, 29.7635f, -5.76477f, 27.4529f
path.quadTo(SkBits2Float(0xc15b75ce), SkBits2Float(0x41cf0dc3), SkBits2Float(0xc19e6455), SkBits2Float(0x419e6455));  // -13.7163f, 25.8817f, -19.799f, 19.799f
path.quadTo(SkBits2Float(0xc1e00000), SkBits2Float(0x41399153), SkBits2Float(0xc1e00000), SkBits2Float(0x00000000));  // -28, 11.598f, -28, 0
path.quadTo(SkBits2Float(0xc1e00000), SkBits2Float(0xc1399153), SkBits2Float(0xc19e6455), SkBits2Float(0xc19e6455));  // -28, -11.598f, -19.799f, -19.799f
path.close();
path.moveTo(SkBits2Float(0xc21564da), SkBits2Float(0x41204f08));  // -37.3485f, 10.0193f
path.quadTo(SkBits2Float(0xc211fc8d), SkBits2Float(0x41536ca0), SkBits2Float(0xc2189a40), SkBits2Float(0x4180a176));  // -36.4966f, 13.214f, -38.1506f, 16.0788f
path.quadTo(SkBits2Float(0xc21f3ade), SkBits2Float(0x419796b9), SkBits2Float(0xc22c087c), SkBits2Float(0x419e7330));  // -39.8075f, 18.9486f, -43.0083f, 19.8062f
path.quadTo(SkBits2Float(0xc238d61a), SkBits2Float(0x41a54fa9), SkBits2Float(0xc24450bb), SkBits2Float(0x41980e6c));  // -46.2091f, 20.6639f, -49.0788f, 19.007f
path.quadTo(SkBits2Float(0xc24fc815), SkBits2Float(0x418ad0f8), SkBits2Float(0xc25337a2), SkBits2Float(0x4162812a));  // -51.9454f, 17.352f, -52.8043f, 14.1565f
path.quadTo(SkBits2Float(0xc25336a6), SkBits2Float(0x41628fdb), SkBits2Float(0xc25335aa), SkBits2Float(0x41629e8a));  // -52.8034f, 14.1601f, -52.8024f, 14.1637f
path.quadTo(SkBits2Float(0xc24fc68b), SkBits2Float(0x418aea08), SkBits2Float(0xc2444b74), SkBits2Float(0x419829ad));  // -51.9439f, 17.3643f, -49.0737f, 19.0203f
path.quadTo(SkBits2Float(0xc238d05c), SkBits2Float(0x41a56952), SkBits2Float(0xc22c02fc), SkBits2Float(0x419e8b10));  // -46.2035f, 20.6764f, -43.0029f, 19.8179f
path.quadTo(SkBits2Float(0xc21f359c), SkBits2Float(0x4197acd2), SkBits2Float(0xc21895c9), SkBits2Float(0x4180b6a4));  // -39.8024f, 18.9594f, -38.1463f, 16.0892f
path.quadTo(SkBits2Float(0xc211f634), SkBits2Float(0x41538295), SkBits2Float(0xc21564da), SkBits2Float(0x41204f08));  // -36.4904f, 13.2194f, -37.3485f, 10.0193f
path.close();
path.moveTo(SkBits2Float(0x41dacdf1), SkBits2Float(0x41daaf93));  // 27.3506f, 27.3357f
path.quadTo(SkBits2Float(0x41ed8b66), SkBits2Float(0x41c7ef83), SkBits2Float(0x420406f0), SkBits2Float(0x41c7edac));  // 29.6931f, 24.9919f, 33.0068f, 24.9911f
path.quadTo(SkBits2Float(0x4211482d), SkBits2Float(0x41c7ebd5), SkBits2Float(0x421aa834), SkBits2Float(0x41daa94b));  // 36.3205f, 24.9902f, 38.6643f, 27.3327f
path.quadTo(SkBits2Float(0x4224083d), SkBits2Float(0x41ed66c1), SkBits2Float(0x42240928), SkBits2Float(0x4203f49d));  // 41.008f, 29.6752f, 41.0089f, 32.9889f
path.quadTo(SkBits2Float(0x42240a14), SkBits2Float(0x421135da), SkBits2Float(0x421aab58), SkBits2Float(0x421a95e2));  // 41.0098f, 36.3026f, 38.6673f, 38.6464f
path.quadTo(SkBits2Float(0x421aa5fc), SkBits2Float(0x421a9b42), SkBits2Float(0x421aa09e), SkBits2Float(0x421aa09e));  // 38.6621f, 38.6516f, 38.6569f, 38.6569f
path.quadTo(SkBits2Float(0x4211413d), SkBits2Float(0x42240000), SkBits2Float(0x42040000), SkBits2Float(0x42240000));  // 36.3137f, 41, 33, 41
path.quadTo(SkBits2Float(0x41ed7d86), SkBits2Float(0x42240000), SkBits2Float(0x41dabec3), SkBits2Float(0x421aa09e));  // 29.6863f, 41, 27.3431f, 38.6569f
path.quadTo(SkBits2Float(0x41c80000), SkBits2Float(0x4211413d), SkBits2Float(0x41c80000), SkBits2Float(0x42040000));  // 25, 36.3137f, 25, 33
path.quadTo(SkBits2Float(0x41c80000), SkBits2Float(0x41ed7d86), SkBits2Float(0x41dabec3), SkBits2Float(0x41dabec3));  // 25, 29.6863f, 27.3431f, 27.3431f
path.quadTo(SkBits2Float(0x41dac1e5), SkBits2Float(0x41dabba1), SkBits2Float(0x41dac507), SkBits2Float(0x41dab880));  // 27.3447f, 27.3416f, 27.3462f, 27.3401f
path.lineTo(SkBits2Float(0x41dac551), SkBits2Float(0x41dab836));  // 27.3463f, 27.3399f
path.quadTo(SkBits2Float(0x41dac9a0), SkBits2Float(0x41dab3e4), SkBits2Float(0x41dacdf1), SkBits2Float(0x41daaf93));  // 27.3484f, 27.3378f, 27.3506f, 27.3357f
path.close();
path.moveTo(SkBits2Float(0xc23c98fa), SkBits2Float(0x408b3eec));  // -47.1494f, 4.35143f
path.quadTo(SkBits2Float(0xc22fcb5c), SkBits2Float(0x405f9a18), SkBits2Float(0xc22450bb), SkBits2Float(0x40a4d200));  // -43.9486f, 3.49378f, -41.0788f, 5.15063f
path.quadTo(SkBits2Float(0xc218d61a), SkBits2Float(0x40d9d6f4), SkBits2Float(0xc21567dd), SkBits2Float(0x412021ef));  // -38.2091f, 6.80749f, -37.3514f, 10.0083f
path.quadTo(SkBits2Float(0xc215672b), SkBits2Float(0x41202c49), SkBits2Float(0xc215667a), SkBits2Float(0x412036a3));  // -37.3507f, 10.0108f, -37.3501f, 10.0133f
path.lineTo(SkBits2Float(0xc2156516), SkBits2Float(0x41204b6b));  // -37.3487f, 10.0184f
path.lineTo(SkBits2Float(0xc2533998), SkBits2Float(0x416263e8));  // -52.8062f, 14.1494f
path.quadTo(SkBits2Float(0xc256a7d4), SkBits2Float(0x412f2d72), SkBits2Float(0xc2500736), SkBits2Float(0x410142ec));  // -53.6639f, 10.9486f, -52.007f, 8.07884f
path.quadTo(SkBits2Float(0xc2496698), SkBits2Float(0x40a6b0cc), SkBits2Float(0xc23c98fa), SkBits2Float(0x408b3eec));  // -50.3502f, 5.20908f, -47.1494f, 4.35143f
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0xc2444b74), SkBits2Float(0x419829ad));  // -49.0737f, 19.0203f
path.quadTo(SkBits2Float(0xc24fc68b), SkBits2Float(0x418aea08), SkBits2Float(0xc25335aa), SkBits2Float(0x41629e8a));  // -51.9439f, 17.3643f, -52.8024f, 14.1637f
path.quadTo(SkBits2Float(0xc256a4ca), SkBits2Float(0x412f6908), SkBits2Float(0xc25004f8), SkBits2Float(0x41017cac));  // -53.6609f, 10.9631f, -52.0049f, 8.09294f
path.quadTo(SkBits2Float(0xc2496525), SkBits2Float(0x40a7209c), SkBits2Float(0xc23c97c4), SkBits2Float(0x408ba7a8));  // -50.3488f, 5.22273f, -47.1482f, 4.36422f
path.quadTo(SkBits2Float(0xc22fca64), SkBits2Float(0x40605d58), SkBits2Float(0xc2244f4d), SkBits2Float(0x40a52d40));  // -43.9476f, 3.5057f, -41.0774f, 5.16177f
path.quadTo(SkBits2Float(0xc218d435), SkBits2Float(0x40da2bd2), SkBits2Float(0xc2156516), SkBits2Float(0x41204b6b));  // -38.2072f, 6.81785f, -37.3487f, 10.0184f
path.quadTo(SkBits2Float(0xc211f5f7), SkBits2Float(0x415380eb), SkBits2Float(0xc21895c9), SkBits2Float(0x4180b6a4));  // -36.4902f, 13.219f, -38.1463f, 16.0892f
path.quadTo(SkBits2Float(0xc21f359c), SkBits2Float(0x4197acd2), SkBits2Float(0xc22c02fc), SkBits2Float(0x419e8b10));  // -39.8024f, 18.9594f, -43.0029f, 19.8179f
path.quadTo(SkBits2Float(0xc238d05c), SkBits2Float(0x41a56952), SkBits2Float(0xc2444b74), SkBits2Float(0x419829ad));  // -46.2035f, 20.6764f, -49.0737f, 19.0203f
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}

// SkOpSegment.cpp:4010: failed assertion "span->fOppSum == -0x7FFFFFFF || span->fOppSum == oppWinding
static void fuzz763_34974(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
#if 0
path.moveTo(SkBits2Float(0x41015326), SkBits2Float(0xc2500694));
path.quadTo(SkBits2Float(0x412f3e30), SkBits2Float(0xc256a6fa), SkBits2Float(0x41627462), SkBits2Float(0xc253387e));
path.quadTo(SkBits2Float(0x418ad549), SkBits2Float(0xc24fca02), SkBits2Float(0x41981613), SkBits2Float(0xc2444f40));
path.quadTo(SkBits2Float(0x41a556de), SkBits2Float(0xc238d47d), SkBits2Float(0x419e79e6), SkBits2Float(0xc22c06f0));
path.quadTo(SkBits2Float(0x41979cee), SkBits2Float(0xc21f3964), SkBits2Float(0x4180a76a), SkBits2Float(0xc21898ff));
path.quadTo(SkBits2Float(0x415363c9), SkBits2Float(0xc211f89a), SkBits2Float(0x41202d96), SkBits2Float(0xc2156716));
path.quadTo(SkBits2Float(0x40d9eeca), SkBits2Float(0xc218d592), SkBits2Float(0x40a4eba0), SkBits2Float(0xc2245054));
path.quadTo(SkBits2Float(0x405fd0f0), SkBits2Float(0xc22fcb17), SkBits2Float(0x408b5c58), SkBits2Float(0xc23c98a3));
path.quadTo(SkBits2Float(0x40a6d038), SkBits2Float(0xc249662f), SkBits2Float(0x41015326), SkBits2Float(0xc2500694));
path.close();
path.moveTo(SkBits2Float(0xc21a9c18), SkBits2Float(0xc21aa524));
path.quadTo(SkBits2Float(0xc2113c71), SkBits2Float(0xc2240440), SkBits2Float(0xc203fb34), SkBits2Float(0xc22403dc));
path.quadTo(SkBits2Float(0xc1ed73ee), SkBits2Float(0xc2240379), SkBits2Float(0xc1dab5b7), SkBits2Float(0xc21aa3d1));
path.quadTo(SkBits2Float(0xc1c7f781), SkBits2Float(0xc211442a), SkBits2Float(0xc1c7f847), SkBits2Float(0xc20402ed));
path.quadTo(SkBits2Float(0xc1c7f90e), SkBits2Float(0xc1ed835f), SkBits2Float(0xc1dab85d), SkBits2Float(0xc1dac529));
path.quadTo(SkBits2Float(0xc1ed77ad), SkBits2Float(0xc1c806f2), SkBits2Float(0xc203fd13), SkBits2Float(0xc1c807b9));
path.quadTo(SkBits2Float(0xc2113e50), SkBits2Float(0xc1c8087f), SkBits2Float(0xc21a9d6b), SkBits2Float(0xc1dac7cf));
path.quadTo(SkBits2Float(0xc223fc87), SkBits2Float(0xc1ed871e), SkBits2Float(0xc223fc24), SkBits2Float(0xc20404cc));
path.quadTo(SkBits2Float(0xc223fbc0), SkBits2Float(0xc2114609), SkBits2Float(0xc21a9c18), SkBits2Float(0xc21aa524));
path.close();
path.moveTo(SkBits2Float(0xc19e6455), SkBits2Float(0xc19e6455));
path.quadTo(SkBits2Float(0xc1399153), SkBits2Float(0xc1e00000), SkBits2Float(0x00000000), SkBits2Float(0xc1e00000));
path.quadTo(SkBits2Float(0x41399153), SkBits2Float(0xc1e00000), SkBits2Float(0x419e6455), SkBits2Float(0xc19e6455));
path.quadTo(SkBits2Float(0x41e00000), SkBits2Float(0xc1399153), SkBits2Float(0x41e00000), SkBits2Float(0x00000000));
path.quadTo(SkBits2Float(0x41e00000), SkBits2Float(0x41399153), SkBits2Float(0x419e6455), SkBits2Float(0x419e6455));
path.quadTo(SkBits2Float(0x415b75ce), SkBits2Float(0x41cf0dc3), SkBits2Float(0x40b878fc), SkBits2Float(0x41db9f74));
path.quadTo(SkBits2Float(0x41000000), SkBits2Float(0x41ee1ba4), SkBits2Float(0x41000000), SkBits2Float(0x42040000));
path.quadTo(SkBits2Float(0x41000000), SkBits2Float(0x4211413d), SkBits2Float(0x40b504f3), SkBits2Float(0x421aa09e));
path.quadTo(SkBits2Float(0x405413cd), SkBits2Float(0x42240000), SkBits2Float(0x00000000), SkBits2Float(0x42240000));
path.quadTo(SkBits2Float(0xc05413cd), SkBits2Float(0x42240000), SkBits2Float(0xc0b504f3), SkBits2Float(0x421aa09e));
path.quadTo(SkBits2Float(0xc1000000), SkBits2Float(0x4211413d), SkBits2Float(0xc1000000), SkBits2Float(0x42040000));
path.quadTo(SkBits2Float(0xc1000000), SkBits2Float(0x41ee1ba4), SkBits2Float(0xc0b878fc), SkBits2Float(0x41db9f74));
path.quadTo(SkBits2Float(0xc15b75ce), SkBits2Float(0x41cf0dc3), SkBits2Float(0xc19e6455), SkBits2Float(0x419e6455));
path.quadTo(SkBits2Float(0xc1e00000), SkBits2Float(0x41399153), SkBits2Float(0xc1e00000), SkBits2Float(0x00000000));
path.quadTo(SkBits2Float(0xc1e00000), SkBits2Float(0xc1399153), SkBits2Float(0xc19e6455), SkBits2Float(0xc19e6455));
path.close();
#endif
#if 01
path.moveTo(SkBits2Float(0xc2533a24), SkBits2Float(0x41625bba));
path.lineTo(SkBits2Float(0xc2533ab2), SkBits2Float(0x4162536e));
path.lineTo(SkBits2Float(0xc2533af7), SkBits2Float(0x41624f68));
path.quadTo(SkBits2Float(0xc2533a8e), SkBits2Float(0x41625591), SkBits2Float(0xc2533a24), SkBits2Float(0x41625bba));
path.close();
#endif
#if 0
path.moveTo(SkBits2Float(0x41dac664), SkBits2Float(0x41dab723));
path.quadTo(SkBits2Float(0x41ed82ea), SkBits2Float(0x41c80000), SkBits2Float(0x42040000), SkBits2Float(0x41c80000));
path.quadTo(SkBits2Float(0x4211413d), SkBits2Float(0x41c80000), SkBits2Float(0x421aa09e), SkBits2Float(0x41dabec3));
path.quadTo(SkBits2Float(0x42240000), SkBits2Float(0x41ed7d86), SkBits2Float(0x42240000), SkBits2Float(0x42040000));
path.quadTo(SkBits2Float(0x42240000), SkBits2Float(0x4211413d), SkBits2Float(0x421aa09e), SkBits2Float(0x421aa09e));
path.lineTo(SkBits2Float(0x421a9d9a), SkBits2Float(0x421aa3a2));
path.quadTo(SkBits2Float(0x42113e0a), SkBits2Float(0x422402d5), SkBits2Float(0x4203fccd), SkBits2Float(0x42240293));
path.quadTo(SkBits2Float(0x41ed7721), SkBits2Float(0x42240251), SkBits2Float(0x41dab8bb), SkBits2Float(0x421aa2c0));
path.quadTo(SkBits2Float(0x41c7fa56), SkBits2Float(0x42114330), SkBits2Float(0x41c7fada), SkBits2Float(0x420401f3));
path.quadTo(SkBits2Float(0x41c7fb5f), SkBits2Float(0x41ed9352), SkBits2Float(0x41daa13c), SkBits2Float(0x41dadc57));
path.quadTo(SkBits2Float(0x41daa91d), SkBits2Float(0x41dad46f), SkBits2Float(0x41dab101), SkBits2Float(0x41dacc89));
path.quadTo(SkBits2Float(0x41dab5bf), SkBits2Float(0x41dac7c8), SkBits2Float(0x41daba7f), SkBits2Float(0x41dac307));
path.lineTo(SkBits2Float(0x41dabec3), SkBits2Float(0x41dabec3));
path.quadTo(SkBits2Float(0x41dac293), SkBits2Float(0x41dabaf3), SkBits2Float(0x41dac664), SkBits2Float(0x41dab723));
path.close();
#endif
#if 01
path.moveTo(SkBits2Float(0xc23c9951), SkBits2Float(0x408b2180));
path.quadTo(SkBits2Float(0xc22fcba2), SkBits2Float(0x405f6340), SkBits2Float(0xc2245122), SkBits2Float(0x40a4b85c));
path.quadTo(SkBits2Float(0xc218dd36), SkBits2Float(0x40d9a0b8), SkBits2Float(0xc2156c96), SkBits2Float(0x411fdb9a));
path.lineTo(SkBits2Float(0xc2156b9c), SkBits2Float(0x411fea15));
path.quadTo(SkBits2Float(0xc2156a20), SkBits2Float(0x4120002c), SkBits2Float(0xc21568a5), SkBits2Float(0x41201647));
path.lineTo(SkBits2Float(0xc21568a3), SkBits2Float(0x41201660));
path.lineTo(SkBits2Float(0xc2156841), SkBits2Float(0x41201c29));
path.quadTo(SkBits2Float(0xc215680f), SkBits2Float(0x41201f0a), SkBits2Float(0xc21567dd), SkBits2Float(0x412021ef));
path.quadTo(SkBits2Float(0xc21562d2), SkBits2Float(0x41206d52), SkBits2Float(0xc2155ca3), SkBits2Float(0x4120cb63));
path.quadTo(SkBits2Float(0xc212057d), SkBits2Float(0x4153a15f), SkBits2Float(0xc2189adf), SkBits2Float(0x41809e82));
path.quadTo(SkBits2Float(0xc21f3b9a), SkBits2Float(0x419793a4), SkBits2Float(0xc22c0940), SkBits2Float(0x419e6fdc));
path.quadTo(SkBits2Float(0xc238d6e6), SkBits2Float(0x41a54c16), SkBits2Float(0xc2445177), SkBits2Float(0x41980aa0));
path.quadTo(SkBits2Float(0xc24fcb1e), SkBits2Float(0x418aca39), SkBits2Float(0xc2533998), SkBits2Float(0x416263e8));
path.quadTo(SkBits2Float(0xc24fcb5c), SkBits2Float(0x418acd2f), SkBits2Float(0xc24450bb), SkBits2Float(0x41980e6c));
path.quadTo(SkBits2Float(0xc238d61a), SkBits2Float(0x41a54fa9), SkBits2Float(0xc22c087c), SkBits2Float(0x419e7330));
path.quadTo(SkBits2Float(0xc21f3ade), SkBits2Float(0x419796b9), SkBits2Float(0xc2189a40), SkBits2Float(0x4180a176));
path.lineTo(SkBits2Float(0xc2533b22), SkBits2Float(0x41624cea));
path.quadTo(SkBits2Float(0xc256a842), SkBits2Float(0x412f19c8), SkBits2Float(0xc25007d7), SkBits2Float(0x410132b2));
path.quadTo(SkBits2Float(0xc24966ff), SkBits2Float(0x40a69160), SkBits2Float(0xc23c9951), SkBits2Float(0x408b2180));
path.close();
#endif

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0xc2445236), SkBits2Float(0x419806c2));
path.quadTo(SkBits2Float(0xc24fccb6), SkBits2Float(0x418ac513), SkBits2Float(0xc2533ab2), SkBits2Float(0x4162536e));
path.quadTo(SkBits2Float(0xc256a8ae), SkBits2Float(0x412f1cb2), SkBits2Float(0xc25007d7), SkBits2Float(0x410132b2));
path.quadTo(SkBits2Float(0xc24966ff), SkBits2Float(0x40a69160), SkBits2Float(0xc23c9951), SkBits2Float(0x408b2180));
path.quadTo(SkBits2Float(0xc22fcba2), SkBits2Float(0x405f6340), SkBits2Float(0xc2245122), SkBits2Float(0x40a4b85c));
path.quadTo(SkBits2Float(0xc218d6a2), SkBits2Float(0x40d9bf1c), SkBits2Float(0xc21568a5), SkBits2Float(0x41201647));
path.quadTo(SkBits2Float(0xc211faaa), SkBits2Float(0x41534d02), SkBits2Float(0xc2189b82), SkBits2Float(0x41809b82));
path.quadTo(SkBits2Float(0xc21f3c59), SkBits2Float(0x41979082), SkBits2Float(0xc22c0a07), SkBits2Float(0x419e6c7a));
path.quadTo(SkBits2Float(0xc238d7b6), SkBits2Float(0x41a54872), SkBits2Float(0xc2445236), SkBits2Float(0x419806c2));
path.close();

    SkPath path2(path);
    testPathOpCheck(reporter, path1, path2, (SkPathOp) 2, filename, FLAGS_runFail);
}

static void fuzz763_2211264(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x41017a68), SkBits2Float(0xc250050e));
path.quadTo(SkBits2Float(0x412f66b2), SkBits2Float(0xc256a4e9), SkBits2Float(0x41629c3c), SkBits2Float(0xc25335d1));
path.quadTo(SkBits2Float(0x418ae8e6), SkBits2Float(0xc24fc6bc), SkBits2Float(0x4198289b), SkBits2Float(0xc2444ba9));
path.quadTo(SkBits2Float(0x41a56850), SkBits2Float(0xc238d096), SkBits2Float(0x419e8a20), SkBits2Float(0xc22c0333));
path.quadTo(SkBits2Float(0x4197abf4), SkBits2Float(0xc21f35d0), SkBits2Float(0x4180b5d0), SkBits2Float(0xc21895f6));
path.quadTo(SkBits2Float(0x41537f55), SkBits2Float(0xc211f61c), SkBits2Float(0x412049c9), SkBits2Float(0xc2156532));
path.quadTo(SkBits2Float(0x40da287c), SkBits2Float(0xc218d449), SkBits2Float(0x40a529a8), SkBits2Float(0xc2244f5b));
path.quadTo(SkBits2Float(0x406055a8), SkBits2Float(0xc22fca6e), SkBits2Float(0x408ba388), SkBits2Float(0xc23c97d0));
path.quadTo(SkBits2Float(0x40a71c3c), SkBits2Float(0xc2496534), SkBits2Float(0x41017a68), SkBits2Float(0xc250050e));
path.close();
path.moveTo(SkBits2Float(0xc21a9126), SkBits2Float(0xc21ab014));
path.quadTo(SkBits2Float(0xc21130d5), SkBits2Float(0xc2240e86), SkBits2Float(0xc203ef98), SkBits2Float(0xc2240d33));
path.quadTo(SkBits2Float(0xc1ed5cb7), SkBits2Float(0xc2240bdf), SkBits2Float(0xc1da9fd4), SkBits2Float(0xc21aab8d));
path.quadTo(SkBits2Float(0xc1c7e2f1), SkBits2Float(0xc2114b3c), SkBits2Float(0xc1c7e598), SkBits2Float(0xc20409ff));
path.quadTo(SkBits2Float(0xc1c7e83e), SkBits2Float(0xc1ed9186), SkBits2Float(0xc1daa8e1), SkBits2Float(0xc1dad4a3));
path.quadTo(SkBits2Float(0xc1ed6984), SkBits2Float(0xc1c817c0), SkBits2Float(0xc203f5ff), SkBits2Float(0xc1c81a66));
path.quadTo(SkBits2Float(0xc211373c), SkBits2Float(0xc1c81d0d), SkBits2Float(0xc21a95ad), SkBits2Float(0xc1daddb0));
path.quadTo(SkBits2Float(0xc223f41f), SkBits2Float(0xc1ed9e53), SkBits2Float(0xc223f2cb), SkBits2Float(0xc2041066));
path.quadTo(SkBits2Float(0xc223f178), SkBits2Float(0xc21151a3), SkBits2Float(0xc21a9126), SkBits2Float(0xc21ab014));
path.close();
path.moveTo(SkBits2Float(0xc19e6455), SkBits2Float(0xc19e6455));
path.quadTo(SkBits2Float(0xc1399153), SkBits2Float(0xc1e00000), SkBits2Float(0x00000000), SkBits2Float(0xc1e00000));
path.quadTo(SkBits2Float(0x41399153), SkBits2Float(0xc1e00000), SkBits2Float(0x419e6455), SkBits2Float(0xc19e6455));
path.quadTo(SkBits2Float(0x41e00000), SkBits2Float(0xc1399153), SkBits2Float(0x41e00000), SkBits2Float(0x00000000));
path.quadTo(SkBits2Float(0x41e00000), SkBits2Float(0x41399153), SkBits2Float(0x419e6455), SkBits2Float(0x419e6455));
path.quadTo(SkBits2Float(0x415b75ce), SkBits2Float(0x41cf0dc3), SkBits2Float(0x40b878fc), SkBits2Float(0x41db9f74));
path.quadTo(SkBits2Float(0x41000000), SkBits2Float(0x41ee1ba4), SkBits2Float(0x41000000), SkBits2Float(0x42040000));
path.quadTo(SkBits2Float(0x41000000), SkBits2Float(0x4211413d), SkBits2Float(0x40b504f3), SkBits2Float(0x421aa09e));
path.quadTo(SkBits2Float(0x405413cd), SkBits2Float(0x42240000), SkBits2Float(0x00000000), SkBits2Float(0x42240000));
path.quadTo(SkBits2Float(0xc05413cd), SkBits2Float(0x42240000), SkBits2Float(0xc0b504f3), SkBits2Float(0x421aa09e));
path.quadTo(SkBits2Float(0xc1000000), SkBits2Float(0x4211413d), SkBits2Float(0xc1000000), SkBits2Float(0x42040000));
path.quadTo(SkBits2Float(0xc1000000), SkBits2Float(0x41ee1ba4), SkBits2Float(0xc0b878fc), SkBits2Float(0x41db9f74));
path.quadTo(SkBits2Float(0xc15b75ce), SkBits2Float(0x41cf0dc3), SkBits2Float(0xc19e6455), SkBits2Float(0x419e6455));
path.quadTo(SkBits2Float(0xc1e00000), SkBits2Float(0x41399153), SkBits2Float(0xc1e00000), SkBits2Float(0x00000000));
path.quadTo(SkBits2Float(0xc1e00000), SkBits2Float(0xc1399153), SkBits2Float(0xc19e6455), SkBits2Float(0xc19e6455));
path.close();
path.moveTo(SkBits2Float(0x41dabec3), SkBits2Float(0x41dabec3));
path.quadTo(SkBits2Float(0x41ed7d86), SkBits2Float(0x41c80000), SkBits2Float(0x42040000), SkBits2Float(0x41c80000));
path.quadTo(SkBits2Float(0x4211413d), SkBits2Float(0x41c80000), SkBits2Float(0x421aa09e), SkBits2Float(0x41dabec3));
path.quadTo(SkBits2Float(0x42240000), SkBits2Float(0x41ed7d86), SkBits2Float(0x42240000), SkBits2Float(0x42040000));
path.quadTo(SkBits2Float(0x42240000), SkBits2Float(0x4211413d), SkBits2Float(0x421aa09e), SkBits2Float(0x421aa09e));
path.quadTo(SkBits2Float(0x421a9e0c), SkBits2Float(0x421aa331), SkBits2Float(0x421a9b79), SkBits2Float(0x421aa5c3));
path.quadTo(SkBits2Float(0x421a98e4), SkBits2Float(0x421aa858), SkBits2Float(0x421a964e), SkBits2Float(0x421aaaec));
path.lineTo(SkBits2Float(0x421a943a), SkBits2Float(0x421aad00));
path.quadTo(SkBits2Float(0x421134d5), SkBits2Float(0x422409ae), SkBits2Float(0x4203f510), SkBits2Float(0x422408cc));
path.quadTo(SkBits2Float(0x41ed67a7), SkBits2Float(0x422407e9), SkBits2Float(0x41daaa24), SkBits2Float(0x421aa7e8));
path.quadTo(SkBits2Float(0x41c7eca1), SkBits2Float(0x421147e7), SkBits2Float(0x41c7ee66), SkBits2Float(0x420406aa));
path.quadTo(SkBits2Float(0x41c7f02a), SkBits2Float(0x41ed8ada), SkBits2Float(0x41dab02f), SkBits2Float(0x41dacd55));
path.lineTo(SkBits2Float(0x41dab02d), SkBits2Float(0x41dacd57));
path.quadTo(SkBits2Float(0x41dab3d4), SkBits2Float(0x41dac9b0), SkBits2Float(0x41dab77c), SkBits2Float(0x41dac60a));
path.quadTo(SkBits2Float(0x41dab83b), SkBits2Float(0x41dac54b), SkBits2Float(0x41dab8fa), SkBits2Float(0x41dac48d));
path.quadTo(SkBits2Float(0x41dabbde), SkBits2Float(0x41dac1a8), SkBits2Float(0x41dabec3), SkBits2Float(0x41dabec3));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0xc24455cc), SkBits2Float(0x4197f43e));
path.quadTo(SkBits2Float(0xc24fcffc), SkBits2Float(0x418ab179), SkBits2Float(0xc2533d5e), SkBits2Float(0x41622b92));
path.quadTo(SkBits2Float(0xc256aac0), SkBits2Float(0x412ef432), SkBits2Float(0xc250095d), SkBits2Float(0x41010b70));
path.quadTo(SkBits2Float(0xc24967fb), SkBits2Float(0x40a64560), SkBits2Float(0xc23c9a22), SkBits2Float(0x408ada54));
path.quadTo(SkBits2Float(0xc22fcc4a), SkBits2Float(0x405ede90), SkBits2Float(0xc224521a), SkBits2Float(0x40a47a5c));
path.quadTo(SkBits2Float(0xc218d7ea), SkBits2Float(0x40d9856e), SkBits2Float(0xc2156a88), SkBits2Float(0x411ffa16));
path.quadTo(SkBits2Float(0xc211fd27), SkBits2Float(0x41533178), SkBits2Float(0xc2189e8a), SkBits2Float(0x41808d1c));
path.quadTo(SkBits2Float(0xc21f3fec), SkBits2Float(0x4197817d), SkBits2Float(0xc22c0dc4), SkBits2Float(0x419e5c3f));
path.quadTo(SkBits2Float(0xc238db9c), SkBits2Float(0x41a53702), SkBits2Float(0xc24455cc), SkBits2Float(0x4197f43e));
path.close();

    SkPath path2(path);
    testPathOp(reporter, path1, path2, (SkPathOp) 2, filename);
}

static void (*firstTest)(skiatest::Reporter* , const char* filename) = 0;
static void (*stopTest)(skiatest::Reporter* , const char* filename) = 0;

static struct TestDesc tests[] = {
    TEST(fuzz763_2211264),
    TEST(fuzz763_34974),
    TEST(fuzz763_1597464),
    TEST(cubicOp115),
    TEST(fuzz763_849020),
    TEST(fuzz763_24588),
    TEST(fuzz763_20016),
    TEST(fuzz763_17370),
    TEST(fuzz763_35322),
    TEST(fuzz763_8712),
    TEST(fuzz763_8712a),
    TEST(fuzz763_4014),
    TEST(fuzz763_4014a),
    TEST(fuzz763_1404),
    TEST(fuzz763_4713),
    TEST(fuzz763_378),
    TEST(fuzz763_378b),
    TEST(fuzz763_378d),
    TEST(fuzz763_378c),
    TEST(fuzz763_3084),
    TEST(fuzz763_1823),
    TEST(fuzz763_558),
    TEST(fuzz763_378a),
    TEST(fuzz763_378a_1),
    TEST(issue2753),  // FIXME: pair of cubics miss intersection
    TEST(cubicOp114),  // FIXME: curve with inflection is ordered the wrong way
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
    // fails because a cubic/quadratic intersection is missed
    // the internal quad/quad is far enough away from the real cubic/quad that it is rejected
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
    TEST(skpcarpetplanet_ru22),  // cubic/cubic intersect detects unwanted coincidence
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
    TEST(cubicOp58d),
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
    TEST(cubicOp58d),
    TEST(cubicOp53d),
};

static const size_t subTestCount = SK_ARRAY_COUNT(subTests);

static void (*firstSubTest)(skiatest::Reporter* , const char* filename) = 0;

static bool runSubTests = false;
static bool runSubTestsFirst = false;
static bool runReverse = false;

DEF_TEST(PathOpsOp, reporter) {
#if DEBUG_SHOW_TEST_NAME
    strncpy(DEBUG_FILENAME_STRING, "", DEBUG_FILENAME_STRING_LENGTH);
#endif
    if (runSubTests && runSubTestsFirst) {
        RunTestSet(reporter, subTests, subTestCount, firstSubTest, stopTest, runReverse);
    }
    RunTestSet(reporter, tests, testCount, firstTest, stopTest, runReverse);
    if (runSubTests && !runSubTestsFirst) {
        RunTestSet(reporter, subTests, subTestCount, firstSubTest, stopTest, runReverse);
    }
}

static void bufferOverflow(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0,0, 300,170141183460469231731687303715884105728.f);
    SkPath pathB;
    pathB.addRect(0,0, 300,16);
    testPathFailOp(reporter, path, pathB, kUnion_PathOp, filename);
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

    testPathFailOp(reporter, path1, path2, kIntersect_PathOp, filename);
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

    testPathFailOp(reporter, path1, path2, kUnion_PathOp, filename);
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
    testPathFailOp(reporter, path1, path2, (SkPathOp) 2, filename);
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
    testPathFailOp(reporter, path1, path2, (SkPathOp) 2, filename);
}

static void fuzz714(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 1);
path.moveTo(SkBits2Float(0x430c0000), SkBits2Float(0x42200000));
path.lineTo(SkBits2Float(0x43480000), SkBits2Float(0x43520000));
path.lineTo(SkBits2Float(0x42200000), SkBits2Float(0x42c80000));
path.lineTo(SkBits2Float(0x64969569), SkBits2Float(0x42c80000));
path.lineTo(SkBits2Float(0x64969569), SkBits2Float(0x43520000));
path.lineTo(SkBits2Float(0x430c0000), SkBits2Float(0x42200000));
path.close();

    SkPath path1(path);
    path.reset();
    path.setFillType((SkPath::FillType) 0);
path.moveTo(SkBits2Float(0x43200000), SkBits2Float(0x42700000));
path.lineTo(SkBits2Float(0x435c0000), SkBits2Float(0x43660000));
path.lineTo(SkBits2Float(0x42700000), SkBits2Float(0x42f00000));
path.lineTo(SkBits2Float(0x64969569), SkBits2Float(0x42f00000));
path.lineTo(SkBits2Float(0x64969569), SkBits2Float(0x43660000));
path.lineTo(SkBits2Float(0x43200000), SkBits2Float(0x42700000));
path.close();

    SkPath path2(path);
    testPathFailOp(reporter, path1, path2, (SkPathOp) 2, filename);
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
    TEST(fuzz1),
    TEST(fuzz714),
    TEST(fuzz487a),
    TEST(fuzz487b),
    TEST(fuzz433b),
    TEST(fuzz433),
    TEST(bufferOverflow),
};

static const size_t failTestCount = SK_ARRAY_COUNT(failTests);

DEF_TEST(PathOpsFailOp, reporter) {
#if DEBUG_SHOW_TEST_NAME
    strncpy(DEBUG_FILENAME_STRING, "", DEBUG_FILENAME_STRING_LENGTH);
#endif
    RunTestSet(reporter, failTests, failTestCount, 0, 0, false);
}
