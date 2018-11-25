/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <cmath>
#include "SkBuffer.h"
#include "SkCubicClipper.h"
#include "SkData.h"
#include "SkGeometry.h"
#include "SkMath.h"
#include "SkMatrixPriv.h"
#include "SkPathPriv.h"
#include "SkPathRef.h"
#include "SkPointPriv.h"
#include "SkRRect.h"

static float poly_eval(float A, float B, float C, float t) {
    return (A * t + B) * t + C;
}

static float poly_eval(float A, float B, float C, float D, float t) {
    return ((A * t + B) * t + C) * t + D;
}

////////////////////////////////////////////////////////////////////////////

/**
 *  Path.bounds is defined to be the bounds of all the control points.
 *  If we called bounds.join(r) we would skip r if r was empty, which breaks
 *  our promise. Hence we have a custom joiner that doesn't look at emptiness
 */
static void joinNoEmptyChecks(SkRect* dst, const SkRect& src) {
    dst->fLeft = SkMinScalar(dst->fLeft, src.fLeft);
    dst->fTop = SkMinScalar(dst->fTop, src.fTop);
    dst->fRight = SkMaxScalar(dst->fRight, src.fRight);
    dst->fBottom = SkMaxScalar(dst->fBottom, src.fBottom);
}

static bool is_degenerate(const SkPath& path) {
    SkPath::Iter iter(path, false);
    SkPoint pts[4];
    return SkPath::kDone_Verb == iter.next(pts);
}

class SkAutoDisableDirectionCheck {
public:
    SkAutoDisableDirectionCheck(SkPath* path) : fPath(path) {
        fSaved = static_cast<SkPathPriv::FirstDirection>(fPath->fFirstDirection.load());
    }

    ~SkAutoDisableDirectionCheck() {
        fPath->fFirstDirection = fSaved;
    }

private:
    SkPath*                     fPath;
    SkPathPriv::FirstDirection  fSaved;
};
#define SkAutoDisableDirectionCheck(...) SK_REQUIRE_LOCAL_VAR(SkAutoDisableDirectionCheck)

/*  This guy's constructor/destructor bracket a path editing operation. It is
    used when we know the bounds of the amount we are going to add to the path
    (usually a new contour, but not required).

    It captures some state about the path up front (i.e. if it already has a
    cached bounds), and then if it can, it updates the cache bounds explicitly,
    avoiding the need to revisit all of the points in getBounds().

    It also notes if the path was originally degenerate, and if so, sets
    isConvex to true. Thus it can only be used if the contour being added is
    convex.
 */
class SkAutoPathBoundsUpdate {
public:
    SkAutoPathBoundsUpdate(SkPath* path, const SkRect& r) : fRect(r) {
        this->init(path);
    }

    SkAutoPathBoundsUpdate(SkPath* path, SkScalar left, SkScalar top,
                           SkScalar right, SkScalar bottom) {
        fRect.set(left, top, right, bottom);
        this->init(path);
    }

    ~SkAutoPathBoundsUpdate() {
        fPath->setConvexity(fDegenerate ? SkPath::kConvex_Convexity
                                        : SkPath::kUnknown_Convexity);
        if ((fEmpty || fHasValidBounds) && fRect.isFinite()) {
            fPath->setBounds(fRect);
        }
    }

private:
    SkPath* fPath;
    SkRect  fRect;
    bool    fHasValidBounds;
    bool    fDegenerate;
    bool    fEmpty;

    void init(SkPath* path) {
        // Cannot use fRect for our bounds unless we know it is sorted
        fRect.sort();
        fPath = path;
        // Mark the path's bounds as dirty if (1) they are, or (2) the path
        // is non-finite, and therefore its bounds are not meaningful
        fHasValidBounds = path->hasComputedBounds() && path->isFinite();
        fEmpty = path->isEmpty();
        if (fHasValidBounds && !fEmpty) {
            joinNoEmptyChecks(&fRect, fPath->getBounds());
        }
        fDegenerate = is_degenerate(*path);
    }
};
#define SkAutoPathBoundsUpdate(...) SK_REQUIRE_LOCAL_VAR(SkAutoPathBoundsUpdate)

////////////////////////////////////////////////////////////////////////////

/*
    Stores the verbs and points as they are given to us, with exceptions:
    - we only record "Close" if it was immediately preceeded by Move | Line | Quad | Cubic
    - we insert a Move(0,0) if Line | Quad | Cubic is our first command

    The iterator does more cleanup, especially if forceClose == true
    1. If we encounter degenerate segments, remove them
    2. if we encounter Close, return a cons'd up Line() first (if the curr-pt != start-pt)
    3. if we encounter Move without a preceeding Close, and forceClose is true, goto #2
    4. if we encounter Line | Quad | Cubic after Close, cons up a Move
*/

////////////////////////////////////////////////////////////////////////////

// flag to require a moveTo if we begin with something else, like lineTo etc.
#define INITIAL_LASTMOVETOINDEX_VALUE   ~0

SkPath::SkPath()
    : fPathRef(SkPathRef::CreateEmpty()) {
    this->resetFields();
    fIsVolatile = false;
}

void SkPath::resetFields() {
    //fPathRef is assumed to have been emptied by the caller.
    fLastMoveToIndex = INITIAL_LASTMOVETOINDEX_VALUE;
    fFillType = kWinding_FillType;
    fConvexity = kUnknown_Convexity;
    fFirstDirection = SkPathPriv::kUnknown_FirstDirection;

    // We don't touch Android's fSourcePath.  It's used to track texture garbage collection, so we
    // don't want to muck with it if it's been set to something non-nullptr.
}

SkPath::SkPath(const SkPath& that)
    : fPathRef(SkRef(that.fPathRef.get())) {
    this->copyFields(that);
    SkDEBUGCODE(that.validate();)
}

SkPath::~SkPath() {
    SkDEBUGCODE(this->validate();)
}

SkPath& SkPath::operator=(const SkPath& that) {
    SkDEBUGCODE(that.validate();)

    if (this != &that) {
        fPathRef.reset(SkRef(that.fPathRef.get()));
        this->copyFields(that);
    }
    SkDEBUGCODE(this->validate();)
    return *this;
}

void SkPath::copyFields(const SkPath& that) {
    //fPathRef is assumed to have been set by the caller.
    fLastMoveToIndex = that.fLastMoveToIndex;
    fFillType        = that.fFillType;
    fIsVolatile      = that.fIsVolatile;

    // Non-atomic assignment of atomic values.
    fConvexity     .store(that.fConvexity     .load());
    fFirstDirection.store(that.fFirstDirection.load());
}

bool operator==(const SkPath& a, const SkPath& b) {
    // note: don't need to look at isConvex or bounds, since just comparing the
    // raw data is sufficient.
    return &a == &b ||
        (a.fFillType == b.fFillType && *a.fPathRef.get() == *b.fPathRef.get());
}

void SkPath::swap(SkPath& that) {
    if (this != &that) {
        fPathRef.swap(that.fPathRef);
        SkTSwap<int>(fLastMoveToIndex, that.fLastMoveToIndex);
        SkTSwap<uint8_t>(fFillType, that.fFillType);
        SkTSwap<SkBool8>(fIsVolatile, that.fIsVolatile);

        // Non-atomic swaps of atomic values.
        Convexity c = fConvexity.load();
        fConvexity.store(that.fConvexity.load());
        that.fConvexity.store(c);

        uint8_t fd = fFirstDirection.load();
        fFirstDirection.store(that.fFirstDirection.load());
        that.fFirstDirection.store(fd);
    }
}

bool SkPath::isInterpolatable(const SkPath& compare) const {
    int count = fPathRef->countVerbs();
    if (count != compare.fPathRef->countVerbs()) {
        return false;
    }
    if (!count) {
        return true;
    }
    if (memcmp(fPathRef->verbsMemBegin(), compare.fPathRef->verbsMemBegin(),
               count)) {
        return false;
    }
    return !fPathRef->countWeights() ||
            !SkToBool(memcmp(fPathRef->conicWeights(), compare.fPathRef->conicWeights(),
            fPathRef->countWeights() * sizeof(*fPathRef->conicWeights())));
}

bool SkPath::interpolate(const SkPath& ending, SkScalar weight, SkPath* out) const {
    int verbCount = fPathRef->countVerbs();
    if (verbCount != ending.fPathRef->countVerbs()) {
        return false;
    }
    if (!verbCount) {
        return true;
    }
    out->reset();
    out->addPath(*this);
    fPathRef->interpolate(*ending.fPathRef, weight, out->fPathRef.get());
    return true;
}

static inline bool check_edge_against_rect(const SkPoint& p0,
                                           const SkPoint& p1,
                                           const SkRect& rect,
                                           SkPathPriv::FirstDirection dir) {
    const SkPoint* edgeBegin;
    SkVector v;
    if (SkPathPriv::kCW_FirstDirection == dir) {
        v = p1 - p0;
        edgeBegin = &p0;
    } else {
        v = p0 - p1;
        edgeBegin = &p1;
    }
    if (v.fX || v.fY) {
        // check the cross product of v with the vec from edgeBegin to each rect corner
        SkScalar yL = v.fY * (rect.fLeft - edgeBegin->fX);
        SkScalar xT = v.fX * (rect.fTop - edgeBegin->fY);
        SkScalar yR = v.fY * (rect.fRight - edgeBegin->fX);
        SkScalar xB = v.fX * (rect.fBottom - edgeBegin->fY);
        if ((xT < yL) || (xT < yR) || (xB < yL) || (xB < yR)) {
            return false;
        }
    }
    return true;
}

bool SkPath::conservativelyContainsRect(const SkRect& rect) const {
    // This only handles non-degenerate convex paths currently.
    if (kConvex_Convexity != this->getConvexity()) {
        return false;
    }

    SkPathPriv::FirstDirection direction;
    if (!SkPathPriv::CheapComputeFirstDirection(*this, &direction)) {
        return false;
    }

    SkPoint firstPt;
    SkPoint prevPt;
    SkPath::Iter iter(*this, true);
    SkPath::Verb verb;
    SkPoint pts[4];
    int segmentCount = 0;
    SkDEBUGCODE(int moveCnt = 0;)
    SkDEBUGCODE(int closeCount = 0;)

    while ((verb = iter.next(pts, true, true)) != kDone_Verb) {
        int nextPt = -1;
        switch (verb) {
            case kMove_Verb:
                SkASSERT(!segmentCount && !closeCount);
                SkDEBUGCODE(++moveCnt);
                firstPt = prevPt = pts[0];
                break;
            case kLine_Verb:
                nextPt = 1;
                SkASSERT(moveCnt && !closeCount);
                ++segmentCount;
                break;
            case kQuad_Verb:
            case kConic_Verb:
                SkASSERT(moveCnt && !closeCount);
                ++segmentCount;
                nextPt = 2;
                break;
            case kCubic_Verb:
                SkASSERT(moveCnt && !closeCount);
                ++segmentCount;
                nextPt = 3;
                break;
            case kClose_Verb:
                SkDEBUGCODE(++closeCount;)
                break;
            default:
                SkDEBUGFAIL("unknown verb");
        }
        if (-1 != nextPt) {
            if (SkPath::kConic_Verb == verb) {
                SkConic orig;
                orig.set(pts, iter.conicWeight());
                SkPoint quadPts[5];
                int count = orig.chopIntoQuadsPOW2(quadPts, 1);
                SkASSERT_RELEASE(2 == count);

                if (!check_edge_against_rect(quadPts[0], quadPts[2], rect, direction)) {
                    return false;
                }
                if (!check_edge_against_rect(quadPts[2], quadPts[4], rect, direction)) {
                    return false;
                }
            } else {
                if (!check_edge_against_rect(prevPt, pts[nextPt], rect, direction)) {
                    return false;
                }
            }
            prevPt = pts[nextPt];
        }
    }

    if (segmentCount) {
        return check_edge_against_rect(prevPt, firstPt, rect, direction);
    }
    return false;
}

uint32_t SkPath::getGenerationID() const {
    uint32_t genID = fPathRef->genID();
#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    SkASSERT((unsigned)fFillType < (1 << (32 - SkPathPriv::kPathRefGenIDBitCnt)));
    genID |= static_cast<uint32_t>(fFillType) << SkPathPriv::kPathRefGenIDBitCnt;
#endif
    return genID;
}

void SkPath::reset() {
    SkDEBUGCODE(this->validate();)

    fPathRef.reset(SkPathRef::CreateEmpty());
    this->resetFields();
}

void SkPath::rewind() {
    SkDEBUGCODE(this->validate();)

    SkPathRef::Rewind(&fPathRef);
    this->resetFields();
}

bool SkPath::isLastContourClosed() const {
    int verbCount = fPathRef->countVerbs();
    if (0 == verbCount) {
        return false;
    }
    return kClose_Verb == fPathRef->atVerb(verbCount - 1);
}

bool SkPath::isLine(SkPoint line[2]) const {
    int verbCount = fPathRef->countVerbs();

    if (2 == verbCount) {
        SkASSERT(kMove_Verb == fPathRef->atVerb(0));
        if (kLine_Verb == fPathRef->atVerb(1)) {
            SkASSERT(2 == fPathRef->countPoints());
            if (line) {
                const SkPoint* pts = fPathRef->points();
                line[0] = pts[0];
                line[1] = pts[1];
            }
            return true;
        }
    }
    return false;
}

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

 first,last,next direction state-machine:
    0x1 is set if the segment is horizontal
    0x2 is set if the segment is moving to the right or down
 thus:
    two directions are opposites iff (dirA ^ dirB) == 0x2
    two directions are perpendicular iff (dirA ^ dirB) == 0x1

 */
