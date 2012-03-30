
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

#if 0 // set to 1 for no debugging whatsoever
const bool gShowDebugf = false; // FIXME: remove once debugging is complete

#define DEBUG_DUMP 0
#define DEBUG_ADD 0
#define DEBUG_ADD_INTERSECTING_TS 0
#define DEBUG_ADD_BOTTOM_TS 0
#define DEBUG_ABOVE_BELOW 0
#define DEBUG_ACTIVE_LESS_THAN 0
#define DEBUG_SORT_HORIZONTAL 0
#define DEBUG_OUT 0
#define DEBUG_OUT_LESS_THAN 0
#define DEBUG_ADJUST_COINCIDENT 0
#define DEBUG_BOTTOM 0
#define DEBUG_SPLIT 0

#else
const bool gShowDebugf = true; // FIXME: remove once debugging is complete

#define DEBUG_DUMP 01
#define DEBUG_ADD 01
#define DEBUG_ADD_INTERSECTING_TS 0
#define DEBUG_ADD_BOTTOM_TS 0
#define DEBUG_ABOVE_BELOW 01
#define DEBUG_ACTIVE_LESS_THAN 0
#define DEBUG_SORT_HORIZONTAL 01
#define DEBUG_OUT 01
#define DEBUG_OUT_LESS_THAN 0
#define DEBUG_ADJUST_COINCIDENT 1
#define DEBUG_BOTTOM 0
#define DEBUG_SPLIT 1

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

static int LineIntersect(const SkPoint a[2], SkScalar left, SkScalar right,
        SkScalar y, double aRange[2]) {
    const _Line aLine = {{a[0].fX, a[0].fY}, {a[1].fX, a[1].fY}};
    return horizontalLineIntersect(aLine, left, right, y, aRange);
}

static int QuadIntersect(const SkPoint a[3], SkScalar left, SkScalar right,
        SkScalar y, double aRange[3]) {
    const Quadratic aQuad = {{a[0].fX, a[0].fY}, {a[1].fX, a[1].fY}, {a[2].fX, a[2].fY}};
    return horizontalIntersect(aQuad, left, right, y, aRange);
}

static int CubicIntersect(const SkPoint a[4], SkScalar left, SkScalar right,
        SkScalar y, double aRange[4]) {
    const Cubic aCubic = {{a[0].fX, a[0].fY}, {a[1].fX, a[1].fY}, {a[2].fX, a[2].fY},
            {a[3].fX, a[3].fY}};
    return horizontalIntersect(aCubic, left, right, y, aRange);
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
    const Quadratic aQuad = {{a[0].fX, a[0].fY}, {a[1].fX, a[1].fY}, {a[2].fX, a[2].fY}};
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
    const Cubic aCubic = {{a[0].fX, a[0].fY}, {a[1].fX, a[1].fY}, {a[2].fX, a[2].fY},
            {a[3].fX, a[3].fY}};
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
 
/*
list of edges
bounds for edge
sort
active T

if a contour's bounds is outside of the active area, no need to create edges 
*/

/* given one or more paths, 
 find the bounds of each contour, select the active contours
 for each active contour, compute a set of edges
 each edge corresponds to one or more lines and curves
 leave edges unbroken as long as possible
 when breaking edges, compute the t at the break but leave the control points alone

 */

void contourBounds(const SkPath& path, SkTDArray<SkRect>& boundsArray) {
    SkPath::Iter iter(path, false);
    SkPoint pts[4];
    SkPath::Verb verb;
    SkRect bounds;
    bounds.setEmpty();
    int count = 0;
    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kMove_Verb:
                if (!bounds.isEmpty()) {
                    *boundsArray.append() = bounds;
                }
                bounds.set(pts[0].fX, pts[0].fY, pts[0].fX, pts[0].fY);
                count = 0;
                break;
            case SkPath::kLine_Verb: 
                count = 1;
                break;
            case SkPath::kQuad_Verb:
                count = 2;
                break;
            case SkPath::kCubic_Verb:
                count = 3;
                break;
            case SkPath::kClose_Verb:
                count = 0;
                break;
            default:
                SkDEBUGFAIL("bad verb");
                return;
        }
        for (int i = 1; i <= count; ++i) {
            bounds.growToInclude(pts[i].fX, pts[i].fY);
        }
    }
}

static bool extendLine(const SkPoint line[2], const SkPoint& add) {
    // FIXME: allow this to extend lines that have slopes that are nearly equal
    SkScalar dx1 = line[1].fX - line[0].fX;
    SkScalar dy1 = line[1].fY - line[0].fY;
    SkScalar dx2 = add.fX - line[0].fX;
    SkScalar dy2 = add.fY - line[0].fY;
    return dx1 * dy2 == dx2 * dy1;
}

// OPTIMIZATION: this should point to a list of input data rather than duplicating
// the line data here. This would reduce the need to assemble the results.
struct OutEdge {
    bool operator<(const OutEdge& rh) const {
        const SkPoint& first = fPts[0];
        const SkPoint& rhFirst = rh.fPts[0];
        return first.fY == rhFirst.fY
                ? first.fX < rhFirst.fX
                : first.fY < rhFirst.fY;
    }

    SkPoint fPts[4];
    int fID; // id of edge generating data
    uint8_t fVerb; // FIXME: not read from everywhere
    bool fCloseCall; // edge is trimmable if not originally coincident
};

class OutEdgeBuilder {
public:
    OutEdgeBuilder(bool fill)
        : fFill(fill) {
        }

    void addCurve(const SkPoint line[4], SkPath::Verb verb, int id,
            bool closeCall) {
        OutEdge& newEdge = fEdges.push_back();
        memcpy(newEdge.fPts, line, (verb + 1) * sizeof(SkPoint));
        newEdge.fVerb = verb;
        newEdge.fID = id;
        newEdge.fCloseCall = closeCall;
    }

    bool trimLine(SkScalar y, int id) {
        size_t count = fEdges.count();
        while (count-- != 0) {
            OutEdge& edge = fEdges[count];
            if (edge.fID != id) {
                continue;
            }
            if (edge.fCloseCall) {
                return false;
            }
            SkASSERT(edge.fPts[0].fY <= y);
            if (edge.fPts[1].fY <= y) {
                continue;
            }
            edge.fPts[1].fX = edge.fPts[0].fX + (y - edge.fPts[0].fY)
                    * (edge.fPts[1].fX - edge.fPts[0].fX)
                    / (edge.fPts[1].fY - edge.fPts[0].fY);
            edge.fPts[1].fY = y;
            if (gShowDebugf) {
                SkDebugf("%s edge=%d %1.9g,%1.9g\n", __FUNCTION__, id,
                        edge.fPts[1].fX, y);
            }
            return true;
        }
        return false;
    }

    void assemble(SkPath& simple) {
        size_t listCount = fEdges.count();
        if (listCount == 0) {
            return;
        }
        do {
            size_t listIndex = 0;
            int advance = 1;
            while (listIndex < listCount && fTops[listIndex] == 0) {
                ++listIndex;
            }
            if (listIndex >= listCount) {
                break;
            }
            int closeEdgeIndex = -listIndex - 1;
            SkPoint firstPt, lastCurve[4];
            uint8_t lastVerb;
            bool doMove = true;
            int edgeIndex;
            do {
                SkPoint* ptArray = fEdges[listIndex].fPts;
                uint8_t verb = fEdges[listIndex].fVerb;
                SkPoint* start, * end;
                if (advance < 0) {
                    start = &ptArray[verb];
                    end = &ptArray[0];
                } else {
                    start = &ptArray[0];
                    end = &ptArray[verb];
                }
                if (doMove) {
                    firstPt = *start;
                    simple.moveTo(start->fX, start->fY);
                    if (gShowDebugf) {
                        SkDebugf("%s moveTo (%g,%g)\n", __FUNCTION__,
                                start->fX, start->fY);
                    }
                    lastCurve[0] = *start;
                    if (verb == SkPath::kQuad_Verb) {
                        lastCurve[1] = ptArray[1];
                    } else if (verb == SkPath::kCubic_Verb) {
                        if (advance < 0) {
                            lastCurve[1] = ptArray[2];
                            lastCurve[2] = ptArray[1];
                        } else {
                            lastCurve[1] = ptArray[1];
                            lastCurve[2] = ptArray[2];
                        }
                    }
                    lastCurve[verb] = *end;
                    lastVerb = verb;
                    doMove = false;
                } else {
                    bool gap = lastCurve[verb] != *start;
                    if (gap) {
                        // FIXME: see comment in bridge -- this probably
                        // conceals errors
                        SkASSERT(fFill && UlpsDiff(lastCurve[lastVerb].fY, start->fY) <= 10);
                        switch (lastVerb) {
                            case SkPath::kLine_Verb:
                                simple.lineTo(lastCurve[1].fX, lastCurve[1].fY);
                                break;
                            case SkPath::kQuad_Verb:
                                simple.quadTo(lastCurve[1].fX, lastCurve[1].fY,
                                        lastCurve[2].fX, lastCurve[2].fY);
                                break;
                            case SkPath::kCubic_Verb:
                                simple.cubicTo(lastCurve[1].fX, lastCurve[1].fY,
                                        lastCurve[2].fX, lastCurve[2].fY,
                                        lastCurve[3].fX, lastCurve[3].fY);
                                break;
                        }
                        if (gShowDebugf) {
                            const char* verbStr[] = {"", "line", "quad", "cubic"};
                            SkDebugf("%s %sTo-1 (%g,%g)\n", __FUNCTION__,
                                    verbStr[lastVerb], lastCurve[lastVerb].fX,
                                    lastCurve[lastVerb].fY);
                        }
                    }
                    if (gap || lastVerb != SkPath::kLine_Verb || !extendLine(lastCurve, *end)) {
                        // FIXME: see comment in bridge -- this probably
                        // conceals errors
                        SkASSERT(lastCurve[lastVerb] == *start ||
                                (fFill && UlpsDiff(lastCurve[lastVerb].fY, start->fY) <= 10));
                        simple.lineTo(start->fX, start->fY);
                        if (gShowDebugf) {
                            SkDebugf("%s lineTo (%g,%g)\n", __FUNCTION__,
                                    start->fX, start->fY);
                        }
                        lastCurve[0] = *start;
                    }
                    if (verb == SkPath::kQuad_Verb) {
                        lastCurve[1] = ptArray[1];
                    } else if (verb == SkPath::kCubic_Verb) {
                        if (advance < 0) {
                            lastCurve[1] = ptArray[2];
                            lastCurve[2] = ptArray[1];
                        } else {
                            lastCurve[1] = ptArray[1];
                            lastCurve[2] = ptArray[2];
                        }
                    }
                    lastCurve[verb] = *end;
                    lastVerb = verb;
                }
                if (advance < 0) {
                    edgeIndex = fTops[listIndex];
                    fTops[listIndex] = 0;
                 } else {
                    edgeIndex = fBottoms[listIndex];
                    fBottoms[listIndex] = 0;
                }
                if (edgeIndex) {
                    listIndex = abs(edgeIndex) - 1;
                    if (edgeIndex < 0) {
                        fTops[listIndex] = 0;
                    } else {
                        fBottoms[listIndex] = 0;
                    }
                }
                if (edgeIndex == closeEdgeIndex || edgeIndex == 0) {
                    if (lastCurve[lastVerb] != firstPt) {
                        switch (lastVerb) {
                            case SkPath::kLine_Verb:
                                simple.lineTo(lastCurve[1].fX, lastCurve[1].fY);
                                break;
                            case SkPath::kQuad_Verb:
                                simple.quadTo(lastCurve[1].fX, lastCurve[1].fY,
                                        lastCurve[2].fX, lastCurve[2].fY);
                                break;
                            case SkPath::kCubic_Verb:
                                simple.cubicTo(lastCurve[1].fX, lastCurve[1].fY,
                                        lastCurve[2].fX, lastCurve[2].fY,
                                        lastCurve[3].fX, lastCurve[3].fY);
                                break;
                        }
                        if (gShowDebugf) {
                            const char* verbStr[] = {"", "line", "quad", "cubic"};
                            SkDebugf("%s %sTo last (%g, %g)\n", __FUNCTION__,
                                    verbStr[lastVerb],
                                    lastCurve[lastVerb].fX, lastCurve[lastVerb].fY);
                        }
                    }
                    simple.lineTo(firstPt.fX, firstPt.fY);
                    simple.close();
                    if (gShowDebugf) {
                        SkDebugf("%s close (%g, %g)\n", __FUNCTION__,
                                firstPt.fX, firstPt.fY);
                    }
                    break;
                }
                // if this and next edge go different directions 
                if (advance > 0 ^ edgeIndex < 0) {
                    advance = -advance;
                }
            } while (edgeIndex);
        } while (true);
    }

