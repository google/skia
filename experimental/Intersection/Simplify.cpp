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

#define PIN_ADD_T 0
#define TRY_ROTATE 1
#define ONE_PASS_COINCIDENCE_CHECK 0
#define APPROXIMATE_CUBICS 1
#define COMPACT_DEBUG_SORT 0

#define DEBUG_UNUSED 0 // set to expose unused functions

#if FORCE_RELEASE || defined SK_RELEASE

const bool gRunTestsInOneThread = false;

#define DEBUG_ACTIVE_OP 0
#define DEBUG_ACTIVE_SPANS 0
#define DEBUG_ACTIVE_SPANS_SHORT_FORM 0
#define DEBUG_ADD_INTERSECTING_TS 0
#define DEBUG_ADD_T_PAIR 0
#define DEBUG_ANGLE 0
#define DEBUG_AS_C_CODE 1
#define DEBUG_ASSEMBLE 0
#define DEBUG_CONCIDENT 0
#define DEBUG_CROSS 0
#define DEBUG_FLOW 0
#define DEBUG_MARK_DONE 0
#define DEBUG_PATH_CONSTRUCTION 0
#define DEBUG_SHOW_WINDING 0
#define DEBUG_SORT 0
#define DEBUG_SWAP_TOP 0
#define DEBUG_UNSORTABLE 0
#define DEBUG_WIND_BUMP 0
#define DEBUG_WINDING 0
#define DEBUG_WINDING_AT_T 0

#else

const bool gRunTestsInOneThread = true;

#define DEBUG_ACTIVE_OP 1
#define DEBUG_ACTIVE_SPANS 1
#define DEBUG_ACTIVE_SPANS_SHORT_FORM 0
#define DEBUG_ADD_INTERSECTING_TS 1
#define DEBUG_ADD_T_PAIR 1
#define DEBUG_ANGLE 1
#define DEBUG_AS_C_CODE 1
#define DEBUG_ASSEMBLE 1
#define DEBUG_CONCIDENT 1
#define DEBUG_CROSS 0
#define DEBUG_FLOW 1
#define DEBUG_MARK_DONE 1
#define DEBUG_PATH_CONSTRUCTION 1
#define DEBUG_SHOW_WINDING 0
#define DEBUG_SORT 1
#define DEBUG_SWAP_TOP 1
#define DEBUG_UNSORTABLE 1
#define DEBUG_WIND_BUMP 0
#define DEBUG_WINDING 1
#define DEBUG_WINDING_AT_T 1

#endif

#define DEBUG_DUMP (DEBUG_ACTIVE_OP | DEBUG_ACTIVE_SPANS | DEBUG_CONCIDENT | DEBUG_SORT | \
        DEBUG_PATH_CONSTRUCTION)

#if DEBUG_AS_C_CODE
#define CUBIC_DEBUG_STR "{{%1.17g,%1.17g}, {%1.17g,%1.17g}, {%1.17g,%1.17g}, {%1.17g,%1.17g}}"
#define QUAD_DEBUG_STR  "{{%1.17g,%1.17g}, {%1.17g,%1.17g}, {%1.17g,%1.17g}}"
#define LINE_DEBUG_STR  "{{%1.17g,%1.17g}, {%1.17g,%1.17g}}"
#define PT_DEBUG_STR "{{%1.17g,%1.17g}}"
#else
#define CUBIC_DEBUG_STR "(%1.9g,%1.9g %1.9g,%1.9g %1.9g,%1.9g %1.9g,%1.9g)"
#define QUAD_DEBUG_STR  "(%1.9g,%1.9g %1.9g,%1.9g %1.9g,%1.9g)"
#define LINE_DEBUG_STR  "(%1.9g,%1.9g %1.9g,%1.9g)"
#define PT_DEBUG_STR "(%1.9g,%1.9g)"
#endif
#define T_DEBUG_STR(t, n) #t "[" #n "]=%1.9g"
#define TX_DEBUG_STR(t) #t "[%d]=%1.9g"
#define CUBIC_DEBUG_DATA(c) c[0].fX, c[0].fY, c[1].fX, c[1].fY, c[2].fX, c[2].fY, c[3].fX, c[3].fY
#define QUAD_DEBUG_DATA(q)  q[0].fX, q[0].fY, q[1].fX, q[1].fY, q[2].fX, q[2].fY
#define LINE_DEBUG_DATA(l)  l[0].fX, l[0].fY, l[1].fX, l[1].fY
#define PT_DEBUG_DATA(i, n) i.fPt[n].x, i.fPt[n].y

#if DEBUG_DUMP
static const char* kLVerbStr[] = {"", "line", "quad", "cubic"};
// static const char* kUVerbStr[] = {"", "Line", "Quad", "Cubic"};
static int gContourID;
static int gSegmentID;
#endif

#if DEBUG_SORT || DEBUG_SWAP_TOP
static int gDebugSortCountDefault = SK_MaxS32;
static int gDebugSortCount;
#endif

#if DEBUG_ACTIVE_OP
static const char* kShapeOpStr[] = {"diff", "sect", "union", "xor"};
#endif

#ifndef DEBUG_TEST
#define DEBUG_TEST 0
#endif

#define MAKE_CONST_LINE(line, pts) \
    const _Line line = {{pts[0].fX, pts[0].fY}, {pts[1].fX, pts[1].fY}}
#define MAKE_CONST_QUAD(quad, pts) \
    const Quadratic quad = {{pts[0].fX, pts[0].fY}, {pts[1].fX, pts[1].fY}, \
            {pts[2].fX, pts[2].fY}}
#define MAKE_CONST_CUBIC(cubic, pts) \
    const Cubic cubic = {{pts[0].fX, pts[0].fY}, {pts[1].fX, pts[1].fY}, \
            {pts[2].fX, pts[2].fY}, {pts[3].fX, pts[3].fY}}

static int LineIntersect(const SkPoint a[2], const SkPoint b[2],
        Intersections& intersections) {
    MAKE_CONST_LINE(aLine, a);
    MAKE_CONST_LINE(bLine, b);
    return intersect(aLine, bLine, intersections);
}

static int QuadLineIntersect(const SkPoint a[3], const SkPoint b[2],
        Intersections& intersections) {
    MAKE_CONST_QUAD(aQuad, a);
    MAKE_CONST_LINE(bLine, b);
    return intersect(aQuad, bLine, intersections);
}

static int CubicLineIntersect(const SkPoint a[4], const SkPoint b[2],
        Intersections& intersections) {
    MAKE_CONST_CUBIC(aCubic, a);
    MAKE_CONST_LINE(bLine, b);
    return intersect(aCubic, bLine, intersections);
}

static int QuadIntersect(const SkPoint a[3], const SkPoint b[3],
        Intersections& intersections) {
    MAKE_CONST_QUAD(aQuad, a);
    MAKE_CONST_QUAD(bQuad, b);
#define TRY_QUARTIC_SOLUTION 1
#if TRY_QUARTIC_SOLUTION
    intersect2(aQuad, bQuad, intersections);
#else
    intersect(aQuad, bQuad, intersections);
#endif
    return intersections.fUsed;
}

#if APPROXIMATE_CUBICS
static int CubicQuadIntersect(const SkPoint a[4], const SkPoint b[3],
        Intersections& intersections) {
    MAKE_CONST_CUBIC(aCubic, a);
    MAKE_CONST_QUAD(bQuad, b);
    return intersect(aCubic, bQuad, intersections);
}
#endif

static int CubicIntersect(const SkPoint a[4], const SkPoint b[4], Intersections& intersections) {
    MAKE_CONST_CUBIC(aCubic, a);
    MAKE_CONST_CUBIC(bCubic, b);
#if APPROXIMATE_CUBICS
    intersect3(aCubic, bCubic, intersections);
#else
    intersect(aCubic, bCubic, intersections);
#endif
    return intersections.fUsed;
}

static int CubicIntersect(const SkPoint a[4], Intersections& intersections) {
    MAKE_CONST_CUBIC(aCubic, a);
    return intersect(aCubic, intersections);
}

static int HLineIntersect(const SkPoint a[2], SkScalar left, SkScalar right,
        SkScalar y, bool flipped, Intersections& intersections) {
    MAKE_CONST_LINE(aLine, a);
    return horizontalIntersect(aLine, left, right, y, flipped, intersections);
}

static int HQuadIntersect(const SkPoint a[3], SkScalar left, SkScalar right,
        SkScalar y, bool flipped, Intersections& intersections) {
    MAKE_CONST_QUAD(aQuad, a);
    return horizontalIntersect(aQuad, left, right, y, flipped, intersections);
}

static int HCubicIntersect(const SkPoint a[4], SkScalar left, SkScalar right,
        SkScalar y, bool flipped, Intersections& intersections) {
    MAKE_CONST_CUBIC(aCubic, a);
    return horizontalIntersect(aCubic, left, right, y, flipped, intersections);
}

static int (* const HSegmentIntersect[])(const SkPoint [], SkScalar ,
        SkScalar , SkScalar , bool , Intersections& ) = {
    NULL,
    HLineIntersect,
    HQuadIntersect,
    HCubicIntersect
};

static int VLineIntersect(const SkPoint a[2], SkScalar top, SkScalar bottom,
        SkScalar x, bool flipped, Intersections& intersections) {
    MAKE_CONST_LINE(aLine, a);
    return verticalIntersect(aLine, top, bottom, x, flipped, intersections);
}

static int VQuadIntersect(const SkPoint a[3], SkScalar top, SkScalar bottom,
        SkScalar x, bool flipped, Intersections& intersections) {
    MAKE_CONST_QUAD(aQuad, a);
    return verticalIntersect(aQuad, top, bottom, x, flipped, intersections);
}

static int VCubicIntersect(const SkPoint a[4], SkScalar top, SkScalar bottom,
        SkScalar x, bool flipped, Intersections& intersections) {
    MAKE_CONST_CUBIC(aCubic, a);
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
    MAKE_CONST_LINE(line, a);
    double x, y;
    xy_at_t(line, t, x, y);
    out->fX = SkDoubleToScalar(x);
    out->fY = SkDoubleToScalar(y);
}

static void LineXYAtT(const SkPoint a[2], double t, _Point* out) {
    MAKE_CONST_LINE(line, a);
    xy_at_t(line, t, out->x, out->y);
}

static void QuadXYAtT(const SkPoint a[3], double t, SkPoint* out) {
    MAKE_CONST_QUAD(quad, a);
    double x, y;
    xy_at_t(quad, t, x, y);
    out->fX = SkDoubleToScalar(x);
    out->fY = SkDoubleToScalar(y);
}

static void QuadXYAtT(const SkPoint a[3], double t, _Point* out) {
    MAKE_CONST_QUAD(quad, a);
    xy_at_t(quad, t, out->x, out->y);
}

static void CubicXYAtT(const SkPoint a[4], double t, SkPoint* out) {
    MAKE_CONST_CUBIC(cubic, a);
    double x, y;
    xy_at_t(cubic, t, x, y);
    out->fX = SkDoubleToScalar(x);
    out->fY = SkDoubleToScalar(y);
}

static void CubicXYAtT(const SkPoint a[4], double t, _Point* out) {
    MAKE_CONST_CUBIC(cubic, a);
    xy_at_t(cubic, t, out->x, out->y);
}

static void (* const SegmentXYAtT[])(const SkPoint [], double , SkPoint* ) = {
    NULL,
    LineXYAtT,
    QuadXYAtT,
    CubicXYAtT
};

static void (* const SegmentXYAtT2[])(const SkPoint [], double , _Point* ) = {
    NULL,
    LineXYAtT,
    QuadXYAtT,
    CubicXYAtT
};

static SkScalar LineXAtT(const SkPoint a[2], double t) {
    MAKE_CONST_LINE(aLine, a);
    double x;
    xy_at_t(aLine, t, x, *(double*) 0);
    return SkDoubleToScalar(x);
}

static SkScalar QuadXAtT(const SkPoint a[3], double t) {
    MAKE_CONST_QUAD(quad, a);
    double x;
    xy_at_t(quad, t, x, *(double*) 0);
    return SkDoubleToScalar(x);
}

static SkScalar CubicXAtT(const SkPoint a[4], double t) {
    MAKE_CONST_CUBIC(cubic, a);
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
    MAKE_CONST_LINE(aLine, a);
    double y;
    xy_at_t(aLine, t, *(double*) 0, y);
    return SkDoubleToScalar(y);
}

static SkScalar QuadYAtT(const SkPoint a[3], double t) {
    MAKE_CONST_QUAD(quad, a);
    double y;
    xy_at_t(quad, t, *(double*) 0, y);
    return SkDoubleToScalar(y);
}

static SkScalar CubicYAtT(const SkPoint a[4], double t) {
    MAKE_CONST_CUBIC(cubic, a);
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
    MAKE_CONST_QUAD(quad, a);
    double x = dx_at_t(quad, t);
    return SkDoubleToScalar(x);
}

static SkScalar CubicDXAtT(const SkPoint a[4], double t) {
    MAKE_CONST_CUBIC(cubic, a);
    double x = dx_at_t(cubic, t);
    return SkDoubleToScalar(x);
}

static SkScalar (* const SegmentDXAtT[])(const SkPoint [], double ) = {
    NULL,
    LineDXAtT,
    QuadDXAtT,
    CubicDXAtT
};

static SkScalar LineDYAtT(const SkPoint a[2], double ) {
    return a[1].fY - a[0].fY;
}

static SkScalar QuadDYAtT(const SkPoint a[3], double t) {
    MAKE_CONST_QUAD(quad, a);
    double y = dy_at_t(quad, t);
    return SkDoubleToScalar(y);
}

static SkScalar CubicDYAtT(const SkPoint a[4], double t) {
    MAKE_CONST_CUBIC(cubic, a);
    double y = dy_at_t(cubic, t);
    return SkDoubleToScalar(y);
}

static SkScalar (* const SegmentDYAtT[])(const SkPoint [], double ) = {
    NULL,
    LineDYAtT,
    QuadDYAtT,
    CubicDYAtT
};

static SkVector LineDXDYAtT(const SkPoint a[2], double ) {
    return a[1] - a[0];
}

static SkVector QuadDXDYAtT(const SkPoint a[3], double t) {
    MAKE_CONST_QUAD(quad, a);
    _Vector v = dxdy_at_t(quad, t);
    return v.asSkVector();
}

static SkVector CubicDXDYAtT(const SkPoint a[4], double t) {
    MAKE_CONST_CUBIC(cubic, a);
    _Vector v = dxdy_at_t(cubic, t);
    return v.asSkVector();
}

static SkVector (* const SegmentDXDYAtT[])(const SkPoint [], double ) = {
    NULL,
    LineDXDYAtT,
    QuadDXDYAtT,
    CubicDXDYAtT
};

static void LineSubDivide(const SkPoint a[2], double startT, double endT,
        SkPoint sub[2]) {
    MAKE_CONST_LINE(aLine, a);
    _Line dst;
    sub_divide(aLine, startT, endT, dst);
    sub[0].fX = SkDoubleToScalar(dst[0].x);
    sub[0].fY = SkDoubleToScalar(dst[0].y);
    sub[1].fX = SkDoubleToScalar(dst[1].x);
    sub[1].fY = SkDoubleToScalar(dst[1].y);
}

static void QuadSubDivide(const SkPoint a[3], double startT, double endT,
        SkPoint sub[3]) {
    MAKE_CONST_QUAD(aQuad, a);
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
    MAKE_CONST_CUBIC(aCubic, a);
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

static void LineSubDivideHD(const SkPoint a[2], double startT, double endT, _Line& dst) {
    MAKE_CONST_LINE(aLine, a);
    sub_divide(aLine, startT, endT, dst);
}

static void QuadSubDivideHD(const SkPoint a[3], double startT, double endT, Quadratic& dst) {
    MAKE_CONST_QUAD(aQuad, a);
    sub_divide(aQuad, startT, endT, dst);
}

static void CubicSubDivideHD(const SkPoint a[4], double startT, double endT, Cubic& dst) {
    MAKE_CONST_CUBIC(aCubic, a);
    sub_divide(aCubic, startT, endT, dst);
}

static SkPoint QuadTop(const SkPoint a[3], double startT, double endT) {
    MAKE_CONST_QUAD(quad, a);
    _Point topPt = top(quad, startT, endT);
    return topPt.asSkPoint();
}

static SkPoint CubicTop(const SkPoint a[3], double startT, double endT) {
    MAKE_CONST_CUBIC(cubic, a);
    _Point topPt = top(cubic, startT, endT);
    return topPt.asSkPoint();
}

static SkPoint (* SegmentTop[])(const SkPoint[], double , double ) = {
    NULL,
    NULL,
    QuadTop,
    CubicTop
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
    MAKE_CONST_QUAD(aQuad, a);
    Quadratic dst;
    int order = reduceOrder(aQuad, dst, kReduceOrder_TreatAsFill);
    if (order == 2) { // quad became line
        for (int index = 0; index < order; ++index) {
            SkPoint* pt = reducePts.append();
            pt->fX = SkDoubleToScalar(dst[index].x);
            pt->fY = SkDoubleToScalar(dst[index].y);
        }
    }
    return (SkPath::Verb) (order - 1);
}

static SkPath::Verb CubicReduceOrder(const SkPoint a[4],
        SkTDArray<SkPoint>& reducePts) {
    MAKE_CONST_CUBIC(aCubic, a);
    Cubic dst;
    int order = reduceOrder(aCubic, dst, kReduceOrder_QuadraticsAllowed, kReduceOrder_TreatAsFill);
    if (order == 2 || order == 3) { // cubic became line or quad
        for (int index = 0; index < order; ++index) {
            SkPoint* pt = reducePts.append();
            pt->fX = SkDoubleToScalar(dst[index].x);
            pt->fY = SkDoubleToScalar(dst[index].y);
        }
    }
    return (SkPath::Verb) (order - 1);
}

static bool QuadIsLinear(const SkPoint a[3]) {
    MAKE_CONST_QUAD(aQuad, a);
    return isLinear(aQuad, 0, 2);
}

static bool CubicIsLinear(const SkPoint a[4]) {
    MAKE_CONST_CUBIC(aCubic, a);
    return isLinear(aCubic, 0, 3);
}

static SkScalar LineLeftMost(const SkPoint a[2], double startT, double endT) {
    MAKE_CONST_LINE(aLine, a);
    double x[2];
    xy_at_t(aLine, startT, x[0], *(double*) 0);
    xy_at_t(aLine, endT, x[1], *(double*) 0);
    return SkMinScalar((float) x[0], (float) x[1]);
}

static SkScalar QuadLeftMost(const SkPoint a[3], double startT, double endT) {
    MAKE_CONST_QUAD(aQuad, a);
    return (float) leftMostT(aQuad, startT, endT);
}

static SkScalar CubicLeftMost(const SkPoint a[4], double startT, double endT) {
    MAKE_CONST_CUBIC(aCubic, a);
    return (float) leftMostT(aCubic, startT, endT);
}

static SkScalar (* const SegmentLeftMost[])(const SkPoint [], double , double) = {
    NULL,
    LineLeftMost,
    QuadLeftMost,
    CubicLeftMost
};

#if 0 // currently unused
static int QuadRayIntersect(const SkPoint a[3], const SkPoint b[2],
        Intersections& intersections) {
    MAKE_CONST_QUAD(aQuad, a);
    MAKE_CONST_LINE(bLine, b);
    return intersectRay(aQuad, bLine, intersections);
}
#endif

static int QuadRayIntersect(const SkPoint a[3], const _Line& bLine, Intersections& intersections) {
    MAKE_CONST_QUAD(aQuad, a);
    return intersectRay(aQuad, bLine, intersections);
}

static int CubicRayIntersect(const SkPoint a[3], const _Line& bLine, Intersections& intersections) {
    MAKE_CONST_CUBIC(aCubic, a);
    return intersectRay(aCubic, bLine, intersections);
}

static int (* const SegmentRayIntersect[])(const SkPoint [], const _Line& , Intersections&) = {
    NULL,
    NULL,
    QuadRayIntersect,
    CubicRayIntersect
};



static bool LineVertical(const SkPoint a[2], double startT, double endT) {
    MAKE_CONST_LINE(aLine, a);
    double x[2];
    xy_at_t(aLine, startT, x[0], *(double*) 0);
    xy_at_t(aLine, endT, x[1], *(double*) 0);
    return AlmostEqualUlps((float) x[0], (float) x[1]);
}

static bool QuadVertical(const SkPoint a[3], double startT, double endT) {
    SkPoint dst[3];
    QuadSubDivide(a, startT, endT, dst);
    return AlmostEqualUlps(dst[0].fX, dst[1].fX) && AlmostEqualUlps(dst[1].fX, dst[2].fX);
}

static bool CubicVertical(const SkPoint a[4], double startT, double endT) {
    SkPoint dst[4];
    CubicSubDivide(a, startT, endT, dst);
    return AlmostEqualUlps(dst[0].fX, dst[1].fX) && AlmostEqualUlps(dst[1].fX, dst[2].fX)
            && AlmostEqualUlps(dst[2].fX, dst[3].fX);
}

static bool (* const SegmentVertical[])(const SkPoint [], double , double) = {
    NULL,
    LineVertical,
    QuadVertical,
    CubicVertical
};

class Segment;

struct Span {
    Segment* fOther;
    mutable SkPoint fPt; // lazily computed as needed
    double fT;
    double fOtherT; // value at fOther[fOtherIndex].fT
    int fOtherIndex;  // can't be used during intersection
    int fWindSum; // accumulated from contours surrounding this one.
    int fOppSum; // for binary operators: the opposite winding sum
    int fWindValue; // 0 == canceled; 1 == normal; >1 == coincident
    int fOppValue; // normally 0 -- when binary coincident edges combine, opp value goes here
    bool fDone; // if set, this span to next higher T has been processed
    bool fUnsortableStart; // set when start is part of an unsortable pair
    bool fUnsortableEnd; // set when end is part of an unsortable pair
    bool fTiny; // if set, span may still be considered once for edge following
    bool fLoop; // set when a cubic loops back to this point
};

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

    /*(
    for quads and cubics, set up a parameterized line (e.g. LineParameters )
    for points [0] to [1]. See if point [2] is on that line, or on one side
    or the other. If it both quads' end points are on the same side, choose
    the shorter tangent. If the tangents are equal, choose the better second
    tangent angle

    maybe I could set up LineParameters lazily
    */
    bool operator<(const Angle& rh) const {
        double y = dy();
        double ry = rh.dy();
        if ((y < 0) ^ (ry < 0)) { // OPTIMIZATION: better to use y * ry < 0 ?
            return y < 0;
        }
        double x = dx();
        double rx = rh.dx();
        if (y == 0 && ry == 0 && x * rx < 0) {
            return x < rx;
        }
        double x_ry = x * ry;
        double rx_y = rx * y;
        double cmp = x_ry - rx_y;
        if (!approximately_zero(cmp)) {
            return cmp < 0;
        }
        if (approximately_zero(x_ry) && approximately_zero(rx_y)
                && !approximately_zero_squared(cmp)) {
            return cmp < 0;
        }
        // at this point, the initial tangent line is coincident
        // see if edges curl away from each other
        if (fSide * rh.fSide <= 0 && (!approximately_zero(fSide)
                || !approximately_zero(rh.fSide))) {
            // FIXME: running demo will trigger this assertion
            // (don't know if commenting out will trigger further assertion or not)
            // commenting it out allows demo to run in release, though
     //       SkASSERT(fSide != rh.fSide);
            return fSide < rh.fSide;
        }
        // see if either curve can be lengthened and try the tangent compare again
        if (cmp && (*fSpans)[fEnd].fOther != rh.fSegment // tangents not absolutely identical
                && (*rh.fSpans)[rh.fEnd].fOther != fSegment) { // and not intersecting
            Angle longer = *this;
            Angle rhLonger = rh;
            if (longer.lengthen() | rhLonger.lengthen()) {
                return longer < rhLonger;
            }
    #if 0
            // what if we extend in the other direction?
            longer = *this;
            rhLonger = rh;
            if (longer.reverseLengthen() | rhLonger.reverseLengthen()) {
                return longer < rhLonger;
            }
    #endif
        }
        if ((fVerb == SkPath::kLine_Verb && approximately_zero(x) && approximately_zero(y))
                || (rh.fVerb == SkPath::kLine_Verb
                && approximately_zero(rx) && approximately_zero(ry))) {
            // See general unsortable comment below. This case can happen when
            // one line has a non-zero change in t but no change in x and y.
            fUnsortable = true;
            rh.fUnsortable = true;
            return this < &rh; // even with no solution, return a stable sort
        }
        if ((*rh.fSpans)[SkMin32(rh.fStart, rh.fEnd)].fTiny
                || (*fSpans)[SkMin32(fStart, fEnd)].fTiny) {
            fUnsortable = true;
            rh.fUnsortable = true;
            return this < &rh; // even with no solution, return a stable sort
        }
        SkASSERT(fVerb >= SkPath::kQuad_Verb);
        SkASSERT(rh.fVerb >= SkPath::kQuad_Verb);
        // FIXME: until I can think of something better, project a ray from the
        // end of the shorter tangent to midway between the end points
        // through both curves and use the resulting angle to sort
        // FIXME: some of this setup can be moved to set() if it works, or cached if it's expensive
        double len = fTangent1.normalSquared();
        double rlen = rh.fTangent1.normalSquared();
        _Line ray;
        Intersections i, ri;
        int roots, rroots;
        bool flip = false;
        do {
            bool useThis = (len < rlen) ^ flip;
            const Cubic& part = useThis ? fCurvePart : rh.fCurvePart;
            SkPath::Verb partVerb = useThis ? fVerb : rh.fVerb;
            ray[0] = partVerb == SkPath::kCubic_Verb && part[0].approximatelyEqual(part[1]) ?
                part[2] : part[1];
            ray[1].x = (part[0].x + part[partVerb].x) / 2;
            ray[1].y = (part[0].y + part[partVerb].y) / 2;
            SkASSERT(ray[0] != ray[1]);
            roots = (*SegmentRayIntersect[fVerb])(fPts, ray, i);
            rroots = (*SegmentRayIntersect[rh.fVerb])(rh.fPts, ray, ri);
        } while ((roots == 0 || rroots == 0) && (flip ^= true));
        if (roots == 0 || rroots == 0) {
            // FIXME: we don't have a solution in this case. The interim solution
            // is to mark the edges as unsortable, exclude them from this and
            // future computations, and allow the returned path to be fragmented
            fUnsortable = true;
            rh.fUnsortable = true;
            return this < &rh; // even with no solution, return a stable sort
        }
        _Point loc;
        double best = SK_ScalarInfinity;
        double dx, dy, dist;
        int index;
        for (index = 0; index < roots; ++index) {
            (*SegmentXYAtT2[fVerb])(fPts, i.fT[0][index], &loc);
            dx = loc.x - ray[0].x;
            dy = loc.y - ray[0].y;
            dist = dx * dx + dy * dy;
            if (best > dist) {
                best = dist;
            }
        }
        for (index = 0; index < rroots; ++index) {
            (*SegmentXYAtT2[rh.fVerb])(rh.fPts, ri.fT[0][index], &loc);
            dx = loc.x - ray[0].x;
            dy = loc.y - ray[0].y;
            dist = dx * dx + dy * dy;
            if (best > dist) {
                return fSide < 0;
            }
        }
        return fSide > 0;
    }

    double dx() const {
        return fTangent1.dx();
    }

    double dy() const {
        return fTangent1.dy();
    }

    int end() const {
        return fEnd;
    }

    bool isHorizontal() const {
        return dy() == 0 && fVerb == SkPath::kLine_Verb;
    }

    bool lengthen() {
        int newEnd = fEnd;
        if (fStart < fEnd ? ++newEnd < fSpans->count() : --newEnd >= 0) {
            fEnd = newEnd;
            setSpans();
            return true;
        }
        return false;
    }

    bool reverseLengthen() {
        if (fReversed) {
            return false;
        }
        int newEnd = fStart;
        if (fStart > fEnd ? ++newEnd < fSpans->count() : --newEnd >= 0) {
            fEnd = newEnd;
            fReversed = true;
            setSpans();
            return true;
        }
        return false;
    }

    void set(const SkPoint* orig, SkPath::Verb verb, const Segment* segment,
            int start, int end, const SkTDArray<Span>& spans) {
        fSegment = segment;
        fStart = start;
        fEnd = end;
        fPts = orig;
        fVerb = verb;
        fSpans = &spans;
        fReversed = false;
        fUnsortable = false;
        setSpans();
    }


    void setSpans() {
        double startT = (*fSpans)[fStart].fT;
        double endT = (*fSpans)[fEnd].fT;
        switch (fVerb) {
        case SkPath::kLine_Verb:
            _Line l;
            LineSubDivideHD(fPts, startT, endT, l);
            // OPTIMIZATION: for pure line compares, we never need fTangent1.c
            fTangent1.lineEndPoints(l);
            fSide = 0;
            break;
        case SkPath::kQuad_Verb: {
            Quadratic& quad = (Quadratic&)fCurvePart;
            QuadSubDivideHD(fPts, startT, endT, quad);
            fTangent1.quadEndPoints(quad, 0, 1);
            if (dx() == 0 && dy() == 0) {
                fTangent1.quadEndPoints(quad);
            }
            fSide = -fTangent1.pointDistance(fCurvePart[2]); // not normalized -- compare sign only
            } break;
        case SkPath::kCubic_Verb: {
            int nextC = 2;
            CubicSubDivideHD(fPts, startT, endT, fCurvePart);
            fTangent1.cubicEndPoints(fCurvePart, 0, 1);
            if (dx() == 0 && dy() == 0) {
                fTangent1.cubicEndPoints(fCurvePart, 0, 2);
                nextC = 3;
                if (dx() == 0 && dy() == 0) {
                    fTangent1.cubicEndPoints(fCurvePart, 0, 3);
                }
            }
            fSide = -fTangent1.pointDistance(fCurvePart[nextC]); // compare sign only
            if (nextC == 2 && approximately_zero(fSide)) {
                fSide = -fTangent1.pointDistance(fCurvePart[3]);
            }
            } break;
        default:
            SkASSERT(0);
        }
        fUnsortable = dx() == 0 && dy() == 0;
        if (fUnsortable) {
            return;
        }
        SkASSERT(fStart != fEnd);
        int step = fStart < fEnd ? 1 : -1; // OPTIMIZE: worth fStart - fEnd >> 31 type macro?
        for (int index = fStart; index != fEnd; index += step) {
#if 1
            const Span& thisSpan = (*fSpans)[index];
            const Span& nextSpan = (*fSpans)[index + step];
            if (thisSpan.fTiny || precisely_equal(thisSpan.fT, nextSpan.fT)) {
                continue;
            }
            fUnsortable = step > 0 ? thisSpan.fUnsortableStart : nextSpan.fUnsortableEnd;
#if DEBUG_UNSORTABLE
            if (fUnsortable) {
                SkPoint iPt, ePt;
                (*SegmentXYAtT[fVerb])(fPts, thisSpan.fT, &iPt);
                (*SegmentXYAtT[fVerb])(fPts, nextSpan.fT, &ePt);
                SkDebugf("%s unsortable [%d] (%1.9g,%1.9g) [%d] (%1.9g,%1.9g)\n", __FUNCTION__,
                        index, iPt.fX, iPt.fY, fEnd, ePt.fX, ePt.fY);
            }
#endif
            return;
#else
            if ((*fSpans)[index].fUnsortableStart) {
                fUnsortable = true;
                return;
            }
#endif
        }
#if 1
#if DEBUG_UNSORTABLE
        SkPoint iPt, ePt;
        (*SegmentXYAtT[fVerb])(fPts, startT, &iPt);
        (*SegmentXYAtT[fVerb])(fPts, endT, &ePt);
        SkDebugf("%s all tiny unsortable [%d] (%1.9g,%1.9g) [%d] (%1.9g,%1.9g)\n", __FUNCTION__,
            fStart, iPt.fX, iPt.fY, fEnd, ePt.fX, ePt.fY);
#endif
        fUnsortable = true;
#endif
    }

    Segment* segment() const {
        return const_cast<Segment*>(fSegment);
    }

    int sign() const {
        return SkSign32(fStart - fEnd);
    }

    const SkTDArray<Span>* spans() const {
        return fSpans;
    }

    int start() const {
        return fStart;
    }

    bool unsortable() const {
        return fUnsortable;
    }

#if DEBUG_ANGLE
    const SkPoint* pts() const {
        return fPts;
    }

    SkPath::Verb verb() const {
        return fVerb;
    }

    void debugShow(const SkPoint& a) const {
        SkDebugf("    d=(%1.9g,%1.9g) side=%1.9g\n", dx(), dy(), fSide);
    }
#endif

private:
    const SkPoint* fPts;
    Cubic fCurvePart;
    SkPath::Verb fVerb;
    double fSide;
    LineParameters fTangent1;
    const SkTDArray<Span>* fSpans;
    const Segment* fSegment;
    int fStart;
    int fEnd;
    bool fReversed;
    mutable bool fUnsortable; // this alone is editable by the less than operator
};

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

    void add(const SkPoint& pt) {
        if (pt.fX < fLeft) fLeft = pt.fX;
        if (pt.fY < fTop) fTop = pt.fY;
        if (pt.fX > fRight) fRight = pt.fX;
        if (pt.fY > fBottom) fBottom = pt.fY;
    }

    bool isEmpty() {
        return fLeft > fRight || fTop > fBottom
                || (fLeft == fRight && fTop == fBottom)
                || sk_double_isnan(fLeft) || sk_double_isnan(fRight)
                || sk_double_isnan(fTop) || sk_double_isnan(fBottom);
    }

    void setCubicBounds(const SkPoint a[4]) {
        _Rect dRect;
        MAKE_CONST_CUBIC(cubic, a);
        dRect.setBounds(cubic);
        set((float) dRect.left, (float) dRect.top, (float) dRect.right,
                (float) dRect.bottom);
    }

    void setLineBounds(const SkPoint a[2]) {
        setPoint(a[0]);
        add(a[1]);
    }

    void setQuadBounds(const SkPoint a[3]) {
        MAKE_CONST_QUAD(quad, a);
        _Rect dRect;
        dRect.setBounds(quad);
        set((float) dRect.left, (float) dRect.top, (float) dRect.right,
                (float) dRect.bottom);
    }

    void setPoint(const SkPoint& pt) {
        fLeft = fRight = pt.fX;
        fTop = fBottom = pt.fY;
    }
};

