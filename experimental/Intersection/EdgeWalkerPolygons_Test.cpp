#include "EdgeWalker_Test.h"
#include "Intersection_Tests.h"

static void testSimplifyTriangle() {
    SkPath path, out;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(10,10); // triangle      |\      .
    path.lineTo(10,30); //               |_\     .
    path.lineTo(20,30);
    path.close();
    path.moveTo(20,10); // triangle        /|
    path.lineTo(10,30); //                /_|
    path.lineTo(20,30);
    path.close();
    simplify(path, true, out); // expect |\/|
    comparePaths(path, out);   //        |__|         
}

static void testSimplifyTriangle3() {
    SkPath path, out;
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(0, 1);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(3, 1);
    path.close();
    simplify(path, true, out);
    comparePaths(path, out);
}

static void testSimplifyTriangle4() {
    SkPath path, out;
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(0, 1);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(2, 1);
    path.close();
    simplify(path, true, out);
    comparePaths(path, out);
}

static void testSimplifyTriangle5() {
    SkPath path, out;
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(0, 1);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(1, 1);
    path.lineTo(2, 1);
    path.close();
    simplify(path, true, out);
    comparePaths(path, out);
}

static void testSimplifyTriangle6() {
    SkPath path, out;
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(0, 1);
    path.lineTo(0, 0);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 1);
    path.lineTo(3, 1);
    path.lineTo(0, 0);
    path.close();
    simplify(path, true, out);
    comparePaths(path, out);
}

static void testSimplifyTriangle7() {
    SkPath path, out;
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(0, 1);
    path.lineTo(0, 0);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(1, 1);
    path.lineTo(0, 2);
    path.lineTo(0, 0);
    path.close();
    simplify(path, true, out);
    comparePaths(path, out);
}

static void testSimplifyTriangle8() {
    SkPath path, out;
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(0, 1);
    path.lineTo(0, 0);
    path.close();
    path.moveTo(0, 1);
    path.lineTo(1, 2);
    path.lineTo(1, 3);
    path.lineTo(0, 1);
    path.close();
    simplify(path, true, out);
    comparePaths(path, out);
}

static void testSimplifyTriangle9() {
    SkPath path, out;
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(1, 1);
    path.lineTo(0, 0);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(1, 1);
    path.lineTo(2, 1);
    path.lineTo(0, 0);
    path.close();
    simplify(path, true, out);
    comparePaths(path, out);
}

static void testSimplifyTriangle10() {
    SkPath path, out;
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(1, 1);
    path.lineTo(0, 0);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(2, 0);
    path.lineTo(0, 1);
    path.lineTo(0, 0);
    path.close();
    simplify(path, true, out);
    comparePaths(path, out);
}

static void testSimplifyTriangle11() {
    SkPath path, out;
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(0, 2);
    path.lineTo(0, 0);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(2, 1);
    path.lineTo(2, 2);
    path.lineTo(0, 0);
    path.close();
    simplify(path, true, out);
    comparePaths(path, out);
}

static void testSimplifyTriangle12() {
    SkPath path, out;
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(1, 2);
    path.lineTo(0, 0);
    path.close();
    path.moveTo(2, 0);
    path.lineTo(0, 3);
    path.lineTo(1, 1);
    path.lineTo(2, 0);
    path.close();
    simplify(path, true, out);
    comparePaths(path, out);
}

static void testSimplifyTriangle13() {
    SkPath path, out;
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(0, 3);
    path.lineTo(0, 0);
    path.close();
    path.moveTo(3, 0);
    path.lineTo(0, 3);
    path.lineTo(1, 1);
    path.lineTo(3, 0);
    path.close();
    simplify(path, true, out);
    comparePaths(path, out);
}

static void testSimplifyTriangle14() {
    SkPath path, out;
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(0, 1);
    path.lineTo(0, 0);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(0, 1);
    path.lineTo(0, 0);
    path.close();
    simplify(path, true, out);
    comparePaths(path, out);
}

static void testSimplifyTriangle15() {
    SkPath path, out;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(0, 0);
    path.lineTo(0, 1);
    path.lineTo(1, 2);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 1);
    path.lineTo(2, 2);
    path.close();
    simplify(path, true, out);
    comparePaths(path, out);
}

static void testSimplifyTriangle16() {
    SkPath path, out;
    path.moveTo(0, 0);
    path.lineTo(0, 1);
    path.lineTo(1, 2);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 1);
    path.lineTo(1, 3);
    path.close();
    simplify(path, true, out);
    comparePaths(path, out);
}

static void testSimplifyTriangle17() {
    SkPath path, out;
    path.moveTo(0, 0);
    path.lineTo(0, 1);
    path.lineTo(1, 2);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(1, 3);
    path.lineTo(0, 1);
    path.close();
    simplify(path, true, out);
    comparePaths(path, out);
}
    
static void testSimplifyWindingParallelogram() {
    SkPath path, out;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(20,10); // parallelogram  _ 
    path.lineTo(30,30); //               \ \      .
    path.lineTo(40,30); //                \_\     .
    path.lineTo(30,10);
    path.close();
    path.moveTo(20,10); // parallelogram   _
    path.lineTo(10,30); //                / /
    path.lineTo(20,30); //               /_/
    path.lineTo(30,10);
    path.close();
    simplify(path, true, out); // expect   _
    comparePaths(path, out);   //         / \     .       
}                              //        /___\    .

