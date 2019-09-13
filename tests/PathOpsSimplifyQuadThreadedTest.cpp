/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkString.h"
#include "tests/PathOpsExtendedTest.h"
#include "tests/PathOpsThreadedCommon.h"

static int quadTest = 66;

static void testSimplifyQuadsMain(PathOpsThreadState* data)
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
                    path.quadTo(SkIntToScalar(bx), SkIntToScalar(by),
                            SkIntToScalar(cx), SkIntToScalar(cy));
                    path.lineTo(SkIntToScalar(dx), SkIntToScalar(dy));
                    path.close();
                    path.moveTo(SkIntToScalar(ex), SkIntToScalar(ey));
                    path.lineTo(SkIntToScalar(fx), SkIntToScalar(fy));
                    path.quadTo(SkIntToScalar(gx), SkIntToScalar(gy),
                            SkIntToScalar(hx), SkIntToScalar(hy));
                    path.close();
                    if (state.fReporter->verbose()) {
                        pathStr.printf("static void testQuads%d(skiatest::Reporter* reporter,"
                                "const char* filename) {\n", quadTest);
                        pathStr.appendf("    SkPath path;\n");
                        pathStr.appendf("    path.moveTo(%d, %d);\n", ax, ay);
                        pathStr.appendf("    path.quadTo(%d, %d, %d, %d);\n", bx, by, cx, cy);
                        pathStr.appendf("    path.lineTo(%d, %d);\n", dx, dy);
                        pathStr.appendf("    path.close();\n");
                        pathStr.appendf("    path.moveTo(%d, %d);\n", ex, ey);
                        pathStr.appendf("    path.lineTo(%d, %d);\n", fx, fy);
                        pathStr.appendf("    path.quadTo(%d, %d, %d, %d);\n", gx, gy, hx, hy);
                        pathStr.appendf("    path.close();\n");
                        pathStr.appendf("    testSimplify(reporter, path, filename);\n");
                        pathStr.appendf("}\n");
                        state.outputProgress(pathStr.c_str(), SkPathFillType::kWinding);
                    }
                    testSimplify(path, false, out, state, pathStr.c_str());
                    path.setFillType(SkPath::kEvenOdd_FillType);
                    if (state.fReporter->verbose()) {
                        state.outputProgress(pathStr.c_str(), SkPathFillType::kEvenOdd);
                    }
                    testSimplify(path, true, out, state, pathStr.c_str());
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
                    *testRunner.fRunnables.append() = new PathOpsThreadedRunnable(
                            &testSimplifyQuadsMain, a, b, c, d, &testRunner);
                }
                if (!reporter->allowExtendedTest()) goto finish;
            }
        }
    }
finish:
    testRunner.render();
}
