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
    SkDebugf(");\n");
}

static void showPathContours(SkPath::RawIter& iter, const char* pathName) {
    uint8_t verb;
    SkPoint pts[4];
    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kMove_Verb:
                SkDebugf("    %s.moveTo(", pathName);
                output_points(&pts[0], 1);
                continue;
            case SkPath::kLine_Verb:
                SkDebugf("    %s.lineTo(", pathName);
                output_points(&pts[1], 1);
                break;
            case SkPath::kQuad_Verb:
                SkDebugf("    %s.quadTo(", pathName);
                output_points(&pts[1], 2);
                break;
            case SkPath::kCubic_Verb:
                SkDebugf("    %s.cubicTo(", pathName);
                output_points(&pts[1], 3);
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

void SkOpAngle::dump() const {
    dumpOne(true);
    SkDebugf("\n");
}

void SkOpAngle::dumpOne(bool functionHeader) const {
//    fSegment->debugValidate();
    const SkOpSpan& mSpan = fSegment->span(SkMin32(fStart, fEnd));
    if (functionHeader) {
        SkDebugf("%s ", __FUNCTION__);
    }
    SkDebugf("[%d", fSegment->debugID());
    SkDebugf("/%d", debugID());
    SkDebugf("] next=");
    if (fNext) {
        SkDebugf("%d", fNext->fSegment->debugID());
        SkDebugf("/%d", fNext->debugID());
    } else {
        SkDebugf("?");
    }
    SkDebugf(" sect=%d/%d ", fSectorStart, fSectorEnd);
    SkDebugf(" s=%1.9g [%d] e=%1.9g [%d]", fSegment->span(fStart).fT, fStart,
            fSegment->span(fEnd).fT, fEnd);
    SkDebugf(" sgn=%d windVal=%d", sign(), mSpan.fWindValue);

    SkDebugf(" windSum=");
    SkPathOpsDebug::WindingPrintf(mSpan.fWindSum);
    if (mSpan.fOppValue != 0 || mSpan.fOppSum != SK_MinS32) {
        SkDebugf(" oppVal=%d", mSpan.fOppValue);
        SkDebugf(" oppSum=");
        SkPathOpsDebug::WindingPrintf(mSpan.fOppSum);
    }
    if (mSpan.fDone) {
        SkDebugf(" done");
    }
    if (unorderable()) {
        SkDebugf(" unorderable");
    }
    if (small()) {
        SkDebugf(" small");
    }
    if (mSpan.fTiny) {
        SkDebugf(" tiny");
    }
    if (fSegment->operand()) {
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
        if (segment == next->fSegment) {
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

void SkOpAngle::dumpLoop() const {
    const SkOpAngle* first = this;
    const SkOpAngle* next = this;
    do {
        next->dumpOne(false);
        SkDebugf("\n");
        next = next->fNext;
    } while (next && next != first);
}

void SkOpAngle::dumpPartials() const {
    const SkOpAngle* first = this;
    const SkOpAngle* next = this;
    do {
        next->fCurvePart.dumpNumber();
        next = next->fNext;
    } while (next && next != first);
}

void SkOpAngleSet::dump() const {
    // FIXME: unimplemented
/* This requires access to the internal SkChunkAlloc data
   Defer implementing this until it is needed for debugging
*/
    SkASSERT(0);
}

void SkOpContour::dump() const {
    int segmentCount = fSegments.count();
    SkDebugf("((SkOpContour*) 0x%p) [%d]\n", this, debugID());
    for (int test = 0; test < segmentCount; ++test) {
        SkDebugf("  [%d] ((SkOpSegment*) 0x%p) [%d]\n", test, &fSegments[test],
                fSegments[test].debugID());
    }
}

void SkOpContour::dumpAngles() const {
    int segmentCount = fSegments.count();
    SkDebugf("((SkOpContour*) 0x%p) [%d]\n", this, debugID());
    for (int test = 0; test < segmentCount; ++test) {
        SkDebugf("  [%d] ", test);
        fSegments[test].dumpAngles();
    }
}

void SkOpContour::dumpCoincidence(const SkCoincidence& coin) const {
    int thisIndex = coin.fSegments[0];
    const SkOpSegment& s1 = fSegments[thisIndex];
    int otherIndex = coin.fSegments[1];
    const SkOpSegment& s2 = coin.fOther->fSegments[otherIndex];
    SkDebugf("((SkOpSegment*) 0x%p) [%d]  ((SkOpSegment*) 0x%p) [%d]\n", &s1, s1.debugID(),
            &s2, s2.debugID());
    for (int index = 0; index < 2; ++index) {
        SkDebugf("    {%1.9gf, %1.9gf}", coin.fPts[0][index].fX, coin.fPts[0][index].fY);
        if (coin.fNearly[index]) {
            SkDebugf("    {%1.9gf, %1.9gf}", coin.fPts[1][index].fX, coin.fPts[1][index].fY);
        }
        SkDebugf("  seg1t=%1.9g seg2t=%1.9g\n", coin.fTs[0][index], coin.fTs[1][index]);
    }
}

void SkOpContour::dumpCoincidences() const {
    int count = fCoincidences.count();
    if (count > 0) {
        SkDebugf("fCoincidences count=%d\n", count);
        for (int test = 0; test < count; ++test) {
            dumpCoincidence(fCoincidences[test]);
        }
    }
    count = fPartialCoincidences.count();
    if (count == 0) {
        return;
    }
    SkDebugf("fPartialCoincidences count=%d\n", count);
    for (int test = 0; test < count; ++test) {
        dumpCoincidence(fPartialCoincidences[test]);
    }
}

void SkOpContour::dumpPt(int index) const {
    int segmentCount = fSegments.count();
    for (int test = 0; test < segmentCount; ++test) {
        const SkOpSegment& segment = fSegments[test];
        if (segment.debugID() == index) {
            fSegments[test].dumpPts();
        }
    }
}

void SkOpContour::dumpPts() const {
    int segmentCount = fSegments.count();
    SkDebugf("((SkOpContour*) 0x%p) [%d]\n", this, debugID());
    for (int test = 0; test < segmentCount; ++test) {
        SkDebugf("  [%d] ", test);
        fSegments[test].dumpPts();
    }
}

void SkOpContour::dumpSpan(int index) const {
    int segmentCount = fSegments.count();
    for (int test = 0; test < segmentCount; ++test) {
        const SkOpSegment& segment = fSegments[test];
        if (segment.debugID() == index) {
            fSegments[test].dumpSpans();
        }
    }
}

void SkOpContour::dumpSpans() const {
    int segmentCount = fSegments.count();
    SkDebugf("((SkOpContour*) 0x%p) [%d]\n", this, debugID());
    for (int test = 0; test < segmentCount; ++test) {
        SkDebugf("  [%d] ", test);
        fSegments[test].dumpSpans();
    }
}

void SkDCubic::dump() const {
    SkDebugf("{{");
    int index = 0;
    do {
        fPts[index].dump();
        SkDebugf(", ");
    } while (++index < 3);
    fPts[index].dump();
    SkDebugf("}}\n");
}

void SkDCubic::dumpNumber() const {
    SkDebugf("{{");
    int index = 0;
    bool dumpedOne = false;
    do {
        if (!(fPts[index].fX == fPts[index].fX && fPts[index].fY == fPts[index].fY)) {
            continue;
        }
        if (dumpedOne) {
            SkDebugf(", ");
        }
        fPts[index].dump();
        dumpedOne = true;
    } while (++index < 3);
    if (fPts[index].fX == fPts[index].fX && fPts[index].fY == fPts[index].fY) {
        if (dumpedOne) {
            SkDebugf(", ");
        }
        fPts[index].dump();
    }
    SkDebugf("}}\n");
}

void SkDLine::dump() const {
    SkDebugf("{{");
    fPts[0].dump();
    SkDebugf(", ");
    fPts[1].dump();
    SkDebugf("}}\n");
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
    dumpComma("");
}

void SkDQuad::dumpComma(const char* comma) const {
    SkDebugf("{{");
    int index = 0;
    do {
        fPts[index].dump();
        SkDebugf(", ");
    } while (++index < 2);
    fPts[index].dump();
    SkDebugf("}}%s\n", comma ? comma : "");
}

void SkIntersectionHelper::dump() const {
    SkDPoint::Dump(pts()[0]);
    SkDPoint::Dump(pts()[1]);
    if (verb() >= SkPath::kQuad_Verb) {
        SkDPoint::Dump(pts()[2]);
    }
    if (verb() >= SkPath::kCubic_Verb) {
        SkDPoint::Dump(pts()[3]);
    }
}

const SkTDArray<SkOpSpan>& SkOpSegment::debugSpans() const {
    return fTs;
}

void SkOpSegment::dumpAngles() const {
    SkDebugf("((SkOpSegment*) 0x%p) [%d]\n", this, debugID());
    const SkOpAngle* fromAngle = NULL;
    const SkOpAngle* toAngle = NULL;
    for (int index = 0; index < count(); ++index) {
        const SkOpAngle* fAngle = fTs[index].fFromAngle;
        const SkOpAngle* tAngle = fTs[index].fToAngle;
        if (fromAngle == fAngle && toAngle == tAngle) {
            continue;
        }
        if (fAngle) {
            SkDebugf("  [%d] from=%d ", index, fAngle->debugID());
            fAngle->dumpTo(this, tAngle);
        }
        if (tAngle) {
            SkDebugf("  [%d] to=%d   ", index, tAngle->debugID());
            tAngle->dumpTo(this, fAngle);
        }
        fromAngle = fAngle;
        toAngle = tAngle;
    }
}

void SkOpSegment::dumpContour(int firstID, int lastID) const {
    if (debugID() < 0) {
        return;
    }
    const SkOpSegment* test = this - (debugID() - 1);
    test += (firstID - 1);
    const SkOpSegment* last = test + (lastID - firstID);
    while (test <= last) {
        test->dumpSpans();
        ++test;
    }
}

void SkOpSegment::dumpPts() const {
    int last = SkPathOpsVerbToPoints(fVerb);
    SkDebugf("((SkOpSegment*) 0x%p) [%d] {{", this, debugID());
    int index = 0;
    do {
        SkDPoint::Dump(fPts[index]);
        SkDebugf(", ");
    } while (++index < last);
    SkDPoint::Dump(fPts[index]);
    SkDebugf("}}\n");
}

void SkOpSegment::dumpHexPts() const {
    int last = SkPathOpsVerbToPoints(fVerb);
    SkDebugf("((SkOpSegment*) 0x%p) [%d] {{", this, debugID());
    int index = 0;
    do {
        SkDPoint::DumpHex(fPts[index]);
        SkDebugf(", ");
    } while (++index < last);
    SkDPoint::DumpHex(fPts[index]);
    SkDebugf("}}\n");
}

void SkOpSegment::dumpDPts() const {
    int count = SkPathOpsVerbToPoints(fVerb);
    SkDebugf("((SkOpSegment*) 0x%p) [%d] {{", this, debugID());
    int index = 0;
    do {
        SkDPoint dPt = {fPts[index].fX, fPts[index].fY};
        dPt.dump();
        if (index != count) {
            SkDebugf(", ");
        }
    } while (++index <= count);
    SkDebugf("}}\n");
}

void SkOpSegment::dumpSpans() const {
    int count = this->count();
    SkDebugf("((SkOpSegment*) 0x%p) [%d]\n", this, debugID());
    for (int index = 0; index < count; ++index) {
        const SkOpSpan& span = this->span(index);
        SkDebugf("  [%d] ", index);
        span.dumpOne();
    }
}

void SkPathOpsDebug::DumpCoincidence(const SkTArray<SkOpContour, true>& contours) {
    int count = contours.count();
    for (int index = 0; index < count; ++index) {
        contours[index].dumpCoincidences();
    }
}

void SkPathOpsDebug::DumpCoincidence(const SkTArray<SkOpContour* , true>& contours) {
    int count = contours.count();
    for (int index = 0; index < count; ++index) {
        contours[index]->dumpCoincidences();
    }
}

void SkPathOpsDebug::DumpContours(const SkTArray<SkOpContour, true>& contours) {
    int count = contours.count();
    for (int index = 0; index < count; ++index) {
        contours[index].dump();
    }
}

void SkPathOpsDebug::DumpContours(const SkTArray<SkOpContour* , true>& contours) {
    int count = contours.count();
    for (int index = 0; index < count; ++index) {
        contours[index]->dump();
    }
}

void SkPathOpsDebug::DumpContourAngles(const SkTArray<SkOpContour, true>& contours) {
    int count = contours.count();
    for (int index = 0; index < count; ++index) {
        contours[index].dumpAngles();
    }
}

void SkPathOpsDebug::DumpContourAngles(const SkTArray<SkOpContour* , true>& contours) {
    int count = contours.count();
    for (int index = 0; index < count; ++index) {
        contours[index]->dumpAngles();
    }
}

void SkPathOpsDebug::DumpContourPts(const SkTArray<SkOpContour, true>& contours) {
    int count = contours.count();
    for (int index = 0; index < count; ++index) {
        contours[index].dumpPts();
    }
}

void SkPathOpsDebug::DumpContourPts(const SkTArray<SkOpContour* , true>& contours) {
    int count = contours.count();
    for (int index = 0; index < count; ++index) {
        contours[index]->dumpPts();
    }
}

void SkPathOpsDebug::DumpContourPt(const SkTArray<SkOpContour, true>& contours, int segmentID) {
    int count = contours.count();
    for (int index = 0; index < count; ++index) {
        contours[index].dumpPt(segmentID);
    }
}

void SkPathOpsDebug::DumpContourPt(const SkTArray<SkOpContour* , true>& contours, int segmentID) {
    int count = contours.count();
    for (int index = 0; index < count; ++index) {
        contours[index]->dumpPt(segmentID);
    }
}

void SkPathOpsDebug::DumpContourSpans(const SkTArray<SkOpContour, true>& contours) {
    int count = contours.count();
    for (int index = 0; index < count; ++index) {
        contours[index].dumpSpans();
    }
}

void SkPathOpsDebug::DumpContourSpans(const SkTArray<SkOpContour* , true>& contours) {
    int count = contours.count();
    for (int index = 0; index < count; ++index) {
        contours[index]->dumpSpans();
    }
}

void SkPathOpsDebug::DumpContourSpan(const SkTArray<SkOpContour, true>& contours, int segmentID) {
    int count = contours.count();
    for (int index = 0; index < count; ++index) {
        contours[index].dumpSpan(segmentID);
    }
}

void SkPathOpsDebug::DumpContourSpan(const SkTArray<SkOpContour* , true>& contours, int segmentID) {
    int count = contours.count();
    for (int index = 0; index < count; ++index) {
        contours[index]->dumpSpan(segmentID);
    }
}

void SkPathOpsDebug::DumpSpans(const SkTDArray<SkOpSpan *>& spans) {
    int count = spans.count();
    for (int index = 0; index < count; ++index) {
        const SkOpSpan* span = spans[index];
        const SkOpSpan& oSpan = span->fOther->span(span->fOtherIndex);
        const SkOpSegment* segment = oSpan.fOther;
        SkDebugf("((SkOpSegment*) 0x%p) [%d] ", segment, segment->debugID());
        SkDebugf("spanIndex:%d ", oSpan.fOtherIndex);
        span->dumpOne();
    }
}

// this does not require that other T index is initialized or correct
const SkOpSegment* SkOpSpan::debugToSegment(ptrdiff_t* spanIndex) const {
    if (!fOther) {
        return NULL;
    }
    int oppCount = fOther->count();
    for (int index = 0; index < oppCount; ++index) {
        const SkOpSpan& otherSpan = fOther->span(index);
        double otherTestT = otherSpan.fT;
        if (otherTestT < fOtherT) {
            continue;
        }
        SkASSERT(otherTestT == fOtherT);
        const SkOpSegment* candidate = otherSpan.fOther;
        const SkOpSpan* first = candidate->debugSpans().begin();
        const SkOpSpan* last = candidate->debugSpans().end() - 1;
        if (first <= this && this <= last) {
            if (spanIndex) {
                *spanIndex = this - first;
            }
            return candidate;
        }
    }
    SkASSERT(0);
    return NULL;
}

void SkOpSpan::dumpOne() const {
    SkDebugf("t=");
    DebugDumpDouble(fT);
    SkDebugf(" pt=");
    SkDPoint::Dump(fPt);
    if (fOther) {
        SkDebugf(" other.fID=%d", fOther->debugID());
        SkDebugf(" [%d] otherT=", fOtherIndex);
        DebugDumpDouble(fOtherT);
    } else {
        SkDebugf(" other.fID=? [?] otherT=?");
    }
    if (fWindSum != SK_MinS32) {
        SkDebugf(" windSum=%d", fWindSum);
    }
    if (fOppSum != SK_MinS32 && (SkPathOpsDebug::ValidWind(fOppSum) || fOppValue != 0)) {
        SkDebugf(" oppSum=%d", fOppSum);
    }
    SkDebugf(" windValue=%d", fWindValue);
    if (SkPathOpsDebug::ValidWind(fOppSum) || fOppValue != 0) {
        SkDebugf(" oppValue=%d", fOppValue);
    }
    if (fFromAngle && fFromAngle->debugID()) {
        SkDebugf(" from=%d", fFromAngle->debugID());
    }
    if (fToAngle && fToAngle->debugID()) {
        SkDebugf(" to=%d", fToAngle->debugID());
    }
    if (fChased) {
        SkDebugf(" chased");
    }
    if (fCoincident) {
        SkDebugf(" coincident");
    }
    if (fDone) {
        SkDebugf(" done");
    }
    if (fLoop) {
        SkDebugf(" loop");
    }
    if (fMultiple) {
        SkDebugf(" multiple");
    }
    if (fNear) {
        SkDebugf(" near");
    }
    if (fSmall) {
        SkDebugf(" small");
    }
    if (fTiny) {
        SkDebugf(" tiny");
    }
    SkDebugf("\n");
}

void SkOpSpan::dump() const {
    ptrdiff_t spanIndex;
    const SkOpSegment* segment = debugToSegment(&spanIndex);
    if (segment) {
        SkDebugf("((SkOpSegment*) 0x%p) [%d]\n", segment, segment->debugID());
        SkDebugf("  [%d] ", spanIndex);
    } else {
        SkDebugf("((SkOpSegment*) ?) [?]\n");
        SkDebugf("  [?] ");
    }
    dumpOne();
}

void Dump(const SkTArray<class SkOpContour, true>& contours) {
    SkPathOpsDebug::DumpContours(contours);
}

void Dump(const SkTArray<class SkOpContour* , true>& contours) {
    SkPathOpsDebug::DumpContours(contours);
}

void Dump(const SkTArray<class SkOpContour, true>* contours) {
    SkPathOpsDebug::DumpContours(*contours);
}

void Dump(const SkTArray<class SkOpContour* , true>* contours) {
    SkPathOpsDebug::DumpContours(*contours);
}

void Dump(const SkTDArray<SkOpSpan *>& chase) {
    SkPathOpsDebug::DumpSpans(chase);
}

void Dump(const SkTDArray<SkOpSpan *>* chase) {
    SkPathOpsDebug::DumpSpans(*chase);
}

void DumpAngles(const SkTArray<class SkOpContour, true>& contours) {
    SkPathOpsDebug::DumpContourAngles(contours);
}

void DumpAngles(const SkTArray<class SkOpContour* , true>& contours) {
    SkPathOpsDebug::DumpContourAngles(contours);
}

void DumpAngles(const SkTArray<class SkOpContour, true>* contours) {
    SkPathOpsDebug::DumpContourAngles(*contours);
}

void DumpAngles(const SkTArray<class SkOpContour* , true>* contours) {
    SkPathOpsDebug::DumpContourAngles(*contours);
}

void DumpCoin(const SkTArray<class SkOpContour, true>& contours) {
    SkPathOpsDebug::DumpCoincidence(contours);
}

void DumpCoin(const SkTArray<class SkOpContour* , true>& contours) {
    SkPathOpsDebug::DumpCoincidence(contours);
}

void DumpCoin(const SkTArray<class SkOpContour, true>* contours) {
    SkPathOpsDebug::DumpCoincidence(*contours);
}

void DumpCoin(const SkTArray<class SkOpContour* , true>* contours) {
    SkPathOpsDebug::DumpCoincidence(*contours);
}

void DumpSpans(const SkTArray<class SkOpContour, true>& contours) {
    SkPathOpsDebug::DumpContourSpans(contours);
}

void DumpSpans(const SkTArray<class SkOpContour* , true>& contours) {
    SkPathOpsDebug::DumpContourSpans(contours);
}

void DumpSpans(const SkTArray<class SkOpContour, true>* contours) {
    SkPathOpsDebug::DumpContourSpans(*contours);
}

void DumpSpans(const SkTArray<class SkOpContour* , true>* contours) {
    SkPathOpsDebug::DumpContourSpans(*contours);
}

void DumpSpan(const SkTArray<class SkOpContour, true>& contours, int segmentID) {
    SkPathOpsDebug::DumpContourSpan(contours, segmentID);
}

void DumpSpan(const SkTArray<class SkOpContour* , true>& contours, int segmentID) {
    SkPathOpsDebug::DumpContourSpan(contours, segmentID);
}

void DumpSpan(const SkTArray<class SkOpContour, true>* contours, int segmentID) {
    SkPathOpsDebug::DumpContourSpan(*contours, segmentID);
}

void DumpSpan(const SkTArray<class SkOpContour* , true>* contours, int segmentID) {
    SkPathOpsDebug::DumpContourSpan(*contours, segmentID);
}

void DumpPts(const SkTArray<class SkOpContour, true>& contours) {
    SkPathOpsDebug::DumpContourPts(contours);
}

void DumpPts(const SkTArray<class SkOpContour* , true>& contours) {
    SkPathOpsDebug::DumpContourPts(contours);
}

void DumpPts(const SkTArray<class SkOpContour, true>* contours) {
    SkPathOpsDebug::DumpContourPts(*contours);
}

void DumpPts(const SkTArray<class SkOpContour* , true>* contours) {
    SkPathOpsDebug::DumpContourPts(*contours);
}

void DumpPt(const SkTArray<class SkOpContour, true>& contours, int segmentID) {
    SkPathOpsDebug::DumpContourPt(contours, segmentID);
}

void DumpPt(const SkTArray<class SkOpContour* , true>& contours, int segmentID) {
    SkPathOpsDebug::DumpContourPt(contours, segmentID);
}

void DumpPt(const SkTArray<class SkOpContour, true>* contours, int segmentID) {
    SkPathOpsDebug::DumpContourPt(*contours, segmentID);
}

void DumpPt(const SkTArray<class SkOpContour* , true>* contours, int segmentID) {
    SkPathOpsDebug::DumpContourPt(*contours, segmentID);
}

static void dumpTestCase(const SkDQuad& quad1, const SkDQuad& quad2, int testNo) {
    SkDebugf("<div id=\"quad%d\">\n", testNo);
    quad1.dumpComma(",");
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
