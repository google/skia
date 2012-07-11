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

static void* testSimplify4x4RectsMain(void* data)
{
    char pathStr[1024]; // gdb: set print elements 400
    bzero(pathStr, sizeof(pathStr));
    SkASSERT(data);
    State4& state = *(State4*) data;
    int aShape = state.a & 0x03;
    int aCW = state.a >> 1;
    int bShape = state.b & 0x03;
    int bCW = state.b >> 1;
    int cShape = state.c & 0x03;
    int cCW = state.c >> 1;
    int dShape = state.d & 0x03;
    int dCW = state.d >> 1;
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
        SkDebugf("%s", pathStr);
        if (!testSimplifyx(path, out, state.bitmap, state.canvas)) {
            SkDebugf("*/\n{ %s %d, %d, %d, %d, %d, %d, %d, %d,"
                    " %d, %d, %d, %d },\n/*\n", 
                    __FUNCTION__, state.a, state.b, state.c, state.d,
                    aXAlign, aYAlign, bXAlign, bYAlign,
                    cXAlign, cYAlign, dXAlign, dYAlign);
            SkFILEStream inFile("../../experimental/Intersection/op.htm");
            if (!inFile.isValid()) {
                continue;
            }
            SkTDArray<char> inData;
            inData.setCount(inFile.getLength());
            size_t inLen = inData.count();
            inFile.read(inData.begin(), inLen);
            inFile.setPath(NULL);
            SkFILEWStream outFile("../../experimental/Intersection/xop.htm");
            if (!outFile.isValid()) {
                continue;
            }
            const char marker[] =
                "</div>\n"
                "\n"
                "<script type=\"text/javascript\">\n"
                "\n"
                "var testDivs = [\n";
            const char testLineStr[] = "    testLine";
            char* insert = strstr(inData.begin(), marker);   
            if (!insert) {
                continue;
            }
            size_t startLen = insert - inData.begin();
            insert += sizeof(marker);
            const char* numLoc = insert + sizeof(testLineStr);
            int testNumber = atoi(numLoc) + 1;
            outFile.write(inData.begin(), startLen);
            outFile.writeText("<div id=\"testLine");
            outFile.writeDecAsText(testNumber);
            outFile.writeText("\">\n");
            outFile.writeText(pathStr);
            outFile.writeText("</div>\n\n");
            outFile.writeText(marker);
            outFile.writeText(testLineStr);
            outFile.writeDecAsText(testNumber);
            outFile.writeText(",\n");
            outFile.write(insert, inLen - startLen - sizeof(marker));
            outFile.flush();
        }               
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

const int maxThreads = 1; // gRunTestsInOneThread ? 1 : 24;

void Simplify4x4RectsThreaded_Test()
{
    State4 threadState[maxThreads];
    int threadIndex = 0;
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
                }
            }
        }
    }
    waitForCompletion(threadState, threadIndex);
}

