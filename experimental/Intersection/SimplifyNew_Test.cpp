/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "EdgeWalker_Test.h"
#include "Intersection_Tests.h"
#include "ShapeOps.h"

#define TEST(name) { name, #name }

static void testLine1() {
    SkPath path;
    path.moveTo(2,0);
    path.lineTo(1,1);
    path.lineTo(0,0);
    path.close();
    testSimplifyx(path);
}

static void testLine1x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(2,0);
    path.lineTo(1,1);
    path.lineTo(0,0);
    path.close();
    testSimplifyx(path);
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
    path.setFillType(SkPath::kEvenOdd_FillType);
    addInnerCWTriangle(path);
    addOuterCWTriangle(path);
    testSimplifyx(path);
}

static void testLine2x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    addInnerCWTriangle(path);
    addOuterCWTriangle(path);
    testSimplifyx(path);
}

static void testLine3() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    addInnerCCWTriangle(path);
    addOuterCWTriangle(path);
    testSimplifyx(path);
}

static void testLine3x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    addInnerCCWTriangle(path);
    addOuterCWTriangle(path);
    testSimplifyx(path);
}

static void testLine3a() {
    SkPath path;
    addInnerCWTriangle(path);
    addOuterCCWTriangle(path);
    testSimplifyx(path);
}

static void testLine3ax() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    addInnerCWTriangle(path);
    addOuterCCWTriangle(path);
    testSimplifyx(path);
}

static void testLine3b() {
    SkPath path;
    addInnerCCWTriangle(path);
    addOuterCCWTriangle(path);
    testSimplifyx(path);
}

static void testLine3bx() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    addInnerCCWTriangle(path);
    addOuterCCWTriangle(path);
    testSimplifyx(path);
}

static void testLine4() {
    SkPath path;
    addOuterCCWTriangle(path);
    addOuterCWTriangle(path);
    testSimplifyx(path);
}

static void testLine4x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    addOuterCCWTriangle(path);
    addOuterCWTriangle(path);
    testSimplifyx(path);
}

static void testLine5() {
    SkPath path;
    addOuterCWTriangle(path);
    addOuterCWTriangle(path);
    testSimplifyx(path);
}

static void testLine5x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    addOuterCWTriangle(path);
    addOuterCWTriangle(path);
    testSimplifyx(path);
}

static void testLine6() {
    SkPath path;
    path.moveTo(0,0);
    path.lineTo(4,0);
    path.lineTo(2,2);
    path.close();
    path.moveTo(2,0);
    path.lineTo(6,0);
    path.lineTo(4,2);
    path.close();
    testSimplifyx(path);
}

static void testLine6x() {
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
    testSimplifyx(path);
}

static void testLine7() {
    SkPath path;
    path.moveTo(0,0);
    path.lineTo(4,0);
    path.lineTo(2,2);
    path.close();
    path.moveTo(6,0);
    path.lineTo(2,0);
    path.lineTo(4,2);
    path.close();
    testSimplifyx(path);
}

static void testLine7x() {
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
    testSimplifyx(path);
}

static void testLine7a() {
    SkPath path;
    path.moveTo(0,0);
    path.lineTo(4,0);
    path.lineTo(2,2);
    path.close();
    testSimplifyx(path);
}

static void testLine7ax() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(0,0);
    path.lineTo(4,0);
    path.lineTo(2,2);
    path.close();
    testSimplifyx(path);
}

static void testLine7b() {
    SkPath path;
    path.moveTo(0,0);
    path.lineTo(4,0);
    path.close();
    path.moveTo(6,0);
    path.lineTo(2,0);
    path.lineTo(4,2);
    path.close();
    testSimplifyx(path);
}

static void testLine7bx() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(0,0);
    path.lineTo(4,0);
    path.close();
    path.moveTo(6,0);
    path.lineTo(2,0);
    path.lineTo(4,2);
    path.close();
    testSimplifyx(path);
}

static void testLine8() {
    SkPath path;
    path.moveTo(0,4);
    path.lineTo(4,4);
    path.lineTo(2,2);
    path.close();
    path.moveTo(2,4);
    path.lineTo(6,4);
    path.lineTo(4,2);
    path.close();
    testSimplifyx(path);
}

static void testLine8x() {
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
    testSimplifyx(path);
}

static void testLine9() {
    SkPath path;
    path.moveTo(0,4);
    path.lineTo(4,4);
    path.lineTo(2,2);
    path.close();
    path.moveTo(6,4);
    path.lineTo(2,4);
    path.lineTo(4,2);
    path.close();
    testSimplifyx(path);
}

static void testLine9x() {
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
    testSimplifyx(path);
}

static void testLine10() {
    SkPath path;
    path.moveTo(0,4);
    path.lineTo(4,4);
    path.lineTo(2,2);
    path.close();
    path.moveTo(2,1);
    path.lineTo(3,4);
    path.lineTo(6,1);
    path.close();
    testSimplifyx(path);
}

static void testLine10x() {
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
    testSimplifyx(path);
}

static void testLine10a() {
    SkPath path;
    path.moveTo(0,4);
    path.lineTo(8,4);
    path.lineTo(4,0);
    path.close();
    path.moveTo(2,2);
    path.lineTo(3,3);
    path.lineTo(4,2);
    path.close();
    testSimplifyx(path);
}

static void testLine10ax() {
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
    testSimplifyx(path);
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

static void testLine11() {
    SkPath path;
    addCWContainer(path);
    addCWContents(path);
    testSimplifyx(path);
}

static void testLine11x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    addCWContainer(path);
    addCWContents(path);
    testSimplifyx(path);
}

static void testLine12() {
    SkPath path;
    addCCWContainer(path);
    addCWContents(path);
    testSimplifyx(path);
}

static void testLine12x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    addCCWContainer(path);
    addCWContents(path);
    testSimplifyx(path);
}

static void testLine13() {
    SkPath path;
    addCWContainer(path);
    addCCWContents(path);
    testSimplifyx(path);
}

static void testLine13x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    addCWContainer(path);
    addCCWContents(path);
    testSimplifyx(path);
}

static void testLine14() {
    SkPath path;
    addCCWContainer(path);
    addCCWContents(path);
    testSimplifyx(path);
}

static void testLine14x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    addCCWContainer(path);
    addCCWContents(path);
    testSimplifyx(path);
}