static void (Bounds::*setSegmentBounds[])(const SkPoint[]) = {
    NULL,
    &Bounds::setLineBounds,
    &Bounds::setQuadBounds,
    &Bounds::setCubicBounds
};

// OPTIMIZATION: does the following also work, and is it any faster?
// return outerWinding * innerWinding > 0
//      || ((outerWinding + innerWinding < 0) ^ ((outerWinding - innerWinding) < 0)))
static bool useInnerWinding(int outerWinding, int innerWinding) {
    SkASSERT(outerWinding != SK_MaxS32);
    SkASSERT(innerWinding != SK_MaxS32);
    int absOut = abs(outerWinding);
    int absIn = abs(innerWinding);
    bool result = absOut == absIn ? outerWinding < 0 : absOut < absIn;
#if 0 && DEBUG_WINDING
    if (outerWinding * innerWinding < 0) {
        SkDebugf("%s outer=%d inner=%d result=%s\n", __FUNCTION__,
                outerWinding, innerWinding, result ? "true" : "false");
    }
#endif
    return result;
}

#define F (false)      // discard the edge
#define T (true)       // keep the edge

static const bool gUnaryActiveEdge[2][2] = {
//  from=0  from=1
//  to=0,1  to=0,1
    {F, T}, {T, F},
};

static const bool gActiveEdge[kShapeOp_Count][2][2][2][2] = {
//                 miFrom=0                              miFrom=1
//         miTo=0            miTo=1              miTo=0             miTo=1
//    suFrom=0    1     suFrom=0     1      suFrom=0    1      suFrom=0    1
//   suTo=0,1 suTo=0,1  suTo=0,1 suTo=0,1  suTo=0,1 suTo=0,1  suTo=0,1 suTo=0,1
    {{{{F, F}, {F, F}}, {{T, F}, {T, F}}}, {{{T, T}, {F, F}}, {{F, T}, {T, F}}}}, // mi - su
    {{{{F, F}, {F, F}}, {{F, T}, {F, T}}}, {{{F, F}, {T, T}}, {{F, T}, {T, F}}}}, // mi & su
    {{{{F, T}, {T, F}}, {{T, T}, {F, F}}}, {{{T, F}, {T, F}}, {{F, F}, {F, F}}}}, // mi | su
    {{{{F, T}, {T, F}}, {{T, F}, {F, T}}}, {{{T, F}, {F, T}}, {{F, T}, {T, F}}}}, // mi ^ su
};

#undef F
#undef T

// wrap path to keep track of whether the contour is initialized and non-empty
class PathWrapper {
public:
    PathWrapper(SkPath& path)
        : fPathPtr(&path)
        , fCloses(0)
        , fMoves(0)
    {
        init();
    }

    void close() {
        if (!fHasMove) {
            return;
        }
        bool callClose = isClosed();
        lineTo();
        if (fEmpty) {
            return;
        }
        if (callClose) {
    #if DEBUG_PATH_CONSTRUCTION
            SkDebugf("path.close();\n");
    #endif
            fPathPtr->close();
            fCloses++;
        }
        init();
    }

    void cubicTo(const SkPoint& pt1, const SkPoint& pt2, const SkPoint& pt3) {
        lineTo();
        moveTo();
        fDefer[1] = pt3;
        nudge();
        fDefer[0] = fDefer[1];
#if DEBUG_PATH_CONSTRUCTION
        SkDebugf("path.cubicTo(%1.9g,%1.9g, %1.9g,%1.9g, %1.9g,%1.9g);\n",
                pt1.fX, pt1.fY, pt2.fX, pt2.fY, fDefer[1].fX, fDefer[1].fY);
#endif
        fPathPtr->cubicTo(pt1.fX, pt1.fY, pt2.fX, pt2.fY, fDefer[1].fX, fDefer[1].fY);
        fEmpty = false;
    }

    void deferredLine(const SkPoint& pt) {
        if (pt == fDefer[1]) {
            return;
        }
        if (changedSlopes(pt)) {
            lineTo();
            fDefer[0] = fDefer[1];
        }
        fDefer[1] = pt;
    }

    void deferredMove(const SkPoint& pt) {
        fMoved = true;
        fHasMove = true;
        fEmpty = true;
        fDefer[0] = fDefer[1] = pt;
    }

    void deferredMoveLine(const SkPoint& pt) {
        if (!fHasMove) {
            deferredMove(pt);
        }
        deferredLine(pt);
    }

    bool hasMove() const {
        return fHasMove;
    }

    void init() {
        fEmpty = true;
        fHasMove = false;
        fMoved = false;
    }

    bool isClosed() const {
        return !fEmpty && fFirstPt == fDefer[1];
    }

    void lineTo() {
        if (fDefer[0] == fDefer[1]) {
            return;
        }
        moveTo();
        nudge();
        fEmpty = false;
#if DEBUG_PATH_CONSTRUCTION
        SkDebugf("path.lineTo(%1.9g,%1.9g);\n", fDefer[1].fX, fDefer[1].fY);
#endif
        fPathPtr->lineTo(fDefer[1].fX, fDefer[1].fY);
        fDefer[0] = fDefer[1];
    }

    const SkPath* nativePath() const {
        return fPathPtr;
    }

    void nudge() {
        if (fEmpty || !AlmostEqualUlps(fDefer[1].fX, fFirstPt.fX)
                || !AlmostEqualUlps(fDefer[1].fY, fFirstPt.fY)) {
            return;
        }
        fDefer[1] = fFirstPt;
    }

    void quadTo(const SkPoint& pt1, const SkPoint& pt2) {
        lineTo();
        moveTo();
        fDefer[1] = pt2;
        nudge();
        fDefer[0] = fDefer[1];
#if DEBUG_PATH_CONSTRUCTION
        SkDebugf("path.quadTo(%1.9g,%1.9g, %1.9g,%1.9g);\n",
                pt1.fX, pt1.fY, fDefer[1].fX, fDefer[1].fY);
#endif
        fPathPtr->quadTo(pt1.fX, pt1.fY, fDefer[1].fX, fDefer[1].fY);
        fEmpty = false;
    }

    bool someAssemblyRequired() const {
        return fCloses < fMoves;
    }

protected:
    bool changedSlopes(const SkPoint& pt) const {
        if (fDefer[0] == fDefer[1]) {
            return false;
        }
        SkScalar deferDx = fDefer[1].fX - fDefer[0].fX;
        SkScalar deferDy = fDefer[1].fY - fDefer[0].fY;
        SkScalar lineDx = pt.fX - fDefer[1].fX;
        SkScalar lineDy = pt.fY - fDefer[1].fY;
        return deferDx * lineDy != deferDy * lineDx;
    }

    void moveTo() {
        if (!fMoved) {
            return;
        }
        fFirstPt = fDefer[0];
#if DEBUG_PATH_CONSTRUCTION
        SkDebugf("path.moveTo(%1.9g,%1.9g);\n", fDefer[0].fX, fDefer[0].fY);
#endif
        fPathPtr->moveTo(fDefer[0].fX, fDefer[0].fY);
        fMoved = false;
        fMoves++;
    }

private:
    SkPath* fPathPtr;
    SkPoint fDefer[2];
    SkPoint fFirstPt;
    int fCloses;
    int fMoves;
    bool fEmpty;
    bool fHasMove;
    bool fMoved;
};

class Segment {
public:
    Segment() {
#if DEBUG_DUMP
        fID = ++gSegmentID;
#endif
    }

    bool operator<(const Segment& rh) const {
        return fBounds.fTop < rh.fBounds.fTop;
    }

    bool activeAngle(int index, int& done, SkTDArray<Angle>& angles) {
        if (activeAngleInner(index, done, angles)) {
            return true;
        }
        int lesser = index;
        while (--lesser >= 0 && equalPoints(index, lesser)) {
            if (activeAngleOther(lesser, done, angles)) {
                return true;
            }
        }
        lesser = index;
        do {
            if (activeAngleOther(index, done, angles)) {
                return true;
            }
        } while (++index < fTs.count() && equalPoints(index, lesser));
        return false;
    }

    bool activeAngleOther(int index, int& done, SkTDArray<Angle>& angles) {
        Span* span = &fTs[index];
        Segment* other = span->fOther;
        int oIndex = span->fOtherIndex;
        return other->activeAngleInner(oIndex, done, angles);
    }

    bool activeAngleInner(int index, int& done, SkTDArray<Angle>& angles) {
        int next = nextExactSpan(index, 1);
        if (next > 0) {
            Span& upSpan = fTs[index];
            if (upSpan.fWindValue || upSpan.fOppValue) {
                addAngle(angles, index, next);
                if (upSpan.fDone || upSpan.fUnsortableEnd) {
                    done++;
                } else if (upSpan.fWindSum != SK_MinS32) {
                    return true;
                }
            } else if (!upSpan.fDone) {
                upSpan.fDone = true;
                fDoneSpans++;
            }
        }
        int prev = nextExactSpan(index, -1);
        // edge leading into junction
        if (prev >= 0) {
            Span& downSpan = fTs[prev];
            if (downSpan.fWindValue || downSpan.fOppValue) {
                addAngle(angles, index, prev);
                if (downSpan.fDone) {
                    done++;
                 } else if (downSpan.fWindSum != SK_MinS32) {
                    return true;
                }
            } else if (!downSpan.fDone) {
                downSpan.fDone = true;
                fDoneSpans++;
            }
        }
        return false;
    }

    SkPoint activeLeftTop(bool onlySortable, int* firstT) const {
        SkASSERT(!done());
        SkPoint topPt = {SK_ScalarMax, SK_ScalarMax};
        int count = fTs.count();
        // see if either end is not done since we want smaller Y of the pair
        bool lastDone = true;
        bool lastUnsortable = false;
        double lastT = -1;
        for (int index = 0; index < count; ++index) {
            const Span& span = fTs[index];
            if (onlySortable && (span.fUnsortableStart || lastUnsortable)) {
                goto next;
            }
            if (span.fDone && lastDone) {
                goto next;
            }
            if (approximately_negative(span.fT - lastT)) {
                goto next;
            }
            {
                const SkPoint& xy = xyAtT(&span);
                if (topPt.fY > xy.fY || (topPt.fY == xy.fY && topPt.fX > xy.fX)) {
                    topPt = xy;
                    if (firstT) {
                        *firstT = index;
                    }
                }
                if (fVerb != SkPath::kLine_Verb && !lastDone) {
                    SkPoint curveTop = (*SegmentTop[fVerb])(fPts, lastT, span.fT);
                    if (topPt.fY > curveTop.fY || (topPt.fY == curveTop.fY
                            && topPt.fX > curveTop.fX)) {
                        topPt = curveTop;
                        if (firstT) {
                            *firstT = index;
                        }
                    }
                }
                lastT = span.fT;
            }
    next:
            lastDone = span.fDone;
            lastUnsortable = span.fUnsortableEnd;
        }
        return topPt;
    }

    bool activeOp(int index, int endIndex, int xorMiMask, int xorSuMask, ShapeOp op) {
        int sumMiWinding = updateWinding(endIndex, index);
        int sumSuWinding = updateOppWinding(endIndex, index);
        if (fOperand) {
            SkTSwap<int>(sumMiWinding, sumSuWinding);
        }
        int maxWinding, sumWinding, oppMaxWinding, oppSumWinding;
        return activeOp(xorMiMask, xorSuMask, index, endIndex, op, sumMiWinding, sumSuWinding,
                maxWinding, sumWinding, oppMaxWinding, oppSumWinding);
    }

    bool activeOp(int xorMiMask, int xorSuMask, int index, int endIndex, ShapeOp op,
            int& sumMiWinding, int& sumSuWinding,
            int& maxWinding, int& sumWinding, int& oppMaxWinding, int& oppSumWinding) {
        setUpWindings(index, endIndex, sumMiWinding, sumSuWinding,
                maxWinding, sumWinding, oppMaxWinding, oppSumWinding);
        bool miFrom;
        bool miTo;
        bool suFrom;
        bool suTo;
        if (operand()) {
            miFrom = (oppMaxWinding & xorMiMask) != 0;
            miTo = (oppSumWinding & xorMiMask) != 0;
            suFrom = (maxWinding & xorSuMask) != 0;
            suTo = (sumWinding & xorSuMask) != 0;
        } else {
            miFrom = (maxWinding & xorMiMask) != 0;
            miTo = (sumWinding & xorMiMask) != 0;
            suFrom = (oppMaxWinding & xorSuMask) != 0;
            suTo = (oppSumWinding & xorSuMask) != 0;
        }
        bool result = gActiveEdge[op][miFrom][miTo][suFrom][suTo];
#if DEBUG_ACTIVE_OP
        SkDebugf("%s op=%s miFrom=%d miTo=%d suFrom=%d suTo=%d result=%d\n", __FUNCTION__,
                kShapeOpStr[op], miFrom, miTo, suFrom, suTo, result);
#endif
        SkASSERT(result != -1);
        return result;
    }

    bool activeWinding(int index, int endIndex) {
        int sumWinding = updateWinding(endIndex, index);
        int maxWinding;
        return activeWinding(index, endIndex, maxWinding, sumWinding);
    }

    bool activeWinding(int index, int endIndex, int& maxWinding, int& sumWinding) {
        setUpWinding(index, endIndex, maxWinding, sumWinding);
        bool from = maxWinding != 0;
        bool to = sumWinding  != 0;
        bool result = gUnaryActiveEdge[from][to];
        SkASSERT(result != -1);
        return result;
    }

    void addAngle(SkTDArray<Angle>& angles, int start, int end) const {
        SkASSERT(start != end);
        Angle* angle = angles.append();
#if DEBUG_ANGLE
        if (angles.count() > 1 && !fTs[start].fTiny) {
            SkPoint angle0Pt, newPt;
            (*SegmentXYAtT[angles[0].verb()])(angles[0].pts(),
                    (*angles[0].spans())[angles[0].start()].fT, &angle0Pt);
            (*SegmentXYAtT[fVerb])(fPts, fTs[start].fT, &newPt);
            SkASSERT(AlmostEqualUlps(angle0Pt.fX, newPt.fX));
            SkASSERT(AlmostEqualUlps(angle0Pt.fY, newPt.fY));
        }
#endif
        angle->set(fPts, fVerb, this, start, end, fTs);
    }

    void addCancelOutsides(double tStart, double oStart, Segment& other,
            double oEnd) {
        int tIndex = -1;
        int tCount = fTs.count();
        int oIndex = -1;
        int oCount = other.fTs.count();
        do {
            ++tIndex;
        } while (!approximately_negative(tStart - fTs[tIndex].fT) && tIndex < tCount);
        int tIndexStart = tIndex;
        do {
            ++oIndex;
        } while (!approximately_negative(oStart - other.fTs[oIndex].fT) && oIndex < oCount);
        int oIndexStart = oIndex;
        double nextT;
        do {
            nextT = fTs[++tIndex].fT;
        } while (nextT < 1 && approximately_negative(nextT - tStart));
        double oNextT;
        do {
            oNextT = other.fTs[++oIndex].fT;
        } while (oNextT < 1 && approximately_negative(oNextT - oStart));
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
                addTPair(fTs[tIndexStart].fT, other, other.fTs[oIndex].fT, false,
                        fTs[tIndexStart].fPt);
            }
            if (nextT < 1 && fTs[tIndex].fWindValue) {
    #if DEBUG_CONCIDENT
                SkDebugf("%s 2 this=%d other=%d t [%d] %1.9g (%1.9g,%1.9g)\n",
                        __FUNCTION__, fID, other.fID, tIndex,
                        fTs[tIndex].fT, xyAtT(tIndex).fX,
                        xyAtT(tIndex).fY);
    #endif
                addTPair(fTs[tIndex].fT, other, other.fTs[oIndexStart].fT, false, fTs[tIndex].fPt);
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
        } while (!approximately_negative(tStart - fTs[tIndex].fT));
        SkPoint ptStart = fTs[tIndex].fPt;
        do {
            ++oIndex;
        } while (!approximately_negative(oStart - other.fTs[oIndex].fT));
        if (tIndex > 0 || oIndex > 0 || fOperand != other.fOperand) {
            addTPair(tStart, other, oStart, false, ptStart);
        }
        tStart = fTs[tIndex].fT;
        oStart = other.fTs[oIndex].fT;
        do {
            double nextT;
            do {
                nextT = fTs[++tIndex].fT;
            } while (approximately_negative(nextT - tStart));
            tStart = nextT;
            ptStart = fTs[tIndex].fPt;
            do {
                nextT = other.fTs[++oIndex].fT;
            } while (approximately_negative(nextT - oStart));
            oStart = nextT;
            if (tStart == 1 && oStart == 1 && fOperand == other.fOperand) {
                break;
            }
            addTPair(tStart, other, oStart, false, ptStart);
        } while (tStart < 1 && oStart < 1 && !approximately_negative(oEnd - oStart));
    }

    void addCubic(const SkPoint pts[4], bool operand, bool evenOdd) {
        init(pts, SkPath::kCubic_Verb, operand, evenOdd);
        fBounds.setCubicBounds(pts);
    }

    /* SkPoint */ void addCurveTo(int start, int end, PathWrapper& path, bool active) const {
        SkPoint edge[4];
        const SkPoint* ePtr;
        int lastT = fTs.count() - 1;
        if (lastT < 0 || (start == 0 && end == lastT) || (start == lastT && end == 0)) {
            ePtr = fPts;
        } else {
        // OPTIMIZE? if not active, skip remainder and return xy_at_t(end)
            subDivide(start, end, edge);
            ePtr = edge;
        }
        if (active) {
            bool reverse = ePtr == fPts && start != 0;
            if (reverse) {
                path.deferredMoveLine(ePtr[fVerb]);
                switch (fVerb) {
                    case SkPath::kLine_Verb:
                        path.deferredLine(ePtr[0]);
                        break;
                    case SkPath::kQuad_Verb:
                        path.quadTo(ePtr[1], ePtr[0]);
                        break;
                    case SkPath::kCubic_Verb:
                        path.cubicTo(ePtr[2], ePtr[1], ePtr[0]);
                        break;
                    default:
                        SkASSERT(0);
                }
       //         return ePtr[0];
           } else {
                path.deferredMoveLine(ePtr[0]);
                switch (fVerb) {
                    case SkPath::kLine_Verb:
                        path.deferredLine(ePtr[1]);
                        break;
                    case SkPath::kQuad_Verb:
                        path.quadTo(ePtr[1], ePtr[2]);
                        break;
                    case SkPath::kCubic_Verb:
                        path.cubicTo(ePtr[1], ePtr[2], ePtr[3]);
                        break;
                    default:
                        SkASSERT(0);
                }
            }
        }
      //  return ePtr[fVerb];
    }

    void addLine(const SkPoint pts[2], bool operand, bool evenOdd) {
        init(pts, SkPath::kLine_Verb, operand, evenOdd);
        fBounds.set(pts, 2);
    }

