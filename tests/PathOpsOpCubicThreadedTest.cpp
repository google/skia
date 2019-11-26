/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkString.h"
#include "tests/PathOpsDebug.h"
#include "tests/PathOpsExtendedTest.h"
#include "tests/PathOpsThreadedCommon.h"

#include <atomic>

static int loopNo = 158;
static std::atomic<int> gCubicsTestNo{0};

static void testOpCubicsMain(PathOpsThreadState* data) {
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
                    for (int e = SkPath::kWinding_FillType ; e <= SkPath::kEvenOdd_FillType; ++e) {
    for (int f = SkPath::kWinding_FillType ; f <= SkPath::kEvenOdd_FillType; ++f) {
        SkPath pathA, pathB;
        pathA.setFillType((SkPath::FillType) e);
        pathA.moveTo(SkIntToScalar(state.fA), SkIntToScalar(state.fB));
        pathA.cubicTo(SkIntToScalar(state.fC), SkIntToScalar(state.fD), SkIntToScalar(b),
                SkIntToScalar(a), SkIntToScalar(d), SkIntToScalar(c));
        pathA.close();
        pathB.setFillType((SkPath::FillType) f);
        pathB.moveTo(SkIntToScalar(a), SkIntToScalar(b));
        pathB.cubicTo(SkIntToScalar(c), SkIntToScalar(d), SkIntToScalar(state.fB),
                SkIntToScalar(state.fA), SkIntToScalar(state.fD), SkIntToScalar(state.fC));
        pathB.close();
        for (int op = 0 ; op <= kXOR_SkPathOp; ++op)    {
            if (state.fReporter->verbose()) {
                pathStr.printf("static void cubicOp%d(skiatest::Reporter* reporter,"
                        " const char* filename) {\n", loopNo);
                pathStr.appendf("    SkPath path, pathB;\n");
                pathStr.appendf("    path.setFillType(SkPath::k%s_FillType);\n",
                        e == SkPath::kWinding_FillType ? "Winding" : e == SkPath::kEvenOdd_FillType
                        ? "EvenOdd" : "?UNDEFINED");
                pathStr.appendf("    path.moveTo(%d,%d);\n", state.fA, state.fB);
                pathStr.appendf("    path.cubicTo(%d,%d, %d,%d, %d,%d);\n", state.fC, state.fD,
                        b, a, d, c);
                pathStr.appendf("    path.close();\n");
                pathStr.appendf("    pathB.setFillType(SkPath::k%s_FillType);\n",
                        f == SkPath::kWinding_FillType ? "Winding" : f == SkPath::kEvenOdd_FillType
                        ? "EvenOdd" : "?UNDEFINED");
                pathStr.appendf("    pathB.moveTo(%d,%d);\n", a, b);
                pathStr.appendf("    pathB.cubicTo(%d,%d, %d,%d, %d,%d);\n", c, d,
                        state.fB, state.fA, state.fD, state.fC);
                pathStr.appendf("    pathB.close();\n");
                pathStr.appendf("    testPathOp(reporter, path, pathB, %s, filename);\n",
                        SkPathOpsDebug::OpStr((SkPathOp) op));
                pathStr.appendf("}\n");
                state.outputProgress(pathStr.c_str(), (SkPathOp) op);
            }
            SkString testName;
            testName.printf("thread_cubics%d", ++gCubicsTestNo);
            if (!testPathOp(state.fReporter, pathA, pathB, (SkPathOp) op, testName.c_str())) {
                if (state.fReporter->verbose()) {
                    ++loopNo;
                    goto skipToNext;
                }
            }
            if (PathOpsDebug::gCheckForDuplicateNames) return;
        }
    }
                    }
skipToNext: ;
                }
            }
        }
    }
}

DEF_TEST(PathOpsOpCubicsThreaded, reporter) {
    initializeTests(reporter, "cubicOp");
    PathOpsThreadedTestRunner testRunner(reporter);
    for (int a = 0; a < 6; ++a) {  // outermost
        for (int b = a + 1; b < 7; ++b) {
            for (int c = 0 ; c < 6; ++c) {
                for (int d = c + 1; d < 7; ++d) {
                    *testRunner.fRunnables.append() =
                            new PathOpsThreadedRunnable(&testOpCubicsMain, a, b, c, d, &testRunner);
                }
            }
            if (!reporter->allowExtendedTest()) goto finish;
        }
    }
finish:
    testRunner.render();
}
