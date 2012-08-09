/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Simplify.h"

#undef SkASSERT
#define SkASSERT(cond) while (!(cond)) { sk_throw(); }

// Terminology:
// A Path contains one of more Contours
// A Contour is made up of Segment array
// A Segment is described by a Verb and a Point array with 2, 3, or 4 points
// A Verb is one of Line, Quad(ratic), or Cubic
// A Segment contains a Span array
// A Span is describes a portion of a Segment using starting and ending T
// T values range from 0 to 1, where 0 is the first Point in the Segment
// An Edge is a Segment generated from a Span

// FIXME: remove once debugging is complete
#ifdef SK_DEBUG
int gDebugMaxWindSum = SK_MaxS32;
int gDebugMaxWindValue = SK_MaxS32;
#endif

#define DEBUG_UNUSED 0 // set to expose unused functions

#if 0 // set to 1 for multiple thread -- no debugging

const bool gRunTestsInOneThread = false;

#define DEBUG_ACTIVE_SPANS 0
#define DEBUG_ADD_INTERSECTING_TS 0
#define DEBUG_ADD_T_PAIR 0
#define DEBUG_CONCIDENT 0
#define DEBUG_CROSS 0
#define DEBUG_DUMP 0
#define DEBUG_MARK_DONE 0
#define DEBUG_PATH_CONSTRUCTION 0
#define DEBUG_SORT 0
#define DEBUG_WIND_BUMP 0
#define DEBUG_WINDING 0

#else

const bool gRunTestsInOneThread = true;

#define DEBUG_ACTIVE_SPANS 1
#define DEBUG_ADD_INTERSECTING_TS 0
#define DEBUG_ADD_T_PAIR 1
#define DEBUG_CONCIDENT 1
#define DEBUG_CROSS 0
#define DEBUG_DUMP 1
#define DEBUG_MARK_DONE 1
#define DEBUG_PATH_CONSTRUCTION 1
#define DEBUG_SORT 1
#define DEBUG_WIND_BUMP 0
#define DEBUG_WINDING 1

#endif

#if (DEBUG_ACTIVE_SPANS || DEBUG_CONCIDENT || DEBUG_SORT) && !DEBUG_DUMP
#undef DEBUG_DUMP
#define DEBUG_DUMP 1
#endif

#if DEBUG_DUMP
static const char* kLVerbStr[] = {"", "line", "quad", "cubic"};
// static const char* kUVerbStr[] = {"", "Line", "Quad", "Cubic"};
static int gContourID;
static int gSegmentID;
#endif

#ifndef DEBUG_TEST
#define DEBUG_TEST 0
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

static int HQuadIntersect(const SkPoint a[3], SkScalar left, SkScalar right,
        SkScalar y, bool flipped, Intersections& intersections) {
    const Quadratic aQuad = {{a[0].fX, a[0].fY}, {a[1].fX, a[1].fY}, {a[2].fX, a[2].fY}};
    return horizontalIntersect(aQuad, left, right, y, flipped, intersections);
}

static int HCubicIntersect(const SkPoint a[4], SkScalar left, SkScalar right,
        SkScalar y, bool flipped, Intersections& intersections) {
    const Cubic aCubic = {{a[0].fX, a[0].fY}, {a[1].fX, a[1].fY}, {a[2].fX, a[2].fY},
            {a[3].fX, a[3].fY}};
    return horizontalIntersect(aCubic, left, right, y, flipped, intersections);
}

static int VLineIntersect(const SkPoint a[2], SkScalar top, SkScalar bottom,
        SkScalar x, bool flipped, Intersections& intersections) {
    const _Line aLine = {{a[0].fX, a[0].fY}, {a[1].fX, a[1].fY}};
    return verticalIntersect(aLine, top, bottom, x, flipped, intersections);
}

static int VQuadIntersect(const SkPoint a[3], SkScalar top, SkScalar bottom,
        SkScalar x, bool flipped, Intersections& intersections) {
    const Quadratic aQuad = {{a[0].fX, a[0].fY}, {a[1].fX, a[1].fY}, {a[2].fX, a[2].fY}};
    return verticalIntersect(aQuad, top, bottom, x, flipped, intersections);
}

static int VCubicIntersect(const SkPoint a[4], SkScalar top, SkScalar bottom,
        SkScalar x, bool flipped, Intersections& intersections) {
    const Cubic aCubic = {{a[0].fX, a[0].fY}, {a[1].fX, a[1].fY}, {a[2].fX, a[2].fY},
            {a[3].fX, a[3].fY}};
    return verticalIntersect(aCubic, top, bottom, x, flipped, intersections);
}

static int (* const VSegmentIntersect[])(const SkPoint [], SkScalar ,
        SkScalar , SkScalar , bool , Intersections& ) = {
    NULL,
    VLineIntersect,
    VQuadIntersect,
    VCubicIntersect
};

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

static SkScalar LineDXAtT(const SkPoint a[2], double ) {
    return a[1].fX - a[0].fX;
}

static SkScalar QuadDXAtT(const SkPoint a[3], double t) {
    const Quadratic quad = {{a[0].fX, a[0].fY}, {a[1].fX, a[1].fY}, {a[2].fX, a[2].fY}};
    double x;
    dxdy_at_t(quad, t, x, *(double*) 0);
    return SkDoubleToScalar(x);
}

static SkScalar CubicDXAtT(const SkPoint a[4], double t) {
    const Cubic cubic = {{a[0].fX, a[0].fY}, {a[1].fX, a[1].fY}, {a[2].fX, a[2].fY},
            {a[3].fX, a[3].fY}};
    double x;
    dxdy_at_t(cubic, t, x, *(double*) 0);
    return SkDoubleToScalar(x);
}

