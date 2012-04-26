/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "CurveIntersection.h"
#include "Intersections.h"
#include "LineIntersection.h"
#include "SkPath.h"
#include "SkRect.h"
#include "SkTArray.h"
#include "SkTDArray.h"
#include "ShapeOps.h"
#include "TSearch.h"

#undef SkASSERT
#define SkASSERT(cond) while (!(cond)) { sk_throw(); }

// FIXME: remove once debugging is complete
#if 0 // set to 1 for no debugging whatsoever

//const bool gxRunTestsInOneThread = false;

#define DEBUG_ADD_INTERSECTING_TS 0
#define DEBUG_BRIDGE 0
#define DEBUG_DUMP 0

#else

//const bool gRunTestsInOneThread = true;

#define DEBUG_ADD_INTERSECTING_TS 1
#define DEBUG_BRIDGE 1
#define DEBUG_DUMP 1

#endif

#if DEBUG_DUMP
static const char* kLVerbStr[] = {"", "line", "quad", "cubic"};
static const char* kUVerbStr[] = {"", "Line", "Quad", "Cubic"};
static int gContourID;
static int gSegmentID;
#endif

static int LineIntersect(const SkPoint a[2], const SkPoint b[2],
        Intersections& intersections) {
    const _Line aLine = {{a[0].fX, a[0].fY}, {a[1].fX, a[1].fY}};
    const _Line bLine = {{b[0].fX, b[0].fY}, {b[1].fX, b[1].fY}};
    return intersect(aLine, bLine, intersections.fT[0], intersections.fT[1]);
}

static int QuadLineIntersect(const SkPoint a[3], const SkPoint b[2],
        Intersections& intersections) {
    const Quadratic aQuad = {{a[0].fX, a[0].fY}, {a[1].fX, a[1].fY}, {a[2].fX, a[2].fY}};
    const _Line bLine = {{b[0].fX, b[0].fY}, {b[1].fX, b[1].fY}};
    intersect(aQuad, bLine, intersections);
    return intersections.fUsed;
}

static int CubicLineIntersect(const SkPoint a[2], const SkPoint b[3],
        Intersections& intersections) {
    const Cubic aCubic = {{a[0].fX, a[0].fY}, {a[1].fX, a[1].fY}, {a[2].fX, a[2].fY},
            {a[3].fX, a[3].fY}};
    const _Line bLine = {{b[0].fX, b[0].fY}, {b[1].fX, b[1].fY}};
    return intersect(aCubic, bLine, intersections.fT[0], intersections.fT[1]);
}

static int QuadIntersect(const SkPoint a[3], const SkPoint b[3],
        Intersections& intersections) {
    const Quadratic aQuad = {{a[0].fX, a[0].fY}, {a[1].fX, a[1].fY}, {a[2].fX, a[2].fY}};
    const Quadratic bQuad = {{b[0].fX, b[0].fY}, {b[1].fX, b[1].fY}, {b[2].fX, b[2].fY}};
    intersect(aQuad, bQuad, intersections);
    return intersections.fUsed;
}

static int CubicIntersect(const SkPoint a[4], const SkPoint b[4],
        Intersections& intersections) {
    const Cubic aCubic = {{a[0].fX, a[0].fY}, {a[1].fX, a[1].fY}, {a[2].fX, a[2].fY},
            {a[3].fX, a[3].fY}};
    const Cubic bCubic = {{b[0].fX, b[0].fY}, {b[1].fX, b[1].fY}, {b[2].fX, b[2].fY},
            {b[3].fX, b[3].fY}};
    intersect(aCubic, bCubic, intersections);
    return intersections.fUsed;
}

static int HLineIntersect(const SkPoint a[2], SkScalar left, SkScalar right,
        SkScalar y, bool flipped, Intersections& intersections) {
    const _Line aLine = {{a[0].fX, a[0].fY}, {a[1].fX, a[1].fY}};
    return horizontalIntersect(aLine, left, right, y, flipped, intersections);
}

static int VLineIntersect(const SkPoint a[2], SkScalar left, SkScalar right,
        SkScalar y, bool flipped, Intersections& intersections) {
    const _Line aLine = {{a[0].fX, a[0].fY}, {a[1].fX, a[1].fY}};
    return verticalIntersect(aLine, left, right, y, flipped, intersections);
}

static int HQuadIntersect(const SkPoint a[3], SkScalar left, SkScalar right,
        SkScalar y, bool flipped, Intersections& intersections) {
    const Quadratic aQuad = {{a[0].fX, a[0].fY}, {a[1].fX, a[1].fY}, {a[2].fX, a[2].fY}};
    return horizontalIntersect(aQuad, left, right, y, flipped, intersections);
}

static int VQuadIntersect(const SkPoint a[3], SkScalar left, SkScalar right,
        SkScalar y, bool flipped, Intersections& intersections) {
    const Quadratic aQuad = {{a[0].fX, a[0].fY}, {a[1].fX, a[1].fY}, {a[2].fX, a[2].fY}};
    return verticalIntersect(aQuad, left, right, y, flipped, intersections);
}

static int HCubicIntersect(const SkPoint a[4], SkScalar left, SkScalar right,
        SkScalar y, bool flipped, Intersections& intersections) {
    const Cubic aCubic = {{a[0].fX, a[0].fY}, {a[1].fX, a[1].fY}, {a[2].fX, a[2].fY},
            {a[3].fX, a[3].fY}};
    return horizontalIntersect(aCubic, left, right, y, flipped, intersections);
}

