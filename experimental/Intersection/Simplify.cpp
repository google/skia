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
#include <algorithm> // used for std::min

#undef SkASSERT
#define SkASSERT(cond) while (!(cond)) { sk_throw(); }

// Terminology:
// A Path contains one of more Contours
// A Contour is made up of Segment array
// A Segment is described by a Verb and a Point array
// A Verb is one of Line, Quad(ratic), and Cubic
// A Segment contains a Span array
// A Span is describes a portion of a Segment using starting and ending T
// T values range from 0 to 1, where 0 is the first Point in the Segment

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

static SkPath::Verb QuadReduceOrder(const SkPoint a[3],
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

static bool QuadIsLinear(const SkPoint a[3]) {
    const Quadratic aQuad = {{a[0].fX, a[0].fY}, {a[1].fX, a[1].fY},
            {a[2].fX, a[2].fY}};
    return isLinear(aQuad, 0, 2);
}

static bool CubicIsLinear(const SkPoint a[4]) {
    const Cubic aCubic = {{a[0].fX, a[0].fY}, {a[1].fX, a[1].fY},
            {a[2].fX, a[2].fY}, {a[3].fX, a[3].fY}};
    return isLinear(aCubic, 0, 3);
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

// sorting angles
// given angles of {dx dy ddx ddy dddx dddy} sort them
class Angle {
public:
    bool operator<(const Angle& rh) const {
        if ((dy < 0) ^ (rh.dy < 0)) {
            return dy < 0;
        }
        SkScalar cmp = dx * rh.dy - rh.dx * dy;
        if (cmp) {
            return cmp < 0;
        }
        if ((ddy < 0) ^ (rh.ddy < 0)) {
            return ddy < 0;
        }
        cmp = ddx * rh.ddy - rh.ddx * ddy;
        if (cmp) {
            return cmp < 0;
        }
        if ((dddy < 0) ^ (rh.dddy < 0)) {
            return ddy < 0;
        }
        return dddx * rh.dddy < rh.dddx * dddy;
    }

    void set(SkPoint* pts, SkPath::Verb verb) {
        dx = pts[1].fX - pts[0].fX; // b - a
        dy = pts[1].fY - pts[0].fY;
        if (verb == SkPath::kLine_Verb) {
            ddx = ddy = dddx = dddy = 0;
            return;
        }
        ddx = pts[2].fX - pts[1].fX - dx; // a - 2b + c
        ddy = pts[2].fY - pts[2].fY - dy;
        if (verb == SkPath::kQuad_Verb) {
            dddx = dddy = 0;
            return;
        }
        dddx = pts[3].fX + 3 * (pts[1].fX - pts[2].fX) - pts[0].fX;
        dddy = pts[3].fY + 3 * (pts[1].fY - pts[2].fY) - pts[0].fY;
    }

private:
    SkScalar dx;
    SkScalar dy;
    SkScalar ddx;
    SkScalar ddy;
    SkScalar dddx;
    SkScalar dddy;
};

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

struct Span {
    double fT;
    Segment* fOther;
    double fOtherT;
    int fWinding; // accumulated from contours surrounding this one
    // OPTIMIZATION: done needs only 2 bits (values are -1, 0, 1)
    int fDone; // set when t to t+fDone is processed 
    // OPTIMIZATION: done needs only 2 bits (values are -1, 0, 1)
    int fCoincident; // -1 start of coincidence, 0 no coincidence, 1 end
};

class Segment {
public:
    Segment() {
#if DEBUG_DUMP
        fID = ++gSegmentID;
#endif
    }
    
    void addAngle(SkTDArray<Angle>& angles, double start, double end) {
        // FIXME complete this
        // start here;
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
    void addOtherT(int index, double other) {
        fTs[index].fOtherT = other;
    }

    bool addQuad(const SkPoint pts[3]) {
        fPts = pts;
        fVerb = SkPath::kQuad_Verb;
        fBounds.setQuadBounds(pts);
    }

    int addT(double newT, Segment& other, int coincident) {
        // FIXME: in the pathological case where there is a ton of intercepts,
        //  binary search?
        int insertedAt = -1;
        Span* span;
        size_t tCount = fTs.count();
        double delta;
        for (size_t idx2 = 0; idx2 < tCount; ++idx2) {
            // OPTIMIZATION: if there are three or more identical Ts, then
            // the fourth and following could be further insertion-sorted so
            // that all the edges are clockwise or counterclockwise.
            // This could later limit segment tests to the two adjacent
            // neighbors, although it doesn't help with determining which
            // circular direction to go in.
            if (newT <= fTs[idx2].fT) {
                insertedAt = idx2;
                span = fTs.insert(idx2);
                goto finish;
            }
        }
        insertedAt = tCount;
        span = fTs.append();
finish:
        span->fT = newT;
        span->fOther = &other;
        span->fWinding = 1;
        span->fDone = 0;
        span->fCoincident = coincident;
        fCoincident |= coincident;
        return insertedAt;
    }

    const Bounds& bounds() const {
        return fBounds;
    }

    bool done() const {
        return fDone;
    }

    int findCoincidentEnd(int start) const {
        int tCount = fTs.count();
        SkASSERT(start < tCount);
        const Span& span = fTs[start];
        SkASSERT(span.fCoincident);
        for (int index = start + 1; index < tCount; ++index) {
            const Span& match = fTs[index];
            if (match.fOther == span.fOther) {
                SkASSERT(match.fCoincident);
                return index;
            }
        }
        SkASSERT(0); // should never get here
        return -1;
    }
    
    // start is the index of the beginning T of this edge
    // it is guaranteed to have an end which describes a non-zero length (?)
    // winding -1 means ccw, 1 means cw
    // step is in/out -1 or 1
    // spanIndex is returned
    Segment* findNext(int start, int winding, int& step, int& spanIndex) {
        SkASSERT(step == 1 || step == -1);
        int count = fTs.count();
        SkASSERT(step > 0 ? start < count - 1 : start > 0);
        Span* startSpan = &fTs[start];
        // FIXME:
        // since Ts can be stepped either way, done markers must be careful
        // not to assume that segment was only ascending in T. This shouldn't
        // be a problem unless pathologically a segment can be partially
        // ascending and partially descending -- maybe quads/cubic can do this?
        startSpan->fDone = step;
        SkPoint startLoc; // OPTIMIZATION: store this in the t span?
        xyAtT(startSpan->fT, &startLoc);
        SkPoint endLoc;
        Span* endSpan;
        int end = nextSpan(start, step, startLoc, startSpan, &endLoc, &endSpan);
        
        // if we hit the end looking for span end, is that always an error?
        SkASSERT(step > 0 ? end + 1 < count : end - 1 >= 0);

        // preflight for coincidence -- if present, it may change winding
        // considerations and whether reversed edges can be followed
        bool foundCoincident = false;
        int last = lastSpan(end, step, &startLoc, startSpan, foundCoincident);
        
        // Discard opposing direction candidates if no coincidence was found.
        int candidateCount = abs(last - end);
        if (candidateCount == 1) {
            SkASSERT(!foundCoincident);
            // move in winding direction until edge in correct direction
            // balance wrong direction edges before finding correct one
            // this requres that the intersection is angularly sorted
            // for a single intersection, special case -- choose the opposite
            // edge that steps the same
            Segment* other = endSpan->fOther;
            SkASSERT(!other->fDone);
            spanIndex = other->matchSpan(this, endSpan->fT);
            SkASSERT(step < 0 ? spanIndex > 0 : spanIndex < other->fTs.count() - 1);
            return other;
        }

        // find the next T that describes a length
        SkTDArray<Angle> angles;
        Segment* segmentCandidate = NULL;
        int spanCandidate = -1;
        int directionCandidate;
        do {
            endSpan = &fTs[end];
            Segment* other = endSpan->fOther;
            if (other->fDone) {
                continue;
            }
        // if there is only one live crossing, and no coincidence, continue
        // in the same direction
        // if there is coincidence, the only choice may be to reverse direction
            // find edge on either side of intersection
            int oCount = other->fTs.count();
            for (int oIndex = 0; oIndex < oCount; ++oIndex) {
                Span& otherSpan = other->fTs[oIndex];
                if (otherSpan.fOther != this) {
                    continue;
                }
                if (otherSpan.fOtherT != endSpan->fT) {
                    continue;
                }
                // if done == -1, prior span has already been processed
                int next = other->nextSpan(oIndex, step, endLoc, &otherSpan,
                        NULL, NULL);
                if (next < 0) {
                    continue;
                }
                bool otherIsCoincident;
                last = other->lastSpan(next, step, &endLoc, &otherSpan,
                        otherIsCoincident);
                if (step < 0) {
                
                    if (otherSpan.fDone >= 0 && oIndex > 0) {
                        // FIXME: this needs to loop on -- until t && pt are different
                        Span& prior = other->fTs[oIndex - 1];
                        if (prior.fDone > 0) {
                            continue;
                        }
                        
                    }
                } else { // step == 1
                    if (otherSpan.fDone <= 0 && oIndex < oCount - 1) {
                        // FIXME: this needs to loop on ++ until t && pt are different
                        Span& next = other->fTs[oIndex + 1];
                        if (next.fDone < 0) {
                            continue;
                        }
                    }
                }
                if (!segmentCandidate) {
                    segmentCandidate = other;
                    spanCandidate = oIndex;
                    directionCandidate = step;
                    continue;
                }
                // there's two or more matches
                if (spanCandidate >= 0) { // retrieve first stored candidate
                    // add edge leading into junction
                    addAngle(angles, endSpan->fT, startSpan->fT);
                    // add edge leading away from junction
                    double nextT = nextSpan(end, step, endLoc, endSpan, NULL,
                            NULL);
                    if (nextT >= 0) {
                        addAngle(angles, endSpan->fT, nextT);
                    }
                    // add first stored candidate into junction
                    segmentCandidate->addAngle(angles,
                            segmentCandidate->fTs[spanCandidate - 1].fT,
                            segmentCandidate->fTs[spanCandidate].fT);
                    // add first stored candidate away from junction
                    segmentCandidate->addAngle(angles,
                            segmentCandidate->fTs[spanCandidate].fT,
                            segmentCandidate->fTs[spanCandidate + 1].fT);
                }
                // add candidate into and away from junction
                
                
           //     start here;
                // more than once viable candidate -- need to
                //  measure angles to find best
                // noncoincident quads/cubics may have the same initial angle
                // as lines, so must sort by derivatives as well
                // while we're here, figure out all connections given the 
                //  initial winding info
                // so the span needs to contain the pairing info found here
                // this should include the winding computed for the edge, and
                //  what edge it connects to, and whether it is discarded
                //  (maybe discarded == abs(winding) > 1) ?
                // only need derivatives for duration of sorting, add a new struct
                // for pairings, remove extra spans that have zero length and
                //  reference an unused other
                // for coincident, the last span on the other may be marked done
                //  (always?)
            }
        } while ((end += step) != last);
        // if loop is exhausted, contour may be closed.
        // FIXME: pass in close point so we can check for closure

        // given a segment, and a sense of where 'inside' is, return the next
        // segment. If this segment has an intersection, or ends in multiple
        // segments, find the mate that continues the outside.
        // note that if there are multiples, but no coincidence, we can limit
        // choices to connections in the correct direction
        
        // mark found segments as done
    }

    void findTooCloseToCall(int winding) {
        int count = fTs.count();
        if (count < 3) { // require t=0, x, 1 at minimum
            return;
        }
        int matchIndex = 0;
        int moCount;
        Span* match;
        Segment* mOther;
        do {
            match = &fTs[matchIndex];
            mOther = match->fOther;
            moCount = mOther->fTs.count();
        } while (moCount >= 3 || ++matchIndex < count - 1); // require t=0, x, 1 at minimum
        SkPoint matchPt;
        // OPTIMIZATION: defer matchPt until qualifying toCount is found?
        xyAtT(match->fT, &matchPt);
        // look for a pair of nearby T values that map to the same (x,y) value
        // if found, see if the pair of other segments share a common point. If
        // so, the span from here to there is coincident.
        for (int index = matchIndex + 1; index < count; ++index) {
            Span* test = &fTs[index];
            Segment* tOther = test->fOther;
            int toCount = tOther->fTs.count();
            if (toCount < 3) { // require t=0, x, 1 at minimum
                continue;
            }
            SkPoint testPt;
            xyAtT(test->fT, &testPt);
            if (matchPt != testPt) {
                matchIndex = index;
                moCount = toCount;
                match = test;
                mOther = tOther;
                matchPt = testPt;
                continue;
            }
            int moStart = -1; // FIXME: initialization is debugging only
            for (int moIndex = 0; moIndex < moCount; ++moIndex) {
                Span& moSpan = mOther->fTs[moIndex];
                if (moSpan.fOther == this) {
                    if (moSpan.fOtherT == match->fT) {
                        moStart = moIndex;
                    }
                    continue;
                }
                if (moSpan.fOther != tOther) {
                    continue;
                }
                int toStart = -1;
                int toIndex; // FIXME: initialization is debugging only
                bool found = false;
                for (toIndex = 0; toIndex < toCount; ++toIndex) {
                    Span& toSpan = tOther->fTs[toIndex];
                    if (toSpan.fOther == this) {
                        if (toSpan.fOtherT == test->fT) {
                            toStart = toIndex;
                        }
                        continue;
                    }
                    if (toSpan.fOther == mOther && toSpan.fOtherT
                            == moSpan.fT) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    continue;
                }
                SkASSERT(moStart >= 0);
                SkASSERT(toStart >= 0);
                // test to see if the segment between there and here is linear
                if (!mOther->isLinear(moStart, moIndex)
                        || !tOther->isLinear(toStart, toIndex)) {
                    continue;
                }
                mOther->fTs[moStart].fCoincident = -1;
                tOther->fTs[toStart].fCoincident = -1;
                mOther->fTs[moIndex].fCoincident = 1;
                tOther->fTs[toIndex].fCoincident = 1;
            }
    nextStart:
            ;
        }
    }

    int findByT(double t, const Segment* match) const {
        // OPTIMIZATION: bsearch if count is honkin huge
        int count = fTs.count();
        for (int index = 0; index < count; ++index) {
            const Span& span = fTs[index];
            if (t == span.fT && match == span.fOther) {
                return index;
            }
        }
        SkASSERT(0); // should never get here
        return -1;
    }

    // find the adjacent T that is leftmost, with a point != base
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
        SkASSERT(0); // can't get here (?)
        return -1;
    }

    // OPTIMIZATION : for a pair of lines, can we compute points at T (cached)
    // and use more concise logic like the old edge walker code?
    // FIXME: this needs to deal with coincident edges
    const Segment* findTop(int& tIndex) const {
        // iterate through T intersections and return topmost
        // topmost tangent from y-min to first pt is closer to horizontal
        int firstT = 0;
        int lastT = 0;
        SkScalar topY = fPts[0].fY;
        int count = fTs.count();
        int index;
        for (index = 1; index < count; ++index) {
            const Span& span = fTs[index];
            double t = span.fT;
            SkScalar yIntercept = yAtT(t);
            if (topY > yIntercept) {
                topY = yIntercept;
                firstT = lastT = index;
            } else if (topY == yIntercept) {
                lastT = index;
            }
        }
        // if there's only a pair of segments, go with the endpoint chosen above
        if (firstT == lastT && (firstT == 0 || firstT == count - 1)) {
            tIndex = firstT;
            return this;
        }
        // if the topmost T is not on end, or is three-way or more, find left
        SkPoint leftBase;
        xyAtT(firstT, &leftBase);
        int tLeft = findLefty(firstT, leftBase);
        const Segment* leftSegment = this;
        // look for left-ness from tLeft to firstT (matching y of other)
        for (index = firstT; index <= lastT; ++index) {
            const Segment* other = fTs[index].fOther;
            double otherT = fTs[index].fOtherT;
            int otherTIndex = other->findByT(otherT, this);
            // pick companionT closest (but not too close) on either side
            int otherTLeft = other->findLefty(otherTIndex, leftBase);
            // within this span, find highest y
            SkPoint testPt, otherPt;
            testPt.fY = yAtT(tLeft);
            otherPt.fY = other->yAtT(otherTLeft);
            // FIXME: incomplete
            // find the y intercept with the opposite segment
            if (testPt.fY < otherPt.fY) {

            } else if (testPt.fY > otherPt.fY) {

            }
            // FIXME: leftMost no good. Use y intercept instead
#if 0
            SkScalar otherMost = other->leftMost(otherTIndex, otherTLeft);
            if (otherMost < left) {
                leftSegment = other;
            }
#endif
        }
        return leftSegment;
    }

    bool intersected() const {
        return fTs.count() > 0;
    }
    
    bool isLinear(int start, int end) const {
        if (fVerb == SkPath::kLine_Verb) {
            return true;
        }
        if (fVerb == SkPath::kQuad_Verb) {
            SkPoint qPart[3];
            QuadSubDivide(fPts, fTs[start].fT, fTs[end].fT, qPart);
            return QuadIsLinear(qPart);
        } else {
            SkASSERT(fVerb == SkPath::kCubic_Verb);
            SkPoint cPart[4];
            CubicSubDivide(fPts, fTs[start].fT, fTs[end].fT, cPart);
            return CubicIsLinear(cPart);
        }
    }

    bool isHorizontal() const {
        return fBounds.fTop == fBounds.fBottom;
    }

    bool isVertical() const {
        return fBounds.fLeft == fBounds.fRight;
    }

    int lastSpan(int end, int step, const SkPoint* startLoc,
            const Span* startSpan, bool& coincident) {
        int last = end;
        int count = fTs.count();
        coincident = false;
        SkPoint lastLoc;
        do {
            if (fTs[last].fCoincident == -step) {
                coincident = true;
            }
            if (step > 0 ? ++last < count : --last >= 0) {
                break;
            }
            Span* lastSpan = &fTs[last];
            if (lastSpan->fT == startSpan->fT) {
                continue;
            }
            xyAtT(lastSpan->fT, &lastLoc);
        } while (*startLoc == lastLoc);
    }

    SkScalar leftMost(int start, int end) const {
        return (*SegmentLeftMost[fVerb])(fPts, fTs[start].fT, fTs[end].fT);
    }

    int matchSpan(const Segment* match, double matchT)
    {
        int count = fTs.count();
        for (int index = 0; index < count; ++index) {
            Span& span = fTs[index];
            if (span.fOther != match) {
                continue;
            }
            if (span.fOtherT != matchT) {
                continue;
            }
            return index;
        }
        SkASSERT(0); // should never get here
        return -1;
    }

    int nextSpan(int from, int step, const SkPoint& fromLoc,
            const Span* fromSpan, SkPoint* toLoc, Span** toSpan) {
        int count = fTs.count();
        int to = from;
        while (step > 0 ? ++to < count : --to >= 0) {
            Span* span = &fTs[to];
            if (span->fT == fromSpan->fT) {
                continue;
            }
            SkPoint loc;
            xyAtT(span->fT, &loc);
            if (fromLoc == loc) {
                continue;
            }
            if (toLoc) {
                *toLoc = loc;
            }
            if (toSpan) {
                *toSpan = span;
            }
            return to;
        }
        return -1;
    }

    const SkPoint* pts() const {
        return fPts;
    }

    void reset() {
        fPts = NULL;
        fVerb = (SkPath::Verb) -1;
        fBounds.set(SK_ScalarMax, SK_ScalarMax, SK_ScalarMax, SK_ScalarMax);
        fTs.reset();
        fDone = false;
        fCoincident = 0;
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

    SkScalar yAtT(double t) const {
        return (*SegmentYAtT[fVerb])(fPts, t);
    }

#if DEBUG_DUMP
    void dump() const {
        const char className[] = "Segment";
        const int tab = 4;
        for (int i = 0; i < fTs.count(); ++i) {
            SkPoint out;
            (*SegmentXYAtT[fVerb])(fPts, t(i), &out);
            SkDebugf("%*s [%d] %s.fTs[%d]=%1.9g (%1.9g,%1.9g) other=%d"
                    " otherT=%1.9g winding=%d\n",
                    tab + sizeof(className), className, fID,
                    kLVerbStr[fVerb], i, fTs[i].fT, out.fX, out.fY,
                    fTs[i].fOther->fID, fTs[i].fOtherT, fTs[i].fWinding);
        }
        SkDebugf("%*s [%d] fBounds=(l:%1.9g, t:%1.9g r:%1.9g, b:%1.9g)",
                tab + sizeof(className), className, fID,
                fBounds.fLeft, fBounds.fTop, fBounds.fRight, fBounds.fBottom);
    }
#endif

private:
    const SkPoint* fPts;
    SkPath::Verb fVerb;
    Bounds fBounds;
    SkTDArray<Span> fTs; // two or more (always includes t=0 t=1)
    // FIXME: coincident only needs two bits (-1, 0, 1)
    int fCoincident; // non-zero if some coincident span inside 
    bool fDone;
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

    void findTooCloseToCall(int winding) {
        int segmentCount = fSegments.count();
        for (int sIndex = 0; sIndex < segmentCount; ++sIndex) {
            fSegments[sIndex].findTooCloseToCall(winding);
        }
    }

    void reset() {
        fSegments.reset();
        fBounds.set(SK_ScalarMax, SK_ScalarMax, SK_ScalarMax, SK_ScalarMax);
        fContainsCurves = fContainsIntercepts = false;
    }
    
    // OPTIMIZATION: feel pretty uneasy about this. It seems like once again
    // we need to sort and walk edges in y, but that on the surface opens the
    // same can of worms as before. But then, this is a rough sort based on 
    // segments' top, and not a true sort, so it could be ameniable to regular
    // sorting instead of linear searching. Still feel like I'm missing something
    Segment* topSegment() {
        int segmentCount = fSegments.count();
        SkASSERT(segmentCount > 0);
        int best = -1;
        Segment* bestSegment = NULL;
        while (++best < segmentCount) {
            Segment* testSegment = &fSegments[best];
            if (testSegment->done()) {
                continue;
            }
            bestSegment = testSegment;
            break;
        }
        if (!bestSegment) {
            return NULL;
        }
        SkScalar bestTop = bestSegment->bounds().fTop;
        for (int test = best + 1; test < segmentCount; ++test) {
            Segment* testSegment = &fSegments[test];
            if (testSegment->done()) {
                continue;
            }
            SkScalar testTop = testSegment->bounds().fTop;
            if (bestTop > testTop) {
                bestTop = testTop;
                bestSegment = testSegment;
            }
        }
        return bestSegment;
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
        fBounds = fSegments.front().bounds();
        for (int index = 1; index < count; ++index) {
            fBounds.growToInclude(fSegments[index].bounds());
        }
    }

public:
    SkTArray<Segment> fSegments; // not worth accessor functions?

private:
    Bounds fBounds;
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

    void addOtherT(int index, double other) {
        fContour->fSegments[fIndex].addOtherT(index, other);
    }

    // Avoid collapsing t values that are close to the same since
    // we walk ts to describe consecutive intersections. Since a pair of ts can
    // be nearly equal, any problems caused by this should be taken care
    // of later.
    // On the edge or out of range values are negative; add 2 to get end
    int addT(double newT, const Work& other, int coincident) {
        fContour->containsIntercepts();
        return fContour->fSegments[fIndex].addT(newT,
                other.fContour->fSegments[other.fIndex], coincident);
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

static bool addIntersectTs(Contour* test, Contour* next, int winding) {
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
            int coincident = pts == 2 && wn.segmentType() <= Work::kLine_Segment
                    && wt.segmentType() <= Work::kLine_Segment ? -1 :0;
            for (int pt = 0; pt < pts; ++pt) {
                SkASSERT(ts.fT[0][pt] >= 0 && ts.fT[0][pt] <= 1);
                SkASSERT(ts.fT[1][pt] >= 0 && ts.fT[1][pt] <= 1);
                int testTAt = wt.addT(ts.fT[swap][pt], wn, coincident);
                int nextTAt = wn.addT(ts.fT[!swap][pt], wt, coincident);
                wt.addOtherT(testTAt, ts.fT[!swap][pt]);
                wn.addOtherT(nextTAt, ts.fT[swap][pt]);
                coincident = -coincident;
            }
        } while (wn.advance());
    } while (wt.advance());
    return true;
}

// see if coincidence is formed by clipping non-concident segments
static void coincidenceCheck(SkTDArray<Contour*>& contourList, int winding) {
    int contourCount = contourList.count();
    for (size_t cIndex = 0; cIndex < contourCount; ++cIndex) {
        Contour* contour = contourList[cIndex];
        contour->findTooCloseToCall(winding);
    }
}

// Each segment may have an inside or an outside. Segments contained within
// winding may have insides on either side, and form a contour that should be
// ignored. Segments that are coincident with opposing direction segments may
// have outsides on either side, and should also disappear.
// 'Normal' segments will have one inside and one outside. Subsequent connections
// when winding should follow the intersection direction. If more than one edge
// is an option, choose first edge that continues the inside.

static void bridge(SkTDArray<Contour*>& contourList) {
    int contourCount = contourList.count();
    do {
    // OPTIMIZATION: not crazy about linear search here to find top active y.
    // seems like we should break down and do the sort, or maybe sort each
    // contours' segments? 
    // Once the segment array is built, there's no reason I can think of not to
    // sort it in Y. hmmm
        int cIndex = 0;
        Segment* topStart;
        do {
            Contour* topContour = contourList[cIndex];
            topStart = topContour->topSegment();
        } while (!topStart && ++cIndex < contourCount);
        if (!topStart) {
            break;
        }
        SkScalar top = topStart->bounds().fTop;
        for (int cTest = cIndex + 1; cTest < contourCount; ++cTest) {
            Contour* contour = contourList[cTest];
            if (top < contour->bounds().fTop) {
                continue;
            }
            Segment* test = contour->topSegment();
            if (top > test->bounds().fTop) {
                cIndex = cTest;
                topStart = test;
                top = test->bounds().fTop;
            }
        }
        int index;
        const Segment* topSegment = topStart->findTop(index);
        // Start at the top. Above the top is outside, below is inside.
        // follow edges to intersection
        // at intersection, stay on outside, but mark remaining edges as inside
            // or, only mark first pair as inside?
            // how is this going to work for contained (but not intersecting)
            //  segments?
 //   start here ;
    // find span
    // mark neighbors winding coverage
    // output span
    // mark span as processed
        
    } while (true);
        

}

static void makeContourList(SkTArray<Contour>& contours, Contour& sentinel,
        SkTDArray<Contour*>& list) {
    int count = contours.count();
    if (count == 0) {
        return;
    }
    for (int index = 0; index < count; ++index) {
        *list.append() = &contours[index];
    }
    *list.append() = &sentinel;
    QSort<Contour>(list.begin(), list.end() - 1);
}

void simplifyx(const SkPath& path, bool asFill, SkPath& simple) {
    // returns 1 for evenodd, -1 for winding, regardless of inverse-ness
    int winding = (path.getFillType() & 1) ? 1 : -1;
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
        } while (next != &sentinel && addIntersectTs(current, next, winding));
    } while (*currentPtr != &sentinel);
    // eat through coincident edges
    coincidenceCheck(contourList, winding);
    // construct closed contours
    bridge(contourList);
}

