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

#include <stdlib.h>
#include <stdio.h>

enum class SkOpPhase : char;
class SkOpContourHead;

#ifdef SK_RELEASE
#define FORCE_RELEASE 1
#else
#define FORCE_RELEASE 1  // set force release to 1 for multiple thread -- no debugging
#endif

#define DEBUG_UNDER_DEVELOPMENT 0

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
#define DEBUG_ADD_INTERSECTING_TS 0
#define DEBUG_ADD_T 0
#define DEBUG_ALIGNMENT 0
#define DEBUG_ANGLE 0
#define DEBUG_ASSEMBLE 0
#define DEBUG_COINCIDENCE 0  // sanity checking
#define DEBUG_COINCIDENCE_DUMP 0  // accumulate and dump which algorithms fired
#define DEBUG_COINCIDENCE_ORDER 0  // for well behaved curves, check if pairs match up in t-order
#define DEBUG_COINCIDENCE_VERBOSE 0  // usually whether the next function generates coincidence
#define DEBUG_CUBIC_BINARY_SEARCH 0
#define DEBUG_CUBIC_SPLIT 0
#define DEBUG_DUMP_SEGMENTS 0
#define DEBUG_DUMP_VERIFY 0
#define DEBUG_FLOW 0
#define DEBUG_LIMIT_WIND_SUM 0
#define DEBUG_MARK_DONE 0
#define DEBUG_PATH_CONSTRUCTION 0
#define DEBUG_PERP 0
#define DEBUG_SHOW_TEST_NAME 0
#define DEBUG_SORT 0
#define DEBUG_T_SECT 0
#define DEBUG_T_SECT_DUMP 0
#define DEBUG_T_SECT_LOOP_COUNT 0
#define DEBUG_VALIDATE 0
#define DEBUG_WINDING 0
#define DEBUG_WINDING_AT_T 0

#else

#define DEBUG_ACTIVE_OP 1
#define DEBUG_ACTIVE_SPANS 1
#define DEBUG_ADD_INTERSECTING_TS 1
#define DEBUG_ADD_T 1
#define DEBUG_ALIGNMENT 0
#define DEBUG_ANGLE 1
#define DEBUG_ASSEMBLE 1
#define DEBUG_COINCIDENCE 1
#define DEBUG_COINCIDENCE_DUMP 0
#define DEBUG_COINCIDENCE_ORDER 0  // tight arc quads may generate out-of-order coincdence spans
#define DEBUG_COINCIDENCE_VERBOSE 1
#define DEBUG_CUBIC_BINARY_SEARCH 0
#define DEBUG_CUBIC_SPLIT 1
#define DEBUG_DUMP_VERIFY 0
#define DEBUG_DUMP_SEGMENTS 1
#define DEBUG_FLOW 1
#define DEBUG_LIMIT_WIND_SUM 15
#define DEBUG_MARK_DONE 1
#define DEBUG_PATH_CONSTRUCTION 1
#define DEBUG_PERP 1
#define DEBUG_SHOW_TEST_NAME 1
#define DEBUG_SORT 1
#define DEBUG_T_SECT 0
#define DEBUG_T_SECT_DUMP 0  // Use 1 normally. Use 2 to number segments, 3 for script output
#define DEBUG_T_SECT_LOOP_COUNT 0
#define DEBUG_VALIDATE 1
#define DEBUG_WINDING 1
#define DEBUG_WINDING_AT_T 1

#endif

#ifdef SK_RELEASE
    #define SkDEBUGRELEASE(a, b) b
    #define SkDEBUGPARAMS(...)
#else
    #define SkDEBUGRELEASE(a, b) a
    #define SkDEBUGPARAMS(...) , __VA_ARGS__
#endif

#if DEBUG_VALIDATE == 0
    #define PATH_OPS_DEBUG_VALIDATE_PARAMS(...)
#else
    #define PATH_OPS_DEBUG_VALIDATE_PARAMS(...) , __VA_ARGS__
#endif

#if DEBUG_T_SECT == 0
    #define PATH_OPS_DEBUG_T_SECT_RELEASE(a, b) b
    #define PATH_OPS_DEBUG_T_SECT_PARAMS(...)
    #define PATH_OPS_DEBUG_T_SECT_CODE(...)
#else
    #define PATH_OPS_DEBUG_T_SECT_RELEASE(a, b) a
    #define PATH_OPS_DEBUG_T_SECT_PARAMS(...) , __VA_ARGS__
    #define PATH_OPS_DEBUG_T_SECT_CODE(...) __VA_ARGS__
