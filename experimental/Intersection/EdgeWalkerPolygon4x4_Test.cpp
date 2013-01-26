/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "EdgeWalker_Test.h"
#include "Intersection_Tests.h"

static void* testSimplify4x4QuadralateralsMain(void* data)
{
    SkASSERT(data);
    State4& state = *(State4*) data;
    char pathStr[1024];
    bzero(pathStr, sizeof(pathStr));
    do {
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
                            str += sprintf(str, "    path.close();\n");
                        }
                        outputProgress(state, pathStr, SkPath::kWinding_FillType);
                        testSimplifyx(path, false, out, state, pathStr);
                        state.testsRun++;
                        path.setFillType(SkPath::kEvenOdd_FillType);
                        outputProgress(state, pathStr, SkPath::kEvenOdd_FillType);
                        testSimplifyx(path, true, out, state, pathStr);
                        state.testsRun++;
                    }
                }
            }
        }
    } while (runNextTestSet(state));
    return NULL;
}

void Simplify4x4QuadralateralsThreaded_Test(int& testsRun)
{
    SkDebugf("%s\n", __FUNCTION__);
#ifdef SK_DEBUG
    gDebugMaxWindSum = 4; // FIXME: 3?
    gDebugMaxWindValue = 4;
#endif
    const char testStr[] = "testQuadralateral";
    initializeTests(testStr, sizeof(testStr));
    int testsStart = testsRun;
    for (int a = 0; a < 16; ++a) {
        for (int b = a ; b < 16; ++b) {
            for (int c = b ; c < 16; ++c) {
                for (int d = c; d < 16; ++d) {
                    testsRun += dispatchTest4(testSimplify4x4QuadralateralsMain,
                            a, b, c, d);
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


static void* testSimplify4x4NondegeneratesMain(void* data) {
    SkASSERT(data);
    State4& state = *(State4*) data;
    char pathStr[1024];
    bzero(pathStr, sizeof(pathStr));
    do {
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
                        str += sprintf(str, "    path.close();\n");
                    }
                    outputProgress(state, pathStr, SkPath::kWinding_FillType);
                    testSimplifyx(path, false, out, state, pathStr);
                    state.testsRun++;
                    path.setFillType(SkPath::kEvenOdd_FillType);
                    outputProgress(state, pathStr, SkPath::kEvenOdd_FillType);
                    testSimplifyx(path, true, out, state, pathStr);
                    state.testsRun++;
                }
            }
        }
    } while (runNextTestSet(state));
    return NULL;
}

void SimplifyNondegenerate4x4TrianglesThreaded_Test(int& testsRun) {
    SkDebugf("%s\n", __FUNCTION__);
#ifdef SK_DEBUG
    gDebugMaxWindSum = 2;
    gDebugMaxWindValue = 2;
#endif
    const char testStr[] = "testNondegenerate";
    initializeTests(testStr, sizeof(testStr));
    int testsStart = testsRun;
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
                testsRun += dispatchTest4(testSimplify4x4NondegeneratesMain,
                        a, b, c, 0);
            }
            if (!gRunTestsInOneThread) SkDebugf(".");
        }
        if (!gRunTestsInOneThread) SkDebugf("\n%d", a);
    }
    testsRun += waitForCompletion();
    SkDebugf("%s tests=%d total=%d\n", __FUNCTION__, testsRun - testsStart, testsRun);
}

static void* testSimplify4x4DegeneratesMain(void* data) {
    SkASSERT(data);
    State4& state = *(State4*) data;
    char pathStr[1024];
    bzero(pathStr, sizeof(pathStr));
    do {
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
                    if (state.d && (ex - dx) * (fy - dy)
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
                        str += sprintf(str, "    path.close();\n");
                    }
                    outputProgress(state, pathStr, SkPath::kWinding_FillType);
                    testSimplifyx(path, false, out, state, pathStr);
                    state.testsRun++;
                    path.setFillType(SkPath::kEvenOdd_FillType);
                    outputProgress(state, pathStr, SkPath::kEvenOdd_FillType);
                    testSimplifyx(path, true, out, state, pathStr);
                    state.testsRun++;
                }
            }
        }
    } while (runNextTestSet(state));
    return NULL;
}

void SimplifyDegenerate4x4TrianglesThreaded_Test(int& testsRun) {
    SkDebugf("%s\n", __FUNCTION__);
#ifdef SK_DEBUG
    gDebugMaxWindSum = 2;
    gDebugMaxWindValue = 2;
#endif
    const char testStr[] = "testDegenerate";
    initializeTests(testStr, sizeof(testStr));
    int testsStart = testsRun;
    for (int a = 0; a < 16; ++a) {
        int ax = a & 0x03;
        int ay = a >> 2;
        for (int b = a ; b < 16; ++b) {
            int bx = b & 0x03;
            int by = b >> 2;
            for (int c = a ; c < 16; ++c) {
                int cx = c & 0x03;
                int cy = c >> 2;
                bool abcIsATriangle = (bx - ax) * (cy - ay) != (by - ay) * (cx - ax);
                testsRun += dispatchTest4(testSimplify4x4DegeneratesMain,
                        a, b, c, abcIsATriangle);
            }
            if (!gRunTestsInOneThread) SkDebugf(".");
        }
        if (!gRunTestsInOneThread) SkDebugf("\n%d", a);
    }
    testsRun += waitForCompletion();
    SkDebugf("%s tests=%d total=%d\n", __FUNCTION__, testsRun - testsStart, testsRun);
}
