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
    enum class Direction {  // SkPath::Direction doesn't have 'none' state
        kCCW = -1,
        kNone,
        kCW,
    };

    Container(const SkRect& bounds, int lastStart, int verbStart)
        : fBounds(bounds)
        , fVerbStart(lastStart)
        , fVerbEnd(verbStart) {
    }

    vector<Container*> fChildren;
    const SkRect fBounds;
    SkPoint fMinXY{SK_ScalarMax, SK_ScalarMax};
    const int fVerbStart;
    const int fVerbEnd;
    Direction fDirection{Direction::kNone};
    bool fContained{false};
    bool fReverse{false};
};

static const int kPtCount[] = { 1, 1, 2, 2, 3, 1 };
static const int kPtIndex[] = { 0, 1, 1, 1, 1, 1 };

static Container::Direction to_direction(SkScalar dy) {
    return dy > 0 ? Container::Direction::kCCW : dy < 0 ? Container::Direction::kCW :
            Container::Direction::kNone;
}

static int compare_edge(SkPoint pts[4], SkPath::Verb verb, SkScalar weight, SkScalar yVal) {
    SkScalar yMin = pts[0].fY;
    SkScalar yMax = yMin;
    for (int index = 1; index <= kPtCount[verb]; ++index) {
        yMin = SkMinScalar(yMin, pts[index].fY);
        yMax = SkMaxScalar(yMax, pts[index].fY);
    }
    if (yMin > yVal) {
        return 0;
    }
    if (yMax <= yVal) {  // check to see if y is at line end to avoid double counting
        return 0;
    }
    start here;
    // need to see if pt x bounds are to left of compare x (not yet a param)
    // even if it is, need to see if intercept pt is to the left of compare x
    int winding = 0;
    double tVals[3];
    Container::Direction directions[3];
    // must intersect horz ray with curve in case it intersects more than once
    int count = (*CurveIntercept[verb * 2])(pts, weight, yVal, tVals);
    for (int index = 0; index < count; ++index) {
        directions[index] = to_direction((*CurveSlopeAtT[verb])(pts, weight, tVals[index]).fY);
    }
    SkASSERT(between(0, count, 3));
    for (int index = 0; index < count; ++index) {
        if (zero_or_one(tVals[index]) && Container::Direction::kCCW != directions[index]) {
            continue;
        }
        winding += (int) directions[index];
    }
    return winding;  // note winding does not indicate contour direction
}

static SkScalar conic_weight(const SkPath::Iter& iter, SkPath::Verb verb) {
    return SkPath::kConic_Verb == verb ? iter.conicWeight() : 1;
}

