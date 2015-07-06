/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "PathOpsExtendedTest.h"
#include "PathOpsThreadedCommon.h"

static void testSimplifyQuadsMain(PathOpsThreadState* data)
{
    SkASSERT(data);
    PathOpsThreadState& state = *data;
    char pathStr[1024];
    bool progress = state.fReporter->verbose(); // FIXME: break out into its own parameter?
    if (progress) {
        sk_bzero(pathStr, sizeof(pathStr));
    }
    int ax = state.fA & 0x03;
    int ay = state.fA >> 2;
    int bx = state.fB & 0x03;
    int by = state.fB >> 2;
    int cx = state.fC & 0x03;
    int cy = state.fC >> 2;
    int dx = state.fD & 0x03;
    int dy = state.fD >> 2;
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
                    path.moveTo(SkIntToScalar(ax), SkIntToScalar(ay));
                    path.quadTo(SkIntToScalar(bx), SkIntToScalar(by),
                            SkIntToScalar(cx), SkIntToScalar(cy));
                    path.lineTo(SkIntToScalar(dx), SkIntToScalar(dy));
                    path.close();
                    path.moveTo(SkIntToScalar(ex), SkIntToScalar(ey));
                    path.lineTo(SkIntToScalar(fx), SkIntToScalar(fy));
                    path.quadTo(SkIntToScalar(gx), SkIntToScalar(gy),
                            SkIntToScalar(hx), SkIntToScalar(hy));
                    path.close();
                    if (progress) {
                        static int quadTest = 66;
                        char* str = pathStr;
                        str += sprintf(str, "static void testQuads%d(skiatest::Reporter* reporter,"
                                "const char* filename) {\n", quadTest);
                        str += sprintf(str, "    SkPath path;\n");
                        str += sprintf(str, "    path.moveTo(%d, %d);\n", ax, ay);
                        str += sprintf(str, "    path.quadTo(%d, %d, %d, %d);\n", bx, by, cx, cy);
                        str += sprintf(str, "    path.lineTo(%d, %d);\n", dx, dy);
                        str += sprintf(str, "    path.close();\n");
                        str += sprintf(str, "    path.moveTo(%d, %d);\n", ex, ey);
                        str += sprintf(str, "    path.lineTo(%d, %d);\n", fx, fy);
                        str += sprintf(str, "    path.quadTo(%d, %d, %d, %d);\n", gx, gy, hx, hy);
                        str += sprintf(str, "    path.close();\n");
                        str += sprintf(str, "    testSimplify(reporter, path, filename);\n");
                        str += sprintf(str, "}\n");
                        outputProgress(state.fPathStr, pathStr, SkPath::kWinding_FillType);
                    }
                    testSimplify(path, false, out, state, pathStr);
                    path.setFillType(SkPath::kEvenOdd_FillType);
                    if (progress) {
                        outputProgress(state.fPathStr, pathStr, SkPath::kEvenOdd_FillType);
                    }
                    testSimplify(path, true, out, state, pathStr);
                }
            }
        }
    }
}

DEF_TEST(PathOpsSimplifyQuadsThreaded, reporter) {
    initializeTests(reporter, "testQuads");
    PathOpsThreadedTestRunner testRunner(reporter);
    int a = 0;
    for (; a < 16; ++a) {
        for (int b = a ; b < 16; ++b) {
            for (int c = b ; c < 16; ++c) {
                for (int d = c; d < 16; ++d) {
                    *testRunner.fRunnables.append() = SkNEW_ARGS(PathOpsThreadedRunnable,
                            (&testSimplifyQuadsMain, a, b, c, d, &testRunner));
                }
                if (!reporter->allowExtendedTest()) goto finish;
            }
        }
    }
finish:
    testRunner.render();
    ShowTestArray("testQuads");
}