static SkScalar (* const SegmentDXAtT[])(const SkPoint [], double ) = {
    NULL,
    LineDXAtT,
    QuadDXAtT,
    CubicDXAtT
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

static void (* const SegmentSubDivide[])(const SkPoint [], double , double ,
        SkPoint []) = {
    NULL,
    LineSubDivide,
    QuadSubDivide,
    CubicSubDivide
};

#if DEBUG_UNUSED
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
#endif

static SkPath::Verb QuadReduceOrder(const SkPoint a[3],
        SkTDArray<SkPoint>& reducePts) {
    const Quadratic aQuad = {{a[0].fX, a[0].fY}, {a[1].fX, a[1].fY},
            {a[2].fX, a[2].fY}};
    Quadratic dst;
    int order = reduceOrder(aQuad, dst);
    if (order == 3) {
        return SkPath::kQuad_Verb;
    }
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
    if (order == 4) {
        return SkPath::kCubic_Verb;
    }
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
    xy_at_t(aLine, endT, x[1], *(double*) 0);
    return SkMinScalar((float) x[0], (float) x[1]);
}

static SkScalar QuadLeftMost(const SkPoint a[3], double startT, double endT) {
    const Quadratic aQuad = {{a[0].fX, a[0].fY}, {a[1].fX, a[1].fY},
            {a[2].fX, a[2].fY}};
    return (float) leftMostT(aQuad, startT, endT);
}

static SkScalar CubicLeftMost(const SkPoint a[4], double startT, double endT) {
    const Cubic aCubic = {{a[0].fX, a[0].fY}, {a[1].fX, a[1].fY},
            {a[2].fX, a[2].fY}, {a[3].fX, a[3].fY}};
    return (float) leftMostT(aCubic, startT, endT);
}

static SkScalar (* const SegmentLeftMost[])(const SkPoint [], double , double) = {
    NULL,
    LineLeftMost,
    QuadLeftMost,
    CubicLeftMost
};

#if DEBUG_UNUSED
static bool IsCoincident(const SkPoint a[2], const SkPoint& above,
        const SkPoint& below) {
    const _Line aLine = {{a[0].fX, a[0].fY}, {a[1].fX, a[1].fY}};
    const _Line bLine = {{above.fX, above.fY}, {below.fX, below.fY}};
    return implicit_matches_ulps(aLine, bLine, 32);
}
#endif

class Segment;

// sorting angles
// given angles of {dx dy ddx ddy dddx dddy} sort them
class Angle {
public:
    // FIXME: this is bogus for quads and cubics
    // if the quads and cubics' line from end pt to ctrl pt are coincident,
    // there's no obvious way to determine the curve ordering from the
    // derivatives alone. In particular, if one quadratic's coincident tangent
    // is longer than the other curve, the final control point can place the
    // longer curve on either side of the shorter one.
    // Using Bezier curve focus http://cagd.cs.byu.edu/~tom/papers/bezclip.pdf
    // may provide some help, but nothing has been figured out yet.
    bool operator<(const Angle& rh) const {
        if ((fDy < 0) ^ (rh.fDy < 0)) {
            return fDy < 0;
        }
        if (fDy == 0 && rh.fDy == 0 && fDx != rh.fDx) {
            return fDx < rh.fDx;
        }
        SkScalar cmp = fDx * rh.fDy - rh.fDx * fDy;
        if (cmp) {
            return cmp < 0;
        }
        if ((fDDy < 0) ^ (rh.fDDy < 0)) {
            return fDDy < 0;
        }
        if (fDDy == 0 && rh.fDDy == 0 && fDDx != rh.fDDx) {
            return fDDx < rh.fDDx;
        }
        cmp = fDDx * rh.fDDy - rh.fDDx * fDDy;
        if (cmp) {
            return cmp < 0;
        }
        if ((fDDDy < 0) ^ (rh.fDDDy < 0)) {
            return fDDDy < 0;
        }
        if (fDDDy == 0 && rh.fDDDy == 0) {
            return fDDDx < rh.fDDDx;
        }
        return fDDDx * rh.fDDDy < rh.fDDDx * fDDDy;
    }
    
    double dx() const {
        return fDx;
    }

    double dy() const {
        return fDy;
    }

    int end() const {
        return fEnd;
    }

    bool isHorizontal() const {
        return fDy == 0 && fDDy == 0 && fDDDy == 0;
    }

    // since all angles share a point, this needs to know which point
    // is the common origin, i.e., whether the center is at pts[0] or pts[verb]
    // practically, this should only be called by addAngle
    void set(const SkPoint* pts, SkPath::Verb verb, const Segment* segment,
            int start, int end) {
        SkASSERT(start != end);
        fSegment = segment;
        fStart = start;
        fEnd = end;
        fDx = pts[1].fX - pts[0].fX; // b - a
        fDy = pts[1].fY - pts[0].fY;
        if (verb == SkPath::kLine_Verb) {
            fDDx = fDDy = fDDDx = fDDDy = 0;
            return;
        }
        fDDx = pts[2].fX - pts[1].fX - fDx; // a - 2b + c
        fDDy = pts[2].fY - pts[1].fY - fDy;
        if (verb == SkPath::kQuad_Verb) {
            fDDDx = fDDDy = 0;
            return;
        }
        fDDDx = pts[3].fX + 3 * (pts[1].fX - pts[2].fX) - pts[0].fX;
        fDDDy = pts[3].fY + 3 * (pts[1].fY - pts[2].fY) - pts[0].fY;
    }

    // noncoincident quads/cubics may have the same initial angle
    // as lines, so must sort by derivatives as well
    // if flatness turns out to be a reasonable way to sort, use the below:
    void setFlat(const SkPoint* pts, SkPath::Verb verb, Segment* segment,
            int start, int end) {
        fSegment = segment;
        fStart = start;
        fEnd = end;
        fDx = pts[1].fX - pts[0].fX; // b - a
        fDy = pts[1].fY - pts[0].fY;
        if (verb == SkPath::kLine_Verb) {
            fDDx = fDDy = fDDDx = fDDDy = 0;
            return;
        }
        if (verb == SkPath::kQuad_Verb) {
            int uplsX = FloatAsInt(pts[2].fX - pts[1].fY - fDx);
            int uplsY = FloatAsInt(pts[2].fY - pts[1].fY - fDy);
            int larger = std::max(abs(uplsX), abs(uplsY));
            int shift = 0;
            double flatT;
            SkPoint ddPt; // FIXME: get rid of copy (change fDD_ to point)
            LineParameters implicitLine;
            _Line tangent = {{pts[0].fX, pts[0].fY}, {pts[1].fX, pts[1].fY}};
            implicitLine.lineEndPoints(tangent);
            implicitLine.normalize();
            while (larger > UlpsEpsilon * 1024) {
                larger >>= 2;
                ++shift;
                flatT = 0.5 / (1 << shift);
                QuadXYAtT(pts, flatT, &ddPt);
                _Point _pt = {ddPt.fX, ddPt.fY};
                double distance = implicitLine.pointDistance(_pt);
                if (approximately_zero(distance)) {
                    SkDebugf("%s ulps too small %1.9g\n", __FUNCTION__, distance);
                    break;
                }
            }
            flatT = 0.5 / (1 << shift);
            QuadXYAtT(pts, flatT, &ddPt);
            fDDx = ddPt.fX - pts[0].fX;
            fDDy = ddPt.fY - pts[0].fY;
            SkASSERT(fDDx != 0 || fDDy != 0);
            fDDDx = fDDDy = 0;
            return;
        }
        SkASSERT(0); // FIXME: add cubic case
    }
    
    Segment* segment() const {
        return const_cast<Segment*>(fSegment);
    }
    
    int sign() const {
        return SkSign32(fStart - fEnd);
    }
    
    int start() const {
        return fStart;
    }

private:
    SkScalar fDx;
    SkScalar fDy;
    SkScalar fDDx;
    SkScalar fDDy;
    SkScalar fDDDx;
    SkScalar fDDDy;
    const Segment* fSegment;
    int fStart;
    int fEnd;
};

static void sortAngles(SkTDArray<Angle>& angles, SkTDArray<Angle*>& angleList) {
    int angleCount = angles.count();
    int angleIndex;
    angleList.setReserve(angleCount);
    for (angleIndex = 0; angleIndex < angleCount; ++angleIndex) {
        *angleList.append() = &angles[angleIndex];
    }
    QSort<Angle>(angleList.begin(), angleList.end() - 1);
}

// Bounds, unlike Rect, does not consider a line to be empty.
struct Bounds : public SkRect {
    static bool Intersects(const Bounds& a, const Bounds& b) {
        return a.fLeft <= b.fRight && b.fLeft <= a.fRight &&
                a.fTop <= b.fBottom && b.fTop <= a.fBottom;
    }

    void add(SkScalar left, SkScalar top, SkScalar right, SkScalar bottom) {
        if (left < fLeft) {
            fLeft = left;
        }
        if (top < fTop) {
            fTop = top;
        }
        if (right > fRight) {
            fRight = right;
        }
        if (bottom > fBottom) {
            fBottom = bottom;
        }
    }

    void add(const Bounds& toAdd) {
        add(toAdd.fLeft, toAdd.fTop, toAdd.fRight, toAdd.fBottom);
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
        set((float) dRect.left, (float) dRect.top, (float) dRect.right,
                (float) dRect.bottom);
    }

    void setQuadBounds(const SkPoint a[3]) {
        const Quadratic quad = {{a[0].fX, a[0].fY}, {a[1].fX, a[1].fY},
                {a[2].fX, a[2].fY}};
        _Rect dRect;
        dRect.setBounds(quad);
        set((float) dRect.left, (float) dRect.top, (float) dRect.right,
                (float) dRect.bottom);
    }
};

static bool useInnerWinding(int outerWinding, int innerWinding) {
    SkASSERT(outerWinding != innerWinding);
    int absOut = abs(outerWinding);
    int absIn = abs(innerWinding);
    bool result = absOut == absIn ? outerWinding < 0 : absOut < absIn;
    if (outerWinding * innerWinding < 0) {
#if DEBUG_WINDING
        SkDebugf("%s *** outer=%d inner=%d result=%s\n", __FUNCTION__,
                outerWinding, innerWinding, result ? "true" : "false");
#endif
    }
    return result;
}

struct Span {
    Segment* fOther;
    mutable SkPoint fPt; // lazily computed as needed
    double fT;
    double fOtherT; // value at fOther[fOtherIndex].fT
    int fOtherIndex;  // can't be used during intersection
    int fWindSum; // accumulated from contours surrounding this one
    int fWindValue; // 0 == canceled; 1 == normal; >1 == coincident
    bool fDone; // if set, this span to next higher T has been processed
};

class Segment {
public:
    Segment() {
#if DEBUG_DUMP
        fID = ++gSegmentID;
#endif
    }

    bool activeAngle(int index, int& done, SkTDArray<Angle>& angles) const {
        if (activeAngleInner(index, done, angles)) {
            return true;
        }
        double referenceT = fTs[index].fT;
        int lesser = index;
        while (--lesser >= 0 && referenceT - fTs[lesser].fT < FLT_EPSILON) {
            if (activeAngleOther(lesser, done, angles)) {
                return true;
            }
        }
        do {
            if (activeAngleOther(index, done, angles)) {
                return true;
            }
        } while (++index < fTs.count() && fTs[index].fT - referenceT < FLT_EPSILON);
        return false;
    }

    bool activeAngleOther(int index, int& done, SkTDArray<Angle>& angles) const {
        Span* span = &fTs[index];
        Segment* other = span->fOther;
        int oIndex = span->fOtherIndex;
        return other->activeAngleInner(oIndex, done, angles);
    }
    
    bool activeAngleInner(int index, int& done, SkTDArray<Angle>& angles) const {
        int next = nextSpan(index, 1);
        if (next > 0) {
            const Span& upSpan = fTs[index];
            if (upSpan.fWindValue) {
                addAngle(angles, index, next);
                if (upSpan.fDone) {
                    done++;
                } else if (upSpan.fWindSum != SK_MinS32) {
                    return true;
                }
            }
        }
        int prev = nextSpan(index, -1);
        // edge leading into junction
        if (prev >= 0) {
            const Span& downSpan = fTs[prev];
            if (downSpan.fWindValue) {
                addAngle(angles, index, prev);
                if (downSpan.fDone) {
                    done++;
                 } else if (downSpan.fWindSum != SK_MinS32) {
                    return true;
                }
            }
        }
        return false;
    }

    SkScalar activeTop() const {
        SkASSERT(!done());
        int count = fTs.count();
        SkScalar result = SK_ScalarMax;
        bool lastDone = true;
        for (int index = 0; index < count; ++index) {
            bool done = fTs[index].fDone;
            if (!done || !lastDone) {
                SkScalar y = yAtT(index);
                if (result > y) {
                    result = y;
                }
            }
            lastDone = done;
        }
        SkASSERT(result < SK_ScalarMax);
        return result;
    }

    void addAngle(SkTDArray<Angle>& angles, int start, int end) const {
        SkASSERT(start != end);
        SkPoint edge[4];
        (*SegmentSubDivide[fVerb])(fPts, fTs[start].fT, fTs[end].fT, edge);
        Angle* angle = angles.append();
        angle->set(edge, fVerb, this, start, end);
    }

    void addCancelOutsides(double tStart, double oStart, Segment& other,
            double oEnd) {
        int tIndex = -1;
        int tCount = fTs.count();
        int oIndex = -1;
        int oCount = other.fTs.count();
        do {
            ++tIndex;
        } while (tStart - fTs[tIndex].fT >= FLT_EPSILON && tIndex < tCount);
        int tIndexStart = tIndex;
        do {
            ++oIndex;
        } while (oStart - other.fTs[oIndex].fT >= FLT_EPSILON && oIndex < oCount);
        int oIndexStart = oIndex;
        double nextT;
        do {
            nextT = fTs[++tIndex].fT;
        } while (nextT < 1 && nextT - tStart < FLT_EPSILON);
        double oNextT;
        do {
            oNextT = other.fTs[++oIndex].fT;
        } while (oNextT < 1 && oNextT - oStart < FLT_EPSILON);
        // at this point, spans before and after are at:
        //  fTs[tIndexStart - 1], fTs[tIndexStart], fTs[tIndex]
        // if tIndexStart == 0, no prior span
        // if nextT == 1, no following span
        
        // advance the span with zero winding
        // if the following span exists (not past the end, non-zero winding)
        // connect the two edges
        if (!fTs[tIndexStart].fWindValue) {
            if (tIndexStart > 0 && fTs[tIndexStart - 1].fWindValue) {
    #if DEBUG_CONCIDENT
                SkDebugf("%s 1 this=%d other=%d t [%d] %1.9g (%1.9g,%1.9g)\n",
                        __FUNCTION__, fID, other.fID, tIndexStart - 1,
                        fTs[tIndexStart].fT, xyAtT(tIndexStart).fX,
                        xyAtT(tIndexStart).fY);
    #endif
                addTPair(fTs[tIndexStart].fT, other, other.fTs[oIndex].fT, false);
            }
            if (nextT < 1 && fTs[tIndex].fWindValue) {
    #if DEBUG_CONCIDENT
                SkDebugf("%s 2 this=%d other=%d t [%d] %1.9g (%1.9g,%1.9g)\n",
                        __FUNCTION__, fID, other.fID, tIndex,
                        fTs[tIndex].fT, xyAtT(tIndex).fX,
                        xyAtT(tIndex).fY);
    #endif
                addTPair(fTs[tIndex].fT, other, other.fTs[oIndexStart].fT, false);
            }
        } else {
            SkASSERT(!other.fTs[oIndexStart].fWindValue);
            if (oIndexStart > 0 && other.fTs[oIndexStart - 1].fWindValue) {
    #if DEBUG_CONCIDENT
                SkDebugf("%s 3 this=%d other=%d t [%d] %1.9g (%1.9g,%1.9g)\n",
                        __FUNCTION__, fID, other.fID, oIndexStart - 1,
                        other.fTs[oIndexStart].fT, other.xyAtT(oIndexStart).fX,
                        other.xyAtT(oIndexStart).fY);
                other.debugAddTPair(other.fTs[oIndexStart].fT, *this, fTs[tIndex].fT);
    #endif
            }
            if (oNextT < 1 && other.fTs[oIndex].fWindValue) {
    #if DEBUG_CONCIDENT
                SkDebugf("%s 4 this=%d other=%d t [%d] %1.9g (%1.9g,%1.9g)\n",
                        __FUNCTION__, fID, other.fID, oIndex,
                        other.fTs[oIndex].fT, other.xyAtT(oIndex).fX,
                        other.xyAtT(oIndex).fY);
                other.debugAddTPair(other.fTs[oIndex].fT, *this, fTs[tIndexStart].fT);
    #endif
            }
        }
    }
            
    void addCoinOutsides(const SkTDArray<double>& outsideTs, Segment& other,
            double oEnd) {
        // walk this to outsideTs[0]
        // walk other to outsideTs[1]
        // if either is > 0, add a pointer to the other, copying adjacent winding
        int tIndex = -1;
        int oIndex = -1;
        double tStart = outsideTs[0];
        double oStart = outsideTs[1];
        do {
            ++tIndex;
        } while (tStart - fTs[tIndex].fT >= FLT_EPSILON);
        do {
            ++oIndex;
        } while (oStart - other.fTs[oIndex].fT >= FLT_EPSILON);
        if (tIndex > 0 || oIndex > 0) {
            addTPair(tStart, other, oStart, false);
        }
        tStart = fTs[tIndex].fT;
        oStart = other.fTs[oIndex].fT;
        do {
            double nextT;
            do {
                nextT = fTs[++tIndex].fT;
            } while (nextT - tStart < FLT_EPSILON);
            tStart = nextT;
            do {
                nextT = other.fTs[++oIndex].fT;
            } while (nextT - oStart < FLT_EPSILON);
            oStart = nextT;
            if (tStart == 1 && oStart == 1) {
                break;
            }
            addTPair(tStart, other, oStart, false);
        } while (tStart < 1 && oStart < 1 && oEnd - oStart >= FLT_EPSILON);
    }
    
    void addCubic(const SkPoint pts[4]) {
        init(pts, SkPath::kCubic_Verb);
        fBounds.setCubicBounds(pts);
    }

    // FIXME: this needs to defer add for aligned consecutive line segments
    SkPoint addCurveTo(int start, int end, SkPath& path, bool active) {
        SkPoint edge[4];
        // OPTIMIZE? if not active, skip remainder and return xy_at_t(end)
        (*SegmentSubDivide[fVerb])(fPts, fTs[start].fT, fTs[end].fT, edge);
        if (active) {
    #if DEBUG_PATH_CONSTRUCTION
            SkDebugf("%s %s (%1.9g,%1.9g)", __FUNCTION__,
                    kLVerbStr[fVerb], edge[1].fX, edge[1].fY);
            if (fVerb > 1) {
                SkDebugf(" (%1.9g,%1.9g)", edge[2].fX, edge[2].fY);
            }
            if (fVerb > 2) {
                SkDebugf(" (%1.9g,%1.9g)", edge[3].fX, edge[3].fY);
            }
            SkDebugf("\n");
    #endif
            switch (fVerb) {
                case SkPath::kLine_Verb:
                    path.lineTo(edge[1].fX, edge[1].fY);
                    break;
                case SkPath::kQuad_Verb:
                    path.quadTo(edge[1].fX, edge[1].fY, edge[2].fX, edge[2].fY);
                    break;
                case SkPath::kCubic_Verb:
                    path.cubicTo(edge[1].fX, edge[1].fY, edge[2].fX, edge[2].fY,
                            edge[3].fX, edge[3].fY);
                    break;
            }
        }
        return edge[fVerb];
    }

    void addLine(const SkPoint pts[2]) {
        init(pts, SkPath::kLine_Verb);
        fBounds.set(pts, 2);
    }

    const SkPoint& addMoveTo(int tIndex, SkPath& path, bool active) {
        const SkPoint& pt = xyAtT(tIndex);
        if (active) {
    #if DEBUG_PATH_CONSTRUCTION
            SkDebugf("%s (%1.9g,%1.9g)\n", __FUNCTION__, pt.fX, pt.fY);
    #endif
            path.moveTo(pt.fX, pt.fY);
        }
        return pt;
    }

    // add 2 to edge or out of range values to get T extremes
    void addOtherT(int index, double otherT, int otherIndex) {
        Span& span = fTs[index];
        span.fOtherT = otherT;
        span.fOtherIndex = otherIndex;
    }

    void addQuad(const SkPoint pts[3]) {
        init(pts, SkPath::kQuad_Verb);
        fBounds.setQuadBounds(pts);
    }
    
    // Defer all coincident edge processing until
    // after normal intersections have been computed

// no need to be tricky; insert in normal T order
// resolve overlapping ts when considering coincidence later

    // add non-coincident intersection. Resulting edges are sorted in T.
    int addT(double newT, Segment* other) {
        // FIXME: in the pathological case where there is a ton of intercepts,
        //  binary search?
        int insertedAt = -1;
        size_t tCount = fTs.count();
        for (size_t index = 0; index < tCount; ++index) {
            // OPTIMIZATION: if there are three or more identical Ts, then
            // the fourth and following could be further insertion-sorted so
            // that all the edges are clockwise or counterclockwise.
            // This could later limit segment tests to the two adjacent
            // neighbors, although it doesn't help with determining which
            // circular direction to go in.
            if (newT < fTs[index].fT) {
                insertedAt = index;
                break;
            }
        }
        Span* span;
        if (insertedAt >= 0) {
            span = fTs.insert(insertedAt);
        } else {
            insertedAt = tCount;
            span = fTs.append();
        }
        span->fT = newT;
        span->fOther = other;
        span->fPt.fX = SK_ScalarNaN;
        span->fWindSum = SK_MinS32;
        span->fWindValue = 1;
        if ((span->fDone = newT == 1)) {
            ++fDoneSpans;
        } 
        return insertedAt;
    }

    // set spans from start to end to decrement by one
    // note this walks other backwards
    // FIMXE: there's probably an edge case that can be constructed where
    // two span in one segment are separated by float epsilon on one span but
    // not the other, if one segment is very small. For this
    // case the counts asserted below may or may not be enough to separate the
    // spans. Even if the counts work out, what if the spans aren't correctly
    // sorted? It feels better in such a case to match the span's other span
    // pointer since both coincident segments must contain the same spans.
    void addTCancel(double startT, double endT, Segment& other,
            double oStartT, double oEndT) {
        SkASSERT(endT - startT >= FLT_EPSILON);
        SkASSERT(oEndT - oStartT >= FLT_EPSILON);
        int index = 0;
        while (startT - fTs[index].fT >= FLT_EPSILON) {
            ++index;
        }
        int oIndex = other.fTs.count();
        while (other.fTs[--oIndex].fT - oEndT > -FLT_EPSILON)
            ;
        double tRatio = (oEndT - oStartT) / (endT - startT);
        Span* test = &fTs[index];
        Span* oTest = &other.fTs[oIndex];
        SkTDArray<double> outsideTs;
        SkTDArray<double> oOutsideTs;
        do {
            bool decrement = test->fWindValue && oTest->fWindValue;
            bool track = test->fWindValue || oTest->fWindValue;
            double testT = test->fT;
            double oTestT = oTest->fT;
            Span* span = test;
            do {
                if (decrement) {
                    decrementSpan(span);
                } else if (track && span->fT < 1 && oTestT < 1) {
                    TrackOutside(outsideTs, span->fT, oTestT);
                }
                span = &fTs[++index];
            } while (span->fT - testT < FLT_EPSILON);
            Span* oSpan = oTest;
            double otherTMatchStart = oEndT - (span->fT - startT) * tRatio;
            double otherTMatchEnd = oEndT - (test->fT - startT) * tRatio;
            SkDEBUGCODE(int originalWindValue = oSpan->fWindValue);
            while (oSpan->fT > otherTMatchStart - FLT_EPSILON
                    && otherTMatchEnd - FLT_EPSILON > oSpan->fT) {
                SkASSERT(originalWindValue == oSpan->fWindValue);
                if (decrement) {
                    other.decrementSpan(oSpan);
                } else if (track && oSpan->fT < 1 && testT < 1) {
                    TrackOutside(oOutsideTs, oSpan->fT, testT);
                }
                if (!oIndex) {
                    break;
                }
                oSpan = &other.fTs[--oIndex];
            } 
            test = span;
            oTest = oSpan;
        } while (test->fT < endT - FLT_EPSILON);
        SkASSERT(!oIndex || oTest->fT < oStartT + FLT_EPSILON);
        // FIXME: determine if canceled edges need outside ts added
        if (!done() && outsideTs.count()) {
            double tStart = outsideTs[0];
            double oStart = outsideTs[1];
            addCancelOutsides(tStart, oStart, other, oEndT);
            int count = outsideTs.count();
            if (count > 2) {
                double tStart = outsideTs[count - 2];
                double oStart = outsideTs[count - 1];
                addCancelOutsides(tStart, oStart, other, oEndT);
            }
        }
        if (!other.done() && oOutsideTs.count()) {
            double tStart = oOutsideTs[0];
            double oStart = oOutsideTs[1];
            other.addCancelOutsides(tStart, oStart, *this, endT);
        }
    }

    // set spans from start to end to increment the greater by one and decrement
    // the lesser
    void addTCoincident(double startT, double endT, Segment& other,
            double oStartT, double oEndT) {
        SkASSERT(endT - startT >= FLT_EPSILON);
        SkASSERT(oEndT - oStartT >= FLT_EPSILON);
        int index = 0;
        while (startT - fTs[index].fT >= FLT_EPSILON) {
            ++index;
        }
        int oIndex = 0;
        while (oStartT - other.fTs[oIndex].fT >= FLT_EPSILON) {
            ++oIndex;
        }
        double tRatio = (oEndT - oStartT) / (endT - startT);
        Span* test = &fTs[index];
        Span* oTest = &other.fTs[oIndex];
        SkTDArray<double> outsideTs;
        SkTDArray<double> xOutsideTs;
        SkTDArray<double> oOutsideTs;
        SkTDArray<double> oxOutsideTs;
        do {
            bool transfer = test->fWindValue && oTest->fWindValue;
            bool decrementOther = test->fWindValue >= oTest->fWindValue;
            Span* end = test;
            double startT = end->fT;
            int startIndex = index;
            double oStartT = oTest->fT;
            int oStartIndex = oIndex;
            do {
                if (transfer) {
                    if (decrementOther) {
                        SkASSERT(abs(end->fWindValue) < gDebugMaxWindValue);
                        ++(end->fWindValue);
                    } else if (decrementSpan(end)) {
                        TrackOutside(outsideTs, end->fT, oStartT);
                    }
                } else if (oTest->fWindValue) {
                    SkASSERT(!decrementOther);
                    if (startIndex > 0 && fTs[startIndex - 1].fWindValue) {
                        TrackOutside(xOutsideTs, end->fT, oStartT);
                    }
                }
                end = &fTs[++index];
            } while (end->fT - test->fT < FLT_EPSILON);
        // because of the order in which coincidences are resolved, this and other
        // may not have the same intermediate points. Compute the corresponding
        // intermediate T values (using this as the master, other as the follower)
        // and walk other conditionally -- hoping that it catches up in the end
            double otherTMatch = (test->fT - startT) * tRatio + oStartT;
            Span* oEnd = oTest;
            while (oEnd->fT < oEndT - FLT_EPSILON && oEnd->fT - otherTMatch < FLT_EPSILON) {
                if (transfer) {
                    if (!decrementOther) {
                        SkASSERT(abs(oEnd->fWindValue) < gDebugMaxWindValue);
                        ++(oEnd->fWindValue);
                    } else if (other.decrementSpan(oEnd)) {
                        TrackOutside(oOutsideTs, oEnd->fT, startT);
                    }
                } else if (test->fWindValue) {
                    SkASSERT(!decrementOther);
                    if (oStartIndex > 0 && other.fTs[oStartIndex - 1].fWindValue) {
                        SkASSERT(0); // track for later?
                    }
                }
                oEnd = &other.fTs[++oIndex];
            }
            test = end;
            oTest = oEnd;
        } while (test->fT < endT - FLT_EPSILON);
        SkASSERT(oTest->fT < oEndT + FLT_EPSILON);
        SkASSERT(oTest->fT > oEndT - FLT_EPSILON);
        if (!done()) {
            if (outsideTs.count()) {
                addCoinOutsides(outsideTs, other, oEndT);
            }
            if (xOutsideTs.count()) {
                addCoinOutsides(xOutsideTs, other, oEndT);
            }
        }
        if (!other.done() && oOutsideTs.count()) {
            other.addCoinOutsides(oOutsideTs, *this, endT);
        }
    }
    
    // FIXME: this doesn't prevent the same span from being added twice
    // fix in caller, assert here?
    void addTPair(double t, Segment& other, double otherT, bool borrowWind) {
        int tCount = fTs.count();
        for (int tIndex = 0; tIndex < tCount; ++tIndex) {
            const Span& span = fTs[tIndex];
            if (span.fT - t >= FLT_EPSILON) {
                break;
            }
            if (span.fT - t < FLT_EPSILON && span.fOther == &other && span.fOtherT == otherT) {
#if DEBUG_ADD_T_PAIR
                SkDebugf("%s addTPair duplicate this=%d %1.9g other=%d %1.9g\n",
                        __FUNCTION__, fID, t, other.fID, otherT);
#endif
                return;
            }
        }
#if DEBUG_ADD_T_PAIR
        SkDebugf("%s addTPair this=%d %1.9g other=%d %1.9g\n",
                __FUNCTION__, fID, t, other.fID, otherT);
#endif
        int insertedAt = addT(t, &other);
        int otherInsertedAt = other.addT(otherT, this);
        addOtherT(insertedAt, otherT, otherInsertedAt);
        other.addOtherT(otherInsertedAt, t, insertedAt);
        matchWindingValue(insertedAt, t, borrowWind);
        other.matchWindingValue(otherInsertedAt, otherT, borrowWind);
    }
    
    void addTwoAngles(int start, int end, SkTDArray<Angle>& angles) const {
        // add edge leading into junction
        if (fTs[SkMin32(end, start)].fWindValue > 0) {
            addAngle(angles, end, start);
        }
        // add edge leading away from junction
        int step = SkSign32(end - start);
        int tIndex = nextSpan(end, step);
        if (tIndex >= 0 && fTs[SkMin32(end, tIndex)].fWindValue > 0) {
            addAngle(angles, end, tIndex);
        }
    }
    
    const Bounds& bounds() const {
        return fBounds;
    }

    void buildAngles(int index, SkTDArray<Angle>& angles) const {
        double referenceT = fTs[index].fT;
        int lesser = index;
        while (--lesser >= 0 && referenceT - fTs[lesser].fT < FLT_EPSILON) {
            buildAnglesInner(lesser, angles);
        }
        do {
            buildAnglesInner(index, angles);
        } while (++index < fTs.count() && fTs[index].fT - referenceT < FLT_EPSILON);
    }

    void buildAnglesInner(int index, SkTDArray<Angle>& angles) const {
        Span* span = &fTs[index];
        Segment* other = span->fOther;
    // if there is only one live crossing, and no coincidence, continue
    // in the same direction
    // if there is coincidence, the only choice may be to reverse direction
        // find edge on either side of intersection
        int oIndex = span->fOtherIndex;
        // if done == -1, prior span has already been processed
        int step = 1;
        int next = other->nextSpan(oIndex, step);
        if (next < 0) {
            step = -step;
            next = other->nextSpan(oIndex, step);
        }
        // add candidate into and away from junction
        other->addTwoAngles(next, oIndex, angles);
    }

    bool cancels(const Segment& other) const {
        SkASSERT(fVerb == SkPath::kLine_Verb);
        SkASSERT(other.fVerb == SkPath::kLine_Verb);
        SkPoint dxy = fPts[0] - fPts[1];
        SkPoint odxy = other.fPts[0] - other.fPts[1];
        return dxy.fX * odxy.fX < 0 || dxy.fY * odxy.fY < 0;
    }

    // figure out if the segment's ascending T goes clockwise or not
    // not enough context to write this as shown
    // instead, add all segments meeting at the top
    // sort them using buildAngleList
    // find the first in the sort
    // see if ascendingT goes to top
    bool clockwise(int /* tIndex */) const {
        SkASSERT(0); // incomplete
        return false;
    }
    
    int computeSum(int startIndex, int endIndex) {
        SkTDArray<Angle> angles;
        addTwoAngles(startIndex, endIndex, angles);
        buildAngles(endIndex, angles);
        SkTDArray<Angle*> sorted;
        sortAngles(angles, sorted);
        int angleCount = angles.count();
        const Angle* angle;
        const Segment* base;
        int winding;
        int firstIndex = 0;
        do {
            angle = sorted[firstIndex];
            base = angle->segment();
            winding = base->windSum(angle);
            if (winding != SK_MinS32) {
                break;
            }
            if (++firstIndex == angleCount) {
                return SK_MinS32;
            }
        } while (true);
        // turn winding into contourWinding
        int spanWinding = base->spanSign(angle);
        bool inner = useInnerWinding(winding + spanWinding, winding);
    #if DEBUG_WINDING
        SkDebugf("%s --- spanWinding=%d winding=%d sign=%d inner=%d result=%d\n", __FUNCTION__,
            spanWinding, winding, angle->sign(), inner,
            inner ? winding + spanWinding : winding);
    #endif
        if (inner) {
            winding += spanWinding;
        }
    #if DEBUG_SORT
        base->debugShowSort(sorted, firstIndex, winding);
    #endif
        int nextIndex = firstIndex + 1;
        int lastIndex = firstIndex != 0 ? firstIndex : angleCount;
        winding -= base->spanSign(angle);
        do {
            if (nextIndex == angleCount) {
                nextIndex = 0;
            }
            angle = sorted[nextIndex];
            Segment* segment = angle->segment();
            int maxWinding = winding;
            winding -= segment->spanSign(angle);
            if (segment->windSum(angle) == SK_MinS32) {
                if (useInnerWinding(maxWinding, winding)) {
                    maxWinding = winding;
                }
                segment->markAndChaseWinding(angle, maxWinding);
            }
        } while (++nextIndex != lastIndex);
        return windSum(SkMin32(startIndex, endIndex));
    }

    int crossedSpan(const SkPoint& basePt, SkScalar& bestY, double& hitT) const {
        int bestT = -1;
        SkScalar top = bounds().fTop;
        SkScalar bottom = bounds().fBottom;
        int end = 0;
        do {
            int start = end;
            end = nextSpan(start, 1);
            if (fTs[start].fWindValue == 0) {
                continue;
            }
            SkPoint edge[4];
            // OPTIMIZE: wrap this so that if start==0 end==fTCount-1 we can 
            // work with the original data directly
            (*SegmentSubDivide[fVerb])(fPts, fTs[start].fT, fTs[end].fT, edge);
            // intersect ray starting at basePt with edge
            Intersections intersections;
            int pts = (*VSegmentIntersect[fVerb])(edge, top, bottom, basePt.fX,
                    false, intersections);
            if (pts == 0) {
                continue;
            }
            if (pts > 1 && fVerb == SkPath::kLine_Verb) {
            // if the intersection is edge on, wait for another one
                continue;
            }
            SkASSERT(pts == 1); // FIXME: more code required to disambiguate
            SkPoint pt;
            double foundT = intersections.fT[0][0];
            (*SegmentXYAtT[fVerb])(fPts, foundT, &pt);
            if (bestY < pt.fY && pt.fY < basePt.fY) {
                bestY = pt.fY;
                bestT = foundT < 1 ? start : end;
                hitT = fTs[start].fT + (fTs[end].fT - fTs[start].fT) * foundT;
            }
        } while (fTs[end].fT != 1);
        return bestT;
    }

    bool crossedSpanHalves(const SkPoint& basePt, bool leftHalf, bool rightHalf) {
        // if a segment is connected to this one, consider it crossing
        int tIndex;
        if (fPts[0].fX == basePt.fX) {
            tIndex = 0;
            do {
                const Span& sSpan = fTs[tIndex];
                const Segment* sOther = sSpan.fOther;
                if (!sOther->fTs[sSpan.fOtherIndex].fWindValue) {
                    continue;
                }
                if (leftHalf ? sOther->fBounds.fLeft < basePt.fX
                        : sOther->fBounds.fRight > basePt.fX) {
                    return true;
                }
            } while (fTs[++tIndex].fT == 0);
        }
        if (fPts[fVerb].fX == basePt.fX) {
            tIndex = fTs.count() - 1;
            do {
                const Span& eSpan = fTs[tIndex];
                const Segment* eOther = eSpan.fOther;
                if (!eOther->fTs[eSpan.fOtherIndex].fWindValue) {
                    continue;
                }
                if (leftHalf ? eOther->fBounds.fLeft < basePt.fX
                        : eOther->fBounds.fRight > basePt.fX) {
                    return true;
                }
            } while (fTs[--tIndex].fT == 1);
        }
        return false;
    }
    
    bool decrementSpan(Span* span) {
        SkASSERT(span->fWindValue > 0);
        if (--(span->fWindValue) == 0) {
            span->fDone = true;
            ++fDoneSpans;
            return true;
        }
        return false;
    }

    bool done() const {
        SkASSERT(fDoneSpans <= fTs.count());
        return fDoneSpans == fTs.count();
    }

    bool done(const Angle& angle) const {
        int start = angle.start();
        int end = angle.end();
        const Span& mSpan = fTs[SkMin32(start, end)];
        return mSpan.fDone;
    }

    // so the span needs to contain the pairing info found here
    // this should include the winding computed for the edge, and
    //  what edge it connects to, and whether it is discarded
    //  (maybe discarded == abs(winding) > 1) ?
    // only need derivatives for duration of sorting, add a new struct
    // for pairings, remove extra spans that have zero length and
    //  reference an unused other
    // for coincident, the last span on the other may be marked done
    //  (always?)
    
    // if loop is exhausted, contour may be closed.
    // FIXME: pass in close point so we can check for closure

    // given a segment, and a sense of where 'inside' is, return the next
    // segment. If this segment has an intersection, or ends in multiple
    // segments, find the mate that continues the outside.
    // note that if there are multiples, but no coincidence, we can limit
    // choices to connections in the correct direction
    
    // mark found segments as done

    // start is the index of the beginning T of this edge
    // it is guaranteed to have an end which describes a non-zero length (?)
    // winding -1 means ccw, 1 means cw
    // firstFind allows coincident edges to be treated differently
    Segment* findNext(SkTDArray<Span*>& chase, bool firstFind, bool active,
            const int startIndex, const int endIndex, int& nextStart,
            int& nextEnd, int& winding, int& spanWinding) {
        int outerWinding = winding;
        int innerWinding = winding + spanWinding;
    #if DEBUG_WINDING
        SkDebugf("%s winding=%d spanWinding=%d outerWinding=%d innerWinding=%d\n",
                __FUNCTION__, winding, spanWinding, outerWinding, innerWinding);
    #endif
        if (useInnerWinding(outerWinding, innerWinding)) {
            outerWinding = innerWinding;
        }
        SkASSERT(startIndex != endIndex);
        int count = fTs.count();
        SkASSERT(startIndex < endIndex ? startIndex < count - 1
                : startIndex > 0);
        int step = SkSign32(endIndex - startIndex);
        int end = nextSpan(startIndex, step);
        SkASSERT(end >= 0);
        Span* endSpan = &fTs[end];
        Segment* other;
        if (isSimple(end)) {
        // mark the smaller of startIndex, endIndex done, and all adjacent
        // spans with the same T value (but not 'other' spans)
    #if DEBUG_WINDING
            SkDebugf("%s simple\n", __FUNCTION__);
    #endif
            markDone(SkMin32(startIndex, endIndex), outerWinding);
            other = endSpan->fOther;
            nextStart = endSpan->fOtherIndex;
            double startT = other->fTs[nextStart].fT;
            nextEnd = nextStart;
            do {
                nextEnd += step;
            } while (fabs(startT - other->fTs[nextEnd].fT) < FLT_EPSILON);
            SkASSERT(step < 0 ? nextEnd >= 0 : nextEnd < other->fTs.count());
            return other;
        }
        // more than one viable candidate -- measure angles to find best
        SkTDArray<Angle> angles;
        SkASSERT(startIndex - endIndex != 0);
        SkASSERT((startIndex - endIndex < 0) ^ (step < 0));
        addTwoAngles(startIndex, end, angles);
        buildAngles(end, angles);
        SkTDArray<Angle*> sorted;
        sortAngles(angles, sorted);
        int angleCount = angles.count();
        int firstIndex = findStartingEdge(sorted, startIndex, end);
        SkASSERT(firstIndex >= 0);
    #if DEBUG_SORT
        debugShowSort(sorted, firstIndex, winding);
    #endif
        SkASSERT(sorted[firstIndex]->segment() == this);
    #if DEBUG_WINDING
        SkDebugf("%s sign=%d\n", __FUNCTION__, sorted[firstIndex]->sign());
    #endif
        int sumWinding = winding - spanSign(sorted[firstIndex]);
        int nextIndex = firstIndex + 1;
        int lastIndex = firstIndex != 0 ? firstIndex : angleCount;
        const Angle* foundAngle = NULL;
        bool foundDone = false;
        // iterate through the angle, and compute everyone's winding
        int toggleWinding = SK_MinS32;
        bool flipFound = false;
        int flipped = 1;
        Segment* nextSegment;
        do {
            if (nextIndex == angleCount) {
                nextIndex = 0;
            }
            const Angle* nextAngle = sorted[nextIndex];
            int maxWinding = sumWinding;
            nextSegment = nextAngle->segment();
            sumWinding -= nextSegment->spanSign(nextAngle);
            SkASSERT(abs(sumWinding) <= gDebugMaxWindSum);
    #if DEBUG_WINDING
            SkDebugf("%s maxWinding=%d sumWinding=%d sign=%d\n", __FUNCTION__,
                    maxWinding, sumWinding, nextAngle->sign());
    #endif
            if (maxWinding * sumWinding < 0) {
                flipFound ^= true;
    #if DEBUG_WINDING
                SkDebugf("%s flipFound=%d maxWinding=%d sumWinding=%d\n",
                        __FUNCTION__, flipFound, maxWinding, sumWinding);
    #endif
            }
            if (!sumWinding) {
                if (!active) {
                    markDone(SkMin32(startIndex, endIndex), outerWinding);
                    // FIXME: seems like a bug that this isn't calling userInnerWinding
                    nextSegment->markWinding(SkMin32(nextAngle->start(),
                                nextAngle->end()), maxWinding);
    #if DEBUG_WINDING
                    SkDebugf("%s inactive\n", __FUNCTION__);
    #endif
                    return NULL;
                }
                if (!foundAngle || foundDone) {
                    foundAngle = nextAngle;
                    foundDone = nextSegment->done(*nextAngle);
                    if (flipFound || (maxWinding * outerWinding < 0)) {
                        flipped = -flipped;
            #if DEBUG_WINDING
                        SkDebugf("%s flipped=%d flipFound=%d maxWinding=%d"
                                " outerWinding=%d\n", __FUNCTION__, flipped,
                                flipFound, maxWinding, outerWinding);
            #endif
                    }
                }
                continue;
            }
            if (!maxWinding && !foundAngle) {
        #if DEBUG_WINDING
                if (flipped > 0) {
                    SkDebugf("%s sumWinding=%d * outerWinding=%d < 0 (%s)\n",
                            __FUNCTION__, sumWinding, outerWinding, 
                            sumWinding * outerWinding < 0 ? "true" : "false");
                }
        #endif
                if (sumWinding * outerWinding < 0 && flipped > 0) {
        #if DEBUG_WINDING
                    SkDebugf("%s toggleWinding=%d\n", __FUNCTION__, sumWinding);
        #endif
                    toggleWinding = sumWinding;
                } else if (outerWinding != sumWinding) {
        #if DEBUG_WINDING
                    SkDebugf("%s outerWinding=%d != sumWinding=%d winding=%d\n",
                            __FUNCTION__, outerWinding, sumWinding, winding);
        #endif
                    winding = sumWinding;
                }
                foundAngle = nextAngle;
                if (flipFound) {
                    flipped = -flipped;
        #if DEBUG_WINDING
                    SkDebugf("%s flipped flipFound=%d\n", __FUNCTION__, flipFound);
        #endif
                }
            }
            if (nextSegment->done()) {
                continue;
            }
            // if the winding is non-zero, nextAngle does not connect to
            // current chain. If we haven't done so already, mark the angle
            // as done, record the winding value, and mark connected unambiguous
            // segments as well.
            if (nextSegment->windSum(nextAngle) == SK_MinS32) {
                if (useInnerWinding(maxWinding, sumWinding)) {
                    maxWinding = sumWinding;
                }
                Span* last;
                if (foundAngle) {
                    last = nextSegment->markAndChaseWinding(nextAngle, maxWinding);
                } else {
                    last = nextSegment->markAndChaseDone(nextAngle, maxWinding);
                }
                if (last) {
                    *chase.append() = last;
                }
            }
        } while (++nextIndex != lastIndex);
        SkASSERT(sorted[firstIndex]->segment() == this);
        markDone(SkMin32(startIndex, endIndex), outerWinding);
        if (!foundAngle) {
            return NULL;
        }
        nextStart = foundAngle->start();
        nextEnd = foundAngle->end();
        nextSegment = foundAngle->segment();
        spanWinding = SkSign32(spanWinding) * flipped * nextSegment->windValue(
                SkMin32(nextStart, nextEnd));
        if (toggleWinding != SK_MinS32) {
            winding = toggleWinding;
            spanWinding = -spanWinding;
        }
    #if DEBUG_WINDING
        SkDebugf("%s spanWinding=%d\n", __FUNCTION__, spanWinding);
    #endif
        return nextSegment;
    }

    int findStartingEdge(SkTDArray<Angle*>& sorted, int start, int end) {
        int angleCount = sorted.count();
        int firstIndex = -1;
        for (int angleIndex = 0; angleIndex < angleCount; ++angleIndex) {
            const Angle* angle = sorted[angleIndex];
            if (angle->segment() == this && angle->start() == end &&
                    angle->end() == start) {
                firstIndex = angleIndex;
                break;
            }
        }
        return firstIndex;
    }

    // FIXME: this is tricky code; needs its own unit test
    void findTooCloseToCall(int /* winding */ ) { // FIXME: winding should be considered
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
            if (moCount >= 3) {
                break;
            }
            if (++matchIndex >= count) {
                return;
            }
        } while (true); // require t=0, x, 1 at minimum
        // OPTIMIZATION: defer matchPt until qualifying toCount is found?
        const SkPoint* matchPt = &xyAtT(match);
        // look for a pair of nearby T values that map to the same (x,y) value
        // if found, see if the pair of other segments share a common point. If
        // so, the span from here to there is coincident.
        for (int index = matchIndex + 1; index < count; ++index) {
            Span* test = &fTs[index];
            if (test->fDone) {
                continue;
            }
            Segment* tOther = test->fOther;
            int toCount = tOther->fTs.count();
            if (toCount < 3) { // require t=0, x, 1 at minimum
                continue;
            }
            const SkPoint* testPt = &xyAtT(test);
            if (*matchPt != *testPt) {
                matchIndex = index;
                moCount = toCount;
                match = test;
                mOther = tOther;
                matchPt = testPt;
                continue;
            }
            int moStart = -1;
            int moEnd = -1;
            double moStartT, moEndT;
            for (int moIndex = 0; moIndex < moCount; ++moIndex) {
                Span& moSpan = mOther->fTs[moIndex];
                if (moSpan.fDone) {
                    continue;
                }
                if (moSpan.fOther == this) {
                    if (moSpan.fOtherT == match->fT) {
                        moStart = moIndex;
                        moStartT = moSpan.fT;
                    }
                    continue;
                }
                if (moSpan.fOther == tOther) {
                    SkASSERT(moEnd == -1);
                    moEnd = moIndex;
                    moEndT = moSpan.fT;
                }
            }
            if (moStart < 0 || moEnd < 0) {
                continue;
            }
            // FIXME: if moStartT, moEndT are initialized to NaN, can skip this test
            if (moStartT == moEndT) {
                continue;
            }
            int toStart = -1;
            int toEnd = -1;
            double toStartT, toEndT;
            for (int toIndex = 0; toIndex < toCount; ++toIndex) {
                Span& toSpan = tOther->fTs[toIndex];
                if (toSpan.fOther == this) {
                    if (toSpan.fOtherT == test->fT) {
                        toStart = toIndex;
                        toStartT = toSpan.fT;
                    }
                    continue;
                }
                if (toSpan.fOther == mOther && toSpan.fOtherT == moEndT) {
                    SkASSERT(toEnd == -1);
                    toEnd = toIndex;
                    toEndT = toSpan.fT;
                }
            }
            // FIXME: if toStartT, toEndT are initialized to NaN, can skip this test
            if (toStart <= 0 || toEnd <= 0) {
                continue;
            }
            if (toStartT == toEndT) {
                continue;
            }
            // test to see if the segment between there and here is linear
            if (!mOther->isLinear(moStart, moEnd)
                    || !tOther->isLinear(toStart, toEnd)) {
                continue;
            }
            // FIXME: defer implementation until the rest works
            // this may share code with regular coincident detection
            SkASSERT(0);
        #if 0
            if (flipped) {
                mOther->addTCancel(moStart, moEnd, tOther, tStart, tEnd);
            } else {
                mOther->addTCoincident(moStart, moEnd, tOther, tStart, tEnd);
            }
        #endif
        }
    }

    // OPTIMIZATION : for a pair of lines, can we compute points at T (cached)
    // and use more concise logic like the old edge walker code?
    // FIXME: this needs to deal with coincident edges
    Segment* findTop(int& tIndex, int& endIndex) {
        // iterate through T intersections and return topmost
        // topmost tangent from y-min to first pt is closer to horizontal
        SkASSERT(!done());
        int firstT;
        int lastT;
        SkPoint topPt;
        topPt.fY = SK_ScalarMax;
        int count = fTs.count();
        // see if either end is not done since we want smaller Y of the pair
        bool lastDone = true;
        for (int index = 0; index < count; ++index) {
            const Span& span = fTs[index];
            if (!span.fDone || !lastDone) {
                const SkPoint& intercept = xyAtT(&span);
                if (topPt.fY > intercept.fY || (topPt.fY == intercept.fY
                        && topPt.fX > intercept.fX)) {
                    topPt = intercept;
                    firstT = lastT = index;
                } else if (topPt == intercept) {
                    lastT = index;
                }
            }
            lastDone = span.fDone;
        }
        // sort the edges to find the leftmost
        int step = 1;
        int end = nextSpan(firstT, step);
        if (end == -1) {
            step = -1;
            end = nextSpan(firstT, step);
            SkASSERT(end != -1);
        }
        // if the topmost T is not on end, or is three-way or more, find left
        // look for left-ness from tLeft to firstT (matching y of other)
        SkTDArray<Angle> angles;
        SkASSERT(firstT - end != 0);
        addTwoAngles(end, firstT, angles);
        buildAngles(firstT, angles);
        SkTDArray<Angle*> sorted;
        sortAngles(angles, sorted);
        // skip edges that have already been processed
        firstT = -1;
        Segment* leftSegment;
        do {
            const Angle* angle = sorted[++firstT];
            leftSegment = angle->segment();
            tIndex = angle->end();
            endIndex = angle->start();
        } while (leftSegment->fTs[SkMin32(tIndex, endIndex)].fDone);
        return leftSegment;
    }
    
    // FIXME: not crazy about this
    // when the intersections are performed, the other index is into an
    // incomplete array. as the array grows, the indices become incorrect
    // while the following fixes the indices up again, it isn't smart about
    // skipping segments whose indices are already correct
    // assuming we leave the code that wrote the index in the first place
    void fixOtherTIndex() {
        int iCount = fTs.count();
        for (int i = 0; i < iCount; ++i) {
            Span& iSpan = fTs[i];
            double oT = iSpan.fOtherT;
            Segment* other = iSpan.fOther;
            int oCount = other->fTs.count();
            for (int o = 0; o < oCount; ++o) {
                Span& oSpan = other->fTs[o];
                if (oT == oSpan.fT && this == oSpan.fOther) {
                    iSpan.fOtherIndex = o;
                    break;
                }
            }
        }
    }
    
    // OPTIMIZATION: uses tail recursion. Unwise?
    Span* innerChaseDone(int index, int step, int winding) {
        int end = nextSpan(index, step);
        SkASSERT(end >= 0);
        if (multipleSpans(end)) {
            return &fTs[end];
        }
        const Span& endSpan = fTs[end];
        Segment* other = endSpan.fOther;
        index = endSpan.fOtherIndex;
        int otherEnd = other->nextSpan(index, step);
        Span* last = other->innerChaseDone(index, step, winding);
        other->markDone(SkMin32(index, otherEnd), winding);
        return last;
    }
    
    Span* innerChaseWinding(int index, int step, int winding) {
        int end = nextSpan(index, step);
        SkASSERT(end >= 0);
        if (multipleSpans(end)) {
            return &fTs[end];
        }
        const Span& endSpan = fTs[end];
        Segment* other = endSpan.fOther;
        index = endSpan.fOtherIndex;
        int otherEnd = other->nextSpan(index, step);
        int min = SkMin32(index, otherEnd);
        if (other->fTs[min].fWindSum != SK_MinS32) {
            SkASSERT(other->fTs[min].fWindSum == winding);
            return NULL;
        }
        Span* last = other->innerChaseWinding(index, step, winding);
        other->markWinding(min, winding);
        return last;
    }
    
    void init(const SkPoint pts[], SkPath::Verb verb) {
        fPts = pts;
        fVerb = verb;
        fDoneSpans = 0;
    }

    bool intersected() const {
        return fTs.count() > 0;
    }

    bool isConnected(int startIndex, int endIndex) const {
        return fTs[startIndex].fWindSum != SK_MinS32
                || fTs[endIndex].fWindSum != SK_MinS32;
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

    // OPTIMIZE: successive calls could start were the last leaves off
    // or calls could specialize to walk forwards or backwards
    bool isMissing(double startT) const {
        size_t tCount = fTs.count();
        for (size_t index = 0; index < tCount; ++index) {
            if (fabs(startT - fTs[index].fT) < FLT_EPSILON) {
                return false;
            }
        }
        return true;
    }

    bool isSimple(int end) const {
        int count = fTs.count();
        if (count == 2) {
            return true;
        }
        double t = fTs[end].fT;
        if (t < FLT_EPSILON) {
            return fTs[1].fT >= FLT_EPSILON;
        }
        if (t > 1 - FLT_EPSILON) {
            return fTs[count - 2].fT <= 1 - FLT_EPSILON;
        }
        return false;
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
    
    // this span is excluded by the winding rule -- chase the ends
    // as long as they are unambiguous to mark connections as done
    // and give them the same winding value
    Span* markAndChaseDone(const Angle* angle, int winding) {
        int index = angle->start();
        int endIndex = angle->end();
        int step = SkSign32(endIndex - index);
        Span* last = innerChaseDone(index, step, winding);
        markDone(SkMin32(index, endIndex), winding);
        return last;
    }
    
    Span* markAndChaseWinding(const Angle* angle, int winding) {
        int index = angle->start();
        int endIndex = angle->end();
        int min = SkMin32(index, endIndex);
        int step = SkSign32(endIndex - index);
        Span* last = innerChaseWinding(index, step, winding);
        markWinding(min, winding);
        return last;
    }
    
    // FIXME: this should also mark spans with equal (x,y)
    // This may be called when the segment is already marked done. While this
    // wastes time, it shouldn't do any more than spin through the T spans.
    // OPTIMIZATION: abort on first done found (assuming that this code is 
    // always called to mark segments done).
    void markDone(int index, int winding) {
      //  SkASSERT(!done());
        double referenceT = fTs[index].fT;
        int lesser = index;
        while (--lesser >= 0 && referenceT - fTs[lesser].fT < FLT_EPSILON) {
            Span& span = fTs[lesser];
            if (span.fDone) {
                continue;
            }
        #if DEBUG_MARK_DONE
            debugShowNewWinding(__FUNCTION__, span, winding);
        #endif
            span.fDone = true;
            SkASSERT(span.fWindSum == SK_MinS32 || span.fWindSum == winding);
            SkASSERT(abs(winding) <= gDebugMaxWindSum);
            span.fWindSum = winding;
            fDoneSpans++;
        }
        do {
            Span& span = fTs[index];
     //       SkASSERT(!span.fDone);
            if (span.fDone) {
                continue;
            }
        #if DEBUG_MARK_DONE
            debugShowNewWinding(__FUNCTION__, span, winding);
        #endif
            span.fDone = true;
            SkASSERT(span.fWindSum == SK_MinS32 || span.fWindSum == winding);
            SkASSERT(abs(winding) <= gDebugMaxWindSum);
            span.fWindSum = winding;
            fDoneSpans++;
        } while (++index < fTs.count() && fTs[index].fT - referenceT < FLT_EPSILON);
    }

    void markWinding(int index, int winding) {
    //    SkASSERT(!done());
        double referenceT = fTs[index].fT;
        int lesser = index;
        while (--lesser >= 0 && referenceT - fTs[lesser].fT < FLT_EPSILON) {
            Span& span = fTs[lesser];
            if (span.fDone) {
                continue;
            }
      //      SkASSERT(span.fWindValue == 1 || winding == 0);
        #if DEBUG_MARK_DONE
            debugShowNewWinding(__FUNCTION__, span, winding);
        #endif
            SkASSERT(span.fWindSum == SK_MinS32 || span.fWindSum == winding);
            SkASSERT(abs(winding) <= gDebugMaxWindSum);
            span.fWindSum = winding;
        }
        do {
            Span& span = fTs[index];
     //       SkASSERT(!span.fDone || span.fCoincident);
            if (span.fDone) {
                continue;
            }
     //       SkASSERT(span.fWindValue == 1 || winding == 0);
            SkASSERT(span.fWindSum == SK_MinS32 || span.fWindSum == winding);
        #if DEBUG_MARK_DONE
            debugShowNewWinding(__FUNCTION__, span, winding);
        #endif
            SkASSERT(abs(winding) <= gDebugMaxWindSum);
            span.fWindSum = winding;
        } while (++index < fTs.count() && fTs[index].fT - referenceT < FLT_EPSILON);
    }

    void matchWindingValue(int tIndex, double t, bool borrowWind) {
        int nextDoorWind = SK_MaxS32;
        if (tIndex > 0) {
            const Span& below = fTs[tIndex - 1];
            if (t - below.fT < FLT_EPSILON) {
                nextDoorWind = below.fWindValue;
            }
        }
        if (nextDoorWind == SK_MaxS32 && tIndex + 1 < fTs.count()) {
            const Span& above = fTs[tIndex + 1];
            if (above.fT - t < FLT_EPSILON) {
                nextDoorWind = above.fWindValue;
            }
        }
        if (nextDoorWind == SK_MaxS32 && borrowWind && tIndex > 0 && t < 1) {
            const Span& below = fTs[tIndex - 1];
            nextDoorWind = below.fWindValue;
        }
        if (nextDoorWind != SK_MaxS32) {
            Span& newSpan = fTs[tIndex];
            newSpan.fWindValue = nextDoorWind;
            if (!nextDoorWind) {
                newSpan.fDone = true;
                ++fDoneSpans;
            }
        }
    }

    // return span if when chasing, two or more radiating spans are not done
    // OPTIMIZATION: ? multiple spans is detected when there is only one valid
    // candidate and the remaining spans have windValue == 0 (canceled by
    // coincidence). The coincident edges could either be removed altogether,
    // or this code could be more complicated in detecting this case. Worth it?
    bool multipleSpans(int end) const {
        return end > 0 && end < fTs.count() - 1;
    }

    // This has callers for two different situations: one establishes the end
    // of the current span, and one establishes the beginning of the next span
    // (thus the name). When this is looking for the end of the current span,
    // coincidence is found when the beginning Ts contain -step and the end
    // contains step. When it is looking for the beginning of the next, the
    // first Ts found can be ignored and the last Ts should contain -step.
    // OPTIMIZATION: probably should split into two functions
    int nextSpan(int from, int step) const {
        const Span& fromSpan = fTs[from];
        int count = fTs.count();
        int to = from;
        while (step > 0 ? ++to < count : --to >= 0) {
            const Span& span = fTs[to];
            if ((step > 0 ? span.fT - fromSpan.fT : fromSpan.fT - span.fT) < FLT_EPSILON) {
                continue;
            }
            return to;
        }
        return -1;
    }

    const SkPoint* pts() const {
        return fPts;
    }

    void reset() {
        init(NULL, (SkPath::Verb) -1);
        fBounds.set(SK_ScalarMax, SK_ScalarMax, SK_ScalarMax, SK_ScalarMax);
        fTs.reset();
    }

    // OPTIMIZATION: mark as debugging only if used solely by tests
    const Span& span(int tIndex) const {
        return fTs[tIndex];
    }
    
    int spanSign(int startIndex, int endIndex) const {
        int result = startIndex < endIndex ? -fTs[startIndex].fWindValue :
                fTs[endIndex].fWindValue;
#if DEBUG_WIND_BUMP
        SkDebugf("%s spanSign=%d\n", __FUNCTION__, result);
#endif
        return result;
    }

    int spanSign(const Angle* angle) const {
        SkASSERT(angle->segment() == this);
        return spanSign(angle->start(), angle->end());
    }

    // OPTIMIZATION: mark as debugging only if used solely by tests
    double t(int tIndex) const {
        return fTs[tIndex].fT;
    }

    static void TrackOutside(SkTDArray<double>& outsideTs, double end,
            double start) {
        int outCount = outsideTs.count();
        if (outCount == 0 || end - outsideTs[outCount - 2] >= FLT_EPSILON) {
            *outsideTs.append() = end;
            *outsideTs.append() = start;
        }
    }

    void updatePts(const SkPoint pts[]) {
        fPts = pts;
    }

    SkPath::Verb verb() const {
        return fVerb;
    }

    int windSum(int tIndex) const {
        return fTs[tIndex].fWindSum;
    }
    
    int windSum(const Angle* angle) const {
        int start = angle->start();
        int end = angle->end();
        int index = SkMin32(start, end);
        return windSum(index);
    }

    int windValue(int tIndex) const {
        return fTs[tIndex].fWindValue;
    }
    
    int windValue(const Angle* angle) const {
        int start = angle->start();
        int end = angle->end();
        int index = SkMin32(start, end);
        return windValue(index);
    }

    SkScalar xAtT(const Span* span) const {
        return xyAtT(span).fX;
    }

    const SkPoint& xyAtT(int index) const {
        return xyAtT(&fTs[index]);
    }

    const SkPoint& xyAtT(const Span* span) const {
        if (SkScalarIsNaN(span->fPt.fX)) {
            if (span->fT == 0) {
                span->fPt = fPts[0];
            } else if (span->fT == 1) {
                span->fPt = fPts[fVerb];
            } else {
                (*SegmentXYAtT[fVerb])(fPts, span->fT, &span->fPt);
            }
        }
        return span->fPt;
    }
    
    SkScalar yAtT(int index) const {
        return yAtT(&fTs[index]);
    }

    SkScalar yAtT(const Span* span) const {
        return xyAtT(span).fY;
    }

#if DEBUG_DUMP
    void dump() const {
        const char className[] = "Segment";
        const int tab = 4;
        for (int i = 0; i < fTs.count(); ++i) {
            SkPoint out;
            (*SegmentXYAtT[fVerb])(fPts, t(i), &out);
            SkDebugf("%*s [%d] %s.fTs[%d]=%1.9g (%1.9g,%1.9g) other=%d"
                    " otherT=%1.9g windSum=%d\n",
                    tab + sizeof(className), className, fID,
                    kLVerbStr[fVerb], i, fTs[i].fT, out.fX, out.fY,
                    fTs[i].fOther->fID, fTs[i].fOtherT, fTs[i].fWindSum);
        }
        SkDebugf("%*s [%d] fBounds=(l:%1.9g, t:%1.9g r:%1.9g, b:%1.9g)",
                tab + sizeof(className), className, fID,
                fBounds.fLeft, fBounds.fTop, fBounds.fRight, fBounds.fBottom);
    }
#endif

#if DEBUG_CONCIDENT
    // assert if pair has not already been added
     void debugAddTPair(double t, const Segment& other, double otherT) const {
        for (int i = 0; i < fTs.count(); ++i) {
            if (fTs[i].fT == t && fTs[i].fOther == &other && fTs[i].fOtherT == otherT) {
                return;
            }
        }
        SkASSERT(0);
     }
#endif

#if DEBUG_DUMP
    int debugID() const {
        return fID;
    }
#endif

#if DEBUG_CONCIDENT
    void debugShowTs() const {
        SkDebugf("%s %d", __FUNCTION__, fID);
        for (int i = 0; i < fTs.count(); ++i) {
            SkDebugf(" [o=%d t=%1.3g %1.9g,%1.9g w=%d]", fTs[i].fOther->fID,
                    fTs[i].fT, xAtT(&fTs[i]), yAtT(&fTs[i]), fTs[i].fWindValue);
        }
        SkDebugf("\n");
    }
#endif

#if DEBUG_ACTIVE_SPANS
    void debugShowActiveSpans() const {
        if (done()) {
            return;
        }
        for (int i = 0; i < fTs.count(); ++i) {
            if (fTs[i].fDone) {
                continue;
            }
            SkDebugf("%s id=%d", __FUNCTION__, fID);
            SkDebugf(" (%1.9g,%1.9g", fPts[0].fX, fPts[0].fY);
            for (int vIndex = 1; vIndex <= fVerb; ++vIndex) {
                SkDebugf(" %1.9g,%1.9g", fPts[vIndex].fX, fPts[vIndex].fY);
            }
            const Span* span = &fTs[i];
            SkDebugf(") t=%1.9g (%1.9g,%1.9g)", fTs[i].fT, 
                     xAtT(span), yAtT(span));
            const Segment* other = fTs[i].fOther;
            SkDebugf(" other=%d otherT=%1.9g otherIndex=%d windSum=",
                    other->fID, fTs[i].fOtherT, fTs[i].fOtherIndex);
            if (fTs[i].fWindSum == SK_MinS32) {
                SkDebugf("?");
            } else {
                SkDebugf("%d", fTs[i].fWindSum);
            }
            SkDebugf(" windValue=%d\n", fTs[i].fWindValue);
        }
    }
#endif

#if DEBUG_MARK_DONE
    void debugShowNewWinding(const char* fun, const Span& span, int winding) {
        const SkPoint& pt = xyAtT(&span);
        SkDebugf("%s id=%d", fun, fID);
        SkDebugf(" (%1.9g,%1.9g", fPts[0].fX, fPts[0].fY);
        for (int vIndex = 1; vIndex <= fVerb; ++vIndex) {
            SkDebugf(" %1.9g,%1.9g", fPts[vIndex].fX, fPts[vIndex].fY);
        }
        SkDebugf(") t=%1.9g (%1.9g,%1.9g) newWindSum=%d windSum=",
                span.fT, pt.fX, pt.fY, winding);
        if (span.fWindSum == SK_MinS32) {
            SkDebugf("?");
        } else {
            SkDebugf("%d", span.fWindSum);
        }
        SkDebugf(" windValue=%d\n", span.fWindValue);
    }
#endif

#if DEBUG_SORT
    void debugShowSort(const SkTDArray<Angle*>& angles, int first,
            const int contourWinding) const {
        SkASSERT(angles[first]->segment() == this);
        SkASSERT(angles.count() > 1);
        int lastSum = contourWinding;
        int windSum = lastSum - spanSign(angles[first]);
        SkDebugf("%s contourWinding=%d bump=%d\n", __FUNCTION__,
                contourWinding, spanSign(angles[first]));
        int index = first;
        bool firstTime = true;
        do {
            const Angle& angle = *angles[index];
            const Segment& segment = *angle.segment();
            int start = angle.start();
            int end = angle.end();
            const Span& sSpan = segment.fTs[start];
            const Span& eSpan = segment.fTs[end];
            const Span& mSpan = segment.fTs[SkMin32(start, end)];
            if (!firstTime) {
                lastSum = windSum;
                windSum -= segment.spanSign(&angle);
            }
            SkDebugf("%s [%d] id=%d start=%d (%1.9g,%,1.9g) end=%d (%1.9g,%,1.9g)"
                     " sign=%d windValue=%d winding: %d->%d (max=%d) done=%d\n",
                     __FUNCTION__, index, segment.fID, start, segment.xAtT(&sSpan),
                     segment.yAtT(&sSpan), end, segment.xAtT(&eSpan),
                     segment.yAtT(&eSpan), angle.sign(), mSpan.fWindValue,
                     lastSum, windSum, useInnerWinding(lastSum, windSum)
                     ? windSum : lastSum, mSpan.fDone);
            ++index;
            if (index == angles.count()) {
                index = 0;
            }
            if (firstTime) {
                firstTime = false;
            }
        } while (index != first);
    }
#endif

#if DEBUG_WINDING
    bool debugVerifyWinding(int start, int end, int winding) const {
        const Span& span = fTs[SkMin32(start, end)];
        int spanWinding = span.fWindSum;
        if (spanWinding == SK_MinS32) {
            return true;
        }
        int spanSign = SkSign32(start - end);
        int signedVal = spanSign * span.fWindValue;
        if (signedVal < 0) {
            spanWinding -= signedVal;
        }
        return span.fWindSum == winding;
    }
#endif

private:
    const SkPoint* fPts;
    SkPath::Verb fVerb;
    Bounds fBounds;
    SkTDArray<Span> fTs; // two or more (always includes t=0 t=1)
    int fDoneSpans; // used for quick check that segment is finished
#if DEBUG_DUMP
    int fID;
#endif
};

class Contour;

struct Coincidence {
    Contour* fContours[2];
    int fSegments[2];
    double fTs[2][2];
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

    void addCoincident(int index, Contour* other, int otherIndex,
            const Intersections& ts, bool swap) {
        Coincidence& coincidence = *fCoincidences.append();
        coincidence.fContours[0] = this;
        coincidence.fContours[1] = other;
        coincidence.fSegments[0] = index;
        coincidence.fSegments[1] = otherIndex;
        coincidence.fTs[swap][0] = ts.fT[0][0];
        coincidence.fTs[swap][1] = ts.fT[0][1];
        coincidence.fTs[!swap][0] = ts.fT[1][0];
        coincidence.fTs[!swap][1] = ts.fT[1][1];
    }

    void addCross(const Contour* crosser) {
#ifdef DEBUG_CROSS
        for (int index = 0; index < fCrosses.count(); ++index) {
            SkASSERT(fCrosses[index] != crosser);
        }
#endif
        *fCrosses.append() = crosser;
    }

    void addCubic(const SkPoint pts[4]) {
        fSegments.push_back().addCubic(pts);
        fContainsCurves = true;
    }

    int addLine(const SkPoint pts[2]) {
        fSegments.push_back().addLine(pts);
        return fSegments.count();
    }
    
    void addOtherT(int segIndex, int tIndex, double otherT, int otherIndex) {
        fSegments[segIndex].addOtherT(tIndex, otherT, otherIndex);
    }

    int addQuad(const SkPoint pts[3]) {
        fSegments.push_back().addQuad(pts);
        fContainsCurves = true;
        return fSegments.count();
    }

    int addT(int segIndex, double newT, Contour* other, int otherIndex) {
        containsIntercepts();
        return fSegments[segIndex].addT(newT, &other->fSegments[otherIndex]);
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

    const Segment* crossedSegment(const SkPoint& basePt, SkScalar& bestY, 
            int &tIndex, double& hitT) {
        int segmentCount = fSegments.count();
        const Segment* bestSegment = NULL;
        for (int test = 0; test < segmentCount; ++test) {
            Segment* testSegment = &fSegments[test];
            const SkRect& bounds = testSegment->bounds();
            if (bounds.fBottom <= bestY) {
                continue;
            }
            if (bounds.fTop >= basePt.fY) {
                continue;
            }
            if (bounds.fLeft > basePt.fX) {
                continue;
            }
            if (bounds.fRight < basePt.fX) {
                continue;
            }
            if (bounds.fLeft == bounds.fRight) {
                continue;
            }
     #if 0
            bool leftHalf = bounds.fLeft == basePt.fX;
            bool rightHalf = bounds.fRight == basePt.fX;
            if ((leftHalf || rightHalf) && !testSegment->crossedSpanHalves(
                    basePt, leftHalf, rightHalf)) {
                continue;
            }
     #endif
            double testHitT;
            int testT = testSegment->crossedSpan(basePt, bestY, testHitT);
            if (testT >= 0) {
                bestSegment = testSegment;
                tIndex = testT;
                hitT = testHitT;
            }
        }
        return bestSegment;
    }
    
    bool crosses(const Contour* crosser) const {
        for (int index = 0; index < fCrosses.count(); ++index) {
            if (fCrosses[index] == crosser) {
                return true;
            }
        }
        return false;
    }

    void findTooCloseToCall(int winding) {
        int segmentCount = fSegments.count();
        for (int sIndex = 0; sIndex < segmentCount; ++sIndex) {
            fSegments[sIndex].findTooCloseToCall(winding);
        }
    }

    void fixOtherTIndex() {
        int segmentCount = fSegments.count();
        for (int sIndex = 0; sIndex < segmentCount; ++sIndex) {
            fSegments[sIndex].fixOtherTIndex();
        }
    }

    void reset() {
        fSegments.reset();
        fBounds.set(SK_ScalarMax, SK_ScalarMax, SK_ScalarMax, SK_ScalarMax);
        fContainsCurves = fContainsIntercepts = false;
    }

    void resolveCoincidence(int winding) {
        int count = fCoincidences.count();
        for (int index = 0; index < count; ++index) {
            Coincidence& coincidence = fCoincidences[index];
            Contour* thisContour = coincidence.fContours[0];
            Contour* otherContour = coincidence.fContours[1];
            int thisIndex = coincidence.fSegments[0];
            int otherIndex = coincidence.fSegments[1];
            Segment& thisOne = thisContour->fSegments[thisIndex];
            Segment& other = otherContour->fSegments[otherIndex];
        #if DEBUG_CONCIDENT
            thisOne.debugShowTs();
            other.debugShowTs();
        #endif
            double startT = coincidence.fTs[0][0];
            double endT = coincidence.fTs[0][1];
            if (startT > endT) {
                SkTSwap<double>(startT, endT);
            }
            SkASSERT(endT - startT >= FLT_EPSILON);
            double oStartT = coincidence.fTs[1][0];
            double oEndT = coincidence.fTs[1][1];
            if (oStartT > oEndT) {
                SkTSwap<double>(oStartT, oEndT);
            }
            SkASSERT(oEndT - oStartT >= FLT_EPSILON);
            if (winding > 0 || thisOne.cancels(other)) {
                        // make sure startT and endT have t entries
                if (startT > 0 || oEndT < 1
                        || thisOne.isMissing(startT) || other.isMissing(oEndT)) {
                    thisOne.addTPair(startT, other, oEndT, true);
                }
                if (oStartT > 0 || endT < 1
                        || thisOne.isMissing(endT) || other.isMissing(oStartT)) {
                    other.addTPair(oStartT, thisOne, endT, true);
                }
                thisOne.addTCancel(startT, endT, other, oStartT, oEndT);
            } else {
                if (startT > 0 || oStartT > 0
                        || thisOne.isMissing(startT) || other.isMissing(oStartT)) {
                    thisOne.addTPair(startT, other, oStartT, true);
                }
                if (endT < 1 || oEndT < 1
                        || thisOne.isMissing(endT) || other.isMissing(oEndT)) {
                    other.addTPair(oEndT, thisOne, endT, true);
                }
                thisOne.addTCoincident(startT, endT, other, oStartT, oEndT);
            }
        #if DEBUG_CONCIDENT
            thisOne.debugShowTs();
            other.debugShowTs();
        #endif
        }
    }
    
    const SkTArray<Segment>& segments() {
        return fSegments;
    }
    
    // OPTIMIZATION: feel pretty uneasy about this. It seems like once again
    // we need to sort and walk edges in y, but that on the surface opens the
    // same can of worms as before. But then, this is a rough sort based on 
    // segments' top, and not a true sort, so it could be ameniable to regular
    // sorting instead of linear searching. Still feel like I'm missing something
    Segment* topSegment(SkScalar& bestY) {
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
        SkScalar bestTop = bestSegment->activeTop();
        for (int test = best + 1; test < segmentCount; ++test) {
            Segment* testSegment = &fSegments[test];
            if (testSegment->done()) {
                continue;
            }
            if (testSegment->bounds().fTop > bestTop) {
                continue;
            }
            SkScalar testTop = testSegment->activeTop();
            if (bestTop > testTop) {
                bestTop = testTop;
                bestSegment = testSegment;
            }
        }
        bestY = bestTop;
        return bestSegment;
    }

    int updateSegment(int index, const SkPoint* pts) {
        Segment& segment = fSegments[index];
        segment.updatePts(pts);
        return segment.verb() + 1;
    }

#if DEBUG_TEST
    SkTArray<Segment>& debugSegments() {
        return fSegments;
    }
#endif

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

#if DEBUG_ACTIVE_SPANS
    void debugShowActiveSpans() {
        for (int index = 0; index < fSegments.count(); ++index) {
            fSegments[index].debugShowActiveSpans();
        }
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
            fBounds.add(fSegments[index].bounds());
        }
    }

private:
    SkTArray<Segment> fSegments;
    SkTDArray<Coincidence> fCoincidences;
    SkTDArray<const Contour*> fCrosses;
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
    if (fCurrentContour && fCurrentContour->segments().count()) {
        fCurrentContour->complete();
        fCurrentContour = NULL;
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
    const SkPoint* finalCurveStart = NULL;
    const SkPoint* finalCurveEnd = NULL;
    while ((verb = (SkPath::Verb) *verbPtr++) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kMove_Verb:
                complete();
                if (!fCurrentContour) {
                    fCurrentContour = fContours.push_back_n(1);
                    *fExtra.append() = -1; // start new contour
                }
                finalCurveEnd = pointsPtr++;
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
                    *fExtra.append() = 
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
                    *fExtra.append() =
                            fCurrentContour->addLine(fReducePts.end() - 2);
                    break;
                }
                if (reducedVerb == 2) {
                    *fExtra.append() =
                            fCurrentContour->addQuad(fReducePts.end() - 3);
                    break;
                }
                fCurrentContour->addCubic(&pointsPtr[-1]);
                break;
            case SkPath::kClose_Verb:
                SkASSERT(fCurrentContour);
                if (finalCurveStart && finalCurveEnd
                        && *finalCurveStart != *finalCurveEnd) {
                    *fReducePts.append() = *finalCurveStart;
                    *fReducePts.append() = *finalCurveEnd;
                    *fExtra.append() =
                            fCurrentContour->addLine(fReducePts.end() - 2);
                }
                complete();
                continue;
            default:
                SkDEBUGFAIL("bad verb");
                return;
        }
        finalCurveStart = &pointsPtr[verb - 1];
        pointsPtr += verb;
        SkASSERT(fCurrentContour);
    }
    complete();
    if (fCurrentContour && !fCurrentContour->segments().count()) {
        fContours.pop_back();
    }
    // correct pointers in contours since fReducePts may have moved as it grew
    int cIndex = 0;
    int extraCount = fExtra.count();
    SkASSERT(extraCount == 0 || fExtra[0] == -1);
    int eIndex = 0;
    int rIndex = 0;
    while (++eIndex < extraCount) {
        int offset = fExtra[eIndex];
        if (offset < 0) {
            ++cIndex;
            continue;
        }
        fCurrentContour = &fContours[cIndex];
        rIndex += fCurrentContour->updateSegment(offset - 1,
                &fReducePts[rIndex]);
    }
    fExtra.reset(); // we're done with this
}

private:
    const SkPath& fPath;
    SkTDArray<SkPoint> fPathPts; // FIXME: point directly to path pts instead
    SkTDArray<uint8_t> fPathVerbs; // FIXME: remove
    Contour* fCurrentContour;
    SkTArray<Contour>& fContours;
    SkTDArray<SkPoint> fReducePts; // segments created on the fly
    SkTDArray<int> fExtra; // -1 marks new contour, > 0 offsets into contour
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
    
    void addCoincident(Work& other, const Intersections& ts, bool swap) {
        fContour->addCoincident(fIndex, other.fContour, other.fIndex, ts, swap);
    }

    // FIXME: does it make sense to write otherIndex now if we're going to
    // fix it up later?
    void addOtherT(int index, double otherT, int otherIndex) {
        fContour->addOtherT(fIndex, index, otherT, otherIndex);
    }

    // Avoid collapsing t values that are close to the same since
    // we walk ts to describe consecutive intersections. Since a pair of ts can
    // be nearly equal, any problems caused by this should be taken care
    // of later.
    // On the edge or out of range values are negative; add 2 to get end
    int addT(double newT, const Work& other) {
        return fContour->addT(fIndex, newT, other.fContour, other.fIndex);
    }

    bool advance() {
        return ++fIndex < fLast;
    }

    SkScalar bottom() const {
        return bounds().fBottom;
    }

    const Bounds& bounds() const {
        return fContour->segments()[fIndex].bounds();
    }
    
    const SkPoint* cubic() const {
        return fCubic;
    }

    void init(Contour* contour) {
        fContour = contour;
        fIndex = 0;
        fLast = contour->segments().count();
    }
    
    bool isAdjacent(const Work& next) {
        return fContour == next.fContour && fIndex + 1 == next.fIndex;
    }

    bool isFirstLast(const Work& next) {
        return fContour == next.fContour && fIndex == 0
                && next.fIndex == fLast - 1;
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
        return fContour->segments()[fIndex].pts();
    }

    SkScalar right() const {
        return bounds().fRight;
    }

    ptrdiff_t segmentIndex() const {
        return fIndex;
    }

    SegmentType segmentType() const {
        const Segment& segment = fContour->segments()[fIndex];
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
        return fContour->segments()[fIndex].verb();
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
        return y() != pts()[0].fY;
    }

protected:
    Contour* fContour;
    SkPoint fCubic[4];
    int fIndex;
    int fLast;
};

