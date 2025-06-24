/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPath.h"
#include "include/core/SkPoint.h"
#include "include/core/SkTypes.h"
#include "include/pathops/SkPathOps.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkMath.h"
#include "src/base/SkArenaAlloc.h"
#include "src/base/SkFloatBits.h"
#include "src/pathops/SkIntersections.h"
#include "src/pathops/SkOpAngle.h"
#include "src/pathops/SkOpCoincidence.h"
#include "src/pathops/SkOpContour.h"
#include "src/pathops/SkOpSegment.h"
#include "src/pathops/SkOpSpan.h"
#include "src/pathops/SkPathOpsConic.h"
#include "src/pathops/SkPathOpsCubic.h"
#include "src/pathops/SkPathOpsCurve.h"
#include "src/pathops/SkPathOpsLine.h"
#include "src/pathops/SkPathOpsPoint.h"
#include "src/pathops/SkPathOpsQuad.h"
#include "src/pathops/SkPathOpsRect.h"
#include "src/pathops/SkPathOpsTSect.h"
#include "src/pathops/SkPathOpsTypes.h"
#include "tests/PathOpsDebug.h"

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstdint>

bool PathOpsDebug::gJson;
bool PathOpsDebug::gMarkJsonFlaky;
bool PathOpsDebug::gOutFirst;
bool PathOpsDebug::gCheckForDuplicateNames;
bool PathOpsDebug::gOutputSVG;
FILE* PathOpsDebug::gOut;

inline void DebugDumpDouble(double x) {
    if (x == floor(x)) {
        SkDebugf("%.0f", x);
    } else {
        SkDebugf("%1.19g", x);
    }
}

inline void DebugDumpFloat(float x) {
    if (x == floorf(x)) {
        SkDebugf("%.0f", x);
    } else {
        SkDebugf("%1.9gf", x);
    }
}

inline void DebugDumpHexFloat(float x) {
    SkDebugf("SkBits2Float(0x%08x)", SkFloat2Bits(x));
}

// if not defined by PathOpsDebug.cpp ...
#if !defined SK_DEBUG && FORCE_RELEASE
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
#endif

static void DumpID(int id) {
    SkDebugf("} ");
    if (id >= 0) {
        SkDebugf("id=%d", id);
    }
    SkDebugf("\n");
}

void SkDConic::dump() const {
    dumpInner();
    SkDebugf("},\n");
}

void SkDConic::dumpID(int id) const {
    dumpInner();
    DumpID(id);
}

void SkDConic::dumpInner() const {
    SkDebugf("{");
    fPts.dumpInner();
    SkDebugf("}}, %1.9gf", fWeight);
}

void SkDCubic::dump() const {
    this->dumpInner();
    SkDebugf("}},\n");
}

void SkDCubic::dumpID(int id) const {
    this->dumpInner();
    SkDebugf("}");
    DumpID(id);
}

static inline bool double_is_NaN(double x) { return x != x; }

void SkDCubic::dumpInner() const {
    SkDebugf("{{");
    int index = 0;
    do {
        if (index != 0) {
            if (double_is_NaN(fPts[index].fX) && double_is_NaN(fPts[index].fY)) {
                return;
            }
            SkDebugf(", ");
        }
        fPts[index].dump();
    } while (++index < 3);
    if (double_is_NaN(fPts[index].fX) && double_is_NaN(fPts[index].fY)) {
        return;
    }
    SkDebugf(", ");
    fPts[index].dump();
}

void SkDCurve::dump() const {
    dumpID(-1);
}

void SkDCurve::dumpID(int id) const {
#ifndef SK_RELEASE
    switch(fVerb) {
        case SkPath::kLine_Verb:
            fLine.dumpID(id);
            break;
        case SkPath::kQuad_Verb:
            fQuad.dumpID(id);
            break;
        case SkPath::kConic_Verb:
            fConic.dumpID(id);
            break;
        case SkPath::kCubic_Verb:
            fCubic.dumpID(id);
            break;
        default:
            SkASSERT(0);
    }
#else
    fCubic.dumpID(id);
#endif
}

void SkDLine::dump() const {
    this->dumpInner();
    SkDebugf("}},\n");
}

void SkDLine::dumpID(int id) const {
    this->dumpInner();
    SkDebugf("}");
    DumpID(id);
}

void SkDLine::dumpInner() const {
    SkDebugf("{{");
    fPts[0].dump();
    SkDebugf(", ");
    fPts[1].dump();
}

void SkDPoint::dump() const {
    SkDebugf("{");
    DebugDumpDouble(fX);
    SkDebugf(", ");
    DebugDumpDouble(fY);
    SkDebugf("}");
}

void SkDPoint::Dump(const SkPoint& pt) {
    SkDebugf("{");
    DebugDumpFloat(pt.fX);
    SkDebugf(", ");
    DebugDumpFloat(pt.fY);
    SkDebugf("}");
}

void SkDPoint::DumpHex(const SkPoint& pt) {
    SkDebugf("{");
    DebugDumpHexFloat(pt.fX);
    SkDebugf(", ");
    DebugDumpHexFloat(pt.fY);
    SkDebugf("}");
}

void SkDQuad::dump() const {
    dumpInner();
    SkDebugf("}},\n");
}

void SkDQuad::dumpID(int id) const {
    dumpInner();
    SkDebugf("}");
    DumpID(id);
}

void SkDQuad::dumpInner() const {
    SkDebugf("{{");
    int index = 0;
    do {
        fPts[index].dump();
        SkDebugf(", ");
    } while (++index < 2);
    fPts[index].dump();
}

