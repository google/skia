/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPathOpsDebug.h"
#include "SkPath.h"
#if DEBUG_ANGLE
#include "SkString.h"
#endif

#if DEBUG_VALIDATE
extern bool FLAGS_runFail;
#endif

#if defined SK_DEBUG || !FORCE_RELEASE

const char* SkPathOpsDebug::kLVerbStr[] = {"", "line", "quad", "cubic"};

#if defined(SK_DEBUG) || !FORCE_RELEASE
int SkPathOpsDebug::gContourID = 0;
int SkPathOpsDebug::gSegmentID = 0;
#endif

#if DEBUG_SORT || DEBUG_SWAP_TOP
int SkPathOpsDebug::gSortCountDefault = SK_MaxS32;
int SkPathOpsDebug::gSortCount;
#endif

#if DEBUG_ACTIVE_OP
const char* SkPathOpsDebug::kPathOpStr[] = {"diff", "sect", "union", "xor"};
#endif

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
void* SkPathOpsDebug::CreateNameStr() {
    return SkNEW_ARRAY(char, DEBUG_FILENAME_STRING_LENGTH);
}

void SkPathOpsDebug::DeleteNameStr(void* v) {
    SkDELETE_ARRAY(reinterpret_cast<char* >(v));
}

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

#if !DEBUG_SHOW_TEST_NAME  // enable when building without extended test
void SkPathOpsDebug::ShowPath(const SkPath& one, const SkPath& two, SkPathOp op, const char* name) {
}
#endif

#include "SkOpAngle.h"
#include "SkOpSegment.h"

#if DEBUG_SWAP_TOP
int SkOpSegment::debugInflections(const SkOpSpanBase* start, const SkOpSpanBase* end) const {
    if (fVerb != SkPath::kCubic_Verb) {
        return false;
    }
    SkDCubic dst = SkDCubic::SubDivide(fPts, start->t(), end->t());
    double inflections[2];
    return dst.findInflections(inflections);
}
#endif

SkOpAngle* SkOpSegment::debugLastAngle() {
    SkOpAngle* result = NULL;
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

void SkOpSegment::debugReset() {
    this->init(this->fPts, this->contour(), this->verb());
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
        if (lastId == fID && lastT == span->t()) {
            continue;
        }
        lastId = fID;
        lastT = span->t();
        SkDebugf("%s id=%d", __FUNCTION__, fID);
        SkDebugf(" (%1.9g,%1.9g", fPts[0].fX, fPts[0].fY);
        for (int vIndex = 1; vIndex <= SkPathOpsVerbToPoints(fVerb); ++vIndex) {
            SkDebugf(" %1.9g,%1.9g", fPts[vIndex].fX, fPts[vIndex].fY);
        }
        const SkOpPtT* ptT = span->ptT();
        SkDebugf(") t=%1.9g (%1.9g,%1.9g)", ptT->fT, ptT->fPt.fX, ptT->fPt.fY);
        SkDebugf(" tEnd=%1.9g", span->next()->t());
        SkDebugf(" windSum=");
        if (span->windSum() == SK_MinS32) {
            SkDebugf("?");
        } else {
            SkDebugf("%d", span->windSum());
        }
        SkDebugf(" windValue=%d oppValue=%d", span->windValue(), span->oppValue());
        SkDebugf("\n");
   } while ((span = span->next()->upCastable()));
}
#endif

#if DEBUG_MARK_DONE
void SkOpSegment::debugShowNewWinding(const char* fun, const SkOpSpan* span, int winding) {
    const SkPoint& pt = span->ptT()->fPt;
    SkDebugf("%s id=%d", fun, fID);
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
    SkDebugf("%s id=%d", fun, fID);
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
        wind += next->sign() * (op ? minSpan->oppValue() : minSpan->windValue());
        if (useXor) {
            wind &= 1;
        }
        useXor = op ? isXor : oppXor;
        SkASSERT(lastOppXor == -1 || lastOppXor == (int) useXor);
        lastOppXor = (int) useXor;
        opp += next->sign() * (op ? minSpan->windValue() : minSpan->oppValue());
        if (useXor) {
            opp &= 1;
        }
        next = next->fNext;
    } while (next && next != first);
    SkASSERT(wind == 0);
    SkASSERT(opp == 0 || !FLAGS_runFail);
#endif
}

void SkOpAngle::debugValidateNext() const {
#if !FORCE_RELEASE
    const SkOpAngle* first = this;
    const SkOpAngle* next = first;
    SkTDArray<const SkOpAngle*>(angles);
    do {
//        SK_ALWAYSBREAK(next->fSegment->debugContains(next));
        angles.push(next);
        next = next->next();
        if (next == first) {
            break;
        }
        SK_ALWAYSBREAK(!angles.contains(next));
        if (!next) {
            return;
        }
    } while (true);
#endif
}

void SkOpSegment::debugValidate() const {
#if DEBUG_VALIDATE
    const SkOpSpanBase* span = &fHead;
    double lastT = -1;
    const SkOpSpanBase* prev = NULL;
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
    SkASSERT(span->final());
    span->debugValidate();
#endif
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

#include "SkOpContour.h"

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
        ++loop;
    } while ((next = next->fNext) && next != this);
    return 0;
}

void SkOpPtT::debugValidate() const {
#if DEBUG_VALIDATE
    if (contour()->globalState()->phase() == SkOpGlobalState::kIntersecting) {
        return;
    }
    SkASSERT(fNext);
    SkASSERT(fNext != this);
    SkASSERT(fNext->fNext);
    SkASSERT(debugLoopLimit(false) == 0);
#endif
}