static void testSimplifyXorParallelogram() {
    SkPath path, out;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(20,10); // parallelogram  _ 
    path.lineTo(30,30); //               \ \      .
    path.lineTo(40,30); //                \_\     .
    path.lineTo(30,10);
    path.close();
    path.moveTo(20,10); // parallelogram   _
    path.lineTo(10,30); //                / /
    path.lineTo(20,30); //               /_/
    path.lineTo(30,10);
    path.close();
    simplify(path, true, out); // expect   _
    comparePaths(path, out);   //         \ /
}                              //

static void testSimplifyTriangle2() {
    SkPath path, out;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(10,10); // triangle      |\      .
    path.lineTo(10,30); //               |_\     .
    path.lineTo(20,30);
    path.close();
    path.moveTo(10,10); // triangle       _ 
    path.lineTo(20,10); //               \ |
    path.lineTo(20,30); //                \|
    path.close();              //         _
    simplify(path, true, out); // expect | |
    comparePaths(path, out);   //        |_|         
}

static void testSimplifyNondegenerate4x4Triangles() {
    char pathStr[1024];
    bzero(pathStr, sizeof(pathStr));
    for (int a = 0; a < 15; ++a) {
        int ax = a & 0x03;
        int ay = a >> 2;
        for (int b = a + 1; b < 16; ++b) {
            int bx = b & 0x03;
            int by = b >> 2;
            for (int c = a + 1; c < 16; ++c) {
                if (b == c) {
                    continue;
                }
                int cx = c & 0x03;
                int cy = c >> 2;
                if ((bx - ax) * (cy - ay) == (by - ay) * (cx - ax)) {
                    continue;
                }
                for (int d = 0; d < 15; ++d) {
                    int dx = d & 0x03;
                    int dy = d >> 2;
                    for (int e = d + 1; e < 16; ++e) {
                        int ex = e & 0x03;
                        int ey = e >> 2;
                        for (int f = d + 1; f < 16; ++f) {
                            if (e == f) {
                                continue;
                            }
                            int fx = f & 0x03;
                            int fy = f >> 2;
                            if ((ex - dx) * (fy - dy) == (ey - dy) * (fx - dx)) {
                                continue;
                            }
                            SkPath path, out;
                            path.setFillType(SkPath::kWinding_FillType);
                            path.moveTo(ax, ay);
                            path.lineTo(bx, by);
                            path.lineTo(cx, cy);
                            path.close();
                            path.moveTo(dx, dy);
                            path.lineTo(ex, ey);
                            path.lineTo(fx, fy);
                            path.close();
                            if (1) {
                                char* str = pathStr;
                                str += sprintf(str, "    path.moveTo(%d, %d);\n", ax, ay);
                                str += sprintf(str, "    path.lineTo(%d, %d);\n", bx, by);
                                str += sprintf(str, "    path.lineTo(%d, %d);\n", cx, cy);
                                str += sprintf(str, "    path.close();\n");
                                str += sprintf(str, "    path.moveTo(%d, %d);\n", dx, dy);
                                str += sprintf(str, "    path.lineTo(%d, %d);\n", ex, ey);
                                str += sprintf(str, "    path.lineTo(%d, %d);\n", fx, fy);
                                str += sprintf(str, "    path.close();");
                            }
                            simplify(path, true, out);
                            comparePaths(path, out);
                            path.setFillType(SkPath::kEvenOdd_FillType);
                            simplify(path, true, out);
                            comparePaths(path, out);
                        }
                    }
                }
            }
        }
    }
}

static void testPathTriangleRendering() {
    SkPath one, two;
    one.moveTo(0, 0);
    one.lineTo(3, 3);
    one.lineTo(0, 3);
    one.lineTo(1, 2);
    one.close();
    for (float x = .1f; x <= 2.9f; x += .1f) {
        SkDebugf("%s x=%g\n", __FUNCTION__, x);
        two.moveTo(0, 0);
        two.lineTo(x, x);
        two.lineTo(3, 3);
        two.lineTo(0, 3);
        two.lineTo(1, 2);
        two.close();
        comparePaths(one, two);
        two.reset();
    }
}

static void (*simplifyTests[])() = {
    testSimplifyTriangle17,
    testSimplifyTriangle16,
    testSimplifyTriangle15,
    testSimplifyTriangle14,
    testSimplifyTriangle13,
    testSimplifyTriangle12,
    testSimplifyTriangle11,
    testSimplifyTriangle10,
    testSimplifyTriangle7,
    testSimplifyTriangle9,
    testSimplifyTriangle8,
    testSimplifyTriangle6,
    testSimplifyTriangle5,
    testSimplifyTriangle4,
    testSimplifyTriangle3,
    testSimplifyTriangle,
    testSimplifyTriangle2,
    testSimplifyWindingParallelogram,
    testSimplifyXorParallelogram,
    testSimplifyNondegenerate4x4Triangles,
    testPathTriangleRendering,
};

static size_t simplifyTestsCount = sizeof(simplifyTests) / sizeof(simplifyTests[0]);

static void (*firstTest)() = 0;

void SimplifyPolygonPaths_Test() {
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