void SkIntersections::dump() const {
    SkDebugf("used=%d of %d", fUsed, fMax);
    for (int index = 0; index < fUsed; ++index) {
        SkDebugf(" t=(%s%1.9g,%s%1.9g) pt=(%1.9g,%1.9g)",
                fIsCoincident[0] & (1 << index) ? "*" : "", fT[0][index],
                fIsCoincident[1] & (1 << index) ? "*" : "", fT[1][index],
                fPt[index].fX, fPt[index].fY);
        if (index < 2 && fNearlySame[index]) {
            SkDebugf(" pt2=(%1.9g,%1.9g)",fPt2[index].fX, fPt2[index].fY);
        }
    }
    SkDebugf("\n");
}

const SkOpAngle* AngleAngle(const SkOpAngle* angle, int id) {
    return angle->debugAngle(id);
}

SkOpContour* AngleContour(SkOpAngle* angle, int id) {
    return angle->debugContour(id);
}

const SkOpPtT* AnglePtT(const SkOpAngle* angle, int id) {
    return angle->debugPtT(id);
}

const SkOpSegment* AngleSegment(const SkOpAngle* angle, int id) {
    return angle->debugSegment(id);
}

const SkOpSpanBase* AngleSpan(const SkOpAngle* angle, int id) {
    return angle->debugSpan(id);
}

const SkOpAngle* ContourAngle(SkOpContour* contour, int id) {
    return contour->debugAngle(id);
}

SkOpContour* ContourContour(SkOpContour* contour, int id) {
    return contour->debugContour(id);
}

const SkOpPtT* ContourPtT(SkOpContour* contour, int id) {
    return contour->debugPtT(id);
}

const SkOpSegment* ContourSegment(SkOpContour* contour, int id) {
    return contour->debugSegment(id);
}

const SkOpSpanBase* ContourSpan(SkOpContour* contour, int id) {
    return contour->debugSpan(id);
}

const SkOpAngle* CoincidenceAngle(SkOpCoincidence* coin, int id) {
    return coin->debugAngle(id);
}

SkOpContour* CoincidenceContour(SkOpCoincidence* coin, int id) {
    return coin->debugContour(id);
}

const SkOpPtT* CoincidencePtT(SkOpCoincidence* coin, int id) {
    return coin->debugPtT(id);
}

const SkOpSegment* CoincidenceSegment(SkOpCoincidence* coin, int id) {
    return coin->debugSegment(id);
}

const SkOpSpanBase* CoincidenceSpan(SkOpCoincidence* coin, int id) {
    return coin->debugSpan(id);
}

const SkOpAngle* PtTAngle(const SkOpPtT* ptT, int id) {
    return ptT->debugAngle(id);
}

SkOpContour* PtTContour(SkOpPtT* ptT, int id) {
    return ptT->debugContour(id);
}

const SkOpPtT* PtTPtT(const SkOpPtT* ptT, int id) {
    return ptT->debugPtT(id);
}

const SkOpSegment* PtTSegment(const SkOpPtT* ptT, int id) {
    return ptT->debugSegment(id);
}

const SkOpSpanBase* PtTSpan(const SkOpPtT* ptT, int id) {
    return ptT->debugSpan(id);
}

const SkOpAngle* SegmentAngle(const SkOpSegment* span, int id) {
    return span->debugAngle(id);
}

SkOpContour* SegmentContour(SkOpSegment* span, int id) {
    return span->debugContour(id);
}

const SkOpPtT* SegmentPtT(const SkOpSegment* span, int id) {
    return span->debugPtT(id);
}

const SkOpSegment* SegmentSegment(const SkOpSegment* span, int id) {
    return span->debugSegment(id);
}

const SkOpSpanBase* SegmentSpan(const SkOpSegment* span, int id) {
    return span->debugSpan(id);
}

const SkOpAngle* SpanAngle(const SkOpSpanBase* span, int id) {
    return span->debugAngle(id);
}

SkOpContour* SpanContour(SkOpSpanBase* span, int id) {
    return span->debugContour(id);
}

const SkOpPtT* SpanPtT(const SkOpSpanBase* span, int id) {
    return span->debugPtT(id);
}

const SkOpSegment* SpanSegment(const SkOpSpanBase* span, int id) {
    return span->debugSegment(id);
}

const SkOpSpanBase* SpanSpan(const SkOpSpanBase* span, int id) {
    return span->debugSpan(id);
}

#if DEBUG_COIN
void SkPathOpsDebug::DumpCoinDict() {
    SkPathOpsDebug::gCoinSumChangedDict.dump("unused coin algorithm", false);
    SkPathOpsDebug::gCoinSumVisitedDict.dump("visited coin function", true);
}

void SkPathOpsDebug::CoinDict::dump(const char* str, bool visitCheck) const {
    int count = fDict.size();
    for (int index = 0; index < count; ++index) {
        const auto& entry = fDict[index];
        if (visitCheck || entry.fGlitchType == kUninitialized_Glitch) {
            SkDebugf("%s %s : line %d iteration %d", str, entry.fFunctionName,
                    entry.fLineNumber, entry.fIteration);
            DumpGlitchType(entry.fGlitchType);
            SkDebugf("\n");
        }
    }
}
#endif

void SkOpContour::dumpContours() const {
    SkOpContour* contour = this->globalState()->contourHead();
    do {
        contour->dump();
    } while ((contour = contour->next()));
}

void SkOpContour::dumpContoursAll() const {
    SkOpContour* contour = this->globalState()->contourHead();
    do {
        contour->dumpAll();
    } while ((contour = contour->next()));
}

void SkOpContour::dumpContoursAngles() const {
    SkOpContour* contour = this->globalState()->contourHead();
    do {
        contour->dumpAngles();
    } while ((contour = contour->next()));
}

void SkOpContour::dumpContoursPts() const {
    SkOpContour* contour = this->globalState()->contourHead();
    do {
        contour->dumpPts();
    } while ((contour = contour->next()));
}

void SkOpContour::dumpContoursPt(int segmentID) const {
    SkOpContour* contour = this->globalState()->contourHead();
    do {
        contour->dumpPt(segmentID);
    } while ((contour = contour->next()));
}

