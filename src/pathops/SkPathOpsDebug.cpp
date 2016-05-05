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

struct SkCoincidentSpans;

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

#if DEBUG_COINCIDENCE
enum GlitchType {
    kAddCorruptCoin_Glitch,
    kAddExpandedCoin_Glitch,
    kAddMissingCoin_Glitch,
    kCollapsedCoin_Glitch,
    kCollapsedDone_Glitch,
    kCollapsedOppValue_Glitch,
    kCollapsedSpan_Glitch,
    kCollapsedWindValue_Glitch,
    kDeletedCoin_Glitch,
    kExpandCoin_Glitch,
    kMarkCoinEnd_Glitch,
    kMarkCoinInsert_Glitch,
    kMissingCoin_Glitch,
    kMissingDone_Glitch,
    kMissingIntersection_Glitch,
    kMoveMultiple_Glitch,
    kUnaligned_Glitch,
    kUnalignedHead_Glitch,
    kUnalignedTail_Glitch,
    kUndetachedSpan_Glitch,
    kUnmergedSpan_Glitch,
};

static const int kGlitchType_Count = kUnmergedSpan_Glitch + 1;

struct SpanGlitch {
    const char* fStage;
    const SkOpSpanBase* fBase;
    const SkOpSpanBase* fSuspect;
    const SkCoincidentSpans* fCoin;
    const SkOpSegment* fSegment;
    const SkOpPtT* fCoinSpan;
    const SkOpPtT* fEndSpan;
    const SkOpPtT* fOppSpan;
    const SkOpPtT* fOppEndSpan;
    double fT;
    SkPoint fPt;
    GlitchType fType;
};

