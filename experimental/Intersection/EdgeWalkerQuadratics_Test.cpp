#include "EdgeWalker_Test.h"
#include "Intersection_Tests.h"
#include "SkBitmap.h"

static SkBitmap bitmap;

static void testSimplifyQuadratic1() {
    SkPath path, out;
    path.moveTo(0, 0);
    path.quadTo(1, 0, 1, 1);
    path.close();
    path.moveTo(1, 0);
    path.quadTo(0, 0, 0, 1);
    path.close();
    testSimplify(path, true, out, bitmap);
}

static void testSimplifyQuadratic2() {
    SkPath path, out;
    path.moveTo(0, 0);
    path.quadTo(20, 0, 20, 20);
    path.close();
    path.moveTo(20, 0);
    path.quadTo(0, 0, 0, 20);
    path.close();
    testSimplify(path, true, out, bitmap);
}

static void testSimplifyQuadratic3() {
    SkPath path, out;
    path.moveTo(0, 0);
    path.quadTo(20, 0, 20, 20);
    path.close();
    path.moveTo(0, 20);
    path.quadTo(0, 0, 20, 0);
    path.close();
    testSimplify(path, true, out, bitmap);
}

static void testSimplifyQuadratic4() {
    SkPath path, out;
    path.moveTo(0, 20);
    path.quadTo(20, 0, 40, 20);
    path.close();
    path.moveTo(40, 10);
    path.quadTo(20, 30, 0, 10);
    path.close();
    testSimplify(path, true, out, bitmap);
    drawAsciiPaths(path, out, true);
}

static void testSimplifyQuadratic5() {
    SkPath path, out;
    path.moveTo(0, 0);
    path.quadTo(0, 0, 0, 0);
    path.lineTo(0, 0);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(0, 0, 0, 1);
    path.close();
    testSimplify(path, true, out, bitmap);
    drawAsciiPaths(path, out, true);
}

static void testSimplifyQuadratic6() {
    SkPath path, out;
    path.moveTo(0, 0);
    path.quadTo(0, 0, 0, 0);
    path.lineTo(1, 0);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(1, 0, 0, 1);
    path.close();
    testSimplify(path, true, out, bitmap);
    drawAsciiPaths(path, out, true);
}

static void testSimplifyQuadratic7() {
    SkPath path, out;
    path.moveTo(0, 0);
    path.quadTo(0, 0, 0, 0);
    path.lineTo(0, 1);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(1, 0, 0, 2);
    path.close();
    testSimplify(path, true, out, bitmap);
    drawAsciiPaths(path, out, true);
}

static void testSimplifyQuadratic8() {
    SkPath path, out;
    path.moveTo(0, 0);
    path.quadTo(0, 0, 0, 0);
    path.lineTo(0, 0);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(1, 0, 0, 2);
    path.close();
    testSimplify(path, true, out, bitmap);
    drawAsciiPaths(path, out, true);
}

static void testSimplifyQuadratic9() {
    SkPath path, out;
    path.moveTo(0, 0);
    path.quadTo(0, 0, 0, 0);
    path.lineTo(1, 1);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(1, 0, 2, 2);
    path.close();
    testSimplify(path, true, out, bitmap);
    drawAsciiPaths(path, out, true);
}

static void testSimplifyQuadratic10() {
    SkPath path, out;
    path.moveTo(0, 0);
    path.quadTo(0, 0, 0, 0);
    path.lineTo(0, 0);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 1);
    path.quadTo(1, 1, 1, 2);
    path.close();
    testSimplify(path, true, out, bitmap);
    drawAsciiPaths(path, out, true);
}

static void testSimplifyQuadratic11() {
    SkPath path, out;
    path.moveTo(0, 0);
    path.quadTo(0, 0, 0, 0);
    path.lineTo(0, 2);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(2, 1);
    path.quadTo(2, 2, 3, 3);
    path.close();
    testSimplify(path, true, out, bitmap);
    drawAsciiPaths(path, out, true);
}

static void testSimplifyQuadratic12() {
    SkPath path, out;
    path.moveTo(0, 0);
    path.lineTo(0, 2);
    path.lineTo(0, 0);
    path.close();
    path.moveTo(3, 0);
    path.quadTo(1, 1, 0, 2);
    path.lineTo(3, 0);
    path.close();
    testSimplify(path, true, out, bitmap);
    drawAsciiPaths(path, out, true);
}

static void testSimplifyQuadratic13() {
    SkPath path, out;
path.moveTo(0, 0);
path.quadTo(0, 0, 1, 0);
path.lineTo(1, 1);
path.lineTo(0, 0);
path.close();
path.moveTo(0, 0);
path.quadTo(3, 0, 1, 1);
path.lineTo(0, 0);
path.close();
    testSimplify(path, true, out, bitmap);
    drawAsciiPaths(path, out, true);
}

static void testSimplifyQuadratic14() {
    SkPath path, out;
    path.moveTo(0, 0);
    path.quadTo(0, 0, 0, 0);
    path.lineTo(1, 1);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(0, 1, 2, 1);
    path.close();
    testSimplify(path, true, out, bitmap);
    drawAsciiPaths(path, out, true);
}

static void testSimplifyQuadratic15() {
    SkPath path, out;
    path.moveTo(0, 0);
    path.quadTo(0, 0, 1, 3);
    path.lineTo(3, 3);
    path.close();
    path.moveTo(0, 1);
    path.lineTo(1, 1);
    path.quadTo(0, 3, 3, 3);
    path.close();
    testSimplify(path, true, out, bitmap);
    drawAsciiPaths(path, out, true);
}

static void testSimplifyQuadratic16() {
    SkPath path, out;
    path.moveTo(0, 0);
    path.quadTo(0, 0, 0, 0);
    path.lineTo(0, 1);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.quadTo(1, 0, 0, 1);
    path.close();
    testSimplify(path, true, out, bitmap);
    drawAsciiPaths(path, out, true);
}

static void (*simplifyTests[])() = {
    testSimplifyQuadratic16,
    testSimplifyQuadratic15,
    testSimplifyQuadratic14,
    testSimplifyQuadratic13,
    testSimplifyQuadratic12,
    testSimplifyQuadratic11,
    testSimplifyQuadratic10,
    testSimplifyQuadratic9,
    testSimplifyQuadratic8,
    testSimplifyQuadratic7,
    testSimplifyQuadratic6,
    testSimplifyQuadratic5,
    testSimplifyQuadratic4,
    testSimplifyQuadratic3,
    testSimplifyQuadratic2,
    testSimplifyQuadratic1,
};

static size_t simplifyTestsCount = sizeof(simplifyTests) / sizeof(simplifyTests[0]);

static void (*firstTest)() = 0;
static bool skipAll = false;

void SimplifyQuadraticPaths_Test() {
    if (skipAll) {
        return;
    }
    size_t index = 0;
    if (firstTest) {
        while (index < simplifyTestsCount && simplifyTests[index] != firstTest) {
            ++index;
        }
    }
    bool firstTestComplete = false;
    for ( ; index < simplifyTestsCount; ++index) {
        (*simplifyTests[index])();
        if (simplifyTests[index] == testSimplifyQuadratic1) {
            SkDebugf("%s last fast quad test\n", __FUNCTION__);
        }
        firstTestComplete = true;
    }
}