void SkOpContour::dumpContoursSegment(int segmentID) const {
    SkOpContour* contour = this->globalState()->contourHead();
    do {
        contour->dumpSegment(segmentID);
    } while ((contour = contour->next()));
}

void SkOpContour::dumpContoursSpan(int spanID) const {
    SkOpContour* contour = this->globalState()->contourHead();
    do {
        contour->dumpSpan(spanID);
    } while ((contour = contour->next()));
}

void SkOpContour::dumpContoursSpans() const {
    SkOpContour* contour = this->globalState()->contourHead();
    do {
        contour->dumpSpans();
    } while ((contour = contour->next()));
}

#ifdef SK_DEBUG
const SkTSpan* DebugSpan(const SkTSect* sect, int id) {
    return sect->debugSpan(id);
}

const SkTSpan* DebugT(const SkTSect* sect, double t) {
    return sect->debugT(t);
}
#endif

void Dump(const SkTSect* sect) {
    sect->dump();
}

void DumpBoth(SkTSect* sect1, SkTSect* sect2) {
    sect1->dumpBoth(sect2);
}

void DumpBounded(SkTSect* sect1, int id) {
    sect1->dumpBounded(id);
}

void DumpBounds(SkTSect* sect1) {
    sect1->dumpBounds();
}

void DumpCoin(SkTSect* sect1) {
    sect1->dumpCoin();
}

void DumpCoinCurves(SkTSect* sect1) {
    sect1->dumpCoinCurves();
}

void DumpCurves(const SkTSect* sect) {
    sect->dumpCurves();
}

void Dump(const SkTSpan* span) {
    span->dump();
}

void DumpAll(const SkTSpan* span) {
    span->dumpAll();
}

void DumpBounded(const SkTSpan* span) {
    span->dumpBounded(0);
}

void DumpCoin(const SkTSpan* span) {
    span->dumpCoin();
}

static void dumpTestCase(const SkDQuad& quad1, const SkDQuad& quad2, int testNo) {
    SkDebugf("\n<div id=\"quad%d\">\n", testNo);
    quad1.dumpInner();
    SkDebugf("}}, ");
    quad2.dump();
    SkDebugf("</div>\n\n");
}

static void dumpTestTrailer() {
    SkDebugf("</div>\n\n<script type=\"text/javascript\">\n\n");
    SkDebugf("    var testDivs = [\n");
}

static void dumpTestList(int testNo, double min) {
    SkDebugf("        quad%d,", testNo);
    if (min > 0) {
        SkDebugf("  // %1.9g", min);
    }
    SkDebugf("\n");
}

void DumpQ(const SkDQuad& quad1, const SkDQuad& quad2, int testNo) {
    SkDebugf("\n");
    dumpTestCase(quad1, quad2, testNo);
    dumpTestTrailer();
    dumpTestList(testNo, 0);
    SkDebugf("\n");
}

void DumpT(const SkDQuad& quad, double t) {
    SkDLine line = {{quad.ptAtT(t), quad[0]}};
    line.dump();
}

const SkOpAngle* SkOpAngle::debugAngle(int id) const {
    return this->segment()->debugAngle(id);
}

const SkOpCoincidence* SkOpAngle::debugCoincidence() const {
    return this->segment()->debugCoincidence();
}

SkOpContour* SkOpAngle::debugContour(int id) const {
    return this->segment()->debugContour(id);
}

const SkOpPtT* SkOpAngle::debugPtT(int id) const {
    return this->segment()->debugPtT(id);
}

const SkOpSegment* SkOpAngle::debugSegment(int id) const {
    return this->segment()->debugSegment(id);
}

int SkOpAngle::debugSign() const {
    SkASSERT(fStart->t() != fEnd->t());
    return fStart->t() < fEnd->t() ? -1 : 1;
}

const SkOpSpanBase* SkOpAngle::debugSpan(int id) const {
    return this->segment()->debugSpan(id);
}

void SkOpAngle::dump() const {
    dumpOne(true);
    SkDebugf("\n");
}

void SkOpAngle::dumpOne(bool functionHeader) const {
//    fSegment->debugValidate();
    const SkOpSegment* segment = this->segment();
    const SkOpSpan& mSpan = *fStart->starter(fEnd);
    if (functionHeader) {
        SkDebugf("%s ", __FUNCTION__);
    }
    SkDebugf("[%d", segment->debugID());
    SkDebugf("/%d", debugID());
    SkDebugf("] next=");
    if (fNext) {
        SkDebugf("%d", fNext->fStart->segment()->debugID());
        SkDebugf("/%d", fNext->debugID());
    } else {
        SkDebugf("?");
    }
    SkDebugf(" sect=%d/%d ", fSectorStart, fSectorEnd);
    SkDebugf(" s=%1.9g [%d] e=%1.9g [%d]", fStart->t(), fStart->debugID(),
                fEnd->t(), fEnd->debugID());
    SkDebugf(" sgn=%d windVal=%d", this->debugSign(), mSpan.windValue());

    SkDebugf(" windSum=");
    SkPathOpsDebug::WindingPrintf(mSpan.windSum());
    if (mSpan.oppValue() != 0 || mSpan.oppSum() != SK_MinS32) {
        SkDebugf(" oppVal=%d", mSpan.oppValue());
        SkDebugf(" oppSum=");
        SkPathOpsDebug::WindingPrintf(mSpan.oppSum());
    }
    if (mSpan.done()) {
        SkDebugf(" done");
    }
    if (unorderable()) {
        SkDebugf(" unorderable");
    }
    if (segment->operand()) {
        SkDebugf(" operand");
    }
}

