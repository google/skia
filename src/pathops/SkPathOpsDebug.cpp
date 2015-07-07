/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkMutex.h"
#include "SkPath.h"
#include "SkPathOpsDebug.h"
#include "SkString.h"

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

#if defined(SK_DEBUG) || !FORCE_RELEASE
int SkPathOpsDebug::gContourID = 0;
int SkPathOpsDebug::gSegmentID = 0;
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

#ifdef SK_DEBUG
bool SkOpGlobalState::debugRunFail() const {
#if DEBUG_VALIDATE
    return FLAGS_runFail;
#else
    return false;
#endif
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

#include "SkOpAngle.h"
#include "SkOpCoincidence.h"
#include "SkOpSegment.h"

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
    SkASSERT(count >= fDoneCount);
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
    int rectCount = path.isRectContours() ? path.rectContours(NULL, NULL) : 0;
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
