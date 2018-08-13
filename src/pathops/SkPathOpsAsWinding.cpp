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
        fDirection = -1;
    }

    Container* fParent;
    vector<Container*> fChildren;
    SkRect fBounds;
    SkRect fMinLeft;
    int fVerbLeft;
    int fVerbStart;
    int fVerbEnd;
    int fDirection;
};

static const int kPtCount[] = { 1, 1, 2, 2, 3, 1 };
static const int kPtIndex[] = { 0, 1, 1, 1, 1, 1 };

// TODO: this has significant overhead. That overhead could be amortized
// if many curves required tight bounds computations
static SkScalar tight_bounds(SkPath::Verb verb, SkPoint pts[4], SkScalar weight, SkScalar minX) {
    SkPath path;
    path.moveTo(pts[0]);
    if (SkPath::kQuad_Verb == verb) {
        path.quadTo(pts[1], pts[2]);
    } else if (SkPath::kConic_Verb == verb) {
        path.conicTo(pts[1], pts[2], weight);
    } else {
        SkASSERT(SkPath::kCubic_Verb == verb);
        path.cubicTo(pts[1], pts[2], pts[3]);
    }
    SkRect bounds;
    bool success = TightBounds(path, &bounds);
    return success ? bounds.fLeft : minX;
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
            SkScalar minX = pts[0].fX;
            for (int index = 1; index <= kPtCount[verb]; ++index) {
                minX = SkMinScalar(minX, pts[index].fX);
            }
            if (minX > bound.fMinLeft.fLeft) {
                continue;
            }
            if (SkPath::kQuad_Verb <= verb && verb <= SkPath::kCubic_Verb) {
                // find tight bounds if necessary
                SkScalar end = pts[kPtIndex[verb]].fX;
                if ((pts[1].fX < end && pts[1].fX < pts[0].fX)
                        || (SkPath::kCubic_Verb == verb &&
                        pts[2].fX < end && pts[2].fX < pts[0].fX)) {
                    minX = tight_bounds(verb, pts, iter.conicWeight(), minX);
                }
            }
            if (minX > bound.fMinLeft.fLeft) {
                continue;
            }
            if (minX == bound.fMinLeft.fLeft) {
                // incomplete: must sort edges to find the one most to left
                SkDebugf("incomplete\n");
                // TODO: add edges as opangle and sort
            }
            if (edge == Container::Edge::kInitial) {
                bound.fMinLeft.setXYWH(minX, pts[0].fY, 0, 0);
                for (int index = 1; index <= kPtCount[verb]; ++index) {
                    bound.fMinLeft.fTop = SkMinScalar(bound.fMinLeft.fTop, pts[index].fY);
                    bound.fMinLeft.fBottom = SkMaxScalar(bound.fMinLeft.fBottom, pts[index].fY);
                }
                bound.fVerbLeft = verbCount;
            } else {

            }
        } while (SkPath::kDone_Verb != verb);
    }

    bool containerContains(Container& bound, Container& test) {
        if (!bound.fBounds.contains(test.fBounds)) {
            return false;
        }
        if (test.fBounds.contains(bound.fBounds)) {
            return false;
        }
        Container& lesser = bound.fBounds.fLeft > test.fBounds.fLeft ? bound : test;
        Container& greater = &bound != &lesser ? bound : test;
        // find outside point on lesser contour
        // arbitrarily, choose non-horizontal edge where point <= bounds left
        // note that if leftmost point is control point, may need tight bounds
            // to find edge with minimum-x
        lesser.fMinLeft.fLeft = SK_ScalarMax;
        this->leftEdge(lesser, Container::Edge::kInitial);
        // find all edges on greater equal or to the left of one on lesser
        greater.fMinLeft = lesser.fMinLeft;
        this->leftEdge(greater, Container::Edge::kCompare);
        // if edge is up, mark contour cw, otherwise, ccw
        // sum of greater edges direction should be cw, 0, ccw
        return true;
    }

    void inParent(Container& bound, Container& parent) {
        // move bound into sibling list contained by parent
        for (auto test : parent.fChildren) {
            if (this->containerContains(*test, bound)) {
                inParent(bound, *test);
                return;
            }
        }
        // move parent's children into bound's children if contained by bound
        for (auto iter = parent.fChildren.begin(); iter != parent.fChildren.end(); ) {
            if (this->containerContains(bound, **iter)) {
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

bool SK_API AsWinding(const SkPath& path, SkPath* result) {
    if (!path.isFinite()) {
        return false;
    }
    SkPath::FillType fillType = path.getFillType();
    if (fillType == SkPath::kWinding_FillType
            || fillType == SkPath::kInverseWinding_FillType ) {
        *result = path;
        return true;
    }
    fillType = path.isInverseFillType() ? SkPath::kInverseWinding_FillType :
            SkPath::kWinding_FillType;
    if (path.isEmpty() || path.isConvex()) {
        *result = path;
        result->setFillType(fillType);
        return true;
    }
    // count contours
    vector<Container> bounds;
    OpAsWinding winder(path);
    winder.contourBounds(&bounds);
    if (bounds.size() <= 1) {
        *result = path;
        result->setFillType(fillType);
        return true;
    }
    Container sorted;
    for (auto bound : bounds) {
        winder.inParent(bound, sorted);
    }
    // incomplete
    return false;
}