#if DEBUG_ADD_INTERSECTING_TS
static void debugShowLineIntersection(int pts, const Work& wt,
        const Work& wn, const double wtTs[2], const double wnTs[2]) {
    if (!pts) {
        SkDebugf("%s no intersect (%1.9g,%1.9g %1.9g,%1.9g) (%1.9g,%1.9g %1.9g,%1.9g)\n",
                __FUNCTION__, wt.pts()[0].fX, wt.pts()[0].fY,
                wt.pts()[1].fX, wt.pts()[1].fY, wn.pts()[0].fX, wn.pts()[0].fY,
                wn.pts()[1].fX, wn.pts()[1].fY);
        return;
    }
    SkPoint wtOutPt, wnOutPt;
    LineXYAtT(wt.pts(), wtTs[0], &wtOutPt);
    LineXYAtT(wn.pts(), wnTs[0], &wnOutPt);
    SkDebugf("%s wtTs[0]=%g (%g,%g, %g,%g) (%g,%g)",
            __FUNCTION__,
            wtTs[0], wt.pts()[0].fX, wt.pts()[0].fY,
            wt.pts()[1].fX, wt.pts()[1].fY, wtOutPt.fX, wtOutPt.fY);
    if (pts == 2) {
        SkDebugf(" wtTs[1]=%g", wtTs[1]);
    }
    SkDebugf(" wnTs[0]=%g (%g,%g, %g,%g) (%g,%g)",
            wnTs[0], wn.pts()[0].fX, wn.pts()[0].fY,
            wn.pts()[1].fX, wn.pts()[1].fY, wnOutPt.fX, wnOutPt.fY);
    if (pts == 2) {
        SkDebugf(" wnTs[1]=%g", wnTs[1]);
    }
    SkDebugf("\n");
}
#else
static void debugShowLineIntersection(int , const Work& ,
        const Work& , const double [2], const double [2]) {
}
#endif

