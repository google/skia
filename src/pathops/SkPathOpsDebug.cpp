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

#if DEBUG_SHOW_TEST_NAME
void* PathOpsDebugCreateNameStr() {
    return SkNEW_ARRAY(char, DEBUG_FILENAME_STRING_LENGTH);
}

void PathOpsDebugDeleteNameStr(void* v) {
    SkDELETE_ARRAY(reinterpret_cast<char* >(v));
}

void DebugBumpTestName(char* test) {
    char* num = test + strlen(test);
    while (num[-1] >= '0' && num[-1] <= '9') {
        --num;
    }
    if (num[0] == '\0') {
        return;
    }
    int dec = atoi(num);
    if (dec == 0) {
        return;
    }
    ++dec;
    SK_SNPRINTF(num, DEBUG_FILENAME_STRING_LENGTH - (num - test), "%d", dec);
}
#endif
