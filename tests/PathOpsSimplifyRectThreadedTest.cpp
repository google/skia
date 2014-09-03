/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "PathOpsExtendedTest.h"
#include "PathOpsThreadedCommon.h"

// four rects, of four sizes
// for 3 smaller sizes, tall, wide
    // top upper mid lower bottom aligned (3 bits, 5 values)
    // same with x (3 bits, 5 values)
// not included, square, tall, wide (2 bits)
// cw or ccw (1 bit)

static void testSimplify4x4RectsMain(PathOpsThreadState* data)
{
    SkASSERT(data);
    PathOpsThreadState& state = *data;
    char pathStr[1024];  // gdb: set print elements 400
    bool progress = state.fReporter->verbose(); // FIXME: break out into its own parameter?
    if (progress) {
        sk_bzero(pathStr, sizeof(pathStr));
    }
    int aShape = state.fA & 0x03;
    SkPath::Direction aCW = state.fA >> 2 ? SkPath::kCCW_Direction : SkPath::kCW_Direction;
    int bShape = state.fB & 0x03;
    SkPath::Direction bCW = state.fB >> 2 ? SkPath::kCCW_Direction : SkPath::kCW_Direction;
    int cShape = state.fC & 0x03;
    SkPath::Direction cCW = state.fC >> 2 ? SkPath::kCCW_Direction : SkPath::kCW_Direction;
    int dShape = state.fD & 0x03;
    SkPath::Direction dCW = state.fD >> 2 ? SkPath::kCCW_Direction : SkPath::kCW_Direction;
    for (int aXAlign = 0; aXAlign < 5; ++aXAlign) {
        for (int aYAlign = 0; aYAlign < 5; ++aYAlign) {
            for (int bXAlign = 0; bXAlign < 5; ++bXAlign) {
                for (int bYAlign = 0; bYAlign < 5; ++bYAlign) {
                    for (int cXAlign = 0; cXAlign < 5; ++cXAlign) {
                         for (int cYAlign = 0; cYAlign < 5; ++cYAlign) {
                            for (int dXAlign = 0; dXAlign < 5; ++dXAlign) {
    for (int dYAlign = 0; dYAlign < 5; ++dYAlign) {
        SkPath path, out;
        char* str = pathStr;
        path.setFillType(SkPath::kWinding_FillType);
        int l, t, r, b;
        if (aShape) {
            switch (aShape) {
                case 1:  // square
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
            path.addRect(SkIntToScalar(l), SkIntToScalar(t), SkIntToScalar(r), SkIntToScalar(b),
                    aCW);
            if (progress) {
                str += sprintf(str, "    path.addRect(%d, %d, %d, %d,"
                        " SkPath::kC%sW_Direction);\n", l, t, r, b, aCW ? "C" : "");
            }
        } else {
            aXAlign = 5;
            aYAlign = 5;
        }
        if (bShape) {
            switch (bShape) {
                case 1:  // square
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
            path.addRect(SkIntToScalar(l), SkIntToScalar(t), SkIntToScalar(r), SkIntToScalar(b),
                    bCW);
            if (progress) {
                str += sprintf(str, "    path.addRect(%d, %d, %d, %d,"
                        " SkPath::kC%sW_Direction);\n", l, t, r, b, bCW ? "C" : "");
            }
        } else {
            bXAlign = 5;
            bYAlign = 5;
        }
        if (cShape) {
            switch (cShape) {
                case 1:  // square
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
            path.addRect(SkIntToScalar(l), SkIntToScalar(t), SkIntToScalar(r), SkIntToScalar(b),
                    cCW);
            if (progress) {
                str += sprintf(str, "    path.addRect(%d, %d, %d, %d,"
                        " SkPath::kC%sW_Direction);\n", l, t, r, b, cCW ? "C" : "");
            }
        } else {
            cXAlign = 5;
            cYAlign = 5;
        }
        if (dShape) {
            switch (dShape) {
                case 1:  // square
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
            path.addRect(SkIntToScalar(l), SkIntToScalar(t), SkIntToScalar(r), SkIntToScalar(b),
                    dCW);
            if (progress) {
                str += sprintf(str, "    path.addRect(%d, %d, %d, %d,"
                        " SkPath::kC%sW_Direction);\n", l, t, r, b, dCW ? "C" : "");
            }
        } else {
            dXAlign = 5;
            dYAlign = 5;
        }
        path.close();
        if (progress) {
            outputProgress(state.fPathStr, pathStr, SkPath::kWinding_FillType);
        }
        testSimplify(path, false, out, state, pathStr);
        if (progress) {
            outputProgress(state.fPathStr, pathStr, SkPath::kEvenOdd_FillType);
        }
        testSimplify(path, true, out, state, pathStr);
    }
                            }
                        }
                    }
                }
            }
        }
    }
}

DEF_TEST(PathOpsSimplifyRectsThreaded, reporter) {
    initializeTests(reporter, "testLine");
    PathOpsThreadedTestRunner testRunner(reporter);
    for (int a = 0; a < 8; ++a) {  // outermost
        for (int b = a ; b < 8; ++b) {
            for (int c = b ; c < 8; ++c) {
                for (int d = c; d < 8; ++d) {
                        *testRunner.fRunnables.append() = SkNEW_ARGS(PathOpsThreadedRunnable,
                                (&testSimplify4x4RectsMain, a, b, c, d, &testRunner));
                }
                if (!reporter->allowExtendedTest()) goto finish;
            }
        }
    }
finish:
    testRunner.render();
}
