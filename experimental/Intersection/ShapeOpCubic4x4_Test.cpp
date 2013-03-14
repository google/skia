/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "EdgeWalker_Test.h"
#include "Intersection_Tests.h"
#include "ShapeOps.h"

// four rects, of four sizes
// for 3 smaller sizes, tall, wide
    // top upper mid lower bottom aligned (3 bits, 5 values)
    // same with x (3 bits, 5 values)
// not included, square, tall, wide (2 bits)
// cw or ccw (1 bit)

int failSet[][8] = {
    { 0, 1, 0, 6,   2, 3, 1, 4 }
};

static void* testShapeOps4x4CubicsMain(void* data)
{
    SkASSERT(data);
    State4& state = *(State4*) data;
    char pathStr[1024]; // gdb: set print elements 400
    bzero(pathStr, sizeof(pathStr));
    do {
        for (int a = 0 ; a < 6; ++a) {
        for (int b = a + 1 ; b < 7; ++b)  {
        for (int c = 0 ; c < 6; ++c)          {
        for (int d = c + 1 ; d < 7; ++d)           {
        for (int e = SkPath::kWinding_FillType ; e <= SkPath::kEvenOdd_FillType; ++e) {
        for (int f = SkPath::kWinding_FillType ; f <= SkPath::kEvenOdd_FillType; ++f)   {

#if 0
  if (state.a == fail[0] && state.b == fail[1] && state.c == fail[2] && state.d == fail[3]
        && a == fail[4] && b == fail[5] && c == fail[6] && d == fail[7]) {
            SkDebugf("skip failing case\n");
    }
    // skip this troublesome cubic pair
#endif
            SkPath pathA, pathB;
            char* str = pathStr;
            pathA.setFillType((SkPath::FillType) e);
            str += sprintf(str, "    path.setFillType(SkPath::k%s_FillType);\n",
                    e == SkPath::kWinding_FillType ? "Winding" : e == SkPath::kEvenOdd_FillType
                    ? "EvenOdd" : "?UNDEFINED");
            pathA.moveTo(state.a, state.b);
            str += sprintf(str, "    path.moveTo(%d,%d);\n", state.a, state.b);
            pathA.cubicTo(state.c, state.d, b, a, d, c);
            str += sprintf(str, "    path.cubicTo(%d,%d, %d,%d, %d,%d);\n", state.c, state.d,
                    b, a, d, c);
            pathA.close();
            str += sprintf(str, "    path.close();\n");
            pathB.setFillType((SkPath::FillType) f);
            str += sprintf(str, "    pathB.setFillType(SkPath::k%s_FillType);\n",
                    f == SkPath::kWinding_FillType ? "Winding" : f == SkPath::kEvenOdd_FillType
                    ? "EvenOdd" : "?UNDEFINED");
            pathB.moveTo(a, b);
            str += sprintf(str, "    pathB.moveTo(%d,%d);\n", a, b);
            pathB.cubicTo(c, d, state.b, state.a, state.d, state.c);
            str += sprintf(str, "    pathB.cubicTo(%d,%d, %d,%d, %d,%d);\n", c, d,
                    state.b, state.a, state.d, state.c);
            pathB.close();
            str += sprintf(str, "    pathB.close();\n");
            for (int op = 0 ; op < kShapeOp_Count; ++op)    {
                outputProgress(state, pathStr, (ShapeOp) op);
                testShapeOp(pathA, pathB, (ShapeOp) op);
                state.testsRun++;
            }
                                }
                            }
                        }
                    }
                }
            }
    } while (runNextTestSet(state));
    return NULL;
}

void ShapeOps4x4CubicsThreaded_Test(int& testsRun)
{
    SkDebugf("%s\n", __FUNCTION__);
#ifdef SK_DEBUG
    gDebugMaxWindSum = 4;
    gDebugMaxWindValue = 4;
#endif
    const char testLineStr[] = "cubicOp";
    initializeTests(testLineStr, sizeof(testLineStr));
    int testsStart = testsRun;
    for (int a = 0; a < 6; ++a) { // outermost
        for (int b = a + 1; b < 7; ++b) {
            for (int c = 0 ; c < 6; ++c) {
                for (int d = c + 1; d < 7; ++d) {
                    testsRun += dispatchTest4(testShapeOps4x4CubicsMain, a, b, c, d);
                }
                if (!gRunTestsInOneThread) SkDebugf(".");
            }
            if (!gRunTestsInOneThread) SkDebugf("%d", b);
        }
        if (!gRunTestsInOneThread) SkDebugf("\n%d", a);
    }
    testsRun += waitForCompletion();
    SkDebugf("%s tests=%d total=%d\n", __FUNCTION__, testsRun - testsStart, testsRun);
}
