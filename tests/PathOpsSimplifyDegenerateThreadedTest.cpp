/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "PathOpsExtendedTest.h"
#include "PathOpsThreadedCommon.h"

static void testSimplifyDegeneratesMain(PathOpsThreadState* data) {
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
    for (int d = 0; d < 16; ++d) {
        int dx = d & 0x03;
        int dy = d >> 2;
        for (int e = d ; e < 16; ++e) {
            int ex = e & 0x03;
            int ey = e >> 2;
            for (int f = d ; f < 16; ++f) {
                int fx = f & 0x03;
                int fy = f >> 2;
                if (state.fD && (ex - dx) * (fy - dy)
                        != (ey - dy) * (fx - dx)) {
                    continue;
                }
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
                if (progress) {
                    char* str = pathStr;
                    str += sprintf(str, "    path.moveTo(%d, %d);\n", ax, ay);
                    str += sprintf(str, "    path.lineTo(%d, %d);\n", bx, by);
                    str += sprintf(str, "    path.lineTo(%d, %d);\n", cx, cy);
                    str += sprintf(str, "    path.close();\n");
                    str += sprintf(str, "    path.moveTo(%d, %d);\n", dx, dy);
                    str += sprintf(str, "    path.lineTo(%d, %d);\n", ex, ey);
                    str += sprintf(str, "    path.lineTo(%d, %d);\n", fx, fy);
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

DEF_TEST(PathOpsSimplifyDegeneratesThreaded, reporter) {
    initializeTests(reporter, "testDegenerates");
    PathOpsThreadedTestRunner testRunner(reporter);
    for (int a = 0; a < 16; ++a) {
        int ax = a & 0x03;
        int ay = a >> 2;
        for (int b = a ; b < 16; ++b) {
            int bx = b & 0x03;
            int by = b >> 2;
            for (int c = a ; c < 16; ++c) {
                int cx = c & 0x03;
                int cy = c >> 2;
                bool abcIsATriangle = (bx - ax) * (cy - ay) != (by - ay) * (cx - ax);
                *testRunner.fRunnables.append() = SkNEW_ARGS(PathOpsThreadedRunnable,
                        (&testSimplifyDegeneratesMain, a, b, c, abcIsATriangle,
                        &testRunner));
            }
            if (!reporter->allowExtendedTest()) goto finish;
        }
    }
finish:
    testRunner.render();
}
