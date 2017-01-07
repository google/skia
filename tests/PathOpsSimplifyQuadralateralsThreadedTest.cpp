/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "PathOpsExtendedTest.h"
#include "PathOpsThreadedCommon.h"
#include "SkString.h"

static int loopNo = 1;

static void testSimplifyQuadralateralsMain(PathOpsThreadState* data)
{
    SkASSERT(data);
    PathOpsThreadState& state = *data;
    SkString pathStr;
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
                    if (state.fReporter->verbose()) {
                        pathStr.printf("static void quadralateralSimplify%d(skiatest::Reporter*"
                                "reporter, const char* filename) {\n", loopNo);
                        pathStr.appendf("    SkPath path;\n");
                        pathStr.appendf("    path.moveTo(%d, %d);\n", ax, ay);
                        pathStr.appendf("    path.lineTo(%d, %d);\n", bx, by);
                        pathStr.appendf("    path.lineTo(%d, %d);\n", cx, cy);
                        pathStr.appendf("    path.lineTo(%d, %d);\n", dx, dy);
                        pathStr.appendf("    path.close();\n");
                        pathStr.appendf("    path.moveTo(%d, %d);\n", ex, ey);
                        pathStr.appendf("    path.lineTo(%d, %d);\n", fx, fy);
                        pathStr.appendf("    path.lineTo(%d, %d);\n", gx, gy);
                        pathStr.appendf("    path.lineTo(%d, %d);\n", hx, hy);
                        pathStr.appendf("    path.close();\n");
                        pathStr.appendf("    testPathSimplify(reporter, path, filename);\n");
                        pathStr.appendf("}\n");
                        state.outputProgress(pathStr.c_str(), SkPath::kWinding_FillType);
                    }
                    testSimplify(path, false, out, state, pathStr.c_str());
                    path.setFillType(SkPath::kEvenOdd_FillType);
                    if (state.fReporter->verbose()) {
                        state.outputProgress(pathStr.c_str(), SkPath::kEvenOdd_FillType);
                    }
                    testSimplify(path, true, out, state, pathStr.c_str());
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
                    *testRunner.fRunnables.append() = new PathOpsThreadedRunnable(
                            &testSimplifyQuadralateralsMain, a, b, c, d, &testRunner);
                }
                if (!reporter->allowExtendedTest()) goto finish;
            }
        }
    }
finish:
    testRunner.render();
}
