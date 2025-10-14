/*
 * Copyright 2025 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkPathRef.h"
#include "src/core/SkPathPriv.h"

/*
 Determines if path is a rect by keeping track of changes in direction
 and looking for a loop either clockwise or counterclockwise.

 The direction is computed such that:
  0: vertical up
  1: horizontal left
  2: vertical down
  3: horizontal right

A rectangle cycles up/right/down/left or up/left/down/right.

The test fails if:
  The path is closed, and followed by a line.
  A second move creates a new endpoint.
  A diagonal line is parsed.
  There's more than four changes of direction.
  There's a discontinuity on the line (e.g., a move in the middle)
  The line reverses direction.
  The path contains a quadratic or cubic.
  The path contains fewer than four points.
 *The rectangle doesn't complete a cycle.
 *The final point isn't equal to the first point.

  *These last two conditions we relax if we have a 3-edge path that would
   form a rectangle if it were closed (as we do when we fill a path)

It's OK if the path has:
  Several colinear line segments composing a rectangle side.
  Single points on the rectangle side.

The direction takes advantage of the corners found since opposite sides
must travel in opposite directions.

FIXME: Allow colinear quads and cubics to be treated like lines.
FIXME: If the API passes fill-only, return true if the filled stroke
       is a rectangle, though the caller failed to close the path.

 directions values:
    0x1 is set if the segment is horizontal
    0x2 is set if the segment is moving to the right or down
 thus:
    two directions are opposites iff (dirA ^ dirB) == 0x2
    two directions are perpendicular iff (dirA ^ dirB) == 0x1

 */
static int rect_make_dir(SkScalar dx, SkScalar dy) {
    return ((0 != dx) << 0) | ((dx > 0 || dy > 0) << 1);
}

std::optional<SkPathPriv::RectContour> SkPathPriv::IsRectContour(SkSpan<const SkPoint> ptSpan,
                                                                 SkSpan<const SkPathVerb> vbSpan,
                                                                 bool allowPartial) {
    if (ptSpan.size() < 4) {
        return {};
    }

    size_t currVerb = 0;
    const size_t verbCnt = vbSpan.size();

    int corners = 0;
    SkPoint closeXY;  // used to determine if final line falls on a diagonal
    SkPoint lineStart;  // used to construct line from previous point
    const SkPoint* firstPt = nullptr; // first point in the rect (last of first moves)
    const SkPoint* lastPt = nullptr;  // last point in the rect (last of lines or first if closed)
    SkPoint firstCorner;
    SkPoint thirdCorner;
    const SkPoint* pts = ptSpan.data();
    const SkPoint* savePts = nullptr; // used to allow caller to iterate through a pair of rects
    lineStart.set(0, 0);
    signed char directions[] = {-1, -1, -1, -1, -1};  // -1 to 3; -1 is uninitialized
    bool closedOrMoved = false;
    bool autoClose = false;
    bool insertClose = false;
    while (currVerb < verbCnt && (!allowPartial || !autoClose)) {
        SkPathVerb verb = insertClose ? SkPathVerb::kClose : vbSpan[currVerb];
        switch (verb) {
            case SkPathVerb::kClose:
                savePts = pts;
                autoClose = true;
                insertClose = false;
                [[fallthrough]];
            case SkPathVerb::kLine: {
                if (SkPathVerb::kClose != verb) {
                    lastPt = pts;
                }
                SkPoint lineEnd = SkPathVerb::kClose == verb ? *firstPt : *pts++;
                SkVector lineDelta = lineEnd - lineStart;
                if (lineDelta.fX && lineDelta.fY) {
                    return {}; // diagonal
                }
                if (!lineDelta.isFinite()) {
                    return {}; // path contains infinity or NaN
                }
                if (lineStart == lineEnd) {
                    break; // single point on side OK
                }
                int nextDirection = rect_make_dir(lineDelta.fX, lineDelta.fY); // 0 to 3
                if (0 == corners) {
                    directions[0] = nextDirection;
                    corners = 1;
                    closedOrMoved = false;
                    lineStart = lineEnd;
                    break;
                }
                if (closedOrMoved) {
                    return {}; // closed followed by a line
                }
                if (autoClose && nextDirection == directions[0]) {
                    break; // colinear with first
                }
                closedOrMoved = autoClose;
                if (directions[corners - 1] == nextDirection) {
                    if (3 == corners && SkPathVerb::kLine == verb) {
                        thirdCorner = lineEnd;
                    }
                    lineStart = lineEnd;
                    break; // colinear segment
                }
                directions[corners++] = nextDirection;
                // opposite lines must point in opposite directions; xoring them should equal 2
                switch (corners) {
                    case 2:
                        firstCorner = lineStart;
                        break;
                    case 3:
                        if ((directions[0] ^ directions[2]) != 2) {
                            return {};
                        }
                        thirdCorner = lineEnd;
                        break;
                    case 4:
                        if ((directions[1] ^ directions[3]) != 2) {
                            return {};
                        }
                        break;
                    default:
                        return {}; // too many direction changes
                }
                lineStart = lineEnd;
                break;
            }
            case SkPathVerb::kQuad:
            case SkPathVerb::kConic:
            case SkPathVerb::kCubic:
                return {}; // quadratic, cubic not allowed
            case SkPathVerb::kMove:
                if (allowPartial && !autoClose && directions[0] >= 0) {
                    insertClose = true;
                    currVerb -= 1;  // try move again afterwards
                    goto addMissingClose;
                }
                if (!corners) {
                    firstPt = pts;
                } else {
                    closeXY = *firstPt - *lastPt;
                    if (closeXY.fX && closeXY.fY) {
                        return {};   // we're diagonal, abort
                    }
                }
                lineStart = *pts++;
                closedOrMoved = true;
                break;
            default:
                SkDEBUGFAIL("unexpected verb");
                break;
        }
        currVerb += 1;
    addMissingClose:
        ;
    }
    // Success if 4 corners and first point equals last
    if (corners < 3 || corners > 4) {
        return {};
    }
    // check if close generates diagonal
    closeXY = *firstPt - *lastPt;
    if (closeXY.fX && closeXY.fY) {
        return {};
    }

    auto bounds = [](SkPoint a, SkPoint b) {
        SkRect r;
        r.set(a, b);
        return r;
    };

    return {{
        bounds(firstCorner, thirdCorner),
        autoClose,
        directions[0] == ((directions[1] + 1) & 3) ? SkPathDirection::kCW
                                                   : SkPathDirection::kCCW,
        savePts ? size_t(savePts - ptSpan.data()) : 0,
        currVerb,
    }};
}