static int VCubicIntersect(const SkPoint a[4], SkScalar left, SkScalar right,
        SkScalar y, bool flipped, Intersections& intersections) {
    const Cubic aCubic = {{a[0].fX, a[0].fY}, {a[1].fX, a[1].fY}, {a[2].fX, a[2].fY},
            {a[3].fX, a[3].fY}};
    return verticalIntersect(aCubic, left, right, y, flipped, intersections);
}

static void LineXYAtT(const SkPoint a[2], double t, SkPoint* out) {
    const _Line line = {{a[0].fX, a[0].fY}, {a[1].fX, a[1].fY}};
    double x, y;
    xy_at_t(line, t, x, y);
    out->fX = SkDoubleToScalar(x);
    out->fY = SkDoubleToScalar(y);
}

static void QuadXYAtT(const SkPoint a[3], double t, SkPoint* out) {
    const Quadratic quad = {{a[0].fX, a[0].fY}, {a[1].fX, a[1].fY}, {a[2].fX, a[2].fY}};
    double x, y;
    xy_at_t(quad, t, x, y);
    out->fX = SkDoubleToScalar(x);
    out->fY = SkDoubleToScalar(y);
}

static void CubicXYAtT(const SkPoint a[4], double t, SkPoint* out) {
    const Cubic cubic = {{a[0].fX, a[0].fY}, {a[1].fX, a[1].fY}, {a[2].fX, a[2].fY},
            {a[3].fX, a[3].fY}};
    double x, y;
    xy_at_t(cubic, t, x, y);
    out->fX = SkDoubleToScalar(x);
    out->fY = SkDoubleToScalar(y);
}

static void (* const SegmentXYAtT[])(const SkPoint [], double , SkPoint* ) = {
    NULL,
    LineXYAtT,
    QuadXYAtT,
    CubicXYAtT
};

static SkScalar LineXAtT(const SkPoint a[2], double t) {
    const _Line aLine = {{a[0].fX, a[0].fY}, {a[1].fX, a[1].fY}};
    double x;
    xy_at_t(aLine, t, x, *(double*) 0);
    return SkDoubleToScalar(x);
}

static SkScalar QuadXAtT(const SkPoint a[3], double t) {
    const Quadratic quad = {{a[0].fX, a[0].fY}, {a[1].fX, a[1].fY}, {a[2].fX, a[2].fY}};
    double x;
    xy_at_t(quad, t, x, *(double*) 0);
    return SkDoubleToScalar(x);
}

static SkScalar CubicXAtT(const SkPoint a[4], double t) {
    const Cubic cubic = {{a[0].fX, a[0].fY}, {a[1].fX, a[1].fY}, {a[2].fX, a[2].fY},
            {a[3].fX, a[3].fY}};
    double x;
    xy_at_t(cubic, t, x, *(double*) 0);
    return SkDoubleToScalar(x);
}

static SkScalar (* const SegmentXAtT[])(const SkPoint [], double ) = {
    NULL,
    LineXAtT,
    QuadXAtT,
    CubicXAtT
};

static SkScalar LineYAtT(const SkPoint a[2], double t) {
    const _Line aLine = {{a[0].fX, a[0].fY}, {a[1].fX, a[1].fY}};
    double y;
    xy_at_t(aLine, t, *(double*) 0, y);
    return SkDoubleToScalar(y);
}

static SkScalar QuadYAtT(const SkPoint a[3], double t) {
    const Quadratic quad = {{a[0].fX, a[0].fY}, {a[1].fX, a[1].fY}, {a[2].fX, a[2].fY}};
    double y;
    xy_at_t(quad, t, *(double*) 0, y);
    return SkDoubleToScalar(y);
}

static SkScalar CubicYAtT(const SkPoint a[4], double t) {
    const Cubic cubic = {{a[0].fX, a[0].fY}, {a[1].fX, a[1].fY}, {a[2].fX, a[2].fY},
            {a[3].fX, a[3].fY}};
    double y;
    xy_at_t(cubic, t, *(double*) 0, y);
    return SkDoubleToScalar(y);
}

static SkScalar (* const SegmentYAtT[])(const SkPoint [], double ) = {
    NULL,
    LineYAtT,
    QuadYAtT,
    CubicYAtT
};

static void LineSubDivide(const SkPoint a[2], double startT, double endT,
        SkPoint sub[2]) {
    const _Line aLine = {{a[0].fX, a[0].fY}, {a[1].fX, a[1].fY}};
    _Line dst;
    sub_divide(aLine, startT, endT, dst);
    sub[0].fX = SkDoubleToScalar(dst[0].x);
    sub[0].fY = SkDoubleToScalar(dst[0].y);
    sub[1].fX = SkDoubleToScalar(dst[1].x);
    sub[1].fY = SkDoubleToScalar(dst[1].y);
}

static void QuadSubDivide(const SkPoint a[3], double startT, double endT,
        SkPoint sub[3]) {
    const Quadratic aQuad = {{a[0].fX, a[0].fY}, {a[1].fX, a[1].fY},
            {a[2].fX, a[2].fY}};
    Quadratic dst;
    sub_divide(aQuad, startT, endT, dst);
    sub[0].fX = SkDoubleToScalar(dst[0].x);
    sub[0].fY = SkDoubleToScalar(dst[0].y);
    sub[1].fX = SkDoubleToScalar(dst[1].x);
    sub[1].fY = SkDoubleToScalar(dst[1].y);
    sub[2].fX = SkDoubleToScalar(dst[2].x);
    sub[2].fY = SkDoubleToScalar(dst[2].y);
}

