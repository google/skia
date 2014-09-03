/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "PathOpsExtendedTest.h"
#include "PathOpsThreadedCommon.h"

static void testSimplifyQuadralateralsMain(PathOpsThreadState* data)
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
                    path.lineTo(SkIntToScalar(bx), SkIntToScalar(by));
                    path.lineTo(SkIntToScalar(cx), SkIntToScalar(cy));
                    path.lineTo(SkIntToScalar(dx), SkIntToScalar(dy));
                    path.close();
                    path.moveTo(SkIntToScalar(ex), SkIntToScalar(ey));
                    path.lineTo(SkIntToScalar(fx), SkIntToScalar(fy));
                    path.lineTo(SkIntToScalar(gx), SkIntToScalar(gy));
                    path.lineTo(SkIntToScalar(hx), SkIntToScalar(hy));
                    path.close();
                    if (progress) {
                       // gdb: set print elements 400
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
                        str += sprintf(str, "    path.close();\n");
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

DEF_TEST(PathOpsSimplifyQuadralateralsThreaded, reporter) {
    initializeTests(reporter, "testQuadralaterals");
    PathOpsThreadedTestRunner testRunner(reporter);
    for (int a = 0; a < 16; ++a) {
        for (int b = a ; b < 16; ++b) {
            for (int c = b ; c < 16; ++c) {
                for (int d = c; d < 16; ++d) {
                    *testRunner.fRunnables.append() = SkNEW_ARGS(PathOpsThreadedRunnable,
                            (&testSimplifyQuadralateralsMain, a, b, c, d, &testRunner));
                }
                if (!reporter->allowExtendedTest()) goto finish;
            }
        }
    }
finish:
    testRunner.render();
}