void SkOpAngle::dumpTo(const SkOpSegment* segment, const SkOpAngle* to) const {
    const SkOpAngle* first = this;
    const SkOpAngle* next = this;
    const char* indent = "";
    do {
        SkDebugf("%s", indent);
        next->dumpOne(false);
        if (segment == next->fStart->segment()) {
            if (this == fNext) {
                SkDebugf(" << from");
            }
            if (to == fNext) {
                SkDebugf(" << to");
            }
        }
        SkDebugf("\n");
        indent = "           ";
        next = next->fNext;
    } while (next && next != first);
}

void SkOpAngle::dumpCurves() const {
    const SkOpAngle* first = this;
    const SkOpAngle* next = this;
    do {
        next->fPart.fCurve.dumpID(next->segment()->debugID());
        next = next->fNext;
    } while (next && next != first);
}

void SkOpAngle::dumpLoop() const {
    const SkOpAngle* first = this;
    const SkOpAngle* next = this;
    do {
        next->dumpOne(false);
        SkDebugf("\n");
        next = next->fNext;
    } while (next && next != first);
}

void SkOpAngle::dumpTest() const {
    const SkOpAngle* first = this;
    const SkOpAngle* next = this;
    do {
        SkDebugf("{ ");
        SkOpSegment* segment = next->segment();
        segment->dumpPts();
        SkDebugf(", %d, %1.9g, %1.9g, {} },\n", SkPathOpsVerbToPoints(segment->verb()) + 1,
                next->start()->t(), next->end()->t());
        next = next->fNext;
    } while (next && next != first);
}

bool SkOpPtT::debugMatchID(int id) const {
    int limit = this->debugLoopLimit(false);
    int loop = 0;
    const SkOpPtT* ptT = this;
    do {
        if (ptT->debugID() == id) {
            return true;
        }
    } while ((!limit || ++loop <= limit) && (ptT = ptT->next()) && ptT != this);
    return false;
}

const SkOpAngle* SkOpPtT::debugAngle(int id) const {
    return this->span()->debugAngle(id);
}

SkOpContour* SkOpPtT::debugContour(int id) const {
    return this->span()->debugContour(id);
}

const SkOpCoincidence* SkOpPtT::debugCoincidence() const {
    return this->span()->debugCoincidence();
}

const SkOpPtT* SkOpPtT::debugPtT(int id) const {
    return this->span()->debugPtT(id);
}

const SkOpSegment* SkOpPtT::debugSegment(int id) const {
    return this->span()->debugSegment(id);
}

const SkOpSpanBase* SkOpPtT::debugSpan(int id) const {
    return this->span()->debugSpan(id);
}

void SkOpPtT::dump() const {
    SkDebugf("seg=%d span=%d ptT=%d",
            this->segment()->debugID(), this->span()->debugID(), this->debugID());
    this->dumpBase();
    SkDebugf("\n");
}

void SkOpPtT::dumpAll() const {
    contour()->indentDump();
    const SkOpPtT* next = this;
    int limit = debugLoopLimit(true);
    int loop = 0;
    do {
        SkDebugf("%.*s", contour()->debugIndent(), "        ");
        SkDebugf("seg=%d span=%d ptT=%d",
                next->segment()->debugID(), next->span()->debugID(), next->debugID());
        next->dumpBase();
        SkDebugf("\n");
        if (limit && ++loop >= limit) {
            SkDebugf("*** abort loop ***\n");
            break;
        }
    } while ((next = next->fNext) && next != this);
    contour()->outdentDump();
}

void SkOpPtT::dumpBase() const {
    SkDebugf(" t=%1.9g pt=(%1.9g,%1.9g)%s%s%s", this->fT, this->fPt.fX, this->fPt.fY,
            this->fCoincident ? " coin" : "",
            this->fDuplicatePt ? " dup" : "", this->fDeleted ? " deleted" : "");
}

const SkOpAngle* SkOpSpanBase::debugAngle(int id) const {
    return this->segment()->debugAngle(id);
}

const SkOpCoincidence* SkOpSpanBase::debugCoincidence() const {
    return this->segment()->debugCoincidence();
}

SkOpContour* SkOpSpanBase::debugContour(int id) const {
    return this->segment()->debugContour(id);
}

const SkOpPtT* SkOpSpanBase::debugPtT(int id) const {
    return this->segment()->debugPtT(id);
}

const SkOpSegment* SkOpSpanBase::debugSegment(int id) const {
    return this->segment()->debugSegment(id);
}

const SkOpSpanBase* SkOpSpanBase::debugSpan(int id) const {
    return this->segment()->debugSpan(id);
}

void SkOpSpanBase::dump() const {
    this->dumpHead();
    this->fPtT.dump();
}

void SkOpSpanBase::dumpHead() const {
    SkDebugf("%.*s", contour()->debugIndent(), "        ");
    SkDebugf("seg=%d span=%d", this->segment()->debugID(), this->debugID());
    this->dumpBase();
    SkDebugf("\n");
}

void SkOpSpanBase::dumpAll() const {
    this->dumpHead();
    this->fPtT.dumpAll();
}

void SkOpSpanBase::dumpBase() const {
    if (this->fAligned) {
        SkDebugf(" aligned");
    }
    if (this->fChased) {
        SkDebugf(" chased");
    }
#ifdef SK_DEBUG
    if (this->fDebugDeleted) {
        SkDebugf(" deleted");
    }
#endif
    if (!this->final()) {
        this->upCast()->dumpSpan();
    }
    const SkOpSpanBase* coin = this->coinEnd();
    if (this != coin) {
        SkDebugf(" coinEnd seg/span=%d/%d", coin->segment()->debugID(), coin->debugID());
    } else if (this->final() || !this->upCast()->isCoincident()) {
        const SkOpPtT* oPt = this->ptT()->next();
        SkDebugf(" seg/span=%d/%d", oPt->segment()->debugID(), oPt->span()->debugID());
    }
    SkDebugf(" adds=%d", fSpanAdds);
}

