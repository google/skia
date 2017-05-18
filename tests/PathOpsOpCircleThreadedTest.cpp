/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "PathOpsExtendedTest.h"
#include "PathOpsThreadedCommon.h"
#include "SkString.h"

static int loopNo = 4;

static void testOpCirclesMain(PathOpsThreadState* data) {
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
        pathA.addCircle(SkIntToScalar(state.fA), SkIntToScalar(state.fB), SkIntToScalar(state.fC),
                state.fD ? SkPath::kCW_Direction : SkPath::kCCW_Direction);
        pathB.setFillType((SkPath::FillType) f);
        pathB.addCircle(SkIntToScalar(a), SkIntToScalar(b), SkIntToScalar(c),
                d ? SkPath::kCW_Direction : SkPath::kCCW_Direction);
        for (int op = 0 ; op <= kXOR_SkPathOp; ++op)    {
            if (state.fReporter->verbose()) {
                pathStr.printf("static void circlesOp%d(skiatest::Reporter* reporter,"
                        " const char* filename) {\n", loopNo);
                pathStr.appendf("    SkPath path, pathB;\n");
                pathStr.appendf("    path.setFillType(SkPath::k%s_FillType);\n",
                        e == SkPath::kWinding_FillType ? "Winding" : e == SkPath::kEvenOdd_FillType
                        ? "EvenOdd" : "?UNDEFINED");
                pathStr.appendf("    path.addCircle(%d, %d, %d, %s);\n", state.fA, state.fB,
                        state.fC, state.fD ? "SkPath::kCW_Direction" : "SkPath::kCCW_Direction");
                pathStr.appendf("    pathB.setFillType(SkPath::k%s_FillType);\n",
                        f == SkPath::kWinding_FillType ? "Winding" : f == SkPath::kEvenOdd_FillType
                        ? "EvenOdd" : "?UNDEFINED");
                pathStr.appendf("    pathB.addCircle(%d, %d, %d, %s);\n", a, b,
                        c, d ? "SkPath::kCW_Direction" : "SkPath::kCCW_Direction");
                pathStr.appendf("    testPathOp(reporter, path, pathB, %s, filename);\n",
                        SkPathOpsDebug::OpStr((SkPathOp) op));
                pathStr.appendf("}\n");
                state.outputProgress(pathStr.c_str(), (SkPathOp) op);
            }
            if (!testPathOp(state.fReporter, pathA, pathB, (SkPathOp) op, "circles")) {
                if (state.fReporter->verbose()) {
                    ++loopNo;
                    goto skipToNext;
                }
            }
        }
    }
                    }
skipToNext: ;
                }
            }
        }
    }
}

DEF_TEST(PathOpsOpCircleThreaded, reporter) {
    initializeTests(reporter, "circleOp");
    PathOpsThreadedTestRunner testRunner(reporter);
    for (int a = 0; a < 6; ++a) {  // outermost
        for (int b = a + 1; b < 7; ++b) {
            for (int c = 0 ; c < 6; ++c) {
                for (int d = 0; d < 2; ++d) {
                    *testRunner.fRunnables.append() = new PathOpsThreadedRunnable(
                            &testOpCirclesMain, a, b, c, d, &testRunner);
                }
            }
            if (!reporter->allowExtendedTest()) goto finish;
        }
    }
finish:
    testRunner.render();
}