static bool addIntersectTs(Contour* test, Contour* next) {

    if (test != next) {
        if (test->bounds().fBottom < next->bounds().fTop) {
            return false;
        }
        if (!Bounds::Intersects(test->bounds(), next->bounds())) {
            return true;
        }
    }
    Work wt;
    wt.init(test);
    bool foundCommonContour = test == next;
    do {
        Work wn;
        wn.init(next);
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
                            debugShowLineIntersection(pts, wt, wn,
                                    ts.fT[1], ts.fT[0]);
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
                            debugShowLineIntersection(pts, wt, wn,
                                    ts.fT[1], ts.fT[0]);
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
                            debugShowLineIntersection(pts, wt, wn,
                                    ts.fT[1], ts.fT[0]);
                            break;
                        case Work::kVerticalLine_Segment:
                            pts = VLineIntersect(wt.pts(), wn.top(),
                                    wn.bottom(), wn.x(), wn.yFlipped(), ts);
                            debugShowLineIntersection(pts, wt, wn,
                                    ts.fT[1], ts.fT[0]);
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
            if (!foundCommonContour && pts > 0) {
                test->addCross(next);
                next->addCross(test);
                foundCommonContour = true;
            }
            // in addition to recording T values, record matching segment
            if (pts == 2 && wn.segmentType() <= Work::kLine_Segment
                    && wt.segmentType() <= Work::kLine_Segment) {
                wt.addCoincident(wn, ts, swap);
                continue;
            }
            for (int pt = 0; pt < pts; ++pt) {
                SkASSERT(ts.fT[0][pt] >= 0 && ts.fT[0][pt] <= 1);
                SkASSERT(ts.fT[1][pt] >= 0 && ts.fT[1][pt] <= 1);
                int testTAt = wt.addT(ts.fT[swap][pt], wn);
                int nextTAt = wn.addT(ts.fT[!swap][pt], wt);
                wt.addOtherT(testTAt, ts.fT[!swap][pt], nextTAt);
                wn.addOtherT(nextTAt, ts.fT[swap][pt], testTAt);
            }
        } while (wn.advance());
    } while (wt.advance());
    return true;
}

