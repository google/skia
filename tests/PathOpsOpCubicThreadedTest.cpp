/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "PathOpsExtendedTest.h"
#include "PathOpsThreadedCommon.h"

static int loopNo = 158;

static void testOpCubicsMain(PathOpsThreadState* data) {
#if DEBUG_SHOW_TEST_NAME
    strncpy(DEBUG_FILENAME_STRING, "", DEBUG_FILENAME_STRING_LENGTH);
#endif
    SkASSERT(data);
    PathOpsThreadState& state = *data;
    char pathStr[1024];
    bool progress = state.fReporter->verbose(); // FIXME: break out into its own parameter?
    if (progress) {
        sk_bzero(pathStr, sizeof(pathStr));
    }
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
            if (progress) {
                outputProgress(state.fPathStr, pathStr, (SkPathOp) op);
            }
            if (progress) {
                char* str = pathStr;
                str += sprintf(str, "static void cubicOp%d(skiatest::Reporter* reporter,"
                        " const char* filename) {\n", loopNo);
                str += sprintf(str, "    SkPath path, pathB;\n");
                str += sprintf(str, "    path.setFillType(SkPath::k%s_FillType);\n",
                        e == SkPath::kWinding_FillType ? "Winding" : e == SkPath::kEvenOdd_FillType
                        ? "EvenOdd" : "?UNDEFINED");
                str += sprintf(str, "    path.moveTo(%d,%d);\n", state.fA, state.fB);
                str += sprintf(str, "    path.cubicTo(%d,%d, %d,%d, %d,%d);\n", state.fC, state.fD,
                        b, a, d, c);
                str += sprintf(str, "    path.close();\n");
                str += sprintf(str, "    pathB.setFillType(SkPath::k%s_FillType);\n",
                        f == SkPath::kWinding_FillType ? "Winding" : f == SkPath::kEvenOdd_FillType
                        ? "EvenOdd" : "?UNDEFINED");
                str += sprintf(str, "    pathB.moveTo(%d,%d);\n", a, b);
                str += sprintf(str, "    pathB.cubicTo(%d,%d, %d,%d, %d,%d);\n", c, d,
                        state.fB, state.fA, state.fD, state.fC);
                str += sprintf(str, "    pathB.close();\n");
                str += sprintf(str, "    testPathOp(reporter, path, pathB, %s, filename);\n",
                        SkPathOpsDebug::OpStr((SkPathOp) op));
                str += sprintf(str, "}\n");
            }
            if (!testPathOp(state.fReporter, pathA, pathB, (SkPathOp) op, "cubics")) {
                if (progress) {
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

DEF_TEST(PathOpsOpCubicsThreaded, reporter) {
    initializeTests(reporter, "cubicOp");
    PathOpsThreadedTestRunner testRunner(reporter);
    for (int a = 0; a < 6; ++a) {  // outermost
        for (int b = a + 1; b < 7; ++b) {
            for (int c = 0 ; c < 6; ++c) {
                for (int d = c + 1; d < 7; ++d) {
                    *testRunner.fRunnables.append() = SkNEW_ARGS(PathOpsThreadedRunnable,
                            (&testOpCubicsMain, a, b, c, d, &testRunner));
                }
            }
            if (!reporter->allowExtendedTest()) goto finish;
        }
    }
finish:
    testRunner.render();
    ShowTestArray("cubicOp");
}