#if 0
    const SkPoint& addMoveTo(int tIndex, PathWrapper& path, bool active) const {
        const SkPoint& pt = xyAtT(tIndex);
        if (active) {
            path.deferredMove(pt);
        }
        return pt;
    }
#endif

    // add 2 to edge or out of range values to get T extremes
    void addOtherT(int index, double otherT, int otherIndex) {
        Span& span = fTs[index];
    #if PIN_ADD_T
        if (precisely_less_than_zero(otherT)) {
            otherT = 0;
        } else if (precisely_greater_than_one(otherT)) {
            otherT = 1;
        }
    #endif
        span.fOtherT = otherT;
        span.fOtherIndex = otherIndex;
    }

    void addQuad(const SkPoint pts[3], bool operand, bool evenOdd) {
        init(pts, SkPath::kQuad_Verb, operand, evenOdd);
        fBounds.setQuadBounds(pts);
    }

    // Defer all coincident edge processing until
    // after normal intersections have been computed

// no need to be tricky; insert in normal T order
// resolve overlapping ts when considering coincidence later

    // add non-coincident intersection. Resulting edges are sorted in T.
    int addT(Segment* other, const SkPoint& pt, double& newT) {
        // FIXME: in the pathological case where there is a ton of intercepts,
        //  binary search?
        int insertedAt = -1;
        size_t tCount = fTs.count();
    #if PIN_ADD_T
        // FIXME: only do this pinning here (e.g. this is done also in quad/line intersect)
        if (precisely_less_than_zero(newT)) {
            newT = 0;
        } else if (precisely_greater_than_one(newT)) {
            newT = 1;
        }
    #endif
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
        span->fPt = pt;
        span->fWindSum = SK_MinS32;
        span->fOppSum = SK_MinS32;
        span->fWindValue = 1;
        span->fOppValue = 0;
        span->fTiny = false;
        span->fLoop = false;
        if ((span->fDone = newT == 1)) {
            ++fDoneSpans;
        }
        span->fUnsortableStart = false;
        span->fUnsortableEnd = false;
        int less = -1;
        while (&span[less + 1] - fTs.begin() > 0 && xyAtT(&span[less]) == xyAtT(span)) {
#if 1
            if (span[less].fDone) {
                break;
            }
            double tInterval = newT - span[less].fT;
            if (precisely_negative(tInterval)) {
                break;
            }
            if (fVerb == SkPath::kCubic_Verb) {
                double tMid = newT - tInterval / 2;
                _Point midPt;
                CubicXYAtT(fPts, tMid, &midPt);
                if (!midPt.approximatelyEqual(xyAtT(span))) {
                    break;
                }
            }
            span[less].fTiny = true;
            span[less].fDone = true;
            if (approximately_negative(newT - span[less].fT)) {
                if (approximately_greater_than_one(newT)) {
                    span[less].fUnsortableStart = true;
                    span[less - 1].fUnsortableEnd = true;
                }
                if (approximately_less_than_zero(span[less].fT)) {
                    span[less + 1].fUnsortableStart = true;
                    span[less].fUnsortableEnd = true;
                }
            }
            ++fDoneSpans;
#else
            double tInterval = newT - span[less].fT;
            if (precisely_negative(tInterval)) {
                break;
            }
            if (fVerb == SkPath::kCubic_Verb) {
                double tMid = newT - tInterval / 2;
                _Point midPt;
                CubicXYAtT(fPts, tMid, &midPt);
                if (!midPt.approximatelyEqual(xyAtT(span))) {
                    break;
                }
            }
            SkASSERT(span[less].fDone == span->fDone);
            if (span[less].fT == 0) {
                span->fT = newT = 0;
            } else {
                setSpanT(less, newT);
            }
#endif
            --less;
        }
        int more = 1;
        while (fTs.end() - &span[more - 1] > 1 && xyAtT(&span[more]) == xyAtT(span)) {
#if 1
            if (span[more - 1].fDone) {
                break;
            }
            double tEndInterval = span[more].fT - newT;
            if (precisely_negative(tEndInterval)) {
                break;
            }
            if (fVerb == SkPath::kCubic_Verb) {
                double tMid = newT - tEndInterval / 2;
                _Point midEndPt;
                CubicXYAtT(fPts, tMid, &midEndPt);
                if (!midEndPt.approximatelyEqual(xyAtT(span))) {
                    break;
                }
            }
            span[more - 1].fTiny = true;
            span[more - 1].fDone = true;
            if (approximately_negative(span[more].fT - newT)) {
                if (approximately_greater_than_one(span[more].fT)) {
                    span[more + 1].fUnsortableStart = true;
                    span[more].fUnsortableEnd = true;
                }
                if (approximately_less_than_zero(newT)) {
                    span[more].fUnsortableStart = true;
                    span[more - 1].fUnsortableEnd = true;
                }
            }
            ++fDoneSpans;
#else
            double tEndInterval = span[more].fT - newT;
            if (precisely_negative(tEndInterval)) {
                break;
            }
            if (fVerb == SkPath::kCubic_Verb) {
                double tMid = newT - tEndInterval / 2;
                _Point midEndPt;
                CubicXYAtT(fPts, tMid, &midEndPt);
                if (!midEndPt.approximatelyEqual(xyAtT(span))) {
                    break;
                }
            }
            SkASSERT(span[more - 1].fDone == span[more].fDone);
            if (newT == 0) {
                setSpanT(more, 0);
            } else {
                span->fT = newT = span[more].fT;
            }
#endif
            ++more;
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
        SkASSERT(!approximately_negative(endT - startT));
        SkASSERT(!approximately_negative(oEndT - oStartT));
        bool binary = fOperand != other.fOperand;
        int index = 0;
        while (!approximately_negative(startT - fTs[index].fT)) {
            ++index;
        }
        int oIndex = other.fTs.count();
        while (approximately_positive(other.fTs[--oIndex].fT - oEndT))
            ;
        double tRatio = (oEndT - oStartT) / (endT - startT);
        Span* test = &fTs[index];
        Span* oTest = &other.fTs[oIndex];
        SkTDArray<double> outsideTs;
        SkTDArray<double> oOutsideTs;
        do {
            bool decrement = test->fWindValue && oTest->fWindValue && !binary;
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
            } while (approximately_negative(span->fT - testT));
            Span* oSpan = oTest;
            double otherTMatchStart = oEndT - (span->fT - startT) * tRatio;
            double otherTMatchEnd = oEndT - (test->fT - startT) * tRatio;
            SkDEBUGCODE(int originalWindValue = oSpan->fWindValue);
            while (approximately_negative(otherTMatchStart - oSpan->fT)
                    && !approximately_negative(otherTMatchEnd - oSpan->fT)) {
        #ifdef SK_DEBUG
                SkASSERT(originalWindValue == oSpan->fWindValue);
        #endif
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
        } while (!approximately_negative(endT - test->fT));
        SkASSERT(!oIndex || approximately_negative(oTest->fT - oStartT));
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

    int addSelfT(Segment* other, const SkPoint& pt, double& newT) {
        int result = addT(other, pt, newT);
        Span* span = &fTs[result];
        span->fLoop = true;
        return result;
    }

    int addUnsortableT(Segment* other, bool start, const SkPoint& pt, double& newT) {
        int result = addT(other, pt, newT);
        Span* span = &fTs[result];
        if (start) {
            if (result > 0) {
                span[result - 1].fUnsortableEnd = true;
            }
            span[result].fUnsortableStart = true;
        } else {
            span[result].fUnsortableEnd = true;
            if (result + 1 < fTs.count()) {
                span[result + 1].fUnsortableStart = true;
            }
        }
        return result;
    }

    int bumpCoincidentThis(const Span* oTest, bool opp, int index,
            SkTDArray<double>& outsideTs) {
        int oWindValue = oTest->fWindValue;
        int oOppValue = oTest->fOppValue;
        if (opp) {
            SkTSwap<int>(oWindValue, oOppValue);
        }
        Span* const test = &fTs[index];
        Span* end = test;
        const double oStartT = oTest->fT;
        do {
            if (bumpSpan(end, oWindValue, oOppValue)) {
                TrackOutside(outsideTs, end->fT, oStartT);
            }
            end = &fTs[++index];
        } while (approximately_negative(end->fT - test->fT));
        return index;
    }

    // because of the order in which coincidences are resolved, this and other
    // may not have the same intermediate points. Compute the corresponding
    // intermediate T values (using this as the master, other as the follower)
    // and walk other conditionally -- hoping that it catches up in the end
    int bumpCoincidentOther(const Span* test, double oEndT, int& oIndex,
            SkTDArray<double>& oOutsideTs) {
        Span* const oTest = &fTs[oIndex];
        Span* oEnd = oTest;
        const double startT = test->fT;
        const double oStartT = oTest->fT;
        while (!approximately_negative(oEndT - oEnd->fT)
                && approximately_negative(oEnd->fT - oStartT)) {
            zeroSpan(oEnd);
            TrackOutside(oOutsideTs, oEnd->fT, startT);
            oEnd = &fTs[++oIndex];
        }
        return oIndex;
    }

    // FIXME: need to test this case:
    // contourA has two segments that are coincident
    // contourB has two segments that are coincident in the same place
    // each ends up with +2/0 pairs for winding count
    // since logic below doesn't transfer count (only increments/decrements) can this be
    // resolved to +4/0 ?

    // set spans from start to end to increment the greater by one and decrement
    // the lesser
    void addTCoincident(double startT, double endT, Segment& other, double oStartT, double oEndT) {
        SkASSERT(!approximately_negative(endT - startT));
        SkASSERT(!approximately_negative(oEndT - oStartT));
        bool opp = fOperand ^ other.fOperand;
        int index = 0;
        while (!approximately_negative(startT - fTs[index].fT)) {
            ++index;
        }
        int oIndex = 0;
        while (!approximately_negative(oStartT - other.fTs[oIndex].fT)) {
            ++oIndex;
        }
        Span* test = &fTs[index];
        Span* oTest = &other.fTs[oIndex];
        SkTDArray<double> outsideTs;
        SkTDArray<double> oOutsideTs;
        do {
            // if either span has an opposite value and the operands don't match, resolve first
     //       SkASSERT(!test->fDone || !oTest->fDone);
            if (test->fDone || oTest->fDone) {
                index = advanceCoincidentThis(oTest, opp, index);
                oIndex = other.advanceCoincidentOther(test, oEndT, oIndex);
            } else {
                index = bumpCoincidentThis(oTest, opp, index, outsideTs);
                oIndex = other.bumpCoincidentOther(test, oEndT, oIndex, oOutsideTs);
            }
            test = &fTs[index];
            oTest = &other.fTs[oIndex];
        } while (!approximately_negative(endT - test->fT));
        SkASSERT(approximately_negative(oTest->fT - oEndT));
        SkASSERT(approximately_negative(oEndT - oTest->fT));
        if (!done() && outsideTs.count()) {
            addCoinOutsides(outsideTs, other, oEndT);
        }
        if (!other.done() && oOutsideTs.count()) {
            other.addCoinOutsides(oOutsideTs, *this, endT);
        }
    }

    // FIXME: this doesn't prevent the same span from being added twice
    // fix in caller, SkASSERT here?
    void addTPair(double t, Segment& other, double otherT, bool borrowWind, const SkPoint& pt) {
        int tCount = fTs.count();
        for (int tIndex = 0; tIndex < tCount; ++tIndex) {
            const Span& span = fTs[tIndex];
            if (!approximately_negative(span.fT - t)) {
                break;
            }
            if (approximately_negative(span.fT - t) && span.fOther == &other
                    && approximately_equal(span.fOtherT, otherT)) {
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
        int insertedAt = addT(&other, pt, t);
        int otherInsertedAt = other.addT(this, pt, otherT);
        addOtherT(insertedAt, otherT, otherInsertedAt);
        other.addOtherT(otherInsertedAt, t, insertedAt);
        matchWindingValue(insertedAt, t, borrowWind);
        other.matchWindingValue(otherInsertedAt, otherT, borrowWind);
    }

    void addTwoAngles(int start, int end, SkTDArray<Angle>& angles) const {
        // add edge leading into junction
        int min = SkMin32(end, start);
        if (fTs[min].fWindValue > 0 || fTs[min].fOppValue > 0) {
            addAngle(angles, end, start);
        }
        // add edge leading away from junction
        int step = SkSign32(end - start);
        int tIndex = nextExactSpan(end, step);
        min = SkMin32(end, tIndex);
        if (tIndex >= 0 && (fTs[min].fWindValue > 0 || fTs[min].fOppValue > 0)) {
            addAngle(angles, end, tIndex);
        }
    }

    int advanceCoincidentThis(const Span* oTest, bool opp, int index) {
        Span* const test = &fTs[index];
        Span* end = test;
        do {
            end = &fTs[++index];
        } while (approximately_negative(end->fT - test->fT));
        return index;
    }

    int advanceCoincidentOther(const Span* test, double oEndT, int& oIndex) {
        Span* const oTest = &fTs[oIndex];
        Span* oEnd = oTest;
        const double oStartT = oTest->fT;
        while (!approximately_negative(oEndT - oEnd->fT)
                && approximately_negative(oEnd->fT - oStartT)) {
            oEnd = &fTs[++oIndex];
        }
        return oIndex;
    }

    bool betweenTs(int lesser, double testT, int greater) {
        if (lesser > greater) {
            SkTSwap<int>(lesser, greater);
        }
        return approximately_between(fTs[lesser].fT, testT, fTs[greater].fT);
    }

    const Bounds& bounds() const {
        return fBounds;
    }

    void buildAngles(int index, SkTDArray<Angle>& angles, bool includeOpp) const {
        double referenceT = fTs[index].fT;
        int lesser = index;
        while (--lesser >= 0 && (includeOpp || fTs[lesser].fOther->fOperand == fOperand)
                && precisely_negative(referenceT - fTs[lesser].fT)) {
            buildAnglesInner(lesser, angles);
        }
        do {
            buildAnglesInner(index, angles);
        } while (++index < fTs.count() && (includeOpp || fTs[index].fOther->fOperand == fOperand)
                && precisely_negative(fTs[index].fT - referenceT));
    }

    void buildAnglesInner(int index, SkTDArray<Angle>& angles) const {
        const Span* span = &fTs[index];
        Segment* other = span->fOther;
    // if there is only one live crossing, and no coincidence, continue
    // in the same direction
    // if there is coincidence, the only choice may be to reverse direction
        // find edge on either side of intersection
        int oIndex = span->fOtherIndex;
        // if done == -1, prior span has already been processed
        int step = 1;
        int next = other->nextExactSpan(oIndex, step);
       if (next < 0) {
            step = -step;
            next = other->nextExactSpan(oIndex, step);
        }
        // add candidate into and away from junction
        other->addTwoAngles(next, oIndex, angles);
    }

    int computeSum(int startIndex, int endIndex, bool binary) {
        SkTDArray<Angle> angles;
        addTwoAngles(startIndex, endIndex, angles);
        buildAngles(endIndex, angles, false);
        // OPTIMIZATION: check all angles to see if any have computed wind sum
        // before sorting (early exit if none)
        SkTDArray<Angle*> sorted;
        bool sortable = SortAngles(angles, sorted);
#if DEBUG_SORT
        sorted[0]->segment()->debugShowSort(__FUNCTION__, sorted, 0, 0, 0);
#endif
        if (!sortable) {
            return SK_MinS32;
        }
        int angleCount = angles.count();
        const Angle* angle;
        const Segment* base;
        int winding;
        int oWinding;
        int firstIndex = 0;
        do {
            angle = sorted[firstIndex];
            base = angle->segment();
            winding = base->windSum(angle);
            if (winding != SK_MinS32) {
                oWinding = base->oppSum(angle);
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
        SkDebugf("%s spanWinding=%d winding=%d sign=%d inner=%d result=%d\n", __FUNCTION__,
            spanWinding, winding, angle->sign(), inner,
            inner ? winding + spanWinding : winding);
    #endif
        if (inner) {
            winding += spanWinding;
        }
    #if DEBUG_SORT
        base->debugShowSort(__FUNCTION__, sorted, firstIndex, winding, oWinding);
    #endif
        int nextIndex = firstIndex + 1;
        int lastIndex = firstIndex != 0 ? firstIndex : angleCount;
        winding -= base->spanSign(angle);
        oWinding -= base->oppSign(angle);
        do {
            if (nextIndex == angleCount) {
                nextIndex = 0;
            }
            angle = sorted[nextIndex];
            Segment* segment = angle->segment();
            bool opp = base->fOperand ^ segment->fOperand;
            int maxWinding, oMaxWinding;
            int spanSign = segment->spanSign(angle);
            int oppoSign = segment->oppSign(angle);
            if (opp) {
                oMaxWinding = oWinding;
                oWinding -= spanSign;
                maxWinding = winding;
                if (oppoSign) {
                    winding -= oppoSign;
                }
            } else {
                maxWinding = winding;
                winding -= spanSign;
                oMaxWinding = oWinding;
                if (oppoSign) {
                    oWinding -= oppoSign;
                }
            }
            if (segment->windSum(angle) == SK_MinS32) {
                if (opp) {
                    if (useInnerWinding(oMaxWinding, oWinding)) {
                        oMaxWinding = oWinding;
                    }
                    if (oppoSign && useInnerWinding(maxWinding, winding)) {
                        maxWinding = winding;
                    }
                    (void) segment->markAndChaseWinding(angle, oMaxWinding, maxWinding);
                } else {
                    if (useInnerWinding(maxWinding, winding)) {
                        maxWinding = winding;
                    }
                    if (oppoSign && useInnerWinding(oMaxWinding, oWinding)) {
                        oMaxWinding = oWinding;
                    }
                    (void) segment->markAndChaseWinding(angle, maxWinding,
                            binary ? oMaxWinding : 0);
                }
            }
        } while (++nextIndex != lastIndex);
        int minIndex = SkMin32(startIndex, endIndex);
        return windSum(minIndex);
    }

    int crossedSpanY(const SkPoint& basePt, SkScalar& bestY, double& hitT, bool& hitSomething,
            double mid, bool opp, bool current) const {
        SkScalar bottom = fBounds.fBottom;
        int bestTIndex = -1;
        if (bottom <= bestY) {
            return bestTIndex;
        }
        SkScalar top = fBounds.fTop;
        if (top >= basePt.fY) {
            return bestTIndex;
        }
        if (fBounds.fLeft > basePt.fX) {
            return bestTIndex;
        }
        if (fBounds.fRight < basePt.fX) {
            return bestTIndex;
        }
        if (fBounds.fLeft == fBounds.fRight) {
            // if vertical, and directly above test point, wait for another one
            return AlmostEqualUlps(basePt.fX, fBounds.fLeft) ? SK_MinS32 : bestTIndex;
        }
        // intersect ray starting at basePt with edge
        Intersections intersections;
        // OPTIMIZE: use specialty function that intersects ray with curve,
        // returning t values only for curve (we don't care about t on ray)
        int pts = (*VSegmentIntersect[fVerb])(fPts, top, bottom, basePt.fX, false, intersections);
        if (pts == 0 || (current && pts == 1)) {
            return bestTIndex;
        }
        if (current) {
            SkASSERT(pts > 1);
            int closestIdx = 0;
            double closest = fabs(intersections.fT[0][0] - mid);
            for (int idx = 1; idx < pts; ++idx) {
                double test = fabs(intersections.fT[0][idx] - mid);
                if (closest > test) {
                    closestIdx = idx;
                    closest = test;
                }
            }
            if (closestIdx < pts - 1) {
                intersections.fT[0][closestIdx] = intersections.fT[0][pts - 1];
            }
            --pts;
        }
        double bestT = -1;
        for (int index = 0; index < pts; ++index) {
            double foundT = intersections.fT[0][index];
            if (approximately_less_than_zero(foundT)
                        || approximately_greater_than_one(foundT)) {
                continue;
            }
            SkScalar testY = (*SegmentYAtT[fVerb])(fPts, foundT);
            if (approximately_negative(testY - bestY)
                    || approximately_negative(basePt.fY - testY)) {
                continue;
            }
            if (pts > 1 && fVerb == SkPath::kLine_Verb) {
                return SK_MinS32; // if the intersection is edge on, wait for another one
            }
            if (fVerb > SkPath::kLine_Verb) {
                SkScalar dx = (*SegmentDXAtT[fVerb])(fPts, foundT);
                if (approximately_zero(dx)) {
                    return SK_MinS32; // hit vertical, wait for another one
                }
            }
            bestY = testY;
            bestT = foundT;
        }
        if (bestT < 0) {
            return bestTIndex;
        }
        SkASSERT(bestT >= 0);
        SkASSERT(bestT <= 1);
        int start;
        int end = 0;
        do {
            start = end;
            end = nextSpan(start, 1);
        } while (fTs[end].fT < bestT);
        // FIXME: see next candidate for a better pattern to find the next start/end pair
        while (start + 1 < end && fTs[start].fDone) {
            ++start;
        }
        if (!isCanceled(start)) {
            hitT = bestT;
            bestTIndex = start;
            hitSomething = true;
        }
        return bestTIndex;
    }

    void decrementSpan(Span* span) {
        SkASSERT(span->fWindValue > 0);
        if (--(span->fWindValue) == 0) {
            if (!span->fOppValue && !span->fDone) {
                span->fDone = true;
                ++fDoneSpans;
            }
        }
    }

    bool bumpSpan(Span* span, int windDelta, int oppDelta) {
        SkASSERT(!span->fDone);
        span->fWindValue += windDelta;
        SkASSERT(span->fWindValue >= 0);
        span->fOppValue += oppDelta;
        SkASSERT(span->fOppValue >= 0);
        if (fXor) {
            span->fWindValue &= 1;
        }
        if (fOppXor) {
            span->fOppValue &= 1;
        }
        if (!span->fWindValue && !span->fOppValue) {
            span->fDone = true;
            ++fDoneSpans;
            return true;
        }
        return false;
    }

    // OPTIMIZE
    // when the edges are initially walked, they don't automatically get the prior and next
    // edges assigned to positions t=0 and t=1. Doing that would remove the need for this check,
    // and would additionally remove the need for similar checks in condition edges. It would
    // also allow intersection code to assume end of segment intersections (maybe?)
    bool complete() const {
        int count = fTs.count();
        return count > 1 && fTs[0].fT == 0 && fTs[--count].fT == 1;
    }

    bool done() const {
        SkASSERT(fDoneSpans <= fTs.count());
        return fDoneSpans == fTs.count();
    }

    bool done(int min) const {
        return fTs[min].fDone;
    }

    bool done(const Angle* angle) const {
        return done(SkMin32(angle->start(), angle->end()));
    }

    SkVector dxdy(int index) const {
        return (*SegmentDXDYAtT[fVerb])(fPts, fTs[index].fT);
    }

    SkScalar dy(int index) const {
        return (*SegmentDYAtT[fVerb])(fPts, fTs[index].fT);
    }

    bool equalPoints(int greaterTIndex, int lesserTIndex) {
        SkASSERT(greaterTIndex >= lesserTIndex);
        double greaterT = fTs[greaterTIndex].fT;
        double lesserT = fTs[lesserTIndex].fT;
        if (greaterT == lesserT) {
            return true;
        }
        if (!approximately_negative(greaterT - lesserT)) {
            return false;
        }
        return xyAtT(greaterTIndex) == xyAtT(lesserTIndex);
    }

    /*
     The M and S variable name parts stand for the operators.
       Mi stands for Minuend (see wiki subtraction, analogous to difference)
       Su stands for Subtrahend
     The Opp variable name part designates that the value is for the Opposite operator.
     Opposite values result from combining coincident spans.
     */

    Segment* findNextOp(SkTDArray<Span*>& chase, int& nextStart, int& nextEnd,
            bool& unsortable, ShapeOp op, const int xorMiMask, const int xorSuMask) {
        const int startIndex = nextStart;
        const int endIndex = nextEnd;
        SkASSERT(startIndex != endIndex);
        const int count = fTs.count();
        SkASSERT(startIndex < endIndex ? startIndex < count - 1 : startIndex > 0);
        const int step = SkSign32(endIndex - startIndex);
        const int end = nextExactSpan(startIndex, step);
        SkASSERT(end >= 0);
        Span* endSpan = &fTs[end];
        Segment* other;
        if (isSimple(end)) {
        // mark the smaller of startIndex, endIndex done, and all adjacent
        // spans with the same T value (but not 'other' spans)
    #if DEBUG_WINDING
            SkDebugf("%s simple\n", __FUNCTION__);
    #endif
            int min = SkMin32(startIndex, endIndex);
            if (fTs[min].fDone) {
                return NULL;
            }
            markDoneBinary(min);
            other = endSpan->fOther;
            nextStart = endSpan->fOtherIndex;
            double startT = other->fTs[nextStart].fT;
            nextEnd = nextStart;
            do {
                nextEnd += step;
            }
            while (precisely_zero(startT - other->fTs[nextEnd].fT));
            SkASSERT(step < 0 ? nextEnd >= 0 : nextEnd < other->fTs.count());
            return other;
        }
        // more than one viable candidate -- measure angles to find best
        SkTDArray<Angle> angles;
        SkASSERT(startIndex - endIndex != 0);
        SkASSERT((startIndex - endIndex < 0) ^ (step < 0));
        addTwoAngles(startIndex, end, angles);
        buildAngles(end, angles, true);
        SkTDArray<Angle*> sorted;
        bool sortable = SortAngles(angles, sorted);
        int angleCount = angles.count();
        int firstIndex = findStartingEdge(sorted, startIndex, end);
        SkASSERT(firstIndex >= 0);
    #if DEBUG_SORT
        debugShowSort(__FUNCTION__, sorted, firstIndex);
    #endif
        if (!sortable) {
            unsortable = true;
            return NULL;
        }
        SkASSERT(sorted[firstIndex]->segment() == this);
    #if DEBUG_WINDING
        SkDebugf("%s firstIndex=[%d] sign=%d\n", __FUNCTION__, firstIndex,
                sorted[firstIndex]->sign());
    #endif
        int sumMiWinding = updateWinding(endIndex, startIndex);
        int sumSuWinding = updateOppWinding(endIndex, startIndex);
        if (operand()) {
            SkTSwap<int>(sumMiWinding, sumSuWinding);
        }
        int nextIndex = firstIndex + 1;
        int lastIndex = firstIndex != 0 ? firstIndex : angleCount;
        const Angle* foundAngle = NULL;
        bool foundDone = false;
        // iterate through the angle, and compute everyone's winding
        Segment* nextSegment;
        int activeCount = 0;
        do {
            SkASSERT(nextIndex != firstIndex);
            if (nextIndex == angleCount) {
                nextIndex = 0;
            }
            const Angle* nextAngle = sorted[nextIndex];
            nextSegment = nextAngle->segment();
            int maxWinding, sumWinding, oppMaxWinding, oppSumWinding;
            bool activeAngle = nextSegment->activeOp(xorMiMask, xorSuMask, nextAngle->start(),
                    nextAngle->end(), op, sumMiWinding, sumSuWinding,
                    maxWinding, sumWinding, oppMaxWinding, oppSumWinding);
            if (activeAngle) {
                ++activeCount;
                if (!foundAngle || (foundDone && activeCount & 1)) {
                    if (nextSegment->tiny(nextAngle)) {
                        unsortable = true;
                        return NULL;
                    }
                    foundAngle = nextAngle;
                    foundDone = nextSegment->done(nextAngle) && !nextSegment->tiny(nextAngle);
                }
            }
            if (nextSegment->done()) {
                continue;
            }
            if (nextSegment->windSum(nextAngle) != SK_MinS32) {
                continue;
            }
            Span* last = nextSegment->markAngle(maxWinding, sumWinding, oppMaxWinding,
                    oppSumWinding, activeAngle, nextAngle);
            if (last) {
                *chase.append() = last;
#if DEBUG_WINDING
                SkDebugf("%s chase.append id=%d\n", __FUNCTION__,
                        last->fOther->fTs[last->fOtherIndex].fOther->debugID());
#endif
            }
        } while (++nextIndex != lastIndex);
        markDoneBinary(SkMin32(startIndex, endIndex));
        if (!foundAngle) {
            return NULL;
        }
        nextStart = foundAngle->start();
        nextEnd = foundAngle->end();
        nextSegment = foundAngle->segment();

    #if DEBUG_WINDING
        SkDebugf("%s from:[%d] to:[%d] start=%d end=%d\n",
                __FUNCTION__, debugID(), nextSegment->debugID(), nextStart, nextEnd);
     #endif
        return nextSegment;
    }

    Segment* findNextWinding(SkTDArray<Span*>& chase, int& nextStart, int& nextEnd,
            bool& unsortable) {
        const int startIndex = nextStart;
        const int endIndex = nextEnd;
        SkASSERT(startIndex != endIndex);
        const int count = fTs.count();
        SkASSERT(startIndex < endIndex ? startIndex < count - 1 : startIndex > 0);
        const int step = SkSign32(endIndex - startIndex);
        const int end = nextExactSpan(startIndex, step);
        SkASSERT(end >= 0);
        Span* endSpan = &fTs[end];
        Segment* other;
        if (isSimple(end)) {
        // mark the smaller of startIndex, endIndex done, and all adjacent
        // spans with the same T value (but not 'other' spans)
    #if DEBUG_WINDING
            SkDebugf("%s simple\n", __FUNCTION__);
    #endif
            int min = SkMin32(startIndex, endIndex);
            if (fTs[min].fDone) {
                return NULL;
            }
            markDoneUnary(min);
            other = endSpan->fOther;
            nextStart = endSpan->fOtherIndex;
            double startT = other->fTs[nextStart].fT;
            nextEnd = nextStart;
            do {
                nextEnd += step;
            }
            while (precisely_zero(startT - other->fTs[nextEnd].fT));
            SkASSERT(step < 0 ? nextEnd >= 0 : nextEnd < other->fTs.count());
            return other;
        }
        // more than one viable candidate -- measure angles to find best
        SkTDArray<Angle> angles;
        SkASSERT(startIndex - endIndex != 0);
        SkASSERT((startIndex - endIndex < 0) ^ (step < 0));
        addTwoAngles(startIndex, end, angles);
        buildAngles(end, angles, true);
        SkTDArray<Angle*> sorted;
        bool sortable = SortAngles(angles, sorted);
        int angleCount = angles.count();
        int firstIndex = findStartingEdge(sorted, startIndex, end);
        SkASSERT(firstIndex >= 0);
    #if DEBUG_SORT
        debugShowSort(__FUNCTION__, sorted, firstIndex);
    #endif
        if (!sortable) {
            unsortable = true;
            return NULL;
        }
        SkASSERT(sorted[firstIndex]->segment() == this);
    #if DEBUG_WINDING
        SkDebugf("%s firstIndex=[%d] sign=%d\n", __FUNCTION__, firstIndex,
                sorted[firstIndex]->sign());
    #endif
        int sumWinding = updateWinding(endIndex, startIndex);
        int nextIndex = firstIndex + 1;
        int lastIndex = firstIndex != 0 ? firstIndex : angleCount;
        const Angle* foundAngle = NULL;
        bool foundDone = false;
        // iterate through the angle, and compute everyone's winding
        Segment* nextSegment;
        int activeCount = 0;
        do {
            SkASSERT(nextIndex != firstIndex);
            if (nextIndex == angleCount) {
                nextIndex = 0;
            }
            const Angle* nextAngle = sorted[nextIndex];
            nextSegment = nextAngle->segment();
            int maxWinding;
            bool activeAngle = nextSegment->activeWinding(nextAngle->start(), nextAngle->end(),
                    maxWinding, sumWinding);
            if (activeAngle) {
                ++activeCount;
                if (!foundAngle || (foundDone && activeCount & 1)) {
                    if (nextSegment->tiny(nextAngle)) {
                        unsortable = true;
                        return NULL;
                    }
                    foundAngle = nextAngle;
                    foundDone = nextSegment->done(nextAngle);
                }
            }
            if (nextSegment->done()) {
                continue;
            }
            if (nextSegment->windSum(nextAngle) != SK_MinS32) {
                continue;
            }
            Span* last = nextSegment->markAngle(maxWinding, sumWinding, activeAngle, nextAngle);
            if (last) {
                *chase.append() = last;
#if DEBUG_WINDING
                SkDebugf("%s chase.append id=%d\n", __FUNCTION__,
                        last->fOther->fTs[last->fOtherIndex].fOther->debugID());
#endif
            }
        } while (++nextIndex != lastIndex);
        markDoneUnary(SkMin32(startIndex, endIndex));
        if (!foundAngle) {
            return NULL;
        }
        nextStart = foundAngle->start();
        nextEnd = foundAngle->end();
        nextSegment = foundAngle->segment();
    #if DEBUG_WINDING
        SkDebugf("%s from:[%d] to:[%d] start=%d end=%d\n",
                __FUNCTION__, debugID(), nextSegment->debugID(), nextStart, nextEnd);
     #endif
        return nextSegment;
    }

    Segment* findNextXor(int& nextStart, int& nextEnd, bool& unsortable) {
        const int startIndex = nextStart;
        const int endIndex = nextEnd;
        SkASSERT(startIndex != endIndex);
        int count = fTs.count();
        SkASSERT(startIndex < endIndex ? startIndex < count - 1
                : startIndex > 0);
        int step = SkSign32(endIndex - startIndex);
        int end = nextExactSpan(startIndex, step);
        SkASSERT(end >= 0);
        Span* endSpan = &fTs[end];
        Segment* other;
        if (isSimple(end)) {
    #if DEBUG_WINDING
            SkDebugf("%s simple\n", __FUNCTION__);
    #endif
            int min = SkMin32(startIndex, endIndex);
            if (fTs[min].fDone) {
                return NULL;
            }
            markDone(min, 1);
            other = endSpan->fOther;
            nextStart = endSpan->fOtherIndex;
            double startT = other->fTs[nextStart].fT;
        #if 01 // FIXME: I don't know why the logic here is difference from the winding case
            SkDEBUGCODE(bool firstLoop = true;)
            if ((approximately_less_than_zero(startT) && step < 0)
                    || (approximately_greater_than_one(startT) && step > 0)) {
                step = -step;
                SkDEBUGCODE(firstLoop = false;)
            }
            do {
        #endif
                nextEnd = nextStart;
                do {
                    nextEnd += step;
                }
                 while (precisely_zero(startT - other->fTs[nextEnd].fT));
        #if 01
                if (other->fTs[SkMin32(nextStart, nextEnd)].fWindValue) {
                    break;
                }
 #ifdef SK_DEBUG
                SkASSERT(firstLoop);
 #endif
                SkDEBUGCODE(firstLoop = false;)
                step = -step;
            } while (true);
        #endif
            SkASSERT(step < 0 ? nextEnd >= 0 : nextEnd < other->fTs.count());
            return other;
        }
        SkTDArray<Angle> angles;
        SkASSERT(startIndex - endIndex != 0);
        SkASSERT((startIndex - endIndex < 0) ^ (step < 0));
        addTwoAngles(startIndex, end, angles);
        buildAngles(end, angles, false);
        SkTDArray<Angle*> sorted;
        bool sortable = SortAngles(angles, sorted);
        if (!sortable) {
            unsortable = true;
    #if DEBUG_SORT
            debugShowSort(__FUNCTION__, sorted, findStartingEdge(sorted, startIndex, end), 0, 0);
    #endif
            return NULL;
        }
        int angleCount = angles.count();
        int firstIndex = findStartingEdge(sorted, startIndex, end);
        SkASSERT(firstIndex >= 0);
    #if DEBUG_SORT
        debugShowSort(__FUNCTION__, sorted, firstIndex, 0, 0);
    #endif
        SkASSERT(sorted[firstIndex]->segment() == this);
        int nextIndex = firstIndex + 1;
        int lastIndex = firstIndex != 0 ? firstIndex : angleCount;
        const Angle* foundAngle = NULL;
        bool foundDone = false;
        Segment* nextSegment;
        int activeCount = 0;
        do {
            SkASSERT(nextIndex != firstIndex);
            if (nextIndex == angleCount) {
                nextIndex = 0;
            }
            const Angle* nextAngle = sorted[nextIndex];
            nextSegment = nextAngle->segment();
            ++activeCount;
            if (!foundAngle || (foundDone && activeCount & 1)) {
                if (nextSegment->tiny(nextAngle)) {
                    unsortable = true;
                    return NULL;
                }
                foundAngle = nextAngle;
                foundDone = nextSegment->done(nextAngle);
            }
            if (nextSegment->done()) {
                continue;
            }
        } while (++nextIndex != lastIndex);
        markDone(SkMin32(startIndex, endIndex), 1);
        if (!foundAngle) {
            return NULL;
        }
        nextStart = foundAngle->start();
        nextEnd = foundAngle->end();
        nextSegment = foundAngle->segment();
    #if DEBUG_WINDING
        SkDebugf("%s from:[%d] to:[%d] start=%d end=%d\n",
                __FUNCTION__, debugID(), nextSegment->debugID(), nextStart, nextEnd);
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
    // note that fOtherIndex isn't computed yet, so it can't be used here
    void findTooCloseToCall() {
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
            // FIXME: allow quads, cubics to be near coincident?
            if (mOther->fVerb == SkPath::kLine_Verb) {
                moCount = mOther->fTs.count();
                if (moCount >= 3) {
                    break;
                }
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
            if (tOther->fVerb != SkPath::kLine_Verb) {
                continue; // FIXME: allow quads, cubics to be near coincident?
            }
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
                    if (tOther->windValueAt(moSpan.fOtherT) == 0) {
                        moStart = -1;
                        break;
                    }
                    SkASSERT(moEnd == -1);
                    moEnd = moIndex;
                    moEndT = moSpan.fT;
                }
            }
            if (moStart < 0 || moEnd < 0) {
                continue;
            }
            // FIXME: if moStartT, moEndT are initialized to NaN, can skip this test
            if (approximately_equal(moStartT, moEndT)) {
                continue;
            }
            int toStart = -1;
            int toEnd = -1;
            double toStartT, toEndT;
            for (int toIndex = 0; toIndex < toCount; ++toIndex) {
                Span& toSpan = tOther->fTs[toIndex];
                if (toSpan.fDone) {
                    continue;
                }
                if (toSpan.fOther == this) {
                    if (toSpan.fOtherT == test->fT) {
                        toStart = toIndex;
                        toStartT = toSpan.fT;
                    }
                    continue;
                }
                if (toSpan.fOther == mOther && toSpan.fOtherT == moEndT) {
                    if (mOther->windValueAt(toSpan.fOtherT) == 0) {
                        moStart = -1;
                        break;
                    }
                    SkASSERT(toEnd == -1);
                    toEnd = toIndex;
                    toEndT = toSpan.fT;
                }
            }
            // FIXME: if toStartT, toEndT are initialized to NaN, can skip this test
            if (toStart <= 0 || toEnd <= 0) {
                continue;
            }
            if (approximately_equal(toStartT, toEndT)) {
                continue;
            }
            // test to see if the segment between there and here is linear
            if (!mOther->isLinear(moStart, moEnd)
                    || !tOther->isLinear(toStart, toEnd)) {
                continue;
            }
            bool flipped = (moStart - moEnd) * (toStart - toEnd) < 1;
            if (flipped) {
                mOther->addTCancel(moStartT, moEndT, *tOther, toEndT, toStartT);
            } else {
                mOther->addTCoincident(moStartT, moEndT, *tOther, toStartT, toEndT);
            }
        }
    }

    // FIXME: either:
    // a) mark spans with either end unsortable as done, or
    // b) rewrite findTop / findTopSegment / findTopContour to iterate further
    //    when encountering an unsortable span

    // OPTIMIZATION : for a pair of lines, can we compute points at T (cached)
    // and use more concise logic like the old edge walker code?
    // FIXME: this needs to deal with coincident edges
    Segment* findTop(int& tIndex, int& endIndex, bool& unsortable, bool onlySortable) {
        // iterate through T intersections and return topmost
        // topmost tangent from y-min to first pt is closer to horizontal
        SkASSERT(!done());
        int firstT = -1;
        /* SkPoint topPt = */ activeLeftTop(onlySortable, &firstT);
        if (firstT < 0) {
            unsortable = true;
            firstT = 0;
            while (fTs[firstT].fDone) {
                SkASSERT(firstT < fTs.count());
                ++firstT;
            }
            tIndex = firstT;
            endIndex = nextExactSpan(firstT, 1);
            return this;
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
        buildAngles(firstT, angles, true);
        SkTDArray<Angle*> sorted;
        bool sortable = SortAngles(angles, sorted);
        int first = SK_MaxS32;
        SkScalar top = SK_ScalarMax;
        int count = sorted.count();
        for (int index = 0; index < count; ++index) {
            const Angle* angle = sorted[index];
            Segment* next = angle->segment();
            Bounds bounds;
            next->subDivideBounds(angle->end(), angle->start(), bounds);
            if (approximately_greater(top, bounds.fTop)) {
                top = bounds.fTop;
                first = index;
            }
        }
        SkASSERT(first < SK_MaxS32);
    #if DEBUG_SORT // || DEBUG_SWAP_TOP
        sorted[first]->segment()->debugShowSort(__FUNCTION__, sorted, first, 0, 0);
    #endif
        if (onlySortable && !sortable) {
            unsortable = true;
            return NULL;
        }
        // skip edges that have already been processed
        firstT = first - 1;
        Segment* leftSegment;
        do {
            if (++firstT == count) {
                firstT = 0;
            }
            const Angle* angle = sorted[firstT];
            SkASSERT(!onlySortable || !angle->unsortable());
            leftSegment = angle->segment();
            tIndex = angle->end();
            endIndex = angle->start();
        } while (leftSegment->fTs[SkMin32(tIndex, endIndex)].fDone);
        if (leftSegment->verb() >= SkPath::kQuad_Verb) {
            if (!leftSegment->clockwise(tIndex, endIndex)) {
                bool swap = leftSegment->verb() == SkPath::kQuad_Verb
                        || (!leftSegment->monotonic_in_y(tIndex, endIndex)
                        && !leftSegment->serpentine(tIndex, endIndex));
        #if DEBUG_SWAP_TOP
                SkDebugf("%s swap=%d serpentine=%d controls_contained_by_ends=%d\n", __FUNCTION__,
                        swap,
                        leftSegment->serpentine(tIndex, endIndex),
                        leftSegment->controls_contained_by_ends(tIndex, endIndex),
                        leftSegment->monotonic_in_y(tIndex, endIndex));
        #endif
                if (swap) {
        // FIXME: I doubt it makes sense to (necessarily) swap if the edge was not the first
        // sorted but merely the first not already processed (i.e., not done)
                    SkTSwap(tIndex, endIndex);
                }
            }
        }
        SkASSERT(!leftSegment->fTs[SkMin32(tIndex, endIndex)].fTiny);
        return leftSegment;
    }

    // FIXME: not crazy about this
    // when the intersections are performed, the other index is into an
    // incomplete array. As the array grows, the indices become incorrect
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
                if (oT == oSpan.fT && this == oSpan.fOther && oSpan.fOtherT == iSpan.fT) {
                    iSpan.fOtherIndex = o;
                    break;
                }
            }
        }
    }

    void init(const SkPoint pts[], SkPath::Verb verb, bool operand, bool evenOdd) {
        fDoneSpans = 0;
        fOperand = operand;
        fXor = evenOdd;
        fPts = pts;
        fVerb = verb;
    }

    void initWinding(int start, int end) {
        int local = spanSign(start, end);
        int oppLocal = oppSign(start, end);
        (void) markAndChaseWinding(start, end, local, oppLocal);
        // OPTIMIZATION: the reverse mark and chase could skip the first marking
        (void) markAndChaseWinding(end, start, local, oppLocal);
    }

    void initWinding(int start, int end, int winding, int oppWinding) {
        int local = spanSign(start, end);
        if (local * winding >= 0) {
            winding += local;
        }
        int oppLocal = oppSign(start, end);
        if (oppLocal * oppWinding >= 0) {
            oppWinding += oppLocal;
        }
        (void) markAndChaseWinding(start, end, winding, oppWinding);
    }

/*
when we start with a vertical intersect, we try to use the dx to determine if the edge is to
the left or the right of vertical. This determines if we need to add the span's
sign or not. However, this isn't enough.
If the supplied sign (winding) is zero, then we didn't hit another vertical span, so dx is needed.
If there was a winding, then it may or may not need adjusting. If the span the winding was borrowed
from has the same x direction as this span, the winding should change. If the dx is opposite, then
the same winding is shared by both.
*/
    void initWinding(int start, int end, double tHit, int winding, SkScalar hitDx, int oppWind,
            SkScalar hitOppDx) {
        SkASSERT(hitDx || !winding);
        SkScalar dx = (*SegmentDXAtT[fVerb])(fPts, tHit);
        SkASSERT(dx);
        int windVal = windValue(SkMin32(start, end));
    #if DEBUG_WINDING_AT_T
        SkDebugf("%s oldWinding=%d hitDx=%c dx=%c windVal=%d", __FUNCTION__, winding,
                hitDx ? hitDx > 0 ? '+' : '-' : '0', dx > 0 ? '+' : '-', windVal);
    #endif
        if (!winding) {
            winding = dx < 0 ? windVal : -windVal;
        } else if (winding * dx < 0) {
            int sideWind = winding + (dx < 0 ? windVal : -windVal);
            if (abs(winding) < abs(sideWind)) {
                winding = sideWind;
            }
        }
    #if DEBUG_WINDING_AT_T
        SkDebugf(" winding=%d\n", winding);
    #endif
        int oppLocal = oppSign(start, end);
        SkASSERT(hitOppDx || !oppWind || !oppLocal);
        int oppWindVal = oppValue(SkMin32(start, end));
        if (!oppWind) {
            oppWind = dx < 0 ? oppWindVal : -oppWindVal;
        } else if (hitOppDx * dx >= 0) {
            int oppSideWind = oppWind + (dx < 0 ? oppWindVal : -oppWindVal);
            if (abs(oppWind) < abs(oppSideWind)) {
                oppWind = oppSideWind;
            }
        }
        (void) markAndChaseWinding(start, end, winding, oppWind);
    }

    bool intersected() const {
        return fTs.count() > 0;
    }

    bool isCanceled(int tIndex) const {
        return fTs[tIndex].fWindValue == 0 && fTs[tIndex].fOppValue == 0;
    }

    bool isConnected(int startIndex, int endIndex) const {
        return fTs[startIndex].fWindSum != SK_MinS32
                || fTs[endIndex].fWindSum != SK_MinS32;
    }

    bool isHorizontal() const {
        return fBounds.fTop == fBounds.fBottom;
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
            if (approximately_zero(startT - fTs[index].fT)) {
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
        if (approximately_less_than_zero(t)) {
            return !approximately_less_than_zero(fTs[1].fT);
        }
        if (approximately_greater_than_one(t)) {
            return !approximately_greater_than_one(fTs[count - 2].fT);
        }
        return false;
    }

    bool isVertical() const {
        return fBounds.fLeft == fBounds.fRight;
    }

    bool isVertical(int start, int end) const {
        return (*SegmentVertical[fVerb])(fPts, start, end);
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
        return markAndChaseDone(index, endIndex, winding);
    }

    Span* markAndChaseDone(int index, int endIndex, int winding) {
        int step = SkSign32(endIndex - index);
        int min = SkMin32(index, endIndex);
        markDone(min, winding);
        Span* last;
        Segment* other = this;
        while ((other = other->nextChase(index, step, min, last))) {
            other->markDone(min, winding);
        }
        return last;
    }

    Span* markAndChaseDoneBinary(const Angle* angle, int winding, int oppWinding) {
        int index = angle->start();
        int endIndex = angle->end();
        int step = SkSign32(endIndex - index);
        int min = SkMin32(index, endIndex);
        markDoneBinary(min, winding, oppWinding);
        Span* last;
        Segment* other = this;
        while ((other = other->nextChase(index, step, min, last))) {
            other->markDoneBinary(min, winding, oppWinding);
        }
        return last;
    }

    Span* markAndChaseDoneBinary(int index, int endIndex) {
        int step = SkSign32(endIndex - index);
        int min = SkMin32(index, endIndex);
        markDoneBinary(min);
        Span* last;
        Segment* other = this;
        while ((other = other->nextChase(index, step, min, last))) {
            if (other->done()) {
                return NULL;
            }
            other->markDoneBinary(min);
        }
        return last;
    }

    Span* markAndChaseDoneUnary(int index, int endIndex) {
        int step = SkSign32(endIndex - index);
        int min = SkMin32(index, endIndex);
        markDoneUnary(min);
        Span* last;
        Segment* other = this;
        while ((other = other->nextChase(index, step, min, last))) {
            if (other->done()) {
                return NULL;
            }
            other->markDoneUnary(min);
        }
        return last;
    }

    Span* markAndChaseDoneUnary(const Angle* angle, int winding) {
        int index = angle->start();
        int endIndex = angle->end();
        return markAndChaseDone(index, endIndex, winding);
    }

    Span* markAndChaseWinding(const Angle* angle, const int winding) {
        int index = angle->start();
        int endIndex = angle->end();
        int step = SkSign32(endIndex - index);
        int min = SkMin32(index, endIndex);
        markWinding(min, winding);
        Span* last;
        Segment* other = this;
        while ((other = other->nextChase(index, step, min, last))) {
            if (other->fTs[min].fWindSum != SK_MinS32) {
                SkASSERT(other->fTs[min].fWindSum == winding);
                return NULL;
            }
            other->markWinding(min, winding);
        }
        return last;
    }

    Span* markAndChaseWinding(int index, int endIndex, int winding, int oppWinding) {
        int min = SkMin32(index, endIndex);
        int step = SkSign32(endIndex - index);
        markWinding(min, winding, oppWinding);
        Span* last;
        Segment* other = this;
        while ((other = other->nextChase(index, step, min, last))) {
            if (other->fTs[min].fWindSum != SK_MinS32) {
                SkASSERT(other->fTs[min].fWindSum == winding || other->fTs[min].fLoop);
                return NULL;
            }
            other->markWinding(min, winding, oppWinding);
        }
        return last;
    }

    Span* markAndChaseWinding(const Angle* angle, int winding, int oppWinding) {
        int start = angle->start();
        int end = angle->end();
        return markAndChaseWinding(start, end, winding, oppWinding);
    }

    Span* markAngle(int maxWinding, int sumWinding, bool activeAngle, const Angle* angle) {
        SkASSERT(angle->segment() == this);
        if (useInnerWinding(maxWinding, sumWinding)) {
            maxWinding = sumWinding;
        }
        Span* last;
        if (activeAngle) {
            last = markAndChaseWinding(angle, maxWinding);
        } else {
            last = markAndChaseDoneUnary(angle, maxWinding);
        }
        return last;
    }

    Span* markAngle(int maxWinding, int sumWinding, int oppMaxWinding, int oppSumWinding,
            bool activeAngle, const Angle* angle) {
        SkASSERT(angle->segment() == this);
        if (useInnerWinding(maxWinding, sumWinding)) {
            maxWinding = sumWinding;
        }
        if (oppMaxWinding != oppSumWinding && useInnerWinding(oppMaxWinding, oppSumWinding)) {
            oppMaxWinding = oppSumWinding;
        }
        Span* last;
        if (activeAngle) {
            last = markAndChaseWinding(angle, maxWinding, oppMaxWinding);
        } else {
            last = markAndChaseDoneBinary(angle, maxWinding, oppMaxWinding);
        }
        return last;
    }

    // FIXME: this should also mark spans with equal (x,y)
    // This may be called when the segment is already marked done. While this
    // wastes time, it shouldn't do any more than spin through the T spans.
    // OPTIMIZATION: abort on first done found (assuming that this code is
    // always called to mark segments done).
    void markDone(int index, int winding) {
      //  SkASSERT(!done());
        SkASSERT(winding);
        double referenceT = fTs[index].fT;
        int lesser = index;
        while (--lesser >= 0 && precisely_negative(referenceT - fTs[lesser].fT)) {
            markOneDone(__FUNCTION__, lesser, winding);
        }
        do {
            markOneDone(__FUNCTION__, index, winding);
        } while (++index < fTs.count() && precisely_negative(fTs[index].fT - referenceT));
    }

    void markDoneBinary(int index, int winding, int oppWinding) {
      //  SkASSERT(!done());
        SkASSERT(winding || oppWinding);
        double referenceT = fTs[index].fT;
        int lesser = index;
        while (--lesser >= 0 && precisely_negative(referenceT - fTs[lesser].fT)) {
            markOneDoneBinary(__FUNCTION__, lesser, winding, oppWinding);
        }
        do {
            markOneDoneBinary(__FUNCTION__, index, winding, oppWinding);
        } while (++index < fTs.count() && precisely_negative(fTs[index].fT - referenceT));
    }

    void markDoneBinary(int index) {
        double referenceT = fTs[index].fT;
        int lesser = index;
        while (--lesser >= 0 && precisely_negative(referenceT - fTs[lesser].fT)) {
            markOneDoneBinary(__FUNCTION__, lesser);
        }
        do {
            markOneDoneBinary(__FUNCTION__, index);
        } while (++index < fTs.count() && precisely_negative(fTs[index].fT - referenceT));
    }

    void markDoneUnary(int index, int winding) {
      //  SkASSERT(!done());
        SkASSERT(winding);
        double referenceT = fTs[index].fT;
        int lesser = index;
        while (--lesser >= 0 && precisely_negative(referenceT - fTs[lesser].fT)) {
            markOneDoneUnary(__FUNCTION__, lesser, winding);
        }
        do {
            markOneDoneUnary(__FUNCTION__, index, winding);
        } while (++index < fTs.count() && precisely_negative(fTs[index].fT - referenceT));
    }

    void markDoneUnary(int index) {
        double referenceT = fTs[index].fT;
        int lesser = index;
        while (--lesser >= 0 && precisely_negative(referenceT - fTs[lesser].fT)) {
            markOneDoneUnary(__FUNCTION__, lesser);
        }
        do {
            markOneDoneUnary(__FUNCTION__, index);
        } while (++index < fTs.count() && precisely_negative(fTs[index].fT - referenceT));
    }

    void markOneDone(const char* funName, int tIndex, int winding) {
        Span* span = markOneWinding(funName, tIndex, winding);
        if (!span) {
            return;
        }
        span->fDone = true;
        fDoneSpans++;
    }

    void markOneDoneBinary(const char* funName, int tIndex) {
        Span* span = verifyOneWinding(funName, tIndex);
        if (!span) {
            return;
        }
        span->fDone = true;
        fDoneSpans++;
    }

    void markOneDoneBinary(const char* funName, int tIndex, int winding, int oppWinding) {
        Span* span = markOneWinding(funName, tIndex, winding, oppWinding);
        if (!span) {
            return;
        }
        span->fDone = true;
        fDoneSpans++;
    }

    void markOneDoneUnary(const char* funName, int tIndex) {
        Span* span = verifyOneWindingU(funName, tIndex);
        if (!span) {
            return;
        }
        span->fDone = true;
        fDoneSpans++;
    }

    void markOneDoneUnary(const char* funName, int tIndex, int winding) {
        Span* span = markOneWinding(funName, tIndex, winding);
        if (!span) {
            return;
        }
        span->fDone = true;
        fDoneSpans++;
    }

    Span* markOneWinding(const char* funName, int tIndex, int winding) {
        Span& span = fTs[tIndex];
        if (span.fDone) {
            return NULL;
        }
    #if DEBUG_MARK_DONE
        debugShowNewWinding(funName, span, winding);
    #endif
        SkASSERT(span.fWindSum == SK_MinS32 || span.fWindSum == winding);
   #ifdef SK_DEBUG
        SkASSERT(abs(winding) <= gDebugMaxWindSum);
   #endif
        span.fWindSum = winding;
        return &span;
    }

    Span* markOneWinding(const char* funName, int tIndex, int winding, int oppWinding) {
        Span& span = fTs[tIndex];
        if (span.fDone) {
            return NULL;
        }
    #if DEBUG_MARK_DONE
        debugShowNewWinding(funName, span, winding, oppWinding);
    #endif
        SkASSERT(span.fWindSum == SK_MinS32 || span.fWindSum == winding);
   #ifdef SK_DEBUG
        SkASSERT(abs(winding) <= gDebugMaxWindSum);
   #endif
        span.fWindSum = winding;
        SkASSERT(span.fOppSum == SK_MinS32 || span.fOppSum == oppWinding);
   #ifdef SK_DEBUG
        SkASSERT(abs(oppWinding) <= gDebugMaxWindSum);
   #endif
        span.fOppSum = oppWinding;
        return &span;
    }

    bool controls_contained_by_ends(int tStart, int tEnd) const {
        if (fVerb != SkPath::kCubic_Verb) {
            return false;
        }
        MAKE_CONST_CUBIC(aCubic, fPts);
        Cubic dst;
        sub_divide(aCubic, fTs[tStart].fT, fTs[tEnd].fT, dst);
        return ::controls_contained_by_ends(dst);
    }

    // from http://stackoverflow.com/questions/1165647/how-to-determine-if-a-list-of-polygon-points-are-in-clockwise-order
    bool clockwise(int tStart, int tEnd) const {
        SkASSERT(fVerb != SkPath::kLine_Verb);
        SkPoint edge[4];
        subDivide(tStart, tEnd, edge);
        double sum = (edge[0].fX - edge[fVerb].fX) * (edge[0].fY + edge[fVerb].fY);
        if (fVerb == SkPath::kCubic_Verb) {
            SkScalar lesser = SkTMin(edge[0].fY, edge[3].fY);
            if (edge[1].fY < lesser && edge[2].fY < lesser) {
                _Line tangent1 = { {edge[0].fX, edge[0].fY}, {edge[1].fX, edge[1].fY} };
                _Line tangent2 = { {edge[2].fX, edge[2].fY}, {edge[3].fX, edge[3].fY} };
                if (testIntersect(tangent1, tangent2)) {
                    SkPoint topPt = CubicTop(fPts, fTs[tStart].fT, fTs[tEnd].fT);
                    sum += (topPt.fX - edge[0].fX) * (topPt.fY + edge[0].fY);
                    sum += (edge[3].fX - topPt.fX) * (edge[3].fY + topPt.fY);
                    return sum <= 0;
                }
            }
        }
        for (int idx = 0; idx < fVerb; ++idx){
            sum += (edge[idx + 1].fX - edge[idx].fX) * (edge[idx + 1].fY + edge[idx].fY);
        }
        return sum <= 0;
    }

    bool monotonic_in_y(int tStart, int tEnd) const {
        if (fVerb != SkPath::kCubic_Verb) {
            return false;
        }
        MAKE_CONST_CUBIC(aCubic, fPts);
        Cubic dst;
        sub_divide(aCubic, fTs[tStart].fT, fTs[tEnd].fT, dst);
        return ::monotonic_in_y(dst);
    }

    bool serpentine(int tStart, int tEnd) const {
        if (fVerb != SkPath::kCubic_Verb) {
            return false;
        }
        MAKE_CONST_CUBIC(aCubic, fPts);
        Cubic dst;
        sub_divide(aCubic, fTs[tStart].fT, fTs[tEnd].fT, dst);
        return ::serpentine(dst);
    }

    Span* verifyOneWinding(const char* funName, int tIndex) {
        Span& span = fTs[tIndex];
        if (span.fDone) {
            return NULL;
        }
    #if DEBUG_MARK_DONE
        debugShowNewWinding(funName, span, span.fWindSum, span.fOppSum);
    #endif
        SkASSERT(span.fWindSum != SK_MinS32);
        SkASSERT(span.fOppSum != SK_MinS32);
        return &span;
    }

    Span* verifyOneWindingU(const char* funName, int tIndex) {
        Span& span = fTs[tIndex];
        if (span.fDone) {
            return NULL;
        }
    #if DEBUG_MARK_DONE
        debugShowNewWinding(funName, span, span.fWindSum);
    #endif
        SkASSERT(span.fWindSum != SK_MinS32);
        return &span;
    }

    // note that just because a span has one end that is unsortable, that's
    // not enough to mark it done. The other end may be sortable, allowing the
    // span to be added.
    // FIXME: if abs(start - end) > 1, mark intermediates as unsortable on both ends
    void markUnsortable(int start, int end) {
        Span* span = &fTs[start];
        if (start < end) {
#if DEBUG_UNSORTABLE
            debugShowNewWinding(__FUNCTION__, *span, 0);
#endif
            span->fUnsortableStart = true;
        } else {
            --span;
#if DEBUG_UNSORTABLE
            debugShowNewWinding(__FUNCTION__, *span, 0);
#endif
            span->fUnsortableEnd = true;
        }
        if (!span->fUnsortableStart || !span->fUnsortableEnd || span->fDone) {
            return;
        }
        span->fDone = true;
        fDoneSpans++;
    }

    void markWinding(int index, int winding) {
    //    SkASSERT(!done());
        SkASSERT(winding);
        double referenceT = fTs[index].fT;
        int lesser = index;
        while (--lesser >= 0 && precisely_negative(referenceT - fTs[lesser].fT)) {
            markOneWinding(__FUNCTION__, lesser, winding);
        }
        do {
            markOneWinding(__FUNCTION__, index, winding);
       } while (++index < fTs.count() && precisely_negative(fTs[index].fT - referenceT));
    }

    void markWinding(int index, int winding, int oppWinding) {
    //    SkASSERT(!done());
        SkASSERT(winding || oppWinding);
        double referenceT = fTs[index].fT;
        int lesser = index;
        while (--lesser >= 0 && precisely_negative(referenceT - fTs[lesser].fT)) {
            markOneWinding(__FUNCTION__, lesser, winding, oppWinding);
        }
        do {
            markOneWinding(__FUNCTION__, index, winding, oppWinding);
       } while (++index < fTs.count() && precisely_negative(fTs[index].fT - referenceT));
    }

    void matchWindingValue(int tIndex, double t, bool borrowWind) {
        int nextDoorWind = SK_MaxS32;
        int nextOppWind = SK_MaxS32;
        if (tIndex > 0) {
            const Span& below = fTs[tIndex - 1];
            if (approximately_negative(t - below.fT)) {
                nextDoorWind = below.fWindValue;
                nextOppWind = below.fOppValue;
            }
        }
        if (nextDoorWind == SK_MaxS32 && tIndex + 1 < fTs.count()) {
            const Span& above = fTs[tIndex + 1];
            if (approximately_negative(above.fT - t)) {
                nextDoorWind = above.fWindValue;
                nextOppWind = above.fOppValue;
            }
        }
        if (nextDoorWind == SK_MaxS32 && borrowWind && tIndex > 0 && t < 1) {
            const Span& below = fTs[tIndex - 1];
            nextDoorWind = below.fWindValue;
            nextOppWind = below.fOppValue;
        }
        if (nextDoorWind != SK_MaxS32) {
            Span& newSpan = fTs[tIndex];
            newSpan.fWindValue = nextDoorWind;
            newSpan.fOppValue = nextOppWind;
            if (!nextDoorWind && !nextOppWind && !newSpan.fDone) {
                newSpan.fDone = true;
                ++fDoneSpans;
            }
        }
    }

    bool moreHorizontal(int index, int endIndex, bool& unsortable) const {
        // find bounds
        Bounds bounds;
        bounds.setPoint(xyAtT(index));
        bounds.add(xyAtT(endIndex));
        SkScalar width = bounds.width();
        SkScalar height = bounds.height();
        if (width > height) {
            if (approximately_negative(width)) {
                unsortable = true; // edge is too small to resolve meaningfully
            }
            return false;
        } else {
            if (approximately_negative(height)) {
                unsortable = true; // edge is too small to resolve meaningfully
            }
            return true;
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

    bool nextCandidate(int& start, int& end) const {
        while (fTs[end].fDone) {
            if (fTs[end].fT == 1) {
                return false;
            }
            ++end;
        }
        start = end;
        end = nextExactSpan(start, 1);
        return true;
    }

    Segment* nextChase(int& index, const int step, int& min, Span*& last) {
        int end = nextExactSpan(index, step);
        SkASSERT(end >= 0);
        if (multipleSpans(end)) {
            last = &fTs[end];
            return NULL;
        }
        const Span& endSpan = fTs[end];
        Segment* other = endSpan.fOther;
        index = endSpan.fOtherIndex;
        SkASSERT(index >= 0);
        int otherEnd = other->nextExactSpan(index, step);
        SkASSERT(otherEnd >= 0);
        min = SkMin32(index, otherEnd);
        return other;
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
            if (approximately_zero(span.fT - fromSpan.fT)) {
                continue;
            }
            return to;
        }
        return -1;
    }

    // FIXME
    // this returns at any difference in T, vs. a preset minimum. It may be
    // that all callers to nextSpan should use this instead.
    // OPTIMIZATION splitting this into separate loops for up/down steps
    // would allow using precisely_negative instead of precisely_zero
    int nextExactSpan(int from, int step) const {
        const Span& fromSpan = fTs[from];
        int count = fTs.count();
        int to = from;
        while (step > 0 ? ++to < count : --to >= 0) {
            const Span& span = fTs[to];
            if (precisely_zero(span.fT - fromSpan.fT)) {
                continue;
            }
            return to;
        }
        return -1;
    }

    bool operand() const {
        return fOperand;
    }

    int oppSign(const Angle* angle) const {
        SkASSERT(angle->segment() == this);
        return oppSign(angle->start(), angle->end());
    }

    int oppSign(int startIndex, int endIndex) const {
        int result = startIndex < endIndex ? -fTs[startIndex].fOppValue
                : fTs[endIndex].fOppValue;
#if DEBUG_WIND_BUMP
        SkDebugf("%s oppSign=%d\n", __FUNCTION__, result);
#endif
        return result;
    }

    int oppSum(int tIndex) const {
        return fTs[tIndex].fOppSum;
    }

    int oppSum(const Angle* angle) const {
        int lesser = SkMin32(angle->start(), angle->end());
        return fTs[lesser].fOppSum;
    }

    int oppValue(int tIndex) const {
        return fTs[tIndex].fOppValue;
    }

    int oppValue(const Angle* angle) const {
        int lesser = SkMin32(angle->start(), angle->end());
        return fTs[lesser].fOppValue;
    }

    const SkPoint* pts() const {
        return fPts;
    }

    void reset() {
        init(NULL, (SkPath::Verb) -1, false, false);
        fBounds.set(SK_ScalarMax, SK_ScalarMax, SK_ScalarMax, SK_ScalarMax);
        fTs.reset();
    }

    void setOppXor(bool isOppXor) {
        fOppXor = isOppXor;
    }

    void setSpanT(int index, double t) {
        Span& span = fTs[index];
        span.fT = t;
        span.fOther->fTs[span.fOtherIndex].fOtherT = t;
    }

    void setUpWinding(int index, int endIndex, int& maxWinding, int& sumWinding) {
        int deltaSum = spanSign(index, endIndex);
        maxWinding = sumWinding;
        sumWinding = sumWinding -= deltaSum;
    }

    void setUpWindings(int index, int endIndex, int& sumMiWinding, int& sumSuWinding,
            int& maxWinding, int& sumWinding, int& oppMaxWinding, int& oppSumWinding) {
        int deltaSum = spanSign(index, endIndex);
        int oppDeltaSum = oppSign(index, endIndex);
        if (operand()) {
            maxWinding = sumSuWinding;
            sumWinding = sumSuWinding -= deltaSum;
            oppMaxWinding = sumMiWinding;
            oppSumWinding = sumMiWinding -= oppDeltaSum;
        } else {
            maxWinding = sumMiWinding;
            sumWinding = sumMiWinding -= deltaSum;
            oppMaxWinding = sumSuWinding;
            oppSumWinding = sumSuWinding -= oppDeltaSum;
        }
    }

    // This marks all spans unsortable so that this info is available for early
    // exclusion in find top and others. This could be optimized to only mark
    // adjacent spans that unsortable. However, this makes it difficult to later
    // determine starting points for edge detection in find top and the like.
    static bool SortAngles(SkTDArray<Angle>& angles, SkTDArray<Angle*>& angleList) {
        bool sortable = true;
        int angleCount = angles.count();
        int angleIndex;
        angleList.setReserve(angleCount);
        for (angleIndex = 0; angleIndex < angleCount; ++angleIndex) {
            Angle& angle = angles[angleIndex];
            *angleList.append() = &angle;
            sortable &= !angle.unsortable();
        }
        if (sortable) {
            QSort<Angle>(angleList.begin(), angleList.end() - 1);
            for (angleIndex = 0; angleIndex < angleCount; ++angleIndex) {
                if (angles[angleIndex].unsortable()) {
                    sortable = false;
                    break;
                }
            }
        }
        if (!sortable) {
            for (angleIndex = 0; angleIndex < angleCount; ++angleIndex) {
                Angle& angle = angles[angleIndex];
                angle.segment()->markUnsortable(angle.start(), angle.end());
            }
        }
        return sortable;
    }

    // OPTIMIZATION: mark as debugging only if used solely by tests
    const Span& span(int tIndex) const {
        return fTs[tIndex];
    }

    int spanSign(const Angle* angle) const {
        SkASSERT(angle->segment() == this);
        return spanSign(angle->start(), angle->end());
    }

    int spanSign(int startIndex, int endIndex) const {
        int result = startIndex < endIndex ? -fTs[startIndex].fWindValue
                : fTs[endIndex].fWindValue;
#if DEBUG_WIND_BUMP
        SkDebugf("%s spanSign=%d\n", __FUNCTION__, result);
#endif
        return result;
    }

    void subDivide(int start, int end, SkPoint edge[4]) const {
        edge[0] = fTs[start].fPt;
        edge[fVerb] = fTs[end].fPt;
        if (fVerb == SkPath::kQuad_Verb || fVerb == SkPath::kCubic_Verb) {
            _Point sub[2] = {{ edge[0].fX, edge[0].fY}, {edge[fVerb].fX, edge[fVerb].fY }};
            if (fVerb == SkPath::kQuad_Verb) {
                MAKE_CONST_QUAD(aQuad, fPts);
                edge[1] = sub_divide(aQuad, sub[0], sub[1], fTs[start].fT, fTs[end].fT).asSkPoint();
            } else {
                MAKE_CONST_CUBIC(aCubic, fPts);
                sub_divide(aCubic, sub[0], sub[1], fTs[start].fT, fTs[end].fT, sub);
                edge[1] = sub[0].asSkPoint();
                edge[2] = sub[1].asSkPoint();
            }
        }
    }

    void subDivideBounds(int start, int end, Bounds& bounds) const {
        SkPoint edge[4];
        subDivide(start, end, edge);
        (bounds.*setSegmentBounds[fVerb])(edge);
    }

    // OPTIMIZATION: mark as debugging only if used solely by tests
    double t(int tIndex) const {
        return fTs[tIndex].fT;
    }

    double tAtMid(int start, int end, double mid) const {
        return fTs[start].fT * (1 - mid) + fTs[end].fT * mid;
    }

    bool tiny(const Angle* angle) const {
        int start = angle->start();
        int end = angle->end();
        const Span& mSpan = fTs[SkMin32(start, end)];
        return mSpan.fTiny;
    }

    static void TrackOutside(SkTDArray<double>& outsideTs, double end,
            double start) {
        int outCount = outsideTs.count();
        if (outCount == 0 || !approximately_negative(end - outsideTs[outCount - 2])) {
            *outsideTs.append() = end;
            *outsideTs.append() = start;
        }
    }

    void undoneSpan(int& start, int& end) {
        size_t tCount = fTs.count();
        size_t index;
        for (index = 0; index < tCount; ++index) {
            if (!fTs[index].fDone) {
                break;
            }
        }
        SkASSERT(index < tCount - 1);
        start = index;
        double startT = fTs[index].fT;
        while (approximately_negative(fTs[++index].fT - startT))
            SkASSERT(index < tCount);
        SkASSERT(index < tCount);
        end = index;
    }

    bool unsortable(int index) const {
        return fTs[index].fUnsortableStart || fTs[index].fUnsortableEnd;
    }

    void updatePts(const SkPoint pts[]) {
        fPts = pts;
    }

    int updateOppWinding(int index, int endIndex) const {
        int lesser = SkMin32(index, endIndex);
        int oppWinding = oppSum(lesser);
        int oppSpanWinding = oppSign(index, endIndex);
        if (oppSpanWinding && useInnerWinding(oppWinding - oppSpanWinding, oppWinding)
                && oppWinding != SK_MaxS32) {
            oppWinding -= oppSpanWinding;
        }
        return oppWinding;
    }

    int updateOppWinding(const Angle* angle) const {
        int startIndex = angle->start();
        int endIndex = angle->end();
        return updateOppWinding(endIndex, startIndex);
    }

    int updateOppWindingReverse(const Angle* angle) const {
        int startIndex = angle->start();
        int endIndex = angle->end();
        return updateOppWinding(startIndex, endIndex);
    }

    int updateWinding(int index, int endIndex) const {
        int lesser = SkMin32(index, endIndex);
        int winding = windSum(lesser);
        int spanWinding = spanSign(index, endIndex);
        if (winding && useInnerWinding(winding - spanWinding, winding) && winding != SK_MaxS32) {
            winding -= spanWinding;
        }
        return winding;
    }

    int updateWinding(const Angle* angle) const {
        int startIndex = angle->start();
        int endIndex = angle->end();
        return updateWinding(endIndex, startIndex);
    }

    int updateWindingReverse(const Angle* angle) const {
        int startIndex = angle->start();
        int endIndex = angle->end();
        return updateWinding(startIndex, endIndex);
    }

    SkPath::Verb verb() const {
        return fVerb;
    }

    int windingAtT(double tHit, int tIndex, bool crossOpp, SkScalar& dx) const {
        if (approximately_zero(tHit - t(tIndex))) { // if we hit the end of a span, disregard
            return SK_MinS32;
        }
        int winding = crossOpp ? oppSum(tIndex) : windSum(tIndex);
        SkASSERT(winding != SK_MinS32);
        int windVal = crossOpp ? oppValue(tIndex) : windValue(tIndex);
    #if DEBUG_WINDING_AT_T
        SkDebugf("%s oldWinding=%d windValue=%d", __FUNCTION__, winding, windVal);
    #endif
        // see if a + change in T results in a +/- change in X (compute x'(T))
        dx = (*SegmentDXAtT[fVerb])(fPts, tHit);
        if (fVerb > SkPath::kLine_Verb && approximately_zero(dx)) {
            dx = fPts[2].fX - fPts[1].fX - dx;
        }
        if (dx == 0) {
    #if DEBUG_WINDING_AT_T
            SkDebugf(" dx=0 winding=SK_MinS32\n");
    #endif
            return SK_MinS32;
        }
        if (winding * dx > 0) { // if same signs, result is negative
            winding += dx > 0 ? -windVal : windVal;
        }
    #if DEBUG_WINDING_AT_T
        SkDebugf(" dx=%c winding=%d\n", dx > 0 ? '+' : '-', winding);
    #endif
        return winding;
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

    int windValueAt(double t) const {
        int count = fTs.count();
        for (int index = 0; index < count; ++index) {
            if (fTs[index].fT == t) {
                return fTs[index].fWindValue;
            }
        }
        SkASSERT(0);
        return 0;
    }

    SkScalar xAtT(int index) const {
        return xAtT(&fTs[index]);
    }

    SkScalar xAtT(const Span* span) const {
        return xyAtT(span).fX;
    }

    const SkPoint& xyAtT(int index) const {
        return xyAtT(&fTs[index]);
    }

    const SkPoint& xyAtT(const Span* span) const {
        if (SkScalarIsNaN(span->fPt.fX)) {
            SkASSERT(0); // make sure this path is never used
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

    // used only by right angle winding finding
    void xyAtT(double mid, SkPoint& pt) const {
        (*SegmentXYAtT[fVerb])(fPts, mid, &pt);
    }

    SkScalar yAtT(int index) const {
        return yAtT(&fTs[index]);
    }

    SkScalar yAtT(const Span* span) const {
        return xyAtT(span).fY;
    }

    void zeroCoincidentOpp(Span* oTest, int index) {
        Span* const test = &fTs[index];
        Span* end = test;
        do {
            end->fOppValue = 0;
            end = &fTs[++index];
        } while (approximately_negative(end->fT - test->fT));
    }

    void zeroCoincidentOther(Span* test, const double tRatio, const double oEndT, int oIndex) {
        Span* const oTest = &fTs[oIndex];
        Span* oEnd = oTest;
        const double startT = test->fT;
        const double oStartT = oTest->fT;
        double otherTMatch = (test->fT - startT) * tRatio + oStartT;
        while (!approximately_negative(oEndT - oEnd->fT)
                && approximately_negative(oEnd->fT - otherTMatch)) {
            oEnd->fOppValue = 0;
            oEnd = &fTs[++oIndex];
        }
    }

    void zeroSpan(Span* span) {
        SkASSERT(span->fWindValue > 0 || span->fOppValue > 0);
        span->fWindValue = 0;
        span->fOppValue = 0;
        SkASSERT(!span->fDone);
        span->fDone = true;
        ++fDoneSpans;
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
    // SkASSERT if pair has not already been added
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

#if DEBUG_WINDING
    void debugShowSums() const {
        SkDebugf("%s id=%d (%1.9g,%1.9g %1.9g,%1.9g)", __FUNCTION__, fID,
            fPts[0].fX, fPts[0].fY, fPts[fVerb].fX, fPts[fVerb].fY);
        for (int i = 0; i < fTs.count(); ++i) {
            const Span& span = fTs[i];
            SkDebugf(" [t=%1.3g %1.9g,%1.9g w=", span.fT, xAtT(&span), yAtT(&span));
            if (span.fWindSum == SK_MinS32) {
                SkDebugf("?");
            } else {
                SkDebugf("%d", span.fWindSum);
            }
            SkDebugf("]");
        }
        SkDebugf("\n");
    }
#endif

#if DEBUG_CONCIDENT
    void debugShowTs() const {
        SkDebugf("%s id=%d", __FUNCTION__, fID);
        int lastWind = -1;
        int lastOpp = -1;
        double lastT = -1;
        int i;
        for (i = 0; i < fTs.count(); ++i) {
            bool change = lastT != fTs[i].fT || lastWind != fTs[i].fWindValue
                    || lastOpp != fTs[i].fOppValue;
            if (change && lastWind >= 0) {
                SkDebugf(" t=%1.3g %1.9g,%1.9g w=%d o=%d]",
                        lastT, xyAtT(i - 1).fX, xyAtT(i - 1).fY, lastWind, lastOpp);
            }
            if (change) {
                SkDebugf(" [o=%d", fTs[i].fOther->fID);
                lastWind = fTs[i].fWindValue;
                lastOpp = fTs[i].fOppValue;
                lastT = fTs[i].fT;
            } else {
                SkDebugf(",%d", fTs[i].fOther->fID);
            }
        }
        if (i <= 0) {
            return;
        }
        SkDebugf(" t=%1.3g %1.9g,%1.9g w=%d o=%d]",
                lastT, xyAtT(i - 1).fX, xyAtT(i - 1).fY, lastWind, lastOpp);
        if (fOperand) {
            SkDebugf(" operand");
        }
        if (done()) {
            SkDebugf(" done");
        }
        SkDebugf("\n");
    }
#endif

#if DEBUG_ACTIVE_SPANS
    void debugShowActiveSpans() const {
        if (done()) {
            return;
        }
#if DEBUG_ACTIVE_SPANS_SHORT_FORM
        int lastId = -1;
        double lastT = -1;
#endif
        for (int i = 0; i < fTs.count(); ++i) {
            SkASSERT(&fTs[i] == &fTs[i].fOther->fTs[fTs[i].fOtherIndex].fOther->
                    fTs[fTs[i].fOther->fTs[fTs[i].fOtherIndex].fOtherIndex]);
            if (fTs[i].fDone) {
                continue;
            }
#if DEBUG_ACTIVE_SPANS_SHORT_FORM
            if (lastId == fID && lastT == fTs[i].fT) {
                continue;
            }
            lastId = fID;
            lastT = fTs[i].fT;
#endif
            SkDebugf("%s id=%d", __FUNCTION__, fID);
            SkDebugf(" (%1.9g,%1.9g", fPts[0].fX, fPts[0].fY);
            for (int vIndex = 1; vIndex <= fVerb; ++vIndex) {
                SkDebugf(" %1.9g,%1.9g", fPts[vIndex].fX, fPts[vIndex].fY);
            }
            const Span* span = &fTs[i];
            SkDebugf(") t=%1.9g (%1.9g,%1.9g)", fTs[i].fT,
                     xAtT(span), yAtT(span));
            int iEnd = i + 1;
            while (fTs[iEnd].fT < 1 && approximately_equal(fTs[i].fT, fTs[iEnd].fT)) {
                ++iEnd;
            }
            SkDebugf(" tEnd=%1.9g", fTs[iEnd].fT);
            const Segment* other = fTs[i].fOther;
            SkDebugf(" other=%d otherT=%1.9g otherIndex=%d windSum=",
                    other->fID, fTs[i].fOtherT, fTs[i].fOtherIndex);
            if (fTs[i].fWindSum == SK_MinS32) {
                SkDebugf("?");
            } else {
                SkDebugf("%d", fTs[i].fWindSum);
            }
            SkDebugf(" windValue=%d oppValue=%d\n", fTs[i].fWindValue, fTs[i].fOppValue);
        }
    }

    // This isn't useful yet -- but leaving it in for now in case i think of something
    // to use it for
    void validateActiveSpans() const {
        if (done()) {
            return;
        }
        int tCount = fTs.count();
        for (int index = 0; index < tCount; ++index) {
            if (fTs[index].fDone) {
                continue;
            }
            // count number of connections which are not done
            int first = index;
            double baseT = fTs[index].fT;
            while (first > 0 && approximately_equal(fTs[first - 1].fT, baseT)) {
                --first;
            }
            int last = index;
            while (last < tCount - 1 && approximately_equal(fTs[last + 1].fT, baseT)) {
                ++last;
            }
            int connections = 0;
            connections += first > 0 && !fTs[first - 1].fDone;
            for (int test = first; test <= last; ++test) {
                connections += !fTs[test].fDone;
                const Segment* other = fTs[test].fOther;
                int oIndex = fTs[test].fOtherIndex;
                connections += !other->fTs[oIndex].fDone;
                connections += oIndex > 0 && !other->fTs[oIndex - 1].fDone;
            }
      //      SkASSERT(!(connections & 1));
        }
    }
#endif

#if DEBUG_MARK_DONE || DEBUG_UNSORTABLE
    void debugShowNewWinding(const char* fun, const Span& span, int winding) {
        const SkPoint& pt = xyAtT(&span);
        SkDebugf("%s id=%d", fun, fID);
        SkDebugf(" (%1.9g,%1.9g", fPts[0].fX, fPts[0].fY);
        for (int vIndex = 1; vIndex <= fVerb; ++vIndex) {
            SkDebugf(" %1.9g,%1.9g", fPts[vIndex].fX, fPts[vIndex].fY);
        }
        SkASSERT(&span == &span.fOther->fTs[span.fOtherIndex].fOther->
                fTs[span.fOther->fTs[span.fOtherIndex].fOtherIndex]);
        SkDebugf(") t=%1.9g [%d] (%1.9g,%1.9g) tEnd=%1.9g newWindSum=%d windSum=",
                span.fT, span.fOther->fTs[span.fOtherIndex].fOtherIndex, pt.fX, pt.fY,
                (&span)[1].fT, winding);
        if (span.fWindSum == SK_MinS32) {
            SkDebugf("?");
        } else {
            SkDebugf("%d", span.fWindSum);
        }
        SkDebugf(" windValue=%d\n", span.fWindValue);
    }

    void debugShowNewWinding(const char* fun, const Span& span, int winding, int oppWinding) {
        const SkPoint& pt = xyAtT(&span);
        SkDebugf("%s id=%d", fun, fID);
        SkDebugf(" (%1.9g,%1.9g", fPts[0].fX, fPts[0].fY);
        for (int vIndex = 1; vIndex <= fVerb; ++vIndex) {
            SkDebugf(" %1.9g,%1.9g", fPts[vIndex].fX, fPts[vIndex].fY);
        }
        SkASSERT(&span == &span.fOther->fTs[span.fOtherIndex].fOther->
                fTs[span.fOther->fTs[span.fOtherIndex].fOtherIndex]);
        SkDebugf(") t=%1.9g [%d] (%1.9g,%1.9g) tEnd=%1.9g newWindSum=%d newOppSum=%d oppSum=",
                span.fT, span.fOther->fTs[span.fOtherIndex].fOtherIndex, pt.fX, pt.fY,
                (&span)[1].fT, winding, oppWinding);
        if (span.fOppSum == SK_MinS32) {
            SkDebugf("?");
        } else {
            SkDebugf("%d", span.fOppSum);
        }
        SkDebugf(" windSum=");
        if (span.fWindSum == SK_MinS32) {
            SkDebugf("?");
        } else {
            SkDebugf("%d", span.fWindSum);
        }
        SkDebugf(" windValue=%d\n", span.fWindValue);
    }
#endif

#if DEBUG_SORT || DEBUG_SWAP_TOP
    void debugShowSort(const char* fun, const SkTDArray<Angle*>& angles, int first,
            const int contourWinding, const int oppContourWinding) const {
        if (--gDebugSortCount < 0) {
            return;
        }
        SkASSERT(angles[first]->segment() == this);
        SkASSERT(angles.count() > 1);
        int lastSum = contourWinding;
        int oppLastSum = oppContourWinding;
        const Angle* firstAngle = angles[first];
        int windSum = lastSum - spanSign(firstAngle);
        int oppoSign = oppSign(firstAngle);
        int oppWindSum = oppLastSum - oppoSign;
        #define WIND_AS_STRING(x) char x##Str[12]; if (!valid_wind(x)) strcpy(x##Str, "?"); \
            else snprintf(x##Str, sizeof(x##Str), "%d", x)
        WIND_AS_STRING(contourWinding);
        WIND_AS_STRING(oppContourWinding);
        SkDebugf("%s %s contourWinding=%s oppContourWinding=%s sign=%d\n", fun, __FUNCTION__,
                contourWindingStr, oppContourWindingStr, spanSign(angles[first]));
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
            bool opp = segment.fOperand ^ fOperand;
            if (!firstTime) {
                oppoSign = segment.oppSign(&angle);
                if (opp) {
                    oppLastSum = oppWindSum;
                    oppWindSum -= segment.spanSign(&angle);
                    if (oppoSign) {
                        lastSum = windSum;
                        windSum -= oppoSign;
                    }
                } else {
                    lastSum = windSum;
                    windSum -= segment.spanSign(&angle);
                    if (oppoSign) {
                        oppLastSum = oppWindSum;
                        oppWindSum -= oppoSign;
                    }
                }
            }
            SkDebugf("%s [%d] %s", __FUNCTION__, index,
                    angle.unsortable() ? "*** UNSORTABLE *** " : "");
        #if COMPACT_DEBUG_SORT
            SkDebugf("id=%d %s start=%d (%1.9g,%,1.9g) end=%d (%1.9g,%,1.9g)",
                    segment.fID, kLVerbStr[segment.fVerb],
                    start, segment.xAtT(&sSpan), segment.yAtT(&sSpan), end,
                    segment.xAtT(&eSpan), segment.yAtT(&eSpan));
        #else
            switch (segment.fVerb) {
                case SkPath::kLine_Verb:
                    SkDebugf(LINE_DEBUG_STR, LINE_DEBUG_DATA(segment.fPts));
                    break;
                case SkPath::kQuad_Verb:
                    SkDebugf(QUAD_DEBUG_STR, QUAD_DEBUG_DATA(segment.fPts));
                    break;
                case SkPath::kCubic_Verb:
                    SkDebugf(CUBIC_DEBUG_STR, CUBIC_DEBUG_DATA(segment.fPts));
                    break;
                default:
                    SkASSERT(0);
            }
            SkDebugf(" tStart=%1.9g tEnd=%1.9g", sSpan.fT, eSpan.fT);
        #endif
            SkDebugf(" sign=%d windValue=%d windSum=", angle.sign(), mSpan.fWindValue);
            winding_printf(mSpan.fWindSum);
            int last, wind;
            if (opp) {
                last = oppLastSum;
                wind = oppWindSum;
            } else {
                last = lastSum;
                wind = windSum;
            }
            bool useInner = valid_wind(last) && valid_wind(wind) && useInnerWinding(last, wind);
            WIND_AS_STRING(last);
            WIND_AS_STRING(wind);
            WIND_AS_STRING(lastSum);
            WIND_AS_STRING(oppLastSum);
            WIND_AS_STRING(windSum);
            WIND_AS_STRING(oppWindSum);
            #undef WIND_AS_STRING
            if (!oppoSign) {
                SkDebugf(" %s->%s (max=%s)", lastStr, windStr, useInner ? windStr : lastStr);
            } else {
                SkDebugf(" %s->%s (%s->%s)", lastStr, windStr, opp ? lastSumStr : oppLastSumStr,
                        opp ? windSumStr : oppWindSumStr);
            }
            SkDebugf(" done=%d tiny=%d opp=%d\n", mSpan.fDone, mSpan.fTiny, opp);
#if false && DEBUG_ANGLE
            angle.debugShow(segment.xyAtT(&sSpan));
#endif
            ++index;
            if (index == angles.count()) {
                index = 0;
            }
            if (firstTime) {
                firstTime = false;
            }
        } while (index != first);
    }

    void debugShowSort(const char* fun, const SkTDArray<Angle*>& angles, int first) {
        const Angle* firstAngle = angles[first];
        const Segment* segment = firstAngle->segment();
        int winding = segment->updateWinding(firstAngle);
        int oppWinding = segment->updateOppWinding(firstAngle);
        debugShowSort(fun, angles, first, winding, oppWinding);
    }

#endif

#if DEBUG_WINDING
    static char as_digit(int value) {
        return value < 0 ? '?' : value <= 9 ? '0' + value : '+';
    }
#endif

#if DEBUG_SHOW_WINDING
    int debugShowWindingValues(int slotCount, int ofInterest) const {
        if (!(1 << fID & ofInterest)) {
            return 0;
        }
        int sum = 0;
        SkTDArray<char> slots;
        slots.setCount(slotCount * 2);
        memset(slots.begin(), ' ', slotCount * 2);
        for (int i = 0; i < fTs.count(); ++i) {
       //     if (!(1 << fTs[i].fOther->fID & ofInterest)) {
       //         continue;
       //     }
            sum += fTs[i].fWindValue;
            slots[fTs[i].fOther->fID - 1] = as_digit(fTs[i].fWindValue);
            sum += fTs[i].fOppValue;
            slots[slotCount + fTs[i].fOther->fID - 1] = as_digit(fTs[i].fOppValue);
        }
        SkDebugf("%s id=%2d %.*s | %.*s\n", __FUNCTION__, fID, slotCount, slots.begin(), slotCount,
                slots.begin() + slotCount);
        return sum;
    }
#endif

private:
    const SkPoint* fPts;
    Bounds fBounds;
    SkTDArray<Span> fTs; // two or more (always includes t=0 t=1)
    // OPTIMIZATION: could pack donespans, verb, operand, xor into 1 int-sized value
    int fDoneSpans; // quick check that segment is finished
    // OPTIMIZATION: force the following to be byte-sized
    SkPath::Verb fVerb;
    bool fOperand;
    bool fXor; // set if original contour had even-odd fill
    bool fOppXor; // set if opposite operand had even-odd fill
#if DEBUG_DUMP
    int fID;
#endif
};

class Contour;

struct Coincidence {
    Contour* fContours[2];
    int fSegments[2];
    double fTs[2][2];
    SkPoint fPts[2];
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
        coincidence.fContours[0] = this; // FIXME: no need to store
        coincidence.fContours[1] = other;
        coincidence.fSegments[0] = index;
        coincidence.fSegments[1] = otherIndex;
        coincidence.fTs[swap][0] = ts.fT[0][0];
        coincidence.fTs[swap][1] = ts.fT[0][1];
        coincidence.fTs[!swap][0] = ts.fT[1][0];
        coincidence.fTs[!swap][1] = ts.fT[1][1];
        coincidence.fPts[0] = ts.fPt[0].asSkPoint();
        coincidence.fPts[1] = ts.fPt[1].asSkPoint();
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
        fSegments.push_back().addCubic(pts, fOperand, fXor);
        fContainsCurves = fContainsCubics = true;
    }

    int addLine(const SkPoint pts[2]) {
        fSegments.push_back().addLine(pts, fOperand, fXor);
        return fSegments.count();
    }

    void addOtherT(int segIndex, int tIndex, double otherT, int otherIndex) {
        fSegments[segIndex].addOtherT(tIndex, otherT, otherIndex);
    }

    int addQuad(const SkPoint pts[3]) {
        fSegments.push_back().addQuad(pts, fOperand, fXor);
        fContainsCurves = true;
        return fSegments.count();
    }

    int addT(int segIndex, Contour* other, int otherIndex, const SkPoint& pt, double& newT) {
        setContainsIntercepts();
        return fSegments[segIndex].addT(&other->fSegments[otherIndex], pt, newT);
    }

    int addSelfT(int segIndex, Contour* other, int otherIndex, const SkPoint& pt, double& newT) {
        setContainsIntercepts();
        return fSegments[segIndex].addSelfT(&other->fSegments[otherIndex], pt, newT);
    }

    int addUnsortableT(int segIndex, Contour* other, int otherIndex, bool start,
            const SkPoint& pt, double& newT) {
        return fSegments[segIndex].addUnsortableT(&other->fSegments[otherIndex], start, pt, newT);
    }

    const Bounds& bounds() const {
        return fBounds;
    }

    void complete() {
        setBounds();
        fContainsIntercepts = false;
    }

    bool containsCubics() const {
        return fContainsCubics;
    }

    bool crosses(const Contour* crosser) const {
        for (int index = 0; index < fCrosses.count(); ++index) {
            if (fCrosses[index] == crosser) {
                return true;
            }
        }
        return false;
    }

    bool done() const {
        return fDone;
    }

    const SkPoint& end() const {
        const Segment& segment = fSegments.back();
        return segment.pts()[segment.verb()];
    }

    void findTooCloseToCall() {
        int segmentCount = fSegments.count();
        for (int sIndex = 0; sIndex < segmentCount; ++sIndex) {
            fSegments[sIndex].findTooCloseToCall();
        }
    }

    void fixOtherTIndex() {
        int segmentCount = fSegments.count();
        for (int sIndex = 0; sIndex < segmentCount; ++sIndex) {
            fSegments[sIndex].fixOtherTIndex();
        }
    }

    Segment* nonVerticalSegment(int& start, int& end) {
        int segmentCount = fSortedSegments.count();
        SkASSERT(segmentCount > 0);
        for (int sortedIndex = fFirstSorted; sortedIndex < segmentCount; ++sortedIndex) {
            Segment* testSegment = fSortedSegments[sortedIndex];
            if (testSegment->done()) {
                continue;
            }
            start = end = 0;
            while (testSegment->nextCandidate(start, end)) {
                if (!testSegment->isVertical(start, end)) {
                    return testSegment;
                }
            }
        }
        return NULL;
    }

    bool operand() const {
        return fOperand;
    }

    void reset() {
        fSegments.reset();
        fBounds.set(SK_ScalarMax, SK_ScalarMax, SK_ScalarMax, SK_ScalarMax);
        fContainsCurves = fContainsCubics = fContainsIntercepts = fDone = false;
    }

    void resolveCoincidence(SkTDArray<Contour*>& contourList) {
        int count = fCoincidences.count();
        for (int index = 0; index < count; ++index) {
            Coincidence& coincidence = fCoincidences[index];
            SkASSERT(coincidence.fContours[0] == this);
            int thisIndex = coincidence.fSegments[0];
            Segment& thisOne = fSegments[thisIndex];
            Contour* otherContour = coincidence.fContours[1];
            int otherIndex = coincidence.fSegments[1];
            Segment& other = otherContour->fSegments[otherIndex];
            if ((thisOne.done() || other.done()) && thisOne.complete() && other.complete()) {
                continue;
            }
        #if DEBUG_CONCIDENT
            thisOne.debugShowTs();
            other.debugShowTs();
        #endif
            double startT = coincidence.fTs[0][0];
            double endT = coincidence.fTs[0][1];
            bool cancelers = false;
            if (startT > endT) {
                SkTSwap<double>(startT, endT);
                cancelers ^= true; // FIXME: just assign true
            }
            SkASSERT(!approximately_negative(endT - startT));
            double oStartT = coincidence.fTs[1][0];
            double oEndT = coincidence.fTs[1][1];
            if (oStartT > oEndT) {
                SkTSwap<double>(oStartT, oEndT);
                cancelers ^= true;
            }
            SkASSERT(!approximately_negative(oEndT - oStartT));
            bool opp = fOperand ^ otherContour->fOperand;
            if (cancelers && !opp) {
                // make sure startT and endT have t entries
                if (startT > 0 || oEndT < 1
                        || thisOne.isMissing(startT) || other.isMissing(oEndT)) {
                    thisOne.addTPair(startT, other, oEndT, true, coincidence.fPts[0]);
                }
                if (oStartT > 0 || endT < 1
                        || thisOne.isMissing(endT) || other.isMissing(oStartT)) {
                    other.addTPair(oStartT, thisOne, endT, true, coincidence.fPts[1]);
                }
                if (!thisOne.done() && !other.done()) {
                    thisOne.addTCancel(startT, endT, other, oStartT, oEndT);
                }
            } else {
                if (startT > 0 || oStartT > 0
                        || thisOne.isMissing(startT) || other.isMissing(oStartT)) {
                    thisOne.addTPair(startT, other, oStartT, true, coincidence.fPts[0]);
                }
                if (endT < 1 || oEndT < 1
                        || thisOne.isMissing(endT) || other.isMissing(oEndT)) {
                    other.addTPair(oEndT, thisOne, endT, true, coincidence.fPts[1]);
                }
                if (!thisOne.done() && !other.done()) {
                    thisOne.addTCoincident(startT, endT, other, oStartT, oEndT);
                }
            }
        #if DEBUG_CONCIDENT
            thisOne.debugShowTs();
            other.debugShowTs();
        #endif
        #if DEBUG_SHOW_WINDING
            debugShowWindingValues(contourList);
        #endif
        }
    }

    // first pass, add missing T values
    // second pass, determine winding values of overlaps
    void addCoincidentPoints() {
        int count = fCoincidences.count();
        for (int index = 0; index < count; ++index) {
            Coincidence& coincidence = fCoincidences[index];
            SkASSERT(coincidence.fContours[0] == this);
            int thisIndex = coincidence.fSegments[0];
            Segment& thisOne = fSegments[thisIndex];
            Contour* otherContour = coincidence.fContours[1];
            int otherIndex = coincidence.fSegments[1];
            Segment& other = otherContour->fSegments[otherIndex];
            if ((thisOne.done() || other.done()) && thisOne.complete() && other.complete()) {
                // OPTIMIZATION: remove from array
                continue;
            }
        #if DEBUG_CONCIDENT
            thisOne.debugShowTs();
            other.debugShowTs();
        #endif
            double startT = coincidence.fTs[0][0];
            double endT = coincidence.fTs[0][1];
            bool cancelers;
            if ((cancelers = startT > endT)) {
                SkTSwap(startT, endT);
                SkTSwap(coincidence.fPts[0], coincidence.fPts[1]);
            }
            SkASSERT(!approximately_negative(endT - startT));
            double oStartT = coincidence.fTs[1][0];
            double oEndT = coincidence.fTs[1][1];
            if (oStartT > oEndT) {
                SkTSwap<double>(oStartT, oEndT);
                cancelers ^= true;
            }
            SkASSERT(!approximately_negative(oEndT - oStartT));
            bool opp = fOperand ^ otherContour->fOperand;
            if (cancelers && !opp) {
                // make sure startT and endT have t entries
                if (startT > 0 || oEndT < 1
                        || thisOne.isMissing(startT) || other.isMissing(oEndT)) {
                    thisOne.addTPair(startT, other, oEndT, true, coincidence.fPts[0]);
                }
                if (oStartT > 0 || endT < 1
                        || thisOne.isMissing(endT) || other.isMissing(oStartT)) {
                    other.addTPair(oStartT, thisOne, endT, true, coincidence.fPts[1]);
                }
            } else {
                if (startT > 0 || oStartT > 0
                        || thisOne.isMissing(startT) || other.isMissing(oStartT)) {
                    thisOne.addTPair(startT, other, oStartT, true, coincidence.fPts[0]);
                }
                if (endT < 1 || oEndT < 1
                        || thisOne.isMissing(endT) || other.isMissing(oEndT)) {
                    other.addTPair(oEndT, thisOne, endT, true, coincidence.fPts[1]);
                }
            }
        #if DEBUG_CONCIDENT
            thisOne.debugShowTs();
            other.debugShowTs();
        #endif
        }
    }

    void calcCoincidentWinding() {
        int count = fCoincidences.count();
        for (int index = 0; index < count; ++index) {
            Coincidence& coincidence = fCoincidences[index];
            SkASSERT(coincidence.fContours[0] == this);
            int thisIndex = coincidence.fSegments[0];
            Segment& thisOne = fSegments[thisIndex];
            if (thisOne.done()) {
                continue;
            }
            Contour* otherContour = coincidence.fContours[1];
            int otherIndex = coincidence.fSegments[1];
            Segment& other = otherContour->fSegments[otherIndex];
            if (other.done()) {
                continue;
            }
            double startT = coincidence.fTs[0][0];
            double endT = coincidence.fTs[0][1];
            bool cancelers;
            if ((cancelers = startT > endT)) {
                SkTSwap<double>(startT, endT);
            }
            SkASSERT(!approximately_negative(endT - startT));
            double oStartT = coincidence.fTs[1][0];
            double oEndT = coincidence.fTs[1][1];
            if (oStartT > oEndT) {
                SkTSwap<double>(oStartT, oEndT);
                cancelers ^= true;
            }
            SkASSERT(!approximately_negative(oEndT - oStartT));
            bool opp = fOperand ^ otherContour->fOperand;
            if (cancelers && !opp) {
                // make sure startT and endT have t entries
                if (!thisOne.done() && !other.done()) {
                    thisOne.addTCancel(startT, endT, other, oStartT, oEndT);
                }
            } else {
                if (!thisOne.done() && !other.done()) {
                    thisOne.addTCoincident(startT, endT, other, oStartT, oEndT);
                }
            }
        #if DEBUG_CONCIDENT
            thisOne.debugShowTs();
            other.debugShowTs();
        #endif
        }
    }

    SkTArray<Segment>& segments() {
        return fSegments;
    }

    void setContainsIntercepts() {
        fContainsIntercepts = true;
    }

    void setOperand(bool isOp) {
        fOperand = isOp;
    }

    void setOppXor(bool isOppXor) {
        fOppXor = isOppXor;
        int segmentCount = fSegments.count();
        for (int test = 0; test < segmentCount; ++test) {
            fSegments[test].setOppXor(isOppXor);
        }
    }

    void setXor(bool isXor) {
        fXor = isXor;
    }

    void sortSegments() {
        int segmentCount = fSegments.count();
        fSortedSegments.setReserve(segmentCount);
        for (int test = 0; test < segmentCount; ++test) {
            *fSortedSegments.append() = &fSegments[test];
        }
        QSort<Segment>(fSortedSegments.begin(), fSortedSegments.end() - 1);
        fFirstSorted = 0;
    }

    const SkPoint& start() const {
        return fSegments.front().pts()[0];
    }

    void toPath(PathWrapper& path) const {
        int segmentCount = fSegments.count();
        const SkPoint& pt = fSegments.front().pts()[0];
        path.deferredMove(pt);
        for (int test = 0; test < segmentCount; ++test) {
            fSegments[test].addCurveTo(0, 1, path, true);
        }
        path.close();
    }

    void toPartialBackward(PathWrapper& path) const {
        int segmentCount = fSegments.count();
        for (int test = segmentCount - 1; test >= 0; --test) {
            fSegments[test].addCurveTo(1, 0, path, true);
        }
    }

    void toPartialForward(PathWrapper& path) const {
        int segmentCount = fSegments.count();
        for (int test = 0; test < segmentCount; ++test) {
            fSegments[test].addCurveTo(0, 1, path, true);
        }
    }

    void topSortableSegment(const SkPoint& topLeft, SkPoint& bestXY, Segment*& topStart) {
        int segmentCount = fSortedSegments.count();
        SkASSERT(segmentCount > 0);
        int sortedIndex = fFirstSorted;
        fDone = true; // may be cleared below
        for ( ; sortedIndex < segmentCount; ++sortedIndex) {
            Segment* testSegment = fSortedSegments[sortedIndex];
            if (testSegment->done()) {
                if (sortedIndex == fFirstSorted) {
                    ++fFirstSorted;
                }
                continue;
            }
            fDone = false;
            SkPoint testXY = testSegment->activeLeftTop(true, NULL);
            if (topStart) {
                if (testXY.fY < topLeft.fY) {
                    continue;
                }
                if (testXY.fY == topLeft.fY && testXY.fX < topLeft.fX) {
                    continue;
                }
                if (bestXY.fY < testXY.fY) {
                    continue;
                }
                if (bestXY.fY == testXY.fY && bestXY.fX < testXY.fX) {
                    continue;
                }
            }
            topStart = testSegment;
            bestXY = testXY;
        }
    }

    Segment* undoneSegment(int& start, int& end) {
        int segmentCount = fSegments.count();
        for (int test = 0; test < segmentCount; ++test) {
            Segment* testSegment = &fSegments[test];
            if (testSegment->done()) {
                continue;
            }
            testSegment->undoneSpan(start, end);
            return testSegment;
        }
        return NULL;
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

    void validateActiveSpans() {
        for (int index = 0; index < fSegments.count(); ++index) {
            fSegments[index].validateActiveSpans();
        }
    }
#endif

#if DEBUG_SHOW_WINDING
    int debugShowWindingValues(int totalSegments, int ofInterest) {
        int count = fSegments.count();
        int sum = 0;
        for (int index = 0; index < count; ++index) {
            sum += fSegments[index].debugShowWindingValues(totalSegments, ofInterest);
        }
  //      SkDebugf("%s sum=%d\n", __FUNCTION__, sum);
        return sum;
    }

    static void debugShowWindingValues(SkTDArray<Contour*>& contourList) {
   //     int ofInterest = 1 << 1 | 1 << 5 | 1 << 9 | 1 << 13;
    //    int ofInterest = 1 << 4 | 1 << 8 | 1 << 12 | 1 << 16;
        int ofInterest = 1 << 5 | 1 << 8;
        int total = 0;
        int index;
        for (index = 0; index < contourList.count(); ++index) {
            total += contourList[index]->segments().count();
        }
        int sum = 0;
        for (index = 0; index < contourList.count(); ++index) {
            sum += contourList[index]->debugShowWindingValues(total, ofInterest);
        }
 //       SkDebugf("%s total=%d\n", __FUNCTION__, sum);
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
    SkTDArray<Segment*> fSortedSegments;
    int fFirstSorted;
    SkTDArray<Coincidence> fCoincidences;
    SkTDArray<const Contour*> fCrosses;
    Bounds fBounds;
    bool fContainsIntercepts; // FIXME: is this used by anybody?
    bool fContainsCubics;
    bool fContainsCurves;
    bool fDone;
    bool fOperand; // true for the second argument to a binary operator
    bool fXor;
    bool fOppXor;
#if DEBUG_DUMP
    int fID;
#endif
};

class EdgeBuilder {
public:

EdgeBuilder(const PathWrapper& path, SkTArray<Contour>& contours)
    : fPath(path.nativePath())
    , fContours(contours)
{
    init();
}

EdgeBuilder(const SkPath& path, SkTArray<Contour>& contours)
    : fPath(&path)
    , fContours(contours)
{
    init();
}

void init() {
    fCurrentContour = NULL;
    fOperand = false;
    fXorMask[0] = fXorMask[1] = (fPath->getFillType() & 1) ? kEvenOdd_Mask : kWinding_Mask;
#if DEBUG_DUMP
    gContourID = 0;
    gSegmentID = 0;
#endif
    fSecondHalf = preFetch();
}

void addOperand(const SkPath& path) {
    SkASSERT(fPathVerbs.count() > 0 && fPathVerbs.end()[-1] == SkPath::kDone_Verb);
    fPathVerbs.pop();
    fPath = &path;
    fXorMask[1] = (fPath->getFillType() & 1) ? kEvenOdd_Mask : kWinding_Mask;
    preFetch();
}

void finish() {
    walk();
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

ShapeOpMask xorMask() const {
    return fXorMask[fOperand];
}

protected:

void complete() {
    if (fCurrentContour && fCurrentContour->segments().count()) {
        fCurrentContour->complete();
        fCurrentContour = NULL;
    }
}

// FIXME:remove once we can access path pts directly
int preFetch() {
    SkPath::RawIter iter(*fPath); // FIXME: access path directly when allowed
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
    return fPathVerbs.count() - 1;
}

void walk() {
    SkPath::Verb reducedVerb;
    uint8_t* verbPtr = fPathVerbs.begin();
    uint8_t* endOfFirstHalf = &verbPtr[fSecondHalf];
    const SkPoint* pointsPtr = fPathPts.begin();
    const SkPoint* finalCurveStart = NULL;
    const SkPoint* finalCurveEnd = NULL;
    SkPath::Verb verb;
    while ((verb = (SkPath::Verb) *verbPtr++) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kMove_Verb:
                complete();
                if (!fCurrentContour) {
                    fCurrentContour = fContours.push_back_n(1);
                    fCurrentContour->setOperand(fOperand);
                    fCurrentContour->setXor(fXorMask[fOperand] == kEvenOdd_Mask);
                    *fExtra.append() = -1; // start new contour
                }
                finalCurveEnd = pointsPtr++;
                goto nextVerb;
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
                goto nextVerb;
            default:
                SkDEBUGFAIL("bad verb");
                return;
        }
        finalCurveStart = &pointsPtr[verb - 1];
        pointsPtr += verb;
        SkASSERT(fCurrentContour);
    nextVerb:
        if (verbPtr == endOfFirstHalf) {
            fOperand = true;
        }
    }
}

private:
    const SkPath* fPath;
    SkTDArray<SkPoint> fPathPts; // FIXME: point directly to path pts instead
    SkTDArray<uint8_t> fPathVerbs; // FIXME: remove
    Contour* fCurrentContour;
    SkTArray<Contour>& fContours;
    SkTDArray<SkPoint> fReducePts; // segments created on the fly
    SkTDArray<int> fExtra; // -1 marks new contour, > 0 offsets into contour
    ShapeOpMask fXorMask[2];
    int fSecondHalf;
    bool fOperand;
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
    int addT(const Work& other, const SkPoint& pt, double& newT) {
        return fContour->addT(fIndex, other.fContour, other.fIndex, pt, newT);
    }

    int addSelfT(const Work& other, const SkPoint& pt, double& newT) {
        return fContour->addSelfT(fIndex, other.fContour, other.fIndex, pt, newT);
    }

    int addUnsortableT(const Work& other, bool start, const SkPoint& pt, double& newT) {
        return fContour->addUnsortableT(fIndex, other.fContour, other.fIndex, start, pt, newT);
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

#if !APPROXIMATE_CUBICS
    const SkPoint* cubic() const {
        return fCubic;
    }
#endif

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

#if !APPROXIMATE_CUBICS
    void promoteToCubic() {
        fCubic[0] = pts()[0];
        fCubic[2] = pts()[1];
        fCubic[3] = pts()[2];
        fCubic[1].fX = (fCubic[0].fX + fCubic[2].fX * 2) / 3;
        fCubic[1].fY = (fCubic[0].fY + fCubic[2].fY * 2) / 3;
        fCubic[2].fX = (fCubic[3].fX + fCubic[2].fX * 2) / 3;
        fCubic[2].fY = (fCubic[3].fY + fCubic[2].fY * 2) / 3;
    }
#endif

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
#if !APPROXIMATE_CUBICS
    SkPoint fCubic[4];
#endif
    int fIndex;
    int fLast;
};

#if DEBUG_ADD_INTERSECTING_TS

static void debugShowLineIntersection(int pts, const Work& wt, const Work& wn,
        const Intersections& i) {
    SkASSERT(i.used() == pts);
    if (!pts) {
        SkDebugf("%s no intersect " LINE_DEBUG_STR " " LINE_DEBUG_STR "\n",
                __FUNCTION__, LINE_DEBUG_DATA(wt.pts()), LINE_DEBUG_DATA(wn.pts()));
        return;
    }
    SkDebugf("%s " T_DEBUG_STR(wtTs, 0) " " LINE_DEBUG_STR " " PT_DEBUG_STR, __FUNCTION__,
            i.fT[0][0], LINE_DEBUG_DATA(wt.pts()), PT_DEBUG_DATA(i, 0));
    if (pts == 2) {
        SkDebugf(" " T_DEBUG_STR(wtTs, 1) " " PT_DEBUG_STR, i.fT[0][1], PT_DEBUG_DATA(i, 1));
    }
    SkDebugf(" wnTs[0]=%g " LINE_DEBUG_STR, i.fT[1][0], LINE_DEBUG_DATA(wn.pts()));
    if (pts == 2) {
        SkDebugf(" " T_DEBUG_STR(wnTs, 1), i.fT[1][1]);
    }
    SkDebugf("\n");
}

static void debugShowQuadLineIntersection(int pts, const Work& wt,
        const Work& wn, const Intersections& i) {
    SkASSERT(i.used() == pts);
    if (!pts) {
        SkDebugf("%s no intersect " QUAD_DEBUG_STR " " LINE_DEBUG_STR "\n",
                __FUNCTION__, QUAD_DEBUG_DATA(wt.pts()), LINE_DEBUG_DATA(wn.pts()));
        return;
    }
    SkDebugf("%s " T_DEBUG_STR(wtTs, 0) " " QUAD_DEBUG_STR " " PT_DEBUG_STR, __FUNCTION__,
            i.fT[0][0], QUAD_DEBUG_DATA(wt.pts()), PT_DEBUG_DATA(i, 0));
    for (int n = 1; n < pts; ++n) {
        SkDebugf(" " TX_DEBUG_STR(wtTs) " " PT_DEBUG_STR, n, i.fT[0][n], PT_DEBUG_DATA(i, n));
    }
    SkDebugf(" wnTs[0]=%g " LINE_DEBUG_STR, i.fT[1][0], LINE_DEBUG_DATA(wn.pts()));
    for (int n = 1; n < pts; ++n) {
        SkDebugf(" " TX_DEBUG_STR(wnTs), n, i.fT[1][n]);
    }
    SkDebugf("\n");
}

static void debugShowQuadIntersection(int pts, const Work& wt,
        const Work& wn, const Intersections& i) {
    SkASSERT(i.used() == pts);
    if (!pts) {
        SkDebugf("%s no intersect " QUAD_DEBUG_STR " " QUAD_DEBUG_STR "\n",
                __FUNCTION__, QUAD_DEBUG_DATA(wt.pts()), QUAD_DEBUG_DATA(wn.pts()));
        return;
    }
    SkDebugf("%s " T_DEBUG_STR(wtTs, 0) " " QUAD_DEBUG_STR " " PT_DEBUG_STR, __FUNCTION__,
            i.fT[0][0], QUAD_DEBUG_DATA(wt.pts()), PT_DEBUG_DATA(i, 0));
    for (int n = 1; n < pts; ++n) {
        SkDebugf(" " TX_DEBUG_STR(wtTs) " " PT_DEBUG_STR, n, i.fT[0][n], PT_DEBUG_DATA(i, n));
    }
    SkDebugf(" wnTs[0]=%g " QUAD_DEBUG_STR, i.fT[1][0], QUAD_DEBUG_DATA(wn.pts()));
    for (int n = 1; n < pts; ++n) {
        SkDebugf(" " TX_DEBUG_STR(wnTs), n, i.fT[1][n]);
    }
    SkDebugf("\n");
}

static void debugShowCubicLineIntersection(int pts, const Work& wt,
        const Work& wn, const Intersections& i) {
    SkASSERT(i.used() == pts);
    if (!pts) {
        SkDebugf("%s no intersect " CUBIC_DEBUG_STR " " LINE_DEBUG_STR "\n",
                __FUNCTION__, CUBIC_DEBUG_DATA(wt.pts()), LINE_DEBUG_DATA(wn.pts()));
        return;
    }
    SkDebugf("%s " T_DEBUG_STR(wtTs, 0) " " CUBIC_DEBUG_STR " " PT_DEBUG_STR, __FUNCTION__,
            i.fT[0][0], CUBIC_DEBUG_DATA(wt.pts()), PT_DEBUG_DATA(i, 0));
    for (int n = 1; n < pts; ++n) {
        SkDebugf(" " TX_DEBUG_STR(wtTs) " " PT_DEBUG_STR, n, i.fT[0][n], PT_DEBUG_DATA(i, n));
    }
    SkDebugf(" wnTs[0]=%g " LINE_DEBUG_STR, i.fT[1][0], LINE_DEBUG_DATA(wn.pts()));
    for (int n = 1; n < pts; ++n) {
        SkDebugf(" " TX_DEBUG_STR(wnTs), n, i.fT[1][n]);
    }
    SkDebugf("\n");
}

static void debugShowCubicQuadIntersection(int pts, const Work& wt,
        const Work& wn, const Intersections& i) {
    SkASSERT(i.used() == pts);
    if (!pts) {
        SkDebugf("%s no intersect " CUBIC_DEBUG_STR " " QUAD_DEBUG_STR "\n",
                __FUNCTION__, CUBIC_DEBUG_DATA(wt.pts()), QUAD_DEBUG_DATA(wn.pts()));
        return;
    }
    SkDebugf("%s " T_DEBUG_STR(wtTs, 0) " " CUBIC_DEBUG_STR " " PT_DEBUG_STR, __FUNCTION__,
            i.fT[0][0], CUBIC_DEBUG_DATA(wt.pts()), PT_DEBUG_DATA(i, 0));
    for (int n = 1; n < pts; ++n) {
        SkDebugf(" " TX_DEBUG_STR(wtTs) " " PT_DEBUG_STR, n, i.fT[0][n], PT_DEBUG_DATA(i, n));
    }
    SkDebugf(" wnTs[0]=%g " QUAD_DEBUG_STR, i.fT[1][0], QUAD_DEBUG_DATA(wn.pts()));
    for (int n = 1; n < pts; ++n) {
        SkDebugf(" " TX_DEBUG_STR(wnTs), n, i.fT[1][n]);
    }
    SkDebugf("\n");
}

static void debugShowCubicIntersection(int pts, const Work& wt,
        const Work& wn, const Intersections& i) {
    SkASSERT(i.used() == pts);
    if (!pts) {
        SkDebugf("%s no intersect " CUBIC_DEBUG_STR " " CUBIC_DEBUG_STR "\n",
                __FUNCTION__, CUBIC_DEBUG_DATA(wt.pts()), CUBIC_DEBUG_DATA(wn.pts()));
        return;
    }
    SkDebugf("%s " T_DEBUG_STR(wtTs, 0) " " CUBIC_DEBUG_STR " " PT_DEBUG_STR, __FUNCTION__,
            i.fT[0][0], CUBIC_DEBUG_DATA(wt.pts()), PT_DEBUG_DATA(i, 0));
    for (int n = 1; n < pts; ++n) {
        SkDebugf(" " TX_DEBUG_STR(wtTs) " " PT_DEBUG_STR, n, i.fT[0][n], PT_DEBUG_DATA(i, n));
    }
    SkDebugf(" wnTs[0]=%g " CUBIC_DEBUG_STR, i.fT[1][0], CUBIC_DEBUG_DATA(wn.pts()));
    for (int n = 1; n < pts; ++n) {
        SkDebugf(" " TX_DEBUG_STR(wnTs), n, i.fT[1][n]);
    }
    SkDebugf("\n");
}

static void debugShowCubicIntersection(int pts, const Work& wt, const Intersections& i) {
    SkASSERT(i.used() == pts);
    if (!pts) {
        SkDebugf("%s no self intersect " CUBIC_DEBUG_STR "\n", __FUNCTION__,
                CUBIC_DEBUG_DATA(wt.pts()));
        return;
    }
    SkDebugf("%s " T_DEBUG_STR(wtTs, 0) " " CUBIC_DEBUG_STR " " PT_DEBUG_STR, __FUNCTION__,
            i.fT[0][0], CUBIC_DEBUG_DATA(wt.pts()), PT_DEBUG_DATA(i, 0));
    SkDebugf(" " T_DEBUG_STR(wtTs, 1), i.fT[1][0]);
    SkDebugf("\n");
}

#else
static void debugShowLineIntersection(int , const Work& , const Work& , const Intersections& ) {
}

static void debugShowQuadLineIntersection(int , const Work& , const Work& , const Intersections& ) {
}

static void debugShowQuadIntersection(int , const Work& , const Work& , const Intersections& ) {
}

static void debugShowCubicLineIntersection(int , const Work& , const Work& ,
        const Intersections& ) {
}

static void debugShowCubicQuadIntersection(int , const Work& , const Work& ,
        const Intersections& ) {
}

static void debugShowCubicIntersection(int , const Work& , const Work& , const Intersections& ) {
}

static void debugShowCubicIntersection(int , const Work& , const Intersections& ) {
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
                            debugShowLineIntersection(pts, wt, wn, ts);
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
                            debugShowCubicLineIntersection(pts, wn, wt, ts);
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
                            debugShowLineIntersection(pts, wt, wn, ts);
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
                            debugShowCubicLineIntersection(pts, wn, wt, ts);
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
                            debugShowLineIntersection(pts, wt, wn, ts);
                            break;
                        case Work::kVerticalLine_Segment:
                            pts = VLineIntersect(wt.pts(), wn.top(),
                                    wn.bottom(), wn.x(), wn.yFlipped(), ts);
                            debugShowLineIntersection(pts, wt, wn, ts);
                            break;
                        case Work::kLine_Segment: {
                            pts = LineIntersect(wt.pts(), wn.pts(), ts);
                            debugShowLineIntersection(pts, wt, wn, ts);
                            break;
                        }
                        case Work::kQuad_Segment: {
                            swap = true;
                            pts = QuadLineIntersect(wn.pts(), wt.pts(), ts);
                            debugShowQuadLineIntersection(pts, wn, wt, ts);
                            break;
                        }
                        case Work::kCubic_Segment: {
                            swap = true;
                            pts = CubicLineIntersect(wn.pts(), wt.pts(), ts);
                            debugShowCubicLineIntersection(pts, wn, wt,  ts);
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
                            debugShowQuadLineIntersection(pts, wt, wn, ts);
                            break;
                        }
                        case Work::kQuad_Segment: {
                            pts = QuadIntersect(wt.pts(), wn.pts(), ts);
                            debugShowQuadIntersection(pts, wt, wn, ts);
                            break;
                        }
                        case Work::kCubic_Segment: {
                    #if APPROXIMATE_CUBICS
                            swap = true;
                            pts = CubicQuadIntersect(wn.pts(), wt.pts(), ts);
                            debugShowCubicQuadIntersection(pts, wn, wt, ts);
                    #else
                            wt.promoteToCubic();
                            pts = CubicIntersect(wt.cubic(), wn.pts(), ts);
                            debugShowCubicIntersection(pts, wt, wn, ts);
                    #endif
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
                            debugShowCubicLineIntersection(pts, wt, wn, ts);
                            break;
                        case Work::kVerticalLine_Segment:
                            pts = VCubicIntersect(wt.pts(), wn.top(),
                                    wn.bottom(), wn.x(), wn.yFlipped(), ts);
                            debugShowCubicLineIntersection(pts, wt, wn, ts);
                            break;
                        case Work::kLine_Segment: {
                            pts = CubicLineIntersect(wt.pts(), wn.pts(), ts);
                            debugShowCubicLineIntersection(pts, wt, wn, ts);
                            break;
                        }
                        case Work::kQuad_Segment: {
                    #if APPROXIMATE_CUBICS
                            pts = CubicQuadIntersect(wt.pts(), wn.pts(), ts);
                            debugShowCubicQuadIntersection(pts, wt, wn, ts);
                    #else
                            wn.promoteToCubic();
                            pts = CubicIntersect(wt.pts(), wn.cubic(), ts);
                            debugShowCubicIntersection(pts, wt, wn, ts);
                    #endif
                            break;
                        }
                        case Work::kCubic_Segment: {
                            pts = CubicIntersect(wt.pts(), wn.pts(), ts);
                            debugShowCubicIntersection(pts, wt, wn, ts);
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
            if (ts.unsortable()) {
                bool start = true;
                for (int pt = 0; pt < ts.used(); ++pt) {
                    // FIXME: if unsortable, the other points to the original. This logic is
                    // untested downstream.
                    SkPoint point = ts.fPt[pt].asSkPoint();
                    int testTAt = wt.addUnsortableT(wt, start, point, ts.fT[swap][pt]);
                    wt.addOtherT(testTAt, ts.fT[swap][pt], testTAt);
                    testTAt = wn.addUnsortableT(wn, start ^ ts.fFlip, point, ts.fT[!swap][pt]);
                    wn.addOtherT(testTAt, ts.fT[!swap][pt], testTAt);
                    start ^= true;
                }
                continue;
            }
            if (pts == 2) {
                if (wn.segmentType() <= Work::kLine_Segment
                        && wt.segmentType() <= Work::kLine_Segment) {
                    wt.addCoincident(wn, ts, swap);
                    continue;
                }
                if (wn.segmentType() >= Work::kQuad_Segment
                        && wt.segmentType() >= Work::kQuad_Segment
                        && ts.fIsCoincident[0]) {
                    SkASSERT(ts.coincidentUsed() == 2);
                    wt.addCoincident(wn, ts, swap);
                    continue;
                }

            }
            for (int pt = 0; pt < pts; ++pt) {
                SkASSERT(ts.fT[0][pt] >= 0 && ts.fT[0][pt] <= 1);
                SkASSERT(ts.fT[1][pt] >= 0 && ts.fT[1][pt] <= 1);
                SkPoint point = ts.fPt[pt].asSkPoint();
                int testTAt = wt.addT(wn, point, ts.fT[swap][pt]);
                int nextTAt = wn.addT(wt, point, ts.fT[!swap][pt]);
                wt.addOtherT(testTAt, ts.fT[!swap][pt ^ ts.fFlip], nextTAt);
                wn.addOtherT(nextTAt, ts.fT[swap][pt ^ ts.fFlip], testTAt);
            }
        } while (wn.advance());
    } while (wt.advance());
    return true;
}

static void addSelfIntersectTs(Contour* test) {
    Work wt;
    wt.init(test);
    do {
        if (wt.segmentType() != Work::kCubic_Segment) {
            continue;
        }
        Intersections ts;
        int pts = CubicIntersect(wt.pts(), ts);
        debugShowCubicIntersection(pts, wt, ts);
        if (!pts) {
            continue;
        }
        SkASSERT(pts == 1);
        SkASSERT(ts.fT[0][0] >= 0 && ts.fT[0][0] <= 1);
        SkASSERT(ts.fT[1][0] >= 0 && ts.fT[1][0] <= 1);
        SkPoint point = ts.fPt[0].asSkPoint();
        int testTAt = wt.addSelfT(wt, point, ts.fT[0][0]);
        int nextTAt = wt.addT(wt, point, ts.fT[1][0]);
        wt.addOtherT(testTAt, ts.fT[1][0], nextTAt);
        wt.addOtherT(nextTAt, ts.fT[0][0], testTAt);
    } while (wt.advance());
}

// resolve any coincident pairs found while intersecting, and
// see if coincidence is formed by clipping non-concident segments
static void coincidenceCheck(SkTDArray<Contour*>& contourList, int total) {
    int contourCount = contourList.count();
#if ONE_PASS_COINCIDENCE_CHECK
    for (int cIndex = 0; cIndex < contourCount; ++cIndex) {
        Contour* contour = contourList[cIndex];
        contour->resolveCoincidence(contourList);
    }
#else
    for (int cIndex = 0; cIndex < contourCount; ++cIndex) {
        Contour* contour = contourList[cIndex];
        contour->addCoincidentPoints();
    }
    for (int cIndex = 0; cIndex < contourCount; ++cIndex) {
        Contour* contour = contourList[cIndex];
        contour->calcCoincidentWinding();
    }
#endif
    for (int cIndex = 0; cIndex < contourCount; ++cIndex) {
        Contour* contour = contourList[cIndex];
        contour->findTooCloseToCall();
    }
}

static int contourRangeCheckY(SkTDArray<Contour*>& contourList,  Segment*& current, int& index,
        int& endIndex, double& bestHit, SkScalar& bestDx, bool& tryAgain, double& mid, bool opp) {
    SkPoint basePt;
    double tAtMid = current->tAtMid(index, endIndex, mid);
    current->xyAtT(tAtMid, basePt);
    int contourCount = contourList.count();
    SkScalar bestY = SK_ScalarMin;
    Segment* bestSeg = NULL;
    int bestTIndex;
    bool bestOpp;
    bool hitSomething = false;
    for (int cTest = 0; cTest < contourCount; ++cTest) {
        Contour* contour = contourList[cTest];
        bool testOpp = contour->operand() ^ current->operand() ^ opp;
        if (basePt.fY < contour->bounds().fTop) {
            continue;
        }
        if (bestY > contour->bounds().fBottom) {
            continue;
        }
        int segmentCount = contour->segments().count();
        for (int test = 0; test < segmentCount; ++test) {
            Segment* testSeg = &contour->segments()[test];
            SkScalar testY = bestY;
            double testHit;
            int testTIndex = testSeg->crossedSpanY(basePt, testY, testHit, hitSomething, tAtMid,
                    testOpp, testSeg == current);
            if (testTIndex < 0) {
                if (testTIndex == SK_MinS32) {
                    hitSomething = true;
                    bestSeg = NULL;
                    goto abortContours; // vertical encountered, return and try different point
                }
                continue;
            }
            if (testSeg == current && current->betweenTs(index, testHit, endIndex)) {
                double baseT = current->t(index);
                double endT = current->t(endIndex);
                double newMid = (testHit - baseT) / (endT - baseT);
#if DEBUG_WINDING
                SkPoint midXY, newXY;
                double midT = current->tAtMid(index, endIndex, mid);
                current->xyAtT(midT, midXY);
                double newMidT = current->tAtMid(index, endIndex, newMid);
                current->xyAtT(newMidT, newXY);
                SkDebugf("%s [%d] mid=%1.9g->%1.9g s=%1.9g (%1.9g,%1.9g) m=%1.9g (%1.9g,%1.9g)"
                        " n=%1.9g (%1.9g,%1.9g) e=%1.9g (%1.9g,%1.9g)\n", __FUNCTION__,
                        current->debugID(), mid, newMid,
                        baseT, current->xAtT(index), current->yAtT(index),
                        baseT + mid * (endT - baseT), midXY.fX, midXY.fY,
                        baseT + newMid * (endT - baseT), newXY.fX, newXY.fY,
                        endT, current->xAtT(endIndex), current->yAtT(endIndex));
#endif
                mid = newMid * 2; // calling loop with divide by 2 before continuing
                return SK_MinS32;
            }
            bestSeg = testSeg;
            bestHit = testHit;
            bestOpp = testOpp;
            bestTIndex = testTIndex;
            bestY = testY;
        }
    }
abortContours:
    int result;
    if (!bestSeg) {
        result = hitSomething ? SK_MinS32 : 0;
    } else {
        if (bestSeg->windSum(bestTIndex) == SK_MinS32) {
            current = bestSeg;
            index = bestTIndex;
            endIndex = bestSeg->nextSpan(bestTIndex, 1);
            SkASSERT(index != endIndex && index >= 0 && endIndex >= 0);
            tryAgain = true;
            return 0;
        }
        result = bestSeg->windingAtT(bestHit, bestTIndex, bestOpp, bestDx);
        SkASSERT(bestDx);
    }
    double baseT = current->t(index);
    double endT = current->t(endIndex);
    bestHit = baseT + mid * (endT - baseT);
    return result;
}

static Segment* findUndone(SkTDArray<Contour*>& contourList, int& start, int& end) {
    int contourCount = contourList.count();
    Segment* result;
    for (int cIndex = 0; cIndex < contourCount; ++cIndex) {
        Contour* contour = contourList[cIndex];
        result = contour->undoneSegment(start, end);
        if (result) {
            return result;
        }
    }
    return NULL;
}

#define OLD_FIND_CHASE 1

static Segment* findChase(SkTDArray<Span*>& chase, int& tIndex, int& endIndex) {
    while (chase.count()) {
        Span* span;
        chase.pop(&span);
        const Span& backPtr = span->fOther->span(span->fOtherIndex);
        Segment* segment = backPtr.fOther;
        tIndex = backPtr.fOtherIndex;
        SkTDArray<Angle> angles;
        int done = 0;
        if (segment->activeAngle(tIndex, done, angles)) {
            Angle* last = angles.end() - 1;
            tIndex = last->start();
            endIndex = last->end();
   #if TRY_ROTATE
            *chase.insert(0) = span;
   #else
            *chase.append() = span;
   #endif
            return last->segment();
        }
        if (done == angles.count()) {
            continue;
        }
        SkTDArray<Angle*> sorted;
        bool sortable = Segment::SortAngles(angles, sorted);
        int angleCount = sorted.count();
#if DEBUG_SORT
        sorted[0]->segment()->debugShowSort(__FUNCTION__, sorted, 0, 0, 0);
#endif
        if (!sortable) {
            continue;
        }
        // find first angle, initialize winding to computed fWindSum
        int firstIndex = -1;
        const Angle* angle;
#if OLD_FIND_CHASE
        int winding;
        do {
            angle = sorted[++firstIndex];
            segment = angle->segment();
            winding = segment->windSum(angle);
        } while (winding == SK_MinS32);
        int spanWinding = segment->spanSign(angle->start(), angle->end());
    #if DEBUG_WINDING
        SkDebugf("%s winding=%d spanWinding=%d\n",
                __FUNCTION__, winding, spanWinding);
    #endif
        // turn span winding into contour winding
        if (spanWinding * winding < 0) {
            winding += spanWinding;
        }
    #if DEBUG_SORT
        segment->debugShowSort(__FUNCTION__, sorted, firstIndex, winding, 0);
    #endif
        // we care about first sign and whether wind sum indicates this
        // edge is inside or outside. Maybe need to pass span winding
        // or first winding or something into this function?
        // advance to first undone angle, then return it and winding
        // (to set whether edges are active or not)
        int nextIndex = firstIndex + 1;
        int lastIndex = firstIndex != 0 ? firstIndex : angleCount;
        angle = sorted[firstIndex];
        winding -= angle->segment()->spanSign(angle);
#else
        do {
            angle = sorted[++firstIndex];
            segment = angle->segment();
        } while (segment->windSum(angle) == SK_MinS32);
    #if DEBUG_SORT
        segment->debugShowSort(__FUNCTION__, sorted, firstIndex);
    #endif
        int sumWinding = segment->updateWindingReverse(angle);
        int nextIndex = firstIndex + 1;
        int lastIndex = firstIndex != 0 ? firstIndex : angleCount;
        Segment* first = NULL;
#endif
        do {
            SkASSERT(nextIndex != firstIndex);
            if (nextIndex == angleCount) {
                nextIndex = 0;
            }
            angle = sorted[nextIndex];
            segment = angle->segment();
#if OLD_FIND_CHASE
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
            // FIXME: this be wrong? assign startWinding if edge is in
            // same direction. If the direction is opposite, winding to
            // assign is flipped sign or +/- 1?
                if (useInnerWinding(maxWinding, winding)) {
                    maxWinding = winding;
                }
                segment->markAndChaseWinding(angle, maxWinding, 0);
#endif
                break;
            }
#else
            int start = angle->start();
            int end = angle->end();
            int maxWinding;
            segment->setUpWinding(start, end, maxWinding, sumWinding);
            if (!segment->done(angle)) {
                if (!first) {
                    first = segment;
                    tIndex = start;
                    endIndex = end;
                }
                (void) segment->markAngle(maxWinding, sumWinding, true, angle);
            }
#endif
        } while (++nextIndex != lastIndex);
   #if TRY_ROTATE
        *chase.insert(0) = span;
   #else
        *chase.append() = span;
   #endif
        return segment;
    }
    return NULL;
}

#if DEBUG_ACTIVE_SPANS
static void debugShowActiveSpans(SkTDArray<Contour*>& contourList) {
    int index;
    for (index = 0; index < contourList.count(); ++ index) {
        contourList[index]->debugShowActiveSpans();
    }
    for (index = 0; index < contourList.count(); ++ index) {
        contourList[index]->validateActiveSpans();
    }
}
#endif

static Segment* findSortableTop(SkTDArray<Contour*>& contourList, int& index,
        int& endIndex, SkPoint& topLeft, bool& unsortable, bool& done, bool onlySortable) {
    Segment* result;
    do {
        SkPoint bestXY = {SK_ScalarMax, SK_ScalarMax};
        int contourCount = contourList.count();
        Segment* topStart = NULL;
        done = true;
        for (int cIndex = 0; cIndex < contourCount; ++cIndex) {
            Contour* contour = contourList[cIndex];
            if (contour->done()) {
                continue;
            }
            const Bounds& bounds = contour->bounds();
            if (bounds.fBottom < topLeft.fY) {
                done = false;
                continue;
            }
            if (bounds.fBottom == topLeft.fY && bounds.fRight < topLeft.fX) {
                done = false;
                continue;
            }
            contour->topSortableSegment(topLeft, bestXY, topStart);
            if (!contour->done()) {
                done = false;
            }
        }
        if (!topStart) {
            return NULL;
        }
        topLeft = bestXY;
        result = topStart->findTop(index, endIndex, unsortable, onlySortable);
    } while (!result);
    return result;
}

static int rightAngleWinding(SkTDArray<Contour*>& contourList,
        Segment*& current, int& index, int& endIndex, double& tHit, SkScalar& hitDx, bool& tryAgain,
        bool opp) {
    double test = 0.9;
    int contourWinding;
    do {
        contourWinding = contourRangeCheckY(contourList, current, index, endIndex, tHit, hitDx,
                tryAgain, test, opp);
        if (contourWinding != SK_MinS32 || tryAgain) {
            return contourWinding;
        }
        test /= 2;
    } while (!approximately_negative(test));
    SkASSERT(0); // should be OK to comment out, but interested when this hits
    return contourWinding;
}

static void skipVertical(SkTDArray<Contour*>& contourList,
        Segment*& current, int& index, int& endIndex) {
    if (!current->isVertical(index, endIndex)) {
        return;
    }
    int contourCount = contourList.count();
    for (int cIndex = 0; cIndex < contourCount; ++cIndex) {
        Contour* contour = contourList[cIndex];
        if (contour->done()) {
            continue;
        }
        current = contour->nonVerticalSegment(index, endIndex);
        if (current) {
            return;
        }
    }
}

static Segment* findSortableTop(SkTDArray<Contour*>& contourList, bool& firstContour, int& index,
        int& endIndex, SkPoint& topLeft, bool& unsortable, bool& done, bool binary) {
    Segment* current = findSortableTop(contourList, index, endIndex, topLeft, unsortable, done,
            true);
    if (!current) {
        return NULL;
    }
    if (firstContour) {
        current->initWinding(index, endIndex);
        firstContour = false;
        return current;
    }
    int minIndex = SkMin32(index, endIndex);
    int sumWinding = current->windSum(minIndex);
    if (sumWinding != SK_MinS32) {
        return current;
    }
    sumWinding = current->computeSum(index, endIndex, binary);
    if (sumWinding != SK_MinS32) {
        return current;
    }
    int contourWinding;
    int oppContourWinding = 0;
    // the simple upward projection of the unresolved points hit unsortable angles
    // shoot rays at right angles to the segment to find its winding, ignoring angle cases
    bool tryAgain;
    double tHit;
    SkScalar hitDx = 0;
    SkScalar hitOppDx = 0;
    do {
        // if current is vertical, find another candidate which is not
        // if only remaining candidates are vertical, then they can be marked done
        SkASSERT(index != endIndex && index >= 0 && endIndex >= 0);
        skipVertical(contourList, current, index, endIndex);
        SkASSERT(index != endIndex && index >= 0 && endIndex >= 0);
        tryAgain = false;
        contourWinding = rightAngleWinding(contourList, current, index, endIndex, tHit, hitDx,
                tryAgain, false);
        if (tryAgain) {
            continue;
        }
        if (!binary) {
            break;
        }
        oppContourWinding = rightAngleWinding(contourList, current, index, endIndex, tHit, hitOppDx,
                tryAgain, true);
    } while (tryAgain);

    current->initWinding(index, endIndex, tHit, contourWinding, hitDx, oppContourWinding, hitOppDx);
    return current;
}

// rewrite that abandons keeping local track of winding
static bool bridgeWinding(SkTDArray<Contour*>& contourList, PathWrapper& simple) {
    bool firstContour = true;
    bool unsortable = false;
    bool topUnsortable = false;
    SkPoint topLeft = {SK_ScalarMin, SK_ScalarMin};
    do {
        int index, endIndex;
        bool topDone;
        Segment* current = findSortableTop(contourList, firstContour, index, endIndex, topLeft,
                topUnsortable, topDone, false);
        if (!current) {
            if (topUnsortable || !topDone) {
                topUnsortable = false;
                SkASSERT(topLeft.fX != SK_ScalarMin && topLeft.fY != SK_ScalarMin);
                topLeft.fX = topLeft.fY = SK_ScalarMin;
                continue;
            }
            break;
        }
        SkTDArray<Span*> chaseArray;
        do {
            if (current->activeWinding(index, endIndex)) {
                do {
            #if DEBUG_ACTIVE_SPANS
                    if (!unsortable && current->done()) {
                        debugShowActiveSpans(contourList);
                    }
            #endif
                    SkASSERT(unsortable || !current->done());
                    int nextStart = index;
                    int nextEnd = endIndex;
                    Segment* next = current->findNextWinding(chaseArray, nextStart, nextEnd,
                            unsortable);
                    if (!next) {
                        if (!unsortable && simple.hasMove()
                                && current->verb() != SkPath::kLine_Verb
                                && !simple.isClosed()) {
                            current->addCurveTo(index, endIndex, simple, true);
                            SkASSERT(simple.isClosed());
                        }
                        break;
                    }
        #if DEBUG_FLOW
            SkDebugf("%s current id=%d from=(%1.9g,%1.9g) to=(%1.9g,%1.9g)\n", __FUNCTION__,
                    current->debugID(), current->xyAtT(index).fX, current->xyAtT(index).fY,
                    current->xyAtT(endIndex).fX, current->xyAtT(endIndex).fY);
        #endif
                    current->addCurveTo(index, endIndex, simple, true);
                    current = next;
                    index = nextStart;
                    endIndex = nextEnd;
                } while (!simple.isClosed() && (!unsortable
                        || !current->done(SkMin32(index, endIndex))));
                if (current->activeWinding(index, endIndex) && !simple.isClosed()) {
                    SkASSERT(unsortable);
                    int min = SkMin32(index, endIndex);
                    if (!current->done(min)) {
                        current->addCurveTo(index, endIndex, simple, true);
                        current->markDoneUnary(min);
                    }
                }
                simple.close();
            } else {
                Span* last = current->markAndChaseDoneUnary(index, endIndex);
                if (last && !last->fLoop) {
                    *chaseArray.append() = last;
                }
            }
            current = findChase(chaseArray, index, endIndex);
        #if DEBUG_ACTIVE_SPANS
            debugShowActiveSpans(contourList);
        #endif
            if (!current) {
                break;
            }
        } while (true);
    } while (true);
    return simple.someAssemblyRequired();
}

// returns true if all edges were processed
static bool bridgeXor(SkTDArray<Contour*>& contourList, PathWrapper& simple) {
    Segment* current;
    int start, end;
    bool unsortable = false;
    bool closable = true;
    while ((current = findUndone(contourList, start, end))) {
        do {
    #if DEBUG_ACTIVE_SPANS
            if (!unsortable && current->done()) {
                debugShowActiveSpans(contourList);
            }
    #endif
            SkASSERT(unsortable || !current->done());
            int nextStart = start;
            int nextEnd = end;
            Segment* next = current->findNextXor(nextStart, nextEnd, unsortable);
            if (!next) {
                if (!unsortable && simple.hasMove()
                        && current->verb() != SkPath::kLine_Verb
                        && !simple.isClosed()) {
                    current->addCurveTo(start, end, simple, true);
                    SkASSERT(simple.isClosed());
                }
                break;
            }
        #if DEBUG_FLOW
            SkDebugf("%s current id=%d from=(%1.9g,%1.9g) to=(%1.9g,%1.9g)\n", __FUNCTION__,
                    current->debugID(), current->xyAtT(start).fX, current->xyAtT(start).fY,
                    current->xyAtT(end).fX, current->xyAtT(end).fY);
        #endif
            current->addCurveTo(start, end, simple, true);
            current = next;
            start = nextStart;
            end = nextEnd;
        } while (!simple.isClosed() && (!unsortable || !current->done(SkMin32(start, end))));
        if (!simple.isClosed()) {
            SkASSERT(unsortable);
            int min = SkMin32(start, end);
            if (!current->done(min)) {
                current->addCurveTo(start, end, simple, true);
                current->markDone(min, 1);
            }
            closable = false;
        }
        simple.close();
    #if DEBUG_ACTIVE_SPANS
        debugShowActiveSpans(contourList);
    #endif
    }
    return closable;
}

static void fixOtherTIndex(SkTDArray<Contour*>& contourList) {
    int contourCount = contourList.count();
    for (int cTest = 0; cTest < contourCount; ++cTest) {
        Contour* contour = contourList[cTest];
        contour->fixOtherTIndex();
    }
}

static void sortSegments(SkTDArray<Contour*>& contourList) {
    int contourCount = contourList.count();
    for (int cTest = 0; cTest < contourCount; ++cTest) {
        Contour* contour = contourList[cTest];
        contour->sortSegments();
    }
}

static void makeContourList(SkTArray<Contour>& contours, SkTDArray<Contour*>& list,
        bool evenOdd, bool oppEvenOdd) {
    int count = contours.count();
    if (count == 0) {
        return;
    }
    for (int index = 0; index < count; ++index) {
        Contour& contour = contours[index];
        contour.setOppXor(contour.operand() ? evenOdd : oppEvenOdd);
        *list.append() = &contour;
    }
    QSort<Contour>(list.begin(), list.end() - 1);
}

static bool approximatelyEqual(const SkPoint& a, const SkPoint& b) {
    return AlmostEqualUlps(a.fX, b.fX) && AlmostEqualUlps(a.fY, b.fY);
}

static bool lessThan(SkTDArray<double>& distances, const int one, const int two) {
    return distances[one] < distances[two];
}
    /*
        check start and end of each contour
        if not the same, record them
        match them up
        connect closest
        reassemble contour pieces into new path
    */
static void assemble(const PathWrapper& path, PathWrapper& simple) {
#if DEBUG_PATH_CONSTRUCTION
    SkDebugf("%s\n", __FUNCTION__);
#endif
    SkTArray<Contour> contours;
    EdgeBuilder builder(path, contours);
    builder.finish();
    int count = contours.count();
    int outer;
    SkTDArray<int> runs; // indices of partial contours
    for (outer = 0; outer < count; ++outer) {
        const Contour& eContour = contours[outer];
        const SkPoint& eStart = eContour.start();
        const SkPoint& eEnd = eContour.end();
#if DEBUG_ASSEMBLE
        SkDebugf("%s contour", __FUNCTION__);
        if (!approximatelyEqual(eStart, eEnd)) {
            SkDebugf("[%d]", runs.count());
        } else {
            SkDebugf("   ");
        }
        SkDebugf(" start=(%1.9g,%1.9g) end=(%1.9g,%1.9g)\n",
                eStart.fX, eStart.fY, eEnd.fX, eEnd.fY);
#endif
        if (approximatelyEqual(eStart, eEnd)) {
            eContour.toPath(simple);
            continue;
        }
        *runs.append() = outer;
    }
    count = runs.count();
    if (count == 0) {
        return;
    }
    SkTDArray<int> sLink, eLink;
    sLink.setCount(count);
    eLink.setCount(count);
    int rIndex, iIndex;
    for (rIndex = 0; rIndex < count; ++rIndex) {
        sLink[rIndex] = eLink[rIndex] = SK_MaxS32;
    }
    SkTDArray<double> distances;
    const int ends = count * 2; // all starts and ends
    const int entries = (ends - 1) * count; // folded triangle : n * (n - 1) / 2
    distances.setCount(entries);
    for (rIndex = 0; rIndex < ends - 1; ++rIndex) {
        outer = runs[rIndex >> 1];
        const Contour& oContour = contours[outer];
        const SkPoint& oPt = rIndex & 1 ? oContour.end() : oContour.start();
        const int row = rIndex < count - 1 ? rIndex * ends : (ends - rIndex - 2)
                * ends - rIndex - 1;
        for (iIndex = rIndex + 1; iIndex < ends; ++iIndex) {
            int inner = runs[iIndex >> 1];
            const Contour& iContour = contours[inner];
            const SkPoint& iPt = iIndex & 1 ? iContour.end() : iContour.start();
            double dx = iPt.fX - oPt.fX;
            double dy = iPt.fY - oPt.fY;
            double dist = dx * dx + dy * dy;
            distances[row + iIndex] = dist; // oStart distance from iStart
        }
    }
    SkTDArray<int> sortedDist;
    sortedDist.setCount(entries);
    for (rIndex = 0; rIndex < entries; ++rIndex) {
        sortedDist[rIndex] = rIndex;
    }
    QSort<SkTDArray<double>, int>(distances, sortedDist.begin(), sortedDist.end() - 1, lessThan);
    int remaining = count; // number of start/end pairs
    for (rIndex = 0; rIndex < entries; ++rIndex) {
        int pair = sortedDist[rIndex];
        int row = pair / ends;
        int col = pair - row * ends;
        int thingOne = row < col ? row : ends - row - 2;
        int ndxOne = thingOne >> 1;
        bool endOne = thingOne & 1;
        int* linkOne = endOne ? eLink.begin() : sLink.begin();
        if (linkOne[ndxOne] != SK_MaxS32) {
            continue;
        }
        int thingTwo = row < col ? col : ends - row + col - 1;
        int ndxTwo = thingTwo >> 1;
        bool endTwo = thingTwo & 1;
        int* linkTwo = endTwo ? eLink.begin() : sLink.begin();
        if (linkTwo[ndxTwo] != SK_MaxS32) {
            continue;
        }
        SkASSERT(&linkOne[ndxOne] != &linkTwo[ndxTwo]);
        bool flip = endOne == endTwo;
        linkOne[ndxOne] = flip ? ~ndxTwo : ndxTwo;
        linkTwo[ndxTwo] = flip ? ~ndxOne : ndxOne;
        if (!--remaining) {
            break;
        }
    }
    SkASSERT(!remaining);
#if DEBUG_ASSEMBLE
    for (rIndex = 0; rIndex < count; ++rIndex) {
        int s = sLink[rIndex];
        int e = eLink[rIndex];
        SkDebugf("%s %c%d <- s%d - e%d -> %c%d\n", __FUNCTION__, s < 0 ? 's' : 'e',
                s < 0 ? ~s : s, rIndex, rIndex, e < 0 ? 'e' : 's', e < 0 ? ~e : e);
    }
#endif
    rIndex = 0;
    do {
        bool forward = true;
        bool first = true;
        int sIndex = sLink[rIndex];
        SkASSERT(sIndex != SK_MaxS32);
        sLink[rIndex] = SK_MaxS32;
        int eIndex;
        if (sIndex < 0) {
            eIndex = sLink[~sIndex];
            sLink[~sIndex] = SK_MaxS32;
        } else {
            eIndex = eLink[sIndex];
            eLink[sIndex] = SK_MaxS32;
        }
        SkASSERT(eIndex != SK_MaxS32);
#if DEBUG_ASSEMBLE
        SkDebugf("%s sIndex=%c%d eIndex=%c%d\n", __FUNCTION__, sIndex < 0 ? 's' : 'e',
                    sIndex < 0 ? ~sIndex : sIndex, eIndex < 0 ? 's' : 'e',
                    eIndex < 0 ? ~eIndex : eIndex);
#endif
        do {
            outer = runs[rIndex];
            const Contour& contour = contours[outer];
            if (first) {
                first = false;
                const SkPoint* startPtr = &contour.start();
                simple.deferredMove(startPtr[0]);
            }
            if (forward) {
                contour.toPartialForward(simple);
            } else {
                contour.toPartialBackward(simple);
            }
#if DEBUG_ASSEMBLE
            SkDebugf("%s rIndex=%d eIndex=%s%d close=%d\n", __FUNCTION__, rIndex,
                eIndex < 0 ? "~" : "", eIndex < 0 ? ~eIndex : eIndex,
                sIndex == ((rIndex != eIndex) ^ forward ? eIndex : ~eIndex));
#endif
            if (sIndex == ((rIndex != eIndex) ^ forward ? eIndex : ~eIndex)) {
                simple.close();
                break;
            }
            if (forward) {
                eIndex = eLink[rIndex];
                SkASSERT(eIndex != SK_MaxS32);
                eLink[rIndex] = SK_MaxS32;
                if (eIndex >= 0) {
                    SkASSERT(sLink[eIndex] == rIndex);
                    sLink[eIndex] = SK_MaxS32;
                } else {
                    SkASSERT(eLink[~eIndex] == ~rIndex);
                    eLink[~eIndex] = SK_MaxS32;
                }
            } else {
                eIndex = sLink[rIndex];
                SkASSERT(eIndex != SK_MaxS32);
                sLink[rIndex] = SK_MaxS32;
                if (eIndex >= 0) {
                    SkASSERT(eLink[eIndex] == rIndex);
                    eLink[eIndex] = SK_MaxS32;
                } else {
                    SkASSERT(sLink[~eIndex] == ~rIndex);
                    sLink[~eIndex] = SK_MaxS32;
                }
            }
            rIndex = eIndex;
            if (rIndex < 0) {
                forward ^= 1;
                rIndex = ~rIndex;
            }
        } while (true);
        for (rIndex = 0; rIndex < count; ++rIndex) {
            if (sLink[rIndex] != SK_MaxS32) {
                break;
            }
        }
    } while (rIndex < count);
#if DEBUG_ASSEMBLE
    for (rIndex = 0; rIndex < count; ++rIndex) {
       SkASSERT(sLink[rIndex] == SK_MaxS32);
       SkASSERT(eLink[rIndex] == SK_MaxS32);
    }
#endif
}

void simplifyx(const SkPath& path, SkPath& result) {
#if DEBUG_SORT || DEBUG_SWAP_TOP
    gDebugSortCount = gDebugSortCountDefault;
#endif
    // returns 1 for evenodd, -1 for winding, regardless of inverse-ness
    result.reset();
    result.setFillType(SkPath::kEvenOdd_FillType);
    PathWrapper simple(result);

    // turn path into list of segments
    SkTArray<Contour> contours;
    EdgeBuilder builder(path, contours);
    builder.finish();
    SkTDArray<Contour*> contourList;
    makeContourList(contours, contourList, false, false);
    Contour** currentPtr = contourList.begin();
    if (!currentPtr) {
        return;
    }
    Contour** listEnd = contourList.end();
    // find all intersections between segments
    do {
        Contour** nextPtr = currentPtr;
        Contour* current = *currentPtr++;
        if (current->containsCubics()) {
            addSelfIntersectTs(current);
        }
        Contour* next;
        do {
            next = *nextPtr++;
        } while (addIntersectTs(current, next) && nextPtr != listEnd);
    } while (currentPtr != listEnd);
    // eat through coincident edges
    coincidenceCheck(contourList, 0);
    fixOtherTIndex(contourList);
    sortSegments(contourList);
#if DEBUG_ACTIVE_SPANS
    debugShowActiveSpans(contourList);
#endif
    // construct closed contours
    if (builder.xorMask() == kWinding_Mask ? bridgeWinding(contourList, simple)
                : !bridgeXor(contourList, simple))
    { // if some edges could not be resolved, assemble remaining fragments
        SkPath temp;
        temp.setFillType(SkPath::kEvenOdd_FillType);
        PathWrapper assembled(temp);
        assemble(simple, assembled);
        result = *assembled.nativePath();
    }
}
