
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkBuffer.h"
#include "SkErrorInternals.h"
#include "SkMath.h"
#include "SkPath.h"
#include "SkPathRef.h"
#include "SkRRect.h"
#include "SkThread.h"

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
        fSaved = static_cast<SkPath::Direction>(fPath->fDirection);
    }

    ~SkAutoDisableDirectionCheck() {
        fPath->fDirection = fSaved;
    }

private:
    SkPath*              fPath;
    SkPath::Direction    fSaved;
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
        if (fEmpty || fHasValidBounds) {
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
    : fPathRef(SkPathRef::CreateEmpty())
#ifdef SK_BUILD_FOR_ANDROID
    , fSourcePath(NULL)
#endif
{
    this->resetFields();
}

void SkPath::resetFields() {
    //fPathRef is assumed to have been emptied by the caller.
    fLastMoveToIndex = INITIAL_LASTMOVETOINDEX_VALUE;
    fFillType = kWinding_FillType;
    fConvexity = kUnknown_Convexity;
    fDirection = kUnknown_Direction;

    // We don't touch Android's fSourcePath.  It's used to track texture garbage collection, so we
    // don't want to muck with it if it's been set to something non-NULL.
}

SkPath::SkPath(const SkPath& that)
    : fPathRef(SkRef(that.fPathRef.get())) {
    this->copyFields(that);
#ifdef SK_BUILD_FOR_ANDROID
    fSourcePath   = that.fSourcePath;
#endif
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
#ifdef SK_BUILD_FOR_ANDROID
        fSourcePath = that.fSourcePath;
#endif
    }
    SkDEBUGCODE(this->validate();)
    return *this;
}

void SkPath::copyFields(const SkPath& that) {
    //fPathRef is assumed to have been set by the caller.
    fLastMoveToIndex = that.fLastMoveToIndex;
    fFillType        = that.fFillType;
    fConvexity       = that.fConvexity;
    fDirection       = that.fDirection;
}

bool operator==(const SkPath& a, const SkPath& b) {
    // note: don't need to look at isConvex or bounds, since just comparing the
    // raw data is sufficient.
    return &a == &b ||
        (a.fFillType == b.fFillType && *a.fPathRef.get() == *b.fPathRef.get());
}

void SkPath::swap(SkPath& that) {
    SkASSERT(&that != NULL);

    if (this != &that) {
        fPathRef.swap(&that.fPathRef);
        SkTSwap<int>(fLastMoveToIndex, that.fLastMoveToIndex);
        SkTSwap<uint8_t>(fFillType, that.fFillType);
        SkTSwap<uint8_t>(fConvexity, that.fConvexity);
        SkTSwap<uint8_t>(fDirection, that.fDirection);
#ifdef SK_BUILD_FOR_ANDROID
        SkTSwap<const SkPath*>(fSourcePath, that.fSourcePath);
#endif
    }
}

static inline bool check_edge_against_rect(const SkPoint& p0,
                                           const SkPoint& p1,
                                           const SkRect& rect,
                                           SkPath::Direction dir) {
    const SkPoint* edgeBegin;
    SkVector v;
    if (SkPath::kCW_Direction == dir) {
        v = p1 - p0;
        edgeBegin = &p0;
    } else {
        v = p0 - p1;
        edgeBegin = &p1;
    }
    if (v.fX || v.fY) {
        // check the cross product of v with the vec from edgeBegin to each rect corner
        SkScalar yL = SkScalarMul(v.fY, rect.fLeft - edgeBegin->fX);
        SkScalar xT = SkScalarMul(v.fX, rect.fTop - edgeBegin->fY);
        SkScalar yR = SkScalarMul(v.fY, rect.fRight - edgeBegin->fX);
        SkScalar xB = SkScalarMul(v.fX, rect.fBottom - edgeBegin->fY);
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

    Direction direction;
    if (!this->cheapComputeDirection(&direction)) {
        return false;
    }

    SkPoint firstPt;
    SkPoint prevPt;
    RawIter iter(*this);
    SkPath::Verb verb;
    SkPoint pts[4];
    SkDEBUGCODE(int moveCnt = 0;)
    SkDEBUGCODE(int segmentCount = 0;)
    SkDEBUGCODE(int closeCount = 0;)

    while ((verb = iter.next(pts)) != kDone_Verb) {
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
                SkDEBUGCODE(++segmentCount);
                break;
            case kQuad_Verb:
            case kConic_Verb:
                SkASSERT(moveCnt && !closeCount);
                SkDEBUGCODE(++segmentCount);
                nextPt = 2;
                break;
            case kCubic_Verb:
                SkASSERT(moveCnt && !closeCount);
                SkDEBUGCODE(++segmentCount);
                nextPt = 3;
                break;
            case kClose_Verb:
                SkDEBUGCODE(++closeCount;)
                break;
            default:
                SkDEBUGFAIL("unknown verb");
        }
        if (-1 != nextPt) {
            if (!check_edge_against_rect(prevPt, pts[nextPt], rect, direction)) {
                return false;
            }
            prevPt = pts[nextPt];
        }
    }

    return check_edge_against_rect(prevPt, firstPt, rect, direction);
}

uint32_t SkPath::getGenerationID() const {
    uint32_t genID = fPathRef->genID();
#ifdef SK_BUILD_FOR_ANDROID
    SkASSERT((unsigned)fFillType < (1 << (32 - kPathRefGenIDBitCnt)));
    genID |= static_cast<uint32_t>(fFillType) << kPathRefGenIDBitCnt;
#endif
    return genID;
}

#ifdef SK_BUILD_FOR_ANDROID
const SkPath* SkPath::getSourcePath() const {
    return fSourcePath;
}