static void testLine15() {
    SkPath path;
    path.addRect(0, 0, 9, 9, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine15x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 9, 9, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine16() {
    SkPath path;
    path.addRect(0, 0, 12, 12, SkPath::kCW_Direction);
    path.addRect(0, 4, 9, 9, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine16x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 12, 12, SkPath::kCW_Direction);
    path.addRect(0, 4, 9, 9, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine17() {
    SkPath path;
    path.addRect(0, 0, 12, 12, SkPath::kCW_Direction);
    path.addRect(4, 12, 13, 13, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine17x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 12, 12, SkPath::kCW_Direction);
    path.addRect(4, 12, 13, 13, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine18() {
    SkPath path;
    path.addRect(0, 0, 12, 12, SkPath::kCW_Direction);
    path.addRect(12, 4, 21, 21, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine18x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 12, 12, SkPath::kCW_Direction);
    path.addRect(12, 4, 21, 21, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine19() {
    SkPath path;
    path.addRect(0, 0, 12, 12, SkPath::kCW_Direction);
    path.addRect(12, 16, 21, 21, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine19x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 12, 12, SkPath::kCW_Direction);
    path.addRect(12, 16, 21, 21, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine20() {
    SkPath path;
    path.addRect(0, 12, 12, 12, SkPath::kCW_Direction);
    path.addRect(0, 12, 9, 9, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine20x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 12, 12, 12, SkPath::kCW_Direction);
    path.addRect(0, 12, 9, 9, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine21() {
    SkPath path;
    path.addRect(0, 12, 12, 12, SkPath::kCW_Direction);
    path.addRect(0, 16, 9, 9, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine21x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 12, 12, 12, SkPath::kCW_Direction);
    path.addRect(0, 16, 9, 9, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine22() {
    SkPath path;
    path.addRect(0, 12, 12, 12, SkPath::kCW_Direction);
    path.addRect(4, 12, 13, 13, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine22x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 12, 12, 12, SkPath::kCW_Direction);
    path.addRect(4, 12, 13, 13, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine23() {
    SkPath path;
    path.addRect(0, 12, 12, 12, SkPath::kCW_Direction);
    path.addRect(12, 0, 21, 21, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine23x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 12, 12, 12, SkPath::kCW_Direction);
    path.addRect(12, 0, 21, 21, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine24a() {
    SkPath path;
    path.moveTo(2,0);
    path.lineTo(4,4);
    path.lineTo(0,4);
    path.close();
    path.moveTo(2,0);
    path.lineTo(1,2);
    path.lineTo(2,2);
    path.close();
    testSimplifyx(path);
}

static void testLine24ax() {
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
    testSimplifyx(path);
}

static void testLine24() {
    SkPath path;
    path.addRect(0, 18, 12, 12, SkPath::kCW_Direction);
    path.addRect(4, 12, 13, 13, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine24x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 18, 12, 12, SkPath::kCW_Direction);
    path.addRect(4, 12, 13, 13, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine25() {
    SkPath path;
    path.addRect(0, 6, 12, 12, SkPath::kCW_Direction);
    path.addRect(12, 0, 21, 21, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine25x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 6, 12, 12, SkPath::kCW_Direction);
    path.addRect(12, 0, 21, 21, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine26() {
    SkPath path;
    path.addRect(0, 18, 12, 12, SkPath::kCW_Direction);
    path.addRect(0, 12, 9, 9, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine26x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 18, 12, 12, SkPath::kCW_Direction);
    path.addRect(0, 12, 9, 9, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine27() {
    SkPath path;
    path.addRect(0, 18, 12, 12, SkPath::kCW_Direction);
    path.addRect(12, 8, 21, 21, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine27x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 18, 12, 12, SkPath::kCW_Direction);
    path.addRect(12, 8, 21, 21, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine28() {
    SkPath path;
    path.addRect(0, 6, 12, 12, SkPath::kCW_Direction);
    path.addRect(0, 0, 9, 9, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine28x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 6, 12, 12, SkPath::kCW_Direction);
    path.addRect(0, 0, 9, 9, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine29() {
    SkPath path;
    path.addRect(0, 18, 12, 12, SkPath::kCW_Direction);
    path.addRect(12, 12, 21, 21, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine29x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 18, 12, 12, SkPath::kCW_Direction);
    path.addRect(12, 12, 21, 21, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine30() {
    SkPath path;
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(0, 0, 12, 12, SkPath::kCW_Direction);
    path.addRect(4, 4, 13, 13, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine30x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(0, 0, 12, 12, SkPath::kCW_Direction);
    path.addRect(4, 4, 13, 13, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine31() {
    SkPath path;
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(0, 0, 12, 12, SkPath::kCW_Direction);
    path.addRect(0, 4, 9, 9, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine31x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(0, 0, 12, 12, SkPath::kCW_Direction);
    path.addRect(0, 4, 9, 9, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine32() {
    SkPath path;
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(0, 0, 12, 12, SkPath::kCW_Direction);
    path.addRect(4, 12, 13, 13, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine32x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(0, 0, 12, 12, SkPath::kCW_Direction);
    path.addRect(4, 12, 13, 13, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine33() {
    SkPath path;
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(0, 0, 12, 12, SkPath::kCW_Direction);
    path.addRect(4, 16, 13, 13, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine33x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(0, 0, 12, 12, SkPath::kCW_Direction);
    path.addRect(4, 16, 13, 13, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine34() {
    SkPath path;
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(0, 6, 12, 12, SkPath::kCW_Direction);
    path.addRect(4, 12, 13, 13, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine34x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(0, 6, 12, 12, SkPath::kCW_Direction);
    path.addRect(4, 12, 13, 13, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine35() {
    SkPath path;
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(6, 0, 18, 18, SkPath::kCW_Direction);
    path.addRect(4, 16, 13, 13, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine35x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(6, 0, 18, 18, SkPath::kCW_Direction);
    path.addRect(4, 16, 13, 13, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine36() {
    SkPath path;
    path.addRect(0, 10, 20, 20, SkPath::kCW_Direction);
    path.addRect(6, 12, 18, 18, SkPath::kCW_Direction);
    path.addRect(4, 16, 13, 13, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine36x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 10, 20, 20, SkPath::kCW_Direction);
    path.addRect(6, 12, 18, 18, SkPath::kCW_Direction);
    path.addRect(4, 16, 13, 13, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine37() {
    SkPath path;
    path.addRect(0, 20, 20, 20, SkPath::kCW_Direction);
    path.addRect(18, 24, 30, 30, SkPath::kCW_Direction);
    path.addRect(0, 0, 9, 9, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine37x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 20, 20, 20, SkPath::kCW_Direction);
    path.addRect(18, 24, 30, 30, SkPath::kCW_Direction);
    path.addRect(0, 0, 9, 9, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine38() {
    SkPath path;
    path.addRect(10, 0, 30, 30, SkPath::kCW_Direction);
    path.addRect(6, 12, 18, 18, SkPath::kCW_Direction);
    path.addRect(12, 12, 21, 21, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine38x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(10, 0, 30, 30, SkPath::kCW_Direction);
    path.addRect(6, 12, 18, 18, SkPath::kCW_Direction);
    path.addRect(12, 12, 21, 21, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine40() {
    SkPath path;
    path.addRect(10, 0, 30, 30, SkPath::kCW_Direction);
    path.addRect(12, 18, 24, 24, SkPath::kCW_Direction);
    path.addRect(4, 16, 13, 13, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine40x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(10, 0, 30, 30, SkPath::kCW_Direction);
    path.addRect(12, 18, 24, 24, SkPath::kCW_Direction);
    path.addRect(4, 16, 13, 13, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine41() {
    SkPath path;
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(18, 24, 30, 30, SkPath::kCW_Direction);
    path.addRect(12, 0, 21, 21, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine41x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(18, 24, 30, 30, SkPath::kCW_Direction);
    path.addRect(12, 0, 21, 21, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine42() {
    SkPath path;
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(0, 0, 12, 12, SkPath::kCW_Direction);
    path.addRect(8, 16, 17, 17, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine42x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(0, 0, 12, 12, SkPath::kCW_Direction);
    path.addRect(8, 16, 17, 17, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine43() {
    SkPath path;
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(6, 24, 18, 18, SkPath::kCW_Direction);
    path.addRect(0, 32, 9, 36, SkPath::kCCW_Direction);
    testSimplifyx(path);
}

static void testLine43x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(6, 24, 18, 18, SkPath::kCW_Direction);
    path.addRect(0, 32, 9, 36, SkPath::kCCW_Direction);
    testSimplifyx(path);
}

static void testLine44() {
    SkPath path;
    path.addRect(10, 40, 30, 30, SkPath::kCW_Direction);
    path.addRect(18, 0, 30, 30, SkPath::kCW_Direction);
    path.addRect(18, 32, 27, 36, SkPath::kCCW_Direction);
    testSimplifyx(path);
}

static void testLine44x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(10, 40, 30, 30, SkPath::kCW_Direction);
    path.addRect(18, 0, 30, 30, SkPath::kCW_Direction);
    path.addRect(18, 32, 27, 36, SkPath::kCCW_Direction);
    testSimplifyx(path);
}

static void testLine45() {
    SkPath path;
    path.addRect(10, 0, 30, 30, SkPath::kCW_Direction);
    path.addRect(18, 0, 30, 30, SkPath::kCW_Direction);
    path.addRect(24, 32, 33, 36, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine45x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(10, 0, 30, 30, SkPath::kCW_Direction);
    path.addRect(18, 0, 30, 30, SkPath::kCW_Direction);
    path.addRect(24, 32, 33, 36, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine46() {
    SkPath path;
    path.addRect(10, 40, 30, 30, SkPath::kCW_Direction);
    path.addRect(24, 0, 36, 36, SkPath::kCW_Direction);
    path.addRect(24, 32, 33, 36, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine46x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(10, 40, 30, 30, SkPath::kCW_Direction);
    path.addRect(24, 0, 36, 36, SkPath::kCW_Direction);
    path.addRect(24, 32, 33, 36, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine47() {
    SkPath path;
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(0, 0, 12, 12, SkPath::kCW_Direction);
    path.addRect(0, 0, 9, 9, SkPath::kCCW_Direction);
    testSimplifyx(path);
}

static void testLine47x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(0, 0, 12, 12, SkPath::kCW_Direction);
    path.addRect(0, 0, 9, 9, SkPath::kCCW_Direction);
    testSimplifyx(path);
}

static void testLine48() {
    SkPath path;
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(0, 6, 12, 12, SkPath::kCW_Direction);
    path.addRect(0, 0, 9, 9, SkPath::kCCW_Direction);
    testSimplifyx(path);
}

static void testLine48x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(0, 6, 12, 12, SkPath::kCW_Direction);
    path.addRect(0, 0, 9, 9, SkPath::kCCW_Direction);
    testSimplifyx(path);
}

static void testLine49() {
    SkPath path;
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(0, 0, 12, 12, SkPath::kCW_Direction);
    path.addRect(0, 0, 9, 9, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine49x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(0, 0, 12, 12, SkPath::kCW_Direction);
    path.addRect(0, 0, 9, 9, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine50() {
    SkPath path;
    path.addRect(10, 30, 30, 30, SkPath::kCW_Direction);
    path.addRect(24, 20, 36, 30, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine50x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(10, 30, 30, 30, SkPath::kCW_Direction);
    path.addRect(24, 20, 36, 30, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine51() {
    SkPath path;
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(0, 12, 12, 12, SkPath::kCW_Direction);
    path.addRect(4, 12, 13, 13, SkPath::kCCW_Direction);
    testSimplifyx(path);
}

static void testLine51x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(0, 12, 12, 12, SkPath::kCW_Direction);
    path.addRect(4, 12, 13, 13, SkPath::kCCW_Direction);
    testSimplifyx(path);
}

static void testLine52() {
    SkPath path;
    path.addRect(0, 30, 20, 20, SkPath::kCW_Direction);
    path.addRect(6, 20, 18, 30, SkPath::kCW_Direction);
    path.addRect(32, 0, 36, 41, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine52x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 30, 20, 20, SkPath::kCW_Direction);
    path.addRect(6, 20, 18, 30, SkPath::kCW_Direction);
    path.addRect(32, 0, 36, 41, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine53() {
    SkPath path;
    path.addRect(10, 30, 30, 30, SkPath::kCW_Direction);
    path.addRect(12, 20, 24, 30, SkPath::kCW_Direction);
    path.addRect(12, 32, 21, 36, SkPath::kCCW_Direction);
    testSimplifyx(path);
}

static void testLine53x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(10, 30, 30, 30, SkPath::kCW_Direction);
    path.addRect(12, 20, 24, 30, SkPath::kCW_Direction);
    path.addRect(12, 32, 21, 36, SkPath::kCCW_Direction);
    testSimplifyx(path);
}

static void testLine54() {
    SkPath path;
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(6, 0, 18, 18, SkPath::kCW_Direction);
    path.addRect(8, 4, 17, 17, SkPath::kCCW_Direction);
    testSimplifyx(path);
}

static void testLine54x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(6, 0, 18, 18, SkPath::kCW_Direction);
    path.addRect(8, 4, 17, 17, SkPath::kCCW_Direction);
    testSimplifyx(path);
}

static void testLine55() {
    SkPath path;
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(6, 6, 18, 18, SkPath::kCW_Direction);
    path.addRect(4, 4, 13, 13, SkPath::kCCW_Direction);
    testSimplifyx(path);
}

static void testLine55x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(6, 6, 18, 18, SkPath::kCW_Direction);
    path.addRect(4, 4, 13, 13, SkPath::kCCW_Direction);
    testSimplifyx(path);
}

static void testLine56() {
    SkPath path;
    path.addRect(0, 20, 20, 20, SkPath::kCW_Direction);
    path.addRect(18, 20, 30, 30, SkPath::kCW_Direction);
    path.addRect(12, 0, 21, 21, SkPath::kCCW_Direction);
    testSimplifyx(path);
}

static void testLine56x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 20, 20, 20, SkPath::kCW_Direction);
    path.addRect(18, 20, 30, 30, SkPath::kCW_Direction);
    path.addRect(12, 0, 21, 21, SkPath::kCCW_Direction);
    testSimplifyx(path);
}

static void testLine57() {
    SkPath path;
    path.addRect(20, 0, 40, 40, SkPath::kCW_Direction);
    path.addRect(20, 0, 30, 40, SkPath::kCW_Direction);
    path.addRect(12, 0, 21, 21, SkPath::kCCW_Direction);
    testSimplifyx(path);
}

static void testLine57x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(20, 0, 40, 40, SkPath::kCW_Direction);
    path.addRect(20, 0, 30, 40, SkPath::kCW_Direction);
    path.addRect(12, 0, 21, 21, SkPath::kCCW_Direction);
    testSimplifyx(path);
}

static void testLine58() {
    SkPath path;
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(0, 0, 12, 12, SkPath::kCCW_Direction);
    path.addRect(0, 12, 9, 9, SkPath::kCCW_Direction);
    testSimplifyx(path);
}

static void testLine58x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(0, 0, 12, 12, SkPath::kCCW_Direction);
    path.addRect(0, 12, 9, 9, SkPath::kCCW_Direction);
    testSimplifyx(path);
}

static void testLine59() {
    SkPath path;
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(6, 6, 18, 18, SkPath::kCCW_Direction);
    path.addRect(4, 4, 13, 13, SkPath::kCCW_Direction);
    testSimplifyx(path);
}

static void testLine59x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(6, 6, 18, 18, SkPath::kCCW_Direction);
    path.addRect(4, 4, 13, 13, SkPath::kCCW_Direction);
    testSimplifyx(path);
}

static void testLine60() {
    SkPath path;
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(6, 12, 18, 18, SkPath::kCCW_Direction);
    path.addRect(4, 12, 13, 13, SkPath::kCCW_Direction);
    testSimplifyx(path);
}

static void testLine60x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(6, 12, 18, 18, SkPath::kCCW_Direction);
    path.addRect(4, 12, 13, 13, SkPath::kCCW_Direction);
    testSimplifyx(path);
}

static void testLine61() {
    SkPath path;
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(12, 0, 24, 24, SkPath::kCCW_Direction);
    path.addRect(12, 0, 21, 21, SkPath::kCCW_Direction);
    testSimplifyx(path);
}

static void testLine61x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(12, 0, 24, 24, SkPath::kCCW_Direction);
    path.addRect(12, 0, 21, 21, SkPath::kCCW_Direction);
    testSimplifyx(path);
}

static void testLine62() {
    SkPath path;
    path.addRect(0, 0, 60, 60, SkPath::kCW_Direction);
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(0, 12, 12, 12, SkPath::kCW_Direction);
    path.addRect(4, 12, 13, 13, SkPath::kCCW_Direction);
    testSimplifyx(path);
}

static void testLine62x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 60, 60, SkPath::kCW_Direction);
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(0, 12, 12, 12, SkPath::kCW_Direction);
    path.addRect(4, 12, 13, 13, SkPath::kCCW_Direction);
    testSimplifyx(path);
}

static void testLine63() {
    SkPath path;
    path.addRect(0, 0, 60, 60, SkPath::kCW_Direction);
    path.addRect(0, 10, 20, 20, SkPath::kCW_Direction);
    path.addRect(0, 6, 12, 12, SkPath::kCCW_Direction);
    path.addRect(0, 32, 9, 36, SkPath::kCCW_Direction);
    testSimplifyx(path);
}

static void testLine63x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 60, 60, SkPath::kCW_Direction);
    path.addRect(0, 10, 20, 20, SkPath::kCW_Direction);
    path.addRect(0, 6, 12, 12, SkPath::kCCW_Direction);
    path.addRect(0, 32, 9, 36, SkPath::kCCW_Direction);
    testSimplifyx(path);
}

static void testLine64() {
    SkPath path;
    path.addRect(0, 0, 60, 60, SkPath::kCW_Direction);
    path.addRect(10, 40, 30, 30, SkPath::kCW_Direction);
    path.addRect(18, 6, 30, 30, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine64x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 60, 60, SkPath::kCW_Direction);
    path.addRect(10, 40, 30, 30, SkPath::kCW_Direction);
    path.addRect(18, 6, 30, 30, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine65() {
    SkPath path;
    path.addRect(0, 0, 60, 60, SkPath::kCW_Direction);
    path.addRect(10, 0, 30, 30, SkPath::kCW_Direction);
    path.addRect(24, 0, 36, 36, SkPath::kCW_Direction);
    path.addRect(32, 6, 36, 41, SkPath::kCCW_Direction);
    testSimplifyx(path);
}

static void testLine65x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 60, 60, SkPath::kCW_Direction);
    path.addRect(10, 0, 30, 30, SkPath::kCW_Direction);
    path.addRect(24, 0, 36, 36, SkPath::kCW_Direction);
    path.addRect(32, 6, 36, 41, SkPath::kCCW_Direction);
    testSimplifyx(path);
}

static void testLine66() {
    SkPath path;
    path.addRect(0, 0, 60, 60, SkPath::kCW_Direction);
    path.addRect(0, 30, 20, 20, SkPath::kCW_Direction);
    path.addRect(12, 20, 24, 30, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine66x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 60, 60, SkPath::kCW_Direction);
    path.addRect(0, 30, 20, 20, SkPath::kCW_Direction);
    path.addRect(12, 20, 24, 30, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine67() {
    SkPath path;
    path.addRect(0, 0, 60, 60, SkPath::kCW_Direction);
    path.addRect(10, 40, 30, 30, SkPath::kCW_Direction);
    path.addRect(24, 20, 36, 30, SkPath::kCW_Direction);
    path.addRect(32, 0, 36, 41, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine67x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 60, 60, SkPath::kCW_Direction);
    path.addRect(10, 40, 30, 30, SkPath::kCW_Direction);
    path.addRect(24, 20, 36, 30, SkPath::kCW_Direction);
    path.addRect(32, 0, 36, 41, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine68a() {
    SkPath path;
    path.addRect(0, 0, 8, 8, SkPath::kCW_Direction);
    path.addRect(2, 2, 6, 6, SkPath::kCW_Direction);
    path.addRect(1, 2, 4, 2, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine68ax() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 8, 8, SkPath::kCW_Direction);
    path.addRect(2, 2, 6, 6, SkPath::kCW_Direction);
    path.addRect(1, 2, 4, 2, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine68b() {
    SkPath path;
    path.addRect(0, 0, 8, 8, SkPath::kCW_Direction);
    path.addRect(2, 2, 6, 6, SkPath::kCCW_Direction);
    path.addRect(1, 2, 2, 2, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine68bx() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 8, 8, SkPath::kCW_Direction);
    path.addRect(2, 2, 6, 6, SkPath::kCCW_Direction);
    path.addRect(1, 2, 2, 2, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine68c() {
    SkPath path;
    path.addRect(0, 0, 8, 8, SkPath::kCCW_Direction);
    path.addRect(2, 2, 6, 6, SkPath::kCW_Direction);
    path.addRect(1, 2, 4, 2, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine68cx() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 8, 8, SkPath::kCCW_Direction);
    path.addRect(2, 2, 6, 6, SkPath::kCW_Direction);
    path.addRect(1, 2, 4, 2, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine68d() {
    SkPath path;
    path.addRect(0, 0, 8, 8, SkPath::kCCW_Direction);
    path.addRect(2, 2, 6, 6, SkPath::kCCW_Direction);
    path.addRect(1, 2, 4, 2, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine68dx() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 8, 8, SkPath::kCCW_Direction);
    path.addRect(2, 2, 6, 6, SkPath::kCCW_Direction);
    path.addRect(1, 2, 4, 2, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine68e() {
    SkPath path;
    path.addRect(0, 0, 8, 8, SkPath::kCW_Direction);
    path.addRect(0, 0, 8, 8, SkPath::kCW_Direction);
    path.addRect(2, 2, 6, 6, SkPath::kCCW_Direction);
    path.addRect(1, 2, 2, 2, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine68ex() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 8, 8, SkPath::kCW_Direction);
    path.addRect(0, 0, 8, 8, SkPath::kCW_Direction);
    path.addRect(2, 2, 6, 6, SkPath::kCCW_Direction);
    path.addRect(1, 2, 2, 2, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine68f() {
    SkPath path;
    path.addRect(0, 0, 8, 8, SkPath::kCW_Direction);
    path.addRect(2, 2, 6, 6, SkPath::kCCW_Direction);
    path.addRect(2, 2, 6, 6, SkPath::kCCW_Direction);
    path.addRect(1, 2, 2, 2, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine68fx() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 8, 8, SkPath::kCW_Direction);
    path.addRect(2, 2, 6, 6, SkPath::kCCW_Direction);
    path.addRect(2, 2, 6, 6, SkPath::kCCW_Direction);
    path.addRect(1, 2, 2, 2, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine68g() {
    SkPath path;
    path.addRect(0, 0, 8, 8, SkPath::kCW_Direction);
    path.addRect(0, 0, 8, 8, SkPath::kCW_Direction);
    path.addRect(2, 2, 6, 6, SkPath::kCCW_Direction);
    path.addRect(2, 2, 6, 6, SkPath::kCCW_Direction);
    path.addRect(1, 2, 2, 2, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine68gx() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 8, 8, SkPath::kCW_Direction);
    path.addRect(0, 0, 8, 8, SkPath::kCW_Direction);
    path.addRect(2, 2, 6, 6, SkPath::kCCW_Direction);
    path.addRect(2, 2, 6, 6, SkPath::kCCW_Direction);
    path.addRect(1, 2, 2, 2, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine68h() {
    SkPath path;
    path.addRect(0, 0, 8, 8, SkPath::kCW_Direction);
    path.addRect(2, 2, 6, 6, SkPath::kCCW_Direction);
    path.addRect(2, 2, 6, 6, SkPath::kCCW_Direction);
    path.addRect(2, 2, 6, 6, SkPath::kCCW_Direction);
    path.addRect(1, 2, 2, 2, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine68hx() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 8, 8, SkPath::kCW_Direction);
    path.addRect(2, 2, 6, 6, SkPath::kCCW_Direction);
    path.addRect(2, 2, 6, 6, SkPath::kCCW_Direction);
    path.addRect(2, 2, 6, 6, SkPath::kCCW_Direction);
    path.addRect(1, 2, 2, 2, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine69() {
    SkPath path;
    path.addRect(0, 20, 20, 20, SkPath::kCW_Direction);
    path.addRect(0, 20, 12, 30, SkPath::kCW_Direction);
    path.addRect(12, 32, 21, 36, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine69x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 20, 20, 20, SkPath::kCW_Direction);
    path.addRect(0, 20, 12, 30, SkPath::kCW_Direction);
    path.addRect(12, 32, 21, 36, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine70() {
    SkPath path;
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(0, 24, 12, 12, SkPath::kCW_Direction);
    path.addRect(12, 32, 21, 36, SkPath::kCCW_Direction);
    testSimplifyx(path);
}

static void testLine70x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(0, 24, 12, 12, SkPath::kCW_Direction);
    path.addRect(12, 32, 21, 36, SkPath::kCCW_Direction);
    testSimplifyx(path);
}

static void testLine71() {
    SkPath path;
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(12, 0, 24, 24, SkPath::kCW_Direction);
    path.addRect(12, 32, 21, 36, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine71x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 20, 20, SkPath::kCW_Direction);
    path.addRect(12, 0, 24, 24, SkPath::kCW_Direction);
    path.addRect(12, 32, 21, 36, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine72() {
    SkPath path;
    path.addRect(0, 0, 60, 60, SkPath::kCW_Direction);
    path.addRect(10, 40, 30, 30, SkPath::kCW_Direction);
    path.addRect(6, 20, 18, 30, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine72x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 60, 60, SkPath::kCW_Direction);
    path.addRect(10, 40, 30, 30, SkPath::kCW_Direction);
    path.addRect(6, 20, 18, 30, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine73() {
    SkPath path;
    path.addRect(0, 0, 60, 60, SkPath::kCW_Direction);
    path.addRect(0, 40, 20, 20, SkPath::kCW_Direction);
    path.addRect(0, 20, 12, 30, SkPath::kCW_Direction);
    path.addRect(0, 0, 9, 9, SkPath::kCCW_Direction);
    testSimplifyx(path);
}

static void testLine73x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 60, 60, SkPath::kCW_Direction);
    path.addRect(0, 40, 20, 20, SkPath::kCW_Direction);
    path.addRect(0, 20, 12, 30, SkPath::kCW_Direction);
    path.addRect(0, 0, 9, 9, SkPath::kCCW_Direction);
    testSimplifyx(path);
}

static void testLine74() {
    SkPath path;
    path.addRect(20, 30, 40, 40, SkPath::kCW_Direction);
    path.addRect(24, 20, 36, 30, SkPath::kCCW_Direction);
    path.addRect(32, 24, 36, 41, SkPath::kCCW_Direction);
    testSimplifyx(path);
}

static void testLine74x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(20, 30, 40, 40, SkPath::kCW_Direction);
    path.addRect(24, 20, 36, 30, SkPath::kCCW_Direction);
    path.addRect(32, 24, 36, 41, SkPath::kCCW_Direction);
    testSimplifyx(path);
}

static void testLine75() {
    SkPath path;
    path.addRect(0, 0, 60, 60, SkPath::kCW_Direction);
    path.addRect(10, 0, 30, 30, SkPath::kCCW_Direction);
    path.addRect(18, 0, 30, 30, SkPath::kCCW_Direction);
    path.addRect(12, 0, 21, 21, SkPath::kCCW_Direction);
    testSimplifyx(path);
}

static void testLine75x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 60, 60, SkPath::kCW_Direction);
    path.addRect(10, 0, 30, 30, SkPath::kCCW_Direction);
    path.addRect(18, 0, 30, 30, SkPath::kCCW_Direction);
    path.addRect(12, 0, 21, 21, SkPath::kCCW_Direction);
    testSimplifyx(path);
}

static void testLine76() {
    SkPath path;
    path.addRect(36, 0, 66, 60, SkPath::kCW_Direction);
    path.addRect(10, 20, 40, 30, SkPath::kCW_Direction);
    path.addRect(24, 20, 36, 30, SkPath::kCCW_Direction);
    path.addRect(32, 6, 36, 41, SkPath::kCCW_Direction);
    testSimplifyx(path);
}

static void testLine76x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(36, 0, 66, 60, SkPath::kCW_Direction);
    path.addRect(10, 20, 40, 30, SkPath::kCW_Direction);
    path.addRect(24, 20, 36, 30, SkPath::kCCW_Direction);
    path.addRect(32, 6, 36, 41, SkPath::kCCW_Direction);
    testSimplifyx(path);
}

static void testLine77() {
    SkPath path;
    path.addRect(20, 0, 40, 40, SkPath::kCW_Direction);
    path.addRect(24, 6, 36, 36, SkPath::kCCW_Direction);
    path.addRect(24, 32, 33, 36, SkPath::kCCW_Direction);
    testSimplifyx(path);
}

static void testLine77x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(20, 0, 40, 40, SkPath::kCW_Direction);
    path.addRect(24, 6, 36, 36, SkPath::kCCW_Direction);
    path.addRect(24, 32, 33, 36, SkPath::kCCW_Direction);
    testSimplifyx(path);
}

static void testLine78() {
    SkPath path;
    path.addRect(0, 0, 30, 60, SkPath::kCW_Direction);
    path.addRect(10, 20, 30, 30, SkPath::kCCW_Direction);
    path.addRect(18, 20, 30, 30, SkPath::kCCW_Direction);
    path.addRect(32, 0, 36, 41, SkPath::kCCW_Direction);
    testSimplifyx(path);
}

static void testLine78x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 30, 60, SkPath::kCW_Direction);
    path.addRect(10, 20, 30, 30, SkPath::kCCW_Direction);
    path.addRect(18, 20, 30, 30, SkPath::kCCW_Direction);
    path.addRect(32, 0, 36, 41, SkPath::kCCW_Direction);
    testSimplifyx(path);
}

static void testLine79() {
    SkPath path;
    path.addRect(0, 36, 60, 30, SkPath::kCW_Direction);
    path.addRect(10, 30, 40, 30, SkPath::kCW_Direction);
    path.addRect(0, 20, 12, 30, SkPath::kCCW_Direction);
    path.addRect(0, 32, 9, 36, SkPath::kCCW_Direction);
    testSimplifyx(path);
}

static void testLine79x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 36, 60, 30, SkPath::kCW_Direction);
    path.addRect(10, 30, 40, 30, SkPath::kCW_Direction);
    path.addRect(0, 20, 12, 30, SkPath::kCCW_Direction);
    path.addRect(0, 32, 9, 36, SkPath::kCCW_Direction);
    testSimplifyx(path);
}

static void testLine81() {
    SkPath path;
    path.addRect(-1, -1, 3, 3, SkPath::kCW_Direction);
    path.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    path.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    path.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    path.addRect(1, 1, 2, 2, SkPath::kCCW_Direction);
    testSimplifyx(path);
}

static void testDegenerate1() {
    SkPath path;
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.lineTo(2, 0);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(2, 0);
    path.close();
    testSimplifyx(path);
}

static void testDegenerate1x() {
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
    testSimplifyx(path);
}

static void testDegenerate2() {
    SkPath path;
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.lineTo(0, 0);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(0, 1);
    path.close();
    testSimplifyx(path);
}

static void testDegenerate2x() {
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
    testSimplifyx(path);
}

static void testDegenerate3() {
    SkPath path;
    path.moveTo(0, 0);
    path.lineTo(2, 0);
    path.lineTo(1, 0);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.lineTo(3, 0);
    path.close();
    testSimplifyx(path);
}

static void testDegenerate3x() {
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
    testSimplifyx(path);
}

static void testDegenerate4() {
    SkPath path;
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(1, 3);
    path.close();
    path.moveTo(1, 0);
    path.lineTo(1, 1);
    path.lineTo(1, 2);
    path.close();
    testSimplifyx(path);
}

static void testDegenerate4x() {
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
    testSimplifyx(path);
}

static void testNondegenerate1() {
    SkPath path;
    path.moveTo(0, 0);
    path.lineTo(3, 0);
    path.lineTo(1, 3);
    path.close();
    path.moveTo(1, 1);
    path.lineTo(2, 1);
    path.lineTo(1, 2);
    path.close();
    testSimplifyx(path);
}

static void testNondegenerate1x() {
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
    testSimplifyx(path);
}

static void testNondegenerate2() {
    SkPath path;
    path.moveTo(1, 0);
    path.lineTo(0, 1);
    path.lineTo(1, 1);
    path.close();
    path.moveTo(0, 2);
    path.lineTo(0, 3);
    path.lineTo(1, 2);
    path.close();
    testSimplifyx(path);
}

static void testNondegenerate2x() {
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
    testSimplifyx(path);
}

static void testNondegenerate3() {
    SkPath path;
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(2, 1);
    path.close();
    path.moveTo(0, 1);
    path.lineTo(1, 1);
    path.lineTo(0, 2);
    path.close();
    testSimplifyx(path);
}

static void testNondegenerate3x() {
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
    testSimplifyx(path);
}

static void testNondegenerate4() {
    SkPath path;
    path.moveTo(1, 0);
    path.lineTo(0, 1);
    path.lineTo(1, 2);
    path.close();
    path.moveTo(0, 2);
    path.lineTo(0, 3);
    path.lineTo(1, 3);
    path.close();
    testSimplifyx(path);
}

static void testNondegenerate4x() {
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
    testSimplifyx(path);
}

static void testQuadralateral5() {
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
    testSimplifyx(path);
}

static void testQuadralateral5x() {
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
    testSimplifyx(path);
}

static void testQuadralateral6() {
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
    testSimplifyx(path);
}

static void testQuadralateral6x() {
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
    testSimplifyx(path);
}

static void testFauxQuadralateral6() {
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
    testSimplifyx(path);
}

static void testFauxQuadralateral6x() {
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
    testSimplifyx(path);
}

static void testFauxQuadralateral6a() {
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
    testSimplifyx(path);
}

static void testFauxQuadralateral6ax() {
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
    testSimplifyx(path);
}

static void testFauxQuadralateral6b() {
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
    testSimplifyx(path);
}

static void testFauxQuadralateral6bx() {
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
    testSimplifyx(path);
}

static void testFauxQuadralateral6c() {
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
    testSimplifyx(path);
}

static void testFauxQuadralateral6cx() {
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
    testSimplifyx(path);
}

static void testFauxQuadralateral6d() {
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
    testSimplifyx(path);
}

static void testFauxQuadralateral6dx() {
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
    testSimplifyx(path);
}

static void testQuadralateral6a() {
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
    testSimplifyx(path);
}

static void testQuadralateral6ax() {
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
    testSimplifyx(path);
}

static void testQuadralateral7() {
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
    testSimplifyx(path);
}

static void testQuadralateral7x() {
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
    testSimplifyx(path);
}

static void testQuadralateral8() {
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
    testSimplifyx(path);
}

static void testQuadralateral8x() {
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
    testSimplifyx(path);
}

static void testQuadralateral9() {
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
    testSimplifyx(path);
}

static void testQuadralateral9x() {
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
    testSimplifyx(path);
}

static void testLine1a() {
    SkPath path;
    path.setFillType(SkPath::kWinding_FillType);
    path.addRect(0, 0, 12, 12, SkPath::kCW_Direction);
    path.addRect(4, 0, 13, 13, SkPath::kCCW_Direction);
    testSimplifyx(path);
}

static void testLine1ax() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 12, 12, SkPath::kCW_Direction);
    path.addRect(4, 0, 13, 13, SkPath::kCCW_Direction);
    testSimplifyx(path);
}

static void testLine2ax() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 20, 20, 20, SkPath::kCW_Direction);
    path.addRect(0, 20, 12, 30, SkPath::kCW_Direction);
    path.addRect(12, 0, 21, 21, SkPath::kCCW_Direction);
    testSimplifyx(path);
}

static void testLine3aax() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(10, 30, 30, 30, SkPath::kCW_Direction);
    path.addRect(18, 20, 30, 30, SkPath::kCCW_Direction);
    path.addRect(0, 32, 9, 36, SkPath::kCCW_Direction);
    testSimplifyx(path);
}

static void testLine4ax() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(10, 30, 30, 30, SkPath::kCW_Direction);
    path.addRect(24, 20, 36, 30, SkPath::kCCW_Direction);
    path.addRect(0, 32, 9, 36, SkPath::kCCW_Direction);
    testSimplifyx(path);
}

static void testQuadratic1() {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(0, 0, 0, 0);
    path.lineTo(1, 0);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(0, 0, 0, 0);
    path.close();
    testSimplifyx(path);
}

static void testQuadratic1x() {
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
    testSimplifyx(path);
}

static void testQuadratic2() {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(0, 0, 0, 0);
    path.lineTo(3, 0);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(1, 0, 0, 1);
    path.close();
    testSimplifyx(path);
}

static void testQuadratic2x() {
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
    testSimplifyx(path);
}

static void testQuadratic3() {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(0, 0, 1, 0);
    path.lineTo(0, 2);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(1, 0, 0, 1);
    path.close();
    testSimplifyx(path);
}

static void testQuadratic3x() {
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
    testSimplifyx(path);
}

static void testQuadratic4() {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(0, 0, 1, 0);
    path.lineTo(0, 2);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(1, 0, 0, 2);
    path.close();
    testSimplifyx(path);
}

static void testQuadratic4x() {
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
    testSimplifyx(path);
}

static void testQuadratic5() {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(0, 0, 0, 0);
    path.lineTo(0, 1);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.quadTo(0, 1, 0, 2);
    path.close();
    testSimplifyx(path);
}

static void testQuadratic6() {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(0, 0, 1, 0);
    path.lineTo(2, 1);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(2, 0, 0, 1);
    path.close();
    testSimplifyx(path);
}

static void testQuadratic7() {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(0, 0, 1, 0);
    path.lineTo(3, 1);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(3, 0, 1, 2);
    path.close();
    testSimplifyx(path);
}

static void testQuadratic8() {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(0, 0, 1, 0);
    path.lineTo(0, 2);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.quadTo(0, 1, 1, 2);
    path.close();
    testSimplifyx(path);
}

static void testQuadratic9() {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(0, 0, 1, 0);
    path.lineTo(3, 1);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.quadTo(1, 2, 3, 2);
    path.close();
    testSimplifyx(path);
}

static void testQuadratic14() {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(0, 0, 1, 0);
    path.lineTo(3, 2);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.quadTo(3, 2, 3, 3);
    path.close();
    testSimplifyx(path);
}

static void testQuadratic15() {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(0, 0, 1, 0);
    path.lineTo(1, 3);
    path.close();
    path.moveTo(1, 0);
    path.lineTo(0, 1);
    path.quadTo(1, 1, 0, 3);
    path.close();
    testSimplifyx(path);
}

static void testQuadratic17x() {
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
    testSimplifyx(path);
}

static void testQuadratic18() {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(1, 0, 0, 1);
    path.lineTo(0, 1);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(1, 0, 1, 1);
    path.close();
    testSimplifyx(path);
}

static void testQuadratic19() {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(1, 0, 0, 1);
    path.lineTo(0, 1);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(2, 0, 0, 1);
    path.close();
    testSimplifyx(path);
}

static void testQuadratic20() {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(1, 0, 0, 1);
    path.lineTo(0, 1);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(1, 0, 0, 1);
    path.close();
    testSimplifyx(path);
}

static void testQuadratic21() {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(1, 0, 0, 1);
    path.lineTo(0, 1);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(1, 0, 0, 2);
    path.close();
    testSimplifyx(path);
}

static void testQuadratic22() {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(1, 0, 0, 1);
    path.lineTo(0, 1);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(0, 1, 2, 1);
    path.close();
    testSimplifyx(path);
}

static void testQuadratic23() {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(1, 0, 0, 1);
    path.lineTo(0, 1);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(0, 2, 1, 2);
    path.close();
    testSimplifyx(path);
}

static void testQuadratic24() {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(1, 0, 0, 1);
    path.lineTo(0, 1);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.quadTo(2, 0, 0, 1);
    path.close();
    testSimplifyx(path);
}

static void testQuadratic25() {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(1, 0, 1, 1);
    path.lineTo(1, 1);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(2, 1, 0, 2);
    path.close();
    testSimplifyx(path);
}

static void testQuadratic26() {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(1, 0, 1, 1);
    path.lineTo(0, 2);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(1, 0, 0, 1);
    path.close();
    testSimplifyx(path);
}

static void testQuadratic27() {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(1, 0, 1, 1);
    path.lineTo(2, 1);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(2, 1, 0, 2);
    path.close();
    testSimplifyx(path);
}

static void testQuadratic28() {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(1, 0, 0, 1);
    path.lineTo(0, 1);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 2);
    path.quadTo(1, 2, 0, 3);
    path.close();
    testSimplifyx(path);
}

static void testQuadratic29() {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(1, 0, 2, 1);
    path.lineTo(0, 2);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(1, 0, 0, 1);
    path.close();
    testSimplifyx(path);
}

static void testQuadratic30() {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(1, 0, 1, 2);
    path.lineTo(1, 2);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.quadTo(0, 1, 1, 2);
    path.close();
    testSimplifyx(path);
}

static void testQuadratic31() {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(1, 0, 1, 2);
    path.lineTo(1, 2);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.quadTo(0, 1, 1, 3);
    path.close();
    testSimplifyx(path);
}

static void testQuadratic32() {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(1, 0, 2, 3);
    path.lineTo(2, 3);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(3, 1, 0, 2);
    path.close();
    testSimplifyx(path);
}

static void testQuadratic33() {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(2, 0, 0, 1);
    path.lineTo(0, 1);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(1, 1);
    path.quadTo(2, 1, 2, 2);
    path.close();
    testSimplifyx(path);
}

static void testQuadratic34() {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(2, 0, 0, 1);
    path.lineTo(0, 1);
    path.close();
    path.moveTo(1, 0);
    path.lineTo(1, 1);
    path.quadTo(2, 1, 1, 2);
    path.close();
    testSimplifyx(path);
}

static void testQuadratic35() {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(0, 1, 1, 1);
    path.lineTo(1, 3);
    path.close();
    path.moveTo(2, 0);
    path.lineTo(3, 0);
    path.quadTo(0, 1, 1, 1);
    path.close();
    testSimplifyx(path);
}

static void testQuadratic36() {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(2, 1, 2, 3);
    path.lineTo(2, 3);
    path.close();
    path.moveTo(3, 1);
    path.lineTo(1, 2);
    path.quadTo(3, 2, 1, 3);
    path.close();
    testSimplifyx(path);
}

static void testQuadratic37() {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(0, 2, 1, 2);
    path.lineTo(1, 2);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(3, 1);
    path.quadTo(0, 2, 1, 2);
    path.close();
    testSimplifyx(path);
}

static void testQuadratic38() {
    SkPath path;
    path.moveTo(1, 0);
    path.quadTo(0, 1, 1, 1);
    path.lineTo(1, 1);
    path.close();
    path.moveTo(1, 0);
    path.lineTo(1, 2);
    path.quadTo(2, 2, 1, 3);
    path.close();
    testSimplifyx(path);
}

static void testQuadratic51() {
    SkPath path;
    path.moveTo(369.863983f, 145.645813f);
    path.quadTo(382.380371f, 121.254936f, 406.236359f, 121.254936f);
    path.lineTo(369.863983f, 145.645813f);
    path.close();
    path.moveTo(369.970581f, 137.94342f);
    path.quadTo(383.98465f, 121.254936f, 406.235992f, 121.254936f);
    path.lineTo(369.970581f, 137.94342f);
    path.close();
    testSimplifyx(path);
}

static void testQuadratic53() {
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
    testSimplifyx(path);
}
static void testQuadratic55() {
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
    testSimplifyx(path);
}

static void testQuadratic56() {
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
    testSimplifyx(path);
}

static void testLine80() {
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
    testSimplifyx(path);
}

static void testQuadratic58() {
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
    testSimplifyx(path);
}

static void testQuadratic59x() {
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
    testSimplifyx(path);
}

static void testQuadratic59() {
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
    testSimplifyx(path);
}

static void testQuadratic63() {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(0, 0, 0, 0);
    path.lineTo(3, 2);
    path.close();
    path.moveTo(1, 0);
    path.lineTo(2, 1);
    path.quadTo(2, 1, 2, 2);
    path.close();
    testSimplifyx(path);
}

static void testQuadratic64() {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(0, 0, 0, 0);
    path.lineTo(2, 3);
    path.close();
    path.moveTo(1, 2);
    path.lineTo(2, 2);
    path.quadTo(0, 3, 3, 3);
    path.close();
    testSimplifyx(path);
}

static void testQuadratic65() {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(0, 0, 0, 0);
    path.lineTo(3, 2);
    path.close();
    path.moveTo(2, 1);
    path.lineTo(2, 2);
    path.quadTo(0, 3, 1, 3);
    path.close();
    testSimplifyx(path);
}

static void testQuadratic67x() {
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
    testSimplifyx(path);
}

static void testQuadratic68() {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(1, 0, 0, 1);
    path.lineTo(1, 2);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(0, 1, 2, 1);
    path.close();
    testSimplifyx(path);
}

static void testQuadratic69() {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(0, 0, 0, 1);
    path.lineTo(3, 2);
    path.close();
    path.moveTo(2, 0);
    path.lineTo(1, 1);
    path.quadTo(3, 2, 2, 3);
    path.close();
    testSimplifyx(path);
}

static void testQuadratic70x() {
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
    testSimplifyx(path);
}

static void testQuadratic71() {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(1, 0, 1, 1);
    path.lineTo(3, 2);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(1, 1, 3, 1);
    path.close();
    testSimplifyx(path);
}

static void testQuadratic72() {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(1, 0, 1, 2);
    path.lineTo(1, 2);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.quadTo(0, 1, 3, 2);
    path.close();
    testSimplifyx(path);
}

static void testQuadratic73() {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(1, 0, 0, 3);
    path.lineTo(0, 3);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.quadTo(0, 1, 1, 1);
    path.close();
    testSimplifyx(path);
}

static void testQuadratic74() {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(1, 0, 1, 3);
    path.lineTo(1, 3);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 1);
    path.quadTo(3, 2, 2, 3);
    path.close();
    testSimplifyx(path);
}

static void testQuadratic75() {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(1, 0, 1, 3);
    path.lineTo(2, 3);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 1);
    path.quadTo(3, 2, 2, 3);
    path.close();
    testSimplifyx(path);
}

static void testQuadratic76() {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(0, 0, 0, 0);
    path.lineTo(2, 3);
    path.close();
    path.moveTo(1, 0);
    path.lineTo(1, 2);
    path.quadTo(1, 2, 2, 2);
    path.close();
    testSimplifyx(path);
}

static void testQuadratic77() {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(1, 0, 1, 1);
    path.lineTo(3, 1);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.quadTo(0, 1, 3, 2);
    path.close();
    testSimplifyx(path);
}

static void testQuadratic78() {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(1, 0, 1, 2);
    path.lineTo(3, 2);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(2, 1, 0, 2);
    path.close();
    testSimplifyx(path);
}

static void testQuadratic79() {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(1, 0, 1, 2);
    path.lineTo(3, 2);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.quadTo(0, 1, 3, 2);
    path.close();
    testSimplifyx(path);
}

static void testEight1() {
    SkPath path;
    path.moveTo(0, 0);
    path.lineTo(2, 2);
    path.lineTo(0, 2);
    path.lineTo(2, 0);
    path.close();
    testSimplifyx(path);
}

static void testEight2() {
    SkPath path;
    path.moveTo(0, 0);
    path.lineTo(2, 0);
    path.lineTo(0, 2);
    path.lineTo(2, 2);
    path.close();
    testSimplifyx(path);
}

static void testEight3() {
    SkPath path;
    path.moveTo(0, 0);
    path.lineTo(0, 2);
    path.lineTo(2, 0);
    path.lineTo(2, 2);
    path.close();
    testSimplifyx(path);
}

static void testEight4() {
    SkPath path;
    path.moveTo(0, 0);
    path.lineTo(2, 2);
    path.lineTo(2, 0);
    path.lineTo(0, 2);
    path.close();
    testSimplifyx(path);
}

static void testEight5() {
    SkPath path;
    path.moveTo(1, 0);
    path.lineTo(1, 2);
    path.lineTo(0, 2);
    path.lineTo(2, 0);
    path.close();
    testSimplifyx(path);
}

static void testEight6() {
    SkPath path;
    path.moveTo(1, 0);
    path.lineTo(2, 0);
    path.lineTo(0, 2);
    path.lineTo(1, 2);
    path.close();
    testSimplifyx(path);
}

static void testEight7() {
    SkPath path;
    path.moveTo(0, 0);
    path.lineTo(0, 1);
    path.lineTo(2, 1);
    path.lineTo(2, 2);
    path.close();
    testSimplifyx(path);
}

static void testEight8() {
    SkPath path;
    path.moveTo(0, 0);
    path.lineTo(2, 2);
    path.lineTo(2, 1);
    path.lineTo(0, 1);
    path.close();
    testSimplifyx(path);
}

static void testEight9() {
    SkPath path;
    path.moveTo(1, 0);
    path.lineTo(1, 2);
    path.lineTo(2, 1);
    path.lineTo(0, 1);
    path.close();
    testSimplifyx(path);
}

static void testEight10() {
    SkPath path;
    path.moveTo(1, 0);
    path.lineTo(0, 1);
    path.lineTo(2, 1);
    path.lineTo(1, 2);
    path.close();
    testSimplifyx(path);
}

static void testQuadratic80() {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(1, 0, 2, 3);
    path.lineTo(2, 3);
    path.close();
    path.moveTo(1, 0);
    path.lineTo(3, 0);
    path.quadTo(0, 1, 1, 1);
    path.close();
    testSimplifyx(path);
}

static void testQuadratic81() {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(2, 0, 1, 1);
    path.lineTo(1, 1);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(2, 1, 0, 2);
    path.close();
    testSimplifyx(path);
}

static void testQuadratic82() {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(2, 0, 1, 1);
    path.lineTo(0, 3);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(2, 1, 0, 2);
    path.close();
    testSimplifyx(path);
}

static void testQuadratic83() {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(0, 0, 2, 0);
    path.lineTo(2, 2);
    path.close();
    path.moveTo(0, 1);
    path.lineTo(0, 2);
    path.quadTo(2, 2, 1, 3);
    path.close();
    testSimplifyx(path);
}

static void testQuadratic84() {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(2, 0, 1, 1);
    path.lineTo(2, 1);
    path.close();
    path.moveTo(1, 0);
    path.lineTo(2, 0);
    path.quadTo(0, 1, 2, 2);
    path.close();
    testSimplifyx(path);
}

static void testQuadratic85() {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(3, 0, 1, 1);
    path.lineTo(1, 1);
    path.close();
    path.moveTo(1, 0);
    path.lineTo(3, 0);
    path.quadTo(0, 1, 1, 2);
    path.close();
    testSimplifyx(path);
}

static void testQuadratic86() {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(0, 1, 1, 1);
    path.lineTo(2, 3);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(1, 1, 1, 3);
    path.close();
    testSimplifyx(path);
}

static void testQuadratic87() {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(2, 1, 0, 2);
    path.lineTo(2, 3);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(1, 1);
    path.quadTo(0, 2, 3, 2);
    path.close();
    testSimplifyx(path);
}

static void testQuadratic88() {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(2, 1, 0, 2);
    path.lineTo(2, 2);
    path.close();
    path.moveTo(1, 0);
    path.lineTo(1, 1);
    path.quadTo(0, 2, 2, 2);
    path.close();
    testSimplifyx(path);
}

static void testQuadratic89x() {
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
    testSimplifyx(path);
}

static void testQuadratic90x() {
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
    testSimplifyx(path);
}

static void testQuadratic91() {
    SkPath path;
    path.moveTo(0, 0);
    path.quadTo(3, 2, 2, 3);
    path.lineTo(2, 3);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(1, 1);
    path.quadTo(2, 1, 2, 3);
    path.close();
    testSimplifyx(path);
}

static void testQuadratic92x() {
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
    testSimplifyx(path);
}

static void testLine82() {
    SkPath path;
    path.addRect(20, 0, 40, 40, SkPath::kCCW_Direction);
    path.addRect(24, 20, 36, 30, SkPath::kCCW_Direction);
    path.addRect(24, 32, 33, 36, SkPath::kCCW_Direction);
    testSimplifyx(path);
}

static void testLine82a() {
    SkPath path;
    path.addRect(0, 0, 6, 10, SkPath::kCW_Direction);
    path.addRect(2, 2, 4, 4, SkPath::kCW_Direction);
    path.addRect(2, 6, 4, 8, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine82b() {
    SkPath path;
    path.addRect(0, 0, 6, 10, SkPath::kCW_Direction);
    path.addRect(2, 2, 4, 4, SkPath::kCW_Direction);
    path.addRect(2, 6, 4, 8, SkPath::kCCW_Direction);
    testSimplifyx(path);
}

static void testLine82c() {
    SkPath path;
    path.addRect(0, 0, 6, 10, SkPath::kCW_Direction);
    path.addRect(2, 2, 4, 4, SkPath::kCCW_Direction);
    path.addRect(2, 6, 4, 8, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine82d() {
    SkPath path;
    path.addRect(0, 0, 6, 10, SkPath::kCW_Direction);
    path.addRect(2, 2, 4, 4, SkPath::kCCW_Direction);
    path.addRect(2, 6, 4, 8, SkPath::kCCW_Direction);
    testSimplifyx(path);
}

static void testLine82e() {
    SkPath path;
    path.addRect(0, 0, 6, 10, SkPath::kCCW_Direction);
    path.addRect(2, 2, 4, 4, SkPath::kCW_Direction);
    path.addRect(2, 6, 4, 8, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine82f() {
    SkPath path;
    path.addRect(0, 0, 6, 10, SkPath::kCCW_Direction);
    path.addRect(2, 2, 4, 4, SkPath::kCW_Direction);
    path.addRect(2, 6, 4, 8, SkPath::kCCW_Direction);
    testSimplifyx(path);
}

static void testLine82g() {
    SkPath path;
    path.addRect(0, 0, 6, 10, SkPath::kCCW_Direction);
    path.addRect(2, 2, 4, 4, SkPath::kCCW_Direction);
    path.addRect(2, 6, 4, 8, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine82h() {
    SkPath path;
    path.addRect(0, 0, 6, 10, SkPath::kCCW_Direction);
    path.addRect(2, 2, 4, 4, SkPath::kCCW_Direction);
    path.addRect(2, 6, 4, 8, SkPath::kCCW_Direction);
    testSimplifyx(path);
}

static void testLine83() {
    SkPath path;
path.addRect(10, 30, 30, 40, SkPath::kCCW_Direction);
path.addRect(0, 12, 12, 18, SkPath::kCCW_Direction);
path.addRect(4, 13, 13, 16, SkPath::kCCW_Direction);
    testSimplifyx(path);
}

static void testLine84() {
    SkPath path;
    path.addRect(0, 12, 60, 30, SkPath::kCCW_Direction);
    path.addRect(10, 20, 40, 30, SkPath::kCW_Direction);
    path.addRect(0, 12, 12, 12, SkPath::kCW_Direction);
    path.addRect(4, 12, 13, 13, SkPath::kCW_Direction);
    testSimplifyx(path);
}

static void testLine84x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 12, 60, 30, SkPath::kCCW_Direction);
    path.addRect(10, 20, 40, 30, SkPath::kCCW_Direction);
    path.addRect(0, 12, 12, 12, SkPath::kCCW_Direction);
    path.addRect(4, 12, 13, 13, SkPath::kCCW_Direction);
    testSimplifyx(path);
}

static void testLine85() {
    SkPath path;
    path.addRect(36, 0, 66, 60, SkPath::kCCW_Direction);
    path.addRect(20, 0, 40, 40, SkPath::kCCW_Direction);
    path.addRect(12, 0, 24, 24, SkPath::kCCW_Direction);
    path.addRect(32, 0, 36, 41, SkPath::kCCW_Direction);
    testSimplifyx(path);
}

static void testQuadralateral1() {
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
    testSimplifyx(path);
}

static void testCubic1() {
    SkPath path;
    path.moveTo(0, 0);
    path.cubicTo(0, 1, 1, 1, 1, 0);
    path.close();
    path.moveTo(1, 0);
    path.cubicTo(0, 0, 0, 1, 1, 1);
    path.close();
    testSimplifyx(path);
}

static void testQuadratic93() {
    SkPath path;
    path.moveTo(3, 0);
    path.quadTo(0, 1, 3, 2);
    path.lineTo(0, 3);
    path.close();
    path.moveTo(1, 0);
    path.lineTo(2, 0);
    path.quadTo(1, 1, 2, 2);
    path.close();
    testSimplifyx(path);
}

static void cubicOp1d() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(0,2, 1,0, 1,0);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,1);
    pathB.cubicTo(0,1, 1,0, 2,0);
    pathB.close();
    testShapeOp(path, pathB, kDifference_Op);
}

static void cubicOp2d() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,2);
    path.cubicTo(0,1, 1,0, 1,0);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,1);
    pathB.cubicTo(0,1, 2,0, 1,0);
    pathB.close();
    testShapeOp(path, pathB, kDifference_Op);
}

static void cubicOp3d() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(2,3, 1,0, 1,0);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,1);
    pathB.cubicTo(0,1, 1,0, 3,2);
    pathB.close();
    testShapeOp(path, pathB, kDifference_Op);
}

static void cubicOp5d() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(0,2, 1,0, 2,0);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,1);
    pathB.cubicTo(0,2, 1,0, 2,0);
    pathB.close();
    testShapeOp(path, pathB, kDifference_Op);
}

static void cubicOp6d() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(0,6, 1,0, 3,0);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,1);
    pathB.cubicTo(0,3, 1,0, 6,0);
    pathB.close();
    testShapeOp(path, pathB, kDifference_Op);
}

static void cubicOp7d() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(3,4, 1,0, 3,0);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,1);
    pathB.cubicTo(0,3, 1,0, 4,3);
    pathB.close();
    testShapeOp(path, pathB, kDifference_Op);
}

static void cubicOp8d() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(0,5, 1,0, 4,0);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,1);
    pathB.cubicTo(0,4, 1,0, 5,0);
    pathB.close();
    testShapeOp(path, pathB, kDifference_Op);
}

static void cubicOp9d() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(1,6, 1,0, 2,1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,1);
    pathB.cubicTo(1,2, 1,0, 6,1);
    pathB.close();
    testShapeOp(path, pathB, kDifference_Op);
}

static void quadOp9d() {
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
    testShapeOp(path, pathB, kDifference_Op);
}

static void lineOp9d() {
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
    testShapeOp(path, pathB, kDifference_Op);
}

static void cubicOp1i() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(1,2, 1,0, 2,1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,1);
    pathB.cubicTo(1,2, 1,0, 2,1);
    pathB.close();
    testShapeOp(path, pathB, kIntersect_Op);
}

static void cubicOp10d() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(1,3, 1,0, 4,1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,1);
    pathB.cubicTo(1,4, 1,0, 3,1);
    pathB.close();
    testShapeOp(path, pathB, kDifference_Op);
}

static void cubicOp11d() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(3,4, 1,0, 5,1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,1);
    pathB.cubicTo(1,5, 1,0, 4,3);
    pathB.close();
    testShapeOp(path, pathB, kDifference_Op);
}

static void cubicOp12d() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(1,6, 1,0, 1,0);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,1);
    pathB.cubicTo(0,1, 1,0, 6,1);
    pathB.close();
    testShapeOp(path, pathB, kDifference_Op);
}

static void cubicOp13d() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(4,5, 1,0, 5,3);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,1);
    pathB.cubicTo(3,5, 1,0, 5,4);
    pathB.close();
    testShapeOp(path, pathB, kDifference_Op);
}

static void cubicOp14d() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(0,2, 2,0, 2,1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,2);
    pathB.cubicTo(1,2, 1,0, 2,0);
    pathB.close();
    testShapeOp(path, pathB, kDifference_Op);
}

static void cubicOp15d() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(3,6, 2,0, 2,1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,2);
    pathB.cubicTo(1,2, 1,0, 6,3);
    pathB.close();
    testShapeOp(path, pathB, kDifference_Op);
}

static void cubicOp16d() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,2);
    path.cubicTo(0,1, 3,0, 1,0);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,3);
    pathB.cubicTo(0,1, 2,0, 1,0);
    pathB.close();
    testShapeOp(path, pathB, kDifference_Op);
}

static void cubicOp17d() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,2);
    path.cubicTo(0,2, 4,0, 2,1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,4);
    pathB.cubicTo(1,2, 2,0, 2,0);
    pathB.close();
    testShapeOp(path, pathB, kDifference_Op);
}

static void cubicOp18d() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(3,5, 2,0, 2,1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,2);
    pathB.cubicTo(1,2, 1,0, 5,3);
    pathB.close();
    testShapeOp(path, pathB, kDifference_Op);
}

static void cubicOp19i() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,2);
    path.cubicTo(0,1, 2,1, 6,2);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1,2);
    pathB.cubicTo(2,6, 2,0, 1,0);
    pathB.close();
    testShapeOp(path, pathB, kIntersect_Op);
}

static void cubicOp20d() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(0,1, 6,0, 2,1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,6);
    pathB.cubicTo(1,2, 1,0, 1,0);
    pathB.close();
    testShapeOp(path, pathB, kDifference_Op);
}

static void cubicOp21d() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(0,1, 2,1, 6,5);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1,2);
    pathB.cubicTo(5,6, 1,0, 1,0);
    pathB.close();
    testShapeOp(path, pathB, kDifference_Op);
}

static void cubicOp22d() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(2,3, 3,0, 2,1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,3);
    pathB.cubicTo(1,2, 1,0, 3,2);
    pathB.close();
    testShapeOp(path, pathB, kDifference_Op);
}

static void cubicOp23d() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(1,2, 4,0, 2,1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,4);
    pathB.cubicTo(1,2, 1,0, 2,1);
    pathB.close();
    testShapeOp(path, pathB, kDifference_Op);
}

static void cubicOp24d() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(1,2, 2,0, 3,2);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,2);
    pathB.cubicTo(2,3, 1,0, 2,1);
    pathB.close();
    testShapeOp(path, pathB, kDifference_Op);
}

static void testIntersect1() {
    SkPath one, two;
    one.addRect(0, 0, 6, 6, SkPath::kCW_Direction);
    two.addRect(3, 3, 9, 9, SkPath::kCW_Direction);
    testShapeOp(one, two, kIntersect_Op);
}

static void testUnion1() {
    SkPath one, two;
    one.addRect(0, 0, 6, 6, SkPath::kCW_Direction);
    two.addRect(3, 3, 9, 9, SkPath::kCW_Direction);
    testShapeOp(one, two, kUnion_Op);
}

static void testDiff1() {
    SkPath one, two;
    one.addRect(0, 0, 6, 6, SkPath::kCW_Direction);
    two.addRect(3, 3, 9, 9, SkPath::kCW_Direction);
    testShapeOp(one, two, kDifference_Op);
}

static void testXor1() {
    SkPath one, two;
    one.addRect(0, 0, 6, 6, SkPath::kCW_Direction);
    two.addRect(3, 3, 9, 9, SkPath::kCW_Direction);
    testShapeOp(one, two, kXor_Op);
}

static void testIntersect2() {
    SkPath one, two;
    one.addRect(0, 0, 6, 6, SkPath::kCW_Direction);
    two.addRect(0, 3, 9, 9, SkPath::kCW_Direction);
    testShapeOp(one, two, kIntersect_Op);
}

static void testUnion2() {
    SkPath one, two;
    one.addRect(0, 0, 6, 6, SkPath::kCW_Direction);
    two.addRect(0, 3, 9, 9, SkPath::kCW_Direction);
    testShapeOp(one, two, kUnion_Op);
}

static void testDiff2() {
    SkPath one, two;
    one.addRect(0, 0, 6, 6, SkPath::kCW_Direction);
    two.addRect(0, 3, 9, 9, SkPath::kCW_Direction);
    testShapeOp(one, two, kDifference_Op);
}

static void testXor2() {
    SkPath one, two;
    one.addRect(0, 0, 6, 6, SkPath::kCW_Direction);
    two.addRect(0, 3, 9, 9, SkPath::kCW_Direction);
    testShapeOp(one, two, kXor_Op);
}

static void testOp1d() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    path.addRect(0, 0, 2, 2, SkPath::kCW_Direction);
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    pathB.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    testShapeOp(path, pathB, kDifference_Op);
}

static void testOp2d() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    path.addRect(0, 0, 2, 2, SkPath::kCW_Direction);
    pathB.setFillType(SkPath::kEvenOdd_FillType);
    pathB.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    pathB.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    testShapeOp(path, pathB, kDifference_Op);
}

static void testOp3d() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    path.addRect(1, 1, 2, 2, SkPath::kCW_Direction);
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    pathB.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    testShapeOp(path, pathB, kDifference_Op);
}

static void testOp1u() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    path.addRect(0, 0, 3, 3, SkPath::kCW_Direction);
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    pathB.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    testShapeOp(path, pathB, kUnion_Op);
}

static void testOp4d() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    path.addRect(2, 2, 4, 4, SkPath::kCW_Direction);
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    pathB.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    testShapeOp(path, pathB, kDifference_Op);
}

static void testOp5d() {
    SkPath path, pathB;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 2, 2, SkPath::kCW_Direction);
    path.addRect(0, 0, 3, 3, SkPath::kCW_Direction);
    pathB.setFillType(SkPath::kEvenOdd_FillType);
    pathB.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    pathB.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    testShapeOp(path, pathB, kDifference_Op);
}

static void testOp6d() {
    SkPath path, pathB;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    path.addRect(0, 0, 3, 3, SkPath::kCW_Direction);
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    pathB.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    testShapeOp(path, pathB, kDifference_Op);
}

static void testOp7d() {
    SkPath path, pathB;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 2, 2, SkPath::kCW_Direction);
    path.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    pathB.setFillType(SkPath::kEvenOdd_FillType);
    pathB.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    pathB.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    testShapeOp(path, pathB, kDifference_Op);
}

static void testOp2u() {
    SkPath path, pathB;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 2, 2, SkPath::kCW_Direction);
    path.addRect(0, 0, 2, 2, SkPath::kCW_Direction);
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.addRect(0, 0, 3, 3, SkPath::kCW_Direction);
    pathB.addRect(1, 1, 2, 2, SkPath::kCW_Direction);
    testShapeOp(path, pathB, kUnion_Op);
}

static void testOp8d() {
    SkPath path, pathB;
    path.addRect(0, 0, 640, 480);
    pathB.moveTo(577330, 1971.72f);
    pathB.cubicTo(10.7082f, -116.596f, 262.057f, 45.6468f, 294.694f, 1.96237f);
    pathB.close();
    testShapeOp(path, pathB, kDifference_Op);
}
static void cubicOp25i() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(2,4, 5,0, 3,2);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,5);
    pathB.cubicTo(2,3, 1,0, 4,2);
    pathB.close();
    testShapeOp(path, pathB, kIntersect_Op);
}

static void cubicOp26d() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(3,4, 4,0, 3,2);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,4);
    pathB.cubicTo(2,3, 1,0, 4,3);
    pathB.close();
    testShapeOp(path, pathB, kDifference_Op);
}

static void cubicOp27d() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(3,6, 1,0, 5,2);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,1);
    pathB.cubicTo(2,5, 1,0, 6,3);
    pathB.close();
    testShapeOp(path, pathB, kDifference_Op);
}

static void cubicOp28u() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(1,4, 6,0, 3,2);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,6);
    pathB.cubicTo(2,3, 1,0, 4,1);
    pathB.close();
    testShapeOp(path, pathB, kUnion_Op);
}

static void cubicOp29d() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(2,5, 6,0, 4,2);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,6);
    pathB.cubicTo(2,4, 1,0, 5,2);
    pathB.close();
    testShapeOp(path, pathB, kDifference_Op);
}

static void cubicOp30d() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(2,5, 6,0, 5,3);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,6);
    pathB.cubicTo(3,5, 1,0, 5,2);
    pathB.close();
    testShapeOp(path, pathB, kDifference_Op);
}

static void cubicOp31d() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,2);
    path.cubicTo(0,3, 2,1, 4,0);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1,2);
    pathB.cubicTo(0,4, 2,0, 3,0);
    pathB.close();
    testShapeOp(path, pathB, kDifference_Op);
}

static void cubicOp31u() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,2);
    path.cubicTo(0,3, 2,1, 4,0);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1,2);
    pathB.cubicTo(0,4, 2,0, 3,0);
    pathB.close();
    testShapeOp(path, pathB, kUnion_Op);
}

static void cubicOp31x() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,2);
    path.cubicTo(0,3, 2,1, 4,0);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1,2);
    pathB.cubicTo(0,4, 2,0, 3,0);
    pathB.close();
    testShapeOp(path, pathB, kXor_Op);
}