bool SkPathPriv::IsNestedFillRects(const SkPathRaw& raw, SkRect rects[2], SkPathDirection dirs[2]) {
    SkPathDirection testDirs[2];
    SkRect testRects[2];

    SkSpan<const SkPoint> pts = raw.points();
    SkSpan<const SkPathVerb> vbs = raw.verbs();

    auto rc = IsRectContour(pts, vbs, true);
    if (!rc) {
        return false;
    }

    testDirs[0] = rc->fDirection;
    testRects[0] = rc->fRect;
    pts = pts.subspan(rc->fPointsConsumed);
    vbs = vbs.subspan(rc->fVerbsConsumed);

    rc = IsRectContour(pts, vbs, false);
    if (rc) {
        testDirs[1] = rc->fDirection;
        testRects[1] = rc->fRect;
        if (testRects[0].contains(testRects[1])) {
            if (rects) {
                rects[0] = testRects[0];
                rects[1] = testRects[1];
            }
            if (dirs) {
                dirs[0] = testDirs[0];
                dirs[1] = testDirs[1];
            }
            return true;
        }
        if (testRects[1].contains(testRects[0])) {
            if (rects) {
                rects[0] = testRects[1];
                rects[1] = testRects[0];
            }
            if (dirs) {
                dirs[0] = testDirs[1];
                dirs[1] = testDirs[0];
            }
            return true;
        }
    }
    return false;
}

bool SkPathPriv::DrawArcIsConvex(SkScalar sweepAngle,
                                 SkArc::Type arcType,
                                 bool isFillNoPathEffect) {
    if (isFillNoPathEffect && SkScalarAbs(sweepAngle) >= 360.f) {
        // This gets converted to an oval.
        return true;
    }
    if (arcType == SkArc::Type::kWedge) {
        // This is a pie wedge. It's convex if the angle is <= 180.
        return SkScalarAbs(sweepAngle) <= 180.f;
    }
    // When the angle exceeds 360 this wraps back on top of itself. Otherwise it is a circle clipped
    // to a secant, i.e. convex.
    return SkScalarAbs(sweepAngle) <= 360.f;
}

