/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPathOpsDebug_DEFINED
#define SkPathOpsDebug_DEFINED

#include "SkPathOps.h"
#include "SkTypes.h"

#ifdef SK_RELEASE
#define FORCE_RELEASE 1
#else
#define FORCE_RELEASE 1  // set force release to 1 for multiple thread -- no debugging
#endif

#define ONE_OFF_DEBUG 0
#define ONE_OFF_DEBUG_MATHEMATICA 0

#if defined(SK_BUILD_FOR_WIN) || defined(SK_BUILD_FOR_ANDROID)
    #define SK_RAND(seed) rand()
#else
    #define SK_RAND(seed) rand_r(&seed)
#endif
#ifdef SK_BUILD_FOR_WIN
    #define SK_SNPRINTF _snprintf
#else
    #define SK_SNPRINTF snprintf
#endif

#if defined SK_DEBUG || !FORCE_RELEASE

void mathematica_ize(char* str, size_t bufferSize);

extern int gDebugMaxWindSum;
extern int gDebugMaxWindValue;

#endif

#if FORCE_RELEASE

#define DEBUG_ACTIVE_OP 0
#define DEBUG_ACTIVE_SPANS 0
#define DEBUG_ACTIVE_SPANS_FIRST_ONLY 0
#define DEBUG_ACTIVE_SPANS_SHORT_FORM 1
#define DEBUG_ADD_INTERSECTING_TS 0
#define DEBUG_ADD_T_PAIR 0
#define DEBUG_ANGLE 0
#define DEBUG_AS_C_CODE 1
#define DEBUG_ASSEMBLE 0
#define DEBUG_CONCIDENT 0
#define DEBUG_CROSS 0
#define DEBUG_FLAT_QUADS 0
#define DEBUG_FLOW 0
#define DEBUG_MARK_DONE 0
#define DEBUG_PATH_CONSTRUCTION 0
#define DEBUG_SHOW_TEST_NAME 0
#define DEBUG_SHOW_TEST_PROGRESS 0
#define DEBUG_SHOW_WINDING 0
#define DEBUG_SORT 0
#define DEBUG_SORT_COMPACT 0
#define DEBUG_SORT_SINGLE 0
#define DEBUG_SWAP_TOP 0
#define DEBUG_UNSORTABLE 0
#define DEBUG_VALIDATE 0
#define DEBUG_WIND_BUMP 0
#define DEBUG_WINDING 0
#define DEBUG_WINDING_AT_T 0

#else

#define DEBUG_ACTIVE_OP 1
#define DEBUG_ACTIVE_SPANS 1
#define DEBUG_ACTIVE_SPANS_FIRST_ONLY 0
#define DEBUG_ACTIVE_SPANS_SHORT_FORM 1
#define DEBUG_ADD_INTERSECTING_TS 1
#define DEBUG_ADD_T_PAIR 1
#define DEBUG_ANGLE 1
#define DEBUG_AS_C_CODE 1
#define DEBUG_ASSEMBLE 1
#define DEBUG_CONCIDENT 1
#define DEBUG_CROSS 0
#define DEBUG_FLAT_QUADS 0
#define DEBUG_FLOW 1
#define DEBUG_MARK_DONE 1
#define DEBUG_PATH_CONSTRUCTION 1
#define DEBUG_SHOW_TEST_NAME 1
#define DEBUG_SHOW_TEST_PROGRESS 1
#define DEBUG_SHOW_WINDING 0
#define DEBUG_SORT 1
#define DEBUG_SORT_COMPACT 0
#define DEBUG_SORT_SINGLE 0
#define DEBUG_SWAP_TOP 1
#define DEBUG_UNSORTABLE 1
#define DEBUG_VALIDATE 1
#define DEBUG_WIND_BUMP 0
#define DEBUG_WINDING 1
#define DEBUG_WINDING_AT_T 1

#endif

#define DEBUG_DUMP (DEBUG_ACTIVE_OP | DEBUG_ACTIVE_SPANS | DEBUG_CONCIDENT | DEBUG_SORT | \
        DEBUG_SORT_SINGLE | DEBUG_PATH_CONSTRUCTION)

#if DEBUG_AS_C_CODE
#define CUBIC_DEBUG_STR "{{%1.9g,%1.9g}, {%1.9g,%1.9g}, {%1.9g,%1.9g}, {%1.9g,%1.9g}}"
#define QUAD_DEBUG_STR  "{{%1.9g,%1.9g}, {%1.9g,%1.9g}, {%1.9g,%1.9g}}"
#define LINE_DEBUG_STR  "{{%1.9g,%1.9g}, {%1.9g,%1.9g}}"
#define PT_DEBUG_STR "{{%1.9g,%1.9g}}"
#else
#define CUBIC_DEBUG_STR "(%1.9g,%1.9g %1.9g,%1.9g %1.9g,%1.9g %1.9g,%1.9g)"
#define QUAD_DEBUG_STR  "(%1.9g,%1.9g %1.9g,%1.9g %1.9g,%1.9g)"
#define LINE_DEBUG_STR  "(%1.9g,%1.9g %1.9g,%1.9g)"
#define PT_DEBUG_STR "(%1.9g,%1.9g)"
#endif
#define T_DEBUG_STR(t, n) #t "[" #n "]=%1.9g"
#define TX_DEBUG_STR(t) #t "[%d]=%1.9g"
#define CUBIC_DEBUG_DATA(c) c[0].fX, c[0].fY, c[1].fX, c[1].fY, c[2].fX, c[2].fY, c[3].fX, c[3].fY
#define QUAD_DEBUG_DATA(q)  q[0].fX, q[0].fY, q[1].fX, q[1].fY, q[2].fX, q[2].fY
#define LINE_DEBUG_DATA(l)  l[0].fX, l[0].fY, l[1].fX, l[1].fY
#define PT_DEBUG_DATA(i, n) i.pt(n).fX, i.pt(n).fY

#if DEBUG_DUMP
extern const char* kLVerbStr[];
// extern const char* kUVerbStr[];
extern int gContourID;
extern int gSegmentID;
#endif

#if DEBUG_SORT || DEBUG_SWAP_TOP
extern int gDebugSortCountDefault;
extern int gDebugSortCount;

bool valid_wind(int winding);
void winding_printf(int winding);
#endif

#if DEBUG_ACTIVE_OP
extern const char* kPathOpStr[];
#endif

#if DEBUG_SHOW_TEST_NAME
#include "SkTLS.h"

extern void* PathOpsDebugCreateNameStr();
extern void PathOpsDebugDeleteNameStr(void* v);
#define DEBUG_FILENAME_STRING_LENGTH 64
#define DEBUG_FILENAME_STRING \
    (reinterpret_cast<char* >(SkTLS::Get(PathOpsDebugCreateNameStr, PathOpsDebugDeleteNameStr)))
extern void DebugBumpTestName(char* );
extern void DebugShowPath(const SkPath& one, const SkPath& two, SkPathOp op, const char* name);
#endif

#ifndef DEBUG_TEST
#define DEBUG_TEST 0
#endif

#endif
