#include "EdgeWalker_Test.h"
#include "Intersection_Tests.h"

static void testSimplifyQuad1() {
    SkPath path, out;
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(3, 2);
    path.lineTo(3, 3);
    path.close();
    path.moveTo(1, 0);
    path.lineTo(1, 3);
    path.lineTo(1, 3);
    path.lineTo(1, 3);
    path.close();
    testSimplify(path, true, out);
}

static void testSimplifyQuad2() {
    SkPath path, out;
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.lineTo(0, 0);
    path.lineTo(0, 2);
    path.close();
    path.moveTo(0, 1);
    path.lineTo(0, 1);
    path.lineTo(1, 1);
    path.lineTo(0, 2);
    path.close();
    testSimplify(path, true, out);
}

static void testSimplifyQuad3() {
    SkPath path, out;
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(1, 2);
    path.close();
    path.moveTo(0, 1);
    path.lineTo(1, 1);
    path.lineTo(2, 1);
    path.lineTo(0, 2);
    path.close();
    testSimplify(path, true, out);
}

static void testSimplifyQuad4() {
    SkPath path, out;
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(2, 2);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(2, 1);
    path.lineTo(3, 1);
    path.lineTo(3, 3);
    path.close();
    testSimplify(path, true, out);
}

static void testSimplifyQuad5() {
    SkPath path, out;
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(3, 2);
    path.close();
    path.moveTo(0, 1);
    path.lineTo(1, 1);
    path.lineTo(2, 1);
    path.lineTo(0, 2);
    path.close();
    testSimplify(path, true, out);
}

static void testSimplifyQuad6() {
    SkPath path, out;
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(1, 1);
    path.lineTo(3, 3);
    path.close();
    path.moveTo(1, 1);
    path.lineTo(1, 1);
    path.lineTo(1, 1);
    path.lineTo(2, 2);
    path.close();
    testSimplify(path, true, out);
}

static void (*simplifyTests[])() = {
    testSimplifyQuad6,
    testSimplifyQuad5,
    testSimplifyQuad4,
    testSimplifyQuad3,
    testSimplifyQuad2,
    testSimplifyQuad1,
};

static size_t simplifyTestsCount = sizeof(simplifyTests) / sizeof(simplifyTests[0]);

static void (*firstTest)() = 0;

void SimplifyQuadralateralPaths_Test() {
    size_t index = 0;
    if (firstTest) {
        while (index < simplifyTestsCount && simplifyTests[index] != firstTest) {
            ++index;
        }
    }
    for ( ; index < simplifyTestsCount; ++index) {
        (*simplifyTests[index])();
    }
}