static void CubicSubDivide(const SkPoint a[4], double startT, double endT,
        SkPoint sub[4]) {
    const Cubic aCubic = {{a[0].fX, a[0].fY}, {a[1].fX, a[1].fY},
            {a[2].fX, a[2].fY}, {a[3].fX, a[3].fY}};
    Cubic dst;
    sub_divide(aCubic, startT, endT, dst);
    sub[0].fX = SkDoubleToScalar(dst[0].x);
    sub[0].fY = SkDoubleToScalar(dst[0].y);
    sub[1].fX = SkDoubleToScalar(dst[1].x);
    sub[1].fY = SkDoubleToScalar(dst[1].y);
    sub[2].fX = SkDoubleToScalar(dst[2].x);
    sub[2].fY = SkDoubleToScalar(dst[2].y);
    sub[3].fX = SkDoubleToScalar(dst[3].x);
    sub[3].fY = SkDoubleToScalar(dst[3].y);
}

static void QuadSubBounds(const SkPoint a[3], double startT, double endT,
        SkRect& bounds) {
    SkPoint dst[3];
    QuadSubDivide(a, startT, endT, dst);
    bounds.fLeft = bounds.fRight = dst[0].fX;
    bounds.fTop = bounds.fBottom = dst[0].fY;
    for (int index = 1; index < 3; ++index) {
        bounds.growToInclude(dst[index].fX, dst[index].fY);
    }
}

static void CubicSubBounds(const SkPoint a[4], double startT, double endT,
        SkRect& bounds) {
    SkPoint dst[4];
    CubicSubDivide(a, startT, endT, dst);
    bounds.fLeft = bounds.fRight = dst[0].fX;
    bounds.fTop = bounds.fBottom = dst[0].fY;
    for (int index = 1; index < 4; ++index) {
        bounds.growToInclude(dst[index].fX, dst[index].fY);
    }
}

static SkPath::Verb QuadReduceOrder(const SkPoint a[4],
        SkTDArray<SkPoint>& reducePts) {
    const Quadratic aQuad = {{a[0].fX, a[0].fY}, {a[1].fX, a[1].fY},
            {a[2].fX, a[2].fY}};
    Quadratic dst;
    int order = reduceOrder(aQuad, dst);
    for (int index = 0; index < order; ++index) {
        SkPoint* pt = reducePts.append();
        pt->fX = SkDoubleToScalar(dst[index].x);
        pt->fY = SkDoubleToScalar(dst[index].y);
    }
    return (SkPath::Verb) (order - 1);
}

static SkPath::Verb CubicReduceOrder(const SkPoint a[4],
        SkTDArray<SkPoint>& reducePts) {
    const Cubic aCubic = {{a[0].fX, a[0].fY}, {a[1].fX, a[1].fY},
            {a[2].fX, a[2].fY}, {a[3].fX, a[3].fY}};
    Cubic dst;
    int order = reduceOrder(aCubic, dst, kReduceOrder_QuadraticsAllowed);
    for (int index = 0; index < order; ++index) {
        SkPoint* pt = reducePts.append();
        pt->fX = SkDoubleToScalar(dst[index].x);
        pt->fY = SkDoubleToScalar(dst[index].y);
    }
    return (SkPath::Verb) (order - 1);
}

static SkScalar LineLeftMost(const SkPoint a[2], double startT, double endT) {
    const _Line aLine = {{a[0].fX, a[0].fY}, {a[1].fX, a[1].fY}};
    double x[2];
    xy_at_t(aLine, startT, x[0], *(double*) 0);
    xy_at_t(aLine, endT, x[0], *(double*) 0);
    return startT < endT ? startT : endT;
}

static SkScalar QuadLeftMost(const SkPoint a[3], double startT, double endT) {
    const Quadratic aQuad = {{a[0].fX, a[0].fY}, {a[1].fX, a[1].fY},
            {a[2].fX, a[2].fY}};
    return leftMostT(aQuad, startT, endT);
}

static SkScalar CubicLeftMost(const SkPoint a[4], double startT, double endT) {
    const Cubic aCubic = {{a[0].fX, a[0].fY}, {a[1].fX, a[1].fY},
            {a[2].fX, a[2].fY}, {a[3].fX, a[3].fY}};
    return leftMostT(aCubic, startT, endT);
}

static SkScalar (* const SegmentLeftMost[])(const SkPoint [], double , double) = {
    NULL,
    LineLeftMost,
    QuadLeftMost,
    CubicLeftMost
};

static bool IsCoincident(const SkPoint a[2], const SkPoint& above,
        const SkPoint& below) {
    const _Line aLine = {{a[0].fX, a[0].fY}, {a[1].fX, a[1].fY}};
    const _Line bLine = {{above.fX, above.fY}, {below.fX, below.fY}};
    return implicit_matches_ulps(aLine, bLine, 32);
}

// Bounds, unlike Rect, does not consider a vertical line to be empty.
struct Bounds : public SkRect {
    static bool Intersects(const Bounds& a, const Bounds& b) {
        return a.fLeft <= b.fRight && b.fLeft <= a.fRight &&
                a.fTop <= b.fBottom && b.fTop <= a.fBottom;
    }

    bool isEmpty() {
        return fLeft > fRight || fTop > fBottom
                || fLeft == fRight && fTop == fBottom
                || isnan(fLeft) || isnan(fRight)
                || isnan(fTop) || isnan(fBottom);
    }