void SkOpSpanBase::dumpCoin() const {
    const SkOpSpan* span = this->upCastable();
    if (!span) {
        return;
    }
    if (!span->isCoincident()) {
        return;
    }
    span->dumpCoin();
}

void SkOpSpan::dumpCoin() const {
    const SkOpSpan* coincident = fCoincident;
    bool ok = debugCoinLoopCheck();
    this->dump();
    int loop = 0;
    do {
        coincident->dump();
        if (!ok && ++loop > 10) {
            SkDebugf("*** abort loop ***\n");
            break;
        }
    } while ((coincident = coincident->fCoincident) != this);
}

bool SkOpSpan::dumpSpan() const {
    SkOpSpan* coin = fCoincident;
    if (this != coin) {
        SkDebugf(" coinStart seg/span=%d/%d", coin->segment()->debugID(), coin->debugID());
    }
    SkDebugf(" windVal=%d", this->windValue());
    SkDebugf(" windSum=");
    SkPathOpsDebug::WindingPrintf(this->windSum());
    if (this->oppValue() != 0 || this->oppSum() != SK_MinS32) {
        SkDebugf(" oppVal=%d", this->oppValue());
        SkDebugf(" oppSum=");
        SkPathOpsDebug::WindingPrintf(this->oppSum());
    }
    if (this->done()) {
        SkDebugf(" done");
    }
    return this != coin;
}

const SkOpAngle* SkOpSegment::debugAngle(int id) const {
    return this->contour()->debugAngle(id);
}

const SkOpCoincidence* SkOpSegment::debugCoincidence() const {
    return this->contour()->debugCoincidence();
}

SkOpContour* SkOpSegment::debugContour(int id) const {
    return this->contour()->debugContour(id);
}

const SkOpPtT* SkOpSegment::debugPtT(int id) const {
    return this->contour()->debugPtT(id);
}

const SkOpSegment* SkOpSegment::debugSegment(int id) const {
    return this->contour()->debugSegment(id);
}

const SkOpSpanBase* SkOpSegment::debugSpan(int id) const {
    return this->contour()->debugSpan(id);
}

void SkOpSegment::dump() const {
    SkDebugf("%.*s", contour()->debugIndent(), "        ");
    this->dumpPts();
    const SkOpSpanBase* span = &fHead;
    contour()->indentDump();
    do {
        SkDebugf("%.*s span=%d ", contour()->debugIndent(), "        ", span->debugID());
        span->ptT()->dumpBase();
        span->dumpBase();
        SkDebugf("\n");
    } while (!span->final() && (span = span->upCast()->next()));
    contour()->outdentDump();
}

void SkOpSegment::dumpAll() const {
    SkDebugf("%.*s", contour()->debugIndent(), "        ");
    this->dumpPts();
    const SkOpSpanBase* span = &fHead;
    contour()->indentDump();
    do {
        span->dumpAll();
    } while (!span->final() && (span = span->upCast()->next()));
    contour()->outdentDump();
}

void SkOpSegment::dumpAngles() const {
    SkDebugf("seg=%d\n", debugID());
    const SkOpSpanBase* span = &fHead;
    do {
        const SkOpAngle* fAngle = span->fromAngle();
        const SkOpAngle* tAngle = span->final() ? nullptr : span->upCast()->toAngle();
        if (fAngle) {
            SkDebugf("  span=%d from=%d ", span->debugID(), fAngle->debugID());
            fAngle->dumpTo(this, tAngle);
        }
        if (tAngle) {
            SkDebugf("  span=%d to=%d   ", span->debugID(), tAngle->debugID());
            tAngle->dumpTo(this, fAngle);
        }
    } while (!span->final() && (span = span->upCast()->next()));
}

void SkOpSegment::dumpCoin() const {
    const SkOpSpan* span = &fHead;
    do {
        span->dumpCoin();
    } while ((span = span->next()->upCastable()));
}

void SkOpSegment::dumpPtsInner(const char* prefix) const {
    int last = SkPathOpsVerbToPoints(fVerb);
    SkDebugf("%s=%d {{", prefix, this->debugID());
    if (fVerb == SkPath::kConic_Verb) {
        SkDebugf("{");
    }
    int index = 0;
    do {
        SkDPoint::Dump(fPts[index]);
        SkDebugf(", ");
    } while (++index < last);
    SkDPoint::Dump(fPts[index]);
    SkDebugf("}}");
    if (fVerb == SkPath::kConic_Verb) {
        SkDebugf(", %1.9gf}", fWeight);
    }
}

void SkOpSegment::dumpPts(const char* prefix) const {
    dumpPtsInner(prefix);
    SkDebugf("\n");
}

void SkCoincidentSpans::dump() const {
    SkDebugf("- seg=%d span=%d ptT=%d ", fCoinPtTStart->segment()->debugID(),
        fCoinPtTStart->span()->debugID(), fCoinPtTStart->debugID());
    fCoinPtTStart->dumpBase();
    SkDebugf(" span=%d ptT=%d ", fCoinPtTEnd->span()->debugID(), fCoinPtTEnd->debugID());
    fCoinPtTEnd->dumpBase();
    if (fCoinPtTStart->segment()->operand()) {
        SkDebugf(" operand");
    }
    if (fCoinPtTStart->segment()->isXor()) {
        SkDebugf(" xor");
    }
    SkDebugf("\n");
    SkDebugf("+ seg=%d span=%d ptT=%d ", fOppPtTStart->segment()->debugID(),
        fOppPtTStart->span()->debugID(), fOppPtTStart->debugID());
    fOppPtTStart->dumpBase();
    SkDebugf(" span=%d ptT=%d ", fOppPtTEnd->span()->debugID(), fOppPtTEnd->debugID());
    fOppPtTEnd->dumpBase();
    if (fOppPtTStart->segment()->operand()) {
        SkDebugf(" operand");
    }
    if (fOppPtTStart->segment()->isXor()) {
        SkDebugf(" xor");
    }
    SkDebugf("\n");
}

