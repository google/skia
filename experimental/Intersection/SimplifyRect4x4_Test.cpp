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
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/sysctl.h>

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
#if 0
static const char filename[] = "../../experimental/Intersection/debugXX.txt";
#else
static const char filename[] = "/flash/debug/XX.txt";
#endif
static int testNumber;

#define BETTER_THREADS 01
#define DEBUG_BETTER_THREADS 0

static void* testSimplify4x4RectsMain(void* data)
{
    SkASSERT(data);
    State4& state = *(State4*) data;
#if BETTER_THREADS
    do {
#endif
    char pathStr[1024]; // gdb: set print elements 400
    bzero(pathStr, sizeof(pathStr));
    state.testsRun = 0;
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
    #if 0
            char pwd[1024];
            getcwd(pwd, sizeof(pwd));
            SkDebugf("%s\n", pwd);
    #endif
    #if 1
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
            outFile.writeText("    testSimplifyx(path);\n}\n\n");
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
    #endif
        }
        testSimplifyx(path, out, state.bitmap, state.canvas);
        state.testsRun++;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    if (gRunTestsInOneThread) {
        return NULL;
    }
#if BETTER_THREADS
    if (DEBUG_BETTER_THREADS) SkDebugf("%s done %d\n", __FUNCTION__, state.index);
    pthread_mutex_lock(&State4::addQueue);
    if (DEBUG_BETTER_THREADS) SkDebugf("%s lock %d\n", __FUNCTION__, state.index);
    state.next = State4::queue ? State4::queue->next : NULL;
    state.done = true;
    State4::queue = &state;
    pthread_cond_signal(&State4::checkQueue);
    while (state.done && !state.last) {
        if (DEBUG_BETTER_THREADS) SkDebugf("%s wait %d\n", __FUNCTION__, state.index);
        pthread_cond_wait(&state.initialized, &State4::addQueue);
        if (DEBUG_BETTER_THREADS) SkDebugf("%s wait done %d\n", __FUNCTION__, state.index);
    }
    pthread_mutex_unlock(&State4::addQueue);
    if (DEBUG_BETTER_THREADS) SkDebugf("%s unlock %d\n", __FUNCTION__, state.index);
} while (!state.last);
#endif
    return NULL;
}

const int maxThreadsAllocated = 64;

State4* State4::queue = NULL;
pthread_mutex_t State4::addQueue = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t State4::checkQueue = PTHREAD_COND_INITIALIZER;

void Simplify4x4RectsThreaded_Test()
{
#ifdef SK_DEBUG
    gDebugMaxWindSum = 4;
    gDebugMaxWindValue = 4;
#endif
    int maxThreads = 1;
    if (!gRunTestsInOneThread) {
        int threads = -1;
        size_t size = sizeof(threads);
        sysctlbyname("hw.logicalcpu_max", &threads, &size, NULL, 0);
        SkDebugf("%s errno=%d size=%d processors=%d\n", __FUNCTION__, 
                errno, size, threads);
        if (threads > 0) {
            maxThreads = threads;
        } else {
            maxThreads = 8;
        }
        SkDebugf("%s maxThreads=%d\n", __FUNCTION__, maxThreads);
    }
    if (!gRunTestsInOneThread) {
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
    State4 threadState[maxThreadsAllocated];
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
    int testsRun = 0;
    for (int a = 0; a < 8; ++a) { // outermost
        for (int b = a ; b < 8; ++b) {
            for (int c = b ; c < 8; ++c) {
                for (int d = c; d < 8; ++d) {                 
                    if (!gRunTestsInOneThread) {
                #if BETTER_THREADS == 0
                        State4* statePtr = &threadState[threadIndex];
                        statePtr->a = a;
                        statePtr->b = b;
                        statePtr->c = c;
                        statePtr->d = d;
                        createThread(statePtr, testSimplify4x4RectsMain);
                        if (++threadIndex >= maxThreads) {
                            waitForCompletion(threadState, threadIndex);
                            for (int index = 0; index < maxThreads; ++index) {
                                testsRun += threadState[index].testsRun;
                            }
                        }
                #else
                        State4* statePtr;
                        pthread_mutex_lock(&State4::addQueue);
                        if (threadIndex < maxThreads) {
                            statePtr = &threadState[threadIndex];
                            statePtr->a = a;
                            statePtr->b = b;
                            statePtr->c = c;
                            statePtr->d = d;
                            statePtr->index = threadIndex;
                            statePtr->done = false;
                            statePtr->last = false;
                            pthread_cond_init(&statePtr->initialized, NULL);
                            ++threadIndex;
                            createThread(statePtr, testSimplify4x4RectsMain);
                        } else {
                            while (!State4::queue) {
                                if (DEBUG_BETTER_THREADS) SkDebugf("%s wait\n", __FUNCTION__);
                                pthread_cond_wait(&State4::checkQueue, &State4::addQueue);
                            }
                            statePtr = State4::queue;
                            testsRun += statePtr->testsRun;
                            if (DEBUG_BETTER_THREADS) SkDebugf("%s dequeue %d\n", __FUNCTION__, statePtr->index);
                            statePtr->a = a;
                            statePtr->b = b;
                            statePtr->c = c;
                            statePtr->d = d;
                            statePtr->done = false;
                            State4::queue = State4::queue->next;
                            pthread_cond_signal(&statePtr->initialized);
                        }
                        pthread_mutex_unlock(&State4::addQueue);
                #endif
                    } else {
                        State4 state;
                        state.a = a;
                        state.b = b;
                        state.c = c;
                        state.d = d;
                        testSimplify4x4RectsMain(&state);
                    }
                    if (!gRunTestsInOneThread) SkDebugf(".");
                }
                if (!gRunTestsInOneThread) SkDebugf("%d", c);
            }
            if (!gRunTestsInOneThread) SkDebugf("\n%d", b);
        }
        if (!gRunTestsInOneThread) SkDebugf("\n\n%d", a);
    }
    if (!gRunTestsInOneThread) {
        pthread_mutex_lock(&State4::addQueue);
        int runningThreads = maxThreads;
        while (runningThreads > 0) {
            while (!State4::queue) {
                if (DEBUG_BETTER_THREADS) SkDebugf("%s wait\n", __FUNCTION__);
                pthread_cond_wait(&State4::checkQueue, &State4::addQueue);
            }
            while (State4::queue) {
                State4::queue->last = true;
                pthread_cond_signal(&State4::queue->initialized);
                State4::queue = State4::queue->next;
                --runningThreads;
            }
        }
        pthread_mutex_unlock(&State4::addQueue);
        for (threadIndex = 0; threadIndex < maxThreads; ++threadIndex) {
            pthread_join(threadState[threadIndex].threadID, NULL);
            testsRun += threadState[threadIndex].testsRun;
        }
        SkDebugf("%s total tests run=%d\n", __FUNCTION__, testsRun);
    }
#ifdef SK_DEBUG
    gDebugMaxWindSum = SK_MaxS32;
    gDebugMaxWindValue = SK_MaxS32;
#endif
}