    // sort points by y, then x
    // if x/y is identical, sort bottoms before tops
    // if identical and both tops/bottoms, sort by angle
    static bool lessThan(SkTArray<OutEdge>& edges, const int one,
            const int two) {
        const OutEdge& oneEdge = edges[abs(one) - 1];
        int oneIndex = one < 0 ? 0 : oneEdge.fVerb;
        const SkPoint& startPt1 = oneEdge.fPts[oneIndex];
        const OutEdge& twoEdge = edges[abs(two) - 1];
        int twoIndex = two < 0 ? 0 : twoEdge.fVerb;
        const SkPoint& startPt2 = twoEdge.fPts[twoIndex];
        if (startPt1.fY != startPt2.fY) {
    #if DEBUG_OUT_LESS_THAN
            SkDebugf("%s %d<%d (%g,%g) %s startPt1.fY < startPt2.fY\n", __FUNCTION__,
                    one, two, startPt1.fY, startPt2.fY,
                    startPt1.fY < startPt2.fY ? "true" : "false");
    #endif
            return startPt1.fY < startPt2.fY;
        }
        if (startPt1.fX != startPt2.fX) {
    #if DEBUG_OUT_LESS_THAN
            SkDebugf("%s %d<%d (%g,%g) %s startPt1.fX < startPt2.fX\n", __FUNCTION__,
                    one, two, startPt1.fX, startPt2.fX,
                    startPt1.fX < startPt2.fX ? "true" : "false");
    #endif
            return startPt1.fX < startPt2.fX;
        }
        const SkPoint& endPt1 = oneEdge.fPts[oneIndex ^ oneEdge.fVerb];
        const SkPoint& endPt2 = twoEdge.fPts[twoIndex ^ twoEdge.fVerb];
        SkScalar dy1 = startPt1.fY - endPt1.fY;
        SkScalar dy2 = startPt2.fY - endPt2.fY;
        SkScalar dy1y2 = dy1 * dy2;
        if (dy1y2 < 0) { // different signs
    #if DEBUG_OUT_LESS_THAN
                SkDebugf("%s %d<%d %s dy1 > 0\n", __FUNCTION__, one, two,
                        dy1 > 0 ? "true" : "false");
    #endif
            return dy1 > 0; // one < two if one goes up and two goes down
        }
        if (dy1y2 == 0) {
    #if DEBUG_OUT_LESS_THAN
            SkDebugf("%s %d<%d %s endPt1.fX < endPt2.fX\n", __FUNCTION__,
                    one, two, endPt1.fX < endPt2.fX ? "true" : "false");
    #endif
            return endPt1.fX < endPt2.fX;
        } 
        SkScalar dx1y2 = (startPt1.fX - endPt1.fX) * dy2;
        SkScalar dx2y1 = (startPt2.fX - endPt2.fX) * dy1;
    #if DEBUG_OUT_LESS_THAN
        SkDebugf("%s %d<%d %s dy2 < 0 ^ dx1y2 < dx2y1\n", __FUNCTION__,
                one, two, dy2 < 0 ^ dx1y2 < dx2y1 ? "true" : "false");
    #endif
        return dy2 > 0 ^ dx1y2 < dx2y1;
    }

    // Sort the indices of paired points and then create more indices so
    // assemble() can find the next edge and connect the top or bottom
    void bridge() {
        size_t index;
        size_t count = fEdges.count();
        if (!count) {
            return;
        }
        SkASSERT(!fFill || count > 1);
        fTops.setCount(count);
        sk_bzero(fTops.begin(), sizeof(fTops[0]) * count);
        fBottoms.setCount(count);
        sk_bzero(fBottoms.begin(), sizeof(fBottoms[0]) * count);
        SkTDArray<int> order;
        for (index = 1; index <= count; ++index) {
            *order.append() = -index;
        }
        for (index = 1; index <= count; ++index) {
            *order.append() = index;
        }
        QSort<SkTArray<OutEdge>, int>(fEdges, order.begin(), order.end() - 1, lessThan);
        int* lastPtr = order.end() - 1;
        int* leftPtr = order.begin();
        while (leftPtr < lastPtr) {
            int leftIndex = *leftPtr;
            int leftOutIndex = abs(leftIndex) - 1;
            const OutEdge& left = fEdges[leftOutIndex];
            int* rightPtr = leftPtr + 1;
            int rightIndex = *rightPtr;
            int rightOutIndex = abs(rightIndex) - 1;
            const OutEdge& right = fEdges[rightOutIndex];
            bool pairUp = fFill;
            if (!pairUp) {
                const SkPoint& leftMatch =
                        left.fPts[leftIndex < 0 ? 0 : left.fVerb];
                const SkPoint& rightMatch =
                        right.fPts[rightIndex < 0 ? 0 : right.fVerb];
                pairUp = leftMatch == rightMatch;
            } else {
        #if DEBUG_OUT
        // FIXME : not happy that error in low bit is allowed
        // this probably conceals error elsewhere
                if (UlpsDiff(left.fPts[leftIndex < 0 ? 0 : left.fVerb].fY,
                        right.fPts[rightIndex < 0 ? 0 : right.fVerb].fY) > 1) {
                    *fMismatches.append() = leftIndex;
                    if (rightPtr == lastPtr) {
                        *fMismatches.append() = rightIndex;
                    }
                    pairUp = false;
                }
        #else
                SkASSERT(UlpsDiff(left.fPts[leftIndex < 0 ? 0 : left.fVerb].fY,
                        right.fPts[rightIndex < 0 ? 0 : right.fVerb].fY) <= 10);
        #endif
            }
            if (pairUp) {
                if (leftIndex < 0) {
                    fTops[leftOutIndex] = rightIndex;
                } else {
                    fBottoms[leftOutIndex] = rightIndex;
                }
                if (rightIndex < 0) {
                    fTops[rightOutIndex] = leftIndex;
                } else {
                    fBottoms[rightOutIndex] = leftIndex;
                }
                ++rightPtr;
            }
            leftPtr = rightPtr;
        }
#if DEBUG_OUT
        int* mismatch = fMismatches.begin();
        while (mismatch != fMismatches.end()) {
            int leftIndex = *mismatch++;
            int leftOutIndex = abs(leftIndex) - 1;
            const OutEdge& left = fEdges[leftOutIndex];
            const SkPoint& leftPt = left.fPts[leftIndex < 0 ? 0 : left.fVerb];
            SkDebugf("%s left=%d %s (%1.9g,%1.9g)\n",
                    __FUNCTION__, left.fID, leftIndex < 0 ? "top" : "bot",
                    leftPt.fX, leftPt.fY);
        }
        SkASSERT(fMismatches.count() == 0);
#endif
    }

protected:
    SkTArray<OutEdge> fEdges;
    SkTDArray<int> fTops;
    SkTDArray<int> fBottoms;
    bool fFill;
#if DEBUG_OUT
    SkTDArray<int> fMismatches;
#endif
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
};

class Intercepts {
public:
    Intercepts()
        : fTopIntercepts(0)
        , fBottomIntercepts(0)
        , fExplicit(false) {
    }
    
    Intercepts& operator=(const Intercepts& src) {
        fTs = src.fTs;
        fTopIntercepts = src.fTopIntercepts;
        fBottomIntercepts = src.fBottomIntercepts;
    }

    // OPTIMIZATION: remove this function if it's never called
    double t(int tIndex) const {
        if (tIndex == 0) {
            return 0;
        }
        if (tIndex > fTs.count()) {
            return 1;
        }
        return fTs[tIndex - 1];
    }

#if DEBUG_DUMP
    void dump(const SkPoint* pts, SkPath::Verb verb) {
        const char className[] = "Intercepts";
        const int tab = 8;
        for (int i = 0; i < fTs.count(); ++i) {
            SkPoint out;
            switch (verb) {
                case SkPath::kLine_Verb:
                    LineXYAtT(pts, fTs[i], &out);
                    break;
                case SkPath::kQuad_Verb:
                    QuadXYAtT(pts, fTs[i], &out);
                    break;
                case SkPath::kCubic_Verb:
                    CubicXYAtT(pts, fTs[i], &out);
                    break;
                default:
                    SkASSERT(0);
            }
            SkDebugf("%*s.fTs[%d]=%1.9g (%1.9g,%1.9g)\n", tab + sizeof(className),
                    className, i, fTs[i], out.fX, out.fY);
        }
        SkDebugf("%*s.fTopIntercepts=%u\n", tab + sizeof(className),
                className, fTopIntercepts);
        SkDebugf("%*s.fBottomIntercepts=%u\n", tab + sizeof(className),
                className, fBottomIntercepts);
    }
#endif

    SkTDArray<double> fTs;
    unsigned char fTopIntercepts; // 0=init state 1=1 edge >1=multiple edges
    unsigned char fBottomIntercepts;
    bool fExplicit; // if set, suppress 0 and 1
    
};

struct HorizontalEdge {
    bool operator<(const HorizontalEdge& rh) const {
        return fY == rh.fY ? fLeft == rh.fLeft ? fRight < rh.fRight
                : fLeft < rh.fLeft : fY < rh.fY;
    }

#if DEBUG_DUMP
    void dump() {
        const char className[] = "HorizontalEdge";
        const int tab = 4;
        SkDebugf("%*s.fLeft=%1.9g\n", tab + sizeof(className), className, fLeft);
        SkDebugf("%*s.fRight=%1.9g\n", tab + sizeof(className), className, fRight);
        SkDebugf("%*s.fY=%1.9g\n", tab + sizeof(className), className, fY);
    }
#endif

    SkScalar fLeft;
    SkScalar fRight;
    SkScalar fY;
};

struct InEdge {
    bool operator<(const InEdge& rh) const {
        return fBounds.fTop == rh.fBounds.fTop
                ? fBounds.fLeft < rh.fBounds.fLeft
                : fBounds.fTop < rh.fBounds.fTop;
    }

