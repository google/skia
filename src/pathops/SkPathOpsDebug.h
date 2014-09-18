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
#include <stdio.h>

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

#define WIND_AS_STRING(x) char x##Str[12]; \
        if (!SkPathOpsDebug::ValidWind(x)) strcpy(x##Str, "?"); \
        else SK_SNPRINTF(x##Str, sizeof(x##Str), "%d", x)

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
#define DEBUG_CHECK_ENDS 0
#define DEBUG_CHECK_TINY 0
#define DEBUG_CONCIDENT 0
#define DEBUG_CROSS 0
#define DEBUG_CUBIC_BINARY_SEARCH 0
#define DEBUG_DUPLICATES 0
#define DEBUG_FLAT_QUADS 0
#define DEBUG_FLOW 0
#define DEBUG_LIMIT_WIND_SUM 0
#define DEBUG_MARK_DONE 0
#define DEBUG_PATH_CONSTRUCTION 0
#define DEBUG_SHOW_TEST_NAME 0
#define DEBUG_SHOW_TEST_PROGRESS 0
#define DEBUG_SHOW_WINDING 0
#define DEBUG_SORT 0
#define DEBUG_SORT_COMPACT 0
#define DEBUG_SORT_RAW 0
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
#define DEBUG_CHECK_ENDS 1
#define DEBUG_CHECK_TINY 1
#define DEBUG_CONCIDENT 1
#define DEBUG_CROSS 01
#define DEBUG_CUBIC_BINARY_SEARCH 0
#define DEBUG_DUPLICATES 1
#define DEBUG_FLAT_QUADS 0
#define DEBUG_FLOW 1
#define DEBUG_LIMIT_WIND_SUM 4
#define DEBUG_MARK_DONE 1
#define DEBUG_PATH_CONSTRUCTION 1
#define DEBUG_SHOW_TEST_NAME 1
#define DEBUG_SHOW_TEST_PROGRESS 1
#define DEBUG_SHOW_WINDING 0
#define DEBUG_SORT 1
#define DEBUG_SORT_COMPACT 0
#define DEBUG_SORT_RAW 0
#define DEBUG_SORT_SINGLE 0
#define DEBUG_SWAP_TOP 1
#define DEBUG_UNSORTABLE 1
#define DEBUG_VALIDATE 0
#define DEBUG_WIND_BUMP 0
#define DEBUG_WINDING 1
#define DEBUG_WINDING_AT_T 1

#endif

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
#define PT_DEBUG_DATA(i, n) i.pt(n).asSkPoint().fX, i.pt(n).asSkPoint().fY

#ifndef DEBUG_TEST
#define DEBUG_TEST 0
#endif

#if DEBUG_SHOW_TEST_NAME
#include "SkTLS.h"
#endif

#include "SkTArray.h"
#include "SkTDArray.h"

class SkPathOpsDebug {
public:
    static const char* kLVerbStr[];

#if defined(SK_DEBUG) || !FORCE_RELEASE
    static int gContourID;
    static int gSegmentID;
#endif

#if DEBUG_SORT || DEBUG_SWAP_TOP
    static int gSortCountDefault;
    static int gSortCount;
#endif

#if DEBUG_ACTIVE_OP
    static const char* kPathOpStr[];
#endif

    static bool ChaseContains(const SkTDArray<struct SkOpSpan *>& , const struct SkOpSpan * );
    static void MathematicaIze(char* str, size_t bufferSize);
    static bool ValidWind(int winding);
    static void WindingPrintf(int winding);

#if DEBUG_SHOW_TEST_NAME
    static void* CreateNameStr();
    static void DeleteNameStr(void* v);
#define DEBUG_FILENAME_STRING_LENGTH 64
#define DEBUG_FILENAME_STRING (reinterpret_cast<char* >(SkTLS::Get(SkPathOpsDebug::CreateNameStr, \
        SkPathOpsDebug::DeleteNameStr)))
    static void BumpTestName(char* );
