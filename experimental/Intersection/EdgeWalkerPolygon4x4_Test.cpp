#include "EdgeWalker_Test.h"
#include "Intersection_Tests.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include <assert.h>
#include <pthread.h>

struct State {
    State() {
    bitmap.setConfig(SkBitmap::kARGB_8888_Config, 150 * 2, 100);
    bitmap.allocPixels();
    canvas = new SkCanvas(bitmap);
    }

    int a;
    int b;
    int c;
    int d;
    pthread_t threadID;
    SkCanvas* canvas;
    SkBitmap bitmap;
    bool abcIsATriangle;
};

void createThread(State* statePtr, void* (*test)(void* )) {
    int threadError = pthread_create(&statePtr->threadID, NULL, test,
            (void*) statePtr);
    SkASSERT(!threadError);
}

void waitForCompletion(State threadState[], int& threadIndex) {
    for (int index = 0; index < threadIndex; ++index) {
        pthread_join(threadState[index].threadID, NULL);
    }
    SkDebugf(".");
    threadIndex = 0;
}

static void* testSimplify4x4QuadralateralsMain(void* data)
{
    char pathStr[1024];
    bzero(pathStr, sizeof(pathStr));
    SkASSERT(data);
    State& state = *(State*) data;
    int ax = state.a & 0x03;
    int ay = state.a >> 2;
    int bx = state.b & 0x03;
    int by = state.b >> 2;
    int cx = state.c & 0x03;
    int cy = state.c >> 2;
    int dx = state.d & 0x03;
    int dy = state.d >> 2;
    for (int e = 0 ; e < 16; ++e) {
        int ex = e & 0x03;
        int ey = e >> 2;
        for (int f = e ; f < 16; ++f) {
            int fx = f & 0x03;
            int fy = f >> 2;
            for (int g = f ; g < 16; ++g) {
                int gx = g & 0x03;
                int gy = g >> 2;
                for (int h = g ; h < 16; ++h) {
                    int hx = h & 0x03;
                    int hy = h >> 2;
                    SkPath path, out;
                    path.setFillType(SkPath::kWinding_FillType);
                    path.moveTo(ax, ay);
                    path.lineTo(bx, by);
                    path.lineTo(cx, cy);
                    path.lineTo(dx, dy);
                    path.close();
                    path.moveTo(ex, ey);
                    path.lineTo(fx, fy);
                    path.lineTo(gx, gy);
                    path.lineTo(hx, hy);
                    path.close();
                    if (1) {  // gdb: set print elements 400
                        char* str = pathStr;
                        str += sprintf(str, "    path.moveTo(%d, %d);\n", ax, ay);
                        str += sprintf(str, "    path.lineTo(%d, %d);\n", bx, by);
                        str += sprintf(str, "    path.lineTo(%d, %d);\n", cx, cy);
                        str += sprintf(str, "    path.lineTo(%d, %d);\n", dx, dy);
                        str += sprintf(str, "    path.close();\n");
                        str += sprintf(str, "    path.moveTo(%d, %d);\n", ex, ey);
                        str += sprintf(str, "    path.lineTo(%d, %d);\n", fx, fy);
                        str += sprintf(str, "    path.lineTo(%d, %d);\n", gx, gy);
                        str += sprintf(str, "    path.lineTo(%d, %d);\n", hx, hy);
                        str += sprintf(str, "    path.close();");
                    }
                    if (!testSimplify(path, true, out, state.bitmap, state.canvas)) {
                        SkDebugf("*/\n{ SkPath::kWinding_FillType, %d, %d, %d, %d,"
                                " %d, %d, %d, %d },\n/*\n", state.a, state.b, state.c, state.d,
                                e, f, g, h);
                    }
                    path.setFillType(SkPath::kEvenOdd_FillType);
                    if (!testSimplify(path, true, out, state.bitmap, state.canvas)) {
                        SkDebugf("*/\n{ SkPath::kEvenOdd_FillType, %d, %d, %d, %d,"
                                " %d, %d, %d, %d },\n/*\n", state.a, state.b, state.c, state.d,
                                e, f, g, h);
                    }
                }
            }
        }
    }
    return NULL;
}

