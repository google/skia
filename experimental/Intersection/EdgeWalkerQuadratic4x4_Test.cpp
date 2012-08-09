#include "EdgeWalker_Test.h"
#include "Intersection_Tests.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include <assert.h>


static void* testSimplify4x4QuadraticsMain(void* data)
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
                        path.quadTo(bx, by, cx, cy);
                        path.lineTo(dx, dy);
                        path.close();
                        path.moveTo(ex, ey);
                        path.lineTo(fx, fy);
                        path.quadTo(gx, gy, hx, hy);
                        path.close();
                        if (1) {  // gdb: set print elements 400
                            char* str = pathStr;
                            str += sprintf(str, "    path.moveTo(%d, %d);\n", ax, ay);
                            str += sprintf(str, "    path.quadTo(%d, %d, %d, %d);\n", bx, by, cx, cy);
                            str += sprintf(str, "    path.lineTo(%d, %d);\n", dx, dy);
                            str += sprintf(str, "    path.close();\n");
                            str += sprintf(str, "    path.moveTo(%d, %d);\n", ex, ey);
                            str += sprintf(str, "    path.lineTo(%d, %d);\n", fx, fy);
                            str += sprintf(str, "    path.quadTo(%d, %d, %d, %d);\n", gx, gy, hx, hy);
                            str += sprintf(str, "    path.close();\n");
                        }
                        outputProgress(state, pathStr);
                        testSimplifyx(path, out, state, pathStr);
                        state.testsRun++;
                #if 0 // FIXME: enable once we have support for even/odd
                        path.setFillType(SkPath::kEvenOdd_FillType);
                        outputProgress(state, pathStr, SkPath::kEvenOdd_FillType);
                        testSimplifyx(path, true, out, state, pathStr);
                        state.testsRun++;
                #endif
                    }
                }
            }
        }
    } while (runNextTestSet(state));
    return NULL;
}

void Simplify4x4QuadraticsThreaded_Test()
{
    SkDebugf("%s\n", __FUNCTION__);
#ifdef SK_DEBUG
    gDebugMaxWindSum = 4; // FIXME: 3?
    gDebugMaxWindValue = 4;
#endif
    const char testStr[] = "testQuadratic";
    initializeTests(testStr, sizeof(testStr));
    int testsRun = 0;
    for (int a = 0; a < 16; ++a) {
        for (int b = a ; b < 16; ++b) {
            for (int c = b ; c < 16; ++c) {
                for (int d = c; d < 16; ++d) {                 
                    testsRun += dispatchTest4(testSimplify4x4QuadraticsMain,
                            a, b, c, d);
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