    void setCubicBounds(const SkPoint a[4]) {
        _Rect dRect;
        Cubic cubic  = {{a[0].fX, a[0].fY}, {a[1].fX, a[1].fY},
            {a[2].fX, a[2].fY}, {a[3].fX, a[3].fY}};
        dRect.setBounds(cubic);
        set(dRect.left, dRect.top, dRect.right, dRect.bottom);
    }

    void setQuadBounds(const SkPoint a[3]) {
        const Quadratic quad = {{a[0].fX, a[0].fY}, {a[1].fX, a[1].fY},
                {a[2].fX, a[2].fY}};
        _Rect dRect;
        dRect.setBounds(quad);
        set(dRect.left, dRect.top, dRect.right, dRect.bottom);
    }
};

class Segment;

struct TEntry {
    double fT;
    const Segment* fOther;
    double fOtherT;
    bool fCoincident;
};

class Segment {
public:
    Segment() {
#if DEBUG_DUMP
        fID = ++gSegmentID;
#endif
    }
    
    int addT(double newT, const Segment& other) {
        // FIXME: in the pathological case where there is a ton of intercepts,
        //  binary search?
        int insertedAt = -1;
        TEntry* entry;
        size_t tCount = fTs.count();
        double delta;
        for (size_t idx2 = 0; idx2 < tCount; ++idx2) {
            if (newT <= fTs[idx2].fT) {
                insertedAt = idx2;
                entry = fTs.insert(idx2);
                goto finish;
            }
        }
        insertedAt = tCount;
        entry = fTs.append();
finish:
        entry->fT = newT;
        entry->fOther = &other;
        return insertedAt;
    }
    
    bool addCubic(const SkPoint pts[4]) {
        fPts = pts;
        fVerb = SkPath::kCubic_Verb;
        fBounds.setCubicBounds(pts);
    }

    bool addLine(const SkPoint pts[2]) {
        fPts = pts;
        fVerb = SkPath::kLine_Verb;
        fBounds.set(pts, 2);
    }
    
    // add 2 to edge or out of range values to get T extremes
    void addOtherT(int index, double other, bool coincident) {
        fTs[index].fOtherT = other;
        fTs[index].fCoincident = coincident;
    }
    
    bool addQuad(const SkPoint pts[3]) {
        fPts = pts;
        fVerb = SkPath::kQuad_Verb;
        fBounds.setQuadBounds(pts);
    }
    
    const Bounds& bounds() const {
        return fBounds;
    }
    
    int findByT(double t, const Segment* match) const {
        // OPTIMIZATION: bsearch if count is honkin huge
        int count = fTs.count();
        for (int index = 0; index < count; ++index) {
            const TEntry& entry = fTs[index];
            if (t == entry.fT && match == entry.fOther) {
                return index;
            }
        }
        SkASSERT(0); // should never get here
        return -1;
    }
    
    int findLefty(int tIndex, const SkPoint& base) const {
        int bestTIndex;
        SkPoint test;
        SkScalar bestX = DBL_MAX;
        int testTIndex = tIndex;
        while (--testTIndex >= 0) {
            xyAtT(testTIndex, &test);
            if (test != base) {
                continue;
            }
            bestX = test.fX;
            bestTIndex = testTIndex;
            break;
        }
        int count = fTs.count();
        testTIndex = tIndex;
        while (++testTIndex < count) {
            xyAtT(testTIndex, &test);
            if (test == base) {
                continue;
            }
            return bestX > test.fX ? testTIndex : bestTIndex;
        }
        return -1;
    }

    const Segment* findTop(int& tIndex) const {
        // iterate through T intersections and return topmost
        // topmost tangent from y-min to first pt is closer to horizontal
        int firstT = 0;
        int lastT = 0;
        SkScalar topY = fPts[0].fY;
        int count = fTs.count();
        int index;
        for (index = 1; index < count; ++index) {
            const TEntry& entry = fTs[index];
            double t = entry.fT;
            SkScalar yIntercept = (*SegmentYAtT[fVerb])(fPts, t);
            if (topY > yIntercept) {
                topY = yIntercept;
                firstT = lastT = index;
            } else if (topY == yIntercept) {
                lastT = index;
            }
        }
        // if a pair of segments go down, choose the higher endpoint
        if (firstT == lastT && (firstT == 0 || firstT == count - 1)) {
            tIndex = firstT;
            return this;
        }
        // if the topmost T is not on end, or is three-way or more, find left
        SkPoint leftBase;
        xyAtT(firstT, &leftBase);
        int tLeft = findLefty(firstT, leftBase);
        SkASSERT(tLeft > 0);
        const Segment* leftSegment = this;
        SkScalar left = leftMost(firstT, tLeft);
        for (index = firstT; index <= lastT; ++index) {
            const Segment* other = fTs[index].fOther;
            double otherT = fTs[index].fOtherT;
            int otherTIndex = other->findByT(otherT, this);
            // pick companionT closest (but not too close) on either side
            int otherTLeft = other->findLefty(otherTIndex, leftBase);
            if (otherTLeft < 0) {
                continue;
            }
            SkScalar otherMost = other->leftMost(otherTIndex, otherTLeft);
            if (otherMost < left) {
                leftSegment = other;
            }
        }
        return leftSegment;
    }

    bool intersected() const {
        return fTs.count() > 0;
    }

    bool isHorizontal() const {
        return fBounds.fTop == fBounds.fBottom;
    }
    
    bool isVertical() const {
        return fBounds.fLeft == fBounds.fRight;
    }
    
    SkScalar leftMost(int start, int end) const {
        return (*SegmentLeftMost[fVerb])(fPts, fTs[start].fT, fTs[end].fT);
    }

    const SkPoint* pts() const {
        return fPts;
    }
    