struct SkPathOpsDebug::GlitchLog {
    SpanGlitch* recordCommon(GlitchType type, const char* stage) {
        SpanGlitch* glitch = fGlitches.push();
        glitch->fStage = stage;
        glitch->fBase = nullptr;
        glitch->fSuspect = nullptr;
        glitch->fCoin = nullptr;
        glitch->fSegment = nullptr;
        glitch->fCoinSpan = nullptr;
        glitch->fEndSpan = nullptr;
        glitch->fOppSpan = nullptr;
        glitch->fOppEndSpan = nullptr;
        glitch->fT = SK_ScalarNaN;
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

    void record(GlitchType type, const char* stage, const SkCoincidentSpans* coin,
            const SkOpPtT* coinSpan) {
        SpanGlitch* glitch = recordCommon(type, stage);
        glitch->fCoin = coin;
        glitch->fCoinSpan = coinSpan;
    }

    void record(GlitchType type, const char* stage, const SkOpSpanBase* base,
            const SkOpSegment* seg, double t, SkPoint pt) {
        SpanGlitch* glitch = recordCommon(type, stage);
        glitch->fBase = base;
        glitch->fSegment = seg;
        glitch->fT = t;
        glitch->fPt = pt;
    }

    void record(GlitchType type, const char* stage, const SkOpSpanBase* base, double t,
            SkPoint pt) {
        SpanGlitch* glitch = recordCommon(type, stage);
        glitch->fBase = base;
        glitch->fT = t;
        glitch->fPt = pt;
    }

    void record(GlitchType type, const char* stage, const SkCoincidentSpans* coin,
            const SkOpPtT* coinSpan, const SkOpPtT* endSpan) {
        SpanGlitch* glitch = recordCommon(type, stage);
        glitch->fCoin = coin;
        glitch->fCoinSpan = coinSpan;
        glitch->fEndSpan = endSpan;
    }

    void record(GlitchType type, const char* stage, const SkCoincidentSpans* coin,
            const SkOpSpanBase* suspect) {
        SpanGlitch* glitch = recordCommon(type, stage);
        glitch->fSuspect = suspect;
        glitch->fCoin = coin;
    }

    void record(GlitchType type, const char* stage, const SkOpPtT* ptTS, const SkOpPtT* ptTE,
            const SkOpPtT* oPtTS, const SkOpPtT* oPtTE) {
        SpanGlitch* glitch = recordCommon(type, stage);
        glitch->fCoinSpan = ptTS;
        glitch->fEndSpan = ptTE;
        glitch->fOppSpan = oPtTS;
        glitch->fOppEndSpan = oPtTE;
    }

    SkTDArray<SpanGlitch> fGlitches;
};

void SkPathOpsDebug::CheckHealth(SkOpContourHead* contourList, const char* id) {
    GlitchLog glitches;
    const SkOpContour* contour = contourList;
    const SkOpCoincidence* coincidence = contour->globalState()->coincidence();
    do {
        contour->debugCheckHealth(id, &glitches);
        contour->debugMissingCoincidence(id, &glitches, coincidence);
    } while ((contour = contour->next()));
    coincidence->debugFixAligned(id, &glitches);
    coincidence->debugAddMissing(id, &glitches);
    coincidence->debugExpand(id, &glitches);
    coincidence->debugAddExpanded(id, &glitches);
    coincidence->debugMark(id, &glitches);
    unsigned mask = 0;
    for (int index = 0; index < glitches.fGlitches.count(); ++index) {
        const SpanGlitch& glitch = glitches.fGlitches[index];
        mask |= 1 << glitch.fType;
    }
    for (int index = 0; index < kGlitchType_Count; ++index) {
        SkDebugf(mask & (1 << index) ? "x" : "-");
    }
    SkDebugf("  %s\n", id);
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
    "kXor_PathOp",
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

#if DEBUG_COINCIDENCE
void SkOpSegment::debugAddAlignIntersection(const char* id, SkPathOpsDebug::GlitchLog* log,
        const SkOpPtT& endPtT, const SkPoint& oldPt,  const SkOpContourHead* contourList) const {
    const SkPoint& newPt = endPtT.fPt;
    if (newPt == oldPt) {
        return;
    }
    SkPoint line[2] = { newPt, oldPt };
    SkPathOpsBounds lineBounds;
    lineBounds.setBounds(line, 2);
    SkDLine aLine;
    aLine.set(line);
    const SkOpContour* current = contourList;
    do {
        if (!SkPathOpsBounds::Intersects(current->bounds(), lineBounds)) {
            continue;
        }
        const SkOpSegment* segment = current->first();
        do {
            if (!SkPathOpsBounds::Intersects(segment->bounds(), lineBounds)) {
                continue;
            }
            if (newPt == segment->fPts[0]) {
                continue;
            }
            if (newPt == segment->fPts[SkPathOpsVerbToPoints(segment->fVerb)]) {
                continue;
            }
            if (oldPt == segment->fPts[0]) {
                continue;
            }
            if (oldPt == segment->fPts[SkPathOpsVerbToPoints(segment->fVerb)]) {
                continue;
            }
            if (endPtT.debugContains(segment)) {
                continue;
            }
            SkIntersections i;
            switch (segment->fVerb) {
                case SkPath::kLine_Verb: {
                    SkDLine bLine;
                    bLine.set(segment->fPts);
                    i.intersect(bLine, aLine);
                    } break;
                case SkPath::kQuad_Verb: {
                    SkDQuad bQuad;
                    bQuad.set(segment->fPts);
                    i.intersect(bQuad, aLine);
                    } break;
                case SkPath::kConic_Verb: {
                    SkDConic bConic;
                    bConic.set(segment->fPts, segment->fWeight);
                    i.intersect(bConic, aLine);
                    } break;
                case SkPath::kCubic_Verb: {
                    SkDCubic bCubic;
                    bCubic.set(segment->fPts);
                    i.intersect(bCubic, aLine);
                    } break;
                default:
                    SkASSERT(0);
            }
            if (i.used()) {
                SkASSERT(i.used() == 1);
                SkASSERT(!zero_or_one(i[0][0]));
                SkOpSpanBase* checkSpan = fHead.next();
                while (!checkSpan->final()) {
                    if (checkSpan->contains(segment)) {
                        goto nextSegment;
                    }
                    checkSpan = checkSpan->upCast()->next();
                }
                log->record(kMissingIntersection_Glitch, id, checkSpan, segment, i[0][0], newPt);
            }
    nextSegment:
            ;
        } while ((segment = segment->next()));
    } while ((current = current->next()));
}

bool SkOpSegment::debugAddMissing(double t, const SkOpSegment* opp) const {
    const SkOpSpanBase* existing = nullptr;
    const SkOpSpanBase* test = &fHead;
    double testT;
    do {
        if ((testT = test->ptT()->fT) >= t) {
            if (testT == t) {
                existing = test;
            }
            break;
        }
    } while ((test = test->upCast()->next()));
    return !existing || !existing->debugContains(opp);
}

void SkOpSegment::debugAlign(const char* id, SkPathOpsDebug::GlitchLog* glitches) const {
    const SkOpSpanBase* span = &fHead;
    if (!span->aligned()) {
        if (!span->debugAlignedEnd(0, fPts[0])) {
            glitches->record(kUnalignedHead_Glitch, id, span);
        }
    }
    while ((span = span->upCast()->next())) {
        if (span == &fTail) {
            break;
        }
        if (!span->aligned()) {
            glitches->record(kUnaligned_Glitch, id, span);
        }
    }
    if (!span->aligned()) {
        span->debugAlignedEnd(1, fPts[SkPathOpsVerbToPoints(fVerb)]);
    }
    if (this->collapsed()) {
        const SkOpSpan* span = &fHead;
        do {
            if (span->windValue()) {
                glitches->record(kCollapsedWindValue_Glitch, id, span);
            }
            if (span->oppValue()) {
                glitches->record(kCollapsedOppValue_Glitch, id, span);
            }
            if (!span->done()) {
                glitches->record(kCollapsedDone_Glitch, id, span);
            }
        } while ((span = span->next()->upCastable()));
    }
}
#endif

#if DEBUG_ANGLE
void SkOpSegment::debugCheckAngleCoin() const {
    const SkOpSpanBase* base = &fHead;
    const SkOpSpan* span;
    do {
        const SkOpAngle* angle = base->fromAngle();
        if (angle && angle->fCheckCoincidence) {
            angle->debugCheckNearCoincidence();
        }
        if (base->final()) {
             break;
        }
        span = base->upCast();
        angle = span->toAngle();
        if (angle && angle->fCheckCoincidence) {
            angle->debugCheckNearCoincidence();
        }
    } while ((base = span->next()));
}
#endif

#if DEBUG_COINCIDENCE
// this mimics the order of the checks in handle coincidence
void SkOpSegment::debugCheckHealth(const char* id, SkPathOpsDebug::GlitchLog* glitches) const {
    debugMoveMultiples(id, glitches);
    debugFindCollapsed(id, glitches);
    debugMoveNearby(id, glitches);
    debugAlign(id, glitches);
    debugAddAlignIntersections(id, glitches, this->globalState()->contourHead());

}

void SkOpSegment::debugFindCollapsed(const char* id, SkPathOpsDebug::GlitchLog* glitches) const {
    if (fHead.contains(&fTail)) {
        const SkOpSpan* span = this->head();
        bool missingDone = false;
        do {
            missingDone |= !span->done();
        } while ((span = span->next()->upCastable()));
        if (missingDone) {
            glitches->record(kMissingDone_Glitch, id, &fHead);
        }
        if (!fHead.debugAlignedEnd(0, fHead.pt())) {
            glitches->record(kUnalignedHead_Glitch, id, &fHead);
        }
        if (!fTail.aligned()) {
            glitches->record(kUnalignedTail_Glitch, id, &fTail);
        }
    }
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
void SkOpSegment::debugMissingCoincidence(const char* id, SkPathOpsDebug::GlitchLog* log,
        const SkOpCoincidence* coincidences) const {
    if (this->verb() != SkPath::kLine_Verb) {
        return;
    }
    if (this->done()) {
        return;
    }
    const SkOpSpan* prior = nullptr;
    const SkOpSpanBase* spanBase = &fHead;
    do {
        const SkOpPtT* ptT = spanBase->ptT(), * spanStopPtT = ptT;
        SkASSERT(ptT->span() == spanBase);
        while ((ptT = ptT->next()) != spanStopPtT) {
            if (ptT->deleted()) {
                continue;
            }
            SkOpSegment* opp = ptT->span()->segment();
//            if (opp->verb() == SkPath::kLine_Verb) {
//                continue;
//            }
            if (opp->done()) {
                continue;
            }
            // when opp is encounted the 1st time, continue; on 2nd encounter, look for coincidence
            if (!opp->visited()) {
                continue;
            }
            if (spanBase == &fHead) {
                continue;
            }
            const SkOpSpan* span = spanBase->upCastable();
            // FIXME?: this assumes that if the opposite segment is coincident then no more
            // coincidence needs to be detected. This may not be true.
            if (span && span->segment() != opp && span->containsCoincidence(opp)) {
                continue;
            }
            if (spanBase->segment() != opp && spanBase->containsCoinEnd(opp)) {
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
            const SkOpPtT* oppStart = prior->ptT();
            const SkOpPtT* oppEnd = spanBase->ptT();
            bool swapped = priorPtT->fT > ptT->fT;
            if (swapped) {
                SkTSwap(priorPtT, ptT);
                SkTSwap(oppStart, oppEnd);
            }
            bool flipped = oppStart->fT > oppEnd->fT;
            bool coincident = false;
            if (coincidences->contains(priorPtT, ptT, oppStart, oppEnd, flipped)) {
                goto swapBack;
            }
            if (opp->verb() == SkPath::kLine_Verb) {
                coincident = (SkDPoint::ApproximatelyEqual(priorPtT->fPt, oppStart->fPt) ||
                        SkDPoint::ApproximatelyEqual(priorPtT->fPt, oppEnd->fPt)) &&
                        (SkDPoint::ApproximatelyEqual(ptT->fPt, oppStart->fPt) ||
                        SkDPoint::ApproximatelyEqual(ptT->fPt, oppEnd->fPt));
            }
            if (!coincident) {
                coincident = testForCoincidence(priorPtT, ptT, prior, spanBase, opp, 5000);
            }
            if (coincident) {
                log->record(kMissingCoin_Glitch, id, priorPtT, ptT, oppStart, oppEnd);
            }
    swapBack:
            if (swapped) {
                SkTSwap(priorPtT, ptT);
            }
        }
    } while ((spanBase = spanBase->final() ? nullptr : spanBase->upCast()->next()));
}

void SkOpSegment::debugMoveMultiples(const char* id, SkPathOpsDebug::GlitchLog* glitches) const {
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
                    if (oppTest == &oppSegment->fTail || oppTest == &oppSegment->fHead) {
                        SkASSERT(oppSpan != &oppSegment->fHead); // don't expect collapse
                        SkASSERT(oppSpan != &oppSegment->fTail);
                        glitches->record(kMoveMultiple_Glitch, id, oppTest, oppSpan);
                    } else {
                        glitches->record(kMoveMultiple_Glitch, id, oppSpan, oppTest);
                    }
                    goto checkNextSpan;
                }
        tryNextSpan: 
                ;
            } while (oppTest != oppLast && (oppTest = oppTest->upCast()->next()));
        } while ((testPtT = testPtT->next()) != startPtT);
checkNextSpan: 
        ;
    } while ((test = test->final() ? nullptr : test->upCast()->next()));
}