void SkOpCoincidence::dump() const {
    SkCoincidentSpans* span = fHead;
    while (span) {
        span->dump();
        span = span->next();
    }
    if (!fTop || fHead == fTop) {
        return;
    }
    SkDebugf("top:\n");
    span = fTop;
    int count = 0;
    while (span) {
        span->dump();
        span = span->next();
        SkCoincidentSpans* check = fTop;
        ++count;
        for (int index = 0; index < count; ++index) {
            if (span == check) {
                SkDebugf("(loops to #%d)\n", index);
                return;
            }
            check = check->next();
        }
    }
}

void SkOpContour::dump() const {
    SkDebugf("contour=%d count=%d op=%d xor=%d\n", this->debugID(), fCount, fOperand, fXor);
    if (!fCount) {
        return;
    }
    const SkOpSegment* segment = &fHead;
    SkDEBUGCODE(fDebugIndent = 0);
    this->indentDump();
    do {
        segment->dump();
    } while ((segment = segment->next()));
    this->outdentDump();
}

void SkOpContour::dumpAll() const {
    SkDebugf("contour=%d count=%d op=%d xor=%d\n", this->debugID(), fCount, fOperand, fXor);
    if (!fCount) {
        return;
    }
    const SkOpSegment* segment = &fHead;
    SkDEBUGCODE(fDebugIndent = 0);
    this->indentDump();
    do {
        segment->dumpAll();
    } while ((segment = segment->next()));
    this->outdentDump();
}


void SkOpContour::dumpAngles() const {
    SkDebugf("contour=%d\n", this->debugID());
    const SkOpSegment* segment = &fHead;
    do {
        SkDebugf("  seg=%d ", segment->debugID());
        segment->dumpAngles();
    } while ((segment = segment->next()));
}

void SkOpContour::dumpPt(int index) const {
    const SkOpSegment* segment = &fHead;
    do {
        if (segment->debugID() == index) {
            segment->dumpPts();
        }
    } while ((segment = segment->next()));
}

void SkOpContour::dumpPts(const char* prefix) const {
    SkDebugf("contour=%d\n", this->debugID());
    const SkOpSegment* segment = &fHead;
    do {
        SkDebugf("  %s=%d ", prefix, segment->debugID());
        segment->dumpPts(prefix);
    } while ((segment = segment->next()));
}

void SkOpContour::dumpPtsX(const char* prefix) const {
    if (!this->fCount) {
        SkDebugf("<empty>\n");
        return;
    }
    const SkOpSegment* segment = &fHead;
    do {
        segment->dumpPts(prefix);
    } while ((segment = segment->next()));
}

void SkOpContour::dumpSegment(int index) const {
    debugSegment(index)->dump();
}

void SkOpContour::dumpSegments(const char* prefix, SkPathOp op) const {
    bool firstOp = false;
    const SkOpContour* c = this;
    do {
        if (!firstOp && c->operand()) {
#if DEBUG_ACTIVE_OP
            SkDebugf("op %s\n", SkPathOpsDebug::kPathOpStr[op]);
#endif
            firstOp = true;
        }
        c->dumpPtsX(prefix);
    } while ((c = c->next()));
}

void SkOpContour::dumpSpan(int index) const {
    debugSpan(index)->dump();
}

void SkOpContour::dumpSpans() const {
    SkDebugf("contour=%d\n", this->debugID());
    const SkOpSegment* segment = &fHead;
    do {
        SkDebugf("  seg=%d ", segment->debugID());
        segment->dump();
    } while ((segment = segment->next()));
}

void SkOpCurve::dump() const {
    int count = SkPathOpsVerbToPoints(SkDEBUGRELEASE(fVerb, SkPath::kCubic_Verb));
    SkDebugf("{{");
    int index;
    for (index = 0; index <= count - 1; ++index) {
        SkDebugf("{%1.9gf,%1.9gf}, ", fPts[index].fX, fPts[index].fY);
    }
    SkDebugf("{%1.9gf,%1.9gf}}}\n", fPts[index].fX, fPts[index].fY);
}

#ifdef SK_DEBUG
const SkOpAngle* SkOpGlobalState::debugAngle(int id) const {
    const SkOpContour* contour = fContourHead;
    do {
        const SkOpSegment* segment = contour->first();
        while (segment) {
            const SkOpSpan* span = segment->head();
            do {
                SkOpAngle* angle = span->fromAngle();
                if (angle && angle->debugID() == id) {
                    return angle;
                }
                angle = span->toAngle();
                if (angle && angle->debugID() == id) {
                    return angle;
                }
            } while ((span = span->next()->upCastable()));
            const SkOpSpanBase* tail = segment->tail();
            SkOpAngle* angle = tail->fromAngle();
            if (angle && angle->debugID() == id) {
                return angle;
            }
            segment = segment->next();
        }
    } while ((contour = contour->next()));
    return nullptr;
}

SkOpContour* SkOpGlobalState::debugContour(int id) const {
    SkOpContour* contour = fContourHead;
    do {
        if (contour->debugID() == id) {
            return contour;
        }
    } while ((contour = contour->next()));
    return nullptr;
}

const SkOpPtT* SkOpGlobalState::debugPtT(int id) const {
    const SkOpContour* contour = fContourHead;
    do {
        const SkOpSegment* segment = contour->first();
        while (segment) {
            const SkOpSpan* span = segment->head();
            do {
                const SkOpPtT* ptT = span->ptT();
                if (ptT->debugMatchID(id)) {
                    return ptT;
                }
            } while ((span = span->next()->upCastable()));
            const SkOpSpanBase* tail = segment->tail();
            const SkOpPtT* ptT = tail->ptT();
            if (ptT->debugMatchID(id)) {
                return ptT;
            }
            segment = segment->next();
        }
    } while ((contour = contour->next()));
    return nullptr;
}

