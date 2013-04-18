/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "PathOpsExtendedTest.h"
#include "PathOpsThreadedCommon.h"

// four rects, of four sizes
// for 3 smaller sizes, tall, wide
    // top upper mid lower bottom aligned (3 bits, 5 values)
    // same with x (3 bits, 5 values)
// not included, square, tall, wide (2 bits)
// cw or ccw (1 bit)

static void testPathOpsRectsMain(PathOpsThreadState* data)
{
    SkASSERT(data);
    PathOpsThreadState& state = *data;
    char pathStr[1024];  // gdb: set print elements 400
    sk_bzero(pathStr, sizeof(pathStr));
    for (int a = 0 ; a < 6; ++a) {
        for (int b = a + 1 ; b < 7; ++b) {
            for (int c = 0 ; c < 6; ++c) {
                for (int d = c + 1 ; d < 7; ++d) {
                    for (int e = SkPath::kWinding_FillType ; e <= SkPath::kEvenOdd_FillType; ++e) {
    for (int f = SkPath::kWinding_FillType ; f <= SkPath::kEvenOdd_FillType; ++f)   {
        SkPath pathA, pathB;
        char* str = pathStr;
        pathA.setFillType((SkPath::FillType) e);
        str += sprintf(str, "    path.setFillType(SkPath::k%s_FillType);\n",
                e == SkPath::kWinding_FillType ? "Winding" : e == SkPath::kEvenOdd_FillType
                ? "EvenOdd" : "?UNDEFINED");
        pathA.addRect(SkIntToScalar(state.fA), SkIntToScalar(state.fA), SkIntToScalar(state.fB),
                SkIntToScalar(state.fB), SkPath::kCW_Direction);
        str += sprintf(str, "    path.addRect(%d, %d, %d, %d,"
                " SkPath::kCW_Direction);\n", state.fA, state.fA, state.fB, state.fB);
        pathA.addRect(SkIntToScalar(state.fC), SkIntToScalar(state.fC), SkIntToScalar(state.fD),
                SkIntToScalar(state.fD), SkPath::kCW_Direction);
        str += sprintf(str, "    path.addRect(%d, %d, %d, %d,"
                " SkPath::kCW_Direction);\n", state.fC, state.fC, state.fD, state.fD);
        pathA.close();
        pathB.setFillType((SkPath::FillType) f);
        str += sprintf(str, "    pathB.setFillType(SkPath::k%s_FillType);\n",
                f == SkPath::kWinding_FillType ? "Winding" : f == SkPath::kEvenOdd_FillType
                ? "EvenOdd" : "?UNDEFINED");
        pathB.addRect(SkIntToScalar(a), SkIntToScalar(a), SkIntToScalar(b),
                SkIntToScalar(b), SkPath::kCW_Direction);
        str += sprintf(str, "    pathB.addRect(%d, %d, %d, %d,"
                " SkPath::kCW_Direction);\n", a, a, b, b);
        pathB.addRect(SkIntToScalar(c), SkIntToScalar(c), SkIntToScalar(d),
                SkIntToScalar(d), SkPath::kCW_Direction);
        str += sprintf(str, "    pathB.addRect(%d, %d, %d, %d,"
                " SkPath::kCW_Direction);\n", c, c, d, d);
        pathB.close();
        for (int op = 0 ; op <= kXOR_PathOp; ++op)    {
            outputProgress(state.fPathStr, pathStr, (SkPathOp) op);
            testPathOp(state.fReporter, pathA, pathB, (SkPathOp) op);
        }
    }
                    }
                }
            }
        }
    }
}

static void PathOpsRectsThreadedTest(skiatest::Reporter* reporter) {
    int threadCount = initializeTests(reporter, "testOp");
    PathOpsThreadedTestRunner testRunner(reporter, threadCount);
    for (int a = 0; a < 6; ++a) {  // outermost
        for (int b = a + 1; b < 7; ++b) {
            for (int c = 0 ; c < 6; ++c) {
                for (int d = c + 1; d < 7; ++d) {
                    *testRunner.fRunnables.append() = SkNEW_ARGS(PathOpsThreadedRunnable,
                            (&testPathOpsRectsMain, a, b, c, d, &testRunner));
                }
            }
            if (!reporter->allowExtendedTest()) goto finish;
       }
    }
finish:
    testRunner.render();
}

#include "TestClassDef.h"
DEFINE_TESTCLASS_SHORT(PathOpsRectsThreadedTest)