SkPath SkPathPriv::CreateDrawArcPath(const SkArc& arc, bool isFillNoPathEffect) {
    SkRect oval = arc.fOval;
    SkScalar startAngle = arc.fStartAngle, sweepAngle = arc.fSweepAngle;
    SkASSERT(!oval.isEmpty());
    SkASSERT(sweepAngle);
    // We cap the number of total rotations. This keeps the resulting paths simpler. More important,
    // it prevents values so large that the loops below never terminate (once ULP > 360).
    if (SkScalarAbs(sweepAngle) > 3600.0f) {
        sweepAngle = std::copysign(3600.0f, sweepAngle) + std::fmod(sweepAngle, 360.0f);
    }

    SkPathBuilder builder(SkPathFillType::kWinding);
    builder.setIsVolatile(true);

    if (isFillNoPathEffect && SkScalarAbs(sweepAngle) >= 360.f) {
        builder.addOval(oval);
        SkASSERT(DrawArcIsConvex(sweepAngle, SkArc::Type::kArc, isFillNoPathEffect));
        return builder.detach();
    }

    if (arc.isWedge()) {
        builder.moveTo(oval.centerX(), oval.centerY());
    }
    auto firstDir =
            sweepAngle > 0 ? SkPathFirstDirection::kCW : SkPathFirstDirection::kCCW;
    bool convex = DrawArcIsConvex(sweepAngle, arc.fType, isFillNoPathEffect);
    // Arc to mods at 360 and drawArc is not supposed to.
    bool forceMoveTo = !arc.isWedge();
    while (sweepAngle <= -360.f) {
        builder.arcTo(oval, startAngle, -180.f, forceMoveTo);
        startAngle -= 180.f;
        builder.arcTo(oval, startAngle, -180.f, false);
        startAngle -= 180.f;
        forceMoveTo = false;
        sweepAngle += 360.f;
    }
    while (sweepAngle >= 360.f) {
        builder.arcTo(oval, startAngle, 180.f, forceMoveTo);
        startAngle += 180.f;
        builder.arcTo(oval, startAngle, 180.f, false);
        startAngle += 180.f;
        forceMoveTo = false;
        sweepAngle -= 360.f;
    }
    builder.arcTo(oval, startAngle, sweepAngle, forceMoveTo);
    if (arc.isWedge()) {
        builder.close();
    }

    auto path = builder.detach();
    const auto convexity = convex ? SkPathFirstDirection_ToConvexity(firstDir)
                                  : SkPathConvexity::kConcave;
    path.setConvexity(convexity);
    return path;
}

///////////////////////////////////////////////////////////////////////////

static int sign(SkScalar x) { return x < 0; }
#define kValueNeverReturnedBySign   2

enum DirChange {
    kUnknown_DirChange,
    kLeft_DirChange,
    kRight_DirChange,
    kStraight_DirChange,
    kBackwards_DirChange, // if double back, allow simple lines to be convex
    kInvalid_DirChange
};

// only valid for a single contour
struct Convexicator {

    /** The direction returned is only valid if the path is determined convex */
    SkPathFirstDirection getFirstDirection() const { return fFirstDirection; }

    void setMovePt(const SkPoint& pt) {
        fFirstPt = fLastPt = pt;
        fExpectedDir = kInvalid_DirChange;
    }

    bool addPt(const SkPoint& pt) {
        if (fLastPt == pt) {
            return true;
        }
        // should only be true for first non-zero vector after setMovePt was called. It is possible
        // we doubled backed at the start so need to check if fLastVec is zero or not.
        if (fFirstPt == fLastPt && fExpectedDir == kInvalid_DirChange && fLastVec.equals(0,0)) {
            fLastVec = pt - fLastPt;
            fFirstVec = fLastVec;
        } else if (!this->addVec(pt - fLastPt)) {
            return false;
        }
        fLastPt = pt;
        return true;
    }

