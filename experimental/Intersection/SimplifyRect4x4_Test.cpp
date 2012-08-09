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

static void* testSimplify4x4RectsMain(void* data)
{
    SkASSERT(data);
    State4& state = *(State4*) data;
    char pathStr[1024]; // gdb: set print elements 400
    bzero(pathStr, sizeof(pathStr));
    do {
        int aShape = state.a & 0x03;
        int aCW = state.a >> 2;
        int bShape = state.b & 0x03;
        int bCW = state.b >> 2;
        int cShape = state.c & 0x03;
        int cCW = state.c >> 2;
        int dShape = state.d & 0x03;
        int dCW = state.d >> 2;
        for (int aXAlign = 0 ; aXAlign < 5; ++aXAlign) {
        for (int aYAlign = 0 ; aYAlign < 5; ++aYAlign)      {
        for (int bXAlign = 0 ; bXAlign < 5; ++bXAlign)          {
        for (int bYAlign = 0 ; bYAlign < 5; ++bYAlign)              {
        for (int cXAlign = 0 ; cXAlign < 5; ++cXAlign)                  {
        for (int cYAlign = 0 ; cYAlign < 5; ++cYAlign)                      {
        for (int dXAlign = 0 ; dXAlign < 5; ++dXAlign)                          {
        for (int dYAlign = 0 ; dYAlign < 5; ++dYAlign)                              {
            SkPath path, out;
            char* str = pathStr;
            path.setFillType(SkPath::kWinding_FillType);
            int l, t, r, b;
            if (aShape) {
                switch (aShape) {
                    case 1: // square
                        l =  0; r = 60;
                        t =  0; b = 60;
                        aXAlign = 5;
                        aYAlign = 5;
                        break;
                    case 2:
                        l =  aXAlign * 12;
                        r =  l + 30; 
                        t =  0; b = 60;
                        aYAlign = 5;
                        break;
                    case 3:
                        l =  0; r = 60;
                        t =  aYAlign * 12;
                        b =  l + 30; 
                        aXAlign = 5;
                        break;
                }
                path.addRect(l, t, r, b, (SkPath::Direction) aCW);
                str += sprintf(str, "    path.addRect(%d, %d, %d, %d,"
                        " (SkPath::Direction) %d);\n", l, t, r, b, aCW);
            } else {
                aXAlign = 5;
                aYAlign = 5;
            }
            if (bShape) {
                switch (bShape) {
                    case 1: // square
                        l =  bXAlign * 10;
                        r =  l + 20; 
                        t =  bYAlign * 10;
                        b =  l + 20; 
                        break;
                    case 2:
                        l =  bXAlign * 10;
                        r =  l + 20; 
                        t =  10; b = 40;
                        bYAlign = 5;
                        break;
                    case 3:
                        l =  10; r = 40;
                        t =  bYAlign * 10;
                        b =  l + 20; 
                        bXAlign = 5;
                        break;
                }
                path.addRect(l, t, r, b, (SkPath::Direction) bCW);
                str += sprintf(str, "    path.addRect(%d, %d, %d, %d,"
                        " (SkPath::Direction) %d);\n", l, t, r, b, bCW);
            } else {
                bXAlign = 5;
                bYAlign = 5;
            }
            if (cShape) {
                switch (cShape) {
                    case 1: // square
                        l =  cXAlign * 6;
                        r =  l + 12; 
                        t =  cYAlign * 6;
                        b =  l + 12; 
                        break;
                    case 2:
                        l =  cXAlign * 6;
                        r =  l + 12; 
                        t =  20; b = 30;
                        cYAlign = 5;
                        break;
                    case 3:
                        l =  20; r = 30;
                        t =  cYAlign * 6;
                        b =  l + 20; 
                        cXAlign = 5;
                        break;
                }
                path.addRect(l, t, r, b, (SkPath::Direction) cCW);
                str += sprintf(str, "    path.addRect(%d, %d, %d, %d,"
                        " (SkPath::Direction) %d);\n", l, t, r, b, cCW);
            } else {
                cXAlign = 5;
                cYAlign = 5;
            }
            if (dShape) {
                switch (dShape) {
                    case 1: // square
                        l =  dXAlign * 4;
                        r =  l + 9; 
                        t =  dYAlign * 4;
                        b =  l + 9; 
                        break;
                    case 2:
                        l =  dXAlign * 6;
                        r =  l + 9; 
                        t =  32; b = 36;
                        dYAlign = 5;
                        break;
                    case 3:
                        l =  32; r = 36;
                        t =  dYAlign * 6;
                        b =  l + 9; 
                        dXAlign = 5;
                        break;
                }
                path.addRect(l, t, r, b, (SkPath::Direction) dCW);
                str += sprintf(str, "    path.addRect(%d, %d, %d, %d,"
                        " (SkPath::Direction) %d);\n", l, t, r, b, dCW);
            } else {
                dXAlign = 5;
                dYAlign = 5;
            }
            path.close();
            outputProgress(state, pathStr);
            testSimplifyx(path, out, state, pathStr);
            state.testsRun++;
                                    }
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

void Simplify4x4RectsThreaded_Test()
{
    SkDebugf("%s\n", __FUNCTION__);
#ifdef SK_DEBUG
    gDebugMaxWindSum = 4;
    gDebugMaxWindValue = 4;
#endif
    const char testLineStr[] = "testLine";
    initializeTests(testLineStr, sizeof(testLineStr));
    int testsRun = 0;
    for (int a = 0; a < 8; ++a) { // outermost
        for (int b = a ; b < 8; ++b) {
            for (int c = b ; c < 8; ++c) {
                for (int d = c; d < 8; ++d) {                 
                    testsRun += dispatchTest4(testSimplify4x4RectsMain, a, b, c, d);
                }
                if (!gRunTestsInOneThread) SkDebugf(".");
            }
            if (!gRunTestsInOneThread) SkDebugf("%d", b);
        }
        if (!gRunTestsInOneThread) SkDebugf("\n%d", a);
    }
    testsRun += waitForCompletion();
    SkDebugf("%s total tests run=%d\n", __FUNCTION__, testsRun);
}

