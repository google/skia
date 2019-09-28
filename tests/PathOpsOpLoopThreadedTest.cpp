/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkString.h"
#include "tests/PathOpsDebug.h"
#include "tests/PathOpsExtendedTest.h"
#include "tests/PathOpsThreadedCommon.h"

#include <atomic>

static int loopNo = 17;

static void add_point(SkString* str, SkScalar x, SkScalar y) {
    int asInt = SkScalarRoundToInt(x);
    if (SkIntToScalar(asInt) == x) {
        str->appendf("%d", asInt);
    } else {
        str->appendf("%1.9gf", x);
    }
    str->appendf(",");
    asInt = SkScalarRoundToInt(y);
    if (SkIntToScalar(asInt) == y) {
        str->appendf("%d", asInt);
    } else {
        str->appendf("%1.9gf", y);
    }
}

static std::atomic<int> gLoopsTestNo{0};

static void testOpLoopsMain(PathOpsThreadState* data) {
#if DEBUG_SHOW_TEST_NAME
    strncpy(DEBUG_FILENAME_STRING, "", DEBUG_FILENAME_STRING_LENGTH);
#endif
    SkASSERT(data);
    PathOpsThreadState& state = *data;
    SkString pathStr;
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
        pathA.moveTo(SkIntToScalar(a), SkIntToScalar(b));
        pathA.cubicTo(SkIntToScalar(c), SkIntToScalar(d), endC.fX, endC.fY, endD.fX, endD.fY);
        pathA.close();
        pathB.moveTo(SkIntToScalar(c), SkIntToScalar(d));
        pathB.cubicTo(endC.fX, endC.fY, endD.fX, endD.fY, SkIntToScalar(a), SkIntToScalar(b));
        pathB.close();
//        SkDebugf("%s\n", pathStr);
        if (state.fReporter->verbose()) {
            pathStr.printf("static void loop%d(skiatest::Reporter* reporter,"
                    " const char* filename) {\n", loopNo);
            pathStr.appendf("    SkPath path, pathB;\n");
            pathStr.appendf("    path.moveTo(%d,%d);\n", a, b);
            pathStr.appendf("    path.cubicTo(%d,%d, ", c, d);
            add_point(&pathStr, endC.fX, endC.fY);
            pathStr.appendf(", ");
            add_point(&pathStr, endD.fX, endD.fY);
            pathStr.appendf(");\n");
            pathStr.appendf("    path.close();\n");
            pathStr.appendf("    pathB.moveTo(%d,%d);\n", c, d);
            pathStr.appendf("    pathB.cubicTo(");
            add_point(&pathStr, endC.fX, endC.fY);
            pathStr.appendf(", ");
            add_point(&pathStr, endD.fX, endD.fY);
            pathStr.appendf(", %d,%d);\n", a, b);
            pathStr.appendf("    pathB.close();\n");
            pathStr.appendf("    testPathOp(reporter, path, pathB, kIntersect_SkPathOp,"
                    " filename);\n");
            pathStr.appendf("}\n");
            state.outputProgress(pathStr.c_str(), kIntersect_SkPathOp);
        }
        SkString testName;
        testName.printf("thread_loops%d", ++gLoopsTestNo);
        testPathOp(state.fReporter, pathA, pathB, kIntersect_SkPathOp, testName.c_str());
        if (PathOpsDebug::gCheckForDuplicateNames) return;
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
                    *testRunner.fRunnables.append() =
                            new PathOpsThreadedRunnable(&testOpLoopsMain, a, b, c, d, &testRunner);
                }
            }
            if (!reporter->allowExtendedTest()) goto finish;
        }
    }
finish:
    testRunner.render();
}