static void testCubic2() {
    SkPath path;
    path.moveTo(0,2);
    path.cubicTo(0,3, 2,1, 4,0);
    path.close();
    path.moveTo(1,2);
    path.cubicTo(0,4, 2,0, 3,0);
    path.close();
    testSimplifyx(path);
}

static void cubicOp32d() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(1,2, 6,0, 3,1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,6);
    pathB.cubicTo(1,3, 1,0, 2,1);
    pathB.close();
    testShapeOp(path, pathB, kDifference_Op);
}

static void cubicOp33i() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(1,2, 6,0, 3,1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,6);
    pathB.cubicTo(1,3, 1,0, 2,1);
    pathB.close();
    testShapeOp(path, pathB, kIntersect_Op);
}

static void cubicOp34d() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(3,5, 2,1, 3,1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1,2);
    pathB.cubicTo(1,3, 1,0, 5,3);
    pathB.close();
    testShapeOp(path, pathB, kDifference_Op);
}

static void cubicOp35d() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(1,5, 2,1, 4,0);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1,2);
    pathB.cubicTo(0,4, 1,0, 5,1);
    pathB.close();
    testShapeOp(path, pathB, kDifference_Op);
}

static void cubicOp36u() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(1,6, 2,0, 5,1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,2);
    pathB.cubicTo(1,5, 1,0, 6,1);
    pathB.close();
    testShapeOp(path, pathB, kUnion_Op);
}

