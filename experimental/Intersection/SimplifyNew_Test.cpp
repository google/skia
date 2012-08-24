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
    path.addRect(0, 0, 9, 9, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine15x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 9, 9, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine16() {
    SkPath path;
    path.addRect(0, 0, 12, 12, (SkPath::Direction) 0);
    path.addRect(0, 4, 9, 9, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine16x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 12, 12, (SkPath::Direction) 0);
    path.addRect(0, 4, 9, 9, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine17() {
    SkPath path;
    path.addRect(0, 0, 12, 12, (SkPath::Direction) 0);
    path.addRect(4, 12, 13, 13, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine17x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 12, 12, (SkPath::Direction) 0);
    path.addRect(4, 12, 13, 13, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine18() {
    SkPath path;
    path.addRect(0, 0, 12, 12, (SkPath::Direction) 0);
    path.addRect(12, 4, 21, 21, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine18x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 12, 12, (SkPath::Direction) 0);
    path.addRect(12, 4, 21, 21, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine19() {
    SkPath path;
    path.addRect(0, 0, 12, 12, (SkPath::Direction) 0);
    path.addRect(12, 16, 21, 21, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine19x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 12, 12, (SkPath::Direction) 0);
    path.addRect(12, 16, 21, 21, (SkPath::Direction) 0);    
    testSimplifyx(path);
}

static void testLine20() {
    SkPath path;
    path.addRect(0, 12, 12, 12, (SkPath::Direction) 0);
    path.addRect(0, 12, 9, 9, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine20x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 12, 12, 12, (SkPath::Direction) 0);
    path.addRect(0, 12, 9, 9, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine21() {
    SkPath path;
    path.addRect(0, 12, 12, 12, (SkPath::Direction) 0);
    path.addRect(0, 16, 9, 9, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine21x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 12, 12, 12, (SkPath::Direction) 0);
    path.addRect(0, 16, 9, 9, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine22() {
    SkPath path;
    path.addRect(0, 12, 12, 12, (SkPath::Direction) 0);
    path.addRect(4, 12, 13, 13, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine22x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 12, 12, 12, (SkPath::Direction) 0);
    path.addRect(4, 12, 13, 13, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine23() {
    SkPath path;
    path.addRect(0, 12, 12, 12, (SkPath::Direction) 0);
    path.addRect(12, 0, 21, 21, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine23x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 12, 12, 12, (SkPath::Direction) 0);
    path.addRect(12, 0, 21, 21, (SkPath::Direction) 0);
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
    path.addRect(0, 18, 12, 12, (SkPath::Direction) 0);
    path.addRect(4, 12, 13, 13, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine24x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 18, 12, 12, (SkPath::Direction) 0);
    path.addRect(4, 12, 13, 13, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine25() {
    SkPath path;
    path.addRect(0, 6, 12, 12, (SkPath::Direction) 0);
    path.addRect(12, 0, 21, 21, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine25x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 6, 12, 12, (SkPath::Direction) 0);
    path.addRect(12, 0, 21, 21, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine26() {
    SkPath path;
    path.addRect(0, 18, 12, 12, (SkPath::Direction) 0);
    path.addRect(0, 12, 9, 9, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine26x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 18, 12, 12, (SkPath::Direction) 0);
    path.addRect(0, 12, 9, 9, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine27() {
    SkPath path;
    path.addRect(0, 18, 12, 12, (SkPath::Direction) 0);
    path.addRect(12, 8, 21, 21, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine27x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 18, 12, 12, (SkPath::Direction) 0);
    path.addRect(12, 8, 21, 21, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine28() {
    SkPath path;
    path.addRect(0, 6, 12, 12, (SkPath::Direction) 0);
    path.addRect(0, 0, 9, 9, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine28x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 6, 12, 12, (SkPath::Direction) 0);
    path.addRect(0, 0, 9, 9, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine29() {
    SkPath path;
    path.addRect(0, 18, 12, 12, (SkPath::Direction) 0);
    path.addRect(12, 12, 21, 21, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine29x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 18, 12, 12, (SkPath::Direction) 0);
    path.addRect(12, 12, 21, 21, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine30() {
    SkPath path;
    path.addRect(0, 0, 20, 20, (SkPath::Direction) 0);
    path.addRect(0, 0, 12, 12, (SkPath::Direction) 0);
    path.addRect(4, 4, 13, 13, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine30x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 20, 20, (SkPath::Direction) 0);
    path.addRect(0, 0, 12, 12, (SkPath::Direction) 0);
    path.addRect(4, 4, 13, 13, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine31() {
    SkPath path;
    path.addRect(0, 0, 20, 20, (SkPath::Direction) 0);
    path.addRect(0, 0, 12, 12, (SkPath::Direction) 0);
    path.addRect(0, 4, 9, 9, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine31x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 20, 20, (SkPath::Direction) 0);
    path.addRect(0, 0, 12, 12, (SkPath::Direction) 0);
    path.addRect(0, 4, 9, 9, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine32() {
    SkPath path;
    path.addRect(0, 0, 20, 20, (SkPath::Direction) 0);
    path.addRect(0, 0, 12, 12, (SkPath::Direction) 0);
    path.addRect(4, 12, 13, 13, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine32x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 20, 20, (SkPath::Direction) 0);
    path.addRect(0, 0, 12, 12, (SkPath::Direction) 0);
    path.addRect(4, 12, 13, 13, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine33() {
    SkPath path;
    path.addRect(0, 0, 20, 20, (SkPath::Direction) 0);
    path.addRect(0, 0, 12, 12, (SkPath::Direction) 0);
    path.addRect(4, 16, 13, 13, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine33x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 20, 20, (SkPath::Direction) 0);
    path.addRect(0, 0, 12, 12, (SkPath::Direction) 0);
    path.addRect(4, 16, 13, 13, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine34() {
    SkPath path;
    path.addRect(0, 0, 20, 20, (SkPath::Direction) 0);
    path.addRect(0, 6, 12, 12, (SkPath::Direction) 0);
    path.addRect(4, 12, 13, 13, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine34x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 20, 20, (SkPath::Direction) 0);
    path.addRect(0, 6, 12, 12, (SkPath::Direction) 0);
    path.addRect(4, 12, 13, 13, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine35() {
    SkPath path;
    path.addRect(0, 0, 20, 20, (SkPath::Direction) 0);
    path.addRect(6, 0, 18, 18, (SkPath::Direction) 0);
    path.addRect(4, 16, 13, 13, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine35x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 20, 20, (SkPath::Direction) 0);
    path.addRect(6, 0, 18, 18, (SkPath::Direction) 0);
    path.addRect(4, 16, 13, 13, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine36() {
    SkPath path;
    path.addRect(0, 10, 20, 20, (SkPath::Direction) 0);
    path.addRect(6, 12, 18, 18, (SkPath::Direction) 0);
    path.addRect(4, 16, 13, 13, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine36x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 10, 20, 20, (SkPath::Direction) 0);
    path.addRect(6, 12, 18, 18, (SkPath::Direction) 0);
    path.addRect(4, 16, 13, 13, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine37() {
    SkPath path;
    path.addRect(0, 20, 20, 20, (SkPath::Direction) 0);
    path.addRect(18, 24, 30, 30, (SkPath::Direction) 0);
    path.addRect(0, 0, 9, 9, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine37x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 20, 20, 20, (SkPath::Direction) 0);
    path.addRect(18, 24, 30, 30, (SkPath::Direction) 0);
    path.addRect(0, 0, 9, 9, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine38() {
    SkPath path;
    path.addRect(10, 0, 30, 30, (SkPath::Direction) 0);
    path.addRect(6, 12, 18, 18, (SkPath::Direction) 0);
    path.addRect(12, 12, 21, 21, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine38x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(10, 0, 30, 30, (SkPath::Direction) 0);
    path.addRect(6, 12, 18, 18, (SkPath::Direction) 0);
    path.addRect(12, 12, 21, 21, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine40() {
    SkPath path;
    path.addRect(10, 0, 30, 30, (SkPath::Direction) 0);
    path.addRect(12, 18, 24, 24, (SkPath::Direction) 0);
    path.addRect(4, 16, 13, 13, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine40x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(10, 0, 30, 30, (SkPath::Direction) 0);
    path.addRect(12, 18, 24, 24, (SkPath::Direction) 0);
    path.addRect(4, 16, 13, 13, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine41() {
    SkPath path;
    path.addRect(0, 0, 20, 20, (SkPath::Direction) 0);
    path.addRect(18, 24, 30, 30, (SkPath::Direction) 0);
    path.addRect(12, 0, 21, 21, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine41x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 20, 20, (SkPath::Direction) 0);
    path.addRect(18, 24, 30, 30, (SkPath::Direction) 0);
    path.addRect(12, 0, 21, 21, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine42() {
    SkPath path;
    path.addRect(0, 0, 20, 20, (SkPath::Direction) 0);
    path.addRect(0, 0, 12, 12, (SkPath::Direction) 0);
    path.addRect(8, 16, 17, 17, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine42x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 20, 20, (SkPath::Direction) 0);
    path.addRect(0, 0, 12, 12, (SkPath::Direction) 0);
    path.addRect(8, 16, 17, 17, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine43() {
    SkPath path;
    path.addRect(0, 0, 20, 20, (SkPath::Direction) 0);
    path.addRect(6, 24, 18, 18, (SkPath::Direction) 0);
    path.addRect(0, 32, 9, 36, (SkPath::Direction) 1);
    testSimplifyx(path);
}

static void testLine43x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 20, 20, (SkPath::Direction) 0);
    path.addRect(6, 24, 18, 18, (SkPath::Direction) 0);
    path.addRect(0, 32, 9, 36, (SkPath::Direction) 1);
    testSimplifyx(path);
}

static void testLine44() {
    SkPath path;
    path.addRect(10, 40, 30, 30, (SkPath::Direction) 0);
    path.addRect(18, 0, 30, 30, (SkPath::Direction) 0);
    path.addRect(18, 32, 27, 36, (SkPath::Direction) 1);
    testSimplifyx(path);
}

static void testLine44x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(10, 40, 30, 30, (SkPath::Direction) 0);
    path.addRect(18, 0, 30, 30, (SkPath::Direction) 0);
    path.addRect(18, 32, 27, 36, (SkPath::Direction) 1);
    testSimplifyx(path);
}

static void testLine45() {
    SkPath path;
    path.addRect(10, 0, 30, 30, (SkPath::Direction) 0);
    path.addRect(18, 0, 30, 30, (SkPath::Direction) 0);
    path.addRect(24, 32, 33, 36, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine45x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(10, 0, 30, 30, (SkPath::Direction) 0);
    path.addRect(18, 0, 30, 30, (SkPath::Direction) 0);
    path.addRect(24, 32, 33, 36, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine46() {
    SkPath path;
    path.addRect(10, 40, 30, 30, (SkPath::Direction) 0);
    path.addRect(24, 0, 36, 36, (SkPath::Direction) 0);
    path.addRect(24, 32, 33, 36, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine46x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(10, 40, 30, 30, (SkPath::Direction) 0);
    path.addRect(24, 0, 36, 36, (SkPath::Direction) 0);
    path.addRect(24, 32, 33, 36, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine47() {
    SkPath path;
    path.addRect(0, 0, 20, 20, (SkPath::Direction) 0);
    path.addRect(0, 0, 12, 12, (SkPath::Direction) 0);
    path.addRect(0, 0, 9, 9, (SkPath::Direction) 1);
    testSimplifyx(path);
}

static void testLine47x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 20, 20, (SkPath::Direction) 0);
    path.addRect(0, 0, 12, 12, (SkPath::Direction) 0);
    path.addRect(0, 0, 9, 9, (SkPath::Direction) 1);
    testSimplifyx(path);
}

static void testLine48() {
    SkPath path;
    path.addRect(0, 0, 20, 20, (SkPath::Direction) 0);
    path.addRect(0, 6, 12, 12, (SkPath::Direction) 0);
    path.addRect(0, 0, 9, 9, (SkPath::Direction) 1);
    testSimplifyx(path);
}

static void testLine48x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 20, 20, (SkPath::Direction) 0);
    path.addRect(0, 6, 12, 12, (SkPath::Direction) 0);
    path.addRect(0, 0, 9, 9, (SkPath::Direction) 1);
    testSimplifyx(path);
}

static void testLine49() {
    SkPath path;
    path.addRect(0, 0, 20, 20, (SkPath::Direction) 0);
    path.addRect(0, 0, 12, 12, (SkPath::Direction) 0);
    path.addRect(0, 0, 9, 9, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine49x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 20, 20, (SkPath::Direction) 0);
    path.addRect(0, 0, 12, 12, (SkPath::Direction) 0);
    path.addRect(0, 0, 9, 9, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine50() {
    SkPath path;
    path.addRect(10, 30, 30, 30, (SkPath::Direction) 0);
    path.addRect(24, 20, 36, 30, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine50x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(10, 30, 30, 30, (SkPath::Direction) 0);
    path.addRect(24, 20, 36, 30, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine51() {
    SkPath path;
    path.addRect(0, 0, 20, 20, (SkPath::Direction) 0);
    path.addRect(0, 12, 12, 12, (SkPath::Direction) 0);
    path.addRect(4, 12, 13, 13, (SkPath::Direction) 1);
    testSimplifyx(path);
}

static void testLine51x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 20, 20, (SkPath::Direction) 0);
    path.addRect(0, 12, 12, 12, (SkPath::Direction) 0);
    path.addRect(4, 12, 13, 13, (SkPath::Direction) 1);
    testSimplifyx(path);
}

static void testLine52() {
    SkPath path;
    path.addRect(0, 30, 20, 20, (SkPath::Direction) 0);
    path.addRect(6, 20, 18, 30, (SkPath::Direction) 0);
    path.addRect(32, 0, 36, 41, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine52x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 30, 20, 20, (SkPath::Direction) 0);
    path.addRect(6, 20, 18, 30, (SkPath::Direction) 0);
    path.addRect(32, 0, 36, 41, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine53() {
    SkPath path;
    path.addRect(10, 30, 30, 30, (SkPath::Direction) 0);
    path.addRect(12, 20, 24, 30, (SkPath::Direction) 0);
    path.addRect(12, 32, 21, 36, (SkPath::Direction) 1);
    testSimplifyx(path);
}

static void testLine53x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(10, 30, 30, 30, (SkPath::Direction) 0);
    path.addRect(12, 20, 24, 30, (SkPath::Direction) 0);
    path.addRect(12, 32, 21, 36, (SkPath::Direction) 1);
    testSimplifyx(path);
}

static void testLine54() {
    SkPath path;
    path.addRect(0, 0, 20, 20, (SkPath::Direction) 0);
    path.addRect(6, 0, 18, 18, (SkPath::Direction) 0);
    path.addRect(8, 4, 17, 17, (SkPath::Direction) 1);
    testSimplifyx(path);
}

static void testLine54x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 20, 20, (SkPath::Direction) 0);
    path.addRect(6, 0, 18, 18, (SkPath::Direction) 0);
    path.addRect(8, 4, 17, 17, (SkPath::Direction) 1);
    testSimplifyx(path);
}

static void testLine55() {
    SkPath path;
    path.addRect(0, 0, 20, 20, (SkPath::Direction) 0);
    path.addRect(6, 6, 18, 18, (SkPath::Direction) 0);
    path.addRect(4, 4, 13, 13, (SkPath::Direction) 1);
    testSimplifyx(path);
}

static void testLine55x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 20, 20, (SkPath::Direction) 0);
    path.addRect(6, 6, 18, 18, (SkPath::Direction) 0);
    path.addRect(4, 4, 13, 13, (SkPath::Direction) 1);
    testSimplifyx(path);
}

static void testLine56() {
    SkPath path;
    path.addRect(0, 20, 20, 20, (SkPath::Direction) 0);
    path.addRect(18, 20, 30, 30, (SkPath::Direction) 0);
    path.addRect(12, 0, 21, 21, (SkPath::Direction) 1);
    testSimplifyx(path);
}

static void testLine56x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 20, 20, 20, (SkPath::Direction) 0);
    path.addRect(18, 20, 30, 30, (SkPath::Direction) 0);
    path.addRect(12, 0, 21, 21, (SkPath::Direction) 1);
    testSimplifyx(path);
}

static void testLine57() {
    SkPath path;
    path.addRect(20, 0, 40, 40, (SkPath::Direction) 0);
    path.addRect(20, 0, 30, 40, (SkPath::Direction) 0);
    path.addRect(12, 0, 21, 21, (SkPath::Direction) 1);
    testSimplifyx(path);
}

static void testLine57x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(20, 0, 40, 40, (SkPath::Direction) 0);
    path.addRect(20, 0, 30, 40, (SkPath::Direction) 0);
    path.addRect(12, 0, 21, 21, (SkPath::Direction) 1);
    testSimplifyx(path);
}

static void testLine58() {
    SkPath path;
    path.addRect(0, 0, 20, 20, (SkPath::Direction) 0);
    path.addRect(0, 0, 12, 12, (SkPath::Direction) 1);
    path.addRect(0, 12, 9, 9, (SkPath::Direction) 1);
    testSimplifyx(path);
}

static void testLine58x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 20, 20, (SkPath::Direction) 0);
    path.addRect(0, 0, 12, 12, (SkPath::Direction) 1);
    path.addRect(0, 12, 9, 9, (SkPath::Direction) 1);
    testSimplifyx(path);
}

static void testLine59() {
    SkPath path;
    path.addRect(0, 0, 20, 20, (SkPath::Direction) 0);
    path.addRect(6, 6, 18, 18, (SkPath::Direction) 1);
    path.addRect(4, 4, 13, 13, (SkPath::Direction) 1);
    testSimplifyx(path);
}

static void testLine59x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 20, 20, (SkPath::Direction) 0);
    path.addRect(6, 6, 18, 18, (SkPath::Direction) 1);
    path.addRect(4, 4, 13, 13, (SkPath::Direction) 1);
    testSimplifyx(path);
}

static void testLine60() {
    SkPath path;
    path.addRect(0, 0, 20, 20, (SkPath::Direction) 0);
    path.addRect(6, 12, 18, 18, (SkPath::Direction) 1);
    path.addRect(4, 12, 13, 13, (SkPath::Direction) 1);
    testSimplifyx(path);
}

static void testLine60x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 20, 20, (SkPath::Direction) 0);
    path.addRect(6, 12, 18, 18, (SkPath::Direction) 1);
    path.addRect(4, 12, 13, 13, (SkPath::Direction) 1);
    testSimplifyx(path);
}

static void testLine61() {
    SkPath path;
    path.addRect(0, 0, 20, 20, (SkPath::Direction) 0);
    path.addRect(12, 0, 24, 24, (SkPath::Direction) 1);
    path.addRect(12, 0, 21, 21, (SkPath::Direction) 1);
    testSimplifyx(path);
}

static void testLine61x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 20, 20, (SkPath::Direction) 0);
    path.addRect(12, 0, 24, 24, (SkPath::Direction) 1);
    path.addRect(12, 0, 21, 21, (SkPath::Direction) 1);
    testSimplifyx(path);
}

static void testLine62() {
    SkPath path;
    path.addRect(0, 0, 60, 60, (SkPath::Direction) 0);
    path.addRect(0, 0, 20, 20, (SkPath::Direction) 0);
    path.addRect(0, 12, 12, 12, (SkPath::Direction) 0);
    path.addRect(4, 12, 13, 13, (SkPath::Direction) 1);
    testSimplifyx(path);
}

static void testLine62x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 60, 60, (SkPath::Direction) 0);
    path.addRect(0, 0, 20, 20, (SkPath::Direction) 0);
    path.addRect(0, 12, 12, 12, (SkPath::Direction) 0);
    path.addRect(4, 12, 13, 13, (SkPath::Direction) 1);
    testSimplifyx(path);
}

static void testLine63() {
    SkPath path;
    path.addRect(0, 0, 60, 60, (SkPath::Direction) 0);
    path.addRect(0, 10, 20, 20, (SkPath::Direction) 0);
    path.addRect(0, 6, 12, 12, (SkPath::Direction) 1);
    path.addRect(0, 32, 9, 36, (SkPath::Direction) 1);
    testSimplifyx(path);
}

static void testLine63x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 60, 60, (SkPath::Direction) 0);
    path.addRect(0, 10, 20, 20, (SkPath::Direction) 0);
    path.addRect(0, 6, 12, 12, (SkPath::Direction) 1);
    path.addRect(0, 32, 9, 36, (SkPath::Direction) 1);
    testSimplifyx(path);
}

static void testLine64() {
    SkPath path;
    path.addRect(0, 0, 60, 60, (SkPath::Direction) 0);
    path.addRect(10, 40, 30, 30, (SkPath::Direction) 0);
    path.addRect(18, 6, 30, 30, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine64x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 60, 60, (SkPath::Direction) 0);
    path.addRect(10, 40, 30, 30, (SkPath::Direction) 0);
    path.addRect(18, 6, 30, 30, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine65() {
    SkPath path;
    path.addRect(0, 0, 60, 60, (SkPath::Direction) 0);
    path.addRect(10, 0, 30, 30, (SkPath::Direction) 0);
    path.addRect(24, 0, 36, 36, (SkPath::Direction) 0);
    path.addRect(32, 6, 36, 41, (SkPath::Direction) 1);
    testSimplifyx(path);
}

static void testLine65x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 60, 60, (SkPath::Direction) 0);
    path.addRect(10, 0, 30, 30, (SkPath::Direction) 0);
    path.addRect(24, 0, 36, 36, (SkPath::Direction) 0);
    path.addRect(32, 6, 36, 41, (SkPath::Direction) 1);
    testSimplifyx(path);
}

static void testLine66() {
    SkPath path;
    path.addRect(0, 0, 60, 60, (SkPath::Direction) 0);
    path.addRect(0, 30, 20, 20, (SkPath::Direction) 0);
    path.addRect(12, 20, 24, 30, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine66x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 60, 60, (SkPath::Direction) 0);
    path.addRect(0, 30, 20, 20, (SkPath::Direction) 0);
    path.addRect(12, 20, 24, 30, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine67() {
    SkPath path;
    path.addRect(0, 0, 60, 60, (SkPath::Direction) 0);
    path.addRect(10, 40, 30, 30, (SkPath::Direction) 0);
    path.addRect(24, 20, 36, 30, (SkPath::Direction) 0);
    path.addRect(32, 0, 36, 41, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine67x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 60, 60, (SkPath::Direction) 0);
    path.addRect(10, 40, 30, 30, (SkPath::Direction) 0);
    path.addRect(24, 20, 36, 30, (SkPath::Direction) 0);
    path.addRect(32, 0, 36, 41, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine68a() {
    SkPath path;
    path.addRect(0, 0, 8, 8, (SkPath::Direction) 0);
    path.addRect(2, 2, 6, 6, (SkPath::Direction) 0);
    path.addRect(1, 2, 4, 2, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine68ax() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 8, 8, (SkPath::Direction) 0);
    path.addRect(2, 2, 6, 6, (SkPath::Direction) 0);
    path.addRect(1, 2, 4, 2, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine68b() {
    SkPath path;
    path.addRect(0, 0, 8, 8, (SkPath::Direction) 0);
    path.addRect(2, 2, 6, 6, (SkPath::Direction) 1);
    path.addRect(1, 2, 2, 2, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine68bx() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 8, 8, (SkPath::Direction) 0);
    path.addRect(2, 2, 6, 6, (SkPath::Direction) 1);
    path.addRect(1, 2, 2, 2, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine68c() {
    SkPath path;
    path.addRect(0, 0, 8, 8, (SkPath::Direction) 1);
    path.addRect(2, 2, 6, 6, (SkPath::Direction) 0);
    path.addRect(1, 2, 4, 2, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine68cx() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 8, 8, (SkPath::Direction) 1);
    path.addRect(2, 2, 6, 6, (SkPath::Direction) 0);
    path.addRect(1, 2, 4, 2, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine68d() {
    SkPath path;
    path.addRect(0, 0, 8, 8, (SkPath::Direction) 1);
    path.addRect(2, 2, 6, 6, (SkPath::Direction) 1);
    path.addRect(1, 2, 4, 2, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine68dx() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 8, 8, (SkPath::Direction) 1);
    path.addRect(2, 2, 6, 6, (SkPath::Direction) 1);
    path.addRect(1, 2, 4, 2, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine68e() {
    SkPath path;
    path.addRect(0, 0, 8, 8, (SkPath::Direction) 0);
    path.addRect(0, 0, 8, 8, (SkPath::Direction) 0);
    path.addRect(2, 2, 6, 6, (SkPath::Direction) 1);
    path.addRect(1, 2, 2, 2, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine68ex() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 8, 8, (SkPath::Direction) 0);
    path.addRect(0, 0, 8, 8, (SkPath::Direction) 0);
    path.addRect(2, 2, 6, 6, (SkPath::Direction) 1);
    path.addRect(1, 2, 2, 2, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine68f() {
    SkPath path;
    path.addRect(0, 0, 8, 8, (SkPath::Direction) 0);
    path.addRect(2, 2, 6, 6, (SkPath::Direction) 1);
    path.addRect(2, 2, 6, 6, (SkPath::Direction) 1);
    path.addRect(1, 2, 2, 2, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine68fx() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 8, 8, (SkPath::Direction) 0);
    path.addRect(2, 2, 6, 6, (SkPath::Direction) 1);
    path.addRect(2, 2, 6, 6, (SkPath::Direction) 1);
    path.addRect(1, 2, 2, 2, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine68g() {
    SkPath path;
    path.addRect(0, 0, 8, 8, (SkPath::Direction) 0);
    path.addRect(0, 0, 8, 8, (SkPath::Direction) 0);
    path.addRect(2, 2, 6, 6, (SkPath::Direction) 1);
    path.addRect(2, 2, 6, 6, (SkPath::Direction) 1);
    path.addRect(1, 2, 2, 2, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine68gx() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 8, 8, (SkPath::Direction) 0);
    path.addRect(0, 0, 8, 8, (SkPath::Direction) 0);
    path.addRect(2, 2, 6, 6, (SkPath::Direction) 1);
    path.addRect(2, 2, 6, 6, (SkPath::Direction) 1);
    path.addRect(1, 2, 2, 2, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine68h() {
    SkPath path;
    path.addRect(0, 0, 8, 8, (SkPath::Direction) 0);
    path.addRect(2, 2, 6, 6, (SkPath::Direction) 1);
    path.addRect(2, 2, 6, 6, (SkPath::Direction) 1);
    path.addRect(2, 2, 6, 6, (SkPath::Direction) 1);
    path.addRect(1, 2, 2, 2, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine68hx() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 8, 8, (SkPath::Direction) 0);
    path.addRect(2, 2, 6, 6, (SkPath::Direction) 1);
    path.addRect(2, 2, 6, 6, (SkPath::Direction) 1);
    path.addRect(2, 2, 6, 6, (SkPath::Direction) 1);
    path.addRect(1, 2, 2, 2, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine69() {
    SkPath path;
    path.addRect(0, 20, 20, 20, (SkPath::Direction) 0);
    path.addRect(0, 20, 12, 30, (SkPath::Direction) 0);
    path.addRect(12, 32, 21, 36, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine69x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 20, 20, 20, (SkPath::Direction) 0);
    path.addRect(0, 20, 12, 30, (SkPath::Direction) 0);
    path.addRect(12, 32, 21, 36, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine70() {
    SkPath path;
    path.addRect(0, 0, 20, 20, (SkPath::Direction) 0);
    path.addRect(0, 24, 12, 12, (SkPath::Direction) 0);
    path.addRect(12, 32, 21, 36, (SkPath::Direction) 1);
    testSimplifyx(path);
}

static void testLine70x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 20, 20, (SkPath::Direction) 0);
    path.addRect(0, 24, 12, 12, (SkPath::Direction) 0);
    path.addRect(12, 32, 21, 36, (SkPath::Direction) 1);
    testSimplifyx(path);
}

static void testLine71() {
    SkPath path;
    path.addRect(0, 0, 20, 20, (SkPath::Direction) 0);
    path.addRect(12, 0, 24, 24, (SkPath::Direction) 0);
    path.addRect(12, 32, 21, 36, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine71x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 20, 20, (SkPath::Direction) 0);
    path.addRect(12, 0, 24, 24, (SkPath::Direction) 0);
    path.addRect(12, 32, 21, 36, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine72() {
    SkPath path;
    path.addRect(0, 0, 60, 60, (SkPath::Direction) 0);
    path.addRect(10, 40, 30, 30, (SkPath::Direction) 0);
    path.addRect(6, 20, 18, 30, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine72x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 60, 60, (SkPath::Direction) 0);
    path.addRect(10, 40, 30, 30, (SkPath::Direction) 0);
    path.addRect(6, 20, 18, 30, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine73() {
    SkPath path;
    path.addRect(0, 0, 60, 60, (SkPath::Direction) 0);
    path.addRect(0, 40, 20, 20, (SkPath::Direction) 0);
    path.addRect(0, 20, 12, 30, (SkPath::Direction) 0);
    path.addRect(0, 0, 9, 9, (SkPath::Direction) 1);
    testSimplifyx(path);
}

static void testLine73x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 60, 60, (SkPath::Direction) 0);
    path.addRect(0, 40, 20, 20, (SkPath::Direction) 0);
    path.addRect(0, 20, 12, 30, (SkPath::Direction) 0);
    path.addRect(0, 0, 9, 9, (SkPath::Direction) 1);
    testSimplifyx(path);
}

static void testLine74() {
    SkPath path;
    path.addRect(20, 30, 40, 40, (SkPath::Direction) 0);
    path.addRect(24, 20, 36, 30, (SkPath::Direction) 1);
    path.addRect(32, 24, 36, 41, (SkPath::Direction) 1);
    testSimplifyx(path);
}

static void testLine74x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(20, 30, 40, 40, (SkPath::Direction) 0);
    path.addRect(24, 20, 36, 30, (SkPath::Direction) 1);
    path.addRect(32, 24, 36, 41, (SkPath::Direction) 1);
    testSimplifyx(path);
}

static void testLine75() {
    SkPath path;
    path.addRect(0, 0, 60, 60, (SkPath::Direction) 0);
    path.addRect(10, 0, 30, 30, (SkPath::Direction) 1);
    path.addRect(18, 0, 30, 30, (SkPath::Direction) 1);
    path.addRect(12, 0, 21, 21, (SkPath::Direction) 1);
    testSimplifyx(path);
}

static void testLine75x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 60, 60, (SkPath::Direction) 0);
    path.addRect(10, 0, 30, 30, (SkPath::Direction) 1);
    path.addRect(18, 0, 30, 30, (SkPath::Direction) 1);
    path.addRect(12, 0, 21, 21, (SkPath::Direction) 1);
    testSimplifyx(path);
}

static void testLine76() {
    SkPath path;
    path.addRect(36, 0, 66, 60, (SkPath::Direction) 0);
    path.addRect(10, 20, 40, 30, (SkPath::Direction) 0);
    path.addRect(24, 20, 36, 30, (SkPath::Direction) 1);
    path.addRect(32, 6, 36, 41, (SkPath::Direction) 1);
    testSimplifyx(path);
}

static void testLine76x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(36, 0, 66, 60, (SkPath::Direction) 0);
    path.addRect(10, 20, 40, 30, (SkPath::Direction) 0);
    path.addRect(24, 20, 36, 30, (SkPath::Direction) 1);
    path.addRect(32, 6, 36, 41, (SkPath::Direction) 1);
    testSimplifyx(path);
}

static void testLine77() {
    SkPath path;
    path.addRect(20, 0, 40, 40, (SkPath::Direction) 0);
    path.addRect(24, 6, 36, 36, (SkPath::Direction) 1);
    path.addRect(24, 32, 33, 36, (SkPath::Direction) 1);
    testSimplifyx(path);
}

static void testLine77x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(20, 0, 40, 40, (SkPath::Direction) 0);
    path.addRect(24, 6, 36, 36, (SkPath::Direction) 1);
    path.addRect(24, 32, 33, 36, (SkPath::Direction) 1);
    testSimplifyx(path);
}

static void testLine78() {
    SkPath path;
    path.addRect(0, 0, 30, 60, (SkPath::Direction) 0);
    path.addRect(10, 20, 30, 30, (SkPath::Direction) 1);
    path.addRect(18, 20, 30, 30, (SkPath::Direction) 1);
    path.addRect(32, 0, 36, 41, (SkPath::Direction) 1);
    testSimplifyx(path);
}

static void testLine78x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 30, 60, (SkPath::Direction) 0);
    path.addRect(10, 20, 30, 30, (SkPath::Direction) 1);
    path.addRect(18, 20, 30, 30, (SkPath::Direction) 1);
    path.addRect(32, 0, 36, 41, (SkPath::Direction) 1);
    testSimplifyx(path);
}

static void testLine79() {
    SkPath path;
    path.addRect(0, 36, 60, 30, (SkPath::Direction) 0);
    path.addRect(10, 30, 40, 30, (SkPath::Direction) 0);
    path.addRect(0, 20, 12, 30, (SkPath::Direction) 1);
    path.addRect(0, 32, 9, 36, (SkPath::Direction) 1);
    testSimplifyx(path);
}

static void testLine79x() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 36, 60, 30, (SkPath::Direction) 0);
    path.addRect(10, 30, 40, 30, (SkPath::Direction) 0);
    path.addRect(0, 20, 12, 30, (SkPath::Direction) 1);
    path.addRect(0, 32, 9, 36, (SkPath::Direction) 1);
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

static void testLine1ax() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 0, 12, 12, (SkPath::Direction) 0);
    path.addRect(4, 0, 13, 13, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine2ax() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(0, 20, 20, 20, (SkPath::Direction) 0);
    path.addRect(0, 20, 12, 30, (SkPath::Direction) 0);
    path.addRect(12, 0, 21, 21, (SkPath::Direction) 1);
    testSimplifyx(path);
}

static void testLine3aax() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(10, 30, 30, 30, (SkPath::Direction) 0);
    path.addRect(18, 20, 30, 30, (SkPath::Direction) 1);
    path.addRect(0, 32, 9, 36, (SkPath::Direction) 1);
    testSimplifyx(path);
}

static void testLine4ax() {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.addRect(10, 30, 30, 30, (SkPath::Direction) 0);
    path.addRect(24, 20, 36, 30, (SkPath::Direction) 1);
    path.addRect(0, 32, 9, 36, (SkPath::Direction) 1);
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

static void (*firstTest)() = testQuadratic18;

static struct {
    void (*fun)();
    const char* str;
} tests[] = {
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
    TEST(testQuadralateral6),
    TEST(testQuadralateral6a),
    TEST(testFauxQuadralateral6d),
    TEST(testFauxQuadralateral6c),
    TEST(testFauxQuadralateral6b),
    TEST(testFauxQuadralateral6a),
    TEST(testFauxQuadralateral6),
};

static const size_t subTestCount = sizeof(subTests) / sizeof(subTests[0]);

static bool skipAll = false;
static bool runSubTests = false;

void SimplifyNew_Test() {
    if (skipAll) {
        return;
    }
#ifdef SK_DEBUG
    gDebugMaxWindSum = 4;
    gDebugMaxWindValue = 4;
    size_t index;
#endif
    if (runSubTests) {
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
        SkDebugf("  %s [%s]\n", __FUNCTION__, tests[index].str);
        (*tests[index].fun)();
    }
    index = testCount - 1;
    bool firstTestComplete = false;
    do {
        SkDebugf("  %s [%s]\n", __FUNCTION__, tests[index].str);
        (*tests[index].fun)();
        firstTestComplete = true;
    } while (index--);
#ifdef SK_DEBUG
    gDebugMaxWindSum = SK_MaxS32;
    gDebugMaxWindValue = SK_MaxS32;
#endif
}