// resolve any coincident pairs found while intersecting, and
// see if coincidence is formed by clipping non-concident segments
static void coincidenceCheck(SkTDArray<Contour*>& contourList, int winding) {
    int contourCount = contourList.count();
    for (int cIndex = 0; cIndex < contourCount; ++cIndex) {
        Contour* contour = contourList[cIndex];
        contour->findTooCloseToCall(winding);
    }
    for (int cIndex = 0; cIndex < contourCount; ++cIndex) {
        Contour* contour = contourList[cIndex];
        contour->resolveCoincidence(winding);
    }
}

// project a ray from the top of the contour up and see if it hits anything
// note: when we compute line intersections, we keep track of whether
// two contours touch, so we need only look at contours not touching this one.
// OPTIMIZATION: sort contourList vertically to avoid linear walk
static int innerContourCheck(SkTDArray<Contour*>& contourList,
        const Segment* current, int index, int endIndex) {
    const SkPoint& basePt = current->xyAtT(endIndex);
    int contourCount = contourList.count();
    SkScalar bestY = SK_ScalarMin;
    const Segment* test = NULL;
    int tIndex;
    double tHit;
 //   bool checkCrosses = true;
    for (int cTest = 0; cTest < contourCount; ++cTest) {
        Contour* contour = contourList[cTest];
        if (basePt.fY < contour->bounds().fTop) {
            continue;
        }
        if (bestY > contour->bounds().fBottom) {
            continue;
        }
#if 0
        // even though the contours crossed, if spans cancel through concidence,
        // the contours may be not have any span links to chase, and the current
        // segment may be isolated. Detect this by seeing if current has
        // uninitialized wind sums. If so, project a ray instead of relying on
        // previously found intersections.
        if (baseContour == contour) {
            continue;
        }
        if (checkCrosses && baseContour->crosses(contour)) {
            if (current->isConnected(index, endIndex)) {
                continue;
            }
            checkCrosses = false;
        }
#endif
        const Segment* next = contour->crossedSegment(basePt, bestY, tIndex, tHit);
        if (next) {
            test = next;
        }
    }
    if (!test) {
        return 0;
    }
    int winding, windValue;
    // If the ray hit the end of a span, we need to construct the wheel of
    // angles to find the span closest to the ray -- even if there are just
    // two spokes on the wheel.
    const Angle* angle = NULL;
    if (fabs(tHit - test->t(tIndex)) < FLT_EPSILON) {
        SkTDArray<Angle> angles;
        int end = test->nextSpan(tIndex, 1);
        if (end < 0) {
            end = test->nextSpan(tIndex, -1);
        }
        test->addTwoAngles(end, tIndex, angles);
        SkASSERT(angles.count() > 0);
        if (angles[0].segment()->yAtT(angles[0].start()) >= basePt.fY) {
#if DEBUG_SORT
            SkDebugf("%s *** early return\n", __FUNCTION__);
#endif
            return 0;
        }
        test->buildAngles(tIndex, angles);
        SkTDArray<Angle*> sorted;
        // OPTIMIZATION: call a sort that, if base point is the leftmost, 
        // returns the first counterclockwise hour before 6 o'clock,
        // or if the base point is rightmost, returns the first clockwise 
        // hour after 6 o'clock
        sortAngles(angles, sorted);
#if DEBUG_SORT
        sorted[0]->segment()->debugShowSort(sorted, 0, 0);  
#endif
        // walk the sorted angle fan to find the lowest angle
        // above the base point. Currently, the first angle in the sorted array
        // is 12 noon or an earlier hour (the next counterclockwise)
        int count = sorted.count();
        int left = -1;
        int mid = -1;
        int right = -1;
        bool baseMatches = test->yAtT(tIndex) == basePt.fY;
        for (int index = 0; index < count; ++index) {
            angle = sorted[index];
            if (baseMatches && angle->isHorizontal()) {
                continue;
            }
            double indexDx = angle->dx();
            if (indexDx < 0) {
                left = index;
            } else if (indexDx > 0) {
                right = index;
                int previous = index - 1;
                if (previous < 0) {
                    previous = count - 1;
                }
                const Angle* prev = sorted[previous];
                if (prev->dy() >= 0 && prev->dx() > 0 && angle->dy() < 0) {
#if DEBUG_SORT
                    SkDebugf("%s use prev\n", __FUNCTION__);
#endif
                    right = previous;
                }
                break;
            } else {
                mid = index;
            }
        }
        if (left < 0 && right < 0) {
            left = mid;
        }
        SkASSERT(left >= 0 || right >= 0);
        if (left < 0) {
            left = right;
        } else if (left >= 0 && mid >= 0 && right >= 0
                && sorted[mid]->sign() == sorted[right]->sign()) {
            left = right;
        }
        angle = sorted[left];
        test = angle->segment();
        winding = test->windSum(angle);
        SkASSERT(winding != SK_MinS32);
        windValue = test->windValue(angle);
#if DEBUG_WINDING
        SkDebugf("%s angle winding=%d windValue=%d sign=%d\n", __FUNCTION__, winding,
                windValue, angle->sign());
#endif
    } else {
        winding = test->windSum(tIndex);
        SkASSERT(winding != SK_MinS32);
        windValue = test->windValue(tIndex);
#if DEBUG_WINDING
        SkDebugf("%s single winding=%d windValue=%d\n", __FUNCTION__, winding,
                windValue);
#endif
    }
    // see if a + change in T results in a +/- change in X (compute x'(T))
    SkScalar dx = (*SegmentDXAtT[test->verb()])(test->pts(), tHit);
#if DEBUG_WINDING
    SkDebugf("%s dx=%1.9g\n", __FUNCTION__, dx);
#endif
    SkASSERT(dx != 0);
    if (winding * dx > 0) { // if same signs, result is negative
        winding += dx > 0 ? -windValue : windValue;
#if DEBUG_WINDING
        SkDebugf("%s final winding=%d\n", __FUNCTION__, winding);
#endif
    }
 //   start here;
    // we're broken because we find a vertical span
    return winding;
}
    