#endif

#if DEBUG_T_SECT_DUMP > 1
    extern int gDumpTSectNum;
#endif

#if DEBUG_COINCIDENCE || DEBUG_COINCIDENCE_DUMP 
    #define DEBUG_COIN 1
#else
    #define DEBUG_COIN 0
#endif

#if DEBUG_COIN
    #define DEBUG_COIN_DECLARE_ONLY_PARAMS() \
            int lineNo, SkOpPhase phase, int iteration
    #define DEBUG_COIN_DECLARE_PARAMS() \
            , DEBUG_COIN_DECLARE_ONLY_PARAMS()
    #define DEBUG_COIN_ONLY_PARAMS() \
            __LINE__, SkOpPhase::kNoChange, 0
    #define DEBUG_COIN_PARAMS() \
            , DEBUG_COIN_ONLY_PARAMS()
    #define DEBUG_ITER_ONLY_PARAMS(iteration) \
            __LINE__, SkOpPhase::kNoChange, iteration
    #define DEBUG_ITER_PARAMS(iteration) \
            , DEBUG_ITER_ONLY_PARAMS(iteration)
    #define DEBUG_PHASE_ONLY_PARAMS(phase) \
            __LINE__, SkOpPhase::phase, 0
    #define DEBUG_PHASE_PARAMS(phase) \
            , DEBUG_PHASE_ONLY_PARAMS(phase)
    #define DEBUG_SET_PHASE() \
            this->globalState()->debugSetPhase(__func__, lineNo, phase, iteration)
    #define DEBUG_STATIC_SET_PHASE(obj) \
            obj->globalState()->debugSetPhase(__func__, lineNo, phase, iteration)
#elif DEBUG_VALIDATE
    #define DEBUG_COIN_DECLARE_ONLY_PARAMS() \
            SkOpPhase phase
    #define DEBUG_COIN_DECLARE_PARAMS() \
            , DEBUG_COIN_DECLARE_ONLY_PARAMS()
    #define DEBUG_COIN_ONLY_PARAMS() \
            SkOpPhase::kNoChange
    #define DEBUG_COIN_PARAMS() \
            , DEBUG_COIN_ONLY_PARAMS()
    #define DEBUG_ITER_ONLY_PARAMS(iteration) \
            SkOpPhase::kNoChange
    #define DEBUG_ITER_PARAMS(iteration) \
            , DEBUG_ITER_ONLY_PARAMS(iteration)
    #define DEBUG_PHASE_ONLY_PARAMS(phase) \
            SkOpPhase::phase
    #define DEBUG_PHASE_PARAMS(phase) \
            , DEBUG_PHASE_ONLY_PARAMS(phase)
    #define DEBUG_SET_PHASE() \
            this->globalState()->debugSetPhase(phase)
    #define DEBUG_STATIC_SET_PHASE(obj) \
            obj->globalState()->debugSetPhase(phase)
#else
    #define DEBUG_COIN_DECLARE_ONLY_PARAMS()
    #define DEBUG_COIN_DECLARE_PARAMS()
    #define DEBUG_COIN_ONLY_PARAMS()
    #define DEBUG_COIN_PARAMS()
    #define DEBUG_ITER_ONLY_PARAMS(iteration)
    #define DEBUG_ITER_PARAMS(iteration)
    #define DEBUG_PHASE_ONLY_PARAMS(phase)
    #define DEBUG_PHASE_PARAMS(phase)
    #define DEBUG_SET_PHASE()
    #define DEBUG_STATIC_SET_PHASE(obj)
#endif

#define CUBIC_DEBUG_STR  "{{{%1.9g,%1.9g}, {%1.9g,%1.9g}, {%1.9g,%1.9g}, {%1.9g,%1.9g}}}"
#define CONIC_DEBUG_STR "{{{{%1.9g,%1.9g}, {%1.9g,%1.9g}, {%1.9g,%1.9g}}}, %1.9g}"
#define QUAD_DEBUG_STR   "{{{%1.9g,%1.9g}, {%1.9g,%1.9g}, {%1.9g,%1.9g}}}"
#define LINE_DEBUG_STR   "{{{%1.9g,%1.9g}, {%1.9g,%1.9g}}}"
#define PT_DEBUG_STR "{{%1.9g,%1.9g}}"

