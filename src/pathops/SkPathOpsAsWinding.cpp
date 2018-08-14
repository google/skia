/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkOpEdgeBuilder.h"
#include "SkPathOpsCommon.h"
#include "SkRect.h"
#include <vector>

using std::vector;

struct Container {
    enum class Edge {
        kInitial,
        kCompare,
    };

    void set(int lastStart, int verbStart) {
        fVerbStart = lastStart;
        fVerbEnd = verbStart;
        fParent = nullptr;
    }

    Container* fParent;
    vector<Container*> fChildren;
    SkRect fBounds;
    SkPoint fMinXY;
    int fVerbLeft;
    int fVerbStart;
    int fVerbEnd;
    SkPath::Direction fDirection;
};

static const int kPtCount[] = { 1, 1, 2, 2, 3, 1 };
static const int kPtIndex[] = { 0, 1, 1, 1, 1, 1 };


// TODO: this has significant overhead. That overhead could be amortized
// if many curves required tight bounds computations
static SkPoint left_edge(SkPoint pts[4], SkPath::Verb verb, SkScalar weight,
        SkPath::Direction* direction) {
    SkPoint result;
    double dy;
    double t;
    int roots = 0;
    if (SkPath::kQuad_Verb == verb) {
        SkDQuad quad;
        quad.set(pts);
        if (!quad.monotonicInX()) {
            roots = SkDQuad::FindExtrema(&quad[0].fX, &t);
        }
        if (roots) {
            result = quad.ptAtT(t).asSkPoint();
        } else {
            result = pts[0].fX < pts[2].fX ? pts[0] : pts[2];
            t = pts[0].fX < pts[2].fX ? 0 : 1;
        }
        dy = quad.dxdyAtT(t).fY;
    } else if (SkPath::kConic_Verb == verb) {
        SkDConic conic;
        conic.set(pts, weight);
        if (!conic.monotonicInX()) {
            roots = SkDConic::FindExtrema(&conic[0].fX, weight, &t);
        }
        if (roots) {
            result = conic.ptAtT(t).asSkPoint();
        } else {
            result = pts[0].fX < pts[2].fX ? pts[0] : pts[2];
            t = pts[0].fX < pts[2].fX ? 0 : 1;
        }
        dy = conic.dxdyAtT(t).fY;
    } else {
        SkASSERT(SkPath::kCubic_Verb == verb);
        SkDCubic cubic;
        cubic.set(pts);
        if (cubic.monotonicInX()) {
            double tValues[2];
            roots = SkDCubic::FindExtrema(&cubic[0].fX, tValues);
            for (int index = 0; index < roots; ++index) {
                SkPoint temp = cubic.ptAtT(tValues[index]).asSkPoint();
                if (0 == index || result.fX > temp.fX) {
                    result = temp;
                    t = tValues[index];
                }
            }
        }
        if (roots) {
            result = cubic.ptAtT(t).asSkPoint();
        } else {
            result = pts[0].fX < pts[3].fX ? pts[0] : pts[3];
            t = pts[0].fX < pts[3].fX ? 0 : 1;
        }
        dy = cubic.dxdyAtT(t).fY;
    }
    *direction = dy > 0 ? SkPath::kCCW_Direction : SkPath::kCW_Direction;
    return result;
}

static SkRect set_edge_bounds(SkScalar minX, SkPoint pts[4], SkPath::Verb verb) {
    SkRect result;
    result.setXYWH(minX, pts[0].fY, 0, 0);
    for (int index = 1; index <= kPtCount[verb]; ++index) {
        result.fTop = SkMinScalar(result.fTop, pts[index].fY);
        result.fBottom = SkMaxScalar(result.fBottom, pts[index].fY);
    }
    return result;
}

class OpAsWinding {
public:
    OpAsWinding(const SkPath& path)
        : fPath(path) {
    }

    void contourBounds(vector<Container>* bounds) {
        SkRect verbBounds;
        Container working;
        working.fBounds.setEmpty();
        SkPath::RawIter iter(fPath);
        SkPoint pts[4];
        SkPath::Verb verb;
        int lastStart = 0;
        int verbStart = 0;
        do {
            verb = iter.next(pts);
            if (SkPath::kMove_Verb == verb) {
                if (!working.fBounds.isEmpty()) {
                    working.set(lastStart, verbStart);
                    bounds->push_back(working);
                    lastStart = verbStart;
               }
            }
            if (SkPath::kMove_Verb <= verb && verb <= SkPath::kCubic_Verb) {
                verbBounds.setBounds(&pts[kPtIndex[verb]], kPtCount[verb]);
                working.fBounds.joinPossiblyEmptyRect(verbBounds);
            }
            ++verbStart;
        } while (SkPath::kDone_Verb != verb);
        if (!working.fBounds.isEmpty()) {
            working.set(lastStart, verbStart);
            bounds->push_back(working);
        }
    }