const int maxThreads = gShowDebugf ? 1 : 24;

void Simplify4x4QuadralateralsThreaded_Test()
{
    State threadState[maxThreads];
    int threadIndex = 0;
    for (int a = 0; a < 16; ++a) {
        for (int b = a ; b < 16; ++b) {
            for (int c = b ; c < 16; ++c) {
                for (int d = c; d < 16; ++d) {                 
                    State* statePtr = &threadState[threadIndex];
                    statePtr->a = a;
                    statePtr->b = b;
                    statePtr->c = c;
                    statePtr->d = d;
                    if (maxThreads > 1) {
                        createThread(statePtr, testSimplify4x4QuadralateralsMain);
                        if (++threadIndex >= maxThreads) {
                            waitForCompletion(threadState, threadIndex);
                        }
                    } else {
                        testSimplify4x4QuadralateralsMain(statePtr);
                    }
                }
            }
        }
    }
    waitForCompletion(threadState, threadIndex);
}


static void* testSimplify4x4NondegeneratesMain(void* data) {
    char pathStr[1024];
    bzero(pathStr, sizeof(pathStr));
    SkASSERT(data);
    State& state = *(State*) data;
    int ax = state.a & 0x03;
    int ay = state.a >> 2;
    int bx = state.b & 0x03;
    int by = state.b >> 2;
    int cx = state.c & 0x03;
    int cy = state.c >> 2;
    for (int d = 0; d < 15; ++d) {
        int dx = d & 0x03;
        int dy = d >> 2;
        for (int e = d + 1; e < 16; ++e) {
            int ex = e & 0x03;
            int ey = e >> 2;
            for (int f = d + 1; f < 16; ++f) {
                if (e == f) {
                    continue;
                }
                int fx = f & 0x03;
                int fy = f >> 2;
                if ((ex - dx) * (fy - dy) == (ey - dy) * (fx - dx)) {
                    continue;
                }
                SkPath path, out;
                path.setFillType(SkPath::kWinding_FillType);
                path.moveTo(ax, ay);
                path.lineTo(bx, by);
                path.lineTo(cx, cy);
                path.close();
                path.moveTo(dx, dy);
                path.lineTo(ex, ey);
                path.lineTo(fx, fy);
                path.close();
                if (1) {
                    char* str = pathStr;
                    str += sprintf(str, "    path.moveTo(%d, %d);\n", ax, ay);
                    str += sprintf(str, "    path.lineTo(%d, %d);\n", bx, by);
                    str += sprintf(str, "    path.lineTo(%d, %d);\n", cx, cy);
                    str += sprintf(str, "    path.close();\n");
                    str += sprintf(str, "    path.moveTo(%d, %d);\n", dx, dy);
                    str += sprintf(str, "    path.lineTo(%d, %d);\n", ex, ey);
                    str += sprintf(str, "    path.lineTo(%d, %d);\n", fx, fy);
                    str += sprintf(str, "    path.close();");
                }
                testSimplify(path, true, out, state.bitmap, state.canvas);
                path.setFillType(SkPath::kEvenOdd_FillType);
                testSimplify(path, true, out, state.bitmap, state.canvas);
            }
        }
    }
    return NULL;
}

