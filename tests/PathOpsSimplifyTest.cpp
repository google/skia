/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "PathOpsExtendedTest.h"

#define TEST(name) { name, #name }

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
    path.setFillType(SkPath::kEvenOdd_FillType);
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
    path.setFillType(SkPath::kEvenOdd_FillType);
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
    path.setFillType(SkPath::kEvenOdd_FillType);
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
    path.setFillType(SkPath::kEvenOdd_FillType);
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
    path.setFillType(SkPath::kEvenOdd_FillType);
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
    path.setFillType(SkPath::kEvenOdd_FillType);
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
    path.setFillType(SkPath::kEvenOdd_FillType);
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
    path.setFillType(SkPath::kEvenOdd_FillType);
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
    path.setFillType(SkPath::kEvenOdd_FillType);
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
    path.setFillType(SkPath::kEvenOdd_FillType);
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
    path.setFillType(SkPath::kEvenOdd_FillType);
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
    path.setFillType(SkPath::kEvenOdd_FillType);
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
    path.setFillType(SkPath::kEvenOdd_FillType);
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
    path.setFillType(SkPath::kEvenOdd_FillType);
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
    path.setFillType(SkPath::kEvenOdd_FillType);
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
    path.setFillType(SkPath::kEvenOdd_FillType);
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
    path.setFillType(SkPath::kEvenOdd_FillType);
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
    path.setFillType(SkPath::kEvenOdd_FillType);
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
    path.setFillType(SkPath::kEvenOdd_FillType);
    addCCWContainer(path);
    addCCWContents(path);
    testSimplify(reporter, path, filename);
}