const SkOpSegment* SkOpGlobalState::debugSegment(int id) const {
    const SkOpContour* contour = fContourHead;
    do {
        const SkOpSegment* segment = contour->first();
        while (segment) {
            if (segment->debugID() == id) {
                return segment;
            }
            segment = segment->next();
        }
    } while ((contour = contour->next()));
    return nullptr;
}

const SkOpSpanBase* SkOpGlobalState::debugSpan(int id) const {
    const SkOpContour* contour = fContourHead;
    do {
        const SkOpSegment* segment = contour->first();
        while (segment) {
            const SkOpSpan* span = segment->head();
            do {
                if (span->debugID() == id) {
                    return span;
                }
            } while ((span = span->next()->upCastable()));
            const SkOpSpanBase* tail = segment->tail();
            if (tail->debugID() == id) {
                return tail;
            }
            segment = segment->next();
        }
    } while ((contour = contour->next()));
    return nullptr;
}
#endif

char SkTCoincident::dumpIsCoincidentStr() const {
    if (!!fMatch != fMatch) {
        return '?';
    }
    return fMatch ? '*' : 0;
}

void SkTCoincident::dump() const {
    SkDebugf("t=%1.9g pt=(%1.9g,%1.9g)%s\n", fPerpT, fPerpPt.fX, fPerpPt.fY,
            fMatch ? " match" : "");
}

#ifdef SK_DEBUG

const SkTSpan* SkTSect::debugSpan(int id) const {
    const SkTSpan* test = fHead;
    do {
        if (test->debugID() == id) {
            return test;
        }
    } while ((test = test->next()));
    return nullptr;
}

const SkTSpan* SkTSect::debugT(double t) const {
    const SkTSpan* test = fHead;
    const SkTSpan* closest = nullptr;
    double bestDist = DBL_MAX;
    do {
        if (between(test->fStartT, t, test->fEndT)) {
            return test;
        }
        double testDist = std::min(fabs(test->fStartT - t), fabs(test->fEndT - t));
        if (bestDist > testDist) {
            bestDist = testDist;
            closest = test;
        }
    } while ((test = test->next()));
    SkASSERT(closest);
    return closest;
}

#endif

void SkTSect::dump() const {
    dumpCommon(fHead);
}

extern int gDumpTSectNum;

void SkTSect::dumpBoth(SkTSect* opp) const {
#if DEBUG_T_SECT_DUMP <= 2
#if DEBUG_T_SECT_DUMP == 2
    SkDebugf("%d ", ++gDumpTSectNum);
#endif
    this->dump();
    SkDebugf("\n");
    opp->dump();
    SkDebugf("\n");
#elif DEBUG_T_SECT_DUMP == 3
    SkDebugf("<div id=\"sect%d\">\n", ++gDumpTSectNum);
    if (this->fHead) {
        this->dumpCurves();
    }
    if (opp->fHead) {
        opp->dumpCurves();
    }
    SkDebugf("</div>\n\n");
#endif
}

void SkTSect::dumpBounded(int id) const {
#ifdef SK_DEBUG
    const SkTSpan* bounded = debugSpan(id);
    if (!bounded) {
        SkDebugf("no span matches %d\n", id);
        return;
    }
    const SkTSpan* test = bounded->debugOpp()->fHead;
    do {
        if (test->findOppSpan(bounded)) {
            test->dump();
            SkDebugf(" ");
        }
    } while ((test = test->next()));
    SkDebugf("\n");
#endif
}

void SkTSect::dumpBounds() const {
    const SkTSpan* test = fHead;
    do {
        test->dumpBounds();
    } while ((test = test->next()));
}

void SkTSect::dumpCoin() const {
    dumpCommon(fCoincident);
}

void SkTSect::dumpCoinCurves() const {
    dumpCommonCurves(fCoincident);
}

void SkTSect::dumpCommon(const SkTSpan* test) const {
    SkDebugf("id=%d", debugID());
    if (!test) {
        SkDebugf(" (empty)");
        return;
    }
    do {
        SkDebugf(" ");
        test->dump();
    } while ((test = test->next()));
}

void SkTSect::dumpCommonCurves(const SkTSpan* test) const {
#if DEBUG_T_SECT
    do {
        test->fPart->dumpID(test->debugID());
    } while ((test = test->next()));
#endif
}

void SkTSect::dumpCurves() const {
    dumpCommonCurves(fHead);
}

#ifdef SK_DEBUG

const SkTSpan* SkTSpan::debugSpan(int id) const {
    return fDebugSect->debugSpan(id);
}

const SkTSpan* SkTSpan::debugT(double t) const {
    return fDebugSect->debugT(t);
}

#endif

void SkTSpan::dumpAll() const {
    dumpID();
    SkDebugf("=(%g,%g) [", fStartT, fEndT);
    const SkTSpanBounded* testBounded = fBounded;
    while (testBounded) {
        const SkTSpan* span = testBounded->fBounded;
        const SkTSpanBounded* next = testBounded->fNext;
        span->dumpID();
        SkDebugf("=(%g,%g)", span->fStartT, span->fEndT);
        if (next) {
            SkDebugf(" ");
        }
        testBounded = next;
    }
    SkDebugf("]\n");
}

void SkTSpan::dump() const {
    dumpID();
    SkDebugf("=(%g,%g) [", fStartT, fEndT);
    const SkTSpanBounded* testBounded = fBounded;
    while (testBounded) {
        const SkTSpan* span = testBounded->fBounded;
        const SkTSpanBounded* next = testBounded->fNext;
        span->dumpID();
        if (next) {
            SkDebugf(",");
        }
        testBounded = next;
    }
    SkDebugf("]");
}