static int rect_make_dir(SkScalar dx, SkScalar dy) {
    return ((0 != dx) << 0) | ((dx > 0 || dy > 0) << 1);
}
bool SkPath::isRectContour(bool allowPartial, int* currVerb, const SkPoint** ptsPtr,
        bool* isClosed, Direction* direction) const {
    int corners = 0;
    SkPoint first, last;
    const SkPoint* pts = *ptsPtr;
    const SkPoint* savePts = nullptr;
    first.set(0, 0);
    last.set(0, 0);
    int firstDirection = 0;
    int lastDirection = 0;
    int nextDirection = 0;
    bool closedOrMoved = false;
    bool autoClose = false;
    bool insertClose = false;
    int verbCnt = fPathRef->countVerbs();
    while (*currVerb < verbCnt && (!allowPartial || !autoClose)) {
        uint8_t verb = insertClose ? (uint8_t) kClose_Verb : fPathRef->atVerb(*currVerb);
        switch (verb) {
            case kClose_Verb:
                savePts = pts;
                pts = *ptsPtr;
                autoClose = true;
                insertClose = false;
            case kLine_Verb: {
                SkScalar left = last.fX;
                SkScalar top = last.fY;
                SkScalar right = pts->fX;
                SkScalar bottom = pts->fY;
                ++pts;
                if (left != right && top != bottom) {
                    return false; // diagonal
                }
                if (left == right && top == bottom) {
                    break; // single point on side OK
                }
                nextDirection = rect_make_dir(right - left, bottom - top);
                if (0 == corners) {
                    firstDirection = nextDirection;
                    first = last;
                    last = pts[-1];
                    corners = 1;
                    closedOrMoved = false;
                    break;
                }
                if (closedOrMoved) {
                    return false; // closed followed by a line
                }
                if (autoClose && nextDirection == firstDirection) {
                    break; // colinear with first
                }
                closedOrMoved = autoClose;
                if (lastDirection != nextDirection) {
                    if (++corners > 4) {
                        return false; // too many direction changes
                    }
                }
                last = pts[-1];
                if (lastDirection == nextDirection) {
                    break; // colinear segment
                }
                // Possible values for corners are 2, 3, and 4.
                // When corners == 3, nextDirection opposes firstDirection.
                // Otherwise, nextDirection at corner 2 opposes corner 4.
                int turn = firstDirection ^ (corners - 1);
                int directionCycle = 3 == corners ? 0 : nextDirection ^ turn;
                if ((directionCycle ^ turn) != nextDirection) {
                    return false; // direction didn't follow cycle
                }
                break;
            }
            case kQuad_Verb:
            case kConic_Verb:
            case kCubic_Verb:
                return false; // quadratic, cubic not allowed
            case kMove_Verb:
                if (allowPartial && !autoClose && firstDirection) {
                    insertClose = true;
                    *currVerb -= 1;  // try move again afterwards
                    goto addMissingClose;
                }
                last = *pts++;
                closedOrMoved = true;
                break;
            default:
                SkDEBUGFAIL("unexpected verb");
                break;
        }
        *currVerb += 1;
        lastDirection = nextDirection;
addMissingClose:
        ;
    }
    // Success if 4 corners and first point equals last
    bool result = 4 == corners && (first == last || autoClose);
    if (!result) {
        // check if we are just an incomplete rectangle, in which case we can
        // return true, but not claim to be closed.
        // e.g.
        //    3 sided rectangle
        //    4 sided but the last edge is not long enough to reach the start
        //
        SkScalar closeX = first.x() - last.x();
        SkScalar closeY = first.y() - last.y();
        if (closeX && closeY) {
            return false;   // we're diagonal, abort (can we ever reach this?)
        }
        int closeDirection = rect_make_dir(closeX, closeY);
        // make sure the close-segment doesn't double-back on itself
        if (3 == corners || (4 == corners && closeDirection == lastDirection)) {
            result = true;
            autoClose = false;  // we are not closed
        }
    }
    if (savePts) {
        *ptsPtr = savePts;
    }
    if (result && isClosed) {
        *isClosed = autoClose;
    }
    if (result && direction) {
        *direction = firstDirection == ((lastDirection + 1) & 3) ? kCCW_Direction : kCW_Direction;
    }
    return result;
}

bool SkPath::isRect(SkRect* rect, bool* isClosed, Direction* direction) const {
    SkDEBUGCODE(this->validate();)
    int currVerb = 0;
    const SkPoint* pts = fPathRef->points();
    const SkPoint* first = pts;
    if (!this->isRectContour(false, &currVerb, &pts, isClosed, direction)) {
        return false;
    }
    if (rect) {
        int32_t num = SkToS32(pts - first);
        if (num) {
            rect->set(first, num);
        } else {
            // 'pts' isn't updated for open rects
            *rect = this->getBounds();
        }
    }
    return true;
}

bool SkPath::isNestedFillRects(SkRect rects[2], Direction dirs[2]) const {
    SkDEBUGCODE(this->validate();)
    int currVerb = 0;
    const SkPoint* pts = fPathRef->points();
    const SkPoint* first = pts;
    Direction testDirs[2];
    if (!isRectContour(true, &currVerb, &pts, nullptr, &testDirs[0])) {
        return false;
    }
    const SkPoint* last = pts;
    SkRect testRects[2];
    bool isClosed;
    if (isRectContour(false, &currVerb, &pts, &isClosed, &testDirs[1])) {
        testRects[0].set(first, SkToS32(last - first));
        if (!isClosed) {
            pts = fPathRef->points() + fPathRef->countPoints();
        }
        testRects[1].set(last, SkToS32(pts - last));
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

int SkPath::countPoints() const {
    return fPathRef->countPoints();
}

int SkPath::getPoints(SkPoint dst[], int max) const {
    SkDEBUGCODE(this->validate();)

    SkASSERT(max >= 0);
    SkASSERT(!max || dst);
    int count = SkMin32(max, fPathRef->countPoints());
    sk_careful_memcpy(dst, fPathRef->points(), count * sizeof(SkPoint));
    return fPathRef->countPoints();
}

SkPoint SkPath::getPoint(int index) const {
    if ((unsigned)index < (unsigned)fPathRef->countPoints()) {
        return fPathRef->atPoint(index);
    }
    return SkPoint::Make(0, 0);
}

int SkPath::countVerbs() const {
    return fPathRef->countVerbs();
}

static inline void copy_verbs_reverse(uint8_t* inorderDst,
                                      const uint8_t* reversedSrc,
                                      int count) {
    for (int i = 0; i < count; ++i) {
        inorderDst[i] = reversedSrc[~i];
    }
}

int SkPath::getVerbs(uint8_t dst[], int max) const {
    SkDEBUGCODE(this->validate();)

    SkASSERT(max >= 0);
    SkASSERT(!max || dst);
    int count = SkMin32(max, fPathRef->countVerbs());
    copy_verbs_reverse(dst, fPathRef->verbs(), count);
    return fPathRef->countVerbs();
}

bool SkPath::getLastPt(SkPoint* lastPt) const {
    SkDEBUGCODE(this->validate();)

    int count = fPathRef->countPoints();
    if (count > 0) {
        if (lastPt) {
            *lastPt = fPathRef->atPoint(count - 1);
        }
        return true;
    }
    if (lastPt) {
        lastPt->set(0, 0);
    }
    return false;
}

void SkPath::setPt(int index, SkScalar x, SkScalar y) {
    SkDEBUGCODE(this->validate();)

    int count = fPathRef->countPoints();
    if (count <= index) {
        return;
    } else {
        SkPathRef::Editor ed(&fPathRef);
        ed.atPoint(index)->set(x, y);
    }
}

void SkPath::setLastPt(SkScalar x, SkScalar y) {
    SkDEBUGCODE(this->validate();)

    int count = fPathRef->countPoints();
    if (count == 0) {
        this->moveTo(x, y);
    } else {
        SkPathRef::Editor ed(&fPathRef);
        ed.atPoint(count-1)->set(x, y);
    }
}

void SkPath::setConvexity(Convexity c) {
    if (fConvexity != c) {
        fConvexity = c;
    }
}

//////////////////////////////////////////////////////////////////////////////
//  Construction methods

#define DIRTY_AFTER_EDIT                                        \
    do {                                                        \
        fConvexity = kUnknown_Convexity;                        \
        fFirstDirection = SkPathPriv::kUnknown_FirstDirection;  \
    } while (0)

void SkPath::incReserve(U16CPU inc) {
    SkDEBUGCODE(this->validate();)
    SkPathRef::Editor(&fPathRef, inc, inc);
    SkDEBUGCODE(this->validate();)
}

void SkPath::moveTo(SkScalar x, SkScalar y) {
    SkDEBUGCODE(this->validate();)

    SkPathRef::Editor ed(&fPathRef);

    // remember our index
    fLastMoveToIndex = fPathRef->countPoints();

    ed.growForVerb(kMove_Verb)->set(x, y);

    DIRTY_AFTER_EDIT;
}

void SkPath::rMoveTo(SkScalar x, SkScalar y) {
    SkPoint pt;
    this->getLastPt(&pt);
    this->moveTo(pt.fX + x, pt.fY + y);
}

void SkPath::injectMoveToIfNeeded() {
    if (fLastMoveToIndex < 0) {
        SkScalar x, y;
        if (fPathRef->countVerbs() == 0) {
            x = y = 0;
        } else {
            const SkPoint& pt = fPathRef->atPoint(~fLastMoveToIndex);
            x = pt.fX;
            y = pt.fY;
        }
        this->moveTo(x, y);
    }
}

void SkPath::lineTo(SkScalar x, SkScalar y) {
    SkDEBUGCODE(this->validate();)

    this->injectMoveToIfNeeded();

    SkPathRef::Editor ed(&fPathRef);
    ed.growForVerb(kLine_Verb)->set(x, y);

    DIRTY_AFTER_EDIT;
}

void SkPath::rLineTo(SkScalar x, SkScalar y) {
    this->injectMoveToIfNeeded();  // This can change the result of this->getLastPt().
    SkPoint pt;
    this->getLastPt(&pt);
    this->lineTo(pt.fX + x, pt.fY + y);
}

void SkPath::quadTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2) {
    SkDEBUGCODE(this->validate();)

    this->injectMoveToIfNeeded();

    SkPathRef::Editor ed(&fPathRef);
    SkPoint* pts = ed.growForVerb(kQuad_Verb);
    pts[0].set(x1, y1);
    pts[1].set(x2, y2);

    DIRTY_AFTER_EDIT;
}

void SkPath::rQuadTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2) {
    this->injectMoveToIfNeeded();  // This can change the result of this->getLastPt().
    SkPoint pt;
    this->getLastPt(&pt);
    this->quadTo(pt.fX + x1, pt.fY + y1, pt.fX + x2, pt.fY + y2);
}

void SkPath::conicTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2,
                     SkScalar w) {
    // check for <= 0 or NaN with this test
    if (!(w > 0)) {
        this->lineTo(x2, y2);
    } else if (!SkScalarIsFinite(w)) {
        this->lineTo(x1, y1);
        this->lineTo(x2, y2);
    } else if (SK_Scalar1 == w) {
        this->quadTo(x1, y1, x2, y2);
    } else {
        SkDEBUGCODE(this->validate();)

        this->injectMoveToIfNeeded();

        SkPathRef::Editor ed(&fPathRef);
        SkPoint* pts = ed.growForVerb(kConic_Verb, w);
        pts[0].set(x1, y1);
        pts[1].set(x2, y2);

        DIRTY_AFTER_EDIT;
    }
}

void SkPath::rConicTo(SkScalar dx1, SkScalar dy1, SkScalar dx2, SkScalar dy2,
                      SkScalar w) {
    this->injectMoveToIfNeeded();  // This can change the result of this->getLastPt().
    SkPoint pt;
    this->getLastPt(&pt);
    this->conicTo(pt.fX + dx1, pt.fY + dy1, pt.fX + dx2, pt.fY + dy2, w);
}

void SkPath::cubicTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2,
                     SkScalar x3, SkScalar y3) {
    SkDEBUGCODE(this->validate();)

    this->injectMoveToIfNeeded();

    SkPathRef::Editor ed(&fPathRef);
    SkPoint* pts = ed.growForVerb(kCubic_Verb);
    pts[0].set(x1, y1);
    pts[1].set(x2, y2);
    pts[2].set(x3, y3);

    DIRTY_AFTER_EDIT;
}

void SkPath::rCubicTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2,
                      SkScalar x3, SkScalar y3) {
    this->injectMoveToIfNeeded();  // This can change the result of this->getLastPt().
    SkPoint pt;
    this->getLastPt(&pt);
    this->cubicTo(pt.fX + x1, pt.fY + y1, pt.fX + x2, pt.fY + y2,
                  pt.fX + x3, pt.fY + y3);
}

void SkPath::close() {
    SkDEBUGCODE(this->validate();)

    int count = fPathRef->countVerbs();
    if (count > 0) {
        switch (fPathRef->atVerb(count - 1)) {
            case kLine_Verb:
            case kQuad_Verb:
            case kConic_Verb:
            case kCubic_Verb:
            case kMove_Verb: {
                SkPathRef::Editor ed(&fPathRef);
                ed.growForVerb(kClose_Verb);
                break;
            }
            case kClose_Verb:
                // don't add a close if it's the first verb or a repeat
                break;
            default:
                SkDEBUGFAIL("unexpected verb");
                break;
        }
    }

    // signal that we need a moveTo to follow us (unless we're done)
#if 0
    if (fLastMoveToIndex >= 0) {
        fLastMoveToIndex = ~fLastMoveToIndex;
    }
#else
    fLastMoveToIndex ^= ~fLastMoveToIndex >> (8 * sizeof(fLastMoveToIndex) - 1);
#endif
}

///////////////////////////////////////////////////////////////////////////////

namespace {

template <unsigned N>
class PointIterator {
public:
    PointIterator(SkPath::Direction dir, unsigned startIndex)
        : fCurrent(startIndex % N)
        , fAdvance(dir == SkPath::kCW_Direction ? 1 : N - 1) { }

    const SkPoint& current() const {
        SkASSERT(fCurrent < N);
        return fPts[fCurrent];
    }

    const SkPoint& next() {
        fCurrent = (fCurrent + fAdvance) % N;
        return this->current();
    }

protected:
    SkPoint fPts[N];

private:
    unsigned fCurrent;
    unsigned fAdvance;
};

class RectPointIterator : public PointIterator<4> {
public:
    RectPointIterator(const SkRect& rect, SkPath::Direction dir, unsigned startIndex)
        : PointIterator(dir, startIndex) {

        fPts[0] = SkPoint::Make(rect.fLeft, rect.fTop);
        fPts[1] = SkPoint::Make(rect.fRight, rect.fTop);
        fPts[2] = SkPoint::Make(rect.fRight, rect.fBottom);
        fPts[3] = SkPoint::Make(rect.fLeft, rect.fBottom);
    }
};

class OvalPointIterator : public PointIterator<4> {
public:
    OvalPointIterator(const SkRect& oval, SkPath::Direction dir, unsigned startIndex)
        : PointIterator(dir, startIndex) {

        const SkScalar cx = oval.centerX();
        const SkScalar cy = oval.centerY();

        fPts[0] = SkPoint::Make(cx, oval.fTop);
        fPts[1] = SkPoint::Make(oval.fRight, cy);
        fPts[2] = SkPoint::Make(cx, oval.fBottom);
        fPts[3] = SkPoint::Make(oval.fLeft, cy);
    }
};

class RRectPointIterator : public PointIterator<8> {
public:
    RRectPointIterator(const SkRRect& rrect, SkPath::Direction dir, unsigned startIndex)
        : PointIterator(dir, startIndex) {

        const SkRect& bounds = rrect.getBounds();
        const SkScalar L = bounds.fLeft;
        const SkScalar T = bounds.fTop;
        const SkScalar R = bounds.fRight;
        const SkScalar B = bounds.fBottom;

        fPts[0] = SkPoint::Make(L + rrect.radii(SkRRect::kUpperLeft_Corner).fX, T);
        fPts[1] = SkPoint::Make(R - rrect.radii(SkRRect::kUpperRight_Corner).fX, T);
        fPts[2] = SkPoint::Make(R, T + rrect.radii(SkRRect::kUpperRight_Corner).fY);
        fPts[3] = SkPoint::Make(R, B - rrect.radii(SkRRect::kLowerRight_Corner).fY);
        fPts[4] = SkPoint::Make(R - rrect.radii(SkRRect::kLowerRight_Corner).fX, B);
        fPts[5] = SkPoint::Make(L + rrect.radii(SkRRect::kLowerLeft_Corner).fX, B);
        fPts[6] = SkPoint::Make(L, B - rrect.radii(SkRRect::kLowerLeft_Corner).fY);
        fPts[7] = SkPoint::Make(L, T + rrect.radii(SkRRect::kUpperLeft_Corner).fY);
    }
};

} // anonymous namespace

