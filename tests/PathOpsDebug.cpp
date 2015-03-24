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

#if DEBUG_SHOW_TEST_NAME

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

static void show_function_header(const char* functionName) {
    SkDebugf("\nstatic void %s(skiatest::Reporter* reporter, const char* filename) {\n", functionName);
    if (strcmp("skphealth_com76", functionName) == 0) {
        SkDebugf("found it\n");
    }
}

static const char* gOpStrs[] = {
    "kDifference_PathOp",
    "kIntersect_PathOp",
    "kUnion_PathOp",
    "kXor_PathOp",
    "kReverseDifference_PathOp",
};

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
#endif

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

void SkDCubic::dump() const {
    dumpInner();
    SkDebugf("}},\n");
}

void SkDCubic::dumpID(int id) const {
    dumpInner();
    SkDebugf("}} id=%d\n", id);
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

void SkDLine::dump() const {
    SkDebugf("{{");
    fPts[0].dump();
    SkDebugf(", ");
    fPts[1].dump();
    SkDebugf("}},\n");
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
    SkDebugf("}} id=%d\n", id);
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

void SkPathOpsDebug::DumpContours(SkTDArray<SkOpContour* >* contours) {
    int count = contours->count();
    for (int index = 0; index < count; ++index) {
        (*contours)[index]->dump();
    }
}

void SkPathOpsDebug::DumpContoursAll(SkTDArray<SkOpContour* >* contours) {
    int count = contours->count();
    for (int index = 0; index < count; ++index) {
        (*contours)[index]->dumpAll();
    }
}

void SkPathOpsDebug::DumpContoursAngles(const SkTDArray<SkOpContour* >* contours) {
    int count = contours->count();
    for (int index = 0; index < count; ++index) {
        (*contours)[index]->dumpAngles();
    }
}

void SkPathOpsDebug::DumpContoursPts(const SkTDArray<SkOpContour* >* contours) {
    int count = contours->count();
    for (int index = 0; index < count; ++index) {
        (*contours)[index]->dumpPts();
    }
}

void SkPathOpsDebug::DumpContoursPt(const SkTDArray<SkOpContour* >* contours, int segmentID) {
    int count = contours->count();
    for (int index = 0; index < count; ++index) {
        (*contours)[index]->dumpPt(segmentID);
    }
}

void SkPathOpsDebug::DumpContoursSegment(const SkTDArray<SkOpContour* >* contours,
        int segmentID) {
    if (contours->count()) {
        (*contours)[0]->dumpSegment(segmentID);
    }
}

void SkPathOpsDebug::DumpContoursSpan(const SkTDArray<SkOpContour* >* contours,
        int spanID) {
    if (contours->count()) {
        (*contours)[0]->dumpSpan(spanID);
    }
}

void SkPathOpsDebug::DumpContoursSpans(const SkTDArray<SkOpContour* >* contours) {
    int count = contours->count();
    for (int index = 0; index < count; ++index) {
        (*contours)[index]->dumpSpans();
    }
}

const SkTSpan<SkDCubic>* DebugSpan(const SkTSect<SkDCubic>* sect, int id) {
    return sect->debugSpan(id);
}

const SkTSpan<SkDQuad>* DebugSpan(const SkTSect<SkDQuad>* sect, int id) {
    return sect->debugSpan(id);
}

const SkTSpan<SkDCubic>* DebugT(const SkTSect<SkDCubic>* sect, double t) {
    return sect->debugT(t);
}

const SkTSpan<SkDQuad>* DebugT(const SkTSect<SkDQuad>* sect, double t) {
    return sect->debugT(t);
}

const SkTSpan<SkDCubic>* DebugSpan(const SkTSpan<SkDCubic>* span, int id) {
    return span->debugSpan(id);
}

const SkTSpan<SkDQuad>* DebugSpan(const SkTSpan<SkDQuad>* span, int id) {
    return span->debugSpan(id);
}

const SkTSpan<SkDCubic>* DebugT(const SkTSpan<SkDCubic>* span, double t) {
    return span->debugT(t);
}

const SkTSpan<SkDQuad>* DebugT(const SkTSpan<SkDQuad>* span, double t) {
    return span->debugT(t);
}

void Dump(const SkTSect<SkDCubic>* sect) {
    sect->dump();
}

void Dump(const SkTSect<SkDQuad>* sect) {
    sect->dump();
}

void Dump(const SkTSpan<SkDCubic>* span) {
    span->dump();
}

void Dump(const SkTSpan<SkDQuad>* span) {
    span->dump();
}

void DumpBoth(SkTSect<SkDCubic>* sect1, SkTSect<SkDCubic>* sect2) {
    sect1->dumpBoth(sect2);
}

void DumpBoth(SkTSect<SkDQuad>* sect1, SkTSect<SkDQuad>* sect2) {
    sect1->dumpBoth(sect2);
}

void DumpCoin(SkTSect<SkDCubic>* sect1) {
    sect1->dumpCoin();
}

void DumpCoin(SkTSect<SkDQuad>* sect1) {
    sect1->dumpCoin();
}

void DumpCoinCurves(SkTSect<SkDCubic>* sect1) {
    sect1->dumpCoinCurves();
}

void DumpCoinCurves(SkTSect<SkDQuad>* sect1) {
    sect1->dumpCoinCurves();
}

void DumpCurves(const SkTSect<SkDQuad>* sect) {
    sect->dumpCurves();
}

void DumpCurves(const SkTSect<SkDCubic>* sect) {
    sect->dumpCurves();
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

SkOpContour* SkOpAngle::debugContour(int id) {
    return this->segment()->debugContour(id);
}

const SkOpPtT* SkOpAngle::debugPtT(int id) const {
    return this->segment()->debugPtT(id);
}

const SkOpSegment* SkOpAngle::debugSegment(int id) const {
    return this->segment()->debugSegment(id);
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
    SkDebugf(" sgn=%d windVal=%d", this->sign(), mSpan.windValue());

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
    if (fStop) {
        SkDebugf(" stop");
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
        next->fCurvePart.dumpID(next->segment()->debugID());
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

SkOpContour* SkOpPtT::debugContour(int id) {
    return this->span()->debugContour(id);
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
    SkDebugf(" t=%1.9g pt=(%1.9g,%1.9g)%s%s", this->fT, this->fPt.fX, this->fPt.fY,
            this->fDuplicatePt ? " dup" : "", this->fDeleted ? " deleted" : "");
}

const SkOpAngle* SkOpSpanBase::debugAngle(int id) const {
    return this->segment()->debugAngle(id);
}

SkOpContour* SkOpSpanBase::debugContour(int id) {
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
    this->dumpAll();
    SkDebugf("\n");
}

void SkOpSpanBase::dumpAll() const {
    SkDebugf("%.*s", contour()->debugIndent(), "        ");
    SkDebugf("seg=%d span=%d", this->segment()->debugID(), this->debugID());
    this->dumpBase();
    SkDebugf("\n");
    this->fPtT.dumpAll();
}

void SkOpSpanBase::dumpBase() const {
    if (this->fAligned) {
        SkDebugf(" aligned");
    }
    if (this->fChased) {
        SkDebugf(" chased");
    }
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

SkOpContour* SkOpSegment::debugContour(int id) {
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
        const SkOpAngle* tAngle = span->final() ? NULL : span->upCast()->toAngle();
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

void SkOpSegment::dumpPts() const {
    int last = SkPathOpsVerbToPoints(fVerb);
    SkDebugf("seg=%d {{", this->debugID());
    int index = 0;
    do {
        SkDPoint::Dump(fPts[index]);
        SkDebugf(", ");
    } while (++index < last);
    SkDPoint::Dump(fPts[index]);
    SkDebugf("}}\n");
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
        span = span->fNext;
    }
}

void SkOpContour::dump() {
    SkDebugf("contour=%d count=%d\n", this->debugID(), fCount);
    if (!fCount) {
        return;
    }
    const SkOpSegment* segment = &fHead;
    PATH_OPS_DEBUG_CODE(fIndent = 0);
    indentDump();
    do {
        segment->dump();
    } while ((segment = segment->next()));
    outdentDump();
}

void SkOpContour::dumpAll() {
    SkDebugf("contour=%d count=%d\n", this->debugID(), fCount);
    if (!fCount) {
        return;
    }
    const SkOpSegment* segment = &fHead;
    PATH_OPS_DEBUG_CODE(fIndent = 0);
    indentDump();
    do {
        segment->dumpAll();
    } while ((segment = segment->next()));
    outdentDump();
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

void SkOpContour::dumpPts() const {
    SkDebugf("contour=%d\n", this->debugID());
    const SkOpSegment* segment = &fHead;
    do {
        SkDebugf("  seg=%d ", segment->debugID());
        segment->dumpPts();
    } while ((segment = segment->next()));
}

void SkOpContour::dumpPtsX() const {
    if (!this->fCount) {
        SkDebugf("<empty>\n");
        return;
    }
    const SkOpSegment* segment = &fHead;
    do {
        segment->dumpPts();
    } while ((segment = segment->next()));
}

void SkOpContour::dumpSegment(int index) const {
    debugSegment(index)->dump();
}

void SkOpContour::dumpSegments(SkPathOp op) const {
    bool firstOp = false;
    const SkOpContour* c = this;
    do {
        if (!firstOp && c->operand()) {
#if DEBUG_ACTIVE_OP
            SkDebugf("op %s\n", SkPathOpsDebug::kPathOpStr[op]);
#endif
            firstOp = true;
        }
        c->dumpPtsX();
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

#ifdef SK_DEBUG
const SkOpAngle* SkOpGlobalState::debugAngle(int id) const {
    const SkOpContour* contour = fHead;
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
    return NULL;
}

SkOpContour* SkOpGlobalState::debugContour(int id) {
    SkOpContour* contour = fHead;
    do {
        if (contour->debugID() == id) {
            return contour;
        }
    } while ((contour = contour->next()));
    return NULL;
}

const SkOpPtT* SkOpGlobalState::debugPtT(int id) const {
    const SkOpContour* contour = fHead;
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
    return NULL;
}

const SkOpSegment* SkOpGlobalState::debugSegment(int id) const {
    const SkOpContour* contour = fHead;
    do {
        const SkOpSegment* segment = contour->first();
        while (segment) {
            if (segment->debugID() == id) {
                return segment;
            }
            segment = segment->next();
        }
    } while ((contour = contour->next()));
    return NULL;
}

const SkOpSpanBase* SkOpGlobalState::debugSpan(int id) const {
    const SkOpContour* contour = fHead;
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
    return NULL;
}
#endif

const SkOpAngle* DebugAngle(const SkTArray<SkOpContour*, true>* contours, int id) {
    return (*contours)[0]->debugAngle(id);
}

SkOpContour* DumpContour(const SkTArray<SkOpContour*, true>* contours, int id) {
    return (*contours)[0]->debugContour(id);
}

const SkOpPtT* DebugPtT(const SkTArray<SkOpContour*, true>* contours, int id) {
    return (*contours)[0]->debugPtT(id);
}

const SkOpSegment* DebugSegment(const SkTArray<SkOpContour*, true>* contours, int id) {
    return (*contours)[0]->debugSegment(id);
}

const SkOpSpanBase* DebugSpan(const SkTArray<SkOpContour*, true>* contours, int id) {
    return (*contours)[0]->debugSpan(id);
}

void Dump(SkTDArray<SkOpContour* >* contours) {
    SkPathOpsDebug::DumpContours(contours);
}

void DumpAll(SkTDArray<SkOpContour* >* contours) {
    SkPathOpsDebug::DumpContoursAll(contours);
}

void DumpAngles(const SkTDArray<SkOpContour* >* contours) {
    SkPathOpsDebug::DumpContoursAngles(contours);
}

void DumpSegment(const SkTDArray<SkOpContour* >* contours, int segmentID) {
    SkPathOpsDebug::DumpContoursSegment(contours, segmentID);
}

void DumpSpan(const SkTDArray<SkOpContour* >* contours, int spanID) {
    SkPathOpsDebug::DumpContoursSpan(contours, spanID);
}

void DumpSpans(const SkTDArray<SkOpContour* >* contours) {
    SkPathOpsDebug::DumpContoursSpans(contours);
}

void DumpPt(const SkTDArray<SkOpContour* >* contours, int segmentID) {
    SkPathOpsDebug::DumpContoursPt(contours, segmentID);
}

void DumpPts(const SkTDArray<SkOpContour* >* contours) {
    SkPathOpsDebug::DumpContoursPts(contours);
}

#if DEBUG_T_SECT_DUMP > 1
int gDumpTSectNum;
#endif