    void reset() {
        fPts = NULL;
        fVerb = (SkPath::Verb) -1;
        fBounds.set(SK_ScalarMax, SK_ScalarMax, SK_ScalarMax, SK_ScalarMax);
        fTs.reset();
    }

    // OPTIMIZATION: remove this function if it's never called
    double t(int tIndex) const {
        return fTs[tIndex].fT;
    }
    
    SkPath::Verb verb() const {
        return fVerb;
    }
    
    SkScalar xAtT(double t) const {
        return (*SegmentXAtT[fVerb])(fPts, t);
    }

    void xyAtT(double t, SkPoint* pt) const {
        (*SegmentXYAtT[fVerb])(fPts, t, pt);
    }

#if DEBUG_DUMP
    void dump() const {
        const char className[] = "Segment";
        const int tab = 4;
        for (int i = 0; i < fTs.count(); ++i) {
            SkPoint out;
            (*SegmentXYAtT[fVerb])(fPts, t(i), &out);
            SkDebugf("%*s [%d] %s.fTs[%d]=%1.9g (%1.9g,%1.9g) other=%d"
                    " otherT=%1.9g coincident=%d\n",
                    tab + sizeof(className), className, fID,
                    kLVerbStr[fVerb], i, fTs[i].fT, out.fX, out.fY,
                    fTs[i].fOther->fID, fTs[i].fOtherT, fTs[i].fCoincident);
        }
        SkDebugf("%*s [%d] fBounds=(l:%1.9g, t:%1.9g r:%1.9g, b:%1.9g)\n",
                tab + sizeof(className), className, fID,
                fBounds.fLeft, fBounds.fTop, fBounds.fRight, fBounds.fBottom);
    }
#endif

private:
    const SkPoint* fPts;
    SkPath::Verb fVerb;
    Bounds fBounds;
    SkTDArray<TEntry> fTs;
#if DEBUG_DUMP
    int fID;
#endif
};

class Contour {
public:
    Contour() {
        reset();
#if DEBUG_DUMP
        fID = ++gContourID;
#endif
    }

    bool operator<(const Contour& rh) const {
        return fBounds.fTop == rh.fBounds.fTop
                ? fBounds.fLeft < rh.fBounds.fLeft
                : fBounds.fTop < rh.fBounds.fTop;
    }

    void addCubic(const SkPoint pts[4]) {
        fSegments.push_back().addCubic(pts);
        fContainsCurves = true;
    }
    void addLine(const SkPoint pts[2]) {
        fSegments.push_back().addLine(pts);
    }
    
    void addQuad(const SkPoint pts[3]) {
        fSegments.push_back().addQuad(pts);
        fContainsCurves = true;
    }
    
    const Bounds& bounds() const { 
        return fBounds;
    }

    void complete() {
        setBounds();
        fContainsIntercepts = false;
    }
    
    void containsIntercepts() {
        fContainsIntercepts = true;
    }
    
    void reset() {
        fSegments.reset();
        fBounds.set(SK_ScalarMax, SK_ScalarMax, SK_ScalarMax, SK_ScalarMax);
        fContainsCurves = fContainsIntercepts = false;
    }
    
    Segment& topSegment() {
        return fSegments[fTopSegment];
    }

#if DEBUG_DUMP
    void dump() {
        int i;
        const char className[] = "Contour";
        const int tab = 4;
        SkDebugf("%s %p (contour=%d)\n", className, this, fID);
        for (i = 0; i < fSegments.count(); ++i) {
            SkDebugf("%*s.fSegments[%d]:\n", tab + sizeof(className),
                    className, i);
            fSegments[i].dump();
        }
        SkDebugf("%*s.fBounds=(l:%1.9g, t:%1.9g r:%1.9g, b:%1.9g)\n",
                tab + sizeof(className), className,
                fBounds.fLeft, fBounds.fTop,
                fBounds.fRight, fBounds.fBottom);
        SkDebugf("%*s.fTopSegment=%d\n", tab + sizeof(className), className,
                fTopSegment);
        SkDebugf("%*s.fContainsIntercepts=%d\n", tab + sizeof(className),
                className, fContainsIntercepts);
        SkDebugf("%*s.fContainsCurves=%d\n", tab + sizeof(className),
                className, fContainsCurves);
    }
#endif

protected:
    void setBounds() {
        int count = fSegments.count();
        if (count == 0) {
            SkDebugf("%s empty contour\n", __FUNCTION__);
            SkASSERT(0);
            // FIXME: delete empty contour?
            return;
        }
        fTopSegment = 0;
        fBounds = fSegments.front().bounds();
        SkScalar top = fBounds.fTop;
        for (int index = 1; index < count; ++index) {
            fBounds.growToInclude(fSegments[index].bounds());
            if (top > fBounds.fTop) {
                fTopSegment = index;
                top = fBounds.fTop;
            }
        }
    }
    
public:
    SkTArray<Segment> fSegments; // not worth accessor functions?
    
private:
    Bounds fBounds;
    int fTopSegment;
    bool fContainsIntercepts;
    bool fContainsCurves;
#if DEBUG_DUMP
    int fID;
#endif
};