    static bool IsConcaveBySign(const SkPoint points[], int count) {
        if (count <= 3) {
            // point, line, or triangle are always convex
            return false;
        }

        const SkPoint* last = points + count;
        SkPoint currPt = *points++;
        SkPoint firstPt = currPt;
        int dxes = 0;
        int dyes = 0;
        int lastSx = kValueNeverReturnedBySign;
        int lastSy = kValueNeverReturnedBySign;
        for (int outerLoop = 0; outerLoop < 2; ++outerLoop ) {
            while (points != last) {
                SkVector vec = *points - currPt;
                if (!vec.isZero()) {
                    // give up if vector construction failed
                    if (!vec.isFinite()) {
                        return true;    // treat as concave
                    }
                    int sx = sign(vec.fX);
                    int sy = sign(vec.fY);
                    dxes += (sx != lastSx);
                    dyes += (sy != lastSy);
                    if (dxes > 3 || dyes > 3) {
                        return true;
                    }
                    lastSx = sx;
                    lastSy = sy;
                }
                currPt = *points++;
                if (outerLoop) {
                    break;
                }
            }
            points = &firstPt;
        }
        return false;  // that is, it may be convex, don't know yet
    }

    bool close() {
        // If this was an explicit close, there was already a lineTo to fFirstPoint, so this
        // addPt() is a no-op. Otherwise, the addPt implicitly closes the contour. In either case,
        // we have to check the direction change along the first vector in case it is concave.
        return this->addPt(fFirstPt) && this->addVec(fFirstVec);
    }

    bool isFinite() const {
        return fIsFinite;
    }

    int reversals() const {
        return fReversals;
    }

private:
    DirChange directionChange(const SkVector& curVec) {
        SkScalar cross = SkPoint::CrossProduct(fLastVec, curVec);
        if (!SkIsFinite(cross)) {
            return kUnknown_DirChange;
        }
        if (cross == 0) {
            return fLastVec.dot(curVec) < 0 ? kBackwards_DirChange : kStraight_DirChange;
        }
        return 1 == SkScalarSignAsInt(cross) ? kRight_DirChange : kLeft_DirChange;
    }

    bool addVec(const SkVector& curVec) {
        DirChange dir = this->directionChange(curVec);
        switch (dir) {
            case kLeft_DirChange:       // fall through
            case kRight_DirChange:
                if (kInvalid_DirChange == fExpectedDir) {
                    fExpectedDir = dir;
                    fFirstDirection = (kRight_DirChange == dir) ? SkPathFirstDirection::kCW
                                                                : SkPathFirstDirection::kCCW;
                } else if (dir != fExpectedDir) {
                    fFirstDirection = SkPathFirstDirection::kUnknown;
                    return false;
                }
                fLastVec = curVec;
                break;
            case kStraight_DirChange:
                break;
            case kBackwards_DirChange:
                //  allow path to reverse direction twice
                //    Given path.moveTo(0, 0); path.lineTo(1, 1);
                //    - 1st reversal: direction change formed by line (0,0 1,1), line (1,1 0,0)
                //    - 2nd reversal: direction change formed by line (1,1 0,0), line (0,0 1,1)
                fLastVec = curVec;
                return ++fReversals < 3;
            case kUnknown_DirChange:
                return (fIsFinite = false);
            case kInvalid_DirChange:
                SK_ABORT("Use of invalid direction change flag");
                break;
        }
        return true;
    }

    SkPoint              fFirstPt {0, 0};  // The first point of the contour, e.g. moveTo(x,y)
    SkVector             fFirstVec {0, 0}; // The direction leaving fFirstPt to the next vertex

    SkPoint              fLastPt {0, 0};   // The last point passed to addPt()
    SkVector             fLastVec {0, 0};  // The direction that brought the path to fLastPt

    DirChange            fExpectedDir { kInvalid_DirChange };
    SkPathFirstDirection fFirstDirection { SkPathFirstDirection::kUnknown };
    int                  fReversals { 0 };
    bool                 fIsFinite { true };
};

static void trim_trailing_moves(SkSpan<const SkPoint>& pts, SkSpan<const SkPathVerb>& vbs) {
    size_t vbCount = vbs.size();
    while (vbCount > 0 && vbs[vbCount - 1] == SkPathVerb::kMove) {
        vbCount -= 1;
    }
    if (size_t delta = vbs.size() - vbCount) {
        SkASSERT(pts.size() >= delta);
        pts = {pts.data(), pts.size() - delta};
        vbs = {vbs.data(), vbs.size() - delta};
    }
}