#define T_DEBUG_STR(t, n) #t "[" #n "]=%1.9g"
#define TX_DEBUG_STR(t) #t "[%d]=%1.9g"
#define CUBIC_DEBUG_DATA(c) c[0].fX, c[0].fY, c[1].fX, c[1].fY, c[2].fX, c[2].fY, c[3].fX, c[3].fY
#define CONIC_DEBUG_DATA(c, w) c[0].fX, c[0].fY, c[1].fX, c[1].fY, c[2].fX, c[2].fY, w
#define QUAD_DEBUG_DATA(q)  q[0].fX, q[0].fY, q[1].fX, q[1].fY, q[2].fX, q[2].fY
#define LINE_DEBUG_DATA(l)  l[0].fX, l[0].fY, l[1].fX, l[1].fY
#define PT_DEBUG_DATA(i, n) i.pt(n).asSkPoint().fX, i.pt(n).asSkPoint().fY

#ifndef DEBUG_TEST
#define DEBUG_TEST 0
#endif

#if DEBUG_SHOW_TEST_NAME
#include "SkTLS.h"
#endif

// Tests with extreme numbers may fail, but all other tests should never fail.
#define FAIL_IF(cond) \
        do { bool fail = (cond); SkOPASSERT(!fail); if (fail) return false; } while (false)

#define FAIL_WITH_NULL_IF(cond) \
        do { bool fail = (cond); SkOPASSERT(!fail); if (fail) return nullptr; } while (false)

// Some functions serve two masters: one allows the function to fail, the other expects success
// always. If abort is true, tests with normal numbers may not fail and assert if they do so.
// If abort is false, both normal and extreme numbers may return false without asserting.
#define RETURN_FALSE_IF(abort, cond) \
        do { bool fail = (cond); SkOPASSERT(!(abort) || !fail); if (fail) return false; \
        } while (false)

class SkPathOpsDebug {
public:
    static const char* kLVerbStr[];

#if DEBUG_COIN
    struct GlitchLog;

    enum GlitchType {
        kUninitialized_Glitch,
        kAddCorruptCoin_Glitch,
        kAddExpandedCoin_Glitch,
        kAddExpandedFail_Glitch,
        kAddIfCollapsed_Glitch,
        kAddIfMissingCoin_Glitch,
        kAddMissingCoin_Glitch,
        kAddMissingExtend_Glitch,
        kAddOrOverlap_Glitch,
        kCollapsedCoin_Glitch,
        kCollapsedDone_Glitch,
        kCollapsedOppValue_Glitch,
        kCollapsedSpan_Glitch,
        kCollapsedWindValue_Glitch,
        kCorrectEnd_Glitch,
        kDeletedCoin_Glitch,
        kExpandCoin_Glitch,
        kFail_Glitch,
        kMarkCoinEnd_Glitch,
        kMarkCoinInsert_Glitch,
        kMarkCoinMissing_Glitch,
        kMarkCoinStart_Glitch,
        kMergeMatches_Glitch,
        kMissingCoin_Glitch,
        kMissingDone_Glitch,
        kMissingIntersection_Glitch,
        kMoveMultiple_Glitch,
        kMoveNearbyClearAll_Glitch,
        kMoveNearbyClearAll2_Glitch,
        kMoveNearbyMerge_Glitch,
        kMoveNearbyMergeFinal_Glitch,
        kMoveNearbyRelease_Glitch,
        kMoveNearbyReleaseFinal_Glitch,
        kReleasedSpan_Glitch,
        kReturnFalse_Glitch,
        kUnaligned_Glitch,
        kUnalignedHead_Glitch,
        kUnalignedTail_Glitch,
    };

    struct CoinDictEntry {
        int fIteration;
        int fLineNumber;
        GlitchType fGlitchType;
        const char* fFunctionName;
    };

    struct CoinDict {
        void add(const CoinDictEntry& key);
        void add(const CoinDict& dict);
        void dump(const char* str, bool visitCheck) const;
        SkTDArray<CoinDictEntry> fDict;
    };

    static CoinDict gCoinSumChangedDict;
    static CoinDict gCoinSumVisitedDict;
    static CoinDict gCoinVistedDict;
#endif

#if defined(SK_DEBUG) || !FORCE_RELEASE
    static int gContourID;
    static int gSegmentID;
#endif

#if DEBUG_SORT
    static int gSortCountDefault;
    static int gSortCount;
#endif

#if DEBUG_ACTIVE_OP
    static const char* kPathOpStr[];
#endif

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
    static const char* OpStr(SkPathOp );
    static void ShowActiveSpans(SkOpContourHead* contourList);
    static void ShowOnePath(const SkPath& path, const char* name, bool includeDeclaration);
    static void ShowPath(const SkPath& one, const SkPath& two, SkPathOp op, const char* name);