// OPTIMIZATION: not crazy about linear search here to find top active y.
// seems like we should break down and do the sort, or maybe sort each
// contours' segments? 
// Once the segment array is built, there's no reason I can think of not to
// sort it in Y. hmmm
// FIXME: return the contour found to pass to inner contour check
static Segment* findTopContour(SkTDArray<Contour*>& contourList) {
    int contourCount = contourList.count();
    int cIndex = 0;
    Segment* topStart;
    SkScalar bestY = SK_ScalarMax;
    Contour* contour;
    do {
        contour = contourList[cIndex];
        topStart = contour->topSegment(bestY);
    } while (!topStart && ++cIndex < contourCount);
    if (!topStart) {
        return NULL;
    }
    while (++cIndex < contourCount) {
        contour = contourList[cIndex];
        if (bestY < contour->bounds().fTop) {
            continue;
        }
        SkScalar testY = SK_ScalarMax;
        Segment* test = contour->topSegment(testY);
        if (!test || bestY <= testY) {
            continue;
        }
        topStart = test;
        bestY = testY;
    }
    return topStart;
}

static Segment* findChase(SkTDArray<Span*>& chase, int& tIndex, int& endIndex,
        int contourWinding) {
    while (chase.count()) {
        Span* span = chase[chase.count() - 1];
        const Span& backPtr = span->fOther->span(span->fOtherIndex);
        Segment* segment = backPtr.fOther;
        tIndex = backPtr.fOtherIndex;
        SkTDArray<Angle> angles;
        int done = 0;
        if (segment->activeAngle(tIndex, done, angles)) {
            Angle* last = angles.end() - 1;
            tIndex = last->start();
            endIndex = last->end();
            return last->segment();
        }
        if (done == angles.count()) {
            chase.pop(&span);
            continue;
        }
        SkTDArray<Angle*> sorted;
        sortAngles(angles, sorted);
        // find first angle, initialize winding to computed fWindSum
        int firstIndex = -1;
        const Angle* angle;
        int winding;
        do {
            angle = sorted[++firstIndex];
            segment = angle->segment();
            winding = segment->windSum(angle);
        } while (winding == SK_MinS32);
        int spanWinding = segment->spanSign(angle->start(), angle->end());
    #if DEBUG_WINDING
        SkDebugf("%s winding=%d spanWinding=%d contourWinding=%d\n",
                __FUNCTION__, winding, spanWinding, contourWinding);
    #endif
        // turn swinding into contourWinding
        if (spanWinding * winding < 0) {
            winding += spanWinding;
        }
    #if DEBUG_SORT
        segment->debugShowSort(sorted, firstIndex, winding);  
    #endif
        // we care about first sign and whether wind sum indicates this
        // edge is inside or outside. Maybe need to pass span winding
        // or first winding or something into this function?
        // advance to first undone angle, then return it and winding
        // (to set whether edges are active or not)
        int nextIndex = firstIndex + 1;
        int angleCount = sorted.count();
        int lastIndex = firstIndex != 0 ? firstIndex : angleCount;
        angle = sorted[firstIndex];
        winding -= angle->segment()->spanSign(angle);
        do {
            SkASSERT(nextIndex != firstIndex);
            if (nextIndex == angleCount) {
                nextIndex = 0;
            }
            angle = sorted[nextIndex];
            segment = angle->segment();
            int maxWinding = winding;
            winding -= segment->spanSign(angle);
    #if DEBUG_SORT
            SkDebugf("%s id=%d maxWinding=%d winding=%d sign=%d\n", __FUNCTION__,
                    segment->debugID(), maxWinding, winding, angle->sign());
    #endif
            tIndex = angle->start();
            endIndex = angle->end();
            int lesser = SkMin32(tIndex, endIndex);
            const Span& nextSpan = segment->span(lesser);
            if (!nextSpan.fDone) {
#if 1
            // FIXME: this be wrong. assign startWinding if edge is in
            // same direction. If the direction is opposite, winding to
            // assign is flipped sign or +/- 1?
                if (useInnerWinding(maxWinding, winding)) {
                    maxWinding = winding;
                }
                segment->markWinding(lesser, maxWinding);
#endif
                break;
            }
        } while (++nextIndex != lastIndex);
        return segment;
    }
    return NULL;
}