void SimplifyNondegenerate4x4TrianglesThreaded_Test() {
    State threadState[maxThreads];
    int threadIndex = 0;
    for (int a = 0; a < 15; ++a) {
        int ax = a & 0x03;
        int ay = a >> 2;
        for (int b = a + 1; b < 16; ++b) {
            int bx = b & 0x03;
            int by = b >> 2;
            for (int c = a + 1; c < 16; ++c) {
                if (b == c) {
                    continue;
                }
                int cx = c & 0x03;
                int cy = c >> 2;
                if ((bx - ax) * (cy - ay) == (by - ay) * (cx - ax)) {
                    continue;
                }
                State* statePtr = &threadState[threadIndex];
                statePtr->a = a;
                statePtr->b = b;
                statePtr->c = c;
                if (maxThreads > 1) {
                    createThread(statePtr, testSimplify4x4NondegeneratesMain);
                    if (++threadIndex >= maxThreads) {
                        waitForCompletion(threadState, threadIndex);
                    }
                } else {
                    testSimplify4x4NondegeneratesMain(statePtr);
                }
            }
        }
    }
    waitForCompletion(threadState, threadIndex);
}

static void* testSimplify4x4DegeneratesMain(void* data) {
    char pathStr[1024];
    bzero(pathStr, sizeof(pathStr));
    SkASSERT(data);
    State& state = *(State*) data;
    int ax = state.a & 0x03;
    int ay = state.a >> 2;
    int bx = state.b & 0x03;
    int by = state.b >> 2;
    int cx = state.c & 0x03;
    int cy = state.c >> 2;
    for (int d = 0; d < 16; ++d) {
        int dx = d & 0x03;
        int dy = d >> 2;
        for (int e = d ; e < 16; ++e) {
            int ex = e & 0x03;
            int ey = e >> 2;
            for (int f = d ; f < 16; ++f) {
                int fx = f & 0x03;
                int fy = f >> 2;
                if (state.abcIsATriangle && (ex - dx) * (fy - dy)
                        != (ey - dy) * (fx - dx)) {
                    continue;
                }
                SkPath path, out;
                path.setFillType(SkPath::kWinding_FillType);
                path.moveTo(ax, ay);
                path.lineTo(bx, by);
                path.lineTo(cx, cy);
                path.close();
                path.moveTo(dx, dy);
                path.lineTo(ex, ey);
                path.lineTo(fx, fy);
                path.close();
                if (1) {
                    char* str = pathStr;
                    str += sprintf(str, "    path.moveTo(%d, %d);\n", ax, ay);
                    str += sprintf(str, "    path.lineTo(%d, %d);\n", bx, by);
                    str += sprintf(str, "    path.lineTo(%d, %d);\n", cx, cy);
                    str += sprintf(str, "    path.close();\n");
                    str += sprintf(str, "    path.moveTo(%d, %d);\n", dx, dy);
                    str += sprintf(str, "    path.lineTo(%d, %d);\n", ex, ey);
                    str += sprintf(str, "    path.lineTo(%d, %d);\n", fx, fy);
                    str += sprintf(str, "    path.close();");
                }
                testSimplify(path, true, out, state.bitmap, state.canvas);
                path.setFillType(SkPath::kEvenOdd_FillType);
                testSimplify(path, true, out, state.bitmap, state.canvas);
            }
        }
    }
    return NULL;
}

void SimplifyDegenerate4x4TrianglesThreaded_Test() {
    State threadState[maxThreads];
    int threadIndex = 0;
    for (int a = 0; a < 16; ++a) {
        int ax = a & 0x03;
        int ay = a >> 2;
        for (int b = a ; b < 16; ++b) {
            int bx = b & 0x03;
            int by = b >> 2;
            for (int c = a ; c < 16; ++c) {
                int cx = c & 0x03;
                int cy = c >> 2;
                State* statePtr = &threadState[threadIndex];
                statePtr->abcIsATriangle = (bx - ax) * (cy - ay)
                        != (by - ay) * (cx - ax);
                statePtr->a = a;
                statePtr->b = b;
                statePtr->c = c;
                if (maxThreads > 1) {
                    createThread(statePtr, testSimplify4x4DegeneratesMain);
                    if (++threadIndex >= maxThreads) {
                        waitForCompletion(threadState, threadIndex);
                    }
                } else {
                    testSimplify4x4DegeneratesMain(statePtr);
                }
            }
        }
    }
    waitForCompletion(threadState, threadIndex);
}