static void assert_known_direction(int dir) {
    SkASSERT(SkPath::kCW_Direction == dir || SkPath::kCCW_Direction == dir);
}

void SkPath::addRect(const SkRect& rect, Direction dir) {
    this->addRect(rect, dir, 0);
}

void SkPath::addRect(SkScalar left, SkScalar top, SkScalar right,
                     SkScalar bottom, Direction dir) {
    this->addRect(SkRect::MakeLTRB(left, top, right, bottom), dir, 0);
}

void SkPath::addRect(const SkRect &rect, Direction dir, unsigned startIndex) {
    assert_known_direction(dir);
    fFirstDirection = this->hasOnlyMoveTos() ?
        (SkPathPriv::FirstDirection)dir : SkPathPriv::kUnknown_FirstDirection;
    SkAutoDisableDirectionCheck addc(this);
    SkAutoPathBoundsUpdate apbu(this, rect);

    SkDEBUGCODE(int initialVerbCount = this->countVerbs());

    const int kVerbs = 5; // moveTo + 3x lineTo + close
    this->incReserve(kVerbs);

    RectPointIterator iter(rect, dir, startIndex);

    this->moveTo(iter.current());
    this->lineTo(iter.next());
    this->lineTo(iter.next());
    this->lineTo(iter.next());
    this->close();

    SkASSERT(this->countVerbs() == initialVerbCount + kVerbs);
}

void SkPath::addPoly(const SkPoint pts[], int count, bool close) {
    SkDEBUGCODE(this->validate();)
    if (count <= 0) {
        return;
    }

    fLastMoveToIndex = fPathRef->countPoints();

    // +close makes room for the extra kClose_Verb
    SkPathRef::Editor ed(&fPathRef, count+close, count);

    ed.growForVerb(kMove_Verb)->set(pts[0].fX, pts[0].fY);
    if (count > 1) {
        SkPoint* p = ed.growForRepeatedVerb(kLine_Verb, count - 1);
        memcpy(p, &pts[1], (count-1) * sizeof(SkPoint));
    }

    if (close) {
        ed.growForVerb(kClose_Verb);
        fLastMoveToIndex ^= ~fLastMoveToIndex >> (8 * sizeof(fLastMoveToIndex) - 1);
    }

    DIRTY_AFTER_EDIT;
    SkDEBUGCODE(this->validate();)
}

#include "SkGeometry.h"

static bool arc_is_lone_point(const SkRect& oval, SkScalar startAngle, SkScalar sweepAngle,
                              SkPoint* pt) {
    if (0 == sweepAngle && (0 == startAngle || SkIntToScalar(360) == startAngle)) {
        // Chrome uses this path to move into and out of ovals. If not
        // treated as a special case the moves can distort the oval's
        // bounding box (and break the circle special case).
        pt->set(oval.fRight, oval.centerY());
        return true;
    } else if (0 == oval.width() && 0 == oval.height()) {
        // Chrome will sometimes create 0 radius round rects. Having degenerate
        // quad segments in the path prevents the path from being recognized as
        // a rect.
        // TODO: optimizing the case where only one of width or height is zero
        // should also be considered. This case, however, doesn't seem to be
        // as common as the single point case.
        pt->set(oval.fRight, oval.fTop);
        return true;
    }
    return false;
}

// Return the unit vectors pointing at the start/stop points for the given start/sweep angles
//
static void angles_to_unit_vectors(SkScalar startAngle, SkScalar sweepAngle,
                                   SkVector* startV, SkVector* stopV, SkRotationDirection* dir) {
    startV->fY = SkScalarSinCos(SkDegreesToRadians(startAngle), &startV->fX);
    stopV->fY = SkScalarSinCos(SkDegreesToRadians(startAngle + sweepAngle), &stopV->fX);

    /*  If the sweep angle is nearly (but less than) 360, then due to precision
     loss in radians-conversion and/or sin/cos, we may end up with coincident
     vectors, which will fool SkBuildQuadArc into doing nothing (bad) instead
     of drawing a nearly complete circle (good).
     e.g. canvas.drawArc(0, 359.99, ...)
     -vs- canvas.drawArc(0, 359.9, ...)
     We try to detect this edge case, and tweak the stop vector
     */
    if (*startV == *stopV) {
        SkScalar sw = SkScalarAbs(sweepAngle);
        if (sw < SkIntToScalar(360) && sw > SkIntToScalar(359)) {
            SkScalar stopRad = SkDegreesToRadians(startAngle + sweepAngle);
            // make a guess at a tiny angle (in radians) to tweak by
            SkScalar deltaRad = SkScalarCopySign(SK_Scalar1/512, sweepAngle);
            // not sure how much will be enough, so we use a loop
            do {
                stopRad -= deltaRad;
                stopV->fY = SkScalarSinCos(stopRad, &stopV->fX);
            } while (*startV == *stopV);
        }
    }
    *dir = sweepAngle > 0 ? kCW_SkRotationDirection : kCCW_SkRotationDirection;
}

/**
 *  If this returns 0, then the caller should just line-to the singlePt, else it should
 *  ignore singlePt and append the specified number of conics.
 */
static int build_arc_conics(const SkRect& oval, const SkVector& start, const SkVector& stop,
                            SkRotationDirection dir, SkConic conics[SkConic::kMaxConicsForArc],
                            SkPoint* singlePt) {
    SkMatrix    matrix;

    matrix.setScale(SkScalarHalf(oval.width()), SkScalarHalf(oval.height()));
    matrix.postTranslate(oval.centerX(), oval.centerY());

    int count = SkConic::BuildUnitArc(start, stop, dir, &matrix, conics);
    if (0 == count) {
        matrix.mapXY(stop.x(), stop.y(), singlePt);
    }
    return count;
}

void SkPath::addRoundRect(const SkRect& rect, const SkScalar radii[],
                          Direction dir) {
    SkRRect rrect;
    rrect.setRectRadii(rect, (const SkVector*) radii);
    this->addRRect(rrect, dir);
}

void SkPath::addRRect(const SkRRect& rrect, Direction dir) {
    // legacy start indices: 6 (CW) and 7(CCW)
    this->addRRect(rrect, dir, dir == kCW_Direction ? 6 : 7);
}

void SkPath::addRRect(const SkRRect &rrect, Direction dir, unsigned startIndex) {
        assert_known_direction(dir);

        bool isRRect = hasOnlyMoveTos();
        const SkRect& bounds = rrect.getBounds();

        if (rrect.isRect() || rrect.isEmpty()) {
            // degenerate(rect) => radii points are collapsing
            this->addRect(bounds, dir, (startIndex + 1) / 2);
        } else if (rrect.isOval()) {
            // degenerate(oval) => line points are collapsing
            this->addOval(bounds, dir, startIndex / 2);
        } else {
            fFirstDirection = this->hasOnlyMoveTos() ?
                                (SkPathPriv::FirstDirection)dir : SkPathPriv::kUnknown_FirstDirection;

            SkAutoPathBoundsUpdate apbu(this, bounds);
            SkAutoDisableDirectionCheck addc(this);

            // we start with a conic on odd indices when moving CW vs. even indices when moving CCW
            const bool startsWithConic = ((startIndex & 1) == (dir == kCW_Direction));
            const SkScalar weight = SK_ScalarRoot2Over2;

            SkDEBUGCODE(int initialVerbCount = this->countVerbs());
            const int kVerbs = startsWithConic
                ? 9   // moveTo + 4x conicTo + 3x lineTo + close
                : 10; // moveTo + 4x lineTo + 4x conicTo + close
            this->incReserve(kVerbs);

            RRectPointIterator rrectIter(rrect, dir, startIndex);
            // Corner iterator indices follow the collapsed radii model,
            // adjusted such that the start pt is "behind" the radii start pt.
            const unsigned rectStartIndex = startIndex / 2 + (dir == kCW_Direction ? 0 : 1);
            RectPointIterator rectIter(bounds, dir, rectStartIndex);

            this->moveTo(rrectIter.current());
            if (startsWithConic) {
                for (unsigned i = 0; i < 3; ++i) {
                    this->conicTo(rectIter.next(), rrectIter.next(), weight);
                    this->lineTo(rrectIter.next());
                }
                this->conicTo(rectIter.next(), rrectIter.next(), weight);
                // final lineTo handled by close().
            } else {
                for (unsigned i = 0; i < 4; ++i) {
                    this->lineTo(rrectIter.next());
                    this->conicTo(rectIter.next(), rrectIter.next(), weight);
                }
            }
            this->close();

            SkPathRef::Editor ed(&fPathRef);
            ed.setIsRRect(isRRect, dir, startIndex % 8);

            SkASSERT(this->countVerbs() == initialVerbCount + kVerbs);
        }

        SkDEBUGCODE(fPathRef->validate();)
}

bool SkPath::hasOnlyMoveTos() const {
    int count = fPathRef->countVerbs();
    const uint8_t* verbs = const_cast<const SkPathRef*>(fPathRef.get())->verbsMemBegin();
    for (int i = 0; i < count; ++i) {
        if (*verbs == kLine_Verb ||
            *verbs == kQuad_Verb ||
            *verbs == kConic_Verb ||
            *verbs == kCubic_Verb) {
            return false;
        }
        ++verbs;
    }
    return true;
}

bool SkPath::isZeroLengthSincePoint(int startPtIndex) const {
    int count = fPathRef->countPoints() - startPtIndex;
    if (count < 2) {
        return true;
    }
    const SkPoint* pts = fPathRef.get()->points() + startPtIndex;
    const SkPoint& first = *pts;
    for (int index = 1; index < count; ++index) {
        if (first != pts[index]) {
            return false;
        }
    }
    return true;
}

void SkPath::addRoundRect(const SkRect& rect, SkScalar rx, SkScalar ry,
                          Direction dir) {
    assert_known_direction(dir);

    if (rx < 0 || ry < 0) {
        return;
    }

    SkRRect rrect;
    rrect.setRectXY(rect, rx, ry);
    this->addRRect(rrect, dir);
}

void SkPath::addOval(const SkRect& oval, Direction dir) {
    // legacy start index: 1
    this->addOval(oval, dir, 1);
}

void SkPath::addOval(const SkRect &oval, Direction dir, unsigned startPointIndex) {
    assert_known_direction(dir);

    /* If addOval() is called after previous moveTo(),
       this path is still marked as an oval. This is used to
       fit into WebKit's calling sequences.
       We can't simply check isEmpty() in this case, as additional
       moveTo() would mark the path non empty.
     */
    bool isOval = hasOnlyMoveTos();
    if (isOval) {
        fFirstDirection = (SkPathPriv::FirstDirection)dir;
    } else {
        fFirstDirection = SkPathPriv::kUnknown_FirstDirection;
    }

    SkAutoDisableDirectionCheck addc(this);
    SkAutoPathBoundsUpdate apbu(this, oval);

    SkDEBUGCODE(int initialVerbCount = this->countVerbs());
    const int kVerbs = 6; // moveTo + 4x conicTo + close
    this->incReserve(kVerbs);

    OvalPointIterator ovalIter(oval, dir, startPointIndex);
    // The corner iterator pts are tracking "behind" the oval/radii pts.
    RectPointIterator rectIter(oval, dir, startPointIndex + (dir == kCW_Direction ? 0 : 1));
    const SkScalar weight = SK_ScalarRoot2Over2;

    this->moveTo(ovalIter.current());
    for (unsigned i = 0; i < 4; ++i) {
        this->conicTo(rectIter.next(), ovalIter.next(), weight);
    }
    this->close();

    SkASSERT(this->countVerbs() == initialVerbCount + kVerbs);

    SkPathRef::Editor ed(&fPathRef);

    ed.setIsOval(isOval, kCCW_Direction == dir, startPointIndex % 4);
}

void SkPath::addCircle(SkScalar x, SkScalar y, SkScalar r, Direction dir) {
    if (r > 0) {
        this->addOval(SkRect::MakeLTRB(x - r, y - r, x + r, y + r), dir);
    }
}

void SkPath::arcTo(const SkRect& oval, SkScalar startAngle, SkScalar sweepAngle,
                   bool forceMoveTo) {
    if (oval.width() < 0 || oval.height() < 0) {
        return;
    }

    if (fPathRef->countVerbs() == 0) {
        forceMoveTo = true;
    }

    SkPoint lonePt;
    if (arc_is_lone_point(oval, startAngle, sweepAngle, &lonePt)) {
        forceMoveTo ? this->moveTo(lonePt) : this->lineTo(lonePt);
        return;
    }

    SkVector startV, stopV;
    SkRotationDirection dir;
    angles_to_unit_vectors(startAngle, sweepAngle, &startV, &stopV, &dir);

    SkPoint singlePt;

    // At this point, we know that the arc is not a lone point, but startV == stopV
    // indicates that the sweepAngle is too small such that angles_to_unit_vectors
    // cannot handle it.
    if (startV == stopV) {
        SkScalar endAngle = SkDegreesToRadians(startAngle + sweepAngle);
        SkScalar radiusX = oval.width() / 2;
        SkScalar radiusY = oval.height() / 2;
        // We cannot use SkScalarSinCos function in the next line because
        // SkScalarSinCos has a threshold *SkScalarNearlyZero*. When sin(startAngle)
        // is 0 and sweepAngle is very small and radius is huge, the expected
        // behavior here is to draw a line. But calling SkScalarSinCos will
        // make sin(endAngle) to be 0 which will then draw a dot.
        singlePt.set(oval.centerX() + radiusX * sk_float_cos(endAngle),
            oval.centerY() + radiusY * sk_float_sin(endAngle));
        forceMoveTo ? this->moveTo(singlePt) : this->lineTo(singlePt);
        return;
    }

    SkConic conics[SkConic::kMaxConicsForArc];
    int count = build_arc_conics(oval, startV, stopV, dir, conics, &singlePt);
    if (count) {
        this->incReserve(count * 2 + 1);
        const SkPoint& pt = conics[0].fPts[0];
        forceMoveTo ? this->moveTo(pt) : this->lineTo(pt);
        for (int i = 0; i < count; ++i) {
            this->conicTo(conics[i].fPts[1], conics[i].fPts[2], conics[i].fW);
        }
    } else {
        forceMoveTo ? this->moveTo(singlePt) : this->lineTo(singlePt);
    }
}