    static bool ChaseContains(const SkTDArray<class SkOpSpanBase*>& , const class SkOpSpanBase* );

    static void CheckHealth(class SkOpContourHead* contourList);

    static const class SkOpAngle* DebugAngleAngle(const class SkOpAngle*, int id);
    static class SkOpContour* DebugAngleContour(class SkOpAngle*, int id);
    static const class SkOpPtT* DebugAnglePtT(const class SkOpAngle*, int id);
    static const class SkOpSegment* DebugAngleSegment(const class SkOpAngle*, int id);
    static const class SkOpSpanBase* DebugAngleSpan(const class SkOpAngle*, int id);

    static const class SkOpAngle* DebugContourAngle(class SkOpContour*, int id);
    static class SkOpContour* DebugContourContour(class SkOpContour*, int id);
    static const class SkOpPtT* DebugContourPtT(class SkOpContour*, int id);
    static const class SkOpSegment* DebugContourSegment(class SkOpContour*, int id);
    static const class SkOpSpanBase* DebugContourSpan(class SkOpContour*, int id);

    static const class SkOpAngle* DebugCoincidenceAngle(class SkOpCoincidence*, int id);
    static class SkOpContour* DebugCoincidenceContour(class SkOpCoincidence*, int id);
    static const class SkOpPtT* DebugCoincidencePtT(class SkOpCoincidence*, int id);
    static const class SkOpSegment* DebugCoincidenceSegment(class SkOpCoincidence*, int id);
    static const class SkOpSpanBase* DebugCoincidenceSpan(class SkOpCoincidence*, int id);

    static const class SkOpAngle* DebugPtTAngle(const class SkOpPtT*, int id);
    static class SkOpContour* DebugPtTContour(class SkOpPtT*, int id);
    static const class SkOpPtT* DebugPtTPtT(const class SkOpPtT*, int id);
    static const class SkOpSegment* DebugPtTSegment(const class SkOpPtT*, int id);
    static const class SkOpSpanBase* DebugPtTSpan(const class SkOpPtT*, int id);

    static const class SkOpAngle* DebugSegmentAngle(const class SkOpSegment*, int id);
    static class SkOpContour* DebugSegmentContour(class SkOpSegment*, int id);
    static const class SkOpPtT* DebugSegmentPtT(const class SkOpSegment*, int id);
    static const class SkOpSegment* DebugSegmentSegment(const class SkOpSegment*, int id);
    static const class SkOpSpanBase* DebugSegmentSpan(const class SkOpSegment*, int id);

    static const class SkOpAngle* DebugSpanAngle(const class SkOpSpanBase*, int id);
    static class SkOpContour* DebugSpanContour(class SkOpSpanBase*, int id);
    static const class SkOpPtT* DebugSpanPtT(const class SkOpSpanBase*, int id);
    static const class SkOpSegment* DebugSpanSegment(const class SkOpSpanBase*, int id);
    static const class SkOpSpanBase* DebugSpanSpan(const class SkOpSpanBase*, int id);

#if DEBUG_COIN
    static void DumpCoinDict();
    static void DumpGlitchType(GlitchType );
#endif

    static bool gRunFail;
    static bool gVeryVerbose;

#if DEBUG_DUMP_VERIFY
    static bool gDumpOp;
    static bool gVerifyOp;

    static void DumpOp(const SkPath& one, const SkPath& two, SkPathOp op,
            const char* testName);
    static void DumpOp(FILE* file, const SkPath& one, const SkPath& two, SkPathOp op,
            const char* testName);
    static void DumpSimplify(const SkPath& path, const char* testName);
    static void DumpSimplify(FILE* file, const SkPath& path, const char* testName);
    static void ReportOpFail(const SkPath& one, const SkPath& two, SkPathOp op);
    static void ReportSimplifyFail(const SkPath& path);
    static void VerifyOp(const SkPath& one, const SkPath& two, SkPathOp op,
        const SkPath& result);
    static void VerifySimplify(const SkPath& path, const SkPath& result);
#endif

#if DEBUG_ACTIVE_SPANS
    static SkString gActiveSpans;
#endif

};

struct SkDQuad;

// generates tools/path_sorter.htm and path_visualizer.htm compatible data
void DumpQ(const SkDQuad& quad1, const SkDQuad& quad2, int testNo);
void DumpT(const SkDQuad& quad, double t);

#endif