    // Avoid collapsing t values that are close to the same since
    // we walk ts to describe consecutive intersections. Since a pair of ts can
    // be nearly equal, any problems caused by this should be taken care
    // of later. 
    int add(double* ts, size_t count, ptrdiff_t verbIndex) {
        // FIXME: in the pathological case where there is a ton of intercepts, binary search?
        bool foundIntercept = false;
        int insertedAt = -1;
        Intercepts& intercepts = fIntercepts[verbIndex];
        for (size_t index = 0; index < count; ++index) {
            double t = ts[index];
            if (t <= 0) {
                intercepts.fTopIntercepts <<= 1;
                fContainsIntercepts |= ++intercepts.fTopIntercepts > 1;
                continue;
            }
            if (t >= 1) {
                intercepts.fBottomIntercepts <<= 1;
                fContainsIntercepts |= ++intercepts.fBottomIntercepts > 1;
                continue;
            }
            fIntersected = true;
            foundIntercept = true;
            size_t tCount = intercepts.fTs.count();
            double delta;
            for (size_t idx2 = 0; idx2 < tCount; ++idx2) {
                if (t <= intercepts.fTs[idx2]) {
                    // FIXME: ?  if (t < intercepts.fTs[idx2]) // failed
                    delta = intercepts.fTs[idx2] - t;
                    if (delta > 0) {
                        insertedAt = idx2;
                        *intercepts.fTs.insert(idx2) = t;
                    }
                    goto nextPt;
                }
            }
            if (tCount == 0 || (delta = t - intercepts.fTs[tCount - 1]) > 0) {
                insertedAt = tCount;
                *intercepts.fTs.append() = t;
            }
    nextPt:
            ;
        }
        fContainsIntercepts |= foundIntercept;
        return insertedAt;
    }
    
    void addPartial(SkTArray<InEdge>& edges, int id, int ptStart, int ptEnd,
            int verbStart, int verbEnd) {
        InEdge* edge = edges.push_back_n(1);
        int verbCount = verbEnd - verbStart;
        edge->fIntercepts.push_back_n(verbCount);
        uint8_t* verbs = &fVerbs[verbStart];
        for (int ceptIdx = 0; ceptIdx < verbCount; ++ceptIdx) {
            edge->fIntercepts[ceptIdx] = fIntercepts[verbStart + ceptIdx];
        }
        edge->fPts.append(ptEnd - ptStart, &fPts[ptStart]);
        edge->fVerbs.append(verbCount, &fVerbs[verbStart]);
        edge->setBounds();
        edge->fID = id + edges.count();
        edge->fWinding = fWinding;
        edge->fContainsIntercepts = fContainsIntercepts; // FIXME: may not be correct -- but do we need to know?
    }

    void addSplit(SkTArray<InEdge>& edges, int id, SkPoint* pts, uint8_t verb,
            double* ts, int tCount, bool flipped) {
        InEdge* edge = edges.push_back_n(1);
        edge->fIntercepts.push_back_n(1);
        edge->fIntercepts[0].fTs.append(tCount, ts);
        edge->fIntercepts[0].fExplicit = true;
        edge->fPts.append(verb, pts);
        edge->fVerbs.append(1, &verb);
        edge->setBounds();
        edge->fID = id + edges.count();
        edge->fWinding = fWinding;
        edge->fContainsIntercepts = fContainsIntercepts; // FIXME: may not be correct -- but do we need to know?
        if (flipped) {
            flip();
        }
    }

    bool cached(const InEdge* edge) {
        // FIXME: in the pathological case where there is a ton of edges, binary search?
        size_t count = fCached.count();
        for (size_t index = 0; index < count; ++index) {
            if (edge == fCached[index]) {
                return true;
            }
            if (edge < fCached[index]) {
                *fCached.insert(index) = edge;
                return false;
            }
        }
        *fCached.append() = edge;
        return false;
    }

    void complete(signed char winding, int id) {
        setBounds();
        fIntercepts.push_back_n(fVerbs.count());
        if ((fWinding = winding) < 0) { // reverse verbs, pts, if bottom to top
            flip();
        }
        fContainsIntercepts = fIntersected = false;
        fID = id;
    }
    
     void flip() {
        size_t index;
        size_t last = fPts.count() - 1;
        for (index = 0; index < last; ++index, --last) {
            SkTSwap<SkPoint>(fPts[index], fPts[last]);
        }
        last = fVerbs.count() - 1;
        for (index = 0; index < last; ++index, --last) {
            SkTSwap<uint8_t>(fVerbs[index], fVerbs[last]);
        }
    }

    void reset() {
        fCached.reset();
        fIntercepts.reset();
        fPts.reset();
        fVerbs.reset();
        fBounds.set(SK_ScalarMax, SK_ScalarMax, SK_ScalarMax, SK_ScalarMax);
        fID = -1;
        fWinding = 0;
        fContainsIntercepts = false;
        fIntersected = false;
    }

    void setBounds() {
        SkPoint* ptPtr = fPts.begin();
        SkPoint* ptLast = fPts.end();
        if (ptPtr == ptLast) {
            SkDebugf("%s empty edge\n", __FUNCTION__);
            SkASSERT(0);
            // FIXME: delete empty edge?
            return;
        }
        fBounds.set(ptPtr->fX, ptPtr->fY, ptPtr->fX, ptPtr->fY);
        ++ptPtr;
        while (ptPtr != ptLast) {
            fBounds.growToInclude(ptPtr->fX, ptPtr->fY);
            ++ptPtr;
        }
    }

    void splitInflectionPts(SkTArray<InEdge>& edges, int idStart) {
        if (!fIntersected) {
            return;
        }
        uint8_t* verbs = fVerbs.begin();
        SkPoint* pts = fPts.begin();
        int lastVerb = 0;
        int lastPt = 0;
        uint8_t verb;
        bool edgeSplit = false;
        for (int ceptIdx = 0; ceptIdx < fIntercepts.count(); ++ceptIdx, pts += verb) {
            Intercepts& intercepts = fIntercepts[ceptIdx];
            verb = *verbs++;
            if (verb <= SkPath::kLine_Verb) {
                continue;
            }
            size_t tCount = intercepts.fTs.count();
            if (!tCount) {
                continue;
            }
            size_t tIndex = -1;
            SkScalar y = pts[0].fY;
            int lastSplit = 0;
            int firstSplit = -1;
            bool curveSplit = false;
            while (++tIndex < tCount) {
                double nextT = intercepts.fTs[tIndex];
                SkScalar nextY = verb == SkPath::kQuad_Verb
                        ? QuadYAtT(pts, nextT) : CubicYAtT(pts, nextT);
                if (nextY < y) {
                    edgeSplit = curveSplit = true;
                    if (firstSplit < 0) {
                        firstSplit = tIndex;
                        int nextPt = pts - fPts.begin();
                        int nextVerb = verbs - 1 - fVerbs.begin();
                        if (lastVerb < nextVerb) {
                            addPartial(edges, idStart, lastPt, nextPt,
                                    lastVerb, nextVerb);
            #if DEBUG_SPLIT
                            SkDebugf("%s addPartial 1 edge=%d\n", __FUNCTION__,
                                    fID);
            #endif
                        }
                        lastPt = nextPt;
                        lastVerb = nextVerb;
                    }
                } else {
                    if (firstSplit >= 0) {
                        if (lastSplit < firstSplit) {
                            addSplit(edges, idStart, pts, verb,
                                    &intercepts.fTs[lastSplit],
                                    firstSplit - lastSplit, false);
            #if DEBUG_SPLIT
                            SkDebugf("%s addSplit 1 edge=%d tIndex=%d,%d\n",
                                    __FUNCTION__, fID, lastSplit, firstSplit);
            #endif
                        }
                        addSplit(edges, idStart, pts, verb,
                                &intercepts.fTs[firstSplit],
                                tIndex - firstSplit, true);
            #if DEBUG_SPLIT
                        SkDebugf("%s addSplit 2 edge=%d tIndex=%d,%d flip\n",
                                __FUNCTION__, fID, firstSplit, tIndex);
            #endif
                        lastSplit = tIndex;
                        firstSplit = -1;
                    }
                }
                y = nextY;
            }
            if (curveSplit) {
                if (firstSplit < 0) {
                    firstSplit = lastSplit;
                } else {
                    addSplit(edges, idStart, pts, verb,
                            &intercepts.fTs[lastSplit], firstSplit - lastSplit,
                            false);
            #if DEBUG_SPLIT
                    SkDebugf("%s addSplit 3 edge=%d tIndex=%d,%d\n", __FUNCTION__,
                            fID, lastSplit, firstSplit);
            #endif
                }
                addSplit(edges, idStart, pts, verb,
                        &intercepts.fTs[firstSplit], tIndex - firstSplit,
                        pts[verb].fY < y);
            #if DEBUG_SPLIT
                SkDebugf("%s addSplit 4 edge=%d tIndex=%d,%d %s\n", __FUNCTION__,
                        fID, firstSplit, tIndex, pts[verb].fY < y ? "flip" : "");
            #endif
            }
        }
        // collapse remainder -- if there's nothing left, clear it somehow?
        if (edgeSplit) {
            int nextVerb = verbs - 1 - fVerbs.begin();
            if (lastVerb < nextVerb) {
                int nextPt = pts - fPts.begin();
                addPartial(edges, idStart, lastPt, nextPt,
                        lastVerb, nextVerb);
            #if DEBUG_SPLIT
                SkDebugf("%s addPartial 2 edge=%d\n", __FUNCTION__, fID);
            #endif
            }
            // OPTIMIZATION: reuse the edge instead of marking it empty
            reset();
        }
    }

#if DEBUG_DUMP
    void dump() {
        int i;
        const char className[] = "InEdge";
        const int tab = 4;
        SkDebugf("InEdge %p (edge=%d)\n", this, fID);
        for (i = 0; i < fCached.count(); ++i) {
            SkDebugf("%*s.fCached[%d]=0x%08x\n", tab + sizeof(className),
                    className, i, fCached[i]);
        }
        uint8_t* verbs = fVerbs.begin();
        SkPoint* pts = fPts.begin();
        for (i = 0; i < fIntercepts.count(); ++i) {
            SkDebugf("%*s.fIntercepts[%d]:\n", tab + sizeof(className),
                    className, i);
            fIntercepts[i].dump(pts, (SkPath::Verb) *verbs);
            pts += *verbs++;
        }
        for (i = 0; i < fPts.count(); ++i) {
            SkDebugf("%*s.fPts[%d]=(%1.9g,%1.9g)\n", tab + sizeof(className),
                    className, i, fPts[i].fX, fPts[i].fY);
        }
        for (i = 0; i < fVerbs.count(); ++i) {
            SkDebugf("%*s.fVerbs[%d]=%d\n", tab + sizeof(className),
                    className, i, fVerbs[i]);
        }
        SkDebugf("%*s.fBounds=(%1.9g. %1.9g, %1.9g, %1.9g)\n", tab + sizeof(className),
                className, fBounds.fLeft, fBounds.fTop,
                fBounds.fRight, fBounds.fBottom);
        SkDebugf("%*s.fWinding=%d\n", tab + sizeof(className), className,
                fWinding);
        SkDebugf("%*s.fContainsIntercepts=%d\n", tab + sizeof(className),
                className, fContainsIntercepts);
        SkDebugf("%*s.fIntersected=%d\n", tab + sizeof(className),
                className, fIntersected);
    }
#endif

    // FIXME: temporary data : move this to a separate struct?
    SkTDArray<const InEdge*> fCached; // list of edges already intercepted
    SkTArray<Intercepts> fIntercepts; // one per verb

