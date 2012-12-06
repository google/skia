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

static void* testShapeOps4x4RectsMain(void* data)
{
    SkASSERT(data);
    State4& state = *(State4*) data;
    char pathStr[1024]; // gdb: set print elements 400
    bzero(pathStr, sizeof(pathStr));
    do {
        for (int a = 0 ; a < 6; ++a) {
        for (int b = a + 1 ; a < 7; ++b)  {
        for (int c = 0 ; c < 6; ++c)          {
        for (int d = c + 1 ; d < 7; ++d)           {
        for (int op = 0 ; op < kShapeOp_Count; ++op)    {
        for (int e = SkPath::kWinding_FillType ; e <= SkPath::kEvenOdd_FillType; ++e) {
        for (int f = SkPath::kWinding_FillType ; f <= SkPath::kEvenOdd_FillType; ++f)   {
            SkPath pathA, pathB;
            char* str = pathStr;
            pathA.setFillType((SkPath::FillType) e);
            str += sprintf(str, "    path.setFillType(SkPath::k%s_FillType);\n",
                    e == SkPath::kWinding_FillType ? "Winding" : e == SkPath::kEvenOdd_FillType
                    ? "EvenOdd" : "?UNDEFINED");
            pathA.addRect(state.a, state.a, state.b, state.b, SkPath::kCW_Direction);
            str += sprintf(str, "    path.addRect(%d, %d, %d, %d,"
                    " SkPath::kCW_Direction);\n", state.a, state.a, state.b, state.b);
            pathA.addRect(state.c, state.c, state.d, state.d, SkPath::kCW_Direction);
            str += sprintf(str, "    path.addRect(%d, %d, %d, %d,"
                    " SkPath::kCW_Direction);\n", state.c, state.c, state.d, state.d);
            pathA.close();
            pathB.setFillType((SkPath::FillType) f);
            str += sprintf(str, "    pathB.setFillType(SkPath::k%s_FillType);\n",
                    f == SkPath::kWinding_FillType ? "Winding" : f == SkPath::kEvenOdd_FillType
                    ? "EvenOdd" : "?UNDEFINED");
            pathB.addRect(a, a, b, b, SkPath::kCW_Direction);
            str += sprintf(str, "    pathB.addRect(%d, %d, %d, %d,"
                    " SkPath::kCW_Direction);\n", a, a, b, b);
            pathB.addRect(c, c, d, d, SkPath::kCW_Direction);
            str += sprintf(str, "    pathB.addRect(%d, %d, %d, %d,"
                    " SkPath::kCW_Direction);\n", c, c, d, d);
            pathB.close();
            outputProgress(state, pathStr, kDifference_Op);
            testShapeOp(pathA, pathB, kDifference_Op);
            state.testsRun++;
            outputProgress(state, pathStr, kIntersect_Op);
            testShapeOp(pathA, pathB, kIntersect_Op);
            state.testsRun++;
            outputProgress(state, pathStr, kUnion_Op);
            testShapeOp(pathA, pathB, kUnion_Op);
            state.testsRun++;
            outputProgress(state, pathStr, kXor_Op);
            testShapeOp(pathA, pathB, kXor_Op);
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

void ShapeOps4x4RectsThreaded_Test(int& testsRun)
{
    SkDebugf("%s\n", __FUNCTION__);
#ifdef SK_DEBUG
    gDebugMaxWindSum = 4;
    gDebugMaxWindValue = 4;
#endif
    const char testLineStr[] = "testOp";
    initializeTests(testLineStr, sizeof(testLineStr));
    int testsStart = testsRun;
    for (int a = 0; a < 6; ++a) { // outermost
        for (int b = a + 1; b < 7; ++b) {
            for (int c = 0 ; c < 6; ++c) {
                for (int d = c + 1; d < 7; ++d) {
                    testsRun += dispatchTest4(testShapeOps4x4RectsMain, a, b, c, d);
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