static void cubicOp37d() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(2,6, 6,1, 4,3);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1,6);
    pathB.cubicTo(3,4, 1,0, 6,2);
    pathB.close();
    testShapeOp(path, pathB, kDifference_Op);
}

#if 1
// this fails to detect a cubic/cubic intersection
// the slight overlap is missed when the cubics are approximated by quadratics
// and the subsequent line/cubic intersection also (correctly) misses the intersection
// if the line/cubic was a matching line/approx.quadratic then the missing intersection
// could have been detected
static void cubicOp38d() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(0,6, 3,2, 4,1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(2,3);
    pathB.cubicTo(1,4, 1,0, 6,0);
    pathB.close();
    testShapeOp(path, pathB, kDifference_Op);
}
#endif

static void cubicOp39d() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(2,3, 5,1, 4,3);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1,5);
    pathB.cubicTo(3,4, 1,0, 3,2);
    pathB.close();
    testShapeOp(path, pathB, kDifference_Op);
}

static void cubicOp40d() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(1,5, 3,2, 4,2);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(2,3);
    pathB.cubicTo(2,4, 1,0, 5,1);
    pathB.close();
    testShapeOp(path, pathB, kDifference_Op);
}

static void cubicOp41i() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(2,6, 4,3, 6,4);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(3,4);
    pathB.cubicTo(4,6, 1,0, 6,2);
    pathB.close();
    testShapeOp(path, pathB, kIntersect_Op);
}

