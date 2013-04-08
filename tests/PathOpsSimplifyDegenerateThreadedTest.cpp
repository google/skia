/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "PathOpsExtendedTest.h"

static THREAD_TYPE testSimplifyDegeneratesMain(void* data) {
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
                    path.moveTo(SkIntToScalar(ax), SkIntToScalar(ay));
                    path.lineTo(SkIntToScalar(bx), SkIntToScalar(by));
                    path.lineTo(SkIntToScalar(cx), SkIntToScalar(cy));
                    path.close();
                    path.moveTo(SkIntToScalar(dx), SkIntToScalar(dy));
                    path.lineTo(SkIntToScalar(ex), SkIntToScalar(ey));
                    path.lineTo(SkIntToScalar(fx), SkIntToScalar(fy));
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
                    testSimplify(path, false, out, state, pathStr);
                    state.testsRun++;
                    path.setFillType(SkPath::kEvenOdd_FillType);
                    outputProgress(state, pathStr, SkPath::kEvenOdd_FillType);
                    testSimplify(path, true, out, state, pathStr);
                    state.testsRun++;
                }
            }
        }
    } while (runNextTestSet(state));
    THREAD_RETURN
}

static void TestSimplifyDegeneratesThreaded(skiatest::Reporter* reporter) {
    int testsRun = 0;
    if (gShowTestProgress) SkDebugf("%s\n", __FUNCTION__);
#ifdef SK_DEBUG
    gDebugMaxWindSum = 2;
    gDebugMaxWindValue = 2;
#endif
    const char testStr[] = "testDegenerates";
    initializeTests(reporter, testStr, sizeof(testStr));
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
                testsRun += dispatchTest4(testSimplifyDegeneratesMain,
                        a, b, c, abcIsATriangle);
            }
            if (!gAllowExtendedTest) goto finish;
            if (gShowTestProgress) SkDebugf(".");
        }
        if (gShowTestProgress) SkDebugf("\n%d", a);
    }
finish:
    testsRun += waitForCompletion();
    if (gShowTestProgress) SkDebugf("%s tests=%d\n", __FUNCTION__, testsRun);
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("PathOpsSimplifyDegeneratesThreaded", SimplifyDegeneratesThreadedTestClass, \
        TestSimplifyDegeneratesThreaded)