SkPathConvexity SkPathPriv::ComputeConvexity(SkSpan<const SkPoint> points,
                                             SkSpan<const SkPathVerb> vbs,
                                             SkSpan<const float> conicWeights) {
    // callers need to give us finite values
    SkASSERT(SkRect::Bounds(points).has_value());

    trim_trailing_moves(points, vbs);

    if (vbs.empty()) {
        return SkPathConvexity::kConvex_Degenerate;
    }

    // Check to see if path changes direction more than three times as quick concave test
    if (Convexicator::IsConcaveBySign(points.data(), points.size())) {
        return SkPathConvexity::kConcave;
    }

    int contourCount = 0;
    bool needsClose = false;
    Convexicator state;

    auto iter = SkPathIter(points, vbs, conicWeights);
    while (auto rec = iter.next()) {
        auto verb = rec->fVerb;
        auto pts = rec->fPoints;

        // Looking for the last moveTo before non-move verbs start
        if (contourCount == 0) {
            if (verb == SkPathVerb::kMove) {
                state.setMovePt(pts[0]);
            } else {
                // Starting the actual contour, fall through to c=1 to add the points
                contourCount++;
                needsClose = true;
            }
        }
        // Accumulating points into the Convexicator until we hit a close or another move
        if (contourCount == 1) {
            if (verb == SkPathVerb::kClose || verb == SkPathVerb::kMove) {
                if (!state.close()) {
                    return SkPathConvexity::kConcave;
                }
                needsClose = false;
                contourCount++;
            } else {
                // lines add 1 point, cubics add 3, conics and quads add 2
                int count = SkPathPriv::PtsInVerb((unsigned) verb);
                SkASSERT(count > 0);
                for (int i = 1; i <= count; ++i) {
                    if (!state.addPt(pts[i])) {
                        return SkPathConvexity::kConcave;
                    }
                }
            }
        } else {
            // The first contour has closed and anything other than spurious trailing moves means
            // there's multiple contours and the path can't be convex
            if (verb != SkPathVerb::kMove) {
                return SkPathConvexity::kConcave;
            }
        }
    }

    // If the path isn't explicitly closed do so implicitly
    if (needsClose && !state.close()) {
        return SkPathConvexity::kConcave;
    }

    const auto firstDir = state.getFirstDirection();
    if (firstDir == SkPathFirstDirection::kUnknown && state.reversals() >= 3) {
        return SkPathConvexity::kConcave;
    }
    return SkPathFirstDirection_ToConvexity(firstDir);
}

///////////////////////////////////////////////////////////////////////////////

// returns cross product of (p1 - p0) and (p2 - p0)
static SkScalar cross_prod(const SkPoint& p0, const SkPoint& p1, const SkPoint& p2) {
    SkScalar cross = SkPoint::CrossProduct(p1 - p0, p2 - p0);
    // We may get 0 when the above subtracts underflow. We expect this to be
    // very rare and lazily promote to double.
    if (0 == cross) {
        double p0x = SkScalarToDouble(p0.fX);
        double p0y = SkScalarToDouble(p0.fY);

        double p1x = SkScalarToDouble(p1.fX);
        double p1y = SkScalarToDouble(p1.fY);

        double p2x = SkScalarToDouble(p2.fX);
        double p2y = SkScalarToDouble(p2.fY);

        cross = SkDoubleToScalar((p1x - p0x) * (p2y - p0y) -
                                 (p1y - p0y) * (p2x - p0x));

    }
    return cross;
}

// Returns the first pt with the maximum Y coordinate
static int find_max_y(const SkPoint pts[], int count) {
    SkASSERT(count > 0);
    SkScalar max = pts[0].fY;
    int firstIndex = 0;
    for (int i = 1; i < count; ++i) {
        SkScalar y = pts[i].fY;
        if (y > max) {
            max = y;
            firstIndex = i;
        }
    }
    return firstIndex;
}

static int find_diff_pt(const SkPoint pts[], int index, int n, int inc) {
    int i = index;
    for (;;) {
        i = (i + inc) % n;
        if (i == index) {   // we wrapped around, so abort
            break;
        }
        if (pts[index] != pts[i]) { // found a different point, success!
            break;
        }
    }
    return i;
}