class EdgeBuilder {
public:

EdgeBuilder(const SkPath& path, SkTArray<Contour>& contours) 
    : fPath(path)
    , fCurrentContour(NULL)
    , fContours(contours)
{
#if DEBUG_DUMP
    gContourID = 0;
    gSegmentID = 0;
#endif
    walk();
}

protected:

void complete() {
    if (fCurrentContour && fCurrentContour->fSegments.count()) {
        fCurrentContour->complete();
        fCurrentContour = NULL;
    }
}

void startContour() {
    if (!fCurrentContour) {
        fCurrentContour = fContours.push_back_n(1);
    }
}

void walk() {
    // FIXME:remove once we can access path pts directly
    SkPath::RawIter iter(fPath); // FIXME: access path directly when allowed
    SkPoint pts[4];
    SkPath::Verb verb;
    do {
        verb = iter.next(pts);
        *fPathVerbs.append() = verb;
        if (verb == SkPath::kMove_Verb) {
            *fPathPts.append() = pts[0];
        } else if (verb >= SkPath::kLine_Verb && verb <= SkPath::kCubic_Verb) {
            fPathPts.append(verb, &pts[1]);
        }
    } while (verb != SkPath::kDone_Verb);
    // FIXME: end of section to remove once path pts are accessed directly
    
    SkPath::Verb reducedVerb;
    uint8_t* verbPtr = fPathVerbs.begin();
    const SkPoint* pointsPtr = fPathPts.begin();
    while ((verb = (SkPath::Verb) *verbPtr++) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kMove_Verb:
                complete();
                startContour();
                pointsPtr += 1;
                continue;
            case SkPath::kLine_Verb:
                // skip degenerate points
                if (pointsPtr[-1].fX != pointsPtr[0].fX
                        || pointsPtr[-1].fY != pointsPtr[0].fY) {
                    fCurrentContour->addLine(&pointsPtr[-1]);
                }
                break;
            case SkPath::kQuad_Verb:
                reducedVerb = QuadReduceOrder(&pointsPtr[-1], fReducePts);
                if (reducedVerb == 0) {
                    break; // skip degenerate points
                }
                if (reducedVerb == 1) {
                    fCurrentContour->addLine(fReducePts.end() - 2);
                    break;
                }
                fCurrentContour->addQuad(&pointsPtr[-1]);
                break;
            case SkPath::kCubic_Verb:
                reducedVerb = CubicReduceOrder(&pointsPtr[-1], fReducePts);
                if (reducedVerb == 0) {
                    break; // skip degenerate points
                }
                if (reducedVerb == 1) {
                    fCurrentContour->addLine(fReducePts.end() - 2);
                    break;
                }
                if (reducedVerb == 2) {
                    fCurrentContour->addQuad(fReducePts.end() - 3);
                    break;
                }
                fCurrentContour->addCubic(&pointsPtr[-1]);
                break;
            case SkPath::kClose_Verb:
                SkASSERT(fCurrentContour);
                complete();
                continue;
            default:
                SkDEBUGFAIL("bad verb");
                return;
        }
        pointsPtr += verb;
        SkASSERT(fCurrentContour);
    }
    complete();
    if (fCurrentContour && !fCurrentContour->fSegments.count()) {
        fContours.pop_back();
    }
}

private:
    const SkPath& fPath;
    SkTDArray<SkPoint> fPathPts; // FIXME: point directly to path pts instead
    SkTDArray<uint8_t> fPathVerbs; // FIXME: remove 
    Contour* fCurrentContour;
    SkTArray<Contour>& fContours;
    SkTDArray<SkPoint> fReducePts; // segments created on the fly
};

class Work {
public:
    enum SegmentType {
        kHorizontalLine_Segment = -1,
        kVerticalLine_Segment = 0,
        kLine_Segment = SkPath::kLine_Verb,
        kQuad_Segment = SkPath::kQuad_Verb,
        kCubic_Segment = SkPath::kCubic_Verb,
    };
    
    void addOtherT(int index, double other, bool coincident) {
        fContour->fSegments[fIndex].addOtherT(index, other, coincident);
    }
    
    // Avoid collapsing t values that are close to the same since
    // we walk ts to describe consecutive intersections. Since a pair of ts can
    // be nearly equal, any problems caused by this should be taken care
    // of later.
    // On the edge or out of range values are negative; add 2 to get end
    int addT(double newT, const Work& other) {
        fContour->containsIntercepts();
        return fContour->fSegments[fIndex].addT(newT,
                other.fContour->fSegments[other.fIndex]);
    }
    
    bool advance() {
        return ++fIndex < fLast;
    }
    
    SkScalar bottom() const {
        return bounds().fBottom;
    }
    
    const Bounds& bounds() const {
        return fContour->fSegments[fIndex].bounds();
    }
    
    const SkPoint* cubic() const {
        return fCubic;
    }

    void init(Contour* contour) {
        fContour = contour;
        fIndex = 0;
        fLast = contour->fSegments.count();
    }

    SkScalar left() const {
        return bounds().fLeft;
    }
    
    void promoteToCubic() {
        fCubic[0] = pts()[0];
        fCubic[2] = pts()[1];
        fCubic[3] = pts()[2];
        fCubic[1].fX = (fCubic[0].fX + fCubic[2].fX * 2) / 3;
        fCubic[1].fY = (fCubic[0].fY + fCubic[2].fY * 2) / 3;
        fCubic[2].fX = (fCubic[3].fX + fCubic[2].fX * 2) / 3;
        fCubic[2].fY = (fCubic[3].fY + fCubic[2].fY * 2) / 3;
    }
    
    const SkPoint* pts() const {
        return fContour->fSegments[fIndex].pts();
    }

    SkScalar right() const {
        return bounds().fRight;
    }
    
    ptrdiff_t segmentIndex() const {
        return fIndex;
    }
    