#endif
    static void ShowOnePath(const SkPath& path, const char* name, bool includeDeclaration);
    static void ShowPath(const SkPath& one, const SkPath& two, SkPathOp op, const char* name);
    static void DumpCoincidence(const SkTArray<class SkOpContour, true>& contours);
    static void DumpCoincidence(const SkTArray<class SkOpContour* , true>& contours);
    static void DumpContours(const SkTArray<class SkOpContour, true>& contours);
    static void DumpContours(const SkTArray<class SkOpContour* , true>& contours);
    static void DumpContourAngles(const SkTArray<class SkOpContour, true>& contours);
    static void DumpContourAngles(const SkTArray<class SkOpContour* , true>& contours);
    static void DumpContourPt(const SkTArray<class SkOpContour, true>& contours, int id);
    static void DumpContourPt(const SkTArray<class SkOpContour* , true>& contours, int id);
    static void DumpContourPts(const SkTArray<class SkOpContour, true>& contours);
    static void DumpContourPts(const SkTArray<class SkOpContour* , true>& contours);
    static void DumpContourSpan(const SkTArray<class SkOpContour, true>& contours, int id);
    static void DumpContourSpan(const SkTArray<class SkOpContour* , true>& contours, int id);
    static void DumpContourSpans(const SkTArray<class SkOpContour, true>& contours);
    static void DumpContourSpans(const SkTArray<class SkOpContour* , true>& contours);
    static void DumpSpans(const SkTDArray<struct SkOpSpan *>& );
    static void DumpSpans(const SkTDArray<struct SkOpSpan *>* );
};

// shorthand for calling from debugger
void Dump(const SkTArray<class SkOpContour, true>& contours);
void Dump(const SkTArray<class SkOpContour* , true>& contours);
void Dump(const SkTArray<class SkOpContour, true>* contours);
void Dump(const SkTArray<class SkOpContour* , true>* contours);

void Dump(const SkTDArray<SkOpSpan* >& chase);
void Dump(const SkTDArray<SkOpSpan* >* chase);

void DumpAngles(const SkTArray<class SkOpContour, true>& contours);
void DumpAngles(const SkTArray<class SkOpContour* , true>& contours);
void DumpAngles(const SkTArray<class SkOpContour, true>* contours);
void DumpAngles(const SkTArray<class SkOpContour* , true>* contours);

void DumpCoin(const SkTArray<class SkOpContour, true>& contours);
void DumpCoin(const SkTArray<class SkOpContour* , true>& contours);
void DumpCoin(const SkTArray<class SkOpContour, true>* contours);
void DumpCoin(const SkTArray<class SkOpContour* , true>* contours);

void DumpPts(const SkTArray<class SkOpContour, true>& contours);
void DumpPts(const SkTArray<class SkOpContour* , true>& contours);
void DumpPts(const SkTArray<class SkOpContour, true>* contours);
void DumpPts(const SkTArray<class SkOpContour* , true>* contours);

void DumpPt(const SkTArray<class SkOpContour, true>& contours, int segmentID);
void DumpPt(const SkTArray<class SkOpContour* , true>& contours, int segmentID);
void DumpPt(const SkTArray<class SkOpContour, true>* contours, int segmentID);
void DumpPt(const SkTArray<class SkOpContour* , true>* contours, int segmentID);

void DumpSpans(const SkTArray<class SkOpContour, true>& contours);
void DumpSpans(const SkTArray<class SkOpContour* , true>& contours);
void DumpSpans(const SkTArray<class SkOpContour, true>* contours);
void DumpSpans(const SkTArray<class SkOpContour* , true>* contours);

void DumpSpan(const SkTArray<class SkOpContour, true>& contours, int segmentID);
void DumpSpan(const SkTArray<class SkOpContour* , true>& contours, int segmentID);
void DumpSpan(const SkTArray<class SkOpContour, true>* contours, int segmentID);
void DumpSpan(const SkTArray<class SkOpContour* , true>* contours, int segmentID);

// generates tools/path_sorter.htm and path_visualizer.htm compatible data
void DumpQ(const struct SkDQuad& quad1, const struct SkDQuad& quad2, int testNo);

void DumpT(const struct SkDQuad& quad, double t);

#endif
