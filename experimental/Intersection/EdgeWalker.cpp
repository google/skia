
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "CurveIntersection.h"
#include "LineIntersection.h"
#include "SkPath.h"
#include "SkRect.h"
#include "SkTArray.h"
#include "SkTDArray.h"
#include "TSearch.h"

static bool gShowDebugf = true; // FIXME: remove once debugging is complete
static bool gShowPath = false;
static bool gDebugLessThan = true;

static int LineIntersect(const SkPoint a[2], const SkPoint b[2],
        double aRange[2], double bRange[2]) {
    _Line aLine = {{a[0].fX, a[0].fY}, {a[1].fX, a[1].fY}};
    _Line bLine = {{b[0].fX, b[0].fY}, {b[1].fX, b[1].fY}};
    return intersect(aLine, bLine, aRange, bRange);
}

static int LineIntersect(const SkPoint a[2], SkScalar y, double aRange[2]) {
    _Line aLine = {{a[0].fX, a[0].fY}, {a[1].fX, a[1].fY}};
    return horizontalIntersect(aLine, y, aRange);
}

static void LineXYAtT(const SkPoint a[2], double t, SkPoint* out) {
    _Line aLine = {{a[0].fX, a[0].fY}, {a[1].fX, a[1].fY}};
    double x, y;
    xy_at_t(aLine, t, x, y);
    out->fX = SkDoubleToScalar(x);
    out->fY = SkDoubleToScalar(y);
}

static SkScalar LineYAtT(const SkPoint a[2], double t) {
    _Line aLine = {{a[0].fX, a[0].fY}, {a[1].fX, a[1].fY}};
    double y;
    xy_at_t(aLine, t, *(double*) 0, y);
    return SkDoubleToScalar(y);
}

static void LineSubDivide(const SkPoint a[2], double startT, double endT,
        SkPoint sub[2]) {
    _Line aLine = {{a[0].fX, a[0].fY}, {a[1].fX, a[1].fY}};
    _Line dst;
    sub_divide(aLine, startT, endT, dst);
    sub[0].fX = SkDoubleToScalar(dst[0].x);
    sub[0].fY = SkDoubleToScalar(dst[0].y);
    sub[1].fX = SkDoubleToScalar(dst[1].x);
    sub[1].fY = SkDoubleToScalar(dst[1].y);
}


// functions
void contourBounds(const SkPath& path, SkTDArray<SkRect>& boundsArray);
void simplify(const SkPath& path, bool asFill, SkPath& simple);
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

struct OutEdge {
    bool operator<(const OutEdge& rh) const {
        const SkPoint& first = fPts[0];
        const SkPoint& rhFirst = rh.fPts[0];
        return first.fY == rhFirst.fY
                ? first.fX < rhFirst.fX
                : first.fY < rhFirst.fY;
    }
    
    SkPoint fPts[4];
    uint8_t fVerb; // FIXME: not read from everywhere
};

class OutEdgeBuilder {
public:
    OutEdgeBuilder(bool fill)
        : fFill(fill) {
        }