    // persistent data
    SkTDArray<SkPoint> fPts;
    SkTDArray<uint8_t> fVerbs;
    Bounds fBounds;
    int fID;
    signed char fWinding;
    bool fContainsIntercepts;
    bool fIntersected;
};

class InEdgeBuilder {
public:

InEdgeBuilder(const SkPath& path, bool ignoreHorizontal, SkTArray<InEdge>& edges,
        SkTDArray<HorizontalEdge>& horizontalEdges) 
    : fPath(path)
    , fCurrentEdge(NULL)
    , fEdges(edges)
    , fID(0)
    , fHorizontalEdges(horizontalEdges)
    , fIgnoreHorizontal(ignoreHorizontal)
    , fContainsCurves(false)
{
    walk();
}

bool containsCurves() const {
    return fContainsCurves;
}

int nextID() {
    return ++fID;
}

protected:

void addEdge() {
    SkASSERT(fCurrentEdge);
    fCurrentEdge->fPts.append(fPtCount - fPtOffset, &fPts[fPtOffset]);
    fPtOffset = 1;
    *fCurrentEdge->fVerbs.append() = fVerb;
}

bool complete() {
    if (fCurrentEdge && fCurrentEdge->fVerbs.count()) {
        fCurrentEdge->complete(fWinding, nextID());
        fCurrentEdge = NULL;
        return true;
    }
    return false;
}

int direction(int count) {
    fPtCount = count;
    if (fIgnoreHorizontal && isHorizontal()) {
        return 0;
    }
    int last = count - 1;
    return fPts[0].fY == fPts[last].fY
            ? fPts[0].fX == fPts[last].fX ? 0 : fPts[0].fX < fPts[last].fX
            ? 1 : -1 : fPts[0].fY < fPts[last].fY ? 1 : -1;
}

bool isHorizontal() {
    SkScalar y = fPts[0].fY;
    for (int i = 1; i < fPtCount; ++i) {
        if (fPts[i].fY != y) {
            return false;
        }
    }
    return true;
}

void startEdge() {
    if (!fCurrentEdge) {
        fCurrentEdge = fEdges.push_back_n(1);
    }
    fWinding = 0;
    fPtOffset = 0;
}

void walk() {
    SkPath::Iter iter(fPath, true);
    int winding = 0;
    while ((fVerb = iter.next(fPts)) != SkPath::kDone_Verb) {
        switch (fVerb) {
            case SkPath::kMove_Verb:
                startEdge();
                continue;
            case SkPath::kLine_Verb:
                winding = direction(2);
                break;
            case SkPath::kQuad_Verb:
                winding = direction(3);
                fContainsCurves = true;
                break;
            case SkPath::kCubic_Verb:
                winding = direction(4);
                fContainsCurves = true;
                break;
            case SkPath::kClose_Verb:
                SkASSERT(fCurrentEdge);
                complete();
                continue;
            default:
                SkDEBUGFAIL("bad verb");
                return;
        }
        if (winding == 0) {
            HorizontalEdge* horizontalEdge = fHorizontalEdges.append();
            // FIXME: for degenerate quads and cubics, compute x extremes
            horizontalEdge->fLeft = fPts[0].fX;
            horizontalEdge->fRight = fPts[fVerb].fX;
            horizontalEdge->fY = fPts[0].fY;
            if (horizontalEdge->fLeft > horizontalEdge->fRight) {
                SkTSwap<SkScalar>(horizontalEdge->fLeft, horizontalEdge->fRight);
            }
            if (complete()) {
                startEdge();
            }
            continue;
        }
        if (fWinding + winding == 0) {
            // FIXME: if prior verb or this verb is a horizontal line, reverse
            // it instead of starting a new edge
            SkASSERT(fCurrentEdge);
            if (complete()) {
                startEdge();
            }
        }
        fWinding = winding;
        addEdge();
    }
    if (!complete()) {
        if (fCurrentEdge) {
            fEdges.pop_back();
        }
    }
}

private:
    const SkPath& fPath;
    InEdge* fCurrentEdge;
    SkTArray<InEdge>& fEdges;
    SkTDArray<HorizontalEdge>& fHorizontalEdges;
    SkPoint fPts[4];
    SkPath::Verb fVerb;
    int fPtCount;
    int fPtOffset;
    int fID;
    int8_t fWinding;
    bool fIgnoreHorizontal;
    bool fContainsCurves;
};

struct WorkEdge {
    SkScalar bottom() const {
        return fPts[verb()].fY;
    }

    void init(const InEdge* edge) {
        fEdge = edge;
        fPts = edge->fPts.begin();
        fVerb = edge->fVerbs.begin();
    }

    bool advance() {
        SkASSERT(fVerb < fEdge->fVerbs.end());
        fPts += *fVerb++;
        return fVerb != fEdge->fVerbs.end();
    }

    SkPath::Verb lastVerb() const {
        SkASSERT(fVerb > fEdge->fVerbs.begin());
        return (SkPath::Verb) fVerb[-1];
    }


    SkPath::Verb verb() const {
        return (SkPath::Verb) *fVerb;
    }

    ptrdiff_t verbIndex() const {
        return fVerb - fEdge->fVerbs.begin();
    }

    int winding() const {
        return fEdge->fWinding;
    }

    const InEdge* fEdge;
    const SkPoint* fPts;
    const uint8_t* fVerb;
};

// always constructed with SkTDArray because new edges are inserted
// this may be a inappropriate optimization, suggesting that a separate array of
// ActiveEdge* may be faster to insert and search

// OPTIMIZATION: Brian suggests that global sorting should be unnecessary, since
// as active edges are introduced, only local sorting should be required
class ActiveEdge {
public:
    bool operator<(const ActiveEdge& rh) const {
        double topD = fAbove.fX - rh.fAbove.fX;
        if (rh.fAbove.fY < fAbove.fY) {
            topD = (rh.fBelow.fY - rh.fAbove.fY) * topD
                    - (fAbove.fY - rh.fAbove.fY) * (rh.fBelow.fX - rh.fAbove.fX);
        } else if (rh.fAbove.fY > fAbove.fY) {
            topD = (fBelow.fY - fAbove.fY) * topD
                    + (rh.fAbove.fY - fAbove.fY) * (fBelow.fX - fAbove.fX);
        }
        double botD = fBelow.fX - rh.fBelow.fX;
        if (rh.fBelow.fY > fBelow.fY) {
            botD = (rh.fBelow.fY - rh.fAbove.fY) * botD
                    - (fBelow.fY - rh.fBelow.fY) * (rh.fBelow.fX - rh.fAbove.fX);
        } else if (rh.fBelow.fY < fBelow.fY) {
            botD = (fBelow.fY - fAbove.fY) * botD
                    + (rh.fBelow.fY - fBelow.fY) * (fBelow.fX - fAbove.fX);
        }
        // return sign of greater absolute value
        return (fabs(topD) > fabs(botD) ? topD : botD) < 0;
    }

    // OPTIMIZATION: fold return statements into one
    bool operator_less_than(const ActiveEdge& rh) const {
        if (rh.fAbove.fY - fAbove.fY > fBelow.fY - rh.fAbove.fY
                && fBelow.fY < rh.fBelow.fY
                || fAbove.fY - rh.fAbove.fY < rh.fBelow.fY - fAbove.fY
                && rh.fBelow.fY < fBelow.fY) {
            // FIXME: need to compute distance, not check for points equal
            const SkPoint& check = rh.fBelow.fY <= fBelow.fY
                    && fBelow != rh.fBelow ? rh.fBelow :
                    rh.fAbove;
        #if DEBUG_ACTIVE_LESS_THAN
            SkDebugf("%s 1 %c %cthis (edge=%d) {%g,%g %g,%g}"
                    " < rh (edge=%d) {%g,%g %g,%g} upls=(%d)\n", __FUNCTION__,
                    rh.fBelow.fY <= fBelow.fY && fBelow != rh.fBelow ? 'B' : 'A',
                    (check.fY - fAbove.fY) * (fBelow.fX - fAbove.fX)
                    < (fBelow.fY - fAbove.fY) * (check.fX - fAbove.fX) ? ' '
                    : '!', ID(), fAbove.fX, fAbove.fY, fBelow.fX, fBelow.fY,
                    rh.ID(), rh.fAbove.fX, rh.fAbove.fY, rh.fBelow.fX, rh.fBelow.fY,
                    UlpsDiff((check.fY - fAbove.fY) * (fBelow.fX - fAbove.fX),
                        (fBelow.fY - fAbove.fY) * (check.fX - fAbove.fX)));
        #endif
            return (check.fY - fAbove.fY) * (fBelow.fX - fAbove.fX)
                    < (fBelow.fY - fAbove.fY) * (check.fX - fAbove.fX);
        }
        // FIXME: need to compute distance, not check for points equal
        const SkPoint& check = fBelow.fY <= rh.fBelow.fY 
                && fBelow != rh.fBelow ? fBelow : fAbove;
    #if DEBUG_ACTIVE_LESS_THAN
        SkDebugf("%s 2 %c %cthis (edge=%d) {%g,%g %g,%g}"
                " < rh (edge=%d) {%g,%g %g,%g} upls=(%d) (%d,%d)\n", __FUNCTION__,
                fBelow.fY <= rh.fBelow.fY && fBelow != rh.fBelow ? 'B' : 'A',
                (rh.fBelow.fY - rh.fAbove.fY) * (check.fX - rh.fAbove.fX)
                < (check.fY - rh.fAbove.fY) * (rh.fBelow.fX - rh.fAbove.fX) 
                ? ' ' : '!', ID(), fAbove.fX, fAbove.fY, fBelow.fX, fBelow.fY,
                rh.ID(), rh.fAbove.fX, rh.fAbove.fY, rh.fBelow.fX, rh.fBelow.fY,
                UlpsDiff((rh.fBelow.fY - rh.fAbove.fY) * (check.fX - rh.fAbove.fX),
                    (check.fY - rh.fAbove.fY) * (rh.fBelow.fX - rh.fAbove.fX)),
                UlpsDiff(fBelow.fX, rh.fBelow.fX), UlpsDiff(fBelow.fY, rh.fBelow.fY));
    #endif
        return (rh.fBelow.fY - rh.fAbove.fY) * (check.fX - rh.fAbove.fX)
                < (check.fY - rh.fAbove.fY) * (rh.fBelow.fX - rh.fAbove.fX);
    }

    // If a pair of edges are nearly coincident for some span, add a T in the
    // edge so it can be shortened to match the other edge. Note that another
    // approach is to trim the edge after it is added to the OutBuilder list --
    // FIXME: since this has no effect if the edge is already done (i.e.,
    // fYBottom >= y) maybe this can only be done by calling trimLine later.
    void addTatYBelow(SkScalar y) {
        if (fBelow.fY <= y || fYBottom >= y) {
            return;
        }
        addTatYInner(y);
        fFixBelow = true;
    }

    void addTatYAbove(SkScalar y) {
        if (fBelow.fY <= y) {
            return;
        }
        addTatYInner(y);
    }