void SkTSpan::dumpBounded(int id) const {
    SkDEBUGCODE(fDebugSect->dumpBounded(id));
}

void SkTSpan::dumpBounds() const {
    dumpID();
    SkDebugf(" bounds=(%1.9g,%1.9g, %1.9g,%1.9g) boundsMax=%1.9g%s\n",
            fBounds.fLeft, fBounds.fTop, fBounds.fRight, fBounds.fBottom, fBoundsMax,
            fCollapsed ? " collapsed" : "");
}

void SkTSpan::dumpCoin() const {
    dumpID();
    SkDebugf(" coinStart ");
    fCoinStart.dump();
    SkDebugf(" coinEnd ");
    fCoinEnd.dump();
}

void SkTSpan::dumpID() const {
    char cS = fCoinStart.dumpIsCoincidentStr();
    if (cS) {
        SkDebugf("%c", cS);
    }
    SkDebugf("%d", debugID());
    char cE = fCoinEnd.dumpIsCoincidentStr();
    if (cE) {
        SkDebugf("%c", cE);
    }
}

#if DEBUG_T_SECT_DUMP > 1
int gDumpTSectNum;
#endif

// global path dumps for msvs Visual Studio 17 to use from Immediate Window
void Dump(const SkOpContour& contour) {
    contour.dump();
}

void DumpAll(const SkOpContour& contour) {
    contour.dumpAll();
}

void DumpAngles(const SkOpContour& contour) {
    contour.dumpAngles();
}

void DumpContours(const SkOpContour& contour) {
    contour.dumpContours();
}

void DumpContoursAll(const SkOpContour& contour) {
    contour.dumpContoursAll();
}

void DumpContoursAngles(const SkOpContour& contour) {
    contour.dumpContoursAngles();
}

void DumpContoursPts(const SkOpContour& contour) {
    contour.dumpContoursPts();
}

void DumpContoursPt(const SkOpContour& contour, int segmentID) {
    contour.dumpContoursPt(segmentID);
}

void DumpContoursSegment(const SkOpContour& contour, int segmentID) {
    contour.dumpContoursSegment(segmentID);
}

void DumpContoursSpan(const SkOpContour& contour, int segmentID) {
    contour.dumpContoursSpan(segmentID);
}

void DumpContoursSpans(const SkOpContour& contour) {
    contour.dumpContoursSpans();
}

void DumpPt(const SkOpContour& contour, int pt) {
    contour.dumpPt(pt);
}

void DumpPts(const SkOpContour& contour, const char* prefix) {
    contour.dumpPts(prefix);
}

void DumpSegment(const SkOpContour& contour, int seg) {
    contour.dumpSegment(seg);
}

void DumpSegments(const SkOpContour& contour, const char* prefix, SkPathOp op) {
    contour.dumpSegments(prefix, op);
}

void DumpSpan(const SkOpContour& contour, int span) {
    contour.dumpSpan(span);
}

void DumpSpans(const SkOpContour& contour ) {
    contour.dumpSpans();
}

void Dump(const SkOpSegment& segment) {
    segment.dump();
}

void DumpAll(const SkOpSegment& segment) {
    segment.dumpAll();
}

void DumpAngles(const SkOpSegment& segment) {
    segment.dumpAngles();
}

void DumpCoin(const SkOpSegment& segment) {
    segment.dumpCoin();
}

void DumpPts(const SkOpSegment& segment, const char* prefix) {
    segment.dumpPts(prefix);
}

void Dump(const SkOpPtT& ptT) {
    ptT.dump();
}

void DumpAll(const SkOpPtT& ptT) {
    ptT.dumpAll();
}

void Dump(const SkOpSpanBase& spanBase) {
    spanBase.dump();
}

void DumpCoin(const SkOpSpanBase& spanBase) {
    spanBase.dumpCoin();
}

void DumpAll(const SkOpSpanBase& spanBase) {
    spanBase.dumpAll();
}

void DumpCoin(const SkOpSpan& span) {
    span.dumpCoin();
}

bool DumpSpan(const SkOpSpan& span) {
    return span.dumpSpan();
}

void Dump(const SkDConic& conic) {
    conic.dump();
}

void DumpID(const SkDConic& conic, int id) {
    conic.dumpID(id);
}

void Dump(const SkDCubic& cubic) {
    cubic.dump();
}

void DumpID(const SkDCubic& cubic, int id) {
    cubic.dumpID(id);
}

void Dump(const SkDLine& line) {
    line.dump();
}

void DumpID(const SkDLine& line, int id) {
    line.dumpID(id);
}

void Dump(const SkDQuad& quad) {
    quad.dump();
}

void DumpID(const SkDQuad& quad, int id) {
    quad.dumpID(id);
}

void Dump(const SkDPoint& point) {
    point.dump();
}

void Dump(const SkOpAngle& angle) {
    angle.dump();
}

void SkOpSegment::debugAddAngle(double startT, double endT) {
    SkOpPtT* startPtT = startT == 0 ? fHead.ptT() : startT == 1 ? fTail.ptT()
            : this->addT(startT);
    SkOpPtT* endPtT = endT == 0 ? fHead.ptT() : endT == 1 ? fTail.ptT()
            : this->addT(endT);
    SkOpAngle* angle = this->globalState()->allocator()->make<SkOpAngle>();
    SkOpSpanBase* startSpan = &fHead;
    while (startSpan->ptT() != startPtT) {
        startSpan = startSpan->upCast()->next();
    }
    SkOpSpanBase* endSpan = &fHead;
    while (endSpan->ptT() != endPtT) {
        endSpan = endSpan->upCast()->next();
    }
    angle->set(startSpan, endSpan);
    if (startT < endT) {
        startSpan->upCast()->setToAngle(angle);
        endSpan->setFromAngle(angle);
    } else {
        endSpan->upCast()->setToAngle(angle);
        startSpan->setFromAngle(angle);
    }
}
