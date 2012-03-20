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
    testSimplify(path, true, out); // expect |\/|
                                   //        |__|         
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
    testSimplify(path, true, out);
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
    testSimplify(path, true, out);
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
    testSimplify(path, true, out);
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
    testSimplify(path, true, out);
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
    testSimplify(path, true, out);
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
    testSimplify(path, true, out);
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
    testSimplify(path, true, out);
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
    testSimplify(path, true, out);
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
    testSimplify(path, true, out);
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
    testSimplify(path, true, out);
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
    testSimplify(path, true, out);
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
    testSimplify(path, true, out);
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
    testSimplify(path, true, out);
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
    testSimplify(path, true, out);
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
    testSimplify(path, true, out);
}
    
static void testSimplifyTriangle18() {
    SkPath path, out;
    path.moveTo(0, 0);
    path.lineTo(0, 1);
    path.lineTo(1, 2);
    path.close();
    path.moveTo(1, 0);
    path.lineTo(0, 1);
    path.lineTo(0, 3);
    path.close();
    testSimplify(path, true, out);
}

static void testSimplifyTriangle19() {
    SkPath path, out;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(0, 0);
    path.lineTo(0, 1);
    path.lineTo(3, 2);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(1, 1);
    path.lineTo(2, 1);
    path.close();
    testSimplify(path, true, out);
}

static void testSimplifyTriangle20() {
    SkPath path, out;
    path.moveTo(0, 0);
    path.lineTo(2, 1);
    path.lineTo(1, 3);
    path.close();
    path.moveTo(2, 0);
    path.lineTo(3, 2);
    path.lineTo(0, 3);
    path.close();
    testSimplify(path, true, out);
}

static void testSimplifyTriangle21() {
    SkPath path, out;
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(1, 2);
    path.close();
    path.moveTo(2, 0);
    path.lineTo(2, 1);
    path.lineTo(0, 3);
    path.close();
    testSimplify(path, true, out);
}

static void testSimplifyDegenerateTriangle1() {
    SkPath path, out;
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.lineTo(0, 0);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.lineTo(0, 0);
    path.close();
    testSimplify(path, true, out);
}

static void testSimplifyDegenerateTriangle2() {
    SkPath path, out;
    path.moveTo(0, 0);
    path.lineTo(1, 1);
    path.lineTo(2, 2);
    path.close();
    path.moveTo(1, 0);
    path.lineTo(2, 2);
    path.lineTo(3, 3);
    path.close();
    testSimplify(path, true, out);
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
    testSimplify(path, true, out); // expect   _
                                   //         / \     .       
}                                  //        /___\    .

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
    testSimplify(path, true, out); // expect   _
}                                  //         \ /

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
    path.close();                  //         _
    testSimplify(path, true, out); // expect | |
}                                  //        |_|

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
                            testSimplify(path, true, out);
                            path.setFillType(SkPath::kEvenOdd_FillType);
                            testSimplify(path, true, out);
                        }
                    }
                }
            }
        }
    }
}