static void cubicOp42d() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(1,2, 6,5, 5,4);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(5,6);
    pathB.cubicTo(4,5, 1,0, 2,1);
    pathB.close();
    testShapeOp(path, pathB, kDifference_Op);
}

static void cubicOp43d() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,2);
    path.cubicTo(1,2, 4,0, 3,1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,4);
    pathB.cubicTo(1,3, 2,0, 2,1);
    pathB.close();
    testShapeOp(path, pathB, kDifference_Op);
}

static void cubicOp44d() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,2);
    path.cubicTo(3,6, 4,0, 3,2);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,4);
    pathB.cubicTo(2,3, 2,0, 6,3);
    pathB.close();
    testShapeOp(path, pathB, kDifference_Op);
}

static void cubicOp45d() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,2);
    path.cubicTo(2,4, 4,0, 3,2);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,4);
    pathB.cubicTo(2,3, 2,0, 4,2);
    pathB.close();
    testShapeOp(path, pathB, kDifference_Op);
}

static void cubicOp46d() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,2);
    path.cubicTo(3,5, 5,0, 4,2);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,5);
    pathB.cubicTo(2,4, 2,0, 5,3);
    pathB.close();
    testShapeOp(path, pathB, kDifference_Op);
}

static void cubicOp47d() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(1,6, 6,2, 5,4);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(2,6);
    pathB.cubicTo(4,5, 1,0, 6,1);
    pathB.close();
    testShapeOp(path, pathB, kDifference_Op);
}

