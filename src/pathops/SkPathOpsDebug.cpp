/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkMutex.h"
#include "SkOpCoincidence.h"
#include "SkOpContour.h"
#include "SkPath.h"
#include "SkPathOpsDebug.h"
#include "SkString.h"

#undef FAIL_IF
#define FAIL_IF(cond, coin) \
         do { if (cond) log->record(kAddExpandedFail_Glitch, id, coin); } while (false)

#undef FAIL_WITH_NULL_IF
#define FAIL_WITH_NULL_IF(cond, span) \
         do { if (cond) log->record(kAddExpandedFail_Glitch, id, span); } while (false)

#undef RETURN_FALSE_IF
#define RETURN_FALSE_IF(cond, span) \
         do { if (cond) log->record(kAddExpandedFail_Glitch, id, span); } while (false)

class SkCoincidentSpans;

#if DEBUG_VALIDATE
extern bool FLAGS_runFail;
#endif

#if DEBUG_SORT
int SkPathOpsDebug::gSortCountDefault = SK_MaxS32;
int SkPathOpsDebug::gSortCount;
#endif

#if DEBUG_ACTIVE_OP
const char* SkPathOpsDebug::kPathOpStr[] = {"diff", "sect", "union", "xor"};
#endif

#if defined SK_DEBUG || !FORCE_RELEASE

const char* SkPathOpsDebug::kLVerbStr[] = {"", "line", "quad", "cubic"};

int SkPathOpsDebug::gContourID = 0;
int SkPathOpsDebug::gSegmentID = 0;

bool SkPathOpsDebug::ChaseContains(const SkTDArray<SkOpSpanBase* >& chaseArray,
        const SkOpSpanBase* span) {
    for (int index = 0; index < chaseArray.count(); ++index) {
        const SkOpSpanBase* entry = chaseArray[index];
        if (entry == span) {
            return true;
        }
    }
    return false;
}
#endif

#if DEBUG_COINCIDENCE_VERBOSE
enum GlitchType {
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
    kDeletedCoin_Glitch,
    kExpandCoin_Glitch,
    kMarkCoinEnd_Glitch,
    kMarkCoinInsert_Glitch,
    kMarkCoinMissing_Glitch,
    kMarkCoinStart_Glitch,
    kMergeContained_Glitch,
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
    kUnaligned_Glitch,
    kUnalignedHead_Glitch,
    kUnalignedTail_Glitch,
};

static const int kGlitchType_Count = kUnalignedTail_Glitch + 1;

struct SpanGlitch {
    const char* fStage;
    const SkOpSpanBase* fBase;
    const SkOpSpanBase* fSuspect;
    const SkOpSegment* fSegment;
    const SkOpSegment* fOppSegment;
    const SkOpPtT* fCoinSpan;
    const SkOpPtT* fEndSpan;
    const SkOpPtT* fOppSpan;
    const SkOpPtT* fOppEndSpan;
    double fStartT;
    double fEndT;
    double fOppStartT;
    double fOppEndT;
    SkPoint fPt;
    GlitchType fType;
};

struct SkPathOpsDebug::GlitchLog {
    SpanGlitch* recordCommon(GlitchType type, const char* stage) {
        SpanGlitch* glitch = fGlitches.push();
        glitch->fStage = stage;
        glitch->fBase = nullptr;
        glitch->fSuspect = nullptr;
        glitch->fSegment = nullptr;
        glitch->fOppSegment = nullptr;
        glitch->fCoinSpan = nullptr;
        glitch->fEndSpan = nullptr;
        glitch->fOppSpan = nullptr;
        glitch->fOppEndSpan = nullptr;
        glitch->fStartT = SK_ScalarNaN;
        glitch->fEndT = SK_ScalarNaN;
        glitch->fOppStartT = SK_ScalarNaN;
        glitch->fOppEndT = SK_ScalarNaN;
        glitch->fPt = { SK_ScalarNaN, SK_ScalarNaN };
        glitch->fType = type;
        return glitch;
    }

    void record(GlitchType type, const char* stage, const SkOpSpanBase* base,
            const SkOpSpanBase* suspect = NULL) {
        SpanGlitch* glitch = recordCommon(type, stage);
        glitch->fBase = base;
        glitch->fSuspect = suspect;
    }

    void record(GlitchType type, const char* stage, const SkOpSpanBase* base,
            const SkOpPtT* ptT) {
        SpanGlitch* glitch = recordCommon(type, stage);
        glitch->fBase = base;
        glitch->fCoinSpan = ptT;
    }

    void record(GlitchType type, const char* stage, const SkCoincidentSpans* coin,
            const SkCoincidentSpans* opp = NULL) {
        SpanGlitch* glitch = recordCommon(type, stage);
        glitch->fCoinSpan = coin->coinPtTStart();
        glitch->fEndSpan = coin->coinPtTEnd();
        if (opp) {
            glitch->fOppSpan = opp->coinPtTStart();
            glitch->fOppEndSpan = opp->coinPtTEnd();
        }
    }

    void record(GlitchType type, const char* stage, const SkOpSpanBase* base,
            const SkOpSegment* seg, double t, SkPoint pt) {
        SpanGlitch* glitch = recordCommon(type, stage);
        glitch->fBase = base;
        glitch->fSegment = seg;
        glitch->fStartT = t;
        glitch->fPt = pt;
    }

    void record(GlitchType type, const char* stage, const SkOpSpanBase* base, double t,
            SkPoint pt) {
        SpanGlitch* glitch = recordCommon(type, stage);
        glitch->fBase = base;
        glitch->fStartT = t;
        glitch->fPt = pt;
    }

    void record(GlitchType type, const char* stage, const SkCoincidentSpans* coin,
            const SkOpPtT* coinSpan, const SkOpPtT* endSpan) {
        SpanGlitch* glitch = recordCommon(type, stage);
        glitch->fCoinSpan = coin->coinPtTStart();
        glitch->fEndSpan = coin->coinPtTEnd();
        glitch->fEndSpan = endSpan;
        glitch->fOppSpan = coinSpan;
        glitch->fOppEndSpan = endSpan;
    }

    void record(GlitchType type, const char* stage, const SkCoincidentSpans* coin,
            const SkOpSpanBase* base) {
        SpanGlitch* glitch = recordCommon(type, stage);
        glitch->fBase = base;
        glitch->fCoinSpan = coin->coinPtTStart();
        glitch->fEndSpan = coin->coinPtTEnd();
    }

    void record(GlitchType type, const char* stage, const SkOpPtT* ptTS, const SkOpPtT* ptTE,
            const SkOpPtT* oPtTS, const SkOpPtT* oPtTE) {
        SpanGlitch* glitch = recordCommon(type, stage);
        glitch->fCoinSpan = ptTS;
        glitch->fEndSpan = ptTE;
        glitch->fOppSpan = oPtTS;
        glitch->fOppEndSpan = oPtTE;
    }

    void record(GlitchType type, const char* stage, const SkOpSegment* seg, double startT,
            double endT, const SkOpSegment* oppSeg, double oppStartT, double oppEndT) {
        SpanGlitch* glitch = recordCommon(type, stage);
        glitch->fSegment = seg;
        glitch->fStartT = startT;
        glitch->fEndT = endT;
        glitch->fOppSegment = oppSeg;
        glitch->fOppStartT = oppStartT;
        glitch->fOppEndT = oppEndT;
    }

    void record(GlitchType type, const char* stage, const SkOpSegment* seg,
            const SkOpSpan* span) {
        SpanGlitch* glitch = recordCommon(type, stage);
        glitch->fSegment = seg;
        glitch->fBase = span;
    }

    void record(GlitchType type, const char* stage, double t, const SkOpSpanBase* span) {
        SpanGlitch* glitch = recordCommon(type, stage);
        glitch->fStartT = t;
        glitch->fBase = span;
    }

    void record(GlitchType type, const char* stage, const SkOpSegment* seg) {
        SpanGlitch* glitch = recordCommon(type, stage);
        glitch->fSegment = seg;
    }

    void record(GlitchType type, const char* stage, const SkCoincidentSpans* coin,
            const SkOpPtT* ptT) {
        SpanGlitch* glitch = recordCommon(type, stage);
        glitch->fCoinSpan = coin->coinPtTStart();
        glitch->fEndSpan = ptT;
    }

    SkTDArray<SpanGlitch> fGlitches;
};
#endif

void SkPathOpsDebug::ShowActiveSpans(SkOpContourHead* contourList) {
#if DEBUG_ACTIVE_SPANS
    SkOpContour* contour = contourList;
    do {
        contour->debugShowActiveSpans();
    } while ((contour = contour->next()));
#endif
}

#if DEBUG_COINCIDENCE
void SkPathOpsDebug::CheckHealth(SkOpContourHead* contourList, const char* id) {
    contourList->globalState()->debugSetCheckHealth(true);
#if DEBUG_COINCIDENCE_VERBOSE
    GlitchLog glitches;
    const SkOpContour* contour = contourList;
    const SkOpCoincidence* coincidence = contour->globalState()->coincidence();
    coincidence->debugCheckValid(id, &glitches); // don't call validate; spans may be inconsistent
    do {
        contour->debugCheckHealth(id, &glitches);
        contour->debugMissingCoincidence(id, &glitches);
    } while ((contour = contour->next()));
    coincidence->debugRemoveCollapsed(id, &glitches);
    bool added;
    coincidence->debugAddMissing(id, &glitches, &added);
    coincidence->debugExpand(id, &glitches);
    coincidence->debugAddExpanded(id, &glitches);
    coincidence->debugMark(id, &glitches);
    coincidence->debugReorder(id, &glitches);
    unsigned mask = 0;
    for (int index = 0; index < glitches.fGlitches.count(); ++index) {
        const SpanGlitch& glitch = glitches.fGlitches[index];
        mask |= 1 << glitch.fType;
    }
    for (int index = 0; index < kGlitchType_Count; ++index) {
        SkDebugf(mask & (1 << index) ? "x" : "-");
    }
    SkDebugf("  %s\n", id);
    for (int index = 0; index < glitches.fGlitches.count(); ++index) {
        const SpanGlitch& glitch = glitches.fGlitches[index];
        SkDebugf("%02d: ", index);
        if (glitch.fBase) {
            SkDebugf(" seg/base=%d/%d", glitch.fBase->segment()->debugID(),
                    glitch.fBase->debugID());
        }
        if (glitch.fSuspect) {
            SkDebugf(" seg/base=%d/%d", glitch.fSuspect->segment()->debugID(),
                    glitch.fSuspect->debugID());
        }
        if (glitch.fSegment) {
            SkDebugf(" segment=%d", glitch.fSegment->debugID());
        }
        if (glitch.fCoinSpan) {
            SkDebugf(" coinSeg/Span/PtT=%d/%d/%d", glitch.fCoinSpan->segment()->debugID(),
                    glitch.fCoinSpan->span()->debugID(), glitch.fCoinSpan->debugID());
        }
        if (glitch.fEndSpan) {
            SkDebugf(" endSpan=%d", glitch.fEndSpan->debugID());
        }
        if (glitch.fOppSpan) {
            SkDebugf(" oppSeg/Span/PtT=%d/%d/%d", glitch.fOppSpan->segment()->debugID(),
                    glitch.fOppSpan->span()->debugID(), glitch.fOppSpan->debugID());
        }
        if (glitch.fOppEndSpan) {
            SkDebugf(" oppEndSpan=%d", glitch.fOppEndSpan->debugID());
        }
        if (!SkScalarIsNaN(glitch.fStartT)) {
            SkDebugf(" startT=%g", glitch.fStartT);
        }
        if (!SkScalarIsNaN(glitch.fEndT)) {
            SkDebugf(" endT=%g", glitch.fEndT);
        }
        if (glitch.fOppSegment) {
            SkDebugf(" segment=%d", glitch.fOppSegment->debugID());
        }
        if (!SkScalarIsNaN(glitch.fOppStartT)) {
            SkDebugf(" oppStartT=%g", glitch.fOppStartT);
        }
        if (!SkScalarIsNaN(glitch.fOppEndT)) {
            SkDebugf(" oppEndT=%g", glitch.fOppEndT);
        }
        if (!SkScalarIsNaN(glitch.fPt.fX) || !SkScalarIsNaN(glitch.fPt.fY)) {
            SkDebugf(" pt=%g,%g", glitch.fPt.fX, glitch.fPt.fY);
        }
        switch (glitch.fType) {
            case kAddCorruptCoin_Glitch: SkDebugf(" AddCorruptCoin"); break;
            case kAddExpandedCoin_Glitch: SkDebugf(" AddExpandedCoin"); break;
            case kAddExpandedFail_Glitch: SkDebugf(" AddExpandedFail"); break;
            case kAddIfCollapsed_Glitch: SkDebugf(" AddIfCollapsed"); break;; break;
            case kAddIfMissingCoin_Glitch: SkDebugf(" AddIfMissingCoin"); break;
            case kAddMissingCoin_Glitch: SkDebugf(" AddMissingCoin"); break;
            case kAddMissingExtend_Glitch: SkDebugf(" AddMissingExtend"); break;
            case kAddOrOverlap_Glitch: SkDebugf(" AAddOrOverlap"); break;
            case kCollapsedCoin_Glitch: SkDebugf(" CollapsedCoin"); break;
            case kCollapsedDone_Glitch: SkDebugf(" CollapsedDone"); break;
            case kCollapsedOppValue_Glitch: SkDebugf(" CollapsedOppValue"); break;
            case kCollapsedSpan_Glitch: SkDebugf(" CollapsedSpan"); break;
            case kCollapsedWindValue_Glitch: SkDebugf(" CollapsedWindValue"); break;
            case kDeletedCoin_Glitch: SkDebugf(" DeletedCoin"); break;
            case kExpandCoin_Glitch: SkDebugf(" ExpandCoin"); break;
            case kMarkCoinEnd_Glitch: SkDebugf(" MarkCoinEnd"); break;
            case kMarkCoinInsert_Glitch: SkDebugf(" MarkCoinInsert"); break;
            case kMarkCoinMissing_Glitch: SkDebugf(" MarkCoinMissing"); break;
            case kMarkCoinStart_Glitch: SkDebugf(" MarkCoinStart"); break;
            case kMergeContained_Glitch: SkDebugf(" MergeContained"); break;
            case kMergeMatches_Glitch: SkDebugf(" MergeMatches"); break;
            case kMissingCoin_Glitch: SkDebugf(" MissingCoin"); break;
            case kMissingDone_Glitch: SkDebugf(" MissingDone"); break;
            case kMissingIntersection_Glitch: SkDebugf(" MissingIntersection"); break;
            case kMoveMultiple_Glitch: SkDebugf(" MoveMultiple"); break;
            case kMoveNearbyClearAll_Glitch: SkDebugf(" MoveNearbyClearAll"); break;
            case kMoveNearbyClearAll2_Glitch: SkDebugf(" MoveNearbyClearAll2"); break;
            case kMoveNearbyMerge_Glitch: SkDebugf(" MoveNearbyMerge"); break;
            case kMoveNearbyMergeFinal_Glitch: SkDebugf(" MoveNearbyMergeFinal"); break;
            case kMoveNearbyRelease_Glitch: SkDebugf(" MoveNearbyRelease"); break;
            case kMoveNearbyReleaseFinal_Glitch: SkDebugf(" MoveNearbyReleaseFinal"); break;
            case kReleasedSpan_Glitch: SkDebugf(" ReleasedSpan"); break;
            case kUnaligned_Glitch: SkDebugf(" Unaligned"); break;
            case kUnalignedHead_Glitch: SkDebugf(" UnalignedHead"); break;
            case kUnalignedTail_Glitch: SkDebugf(" UnalignedTail"); break;
            default: SkASSERT(0);
        }
        SkDebugf("\n");
    }
    contourList->globalState()->debugSetCheckHealth(false);
#if 0 && DEBUG_ACTIVE_SPANS
    SkDebugf("active after %s:\n", id);
    ShowActiveSpans(contourList);
#endif
#endif
}
#endif