    SegmentType segmentType() const {
        const Segment& segment = fContour->fSegments[fIndex];
        SegmentType type = (SegmentType) segment.verb();
        if (type != kLine_Segment) {
            return type;
        }
        if (segment.isHorizontal()) {
            return kHorizontalLine_Segment;
        }
        if (segment.isVertical()) {
            return kVerticalLine_Segment;
        }
        return kLine_Segment;
    }
    
    bool startAfter(const Work& after) {
        fIndex = after.fIndex;
        return advance();
    }

    SkScalar top() const {
        return bounds().fTop;
    }
    
    SkPath::Verb verb() const {
        return fContour->fSegments[fIndex].verb();
    }
    
    SkScalar x() const {
        return bounds().fLeft;
    }

    bool xFlipped() const {
        return x() != pts()[0].fX;
    }

    SkScalar y() const {
        return bounds().fTop;
    }

    bool yFlipped() const {
        return y() != pts()[0].fX;
    }

protected:
    Contour* fContour;
    SkPoint fCubic[4];
    int fIndex;
    int fLast;
};

static void debugShowLineIntersection(int pts, const Work& wt,
        const Work& wn, const double wtTs[2], const double wnTs[2]) {
#if DEBUG_ADD_INTERSECTING_TS
    if (!pts) {
        return;
    }
    SkPoint wtOutPt, wnOutPt;
    LineXYAtT(wt.pts(), wtTs[0], &wtOutPt);
    LineXYAtT(wn.pts(), wnTs[0], &wnOutPt);
    SkDebugf("%s wtTs[0]=%g (%g,%g, %g,%g) (%g,%g)\n",
            __FUNCTION__,
            wtTs[0], wt.pts()[0].fX, wt.pts()[0].fY,
            wt.pts()[1].fX, wt.pts()[1].fY, wtOutPt.fX, wtOutPt.fY);
    if (pts == 2) {
        SkDebugf("%s wtTs[1]=%g\n", __FUNCTION__, wtTs[1]);
    }
    SkDebugf("%s wnTs[0]=%g (%g,%g, %g,%g) (%g,%g)\n",
            __FUNCTION__,
            wnTs[0], wn.pts()[0].fX, wn.pts()[0].fY,
            wn.pts()[1].fX, wn.pts()[1].fY, wnOutPt.fX, wnOutPt.fY);
    if (pts == 2) {
        SkDebugf("%s wnTs[1]=%g\n", __FUNCTION__, wnTs[1]);
    }
#endif
}

