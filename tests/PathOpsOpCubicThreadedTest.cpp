/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "PathOpsExtendedTest.h"

static THREAD_TYPE testOpCubicsMain(void* data)
{
    SkASSERT(data);
    State4& state = *(State4*) data;
    char pathStr[1024];  // gdb: set print elements 400
    sk_bzero(pathStr, sizeof(pathStr));
    do {
        for (int a = 0 ; a < 6; ++a) {
        for (int b = a + 1 ; b < 7; ++b)  {
        for (int c = 0 ; c < 6; ++c)          {
        for (int d = c + 1 ; d < 7; ++d)           {
        for (int e = SkPath::kWinding_FillType ; e <= SkPath::kEvenOdd_FillType; ++e) {
        for (int f = SkPath::kWinding_FillType ; f <= SkPath::kEvenOdd_FillType; ++f)   {
            SkPath pathA, pathB;
            char* str = pathStr;
            pathA.setFillType((SkPath::FillType) e);
            str += sprintf(str, "    path.setFillType(SkPath::k%s_FillType);\n",
                    e == SkPath::kWinding_FillType ? "Winding" : e == SkPath::kEvenOdd_FillType
                    ? "EvenOdd" : "?UNDEFINED");
            pathA.moveTo(SkIntToScalar(state.a), SkIntToScalar(state.b));
            str += sprintf(str, "    path.moveTo(%d,%d);\n", state.a, state.b);
            pathA.cubicTo(SkIntToScalar(state.c), SkIntToScalar(state.d), SkIntToScalar(b),
                    SkIntToScalar(a), SkIntToScalar(d), SkIntToScalar(c));
            str += sprintf(str, "    path.cubicTo(%d,%d, %d,%d, %d,%d);\n", state.c, state.d,
                    b, a, d, c);
            pathA.close();
            str += sprintf(str, "    path.close();\n");
            pathB.setFillType((SkPath::FillType) f);
            str += sprintf(str, "    pathB.setFillType(SkPath::k%s_FillType);\n",
                    f == SkPath::kWinding_FillType ? "Winding" : f == SkPath::kEvenOdd_FillType
                    ? "EvenOdd" : "?UNDEFINED");
            pathB.moveTo(SkIntToScalar(a), SkIntToScalar(b));
            str += sprintf(str, "    pathB.moveTo(%d,%d);\n", a, b);
            pathB.cubicTo(SkIntToScalar(c), SkIntToScalar(d), SkIntToScalar(state.b),
                    SkIntToScalar(state.a), SkIntToScalar(state.d), SkIntToScalar(state.c));
            str += sprintf(str, "    pathB.cubicTo(%d,%d, %d,%d, %d,%d);\n", c, d,
                    state.b, state.a, state.d, state.c);
            pathB.close();
            str += sprintf(str, "    pathB.close();\n");
            for (int op = 0 ; op <= kXOR_PathOp; ++op)    {
                outputProgress(state, pathStr, (SkPathOp) op);
                testPathOp(state.reporter, pathA, pathB, (SkPathOp) op);
                state.testsRun++;
            }
                                }
                            }
                        }
                    }
                }
            }
    } while (runNextTestSet(state));
    THREAD_RETURN
}

static void TestOpCubicsThreaded(skiatest::Reporter* reporter)
{
    int testsRun = 0;
    if (gShowTestProgress) SkDebugf("%s\n", __FUNCTION__);
#ifdef SK_DEBUG
    gDebugMaxWindSum = 4;
    gDebugMaxWindValue = 4;
#endif
    const char testLineStr[] = "cubicOp";
    initializeTests(reporter, testLineStr, sizeof(testLineStr));
    for (int a = 0; a < 6; ++a) {  // outermost
        for (int b = a + 1; b < 7; ++b) {
            for (int c = 0 ; c < 6; ++c) {
                for (int d = c + 1; d < 7; ++d) {
                    testsRun += dispatchTest4(testOpCubicsMain, a, b, c, d);
                }
                if (gShowTestProgress) SkDebugf(".");
            }
            if (!gAllowExtendedTest) goto finish;
            if (gShowTestProgress) SkDebugf("%d", b);
        }
        if (gShowTestProgress) SkDebugf("\n%d", a);
    }
finish:
    testsRun += waitForCompletion();
    if (gShowTestProgress) SkDebugf("%s tests=%d\n", __FUNCTION__, testsRun);
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("PathOpsOpCubicsThreaded", OpCubicsThreadedTestClass, \
        TestOpCubicsThreaded)
