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

static void testSimplify4x4Quadralaterals() {
    char pathStr[1024];
    bzero(pathStr, sizeof(pathStr));
    for (int a = 0; a < 16; ++a) {
        int ax = a & 0x03;
        int ay = a >> 2;
        for (int b = a ; b < 16; ++b) {
            int bx = b & 0x03;
            int by = b >> 2;
            for (int c = b ; c < 16; ++c) {
                int cx = c & 0x03;
                int cy = c >> 2;
                for (int d = c; d < 16; ++d) {
                    int dx = d & 0x03;
                    int dy = d >> 2;
                    for (int e = 0 ; e < 16; ++e) {
                        int ex = e & 0x03;
                        int ey = e >> 2;
                        for (int f = e ; f < 16; ++f) {
                            int fx = f & 0x03;
                            int fy = f >> 2;
                            for (int g = f ; g < 16; ++g) {
                                int gx = g & 0x03;
                                int gy = g >> 2;
                                for (int h = g ; h < 16; ++h) {
                                    int hx = h & 0x03;
                                    int hy = h >> 2;
        SkPath path, out;
        path.setFillType(SkPath::kWinding_FillType);
        path.moveTo(ax, ay);
        path.lineTo(bx, by);
        path.lineTo(cx, cy);
        path.lineTo(dx, dy);
        path.close();
        path.moveTo(ex, ey);
        path.lineTo(fx, fy);
        path.lineTo(gx, gy);
        path.lineTo(hx, hy);
        path.close();
        if (1) {  // gdb: set print elements 400
            char* str = pathStr;
            str += sprintf(str, "    path.moveTo(%d, %d);\n", ax, ay);
            str += sprintf(str, "    path.lineTo(%d, %d);\n", bx, by);
            str += sprintf(str, "    path.lineTo(%d, %d);\n", cx, cy);
            str += sprintf(str, "    path.lineTo(%d, %d);\n", dx, dy);
            str += sprintf(str, "    path.close();\n");
            str += sprintf(str, "    path.moveTo(%d, %d);\n", ex, ey);
            str += sprintf(str, "    path.lineTo(%d, %d);\n", fx, fy);
            str += sprintf(str, "    path.lineTo(%d, %d);\n", gx, gy);
            str += sprintf(str, "    path.lineTo(%d, %d);\n", hx, hy);
            str += sprintf(str, "    path.close();");
        }
        if (!testSimplify(path, true, out)) {
            SkDebugf("*/\n{ SkPath::kWinding_FillType, %d, %d, %d, %d, %d, %d, %d, %d },\n/*\n",
                    a, b, c, d, e, f, g, h);
        }
        path.setFillType(SkPath::kEvenOdd_FillType);
        if (!testSimplify(path, true, out)) {
            SkDebugf("*/\n{ SkPath::kEvenOdd_FillType, %d, %d, %d, %d, %d, %d, %d, %d },\n/*\n",
                    a, b, c, d, e, f, g, h);
        }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}



static void (*simplifyTests[])() = {
    testSimplifyQuad6,
    testSimplifyQuad5,
    testSimplifyQuad4,
    testSimplifyQuad3,
    testSimplifyQuad2,
    testSimplifyQuad1,
    testSimplify4x4Quadralaterals,
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