static void cubicOp48d() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,2);
    path.cubicTo(2,3, 5,1, 3,2);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1,5);
    pathB.cubicTo(2,3, 2,0, 3,2);
    pathB.close();
    testShapeOp(path, pathB, kDifference_Op);
}

static void cubicOp49d() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,2);
    path.cubicTo(1,5, 3,2, 4,1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(2,3);
    pathB.cubicTo(1,4, 2,0, 5,1);
    pathB.close();
    testShapeOp(path, pathB, kDifference_Op);
}

static void cubicOp50d() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,3);
    path.cubicTo(1,6, 5,0, 5,1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,5);
    pathB.cubicTo(1,5, 3,0, 6,1);
    pathB.close();
    testShapeOp(path, pathB, kDifference_Op);
}

static void cubicOp51d() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,3);
    path.cubicTo(1,2, 4,1, 6,0);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(1,4);
    pathB.cubicTo(0,6, 3,0, 2,1);
    pathB.close();
    testShapeOp(path, pathB, kDifference_Op);
}

static void cubicOp52d() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,2);
    path.cubicTo(1,2, 5,4, 4,3);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(4,5);
    pathB.cubicTo(3,4, 2,0, 2,1);
    pathB.close();
    testShapeOp(path, pathB, kDifference_Op);
}