void SkPath::setSourcePath(const SkPath* path) {
    fSourcePath = path;
}
#endif

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
    const SkPoint* savePts = NULL;
    first.set(0, 0);
    last.set(0, 0);
    int firstDirection = 0;
    int lastDirection = 0;
    int nextDirection = 0;
    bool closedOrMoved = false;
    bool autoClose = false;
    int verbCnt = fPathRef->countVerbs();
    while (*currVerb < verbCnt && (!allowPartial || !autoClose)) {
        switch (fPathRef->atVerb(*currVerb)) {
            case kClose_Verb:
                savePts = pts;
                pts = *ptsPtr;
                autoClose = true;
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
                last = *pts++;
                closedOrMoved = true;
                break;
            default:
                SkDEBUGFAIL("unexpected verb");
                break;
        }
        *currVerb += 1;
        lastDirection = nextDirection;
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

SkPath::PathAsRect SkPath::asRect(Direction* direction) const {
    SK_COMPILE_ASSERT(0 == kNone_PathAsRect, path_as_rect_mismatch);
    SK_COMPILE_ASSERT(1 == kFill_PathAsRect, path_as_rect_mismatch);
    SK_COMPILE_ASSERT(2 == kStroke_PathAsRect, path_as_rect_mismatch);
    bool isClosed = false;
    return (PathAsRect) (isRect(&isClosed, direction) + isClosed);
}

bool SkPath::isRect(SkRect* rect) const {
    SkDEBUGCODE(this->validate();)
    int currVerb = 0;
    const SkPoint* pts = fPathRef->points();
    bool result = isRectContour(false, &currVerb, &pts, NULL, NULL);
    if (result && rect) {
        *rect = getBounds();
    }
    return result;
}

bool SkPath::isRect(bool* isClosed, Direction* direction) const {
    SkDEBUGCODE(this->validate();)
    int currVerb = 0;
    const SkPoint* pts = fPathRef->points();
    return isRectContour(false, &currVerb, &pts, isClosed, direction);
}

bool SkPath::isNestedRects(SkRect rects[2], Direction dirs[2]) const {
    SkDEBUGCODE(this->validate();)
    int currVerb = 0;
    const SkPoint* pts = fPathRef->points();
    const SkPoint* first = pts;
    Direction testDirs[2];
    if (!isRectContour(true, &currVerb, &pts, NULL, &testDirs[0])) {
        return false;
    }
    const SkPoint* last = pts;
    SkRect testRects[2];
    if (isRectContour(false, &currVerb, &pts, NULL, &testDirs[1])) {
        testRects[0].set(first, SkToS32(last - first));
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
    memcpy(dst, fPathRef->points(), count * sizeof(SkPoint));
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

#define DIRTY_AFTER_EDIT                 \
    do {                                 \
        fConvexity = kUnknown_Convexity; \
        fDirection = kUnknown_Direction; \
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

static void assert_known_direction(int dir) {
    SkASSERT(SkPath::kCW_Direction == dir || SkPath::kCCW_Direction == dir);
}

void SkPath::addRect(const SkRect& rect, Direction dir) {
    this->addRect(rect.fLeft, rect.fTop, rect.fRight, rect.fBottom, dir);
}

void SkPath::addRect(SkScalar left, SkScalar top, SkScalar right,
                     SkScalar bottom, Direction dir) {
    assert_known_direction(dir);
    fDirection = this->hasOnlyMoveTos() ? dir : kUnknown_Direction;
    SkAutoDisableDirectionCheck addc(this);

    SkAutoPathBoundsUpdate apbu(this, left, top, right, bottom);

    this->incReserve(5);

    this->moveTo(left, top);
    if (dir == kCCW_Direction) {
        this->lineTo(left, bottom);
        this->lineTo(right, bottom);
        this->lineTo(right, top);
    } else {
        this->lineTo(right, top);
        this->lineTo(right, bottom);
        this->lineTo(left, bottom);
    }
    this->close();
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
    }

    DIRTY_AFTER_EDIT;
    SkDEBUGCODE(this->validate();)
}

#include "SkGeometry.h"

static int build_arc_points(const SkRect& oval, SkScalar startAngle,
                            SkScalar sweepAngle,
                            SkPoint pts[kSkBuildQuadArcStorage]) {

    if (0 == sweepAngle &&
        (0 == startAngle || SkIntToScalar(360) == startAngle)) {
        // Chrome uses this path to move into and out of ovals. If not
        // treated as a special case the moves can distort the oval's
        // bounding box (and break the circle special case).
        pts[0].set(oval.fRight, oval.centerY());
        return 1;
    } else if (0 == oval.width() && 0 == oval.height()) {
        // Chrome will sometimes create 0 radius round rects. Having degenerate
        // quad segments in the path prevents the path from being recognized as
        // a rect.
        // TODO: optimizing the case where only one of width or height is zero
        // should also be considered. This case, however, doesn't seem to be
        // as common as the single point case.
        pts[0].set(oval.fRight, oval.fTop);
        return 1;
    }

    SkVector start, stop;

    start.fY = SkScalarSinCos(SkDegreesToRadians(startAngle), &start.fX);
    stop.fY = SkScalarSinCos(SkDegreesToRadians(startAngle + sweepAngle),
                             &stop.fX);

    /*  If the sweep angle is nearly (but less than) 360, then due to precision
        loss in radians-conversion and/or sin/cos, we may end up with coincident
        vectors, which will fool SkBuildQuadArc into doing nothing (bad) instead
        of drawing a nearly complete circle (good).
             e.g. canvas.drawArc(0, 359.99, ...)
             -vs- canvas.drawArc(0, 359.9, ...)
        We try to detect this edge case, and tweak the stop vector
     */
    if (start == stop) {
        SkScalar sw = SkScalarAbs(sweepAngle);
        if (sw < SkIntToScalar(360) && sw > SkIntToScalar(359)) {
            SkScalar stopRad = SkDegreesToRadians(startAngle + sweepAngle);
            // make a guess at a tiny angle (in radians) to tweak by
            SkScalar deltaRad = SkScalarCopySign(SK_Scalar1/512, sweepAngle);
            // not sure how much will be enough, so we use a loop
            do {
                stopRad -= deltaRad;
                stop.fY = SkScalarSinCos(stopRad, &stop.fX);
            } while (start == stop);
        }
    }

    SkMatrix    matrix;

    matrix.setScale(SkScalarHalf(oval.width()), SkScalarHalf(oval.height()));
    matrix.postTranslate(oval.centerX(), oval.centerY());

    return SkBuildQuadArc(start, stop,
                          sweepAngle > 0 ? kCW_SkRotationDirection :
                                           kCCW_SkRotationDirection,
                          &matrix, pts);
}

void SkPath::addRoundRect(const SkRect& rect, const SkScalar radii[],
                          Direction dir) {
    SkRRect rrect;
    rrect.setRectRadii(rect, (const SkVector*) radii);
    this->addRRect(rrect, dir);
}

/* The inline clockwise and counterclockwise round rect quad approximations
   make it easier to see the symmetry patterns used by add corner quads.
Clockwise                                                     corner value
    path->lineTo(rect.fLeft,           rect.fTop    + ry);    0 upper left
    path->quadTo(rect.fLeft,           rect.fTop    + offPtY,
                 rect.fLeft  + midPtX, rect.fTop    + midPtY);
    path->quadTo(rect.fLeft  + offPtX, rect.fTop,
                 rect.fLeft  + rx,     rect.fTop);

    path->lineTo(rect.fRight - rx,     rect.fTop);            1 upper right
    path->quadTo(rect.fRight - offPtX, rect.fTop,
                 rect.fRight - midPtX, rect.fTop    + midPtY);
    path->quadTo(rect.fRight,          rect.fTop    + offPtY,
                 rect.fRight,          rect.fTop    + ry);

    path->lineTo(rect.fRight,          rect.fBottom - ry);    2 lower right
    path->quadTo(rect.fRight,          rect.fBottom - offPtY,
                 rect.fRight - midPtX, rect.fBottom - midPtY);
    path->quadTo(rect.fRight - offPtX, rect.fBottom,
                 rect.fRight - rx,     rect.fBottom);

    path->lineTo(rect.fLeft  + rx,     rect.fBottom);         3 lower left
    path->quadTo(rect.fLeft  + offPtX, rect.fBottom,
                 rect.fLeft  + midPtX, rect.fBottom - midPtY);
    path->quadTo(rect.fLeft,           rect.fBottom - offPtY,
                 rect.fLeft,           rect.fBottom - ry);

Counterclockwise
    path->lineTo(rect.fLeft,           rect.fBottom - ry);    3 lower left
    path->quadTo(rect.fLeft,           rect.fBottom - offPtY,
                 rect.fLeft  + midPtX, rect.fBottom - midPtY);
    path->quadTo(rect.fLeft  + offPtX, rect.fBottom,
                 rect.fLeft  + rx,     rect.fBottom);

    path->lineTo(rect.fRight - rx,     rect.fBottom);         2 lower right
    path->quadTo(rect.fRight - offPtX, rect.fBottom,
                 rect.fRight - midPtX, rect.fBottom - midPtY);
    path->quadTo(rect.fRight,          rect.fBottom - offPtY,
                 rect.fRight,          rect.fBottom - ry);

    path->lineTo(rect.fRight,          rect.fTop    + ry);    1 upper right
    path->quadTo(rect.fRight,          rect.fTop    + offPtY,
                 rect.fRight - midPtX, rect.fTop    + midPtY);
    path->quadTo(rect.fRight - offPtX, rect.fTop,
                 rect.fRight - rx,     rect.fTop);

    path->lineTo(rect.fLeft  + rx,     rect.fTop);            0 upper left
    path->quadTo(rect.fLeft  + offPtX, rect.fTop,
                 rect.fLeft  + midPtX, rect.fTop    + midPtY);
    path->quadTo(rect.fLeft,           rect.fTop    + offPtY,
                 rect.fLeft,           rect.fTop    + ry);
*/
static void add_corner_quads(SkPath* path, const SkRRect& rrect,
                             SkRRect::Corner corner, SkPath::Direction dir) {
    const SkRect& rect = rrect.rect();
    const SkVector& radii = rrect.radii(corner);
    SkScalar rx = radii.fX;
    SkScalar ry = radii.fY;
    // The mid point of the quadratic arc approximation is half way between the two
    // control points.
    const SkScalar mid = 1 - (SK_Scalar1 + SK_ScalarTanPIOver8) / 2;
    SkScalar midPtX = rx * mid;
    SkScalar midPtY = ry * mid;
    const SkScalar control = 1 - SK_ScalarTanPIOver8;
    SkScalar offPtX = rx * control;
    SkScalar offPtY = ry * control;
    static const int kCornerPts = 5;
    SkScalar xOff[kCornerPts];
    SkScalar yOff[kCornerPts];

    if ((corner & 1) == (dir == SkPath::kCCW_Direction)) {  // corners always alternate direction
        SkASSERT(dir == SkPath::kCCW_Direction
             ? corner == SkRRect::kLowerLeft_Corner || corner == SkRRect::kUpperRight_Corner
             : corner == SkRRect::kUpperLeft_Corner || corner == SkRRect::kLowerRight_Corner);
        xOff[0] = xOff[1] = 0;
        xOff[2] = midPtX;
        xOff[3] = offPtX;
        xOff[4] = rx;
        yOff[0] = ry;
        yOff[1] = offPtY;
        yOff[2] = midPtY;
        yOff[3] = yOff[4] = 0;
    } else {
        xOff[0] = rx;
        xOff[1] = offPtX;
        xOff[2] = midPtX;
        xOff[3] = xOff[4] = 0;
        yOff[0] = yOff[1] = 0;
        yOff[2] = midPtY;
        yOff[3] = offPtY;
        yOff[4] = ry;
    }
    if ((corner - 1) & 2) {
        SkASSERT(corner == SkRRect::kLowerLeft_Corner || corner == SkRRect::kUpperLeft_Corner);
        for (int i = 0; i < kCornerPts; ++i) {
            xOff[i] = rect.fLeft + xOff[i];
        }
    } else {
        SkASSERT(corner == SkRRect::kLowerRight_Corner || corner == SkRRect::kUpperRight_Corner);
        for (int i = 0; i < kCornerPts; ++i) {
            xOff[i] = rect.fRight - xOff[i];
        }
    }
    if (corner < SkRRect::kLowerRight_Corner) {
        for (int i = 0; i < kCornerPts; ++i) {
            yOff[i] = rect.fTop + yOff[i];
        }
    } else {
        for (int i = 0; i < kCornerPts; ++i) {
            yOff[i] = rect.fBottom - yOff[i];
        }
    }

    SkPoint lastPt;
    SkAssertResult(path->getLastPt(&lastPt));
    if (lastPt.fX != xOff[0] || lastPt.fY != yOff[0]) {
        path->lineTo(xOff[0], yOff[0]);
    }
    if (rx || ry) {
        path->quadTo(xOff[1], yOff[1], xOff[2], yOff[2]);
        path->quadTo(xOff[3], yOff[3], xOff[4], yOff[4]);
    } else {
        path->lineTo(xOff[2], yOff[2]);
        path->lineTo(xOff[4], yOff[4]);
    }
}

void SkPath::addRRect(const SkRRect& rrect, Direction dir) {
    assert_known_direction(dir);

    if (rrect.isEmpty()) {
        return;
    }

    const SkRect& bounds = rrect.getBounds();

    if (rrect.isRect()) {
        this->addRect(bounds, dir);
    } else if (rrect.isOval()) {
        this->addOval(bounds, dir);
#ifdef SK_IGNORE_QUAD_RR_CORNERS_OPT
    } else if (rrect.isSimple()) {
        const SkVector& rad = rrect.getSimpleRadii();
        this->addRoundRect(bounds, rad.x(), rad.y(), dir);
#endif
    } else {
        fDirection = this->hasOnlyMoveTos() ? dir : kUnknown_Direction;

        SkAutoPathBoundsUpdate apbu(this, bounds);
        SkAutoDisableDirectionCheck addc(this);

        this->incReserve(21);
        if (kCW_Direction == dir) {
            this->moveTo(bounds.fLeft,
                         bounds.fBottom - rrect.fRadii[SkRRect::kLowerLeft_Corner].fY);
            add_corner_quads(this, rrect, SkRRect::kUpperLeft_Corner, dir);
            add_corner_quads(this, rrect, SkRRect::kUpperRight_Corner, dir);
            add_corner_quads(this, rrect, SkRRect::kLowerRight_Corner, dir);
            add_corner_quads(this, rrect, SkRRect::kLowerLeft_Corner, dir);
        } else {
            this->moveTo(bounds.fLeft,
                         bounds.fTop + rrect.fRadii[SkRRect::kUpperLeft_Corner].fY);
            add_corner_quads(this, rrect, SkRRect::kLowerLeft_Corner, dir);
            add_corner_quads(this, rrect, SkRRect::kLowerRight_Corner, dir);
            add_corner_quads(this, rrect, SkRRect::kUpperRight_Corner, dir);
            add_corner_quads(this, rrect, SkRRect::kUpperLeft_Corner, dir);
        }
        this->close();
    }
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

#ifdef SK_IGNORE_QUAD_RR_CORNERS_OPT
#define CUBIC_ARC_FACTOR    ((SK_ScalarSqrt2 - SK_Scalar1) * 4 / 3)
#endif

void SkPath::addRoundRect(const SkRect& rect, SkScalar rx, SkScalar ry,
                          Direction dir) {
    assert_known_direction(dir);

    if (rx < 0 || ry < 0) {
        SkErrorInternals::SetError( kInvalidArgument_SkError,
                                    "I got %f and %f as radii to SkPath::AddRoundRect, "
                                    "but negative radii are not allowed.",
                                    SkScalarToDouble(rx), SkScalarToDouble(ry) );
        return;
    }

#ifdef SK_IGNORE_QUAD_RR_CORNERS_OPT
    SkScalar    w = rect.width();
    SkScalar    halfW = SkScalarHalf(w);
    SkScalar    h = rect.height();
    SkScalar    halfH = SkScalarHalf(h);

    if (halfW <= 0 || halfH <= 0) {
        return;
    }

    bool skip_hori = rx >= halfW;
    bool skip_vert = ry >= halfH;

    if (skip_hori && skip_vert) {
        this->addOval(rect, dir);
        return;
    }

    fDirection = this->hasOnlyMoveTos() ? dir : kUnknown_Direction;

    SkAutoPathBoundsUpdate apbu(this, rect);
    SkAutoDisableDirectionCheck addc(this);

    if (skip_hori) {
        rx = halfW;
    } else if (skip_vert) {
        ry = halfH;
    }
    SkScalar    sx = SkScalarMul(rx, CUBIC_ARC_FACTOR);
    SkScalar    sy = SkScalarMul(ry, CUBIC_ARC_FACTOR);

    this->incReserve(17);
    this->moveTo(rect.fRight - rx, rect.fTop);                  // top-right
    if (dir == kCCW_Direction) {
        if (!skip_hori) {
            this->lineTo(rect.fLeft + rx, rect.fTop);           // top
        }
        this->cubicTo(rect.fLeft + rx - sx, rect.fTop,
                      rect.fLeft, rect.fTop + ry - sy,
                      rect.fLeft, rect.fTop + ry);          // top-left
        if (!skip_vert) {
            this->lineTo(rect.fLeft, rect.fBottom - ry);        // left
        }
        this->cubicTo(rect.fLeft, rect.fBottom - ry + sy,
                      rect.fLeft + rx - sx, rect.fBottom,
                      rect.fLeft + rx, rect.fBottom);       // bot-left
        if (!skip_hori) {
            this->lineTo(rect.fRight - rx, rect.fBottom);       // bottom
        }
        this->cubicTo(rect.fRight - rx + sx, rect.fBottom,
                      rect.fRight, rect.fBottom - ry + sy,
                      rect.fRight, rect.fBottom - ry);      // bot-right
        if (!skip_vert) {
            this->lineTo(rect.fRight, rect.fTop + ry);          // right
        }
        this->cubicTo(rect.fRight, rect.fTop + ry - sy,
                      rect.fRight - rx + sx, rect.fTop,
                      rect.fRight - rx, rect.fTop);         // top-right
    } else {
        this->cubicTo(rect.fRight - rx + sx, rect.fTop,
                      rect.fRight, rect.fTop + ry - sy,
                      rect.fRight, rect.fTop + ry);         // top-right
        if (!skip_vert) {
            this->lineTo(rect.fRight, rect.fBottom - ry);       // right
        }
        this->cubicTo(rect.fRight, rect.fBottom - ry + sy,
                      rect.fRight - rx + sx, rect.fBottom,
                      rect.fRight - rx, rect.fBottom);      // bot-right
        if (!skip_hori) {
            this->lineTo(rect.fLeft + rx, rect.fBottom);        // bottom
        }
        this->cubicTo(rect.fLeft + rx - sx, rect.fBottom,
                      rect.fLeft, rect.fBottom - ry + sy,
                      rect.fLeft, rect.fBottom - ry);       // bot-left
        if (!skip_vert) {
            this->lineTo(rect.fLeft, rect.fTop + ry);           // left
        }
        this->cubicTo(rect.fLeft, rect.fTop + ry - sy,
                      rect.fLeft + rx - sx, rect.fTop,
                      rect.fLeft + rx, rect.fTop);          // top-left
        if (!skip_hori) {
            this->lineTo(rect.fRight - rx, rect.fTop);          // top
        }
    }
    this->close();
#else
    SkRRect rrect;
    rrect.setRectXY(rect, rx, ry);
    this->addRRect(rrect, dir);
#endif
}

void SkPath::addOval(const SkRect& oval, Direction dir) {
    assert_known_direction(dir);

    /* If addOval() is called after previous moveTo(),
       this path is still marked as an oval. This is used to
       fit into WebKit's calling sequences.
       We can't simply check isEmpty() in this case, as additional
       moveTo() would mark the path non empty.
     */
    bool isOval = hasOnlyMoveTos();
    if (isOval) {
        fDirection = dir;
    } else {
        fDirection = kUnknown_Direction;
    }

    SkAutoDisableDirectionCheck addc(this);

    SkAutoPathBoundsUpdate apbu(this, oval);

    SkScalar    cx = oval.centerX();
    SkScalar    cy = oval.centerY();
    SkScalar    rx = SkScalarHalf(oval.width());
    SkScalar    ry = SkScalarHalf(oval.height());

    SkScalar    sx = SkScalarMul(rx, SK_ScalarTanPIOver8);
    SkScalar    sy = SkScalarMul(ry, SK_ScalarTanPIOver8);
    SkScalar    mx = SkScalarMul(rx, SK_ScalarRoot2Over2);
    SkScalar    my = SkScalarMul(ry, SK_ScalarRoot2Over2);

    /*
        To handle imprecision in computing the center and radii, we revert to
        the provided bounds when we can (i.e. use oval.fLeft instead of cx-rx)
        to ensure that we don't exceed the oval's bounds *ever*, since we want
        to use oval for our fast-bounds, rather than have to recompute it.
    */
    const SkScalar L = oval.fLeft;      // cx - rx
    const SkScalar T = oval.fTop;       // cy - ry
    const SkScalar R = oval.fRight;     // cx + rx
    const SkScalar B = oval.fBottom;    // cy + ry

    this->incReserve(17);   // 8 quads + close
    this->moveTo(R, cy);
    if (dir == kCCW_Direction) {
        this->quadTo(      R, cy - sy, cx + mx, cy - my);
        this->quadTo(cx + sx,       T, cx     ,       T);
        this->quadTo(cx - sx,       T, cx - mx, cy - my);
        this->quadTo(      L, cy - sy,       L, cy     );
        this->quadTo(      L, cy + sy, cx - mx, cy + my);
        this->quadTo(cx - sx,       B, cx     ,       B);
        this->quadTo(cx + sx,       B, cx + mx, cy + my);
        this->quadTo(      R, cy + sy,       R, cy     );
    } else {
        this->quadTo(      R, cy + sy, cx + mx, cy + my);
        this->quadTo(cx + sx,       B, cx     ,       B);
        this->quadTo(cx - sx,       B, cx - mx, cy + my);
        this->quadTo(      L, cy + sy,       L, cy     );
        this->quadTo(      L, cy - sy, cx - mx, cy - my);
        this->quadTo(cx - sx,       T, cx     ,       T);
        this->quadTo(cx + sx,       T, cx + mx, cy - my);
        this->quadTo(      R, cy - sy,       R, cy     );
    }
    this->close();

    SkPathRef::Editor ed(&fPathRef);

    ed.setIsOval(isOval);
}

void SkPath::addCircle(SkScalar x, SkScalar y, SkScalar r, Direction dir) {
    if (r > 0) {
        SkRect  rect;
        rect.set(x - r, y - r, x + r, y + r);
        this->addOval(rect, dir);
    }
}

void SkPath::arcTo(const SkRect& oval, SkScalar startAngle, SkScalar sweepAngle,
                   bool forceMoveTo) {
    if (oval.width() < 0 || oval.height() < 0) {
        return;
    }

    SkPoint pts[kSkBuildQuadArcStorage];
    int count = build_arc_points(oval, startAngle, sweepAngle, pts);
    SkASSERT((count & 1) == 1);

    if (fPathRef->countVerbs() == 0) {
        forceMoveTo = true;
    }
    this->incReserve(count);
    forceMoveTo ? this->moveTo(pts[0]) : this->lineTo(pts[0]);
    for (int i = 1; i < count; i += 2) {
        this->quadTo(pts[i], pts[i+1]);
    }
}

void SkPath::addArc(const SkRect& oval, SkScalar startAngle, SkScalar sweepAngle) {
    if (oval.isEmpty() || 0 == sweepAngle) {
        return;
    }

    const SkScalar kFullCircleAngle = SkIntToScalar(360);

    if (sweepAngle >= kFullCircleAngle || sweepAngle <= -kFullCircleAngle) {
        this->addOval(oval, sweepAngle > 0 ? kCW_Direction : kCCW_Direction);
        return;
    }

    SkPoint pts[kSkBuildQuadArcStorage];
    int count = build_arc_points(oval, startAngle, sweepAngle, pts);

    SkDEBUGCODE(this->validate();)
    SkASSERT(count & 1);

    fLastMoveToIndex = fPathRef->countPoints();

    SkPathRef::Editor ed(&fPathRef, 1+(count-1)/2, count);

    ed.growForVerb(kMove_Verb)->set(pts[0].fX, pts[0].fY);
    if (count > 1) {
        SkPoint* p = ed.growForRepeatedVerb(kQuad_Verb, (count-1)/2);
        memcpy(p, &pts[1], (count-1) * sizeof(SkPoint));
    }

    DIRTY_AFTER_EDIT;
    SkDEBUGCODE(this->validate();)
}

/*
    Need to handle the case when the angle is sharp, and our computed end-points
    for the arc go behind pt1 and/or p2...
*/
void SkPath::arcTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2,
                   SkScalar radius) {
    SkVector    before, after;

    // need to know our prev pt so we can construct tangent vectors
    {
        SkPoint start;
        this->getLastPt(&start);
        // Handle degenerate cases by adding a line to the first point and
        // bailing out.
        if ((x1 == start.fX && y1 == start.fY) ||
            (x1 == x2 && y1 == y2) ||
            radius == 0) {
            this->lineTo(x1, y1);
            return;
        }
        before.setNormalize(x1 - start.fX, y1 - start.fY);
        after.setNormalize(x2 - x1, y2 - y1);
    }

    SkScalar cosh = SkPoint::DotProduct(before, after);
    SkScalar sinh = SkPoint::CrossProduct(before, after);

    if (SkScalarNearlyZero(sinh)) {   // angle is too tight
        this->lineTo(x1, y1);
        return;
    }

    SkScalar dist = SkScalarMulDiv(radius, SK_Scalar1 - cosh, sinh);
    if (dist < 0) {
        dist = -dist;
    }

    SkScalar xx = x1 - SkScalarMul(dist, before.fX);
    SkScalar yy = y1 - SkScalarMul(dist, before.fY);
    SkRotationDirection arcDir;

    // now turn before/after into normals
    if (sinh > 0) {
        before.rotateCCW();
        after.rotateCCW();
        arcDir = kCW_SkRotationDirection;
    } else {
        before.rotateCW();
        after.rotateCW();
        arcDir = kCCW_SkRotationDirection;
    }

    SkMatrix    matrix;
    SkPoint     pts[kSkBuildQuadArcStorage];

    matrix.setScale(radius, radius);
    matrix.postTranslate(xx - SkScalarMul(radius, before.fX),
                         yy - SkScalarMul(radius, before.fY));

    int count = SkBuildQuadArc(before, after, arcDir, &matrix, pts);

    this->incReserve(count);
    // [xx,yy] == pts[0]
    this->lineTo(xx, yy);
    for (int i = 1; i < count; i += 2) {
        this->quadTo(pts[i], pts[i+1]);
    }
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

    SkMatrix::MapPtsProc proc = matrix.getMapPtsProc();
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
    int i, vcount = path.fPathRef->countVerbs();
    // exit early if the path is empty, or just has a moveTo.
    if (vcount < 2) {
        return;
    }

    SkPathRef::Editor(&fPathRef, vcount, path.countPoints());

    const uint8_t*  verbs = path.fPathRef->verbs();
    const SkPoint*  pts = path.fPathRef->points();
    const SkScalar* conicWeights = path.fPathRef->conicWeights();

    SkASSERT(verbs[~0] == kMove_Verb);
    for (i = 1; i < vcount; ++i) {
        unsigned v = verbs[~i];
        int n = pts_in_verb(v);
        if (n == 0) {
            break;
        }
        pts += n;
        conicWeights += (SkPath::kConic_Verb == v);
    }

    while (--i > 0) {
        switch (verbs[~i]) {
            case kLine_Verb:
                this->lineTo(pts[-1].fX, pts[-1].fY);
                break;
            case kQuad_Verb:
                this->quadTo(pts[-1].fX, pts[-1].fY, pts[-2].fX, pts[-2].fY);
                break;
            case kConic_Verb:
                this->conicTo(pts[-1], pts[-2], *--conicWeights);
                break;
            case kCubic_Verb:
                this->cubicTo(pts[-1].fX, pts[-1].fY, pts[-2].fX, pts[-2].fY,
                              pts[-3].fX, pts[-3].fY);
                break;
            default:
                SkDEBUGFAIL("bad verb");
                break;
        }
        pts -= pts_in_verb(verbs[~i]);
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

#include "SkGeometry.h"

static void subdivide_quad_to(SkPath* path, const SkPoint pts[3],
                              int level = 2) {
    if (--level >= 0) {
        SkPoint tmp[5];

        SkChopQuadAtHalf(pts, tmp);
        subdivide_quad_to(path, &tmp[0], level);
        subdivide_quad_to(path, &tmp[2], level);
    } else {
        path->quadTo(pts[1], pts[2]);
    }
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
    if (dst == NULL) {
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
                    subdivide_quad_to(&tmp, pts);
                    break;
                case kConic_Verb:
                    SkDEBUGFAIL("TODO: compute new weight");
                    tmp.conicTo(pts[1], pts[2], iter.conicWeight());
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
        dst->fDirection = kUnknown_Direction;
    } else {
        SkPathRef::CreateTransformedCopy(&dst->fPathRef, *fPathRef.get(), matrix);

        if (this != dst) {
            dst->fFillType = fFillType;
            dst->fConvexity = fConvexity;
        }

        if (kUnknown_Direction == fDirection) {
            dst->fDirection = kUnknown_Direction;
        } else {
            SkScalar det2x2 =
                SkScalarMul(matrix.get(SkMatrix::kMScaleX), matrix.get(SkMatrix::kMScaleY)) -
                SkScalarMul(matrix.get(SkMatrix::kMSkewX), matrix.get(SkMatrix::kMSkewY));
            if (det2x2 < 0) {
                dst->fDirection = SkPath::OppositeDirection(static_cast<Direction>(fDirection));
            } else if (det2x2 > 0) {
                dst->fDirection = fDirection;
            } else {
                dst->fConvexity = kUnknown_Convexity;
                dst->fDirection = kUnknown_Direction;
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
    fPts = NULL;
    fConicWeights = NULL;
    fMoveTo.fX = fMoveTo.fY = fLastPt.fX = fLastPt.fY = 0;
    fForceClose = fCloseLine = false;
    fSegmentState = kEmptyContour_SegmentState;
#endif
    // need to init enough to make next() harmlessly return kDone_Verb
    fVerbs = NULL;
    fVerbStop = NULL;
    fNeedClose = false;
}

SkPath::Iter::Iter(const SkPath& path, bool forceClose) {
    this->setPath(path, forceClose);
}

void SkPath::Iter::setPath(const SkPath& path, bool forceClose) {
    fPts = path.fPathRef->points();
    fVerbs = path.fPathRef->verbs();
    fVerbStop = path.fPathRef->verbsMemBegin();
    fConicWeights = path.fPathRef->conicWeights() - 1; // begin one behind
    fLastPt.fX = fLastPt.fY = 0;
    fMoveTo.fX = fMoveTo.fY = 0;
    fForceClose = SkToU8(forceClose);
    fNeedClose = false;
    fSegmentState = kEmptyContour_SegmentState;
}

bool SkPath::Iter::isClosedContour() const {
    if (fVerbs == NULL || fVerbs == fVerbStop) {
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

void SkPath::Iter::consumeDegenerateSegments() {
    // We need to step over anything that will not move the current draw point
    // forward before the next move is seen
    const uint8_t* lastMoveVerb = 0;
    const SkPoint* lastMovePt = 0;
    SkPoint lastPt = fLastPt;
    while (fVerbs != fVerbStop) {
        unsigned verb = *(fVerbs - 1); // fVerbs is one beyond the current verb
        switch (verb) {
            case kMove_Verb:
                // Keep a record of this most recent move
                lastMoveVerb = fVerbs;
                lastMovePt = fPts;
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
                if (!IsLineDegenerate(lastPt, fPts[0])) {
                    if (lastMoveVerb) {
                        fVerbs = lastMoveVerb;
                        fPts = lastMovePt;
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
                if (!IsQuadDegenerate(lastPt, fPts[0], fPts[1])) {
                    if (lastMoveVerb) {
                        fVerbs = lastMoveVerb;
                        fPts = lastMovePt;
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
                if (!IsCubicDegenerate(lastPt, fPts[0], fPts[1], fPts[2])) {
                    if (lastMoveVerb) {
                        fVerbs = lastMoveVerb;
                        fPts = lastMovePt;
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

SkPath::RawIter::RawIter() {
#ifdef SK_DEBUG
    fPts = NULL;
    fConicWeights = NULL;
    fMoveTo.fX = fMoveTo.fY = fLastPt.fX = fLastPt.fY = 0;
#endif
    // need to init enough to make next() harmlessly return kDone_Verb
    fVerbs = NULL;
    fVerbStop = NULL;
}

SkPath::RawIter::RawIter(const SkPath& path) {
    this->setPath(path);
}

void SkPath::RawIter::setPath(const SkPath& path) {
    fPts = path.fPathRef->points();
    fVerbs = path.fPathRef->verbs();
    fVerbStop = path.fPathRef->verbsMemBegin();
    fConicWeights = path.fPathRef->conicWeights() - 1; // begin one behind
    fMoveTo.fX = fMoveTo.fY = 0;
    fLastPt.fX = fLastPt.fY = 0;
}

SkPath::Verb SkPath::RawIter::next(SkPoint pts[4]) {
    SkASSERT(NULL != pts);
    if (fVerbs == fVerbStop) {
        return kDone_Verb;
    }

    // fVerbs points one beyond next verb so decrement first.
    unsigned verb = *(--fVerbs);
    const SkPoint* srcPts = fPts;

    switch (verb) {
        case kMove_Verb:
            pts[0] = *srcPts;
            fMoveTo = srcPts[0];
            fLastPt = fMoveTo;
            srcPts += 1;
            break;
        case kLine_Verb:
            pts[0] = fLastPt;
            pts[1] = srcPts[0];
            fLastPt = srcPts[0];
            srcPts += 1;
            break;
        case kConic_Verb:
            fConicWeights += 1;
            // fall-through
        case kQuad_Verb:
            pts[0] = fLastPt;
            memcpy(&pts[1], srcPts, 2 * sizeof(SkPoint));
            fLastPt = srcPts[1];
            srcPts += 2;
            break;
        case kCubic_Verb:
            pts[0] = fLastPt;
            memcpy(&pts[1], srcPts, 3 * sizeof(SkPoint));
            fLastPt = srcPts[2];
            srcPts += 3;
            break;
        case kClose_Verb:
            fLastPt = fMoveTo;
            pts[0] = fMoveTo;
            break;
    }
    fPts = srcPts;
    return (Verb)verb;
}

///////////////////////////////////////////////////////////////////////////////

/*
    Format in compressed buffer: [ptCount, verbCount, pts[], verbs[]]
*/

size_t SkPath::writeToMemory(void* storage) const {
    SkDEBUGCODE(this->validate();)

    if (NULL == storage) {
        const int byteCount = sizeof(int32_t) + fPathRef->writeSize();
        return SkAlign4(byteCount);
    }

    SkWBuffer   buffer(storage);

    int32_t packed = (fConvexity << kConvexity_SerializationShift) |
                     (fFillType << kFillType_SerializationShift) |
                     (fDirection << kDirection_SerializationShift);

    buffer.write32(packed);

    fPathRef->writeToBuffer(&buffer);

    buffer.padToAlign4();
    return buffer.pos();
}

size_t SkPath::readFromMemory(const void* storage, size_t length) {
    SkRBufferWithSizeCheck buffer(storage, length);

    int32_t packed;
    if (!buffer.readS32(&packed)) {
        return 0;
    }

    fConvexity = (packed >> kConvexity_SerializationShift) & 0xFF;
    fFillType = (packed >> kFillType_SerializationShift) & 0xFF;
    fDirection = (packed >> kDirection_SerializationShift) & 0x3;
    SkPathRef* pathRef = SkPathRef::CreateFromBuffer(&buffer);

    size_t sizeRead = 0;
    if (buffer.isValid()) {
        fPathRef.reset(pathRef);
        SkDEBUGCODE(this->validate();)
        buffer.skipToAlign4();
        sizeRead = buffer.pos();
    } else if (NULL != pathRef) {
        // If the buffer is not valid, pathRef should be NULL
        sk_throw();
    }
    return sizeRead;
}

///////////////////////////////////////////////////////////////////////////////

#include "SkString.h"
#include "SkStream.h"

static void append_scalar(SkString* str, SkScalar value) {
    SkString tmp;
    tmp.printf("%g", value);
    if (tmp.contains('.')) {
        tmp.appendUnichar('f');
    }
    str->append(tmp);
}

static void append_params(SkString* str, const char label[], const SkPoint pts[],
                          int count, SkScalar conicWeight = -1) {
    str->append(label);
    str->append("(");

    const SkScalar* values = &pts[0].fX;
    count *= 2;

    for (int i = 0; i < count; ++i) {
        append_scalar(str, values[i]);
        if (i < count - 1) {
            str->append(", ");
        }
    }
    if (conicWeight >= 0) {
        str->append(", ");
        append_scalar(str, conicWeight);
    }
    str->append(");\n");
}

void SkPath::dump(SkWStream* wStream, bool forceClose) const {
    Iter    iter(*this, forceClose);
    SkPoint pts[4];
    Verb    verb;

    if (!wStream) {
        SkDebugf("path: forceClose=%s\n", forceClose ? "true" : "false");
    }
    SkString builder;

    while ((verb = iter.next(pts, false)) != kDone_Verb) {
        switch (verb) {
            case kMove_Verb:
                append_params(&builder, "path.moveTo", &pts[0], 1);
                break;
            case kLine_Verb:
                append_params(&builder, "path.lineTo", &pts[1], 1);
                break;
            case kQuad_Verb:
                append_params(&builder, "path.quadTo", &pts[1], 2);
                break;
            case kConic_Verb:
                append_params(&builder, "path.conicTo", &pts[1], 2, iter.conicWeight());
                break;
            case kCubic_Verb:
                append_params(&builder, "path.cubicTo", &pts[1], 3);
                break;
            case kClose_Verb:
                builder.append("path.close();\n");
                break;
            default:
                SkDebugf("  path: UNKNOWN VERB %d, aborting dump...\n", verb);
                verb = kDone_Verb;  // stop the loop
                break;
        }
    }
    if (wStream) {
        wStream->writeText(builder.c_str());
    } else {
        SkDebugf("%s", builder.c_str());
    }
}

void SkPath::dump() const {
    this->dump(NULL, false);
}

#ifdef SK_DEBUG
void SkPath::validate() const {
    SkASSERT(this != NULL);
    SkASSERT((fFillType & ~3) == 0);

#ifdef SK_DEBUG_PATH
    if (!fBoundsIsDirty) {
        SkRect bounds;

        bool isFinite = compute_pt_bounds(&bounds, *fPathRef.get());
        SkASSERT(SkToBool(fIsFinite) == isFinite);

        if (fPathRef->countPoints() <= 1) {
            // if we're empty, fBounds may be empty but translated, so we can't
            // necessarily compare to bounds directly
            // try path.addOval(2, 2, 2, 2) which is empty, but the bounds will
            // be [2, 2, 2, 2]
            SkASSERT(bounds.isEmpty());
            SkASSERT(fBounds.isEmpty());
        } else {
            if (bounds.isEmpty()) {
                SkASSERT(fBounds.isEmpty());
            } else {
                if (!fBounds.isEmpty()) {
                    SkASSERT(fBounds.contains(bounds));
                }
            }
        }
    }
#endif // SK_DEBUG_PATH
}
#endif // SK_DEBUG

///////////////////////////////////////////////////////////////////////////////

static int sign(SkScalar x) { return x < 0; }
#define kValueNeverReturnedBySign   2

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

// only valid for a single contour
struct Convexicator {
    Convexicator()
    : fPtCount(0)
    , fConvexity(SkPath::kConvex_Convexity)
    , fDirection(SkPath::kUnknown_Direction) {
        fSign = 0;
        // warnings
        fLastPt.set(0, 0);
        fCurrPt.set(0, 0);
        fLastVec.set(0, 0);
        fFirstVec.set(0, 0);

        fDx = fDy = 0;
        fSx = fSy = kValueNeverReturnedBySign;
    }

    SkPath::Convexity getConvexity() const { return fConvexity; }

    /** The direction returned is only valid if the path is determined convex */
    SkPath::Direction getDirection() const { return fDirection; }

    void addPt(const SkPoint& pt) {
        if (SkPath::kConcave_Convexity == fConvexity) {
            return;
        }

        if (0 == fPtCount) {
            fCurrPt = pt;
            ++fPtCount;
        } else {
            SkVector vec = pt - fCurrPt;
            if (vec.fX || vec.fY) {
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

private:
    void addVec(const SkVector& vec) {
        SkASSERT(vec.fX || vec.fY);
        SkScalar cross = SkPoint::CrossProduct(fLastVec, vec);
        SkScalar smallest = SkTMin(fCurrPt.fX, SkTMin(fCurrPt.fY, SkTMin(fLastPt.fX, fLastPt.fY)));
        SkScalar largest = SkTMax(fCurrPt.fX, SkTMax(fCurrPt.fY, SkTMax(fLastPt.fX, fLastPt.fY)));
        largest = SkTMax(largest, -smallest);
        if (!almost_equal(largest, largest + cross)) {
            int sign = SkScalarSignAsInt(cross);
            if (0 == fSign) {
                fSign = sign;
                fDirection = (1 == sign) ? SkPath::kCW_Direction : SkPath::kCCW_Direction;
            } else if (sign && fSign != sign) {
                fConvexity = SkPath::kConcave_Convexity;
                fDirection = SkPath::kUnknown_Direction;
            }
            fLastVec = vec;
        }
    }

    SkPoint             fLastPt;
    SkPoint             fCurrPt;
    // fLastVec does not necessarily start at fLastPt. We only advance it when the cross product
    // value with the current vec is deemed to be of a significant value.
    SkVector            fLastVec, fFirstVec;
    int                 fPtCount;   // non-degenerate points
    int                 fSign;
    SkPath::Convexity   fConvexity;
    SkPath::Direction   fDirection;
    int                 fDx, fDy, fSx, fSy;
};

SkPath::Convexity SkPath::internalGetConvexity() const {
    SkASSERT(kUnknown_Convexity == fConvexity);
    SkPoint         pts[4];
    SkPath::Verb    verb;
    SkPath::Iter    iter(*this, true);

    int             contourCount = 0;
    int             count;
    Convexicator    state;

    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        switch (verb) {
            case kMove_Verb:
                if (++contourCount > 1) {
                    fConvexity = kConcave_Convexity;
                    return kConcave_Convexity;
                }
                pts[1] = pts[0];
                count = 1;
                break;
            case kLine_Verb: count = 1; break;
            case kQuad_Verb: count = 2; break;
            case kConic_Verb: count = 2; break;
            case kCubic_Verb: count = 3; break;
            case kClose_Verb:
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
        if (kConcave_Convexity == state.getConvexity()) {
            fConvexity = kConcave_Convexity;
            return kConcave_Convexity;
        }
    }
    fConvexity = state.getConvexity();
    if (kConvex_Convexity == fConvexity && kUnknown_Direction == fDirection) {
        fDirection = state.getDirection();
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

static void crossToDir(SkScalar cross, SkPath::Direction* dir) {
    *dir = cross > 0 ? SkPath::kCW_Direction : SkPath::kCCW_Direction;
}

/*
 *  We loop through all contours, and keep the computed cross-product of the
 *  contour that contained the global y-max. If we just look at the first
 *  contour, we may find one that is wound the opposite way (correctly) since
 *  it is the interior of a hole (e.g. 'o'). Thus we must find the contour
 *  that is outer most (or at least has the global y-max) before we can consider
 *  its cross product.
 */
bool SkPath::cheapComputeDirection(Direction* dir) const {
    if (kUnknown_Direction != fDirection) {
        *dir = static_cast<Direction>(fDirection);
        return true;
    }

    // don't want to pay the cost for computing this if it
    // is unknown, so we don't call isConvex()
    if (kConvex_Convexity == this->getConvexityOrUnknown()) {
        SkASSERT(kUnknown_Direction == fDirection);
        *dir = static_cast<Direction>(fDirection);
        return false;
    }

    ContourIter iter(*fPathRef.get());

    // initialize with our logical y-min
    SkScalar ymax = this->getBounds().fTop;
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
        fDirection = *dir;
        return true;
    } else {
        return false;
    }
}

///////////////////////////////////////////////////////////////////////////////

static SkScalar eval_cubic_coeff(SkScalar A, SkScalar B, SkScalar C,
                                 SkScalar D, SkScalar t) {
    return SkScalarMulAdd(SkScalarMulAdd(SkScalarMulAdd(A, t, B), t, C), t, D);
}

static SkScalar eval_cubic_pts(SkScalar c0, SkScalar c1, SkScalar c2, SkScalar c3,
                               SkScalar t) {
    SkScalar A = c3 + 3*(c1 - c2) - c0;
    SkScalar B = 3*(c2 - c1 - c1 + c0);
    SkScalar C = 3*(c1 - c0);
    SkScalar D = c0;
    return eval_cubic_coeff(A, B, C, D, t);
}

/*  Given 4 cubic points (either Xs or Ys), and a target X or Y, compute the
 t value such that cubic(t) = target
 */
static void chopMonoCubicAt(SkScalar c0, SkScalar c1, SkScalar c2, SkScalar c3,
                            SkScalar target, SkScalar* t) {
    //   SkASSERT(c0 <= c1 && c1 <= c2 && c2 <= c3);
    SkASSERT(c0 < target && target < c3);

    SkScalar D = c0 - target;
    SkScalar A = c3 + 3*(c1 - c2) - c0;
    SkScalar B = 3*(c2 - c1 - c1 + c0);
    SkScalar C = 3*(c1 - c0);

    const SkScalar TOLERANCE = SK_Scalar1 / 4096;
    SkScalar minT = 0;
    SkScalar maxT = SK_Scalar1;
    SkScalar mid;
    int i;
    for (i = 0; i < 16; i++) {
        mid = SkScalarAve(minT, maxT);
        SkScalar delta = eval_cubic_coeff(A, B, C, D, mid);
        if (delta < 0) {
            minT = mid;
            delta = -delta;
        } else {
            maxT = mid;
        }
        if (delta < TOLERANCE) {
            break;
        }
    }
    *t = mid;
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

static int winding_mono_cubic(const SkPoint pts[], SkScalar x, SkScalar y) {
    SkPoint storage[4];

    int dir = 1;
    if (pts[0].fY > pts[3].fY) {
        storage[0] = pts[3];
        storage[1] = pts[2];
        storage[2] = pts[1];
        storage[3] = pts[0];
        pts = storage;
        dir = -1;
    }
    if (y < pts[0].fY || y >= pts[3].fY) {
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
    chopMonoCubicAt(pts[0].fY, pts[1].fY, pts[2].fY, pts[3].fY, y, &t);
    SkScalar xt = eval_cubic_pts(pts[0].fX, pts[1].fX, pts[2].fX, pts[3].fX, t);
    return xt < x ? dir : 0;
}

static int winding_cubic(const SkPoint pts[], SkScalar x, SkScalar y) {
    SkPoint dst[10];
    int n = SkChopCubicAtYExtrema(pts, dst);
    int w = 0;
    for (int i = 0; i <= n; ++i) {
        w += winding_mono_cubic(&dst[i * 3], x, y);
    }
    return w;
}

static int winding_mono_quad(const SkPoint pts[], SkScalar x, SkScalar y) {
    SkScalar y0 = pts[0].fY;
    SkScalar y2 = pts[2].fY;

    int dir = 1;
    if (y0 > y2) {
        SkTSwap(y0, y2);
        dir = -1;
    }
    if (y < y0 || y >= y2) {
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
        SkScalar mid = SkScalarAve(y0, y2);
        // Need [0] and [2] if dir == 1
        // and  [2] and [0] if dir == -1
        xt = y < mid ? pts[1 - dir].fX : pts[dir - 1].fX;
    } else {
        SkScalar t = roots[0];
        SkScalar C = pts[0].fX;
        SkScalar A = pts[2].fX - 2 * pts[1].fX + C;
        SkScalar B = 2 * (pts[1].fX - C);
        xt = SkScalarMulAdd(SkScalarMulAdd(A, t, B), t, C);
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

static int winding_quad(const SkPoint pts[], SkScalar x, SkScalar y) {
    SkPoint dst[5];
    int     n = 0;

    if (!is_mono_quad(pts[0].fY, pts[1].fY, pts[2].fY)) {
        n = SkChopQuadAtYExtrema(pts, dst);
        pts = dst;
    }
    int w = winding_mono_quad(pts, x, y);
    if (n > 0) {
        w += winding_mono_quad(&pts[2], x, y);
    }
    return w;
}

static int winding_line(const SkPoint pts[], SkScalar x, SkScalar y) {
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
    if (y < y0 || y >= y1) {
        return 0;
    }

    SkScalar cross = SkScalarMul(x1 - x0, y - pts[0].fY) -
    SkScalarMul(dy, x - pts[0].fX);

    if (SkScalarSignAsInt(cross) == dir) {
        dir = 0;
    }
    return dir;
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
    do {
        SkPoint pts[4];
        switch (iter.next(pts, false)) {
            case SkPath::kMove_Verb:
            case SkPath::kClose_Verb:
                break;
            case SkPath::kLine_Verb:
                w += winding_line(pts, x, y);
                break;
            case SkPath::kQuad_Verb:
                w += winding_quad(pts, x, y);
                break;
            case SkPath::kConic_Verb:
                SkASSERT(0);
                break;
            case SkPath::kCubic_Verb:
                w += winding_cubic(pts, x, y);
                break;
            case SkPath::kDone_Verb:
                done = true;
                break;
       }
    } while (!done);

    switch (this->getFillType()) {
        case SkPath::kEvenOdd_FillType:
        case SkPath::kInverseEvenOdd_FillType:
            w &= 1;
            break;
        default:
            break;
    }
    return SkToBool(w);
}
