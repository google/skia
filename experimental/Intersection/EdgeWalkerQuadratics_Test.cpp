#include "EdgeWalker_Test.h"
#include "Intersection_Tests.h"

static void testSimplifyQuadratic1() {
    SkPath path, out;
    path.moveTo(0, 0);
    path.quadTo(1, 0, 1, 1);
    path.close();
    path.moveTo(1, 0);
    path.quadTo(0, 0, 0, 1);
    path.close();
    testSimplify(path, true, out);
}

static void testSimplifyQuadratic2() {
    SkPath path, out;
    path.moveTo(0, 0);
    path.quadTo(20, 0, 20, 20);
    path.close();
    path.moveTo(20, 0);
    path.quadTo(0, 0, 0, 20);
    path.close();
    testSimplify(path, true, out);
    drawAsciiPaths(path, out, true);
}

static void testSimplifyQuadratic3() {
    SkPath path, out;
    path.moveTo(0, 0);
    path.quadTo(20, 0, 20, 20);
    path.close();
    path.moveTo(0, 20);
    path.quadTo(0, 0, 20, 0);
    path.close();
    testSimplify(path, true, out);
    drawAsciiPaths(path, out, true);
}

static void (*simplifyTests[])() = {
    testSimplifyQuadratic3,
    testSimplifyQuadratic2,
    testSimplifyQuadratic1,
};

static size_t simplifyTestsCount = sizeof(simplifyTests) / sizeof(simplifyTests[0]);

static void (*firstTest)() = 0;

void SimplifyQuadraticPaths_Test() {
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
