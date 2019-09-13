/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkString.h"
#include "tests/PathOpsExtendedTest.h"
#include "tests/PathOpsThreadedCommon.h"

static void testSimplifyTrianglesMain(PathOpsThreadState* data) {
    SkASSERT(data);
    PathOpsThreadState& state = *data;
    state.fKey = "?";
    int ax = state.fA & 0x03;
    int ay = state.fA >> 2;
    int bx = state.fB & 0x03;
    int by = state.fB >> 2;
    int cx = state.fC & 0x03;
    int cy = state.fC >> 2;
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
                SkString pathStr;
                SkPath path, out;
                path.setFillType(SkPath::kWinding_FillType);
                path.moveTo(SkIntToScalar(ax), SkIntToScalar(ay));
                path.lineTo(SkIntToScalar(bx), SkIntToScalar(by));
                path.lineTo(SkIntToScalar(cx), SkIntToScalar(cy));
                path.close();
                path.moveTo(SkIntToScalar(dx), SkIntToScalar(dy));
                path.lineTo(SkIntToScalar(ex), SkIntToScalar(ey));
                path.lineTo(SkIntToScalar(fx), SkIntToScalar(fy));
                path.close();
                if (state.fReporter->verbose()) {
                    pathStr.appendf("    path.moveTo(%d, %d);\n", ax, ay);
                    pathStr.appendf("    path.lineTo(%d, %d);\n", bx, by);
                    pathStr.appendf("    path.lineTo(%d, %d);\n", cx, cy);
                    pathStr.appendf("    path.close();\n");
                    pathStr.appendf("    path.moveTo(%d, %d);\n", dx, dy);
                    pathStr.appendf("    path.lineTo(%d, %d);\n", ex, ey);
                    pathStr.appendf("    path.lineTo(%d, %d);\n", fx, fy);
                    pathStr.appendf("    path.close();\n");
                    state.outputProgress(pathStr.c_str(), SkPathFillType::kWinding);
                }
                ShowTestName(&state, d, e, f, 0);
                testSimplify(path, false, out, state, pathStr.c_str());
                path.setFillType(SkPath::kEvenOdd_FillType);
                if (state.fReporter->verbose()) {
                    state.outputProgress(pathStr.c_str(), SkPathFillType::kEvenOdd);
                }
                ShowTestName(&state, d, e, f, 1);
                testSimplify(path, true, out, state, pathStr.c_str());
            }
        }
    }
}

DEF_TEST(PathOpsSimplifyTrianglesThreaded, reporter) {
    initializeTests(reporter, "testTriangles");
    PathOpsThreadedTestRunner testRunner(reporter);
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
                *testRunner.fRunnables.append() = new PathOpsThreadedRunnable(
                        &testSimplifyTrianglesMain, a, b, c, 0, &testRunner);
            }
            if (!reporter->allowExtendedTest()) goto finish;
        }
    }
finish:
    testRunner.render();
}