    void addLine(const SkPoint line[2]) {
        OutEdge& newEdge = fEdges.push_back();
        newEdge.fPts[0] = line[0];
        newEdge.fPts[1] = line[1];
        newEdge.fVerb = SkPath::kLine_Verb;
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
            SkPoint firstPt, lastLine[2];
            bool doMove = true;
            bool closed = false;
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
                switch (verb) {
                    case SkPath::kLine_Verb:
                        bool gap;
                        if (doMove) {
                            firstPt = *start;
                            simple.moveTo(start->fX, start->fY);
                            if (gShowDebugf) {
                                SkDebugf("%s moveTo (%g,%g)\n", __FUNCTION__,
                                        start->fX, start->fY);
                            }
                            lastLine[0] = *start;
                            lastLine[1] = *end;
                            doMove = false;
                            closed = false;
                            break;
                        }
                        gap = lastLine[1] != *start;
                        if (gap) {
                            SkASSERT(fFill && lastLine[1].fY == start->fY);
                            simple.lineTo(lastLine[1].fX, lastLine[1].fY);
                            if (gShowDebugf) {
                                SkDebugf("%s lineTo x (%g,%g)\n", __FUNCTION__,
                                        lastLine[1].fX, lastLine[1].fY);
                            }
                        }
                        if (gap || !extendLine(lastLine, *end)) {
                            SkASSERT(lastLine[1] == *start ||
                                    (fFill && lastLine[1].fY == start->fY));
                            simple.lineTo(start->fX, start->fY);
                            if (gShowDebugf) {
                                SkDebugf("%s lineTo (%g,%g)\n", __FUNCTION__,
                                        start->fX, start->fY);
                            }
                            lastLine[0] = *start;
                        }
                        lastLine[1] = *end;
                        if (firstPt == *end) {
                            simple.lineTo(end->fX, end->fY);
                            simple.close();
                            if (gShowDebugf) {
                                SkDebugf("%s close 1 (%g, %g)\n", __FUNCTION__,
                                        end->fX, end->fY);
                            }
                            closed = true;
                        }
                        break;
                    default:
                        // FIXME: add other curve types
                        ;
                }
                if (advance < 0) {
                    edgeIndex = fTops[listIndex];
                    fTops[listIndex] = 0;
                 } else {
                    edgeIndex = fBottoms[listIndex];
                    fBottoms[listIndex] = 0;
                }
                if (!edgeIndex) {
                    simple.lineTo(firstPt.fX, firstPt.fY);
                    simple.close();
                    if (gShowDebugf) {
                        SkDebugf("%s close 2 (%g,%g)\n", __FUNCTION__,
                            firstPt.fX, firstPt.fY);
                    }
                    break;
                }
                listIndex = abs(edgeIndex) - 1;
                if (edgeIndex < 0) {
                    fTops[listIndex] = 0;
                } else {
                    fBottoms[listIndex] = 0;
                }
                // if this and next edge go different directions 
                if (advance > 0 ^ edgeIndex < 0) {
                    advance = -advance;
                }
            } while (edgeIndex && !closed);
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
            if (gDebugLessThan) {
                SkDebugf("%s %d<%d (%g,%g) %s startPt1.fY < startPt2.fY\n", __FUNCTION__,
                        one, two, startPt1.fY, startPt2.fY,
                        startPt1.fY < startPt2.fY ? "true" : "false");
            }
            return startPt1.fY < startPt2.fY;
        }
        if (startPt1.fX != startPt2.fX) {
            if (gDebugLessThan) {
                SkDebugf("%s %d<%d (%g,%g) %s startPt1.fX < startPt2.fX\n", __FUNCTION__,
                        one, two, startPt1.fX, startPt2.fX,
                        startPt1.fX < startPt2.fX ? "true" : "false");
            }
            return startPt1.fX < startPt2.fX;
        }
        const SkPoint& endPt1 = oneEdge.fPts[oneIndex ^ oneEdge.fVerb];
        const SkPoint& endPt2 = twoEdge.fPts[twoIndex ^ twoEdge.fVerb];
        SkScalar dy1 = startPt1.fY - endPt1.fY;
        SkScalar dy2 = startPt2.fY - endPt2.fY;
        SkScalar dy1y2 = dy1 * dy2;
        if (dy1y2 < 0) { // different signs
            if (gDebugLessThan) {
                SkDebugf("%s %d<%d %s dy1 > 0\n", __FUNCTION__, one, two,
                        dy1 > 0 ? "true" : "false");
            }
            return dy1 > 0; // one < two if one goes up and two goes down
        }
        if (dy1y2 == 0) {
            if (gDebugLessThan) {
                SkDebugf("%s %d<%d %s endPt1.fX < endPt2.fX\n", __FUNCTION__,
                        one, two, endPt1.fX < endPt2.fX ? "true" : "false");
            }
            return endPt1.fX < endPt2.fX;
        } 
        SkScalar dx1y2 = (startPt1.fX - endPt1.fX) * dy2;
        SkScalar dx2y1 = (startPt2.fX - endPt2.fX) * dy1;
        if (gDebugLessThan) {
            SkDebugf("%s %d<%d %s dy2 < 0 ^ dx1y2 < dx2y1\n", __FUNCTION__,
                    one, two, dy2 < 0 ^ dx1y2 < dx2y1 ? "true" : "false");
        }
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
                SkASSERT(left.fPts[leftIndex < 0 ? 0 : left.fVerb].fY
                        == right.fPts[rightIndex < 0 ? 0 : right.fVerb].fY);
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
    }

