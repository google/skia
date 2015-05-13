/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "PathOpsExtendedTest.h"
#include "PathOpsThreadedCommon.h"

static int add_point(char* str, SkScalar x, SkScalar y) {
    int result;
    int asInt = SkScalarRoundToInt(x);
    if (SkIntToScalar(asInt) == x) {
        result = sprintf(str, "%d", asInt);
    } else {
        result = sprintf(str, "%1.9gf", x);
    }
    result += sprintf(str + result, ",");
    asInt = SkScalarRoundToInt(y);
    if (SkIntToScalar(asInt) == y) {
        result += sprintf(str + result, "%d", asInt);
    } else {
        result += sprintf(str + result, "%1.9gf", y);
    }
    return result;
}

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
            const int loopNo = 17;
            str += sprintf(str, "static void loop%d(skiatest::Reporter* reporter,"
                    " const char* filename) {\n", loopNo);
            str += sprintf(str, "    SkPath path, pathB;\n");
            str += sprintf(str, "    path.moveTo(%d,%d);\n", a, b);
            str += sprintf(str, "    path.cubicTo(%d,%d, ", c, d);
            str += add_point(str, endC.fX, endC.fY);
            str += sprintf(str, ", ");
            str += add_point(str, endD.fX, endD.fY);
            str += sprintf(str, ");\n");
            str += sprintf(str, "    path.close();\n");
            str += sprintf(str, "    pathB.moveTo(%d,%d);\n", c, d);
            str += sprintf(str, "    pathB.cubicTo(");
            str += add_point(str, endC.fX, endC.fY);
            str += sprintf(str, ", ");
            str += add_point(str, endD.fX, endD.fY);
            str += sprintf(str, ", %d,%d);\n", a, b);
            str += sprintf(str, "    pathB.close();\n");
            str += sprintf(str, "    testPathOp(reporter, path, pathB, kIntersect_SkPathOp,"
                    " filename);\n");
            str += sprintf(str, "}\n");
        }
        pathA.moveTo(SkIntToScalar(a), SkIntToScalar(b));
        pathA.cubicTo(SkIntToScalar(c), SkIntToScalar(d), endC.fX, endC.fY, endD.fX, endD.fY);
        pathA.close();
        pathB.moveTo(SkIntToScalar(c), SkIntToScalar(d));
        pathB.cubicTo(endC.fX, endC.fY, endD.fX, endD.fY, SkIntToScalar(a), SkIntToScalar(b));
        pathB.close();
//        SkDebugf("%s\n", pathStr);
        if (progress) {
            outputProgress(state.fPathStr, pathStr, kIntersect_SkPathOp);
        }
        testPathOp(state.fReporter, pathA, pathB, kIntersect_SkPathOp, "loops");
                }
            }
        }
    }
}

DEF_TEST(PathOpsOpLoopsThreaded, reporter) {
    initializeTests(reporter, "loopOp");
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
    ShowTestArray("loopOp");
}
