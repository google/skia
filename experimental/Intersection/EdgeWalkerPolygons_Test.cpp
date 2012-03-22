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

static void simplify(const char* functionName, const SkPath& path,
        bool fill, SkPath& out) {
    SkDebugf("%s\n", functionName);
    simplify(path, fill, out);
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
        simplify(path, true, out);
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
    simplify(__FUNCTION__, path, true, out);
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
    simplify(__FUNCTION__, path, true, out);
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
    simplify(__FUNCTION__, path, true, out);
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
    simplify(__FUNCTION__, path, true, out);
}

static void testSimplifySkinnyTriangle6() {
        SkPath path, out;
path.moveTo(591.091064, 627.534851);
path.lineTo(541.088135, 560.707642);
path.lineTo(491.085175, 493.880310);
path.lineTo(441.082214, 427.053101);
path.lineTo(591.091064, 627.534851);
path.close();
path.moveTo(317.093445, 592.013306);
path.lineTo(366.316162, 542.986572);
path.lineTo(416.051514, 486.978577);
path.lineTo(465.786865, 430.970581);
path.lineTo(317.093445, 592.013306);
path.close();
path.moveTo(289.392517, 517.138489);
path.lineTo(249.886078, 508.598022);
path.lineTo(217.110916, 450.916443);
path.lineTo(196.621033, 394.917633);
path.lineTo(289.392517, 517.138489);
path.close();
    simplify(__FUNCTION__, path, true, out);
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

static void testSimplifySkinnyTriangle7() {
        SkPath path, out;
path.moveTo(487.502319, 550.811279);
path.lineTo(448.826050, 491.720123);
path.lineTo(410.149780, 432.628967);
path.lineTo(371.473572, 373.537781);
path.lineTo(487.502319, 550.811279);
path.close();
path.moveTo(295.817108, 532.655579);
path.lineTo(342.896271, 485.912292);
path.lineTo(389.975433, 439.169006);
path.lineTo(437.054596, 392.425781);
path.lineTo(295.817108, 532.655579);
path.close();
path.moveTo(239.726822, 575.025269);
path.lineTo(204.117569, 521.429688);
path.lineTo(171.275452, 454.110382);
path.lineTo(193.328583, 397.859497);
path.lineTo(239.726822, 575.025269);
path.close();
    simplify(__FUNCTION__, path, true, out);
}

static void testSimplifySkinnyTriangle8() {
        SkPath path, out;
path.moveTo(441.943115, 511.678040);
path.lineTo(408.487549, 456.880920);
path.lineTo(375.031952, 402.083801);
path.lineTo(341.576385, 347.286682);
path.lineTo(441.943115, 511.678040);
path.close();
path.moveTo(297.548492, 557.246704);
path.lineTo(350.768494, 507.627014);
path.lineTo(403.988525, 458.007385);
path.lineTo(457.208527, 408.387695);
path.lineTo(297.548492, 557.246704);
path.close();
path.moveTo(209.857895, 615.802979);
path.lineTo(178.249481, 534.230347);
path.lineTo(144.905640, 460.056824);
path.lineTo(192.953125, 404.972900);
path.lineTo(209.857895, 615.802979);
path.close();
    simplify(__FUNCTION__, path, true, out);
}

static void testSimplifySkinnyTriangle9() {
        SkPath path, out;
path.moveTo(439.867065, 528.291931);
path.lineTo(405.413025, 469.107178);
path.lineTo(370.958954, 409.922363);
path.lineTo(336.504883, 350.737610);
path.lineTo(439.867065, 528.291931);
path.close();
path.moveTo(298.922455, 573.251953);
path.lineTo(356.360962, 521.905090);
path.lineTo(413.799438, 470.558228);
path.lineTo(471.237915, 419.211365);
path.lineTo(298.922455, 573.251953);
path.close();
path.moveTo(187.200775, 643.035156);
path.lineTo(159.713165, 540.993774);
path.lineTo(126.257164, 462.198517);
path.lineTo(193.534012, 409.266235);
path.lineTo(187.200775, 643.035156);
path.close();
path.close();
    simplify(__FUNCTION__, path, true, out);
}

static void testSimplifySkinnyTriangle10() {
        SkPath path, out;
#if 0
path.moveTo(99.270325, 239.365234);
path.lineTo(105.967056, 173.361206);
path.lineTo(148.821381, 141.309891);
path.lineTo(159.101013, 189.235138);
path.lineTo(99.270325, 239.365234);
path.close();
#endif
path.moveTo(213.673737, 413.292938);
path.lineTo(225.200134, 343.616821);
path.lineTo(236.726532, 273.940704);
path.lineTo(219.386414, 231.373322);
path.lineTo(213.673737, 413.292938);
path.close();
path.moveTo(43.485352, 308.984497);
path.lineTo(122.610657, 305.950134);
path.lineTo(201.735962, 302.915802);
path.lineTo(280.861267, 299.881470);
path.lineTo(43.485352, 308.984497);
path.close();
    simplify(__FUNCTION__, path, true, out);
}

static void (*simplifyTests[])() = {
    testSimplifySkinnyTriangle10,
    testSimplifySkinnyTriangle9,
    testSimplifySkinnyTriangle8,
    testSimplifySkinnyTriangle7,
    testSimplifySkinnyTriangle6,
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

static void (*firstTest)() = 0;

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
        if (simplifyTests[index] == testSimplifySkinnyTriangle2) {
            SkDebugf("%s last fast skinny test\n", __FUNCTION__);
        }
        firstTestComplete = true;
    }
}

