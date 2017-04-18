/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "PathOpsTSectDebug.h"
#include "SkOpCoincidence.h"
#include "SkOpContour.h"
#include "SkIntersectionHelper.h"
#include "SkMutex.h"
#include "SkOpSegment.h"
#include "SkString.h"

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

const SkOpAngle* SkPathOpsDebug::DebugAngleAngle(const SkOpAngle* angle, int id) {
    return angle->debugAngle(id);
}

SkOpContour* SkPathOpsDebug::DebugAngleContour(SkOpAngle* angle, int id) {
    return angle->debugContour(id);
}

const SkOpPtT* SkPathOpsDebug::DebugAnglePtT(const SkOpAngle* angle, int id) {
    return angle->debugPtT(id);
}

const SkOpSegment* SkPathOpsDebug::DebugAngleSegment(const SkOpAngle* angle, int id) {
    return angle->debugSegment(id);
}

const SkOpSpanBase* SkPathOpsDebug::DebugAngleSpan(const SkOpAngle* angle, int id) {
    return angle->debugSpan(id);
}

const SkOpAngle* SkPathOpsDebug::DebugContourAngle(SkOpContour* contour, int id) {
    return contour->debugAngle(id);
}

SkOpContour* SkPathOpsDebug::DebugContourContour(SkOpContour* contour, int id) {
    return contour->debugContour(id);
}

const SkOpPtT* SkPathOpsDebug::DebugContourPtT(SkOpContour* contour, int id) {
    return contour->debugPtT(id);
}

const SkOpSegment* SkPathOpsDebug::DebugContourSegment(SkOpContour* contour, int id) {
    return contour->debugSegment(id);
}

const SkOpSpanBase* SkPathOpsDebug::DebugContourSpan(SkOpContour* contour, int id) {
    return contour->debugSpan(id);
}

const SkOpAngle* SkPathOpsDebug::DebugCoincidenceAngle(SkOpCoincidence* coin, int id) {
    return coin->debugAngle(id);
}

SkOpContour* SkPathOpsDebug::DebugCoincidenceContour(SkOpCoincidence* coin, int id) {
    return coin->debugContour(id);
}

const SkOpPtT* SkPathOpsDebug::DebugCoincidencePtT(SkOpCoincidence* coin, int id) {
    return coin->debugPtT(id);
}

const SkOpSegment* SkPathOpsDebug::DebugCoincidenceSegment(SkOpCoincidence* coin, int id) {
    return coin->debugSegment(id);
}

const SkOpSpanBase* SkPathOpsDebug::DebugCoincidenceSpan(SkOpCoincidence* coin, int id) {
    return coin->debugSpan(id);
}

const SkOpAngle* SkPathOpsDebug::DebugPtTAngle(const SkOpPtT* ptT, int id) {
    return ptT->debugAngle(id);
}

SkOpContour* SkPathOpsDebug::DebugPtTContour(SkOpPtT* ptT, int id) {
    return ptT->debugContour(id);
}

const SkOpPtT* SkPathOpsDebug::DebugPtTPtT(const SkOpPtT* ptT, int id) {
    return ptT->debugPtT(id);
}

const SkOpSegment* SkPathOpsDebug::DebugPtTSegment(const SkOpPtT* ptT, int id) {
    return ptT->debugSegment(id);
}

const SkOpSpanBase* SkPathOpsDebug::DebugPtTSpan(const SkOpPtT* ptT, int id) {
    return ptT->debugSpan(id);
}

const SkOpAngle* SkPathOpsDebug::DebugSegmentAngle(const SkOpSegment* span, int id) {
    return span->debugAngle(id);
}

SkOpContour* SkPathOpsDebug::DebugSegmentContour(SkOpSegment* span, int id) {
    return span->debugContour(id);
}

const SkOpPtT* SkPathOpsDebug::DebugSegmentPtT(const SkOpSegment* span, int id) {
    return span->debugPtT(id);
}

const SkOpSegment* SkPathOpsDebug::DebugSegmentSegment(const SkOpSegment* span, int id) {
    return span->debugSegment(id);
}

const SkOpSpanBase* SkPathOpsDebug::DebugSegmentSpan(const SkOpSegment* span, int id) {
    return span->debugSpan(id);
}

const SkOpAngle* SkPathOpsDebug::DebugSpanAngle(const SkOpSpanBase* span, int id) {
    return span->debugAngle(id);
}

