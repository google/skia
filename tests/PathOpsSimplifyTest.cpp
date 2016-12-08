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

static void fuzz994s_11(skiatest::Reporter* reporter, const char* filename) {
    SkPath path;
    path.setFillType((SkPath::FillType) 0);
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
    path.setFillType((SkPath::FillType) 0);
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
    path.setFillType((SkPath::FillType) 0);
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
    path.setFillType((SkPath::FillType) 0);
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
    path.setFillType((SkPath::FillType) 0);
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
    path.setFillType((SkPath::FillType) 0);
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
    path.setFillType((SkPath::FillType) 0);
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
    path.setFillType((SkPath::FillType) 0);

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
    path.setFillType((SkPath::FillType) 0);
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
}

static void (*skipTest)(skiatest::Reporter* , const char* filename) = 0;
static void (*firstTest)(skiatest::Reporter* , const char* filename) = joel_2;
static void (*stopTest)(skiatest::Reporter* , const char* filename) = 0;

static TestDesc tests[] = {
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
    TEST(fuzz994s_3414),
    TEST(fuzz994s_11),
};

static const size_t subTestCount = SK_ARRAY_COUNT(subTests);

static void (*firstSubTest)(skiatest::Reporter* , const char* filename) = 0;

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