    void addTatYInner(SkScalar y) {
        if (fWorkEdge.fPts[0].fY > y) {
            backup(y);
        }
        SkScalar left = fWorkEdge.fPts[0].fX;
        SkScalar right = fWorkEdge.fPts[1].fX;
        if (left > right) {
            SkTSwap(left, right);
        }
        double ts[2];
        int pts = LineIntersect(fWorkEdge.fPts, left, right, y, ts);
        SkASSERT(pts == 1);
        // An ActiveEdge or WorkEdge has no need to modify the T values computed
        // in the InEdge, except in the following case. If a pair of edges are
        // nearly coincident, this may not be detected when the edges are
        // intersected. Later, when sorted, and this near-coincidence is found,
        // an additional t value must be added, requiring the cast below.
        InEdge* writable = const_cast<InEdge*>(fWorkEdge.fEdge);
        int insertedAt = writable->add(ts, pts, fWorkEdge.verbIndex());
    #if DEBUG_ADJUST_COINCIDENT
        SkDebugf("%s edge=%d y=%1.9g t=%1.9g\n", __FUNCTION__, ID(), y, ts[0]);
    #endif
        if (insertedAt >= 0) {
            if (insertedAt + 1 < fTIndex) {
                SkASSERT(insertedAt + 2 == fTIndex);
                --fTIndex;
            }
        }
    }

    bool advanceT() {
        SkASSERT(fTIndex <= fTs->count() - fExplicitTs);
        return ++fTIndex <= fTs->count() - fExplicitTs;
    }

    bool advance() {
    // FIXME: flip sense of next
        bool result = fWorkEdge.advance();
        fDone = !result;
        initT();
        return result;
    }

    void backup(SkScalar y) {
        do {
            SkASSERT(fWorkEdge.fEdge->fVerbs.begin() < fWorkEdge.fVerb);
            fWorkEdge.fPts -= *--fWorkEdge.fVerb;
            SkASSERT(fWorkEdge.fEdge->fPts.begin() <= fWorkEdge.fPts);
        } while (fWorkEdge.fPts[0].fY >= y);
        initT();
        SkASSERT(!fExplicitTs);
        fTIndex = fTs->count() + 1;
    }

    void calcLeft(SkScalar y) {
        // OPTIMIZE: put a kDone_Verb at the end of the verb list?
        if (fDone || fBelow.fY > y) {
            return; // nothing to do; use last
        }
        calcLeft();
        if (fAbove.fY == fBelow.fY) {
            SkDebugf("%s edge=%d fAbove.fY != fBelow.fY %1.9g\n", __FUNCTION__,
                    ID(), fAbove.fY);
        }
    }

    void calcLeft() {
        void (*xyAtTFunc)(const SkPoint a[], double t, SkPoint* out);
        switch (fWorkEdge.verb()) {
            case SkPath::kLine_Verb:
                xyAtTFunc = LineXYAtT;
                break;
            case SkPath::kQuad_Verb:
                xyAtTFunc = QuadXYAtT;
                break;
            case SkPath::kCubic_Verb:
                xyAtTFunc = CubicXYAtT;
                break;
            default:
                SkASSERT(0);
        }
        // OPTIMIZATION: if fXAbove, fXBelow have already been computed
        //  for the fTIndex, don't do it again
        // For identical x, this lets us know which edge is first.
        // If both edges have T values < 1, check x at next T (fXBelow).
        int add = (fTIndex <= fTs->count() - fExplicitTs) - 1;
        double tAbove = t(fTIndex + add);
        (*xyAtTFunc)(fWorkEdge.fPts, tAbove, &fAbove);
        double tBelow = t(fTIndex - ~add);
        (*xyAtTFunc)(fWorkEdge.fPts, tBelow, &fBelow);
        SkASSERT(tAbove != tBelow);
        // FIXME: this can loop forever
        // need a break if we hit the end
        // FIXME: in unit test, figure out how explicit Ts work as well
        while (fAbove.fY == fBelow.fY) {
            if (add < 0 || fTIndex == fTs->count()) {
                add -= 1;
                SkASSERT(fTIndex + add >= 0);
                tAbove = t(fTIndex + add);
                (*xyAtTFunc)(fWorkEdge.fPts, tAbove, &fAbove);
            } else {
                add += 1;
                SkASSERT(fTIndex - ~add <= fTs->count() + 1);
                tBelow = t(fTIndex - ~add);
                (*xyAtTFunc)(fWorkEdge.fPts, tBelow, &fBelow);
            }
        }
    #if DEBUG_ABOVE_BELOW
        fTAbove = tAbove;
        fTBelow = tBelow;
    #endif
    }

    bool done(SkScalar bottom) const {
        return fDone || fYBottom >= bottom;
    }

    void fixBelow() {
        if (fFixBelow) {
            switch (fWorkEdge.verb()) {
                case SkPath::kLine_Verb:
                    LineXYAtT(fWorkEdge.fPts, nextT(), &fBelow);
                    break;
                case SkPath::kQuad_Verb:
                    QuadXYAtT(fWorkEdge.fPts, nextT(), &fBelow);
                    break;
                case SkPath::kCubic_Verb:
                    CubicXYAtT(fWorkEdge.fPts, nextT(), &fBelow);
                    break;
                default:
                    SkASSERT(0);
            }
            fFixBelow = false;
        }
    }

    void init(const InEdge* edge) {
        fWorkEdge.init(edge);
        initT();
        fBelow.fY = SK_ScalarMin;
        fDone = false;
        fYBottom = SK_ScalarMin;
    }

    void initT() {
        const Intercepts& intercepts = fWorkEdge.fEdge->fIntercepts.front();
        SkASSERT(fWorkEdge.verbIndex() <= fWorkEdge.fEdge->fIntercepts.count());
        const Intercepts* interceptPtr = &intercepts + fWorkEdge.verbIndex();
        fTs = &interceptPtr->fTs;
        fExplicitTs = interceptPtr->fExplicit;
  //  the above is conceptually the same as
  //    fTs = &fWorkEdge.fEdge->fIntercepts[fWorkEdge.verbIndex()].fTs;
  //  but templated arrays don't allow returning a pointer to the end() element
        fTIndex = 0;
    }

    // OPTIMIZATION: record if two edges are coincident when the are intersected
    // It's unclear how to do this -- seems more complicated than recording the
    // t values, since the same t values could exist intersecting non-coincident
    // edges.
    bool isCoincidentWith(const ActiveEdge* edge, SkScalar y) const {
        if (fAbove != edge->fAbove || fBelow != edge->fBelow) {
            return false;
        }
        uint8_t verb = fDone ? fWorkEdge.lastVerb() : fWorkEdge.verb();
        uint8_t edgeVerb = edge->fDone ? edge->fWorkEdge.lastVerb() 
                : edge->fWorkEdge.verb();
        if (verb != edgeVerb) {
            return false;
        }
        switch (verb) {
            case SkPath::kLine_Verb: {
                return true;
            }
            default:
                // FIXME: add support for all curve types
                SkASSERT(0);
        }
        return false;
    }

    // The shortest close call edge should be moved into a position where
    // it contributes if the winding is transitioning to or from zero.
    bool swapClose(const ActiveEdge* next, int prev, int wind, int mask) const {
#if DEBUG_ADJUST_COINCIDENT
        SkDebugf("%s edge=%d (%g) next=%d (%g) prev=%d wind=%d nextWind=%d\n",
                __FUNCTION__, ID(), fBelow.fY, next->ID(), next->fBelow.fY,
                prev, wind, wind + next->fWorkEdge.winding());
#endif
        if ((prev & mask) == 0 || (wind & mask) == 0) {
            return next->fBelow.fY < fBelow.fY;
        }
        int nextWinding = wind + next->fWorkEdge.winding();
        if ((nextWinding & mask) == 0) {
            return fBelow.fY < next->fBelow.fY;
        }
        return false;
    }

    bool swapCoincident(const ActiveEdge* edge, SkScalar bottom) const {
        if (fBelow.fY >= bottom || fDone || edge->fDone) {
            return false;
        }
        ActiveEdge thisWork = *this;
        ActiveEdge edgeWork = *edge;
        while ((thisWork.advanceT() || thisWork.advance())
                && (edgeWork.advanceT() || edgeWork.advance())) {
            thisWork.calcLeft();
            edgeWork.calcLeft();
            if (thisWork < edgeWork) {
                return false;
            }
            if (edgeWork < thisWork) {
                return true;
            }
        }
        return false;
    }

    bool tooCloseToCall(const ActiveEdge* edge) const {
        int ulps;
    // FIXME: the first variation works better (or at least causes fewer tests
    // to fail than the second, although the second's logic better matches the
    // current sort criteria. Need to track down the cause of the crash, and
    // see if the second case isn't somehow buggy.
#if 01
        // FIXME: don't compare points for equality
        // OPTIMIZATION: refactor to make one call to UlpsDiff
        if (edge->fAbove.fY - fAbove.fY > fBelow.fY - edge->fAbove.fY
                && fBelow.fY < edge->fBelow.fY
                || fAbove.fY - edge->fAbove.fY < edge->fBelow.fY - fAbove.fY
                && edge->fBelow.fY < fBelow.fY) {
            const SkPoint& check = edge->fBelow.fY <= fBelow.fY
                    && fBelow != edge->fBelow ? edge->fBelow :
                    edge->fAbove;
            ulps = UlpsDiff((check.fY - fAbove.fY) * (fBelow.fX - fAbove.fX),
                    (fBelow.fY - fAbove.fY) * (check.fX - fAbove.fX));
        } else {
            const SkPoint& check = fBelow.fY <= edge->fBelow.fY 
                    && fBelow != edge->fBelow ? fBelow : fAbove;
            ulps = UlpsDiff((edge->fBelow.fY - edge->fAbove.fY)
                    * (check.fX - edge->fAbove.fX),
                    (check.fY - edge->fAbove.fY)
                    * (edge->fBelow.fX - edge->fAbove.fX));
        }
#else
        double t1, t2, b1, b2;
        if (edge->fAbove.fY < fAbove.fY) {
            t1 = (edge->fBelow.fY - edge->fAbove.fY) * (fAbove.fX - edge->fAbove.fX);
            t2 = (fAbove.fY - edge->fAbove.fY) * (edge->fBelow.fX - edge->fAbove.fX);
        } else if (edge->fAbove.fY > fAbove.fY) {
            t1 = (fBelow.fY - fAbove.fY) * (fAbove.fX - edge->fAbove.fX);
            t2 = (fAbove.fY - edge->fAbove.fY) * (fBelow.fX - fAbove.fX);
        } else {
            t1 = fAbove.fX;
            t2 = edge->fAbove.fX;
        }
        if (edge->fBelow.fY > fBelow.fY) {
            b1 = (edge->fBelow.fY - edge->fAbove.fY) * (fBelow.fX - edge->fBelow.fX);
            b2 = (fBelow.fY - edge->fBelow.fY) * (edge->fBelow.fX - edge->fAbove.fX);
        } else if (edge->fBelow.fY < fBelow.fY) {
            b1 = (fBelow.fY - fAbove.fY) * (fBelow.fX - edge->fBelow.fX);
            b2 = (fBelow.fY - edge->fBelow.fY) * (fBelow.fX - fAbove.fX);
        } else {
            b1 = fBelow.fX;
            b2 = edge->fBelow.fX;
        }
        if (fabs(t1 - t2) > fabs(b1 - b2)) {
            ulps = UlpsDiff(t1, t2);
        } else {
            ulps = UlpsDiff(b1, b2);
        }
#if DEBUG_ADJUST_COINCIDENT
        SkDebugf("%s this=%d edge=%d ulps=%d\n", __FUNCTION__, ID(), edge->ID(),
                ulps);
#endif
#endif
        return ulps >= 0 && ulps <= 32;
    }
    
