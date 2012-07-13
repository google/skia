/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "EdgeWalker_Test.h"
#include "Intersection_Tests.h"
#include "ShapeOps.h"

static void testLine1() {
    SkPath path, simple;
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
    SkPath path, simple;
    addInnerCWTriangle(path);
    addOuterCWTriangle(path);
    testSimplifyx(path);
}

static void testLine3() {
    SkPath path, simple;
    addInnerCCWTriangle(path);
    addOuterCWTriangle(path);
    testSimplifyx(path);
}

static void testLine4() {
    SkPath path, simple;
    addOuterCCWTriangle(path);
    addOuterCWTriangle(path);
    testSimplifyx(path);
}

static void testLine5() {
    SkPath path, simple;
    addOuterCWTriangle(path);
    addOuterCWTriangle(path);
    testSimplifyx(path);
}

static void testLine6() {
    SkPath path, simple;
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
    SkPath path, simple;
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
    SkPath path, simple;
    path.moveTo(0,0);
    path.lineTo(4,0);
    path.lineTo(2,2);
    path.close();
    testSimplifyx(path);
}

static void testLine7b() {
    SkPath path, simple;
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
    SkPath path, simple;
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
    SkPath path, simple;
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
    SkPath path, simple;
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
    SkPath path, simple;
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
    SkPath path, simple;
    addCWContainer(path);
    addCWContents(path);
    testSimplifyx(path);
}

static void testLine12() {
    SkPath path, simple;
    addCCWContainer(path);
    addCWContents(path);
    testSimplifyx(path);
}

static void testLine13() {
    SkPath path, simple;
    addCWContainer(path);
    addCCWContents(path);
    testSimplifyx(path);
}

static void testLine14() {
    SkPath path, simple;
    addCCWContainer(path);
    addCCWContents(path);
    testSimplifyx(path);
}