static void cubicOp53d() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,3);
    path.cubicTo(1,2, 5,3, 2,1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(3,5);
    pathB.cubicTo(1,2, 3,0, 2,1);
    pathB.close();
    testShapeOp(path, pathB, kDifference_Op);
}

static void cubicOp54d() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,4);
    path.cubicTo(1,3, 5,4, 4,2);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(4,5);
    pathB.cubicTo(2,4, 4,0, 3,1);
    pathB.close();
    testShapeOp(path, pathB, kDifference_Op);
}

static void cubicOp55d() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,5);
    path.cubicTo(1,3, 3,2, 5,0);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(2,3);
    pathB.cubicTo(0,5, 5,0, 3,1);
    pathB.close();
    testShapeOp(path, pathB, kDifference_Op);
}

static void cubicOp56d() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(2,6, 5,0, 2,1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,5);
    pathB.cubicTo(1,2, 1,0, 6,2);
    pathB.close();
    testShapeOp(path, pathB, kDifference_Op);
}

static void cubicOp57d() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,5);
    path.cubicTo(0,5, 5,4, 6,4);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(4,5);
    pathB.cubicTo(4,6, 5,0, 5,0);
    pathB.close();
    testShapeOp(path, pathB, kDifference_Op);
}

static void cubicOp58d() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,5);
    path.cubicTo(3,4, 6,5, 5,3);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(5,6);
    pathB.cubicTo(3,5, 5,0, 4,3);
    pathB.close();
    testShapeOp(path, pathB, kDifference_Op);
}