// This converts the SVG arc to conics.
// Partly adapted from Niko's code in kdelibs/kdecore/svgicons.
// Then transcribed from webkit/chrome's SVGPathNormalizer::decomposeArcToCubic()
// See also SVG implementation notes:
// http://www.w3.org/TR/SVG/implnote.html#ArcConversionEndpointToCenter
// Note that arcSweep bool value is flipped from the original implementation.
void SkPath::arcTo(SkScalar rx, SkScalar ry, SkScalar angle, SkPath::ArcSize arcLarge,
                   SkPath::Direction arcSweep, SkScalar x, SkScalar y) {
    this->injectMoveToIfNeeded();
    SkPoint srcPts[2];
    this->getLastPt(&srcPts[0]);
    // If rx = 0 or ry = 0 then this arc is treated as a straight line segment (a "lineto")
    // joining the endpoints.
    // http://www.w3.org/TR/SVG/implnote.html#ArcOutOfRangeParameters
    if (!rx || !ry) {
        this->lineTo(x, y);
        return;
    }
    // If the current point and target point for the arc are identical, it should be treated as a
    // zero length path. This ensures continuity in animations.
    srcPts[1].set(x, y);
    if (srcPts[0] == srcPts[1]) {
        this->lineTo(x, y);
        return;
    }
    rx = SkScalarAbs(rx);
    ry = SkScalarAbs(ry);
    SkVector midPointDistance = srcPts[0] - srcPts[1];
    midPointDistance *= 0.5f;

    SkMatrix pointTransform;
    pointTransform.setRotate(-angle);

    SkPoint transformedMidPoint;
    pointTransform.mapPoints(&transformedMidPoint, &midPointDistance, 1);
    SkScalar squareRx = rx * rx;
    SkScalar squareRy = ry * ry;
    SkScalar squareX = transformedMidPoint.fX * transformedMidPoint.fX;
    SkScalar squareY = transformedMidPoint.fY * transformedMidPoint.fY;

    // Check if the radii are big enough to draw the arc, scale radii if not.
    // http://www.w3.org/TR/SVG/implnote.html#ArcCorrectionOutOfRangeRadii
    SkScalar radiiScale = squareX / squareRx + squareY / squareRy;
    if (radiiScale > 1) {
        radiiScale = SkScalarSqrt(radiiScale);
        rx *= radiiScale;
        ry *= radiiScale;
    }

    pointTransform.setScale(1 / rx, 1 / ry);
    pointTransform.preRotate(-angle);

    SkPoint unitPts[2];
    pointTransform.mapPoints(unitPts, srcPts, (int) SK_ARRAY_COUNT(unitPts));
    SkVector delta = unitPts[1] - unitPts[0];

    SkScalar d = delta.fX * delta.fX + delta.fY * delta.fY;
    SkScalar scaleFactorSquared = SkTMax(1 / d - 0.25f, 0.f);

    SkScalar scaleFactor = SkScalarSqrt(scaleFactorSquared);
    if (SkToBool(arcSweep) != SkToBool(arcLarge)) {  // flipped from the original implementation
        scaleFactor = -scaleFactor;
    }
    delta.scale(scaleFactor);
    SkPoint centerPoint = unitPts[0] + unitPts[1];
    centerPoint *= 0.5f;
    centerPoint.offset(-delta.fY, delta.fX);
    unitPts[0] -= centerPoint;
    unitPts[1] -= centerPoint;
    SkScalar theta1 = SkScalarATan2(unitPts[0].fY, unitPts[0].fX);
    SkScalar theta2 = SkScalarATan2(unitPts[1].fY, unitPts[1].fX);
    SkScalar thetaArc = theta2 - theta1;
    if (thetaArc < 0 && !arcSweep) {  // arcSweep flipped from the original implementation
        thetaArc += SK_ScalarPI * 2;
    } else if (thetaArc > 0 && arcSweep) {  // arcSweep flipped from the original implementation
        thetaArc -= SK_ScalarPI * 2;
    }
    pointTransform.setRotate(angle);
    pointTransform.preScale(rx, ry);

#ifdef SK_SUPPORT_LEGACY_SVG_ARC_TO
    int segments = SkScalarCeilToInt(SkScalarAbs(thetaArc / (SK_ScalarPI / 2)));
#else
    // the arc may be slightly bigger than 1/4 circle, so allow up to 1/3rd
    int segments = SkScalarCeilToInt(SkScalarAbs(thetaArc / (2 * SK_ScalarPI / 3)));
#endif
    SkScalar thetaWidth = thetaArc / segments;
    SkScalar t = SkScalarTan(0.5f * thetaWidth);
    if (!SkScalarIsFinite(t)) {
        return;
    }
    SkScalar startTheta = theta1;
    SkScalar w = SkScalarSqrt(SK_ScalarHalf + SkScalarCos(thetaWidth) * SK_ScalarHalf);
#ifndef SK_SUPPORT_LEGACY_SVG_ARC_TO
    auto scalar_is_integer = [](SkScalar scalar) -> bool {
        return scalar == SkScalarFloorToScalar(scalar);
    };
    bool expectIntegers = SkScalarNearlyZero(SK_ScalarPI/2 - SkScalarAbs(thetaWidth)) &&
        scalar_is_integer(rx) && scalar_is_integer(ry) &&
        scalar_is_integer(x) && scalar_is_integer(y);
#endif
    for (int i = 0; i < segments; ++i) {
        SkScalar endTheta = startTheta + thetaWidth;
        SkScalar cosEndTheta, sinEndTheta = SkScalarSinCos(endTheta, &cosEndTheta);

        unitPts[1].set(cosEndTheta, sinEndTheta);
        unitPts[1] += centerPoint;
        unitPts[0] = unitPts[1];
        unitPts[0].offset(t * sinEndTheta, -t * cosEndTheta);
        SkPoint mapped[2];
        pointTransform.mapPoints(mapped, unitPts, (int) SK_ARRAY_COUNT(unitPts));
        /*
        Computing the arc width introduces rounding errors that cause arcs to start
        outside their marks. A round rect may lose convexity as a result. If the input
        values are on integers, place the conic on integers as well.
         */
#ifndef SK_SUPPORT_LEGACY_SVG_ARC_TO
        if (expectIntegers) {
            SkScalar* mappedScalars = &mapped[0].fX;
            for (unsigned index = 0; index < sizeof(mapped) / sizeof(SkScalar); ++index) {
                mappedScalars[index] = SkScalarRoundToScalar(mappedScalars[index]);
            }
        }
#endif
        this->conicTo(mapped[0], mapped[1], w);
        startTheta = endTheta;
    }
}

void SkPath::rArcTo(SkScalar rx, SkScalar ry, SkScalar xAxisRotate, SkPath::ArcSize largeArc,
                    SkPath::Direction sweep, SkScalar dx, SkScalar dy) {
    SkPoint currentPoint;
    this->getLastPt(&currentPoint);
    this->arcTo(rx, ry, xAxisRotate, largeArc, sweep, currentPoint.fX + dx, currentPoint.fY + dy);
}

void SkPath::addArc(const SkRect& oval, SkScalar startAngle, SkScalar sweepAngle) {
    if (oval.isEmpty() || 0 == sweepAngle) {
        return;
    }

    const SkScalar kFullCircleAngle = SkIntToScalar(360);

    if (sweepAngle >= kFullCircleAngle || sweepAngle <= -kFullCircleAngle) {
        // We can treat the arc as an oval if it begins at one of our legal starting positions.
        // See SkPath::addOval() docs.
        SkScalar startOver90 = startAngle / 90.f;
        SkScalar startOver90I = SkScalarRoundToScalar(startOver90);
        SkScalar error = startOver90 - startOver90I;
        if (SkScalarNearlyEqual(error, 0)) {
            // Index 1 is at startAngle == 0.
            SkScalar startIndex = std::fmod(startOver90I + 1.f, 4.f);
            startIndex = startIndex < 0 ? startIndex + 4.f : startIndex;
            this->addOval(oval, sweepAngle > 0 ? kCW_Direction : kCCW_Direction,
                          (unsigned) startIndex);
            return;
        }
    }
    this->arcTo(oval, startAngle, sweepAngle, true);
}

/*
    Need to handle the case when the angle is sharp, and our computed end-points
    for the arc go behind pt1 and/or p2...
*/
void SkPath::arcTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2, SkScalar radius) {
    if (radius == 0) {
        this->lineTo(x1, y1);
        return;
    }

    SkVector before, after;

    // need to know our prev pt so we can construct tangent vectors
    {
        SkPoint start;
        this->getLastPt(&start);
        // Handle degenerate cases by adding a line to the first point and
        // bailing out.
        before.setNormalize(x1 - start.fX, y1 - start.fY);
        after.setNormalize(x2 - x1, y2 - y1);
    }

    SkScalar cosh = SkPoint::DotProduct(before, after);
    SkScalar sinh = SkPoint::CrossProduct(before, after);

    if (SkScalarNearlyZero(sinh)) {   // angle is too tight
        this->lineTo(x1, y1);
        return;
    }

    SkScalar dist = SkScalarAbs(radius * (1 - cosh) / sinh);

    SkScalar xx = x1 - dist * before.fX;
    SkScalar yy = y1 - dist * before.fY;
    after.setLength(dist);
    this->lineTo(xx, yy);
    SkScalar weight = SkScalarSqrt(SK_ScalarHalf + cosh * SK_ScalarHalf);
    this->conicTo(x1, y1, x1 + after.fX, y1 + after.fY, weight);
}

///////////////////////////////////////////////////////////////////////////////

void SkPath::addPath(const SkPath& path, SkScalar dx, SkScalar dy, AddPathMode mode) {
    SkMatrix matrix;

    matrix.setTranslate(dx, dy);
    this->addPath(path, matrix, mode);
}

void SkPath::addPath(const SkPath& path, const SkMatrix& matrix, AddPathMode mode) {
    SkPathRef::Editor(&fPathRef, path.countVerbs(), path.countPoints());

    RawIter iter(path);
    SkPoint pts[4];
    Verb    verb;

    SkMatrixPriv::MapPtsProc proc = SkMatrixPriv::GetMapPtsProc(matrix);
    bool firstVerb = true;
    while ((verb = iter.next(pts)) != kDone_Verb) {
        switch (verb) {
            case kMove_Verb:
                proc(matrix, &pts[0], &pts[0], 1);
                if (firstVerb && mode == kExtend_AddPathMode && !isEmpty()) {
                    injectMoveToIfNeeded(); // In case last contour is closed
                    this->lineTo(pts[0]);
                } else {
                    this->moveTo(pts[0]);
                }
                break;
            case kLine_Verb:
                proc(matrix, &pts[1], &pts[1], 1);
                this->lineTo(pts[1]);
                break;
            case kQuad_Verb:
                proc(matrix, &pts[1], &pts[1], 2);
                this->quadTo(pts[1], pts[2]);
                break;
            case kConic_Verb:
                proc(matrix, &pts[1], &pts[1], 2);
                this->conicTo(pts[1], pts[2], iter.conicWeight());
                break;
            case kCubic_Verb:
                proc(matrix, &pts[1], &pts[1], 3);
                this->cubicTo(pts[1], pts[2], pts[3]);
                break;
            case kClose_Verb:
                this->close();
                break;
            default:
                SkDEBUGFAIL("unknown verb");
        }
        firstVerb = false;
    }
}

///////////////////////////////////////////////////////////////////////////////

static int pts_in_verb(unsigned verb) {
    static const uint8_t gPtsInVerb[] = {
        1,  // kMove
        1,  // kLine
        2,  // kQuad
        2,  // kConic
        3,  // kCubic
        0,  // kClose
        0   // kDone
    };

    SkASSERT(verb < SK_ARRAY_COUNT(gPtsInVerb));
    return gPtsInVerb[verb];
}

// ignore the last point of the 1st contour
void SkPath::reversePathTo(const SkPath& path) {
    const uint8_t* verbs = path.fPathRef->verbsMemBegin(); // points at the last verb
    if (!verbs) {  // empty path returns nullptr
        return;
    }
    const uint8_t* verbsEnd = path.fPathRef->verbs() - 1; // points just past the first verb
    SkASSERT(verbsEnd[0] == kMove_Verb);
    const SkPoint*  pts = path.fPathRef->pointsEnd() - 1;
    const SkScalar* conicWeights = path.fPathRef->conicWeightsEnd();

    while (verbs < verbsEnd) {
        uint8_t v = *verbs++;
        pts -= pts_in_verb(v);
        switch (v) {
            case kMove_Verb:
                // if the path has multiple contours, stop after reversing the last
                return;
            case kLine_Verb:
                this->lineTo(pts[0]);
                break;
            case kQuad_Verb:
                this->quadTo(pts[1], pts[0]);
                break;
            case kConic_Verb:
                this->conicTo(pts[1], pts[0], *--conicWeights);
                break;
            case kCubic_Verb:
                this->cubicTo(pts[2], pts[1], pts[0]);
                break;
            case kClose_Verb:
                SkASSERT(verbs - path.fPathRef->verbsMemBegin() == 1);
                break;
            default:
                SkDEBUGFAIL("bad verb");
                break;
        }
    }
}