static SkPoint left_edge(SkPoint pts[4], SkPath::Verb verb, SkScalar weight,
        Container::Direction* direction) {
    SkASSERT(SkPath::kLine_Verb <= verb && verb <= SkPath::kCubic_Verb);
    SkPoint result;
    double dy;
    double t;
    int roots = 0;
    if (SkPath::kLine_Verb == verb) {
        result = pts[0].fX < pts[1].fX ? pts[0] : pts[1];
        dy = pts[1].fY - pts[0].fY;
    } else if (SkPath::kQuad_Verb == verb) {
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
    *direction = to_direction(dy);
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
    enum class Edge {
        kInitial,
        kCompare,
    };

    OpAsWinding(const SkPath& path)
        : fPath(path) {
    }

    void contourBounds(vector<Container>* containers) {
        SkRect bounds;
        bounds.setEmpty();
        SkPath::RawIter iter(fPath);
        SkPoint pts[4];
        SkPath::Verb verb;
        int lastStart = 0;
        int verbStart = 0;
        do {
            verb = iter.next(pts);
            if (SkPath::kMove_Verb == verb) {
                if (!bounds.isEmpty()) {
                    containers->emplace_back(bounds, lastStart, verbStart);
                    lastStart = verbStart;
               }
               bounds.setBounds(&pts[kPtIndex[verb]], kPtCount[verb]);
            }
            if (SkPath::kLine_Verb <= verb && verb <= SkPath::kCubic_Verb) {
                SkRect verbBounds;
                verbBounds.setBounds(&pts[kPtIndex[verb]], kPtCount[verb]);
                bounds.joinPossiblyEmptyRect(verbBounds);
            }
            ++verbStart;
        } while (SkPath::kDone_Verb != verb);
        if (!bounds.isEmpty()) {
            containers->emplace_back(bounds, lastStart, verbStart);
        }
    }

    int nextEdge(Container& bound, Edge edge) {
        SkPath::Iter iter(fPath, true);
        SkPoint pts[4];
        SkPath::Verb verb;
        int verbCount = -1;
        int winding = 0;
        do {
            verb = iter.next(pts);
            if (++verbCount < bound.fVerbStart) {
                continue;
            }
            if (verbCount >= bound.fVerbEnd) {
                continue;
            }
            if (SkPath::kLine_Verb > verb || verb > SkPath::kCubic_Verb) {
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
            if (edge == Edge::kCompare) {
                winding += compare_edge(pts, verb, conic_weight(iter, verb), bound.fMinXY.fY);
                continue;
            }
            SkASSERT(edge == Edge::kInitial);
            Container::Direction direction;
            SkPoint minXY = left_edge(pts, verb, conic_weight(iter, verb), &direction);
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
            bound.fMinXY = minXY;
            bound.fDirection = direction;
        } while (SkPath::kDone_Verb != verb);
        return winding;
    }

    void containerContains(Container& bound, Container& test) {
        // find outside point on lesser contour
        // arbitrarily, choose non-horizontal edge where point <= bounds left
        // note that if leftmost point is control point, may need tight bounds
            // to find edge with minimum-x
        if (SK_ScalarMax == test.fMinXY.fX) {
            this->nextEdge(test, Edge::kInitial);
        }
        // find all edges on greater equal or to the left of one on lesser
        bound.fMinXY = test.fMinXY;
        int winding = this->nextEdge(bound, Edge::kCompare);
        // if edge is up, mark contour cw, otherwise, ccw
        // sum of greater edges direction should be cw, 0, ccw
        SkASSERT(-1 <= winding && winding <= 1);
        test.fContained = winding != 0;
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

    void checkContainerChildren(Container* parent, Container* child) {
        for (auto grandChild : child->fChildren) {
            checkContainerChildren(child, grandChild);
        }
        if (parent) {
            containerContains(*parent, *child);
        }
    }

    bool markReverse(Container* parent, Container* child) {
        bool reversed = false;
        for (auto grandChild : child->fChildren) {
            reversed |= markReverse(grandChild->fContained ? child : parent, grandChild);
        }
        if (parent && parent->fDirection == child->fDirection) {
            child->fReverse = true;
            child->fDirection = (Container::Direction) -(int) child->fDirection;
            return true;
        }
        return reversed;
    }

    void reverseMarkedContours(vector<Container>& bounds, SkPath* result) {
        SkPath::RawIter iter(fPath);
        int verbCount = 0;
        for (auto bound : bounds) {
            SkPath reverse;
            SkPath* temp = bound.fReverse ? &reverse : result;
            do {
                SkPoint pts[4];
                switch (iter.next(pts)) {
                    case SkPath::kMove_Verb:
                        temp->moveTo(pts[0]);
                        break;
                    case SkPath::kLine_Verb:
                        temp->lineTo(pts[1]);
                        break;
                    case SkPath::kQuad_Verb:
                        temp->quadTo(pts[1], pts[2]);
                        break;
                    case SkPath::kConic_Verb:
                        temp->conicTo(pts[1], pts[2], iter.conicWeight());
                        break;
                    case SkPath::kCubic_Verb:
                        temp->cubicTo(pts[1], pts[2], pts[3]);
                        break;
                    case SkPath::kClose_Verb:
                        temp->close();
                        break;
                    default:
                        SkASSERT(0);  // should not hit done verb
                }
            } while (++verbCount < bound.fVerbEnd);
            if (bound.fReverse) {
                result->reverseAddPath(reverse);
            }
        }
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
    Container sorted(SkRect(), 0, 0);
    for (auto& bound : bounds) {
        winder.inParent(bound, sorted);
    }
    // if sorted has no grandchildren, no child has to fix its children's winding
    if (std::all_of(sorted.fChildren.begin(), sorted.fChildren.end(),
            [](const Container* bound) -> bool { return !bound->fChildren.size(); } )) {
        return set_result_path(result, path, fillType);
    }
    // starting with outermost and moving inward, see if one path contains another
    for (auto bound : sorted.fChildren) {
        winder.nextEdge(*bound, OpAsWinding::Edge::kInitial);
        winder.checkContainerChildren(nullptr, bound);
    }
    // starting with outermost and moving inward, mark paths to reverse
    bool reversed = false;
    for (auto bound : sorted.fChildren) {
        reversed |= winder.markReverse(nullptr, bound);
    }
    if (!reversed) {
        return set_result_path(result, path, fillType);
    }
    winder.reverseMarkedContours(bounds, result);
    return true;
}