    double nextT() const {
        SkASSERT(fTIndex <= fTs->count() - fExplicitTs);
        return t(fTIndex + 1);
    }

    double t() const {
        return t(fTIndex);
    }

    double t(int tIndex) const {
        if (fExplicitTs) {
            SkASSERT(tIndex < fTs->count());
            return (*fTs)[tIndex];
        } 
        if (tIndex == 0) {
            return 0;
        }
        if (tIndex > fTs->count()) {
            return 1;
        }
        return (*fTs)[tIndex - 1];
    }

    // FIXME: debugging only
    int ID() const {
        return fWorkEdge.fEdge->fID;
    }

public:
    WorkEdge fWorkEdge;
    const SkTDArray<double>* fTs;
    SkPoint fAbove;
    SkPoint fBelow;
#if DEBUG_ABOVE_BELOW
    double fTAbove;
    double fTBelow;
#endif
    SkScalar fYBottom;
    int fCoincident;
    int fTIndex;
    bool fSkip; // OPTIMIZATION: use bitfields?
    bool fCloseCall;
    bool fDone;
    bool fFixBelow;
    bool fExplicitTs;
};

static void addToActive(SkTDArray<ActiveEdge>& activeEdges, const InEdge* edge) {
    size_t count = activeEdges.count();
    for (size_t index = 0; index < count; ++index) {
        if (edge == activeEdges[index].fWorkEdge.fEdge) {
            return;
        }
    }
    ActiveEdge* active = activeEdges.append();
    active->init(edge);
}

// Find any intersections in the range of active edges. A pair of edges, on
// either side of another edge, may change the winding contribution for part of
// the edge. 
// Keep horizontal edges just for
// the purpose of computing when edges change their winding contribution, since
// this is essentially computing the horizontal intersection. 
static void addBottomT(InEdge** currentPtr, InEdge** lastPtr,
        HorizontalEdge** horizontal) {
    InEdge** testPtr = currentPtr - 1;
    HorizontalEdge* horzEdge = *horizontal;
    SkScalar left = horzEdge->fLeft;
    SkScalar bottom = horzEdge->fY;
    while (++testPtr != lastPtr) {
        InEdge* test = *testPtr;
        if (test->fBounds.fBottom <= bottom || test->fBounds.fRight <= left) {
            continue;
        }
        WorkEdge wt;
        wt.init(test);
        do {
            HorizontalEdge** sorted = horizontal;
            horzEdge = *sorted;
            do {
                double wtTs[4];
                int pts;
                uint8_t verb = wt.verb();
                switch (verb) {
                    case SkPath::kLine_Verb:
                        pts = LineIntersect(wt.fPts, horzEdge->fLeft,
                                horzEdge->fRight, horzEdge->fY, wtTs);
                        break;
                    case SkPath::kQuad_Verb:
                        pts = QuadIntersect(wt.fPts, horzEdge->fLeft,
                                horzEdge->fRight, horzEdge->fY, wtTs);
                        break;
                    case SkPath::kCubic_Verb:
                        pts = CubicIntersect(wt.fPts, horzEdge->fLeft,
                                horzEdge->fRight, horzEdge->fY, wtTs);
                        break;
                }
                if (pts) {
#if DEBUG_ADD_BOTTOM_TS
                    for (int x = 0; x < pts; ++x) {
                        SkDebugf("%s y=%g wtTs[0]=%g (%g,%g", __FUNCTION__,
                                horzEdge->fY, wtTs[x], wt.fPts[0].fX, wt.fPts[0].fY);
                        for (int y = 0; y < verb; ++y) {
                            SkDebugf(" %g,%g", wt.fPts[y + 1].fX, wt.fPts[y + 1].fY));
                        }
                        SkDebugf(")\n");
                    }
                    if (pts > verb) {
                        SkASSERT(0); // FIXME ? should this work?
                        SkDebugf("%s wtTs[1]=%g\n", __FUNCTION__, wtTs[1]);
                    }
#endif
                    test->add(wtTs, pts, wt.verbIndex());
                }
                horzEdge = *++sorted;
            } while (horzEdge->fY == bottom
                    && horzEdge->fLeft <= test->fBounds.fRight);
        } while (wt.advance());
    }
}

static void debugShowLineIntersection(int pts, const WorkEdge& wt,
        const WorkEdge& wn, const double wtTs[2], const double wnTs[2]) {
#if DEBUG_ADD_INTERSECTING_TS
    if (!pts) {
        return;
    }
    SkPoint wtOutPt, wnOutPt;
    LineXYAtT(wt.fPts, wtTs[0], &wtOutPt);
    LineXYAtT(wn.fPts, wnTs[0], &wnOutPt);
    SkDebugf("%s wtTs[0]=%g (%g,%g, %g,%g) (%g,%g) (%d,%d)\n",
            __FUNCTION__,
            wtTs[0], wt.fPts[0].fX, wt.fPts[0].fY,
            wt.fPts[1].fX, wt.fPts[1].fY, wtOutPt.fX, wtOutPt.fY,
            test->fID, next->fID);
    if (pts == 2) {
        SkDebugf("%s wtTs[1]=%g\n", __FUNCTION__, wtTs[1]);
    }
    SkDebugf("%s wnTs[0]=%g (%g,%g, %g,%g) (%g,%g) (%d,%d)\n",
            __FUNCTION__,
            wnTs[0], wn.fPts[0].fX, wn.fPts[0].fY,
            wn.fPts[1].fX, wn.fPts[1].fY, wnOutPt.fX, wnOutPt.fY,
            test->fID, next->fID);
    if (pts == 2) {
        SkDebugf("%s wnTs[1]=%g\n", __FUNCTION__, wnTs[1]);
    }
#endif
}

static void addIntersectingTs(InEdge** currentPtr, InEdge** lastPtr) {
    InEdge** testPtr = currentPtr - 1;
    // FIXME: lastPtr should be past the point of interest, so
    // test below should be  lastPtr - 2
    // that breaks testSimplifyTriangle22, so further investigation is needed
    while (++testPtr != lastPtr - 1) {
        InEdge* test = *testPtr;
        InEdge** nextPtr = testPtr;
        do {
            InEdge* next = *++nextPtr;
            // FIXME: this compares against the sentinel sometimes
            // OPTIMIZATION: this may never be needed since this gets called
            // in two passes now. Verify that double hits are appropriate.
            if (test->cached(next)) {
                continue;
            }
            if (!Bounds::Intersects(test->fBounds, next->fBounds)) {
                continue;
            }
            WorkEdge wt, wn;
            wt.init(test);
            wn.init(next);
            do {
                int pts;
                Intersections ts;
                bool swap = false;
                switch (wt.verb()) {
                    case SkPath::kLine_Verb:
                        switch (wn.verb()) {
                            case SkPath::kLine_Verb: {
                                pts = LineIntersect(wt.fPts, wn.fPts, ts);
                                debugShowLineIntersection(pts, wt, wn,
                                        ts.fT[0], ts.fT[1]);
                                break;
                            }
                            case SkPath::kQuad_Verb: {
                                swap = true;
                                pts = QuadLineIntersect(wn.fPts, wt.fPts, ts);
                                break;
                            }
                            case SkPath::kCubic_Verb: {
                                swap = true;
                                pts = CubicLineIntersect(wn.fPts, wt.fPts, ts);
                                break;
                            }
                            default:
                                SkASSERT(0);
                        }
                        break;
                    case SkPath::kQuad_Verb:
                        switch (wn.verb()) {
                            case SkPath::kLine_Verb: {
                                pts = QuadLineIntersect(wt.fPts, wn.fPts, ts);
                                break;
                            }
                            case SkPath::kQuad_Verb: {
                                pts = QuadIntersect(wt.fPts, wn.fPts, ts);
                                break;
                            }
                            case SkPath::kCubic_Verb: {
                                // FIXME: promote quad to cubic
                                pts = CubicIntersect(wt.fPts, wn.fPts, ts);
                                break;
                            }
                            default:
                                SkASSERT(0);
                        }
                        break;
                    case SkPath::kCubic_Verb:
                        switch (wn.verb()) {
                            case SkPath::kLine_Verb: {
                                pts = CubicLineIntersect(wt.fPts, wn.fPts, ts);
                                break;
                            }
                            case SkPath::kQuad_Verb: {
                                 // FIXME: promote quad to cubic
                                pts = CubicIntersect(wt.fPts, wn.fPts, ts);
                                break;
                            }
                            case SkPath::kCubic_Verb: {
                                pts = CubicIntersect(wt.fPts, wn.fPts, ts);
                                break;
                            }
                            default:
                                SkASSERT(0);
                        }
                        break;
                    default:
                        SkASSERT(0);
                }
                test->add(ts.fT[swap], pts, wt.verbIndex());
                next->add(ts.fT[!swap], pts, wn.verbIndex());
            } while (wt.bottom() <= wn.bottom() ? wt.advance() : wn.advance());
        } while (nextPtr != lastPtr);
    }
}

static InEdge** advanceEdges(SkTDArray<ActiveEdge>* activeEdges,
        InEdge** currentPtr, InEdge** lastPtr,  SkScalar y) {
    InEdge** testPtr = currentPtr - 1;
    while (++testPtr != lastPtr) {
        if ((*testPtr)->fBounds.fBottom > y) {
            continue;
        }
        if (activeEdges) {
            InEdge* test = *testPtr;
            ActiveEdge* activePtr = activeEdges->begin() - 1;
            ActiveEdge* lastActive = activeEdges->end();
            while (++activePtr != lastActive) {
                if (activePtr->fWorkEdge.fEdge == test) {
                    activeEdges->remove(activePtr - activeEdges->begin());
                    break;
                }
            }
        }
        if (testPtr == currentPtr) {
            ++currentPtr;
        }
    }
    return currentPtr;
}

// OPTIMIZE: inline?
static HorizontalEdge** advanceHorizontal(HorizontalEdge** edge, SkScalar y) {
    while ((*edge)->fY < y) {
        ++edge;
    }
    return edge;
}

// compute bottom taking into account any intersected edges
static SkScalar computeInterceptBottom(SkTDArray<ActiveEdge>& activeEdges,
        SkScalar y, SkScalar bottom) {
    ActiveEdge* activePtr = activeEdges.begin() - 1;
    ActiveEdge* lastActive = activeEdges.end();
    while (++activePtr != lastActive) {
        const InEdge* test = activePtr->fWorkEdge.fEdge;
        if (!test->fContainsIntercepts) {
            continue;
        }
        WorkEdge wt;
        wt.init(test);
        do {
            const Intercepts& intercepts = test->fIntercepts[wt.verbIndex()];
            if (intercepts.fTopIntercepts > 1) {
                SkScalar yTop = wt.fPts[0].fY;
                if (yTop > y && bottom > yTop) {
                    bottom = yTop;
                }
            }
            if (intercepts.fBottomIntercepts > 1) {
                SkScalar yBottom = wt.fPts[wt.verb()].fY;
                if (yBottom > y && bottom > yBottom) {
                    bottom = yBottom;
                }
            }
            const SkTDArray<double>& fTs = intercepts.fTs;
            size_t count = fTs.count();
            for (size_t index = 0; index < count; ++index) {
                SkScalar yIntercept;
                switch (wt.verb()) {
                    case SkPath::kLine_Verb: {
                        yIntercept = LineYAtT(wt.fPts, fTs[index]);
                        break;
                    }
                    case SkPath::kQuad_Verb: {
                        yIntercept = QuadYAtT(wt.fPts, fTs[index]);
                        break;
                    }
                    case SkPath::kCubic_Verb: {
                        yIntercept = CubicYAtT(wt.fPts, fTs[index]);
                        break;
                    }
                    default:
                        SkASSERT(0); // should never get here
                }
                if (yIntercept > y && bottom > yIntercept) {
                    bottom = yIntercept;
                }
            }
        } while (wt.advance());
    }
#if DEBUG_BOTTOM
    SkDebugf("%s bottom=%1.9g\n", __FUNCTION__, bottom);
#endif
    return bottom;
}

