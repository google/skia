/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "EdgeWalker_Test.h"
#include "Intersection_Tests.h"
#include "ShapeOps.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkStream.h"
#include <assert.h>
#include <pthread.h>

// four rects, of four sizes
// for 3 smaller sizes, tall, wide
    // top upper mid lower bottom aligned (3 bits, 5 values)
    // same with x (3 bits, 5 values)
// not included, square, tall, wide (2 bits)
// cw or ccw (1 bit)
static const char marker[] =
    "</div>\n"
    "\n"
    "<script type=\"text/javascript\">\n"
    "\n"
    "var testDivs = [\n";
static const char testLineStr[] = "    testLine";
static const char filename[] = "../../experimental/Intersection/debugXX.txt";
static int testNumber;

static void* testSimplify4x4RectsMain(void* data)
{
    char pathStr[1024]; // gdb: set print elements 400
    bzero(pathStr, sizeof(pathStr));
    SkASSERT(data);
    State4& state = *(State4*) data;
    int aShape = state.a & 0x03;
    int aCW = state.a >> 2;
    int bShape = state.b & 0x03;
    int bCW = state.b >> 2;
    int cShape = state.c & 0x03;
    int cCW = state.c >> 2;
    int dShape = state.d & 0x03;
    int dCW = state.d >> 2;
    for (int aXAlign = 0 ; aXAlign < 5; ++aXAlign) {
    for (int aYAlign = 0 ; aYAlign < 5; ++aYAlign) {
    for (int bXAlign = 0 ; bXAlign < 5; ++bXAlign) {
    for (int bYAlign = 0 ; bYAlign < 5; ++bYAlign) {
    for (int cXAlign = 0 ; cXAlign < 5; ++cXAlign) {
    for (int cYAlign = 0 ; cYAlign < 5; ++cYAlign) {
    for (int dXAlign = 0 ; dXAlign < 5; ++dXAlign) {
    for (int dYAlign = 0 ; dYAlign < 5; ++dYAlign) {
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
        if (gRunTestsInOneThread) {
            SkDebugf("%s\n", pathStr);
        } else {
            SkFILEWStream outFile(state.filename);
            if (!outFile.isValid()) {
                continue;
            }
            outFile.writeText("<div id=\"testLine");
            outFile.writeDecAsText(testNumber);
            outFile.writeText("\">\n");
            outFile.writeText(pathStr);
            outFile.writeText("</div>\n\n");
            
            outFile.writeText(marker);
            outFile.writeText(testLineStr);
            outFile.writeDecAsText(testNumber);
            outFile.writeText(",\n\n\n");
            
            outFile.writeText("static void testLine");
            outFile.writeDecAsText(testNumber);
            outFile.writeText("() {\n    SkPath path, simple;\n");
            outFile.writeText(pathStr);
            outFile.writeText("    testSimplifyx(path);\n}\n");
            outFile.writeText("static void (*firstTest)() = testLine");
            outFile.writeDecAsText(testNumber);
            outFile.writeText(";\n\n");

            outFile.writeText("static struct {\n");
            outFile.writeText("    void (*fun)();\n");
            outFile.writeText("    const char* str;\n");
            outFile.writeText("} tests[] = {\n");
            outFile.writeText("    TEST(testLine");
            outFile.writeDecAsText(testNumber);
            outFile.writeText("),\n");
            outFile.flush();
        }
        testSimplifyx(path, out, state.bitmap, state.canvas);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return NULL;
}

const int maxThreads = gRunTestsInOneThread ? 1 : 8;

void Simplify4x4RectsThreaded_Test()
{
#ifdef SK_DEBUG
    gDebugMaxWindSum = 3;
    gDebugMaxWindValue = 3;
#endif
    if (maxThreads > 1) {
        SkFILEStream inFile("../../experimental/Intersection/op.htm");
        if (inFile.isValid()) {
            SkTDArray<char> inData;
            inData.setCount(inFile.getLength());
            size_t inLen = inData.count();
            inFile.read(inData.begin(), inLen);
            inFile.setPath(NULL);
            char* insert = strstr(inData.begin(), marker);   
            if (insert) {
                insert += sizeof(marker) - 1;
                const char* numLoc = insert + sizeof(testLineStr) - 1;
                testNumber = atoi(numLoc) + 1;
            }
        }
    }
    State4 threadState[maxThreads];
    int threadIndex;
    for (threadIndex = 0; threadIndex < maxThreads; ++threadIndex) {
        State4* statePtr = &threadState[threadIndex];
        strcpy(statePtr->filename, filename);
        SkASSERT(statePtr->filename[sizeof(filename) - 7] == 'X');
        SkASSERT(statePtr->filename[sizeof(filename) - 6] == 'X');
        statePtr->filename[sizeof(filename) - 7] = '0' + threadIndex / 10;
        statePtr->filename[sizeof(filename) - 6] = '0' + threadIndex % 10;
    }
    threadIndex = 0;
    for (int a = 0; a < 8; ++a) { // outermost
        for (int b = a ; b < 8; ++b) {
            for (int c = b ; c < 8; ++c) {
                for (int d = c; d < 8; ++d) {                 
                    State4* statePtr = &threadState[threadIndex];
                    statePtr->a = a;
                    statePtr->b = b;
                    statePtr->c = c;
                    statePtr->d = d;
                    if (maxThreads > 1) {
                        createThread(statePtr, testSimplify4x4RectsMain);
                        if (++threadIndex >= maxThreads) {
                            waitForCompletion(threadState, threadIndex);
                        }
                    } else {
                        testSimplify4x4RectsMain(statePtr);
                    }
                    if (maxThreads > 1) SkDebugf(".");
                }
                if (maxThreads > 1) SkDebugf("%d", c);
            }
            if (maxThreads > 1) SkDebugf("\n%d", b);
        }
        if (maxThreads > 1) SkDebugf("\n\n%d", a);
    }
    waitForCompletion(threadState, threadIndex);
#ifdef SK_DEBUG
    gDebugMaxWindSum = SK_MaxS32;
    gDebugMaxWindValue = SK_MaxS32;
#endif
}

