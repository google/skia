/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPathOpsDebug.h"
#include "SkPath.h"

#if defined SK_DEBUG || !FORCE_RELEASE

int gDebugMaxWindSum = SK_MaxS32;
int gDebugMaxWindValue = SK_MaxS32;

void mathematica_ize(char* str, size_t bufferLen) {
    size_t len = strlen(str);
    bool num = false;
    for (size_t idx = 0; idx < len; ++idx) {
        if (num && str[idx] == 'e') {
            if (len + 2 >= bufferLen) {
                return;
            }
            memmove(&str[idx + 2], &str[idx + 1], len - idx);
            str[idx] = '*';
            str[idx + 1] = '^';
            ++len;
        }
        num = str[idx] >= '0' && str[idx] <= '9';
    }
}
#endif

#if DEBUG_SORT || DEBUG_SWAP_TOP
bool valid_wind(int wind) {
    return wind > SK_MinS32 + 0xFFFF && wind < SK_MaxS32 - 0xFFFF;
}

void winding_printf(int wind) {
    if (wind == SK_MinS32) {
        SkDebugf("?");
    } else {
        SkDebugf("%d", wind);
    }
}
#endif

#if DEBUG_DUMP
const char* kLVerbStr[] = {"", "line", "quad", "cubic"};
// static const char* kUVerbStr[] = {"", "Line", "Quad", "Cubic"};
int gContourID;
int gSegmentID;
#endif

#if DEBUG_SORT || DEBUG_SWAP_TOP
int gDebugSortCountDefault = SK_MaxS32;
int gDebugSortCount;
#endif

#if DEBUG_ACTIVE_OP
const char* kPathOpStr[] = {"diff", "sect", "union", "xor"};
#endif

#if DEBUG_SHOW_PATH
static void showPathContours(SkPath::Iter& iter, const char* pathName) {
    uint8_t verb;
    SkPoint pts[4];
    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kMove_Verb:
                SkDebugf("    %s.moveTo(%#1.9gf, %#1.9gf);\n", pathName, pts[0].fX, pts[0].fY);
                continue;
            case SkPath::kLine_Verb:
                SkDebugf("    %s.lineTo(%#1.9gf, %#1.9gf);\n", pathName, pts[1].fX, pts[1].fY);
                break;
            case SkPath::kQuad_Verb:
                SkDebugf("    %s.quadTo(%#1.9gf, %#1.9gf, %#1.9gf, %#1.9gf);\n", pathName,
                    pts[1].fX, pts[1].fY, pts[2].fX, pts[2].fY);
                break;
            case SkPath::kCubic_Verb:
                SkDebugf("    %s.cubicTo(%#1.9gf, %#1.9gf, %#1.9gf, %#1.9gf, %#1.9gf, %#1.9gf);\n",
                    pathName, pts[1].fX, pts[1].fY, pts[2].fX, pts[2].fY, pts[3].fX, pts[3].fY);
                break;
            case SkPath::kClose_Verb:
                SkDebugf("    %s.close();\n", pathName);
                break;
            default:
                SkDEBUGFAIL("bad verb");
                return;
        }
    }
}

static const char* gFillTypeStr[] = {
    "kWinding_FillType",
    "kEvenOdd_FillType",
    "kInverseWinding_FillType",
    "kInverseEvenOdd_FillType"
};


void ShowFunctionHeader() {
    SkDebugf("\nstatic void test#(skiatest::Reporter* reporter) {\n");
}

void ShowPath(const SkPath& path, const char* pathName) {
    SkPath::Iter iter(path, true);
    SkPath::FillType fillType = path.getFillType();
    SkASSERT(fillType >= SkPath::kWinding_FillType && fillType <= SkPath::kInverseEvenOdd_FillType);
    SkDebugf("    SkPath %s;\n", pathName);
    SkDebugf("    %s.setFillType(SkPath::%s);\n", pathName, gFillTypeStr[fillType]);
    iter.setPath(path, true);
    showPathContours(iter, pathName);
}

static const char* gOpStrs[] = {
    "kDifference_PathOp",
    "kIntersect_PathOp",
    "kUnion_PathOp",
    "kXor_PathOp",
    "kReverseDifference_PathOp",
};

void ShowOp(SkPathOp op, const char* pathOne, const char* pathTwo) {
    SkDebugf("    testPathOp(reporter, %s, %s, %s);\n", pathOne, pathTwo, gOpStrs[op]);
    SkDebugf("}\n");
}
#endif