protected:
    SkTArray<OutEdge> fEdges;
    SkTDArray<int> fTops;
    SkTDArray<int> fBottoms;
    bool fFill;
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

struct Intercepts {
    SkTDArray<double> fTs;
};

struct InEdge {
    bool operator<(const InEdge& rh) const {
        return fBounds.fTop == rh.fBounds.fTop
                ? fBounds.fLeft < rh.fBounds.fLeft
                : fBounds.fTop < rh.fBounds.fTop;
    }

    bool add(double* ts, size_t count, ptrdiff_t verbIndex) {
        // FIXME: in the pathological case where there is a ton of intercepts, binary search?
        bool foundIntercept = false;
        Intercepts& intercepts = fIntercepts[verbIndex];
        for (size_t index = 0; index < count; ++index) {
            double t = ts[index];
            if (t <= 0 || t >= 1) {
                continue;
            }
            foundIntercept = true;
            size_t tCount = intercepts.fTs.count();
            for (size_t idx2 = 0; idx2 < tCount; ++idx2) {
                if (t <= intercepts.fTs[idx2]) {
                    double delta = intercepts.fTs[idx2] - t;
                    if (delta > 0) {
                        *intercepts.fTs.insert(idx2) = t;
                    }
                    return foundIntercept;
                }
            }
            if (tCount == 0 || t > intercepts.fTs[tCount - 1]) {
                *intercepts.fTs.append() = t;
            }
        }
        return foundIntercept;
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

    void complete(signed char winding) {
        SkPoint* ptPtr = fPts.begin();
        SkPoint* ptLast = fPts.end();
        if (ptPtr == ptLast) {
            SkDebugf("empty edge\n");
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
        fIntercepts.push_back_n(fVerbs.count());
        if ((fWinding = winding) < 0) { // reverse verbs, pts, if bottom to top
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
        fContainsIntercepts = false;
    }

    // temporary data : move this to a separate struct?
    SkTDArray<const InEdge*> fCached; // list of edges already intercepted
    SkTArray<Intercepts> fIntercepts; // one per verb
    
    // persistent data
    SkTDArray<SkPoint> fPts;
    SkTDArray<uint8_t> fVerbs;
    Bounds fBounds;
    signed char fWinding;
    bool fContainsIntercepts;
};

class InEdgeBuilder {
public:

InEdgeBuilder(const SkPath& path, bool ignoreHorizontal, SkTArray<InEdge>& edges) 
    : fPath(path)
    , fCurrentEdge(NULL)
    , fEdges(edges)
    , fIgnoreHorizontal(ignoreHorizontal)
{
    walk();
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
        fCurrentEdge->complete(fWinding);
        fCurrentEdge = NULL;
        return true;
    }
    return false;
}

int direction(int count) {
    fPtCount = count;
    fIgnorableHorizontal = fIgnoreHorizontal && isHorizontal();
    if (fIgnorableHorizontal) {
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
    int winding;
    while ((fVerb = iter.next(fPts)) != SkPath::kDone_Verb) {
        switch (fVerb) {
            case SkPath::kMove_Verb:
                if (gShowPath) {
                    SkDebugf("path.moveTo(%g, %g);\n", fPts[0].fX, fPts[0].fY);
                }
                startEdge();
                continue;
            case SkPath::kLine_Verb:
                if (gShowPath) {
                    SkDebugf("path.lineTo(%g, %g);\n", fPts[1].fX, fPts[1].fY);
                }
                winding = direction(2);
                break;
            case SkPath::kQuad_Verb:
                if (gShowPath) {
                    SkDebugf("path.quadTo(%g, %g, %g, %g);\n",
                        fPts[1].fX, fPts[1].fY, fPts[2].fX, fPts[2].fY);
                }
                winding = direction(3);
                break;
            case SkPath::kCubic_Verb:
                if (gShowPath) {
                    SkDebugf("path.cubicTo(%g, %g, %g, %g);\n",
                        fPts[1].fX, fPts[1].fY, fPts[2].fX, fPts[2].fY,
                        fPts[3].fX, fPts[3].fY);
                }
                winding = direction(4);
                break;
            case SkPath::kClose_Verb:
                if (gShowPath) {
                    SkDebugf("path.close();\n");
                }
                SkASSERT(fCurrentEdge);
                complete();
                continue;
            default:
                SkDEBUGFAIL("bad verb");
                return;
        }
        if (fIgnorableHorizontal) {
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
    if (gShowPath) {
        SkDebugf("\n");
    }
}

private:
    const SkPath& fPath;
    InEdge* fCurrentEdge;
    SkTArray<InEdge>& fEdges;
    SkPoint fPts[4];
    SkPath::Verb fVerb;
    int fPtCount;
    int fPtOffset;
    int8_t fWinding;
    bool fIgnorableHorizontal;
    bool fIgnoreHorizontal;
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
struct ActiveEdge {
    // OPTIMIZATION: fold return statements into one
    bool operator<(const ActiveEdge& rh) const {
        if (rh.fAbove.fY - fAbove.fY > fBelow.fY - rh.fAbove.fY
                && fBelow.fY < rh.fBelow.fY
                || fAbove.fY - rh.fAbove.fY < rh.fBelow.fY - fAbove.fY
                && rh.fBelow.fY < fBelow.fY) {
            // FIXME: need to compute distance, not check for points equal
            const SkPoint& check = rh.fBelow.fY <= fBelow.fY
                    && fBelow != rh.fBelow ? rh.fBelow :
                    rh.fAbove;
            if (gDebugLessThan) {
                SkDebugf("%s < %c %cthis (%d){%1.2g,%1.2g %1.2g,%1.2g}"
                        " < rh (%d){%1.2g,%1.2g %1.2g,%1.2g}\n", __FUNCTION__,
                        rh.fBelow.fY <= fBelow.fY && fBelow != rh.fBelow ? 'B' : 'A',
                        (check.fY - fAbove.fY) * (fBelow.fX - fAbove.fX)
                        < (fBelow.fY - fAbove.fY) * (check.fX - fAbove.fX) 
                        ? ' ' : '!',
                        fIndex, fAbove.fX, fAbove.fY, fBelow.fX, fBelow.fY,
                        rh.fIndex, rh.fAbove.fX, rh.fAbove.fY,
                        rh.fBelow.fX, rh.fBelow.fY);
            }
            return (check.fY - fAbove.fY) * (fBelow.fX - fAbove.fX)
                    < (fBelow.fY - fAbove.fY) * (check.fX - fAbove.fX);
        }
        // FIXME: need to compute distance, not check for points equal
        const SkPoint& check = fBelow.fY <= rh.fBelow.fY 
                && fBelow != rh.fBelow ? fBelow : fAbove;
        if (gDebugLessThan) {
            SkDebugf("%s > %c  %cthis (%d){%1.2g,%1.2g %1.2g,%1.2g}"
                    " < rh (%d){%1.2g,%1.2g %1.2g,%1.2g}\n", __FUNCTION__,
                    fBelow.fY <= rh.fBelow.fY & fBelow != rh.fBelow ? 'B' : 'A',
                    (rh.fBelow.fY - rh.fAbove.fY) * (check.fX - rh.fAbove.fX)
                    < (check.fY - rh.fAbove.fY) * (rh.fBelow.fX - rh.fAbove.fX) 
                    ? ' ' : '!',
                    fIndex, fAbove.fX, fAbove.fY, fBelow.fX, fBelow.fY,
                    rh.fIndex, rh.fAbove.fX, rh.fAbove.fY,
                    rh.fBelow.fX, rh.fBelow.fY);
        }
        return (rh.fBelow.fY - rh.fAbove.fY) * (check.fX - rh.fAbove.fX)
                < (check.fY - rh.fAbove.fY) * (rh.fBelow.fX - rh.fAbove.fX);
    }

    bool advanceT() {
        SkASSERT(fTIndex <= fTs->count());
        return ++fTIndex <= fTs->count();
    }
    
    bool advance() {
    // FIXME: flip sense of next
        bool result = fWorkEdge.advance();
        fDone = !result;
        initT();
        return result;
    }
    
    void calcLeft(SkScalar y) {
        // OPTIMIZE: put a kDone_Verb at the end of the verb list?
        if (done(y))
            return; // nothing to do; use last
        calcLeft();
    }
    
    void calcLeft() {
        switch (fWorkEdge.verb()) {
            case SkPath::kLine_Verb: {
                // OPTIMIZATION: if fXAbove, fXBelow have already been computed
                //  for the fTIndex, don't do it again
                // For identical x, this lets us know which edge is first.
                // If both edges have T values < 1, check x at next T (fXBelow).
                int add = (fTIndex <= fTs->count()) - 1;
                double tAbove = t(fTIndex + add);
                // OPTIMIZATION: may not need Y
                LineXYAtT(fWorkEdge.fPts, tAbove, &fAbove);
                double tBelow = t(fTIndex - ~add);
                LineXYAtT(fWorkEdge.fPts, tBelow, &fBelow);
                break;
            }
            default:
                // FIXME: add support for all curve types
                SkASSERT(0);
        }
    }

    bool done(SkScalar y) {
        return fDone || fYBottom > y;
    }
    
    void init(const InEdge* edge) {
        fWorkEdge.init(edge);
        initT();
        fDone = false;
        fYBottom = SK_ScalarMin;
    }
    
    void initT() {
        const Intercepts& intercepts = fWorkEdge.fEdge->fIntercepts.front();
        SkASSERT(fWorkEdge.verbIndex() <= fWorkEdge.fEdge->fIntercepts.count());
        const Intercepts* interceptPtr = &intercepts + fWorkEdge.verbIndex();
        fTs = &interceptPtr->fTs; 
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
        if (fAbove.fX != edge->fAbove.fX || fBelow.fX != edge->fBelow.fX) {
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
                int offset = fDone ? -1 : 1;
                int edgeOffset = edge->fDone ? -1 : 1;
                const SkPoint* pts = fWorkEdge.fPts;
                const SkPoint* edgePts = edge->fWorkEdge.fPts;
                return (pts->fX - pts[offset].fX)
                        * (edgePts->fY - edgePts[edgeOffset].fY)
                        == (pts->fY - pts[offset].fY)
                        * (edgePts->fX - edgePts[edgeOffset].fX);
            }
            default:
                // FIXME: add support for all curve types
                SkASSERT(0);
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

    double nextT() {
        SkASSERT(fTIndex <= fTs->count());
        return t(fTIndex + 1);
    }

    double t() const {
        if (fTIndex == 0) {
            return 0;
        }
        if (fTIndex > fTs->count()) {
            return 1;
        }
        return (*fTs)[fTIndex - 1];
    }

    double t(int tIndex) const {
        if (tIndex == 0) {
            return 0;
        }
        if (tIndex > fTs->count()) {
            return 1;
        }
        return (*fTs)[tIndex - 1];
    }

    WorkEdge fWorkEdge;
    const SkTDArray<double>* fTs;
    SkPoint fAbove;
    SkPoint fBelow;
    SkScalar fYBottom;
    int fTIndex;
    bool fSkip;
    bool fDone;
    int fIndex; // REMOVE: debugging only
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

    // find any intersections in the range of active edges
static void addBottomT(InEdge** currentPtr, InEdge** lastPtr, SkScalar bottom) {
    InEdge** testPtr = currentPtr;
    InEdge* test = *testPtr;
    while (testPtr != lastPtr) {
        if (test->fBounds.fBottom > bottom) {
            WorkEdge wt;
            wt.init(test);
            do {
                // OPTIMIZATION: if bottom intersection does not change
                // the winding on either side of the split, don't intersect
                if (wt.verb() == SkPath::kLine_Verb) {
                    double wtTs[2];
                    int pts = LineIntersect(wt.fPts, bottom, wtTs);
                    if (pts) {
                        test->fContainsIntercepts |= test->add(wtTs, pts,
                                wt.verbIndex());
                    }
                } else {
                    // FIXME: add all curve types
                    SkASSERT(0);
                }
            } while (wt.advance());
        }
        test = *++testPtr;
    }
}

static void addIntersectingTs(InEdge** currentPtr, InEdge** lastPtr) {
    InEdge** testPtr = currentPtr - 1;
    while (++testPtr != lastPtr - 1) {
        InEdge* test = *testPtr;
        InEdge** nextPtr = testPtr;
        do {
            InEdge* next = *++nextPtr;
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
                if (wt.verb() == SkPath::kLine_Verb
                        && wn.verb() == SkPath::kLine_Verb) {
                    double wtTs[2], wnTs[2];
                    int pts = LineIntersect(wt.fPts, wn.fPts, wtTs, wnTs);
                    if (pts) {
                        test->fContainsIntercepts |= test->add(wtTs, pts,
                                wt.verbIndex());
                        next->fContainsIntercepts |= next->add(wnTs, pts,
                                wn.verbIndex());
                    }
                } else {
                    // FIXME: add all combinations of curve types
                    SkASSERT(0);
                }
            } while (wt.bottom() <= wn.bottom() ? wt.advance() : wn.advance());
        } while (nextPtr != lastPtr);
    }
}

static InEdge** advanceEdges(SkTDArray<ActiveEdge>& activeEdges,
        InEdge** currentPtr, InEdge** lastPtr,  SkScalar y) {
    InEdge** testPtr = currentPtr - 1;
    while (++testPtr != lastPtr) {
        if ((*testPtr)->fBounds.fBottom > y) {
            continue;
        }
        InEdge* test = *testPtr;
        ActiveEdge* activePtr = activeEdges.begin() - 1;
        ActiveEdge* lastActive = activeEdges.end();
        while (++activePtr != lastActive) {
            if (activePtr->fWorkEdge.fEdge == test) {
                activeEdges.remove(activePtr - activeEdges.begin());
                break;
            }
        }
        if (testPtr == currentPtr) {
            ++currentPtr;
        }
    }
    return currentPtr;
}

// compute bottom taking into account any intersected edges
static void computeInterceptBottom(SkTDArray<ActiveEdge>& activeEdges,
        SkScalar y, SkScalar& bottom) {
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
            const SkTDArray<double>& fTs = intercepts.fTs;
            size_t count = fTs.count();
            for (size_t index = 0; index < count; ++index) {
                if (wt.verb() == SkPath::kLine_Verb) {
                    SkScalar yIntercept = LineYAtT(wt.fPts, fTs[index]);
                    if (yIntercept > y && bottom > yIntercept) {
                        bottom = yIntercept;
                    }
                } else {
                    // FIXME: add all curve types
                    SkASSERT(0);
                }
            }
        } while (wt.advance());
    }
}

static SkScalar findBottom(InEdge** currentPtr, 
        InEdge** edgeListEnd, SkTDArray<ActiveEdge>& activeEdges, SkScalar y,
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
            addToActive(activeEdges, test);
        }
        test = *++testPtr;
    }
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


static void skipCoincidence(int lastWinding, int winding, int windingMask,
            ActiveEdge* activePtr, ActiveEdge* firstCoincident) {
    if (((lastWinding & windingMask) == 0) ^ (winding & windingMask) != 0) {
        return;
    } 
    if (lastWinding) {
        activePtr->fSkip = false;
    } else {
        firstCoincident->fSkip = false;
    }
}

static void sortHorizontal(SkTDArray<ActiveEdge>& activeEdges,
        SkTDArray<ActiveEdge*>& edgeList, int windingMask, SkScalar y,
        SkScalar bottom) {
    size_t edgeCount = activeEdges.count();
    if (edgeCount == 0) {
        return;
    }
    size_t index;
    for (index = 0; index < edgeCount; ++index) {
        ActiveEdge& activeEdge = activeEdges[index];
        activeEdge.calcLeft(y);
        activeEdge.fSkip = false;
        activeEdge.fIndex = index; // REMOVE: debugging only
        *edgeList.append() = &activeEdge;
    }
    QSort<ActiveEdge>(edgeList.begin(), edgeList.end() - 1);
    // remove coincident edges
    // OPTIMIZE: remove edges? This is tricky because the current logic expects
    // the winding count to be maintained while skipping coincident edges. In
    // addition to removing the coincident edges, the remaining edges would need
    // to have a different winding value, possibly different per intercept span.
    int lastWinding = 0;
    bool lastSkipped = false;
    ActiveEdge* activePtr = edgeList[0];
    ActiveEdge* firstCoincident = NULL;
    int winding = 0;
    for (index = 1; index < edgeCount; ++index) {
        winding += activePtr->fWorkEdge.winding();
        ActiveEdge* nextPtr = edgeList[index];
        if (activePtr->isCoincidentWith(nextPtr, y)) {
            // the coincident edges may not have been sorted above -- advance
            // the edges and resort if needed
            // OPTIMIZE: if sorting is done incrementally as new edges are added
            // and not all at once as is done here, fold this test into the
            // current less than test.
            if (activePtr->swapCoincident(nextPtr, bottom)) {
                SkTSwap<ActiveEdge*>(edgeList[index - 1], edgeList[index]);
                SkTSwap<ActiveEdge*>(activePtr, nextPtr);
            }
            if (!firstCoincident) {
                firstCoincident = activePtr;
            }
            activePtr->fSkip = nextPtr->fSkip = lastSkipped = true;
        } else if (lastSkipped) {
            skipCoincidence(lastWinding, winding, windingMask, activePtr,
                    firstCoincident);
            lastSkipped = false;
            firstCoincident = NULL;
        }
        if (!lastSkipped) {
            lastWinding = winding;
        }
        activePtr = nextPtr;
    }
    if (lastSkipped) {
        winding += activePtr->fWorkEdge.winding();
        skipCoincidence(lastWinding, winding, windingMask, activePtr,
                firstCoincident);
    }
}

// stitch edge and t range that satisfies operation
static void stitchEdge(SkTDArray<ActiveEdge*>& edgeList, SkScalar y,
        SkScalar bottom, int windingMask, OutEdgeBuilder& outBuilder) {
    int winding = 0;
    ActiveEdge** activeHandle = edgeList.begin() - 1;
    ActiveEdge** lastActive = edgeList.end();
    if (gShowDebugf) {
        SkDebugf("%s y=%g bottom=%g\n", __FUNCTION__, y, bottom);
    }
    while (++activeHandle != lastActive) {
        ActiveEdge* activePtr = *activeHandle;
        const WorkEdge& wt = activePtr->fWorkEdge;
        int lastWinding = winding;
        winding += wt.winding();
        if (activePtr->done(y)) {
            // FIXME: if this is successful, rewrite done to take bottom as well
            if (activePtr->fDone) {
                continue;
            }
            if (activePtr->fYBottom >= bottom) {
                continue;
            }
            if (0) {
                SkDebugf("%s bot %g,%g\n", __FUNCTION__, activePtr->fYBottom, bottom);
            }
        }
        int opener = (lastWinding & windingMask) == 0;
        bool closer = (winding & windingMask) == 0;
        SkASSERT(!opener | !closer);
        bool inWinding = opener | closer;
        const SkPoint* clipped;
        uint8_t verb = wt.verb();
        bool moreToDo, aboveBottom;
        do {
            double currentT = activePtr->t();
            SkASSERT(currentT < 1);
            const SkPoint* points = wt.fPts;
            double nextT;
            do {
                nextT = activePtr->nextT();
                if (verb == SkPath::kLine_Verb) {
                    SkPoint clippedPts[2];
                    // FIXME: obtuse: want efficient way to say 
                    // !currentT && currentT != 1 || !nextT && nextT != 1
                    if (currentT * nextT != 0 || currentT + nextT != 1) {
                        // OPTIMIZATION: if !inWinding, we only need 
                        // clipped[1].fY
                        LineSubDivide(points, currentT, nextT, clippedPts);
                        clipped = clippedPts;
                    } else {
                        clipped = points;
                    }
                    if (inWinding && !activePtr->fSkip) {
                        if (gShowDebugf) {
                            SkDebugf("%s line %g,%g %g,%g\n", __FUNCTION__,
                                    clipped[0].fX, clipped[0].fY,
                                    clipped[1].fX, clipped[1].fY);
                        }
                        outBuilder.addLine(clipped);
                    }
                    activePtr->fSkip = false;
                } else {
                    // FIXME: add all curve types
                    SkASSERT(0);
                }
                currentT = nextT;
                moreToDo = activePtr->advanceT();
                activePtr->fYBottom = clipped[verb].fY;
                aboveBottom = activePtr->fYBottom < bottom;
            } while (moreToDo & aboveBottom);
        } while ((moreToDo || activePtr->advance()) & aboveBottom);
    }
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
    InEdgeBuilder builder(path, asFill, edges);
    SkTDArray<InEdge*> edgeList;
    InEdge edgeSentinel;
    makeEdgeList(edges, edgeSentinel, edgeList);
    InEdge** currentPtr = edgeList.begin();
    // walk the sorted edges from top to bottom, computing accumulated winding
    SkTDArray<ActiveEdge> activeEdges;
    OutEdgeBuilder outBuilder(asFill);
    SkScalar y = (*currentPtr)->fBounds.fTop;
    do {
        InEdge** lastPtr = currentPtr; // find the edge below the bottom of the first set
        SkScalar bottom = findBottom(currentPtr, edgeList.end(),
            activeEdges, y, asFill, lastPtr);
        if (lastPtr > currentPtr) {
            addBottomT(currentPtr, lastPtr, bottom);
            addIntersectingTs(currentPtr, lastPtr);
            computeInterceptBottom(activeEdges, y, bottom);
            SkTDArray<ActiveEdge*> activeEdgeList;
            sortHorizontal(activeEdges, activeEdgeList, windingMask, y, bottom);
            stitchEdge(activeEdgeList, y, bottom, windingMask, outBuilder);
        }
        y = bottom;
        currentPtr = advanceEdges(activeEdges, currentPtr, lastPtr, y);
    } while (*currentPtr != &edgeSentinel);
    // assemble output path from string of pts, verbs
    outBuilder.bridge();
    outBuilder.assemble(simple);
}
