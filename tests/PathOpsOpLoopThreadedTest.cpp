/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "PathOpsExtendedTest.h"
#include "PathOpsThreadedCommon.h"

static void testOpLoopsMain(PathOpsThreadState* data) {
#if DEBUG_SHOW_TEST_NAME
    strncpy(DEBUG_FILENAME_STRING, "", DEBUG_FILENAME_STRING_LENGTH);
#endif
    SkASSERT(data);
    PathOpsThreadState& state = *data;
    char pathStr[1024];  // gdb: set print elements 400
    bool progress = state.fReporter->verbose(); // FIXME: break out into its own parameter?
    if (progress) {
        sk_bzero(pathStr, sizeof(pathStr));
    }
    for (int a = 0 ; a < 6; ++a) {
        for (int b = a + 1 ; b < 7; ++b) {
            for (int c = 0 ; c < 6; ++c) {
                for (int d = c + 1 ; d < 7; ++d) {
        // define 4 points that form two lines that often cross; one line is (a, b) (c, d)
        SkVector v = {SkIntToScalar(a - c), SkIntToScalar(b - d)};
        SkPoint midA = { SkIntToScalar(a * state.fA + c * (6 - state.fA)) / 6,
                         SkIntToScalar(b * state.fA + d * (6 - state.fA)) / 6 };
        SkPoint midB = { SkIntToScalar(a * state.fB + c * (6 - state.fB)) / 6,
                         SkIntToScalar(b * state.fB + d * (6 - state.fB)) / 6 };
        SkPoint endC = { midA.fX + v.fY * state.fC / 3,
                          midA.fY + v.fX * state.fC / 3 };
        SkPoint endD = { midB.fX - v.fY * state.fD / 3,
                          midB.fY + v.fX * state.fD / 3 };
        SkPath pathA, pathB;
        if (progress) {
            char* str = pathStr;
            str += sprintf(str, "    path.moveTo(%d,%d);\n", a, b);
            str += sprintf(str, "    path.cubicTo(%d,%d, %1.9gf,%1.9gf, %1.9gf,%1.9gf);\n",
                    c, d, endC.fX, endC.fY, endD.fX, endD.fY);
            str += sprintf(str, "    path.close();\n");
            str += sprintf(str, "    pathB.moveTo(%d,%d);\n", c, d);
            str += sprintf(str, "    pathB.cubicTo(%1.9gf,%1.9gf, %1.9gf,%1.9gf, %d,%d);\n",
                    endC.fX, endC.fY, endD.fX, endD.fY, a, b);
            str += sprintf(str, "    pathB.close();\n");
        }
        pathA.moveTo(SkIntToScalar(a), SkIntToScalar(b));
        pathA.cubicTo(SkIntToScalar(c), SkIntToScalar(d), endC.fX, endC.fY, endD.fX, endD.fY);
        pathA.close();
        pathB.moveTo(SkIntToScalar(c), SkIntToScalar(d));
        pathB.cubicTo(endC.fX, endC.fY, endD.fX, endD.fY, SkIntToScalar(a), SkIntToScalar(b));
        pathB.close();
//        SkDebugf("%s\n", pathStr);
        if (progress) {
            outputProgress(state.fPathStr, pathStr, kIntersect_PathOp);
        }
        testThreadedPathOp(state.fReporter, pathA, pathB, kIntersect_PathOp, "loops");
                }
            }
        }
    }
}

DEF_TEST(PathOpsOpLoopsThreaded, reporter) {
    if (!FLAGS_runFail) {
        return;
    }
    initializeTests(reporter, "cubicOp");
    PathOpsThreadedTestRunner testRunner(reporter);
    for (int a = 0; a < 6; ++a) {  // outermost
        for (int b = a + 1; b < 7; ++b) {
            for (int c = 0 ; c < 6; ++c) {
                for (int d = c + 1; d < 7; ++d) {
                    *testRunner.fRunnables.append() = SkNEW_ARGS(PathOpsThreadedRunnable,
                            (&testOpLoopsMain, a, b, c, d, &testRunner));
                }
            }
            if (!reporter->allowExtendedTest()) goto finish;
        }
    }
finish:
    testRunner.render();
    ShowTestArray();
}

DEF_TEST(PathOpsOpLoops, reporter) {
    if (!FLAGS_runFail) {
        return;
    }
    initializeTests(reporter, "cubicOp");
    PathOpsThreadState state;
    state.fReporter = reporter;
    SkBitmap bitmap;
    state.fBitmap = &bitmap;
    char pathStr[PATH_STR_SIZE];
    state.fPathStr = pathStr;
    for (state.fA = 0; state.fA < 6; ++state.fA) {  // outermost
        for (state.fB = state.fA + 1; state.fB < 7; ++state.fB) {
            for (state.fC = 0 ; state.fC < 6; ++state.fC) {
                for (state.fD = state.fC + 1; state.fD < 7; ++state.fD) {
                    testOpLoopsMain(&state);
                }
            }
            if (!reporter->allowExtendedTest()) goto finish;
        }
    }
finish:
    ShowTestArray();
}
