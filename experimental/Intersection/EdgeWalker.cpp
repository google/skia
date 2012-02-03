
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "LineIntersection.h"
#include "SkPath.h"
#include "SkRect.h"
#include "SkTArray.h"
#include "SkTDArray.h"
#include "TSearch.h"

static int lineIntersect(const SkPoint a[2], const SkPoint b[2],
        double aRange[2], double bRange[2]) {
    _Line aLine = {{a[0].fX, a[0].fY}, {a[1].fX, a[1].fY}};
    _Line bLine = {{b[0].fX, b[0].fY}, {b[1].fX, b[1].fY}};
    return intersect(aLine, bLine, aRange, bRange);
}

static int lineIntersect(const SkPoint a[2], SkScalar y, double aRange[2]) {
    _Line aLine = {{a[0].fX, a[0].fY}, {a[1].fX, a[1].fY}};
    return horizontalIntersect(aLine, y, aRange);
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

// Bounds, unlike Rect, does not consider a vertical line to be empty.
struct Bounds : public SkRect {
    static bool Intersects(const Bounds& a, const Bounds& b) {
        return a.fLeft <= b.fRight && b.fLeft <= a.fRight &&
                a.fTop <= b.fBottom && b.fTop <= a.fBottom;
    }
};

struct Edge;

struct Intercepts {
    SkTDArray<double> fTs;
};

struct Edge {
    bool operator<(const Edge& rh) const {
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

    bool cached(const Edge* edge) {
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
        fWinding = winding;
    }

    // temporary data : move this to a separate struct?
    SkTDArray<const Edge*> fCached; // list of edges already intercepted
    SkTArray<Intercepts> fIntercepts; // one per verb
    

    // persistent data
    SkTDArray<SkPoint> fPts;
    SkTDArray<uint8_t> fVerbs;
    Bounds fBounds;
    signed char fWinding;
};

class EdgeBuilder {
public:

EdgeBuilder(const SkPath& path, bool ignoreHorizontal, SkTArray<Edge>& edges) 
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
            ? fPts[0].fX == fPts[last].fX ? 0 : fPts[0].fX > fPts[last].fX
            ? 1 : -1 : fPts[0].fY > fPts[last].fY ? 1 : -1;
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
    Edge* fCurrentEdge;
    SkTArray<Edge>& fEdges;
    SkPoint fPts[4];
    SkPath::Verb fVerb;
    int fPtCount;
    int fPtOffset;
    int8_t fWinding;
    bool fIgnorableHorizontal;
    bool fIgnoreHorizontal;
};

class WorkEdge {
public:
    WorkEdge(const Edge* edge) {
        fVerbStart = edge->fVerbs.begin();
        if ((fWinding = edge->fWinding) > 0) {
            fPts = edge->fPts.begin();
            fVerb = fVerbStart;
            fVerbEnd = edge->fVerbs.end();
        } else {
            fPts = edge->fPts.end();
            fVerb = edge->fVerbs.end();
            fVerbEnd = fVerbStart;
            next();
        }
    }

    SkScalar bottom() const {
        return fPts[fWinding > 0 ? verb() : 0].fY;
    }

    bool next() {
        if (fWinding > 0) {
            fPts += *fVerb;
            return ++fVerb != fVerbEnd;
        } else {
            if (fVerb == fVerbEnd) {
                return false;
            }
            fPts -= *--fVerb;
            return true; 
        }
    }

    const SkPoint* points() const {
        return fPts;
    }

    SkPath::Verb verb() const {
        return (SkPath::Verb) *fVerb;
    }

    int verbIndex() const {
        return fVerb - fVerbStart;
    }

protected:     
    const SkPoint* fPts;
    const uint8_t* fVerb;
    const uint8_t* fVerbEnd;
    const uint8_t* fVerbStart;
    int8_t fWinding;
};

struct ActiveEdge {
    void init(const Edge* test) {
        fEdge = test;
        if (!fEdge->fIntercepts.count()) {
            fBounds = test->fBounds;
            fPtStart = 0;
            fPtEnd = test->fPts.count();
            fVerbStart = 0;
            fVerbEnd = test->fVerbs.count();
            fTStart = 0;
            fTEnd = SK_Scalar1;
        } else {
            // FIXME: initialize from intercepts

        }
    }

    const Edge* fEdge;
    SkRect fBounds;
    int fPtStart;
    int fPtEnd;
    int fVerbStart;
    int fVerbEnd;
    SkScalar fTStart;
    SkScalar fTEnd;
};

void simplify(const SkPath& path, bool asFill, SkPath& simple) {
    // turn path into list of edges increasing in y
    // if an edge is a quad or a cubic with a y extrema, note it, but leave it unbroken
    // once we have a list, sort it, then walk the list (walk edges twice that have y extrema's on top)
    //  and detect crossings -- look for raw bounds that cross over, then tight bounds that cross
    SkTArray<Edge> edges;
    EdgeBuilder builder(path, asFill, edges);
    size_t edgeCount = edges.count();
    simple.reset();
    if (edgeCount == 0) {
        return;
    }
    // returns 1 for evenodd, -1 for winding, regardless of inverse-ness
    int windingMask = (path.getFillType() & 1) ? 1 : -1;
    SkTDArray<Edge*> edgeList;
    for (size_t index = 0; index < edgeCount; ++index) {
        *edgeList.append() = &edges[index];
    }
    Edge edgeSentinel;
    edgeSentinel.fBounds.set(SK_ScalarMax, SK_ScalarMax, SK_ScalarMax, SK_ScalarMax);
    *edgeList.append() = &edgeSentinel;
    ++edgeCount;
    QSort<Edge>(edgeList.begin(), edgeCount);
    Edge** currentPtr = edgeList.begin();
    Edge* current = *currentPtr;
    SkScalar y = current->fBounds.fTop;
    SkScalar bottom = current->fBounds.fBottom;
    // walk the sorted edges from top to bottom, computing accumulated winding
    do {
        // find the list of edges that cross y
        Edge** lastPtr = currentPtr; // find the edge below the bottom of the first set
        Edge* last = *lastPtr;
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
            last = *++lastPtr;
        }
        if (asFill && lastPtr - currentPtr <= 1) {
            SkDebugf("expect 2 or more edges\n");
            SkASSERT(0);
            return;
        }
        // find any intersections in the range of active edges
        Edge** testPtr = currentPtr;
        Edge* test = *testPtr;
        while (testPtr != lastPtr) {
            if (test->fBounds.fBottom > bottom) {
                WorkEdge wt(test);
                do {
                    // FIXME: add all combinations of curve types
                    if (wt.verb() == SkPath::kLine_Verb) {
                        double wtTs[2];
                        int pts = lineIntersect(wt.points(), bottom, wtTs);
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
            Edge* next = *++testPtr;
            // OPTIMIZATION: if test and next is inside the winding of outer
            // edges such that intersecting them is irrelevent, skip them.
            if (!test->cached(next)
                    && Bounds::Intersects(test->fBounds, next->fBounds)) {
                WorkEdge wt(test);
                WorkEdge wn(next);
                do {
                    // FIXME: add all combinations of curve types
                    if (wt.verb() == SkPath::kLine_Verb && wn.verb() == SkPath::kLine_Verb) {
                        double wtTs[2], wnTs[2];
                        int pts = lineIntersect(wt.points(), wn.points(), wtTs, wnTs);
                        if (pts) {
                            test->add(wtTs, pts, wt.verbIndex());
                            next->add(wnTs, pts, wn.verbIndex());
                        }
                    }
                } while (wt.bottom() <= wn.bottom() ? wt.next() : wn.next());
            }
            test = next;
        }
        // stitch edge and t range that satisfies operation
        int winding = 0;
        testPtr = currentPtr;
        test = *testPtr;
        while (testPtr != lastPtr - 1) {
            int lastWinding = winding;
            winding += test->fWinding;
            if ((lastWinding & windingMask) == 0 || (winding & windingMask) == 0) {
                // append pts, verbs, in front of or behind output
                // a verb may have one or more inter-T value, but only break
                // curve if curve at t changes winding inclusion
                ;
            }
            test = *++testPtr;
        }
        y = bottom;
        while ((*currentPtr)->fBounds.fBottom >= y) {
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
