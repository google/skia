
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

struct OutEdge {

    SkTDArray<SkPoint> fPts;
    SkTDArray<uint8_t> fVerbs;
};

class OutEdgeBuilder {
public:
    void addLine(SkPoint pts[2]) {
        ;
        OutEdge* edge;
        
        edge = fEdges.append();
        
        if (empty) {
            *edge->fPts.append() = pts[0];
        }
        *edge->fPts.append() = pts[1];
    }

    SkTArray<OutEdge> fEdges;
};

// Bounds, unlike Rect, does not consider a vertical line to be empty.
struct Bounds : public SkRect {
    static bool Intersects(const Bounds& a, const Bounds& b) {
        return a.fLeft <= b.fRight && b.fLeft <= a.fRight &&
                a.fTop <= b.fBottom && b.fTop <= a.fBottom;
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

    void add(double* ts, size_t count, int verbIndex) {
        Intercepts& intercepts = fIntercepts[verbIndex];
        // FIXME: in the pathological case where there is a ton of intercepts, binary search?
        for (size_t index = 0; index < count; ++index) {
            double t = ts[index];
            size_t tCount = intercepts.fTs.count();
            for (size_t idx2 = 0; idx2 < tCount; ++idx2) {
                if (t <= intercepts.fTs[idx2]) {
                    if (t < intercepts.fTs[idx2]) {
                        *intercepts.fTs.insert(idx2) = t;
                        break;
                    }
                }
            }
            if (tCount == 0 || t > intercepts.fTs[tCount - 1]) {
                *intercepts.fTs.append() = t;
            }
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
        fIntercepts.push_back_n(1);
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
    fCurrentEdge->fPts.append(fPtCount - fPtOffset, &fPts[fPtOffset]);
    fPtOffset = 1;
    *fCurrentEdge->fVerbs.append() = fVerb;
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
    fCurrentEdge = fEdges.push_back_n(1);
    fWinding = 0;
    fPtOffset = 0;
}

void walk() {
    SkPath::Iter iter(fPath, true);
    int winding = 0;
    while ((fVerb = iter.next(fPts)) != SkPath::kDone_Verb) {
        switch (fVerb) {
            case SkPath::kMove_Verb:
                winding = 0;
                startEdge();
                continue;
            case SkPath::kLine_Verb:
                winding = direction(2);
                break;
            case SkPath::kQuad_Verb:
                winding = direction(3);
                break;
            case SkPath::kCubic_Verb:
                winding = direction(4);
                break;
            case SkPath::kClose_Verb:
                if (fCurrentEdge->fVerbs.count()) {
                    fCurrentEdge->complete(fWinding);
                }
                continue;
            default:
                SkDEBUGFAIL("bad verb");
                return;
        }
        if (fIgnorableHorizontal) {
            continue;
        }
        if (fWinding + winding == 0) {
            // FIXME: if prior verb or this verb is a horizontal line, reverse
            // it instead of starting a new edge
            fCurrentEdge->complete(fWinding);
            startEdge();
        }
        fWinding = winding;
        addEdge();
    }
    fCurrentEdge->complete(fWinding);
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

    bool next() {
        SkASSERT(fVerb < fEdge->fVerbs.end());
        fPts += *fVerb++;
        return fVerb != fEdge->fVerbs.end();
    }

    SkPath::Verb verb() const {
        return (SkPath::Verb) *fVerb;
    }

    int verbIndex() const {
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
struct ActiveEdge {
    void init(const InEdge* edge) {
        fWorkEdge.init(edge);
        initT();
    }
    
    void initT() {
        fTs = &fWorkEdge.fEdge->fIntercepts[fWorkEdge.verbIndex()].fTs;
        fTIndex = 0;
    }

    bool nextT() {
        SkASSERT(fTIndex <= fTs->count());
        return ++fTIndex == fTs->count() + 1;
    }
    
    bool next() {
        bool result = fWorkEdge.next();
        initT();
        return result;
    }

    double t() {
        if (fTIndex == 0) {
            return 0;
        }
        if (fTIndex > fTs->count()) {
            return 1;
        }
        return (*fTs)[fTIndex - 1];
    }

    WorkEdge fWorkEdge;
    const SkTDArray<double>* fTs;
    int fTIndex;
};

static void addToActive(SkTDArray<ActiveEdge>& activeEdges, const InEdge* edge) {
    // FIXME: in the pathological case where there is a ton of intercepts, binary search?
    size_t count = activeEdges.count();
    for (size_t index = 0; index < count; ++index) {
        if (edge < activeEdges[index].fWorkEdge.fEdge) {
            ActiveEdge* active = activeEdges.insert(index);
            active->init(edge);
            return;
        }
        if (edge == activeEdges[index].fWorkEdge.fEdge) {
            return;
        }
    }
    ActiveEdge* active = activeEdges.append();
    active->init(edge);
}

void simplify(const SkPath& path, bool asFill, SkPath& simple) {
    // turn path into list of edges increasing in y
    // if an edge is a quad or a cubic with a y extrema, note it, but leave it unbroken
    // once we have a list, sort it, then walk the list (walk edges twice that have y extrema's on top)
    //  and detect crossings -- look for raw bounds that cross over, then tight bounds that cross
    SkTArray<InEdge> edges;
    InEdgeBuilder builder(path, asFill, edges);
    size_t edgeCount = edges.count();
    simple.reset();
    if (edgeCount == 0) {
        return;
    }
    // returns 1 for evenodd, -1 for winding, regardless of inverse-ness
    int windingMask = (path.getFillType() & 1) ? 1 : -1;
    SkTDArray<InEdge*> edgeList;
    for (size_t index = 0; index < edgeCount; ++index) {
        *edgeList.append() = &edges[index];
    }
    InEdge edgeSentinel;
    edgeSentinel.fBounds.set(SK_ScalarMax, SK_ScalarMax, SK_ScalarMax, SK_ScalarMax);
    *edgeList.append() = &edgeSentinel;
    ++edgeCount;
    QSort<InEdge>(edgeList.begin(), edgeCount);
    InEdge** currentPtr = edgeList.begin();
    InEdge* current = *currentPtr;
    SkScalar y = current->fBounds.fTop;
    SkScalar bottom = current->fBounds.fBottom;
    // walk the sorted edges from top to bottom, computing accumulated winding
    SkTDArray<ActiveEdge> activeEdges;
    OutEdgeBuilder outBuilder;
    do {
        // find the list of edges that cross y
        InEdge** lastPtr = currentPtr; // find the edge below the bottom of the first set
        InEdge* last = *lastPtr;
        while (lastPtr != edgeList.end()) {
            if (bottom <= last->fBounds.fTop) {
                break;
            }
            SkScalar lastTop = last->fBounds.fTop;
            // OPTIMIZATION: Shortening the bottom is only interesting when filling
            // and when the edge is to the left of a longer edge. If it's a framing
            // edge, or part of the right, it won't effect the longer edges.
            if (lastTop > y) {
                if (bottom > lastTop) {
                    bottom = lastTop;
                    break;
                }
            } else if (bottom > last->fBounds.fBottom) {
                bottom = last->fBounds.fBottom;
            }
            addToActive(activeEdges, last);
            last = *++lastPtr;
        }
        if (asFill && lastPtr - currentPtr <= 1) {
            SkDebugf("expect 2 or more edges\n");
            SkASSERT(0);
            return;
        }

        // find any intersections in the range of active edges
        InEdge** testPtr = currentPtr;
        InEdge* test = *testPtr;
        while (testPtr != lastPtr) {
            if (test->fBounds.fBottom > bottom) {
                WorkEdge wt;
                wt.init(test);
                do {
                    // FIXME: add all curve types
                    // OPTIMIZATION: if bottom intersection does not change
                    // the winding on either side of the split, don't intersect
                    if (wt.verb() == SkPath::kLine_Verb) {
                        double wtTs[2];
                        int pts = LineIntersect(wt.fPts, bottom, wtTs);
                        if (pts) {
                            test->add(wtTs, pts, wt.verbIndex());
                        }
                    }
                } while (wt.next());
            }
            test = *++testPtr;
        }
        testPtr = currentPtr;
        test = *testPtr;
        while (testPtr != lastPtr - 1) {
            InEdge* next = *++testPtr;
            if (!test->cached(next)
                    && Bounds::Intersects(test->fBounds, next->fBounds)) {
                WorkEdge wt, wn;
                wt.init(test);
                wn.init(next);
                do {
                    // FIXME: add all combinations of curve types
                    if (wt.verb() == SkPath::kLine_Verb
                            && wn.verb() == SkPath::kLine_Verb) {
                        double wtTs[2], wnTs[2];
                        int pts = LineIntersect(wt.fPts, wn.fPts, wtTs, wnTs);
                        if (pts) {
                            test->add(wtTs, pts, wt.verbIndex());
                            test->fContainsIntercepts = true;
                            next->add(wnTs, pts, wn.verbIndex());
                            next->fContainsIntercepts = true;
                        }
                    }
                } while (wt.bottom() <= wn.bottom() ? wt.next() : wn.next());
            }
            test = next;
        }

        // compute bottom taking into account any intersected edges
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
                // FIXME: add all curve types
                const Intercepts& intercepts = test->fIntercepts[wt.verbIndex()];
                const SkTDArray<double>& fTs = intercepts.fTs;
                size_t count = fTs.count();
                for (size_t index = 0; index < count; ++index) {
                    if (wt.verb() == SkPath::kLine_Verb) {
                        SkScalar yIntercept = LineYAtT(wt.fPts, fTs[index]);
                        if (bottom > yIntercept) {
                            bottom = yIntercept;
                        }
                    }
                }
            } while (wt.next());
        }

        // stitch edge and t range that satisfies operation
        int winding = 0;
        activePtr = activeEdges.begin() - 1;
        lastActive = activeEdges.end();
        SkDebugf("%s y=%g bottom=%g\n", __FUNCTION__, y, bottom);
        while (++activePtr != lastActive) {
            const WorkEdge& wt = activePtr->fWorkEdge;
            int lastWinding = winding;
            winding += wt.winding();
            if (!(lastWinding & windingMask) && !(winding & windingMask)) {
                continue;
            }
            do {
                double currentT = activePtr->t();
                const SkPoint* points = wt.fPts;
                bool last;
                do {
                    last = activePtr->nextT();
                    double nextT = activePtr->t();
                    // FIXME: add all combinations of curve types
                    if (wt.verb() == SkPath::kLine_Verb) {
                        SkPoint clippedPts[2];
                        const SkPoint* clipped;
                        if (currentT * nextT != 0 || currentT + nextT != 1) {
                            LineSubDivide(points, currentT, nextT, clippedPts);
                            clipped = clippedPts;
                        } else {
                            clipped = points;
                        }
                        SkDebugf("%s line %g,%g %g,%g\n", __FUNCTION__,
                                clipped[0].fX, clipped[0].fY,
                                clipped[1].fX, clipped[1].fY);
                        outBuilder->addLine(clipped);
                        if (clipped[1].fY >= bottom) {
                            goto nextEdge;
                        }
                    }
                    currentT = nextT;
                } while (!last);
            } while (activePtr->next());
    nextEdge:
            ;
        }

        y = bottom;
        while ((*currentPtr)->fBounds.fBottom <= y) {
            ++currentPtr;
        }
    } while (*currentPtr != &edgeSentinel);

    // assemble output path from string of pts, verbs
    ;
}

void testSimplify();

void testSimplify() {
    SkPath path, out;
    path.setFillType(SkPath::kWinding_FillType);
    path.addRect(10, 10, 30, 30);
    path.addRect(20, 20, 40, 40);
    simplify(path, true, out);
    path = out;
    path.addRect(30, 10, 40, 20);
    path.addRect(10, 30, 20, 40);
    simplify(path, true, out);
    path = out;
    path.addRect(10, 10, 40, 40, SkPath::kCCW_Direction);
    simplify(path, true, out);
    if (!out.isEmpty()) {
        SkDebugf("expected empty\n");
    }
}
