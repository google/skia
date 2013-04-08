/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "PathOpsExtendedTest.h"

static THREAD_TYPE testSimplifyQuadsMain(void* data)
{
    SkASSERT(data);
    State4& state = *(State4*) data;
    char pathStr[1024];
    sk_bzero(pathStr, sizeof(pathStr));
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
                        path.moveTo(SkIntToScalar(ax), SkIntToScalar(ay));
                        path.quadTo(SkIntToScalar(bx), SkIntToScalar(by),
                                SkIntToScalar(cx), SkIntToScalar(cy));
                        path.lineTo(SkIntToScalar(dx), SkIntToScalar(dy));
                        path.close();
                        path.moveTo(SkIntToScalar(ex), SkIntToScalar(ey));
                        path.lineTo(SkIntToScalar(fx), SkIntToScalar(fy));
                        path.quadTo(SkIntToScalar(gx), SkIntToScalar(gy),
                                SkIntToScalar(hx), SkIntToScalar(hy));
                        path.close();
                        // gdb: set print elements 400
                        char* str = pathStr;
                        str += sprintf(str, "    path.moveTo(%d, %d);\n", ax, ay);
                        str += sprintf(str, "    path.quadTo(%d, %d, %d, %d);\n", bx, by, cx, cy);
                        str += sprintf(str, "    path.lineTo(%d, %d);\n", dx, dy);
                        str += sprintf(str, "    path.close();\n");
                        str += sprintf(str, "    path.moveTo(%d, %d);\n", ex, ey);
                        str += sprintf(str, "    path.lineTo(%d, %d);\n", fx, fy);
                        str += sprintf(str, "    path.quadTo(%d, %d, %d, %d);\n", gx, gy, hx, hy);
                        str += sprintf(str, "    path.close();\n");
                        outputProgress(state, pathStr, SkPath::kWinding_FillType);
                        testSimplify(path, false, out, state, pathStr);
                        state.testsRun++;
                        path.setFillType(SkPath::kEvenOdd_FillType);
                        outputProgress(state, pathStr, SkPath::kEvenOdd_FillType);
                        testSimplify(path, true, out, state, pathStr);
                        state.testsRun++;
                    }
                }
            }
        }
    } while (runNextTestSet(state));
    THREAD_RETURN
}

static void TestSimplifyQuadsThreaded(skiatest::Reporter* reporter)
{
    int testsRun = 0;
    if (gShowTestProgress) SkDebugf("%s\n", __FUNCTION__);
#ifdef SK_DEBUG
    gDebugMaxWindSum = 4;  // FIXME: 3?
    gDebugMaxWindValue = 4;
#endif
    const char testStr[] = "testQuads";
    initializeTests(reporter, testStr, sizeof(testStr));
    int a = 0;
    for (; a < 16; ++a) {
        for (int b = a ; b < 16; ++b) {
            for (int c = b ; c < 16; ++c) {
                for (int d = c; d < 16; ++d) {
                    testsRun += dispatchTest4(testSimplifyQuadsMain, a, b, c, d);
                }
                if (!gAllowExtendedTest) goto finish;
                if (gShowTestProgress) SkDebugf(".");
            }
            if (gShowTestProgress) SkDebugf("%d", b);
        }
        if (gShowTestProgress) SkDebugf("\n%d", a);
    }
finish:
    testsRun += waitForCompletion();
    if (gShowTestProgress) SkDebugf("%s tests=%d\n", __FUNCTION__, testsRun);
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("PathOpsSimplifyQuadsThreaded", SimplifyQuadsThreadedTestClass, \
        TestSimplifyQuadsThreaded)