/**
 *  Starting at index, and moving forward (incrementing), find the xmin and
 *  xmax of the contiguous points that have the same Y.
 */
static int find_min_max_x_at_y(const SkPoint pts[], int index, int n,
                               int* maxIndexPtr) {
    const SkScalar y = pts[index].fY;
    SkScalar min = pts[index].fX;
    SkScalar max = min;
    int minIndex = index;
    int maxIndex = index;
    for (int i = index + 1; i < n; ++i) {
        if (pts[i].fY != y) {
            break;
        }
        SkScalar x = pts[i].fX;
        if (x < min) {
            min = x;
            minIndex = i;
        } else if (x > max) {
            max = x;
            maxIndex = i;
        }
    }
    *maxIndexPtr = maxIndex;
    return minIndex;
}

static SkPathFirstDirection crossToDir(SkScalar cross) {
    return cross > 0 ? SkPathFirstDirection::kCW : SkPathFirstDirection::kCCW;
}

class ContourIter {
public:
    ContourIter(SkSpan<const SkPoint>, SkSpan<const SkPathVerb>, SkSpan<const float> conicWeights);

    bool done() const { return fDone; }
    // if !done() then these may be called
    int count() const { return fCurrPtCount; }
    const SkPoint* pts() const { return fCurrPt; }
    void next();

private:
    int fCurrPtCount;
    const SkPoint* fCurrPt;
    const SkPathVerb* fCurrVerb;
    const SkPathVerb* fStopVerbs;
    const SkScalar* fCurrConicWeight;
    bool fDone;
    SkDEBUGCODE(int fContourCounter;)
};

ContourIter::ContourIter(SkSpan<const SkPoint> pts, SkSpan<const SkPathVerb> vbs,
                         SkSpan<const float> conicWeights) {
    fStopVerbs = vbs.end();
    fDone = false;
    fCurrPt = pts.data();
    fCurrVerb = vbs.data();
    fCurrConicWeight = conicWeights.data();
    fCurrPtCount = 0;
    SkDEBUGCODE(fContourCounter = 0;)
    this->next();
}

void ContourIter::next() {
    if (fCurrVerb >= fStopVerbs) {
        fDone = true;
    }
    if (fDone) {
        return;
    }

    // skip pts of prev contour
    fCurrPt += fCurrPtCount;

    SkASSERT(SkPathVerb::kMove == fCurrVerb[0]);
    int ptCount = 1;    // moveTo
    const SkPathVerb* verbs = fCurrVerb;

    for (verbs++; verbs < fStopVerbs; verbs++) {
        switch (*verbs) {
            case SkPathVerb::kMove:
                goto CONTOUR_END;
            case SkPathVerb::kLine:
                ptCount += 1;
                break;
            case SkPathVerb::kConic:
                fCurrConicWeight += 1;
                [[fallthrough]];
            case SkPathVerb::kQuad:
                ptCount += 2;
                break;
            case SkPathVerb::kCubic:
                ptCount += 3;
                break;
            case SkPathVerb::kClose:
                break;
        }
    }
CONTOUR_END:
    fCurrPtCount = ptCount;
    fCurrVerb = verbs;
    SkDEBUGCODE(++fContourCounter;)
}

/*
 *  We loop through all contours, and keep the computed cross-product of the
 *  contour that contained the global y-max. If we just look at the first
 *  contour, we may find one that is wound the opposite way (correctly) since
 *  it is the interior of a hole (e.g. 'o'). Thus we must find the contour
 *  that is outer most (or at least has the global y-max) before we can consider
 *  its cross product.
 */
