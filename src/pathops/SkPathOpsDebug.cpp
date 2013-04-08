/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPathOpsDebug.h"

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
