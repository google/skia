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

// FIXME: remove once debugging is complete
#if 0 // set to 1 for no debugging whatsoever

//const bool gxRunTestsInOneThread = false;

#define DEBUG_ADD_INTERSECTING_TS 0
#define DEBUG_BRIDGE 0
#define DEBUG_DUMP 0
#define DEBUG_PATH_CONSTRUCTION 0
#define DEBUG_UNUSED 0 // set to expose unused functions

#else

//const bool gRunTestsInOneThread = true;

#define DEBUG_ADD_INTERSECTING_TS 0
#define DEBUG_BRIDGE 1
#define DEBUG_DUMP 1
#define DEBUG_PATH_CONSTRUCTION 1
#define DEBUG_UNUSED 0 // set to expose unused functions

#endif

#if DEBUG_DUMP
static const char* kLVerbStr[] = {"", "line", "quad", "cubic"};
// static const char* kUVerbStr[] = {"", "Line", "Quad", "Cubic"};
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

    int end() const {
        return fEnd;
    }

    // since all angles share a point, this needs to know which point
    // is the common origin, i.e., whether the center is at pts[0] or pts[verb]
    // practically, this should only be called by addAngle
    void set(const SkPoint* pts, SkPath::Verb verb, Segment* segment,
            int start, int end, bool coincident) {
        SkASSERT(start != end);
        fSegment = segment;
        fStart = start;
        fEnd = end;
        fCoincident = coincident;
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
            int start, int end, bool coincident) {
        fSegment = segment;
        fStart = start;
        fEnd = end;
        fCoincident = coincident;
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
        return fSegment;
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
    Segment* fSegment;
    int fStart;
    int fEnd;
    bool fCoincident;
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

struct Span {
    double fT;
    Segment* fOther;
    double fOtherT; // value at fOther[fOtherIndex].fT
    int fOtherIndex;  // can't be used during intersection
    int fWinding; // accumulated from contours surrounding this one
    // OPTIMIZATION: coincident needs only 2 bits (values are -1, 0, 1)
    int fCoincident; // -1 start of coincidence, 0 no coincidence, 1 end
    bool fDone; // if set, this span to next higher T has been processed
};

class Segment {
public:
    Segment() {
#if DEBUG_DUMP
        fID = ++gSegmentID;
#endif
    }
    
    void addAngle(SkTDArray<Angle>& angles, int start, int end,
            bool coincident) {
        SkASSERT(start != end);
        int smaller = SkMin32(start, end);
        if (fTs[smaller].fDone) {
            return;
        }
        SkPoint edge[4];
        (*SegmentSubDivide[fVerb])(fPts, fTs[start].fT, fTs[end].fT, edge);
        Angle* angle = angles.append();
        angle->set(edge, fVerb, this, start, end, coincident);
    }

    void addCubic(const SkPoint pts[4]) {
        init(pts, SkPath::kCubic_Verb);
        fBounds.setCubicBounds(pts);
    }

    void addCurveTo(int start, int end, SkPath& path) {
        SkPoint edge[4];
        (*SegmentSubDivide[fVerb])(fPts, fTs[start].fT, fTs[end].fT, edge);
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

    void addLine(const SkPoint pts[2]) {
        init(pts, SkPath::kLine_Verb);
        fBounds.set(pts, 2);
    }

    void addMoveTo(int tIndex, SkPath& path) {
        SkPoint pt;
        double firstT = t(tIndex);
        xyAtT(firstT, &pt);
    #if DEBUG_PATH_CONSTRUCTION
        SkDebugf("%s (%1.9g,%1.9g)\n", __FUNCTION__, pt.fX, pt.fY);
    #endif
        path.moveTo(pt.fX, pt.fY);
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

    int addT(double newT, Segment& other, int coincident) {
        // FIXME: in the pathological case where there is a ton of intercepts,
        //  binary search?
        int insertedAt = -1;
        Span* span;
        size_t tCount = fTs.count();
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
        span->fWinding = 0;
        if (span->fDone = newT == 1) {
            ++fDoneSpans;
        } 
        span->fCoincident = coincident;
        fCoincident |= coincident;
        return insertedAt;
    }

    void addTwoAngles(int start, int end, const SkPoint& endLoc,
            const Span* endSpan, bool startCo, SkTDArray<Angle>& angles) {
        // add edge leading into junction
        addAngle(angles, end, start, startCo);
        // add edge leading away from junction
        bool coincident;
        int step = SkSign32(end - start);
        int tIndex = nextSpan(end, step, endLoc, endSpan, NULL, coincident);
        if (tIndex >= 0) {
            lastSpan(tIndex, step, endLoc, endSpan->fT, coincident);
            addAngle(angles, end, tIndex, coincident);
        }
    }

    const Bounds& bounds() const {
        return fBounds;
    }

    void buildAngles(int index, int last, int step, const SkPoint& loc,
            SkTDArray<Angle>& angles) const {
        SkASSERT(index - last != 0);
        SkASSERT((index - last  < 0) ^ (step < 0));
        int end = last + step;  
        do {
            Span* span = &fTs[index];
            Segment* other = span->fOther;
            if (other->done()) {
                continue;
            }
        // if there is only one live crossing, and no coincidence, continue
        // in the same direction
        // if there is coincidence, the only choice may be to reverse direction
            // find edge on either side of intersection
            int oIndex = span->fOtherIndex;
            Span* otherSpan = &other->fTs[oIndex];
            SkASSERT(otherSpan->fOther == this);
            // if done == -1, prior span has already been processed
            bool otherCo;
            int localStep = step;
            int next = other->nextSpan(oIndex, localStep, loc, otherSpan,
                    NULL, otherCo);
            if (next < 0) {
                localStep = -step;
                next = other->nextSpan(oIndex, localStep, loc, otherSpan,
                    NULL, otherCo);
            }
            other->lastSpan(next, localStep, loc, otherSpan->fT, otherCo);
            // add candidate into and away from junction
            other->addTwoAngles(next, oIndex, loc, span, otherCo, angles);
            
        } while ((index += step) != end);
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

    bool coincident(int index, const Angle* angle) const {
        Span* span;
        double referenceT = fTs[index].fT;
        int lesser = index;
        while (--lesser >= 0 && referenceT == fTs[lesser].fT) {
            span = &fTs[lesser];
            if (span->fOther == angle->segment()) {
                goto checkOther;
            }
        }
        do {
            span = &fTs[index];
            if (span->fOther == angle->segment()) {
                break;
            }
            
        } while (++index < fTs.count() && referenceT == fTs[index].fT);
    checkOther:
        SkASSERT(!span->fDone);
        return span->fCoincident;
    }
    
    bool done() const {
        SkASSERT(fDoneSpans <= fTs.count());
        return fDoneSpans == fTs.count();
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
    Segment* findNext(int winding, const int startIndex, const int endIndex,
            int& nextStart, int& nextEnd) {
        SkASSERT(startIndex != endIndex);
        int count = fTs.count();
        SkASSERT(startIndex < endIndex ? startIndex < count - 1
                : startIndex > 0);
        Span* startSpan = &fTs[startIndex];
        // FIXME:
        // since Ts can be stepped either way, done markers must be careful
        // not to assume that segment was only ascending in T. This shouldn't
        // be a problem unless pathologically a segment can be partially
        // ascending and partially descending -- maybe quads/cubic can do this?
        

        int step = SkSign32(endIndex - startIndex);
        SkPoint startLoc; // OPTIMIZATION: store this in the t span?
        xyAtT(startSpan->fT, &startLoc);
        SkPoint endLoc;
        bool startCo;
        int end = nextSpan(startIndex, step, startLoc, startSpan, &endLoc,
                startCo);
        SkASSERT(end >= 0);

        // preflight for coincidence -- if present, it may change winding
        // considerations and whether reversed edges can be followed
        bool many;
        int last = lastSpan(end, step, endLoc, fTs[end].fT, startCo, &many);
        
        // Discard opposing direction candidates if no coincidence was found.
        Span* endSpan = &fTs[end];
        Segment* other;
        if (!many) {
        // mark the smaller of startIndex, endIndex done, and all adjacent
        // spans with the same T value (but not 'other' spans)
            markDone(SkMin32(startIndex, endIndex), winding);
            SkASSERT(!startCo);
            // move in winding direction until edge in correct direction
            // balance wrong direction edges before finding correct one
            // this requres that the intersection is angularly sorted
            // for a single intersection, special case -- choose the opposite
            // edge that steps the same
            other = endSpan->fOther;
            nextStart = endSpan->fOtherIndex;
            nextEnd = nextStart + step;
            SkASSERT(step < 0 ? nextEnd >= 0 : nextEnd < other->fTs.count());
            return other;
        }

        // more than one viable candidate -- measure angles to find best
        SkTDArray<Angle> angles;
        SkASSERT(startIndex - endIndex != 0);
        SkASSERT((startIndex - endIndex < 0) ^ (step < 0));
        addTwoAngles(startIndex, end, endLoc, endSpan, startCo, angles);
        buildAngles(end, last, step, endLoc, angles);
        SkTDArray<Angle*> sorted;
        sortAngles(angles, sorted);
        // find the starting edge
        int firstIndex = -1;
        int angleCount = angles.count();
        int angleIndex;
        const Angle* angle;
        for (angleIndex = 0; angleIndex < angleCount; ++angleIndex) {
            angle = sorted[angleIndex];
            if (angle->segment() == this && angle->start() == end &&
                    angle->end() == startIndex) {
                firstIndex = angleIndex;
                break;
            }
        }
        
    // put some thought into handling coincident edges
    // 1) defer the initial moveTo/curveTo until we know that firstIndex
    //    isn't coincident (although maybe findTop could tell us that)
    // 2) allow the below to mark and skip coincident pairs
    // 3) return something (null?) if all segments cancel each other out
    // 4) if coincident edges don't cancel, figure out which to return (follow)   
    
        SkASSERT(firstIndex >= 0);
        int startWinding = winding;
        int nextIndex = firstIndex;
        const Angle* nextAngle;
        do {
            if (++nextIndex == angleCount) {
                nextIndex = 0;
            }
            SkASSERT(nextIndex != firstIndex); // should never wrap around
            nextAngle = sorted[nextIndex];
            int maxWinding = winding;
            winding -= nextAngle->sign();
            if (abs(maxWinding) < abs(winding)) {
                maxWinding = winding;
            }
            other = nextAngle->segment();
            if (!winding) {
                if (!startCo || !coincident(startIndex, nextAngle)) {
                    break;
                }
                markAndChaseCoincident(startIndex, endIndex, other);
                return NULL;
            }
            // if the winding is non-zero, nextAngle does not connect to
            // current chain. If we haven't done so already, mark the angle
            // as done, record the winding value, and mark connected unambiguous
            // segments as well.
            if (other->winding(nextAngle) == 0) {
                other->markAndChaseWinding(nextAngle, maxWinding);
            }
            
        } while (true);
        markDone(SkMin32(startIndex, endIndex), startWinding);
        nextStart = nextAngle->start();
        nextEnd = nextAngle->end();
        return other;
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
        SkPoint matchPt;
        // OPTIMIZATION: defer matchPt until qualifying toCount is found?
        xyAtT(match->fT, &matchPt);
        // look for a pair of nearby T values that map to the same (x,y) value
        // if found, see if the pair of other segments share a common point. If
        // so, the span from here to there is coincident.
        for (int index = matchIndex + 1; index < count; ++index) {
            Span* test = &fTs[index];
            if (test->fCoincident) {
                continue;
            }
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
            int moStart = -1;
            int moEnd = -1;
            double moStartT, moEndT;
            for (int moIndex = 0; moIndex < moCount; ++moIndex) {
                Span& moSpan = mOther->fTs[moIndex];
                if (moSpan.fCoincident) {
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
            mOther->fTs[moStart].fCoincident = -1;
            tOther->fTs[toStart].fCoincident = -1;
            mOther->fTs[moEnd].fCoincident = 1;
            tOther->fTs[toEnd].fCoincident = 1;
        }
    }

    // OPTIMIZATION : for a pair of lines, can we compute points at T (cached)
    // and use more concise logic like the old edge walker code?
    // FIXME: this needs to deal with coincident edges
    Segment* findTop(int& tIndex, int& endIndex) {
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
            SkScalar yIntercept = t == 1 ? fPts[fVerb].fY : yAtT(t);
            if (topY > yIntercept) {
                topY = yIntercept;
                firstT = lastT = index;
            } else if (topY == yIntercept) {
                lastT = index;
            }
        }
        // if there's only a pair of segments, go with the endpoint chosen above
        if (firstT == lastT) {
            tIndex = firstT;
            endIndex = firstT > 0 ? tIndex - 1 : tIndex + 1;
            return this;
        }
        // sort the edges to find the leftmost
        SkPoint startLoc; // OPTIMIZATION: store this in the t span?
        const Span* startSpan = &fTs[firstT];
        xyAtT(startSpan->fT, &startLoc);
        SkPoint endLoc;
        bool nextCo;
        int end = nextSpan(firstT, 1, startLoc, startSpan, &endLoc, nextCo);
        if (end == -1) {
            end = nextSpan(firstT, -1, startLoc, startSpan, &endLoc, nextCo);
            SkASSERT(end != -1);
        }
        // if the topmost T is not on end, or is three-way or more, find left
        // look for left-ness from tLeft to firstT (matching y of other)
        SkTDArray<Angle> angles;
        SkASSERT(firstT - end != 0);
        addTwoAngles(end, firstT, endLoc, &fTs[firstT], nextCo, angles);
        buildAngles(firstT, lastT, 1, startLoc, angles);
        SkTDArray<Angle*> sorted;
        sortAngles(angles, sorted);
        Segment* leftSegment = sorted[0]->segment();
        tIndex = sorted[0]->end();
        endIndex = sorted[0]->start();
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
                }
            }
        }
    }
    
    // OPTIMIZATION: uses tail recursion. Unwise?
    void innerCoincidentChase(int step, Segment* other) {
        // find other at index
        SkASSERT(!done());
        const Span* start = NULL;
        const Span* end = NULL;
        int index, startIndex, endIndex;
        int count = fTs.count();
        for (index = 0; index < count; ++index) {
            const Span& span = fTs[index];
            if (!span.fCoincident || span.fOther != other) {
                continue;
            }
            if (!start) {
                if (span.fDone) {
                    continue;
                }
                startIndex = index;
                start = &span;
            } else {
                SkASSERT(!end);
                endIndex = index;
                end = &span;
            }
        }
        if (!end) {
            return;
        }
        Segment* next;
        Segment* nextOther;
        if (step < 0) {
            next = start->fT <= 0 ? NULL : this;
            nextOther = other->fTs[start->fOtherIndex].fT >= 1 ? NULL : other;
        } else {
            next = end->fT >= 1 ? NULL : this;
            nextOther = other->fTs[end->fOtherIndex].fT <= 0 ? NULL : other;
        }
        SkASSERT(!next || !nextOther);
        for (index = 0; index < count; ++index) {
            const Span& span = fTs[index];
            if (span.fCoincident || span.fOther == other) {
                continue;
            }
            bool checkNext = !next && (step < 0 ? span.fT <= 0
                && span.fOtherT >= 1 : span.fT >= 1 && span.fOtherT <= 0);
            bool checkOther = !nextOther && (step < 0 ? span.fT == start->fT
                && span.fOtherT <= 0 : span.fT == end->fT && span.fOtherT >= 1);
            if (!checkNext && !checkOther) {
                continue;
            }
            Segment* oSegment = span.fOther;
            if (oSegment->done()) {
                continue;
            }
            int oCount = oSegment->fTs.count();
            for (int oIndex = 0; oIndex < oCount; ++oIndex) {
                const Span& oSpan = oSegment->fTs[oIndex];
                if (oSpan.fT > 0 && oSpan.fT < 1) {
                    continue;
                }
                if (!oSpan.fCoincident) {
                    continue;
                }
                if (checkNext && (oSpan.fT <= 0 ^ step < 0)) { 
                    next = oSegment;
                    checkNext = false;
                }
                if (checkOther && (oSpan.fT >= 1 ^ step < 0)) {
                    nextOther = oSegment;
                    checkOther = false;
                }
            }
        }
        markDone(SkMin32(startIndex, endIndex), 0);
        other->markDone(SkMin32(start->fOtherIndex, end->fOtherIndex), 0);
        if (next && nextOther) {
            next->innerCoincidentChase(step, nextOther);
        }
    }
    
    // OPTIMIZATION: uses tail recursion. Unwise?
    void innerChase(int index, int step, int winding) {
        SkPoint loc; // OPTIMIZATION: store this in the t span?
        bool coincident;
        int end = nextSpan(index, step, &loc, coincident);
        bool many;
        lastSpan(end, step, loc, fTs[end].fT, coincident, &many);
        if (many) {
            return;
        }
        Span* endSpan = &fTs[end];
        Segment* other = endSpan->fOther;
        index = endSpan->fOtherIndex;
        int otherEnd = other->nextSpan(index, step, &loc, coincident);
        other->innerChase(index, step, winding);
        other->markDone(SkMin32(index, otherEnd), winding);
    }
    
    void init(const SkPoint pts[], SkPath::Verb verb) {
        fPts = pts;
        fVerb = verb;
        fDoneSpans = 0;
        fCoincident = 0;
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

    // last does not check for done spans because done is only set for the start
    int lastSpan(int end, int step, const SkPoint& startLoc,
            double startT, bool& coincident, bool* manyPtr = NULL) const {
        int last = end;
        int count = fTs.count();
        SkPoint lastLoc;
        int found = 0;
        do {
            end = last;
            if (fTs[end].fCoincident == -step) {
                coincident = true;
            }
            if (step > 0 ? ++last >= count : --last < 0) {
                break;
            }
            const Span& lastSpan = fTs[last];
            if (lastSpan.fT == startT) {
                ++found;
                continue;
            }
            xyAtT(lastSpan.fT, &lastLoc);
            if (startLoc != lastLoc) {
                break;
            }
            ++found;
        } while (true);
        if (manyPtr) {
            *manyPtr = found > 0;
        }
        return end;
    }

    SkScalar leftMost(int start, int end) const {
        return (*SegmentLeftMost[fVerb])(fPts, fTs[start].fT, fTs[end].fT);
    }
    
    void markAndChaseCoincident(int index, int endIndex, Segment* other) {
        int step = SkSign32(endIndex - index);
        innerCoincidentChase(step, other);
    }

    // this span is excluded by the winding rule -- chase the ends
    // as long as they are unambiguous to mark connections as done
    // and give them the same winding value
    void markAndChaseWinding(const Angle* angle, int winding) {
        int index = angle->start();
        int endIndex = angle->end();
        int step = SkSign32(endIndex - index);
        innerChase(index, step, winding);
        markDone(SkMin32(index, endIndex), winding);
    }
    
    // FIXME: this should also mark spans with equal (x,y)
    void markDone(int index, int winding) {
        SkASSERT(!done());
        double referenceT = fTs[index].fT;
        int lesser = index;
        while (--lesser >= 0 && referenceT == fTs[lesser].fT) {
            Span& span = fTs[lesser];
            SkASSERT(!span.fDone);
            fTs[lesser].fDone = true;
            SkASSERT(!span.fWinding || span.fWinding == winding);
            span.fWinding = winding;
            fDoneSpans++;
        }
        do {
            Span& span = fTs[index];
            SkASSERT(!span.fDone);
            span.fDone = true;
            SkASSERT(!span.fWinding || span.fWinding == winding);
            span.fWinding = winding;
            fDoneSpans++;
        } while (++index < fTs.count() && referenceT == fTs[index].fT);
    }

    // note the assert logic looks for unexpected done of span start
    // FIXME: compute fromLoc on the fly
    int nextSpan(int from, int step, const SkPoint& fromLoc,
            const Span* fromSpan, SkPoint* toLoc, bool& coincident) const {
        coincident = false;
        SkASSERT(!done());
        int count = fTs.count();
        int to = from;
        while (step > 0 ? ++to < count : --to >= 0) {
            Span* span = &fTs[to];
            if (span->fCoincident == step) {
                coincident = true;
            }
            if (fromSpan->fT == span->fT) {
                continue;
            }
            SkPoint loc;
            xyAtT(span->fT, &loc);
            if (fromLoc == loc) {
                continue;
            }
            SkASSERT(step < 0 || !fTs[from].fDone);
            SkASSERT(step > 0 || !span->fDone);
            if (toLoc) {
                *toLoc = loc;
            }
            return to;
        }
        return -1;
    }

    int nextSpan(int from, int step, SkPoint* toLoc, bool& coincident) const {
        const Span& fromSpan = fTs[from];
        coincident = false;
        SkASSERT(!done());
        int count = fTs.count();
        int to = from;
        SkPoint fromLoc;
        fromLoc.fX = SK_ScalarNaN;
        while (step > 0 ? ++to < count : --to >= 0) {
            const Span& span = fTs[to];
            if (span.fCoincident == step) {
                coincident = true;
            }
            if (fromSpan.fT == span.fT) {
                continue;
            }
            SkPoint loc;
            xyAtT(span.fT, &loc);
            if (SkScalarIsNaN(fromLoc.fX)) {
                xyAtT(fromSpan.fT, &fromLoc);
            }
            if (fromLoc == loc) {
                continue;
            }
            SkASSERT(step < 0 || !fromSpan.fDone);
            SkASSERT(step > 0 || !span.fDone);
            if (toLoc) {
                *toLoc = loc;
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
    double t(int tIndex) const {
        SkASSERT(tIndex >= 0);
        SkASSERT(tIndex < fTs.count());
        return fTs[tIndex].fT;
    }
    
    void updatePts(const SkPoint pts[]) {
        fPts = pts;
    }

    SkPath::Verb verb() const {
        return fVerb;
    }
    
    bool winding(const Angle* angle) {
        int start = angle->start();
        int end = angle->end();
        int index = SkMin32(start, end);
        Span& span = fTs[index];
        return span.fWinding;
    }

    SkScalar xAtT(double t) const {
        SkASSERT(t >= 0 && t <= 1);
        return (*SegmentXAtT[fVerb])(fPts, t);
    }

    void xyAtT(double t, SkPoint* pt) const {
        SkASSERT(t >= 0 && t <= 1);
        (*SegmentXYAtT[fVerb])(fPts, t, pt);
    }

    SkScalar yAtT(double t) const {
        SkASSERT(t >= 0 && t <= 1);
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
    int fDoneSpans; // used for quick check that segment is finished
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

    int addLine(const SkPoint pts[2]) {
        fSegments.push_back().addLine(pts);
        return fSegments.count();
    }

    int addQuad(const SkPoint pts[3]) {
        fSegments.push_back().addQuad(pts);
        fContainsCurves = true;
        return fSegments.count();
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
                    finalCurveEnd = pointsPtr++;
                    *fExtra.append() = -1; // start new contour
                }
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
    if (fCurrentContour && !fCurrentContour->fSegments.count()) {
        fContours.pop_back();
    }
    // correct pointers in contours since fReducePts may have moved as it grew
    int cIndex = 0;
    fCurrentContour = &fContours[0];
    int extraCount = fExtra.count();
    SkASSERT(fExtra[0] == -1);
    int eIndex = 0;
    int rIndex = 0;
    while (++eIndex < extraCount) {
        int offset = fExtra[eIndex];
        if (offset < 0) {
            fCurrentContour = &fContours[++cIndex];
            continue;
        }
        Segment& segment = fCurrentContour->fSegments[offset - 1];
        segment.updatePts(&fReducePts[rIndex]);
        rIndex += segment.verb() + 1;
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

    // FIXME: does it make sense to write otherIndex now if we're going to
    // fix it up later?
    void addOtherT(int index, double otherT, int otherIndex) {
        fContour->fSegments[fIndex].addOtherT(index, otherT, otherIndex);
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
    SkDebugf(" wnTs[0]=%g (%g,%g, %g,%g) (%g,%g)\n",
            wnTs[0], wn.pts()[0].fX, wn.pts()[0].fY,
            wn.pts()[1].fX, wn.pts()[1].fY, wnOutPt.fX, wnOutPt.fY);
    if (pts == 2) {
        SkDebugf(" wnTs[1]=%g", wnTs[1]);
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
            // in addition to recording T values, record matching segment
            int coincident = pts == 2 && wn.segmentType() <= Work::kLine_Segment
                    && wt.segmentType() <= Work::kLine_Segment ? -1 :0;
            for (int pt = 0; pt < pts; ++pt) {
                SkASSERT(ts.fT[0][pt] >= 0 && ts.fT[0][pt] <= 1);
                SkASSERT(ts.fT[1][pt] >= 0 && ts.fT[1][pt] <= 1);
                int testTAt = wt.addT(ts.fT[swap][pt], wn, coincident);
                int nextTAt = wn.addT(ts.fT[!swap][pt], wt, coincident);
                wt.addOtherT(testTAt, ts.fT[!swap][pt], nextTAt);
                wn.addOtherT(nextTAt, ts.fT[swap][pt], testTAt);
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

    
// OPTIMIZATION: not crazy about linear search here to find top active y.
// seems like we should break down and do the sort, or maybe sort each
// contours' segments? 
// Once the segment array is built, there's no reason I can think of not to
// sort it in Y. hmmm
static Segment* findTopContour(SkTDArray<Contour*>& contourList,
        int contourCount) {
    int cIndex = 0;
    Segment* topStart;
    do {
        Contour* topContour = contourList[cIndex];
        topStart = topContour->topSegment();
    } while (!topStart && ++cIndex < contourCount);
    if (!topStart) {
        return NULL;
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
    return topStart;
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
    int contourCount = contourList.count();
    int winding = 0; // there are no contours outside this one
    do {
        Segment* topStart = findTopContour(contourList, contourCount);
        if (!topStart) {
            break;
        }
        // Start at the top. Above the top is outside, below is inside.
        // follow edges to intersection by changing the index by direction.
        int index, endIndex;
        Segment* topSegment = topStart->findTop(index, endIndex);
        Segment* current = topSegment;
        winding += SkSign32(index - endIndex);
        bool first = true;
        do {
            SkASSERT(!current->done());
            int nextStart, nextEnd;
            Segment* next = current->findNext(winding, index, endIndex,
                    nextStart, nextEnd);
            if (!next) {
                break;
            }
            if (first) {
                current->addMoveTo(index, simple);
                first = false;
            }
            current->addCurveTo(index, endIndex, simple);
            current = next;
            index = nextStart;
            endIndex = nextEnd;
        } while (current != topSegment);
        if (!first) {
    #if DEBUG_PATH_CONSTRUCTION
            SkDebugf("%s close\n", __FUNCTION__);
    #endif
            simple.close();
        }
    } while (true);
    // FIXME: more work to be done for contained (but not intersecting)
    //  segments
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
    fixOtherTIndex(contourList);
    // eat through coincident edges
    coincidenceCheck(contourList, winding);
    // construct closed contours
    bridge(contourList, simple);
}

