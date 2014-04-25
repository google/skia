#include "SkOpContour.h"
#include "SkIntersectionHelper.h"
#include "SkOpSegment.h"

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
#if DEBUG_SORT
    debugOne(false);
#endif
    SkDebugf("\n");
}

void SkOpAngle::dumpFromTo(const SkOpSegment* segment, int from, int to) const {
#if DEBUG_SORT && DEBUG_ANGLE
    const SkOpAngle* first = this;
    const SkOpAngle* next = this;
    const char* indent = "";
    do {
        SkDebugf("%s", indent);
        next->debugOne(false);
        if (segment == next->fSegment) {
            if (fNext && from == fNext->debugID()) {
                SkDebugf(" << from");
            }
            if (fNext && to == fNext->debugID()) {
                SkDebugf(" << to");
            }
        }
        SkDebugf("\n");
        indent = "           ";
        next = next->fNext;
    } while (next && next != first);
#endif
}

void SkOpAngle::dumpLoop() const {
    const SkOpAngle* first = this;
    const SkOpAngle* next = this;
    do {
        next->dump();
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

void SkOpContour::dumpPts() const {
    int segmentCount = fSegments.count();
    SkDebugf("((SkOpContour*) 0x%p) [%d]\n", this, debugID());
    for (int test = 0; test < segmentCount; ++test) {
        SkDebugf("  [%d] ", test);
        fSegments[test].dumpPts();
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

void SkDQuad::dump() const {
    dumpComma("");
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
    int fromIndex = -1, toIndex = -1;
    for (int index = 0; index < count(); ++index) {
        int fIndex = fTs[index].fFromAngleIndex;
        int tIndex = fTs[index].fToAngleIndex;
        if (fromIndex == fIndex && tIndex == toIndex) {
            continue;
        }
        if (fIndex >= 0) {
            SkDebugf("  [%d] from=%d ", index, fIndex);
            const SkOpAngle& angle = this->angle(fIndex);
            angle.dumpFromTo(this, fIndex, tIndex);
        }
        if (tIndex >= 0) {
            SkDebugf("  [%d] to=%d   ", index, tIndex);
            const SkOpAngle& angle = this->angle(tIndex);
            angle.dumpFromTo(this, fIndex, tIndex);
        }
        fromIndex = fIndex;
        toIndex = tIndex;
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

void SkPathOpsDebug::DumpAngles(const SkTArray<SkOpAngle, true>& angles) {
    int count = angles.count();
    for (int index = 0; index < count; ++index) {
        angles[index].dump();
    }
}

void SkPathOpsDebug::DumpAngles(const SkTArray<SkOpAngle* , true>& angles) {
    int count = angles.count();
    for (int index = 0; index < count; ++index) {
        angles[index]->dump();
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
#if DEBUG_WINDING
    SkDebugf(" windSum=");
    SkPathOpsDebug::WindingPrintf(fWindSum);
#endif
    if (SkPathOpsDebug::ValidWind(fOppSum) || fOppValue != 0) {
#if DEBUG_WINDING
        SkDebugf(" oppSum=");
        SkPathOpsDebug::WindingPrintf(fOppSum);
#endif
    }
    SkDebugf(" windValue=%d", fWindValue);
    if (SkPathOpsDebug::ValidWind(fOppSum) || fOppValue != 0) {
        SkDebugf(" oppValue=%d", fOppValue);
    }
    SkDebugf(" from=%d", fFromAngleIndex);
    SkDebugf(" to=%d", fToAngleIndex);
    if (fDone) {
        SkDebugf(" done");
    }
    if (fTiny) {
        SkDebugf(" tiny");
    }
    if (fSmall) {
        SkDebugf(" small");
    }
    if (fLoop) {
        SkDebugf(" loop");
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

void Dump(const SkTArray<class SkOpAngle, true>& angles) {
    SkPathOpsDebug::DumpAngles(angles);
}

void Dump(const SkTArray<class SkOpAngle* , true>& angles) {
    SkPathOpsDebug::DumpAngles(angles);
}

void Dump(const SkTArray<class SkOpAngle, true>* angles) {
    SkPathOpsDebug::DumpAngles(*angles);
}

void Dump(const SkTArray<class SkOpAngle* , true>* angles) {
    SkPathOpsDebug::DumpAngles(*angles);
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

void Dump(const SkTDArray<SkOpSpan *>& chaseArray) {
    SkPathOpsDebug::DumpSpans(chaseArray);
}

void Dump(const SkTDArray<SkOpSpan *>* chaseArray) {
    SkPathOpsDebug::DumpSpans(*chaseArray);
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