static void testLine15() {
    SkPath path, simple;
    path.addRect(0, 0, 9, 9, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine16() {
    SkPath path, simple;
    path.addRect(0, 0, 12, 12, (SkPath::Direction) 0);
    path.addRect(0, 4, 9, 9, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine17() {
    SkPath path, simple;
    path.addRect(0, 0, 12, 12, (SkPath::Direction) 0);
    path.addRect(4, 12, 13, 13, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine18() {
    SkPath path, simple;
    path.addRect(0, 0, 12, 12, (SkPath::Direction) 0);
    path.addRect(12, 4, 21, 21, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine19() {
    SkPath path, simple;
    path.addRect(0, 0, 12, 12, (SkPath::Direction) 0);
    path.addRect(12, 16, 21, 21, (SkPath::Direction) 0);    
    testSimplifyx(path);
}

static void testLine20() {
    SkPath path, simple;
    path.addRect(0, 12, 12, 12, (SkPath::Direction) 0);
    path.addRect(0, 12, 9, 9, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine21() {
    SkPath path, simple;
    path.addRect(0, 12, 12, 12, (SkPath::Direction) 0);
    path.addRect(0, 16, 9, 9, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine22() {
    SkPath path, simple;
    path.addRect(0, 12, 12, 12, (SkPath::Direction) 0);
    path.addRect(4, 12, 13, 13, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine23() {
    SkPath path, simple;
    path.addRect(0, 12, 12, 12, (SkPath::Direction) 0);
    path.addRect(12, 0, 21, 21, (SkPath::Direction) 0);
    testSimplifyx(path);
}



static void testLine24a() {
    SkPath path, simple;
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
    SkPath path, simple;
    path.addRect(0, 18, 12, 12, (SkPath::Direction) 0);
    path.addRect(4, 12, 13, 13, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine25() {
    SkPath path, simple;
    path.addRect(0, 6, 12, 12, (SkPath::Direction) 0);
    path.addRect(12, 0, 21, 21, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine26() {
    SkPath path, simple;
    path.addRect(0, 18, 12, 12, (SkPath::Direction) 0);
    path.addRect(0, 12, 9, 9, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine27() {
    SkPath path, simple;
    path.addRect(0, 18, 12, 12, (SkPath::Direction) 0);
    path.addRect(12, 8, 21, 21, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine28() {
    SkPath path, simple;
    path.addRect(0, 6, 12, 12, (SkPath::Direction) 0);
    path.addRect(0, 0, 9, 9, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine29() {
    SkPath path, simple;
    path.addRect(0, 18, 12, 12, (SkPath::Direction) 0);
    path.addRect(12, 12, 21, 21, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine30() {
    SkPath path, simple;
    path.addRect(0, 0, 20, 20, (SkPath::Direction) 0);
    path.addRect(0, 0, 12, 12, (SkPath::Direction) 0);
    path.addRect(4, 4, 13, 13, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine31() {
    SkPath path, simple;
    path.addRect(0, 0, 20, 20, (SkPath::Direction) 0);
    path.addRect(0, 0, 12, 12, (SkPath::Direction) 0);
    path.addRect(0, 4, 9, 9, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine32() {
    SkPath path, simple;
    path.addRect(0, 0, 20, 20, (SkPath::Direction) 0);
    path.addRect(0, 0, 12, 12, (SkPath::Direction) 0);
    path.addRect(4, 12, 13, 13, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine33() {
    SkPath path, simple;
    path.addRect(0, 0, 20, 20, (SkPath::Direction) 0);
    path.addRect(0, 0, 12, 12, (SkPath::Direction) 0);
    path.addRect(4, 16, 13, 13, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine34() {
    SkPath path, simple;
    path.addRect(0, 0, 20, 20, (SkPath::Direction) 0);
    path.addRect(0, 6, 12, 12, (SkPath::Direction) 0);
    path.addRect(4, 12, 13, 13, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine35() {
    SkPath path, simple;
    path.addRect(0, 0, 20, 20, (SkPath::Direction) 0);
    path.addRect(6, 0, 18, 18, (SkPath::Direction) 0);
    path.addRect(4, 16, 13, 13, (SkPath::Direction) 0);
    testSimplifyx(path);
}

static void testLine36() {
    SkPath path, simple;
    path.addRect(0, 10, 20, 20, (SkPath::Direction) 0);
    path.addRect(6, 12, 18, 18, (SkPath::Direction) 0);
    path.addRect(4, 16, 13, 13, (SkPath::Direction) 0);
    testSimplifyx(path);
}

#define TEST(name) { name, #name }

static struct {
    void (*fun)();
    const char* str;
} tests[] = {
    TEST(testLine1),
    TEST(testLine2),
    TEST(testLine3),
    TEST(testLine4),
    TEST(testLine5),
    TEST(testLine6),
    TEST(testLine7a),
    TEST(testLine7b),
    TEST(testLine7),
    TEST(testLine8),
    TEST(testLine9),
    TEST(testLine10),
    TEST(testLine10a),
    TEST(testLine11),
    TEST(testLine12),
    TEST(testLine13),
    TEST(testLine14),
    TEST(testLine15),
    TEST(testLine16),
    TEST(testLine17),
    TEST(testLine18),
    TEST(testLine19),
    TEST(testLine20),
    TEST(testLine21),
    TEST(testLine22),
    TEST(testLine23),
    TEST(testLine24a),
    TEST(testLine24),
    TEST(testLine25),
    TEST(testLine26),
    TEST(testLine27),
    TEST(testLine28),
    TEST(testLine29),
    TEST(testLine30),
    TEST(testLine31),
    TEST(testLine32),
    TEST(testLine33),
    TEST(testLine34),
    TEST(testLine35),
    TEST(testLine36),
};

static const size_t testCount = sizeof(tests) / sizeof(tests[0]);

static void (*firstTest)() = 0;
static bool skipAll = false;

void SimplifyNew_Test() {
    if (skipAll) {
        return;
    }
    size_t index = 0;
    if (firstTest) {
        while (index < testCount && tests[index].fun != firstTest) {
            ++index;
        }
    }
    bool firstTestComplete = false;
    for ( ; index < testCount; ++index) {
        SkDebugf("%s [%s]\n", __FUNCTION__, tests[index].str);
        (*tests[index].fun)();
        firstTestComplete = true;
    }
}