static void cubicOp59d() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,1);
    path.cubicTo(5,6, 4,0, 4,1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,4);
    pathB.cubicTo(1,4, 1,0, 6,5);
    pathB.close();
    testShapeOp(path, pathB, kDifference_Op);
}

static void cubicOp60d() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(0,2);
    path.cubicTo(4,6, 6,0, 5,2);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(0,6);
    pathB.cubicTo(2,5, 2,0, 6,4);
    pathB.close();
    testShapeOp(path, pathB, kDifference_Op);
}

static void cubicOp61d() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(1,2);
    path.cubicTo(0,5, 3,2, 6,1);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(2,3);
    pathB.cubicTo(1,6, 2,1, 5,0);
    pathB.close();
    testShapeOp(path, pathB, kDifference_Op);
}

static void cubicOp62d() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(1,3);
    path.cubicTo(5,6, 5,3, 5,4);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(3,5);
    pathB.cubicTo(4,5, 3,1, 6,5);
    pathB.close();
    testShapeOp(path, pathB, kDifference_Op);
}

static void cubicOp63d() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(2,3);
    path.cubicTo(0,4, 3,2, 5,3);
    path.close();
    pathB.setFillType(SkPath::kWinding_FillType);
    pathB.moveTo(2,3);
    pathB.cubicTo(3,5, 3,2, 4,0);
    pathB.close();
    testShapeOp(path, pathB, kDifference_Op);
}

static void testQuad1() {
    SkPath path;
    path.moveTo(0,0);
    path.quadTo(0,0, 0,1);
    path.lineTo(1,1);
    path.close();
    path.moveTo(0,0);
    path.quadTo(1,1, 0,2);
    path.close();
    testSimplifyx(path);
}

static void testQuad1z() {
    SkPath path;
    path.moveTo(0,0);
    path.quadTo(0,0, 0,1);
    path.lineTo(1,1);
    path.close();
    path.moveTo(0,0);
    path.lineTo(0.5f,1);
    path.lineTo(0,2);
    path.close();
    testSimplifyx(path);
}

static void testQuad1c() {
    SkPath path;
    path.moveTo(0,0);
    path.quadTo(0,0, 0,1);
    path.lineTo(1,1);
    path.close();
    path.moveTo(0,0);
    path.cubicTo(0.5f,0.5f, 0.5f,1.5f, 0,2);
    path.close();
    testSimplifyx(path);
}

static void testRectOp1d() {
    SkPath path, pathB;
    path.moveTo(0,1);
    path.cubicTo(0,1, 1,0, 3,0);
    path.lineTo(0,1);
    path.close();
    pathB.moveTo(0,1);
    pathB.cubicTo(0,3, 1,0, 1,0);
    pathB.lineTo(0,1);
    pathB.close();
    testShapeOp(path, pathB, kDifference_Op);
}

static void testCubicOp64d() {
    SkPath path, pathB;
    path.setFillType(SkPath::kWinding_FillType);
    path.addRect(0, 0, 1, 1, SkPath::kCW_Direction);
    path.addRect(2, 2, 3, 3, SkPath::kCW_Direction);
    pathB.setFillType(SkPath::kEvenOdd_FillType);
    pathB.addRect(0, 0, 4, 4, SkPath::kCW_Direction);
    pathB.addRect(0, 0, 3, 3, SkPath::kCW_Direction);
    testShapeOp(path, pathB, kDifference_Op);
}

static void (*firstTest)() = testRectOp1d;

static struct {
    void (*fun)();
    const char* str;
} tests[] = {
    TEST(testRectOp1d),
    TEST(testCubicOp64d),
    TEST(testQuad1c),
    TEST(testQuad1z),
    TEST(testQuad1),
    TEST(testQuadratic93),    // FIXME: gets stuck in a loop because top is unsortable
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
    TEST(testCubic2),
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
    TEST(testQuadratic56),
    TEST(testQuadratic55),
    TEST(testQuadratic53),
    TEST(testQuadratic51),
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

static const size_t testCount = sizeof(tests) / sizeof(tests[0]);


static struct {
    void (*fun)();
    const char* str;
} subTests[] = {
    TEST(cubicOp43d),
    TEST(quadOp9d),
    TEST(cubicOp9d),
    TEST(cubicOp1i),
    TEST(cubicOp10d),
    TEST(cubicOp11d),
    TEST(cubicOp15d),
    TEST(cubicOp18d),
    TEST(cubicOp22d),
    TEST(cubicOp23d),
    TEST(cubicOp24d),
    TEST(cubicOp28u),
    TEST(cubicOp33i),
    TEST(cubicOp36u),
    TEST(cubicOp40d),
};

static const size_t subTestCount = sizeof(subTests) / sizeof(subTests[0]);

static void (*firstSubTest)() = 0;

static bool skipAll = false;
static bool runSubTestsFirst = false;
static bool runReverse = false;
static void (*stopTest)() = 0;

void SimplifyNew_Test() {
    if (skipAll) {
        return;
    }
#ifdef SK_DEBUG
    gDebugMaxWindSum = 4;
    gDebugMaxWindValue = 4;
#endif
    size_t index;
    if (runSubTestsFirst && firstSubTest) {
        index = subTestCount - 1;
        while (index > 0 && subTests[index].fun != firstSubTest) {
            --index;
        }
        SkDebugf("  %s [%s]\n", __FUNCTION__, subTests[index].str);
        (*subTests[index].fun)();
    }
    if (runSubTestsFirst) {
        index = subTestCount - 1;
        do {
            SkDebugf("  %s [%s]\n", __FUNCTION__, subTests[index].str);
            (*subTests[index].fun)();
        } while (index--);
    }
    index = testCount - 1;
    if (firstTest) {
        while (index > 0 && tests[index].fun != firstTest) {
            --index;
        }
    #if FORCE_RELEASE == 0
        SkDebugf("<div id=\"%s\">\n", tests[index].str);
    #endif
        SkDebugf("  %s [%s]\n", __FUNCTION__, tests[index].str);
        (*tests[index].fun)();
    }
    index = runReverse ? testCount - 1 : 0;
    size_t last = runReverse ? 0 : testCount - 1;
    bool firstTestComplete = false;
    do {
        SkDebugf("  %s [%s]\n", __FUNCTION__, tests[index].str);
        (*tests[index].fun)();
        firstTestComplete = true;
        if (tests[index].fun == stopTest) {
            SkDebugf("lastTest\n");
        }
        if (index == last) {
            break;
        }
        index += runReverse ? -1 : 1;
    } while (true);
    if (!runSubTestsFirst) {
        index = subTestCount - 1;
        do {
            SkDebugf("  %s [%s]\n", __FUNCTION__, subTests[index].str);
            (*subTests[index].fun)();
        } while (index--);
    }
#ifdef SK_DEBUG
    gDebugMaxWindSum = SK_MaxS32;
    gDebugMaxWindValue = SK_MaxS32;
#endif
}