SkPathFirstDirection SkPathPriv::ComputeFirstDirection(const SkPathRaw& raw) {
    ContourIter iter(raw.points(), raw.verbs(), raw.conics());

    // initialize with our logical y-min
    SkScalar ymax = raw.bounds().fTop;
    SkScalar ymaxCross = 0;

    for (; !iter.done(); iter.next()) {
        int n = iter.count();
        if (n < 3) {
            continue;
        }

        const SkPoint* pts = iter.pts();
        SkScalar cross = 0;
        int index = find_max_y(pts, n);
        if (pts[index].fY < ymax) {
            continue;
        }

        // If there is more than 1 distinct point at the y-max, we take the
        // x-min and x-max of them and just subtract to compute the dir.
        if (pts[(index + 1) % n].fY == pts[index].fY) {
            int maxIndex;
            int minIndex = find_min_max_x_at_y(pts, index, n, &maxIndex);
            if (minIndex == maxIndex) {
                goto TRY_CROSSPROD;
            }
            SkASSERT(pts[minIndex].fY == pts[index].fY);
            SkASSERT(pts[maxIndex].fY == pts[index].fY);
            SkASSERT(pts[minIndex].fX <= pts[maxIndex].fX);
            // we just subtract the indices, and let that auto-convert to
            // SkScalar, since we just want - or + to signal the direction.
            cross = minIndex - maxIndex;
        } else {
            TRY_CROSSPROD:
            // Find a next and prev index to use for the cross-product test,
            // but we try to find pts that form non-zero vectors from pts[index]
            //
            // Its possible that we can't find two non-degenerate vectors, so
            // we have to guard our search (e.g. all the pts could be in the
            // same place).

            // we pass n - 1 instead of -1 so we don't foul up % operator by
            // passing it a negative LH argument.
            int prev = find_diff_pt(pts, index, n, n - 1);
            if (prev == index) {
                // completely degenerate, skip to next contour
                continue;
            }
            int next = find_diff_pt(pts, index, n, 1);
            SkASSERT(next != index);
            cross = cross_prod(pts[prev], pts[index], pts[next]);
            // if we get a zero and the points are horizontal, then we look at the spread in
            // x-direction. We really should continue to walk away from the degeneracy until
            // there is a divergence.
            if (0 == cross && pts[prev].fY == pts[index].fY && pts[next].fY == pts[index].fY) {
                // construct the subtract so we get the correct Direction below
                cross = pts[index].fX - pts[next].fX;
            }
        }

        if (cross) {
            // record our best guess so far
            ymax = pts[index].fY;
            ymaxCross = cross;
        }
    }

    return ymaxCross ? crossToDir(ymaxCross) : SkPathFirstDirection::kUnknown;
}

//////////////////////////////////////////////////////////////////////////////

SkPathVerbAnalysis SkPathPriv::AnalyzeVerbs(SkSpan<const SkPathVerb> vbs) {
    SkPathVerbAnalysis info = {false, 0, 0, 0};
    bool needMove = true;
    bool invalid = false;

    if (vbs.size() >= (INT_MAX / 3)) SK_UNLIKELY {
        // A path with an extremely high number of quad, conic or cubic verbs could cause
        // `info.points` to overflow. To prevent against this, we reject extremely large paths. This
        // check is conservative and assumes the worst case (in particular, it assumes that every
        // verb consumes 3 points, which would only happen for a path composed entirely of cubics).
        // This limits us to 700 million verbs, which is large enough for any reasonable use case.
        invalid = true;
    } else {
        for (auto v : vbs) {
            switch (v) {
                case SkPathVerb::kMove:
                    needMove = false;
                    info.points += 1;
                    break;
                case SkPathVerb::kLine:
                    invalid |= needMove;
                    info.segmentMask |= kLine_SkPathSegmentMask;
                    info.points += 1;
                    break;
                case SkPathVerb::kQuad:
                    invalid |= needMove;
                    info.segmentMask |= kQuad_SkPathSegmentMask;
                    info.points += 2;
                    break;
                case SkPathVerb::kConic:
                    invalid |= needMove;
                    info.segmentMask |= kConic_SkPathSegmentMask;
                    info.points += 2;
                    info.weights += 1;
                    break;
                case SkPathVerb::kCubic:
                    invalid |= needMove;
                    info.segmentMask |= kCubic_SkPathSegmentMask;
                    info.points += 3;
                    break;
                case SkPathVerb::kClose:
                    invalid |= needMove;
                    needMove = true;
                    break;
                default:
                    invalid = true;
                    break;
            }
        }
    }
    info.valid = !invalid;
    return info;
}

bool SkPathPriv::IsAxisAligned(SkSpan<const SkPoint> pts) {
    // Conservative (quick) test to see if all segments are axis-aligned.
    // Multiple contours might give a false-negative, but for speed, we ignore that
    // and just look at the raw points.

    for (size_t i = 1; i < pts.size(); ++i) {
        if (pts[i-1].fX != pts[i].fX && pts[i-1].fY != pts[i].fY) {
            return false;
        }
    }
    return true;
}