static void testLine15(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 9, 9, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine15x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 9, 9, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine16(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 12, 12, SkPath::kCW_Direction);
    path.addRect(0, 4, 9, 9, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine16x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 12, 12, SkPath::kCW_Direction);
    path.addRect(0, 4, 9, 9, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine17(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 12, 12, SkPath::kCW_Direction);
    path.addRect(4, 12, 13, 13, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine17x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 12, 12, SkPath::kCW_Direction);
    path.addRect(4, 12, 13, 13, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine18(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 12, 12, SkPath::kCW_Direction);
    path.addRect(12, 4, 21, 21, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine18x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 12, 12, SkPath::kCW_Direction);
    path.addRect(12, 4, 21, 21, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine19(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 12, 12, SkPath::kCW_Direction);
    path.addRect(12, 16, 21, 21, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine19x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 12, 12, SkPath::kCW_Direction);
    path.addRect(12, 16, 21, 21, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine20(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 12, 12, 12, SkPath::kCW_Direction);
    path.addRect(0, 12, 9, 9, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine20x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 12, 12, 12, SkPath::kCW_Direction);
    path.addRect(0, 12, 9, 9, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine21(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 12, 12, 12, SkPath::kCW_Direction);
    path.addRect(0, 16, 9, 9, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine21x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 12, 12, 12, SkPath::kCW_Direction);
    path.addRect(0, 16, 9, 9, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine22(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 12, 12, 12, SkPath::kCW_Direction);
    path.addRect(4, 12, 13, 13, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine22x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 12, 12, 12, SkPath::kCW_Direction);
    path.addRect(4, 12, 13, 13, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine23(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 12, 12, 12, SkPath::kCW_Direction);
    path.addRect(12, 0, 21, 21, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine23x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 12, 12, 12, SkPath::kCW_Direction);
    path.addRect(12, 0, 21, 21, SkPath::kCW_Direction);
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
    path.setFillType(SkPath::kEvenOdd_FillType);
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
    path.addRect(0, 18, 12, 12, SkPath::kCW_Direction);
    path.addRect(4, 12, 13, 13, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine24x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 18, 12, 12, SkPath::kCW_Direction);
    path.addRect(4, 12, 13, 13, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine25(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 6, 12, 12, SkPath::kCW_Direction);
    path.addRect(12, 0, 21, 21, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine25x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 6, 12, 12, SkPath::kCW_Direction);
    path.addRect(12, 0, 21, 21, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine26(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 18, 12, 12, SkPath::kCW_Direction);
    path.addRect(0, 12, 9, 9, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine26x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 18, 12, 12, SkPath::kCW_Direction);
    path.addRect(0, 12, 9, 9, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine27(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 18, 12, 12, SkPath::kCW_Direction);
    path.addRect(12, 8, 21, 21, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine27x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 18, 12, 12, SkPath::kCW_Direction);
    path.addRect(12, 8, 21, 21, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine28(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 6, 12, 12, SkPath::kCW_Direction);
    path.addRect(0, 0, 9, 9, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine28x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 6, 12, 12, SkPath::kCW_Direction);
    path.addRect(0, 0, 9, 9, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine29(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 18, 12, 12, SkPath::kCW_Direction);
    path.addRect(12, 12, 21, 21, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine29x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 18, 12, 12, SkPath::kCW_Direction);
    path.addRect(12, 12, 21, 21, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine30(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(0, 0, 12, 12, SkPath::kCW_Direction);
    path.addRect(4, 4, 13, 13, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine30x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(0, 0, 12, 12, SkPath::kCW_Direction);
    path.addRect(4, 4, 13, 13, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine31(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(0, 0, 12, 12, SkPath::kCW_Direction);
    path.addRect(0, 4, 9, 9, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine31x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(0, 0, 12, 12, SkPath::kCW_Direction);
    path.addRect(0, 4, 9, 9, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine32(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(0, 0, 12, 12, SkPath::kCW_Direction);
    path.addRect(4, 12, 13, 13, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine32x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(0, 0, 12, 12, SkPath::kCW_Direction);
    path.addRect(4, 12, 13, 13, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine33(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(0, 0, 12, 12, SkPath::kCW_Direction);
    path.addRect(4, 16, 13, 13, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine33x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(0, 0, 12, 12, SkPath::kCW_Direction);
    path.addRect(4, 16, 13, 13, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine34(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(0, 6, 12, 12, SkPath::kCW_Direction);
    path.addRect(4, 12, 13, 13, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine34x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(0, 6, 12, 12, SkPath::kCW_Direction);
    path.addRect(4, 12, 13, 13, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine35(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(6, 0, 18, 18, SkPath::kCW_Direction);
    path.addRect(4, 16, 13, 13, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine35x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(6, 0, 18, 18, SkPath::kCW_Direction);
    path.addRect(4, 16, 13, 13, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine36(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 10, 20, 20, SkPath::kCW_Direction);
    path.addRect(6, 12, 18, 18, SkPath::kCW_Direction);
    path.addRect(4, 16, 13, 13, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine36x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 10, 20, 20, SkPath::kCW_Direction);
    path.addRect(6, 12, 18, 18, SkPath::kCW_Direction);
    path.addRect(4, 16, 13, 13, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine37(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 20, 20, 20, SkPath::kCW_Direction);
    path.addRect(18, 24, 30, 30, SkPath::kCW_Direction);
    path.addRect(0, 0, 9, 9, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine37x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 20, 20, 20, SkPath::kCW_Direction);
    path.addRect(18, 24, 30, 30, SkPath::kCW_Direction);
    path.addRect(0, 0, 9, 9, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine38(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(10, 0, 30, 30, SkPath::kCW_Direction);
    path.addRect(6, 12, 18, 18, SkPath::kCW_Direction);
    path.addRect(12, 12, 21, 21, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine38x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(10, 0, 30, 30, SkPath::kCW_Direction);
    path.addRect(6, 12, 18, 18, SkPath::kCW_Direction);
    path.addRect(12, 12, 21, 21, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine40(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(10, 0, 30, 30, SkPath::kCW_Direction);
    path.addRect(12, 18, 24, 24, SkPath::kCW_Direction);
    path.addRect(4, 16, 13, 13, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine40x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(10, 0, 30, 30, SkPath::kCW_Direction);
    path.addRect(12, 18, 24, 24, SkPath::kCW_Direction);
    path.addRect(4, 16, 13, 13, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine41(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(18, 24, 30, 30, SkPath::kCW_Direction);
    path.addRect(12, 0, 21, 21, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine41x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(18, 24, 30, 30, SkPath::kCW_Direction);
    path.addRect(12, 0, 21, 21, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine42(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(0, 0, 12, 12, SkPath::kCW_Direction);
    path.addRect(8, 16, 17, 17, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine42x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(0, 0, 12, 12, SkPath::kCW_Direction);
    path.addRect(8, 16, 17, 17, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine43(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(6, 24, 18, 18, SkPath::kCW_Direction);
    path.addRect(0, 32, 9, 36, SkPath::kCCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine43x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(6, 24, 18, 18, SkPath::kCW_Direction);
    path.addRect(0, 32, 9, 36, SkPath::kCCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine44(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(10, 40, 30, 30, SkPath::kCW_Direction);
    path.addRect(18, 0, 30, 30, SkPath::kCW_Direction);
    path.addRect(18, 32, 27, 36, SkPath::kCCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine44x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(10, 40, 30, 30, SkPath::kCW_Direction);
    path.addRect(18, 0, 30, 30, SkPath::kCW_Direction);
    path.addRect(18, 32, 27, 36, SkPath::kCCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine45(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(10, 0, 30, 30, SkPath::kCW_Direction);
    path.addRect(18, 0, 30, 30, SkPath::kCW_Direction);
    path.addRect(24, 32, 33, 36, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine45x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(10, 0, 30, 30, SkPath::kCW_Direction);
    path.addRect(18, 0, 30, 30, SkPath::kCW_Direction);
    path.addRect(24, 32, 33, 36, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine46(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(10, 40, 30, 30, SkPath::kCW_Direction);
    path.addRect(24, 0, 36, 36, SkPath::kCW_Direction);
    path.addRect(24, 32, 33, 36, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine46x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(10, 40, 30, 30, SkPath::kCW_Direction);
    path.addRect(24, 0, 36, 36, SkPath::kCW_Direction);
    path.addRect(24, 32, 33, 36, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine47(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(0, 0, 12, 12, SkPath::kCW_Direction);
    path.addRect(0, 0, 9, 9, SkPath::kCCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine47x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(0, 0, 12, 12, SkPath::kCW_Direction);
    path.addRect(0, 0, 9, 9, SkPath::kCCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine48(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(0, 6, 12, 12, SkPath::kCW_Direction);
    path.addRect(0, 0, 9, 9, SkPath::kCCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine48x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(0, 6, 12, 12, SkPath::kCW_Direction);
    path.addRect(0, 0, 9, 9, SkPath::kCCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine49(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(0, 0, 12, 12, SkPath::kCW_Direction);
    path.addRect(0, 0, 9, 9, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine49x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(0, 0, 12, 12, SkPath::kCW_Direction);
    path.addRect(0, 0, 9, 9, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine50(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(10, 30, 30, 30, SkPath::kCW_Direction);
    path.addRect(24, 20, 36, 30, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine50x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(10, 30, 30, 30, SkPath::kCW_Direction);
    path.addRect(24, 20, 36, 30, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine51(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(0, 12, 12, 12, SkPath::kCW_Direction);
    path.addRect(4, 12, 13, 13, SkPath::kCCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine51x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(0, 12, 12, 12, SkPath::kCW_Direction);
    path.addRect(4, 12, 13, 13, SkPath::kCCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine52(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 30, 20, 20, SkPath::kCW_Direction);
    path.addRect(6, 20, 18, 30, SkPath::kCW_Direction);
    path.addRect(32, 0, 36, 41, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine52x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 30, 20, 20, SkPath::kCW_Direction);
    path.addRect(6, 20, 18, 30, SkPath::kCW_Direction);
    path.addRect(32, 0, 36, 41, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine53(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(10, 30, 30, 30, SkPath::kCW_Direction);
    path.addRect(12, 20, 24, 30, SkPath::kCW_Direction);
    path.addRect(12, 32, 21, 36, SkPath::kCCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine53x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(10, 30, 30, 30, SkPath::kCW_Direction);
    path.addRect(12, 20, 24, 30, SkPath::kCW_Direction);
    path.addRect(12, 32, 21, 36, SkPath::kCCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine54(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(6, 0, 18, 18, SkPath::kCW_Direction);
    path.addRect(8, 4, 17, 17, SkPath::kCCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine54x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(6, 0, 18, 18, SkPath::kCW_Direction);
    path.addRect(8, 4, 17, 17, SkPath::kCCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine55(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(6, 6, 18, 18, SkPath::kCW_Direction);
    path.addRect(4, 4, 13, 13, SkPath::kCCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine55x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(6, 6, 18, 18, SkPath::kCW_Direction);
    path.addRect(4, 4, 13, 13, SkPath::kCCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine56(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 20, 20, 20, SkPath::kCW_Direction);
    path.addRect(18, 20, 30, 30, SkPath::kCW_Direction);
    path.addRect(12, 0, 21, 21, SkPath::kCCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine56x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 20, 20, 20, SkPath::kCW_Direction);
    path.addRect(18, 20, 30, 30, SkPath::kCW_Direction);
    path.addRect(12, 0, 21, 21, SkPath::kCCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine57(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(20, 0, 40, 40, SkPath::kCW_Direction);
    path.addRect(20, 0, 30, 40, SkPath::kCW_Direction);
    path.addRect(12, 0, 21, 21, SkPath::kCCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine57x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(20, 0, 40, 40, SkPath::kCW_Direction);
    path.addRect(20, 0, 30, 40, SkPath::kCW_Direction);
    path.addRect(12, 0, 21, 21, SkPath::kCCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine58(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(0, 0, 12, 12, SkPath::kCCW_Direction);
    path.addRect(0, 12, 9, 9, SkPath::kCCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine58x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(0, 0, 12, 12, SkPath::kCCW_Direction);
    path.addRect(0, 12, 9, 9, SkPath::kCCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine59(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(6, 6, 18, 18, SkPath::kCCW_Direction);
    path.addRect(4, 4, 13, 13, SkPath::kCCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine59x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(6, 6, 18, 18, SkPath::kCCW_Direction);
    path.addRect(4, 4, 13, 13, SkPath::kCCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine60(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(6, 12, 18, 18, SkPath::kCCW_Direction);
    path.addRect(4, 12, 13, 13, SkPath::kCCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine60x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(6, 12, 18, 18, SkPath::kCCW_Direction);
    path.addRect(4, 12, 13, 13, SkPath::kCCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine61(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(12, 0, 24, 24, SkPath::kCCW_Direction);
    path.addRect(12, 0, 21, 21, SkPath::kCCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine61x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(12, 0, 24, 24, SkPath::kCCW_Direction);
    path.addRect(12, 0, 21, 21, SkPath::kCCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine62(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 60, 60, SkPath::kCW_Direction);
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(0, 12, 12, 12, SkPath::kCW_Direction);
    path.addRect(4, 12, 13, 13, SkPath::kCCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine62x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 60, 60, SkPath::kCW_Direction);
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(0, 12, 12, 12, SkPath::kCW_Direction);
    path.addRect(4, 12, 13, 13, SkPath::kCCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine63(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 60, 60, SkPath::kCW_Direction);
    path.addRect(0, 10, 20, 20, SkPath::kCW_Direction);
    path.addRect(0, 6, 12, 12, SkPath::kCCW_Direction);
    path.addRect(0, 32, 9, 36, SkPath::kCCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine63x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 60, 60, SkPath::kCW_Direction);
    path.addRect(0, 10, 20, 20, SkPath::kCW_Direction);
    path.addRect(0, 6, 12, 12, SkPath::kCCW_Direction);
    path.addRect(0, 32, 9, 36, SkPath::kCCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine64(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 60, 60, SkPath::kCW_Direction);
    path.addRect(10, 40, 30, 30, SkPath::kCW_Direction);
    path.addRect(18, 6, 30, 30, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine64x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 60, 60, SkPath::kCW_Direction);
    path.addRect(10, 40, 30, 30, SkPath::kCW_Direction);
    path.addRect(18, 6, 30, 30, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine65(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 60, 60, SkPath::kCW_Direction);
    path.addRect(10, 0, 30, 30, SkPath::kCW_Direction);
    path.addRect(24, 0, 36, 36, SkPath::kCW_Direction);
    path.addRect(32, 6, 36, 41, SkPath::kCCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine65x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 60, 60, SkPath::kCW_Direction);
    path.addRect(10, 0, 30, 30, SkPath::kCW_Direction);
    path.addRect(24, 0, 36, 36, SkPath::kCW_Direction);
    path.addRect(32, 6, 36, 41, SkPath::kCCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine66(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 60, 60, SkPath::kCW_Direction);
    path.addRect(0, 30, 20, 20, SkPath::kCW_Direction);
    path.addRect(12, 20, 24, 30, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine66x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 60, 60, SkPath::kCW_Direction);
    path.addRect(0, 30, 20, 20, SkPath::kCW_Direction);
    path.addRect(12, 20, 24, 30, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine67(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 60, 60, SkPath::kCW_Direction);
    path.addRect(10, 40, 30, 30, SkPath::kCW_Direction);
    path.addRect(24, 20, 36, 30, SkPath::kCW_Direction);
    path.addRect(32, 0, 36, 41, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine67x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 60, 60, SkPath::kCW_Direction);
    path.addRect(10, 40, 30, 30, SkPath::kCW_Direction);
    path.addRect(24, 20, 36, 30, SkPath::kCW_Direction);
    path.addRect(32, 0, 36, 41, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine68a(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 8, 8, SkPath::kCW_Direction);
    path.addRect(2, 2, 6, 6, SkPath::kCW_Direction);
    path.addRect(1, 2, 4, 2, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine68ax(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 8, 8, SkPath::kCW_Direction);
    path.addRect(2, 2, 6, 6, SkPath::kCW_Direction);
    path.addRect(1, 2, 4, 2, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine68b(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 8, 8, SkPath::kCW_Direction);
    path.addRect(2, 2, 6, 6, SkPath::kCCW_Direction);
    path.addRect(1, 2, 2, 2, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine68bx(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 8, 8, SkPath::kCW_Direction);
    path.addRect(2, 2, 6, 6, SkPath::kCCW_Direction);
    path.addRect(1, 2, 2, 2, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine68c(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 8, 8, SkPath::kCCW_Direction);
    path.addRect(2, 2, 6, 6, SkPath::kCW_Direction);
    path.addRect(1, 2, 4, 2, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine68cx(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 8, 8, SkPath::kCCW_Direction);
    path.addRect(2, 2, 6, 6, SkPath::kCW_Direction);
    path.addRect(1, 2, 4, 2, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine68d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 8, 8, SkPath::kCCW_Direction);
    path.addRect(2, 2, 6, 6, SkPath::kCCW_Direction);
    path.addRect(1, 2, 4, 2, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine68dx(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 8, 8, SkPath::kCCW_Direction);
    path.addRect(2, 2, 6, 6, SkPath::kCCW_Direction);
    path.addRect(1, 2, 4, 2, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine68e(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 8, 8, SkPath::kCW_Direction);
    path.addRect(0, 0, 8, 8, SkPath::kCW_Direction);
    path.addRect(2, 2, 6, 6, SkPath::kCCW_Direction);
    path.addRect(1, 2, 2, 2, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine68ex(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 8, 8, SkPath::kCW_Direction);
    path.addRect(0, 0, 8, 8, SkPath::kCW_Direction);
    path.addRect(2, 2, 6, 6, SkPath::kCCW_Direction);
    path.addRect(1, 2, 2, 2, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine68f(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 8, 8, SkPath::kCW_Direction);
    path.addRect(2, 2, 6, 6, SkPath::kCCW_Direction);
    path.addRect(2, 2, 6, 6, SkPath::kCCW_Direction);
    path.addRect(1, 2, 2, 2, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine68fx(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 8, 8, SkPath::kCW_Direction);
    path.addRect(2, 2, 6, 6, SkPath::kCCW_Direction);
    path.addRect(2, 2, 6, 6, SkPath::kCCW_Direction);
    path.addRect(1, 2, 2, 2, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine68g(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 8, 8, SkPath::kCW_Direction);
    path.addRect(0, 0, 8, 8, SkPath::kCW_Direction);
    path.addRect(2, 2, 6, 6, SkPath::kCCW_Direction);
    path.addRect(2, 2, 6, 6, SkPath::kCCW_Direction);
    path.addRect(1, 2, 2, 2, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine68gx(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 8, 8, SkPath::kCW_Direction);
    path.addRect(0, 0, 8, 8, SkPath::kCW_Direction);
    path.addRect(2, 2, 6, 6, SkPath::kCCW_Direction);
    path.addRect(2, 2, 6, 6, SkPath::kCCW_Direction);
    path.addRect(1, 2, 2, 2, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine68h(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 8, 8, SkPath::kCW_Direction);
    path.addRect(2, 2, 6, 6, SkPath::kCCW_Direction);
    path.addRect(2, 2, 6, 6, SkPath::kCCW_Direction);
    path.addRect(2, 2, 6, 6, SkPath::kCCW_Direction);
    path.addRect(1, 2, 2, 2, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine68hx(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 8, 8, SkPath::kCW_Direction);
    path.addRect(2, 2, 6, 6, SkPath::kCCW_Direction);
    path.addRect(2, 2, 6, 6, SkPath::kCCW_Direction);
    path.addRect(2, 2, 6, 6, SkPath::kCCW_Direction);
    path.addRect(1, 2, 2, 2, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine69(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 20, 20, 20, SkPath::kCW_Direction);
    path.addRect(0, 20, 12, 30, SkPath::kCW_Direction);
    path.addRect(12, 32, 21, 36, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine69x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 20, 20, 20, SkPath::kCW_Direction);
    path.addRect(0, 20, 12, 30, SkPath::kCW_Direction);
    path.addRect(12, 32, 21, 36, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine70(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(0, 24, 12, 12, SkPath::kCW_Direction);
    path.addRect(12, 32, 21, 36, SkPath::kCCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine70x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(0, 24, 12, 12, SkPath::kCW_Direction);
    path.addRect(12, 32, 21, 36, SkPath::kCCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine71(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(12, 0, 24, 24, SkPath::kCW_Direction);
    path.addRect(12, 32, 21, 36, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine71x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(12, 0, 24, 24, SkPath::kCW_Direction);
    path.addRect(12, 32, 21, 36, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine72(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 60, 60, SkPath::kCW_Direction);
    path.addRect(10, 40, 30, 30, SkPath::kCW_Direction);
    path.addRect(6, 20, 18, 30, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine72x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 60, 60, SkPath::kCW_Direction);
    path.addRect(10, 40, 30, 30, SkPath::kCW_Direction);
    path.addRect(6, 20, 18, 30, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine73(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 60, 60, SkPath::kCW_Direction);
    path.addRect(0, 40, 20, 20, SkPath::kCW_Direction);
    path.addRect(0, 20, 12, 30, SkPath::kCW_Direction);
    path.addRect(0, 0, 9, 9, SkPath::kCCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine73x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 60, 60, SkPath::kCW_Direction);
    path.addRect(0, 40, 20, 20, SkPath::kCW_Direction);
    path.addRect(0, 20, 12, 30, SkPath::kCW_Direction);
    path.addRect(0, 0, 9, 9, SkPath::kCCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine74(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(20, 30, 40, 40, SkPath::kCW_Direction);
    path.addRect(24, 20, 36, 30, SkPath::kCCW_Direction);
    path.addRect(32, 24, 36, 41, SkPath::kCCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine74x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(20, 30, 40, 40, SkPath::kCW_Direction);
    path.addRect(24, 20, 36, 30, SkPath::kCCW_Direction);
    path.addRect(32, 24, 36, 41, SkPath::kCCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine75(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 60, 60, SkPath::kCW_Direction);
    path.addRect(10, 0, 30, 30, SkPath::kCCW_Direction);
    path.addRect(18, 0, 30, 30, SkPath::kCCW_Direction);
    path.addRect(12, 0, 21, 21, SkPath::kCCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine75x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 60, 60, SkPath::kCW_Direction);
    path.addRect(10, 0, 30, 30, SkPath::kCCW_Direction);
    path.addRect(18, 0, 30, 30, SkPath::kCCW_Direction);
    path.addRect(12, 0, 21, 21, SkPath::kCCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine76(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(36, 0, 66, 60, SkPath::kCW_Direction);
    path.addRect(10, 20, 40, 30, SkPath::kCW_Direction);
    path.addRect(24, 20, 36, 30, SkPath::kCCW_Direction);
    path.addRect(32, 6, 36, 41, SkPath::kCCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine76x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(36, 0, 66, 60, SkPath::kCW_Direction);
    path.addRect(10, 20, 40, 30, SkPath::kCW_Direction);
    path.addRect(24, 20, 36, 30, SkPath::kCCW_Direction);
    path.addRect(32, 6, 36, 41, SkPath::kCCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine77(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(20, 0, 40, 40, SkPath::kCW_Direction);
    path.addRect(24, 6, 36, 36, SkPath::kCCW_Direction);
    path.addRect(24, 32, 33, 36, SkPath::kCCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine77x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(20, 0, 40, 40, SkPath::kCW_Direction);
    path.addRect(24, 6, 36, 36, SkPath::kCCW_Direction);
    path.addRect(24, 32, 33, 36, SkPath::kCCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine78(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 30, 60, SkPath::kCW_Direction);
    path.addRect(10, 20, 30, 30, SkPath::kCCW_Direction);
    path.addRect(18, 20, 30, 30, SkPath::kCCW_Direction);
    path.addRect(32, 0, 36, 41, SkPath::kCCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine78x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 30, 60, SkPath::kCW_Direction);
    path.addRect(10, 20, 30, 30, SkPath::kCCW_Direction);
    path.addRect(18, 20, 30, 30, SkPath::kCCW_Direction);
    path.addRect(32, 0, 36, 41, SkPath::kCCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine79(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 36, 60, 30, SkPath::kCW_Direction);
    path.addRect(10, 30, 40, 30, SkPath::kCW_Direction);
    path.addRect(0, 20, 12, 30, SkPath::kCCW_Direction);
    path.addRect(0, 32, 9, 36, SkPath::kCCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine79x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 36, 60, 30, SkPath::kCW_Direction);
    path.addRect(10, 30, 40, 30, SkPath::kCW_Direction);
    path.addRect(0, 20, 12, 30, SkPath::kCCW_Direction);
    path.addRect(0, 32, 9, 36, SkPath::kCCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine81(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(-1, -1, 3, 3, SkPath::kCW_Direction);
    path.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    path.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    path.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    path.addRect(1, 1, 2, 2, SkPath::kCCW_Direction);
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
    path.setFillType(SkPath::kEvenOdd_FillType);
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
    path.setFillType(SkPath::kEvenOdd_FillType);
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
    path.setFillType(SkPath::kEvenOdd_FillType);
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
    path.setFillType(SkPath::kEvenOdd_FillType);
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
    path.setFillType(SkPath::kEvenOdd_FillType);
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
    path.setFillType(SkPath::kEvenOdd_FillType);
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
    path.setFillType(SkPath::kEvenOdd_FillType);
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
    path.setFillType(SkPath::kEvenOdd_FillType);
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
    path.setFillType(SkPath::kEvenOdd_FillType);
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
    path.setFillType(SkPath::kEvenOdd_FillType);
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
    path.setFillType(SkPath::kEvenOdd_FillType);
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
    path.setFillType(SkPath::kEvenOdd_FillType);
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
    path.setFillType(SkPath::kEvenOdd_FillType);
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
    path.setFillType(SkPath::kEvenOdd_FillType);
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
    path.setFillType(SkPath::kEvenOdd_FillType);
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
    path.setFillType(SkPath::kEvenOdd_FillType);
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
    path.setFillType(SkPath::kEvenOdd_FillType);
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
    path.setFillType(SkPath::kEvenOdd_FillType);
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
    path.setFillType(SkPath::kEvenOdd_FillType);
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
    path.setFillType(SkPath::kWinding_FillType);
    path.addRect(0, 0, 12, 12, SkPath::kCW_Direction);
    path.addRect(4, 0, 13, 13, SkPath::kCCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine1ax(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 12, 12, SkPath::kCW_Direction);
    path.addRect(4, 0, 13, 13, SkPath::kCCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine2ax(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 20, 20, 20, SkPath::kCW_Direction);
    path.addRect(0, 20, 12, 30, SkPath::kCW_Direction);
    path.addRect(12, 0, 21, 21, SkPath::kCCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine3aax(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(10, 30, 30, 30, SkPath::kCW_Direction);
    path.addRect(18, 20, 30, 30, SkPath::kCCW_Direction);
    path.addRect(0, 32, 9, 36, SkPath::kCCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine4ax(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(10, 30, 30, 30, SkPath::kCW_Direction);
    path.addRect(24, 20, 36, 30, SkPath::kCCW_Direction);
    path.addRect(0, 32, 9, 36, SkPath::kCCW_Direction);
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
    path.setFillType(SkPath::kEvenOdd_FillType);
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
    path.setFillType(SkPath::kEvenOdd_FillType);
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
    path.setFillType(SkPath::kEvenOdd_FillType);
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
    path.setFillType(SkPath::kEvenOdd_FillType);
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
    path.setFillType(SkPath::kEvenOdd_FillType);
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
    path.setFillType(SkPath::kEvenOdd_FillType);
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
    path.setFillType(SkPath::kWinding_FillType);
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
    path.setFillType(SkPath::kEvenOdd_FillType);
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
    testSimplify(reporter, path, filename);
}

static void testQuadratic70x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
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
    path.setFillType(SkPath::kEvenOdd_FillType);
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
    path.setFillType(SkPath::kEvenOdd_FillType);
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
    path.setFillType(SkPath::kEvenOdd_FillType);
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
    path.addRect(20, 0, 40, 40, SkPath::kCCW_Direction);
    path.addRect(24, 20, 36, 30, SkPath::kCCW_Direction);
    path.addRect(24, 32, 33, 36, SkPath::kCCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine82a(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 6, 10, SkPath::kCW_Direction);
    path.addRect(2, 2, 4, 4, SkPath::kCW_Direction);
    path.addRect(2, 6, 4, 8, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine82b(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 6, 10, SkPath::kCW_Direction);
    path.addRect(2, 2, 4, 4, SkPath::kCW_Direction);
    path.addRect(2, 6, 4, 8, SkPath::kCCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine82c(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 6, 10, SkPath::kCW_Direction);
    path.addRect(2, 2, 4, 4, SkPath::kCCW_Direction);
    path.addRect(2, 6, 4, 8, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine82d(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 6, 10, SkPath::kCW_Direction);
    path.addRect(2, 2, 4, 4, SkPath::kCCW_Direction);
    path.addRect(2, 6, 4, 8, SkPath::kCCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine82e(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 6, 10, SkPath::kCCW_Direction);
    path.addRect(2, 2, 4, 4, SkPath::kCW_Direction);
    path.addRect(2, 6, 4, 8, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine82f(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 6, 10, SkPath::kCCW_Direction);
    path.addRect(2, 2, 4, 4, SkPath::kCW_Direction);
    path.addRect(2, 6, 4, 8, SkPath::kCCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine82g(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 6, 10, SkPath::kCCW_Direction);
    path.addRect(2, 2, 4, 4, SkPath::kCCW_Direction);
    path.addRect(2, 6, 4, 8, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine82h(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 6, 10, SkPath::kCCW_Direction);
    path.addRect(2, 2, 4, 4, SkPath::kCCW_Direction);
    path.addRect(2, 6, 4, 8, SkPath::kCCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine83(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
path.addRect(10, 30, 30, 40, SkPath::kCCW_Direction);
path.addRect(0, 12, 12, 18, SkPath::kCCW_Direction);
path.addRect(4, 13, 13, 16, SkPath::kCCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine84(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 12, 60, 30, SkPath::kCCW_Direction);
    path.addRect(10, 20, 40, 30, SkPath::kCW_Direction);
    path.addRect(0, 12, 12, 12, SkPath::kCW_Direction);
    path.addRect(4, 12, 13, 13, SkPath::kCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine84x(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 12, 60, 30, SkPath::kCCW_Direction);
    path.addRect(10, 20, 40, 30, SkPath::kCCW_Direction);
    path.addRect(0, 12, 12, 12, SkPath::kCCW_Direction);
    path.addRect(4, 12, 13, 13, SkPath::kCCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testLine85(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(36, 0, 66, 60, SkPath::kCCW_Direction);
    path.addRect(20, 0, 40, 40, SkPath::kCCW_Direction);
    path.addRect(12, 0, 24, 24, SkPath::kCCW_Direction);
    path.addRect(32, 0, 36, 41, SkPath::kCCW_Direction);
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

static void skphealth_com76(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kWinding_FillType);
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

static void testRect1(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.addRect(0, 0, 60, 60, SkPath::kCCW_Direction);
    path.addRect(30, 20, 50, 50, SkPath::kCCW_Direction);
    path.addRect(24, 20, 36, 30, SkPath::kCCW_Direction);
    path.addRect(32, 24, 36, 41, SkPath::kCCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testRect2(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kWinding_FillType);
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
    path.setFillType(SkPath::kEvenOdd_FillType);
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
    path.setFillType(SkPath::kEvenOdd_FillType);
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
    path.setFillType(SkPath::kEvenOdd_FillType);
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
    path.setFillType(SkPath::kEvenOdd_FillType);
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
    path.setFillType(SkPath::kWinding_FillType);
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
    path.setFillType(SkPath::kEvenOdd_FillType);
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
    path.setFillType(SkPath::kEvenOdd_FillType);
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
    path.setFillType(SkPath::kEvenOdd_FillType);
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
    path.setFillType(SkPath::kWinding_FillType);
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
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 60, 60, SkPath::kCCW_Direction);
    path.addRect(10, 30, 40, 30, SkPath::kCCW_Direction);
    path.addRect(24, 6, 36, 36, SkPath::kCCW_Direction);
    path.addRect(32, 6, 36, 41, SkPath::kCCW_Direction);
    testSimplify(reporter, path, filename);
}

static void testRect4(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 30, 60, SkPath::kCCW_Direction);
    path.addRect(10, 0, 40, 30, SkPath::kCCW_Direction);
    path.addRect(20, 0, 30, 40, SkPath::kCCW_Direction);
    path.addRect(32, 0, 36, 41, SkPath::kCCW_Direction);
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

static void (*skipTest)(skiatest::Reporter* , const char* filename) = 0;
static void (*firstTest)(skiatest::Reporter* , const char* filename) = 0;
static void (*stopTest)(skiatest::Reporter* , const char* filename) = 0;

static TestDesc tests[] = {
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
    TEST(testRect2),
    TEST(testRect1),
    TEST(tooCloseTest),
    TEST(skphealth_com76),
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
    TEST(testLine1ax),
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
};

static const size_t testCount = SK_ARRAY_COUNT(tests);

static TestDesc subTests[] = {
    TEST(testLine3),
    TEST(testLine2),
    TEST(testLine1),
};

static const size_t subTestCount = SK_ARRAY_COUNT(subTests);

static void (*firstSubTest)(skiatest::Reporter* , const char* filename) = 0;

static bool runSubTests = false;
static bool runSubTestsFirst = false;
static bool runReverse = false;

DEF_TEST(PathOpsSimplify, reporter) {
    if (runSubTests && runSubTestsFirst) {
        RunTestSet(reporter, subTests, subTestCount, firstSubTest, NULL, stopTest, runReverse);
    }
    RunTestSet(reporter, tests, testCount, firstTest, skipTest, stopTest, runReverse);
    if (runSubTests && !runSubTestsFirst) {
        RunTestSet(reporter, subTests, subTestCount, firstSubTest, NULL, stopTest, runReverse);
    }
}