void SkPath::reverseAddPath(const SkPath& src) {
    SkPathRef::Editor ed(&fPathRef, src.fPathRef->countPoints(), src.fPathRef->countVerbs());

    const SkPoint* pts = src.fPathRef->pointsEnd();
    // we will iterator through src's verbs backwards
    const uint8_t* verbs = src.fPathRef->verbsMemBegin(); // points at the last verb
    const uint8_t* verbsEnd = src.fPathRef->verbs(); // points just past the first verb
    const SkScalar* conicWeights = src.fPathRef->conicWeightsEnd();

    bool needMove = true;
    bool needClose = false;
    while (verbs < verbsEnd) {
        uint8_t v = *(verbs++);
        int n = pts_in_verb(v);

        if (needMove) {
            --pts;
            this->moveTo(pts->fX, pts->fY);
            needMove = false;
        }
        pts -= n;
        switch (v) {
            case kMove_Verb:
                if (needClose) {
                    this->close();
                    needClose = false;
                }
                needMove = true;
                pts += 1;   // so we see the point in "if (needMove)" above
                break;
            case kLine_Verb:
                this->lineTo(pts[0]);
                break;
            case kQuad_Verb:
                this->quadTo(pts[1], pts[0]);
                break;
            case kConic_Verb:
                this->conicTo(pts[1], pts[0], *--conicWeights);
                break;
            case kCubic_Verb:
                this->cubicTo(pts[2], pts[1], pts[0]);
                break;
            case kClose_Verb:
                needClose = true;
                break;
            default:
                SkDEBUGFAIL("unexpected verb");
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void SkPath::offset(SkScalar dx, SkScalar dy, SkPath* dst) const {
    SkMatrix    matrix;

    matrix.setTranslate(dx, dy);
    this->transform(matrix, dst);
}

static void subdivide_cubic_to(SkPath* path, const SkPoint pts[4],
                               int level = 2) {
    if (--level >= 0) {
        SkPoint tmp[7];

        SkChopCubicAtHalf(pts, tmp);
        subdivide_cubic_to(path, &tmp[0], level);
        subdivide_cubic_to(path, &tmp[3], level);
    } else {
        path->cubicTo(pts[1], pts[2], pts[3]);
    }
}

void SkPath::transform(const SkMatrix& matrix, SkPath* dst) const {
    SkDEBUGCODE(this->validate();)
    if (dst == nullptr) {
        dst = (SkPath*)this;
    }

    if (matrix.hasPerspective()) {
        SkPath  tmp;
        tmp.fFillType = fFillType;

        SkPath::Iter    iter(*this, false);
        SkPoint         pts[4];
        SkPath::Verb    verb;

        while ((verb = iter.next(pts, false)) != kDone_Verb) {
            switch (verb) {
                case kMove_Verb:
                    tmp.moveTo(pts[0]);
                    break;
                case kLine_Verb:
                    tmp.lineTo(pts[1]);
                    break;
                case kQuad_Verb:
                    // promote the quad to a conic
                    tmp.conicTo(pts[1], pts[2],
                                SkConic::TransformW(pts, SK_Scalar1, matrix));
                    break;
                case kConic_Verb:
                    tmp.conicTo(pts[1], pts[2],
                                SkConic::TransformW(pts, iter.conicWeight(), matrix));
                    break;
                case kCubic_Verb:
                    subdivide_cubic_to(&tmp, pts);
                    break;
                case kClose_Verb:
                    tmp.close();
                    break;
                default:
                    SkDEBUGFAIL("unknown verb");
                    break;
            }
        }

        dst->swap(tmp);
        SkPathRef::Editor ed(&dst->fPathRef);
        matrix.mapPoints(ed.points(), ed.pathRef()->countPoints());
        dst->fFirstDirection = SkPathPriv::kUnknown_FirstDirection;
    } else {
        SkPathRef::CreateTransformedCopy(&dst->fPathRef, *fPathRef.get(), matrix);

        if (this != dst) {
            dst->fFillType = fFillType;
            dst->fConvexity.store(fConvexity);
            dst->fIsVolatile = fIsVolatile;
        }

        if (SkPathPriv::kUnknown_FirstDirection == fFirstDirection) {
            dst->fFirstDirection = SkPathPriv::kUnknown_FirstDirection;
        } else {
            SkScalar det2x2 =
                matrix.get(SkMatrix::kMScaleX) * matrix.get(SkMatrix::kMScaleY) -
                matrix.get(SkMatrix::kMSkewX)  * matrix.get(SkMatrix::kMSkewY);
            if (det2x2 < 0) {
                dst->fFirstDirection = SkPathPriv::OppositeFirstDirection(
                        (SkPathPriv::FirstDirection)fFirstDirection.load());
            } else if (det2x2 > 0) {
                dst->fFirstDirection = fFirstDirection.load();
            } else {
                dst->fConvexity = kUnknown_Convexity;
                dst->fFirstDirection = SkPathPriv::kUnknown_FirstDirection;
            }
        }

        SkDEBUGCODE(dst->validate();)
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

enum SegmentState {
    kEmptyContour_SegmentState,   // The current contour is empty. We may be
                                  // starting processing or we may have just
                                  // closed a contour.
    kAfterMove_SegmentState,      // We have seen a move, but nothing else.
    kAfterPrimitive_SegmentState  // We have seen a primitive but not yet
                                  // closed the path. Also the initial state.
};

SkPath::Iter::Iter() {
#ifdef SK_DEBUG
    fPts = nullptr;
    fConicWeights = nullptr;
    fMoveTo.fX = fMoveTo.fY = fLastPt.fX = fLastPt.fY = 0;
    fForceClose = fCloseLine = false;
    fSegmentState = kEmptyContour_SegmentState;
#endif
    // need to init enough to make next() harmlessly return kDone_Verb
    fVerbs = nullptr;
    fVerbStop = nullptr;
    fNeedClose = false;
}

SkPath::Iter::Iter(const SkPath& path, bool forceClose) {
    this->setPath(path, forceClose);
}

void SkPath::Iter::setPath(const SkPath& path, bool forceClose) {
    fPts = path.fPathRef->points();
    fVerbs = path.fPathRef->verbs();
    fVerbStop = path.fPathRef->verbsMemBegin();
    fConicWeights = path.fPathRef->conicWeights();
    if (fConicWeights) {
      fConicWeights -= 1;  // begin one behind
    }
    fLastPt.fX = fLastPt.fY = 0;
    fMoveTo.fX = fMoveTo.fY = 0;
    fForceClose = SkToU8(forceClose);
    fNeedClose = false;
    fSegmentState = kEmptyContour_SegmentState;
}

bool SkPath::Iter::isClosedContour() const {
    if (fVerbs == nullptr || fVerbs == fVerbStop) {
        return false;
    }
    if (fForceClose) {
        return true;
    }

    const uint8_t* verbs = fVerbs;
    const uint8_t* stop = fVerbStop;

    if (kMove_Verb == *(verbs - 1)) {
        verbs -= 1; // skip the initial moveto
    }

    while (verbs > stop) {
        // verbs points one beyond the current verb, decrement first.
        unsigned v = *(--verbs);
        if (kMove_Verb == v) {
            break;
        }
        if (kClose_Verb == v) {
            return true;
        }
    }
    return false;
}

SkPath::Verb SkPath::Iter::autoClose(SkPoint pts[2]) {
    SkASSERT(pts);
    if (fLastPt != fMoveTo) {
        // A special case: if both points are NaN, SkPoint::operation== returns
        // false, but the iterator expects that they are treated as the same.
        // (consider SkPoint is a 2-dimension float point).
        if (SkScalarIsNaN(fLastPt.fX) || SkScalarIsNaN(fLastPt.fY) ||
            SkScalarIsNaN(fMoveTo.fX) || SkScalarIsNaN(fMoveTo.fY)) {
            return kClose_Verb;
        }

        pts[0] = fLastPt;
        pts[1] = fMoveTo;
        fLastPt = fMoveTo;
        fCloseLine = true;
        return kLine_Verb;
    } else {
        pts[0] = fMoveTo;
        return kClose_Verb;
    }
}

const SkPoint& SkPath::Iter::cons_moveTo() {
    if (fSegmentState == kAfterMove_SegmentState) {
        // Set the first return pt to the move pt
        fSegmentState = kAfterPrimitive_SegmentState;
        return fMoveTo;
    } else {
        SkASSERT(fSegmentState == kAfterPrimitive_SegmentState);
         // Set the first return pt to the last pt of the previous primitive.
        return fPts[-1];
    }
}

void SkPath::Iter::consumeDegenerateSegments(bool exact) {
    // We need to step over anything that will not move the current draw point
    // forward before the next move is seen
    const uint8_t* lastMoveVerb = nullptr;
    const SkPoint* lastMovePt = nullptr;
    const SkScalar* lastMoveWeight = nullptr;
    SkPoint lastPt = fLastPt;
    while (fVerbs != fVerbStop) {
        unsigned verb = *(fVerbs - 1); // fVerbs is one beyond the current verb
        switch (verb) {
            case kMove_Verb:
                // Keep a record of this most recent move
                lastMoveVerb = fVerbs;
                lastMovePt = fPts;
                lastMoveWeight = fConicWeights;
                lastPt = fPts[0];
                fVerbs--;
                fPts++;
                break;

            case kClose_Verb:
                // A close when we are in a segment is always valid except when it
                // follows a move which follows a segment.
                if (fSegmentState == kAfterPrimitive_SegmentState && !lastMoveVerb) {
                    return;
                }
                // A close at any other time must be ignored
                fVerbs--;
                break;

            case kLine_Verb:
                if (!IsLineDegenerate(lastPt, fPts[0], exact)) {
                    if (lastMoveVerb) {
                        fVerbs = lastMoveVerb;
                        fPts = lastMovePt;
                        fConicWeights = lastMoveWeight;
                        return;
                    }
                    return;
                }
                // Ignore this line and continue
                fVerbs--;
                fPts++;
                break;

            case kConic_Verb:
            case kQuad_Verb:
                if (!IsQuadDegenerate(lastPt, fPts[0], fPts[1], exact)) {
                    if (lastMoveVerb) {
                        fVerbs = lastMoveVerb;
                        fPts = lastMovePt;
                        fConicWeights = lastMoveWeight;
                        return;
                    }
                    return;
                }
                // Ignore this line and continue
                fVerbs--;
                fPts += 2;
                fConicWeights += (kConic_Verb == verb);
                break;

            case kCubic_Verb:
                if (!IsCubicDegenerate(lastPt, fPts[0], fPts[1], fPts[2], exact)) {
                    if (lastMoveVerb) {
                        fVerbs = lastMoveVerb;
                        fPts = lastMovePt;
                        fConicWeights = lastMoveWeight;
                        return;
                    }
                    return;
                }
                // Ignore this line and continue
                fVerbs--;
                fPts += 3;
                break;

            default:
                SkDEBUGFAIL("Should never see kDone_Verb");
        }
    }
}

SkPath::Verb SkPath::Iter::doNext(SkPoint ptsParam[4]) {
    SkASSERT(ptsParam);

    if (fVerbs == fVerbStop) {
        // Close the curve if requested and if there is some curve to close
        if (fNeedClose && fSegmentState == kAfterPrimitive_SegmentState) {
            if (kLine_Verb == this->autoClose(ptsParam)) {
                return kLine_Verb;
            }
            fNeedClose = false;
            return kClose_Verb;
        }
        return kDone_Verb;
    }

    // fVerbs is one beyond the current verb, decrement first
    unsigned verb = *(--fVerbs);
    const SkPoint* SK_RESTRICT srcPts = fPts;
    SkPoint* SK_RESTRICT       pts = ptsParam;

    switch (verb) {
        case kMove_Verb:
            if (fNeedClose) {
                fVerbs++; // move back one verb
                verb = this->autoClose(pts);
                if (verb == kClose_Verb) {
                    fNeedClose = false;
                }
                return (Verb)verb;
            }
            if (fVerbs == fVerbStop) {    // might be a trailing moveto
                return kDone_Verb;
            }
            fMoveTo = *srcPts;
            pts[0] = *srcPts;
            srcPts += 1;
            fSegmentState = kAfterMove_SegmentState;
            fLastPt = fMoveTo;
            fNeedClose = fForceClose;
            break;
        case kLine_Verb:
            pts[0] = this->cons_moveTo();
            pts[1] = srcPts[0];
            fLastPt = srcPts[0];
            fCloseLine = false;
            srcPts += 1;
            break;
        case kConic_Verb:
            fConicWeights += 1;
            // fall-through
        case kQuad_Verb:
            pts[0] = this->cons_moveTo();
            memcpy(&pts[1], srcPts, 2 * sizeof(SkPoint));
            fLastPt = srcPts[1];
            srcPts += 2;
            break;
        case kCubic_Verb:
            pts[0] = this->cons_moveTo();
            memcpy(&pts[1], srcPts, 3 * sizeof(SkPoint));
            fLastPt = srcPts[2];
            srcPts += 3;
            break;
        case kClose_Verb:
            verb = this->autoClose(pts);
            if (verb == kLine_Verb) {
                fVerbs++; // move back one verb
            } else {
                fNeedClose = false;
                fSegmentState = kEmptyContour_SegmentState;
            }
            fLastPt = fMoveTo;
            break;
    }
    fPts = srcPts;
    return (Verb)verb;
}

///////////////////////////////////////////////////////////////////////////////

/*
    Format in compressed buffer: [ptCount, verbCount, pts[], verbs[]]
*/

size_t SkPath::writeToMemoryAsRRect(int32_t packedHeader, void* storage) const {
    SkRect oval;
    SkRRect rrect;
    bool isCCW;
    unsigned start;
    if (fPathRef->isOval(&oval, &isCCW, &start)) {
        rrect.setOval(oval);
        // Convert to rrect start indices.
        start *= 2;
    } else if (!fPathRef->isRRect(&rrect, &isCCW, &start)) {
        return false;
    }
    if (!storage) {
        // packed header, rrect, start index.
        return sizeof(int32_t) + SkRRect::kSizeInMemory + sizeof(int32_t);
    }

    SkWBuffer buffer(storage);
    // Rewrite header's first direction based on rrect direction.
    uint8_t firstDir = isCCW ? SkPathPriv::kCCW_FirstDirection : SkPathPriv::kCW_FirstDirection;
    packedHeader &= ~(0x3 << kDirection_SerializationShift);
    packedHeader |= firstDir << kDirection_SerializationShift;
    packedHeader |= SerializationType::kRRect << kType_SerializationShift;
    buffer.write32(packedHeader);
    rrect.writeToBuffer(&buffer);
    buffer.write32(SkToS32(start));
    buffer.padToAlign4();
    return buffer.pos();
}

size_t SkPath::writeToMemory(void* storage) const {
    SkDEBUGCODE(this->validate();)

    int32_t packed = (fConvexity << kConvexity_SerializationShift) |
                     (fFillType << kFillType_SerializationShift) |
                     (fFirstDirection << kDirection_SerializationShift) |
                     (fIsVolatile << kIsVolatile_SerializationShift) |
                     kCurrent_Version;
    if (size_t bytes = this->writeToMemoryAsRRect(packed, storage)) {
        return bytes;
    }

    SkWBuffer   buffer(storage);

    static_assert(0 == SerializationType::kGeneral, "packed has zero in type bits");
    if (nullptr == storage) {
        // packed header, pathref, start index
        const int byteCount = sizeof(int32_t) * 2 + fPathRef->writeSize();
        return SkAlign4(byteCount);
    }
    buffer.write32(packed);
    buffer.write32(fLastMoveToIndex);

    fPathRef->writeToBuffer(&buffer);

    buffer.padToAlign4();
    return buffer.pos();
}

sk_sp<SkData> SkPath::serialize() const {
    size_t size = this->writeToMemory(nullptr);
    sk_sp<SkData> data = SkData::MakeUninitialized(size);
    this->writeToMemory(data->writable_data());
    return data;
}

size_t SkPath::readFromMemory(const void* storage, size_t length) {
    SkRBuffer buffer(storage, length);

    int32_t packed;
    if (!buffer.readS32(&packed)) {
        return 0;
    }

    unsigned version = packed & 0xFF;
    uint8_t dir = (packed >> kDirection_SerializationShift) & 0x3;
    FillType fillType = static_cast<FillType>((packed >> kFillType_SerializationShift) & 0x3);
    if (version >= kPathPrivTypeEnumVersion) {
        SerializationType type =
                static_cast<SerializationType>((packed >> kType_SerializationShift) & 0xF);
        switch (type) {
            case SerializationType::kRRect: {
                Direction rrectDir;
                SkRRect rrect;
                int32_t start;
                switch (dir) {
                    case SkPathPriv::kCW_FirstDirection:
                        rrectDir = kCW_Direction;
                        break;
                    case SkPathPriv::kCCW_FirstDirection:
                        rrectDir = kCCW_Direction;
                        break;
                    default:
                        return 0;
                }
                if (!rrect.readFromBuffer(&buffer)) {
                    return 0;
                }
                if (!buffer.readS32(&start) || start != SkTPin(start, 0, 7)) {
                    return 0;
                }
                this->reset();
                this->addRRect(rrect, rrectDir, SkToUInt(start));
                this->setFillType(fillType);
                buffer.skipToAlign4();
                return buffer.pos();
            }
            case SerializationType::kGeneral:
                // Fall through to general path deserialization
                break;
            default:
                return 0;
        }
    }
    if (version >= kPathPrivLastMoveToIndex_Version && !buffer.readS32(&fLastMoveToIndex)) {
        return 0;
    }

    // These are written into the serialized data but we no longer use them in the deserialized
    // path. If convexity is corrupted it may cause the GPU backend to make incorrect
    // rendering choices, possibly crashing. We set them to unknown so that they'll be recomputed if
    // requested.
    fConvexity = kUnknown_Convexity;
    fFirstDirection = SkPathPriv::kUnknown_FirstDirection;

    fFillType = fillType;
    fIsVolatile = (packed >> kIsVolatile_SerializationShift) & 0x1;
    SkPathRef* pathRef = SkPathRef::CreateFromBuffer(&buffer);
    if (!pathRef) {
        return 0;
    }

    fPathRef.reset(pathRef);
    SkDEBUGCODE(this->validate();)
    buffer.skipToAlign4();
    return buffer.pos();
}

///////////////////////////////////////////////////////////////////////////////

#include "SkString.h"
#include "SkStringUtils.h"
#include "SkStream.h"

static void append_params(SkString* str, const char label[], const SkPoint pts[],
                          int count, SkScalarAsStringType strType, SkScalar conicWeight = -12345) {
    str->append(label);
    str->append("(");

    const SkScalar* values = &pts[0].fX;
    count *= 2;

    for (int i = 0; i < count; ++i) {
        SkAppendScalar(str, values[i], strType);
        if (i < count - 1) {
            str->append(", ");
        }
    }
    if (conicWeight != -12345) {
        str->append(", ");
        SkAppendScalar(str, conicWeight, strType);
    }
    str->append(");");
    if (kHex_SkScalarAsStringType == strType) {
        str->append("  // ");
        for (int i = 0; i < count; ++i) {
            SkAppendScalarDec(str, values[i]);
            if (i < count - 1) {
                str->append(", ");
            }
        }
        if (conicWeight >= 0) {
            str->append(", ");
            SkAppendScalarDec(str, conicWeight);
        }
    }
    str->append("\n");
}

void SkPath::dump(SkWStream* wStream, bool forceClose, bool dumpAsHex) const {
    SkScalarAsStringType asType = dumpAsHex ? kHex_SkScalarAsStringType : kDec_SkScalarAsStringType;
    Iter    iter(*this, forceClose);
    SkPoint pts[4];
    Verb    verb;

    SkString builder;
    char const * const gFillTypeStrs[] = {
        "Winding",
        "EvenOdd",
        "InverseWinding",
        "InverseEvenOdd",
    };
    builder.printf("path.setFillType(SkPath::k%s_FillType);\n",
            gFillTypeStrs[(int) this->getFillType()]);
    while ((verb = iter.next(pts, false)) != kDone_Verb) {
        switch (verb) {
            case kMove_Verb:
                append_params(&builder, "path.moveTo", &pts[0], 1, asType);
                break;
            case kLine_Verb:
                append_params(&builder, "path.lineTo", &pts[1], 1, asType);
                break;
            case kQuad_Verb:
                append_params(&builder, "path.quadTo", &pts[1], 2, asType);
                break;
            case kConic_Verb:
                append_params(&builder, "path.conicTo", &pts[1], 2, asType, iter.conicWeight());
                break;
            case kCubic_Verb:
                append_params(&builder, "path.cubicTo", &pts[1], 3, asType);
                break;
            case kClose_Verb:
                builder.append("path.close();\n");
                break;
            default:
                SkDebugf("  path: UNKNOWN VERB %d, aborting dump...\n", verb);
                verb = kDone_Verb;  // stop the loop
                break;
        }
        if (!wStream && builder.size()) {
            SkDebugf("%s", builder.c_str());
            builder.reset();
        }
    }
    if (wStream) {
        wStream->writeText(builder.c_str());
    }
}

void SkPath::dump() const {
    this->dump(nullptr, false, false);
}

void SkPath::dumpHex() const {
    this->dump(nullptr, false, true);
}


bool SkPath::isValidImpl() const {
    if ((fFillType & ~3) != 0) {
        return false;
    }

#ifdef SK_DEBUG_PATH
    if (!fBoundsIsDirty) {
        SkRect bounds;

        bool isFinite = compute_pt_bounds(&bounds, *fPathRef.get());
        if (SkToBool(fIsFinite) != isFinite) {
            return false;
        }

        if (fPathRef->countPoints() <= 1) {
            // if we're empty, fBounds may be empty but translated, so we can't
            // necessarily compare to bounds directly
            // try path.addOval(2, 2, 2, 2) which is empty, but the bounds will
            // be [2, 2, 2, 2]
            if (!bounds.isEmpty() || !fBounds.isEmpty()) {
                return false;
            }
        } else {
            if (bounds.isEmpty()) {
                if (!fBounds.isEmpty()) {
                    return false;
                }
            } else {
                if (!fBounds.isEmpty()) {
                    if (!fBounds.contains(bounds)) {
                        return false;
                    }
                }
            }
        }
    }
#endif // SK_DEBUG_PATH
    return true;
}

///////////////////////////////////////////////////////////////////////////////

static int sign(SkScalar x) { return x < 0; }
#define kValueNeverReturnedBySign   2

enum DirChange {
    kLeft_DirChange,
    kRight_DirChange,
    kStraight_DirChange,
    kBackwards_DirChange,

    kInvalid_DirChange
};


static bool almost_equal(SkScalar compA, SkScalar compB) {
    // The error epsilon was empirically derived; worse case round rects
    // with a mid point outset by 2x float epsilon in tests had an error
    // of 12.
    const int epsilon = 16;
    if (!SkScalarIsFinite(compA) || !SkScalarIsFinite(compB)) {
        return false;
    }
    // no need to check for small numbers because SkPath::Iter has removed degenerate values
    int aBits = SkFloatAs2sCompliment(compA);
    int bBits = SkFloatAs2sCompliment(compB);
    return aBits < bBits + epsilon && bBits < aBits + epsilon;
}

static bool approximately_zero_when_compared_to(double x, double y) {
    return x == 0 || fabs(x) < fabs(y * FLT_EPSILON);
}


// only valid for a single contour
struct Convexicator {
    Convexicator()
    : fPtCount(0)
    , fConvexity(SkPath::kConvex_Convexity)
    , fFirstDirection(SkPathPriv::kUnknown_FirstDirection)
    , fIsFinite(true)
    , fIsCurve(false)
    , fBackwards(false) {
        fExpectedDir = kInvalid_DirChange;
        // warnings
        fPriorPt.set(0,0);
        fLastPt.set(0, 0);
        fCurrPt.set(0, 0);
        fLastVec.set(0, 0);
        fFirstVec.set(0, 0);

        fDx = fDy = 0;
        fSx = fSy = kValueNeverReturnedBySign;
    }

    SkPath::Convexity getConvexity() const { return fConvexity; }

    /** The direction returned is only valid if the path is determined convex */
    SkPathPriv::FirstDirection getFirstDirection() const { return fFirstDirection; }

    void addPt(const SkPoint& pt) {
        if (SkPath::kConcave_Convexity == fConvexity || !fIsFinite) {
            return;
        }

        if (0 == fPtCount) {
            fCurrPt = pt;
            ++fPtCount;
        } else {
            SkVector vec = pt - fCurrPt;
            SkScalar lengthSqd = SkPointPriv::LengthSqd(vec);
            if (!SkScalarIsFinite(lengthSqd)) {
                fIsFinite = false;
            } else if (lengthSqd) {
                fPriorPt = fLastPt;
                fLastPt = fCurrPt;
                fCurrPt = pt;
                if (++fPtCount == 2) {
                    fFirstVec = fLastVec = vec;
                } else {
                    SkASSERT(fPtCount > 2);
                    this->addVec(vec);
                }

                int sx = sign(vec.fX);
                int sy = sign(vec.fY);
                fDx += (sx != fSx);
                fDy += (sy != fSy);
                fSx = sx;
                fSy = sy;

                if (fDx > 3 || fDy > 3) {
                    fConvexity = SkPath::kConcave_Convexity;
                }
            }
        }
    }

    void close() {
        if (fPtCount > 2) {
            this->addVec(fFirstVec);
        }
    }

    DirChange directionChange(const SkVector& curVec) {
        SkScalar cross = SkPoint::CrossProduct(fLastVec, curVec);

        SkScalar smallest = SkTMin(fCurrPt.fX, SkTMin(fCurrPt.fY, SkTMin(fLastPt.fX, fLastPt.fY)));
        SkScalar largest = SkTMax(fCurrPt.fX, SkTMax(fCurrPt.fY, SkTMax(fLastPt.fX, fLastPt.fY)));
        largest = SkTMax(largest, -smallest);

        if (!almost_equal(largest, largest + cross)) {
            int sign = SkScalarSignAsInt(cross);
            if (sign) {
                return (1 == sign) ? kRight_DirChange : kLeft_DirChange;
            }
        }

        if (cross) {
            double dLastVecX = SkScalarToDouble(fLastPt.fX) - SkScalarToDouble(fPriorPt.fX);
            double dLastVecY = SkScalarToDouble(fLastPt.fY) - SkScalarToDouble(fPriorPt.fY);
            double dCurrVecX = SkScalarToDouble(fCurrPt.fX) - SkScalarToDouble(fLastPt.fX);
            double dCurrVecY = SkScalarToDouble(fCurrPt.fY) - SkScalarToDouble(fLastPt.fY);
            double dCross = dLastVecX * dCurrVecY - dLastVecY * dCurrVecX;
            if (!approximately_zero_when_compared_to(dCross, SkScalarToDouble(largest))) {
                int sign = SkScalarSignAsInt(SkDoubleToScalar(dCross));
                if (sign) {
                    return (1 == sign) ? kRight_DirChange : kLeft_DirChange;
                }
            }
        }

        if (!SkScalarNearlyZero(SkPointPriv::LengthSqd(fLastVec),
                                SK_ScalarNearlyZero*SK_ScalarNearlyZero) &&
            !SkScalarNearlyZero(SkPointPriv::LengthSqd(curVec),
                                SK_ScalarNearlyZero*SK_ScalarNearlyZero) &&
            fLastVec.dot(curVec) < 0.0f) {
            return kBackwards_DirChange;
        }

        return kStraight_DirChange;
    }

    bool hasBackwards() const {
        return fBackwards;
    }

    bool isFinite() const {
        return fIsFinite;
    }

    void setCurve(bool isCurve) {
        fIsCurve = isCurve;
    }

private:
    void addVec(const SkVector& vec) {
        SkASSERT(vec.fX || vec.fY);
        DirChange dir = this->directionChange(vec);
        switch (dir) {
            case kLeft_DirChange:       // fall through
            case kRight_DirChange:
                if (kInvalid_DirChange == fExpectedDir) {
                    fExpectedDir = dir;
                    fFirstDirection = (kRight_DirChange == dir) ? SkPathPriv::kCW_FirstDirection
                                                                : SkPathPriv::kCCW_FirstDirection;
                } else if (dir != fExpectedDir) {
                    fConvexity = SkPath::kConcave_Convexity;
                    fFirstDirection = SkPathPriv::kUnknown_FirstDirection;
                }
                fLastVec = vec;
                break;
            case kStraight_DirChange:
                break;
            case kBackwards_DirChange:
                if (fIsCurve) {
                    // If any of the subsequent dir is non-backward, it'll be concave.
                    // Otherwise, it's still convex.
                    fExpectedDir = dir;
                }
                fLastVec = vec;
                fBackwards = true;
                break;
            case kInvalid_DirChange:
                SK_ABORT("Use of invalid direction change flag");
                break;
        }
    }

    SkPoint             fPriorPt;
    SkPoint             fLastPt;
    SkPoint             fCurrPt;
    // fLastVec does not necessarily start at fLastPt. We only advance it when the cross product
    // value with the current vec is deemed to be of a significant value.
    SkVector            fLastVec, fFirstVec;
    int                 fPtCount;   // non-degenerate points
    DirChange           fExpectedDir;
    SkPath::Convexity   fConvexity;
    SkPathPriv::FirstDirection   fFirstDirection;
    int                 fDx, fDy, fSx, fSy;
    bool                fIsFinite;
    bool                fIsCurve;
    bool                fBackwards;
};

SkPath::Convexity SkPath::internalGetConvexity() const {
    SkASSERT(kUnknown_Convexity == fConvexity);
    SkPoint         pts[4];
    SkPath::Verb    verb;
    SkPath::Iter    iter(*this, true);

    int             contourCount = 0;
    int             count;
    Convexicator    state;

    if (!isFinite()) {
        return kUnknown_Convexity;
    }
    while ((verb = iter.next(pts, false, false)) != SkPath::kDone_Verb) {
        switch (verb) {
            case kMove_Verb:
                if (++contourCount > 1) {
                    fConvexity = kConcave_Convexity;
                    return kConcave_Convexity;
                }
                pts[1] = pts[0];
                // fall through
            case kLine_Verb:
                count = 1;
                state.setCurve(false);
                break;
            case kQuad_Verb:
                // fall through
            case kConic_Verb:
                // fall through
            case kCubic_Verb:
                count = 2 + (kCubic_Verb == verb);
                // As an additional enhancement, this could set curve true only
                // if the curve is nonlinear
                state.setCurve(true);
                break;
            case kClose_Verb:
                state.setCurve(false);
                state.close();
                count = 0;
                break;
            default:
                SkDEBUGFAIL("bad verb");
                fConvexity = kConcave_Convexity;
                return kConcave_Convexity;
        }

        for (int i = 1; i <= count; i++) {
            state.addPt(pts[i]);
        }
        // early exit
        if (!state.isFinite()) {
            return kUnknown_Convexity;
        }
        if (kConcave_Convexity == state.getConvexity()) {
            fConvexity = kConcave_Convexity;
            return kConcave_Convexity;
        }
    }
    fConvexity = state.getConvexity();
    if (kConvex_Convexity == fConvexity && SkPathPriv::kUnknown_FirstDirection == fFirstDirection) {
        if (SkPathPriv::kUnknown_FirstDirection == state.getFirstDirection() &&
                !this->getBounds().isEmpty() && !state.hasBackwards()) {
            fConvexity = Convexity::kConcave_Convexity;
        } else {
            fFirstDirection = state.getFirstDirection();
        }
    }
    return static_cast<Convexity>(fConvexity);
}

///////////////////////////////////////////////////////////////////////////////

class ContourIter {
public:
    ContourIter(const SkPathRef& pathRef);

    bool done() const { return fDone; }
    // if !done() then these may be called
    int count() const { return fCurrPtCount; }
    const SkPoint* pts() const { return fCurrPt; }
    void next();

private:
    int fCurrPtCount;
    const SkPoint* fCurrPt;
    const uint8_t* fCurrVerb;
    const uint8_t* fStopVerbs;
    const SkScalar* fCurrConicWeight;
    bool fDone;
    SkDEBUGCODE(int fContourCounter;)
};

ContourIter::ContourIter(const SkPathRef& pathRef) {
    fStopVerbs = pathRef.verbsMemBegin();
    fDone = false;
    fCurrPt = pathRef.points();
    fCurrVerb = pathRef.verbs();
    fCurrConicWeight = pathRef.conicWeights();
    fCurrPtCount = 0;
    SkDEBUGCODE(fContourCounter = 0;)
    this->next();
}

void ContourIter::next() {
    if (fCurrVerb <= fStopVerbs) {
        fDone = true;
    }
    if (fDone) {
        return;
    }

    // skip pts of prev contour
    fCurrPt += fCurrPtCount;

    SkASSERT(SkPath::kMove_Verb == fCurrVerb[~0]);
    int ptCount = 1;    // moveTo
    const uint8_t* verbs = fCurrVerb;

    for (--verbs; verbs > fStopVerbs; --verbs) {
        switch (verbs[~0]) {
            case SkPath::kMove_Verb:
                goto CONTOUR_END;
            case SkPath::kLine_Verb:
                ptCount += 1;
                break;
            case SkPath::kConic_Verb:
                fCurrConicWeight += 1;
                // fall-through
            case SkPath::kQuad_Verb:
                ptCount += 2;
                break;
            case SkPath::kCubic_Verb:
                ptCount += 3;
                break;
            case SkPath::kClose_Verb:
                break;
            default:
                SkDEBUGFAIL("unexpected verb");
                break;
        }
    }
CONTOUR_END:
    fCurrPtCount = ptCount;
    fCurrVerb = verbs;
    SkDEBUGCODE(++fContourCounter;)
}

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

static void crossToDir(SkScalar cross, SkPathPriv::FirstDirection* dir) {
    *dir = cross > 0 ? SkPathPriv::kCW_FirstDirection : SkPathPriv::kCCW_FirstDirection;
}

/*
 *  We loop through all contours, and keep the computed cross-product of the
 *  contour that contained the global y-max. If we just look at the first
 *  contour, we may find one that is wound the opposite way (correctly) since
 *  it is the interior of a hole (e.g. 'o'). Thus we must find the contour
 *  that is outer most (or at least has the global y-max) before we can consider
 *  its cross product.
 */
bool SkPathPriv::CheapComputeFirstDirection(const SkPath& path, FirstDirection* dir) {
    if (kUnknown_FirstDirection != path.fFirstDirection.load()) {
        *dir = static_cast<FirstDirection>(path.fFirstDirection.load());
        return true;
    }

    // don't want to pay the cost for computing this if it
    // is unknown, so we don't call isConvex()
    if (SkPath::kConvex_Convexity == path.getConvexityOrUnknown()) {
        SkASSERT(kUnknown_FirstDirection == path.fFirstDirection);
        *dir = static_cast<FirstDirection>(path.fFirstDirection.load());
        return false;
    }

    ContourIter iter(*path.fPathRef.get());

    // initialize with our logical y-min
    SkScalar ymax = path.getBounds().fTop;
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
    if (ymaxCross) {
        crossToDir(ymaxCross, dir);
        path.fFirstDirection = *dir;
        return true;
    } else {
        return false;
    }
}

///////////////////////////////////////////////////////////////////////////////

static bool between(SkScalar a, SkScalar b, SkScalar c) {
    SkASSERT(((a <= b && b <= c) || (a >= b && b >= c)) == ((a - b) * (c - b) <= 0)
            || (SkScalarNearlyZero(a) && SkScalarNearlyZero(b) && SkScalarNearlyZero(c)));
    return (a - b) * (c - b) <= 0;
}

static SkScalar eval_cubic_pts(SkScalar c0, SkScalar c1, SkScalar c2, SkScalar c3,
                               SkScalar t) {
    SkScalar A = c3 + 3*(c1 - c2) - c0;
    SkScalar B = 3*(c2 - c1 - c1 + c0);
    SkScalar C = 3*(c1 - c0);
    SkScalar D = c0;
    return poly_eval(A, B, C, D, t);
}

template <size_t N> static void find_minmax(const SkPoint pts[],
                                            SkScalar* minPtr, SkScalar* maxPtr) {
    SkScalar min, max;
    min = max = pts[0].fX;
    for (size_t i = 1; i < N; ++i) {
        min = SkMinScalar(min, pts[i].fX);
        max = SkMaxScalar(max, pts[i].fX);
    }
    *minPtr = min;
    *maxPtr = max;
}

static bool checkOnCurve(SkScalar x, SkScalar y, const SkPoint& start, const SkPoint& end) {
    if (start.fY == end.fY) {
        return between(start.fX, x, end.fX) && x != end.fX;
    } else {
        return x == start.fX && y == start.fY;
    }
}

static int winding_mono_cubic(const SkPoint pts[], SkScalar x, SkScalar y, int* onCurveCount) {
    SkScalar y0 = pts[0].fY;
    SkScalar y3 = pts[3].fY;

    int dir = 1;
    if (y0 > y3) {
        SkTSwap(y0, y3);
        dir = -1;
    }
    if (y < y0 || y > y3) {
        return 0;
    }
    if (checkOnCurve(x, y, pts[0], pts[3])) {
        *onCurveCount += 1;
        return 0;
    }
    if (y == y3) {
        return 0;
    }

    // quickreject or quickaccept
    SkScalar min, max;
    find_minmax<4>(pts, &min, &max);
    if (x < min) {
        return 0;
    }
    if (x > max) {
        return dir;
    }

    // compute the actual x(t) value
    SkScalar t;
    if (!SkCubicClipper::ChopMonoAtY(pts, y, &t)) {
        return 0;
    }
    SkScalar xt = eval_cubic_pts(pts[0].fX, pts[1].fX, pts[2].fX, pts[3].fX, t);
    if (SkScalarNearlyEqual(xt, x)) {
        if (x != pts[3].fX || y != pts[3].fY) {  // don't test end points; they're start points
            *onCurveCount += 1;
            return 0;
        }
    }
    return xt < x ? dir : 0;
}

static int winding_cubic(const SkPoint pts[], SkScalar x, SkScalar y, int* onCurveCount) {
    SkPoint dst[10];
    int n = SkChopCubicAtYExtrema(pts, dst);
    int w = 0;
    for (int i = 0; i <= n; ++i) {
        w += winding_mono_cubic(&dst[i * 3], x, y, onCurveCount);
    }
    return w;
}

static double conic_eval_numerator(const SkScalar src[], SkScalar w, SkScalar t) {
    SkASSERT(src);
    SkASSERT(t >= 0 && t <= 1);
    SkScalar src2w = src[2] * w;
    SkScalar C = src[0];
    SkScalar A = src[4] - 2 * src2w + C;
    SkScalar B = 2 * (src2w - C);
    return poly_eval(A, B, C, t);
}


static double conic_eval_denominator(SkScalar w, SkScalar t) {
    SkScalar B = 2 * (w - 1);
    SkScalar C = 1;
    SkScalar A = -B;
    return poly_eval(A, B, C, t);
}

static int winding_mono_conic(const SkConic& conic, SkScalar x, SkScalar y, int* onCurveCount) {
    const SkPoint* pts = conic.fPts;
    SkScalar y0 = pts[0].fY;
    SkScalar y2 = pts[2].fY;

    int dir = 1;
    if (y0 > y2) {
        SkTSwap(y0, y2);
        dir = -1;
    }
    if (y < y0 || y > y2) {
        return 0;
    }
    if (checkOnCurve(x, y, pts[0], pts[2])) {
        *onCurveCount += 1;
        return 0;
    }
    if (y == y2) {
        return 0;
    }

    SkScalar roots[2];
    SkScalar A = pts[2].fY;
    SkScalar B = pts[1].fY * conic.fW - y * conic.fW + y;
    SkScalar C = pts[0].fY;
    A += C - 2 * B;  // A = a + c - 2*(b*w - yCept*w + yCept)
    B -= C;  // B = b*w - w * yCept + yCept - a
    C -= y;
    int n = SkFindUnitQuadRoots(A, 2 * B, C, roots);
    SkASSERT(n <= 1);
    SkScalar xt;
    if (0 == n) {
        // zero roots are returned only when y0 == y
        // Need [0] if dir == 1
        // and  [2] if dir == -1
        xt = pts[1 - dir].fX;
    } else {
        SkScalar t = roots[0];
        xt = conic_eval_numerator(&pts[0].fX, conic.fW, t) / conic_eval_denominator(conic.fW, t);
    }
    if (SkScalarNearlyEqual(xt, x)) {
        if (x != pts[2].fX || y != pts[2].fY) {  // don't test end points; they're start points
            *onCurveCount += 1;
            return 0;
        }
    }
    return xt < x ? dir : 0;
}

static bool is_mono_quad(SkScalar y0, SkScalar y1, SkScalar y2) {
    //    return SkScalarSignAsInt(y0 - y1) + SkScalarSignAsInt(y1 - y2) != 0;
    if (y0 == y1) {
        return true;
    }
    if (y0 < y1) {
        return y1 <= y2;
    } else {
        return y1 >= y2;
    }
}

static int winding_conic(const SkPoint pts[], SkScalar x, SkScalar y, SkScalar weight,
                         int* onCurveCount) {
    SkConic conic(pts, weight);
    SkConic chopped[2];
    // If the data points are very large, the conic may not be monotonic but may also
    // fail to chop. Then, the chopper does not split the original conic in two.
    bool isMono = is_mono_quad(pts[0].fY, pts[1].fY, pts[2].fY) || !conic.chopAtYExtrema(chopped);
    int w = winding_mono_conic(isMono ? conic : chopped[0], x, y, onCurveCount);
    if (!isMono) {
        w += winding_mono_conic(chopped[1], x, y, onCurveCount);
    }
    return w;
}

static int winding_mono_quad(const SkPoint pts[], SkScalar x, SkScalar y, int* onCurveCount) {
    SkScalar y0 = pts[0].fY;
    SkScalar y2 = pts[2].fY;

    int dir = 1;
    if (y0 > y2) {
        SkTSwap(y0, y2);
        dir = -1;
    }
    if (y < y0 || y > y2) {
        return 0;
    }
    if (checkOnCurve(x, y, pts[0], pts[2])) {
        *onCurveCount += 1;
        return 0;
    }
    if (y == y2) {
        return 0;
    }
    // bounds check on X (not required. is it faster?)
#if 0
    if (pts[0].fX > x && pts[1].fX > x && pts[2].fX > x) {
        return 0;
    }
#endif

    SkScalar roots[2];
    int n = SkFindUnitQuadRoots(pts[0].fY - 2 * pts[1].fY + pts[2].fY,
                                2 * (pts[1].fY - pts[0].fY),
                                pts[0].fY - y,
                                roots);
    SkASSERT(n <= 1);
    SkScalar xt;
    if (0 == n) {
        // zero roots are returned only when y0 == y
        // Need [0] if dir == 1
        // and  [2] if dir == -1
        xt = pts[1 - dir].fX;
    } else {
        SkScalar t = roots[0];
        SkScalar C = pts[0].fX;
        SkScalar A = pts[2].fX - 2 * pts[1].fX + C;
        SkScalar B = 2 * (pts[1].fX - C);
        xt = poly_eval(A, B, C, t);
    }
    if (SkScalarNearlyEqual(xt, x)) {
        if (x != pts[2].fX || y != pts[2].fY) {  // don't test end points; they're start points
            *onCurveCount += 1;
            return 0;
        }
    }
    return xt < x ? dir : 0;
}

static int winding_quad(const SkPoint pts[], SkScalar x, SkScalar y, int* onCurveCount) {
    SkPoint dst[5];
    int     n = 0;

    if (!is_mono_quad(pts[0].fY, pts[1].fY, pts[2].fY)) {
        n = SkChopQuadAtYExtrema(pts, dst);
        pts = dst;
    }
    int w = winding_mono_quad(pts, x, y, onCurveCount);
    if (n > 0) {
        w += winding_mono_quad(&pts[2], x, y, onCurveCount);
    }
    return w;
}

static int winding_line(const SkPoint pts[], SkScalar x, SkScalar y, int* onCurveCount) {
    SkScalar x0 = pts[0].fX;
    SkScalar y0 = pts[0].fY;
    SkScalar x1 = pts[1].fX;
    SkScalar y1 = pts[1].fY;

    SkScalar dy = y1 - y0;

    int dir = 1;
    if (y0 > y1) {
        SkTSwap(y0, y1);
        dir = -1;
    }
    if (y < y0 || y > y1) {
        return 0;
    }
    if (checkOnCurve(x, y, pts[0], pts[1])) {
        *onCurveCount += 1;
        return 0;
    }
    if (y == y1) {
        return 0;
    }
    SkScalar cross = (x1 - x0) * (y - pts[0].fY) - dy * (x - x0);

    if (!cross) {
        // zero cross means the point is on the line, and since the case where
        // y of the query point is at the end point is handled above, we can be
        // sure that we're on the line (excluding the end point) here
        if (x != x1 || y != pts[1].fY) {
            *onCurveCount += 1;
        }
        dir = 0;
    } else if (SkScalarSignAsInt(cross) == dir) {
        dir = 0;
    }
    return dir;
}

static void tangent_cubic(const SkPoint pts[], SkScalar x, SkScalar y,
        SkTDArray<SkVector>* tangents) {
    if (!between(pts[0].fY, y, pts[1].fY) && !between(pts[1].fY, y, pts[2].fY)
             && !between(pts[2].fY, y, pts[3].fY)) {
        return;
    }
    if (!between(pts[0].fX, x, pts[1].fX) && !between(pts[1].fX, x, pts[2].fX)
             && !between(pts[2].fX, x, pts[3].fX)) {
        return;
    }
    SkPoint dst[10];
    int n = SkChopCubicAtYExtrema(pts, dst);
    for (int i = 0; i <= n; ++i) {
        SkPoint* c = &dst[i * 3];
        SkScalar t;
        if (!SkCubicClipper::ChopMonoAtY(c, y, &t)) {
            continue;
        }
        SkScalar xt = eval_cubic_pts(c[0].fX, c[1].fX, c[2].fX, c[3].fX, t);
        if (!SkScalarNearlyEqual(x, xt)) {
            continue;
        }
        SkVector tangent;
        SkEvalCubicAt(c, t, nullptr, &tangent, nullptr);
        tangents->push(tangent);
    }
}

static void tangent_conic(const SkPoint pts[], SkScalar x, SkScalar y, SkScalar w,
            SkTDArray<SkVector>* tangents) {
    if (!between(pts[0].fY, y, pts[1].fY) && !between(pts[1].fY, y, pts[2].fY)) {
        return;
    }
    if (!between(pts[0].fX, x, pts[1].fX) && !between(pts[1].fX, x, pts[2].fX)) {
        return;
    }
    SkScalar roots[2];
    SkScalar A = pts[2].fY;
    SkScalar B = pts[1].fY * w - y * w + y;
    SkScalar C = pts[0].fY;
    A += C - 2 * B;  // A = a + c - 2*(b*w - yCept*w + yCept)
    B -= C;  // B = b*w - w * yCept + yCept - a
    C -= y;
    int n = SkFindUnitQuadRoots(A, 2 * B, C, roots);
    for (int index = 0; index < n; ++index) {
        SkScalar t = roots[index];
        SkScalar xt = conic_eval_numerator(&pts[0].fX, w, t) / conic_eval_denominator(w, t);
        if (!SkScalarNearlyEqual(x, xt)) {
            continue;
        }
        SkConic conic(pts, w);
        tangents->push(conic.evalTangentAt(t));
    }
}

static void tangent_quad(const SkPoint pts[], SkScalar x, SkScalar y,
        SkTDArray<SkVector>* tangents) {
    if (!between(pts[0].fY, y, pts[1].fY) && !between(pts[1].fY, y, pts[2].fY)) {
        return;
    }
    if (!between(pts[0].fX, x, pts[1].fX) && !between(pts[1].fX, x, pts[2].fX)) {
        return;
    }
    SkScalar roots[2];
    int n = SkFindUnitQuadRoots(pts[0].fY - 2 * pts[1].fY + pts[2].fY,
                                2 * (pts[1].fY - pts[0].fY),
                                pts[0].fY - y,
                                roots);
    for (int index = 0; index < n; ++index) {
        SkScalar t = roots[index];
        SkScalar C = pts[0].fX;
        SkScalar A = pts[2].fX - 2 * pts[1].fX + C;
        SkScalar B = 2 * (pts[1].fX - C);
        SkScalar xt = poly_eval(A, B, C, t);
        if (!SkScalarNearlyEqual(x, xt)) {
            continue;
        }
        tangents->push(SkEvalQuadTangentAt(pts, t));
    }
}

static void tangent_line(const SkPoint pts[], SkScalar x, SkScalar y,
        SkTDArray<SkVector>* tangents) {
    SkScalar y0 = pts[0].fY;
    SkScalar y1 = pts[1].fY;
    if (!between(y0, y, y1)) {
        return;
    }
    SkScalar x0 = pts[0].fX;
    SkScalar x1 = pts[1].fX;
    if (!between(x0, x, x1)) {
        return;
    }
    SkScalar dx = x1 - x0;
    SkScalar dy = y1 - y0;
    if (!SkScalarNearlyEqual((x - x0) * dy, dx * (y - y0))) {
        return;
    }
    SkVector v;
    v.set(dx, dy);
    tangents->push(v);
}

static bool contains_inclusive(const SkRect& r, SkScalar x, SkScalar y) {
    return r.fLeft <= x && x <= r.fRight && r.fTop <= y && y <= r.fBottom;
}

bool SkPath::contains(SkScalar x, SkScalar y) const {
    bool isInverse = this->isInverseFillType();
    if (this->isEmpty()) {
        return isInverse;
    }

    if (!contains_inclusive(this->getBounds(), x, y)) {
        return isInverse;
    }

    SkPath::Iter iter(*this, true);
    bool done = false;
    int w = 0;
    int onCurveCount = 0;
    do {
        SkPoint pts[4];
        switch (iter.next(pts, false)) {
            case SkPath::kMove_Verb:
            case SkPath::kClose_Verb:
                break;
            case SkPath::kLine_Verb:
                w += winding_line(pts, x, y, &onCurveCount);
                break;
            case SkPath::kQuad_Verb:
                w += winding_quad(pts, x, y, &onCurveCount);
                break;
            case SkPath::kConic_Verb:
                w += winding_conic(pts, x, y, iter.conicWeight(), &onCurveCount);
                break;
            case SkPath::kCubic_Verb:
                w += winding_cubic(pts, x, y, &onCurveCount);
                break;
            case SkPath::kDone_Verb:
                done = true;
                break;
       }
    } while (!done);
    bool evenOddFill = SkPath::kEvenOdd_FillType == this->getFillType()
            || SkPath::kInverseEvenOdd_FillType == this->getFillType();
    if (evenOddFill) {
        w &= 1;
    }
    if (w) {
        return !isInverse;
    }
    if (onCurveCount <= 1) {
        return SkToBool(onCurveCount) ^ isInverse;
    }
    if ((onCurveCount & 1) || evenOddFill) {
        return SkToBool(onCurveCount & 1) ^ isInverse;
    }
    // If the point touches an even number of curves, and the fill is winding, check for
    // coincidence. Count coincidence as places where the on curve points have identical tangents.
    iter.setPath(*this, true);
    done = false;
    SkTDArray<SkVector> tangents;
    do {
        SkPoint pts[4];
        int oldCount = tangents.count();
        switch (iter.next(pts, false)) {
            case SkPath::kMove_Verb:
            case SkPath::kClose_Verb:
                break;
            case SkPath::kLine_Verb:
                tangent_line(pts, x, y, &tangents);
                break;
            case SkPath::kQuad_Verb:
                tangent_quad(pts, x, y, &tangents);
                break;
            case SkPath::kConic_Verb:
                tangent_conic(pts, x, y, iter.conicWeight(), &tangents);
                break;
            case SkPath::kCubic_Verb:
                tangent_cubic(pts, x, y, &tangents);
                break;
            case SkPath::kDone_Verb:
                done = true;
                break;
       }
       if (tangents.count() > oldCount) {
            int last = tangents.count() - 1;
            const SkVector& tangent = tangents[last];
            if (SkScalarNearlyZero(SkPointPriv::LengthSqd(tangent))) {
                tangents.remove(last);
            } else {
                for (int index = 0; index < last; ++index) {
                    const SkVector& test = tangents[index];
                    if (SkScalarNearlyZero(test.cross(tangent))
                            && SkScalarSignAsInt(tangent.fX * test.fX) <= 0
                            && SkScalarSignAsInt(tangent.fY * test.fY) <= 0) {
                        tangents.remove(last);
                        tangents.removeShuffle(index);
                        break;
                    }
                }
            }
        }
    } while (!done);
    return SkToBool(tangents.count()) ^ isInverse;
}

int SkPath::ConvertConicToQuads(const SkPoint& p0, const SkPoint& p1, const SkPoint& p2,
                                SkScalar w, SkPoint pts[], int pow2) {
    const SkConic conic(p0, p1, p2, w);
    return conic.chopIntoQuadsPOW2(pts, pow2);
}

bool SkPathPriv::IsSimpleClosedRect(const SkPath& path, SkRect* rect, SkPath::Direction* direction,
                                    unsigned* start) {
    if (path.getSegmentMasks() != SkPath::kLine_SegmentMask) {
        return false;
    }
    SkPath::RawIter iter(path);
    SkPoint verbPts[4];
    SkPath::Verb v;
    SkPoint rectPts[5];
    int rectPtCnt = 0;
    while ((v = iter.next(verbPts)) != SkPath::kDone_Verb) {
        switch (v) {
            case SkPath::kMove_Verb:
                if (0 != rectPtCnt) {
                    return false;
                }
                rectPts[0] = verbPts[0];
                ++rectPtCnt;
                break;
            case SkPath::kLine_Verb:
                if (5 == rectPtCnt) {
                    return false;
                }
                rectPts[rectPtCnt] = verbPts[1];
                ++rectPtCnt;
                break;
            case SkPath::kClose_Verb:
                if (4 == rectPtCnt) {
                    rectPts[4] = rectPts[0];
                    rectPtCnt = 5;
                }
                break;
            default:
                return false;
        }
    }
    if (rectPtCnt < 5) {
        return false;
    }
    if (rectPts[0] != rectPts[4]) {
        return false;
    }
    // Check for two cases of rectangles: pts 0 and 3 form a vertical edge or a horizontal edge (
    // and pts 1 and 2 the opposite vertical or horizontal edge).
    bool vec03IsVertical;
    if (rectPts[0].fX == rectPts[3].fX && rectPts[1].fX == rectPts[2].fX &&
        rectPts[0].fY == rectPts[1].fY && rectPts[3].fY == rectPts[2].fY) {
        // Make sure it has non-zero width and height
        if (rectPts[0].fX == rectPts[1].fX || rectPts[0].fY == rectPts[3].fY) {
            return false;
        }
        vec03IsVertical = true;
    } else if (rectPts[0].fY == rectPts[3].fY && rectPts[1].fY == rectPts[2].fY &&
               rectPts[0].fX == rectPts[1].fX && rectPts[3].fX == rectPts[2].fX) {
        // Make sure it has non-zero width and height
        if (rectPts[0].fY == rectPts[1].fY || rectPts[0].fX == rectPts[3].fX) {
            return false;
        }
        vec03IsVertical = false;
    } else {
        return false;
    }
    // Set sortFlags so that it has the low bit set if pt index 0 is on right edge and second bit
    // set if it is on the bottom edge.
    unsigned sortFlags =
            ((rectPts[0].fX < rectPts[2].fX) ? 0b00 : 0b01) |
            ((rectPts[0].fY < rectPts[2].fY) ? 0b00 : 0b10);
    switch (sortFlags) {
        case 0b00:
            rect->set(rectPts[0].fX, rectPts[0].fY, rectPts[2].fX, rectPts[2].fY);
            *direction = vec03IsVertical ? SkPath::kCW_Direction : SkPath::kCCW_Direction;
            *start = 0;
            break;
        case 0b01:
            rect->set(rectPts[2].fX, rectPts[0].fY, rectPts[0].fX, rectPts[2].fY);
            *direction = vec03IsVertical ? SkPath::kCCW_Direction : SkPath::kCW_Direction;
            *start = 1;
            break;
        case 0b10:
            rect->set(rectPts[0].fX, rectPts[2].fY, rectPts[2].fX, rectPts[0].fY);
            *direction = vec03IsVertical ? SkPath::kCCW_Direction : SkPath::kCW_Direction;
            *start = 3;
            break;
        case 0b11:
            rect->set(rectPts[2].fX, rectPts[2].fY, rectPts[0].fX, rectPts[0].fY);
            *direction = vec03IsVertical ? SkPath::kCW_Direction : SkPath::kCCW_Direction;
            *start = 2;
            break;
    }
    return true;
}

void SkPathPriv::CreateDrawArcPath(SkPath* path, const SkRect& oval, SkScalar startAngle,
                                   SkScalar sweepAngle, bool useCenter, bool isFillNoPathEffect) {
    SkASSERT(!oval.isEmpty());
    SkASSERT(sweepAngle);

    path->reset();
    path->setIsVolatile(true);
    path->setFillType(SkPath::kWinding_FillType);
    if (isFillNoPathEffect && SkScalarAbs(sweepAngle) >= 360.f) {
        path->addOval(oval);
        return;
    }
    if (useCenter) {
        path->moveTo(oval.centerX(), oval.centerY());
    }
    // Arc to mods at 360 and drawArc is not supposed to.
    bool forceMoveTo = !useCenter;
    while (sweepAngle <= -360.f) {
        path->arcTo(oval, startAngle, -180.f, forceMoveTo);
        startAngle -= 180.f;
        path->arcTo(oval, startAngle, -180.f, false);
        startAngle -= 180.f;
        forceMoveTo = false;
        sweepAngle += 360.f;
    }
    while (sweepAngle >= 360.f) {
        path->arcTo(oval, startAngle, 180.f, forceMoveTo);
        startAngle += 180.f;
        path->arcTo(oval, startAngle, 180.f, false);
        startAngle += 180.f;
        forceMoveTo = false;
        sweepAngle -= 360.f;
    }
    path->arcTo(oval, startAngle, sweepAngle, forceMoveTo);
    if (useCenter) {
        path->close();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
#include "SkNx.h"

static int compute_quad_extremas(const SkPoint src[3], SkPoint extremas[3]) {
    SkScalar ts[2];
    int n  = SkFindQuadExtrema(src[0].fX, src[1].fX, src[2].fX, ts);
        n += SkFindQuadExtrema(src[0].fY, src[1].fY, src[2].fY, &ts[n]);
    SkASSERT(n >= 0 && n <= 2);
    for (int i = 0; i < n; ++i) {
        extremas[i] = SkEvalQuadAt(src, ts[i]);
    }
    extremas[n] = src[2];
    return n + 1;
}

static int compute_conic_extremas(const SkPoint src[3], SkScalar w, SkPoint extremas[3]) {
    SkConic conic(src[0], src[1], src[2], w);
    SkScalar ts[2];
    int n  = conic.findXExtrema(ts);
        n += conic.findYExtrema(&ts[n]);
    SkASSERT(n >= 0 && n <= 2);
    for (int i = 0; i < n; ++i) {
        extremas[i] = conic.evalAt(ts[i]);
    }
    extremas[n] = src[2];
    return n + 1;
}

static int compute_cubic_extremas(const SkPoint src[3], SkPoint extremas[5]) {
    SkScalar ts[4];
    int n  = SkFindCubicExtrema(src[0].fX, src[1].fX, src[2].fX, src[3].fX, ts);
        n += SkFindCubicExtrema(src[0].fY, src[1].fY, src[2].fY, src[3].fY, &ts[n]);
    SkASSERT(n >= 0 && n <= 4);
    for (int i = 0; i < n; ++i) {
        SkEvalCubicAt(src, ts[i], &extremas[i], nullptr, nullptr);
    }
    extremas[n] = src[3];
    return n + 1;
}

SkRect SkPath::computeTightBounds() const {
    if (0 == this->countVerbs()) {
        return SkRect::MakeEmpty();
    }

    if (this->getSegmentMasks() == SkPath::kLine_SegmentMask) {
        return this->getBounds();
    }

    SkPoint extremas[5]; // big enough to hold worst-case curve type (cubic) extremas + 1
    SkPoint pts[4];
    SkPath::RawIter iter(*this);

    // initial with the first MoveTo, so we don't have to check inside the switch
    Sk2s min, max;
    min = max = from_point(this->getPoint(0));
    for (;;) {
        int count = 0;
        switch (iter.next(pts)) {
            case SkPath::kMove_Verb:
                extremas[0] = pts[0];
                count = 1;
                break;
            case SkPath::kLine_Verb:
                extremas[0] = pts[1];
                count = 1;
                break;
            case SkPath::kQuad_Verb:
                count = compute_quad_extremas(pts, extremas);
                break;
            case SkPath::kConic_Verb:
                count = compute_conic_extremas(pts, iter.conicWeight(), extremas);
                break;
            case SkPath::kCubic_Verb:
                count = compute_cubic_extremas(pts, extremas);
                break;
            case SkPath::kClose_Verb:
                break;
            case SkPath::kDone_Verb:
                goto DONE;
        }
        for (int i = 0; i < count; ++i) {
            Sk2s tmp = from_point(extremas[i]);
            min = Sk2s::Min(min, tmp);
            max = Sk2s::Max(max, tmp);
        }
    }
DONE:
    SkRect bounds;
    min.store((SkPoint*)&bounds.fLeft);
    max.store((SkPoint*)&bounds.fRight);
    return bounds;
}

bool SkPath::IsLineDegenerate(const SkPoint& p1, const SkPoint& p2, bool exact) {
    return exact ? p1 == p2 : SkPointPriv::EqualsWithinTolerance(p1, p2);
}

bool SkPath::IsQuadDegenerate(const SkPoint& p1, const SkPoint& p2,
                                const SkPoint& p3, bool exact) {
    return exact ? p1 == p2 && p2 == p3 : SkPointPriv::EqualsWithinTolerance(p1, p2) &&
            SkPointPriv::EqualsWithinTolerance(p2, p3);
}

bool SkPath::IsCubicDegenerate(const SkPoint& p1, const SkPoint& p2,
                                const SkPoint& p3, const SkPoint& p4, bool exact) {
    return exact ? p1 == p2 && p2 == p3 && p3 == p4 :
            SkPointPriv::EqualsWithinTolerance(p1, p2) &&
            SkPointPriv::EqualsWithinTolerance(p2, p3) &&
            SkPointPriv::EqualsWithinTolerance(p3, p4);
}