void SkOpSegment::debugMoveNearby(const char* id, SkPathOpsDebug::GlitchLog* glitches) const {
        const SkOpSpanBase* spanS = &fHead;
    do {
        const SkOpSpanBase* test = spanS->upCast()->next();
        const SkOpSpanBase* next;
        if (spanS->contains(test)) {
            if (!test->final()) {
                glitches->record(kUndetachedSpan_Glitch, id, test, spanS);
            } else if (spanS != &fHead) {
                glitches->record(kUndetachedSpan_Glitch, id, spanS, test);
            }
        }
        do {  // iterate through all spans associated with start
            const SkOpPtT* startBase = spanS->ptT();
            next = test->final() ? nullptr : test->upCast()->next();
            do {
                const SkOpPtT* testBase = test->ptT();
                do {
                    if (startBase == testBase) {
                        goto checkNextSpan;
                    }
                    if (testBase->duplicate()) {
                        continue;
                    }
                    if (this->match(startBase, testBase->segment(), testBase->fT, testBase->fPt)) {
                        if (test == &this->fTail) {
                            if (spanS == &fHead) {
                                glitches->record(kCollapsedSpan_Glitch, id, spanS);
                            } else {
                                glitches->record(kUnmergedSpan_Glitch, id, &this->fTail, spanS);
                            }
                        } else {
                            glitches->record(kUnmergedSpan_Glitch, id, spanS, test);
                            goto checkNextSpan;
                        }
                    }
                } while ((testBase = testBase->next()) != test->ptT());
            } while ((startBase = startBase->next()) != spanS->ptT());
    checkNextSpan:
            ;
        } while ((test = next));
        spanS = spanS->upCast()->next();
    } while (!spanS->final());
}
#endif