SkOpContour* SkPathOpsDebug::DebugSpanContour(SkOpSpanBase* span, int id) {
    return span->debugContour(id);
}

const SkOpPtT* SkPathOpsDebug::DebugSpanPtT(const SkOpSpanBase* span, int id) {
    return span->debugPtT(id);
}

const SkOpSegment* SkPathOpsDebug::DebugSpanSegment(const SkOpSpanBase* span, int id) {
    return span->debugSegment(id);
}

const SkOpSpanBase* SkPathOpsDebug::DebugSpanSpan(const SkOpSpanBase* span, int id) {
    return span->debugSpan(id);
}

#if DEBUG_COIN
void SkPathOpsDebug::DumpCoinDict() {
    gCoinSumChangedDict.dump("unused coin algorithm", false);
    gCoinSumVisitedDict.dump("visited coin function", true);
}

void SkPathOpsDebug::CoinDict::dump(const char* str, bool visitCheck) const {
    int count = fDict.count();
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

template <typename TCurve, typename OppCurve>
const SkTSpan<TCurve, OppCurve>* DebugSpan(const SkTSect<TCurve, OppCurve>* sect, int id) {
    return sect->debugSpan(id);
}

void DontCallDebugSpan(int id);
void DontCallDebugSpan(int id) {  // exists to instantiate the templates
    SkDQuad quad;
    SkDConic conic;
    SkDCubic cubic;
    SkTSect<SkDQuad, SkDQuad> q1q2(quad  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDQuad, SkDConic> q1k2(quad  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDQuad, SkDCubic> q1c2(quad  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDConic, SkDQuad> k1q2(conic  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDConic, SkDConic> k1k2(conic  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDConic, SkDCubic> k1c2(conic  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDCubic, SkDQuad> c1q2(cubic  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDCubic, SkDConic> c1k2(cubic  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDCubic, SkDCubic> c1c2(cubic  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    DebugSpan(&q1q2, id);
    DebugSpan(&q1k2, id);
    DebugSpan(&q1c2, id);
    DebugSpan(&k1q2, id);
    DebugSpan(&k1k2, id);
    DebugSpan(&k1c2, id);
    DebugSpan(&c1q2, id);
    DebugSpan(&c1k2, id);
    DebugSpan(&c1c2, id);
}

template <typename TCurve, typename OppCurve>
const SkTSpan<TCurve, OppCurve>* DebugT(const SkTSect<TCurve, OppCurve>* sect, double t) {
    return sect->debugT(t);
}

void DontCallDebugT(double t);
void DontCallDebugT(double t) {  // exists to instantiate the templates
    SkDQuad quad;
    SkDConic conic;
    SkDCubic cubic;
    SkTSect<SkDQuad, SkDQuad> q1q2(quad  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDQuad, SkDConic> q1k2(quad  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDQuad, SkDCubic> q1c2(quad  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDConic, SkDQuad> k1q2(conic  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDConic, SkDConic> k1k2(conic  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDConic, SkDCubic> k1c2(conic  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDCubic, SkDQuad> c1q2(cubic  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDCubic, SkDConic> c1k2(cubic  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDCubic, SkDCubic> c1c2(cubic  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    DebugT(&q1q2, t);
    DebugT(&q1k2, t);
    DebugT(&q1c2, t);
    DebugT(&k1q2, t);
    DebugT(&k1k2, t);
    DebugT(&k1c2, t);
    DebugT(&c1q2, t);
    DebugT(&c1k2, t);
    DebugT(&c1c2, t);
}

template <typename TCurve, typename OppCurve>
void Dump(const SkTSect<TCurve, OppCurve>* sect) {
    sect->dump();
}

void DontCallDumpTSect();
void DontCallDumpTSect() {  // exists to instantiate the templates
    SkDQuad quad;
    SkDConic conic;
    SkDCubic cubic;
    SkTSect<SkDQuad, SkDQuad> q1q2(quad  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDQuad, SkDConic> q1k2(quad  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDQuad, SkDCubic> q1c2(quad  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDConic, SkDQuad> k1q2(conic  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDConic, SkDConic> k1k2(conic  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDConic, SkDCubic> k1c2(conic  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDCubic, SkDQuad> c1q2(cubic  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDCubic, SkDConic> c1k2(cubic  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDCubic, SkDCubic> c1c2(cubic  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    Dump(&q1q2);
    Dump(&q1k2);
    Dump(&q1c2);
    Dump(&k1q2);
    Dump(&k1k2);
    Dump(&k1c2);
    Dump(&c1q2);
    Dump(&c1k2);
    Dump(&c1c2);
}

template <typename TCurve, typename OppCurve>
void DumpBoth(SkTSect<TCurve, OppCurve>* sect1, SkTSect<OppCurve, TCurve>* sect2) {
    sect1->dumpBoth(sect2);
}

void DontCallDumpBoth();
void DontCallDumpBoth() {  // exists to instantiate the templates
    SkDQuad quad;
    SkDConic conic;
    SkDCubic cubic;
    SkTSect<SkDQuad, SkDQuad> q1q2(quad  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDQuad, SkDConic> q1k2(quad  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDQuad, SkDCubic> q1c2(quad  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDConic, SkDQuad> k1q2(conic  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDConic, SkDConic> k1k2(conic  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDConic, SkDCubic> k1c2(conic  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDCubic, SkDQuad> c1q2(cubic  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDCubic, SkDConic> c1k2(cubic  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDCubic, SkDCubic> c1c2(cubic  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    DumpBoth(&q1q2, &q1q2);
    DumpBoth(&q1k2, &k1q2);
    DumpBoth(&q1c2, &c1q2);
    DumpBoth(&k1q2, &q1k2);
    DumpBoth(&k1k2, &k1k2);
    DumpBoth(&k1c2, &c1k2);
    DumpBoth(&c1q2, &q1c2);
    DumpBoth(&c1k2, &k1c2);
    DumpBoth(&c1c2, &c1c2);
}

template <typename TCurve, typename OppCurve>
void DumpBounded(SkTSect<TCurve, OppCurve>* sect1, int id) {
    sect1->dumpBounded(id);
}

void DontCallDumpBounded();
void DontCallDumpBounded() {
    SkDQuad quad;
    SkDConic conic;
    SkDCubic cubic;
    SkTSect<SkDQuad, SkDQuad> q1q2(quad  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDQuad, SkDConic> q1k2(quad  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDQuad, SkDCubic> q1c2(quad  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDConic, SkDQuad> k1q2(conic  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDConic, SkDConic> k1k2(conic  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDConic, SkDCubic> k1c2(conic  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDCubic, SkDQuad> c1q2(cubic  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDCubic, SkDConic> c1k2(cubic  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDCubic, SkDCubic> c1c2(cubic  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    DumpBounded(&q1q2, 0);
    DumpBounded(&q1k2, 0);
    DumpBounded(&q1c2, 0);
    DumpBounded(&k1q2, 0);
    DumpBounded(&k1k2, 0);
    DumpBounded(&k1c2, 0);
    DumpBounded(&c1q2, 0);
    DumpBounded(&c1k2, 0);
    DumpBounded(&c1c2, 0);
}

template <typename TCurve, typename OppCurve>
void DumpBounds(SkTSect<TCurve, OppCurve>* sect1) {
    sect1->dumpBounds();
}

void DontCallDumpBounds();
void DontCallDumpBounds() {
    SkDQuad quad;
    SkDConic conic;
    SkDCubic cubic;
    SkTSect<SkDQuad, SkDQuad> q1q2(quad  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDQuad, SkDConic> q1k2(quad  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDQuad, SkDCubic> q1c2(quad  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDConic, SkDQuad> k1q2(conic  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDConic, SkDConic> k1k2(conic  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDConic, SkDCubic> k1c2(conic  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDCubic, SkDQuad> c1q2(cubic  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDCubic, SkDConic> c1k2(cubic  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDCubic, SkDCubic> c1c2(cubic  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    DumpBounds(&q1q2);
    DumpBounds(&q1k2);
    DumpBounds(&q1c2);
    DumpBounds(&k1q2);
    DumpBounds(&k1k2);
    DumpBounds(&k1c2);
    DumpBounds(&c1q2);
    DumpBounds(&c1k2);
    DumpBounds(&c1c2);
}

template <typename TCurve, typename OppCurve>
void DumpCoin(SkTSect<TCurve, OppCurve>* sect1) {
    sect1->dumpCoin();
}

void DontCallDumpCoin();
void DontCallDumpCoin() {  // exists to instantiate the templates
    SkDQuad quad;
    SkDConic conic;
    SkDCubic cubic;
    SkTSect<SkDQuad, SkDQuad> q1q2(quad  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDQuad, SkDConic> q1k2(quad  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDQuad, SkDCubic> q1c2(quad  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDConic, SkDQuad> k1q2(conic  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDConic, SkDConic> k1k2(conic  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDConic, SkDCubic> k1c2(conic  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDCubic, SkDQuad> c1q2(cubic  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDCubic, SkDConic> c1k2(cubic  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDCubic, SkDCubic> c1c2(cubic  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    DumpCoin(&q1q2);
    DumpCoin(&q1k2);
    DumpCoin(&q1c2);
    DumpCoin(&k1q2);
    DumpCoin(&k1k2);
    DumpCoin(&k1c2);
    DumpCoin(&c1q2);
    DumpCoin(&c1k2);
    DumpCoin(&c1c2);
}

template <typename TCurve, typename OppCurve>
void DumpCoinCurves(SkTSect<TCurve, OppCurve>* sect1) {
    sect1->dumpCoinCurves();
}

void DontCallDumpCoinCurves();
void DontCallDumpCoinCurves() {  // exists to instantiate the templates
    SkDQuad quad;
    SkDConic conic;
    SkDCubic cubic;
    SkTSect<SkDQuad, SkDQuad> q1q2(quad  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDQuad, SkDConic> q1k2(quad  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDQuad, SkDCubic> q1c2(quad  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDConic, SkDQuad> k1q2(conic  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDConic, SkDConic> k1k2(conic  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDConic, SkDCubic> k1c2(conic  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDCubic, SkDQuad> c1q2(cubic  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDCubic, SkDConic> c1k2(cubic  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDCubic, SkDCubic> c1c2(cubic  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    DumpCoinCurves(&q1q2);
    DumpCoinCurves(&q1k2);
    DumpCoinCurves(&q1c2);
    DumpCoinCurves(&k1q2);
    DumpCoinCurves(&k1k2);
    DumpCoinCurves(&k1c2);
    DumpCoinCurves(&c1q2);
    DumpCoinCurves(&c1k2);
    DumpCoinCurves(&c1c2);
}

template <typename TCurve, typename OppCurve>
void DumpCurves(const SkTSect<TCurve, OppCurve>* sect) {
    sect->dumpCurves();
}

void DontCallDumpCurves();
void DontCallDumpCurves() {  // exists to instantiate the templates
    SkDQuad quad;
    SkDConic conic;
    SkDCubic cubic;
    SkTSect<SkDQuad, SkDQuad> q1q2(quad  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDQuad, SkDConic> q1k2(quad  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDQuad, SkDCubic> q1c2(quad  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDConic, SkDQuad> k1q2(conic  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDConic, SkDConic> k1k2(conic  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDConic, SkDCubic> k1c2(conic  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDCubic, SkDQuad> c1q2(cubic  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDCubic, SkDConic> c1k2(cubic  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDCubic, SkDCubic> c1c2(cubic  SkDEBUGPARAMS(nullptr)  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    DumpCurves(&q1q2);
    DumpCurves(&q1k2);
    DumpCurves(&q1c2);
    DumpCurves(&k1q2);
    DumpCurves(&k1k2);
    DumpCurves(&k1c2);
    DumpCurves(&c1q2);
    DumpCurves(&c1k2);
    DumpCurves(&c1c2);
}

template <typename TCurve, typename OppCurve>
void Dump(const SkTSpan<TCurve, OppCurve>* span) {
    span->dump();
}

void DontCallDumpTSpan();
void DontCallDumpTSpan() {  // exists to instantiate the templates
    SkTSpan<SkDQuad, SkDQuad> q1q2; q1q2.debugInit();
    SkTSpan<SkDQuad, SkDConic> q1k2; q1k2.debugInit();
    SkTSpan<SkDQuad, SkDCubic> q1c2; q1c2.debugInit();
    SkTSpan<SkDConic, SkDQuad> k1q2; k1q2.debugInit();
    SkTSpan<SkDConic, SkDConic> k1k2; k1k2.debugInit();
    SkTSpan<SkDConic, SkDCubic> k1c2; k1c2.debugInit();
    SkTSpan<SkDCubic, SkDQuad> c1q2; c1q2.debugInit();
    SkTSpan<SkDCubic, SkDConic> c1k2; c1k2.debugInit();
    SkTSpan<SkDCubic, SkDCubic> c1c2; c1c2.debugInit();
    Dump(&q1q2);
    Dump(&q1k2);
    Dump(&q1c2);
    Dump(&k1q2);
    Dump(&k1k2);
    Dump(&k1c2);
    Dump(&c1q2);
    Dump(&c1k2);
    Dump(&c1c2);
}

template <typename TCurve, typename OppCurve>
void DumpAll(const SkTSpan<TCurve, OppCurve>* span) {
    span->dumpAll();
}

void DontCallDumpSpanAll();
void DontCallDumpSpanAll() {  // exists to instantiate the templates
    SkTSpan<SkDQuad, SkDQuad> q1q2; q1q2.debugInit();
    SkTSpan<SkDQuad, SkDConic> q1k2; q1k2.debugInit();
    SkTSpan<SkDQuad, SkDCubic> q1c2; q1c2.debugInit();
    SkTSpan<SkDConic, SkDQuad> k1q2; k1q2.debugInit();
    SkTSpan<SkDConic, SkDConic> k1k2; k1k2.debugInit();
    SkTSpan<SkDConic, SkDCubic> k1c2; k1c2.debugInit();
    SkTSpan<SkDCubic, SkDQuad> c1q2; c1q2.debugInit();
    SkTSpan<SkDCubic, SkDConic> c1k2; c1k2.debugInit();
    SkTSpan<SkDCubic, SkDCubic> c1c2; c1c2.debugInit();
    DumpAll(&q1q2);
    DumpAll(&q1k2);
    DumpAll(&q1c2);
    DumpAll(&k1q2);
    DumpAll(&k1k2);
    DumpAll(&k1c2);
    DumpAll(&c1q2);
    DumpAll(&c1k2);
    DumpAll(&c1c2);
}

template <typename TCurve, typename OppCurve>
void DumpBounded(const SkTSpan<TCurve, OppCurve>* span) {
    span->dumpBounded(0);
}

void DontCallDumpSpanBounded();
void DontCallDumpSpanBounded() {  // exists to instantiate the templates
    SkTSpan<SkDQuad, SkDQuad> q1q2; q1q2.debugInit();
    SkTSpan<SkDQuad, SkDConic> q1k2; q1k2.debugInit();
    SkTSpan<SkDQuad, SkDCubic> q1c2; q1c2.debugInit();
    SkTSpan<SkDConic, SkDQuad> k1q2; k1q2.debugInit();
    SkTSpan<SkDConic, SkDConic> k1k2; k1k2.debugInit();
    SkTSpan<SkDConic, SkDCubic> k1c2; k1c2.debugInit();
    SkTSpan<SkDCubic, SkDQuad> c1q2; c1q2.debugInit();
    SkTSpan<SkDCubic, SkDConic> c1k2; c1k2.debugInit();
    SkTSpan<SkDCubic, SkDCubic> c1c2; c1c2.debugInit();
    DumpBounded(&q1q2);
    DumpBounded(&q1k2);
    DumpBounded(&q1c2);
    DumpBounded(&k1q2);
    DumpBounded(&k1k2);
    DumpBounded(&k1c2);
    DumpBounded(&c1q2);
    DumpBounded(&c1k2);
    DumpBounded(&c1c2);
}

template <typename TCurve, typename OppCurve>
void DumpCoin(const SkTSpan<TCurve, OppCurve>* span) {
    span->dumpCoin();
}

void DontCallDumpSpanCoin();
void DontCallDumpSpanCoin() {  // exists to instantiate the templates
    SkTSpan<SkDQuad, SkDQuad> q1q2; q1q2.debugInit();
    SkTSpan<SkDQuad, SkDConic> q1k2; q1k2.debugInit();
    SkTSpan<SkDQuad, SkDCubic> q1c2; q1c2.debugInit();
    SkTSpan<SkDConic, SkDQuad> k1q2; k1q2.debugInit();
    SkTSpan<SkDConic, SkDConic> k1k2; k1k2.debugInit();
    SkTSpan<SkDConic, SkDCubic> k1c2; k1c2.debugInit();
    SkTSpan<SkDCubic, SkDQuad> c1q2; c1q2.debugInit();
    SkTSpan<SkDCubic, SkDConic> c1k2; c1k2.debugInit();
    SkTSpan<SkDCubic, SkDCubic> c1c2; c1c2.debugInit();
    DumpCoin(&q1q2);
    DumpCoin(&q1k2);
    DumpCoin(&q1c2);
    DumpCoin(&k1q2);
    DumpCoin(&k1k2);
    DumpCoin(&k1c2);
    DumpCoin(&c1q2);
    DumpCoin(&c1k2);
    DumpCoin(&c1c2);
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
        if (!firstOp && c->operand() && op >= 0) {
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

#if DEBUG_T_SECT_DUMP > 1
int gDumpTSectNum;
#endif