#if DEBUG_ACTIVE_SPANS
static void debugShowActiveSpans(SkTDArray<Contour*>& contourList) {
    for (int index = 0; index < contourList.count(); ++ index) {
        contourList[index]->debugShowActiveSpans();
    }
}
#endif

static bool windingIsActive(int winding, int spanWinding) {
    return winding * spanWinding <= 0 && abs(winding) <= abs(spanWinding)
            && (!winding || !spanWinding || winding == -spanWinding);
}

// Each segment may have an inside or an outside. Segments contained within
// winding may have insides on either side, and form a contour that should be
// ignored. Segments that are coincident with opposing direction segments may
// have outsides on either side, and should also disappear.
// 'Normal' segments will have one inside and one outside. Subsequent connections
// when winding should follow the intersection direction. If more than one edge
// is an option, choose first edge that continues the inside.
    // since we start with leftmost top edge, we'll traverse through a
    // smaller angle counterclockwise to get to the next edge.  
static void bridge(SkTDArray<Contour*>& contourList, SkPath& simple) {
    bool firstContour = true;
    do {
        Segment* topStart = findTopContour(contourList);
        if (!topStart) {
            break;
        }
        // Start at the top. Above the top is outside, below is inside.
        // follow edges to intersection by changing the index by direction.
        int index, endIndex;
        Segment* current = topStart->findTop(index, endIndex);
        int contourWinding;
        if (firstContour) {
            contourWinding = 0;
            firstContour = false;
        } else {
            int sumWinding = current->windSum(SkMin32(index, endIndex));
            // FIXME: don't I have to adjust windSum to get contourWinding?
            if (sumWinding == SK_MinS32) {
                sumWinding = current->computeSum(index, endIndex);
            }
            if (sumWinding == SK_MinS32) {
                contourWinding = innerContourCheck(contourList, current,
                        index, endIndex);
            } else {
                contourWinding = sumWinding;
                int spanWinding = current->spanSign(index, endIndex);
                bool inner = useInnerWinding(sumWinding - spanWinding, sumWinding);
                if (inner) {
                    contourWinding -= spanWinding;
                }
#if DEBUG_WINDING
                SkDebugf("%s --- sumWinding=%d spanWinding=%d sign=%d inner=%d result=%d\n", __FUNCTION__,
                        sumWinding, spanWinding, SkSign32(index - endIndex), 
                        inner, contourWinding);
#endif
            }
#if DEBUG_WINDING
         //   SkASSERT(current->debugVerifyWinding(index, endIndex, contourWinding));
            SkDebugf("%s contourWinding=%d\n", __FUNCTION__, contourWinding);
#endif
        }
        SkPoint lastPt;
        bool firstTime = true;
        int winding = contourWinding;
        int spanWinding = current->spanSign(index, endIndex);
        // FIXME: needs work. While it works in limited situations, it does
        // not always compute winding correctly. Active should be removed and instead
        // the initial winding should be correctly passed in so that if the
        // inner contour is wound the same way, it never finds an accumulated
        // winding of zero. Inside 'find next', we need to look for transitions
        // other than zero when resolving sorted angles. 
        bool active = windingIsActive(winding, spanWinding);
        SkTDArray<Span*> chaseArray;
        do {
        #if DEBUG_WINDING
            SkDebugf("%s active=%s winding=%d spanWinding=%d\n",
                    __FUNCTION__, active ? "true" : "false",
                    winding, spanWinding);
        #endif
            const SkPoint* firstPt = NULL;
            do {
                SkASSERT(!current->done());
                int nextStart, nextEnd;
                Segment* next = current->findNext(chaseArray,
                        firstTime, active, index, endIndex,
                        nextStart, nextEnd, winding, spanWinding);
                if (!next) {
                    break;
                }
                if (!firstPt) {
                    firstPt = &current->addMoveTo(index, simple, active);
                }
                lastPt = current->addCurveTo(index, endIndex, simple, active);
                current = next;
                index = nextStart;
                endIndex = nextEnd;
                firstTime = false;
            } while (*firstPt != lastPt && (active || !current->done()));
            if (firstPt && active) {
        #if DEBUG_PATH_CONSTRUCTION
                SkDebugf("%s close\n", __FUNCTION__);
        #endif
                simple.close();
            }
            current = findChase(chaseArray, index, endIndex, contourWinding);
        #if DEBUG_ACTIVE_SPANS
            debugShowActiveSpans(contourList);
        #endif
            if (!current) {
                break;
            }
            int lesser = SkMin32(index, endIndex);
            spanWinding = current->spanSign(index, endIndex);
            winding = current->windSum(lesser);
            bool inner = useInnerWinding(winding - spanWinding, winding);
        #if DEBUG_WINDING
            SkDebugf("%s --- id=%d t=%1.9g spanWinding=%d winding=%d sign=%d"
                    " inner=%d result=%d\n",
                    __FUNCTION__, current->debugID(), current->t(lesser),
                    spanWinding, winding, SkSign32(index - endIndex),
                    useInnerWinding(winding - spanWinding, winding),
                    inner ? winding - spanWinding : winding);
        #endif
            if (inner) {
                winding -= spanWinding;
            }
            active = windingIsActive(winding, spanWinding);
        } while (true);
    } while (true);
}