static SkScalar findBottom(InEdge** currentPtr, 
        InEdge** edgeListEnd, SkTDArray<ActiveEdge>* activeEdges, SkScalar y,
        bool asFill, InEdge**& testPtr) {
    InEdge* current = *currentPtr;
    SkScalar bottom = current->fBounds.fBottom;

    // find the list of edges that cross y
    InEdge* test = *testPtr;
    while (testPtr != edgeListEnd) {
        SkScalar testTop = test->fBounds.fTop;
        if (bottom <= testTop) {
            break;
        }
        SkScalar testBottom = test->fBounds.fBottom;
        // OPTIMIZATION: Shortening the bottom is only interesting when filling
        // and when the edge is to the left of a longer edge. If it's a framing
        // edge, or part of the right, it won't effect the longer edges.
        if (testTop > y) {
            bottom = testTop;
            break;
        } 
        if (y < testBottom) {
            if (bottom > testBottom) {
                bottom = testBottom;
            }
            if (activeEdges) {
                addToActive(*activeEdges, test);
            }
        }
        test = *++testPtr;
    }
#if DEBUG_BOTTOM
    SkDebugf("%s %d bottom=%1.9g\n", __FUNCTION__, activeEdges ? 2 : 1, bottom);
#endif
    return bottom;
}

static void makeEdgeList(SkTArray<InEdge>& edges, InEdge& edgeSentinel,
        SkTDArray<InEdge*>& edgeList) {
    size_t edgeCount = edges.count();
    if (edgeCount == 0) {
        return;
    }
    for (size_t index = 0; index < edgeCount; ++index) {
        *edgeList.append() = &edges[index];
    }
    edgeSentinel.fBounds.set(SK_ScalarMax, SK_ScalarMax, SK_ScalarMax, SK_ScalarMax);
    *edgeList.append() = &edgeSentinel;
    QSort<InEdge>(edgeList.begin(), edgeList.end() - 1);
}

static void makeHorizontalList(SkTDArray<HorizontalEdge>& edges,
        HorizontalEdge& edgeSentinel, SkTDArray<HorizontalEdge*>& edgeList) {
    size_t edgeCount = edges.count();
    if (edgeCount == 0) {
        return;
    }
    for (size_t index = 0; index < edgeCount; ++index) {
        *edgeList.append() = &edges[index];
    }
    edgeSentinel.fLeft = edgeSentinel.fRight = edgeSentinel.fY = SK_ScalarMax;
    *edgeList.append() = &edgeSentinel;
    QSort<HorizontalEdge>(edgeList.begin(), edgeList.end() - 1);
}

static void skipCoincidence(int lastWinding, int winding, int windingMask,
            ActiveEdge* activePtr, ActiveEdge* firstCoincident) {
    if (((lastWinding & windingMask) == 0) ^ (winding & windingMask) != 0) {
        return;
    } 
    // FIXME: ? shouldn't this be if (lastWinding & windingMask) ?
    if (lastWinding) {
#if DEBUG_ADJUST_COINCIDENT
        SkDebugf("%s edge=%d 1 set skip=false\n", __FUNCTION__, activePtr->ID());
#endif
        activePtr->fSkip = false;
    } else {
#if DEBUG_ADJUST_COINCIDENT
        SkDebugf("%s edge=%d 2 set skip=false\n", __FUNCTION__, firstCoincident->ID());
#endif
        firstCoincident->fSkip = false;
    }
}

static void sortHorizontal(SkTDArray<ActiveEdge>& activeEdges,
        SkTDArray<ActiveEdge*>& edgeList, SkScalar y) {
    size_t edgeCount = activeEdges.count();
    if (edgeCount == 0) {
        return;
    }
#if DEBUG_SORT_HORIZONTAL
    const int tab = 3; // FIXME: debugging only
    SkDebugf("%s y=%1.9g\n", __FUNCTION__, y);
#endif
    size_t index;
    for (index = 0; index < edgeCount; ++index) {
        ActiveEdge& activeEdge = activeEdges[index];
        do {
            activeEdge.calcLeft(y);
            // skip segments that don't span y
            if (activeEdge.fAbove != activeEdge.fBelow) {
                break;
            }
            if (activeEdge.fDone) {
#if DEBUG_SORT_HORIZONTAL
                SkDebugf("%*s edge=%d done\n", tab, "", activeEdge.ID());
#endif
                goto nextEdge;
            }
#if DEBUG_SORT_HORIZONTAL
            SkDebugf("%*s edge=%d above==below\n", tab, "", activeEdge.ID());
#endif
        } while (activeEdge.advanceT() || activeEdge.advance());
#if DEBUG_SORT_HORIZONTAL
        SkDebugf("%*s edge=%d above=(%1.9g,%1.9g) (%1.9g) below=(%1.9g,%1.9g)"
                " (%1.9g)\n", tab, "", activeEdge.ID(),
                activeEdge.fAbove.fX, activeEdge.fAbove.fY, activeEdge.fTAbove,
                activeEdge.fBelow.fX, activeEdge.fBelow.fY, activeEdge.fTBelow);
#endif
        activeEdge.fSkip = activeEdge.fCloseCall = activeEdge.fFixBelow = false;
        *edgeList.append() = &activeEdge;
nextEdge:
        ;
    }
    QSort<ActiveEdge>(edgeList.begin(), edgeList.end() - 1);
}

// remove coincident edges
// OPTIMIZE: remove edges? This is tricky because the current logic expects
// the winding count to be maintained while skipping coincident edges. In
// addition to removing the coincident edges, the remaining edges would need
// to have a different winding value, possibly different per intercept span.
static SkScalar adjustCoincident(SkTDArray<ActiveEdge*>& edgeList,
        int windingMask, SkScalar y, SkScalar bottom, OutEdgeBuilder& outBuilder)
{
#if DEBUG_ADJUST_COINCIDENT
    SkDebugf("%s y=%1.9g bottom=%1.9g\n", __FUNCTION__, y, bottom);
#endif
    size_t edgeCount = edgeList.count();
    if (edgeCount == 0) {
        return bottom;
    }
    ActiveEdge* activePtr = edgeList[0];
    size_t index;
    bool foundCoincident = false;
    int firstIndex = 0;
    for (index = 1; index < edgeCount; ++index) {
        ActiveEdge* nextPtr = edgeList[index];
        bool closeCall = false;
        activePtr->fCoincident = firstIndex;
        if (activePtr->isCoincidentWith(nextPtr, y)
                || (closeCall = activePtr->tooCloseToCall(nextPtr))) {
            activePtr->fSkip = nextPtr->fSkip = foundCoincident = true;
            activePtr->fCloseCall = nextPtr->fCloseCall = closeCall;
        } else {
            firstIndex = index;
        }
        activePtr = nextPtr;
    }
    activePtr->fCoincident = firstIndex;
    if (!foundCoincident) {
        return bottom;
    }
    int winding = 0;
    activePtr = edgeList[0];
    for (index = 1; index < edgeCount; ++index) {
        int priorWinding = winding;
        winding += activePtr->fWorkEdge.winding();
        ActiveEdge* nextPtr = edgeList[index];
        if (activePtr->fCoincident == nextPtr->fCoincident) {
            // the coincident edges may not have been sorted above -- advance
            // the edges and resort if needed
            // OPTIMIZE: if sorting is done incrementally as new edges are added
            // and not all at once as is done here, fold this test into the
            // current less than test.
            if (activePtr->fCloseCall ? activePtr->swapClose(nextPtr,
                    priorWinding, winding, windingMask)
                    : activePtr->swapCoincident(nextPtr, bottom)) {
                winding -= activePtr->fWorkEdge.winding();
                SkTSwap<ActiveEdge*>(edgeList[index - 1], edgeList[index]);
                SkTSwap<ActiveEdge*>(activePtr, nextPtr);
                winding += activePtr->fWorkEdge.winding();
            }
        }
        activePtr = nextPtr;
    }
    int firstCoincidentWinding = 0;
    ActiveEdge* firstCoincident = NULL;
    winding = 0;
    activePtr = edgeList[0];
    for (index = 1; index < edgeCount; ++index) {
        int priorWinding = winding;
        winding += activePtr->fWorkEdge.winding();
        ActiveEdge* nextPtr = edgeList[index];
        if (activePtr->fCoincident == nextPtr->fCoincident) {
            if (!firstCoincident) {
                firstCoincident = activePtr;
                firstCoincidentWinding = priorWinding;
            }
            if (activePtr->fCloseCall) {
                // If one of the edges has already been added to out as a non
                // coincident edge, trim it back to the top of this span
                if (outBuilder.trimLine(y, activePtr->ID())) {
                    activePtr->addTatYAbove(y);
            #if DEBUG_ADJUST_COINCIDENT
                    SkDebugf("%s 1 edge=%d y=%1.9g (was fYBottom=%1.9g)\n",
                            __FUNCTION__, activePtr->ID(), y, activePtr->fYBottom);
            #endif
                    activePtr->fYBottom = y;
                }
                if (outBuilder.trimLine(y, nextPtr->ID())) {
                    nextPtr->addTatYAbove(y);
            #if DEBUG_ADJUST_COINCIDENT
                    SkDebugf("%s 2 edge=%d y=%1.9g (was fYBottom=%1.9g)\n",
                            __FUNCTION__, nextPtr->ID(), y, nextPtr->fYBottom);
            #endif
                    nextPtr->fYBottom = y;
                }
                // add missing t values so edges can be the same length
                SkScalar testY = activePtr->fBelow.fY;
                nextPtr->addTatYBelow(testY);
                if (bottom > testY && testY > y) {
            #if DEBUG_ADJUST_COINCIDENT
                    SkDebugf("%s 3 edge=%d bottom=%1.9g (was bottom=%1.9g)\n",
                            __FUNCTION__, activePtr->ID(), testY, bottom);
            #endif
                    bottom = testY;
                }
                testY = nextPtr->fBelow.fY;
                activePtr->addTatYBelow(testY);
                if (bottom > testY && testY > y) {
            #if DEBUG_ADJUST_COINCIDENT
                    SkDebugf("%s 4 edge=%d bottom=%1.9g (was bottom=%1.9g)\n",
                            __FUNCTION__, nextPtr->ID(), testY, bottom);
            #endif
                    bottom = testY;
                }
            }
        } else if (firstCoincident) {
            skipCoincidence(firstCoincidentWinding, winding, windingMask,
                    activePtr, firstCoincident);
            firstCoincident = NULL;
        }
        activePtr = nextPtr;
    }
    if (firstCoincident) {
        winding += activePtr->fWorkEdge.winding();
        skipCoincidence(firstCoincidentWinding, winding, windingMask, activePtr,
                firstCoincident);
    }
    // fix up the bottom for close call edges. OPTIMIZATION: maybe this could
    // be in the loop above, but moved here since loop above reads fBelow and
    // it felt unsafe to write it in that loop
    for (index = 0; index < edgeCount; ++index) {
        (edgeList[index])->fixBelow();
    }
    return bottom;
}

