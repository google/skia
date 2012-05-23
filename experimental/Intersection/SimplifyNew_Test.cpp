/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Simplify.h"

namespace SimplifyNewTest {

#include "Simplify.cpp"

} // end of SimplifyNewTest namespace

#include "EdgeWalker_Test.h"
#include "Intersection_Tests.h"

static SkBitmap bitmap;

static bool testSimplifyx(const SkPath& path, SkPath& out, SkBitmap& bitmap) {
    if (false) {
        showPath(path);
    }
    simplifyx(path, out);
    if (false) {
        return true;
    }
    return comparePaths(path, out, bitmap, 0) == 0;
}

static void testLine1() {
    SkPath path, simple;
    path.moveTo(2,0);
    path.lineTo(1,1);
    path.lineTo(0,0);
    path.close();
    testSimplifyx(path, simple, bitmap);
}

static void addInnerCWTriangle(SkPath& path) {
    path.moveTo(3,0);
    path.lineTo(4,1);
    path.lineTo(2,1);
    path.close();
}

#if DEBUG_UNUSED
static void addInnerCCWTriangle(SkPath& path) {
    path.moveTo(3,0);
    path.lineTo(2,1);
    path.lineTo(4,1);
    path.close();
}
#endif

static void addOuterCWTriangle(SkPath& path) {
    path.moveTo(3,0);
    path.lineTo(6,2);
    path.lineTo(0,2);
    path.close();
}

#if DEBUG_UNUSED
static void addOuterCCWTriangle(SkPath& path) {
    path.moveTo(3,0);
    path.lineTo(0,2);
    path.lineTo(6,2);
    path.close();
}
#endif

static void testLine2() {
    SkPath path, simple;
    addInnerCWTriangle(path);
    addOuterCWTriangle(path);
    testSimplifyx(path, simple, bitmap);
}


static void (*tests[])() = {
    testLine1,
    testLine2,
};

static const size_t testCount = sizeof(tests) / sizeof(tests[0]);

static void (*firstTest)() = testLine2;
static bool skipAll = false;

void SimplifyNew_Test() {
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