static void fixOtherTIndex(SkTDArray<Contour*>& contourList) {
    int contourCount = contourList.count();
    for (int cTest = 0; cTest < contourCount; ++cTest) {
        Contour* contour = contourList[cTest];
        contour->fixOtherTIndex();
    }
}

static void makeContourList(SkTArray<Contour>& contours,
        SkTDArray<Contour*>& list) {
    int count = contours.count();
    if (count == 0) {
        return;
    }
    for (int index = 0; index < count; ++index) {
        *list.append() = &contours[index];
    }
    QSort<Contour>(list.begin(), list.end() - 1);
}

void simplifyx(const SkPath& path, SkPath& simple) {
    // returns 1 for evenodd, -1 for winding, regardless of inverse-ness
    int winding = (path.getFillType() & 1) ? 1 : -1;
    simple.reset();
    simple.setFillType(SkPath::kEvenOdd_FillType);

    // turn path into list of segments
    SkTArray<Contour> contours;
    // FIXME: add self-intersecting cubics' T values to segment
    EdgeBuilder builder(path, contours);
    SkTDArray<Contour*> contourList;
    makeContourList(contours, contourList);
    Contour** currentPtr = contourList.begin();
    if (!currentPtr) {
        return;
    }
    Contour** listEnd = contourList.end();
    // find all intersections between segments
    do {
        Contour** nextPtr = currentPtr;
        Contour* current = *currentPtr++;
        Contour* next;
        do {
            next = *nextPtr++;
        } while (addIntersectTs(current, next) && nextPtr != listEnd);
    } while (currentPtr != listEnd);
    // eat through coincident edges
    coincidenceCheck(contourList, winding);
    fixOtherTIndex(contourList);
    // construct closed contours
    bridge(contourList, simple);
}