static void testSimplifyDegenerate4x4Triangles() {
    char pathStr[1024];
    bzero(pathStr, sizeof(pathStr));
    for (int a = 0; a < 16; ++a) {
        int ax = a & 0x03;
        int ay = a >> 2;
        for (int b = a ; b < 16; ++b) {
            int bx = b & 0x03;
            int by = b >> 2;
            for (int c = a ; c < 16; ++c) {
                int cx = c & 0x03;
                int cy = c >> 2;
                bool abcIsATriangle = (bx - ax) * (cy - ay)
                        != (by - ay) * (cx - ax);
                for (int d = 0; d < 16; ++d) {
                    int dx = d & 0x03;
                    int dy = d >> 2;
                    for (int e = d ; e < 16; ++e) {
                        int ex = e & 0x03;
                        int ey = e >> 2;
                        for (int f = d ; f < 16; ++f) {
                            int fx = f & 0x03;
                            int fy = f >> 2;
                            if (abcIsATriangle && (ex - dx) * (fy - dy)
                                    != (ey - dy) * (fx - dx)) {
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
                            testSimplify(path, true, out);
                            path.setFillType(SkPath::kEvenOdd_FillType);
                            testSimplify(path, true, out);
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

static void testSimplifySkinnyTriangle1() {
    for (int x = 1; x < 255; ++x) {
        SkPath path, out;
        path.moveTo((x * 101) % 10, 0);
        path.lineTo((x * 91) % 10, 1000);
        path.lineTo((x * 71) % 10, 2000);
        path.lineTo((x * 51) % 10, 3000);
        path.close();
        path.moveTo((x * 101) % 20, 0);
        path.lineTo((x * 91) % 20, 1000);
        path.lineTo((x * 71) % 20, 2000);
        path.lineTo((x * 51) % 20, 3000);
        path.close();
        path.moveTo((x * 101) % 30, 0);
        path.lineTo((x * 91) % 30, 1000);
        path.lineTo((x * 71) % 30, 2000);
        path.lineTo((x * 51) % 30, 3000);
        path.close();
    testSimplify(path, true, out);
    }
}

static void testSimplifySkinnyTriangle2() {
        SkPath path, out;
#if 01
path.moveTo(591.091064, 627.534851);
path.lineTo(541.088135, 560.707642);
path.lineTo(491.085175, 493.880310);
path.lineTo(441.082214, 427.053101);
//path.lineTo(591.091064, 627.534851);
path.close();
#endif
path.moveTo(317.093445, 592.013306);
path.lineTo(366.316162, 542.986572);
path.lineTo(416.051514, 486.978577);
path.lineTo(465.786865, 430.970581);
//path.lineTo(317.093445, 592.013306);
path.close();
#if 0
path.moveTo(289.392517, 517.138489);
path.lineTo(249.886078, 508.598022);
path.lineTo(217.110916, 450.916443);
path.lineTo(196.621033, 394.917633);
//path.lineTo(289.392517, 517.138489);
path.close();
#endif
    testSimplify(path, true, out);
}

static void testSimplifySkinnyTriangle3() {
        SkPath path, out;
        path.moveTo(591, 627.534851);
        path.lineTo(541, 560.707642);
        path.lineTo(491, 493.880310);
        path.lineTo(441, 427.053101);
        path.close();
        path.moveTo(317, 592.013306);
        path.lineTo(366, 542.986572);
        path.lineTo(416, 486.978577);
        path.lineTo(465, 430.970581);
        path.close();
    testSimplify(path, true, out);
}

static void testSimplifySkinnyTriangle4() {
        SkPath path, out;
path.moveTo(572.655212, 614.959961);
path.lineTo(524.618896, 549.339600);
path.lineTo(476.582581, 483.719269);
path.lineTo(428.546265, 418.098938);
path.lineTo(572.655212, 614.959961);
path.close();
path.moveTo(312.166382, 583.723083);
path.lineTo(361.047791, 529.824219);
path.lineTo(409.929230, 475.925354);
path.lineTo(458.810669, 422.026520);
path.lineTo(312.166382, 583.723083);
path.close();
path.moveTo(278.742737, 508.065643);
path.lineTo(241.475800, 493.465118);
path.lineTo(210.344177, 437.315125);
path.lineTo(197.019455, 383.794556);
path.lineTo(278.742737, 508.065643);
path.close();
    testSimplify(path, true, out);
}

static void testSimplifySkinnyTriangle5() {
        SkPath path, out;
path.moveTo(554.690613, 602.286072);
path.lineTo(508.590057, 537.906250);
path.lineTo(462.489441, 473.526520);
path.lineTo(416.388855, 409.146729);
path.lineTo(554.690613, 602.286072);
path.close();
path.moveTo(307.216949, 575.189270);
path.lineTo(355.826965, 516.804688);
path.lineTo(403.815918, 464.990753);
path.lineTo(451.804871, 413.176819);
path.lineTo(307.216949, 575.189270);
path.close();
path.moveTo(271.998901, 521.301025);
path.lineTo(234.619705, 499.687683);
path.lineTo(203.059692, 441.332336);
path.lineTo(195.994370, 386.856506);
path.lineTo(271.998901, 521.301025);
path.close();
    testSimplify(path, true, out);
}


static void testSimplifyTriangle22() {
    SkPath path, out;
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(0, 2);
    path.close();
    path.moveTo(1, 0);
    path.lineTo(0, 2);
    path.lineTo(0, 1);
    path.close();
    testSimplify(path, true, out);
}

static void testSimplifyTriangle23() {
    SkPath path, out;
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.lineTo(0, 0);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 1);
    path.lineTo(1, 2);
    path.close();
    testSimplify(path, true, out);
}

static void testSimplifyTriangle24() {
    SkPath path, out;
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.lineTo(0, 1);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(0, 1);
    path.close();
    testSimplify(path, true, out);
}

static void (*simplifyTests[])() = {
    testSimplifySkinnyTriangle5,
    testSimplifySkinnyTriangle4,
    testSimplifySkinnyTriangle3,
    testSimplifySkinnyTriangle2,
    testSimplifySkinnyTriangle1,
    testSimplifyTriangle24,
    testSimplifyTriangle23,
    testSimplifyTriangle22,
    testSimplifyDegenerateTriangle2,
    testSimplifyDegenerateTriangle1,
    testSimplifyTriangle21,
    testSimplifyTriangle20,
    testSimplifyTriangle19,
    testSimplifyTriangle18,
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
    testSimplifyDegenerate4x4Triangles,
    testSimplifyNondegenerate4x4Triangles,
    testPathTriangleRendering,
};

static size_t simplifyTestsCount = sizeof(simplifyTests) / sizeof(simplifyTests[0]);

static void (*firstTest)() = testSimplifySkinnyTriangle4;

void SimplifyPolygonPaths_Test() {
    size_t index = 0;
    if (firstTest) {
        while (index < simplifyTestsCount && simplifyTests[index] != firstTest) {
            ++index;
        }
    }
    bool firstTestComplete = false;
    for ( ; index < simplifyTestsCount; ++index) {
        (*simplifyTests[index])();
        firstTestComplete = true;
    }
}