// stitch edge and t range that satisfies operation
static void stitchEdge(SkTDArray<ActiveEdge*>& edgeList, SkScalar y,
        SkScalar bottom, int windingMask, bool fill, OutEdgeBuilder& outBuilder) {
    int winding = 0;
    ActiveEdge** activeHandle = edgeList.begin() - 1;
    ActiveEdge** lastActive = edgeList.end();
    const int tab = 7; // FIXME: debugging only
    if (gShowDebugf) {
        SkDebugf("%s y=%1.9g bottom=%1.9g\n", __FUNCTION__, y, bottom);
    }
    while (++activeHandle != lastActive) {
        ActiveEdge* activePtr = *activeHandle;
        const WorkEdge& wt = activePtr->fWorkEdge;
        int lastWinding = winding;
        winding += wt.winding();
        if (gShowDebugf) {
            SkDebugf("%*s edge=%d lastWinding=%d winding=%d skip=%d close=%d"
#if DEBUG_ABOVE_BELOW
                    " above=%1.9g below=%1.9g"
#endif
                    "\n",
                    tab-4, "", activePtr->ID(), lastWinding,
                    winding, activePtr->fSkip, activePtr->fCloseCall
#if DEBUG_ABOVE_BELOW
                    , activePtr->fTAbove, activePtr->fTBelow
#endif
                    );
        }
        if (activePtr->done(bottom)) {
            if (gShowDebugf) {
                SkDebugf("%*s fDone=%d || fYBottom=%1.9g >= bottom\n", tab, "",
                        activePtr->fDone, activePtr->fYBottom);
            }
            continue;
        }
        int opener = (lastWinding & windingMask) == 0;
        bool closer = (winding & windingMask) == 0;
        SkASSERT(!opener | !closer);
        bool inWinding = opener | closer;
        SkPoint clippedPts[4];
        const SkPoint* clipped = NULL;
        uint8_t verb = wt.verb();
        bool moreToDo, aboveBottom;
        do {
            double currentT = activePtr->t();
            SkASSERT(currentT < 1);
            const SkPoint* points = wt.fPts;
            double nextT;
            do {
                nextT = activePtr->nextT();
                // FIXME: obtuse: want efficient way to say 
                // !currentT && currentT != 1 || !nextT && nextT != 1
                if (currentT * nextT != 0 || currentT + nextT != 1) {
                    // OPTIMIZATION: if !inWinding, we only need 
                    // clipped[1].fY
                    switch (verb) {
                        case SkPath::kLine_Verb:
                            LineSubDivide(points, currentT, nextT, clippedPts);
                            break;
                        case SkPath::kQuad_Verb:
                            QuadSubDivide(points, currentT, nextT, clippedPts);
                            break;
                        case SkPath::kCubic_Verb:
                            CubicSubDivide(points, currentT, nextT, clippedPts);
                            break;
                        default:
                            SkASSERT(0);
                            break;
                    }
                    clipped = clippedPts;
                } else {
                    clipped = points;
                }
                if (inWinding && !activePtr->fSkip && (fill ? clipped[0].fY
                        != clipped[verb].fY : clipped[0] != clipped[verb])) {
                    if (gShowDebugf) {
                        const char* verbStr[] = {"", "Line", "Quad", "Cubic"};
                        SkDebugf("%*s add%s %1.9g,%1.9g %1.9g,%1.9g edge=%d"
                                " v=%d t=(%1.9g,%1.9g)\n", tab, "",
                                verbStr[verb], clipped[0].fX, clipped[0].fY,
                                clipped[verb].fX, clipped[verb].fY,
                                activePtr->ID(),
                                activePtr->fWorkEdge.fVerb
                                - activePtr->fWorkEdge.fEdge->fVerbs.begin(),
                                currentT, nextT);
                    }
                    outBuilder.addCurve(clipped, (SkPath::Verb) verb,
                            activePtr->fWorkEdge.fEdge->fID,
                            activePtr->fCloseCall);
                } else {
                    if (gShowDebugf ) {
                        const char* verbStr[] = {"", "Line", "Quad", "Cubic"};
                        SkDebugf("%*s skip%s %1.9g,%1.9g %1.9g,%1.9g"
                                " edge=%d v=%d t=(%1.9g,%1.9g)\n", tab, "",
                                verbStr[verb], clipped[0].fX, clipped[0].fY,
                                clipped[verb].fX, clipped[verb].fY,
                                activePtr->ID(),
                                activePtr->fWorkEdge.fVerb
                                - activePtr->fWorkEdge.fEdge->fVerbs.begin(),
                                currentT, nextT);
                    }
                }
            // by advancing fAbove/fBelow, the next call to sortHorizontal
            // will use these values if they're still valid instead of
            // recomputing
                if (clipped[1].fY > activePtr->fBelow.fY
                        && bottom >= activePtr->fBelow.fY ) {
                    activePtr->fAbove = activePtr->fBelow;
                    activePtr->fBelow = clipped[1];
            #if DEBUG_ABOVE_BELOW
                    activePtr->fTAbove = activePtr->fTBelow;
                    activePtr->fTBelow = nextT;
            #endif
                }
                currentT = nextT;
                moreToDo = activePtr->advanceT();
                activePtr->fYBottom = clipped[verb].fY; // was activePtr->fCloseCall ? bottom : 

                // clearing the fSkip/fCloseCall bit here means that trailing edges
                // fall out of sync, if one edge is long and another is a series of short pieces
                // if fSkip/fCloseCall is set, need to recompute coincidence/too-close-to-call
                // after advancing
                // another approach would be to restrict bottom to smaller part of close call
                // maybe this is already happening with coincidence when intersection is computed,
                // and needs to be added to the close call computation as well
                // this is hard to do because that the bottom is important is not known when
                // the lines are intersected; only when the computation for edge sorting is done
                // does the need for new bottoms become apparent.
                // maybe this is good incentive to scrap the current sort and do an insertion
                // sort that can take this into consideration when the x value is computed

                // FIXME: initialized in sortHorizontal, cleared here as well so
                // that next edge is not skipped -- but should skipped edges ever
                // continue? (probably not)
                aboveBottom = clipped[verb].fY < bottom;
                if (clipped[0].fY != clipped[verb].fY) {
                    activePtr->fSkip = false;
                    activePtr->fCloseCall = false;
                    aboveBottom &= !activePtr->fCloseCall;
                } else {
                    if (activePtr->fSkip || activePtr->fCloseCall) {
                        if (gShowDebugf) SkDebugf("== %1.9g\n", clippedPts[0].fY);
                    }
                }
            } while (moreToDo & aboveBottom);
        } while ((moreToDo || activePtr->advance()) & aboveBottom);
    }
}

static void dumpEdgeList(const SkTDArray<InEdge*>& edgeList,
        const InEdge& edgeSentinel) {
#if DEBUG_DUMP
    InEdge** debugPtr = edgeList.begin();
    do {
        (*debugPtr++)->dump();
    } while (*debugPtr != &edgeSentinel);
#endif
}

void simplify(const SkPath& path, bool asFill, SkPath& simple) {
    // returns 1 for evenodd, -1 for winding, regardless of inverse-ness
    int windingMask = (path.getFillType() & 1) ? 1 : -1;
    simple.reset();
    simple.setFillType(SkPath::kEvenOdd_FillType);
    // turn path into list of edges increasing in y
    // if an edge is a quad or a cubic with a y extrema, note it, but leave it
    // unbroken. Once we have a list, sort it, then walk the list (walk edges
    // twice that have y extrema's on top)  and detect crossings -- look for raw
    // bounds that cross over, then tight bounds that cross
    SkTArray<InEdge> edges;
    SkTDArray<HorizontalEdge> horizontalEdges;
    InEdgeBuilder builder(path, asFill, edges, horizontalEdges);
    SkTDArray<InEdge*> edgeList;
    InEdge edgeSentinel;
    makeEdgeList(edges, edgeSentinel, edgeList);
    SkTDArray<HorizontalEdge*> horizontalList;
    HorizontalEdge horizontalSentinel;
    makeHorizontalList(horizontalEdges, horizontalSentinel, horizontalList);
    InEdge** currentPtr = edgeList.begin();
    if (!currentPtr) {
        return;
    }
    // find all intersections between edges
// beyond looking for horizontal intercepts, we need to know if any active edges
// intersect edges below 'bottom', but above the active edge segment.
// maybe it makes more sense to compute all intercepts before doing anything
// else, since the intercept list is long-lived, at least in the current design.
    SkScalar y = (*currentPtr)->fBounds.fTop;
    HorizontalEdge** currentHorizontal = horizontalList.begin();
    do {
        InEdge** lastPtr = currentPtr; // find the edge below the bottom of the first set
        SkScalar bottom = findBottom(currentPtr, edgeList.end(),
                NULL, y, asFill, lastPtr);
        if (lastPtr > currentPtr) {
            if (currentHorizontal) {
                if ((*currentHorizontal)->fY < SK_ScalarMax) {
                    addBottomT(currentPtr, lastPtr, currentHorizontal);
                }
                currentHorizontal = advanceHorizontal(currentHorizontal, bottom);
            }
            addIntersectingTs(currentPtr, lastPtr);
        }
        y = bottom;
        currentPtr = advanceEdges(NULL, currentPtr, lastPtr, y);
    } while (*currentPtr != &edgeSentinel);
    // if a quadratic or cubic now has an intermediate T value, see if the Ts
    // on either side cause the Y values to monotonically increase. If not, split
    // the curve at the new T.
    if (builder.containsCurves()) {
        currentPtr = edgeList.begin();
        SkTArray<InEdge> splits;
        do {
            (*currentPtr)->splitInflectionPts(splits, builder.nextID());
        } while (*++currentPtr != &edgeSentinel);
        if (splits.count()) {
            edges.pop_back(); // pop the sentinel
            for (int index = 0; index < splits.count(); ++index) {
                edges.push_back(splits[index]);
            }
            makeEdgeList(edges, edgeSentinel, edgeList);
        }
    }
    dumpEdgeList(edgeList, edgeSentinel);
    // walk the sorted edges from top to bottom, computing accumulated winding
    SkTDArray<ActiveEdge> activeEdges;
    OutEdgeBuilder outBuilder(asFill);
    currentPtr = edgeList.begin();
    y = (*currentPtr)->fBounds.fTop;
    do {
        InEdge** lastPtr = currentPtr; // find the edge below the bottom of the first set
        SkScalar bottom = findBottom(currentPtr, edgeList.end(),
                &activeEdges, y, asFill, lastPtr);
        if (lastPtr > currentPtr) {
            bottom = computeInterceptBottom(activeEdges, y, bottom);
            SkTDArray<ActiveEdge*> activeEdgeList;
            sortHorizontal(activeEdges, activeEdgeList, y);
            bottom = adjustCoincident(activeEdgeList, windingMask, y, bottom,
                outBuilder);
            stitchEdge(activeEdgeList, y, bottom, windingMask, asFill, outBuilder);
        }
        y = bottom;
        // OPTIMIZATION: as edges expire, InEdge allocations could be released
        currentPtr = advanceEdges(&activeEdges, currentPtr, lastPtr, y);
    } while (*currentPtr != &edgeSentinel);
    // assemble output path from string of pts, verbs
    outBuilder.bridge();
    outBuilder.assemble(simple);
}