#if defined SK_DEBUG || !FORCE_RELEASE
void SkPathOpsDebug::MathematicaIze(char* str, size_t bufferLen) {
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

bool SkPathOpsDebug::ValidWind(int wind) {
    return wind > SK_MinS32 + 0xFFFF && wind < SK_MaxS32 - 0xFFFF;
}

void SkPathOpsDebug::WindingPrintf(int wind) {
    if (wind == SK_MinS32) {
        SkDebugf("?");
    } else {
        SkDebugf("%d", wind);
    }
}
#endif //  defined SK_DEBUG || !FORCE_RELEASE


#if DEBUG_SHOW_TEST_NAME
void* SkPathOpsDebug::CreateNameStr() { return new char[DEBUG_FILENAME_STRING_LENGTH]; }

void SkPathOpsDebug::DeleteNameStr(void* v) { delete[] reinterpret_cast<char*>(v); }

void SkPathOpsDebug::BumpTestName(char* test) {
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

static void show_function_header(const char* functionName) {
    SkDebugf("\nstatic void %s(skiatest::Reporter* reporter, const char* filename) {\n", functionName);
    if (strcmp("skphealth_com76", functionName) == 0) {
        SkDebugf("found it\n");
    }
}

static const char* gOpStrs[] = {
    "kDifference_SkPathOp",
    "kIntersect_SkPathOp",
    "kUnion_SkPathOp",
    "kXOR_PathOp",
    "kReverseDifference_SkPathOp",
};

const char* SkPathOpsDebug::OpStr(SkPathOp op) {
    return gOpStrs[op];
}

static void show_op(SkPathOp op, const char* pathOne, const char* pathTwo) {
    SkDebugf("    testPathOp(reporter, %s, %s, %s, filename);\n", pathOne, pathTwo, gOpStrs[op]);
    SkDebugf("}\n");
}

SK_DECLARE_STATIC_MUTEX(gTestMutex);

void SkPathOpsDebug::ShowPath(const SkPath& a, const SkPath& b, SkPathOp shapeOp,
        const char* testName) {
    SkAutoMutexAcquire ac(gTestMutex);
    show_function_header(testName);
    ShowOnePath(a, "path", true);
    ShowOnePath(b, "pathB", true);
    show_op(shapeOp, "path", "pathB");
}

#include "SkPathOpsTypes.h"
#include "SkIntersectionHelper.h"
#include "SkIntersections.h"

#if DEBUG_T_SECT_LOOP_COUNT
void SkOpGlobalState::debugAddLoopCount(SkIntersections* i, const SkIntersectionHelper& wt,
        const SkIntersectionHelper& wn) {
    for (int index = 0; index < (int) SK_ARRAY_COUNT(fDebugLoopCount); ++index) {
        SkIntersections::DebugLoop looper = (SkIntersections::DebugLoop) index;
        if (fDebugLoopCount[index] >= i->debugLoopCount(looper)) {
            continue;
        }
        fDebugLoopCount[index] = i->debugLoopCount(looper);
        fDebugWorstVerb[index * 2] = wt.segment()->verb();
        fDebugWorstVerb[index * 2 + 1] = wn.segment()->verb();
        sk_bzero(&fDebugWorstPts[index * 8], sizeof(SkPoint) * 8);
        memcpy(&fDebugWorstPts[index * 2 * 4], wt.pts(),
                (SkPathOpsVerbToPoints(wt.segment()->verb()) + 1) * sizeof(SkPoint));
        memcpy(&fDebugWorstPts[(index * 2 + 1) * 4], wn.pts(),
                (SkPathOpsVerbToPoints(wn.segment()->verb()) + 1) * sizeof(SkPoint));
        fDebugWorstWeight[index * 2] = wt.weight();
        fDebugWorstWeight[index * 2 + 1] = wn.weight();
    }
    i->debugResetLoopCount();
}

void SkOpGlobalState::debugDoYourWorst(SkOpGlobalState* local) {
    for (int index = 0; index < (int) SK_ARRAY_COUNT(fDebugLoopCount); ++index) {
        if (fDebugLoopCount[index] >= local->fDebugLoopCount[index]) {
            continue;
        }
        fDebugLoopCount[index] = local->fDebugLoopCount[index];
        fDebugWorstVerb[index * 2] = local->fDebugWorstVerb[index * 2];
        fDebugWorstVerb[index * 2 + 1] = local->fDebugWorstVerb[index * 2 + 1];
        memcpy(&fDebugWorstPts[index * 2 * 4], &local->fDebugWorstPts[index * 2 * 4],
                sizeof(SkPoint) * 8);
        fDebugWorstWeight[index * 2] = local->fDebugWorstWeight[index * 2];
        fDebugWorstWeight[index * 2 + 1] = local->fDebugWorstWeight[index * 2 + 1];
    }
    local->debugResetLoopCounts();
}

static void dump_curve(SkPath::Verb verb, const SkPoint& pts, float weight) {
    if (!verb) {
        return;
    }
    const char* verbs[] = { "", "line", "quad", "conic", "cubic" };
    SkDebugf("%s: {{", verbs[verb]);
    int ptCount = SkPathOpsVerbToPoints(verb);
    for (int index = 0; index <= ptCount; ++index) {
        SkDPoint::Dump((&pts)[index]);
        if (index < ptCount - 1) {
            SkDebugf(", ");
        }
    }
    SkDebugf("}");
    if (weight != 1) {
        SkDebugf(", ");
        if (weight == floorf(weight)) {
            SkDebugf("%.0f", weight);
        } else {
            SkDebugf("%1.9gf", weight);
        }
    }
    SkDebugf("}\n");
}

void SkOpGlobalState::debugLoopReport() {
    const char* loops[] = { "iterations", "coinChecks", "perpCalcs" };
    SkDebugf("\n");
    for (int index = 0; index < (int) SK_ARRAY_COUNT(fDebugLoopCount); ++index) {
        SkDebugf("%s: %d\n", loops[index], fDebugLoopCount[index]);
        dump_curve(fDebugWorstVerb[index * 2], fDebugWorstPts[index * 2 * 4],
                fDebugWorstWeight[index * 2]);
        dump_curve(fDebugWorstVerb[index * 2 + 1], fDebugWorstPts[(index * 2 + 1) * 4],
                fDebugWorstWeight[index * 2 + 1]);
    }
}

void SkOpGlobalState::debugResetLoopCounts() {
    sk_bzero(fDebugLoopCount, sizeof(fDebugLoopCount));
    sk_bzero(fDebugWorstVerb, sizeof(fDebugWorstVerb));
    sk_bzero(fDebugWorstPts, sizeof(fDebugWorstPts));
    sk_bzero(fDebugWorstWeight, sizeof(fDebugWorstWeight));
}
#endif

#ifdef SK_DEBUG
bool SkOpGlobalState::debugRunFail() const {
#if DEBUG_VALIDATE
    return FLAGS_runFail;
#else
    return false;
#endif
}
#endif

#if DEBUG_T_SECT_LOOP_COUNT
void SkIntersections::debugBumpLoopCount(DebugLoop index) {
    fDebugLoopCount[index]++;
}

int SkIntersections::debugLoopCount(DebugLoop index) const {
    return fDebugLoopCount[index];
}

void SkIntersections::debugResetLoopCount() {
    sk_bzero(fDebugLoopCount, sizeof(fDebugLoopCount));
}
#endif

#include "SkPathOpsCubic.h"
#include "SkPathOpsQuad.h"

SkDCubic SkDQuad::debugToCubic() const {
    SkDCubic cubic;
    cubic[0] = fPts[0];
    cubic[2] = fPts[1];
    cubic[3] = fPts[2];
    cubic[1].fX = (cubic[0].fX + cubic[2].fX * 2) / 3;
    cubic[1].fY = (cubic[0].fY + cubic[2].fY * 2) / 3;
    cubic[2].fX = (cubic[3].fX + cubic[2].fX * 2) / 3;
    cubic[2].fY = (cubic[3].fY + cubic[2].fY * 2) / 3;
    return cubic;
}

void SkDRect::debugInit() {
    fLeft = fTop = fRight = fBottom = SK_ScalarNaN;
}

#include "SkOpAngle.h"
#include "SkOpSegment.h"

#if DEBUG_COINCIDENCE_VERBOSE
// commented-out lines keep this in sync with addT()
 const SkOpPtT* SkOpSegment::debugAddT(double t, const char* id, SkPathOpsDebug::GlitchLog* log) const {
    debugValidate();
    SkPoint pt = this->ptAtT(t);
    const SkOpSpanBase* span = &fHead;
    do {
        const SkOpPtT* result = span->ptT();
        if (t == result->fT || this->match(result, this, t, pt)) {
//             span->bumpSpanAdds();
             return result;
        }
        if (t < result->fT) {
            const SkOpSpan* prev = result->span()->prev();
            FAIL_WITH_NULL_IF(!prev, span);
            // marks in global state that new op span has been allocated
            this->globalState()->setAllocatedOpSpan();
//             span->init(this, prev, t, pt);
            this->debugValidate();
// #if DEBUG_ADD_T
//             SkDebugf("%s insert t=%1.9g segID=%d spanID=%d\n", __FUNCTION__, t,
//                     span->segment()->debugID(), span->debugID());
// #endif
//             span->bumpSpanAdds();
            return nullptr;
        }
        FAIL_WITH_NULL_IF(span != &fTail, span);
    } while ((span = span->upCast()->next()));
    SkASSERT(0);
    return nullptr;  // we never get here, but need this to satisfy compiler
}
#endif

#if DEBUG_ANGLE
void SkOpSegment::debugCheckAngleCoin() const {
    const SkOpSpanBase* base = &fHead;
    const SkOpSpan* span;
    do {
        const SkOpAngle* angle = base->fromAngle();
        if (angle && angle->debugCheckCoincidence()) {
            angle->debugCheckNearCoincidence();
        }
        if (base->final()) {
             break;
        }
        span = base->upCast();
        angle = span->toAngle();
        if (angle && angle->debugCheckCoincidence()) {
            angle->debugCheckNearCoincidence();
        }
    } while ((base = span->next()));
}
#endif

#if DEBUG_COINCIDENCE_VERBOSE
// this mimics the order of the checks in handle coincidence
void SkOpSegment::debugCheckHealth(const char* id, SkPathOpsDebug::GlitchLog* glitches) const {
    debugMoveMultiples(id, glitches);
    debugMoveNearby(id, glitches);
    debugMissingCoincidence(id, glitches);
}

// commented-out lines keep this in sync with clearAll()
void SkOpSegment::debugClearAll(const char* id, SkPathOpsDebug::GlitchLog* glitches) const {
    const SkOpSpan* span = &fHead;
    do {
        this->debugClearOne(span, id, glitches);
    } while ((span = span->next()->upCastable()));
    this->globalState()->coincidence()->debugRelease(id, glitches, this);
}

// commented-out lines keep this in sync with clearOne()
void SkOpSegment::debugClearOne(const SkOpSpan* span, const char* id, SkPathOpsDebug::GlitchLog* glitches) const {
    if (span->windValue()) glitches->record(kCollapsedWindValue_Glitch, id, span);
    if (span->oppValue()) glitches->record(kCollapsedOppValue_Glitch, id, span);
    if (!span->done()) glitches->record(kCollapsedDone_Glitch, id, span);
}
#endif

SkOpAngle* SkOpSegment::debugLastAngle() {
    SkOpAngle* result = nullptr;
    SkOpSpan* span = this->head();
    do {
        if (span->toAngle()) {
            SkASSERT(!result);
            result = span->toAngle();
        }
    } while ((span = span->next()->upCastable()));
    SkASSERT(result);
    return result;
}

#if DEBUG_COINCIDENCE
// commented-out lines keep this in sync with ClearVisited
void SkOpSegment::DebugClearVisited(const SkOpSpanBase* span) {
    // reset visited flag back to false
    do {
        const SkOpPtT* ptT = span->ptT(), * stopPtT = ptT;
        while ((ptT = ptT->next()) != stopPtT) {
            const SkOpSegment* opp = ptT->segment();
            opp->resetDebugVisited();
        }
    } while (!span->final() && (span = span->upCast()->next()));
}
#endif

#if DEBUG_COINCIDENCE_VERBOSE
// commented-out lines keep this in sync with missingCoincidence()
// look for pairs of undetected coincident curves
// assumes that segments going in have visited flag clear
// Even though pairs of curves correct detect coincident runs, a run may be missed
// if the coincidence is a product of multiple intersections. For instance, given
// curves A, B, and C:
// A-B intersect at a point 1; A-C and B-C intersect at point 2, so near
// the end of C that the intersection is replaced with the end of C.
// Even though A-B correctly do not detect an intersection at point 2,
// the resulting run from point 1 to point 2 is coincident on A and B.
void SkOpSegment::debugMissingCoincidence(const char* id, SkPathOpsDebug::GlitchLog* log) const {
    if (this->done()) {
        return;
    }
    const SkOpSpan* prior = nullptr;
    const SkOpSpanBase* spanBase = &fHead;
//    bool result = false;
    do {
        const SkOpPtT* ptT = spanBase->ptT(), * spanStopPtT = ptT;
        SkASSERT(ptT->span() == spanBase);
        while ((ptT = ptT->next()) != spanStopPtT) {
            if (ptT->deleted()) {
                continue;
            }
            const SkOpSegment* opp = ptT->span()->segment();
            if (opp->done()) {
                continue;
            }
            // when opp is encounted the 1st time, continue; on 2nd encounter, look for coincidence
            if (!opp->debugVisited()) {
                continue;
            }
            if (spanBase == &fHead) {
                continue;
            }
            if (ptT->segment() == this) {
                continue;
            }
            const SkOpSpan* span = spanBase->upCastable();
            // FIXME?: this assumes that if the opposite segment is coincident then no more
            // coincidence needs to be detected. This may not be true.
            if (span && span->segment() != opp && span->containsCoincidence(opp)) {  // debug has additional condition since it may be called before inner duplicate points have been deleted
                continue;
            }
            if (spanBase->segment() != opp && spanBase->containsCoinEnd(opp)) {  // debug has additional condition since it may be called before inner duplicate points have been deleted
                continue;
            }
            const SkOpPtT* priorPtT = nullptr, * priorStopPtT;
            // find prior span containing opp segment
            const SkOpSegment* priorOpp = nullptr;
            const SkOpSpan* priorTest = spanBase->prev();
            while (!priorOpp && priorTest) {
                priorStopPtT = priorPtT = priorTest->ptT();
                while ((priorPtT = priorPtT->next()) != priorStopPtT) {
                    if (priorPtT->deleted()) {
                        continue;
                    }
                    SkOpSegment* segment = priorPtT->span()->segment();
                    if (segment == opp) {
                        prior = priorTest;
                        priorOpp = opp;
                        break;
                    }
                }
                priorTest = priorTest->prev();
            }
            if (!priorOpp) {
                continue;
            }
            if (priorPtT == ptT) {
                continue;
            }
            const SkOpPtT* oppStart = prior->ptT();
            const SkOpPtT* oppEnd = spanBase->ptT();
            bool swapped = priorPtT->fT > ptT->fT;
            if (swapped) {
                SkTSwap(priorPtT, ptT);
                SkTSwap(oppStart, oppEnd);
            }
            const SkOpCoincidence* coincidence = this->globalState()->coincidence();
            const SkOpPtT* rootPriorPtT = priorPtT->span()->ptT();
            const SkOpPtT* rootPtT = ptT->span()->ptT();
            const SkOpPtT* rootOppStart = oppStart->span()->ptT();
            const SkOpPtT* rootOppEnd = oppEnd->span()->ptT();
            if (coincidence->contains(rootPriorPtT, rootPtT, rootOppStart, rootOppEnd)) {
                goto swapBack;
            }
            if (testForCoincidence(rootPriorPtT, rootPtT, prior, spanBase, opp)) {
            // mark coincidence
#if DEBUG_COINCIDENCE_VERBOSE
//                 SkDebugf("%s coinSpan=%d endSpan=%d oppSpan=%d oppEndSpan=%d\n", __FUNCTION__,
//                         rootPriorPtT->debugID(), rootPtT->debugID(), rootOppStart->debugID(),
//                         rootOppEnd->debugID());
#endif
                log->record(kMissingCoin_Glitch, id, priorPtT, ptT, oppStart, oppEnd);
                //   coincidences->add(rootPriorPtT, rootPtT, rootOppStart, rootOppEnd);
                // }
#if DEBUG_COINCIDENCE
//                SkASSERT(coincidences->contains(rootPriorPtT, rootPtT, rootOppStart, rootOppEnd);
#endif
                // result = true;
            }
    swapBack:
            if (swapped) {
                SkTSwap(priorPtT, ptT);
            }
        }
    } while ((spanBase = spanBase->final() ? nullptr : spanBase->upCast()->next()));
    DebugClearVisited(&fHead);
    return;
}

// commented-out lines keep this in sync with moveMultiples()
// if a span has more than one intersection, merge the other segments' span as needed
void SkOpSegment::debugMoveMultiples(const char* id, SkPathOpsDebug::GlitchLog* glitches) const {
    debugValidate();
    const SkOpSpanBase* test = &fHead;
    do {
        int addCount = test->spanAddsCount();
        SkASSERT(addCount >= 1);
        if (addCount == 1) {
            continue;
        }
        const SkOpPtT* startPtT = test->ptT();
        const SkOpPtT* testPtT = startPtT;
        do {  // iterate through all spans associated with start
            const SkOpSpanBase* oppSpan = testPtT->span();
            if (oppSpan->spanAddsCount() == addCount) {
                continue;
            }
            if (oppSpan->deleted()) {
                continue;
            }
            const SkOpSegment* oppSegment = oppSpan->segment();
            if (oppSegment == this) {
                continue;
            }
            // find range of spans to consider merging
            const SkOpSpanBase* oppPrev = oppSpan;
            const SkOpSpanBase* oppFirst = oppSpan;
            while ((oppPrev = oppPrev->prev())) {
                if (!roughly_equal(oppPrev->t(), oppSpan->t())) {
                    break;
                }
                if (oppPrev->spanAddsCount() == addCount) {
                    continue;
                }
                if (oppPrev->deleted()) {
                    continue;
                }
                oppFirst = oppPrev;
            }
            const SkOpSpanBase* oppNext = oppSpan;
            const SkOpSpanBase* oppLast = oppSpan;
            while ((oppNext = oppNext->final() ? nullptr : oppNext->upCast()->next())) {
                if (!roughly_equal(oppNext->t(), oppSpan->t())) {
                    break;
                }
                if (oppNext->spanAddsCount() == addCount) {
                    continue;
                }
                if (oppNext->deleted()) {
                    continue;
                }
                oppLast = oppNext;
            }
            if (oppFirst == oppLast) {
                continue;
            }
            const SkOpSpanBase* oppTest = oppFirst;
            do {
                if (oppTest == oppSpan) {
                    continue;
                }
                // check to see if the candidate meets specific criteria:
                // it contains spans of segments in test's loop but not including 'this'
                const SkOpPtT* oppStartPtT = oppTest->ptT();
                const SkOpPtT* oppPtT = oppStartPtT;
                while ((oppPtT = oppPtT->next()) != oppStartPtT) {
                    const SkOpSegment* oppPtTSegment = oppPtT->segment();
                    if (oppPtTSegment == this) {
                        goto tryNextSpan;
                    }
                    const SkOpPtT* matchPtT = startPtT;
                    do {
                        if (matchPtT->segment() == oppPtTSegment) {
                            goto foundMatch;
                        }
                    } while ((matchPtT = matchPtT->next()) != startPtT);
                    goto tryNextSpan;
            foundMatch:  // merge oppTest and oppSpan
                    oppSegment->debugValidate();
                    oppTest->debugMergeMatches(id, glitches, oppSpan);
                    oppTest->debugAddOpp(id, glitches, oppSpan);
                    oppSegment->debugValidate();
                    goto checkNextSpan;
                }
        tryNextSpan:
                ;
            } while (oppTest != oppLast && (oppTest = oppTest->upCast()->next()));
        } while ((testPtT = testPtT->next()) != startPtT);
checkNextSpan:
        ;
    } while ((test = test->final() ? nullptr : test->upCast()->next()));
   debugValidate();
   return;
}

// commented-out lines keep this in sync with moveNearby()
// Move nearby t values and pts so they all hang off the same span. Alignment happens later.
void SkOpSegment::debugMoveNearby(const char* id, SkPathOpsDebug::GlitchLog* glitches) const {
    debugValidate();
    // release undeleted spans pointing to this seg that are linked to the primary span
    const SkOpSpanBase* spanBase = &fHead;
    do {
        const SkOpPtT* ptT = spanBase->ptT();
        const SkOpPtT* headPtT = ptT;
        while ((ptT = ptT->next()) != headPtT) {
              const SkOpSpanBase* test = ptT->span();
            if (ptT->segment() == this && !ptT->deleted() && test != spanBase
                    && test->ptT() == ptT) {
                if (test->final()) {
                    if (spanBase == &fHead) {
                        glitches->record(kMoveNearbyClearAll_Glitch, id, this);
//                        return;
                    }
                    glitches->record(kMoveNearbyReleaseFinal_Glitch, id, spanBase, ptT);
                } else if (test->prev()) {
                    glitches->record(kMoveNearbyRelease_Glitch, id, test, headPtT);
                }
//                break;
            }
        }
        spanBase = spanBase->upCast()->next();
    } while (!spanBase->final());

    // This loop looks for adjacent spans which are near by
    spanBase = &fHead;
    do {  // iterate through all spans associated with start
        const SkOpSpanBase* test = spanBase->upCast()->next();
        if (this->spansNearby(spanBase, test)) {
            if (test->final()) {
                if (spanBase->prev()) {
                    glitches->record(kMoveNearbyMergeFinal_Glitch, id, test);
                } else {
                    glitches->record(kMoveNearbyClearAll2_Glitch, id, this);
                    // return
                }
            } else {
                glitches->record(kMoveNearbyMerge_Glitch, id, spanBase);
            }
        }
        spanBase = test;
    } while (!spanBase->final());
    debugValidate();
}
#endif

void SkOpSegment::debugReset() {
    this->init(this->fPts, this->fWeight, this->contour(), this->verb());
}

#if DEBUG_COINCIDENCE_ORDER
void SkOpSegment::debugSetCoinT(int index, SkScalar t) const {
    if (fDebugBaseMax < 0 || fDebugBaseIndex == index) {
        fDebugBaseIndex = index;
        fDebugBaseMin = SkTMin(t, fDebugBaseMin);
        fDebugBaseMax = SkTMax(t, fDebugBaseMax);
        return;
    } 
    SkASSERT(fDebugBaseMin >= t || t >= fDebugBaseMax);
    if (fDebugLastMax < 0 || fDebugLastIndex == index) {
        fDebugLastIndex = index;
        fDebugLastMin = SkTMin(t, fDebugLastMin);
        fDebugLastMax = SkTMax(t, fDebugLastMax);
        return;
    }
    SkASSERT(fDebugLastMin >= t || t >= fDebugLastMax);
    SkASSERT((t - fDebugBaseMin > 0) == (fDebugLastMin - fDebugBaseMin > 0));
}
#endif

#if DEBUG_ACTIVE_SPANS
void SkOpSegment::debugShowActiveSpans() const {
    debugValidate();
    if (done()) {
        return;
    }
    int lastId = -1;
    double lastT = -1;
    const SkOpSpan* span = &fHead;
    do {
        if (span->done()) {
            continue;
        }
        if (lastId == this->debugID() && lastT == span->t()) {
            continue;
        }
        lastId = this->debugID();
        lastT = span->t();
        SkDebugf("%s id=%d", __FUNCTION__, this->debugID());
        // since endpoints may have be adjusted, show actual computed curves
        SkDCurve curvePart;
        this->subDivide(span, span->next(), &curvePart);
        const SkDPoint* pts = curvePart.fCubic.fPts;
        SkDebugf(" (%1.9g,%1.9g", pts[0].fX, pts[0].fY);
        for (int vIndex = 1; vIndex <= SkPathOpsVerbToPoints(fVerb); ++vIndex) {
            SkDebugf(" %1.9g,%1.9g", pts[vIndex].fX, pts[vIndex].fY);
        }
        if (SkPath::kConic_Verb == fVerb) {
            SkDebugf(" %1.9gf", curvePart.fConic.fWeight);
        }
        SkDebugf(") t=%1.9g tEnd=%1.9g", span->t(), span->next()->t());
        if (span->windSum() == SK_MinS32) {
            SkDebugf(" windSum=?");
        } else {
            SkDebugf(" windSum=%d", span->windSum());
        }
        if (span->oppValue() && span->oppSum() == SK_MinS32) {
            SkDebugf(" oppSum=?");
        } else if (span->oppValue() || span->oppSum() != SK_MinS32) {
            SkDebugf(" oppSum=%d", span->oppSum());
        }
        SkDebugf(" windValue=%d", span->windValue());
        if (span->oppValue() || span->oppSum() != SK_MinS32) {
            SkDebugf(" oppValue=%d", span->oppValue());
        }
        SkDebugf("\n");
   } while ((span = span->next()->upCastable()));
}
#endif

#if DEBUG_MARK_DONE
void SkOpSegment::debugShowNewWinding(const char* fun, const SkOpSpan* span, int winding) {
    const SkPoint& pt = span->ptT()->fPt;
    SkDebugf("%s id=%d", fun, this->debugID());
    SkDebugf(" (%1.9g,%1.9g", fPts[0].fX, fPts[0].fY);
    for (int vIndex = 1; vIndex <= SkPathOpsVerbToPoints(fVerb); ++vIndex) {
        SkDebugf(" %1.9g,%1.9g", fPts[vIndex].fX, fPts[vIndex].fY);
    }
    SkDebugf(") t=%1.9g [%d] (%1.9g,%1.9g) tEnd=%1.9g newWindSum=",
            span->t(), span->debugID(), pt.fX, pt.fY, span->next()->t());
    if (winding == SK_MinS32) {
        SkDebugf("?");
    } else {
        SkDebugf("%d", winding);
    }
    SkDebugf(" windSum=");
    if (span->windSum() == SK_MinS32) {
        SkDebugf("?");
    } else {
        SkDebugf("%d", span->windSum());
    }
    SkDebugf(" windValue=%d\n", span->windValue());
}

void SkOpSegment::debugShowNewWinding(const char* fun, const SkOpSpan* span, int winding,
                                      int oppWinding) {
    const SkPoint& pt = span->ptT()->fPt;
    SkDebugf("%s id=%d", fun, this->debugID());
    SkDebugf(" (%1.9g,%1.9g", fPts[0].fX, fPts[0].fY);
    for (int vIndex = 1; vIndex <= SkPathOpsVerbToPoints(fVerb); ++vIndex) {
        SkDebugf(" %1.9g,%1.9g", fPts[vIndex].fX, fPts[vIndex].fY);
    }
    SkDebugf(") t=%1.9g [%d] (%1.9g,%1.9g) tEnd=%1.9g newWindSum=",
            span->t(), span->debugID(), pt.fX, pt.fY, span->next()->t(), winding, oppWinding);
    if (winding == SK_MinS32) {
        SkDebugf("?");
    } else {
        SkDebugf("%d", winding);
    }
    SkDebugf(" newOppSum=");
    if (oppWinding == SK_MinS32) {
        SkDebugf("?");
    } else {
        SkDebugf("%d", oppWinding);
    }
    SkDebugf(" oppSum=");
    if (span->oppSum() == SK_MinS32) {
        SkDebugf("?");
    } else {
        SkDebugf("%d", span->oppSum());
    }
    SkDebugf(" windSum=");
    if (span->windSum() == SK_MinS32) {
        SkDebugf("?");
    } else {
        SkDebugf("%d", span->windSum());
    }
    SkDebugf(" windValue=%d oppValue=%d\n", span->windValue(), span->oppValue());
}

#endif

// loop looking for a pair of angle parts that are too close to be sorted
/* This is called after other more simple intersection and angle sorting tests have been exhausted.
   This should be rarely called -- the test below is thorough and time consuming.
   This checks the distance between start points; the distance between
*/
#if DEBUG_ANGLE
void SkOpAngle::debugCheckNearCoincidence() const {
    const SkOpAngle* test = this;
    do {
        const SkOpSegment* testSegment = test->segment();
        double testStartT = test->start()->t();
        SkDPoint testStartPt = testSegment->dPtAtT(testStartT);
        double testEndT = test->end()->t();
        SkDPoint testEndPt = testSegment->dPtAtT(testEndT);
        double testLenSq = testStartPt.distanceSquared(testEndPt);
        SkDebugf("%s testLenSq=%1.9g id=%d\n", __FUNCTION__, testLenSq, testSegment->debugID());
        double testMidT = (testStartT + testEndT) / 2;
        const SkOpAngle* next = test;
        while ((next = next->fNext) != this) {
            SkOpSegment* nextSegment = next->segment();
            double testMidDistSq = testSegment->distSq(testMidT, next);
            double testEndDistSq = testSegment->distSq(testEndT, next);
            double nextStartT = next->start()->t();
            SkDPoint nextStartPt = nextSegment->dPtAtT(nextStartT);
            double distSq = testStartPt.distanceSquared(nextStartPt);
            double nextEndT = next->end()->t();
            double nextMidT = (nextStartT + nextEndT) / 2;
            double nextMidDistSq = nextSegment->distSq(nextMidT, test);
            double nextEndDistSq = nextSegment->distSq(nextEndT, test);
            SkDebugf("%s distSq=%1.9g testId=%d nextId=%d\n", __FUNCTION__, distSq,
                    testSegment->debugID(), nextSegment->debugID());
            SkDebugf("%s testMidDistSq=%1.9g\n", __FUNCTION__, testMidDistSq);
            SkDebugf("%s testEndDistSq=%1.9g\n", __FUNCTION__, testEndDistSq);
            SkDebugf("%s nextMidDistSq=%1.9g\n", __FUNCTION__, nextMidDistSq);
            SkDebugf("%s nextEndDistSq=%1.9g\n", __FUNCTION__, nextEndDistSq);
            SkDPoint nextEndPt = nextSegment->dPtAtT(nextEndT);
            double nextLenSq = nextStartPt.distanceSquared(nextEndPt);
            SkDebugf("%s nextLenSq=%1.9g\n", __FUNCTION__, nextLenSq);
            SkDebugf("\n");
        }
        test = test->fNext;
    } while (test->fNext != this);
}
#endif

#if DEBUG_ANGLE
SkString SkOpAngle::debugPart() const {
    SkString result;
    switch (this->segment()->verb()) {
        case SkPath::kLine_Verb:
            result.printf(LINE_DEBUG_STR " id=%d", LINE_DEBUG_DATA(fPart.fCurve),
                    this->segment()->debugID());
            break;
        case SkPath::kQuad_Verb:
            result.printf(QUAD_DEBUG_STR " id=%d", QUAD_DEBUG_DATA(fPart.fCurve),
                    this->segment()->debugID());
            break;
        case SkPath::kConic_Verb:
            result.printf(CONIC_DEBUG_STR " id=%d",
                    CONIC_DEBUG_DATA(fPart.fCurve, fPart.fCurve.fConic.fWeight),
                    this->segment()->debugID());
            break;
        case SkPath::kCubic_Verb:
            result.printf(CUBIC_DEBUG_STR " id=%d", CUBIC_DEBUG_DATA(fPart.fCurve),
                    this->segment()->debugID());
            break;
        default:
            SkASSERT(0);
    }
    return result;
}
#endif

#if DEBUG_SORT
void SkOpAngle::debugLoop() const {
    const SkOpAngle* first = this;
    const SkOpAngle* next = this;
    do {
        next->dumpOne(true);
        SkDebugf("\n");
        next = next->fNext;
    } while (next && next != first);
    next = first;
    do {
        next->debugValidate();
        next = next->fNext;
    } while (next && next != first);
}
#endif

void SkOpAngle::debugValidate() const {
#if DEBUG_COINCIDENCE
    if (this->globalState()->debugCheckHealth()) {
        return;
    }
#endif
#if DEBUG_VALIDATE
    const SkOpAngle* first = this;
    const SkOpAngle* next = this;
    int wind = 0;
    int opp = 0;
    int lastXor = -1;
    int lastOppXor = -1;
    do {
        if (next->unorderable()) {
            return;
        }
        const SkOpSpan* minSpan = next->start()->starter(next->end());
        if (minSpan->windValue() == SK_MinS32) {
            return;
        }
        bool op = next->segment()->operand();
        bool isXor = next->segment()->isXor();
        bool oppXor = next->segment()->oppXor();
        SkASSERT(!DEBUG_LIMIT_WIND_SUM || between(0, minSpan->windValue(), DEBUG_LIMIT_WIND_SUM));
        SkASSERT(!DEBUG_LIMIT_WIND_SUM
                || between(-DEBUG_LIMIT_WIND_SUM, minSpan->oppValue(), DEBUG_LIMIT_WIND_SUM));
        bool useXor = op ? oppXor : isXor;
        SkASSERT(lastXor == -1 || lastXor == (int) useXor);
        lastXor = (int) useXor;
        wind += next->debugSign() * (op ? minSpan->oppValue() : minSpan->windValue());
        if (useXor) {
            wind &= 1;
        }
        useXor = op ? isXor : oppXor;
        SkASSERT(lastOppXor == -1 || lastOppXor == (int) useXor);
        lastOppXor = (int) useXor;
        opp += next->debugSign() * (op ? minSpan->windValue() : minSpan->oppValue());
        if (useXor) {
            opp &= 1;
        }
        next = next->fNext;
    } while (next && next != first);
    SkASSERT(wind == 0 || !FLAGS_runFail);
    SkASSERT(opp == 0 || !FLAGS_runFail);
#endif
}

void SkOpAngle::debugValidateNext() const {
#if !FORCE_RELEASE
    const SkOpAngle* first = this;
    const SkOpAngle* next = first;
    SkTDArray<const SkOpAngle*>(angles);
    do {
//        SkASSERT_RELEASE(next->fSegment->debugContains(next));
        angles.push(next);
        next = next->next();
        if (next == first) {
            break;
        }
        SkASSERT_RELEASE(!angles.contains(next));
        if (!next) {
            return;
        }
    } while (true);
#endif
}

#ifdef SK_DEBUG
void SkCoincidentSpans::debugStartCheck(const SkOpSpanBase* outer, const SkOpSpanBase* over,
        const SkOpGlobalState* debugState) const {
    SkASSERT(coinPtTEnd()->span() == over || !debugState->debugRunFail());
    SkASSERT(oppPtTEnd()->span() == outer || !debugState->debugRunFail());
}
#endif

#if DEBUG_COINCIDENCE_VERBOSE
/* Commented-out lines keep this in sync with expand */
// expand the range by checking adjacent spans for coincidence
bool SkCoincidentSpans::debugExpand(const char* id, SkPathOpsDebug::GlitchLog* log) const {
    bool expanded = false;
    const SkOpSegment* segment = coinPtTStart()->segment();
    const SkOpSegment* oppSegment = oppPtTStart()->segment();
    do {
        const SkOpSpan* start = coinPtTStart()->span()->upCast();
        const SkOpSpan* prev = start->prev();
        const SkOpPtT* oppPtT;
        if (!prev || !(oppPtT = prev->contains(oppSegment))) {
            break;
        }
        double midT = (prev->t() + start->t()) / 2;
        if (!segment->isClose(midT, oppSegment)) {
            break;
        }
        if (log) log->record(kExpandCoin_Glitch, id, this, prev->ptT(), oppPtT);
        expanded = true;
    } while (false);  // actual continues while expansion is possible
    do {
        const SkOpSpanBase* end = coinPtTEnd()->span();
        SkOpSpanBase* next = end->final() ? nullptr : end->upCast()->next();
        if (next && next->deleted()) {
            break;
        }
        const SkOpPtT* oppPtT;
        if (!next || !(oppPtT = next->contains(oppSegment))) {
            break;
        }
        double midT = (end->t() + next->t()) / 2;
        if (!segment->isClose(midT, oppSegment)) {
            break;
        }
        if (log) log->record(kExpandCoin_Glitch, id, this, next->ptT(), oppPtT);
        expanded = true;
    } while (false);  // actual continues while expansion is possible
    return expanded;
}

/* Commented-out lines keep this in sync with addExpanded */
// for each coincident pair, match the spans
// if the spans don't match, add the mssing pt to the segment and loop it in the opposite span
void SkOpCoincidence::debugAddExpanded(const char* id, SkPathOpsDebug::GlitchLog* log) const {
    const SkCoincidentSpans* coin = this->fHead;
    if (!coin) {
        return;
    }
    do {
        const SkOpPtT* startPtT = coin->coinPtTStart();
        const SkOpPtT* oStartPtT = coin->oppPtTStart();
        SkASSERT(startPtT->contains(oStartPtT));
        SkASSERT(coin->coinPtTEnd()->contains(coin->oppPtTEnd()));
        const SkOpSpanBase* start = startPtT->span();
        const SkOpSpanBase* oStart = oStartPtT->span();
        const SkOpSpanBase* end = coin->coinPtTEnd()->span();
        const SkOpSpanBase* oEnd = coin->oppPtTEnd()->span();
        FAIL_IF(oEnd->deleted(), coin);
        FAIL_IF(!start->upCastable(), coin);
        const SkOpSpanBase* test = start->upCast()->next();
        const SkOpSpanBase* oTest = coin->flipped() ? oStart->prev() : oStart->upCast()->next();
        if (!oTest) {
            return;
        }
        while (test != end || oTest != oEnd) {
            if (!test->ptT()->contains(oTest->segment())
                    || !oTest->ptT()->contains(start->segment())) {
                // use t ranges to guess which one is missing
                double startRange = coin->coinPtTEnd()->fT - startPtT->fT;
                FAIL_IF(!startRange, coin);
                double startPart = (test->t() - startPtT->fT) / startRange;
                double oStartRange = coin->oppPtTEnd()->fT - oStartPtT->fT;
                FAIL_IF(!oStartRange, coin);
                double oStartPart = (oTest->t() - oStartPtT->fT) / oStartRange;
                FAIL_IF(startPart == oStartPart, coin);
                bool startOver = false;
                if (startPart < oStartPart)
                        log->record(kAddExpandedCoin_Glitch, id,  // strange debug formatting lines up with original
                                oStartPtT->fT + oStartRange * startPart, test);
                        else log->record(kAddExpandedCoin_Glitch, id,
                                startPtT->fT + startRange * oStartPart, oTest);
                if (false) {
                    SkASSERT(0);
                    return;
                }
                if (startOver) {
                    test = start;
                    oTest = oStart;
                }
            }
            if (test != end) {
                if (!test->upCastable()) {
                    return;
                }
                test = test->upCast()->next();
            }
            if (oTest != oEnd) {
                oTest = coin->flipped() ? oTest->prev() : oTest->upCast()->next();
                if (!oTest) {
                    return;
                }
            }
        }
    } while ((coin = coin->next()));
    return;
}

/* Commented-out lines keep this in sync with addIfMissing() */
void SkOpCoincidence::debugAddIfMissing(const char* id, SkPathOpsDebug::GlitchLog* log, const SkCoincidentSpans* outer, const SkOpPtT* over1s,
            const SkOpPtT* over1e) const {
//     SkASSERT(fTop);
    if (fTop && alreadyAdded(fTop, outer, over1s, over1e)) {  // in debug, fTop may be null
        return;
    }
    if (fHead && alreadyAdded(fHead, outer, over1s, over1e)) {
        return;
    }
    log->record(kAddIfMissingCoin_Glitch, id, outer->coinPtTStart(), outer->coinPtTEnd(), over1s, over1e);
    this->debugValidate();
    return;
}

/* Commented-out lines keep this in sync addIfMissing() */
// note that over1s, over1e, over2s, over2e are ordered
void SkOpCoincidence::debugAddIfMissing(const char* id, SkPathOpsDebug::GlitchLog* log, const SkOpPtT* over1s, const SkOpPtT* over2s,
        double tStart, double tEnd, const SkOpSegment* coinSeg, const SkOpSegment* oppSeg, bool* added,
        const SkOpPtT* over1e, const SkOpPtT* over2e) const {
    SkASSERT(tStart < tEnd);
    SkASSERT(over1s->fT < over1e->fT);
    SkASSERT(between(over1s->fT, tStart, over1e->fT));
    SkASSERT(between(over1s->fT, tEnd, over1e->fT));
    SkASSERT(over2s->fT < over2e->fT);
    SkASSERT(between(over2s->fT, tStart, over2e->fT));
    SkASSERT(between(over2s->fT, tEnd, over2e->fT));
    SkASSERT(over1s->segment() == over1e->segment());
    SkASSERT(over2s->segment() == over2e->segment());
    SkASSERT(over1s->segment() == over2s->segment());
    SkASSERT(over1s->segment() != coinSeg);
    SkASSERT(over1s->segment() != oppSeg);
    SkASSERT(coinSeg != oppSeg);
    double coinTs, coinTe, oppTs, oppTe;
    coinTs = TRange(over1s, tStart, coinSeg  SkDEBUGPARAMS(over1e));
    coinTe = TRange(over1s, tEnd, coinSeg  SkDEBUGPARAMS(over1e));
    if (coinSeg->collapsed(coinTs, coinTe)) {
        return log->record(kAddIfCollapsed_Glitch, id, coinSeg);
    }
    oppTs = TRange(over2s, tStart, oppSeg  SkDEBUGPARAMS(over2e));
    oppTe = TRange(over2s, tEnd, oppSeg  SkDEBUGPARAMS(over2e));
    if (oppSeg->collapsed(oppTs, oppTe)) {
        return log->record(kAddIfCollapsed_Glitch, id, oppSeg);
    }
    if (coinTs > coinTe) {
        SkTSwap(coinTs, coinTe);
        SkTSwap(oppTs, oppTe);
    }
    return this->debugAddOrOverlap(id, log, coinSeg, oppSeg, coinTs, coinTe, oppTs, oppTe, added
            );
}

/* Commented-out lines keep this in sync addOrOverlap() */
// If this is called by addEndMovedSpans(), a returned false propogates out to an abort.
// If this is called by AddIfMissing(), a returned false indicates there was nothing to add
void SkOpCoincidence::debugAddOrOverlap(const char* id, SkPathOpsDebug::GlitchLog* log,
        const SkOpSegment* coinSeg, const SkOpSegment* oppSeg,
        double coinTs, double coinTe, double oppTs, double oppTe, bool* added) const {
    SkTDArray<SkCoincidentSpans*> overlaps;
    SkASSERT(!fTop);   // this is (correctly) reversed in addifMissing()
    if (fTop && !this->checkOverlap(fTop, coinSeg, oppSeg, coinTs, coinTe, oppTs, oppTe,
            &overlaps)) {
        return;
    }
    if (fHead && !this->checkOverlap(fHead, coinSeg, oppSeg, coinTs,
            coinTe, oppTs, oppTe, &overlaps)) {
        return;
    }
    const SkCoincidentSpans* overlap = overlaps.count() ? overlaps[0] : nullptr;
    for (int index = 1; index < overlaps.count(); ++index) { // combine overlaps before continuing
        const SkCoincidentSpans* test = overlaps[index];
        if (overlap->coinPtTStart()->fT > test->coinPtTStart()->fT) {
            log->record(kAddOrOverlap_Glitch, id, overlap, test->coinPtTStart());
        }
        if (overlap->coinPtTEnd()->fT < test->coinPtTEnd()->fT) {
            log->record(kAddOrOverlap_Glitch, id, overlap, test->coinPtTEnd());
        }
        if (overlap->flipped()
                ? overlap->oppPtTStart()->fT < test->oppPtTStart()->fT
                : overlap->oppPtTStart()->fT > test->oppPtTStart()->fT) {
            log->record(kAddOrOverlap_Glitch, id, overlap, test->oppPtTStart());
        }
        if (overlap->flipped()
                ? overlap->oppPtTEnd()->fT > test->oppPtTEnd()->fT
                : overlap->oppPtTEnd()->fT < test->oppPtTEnd()->fT) {
            log->record(kAddOrOverlap_Glitch, id, overlap, test->oppPtTEnd());
        }
        if (!fHead) { this->debugRelease(id, log, fHead, test);
            this->debugRelease(id, log, fTop, test);
        }
    }
    const SkOpPtT* cs = coinSeg->existing(coinTs, oppSeg);
    const SkOpPtT* ce = coinSeg->existing(coinTe, oppSeg);
    RETURN_FALSE_IF(overlap && cs && ce && overlap->contains(cs, ce), coinSeg);
    RETURN_FALSE_IF(cs != ce || !cs, coinSeg);
    const SkOpPtT* os = oppSeg->existing(oppTs, coinSeg);
    const SkOpPtT* oe = oppSeg->existing(oppTe, coinSeg);
    RETURN_FALSE_IF(overlap && os && oe && overlap->contains(os, oe), oppSeg);
    SkASSERT(true || !cs || !cs->deleted());
    SkASSERT(true || !os || !os->deleted());
    SkASSERT(true || !ce || !ce->deleted());
    SkASSERT(true || !oe || !oe->deleted());
    const SkOpPtT* csExisting = !cs ? coinSeg->existing(coinTs, nullptr) : nullptr;
    const SkOpPtT* ceExisting = !ce ? coinSeg->existing(coinTe, nullptr) : nullptr;
    RETURN_FALSE_IF(csExisting && csExisting == ceExisting, coinSeg);
    RETURN_FALSE_IF(csExisting && (csExisting == ce ||
            csExisting->contains(ceExisting ? ceExisting : ce)), coinSeg);
    RETURN_FALSE_IF(ceExisting && (ceExisting == cs ||
            ceExisting->contains(csExisting ? csExisting : cs)), coinSeg);
    const SkOpPtT* osExisting = !os ? oppSeg->existing(oppTs, nullptr) : nullptr;
    const SkOpPtT* oeExisting = !oe ? oppSeg->existing(oppTe, nullptr) : nullptr;
    RETURN_FALSE_IF(osExisting && osExisting == oeExisting, oppSeg);
    RETURN_FALSE_IF(osExisting && (osExisting == oe ||
            osExisting->contains(oeExisting ? oeExisting : oe)), oppSeg);
    RETURN_FALSE_IF(oeExisting && (oeExisting == os ||
            oeExisting->contains(osExisting ? osExisting : os)), oppSeg);
    bool csDeleted = false, osDeleted = false, ceDeleted = false,  oeDeleted = false;
    this->debugValidate();
    if (!cs || !os) {
        if (!cs)
            cs = coinSeg->debugAddT(coinTs, id, log);
        if (!os)
            os = oppSeg->debugAddT(oppTs, id, log);
//      RETURN_FALSE_IF(callerAborts, !csWritable || !osWritable);
        if (cs && os) cs->span()->debugAddOpp(id, log, os->span());
//         cs = csWritable;
//         os = osWritable->active();
        RETURN_FALSE_IF((ce && ce->deleted()) || (oe && oe->deleted()), coinSeg);
    }
    if (!ce || !oe) {
        if (!ce)
            ce = coinSeg->debugAddT(coinTe, id, log);
        if (!oe)
            oe = oppSeg->debugAddT(oppTe, id, log);
        if (ce && oe) ce->span()->debugAddOpp(id, log, oe->span());
//         ce = ceWritable;
//         oe = oeWritable;
    }
    this->debugValidate();
    RETURN_FALSE_IF(csDeleted, coinSeg);
    RETURN_FALSE_IF(osDeleted, oppSeg); 
    RETURN_FALSE_IF(ceDeleted, coinSeg); 
    RETURN_FALSE_IF(oeDeleted, oppSeg);
    RETURN_FALSE_IF(!cs || !ce || cs == ce || cs->contains(ce) || !os || !oe || os == oe || os->contains(oe), coinSeg);
    bool result = true;
    if (overlap) {
        if (overlap->coinPtTStart()->segment() == coinSeg) {
                log->record(kAddMissingExtend_Glitch, id, coinSeg, coinTs, coinTe, oppSeg, oppTs, oppTe);
        } else {
            if (oppTs > oppTe) {
                SkTSwap(coinTs, coinTe);
                SkTSwap(oppTs, oppTe);
            }
            log->record(kAddMissingExtend_Glitch, id, oppSeg, oppTs, oppTe, coinSeg, coinTs, coinTe);
        }
#if 0 && DEBUG_COINCIDENCE_VERBOSE
        if (result) {
             overlap->debugShow();
        }
#endif
    } else {
        log->record(kAddMissingCoin_Glitch, id, coinSeg, coinTs, coinTe, oppSeg, oppTs, oppTe);
#if 0 && DEBUG_COINCIDENCE_VERBOSE
        fHead->debugShow();
#endif
    }
    this->debugValidate();
    return (void) result;
}

// Extra commented-out lines keep this in sync with addMissing()
/* detects overlaps of different coincident runs on same segment */
/* does not detect overlaps for pairs without any segments in common */
// returns true if caller should loop again
void SkOpCoincidence::debugAddMissing(const char* id, SkPathOpsDebug::GlitchLog* log, bool* added) const {
    const SkCoincidentSpans* outer = fHead;
    if (!outer) {
        return;
    }
    // bool added = false;
    // fTop = outer;
    // fHead = nullptr;
    do {
    // addifmissing can modify the list that this is walking
    // save head so that walker can iterate over old data unperturbed
    // addifmissing adds to head freely then add saved head in the end
        const SkOpPtT* ocs = outer->coinPtTStart();
        SkASSERT(!ocs->deleted());
        const SkOpSegment* outerCoin = ocs->segment();
        SkASSERT(!outerCoin->done());  // if it's done, should have already been removed from list
        const SkOpPtT* oos = outer->oppPtTStart();
        SkASSERT(!oos->deleted());
        const SkOpSegment* outerOpp = oos->segment();
        SkASSERT(!outerOpp->done());
//        SkOpSegment* outerCoinWritable = const_cast<SkOpSegment*>(outerCoin);
//        SkOpSegment* outerOppWritable = const_cast<SkOpSegment*>(outerOpp);
        const SkCoincidentSpans* inner = outer;
        while ((inner = inner->next())) {
            this->debugValidate();
            double overS, overE;
            const SkOpPtT* ics = inner->coinPtTStart();
            SkASSERT(!ics->deleted());
            const SkOpSegment* innerCoin = ics->segment();
            SkASSERT(!innerCoin->done());
            const SkOpPtT* ios = inner->oppPtTStart();
            SkASSERT(!ios->deleted());
            const SkOpSegment* innerOpp = ios->segment();
            SkASSERT(!innerOpp->done());
//            SkOpSegment* innerCoinWritable = const_cast<SkOpSegment*>(innerCoin);
//            SkOpSegment* innerOppWritable = const_cast<SkOpSegment*>(innerOpp);
            if (outerCoin == innerCoin) {
                const SkOpPtT* oce = outer->coinPtTEnd();
                SkASSERT(!oce->deleted());
                const SkOpPtT* ice = inner->coinPtTEnd();
                SkASSERT(!ice->deleted());
                if (outerOpp != innerOpp && this->overlap(ocs, oce, ics, ice, &overS, &overE)) {
                    this->debugAddIfMissing(id, log, ocs->starter(oce), ics->starter(ice),
                            overS, overE, outerOpp, innerOpp, added,
                            ocs->debugEnder(oce),
                            ics->debugEnder(ice));
                }
            } else if (outerCoin == innerOpp) {
                const SkOpPtT* oce = outer->coinPtTEnd();
                SkASSERT(!oce->deleted());
                const SkOpPtT* ioe = inner->oppPtTEnd();
                SkASSERT(!ioe->deleted());
                if (outerOpp != innerCoin && this->overlap(ocs, oce, ios, ioe, &overS, &overE)) {
                    this->debugAddIfMissing(id, log, ocs->starter(oce), ios->starter(ioe),
                            overS, overE, outerOpp, innerCoin, added,
                            ocs->debugEnder(oce),
                            ios->debugEnder(ioe));
                }
            } else if (outerOpp == innerCoin) {
                const SkOpPtT* ooe = outer->oppPtTEnd();
                SkASSERT(!ooe->deleted());
                const SkOpPtT* ice = inner->coinPtTEnd();
                SkASSERT(!ice->deleted());
                SkASSERT(outerCoin != innerOpp);
                if (this->overlap(oos, ooe, ics, ice, &overS, &overE)) {
                    this->debugAddIfMissing(id, log, oos->starter(ooe), ics->starter(ice),
                            overS, overE, outerCoin, innerOpp, added,
                            oos->debugEnder(ooe),
                            ics->debugEnder(ice));
                }
            } else if (outerOpp == innerOpp) {
                const SkOpPtT* ooe = outer->oppPtTEnd();
                SkASSERT(!ooe->deleted());
                const SkOpPtT* ioe = inner->oppPtTEnd();
                SkASSERT(!ioe->deleted());
                SkASSERT(outerCoin != innerCoin);
                if (this->overlap(oos, ooe, ios, ioe, &overS, &overE)) {
                    this->debugAddIfMissing(id, log, oos->starter(ooe), ios->starter(ioe),
                            overS, overE, outerCoin, innerCoin, added,
                            oos->debugEnder(ooe),
                            ios->debugEnder(ioe));
                }
            }
            this->debugValidate();
        }
    } while ((outer = outer->next()));
    // this->restoreHead();
    return;
}

// Commented-out lines keep this in sync with release()
void SkOpCoincidence::debugRelease(const char* id, SkPathOpsDebug::GlitchLog* log, const SkCoincidentSpans* coin, const SkCoincidentSpans* remove) const {
    const SkCoincidentSpans* head = coin;
    const SkCoincidentSpans* prev = nullptr;
    const SkCoincidentSpans* next;
    do {
        next = coin->next();
        if (coin == remove) {
            if (prev) {
//                prev->setNext(next);
            } else if (head == fHead) {
//                fHead = next;
            } else {
//                fTop = next;
            }
            log->record(kReleasedSpan_Glitch, id, coin);
        }
        prev = coin;
    } while ((coin = next));
    return;
}

void SkOpCoincidence::debugRelease(const char* id, SkPathOpsDebug::GlitchLog* log, const SkOpSegment* deleted) const {
    const SkCoincidentSpans* coin = fHead;
    if (!coin) {
        return;
    }
    do {
        if (coin->coinPtTStart()->segment() == deleted
                || coin->coinPtTEnd()->segment() == deleted
                || coin->oppPtTStart()->segment() == deleted
                || coin->oppPtTEnd()->segment() == deleted) {
            log->record(kReleasedSpan_Glitch, id, coin);
        }
    } while ((coin = coin->next()));
}


// Commented-out lines keep this in sync with reorder()
// iterate through all coincident pairs, looking for ranges greater than 1
// if found, see if the opposite pair can match it -- which may require
// reordering the ptT pairs
void SkOpCoincidence::debugReorder(const char* id, SkPathOpsDebug::GlitchLog* log) const {
    const SkCoincidentSpans* coin = fHead;
    if (!coin) {
        return;
    }
    do {
        // most commonly, concidence are one span long; check for that first
        int intervals = coin->spanCount();
        if (intervals <= 0) {
            return;
        }
        if (1 == intervals) {
#if DEBUG_COINCIDENCE_VERBOSE
            // SkASSERT(!coin->debugExpand(nullptr, nullptr));
#endif
            continue;
        }
        coin->debugExpand(id, log);
        if (coin->spanCount() <= 0) {
            return;
        }
        // check to see if every span in coin has a mate in opp
        const SkOpSpan* start = coin->coinPtTStart()->span()->upCast();
        bool flipped = coin->flipped();
        const SkOpSpanBase* oppStartBase = coin->oppPtTStart()->span();
        const SkOpSpan* oppStart = flipped ? oppStartBase->prev() : oppStartBase->upCast();
        SkDebugf("", start, oppStart);
    } while ((coin = coin->next()));
    return;
}

// Commented-out lines keep this in sync with expand()
// expand the range by checking adjacent spans for coincidence
bool SkOpCoincidence::debugExpand(const char* id, SkPathOpsDebug::GlitchLog* log) const {
    const SkCoincidentSpans* coin = fHead;
    if (!coin) {
        return false;
    }
    bool expanded = false;
    do {
        if (coin->debugExpand(id, log)) {
            // check to see if multiple spans expanded so they are now identical
            const SkCoincidentSpans* test = fHead;
            do {
                if (coin == test) {
                    continue;
                }
                if (coin->coinPtTStart() == test->coinPtTStart()
                        && coin->oppPtTStart() == test->oppPtTStart()) {
                    if (log) log->record(kExpandCoin_Glitch, id, fHead, test->coinPtTStart());
                    break;
                }
            } while ((test = test->next()));
            expanded = true;
        }
    } while ((coin = coin->next()));
    return expanded;
}

// Commented-out lines keep this in sync with removeCollapsed()
void SkOpCoincidence::debugRemoveCollapsed(const char* id, SkPathOpsDebug::GlitchLog* log) const {
    const SkCoincidentSpans* coin = fHead;
    if (!coin) {
        return;
    }
    // SkCoincidentSpans** priorPtr = &fHead;
    do {
        if (coin->coinPtTStart() == coin->coinPtTEnd()) {
            return;
        }
        if (coin->oppPtTStart() == coin->oppPtTEnd()) {
            return;
        }
        if (coin->coinPtTStart()->collapsed(coin->coinPtTEnd())) {
            log->record(kCollapsedCoin_Glitch, id, coin);
//             continue;
        }
        if (coin->oppPtTStart()->collapsed(coin->oppPtTEnd())) {
            log->record(kCollapsedCoin_Glitch, id, coin, coin);
//             continue;
        }
        // priorPtr = &coin->nextPtr();
    } while ((coin = coin->next()));
    return;
}

// Commented-out lines keep this in sync with mark()
/* this sets up the coincidence links in the segments when the coincidence crosses multiple spans */
void SkOpCoincidence::debugMark(const char* id, SkPathOpsDebug::GlitchLog* log) const {
    const SkCoincidentSpans* coin = fHead;
    if (!coin) {
        return;
    }
    do {
        if (!coin->coinPtTStartWritable()->span()->upCastable()) {
            return;
        }
        const SkOpSpan* start = coin->coinPtTStartWritable()->span()->upCast();
//         SkASSERT(start->deleted());
        const SkOpSpanBase* end = coin->coinPtTEndWritable()->span();
//         SkASSERT(end->deleted());
        const SkOpSpanBase* oStart = coin->oppPtTStartWritable()->span();
//         SkASSERT(oStart->deleted());
        const SkOpSpanBase* oEnd = coin->oppPtTEndWritable()->span();
//         SkASSERT(oEnd->deleted());
        bool flipped = coin->flipped();
        if (flipped) {
            SkTSwap(oStart, oEnd);
        }
        /* coin and opp spans may not match up. Mark the ends, and then let the interior
           get marked as many times as the spans allow */
        start->debugInsertCoincidence(id, log, oStart->upCast());
        end->debugInsertCoinEnd(id, log, oEnd);
        const SkOpSegment* segment = start->segment();
        const SkOpSegment* oSegment = oStart->segment();
        const SkOpSpanBase* next = start;
        const SkOpSpanBase* oNext = oStart;
        while ((next = next->upCast()->next()) != end) {
            if (!next->upCastable()) {
                return;
            }
            if (next->upCast()->debugInsertCoincidence(id, log, oSegment, flipped), false) {
                return;
            }
        }
        while ((oNext = oNext->upCast()->next()) != oEnd) {
            if (!oNext->upCastable()) {
                return;
            }
            if (oNext->upCast()->debugInsertCoincidence(id, log, segment, flipped), false) {
                return;
            }
        }
    } while ((coin = coin->next()));
    return;
}
#endif

#if DEBUG_COINCIDENCE_VERBOSE
// Commented-out lines keep this in sync with markCollapsed()
void SkOpCoincidence::debugMarkCollapsed(const char* id, SkPathOpsDebug::GlitchLog* log, const SkCoincidentSpans* coin, const SkOpPtT* test) const {
    const SkCoincidentSpans* head = coin;
    while (coin) {
        if (coin->collapsed(test)) {
            if (zero_or_one(coin->coinPtTStart()->fT) && zero_or_one(coin->coinPtTEnd()->fT)) {
                log->record(kCollapsedCoin_Glitch, id, coin);
            }
            if (zero_or_one(coin->oppPtTStart()->fT) && zero_or_one(coin->oppPtTEnd()->fT)) {
                log->record(kCollapsedCoin_Glitch, id, coin);
            }
            this->debugRelease(id, log, head, coin);
        }
        coin = coin->next();
    }
}

// Commented-out lines keep this in sync with markCollapsed()
void SkOpCoincidence::debugMarkCollapsed(const char* id, SkPathOpsDebug::GlitchLog* log, const SkOpPtT* test) const {
    this->debugMarkCollapsed(id, log, fHead, test);
    this->debugMarkCollapsed(id, log, fTop, test);
}
#endif

void SkCoincidentSpans::debugShow() const {
    SkDebugf("%s - id=%d t=%1.9g tEnd=%1.9g\n", __FUNCTION__,
            coinPtTStart()->segment()->debugID(),
            coinPtTStart()->fT, coinPtTEnd()->fT);
    SkDebugf("%s + id=%d t=%1.9g tEnd=%1.9g\n", __FUNCTION__,
            oppPtTStart()->segment()->debugID(),
            oppPtTStart()->fT, oppPtTEnd()->fT);
}

void SkOpCoincidence::debugShowCoincidence() const {
#if DEBUG_COINCIDENCE
    const SkCoincidentSpans* span = fHead;
    while (span) {
        span->debugShow();
        span = span->next();
    }
#endif
}

#if DEBUG_COINCIDENCE
static void DebugValidate(const SkOpSpanBase* next, const SkOpSpanBase* end,
        double oStart, double oEnd, const SkOpSegment* oSegment,
        const char* id, SkPathOpsDebug::GlitchLog* log) {
    SkASSERT(next != end);
    SkASSERT(!next->contains(end) || log);
    if (next->t() > end->t()) {
        SkTSwap(next, end);
    }
    do {
        const SkOpPtT* ptT = next->ptT();
        int index = 0;
        bool somethingBetween = false;
        do {
            ++index;
            ptT = ptT->next();
            const SkOpPtT* checkPtT = next->ptT();
            if (ptT == checkPtT) {
                break;
            }
            bool looped = false;
            for (int check = 0; check < index; ++check) {
                if ((looped = checkPtT == ptT)) {
                    break;
                }
                checkPtT = checkPtT->next();
            }
            if (looped) {
                SkASSERT(0);
                break;
            }
            if (ptT->deleted()) {
                continue;
            }
            if (ptT->segment() != oSegment) {
                continue;
            }
            somethingBetween |= between(oStart, ptT->fT, oEnd);
        } while (true);
        SkASSERT(somethingBetween);
    } while (next != end && (next = next->upCast()->next()));
}

static void DebugCheckOverlap(const SkCoincidentSpans* test, const SkCoincidentSpans* list,
        const char* id, SkPathOpsDebug::GlitchLog* log) {
    if (!list) {
        return;
    }
    const SkOpSegment* coinSeg = test->coinPtTStart()->segment();
    SkASSERT(coinSeg == test->coinPtTEnd()->segment());
    const SkOpSegment* oppSeg = test->oppPtTStart()->segment();
    SkASSERT(oppSeg == test->oppPtTEnd()->segment());
    SkASSERT(coinSeg != test->oppPtTStart()->segment());
    SkDEBUGCODE(double tcs = test->coinPtTStart()->fT);
    SkASSERT(between(0, tcs, 1));
    SkDEBUGCODE(double tce = test->coinPtTEnd()->fT);
    SkASSERT(between(0, tce, 1));
    SkASSERT(tcs < tce);
    double tos = test->oppPtTStart()->fT;
    SkASSERT(between(0, tos, 1));
    double toe = test->oppPtTEnd()->fT;
    SkASSERT(between(0, toe, 1));
    SkASSERT(tos != toe);
    if (tos > toe) {
        SkTSwap(tos, toe);
    }
    do {
        double lcs, lce, los, loe;
        if (coinSeg == list->coinPtTStart()->segment()) {
            if (oppSeg != list->oppPtTStart()->segment()) {
                continue;
            }
            lcs = list->coinPtTStart()->fT;
            lce = list->coinPtTEnd()->fT;
            los = list->oppPtTStart()->fT;
            loe = list->oppPtTEnd()->fT;
            if (los > loe) {
                SkTSwap(los, loe);
            }
        } else if (coinSeg == list->oppPtTStart()->segment()) {
            if (oppSeg != list->coinPtTStart()->segment()) {
                continue;
            }
            lcs = list->oppPtTStart()->fT;
            lce = list->oppPtTEnd()->fT;
            if (lcs > lce) {
                SkTSwap(lcs, lce);
            }
            los = list->coinPtTStart()->fT;
            loe = list->coinPtTEnd()->fT;
        } else {
            continue;
        }
        SkASSERT(tce < lcs || lce < tcs);
        SkASSERT(toe < los || loe < tos);
    } while ((list = list->next()));
}


static void DebugCheckOverlapTop(const SkCoincidentSpans* head, const SkCoincidentSpans* opt,
        const char* id, SkPathOpsDebug::GlitchLog* log) {
    // check for overlapping coincident spans
    const SkCoincidentSpans* test = head;
    while (test) {
        const SkCoincidentSpans* next = test->next();
        DebugCheckOverlap(test, next, id, log);
        DebugCheckOverlap(test, opt, id, log);
        test = next;
    }
}

static void DebugValidate(const SkCoincidentSpans* head, const SkCoincidentSpans* opt,
        const char* id, SkPathOpsDebug::GlitchLog* log) {
    // look for pts inside coincident spans that are not inside the opposite spans
    const SkCoincidentSpans* coin = head;
    while (coin) {
        SkASSERT(SkOpCoincidence::Ordered(coin->coinPtTStart()->segment(),
                coin->oppPtTStart()->segment()));
        SkASSERT(coin->coinPtTStart()->span()->ptT() == coin->coinPtTStart());
        SkASSERT(coin->coinPtTEnd()->span()->ptT() == coin->coinPtTEnd());
        SkASSERT(coin->oppPtTStart()->span()->ptT() == coin->oppPtTStart());
        SkASSERT(coin->oppPtTEnd()->span()->ptT() == coin->oppPtTEnd());
        DebugValidate(coin->coinPtTStart()->span(), coin->coinPtTEnd()->span(),
                coin->oppPtTStart()->fT, coin->oppPtTEnd()->fT, coin->oppPtTStart()->segment(),
                id, log);
        DebugValidate(coin->oppPtTStart()->span(), coin->oppPtTEnd()->span(),
                coin->coinPtTStart()->fT, coin->coinPtTEnd()->fT, coin->coinPtTStart()->segment(),
                id, log);
        coin = coin->next();
    }
    DebugCheckOverlapTop(head, opt, id, log);
}
#endif

void SkOpCoincidence::debugValidate() const {
#if DEBUG_COINCIDENCE
 //   if (fGlobalState->debugCheckHealth()) {
//        return;
//    }
    DebugValidate(fHead, fTop, nullptr, nullptr);
    DebugValidate(fTop, nullptr, nullptr, nullptr);
#endif
}

#if DEBUG_COINCIDENCE_VERBOSE
void SkOpCoincidence::debugCheckValid(const char* id, SkPathOpsDebug::GlitchLog* log) const {
    DebugValidate(fHead, fTop, id, log);
    DebugValidate(fTop, nullptr, id, log);
}
#endif

#if DEBUG_COINCIDENCE_VERBOSE
void SkOpContour::debugCheckHealth(const char* id, SkPathOpsDebug::GlitchLog* log) const {
    const SkOpSegment* segment = &fHead;
    do {
        segment->debugCheckHealth(id, log);
    } while ((segment = segment->next()));
}

// commmented-out lines keep this aligned with missingCoincidence()
void SkOpContour::debugMissingCoincidence(const char* id, SkPathOpsDebug::GlitchLog* log) const {
//    SkASSERT(fCount > 0);
    const SkOpSegment* segment = &fHead;
//    bool result = false;
    do {
        if (fState->angleCoincidence()) {
// #if DEBUG_ANGLE
//          segment->debugCheckAngleCoin();
// #endif
        } else if (segment->debugMissingCoincidence(id, log), false) {
//          result = true;
// see FIXME in missingCoincidence()
//
//
//
    //      continue;
        }
        segment = segment->next();
    } while (segment);
    return;
}
#endif

#if DEBUG_COINCIDENCE_ORDER
void SkOpSegment::debugResetCoinT() const {
    fDebugBaseIndex = -1;
    fDebugBaseMin = 1;
    fDebugBaseMax = -1;
    fDebugLastIndex = -1;
    fDebugLastMin = 1;
    fDebugLastMax = -1;
}
#endif

void SkOpSegment::debugValidate() const {
#if DEBUG_COINCIDENCE_ORDER
    {
        const SkOpSpanBase* span = &fHead;
        do {
            span->debugResetCoinT();
        } while (!span->final() && (span = span->upCast()->next()));
        span = &fHead;
        int index = 0;
        do {
            span->debugSetCoinT(index++);
        } while (!span->final() && (span = span->upCast()->next()));
    }
#endif
#if DEBUG_COINCIDENCE
    if (this->globalState()->debugCheckHealth()) {
        return;
    }
#endif
#if DEBUG_VALIDATE
    const SkOpSpanBase* span = &fHead;
    double lastT = -1;
    const SkOpSpanBase* prev = nullptr;
    int count = 0;
    int done = 0;
    do {
        if (!span->final()) {
            ++count;
            done += span->upCast()->done() ? 1 : 0;
        }
        SkASSERT(span->segment() == this);
        SkASSERT(!prev || prev->upCast()->next() == span);
        SkASSERT(!prev || prev == span->prev());
        prev = span;
        double t = span->ptT()->fT;
        SkASSERT(lastT < t);
        lastT = t;
        span->debugValidate();
    } while (!span->final() && (span = span->upCast()->next()));
    SkASSERT(count == fCount);
    SkASSERT(done == fDoneCount);
    SkASSERT(count >= fDoneCount);
    SkASSERT(span->final());
    span->debugValidate();
#endif
}

#if DEBUG_COINCIDENCE_VERBOSE

// Commented-out lines keep this in sync with addOpp()
void SkOpSpanBase::debugAddOpp(const char* id, SkPathOpsDebug::GlitchLog* log, const SkOpSpanBase* opp) const {
    const SkOpPtT* oppPrev = this->ptT()->oppPrev(opp->ptT());
    if (!oppPrev) {
        return;
    }
    this->debugMergeMatches(id, log, opp);
    this->ptT()->debugAddOpp(opp->ptT(), oppPrev);
    this->debugCheckForCollapsedCoincidence(id, log);
}

// Commented-out lines keep this in sync with checkForCollapsedCoincidence()
void SkOpSpanBase::debugCheckForCollapsedCoincidence(const char* id, SkPathOpsDebug::GlitchLog* log) const {
    const SkOpCoincidence* coins = this->globalState()->coincidence();
    if (coins->isEmpty()) {
        return;
    }
// the insert above may have put both ends of a coincident run in the same span
// for each coincident ptT in loop; see if its opposite in is also in the loop
// this implementation is the motivation for marking that a ptT is referenced by a coincident span
    const SkOpPtT* head = this->ptT();
    const SkOpPtT* test = head;
    do {
        if (!test->coincident()) {
            continue;
        }
        coins->debugMarkCollapsed(id, log, test);
    } while ((test = test->next()) != head);
}
#endif

bool SkOpSpanBase::debugCoinEndLoopCheck() const {
    int loop = 0;
    const SkOpSpanBase* next = this;
    SkOpSpanBase* nextCoin;
    do {
        nextCoin = next->fCoinEnd;
        SkASSERT(nextCoin == this || nextCoin->fCoinEnd != nextCoin);
        for (int check = 1; check < loop - 1; ++check) {
            const SkOpSpanBase* checkCoin = this->fCoinEnd;
            const SkOpSpanBase* innerCoin = checkCoin;
            for (int inner = check + 1; inner < loop; ++inner) {
                innerCoin = innerCoin->fCoinEnd;
                if (checkCoin == innerCoin) {
                    SkDebugf("*** bad coincident end loop ***\n");
                    return false;
                }
            }
        }
        ++loop;
    } while ((next = nextCoin) && next != this);
    return true;
}

#if DEBUG_COINCIDENCE_VERBOSE
// Commented-out lines keep this in sync with insertCoinEnd()
void SkOpSpanBase::debugInsertCoinEnd(const char* id, SkPathOpsDebug::GlitchLog* log, const SkOpSpanBase* coin) const {
    if (containsCoinEnd(coin)) {
//         SkASSERT(coin->containsCoinEnd(this));
        return;
    }
    debugValidate();
//     SkASSERT(this != coin);
    log->record(kMarkCoinEnd_Glitch, id, this, coin);
//     coin->fCoinEnd = this->fCoinEnd;
//     this->fCoinEnd = coinNext;
    debugValidate();
}

// Commented-out lines keep this in sync with mergeContained()
void SkOpSpanBase::debugMergeContained(const char* id, SkPathOpsDebug::GlitchLog* log, const SkPathOpsBounds& bounds, bool* deleted) const {
    // while adjacent spans' points are contained by the bounds, merge them
    const SkOpSpanBase* prev = this;
    const SkOpSegment* seg = this->segment();
    while ((prev = prev->prev()) && bounds.contains(prev->pt()) && !seg->ptsDisjoint(prev, this)) {
        if (prev->prev()) {
            log->record(kMergeContained_Glitch, id, this, prev);
        } else if (this->final()) {
            log->record(kMergeContained_Glitch, id, this, prev);
            // return;
        } else {
            log->record(kMergeContained_Glitch, id, prev, this);
        }
    }
    const SkOpSpanBase* current = this;
    const SkOpSpanBase* next = this;
    while (next->upCastable() && (next = next->upCast()->next())
            && bounds.contains(next->pt()) && !seg->ptsDisjoint(this, next)) {
        if (!current->prev() && next->final()) {
            log->record(kMergeContained_Glitch, id, next, current);
            current = next;
        }
        if (current->prev()) {
            log->record(kMergeContained_Glitch, id, next, current);
            current = next;
        } else {
            log->record(kMergeContained_Glitch, id, next, current);
            current = next;
        }
    }
#if DEBUG_COINCIDENCE
    // this->globalState()->coincidence()->debugValidate();
#endif
}

// Commented-out lines keep this in sync with mergeMatches()
// Look to see if pt-t linked list contains same segment more than once
// if so, and if each pt-t is directly pointed to by spans in that segment,
// merge them
// keep the points, but remove spans so that the segment doesn't have 2 or more
// spans pointing to the same pt-t loop at different loop elements
void SkOpSpanBase::debugMergeMatches(const char* id, SkPathOpsDebug::GlitchLog* log, const SkOpSpanBase* opp) const {
    const SkOpPtT* test = &fPtT;
    const SkOpPtT* testNext;
    const SkOpPtT* stop = test;
    do {
        testNext = test->next();
        if (test->deleted()) {
            continue;
        }
        const SkOpSpanBase* testBase = test->span();
        SkASSERT(testBase->ptT() == test);
        const SkOpSegment* segment = test->segment();
        if (segment->done()) {
            continue;
        }
        const SkOpPtT* inner = opp->ptT();
        const SkOpPtT* innerStop = inner;
        do {
            if (inner->segment() != segment) {
                continue;
            }
            if (inner->deleted()) {
                continue;
            }
            const SkOpSpanBase* innerBase = inner->span();
            SkASSERT(innerBase->ptT() == inner);
            // when the intersection is first detected, the span base is marked if there are 
            // more than one point in the intersection.
//            if (!innerBase->hasMultipleHint() && !testBase->hasMultipleHint()) {
                if (!zero_or_one(inner->fT)) {
                    log->record(kMergeMatches_Glitch, id, innerBase, test);
                } else {
                    SkASSERT(inner->fT != test->fT);
                    if (!zero_or_one(test->fT)) {
                        log->record(kMergeMatches_Glitch, id, testBase, inner);
                    } else {
                        log->record(kMergeMatches_Glitch, id, segment);
//                        SkDEBUGCODE(testBase->debugSetDeleted());
//                        test->setDeleted();
//                        SkDEBUGCODE(innerBase->debugSetDeleted());
//                        inner->setDeleted();
                    }
                }
#ifdef SK_DEBUG   // assert if another undeleted entry points to segment
                const SkOpPtT* debugInner = inner;
                while ((debugInner = debugInner->next()) != innerStop) {
                    if (debugInner->segment() != segment) {
                        continue;
                    }
                    if (debugInner->deleted()) {
                        continue;
                    }
                    SkOPASSERT(0);
                }
#endif
                break; 
//            }
            break;
        } while ((inner = inner->next()) != innerStop);
    } while ((test = testNext) != stop);
    this->debugCheckForCollapsedCoincidence(id, log);
}

#endif

void SkOpSpanBase::debugResetCoinT() const {
#if DEBUG_COINCIDENCE_ORDER
    const SkOpPtT* ptT = &fPtT;
    do {
        ptT->debugResetCoinT();
        ptT = ptT->next();
    } while (ptT != &fPtT);
#endif
}

void SkOpSpanBase::debugSetCoinT(int index) const {
#if DEBUG_COINCIDENCE_ORDER
    const SkOpPtT* ptT = &fPtT;
    do {
        if (!ptT->deleted()) {
            ptT->debugSetCoinT(index);
        }
        ptT = ptT->next();
    } while (ptT != &fPtT);
#endif
}

const SkOpSpan* SkOpSpanBase::debugStarter(SkOpSpanBase const** endPtr) const {
    const SkOpSpanBase* end = *endPtr;
    SkASSERT(this->segment() == end->segment());
    const SkOpSpanBase* result;
    if (t() < end->t()) {
        result = this;
    } else {
        result = end;
        *endPtr = this;
    }
    return result->upCast();
}

void SkOpSpanBase::debugValidate() const {
#if DEBUG_COINCIDENCE
    if (this->globalState()->debugCheckHealth()) {
        return;
    }
#endif
#if DEBUG_VALIDATE
    const SkOpPtT* ptT = &fPtT;
    SkASSERT(ptT->span() == this);
    do {
//        SkASSERT(SkDPoint::RoughlyEqual(fPtT.fPt, ptT->fPt));
        ptT->debugValidate();
        ptT = ptT->next();
    } while (ptT != &fPtT);
    SkASSERT(this->debugCoinEndLoopCheck());
    if (!this->final()) {
        SkASSERT(this->upCast()->debugCoinLoopCheck());
    }
    if (fFromAngle) {
        fFromAngle->debugValidate();
    }
    if (!this->final() && this->upCast()->toAngle()) {
        this->upCast()->toAngle()->debugValidate();
    }
#endif
}

bool SkOpSpan::debugCoinLoopCheck() const {
    int loop = 0;
    const SkOpSpan* next = this;
    SkOpSpan* nextCoin;
    do {
        nextCoin = next->fCoincident;
        SkASSERT(nextCoin == this || nextCoin->fCoincident != nextCoin);
        for (int check = 1; check < loop - 1; ++check) {
            const SkOpSpan* checkCoin = this->fCoincident;
            const SkOpSpan* innerCoin = checkCoin;
            for (int inner = check + 1; inner < loop; ++inner) {
                innerCoin = innerCoin->fCoincident;
                if (checkCoin == innerCoin) {
                    SkDebugf("*** bad coincident loop ***\n");
                    return false;
                }
            }
        }
        ++loop;
    } while ((next = nextCoin) && next != this);
    return true;
}

#if DEBUG_COINCIDENCE_VERBOSE
// Commented-out lines keep this in sync with insertCoincidence() in header
void SkOpSpan::debugInsertCoincidence(const char* id, SkPathOpsDebug::GlitchLog* log, const SkOpSpan* coin) const {
    if (containsCoincidence(coin)) {
//         SkASSERT(coin->containsCoincidence(this));
        return;
    }
    debugValidate();
//     SkASSERT(this != coin);
    log->record(kMarkCoinStart_Glitch, id, this, coin);
//     coin->fCoincident = this->fCoincident;
//     this->fCoincident = coinNext;
    debugValidate();
}

// Commented-out lines keep this in sync with insertCoincidence()
void SkOpSpan::debugInsertCoincidence(const char* id, SkPathOpsDebug::GlitchLog* log, const SkOpSegment* segment, bool flipped) const {
    if (this->containsCoincidence(segment)) {
        return;
    }
    const SkOpPtT* next = &fPtT;
    while ((next = next->next()) != &fPtT) {
        if (next->segment() == segment) {
            log->record(kMarkCoinInsert_Glitch, id, flipped ? next->span()->prev() : next->span());
            return;
        }
    }
#if DEBUG_COINCIDENCE
    log->record(kMarkCoinMissing_Glitch, id, segment, this);
#endif
}
#endif

// called only by test code
int SkIntersections::debugCoincidentUsed() const {
    if (!fIsCoincident[0]) {
        SkASSERT(!fIsCoincident[1]);
        return 0;
    }
    int count = 0;
    SkDEBUGCODE(int count2 = 0;)
    for (int index = 0; index < fUsed; ++index) {
        if (fIsCoincident[0] & (1 << index)) {
            ++count;
        }
#ifdef SK_DEBUG
        if (fIsCoincident[1] & (1 << index)) {
            ++count2;
        }
#endif
    }
    SkASSERT(count == count2);
    return count;
}

#include "SkOpContour.h"

// Commented-out lines keep this in sync with addOpp()
void SkOpPtT::debugAddOpp(const SkOpPtT* opp, const SkOpPtT* oppPrev) const {
    SkDEBUGCODE(const SkOpPtT* oldNext = this->fNext);
    SkASSERT(this != opp);
//    this->fNext = opp;
    SkASSERT(oppPrev != oldNext);
//    oppPrev->fNext = oldNext;
}

bool SkOpPtT::debugContains(const SkOpPtT* check) const {
    SkASSERT(this != check);
    const SkOpPtT* ptT = this;
    int links = 0;
    do {
        ptT = ptT->next();
        if (ptT == check) {
            return true;
        }
        ++links;
        const SkOpPtT* test = this;
        for (int index = 0; index < links; ++index) {
            if (ptT == test) {
                return false;
            }
            test = test->next();
        }
    } while (true);
}

const SkOpPtT* SkOpPtT::debugContains(const SkOpSegment* check) const {
    SkASSERT(this->segment() != check);
    const SkOpPtT* ptT = this;
    int links = 0;
    do {
        ptT = ptT->next();
        if (ptT->segment() == check) {
            return ptT;
        }
        ++links;
        const SkOpPtT* test = this;
        for (int index = 0; index < links; ++index) {
            if (ptT == test) {
                return nullptr;
            }
            test = test->next();
        }
    } while (true);
}

const SkOpPtT* SkOpPtT::debugEnder(const SkOpPtT* end) const {
    return fT < end->fT ? end : this;
}

int SkOpPtT::debugLoopLimit(bool report) const {
    int loop = 0;
    const SkOpPtT* next = this;
    do {
        for (int check = 1; check < loop - 1; ++check) {
            const SkOpPtT* checkPtT = this->fNext;
            const SkOpPtT* innerPtT = checkPtT;
            for (int inner = check + 1; inner < loop; ++inner) {
                innerPtT = innerPtT->fNext;
                if (checkPtT == innerPtT) {
                    if (report) {
                        SkDebugf("*** bad ptT loop ***\n");
                    }
                    return loop;
                }
            }
        }
        // there's nothing wrong with extremely large loop counts -- but this may appear to hang
        // by taking a very long time to figure out that no loop entry is a duplicate
        // -- and it's likely that a large loop count is indicative of a bug somewhere
        if (++loop > 1000) {
            SkDebugf("*** loop count exceeds 1000 ***\n");
            return 1000;
        }
    } while ((next = next->fNext) && next != this);
    return 0;
}

const SkOpPtT* SkOpPtT::debugOppPrev(const SkOpPtT* opp) const {
    return this->oppPrev(const_cast<SkOpPtT*>(opp));
}

void SkOpPtT::debugResetCoinT() const {
#if DEBUG_COINCIDENCE_ORDER
    this->segment()->debugResetCoinT(); 
#endif
}

void SkOpPtT::debugSetCoinT(int index) const {
#if DEBUG_COINCIDENCE_ORDER
    this->segment()->debugSetCoinT(index, fT); 
#endif
}

void SkOpPtT::debugValidate() const {
#if DEBUG_COINCIDENCE
    if (this->globalState()->debugCheckHealth()) {
        return;
    }
#endif
#if DEBUG_VALIDATE
    SkOpGlobalState::Phase phase = contour()->globalState()->phase();
    if (phase == SkOpGlobalState::kIntersecting
            || phase == SkOpGlobalState::kFixWinding) {
        return;
    }
    SkASSERT(fNext);
    SkASSERT(fNext != this);
    SkASSERT(fNext->fNext);
    SkASSERT(debugLoopLimit(false) == 0);
#endif
}

static void output_scalar(SkScalar num) {
    if (num == (int) num) {
        SkDebugf("%d", (int) num);
    } else {
        SkString str;
        str.printf("%1.9g", num);
        int width = (int) str.size();
        const char* cStr = str.c_str();
        while (cStr[width - 1] == '0') {
            --width;
        }
        str.resize(width);
        SkDebugf("%sf", str.c_str());
    }
}

static void output_points(const SkPoint* pts, int count) {
    for (int index = 0; index < count; ++index) {
        output_scalar(pts[index].fX);
        SkDebugf(", ");
        output_scalar(pts[index].fY);
        if (index + 1 < count) {
            SkDebugf(", ");
        }
    }
}

static void showPathContours(SkPath::RawIter& iter, const char* pathName) {
    uint8_t verb;
    SkPoint pts[4];
    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kMove_Verb:
                SkDebugf("    %s.moveTo(", pathName);
                output_points(&pts[0], 1);
                SkDebugf(");\n");
                continue;
            case SkPath::kLine_Verb:
                SkDebugf("    %s.lineTo(", pathName);
                output_points(&pts[1], 1);
                SkDebugf(");\n");
                break;
            case SkPath::kQuad_Verb:
                SkDebugf("    %s.quadTo(", pathName);
                output_points(&pts[1], 2);
                SkDebugf(");\n");
                break;
            case SkPath::kConic_Verb:
                SkDebugf("    %s.conicTo(", pathName);
                output_points(&pts[1], 2);
                SkDebugf(", %1.9gf);\n", iter.conicWeight());
                break;
            case SkPath::kCubic_Verb:
                SkDebugf("    %s.cubicTo(", pathName);
                output_points(&pts[1], 3);
                SkDebugf(");\n");
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

void SkPathOpsDebug::ShowOnePath(const SkPath& path, const char* name, bool includeDeclaration) {
    SkPath::RawIter iter(path);
#define SUPPORT_RECT_CONTOUR_DETECTION 0
#if SUPPORT_RECT_CONTOUR_DETECTION
    int rectCount = path.isRectContours() ? path.rectContours(nullptr, nullptr) : 0;
    if (rectCount > 0) {
        SkTDArray<SkRect> rects;
        SkTDArray<SkPath::Direction> directions;
        rects.setCount(rectCount);
        directions.setCount(rectCount);
        path.rectContours(rects.begin(), directions.begin());
        for (int contour = 0; contour < rectCount; ++contour) {
            const SkRect& rect = rects[contour];
            SkDebugf("path.addRect(%1.9g, %1.9g, %1.9g, %1.9g, %s);\n", rect.fLeft, rect.fTop,
                    rect.fRight, rect.fBottom, directions[contour] == SkPath::kCCW_Direction
                    ? "SkPath::kCCW_Direction" : "SkPath::kCW_Direction");
        }
        return;
    }
#endif
    SkPath::FillType fillType = path.getFillType();
    SkASSERT(fillType >= SkPath::kWinding_FillType && fillType <= SkPath::kInverseEvenOdd_FillType);
    if (includeDeclaration) {
        SkDebugf("    SkPath %s;\n", name);
    }
    SkDebugf("    %s.setFillType(SkPath::%s);\n", name, gFillTypeStr[fillType]);
    iter.setPath(path);
    showPathContours(iter, name);
}
