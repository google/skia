/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "PathOpsExtendedTest.h"

// four rects, of four sizes
// for 3 smaller sizes, tall, wide
    // top upper mid lower bottom aligned (3 bits, 5 values)
    // same with x (3 bits, 5 values)
// not included, square, tall, wide (2 bits)
// cw or ccw (1 bit)

static THREAD_TYPE testPathOpsRectsMain(void* data)
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
            pathA.addRect(SkIntToScalar(state.a), SkIntToScalar(state.a), SkIntToScalar(state.b),
                    SkIntToScalar(state.b), SkPath::kCW_Direction);
            str += sprintf(str, "    path.addRect(%d, %d, %d, %d,"
                    " SkPath::kCW_Direction);\n", state.a, state.a, state.b, state.b);
            pathA.addRect(SkIntToScalar(state.c), SkIntToScalar(state.c), SkIntToScalar(state.d),
                    SkIntToScalar(state.d), SkPath::kCW_Direction);
            str += sprintf(str, "    path.addRect(%d, %d, %d, %d,"
                    " SkPath::kCW_Direction);\n", state.c, state.c, state.d, state.d);
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

static void TestPathOpsRectsThreaded(skiatest::Reporter* reporter) {
    int testsRun = 0;
    if (gShowTestProgress) SkDebugf("%s\n", __FUNCTION__);
#ifdef SK_DEBUG
    gDebugMaxWindSum = 4;
    gDebugMaxWindValue = 4;
#endif
    const char testLineStr[] = "testOp";
    initializeTests(reporter, testLineStr, sizeof(testLineStr));
    for (int a = 0; a < 6; ++a) {  // outermost
        for (int b = a + 1; b < 7; ++b) {
            for (int c = 0 ; c < 6; ++c) {
                for (int d = c + 1; d < 7; ++d) {
                    testsRun += dispatchTest4(testPathOpsRectsMain, a, b, c, d);
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
    if (gShowTestProgress) SkDebugf("%s tests=%d total=%d\n", __FUNCTION__, testsRun);
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("PathOpsRectsThreaded", OpRectsThreadedTestClass, \
        TestPathOpsRectsThreaded)