void SkOpSegment::debugReset() {
    this->init(this->fPts, this->fWeight, this->contour(), this->verb());
}

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
        SkDebugf(" (%1.9g,%1.9g", fPts[0].fX, fPts[0].fY);
        for (int vIndex = 1; vIndex <= SkPathOpsVerbToPoints(fVerb); ++vIndex) {
            SkDebugf(" %1.9g,%1.9g", fPts[vIndex].fX, fPts[vIndex].fY);
        }
        if (SkPath::kConic_Verb == fVerb) {
            SkDebugf(" %1.9gf", fWeight);
        }
        const SkOpPtT* ptT = span->ptT();
        SkDebugf(") t=%1.9g (%1.9g,%1.9g)", ptT->fT, ptT->fPt.fX, ptT->fPt.fY);
        SkDebugf(" tEnd=%1.9g", span->next()->t());
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
            result.printf(LINE_DEBUG_STR " id=%d", LINE_DEBUG_DATA(fCurvePart),
                    this->segment()->debugID());
            break;
        case SkPath::kQuad_Verb:
            result.printf(QUAD_DEBUG_STR " id=%d", QUAD_DEBUG_DATA(fCurvePart),
                    this->segment()->debugID());
            break;
        case SkPath::kConic_Verb:
            result.printf(CONIC_DEBUG_STR " id=%d",
                    CONIC_DEBUG_DATA(fCurvePart, fCurvePart.fConic.fWeight),
                    this->segment()->debugID());
            break;
        case SkPath::kCubic_Verb:
            result.printf(CUBIC_DEBUG_STR " id=%d", CUBIC_DEBUG_DATA(fCurvePart),
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


#if DEBUG_COINCIDENCE
void SkOpCoincidence::debugAddExpanded(const char* id, SkPathOpsDebug::GlitchLog* log) const {
    // for each coincident pair, match the spans
    // if the spans don't match, add the mssing pt to the segment and loop it in the opposite span
    const SkCoincidentSpans* coin = this->fHead;
    if (!coin) {
        coin = this->fTop;
    }
    if (!coin) {
        return;
    }
    do {
        const SkOpPtT* startPtT = coin->fCoinPtTStart;
        const SkOpPtT* oStartPtT = coin->fOppPtTStart;
        SkASSERT(startPtT->contains(oStartPtT));
        SkASSERT(coin->fCoinPtTEnd->contains(coin->fOppPtTEnd));
        const SkOpSpanBase* start = startPtT->span();
        const SkOpSpanBase* oStart = oStartPtT->span();
        const SkOpSpanBase* end = coin->fCoinPtTEnd->span();
        const SkOpSpanBase* oEnd = coin->fOppPtTEnd->span();
        const SkOpSpanBase* test = start->upCast()->next();
        const SkOpSpanBase* oTest = coin->fFlipped ? oStart->prev() : oStart->upCast()->next();
        while (test != end || oTest != oEnd) {
            bool bumpTest = true;
            bool bumpOTest = true;
            if (!test->ptT()->contains(oTest->ptT())) {
                // use t ranges to guess which one is missing
                double startRange = coin->fCoinPtTEnd->fT - startPtT->fT;
                double startPart = (test->t() - startPtT->fT) / startRange;
                double oStartRange = coin->fOppPtTEnd->fT - oStartPtT->fT;
                double oStartPart = (oTest->t() - oStartPtT->fT) / oStartRange;
                if (startPart == oStartPart) {
                    // data is corrupt
                    log->record(kAddCorruptCoin_Glitch, id, start, oStart);
                    break;
                }
                if (startPart < oStartPart) {
                    double newT = oStartPtT->fT + oStartRange * startPart;
                    log->record(kAddExpandedCoin_Glitch, id, oStart, newT, test->pt());
                    bumpOTest = false;
                } else {
                    double newT = startPtT->fT + startRange * oStartPart;
                    log->record(kAddExpandedCoin_Glitch, id, start, newT, oTest->pt());
                    bumpTest = false;
                }
            }
            if (bumpTest && test != end) {
                test = test->upCast()->next();
            }
            if (bumpOTest && oTest != oEnd) {
                oTest = coin->fFlipped ? oTest->prev() : oTest->upCast()->next();
            }
        }
    } while ((coin = coin->fNext));
}

static void t_range(const SkOpPtT* overS, const SkOpPtT* overE, double tStart, double tEnd,
        const SkOpPtT* coinPtTStart, const SkOpPtT* coinPtTEnd, double* coinTs, double* coinTe) {
    double denom = overE->fT - overS->fT;
    double start = 0 < denom ? tStart : tEnd;
    double end = 0 < denom ? tEnd : tStart;
    double sRatio = (start - overS->fT) / denom;
    double eRatio = (end - overS->fT) / denom;
    *coinTs = coinPtTStart->fT + (coinPtTEnd->fT - coinPtTStart->fT) * sRatio;
    *coinTe = coinPtTStart->fT + (coinPtTEnd->fT - coinPtTStart->fT) * eRatio;
}

bool SkOpCoincidence::debugAddIfMissing(const SkCoincidentSpans* outer, const SkOpPtT* over1s,
            const SkOpPtT* over1e) const {
    const SkCoincidentSpans* check = this->fTop;
    while (check) {
        if (check->fCoinPtTStart->span() == over1s->span()
                && check->fOppPtTStart->span() == outer->fOppPtTStart->span()) {
            SkASSERT(check->fCoinPtTEnd->span() == over1e->span()
                    || !fDebugState->debugRunFail());
            SkASSERT(check->fOppPtTEnd->span() == outer->fOppPtTEnd->span()
                    || !fDebugState->debugRunFail());
            return false;
        }
        if (check->fCoinPtTStart->span() == outer->fCoinPtTStart->span()
                && check->fOppPtTStart->span() == over1s->span()) {
            SkASSERT(check->fCoinPtTEnd->span() == outer->fCoinPtTEnd->span()
                    || !fDebugState->debugRunFail());
            SkASSERT(check->fOppPtTEnd->span() == over1e->span()
                    || !fDebugState->debugRunFail());
            return false;
        }
        check = check->fNext;
    }
    return true;
}

bool SkOpCoincidence::debugAddIfMissing(const SkOpPtT* over1s, const SkOpPtT* over1e,
                      const SkOpPtT* over2s, const SkOpPtT* over2e, double tStart, double tEnd,
        SkOpPtT* coinPtTStart, const SkOpPtT* coinPtTEnd,
        SkOpPtT* oppPtTStart, const SkOpPtT* oppPtTEnd) const {
    double coinTs, coinTe, oppTs, oppTe;
    t_range(over1s, over1e, tStart, tEnd, coinPtTStart, coinPtTEnd, &coinTs, &coinTe);
    t_range(over2s, over2e, tStart, tEnd, oppPtTStart, oppPtTEnd, &oppTs, &oppTe);
    const SkOpSegment* coinSeg = coinPtTStart->segment();
    const SkOpSegment* oppSeg = oppPtTStart->segment();
    SkASSERT(coinSeg != oppSeg);
    const SkCoincidentSpans* check = this->fTop;
    ;
    while (check) {
        const SkOpSegment* checkCoinSeg = check->fCoinPtTStart->segment();
        const SkOpSegment* checkOppSeg;
        if (checkCoinSeg != coinSeg && checkCoinSeg != oppSeg) {
            goto next;
        }
        checkOppSeg = check->fOppPtTStart->segment();
        if (checkOppSeg != coinSeg && checkOppSeg != oppSeg) {
            goto next;
        }
        {
            int cTs = coinTs;
            int cTe = coinTe;
            int oTs = oppTs;
            int oTe = oppTe;
            if (checkCoinSeg != coinSeg) {
                SkASSERT(checkOppSeg != oppSeg);
                SkTSwap(cTs, oTs);
                SkTSwap(cTe, oTe);
            }
            int tweenCount = (int) between(check->fCoinPtTStart->fT, cTs, check->fCoinPtTEnd->fT)
                           + (int) between(check->fCoinPtTStart->fT, cTe, check->fCoinPtTEnd->fT)
                           + (int) between(check->fOppPtTStart->fT, oTs, check->fOppPtTEnd->fT)
                           + (int) between(check->fOppPtTStart->fT, oTe, check->fOppPtTEnd->fT);
    //        SkASSERT(tweenCount == 0 || tweenCount == 4);
            if (tweenCount) {
                return true;
            }
        }
next:
        check = check->fNext;
    }
    if ((over1s->fT < over1e->fT) != (over2s->fT < over2e->fT)) {
        SkTSwap(oppTs, oppTe);
    }
    if (coinTs > coinTe) {
        SkTSwap(coinTs, coinTe);
        SkTSwap(oppTs, oppTe);
    }
    bool cs = coinSeg->debugAddMissing(coinTs, oppSeg);
    bool ce = coinSeg->debugAddMissing(coinTe, oppSeg);
    if (cs == ce) {
        return false;
    }
    return true;
}

void SkOpCoincidence::debugAddMissing(const char* id, SkPathOpsDebug::GlitchLog* log) const {
    const SkCoincidentSpans* outer = fHead;
    if (!outer) {
        return;
    }
    do {
    // addifmissing can modify the list that this is walking
    // save head so that walker can iterate over old data unperturbed
    // addifmissing adds to head freely then add saved head in the end
        const SkOpSegment* outerCoin = outer->fCoinPtTStart->segment();
        SkASSERT(outerCoin == outer->fCoinPtTEnd->segment());
        const SkOpSegment* outerOpp = outer->fOppPtTStart->segment();
        SkASSERT(outerOpp == outer->fOppPtTEnd->segment());
        const SkCoincidentSpans* inner = outer;
        while ((inner = inner->fNext)) {
            double overS, overE;
            const SkOpSegment* innerCoin = inner->fCoinPtTStart->segment();
            SkASSERT(innerCoin == inner->fCoinPtTEnd->segment());
            const SkOpSegment* innerOpp = inner->fOppPtTStart->segment();
            SkASSERT(innerOpp == inner->fOppPtTEnd->segment());
            if (outerCoin == innerCoin
                    && this->overlap(outer->fCoinPtTStart, outer->fCoinPtTEnd,
                    inner->fCoinPtTStart, inner->fCoinPtTEnd, &overS, &overE)) {
                if (this->debugAddIfMissing(outer->fCoinPtTStart, outer->fCoinPtTEnd,
                        inner->fCoinPtTStart, inner->fCoinPtTEnd, overS, overE,
                        outer->fOppPtTStart, outer->fOppPtTEnd,
                        inner->fOppPtTStart, inner->fOppPtTEnd)) {
                    log->record(kAddMissingCoin_Glitch, id, outer, inner->fCoinPtTStart);
                }
            } else if (outerCoin == innerOpp
                    && this->overlap(outer->fCoinPtTStart, outer->fCoinPtTEnd,
                    inner->fOppPtTStart, inner->fOppPtTEnd, &overS, &overE)) {
                if (this->debugAddIfMissing(outer->fCoinPtTStart, outer->fCoinPtTEnd,
                        inner->fOppPtTStart, inner->fOppPtTEnd, overS, overE,
                        outer->fOppPtTStart, outer->fOppPtTEnd,
                        inner->fCoinPtTStart, inner->fCoinPtTEnd)) {
                    log->record(kAddMissingCoin_Glitch, id, outer, inner->fOppPtTStart);
                }
            } else if (outerOpp == innerCoin
                    && this->overlap(outer->fOppPtTStart, outer->fOppPtTEnd,
                    inner->fCoinPtTStart, inner->fCoinPtTEnd, &overS, &overE)) {
                if (this->debugAddIfMissing(outer->fOppPtTStart, outer->fOppPtTEnd,
                        inner->fCoinPtTStart, inner->fCoinPtTEnd, overS, overE,
                        outer->fCoinPtTStart, outer->fCoinPtTEnd,
                        inner->fOppPtTStart, inner->fOppPtTEnd)) {
                    log->record(kAddMissingCoin_Glitch, id, outer, inner->fCoinPtTStart);
                }
            } else if (outerOpp == innerOpp
                    && this->overlap(outer->fOppPtTStart, outer->fOppPtTEnd,
                    inner->fOppPtTStart, inner->fOppPtTEnd, &overS, &overE)) {
                if (this->debugAddIfMissing(outer->fOppPtTStart, outer->fOppPtTEnd,
                        inner->fOppPtTStart, inner->fOppPtTEnd, overS, overE,
                        outer->fCoinPtTStart, outer->fCoinPtTEnd,
                        inner->fCoinPtTStart, inner->fCoinPtTEnd)) {
                    log->record(kAddMissingCoin_Glitch, id, outer, inner->fOppPtTStart);
                }
            } else if (outerCoin != innerCoin) {
                // check to see if outer span overlaps the inner span
                    // look for inner segment in pt-t list
                    // if present, and if t values are in coincident range
                    // add two pairs of new coincidence
                const SkOpPtT* testS = outer->fCoinPtTStart->debugContains(innerCoin);
                const SkOpPtT* testE = outer->fCoinPtTEnd->debugContains(innerCoin);
                if (testS && testS->fT >= inner->fCoinPtTStart->fT
                        && testE && testE->fT <= inner->fCoinPtTEnd->fT
                        && this->testForCoincidence(outer, testS, testE)) {
                    if (this->debugAddIfMissing(outer, testS, testE)) {
                        log->record(kAddMissingCoin_Glitch, id, outer, testS, testE);
                    }
                } else {
                    testS = inner->fCoinPtTStart->debugContains(outerCoin);
                    testE = inner->fCoinPtTEnd->debugContains(outerCoin);
                    if (testS && testS->fT >= outer->fCoinPtTStart->fT
                            && testE && testE->fT <= outer->fCoinPtTEnd->fT
                            && this->testForCoincidence(inner, testS, testE)) {
                        if (this->debugAddIfMissing(inner, testS, testE)) {
                            log->record(kAddMissingCoin_Glitch, id, inner, testS, testE);
                        }
                    }
                }
            }
        }
    } while ((outer = outer->fNext));
}

bool SkOpCoincidence::debugExpand(const char* id, SkPathOpsDebug::GlitchLog* log) const {
    const SkCoincidentSpans* coin = fHead;
    if (!coin) {
        return false;
    }
    bool expanded = false;
    do {
        const SkOpSpan* start = coin->fCoinPtTStart->span()->upCast();
        const SkOpSpanBase* end = coin->fCoinPtTEnd->span();
        const SkOpSegment* segment = coin->fCoinPtTStart->segment();
        const SkOpSegment* oppSegment = coin->fOppPtTStart->segment();
        const SkOpSpan* prev = start->prev();
        if (prev && prev->debugContains(oppSegment)) {
            double midT = (prev->t() + start->t()) / 2;
            if (segment->isClose(midT, oppSegment)) {
                log->record(kExpandCoin_Glitch, id, coin, prev);
            }
        }
        SkOpSpanBase* next = end->final() ? nullptr : end->upCast()->next();
        if (next && next->debugContains(oppSegment)) {
            double midT = (end->t() + next->t()) / 2;
            if (segment->isClose(midT, oppSegment)) {
                log->record(kExpandCoin_Glitch, id, coin, next);
            }
        }
    } while ((coin = coin->fNext));
    return expanded;
}

void SkOpCoincidence::debugFixAligned(const char* id, SkPathOpsDebug::GlitchLog* log) const {
    const SkCoincidentSpans* coin = fHead;
    if (!coin) {
        return;
    }
    do {
        if (coin->fCoinPtTStart->deleted()) {
            log->record(kDeletedCoin_Glitch, id, coin, coin->fCoinPtTStart);
        }
        if (coin->fCoinPtTEnd->deleted()) {
            log->record(kDeletedCoin_Glitch, id, coin, coin->fCoinPtTEnd);
        }
        if (coin->fOppPtTStart->deleted()) {
            log->record(kDeletedCoin_Glitch, id, coin, coin->fOppPtTStart);
        }
        if (coin->fOppPtTEnd->deleted()) {
            log->record(kDeletedCoin_Glitch, id, coin, coin->fOppPtTEnd);
        }
    } while ((coin = coin->fNext));
    coin = fHead;
    do {
        if (coin->fCoinPtTStart->collapsed(coin->fCoinPtTEnd)) {
            log->record(kCollapsedCoin_Glitch, id, coin, coin->fCoinPtTStart);
        }
        if (coin->fOppPtTStart->collapsed(coin->fOppPtTEnd)) {
            log->record(kCollapsedCoin_Glitch, id, coin, coin->fOppPtTStart);
        }
    } while ((coin = coin->fNext));
}

void SkOpCoincidence::debugMark(const char* id, SkPathOpsDebug::GlitchLog* log) const {
    const SkCoincidentSpans* coin = fHead;
    if (!coin) {
        return;
    }
    do {
        const SkOpSpanBase* end = coin->fCoinPtTEnd->span();
        const SkOpSpanBase* oldEnd = end;
        const SkOpSpan* start = coin->fCoinPtTStart->span()->debugStarter(&end);
        const SkOpSpanBase* oEnd = coin->fOppPtTEnd->span();
        const SkOpSpanBase* oOldEnd = oEnd;
        const SkOpSpanBase* oStart = coin->fOppPtTStart->span()->debugStarter(&oEnd);
        bool flipped = (end == oldEnd) != (oEnd == oOldEnd);
        if (flipped) {
            SkTSwap(oStart, oEnd);
        }
        const SkOpSpanBase* next = start;
        const SkOpSpanBase* oNext = oStart;
        do {
            next = next->upCast()->next();
            oNext = flipped ? oNext->prev() : oNext->upCast()->next();
            if (next == end || oNext == oEnd) {
                break;
            }
            if (!next->containsCoinEnd(oNext)) {
                log->record(kMarkCoinEnd_Glitch, id, next, oNext);
            }
            const SkOpSpan* nextSpan = next->upCast();
            const SkOpSpan* oNextSpan = oNext->upCast();
            if (!nextSpan->containsCoincidence(oNextSpan)) {
                log->record(kMarkCoinInsert_Glitch, id, nextSpan, oNextSpan);
            }
        } while (true);
    } while ((coin = coin->fNext));
}
#endif

void SkOpCoincidence::debugShowCoincidence() const {
    SkCoincidentSpans* span = fHead;
    while (span) {
        SkDebugf("%s - id=%d t=%1.9g tEnd=%1.9g\n", __FUNCTION__,
                span->fCoinPtTStart->segment()->debugID(),
                span->fCoinPtTStart->fT, span->fCoinPtTEnd->fT);
        SkDebugf("%s + id=%d t=%1.9g tEnd=%1.9g\n", __FUNCTION__,
                span->fOppPtTStart->segment()->debugID(),
                span->fOppPtTStart->fT, span->fOppPtTEnd->fT);
        span = span->fNext;
    }
}

#if DEBUG_COINCIDENCE
void SkOpContour::debugCheckHealth(const char* id, SkPathOpsDebug::GlitchLog* log) const {
    const SkOpSegment* segment = &fHead;
    do {
        segment->debugCheckHealth(id, log);
    } while ((segment = segment->next()));
}

void SkOpContour::debugMissingCoincidence(const char* id, SkPathOpsDebug::GlitchLog* log,
        const SkOpCoincidence* coincidence) const {
    const SkOpSegment* segment = &fHead;
    do {
        segment->debugMissingCoincidence(id, log, coincidence);
    } while ((segment = segment->next()));
}
#endif

void SkOpSegment::debugValidate() const {
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

bool SkOpSpanBase::debugAlignedEnd(double t, const SkPoint& pt) const {
    SkASSERT(zero_or_one(t));
    const SkOpSegment* segment = this->segment();
    SkASSERT(t ? segment->lastPt() == pt : segment->pts()[0] == pt);
    if (!debugAlignedInner()) {
          return false;
    }
    if ((t ? segment->lastPt() : segment->pts()[0]) != pt) {
        return false;
    }
    const SkOpPtT* ptT = &this->fPtT;
    SkASSERT(t == ptT->fT);
    SkASSERT(pt == ptT->fPt);
    const SkOpPtT* test = ptT, * stopPtT = ptT;
    while ((test = test->next()) != stopPtT) {
        const SkOpSegment* other = test->segment();
        if (other == this->segment()) {
            continue;
        }
        if (!zero_or_one(test->fT)) {
            continue;
        }
        if ((test->fT ? other->lastPt() : other->pts()[0]) != pt) {
            return false;
        }
    }
    return this->fAligned;
}

bool SkOpSpanBase::debugAlignedInner() const {
    // force the spans to share points and t values
    const SkOpPtT* ptT = &this->fPtT, * stopPtT = ptT;
    const SkPoint& pt = ptT->fPt;
    do {
        if (ptT->fPt != pt) {
            return false;
        }
        const SkOpSpanBase* span = ptT->span();
        const SkOpPtT* test = ptT;
        do {
            if ((test = test->next()) == stopPtT) {
                break;
            }
            if (span == test->span() && !span->segment()->ptsDisjoint(*ptT, *test)) {
                return false;
            }
        } while (true);
    } while ((ptT = ptT->next()) != stopPtT);
    return true;
}

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

bool SkOpSpanBase::debugContains(const SkOpSegment* segment) const {
    const SkOpPtT* start = &fPtT;
    const SkOpPtT* walk = start;
    while ((walk = walk->next()) != start) {
        if (walk->segment() == segment) {
            return true;
        }
    }
    return false;
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

void SkOpPtT::debugValidate() const {
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