    void leftEdge(Container& bound, Container::Edge edge) {
        SkPath::RawIter iter(fPath);
        SkPoint pts[4];
        SkPath::Verb verb;
        int verbCount = -1;
        do {
            verb = iter.next(pts);
            if (++verbCount < bound.fVerbStart) {
                continue;
            }
            if (verbCount >= bound.fVerbEnd) {
                continue;
            }
            if (SkPath::kLine_Verb > verb || verb > SkPath::kClose_Verb) {
                continue;
            }
            bool horizontal = true;
            for (int index = 1; index <= kPtCount[verb]; ++index) {
                if (pts[0].fY != pts[index].fY) {
                    horizontal = false;
                    break;
                }
            }
            if (horizontal) {
                continue;
            }
            if (edge == Container::Edge::kCompare) {
                SkScalar yMin = pts[0].fY;
                SkScalar yMax = yMin;
                for (int index = 1; index <= kPtCount[verb]; ++index) {
                    yMin = SkMinScalar(yMin, pts[index].fY);
                    yMax = SkMaxScalar(yMax, pts[index].fY);
                }
                if (yMin > bound.fMinXY.fY) {
                    continue;
                }
                if (yMax < bound.fMinXY.fY) {
                    continue;
                }
            }
            SkPoint minXY;
            SkPath::Direction direction;
            // start here;
            // its complicated
            // if checking kCompare, must check to see if y is at line end to avoid double counting
                // (end of one, start of next)
            // also, must intersect horz ray with curve in case it intersects more than once

            if (SkPath::kLine_Verb == verb) {
                minXY = pts[0].fX < pts[1].fX ? pts[0] : pts[1];
                direction = pts[0].fY < pts[1].fY ? SkPath::kCCW_Direction : SkPath::kCW_Direction;
            } else {
                SkASSERT(SkPath::kQuad_Verb <= verb && verb <= SkPath::kCubic_Verb);
                minXY = left_edge(pts, verb, iter.conicWeight(), &direction);
            }
            if (minXY.fX > bound.fMinXY.fX) {
                continue;
            }
            if (minXY.fX == bound.fMinXY.fX) {
                if (direction != bound.fDirection) {
                    continue;
                }
                // incomplete: must sort edges to find the one most to left
                SkDebugf("incomplete\n");
                // TODO: add edges as opangle and sort
            }

            if (edge == Container::Edge::kInitial) {
                bound.fMinXY = minXY;
                bound.fVerbLeft = verbCount;
                bound.fDirection = direction;
            }
        } while (SkPath::kDone_Verb != verb);
    }

    bool containerContains(Container& bound, Container& test) {
        Container& lesser = bound.fBounds.fLeft > test.fBounds.fLeft ? bound : test;
        Container& greater = &bound != &lesser ? bound : test;
        // find outside point on lesser contour
        // arbitrarily, choose non-horizontal edge where point <= bounds left
        // note that if leftmost point is control point, may need tight bounds
            // to find edge with minimum-x
        lesser.fMinXY.fX = SK_ScalarMax;
        this->leftEdge(lesser, Container::Edge::kInitial);
        // find all edges on greater equal or to the left of one on lesser
        greater.fMinXY = lesser.fMinXY;
        this->leftEdge(greater, Container::Edge::kCompare);
        // if edge is up, mark contour cw, otherwise, ccw
        // sum of greater edges direction should be cw, 0, ccw
        return true;
    }

    void inParent(Container& bound, Container& parent) {
        // move bound into sibling list contained by parent
        for (auto test : parent.fChildren) {
            if (test->fBounds.contains(bound.fBounds)) {
                inParent(bound, *test);
                return;
            }
        }
        // move parent's children into bound's children if contained by bound
        for (auto iter = parent.fChildren.begin(); iter != parent.fChildren.end(); ) {
            if (bound.fBounds.contains((*iter)->fBounds)) {
                bound.fChildren.push_back(*iter);
                iter = parent.fChildren.erase(iter);
                continue;
            }
            ++iter;
        }
        parent.fChildren.push_back(&bound);
    }

private:
    const SkPath& fPath;
};

static bool set_result_path(SkPath* result, const SkPath& path, SkPath::FillType fillType) {
    *result = path;
    result->setFillType(fillType);
    return true;
}

bool SK_API AsWinding(const SkPath& path, SkPath* result) {
    if (!path.isFinite()) {
        return false;
    }
    SkPath::FillType fillType = path.getFillType();
    if (fillType == SkPath::kWinding_FillType
            || fillType == SkPath::kInverseWinding_FillType ) {
        return set_result_path(result, path, fillType);
    }
    fillType = path.isInverseFillType() ? SkPath::kInverseWinding_FillType :
            SkPath::kWinding_FillType;
    if (path.isEmpty() || path.isConvex()) {
        return set_result_path(result, path, fillType);
    }
    // count contours
    vector<Container> bounds;   // one per contour
    OpAsWinding winder(path);
    winder.contourBounds(&bounds);
    if (bounds.size() <= 1) {
        return set_result_path(result, path, fillType);
    }
    // create contour bounding box tree
    Container sorted;
    for (auto bound : bounds) {
        winder.inParent(bound, sorted);
    }
    // if sorted has no grandchildren, no child has to fix its children's winding
    if (std::all_of(sorted.fChildren.begin(), sorted.fChildren.end(),
            [](const Container* bound) -> bool { return !bound->fChildren.size(); } )) {
        return set_result_path(result, path, fillType);
    }

    // starting with outermost and moving inward, see if one path contains another
    for (auto bound : sorted.fChildren) {

    }
    // if it does, and if the inner path has the same direction as the outer, reverse it

    // incomplete
    return false;
}