static bool addIntersectingTs(Contour* test, Contour* next) {
    if (test != next) {
        if (test->bounds().fBottom < next->bounds().fTop) {
            return false;
        }
        if (!Bounds::Intersects(test->bounds(), next->bounds())) {
            return true;
        }
    }
    Work wt, wn;
    wt.init(test);
    wn.init(next);
    do {
        if (test == next && !wn.startAfter(wt)) {
            continue;
        }
        do {
            if (!Bounds::Intersects(wt.bounds(), wn.bounds())) {
                continue;
            }
            int pts;
            Intersections ts;
            bool swap = false;
            switch (wt.segmentType()) {
                case Work::kHorizontalLine_Segment:
                    swap = true;
                    switch (wn.segmentType()) {
                        case Work::kHorizontalLine_Segment:
                        case Work::kVerticalLine_Segment:
                        case Work::kLine_Segment: {
                            pts = HLineIntersect(wn.pts(), wt.left(),
                                    wt.right(), wt.y(), wt.xFlipped(), ts);
                            break;
                        }
                        case Work::kQuad_Segment: {
                            pts = HQuadIntersect(wn.pts(), wt.left(),
                                    wt.right(), wt.y(), wt.xFlipped(), ts);
                            break;
                        }
                        case Work::kCubic_Segment: {
                            pts = HCubicIntersect(wn.pts(), wt.left(),
                                    wt.right(), wt.y(), wt.xFlipped(), ts);
                            break;
                        }
                        default:
                            SkASSERT(0);
                    }
                    break;
                case Work::kVerticalLine_Segment:
                    swap = true;
                    switch (wn.segmentType()) {
                        case Work::kHorizontalLine_Segment:
                        case Work::kVerticalLine_Segment:
                        case Work::kLine_Segment: {
                            pts = VLineIntersect(wn.pts(), wt.top(),
                                    wt.bottom(), wt.x(), wt.yFlipped(), ts);
                            break;
                        }
                        case Work::kQuad_Segment: {
                            pts = VQuadIntersect(wn.pts(), wt.top(),
                                    wt.bottom(), wt.x(), wt.yFlipped(), ts);
                            break;
                        }
                        case Work::kCubic_Segment: {
                            pts = VCubicIntersect(wn.pts(), wt.top(),
                                    wt.bottom(), wt.x(), wt.yFlipped(), ts);
                            break;
                        }
                        default:
                            SkASSERT(0);
                    }
                    break;
                case Work::kLine_Segment:
                    switch (wn.segmentType()) {
                        case Work::kHorizontalLine_Segment:
                            pts = HLineIntersect(wt.pts(), wn.left(),
                                    wn.right(), wn.y(), wn.xFlipped(), ts);
                            break;
                        case Work::kVerticalLine_Segment:
                            pts = VLineIntersect(wt.pts(), wn.top(),
                                    wn.bottom(), wn.x(), wn.yFlipped(), ts);
                            break;
                        case Work::kLine_Segment: {
                            pts = LineIntersect(wt.pts(), wn.pts(), ts);
                            debugShowLineIntersection(pts, wt, wn,
                                    ts.fT[1], ts.fT[0]);
                            break;
                        }
                        case Work::kQuad_Segment: {
                            swap = true;
                            pts = QuadLineIntersect(wn.pts(), wt.pts(), ts);
                            break;
                        }
                        case Work::kCubic_Segment: {
                            swap = true;
                            pts = CubicLineIntersect(wn.pts(), wt.pts(), ts);
                            break;
                        }
                        default:
                            SkASSERT(0);
                    }
                    break;
                case Work::kQuad_Segment:
                    switch (wn.segmentType()) {
                        case Work::kHorizontalLine_Segment:
                            pts = HQuadIntersect(wt.pts(), wn.left(),
                                    wn.right(), wn.y(), wn.xFlipped(), ts);
                            break;
                        case Work::kVerticalLine_Segment:
                            pts = VQuadIntersect(wt.pts(), wn.top(),
                                    wn.bottom(), wn.x(), wn.yFlipped(), ts);
                            break;
                        case Work::kLine_Segment: {
                            pts = QuadLineIntersect(wt.pts(), wn.pts(), ts);
                            break;
                        }
                        case Work::kQuad_Segment: {
                            pts = QuadIntersect(wt.pts(), wn.pts(), ts);
                            break;
                        }
                        case Work::kCubic_Segment: {
                            wt.promoteToCubic();
                            pts = CubicIntersect(wt.cubic(), wn.pts(), ts);
                            break;
                        }
                        default:
                            SkASSERT(0);
                    }
                    break;
                case Work::kCubic_Segment:
                    switch (wn.segmentType()) {
                        case Work::kHorizontalLine_Segment:
                            pts = HCubicIntersect(wt.pts(), wn.left(),
                                    wn.right(), wn.y(), wn.xFlipped(), ts);
                            break;
                        case Work::kVerticalLine_Segment:
                            pts = VCubicIntersect(wt.pts(), wn.top(),
                                    wn.bottom(), wn.x(), wn.yFlipped(), ts);
                            break;
                        case Work::kLine_Segment: {
                            pts = CubicLineIntersect(wt.pts(), wn.pts(), ts);
                            break;
                        }
                        case Work::kQuad_Segment: {
                            wn.promoteToCubic();
                            pts = CubicIntersect(wt.pts(), wn.cubic(), ts);
                            break;
                        }
                        case Work::kCubic_Segment: {
                            pts = CubicIntersect(wt.pts(), wn.pts(), ts);
                            break;
                        }
                        default:
                            SkASSERT(0);
                    }
                    break;
                default:
                    SkASSERT(0);
            }
            // in addition to recording T values, record matching segment
            bool coincident = pts == 2 && wn.segmentType() <= Work::kLine_Segment
                    && wt.segmentType() <= Work::kLine_Segment;
            for (int pt = 0; pt < pts; ++pt) {
                SkASSERT(ts.fT[0][pt] >= 0 && ts.fT[0][pt] <= 1);
                SkASSERT(ts.fT[1][pt] >= 0 && ts.fT[1][pt] <= 1);
                int testTAt = wt.addT(ts.fT[swap][pt], wn);
                int nextTAt = wn.addT(ts.fT[!swap][pt], wt);
                wt.addOtherT(testTAt, ts.fT[!swap][pt], coincident);
                wn.addOtherT(nextTAt, ts.fT[swap][pt], coincident);
            }
        } while (wn.advance());
    } while (wt.advance());
    return true;
}

// Each segment may have an inside or an outside. Segments contained within
// winding may have insides on either side, and form a contour that should be
// ignored. Segments that are coincident with opposing direction segments may
// have outsides on either side, and should also disappear.
// 'Normal' segments will have one inside and one outside. Subsequent connections
// when winding should follow the intersection direction. If more than one edge
// is an option, choose first edge that continues the inside.
 
static void bridge(SkTDArray<Contour*>& contourList) {
    // Start at the top. Above the top is outside, below is inside.
    Contour* topContour = contourList[0];
    Segment& topStart = topContour->topSegment();
    int index;
    const Segment* topSegment = topStart.findTop(index);
    start here ;
    // find span
    // handle coincident
    // mark neighbors winding coverage
    // output span
    // mark span as processed
    
}

static void makeContourList(SkTArray<Contour>& contours, Contour& sentinel,
        SkTDArray<Contour*>& list) {
    size_t count = contours.count();
    if (count == 0) {
        return;
    }
    for (size_t index = 0; index < count; ++index) {
        *list.append() = &contours[index];
    }
    *list.append() = &sentinel;
    QSort<Contour>(list.begin(), list.end() - 1);
}

void simplifyx(const SkPath& path, bool asFill, SkPath& simple) {
    // returns 1 for evenodd, -1 for winding, regardless of inverse-ness
    int windingMask = (path.getFillType() & 1) ? 1 : -1;
    simple.reset();
    simple.setFillType(SkPath::kEvenOdd_FillType);

    // turn path into list of segments
    SkTArray<Contour> contours;
    // FIXME: add self-intersecting cubics' T values to segment
    EdgeBuilder builder(path, contours);
    SkTDArray<Contour*> contourList;
    Contour sentinel;
    sentinel.reset();
    makeContourList(contours, sentinel, contourList);
    Contour** currentPtr = contourList.begin();
    if (!currentPtr) {
        return;
    }
    // find all intersections between segments
    do {
        Contour** nextPtr = currentPtr;
        Contour* current = *currentPtr++;
        Contour* next;
        do {
            next = *nextPtr++;
        } while (next != &sentinel && addIntersectingTs(current, next));
    } while (*currentPtr != &sentinel);
    // construct closed contours
    bridge(contourList);
}

