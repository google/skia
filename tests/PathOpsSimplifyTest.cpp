/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkMatrix.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkTypes.h"
#include "include/pathops/SkPathOps.h"
#include "src/base/SkFloatBits.h"
#include "src/pathops/SkPathOpsDebug.h"
#include "tests/PathOpsExtendedTest.h"
#include "tests/Test.h"

#include <cstddef>
#include <iterator>

#define TEST(name) { name, #name }

static void testDegenerates(skiatest::Reporter* reporter, const char* filename) {
    SkPath doubleback;
    SkPath simple;

    doubleback.lineTo(1, 0);
    REPORTER_ASSERT(reporter, Simplify(doubleback, &simple));
    REPORTER_ASSERT(reporter, simple.isEmpty());

    doubleback.reset();
    doubleback.lineTo(1, 0);
    doubleback.lineTo(2, 0);
    REPORTER_ASSERT(reporter, Simplify(doubleback, &simple));
    REPORTER_ASSERT(reporter, simple.isEmpty());

    doubleback.reset();
    doubleback.lineTo(-1, 0);
    doubleback.lineTo(-1, 1);
    doubleback.lineTo(-1, 0);
    REPORTER_ASSERT(reporter, Simplify(doubleback, &simple));
    REPORTER_ASSERT(reporter, simple.isEmpty());

    doubleback.reset();
    doubleback.lineTo(1, 0);
    doubleback.lineTo(1, 0);
    doubleback.lineTo(1, 1);
    doubleback.lineTo(1, 1);
    doubleback.lineTo(1, 0);
    REPORTER_ASSERT(reporter, Simplify(doubleback, &simple));
    REPORTER_ASSERT(reporter, simple.isEmpty());
}

static void testLine1(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(2,0);
    path.lineTo(1,1);
    path.lineTo(0,0);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testLine1x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.moveTo(2,0);
    path.lineTo(1,1);
    path.lineTo(0,0);
    path.close();
    testSimplify(reporter, path, filename);
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

static void testLine2(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    addInnerCWTriangle(path);
    addOuterCWTriangle(path);
    testSimplify(reporter, path, filename);
}

static void testLine2x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    addInnerCWTriangle(path);
    addOuterCWTriangle(path);
    testSimplify(reporter, path, filename);
}

static void testLine3(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    addInnerCCWTriangle(path);
    addOuterCWTriangle(path);
    testSimplify(reporter, path, filename);
}

static void testLine3x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    addInnerCCWTriangle(path);
    addOuterCWTriangle(path);
    testSimplify(reporter, path, filename);
}

static void testLine3a(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    addInnerCWTriangle(path);
    addOuterCCWTriangle(path);
    testSimplify(reporter, path, filename);
}

static void testLine3ax(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    addInnerCWTriangle(path);
    addOuterCCWTriangle(path);
    testSimplify(reporter, path, filename);
}

static void testLine3b(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    addInnerCCWTriangle(path);
    addOuterCCWTriangle(path);
    testSimplify(reporter, path, filename);
}

static void testLine3bx(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    addInnerCCWTriangle(path);
    addOuterCCWTriangle(path);
    testSimplify(reporter, path, filename);
}

static void testLine4(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    addOuterCCWTriangle(path);
    addOuterCWTriangle(path);
    testSimplify(reporter, path, filename);
}

static void testLine4x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    addOuterCCWTriangle(path);
    addOuterCWTriangle(path);
    testSimplify(reporter, path, filename);
}

static void testLine5(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    addOuterCWTriangle(path);
    addOuterCWTriangle(path);
    testSimplify(reporter, path, filename);
}

static void testLine5x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    addOuterCWTriangle(path);
    addOuterCWTriangle(path);
    testSimplify(reporter, path, filename);
}

static void testLine6(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0,0);
    path.lineTo(4,0);
    path.lineTo(2,2);
    path.close();
    path.moveTo(2,0);
    path.lineTo(6,0);
    path.lineTo(4,2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testLine6x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.moveTo(0,0);
    path.lineTo(4,0);
    path.lineTo(2,2);
    path.close();
    path.moveTo(2,0);
    path.lineTo(6,0);
    path.lineTo(4,2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testLine7(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0,0);
    path.lineTo(4,0);
    path.lineTo(2,2);
    path.close();
    path.moveTo(6,0);
    path.lineTo(2,0);
    path.lineTo(4,2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testLine7x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.moveTo(0,0);
    path.lineTo(4,0);
    path.lineTo(2,2);
    path.close();
    path.moveTo(6,0);
    path.lineTo(2,0);
    path.lineTo(4,2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testLine7a(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0,0);
    path.lineTo(4,0);
    path.lineTo(2,2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testLine7ax(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.moveTo(0,0);
    path.lineTo(4,0);
    path.lineTo(2,2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testLine7b(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0,0);
    path.lineTo(4,0);
    path.close();
    path.moveTo(6,0);
    path.lineTo(2,0);
    path.lineTo(4,2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testLine7bx(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.moveTo(0,0);
    path.lineTo(4,0);
    path.close();
    path.moveTo(6,0);
    path.lineTo(2,0);
    path.lineTo(4,2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testLine8(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0,4);
    path.lineTo(4,4);
    path.lineTo(2,2);
    path.close();
    path.moveTo(2,4);
    path.lineTo(6,4);
    path.lineTo(4,2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testLine8x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.moveTo(0,4);
    path.lineTo(4,4);
    path.lineTo(2,2);
    path.close();
    path.moveTo(2,4);
    path.lineTo(6,4);
    path.lineTo(4,2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testLine9(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0,4);
    path.lineTo(4,4);
    path.lineTo(2,2);
    path.close();
    path.moveTo(6,4);
    path.lineTo(2,4);
    path.lineTo(4,2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testLine9x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.moveTo(0,4);
    path.lineTo(4,4);
    path.lineTo(2,2);
    path.close();
    path.moveTo(6,4);
    path.lineTo(2,4);
    path.lineTo(4,2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testLine10(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0,4);
    path.lineTo(4,4);
    path.lineTo(2,2);
    path.close();
    path.moveTo(2,1);
    path.lineTo(3,4);
    path.lineTo(6,1);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testLine10x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.moveTo(0,4);
    path.lineTo(4,4);
    path.lineTo(2,2);
    path.close();
    path.moveTo(2,1);
    path.lineTo(3,4);
    path.lineTo(6,1);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testLine10a(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0,4);
    path.lineTo(8,4);
    path.lineTo(4,0);
    path.close();
    path.moveTo(2,2);
    path.lineTo(3,3);
    path.lineTo(4,2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testLine10ax(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.moveTo(0,4);
    path.lineTo(8,4);
    path.lineTo(4,0);
    path.close();
    path.moveTo(2,2);
    path.lineTo(3,3);
    path.lineTo(4,2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void addCWContainer(SkPath& path) {
    path.moveTo(6,4);
    path.lineTo(0,4);
    path.lineTo(3,1);
    path.close();
}

static void addCCWContainer(SkPath& path) {
    path.moveTo(0,4);
    path.lineTo(6,4);
    path.lineTo(3,1);
    path.close();
}

static void addCWContents(SkPath& path) {
    path.moveTo(2,3);
    path.lineTo(3,2);
    path.lineTo(4,3);
    path.close();
}

static void addCCWContents(SkPath& path) {
    path.moveTo(3,2);
    path.lineTo(2,3);
    path.lineTo(4,3);
    path.close();
}

static void testLine11(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    addCWContainer(path);
    addCWContents(path);
    testSimplify(reporter, path, filename);
}

static void testLine11x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    addCWContainer(path);
    addCWContents(path);
    testSimplify(reporter, path, filename);
}

static void testLine12(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    addCCWContainer(path);
    addCWContents(path);
    testSimplify(reporter, path, filename);
}

static void testLine12x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    addCCWContainer(path);
    addCWContents(path);
    testSimplify(reporter, path, filename);
}

static void testLine13(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    addCWContainer(path);
    addCCWContents(path);
    testSimplify(reporter, path, filename);
}

static void testLine13x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    addCWContainer(path);
    addCCWContents(path);
    testSimplify(reporter, path, filename);
}

static void testLine14(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    addCCWContainer(path);
    addCCWContents(path);
    testSimplify(reporter, path, filename);
}

static void testLine14x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    addCCWContainer(path);
    addCCWContents(path);
    testSimplify(reporter, path, filename);
}

static void testLine15(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 9, 9, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine15x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(0, 0, 9, 9, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine16(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 12, 12, SkPathDirection::kCW);
    path.addRect(0, 4, 9, 9, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine16x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(0, 0, 12, 12, SkPathDirection::kCW);
    path.addRect(0, 4, 9, 9, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine17(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 12, 12, SkPathDirection::kCW);
    path.addRect(4, 12, 13, 13, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine17x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(0, 0, 12, 12, SkPathDirection::kCW);
    path.addRect(4, 12, 13, 13, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine18(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 12, 12, SkPathDirection::kCW);
    path.addRect(12, 4, 21, 21, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine18x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(0, 0, 12, 12, SkPathDirection::kCW);
    path.addRect(12, 4, 21, 21, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine19(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 12, 12, SkPathDirection::kCW);
    path.addRect(12, 16, 21, 21, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine19x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(0, 0, 12, 12, SkPathDirection::kCW);
    path.addRect(12, 16, 21, 21, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine20(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 12, 12, 12, SkPathDirection::kCW);
    path.addRect(0, 12, 9, 9, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine20x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(0, 12, 12, 12, SkPathDirection::kCW);
    path.addRect(0, 12, 9, 9, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine21(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 12, 12, 12, SkPathDirection::kCW);
    path.addRect(0, 16, 9, 9, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine21x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(0, 12, 12, 12, SkPathDirection::kCW);
    path.addRect(0, 16, 9, 9, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine22(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 12, 12, 12, SkPathDirection::kCW);
    path.addRect(4, 12, 13, 13, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine22x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(0, 12, 12, 12, SkPathDirection::kCW);
    path.addRect(4, 12, 13, 13, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine23(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 12, 12, 12, SkPathDirection::kCW);
    path.addRect(12, 0, 21, 21, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine23x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(0, 12, 12, 12, SkPathDirection::kCW);
    path.addRect(12, 0, 21, 21, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine24a(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(2,0);
    path.lineTo(4,4);
    path.lineTo(0,4);
    path.close();
    path.moveTo(2,0);
    path.lineTo(1,2);
    path.lineTo(2,2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testLine24ax(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.moveTo(2,0);
    path.lineTo(4,4);
    path.lineTo(0,4);
    path.close();
    path.moveTo(2,0);
    path.lineTo(1,2);
    path.lineTo(2,2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testLine24(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 18, 12, 12, SkPathDirection::kCW);
    path.addRect(4, 12, 13, 13, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine24x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(0, 18, 12, 12, SkPathDirection::kCW);
    path.addRect(4, 12, 13, 13, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine25(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 6, 12, 12, SkPathDirection::kCW);
    path.addRect(12, 0, 21, 21, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine25x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(0, 6, 12, 12, SkPathDirection::kCW);
    path.addRect(12, 0, 21, 21, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine26(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 18, 12, 12, SkPathDirection::kCW);
    path.addRect(0, 12, 9, 9, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine26x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(0, 18, 12, 12, SkPathDirection::kCW);
    path.addRect(0, 12, 9, 9, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine27(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 18, 12, 12, SkPathDirection::kCW);
    path.addRect(12, 8, 21, 21, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine27x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(0, 18, 12, 12, SkPathDirection::kCW);
    path.addRect(12, 8, 21, 21, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine28(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 6, 12, 12, SkPathDirection::kCW);
    path.addRect(0, 0, 9, 9, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine28x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(0, 6, 12, 12, SkPathDirection::kCW);
    path.addRect(0, 0, 9, 9, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine29(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 18, 12, 12, SkPathDirection::kCW);
    path.addRect(12, 12, 21, 21, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine29x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(0, 18, 12, 12, SkPathDirection::kCW);
    path.addRect(12, 12, 21, 21, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine30(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 20, 20, SkPathDirection::kCW);
    path.addRect(0, 0, 12, 12, SkPathDirection::kCW);
    path.addRect(4, 4, 13, 13, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine30x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(0, 0, 20, 20, SkPathDirection::kCW);
    path.addRect(0, 0, 12, 12, SkPathDirection::kCW);
    path.addRect(4, 4, 13, 13, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine31(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 20, 20, SkPathDirection::kCW);
    path.addRect(0, 0, 12, 12, SkPathDirection::kCW);
    path.addRect(0, 4, 9, 9, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine31x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(0, 0, 20, 20, SkPathDirection::kCW);
    path.addRect(0, 0, 12, 12, SkPathDirection::kCW);
    path.addRect(0, 4, 9, 9, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine32(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 20, 20, SkPathDirection::kCW);
    path.addRect(0, 0, 12, 12, SkPathDirection::kCW);
    path.addRect(4, 12, 13, 13, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine32x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(0, 0, 20, 20, SkPathDirection::kCW);
    path.addRect(0, 0, 12, 12, SkPathDirection::kCW);
    path.addRect(4, 12, 13, 13, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine33(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 20, 20, SkPathDirection::kCW);
    path.addRect(0, 0, 12, 12, SkPathDirection::kCW);
    path.addRect(4, 16, 13, 13, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine33x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(0, 0, 20, 20, SkPathDirection::kCW);
    path.addRect(0, 0, 12, 12, SkPathDirection::kCW);
    path.addRect(4, 16, 13, 13, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine34(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 20, 20, SkPathDirection::kCW);
    path.addRect(0, 6, 12, 12, SkPathDirection::kCW);
    path.addRect(4, 12, 13, 13, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine34x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(0, 0, 20, 20, SkPathDirection::kCW);
    path.addRect(0, 6, 12, 12, SkPathDirection::kCW);
    path.addRect(4, 12, 13, 13, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine35(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 20, 20, SkPathDirection::kCW);
    path.addRect(6, 0, 18, 18, SkPathDirection::kCW);
    path.addRect(4, 16, 13, 13, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine35x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(0, 0, 20, 20, SkPathDirection::kCW);
    path.addRect(6, 0, 18, 18, SkPathDirection::kCW);
    path.addRect(4, 16, 13, 13, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine36(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 10, 20, 20, SkPathDirection::kCW);
    path.addRect(6, 12, 18, 18, SkPathDirection::kCW);
    path.addRect(4, 16, 13, 13, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine36x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(0, 10, 20, 20, SkPathDirection::kCW);
    path.addRect(6, 12, 18, 18, SkPathDirection::kCW);
    path.addRect(4, 16, 13, 13, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine37(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 20, 20, 20, SkPathDirection::kCW);
    path.addRect(18, 24, 30, 30, SkPathDirection::kCW);
    path.addRect(0, 0, 9, 9, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine37x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(0, 20, 20, 20, SkPathDirection::kCW);
    path.addRect(18, 24, 30, 30, SkPathDirection::kCW);
    path.addRect(0, 0, 9, 9, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine38(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(10, 0, 30, 30, SkPathDirection::kCW);
    path.addRect(6, 12, 18, 18, SkPathDirection::kCW);
    path.addRect(12, 12, 21, 21, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine38x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(10, 0, 30, 30, SkPathDirection::kCW);
    path.addRect(6, 12, 18, 18, SkPathDirection::kCW);
    path.addRect(12, 12, 21, 21, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine40(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(10, 0, 30, 30, SkPathDirection::kCW);
    path.addRect(12, 18, 24, 24, SkPathDirection::kCW);
    path.addRect(4, 16, 13, 13, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine40x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(10, 0, 30, 30, SkPathDirection::kCW);
    path.addRect(12, 18, 24, 24, SkPathDirection::kCW);
    path.addRect(4, 16, 13, 13, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine41(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 20, 20, SkPathDirection::kCW);
    path.addRect(18, 24, 30, 30, SkPathDirection::kCW);
    path.addRect(12, 0, 21, 21, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine41x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(0, 0, 20, 20, SkPathDirection::kCW);
    path.addRect(18, 24, 30, 30, SkPathDirection::kCW);
    path.addRect(12, 0, 21, 21, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine42(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 20, 20, SkPathDirection::kCW);
    path.addRect(0, 0, 12, 12, SkPathDirection::kCW);
    path.addRect(8, 16, 17, 17, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine42x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(0, 0, 20, 20, SkPathDirection::kCW);
    path.addRect(0, 0, 12, 12, SkPathDirection::kCW);
    path.addRect(8, 16, 17, 17, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine43(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 20, 20, SkPathDirection::kCW);
    path.addRect(6, 24, 18, 18, SkPathDirection::kCW);
    path.addRect(0, 32, 9, 36, SkPathDirection::kCCW);
    testSimplify(reporter, path, filename);
}

static void testLine43x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(0, 0, 20, 20, SkPathDirection::kCW);
    path.addRect(6, 24, 18, 18, SkPathDirection::kCW);
    path.addRect(0, 32, 9, 36, SkPathDirection::kCCW);
    testSimplify(reporter, path, filename);
}

static void testLine44(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(10, 40, 30, 30, SkPathDirection::kCW);
    path.addRect(18, 0, 30, 30, SkPathDirection::kCW);
    path.addRect(18, 32, 27, 36, SkPathDirection::kCCW);
    testSimplify(reporter, path, filename);
}

static void testLine44x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(10, 40, 30, 30, SkPathDirection::kCW);
    path.addRect(18, 0, 30, 30, SkPathDirection::kCW);
    path.addRect(18, 32, 27, 36, SkPathDirection::kCCW);
    testSimplify(reporter, path, filename);
}

static void testLine45(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(10, 0, 30, 30, SkPathDirection::kCW);
    path.addRect(18, 0, 30, 30, SkPathDirection::kCW);
    path.addRect(24, 32, 33, 36, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine45x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(10, 0, 30, 30, SkPathDirection::kCW);
    path.addRect(18, 0, 30, 30, SkPathDirection::kCW);
    path.addRect(24, 32, 33, 36, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine46(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(10, 40, 30, 30, SkPathDirection::kCW);
    path.addRect(24, 0, 36, 36, SkPathDirection::kCW);
    path.addRect(24, 32, 33, 36, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine46x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(10, 40, 30, 30, SkPathDirection::kCW);
    path.addRect(24, 0, 36, 36, SkPathDirection::kCW);
    path.addRect(24, 32, 33, 36, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine47(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 20, 20, SkPathDirection::kCW);
    path.addRect(0, 0, 12, 12, SkPathDirection::kCW);
    path.addRect(0, 0, 9, 9, SkPathDirection::kCCW);
    testSimplify(reporter, path, filename);
}

static void testLine47x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(0, 0, 20, 20, SkPathDirection::kCW);
    path.addRect(0, 0, 12, 12, SkPathDirection::kCW);
    path.addRect(0, 0, 9, 9, SkPathDirection::kCCW);
    testSimplify(reporter, path, filename);
}

static void testLine48(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 20, 20, SkPathDirection::kCW);
    path.addRect(0, 6, 12, 12, SkPathDirection::kCW);
    path.addRect(0, 0, 9, 9, SkPathDirection::kCCW);
    testSimplify(reporter, path, filename);
}

static void testLine48x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(0, 0, 20, 20, SkPathDirection::kCW);
    path.addRect(0, 6, 12, 12, SkPathDirection::kCW);
    path.addRect(0, 0, 9, 9, SkPathDirection::kCCW);
    testSimplify(reporter, path, filename);
}

static void testLine49(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 20, 20, SkPathDirection::kCW);
    path.addRect(0, 0, 12, 12, SkPathDirection::kCW);
    path.addRect(0, 0, 9, 9, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine49x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(0, 0, 20, 20, SkPathDirection::kCW);
    path.addRect(0, 0, 12, 12, SkPathDirection::kCW);
    path.addRect(0, 0, 9, 9, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine50(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(10, 30, 30, 30, SkPathDirection::kCW);
    path.addRect(24, 20, 36, 30, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine50x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(10, 30, 30, 30, SkPathDirection::kCW);
    path.addRect(24, 20, 36, 30, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine51(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 20, 20, SkPathDirection::kCW);
    path.addRect(0, 12, 12, 12, SkPathDirection::kCW);
    path.addRect(4, 12, 13, 13, SkPathDirection::kCCW);
    testSimplify(reporter, path, filename);
}

static void testLine51x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(0, 0, 20, 20, SkPathDirection::kCW);
    path.addRect(0, 12, 12, 12, SkPathDirection::kCW);
    path.addRect(4, 12, 13, 13, SkPathDirection::kCCW);
    testSimplify(reporter, path, filename);
}

static void testLine52(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 30, 20, 20, SkPathDirection::kCW);
    path.addRect(6, 20, 18, 30, SkPathDirection::kCW);
    path.addRect(32, 0, 36, 41, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine52x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(0, 30, 20, 20, SkPathDirection::kCW);
    path.addRect(6, 20, 18, 30, SkPathDirection::kCW);
    path.addRect(32, 0, 36, 41, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine53(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(10, 30, 30, 30, SkPathDirection::kCW);
    path.addRect(12, 20, 24, 30, SkPathDirection::kCW);
    path.addRect(12, 32, 21, 36, SkPathDirection::kCCW);
    testSimplify(reporter, path, filename);
}

static void testLine53x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(10, 30, 30, 30, SkPathDirection::kCW);
    path.addRect(12, 20, 24, 30, SkPathDirection::kCW);
    path.addRect(12, 32, 21, 36, SkPathDirection::kCCW);
    testSimplify(reporter, path, filename);
}

static void testLine54(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 20, 20, SkPathDirection::kCW);
    path.addRect(6, 0, 18, 18, SkPathDirection::kCW);
    path.addRect(8, 4, 17, 17, SkPathDirection::kCCW);
    testSimplify(reporter, path, filename);
}

static void testLine54x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(0, 0, 20, 20, SkPathDirection::kCW);
    path.addRect(6, 0, 18, 18, SkPathDirection::kCW);
    path.addRect(8, 4, 17, 17, SkPathDirection::kCCW);
    testSimplify(reporter, path, filename);
}

static void testLine55(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 20, 20, SkPathDirection::kCW);
    path.addRect(6, 6, 18, 18, SkPathDirection::kCW);
    path.addRect(4, 4, 13, 13, SkPathDirection::kCCW);
    testSimplify(reporter, path, filename);
}

static void testLine55x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(0, 0, 20, 20, SkPathDirection::kCW);
    path.addRect(6, 6, 18, 18, SkPathDirection::kCW);
    path.addRect(4, 4, 13, 13, SkPathDirection::kCCW);
    testSimplify(reporter, path, filename);
}

static void testLine56(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 20, 20, 20, SkPathDirection::kCW);
    path.addRect(18, 20, 30, 30, SkPathDirection::kCW);
    path.addRect(12, 0, 21, 21, SkPathDirection::kCCW);
    testSimplify(reporter, path, filename);
}

static void testLine56x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(0, 20, 20, 20, SkPathDirection::kCW);
    path.addRect(18, 20, 30, 30, SkPathDirection::kCW);
    path.addRect(12, 0, 21, 21, SkPathDirection::kCCW);
    testSimplify(reporter, path, filename);
}

static void testLine57(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(20, 0, 40, 40, SkPathDirection::kCW);
    path.addRect(20, 0, 30, 40, SkPathDirection::kCW);
    path.addRect(12, 0, 21, 21, SkPathDirection::kCCW);
    testSimplify(reporter, path, filename);
}

static void testLine57x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(20, 0, 40, 40, SkPathDirection::kCW);
    path.addRect(20, 0, 30, 40, SkPathDirection::kCW);
    path.addRect(12, 0, 21, 21, SkPathDirection::kCCW);
    testSimplify(reporter, path, filename);
}

static void testLine58(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 20, 20, SkPathDirection::kCW);
    path.addRect(0, 0, 12, 12, SkPathDirection::kCCW);
    path.addRect(0, 12, 9, 9, SkPathDirection::kCCW);
    testSimplify(reporter, path, filename);
}

static void testLine58x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(0, 0, 20, 20, SkPathDirection::kCW);
    path.addRect(0, 0, 12, 12, SkPathDirection::kCCW);
    path.addRect(0, 12, 9, 9, SkPathDirection::kCCW);
    testSimplify(reporter, path, filename);
}

static void testLine59(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 20, 20, SkPathDirection::kCW);
    path.addRect(6, 6, 18, 18, SkPathDirection::kCCW);
    path.addRect(4, 4, 13, 13, SkPathDirection::kCCW);
    testSimplify(reporter, path, filename);
}

static void testLine59x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(0, 0, 20, 20, SkPathDirection::kCW);
    path.addRect(6, 6, 18, 18, SkPathDirection::kCCW);
    path.addRect(4, 4, 13, 13, SkPathDirection::kCCW);
    testSimplify(reporter, path, filename);
}

static void testLine60(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 20, 20, SkPathDirection::kCW);
    path.addRect(6, 12, 18, 18, SkPathDirection::kCCW);
    path.addRect(4, 12, 13, 13, SkPathDirection::kCCW);
    testSimplify(reporter, path, filename);
}

static void testLine60x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(0, 0, 20, 20, SkPathDirection::kCW);
    path.addRect(6, 12, 18, 18, SkPathDirection::kCCW);
    path.addRect(4, 12, 13, 13, SkPathDirection::kCCW);
    testSimplify(reporter, path, filename);
}

static void testLine61(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 20, 20, SkPathDirection::kCW);
    path.addRect(12, 0, 24, 24, SkPathDirection::kCCW);
    path.addRect(12, 0, 21, 21, SkPathDirection::kCCW);
    testSimplify(reporter, path, filename);
}

static void testLine61x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(0, 0, 20, 20, SkPathDirection::kCW);
    path.addRect(12, 0, 24, 24, SkPathDirection::kCCW);
    path.addRect(12, 0, 21, 21, SkPathDirection::kCCW);
    testSimplify(reporter, path, filename);
}

static void testLine62(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 60, 60, SkPathDirection::kCW);
    path.addRect(0, 0, 20, 20, SkPathDirection::kCW);
    path.addRect(0, 12, 12, 12, SkPathDirection::kCW);
    path.addRect(4, 12, 13, 13, SkPathDirection::kCCW);
    testSimplify(reporter, path, filename);
}

static void testLine62x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(0, 0, 60, 60, SkPathDirection::kCW);
    path.addRect(0, 0, 20, 20, SkPathDirection::kCW);
    path.addRect(0, 12, 12, 12, SkPathDirection::kCW);
    path.addRect(4, 12, 13, 13, SkPathDirection::kCCW);
    testSimplify(reporter, path, filename);
}

static void testLine63(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 60, 60, SkPathDirection::kCW);
    path.addRect(0, 10, 20, 20, SkPathDirection::kCW);
    path.addRect(0, 6, 12, 12, SkPathDirection::kCCW);
    path.addRect(0, 32, 9, 36, SkPathDirection::kCCW);
    testSimplify(reporter, path, filename);
}

static void testLine63x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(0, 0, 60, 60, SkPathDirection::kCW);
    path.addRect(0, 10, 20, 20, SkPathDirection::kCW);
    path.addRect(0, 6, 12, 12, SkPathDirection::kCCW);
    path.addRect(0, 32, 9, 36, SkPathDirection::kCCW);
    testSimplify(reporter, path, filename);
}

static void testLine64(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 60, 60, SkPathDirection::kCW);
    path.addRect(10, 40, 30, 30, SkPathDirection::kCW);
    path.addRect(18, 6, 30, 30, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine64x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(0, 0, 60, 60, SkPathDirection::kCW);
    path.addRect(10, 40, 30, 30, SkPathDirection::kCW);
    path.addRect(18, 6, 30, 30, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine65(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 60, 60, SkPathDirection::kCW);
    path.addRect(10, 0, 30, 30, SkPathDirection::kCW);
    path.addRect(24, 0, 36, 36, SkPathDirection::kCW);
    path.addRect(32, 6, 36, 41, SkPathDirection::kCCW);
    testSimplify(reporter, path, filename);
}

static void testLine65x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(0, 0, 60, 60, SkPathDirection::kCW);
    path.addRect(10, 0, 30, 30, SkPathDirection::kCW);
    path.addRect(24, 0, 36, 36, SkPathDirection::kCW);
    path.addRect(32, 6, 36, 41, SkPathDirection::kCCW);
    testSimplify(reporter, path, filename);
}

static void testLine66(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 60, 60, SkPathDirection::kCW);
    path.addRect(0, 30, 20, 20, SkPathDirection::kCW);
    path.addRect(12, 20, 24, 30, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine66x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(0, 0, 60, 60, SkPathDirection::kCW);
    path.addRect(0, 30, 20, 20, SkPathDirection::kCW);
    path.addRect(12, 20, 24, 30, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine67(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 60, 60, SkPathDirection::kCW);
    path.addRect(10, 40, 30, 30, SkPathDirection::kCW);
    path.addRect(24, 20, 36, 30, SkPathDirection::kCW);
    path.addRect(32, 0, 36, 41, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine67x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(0, 0, 60, 60, SkPathDirection::kCW);
    path.addRect(10, 40, 30, 30, SkPathDirection::kCW);
    path.addRect(24, 20, 36, 30, SkPathDirection::kCW);
    path.addRect(32, 0, 36, 41, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine68a(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 8, 8, SkPathDirection::kCW);
    path.addRect(2, 2, 6, 6, SkPathDirection::kCW);
    path.addRect(1, 2, 4, 2, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine68ax(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(0, 0, 8, 8, SkPathDirection::kCW);
    path.addRect(2, 2, 6, 6, SkPathDirection::kCW);
    path.addRect(1, 2, 4, 2, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine68b(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 8, 8, SkPathDirection::kCW);
    path.addRect(2, 2, 6, 6, SkPathDirection::kCCW);
    path.addRect(1, 2, 2, 2, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine68bx(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(0, 0, 8, 8, SkPathDirection::kCW);
    path.addRect(2, 2, 6, 6, SkPathDirection::kCCW);
    path.addRect(1, 2, 2, 2, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine68c(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 8, 8, SkPathDirection::kCCW);
    path.addRect(2, 2, 6, 6, SkPathDirection::kCW);
    path.addRect(1, 2, 4, 2, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine68cx(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(0, 0, 8, 8, SkPathDirection::kCCW);
    path.addRect(2, 2, 6, 6, SkPathDirection::kCW);
    path.addRect(1, 2, 4, 2, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine68d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 8, 8, SkPathDirection::kCCW);
    path.addRect(2, 2, 6, 6, SkPathDirection::kCCW);
    path.addRect(1, 2, 4, 2, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine68dx(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(0, 0, 8, 8, SkPathDirection::kCCW);
    path.addRect(2, 2, 6, 6, SkPathDirection::kCCW);
    path.addRect(1, 2, 4, 2, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine68e(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 8, 8, SkPathDirection::kCW);
    path.addRect(0, 0, 8, 8, SkPathDirection::kCW);
    path.addRect(2, 2, 6, 6, SkPathDirection::kCCW);
    path.addRect(1, 2, 2, 2, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine68ex(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(0, 0, 8, 8, SkPathDirection::kCW);
    path.addRect(0, 0, 8, 8, SkPathDirection::kCW);
    path.addRect(2, 2, 6, 6, SkPathDirection::kCCW);
    path.addRect(1, 2, 2, 2, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine68f(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 8, 8, SkPathDirection::kCW);
    path.addRect(2, 2, 6, 6, SkPathDirection::kCCW);
    path.addRect(2, 2, 6, 6, SkPathDirection::kCCW);
    path.addRect(1, 2, 2, 2, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine68fx(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(0, 0, 8, 8, SkPathDirection::kCW);
    path.addRect(2, 2, 6, 6, SkPathDirection::kCCW);
    path.addRect(2, 2, 6, 6, SkPathDirection::kCCW);
    path.addRect(1, 2, 2, 2, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine68g(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 8, 8, SkPathDirection::kCW);
    path.addRect(0, 0, 8, 8, SkPathDirection::kCW);
    path.addRect(2, 2, 6, 6, SkPathDirection::kCCW);
    path.addRect(2, 2, 6, 6, SkPathDirection::kCCW);
    path.addRect(1, 2, 2, 2, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine68gx(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(0, 0, 8, 8, SkPathDirection::kCW);
    path.addRect(0, 0, 8, 8, SkPathDirection::kCW);
    path.addRect(2, 2, 6, 6, SkPathDirection::kCCW);
    path.addRect(2, 2, 6, 6, SkPathDirection::kCCW);
    path.addRect(1, 2, 2, 2, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine68h(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 8, 8, SkPathDirection::kCW);
    path.addRect(2, 2, 6, 6, SkPathDirection::kCCW);
    path.addRect(2, 2, 6, 6, SkPathDirection::kCCW);
    path.addRect(2, 2, 6, 6, SkPathDirection::kCCW);
    path.addRect(1, 2, 2, 2, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine68hx(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(0, 0, 8, 8, SkPathDirection::kCW);
    path.addRect(2, 2, 6, 6, SkPathDirection::kCCW);
    path.addRect(2, 2, 6, 6, SkPathDirection::kCCW);
    path.addRect(2, 2, 6, 6, SkPathDirection::kCCW);
    path.addRect(1, 2, 2, 2, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine69(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 20, 20, 20, SkPathDirection::kCW);
    path.addRect(0, 20, 12, 30, SkPathDirection::kCW);
    path.addRect(12, 32, 21, 36, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine69x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(0, 20, 20, 20, SkPathDirection::kCW);
    path.addRect(0, 20, 12, 30, SkPathDirection::kCW);
    path.addRect(12, 32, 21, 36, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine70(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 20, 20, SkPathDirection::kCW);
    path.addRect(0, 24, 12, 12, SkPathDirection::kCW);
    path.addRect(12, 32, 21, 36, SkPathDirection::kCCW);
    testSimplify(reporter, path, filename);
}

static void testLine70x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(0, 0, 20, 20, SkPathDirection::kCW);
    path.addRect(0, 24, 12, 12, SkPathDirection::kCW);
    path.addRect(12, 32, 21, 36, SkPathDirection::kCCW);
    testSimplify(reporter, path, filename);
}

static void testLine71(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 20, 20, SkPathDirection::kCW);
    path.addRect(12, 0, 24, 24, SkPathDirection::kCW);
    path.addRect(12, 32, 21, 36, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine71x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(0, 0, 20, 20, SkPathDirection::kCW);
    path.addRect(12, 0, 24, 24, SkPathDirection::kCW);
    path.addRect(12, 32, 21, 36, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine72(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 60, 60, SkPathDirection::kCW);
    path.addRect(10, 40, 30, 30, SkPathDirection::kCW);
    path.addRect(6, 20, 18, 30, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine72x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(0, 0, 60, 60, SkPathDirection::kCW);
    path.addRect(10, 40, 30, 30, SkPathDirection::kCW);
    path.addRect(6, 20, 18, 30, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine73(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 60, 60, SkPathDirection::kCW);
    path.addRect(0, 40, 20, 20, SkPathDirection::kCW);
    path.addRect(0, 20, 12, 30, SkPathDirection::kCW);
    path.addRect(0, 0, 9, 9, SkPathDirection::kCCW);
    testSimplify(reporter, path, filename);
}

static void testLine73x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(0, 0, 60, 60, SkPathDirection::kCW);
    path.addRect(0, 40, 20, 20, SkPathDirection::kCW);
    path.addRect(0, 20, 12, 30, SkPathDirection::kCW);
    path.addRect(0, 0, 9, 9, SkPathDirection::kCCW);
    testSimplify(reporter, path, filename);
}

static void testLine74(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(20, 30, 40, 40, SkPathDirection::kCW);
    path.addRect(24, 20, 36, 30, SkPathDirection::kCCW);
    path.addRect(32, 24, 36, 41, SkPathDirection::kCCW);
    testSimplify(reporter, path, filename);
}

static void testLine74x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(20, 30, 40, 40, SkPathDirection::kCW);
    path.addRect(24, 20, 36, 30, SkPathDirection::kCCW);
    path.addRect(32, 24, 36, 41, SkPathDirection::kCCW);
    testSimplify(reporter, path, filename);
}

static void testLine75(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 60, 60, SkPathDirection::kCW);
    path.addRect(10, 0, 30, 30, SkPathDirection::kCCW);
    path.addRect(18, 0, 30, 30, SkPathDirection::kCCW);
    path.addRect(12, 0, 21, 21, SkPathDirection::kCCW);
    testSimplify(reporter, path, filename);
}

static void testLine75x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(0, 0, 60, 60, SkPathDirection::kCW);
    path.addRect(10, 0, 30, 30, SkPathDirection::kCCW);
    path.addRect(18, 0, 30, 30, SkPathDirection::kCCW);
    path.addRect(12, 0, 21, 21, SkPathDirection::kCCW);
    testSimplify(reporter, path, filename);
}

static void testLine76(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(36, 0, 66, 60, SkPathDirection::kCW);
    path.addRect(10, 20, 40, 30, SkPathDirection::kCW);
    path.addRect(24, 20, 36, 30, SkPathDirection::kCCW);
    path.addRect(32, 6, 36, 41, SkPathDirection::kCCW);
    testSimplify(reporter, path, filename);
}

static void testLine76x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(36, 0, 66, 60, SkPathDirection::kCW);
    path.addRect(10, 20, 40, 30, SkPathDirection::kCW);
    path.addRect(24, 20, 36, 30, SkPathDirection::kCCW);
    path.addRect(32, 6, 36, 41, SkPathDirection::kCCW);
    testSimplify(reporter, path, filename);
}

static void testLine77(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(20, 0, 40, 40, SkPathDirection::kCW);
    path.addRect(24, 6, 36, 36, SkPathDirection::kCCW);
    path.addRect(24, 32, 33, 36, SkPathDirection::kCCW);
    testSimplify(reporter, path, filename);
}

static void testLine77x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(20, 0, 40, 40, SkPathDirection::kCW);
    path.addRect(24, 6, 36, 36, SkPathDirection::kCCW);
    path.addRect(24, 32, 33, 36, SkPathDirection::kCCW);
    testSimplify(reporter, path, filename);
}

static void testLine78(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 30, 60, SkPathDirection::kCW);
    path.addRect(10, 20, 30, 30, SkPathDirection::kCCW);
    path.addRect(18, 20, 30, 30, SkPathDirection::kCCW);
    path.addRect(32, 0, 36, 41, SkPathDirection::kCCW);
    testSimplify(reporter, path, filename);
}

static void testLine78x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(0, 0, 30, 60, SkPathDirection::kCW);
    path.addRect(10, 20, 30, 30, SkPathDirection::kCCW);
    path.addRect(18, 20, 30, 30, SkPathDirection::kCCW);
    path.addRect(32, 0, 36, 41, SkPathDirection::kCCW);
    testSimplify(reporter, path, filename);
}

static void testLine79(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 36, 60, 30, SkPathDirection::kCW);
    path.addRect(10, 30, 40, 30, SkPathDirection::kCW);
    path.addRect(0, 20, 12, 30, SkPathDirection::kCCW);
    path.addRect(0, 32, 9, 36, SkPathDirection::kCCW);
    testSimplify(reporter, path, filename);
}

static void testLine79x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(0, 36, 60, 30, SkPathDirection::kCW);
    path.addRect(10, 30, 40, 30, SkPathDirection::kCW);
    path.addRect(0, 20, 12, 30, SkPathDirection::kCCW);
    path.addRect(0, 32, 9, 36, SkPathDirection::kCCW);
    testSimplify(reporter, path, filename);
}

static void testLine81(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(-1, -1, 3, 3, SkPathDirection::kCW);
    path.addRect(0, 0, 1, 1, SkPathDirection::kCW);
    path.addRect(0, 0, 1, 1, SkPathDirection::kCW);
    path.addRect(0, 0, 1, 1, SkPathDirection::kCW);
    path.addRect(1, 1, 2, 2, SkPathDirection::kCCW);
    testSimplify(reporter, path, filename);
}

static void testDegenerate1(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.lineTo(2, 0);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(2, 0);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testDegenerate1x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.lineTo(2, 0);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(2, 0);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testDegenerate2(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.lineTo(0, 0);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(0, 1);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testDegenerate2x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.lineTo(0, 0);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(0, 1);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testDegenerate3(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.lineTo(2, 0);
    path.lineTo(1, 0);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.lineTo(3, 0);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testDegenerate3x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.moveTo(0, 0);
    path.lineTo(2, 0);
    path.lineTo(1, 0);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.lineTo(3, 0);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testDegenerate4(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(1, 3);
    path.close();
    path.moveTo(1, 0);
    path.lineTo(1, 1);
    path.lineTo(1, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testDegenerate4x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(1, 3);
    path.close();
    path.moveTo(1, 0);
    path.lineTo(1, 1);
    path.lineTo(1, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testNondegenerate1(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.lineTo(3, 0);
    path.lineTo(1, 3);
    path.close();
    path.moveTo(1, 1);
    path.lineTo(2, 1);
    path.lineTo(1, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testNondegenerate1x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.moveTo(0, 0);
    path.lineTo(3, 0);
    path.lineTo(1, 3);
    path.close();
    path.moveTo(1, 1);
    path.lineTo(2, 1);
    path.lineTo(1, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testNondegenerate2(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(1, 0);
    path.lineTo(0, 1);
    path.lineTo(1, 1);
    path.close();
    path.moveTo(0, 2);
    path.lineTo(0, 3);
    path.lineTo(1, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testNondegenerate2x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.moveTo(1, 0);
    path.lineTo(0, 1);
    path.lineTo(1, 1);
    path.close();
    path.moveTo(0, 2);
    path.lineTo(0, 3);
    path.lineTo(1, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testNondegenerate3(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(2, 1);
    path.close();
    path.moveTo(0, 1);
    path.lineTo(1, 1);
    path.lineTo(0, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testNondegenerate3x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(2, 1);
    path.close();
    path.moveTo(0, 1);
    path.lineTo(1, 1);
    path.lineTo(0, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testNondegenerate4(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(1, 0);
    path.lineTo(0, 1);
    path.lineTo(1, 2);
    path.close();
    path.moveTo(0, 2);
    path.lineTo(0, 3);
    path.lineTo(1, 3);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testNondegenerate4x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.moveTo(1, 0);
    path.lineTo(0, 1);
    path.lineTo(1, 2);
    path.close();
    path.moveTo(0, 2);
    path.lineTo(0, 3);
    path.lineTo(1, 3);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadralateral5(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(1, 1);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(2, 2);
    path.lineTo(3, 2);
    path.lineTo(3, 3);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadralateral5x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(1, 1);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(2, 2);
    path.lineTo(3, 2);
    path.lineTo(3, 3);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadralateral6(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(1, 1);
    path.close();
    path.moveTo(1, 0);
    path.lineTo(2, 0);
    path.lineTo(0, 2);
    path.lineTo(2, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadralateral6x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(1, 1);
    path.close();
    path.moveTo(1, 0);
    path.lineTo(2, 0);
    path.lineTo(0, 2);
    path.lineTo(2, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testFauxQuadralateral6(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(1, 1);
    path.close();
    path.moveTo(1, 0);
    path.lineTo(2, 0);
    path.lineTo(1 + 1.0f/3, 2.0f/3);
    path.close();
    path.moveTo(1 + 1.0f/3, 2.0f/3);
    path.lineTo(0, 2);
    path.lineTo(2, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testFauxQuadralateral6x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(1, 1);
    path.close();
    path.moveTo(1, 0);
    path.lineTo(2, 0);
    path.lineTo(1 + 1.0f/3, 2.0f/3);
    path.close();
    path.moveTo(1 + 1.0f/3, 2.0f/3);
    path.lineTo(0, 2);
    path.lineTo(2, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testFauxQuadralateral6a(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.lineTo(3, 0);
    path.lineTo(3, 3);
    path.close();
    path.moveTo(3, 0);
    path.lineTo(6, 0);
    path.lineTo(4, 2);
    path.close();
    path.moveTo(4, 2);
    path.lineTo(0, 6);
    path.lineTo(6, 6);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testFauxQuadralateral6ax(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.moveTo(0, 0);
    path.lineTo(3, 0);
    path.lineTo(3, 3);
    path.close();
    path.moveTo(3, 0);
    path.lineTo(6, 0);
    path.lineTo(4, 2);
    path.close();
    path.moveTo(4, 2);
    path.lineTo(0, 6);
    path.lineTo(6, 6);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testFauxQuadralateral6b(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.lineTo(3, 0);
    path.lineTo(3, 3);
    path.close();
    path.moveTo(3, 0);
    path.lineTo(6, 0);
    path.lineTo(4, 2);
    path.close();
    path.moveTo(4, 2);
    path.lineTo(6, 6);
    path.lineTo(0, 6);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testFauxQuadralateral6bx(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.moveTo(0, 0);
    path.lineTo(3, 0);
    path.lineTo(3, 3);
    path.close();
    path.moveTo(3, 0);
    path.lineTo(6, 0);
    path.lineTo(4, 2);
    path.close();
    path.moveTo(4, 2);
    path.lineTo(6, 6);
    path.lineTo(0, 6);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testFauxQuadralateral6c(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.lineTo(3, 3);
    path.lineTo(3, 0);
    path.close();
    path.moveTo(3, 0);
    path.lineTo(6, 0);
    path.lineTo(4, 2);
    path.close();
    path.moveTo(4, 2);
    path.lineTo(0, 6);
    path.lineTo(6, 6);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testFauxQuadralateral6cx(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.moveTo(0, 0);
    path.lineTo(3, 3);
    path.lineTo(3, 0);
    path.close();
    path.moveTo(3, 0);
    path.lineTo(6, 0);
    path.lineTo(4, 2);
    path.close();
    path.moveTo(4, 2);
    path.lineTo(0, 6);
    path.lineTo(6, 6);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testFauxQuadralateral6d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.lineTo(3, 3);
    path.lineTo(3, 0);
    path.close();
    path.moveTo(3, 0);
    path.lineTo(6, 0);
    path.lineTo(4, 2);
    path.close();
    path.moveTo(4, 2);
    path.lineTo(6, 6);
    path.lineTo(0, 6);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testFauxQuadralateral6dx(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.moveTo(0, 0);
    path.lineTo(3, 3);
    path.lineTo(3, 0);
    path.close();
    path.moveTo(3, 0);
    path.lineTo(6, 0);
    path.lineTo(4, 2);
    path.close();
    path.moveTo(4, 2);
    path.lineTo(6, 6);
    path.lineTo(0, 6);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadralateral6a(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.lineTo(3, 0);
    path.lineTo(3, 3);
    path.close();
    path.moveTo(3, 0);
    path.lineTo(6, 0);
    path.lineTo(0, 6);
    path.lineTo(6, 6);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadralateral6ax(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.lineTo(3, 0);
    path.lineTo(3, 3);
    path.close();
    path.moveTo(3, 0);
    path.lineTo(6, 0);
    path.lineTo(0, 6);
    path.lineTo(6, 6);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadralateral7(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(2, 1);
    path.close();
    path.moveTo(1, 0);
    path.lineTo(1, 1);
    path.lineTo(2, 2);
    path.lineTo(1, 3);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadralateral7x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(2, 1);
    path.close();
    path.moveTo(1, 0);
    path.lineTo(1, 1);
    path.lineTo(2, 2);
    path.lineTo(1, 3);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadralateral8(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.lineTo(3, 1);
    path.lineTo(1, 3);
    path.lineTo(3, 3);
    path.close();
    path.moveTo(2, 1);
    path.lineTo(0, 2);
    path.lineTo(3, 2);
    path.lineTo(2, 3);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadralateral8x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.moveTo(0, 0);
    path.lineTo(3, 1);
    path.lineTo(1, 3);
    path.lineTo(3, 3);
    path.close();
    path.moveTo(2, 1);
    path.lineTo(0, 2);
    path.lineTo(3, 2);
    path.lineTo(2, 3);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadralateral9(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(1, 2);
    path.lineTo(2, 2);
    path.close();
    path.moveTo(1, 1);
    path.lineTo(2, 1);
    path.lineTo(1, 3);
    path.lineTo(2, 3);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadralateral9x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(1, 2);
    path.lineTo(2, 2);
    path.close();
    path.moveTo(1, 1);
    path.lineTo(2, 1);
    path.lineTo(1, 3);
    path.lineTo(2, 3);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testLine1a(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kWinding);
    path.addRect(0, 0, 12, 12, SkPathDirection::kCW);
    path.addRect(4, 0, 13, 13, SkPathDirection::kCCW);
    testSimplify(reporter, path, filename);
}

static void testLine1ax(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(0, 0, 12, 12, SkPathDirection::kCW);
    path.addRect(4, 0, 13, 13, SkPathDirection::kCCW);
    testSimplify(reporter, path, filename);
}

static void testLine2ax(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(0, 20, 20, 20, SkPathDirection::kCW);
    path.addRect(0, 20, 12, 30, SkPathDirection::kCW);
    path.addRect(12, 0, 21, 21, SkPathDirection::kCCW);
    testSimplify(reporter, path, filename);
}

static void testLine3aax(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(10, 30, 30, 30, SkPathDirection::kCW);
    path.addRect(18, 20, 30, 30, SkPathDirection::kCCW);
    path.addRect(0, 32, 9, 36, SkPathDirection::kCCW);
    testSimplify(reporter, path, filename);
}

static void testLine4ax(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(10, 30, 30, 30, SkPathDirection::kCW);
    path.addRect(24, 20, 36, 30, SkPathDirection::kCCW);
    path.addRect(0, 32, 9, 36, SkPathDirection::kCCW);
    testSimplify(reporter, path, filename);
}

static void testQuadratic1(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(0, 0, 0, 0);
    path.lineTo(1, 0);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(0, 0, 0, 0);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic1x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.moveTo(0, 0);
    path.quadTo(0, 0, 0, 0);
    path.lineTo(1, 0);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(0, 0, 0, 0);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic2(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(0, 0, 0, 0);
    path.lineTo(3, 0);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(1, 0, 0, 1);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic2x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.moveTo(0, 0);
    path.quadTo(0, 0, 0, 0);
    path.lineTo(3, 0);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(1, 0, 0, 1);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic3(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(0, 0, 1, 0);
    path.lineTo(0, 2);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(1, 0, 0, 1);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic3x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.moveTo(0, 0);
    path.quadTo(0, 0, 1, 0);
    path.lineTo(0, 2);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(1, 0, 0, 1);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic4(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(0, 0, 1, 0);
    path.lineTo(0, 2);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(1, 0, 0, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic4x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.moveTo(0, 0);
    path.quadTo(0, 0, 1, 0);
    path.lineTo(0, 2);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(1, 0, 0, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic5(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(0, 0, 0, 0);
    path.lineTo(0, 1);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.quadTo(0, 1, 0, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic6(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(0, 0, 1, 0);
    path.lineTo(2, 1);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(2, 0, 0, 1);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic7(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(0, 0, 1, 0);
    path.lineTo(3, 1);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(3, 0, 1, 2);
    path.close();
    markTestFlakyForPathKit();
    testSimplify(reporter, path, filename);
}

static void testQuadratic8(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(0, 0, 1, 0);
    path.lineTo(0, 2);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.quadTo(0, 1, 1, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic9(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(0, 0, 1, 0);
    path.lineTo(3, 1);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.quadTo(1, 2, 3, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic14(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(0, 0, 1, 0);
    path.lineTo(3, 2);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.quadTo(3, 2, 3, 3);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic15(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(0, 0, 1, 0);
    path.lineTo(1, 3);
    path.close();
    path.moveTo(1, 0);
    path.lineTo(0, 1);
    path.quadTo(1, 1, 0, 3);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic17x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.moveTo(0, 0);
    path.quadTo(0, 0, 3, 1);
    path.lineTo(0, 2);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.quadTo(3, 1, 0, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic18(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(1, 0, 0, 1);
    path.lineTo(0, 1);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(1, 0, 1, 1);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic19(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(1, 0, 0, 1);
    path.lineTo(0, 1);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(2, 0, 0, 1);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic20(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(1, 0, 0, 1);
    path.lineTo(0, 1);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(1, 0, 0, 1);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic21(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(1, 0, 0, 1);
    path.lineTo(0, 1);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(1, 0, 0, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic22(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(1, 0, 0, 1);
    path.lineTo(0, 1);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(0, 1, 2, 1);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic23(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(1, 0, 0, 1);
    path.lineTo(0, 1);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(0, 2, 1, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic24(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(1, 0, 0, 1);
    path.lineTo(0, 1);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.quadTo(2, 0, 0, 1);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic25(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(1, 0, 1, 1);
    path.lineTo(1, 1);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(2, 1, 0, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic26(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(1, 0, 1, 1);
    path.lineTo(0, 2);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(1, 0, 0, 1);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic27(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(1, 0, 1, 1);
    path.lineTo(2, 1);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(2, 1, 0, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic28(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(1, 0, 0, 1);
    path.lineTo(0, 1);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 2);
    path.quadTo(1, 2, 0, 3);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic29(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(1, 0, 2, 1);
    path.lineTo(0, 2);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(1, 0, 0, 1);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic30(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(1, 0, 1, 2);
    path.lineTo(1, 2);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.quadTo(0, 1, 1, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic31(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(1, 0, 1, 2);
    path.lineTo(1, 2);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.quadTo(0, 1, 1, 3);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic32(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(1, 0, 2, 3);
    path.lineTo(2, 3);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(3, 1, 0, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic33(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(2, 0, 0, 1);
    path.lineTo(0, 1);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(1, 1);
    path.quadTo(2, 1, 2, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic34(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(2, 0, 0, 1);
    path.lineTo(0, 1);
    path.close();
    path.moveTo(1, 0);
    path.lineTo(1, 1);
    path.quadTo(2, 1, 1, 2);
    path.close();
    markTestFlakyForPathKit();
    testSimplify(reporter, path, filename);
}

static void testQuadratic35(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(0, 1, 1, 1);
    path.lineTo(1, 3);
    path.close();
    path.moveTo(2, 0);
    path.lineTo(3, 0);
    path.quadTo(0, 1, 1, 1);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic36(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(2, 1, 2, 3);
    path.lineTo(2, 3);
    path.close();
    path.moveTo(3, 1);
    path.lineTo(1, 2);
    path.quadTo(3, 2, 1, 3);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic37(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(0, 2, 1, 2);
    path.lineTo(1, 2);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(3, 1);
    path.quadTo(0, 2, 1, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic38(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(1, 0);
    path.quadTo(0, 1, 1, 1);
    path.lineTo(1, 1);
    path.close();
    path.moveTo(1, 0);
    path.lineTo(1, 2);
    path.quadTo(2, 2, 1, 3);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic51(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(369.863983f, 145.645813f);
    path.quadTo(382.380371f, 121.254936f, 406.236359f, 121.254936f);
    path.lineTo(369.863983f, 145.645813f);
    path.close();
    path.moveTo(369.970581f, 137.94342f);
    path.quadTo(383.98465f, 121.254936f, 406.235992f, 121.254936f);
    path.lineTo(369.970581f, 137.94342f);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic53(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(303.12088f, 141.299606f);
    path.lineTo(330.463562f, 217.659027f);
    path.lineTo(303.12088f, 141.299606f);
    path.close();
    path.moveTo(371.919067f, 205.854996f);
    path.lineTo(326.236786f, 205.854996f);
    path.quadTo(329.104431f, 231.663818f, 351.512085f, 231.663818f);
    path.lineTo(371.919067f, 205.854996f);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic55(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
path.moveTo(303.12088f, 141.299606f);
path.lineTo(330.463562f, 217.659027f);
path.lineTo(358.606506f, 141.299606f);
path.lineTo(303.12088f, 141.299606f);
path.close();
path.moveTo(326.236786f, 205.854996f);
path.quadTo(329.104431f, 231.663818f, 351.512085f, 231.663818f);
path.lineTo(326.236786f, 205.854996f);
path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic56(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
path.moveTo(366.608826f, 151.196014f);
path.quadTo(378.803101f, 136.674606f, 398.164948f, 136.674606f);
path.lineTo(354.009216f, 208.816208f);
path.lineTo(393.291473f, 102.232819f);
path.lineTo(359.978058f, 136.581512f);
path.quadTo(378.315979f, 136.581512f, 388.322723f, 149.613556f);
path.lineTo(364.390686f, 157.898193f);
path.quadTo(375.281769f, 136.674606f, 396.039917f, 136.674606f);
path.lineTo(350, 120);
path.lineTo(366.608826f, 151.196014f);
path.close();
    testSimplify(reporter, path, filename);
}

static void testLine80(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
path.moveTo(4, 0);
path.lineTo(3, 7);
path.lineTo(7, 5);
path.lineTo(2, 2);
path.close();
path.moveTo(0, 6);
path.lineTo(6, 12);
path.lineTo(8, 3);
path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic58(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(283.714233f, 240);
    path.lineTo(283.714233f, 141.299606f);
    path.lineTo(303.12088f, 141.299606f);
    path.lineTo(330.463562f, 217.659027f);
    path.lineTo(358.606506f, 141.299606f);
    path.lineTo(362.874634f, 159.705902f);
    path.lineTo(335.665344f, 233.397751f);
    path.lineTo(322.12738f, 233.397751f);
    path.lineTo(295.718353f, 159.505829f);
    path.lineTo(295.718353f, 240);
    path.lineTo(283.714233f, 240);
    path.close();
    path.moveTo(322.935669f, 231.030273f);
    path.quadTo(312.832214f, 220.393295f, 312.832214f, 203.454178f);
    path.quadTo(312.832214f, 186.981888f, 321.73526f, 176.444946f);
    path.quadTo(330.638306f, 165.90802f, 344.509705f, 165.90802f);
    path.quadTo(357.647522f, 165.90802f, 364.81665f, 175.244537f);
    path.lineTo(371.919067f, 205.854996f);
    path.lineTo(326.236786f, 205.854996f);
    path.quadTo(329.104431f, 231.663818f, 351.512085f, 231.663818f);
    path.lineTo(322.935669f, 231.030273f);
    path.close();
    path.moveTo(326.837006f, 195.984955f);
    path.lineTo(358.78125f, 195.984955f);
    path.quadTo(358.78125f, 175.778046f, 343.709442f, 175.778046f);
    path.quadTo(328.570923f, 175.778046f, 326.837006f, 195.984955f);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic59x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.moveTo(0, 0);
    path.quadTo(0, 0, 0, 0);
    path.lineTo(2, 2);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(2, 0);
    path.quadTo(3, 1, 1, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic59(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kWinding);
    path.moveTo(0, 0);
    path.quadTo(0, 0, 0, 0);
    path.lineTo(2, 2);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(2, 0);
    path.quadTo(3, 1, 1, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic63(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(0, 0, 0, 0);
    path.lineTo(3, 2);
    path.close();
    path.moveTo(1, 0);
    path.lineTo(2, 1);
    path.quadTo(2, 1, 2, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic64(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(0, 0, 0, 0);
    path.lineTo(2, 3);
    path.close();
    path.moveTo(1, 2);
    path.lineTo(2, 2);
    path.quadTo(0, 3, 3, 3);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic65(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(0, 0, 0, 0);
    path.lineTo(3, 2);
    path.close();
    path.moveTo(2, 1);
    path.lineTo(2, 2);
    path.quadTo(0, 3, 1, 3);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic67x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.moveTo(0, 0);
    path.quadTo(0, 0, 2, 1);
    path.lineTo(2, 2);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(2, 0);
    path.quadTo(1, 1, 3, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic68(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(1, 0, 0, 1);
    path.lineTo(1, 2);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(0, 1, 2, 1);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic69(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(0, 0, 0, 1);
    path.lineTo(3, 2);
    path.close();
    path.moveTo(2, 0);
    path.lineTo(1, 1);
    path.quadTo(3, 2, 2, 3);
    path.close();
    markTestFlakyForPathKit();
    testSimplify(reporter, path, filename);
}

static void testQuadratic70x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.moveTo(0, 0);
    path.quadTo(1, 0, 0, 1);
    path.lineTo(1, 2);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(0, 1, 2, 1);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic71(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(1, 0, 1, 1);
    path.lineTo(3, 2);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(1, 1, 3, 1);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic72(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(1, 0, 1, 2);
    path.lineTo(1, 2);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.quadTo(0, 1, 3, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic73(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(1, 0, 0, 3);
    path.lineTo(0, 3);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.quadTo(0, 1, 1, 1);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic74(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(1, 0, 1, 3);
    path.lineTo(1, 3);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 1);
    path.quadTo(3, 2, 2, 3);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic75(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(1, 0, 1, 3);
    path.lineTo(2, 3);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 1);
    path.quadTo(3, 2, 2, 3);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic76(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(0, 0, 0, 0);
    path.lineTo(2, 3);
    path.close();
    path.moveTo(1, 0);
    path.lineTo(1, 2);
    path.quadTo(1, 2, 2, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic77(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(1, 0, 1, 1);
    path.lineTo(3, 1);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.quadTo(0, 1, 3, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic78(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(1, 0, 1, 2);
    path.lineTo(3, 2);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(2, 1, 0, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic79(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(1, 0, 1, 2);
    path.lineTo(3, 2);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.quadTo(0, 1, 3, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testEight1(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.lineTo(2, 2);
    path.lineTo(0, 2);
    path.lineTo(2, 0);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testEight2(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.lineTo(2, 0);
    path.lineTo(0, 2);
    path.lineTo(2, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testEight3(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.lineTo(0, 2);
    path.lineTo(2, 0);
    path.lineTo(2, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testEight4(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.lineTo(2, 2);
    path.lineTo(2, 0);
    path.lineTo(0, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testEight5(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(1, 0);
    path.lineTo(1, 2);
    path.lineTo(0, 2);
    path.lineTo(2, 0);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testEight6(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(1, 0);
    path.lineTo(2, 0);
    path.lineTo(0, 2);
    path.lineTo(1, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testEight7(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.lineTo(0, 1);
    path.lineTo(2, 1);
    path.lineTo(2, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testEight8(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.lineTo(2, 2);
    path.lineTo(2, 1);
    path.lineTo(0, 1);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testEight9(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(1, 0);
    path.lineTo(1, 2);
    path.lineTo(2, 1);
    path.lineTo(0, 1);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testEight10(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(1, 0);
    path.lineTo(0, 1);
    path.lineTo(2, 1);
    path.lineTo(1, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic80(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(1, 0, 2, 3);
    path.lineTo(2, 3);
    path.close();
    path.moveTo(1, 0);
    path.lineTo(3, 0);
    path.quadTo(0, 1, 1, 1);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic81(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(2, 0, 1, 1);
    path.lineTo(1, 1);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(2, 1, 0, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic82(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(2, 0, 1, 1);
    path.lineTo(0, 3);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(2, 1, 0, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic83(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(0, 0, 2, 0);
    path.lineTo(2, 2);
    path.close();
    path.moveTo(0, 1);
    path.lineTo(0, 2);
    path.quadTo(2, 2, 1, 3);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic84(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(2, 0, 1, 1);
    path.lineTo(2, 1);
    path.close();
    path.moveTo(1, 0);
    path.lineTo(2, 0);
    path.quadTo(0, 1, 2, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic85(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(3, 0, 1, 1);
    path.lineTo(1, 1);
    path.close();
    path.moveTo(1, 0);
    path.lineTo(3, 0);
    path.quadTo(0, 1, 1, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic86(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(0, 1, 1, 1);
    path.lineTo(2, 3);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(1, 1, 1, 3);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic87(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(2, 1, 0, 2);
    path.lineTo(2, 3);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(1, 1);
    path.quadTo(0, 2, 3, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic88(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(2, 1, 0, 2);
    path.lineTo(2, 2);
    path.close();
    path.moveTo(1, 0);
    path.lineTo(1, 1);
    path.quadTo(0, 2, 2, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic89x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.moveTo(0, 0);
    path.quadTo(3, 1, 2, 2);
    path.lineTo(0, 3);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(2, 1);
    path.quadTo(3, 1, 3, 3);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic90x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.moveTo(0, 0);
    path.quadTo(3, 0, 2, 2);
    path.lineTo(1, 3);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 1);
    path.quadTo(3, 2, 2, 3);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic91(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(3, 2, 2, 3);
    path.lineTo(2, 3);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(1, 1);
    path.quadTo(2, 1, 2, 3);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic92x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.moveTo(1, 0);
    path.quadTo(3, 0, 2, 2);
    path.lineTo(2, 2);
    path.close();
    path.moveTo(2, 0);
    path.lineTo(0, 1);
    path.quadTo(3, 2, 2, 3);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testLine82(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(20, 0, 40, 40, SkPathDirection::kCCW);
    path.addRect(24, 20, 36, 30, SkPathDirection::kCCW);
    path.addRect(24, 32, 33, 36, SkPathDirection::kCCW);
    testSimplify(reporter, path, filename);
}

static void testLine82a(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 6, 10, SkPathDirection::kCW);
    path.addRect(2, 2, 4, 4, SkPathDirection::kCW);
    path.addRect(2, 6, 4, 8, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine82b(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 6, 10, SkPathDirection::kCW);
    path.addRect(2, 2, 4, 4, SkPathDirection::kCW);
    path.addRect(2, 6, 4, 8, SkPathDirection::kCCW);
    testSimplify(reporter, path, filename);
}

static void testLine82c(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 6, 10, SkPathDirection::kCW);
    path.addRect(2, 2, 4, 4, SkPathDirection::kCCW);
    path.addRect(2, 6, 4, 8, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine82d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 6, 10, SkPathDirection::kCW);
    path.addRect(2, 2, 4, 4, SkPathDirection::kCCW);
    path.addRect(2, 6, 4, 8, SkPathDirection::kCCW);
    testSimplify(reporter, path, filename);
}

static void testLine82e(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 6, 10, SkPathDirection::kCCW);
    path.addRect(2, 2, 4, 4, SkPathDirection::kCW);
    path.addRect(2, 6, 4, 8, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine82f(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 6, 10, SkPathDirection::kCCW);
    path.addRect(2, 2, 4, 4, SkPathDirection::kCW);
    path.addRect(2, 6, 4, 8, SkPathDirection::kCCW);
    testSimplify(reporter, path, filename);
}

static void testLine82g(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 6, 10, SkPathDirection::kCCW);
    path.addRect(2, 2, 4, 4, SkPathDirection::kCCW);
    path.addRect(2, 6, 4, 8, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine82h(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 6, 10, SkPathDirection::kCCW);
    path.addRect(2, 2, 4, 4, SkPathDirection::kCCW);
    path.addRect(2, 6, 4, 8, SkPathDirection::kCCW);
    testSimplify(reporter, path, filename);
}

static void testLine83(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
path.addRect(10, 30, 30, 40, SkPathDirection::kCCW);
path.addRect(0, 12, 12, 18, SkPathDirection::kCCW);
path.addRect(4, 13, 13, 16, SkPathDirection::kCCW);
    testSimplify(reporter, path, filename);
}

static void testLine84(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 12, 60, 30, SkPathDirection::kCCW);
    path.addRect(10, 20, 40, 30, SkPathDirection::kCW);
    path.addRect(0, 12, 12, 12, SkPathDirection::kCW);
    path.addRect(4, 12, 13, 13, SkPathDirection::kCW);
    testSimplify(reporter, path, filename);
}

static void testLine84x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(0, 12, 60, 30, SkPathDirection::kCCW);
    path.addRect(10, 20, 40, 30, SkPathDirection::kCCW);
    path.addRect(0, 12, 12, 12, SkPathDirection::kCCW);
    path.addRect(4, 12, 13, 13, SkPathDirection::kCCW);
    testSimplify(reporter, path, filename);
}

static void testLine85(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(36, 0, 66, 60, SkPathDirection::kCCW);
    path.addRect(20, 0, 40, 40, SkPathDirection::kCCW);
    path.addRect(12, 0, 24, 24, SkPathDirection::kCCW);
    path.addRect(32, 0, 36, 41, SkPathDirection::kCCW);
    testSimplify(reporter, path, filename);
}

static void testQuadralateral1(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.lineTo(0, 0);
    path.lineTo(3, 2);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(2, 1);
    path.lineTo(2, 2);
    path.lineTo(2, 3);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testCubic1(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.cubicTo(0, 1, 1, 1, 1, 0);
    path.close();
    path.moveTo(1, 0);
    path.cubicTo(0, 0, 0, 1, 1, 1);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic93(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(3, 0);
    path.quadTo(0, 1, 3, 2);
    path.lineTo(0, 3);
    path.close();
    path.moveTo(1, 0);
    path.lineTo(2, 0);
    path.quadTo(1, 1, 2, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testCubic2(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0,2);
    path.cubicTo(0,3, 2,1, 4,0);
    path.close();
    path.moveTo(1,2);
    path.cubicTo(0,4, 2,0, 3,0);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuad1(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0,0);
    path.quadTo(0,0, 0,1);
    path.lineTo(1,1);
    path.close();
    path.moveTo(0,0);
    path.quadTo(1,1, 0,2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadralateral2(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.lineTo(2, 2);
    path.lineTo(0, 3);
    path.lineTo(3, 3);
    path.close();
    path.moveTo(2, 0);
    path.lineTo(3, 0);
    path.lineTo(0, 1);
    path.lineTo(1, 2);
    path.close();
    markTestFlakyForPathKit();
    testSimplify(reporter, path, filename);
}

static void testQuadratic94(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.lineTo(8, 8);
    path.quadTo(8, 4, 4, 4);
    path.quadTo(4, 0, 0, 0);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic95(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(8, 8);
    path.lineTo(0, 0);
    path.quadTo(4, 0, 4, 4);
    path.quadTo(8, 4, 8, 8);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic96(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(8, 0);
    path.lineTo(0, 8);
    path.quadTo(0, 4, 4, 4);
    path.quadTo(4, 0, 8, 0);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadratic97(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 8);
    path.lineTo(8, 0);
    path.quadTo(4, 0, 4, 4);
    path.quadTo(0, 4, 0, 8);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testTriangles1(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(3, 3);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(1, 2);
    path.lineTo(1, 1);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testTriangles2(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(3, 3);
    path.close();
    path.moveTo(1, 1);
    path.lineTo(2, 3);
    path.lineTo(1, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

// A test for this case:
// contourA has two segments that are coincident
// contourB has two segments that are coincident in the same place
// each ends up with +2/0 pairs for winding count
// since logic in OpSegment::addTCoincident doesn't transfer count (only increments/decrements)
// can this be resolved to +4/0 ?
static void testAddTCoincident1(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(2, 0);
    path.lineTo(2, 2);
    path.lineTo(1, 1);
    path.lineTo(2, 0);
    path.lineTo(2, 2);
    path.lineTo(1, 1);
    path.close();
    path.moveTo(2, 0);
    path.lineTo(2, 2);
    path.lineTo(3, 1);
    path.lineTo(2, 0);
    path.lineTo(2, 2);
    path.lineTo(3, 1);
    path.close();
    testSimplify(reporter, path, filename);
}

// test with implicit close
static void testAddTCoincident2(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(2, 0);
    path.lineTo(2, 2);
    path.lineTo(1, 1);
    path.lineTo(2, 0);
    path.lineTo(2, 2);
    path.lineTo(1, 1);
    path.moveTo(2, 0);
    path.lineTo(2, 2);
    path.lineTo(3, 1);
    path.lineTo(2, 0);
    path.lineTo(2, 2);
    path.lineTo(3, 1);
    testSimplify(reporter, path, filename);
}

static void testQuad2(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(1, 0);
    path.quadTo(0, 1, 3, 2);
    path.lineTo(2, 3);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.quadTo(0, 1, 1, 1);
    path.close();
}

static void testQuad3(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(1, 0);
    path.quadTo(0, 1, 3, 2);
    path.lineTo(3, 3);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.quadTo(0, 1, 1, 1);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuad4(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(2, 0);
    path.quadTo(0, 1, 1, 1);
    path.lineTo(3, 3);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(2, 0);
    path.quadTo(0, 1, 2, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuad5(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(2, 0);
    path.quadTo(0, 1, 2, 2);
    path.lineTo(1, 3);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(2, 0);
    path.quadTo(0, 1, 1, 1);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuad6(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(2, 0);
    path.quadTo(0, 1, 2, 2);
    path.lineTo(1, 3);
    path.close();
    path.moveTo(1, 0);
    path.lineTo(2, 0);
    path.quadTo(0, 1, 1, 1);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuad7(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(3, 0);
    path.quadTo(0, 1, 1, 1);
    path.lineTo(1, 3);
    path.close();
    path.moveTo(1, 0);
    path.lineTo(3, 0);
    path.quadTo(0, 1, 1, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadLineIntersect1(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(3, 1, 0, 3);
    path.lineTo(2, 3);
    path.close();
    path.moveTo(2, 0);
    path.lineTo(0, 1);
    path.quadTo(3, 1, 0, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadLineIntersect2(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(3, 1, 0, 3);
    path.lineTo(0, 3);
    path.close();
    path.moveTo(2, 0);
    path.lineTo(0, 1);
    path.quadTo(3, 1, 0, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadLineIntersect3(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(3, 1, 0, 3);
    path.lineTo(1, 3);
    path.close();
    path.moveTo(2, 0);
    path.lineTo(0, 1);
    path.quadTo(3, 1, 0, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void skphealth_com76s(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kWinding);
    path.moveTo(708.099182f, 7.09919119f);
    path.lineTo(708.099182f, 7.09920025f);
    path.quadTo(704.000000f, 11.2010098f, 704.000000f, 17.0000000f);
    path.lineTo(704.000000f, 33.0000000f);
    path.lineTo(705.000000f, 33.0000000f);
    path.lineTo(705.000000f, 17.0000000f);
    path.cubicTo(705.000000f, 13.4101496f, 706.455078f, 10.1601505f, 708.807617f, 7.80761385f);
    path.lineTo(708.099182f, 7.09919119f);
    path.close();
    path.moveTo(704.000000f, 3.00000000f);
    path.lineTo(704.000000f, 33.0000000f);
    path.lineTo(705.000000f, 33.0000000f);
    path.lineTo(719.500000f, 3.00000000f);
    testSimplify(reporter, path, filename);
}

static void tooCloseTest(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.lineTo(1, 1);
    path.lineTo(1,-1);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(1,-2);
    path.lineTo(1, 2);
    path.lineTo(2, 0);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testRect1s(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 60, 60, SkPathDirection::kCCW);
    path.addRect(30, 20, 50, 50, SkPathDirection::kCCW);
    path.addRect(24, 20, 36, 30, SkPathDirection::kCCW);
    path.addRect(32, 24, 36, 41, SkPathDirection::kCCW);
    testSimplify(reporter, path, filename);
}

static void testRect2s(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kWinding);
    path.moveTo(0, 0);
    path.lineTo(60, 0);
    path.lineTo(60, 60);
    path.lineTo(0, 60);
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
    path.moveTo(32, 24);
    path.lineTo(32, 41);
    path.lineTo(36, 41);
    path.lineTo(36, 24);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testTriangles3x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.moveTo(1, 0);
    path.quadTo(0, 1, 3, 2);
    path.lineTo(1, 3);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(1, 1);
    path.quadTo(2, 1, 0, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuad8(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(3, 0);
    path.quadTo(0, 1, 3, 2);
    path.lineTo(0, 3);
    path.close();
    path.moveTo(1, 0);
    path.lineTo(3, 0);
    path.quadTo(1, 1, 2, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testTriangles4x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.moveTo(0, 0);
    path.quadTo(2, 0, 0, 3);
    path.lineTo(2, 3);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 1);
    path.quadTo(3, 2, 2, 3);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuad9(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.moveTo(1, 0);
    path.quadTo(0, 1, 3, 2);
    path.lineTo(1, 3);
    path.close();
    path.moveTo(1, 0);
    path.lineTo(1, 1);
    path.quadTo(2, 1, 1, 3);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuad10(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(1, 0);
    path.quadTo(0, 1, 3, 2);
    path.lineTo(3, 3);
    path.close();
    path.moveTo(1, 0);
    path.lineTo(2, 0);
    path.quadTo(2, 3, 3, 3);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuad11(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(2, 0);
    path.quadTo(0, 1, 1, 2);
    path.lineTo(1, 2);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(1, 1);
    path.quadTo(1, 3, 3, 3);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuad12(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(0, 0, 0, 0);
    path.lineTo(0, 1);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(1, 0, 0, 1);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadralateral3(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.lineTo(0, 0);
    path.lineTo(1, 0);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(0, 1);
    path.close();
    testSimplify(reporter, path, filename);
}


static void testDegenerate5(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.lineTo(1, 0);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(0, 1);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadralateral4(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.lineTo(0, 0);
    path.lineTo(3, 1);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.lineTo(0, 1);
    path.lineTo(3, 1);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testDegenerates1(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(0, 0, 1, 1);
    path.lineTo(2, 3);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(3, 2, 2, 3);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuad13(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(0, 0, 1, 1);
    path.lineTo(2, 3);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(3, 2, 2, 3);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuad14(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kWinding);
    path.moveTo(0, 0);
    path.quadTo(0, 0, 1, 1);
    path.lineTo(1, 2);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(3, 1, 1, 3);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuad15(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(0, 0, 1, 1);
    path.lineTo(1, 3);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(2, 0, 1, 3);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuads16(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(0, 0, 1, 1);
    path.lineTo(3, 2);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(0, 1, 3, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuads17(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(0, 0, 1, 1);
    path.lineTo(3, 2);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(0, 2, 3, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuads18(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(0, 0, 1, 1);
    path.lineTo(3, 2);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(1, 2, 3, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuads19(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(0, 0, 2, 1);
    path.lineTo(1, 2);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(2, 1, 1, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuads20(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(0, 0, 2, 1);
    path.lineTo(1, 3);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(2, 1, 1, 3);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuads21(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(0, 0, 1, 1);
    path.lineTo(2, 1);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(3, 0, 2, 3);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuads22(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(0, 0, 2, 0);
    path.lineTo(1, 1);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(0, 1, 3, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuads23(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(0, 0, 1, 0);
    path.lineTo(1, 1);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(0, 1, 3, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuads24(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(0, 0, 0, 1);
    path.lineTo(1, 2);
    path.close();
    path.moveTo(0, 1);
    path.lineTo(0, 1);
    path.quadTo(0, 2, 3, 3);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuads25(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(0, 0, 0, 1);
    path.lineTo(2, 1);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(3, 0, 2, 3);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuads26(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(0, 0, 3, 0);
    path.lineTo(1, 1);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(0, 1, 3, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuads27(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(0, 0, 2, 0);
    path.lineTo(2, 1);
    path.close();
    path.moveTo(2, 0);
    path.lineTo(2, 0);
    path.quadTo(3, 0, 1, 3);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuads28(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(0, 0, 0, 1);
    path.lineTo(2, 2);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(3, 0, 2, 3);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuads29(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(0, 0, 3, 0);
    path.lineTo(2, 1);
    path.close();
    path.moveTo(3, 0);
    path.lineTo(3, 0);
    path.quadTo(3, 1, 0, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuads30(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);

    path.quadTo(0, 0, 2, 0);
    path.lineTo(2, 2);
    path.close();
    path.moveTo(2, 0);
    path.lineTo(2, 0);
    path.quadTo(3, 2, 1, 3);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuads31(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(0, 0, 2, 1);
    path.lineTo(1, 3);
    path.close();
    path.moveTo(3, 0);
    path.lineTo(0, 1);

    path.quadTo(2, 1, 1, 3);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuads32(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(0, 0, 1, 1);
    path.lineTo(1, 2);
    path.close();
    path.moveTo(1, 1);
    path.lineTo(1, 1);
    path.quadTo(3, 1, 0, 3);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuads33(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(0, 0, 1, 1);
    path.lineTo(2, 1);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(3, 0, 2, 3);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuads34(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(0, 0, 1, 0);
    path.lineTo(2, 1);
    path.close();
    path.moveTo(1, 0);
    path.lineTo(1, 0);
    path.quadTo(2, 0, 3, 3);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuads35(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(0, 0, 1, 0);
    path.lineTo(1, 2);
    path.close();
    path.moveTo(1, 0);
    path.lineTo(1, 0);
    path.quadTo(3, 1, 0, 3);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuads36(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(1, 0);
    path.quadTo(2, 0, 1, 2);
    path.lineTo(2, 2);
    path.close();
    path.moveTo(1, 0);
    path.lineTo(1, 0);
    path.quadTo(3, 0, 2, 3);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuads37(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.moveTo(1, 0);
    path.quadTo(2, 0, 1, 2);
    path.lineTo(2, 2);
    path.close();
    path.moveTo(1, 0);
    path.lineTo(1, 0);
    path.quadTo(3, 0, 2, 3);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuads38(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(1, 0);
    path.quadTo(3, 0, 0, 2);
    path.lineTo(3, 2);
    path.close();
    path.moveTo(1, 0);
    path.lineTo(1, 0);
    path.quadTo(2, 1, 3, 1);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuads39(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(1, 0);
    path.quadTo(3, 0, 0, 3);
    path.lineTo(0, 3);
    path.close();
    path.moveTo(1, 1);
    path.lineTo(0, 2);
    path.quadTo(1, 2, 0, 3);
    path.close();
    testSimplify(reporter, path, filename);
}
static void testQuads40(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(1, 0);
    path.quadTo(3, 0, 3, 3);
    path.lineTo(3, 3);
    path.close();
    path.moveTo(2, 1);
    path.lineTo(2, 2);
    path.quadTo(3, 2, 3, 3);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuads41(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(0, 0, 1, 0);
    path.lineTo(2, 1);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(0, 1, 1, 2);
    path.close();
    testSimplify(reporter, path, filename);
}


static void testQuads54(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(1, 0);
    path.quadTo(2, 0, 1, 1);
    path.lineTo(3, 1);
    path.close();
    path.moveTo(2, 0);
    path.lineTo(1, 1);
    path.quadTo(1, 1, 2, 3);
    path.close();
    testSimplify(reporter, path, filename);
}
static void testQuads53(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(1, 0);
    path.quadTo(2, 0, 1, 1);
    path.lineTo(3, 1);
    path.close();
    path.moveTo(2, 0);
    path.lineTo(1, 1);
    path.quadTo(2, 3, 2, 3);
    path.close();
    testSimplify(reporter, path, filename);
}
static void testQuads52(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(1, 0);
    path.quadTo(2, 0, 1, 1);
    path.lineTo(3, 1);
    path.close();
    path.moveTo(2, 0);
    path.lineTo(1, 1);
    path.quadTo(2, 3, 3, 3);
    path.close();
    testSimplify(reporter, path, filename);
}
static void testQuads51(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(2, 0);
    path.quadTo(3, 0, 2, 1);
    path.lineTo(3, 2);
    path.close();
    path.moveTo(3, 0);
    path.lineTo(3, 1);
    path.quadTo(3, 1, 1, 2);
    path.close();
    testSimplify(reporter, path, filename);
}
static void testQuads50(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(2, 0);
    path.quadTo(3, 0, 2, 1);
    path.lineTo(3, 2);
    path.close();
    path.moveTo(3, 0);
    path.lineTo(3, 1);
    path.quadTo(1, 2, 1, 2);
    path.close();
    testSimplify(reporter, path, filename);
}
static void testQuads49(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(2, 0);
    path.quadTo(3, 0, 2, 1);
    path.lineTo(3, 2);
    path.close();
    path.moveTo(3, 0);
    path.lineTo(2, 2);
    path.quadTo(2, 2, 0, 3);
    path.close();
    testSimplify(reporter, path, filename);
}
static void testQuads48(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(2, 0);
    path.quadTo(3, 0, 2, 1);
    path.lineTo(3, 2);
    path.close();
    path.moveTo(3, 0);
    path.lineTo(2, 2);
    path.quadTo(3, 2, 0, 3);
    path.close();
    testSimplify(reporter, path, filename);
}
static void testQuads47(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(2, 0);
    path.quadTo(3, 0, 2, 1);
    path.lineTo(3, 2);
    path.close();
    path.moveTo(3, 0);
    path.lineTo(2, 2);
    path.quadTo(0, 3, 0, 3);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuads46x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.moveTo(2, 0);
    path.quadTo(0, 1, 3, 2);
    path.lineTo(1, 3);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(1, 1);
    path.quadTo(3, 2, 1, 3);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuads45(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(2, 0);
    path.quadTo(3, 2, 3, 3);
    path.lineTo(3, 3);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 2);
    path.quadTo(3, 2, 3, 3);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuads44(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(2, 0);
    path.quadTo(3, 2, 3, 3);
    path.lineTo(3, 3);
    path.close();
    path.moveTo(1, 0);
    path.lineTo(0, 2);
    path.quadTo(3, 2, 3, 3);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuads43(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(2, 0);
    path.quadTo(2, 3, 3, 3);
    path.lineTo(3, 3);
    path.close();
    path.moveTo(0, 2);
    path.lineTo(0, 2);
    path.quadTo(2, 3, 3, 3);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuads42(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(2, 0);
    path.quadTo(3, 2, 3, 3);
    path.lineTo(3, 3);
    path.close();
    path.moveTo(2, 0);
    path.lineTo(0, 2);
    path.quadTo(3, 2, 3, 3);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuads56(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(2, 0);
    path.quadTo(3, 1, 0, 2);
    path.lineTo(3, 2);
    path.close();
    path.moveTo(3, 0);
    path.lineTo(2, 1);
    path.quadTo(2, 1, 3, 3);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuads57(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(1, 0);
    path.quadTo(3, 0, 3, 1);
    path.lineTo(2, 2);
    path.close();
    path.moveTo(2, 0);
    path.lineTo(3, 1);
    path.quadTo(2, 2, 3, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuads58(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(1, 0);
    path.quadTo(3, 0, 3, 1);
    path.lineTo(1, 3);
    path.close();
    path.moveTo(2, 0);
    path.lineTo(3, 1);
    path.quadTo(2, 2, 3, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuads59(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(3, 0);
    path.quadTo(3, 1, 3, 1);
    path.lineTo(2, 2);
    path.close();
    path.moveTo(2, 0);
    path.lineTo(3, 1);
    path.quadTo(2, 2, 3, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuads60(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(2, 1);
    path.quadTo(0, 2, 3, 2);
    path.lineTo(2, 3);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(2, 0);
    path.quadTo(1, 1, 2, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuads61(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.moveTo(0, 0);
    path.quadTo(0, 0, 2, 0);
    path.lineTo(1, 1);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(1, 0, 2, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuadralateral10(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kWinding);
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.lineTo(0, 0);
    path.lineTo(2, 2);
    path.close();
    path.moveTo(1, 0);
    path.lineTo(1, 1);
    path.lineTo(2, 2);
    path.lineTo(1, 3);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testRect3(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(0, 0, 60, 60, SkPathDirection::kCCW);
    path.addRect(10, 30, 40, 30, SkPathDirection::kCCW);
    path.addRect(24, 6, 36, 36, SkPathDirection::kCCW);
    path.addRect(32, 6, 36, 41, SkPathDirection::kCCW);
    testSimplify(reporter, path, filename);
}

static void testRect4(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.addRect(0, 0, 30, 60, SkPathDirection::kCCW);
    path.addRect(10, 0, 40, 30, SkPathDirection::kCCW);
    path.addRect(20, 0, 30, 40, SkPathDirection::kCCW);
    path.addRect(32, 0, 36, 41, SkPathDirection::kCCW);
    testSimplify(reporter, path, filename);
}

static void testQuads62(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(3, 2);
    path.quadTo(1, 3, 3, 3);
    path.lineTo(3, 3);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(2, 0);
    path.quadTo(1, 3, 3, 3);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuads63(skiatest::Reporter* reporter,const char* filename) {
    SkPath path;
    path.moveTo(3, 0);
    path.quadTo(0, 1, 1, 2);
    path.lineTo(3, 3);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(1, 1);
    path.quadTo(0, 2, 3, 3);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuads64(skiatest::Reporter* reporter,const char* filename) {
    SkPath path;
    path.moveTo(3, 0);
    path.quadTo(0, 1, 1, 2);
    path.lineTo(2, 2);
    path.close();
    path.moveTo(1, 0);
    path.lineTo(1, 1);
    path.quadTo(0, 2, 3, 3);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testTriangle1(skiatest::Reporter* reporter,const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(2, 3);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(1, 2);
    path.lineTo(1, 0);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testTriangle2(skiatest::Reporter* reporter,const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(0, 1);
    path.close();
    path.moveTo(2, 0);
    path.lineTo(0, 2);
    path.lineTo(2, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testArc(skiatest::Reporter* reporter,const char* filename) {
    SkRect r = SkRect::MakeWH(150, 100);
    SkPath path;
    path.arcTo(r, 0, 0.0025f, false);
    testSimplify(reporter, path, filename);
}

static void testIssue3838(skiatest::Reporter* reporter,const char* filename) {
    SkPath path;
    path.moveTo(220, 170);
    path.lineTo(200, 170);
    path.lineTo(200, 190);
    path.lineTo(180, 190);
    path.lineTo(180, 210);
    path.lineTo(200, 210);
    path.lineTo(200, 250);
    path.lineTo(260, 250);
    path.lineTo(260, 190);
    path.lineTo(220, 190);
    path.lineTo(220, 170);
    path.close();
    path.moveTo(220, 210);
    path.lineTo(220, 230);
    path.lineTo(240, 230);
    path.lineTo(240, 210);
    path.lineTo(220, 210);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testIssue3838_3(skiatest::Reporter* reporter,const char* filename) {
    SkPath path;
    path.moveTo(40, 10);
    path.lineTo(60, 10);
    path.lineTo(60, 30);
    path.lineTo(40, 30);
    path.lineTo(40, 10);
    path.moveTo(41, 11);
    path.lineTo(41, 29);
    path.lineTo(59, 29);
    path.lineTo(59, 11);
    path.lineTo(41, 11);
    testSimplify(reporter, path, filename);
}

static void testQuads65(skiatest::Reporter* reporter,const char* filename) {
    SkPath path;
    path.moveTo(1, 2);
    path.quadTo(3, 2, 0, 3);
    path.lineTo(1, 3);
    path.close();
    path.moveTo(1, 0);
    path.lineTo(1, 2);
    path.quadTo(3, 2, 1, 3);
    path.close();
    testSimplify(reporter, path, filename);
}

static void fuzz864a(skiatest::Reporter* reporter,const char* filename) {
    SkPath path;
    path.moveTo(10, 90);
    path.lineTo(10, 90);
    path.lineTo(10, 30);
    path.lineTo(10, 30);
    path.lineTo(10, 90);
    path.close();
    path.moveTo(10, 90);
    path.lineTo(10, 90);
    path.lineTo(10, 30);
    path.lineTo(10, 30);
    path.lineTo(10, 90);
    path.close();
    path.moveTo(10, 90);
    path.lineTo(110, 90);
    path.lineTo(110, 30);
    path.lineTo(10, 30);
    path.lineTo(10, 90);
    path.close();
    path.moveTo(10, 30);
    path.lineTo(32678, 30);
    path.lineTo(32678, 30);
    path.lineTo(10, 30);
    path.close();
    path.moveTo(10, 3.35545e+07f);
    path.lineTo(110, 3.35545e+07f);
    path.lineTo(110, 30);
    path.lineTo(10, 30);
    path.lineTo(10, 3.35545e+07f);
    path.close();
    path.moveTo(10, 315);
    path.lineTo(110, 315);
    path.lineTo(110, 255);
    path.lineTo(10, 255);
    path.lineTo(10, 315);
    path.close();
    path.moveTo(0, 60);
    path.lineTo(100, 60);
    path.lineTo(100, 0);
    path.lineTo(0, 0);
    path.lineTo(0, 60);
    path.close();
    path.moveTo(10, 90);
    path.lineTo(110, 90);
    path.lineTo(110, 30);
    path.lineTo(10, 30);
    path.lineTo(10, 90);
    path.close();
    path.moveTo(10, 3.35545e+07f);
    path.lineTo(110, 3.35545e+07f);
    path.lineTo(110, 30);
    path.lineTo(10, 30);
    path.lineTo(10, 3.35545e+07f);
    path.close();
    path.moveTo(10, 90);
    path.lineTo(110, 90);
    path.lineTo(110, 30);
    path.lineTo(10, 30);
    path.lineTo(10, 90);
    path.close();
    testSimplify(reporter, path, filename);
}

static void cr514118(skiatest::Reporter* reporter,const char* filename) {
    SkPath path;
path.moveTo(SkBits2Float(0x42c80000), SkBits2Float(0x42480000));  // 100, 50
path.conicTo(SkBits2Float(0x42c80000), SkBits2Float(0x00000000), SkBits2Float(0x42480000), SkBits2Float(0x00000000), SkBits2Float(0x3f3504f3));  // 100, 0, 50, 0, 0.707107f
path.conicTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000), SkBits2Float(0x00000000), SkBits2Float(0x42480000), SkBits2Float(0x3f3504f3));  // 0, 0, 0, 50, 0.707107f
path.conicTo(SkBits2Float(0x00000000), SkBits2Float(0x42c80000), SkBits2Float(0x42480000), SkBits2Float(0x42c80000), SkBits2Float(0x3f3504f3));  // 0, 100, 50, 100, 0.707107f
path.conicTo(SkBits2Float(0x42c80000), SkBits2Float(0x42c80000), SkBits2Float(0x42c80000), SkBits2Float(0x42480000), SkBits2Float(0x3f3504f3));  // 100, 100, 100, 50, 0.707107f
path.close();
path.moveTo(SkBits2Float(0x42c80133), SkBits2Float(0x42480000));  // 100.002f, 50
path.conicTo(SkBits2Float(0x42c80133), SkBits2Float(0x00000000), SkBits2Float(0x42480267), SkBits2Float(0x00000000), SkBits2Float(0x3f3504f3));  // 100.002f, 0, 50.0023f, 0, 0.707107f
path.conicTo(SkBits2Float(0x3b19b530), SkBits2Float(0x00000000), SkBits2Float(0x3b19b530), SkBits2Float(0x42480000), SkBits2Float(0x3f3504f3));  // 0.00234539f, 0, 0.00234539f, 50, 0.707107f
path.conicTo(SkBits2Float(0x3b19b530), SkBits2Float(0x42c80000), SkBits2Float(0x42480267), SkBits2Float(0x42c80000), SkBits2Float(0x3f3504f3));  // 0.00234539f, 100, 50.0023f, 100, 0.707107f
path.conicTo(SkBits2Float(0x42c80133), SkBits2Float(0x42c80000), SkBits2Float(0x42c80133), SkBits2Float(0x42480000), SkBits2Float(0x3f3504f3));  // 100.002f, 100, 100.002f, 50, 0.707107f
path.close();
    testSimplify(reporter, path, filename);
}

static void fuzz994s_11(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPathFillType) 0);
path.moveTo(SkBits2Float(0x41200000), SkBits2Float(0x42b40000));  // 10, 90
path.lineTo(SkBits2Float(0x41200000), SkBits2Float(0x42b40000));  // 10, 90
path.lineTo(SkBits2Float(0x41200000), SkBits2Float(0x41f00000));  // 10, 30
path.lineTo(SkBits2Float(0x41200000), SkBits2Float(0x41f00000));  // 10, 30
path.lineTo(SkBits2Float(0x41200000), SkBits2Float(0x42b40000));  // 10, 90
path.close();
path.moveTo(SkBits2Float(0x41200000), SkBits2Float(0x42b40000));  // 10, 90
path.lineTo(SkBits2Float(0x41200000), SkBits2Float(0x42b40000));  // 10, 90
path.lineTo(SkBits2Float(0x41200000), SkBits2Float(0x41f00000));  // 10, 30
path.lineTo(SkBits2Float(0x41200000), SkBits2Float(0x41f00000));  // 10, 30
path.lineTo(SkBits2Float(0x41200000), SkBits2Float(0x42b40000));  // 10, 90
path.close();
path.moveTo(SkBits2Float(0x41200000), SkBits2Float(0x42b40000));  // 10, 90
path.lineTo(SkBits2Float(0x42dc0000), SkBits2Float(0x42b40000));  // 110, 90
path.lineTo(SkBits2Float(0x42dc0000), SkBits2Float(0x41f00000));  // 110, 30
path.lineTo(SkBits2Float(0x41200000), SkBits2Float(0x41f00000));  // 10, 30
path.lineTo(SkBits2Float(0x41200000), SkBits2Float(0x42b40000));  // 10, 90
path.close();
path.moveTo(SkBits2Float(0x41200000), SkBits2Float(0x41f00000));  // 10, 30
path.lineTo(SkBits2Float(0x46ff4c00), SkBits2Float(0x41f00000));  // 32678, 30
path.lineTo(SkBits2Float(0x46ff4c00), SkBits2Float(0x41f00000));  // 32678, 30
path.lineTo(SkBits2Float(0x41200000), SkBits2Float(0x41f00000));  // 10, 30
path.close();
path.moveTo(SkBits2Float(0x41200000), SkBits2Float(0x4c000006));  // 10, 3.35545e+07f
path.lineTo(SkBits2Float(0x42dc0000), SkBits2Float(0x4c000006));  // 110, 3.35545e+07f
path.lineTo(SkBits2Float(0x42dc0000), SkBits2Float(0x41f00000));  // 110, 30
path.lineTo(SkBits2Float(0x41200000), SkBits2Float(0x41f00000));  // 10, 30
path.lineTo(SkBits2Float(0x41200000), SkBits2Float(0x4c000006));  // 10, 3.35545e+07f
path.close();
path.moveTo(SkBits2Float(0x41200000), SkBits2Float(0x439d8000));  // 10, 315
path.lineTo(SkBits2Float(0x42dc0000), SkBits2Float(0x439d8000));  // 110, 315
path.lineTo(SkBits2Float(0x42dc0000), SkBits2Float(0x437f0000));  // 110, 255
path.lineTo(SkBits2Float(0x41200000), SkBits2Float(0x437f0000));  // 10, 255
path.lineTo(SkBits2Float(0x41200000), SkBits2Float(0x439d8000));  // 10, 315
path.close();
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x42700000));  // 0, 60
path.lineTo(SkBits2Float(0x42c80000), SkBits2Float(0x42700000));  // 100, 60
path.lineTo(SkBits2Float(0x42c80000), SkBits2Float(0x00000000));  // 100, 0
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0x42700000));  // 0, 60
path.close();
path.moveTo(SkBits2Float(0x41200000), SkBits2Float(0x42b40000));  // 10, 90
path.lineTo(SkBits2Float(0x42dc0000), SkBits2Float(0x42b40000));  // 110, 90
path.lineTo(SkBits2Float(0x42dc0000), SkBits2Float(0x41f00000));  // 110, 30
path.lineTo(SkBits2Float(0x41200000), SkBits2Float(0x41f00000));  // 10, 30
path.lineTo(SkBits2Float(0x41200000), SkBits2Float(0x42b40000));  // 10, 90
path.close();
path.moveTo(SkBits2Float(0x41200000), SkBits2Float(0x4c000006));  // 10, 3.35545e+07f
path.lineTo(SkBits2Float(0x42dc0000), SkBits2Float(0x4c000006));  // 110, 3.35545e+07f
path.lineTo(SkBits2Float(0x42dc0000), SkBits2Float(0x41f00000));  // 110, 30
path.lineTo(SkBits2Float(0x41200000), SkBits2Float(0x41f00000));  // 10, 30
path.lineTo(SkBits2Float(0x41200000), SkBits2Float(0x4c000006));  // 10, 3.35545e+07f
path.close();
path.moveTo(SkBits2Float(0x41200000), SkBits2Float(0x42b40000));  // 10, 90
path.lineTo(SkBits2Float(0x42dc0000), SkBits2Float(0x42b40000));  // 110, 90
path.lineTo(SkBits2Float(0x42dc0000), SkBits2Float(0x41f00000));  // 110, 30
path.lineTo(SkBits2Float(0x41200000), SkBits2Float(0x41f00000));  // 10, 30
path.lineTo(SkBits2Float(0x41200000), SkBits2Float(0x42b40000));  // 10, 90
path.close();

    testSimplify(reporter, path, filename);
}

static void fuzz994s_3414(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPathFillType) 0);
path.moveTo(SkBits2Float(0x42c80000), SkBits2Float(0x42480000));  // 100, 50
path.conicTo(SkBits2Float(0x42c80000), SkBits2Float(0x00000000), SkBits2Float(0x42480000), SkBits2Float(0x00000000), SkBits2Float(0x3f3504f3));  // 100, 0, 50, 0, 0.707107f
path.conicTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000), SkBits2Float(0x00000000), SkBits2Float(0x42480000), SkBits2Float(0x3f3504f3));  // 0, 0, 0, 50, 0.707107f
path.conicTo(SkBits2Float(0x00000000), SkBits2Float(0x42c80000), SkBits2Float(0x42480000), SkBits2Float(0x42c80000), SkBits2Float(0x3f3504f3));  // 0, 100, 50, 100, 0.707107f
path.conicTo(SkBits2Float(0x42c80000), SkBits2Float(0x42c80000), SkBits2Float(0x42c80000), SkBits2Float(0x42480000), SkBits2Float(0x3f3504f3));  // 100, 100, 100, 50, 0.707107f
path.close();
path.moveTo(SkBits2Float(0x42c84964), SkBits2Float(0x42480000));  // 100.143f, 50
path.conicTo(SkBits2Float(0x42c84964), SkBits2Float(0x00000000), SkBits2Float(0x424892c8), SkBits2Float(0x00000000), SkBits2Float(0x3f3504f3));  // 100.143f, 0, 50.1433f, 0, 0.707107f
path.conicTo(SkBits2Float(0x3e12c788), SkBits2Float(0x00000000), SkBits2Float(0x3e12c788), SkBits2Float(0x42480000), SkBits2Float(0x3f3504f3));  // 0.143339f, 0, 0.143339f, 50, 0.707107f
path.conicTo(SkBits2Float(0x3e12c788), SkBits2Float(0x42c80000), SkBits2Float(0x424892c8), SkBits2Float(0x42c80000), SkBits2Float(0x3f3504f3));  // 0.143339f, 100, 50.1433f, 100, 0.707107f
path.conicTo(SkBits2Float(0x42c84964), SkBits2Float(0x42c80000), SkBits2Float(0x42c84964), SkBits2Float(0x42480000), SkBits2Float(0x3f3504f3));  // 100.143f, 100, 100.143f, 50, 0.707107f
path.close();
path.moveTo(SkBits2Float(0x42c80000), SkBits2Float(0x42480000));  // 100, 50
path.conicTo(SkBits2Float(0x42c80000), SkBits2Float(0x00000000), SkBits2Float(0x42480000), SkBits2Float(0x00000000), SkBits2Float(0x3f3504f3));  // 100, 0, 50, 0, 0.707107f
path.conicTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000), SkBits2Float(0x00000000), SkBits2Float(0x42480000), SkBits2Float(0x3f3504f3));  // 0, 0, 0, 50, 0.707107f
path.conicTo(SkBits2Float(0x00000000), SkBits2Float(0x42c80000), SkBits2Float(0x42480000), SkBits2Float(0x42c80000), SkBits2Float(0x3f3504f3));  // 0, 100, 50, 100, 0.707107f
path.conicTo(SkBits2Float(0x42c80000), SkBits2Float(0x42c80000), SkBits2Float(0x42c80000), SkBits2Float(0x42480000), SkBits2Float(0x3f3504f3));  // 100, 100, 100, 50, 0.707107f
path.close();
path.moveTo(SkBits2Float(0x4c00006b), SkBits2Float(0x424c0000));  // 3.35549e+07f, 51
path.conicTo(SkBits2Float(0x4c00006b), SkBits2Float(0xcbffffe5), SkBits2Float(0x43d6e720), SkBits2Float(0xcbffffe5), SkBits2Float(0x3f3504f3));  // 3.35549e+07f, -3.35544e+07f, 429.806f, -3.35544e+07f, 0.707107f
path.conicTo(SkBits2Float(0xcbffff28), SkBits2Float(0xcbffffe5), SkBits2Float(0xcbffff28), SkBits2Float(0x424c0000), SkBits2Float(0x3f3504f3));  // -3.3554e+07f, -3.35544e+07f, -3.3554e+07f, 51, 0.707107f
path.conicTo(SkBits2Float(0xcbffff28), SkBits2Float(0x4c00000c), SkBits2Float(0x43d6e720), SkBits2Float(0x4c00000c), SkBits2Float(0x3f3504f3));  // -3.3554e+07f, 3.35545e+07f, 429.806f, 3.35545e+07f, 0.707107f
path.conicTo(SkBits2Float(0x4c00006b), SkBits2Float(0x4c00000c), SkBits2Float(0x4c00006b), SkBits2Float(0x424c0000), SkBits2Float(0x3f3504f3));  // 3.35549e+07f, 3.35545e+07f, 3.35549e+07f, 51, 0.707107f
path.close();
path.moveTo(SkBits2Float(0x43ef6720), SkBits2Float(0x42480000));  // 478.806f, 50
path.conicTo(SkBits2Float(0x43ef6720), SkBits2Float(0x00000000), SkBits2Float(0x43d66720), SkBits2Float(0x00000000), SkBits2Float(0x3f3504f3));  // 478.806f, 0, 428.806f, 0, 0.707107f
path.conicTo(SkBits2Float(0x43bd6720), SkBits2Float(0x00000000), SkBits2Float(0x43bd6720), SkBits2Float(0x42480000), SkBits2Float(0x3f3504f3));  // 378.806f, 0, 378.806f, 50, 0.707107f
path.conicTo(SkBits2Float(0x43bd6720), SkBits2Float(0x42c80000), SkBits2Float(0x43d66720), SkBits2Float(0x42c80000), SkBits2Float(0x3f3504f3));  // 378.806f, 100, 428.806f, 100, 0.707107f
path.conicTo(SkBits2Float(0x43ef6720), SkBits2Float(0x42c80000), SkBits2Float(0x43ef6720), SkBits2Float(0x42480000), SkBits2Float(0x3f3504f3));  // 478.806f, 100, 478.806f, 50, 0.707107f
path.close();

    testSimplify(reporter, path, filename);
}

static void fuzz_twister(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPathFillType) 0);
path.moveTo(0, 600);
path.lineTo(3.35544e+07f, 600);
path.lineTo(3.35544e+07f, 0);
path.lineTo(0, 0);
path.lineTo(0, 600);
path.close();
path.moveTo(63, 600);
path.lineTo(3.35545e+07f, 600);
path.lineTo(3.35545e+07f, 0);
path.lineTo(63, 0);
path.lineTo(63, 600);
path.close();
path.moveTo(93, 600);
path.lineTo(3.35545e+07f, 600);
path.lineTo(3.35545e+07f, 0);
path.lineTo(93, 0);
path.lineTo(93, 600);
path.close();
path.moveTo(123, 600);
path.lineTo(3.35546e+07f, 600);
path.lineTo(3.35546e+07f, 0);
path.lineTo(123, 0);
path.lineTo(123, 600);
path.close();
    testSimplify(reporter, path, filename);
}

static void fuzz_twister2(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;

path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x44160000));  // 0, 600
path.lineTo(SkBits2Float(0x4bfffffe), SkBits2Float(0x44160000));  // 3.35544e+07f, 600
path.lineTo(SkBits2Float(0x4bfffffe), SkBits2Float(0x00000000));  // 3.35544e+07f, 0
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0x44160000));  // 0, 600
path.close();

path.moveTo(SkBits2Float(0x427c0000), SkBits2Float(0x00000000));  // 63, 0
path.lineTo(SkBits2Float(0x4c00000f), SkBits2Float(0x00000000));  // 3.35545e+07f, 0
path.lineTo(SkBits2Float(0x4c00000f), SkBits2Float(0x00000000));  // 3.35545e+07f, 0
path.lineTo(SkBits2Float(0x427c0000), SkBits2Float(0x00000000));  // 63, 0
path.close();

path.moveTo(SkBits2Float(0x42ba0000), SkBits2Float(0x00000000));  // 93, 0
path.lineTo(SkBits2Float(0x4c000016), SkBits2Float(0x00000000));  // 3.35545e+07f, 0
path.lineTo(SkBits2Float(0x4c000016), SkBits2Float(0x00000000));  // 3.35545e+07f, 0
path.lineTo(SkBits2Float(0x42ba0000), SkBits2Float(0x00000000));  // 93, 0
path.close();

path.moveTo(SkBits2Float(0x42f60000), SkBits2Float(0x00000000));  // 123, 0
path.lineTo(SkBits2Float(0x4c00001e), SkBits2Float(0x00000000));  // 3.35546e+07f, 0
path.lineTo(SkBits2Float(0x4c00001e), SkBits2Float(0x00000000));  // 3.35546e+07f, 0
path.lineTo(SkBits2Float(0x42f60000), SkBits2Float(0x00000000));  // 123, 0
path.close();

    testSimplify(reporter, path, filename);
}

static void fuzz763_4713_b(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPathFillType) 0);
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
testSimplify(reporter, path, filename);
}

static void dean4(skiatest::Reporter* reporter, const char* filename) {
  SkPath path;

  // start region
  // start loop, contour: 1
  // Segment 1145.3381097316742 2017.6783947944641 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1145.3381097316742 2017.0033947825432
  path.moveTo(1145.3381347656250, 2017.6783447265625);
  path.lineTo(1145.3381347656250, 2017.0034179687500);
  // Segment 1145.3381097316742 2017.0033947825432 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1143.6927231521568 2017.0033947825432
  path.lineTo(1143.6927490234375, 2017.0034179687500);
  // Segment 1143.6927231521568 2017.0033947825432 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1144.8640675112890 2018.1589246992417
  path.lineTo(1144.8640136718750, 2018.1589355468750);
  // Segment 1144.8640675112890 2018.1589246992417 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1145.3381097316742 2017.6783947944641
  path.lineTo(1145.3381347656250, 2017.6783447265625);
  path.close();
  // start loop, contour: 2
  // Segment 1145.3381097316742 2016.3216052055359 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1144.8640675258462 2015.8410752863977
  path.moveTo(1145.3381347656250, 2016.3216552734375);
  path.lineTo(1144.8640136718750, 2015.8410644531250);
  // Segment 1144.8640675258462 2015.8410752863977 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1143.6927230811802 2016.9966052174568
  path.lineTo(1143.6927490234375, 2016.9965820312500);
  // Segment 1143.6927230811802 2016.9966052174568 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1145.3381097316742 2016.9966052174568
  path.lineTo(1145.3381347656250, 2016.9965820312500);
  // Segment 1145.3381097316742 2016.9966052174568 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1145.3381097316742 2016.3216052055359
  path.lineTo(1145.3381347656250, 2016.3216552734375);
  path.close();
  // start loop, contour: 3
  // Segment 1147.3323798179626 2014.3542600870132 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1147.8064220239557 2014.8347900059885
  path.moveTo(1147.3323974609375, 2014.3542480468750);
  path.lineTo(1147.8063964843750, 2014.8348388671875);
  // Segment 1147.8064220239557 2014.8347900059885 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1147.8064220516883 2014.8347899786306
  path.lineTo(1147.8063964843750, 2014.8348388671875);
  // Segment 1147.8064220516883 2014.8347899786306 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1147.3323798179626 2014.3542600870132
  path.lineTo(1147.3323974609375, 2014.3542480468750);
  path.close();
  // start loop, contour: 4
  // Segment 1146.3696286678314 2013.4045072346926 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1146.8436708778083 2013.8850371497379
  path.moveTo(1146.3696289062500, 2013.4045410156250);
  path.lineTo(1146.8436279296875, 2013.8850097656250);
  // Segment 1146.8436708778083 2013.8850371497379 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1146.8436709015571 2013.8850371263100
  path.lineTo(1146.8436279296875, 2013.8850097656250);
  // Segment 1146.8436709015571 2013.8850371263100 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1146.3696286678314 2013.4045072346926
  path.lineTo(1146.3696289062500, 2013.4045410156250);
  path.close();
  // start loop, contour: 5
  // Segment 1143.2063037902117 2016.5251235961914 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1142.7322615802348 2016.0445936811461
  path.moveTo(1143.2062988281250, 2016.5251464843750);
  path.lineTo(1142.7322998046875, 2016.0445556640625);
  // Segment 1142.7322615802348 2016.0445936811461 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1142.7322615564860 2016.0445937045740
  path.lineTo(1142.7322998046875, 2016.0445556640625);
  // Segment 1142.7322615564860 2016.0445937045740 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1143.2063037902117 2016.5251235961914
  path.lineTo(1143.2062988281250, 2016.5251464843750);
  path.close();
  // start loop, contour: 6
  // Segment 1143.0687679275870 2016.7286419868469 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1143.5428101613127 2017.2091718784643
  path.moveTo(1143.0687255859375, 2016.7286376953125);
  path.lineTo(1143.5428466796875, 2017.2092285156250);
  // Segment 1143.5428101613127 2017.2091718784643 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1143.7437679395080 2017.0109272411960
  path.lineTo(1143.7437744140625, 2017.0109863281250);
  // Segment 1143.7437679395080 2017.0109272411960 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1143.7437679395080 2016.7286419868469
  path.lineTo(1143.7437744140625, 2016.7286376953125);
  // Segment 1143.7437679395080 2016.7286419868469 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1143.0687679275870 2016.7286419868469
  path.lineTo(1143.0687255859375, 2016.7286376953125);
  path.close();
  // start loop, contour: 7
  // Segment 1143.2063037902117 2017.4748764038086 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1142.7322615603032 2017.9554062991915
  path.moveTo(1143.2062988281250, 2017.4748535156250);
  path.lineTo(1142.7322998046875, 2017.9554443359375);
  // Segment 1142.7322615603032 2017.9554062991915 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1142.7322615746241 2017.9554063133189
  path.lineTo(1142.7322998046875, 2017.9554443359375);
  // Segment 1142.7322615746241 2017.9554063133189 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1143.2063037902117 2017.4748764038086
  path.lineTo(1143.2062988281250, 2017.4748535156250);
  path.close();
  // start loop, contour: 8
  // Segment 1146.3696286678314 2020.5954928398132 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1146.8436708977399 2020.1149629444303
  path.moveTo(1146.3696289062500, 2020.5954589843750);
  path.lineTo(1146.8436279296875, 2020.1149902343750);
  // Segment 1146.8436708977399 2020.1149629444303 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1146.8436708834190 2020.1149629303029
  path.lineTo(1146.8436279296875, 2020.1149902343750);
  // Segment 1146.8436708834190 2020.1149629303029 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1146.3696286678314 2020.5954928398132
  path.lineTo(1146.3696289062500, 2020.5954589843750);
  path.close();
  // start loop, contour: 9
  // Segment 1147.3323798179626 2019.6457400321960 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1147.8064220484741 2019.1652101374082
  path.moveTo(1147.3323974609375, 2019.6457519531250);
  path.lineTo(1147.8063964843750, 2019.1651611328125);
  // Segment 1147.8064220484741 2019.1652101374082 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1147.8064220383478 2019.1652101274185
  path.lineTo(1147.8063964843750, 2019.1651611328125);
  // Segment 1147.8064220383478 2019.1652101274185 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1147.3323798179626 2019.6457400321960
  path.lineTo(1147.3323974609375, 2019.6457519531250);
  path.close();
  // start loop, contour: 10
  // Segment 1145.3381097316742 2018.3533948063850 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1156.6848182678223 2018.3533948063850
  path.moveTo(1145.3381347656250, 2018.3533935546875);
  path.lineTo(1156.6848144531250, 2018.3533935546875);
  // Segment 1156.6848182678223 2018.3533948063850 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1156.6848182678223 2017.0033947825432
  path.lineTo(1156.6848144531250, 2017.0034179687500);
  // Segment 1156.6848182678223 2017.0033947825432 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1145.3381097316742 2017.0033947825432
  path.lineTo(1145.3381347656250, 2017.0034179687500);
  // Segment 1145.3381097316742 2017.0033947825432 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1145.3381097316742 2018.3533948063850
  path.lineTo(1145.3381347656250, 2018.3533935546875);
  path.close();
  // start loop, contour: 11
  // Segment 1156.6848182678223 2018.3533948063850 0.3569631313191 0.0000000000000 -0.2645167304388 0.2609454237780 1157.6574279406423 2017.9723661860094
  path.moveTo(1156.6848144531250, 2018.3533935546875);
  path.cubicTo(1157.0417480468750, 2018.3533935546875, 1157.3929443359375, 2018.2332763671875, 1157.6574707031250, 2017.9724121093750);
  // Segment 1157.6574279406423 2017.9723661860094 0.2653344079822 -0.2617520616521 0.0000000000000 0.3596905289350 1158.0474975705147 2017.0000000000000
  path.cubicTo(1157.9227294921875, 2017.7105712890625, 1158.0474853515625, 2017.3597412109375, 1158.0474853515625, 2017.0000000000000);
  // Segment 1158.0474975705147 2017.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1156.6974975466728 2017.0000000000000
  path.lineTo(1156.6975097656250, 2017.0000000000000);
  // Segment 1156.6974975466728 2017.0000000000000 0.0028009248351 0.0403311981485 0.0118595244351 -0.0220843520393 1156.6941780622435 2017.0325257649940
  path.cubicTo(1156.7003173828125, 2017.0402832031250, 1156.7060546875000, 2017.0104980468750, 1156.6942138671875, 2017.0324707031250);
  // Segment 1156.6941780622435 2017.0325257649940 -0.0032637855860 0.0184860248562 0.0120617528380 -0.0065934603083 1156.7093435710913 2017.0113063061967
  path.cubicTo(1156.6909179687500, 2017.0510253906250, 1156.7214355468750, 2017.0047607421875, 1156.7093505859375, 2017.0113525390625);
  // split at 0.4496445953846
  // path.cubicTo(1156.6927490234375, 2017.0407714843750, 1156.6981201171875, 2017.0360107421875, 1156.7033691406250, 2017.0289306640625);
  // path.cubicTo(1156.7097167968750, 2017.0201416015625, 1156.7159423828125, 2017.0076904296875, 1156.7093505859375, 2017.0113525390625);
  // Segment 1156.7093435710913 2017.0113063061967 -0.0070717276929 0.0122220954353 0.0203483811973 -0.0039136894418 1156.7268834554304 2016.9985353221975
  path.cubicTo(1156.7022705078125, 2017.0235595703125, 1156.7471923828125, 2016.9946289062500, 1156.7269287109375, 2016.9985351562500);
  // Segment 1156.7268834554304 2016.9985353221975 -0.0244396787691 0.0123649140586 0.0433322464027 0.0026558844666 1156.6848182678223 2017.0033947825432
  path.cubicTo(1156.7023925781250, 2017.0108642578125, 1156.7281494140625, 2017.0061035156250, 1156.6848144531250, 2017.0034179687500);
  // split at 0.4418420493603
  // path.cubicTo(1156.7160644531250, 2017.0040283203125, 1156.7150878906250, 2017.0061035156250, 1156.7136230468750, 2017.0065917968750);
  // path.cubicTo(1156.7116699218750, 2017.0070800781250, 1156.7089843750000, 2017.0048828125000, 1156.6848144531250, 2017.0034179687500);
  // Segment 1156.6848182678223 2017.0033947825432 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1156.6848182678223 2018.3533948063850
  path.lineTo(1156.6848144531250, 2018.3533935546875);
  path.close();
  // start loop, contour: 12
  // Segment 1158.0474975705147 2017.0000000000000 0.0000000000000 -0.3596905289350 0.2653344079822 0.2617520616521 1157.6574279406423 2016.0276338139906
  path.moveTo(1158.0474853515625, 2017.0000000000000);
  path.cubicTo(1158.0474853515625, 2016.6402587890625, 1157.9227294921875, 2016.2894287109375, 1157.6574707031250, 2016.0275878906250);
  // Segment 1157.6574279406423 2016.0276338139906 -0.2645167304388 -0.2609454237780 0.3569631313191 0.0000000000000 1156.6848182678223 2015.6466051936150
  path.cubicTo(1157.3929443359375, 2015.7667236328125, 1157.0417480468750, 2015.6466064453125, 1156.6848144531250, 2015.6466064453125);
  // split at 0.5481675863266
  // path.cubicTo(1157.5124511718750, 2015.8846435546875, 1157.3414306640625, 2015.7839355468750, 1157.1577148437500, 2015.7220458984375);
  // path.cubicTo(1157.0062255859375, 2015.6711425781250, 1156.8460693359375, 2015.6466064453125, 1156.6848144531250, 2015.6466064453125);
  // Segment 1156.6848182678223 2015.6466051936150 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1156.6848182678223 2016.9966052174568
  path.lineTo(1156.6848144531250, 2016.9965820312500);
  // Segment 1156.6848182678223 2016.9966052174568 0.0433322464027 -0.0026558844666 -0.0244396787691 -0.0123649140586 1156.7268834554304 2017.0014646778025
  path.cubicTo(1156.7281494140625, 2016.9938964843750, 1156.7023925781250, 2016.9891357421875, 1156.7269287109375, 2017.0014648437500);
  // split at 0.5581579208374
  // path.cubicTo(1156.7089843750000, 2016.9951171875000, 1156.7116699218750, 2016.9929199218750, 1156.7136230468750, 2016.9934082031250);
  // path.cubicTo(1156.7150878906250, 2016.9938964843750, 1156.7160644531250, 2016.9959716796875, 1156.7269287109375, 2017.0014648437500);
  // Segment 1156.7268834554304 2017.0014646778025 0.0203483811973 0.0039136894418 -0.0070717276929 -0.0122220954353 1156.7093435710913 2016.9886936938033
  path.cubicTo(1156.7471923828125, 2017.0053710937500, 1156.7022705078125, 2016.9764404296875, 1156.7093505859375, 2016.9886474609375);
  // Segment 1156.7093435710913 2016.9886936938033 0.0120617528380 0.0065934603083 -0.0032637855860 -0.0184860248562 1156.6941780622435 2016.9674742350060
  path.cubicTo(1156.7214355468750, 2016.9952392578125, 1156.6909179687500, 2016.9489746093750, 1156.6942138671875, 2016.9675292968750);
  // Segment 1156.6941780622435 2016.9674742350060 0.0118595244351 0.0220843520393 0.0028009248351 -0.0403311981485 1156.6974975466728 2017.0000000000000
  path.cubicTo(1156.7060546875000, 2016.9895019531250, 1156.7003173828125, 2016.9597167968750, 1156.6975097656250, 2017.0000000000000);
  // split at 0.4572408795357
  // path.cubicTo(1156.6995849609375, 2016.9775390625000, 1156.7014160156250, 2016.9768066406250, 1156.7014160156250, 2016.9768066406250);
  // path.cubicTo(1156.7014160156250, 2016.9769287109375, 1156.6989746093750, 2016.9781494140625, 1156.6975097656250, 2017.0000000000000);
  // Segment 1156.6974975466728 2017.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1158.0474975705147 2017.0000000000000
  path.lineTo(1158.0474853515625, 2017.0000000000000);
  path.close();
  // start loop, contour: 13
  // Segment 1156.6848182678223 2015.6466051936150 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1145.3381097316742 2015.6466051936150
  path.moveTo(1156.6848144531250, 2015.6466064453125);
  path.lineTo(1145.3381347656250, 2015.6466064453125);
  // Segment 1145.3381097316742 2015.6466051936150 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1145.3381097316742 2016.9966052174568
  path.lineTo(1145.3381347656250, 2016.9965820312500);
  // Segment 1145.3381097316742 2016.9966052174568 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1156.6848182678223 2016.9966052174568
  path.lineTo(1156.6848144531250, 2016.9965820312500);
  // Segment 1156.6848182678223 2016.9966052174568 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1156.6848182678223 2015.6466051936150
  path.lineTo(1156.6848144531250, 2015.6466064453125);
  path.close();
  // start loop, contour: 14
  // Segment 1145.8121519375022 2016.8021351246741 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1147.8064220237907 2014.8347900061515
  path.moveTo(1145.8121337890625, 2016.8021240234375);
  path.lineTo(1147.8063964843750, 2014.8348388671875);
  // Segment 1147.8064220237907 2014.8347900061515 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1146.8583376121346 2013.8737301678750
  path.lineTo(1146.8583984375000, 2013.8737792968750);
  // Segment 1146.8583376121346 2013.8737301678750 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1144.8640675258462 2015.8410752863977
  path.lineTo(1144.8640136718750, 2015.8410644531250);
  // Segment 1144.8640675258462 2015.8410752863977 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1145.8121519375022 2016.8021351246741
  path.lineTo(1145.8121337890625, 2016.8021240234375);
  path.close();
  // start loop, contour: 15
  // Segment 1147.8064220516883 2014.8347899786306 0.5430154146087 -0.5356841365729 0.5430154146087 0.5356841365729 1147.8064220516883 2012.9239773430752
  path.moveTo(1147.8063964843750, 2014.8348388671875);
  path.cubicTo(1148.3494873046875, 2014.2990722656250, 1148.3494873046875, 2013.4597167968750, 1147.8063964843750, 2012.9239501953125);
  // Segment 1147.8064220516883 2012.9239773430752 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1146.8583375842370 2013.8850371263100
  path.lineTo(1146.8583984375000, 2013.8850097656250);
  // Segment 1146.8583375842370 2013.8850371263100 0.0071280060876 0.0070317705240 0.0071280060876 -0.0070317705240 1146.8583375842370 2013.8737301953959
  path.cubicTo(1146.8654785156250, 2013.8920898437500, 1146.8654785156250, 2013.8666992187500, 1146.8583984375000, 2013.8737792968750);
  // Segment 1146.8583375842370 2013.8737301953959 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1147.8064220516883 2014.8347899786306
  path.lineTo(1147.8063964843750, 2014.8348388671875);
  path.close();
  // start loop, contour: 16
  // Segment 1147.8064220516883 2012.9239773430752 -0.5379138488298 -0.5306514472866 0.5379138488298 -0.5306514472866 1145.8955864341058 2012.9239773430752
  path.moveTo(1147.8063964843750, 2012.9239501953125);
  path.cubicTo(1147.2685546875000, 2012.3933105468750, 1146.4334716796875, 2012.3933105468750, 1145.8956298828125, 2012.9239501953125);
  // Segment 1145.8955864341058 2012.9239773430752 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1146.8436709015571 2013.8850371263100
  path.lineTo(1146.8436279296875, 2013.8850097656250);
  // Segment 1146.8436709015571 2013.8850371263100 0.0122295718664 -0.0120644598103 -0.0122295718664 -0.0120644598103 1146.8583375842370 2013.8850371263100
  path.cubicTo(1146.8559570312500, 2013.8729248046875, 1146.8460693359375, 2013.8729248046875, 1146.8583984375000, 2013.8850097656250);
  // Segment 1146.8583375842370 2013.8850371263100 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1147.8064220516883 2012.9239773430752
  path.lineTo(1147.8063964843750, 2012.9239501953125);
  path.close();
  // start loop, contour: 17
  // Segment 1145.8955864579798 2012.9239773195236 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1142.7322615803600 2016.0445936810224
  path.moveTo(1145.8956298828125, 2012.9239501953125);
  path.lineTo(1142.7322998046875, 2016.0445556640625);
  // Segment 1142.7322615803600 2016.0445936810224 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1143.6803460000633 2017.0056535113604
  path.lineTo(1143.6802978515625, 2017.0056152343750);
  // Segment 1143.6803460000633 2017.0056535113604 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1146.8436708776831 2013.8850371498615
  path.lineTo(1146.8436279296875, 2013.8850097656250);
  // Segment 1146.8436708776831 2013.8850371498615 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1145.8955864579798 2012.9239773195236
  path.lineTo(1145.8956298828125, 2012.9239501953125);
  path.close();
  // start loop, contour: 18
  // Segment 1142.7322615564860 2016.0445937045740 -0.0343838913237 0.0339196727021 0.0561572931720 -0.0710493024751 1142.5744069596683 2016.2183613784646
  path.moveTo(1142.7322998046875, 2016.0445556640625);
  path.cubicTo(1142.6978759765625, 2016.0784912109375, 1142.6306152343750, 2016.1473388671875, 1142.5744628906250, 2016.2183837890625);
  // Segment 1142.5744069596683 2016.2183613784646 -0.0547779032556 0.0720510806539 0.0000000000000 -0.2570904015602 1142.3937679156661 2016.7286419868469
  path.cubicTo(1142.5196533203125, 2016.2904052734375, 1142.3937988281250, 2016.4715576171875, 1142.3937988281250, 2016.7286376953125);
  // Segment 1142.3937679156661 2016.7286419868469 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1143.7437679395080 2016.7286419868469
  path.lineTo(1143.7437744140625, 2016.7286376953125);
  // Segment 1143.7437679395080 2016.7286419868469 -0.0051909534315 0.0665915567290 0.0133980913650 -0.0361675066532 1143.6976291086639 2016.9514128270803
  path.cubicTo(1143.7385253906250, 2016.7952880859375, 1143.7110595703125, 2016.9152832031250, 1143.6976318359375, 2016.9514160156250);
  // Segment 1143.6976291086639 2016.9514128270803 -0.0142876819622 0.0277028472317 0.0040377216094 -0.0063254385208 1143.6490888124401 2017.0354042045738
  path.cubicTo(1143.6833496093750, 2016.9791259765625, 1143.6530761718750, 2017.0290527343750, 1143.6490478515625, 2017.0354003906250);
  // Segment 1143.6490888124401 2017.0354042045738 -0.0045813437564 0.0032098513409 -0.0343840362634 0.0339198156850 1143.6803460239373 2017.0056534878088
  path.cubicTo(1143.6445312500000, 2017.0385742187500, 1143.6459960937500, 2017.0395507812500, 1143.6802978515625, 2017.0056152343750);
  // Segment 1143.6803460239373 2017.0056534878088 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1142.7322615564860 2016.0445937045740
  path.lineTo(1142.7322998046875, 2016.0445556640625);
  path.close();
  // start loop, contour: 19
  // Segment 1142.5947256938614 2016.2481120952295 -0.1857487117715 0.1832409092043 0.0167379373694 -0.0990717748979 1142.3430278987244 2016.7518748698508
  path.moveTo(1142.5947265625000, 2016.2481689453125);
  path.cubicTo(1142.4089355468750, 2016.4313964843750, 1142.3597412109375, 2016.6528320312500, 1142.3430175781250, 2016.7518310546875);
  // Segment 1142.3430278987244 2016.7518748698508 -0.0156657977007 0.1069052535795 0.0000000000000 -0.0339197441936 1142.3249999880791 2017.0000000000000
  path.cubicTo(1142.3273925781250, 2016.8587646484375, 1142.3249511718750, 2016.9660644531250, 1142.3249511718750, 2017.0000000000000);
  // Segment 1142.3249999880791 2017.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1143.6750000119209 2017.0000000000000
  path.lineTo(1143.6750488281250, 2017.0000000000000);
  // Segment 1143.6750000119209 2017.0000000000000 0.0000000000000 -0.0339197441936 -0.0015261841961 -0.0051459911965 1143.6741640831724 2016.9767671169961
  path.cubicTo(1143.6750488281250, 2016.9660644531250, 1143.6726074218750, 2016.9716796875000, 1143.6741943359375, 2016.9768066406250);
  // Segment 1143.6741640831724 2016.9767671169961 -0.0007886982052 0.0013596649622 0.0074114058388 -0.0224954551713 1143.6525251830094 2017.0486861571169
  path.cubicTo(1143.6733398437500, 2016.9781494140625, 1143.6599121093750, 2017.0262451171875, 1143.6524658203125, 2017.0487060546875);
  // split at 0.4203657805920
  // path.cubicTo(1143.6738281250000, 2016.9774169921875, 1143.6712646484375, 2016.9862060546875, 1143.6678466796875, 2016.9979248046875);
  // path.cubicTo(1143.6630859375000, 2017.0140380859375, 1143.6567382812500, 2017.0356445312500, 1143.6524658203125, 2017.0487060546875);
  // Segment 1143.6525251830094 2017.0486861571169 -0.0119644334077 0.0236755853369 0.0381324473830 -0.0447670202574 1143.5428101613127 2017.2091718784643
  path.cubicTo(1143.6405029296875, 2017.0723876953125, 1143.5809326171875, 2017.1644287109375, 1143.5428466796875, 2017.2092285156250);
  // Segment 1143.5428101613127 2017.2091718784643 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1142.5947256938614 2016.2481120952295
  path.lineTo(1142.5947265625000, 2016.2481689453125);
  path.close();
  // start loop, contour: 20
  // Segment 1142.3249999880791 2017.0000000000000 0.0000000000000 0.0339197441936 -0.0156657977007 -0.1069052535795 1142.3430278987244 2017.2481251301492
  path.moveTo(1142.3249511718750, 2017.0000000000000);
  path.cubicTo(1142.3249511718750, 2017.0339355468750, 1142.3273925781250, 2017.1412353515625, 1142.3430175781250, 2017.2481689453125);
  // Segment 1142.3430278987244 2017.2481251301492 0.0167379373694 0.0990717748979 -0.1857487117715 -0.1832409092043 1142.5947256938614 2017.7518879047705
  path.cubicTo(1142.3597412109375, 2017.3471679687500, 1142.4089355468750, 2017.5686035156250, 1142.5947265625000, 2017.7518310546875);
  // split at 0.4008532166481
  // path.cubicTo(1142.3497314453125, 2017.2878417968750, 1142.3616943359375, 2017.3471679687500, 1142.3854980468750, 2017.4158935546875);
  // path.cubicTo(1142.4211425781250, 2017.5185546875000, 1142.4833984375000, 2017.6420898437500, 1142.5947265625000, 2017.7518310546875);
  // Segment 1142.5947256938614 2017.7518879047705 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1143.5428101613127 2016.7908281215357
  path.lineTo(1143.5428466796875, 2016.7907714843750);
  // Segment 1143.5428101613127 2016.7908281215357 0.0381324473830 0.0447670202574 -0.0119644334077 -0.0236755853369 1143.6525251830094 2016.9513138428831
  path.cubicTo(1143.5809326171875, 2016.8355712890625, 1143.6405029296875, 2016.9276123046875, 1143.6524658203125, 2016.9512939453125);
  // Segment 1143.6525251830094 2016.9513138428831 0.0074114058388 0.0224954551713 -0.0007886982052 -0.0013596649622 1143.6741640831724 2017.0232328830039
  path.cubicTo(1143.6599121093750, 2016.9737548828125, 1143.6733398437500, 2017.0218505859375, 1143.6741943359375, 2017.0231933593750);
  // Segment 1143.6741640831724 2017.0232328830039 -0.0015261841961 0.0051459911965 0.0000000000000 0.0339197441936 1143.6750000119209 2017.0000000000000
  path.cubicTo(1143.6726074218750, 2017.0283203125000, 1143.6750488281250, 2017.0339355468750, 1143.6750488281250, 2017.0000000000000);
  // Segment 1143.6750000119209 2017.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1142.3249999880791 2017.0000000000000
  path.lineTo(1142.3249511718750, 2017.0000000000000);
  path.close();
  // start loop, contour: 21
  // Segment 1142.5947256938614 2017.7518879047705 -0.0799271403989 -0.1522613934208 -0.2174629955730 -0.2879403701950 1142.7322615564860 2017.9554062954260
  path.moveTo(1142.5947265625000, 2017.7518310546875);
  path.cubicTo(1142.5147705078125, 2017.5996093750000, 1142.5147705078125, 2017.6674804687500, 1142.7322998046875, 2017.9554443359375);
  // Segment 1142.7322615564860 2017.9554062954260 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1143.6803460239373 2016.9943465121912
  path.lineTo(1143.6802978515625, 2016.9943847656250);
  // Segment 1143.6803460239373 2016.9943465121912 0.0799271403989 0.1522613934208 0.2174629955730 0.2879403701950 1143.5428101613127 2016.7908281215357
  path.cubicTo(1143.7602539062500, 2017.1466064453125, 1143.7602539062500, 2017.0787353515625, 1143.5428466796875, 2016.7907714843750);
  // Segment 1143.5428101613127 2016.7908281215357 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1142.5947256938614 2017.7518879047705
  path.lineTo(1142.5947265625000, 2017.7518310546875);
  path.close();
  // start loop, contour: 22
  // Segment 1142.7322615746241 2017.9554063133189 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1145.8955864522438 2021.0760227493236
  path.moveTo(1142.7322998046875, 2017.9554443359375);
  path.lineTo(1145.8956298828125, 2021.0760498046875);
  // Segment 1145.8955864522438 2021.0760227493236 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1146.8436708834190 2020.1149629303029
  path.lineTo(1146.8436279296875, 2020.1149902343750);
  // Segment 1146.8436708834190 2020.1149629303029 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1143.6803460057993 2016.9943464942983
  path.lineTo(1143.6802978515625, 2016.9943847656250);
  // Segment 1143.6803460057993 2016.9943464942983 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1142.7322615746241 2017.9554063133189
  path.lineTo(1142.7322998046875, 2017.9554443359375);
  path.close();
  // start loop, contour: 23
  // Segment 1145.8955864341058 2021.0760227314306 0.2730164534637 0.2693304447891 -0.3016608168437 0.0000000000000 1146.8510041236877 2021.4740112423897
  path.moveTo(1145.8956298828125, 2021.0760498046875);
  path.cubicTo(1146.1685791015625, 2021.3453369140625, 1146.5493164062500, 2021.4739990234375, 1146.8509521484375, 2021.4739990234375);
  // Segment 1146.8510041236877 2021.4740112423897 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1146.8510041236877 2020.1240112185478
  path.lineTo(1146.8509521484375, 2020.1240234375000);
  // Segment 1146.8510041236877 2020.1240112185478 -0.0031276099109 0.0031991747760 0.0281856144058 0.0140930868099 1146.8580791488898 2020.1202473991566
  path.cubicTo(1146.8479003906250, 2020.1271972656250, 1146.8862304687500, 2020.1343994140625, 1146.8580322265625, 2020.1202392578125);
  // split at 0.3845077157021
  // path.cubicTo(1146.8497314453125, 2020.1252441406250, 1146.8547363281250, 2020.1270751953125, 1146.8596191406250, 2020.1280517578125);
  // path.cubicTo(1146.8675537109375, 2020.1296386718750, 1146.8753662109375, 2020.1289062500000, 1146.8580322265625, 2020.1202392578125);
  // Segment 1146.8580791488898 2020.1202473991566 -0.0369995545027 -0.0123195805663 0.0067223483810 0.0136883790721 1146.8436709015571 2020.1149629481959
  path.cubicTo(1146.8210449218750, 2020.1079101562500, 1146.8503417968750, 2020.1286621093750, 1146.8436279296875, 2020.1149902343750);
  // Segment 1146.8436709015571 2020.1149629481959 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1145.8955864341058 2021.0760227314306
  path.lineTo(1145.8956298828125, 2021.0760498046875);
  path.close();
  // start loop, contour: 24
  // Segment 1146.8510041236877 2021.4740112423897 0.3016605789999 0.0000000000000 -0.2730166120260 0.2693306012106 1147.8064220516883 2021.0760227314306
  path.moveTo(1146.8509521484375, 2021.4739990234375);
  path.cubicTo(1147.1527099609375, 2021.4739990234375, 1147.5334472656250, 2021.3453369140625, 1147.8063964843750, 2021.0760498046875);
  // Segment 1147.8064220516883 2021.0760227314306 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1146.8583375842370 2020.1149629481959
  path.lineTo(1146.8583984375000, 2020.1149902343750);
  // Segment 1146.8583375842370 2020.1149629481959 -0.0067222671256 0.0136883164611 0.0369996293611 -0.0123196021258 1146.8439293663473 2020.1202473404985
  path.cubicTo(1146.8515625000000, 2020.1286621093750, 1146.8809814453125, 2020.1079101562500, 1146.8438720703125, 2020.1202392578125);
  // Segment 1146.8439293663473 2020.1202473404985 -0.0281857033438 0.0140931104690 0.0031276541428 0.0031991704542 1146.8510041236877 2020.1240112185478
  path.cubicTo(1146.8157958984375, 2020.1343994140625, 1146.8541259765625, 2020.1271972656250, 1146.8509521484375, 2020.1240234375000);
  // Segment 1146.8510041236877 2020.1240112185478 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1146.8510041236877 2021.4740112423897
  path.lineTo(1146.8509521484375, 2021.4739990234375);
  path.close();
  // start loop, contour: 25
  // Segment 1147.8064220516883 2021.0760227314306 0.5430154146087 -0.5356841365729 0.5430154146087 0.5356841365729 1147.8064220516883 2019.1652101405787
  path.moveTo(1147.8063964843750, 2021.0760498046875);
  path.cubicTo(1148.3494873046875, 2020.5402832031250, 1148.3494873046875, 2019.7009277343750, 1147.8063964843750, 2019.1651611328125);
  // Segment 1147.8064220516883 2019.1652101405787 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1146.8583375842370 2020.1262699238134
  path.lineTo(1146.8583984375000, 2020.1262207031250);
  // Segment 1146.8583375842370 2020.1262699238134 0.0071280060876 0.0070317705240 0.0071280060876 -0.0070317705240 1146.8583375842370 2020.1149629481959
  path.cubicTo(1146.8654785156250, 2020.1333007812500, 1146.8654785156250, 2020.1079101562500, 1146.8583984375000, 2020.1149902343750);
  // Segment 1146.8583375842370 2020.1149629481959 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1147.8064220516883 2021.0760227314306
  path.lineTo(1147.8063964843750, 2021.0760498046875);
  path.close();
  // start loop, contour: 26
  // Segment 1147.8064220383478 2019.1652101274185 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1145.8121519520594 2017.1978648896866
  path.moveTo(1147.8063964843750, 2019.1651611328125);
  path.lineTo(1145.8121337890625, 2017.1978759765625);
  // Segment 1145.8121519520594 2017.1978648896866 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1144.8640675112890 2018.1589246992417
  path.lineTo(1144.8640136718750, 2018.1589355468750);
  // Segment 1144.8640675112890 2018.1589246992417 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1146.8583375975775 2020.1262699369736
  path.lineTo(1146.8583984375000, 2020.1262207031250);
  // Segment 1146.8583375975775 2020.1262699369736 0.0000000000000 0.0000000000000 0.0000000000000 0.0000000000000 1147.8064220383478 2019.1652101274185
  path.lineTo(1147.8063964843750, 2019.1651611328125);
  path.close();

testSimplify(reporter, path, filename);
}

static void testQuads66(skiatest::Reporter* reporter,const char* filename) {
    SkPath path;
    path.moveTo(2, 0);
    path.quadTo(3, 1, 2, 2);
    path.lineTo(2, 3);
    path.close();
    path.moveTo(2, 1);
    path.lineTo(2, 1);
    path.quadTo(1, 2, 2, 2);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuads67(skiatest::Reporter* reporter,const char* filename) {
    SkPath path;
    path.moveTo(3, 2);
    path.quadTo(1, 3, 3, 3);
    path.lineTo(3, 3);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.quadTo(2, 3, 3, 3);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuads68(skiatest::Reporter* reporter,const char* filename) {
    SkPath path;
    path.moveTo(1, 2);
    path.quadTo(0, 3, 2, 3);
    path.lineTo(2, 3);
    path.close();
    path.moveTo(1, 0);
    path.lineTo(0, 1);
    path.quadTo(1, 3, 2, 3);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuads69(skiatest::Reporter* reporter,const char* filename) {
    SkPath path;
    path.moveTo(1, 0);
    path.quadTo(2, 2, 2, 3);
    path.lineTo(2, 3);
    path.close();
    path.moveTo(1, 0);
    path.lineTo(1, 0);
    path.quadTo(3, 0, 1, 3);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuads70(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(1, 1);
    path.quadTo(2, 3, 3, 3);
    path.lineTo(3, 3);
    path.close();
    path.moveTo(2, 0);
    path.lineTo(2, 2);
    path.quadTo(1, 3, 3, 3);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuads71(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(1, 1);
    path.quadTo(2, 3, 3, 3);
    path.lineTo(3, 3);
    path.close();
    path.moveTo(3, 0);
    path.lineTo(2, 2);
    path.quadTo(1, 3, 3, 3);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuads72(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(1, 1);
    path.quadTo(2, 3, 3, 3);
    path.lineTo(3, 3);
    path.close();
    path.moveTo(0, 1);
    path.lineTo(2, 2);
    path.quadTo(1, 3, 3, 3);
    path.close();
    testSimplify(reporter, path, filename);
}

static void testQuads73(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(0, 1, 1, 2);
    path.lineTo(0, 3);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(0, 1, 1, 1);
    path.close();
    testSimplify(reporter, path, filename);
}

static void bug5169(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x4281c71c));  // 0, 64.8889f
path.cubicTo(SkBits2Float(0x434e0000), SkBits2Float(0x4281c71c), SkBits2Float(0x00000000), SkBits2Float(0xc2a238e4), SkBits2Float(0x00000000), SkBits2Float(0x4281c71c));  // 206, 64.8889f, 0, -81.1111f, 0, 64.8889f
path.moveTo(SkBits2Float(0x43300000), SkBits2Float(0x41971c72));  // 176, 18.8889f
path.cubicTo(SkBits2Float(0xc29e0000), SkBits2Float(0xc25c71c7), SkBits2Float(0x42b20000), SkBits2Float(0x42fbc71c), SkBits2Float(0x43300000), SkBits2Float(0x41971c72));  // -79, -55.1111f, 89, 125.889f, 176, 18.8889f
    testSimplify(reporter, path, filename);
}

static void tiger8_393(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPathFillType) 0);
path.moveTo(SkBits2Float(0x42b93333), SkBits2Float(0x43d5a666));  // 92.6f, 427.3f
path.cubicTo(SkBits2Float(0x42b93333), SkBits2Float(0x43d5a666), SkBits2Float(0x42b5cccd), SkBits2Float(0x43da1999), SkBits2Float(0x42b80000), SkBits2Float(0x43ddf333));  // 92.6f, 427.3f, 90.9f, 436.2f, 92, 443.9f
path.cubicTo(SkBits2Float(0x42b80000), SkBits2Float(0x43ddf333), SkBits2Float(0x42b30000), SkBits2Float(0x43e17333), SkBits2Float(0x42cf999a), SkBits2Float(0x43e1b333));  // 92, 443.9f, 89.5f, 450.9f, 103.8f, 451.4f
path.cubicTo(SkBits2Float(0x42ec3334), SkBits2Float(0x43e14ccd), SkBits2Float(0x42e73334), SkBits2Float(0x43ddf333), SkBits2Float(0x42e73334), SkBits2Float(0x43ddf333));  // 118.1f, 450.6f, 115.6f, 443.9f, 115.6f, 443.9f
path.cubicTo(SkBits2Float(0x42e7999a), SkBits2Float(0x43de8000), SkBits2Float(0x42ea6667), SkBits2Float(0x43db4000), SkBits2Float(0x42e60001), SkBits2Float(0x43d5a666));  // 115.8f, 445, 117.2f, 438.5f, 115, 427.3f
    testSimplify(reporter, path, filename);
}

// triggers angle assert from distance field code
static void carsvg_1(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPathFillType) 0);
path.moveTo(SkBits2Float(0x4393d61e), SkBits2Float(0x43e768f9));  // 295.673f, 462.82f
path.cubicTo(SkBits2Float(0x4396b50e), SkBits2Float(0x43e63c20), SkBits2Float(0x43998931), SkBits2Float(0x43e6c43e), SkBits2Float(0x439cb6a8), SkBits2Float(0x43e70ef9));  // 301.414f, 460.47f, 307.072f, 461.533f, 313.427f, 462.117f
path.cubicTo(SkBits2Float(0x439dfc1e), SkBits2Float(0x43e72ce0), SkBits2Float(0x439a285c), SkBits2Float(0x43e717fb), SkBits2Float(0x4398e23c), SkBits2Float(0x43e7027c));  // 315.97f, 462.351f, 308.315f, 462.187f, 305.767f, 462.019f
path.cubicTo(SkBits2Float(0x4398136f), SkBits2Float(0x43e6f4db), SkBits2Float(0x439a7e14), SkBits2Float(0x43e6d390), SkBits2Float(0x439b4ba9), SkBits2Float(0x43e6b956));  // 304.152f, 461.913f, 308.985f, 461.653f, 310.591f, 461.448f
path.cubicTo(SkBits2Float(0x439c2b19), SkBits2Float(0x43e68603), SkBits2Float(0x43abf4df), SkBits2Float(0x43e9ca9e), SkBits2Float(0x43a1daea), SkBits2Float(0x43e912a5));  // 312.337f, 461.047f, 343.913f, 467.583f, 323.71f, 466.146f
path.cubicTo(SkBits2Float(0x43a4f45a), SkBits2Float(0x43e78baf), SkBits2Float(0x43a2a391), SkBits2Float(0x43e86a82), SkBits2Float(0x43a946bd), SkBits2Float(0x43e90c56));  // 329.909f, 463.091f, 325.278f, 464.832f, 338.553f, 466.096f
path.lineTo(SkBits2Float(0x43a4250b), SkBits2Float(0x43e998dc));  // 328.289f, 467.194f
path.cubicTo(SkBits2Float(0x43a8a9c8), SkBits2Float(0x43e8f06c), SkBits2Float(0x43a95cb5), SkBits2Float(0x43e84ea6), SkBits2Float(0x43a6f7c1), SkBits2Float(0x43e9bdb5));  // 337.326f, 465.878f, 338.724f, 464.614f, 333.936f, 467.482f
path.cubicTo(SkBits2Float(0x43a59ed0), SkBits2Float(0x43e9d2ca), SkBits2Float(0x4395ea4d), SkBits2Float(0x43e92afe), SkBits2Float(0x43a06569), SkBits2Float(0x43e7773d));  // 331.241f, 467.647f, 299.83f, 466.336f, 320.792f, 462.932f
path.cubicTo(SkBits2Float(0x438bf0ff), SkBits2Float(0x43ea0fef), SkBits2Float(0x43a0e17a), SkBits2Float(0x43e5f41b), SkBits2Float(0x4398f3fb), SkBits2Float(0x43e804c8));  // 279.883f, 468.124f, 321.762f, 459.907f, 305.906f, 464.037f
path.lineTo(SkBits2Float(0x4393d61e), SkBits2Float(0x43e768f9));  // 295.673f, 462.82f
path.close();

    testSimplify(reporter, path, filename);
}

static void simplifyTest_1(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPathFillType) 0);
path.moveTo(SkBits2Float(0x42bfefd4), SkBits2Float(0x42ef80ef));  // 95.9684f, 119.752f
path.quadTo(SkBits2Float(0x42c26810), SkBits2Float(0x42e214b8), SkBits2Float(0x42cdcad5), SkBits2Float(0x42d82aa2));  // 97.2032f, 113.04f, 102.896f, 108.083f
path.lineTo(SkBits2Float(0x42cdcb21), SkBits2Float(0x42d82a61));  // 102.897f, 108.083f
path.quadTo(SkBits2Float(0x42d5e3c8), SkBits2Float(0x42d12140), SkBits2Float(0x42e20ee8), SkBits2Float(0x42cdc937));  // 106.945f, 104.565f, 113.029f, 102.893f
path.lineTo(SkBits2Float(0x42e256e3), SkBits2Float(0x42cdbc92));  // 113.17f, 102.868f
path.lineTo(SkBits2Float(0x42f5eadb), SkBits2Float(0x42cc2cb3));  // 122.959f, 102.087f
path.lineTo(SkBits2Float(0x42f746a6), SkBits2Float(0x42cccf85));  // 123.638f, 102.405f
path.quadTo(SkBits2Float(0x42fa586c), SkBits2Float(0x42d126c4), SkBits2Float(0x42f6c657), SkBits2Float(0x42d5d315));  // 125.173f, 104.576f, 123.387f, 106.912f
path.lineTo(SkBits2Float(0x42f591eb), SkBits2Float(0x42d4e76d));  // 122.785f, 106.452f
path.lineTo(SkBits2Float(0x42f6c6e0), SkBits2Float(0x42d5d261));  // 123.388f, 106.911f
path.quadTo(SkBits2Float(0x42f6bb33), SkBits2Float(0x42d5e1bb), SkBits2Float(0x42f6a3d8), SkBits2Float(0x42d6007c));  // 123.366f, 106.941f, 123.32f, 107.001f
path.quadTo(SkBits2Float(0x42ea3850), SkBits2Float(0x42e65af0), SkBits2Float(0x42d97a6e), SkBits2Float(0x42ed841c));  // 117.11f, 115.178f, 108.739f, 118.758f
path.lineTo(SkBits2Float(0x42d91d92), SkBits2Float(0x42ed9ec0));  // 108.558f, 118.81f
path.lineTo(SkBits2Float(0x42c1a959), SkBits2Float(0x42f146b0));  // 96.8308f, 120.638f
path.lineTo(SkBits2Float(0x42bfefd4), SkBits2Float(0x42ef80f0));  // 95.9684f, 119.752f
path.lineTo(SkBits2Float(0x42bfefd4), SkBits2Float(0x42ef80ef));  // 95.9684f, 119.752f
path.close();
path.moveTo(SkBits2Float(0x42c2eb4e), SkBits2Float(0x42f00d68));  // 97.4596f, 120.026f
path.lineTo(SkBits2Float(0x42c16d91), SkBits2Float(0x42efc72c));  // 96.714f, 119.889f
path.lineTo(SkBits2Float(0x42c131c9), SkBits2Float(0x42ee47a8));  // 96.5972f, 119.14f
path.lineTo(SkBits2Float(0x42d8a602), SkBits2Float(0x42ea9fb8));  // 108.324f, 117.312f
path.lineTo(SkBits2Float(0x42d8e1ca), SkBits2Float(0x42ec1f3c));  // 108.441f, 118.061f
path.lineTo(SkBits2Float(0x42d84926), SkBits2Float(0x42eaba5c));  // 108.143f, 117.364f
path.quadTo(SkBits2Float(0x42e84a40), SkBits2Float(0x42e3e1f0), SkBits2Float(0x42f439a2), SkBits2Float(0x42d42af8));  // 116.145f, 113.941f, 122.113f, 106.084f
path.quadTo(SkBits2Float(0x42f45121), SkBits2Float(0x42d40c08), SkBits2Float(0x42f45cf6), SkBits2Float(0x42d3fc79));  // 122.158f, 106.023f, 122.182f, 105.993f
path.lineTo(SkBits2Float(0x42f45d7f), SkBits2Float(0x42d3fbc5));  // 122.183f, 105.992f
path.quadTo(SkBits2Float(0x42f69510), SkBits2Float(0x42d114f4), SkBits2Float(0x42f4ccce), SkBits2Float(0x42ce8fb7));  // 123.291f, 104.541f, 122.4f, 103.281f
path.lineTo(SkBits2Float(0x42f609ba), SkBits2Float(0x42cdaf9e));  // 123.019f, 102.843f
path.lineTo(SkBits2Float(0x42f62899), SkBits2Float(0x42cf3289));  // 123.079f, 103.599f
path.lineTo(SkBits2Float(0x42e294a1), SkBits2Float(0x42d0c268));  // 113.29f, 104.38f
path.lineTo(SkBits2Float(0x42e275c2), SkBits2Float(0x42cf3f7d));  // 113.23f, 103.624f
path.lineTo(SkBits2Float(0x42e2dc9c), SkBits2Float(0x42d0b5c3));  // 113.431f, 104.355f
path.quadTo(SkBits2Float(0x42d75bb8), SkBits2Float(0x42d3df08), SkBits2Float(0x42cfc853), SkBits2Float(0x42da7457));  // 107.679f, 105.936f, 103.891f, 109.227f
path.lineTo(SkBits2Float(0x42cec9ba), SkBits2Float(0x42d94f5c));  // 103.394f, 108.655f
path.lineTo(SkBits2Float(0x42cfc89f), SkBits2Float(0x42da7416));  // 103.892f, 109.227f
path.quadTo(SkBits2Float(0x42c53268), SkBits2Float(0x42e3ac00), SkBits2Float(0x42c2eb4e), SkBits2Float(0x42f00d67));  // 98.5984f, 113.836f, 97.4596f, 120.026f
path.lineTo(SkBits2Float(0x42c2eb4e), SkBits2Float(0x42f00d68));  // 97.4596f, 120.026f
path.close();

    testSimplify(reporter, path, filename);
}

static void joel_1(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPathFillType) 0);
path.moveTo(144.859f, 285.172f);
path.lineTo(144.859f, 285.172f);
path.lineTo(144.859f, 285.172f);
path.lineTo(143.132f, 284.617f);
path.lineTo(144.859f, 285.172f);
path.close();
path.moveTo(135.922f, 286.844f);
path.lineTo(135.922f, 286.844f);
path.lineTo(135.922f, 286.844f);
path.lineTo(135.367f, 288.571f);
path.lineTo(135.922f, 286.844f);
path.close();
path.moveTo(135.922f, 286.844f);
path.cubicTo(137.07f, 287.219f, 138.242f, 287.086f, 139.242f, 286.578f);
path.cubicTo(140.234f, 286.078f, 141.031f, 285.203f, 141.406f, 284.055f);
path.lineTo(144.859f, 285.172f);
path.cubicTo(143.492f, 289.375f, 138.992f, 291.656f, 134.797f, 290.297f);
path.lineTo(135.922f, 286.844f);
path.close();
path.moveTo(129.68f, 280.242f);
path.lineTo(129.68f, 280.242f);
path.lineTo(129.68f, 280.242f);
path.lineTo(131.407f, 280.804f);
path.lineTo(129.68f, 280.242f);
path.close();
path.moveTo(133.133f, 281.367f);
path.cubicTo(132.758f, 282.508f, 132.883f, 283.687f, 133.391f, 284.679f);
path.cubicTo(133.907f, 285.679f, 134.774f, 286.468f, 135.922f, 286.843f);
path.lineTo(134.797f, 290.296f);
path.cubicTo(130.602f, 288.929f, 128.313f, 284.437f, 129.68f, 280.241f);
path.lineTo(133.133f, 281.367f);
path.close();
path.moveTo(139.742f, 275.117f);
path.lineTo(139.742f, 275.117f);
path.lineTo(139.18f, 276.844f);
path.lineTo(139.742f, 275.117f);
path.close();
path.moveTo(138.609f, 278.57f);
path.cubicTo(137.461f, 278.203f, 136.297f, 278.328f, 135.297f, 278.836f);
path.cubicTo(134.297f, 279.344f, 133.508f, 280.219f, 133.133f, 281.367f);
path.lineTo(129.68f, 280.242f);
path.cubicTo(131.047f, 276.039f, 135.539f, 273.758f, 139.742f, 275.117f);
path.lineTo(138.609f, 278.57f);
path.close();
path.moveTo(141.406f, 284.055f);
path.cubicTo(141.773f, 282.907f, 141.648f, 281.735f, 141.148f, 280.735f);
path.cubicTo(140.625f, 279.735f, 139.757f, 278.946f, 138.609f, 278.571f);
path.lineTo(139.742f, 275.118f);
path.cubicTo(143.937f, 276.493f, 146.219f, 280.977f, 144.859f, 285.173f);
path.lineTo(141.406f, 284.055f);
path.close();
    testSimplify(reporter, path, filename);
}

static void joel_2(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPathFillType) 0);

path.moveTo(403.283f, 497.197f);
path.cubicTo(403.424f, 497.244f, 391.111f, 495.556f, 391.111f, 495.556f);
path.lineTo(392.291f, 493.165f);
path.cubicTo(392.291f, 493.165f, 388.994f, 492.056f, 386.65f, 491.821f);
path.cubicTo(384.244f, 491.454f, 381.603f, 490.774f, 381.603f, 490.774f);
path.lineTo(383.392f, 488.383f);
path.cubicTo(383.392f, 488.383f, 379.119f, 487.453f, 378.939f, 485.695f);
path.cubicTo(378.791f, 483.57f, 383.064f, 485.25f, 384.877f, 485.843f);
path.lineTo(387.697f, 484.351f);
path.cubicTo(382.752f, 483.835f, 376.595f, 482.124f, 374.478f, 480.312f);
path.lineTo(356.22f, 496.304f);
path.lineTo(368.095f, 510.499f);
path.lineTo(373.884f, 510.202f);
path.lineTo(374.478f, 509.007f);
path.lineTo(370.916f, 506.913f);
path.lineTo(371.807f, 506.022f);
path.cubicTo(371.807f, 506.022f, 374.807f, 507.28f, 377.752f, 507.514f);
path.cubicTo(380.752f, 507.881f, 387.4f, 508.108f, 387.4f, 508.108f);
path.lineTo(388.884f, 506.764f);
path.cubicTo(388.884f, 506.764f, 378.345f, 504.998f, 378.345f, 504.819f);
path.lineTo(378.04f, 503.03f);
path.cubicTo(378.04f, 503.03f, 391.415f, 505.796f, 391.399f, 505.866f);
path.lineTo(386.063f, 502.132f);
path.lineTo(387.547f, 500.335f);
path.lineTo(398.375f, 501.976f);
path.lineTo(403.283f, 497.197f);
path.lineTo(403.283f, 497.197f);
path.close();
    testSimplify(reporter, path, filename);
}

static void joel_3(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPathFillType) 0);
path.moveTo(391.097f, 334.453f);
path.lineTo(390.761f, 334.617f);
path.lineTo(390.425f, 333.937f);
path.lineTo(390.761f, 333.765f);
path.lineTo(391.097f, 334.453f);
path.close();
path.moveTo(391.128f, 334.438f);
path.lineTo(390.808f, 334.633f);
path.lineTo(390.402f, 333.992f);
path.lineTo(390.73f, 333.781f);
path.lineTo(391.128f, 334.438f);
path.lineTo(391.128f, 334.438f);
path.close();
path.moveTo(455.073f, 302.219f);
path.lineTo(455.018f, 302.375f);
path.lineTo(454.87f, 302.453f);
path.lineTo(454.706f, 302.109f);
path.lineTo(455.073f, 302.219f);
path.close();
path.moveTo(454.87f, 302.453f);
path.lineTo(391.097f, 334.453f);
path.lineTo(390.761f, 333.765f);
path.lineTo(454.534f, 301.765f);
path.lineTo(454.87f, 302.453f);
path.close();
path.moveTo(456.245f, 296.867f);
path.lineTo(456.659f, 296.953f);
path.lineTo(456.526f, 297.351f);
path.lineTo(456.174f, 297.242f);
path.lineTo(456.245f, 296.867f);
path.lineTo(456.245f, 296.867f);
path.close();
path.moveTo(456.526f, 297.352f);
path.lineTo(455.073f, 302.219f);
path.lineTo(454.339f, 302);
path.lineTo(455.808f, 297.133f);
path.lineTo(456.526f, 297.352f);
path.lineTo(456.526f, 297.352f);
path.close();
path.moveTo(450.979f, 295.891f);
path.lineTo(451.112f, 295.813f);
path.lineTo(451.26f, 295.836f);
path.lineTo(451.19f, 296.211f);
path.lineTo(450.979f, 295.891f);
path.close();
path.moveTo(451.261f, 295.836f);
path.lineTo(456.245f, 296.867f);
path.lineTo(456.089f, 297.617f);
path.lineTo(451.105f, 296.586f);
path.lineTo(451.261f, 295.836f);
path.close();
path.moveTo(390.729f, 333.781f);
path.lineTo(450.979f, 295.89f);
path.lineTo(451.385f, 296.531f);
path.lineTo(391.127f, 334.437f);
path.lineTo(390.729f, 333.781f);
path.close();
    testSimplify(reporter, path, filename);
}

static void joel_4(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPathFillType) 0);
path.moveTo(SkBits2Float(0x4199d4fe), SkBits2Float(0x4265ac08));  // 19.229f, 57.418f
path.cubicTo(SkBits2Float(0x419be979), SkBits2Float(0x426574bc), SkBits2Float(0x419c2b02), SkBits2Float(0x42653c6a), SkBits2Float(0x419af5c3), SkBits2Float(0x42645f3b));  // 19.489f, 57.364f, 19.521f, 57.309f, 19.37f, 57.093f
path.cubicTo(SkBits2Float(0x419a1894), SkBits2Float(0x4263a3d7), SkBits2Float(0x4198cccd), SkBits2Float(0x4262f2b0), SkBits2Float(0x4197c290), SkBits2Float(0x4262374b));  // 19.262f, 56.91f, 19.1f, 56.737f, 18.97f, 56.554f
path.cubicTo(SkBits2Float(0x41960832), SkBits2Float(0x42610c49), SkBits2Float(0x41944dd4), SkBits2Float(0x425fd709), SkBits2Float(0x41927cee), SkBits2Float(0x425ea0c4));  // 18.754f, 56.262f, 18.538f, 55.96f, 18.311f, 55.657f
path.cubicTo(SkBits2Float(0x4191b646), SkBits2Float(0x425e1cab), SkBits2Float(0x418edd30), SkBits2Float(0x425ca4dd), SkBits2Float(0x418f4bc7), SkBits2Float(0x425bdd2e));  // 18.214f, 55.528f, 17.858f, 55.161f, 17.912f, 54.966f
path.lineTo(SkBits2Float(0x41903f7d), SkBits2Float(0x425b6e96));  // 18.031f, 54.858f
path.cubicTo(SkBits2Float(0x41921062), SkBits2Float(0x425aa6e8), SkBits2Float(0x4193872b), SkBits2Float(0x425bd1ea), SkBits2Float(0x41947ae1), SkBits2Float(0x425c77cd));  // 18.258f, 54.663f, 18.441f, 54.955f, 18.56f, 55.117f
path.cubicTo(SkBits2Float(0x4195dd2f), SkBits2Float(0x425d6b83), SkBits2Float(0x4197ae14), SkBits2Float(0x425e1caa), SkBits2Float(0x419924dd), SkBits2Float(0x425ef9d9));  // 18.733f, 55.355f, 18.96f, 55.528f, 19.143f, 55.744f
path.cubicTo(SkBits2Float(0x419a1893), SkBits2Float(0x425f9479), SkBits2Float(0x419adf3b), SkBits2Float(0x42601997), SkBits2Float(0x419bd2f1), SkBits2Float(0x42609db0));  // 19.262f, 55.895f, 19.359f, 56.025f, 19.478f, 56.154f
path.cubicTo(SkBits2Float(0x419c147a), SkBits2Float(0x4260c9b8), SkBits2Float(0x419c8312), SkBits2Float(0x4260e03f), SkBits2Float(0x419cb020), SkBits2Float(0x42610104));  // 19.51f, 56.197f, 19.564f, 56.219f, 19.586f, 56.251f
path.cubicTo(SkBits2Float(0x419d0830), SkBits2Float(0x42613850), SkBits2Float(0x419da3d6), SkBits2Float(0x4261bd6e), SkBits2Float(0x419e126e), SkBits2Float(0x4261d2f0));  // 19.629f, 56.305f, 19.705f, 56.435f, 19.759f, 56.456f
path.lineTo(SkBits2Float(0x419e28f5), SkBits2Float(0x4261d2f0));  // 19.77f, 56.456f
path.lineTo(SkBits2Float(0x419e28f5), SkBits2Float(0x4261f4bb));  // 19.77f, 56.489f
path.cubicTo(SkBits2Float(0x419e3d70), SkBits2Float(0x4261fef8), SkBits2Float(0x419e53f7), SkBits2Float(0x4261f4bb), SkBits2Float(0x419e8105), SkBits2Float(0x4261fef8));  // 19.78f, 56.499f, 19.791f, 56.489f, 19.813f, 56.499f
path.cubicTo(SkBits2Float(0x419eac07), SkBits2Float(0x426220c3), SkBits2Float(0x419eac07), SkBits2Float(0x42624187), SkBits2Float(0x419eef9d), SkBits2Float(0x4262580f));  // 19.834f, 56.532f, 19.834f, 56.564f, 19.867f, 56.586f
path.cubicTo(SkBits2Float(0x419fe353), SkBits2Float(0x4262f2af), SkBits2Float(0x41a0eb84), SkBits2Float(0x426377cd), SkBits2Float(0x41a1b22c), SkBits2Float(0x4263fbe6));  // 19.986f, 56.737f, 20.115f, 56.867f, 20.212f, 56.996f
path.cubicTo(SkBits2Float(0x41a20a3c), SkBits2Float(0x42641db1), SkBits2Float(0x41a2e76b), SkBits2Float(0x4264a1c9), SkBits2Float(0x41a34188), SkBits2Float(0x4264ad0d));  // 20.255f, 57.029f, 20.363f, 57.158f, 20.407f, 57.169f
path.cubicTo(SkBits2Float(0x41a36c8a), SkBits2Float(0x4264ad0d), SkBits2Float(0x41a3c6a7), SkBits2Float(0x4264a1c9), SkBits2Float(0x41a3f1a9), SkBits2Float(0x4264ad0d));  // 20.428f, 57.169f, 20.472f, 57.158f, 20.493f, 57.169f
path.cubicTo(SkBits2Float(0x41a3f1a9), SkBits2Float(0x42648c48), SkBits2Float(0x41a41eb7), SkBits2Float(0x42648105), SkBits2Float(0x41a449b9), SkBits2Float(0x426475c1));  // 20.493f, 57.137f, 20.515f, 57.126f, 20.536f, 57.115f
path.cubicTo(SkBits2Float(0x41a48d4f), SkBits2Float(0x4263f1a8), SkBits2Float(0x41a46040), SkBits2Float(0x42634082), SkBits2Float(0x41a48d4f), SkBits2Float(0x4262bb63));  // 20.569f, 56.986f, 20.547f, 56.813f, 20.569f, 56.683f
path.cubicTo(SkBits2Float(0x41a51061), SkBits2Float(0x426122d0), SkBits2Float(0x41a63126), SkBits2Float(0x425f51ea), SkBits2Float(0x41a82d0d), SkBits2Float(0x425e0624));  // 20.633f, 56.284f, 20.774f, 55.83f, 21.022f, 55.506f
path.cubicTo(SkBits2Float(0x41a90a3c), SkBits2Float(0x425d820b), SkBits2Float(0x41aab01f), SkBits2Float(0x425cba5d), SkBits2Float(0x41ab0830), SkBits2Float(0x425c147a));  // 21.13f, 55.377f, 21.336f, 55.182f, 21.379f, 55.02f
path.cubicTo(SkBits2Float(0x41aa147a), SkBits2Float(0x425bf3b5), SkBits2Float(0x41a8df3a), SkBits2Float(0x425c0936), SkBits2Float(0x41a7d4fd), SkBits2Float(0x425c147a));  // 21.26f, 54.988f, 21.109f, 55.009f, 20.979f, 55.02f
path.cubicTo(SkBits2Float(0x41a74fde), SkBits2Float(0x425c147a), SkBits2Float(0x41a65e34), SkBits2Float(0x425c4082), SkBits2Float(0x41a5c28e), SkBits2Float(0x425c4082));  // 20.914f, 55.02f, 20.796f, 55.063f, 20.72f, 55.063f
path.cubicTo(SkBits2Float(0x41a56a7e), SkBits2Float(0x425c353e), SkBits2Float(0x41a4fbe6), SkBits2Float(0x425c147a), SkBits2Float(0x41a4ced8), SkBits2Float(0x425c0936));  // 20.677f, 55.052f, 20.623f, 55.02f, 20.601f, 55.009f
path.cubicTo(SkBits2Float(0x41a53d70), SkBits2Float(0x425af4bb), SkBits2Float(0x41a5ed90), SkBits2Float(0x425abd6f), SkBits2Float(0x41a85a1c), SkBits2Float(0x425aa6e8));  // 20.655f, 54.739f, 20.741f, 54.685f, 21.044f, 54.663f
path.cubicTo(SkBits2Float(0x41a920c4), SkBits2Float(0x425a9cab), SkBits2Float(0x41a9d0e5), SkBits2Float(0x425aa6e8), SkBits2Float(0x41aa5603), SkBits2Float(0x425a9167));  // 21.141f, 54.653f, 21.227f, 54.663f, 21.292f, 54.642f
path.cubicTo(SkBits2Float(0x41aa8311), SkBits2Float(0x425a8623), SkBits2Float(0x41aa9999), SkBits2Float(0x425a655f), SkBits2Float(0x41aab020), SkBits2Float(0x425a655f));  // 21.314f, 54.631f, 21.325f, 54.599f, 21.336f, 54.599f
path.cubicTo(SkBits2Float(0x41aa3f7c), SkBits2Float(0x42599eb7), SkBits2Float(0x41a9a5e3), SkBits2Float(0x42591998), SkBits2Float(0x41a9374b), SkBits2Float(0x42586871));  // 21.281f, 54.405f, 21.206f, 54.275f, 21.152f, 54.102f
path.cubicTo(SkBits2Float(0x41a8c8b3), SkBits2Float(0x4257e458), SkBits2Float(0x41a8b22c), SkBits2Float(0x42575f3a), SkBits2Float(0x41a85a1c), SkBits2Float(0x4256c49a));  // 21.098f, 53.973f, 21.087f, 53.843f, 21.044f, 53.692f
path.cubicTo(SkBits2Float(0x41a76666), SkBits2Float(0x42551479), SkBits2Float(0x41a68937), SkBits2Float(0x4252cabf), SkBits2Float(0x41a74fdf), SkBits2Float(0x4250a1c9));  // 20.925f, 53.27f, 20.817f, 52.698f, 20.914f, 52.158f
path.cubicTo(SkBits2Float(0x41a77ced), SkBits2Float(0x42500729), SkBits2Float(0x41a870a4), SkBits2Float(0x424e8417), SkBits2Float(0x41a8b22d), SkBits2Float(0x424e4ccb));  // 20.936f, 52.007f, 21.055f, 51.629f, 21.087f, 51.575f
path.cubicTo(SkBits2Float(0x41a8b22d), SkBits2Float(0x424e4187), SkBits2Float(0x41aa147b), SkBits2Float(0x424cc9b9), SkBits2Float(0x41aab021), SkBits2Float(0x424c2f19));  // 21.087f, 51.564f, 21.26f, 51.197f, 21.336f, 51.046f
path.cubicTo(SkBits2Float(0x41aac49c), SkBits2Float(0x424c1892), SkBits2Float(0x41ab49bb), SkBits2Float(0x424b9eb7), SkBits2Float(0x41ab8b44), SkBits2Float(0x424b676b));  // 21.346f, 51.024f, 21.411f, 50.905f, 21.443f, 50.851f
path.cubicTo(SkBits2Float(0x41ac3d71), SkBits2Float(0x424ab644), SkBits2Float(0x41ad45a2), SkBits2Float(0x424a26e8), SkBits2Float(0x41ae22d1), SkBits2Float(0x42498105));  // 21.53f, 50.678f, 21.659f, 50.538f, 21.767f, 50.376f
path.cubicTo(SkBits2Float(0x41ae6667), SkBits2Float(0x42496b84), SkBits2Float(0x41aeeb85), SkBits2Float(0x42491db1), SkBits2Float(0x41af0000), SkBits2Float(0x4248fbe6));  // 21.8f, 50.355f, 21.865f, 50.279f, 21.875f, 50.246f
path.cubicTo(SkBits2Float(0x41b0624e), SkBits2Float(0x4248353e), SkBits2Float(0x41b1db23), SkBits2Float(0x424779da), SkBits2Float(0x41b353f8), SkBits2Float(0x4246bd6f));  // 22.048f, 50.052f, 22.232f, 49.869f, 22.416f, 49.685f
path.cubicTo(SkBits2Float(0x41b3c083), SkBits2Float(0x42468623), SkBits2Float(0x41b445a2), SkBits2Float(0x42464ed7), SkBits2Float(0x41b4cac1), SkBits2Float(0x4246178c));  // 22.469f, 49.631f, 22.534f, 49.577f, 22.599f, 49.523f
path.cubicTo(SkBits2Float(0x41b56667), SkBits2Float(0x4245c9b9), SkBits2Float(0x41b62d0f), SkBits2Float(0x4245872a), SkBits2Float(0x41b6c8b5), SkBits2Float(0x4245449a));  // 22.675f, 49.447f, 22.772f, 49.382f, 22.848f, 49.317f
path.cubicTo(SkBits2Float(0x41b7624f), SkBits2Float(0x42450311), SkBits2Float(0x41b7e76d), SkBits2Float(0x4244a9fa), SkBits2Float(0x41b88313), SkBits2Float(0x42445d2d));  // 22.923f, 49.253f, 22.988f, 49.166f, 23.064f, 49.091f
path.cubicTo(SkBits2Float(0x41b949bb), SkBits2Float(0x4243ee95), SkBits2Float(0x41ba1063), SkBits2Float(0x424374ba), SkBits2Float(0x41baed92), SkBits2Float(0x42431166));  // 23.161f, 48.983f, 23.258f, 48.864f, 23.366f, 48.767f
path.cubicTo(SkBits2Float(0x41bb45a2), SkBits2Float(0x4242c393), SkBits2Float(0x41bbb43a), SkBits2Float(0x424276c6), SkBits2Float(0x41bc0e57), SkBits2Float(0x424228f3));  // 23.409f, 48.691f, 23.463f, 48.616f, 23.507f, 48.54f
path.cubicTo(SkBits2Float(0x41bc6667), SkBits2Float(0x4241e664), SkBits2Float(0x41bc7ae2), SkBits2Float(0x4241a4da), SkBits2Float(0x41bcd2f3), SkBits2Float(0x4241624b));  // 23.55f, 48.475f, 23.56f, 48.411f, 23.603f, 48.346f
path.cubicTo(SkBits2Float(0x41bd0001), SkBits2Float(0x42411478), SkBits2Float(0x41bd0001), SkBits2Float(0x4240c6a5), SkBits2Float(0x41bd1689), SkBits2Float(0x4240851c));  // 23.625f, 48.27f, 23.625f, 48.194f, 23.636f, 48.13f
path.cubicTo(SkBits2Float(0x41bd2d10), SkBits2Float(0x42404cca), SkBits2Float(0x41bdb023), SkBits2Float(0x423fd3f5), SkBits2Float(0x41bd8521), SkBits2Float(0x423f7adf));  // 23.647f, 48.075f, 23.711f, 47.957f, 23.69f, 47.87f
path.lineTo(SkBits2Float(0x41bd6e9a), SkBits2Float(0x423f7adf));  // 23.679f, 47.87f
path.cubicTo(SkBits2Float(0x41bd6e9a), SkBits2Float(0x423f7adf), SkBits2Float(0x41bd5813), SkBits2Float(0x423f4ed7), SkBits2Float(0x41bd168a), SkBits2Float(0x423f4499));  // 23.679f, 47.87f, 23.668f, 47.827f, 23.636f, 47.817f
path.cubicTo(SkBits2Float(0x41bc916b), SkBits2Float(0x423f22ce), SkBits2Float(0x41bc22d4), SkBits2Float(0x423f3955), SkBits2Float(0x41bb893a), SkBits2Float(0x423f178b));  // 23.571f, 47.784f, 23.517f, 47.806f, 23.442f, 47.773f
path.cubicTo(SkBits2Float(0x41bb2f1d), SkBits2Float(0x423f0c47), SkBits2Float(0x41bb041b), SkBits2Float(0x423ee03f), SkBits2Float(0x41baac0b), SkBits2Float(0x423ec9b8));  // 23.398f, 47.762f, 23.377f, 47.719f, 23.334f, 47.697f
path.cubicTo(SkBits2Float(0x41baac0b), SkBits2Float(0x423ebf7b), SkBits2Float(0x41bac086), SkBits2Float(0x423ea8f3), SkBits2Float(0x41bac086), SkBits2Float(0x423e926c));  // 23.334f, 47.687f, 23.344f, 47.665f, 23.344f, 47.643f
path.cubicTo(SkBits2Float(0x41bb2f1e), SkBits2Float(0x423e882f), SkBits2Float(0x41bc0e59), SkBits2Float(0x423e6664), SkBits2Float(0x41bc916b), SkBits2Float(0x423e5c26));  // 23.398f, 47.633f, 23.507f, 47.6f, 23.571f, 47.59f
path.cubicTo(SkBits2Float(0x41be4bc9), SkBits2Float(0x423e50e2), SkBits2Float(0x41c53542), SkBits2Float(0x423e926c), SkBits2Float(0x41c5ba61), SkBits2Float(0x423e24da));  // 23.787f, 47.579f, 24.651f, 47.643f, 24.716f, 47.536f
path.cubicTo(SkBits2Float(0x41c61271), SkBits2Float(0x423de24b), SkBits2Float(0x41c61271), SkBits2Float(0x423d1a9d), SkBits2Float(0x41c63f80), SkBits2Float(0x423ca1c8));  // 24.759f, 47.471f, 24.759f, 47.276f, 24.781f, 47.158f
path.cubicTo(SkBits2Float(0x41c68109), SkBits2Float(0x423bda1a), SkBits2Float(0x41c6ae18), SkBits2Float(0x423afceb), SkBits2Float(0x41c70628), SkBits2Float(0x423a2aff));  // 24.813f, 46.963f, 24.835f, 46.747f, 24.878f, 46.542f
path.cubicTo(SkBits2Float(0x41c71caf), SkBits2Float(0x42399ba3), SkBits2Float(0x41c81065), SkBits2Float(0x42379eb5), SkBits2Float(0x41c79fc2), SkBits2Float(0x4237459f));  // 24.889f, 46.402f, 25.008f, 45.905f, 24.953f, 45.818f
path.cubicTo(SkBits2Float(0x41c70628), SkBits2Float(0x4236e24b), SkBits2Float(0x41c4dd33), SkBits2Float(0x4237459f), SkBits2Float(0x41c45814), SkBits2Float(0x423750e3));  // 24.878f, 45.721f, 24.608f, 45.818f, 24.543f, 45.829f
path.cubicTo(SkBits2Float(0x41c245a5), SkBits2Float(0x42379eb6), SkBits2Float(0x41bea5e7), SkBits2Float(0x42380d4d), SkBits2Float(0x41bbf5c6), SkBits2Float(0x4237ec89));  // 24.284f, 45.905f, 23.831f, 46.013f, 23.495f, 45.981f
path.cubicTo(SkBits2Float(0x41b9f9df), SkBits2Float(0x4237e145), SkBits2Float(0x41b7e770), SkBits2Float(0x4237a9fa), SkBits2Float(0x41b62d12), SkBits2Float(0x4237676a));  // 23.247f, 45.97f, 22.988f, 45.916f, 22.772f, 45.851f
path.cubicTo(SkBits2Float(0x41b4312b), SkBits2Float(0x423724db), SkBits2Float(0x41b1f1ae), SkBits2Float(0x42369fbc), SkBits2Float(0x41af9baa), SkBits2Float(0x423673b4));  // 22.524f, 45.786f, 22.243f, 45.656f, 21.951f, 45.613f
path.cubicTo(SkBits2Float(0x41ae7ae5), SkBits2Float(0x42366977), SkBits2Float(0x41aced96), SkBits2Float(0x42365d2d), SkBits2Float(0x41ab8b48), SkBits2Float(0x42366977));  // 21.81f, 45.603f, 21.616f, 45.591f, 21.443f, 45.603f
path.cubicTo(SkBits2Float(0x41a9e771), SkBits2Float(0x42368a3c), SkBits2Float(0x41a82d13), SkBits2Float(0x4236d708), SkBits2Float(0x41a65e3a), SkBits2Float(0x4236b644));  // 21.238f, 45.635f, 21.022f, 45.71f, 20.796f, 45.678f
path.cubicTo(SkBits2Float(0x41a65e3a), SkBits2Float(0x4236ab00), SkBits2Float(0x41a647b3), SkBits2Float(0x42369fbd), SkBits2Float(0x41a65e3a), SkBits2Float(0x42369479));  // 20.796f, 45.667f, 20.785f, 45.656f, 20.796f, 45.645f
path.cubicTo(SkBits2Float(0x41a672b5), SkBits2Float(0x42366977), SkBits2Float(0x41a7a7f4), SkBits2Float(0x42363125), SkBits2Float(0x41a81898), SkBits2Float(0x42361ba4));  // 20.806f, 45.603f, 20.957f, 45.548f, 21.012f, 45.527f
path.cubicTo(SkBits2Float(0x41a85a21), SkBits2Float(0x42361060), SkBits2Float(0x41a8df40), SkBits2Float(0x4235d915), SkBits2Float(0x41a94dd7), SkBits2Float(0x4235cdd1));  // 21.044f, 45.516f, 21.109f, 45.462f, 21.163f, 45.451f
path.cubicTo(SkBits2Float(0x41ab8b48), SkBits2Float(0x42356a7d), SkBits2Float(0x41af8523), SkBits2Float(0x423575c1), SkBits2Float(0x41b249be), SkBits2Float(0x42359685));  // 21.443f, 45.354f, 21.94f, 45.365f, 22.286f, 45.397f
path.cubicTo(SkBits2Float(0x41b3d70e), SkBits2Float(0x4235a1c9), SkBits2Float(0x41b6168b), SkBits2Float(0x4235cdd1), SkBits2Float(0x41b7e770), SkBits2Float(0x4235ad0c));  // 22.48f, 45.408f, 22.761f, 45.451f, 22.988f, 45.419f
path.cubicTo(SkBits2Float(0x41bac087), SkBits2Float(0x42359685), SkBits2Float(0x41bd6e9b), SkBits2Float(0x4234fbe5), SkBits2Float(0x41c03337), SkBits2Float(0x4234af18));  // 23.344f, 45.397f, 23.679f, 45.246f, 24.025f, 45.171f
path.cubicTo(SkBits2Float(0x41c2cac4), SkBits2Float(0x42346145), SkBits2Float(0x41c56252), SkBits2Float(0x4234820a), SkBits2Float(0x41c81066), SkBits2Float(0x42346145));  // 24.349f, 45.095f, 24.673f, 45.127f, 25.008f, 45.095f
path.cubicTo(SkBits2Float(0x41c824e1), SkBits2Float(0x42340935), SkBits2Float(0x41c89378), SkBits2Float(0x42330a3b), SkBits2Float(0x41c7b649), SkBits2Float(0x4232fef7));  // 25.018f, 45.009f, 25.072f, 44.76f, 24.964f, 44.749f
path.cubicTo(SkBits2Float(0x41c6d91a), SkBits2Float(0x4232e976), SkBits2Float(0x41c5a3da), SkBits2Float(0x42338416), SkBits2Float(0x41c51ebc), SkBits2Float(0x4233a4da));  // 24.856f, 44.728f, 24.705f, 44.879f, 24.64f, 44.911f
path.cubicTo(SkBits2Float(0x41c42b06), SkBits2Float(0x4233bb61), SkBits2Float(0x41c2cac4), SkBits2Float(0x4233d0e2), SkBits2Float(0x41c1d70e), SkBits2Float(0x4233e769));  // 24.521f, 44.933f, 24.349f, 44.954f, 24.23f, 44.976f
path.cubicTo(SkBits2Float(0x41c08b47), SkBits2Float(0x4233f2ad), SkBits2Float(0x41bf1272), SkBits2Float(0x4233c6a4), SkBits2Float(0x41bdf3ba), SkBits2Float(0x4233bb61));  // 24.068f, 44.987f, 23.884f, 44.944f, 23.744f, 44.933f
path.cubicTo(SkBits2Float(0x41bcd2f5), SkBits2Float(0x4233b01d), SkBits2Float(0x41bbf5c6), SkBits2Float(0x4233b01d), SkBits2Float(0x41baed95), SkBits2Float(0x4233a4da));  // 23.603f, 44.922f, 23.495f, 44.922f, 23.366f, 44.911f
path.cubicTo(SkBits2Float(0x41ba26ed), SkBits2Float(0x42338f59), SkBits2Float(0x41b91cb0), SkBits2Float(0x4233580d), SkBits2Float(0x41b83f81), SkBits2Float(0x4233580d));  // 23.269f, 44.89f, 23.139f, 44.836f, 23.031f, 44.836f
path.cubicTo(SkBits2Float(0x41b4b43d), SkBits2Float(0x42333642), SkBits2Float(0x41b19791), SkBits2Float(0x4233a4da), SkBits2Float(0x41aea7f4), SkBits2Float(0x4233d0e2));  // 22.588f, 44.803f, 22.199f, 44.911f, 21.832f, 44.954f
path.cubicTo(SkBits2Float(0x41aba1cf), SkBits2Float(0x42340934), SkBits2Float(0x41a7666b), SkBits2Float(0x4233e769), SkBits2Float(0x41a4b856), SkBits2Float(0x42338415));  // 21.454f, 45.009f, 20.925f, 44.976f, 20.59f, 44.879f
path.cubicTo(SkBits2Float(0x41a46046), SkBits2Float(0x423378d1), SkBits2Float(0x41a3f1ae), SkBits2Float(0x4233580d), SkBits2Float(0x41a3c6ac), SkBits2Float(0x42334cc9));  // 20.547f, 44.868f, 20.493f, 44.836f, 20.472f, 44.825f
path.cubicTo(SkBits2Float(0x41a28f60), SkBits2Float(0x4233157d), SkBits2Float(0x41a19db6), SkBits2Float(0x42330a3a), SkBits2Float(0x41a0c087), SkBits2Float(0x4232c7aa));  // 20.32f, 44.771f, 20.202f, 44.76f, 20.094f, 44.695f
path.cubicTo(SkBits2Float(0x41a0eb89), SkBits2Float(0x4232bc66), SkBits2Float(0x41a0eb89), SkBits2Float(0x4232905e), SkBits2Float(0x41a10210), SkBits2Float(0x4232905e));  // 20.115f, 44.684f, 20.115f, 44.641f, 20.126f, 44.641f
path.cubicTo(SkBits2Float(0x41a19db6), SkBits2Float(0x42325912), SkBits2Float(0x41a2645e), SkBits2Float(0x42326f99), SkBits2Float(0x41a35608), SkBits2Float(0x42326f99));  // 20.202f, 44.587f, 20.299f, 44.609f, 20.417f, 44.609f
path.cubicTo(SkBits2Float(0x41a476cd), SkBits2Float(0x42324ed4), SkBits2Float(0x41a5ed95), SkBits2Float(0x4232384d), SkBits2Float(0x41a724e1), SkBits2Float(0x42320c45));  // 20.558f, 44.577f, 20.741f, 44.555f, 20.893f, 44.512f
path.cubicTo(SkBits2Float(0x41a8c8b8), SkBits2Float(0x4231c9b6), SkBits2Float(0x41aa999d), SkBits2Float(0x42316662), SkBits2Float(0x41ac26ed), SkBits2Float(0x4231188f));  // 21.098f, 44.447f, 21.325f, 44.35f, 21.519f, 44.274f
path.cubicTo(SkBits2Float(0x41af168b), SkBits2Float(0x423072ac), SkBits2Float(0x41b249be), SkBits2Float(0x42300f58), SkBits2Float(0x41b57ae5), SkBits2Float(0x422fe249));  // 21.886f, 44.112f, 22.286f, 44.015f, 22.685f, 43.971f
path.cubicTo(SkBits2Float(0x41b66e9b), SkBits2Float(0x422fd80c), SkBits2Float(0x41b7d0e9), SkBits2Float(0x422fee93), SkBits2Float(0x41b89791), SkBits2Float(0x422fee93));  // 22.804f, 43.961f, 22.977f, 43.983f, 23.074f, 43.983f
path.cubicTo(SkBits2Float(0x41bb1897), SkBits2Float(0x42300f58), SkBits2Float(0x41bd2d12), SkBits2Float(0x423024d9), SkBits2Float(0x41bfc49f), SkBits2Float(0x4230301c));  // 23.387f, 44.015f, 23.647f, 44.036f, 23.971f, 44.047f
path.cubicTo(SkBits2Float(0x41c0e357), SkBits2Float(0x423046a3), SkBits2Float(0x41c245a5), SkBits2Float(0x42305c24), SkBits2Float(0x41c3a7f3), SkBits2Float(0x423051e7));  // 24.111f, 44.069f, 24.284f, 44.09f, 24.457f, 44.08f
path.cubicTo(SkBits2Float(0x41c50835), SkBits2Float(0x423046a3), SkBits2Float(0x41c69791), SkBits2Float(0x42300f58), SkBits2Float(0x41c79fc2), SkBits2Float(0x422fb641));  // 24.629f, 44.069f, 24.824f, 44.015f, 24.953f, 43.928f
path.cubicTo(SkBits2Float(0x41c7f9df), SkBits2Float(0x422fa0c0), SkBits2Float(0x41c86876), SkBits2Float(0x422f5e31), SkBits2Float(0x41c8eb89), SkBits2Float(0x422f52ed));  // 24.997f, 43.907f, 25.051f, 43.842f, 25.115f, 43.831f
path.cubicTo(SkBits2Float(0x41c9b43d), SkBits2Float(0x422f3c66), SkBits2Float(0x41c9df3f), SkBits2Float(0x422fb641), SkBits2Float(0x41c9f5c6), SkBits2Float(0x42300f57));  // 25.213f, 43.809f, 25.234f, 43.928f, 25.245f, 44.015f
path.cubicTo(SkBits2Float(0x41ca0c4d), SkBits2Float(0x4230e143), SkBits2Float(0x41c9df3f), SkBits2Float(0x42319ca7), SkBits2Float(0x41c9f5c6), SkBits2Float(0x4232384d));  // 25.256f, 44.22f, 25.234f, 44.403f, 25.245f, 44.555f
path.cubicTo(SkBits2Float(0x41ca395c), SkBits2Float(0x4234fbe2), SkBits2Float(0x41ca22d4), SkBits2Float(0x4237cabc), SkBits2Float(0x41ca7ae5), SkBits2Float(0x423a6d8c));  // 25.278f, 45.246f, 25.267f, 45.948f, 25.31f, 46.607f
path.cubicTo(SkBits2Float(0x41ca916c), SkBits2Float(0x423b3f78), SkBits2Float(0x41ca645e), SkBits2Float(0x423ca1c5), SkBits2Float(0x41ca916c), SkBits2Float(0x423d9475));  // 25.321f, 46.812f, 25.299f, 47.158f, 25.321f, 47.395f
path.cubicTo(SkBits2Float(0x41ca916c), SkBits2Float(0x423daafc), SkBits2Float(0x41ca7ae5), SkBits2Float(0x423dd704), SkBits2Float(0x41ca916c), SkBits2Float(0x423dec85));  // 25.321f, 47.417f, 25.31f, 47.46f, 25.321f, 47.481f
path.cubicTo(SkBits2Float(0x41caa5e7), SkBits2Float(0x423e0e50), SkBits2Float(0x41cb0004), SkBits2Float(0x423e459c), SkBits2Float(0x41cb2b06), SkBits2Float(0x423e50df));  // 25.331f, 47.514f, 25.375f, 47.568f, 25.396f, 47.579f
path.cubicTo(SkBits2Float(0x41cb6e9c), SkBits2Float(0x423e5c23), SkBits2Float(0x41ce47b2), SkBits2Float(0x423e7ce7), SkBits2Float(0x41ce8b48), SkBits2Float(0x423e6660));  // 25.429f, 47.59f, 25.785f, 47.622f, 25.818f, 47.6f
path.lineTo(SkBits2Float(0x41ceb64a), SkBits2Float(0x423e5c23));  // 25.839f, 47.59f
path.cubicTo(SkBits2Float(0x41d1395c), SkBits2Float(0x423e5c23), SkBits2Float(0x41d41273), SkBits2Float(0x423e50df), SkBits2Float(0x41d6666b), SkBits2Float(0x423e6660));  // 26.153f, 47.59f, 26.509f, 47.579f, 26.8f, 47.6f
path.cubicTo(SkBits2Float(0x41d71898), SkBits2Float(0x423e7ce7), SkBits2Float(0x41d80a42), SkBits2Float(0x423e5c23), SkBits2Float(0x41d8a5e8), SkBits2Float(0x423e7ce7));  // 26.887f, 47.622f, 27.005f, 47.59f, 27.081f, 47.622f
path.cubicTo(SkBits2Float(0x41d8d2f6), SkBits2Float(0x423e882b), SkBits2Float(0x41d8d2f6), SkBits2Float(0x423e9268), SkBits2Float(0x41d8fdf8), SkBits2Float(0x423e9eb2));  // 27.103f, 47.633f, 27.103f, 47.643f, 27.124f, 47.655f
path.cubicTo(SkBits2Float(0x41d8e771), SkBits2Float(0x423ebf77), SkBits2Float(0x41d8fdf8), SkBits2Float(0x423ed4f8), SkBits2Float(0x41d8e771), SkBits2Float(0x423eeb7f));  // 27.113f, 47.687f, 27.124f, 47.708f, 27.113f, 47.73f
path.cubicTo(SkBits2Float(0x41d88f61), SkBits2Float(0x423f4496), SkBits2Float(0x41d71898), SkBits2Float(0x423f4496), SkBits2Float(0x41d6aa00), SkBits2Float(0x423f9162));  // 27.07f, 47.817f, 26.887f, 47.817f, 26.833f, 47.892f
path.cubicTo(SkBits2Float(0x41d547b2), SkBits2Float(0x42406e91), SkBits2Float(0x41d43d75), SkBits2Float(0x4241ba58), SkBits2Float(0x41d38d54), SkBits2Float(0x4242b952));  // 26.66f, 48.108f, 26.53f, 48.432f, 26.444f, 48.681f
path.cubicTo(SkBits2Float(0x41d1395c), SkBits2Float(0x4245a8f0), SkBits2Float(0x41d0b231), SkBits2Float(0x42491dac), SkBits2Float(0x41d2147f), SkBits2Float(0x424c2f15));  // 26.153f, 49.415f, 26.087f, 50.279f, 26.26f, 51.046f
path.cubicTo(SkBits2Float(0x41d2418d), SkBits2Float(0x424c7be2), SkBits2Float(0x41d2999e), SkBits2Float(0x424cc9b5), SkBits2Float(0x41d2b025), SkBits2Float(0x424d0c44));  // 26.282f, 51.121f, 26.325f, 51.197f, 26.336f, 51.262f
path.cubicTo(SkBits2Float(0x41d33544), SkBits2Float(0x424dc7a8), SkBits2Float(0x41d3a3db), SkBits2Float(0x424e8413), SkBits2Float(0x41d453fc), SkBits2Float(0x424f136f));  // 26.401f, 51.445f, 26.455f, 51.629f, 26.541f, 51.769f
path.cubicTo(SkBits2Float(0x41d453fc), SkBits2Float(0x424f136f), SkBits2Float(0x41d59fc3), SkBits2Float(0x42506a79), SkBits2Float(0x41d6c087), SkBits2Float(0x4250e454));  // 26.541f, 51.769f, 26.703f, 52.104f, 26.844f, 52.223f
path.cubicTo(SkBits2Float(0x41d6c087), SkBits2Float(0x4250ef98), SkBits2Float(0x41d6eb89), SkBits2Float(0x4251105c), SkBits2Float(0x41d70210), SkBits2Float(0x4251105c));  // 26.844f, 52.234f, 26.865f, 52.266f, 26.876f, 52.266f
path.cubicTo(SkBits2Float(0x41d71897), SkBits2Float(0x42511ba0), SkBits2Float(0x41d75a20), SkBits2Float(0x4251105c), SkBits2Float(0x41d7872f), SkBits2Float(0x4251105c));  // 26.887f, 52.277f, 26.919f, 52.266f, 26.941f, 52.266f
path.cubicTo(SkBits2Float(0x41d87ae5), SkBits2Float(0x42501ca6), SkBits2Float(0x41d9147f), SkBits2Float(0x424f136e), SkBits2Float(0x41da0835), SkBits2Float(0x424e157b));  // 27.06f, 52.028f, 27.135f, 51.769f, 27.254f, 51.521f
path.cubicTo(SkBits2Float(0x41da1ebc), SkBits2Float(0x424df4b6), SkBits2Float(0x41db1066), SkBits2Float(0x424d0c44), SkBits2Float(0x41db1066), SkBits2Float(0x424d0100));  // 27.265f, 51.489f, 27.383f, 51.262f, 27.383f, 51.251f
path.cubicTo(SkBits2Float(0x41db3d74), SkBits2Float(0x424cc9b4), SkBits2Float(0x41db9585), SkBits2Float(0x424c8725), SkBits2Float(0x41dbd91a), SkBits2Float(0x424c5b1d));  // 27.405f, 51.197f, 27.448f, 51.132f, 27.481f, 51.089f
path.cubicTo(SkBits2Float(0x41dc5e39), SkBits2Float(0x424bcbc1), SkBits2Float(0x41dcf7d2), SkBits2Float(0x424b301b), SkBits2Float(0x41dd7cf1), SkBits2Float(0x424aac02));  // 27.546f, 50.949f, 27.621f, 50.797f, 27.686f, 50.668f
path.cubicTo(SkBits2Float(0x41ddd501), SkBits2Float(0x424a5e2f), SkBits2Float(0x41ddeb89), SkBits2Float(0x424a105c), SkBits2Float(0x41de4399), SkBits2Float(0x4249b84c));  // 27.729f, 50.592f, 27.74f, 50.516f, 27.783f, 50.43f
path.cubicTo(SkBits2Float(0x41de70a7), SkBits2Float(0x4249a1c5), SkBits2Float(0x41def5c6), SkBits2Float(0x42490725), SkBits2Float(0x41df20c8), SkBits2Float(0x4248e660));  // 27.805f, 50.408f, 27.87f, 50.257f, 27.891f, 50.225f
path.cubicTo(SkBits2Float(0x41df8f60), SkBits2Float(0x42488206), SkBits2Float(0x41e0c49f), SkBits2Float(0x42474cc6), SkBits2Float(0x41e10835), SkBits2Float(0x42472c02));  // 27.945f, 50.127f, 28.096f, 49.825f, 28.129f, 49.793f
path.cubicTo(SkBits2Float(0x41e11ebc), SkBits2Float(0x42472c02), SkBits2Float(0x41e13337), SkBits2Float(0x4246fef4), SkBits2Float(0x41e13337), SkBits2Float(0x4246f4b6));  // 28.14f, 49.793f, 28.15f, 49.749f, 28.15f, 49.739f
path.cubicTo(SkBits2Float(0x41e149be), SkBits2Float(0x4246c7a8), SkBits2Float(0x41e226ed), SkBits2Float(0x42461787), SkBits2Float(0x41e253fc), SkBits2Float(0x4245df35));  // 28.161f, 49.695f, 28.269f, 49.523f, 28.291f, 49.468f
path.cubicTo(SkBits2Float(0x41e27efe), SkBits2Float(0x4245d3f1), SkBits2Float(0x41e2ac0c), SkBits2Float(0x42459ca6), SkBits2Float(0x41e2ac0c), SkBits2Float(0x42459162));  // 28.312f, 49.457f, 28.334f, 49.403f, 28.334f, 49.392f
path.cubicTo(SkBits2Float(0x41e372b4), SkBits2Float(0x4244e141), SkBits2Float(0x41e4666a), SkBits2Float(0x42445c23), SkBits2Float(0x41e4eb89), SkBits2Float(0x42437ef3));  // 28.431f, 49.22f, 28.55f, 49.09f, 28.615f, 48.874f
path.cubicTo(SkBits2Float(0x41e4a7f3), SkBits2Float(0x424373af), SkBits2Float(0x41e47ae5), SkBits2Float(0x42435e2e), SkBits2Float(0x41e4666a), SkBits2Float(0x42435e2e));  // 28.582f, 48.863f, 28.56f, 48.842f, 28.55f, 48.842f
path.cubicTo(SkBits2Float(0x41e3893b), SkBits2Float(0x42433c63), SkBits2Float(0x41e1fbeb), SkBits2Float(0x4243686b), SkBits2Float(0x41e18b47), SkBits2Float(0x42431b9f));  // 28.442f, 48.809f, 28.248f, 48.852f, 28.193f, 48.777f
path.cubicTo(SkBits2Float(0x41e16045), SkBits2Float(0x4242f9d4), SkBits2Float(0x41e18b47), SkBits2Float(0x4242ee91), SkBits2Float(0x41e16045), SkBits2Float(0x4242d910));  // 28.172f, 48.744f, 28.193f, 48.733f, 28.172f, 48.712f
path.cubicTo(SkBits2Float(0x41e1a1ce), SkBits2Float(0x4242b84b), SkBits2Float(0x41e1fbeb), SkBits2Float(0x42429681), SkBits2Float(0x41e226ed), SkBits2Float(0x42429681));  // 28.204f, 48.68f, 28.248f, 48.647f, 28.269f, 48.647f
path.cubicTo(SkBits2Float(0x41e3cac4), SkBits2Float(0x42425f35), SkBits2Float(0x41e9c087), SkBits2Float(0x4242b84c), SkBits2Float(0x41ea5c2c), SkBits2Float(0x424248ae));  // 28.474f, 48.593f, 29.219f, 48.68f, 29.295f, 48.571f
path.cubicTo(SkBits2Float(0x41eacac4), SkBits2Float(0x4241fbe1), SkBits2Float(0x41eacac4), SkBits2Float(0x42414aba), SkBits2Float(0x41eaf7d2), SkBits2Float(0x4240d0df));  // 29.349f, 48.496f, 29.349f, 48.323f, 29.371f, 48.204f
path.cubicTo(SkBits2Float(0x41eb395b), SkBits2Float(0x4240580a), SkBits2Float(0x41eba7f3), SkBits2Float(0x423fb121), SkBits2Float(0x41ebd501), SkBits2Float(0x423f21c4));  // 29.403f, 48.086f, 29.457f, 47.923f, 29.479f, 47.783f
path.cubicTo(SkBits2Float(0x41ec2d11), SkBits2Float(0x423e4fd8), SkBits2Float(0x41ec5813), SkBits2Float(0x423d936e), SkBits2Float(0x41ecb230), SkBits2Float(0x423cb63f));  // 29.522f, 47.578f, 29.543f, 47.394f, 29.587f, 47.178f
path.cubicTo(SkBits2Float(0x41ecc8b7), SkBits2Float(0x423c5e2f), SkBits2Float(0x41edba61), SkBits2Float(0x423b332d), SkBits2Float(0x41ed8f5f), SkBits2Float(0x423ac495));  // 29.598f, 47.092f, 29.716f, 46.8f, 29.695f, 46.692f
path.cubicTo(SkBits2Float(0x41ed6251), SkBits2Float(0x423a8d49), SkBits2Float(0x41ec9ba9), SkBits2Float(0x423a407c), SkBits2Float(0x41ec2d11), SkBits2Float(0x423a3539));  // 29.673f, 46.638f, 29.576f, 46.563f, 29.522f, 46.552f
path.cubicTo(SkBits2Float(0x41ec0003), SkBits2Float(0x423a29f5), SkBits2Float(0x41ebeb88), SkBits2Float(0x423a3539), SkBits2Float(0x41ebd501), SkBits2Float(0x423a3539));  // 29.5f, 46.541f, 29.49f, 46.552f, 29.479f, 46.552f
path.cubicTo(SkBits2Float(0x41eb6669), SkBits2Float(0x423a29f5), SkBits2Float(0x41ea72b3), SkBits2Float(0x4239f2aa), SkBits2Float(0x41e9c086), SkBits2Float(0x423a0931));  // 29.425f, 46.541f, 29.306f, 46.487f, 29.219f, 46.509f
path.cubicTo(SkBits2Float(0x41e99584), SkBits2Float(0x423a0931), SkBits2Float(0x41e96876), SkBits2Float(0x423a29f6), SkBits2Float(0x41e953fb), SkBits2Float(0x423a3539));  // 29.198f, 46.509f, 29.176f, 46.541f, 29.166f, 46.552f
path.cubicTo(SkBits2Float(0x41e96876), SkBits2Float(0x423a8d49), SkBits2Float(0x41e9c086), SkBits2Float(0x423acfd9), SkBits2Float(0x41e9d70d), SkBits2Float(0x423b28ef));  // 29.176f, 46.638f, 29.219f, 46.703f, 29.23f, 46.79f
path.cubicTo(SkBits2Float(0x41ea041b), SkBits2Float(0x423bd910), SkBits2Float(0x41e8fbea), SkBits2Float(0x423c73b0), SkBits2Float(0x41e849bd), SkBits2Float(0x423cac01));  // 29.252f, 46.962f, 29.123f, 47.113f, 29.036f, 47.168f
path.cubicTo(SkBits2Float(0x41e75607), SkBits2Float(0x423cf8ce), SkBits2Float(0x41e5f3b9), SkBits2Float(0x423ced8a), SkBits2Float(0x41e4eb88), SkBits2Float(0x423cd809));  // 28.917f, 47.243f, 28.744f, 47.232f, 28.615f, 47.211f
path.cubicTo(SkBits2Float(0x41e372b3), SkBits2Float(0x423cb63e), SkBits2Float(0x41e2ac0b), SkBits2Float(0x423c0517), SkBits2Float(0x41e10834), SkBits2Float(0x423c52ea));  // 28.431f, 47.178f, 28.334f, 47.005f, 28.129f, 47.081f
path.cubicTo(SkBits2Float(0x41e0db26), SkBits2Float(0x423cd809), SkBits2Float(0x41e0999c), SkBits2Float(0x423d46a0), SkBits2Float(0x41dfd0e8), SkBits2Float(0x423d72a8));  // 28.107f, 47.211f, 28.075f, 47.319f, 27.977f, 47.362f
path.cubicTo(SkBits2Float(0x41deb230), SkBits2Float(0x423dcab8), SkBits2Float(0x41dd3b67), SkBits2Float(0x423d8829), SkBits2Float(0x41dc312a), SkBits2Float(0x423d46a0));  // 27.837f, 47.448f, 27.654f, 47.383f, 27.524f, 47.319f
path.cubicTo(SkBits2Float(0x41dae563), SkBits2Float(0x423cf8cd), SkBits2Float(0x41d98316), SkBits2Float(0x423cccc5), SkBits2Float(0x41d8645d), SkBits2Float(0x423c6971));  // 27.362f, 47.243f, 27.189f, 47.2f, 27.049f, 47.103f
path.cubicTo(SkBits2Float(0x41d7df3e), SkBits2Float(0x423c52ea), SkBits2Float(0x41d72d11), SkBits2Float(0x423c311f), SkBits2Float(0x41d6a9ff), SkBits2Float(0x423c0517));  // 26.984f, 47.081f, 26.897f, 47.048f, 26.833f, 47.005f
path.cubicTo(SkBits2Float(0x41d67cf1), SkBits2Float(0x423bfada), SkBits2Float(0x41d572b3), SkBits2Float(0x423b967f), SkBits2Float(0x41d5893a), SkBits2Float(0x423b967f));  // 26.811f, 46.995f, 26.681f, 46.897f, 26.692f, 46.897f
path.cubicTo(SkBits2Float(0x41d5893a), SkBits2Float(0x423b967f), SkBits2Float(0x41d5b648), SkBits2Float(0x423b6a77), SkBits2Float(0x41d5ccd0), SkBits2Float(0x423b6a77));  // 26.692f, 46.897f, 26.714f, 46.854f, 26.725f, 46.854f
path.cubicTo(SkBits2Float(0x41d6eb88), SkBits2Float(0x423b3e6f), SkBits2Float(0x41d8374f), SkBits2Float(0x423b967f), SkBits2Float(0x41d8fdf7), SkBits2Float(0x423bad06));  // 26.865f, 46.811f, 27.027f, 46.897f, 27.124f, 46.919f
path.cubicTo(SkBits2Float(0x41d9c6ab), SkBits2Float(0x423bb84a), SkBits2Float(0x41da49be), SkBits2Float(0x423bb84a), SkBits2Float(0x41db1066), SkBits2Float(0x423bd90e));  // 27.222f, 46.93f, 27.286f, 46.93f, 27.383f, 46.962f
path.cubicTo(SkBits2Float(0x41db810a), SkBits2Float(0x423bd90e), SkBits2Float(0x41dc5e39), SkBits2Float(0x423bfad9), SkBits2Float(0x41dcf7d3), SkBits2Float(0x423bef95));  // 27.438f, 46.962f, 27.546f, 46.995f, 27.621f, 46.984f
path.cubicTo(SkBits2Float(0x41ddd502), SkBits2Float(0x423bc38d), SkBits2Float(0x41dd4fe3), SkBits2Float(0x423b332b), SkBits2Float(0x41dd7cf2), SkBits2Float(0x423ab94f));  // 27.729f, 46.941f, 27.664f, 46.8f, 27.686f, 46.681f
path.cubicTo(SkBits2Float(0x41dda7f4), SkBits2Float(0x423a77c6), SkBits2Float(0x41de2d13), SkBits2Float(0x423a29f3), SkBits2Float(0x41de70a8), SkBits2Float(0x423a136c));  // 27.707f, 46.617f, 27.772f, 46.541f, 27.805f, 46.519f
path.cubicTo(SkBits2Float(0x41dfba62), SkBits2Float(0x4239c69f), SkBits2Float(0x41e253fc), SkBits2Float(0x423a092f), SkBits2Float(0x41e372b4), SkBits2Float(0x423a4bbe));  // 27.966f, 46.444f, 28.291f, 46.509f, 28.431f, 46.574f
path.cubicTo(SkBits2Float(0x41e40e5a), SkBits2Float(0x423a6c83), SkBits2Float(0x41e49379), SkBits2Float(0x423a8d47), SkBits2Float(0x41e55a21), SkBits2Float(0x423ab94f));  // 28.507f, 46.606f, 28.572f, 46.638f, 28.669f, 46.681f
path.lineTo(SkBits2Float(0x41e58523), SkBits2Float(0x423acfd6));  // 28.69f, 46.703f
path.cubicTo(SkBits2Float(0x41e5b231), SkBits2Float(0x423acfd6), SkBits2Float(0x41e60a42), SkBits2Float(0x423ac492), SkBits2Float(0x41e66252), SkBits2Float(0x423acfd6));  // 28.712f, 46.703f, 28.755f, 46.692f, 28.798f, 46.703f
path.cubicTo(SkBits2Float(0x41e66252), SkBits2Float(0x423ab94f), SkBits2Float(0x41e68f60), SkBits2Float(0x423ab94f), SkBits2Float(0x41e6a5e8), SkBits2Float(0x423aae0b));  // 28.798f, 46.681f, 28.82f, 46.681f, 28.831f, 46.67f
path.cubicTo(SkBits2Float(0x41e6fdf8), SkBits2Float(0x423a136b), SkBits2Float(0x41e5dd34), SkBits2Float(0x423978cc), SkBits2Float(0x41e68f61), SkBits2Float(0x4238fef0));  // 28.874f, 46.519f, 28.733f, 46.368f, 28.82f, 46.249f
path.cubicTo(SkBits2Float(0x41e72b07), SkBits2Float(0x42389058), SkBits2Float(0x41eaf7d4), SkBits2Float(0x42391577), SkBits2Float(0x41ec5815), SkBits2Float(0x4238f3ac));  // 28.896f, 46.141f, 29.371f, 46.271f, 29.543f, 46.238f
path.cubicTo(SkBits2Float(0x41ef1cb1), SkBits2Float(0x4238bd66), SkBits2Float(0x41ed6252), SkBits2Float(0x4237d4f4), SkBits2Float(0x41ede771), SkBits2Float(0x42369eae));  // 29.889f, 46.185f, 29.673f, 45.958f, 29.738f, 45.655f
path.cubicTo(SkBits2Float(0x41ee28fa), SkBits2Float(0x423651e1), SkBits2Float(0x41ee8317), SkBits2Float(0x42366868), SkBits2Float(0x41eedb27), SkBits2Float(0x42365c1f));  // 29.77f, 45.58f, 29.814f, 45.602f, 29.857f, 45.59f
path.cubicTo(SkBits2Float(0x41ef0629), SkBits2Float(0x4236a9f2), SkBits2Float(0x41ef3337), SkBits2Float(0x42371889), SkBits2Float(0x41ef3337), SkBits2Float(0x42375b19));  // 29.878f, 45.666f, 29.9f, 45.774f, 29.9f, 45.839f
path.cubicTo(SkBits2Float(0x41ef49be), SkBits2Float(0x4237e038), SkBits2Float(0x41ef3337), SkBits2Float(0x42386450), SkBits2Float(0x41ef49be), SkBits2Float(0x4238d2e8));  // 29.911f, 45.969f, 29.9f, 46.098f, 29.911f, 46.206f
path.cubicTo(SkBits2Float(0x41ef8b47), SkBits2Float(0x42394cc3), SkBits2Float(0x41eff9df), SkBits2Float(0x4239e763), SkBits2Float(0x41f026ed), SkBits2Float(0x423a613e));  // 29.943f, 46.325f, 29.997f, 46.476f, 30.019f, 46.595f
path.cubicTo(SkBits2Float(0x41f0ac0c), SkBits2Float(0x423b967d), SkBits2Float(0x41f11897), SkBits2Float(0x423ca0bb), SkBits2Float(0x41f1893b), SkBits2Float(0x423dd5fa));  // 30.084f, 46.897f, 30.137f, 47.157f, 30.192f, 47.459f
path.cubicTo(SkBits2Float(0x41f19db6), SkBits2Float(0x423e1889), SkBits2Float(0x41f1f5c6), SkBits2Float(0x423e7bdd), SkBits2Float(0x41f20e5a), SkBits2Float(0x423ebe6d));  // 30.202f, 47.524f, 30.245f, 47.621f, 30.257f, 47.686f
path.cubicTo(SkBits2Float(0x41f27ae5), SkBits2Float(0x423f9059), SkBits2Float(0x41f2be7b), SkBits2Float(0x42406d88), SkBits2Float(0x41f3168b), SkBits2Float(0x424128ec));  // 30.31f, 47.891f, 30.343f, 48.107f, 30.386f, 48.29f
path.cubicTo(SkBits2Float(0x41f35814), SkBits2Float(0x42418203), SkBits2Float(0x41f35814), SkBits2Float(0x4241e556), SkBits2Float(0x41f38523), SkBits2Float(0x42423329));  // 30.418f, 48.377f, 30.418f, 48.474f, 30.44f, 48.55f
path.cubicTo(SkBits2Float(0x41f3b025), SkBits2Float(0x424248aa), SkBits2Float(0x41f420c9), SkBits2Float(0x424275b8), SkBits2Float(0x41f46252), SkBits2Float(0x424280fc));  // 30.461f, 48.571f, 30.516f, 48.615f, 30.548f, 48.626f
path.cubicTo(SkBits2Float(0x41f4fdf8), SkBits2Float(0x4242967d), SkBits2Float(0x41f5db27), SkBits2Float(0x424275b8), SkBits2Float(0x41f674c1), SkBits2Float(0x424280fc));  // 30.624f, 48.647f, 30.732f, 48.615f, 30.807f, 48.626f
path.cubicTo(SkBits2Float(0x41f8f5c7), SkBits2Float(0x4242967d), SkBits2Float(0x41fc5609), SkBits2Float(0x424280fc), SkBits2Float(0x41feeb8a), SkBits2Float(0x4242a1c1));  // 31.12f, 48.647f, 31.542f, 48.626f, 31.865f, 48.658f
path.cubicTo(SkBits2Float(0x41ff45a7), SkBits2Float(0x4242a1c1), SkBits2Float(0x41ffdf40), SkBits2Float(0x424280fc), SkBits2Float(0x4200322f), SkBits2Float(0x4242a1c1));  // 31.909f, 48.658f, 31.984f, 48.626f, 32.049f, 48.658f
path.cubicTo(SkBits2Float(0x420048b6), SkBits2Float(0x4242a1c1), SkBits2Float(0x42005e37), SkBits2Float(0x4242c286), SkBits2Float(0x420074be), SkBits2Float(0x4242d90d));  // 32.071f, 48.658f, 32.092f, 48.69f, 32.114f, 48.712f
path.cubicTo(SkBits2Float(0x420074be), SkBits2Float(0x4242ee8e), SkBits2Float(0x42008002), SkBits2Float(0x42431b9c), SkBits2Float(0x420074be), SkBits2Float(0x4243311d));  // 32.114f, 48.733f, 32.125f, 48.777f, 32.114f, 48.798f
path.lineTo(SkBits2Float(0x420052f3), SkBits2Float(0x42433c61));  // 32.081f, 48.809f
path.cubicTo(SkBits2Float(0x42001cad), SkBits2Float(0x42439fb5), SkBits2Float(0x41ff2f1d), SkBits2Float(0x42436869), SkBits2Float(0x41fe7cf0), SkBits2Float(0x4243aaf9));  // 32.028f, 48.906f, 31.898f, 48.852f, 31.811f, 48.917f
path.cubicTo(SkBits2Float(0x41fe24e0), SkBits2Float(0x4243cbbe), SkBits2Float(0x41fd3336), SkBits2Float(0x4244cab7), SkBits2Float(0x41fd0627), SkBits2Float(0x42450203));  // 31.768f, 48.949f, 31.65f, 49.198f, 31.628f, 49.252f
path.cubicTo(SkBits2Float(0x41fcc291), SkBits2Float(0x4245438c), SkBits2Float(0x41fcc291), SkBits2Float(0x42457bde), SkBits2Float(0x41fcae17), SkBits2Float(0x4245be6d));  // 31.595f, 49.316f, 31.595f, 49.371f, 31.585f, 49.436f
path.cubicTo(SkBits2Float(0x41fc9790), SkBits2Float(0x4245fff6), SkBits2Float(0x41fc28f8), SkBits2Float(0x4246634a), SkBits2Float(0x41fc1271), SkBits2Float(0x4246b11d));  // 31.574f, 49.5f, 31.52f, 49.597f, 31.509f, 49.673f
path.cubicTo(SkBits2Float(0x41fbba61), SkBits2Float(0x42478e4c), SkBits2Float(0x41fba3d9), SkBits2Float(0x424880fc), SkBits2Float(0x41fbba61), SkBits2Float(0x424974b2));  // 31.466f, 49.889f, 31.455f, 50.126f, 31.466f, 50.364f
path.cubicTo(SkBits2Float(0x41fbd0e8), SkBits2Float(0x424a7de9), SkBits2Float(0x41fc8109), SkBits2Float(0x424b5b18), SkBits2Float(0x41fd47b1), SkBits2Float(0x424c4ecf));  // 31.477f, 50.623f, 31.563f, 50.839f, 31.66f, 51.077f
path.cubicTo(SkBits2Float(0x41fd8b47), SkBits2Float(0x424c915e), SkBits2Float(0x41fdccd0), SkBits2Float(0x424cde2b), SkBits2Float(0x41fe3b67), SkBits2Float(0x424d167d));  // 31.693f, 51.142f, 31.725f, 51.217f, 31.779f, 51.272f
path.cubicTo(SkBits2Float(0x41fe9377), SkBits2Float(0x424d4dc9), SkBits2Float(0x41fec086), SkBits2Float(0x424d8f52), SkBits2Float(0x41ff2f1d), SkBits2Float(0x424dc69e));  // 31.822f, 51.326f, 31.844f, 51.39f, 31.898f, 51.444f
path.cubicTo(SkBits2Float(0x41ff70a6), SkBits2Float(0x424df3ac), SkBits2Float(0x41ffdf3e), SkBits2Float(0x424e092d), SkBits2Float(0x42000626), SkBits2Float(0x424e3536));  // 31.93f, 51.488f, 31.984f, 51.509f, 32.006f, 51.552f
path.cubicTo(SkBits2Float(0x42003d72), SkBits2Float(0x424e6c82), SkBits2Float(0x4200c18a), SkBits2Float(0x424f3e6d), SkBits2Float(0x4201041a), SkBits2Float(0x424f49b1));  // 32.06f, 51.606f, 32.189f, 51.811f, 32.254f, 51.822f
path.cubicTo(SkBits2Float(0x420172b2), SkBits2Float(0x424f6b7c), SkBits2Float(0x4201ec8d), SkBits2Float(0x424e8309), SkBits2Float(0x42020d51), SkBits2Float(0x424e4bbd));  // 32.362f, 51.855f, 32.481f, 51.628f, 32.513f, 51.574f
path.cubicTo(SkBits2Float(0x4202be78), SkBits2Float(0x424d5807), SkBits2Float(0x42037ae2), SkBits2Float(0x424c6557), SkBits2Float(0x42044cce), SkBits2Float(0x424b9265));  // 32.686f, 51.336f, 32.87f, 51.099f, 33.075f, 50.893f
path.cubicTo(SkBits2Float(0x42049aa1), SkBits2Float(0x424b4598), SkBits2Float(0x4204e874), SkBits2Float(0x424ae13e), SkBits2Float(0x42054084), SkBits2Float(0x424a9471));  // 33.151f, 50.818f, 33.227f, 50.72f, 33.313f, 50.645f
path.cubicTo(SkBits2Float(0x42058d51), SkBits2Float(0x424a51e2), SkBits2Float(0x4206ef9f), SkBits2Float(0x4248fad7), SkBits2Float(0x42071063), SkBits2Float(0x4248cecf));  // 33.388f, 50.58f, 33.734f, 50.245f, 33.766f, 50.202f
path.cubicTo(SkBits2Float(0x42075e36), SkBits2Float(0x424876bf), SkBits2Float(0x4207cccd), SkBits2Float(0x4248342f), SkBits2Float(0x42083021), SkBits2Float(0x4247e65c));  // 33.842f, 50.116f, 33.95f, 50.051f, 34.047f, 49.975f
path.cubicTo(SkBits2Float(0x42088831), SkBits2Float(0x42478308), SkBits2Float(0x4208f6c9), SkBits2Float(0x4247363b), SkBits2Float(0x420970a4), SkBits2Float(0x4246f3ac));  // 34.133f, 49.878f, 34.241f, 49.803f, 34.36f, 49.738f
path.cubicTo(SkBits2Float(0x4209f5c3), SkBits2Float(0x42469a95), SkBits2Float(0x420a645a), SkBits2Float(0x42464285), SkBits2Float(0x420add2f), SkBits2Float(0x4245f4b2));  // 34.49f, 49.651f, 34.598f, 49.565f, 34.716f, 49.489f
path.cubicTo(SkBits2Float(0x420b2b02), SkBits2Float(0x4245be6c), SkBits2Float(0x420bc5a2), SkBits2Float(0x42455a12), SkBits2Float(0x420b8418), SkBits2Float(0x4244eb7b));  // 34.792f, 49.436f, 34.943f, 49.338f, 34.879f, 49.23f
path.cubicTo(SkBits2Float(0x420b624d), SkBits2Float(0x4244cab6), SkBits2Float(0x420b1fbe), SkBits2Float(0x42449eae), SkBits2Float(0x420b0a3d), SkBits2Float(0x42448827));  // 34.846f, 49.198f, 34.781f, 49.155f, 34.76f, 49.133f
path.cubicTo(SkBits2Float(0x420abd70), SkBits2Float(0x424450db), SkBits2Float(0x420a9ba5), SkBits2Float(0x42440e4c), SkBits2Float(0x420a5916), SkBits2Float(0x4243d700));  // 34.685f, 49.079f, 34.652f, 49.014f, 34.587f, 48.96f
path.cubicTo(SkBits2Float(0x420a3851), SkBits2Float(0x4243b63b), SkBits2Float(0x420a21ca), SkBits2Float(0x4243b63b), SkBits2Float(0x4209f5c2), SkBits2Float(0x42439fb4));  // 34.555f, 48.928f, 34.533f, 48.928f, 34.49f, 48.906f
path.cubicTo(SkBits2Float(0x4209ea7e), SkBits2Float(0x42439470), SkBits2Float(0x4209ea7e), SkBits2Float(0x424373ac), SkBits2Float(0x4209d3f7), SkBits2Float(0x42436868));  // 34.479f, 48.895f, 34.479f, 48.863f, 34.457f, 48.852f
path.cubicTo(SkBits2Float(0x4209b332), SkBits2Float(0x424352e7), SkBits2Float(0x42099db1), SkBits2Float(0x42435e2b), SkBits2Float(0x42097be7), SkBits2Float(0x424352e7));  // 34.425f, 48.831f, 34.404f, 48.842f, 34.371f, 48.831f
path.cubicTo(SkBits2Float(0x420970a3), SkBits2Float(0x42433c60), SkBits2Float(0x42096560), SkBits2Float(0x42431b9b), SkBits2Float(0x4209449b), SkBits2Float(0x42431b9b));  // 34.36f, 48.809f, 34.349f, 48.777f, 34.317f, 48.777f
path.cubicTo(SkBits2Float(0x4208f6c8), SkBits2Float(0x4242e349), SkBits2Float(0x42089eb8), SkBits2Float(0x4242c284), SkBits2Float(0x42083020), SkBits2Float(0x4242a1c0));  // 34.241f, 48.722f, 34.155f, 48.69f, 34.047f, 48.658f
path.cubicTo(SkBits2Float(0x42080f5b), SkBits2Float(0x4242967c), SkBits2Float(0x4207d810), SkBits2Float(0x42425f31), SkBits2Float(0x4207c188), SkBits2Float(0x42425f31));  // 34.015f, 48.647f, 33.961f, 48.593f, 33.939f, 48.593f
path.cubicTo(SkBits2Float(0x420748b3), SkBits2Float(0x424227e5), SkBits2Float(0x42066040), SkBits2Float(0x4241fbdd), SkBits2Float(0x4205b957), SkBits2Float(0x42421ca2));  // 33.821f, 48.539f, 33.594f, 48.496f, 33.431f, 48.528f
path.cubicTo(SkBits2Float(0x4204c6a7), SkBits2Float(0x42423329), SkBits2Float(0x42041580), SkBits2Float(0x4242ad04), SkBits2Float(0x42032d0d), SkBits2Float(0x4242c285));  // 33.194f, 48.55f, 33.021f, 48.669f, 32.794f, 48.69f
path.cubicTo(SkBits2Float(0x42032d0d), SkBits2Float(0x4242b848), SkBits2Float(0x420322d0), SkBits2Float(0x4242a1c0), SkBits2Float(0x42032d0d), SkBits2Float(0x4242a1c0));  // 32.794f, 48.68f, 32.784f, 48.658f, 32.794f, 48.658f
path.cubicTo(SkBits2Float(0x42032d0d), SkBits2Float(0x424280fb), SkBits2Float(0x42036459), SkBits2Float(0x424275b8), SkBits2Float(0x42036f9c), SkBits2Float(0x42426a74));  // 32.794f, 48.626f, 32.848f, 48.615f, 32.859f, 48.604f
path.cubicTo(SkBits2Float(0x4203e977), SkBits2Float(0x4241cfd4), SkBits2Float(0x4204580f), SkBits2Float(0x42418201), SkBits2Float(0x420529fa), SkBits2Float(0x42413f72));  // 32.978f, 48.453f, 33.086f, 48.377f, 33.291f, 48.312f
path.lineTo(SkBits2Float(0x42054abf), SkBits2Float(0x424128eb));  // 33.323f, 48.29f
path.cubicTo(SkBits2Float(0x4205cfde), SkBits2Float(0x4240fde9), SkBits2Float(0x420649b9), SkBits2Float(0x4240fde9), SkBits2Float(0x4206b850), SkBits2Float(0x4240b016));  // 33.453f, 48.248f, 33.572f, 48.248f, 33.68f, 48.172f
path.cubicTo(SkBits2Float(0x4206a1c9), SkBits2Float(0x4240998f), SkBits2Float(0x4206b850), SkBits2Float(0x42408e4b), SkBits2Float(0x4206a1c9), SkBits2Float(0x424078ca));  // 33.658f, 48.15f, 33.68f, 48.139f, 33.658f, 48.118f
path.cubicTo(SkBits2Float(0x42068104), SkBits2Float(0x4240363b), SkBits2Float(0x42054081), SkBits2Float(0x423fb11c), SkBits2Float(0x4204d1ea), SkBits2Float(0x423f9057));  // 33.626f, 48.053f, 33.313f, 47.923f, 33.205f, 47.891f
path.cubicTo(SkBits2Float(0x42044ccb), SkBits2Float(0x423f79d0), SkBits2Float(0x42035915), SkBits2Float(0x423f644f), SkBits2Float(0x4202be75), SkBits2Float(0x423f8513));  // 33.075f, 47.869f, 32.837f, 47.848f, 32.686f, 47.88f
path.cubicTo(SkBits2Float(0x42022f19), SkBits2Float(0x423f9b9a), SkBits2Float(0x4201c081), SkBits2Float(0x423fde2a), SkBits2Float(0x420125e2), SkBits2Float(0x423ff3ab));  // 32.546f, 47.902f, 32.438f, 47.967f, 32.287f, 47.988f
path.lineTo(SkBits2Float(0x42010f5b), SkBits2Float(0x423fc7a3));  // 32.265f, 47.945f
path.cubicTo(SkBits2Float(0x4201a9fb), SkBits2Float(0x423f167c), SkBits2Float(0x42036459), SkBits2Float(0x423d5c1e), SkBits2Float(0x4204580f), SkBits2Float(0x423d198f));  // 32.416f, 47.772f, 32.848f, 47.34f, 33.086f, 47.275f
path.cubicTo(SkBits2Float(0x4205b957), SkBits2Float(0x423cabfe), SkBits2Float(0x4207c188), SkBits2Float(0x423cd806), SkBits2Float(0x42090d4e), SkBits2Float(0x423d24d3));  // 33.431f, 47.168f, 33.939f, 47.211f, 34.263f, 47.286f
path.cubicTo(SkBits2Float(0x420ae871), SkBits2Float(0x423d936b), SkBits2Float(0x420c9892), SkBits2Float(0x423e7bdd), SkBits2Float(0x420e6871), SkBits2Float(0x423ed3ee));  // 34.727f, 47.394f, 35.149f, 47.621f, 35.602f, 47.707f
path.cubicTo(SkBits2Float(0x42103956), SkBits2Float(0x423f438c), SkBits2Float(0x42121479), SkBits2Float(0x423f0b3a), SkBits2Float(0x4213c49a), SkBits2Float(0x423e2e0b));  // 36.056f, 47.816f, 36.52f, 47.761f, 36.942f, 47.545f
path.cubicTo(SkBits2Float(0x4214cdd1), SkBits2Float(0x423db536), SkBits2Float(0x4215c081), SkBits2Float(0x423d24d4), SkBits2Float(0x42169db1), SkBits2Float(0x423c696f));  // 37.201f, 47.427f, 37.438f, 47.286f, 37.654f, 47.103f
path.cubicTo(SkBits2Float(0x4216eb84), SkBits2Float(0x423c26e0), SkBits2Float(0x4217df3a), SkBits2Float(0x423afbde), SkBits2Float(0x4218580f), SkBits2Float(0x423b75b9));  // 37.73f, 47.038f, 37.968f, 46.746f, 38.086f, 46.865f
path.cubicTo(SkBits2Float(0x42189a9e), SkBits2Float(0x423bad05), SkBits2Float(0x421820c3), SkBits2Float(0x423c1b9c), SkBits2Float(0x4217ffff), SkBits2Float(0x423c311d));  // 38.151f, 46.919f, 38.032f, 47.027f, 38, 47.048f
path.cubicTo(SkBits2Float(0x4217a6e8), SkBits2Float(0x423c9577), SkBits2Float(0x42173851), SkBits2Float(0x423ced87), SkBits2Float(0x4216cac0), SkBits2Float(0x423d5c1f));  // 37.913f, 47.146f, 37.805f, 47.232f, 37.698f, 47.34f
path.cubicTo(SkBits2Float(0x42168831), SkBits2Float(0x423d9eae), SkBits2Float(0x421650e5), SkBits2Float(0x423deb7b), SkBits2Float(0x4215f7ce), SkBits2Float(0x423e23cd));  // 37.633f, 47.405f, 37.579f, 47.48f, 37.492f, 47.535f
path.lineTo(SkBits2Float(0x4215f7ce), SkBits2Float(0x423e4492));  // 37.492f, 47.567f
path.cubicTo(SkBits2Float(0x4215ed91), SkBits2Float(0x423e4fd6), SkBits2Float(0x4215d709), SkBits2Float(0x423e4492), SkBits2Float(0x4215cbc6), SkBits2Float(0x423e4fd6));  // 37.482f, 47.578f, 37.46f, 47.567f, 37.449f, 47.578f
path.cubicTo(SkBits2Float(0x42158937), SkBits2Float(0x423e8722), SkBits2Float(0x42153126), SkBits2Float(0x423f00fd), SkBits2Float(0x4214ee97), SkBits2Float(0x423f3849));  // 37.384f, 47.632f, 37.298f, 47.751f, 37.233f, 47.805f
path.cubicTo(SkBits2Float(0x4214d810), SkBits2Float(0x423f438d), SkBits2Float(0x4214cdd2), SkBits2Float(0x423f590e), SkBits2Float(0x4214cdd2), SkBits2Float(0x423f590e));  // 37.211f, 47.816f, 37.201f, 47.837f, 37.201f, 47.837f
path.lineTo(SkBits2Float(0x4214b74b), SkBits2Float(0x423f590e));  // 37.179f, 47.837f
path.lineTo(SkBits2Float(0x4214b74b), SkBits2Float(0x423f79d3));  // 37.179f, 47.869f
path.cubicTo(SkBits2Float(0x42147fff), SkBits2Float(0x423f905a), SkBits2Float(0x421474bc), SkBits2Float(0x423fb11f), SkBits2Float(0x421448b3), SkBits2Float(0x423fc7a6));  // 37.125f, 47.891f, 37.114f, 47.923f, 37.071f, 47.945f
path.lineTo(SkBits2Float(0x421448b3), SkBits2Float(0x423fdd27));  // 37.071f, 47.966f
path.lineTo(SkBits2Float(0x42143332), SkBits2Float(0x423fdd27));  // 37.05f, 47.966f
path.lineTo(SkBits2Float(0x4213b957), SkBits2Float(0x424077c7));  // 36.931f, 48.117f
path.cubicTo(SkBits2Float(0x4213a2d0), SkBits2Float(0x4240830b), SkBits2Float(0x4213b957), SkBits2Float(0x4240988c), SkBits2Float(0x4213b957), SkBits2Float(0x4240988c));  // 36.909f, 48.128f, 36.931f, 48.149f, 36.931f, 48.149f
path.cubicTo(SkBits2Float(0x4213c49b), SkBits2Float(0x4240988c), SkBits2Float(0x4213b957), SkBits2Float(0x4240ba57), SkBits2Float(0x4213da1c), SkBits2Float(0x4240af13));  // 36.942f, 48.149f, 36.931f, 48.182f, 36.963f, 48.171f
path.cubicTo(SkBits2Float(0x42141cab), SkBits2Float(0x4240af13), SkBits2Float(0x4214a1ca), SkBits2Float(0x42405703), SkBits2Float(0x4214ee97), SkBits2Float(0x42403538));  // 37.028f, 48.171f, 37.158f, 48.085f, 37.233f, 48.052f
path.cubicTo(SkBits2Float(0x42153126), SkBits2Float(0x42401473), SkBits2Float(0x42157ef9), SkBits2Float(0x423ffdec), SkBits2Float(0x4215cbc6), SkBits2Float(0x423fd1e4));  // 37.298f, 48.02f, 37.374f, 47.998f, 37.449f, 47.955f
path.cubicTo(SkBits2Float(0x421650e5), SkBits2Float(0x423f8f55), SkBits2Float(0x4216cac0), SkBits2Float(0x423f4288), SkBits2Float(0x42178624), SkBits2Float(0x423f20bd));  // 37.579f, 47.89f, 37.698f, 47.815f, 37.881f, 47.782f
path.cubicTo(SkBits2Float(0x42177ae0), SkBits2Float(0x423f8f55), SkBits2Float(0x421770a3), SkBits2Float(0x423fc6a0), SkBits2Float(0x42174395), SkBits2Float(0x423ffdec));  // 37.87f, 47.89f, 37.86f, 47.944f, 37.816f, 47.998f
path.cubicTo(SkBits2Float(0x4216bf7c), SkBits2Float(0x4240ba56), SkBits2Float(0x4215ab02), SkBits2Float(0x4241332b), SkBits2Float(0x4214f9db), SkBits2Float(0x4241c38e));  // 37.687f, 48.182f, 37.417f, 48.3f, 37.244f, 48.441f
path.cubicTo(SkBits2Float(0x42143333), SkBits2Float(0x424274b5), SkBits2Float(0x42136b85), SkBits2Float(0x42433019), SkBits2Float(0x4212c5a2), SkBits2Float(0x4243f7c7));  // 37.05f, 48.614f, 36.855f, 48.797f, 36.693f, 48.992f
path.cubicTo(SkBits2Float(0x42115917), SkBits2Float(0x42459b9e), SkBits2Float(0x421022d1), SkBits2Float(0x42476c83), SkBits2Float(0x420f0313), SkBits2Float(0x4249311f));  // 36.337f, 49.402f, 36.034f, 49.856f, 35.753f, 50.298f
path.cubicTo(SkBits2Float(0x420e1ba6), SkBits2Float(0x424a936d), SkBits2Float(0x420d75c3), SkBits2Float(0x424c21c3), SkBits2Float(0x420cdb23), SkBits2Float(0x424dba56));  // 35.527f, 50.644f, 35.365f, 51.033f, 35.214f, 51.432f
path.cubicTo(SkBits2Float(0x420c3f7d), SkBits2Float(0x424f6a77), SkBits2Float(0x420b8419), SkBits2Float(0x42510e4e), SkBits2Float(0x420b1fbf), SkBits2Float(0x4252d3f0));  // 35.062f, 51.854f, 34.879f, 52.264f, 34.781f, 52.707f
path.cubicTo(SkBits2Float(0x420ad2f2), SkBits2Float(0x42548e4e), SkBits2Float(0x420ab127), SkBits2Float(0x42565e2d), SkBits2Float(0x420a9063), SkBits2Float(0x4258188c));  // 34.706f, 53.139f, 34.673f, 53.592f, 34.641f, 54.024f
path.cubicTo(SkBits2Float(0x420a7ae2), SkBits2Float(0x4258882a), SkBits2Float(0x420a9ba7), SkBits2Float(0x4258e03a), SkBits2Float(0x420a9ba7), SkBits2Float(0x42594ed2));  // 34.62f, 54.133f, 34.652f, 54.219f, 34.652f, 54.327f
path.cubicTo(SkBits2Float(0x420aa6eb), SkBits2Float(0x425e301a), SkBits2Float(0x420c820d), SkBits2Float(0x4262c495), SkBits2Float(0x420ecbc8), SkBits2Float(0x4266fff9));  // 34.663f, 55.547f, 35.127f, 56.692f, 35.699f, 57.75f
path.cubicTo(SkBits2Float(0x420eed93), SkBits2Float(0x426721c4), SkBits2Float(0x420f0e57), SkBits2Float(0x42674dcc), SkBits2Float(0x420f3022), SkBits2Float(0x42676e91));  // 35.732f, 57.783f, 35.764f, 57.826f, 35.797f, 57.858f
path.cubicTo(SkBits2Float(0x420f7df5), SkBits2Float(0x42680a37), SkBits2Float(0x420fbf7e), SkBits2Float(0x42689993), SkBits2Float(0x42100d51), SkBits2Float(0x42693433));  // 35.873f, 58.01f, 35.937f, 58.15f, 36.013f, 58.301f
path.cubicTo(SkBits2Float(0x42102e16), SkBits2Float(0x426955fe), SkBits2Float(0x42105a1e), SkBits2Float(0x426976c2), SkBits2Float(0x42106561), SkBits2Float(0x42698d4a));  // 36.045f, 58.334f, 36.088f, 58.366f, 36.099f, 58.388f
path.cubicTo(SkBits2Float(0x4210872c), SkBits2Float(0x4269e55a), SkBits2Float(0x4210a7f0), SkBits2Float(0x426a3d6b), SkBits2Float(0x4210ea80), SkBits2Float(0x426a6a79));  // 36.132f, 58.474f, 36.164f, 58.56f, 36.229f, 58.604f
path.cubicTo(SkBits2Float(0x42119aa1), SkBits2Float(0x426acdcd), SkBits2Float(0x42131376), SkBits2Float(0x426a48ae), SkBits2Float(0x4213e561), SkBits2Float(0x426a6a79));  // 36.401f, 58.701f, 36.769f, 58.571f, 36.974f, 58.604f
path.cubicTo(SkBits2Float(0x4213fae2), SkBits2Float(0x426a75bd), SkBits2Float(0x42141cad), SkBits2Float(0x426a8b3e), SkBits2Float(0x42143d71), SkBits2Float(0x426a8b3e));  // 36.995f, 58.615f, 37.028f, 58.636f, 37.06f, 58.636f
path.cubicTo(SkBits2Float(0x42141cac), SkBits2Float(0x426acdcd), SkBits2Float(0x42143334), SkBits2Float(0x426aee92), SkBits2Float(0x42141cac), SkBits2Float(0x426b25de));  // 37.028f, 58.701f, 37.05f, 58.733f, 37.028f, 58.787f
path.cubicTo(SkBits2Float(0x4213e560), SkBits2Float(0x426b9fb9), SkBits2Float(0x4212dc29), SkBits2Float(0x426d0d4b), SkBits2Float(0x4212f1aa), SkBits2Float(0x426da7ea));  // 36.974f, 58.906f, 36.715f, 59.263f, 36.736f, 59.414f
path.cubicTo(SkBits2Float(0x4212f1aa), SkBits2Float(0x426dfffa), SkBits2Float(0x4213b958), SkBits2Float(0x426ed1e6), SkBits2Float(0x4213c49c), SkBits2Float(0x426edd29));  // 36.736f, 59.5f, 36.931f, 59.705f, 36.942f, 59.716f
path.cubicTo(SkBits2Float(0x4213e561), SkBits2Float(0x426f1fb8), SkBits2Float(0x42143d71), SkBits2Float(0x426f9993), SkBits2Float(0x421448b5), SkBits2Float(0x426ffce7));  // 36.974f, 59.781f, 37.06f, 59.9f, 37.071f, 59.997f
path.cubicTo(SkBits2Float(0x421448b5), SkBits2Float(0x427076c2), SkBits2Float(0x4214072c), SkBits2Float(0x4270ef97), SkBits2Float(0x4213fae2), SkBits2Float(0x427148ae));  // 37.071f, 60.116f, 37.007f, 60.234f, 36.995f, 60.321f
path.cubicTo(SkBits2Float(0x4213e561), SkBits2Float(0x42717ffa), SkBits2Float(0x4213fae2), SkBits2Float(0x42718b3d), SkBits2Float(0x4213e561), SkBits2Float(0x4271b746));  // 36.974f, 60.375f, 36.995f, 60.386f, 36.974f, 60.429f
path.cubicTo(SkBits2Float(0x4213da1d), SkBits2Float(0x4271ccc7), SkBits2Float(0x4213b959), SkBits2Float(0x42721a9a), SkBits2Float(0x4213a2d2), SkBits2Float(0x42721a9a));  // 36.963f, 60.45f, 36.931f, 60.526f, 36.909f, 60.526f
path.cubicTo(SkBits2Float(0x42134ac2), SkBits2Float(0x42723c65), SkBits2Float(0x4212d0e6), SkBits2Float(0x427225de), SkBits2Float(0x42126d93), SkBits2Float(0x427225de));  // 36.823f, 60.559f, 36.704f, 60.537f, 36.607f, 60.537f
path.cubicTo(SkBits2Float(0x42124bc8), SkBits2Float(0x427225de), SkBits2Float(0x4211bc6c), SkBits2Float(0x42723c65), SkBits2Float(0x42119064), SkBits2Float(0x42723c65));  // 36.574f, 60.537f, 36.434f, 60.559f, 36.391f, 60.559f
path.cubicTo(SkBits2Float(0x4210d3fa), SkBits2Float(0x427246a2), SkBits2Float(0x420ff6ca), SkBits2Float(0x4272301b), SkBits2Float(0x420f676e), SkBits2Float(0x4272686d));  // 36.207f, 60.569f, 35.991f, 60.547f, 35.851f, 60.602f
path.cubicTo(SkBits2Float(0x420eb647), SkBits2Float(0x4272b53a), SkBits2Float(0x420e52f3), SkBits2Float(0x42737ce8), SkBits2Float(0x420dc291), SkBits2Float(0x4273f5bd));  // 35.678f, 60.677f, 35.581f, 60.872f, 35.44f, 60.99f
path.cubicTo(SkBits2Float(0x420d116a), SkBits2Float(0x4274861f), SkBits2Float(0x420c5606), SkBits2Float(0x4274e973), SkBits2Float(0x420b999b), SkBits2Float(0x4275580b));  // 35.267f, 61.131f, 35.084f, 61.228f, 34.9f, 61.336f
path.cubicTo(SkBits2Float(0x420a9ba7), SkBits2Float(0x4275fdee), SkBits2Float(0x4209b335), SkBits2Float(0x42768d4a), SkBits2Float(0x42089eba), SkBits2Float(0x4276f1a5));  // 34.652f, 61.498f, 34.425f, 61.638f, 34.155f, 61.736f
path.cubicTo(SkBits2Float(0x4207ab04), SkBits2Float(0x42773e72), SkBits2Float(0x4206a1cc), SkBits2Float(0x42778101), SkBits2Float(0x4205b95a), SkBits2Float(0x4277c391));  // 33.917f, 61.811f, 33.658f, 61.876f, 33.431f, 61.941f
path.cubicTo(SkBits2Float(0x4203bd73), SkBits2Float(0x42786974), SkBits2Float(0x4201cbc9), SkBits2Float(0x42793b60), SkBits2Float(0x4200ac0a), SkBits2Float(0x427af5be));  // 32.935f, 62.103f, 32.449f, 62.308f, 32.168f, 62.74f
path.cubicTo(SkBits2Float(0x420074be), SkBits2Float(0x427b428b), SkBits2Float(0x41ffb43d), SkBits2Float(0x427c4cc8), SkBits2Float(0x41ff872f), SkBits2Float(0x427ca4d9));  // 32.114f, 62.815f, 31.963f, 63.075f, 31.941f, 63.161f
path.cubicTo(SkBits2Float(0x41ff872f), SkBits2Float(0x427cbb60), SkBits2Float(0x41ff9db6), SkBits2Float(0x427cd0e1), SkBits2Float(0x41ff872f), SkBits2Float(0x427ce768));  // 31.941f, 63.183f, 31.952f, 63.204f, 31.941f, 63.226f
path.cubicTo(SkBits2Float(0x41ffb43d), SkBits2Float(0x427cfce9), SkBits2Float(0x41ffb43d), SkBits2Float(0x427cfce9), SkBits2Float(0x41ffdf3f), SkBits2Float(0x427d1370));  // 31.963f, 63.247f, 31.963f, 63.247f, 31.984f, 63.269f
path.cubicTo(SkBits2Float(0x4200ac0a), SkBits2Float(0x427cfce9), SkBits2Float(0x42010f5e), SkBits2Float(0x427cd0e1), SkBits2Float(0x4201a9fe), SkBits2Float(0x427ca4d8));  // 32.168f, 63.247f, 32.265f, 63.204f, 32.416f, 63.161f
path.cubicTo(SkBits2Float(0x4201c085), SkBits2Float(0x427c9994), SkBits2Float(0x4201f7d1), SkBits2Float(0x427c78d0), SkBits2Float(0x42020315), SkBits2Float(0x427c78d0));  // 32.438f, 63.15f, 32.492f, 63.118f, 32.503f, 63.118f
path.cubicTo(SkBits2Float(0x420223da), SkBits2Float(0x427c6249), SkBits2Float(0x42022f1d), SkBits2Float(0x427c78d0), SkBits2Float(0x42023a61), SkBits2Float(0x427c78d0));  // 32.535f, 63.096f, 32.546f, 63.118f, 32.557f, 63.118f
path.cubicTo(SkBits2Float(0x42025b26), SkBits2Float(0x427c6249), SkBits2Float(0x42028834), SkBits2Float(0x427c4184), SkBits2Float(0x4202a8f9), SkBits2Float(0x427c4184));  // 32.589f, 63.096f, 32.633f, 63.064f, 32.665f, 63.064f
path.cubicTo(SkBits2Float(0x4203e97c), SkBits2Float(0x427bc7a9), SkBits2Float(0x42061db5), SkBits2Float(0x427ba6e4), SkBits2Float(0x4207b649), SkBits2Float(0x427bfef5));  // 32.978f, 62.945f, 33.529f, 62.913f, 33.928f, 62.999f
path.cubicTo(SkBits2Float(0x42089ebc), SkBits2Float(0x427c20c0), SkBits2Float(0x420970a7), SkBits2Float(0x427c78d0), SkBits2Float(0x420a21ce), SkBits2Float(0x427cc59d));  // 34.155f, 63.032f, 34.36f, 63.118f, 34.533f, 63.193f
path.cubicTo(SkBits2Float(0x420a6fa1), SkBits2Float(0x427cdc24), SkBits2Float(0x420ab12a), SkBits2Float(0x427ce768), SkBits2Float(0x420af3ba), SkBits2Float(0x427d1370));  // 34.609f, 63.215f, 34.673f, 63.226f, 34.738f, 63.269f
path.cubicTo(SkBits2Float(0x420b0a41), SkBits2Float(0x427d1370), SkBits2Float(0x420af3ba), SkBits2Float(0x427d353b), SkBits2Float(0x420b1585), SkBits2Float(0x427d407e));  // 34.76f, 63.269f, 34.738f, 63.302f, 34.771f, 63.313f
path.cubicTo(SkBits2Float(0x420b0a41), SkBits2Float(0x427d6143), SkBits2Float(0x420b0a41), SkBits2Float(0x427d8207), SkBits2Float(0x420af3ba), SkBits2Float(0x427dae0f));  // 34.76f, 63.345f, 34.76f, 63.377f, 34.738f, 63.42f
path.cubicTo(SkBits2Float(0x420ad2f5), SkBits2Float(0x427df09e), SkBits2Float(0x420a2d12), SkBits2Float(0x427e54f8), SkBits2Float(0x420a4293), SkBits2Float(0x427ee455));  // 34.706f, 63.485f, 34.544f, 63.583f, 34.565f, 63.723f
path.cubicTo(SkBits2Float(0x420a591a), SkBits2Float(0x427f051a), SkBits2Float(0x420ad2f5), SkBits2Float(0x427f3122), SkBits2Float(0x420af3ba), SkBits2Float(0x427f47a9));  // 34.587f, 63.755f, 34.706f, 63.798f, 34.738f, 63.82f
path.cubicTo(SkBits2Float(0x420af3ba), SkBits2Float(0x427f5d2a), SkBits2Float(0x420af3ba), SkBits2Float(0x427f73b1), SkBits2Float(0x420b0a41), SkBits2Float(0x427f7ef5));  // 34.738f, 63.841f, 34.738f, 63.863f, 34.76f, 63.874f
path.cubicTo(SkBits2Float(0x420add33), SkBits2Float(0x427fccc8), SkBits2Float(0x420a21ce), SkBits2Float(0x42803e74), SkBits2Float(0x420a2d12), SkBits2Float(0x4280701e));  // 34.716f, 63.95f, 34.533f, 64.122f, 34.544f, 64.219f
path.cubicTo(SkBits2Float(0x420a3856), SkBits2Float(0x42808bc4), SkBits2Float(0x420ad2f5), SkBits2Float(0x4280a7ed), SkBits2Float(0x420ae876), SkBits2Float(0x4280b2ad));  // 34.555f, 64.273f, 34.706f, 64.328f, 34.727f, 64.349f
path.cubicTo(SkBits2Float(0x420af3ba), SkBits2Float(0x4280bdf1), SkBits2Float(0x420add32), SkBits2Float(0x4280c8b1), SkBits2Float(0x420af3ba), SkBits2Float(0x4280d3f5));  // 34.738f, 64.371f, 34.716f, 64.392f, 34.738f, 64.414f
path.cubicTo(SkBits2Float(0x420abd74), SkBits2Float(0x4280f53d), SkBits2Float(0x4209f5c6), SkBits2Float(0x4281428d), SkBits2Float(0x420a21ce), SkBits2Float(0x42816e95));  // 34.685f, 64.479f, 34.49f, 64.63f, 34.533f, 64.716f
path.cubicTo(SkBits2Float(0x420a4293), SkBits2Float(0x4281957e), SkBits2Float(0x420ad2f5), SkBits2Float(0x4281a664), SkBits2Float(0x420ae876), SkBits2Float(0x4281c187));  // 34.565f, 64.792f, 34.706f, 64.825f, 34.727f, 64.878f
path.cubicTo(SkBits2Float(0x420ae876), SkBits2Float(0x4281c729), SkBits2Float(0x420add32), SkBits2Float(0x4281d26c), SkBits2Float(0x420ae876), SkBits2Float(0x4281d26c));  // 34.727f, 64.889f, 34.716f, 64.911f, 34.727f, 64.911f
path.cubicTo(SkBits2Float(0x420aa6ed), SkBits2Float(0x4281fe74), SkBits2Float(0x420a591a), SkBits2Float(0x42821a1a), SkBits2Float(0x4209f5c6), SkBits2Float(0x42823b62));  // 34.663f, 64.997f, 34.587f, 65.051f, 34.49f, 65.116f
path.cubicTo(SkBits2Float(0x420a168b), SkBits2Float(0x42825caa), SkBits2Float(0x420a010a), SkBits2Float(0x4282624b), SkBits2Float(0x420a2d12), SkBits2Float(0x42827850));  // 34.522f, 65.181f, 34.501f, 65.192f, 34.544f, 65.235f
path.cubicTo(SkBits2Float(0x420a645e), SkBits2Float(0x428288b2), SkBits2Float(0x420a9baa), SkBits2Float(0x428293f6), SkBits2Float(0x420ad2f5), SkBits2Float(0x4282a458));  // 34.598f, 65.267f, 34.652f, 65.289f, 34.706f, 65.321f
path.cubicTo(SkBits2Float(0x420ad2f5), SkBits2Float(0x4282a458), SkBits2Float(0x420add32), SkBits2Float(0x4282d685), SkBits2Float(0x420abd74), SkBits2Float(0x4282c5a0));  // 34.706f, 65.321f, 34.716f, 65.419f, 34.685f, 65.386f
path.cubicTo(SkBits2Float(0x420aa6ed), SkBits2Float(0x4282cb42), SkBits2Float(0x420a9066), SkBits2Float(0x4282e146), SkBits2Float(0x420a6fa1), SkBits2Float(0x4282e6e8));  // 34.663f, 65.397f, 34.641f, 65.44f, 34.609f, 65.451f
path.cubicTo(SkBits2Float(0x4209df3f), SkBits2Float(0x42830830), SkBits2Float(0x4208f6cc), SkBits2Float(0x4282bae0), SkBits2Float(0x42088834), SkBits2Float(0x4282a459));  // 34.468f, 65.516f, 34.241f, 65.365f, 34.133f, 65.321f
path.cubicTo(SkBits2Float(0x420846ab), SkBits2Float(0x42829915), SkBits2Float(0x42080f5f), SkBits2Float(0x42829915), SkBits2Float(0x4207c18c), SkBits2Float(0x428293f7));  // 34.069f, 65.299f, 34.015f, 65.299f, 33.939f, 65.289f
path.cubicTo(SkBits2Float(0x42079584), SkBits2Float(0x428288b3), SkBits2Float(0x420748b7), SkBits2Float(0x42826d0e), SkBits2Float(0x42071ba9), SkBits2Float(0x42826d0e));  // 33.896f, 65.267f, 33.821f, 65.213f, 33.777f, 65.213f
path.cubicTo(SkBits2Float(0x4206cedc), SkBits2Float(0x4282624d), SkBits2Float(0x42068109), SkBits2Float(0x4282624d), SkBits2Float(0x42061272), SkBits2Float(0x4282624d));  // 33.702f, 65.192f, 33.626f, 65.192f, 33.518f, 65.192f
path.cubicTo(SkBits2Float(0x4205cfe3), SkBits2Float(0x42825cab), SkBits2Float(0x4205614b), SkBits2Float(0x42824bc6), SkBits2Float(0x42051ebc), SkBits2Float(0x42824bc6));  // 33.453f, 65.181f, 33.345f, 65.148f, 33.28f, 65.148f
path.cubicTo(SkBits2Float(0x42037ae5), SkBits2Float(0x428246a7), SkBits2Float(0x4201cbca), SkBits2Float(0x42829eb8), SkBits2Float(0x4200ac0c), SkBits2Float(0x4282e147));  // 32.87f, 65.138f, 32.449f, 65.31f, 32.168f, 65.44f
path.cubicTo(SkBits2Float(0x42008b47), SkBits2Float(0x4282e6e9), SkBits2Float(0x42005e39), SkBits2Float(0x4282fced), SkBits2Float(0x42003d74), SkBits2Float(0x4283028f));  // 32.136f, 65.451f, 32.092f, 65.494f, 32.06f, 65.505f
path.cubicTo(SkBits2Float(0x41fdf9e2), SkBits2Float(0x42833f7d), SkBits2Float(0x41fa4190), SkBits2Float(0x42836041), SkBits2Float(0x41f674c3), SkBits2Float(0x42834fdf));  // 31.747f, 65.624f, 31.282f, 65.688f, 30.807f, 65.656f
path.cubicTo(SkBits2Float(0x41f59794), SkBits2Float(0x4283451e), SkBits2Float(0x41f48d56), SkBits2Float(0x4283451e), SkBits2Float(0x41f3b027), SkBits2Float(0x428339db));  // 30.699f, 65.635f, 30.569f, 65.635f, 30.461f, 65.613f
path.cubicTo(SkBits2Float(0x41f32d15), SkBits2Float(0x42832e97), SkBits2Float(0x41f2666d), SkBits2Float(0x428312f2), SkBits2Float(0x41f1b440), SkBits2Float(0x42830831));  // 30.397f, 65.591f, 30.3f, 65.537f, 30.213f, 65.516f
path.cubicTo(SkBits2Float(0x41f1041f), SkBits2Float(0x4282fced), SkBits2Float(0x41f07f01), SkBits2Float(0x4282f74c), SkBits2Float(0x41efb859), SkBits2Float(0x4282e6e9));  // 30.127f, 65.494f, 30.062f, 65.483f, 29.965f, 65.451f
path.cubicTo(SkBits2Float(0x41efa1d2), SkBits2Float(0x4282e147), SkBits2Float(0x41ef6049), SkBits2Float(0x4282d687), SkBits2Float(0x41ef49c1), SkBits2Float(0x4282d687));  // 29.954f, 65.44f, 29.922f, 65.419f, 29.911f, 65.419f
path.cubicTo(SkBits2Float(0x41ef062b), SkBits2Float(0x4282cb43), SkBits2Float(0x41eec4a2), SkBits2Float(0x4282cb43), SkBits2Float(0x41ee560b), SkBits2Float(0x4282c5a2));  // 29.878f, 65.397f, 29.846f, 65.397f, 29.792f, 65.386f
path.cubicTo(SkBits2Float(0x41ee1275), SkBits2Float(0x4282c000), SkBits2Float(0x41ed8f63), SkBits2Float(0x4282a45a), SkBits2Float(0x41ed3546), SkBits2Float(0x42829eb9));  // 29.759f, 65.375f, 29.695f, 65.321f, 29.651f, 65.31f
path.cubicTo(SkBits2Float(0x41ebbe7d), SkBits2Float(0x42827d71), SkBits2Float(0x41ea72b7), SkBits2Float(0x42825cad), SkBits2Float(0x41e91069), SkBits2Float(0x42823b65));  // 29.468f, 65.245f, 29.306f, 65.181f, 29.133f, 65.116f
path.cubicTo(SkBits2Float(0x41e6fdfa), SkBits2Float(0x42820419), SkBits2Float(0x41e4a7f6), SkBits2Float(0x4281ab86), SkBits2Float(0x41e18b4a), SkBits2Float(0x4281ab86));  // 28.874f, 65.008f, 28.582f, 64.835f, 28.193f, 64.835f
path.cubicTo(SkBits2Float(0x41de9bac), SkBits2Float(0x4281b128), SkBits2Float(0x41dcf7d5), SkBits2Float(0x4281fe78), SkBits2Float(0x41db3d77), SkBits2Float(0x428246a9));  // 27.826f, 64.846f, 27.621f, 64.997f, 27.405f, 65.138f
path.cubicTo(SkBits2Float(0x41dacedf), SkBits2Float(0x4282570b), SkBits2Float(0x41da76cf), SkBits2Float(0x4282570b), SkBits2Float(0x41da0838), SkBits2Float(0x4282676e));  // 27.351f, 65.17f, 27.308f, 65.17f, 27.254f, 65.202f
path.cubicTo(SkBits2Float(0x41d9f1b1), SkBits2Float(0x4282676e), SkBits2Float(0x41d9f1b1), SkBits2Float(0x42827853), SkBits2Float(0x41d9db2a), SkBits2Float(0x42827d72));  // 27.243f, 65.202f, 27.243f, 65.235f, 27.232f, 65.245f
path.cubicTo(SkBits2Float(0x41d96c92), SkBits2Float(0x428288b6), SkBits2Float(0x41d91482), SkBits2Float(0x428288b6), SkBits2Float(0x41d8a5eb), SkBits2Float(0x42829eba));  // 27.178f, 65.267f, 27.135f, 65.267f, 27.081f, 65.31f
path.lineTo(SkBits2Float(0x41d88f64), SkBits2Float(0x4282a9fe));  // 27.07f, 65.332f
path.cubicTo(SkBits2Float(0x41d6eb8d), SkBits2Float(0x4282e14a), SkBits2Float(0x41d4ac10), SkBits2Float(0x42830291), SkBits2Float(0x41d25818), SkBits2Float(0x428312f4));  // 26.865f, 65.44f, 26.584f, 65.505f, 26.293f, 65.537f
path.cubicTo(SkBits2Float(0x41d0b235), SkBits2Float(0x42831896), SkBits2Float(0x41ce74c4), SkBits2Float(0x428312f4), SkBits2Float(0x41cce568), SkBits2Float(0x42830292));  // 26.087f, 65.548f, 25.807f, 65.537f, 25.612f, 65.505f
path.cubicTo(SkBits2Float(0x41cca3df), SkBits2Float(0x4282fcf0), SkBits2Float(0x41cc1ec0), SkBits2Float(0x4282f1ad), SkBits2Float(0x41cbf3be), SkBits2Float(0x4282f1ad));  // 25.58f, 65.494f, 25.515f, 65.472f, 25.494f, 65.472f
path.cubicTo(SkBits2Float(0x41ca9170), SkBits2Float(0x4282dba9), SkBits2Float(0x41c99dba), SkBits2Float(0x4282e14b), SkBits2Float(0x41c8687a), SkBits2Float(0x4282cb47));  // 25.321f, 65.429f, 25.202f, 65.44f, 25.051f, 65.397f
path.cubicTo(SkBits2Float(0x41c7b64d), SkBits2Float(0x4282c003), SkBits2Float(0x41c71cb3), SkBits2Float(0x4282bae5), SkBits2Float(0x41c6560b), SkBits2Float(0x4282a9ff));  // 24.964f, 65.375f, 24.889f, 65.365f, 24.792f, 65.332f
path.lineTo(SkBits2Float(0x41c628fd), SkBits2Float(0x42829ebb));  // 24.77f, 65.31f
path.cubicTo(SkBits2Float(0x41c58d57), SkBits2Float(0x428293fa), SkBits2Float(0x41c53547), SkBits2Float(0x42829919), SkBits2Float(0x41c4b028), SkBits2Float(0x428293fa));  // 24.694f, 65.289f, 24.651f, 65.299f, 24.586f, 65.289f
path.lineTo(SkBits2Float(0x41c46e9f), SkBits2Float(0x42828315));  // 24.554f, 65.256f
path.cubicTo(SkBits2Float(0x41c1d712), SkBits2Float(0x4282570d), SkBits2Float(0x41be20cc), SkBits2Float(0x428209bd), SkBits2Float(0x41bb0420), SkBits2Float(0x42820f5f));  // 24.23f, 65.17f, 23.766f, 65.019f, 23.377f, 65.03f
path.cubicTo(SkBits2Float(0x41b9a1d2), SkBits2Float(0x42820f5f), SkBits2Float(0x41b7e774), SkBits2Float(0x42823024), SkBits2Float(0x41b6dd37), SkBits2Float(0x428246ab));  // 23.204f, 65.03f, 22.988f, 65.094f, 22.858f, 65.138f
path.cubicTo(SkBits2Float(0x41b5eb8d), SkBits2Float(0x4282570d), SkBits2Float(0x41b54fe7), SkBits2Float(0x4282570d), SkBits2Float(0x41b45c31), SkBits2Float(0x42826770));  // 22.74f, 65.17f, 22.664f, 65.17f, 22.545f, 65.202f
path.cubicTo(SkBits2Float(0x41b3ed99), SkBits2Float(0x42826d12), SkBits2Float(0x41b35400), SkBits2Float(0x428288b8), SkBits2Float(0x41b2fbef), SkBits2Float(0x428293fb));  // 22.491f, 65.213f, 22.416f, 65.267f, 22.373f, 65.289f
path.cubicTo(SkBits2Float(0x41b274c4), SkBits2Float(0x4282991a), SkBits2Float(0x41b249c2), SkBits2Float(0x428293fb), SkBits2Float(0x41b1c4a3), SkBits2Float(0x42829ebc));  // 22.307f, 65.299f, 22.286f, 65.289f, 22.221f, 65.31f
path.cubicTo(SkBits2Float(0x41b1560b), SkBits2Float(0x4282a45e), SkBits2Float(0x41b08f64), SkBits2Float(0x4282c004), SkBits2Float(0x41aff3be), SkBits2Float(0x4282cb47));  // 22.167f, 65.321f, 22.07f, 65.375f, 21.994f, 65.397f
path.cubicTo(SkBits2Float(0x41aea7f7), SkBits2Float(0x4282e14b), SkBits2Float(0x41ad893f), SkBits2Float(0x4282f1ad), SkBits2Float(0x41ac3d78), SkBits2Float(0x42830835));  // 21.832f, 65.44f, 21.692f, 65.472f, 21.53f, 65.516f
path.cubicTo(SkBits2Float(0x41ac106a), SkBits2Float(0x428312f6), SkBits2Float(0x41aba1d2), SkBits2Float(0x42831e39), SkBits2Float(0x41ab76d0), SkBits2Float(0x428323db));  // 21.508f, 65.537f, 21.454f, 65.559f, 21.433f, 65.57f
path.cubicTo(SkBits2Float(0x41aac4a3), SkBits2Float(0x4283343d), SkBits2Float(0x41aa560b), SkBits2Float(0x4283343d), SkBits2Float(0x41a9ba66), SkBits2Float(0x42833f81));  // 21.346f, 65.602f, 21.292f, 65.602f, 21.216f, 65.624f
path.lineTo(SkBits2Float(0x41a98f64), SkBits2Float(0x42834fe3));  // 21.195f, 65.656f
path.cubicTo(SkBits2Float(0x41a96256), SkBits2Float(0x42834fe3), SkBits2Float(0x41a93754), SkBits2Float(0x42834522), SkBits2Float(0x41a920cc), SkBits2Float(0x42834fe3));  // 21.173f, 65.656f, 21.152f, 65.635f, 21.141f, 65.656f
path.cubicTo(SkBits2Float(0x41a90a45), SkBits2Float(0x42834fe3), SkBits2Float(0x41a8b234), SkBits2Float(0x42836045), SkBits2Float(0x41a89bad), SkBits2Float(0x42836b89));  // 21.13f, 65.656f, 21.087f, 65.688f, 21.076f, 65.71f
path.cubicTo(SkBits2Float(0x41a7d505), SkBits2Float(0x42837beb), SkBits2Float(0x41a7666e), SkBits2Float(0x4283818d), SkBits2Float(0x41a6cac8), SkBits2Float(0x42839d33));  // 20.979f, 65.742f, 20.925f, 65.753f, 20.849f, 65.807f
path.cubicTo(SkBits2Float(0x41a6b64d), SkBits2Float(0x4283a2d5), SkBits2Float(0x41a672b8), SkBits2Float(0x4283b3ba), SkBits2Float(0x41a65e3d), SkBits2Float(0x4283b8d9));  // 20.839f, 65.818f, 20.806f, 65.851f, 20.796f, 65.861f
path.cubicTo(SkBits2Float(0x41a6312f), SkBits2Float(0x4283be7b), SkBits2Float(0x41a60420), SkBits2Float(0x4283b8d9), SkBits2Float(0x41a5ed99), SkBits2Float(0x4283be7b));  // 20.774f, 65.872f, 20.752f, 65.861f, 20.741f, 65.872f
path.cubicTo(SkBits2Float(0x41a5810e), SkBits2Float(0x4283cedd), SkBits2Float(0x41a4e568), SkBits2Float(0x428406ac), SkBits2Float(0x41a48d57), SkBits2Float(0x42840bcb));  // 20.688f, 65.904f, 20.612f, 66.013f, 20.569f, 66.023f
path.lineTo(SkBits2Float(0x41a41ebf), SkBits2Float(0x42840bcb));  // 20.515f, 66.023f
path.cubicTo(SkBits2Float(0x41a40838), SkBits2Float(0x4283fb69), SkBits2Float(0x41a3f1b1), SkBits2Float(0x428406ac), SkBits2Float(0x41a3f1b1), SkBits2Float(0x4283fb69));  // 20.504f, 65.991f, 20.493f, 66.013f, 20.493f, 65.991f
path.cubicTo(SkBits2Float(0x41a38319), SkBits2Float(0x4283b8da), SkBits2Float(0x41a4b859), SkBits2Float(0x4282f750), SkBits2Float(0x41a4e567), SkBits2Float(0x4282cb48));  // 20.439f, 65.861f, 20.59f, 65.483f, 20.612f, 65.397f
path.cubicTo(SkBits2Float(0x41a5ed98), SkBits2Float(0x4281d273), SkBits2Float(0x41a74fe6), SkBits2Float(0x4280ea00), SkBits2Float(0x41a96255), SkBits2Float(0x42802e19));  // 20.741f, 64.911f, 20.914f, 64.457f, 21.173f, 64.09f
path.cubicTo(SkBits2Float(0x41aa2b09), SkBits2Float(0x427fccd6), SkBits2Float(0x41ab1ebf), SkBits2Float(0x427f6982), SkBits2Float(0x41abfbef), SkBits2Float(0x427eefa7));  // 21.271f, 63.95f, 21.39f, 63.853f, 21.498f, 63.734f
path.cubicTo(SkBits2Float(0x41ac7f01), SkBits2Float(0x427e9690), SkBits2Float(0x41aced99), SkBits2Float(0x427e49c4), SkBits2Float(0x41ad893f), SkBits2Float(0x427e0734));  // 21.562f, 63.647f, 21.616f, 63.572f, 21.692f, 63.507f
path.cubicTo(SkBits2Float(0x41aed506), SkBits2Float(0x427d8215), SkBits2Float(0x41b020cc), SkBits2Float(0x427d137e), SkBits2Float(0x41b1831a), SkBits2Float(0x427cbb6d));  // 21.854f, 63.377f, 22.016f, 63.269f, 22.189f, 63.183f
path.cubicTo(SkBits2Float(0x41b1f1b2), SkBits2Float(0x427c99a2), SkBits2Float(0x41b26049), SkBits2Float(0x427c6256), SkBits2Float(0x41b2cee1), SkBits2Float(0x427c4cd5));  // 22.243f, 63.15f, 22.297f, 63.096f, 22.351f, 63.075f
path.cubicTo(SkBits2Float(0x41b3106a), SkBits2Float(0x427c2b0a), SkBits2Float(0x41b445aa), SkBits2Float(0x427bff02), SkBits2Float(0x41b49dba), SkBits2Float(0x427bde3d));  // 22.383f, 63.042f, 22.534f, 62.999f, 22.577f, 62.967f
path.cubicTo(SkBits2Float(0x41b49dba), SkBits2Float(0x427bd2f9), SkBits2Float(0x41b4cac8), SkBits2Float(0x427ba6f1), SkBits2Float(0x41b4cac8), SkBits2Float(0x427ba6f1));  // 22.577f, 62.956f, 22.599f, 62.913f, 22.599f, 62.913f
path.cubicTo(SkBits2Float(0x41b4cac8), SkBits2Float(0x427b6462), SkBits2Float(0x41b33b6c), SkBits2Float(0x427a4fe7), SkBits2Float(0x41b2fbef), SkBits2Float(0x427a189b));  // 22.599f, 62.848f, 22.404f, 62.578f, 22.373f, 62.524f
path.cubicTo(SkBits2Float(0x41b2cee1), SkBits2Float(0x4279f7d6), SkBits2Float(0x41b19795), SkBits2Float(0x42796774), SkBits2Float(0x41b1560c), SkBits2Float(0x42795c31));  // 22.351f, 62.492f, 22.199f, 62.351f, 22.167f, 62.34f
path.cubicTo(SkBits2Float(0x41b0e774), SkBits2Float(0x42793b6c), SkBits2Float(0x41aff3be), SkBits2Float(0x42795c31), SkBits2Float(0x41af70ac), SkBits2Float(0x42795c31));  // 22.113f, 62.308f, 21.994f, 62.34f, 21.93f, 62.34f
path.cubicTo(SkBits2Float(0x41ae0e5e), SkBits2Float(0x42796775), SkBits2Float(0x41ac9589), SkBits2Float(0x427946b0), SkBits2Float(0x41ab76d1), SkBits2Float(0x42793b6c));  // 21.757f, 62.351f, 21.573f, 62.319f, 21.433f, 62.308f
path.cubicTo(SkBits2Float(0x41aa3f85), SkBits2Float(0x42793028), SkBits2Float(0x41a94ddb), SkBits2Float(0x42793b6c), SkBits2Float(0x41a82d17), SkBits2Float(0x42793028));  // 21.281f, 62.297f, 21.163f, 62.308f, 21.022f, 62.297f
path.cubicTo(SkBits2Float(0x41a5c298), SkBits2Float(0x42791aa7), SkBits2Float(0x41a2e775), SkBits2Float(0x4278ed99), SkBits2Float(0x41a07cf6), SkBits2Float(0x4278c190));  // 20.72f, 62.276f, 20.363f, 62.232f, 20.061f, 62.189f
path.cubicTo(SkBits2Float(0x419f47b7), SkBits2Float(0x4278b753), SkBits2Float(0x419e810f), SkBits2Float(0x4278b753), SkBits2Float(0x419d4bcf), SkBits2Float(0x4278a0cb));  // 19.91f, 62.179f, 19.813f, 62.179f, 19.662f, 62.157f
path.cubicTo(SkBits2Float(0x419c831b), SkBits2Float(0x42788a44), SkBits2Float(0x419b20cd), SkBits2Float(0x42785e3c), SkBits2Float(0x419a45aa), SkBits2Float(0x427847b4));  // 19.564f, 62.135f, 19.391f, 62.092f, 19.284f, 62.07f
path.cubicTo(SkBits2Float(0x41949171), SkBits2Float(0x4277e460), SkBits2Float(0x418e5819), SkBits2Float(0x42778c50), SkBits2Float(0x41896a87), SkBits2Float(0x4275dd35));  // 18.571f, 61.973f, 17.793f, 61.887f, 17.177f, 61.466f
path.cubicTo(SkBits2Float(0x4182efa6), SkBits2Float(0x4273a8fc), SkBits2Float(0x417fd71a), SkBits2Float(0x42703f83), SkBits2Float(0x4180dd37), SkBits2Float(0x426c5b29));  // 16.367f, 60.915f, 15.99f, 60.062f, 16.108f, 59.089f
path.cubicTo(SkBits2Float(0x41813547), SkBits2Float(0x426b5d35), SkBits2Float(0x41821276), SkBits2Float(0x426a8006), SkBits2Float(0x4182560c), SkBits2Float(0x426976cf));  // 16.151f, 58.841f, 16.259f, 58.625f, 16.292f, 58.366f
path.cubicTo(SkBits2Float(0x418228fe), SkBits2Float(0x426976cf), SkBits2Float(0x41823f85), SkBits2Float(0x42696b8b), SkBits2Float(0x418228fe), SkBits2Float(0x42694ac7));  // 16.27f, 58.366f, 16.281f, 58.355f, 16.27f, 58.323f
path.cubicTo(SkBits2Float(0x4181a5ec), SkBits2Float(0x42696b8c), SkBits2Float(0x41813548), SkBits2Float(0x42696b8c), SkBits2Float(0x41809bae), SkBits2Float(0x4269560b));  // 16.206f, 58.355f, 16.151f, 58.355f, 16.076f, 58.334f
path.cubicTo(SkBits2Float(0x4180b235), SkBits2Float(0x4269560b), SkBits2Float(0x4180439e), SkBits2Float(0x426976d0), SkBits2Float(0x4180168f), SkBits2Float(0x42696b8c));  // 16.087f, 58.334f, 16.033f, 58.366f, 16.011f, 58.355f
path.cubicTo(SkBits2Float(0x417eccdc), SkBits2Float(0x4269560b), SkBits2Float(0x417e9fce), SkBits2Float(0x4268d0ec), SkBits2Float(0x417f4fef), SkBits2Float(0x42688319));  // 15.925f, 58.334f, 15.914f, 58.204f, 15.957f, 58.128f
path.cubicTo(SkBits2Float(0x4180168f), SkBits2Float(0x4268364c), SkBits2Float(0x41849589), SkBits2Float(0x4267a5ea), SkBits2Float(0x4185b441), SkBits2Float(0x42679069));  // 16.011f, 58.053f, 16.573f, 57.912f, 16.713f, 57.891f
path.cubicTo(SkBits2Float(0x41891276), SkBits2Float(0x42674296), SkBits2Float(0x418c9dba), SkBits2Float(0x4266df42), SkBits2Float(0x418fd0ed), SkBits2Float(0x4266916f));  // 17.134f, 57.815f, 17.577f, 57.718f, 17.977f, 57.642f
path.cubicTo(SkBits2Float(0x4190ae1c), SkBits2Float(0x42668732), SkBits2Float(0x4191333b), SkBits2Float(0x42668732), SkBits2Float(0x4192106a), SkBits2Float(0x426670aa));  // 18.085f, 57.632f, 18.15f, 57.632f, 18.258f, 57.61f
path.cubicTo(SkBits2Float(0x4193189b), SkBits2Float(0x42665a23), SkBits2Float(0x4194a5eb), SkBits2Float(0x426622d7), SkBits2Float(0x4195dd37), SkBits2Float(0x42660d56));  // 18.387f, 57.588f, 18.581f, 57.534f, 18.733f, 57.513f
path.cubicTo(SkBits2Float(0x41975400), SkBits2Float(0x4265e254), SkBits2Float(0x41988b4b), SkBits2Float(0x4265c18f), SkBits2Float(0x4199d506), SkBits2Float(0x4265ac0e));  // 18.916f, 57.471f, 19.068f, 57.439f, 19.229f, 57.418f
path.moveTo(SkBits2Float(0x41a4e568), SkBits2Float(0x4277d0eb));  // 20.612f, 61.954f
path.cubicTo(SkBits2Float(0x41a4cee1), SkBits2Float(0x4277d0eb), SkBits2Float(0x41a48d58), SkBits2Float(0x4277f1b0), SkBits2Float(0x41a48d58), SkBits2Float(0x4277f1b0));  // 20.601f, 61.954f, 20.569f, 61.986f, 20.569f, 61.986f
path.cubicTo(SkBits2Float(0x41a3831b), SkBits2Float(0x42781275), SkBits2Float(0x41a0c08b), SkBits2Float(0x4277c5a8), SkBits2Float(0x419fe35c), SkBits2Float(0x4277af21));  // 20.439f, 62.018f, 20.094f, 61.943f, 19.986f, 61.921f
path.cubicTo(SkBits2Float(0x419dd0ed), SkBits2Float(0x42778319), SkBits2Float(0x419bbc73), SkBits2Float(0x42775711), SkBits2Float(0x4199c08b), SkBits2Float(0x42771481));  // 19.727f, 61.878f, 19.467f, 61.835f, 19.219f, 61.77f
path.cubicTo(SkBits2Float(0x4199687b), SkBits2Float(0x4277093d), SkBits2Float(0x4198f7d7), SkBits2Float(0x4276f3bc), SkBits2Float(0x4198b64e), SkBits2Float(0x4276dd35));  // 19.176f, 61.759f, 19.121f, 61.738f, 19.089f, 61.716f
path.cubicTo(SkBits2Float(0x419847b6), SkBits2Float(0x4276d1f1), SkBits2Float(0x4198062d), SkBits2Float(0x4276dd35), SkBits2Float(0x4197ae1d), SkBits2Float(0x4276d1f1));  // 19.035f, 61.705f, 19.003f, 61.716f, 18.96f, 61.705f
path.cubicTo(SkBits2Float(0x4196fbf0), SkBits2Float(0x4276c6ad), SkBits2Float(0x4196083a), SkBits2Float(0x42768f62), SkBits2Float(0x4195831b), SkBits2Float(0x427679e1));  // 18.873f, 61.694f, 18.754f, 61.64f, 18.689f, 61.619f
path.cubicTo(SkBits2Float(0x41951690), SkBits2Float(0x4276635a), SkBits2Float(0x41950009), SkBits2Float(0x427679e1), SkBits2Float(0x4194a5ec), SkBits2Float(0x4276635a));  // 18.636f, 61.597f, 18.625f, 61.619f, 18.581f, 61.597f
path.cubicTo(SkBits2Float(0x41940c52), SkBits2Float(0x42764dd9), SkBits2Float(0x41935a25), SkBits2Float(0x4276168d), SkBits2Float(0x4192c08c), SkBits2Float(0x42760006));  // 18.506f, 61.576f, 18.419f, 61.522f, 18.344f, 61.5f
path.cubicTo(SkBits2Float(0x4190c298), SkBits2Float(0x42759cb2), SkBits2Float(0x418f6257), SkBits2Float(0x427544a2), SkBits2Float(0x418e2b0b), SkBits2Float(0x42748837));  // 18.095f, 61.403f, 17.923f, 61.317f, 17.771f, 61.133f
path.cubicTo(SkBits2Float(0x418e1690), SkBits2Float(0x4274666c), SkBits2Float(0x418dd2fb), SkBits2Float(0x4274666c), SkBits2Float(0x418dbe80), SkBits2Float(0x42745c2f));  // 17.761f, 61.1f, 17.728f, 61.1f, 17.718f, 61.09f
path.cubicTo(SkBits2Float(0x418da7f9), SkBits2Float(0x42742f21), SkBits2Float(0x418da7f9), SkBits2Float(0x42740e5c), SkBits2Float(0x418d6670), SkBits2Float(0x4273ed97));  // 17.707f, 61.046f, 17.707f, 61.014f, 17.675f, 60.982f
path.cubicTo(SkBits2Float(0x418d22da), SkBits2Float(0x42739fc4), SkBits2Float(0x418ccaca), SkBits2Float(0x427373bc), SkBits2Float(0x418c9dbc), SkBits2Float(0x42731aa5));  // 17.642f, 60.906f, 17.599f, 60.863f, 17.577f, 60.776f
path.cubicTo(SkBits2Float(0x418bd714), SkBits2Float(0x4271b95d), SkBits2Float(0x418d22db), SkBits2Float(0x4270999f), SkBits2Float(0x418fd0ef), SkBits2Float(0x4270418e));  // 17.48f, 60.431f, 17.642f, 60.15f, 17.977f, 60.064f
path.cubicTo(SkBits2Float(0x41919fc8), SkBits2Float(0x426ffeff), SkBits2Float(0x4193df45), SkBits2Float(0x42701fc3), SkBits2Float(0x4195f3c0), SkBits2Float(0x4270841d));  // 18.203f, 59.999f, 18.484f, 60.031f, 18.744f, 60.129f
path.cubicTo(SkBits2Float(0x419847b8), SkBits2Float(0x4270e771), SkBits2Float(0x419a5a26), SkBits2Float(0x42718211), SkBits2Float(0x419bd2fb), SkBits2Float(0x42723231));  // 19.035f, 60.226f, 19.294f, 60.377f, 19.478f, 60.549f
path.cubicTo(SkBits2Float(0x419be982), SkBits2Float(0x42723e7b), SkBits2Float(0x419be982), SkBits2Float(0x42726a83), SkBits2Float(0x419c1484), SkBits2Float(0x42726a83));  // 19.489f, 60.561f, 19.489f, 60.604f, 19.51f, 60.604f
path.cubicTo(SkBits2Float(0x419c4192), SkBits2Float(0x42728004), SkBits2Float(0x419c831c), SkBits2Float(0x42728004), SkBits2Float(0x419c99a3), SkBits2Float(0x4272968b));  // 19.532f, 60.625f, 19.564f, 60.625f, 19.575f, 60.647f
path.cubicTo(SkBits2Float(0x419cdb2c), SkBits2Float(0x4272b750), SkBits2Float(0x419d083b), SkBits2Float(0x4272ee9b), SkBits2Float(0x419d3549), SkBits2Float(0x427325e7));  // 19.607f, 60.679f, 19.629f, 60.733f, 19.651f, 60.787f
path.cubicTo(SkBits2Float(0x419e28ff), SkBits2Float(0x4273cbca), SkBits2Float(0x419f062e), SkBits2Float(0x4274666a), SkBits2Float(0x419ff7d8), SkBits2Float(0x42750c4d));  // 19.77f, 60.949f, 19.878f, 61.1f, 19.996f, 61.262f
path.cubicTo(SkBits2Float(0x41a0c08c), SkBits2Float(0x42758628), SkBits2Float(0x41a1f5cc), SkBits2Float(0x4275df3f), SkBits2Float(0x41a2d2fb), SkBits2Float(0x42766357));  // 20.094f, 61.381f, 20.245f, 61.468f, 20.353f, 61.597f
path.cubicTo(SkBits2Float(0x41a31484), SkBits2Float(0x42769aa3), SkBits2Float(0x41a36c95), SkBits2Float(0x4276f3b9), SkBits2Float(0x41a3db2c), SkBits2Float(0x42771fc1));  // 20.385f, 61.651f, 20.428f, 61.738f, 20.482f, 61.781f
path.cubicTo(SkBits2Float(0x41a4083a), SkBits2Float(0x42774bc9), SkBits2Float(0x41a4b85b), SkBits2Float(0x42778315), SkBits2Float(0x41a4e569), SkBits2Float(0x4277af1d));  // 20.504f, 61.824f, 20.59f, 61.878f, 20.612f, 61.921f
path.cubicTo(SkBits2Float(0x41a4e569), SkBits2Float(0x4277ba61), SkBits2Float(0x41a4cee2), SkBits2Float(0x4277c5a4), SkBits2Float(0x41a4e569), SkBits2Float(0x4277d0e8));  // 20.612f, 61.932f, 20.601f, 61.943f, 20.612f, 61.954f
path.moveTo(SkBits2Float(0x41ad72b9), SkBits2Float(0x42786044));  // 21.681f, 62.094f
path.cubicTo(SkBits2Float(0x41ac106b), SkBits2Float(0x42788c4c), SkBits2Float(0x41a9d0ee), SkBits2Float(0x4277d0e8), SkBits2Float(0x41a8b236), SkBits2Float(0x42778e58));  // 21.508f, 62.137f, 21.227f, 61.954f, 21.087f, 61.889f
path.cubicTo(SkBits2Float(0x41a2fdfd), SkBits2Float(0x42761689), SkBits2Float(0x41a10215), SkBits2Float(0x42733c6c), SkBits2Float(0x419fb64f), SkBits2Float(0x42704ccf));  // 20.374f, 61.522f, 20.126f, 60.809f, 19.964f, 60.075f
path.cubicTo(SkBits2Float(0x419f9fc8), SkBits2Float(0x42700a40), SkBits2Float(0x419f47b7), SkBits2Float(0x426f9ba8), SkBits2Float(0x419f3130), SkBits2Float(0x426f5919));  // 19.953f, 60.01f, 19.91f, 59.902f, 19.899f, 59.837f
path.cubicTo(SkBits2Float(0x419f3130), SkBits2Float(0x426f0b46), SkBits2Float(0x419f47b7), SkBits2Float(0x426ec9bd), SkBits2Float(0x419f3130), SkBits2Float(0x426e70a6));  // 19.899f, 59.761f, 19.91f, 59.697f, 19.899f, 59.61f
path.cubicTo(SkBits2Float(0x419f1aa9), SkBits2Float(0x426de14a), SkBits2Float(0x419f062e), SkBits2Float(0x426ced94), SkBits2Float(0x419f3130), SkBits2Float(0x426c5d31));  // 19.888f, 59.47f, 19.878f, 59.232f, 19.899f, 59.091f
path.cubicTo(SkBits2Float(0x419f72b9), SkBits2Float(0x426befa0), SkBits2Float(0x419fe35d), SkBits2Float(0x426b8108), SkBits2Float(0x41a00e5f), SkBits2Float(0x426b3335));  // 19.931f, 58.984f, 19.986f, 58.876f, 20.007f, 58.8f
path.cubicTo(SkBits2Float(0x41a0666f), SkBits2Float(0x426acfe1), SkBits2Float(0x41a10215), SkBits2Float(0x4269c6aa), SkBits2Float(0x41a19dbb), SkBits2Float(0x4269bb66));  // 20.05f, 58.703f, 20.126f, 58.444f, 20.202f, 58.433f
path.cubicTo(SkBits2Float(0x41a220cd), SkBits2Float(0x4269bb66), SkBits2Float(0x41a2a5ec), SkBits2Float(0x4269f2b2), SkBits2Float(0x41a31484), SkBits2Float(0x426a3f7f));  // 20.266f, 58.433f, 20.331f, 58.487f, 20.385f, 58.562f
path.cubicTo(SkBits2Float(0x41a3c6b1), SkBits2Float(0x426aa3d9), SkBits2Float(0x41a449c3), SkBits2Float(0x426b1cae), SkBits2Float(0x41a476d2), SkBits2Float(0x426b3e79));  // 20.472f, 58.66f, 20.536f, 58.778f, 20.558f, 58.811f
path.cubicTo(SkBits2Float(0x41a5ac11), SkBits2Float(0x426c0521), SkBits2Float(0x41a6caca), SkBits2Float(0x426ce250), SkBits2Float(0x41a8189d), SkBits2Float(0x426da9fe));  // 20.709f, 59.005f, 20.849f, 59.221f, 21.012f, 59.416f
path.cubicTo(SkBits2Float(0x41aa3f86), SkBits2Float(0x426f1689), SkBits2Float(0x41ac5401), SkBits2Float(0x4270841b), SkBits2Float(0x41ae7aeb), SkBits2Float(0x4271f0a6));  // 21.281f, 59.772f, 21.541f, 60.129f, 21.81f, 60.485f
path.cubicTo(SkBits2Float(0x41af000a), SkBits2Float(0x427248b6), SkBits2Float(0x41afb237), SkBits2Float(0x4272a1cd), SkBits2Float(0x41b020ce), SkBits2Float(0x4272ee9a));  // 21.875f, 60.571f, 21.962f, 60.658f, 22.016f, 60.733f
path.cubicTo(SkBits2Float(0x41b06257), SkBits2Float(0x42731aa2), SkBits2Float(0x41b19797), SkBits2Float(0x4273f7d1), SkBits2Float(0x41b19797), SkBits2Float(0x4274199c));  // 22.048f, 60.776f, 22.199f, 60.992f, 22.199f, 61.025f
path.cubicTo(SkBits2Float(0x41b1c4a5), SkBits2Float(0x427424e0), SkBits2Float(0x41b1831c), SkBits2Float(0x42746669), SkBits2Float(0x41b1831c), SkBits2Float(0x42746669));  // 22.221f, 61.036f, 22.189f, 61.1f, 22.189f, 61.1f
path.cubicTo(SkBits2Float(0x41ac3d7a), SkBits2Float(0x42742f1d), SkBits2Float(0x41a96257), SkBits2Float(0x4271ae17), SkBits2Float(0x41a7a7f9), SkBits2Float(0x426fb12a));  // 21.53f, 61.046f, 21.173f, 60.42f, 20.957f, 59.923f
path.cubicTo(SkBits2Float(0x41a77cf7), SkBits2Float(0x426f9ba9), SkBits2Float(0x41a73b6e), SkBits2Float(0x426f79de), SkBits2Float(0x41a73b6e), SkBits2Float(0x426f591a));  // 20.936f, 59.902f, 20.904f, 59.869f, 20.904f, 59.837f
path.cubicTo(SkBits2Float(0x41a6e151), SkBits2Float(0x426eea82), SkBits2Float(0x41a68941), SkBits2Float(0x426e6564), SkBits2Float(0x41a672ba), SkBits2Float(0x426dec8f));  // 20.86f, 59.729f, 20.817f, 59.599f, 20.806f, 59.481f
path.cubicTo(SkBits2Float(0x41a65e3f), SkBits2Float(0x426daa00), SkBits2Float(0x41a68941), SkBits2Float(0x426d71ae), SkBits2Float(0x41a65e3f), SkBits2Float(0x426d50e9));  // 20.796f, 59.416f, 20.817f, 59.361f, 20.796f, 59.329f
path.cubicTo(SkBits2Float(0x41a63131), SkBits2Float(0x426d24e1), SkBits2Float(0x41a56a89), SkBits2Float(0x426cf8d9), SkBits2Float(0x41a4fbf1), SkBits2Float(0x426cf8d9));  // 20.774f, 59.286f, 20.677f, 59.243f, 20.623f, 59.243f
path.cubicTo(SkBits2Float(0x41a449c4), SkBits2Float(0x426ced95), SkBits2Float(0x41a36c95), SkBits2Float(0x426cf8d9), SkBits2Float(0x41a31484), SkBits2Float(0x426d24e1));  // 20.536f, 59.232f, 20.428f, 59.243f, 20.385f, 59.286f
path.cubicTo(SkBits2Float(0x41a20a47), SkBits2Float(0x426d71ae), SkBits2Float(0x41a1f5cc), SkBits2Float(0x426f645e), SkBits2Float(0x41a220ce), SkBits2Float(0x42701fc2));  // 20.255f, 59.361f, 20.245f, 59.848f, 20.266f, 60.031f
path.cubicTo(SkBits2Float(0x41a28f66), SkBits2Float(0x4272e45e), SkBits2Float(0x41a4b85b), SkBits2Float(0x4274c9be), SkBits2Float(0x41a7eb8e), SkBits2Float(0x427621ce));  // 20.32f, 60.723f, 20.59f, 61.197f, 20.99f, 61.533f
path.cubicTo(SkBits2Float(0x41a82d17), SkBits2Float(0x42764293), SkBits2Float(0x41a870ad), SkBits2Float(0x42764293), SkBits2Float(0x41a8b236), SkBits2Float(0x4276591a));  // 21.022f, 61.565f, 21.055f, 61.565f, 21.087f, 61.587f
path.cubicTo(SkBits2Float(0x41a90a46), SkBits2Float(0x427679df), SkBits2Float(0x41a93755), SkBits2Float(0x4276b12a), SkBits2Float(0x41a98f65), SkBits2Float(0x4276c6ab));  // 21.13f, 61.619f, 21.152f, 61.673f, 21.195f, 61.694f
path.cubicTo(SkBits2Float(0x41aadb2c), SkBits2Float(0x42774086), SkBits2Float(0x41ac958a), SkBits2Float(0x42778e59), SkBits2Float(0x41adb64e), SkBits2Float(0x427828f9));  // 21.357f, 61.813f, 21.573f, 61.889f, 21.714f, 62.04f
path.cubicTo(SkBits2Float(0x41adb64e), SkBits2Float(0x427828f9), SkBits2Float(0x41ad8940), SkBits2Float(0x42786045), SkBits2Float(0x41ad72b8), SkBits2Float(0x42786045));  // 21.714f, 62.04f, 21.692f, 62.094f, 21.681f, 62.094f
path.moveTo(SkBits2Float(0x41bd168f), SkBits2Float(0x4267be7a));  // 23.636f, 57.936f
path.cubicTo(SkBits2Float(0x41bd168f), SkBits2Float(0x42679caf), SkBits2Float(0x41bd2d16), SkBits2Float(0x4267666a), SkBits2Float(0x41bd168f), SkBits2Float(0x42674fe2));  // 23.636f, 57.903f, 23.647f, 57.85f, 23.636f, 57.828f
path.cubicTo(SkBits2Float(0x41bd168f), SkBits2Float(0x4267449e), SkBits2Float(0x41bd0008), SkBits2Float(0x42674fe2), SkBits2Float(0x41bce981), SkBits2Float(0x42672f1d));  // 23.636f, 57.817f, 23.625f, 57.828f, 23.614f, 57.796f
path.cubicTo(SkBits2Float(0x41bcd2fa), SkBits2Float(0x42672f1d), SkBits2Float(0x41bc9171), SkBits2Float(0x4267449e), SkBits2Float(0x41bc7ae9), SkBits2Float(0x42672f1d));  // 23.603f, 57.796f, 23.571f, 57.817f, 23.56f, 57.796f
path.cubicTo(SkBits2Float(0x41bb9dba), SkBits2Float(0x4267d500), SkBits2Float(0x41bbb441), SkBits2Float(0x42693648), SkBits2Float(0x41bb72b8), SkBits2Float(0x426a1377));  // 23.452f, 57.958f, 23.463f, 58.303f, 23.431f, 58.519f
path.cubicTo(SkBits2Float(0x41bb45aa), SkBits2Float(0x426a6c8e), SkBits2Float(0x41bb2f22), SkBits2Float(0x426acfe1), SkBits2Float(0x41bb189b), SkBits2Float(0x426b3335));  // 23.409f, 58.606f, 23.398f, 58.703f, 23.387f, 58.8f
path.lineTo(SkBits2Float(0x41baed99), SkBits2Float(0x426b5f3d));  // 23.366f, 58.843f
path.cubicTo(SkBits2Float(0x41baac10), SkBits2Float(0x426bd918), SkBits2Float(0x41bac08b), SkBits2Float(0x426c3129), SkBits2Float(0x41baac10), SkBits2Float(0x426cab04));  // 23.334f, 58.962f, 23.344f, 59.048f, 23.334f, 59.167f
path.cubicTo(SkBits2Float(0x41ba7f02), SkBits2Float(0x426d50e7), SkBits2Float(0x41ba3b6c), SkBits2Float(0x426e0d52), SkBits2Float(0x41ba106a), SkBits2Float(0x426ec9bc));  // 23.312f, 59.329f, 23.279f, 59.513f, 23.258f, 59.697f
path.cubicTo(SkBits2Float(0x41b9ccd4), SkBits2Float(0x426f645c), SkBits2Float(0x41b974c4), SkBits2Float(0x42701fc0), SkBits2Float(0x41b949c2), SkBits2Float(0x4270c5a3));  // 23.225f, 59.848f, 23.182f, 60.031f, 23.161f, 60.193f
path.cubicTo(SkBits2Float(0x41b9333b), SkBits2Float(0x42713f7e), SkBits2Float(0x41b98b4b), SkBits2Float(0x4271820d), SkBits2Float(0x41b9f9e3), SkBits2Float(0x4271ae16));  // 23.15f, 60.312f, 23.193f, 60.377f, 23.247f, 60.42f
path.cubicTo(SkBits2Float(0x41ba3b6c), SkBits2Float(0x42718d51), SkBits2Float(0x41ba7f02), SkBits2Float(0x4271b95a), SkBits2Float(0x41ba9589), SkBits2Float(0x42716b87));  // 23.279f, 60.388f, 23.312f, 60.431f, 23.323f, 60.355f
path.cubicTo(SkBits2Float(0x41baac10), SkBits2Float(0x4271343b), SkBits2Float(0x41ba9589), SkBits2Float(0x4270e76e), SkBits2Float(0x41ba9589), SkBits2Float(0x4270999b));  // 23.334f, 60.301f, 23.323f, 60.226f, 23.323f, 60.15f
path.cubicTo(SkBits2Float(0x41ba9589), SkBits2Float(0x4270418b), SkBits2Float(0x41bac08b), SkBits2Float(0x426fd1ed), SkBits2Float(0x41baed99), SkBits2Float(0x426f645c));  // 23.323f, 60.064f, 23.344f, 59.955f, 23.366f, 59.848f
path.cubicTo(SkBits2Float(0x41bb2f22), SkBits2Float(0x426e6562), SkBits2Float(0x41bb9dba), SkBits2Float(0x426d3b66), SkBits2Float(0x41bbf5ca), SkBits2Float(0x426c3c6c));  // 23.398f, 59.599f, 23.452f, 59.308f, 23.495f, 59.059f
path.cubicTo(SkBits2Float(0x41bc0e5d), SkBits2Float(0x426bb853), SkBits2Float(0x41bc0e5d), SkBits2Float(0x426b5f3d), SkBits2Float(0x41bc22d8), SkBits2Float(0x426ae562));  // 23.507f, 58.93f, 23.507f, 58.843f, 23.517f, 58.724f
path.cubicTo(SkBits2Float(0x41bc395f), SkBits2Float(0x426a820e), SkBits2Float(0x41bc9170), SkBits2Float(0x4269f2b2), SkBits2Float(0x41bca7f7), SkBits2Float(0x42698f5e));  // 23.528f, 58.627f, 23.571f, 58.487f, 23.582f, 58.39f
path.cubicTo(SkBits2Float(0x41bcd2f9), SkBits2Float(0x426920c6), SkBits2Float(0x41bca7f7), SkBits2Float(0x4268d2f4), SkBits2Float(0x41bcd2f9), SkBits2Float(0x4268645c));  // 23.603f, 58.282f, 23.582f, 58.206f, 23.603f, 58.098f
path.cubicTo(SkBits2Float(0x41bcd2f9), SkBits2Float(0x42684291), SkBits2Float(0x41bd168f), SkBits2Float(0x4267df3d), SkBits2Float(0x41bd168f), SkBits2Float(0x4267be79));  // 23.603f, 58.065f, 23.636f, 57.968f, 23.636f, 57.936f
path.moveTo(SkBits2Float(0x41bd6e9f), SkBits2Float(0x426e916b));  // 23.679f, 59.642f
path.cubicTo(SkBits2Float(0x41bdb028), SkBits2Float(0x426d199c), SkBits2Float(0x41bdf3be), SkBits2Float(0x426bb854), SkBits2Float(0x41be6255), SkBits2Float(0x426a343c));  // 23.711f, 59.275f, 23.744f, 58.93f, 23.798f, 58.551f
path.cubicTo(SkBits2Float(0x41be78dc), SkBits2Float(0x4269f2b3), SkBits2Float(0x41bed0ed), SkBits2Float(0x4269841b), SkBits2Float(0x41bed0ed), SkBits2Float(0x4269418c));  // 23.809f, 58.487f, 23.852f, 58.379f, 23.852f, 58.314f
path.cubicTo(SkBits2Float(0x41bee774), SkBits2Float(0x4268bc6d), SkBits2Float(0x41bee774), SkBits2Float(0x42684edc), SkBits2Float(0x41bf1276), SkBits2Float(0x4267df3e));  // 23.863f, 58.184f, 23.863f, 58.077f, 23.884f, 57.968f
path.cubicTo(SkBits2Float(0x41bf3f84), SkBits2Float(0x4267a7f2), SkBits2Float(0x41bf3f84), SkBits2Float(0x4267872e), SkBits2Float(0x41bf9795), SkBits2Float(0x426770a6));  // 23.906f, 57.914f, 23.906f, 57.882f, 23.949f, 57.86f
path.cubicTo(SkBits2Float(0x41c0ccd4), SkBits2Float(0x42675b25), SkBits2Float(0x41c6810e), SkBits2Float(0x4268d2f4), SkBits2Float(0x41c6d91e), SkBits2Float(0x426920c7));  // 24.1f, 57.839f, 24.813f, 58.206f, 24.856f, 58.282f
path.cubicTo(SkBits2Float(0x41c7333b), SkBits2Float(0x42696d94), SkBits2Float(0x41c7062c), SkBits2Float(0x4270e76f), SkBits2Float(0x41c6ae1c), SkBits2Float(0x42713f7f));  // 24.9f, 58.357f, 24.878f, 60.226f, 24.835f, 60.312f
path.cubicTo(SkBits2Float(0x41c63f84), SkBits2Float(0x4271a2d3), SkBits2Float(0x41c3a7f7), SkBits2Float(0x42716b87), SkBits2Float(0x41c2cac8), SkBits2Float(0x427176cb));  // 24.781f, 60.409f, 24.457f, 60.355f, 24.349f, 60.366f
path.cubicTo(SkBits2Float(0x41c2b441), SkBits2Float(0x427176cb), SkBits2Float(0x41c270ab), SkBits2Float(0x4271a2d3), SkBits2Float(0x41c245a9), SkBits2Float(0x4271a2d3));  // 24.338f, 60.366f, 24.305f, 60.409f, 24.284f, 60.409f
path.cubicTo(SkBits2Float(0x41c1aa03), SkBits2Float(0x4271b95a), SkBits2Float(0x41c1106a), SkBits2Float(0x4271ae17), SkBits2Float(0x41c05e3c), SkBits2Float(0x4271b95a));  // 24.208f, 60.431f, 24.133f, 60.42f, 24.046f, 60.431f
path.cubicTo(SkBits2Float(0x41bf1275), SkBits2Float(0x4271e562), SkBits2Float(0x41be4bcd), SkBits2Float(0x427227f2), SkBits2Float(0x41bcd2f8), SkBits2Float(0x4272322f));  // 23.884f, 60.474f, 23.787f, 60.539f, 23.603f, 60.549f
path.cubicTo(SkBits2Float(0x41bc395e), SkBits2Float(0x427128f8), SkBits2Float(0x41bd2d15), SkBits2Float(0x426f8f5e), SkBits2Float(0x41bd6e9e), SkBits2Float(0x426e916a));  // 23.528f, 60.29f, 23.647f, 59.89f, 23.679f, 59.642f
path.moveTo(SkBits2Float(0x41d21481), SkBits2Float(0x42700a3f));  // 26.26f, 60.01f
path.cubicTo(SkBits2Float(0x41d22b08), SkBits2Float(0x42704cce), SkBits2Float(0x41d299a0), SkBits2Float(0x4270f1ac), SkBits2Float(0x41d2418f), SkBits2Float(0x42713f7e));  // 26.271f, 60.075f, 26.325f, 60.236f, 26.282f, 60.312f
path.cubicTo(SkBits2Float(0x41d2418f), SkBits2Float(0x42714ac2), SkBits2Float(0x41d22b08), SkBits2Float(0x42713f7e), SkBits2Float(0x41d21481), SkBits2Float(0x42715605));  // 26.282f, 60.323f, 26.271f, 60.312f, 26.26f, 60.334f
path.cubicTo(SkBits2Float(0x41d1bc71), SkBits2Float(0x42715605), SkBits2Float(0x41d1916f), SkBits2Float(0x42715605), SkBits2Float(0x41d1395e), SkBits2Float(0x42714ac1));  // 26.217f, 60.334f, 26.196f, 60.334f, 26.153f, 60.323f
path.cubicTo(SkBits2Float(0x41d0b233), SkBits2Float(0x42708419), SkBits2Float(0x41d0c8ba), SkBits2Float(0x426f645b), SkBits2Float(0x41d09db8), SkBits2Float(0x426e5a1d));  // 26.087f, 60.129f, 26.098f, 59.848f, 26.077f, 59.588f
path.cubicTo(SkBits2Float(0x41d09db8), SkBits2Float(0x426e23d7), SkBits2Float(0x41d05a22), SkBits2Float(0x426d9375), SkBits2Float(0x41d070aa), SkBits2Float(0x426d50e6));  // 26.077f, 59.535f, 26.044f, 59.394f, 26.055f, 59.329f
path.cubicTo(SkBits2Float(0x41d09db8), SkBits2Float(0x426d3b65), SkBits2Float(0x41d0b233), SkBits2Float(0x426d50e6), SkBits2Float(0x41d0b233), SkBits2Float(0x426d2f1b));  // 26.077f, 59.308f, 26.087f, 59.329f, 26.087f, 59.296f
path.cubicTo(SkBits2Float(0x41d1395e), SkBits2Float(0x426d3b65), SkBits2Float(0x41d14dd9), SkBits2Float(0x426d2f1b), SkBits2Float(0x41d1916e), SkBits2Float(0x426d50e6));  // 26.153f, 59.308f, 26.163f, 59.296f, 26.196f, 59.329f
path.cubicTo(SkBits2Float(0x41d1a5e9), SkBits2Float(0x426d50e6), SkBits2Float(0x41d1e97e), SkBits2Float(0x426de148), SkBits2Float(0x41d1e97e), SkBits2Float(0x426dec8c));  // 26.206f, 59.329f, 26.239f, 59.47f, 26.239f, 59.481f
path.cubicTo(SkBits2Float(0x41d22b07), SkBits2Float(0x426e9cad), SkBits2Float(0x41d1e97e), SkBits2Float(0x426f4dd4), SkBits2Float(0x41d21480), SkBits2Float(0x42700a3e));  // 26.271f, 59.653f, 26.239f, 59.826f, 26.26f, 60.01f
path.moveTo(SkBits2Float(0x41ee1274), SkBits2Float(0x42564ac1));  // 29.759f, 53.573f
path.cubicTo(SkBits2Float(0x41ee1274), SkBits2Float(0x42566b86), SkBits2Float(0x41ee3f82), SkBits2Float(0x4256c49c), SkBits2Float(0x41ee28fb), SkBits2Float(0x4256fbe8));  // 29.759f, 53.605f, 29.781f, 53.692f, 29.77f, 53.746f
path.cubicTo(SkBits2Float(0x41ee28fb), SkBits2Float(0x42571cad), SkBits2Float(0x41ede772), SkBits2Float(0x425748b5), SkBits2Float(0x41ede772), SkBits2Float(0x42576a80));  // 29.77f, 53.778f, 29.738f, 53.821f, 29.738f, 53.854f
path.cubicTo(SkBits2Float(0x41ed8f62), SkBits2Float(0x425774bd), SkBits2Float(0x41ed20ca), SkBits2Float(0x42579688), SkBits2Float(0x41ec6e9d), SkBits2Float(0x42579688));  // 29.695f, 53.864f, 29.641f, 53.897f, 29.554f, 53.897f
path.cubicTo(SkBits2Float(0x41ebeb8b), SkBits2Float(0x42579688), SkBits2Float(0x41eb666c), SkBits2Float(0x425774bd), SkBits2Float(0x41eaf7d4), SkBits2Float(0x42576a80));  // 29.49f, 53.897f, 29.425f, 53.864f, 29.371f, 53.854f
path.cubicTo(SkBits2Float(0x41eacac6), SkBits2Float(0x425676ca), SkBits2Float(0x41eb666c), SkBits2Float(0x42556d92), SkBits2Float(0x41ebbe7c), SkBits2Float(0x42549063));  // 29.349f, 53.616f, 29.425f, 53.357f, 29.468f, 53.141f
path.cubicTo(SkBits2Float(0x41ebd503), SkBits2Float(0x425421cb), SkBits2Float(0x41ebd503), SkBits2Float(0x4253d3f9), SkBits2Float(0x41ec0005), SkBits2Float(0x42537be8));  // 29.479f, 53.033f, 29.479f, 52.957f, 29.5f, 52.871f
path.cubicTo(SkBits2Float(0x41ec2d13), SkBits2Float(0x42535a1d), SkBits2Float(0x41ec6e9d), SkBits2Float(0x42531894), SkBits2Float(0x41ecb232), SkBits2Float(0x42531894));  // 29.522f, 52.838f, 29.554f, 52.774f, 29.587f, 52.774f
path.cubicTo(SkBits2Float(0x41ed3544), SkBits2Float(0x4253020d), SkBits2Float(0x41edd0ea), SkBits2Float(0x42531894), SkBits2Float(0x41ede771), SkBits2Float(0x4253449c));  // 29.651f, 52.752f, 29.727f, 52.774f, 29.738f, 52.817f
path.cubicTo(SkBits2Float(0x41ee1273), SkBits2Float(0x42534fe0), SkBits2Float(0x41ede771), SkBits2Float(0x42536561), SkBits2Float(0x41ede771), SkBits2Float(0x42537be8));  // 29.759f, 52.828f, 29.738f, 52.849f, 29.738f, 52.871f
path.cubicTo(SkBits2Float(0x41ee3f81), SkBits2Float(0x42544290), SkBits2Float(0x41ede771), SkBits2Float(0x42554ccd), SkBits2Float(0x41ee1273), SkBits2Float(0x42564ac1));  // 29.781f, 53.065f, 29.738f, 53.325f, 29.759f, 53.573f
path.moveTo(SkBits2Float(0x41f51273), SkBits2Float(0x4258cbc7));  // 30.634f, 54.199f
path.cubicTo(SkBits2Float(0x41f4e771), SkBits2Float(0x4259199a), SkBits2Float(0x41f3b025), SkBits2Float(0x4259bf7d), SkBits2Float(0x41f35815), SkBits2Float(0x4259eb85));  // 30.613f, 54.275f, 30.461f, 54.437f, 30.418f, 54.48f
path.cubicTo(SkBits2Float(0x41f2395d), SkBits2Float(0x425aa6e9), SkBits2Float(0x41f2395d), SkBits2Float(0x425a449c), SkBits2Float(0x41f222d6), SkBits2Float(0x42596666));  // 30.278f, 54.663f, 30.278f, 54.567f, 30.267f, 54.35f
path.cubicTo(SkBits2Float(0x41f222d6), SkBits2Float(0x425945a1), SkBits2Float(0x41f1f5c8), SkBits2Float(0x4258e24d), SkBits2Float(0x41f222d6), SkBits2Float(0x4258ab02));  // 30.267f, 54.318f, 30.245f, 54.221f, 30.267f, 54.167f
path.cubicTo(SkBits2Float(0x41f2395d), SkBits2Float(0x42589fbe), SkBits2Float(0x41f2e97e), SkBits2Float(0x42588a3d), SkBits2Float(0x41f30005), SkBits2Float(0x425873b6));  // 30.278f, 54.156f, 30.364f, 54.135f, 30.375f, 54.113f
path.cubicTo(SkBits2Float(0x41f3b026), SkBits2Float(0x42586872), SkBits2Float(0x41f48d55), SkBits2Float(0x42588937), SkBits2Float(0x41f51274), SkBits2Float(0x4258947b));  // 30.461f, 54.102f, 30.569f, 54.134f, 30.634f, 54.145f
path.cubicTo(SkBits2Float(0x41f4fdf9), SkBits2Float(0x42589fbf), SkBits2Float(0x41f51274), SkBits2Float(0x4258b646), SkBits2Float(0x41f51274), SkBits2Float(0x4258cbc7));  // 30.624f, 54.156f, 30.634f, 54.178f, 30.634f, 54.199f
path.moveTo(SkBits2Float(0x41f20e5b), SkBits2Float(0x425727f0));  // 30.257f, 53.789f
path.cubicTo(SkBits2Float(0x41f1cac5), SkBits2Float(0x4256da1d), SkBits2Float(0x41f222d6), SkBits2Float(0x42561375), SkBits2Float(0x41f222d6), SkBits2Float(0x4255d0e6));  // 30.224f, 53.713f, 30.267f, 53.519f, 30.267f, 53.454f
path.cubicTo(SkBits2Float(0x41f222d6), SkBits2Float(0x42553646), SkBits2Float(0x41f1b43e), SkBits2Float(0x4254374c), SkBits2Float(0x41f20e5b), SkBits2Float(0x42539169));  // 30.267f, 53.303f, 30.213f, 53.054f, 30.257f, 52.892f
path.cubicTo(SkBits2Float(0x41f222d6), SkBits2Float(0x42536561), SkBits2Float(0x41f2916d), SkBits2Float(0x4253449c), SkBits2Float(0x41f2be7c), SkBits2Float(0x4253449c));  // 30.267f, 52.849f, 30.321f, 52.817f, 30.343f, 52.817f
path.cubicTo(SkBits2Float(0x41f3b026), SkBits2Float(0x42532e15), SkBits2Float(0x41f845a7), SkBits2Float(0x42539cac), SkBits2Float(0x41f88730), SkBits2Float(0x4253d3f8));  // 30.461f, 52.795f, 31.034f, 52.903f, 31.066f, 52.957f
path.cubicTo(SkBits2Float(0x41f8cac6), SkBits2Float(0x42540000), SkBits2Float(0x41f8cac6), SkBits2Float(0x42544290), SkBits2Float(0x41f8e14d), SkBits2Float(0x4254851f));  // 31.099f, 53, 31.099f, 53.065f, 31.11f, 53.13f
path.cubicTo(SkBits2Float(0x41f8f5c8), SkBits2Float(0x4254d1ec), SkBits2Float(0x41f97ae7), SkBits2Float(0x425578d5), SkBits2Float(0x41f9666c), SkBits2Float(0x4255e76d));  // 31.12f, 53.205f, 31.185f, 53.368f, 31.175f, 53.476f
path.cubicTo(SkBits2Float(0x41f94dd9), SkBits2Float(0x42561375), SkBits2Float(0x41f88731), SkBits2Float(0x4256a2d1), SkBits2Float(0x41f85c2f), SkBits2Float(0x4256c49c));  // 31.163f, 53.519f, 31.066f, 53.659f, 31.045f, 53.692f
path.cubicTo(SkBits2Float(0x41f845a8), SkBits2Float(0x4256da1d), SkBits2Float(0x41f7d710), SkBits2Float(0x4256f0a4), SkBits2Float(0x41f7d710), SkBits2Float(0x4256fbe8));  // 31.034f, 53.713f, 30.98f, 53.735f, 30.98f, 53.746f
path.lineTo(SkBits2Float(0x41f7d710), SkBits2Float(0x42571cad));  // 30.98f, 53.778f
path.cubicTo(SkBits2Float(0x41f79587), SkBits2Float(0x4257322e), SkBits2Float(0x41f73b6a), SkBits2Float(0x425748b5), SkBits2Float(0x41f6f9e1), SkBits2Float(0x42575f3c));  // 30.948f, 53.799f, 30.904f, 53.821f, 30.872f, 53.843f
path.cubicTo(SkBits2Float(0x41f6062b), SkBits2Float(0x425774bd), SkBits2Float(0x41f2395e), SkBits2Float(0x425774bd), SkBits2Float(0x41f20e5c), SkBits2Float(0x425727f0));  // 30.753f, 53.864f, 30.278f, 53.864f, 30.257f, 53.789f
path.moveTo(SkBits2Float(0x42048f5f), SkBits2Float(0x426b072b));  // 33.14f, 58.757f
path.cubicTo(SkBits2Float(0x42046d94), SkBits2Float(0x426acfdf), SkBits2Float(0x42048f5f), SkBits2Float(0x426ab958), SkBits2Float(0x420478d8), SkBits2Float(0x426a77cf));  // 33.107f, 58.703f, 33.14f, 58.681f, 33.118f, 58.617f
path.cubicTo(SkBits2Float(0x42045813), SkBits2Float(0x4269d0e6), SkBits2Float(0x42042c0b), SkBits2Float(0x42693646), SkBits2Float(0x42041584), SkBits2Float(0x4268851f));  // 33.086f, 58.454f, 33.043f, 58.303f, 33.021f, 58.13f
path.cubicTo(SkBits2Float(0x4203e97c), SkBits2Float(0x4267c9bb), SkBits2Float(0x42039caf), SkBits2Float(0x42670d50), SkBits2Float(0x4203a6ec), SkBits2Float(0x426624dd));  // 32.978f, 57.947f, 32.903f, 57.763f, 32.913f, 57.536f
path.cubicTo(SkBits2Float(0x4203a6ec), SkBits2Float(0x426624dd), SkBits2Float(0x4203de38), SkBits2Float(0x4265f8d5), SkBits2Float(0x4203e97b), SkBits2Float(0x4265f8d5));  // 32.913f, 57.536f, 32.967f, 57.493f, 32.978f, 57.493f
path.cubicTo(SkBits2Float(0x42042c0a), SkBits2Float(0x4265ee98), SkBits2Float(0x4204c6aa), SkBits2Float(0x4266199a), SkBits2Float(0x4204e875), SkBits2Float(0x42663b64));  // 33.043f, 57.483f, 33.194f, 57.525f, 33.227f, 57.558f
path.cubicTo(SkBits2Float(0x42051ebb), SkBits2Float(0x42668937), SkBits2Float(0x42051ebb), SkBits2Float(0x42671893), SkBits2Float(0x42054085), SkBits2Float(0x426770a3));  // 33.28f, 57.634f, 33.28f, 57.774f, 33.313f, 57.86f
path.cubicTo(SkBits2Float(0x42058314), SkBits2Float(0x4268a6e9), SkBits2Float(0x4206072d), SkBits2Float(0x4269d0e5), SkBits2Float(0x42061271), SkBits2Float(0x426b3e76));  // 33.378f, 58.163f, 33.507f, 58.454f, 33.518f, 58.811f
path.cubicTo(SkBits2Float(0x4205e669), SkBits2Float(0x426b3e76), SkBits2Float(0x4205e669), SkBits2Float(0x426b49ba), SkBits2Float(0x4205b95a), SkBits2Float(0x426b5f3b));  // 33.475f, 58.811f, 33.475f, 58.822f, 33.431f, 58.843f
path.cubicTo(SkBits2Float(0x42056c8d), SkBits2Float(0x426b5f3b), SkBits2Float(0x4204e875), SkBits2Float(0x426b75c2), SkBits2Float(0x4204b023), SkBits2Float(0x426b49ba));  // 33.356f, 58.843f, 33.227f, 58.865f, 33.172f, 58.822f
path.lineTo(SkBits2Float(0x4204b023), SkBits2Float(0x426b3333));  // 33.172f, 58.8f
path.cubicTo(SkBits2Float(0x4204b023), SkBits2Float(0x426b27ef), SkBits2Float(0x42048f5e), SkBits2Float(0x426b072b), SkBits2Float(0x42048f5e), SkBits2Float(0x426b072b));  // 33.172f, 58.789f, 33.14f, 58.757f, 33.14f, 58.757f
path.moveTo(SkBits2Float(0x42035918), SkBits2Float(0x426b6a7f));  // 32.837f, 58.854f
path.cubicTo(SkBits2Float(0x42032d10), SkBits2Float(0x426b6a7f), SkBits2Float(0x42030108), SkBits2Float(0x426b75c3), SkBits2Float(0x4202d4ff), SkBits2Float(0x426b75c3));  // 32.794f, 58.854f, 32.751f, 58.865f, 32.708f, 58.865f
path.cubicTo(SkBits2Float(0x42026667), SkBits2Float(0x426b75c3), SkBits2Float(0x42020d51), SkBits2Float(0x426b5f3c), SkBits2Float(0x4201ec8c), SkBits2Float(0x426b27f0));  // 32.6f, 58.865f, 32.513f, 58.843f, 32.481f, 58.789f
path.cubicTo(SkBits2Float(0x4201cbc7), SkBits2Float(0x426ae561), SkBits2Float(0x4201cbc7), SkBits2Float(0x426a6c8c), SkBits2Float(0x4201b540), SkBits2Float(0x426a0832));  // 32.449f, 58.724f, 32.449f, 58.606f, 32.427f, 58.508f
path.cubicTo(SkBits2Float(0x42018938), SkBits2Float(0x426920c5), SkBits2Float(0x42016873), SkBits2Float(0x42683853), SkBits2Float(0x42013021), SkBits2Float(0x42672f1b));  // 32.384f, 58.282f, 32.352f, 58.055f, 32.297f, 57.796f
path.cubicTo(SkBits2Float(0x42013021), SkBits2Float(0x4267020d), SkBits2Float(0x4200f9db), SkBits2Float(0x42669375), SkBits2Float(0x4200f9db), SkBits2Float(0x426651ec));  // 32.297f, 57.752f, 32.244f, 57.644f, 32.244f, 57.58f
path.cubicTo(SkBits2Float(0x42010418), SkBits2Float(0x4266199a), SkBits2Float(0x420151eb), SkBits2Float(0x4265ee98), SkBits2Float(0x42018937), SkBits2Float(0x4265ee98));  // 32.254f, 57.525f, 32.33f, 57.483f, 32.384f, 57.483f
path.cubicTo(SkBits2Float(0x4201e147), SkBits2Float(0x4265e24e), SkBits2Float(0x42022f1a), SkBits2Float(0x4265ee98), SkBits2Float(0x42023a5e), SkBits2Float(0x4266199a));  // 32.47f, 57.471f, 32.546f, 57.483f, 32.557f, 57.525f
path.cubicTo(SkBits2Float(0x420271aa), SkBits2Float(0x42665c29), SkBits2Float(0x42027be7), SkBits2Float(0x42670d50), SkBits2Float(0x42029db2), SkBits2Float(0x426770a4));  // 32.611f, 57.59f, 32.621f, 57.763f, 32.654f, 57.86f
path.cubicTo(SkBits2Float(0x42029db2), SkBits2Float(0x4267be77), SkBits2Float(0x4202d4fe), SkBits2Float(0x4268178d), SkBits2Float(0x4202e041), SkBits2Float(0x42684ed9));  // 32.654f, 57.936f, 32.708f, 58.023f, 32.719f, 58.077f
path.cubicTo(SkBits2Float(0x4202ea7e), SkBits2Float(0x4268bc6a), SkBits2Float(0x4202ea7e), SkBits2Float(0x4268fefa), SkBits2Float(0x42030106), SkBits2Float(0x42695810));  // 32.729f, 58.184f, 32.729f, 58.249f, 32.751f, 58.336f
path.cubicTo(SkBits2Float(0x420322d1), SkBits2Float(0x4269fced), SkBits2Float(0x4203645a), SkBits2Float(0x426a820c), SkBits2Float(0x4203645a), SkBits2Float(0x426b49ba));  // 32.784f, 58.497f, 32.848f, 58.627f, 32.848f, 58.822f
path.cubicTo(SkBits2Float(0x42034395), SkBits2Float(0x426b49ba), SkBits2Float(0x42035916), SkBits2Float(0x426b49ba), SkBits2Float(0x42035916), SkBits2Float(0x426b6a7f));  // 32.816f, 58.822f, 32.837f, 58.822f, 32.837f, 58.854f
path.moveTo(SkBits2Float(0x42009580), SkBits2Float(0x426b6a7f));  // 32.146f, 58.854f
path.lineTo(SkBits2Float(0x42008b43), SkBits2Float(0x426b8106));  // 32.136f, 58.876f
path.cubicTo(SkBits2Float(0x42007fff), SkBits2Float(0x426b8106), SkBits2Float(0x42005e35), SkBits2Float(0x426b75c2), SkBits2Float(0x420048b4), SkBits2Float(0x426b8106));  // 32.125f, 58.876f, 32.092f, 58.865f, 32.071f, 58.876f
path.cubicTo(SkBits2Float(0x41fdcccc), SkBits2Float(0x426bad0e), SkBits2Float(0x41f94dd2), SkBits2Float(0x426b8c4a), SkBits2Float(0x41f6cccc), SkBits2Float(0x426b8c4a));  // 31.725f, 58.919f, 31.163f, 58.887f, 30.85f, 58.887f
path.cubicTo(SkBits2Float(0x41f65e34), SkBits2Float(0x426b8106), SkBits2Float(0x41f39ba5), SkBits2Float(0x426b8106), SkBits2Float(0x41f35810), SkBits2Float(0x426b49bb));  // 30.796f, 58.876f, 30.451f, 58.876f, 30.418f, 58.822f
path.cubicTo(SkBits2Float(0x41f35810), SkBits2Float(0x426b3334), SkBits2Float(0x41f2e978), SkBits2Float(0x4267926f), SkBits2Float(0x41f31687), SkBits2Float(0x426723d8));  // 30.418f, 58.8f, 30.364f, 57.893f, 30.386f, 57.785f
path.lineTo(SkBits2Float(0x41f36e97), SkBits2Float(0x4266ec8c));  // 30.429f, 57.731f
path.cubicTo(SkBits2Float(0x41f3f3b6), SkBits2Float(0x4266b540), SkBits2Float(0x41f4d0e5), SkBits2Float(0x4266b540), SkBits2Float(0x41f58106), SkBits2Float(0x42669eb9));  // 30.494f, 57.677f, 30.602f, 57.677f, 30.688f, 57.655f
path.cubicTo(SkBits2Float(0x41f7ed91), SkBits2Float(0x42663b65), SkBits2Float(0x41fac6a8), SkBits2Float(0x4265ee98), SkBits2Float(0x41fdb646), SkBits2Float(0x4265d811));  // 30.991f, 57.558f, 31.347f, 57.483f, 31.714f, 57.461f
path.cubicTo(SkBits2Float(0x41fe51ec), SkBits2Float(0x4265c18a), SkBits2Float(0x41ff2f1b), SkBits2Float(0x4265d811), SkBits2Float(0x41ff872b), SkBits2Float(0x4265f8d6));  // 31.79f, 57.439f, 31.898f, 57.461f, 31.941f, 57.493f
path.cubicTo(SkBits2Float(0x41ffb439), SkBits2Float(0x4266199b), SkBits2Float(0x41ffb439), SkBits2Float(0x42669eb9), SkBits2Float(0x41ffdf3b), SkBits2Float(0x4266d605));  // 31.963f, 57.525f, 31.963f, 57.655f, 31.984f, 57.709f
path.cubicTo(SkBits2Float(0x41fff5c2), SkBits2Float(0x42670d51), SkBits2Float(0x42001cac), SkBits2Float(0x42675b24), SkBits2Float(0x42001cac), SkBits2Float(0x4267926f));  // 31.995f, 57.763f, 32.028f, 57.839f, 32.028f, 57.893f
path.cubicTo(SkBits2Float(0x42003d71), SkBits2Float(0x42684290), SkBits2Float(0x420048b4), SkBits2Float(0x4268c7ae), SkBits2Float(0x42005e35), SkBits2Float(0x42696d92));  // 32.06f, 58.065f, 32.071f, 58.195f, 32.092f, 58.357f
path.cubicTo(SkBits2Float(0x42008000), SkBits2Float(0x4269d0e6), SkBits2Float(0x4200ac08), SkBits2Float(0x426a5605), SkBits2Float(0x4200b74c), SkBits2Float(0x426acfe0));  // 32.125f, 58.454f, 32.168f, 58.584f, 32.179f, 58.703f
path.cubicTo(SkBits2Float(0x4200c189), SkBits2Float(0x426b072c), SkBits2Float(0x4200b74c), SkBits2Float(0x426b49bb), SkBits2Float(0x42009581), SkBits2Float(0x426b6a80));  // 32.189f, 58.757f, 32.179f, 58.822f, 32.146f, 58.854f
path.moveTo(SkBits2Float(0x41eeae14), SkBits2Float(0x426bef9f));  // 29.835f, 58.984f
path.cubicTo(SkBits2Float(0x41ee8312), SkBits2Float(0x426c26eb), SkBits2Float(0x41ed353f), SkBits2Float(0x426c52f3), SkBits2Float(0x41ecc8b4), SkBits2Float(0x426c73b8));  // 29.814f, 59.038f, 29.651f, 59.081f, 29.598f, 59.113f
path.cubicTo(SkBits2Float(0x41eb7ae1), SkBits2Float(0x426cd70c), SkBits2Float(0x41ea3127), SkBits2Float(0x426d9376), SkBits2Float(0x41e96872), SkBits2Float(0x426e2e16));  // 29.435f, 59.21f, 29.274f, 59.394f, 29.176f, 59.545f
path.cubicTo(SkBits2Float(0x41e88b43), SkBits2Float(0x426ed3f9), SkBits2Float(0x41e7c49b), SkBits2Float(0x426fdd31), SkBits2Float(0x41e6a5e3), SkBits2Float(0x4270570c));  // 29.068f, 59.707f, 28.971f, 59.966f, 28.831f, 60.085f
path.cubicTo(SkBits2Float(0x41e678d5), SkBits2Float(0x427078d7), SkBits2Float(0x41e6624d), SkBits2Float(0x42706d93), SkBits2Float(0x41e620c4), SkBits2Float(0x427078d7));  // 28.809f, 60.118f, 28.798f, 60.107f, 28.766f, 60.118f
path.cubicTo(SkBits2Float(0x41e60a3d), SkBits2Float(0x4270841b), SkBits2Float(0x41e5f3b6), SkBits2Float(0x4270999c), SkBits2Float(0x41e5f3b6), SkBits2Float(0x4270999c));  // 28.755f, 60.129f, 28.744f, 60.15f, 28.744f, 60.15f
path.cubicTo(SkBits2Float(0x41e52d0e), SkBits2Float(0x4270d0e8), SkBits2Float(0x41e49374), SkBits2Float(0x4270e76f), SkBits2Float(0x41e39fbe), SkBits2Float(0x4270fcf0));  // 28.647f, 60.204f, 28.572f, 60.226f, 28.453f, 60.247f
path.cubicTo(SkBits2Float(0x41e2c28f), SkBits2Float(0x42711377), SkBits2Float(0x41e1a1ca), SkBits2Float(0x42714ac3), SkBits2Float(0x41e03f7c), SkBits2Float(0x4271343c));  // 28.345f, 60.269f, 28.204f, 60.323f, 28.031f, 60.301f
path.cubicTo(SkBits2Float(0x41de2d0d), SkBits2Float(0x42711377), SkBits2Float(0x41e0c49b), SkBits2Float(0x426e9caf), SkBits2Float(0x41e149b9), SkBits2Float(0x426e23da));  // 27.772f, 60.269f, 28.096f, 59.653f, 28.161f, 59.535f
path.cubicTo(SkBits2Float(0x41e23d6f), SkBits2Float(0x426d2f1e), SkBits2Float(0x41e38936), SkBits2Float(0x426c52f5), SkBits2Float(0x41e4eb84), SkBits2Float(0x426b8109));  // 28.28f, 59.296f, 28.442f, 59.081f, 28.615f, 58.876f
path.cubicTo(SkBits2Float(0x41e55a1c), SkBits2Float(0x426b49bd), SkBits2Float(0x41e5dd2e), SkBits2Float(0x426b1caf), SkBits2Float(0x41e6624d), SkBits2Float(0x426ae563));  // 28.669f, 58.822f, 28.733f, 58.778f, 28.798f, 58.724f
path.cubicTo(SkBits2Float(0x41e78312), SkBits2Float(0x426a77d2), SkBits2Float(0x41e88b43), SkBits2Float(0x4269fcf0), SkBits2Float(0x41e99580), SkBits2Float(0x42698f5f));  // 28.939f, 58.617f, 29.068f, 58.497f, 29.198f, 58.39f
path.cubicTo(SkBits2Float(0x41ea3126), SkBits2Float(0x42695813), SkBits2Float(0x41edd0e4), SkBits2Float(0x4267a7f2), SkBits2Float(0x41eeae13), SkBits2Float(0x42684292));  // 29.274f, 58.336f, 29.727f, 57.914f, 29.835f, 58.065f
path.cubicTo(SkBits2Float(0x41eeae13), SkBits2Float(0x42684292), SkBits2Float(0x41eec49a), SkBits2Float(0x42684edc), SkBits2Float(0x41eec49a), SkBits2Float(0x42685919));  // 29.835f, 58.065f, 29.846f, 58.077f, 29.846f, 58.087f
path.cubicTo(SkBits2Float(0x41ef0623), SkBits2Float(0x4268a6ec), SkBits2Float(0x41eedb21), SkBits2Float(0x426bb854), SkBits2Float(0x41eeae13), SkBits2Float(0x426befa0));  // 29.878f, 58.163f, 29.857f, 58.93f, 29.835f, 58.984f
path.moveTo(SkBits2Float(0x41eaf7cd), SkBits2Float(0x4258947d));  // 29.371f, 54.145f
path.cubicTo(SkBits2Float(0x41ebd4fc), SkBits2Float(0x425873b8), SkBits2Float(0x41ed353e), SkBits2Float(0x42589fc1), SkBits2Float(0x41edba5c), SkBits2Float(0x4258ab04));  // 29.479f, 54.113f, 29.651f, 54.156f, 29.716f, 54.167f
path.cubicTo(SkBits2Float(0x41ede76a), SkBits2Float(0x4259c9bc), SkBits2Float(0x41ee3f7b), SkBits2Float(0x425b6e9a), SkBits2Float(0x41ee126c), SkBits2Float(0x425c8314));  // 29.738f, 54.447f, 29.781f, 54.858f, 29.759f, 55.128f
path.cubicTo(SkBits2Float(0x41ede76a), SkBits2Float(0x425d343b), SkBits2Float(0x41ee5602), SkBits2Float(0x425dda1e), SkBits2Float(0x41edd0e3), SkBits2Float(0x425e74be));  // 29.738f, 55.301f, 29.792f, 55.463f, 29.727f, 55.614f
path.cubicTo(SkBits2Float(0x41ed624b), SkBits2Float(0x425f1aa1), SkBits2Float(0x41ec6e95), SkBits2Float(0x425f947c), SkBits2Float(0x41ebd4fc), SkBits2Float(0x426023d9));  // 29.673f, 55.776f, 29.554f, 55.895f, 29.479f, 56.035f
path.cubicTo(SkBits2Float(0x41eb22cf), SkBits2Float(0x4260c9bc), SkBits2Float(0x41ea5c27), SkBits2Float(0x4261645c), SkBits2Float(0x41e9957f), SkBits2Float(0x42621583));  // 29.392f, 56.197f, 29.295f, 56.348f, 29.198f, 56.521f
path.cubicTo(SkBits2Float(0x41e8e55e), SkBits2Float(0x4262c6aa), SkBits2Float(0x41e849b8), SkBits2Float(0x42638314), SkBits2Float(0x41e78310), SkBits2Float(0x426427f2));  // 29.112f, 56.694f, 29.036f, 56.878f, 28.939f, 57.039f
path.cubicTo(SkBits2Float(0x41e72b00), SkBits2Float(0x42646b88), SkBits2Float(0x41e6e76a), SkBits2Float(0x4264b854), SkBits2Float(0x41e68f5a), SkBits2Float(0x4264efa0));  // 28.896f, 57.105f, 28.863f, 57.18f, 28.82f, 57.234f
path.cubicTo(SkBits2Float(0x41e6624c), SkBits2Float(0x42651ba8), SkBits2Float(0x41e60a3b), SkBits2Float(0x4265322f), SkBits2Float(0x41e5dd2d), SkBits2Float(0x426552f4));  // 28.798f, 57.277f, 28.755f, 57.299f, 28.733f, 57.331f
path.cubicTo(SkBits2Float(0x41e570a2), SkBits2Float(0x4264ad11), SkBits2Float(0x41e620c3), SkBits2Float(0x4263c49e), SkBits2Float(0x41e6624c), SkBits2Float(0x426329fe));  // 28.68f, 57.169f, 28.766f, 56.942f, 28.798f, 56.791f
path.cubicTo(SkBits2Float(0x41e6a5e2), SkBits2Float(0x4262418b), SkBits2Float(0x41e6e76b), SkBits2Float(0x42617ae3), SkBits2Float(0x41e72b00), SkBits2Float(0x42609271));  // 28.831f, 56.564f, 28.863f, 56.37f, 28.896f, 56.143f
path.cubicTo(SkBits2Float(0x41e75602), SkBits2Float(0x42604fe2), SkBits2Float(0x41e7978b), SkBits2Float(0x425fe250), SkBits2Float(0x41e7c49a), SkBits2Float(0x425f9fc1));  // 28.917f, 56.078f, 28.949f, 55.971f, 28.971f, 55.906f
path.cubicTo(SkBits2Float(0x41e7db21), SkBits2Float(0x425f25e6), SkBits2Float(0x41e7db21), SkBits2Float(0x425ec18c), SkBits2Float(0x41e80623), SkBits2Float(0x425e53fa));  // 28.982f, 55.787f, 28.982f, 55.689f, 29.003f, 55.582f
path.lineTo(SkBits2Float(0x41e849b9), SkBits2Float(0x425e26ec));  // 29.036f, 55.538f
path.cubicTo(SkBits2Float(0x41e874bb), SkBits2Float(0x425da2d3), SkBits2Float(0x41e8b851), SkBits2Float(0x425d28f8), SkBits2Float(0x41e8e55f), SkBits2Float(0x425caf1d));  // 29.057f, 55.409f, 29.09f, 55.29f, 29.112f, 55.171f
path.cubicTo(SkBits2Float(0x41e93b63), SkBits2Float(0x425b8f5f), SkBits2Float(0x41e97ef9), SkBits2Float(0x425a7ae4), SkBits2Float(0x41ea0417), SkBits2Float(0x42596669));  // 29.154f, 54.89f, 29.187f, 54.62f, 29.252f, 54.35f
path.cubicTo(SkBits2Float(0x41ea3125), SkBits2Float(0x4259199c), SkBits2Float(0x41ea5c27), SkBits2Float(0x4258ab05), SkBits2Float(0x41eaf7cd), SkBits2Float(0x4258947d));  // 29.274f, 54.275f, 29.295f, 54.167f, 29.371f, 54.145f
path.moveTo(SkBits2Float(0x41e96871), SkBits2Float(0x4256a2d3));  // 29.176f, 53.659f
path.cubicTo(SkBits2Float(0x41e953f6), SkBits2Float(0x4256e562), SkBits2Float(0x41e96871), SkBits2Float(0x425727f2), SkBits2Float(0x41e93b63), SkBits2Float(0x42575f3d));  // 29.166f, 53.724f, 29.176f, 53.789f, 29.154f, 53.843f
path.cubicTo(SkBits2Float(0x41e8fbe6), SkBits2Float(0x42578002), SkBits2Float(0x41e88b42), SkBits2Float(0x42578002), SkBits2Float(0x41e81cab), SkBits2Float(0x42578002));  // 29.123f, 53.875f, 29.068f, 53.875f, 29.014f, 53.875f
path.cubicTo(SkBits2Float(0x41e7db22), SkBits2Float(0x42578002), SkBits2Float(0x41e78311), SkBits2Float(0x42576a81), SkBits2Float(0x41e75603), SkBits2Float(0x42575f3d));  // 28.982f, 53.875f, 28.939f, 53.854f, 28.917f, 53.843f
path.cubicTo(SkBits2Float(0x41e72b01), SkBits2Float(0x4257322f), SkBits2Float(0x41e72b01), SkBits2Float(0x4257322f), SkBits2Float(0x41e72b01), SkBits2Float(0x4256fbe9));  // 28.896f, 53.799f, 28.896f, 53.799f, 28.896f, 53.746f
path.cubicTo(SkBits2Float(0x41e72b01), SkBits2Float(0x4256b95a), SkBits2Float(0x41e78311), SkBits2Float(0x42564ac2), SkBits2Float(0x41e7978c), SkBits2Float(0x42561376));  // 28.896f, 53.681f, 28.939f, 53.573f, 28.949f, 53.519f
path.cubicTo(SkBits2Float(0x41e7db22), SkBits2Float(0x4255570c), SkBits2Float(0x41e80624), SkBits2Float(0x4254b128), SkBits2Float(0x41e86040), SkBits2Float(0x42540b45));  // 28.982f, 53.335f, 29.003f, 53.173f, 29.047f, 53.011f
path.cubicTo(SkBits2Float(0x41e874bb), SkBits2Float(0x4253cac2), SkBits2Float(0x41e86040), SkBits2Float(0x4253916a), SkBits2Float(0x41e8b850), SkBits2Float(0x42536562));  // 29.057f, 52.948f, 29.047f, 52.892f, 29.09f, 52.849f
path.cubicTo(SkBits2Float(0x41e8ced7), SkBits2Float(0x42534fe1), SkBits2Float(0x41e953f6), SkBits2Float(0x42532e16), SkBits2Float(0x41e97ef8), SkBits2Float(0x42532e16));  // 29.101f, 52.828f, 29.166f, 52.795f, 29.187f, 52.795f
path.cubicTo(SkBits2Float(0x41ea0417), SkBits2Float(0x425323d9), SkBits2Float(0x41ea3125), SkBits2Float(0x42534fe1), SkBits2Float(0x41ea72ae), SkBits2Float(0x42535a1e));  // 29.252f, 52.785f, 29.274f, 52.828f, 29.306f, 52.838f
path.cubicTo(SkBits2Float(0x41ea72ae), SkBits2Float(0x42548520), SkBits2Float(0x41e9d708), SkBits2Float(0x4255a4df), SkBits2Float(0x41e96871), SkBits2Float(0x4256a2d2));  // 29.306f, 53.13f, 29.23f, 53.411f, 29.176f, 53.659f
path.moveTo(SkBits2Float(0x41e874bb), SkBits2Float(0x4258b647));  // 29.057f, 54.178f
path.cubicTo(SkBits2Float(0x41e86040), SkBits2Float(0x42595c2a), SkBits2Float(0x41e849b9), SkBits2Float(0x4259bf7e), SkBits2Float(0x41e80623), SkBits2Float(0x425a4eda));  // 29.047f, 54.34f, 29.036f, 54.437f, 29.003f, 54.577f
path.cubicTo(SkBits2Float(0x41e7db21), SkBits2Float(0x425ad3f9), SkBits2Float(0x41e76c89), SkBits2Float(0x425b8520), SkBits2Float(0x41e72b00), SkBits2Float(0x425c147c));  // 28.982f, 54.707f, 28.928f, 54.88f, 28.896f, 55.02f
path.cubicTo(SkBits2Float(0x41e71479), SkBits2Float(0x425c570b), SkBits2Float(0x41e72b00), SkBits2Float(0x425c77d0), SkBits2Float(0x41e71479), SkBits2Float(0x425cba5f));  // 28.885f, 55.085f, 28.896f, 55.117f, 28.885f, 55.182f
path.cubicTo(SkBits2Float(0x41e68f5a), SkBits2Float(0x425dfae2), SkBits2Float(0x41e5dd2d), SkBits2Float(0x425f676d), SkBits2Float(0x41e570a2), SkBits2Float(0x4260a8f7));  // 28.82f, 55.495f, 28.733f, 55.851f, 28.68f, 56.165f
path.cubicTo(SkBits2Float(0x41e52d0c), SkBits2Float(0x42610c4b), SkBits2Float(0x41e55a1b), SkBits2Float(0x42614eda), SkBits2Float(0x41e52d0c), SkBits2Float(0x42619ba7));  // 28.647f, 56.262f, 28.669f, 56.327f, 28.647f, 56.402f
path.cubicTo(SkBits2Float(0x41e51685), SkBits2Float(0x4261f4be), SkBits2Float(0x41e4be74), SkBits2Float(0x42624cce), SkBits2Float(0x41e4a7ed), SkBits2Float(0x42628f5d));  // 28.636f, 56.489f, 28.593f, 56.575f, 28.582f, 56.64f
path.cubicTo(SkBits2Float(0x41e46664), SkBits2Float(0x42634bc7), SkBits2Float(0x41e43b62), SkBits2Float(0x4263e667), SkBits2Float(0x41e3f7cc), SkBits2Float(0x4264a1cc));  // 28.55f, 56.824f, 28.529f, 56.975f, 28.496f, 57.158f
path.cubicTo(SkBits2Float(0x41e39fbc), SkBits2Float(0x42657efb), SkBits2Float(0x41e31a9d), SkBits2Float(0x42669376), SkBits2Float(0x41e2ac05), SkBits2Float(0x426770a5));  // 28.453f, 57.374f, 28.388f, 57.644f, 28.334f, 57.86f
path.cubicTo(SkBits2Float(0x41e27ef7), SkBits2Float(0x426821cc), SkBits2Float(0x41e253f5), SkBits2Float(0x4268bc6c), SkBits2Float(0x41e2105f), SkBits2Float(0x42695812));  // 28.312f, 58.033f, 28.291f, 58.184f, 28.258f, 58.336f
path.cubicTo(SkBits2Float(0x41e1ced6), SkBits2Float(0x4269f2b2), SkBits2Float(0x41e1082e), SkBits2Float(0x426aa3d9), SkBits2Float(0x41e09996), SkBits2Float(0x426b3335));  // 28.226f, 58.487f, 28.129f, 58.66f, 28.075f, 58.8f
path.lineTo(SkBits2Float(0x41e05600), SkBits2Float(0x426b3e79));  // 28.042f, 58.811f
path.cubicTo(SkBits2Float(0x41dfe768), SkBits2Float(0x426bb854), SkBits2Float(0x41dfba5a), SkBits2Float(0x426c3129), SkBits2Float(0x41df4dcf), SkBits2Float(0x426ccccf));  // 27.988f, 58.93f, 27.966f, 59.048f, 27.913f, 59.2f
path.cubicTo(SkBits2Float(0x41def5bf), SkBits2Float(0x426d50e8), SkBits2Float(0x41de5a19), SkBits2Float(0x426de14a), SkBits2Float(0x41ddeb81), SkBits2Float(0x426e70a6));  // 27.87f, 59.329f, 27.794f, 59.47f, 27.74f, 59.61f
path.cubicTo(SkBits2Float(0x41dd3b60), SkBits2Float(0x426f79dd), SkBits2Float(0x41dd4fdb), SkBits2Float(0x426e1896), SkBits2Float(0x41dd6662), SkBits2Float(0x426db43c));  // 27.654f, 59.869f, 27.664f, 59.524f, 27.675f, 59.426f
path.cubicTo(SkBits2Float(0x41de9ba1), SkBits2Float(0x426aa3da), SkBits2Float(0x41e01476), SkBits2Float(0x42679271), SkBits2Float(0x41e1332f), SkBits2Float(0x42648109));  // 27.826f, 58.66f, 28.01f, 57.893f, 28.15f, 57.126f
path.cubicTo(SkBits2Float(0x41e149b6), SkBits2Float(0x42645f3e), SkBits2Float(0x41e1a1c7), SkBits2Float(0x4264072e), SkBits2Float(0x41e1a1c7), SkBits2Float(0x4263f1ad));  // 28.161f, 57.093f, 28.204f, 57.007f, 28.204f, 56.986f
path.cubicTo(SkBits2Float(0x41e253f4), SkBits2Float(0x42626e9b), SkBits2Float(0x41e2c28c), SkBits2Float(0x42610109), SkBits2Float(0x41e3459e), SkBits2Float(0x425f72b3));  // 28.291f, 56.608f, 28.345f, 56.251f, 28.409f, 55.862f
path.cubicTo(SkBits2Float(0x41e372ac), SkBits2Float(0x425f51ee), SkBits2Float(0x41e3b642), SkBits2Float(0x425ef9de), SkBits2Float(0x41e3b642), SkBits2Float(0x425ed813));  // 28.431f, 55.83f, 28.464f, 55.744f, 28.464f, 55.711f
path.cubicTo(SkBits2Float(0x41e46663), SkBits2Float(0x425d76cb), SkBits2Float(0x41e4be73), SkBits2Float(0x425c3542), SkBits2Float(0x41e570a0), SkBits2Float(0x425ad3fa));  // 28.55f, 55.366f, 28.593f, 55.052f, 28.68f, 54.707f
path.cubicTo(SkBits2Float(0x41e570a0), SkBits2Float(0x425a916b), SkBits2Float(0x41e5dd2b), SkBits2Float(0x425a22d3), SkBits2Float(0x41e5f3b2), SkBits2Float(0x4259e044));  // 28.68f, 54.642f, 28.733f, 54.534f, 28.744f, 54.469f
path.cubicTo(SkBits2Float(0x41e620c0), SkBits2Float(0x42595c2b), SkBits2Float(0x41e60a39), SkBits2Float(0x4258ab05), SkBits2Float(0x41e72afe), SkBits2Float(0x4258947d));  // 28.766f, 54.34f, 28.755f, 54.167f, 28.896f, 54.145f
path.cubicTo(SkBits2Float(0x41e79789), SkBits2Float(0x4258947d), SkBits2Float(0x41e80621), SkBits2Float(0x4258ab04), SkBits2Float(0x41e874b8), SkBits2Float(0x4258b648));  // 28.949f, 54.145f, 29.003f, 54.167f, 29.057f, 54.178f
path.moveTo(SkBits2Float(0x41e5b229), SkBits2Float(0x4256a2d3));  // 28.712f, 53.659f
path.cubicTo(SkBits2Float(0x41e5851b), SkBits2Float(0x4256e562), SkBits2Float(0x41e59ba2), SkBits2Float(0x425727f2), SkBits2Float(0x41e570a0), SkBits2Float(0x42575f3d));  // 28.69f, 53.724f, 28.701f, 53.789f, 28.68f, 53.843f
path.cubicTo(SkBits2Float(0x41e52d0a), SkBits2Float(0x42578002), SkBits2Float(0x41e4a7ec), SkBits2Float(0x42579689), SkBits2Float(0x41e43b61), SkBits2Float(0x42578002));  // 28.647f, 53.875f, 28.582f, 53.897f, 28.529f, 53.875f
path.cubicTo(SkBits2Float(0x41e3f7cb), SkBits2Float(0x42578002), SkBits2Float(0x41e39fbb), SkBits2Float(0x425748b6), SkBits2Float(0x41e3459e), SkBits2Float(0x42573e79));  // 28.496f, 53.875f, 28.453f, 53.821f, 28.409f, 53.811f
path.cubicTo(SkBits2Float(0x41e39fbb), SkBits2Float(0x42566044), SkBits2Float(0x41e40e52), SkBits2Float(0x42558e58), SkBits2Float(0x41e47add), SkBits2Float(0x4254c7b0));  // 28.453f, 53.594f, 28.507f, 53.389f, 28.56f, 53.195f
path.cubicTo(SkBits2Float(0x41e49370), SkBits2Float(0x425479dd), SkBits2Float(0x41e49370), SkBits2Float(0x42541689), SkBits2Float(0x41e4eb81), SkBits2Float(0x4253df3d));  // 28.572f, 53.119f, 28.572f, 53.022f, 28.615f, 52.968f
path.cubicTo(SkBits2Float(0x41e4fffc), SkBits2Float(0x4253c9bc), SkBits2Float(0x41e5b229), SkBits2Float(0x4253916a), SkBits2Float(0x41e60a39), SkBits2Float(0x4253916a));  // 28.625f, 52.947f, 28.712f, 52.892f, 28.755f, 52.892f
path.cubicTo(SkBits2Float(0x41e68f58), SkBits2Float(0x4253872d), SkBits2Float(0x41e68f58), SkBits2Float(0x4253a7f1), SkBits2Float(0x41e6e768), SkBits2Float(0x4253be78));  // 28.82f, 52.882f, 28.82f, 52.914f, 28.863f, 52.936f
path.cubicTo(SkBits2Float(0x41e68f58), SkBits2Float(0x4254c7af), SkBits2Float(0x41e60a39), SkBits2Float(0x4255af1c), SkBits2Float(0x41e5b229), SkBits2Float(0x4256a2d2));  // 28.82f, 53.195f, 28.755f, 53.421f, 28.712f, 53.659f
path.moveTo(SkBits2Float(0x41e372ac), SkBits2Float(0x42589fc0));  // 28.431f, 54.156f
path.cubicTo(SkBits2Float(0x41e55a19), SkBits2Float(0x42586874), SkBits2Float(0x41e40e52), SkBits2Float(0x425a178f), SkBits2Float(0x41e3cabc), SkBits2Float(0x425a7ae3));  // 28.669f, 54.102f, 28.507f, 54.523f, 28.474f, 54.62f
path.cubicTo(SkBits2Float(0x41e1fbe3), SkBits2Float(0x425f3b66), SkBits2Float(0x41dfd0e1), SkBits2Float(0x4263f1ac), SkBits2Float(0x41ddeb81), SkBits2Float(0x4268c7b0));  // 28.248f, 55.808f, 27.977f, 56.986f, 27.74f, 58.195f
path.cubicTo(SkBits2Float(0x41ddd4fa), SkBits2Float(0x42690a3f), SkBits2Float(0x41dd7ce9), SkBits2Float(0x42696d93), SkBits2Float(0x41dd6662), SkBits2Float(0x4269999c));  // 27.729f, 58.26f, 27.686f, 58.357f, 27.675f, 58.4f
path.cubicTo(SkBits2Float(0x41dd3b60), SkBits2Float(0x426a29fe), SkBits2Float(0x41dd3b60), SkBits2Float(0x426a8d52), SkBits2Float(0x41dcf7ca), SkBits2Float(0x426b1cae));  // 27.654f, 58.541f, 27.654f, 58.638f, 27.621f, 58.778f
path.cubicTo(SkBits2Float(0x41dcb641), SkBits2Float(0x426bf9dd), SkBits2Float(0x41dc0414), SkBits2Float(0x426cf8d7), SkBits2Float(0x41db957c), SkBits2Float(0x426dec8d));  // 27.589f, 58.994f, 27.502f, 59.243f, 27.448f, 59.481f
path.cubicTo(SkBits2Float(0x41db53f3), SkBits2Float(0x426e916a), SkBits2Float(0x41db3d6c), SkBits2Float(0x426eea81), SkBits2Float(0x41daa3d2), SkBits2Float(0x426f5918));  // 27.416f, 59.642f, 27.405f, 59.729f, 27.33f, 59.837f
path.cubicTo(SkBits2Float(0x41da76c4), SkBits2Float(0x426f4dd4), SkBits2Float(0x41da49b5), SkBits2Float(0x426f4291), SkBits2Float(0x41da082c), SkBits2Float(0x426f21cc));  // 27.308f, 59.826f, 27.286f, 59.815f, 27.254f, 59.783f
path.cubicTo(SkBits2Float(0x41d9db1e), SkBits2Float(0x426f0b45), SkBits2Float(0x41d9f1a5), SkBits2Float(0x426f0b45), SkBits2Float(0x41d9c6a3), SkBits2Float(0x426eea80));  // 27.232f, 59.761f, 27.243f, 59.761f, 27.222f, 59.729f
path.lineTo(SkBits2Float(0x41d99995), SkBits2Float(0x426edf3c));  // 27.2f, 59.718f
path.cubicTo(SkBits2Float(0x41d91476), SkBits2Float(0x426ea7f0), SkBits2Float(0x41d8e768), SkBits2Float(0x426e6561), SkBits2Float(0x41d8a5df), SkBits2Float(0x426e020d));  // 27.135f, 59.664f, 27.113f, 59.599f, 27.081f, 59.502f
path.cubicTo(SkBits2Float(0x41d8a5df), SkBits2Float(0x426e020d), SkBits2Float(0x41d86456), SkBits2Float(0x426dd605), SkBits2Float(0x41d8a5df), SkBits2Float(0x426dd605));  // 27.081f, 59.502f, 27.049f, 59.459f, 27.081f, 59.459f
path.cubicTo(SkBits2Float(0x41d8e768), SkBits2Float(0x426d5c2a), SkBits2Float(0x41d8fdef), SkBits2Float(0x426cf8d6), SkBits2Float(0x41d92afe), SkBits2Float(0x426c7efb));  // 27.113f, 59.34f, 27.124f, 59.243f, 27.146f, 59.124f
path.cubicTo(SkBits2Float(0x41d9830e), SkBits2Float(0x426bb853), SkBits2Float(0x41da1eb4), SkBits2Float(0x426ae561), SkBits2Float(0x41da8d4c), SkBits2Float(0x426a29fd));  // 27.189f, 58.93f, 27.265f, 58.724f, 27.319f, 58.541f
path.cubicTo(SkBits2Float(0x41dccabd), SkBits2Float(0x4265d811), SkBits2Float(0x41e02afe), SkBits2Float(0x42617ae2), SkBits2Float(0x41e1332f), SkBits2Float(0x425cfcef));  // 27.599f, 57.461f, 28.021f, 56.37f, 28.15f, 55.247f
path.cubicTo(SkBits2Float(0x41e149b6), SkBits2Float(0x425c4085), SkBits2Float(0x41e1a1c7), SkBits2Float(0x425b8f5e), SkBits2Float(0x41e1fbe3), SkBits2Float(0x425adf3d));  // 28.161f, 55.063f, 28.204f, 54.89f, 28.248f, 54.718f
path.cubicTo(SkBits2Float(0x41e226e5), SkBits2Float(0x425a4edb), SkBits2Float(0x41e226e5), SkBits2Float(0x42598833), SkBits2Float(0x41e2ac04), SkBits2Float(0x4258f7d0));  // 28.269f, 54.577f, 28.269f, 54.383f, 28.334f, 54.242f
path.cubicTo(SkBits2Float(0x41e2c28b), SkBits2Float(0x4258ec8c), SkBits2Float(0x41e372ac), SkBits2Float(0x42589fc0), SkBits2Float(0x41e372ac), SkBits2Float(0x42589fc0));  // 28.345f, 54.231f, 28.431f, 54.156f, 28.431f, 54.156f
path.moveTo(SkBits2Float(0x41d9830e), SkBits2Float(0x427128f7));  // 27.189f, 60.29f
path.cubicTo(SkBits2Float(0x41d95600), SkBits2Float(0x42714ac2), SkBits2Float(0x41d92afe), SkBits2Float(0x427176ca), SkBits2Float(0x41d8e768), SkBits2Float(0x427176ca));  // 27.167f, 60.323f, 27.146f, 60.366f, 27.113f, 60.366f
path.cubicTo(SkBits2Float(0x41d86456), SkBits2Float(0x42718d51), SkBits2Float(0x41d67ce9), SkBits2Float(0x4271820e), SkBits2Float(0x41d60e51), SkBits2Float(0x42716b86));  // 27.049f, 60.388f, 26.811f, 60.377f, 26.757f, 60.355f
path.cubicTo(SkBits2Float(0x41d5f7ca), SkBits2Float(0x42716b86), SkBits2Float(0x41d5ccc8), SkBits2Float(0x42714ac1), SkBits2Float(0x41d5b641), SkBits2Float(0x42713f7e));  // 26.746f, 60.355f, 26.725f, 60.323f, 26.714f, 60.312f
path.cubicTo(SkBits2Float(0x41d5b641), SkBits2Float(0x42708e57), SkBits2Float(0x41d5f7ca), SkBits2Float(0x426ffefb), SkBits2Float(0x41d69370), SkBits2Float(0x426f8f5d));  // 26.714f, 60.139f, 26.746f, 59.999f, 26.822f, 59.89f
path.cubicTo(SkBits2Float(0x41d6eb80), SkBits2Float(0x426f9ba7), SkBits2Float(0x41d7188f), SkBits2Float(0x426f8f5d), SkBits2Float(0x41d7709f), SkBits2Float(0x426f9ba7));  // 26.865f, 59.902f, 26.887f, 59.89f, 26.93f, 59.902f
path.cubicTo(SkBits2Float(0x41d7b228), SkBits2Float(0x426fb128), SkBits2Float(0x41d99995), SkBits2Float(0x42706d93), SkBits2Float(0x41d9c6a3), SkBits2Float(0x42708e57));  // 26.962f, 59.923f, 27.2f, 60.107f, 27.222f, 60.139f
path.cubicTo(SkBits2Float(0x41d9db1e), SkBits2Float(0x4270d0e6), SkBits2Float(0x41d99995), SkBits2Float(0x42710832), SkBits2Float(0x41d9830d), SkBits2Float(0x427128f7));  // 27.232f, 60.204f, 27.2f, 60.258f, 27.189f, 60.29f
path.moveTo(SkBits2Float(0x41e1603c), SkBits2Float(0x4255f1ab));  // 28.172f, 53.486f
path.cubicTo(SkBits2Float(0x41e149b5), SkBits2Float(0x42563f7e), SkBits2Float(0x41e1603c), SkBits2Float(0x425676ca), SkBits2Float(0x41e1332e), SkBits2Float(0x4256c49d));  // 28.161f, 53.562f, 28.172f, 53.616f, 28.15f, 53.692f
path.cubicTo(SkBits2Float(0x41e11eb3), SkBits2Float(0x4256f0a5), SkBits2Float(0x41e0db1e), SkBits2Float(0x425727f1), SkBits2Float(0x41e0b01c), SkBits2Float(0x425748b6));  // 28.14f, 53.735f, 28.107f, 53.789f, 28.086f, 53.821f
path.lineTo(SkBits2Float(0x41e055ff), SkBits2Float(0x425748b6));  // 28.042f, 53.821f
path.cubicTo(SkBits2Float(0x41e055ff), SkBits2Float(0x425748b6), SkBits2Float(0x41df4dce), SkBits2Float(0x4256e562), SkBits2Float(0x41df3747), SkBits2Float(0x4256da1e));  // 28.042f, 53.821f, 27.913f, 53.724f, 27.902f, 53.713f
path.cubicTo(SkBits2Float(0x41deb228), SkBits2Float(0x4256820e), SkBits2Float(0x41de4391), SkBits2Float(0x42561376), SkBits2Float(0x41ddbe72), SkBits2Float(0x4255ba60));  // 27.837f, 53.627f, 27.783f, 53.519f, 27.718f, 53.432f
path.lineTo(SkBits2Float(0x41dd7ce9), SkBits2Float(0x4255af1c));  // 27.686f, 53.421f
path.cubicTo(SkBits2Float(0x41dd7ce9), SkBits2Float(0x4255a4df), SkBits2Float(0x41dda7eb), SkBits2Float(0x425578d6), SkBits2Float(0x41dd7ce9), SkBits2Float(0x42556d93));  // 27.686f, 53.411f, 27.707f, 53.368f, 27.686f, 53.357f
path.cubicTo(SkBits2Float(0x41de9ba1), SkBits2Float(0x4255147c), SkBits2Float(0x41df8f58), SkBits2Float(0x4254c7b0), SkBits2Float(0x41e0b01c), SkBits2Float(0x42549064));  // 27.826f, 53.27f, 27.945f, 53.195f, 28.086f, 53.141f
path.cubicTo(SkBits2Float(0x41e0c497), SkBits2Float(0x42548520), SkBits2Float(0x41e11eb4), SkBits2Float(0x4254645c), SkBits2Float(0x41e1332e), SkBits2Float(0x4254645c));  // 28.096f, 53.13f, 28.14f, 53.098f, 28.15f, 53.098f
path.cubicTo(SkBits2Float(0x41e18b3e), SkBits2Float(0x42545918), SkBits2Float(0x41e1ced4), SkBits2Float(0x425479dd), SkBits2Float(0x41e1fbe2), SkBits2Float(0x425479dd));  // 28.193f, 53.087f, 28.226f, 53.119f, 28.248f, 53.119f
path.cubicTo(SkBits2Float(0x41e1fbe2), SkBits2Float(0x4255147d), SkBits2Float(0x41e1a1c5), SkBits2Float(0x4255841a), SkBits2Float(0x41e1603c), SkBits2Float(0x4255f1ac));  // 28.248f, 53.27f, 28.204f, 53.379f, 28.172f, 53.486f
path.moveTo(SkBits2Float(0x41df6248), SkBits2Float(0x425b4ccf));  // 27.923f, 54.825f
path.cubicTo(SkBits2Float(0x41dfe767), SkBits2Float(0x425b9aa2), SkBits2Float(0x41df4dcd), SkBits2Float(0x425c6c8d), SkBits2Float(0x41df20bf), SkBits2Float(0x425cd0e8));  // 27.988f, 54.901f, 27.913f, 55.106f, 27.891f, 55.204f
path.cubicTo(SkBits2Float(0x41ddeb80), SkBits2Float(0x425f893a), SkBits2Float(0x41dc8932), SkBits2Float(0x4262374e), SkBits2Float(0x41db105d), SkBits2Float(0x4264e45d));  // 27.74f, 55.884f, 27.567f, 56.554f, 27.383f, 57.223f
path.cubicTo(SkBits2Float(0x41daced4), SkBits2Float(0x42657efd), SkBits2Float(0x41d78726), SkBits2Float(0x426c52f5), SkBits2Float(0x41d6c07e), SkBits2Float(0x426c3c6d));  // 27.351f, 57.374f, 26.941f, 59.081f, 26.844f, 59.059f
path.cubicTo(SkBits2Float(0x41d58932), SkBits2Float(0x426c3129), SkBits2Float(0x41d50620), SkBits2Float(0x426b1caf), SkBits2Float(0x41d48101), SkBits2Float(0x426aa3da));  // 26.692f, 59.048f, 26.628f, 58.778f, 26.563f, 58.66f
path.cubicTo(SkBits2Float(0x41d3d0e0), SkBits2Float(0x426a0834), SkBits2Float(0x41d34bc2), SkBits2Float(0x42696d94), SkBits2Float(0x41d2db1e), SkBits2Float(0x4268bc6d));  // 26.477f, 58.508f, 26.412f, 58.357f, 26.357f, 58.184f
path.cubicTo(SkBits2Float(0x41d21476), SkBits2Float(0x42674fe2), SkBits2Float(0x41d19164), SkBits2Float(0x4265c18c), SkBits2Float(0x41d19164), SkBits2Float(0x426449bd));  // 26.26f, 57.828f, 26.196f, 57.439f, 26.196f, 57.072f
path.cubicTo(SkBits2Float(0x41d1a5df), SkBits2Float(0x4261bd73), SkBits2Float(0x41d3d0e1), SkBits2Float(0x425f51ee), SkBits2Float(0x41d79dae), SkBits2Float(0x425d820f));  // 26.206f, 56.435f, 26.477f, 55.83f, 26.952f, 55.377f
path.cubicTo(SkBits2Float(0x41d8a5df), SkBits2Float(0x425cfcf0), SkBits2Float(0x41d9db1f), SkBits2Float(0x425c8e59), SkBits2Float(0x41db105e), SkBits2Float(0x425c3542));  // 27.081f, 55.247f, 27.232f, 55.139f, 27.383f, 55.052f
path.cubicTo(SkBits2Float(0x41dbed8d), SkBits2Float(0x425bf4bf), SkBits2Float(0x41ddbe72), SkBits2Float(0x425b21cd), SkBits2Float(0x41dec8b0), SkBits2Float(0x425b21cd));  // 27.491f, 54.989f, 27.718f, 54.783f, 27.848f, 54.783f
path.cubicTo(SkBits2Float(0x41df20c0), SkBits2Float(0x425b374e), SkBits2Float(0x41df4dcf), SkBits2Float(0x425b4292), SkBits2Float(0x41df624a), SkBits2Float(0x425b4ccf));  // 27.891f, 54.804f, 27.913f, 54.815f, 27.923f, 54.825f
path.moveTo(SkBits2Float(0x41d453f4), SkBits2Float(0x426fbc6d));  // 26.541f, 59.934f
path.cubicTo(SkBits2Float(0x41d48102), SkBits2Float(0x426f8521), SkBits2Float(0x41d51a9c), SkBits2Float(0x426ea7f2), SkBits2Float(0x41d4957d), SkBits2Float(0x426e872e));  // 26.563f, 59.88f, 26.638f, 59.664f, 26.573f, 59.632f
path.cubicTo(SkBits2Float(0x41d453f4), SkBits2Float(0x426e70a7), SkBits2Float(0x41d428f2), SkBits2Float(0x426e872e), SkBits2Float(0x41d3fbe3), SkBits2Float(0x426e70a7));  // 26.541f, 59.61f, 26.52f, 59.632f, 26.498f, 59.61f
path.cubicTo(SkBits2Float(0x41d3d0e1), SkBits2Float(0x426e916c), SkBits2Float(0x41d3b84d), SkBits2Float(0x426e872e), SkBits2Float(0x41d3a3d3), SkBits2Float(0x426e916c));  // 26.477f, 59.642f, 26.465f, 59.632f, 26.455f, 59.642f
path.cubicTo(SkBits2Float(0x41d3603d), SkBits2Float(0x426f010a), SkBits2Float(0x41d3d0e1), SkBits2Float(0x426f9ba9), SkBits2Float(0x41d4126b), SkBits2Float(0x426fdd33));  // 26.422f, 59.751f, 26.477f, 59.902f, 26.509f, 59.966f
path.lineTo(SkBits2Float(0x41d43d6d), SkBits2Float(0x426fdd33));  // 26.53f, 59.966f
path.cubicTo(SkBits2Float(0x41d43d6d), SkBits2Float(0x426fd1ef), SkBits2Float(0x41d43d6d), SkBits2Float(0x426fbc6e), SkBits2Float(0x41d453f4), SkBits2Float(0x426fbc6e));  // 26.53f, 59.955f, 26.53f, 59.934f, 26.541f, 59.934f
path.moveTo(SkBits2Float(0x42071ba4), SkBits2Float(0x42670210));  // 33.777f, 57.752f
path.cubicTo(SkBits2Float(0x42075e33), SkBits2Float(0x42670d54), SkBits2Float(0x4207957f), SkBits2Float(0x42671897), SkBits2Float(0x4207cccb), SkBits2Float(0x42672f1e));  // 33.842f, 57.763f, 33.896f, 57.774f, 33.95f, 57.796f
path.cubicTo(SkBits2Float(0x4208a9fa), SkBits2Float(0x4267872e), SkBits2Float(0x42097be6), SkBits2Float(0x42681791), SkBits2Float(0x420a3850), SkBits2Float(0x42688522));  // 34.166f, 57.882f, 34.371f, 58.023f, 34.555f, 58.13f
path.cubicTo(SkBits2Float(0x420b0a3c), SkBits2Float(0x4268fefd), SkBits2Float(0x420d1167), SkBits2Float(0x4269e770), SkBits2Float(0x420d27ee), SkBits2Float(0x426ae564));  // 34.76f, 58.249f, 35.267f, 58.476f, 35.289f, 58.724f
path.cubicTo(SkBits2Float(0x420d0729), SkBits2Float(0x426af0a8), SkBits2Float(0x420cdb21), SkBits2Float(0x426afbeb), SkBits2Float(0x420cb956), SkBits2Float(0x426b072f));  // 35.257f, 58.735f, 35.214f, 58.746f, 35.181f, 58.757f
path.cubicTo(SkBits2Float(0x420b9998), SkBits2Float(0x426b27f4), SkBits2Float(0x420a6f9c), SkBits2Float(0x426b27f4), SkBits2Float(0x42095b21), SkBits2Float(0x426b3337));  // 34.9f, 58.789f, 34.609f, 58.789f, 34.339f, 58.8f
path.cubicTo(SkBits2Float(0x42090d4e), SkBits2Float(0x426b3337), SkBits2Float(0x4207b644), SkBits2Float(0x426b49be), SkBits2Float(0x420773b4), SkBits2Float(0x426b3337));  // 34.263f, 58.8f, 33.928f, 58.822f, 33.863f, 58.8f
path.cubicTo(SkBits2Float(0x4207322b), SkBits2Float(0x426b072f), SkBits2Float(0x4206ef9b), SkBits2Float(0x4269999d), SkBits2Float(0x4206ced7), SkBits2Float(0x426920c8));  // 33.799f, 58.757f, 33.734f, 58.4f, 33.702f, 58.282f
path.cubicTo(SkBits2Float(0x42069685), SkBits2Float(0x4268645e), SkBits2Float(0x4205c49a), SkBits2Float(0x4266b543), SkBits2Float(0x42071ba4), SkBits2Float(0x42670210));  // 33.647f, 58.098f, 33.442f, 57.677f, 33.777f, 57.752f
path.moveTo(SkBits2Float(0x41f026e6), SkBits2Float(0x423f0c4e));  // 30.019f, 47.762f
path.cubicTo(SkBits2Float(0x41effbe4), SkBits2Float(0x42400004), SkBits2Float(0x41f0105f), SkBits2Float(0x4240e877), SkBits2Float(0x41f03b61), SkBits2Float(0x4241d0ea));  // 29.998f, 48, 30.008f, 48.227f, 30.029f, 48.454f
path.lineTo(SkBits2Float(0x41f03b61), SkBits2Float(0x424228fa));  // 30.029f, 48.54f
path.cubicTo(SkBits2Float(0x41f051e8), SkBits2Float(0x42423f81), SkBits2Float(0x41f0c080), SkBits2Float(0x424276cd), SkBits2Float(0x41f11890), SkBits2Float(0x424276cd));  // 30.04f, 48.562f, 30.094f, 48.616f, 30.137f, 48.616f
path.cubicTo(SkBits2Float(0x41f11890), SkBits2Float(0x424276cd), SkBits2Float(0x41f18934), SkBits2Float(0x42426b89), SkBits2Float(0x41f12f17), SkBits2Float(0x42426b89));  // 30.137f, 48.616f, 30.192f, 48.605f, 30.148f, 48.605f
path.cubicTo(SkBits2Float(0x41f1459e), SkBits2Float(0x42426045), SkBits2Float(0x41f18934), SkBits2Float(0x42426b89), SkBits2Float(0x41f18934), SkBits2Float(0x42426b89));  // 30.159f, 48.594f, 30.192f, 48.605f, 30.192f, 48.605f
path.cubicTo(SkBits2Float(0x41f19daf), SkBits2Float(0x424249be), SkBits2Float(0x41f19daf), SkBits2Float(0x42423f81), SkBits2Float(0x41f1b436), SkBits2Float(0x42423f81));  // 30.202f, 48.572f, 30.202f, 48.562f, 30.213f, 48.562f
path.cubicTo(SkBits2Float(0x41f18934), SkBits2Float(0x42414087), SkBits2Float(0x41f11890), SkBits2Float(0x424079df), SkBits2Float(0x41f0ac05), SkBits2Float(0x423f9cb0));  // 30.192f, 48.313f, 30.137f, 48.119f, 30.084f, 47.903f
path.cubicTo(SkBits2Float(0x41f0957e), SkBits2Float(0x423f7ae5), SkBits2Float(0x41f0c080), SkBits2Float(0x423f5a21), SkBits2Float(0x41f0957e), SkBits2Float(0x423f395c));  // 30.073f, 47.87f, 30.094f, 47.838f, 30.073f, 47.806f
path.lineTo(SkBits2Float(0x41f026e6), SkBits2Float(0x423f0c4e));  // 30.019f, 47.762f
path.moveTo(SkBits2Float(0x41ed4dcf), SkBits2Float(0x423fd3fc));  // 29.663f, 47.957f
path.cubicTo(SkBits2Float(0x41ecc8b0), SkBits2Float(0x42408523), SkBits2Float(0x41ec580c), SkBits2Float(0x42414bcb), SkBits2Float(0x41ec580c), SkBits2Float(0x42423f81));  // 29.598f, 48.13f, 29.543f, 48.324f, 29.543f, 48.562f
path.cubicTo(SkBits2Float(0x41ec6e93), SkBits2Float(0x42423f81), SkBits2Float(0x41ec9ba2), SkBits2Float(0x42426046), SkBits2Float(0x41ecb229), SkBits2Float(0x42426b89));  // 29.554f, 48.562f, 29.576f, 48.594f, 29.587f, 48.605f
path.cubicTo(SkBits2Float(0x41ecc8b0), SkBits2Float(0x42426b89), SkBits2Float(0x41ecdd2b), SkBits2Float(0x42426045), SkBits2Float(0x41ecf3b2), SkBits2Float(0x42426b89));  // 29.598f, 48.605f, 29.608f, 48.594f, 29.619f, 48.605f
path.cubicTo(SkBits2Float(0x41ecf3b2), SkBits2Float(0x42426b89), SkBits2Float(0x41eda5df), SkBits2Float(0x42426045), SkBits2Float(0x41edba5a), SkBits2Float(0x42423f81));  // 29.619f, 48.605f, 29.706f, 48.594f, 29.716f, 48.562f
path.cubicTo(SkBits2Float(0x41ee126a), SkBits2Float(0x4241e66a), SkBits2Float(0x41edd0e1), SkBits2Float(0x42403750), SkBits2Float(0x41eda5df), SkBits2Float(0x423fdf3f));  // 29.759f, 48.475f, 29.727f, 48.054f, 29.706f, 47.968f
path.lineTo(SkBits2Float(0x41ed4dcf), SkBits2Float(0x423fd3fb));  // 29.663f, 47.957f
path.moveTo(SkBits2Float(0x41d05a19), SkBits2Float(0x4258ab05));  // 26.044f, 54.167f
path.cubicTo(SkBits2Float(0x41d05a19), SkBits2Float(0x42589fc1), SkBits2Float(0x41d070a0), SkBits2Float(0x42588a40), SkBits2Float(0x41d05a19), SkBits2Float(0x42586876));  // 26.044f, 54.156f, 26.055f, 54.135f, 26.044f, 54.102f
path.cubicTo(SkBits2Float(0x41d05a19), SkBits2Float(0x42583c6e), SkBits2Float(0x41d02f17), SkBits2Float(0x4257ee9b), SkBits2Float(0x41d00209), SkBits2Float(0x4257c293));  // 26.044f, 54.059f, 26.023f, 53.983f, 26.001f, 53.94f
path.cubicTo(SkBits2Float(0x41cfeb82), SkBits2Float(0x42571cb0), SkBits2Float(0x41d00209), SkBits2Float(0x42568210), SkBits2Float(0x41cfeb82), SkBits2Float(0x4255c5a5));  // 25.99f, 53.778f, 26.001f, 53.627f, 25.99f, 53.443f
path.cubicTo(SkBits2Float(0x41cfeb82), SkBits2Float(0x4255a4e0), SkBits2Float(0x41cfc080), SkBits2Float(0x42552b05), SkBits2Float(0x41cfd4fb), SkBits2Float(0x4254dd32));  // 25.99f, 53.411f, 25.969f, 53.292f, 25.979f, 53.216f
path.cubicTo(SkBits2Float(0x41cfeb82), SkBits2Float(0x4254b12a), SkBits2Float(0x41d05a1a), SkBits2Float(0x4254b12a), SkBits2Float(0x41d0df38), SkBits2Float(0x4254c7b1));  // 25.99f, 53.173f, 26.044f, 53.173f, 26.109f, 53.195f
path.cubicTo(SkBits2Float(0x41d24186), SkBits2Float(0x42552b05), SkBits2Float(0x41d4ac05), SkBits2Float(0x42563f80), SkBits2Float(0x41d50621), SkBits2Float(0x42566044));  // 26.282f, 53.292f, 26.584f, 53.562f, 26.628f, 53.594f
path.cubicTo(SkBits2Float(0x41d60e52), SkBits2Float(0x4256da1f), SkBits2Float(0x41d70208), SkBits2Float(0x425748b7), SkBits2Float(0x41d80a3a), SkBits2Float(0x4257c292));  // 26.757f, 53.713f, 26.876f, 53.821f, 27.005f, 53.94f
path.cubicTo(SkBits2Float(0x41d8a5e0), SkBits2Float(0x4257f9de), SkBits2Float(0x41da1eb5), SkBits2Float(0x4258947e), SkBits2Float(0x41d8a5e0), SkBits2Float(0x4258ab05));  // 27.081f, 53.994f, 27.265f, 54.145f, 27.081f, 54.167f
path.cubicTo(SkBits2Float(0x41d7df38), SkBits2Float(0x4258cbca), SkBits2Float(0x41d72d0b), SkBits2Float(0x4258b649), SkBits2Float(0x41d66663), SkBits2Float(0x4258b649));  // 26.984f, 54.199f, 26.897f, 54.178f, 26.8f, 54.178f
path.cubicTo(SkBits2Float(0x41d547ab), SkBits2Float(0x4258cbca), SkBits2Float(0x41d1bc67), SkBits2Float(0x42592f1e), SkBits2Float(0x41d0b22a), SkBits2Float(0x4258e251));  // 26.66f, 54.199f, 26.217f, 54.296f, 26.087f, 54.221f
path.lineTo(SkBits2Float(0x41d0b22a), SkBits2Float(0x4258d70d));  // 26.087f, 54.21f
path.cubicTo(SkBits2Float(0x41d09daf), SkBits2Float(0x4258d70d), SkBits2Float(0x41d070a1), SkBits2Float(0x4258b648), SkBits2Float(0x41d05a1a), SkBits2Float(0x4258ab05));  // 26.077f, 54.21f, 26.055f, 54.178f, 26.044f, 54.167f
path.moveTo(SkBits2Float(0x41ce8b41), SkBits2Float(0x42588a40));  // 25.818f, 54.135f
path.cubicTo(SkBits2Float(0x41ceb643), SkBits2Float(0x4258ab05), SkBits2Float(0x41ce74ba), SkBits2Float(0x4258ab05), SkBits2Float(0x41ceccca), SkBits2Float(0x4258ab05));  // 25.839f, 54.167f, 25.807f, 54.167f, 25.85f, 54.167f
path.cubicTo(SkBits2Float(0x41cef7cc), SkBits2Float(0x4258ab05), SkBits2Float(0x41cf0e53), SkBits2Float(0x4258b336), SkBits2Float(0x41cf0e53), SkBits2Float(0x42589db5));  // 25.871f, 54.167f, 25.882f, 54.175f, 25.882f, 54.154f
path.cubicTo(SkBits2Float(0x41cf0e53), SkBits2Float(0x4258395b), SkBits2Float(0x41cf0a3a), SkBits2Float(0x42579790), SkBits2Float(0x41cedd2c), SkBits2Float(0x4257343c));  // 25.882f, 54.056f, 25.88f, 53.898f, 25.858f, 53.801f
path.cubicTo(SkBits2Float(0x41cec8b1), SkBits2Float(0x42564086), SkBits2Float(0x41ceccca), SkBits2Float(0x4254f3b9), SkBits2Float(0x41ce5e32), SkBits2Float(0x425421cd));  // 25.848f, 53.563f, 25.85f, 53.238f, 25.796f, 53.033f
path.lineTo(SkBits2Float(0x41cdef9a), SkBits2Float(0x425421cd));  // 25.742f, 53.033f
path.cubicTo(SkBits2Float(0x41cdd913), SkBits2Float(0x4254dd31), SkBits2Float(0x41ce126b), SkBits2Float(0x425626ec), SkBits2Float(0x41ce28f2), SkBits2Float(0x4256e250));  // 25.731f, 53.216f, 25.759f, 53.538f, 25.77f, 53.721f
path.cubicTo(SkBits2Float(0x41ce3f79), SkBits2Float(0x42579377), SkBits2Float(0x41ce47aa), SkBits2Float(0x42580f5e), SkBits2Float(0x41ce8b40), SkBits2Float(0x42588a40));  // 25.781f, 53.894f, 25.785f, 54.015f, 25.818f, 54.135f
path.moveTo(SkBits2Float(0x41c58d4c), SkBits2Float(0x425271ad));  // 24.694f, 52.611f
path.cubicTo(SkBits2Float(0x41c58d4c), SkBits2Float(0x42525c2c), SkBits2Float(0x41c5ba5a), SkBits2Float(0x42523024), SkBits2Float(0x41c5fbe4), SkBits2Float(0x425224e0));  // 24.694f, 52.59f, 24.716f, 52.547f, 24.748f, 52.536f
path.lineTo(SkBits2Float(0x41c6126b), SkBits2Float(0x4252199c));  // 24.759f, 52.525f
path.cubicTo(SkBits2Float(0x41c6978a), SkBits2Float(0x42520f5f), SkBits2Float(0x41c774b9), SkBits2Float(0x42523023), SkBits2Float(0x41c79fbb), SkBits2Float(0x42525c2b));  // 24.824f, 52.515f, 24.932f, 52.547f, 24.953f, 52.59f
path.cubicTo(SkBits2Float(0x41c7f9d8), SkBits2Float(0x4252a9fe), SkBits2Float(0x41c79fbb), SkBits2Float(0x4258e250), SkBits2Float(0x41c78b40), SkBits2Float(0x4259199c));  // 24.997f, 52.666f, 24.953f, 54.221f, 24.943f, 54.275f
path.lineTo(SkBits2Float(0x41c78b40), SkBits2Float(0x42592f1d));  // 24.943f, 54.296f
path.cubicTo(SkBits2Float(0x41c747aa), SkBits2Float(0x42595c2b), SkBits2Float(0x41c68103), SkBits2Float(0x42596669), SkBits2Float(0x41c5fbe4), SkBits2Float(0x42596669));  // 24.91f, 54.34f, 24.813f, 54.35f, 24.748f, 54.35f
path.cubicTo(SkBits2Float(0x41c5353c), SkBits2Float(0x425971ad), SkBits2Float(0x41c41684), SkBits2Float(0x425971ad), SkBits2Float(0x41c3e975), SkBits2Float(0x42592f1d));  // 24.651f, 54.361f, 24.511f, 54.361f, 24.489f, 54.296f
path.cubicTo(SkBits2Float(0x41c3a7ec), SkBits2Float(0x4258cbc9), SkBits2Float(0x41c42afe), SkBits2Float(0x4257d919), SkBits2Float(0x41c44185), SkBits2Float(0x42578002));  // 24.457f, 54.199f, 24.521f, 53.962f, 24.532f, 53.875f
path.cubicTo(SkBits2Float(0x41c46e93), SkBits2Float(0x42563f7f), SkBits2Float(0x41c4c6a4), SkBits2Float(0x42550a3f), SkBits2Float(0x41c5353b), SkBits2Float(0x4253df3d));  // 24.554f, 53.562f, 24.597f, 53.26f, 24.651f, 52.968f
path.cubicTo(SkBits2Float(0x41c54bc2), SkBits2Float(0x42537be9), SkBits2Float(0x41c56249), SkBits2Float(0x42530d51), SkBits2Float(0x41c58d4b), SkBits2Float(0x4252cac2));  // 24.662f, 52.871f, 24.673f, 52.763f, 24.694f, 52.698f
path.cubicTo(SkBits2Float(0x41c58d4b), SkBits2Float(0x4252a9fd), SkBits2Float(0x41c56249), SkBits2Float(0x42528833), SkBits2Float(0x41c58d4b), SkBits2Float(0x425271ab));  // 24.694f, 52.666f, 24.673f, 52.633f, 24.694f, 52.611f
path.moveTo(SkBits2Float(0x41c36662), SkBits2Float(0x42534fe0));  // 24.425f, 52.828f
path.cubicTo(SkBits2Float(0x41c33954), SkBits2Float(0x4253c9bb), SkBits2Float(0x41c34dcf), SkBits2Float(0x42541688), SkBits2Float(0x41c322cc), SkBits2Float(0x42549063));  // 24.403f, 52.947f, 24.413f, 53.022f, 24.392f, 53.141f
path.cubicTo(SkBits2Float(0x41c2f5be), SkBits2Float(0x4254fefb), SkBits2Float(0x41c2b434), SkBits2Float(0x42558e57), SkBits2Float(0x41c29dad), SkBits2Float(0x42560832));  // 24.37f, 53.249f, 24.338f, 53.389f, 24.327f, 53.508f
path.cubicTo(SkBits2Float(0x41c2709f), SkBits2Float(0x4256e561), SkBits2Float(0x41c2459d), SkBits2Float(0x4257ad0f), SkBits2Float(0x41c1ed8c), SkBits2Float(0x42586874));  // 24.305f, 53.724f, 24.284f, 53.919f, 24.241f, 54.102f
path.cubicTo(SkBits2Float(0x41c1d705), SkBits2Float(0x4258cbc8), SkBits2Float(0x41c20207), SkBits2Float(0x42590e57), SkBits2Float(0x41c1c07e), SkBits2Float(0x425950e7));  // 24.23f, 54.199f, 24.251f, 54.264f, 24.219f, 54.329f
path.cubicTo(SkBits2Float(0x41c1c07e), SkBits2Float(0x42596668), SkBits2Float(0x41c1686e), SkBits2Float(0x42599270), SkBits2Float(0x41c13b5f), SkBits2Float(0x42599270));  // 24.219f, 54.35f, 24.176f, 54.393f, 24.154f, 54.393f
path.cubicTo(SkBits2Float(0x41c0ccc7), SkBits2Float(0x4259a8f7), SkBits2Float(0x41c074b7), SkBits2Float(0x42599270), SkBits2Float(0x41c00620), SkBits2Float(0x425971ab));  // 24.1f, 54.415f, 24.057f, 54.393f, 24.003f, 54.361f
path.cubicTo(SkBits2Float(0x41c00620), SkBits2Float(0x425825e4), SkBits2Float(0x41c08b3f), SkBits2Float(0x4256da1e), SkBits2Float(0x41c0f9d6), SkBits2Float(0x42558e57));  // 24.003f, 54.037f, 24.068f, 53.713f, 24.122f, 53.389f
path.cubicTo(SkBits2Float(0x41c151e6), SkBits2Float(0x425479dc), SkBits2Float(0x41c151e6), SkBits2Float(0x42534fe0), SkBits2Float(0x41c1ed8c), SkBits2Float(0x425245a3));  // 24.165f, 53.119f, 24.165f, 52.828f, 24.241f, 52.568f
path.cubicTo(SkBits2Float(0x41c22f15), SkBits2Float(0x42520f5d), SkBits2Float(0x41c22f15), SkBits2Float(0x4251d70b), SkBits2Float(0x41c25c24), SkBits2Float(0x4251ccce));  // 24.273f, 52.515f, 24.273f, 52.46f, 24.295f, 52.45f
path.cubicTo(SkBits2Float(0x41c2e143), SkBits2Float(0x4251b647), SkBits2Float(0x41c34dce), SkBits2Float(0x4251e24f), SkBits2Float(0x41c3a7eb), SkBits2Float(0x4251e24f));  // 24.36f, 52.428f, 24.413f, 52.471f, 24.457f, 52.471f
path.cubicTo(SkBits2Float(0x41c3be72), SkBits2Float(0x42525c2a), SkBits2Float(0x41c37add), SkBits2Float(0x4252e149), SkBits2Float(0x41c36662), SkBits2Float(0x42534fe0));  // 24.468f, 52.59f, 24.435f, 52.72f, 24.425f, 52.828f
path.moveTo(SkBits2Float(0x41b3105e), SkBits2Float(0x426e020d));  // 22.383f, 59.502f
path.cubicTo(SkBits2Float(0x41b2ced5), SkBits2Float(0x426dcac1), SkBits2Float(0x41b28b3f), SkBits2Float(0x426d9375), SkBits2Float(0x41b21ca8), SkBits2Float(0x426d676d));  // 22.351f, 59.448f, 22.318f, 59.394f, 22.264f, 59.351f
path.lineTo(SkBits2Float(0x41b1f1a6), SkBits2Float(0x426d676d));  // 22.243f, 59.351f
path.lineTo(SkBits2Float(0x41b1f1a6), SkBits2Float(0x426d50e6));  // 22.243f, 59.329f
path.cubicTo(SkBits2Float(0x41b1f1a6), SkBits2Float(0x426d2f1b), SkBits2Float(0x41b1830e), SkBits2Float(0x426d199a), SkBits2Float(0x41b15600), SkBits2Float(0x426d0f5d));  // 22.243f, 59.296f, 22.189f, 59.275f, 22.167f, 59.265f
path.cubicTo(SkBits2Float(0x41b0e768), SkBits2Float(0x426cccce), SkBits2Float(0x41af1683), SkBits2Float(0x426bd917), SkBits2Float(0x41aefffc), SkBits2Float(0x426b8107));  // 22.113f, 59.2f, 21.886f, 58.962f, 21.875f, 58.876f
path.cubicTo(SkBits2Float(0x41aeeb81), SkBits2Float(0x426b3334), SkBits2Float(0x41af5a19), SkBits2Float(0x426acfe0), SkBits2Float(0x41af70a0), SkBits2Float(0x426a8d51));  // 21.865f, 58.8f, 21.919f, 58.703f, 21.93f, 58.638f
path.cubicTo(SkBits2Float(0x41b04dcf), SkBits2Float(0x42693647), SkBits2Float(0x41b1db1f), SkBits2Float(0x4268645b), SkBits2Float(0x41b43123), SkBits2Float(0x4267c9bc));  // 22.038f, 58.303f, 22.232f, 58.098f, 22.524f, 57.947f
path.cubicTo(SkBits2Float(0x41b472ac), SkBits2Float(0x4267a7f1), SkBits2Float(0x41b4f7cb), SkBits2Float(0x426770a5), SkBits2Float(0x41b56662), SkBits2Float(0x42676668));  // 22.556f, 57.914f, 22.621f, 57.86f, 22.675f, 57.85f
path.cubicTo(SkBits2Float(0x41b5a7eb), SkBits2Float(0x42675b24), SkBits2Float(0x41b5d4fa), SkBits2Float(0x42676668), SkBits2Float(0x41b62d0a), SkBits2Float(0x42675b24));  // 22.707f, 57.839f, 22.729f, 57.85f, 22.772f, 57.839f
path.cubicTo(SkBits2Float(0x41b69ba2), SkBits2Float(0x42674fe0), SkBits2Float(0x41b78f58), SkBits2Float(0x42671895), SkBits2Float(0x41b828f1), SkBits2Float(0x42671895));  // 22.826f, 57.828f, 22.945f, 57.774f, 23.02f, 57.774f
path.cubicTo(SkBits2Float(0x41b8ae10), SkBits2Float(0x42671895), SkBits2Float(0x41b8c497), SkBits2Float(0x42672f1c), SkBits2Float(0x41b91ca7), SkBits2Float(0x4267449d));  // 23.085f, 57.774f, 23.096f, 57.796f, 23.139f, 57.817f
path.lineTo(SkBits2Float(0x41b91ca7), SkBits2Float(0x42675b24));  // 23.139f, 57.839f
path.cubicTo(SkBits2Float(0x41b91ca7), SkBits2Float(0x42674fe0), SkBits2Float(0x41b9332e), SkBits2Float(0x426770a5), SkBits2Float(0x41b9332e), SkBits2Float(0x4267872c));  // 23.139f, 57.828f, 23.15f, 57.86f, 23.15f, 57.882f
path.cubicTo(SkBits2Float(0x41b91ca7), SkBits2Float(0x4267df3c), SkBits2Float(0x41b90620), SkBits2Float(0x42685918), SkBits2Float(0x41b8db1e), SkBits2Float(0x4268bc6b));  // 23.139f, 57.968f, 23.128f, 58.087f, 23.107f, 58.184f
path.cubicTo(SkBits2Float(0x41b855ff), SkBits2Float(0x426a29fc), SkBits2Float(0x41b7d0e1), SkBits2Float(0x426bc290), SkBits2Float(0x41b76249), SkBits2Float(0x426d2f1b));  // 23.042f, 58.541f, 22.977f, 58.94f, 22.923f, 59.296f
path.cubicTo(SkBits2Float(0x41b720c0), SkBits2Float(0x426e0d50), SkBits2Float(0x41b720c0), SkBits2Float(0x426ed3f8), SkBits2Float(0x41b69ba1), SkBits2Float(0x426f79dc));  // 22.891f, 59.513f, 22.891f, 59.707f, 22.826f, 59.869f
path.cubicTo(SkBits2Float(0x41b64391), SkBits2Float(0x426f645b), SkBits2Float(0x41b62d09), SkBits2Float(0x426f79dc), SkBits2Float(0x41b5eb80), SkBits2Float(0x426f645b));  // 22.783f, 59.848f, 22.772f, 59.869f, 22.74f, 59.848f
path.cubicTo(SkBits2Float(0x41b5a7ea), SkBits2Float(0x426f5917), SkBits2Float(0x41b57adc), SkBits2Float(0x426f374d), SkBits2Float(0x41b53953), SkBits2Float(0x426f1688));  // 22.707f, 59.837f, 22.685f, 59.804f, 22.653f, 59.772f
path.lineTo(SkBits2Float(0x41b53953), SkBits2Float(0x426f0107));  // 22.653f, 59.751f
path.cubicTo(SkBits2Float(0x41b472ab), SkBits2Float(0x426ea7f0), SkBits2Float(0x41b3ac03), SkBits2Float(0x426e5a1e), SkBits2Float(0x41b3105d), SkBits2Float(0x426e020d));  // 22.556f, 59.664f, 22.459f, 59.588f, 22.383f, 59.502f
    testSimplify(reporter, path, filename);
}

static void joel_5(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPathFillType) 0);
path.moveTo(SkBits2Float(0x43c5145a), SkBits2Float(0x43dc82f2));  // 394.159f, 441.023f
path.lineTo(SkBits2Float(0x43c5145a), SkBits2Float(0x43dc82f2));  // 394.159f, 441.023f
path.close();
path.moveTo(SkBits2Float(0x43af4e56), SkBits2Float(0x43dbc604));  // 350.612f, 439.547f
path.lineTo(SkBits2Float(0x43af4e56), SkBits2Float(0x43dbc604));  // 350.612f, 439.547f
path.close();
path.moveTo(SkBits2Float(0x43af4e56), SkBits2Float(0x43dbc604));  // 350.612f, 439.547f
path.cubicTo(SkBits2Float(0x43b64a5e), SkBits2Float(0x43dc9604), SkBits2Float(0x43be0958), SkBits2Float(0x43dbb604), SkBits2Float(0x43c5145a), SkBits2Float(0x43dc8312));  // 364.581f, 441.172f, 380.073f, 439.422f, 394.159f, 441.024f
path.cubicTo(SkBits2Float(0x43be0958), SkBits2Float(0x43dbb604), SkBits2Float(0x43b64a5e), SkBits2Float(0x43dc9604), SkBits2Float(0x43af4e56), SkBits2Float(0x43dbc604));  // 380.073f, 439.422f, 364.581f, 441.172f, 350.612f, 439.547f
path.close();
path.moveTo(SkBits2Float(0x43a9126f), SkBits2Float(0x43e11604));  // 338.144f, 450.172f
path.lineTo(SkBits2Float(0x43a9126f), SkBits2Float(0x43e11604));  // 338.144f, 450.172f
path.close();
path.moveTo(SkBits2Float(0x43a9126f), SkBits2Float(0x43e11604));  // 338.144f, 450.172f
path.cubicTo(SkBits2Float(0x43ab3c6b), SkBits2Float(0x43debc08), SkBits2Float(0x43ad1b65), SkBits2Float(0x43de18f6), SkBits2Float(0x43af4e77), SkBits2Float(0x43dbc604));  // 342.472f, 445.469f, 346.214f, 444.195f, 350.613f, 439.547f
path.cubicTo(SkBits2Float(0x43ad1b65), SkBits2Float(0x43de18f6), SkBits2Float(0x43ab3c6b), SkBits2Float(0x43debc08), SkBits2Float(0x43a9126f), SkBits2Float(0x43e11604));  // 346.214f, 444.195f, 342.472f, 445.469f, 338.144f, 450.172f
path.close();
path.moveTo(SkBits2Float(0x43aa9d50), SkBits2Float(0x43e173f8));  // 341.229f, 450.906f
path.lineTo(SkBits2Float(0x43aa9d50), SkBits2Float(0x43e173f8));  // 341.229f, 450.906f
path.close();
path.moveTo(SkBits2Float(0x43aa9d50), SkBits2Float(0x43e173f8));  // 341.229f, 450.906f
path.cubicTo(SkBits2Float(0x43aa0852), SkBits2Float(0x43e183f8), SkBits2Float(0x43a9be56), SkBits2Float(0x43e0d2f2), SkBits2Float(0x43a9124e), SkBits2Float(0x43e11604));  // 340.065f, 451.031f, 339.487f, 449.648f, 338.143f, 450.172f
path.cubicTo(SkBits2Float(0x43a9be56), SkBits2Float(0x43e0d2f2), SkBits2Float(0x43aa0852), SkBits2Float(0x43e183f8), SkBits2Float(0x43aa9d50), SkBits2Float(0x43e173f8));  // 339.487f, 449.648f, 340.065f, 451.031f, 341.229f, 450.906f
path.close();
path.moveTo(SkBits2Float(0x43b13667), SkBits2Float(0x43dce106));  // 354.425f, 441.758f
path.lineTo(SkBits2Float(0x43b13667), SkBits2Float(0x43dce106));  // 354.425f, 441.758f
path.close();
path.moveTo(SkBits2Float(0x43b13667), SkBits2Float(0x43dce106));  // 354.425f, 441.758f
path.cubicTo(SkBits2Float(0x43aead71), SkBits2Float(0x43dd9d0e), SkBits2Float(0x43acd375), SkBits2Float(0x43dff20c), SkBits2Float(0x43aa9d71), SkBits2Float(0x43e173f8));  // 349.355f, 443.227f, 345.652f, 447.891f, 341.23f, 450.906f
path.cubicTo(SkBits2Float(0x43acd354), SkBits2Float(0x43dff20c), SkBits2Float(0x43aead50), SkBits2Float(0x43dd9d0f), SkBits2Float(0x43b13667), SkBits2Float(0x43dce106));  // 345.651f, 447.891f, 349.354f, 443.227f, 354.425f, 441.758f
path.close();
path.moveTo(SkBits2Float(0x43ac8561), SkBits2Float(0x43e30106));  // 345.042f, 454.008f
path.lineTo(SkBits2Float(0x43ac8561), SkBits2Float(0x43e30106));  // 345.042f, 454.008f
path.close();
path.moveTo(SkBits2Float(0x43ac8561), SkBits2Float(0x43e30106));  // 345.042f, 454.008f
path.cubicTo(SkBits2Float(0x43adc76d), SkBits2Float(0x43e0f4fe), SkBits2Float(0x43b21a5f), SkBits2Float(0x43df7efa), SkBits2Float(0x43b13667), SkBits2Float(0x43dce106));  // 347.558f, 449.914f, 356.206f, 446.992f, 354.425f, 441.758f
path.cubicTo(SkBits2Float(0x43b21a5f), SkBits2Float(0x43df7efa), SkBits2Float(0x43adc76d), SkBits2Float(0x43e0f4fe), SkBits2Float(0x43ac8561), SkBits2Float(0x43e30106));  // 356.206f, 446.992f, 347.558f, 449.914f, 345.042f, 454.008f
path.close();
path.moveTo(SkBits2Float(0x43b33169), SkBits2Float(0x43dc82f2));  // 358.386f, 441.023f
path.lineTo(SkBits2Float(0x43b33169), SkBits2Float(0x43dc82f2));  // 358.386f, 441.023f
path.close();
path.moveTo(SkBits2Float(0x43b33169), SkBits2Float(0x43dc82f2));  // 358.386f, 441.023f
path.cubicTo(SkBits2Float(0x43b16169), SkBits2Float(0x43ded7f0), SkBits2Float(0x43aef375), SkBits2Float(0x43e13be8), SkBits2Float(0x43ac8561), SkBits2Float(0x43e300e6));  // 354.761f, 445.687f, 349.902f, 450.468f, 345.042f, 454.007f
path.cubicTo(SkBits2Float(0x43aef355), SkBits2Float(0x43e13c09), SkBits2Float(0x43b16169), SkBits2Float(0x43ded811), SkBits2Float(0x43b33169), SkBits2Float(0x43dc82f2));  // 349.901f, 450.469f, 354.761f, 445.688f, 358.386f, 441.023f
path.close();
path.moveTo(SkBits2Float(0x43b4bb65), SkBits2Float(0x43dd4000));  // 361.464f, 442.5f
path.lineTo(SkBits2Float(0x43b4bb65), SkBits2Float(0x43dd4000));  // 361.464f, 442.5f
path.close();
path.moveTo(SkBits2Float(0x43b4bb65), SkBits2Float(0x43dd4000));  // 361.464f, 442.5f
path.cubicTo(SkBits2Float(0x43b44959), SkBits2Float(0x43dcddf4), SkBits2Float(0x43b3e76d), SkBits2Float(0x43dc48f6), SkBits2Float(0x43b33169), SkBits2Float(0x43dc82f2));  // 360.573f, 441.734f, 359.808f, 440.57f, 358.386f, 441.023f
path.cubicTo(SkBits2Float(0x43b3e76d), SkBits2Float(0x43dc48f6), SkBits2Float(0x43b44959), SkBits2Float(0x43dcddf4), SkBits2Float(0x43b4bb65), SkBits2Float(0x43dd4000));  // 359.808f, 440.57f, 360.573f, 441.734f, 361.464f, 442.5f
path.close();
path.moveTo(SkBits2Float(0x43ae7f5d), SkBits2Float(0x43e5a70a));  // 348.995f, 459.305f
path.lineTo(SkBits2Float(0x43ae7f5d), SkBits2Float(0x43e5a70a));  // 348.995f, 459.305f
path.close();
path.moveTo(SkBits2Float(0x43ae7f5d), SkBits2Float(0x43e5a70a));  // 348.995f, 459.305f
path.cubicTo(SkBits2Float(0x43af945b), SkBits2Float(0x43e21d0e), SkBits2Float(0x43b3a74d), SkBits2Float(0x43e0ce14), SkBits2Float(0x43b4bb65), SkBits2Float(0x43dd4000));  // 351.159f, 452.227f, 359.307f, 449.61f, 361.464f, 442.5f
path.cubicTo(SkBits2Float(0x43b3a76d), SkBits2Float(0x43e0cdf4), SkBits2Float(0x43af945b), SkBits2Float(0x43e21d0e), SkBits2Float(0x43ae7f5d), SkBits2Float(0x43e5a70a));  // 359.308f, 449.609f, 351.159f, 452.227f, 348.995f, 459.305f
path.close();
path.moveTo(SkBits2Float(0x43b58a5f), SkBits2Float(0x43dce106));  // 363.081f, 441.758f
path.lineTo(SkBits2Float(0x43b58a5f), SkBits2Float(0x43dce106));  // 363.081f, 441.758f
path.close();
path.moveTo(SkBits2Float(0x43b58a5f), SkBits2Float(0x43dce106));  // 363.081f, 441.758f
path.cubicTo(SkBits2Float(0x43b2c063), SkBits2Float(0x43dfa604), SkBits2Float(0x43b1d561), SkBits2Float(0x43e374fe), SkBits2Float(0x43ae7f5d), SkBits2Float(0x43e5a70a));  // 357.503f, 447.297f, 355.667f, 454.914f, 348.995f, 459.305f
path.cubicTo(SkBits2Float(0x43b1d561), SkBits2Float(0x43e374fe), SkBits2Float(0x43b2c063), SkBits2Float(0x43dfa604), SkBits2Float(0x43b58a5f), SkBits2Float(0x43dce106));  // 355.667f, 454.914f, 357.503f, 447.297f, 363.081f, 441.758f
path.close();
path.moveTo(SkBits2Float(0x43b6b561), SkBits2Float(0x43dd4000));  // 365.417f, 442.5f
path.lineTo(SkBits2Float(0x43b6b561), SkBits2Float(0x43dd4000));  // 365.417f, 442.5f
path.close();
path.moveTo(SkBits2Float(0x43b6b561), SkBits2Float(0x43dd4000));  // 365.417f, 442.5f
path.lineTo(SkBits2Float(0x43b58a5f), SkBits2Float(0x43dce106));  // 363.081f, 441.758f
path.lineTo(SkBits2Float(0x43b6b561), SkBits2Float(0x43dd4000));  // 365.417f, 442.5f
path.close();
path.moveTo(SkBits2Float(0x43b07a5f), SkBits2Float(0x43e7220c));  // 352.956f, 462.266f
path.lineTo(SkBits2Float(0x43b07a5f), SkBits2Float(0x43e7220c));  // 352.956f, 462.266f
path.close();
path.moveTo(SkBits2Float(0x43b07a5f), SkBits2Float(0x43e7220c));  // 352.956f, 462.266f
path.cubicTo(SkBits2Float(0x43b29f5d), SkBits2Float(0x43e3e810), SkBits2Float(0x43b59667), SkBits2Float(0x43e0f916), SkBits2Float(0x43b6b561), SkBits2Float(0x43dd4000));  // 357.245f, 455.813f, 363.175f, 449.946f, 365.417f, 442.5f
path.cubicTo(SkBits2Float(0x43b59667), SkBits2Float(0x43e0f8f6), SkBits2Float(0x43b29f5d), SkBits2Float(0x43e3e7f0), SkBits2Float(0x43b07a5f), SkBits2Float(0x43e7220c));  // 363.175f, 449.945f, 357.245f, 455.812f, 352.956f, 462.266f
path.close();
path.moveTo(SkBits2Float(0x43b0d853), SkBits2Float(0x43e84efa));  // 353.69f, 464.617f
path.lineTo(SkBits2Float(0x43b0d853), SkBits2Float(0x43e84efa));  // 353.69f, 464.617f
path.close();
path.moveTo(SkBits2Float(0x43b0d853), SkBits2Float(0x43e84efa));  // 353.69f, 464.617f
path.cubicTo(SkBits2Float(0x43b03a5f), SkBits2Float(0x43e934fe), SkBits2Float(0x43b1345b), SkBits2Float(0x43e7870a), SkBits2Float(0x43b07a5f), SkBits2Float(0x43e721ec));  // 352.456f, 466.414f, 354.409f, 463.055f, 352.956f, 462.265f
path.cubicTo(SkBits2Float(0x43b1345b), SkBits2Float(0x43e7870b), SkBits2Float(0x43b03a5f), SkBits2Float(0x43e934fe), SkBits2Float(0x43b0d853), SkBits2Float(0x43e84efa));  // 354.409f, 463.055f, 352.456f, 466.414f, 353.69f, 464.617f
path.close();
path.moveTo(SkBits2Float(0x43b84063), SkBits2Float(0x43ddb106));  // 368.503f, 443.383f
path.lineTo(SkBits2Float(0x43b84063), SkBits2Float(0x43ddb106));  // 368.503f, 443.383f
path.close();
path.moveTo(SkBits2Float(0x43b84063), SkBits2Float(0x43ddb106));  // 368.503f, 443.383f
path.cubicTo(SkBits2Float(0x43b42667), SkBits2Float(0x43e039fc), SkBits2Float(0x43b39d71), SkBits2Float(0x43e4e000), SkBits2Float(0x43b0d873), SkBits2Float(0x43e84efa));  // 360.3f, 448.453f, 359.23f, 457.75f, 353.691f, 464.617f
path.cubicTo(SkBits2Float(0x43b39d50), SkBits2Float(0x43e4e000), SkBits2Float(0x43b42667), SkBits2Float(0x43e039fc), SkBits2Float(0x43b84063), SkBits2Float(0x43ddb106));  // 359.229f, 457.75f, 360.3f, 448.453f, 368.503f, 443.383f
path.close();
path.moveTo(SkBits2Float(0x43b89d51), SkBits2Float(0x43de0efa));  // 369.229f, 444.117f
path.lineTo(SkBits2Float(0x43b89d51), SkBits2Float(0x43de0efa));  // 369.229f, 444.117f
path.close();
path.moveTo(SkBits2Float(0x43b89d51), SkBits2Float(0x43de0efa));  // 369.229f, 444.117f
path.lineTo(SkBits2Float(0x43b84043), SkBits2Float(0x43ddb106));  // 368.502f, 443.383f
path.lineTo(SkBits2Float(0x43b89d51), SkBits2Float(0x43de0efa));  // 369.229f, 444.117f
path.close();
path.moveTo(SkBits2Float(0x43b26270), SkBits2Float(0x43e90c08));  // 356.769f, 466.094f
path.lineTo(SkBits2Float(0x43b26270), SkBits2Float(0x43e90c08));  // 356.769f, 466.094f
path.close();
path.moveTo(SkBits2Float(0x43b26270), SkBits2Float(0x43e90c08));  // 356.769f, 466.094f
path.cubicTo(SkBits2Float(0x43b48d72), SkBits2Float(0x43e569fc), SkBits2Float(0x43b7897a), SkBits2Float(0x43e21d0e), SkBits2Float(0x43b89d72), SkBits2Float(0x43de0efa));  // 361.105f, 458.828f, 367.074f, 452.227f, 369.23f, 444.117f
path.cubicTo(SkBits2Float(0x43b78959), SkBits2Float(0x43e21d0e), SkBits2Float(0x43b48d51), SkBits2Float(0x43e569fc), SkBits2Float(0x43b26270), SkBits2Float(0x43e90c08));  // 367.073f, 452.227f, 361.104f, 458.828f, 356.769f, 466.094f
path.close();
path.moveTo(SkBits2Float(0x43b3316a), SkBits2Float(0x43e90c08));  // 358.386f, 466.094f
path.lineTo(SkBits2Float(0x43b3316a), SkBits2Float(0x43e90c08));  // 358.386f, 466.094f
path.close();
path.moveTo(SkBits2Float(0x43b3316a), SkBits2Float(0x43e90c08));  // 358.386f, 466.094f
path.lineTo(SkBits2Float(0x43b26270), SkBits2Float(0x43e90c08));  // 356.769f, 466.094f
path.lineTo(SkBits2Float(0x43b3316a), SkBits2Float(0x43e90c08));  // 358.386f, 466.094f
path.close();
path.moveTo(SkBits2Float(0x43ba2853), SkBits2Float(0x43ddb106));  // 372.315f, 443.383f
path.lineTo(SkBits2Float(0x43ba2853), SkBits2Float(0x43ddb106));  // 372.315f, 443.383f
path.close();
path.moveTo(SkBits2Float(0x43ba2853), SkBits2Float(0x43ddb106));  // 372.315f, 443.383f
path.cubicTo(SkBits2Float(0x43b7d74d), SkBits2Float(0x43e17604), SkBits2Float(0x43b5824f), SkBits2Float(0x43e59604), SkBits2Float(0x43b33149), SkBits2Float(0x43e90c08));  // 367.682f, 450.922f, 363.018f, 459.172f, 358.385f, 466.094f
path.cubicTo(SkBits2Float(0x43b58270), SkBits2Float(0x43e59604), SkBits2Float(0x43b7d76e), SkBits2Float(0x43e17604), SkBits2Float(0x43ba2853), SkBits2Float(0x43ddb106));  // 363.019f, 459.172f, 367.683f, 450.922f, 372.315f, 443.383f
path.close();
path.moveTo(SkBits2Float(0x43bb5355), SkBits2Float(0x43de0efa));  // 374.651f, 444.117f
path.lineTo(SkBits2Float(0x43bb5355), SkBits2Float(0x43de0efa));  // 374.651f, 444.117f
path.close();
path.moveTo(SkBits2Float(0x43bb5355), SkBits2Float(0x43de0efa));  // 374.651f, 444.117f
path.cubicTo(SkBits2Float(0x43bb1853), SkBits2Float(0x43dd92f2), SkBits2Float(0x43ba9e57), SkBits2Float(0x43ddab02), SkBits2Float(0x43ba2853), SkBits2Float(0x43ddb106));  // 374.19f, 443.148f, 373.237f, 443.336f, 372.315f, 443.383f
path.cubicTo(SkBits2Float(0x43ba9e57), SkBits2Float(0x43ddab02), SkBits2Float(0x43bb1853), SkBits2Float(0x43dd92f2), SkBits2Float(0x43bb5355), SkBits2Float(0x43de0efa));  // 373.237f, 443.336f, 374.19f, 443.148f, 374.651f, 444.117f
path.close();
path.moveTo(SkBits2Float(0x43b58a5f), SkBits2Float(0x43e90c08));  // 363.081f, 466.094f
path.lineTo(SkBits2Float(0x43b58a5f), SkBits2Float(0x43e90c08));  // 363.081f, 466.094f
path.close();
path.moveTo(SkBits2Float(0x43b58a5f), SkBits2Float(0x43e90c08));  // 363.081f, 466.094f
path.cubicTo(SkBits2Float(0x43b76c6b), SkBits2Float(0x43e55d0e), SkBits2Float(0x43ba4a5f), SkBits2Float(0x43e21312), SkBits2Float(0x43bb5355), SkBits2Float(0x43de0efa));  // 366.847f, 458.727f, 372.581f, 452.149f, 374.651f, 444.117f
path.cubicTo(SkBits2Float(0x43ba4a5f), SkBits2Float(0x43e212f2), SkBits2Float(0x43b76c6c), SkBits2Float(0x43e55d0e), SkBits2Float(0x43b58a5f), SkBits2Float(0x43e90c08));  // 372.581f, 452.148f, 366.847f, 458.727f, 363.081f, 466.094f
path.close();
path.moveTo(SkBits2Float(0x43b6b561), SkBits2Float(0x43e90c08));  // 365.417f, 466.094f
path.lineTo(SkBits2Float(0x43b6b561), SkBits2Float(0x43e90c08));  // 365.417f, 466.094f
path.close();
path.moveTo(SkBits2Float(0x43b6b561), SkBits2Float(0x43e90c08));  // 365.417f, 466.094f
path.lineTo(SkBits2Float(0x43b58a5f), SkBits2Float(0x43e90c08));  // 363.081f, 466.094f
path.lineTo(SkBits2Float(0x43b6b561), SkBits2Float(0x43e90c08));  // 365.417f, 466.094f
path.close();
path.moveTo(SkBits2Float(0x43bc8063), SkBits2Float(0x43e058f6));  // 377.003f, 448.695f
path.lineTo(SkBits2Float(0x43bc8063), SkBits2Float(0x43e058f6));  // 377.003f, 448.695f
path.close();
path.moveTo(SkBits2Float(0x43bc8063), SkBits2Float(0x43e058f6));  // 377.003f, 448.695f
path.cubicTo(SkBits2Float(0x43b9de57), SkBits2Float(0x43e29df4), SkBits2Float(0x43b84355), SkBits2Float(0x43e5fefa), SkBits2Float(0x43b6b561), SkBits2Float(0x43e90be8));  // 371.737f, 453.234f, 368.526f, 459.992f, 365.417f, 466.093f
path.cubicTo(SkBits2Float(0x43b84355), SkBits2Float(0x43e5fefa), SkBits2Float(0x43b9de57), SkBits2Float(0x43e29df4), SkBits2Float(0x43bc8063), SkBits2Float(0x43e058f6));  // 368.526f, 459.992f, 371.737f, 453.234f, 377.003f, 448.695f
path.close();
path.moveTo(SkBits2Float(0x43b89d51), SkBits2Float(0x43e969fc));  // 369.229f, 466.828f
path.lineTo(SkBits2Float(0x43b89d51), SkBits2Float(0x43e969fc));  // 369.229f, 466.828f
path.close();
path.moveTo(SkBits2Float(0x43b89d51), SkBits2Float(0x43e969fc));  // 369.229f, 466.828f
path.cubicTo(SkBits2Float(0x43b98149), SkBits2Float(0x43e637f0), SkBits2Float(0x43bd3355), SkBits2Float(0x43e3adf4), SkBits2Float(0x43bc8043), SkBits2Float(0x43e058f6));  // 371.01f, 460.437f, 378.401f, 455.359f, 377.002f, 448.695f
path.cubicTo(SkBits2Float(0x43bd3355), SkBits2Float(0x43e3adf4), SkBits2Float(0x43b9816a), SkBits2Float(0x43e638f6), SkBits2Float(0x43b89d51), SkBits2Float(0x43e969fc));  // 378.401f, 455.359f, 371.011f, 460.445f, 369.229f, 466.828f
path.close();
path.moveTo(SkBits2Float(0x43ba8668), SkBits2Float(0x43e9c7f0));  // 373.05f, 467.562f
path.lineTo(SkBits2Float(0x43ba8668), SkBits2Float(0x43e9c7f0));  // 373.05f, 467.562f
path.close();
path.moveTo(SkBits2Float(0x43ba8668), SkBits2Float(0x43e9c7f0));  // 373.05f, 467.562f
path.cubicTo(SkBits2Float(0x43ba1376), SkBits2Float(0x43e90000), SkBits2Float(0x43b94270), SkBits2Float(0x43e8f1ec), SkBits2Float(0x43b89d72), SkBits2Float(0x43e969fc));  // 372.152f, 466, 370.519f, 465.89f, 369.23f, 466.828f
path.cubicTo(SkBits2Float(0x43b94270), SkBits2Float(0x43e8f20c), SkBits2Float(0x43ba1355), SkBits2Float(0x43e90000), SkBits2Float(0x43ba8668), SkBits2Float(0x43e9c7f0));  // 370.519f, 465.891f, 372.151f, 466, 373.05f, 467.562f
path.close();
path.moveTo(SkBits2Float(0x43c0c064), SkBits2Float(0x43dc82f2));  // 385.503f, 441.023f
path.lineTo(SkBits2Float(0x43c0c064), SkBits2Float(0x43dc82f2));  // 385.503f, 441.023f
path.close();
path.moveTo(SkBits2Float(0x43c0c064), SkBits2Float(0x43dc82f2));  // 385.503f, 441.023f
path.cubicTo(SkBits2Float(0x43be095a), SkBits2Float(0x43e0acee), SkBits2Float(0x43bd8a60), SkBits2Float(0x43e5c0e6), SkBits2Float(0x43ba8668), SkBits2Float(0x43e9c7f0));  // 380.073f, 449.351f, 379.081f, 459.507f, 373.05f, 467.562f
path.cubicTo(SkBits2Float(0x43bd8a60), SkBits2Float(0x43e5c107), SkBits2Float(0x43be095a), SkBits2Float(0x43e0ad0f), SkBits2Float(0x43c0c064), SkBits2Float(0x43dc82f2));  // 379.081f, 459.508f, 380.073f, 449.352f, 385.503f, 441.023f
path.close();
path.moveTo(SkBits2Float(0x43c00562), SkBits2Float(0x43e23000));  // 384.042f, 452.375f
path.lineTo(SkBits2Float(0x43c00562), SkBits2Float(0x43e23000));  // 384.042f, 452.375f
path.close();
path.moveTo(SkBits2Float(0x43c00562), SkBits2Float(0x43e23000));  // 384.042f, 452.375f
path.cubicTo(SkBits2Float(0x43bfaf5e), SkBits2Float(0x43e013f8), SkBits2Float(0x43c40668), SkBits2Float(0x43ddc2f2), SkBits2Float(0x43c0c064), SkBits2Float(0x43dc82f2));  // 383.37f, 448.156f, 392.05f, 443.523f, 385.503f, 441.023f
path.cubicTo(SkBits2Float(0x43c40668), SkBits2Float(0x43ddc2f2), SkBits2Float(0x43bfaf5e), SkBits2Float(0x43e013f8), SkBits2Float(0x43c00562), SkBits2Float(0x43e23000));  // 392.05f, 443.523f, 383.37f, 448.156f, 384.042f, 452.375f
path.close();
path.moveTo(SkBits2Float(0x43bed854), SkBits2Float(0x43e5370a));  // 381.69f, 458.43f
path.lineTo(SkBits2Float(0x43bed854), SkBits2Float(0x43e5370a));  // 381.69f, 458.43f
path.close();
path.moveTo(SkBits2Float(0x43bed854), SkBits2Float(0x43e5370a));  // 381.69f, 458.43f
path.cubicTo(SkBits2Float(0x43c06562), SkBits2Float(0x43e4b4fe), SkBits2Float(0x43bf095a), SkBits2Float(0x43e2fd0e), SkBits2Float(0x43c00562), SkBits2Float(0x43e23000));  // 384.792f, 457.414f, 382.073f, 453.977f, 384.042f, 452.375f
path.cubicTo(SkBits2Float(0x43bf095a), SkBits2Float(0x43e2fdf4), SkBits2Float(0x43c06562), SkBits2Float(0x43e4b4fe), SkBits2Float(0x43bed854), SkBits2Float(0x43e5370a));  // 382.073f, 453.984f, 384.792f, 457.414f, 381.69f, 458.43f
path.close();
path.moveTo(SkBits2Float(0x43bf3668), SkBits2Float(0x43e5a70a));  // 382.425f, 459.305f
path.lineTo(SkBits2Float(0x43bf3668), SkBits2Float(0x43e5a70a));  // 382.425f, 459.305f
path.close();
path.moveTo(SkBits2Float(0x43bf3668), SkBits2Float(0x43e5a70a));  // 382.425f, 459.305f
path.lineTo(SkBits2Float(0x43bed874), SkBits2Float(0x43e5370a));  // 381.691f, 458.43f
path.lineTo(SkBits2Float(0x43bf3668), SkBits2Float(0x43e5a70a));  // 382.425f, 459.305f
path.close();
path.moveTo(SkBits2Float(0x43bcde58), SkBits2Float(0x43e9c7ef));  // 377.737f, 467.562f
path.lineTo(SkBits2Float(0x43bcde58), SkBits2Float(0x43e9c7ef));  // 377.737f, 467.562f
path.close();
path.moveTo(SkBits2Float(0x43bcde58), SkBits2Float(0x43e9c7ef));  // 377.737f, 467.562f
path.cubicTo(SkBits2Float(0x43bdfb66), SkBits2Float(0x43e888f5), SkBits2Float(0x43bd6854), SkBits2Float(0x43e69ced), SkBits2Float(0x43bf3668), SkBits2Float(0x43e5a6e9));  // 379.964f, 465.07f, 378.815f, 461.226f, 382.425f, 459.304f
path.cubicTo(SkBits2Float(0x43bd6854), SkBits2Float(0x43e69d0e), SkBits2Float(0x43bdfb66), SkBits2Float(0x43e888f5), SkBits2Float(0x43bcde58), SkBits2Float(0x43e9c7ef));  // 378.815f, 461.227f, 379.964f, 465.07f, 377.737f, 467.562f
path.close();
path.moveTo(SkBits2Float(0x43bf3668), SkBits2Float(0x43ea9810));  // 382.425f, 469.188f
path.lineTo(SkBits2Float(0x43bf3668), SkBits2Float(0x43ea9810));  // 382.425f, 469.188f
path.close();
path.moveTo(SkBits2Float(0x43bf3668), SkBits2Float(0x43ea9810));  // 382.425f, 469.188f
path.cubicTo(SkBits2Float(0x43bebf5e), SkBits2Float(0x43e99e14), SkBits2Float(0x43bdc562), SkBits2Float(0x43e9d70a), SkBits2Float(0x43bcde58), SkBits2Float(0x43e9c810));  // 381.495f, 467.235f, 379.542f, 467.68f, 377.737f, 467.563f
path.cubicTo(SkBits2Float(0x43bdc562), SkBits2Float(0x43e9d70a), SkBits2Float(0x43bebf5e), SkBits2Float(0x43e99df3), SkBits2Float(0x43bf3668), SkBits2Float(0x43ea9810));  // 379.542f, 467.68f, 381.495f, 467.234f, 382.425f, 469.188f
path.close();
path.moveTo(SkBits2Float(0x43c0c064), SkBits2Float(0x43e78000));  // 385.503f, 463
path.lineTo(SkBits2Float(0x43c0c064), SkBits2Float(0x43e78000));  // 385.503f, 463
path.close();
path.moveTo(SkBits2Float(0x43c0c064), SkBits2Float(0x43e78000));  // 385.503f, 463
path.cubicTo(SkBits2Float(0x43bfaf5e), SkBits2Float(0x43e7f9fc), SkBits2Float(0x43bfbe58), SkBits2Float(0x43e98b02), SkBits2Float(0x43bf3668), SkBits2Float(0x43ea9810));  // 383.37f, 463.953f, 383.487f, 467.086f, 382.425f, 469.188f
path.cubicTo(SkBits2Float(0x43bfbe58), SkBits2Float(0x43e98b02), SkBits2Float(0x43bfaf5e), SkBits2Float(0x43e7f9fc), SkBits2Float(0x43c0c064), SkBits2Float(0x43e78000));  // 383.487f, 467.086f, 383.37f, 463.953f, 385.503f, 463
path.close();
path.moveTo(SkBits2Float(0x43c1316a), SkBits2Float(0x43e35efa));  // 386.386f, 454.742f
path.lineTo(SkBits2Float(0x43c1316a), SkBits2Float(0x43e35efa));  // 386.386f, 454.742f
path.close();
path.moveTo(SkBits2Float(0x43c1316a), SkBits2Float(0x43e35efa));  // 386.386f, 454.742f
path.cubicTo(SkBits2Float(0x43c35270), SkBits2Float(0x43e586ea), SkBits2Float(0x43beb064), SkBits2Float(0x43e561ec), SkBits2Float(0x43c0c064), SkBits2Float(0x43e78000));  // 390.644f, 459.054f, 381.378f, 458.765f, 385.503f, 463
path.cubicTo(SkBits2Float(0x43beb064), SkBits2Float(0x43e5620c), SkBits2Float(0x43c35270), SkBits2Float(0x43e5870a), SkBits2Float(0x43c1316a), SkBits2Float(0x43e35efa));  // 381.378f, 458.766f, 390.644f, 459.055f, 386.386f, 454.742f
path.close();
path.moveTo(SkBits2Float(0x43c3e76e), SkBits2Float(0x43df2b02));  // 391.808f, 446.336f
path.lineTo(SkBits2Float(0x43c3e76e), SkBits2Float(0x43df2b02));  // 391.808f, 446.336f
path.close();
path.moveTo(SkBits2Float(0x43c3e76e), SkBits2Float(0x43df2b02));  // 391.808f, 446.336f
path.cubicTo(SkBits2Float(0x43c2ba60), SkBits2Float(0x43e07810), SkBits2Float(0x43c32a60), SkBits2Float(0x43e31106), SkBits2Float(0x43c1316a), SkBits2Float(0x43e35efa));  // 389.456f, 448.938f, 390.331f, 454.133f, 386.386f, 454.742f
path.cubicTo(SkBits2Float(0x43c32a60), SkBits2Float(0x43e31106), SkBits2Float(0x43c2ba60), SkBits2Float(0x43e07811), SkBits2Float(0x43c3e76e), SkBits2Float(0x43df2b02));  // 390.331f, 454.133f, 389.456f, 448.938f, 391.808f, 446.336f
path.close();
path.moveTo(SkBits2Float(0x43c3e76e), SkBits2Float(0x43dd4000));  // 391.808f, 442.5f
path.lineTo(SkBits2Float(0x43c3e76e), SkBits2Float(0x43dd4000));  // 391.808f, 442.5f
path.close();
path.moveTo(SkBits2Float(0x43c3e76e), SkBits2Float(0x43dd4000));  // 391.808f, 442.5f
path.cubicTo(SkBits2Float(0x43c2a668), SkBits2Float(0x43ddbefa), SkBits2Float(0x43c35f7e), SkBits2Float(0x43def4fe), SkBits2Float(0x43c3e76e), SkBits2Float(0x43df2b02));  // 389.3f, 443.492f, 390.746f, 445.914f, 391.808f, 446.336f
path.cubicTo(SkBits2Float(0x43c35f5e), SkBits2Float(0x43def4fe), SkBits2Float(0x43c2a668), SkBits2Float(0x43ddbefa), SkBits2Float(0x43c3e76e), SkBits2Float(0x43dd4000));  // 390.745f, 445.914f, 389.3f, 443.492f, 391.808f, 442.5f
path.close();
path.moveTo(SkBits2Float(0x43c44562), SkBits2Float(0x43ddb106));  // 392.542f, 443.383f
path.lineTo(SkBits2Float(0x43c44562), SkBits2Float(0x43ddb106));  // 392.542f, 443.383f
path.close();
path.moveTo(SkBits2Float(0x43c44562), SkBits2Float(0x43ddb106));  // 392.542f, 443.383f
path.lineTo(SkBits2Float(0x43c3e76e), SkBits2Float(0x43dd4000));  // 391.808f, 442.5f
path.lineTo(SkBits2Float(0x43c44562), SkBits2Float(0x43ddb106));  // 392.542f, 443.383f
path.close();
path.moveTo(SkBits2Float(0x43c5145c), SkBits2Float(0x43dc82f2));  // 394.159f, 441.023f
path.lineTo(SkBits2Float(0x43c44562), SkBits2Float(0x43ddb0e6));  // 392.542f, 443.382f
path.lineTo(SkBits2Float(0x43c5145c), SkBits2Float(0x43dc82f2));  // 394.159f, 441.023f
path.close();
    testSimplify(reporter, path, filename);
}

static void joel_6(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
path.moveTo(SkBits2Float(0x43c38c6a), SkBits2Float(0x43a739fc));  // 391.097f, 334.453f
path.lineTo(SkBits2Float(0x43c36168), SkBits2Float(0x43a74efa));  // 390.761f, 334.617f
path.lineTo(SkBits2Float(0x43c33666), SkBits2Float(0x43a6f7f0));  // 390.425f, 333.937f
path.lineTo(SkBits2Float(0x43c36168), SkBits2Float(0x43a6e1ec));  // 390.761f, 333.765f
path.lineTo(SkBits2Float(0x43c38c6a), SkBits2Float(0x43a739fc));  // 391.097f, 334.453f
path.close();
path.moveTo(SkBits2Float(0x43c39062), SkBits2Float(0x43a73810));  // 391.128f, 334.438f
path.lineTo(SkBits2Float(0x43c3676c), SkBits2Float(0x43a75106));  // 390.808f, 334.633f
path.lineTo(SkBits2Float(0x43c33374), SkBits2Float(0x43a6fefa));  // 390.402f, 333.992f
path.lineTo(SkBits2Float(0x43c35d70), SkBits2Float(0x43a6e3f8));  // 390.73f, 333.781f
path.lineTo(SkBits2Float(0x43c39062), SkBits2Float(0x43a73811));  // 391.128f, 334.438f
path.lineTo(SkBits2Float(0x43c39062), SkBits2Float(0x43a73810));  // 391.128f, 334.438f
path.close();
path.moveTo(SkBits2Float(0x43e38958), SkBits2Float(0x43971c08));  // 455.073f, 302.219f
path.lineTo(SkBits2Float(0x43e3824e), SkBits2Float(0x43973000));  // 455.018f, 302.375f
path.lineTo(SkBits2Float(0x43e36f5c), SkBits2Float(0x439739fc));  // 454.87f, 302.453f
path.lineTo(SkBits2Float(0x43e35a5e), SkBits2Float(0x43970df4));  // 454.706f, 302.109f
path.lineTo(SkBits2Float(0x43e38958), SkBits2Float(0x43971c08));  // 455.073f, 302.219f
path.close();
path.moveTo(SkBits2Float(0x43e36f5c), SkBits2Float(0x439739fc));  // 454.87f, 302.453f
path.lineTo(SkBits2Float(0x43c38c6a), SkBits2Float(0x43a739fc));  // 391.097f, 334.453f
path.lineTo(SkBits2Float(0x43c36168), SkBits2Float(0x43a6e1ec));  // 390.761f, 333.765f
path.lineTo(SkBits2Float(0x43e3445a), SkBits2Float(0x4396e1ec));  // 454.534f, 301.765f
path.lineTo(SkBits2Float(0x43e36f5c), SkBits2Float(0x439739fc));  // 454.87f, 302.453f
path.close();
path.moveTo(SkBits2Float(0x43e41f5c), SkBits2Float(0x43946efa));  // 456.245f, 296.867f
path.lineTo(SkBits2Float(0x43e4545a), SkBits2Float(0x439479fc));  // 456.659f, 296.953f
path.lineTo(SkBits2Float(0x43e44354), SkBits2Float(0x4394acee));  // 456.526f, 297.351f
path.lineTo(SkBits2Float(0x43e41646), SkBits2Float(0x43949efa));  // 456.174f, 297.242f
path.lineTo(SkBits2Float(0x43e41f5d), SkBits2Float(0x43946efa));  // 456.245f, 296.867f
path.lineTo(SkBits2Float(0x43e41f5c), SkBits2Float(0x43946efa));  // 456.245f, 296.867f
path.close();
path.moveTo(SkBits2Float(0x43e44354), SkBits2Float(0x4394ad0e));  // 456.526f, 297.352f
path.lineTo(SkBits2Float(0x43e38958), SkBits2Float(0x43971c08));  // 455.073f, 302.219f
path.lineTo(SkBits2Float(0x43e32b64), SkBits2Float(0x43970000));  // 454.339f, 302
path.lineTo(SkBits2Float(0x43e3e76c), SkBits2Float(0x43949106));  // 455.808f, 297.133f
path.lineTo(SkBits2Float(0x43e44353), SkBits2Float(0x4394ad0e));  // 456.526f, 297.352f
path.lineTo(SkBits2Float(0x43e44354), SkBits2Float(0x4394ad0e));  // 456.526f, 297.352f
path.close();
path.moveTo(SkBits2Float(0x43e17d50), SkBits2Float(0x4393f20c));  // 450.979f, 295.891f
path.lineTo(SkBits2Float(0x43e18e56), SkBits2Float(0x4393e810));  // 451.112f, 295.813f
path.lineTo(SkBits2Float(0x43e1a148), SkBits2Float(0x4393eb02));  // 451.26f, 295.836f
path.lineTo(SkBits2Float(0x43e19852), SkBits2Float(0x43941b02));  // 451.19f, 296.211f
path.lineTo(SkBits2Float(0x43e17d50), SkBits2Float(0x4393f20c));  // 450.979f, 295.891f
path.close();
path.moveTo(SkBits2Float(0x43e1a169), SkBits2Float(0x4393eb02));  // 451.261f, 295.836f
path.lineTo(SkBits2Float(0x43e41f5d), SkBits2Float(0x43946efa));  // 456.245f, 296.867f
path.lineTo(SkBits2Float(0x43e40b65), SkBits2Float(0x4394cefa));  // 456.089f, 297.617f
path.lineTo(SkBits2Float(0x43e18d71), SkBits2Float(0x43944b02));  // 451.105f, 296.586f
path.lineTo(SkBits2Float(0x43e1a169), SkBits2Float(0x4393eb02));  // 451.261f, 295.836f
path.close();
path.moveTo(SkBits2Float(0x43c35d50), SkBits2Float(0x43a6e3f8));  // 390.729f, 333.781f
path.lineTo(SkBits2Float(0x43e17d50), SkBits2Float(0x4393f1ec));  // 450.979f, 295.89f
path.lineTo(SkBits2Float(0x43e1b148), SkBits2Float(0x439443f8));  // 451.385f, 296.531f
path.lineTo(SkBits2Float(0x43c39042), SkBits2Float(0x43a737f0));  // 391.127f, 334.437f
path.lineTo(SkBits2Float(0x43c35d50), SkBits2Float(0x43a6e3f8));  // 390.729f, 333.781f
path.close();
testSimplify(reporter, path, filename);
}

static void joel_7(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
path.moveTo(SkBits2Float(0x4321220c), SkBits2Float(0x43eac70a));  // 161.133f, 469.555f
path.lineTo(SkBits2Float(0x4321220c), SkBits2Float(0x43eac70a));  // 161.133f, 469.555f
path.lineTo(SkBits2Float(0x431f8e14), SkBits2Float(0x43eb3b02));  // 159.555f, 470.461f
path.lineTo(SkBits2Float(0x4321220c), SkBits2Float(0x43eac70a));  // 161.133f, 469.555f
path.close();
path.moveTo(SkBits2Float(0x431e33f8), SkBits2Float(0x43f03b02));  // 158.203f, 480.461f
path.lineTo(SkBits2Float(0x431e33f8), SkBits2Float(0x43f03b02));  // 158.203f, 480.461f
path.lineTo(SkBits2Float(0x431e33f8), SkBits2Float(0x43f03b02));  // 158.203f, 480.461f
path.lineTo(SkBits2Float(0x431d4c08), SkBits2Float(0x43ef720c));  // 157.297f, 478.891f
path.lineTo(SkBits2Float(0x431e33f8), SkBits2Float(0x43f03b02));  // 158.203f, 480.461f
path.close();
path.moveTo(SkBits2Float(0x431c6419), SkBits2Float(0x43eea7f0));  // 156.391f, 477.312f
path.cubicTo(SkBits2Float(0x431d6e15), SkBits2Float(0x43ee5ae2), SkBits2Float(0x431e2000), SkBits2Float(0x43ede000), SkBits2Float(0x431e69fc), SkBits2Float(0x43ed55e4));  // 157.43f, 476.71f, 158.125f, 475.75f, 158.414f, 474.671f
path.cubicTo(SkBits2Float(0x431eb3f8), SkBits2Float(0x43eccbc8), SkBits2Float(0x431e93f8), SkBits2Float(0x43ec35e4), SkBits2Float(0x431df9db), SkBits2Float(0x43ebafe0));  // 158.703f, 473.592f, 158.578f, 472.421f, 157.976f, 471.374f
path.lineTo(SkBits2Float(0x432121cb), SkBits2Float(0x43eac6ea));  // 161.132f, 469.554f
path.cubicTo(SkBits2Float(0x432355c3), SkBits2Float(0x43ecb0e6), SkBits2Float(0x432207ae), SkBits2Float(0x43ef1fe0), SkBits2Float(0x431e33b7), SkBits2Float(0x43f03ae2));  // 163.335f, 473.382f, 162.03f, 478.249f, 158.202f, 480.46f
path.lineTo(SkBits2Float(0x431c6419), SkBits2Float(0x43eea7f0));  // 156.391f, 477.312f
path.close();
path.moveTo(SkBits2Float(0x43134c08), SkBits2Float(0x43eec4fe));  // 147.297f, 477.539f
path.lineTo(SkBits2Float(0x43134c08), SkBits2Float(0x43eec4fe));  // 147.297f, 477.539f
path.lineTo(SkBits2Float(0x43134c08), SkBits2Float(0x43eec4fe));  // 147.297f, 477.539f
path.lineTo(SkBits2Float(0x4314e20c), SkBits2Float(0x43ee5106));  // 148.883f, 476.633f
path.lineTo(SkBits2Float(0x43134c08), SkBits2Float(0x43eec4fe));  // 147.297f, 477.539f
path.close();
path.moveTo(SkBits2Float(0x431673f8), SkBits2Float(0x43eddc08));  // 150.453f, 475.719f
path.cubicTo(SkBits2Float(0x43170e15), SkBits2Float(0x43ee620c), SkBits2Float(0x43180000), SkBits2Float(0x43eebb02), SkBits2Float(0x43191604), SkBits2Float(0x43eee000));  // 151.055f, 476.766f, 152, 477.461f, 153.086f, 477.75f
path.cubicTo(SkBits2Float(0x431a2c08), SkBits2Float(0x43ef04fe), SkBits2Float(0x431b5810), SkBits2Float(0x43eef4fe), SkBits2Float(0x431c6418), SkBits2Float(0x43eea7f0));  // 154.172f, 478.039f, 155.344f, 477.914f, 156.391f, 477.312f
path.lineTo(SkBits2Float(0x431e33f7), SkBits2Float(0x43f03ae2));  // 158.203f, 480.46f
path.cubicTo(SkBits2Float(0x431a620b), SkBits2Float(0x43f154de), SkBits2Float(0x4315820c), SkBits2Float(0x43f0add4), SkBits2Float(0x43134c07), SkBits2Float(0x43eec4de));  // 154.383f, 482.663f, 149.508f, 481.358f, 147.297f, 477.538f
path.lineTo(SkBits2Float(0x431673f8), SkBits2Float(0x43eddc08));  // 150.453f, 475.719f
path.close();
path.moveTo(SkBits2Float(0x43163a1d), SkBits2Float(0x43e95106));  // 150.227f, 466.633f
path.lineTo(SkBits2Float(0x43163a1d), SkBits2Float(0x43e95106));  // 150.227f, 466.633f
path.lineTo(SkBits2Float(0x4317220d), SkBits2Float(0x43ea19fc));  // 151.133f, 468.203f
path.lineTo(SkBits2Float(0x43163a1d), SkBits2Float(0x43e95106));  // 150.227f, 466.633f
path.close();
path.moveTo(SkBits2Float(0x43180c08), SkBits2Float(0x43eae3f8));  // 152.047f, 469.781f
path.cubicTo(SkBits2Float(0x43170000), SkBits2Float(0x43eb31ec), SkBits2Float(0x43164e14), SkBits2Float(0x43ebabe8), SkBits2Float(0x43160418), SkBits2Float(0x43ec3604));  // 151, 470.39f, 150.305f, 471.343f, 150.016f, 472.422f
path.cubicTo(SkBits2Float(0x4315ba1c), SkBits2Float(0x43ecc106), SkBits2Float(0x4315d810), SkBits2Float(0x43ed570a), SkBits2Float(0x43167439), SkBits2Float(0x43eddc08));  // 149.727f, 473.508f, 149.844f, 474.68f, 150.454f, 475.719f
path.lineTo(SkBits2Float(0x43134c49), SkBits2Float(0x43eec4fe));  // 147.298f, 477.539f
path.cubicTo(SkBits2Float(0x43111851), SkBits2Float(0x43ecdb02), SkBits2Float(0x43126830), SkBits2Float(0x43ea6c08), SkBits2Float(0x43163a5d), SkBits2Float(0x43e95106));  // 145.095f, 473.711f, 146.407f, 468.844f, 150.228f, 466.633f
path.lineTo(SkBits2Float(0x43180c08), SkBits2Float(0x43eae3f8));  // 152.047f, 469.781f
path.close();
path.moveTo(SkBits2Float(0x431dfa1d), SkBits2Float(0x43ebb000));  // 157.977f, 471.375f
path.cubicTo(SkBits2Float(0x431d620d), SkBits2Float(0x43eb29fc), SkBits2Float(0x431c6e15), SkBits2Float(0x43ead20c), SkBits2Float(0x431b5811), SkBits2Float(0x43eaad0e));  // 157.383f, 470.328f, 156.43f, 469.641f, 155.344f, 469.352f
path.cubicTo(SkBits2Float(0x431a420d), SkBits2Float(0x43ea8810), SkBits2Float(0x43191605), SkBits2Float(0x43ea970a), SkBits2Float(0x43180c09), SkBits2Float(0x43eae418));  // 154.258f, 469.063f, 153.086f, 469.18f, 152.047f, 469.782f
path.lineTo(SkBits2Float(0x43163a1d), SkBits2Float(0x43e95126));  // 150.227f, 466.634f
path.cubicTo(SkBits2Float(0x431a0c09), SkBits2Float(0x43e8372a), SkBits2Float(0x431eec08), SkBits2Float(0x43e8de34), SkBits2Float(0x4321220d), SkBits2Float(0x43eac72a));  // 154.047f, 464.431f, 158.922f, 465.736f, 161.133f, 469.556f
path.lineTo(SkBits2Float(0x431dfa1d), SkBits2Float(0x43ebb000));  // 157.977f, 471.375f
path.close();
testSimplify(reporter, path, filename);
}

static void joel_8(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
path.moveTo(SkBits2Float(0x42d97520), SkBits2Float(0x410ac429)); // 108.729f, 8.67289f
path.cubicTo(SkBits2Float(0x42d97520), SkBits2Float(0x410ac429), SkBits2Float(0x42e9a9ce), SkBits2Float(0x41834e87), SkBits2Float(0x42e99c8c), SkBits2Float(0x41c5c960)); // 108.729f, 8.67289f, 116.832f, 16.4133f, 116.806f, 24.7233f
path.cubicTo(SkBits2Float(0x42e98f49), SkBits2Float(0x4204221c), SkBits2Float(0x42d97520), SkBits2Float(0x4223825f), SkBits2Float(0x42d97520), SkBits2Float(0x4223825f)); // 116.78f, 33.0333f, 108.729f, 40.8773f, 108.729f, 40.8773f
path.cubicTo(SkBits2Float(0x42d97520), SkBits2Float(0x4223825f), SkBits2Float(0x42dbbc54), SkBits2Float(0x42099f18), SkBits2Float(0x42d1cb74), SkBits2Float(0x41f77dc0)); // 108.729f, 40.8773f, 109.868f, 34.4054f, 104.897f, 30.9364f
path.cubicTo(SkBits2Float(0x42c7da94), SkBits2Float(0x41dbbd4f), SkBits2Float(0x42b1b1a1), SkBits2Float(0x41d802fb), SkBits2Float(0x42b1b1a1), SkBits2Float(0x41d802fb)); // 99.9269f, 27.4674f, 88.8469f, 27.0015f, 88.8469f, 27.0015f
path.cubicTo(SkBits2Float(0x42a75637), SkBits2Float(0x41d6909f), SkBits2Float(0x4296c543), SkBits2Float(0x41f1b139), SkBits2Float(0x4296c543), SkBits2Float(0x41f1b139)); // 83.6684f, 26.8206f, 75.3853f, 30.2115f, 75.3853f, 30.2115f
path.lineTo(SkBits2Float(0x42824475), SkBits2Float(0x41c69d70)); // 65.1337f, 24.8269f
path.lineTo(SkBits2Float(0x4296c543), SkBits2Float(0x419b89a8)); // 75.3853f, 19.4422f
path.cubicTo(SkBits2Float(0x4296c543), SkBits2Float(0x419b89a8), SkBits2Float(0x42a6b798), SkBits2Float(0x41b89815), SkBits2Float(0x42b1b1a1), SkBits2Float(0x41b95c48)); // 75.3853f, 19.4422f, 83.3586f, 23.0743f, 88.8469f, 23.1701f
path.cubicTo(SkBits2Float(0x42b1b1a1), SkBits2Float(0x41b95c48), SkBits2Float(0x42c80258), SkBits2Float(0x41b03f7a), SkBits2Float(0x42d1cb74), SkBits2Float(0x419340ee)); // 88.8469f, 23.1701f, 100.005f, 22.031f, 104.897f, 18.4067f
path.cubicTo(SkBits2Float(0x42db9490), SkBits2Float(0x416c84c2), SkBits2Float(0x42d97520), SkBits2Float(0x410ac42a), SkBits2Float(0x42d97520), SkBits2Float(0x410ac42a)); // 109.79f, 14.7824f, 108.729f, 8.67289f, 108.729f, 8.67289f
path.lineTo(SkBits2Float(0x42d97520), SkBits2Float(0x410ac429)); // 108.729f, 8.67289f
path.close();
testSimplify(reporter, path, filename);
}

static void joel_9(skiatest::Reporter* reporter, const char* filename) {
#if DEBUG_UNDER_DEVELOPMENT
// fails with image mismatch
    SkPath path;
path.moveTo(SkBits2Float(0x4310dbe7), SkBits2Float(0x438e9604));  // 144.859f, 285.172f
path.lineTo(SkBits2Float(0x4310dbe7), SkBits2Float(0x438e9604));  // 144.859f, 285.172f
path.lineTo(SkBits2Float(0x4310dbe7), SkBits2Float(0x438e9604));  // 144.859f, 285.172f
path.lineTo(SkBits2Float(0x430f21ca), SkBits2Float(0x438e4efa));  // 143.132f, 284.617f
path.lineTo(SkBits2Float(0x4310dbe7), SkBits2Float(0x438e9604));  // 144.859f, 285.172f
path.close();
path.moveTo(SkBits2Float(0x4307ec08), SkBits2Float(0x438f6c08));  // 135.922f, 286.844f
path.lineTo(SkBits2Float(0x4307ec08), SkBits2Float(0x438f6c08));  // 135.922f, 286.844f
path.lineTo(SkBits2Float(0x4307ec08), SkBits2Float(0x438f6c08));  // 135.922f, 286.844f
path.lineTo(SkBits2Float(0x43075df4), SkBits2Float(0x43904916));  // 135.367f, 288.571f
path.lineTo(SkBits2Float(0x4307ec08), SkBits2Float(0x438f6c08));  // 135.922f, 286.844f
path.close();
path.moveTo(SkBits2Float(0x4307ec08), SkBits2Float(0x438f6c08));  // 135.922f, 286.844f
path.cubicTo(SkBits2Float(0x430911eb), SkBits2Float(0x438f9c08), SkBits2Float(0x430a3df4), SkBits2Float(0x438f8b02), SkBits2Float(0x430b3df4), SkBits2Float(0x438f49fc));  // 137.07f, 287.219f, 138.242f, 287.086f, 139.242f, 286.578f
path.cubicTo(SkBits2Float(0x430c3be8), SkBits2Float(0x438f09fc), SkBits2Float(0x430d07f0), SkBits2Float(0x438e99fc), SkBits2Float(0x430d67f0), SkBits2Float(0x438e070a));  // 140.234f, 286.078f, 141.031f, 285.203f, 141.406f, 284.055f
path.lineTo(SkBits2Float(0x4310dbe8), SkBits2Float(0x438e9604));  // 144.859f, 285.172f
path.cubicTo(SkBits2Float(0x430f7df4), SkBits2Float(0x4390b000), SkBits2Float(0x430afdf4), SkBits2Float(0x4391d3f8), SkBits2Float(0x4306cc09), SkBits2Float(0x43912604));  // 143.492f, 289.375f, 138.992f, 291.656f, 134.797f, 290.297f
path.lineTo(SkBits2Float(0x4307ec08), SkBits2Float(0x438f6c08));  // 135.922f, 286.844f
path.close();
path.moveTo(SkBits2Float(0x4301ae14), SkBits2Float(0x438c1efa));  // 129.68f, 280.242f
path.lineTo(SkBits2Float(0x4301ae14), SkBits2Float(0x438c1efa));  // 129.68f, 280.242f
path.lineTo(SkBits2Float(0x4301ae14), SkBits2Float(0x438c1efa));  // 129.68f, 280.242f
path.lineTo(SkBits2Float(0x43036831), SkBits2Float(0x438c66ea));  // 131.407f, 280.804f
path.lineTo(SkBits2Float(0x4301ae14), SkBits2Float(0x438c1efa));  // 129.68f, 280.242f
path.close();
path.moveTo(SkBits2Float(0x4305220c), SkBits2Float(0x438caefa));  // 133.133f, 281.367f
path.cubicTo(SkBits2Float(0x4304c20c), SkBits2Float(0x438d4106), SkBits2Float(0x4304e20c), SkBits2Float(0x438dd7f0), SkBits2Float(0x43056418), SkBits2Float(0x438e56ea));  // 132.758f, 282.508f, 132.883f, 283.687f, 133.391f, 284.679f
path.cubicTo(SkBits2Float(0x4305e831), SkBits2Float(0x438ed6ea), SkBits2Float(0x4306c624), SkBits2Float(0x438f3be8), SkBits2Float(0x4307ec08), SkBits2Float(0x438f6be8));  // 133.907f, 285.679f, 134.774f, 286.468f, 135.922f, 286.843f
path.lineTo(SkBits2Float(0x4306cc08), SkBits2Float(0x439125e4));  // 134.797f, 290.296f
path.cubicTo(SkBits2Float(0x43029a1c), SkBits2Float(0x439076ea), SkBits2Float(0x43005021), SkBits2Float(0x438e37f0), SkBits2Float(0x4301ae14), SkBits2Float(0x438c1eda));  // 130.602f, 288.929f, 128.313f, 284.437f, 129.68f, 280.241f
path.lineTo(SkBits2Float(0x4305220c), SkBits2Float(0x438caefa));  // 133.133f, 281.367f
path.close();
path.moveTo(SkBits2Float(0x430bbdf4), SkBits2Float(0x43898efa));  // 139.742f, 275.117f
path.lineTo(SkBits2Float(0x430bbdf4), SkBits2Float(0x43898efa));  // 139.742f, 275.117f
path.lineTo(SkBits2Float(0x430b2e15), SkBits2Float(0x438a6c08));  // 139.18f, 276.844f
path.lineTo(SkBits2Float(0x430bbdf4), SkBits2Float(0x43898efa));  // 139.742f, 275.117f
path.close();
path.moveTo(SkBits2Float(0x430a9be7), SkBits2Float(0x438b48f6));  // 138.609f, 278.57f
path.cubicTo(SkBits2Float(0x43097604), SkBits2Float(0x438b19fc), SkBits2Float(0x43084c08), SkBits2Float(0x438b29fc), SkBits2Float(0x43074c08), SkBits2Float(0x438b6b02));  // 137.461f, 278.203f, 136.297f, 278.328f, 135.297f, 278.836f
path.cubicTo(SkBits2Float(0x43064c08), SkBits2Float(0x438bac08), SkBits2Float(0x4305820c), SkBits2Float(0x438c1c08), SkBits2Float(0x4305220c), SkBits2Float(0x438caefa));  // 134.297f, 279.344f, 133.508f, 280.219f, 133.133f, 281.367f
path.lineTo(SkBits2Float(0x4301ae14), SkBits2Float(0x438c1efa));  // 129.68f, 280.242f
path.cubicTo(SkBits2Float(0x43030c08), SkBits2Float(0x438a04fe), SkBits2Float(0x430789fb), SkBits2Float(0x4388e106), SkBits2Float(0x430bbdf3), SkBits2Float(0x43898efa));  // 131.047f, 276.039f, 135.539f, 273.758f, 139.742f, 275.117f
path.lineTo(SkBits2Float(0x430a9be7), SkBits2Float(0x438b48f6));  // 138.609f, 278.57f
path.close();
path.moveTo(SkBits2Float(0x430d67f0), SkBits2Float(0x438e070a));  // 141.406f, 284.055f
path.cubicTo(SkBits2Float(0x430dc5e4), SkBits2Float(0x438d7418), SkBits2Float(0x430da5e4), SkBits2Float(0x438cde14), SkBits2Float(0x430d25e4), SkBits2Float(0x438c5e14));  // 141.773f, 282.907f, 141.648f, 281.735f, 141.148f, 280.735f
path.cubicTo(SkBits2Float(0x430ca001), SkBits2Float(0x438bde14), SkBits2Float(0x430bc1cb), SkBits2Float(0x438b7916), SkBits2Float(0x430a9be8), SkBits2Float(0x438b4916));  // 140.625f, 279.735f, 139.757f, 278.946f, 138.609f, 278.571f
path.lineTo(SkBits2Float(0x430bbdf4), SkBits2Float(0x43898f1a));  // 139.742f, 275.118f
path.cubicTo(SkBits2Float(0x430fefe0), SkBits2Float(0x438a3f1a), SkBits2Float(0x43123811), SkBits2Float(0x438c7d0e), SkBits2Float(0x4310dbe8), SkBits2Float(0x438e9624));  // 143.937f, 276.493f, 146.219f, 280.977f, 144.859f, 285.173f
path.lineTo(SkBits2Float(0x430d67f0), SkBits2Float(0x438e070a));  // 141.406f, 284.055f
path.close();
testSimplify(reporter, path, filename);
#endif
}

static void joel_10(skiatest::Reporter* reporter, const char* filename) {
#if DEBUG_UNDER_DEVELOPMENT
// fails with image mismatch
    SkPath path;
path.moveTo(SkBits2Float(0x440fc979), SkBits2Float(0x43d88000));  // 575.148f, 433
path.lineTo(SkBits2Float(0x440fc979), SkBits2Float(0x43d88000));  // 575.148f, 433
path.lineTo(SkBits2Float(0x440fc979), SkBits2Float(0x43d88000));  // 575.148f, 433
path.lineTo(SkBits2Float(0x44103800), SkBits2Float(0x43d8c7f0));  // 576.875f, 433.562f
path.lineTo(SkBits2Float(0x440fc979), SkBits2Float(0x43d88000));  // 575.148f, 433
path.close();
path.moveTo(SkBits2Float(0x44124d81), SkBits2Float(0x43d5f000));  // 585.211f, 427.875f
path.lineTo(SkBits2Float(0x44124d81), SkBits2Float(0x43d5f000));  // 585.211f, 427.875f
path.lineTo(SkBits2Float(0x44122906), SkBits2Float(0x43d6cdf4));  // 584.641f, 429.609f
path.lineTo(SkBits2Float(0x44124d81), SkBits2Float(0x43d5f000));  // 585.211f, 427.875f
path.close();
path.moveTo(SkBits2Float(0x44120581), SkBits2Float(0x43d7ab02));  // 584.086f, 431.336f
path.cubicTo(SkBits2Float(0x4411bc08), SkBits2Float(0x43d77b02), SkBits2Float(0x44117083), SkBits2Float(0x43d78b02), SkBits2Float(0x44113106), SkBits2Float(0x43d7cc08));  // 582.938f, 430.961f, 581.758f, 431.086f, 580.766f, 431.594f
path.cubicTo(SkBits2Float(0x4410f189), SkBits2Float(0x43d80d0e), SkBits2Float(0x4410be87), SkBits2Float(0x43d87d0e), SkBits2Float(0x4410a687), SkBits2Float(0x43d91000));  // 579.774f, 432.102f, 578.977f, 432.977f, 578.602f, 434.125f
path.lineTo(SkBits2Float(0x440fc979), SkBits2Float(0x43d88000));  // 575.148f, 433
path.cubicTo(SkBits2Float(0x441020f6), SkBits2Float(0x43d66604), SkBits2Float(0x441140f6), SkBits2Float(0x43d5420c), SkBits2Float(0x44124d71), SkBits2Float(0x43d5f000));  // 576.515f, 428.797f, 581.015f, 426.516f, 585.21f, 427.875f
path.lineTo(SkBits2Float(0x44120581), SkBits2Float(0x43d7ab02));  // 584.086f, 431.336f
path.close();
path.moveTo(SkBits2Float(0x441394fe), SkBits2Float(0x43daf810));  // 590.328f, 437.938f
path.lineTo(SkBits2Float(0x441394fe), SkBits2Float(0x43daf810));  // 590.328f, 437.938f
path.lineTo(SkBits2Float(0x441394fe), SkBits2Float(0x43daf810));  // 590.328f, 437.938f
path.lineTo(SkBits2Float(0x44132677), SkBits2Float(0x43dab020));  // 588.601f, 437.376f
path.lineTo(SkBits2Float(0x441394fe), SkBits2Float(0x43daf810));  // 590.328f, 437.938f
path.close();
path.moveTo(SkBits2Float(0x4412b800), SkBits2Float(0x43da67f0));  // 586.875f, 436.812f
path.cubicTo(SkBits2Float(0x4412d000), SkBits2Float(0x43d9d4fe), SkBits2Float(0x4412c800), SkBits2Float(0x43d94000), SkBits2Float(0x4412a77d), SkBits2Float(0x43d8befa));  // 587.25f, 435.664f, 587.125f, 434.5f, 586.617f, 433.492f
path.cubicTo(SkBits2Float(0x44128677), SkBits2Float(0x43d84000), SkBits2Float(0x44124efa), SkBits2Float(0x43d7d9fc), SkBits2Float(0x44120581), SkBits2Float(0x43d7ab02));  // 586.101f, 432.5f, 585.234f, 431.703f, 584.086f, 431.336f
path.lineTo(SkBits2Float(0x44124d81), SkBits2Float(0x43d5f000));  // 585.211f, 427.875f
path.cubicTo(SkBits2Float(0x441359fc), SkBits2Float(0x43d69efa), SkBits2Float(0x4413ec7b), SkBits2Float(0x43d8ddf4), SkBits2Float(0x441394fe), SkBits2Float(0x43daf7f0));  // 589.406f, 429.242f, 591.695f, 433.734f, 590.328f, 437.937f
path.lineTo(SkBits2Float(0x4412b800), SkBits2Float(0x43da67f0));  // 586.875f, 436.812f
path.close();
path.moveTo(SkBits2Float(0x44111106), SkBits2Float(0x43dd870a));  // 580.266f, 443.055f
path.lineTo(SkBits2Float(0x44111106), SkBits2Float(0x43dd870a));  // 580.266f, 443.055f
path.lineTo(SkBits2Float(0x44111106), SkBits2Float(0x43dd870a));  // 580.266f, 443.055f
path.lineTo(SkBits2Float(0x441134fe), SkBits2Float(0x43dca9fc));  // 580.828f, 441.328f
path.lineTo(SkBits2Float(0x44111106), SkBits2Float(0x43dd870a));  // 580.266f, 443.055f
path.close();
path.moveTo(SkBits2Float(0x44115979), SkBits2Float(0x43dbcd0e));  // 581.398f, 439.602f
path.cubicTo(SkBits2Float(0x4411a27f), SkBits2Float(0x43dbfc08), SkBits2Float(0x4411ed71), SkBits2Float(0x43dbed0e), SkBits2Float(0x44122d71), SkBits2Float(0x43dbac08));  // 582.539f, 439.969f, 583.71f, 439.852f, 584.71f, 439.344f
path.cubicTo(SkBits2Float(0x44126cee), SkBits2Float(0x43db6b02), SkBits2Float(0x44129ff0), SkBits2Float(0x43dafb02), SkBits2Float(0x4412b7f0), SkBits2Float(0x43da6810));  // 585.702f, 438.836f, 586.499f, 437.961f, 586.874f, 436.813f
path.lineTo(SkBits2Float(0x441394ee), SkBits2Float(0x43daf810));  // 590.327f, 437.938f
path.cubicTo(SkBits2Float(0x44133cee), SkBits2Float(0x43dd1106), SkBits2Float(0x44121df4), SkBits2Float(0x43de3604), SkBits2Float(0x441110f6), SkBits2Float(0x43dd870a));  // 588.952f, 442.133f, 584.468f, 444.422f, 580.265f, 443.055f
path.lineTo(SkBits2Float(0x44115979), SkBits2Float(0x43dbcd0e));  // 581.398f, 439.602f
path.close();
path.moveTo(SkBits2Float(0x4410a687), SkBits2Float(0x43d91000));  // 578.602f, 434.125f
path.cubicTo(SkBits2Float(0x44108f0a), SkBits2Float(0x43d9a2f2), SkBits2Float(0x44109687), SkBits2Float(0x43da37f0), SkBits2Float(0x4410b70a), SkBits2Float(0x43dab8f6));  // 578.235f, 435.273f, 578.352f, 436.437f, 578.86f, 437.445f
path.cubicTo(SkBits2Float(0x4410d78d), SkBits2Float(0x43db37f0), SkBits2Float(0x44111010), SkBits2Float(0x43db9cee), SkBits2Float(0x44115989), SkBits2Float(0x43dbccee));  // 579.368f, 438.437f, 580.251f, 439.226f, 581.399f, 439.601f
path.lineTo(SkBits2Float(0x44111106), SkBits2Float(0x43dd86ea));  // 580.266f, 443.054f
path.cubicTo(SkBits2Float(0x4410048b), SkBits2Float(0x43dcd7f0), SkBits2Float(0x440f720c), SkBits2Float(0x43da99dc), SkBits2Float(0x440fc989), SkBits2Float(0x43d87fe0));  // 576.071f, 441.687f, 573.782f, 437.202f, 575.149f, 432.999f
path.lineTo(SkBits2Float(0x4410a687), SkBits2Float(0x43d91000));  // 578.602f, 434.125f
path.close();
testSimplify(reporter, path, filename);
#endif
}

static void joel_11(skiatest::Reporter* reporter, const char* filename) {
#if DEBUG_UNDER_DEVELOPMENT
// fails with image mismatch
    SkPath path;
path.moveTo(SkBits2Float(0x43c9d000), SkBits2Float(0x4411977d));  // 403.625f, 582.367f
path.lineTo(SkBits2Float(0x43c9d000), SkBits2Float(0x4411977d));  // 403.625f, 582.367f
path.lineTo(SkBits2Float(0x43c9d000), SkBits2Float(0x4411977d));  // 403.625f, 582.367f
path.lineTo(SkBits2Float(0x43ca0106), SkBits2Float(0x441208f6));  // 404.008f, 584.14f
path.lineTo(SkBits2Float(0x43c9d000), SkBits2Float(0x4411977d));  // 403.625f, 582.367f
path.close();
path.moveTo(SkBits2Float(0x43ce8d0e), SkBits2Float(0x44132106));  // 413.102f, 588.516f
path.lineTo(SkBits2Float(0x43ce8d0e), SkBits2Float(0x44132106));  // 413.102f, 588.516f
path.lineTo(SkBits2Float(0x43ce8d0e), SkBits2Float(0x44132106));  // 413.102f, 588.516f
path.lineTo(SkBits2Float(0x43cda916), SkBits2Float(0x44133989));  // 411.321f, 588.899f
path.lineTo(SkBits2Float(0x43ce8d0e), SkBits2Float(0x44132106));  // 413.102f, 588.516f
path.close();
path.moveTo(SkBits2Float(0x43ccc4fe), SkBits2Float(0x44135179));  // 409.539f, 589.273f
path.cubicTo(SkBits2Float(0x43cca604), SkBits2Float(0x44130571), SkBits2Float(0x43cc4c08), SkBits2Float(0x4412c8f6), SkBits2Float(0x43cbd4fe), SkBits2Float(0x4412a179));  // 409.297f, 588.085f, 408.594f, 587.14f, 407.664f, 586.523f
path.cubicTo(SkBits2Float(0x43cb5c08), SkBits2Float(0x44127a7f), SkBits2Float(0x43cac7f0), SkBits2Float(0x44126af2), SkBits2Float(0x43ca3106), SkBits2Float(0x44127af2));  // 406.719f, 585.914f, 405.562f, 585.671f, 404.383f, 585.921f
path.lineTo(SkBits2Float(0x43c9d000), SkBits2Float(0x4411976d));  // 403.625f, 582.366f
path.cubicTo(SkBits2Float(0x43cbf9fc), SkBits2Float(0x44115cee), SkBits2Float(0x43ce170a), SkBits2Float(0x44120cee), SkBits2Float(0x43ce8d0e), SkBits2Float(0x441320e6));  // 407.953f, 581.452f, 412.18f, 584.202f, 413.102f, 588.514f
path.lineTo(SkBits2Float(0x43ccc4fe), SkBits2Float(0x44135179));  // 409.539f, 589.273f
path.close();
path.moveTo(SkBits2Float(0x43cb78f6), SkBits2Float(0x44157efa));  // 406.945f, 597.984f
path.lineTo(SkBits2Float(0x43cb78f6), SkBits2Float(0x44157f7d));  // 406.945f, 597.992f
path.lineTo(SkBits2Float(0x43cb78f6), SkBits2Float(0x44157efa));  // 406.945f, 597.984f
path.lineTo(SkBits2Float(0x43cb49fc), SkBits2Float(0x44150d81));  // 406.578f, 596.211f
path.lineTo(SkBits2Float(0x43cb78f6), SkBits2Float(0x44157efa));  // 406.945f, 597.984f
path.close();
path.moveTo(SkBits2Float(0x43cb18f6), SkBits2Float(0x44149b85));  // 406.195f, 594.43f
path.cubicTo(SkBits2Float(0x43cbb000), SkBits2Float(0x44148b85), SkBits2Float(0x43cc28f6), SkBits2Float(0x44145f0a), SkBits2Float(0x43cc76ea), SkBits2Float(0x44142302));  // 407.375f, 594.18f, 408.32f, 593.485f, 408.929f, 592.547f
path.cubicTo(SkBits2Float(0x43ccc4de), SkBits2Float(0x4413e687), SkBits2Float(0x43cce4de), SkBits2Float(0x44139cfe), SkBits2Float(0x43ccc4de), SkBits2Float(0x44135189));  // 409.538f, 591.602f, 409.788f, 590.453f, 409.538f, 589.274f
path.lineTo(SkBits2Float(0x43ce8cce), SkBits2Float(0x44132106));  // 413.1f, 588.516f
path.cubicTo(SkBits2Float(0x43cf00c6), SkBits2Float(0x44143581), SkBits2Float(0x43cda2d2), SkBits2Float(0x44154408), SkBits2Float(0x43cb78d6), SkBits2Float(0x44157f0a));  // 414.006f, 592.836f, 411.272f, 597.063f, 406.944f, 597.985f
path.lineTo(SkBits2Float(0x43cb18f6), SkBits2Float(0x44149b85));  // 406.195f, 594.43f
path.close();
path.moveTo(SkBits2Float(0x43c883f8), SkBits2Float(0x4413c4fe));  // 401.031f, 591.078f
path.lineTo(SkBits2Float(0x43c883f8), SkBits2Float(0x4413c4fe));  // 401.031f, 591.078f
path.lineTo(SkBits2Float(0x43c883f8), SkBits2Float(0x4413c4fe));  // 401.031f, 591.078f
path.lineTo(SkBits2Float(0x43c7a106), SkBits2Float(0x4413dd81));  // 399.258f, 591.461f
path.lineTo(SkBits2Float(0x43c883f8), SkBits2Float(0x4413c4fe));  // 401.031f, 591.078f
path.close();
path.moveTo(SkBits2Float(0x43c883f8), SkBits2Float(0x4413c4fe));  // 401.031f, 591.078f
path.cubicTo(SkBits2Float(0x43c8a4fe), SkBits2Float(0x44141083), SkBits2Float(0x43c8fdf4), SkBits2Float(0x44144d81), SkBits2Float(0x43c974fe), SkBits2Float(0x4414747b));  // 401.289f, 592.258f, 401.984f, 593.211f, 402.914f, 593.82f
path.cubicTo(SkBits2Float(0x43c9edf4), SkBits2Float(0x44149b75), SkBits2Float(0x43ca820c), SkBits2Float(0x4414ab75), SkBits2Float(0x43cb18f6), SkBits2Float(0x44149b75));  // 403.859f, 594.429f, 405.016f, 594.679f, 406.195f, 594.429f
path.lineTo(SkBits2Float(0x43cb78f6), SkBits2Float(0x44157efa));  // 406.945f, 597.984f
path.cubicTo(SkBits2Float(0x43c95000), SkBits2Float(0x4415b979), SkBits2Float(0x43c732f2), SkBits2Float(0x441509fc), SkBits2Float(0x43c6bcee), SkBits2Float(0x4413f581));  // 402.625f, 598.898f, 398.398f, 596.156f, 397.476f, 591.836f
path.lineTo(SkBits2Float(0x43c883f8), SkBits2Float(0x4413c4fe));  // 401.031f, 591.078f
path.close();
path.moveTo(SkBits2Float(0x43ca3106), SkBits2Float(0x44127b02));  // 404.383f, 585.922f
path.cubicTo(SkBits2Float(0x43c999fc), SkBits2Float(0x44128b02), SkBits2Float(0x43c92000), SkBits2Float(0x4412b77d), SkBits2Float(0x43c8d20c), SkBits2Float(0x4412f408));  // 403.203f, 586.172f, 402.25f, 586.867f, 401.641f, 587.813f
path.cubicTo(SkBits2Float(0x43c88418), SkBits2Float(0x44133010), SkBits2Float(0x43c86418), SkBits2Float(0x44137989), SkBits2Float(0x43c88418), SkBits2Float(0x4413c50e));  // 401.032f, 588.751f, 400.782f, 589.899f, 401.032f, 591.079f
path.lineTo(SkBits2Float(0x43c6bd0e), SkBits2Float(0x4413f591));  // 397.477f, 591.837f
path.cubicTo(SkBits2Float(0x43c64810), SkBits2Float(0x4412e116), SkBits2Float(0x43c7a70a), SkBits2Float(0x4411d28f), SkBits2Float(0x43c9d000), SkBits2Float(0x4411978d));  // 396.563f, 587.517f, 399.305f, 583.29f, 403.625f, 582.368f
path.lineTo(SkBits2Float(0x43ca3106), SkBits2Float(0x44127b02));  // 404.383f, 585.922f
path.close();
testSimplify(reporter, path, filename);
#endif
}

static void make_joel_12(SkPath& path) {
path.moveTo(SkBits2Float(0x4324e9fc), SkBits2Float(0x437211ec));  // 164.914f, 242.07f
path.lineTo(SkBits2Float(0x4324e9fc), SkBits2Float(0x437211ec));  // 164.914f, 242.07f
path.lineTo(SkBits2Float(0x4324e9fc), SkBits2Float(0x437211ec));  // 164.914f, 242.07f
path.lineTo(SkBits2Float(0x43235810), SkBits2Float(0x437129fc));  // 163.344f, 241.164f
path.lineTo(SkBits2Float(0x4324e9fc), SkBits2Float(0x437211ec));  // 164.914f, 242.07f
path.close();
path.moveTo(SkBits2Float(0x431a020c), SkBits2Float(0x4374fdf4));  // 154.008f, 244.992f
path.lineTo(SkBits2Float(0x431a020c), SkBits2Float(0x4374fdf4));  // 154.008f, 244.992f
path.lineTo(SkBits2Float(0x431a020c), SkBits2Float(0x4374fdf4));  // 154.008f, 244.992f
path.lineTo(SkBits2Float(0x431aec08), SkBits2Float(0x437369fc));  // 154.922f, 243.414f
path.lineTo(SkBits2Float(0x431a020c), SkBits2Float(0x4374fdf4));  // 154.008f, 244.992f
path.close();
path.moveTo(SkBits2Float(0x431bd3f8), SkBits2Float(0x4371d810));  // 155.828f, 241.844f
path.cubicTo(SkBits2Float(0x431ce000), SkBits2Float(0x4372722d), SkBits2Float(0x431e0e15), SkBits2Float(0x43729020), SkBits2Float(0x431f2000), SkBits2Float(0x43724831));  // 156.875f, 242.446f, 158.055f, 242.563f, 159.125f, 242.282f
path.cubicTo(SkBits2Float(0x43203604), SkBits2Float(0x4371fe35), SkBits2Float(0x43212c08), SkBits2Float(0x43714a3d), SkBits2Float(0x4321c5e3), SkBits2Float(0x43704041));  // 160.211f, 241.993f, 161.172f, 241.29f, 161.773f, 240.251f
path.lineTo(SkBits2Float(0x4324e9fc), SkBits2Float(0x4372122d));  // 164.914f, 242.071f
path.cubicTo(SkBits2Float(0x4322b3f8), SkBits2Float(0x4375e419), SkBits2Float(0x431dd810), SkBits2Float(0x4377322d), SkBits2Float(0x431a020c), SkBits2Float(0x4374fe35));  // 162.703f, 245.891f, 157.844f, 247.196f, 154.008f, 244.993f
path.lineTo(SkBits2Float(0x431bd3f8), SkBits2Float(0x4371d810));  // 155.828f, 241.844f
path.close();
path.moveTo(SkBits2Float(0x43171810), SkBits2Float(0x436a1604));  // 151.094f, 234.086f
path.lineTo(SkBits2Float(0x43171810), SkBits2Float(0x436a1604));  // 151.094f, 234.086f
path.lineTo(SkBits2Float(0x43171810), SkBits2Float(0x436a1604));  // 151.094f, 234.086f
path.lineTo(SkBits2Float(0x4318a9fc), SkBits2Float(0x436afdf4));  // 152.664f, 234.992f
path.lineTo(SkBits2Float(0x43171810), SkBits2Float(0x436a1604));  // 151.094f, 234.086f
path.close();
path.moveTo(SkBits2Float(0x431a4000), SkBits2Float(0x436be7f0));  // 154.25f, 235.906f
path.cubicTo(SkBits2Float(0x4319a20c), SkBits2Float(0x436cf3f8), SkBits2Float(0x431985e3), SkBits2Float(0x436e1df4), SkBits2Float(0x4319ce14), SkBits2Float(0x436f33f8));  // 153.633f, 236.953f, 153.523f, 238.117f, 153.805f, 239.203f
path.cubicTo(SkBits2Float(0x431a1a1c), SkBits2Float(0x437047f0), SkBits2Float(0x431ac831), SkBits2Float(0x43713df4), SkBits2Float(0x431bd3f7), SkBits2Float(0x4371d811));  // 154.102f, 240.281f, 154.782f, 241.242f, 155.828f, 241.844f
path.lineTo(SkBits2Float(0x431a020b), SkBits2Float(0x4374fdf4));  // 154.008f, 244.992f
path.cubicTo(SkBits2Float(0x4316322c), SkBits2Float(0x4372c5e4), SkBits2Float(0x4314e417), SkBits2Float(0x436de9fc), SkBits2Float(0x4317180f), SkBits2Float(0x436a1604));  // 150.196f, 242.773f, 148.891f, 237.914f, 151.094f, 234.086f
path.lineTo(SkBits2Float(0x431a4000), SkBits2Float(0x436be7f0));  // 154.25f, 235.906f
path.close();
path.moveTo(SkBits2Float(0x43220000), SkBits2Float(0x436729fc));  // 162, 231.164f
path.lineTo(SkBits2Float(0x43220000), SkBits2Float(0x436729fc));  // 162, 231.164f
path.lineTo(SkBits2Float(0x43220000), SkBits2Float(0x436729fc));  // 162, 231.164f
path.lineTo(SkBits2Float(0x43211810), SkBits2Float(0x4368bbe8));  // 161.094f, 232.734f
path.lineTo(SkBits2Float(0x43220000), SkBits2Float(0x436729fc));  // 162, 231.164f
path.close();
path.moveTo(SkBits2Float(0x43202e14), SkBits2Float(0x436a4fdf));  // 160.18f, 234.312f
path.cubicTo(SkBits2Float(0x431f2418), SkBits2Float(0x4369b5c2), SkBits2Float(0x431df810), SkBits2Float(0x436995c2), SkBits2Float(0x431ce20c), SkBits2Float(0x4369dfbe));  // 159.141f, 233.71f, 157.969f, 233.585f, 156.883f, 233.874f
path.cubicTo(SkBits2Float(0x431bcc08), SkBits2Float(0x436a2bc6), SkBits2Float(0x431ad810), SkBits2Float(0x436adba5), SkBits2Float(0x431a4000), SkBits2Float(0x436be7ae));  // 155.797f, 234.171f, 154.844f, 234.858f, 154.25f, 235.905f
path.lineTo(SkBits2Float(0x43171810), SkBits2Float(0x436a15c2));  // 151.094f, 234.085f
path.cubicTo(SkBits2Float(0x43194e14), SkBits2Float(0x436643d6), SkBits2Float(0x431e2c08), SkBits2Float(0x4364f3b6), SkBits2Float(0x43220000), SkBits2Float(0x436729ba));  // 153.305f, 230.265f, 158.172f, 228.952f, 162, 231.163f
path.lineTo(SkBits2Float(0x43202e14), SkBits2Float(0x436a4fdf));  // 160.18f, 234.312f
path.close();
path.moveTo(SkBits2Float(0x4321c5e3), SkBits2Float(0x43704000));  // 161.773f, 240.25f
path.cubicTo(SkBits2Float(0x43226000), SkBits2Float(0x436f3604), SkBits2Float(0x43228000), SkBits2Float(0x436e09fc), SkBits2Float(0x43223604), SkBits2Float(0x436cf3f8));  // 162.375f, 239.211f, 162.5f, 238.039f, 162.211f, 236.953f
path.cubicTo(SkBits2Float(0x4321ec08), SkBits2Float(0x436be000), SkBits2Float(0x43213a1d), SkBits2Float(0x436ae9fc), SkBits2Float(0x43202e14), SkBits2Float(0x436a4fdf));  // 161.922f, 235.875f, 161.227f, 234.914f, 160.18f, 234.312f
path.lineTo(SkBits2Float(0x43220000), SkBits2Float(0x436729fc));  // 162, 231.164f
path.cubicTo(SkBits2Float(0x4325d1ec), SkBits2Float(0x43696000), SkBits2Float(0x4327220c), SkBits2Float(0x436e4000), SkBits2Float(0x4324e9fc), SkBits2Float(0x437211ec));  // 165.82f, 233.375f, 167.133f, 238.25f, 164.914f, 242.07f
path.lineTo(SkBits2Float(0x4321c5e3), SkBits2Float(0x43704000));  // 161.773f, 240.25f
path.close();
}

static void joel_12(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    make_joel_12(path);
    testSimplify(reporter, path, filename);
}

static void joel_12x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    make_joel_12(path);
    testSimplify(reporter, path, filename);
}

static void make_joel_13(SkPath& path) {
path.moveTo(SkBits2Float(0x43b4126f), SkBits2Float(0x43c058f6));  // 360.144f, 384.695f
path.cubicTo(SkBits2Float(0x43bd7c6b), SkBits2Float(0x43c05b02), SkBits2Float(0x43c51d71), SkBits2Float(0x43b8e8f6), SkBits2Float(0x43c5276d), SkBits2Float(0x43afc1ec));  // 378.972f, 384.711f, 394.23f, 369.82f, 394.308f, 351.515f
path.cubicTo(SkBits2Float(0x43c51d71), SkBits2Float(0x43a688f6), SkBits2Float(0x43bd7c6b), SkBits2Float(0x439f16ea), SkBits2Float(0x43b4126f), SkBits2Float(0x439f16ea));  // 394.23f, 333.07f, 378.972f, 318.179f, 360.144f, 318.179f
path.cubicTo(SkBits2Float(0x43aaa979), SkBits2Float(0x439f16ea), SkBits2Float(0x43a3076d), SkBits2Float(0x43a688f6), SkBits2Float(0x43a31063), SkBits2Float(0x43afc1ec));  // 341.324f, 318.179f, 326.058f, 333.07f, 326.128f, 351.515f
path.cubicTo(SkBits2Float(0x43a3076d), SkBits2Float(0x43b8e8f6), SkBits2Float(0x43aaa959), SkBits2Float(0x43c05b02), SkBits2Float(0x43b4126f), SkBits2Float(0x43c058f6));  // 326.058f, 369.82f, 341.323f, 384.711f, 360.144f, 384.695f
path.close();
}

static void joel_13(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    make_joel_13(path);
    testSimplify(reporter, path, filename);
}


static void joel_13x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    make_joel_13(path);
    testSimplify(reporter, path, filename);
}

static void make_joel_14(SkPath& path) {
path.moveTo(SkBits2Float(0x43f3b354), SkBits2Float(0x43d6770a));  // 487.401f, 428.93f
path.lineTo(SkBits2Float(0x43f3b354), SkBits2Float(0x43d6770a));  // 487.401f, 428.93f
path.close();
path.moveTo(SkBits2Float(0x43f0fd50), SkBits2Float(0x43d6770a));  // 481.979f, 428.93f
path.lineTo(SkBits2Float(0x43f0fd50), SkBits2Float(0x43d6770a));  // 481.979f, 428.93f
path.close();
path.moveTo(SkBits2Float(0x43f0fd50), SkBits2Float(0x43d6770a));  // 481.979f, 428.93f
path.lineTo(SkBits2Float(0x43f3b354), SkBits2Float(0x43d6770a));  // 487.401f, 428.93f
path.lineTo(SkBits2Float(0x43f0fd50), SkBits2Float(0x43d6770a));  // 481.979f, 428.93f
path.close();
path.moveTo(SkBits2Float(0x43dfe76d), SkBits2Float(0x43d792f1));  // 447.808f, 431.148f
path.lineTo(SkBits2Float(0x43dfe76d), SkBits2Float(0x43d792f1));  // 447.808f, 431.148f
path.close();
path.moveTo(SkBits2Float(0x43dfe76d), SkBits2Float(0x43d792f1));  // 447.808f, 431.148f
path.cubicTo(SkBits2Float(0x43e51979), SkBits2Float(0x43d611eb), SkBits2Float(0x43eb8667), SkBits2Float(0x43d765e3), SkBits2Float(0x43f0fd71), SkBits2Float(0x43d676e9));  // 458.199f, 428.14f, 471.05f, 430.796f, 481.98f, 428.929f
path.cubicTo(SkBits2Float(0x43eb8667), SkBits2Float(0x43d76604), SkBits2Float(0x43e51958), SkBits2Float(0x43d6120c), SkBits2Float(0x43dfe76d), SkBits2Float(0x43d792f1));  // 471.05f, 430.797f, 458.198f, 428.141f, 447.808f, 431.148f
path.close();
path.moveTo(SkBits2Float(0x43df776d), SkBits2Float(0x43d6d603));  // 446.933f, 429.672f
path.lineTo(SkBits2Float(0x43df776d), SkBits2Float(0x43d6d603));  // 446.933f, 429.672f
path.close();
path.moveTo(SkBits2Float(0x43df776d), SkBits2Float(0x43d6d603));  // 446.933f, 429.672f
path.lineTo(SkBits2Float(0x43dfe76d), SkBits2Float(0x43d79311));  // 447.808f, 431.149f
path.lineTo(SkBits2Float(0x43df776d), SkBits2Float(0x43d6d603));  // 446.933f, 429.672f
path.close();
path.moveTo(SkBits2Float(0x43dd3169), SkBits2Float(0x43d792f1));  // 442.386f, 431.148f
path.lineTo(SkBits2Float(0x43dd3169), SkBits2Float(0x43d792f1));  // 442.386f, 431.148f
path.close();
path.moveTo(SkBits2Float(0x43dd3169), SkBits2Float(0x43d792f1));  // 442.386f, 431.148f
path.cubicTo(SkBits2Float(0x43de376d), SkBits2Float(0x43d743f7), SkBits2Float(0x43de2873), SkBits2Float(0x43d68df3), SkBits2Float(0x43df776d), SkBits2Float(0x43d6d5e3));  // 444.433f, 430.531f, 444.316f, 429.109f, 446.933f, 429.671f
path.cubicTo(SkBits2Float(0x43de2852), SkBits2Float(0x43d68df3), SkBits2Float(0x43de376d), SkBits2Float(0x43d743f7), SkBits2Float(0x43dd3169), SkBits2Float(0x43d792f1));  // 444.315f, 429.109f, 444.433f, 430.531f, 442.386f, 431.148f
path.close();
path.moveTo(SkBits2Float(0x43dcc169), SkBits2Float(0x43d6d603));  // 441.511f, 429.672f
path.lineTo(SkBits2Float(0x43dcc169), SkBits2Float(0x43d6d603));  // 441.511f, 429.672f
path.close();
path.moveTo(SkBits2Float(0x43dcc169), SkBits2Float(0x43d6d603));  // 441.511f, 429.672f
path.lineTo(SkBits2Float(0x43dd3169), SkBits2Float(0x43d79311));  // 442.386f, 431.149f
path.lineTo(SkBits2Float(0x43dcc169), SkBits2Float(0x43d6d603));  // 441.511f, 429.672f
path.close();
path.moveTo(SkBits2Float(0x43dad959), SkBits2Float(0x43d6d603));  // 437.698f, 429.672f
path.lineTo(SkBits2Float(0x43dad959), SkBits2Float(0x43d6d603));  // 437.698f, 429.672f
path.close();
path.moveTo(SkBits2Float(0x43dad959), SkBits2Float(0x43d6d603));  // 437.698f, 429.672f
path.lineTo(SkBits2Float(0x43dcc149), SkBits2Float(0x43d6d603));  // 441.51f, 429.672f
path.lineTo(SkBits2Float(0x43dad959), SkBits2Float(0x43d6d603));  // 437.698f, 429.672f
path.close();
path.moveTo(SkBits2Float(0x43e3cb65), SkBits2Float(0x43e3bd0d));  // 455.589f, 455.477f
path.lineTo(SkBits2Float(0x43e3cb65), SkBits2Float(0x43e3bd0d));  // 455.589f, 455.477f
path.close();
path.moveTo(SkBits2Float(0x43e3cb65), SkBits2Float(0x43e3bd0d));  // 455.589f, 455.477f
path.lineTo(SkBits2Float(0x43dad959), SkBits2Float(0x43d6d603));  // 437.698f, 429.672f
path.lineTo(SkBits2Float(0x43e3cb65), SkBits2Float(0x43e3bd0d));  // 455.589f, 455.477f
path.close();
path.moveTo(SkBits2Float(0x43e22d51), SkBits2Float(0x43e41b01));  // 452.354f, 456.211f
path.lineTo(SkBits2Float(0x43e22d51), SkBits2Float(0x43e41b01));  // 452.354f, 456.211f
path.close();
path.moveTo(SkBits2Float(0x43e22d51), SkBits2Float(0x43e41b01));  // 452.354f, 456.211f
path.cubicTo(SkBits2Float(0x43e2ba5f), SkBits2Float(0x43e3f9fb), SkBits2Float(0x43e37e57), SkBits2Float(0x43e46df3), SkBits2Float(0x43e3cb45), SkBits2Float(0x43e3bd0d));  // 453.456f, 455.953f, 454.987f, 456.859f, 455.588f, 455.477f
path.cubicTo(SkBits2Float(0x43e37e57), SkBits2Float(0x43e46df2), SkBits2Float(0x43e2ba60), SkBits2Float(0x43e3f9fb), SkBits2Float(0x43e22d51), SkBits2Float(0x43e41b01));  // 454.987f, 456.859f, 453.456f, 455.953f, 452.354f, 456.211f
path.close();
path.moveTo(SkBits2Float(0x43e22d51), SkBits2Float(0x43e479fb));  // 452.354f, 456.953f
path.lineTo(SkBits2Float(0x43e22d51), SkBits2Float(0x43e479fb));  // 452.354f, 456.953f
path.close();
path.moveTo(SkBits2Float(0x43e22d51), SkBits2Float(0x43e479fb));  // 452.354f, 456.953f
path.lineTo(SkBits2Float(0x43e22d51), SkBits2Float(0x43e41b01));  // 452.354f, 456.211f
path.lineTo(SkBits2Float(0x43e22d51), SkBits2Float(0x43e479fb));  // 452.354f, 456.953f
path.close();
path.moveTo(SkBits2Float(0x43e35a5f), SkBits2Float(0x43e479fb));  // 454.706f, 456.953f
path.lineTo(SkBits2Float(0x43e35a5f), SkBits2Float(0x43e479fb));  // 454.706f, 456.953f
path.close();
path.moveTo(SkBits2Float(0x43e35a5f), SkBits2Float(0x43e479fb));  // 454.706f, 456.953f
path.lineTo(SkBits2Float(0x43e22d51), SkBits2Float(0x43e479fb));  // 452.354f, 456.953f
path.lineTo(SkBits2Float(0x43e35a5f), SkBits2Float(0x43e479fb));  // 454.706f, 456.953f
path.close();
path.moveTo(SkBits2Float(0x43e35a5f), SkBits2Float(0x43e41b01));  // 454.706f, 456.211f
path.lineTo(SkBits2Float(0x43e35a5f), SkBits2Float(0x43e41b01));  // 454.706f, 456.211f
path.close();
path.moveTo(SkBits2Float(0x43e35a5f), SkBits2Float(0x43e41b01));  // 454.706f, 456.211f
path.lineTo(SkBits2Float(0x43e35a5f), SkBits2Float(0x43e479fb));  // 454.706f, 456.953f
path.lineTo(SkBits2Float(0x43e35a5f), SkBits2Float(0x43e41b01));  // 454.706f, 456.211f
path.close();
path.moveTo(SkBits2Float(0x43e1726f), SkBits2Float(0x43e90c07));  // 450.894f, 466.094f
path.lineTo(SkBits2Float(0x43e1726f), SkBits2Float(0x43e90c07));  // 450.894f, 466.094f
path.close();
path.moveTo(SkBits2Float(0x43e1726f), SkBits2Float(0x43e90c07));  // 450.894f, 466.094f
path.cubicTo(SkBits2Float(0x43e2226f), SkBits2Float(0x43e769fb), SkBits2Float(0x43e50a7f), SkBits2Float(0x43e63915), SkBits2Float(0x43e35a5f), SkBits2Float(0x43e41b01));  // 452.269f, 462.828f, 458.082f, 460.446f, 454.706f, 456.211f
path.cubicTo(SkBits2Float(0x43e50a5f), SkBits2Float(0x43e638f5), SkBits2Float(0x43e2226f), SkBits2Float(0x43e769fb), SkBits2Float(0x43e1726f), SkBits2Float(0x43e90c07));  // 458.081f, 460.445f, 452.269f, 462.828f, 450.894f, 466.094f
path.close();
path.moveTo(SkBits2Float(0x43f09f5d), SkBits2Float(0x43ea2709));  // 481.245f, 468.305f
path.lineTo(SkBits2Float(0x43f09f5d), SkBits2Float(0x43ea2709));  // 481.245f, 468.305f
path.close();
path.moveTo(SkBits2Float(0x43f09f5d), SkBits2Float(0x43ea2709));  // 481.245f, 468.305f
path.cubicTo(SkBits2Float(0x43ebbc6b), SkBits2Float(0x43ea4105), SkBits2Float(0x43e56c6b), SkBits2Float(0x43ec9fff), SkBits2Float(0x43e1724f), SkBits2Float(0x43e90c07));  // 471.472f, 468.508f, 458.847f, 473.25f, 450.893f, 466.094f
path.cubicTo(SkBits2Float(0x43e56c6c), SkBits2Float(0x43ec9fff), SkBits2Float(0x43ebbc6c), SkBits2Float(0x43ea4105), SkBits2Float(0x43f09f5d), SkBits2Float(0x43ea2709));  // 458.847f, 473.25f, 471.472f, 468.508f, 481.245f, 468.305f
path.close();
path.moveTo(SkBits2Float(0x43eea45b), SkBits2Float(0x43e9c7ee));  // 477.284f, 467.562f
path.lineTo(SkBits2Float(0x43eea45b), SkBits2Float(0x43e9c7ee));  // 477.284f, 467.562f
path.close();
path.moveTo(SkBits2Float(0x43eea45b), SkBits2Float(0x43e9c7ee));  // 477.284f, 467.562f
path.cubicTo(SkBits2Float(0x43ef0c4b), SkBits2Float(0x43ea7ef8), SkBits2Float(0x43eff355), SkBits2Float(0x43ea10e4), SkBits2Float(0x43f09f5d), SkBits2Float(0x43ea26e8));  // 478.096f, 468.992f, 479.901f, 468.132f, 481.245f, 468.304f
path.cubicTo(SkBits2Float(0x43eff355), SkBits2Float(0x43ea1105), SkBits2Float(0x43ef0c6b), SkBits2Float(0x43ea7ef8), SkBits2Float(0x43eea45b), SkBits2Float(0x43e9c7ee));  // 479.901f, 468.133f, 478.097f, 468.992f, 477.284f, 467.562f
path.close();
path.moveTo(SkBits2Float(0x43ee4667), SkBits2Float(0x43ea2709));  // 476.55f, 468.305f
path.lineTo(SkBits2Float(0x43ee4667), SkBits2Float(0x43ea2709));  // 476.55f, 468.305f
path.close();
path.moveTo(SkBits2Float(0x43ee4667), SkBits2Float(0x43ea2709));  // 476.55f, 468.305f
path.lineTo(SkBits2Float(0x43eea45b), SkBits2Float(0x43e9c80f));  // 477.284f, 467.563f
path.lineTo(SkBits2Float(0x43ee4667), SkBits2Float(0x43ea2709));  // 476.55f, 468.305f
path.close();
path.moveTo(SkBits2Float(0x43e9f26f), SkBits2Float(0x43e6c2f0));  // 467.894f, 461.523f
path.lineTo(SkBits2Float(0x43e9f26f), SkBits2Float(0x43e6c2f0));  // 467.894f, 461.523f
path.close();
path.moveTo(SkBits2Float(0x43e9f26f), SkBits2Float(0x43e6c2f0));  // 467.894f, 461.523f
path.cubicTo(SkBits2Float(0x43eb8873), SkBits2Float(0x43e7dcec), SkBits2Float(0x43eb747b), SkBits2Float(0x43ea9b00), SkBits2Float(0x43ee4667), SkBits2Float(0x43ea26e8));  // 471.066f, 463.726f, 470.91f, 469.211f, 476.55f, 468.304f
path.cubicTo(SkBits2Float(0x43eb745b), SkBits2Float(0x43ea9b01), SkBits2Float(0x43eb8853), SkBits2Float(0x43e7dd0d), SkBits2Float(0x43e9f26f), SkBits2Float(0x43e6c2f0));  // 470.909f, 469.211f, 471.065f, 463.727f, 467.894f, 461.523f
path.close();
path.moveTo(SkBits2Float(0x43ebee56), SkBits2Float(0x43decc07));  // 471.862f, 445.594f
path.lineTo(SkBits2Float(0x43ebee56), SkBits2Float(0x43decc07));  // 471.862f, 445.594f
path.close();
path.moveTo(SkBits2Float(0x43ebee56), SkBits2Float(0x43decc07));  // 471.862f, 445.594f
path.cubicTo(SkBits2Float(0x43e85f5c), SkBits2Float(0x43e04915), SkBits2Float(0x43eaa148), SkBits2Float(0x43e41c07), SkBits2Float(0x43e9f24e), SkBits2Float(0x43e6c311));  // 464.745f, 448.571f, 469.26f, 456.219f, 467.893f, 461.524f
path.cubicTo(SkBits2Float(0x43eaa169), SkBits2Float(0x43e41c07), SkBits2Float(0x43e85f5c), SkBits2Float(0x43e048f4), SkBits2Float(0x43ebee56), SkBits2Float(0x43decc07));  // 469.261f, 456.219f, 464.745f, 448.57f, 471.862f, 445.594f
path.close();
path.moveTo(SkBits2Float(0x43eac168), SkBits2Float(0x43dd3fff));  // 469.511f, 442.5f
path.lineTo(SkBits2Float(0x43eac168), SkBits2Float(0x43dd3fff));  // 469.511f, 442.5f
path.close();
path.moveTo(SkBits2Float(0x43eac168), SkBits2Float(0x43dd3fff));  // 469.511f, 442.5f
path.cubicTo(SkBits2Float(0x43eb245a), SkBits2Float(0x43ddc7ef), SkBits2Float(0x43eaf45a), SkBits2Float(0x43dedd0d), SkBits2Float(0x43ebee76), SkBits2Float(0x43decc07));  // 470.284f, 443.562f, 469.909f, 445.727f, 471.863f, 445.594f
path.cubicTo(SkBits2Float(0x43eaf459), SkBits2Float(0x43dedd0d), SkBits2Float(0x43eb2459), SkBits2Float(0x43ddc7ee), SkBits2Float(0x43eac168), SkBits2Float(0x43dd3fff));  // 469.909f, 445.727f, 470.284f, 443.562f, 469.511f, 442.5f
path.close();
path.moveTo(SkBits2Float(0x43ec4c6a), SkBits2Float(0x43dce105));  // 472.597f, 441.758f
path.lineTo(SkBits2Float(0x43ec4c6a), SkBits2Float(0x43dce105));  // 472.597f, 441.758f
path.close();
path.moveTo(SkBits2Float(0x43ec4c6a), SkBits2Float(0x43dce105));  // 472.597f, 441.758f
path.cubicTo(SkBits2Float(0x43ebcb64), SkBits2Float(0x43dd08f5), SkBits2Float(0x43eb0c6a), SkBits2Float(0x43dc9603), SkBits2Float(0x43eac168), SkBits2Float(0x43dd3fff));  // 471.589f, 442.07f, 470.097f, 441.172f, 469.511f, 442.5f
path.cubicTo(SkBits2Float(0x43eb0c6a), SkBits2Float(0x43dc9603), SkBits2Float(0x43ebcb64), SkBits2Float(0x43dd08f5), SkBits2Float(0x43ec4c6a), SkBits2Float(0x43dce105));  // 470.097f, 441.172f, 471.589f, 442.07f, 472.597f, 441.758f
path.close();
path.moveTo(SkBits2Float(0x43ecbb64), SkBits2Float(0x43ddb105));  // 473.464f, 443.383f
path.lineTo(SkBits2Float(0x43ecbb64), SkBits2Float(0x43ddb105));  // 473.464f, 443.383f
path.close();
path.moveTo(SkBits2Float(0x43ecbb64), SkBits2Float(0x43ddb105));  // 473.464f, 443.383f
path.lineTo(SkBits2Float(0x43ec4c6a), SkBits2Float(0x43dce105));  // 472.597f, 441.758f
path.lineTo(SkBits2Float(0x43ecbb64), SkBits2Float(0x43ddb105));  // 473.464f, 443.383f
path.close();
path.moveTo(SkBits2Float(0x43eea45a), SkBits2Float(0x43dc24fd));  // 477.284f, 440.289f
path.lineTo(SkBits2Float(0x43eea45a), SkBits2Float(0x43dc24fd));  // 477.284f, 440.289f
path.close();
path.moveTo(SkBits2Float(0x43eea45a), SkBits2Float(0x43dc24fd));  // 477.284f, 440.289f
path.cubicTo(SkBits2Float(0x43eef354), SkBits2Float(0x43dd4c07), SkBits2Float(0x43ed4a5e), SkBits2Float(0x43dcfef9), SkBits2Float(0x43ecbb64), SkBits2Float(0x43ddb105));  // 477.901f, 442.594f, 474.581f, 441.992f, 473.464f, 443.383f
path.cubicTo(SkBits2Float(0x43ed4a5e), SkBits2Float(0x43dcfef9), SkBits2Float(0x43eef354), SkBits2Float(0x43dd4c07), SkBits2Float(0x43eea45a), SkBits2Float(0x43dc24fd));  // 474.581f, 441.992f, 477.901f, 442.594f, 477.284f, 440.289f
path.close();
path.moveTo(SkBits2Float(0x43f09f5c), SkBits2Float(0x43dc24fd));  // 481.245f, 440.289f
path.lineTo(SkBits2Float(0x43f09f5c), SkBits2Float(0x43dc24fd));  // 481.245f, 440.289f
path.close();
path.moveTo(SkBits2Float(0x43f09f5c), SkBits2Float(0x43dc24fd));  // 481.245f, 440.289f
path.cubicTo(SkBits2Float(0x43effc6a), SkBits2Float(0x43daeced), SkBits2Float(0x43ef6a5e), SkBits2Float(0x43dbe4fd), SkBits2Float(0x43eea45a), SkBits2Float(0x43dc24fd));  // 479.972f, 437.851f, 478.831f, 439.789f, 477.284f, 440.289f
path.cubicTo(SkBits2Float(0x43ef6a5e), SkBits2Float(0x43dbe4fd), SkBits2Float(0x43effc6a), SkBits2Float(0x43daed0d), SkBits2Float(0x43f09f5c), SkBits2Float(0x43dc24fd));  // 478.831f, 439.789f, 479.972f, 437.852f, 481.245f, 440.289f
path.close();
path.moveTo(SkBits2Float(0x43f2f76c), SkBits2Float(0x43dbc603));  // 485.933f, 439.547f
path.lineTo(SkBits2Float(0x43f2f76c), SkBits2Float(0x43dbc603));  // 485.933f, 439.547f
path.close();
path.moveTo(SkBits2Float(0x43f2f76c), SkBits2Float(0x43dbc603));  // 485.933f, 439.547f
path.cubicTo(SkBits2Float(0x43f24c6a), SkBits2Float(0x43dc3b01), SkBits2Float(0x43f16b64), SkBits2Float(0x43dc2311), SkBits2Float(0x43f09f5c), SkBits2Float(0x43dc24fd));  // 484.597f, 440.461f, 482.839f, 440.274f, 481.245f, 440.289f
path.cubicTo(SkBits2Float(0x43f16b64), SkBits2Float(0x43dc23f7), SkBits2Float(0x43f24c6a), SkBits2Float(0x43dc3b01), SkBits2Float(0x43f2f76c), SkBits2Float(0x43dbc603));  // 482.839f, 440.281f, 484.597f, 440.461f, 485.933f, 439.547f
path.close();
path.moveTo(SkBits2Float(0x43f4de55), SkBits2Float(0x43d97d0d));  // 489.737f, 434.977f
path.lineTo(SkBits2Float(0x43f4de55), SkBits2Float(0x43d97d0d));  // 489.737f, 434.977f
path.close();
path.moveTo(SkBits2Float(0x43f4de55), SkBits2Float(0x43d97d0d));  // 489.737f, 434.977f
path.cubicTo(SkBits2Float(0x43f47665), SkBits2Float(0x43da020b), SkBits2Float(0x43f42851), SkBits2Float(0x43db9417), SkBits2Float(0x43f2f74b), SkBits2Float(0x43dbc603));  // 488.925f, 436.016f, 488.315f, 439.157f, 485.932f, 439.547f
path.cubicTo(SkBits2Float(0x43f42851), SkBits2Float(0x43db93f7), SkBits2Float(0x43f47666), SkBits2Float(0x43da020b), SkBits2Float(0x43f4de55), SkBits2Float(0x43d97d0d));  // 488.315f, 439.156f, 488.925f, 436.016f, 489.737f, 434.977f
path.close();
path.moveTo(SkBits2Float(0x43f48061), SkBits2Float(0x43d97d0d));  // 489.003f, 434.977f
path.lineTo(SkBits2Float(0x43f48061), SkBits2Float(0x43d97d0d));  // 489.003f, 434.977f
path.close();
path.moveTo(SkBits2Float(0x43f48061), SkBits2Float(0x43d97d0d));  // 489.003f, 434.977f
path.lineTo(SkBits2Float(0x43f4de55), SkBits2Float(0x43d97d0d));  // 489.737f, 434.977f
path.lineTo(SkBits2Float(0x43f48061), SkBits2Float(0x43d97d0d));  // 489.003f, 434.977f
path.close();
path.moveTo(SkBits2Float(0x43f3b353), SkBits2Float(0x43d67709));  // 487.401f, 428.93f
path.cubicTo(SkBits2Float(0x43f39957), SkBits2Float(0x43d79ef9), SkBits2Float(0x43f3ca5d), SkBits2Float(0x43d8a603), SkBits2Float(0x43f48061), SkBits2Float(0x43d97d0d));  // 487.198f, 431.242f, 487.581f, 433.297f, 489.003f, 434.977f
path.cubicTo(SkBits2Float(0x43f3ca5d), SkBits2Float(0x43d8a603), SkBits2Float(0x43f39957), SkBits2Float(0x43d79ef9), SkBits2Float(0x43f3b353), SkBits2Float(0x43d67709));  // 487.581f, 433.297f, 487.198f, 431.242f, 487.401f, 428.93f
path.close();
}

static void joel_14(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    make_joel_14(path);
testSimplify(reporter, path, filename);
}

static void joel_14x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    make_joel_14(path);
testSimplify(reporter, path, filename);
}

static void make_joel_15(SkPath& path) {
path.moveTo(SkBits2Float(0x439e276d), SkBits2Float(0x43dad106));  // 316.308f, 437.633f
path.lineTo(SkBits2Float(0x439e276d), SkBits2Float(0x43dad106));  // 316.308f, 437.633f
path.close();
path.moveTo(SkBits2Float(0x439c1959), SkBits2Float(0x43d78000));  // 312.198f, 431
path.lineTo(SkBits2Float(0x439c1959), SkBits2Float(0x43d78000));  // 312.198f, 431
path.close();
path.moveTo(SkBits2Float(0x439c1959), SkBits2Float(0x43d78000));  // 312.198f, 431
path.cubicTo(SkBits2Float(0x439ea45b), SkBits2Float(0x43d6d000), SkBits2Float(0x439cce57), SkBits2Float(0x43d9f3f8), SkBits2Float(0x439e274d), SkBits2Float(0x43dad106));  // 317.284f, 429.625f, 313.612f, 435.906f, 316.307f, 437.633f
path.cubicTo(SkBits2Float(0x439cce57), SkBits2Float(0x43d9f3f8), SkBits2Float(0x439ea45b), SkBits2Float(0x43d6d000), SkBits2Float(0x439c1959), SkBits2Float(0x43d78000));  // 313.612f, 435.906f, 317.284f, 429.625f, 312.198f, 431
path.close();
path.moveTo(SkBits2Float(0x439c1959), SkBits2Float(0x43d8f8f6));  // 312.198f, 433.945f
path.lineTo(SkBits2Float(0x439c1959), SkBits2Float(0x43d8f8f6));  // 312.198f, 433.945f
path.close();
path.moveTo(SkBits2Float(0x439c1959), SkBits2Float(0x43d8f8f6));  // 312.198f, 433.945f
path.lineTo(SkBits2Float(0x439c1959), SkBits2Float(0x43d78000));  // 312.198f, 431
path.lineTo(SkBits2Float(0x439c1959), SkBits2Float(0x43d8f8f6));  // 312.198f, 433.945f
path.close();
path.moveTo(SkBits2Float(0x439f7853), SkBits2Float(0x43e5820c));  // 318.94f, 459.016f
path.lineTo(SkBits2Float(0x439f7853), SkBits2Float(0x43e5820c));  // 318.94f, 459.016f
path.close();
path.moveTo(SkBits2Float(0x439f7853), SkBits2Float(0x43e5820c));  // 318.94f, 459.016f
path.cubicTo(SkBits2Float(0x439e1647), SkBits2Float(0x43e17106), SkBits2Float(0x439d945b), SkBits2Float(0x43dd020c), SkBits2Float(0x439c1959), SkBits2Float(0x43d8f916));  // 316.174f, 450.883f, 315.159f, 442.016f, 312.198f, 433.946f
path.cubicTo(SkBits2Float(0x439d945b), SkBits2Float(0x43dd020c), SkBits2Float(0x439e1667), SkBits2Float(0x43e17106), SkBits2Float(0x439f7853), SkBits2Float(0x43e5820c));  // 315.159f, 442.016f, 316.175f, 450.883f, 318.94f, 459.016f
path.close();
path.moveTo(SkBits2Float(0x439ffc6c), SkBits2Float(0x43e7f106));  // 319.972f, 463.883f
path.lineTo(SkBits2Float(0x439ffc6c), SkBits2Float(0x43e7f106));  // 319.972f, 463.883f
path.close();
path.moveTo(SkBits2Float(0x439ffc6c), SkBits2Float(0x43e7f106));  // 319.972f, 463.883f
path.cubicTo(SkBits2Float(0x439f5668), SkBits2Float(0x43e758f6), SkBits2Float(0x439fec6c), SkBits2Float(0x43e63604), SkBits2Float(0x439f7874), SkBits2Float(0x43e5820c));  // 318.675f, 462.695f, 319.847f, 460.422f, 318.941f, 459.016f
path.cubicTo(SkBits2Float(0x439fec6c), SkBits2Float(0x43e63604), SkBits2Float(0x439f5668), SkBits2Float(0x43e758f5), SkBits2Float(0x439ffc6c), SkBits2Float(0x43e7f106));  // 319.847f, 460.422f, 318.675f, 462.695f, 319.972f, 463.883f
path.close();
path.moveTo(SkBits2Float(0x43a12853), SkBits2Float(0x43ede9fc));  // 322.315f, 475.828f
path.lineTo(SkBits2Float(0x43a12853), SkBits2Float(0x43ede9fc));  // 322.315f, 475.828f
path.close();
path.moveTo(SkBits2Float(0x43a12853), SkBits2Float(0x43ede9fc));  // 322.315f, 475.828f
path.cubicTo(SkBits2Float(0x43a18c4b), SkBits2Float(0x43eb7604), SkBits2Float(0x439fe45b), SkBits2Float(0x43ea4b02), SkBits2Float(0x439ffc4b), SkBits2Float(0x43e7f106));  // 323.096f, 470.922f, 319.784f, 468.586f, 319.971f, 463.883f
path.cubicTo(SkBits2Float(0x439fe45b), SkBits2Float(0x43ea4b02), SkBits2Float(0x43a18c6c), SkBits2Float(0x43eb7604), SkBits2Float(0x43a12853), SkBits2Float(0x43ede9fc));  // 319.784f, 468.586f, 323.097f, 470.922f, 322.315f, 475.828f
path.close();
path.moveTo(SkBits2Float(0x43a1e45b), SkBits2Float(0x43ef63f8));  // 323.784f, 478.781f
path.lineTo(SkBits2Float(0x43a1e45b), SkBits2Float(0x43ef63f8));  // 323.784f, 478.781f
path.close();
path.moveTo(SkBits2Float(0x43a1e45b), SkBits2Float(0x43ef63f8));  // 323.784f, 478.781f
path.cubicTo(SkBits2Float(0x43a20561), SkBits2Float(0x43eeb9fc), SkBits2Float(0x43a1ae57), SkBits2Float(0x43ee4be8), SkBits2Float(0x43a12853), SkBits2Float(0x43ede9fc));  // 324.042f, 477.453f, 323.362f, 476.593f, 322.315f, 475.828f
path.cubicTo(SkBits2Float(0x43a1ae57), SkBits2Float(0x43ee4c08), SkBits2Float(0x43a20561), SkBits2Float(0x43eeb9fc), SkBits2Float(0x43a1e45b), SkBits2Float(0x43ef63f8));  // 323.362f, 476.594f, 324.042f, 477.453f, 323.784f, 478.781f
path.close();
path.moveTo(SkBits2Float(0x439fb169), SkBits2Float(0x43f032f2));  // 319.386f, 480.398f
path.lineTo(SkBits2Float(0x439fb169), SkBits2Float(0x43f032f2));  // 319.386f, 480.398f
path.close();
path.moveTo(SkBits2Float(0x439fb169), SkBits2Float(0x43f032f2));  // 319.386f, 480.398f
path.cubicTo(SkBits2Float(0x43a08063), SkBits2Float(0x43f022f2), SkBits2Float(0x43a1ec6b), SkBits2Float(0x43f078f6), SkBits2Float(0x43a1e45b), SkBits2Float(0x43ef63f8));  // 321.003f, 480.273f, 323.847f, 480.945f, 323.784f, 478.781f
path.cubicTo(SkBits2Float(0x43a1ec6b), SkBits2Float(0x43f078f6), SkBits2Float(0x43a08063), SkBits2Float(0x43f022f2), SkBits2Float(0x439fb169), SkBits2Float(0x43f032f2));  // 323.847f, 480.945f, 321.003f, 480.273f, 319.386f, 480.398f
path.close();
path.moveTo(SkBits2Float(0x439e4d50), SkBits2Float(0x43f16106));  // 316.604f, 482.758f
path.lineTo(SkBits2Float(0x439e4d50), SkBits2Float(0x43f16106));  // 316.604f, 482.758f
path.close();
path.moveTo(SkBits2Float(0x439e4d50), SkBits2Float(0x43f16106));  // 316.604f, 482.758f
path.cubicTo(SkBits2Float(0x439de45a), SkBits2Float(0x43f05000), SkBits2Float(0x439f445a), SkBits2Float(0x43f0b20c), SkBits2Float(0x439fb148), SkBits2Float(0x43f03312));  // 315.784f, 480.625f, 318.534f, 481.391f, 319.385f, 480.399f
path.cubicTo(SkBits2Float(0x439f445a), SkBits2Float(0x43f0b20c), SkBits2Float(0x439de45a), SkBits2Float(0x43f05000), SkBits2Float(0x439e4d50), SkBits2Float(0x43f16106));  // 318.534f, 481.391f, 315.784f, 480.625f, 316.604f, 482.758f
path.close();
path.moveTo(SkBits2Float(0x43a0de56), SkBits2Float(0x43f7470a));  // 321.737f, 494.555f
path.lineTo(SkBits2Float(0x43a0de56), SkBits2Float(0x43f7470a));  // 321.737f, 494.555f
path.close();
path.moveTo(SkBits2Float(0x43a0de56), SkBits2Float(0x43f7470a));  // 321.737f, 494.555f
path.cubicTo(SkBits2Float(0x439f4062), SkBits2Float(0x43f5a106), SkBits2Float(0x439f2b64), SkBits2Float(0x43f33106), SkBits2Float(0x439e4d50), SkBits2Float(0x43f16106));  // 318.503f, 491.258f, 318.339f, 486.383f, 316.604f, 482.758f
path.cubicTo(SkBits2Float(0x439f2b64), SkBits2Float(0x43f33106), SkBits2Float(0x439f4062), SkBits2Float(0x43f5a106), SkBits2Float(0x43a0de56), SkBits2Float(0x43f7470a));  // 318.339f, 486.383f, 318.503f, 491.258f, 321.737f, 494.555f
path.close();
path.moveTo(SkBits2Float(0x43a3945a), SkBits2Float(0x43fa13f8));  // 327.159f, 500.156f
path.lineTo(SkBits2Float(0x43a3945a), SkBits2Float(0x43fa13f8));  // 327.159f, 500.156f
path.close();
path.moveTo(SkBits2Float(0x43a3945a), SkBits2Float(0x43fa13f8));  // 327.159f, 500.156f
path.cubicTo(SkBits2Float(0x43a2dc4a), SkBits2Float(0x43f8ab02), SkBits2Float(0x43a0d74c), SkBits2Float(0x43f8f4fe), SkBits2Float(0x43a0de56), SkBits2Float(0x43f746ea));  // 325.721f, 497.336f, 321.682f, 497.914f, 321.737f, 494.554f
path.cubicTo(SkBits2Float(0x43a0d76d), SkBits2Float(0x43f8f4fe), SkBits2Float(0x43a2dc6a), SkBits2Float(0x43f8ab03), SkBits2Float(0x43a3945a), SkBits2Float(0x43fa13f8));  // 321.683f, 497.914f, 325.722f, 497.336f, 327.159f, 500.156f
path.close();
path.moveTo(SkBits2Float(0x43a58e56), SkBits2Float(0x43fa98f6));  // 331.112f, 501.195f
path.lineTo(SkBits2Float(0x43a58e56), SkBits2Float(0x43fa98f6));  // 331.112f, 501.195f
path.close();
path.moveTo(SkBits2Float(0x43a58e56), SkBits2Float(0x43fa98f6));  // 331.112f, 501.195f
path.cubicTo(SkBits2Float(0x43a50148), SkBits2Float(0x43fa2be8), SkBits2Float(0x43a45646), SkBits2Float(0x43fa02f2), SkBits2Float(0x43a3945a), SkBits2Float(0x43fa13f8));  // 330.01f, 500.343f, 328.674f, 500.023f, 327.159f, 500.156f
path.cubicTo(SkBits2Float(0x43a45666), SkBits2Float(0x43fa02f2), SkBits2Float(0x43a50168), SkBits2Float(0x43fa2c08), SkBits2Float(0x43a58e56), SkBits2Float(0x43fa98f6));  // 328.675f, 500.023f, 330.011f, 500.344f, 331.112f, 501.195f
path.close();
path.moveTo(SkBits2Float(0x43a64958), SkBits2Float(0x43f8c000));  // 332.573f, 497.5f
path.lineTo(SkBits2Float(0x43a64958), SkBits2Float(0x43f8c000));  // 332.573f, 497.5f
path.close();
path.moveTo(SkBits2Float(0x43a64958), SkBits2Float(0x43f8c000));  // 332.573f, 497.5f
path.lineTo(SkBits2Float(0x43a58e56), SkBits2Float(0x43fa98f6));  // 331.112f, 501.195f
path.lineTo(SkBits2Float(0x43a64958), SkBits2Float(0x43f8c000));  // 332.573f, 497.5f
path.close();
path.moveTo(SkBits2Float(0x43a73e56), SkBits2Float(0x43f5820c));  // 334.487f, 491.016f
path.lineTo(SkBits2Float(0x43a73e56), SkBits2Float(0x43f5820c));  // 334.487f, 491.016f
path.close();
path.moveTo(SkBits2Float(0x43a73e56), SkBits2Float(0x43f5820c));  // 334.487f, 491.016f
path.cubicTo(SkBits2Float(0x43a64d50), SkBits2Float(0x43f654fe), SkBits2Float(0x43a7174c), SkBits2Float(0x43f7de14), SkBits2Float(0x43a64958), SkBits2Float(0x43f8c000));  // 332.604f, 492.664f, 334.182f, 495.735f, 332.573f, 497.5f
path.cubicTo(SkBits2Float(0x43a7176c), SkBits2Float(0x43f7ddf4), SkBits2Float(0x43a64d50), SkBits2Float(0x43f654fe), SkBits2Float(0x43a73e56), SkBits2Float(0x43f5820c));  // 334.183f, 495.734f, 332.604f, 492.664f, 334.487f, 491.016f
path.close();
path.moveTo(SkBits2Float(0x43a6f26f), SkBits2Float(0x43f20b02));  // 333.894f, 484.086f
path.lineTo(SkBits2Float(0x43a6f26f), SkBits2Float(0x43f20b02));  // 333.894f, 484.086f
path.close();
path.moveTo(SkBits2Float(0x43a6f26f), SkBits2Float(0x43f20b02));  // 333.894f, 484.086f
path.cubicTo(SkBits2Float(0x43a78d71), SkBits2Float(0x43f2f810), SkBits2Float(0x43a72873), SkBits2Float(0x43f453f8), SkBits2Float(0x43a73e77), SkBits2Float(0x43f5820c));  // 335.105f, 485.938f, 334.316f, 488.656f, 334.488f, 491.016f
path.cubicTo(SkBits2Float(0x43a72852), SkBits2Float(0x43f453f8), SkBits2Float(0x43a78d50), SkBits2Float(0x43f2f810), SkBits2Float(0x43a6f26f), SkBits2Float(0x43f20b02));  // 334.315f, 488.656f, 335.104f, 485.938f, 333.894f, 484.086f
path.close();
path.moveTo(SkBits2Float(0x43a6ba5f), SkBits2Float(0x43ef3d0e));  // 333.456f, 478.477f
path.lineTo(SkBits2Float(0x43a6ba5f), SkBits2Float(0x43ef3d0e));  // 333.456f, 478.477f
path.close();
path.moveTo(SkBits2Float(0x43a6ba5f), SkBits2Float(0x43ef3d0e));  // 333.456f, 478.477f
path.cubicTo(SkBits2Float(0x43a60e57), SkBits2Float(0x43f04000), SkBits2Float(0x43a82355), SkBits2Float(0x43f0fc08), SkBits2Float(0x43a6f26f), SkBits2Float(0x43f20b02));  // 332.112f, 480.5f, 336.276f, 481.969f, 333.894f, 484.086f
path.cubicTo(SkBits2Float(0x43a82354), SkBits2Float(0x43f0fc08), SkBits2Float(0x43a60e56), SkBits2Float(0x43f04000), SkBits2Float(0x43a6ba5f), SkBits2Float(0x43ef3d0e));  // 336.276f, 481.969f, 332.112f, 480.5f, 333.456f, 478.477f
path.close();
path.moveTo(SkBits2Float(0x43a35c6b), SkBits2Float(0x43ef88f5));  // 326.722f, 479.07f
path.lineTo(SkBits2Float(0x43a35c6b), SkBits2Float(0x43ef88f5));  // 326.722f, 479.07f
path.close();
path.moveTo(SkBits2Float(0x43a35c6b), SkBits2Float(0x43ef88f5));  // 326.722f, 479.07f
path.cubicTo(SkBits2Float(0x43a4b26f), SkBits2Float(0x43efe105), SkBits2Float(0x43a5b76d), SkBits2Float(0x43ee2ef9), SkBits2Float(0x43a6ba5f), SkBits2Float(0x43ef3ced));  // 329.394f, 479.758f, 331.433f, 476.367f, 333.456f, 478.476f
path.cubicTo(SkBits2Float(0x43a5b76d), SkBits2Float(0x43ee2ef9), SkBits2Float(0x43a4b26f), SkBits2Float(0x43efe106), SkBits2Float(0x43a35c6b), SkBits2Float(0x43ef88f5));  // 331.433f, 476.367f, 329.394f, 479.758f, 326.722f, 479.07f
path.close();
path.moveTo(SkBits2Float(0x43a08063), SkBits2Float(0x43e5a70a));  // 321.003f, 459.305f
path.lineTo(SkBits2Float(0x43a08063), SkBits2Float(0x43e5a70a));  // 321.003f, 459.305f
path.close();
path.moveTo(SkBits2Float(0x43a08063), SkBits2Float(0x43e5a70a));  // 321.003f, 459.305f
path.cubicTo(SkBits2Float(0x43a15169), SkBits2Float(0x43e90312), SkBits2Float(0x43a2626f), SkBits2Float(0x43ec4312), SkBits2Float(0x43a35c6b), SkBits2Float(0x43ef8916));  // 322.636f, 466.024f, 324.769f, 472.524f, 326.722f, 479.071f
path.cubicTo(SkBits2Float(0x43a2626f), SkBits2Float(0x43ec42f1), SkBits2Float(0x43a15169), SkBits2Float(0x43e902f1), SkBits2Float(0x43a08063), SkBits2Float(0x43e5a70a));  // 324.769f, 472.523f, 322.636f, 466.023f, 321.003f, 459.305f
path.close();
path.moveTo(SkBits2Float(0x43a05a5f), SkBits2Float(0x43e407ef));  // 320.706f, 456.062f
path.lineTo(SkBits2Float(0x43a05a5f), SkBits2Float(0x43e407ef));  // 320.706f, 456.062f
path.close();
path.moveTo(SkBits2Float(0x43a05a5f), SkBits2Float(0x43e407ef));  // 320.706f, 456.062f
path.lineTo(SkBits2Float(0x43a08063), SkBits2Float(0x43e5a6e9));  // 321.003f, 459.304f
path.lineTo(SkBits2Float(0x43a05a5f), SkBits2Float(0x43e407ef));  // 320.706f, 456.062f
path.close();
path.moveTo(SkBits2Float(0x439ecf5d), SkBits2Float(0x43dd3fff));  // 317.62f, 442.5f
path.lineTo(SkBits2Float(0x439ecf5d), SkBits2Float(0x43dd3fff));  // 317.62f, 442.5f
path.close();
path.moveTo(SkBits2Float(0x439ecf5d), SkBits2Float(0x43dd3fff));  // 317.62f, 442.5f
path.cubicTo(SkBits2Float(0x439e9c6b), SkBits2Float(0x43dfcb01), SkBits2Float(0x439fbe57), SkBits2Float(0x43e1cc07), SkBits2Float(0x43a05a5f), SkBits2Float(0x43e407ef));  // 317.222f, 447.586f, 319.487f, 451.594f, 320.706f, 456.062f
path.cubicTo(SkBits2Float(0x439fbe57), SkBits2Float(0x43e1cc08), SkBits2Float(0x439e9c6b), SkBits2Float(0x43dfcb01), SkBits2Float(0x439ecf5d), SkBits2Float(0x43dd3fff));  // 319.487f, 451.594f, 317.222f, 447.586f, 317.62f, 442.5f
path.close();
path.moveTo(SkBits2Float(0x439e276d), SkBits2Float(0x43dad105));  // 316.308f, 437.633f
path.cubicTo(SkBits2Float(0x439e4979), SkBits2Float(0x43dba4fd), SkBits2Float(0x439dc375), SkBits2Float(0x43dce915), SkBits2Float(0x439ecf5d), SkBits2Float(0x43dd3fff));  // 316.574f, 439.289f, 315.527f, 441.821f, 317.62f, 442.5f
path.cubicTo(SkBits2Float(0x439dc355), SkBits2Float(0x43dce8f5), SkBits2Float(0x439e4959), SkBits2Float(0x43dba4fd), SkBits2Float(0x439e276d), SkBits2Float(0x43dad105));  // 315.526f, 441.82f, 316.573f, 439.289f, 316.308f, 437.633f
path.close();
}

static void joel_15(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    make_joel_15(path);
testSimplify(reporter, path, filename);
}

static void joel_15x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    make_joel_15(path);
testSimplify(reporter, path, filename);
}

static void make_joel_16(SkPath& path) {
path.moveTo(SkBits2Float(0x420e6c8b), SkBits2Float(0x426bdf3b));  // 35.606f, 58.968f
path.lineTo(SkBits2Float(0x420fcccd), SkBits2Float(0x426c7ef9));  // 35.95f, 59.124f
path.cubicTo(SkBits2Float(0x420fcccd), SkBits2Float(0x426c7ef9), SkBits2Float(0x42093d71), SkBits2Float(0x426c6e97), SkBits2Float(0x42036c8b), SkBits2Float(0x426cbf7c));  // 35.95f, 59.124f, 34.31f, 59.108f, 32.856f, 59.187f
path.cubicTo(SkBits2Float(0x41fb3958), SkBits2Float(0x426d0f5b), SkBits2Float(0x41f076c8), SkBits2Float(0x426d48b3), SkBits2Float(0x41ef47ae), SkBits2Float(0x426d947a));  // 31.403f, 59.265f, 30.058f, 59.321f, 29.91f, 59.395f
path.cubicTo(SkBits2Float(0x41ee1aa0), SkBits2Float(0x426ddf3b), SkBits2Float(0x41ec6041), SkBits2Float(0x426edb22), SkBits2Float(0x41eb1aa0), SkBits2Float(0x426fee97));  // 29.763f, 59.468f, 29.547f, 59.714f, 29.388f, 59.983f
path.cubicTo(SkBits2Float(0x41eb1eb9), SkBits2Float(0x426feb85), SkBits2Float(0x41e9ba5e), SkBits2Float(0x42711eb8), SkBits2Float(0x41e9ba5e), SkBits2Float(0x42711eb8));  // 29.39f, 59.98f, 29.216f, 60.28f, 29.216f, 60.28f
path.lineTo(SkBits2Float(0x41e99999), SkBits2Float(0x42718f5c));  // 29.2f, 60.39f
path.cubicTo(SkBits2Float(0x41ea76c8), SkBits2Float(0x4271a5e3), SkBits2Float(0x4212dd2f), SkBits2Float(0x42707efa), SkBits2Float(0x4212dd2f), SkBits2Float(0x42707efa));  // 29.308f, 60.412f, 36.716f, 60.124f, 36.716f, 60.124f
path.cubicTo(SkBits2Float(0x4212dd2f), SkBits2Float(0x42707efa), SkBits2Float(0x42124395), SkBits2Float(0x42707be8), SkBits2Float(0x42131ba6), SkBits2Float(0x4270b646));  // 36.716f, 60.124f, 36.566f, 60.121f, 36.777f, 60.178f
path.cubicTo(SkBits2Float(0x42131581), SkBits2Float(0x42710000), SkBits2Float(0x42130831), SkBits2Float(0x42711688), SkBits2Float(0x4213072b), SkBits2Float(0x42711688));  // 36.771f, 60.25f, 36.758f, 60.272f, 36.757f, 60.272f
path.cubicTo(SkBits2Float(0x4212fae1), SkBits2Float(0x42711aa1), SkBits2Float(0x42127cee), SkBits2Float(0x42714eda), SkBits2Float(0x42127cee), SkBits2Float(0x42714eda));  // 36.745f, 60.276f, 36.622f, 60.327f, 36.622f, 60.327f
path.cubicTo(SkBits2Float(0x42127ae2), SkBits2Float(0x42714eda), SkBits2Float(0x41c67ae2), SkBits2Float(0x42730f5d), SkBits2Float(0x41c345a2), SkBits2Float(0x427329fd));  // 36.62f, 60.327f, 24.81f, 60.765f, 24.409f, 60.791f
path.cubicTo(SkBits2Float(0x41c247ae), SkBits2Float(0x42733e78), SkBits2Float(0x41c04396), SkBits2Float(0x42738e57), SkBits2Float(0x41bf4bc7), SkBits2Float(0x4273e45b));  // 24.285f, 60.811f, 24.033f, 60.889f, 23.912f, 60.973f
path.cubicTo(SkBits2Float(0x41bf5c29), SkBits2Float(0x4273e042), SkBits2Float(0x41be9db3), SkBits2Float(0x4274322e), SkBits2Float(0x41be9db3), SkBits2Float(0x4274322e));  // 23.92f, 60.969f, 23.827f, 61.049f, 23.827f, 61.049f
path.lineTo(SkBits2Float(0x41be26ea), SkBits2Float(0x42746c8c));  // 23.769f, 61.106f
path.cubicTo(SkBits2Float(0x41be1eb9), SkBits2Float(0x427470a5), SkBits2Float(0x41bde354), SkBits2Float(0x42748313), SkBits2Float(0x41bde354), SkBits2Float(0x42748313));  // 23.765f, 61.11f, 23.736f, 61.128f, 23.736f, 61.128f
path.lineTo(SkBits2Float(0x41bcc083), SkBits2Float(0x42751582));  // 23.594f, 61.271f
path.lineTo(SkBits2Float(0x41bcf3b6), SkBits2Float(0x427526ea));  // 23.619f, 61.288f
path.lineTo(SkBits2Float(0x41bd0e56), SkBits2Float(0x42756979));  // 23.632f, 61.353f
path.lineTo(SkBits2Float(0x41bd7cee), SkBits2Float(0x42758313));  // 23.686f, 61.378f
path.cubicTo(SkBits2Float(0x41be8107), SkBits2Float(0x427572b1), SkBits2Float(0x41bf2d0f), SkBits2Float(0x42754290), SkBits2Float(0x41bfd2f2), SkBits2Float(0x4275147b));  // 23.813f, 61.362f, 23.897f, 61.315f, 23.978f, 61.27f
path.lineTo(SkBits2Float(0x41c0ba5f), SkBits2Float(0x4274da1d));  // 24.091f, 61.213f
path.lineTo(SkBits2Float(0x41c0ef9e), SkBits2Float(0x4274de36));  // 24.117f, 61.217f
path.lineTo(SkBits2Float(0x41c13f7d), SkBits2Float(0x4274d3f9));  // 24.156f, 61.207f
path.cubicTo(SkBits2Float(0x41c13f7d), SkBits2Float(0x4274d3f9), SkBits2Float(0x41c174bc), SkBits2Float(0x4274c18a), SkBits2Float(0x41c17cee), SkBits2Float(0x4274be78));  // 24.156f, 61.207f, 24.182f, 61.189f, 24.186f, 61.186f
path.cubicTo(SkBits2Float(0x41c18107), SkBits2Float(0x4274bf7e), SkBits2Float(0x41c1e561), SkBits2Float(0x4274b022), SkBits2Float(0x41c1e561), SkBits2Float(0x4274b022));  // 24.188f, 61.187f, 24.237f, 61.172f, 24.237f, 61.172f
path.lineTo(SkBits2Float(0x41c45e36), SkBits2Float(0x42746e99));  // 24.546f, 61.108f
path.cubicTo(SkBits2Float(0x41c4624f), SkBits2Float(0x42746e99), SkBits2Float(0x41cf999a), SkBits2Float(0x42743853), SkBits2Float(0x41cf999a), SkBits2Float(0x42743853));  // 24.548f, 61.108f, 25.95f, 61.055f, 25.95f, 61.055f
path.lineTo(SkBits2Float(0x420d126f), SkBits2Float(0x4272b43a));  // 35.268f, 60.676f
path.cubicTo(SkBits2Float(0x420d0938), SkBits2Float(0x4272c084), SkBits2Float(0x420cfcee), SkBits2Float(0x4272c49c), SkBits2Float(0x420cfcee), SkBits2Float(0x4272d917));  // 35.259f, 60.688f, 35.247f, 60.692f, 35.247f, 60.712f
path.lineTo(SkBits2Float(0x420d0938), SkBits2Float(0x4272b43a));  // 35.259f, 60.676f
path.cubicTo(SkBits2Float(0x420c7be8), SkBits2Float(0x42737efb), SkBits2Float(0x420b3128), SkBits2Float(0x42743128), SkBits2Float(0x420a27f0), SkBits2Float(0x4274c18a));  // 35.121f, 60.874f, 34.798f, 61.048f, 34.539f, 61.189f
path.lineTo(SkBits2Float(0x42099eb9), SkBits2Float(0x42750c4b));  // 34.405f, 61.262f
path.cubicTo(SkBits2Float(0x420872b1), SkBits2Float(0x4275b022), SkBits2Float(0x4206fbe8), SkBits2Float(0x42764397), SkBits2Float(0x42054396), SkBits2Float(0x4276c084));  // 34.112f, 61.422f, 33.746f, 61.566f, 33.316f, 61.688f
path.cubicTo(SkBits2Float(0x42028313), SkBits2Float(0x42776b86), SkBits2Float(0x42007be8), SkBits2Float(0x4278de36), SkBits2Float(0x41fe7ae2), SkBits2Float(0x427b0f5d));  // 32.628f, 61.855f, 32.121f, 62.217f, 31.81f, 62.765f
path.cubicTo(SkBits2Float(0x41fe4fe0), SkBits2Float(0x427b21cc), SkBits2Float(0x41fdbe78), SkBits2Float(0x427b8419), SkBits2Float(0x41fdbe78), SkBits2Float(0x427b8419));  // 31.789f, 62.783f, 31.718f, 62.879f, 31.718f, 62.879f
path.cubicTo(SkBits2Float(0x41fdccce), SkBits2Float(0x427b71aa), SkBits2Float(0x41fd1cad), SkBits2Float(0x427c27f0), SkBits2Float(0x41fd1cad), SkBits2Float(0x427c27f0));  // 31.725f, 62.861f, 31.639f, 63.039f, 31.639f, 63.039f
path.lineTo(SkBits2Float(0x41fc1eb9), SkBits2Float(0x427d178e));  // 31.515f, 63.273f
path.lineTo(SkBits2Float(0x41fc7efb), SkBits2Float(0x427d020d));  // 31.562f, 63.252f
path.lineTo(SkBits2Float(0x41fbb647), SkBits2Float(0x427d3646));  // 31.464f, 63.303f
path.lineTo(SkBits2Float(0x41fbe76e), SkBits2Float(0x427d25e4));  // 31.488f, 63.287f
path.lineTo(SkBits2Float(0x41fae149), SkBits2Float(0x427d1fbf));  // 31.36f, 63.281f
path.lineTo(SkBits2Float(0x41fa5812), SkBits2Float(0x427d178e));  // 31.293f, 63.273f
path.cubicTo(SkBits2Float(0x41f88108), SkBits2Float(0x427cf9dc), SkBits2Float(0x41f73541), SkBits2Float(0x427cb646), SkBits2Float(0x41f5d70c), SkBits2Float(0x427c6d92));  // 31.063f, 63.244f, 30.901f, 63.178f, 30.73f, 63.107f
path.lineTo(SkBits2Float(0x41f5999b), SkBits2Float(0x427c6148));  // 30.7f, 63.095f
path.cubicTo(SkBits2Float(0x41f5999b), SkBits2Float(0x427c6148), SkBits2Float(0x41f2d0e7), SkBits2Float(0x427bdc29), SkBits2Float(0x41f2a9fd), SkBits2Float(0x427bd4fe));  // 30.7f, 63.095f, 30.352f, 62.965f, 30.333f, 62.958f
path.cubicTo(SkBits2Float(0x41f28d51), SkBits2Float(0x427bc49c), SkBits2Float(0x41f26667), SkBits2Float(0x427bb021), SkBits2Float(0x41f26667), SkBits2Float(0x427bb021));  // 30.319f, 62.942f, 30.3f, 62.922f, 30.3f, 62.922f
path.lineTo(SkBits2Float(0x41efed92), SkBits2Float(0x427b1db2));  // 29.991f, 62.779f
path.lineTo(SkBits2Float(0x41ec9582), SkBits2Float(0x427a624e));  // 29.573f, 62.596f
path.cubicTo(SkBits2Float(0x41eca1cc), SkBits2Float(0x427a645a), SkBits2Float(0x41eaf9dc), SkBits2Float(0x427a3021), SkBits2Float(0x41eaf9dc), SkBits2Float(0x427a3021));  // 29.579f, 62.598f, 29.372f, 62.547f, 29.372f, 62.547f
path.cubicTo(SkBits2Float(0x41eaf9dc), SkBits2Float(0x427a3021), SkBits2Float(0x41ea126f), SkBits2Float(0x427a1894), SkBits2Float(0x41e9f3b7), SkBits2Float(0x427a1687));  // 29.372f, 62.547f, 29.259f, 62.524f, 29.244f, 62.522f
path.cubicTo(SkBits2Float(0x41e9ccce), SkBits2Float(0x427a072b), SkBits2Float(0x41e99375), SkBits2Float(0x4279f1aa), SkBits2Float(0x41e99375), SkBits2Float(0x4279f1aa));  // 29.225f, 62.507f, 29.197f, 62.486f, 29.197f, 62.486f
path.lineTo(SkBits2Float(0x41e86e98), SkBits2Float(0x4279d604));  // 29.054f, 62.459f
path.lineTo(SkBits2Float(0x41e6147b), SkBits2Float(0x4279a3d7));  // 28.76f, 62.41f
path.cubicTo(SkBits2Float(0x41e00625), SkBits2Float(0x42796b85), SkBits2Float(0x41db49ba), SkBits2Float(0x427a7ae1), SkBits2Float(0x41d62b02), SkBits2Float(0x427bc8b4));  // 28.003f, 62.355f, 27.411f, 62.62f, 26.771f, 62.946f
path.cubicTo(SkBits2Float(0x41d24fdf), SkBits2Float(0x427cba5e), SkBits2Float(0x41cecccd), SkBits2Float(0x427ce872), SkBits2Float(0x41ca0e56), SkBits2Float(0x427c6872));  // 26.289f, 63.182f, 25.85f, 63.227f, 25.257f, 63.102f
path.cubicTo(SkBits2Float(0x41ca0a3d), SkBits2Float(0x427c676c), SkBits2Float(0x41c9353f), SkBits2Float(0x427c570a), SkBits2Float(0x41c9353f), SkBits2Float(0x427c570a));  // 25.255f, 63.101f, 25.151f, 63.085f, 25.151f, 63.085f
path.lineTo(SkBits2Float(0x41c73b64), SkBits2Float(0x427c26e9));  // 24.904f, 63.038f
path.lineTo(SkBits2Float(0x41c774bc), SkBits2Float(0x427c374b));  // 24.932f, 63.054f
path.lineTo(SkBits2Float(0x41c67ef9), SkBits2Float(0x427c0312));  // 24.812f, 63.003f
path.cubicTo(SkBits2Float(0x41c4df3b), SkBits2Float(0x427bc5a1), SkBits2Float(0x41c2a3d6), SkBits2Float(0x427b8d4f), SkBits2Float(0x41c0851e), SkBits2Float(0x427b6978));  // 24.609f, 62.943f, 24.33f, 62.888f, 24.065f, 62.853f
path.cubicTo(SkBits2Float(0x41bf1893), SkBits2Float(0x427b52f1), SkBits2Float(0x41bd2d0e), SkBits2Float(0x427b52f1), SkBits2Float(0x41bc020c), SkBits2Float(0x427b5e34));  // 23.887f, 62.831f, 23.647f, 62.831f, 23.501f, 62.842f
path.lineTo(SkBits2Float(0x41bac6a8), SkBits2Float(0x427b6871));  // 23.347f, 62.852f
path.cubicTo(SkBits2Float(0x41b9db23), SkBits2Float(0x427b72ae), SkBits2Float(0x41b87cee), SkBits2Float(0x427b820b), SkBits2Float(0x41b7fbe7), SkBits2Float(0x427b655f));  // 23.232f, 62.862f, 23.061f, 62.877f, 22.998f, 62.849f
path.cubicTo(SkBits2Float(0x41b7fbe7), SkBits2Float(0x427b5f3a), SkBits2Float(0x41b7dd2f), SkBits2Float(0x427b48b3), SkBits2Float(0x41b7dd2f), SkBits2Float(0x427b48b3));  // 22.998f, 62.843f, 22.983f, 62.821f, 22.983f, 62.821f
path.lineTo(SkBits2Float(0x41b7a5e3), SkBits2Float(0x427b22d0));  // 22.956f, 62.784f
path.cubicTo(SkBits2Float(0x41b7be76), SkBits2Float(0x427b3332), SkBits2Float(0x41b74395), SkBits2Float(0x427aed91), SkBits2Float(0x41b74395), SkBits2Float(0x427aed91));  // 22.968f, 62.8f, 22.908f, 62.732f, 22.908f, 62.732f
path.lineTo(SkBits2Float(0x41b70c49), SkBits2Float(0x427acfdf));  // 22.881f, 62.703f
path.cubicTo(SkBits2Float(0x41b70418), SkBits2Float(0x427ad916), SkBits2Float(0x41b6d70a), SkBits2Float(0x427a9168), SkBits2Float(0x41b6d70a), SkBits2Float(0x427a9168));  // 22.877f, 62.712f, 22.855f, 62.642f, 22.855f, 62.642f
path.lineTo(SkBits2Float(0x41b6bc6a), SkBits2Float(0x427a645a));  // 22.842f, 62.598f
path.lineTo(SkBits2Float(0x41b66e97), SkBits2Float(0x427a75c2));  // 22.804f, 62.615f
path.cubicTo(SkBits2Float(0x41b6872a), SkBits2Float(0x427a71a9), SkBits2Float(0x41b5a9fb), SkBits2Float(0x4279c6a7), SkBits2Float(0x41b5a9fb), SkBits2Float(0x4279c6a7));  // 22.816f, 62.611f, 22.708f, 62.444f, 22.708f, 62.444f
path.lineTo(SkBits2Float(0x41b59580), SkBits2Float(0x4279b645));  // 22.698f, 62.428f
path.lineTo(SkBits2Float(0x41b549b9), SkBits2Float(0x42799fbe));  // 22.661f, 62.406f
path.lineTo(SkBits2Float(0x41b53957), SkBits2Float(0x42799ba5));  // 22.653f, 62.402f
path.cubicTo(SkBits2Float(0x41b52b01), SkBits2Float(0x42798d4f), SkBits2Float(0x41b4a3d6), SkBits2Float(0x427920c4), SkBits2Float(0x41b4a3d6), SkBits2Float(0x427920c4));  // 22.646f, 62.388f, 22.58f, 62.282f, 22.58f, 62.282f
path.lineTo(SkBits2Float(0x41b43126), SkBits2Float(0x4278be76));  // 22.524f, 62.186f
path.lineTo(SkBits2Float(0x41b3ed90), SkBits2Float(0x4278ab01));  // 22.491f, 62.167f
path.lineTo(SkBits2Float(0x41b3be75), SkBits2Float(0x42789ba5));  // 22.468f, 62.152f
path.lineTo(SkBits2Float(0x41b3d0e4), SkBits2Float(0x4278b957));  // 22.477f, 62.181f
path.lineTo(SkBits2Float(0x41b351ea), SkBits2Float(0x42786353));  // 22.415f, 62.097f
path.lineTo(SkBits2Float(0x41b33957), SkBits2Float(0x42786353));  // 22.403f, 62.097f
path.cubicTo(SkBits2Float(0x41b326e8), SkBits2Float(0x42785a1c), SkBits2Float(0x41b2fbe6), SkBits2Float(0x427846a7), SkBits2Float(0x41b2fbe6), SkBits2Float(0x427846a7));  // 22.394f, 62.088f, 22.373f, 62.069f, 22.373f, 62.069f
path.lineTo(SkBits2Float(0x41b2353e), SkBits2Float(0x4277f8d4));  // 22.276f, 61.993f
path.cubicTo(SkBits2Float(0x41b26040), SkBits2Float(0x42780624), SkBits2Float(0x41b16e96), SkBits2Float(0x4277d0e4), SkBits2Float(0x41b16e96), SkBits2Float(0x4277d0e4));  // 22.297f, 62.006f, 22.179f, 61.954f, 22.179f, 61.954f
path.cubicTo(SkBits2Float(0x41b16e96), SkBits2Float(0x4277d0e4), SkBits2Float(0x41b10417), SkBits2Float(0x4277c188), SkBits2Float(0x41b0fffe), SkBits2Float(0x4277c188));  // 22.179f, 61.954f, 22.127f, 61.939f, 22.125f, 61.939f
path.cubicTo(SkBits2Float(0x41b0fffe), SkBits2Float(0x4277bf7c), SkBits2Float(0x41b03f7b), SkBits2Float(0x427778d4), SkBits2Float(0x41b03f7b), SkBits2Float(0x427778d4));  // 22.125f, 61.937f, 22.031f, 61.868f, 22.031f, 61.868f
path.lineTo(SkBits2Float(0x41ae8729), SkBits2Float(0x4276f7ce));  // 21.816f, 61.742f
path.cubicTo(SkBits2Float(0x41adb644), SkBits2Float(0x4276d0e5), SkBits2Float(0x41ad22cf), SkBits2Float(0x42768e55), SkBits2Float(0x41ac8729), SkBits2Float(0x427648b3));  // 21.714f, 61.704f, 21.642f, 61.639f, 21.566f, 61.571f
path.lineTo(SkBits2Float(0x41ab957f), SkBits2Float(0x4275e24d));  // 21.448f, 61.471f
path.cubicTo(SkBits2Float(0x41aa8f5a), SkBits2Float(0x42757df3), SkBits2Float(0x41a9b644), SkBits2Float(0x42751fbe), SkBits2Float(0x41a8a3d5), SkBits2Float(0x42747fff));  // 21.32f, 61.373f, 21.214f, 61.281f, 21.08f, 61.125f
path.cubicTo(SkBits2Float(0x41a6d708), SkBits2Float(0x4273a3d6), SkBits2Float(0x41a645a0), SkBits2Float(0x4272dd2e), SkBits2Float(0x41a58935), SkBits2Float(0x4271b126));  // 20.855f, 60.91f, 20.784f, 60.716f, 20.692f, 60.423f
path.lineTo(SkBits2Float(0x41a5851c), SkBits2Float(0x4271a7ef));  // 20.69f, 60.414f
path.lineTo(SkBits2Float(0x41a56a7c), SkBits2Float(0x42719687));  // 20.677f, 60.397f
path.lineTo(SkBits2Float(0x41a54dd0), SkBits2Float(0x4271820c));  // 20.663f, 60.377f
path.cubicTo(SkBits2Float(0x41a50209), SkBits2Float(0x42711062), SkBits2Float(0x41a4ced6), SkBits2Float(0x42707efa), SkBits2Float(0x41a4be74), SkBits2Float(0x426ff4bc));  // 20.626f, 60.266f, 20.601f, 60.124f, 20.593f, 59.989f
path.cubicTo(SkBits2Float(0x41a51478), SkBits2Float(0x427073b6), SkBits2Float(0x41a576c6), SkBits2Float(0x42710b43), SkBits2Float(0x41a576c6), SkBits2Float(0x42710b43));  // 20.635f, 60.113f, 20.683f, 60.261f, 20.683f, 60.261f
path.cubicTo(SkBits2Float(0x41a71478), SkBits2Float(0x42730418), SkBits2Float(0x41a9df39), SkBits2Float(0x42746666), SkBits2Float(0x41adc6a5), SkBits2Float(0x427526e9));  // 20.885f, 60.754f, 21.234f, 61.1f, 21.722f, 61.288f
path.cubicTo(SkBits2Float(0x41adc499), SkBits2Float(0x427525e3), SkBits2Float(0x41ae47ab), SkBits2Float(0x42754395), SkBits2Float(0x41ae47ab), SkBits2Float(0x42754395));  // 21.721f, 61.287f, 21.785f, 61.316f, 21.785f, 61.316f
path.lineTo(SkBits2Float(0x41afe55d), SkBits2Float(0x4275978d));  // 21.987f, 61.398f
path.cubicTo(SkBits2Float(0x41b27cea), SkBits2Float(0x4275e147), SkBits2Float(0x41b54dd0), SkBits2Float(0x4275d916), SkBits2Float(0x41b772ad), SkBits2Float(0x42758106));  // 22.311f, 61.47f, 22.663f, 61.462f, 22.931f, 61.376f
path.cubicTo(SkBits2Float(0x41b8df38), SkBits2Float(0x42753d70), SkBits2Float(0x41ba1684), SkBits2Float(0x4274d1eb), SkBits2Float(0x41bb4186), SkBits2Float(0x42746979));  // 23.109f, 61.31f, 23.261f, 61.205f, 23.407f, 61.103f
path.lineTo(SkBits2Float(0x41bdbc67), SkBits2Float(0x4273a1cb));  // 23.717f, 60.908f
path.cubicTo(SkBits2Float(0x41c0f1a6), SkBits2Float(0x4272cccd), SkBits2Float(0x41c3cabd), SkBits2Float(0x4272b958), SkBits2Float(0x41c71684), SkBits2Float(0x4272a3d7));  // 24.118f, 60.7f, 24.474f, 60.681f, 24.886f, 60.66f
path.lineTo(SkBits2Float(0x41ca4392), SkBits2Float(0x42728831));  // 25.283f, 60.633f
path.lineTo(SkBits2Float(0x41def9d8), SkBits2Float(0x42723f7d));  // 27.872f, 60.562f
path.cubicTo(SkBits2Float(0x41e15a1a), SkBits2Float(0x42722d0e), SkBits2Float(0x41e4105f), SkBits2Float(0x42723333), SkBits2Float(0x41e60e53), SkBits2Float(0x4271c7ae));  // 28.169f, 60.544f, 28.508f, 60.55f, 28.757f, 60.445f
path.cubicTo(SkBits2Float(0x41e87ceb), SkBits2Float(0x42715810), SkBits2Float(0x41e97ef7), SkBits2Float(0x427077cf), SkBits2Float(0x41ea9165), SkBits2Float(0x426f8a3d));  // 29.061f, 60.336f, 29.187f, 60.117f, 29.321f, 59.885f
path.lineTo(SkBits2Float(0x41ebccc9), SkBits2Float(0x426e8a3d));  // 29.475f, 59.635f
path.cubicTo(SkBits2Float(0x41ebced5), SkBits2Float(0x426e8937), SkBits2Float(0x41ec2d0b), SkBits2Float(0x426e4ccc), SkBits2Float(0x41ec2d0b), SkBits2Float(0x426e4ccc));  // 29.476f, 59.634f, 29.522f, 59.575f, 29.522f, 59.575f
path.lineTo(SkBits2Float(0x41ecae11), SkBits2Float(0x426dde34));  // 29.585f, 59.467f
path.lineTo(SkBits2Float(0x41ecdf38), SkBits2Float(0x426dde34));  // 29.609f, 59.467f
path.lineTo(SkBits2Float(0x41ed26e6), SkBits2Float(0x426dc082));  // 29.644f, 59.438f
path.cubicTo(SkBits2Float(0x41ee1ca9), SkBits2Float(0x426d5a1c), SkBits2Float(0x41eeccc9), SkBits2Float(0x426d1061), SkBits2Float(0x41f01684), SkBits2Float(0x426ce978));  // 29.764f, 59.338f, 29.85f, 59.266f, 30.011f, 59.228f
path.cubicTo(SkBits2Float(0x41f29fbb), SkBits2Float(0x426c8e55), SkBits2Float(0x420cced8), SkBits2Float(0x426bd4fd), SkBits2Float(0x420e6c8a), SkBits2Float(0x426bdf3b));  // 30.328f, 59.139f, 35.202f, 58.958f, 35.606f, 58.968f
path.moveTo(SkBits2Float(0x41b60622), SkBits2Float(0x427adb22));  // 22.753f, 62.714f
path.lineTo(SkBits2Float(0x41b60416), SkBits2Float(0x427ad709));  // 22.752f, 62.71f
path.cubicTo(SkBits2Float(0x41b60416), SkBits2Float(0x427ad603), SkBits2Float(0x41b60416), SkBits2Float(0x427ad915), SkBits2Float(0x41b60622), SkBits2Float(0x427adb22));  // 22.752f, 62.709f, 22.752f, 62.712f, 22.753f, 62.714f
path.moveTo(SkBits2Float(0x41bed2ef), SkBits2Float(0x4274cbc6));  // 23.853f, 61.199f
path.close();
path.moveTo(SkBits2Float(0x41c04fdd), SkBits2Float(0x42746560));  // 24.039f, 61.099f
path.close();
}

static void joel_16(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    make_joel_16(path);
testSimplify(reporter, path, filename);
}

static void joel_16x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    make_joel_16(path);
testSimplify(reporter, path, filename);
}

static void coincubics(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.moveTo(SkDoubleToScalar(0.00000000000000000), SkDoubleToScalar(0.00000000000000000));
    path.cubicTo(SkDoubleToScalar(0.00022939755581319332), SkDoubleToScalar(0.00022927834652364254),
        SkDoubleToScalar(0.00022930106206331402), SkDoubleToScalar(0.00022929999977350235),
        SkDoubleToScalar(0.00022930069826543331), SkDoubleToScalar(0.00022913678549230099));
    path.lineTo(SkDoubleToScalar(0.00022930069826543331), SkDoubleToScalar(0.00022930069826543331));
    path.cubicTo(SkDoubleToScalar(0.00011465034913271666), SkDoubleToScalar(0.00011465034913271666),
            SkDoubleToScalar(0.00011465061106719077), SkDoubleToScalar(0.00011460937093943357),
            SkDoubleToScalar(0.00014331332931760699), SkDoubleToScalar(0.00014325146912597120));
testSimplify(reporter, path, filename);
}

static void grshapearc(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
path.setFillType(SkPathFillType::kWinding);
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
testSimplifyFail(reporter, path, filename);
}

static void bug8249(skiatest::Reporter* reporter, const char* filename) {
SkPath path;
path.setFillType(SkPathFillType::kWinding);
path.moveTo(SkBits2Float(0x43310000), SkBits2Float(0x43810000));  // 177, 258
path.lineTo(SkBits2Float(0x43480000), SkBits2Float(0x43868000));  // 200, 269
path.cubicTo(SkBits2Float(0x43480000), SkBits2Float(0x43b20000), SkBits2Float(0x437a0000), SkBits2Float(0x43cd0000), SkBits2Float(0x43c80000), SkBits2Float(0x43cd0000));  // 200, 356, 250, 410, 400, 410
path.cubicTo(SkBits2Float(0x44098000), SkBits2Float(0x43cd0000), SkBits2Float(0x44160000), SkBits2Float(0x43b20000), SkBits2Float(0x44160000), SkBits2Float(0x43868000));  // 550, 410, 600, 356, 600, 269
path.lineTo(SkBits2Float(0x44160000), SkBits2Float(0x43808000));  // 600, 257
path.cubicTo(SkBits2Float(0x44160000), SkBits2Float(0x43330000), SkBits2Float(0x44110000), SkBits2Float(0x429c0000), SkBits2Float(0x43cd0000), SkBits2Float(0x429c0000));  // 600, 179, 580, 78, 410, 78
path.cubicTo(SkBits2Float(0x43700000), SkBits2Float(0x429c0000), SkBits2Float(0x43480000), SkBits2Float(0x431f0000), SkBits2Float(0x43480000), SkBits2Float(0x438a8000));  // 240, 78, 200, 159, 200, 277
path.lineTo(SkBits2Float(0x43480000), SkBits2Float(0x4401c000));  // 200, 519
path.cubicTo(SkBits2Float(0x43480000), SkBits2Float(0x441f0000), SkBits2Float(0x43660000), SkBits2Float(0x44340000), SkBits2Float(0x43c80000), SkBits2Float(0x44340000));  // 200, 636, 230, 720, 400, 720
path.cubicTo(SkBits2Float(0x4404c000), SkBits2Float(0x44340000), SkBits2Float(0x440d0000), SkBits2Float(0x442b8000), SkBits2Float(0x44118000), SkBits2Float(0x4416c000));  // 531, 720, 564, 686, 582, 603
path.lineTo(SkBits2Float(0x442cc000), SkBits2Float(0x441c8000));  // 691, 626
path.cubicTo(SkBits2Float(0x44260000), SkBits2Float(0x443d4000), SkBits2Float(0x44114000), SkBits2Float(0x444a8000), SkBits2Float(0x43c88000), SkBits2Float(0x444a8000));  // 664, 757, 581, 810, 401, 810
path.cubicTo(SkBits2Float(0x43350000), SkBits2Float(0x444a8000), SkBits2Float(0x42c80000), SkBits2Float(0x442e0000), SkBits2Float(0x42c80000), SkBits2Float(0x4401c000));  // 181, 810, 100, 696, 100, 519
path.lineTo(SkBits2Float(0x42c80000), SkBits2Float(0x438a8000));  // 100, 277
path.cubicTo(SkBits2Float(0x42c80000), SkBits2Float(0x42cc0000), SkBits2Float(0x433e0000), SkBits2Float(0xc1200000), SkBits2Float(0x43cd0000), SkBits2Float(0xc1200000));  // 100, 102, 190, -10, 410, -10
path.cubicTo(SkBits2Float(0x441d8000), SkBits2Float(0xc1200000), SkBits2Float(0x442f0000), SkBits2Float(0x42e60000), SkBits2Float(0x442f0000), SkBits2Float(0x437a0000));  // 630, -10, 700, 115, 700, 250
path.lineTo(SkBits2Float(0x442f0000), SkBits2Float(0x43880000));  // 700, 272
path.cubicTo(SkBits2Float(0x442f0000), SkBits2Float(0x43d18000), SkBits2Float(0x44164000), SkBits2Float(0x43fa0000), SkBits2Float(0x43c88000), SkBits2Float(0x43fa0000));  // 700, 419, 601, 500, 401, 500
path.cubicTo(SkBits2Float(0x43490000), SkBits2Float(0x43fa0000), SkBits2Float(0x43160000), SkBits2Float(0x43d00000), SkBits2Float(0x43160000), SkBits2Float(0x43868000));  // 201, 500, 150, 416, 150, 269
path.lineTo(SkBits2Float(0x43310000), SkBits2Float(0x43810000));  // 177, 258
path.close();
testSimplify(reporter, path, filename);
}

static void bug8290(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.moveTo(-1e+09, -1e+09);
    path.lineTo(1e+09, -1e+09);
    path.lineTo(1e+09, 1e+09);
    path.lineTo(-1e+09, 1e+09);
    path.lineTo(-1e+09, -1e+09);
    path.close();
    path.moveTo(0, 45);
    path.lineTo(270, 45);
    path.lineTo(270, 45.381f);
    path.lineTo(0, 45.381f);
    path.lineTo(0, 45);
    path.close();
    path.moveTo(0, 90.381f);
    path.lineTo(270, 90.381f);
    path.lineTo(270, 90.7619f);
    path.lineTo(0, 90.7619f);
    path.lineTo(0, 90.381f);
    path.close();
    path.moveTo(0, 135.762f);
    path.lineTo(270, 135.762f);
    path.lineTo(270, 136.143f);
    path.lineTo(0, 136.143f);
    path.lineTo(0, 135.762f);
    path.close();
    path.moveTo(0, 181.143f);
    path.lineTo(270, 181.143f);
    path.lineTo(270, 181.524f);
    path.lineTo(0, 181.524f);
    path.lineTo(0, 181.143f);
    path.close();
    path.moveTo(0, 226.524f);
    path.lineTo(270, 226.524f);
    path.lineTo(270, 226.905f);
    path.lineTo(0, 226.905f);
    path.lineTo(0, 226.524f);
    path.close();
    path.moveTo(0, 271.905f);
    path.lineTo(270, 271.905f);
    path.lineTo(270, 272.286f);
    path.lineTo(0, 272.286f);
    path.lineTo(0, 271.905f);
    path.close();
    path.moveTo(0, 317.286f);
    path.lineTo(270, 317.286f);
    path.lineTo(270, 317.667f);
    path.lineTo(0, 317.667f);
    path.lineTo(0, 317.286f);
    path.close();
    SkMatrix matrix = SkMatrix::MakeAll(
            2.625,	0,	186,
            0,	2.625,	620,
            0,	0,	1);
    path.transform(matrix);
    testSimplify(reporter, path, filename);
}

static void bug11958_a(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPathFillType::kWinding);
    path.moveTo(SkBits2Float(0x44099d81), SkBits2Float(0x00000000));  // 550.461f, 0
    path.lineTo(SkBits2Float(0x44099d81), SkBits2Float(0x43b7276d));  // 550.461f, 366.308f
    path.lineTo(SkBits2Float(0x44324ea8), SkBits2Float(0x43b7276d));  // 713.229f, 366.308f
    path.lineTo(SkBits2Float(0x44324ea8), SkBits2Float(0x00000000));  // 713.229f, 0
    path.lineTo(SkBits2Float(0x44099d81), SkBits2Float(0x00000000));  // 550.461f, 0
    path.close();
    path.moveTo(SkBits2Float(0x440f9d71), SkBits2Float(0x00000000));  // 574.46f, 0
    path.lineTo(SkBits2Float(0x440f9d71), SkBits2Float(0x438a1d91));  // 574.46f, 276.231f
    path.lineTo(SkBits2Float(0x44387127), SkBits2Float(0x438a1d91));  // 737.768f, 276.231f
    path.quadTo(SkBits2Float(0x444d04cd), SkBits2Float(0x438a1d91), SkBits2Float(0x4456f396), SkBits2Float(0x4372a76c));  // 820.075f, 276.231f, 859.806f, 242.654f
    path.quadTo(SkBits2Float(0x4460e25e), SkBits2Float(0x435113b6), SkBits2Float(0x4460e25e), SkBits2Float(0x4310276d));  // 899.537f, 209.077f, 899.537f, 144.154f
    path.quadTo(SkBits2Float(0x4460e25e), SkBits2Float(0x429e0000), SkBits2Float(0x44555d70), SkBits2Float(0x421e0000));  // 899.537f, 79, 853.46f, 39.5f
    path.quadTo(SkBits2Float(0x4449d883), SkBits2Float(0x00000000), SkBits2Float(0x44321883), SkBits2Float(0x00000000));  // 807.383f, 0, 712.383f, 0
    path.lineTo(SkBits2Float(0x440f9d71), SkBits2Float(0x00000000));  // 574.46f, 0
    path.close();

    // TODO(skbug:11958) - This should not fail to simplify
    testSimplifyFail(reporter, path, filename);
}

static void bug11958_b(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;

    path.setFillType(SkPathFillType::kWinding);
    path.moveTo(SkBits2Float(0x42240000), SkBits2Float(0x43420000));  // 41, 194
    path.quadTo(SkBits2Float(0x42240000), SkBits2Float(0x43928000), SkBits2Float(0x42930000), SkBits2Float(0x43b38000));  // 41, 293, 73.5f, 359
    path.quadTo(SkBits2Float(0x42d40000), SkBits2Float(0x43d48000), SkBits2Float(0x43240000), SkBits2Float(0x43e58000));  // 106, 425, 164, 459
    path.quadTo(SkBits2Float(0x435e0000), SkBits2Float(0x43f68000), SkBits2Float(0x43958000), SkBits2Float(0x43f68000));  // 222, 493, 299, 493
    path.quadTo(SkBits2Float(0x43ab0000), SkBits2Float(0x43f68000), SkBits2Float(0x43bd0000), SkBits2Float(0x43f2c000));  // 342, 493, 378, 485.5f
    path.quadTo(SkBits2Float(0x43cf0000), SkBits2Float(0x43ef0000), SkBits2Float(0x43df8000), SkBits2Float(0x43e80000));  // 414, 478, 447, 464
    path.quadTo(SkBits2Float(0x43f00000), SkBits2Float(0x43e10000), SkBits2Float(0x43ff8000), SkBits2Float(0x43d70000));  // 480, 450, 511, 430
    path.lineTo(SkBits2Float(0x43f78000), SkBits2Float(0x43cc0000));  // 495, 408
    path.quadTo(SkBits2Float(0x43e90000), SkBits2Float(0x43d58000), SkBits2Float(0x43d9c000), SkBits2Float(0x43dc4000));  // 466, 427, 435.5f, 440.5f
    path.quadTo(SkBits2Float(0x43ca8000), SkBits2Float(0x43e30000), SkBits2Float(0x43b9c000), SkBits2Float(0x43e68000));  // 405, 454, 371.5f, 461
    path.quadTo(SkBits2Float(0x43a90000), SkBits2Float(0x43ea0000), SkBits2Float(0x43958000), SkBits2Float(0x43ea0000));  // 338, 468, 299, 468
    path.quadTo(SkBits2Float(0x43650000), SkBits2Float(0x43ea0000), SkBits2Float(0x43308000), SkBits2Float(0x43da4000));  // 229, 468, 176.5f, 436.5f
    path.quadTo(SkBits2Float(0x42f80000), SkBits2Float(0x43ca8000), SkBits2Float(0x42c00000), SkBits2Float(0x43ac0000));  // 124, 405, 96, 344
    path.quadTo(SkBits2Float(0x42880000), SkBits2Float(0x438d8000), SkBits2Float(0x42880000), SkBits2Float(0x43420000));  // 68, 283, 68, 194
    path.lineTo(SkBits2Float(0x42240000), SkBits2Float(0x43420000));  // 41, 194
    path.close();
    path.moveTo(SkBits2Float(0x43ddd958), SkBits2Float(0x440e8000));  // 443.698f, 570
    path.quadTo(SkBits2Float(0x43ddd958), SkBits2Float(0x44094000), SkBits2Float(0x43da5958), SkBits2Float(0x4404c000));  // 443.698f, 549, 436.698f, 531
    path.quadTo(SkBits2Float(0x43d6d958), SkBits2Float(0x44004000), SkBits2Float(0x43cfd958), SkBits2Float(0x43f98000));  // 429.698f, 513, 415.698f, 499
    path.quadTo(SkBits2Float(0x43c75958), SkBits2Float(0x43f18000), SkBits2Float(0x43ba9958), SkBits2Float(0x43ee0000));  // 398.698f, 483, 373.198f, 476
    path.quadTo(SkBits2Float(0x43add958), SkBits2Float(0x43ea8000), SkBits2Float(0x4396d958), SkBits2Float(0x43ea8000));  // 347.698f, 469, 301.698f, 469
    path.lineTo(SkBits2Float(0x436cb2b0), SkBits2Float(0x43ea8000));  // 236.698f, 469
    path.lineTo(SkBits2Float(0x436cb2b0), SkBits2Float(0x43f68000));  // 236.698f, 493
    path.lineTo(SkBits2Float(0x43955958), SkBits2Float(0x43f68000));  // 298.698f, 493
    path.quadTo(SkBits2Float(0x43a8d958), SkBits2Float(0x43f68000), SkBits2Float(0x43b3d958), SkBits2Float(0x43f90000));  // 337.698f, 493, 359.698f, 498
    path.quadTo(SkBits2Float(0x43bed958), SkBits2Float(0x43fb8000), SkBits2Float(0x43c55958), SkBits2Float(0x4400c000));  // 381.698f, 503, 394.698f, 515
    path.quadTo(SkBits2Float(0x43cb5958), SkBits2Float(0x44030000), SkBits2Float(0x43cdd958), SkBits2Float(0x4406a000));  // 406.698f, 524, 411.698f, 538.5f
    path.quadTo(SkBits2Float(0x43d05958), SkBits2Float(0x440a4000), SkBits2Float(0x43d05958), SkBits2Float(0x440e8000));  // 416.698f, 553, 416.698f, 570
    path.lineTo(SkBits2Float(0x43ddd958), SkBits2Float(0x440e8000));  // 443.698f, 570
    path.close();

    testSimplify(reporter, path, filename);
}

static void bug11958_c(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;

    path.setFillType(SkPathFillType::kWinding);
    path.moveTo(200.f, 200.f);
    path.lineTo(164.f, 459.f);
    path.quadTo(222.f, 493.f, 299.f, 493.f);
    path.quadTo(342.f, 493.f, 378.f, 485.f);
    path.close();

    path.moveTo(415.698f, 499.f);
    path.lineTo(236.698f, 469.f);
    path.lineTo(236.698f, 493.f);
    path.lineTo(298.698f, 493.f);
    path.quadTo(337.698f, 493.f, 359.698f, 498.f);
    path.close();

    testSimplify(reporter, path, filename);
}

static void (*skipTest)(skiatest::Reporter* , const char* filename) = nullptr;
static void (*firstTest)(skiatest::Reporter* , const char* filename) = nullptr;
static void (*stopTest)(skiatest::Reporter* , const char* filename) = nullptr;

static TestDesc tests[] = {
    TEST(bug8290),
    TEST(bug8249),
    TEST(bug11958_a),
    TEST(bug11958_b),
    TEST(bug11958_c),
    TEST(grshapearc),
    TEST(coincubics),
    TEST(joel_16x),
    TEST(joel_16),
    TEST(joel_15x),
    TEST(joel_15),
    TEST(joel_14x),
    TEST(joel_14),
    TEST(joel_13x),
    TEST(joel_13),
    TEST(joel_12x),
    TEST(joel_12),
    TEST(joel_11),
    TEST(joel_10),
    TEST(joel_9),
    TEST(joel_8),
    TEST(joel_7),
    TEST(joel_6),
    TEST(joel_5),
    TEST(joel_4),
    TEST(joel_3),
    TEST(joel_2),
    TEST(joel_1),
    TEST(simplifyTest_1),
    TEST(carsvg_1),
    TEST(tiger8_393),
    TEST(bug5169),
    TEST(testQuads73),
    TEST(testQuads72),
    TEST(testQuads71),
    TEST(testQuads70),
    TEST(testQuads69),
    TEST(testQuads68),
    TEST(testQuads67),
    TEST(testQuads66),
    TEST(dean4),
    TEST(fuzz763_4713_b),
    TEST(fuzz_twister2),
    TEST(fuzz_twister),
    TEST(fuzz994s_3414),
    TEST(fuzz994s_11),
    TEST(cr514118),
    TEST(fuzz864a),
    TEST(testQuads65),
    TEST(testIssue3838_3),
    TEST(testIssue3838),
    TEST(testArc),
    TEST(testTriangle2),
    TEST(testTriangle1),
    TEST(testQuads64),
    TEST(testQuads63),
    TEST(testQuads62),
    TEST(testRect4),
    TEST(testRect3),
    TEST(testQuadralateral10),
    TEST(testQuads61),
    TEST(testQuads60),
    TEST(testQuads59),
    TEST(testQuads58),
    TEST(testQuads57),
    TEST(testQuads56),
    TEST(testQuads54),
    TEST(testQuads53),
    TEST(testQuads52),
    TEST(testQuads51),
    TEST(testQuads50),
    TEST(testQuads49),
    TEST(testQuads48),
    TEST(testQuads47),
    TEST(testQuads46x),
    TEST(testQuads45),
    TEST(testQuads44),
    TEST(testQuads43),
    TEST(testQuads42),
    TEST(testQuads41),
    TEST(testQuads36),
    TEST(testQuads37),
    TEST(testQuads38),
    TEST(testQuads39),
    TEST(testQuads40),
    TEST(testQuads16),
    TEST(testQuads17),
    TEST(testQuads18),
    TEST(testQuads19),
    TEST(testQuads20),
    TEST(testQuads21),
    TEST(testQuads22),
    TEST(testQuads23),
    TEST(testQuads24),
    TEST(testQuads25),
    TEST(testQuads26),
    TEST(testQuads27),
    TEST(testQuads28),
    TEST(testQuads29),
    TEST(testQuads30),
    TEST(testQuads31),
    TEST(testQuads32),
    TEST(testQuads33),
    TEST(testQuads34),
    TEST(testQuads35),
    TEST(testDegenerates1),
    TEST(testQuad13),
    TEST(testQuad14),
    TEST(testQuad15),
    TEST(testQuadratic56),
    TEST(testQuadralateral4),
    TEST(testQuadralateral3),
    TEST(testDegenerate5),
    TEST(testQuad12),
    TEST(testQuadratic51),
    TEST(testQuad8),
    TEST(testQuad11),
    TEST(testQuad10),
    TEST(testQuad9),
    TEST(testTriangles4x),
    TEST(testTriangles3x),
    TEST(testRect2s),
    TEST(testRect1s),
    TEST(tooCloseTest),
    TEST(skphealth_com76s),
    TEST(testQuadLineIntersect1),
    TEST(testQuadLineIntersect2),
    TEST(testQuadLineIntersect3),
    TEST(testQuad7),
    TEST(testQuad6),
    TEST(testQuad5),
    TEST(testQuad4),
    TEST(testQuad3),
    TEST(testQuad2),
    TEST(testAddTCoincident2),
    TEST(testAddTCoincident1),
    TEST(testTriangles2),
    TEST(testTriangles1),
    TEST(testQuadratic97),
    TEST(testQuadratic96),
    TEST(testQuadratic95),
    TEST(testQuadratic94),
    TEST(testQuadralateral2),
    TEST(testQuad1),
    TEST(testCubic2),
    TEST(testCubic1),
    TEST(testQuadralateral1),
    TEST(testLine85),
    TEST(testLine84),
    TEST(testLine84x),
    TEST(testLine83),
    TEST(testLine82h),
    TEST(testLine82g),
    TEST(testLine82f),
    TEST(testLine82e),
    TEST(testLine82d),
    TEST(testLine82c),
    TEST(testLine82b),
    TEST(testLine82a),
    TEST(testLine82),
    TEST(testQuadratic93),
    TEST(testQuadratic92x),
    TEST(testQuadratic91),
    TEST(testQuadratic90x),
    TEST(testQuadratic89x),
    TEST(testQuadratic88),
    TEST(testQuadratic87),
    TEST(testQuadratic86),
    TEST(testQuadratic85),
    TEST(testQuadratic84),
    TEST(testQuadratic83),
    TEST(testQuadratic82),
    TEST(testQuadratic81),
    TEST(testQuadratic80),
    TEST(testEight1),
    TEST(testEight2),
    TEST(testEight3),
    TEST(testEight4),
    TEST(testEight5),
    TEST(testEight6),
    TEST(testEight7),
    TEST(testEight8),
    TEST(testEight9),
    TEST(testEight10),
    TEST(testQuadratic79),
    TEST(testQuadratic78),
    TEST(testQuadratic77),
    TEST(testQuadratic76),
    TEST(testQuadratic75),
    TEST(testQuadratic74),
    TEST(testQuadratic73),
    TEST(testQuadratic72),
    TEST(testQuadratic71),
    TEST(testQuadratic70x),
    TEST(testQuadratic69),
    TEST(testQuadratic68),
    TEST(testQuadratic67x),
    TEST(testQuadratic65),
    TEST(testQuadratic64),
    TEST(testQuadratic63),
    TEST(testLine1a),
    TEST(testQuadratic59),
    TEST(testQuadratic59x),
    TEST(testQuadratic58),
    TEST(testQuadratic55),
    TEST(testQuadratic53),
    TEST(testQuadratic38),
    TEST(testQuadratic37),
    TEST(testQuadratic36),
    TEST(testQuadratic35),
    TEST(testQuadratic34),
    TEST(testQuadratic33),
    TEST(testQuadratic32),
    TEST(testQuadratic31),
    TEST(testQuadratic30),
    TEST(testQuadratic29),
    TEST(testQuadratic28),
    TEST(testQuadratic27),
    TEST(testQuadratic26),
    TEST(testQuadratic25),
    TEST(testQuadratic24),
    TEST(testQuadratic23),
    TEST(testQuadratic22),
    TEST(testQuadratic21),
    TEST(testQuadratic20),
    TEST(testQuadratic19),
    TEST(testQuadratic18),
    TEST(testQuadratic17x),
    TEST(testQuadratic15),
    TEST(testQuadratic14),
    TEST(testQuadratic9),
    TEST(testQuadratic8),
    TEST(testQuadratic7),
    TEST(testQuadratic6),
    TEST(testQuadratic5),
    TEST(testQuadratic4x),
    TEST(testQuadratic3x),
    TEST(testQuadratic2x),
    TEST(testQuadratic1x),
    TEST(testQuadratic4),
    TEST(testQuadratic3),
    TEST(testQuadratic2),
    TEST(testQuadratic1),
    TEST(testLine4ax),
    TEST(testLine3aax),
    TEST(testLine2ax),
    TEST(testLine1ax),
    TEST(testQuadralateral9x),
    TEST(testQuadralateral8x),
    TEST(testQuadralateral7x),
    TEST(testQuadralateral6x),
    TEST(testQuadralateral6ax),
    TEST(testQuadralateral9),
    TEST(testQuadralateral8),
    TEST(testQuadralateral7),
    TEST(testQuadralateral6),
    TEST(testQuadralateral6a),
    TEST(testFauxQuadralateral6dx),
    TEST(testFauxQuadralateral6cx),
    TEST(testFauxQuadralateral6bx),
    TEST(testFauxQuadralateral6ax),
    TEST(testFauxQuadralateral6x),
    TEST(testFauxQuadralateral6d),
    TEST(testFauxQuadralateral6c),
    TEST(testFauxQuadralateral6b),
    TEST(testFauxQuadralateral6a),
    TEST(testFauxQuadralateral6),
    TEST(testQuadralateral5x),
    TEST(testQuadralateral5),
    TEST(testNondegenerate4x),
    TEST(testNondegenerate3x),
    TEST(testNondegenerate2x),
    TEST(testNondegenerate1x),
    TEST(testNondegenerate4),
    TEST(testNondegenerate3),
    TEST(testNondegenerate2),
    TEST(testNondegenerate1),
    TEST(testDegenerate4x),
    TEST(testDegenerate3x),
    TEST(testDegenerate2x),
    TEST(testDegenerate1x),
    TEST(testDegenerate4),
    TEST(testDegenerate3),
    TEST(testDegenerate2),
    TEST(testDegenerate1),
    TEST(testLine79x),
    TEST(testLine78x),
    TEST(testLine77x),
    TEST(testLine76x),
    TEST(testLine75x),
    TEST(testLine74x),
    TEST(testLine73x),
    TEST(testLine72x),
    TEST(testLine71x),
    TEST(testLine70x),
    TEST(testLine69x),
    TEST(testLine68hx),
    TEST(testLine68gx),
    TEST(testLine68fx),
    TEST(testLine68ex),
    TEST(testLine68dx),
    TEST(testLine68cx),
    TEST(testLine68bx),
    TEST(testLine68ax),
    TEST(testLine67x),
    TEST(testLine66x),
    TEST(testLine65x),
    TEST(testLine64x),
    TEST(testLine63x),
    TEST(testLine62x),
    TEST(testLine61x),
    TEST(testLine60x),
    TEST(testLine59x),
    TEST(testLine58x),
    TEST(testLine57x),
    TEST(testLine56x),
    TEST(testLine55x),
    TEST(testLine54x),
    TEST(testLine53x),
    TEST(testLine52x),
    TEST(testLine51x),
    TEST(testLine50x),
    TEST(testLine49x),
    TEST(testLine48x),
    TEST(testLine47x),
    TEST(testLine46x),
    TEST(testLine45x),
    TEST(testLine44x),
    TEST(testLine43x),
    TEST(testLine42x),
    TEST(testLine41x),
    TEST(testLine40x),
    TEST(testLine38x),
    TEST(testLine37x),
    TEST(testLine36x),
    TEST(testLine35x),
    TEST(testLine34x),
    TEST(testLine33x),
    TEST(testLine32x),
    TEST(testLine31x),
    TEST(testLine30x),
    TEST(testLine29x),
    TEST(testLine28x),
    TEST(testLine27x),
    TEST(testLine26x),
    TEST(testLine25x),
    TEST(testLine24ax),
    TEST(testLine24x),
    TEST(testLine23x),
    TEST(testLine22x),
    TEST(testLine21x),
    TEST(testLine20x),
    TEST(testLine19x),
    TEST(testLine18x),
    TEST(testLine17x),
    TEST(testLine16x),
    TEST(testLine15x),
    TEST(testLine14x),
    TEST(testLine13x),
    TEST(testLine12x),
    TEST(testLine11x),
    TEST(testLine10ax),
    TEST(testLine10x),
    TEST(testLine9x),
    TEST(testLine8x),
    TEST(testLine7bx),
    TEST(testLine7ax),
    TEST(testLine7x),
    TEST(testLine6x),
    TEST(testLine5x),
    TEST(testLine4x),
    TEST(testLine3bx),
    TEST(testLine3ax),
    TEST(testLine3x),
    TEST(testLine2x),
    TEST(testLine1x),
    TEST(testLine81),
    TEST(testLine80),
    TEST(testLine79),
    TEST(testLine78),
    TEST(testLine77),
    TEST(testLine76),
    TEST(testLine75),
    TEST(testLine74),
    TEST(testLine73),
    TEST(testLine72),
    TEST(testLine71),
    TEST(testLine70),
    TEST(testLine69),
    TEST(testLine68h),
    TEST(testLine68g),
    TEST(testLine68f),
    TEST(testLine68e),
    TEST(testLine68d),
    TEST(testLine68c),
    TEST(testLine68b),
    TEST(testLine68a),
    TEST(testLine67),
    TEST(testLine66),
    TEST(testLine65),
    TEST(testLine64),
    TEST(testLine63),
    TEST(testLine62),
    TEST(testLine61),
    TEST(testLine60),
    TEST(testLine59),
    TEST(testLine58),
    TEST(testLine57),
    TEST(testLine56),
    TEST(testLine55),
    TEST(testLine54),
    TEST(testLine53),
    TEST(testLine52),
    TEST(testLine51),
    TEST(testLine50),
    TEST(testLine49),
    TEST(testLine48),
    TEST(testLine47),
    TEST(testLine46),
    TEST(testLine45),
    TEST(testLine44),
    TEST(testLine43),
    TEST(testLine42),
    TEST(testLine41),
    TEST(testLine40),
    TEST(testLine38),
    TEST(testLine37),
    TEST(testLine36),
    TEST(testLine35),
    TEST(testLine34),
    TEST(testLine33),
    TEST(testLine32),
    TEST(testLine31),
    TEST(testLine30),
    TEST(testLine29),
    TEST(testLine28),
    TEST(testLine27),
    TEST(testLine26),
    TEST(testLine25),
    TEST(testLine24a),
    TEST(testLine24),
    TEST(testLine23),
    TEST(testLine22),
    TEST(testLine21),
    TEST(testLine20),
    TEST(testLine19),
    TEST(testLine18),
    TEST(testLine17),
    TEST(testLine16),
    TEST(testLine15),
    TEST(testLine14),
    TEST(testLine13),
    TEST(testLine12),
    TEST(testLine11),
    TEST(testLine10a),
    TEST(testLine10),
    TEST(testLine9),
    TEST(testLine8),
    TEST(testLine7b),
    TEST(testLine7a),
    TEST(testLine7),
    TEST(testLine6),
    TEST(testLine5),
    TEST(testLine4),
    TEST(testLine3b),
    TEST(testLine3a),
    TEST(testLine3),
    TEST(testLine2),
    TEST(testLine1),
    TEST(testDegenerates),
};

static const size_t testCount = std::size(tests);

static TestDesc subTests[] = {
    TEST(fuzz994s_3414),
    TEST(fuzz994s_11),
};

static const size_t subTestCount = std::size(subTests);

static void (*firstSubTest)(skiatest::Reporter* , const char* filename) = nullptr;

static bool runSubTests = false;
static bool runSubTestsFirst = false;
static bool runReverse = false;

DEF_TEST(PathOpsSimplify, reporter) {
    if (runSubTests && runSubTestsFirst) {
        RunTestSet(reporter, subTests, subTestCount, firstSubTest, nullptr, stopTest, runReverse);
    }
    RunTestSet(reporter, tests, testCount, firstTest, skipTest, stopTest, runReverse);
    if (runSubTests && !runSubTestsFirst) {
        RunTestSet(reporter, subTests, subTestCount, firstSubTest, nullptr, stopTest, runReverse);
    }
}
