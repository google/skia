
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkPath.h"
#include "SkReader32.h"
#include "SkWriter32.h"
#include "SkMath.h"

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

class SkAutoDisableOvalCheck {
public:
    SkAutoDisableOvalCheck(SkPath* path) : fPath(path) {
        fSaved = fPath->fIsOval;
    }

    ~SkAutoDisableOvalCheck() {
        fPath->fIsOval = fSaved;
    }

private:
    SkPath* fPath;
    bool    fSaved;
};

/*  This guy's constructor/destructor bracket a path editing operation. It is
    used when we know the bounds of the amount we are going to add to the path
    (usually a new contour, but not required).

    It captures some state about the path up front (i.e. if it already has a
    cached bounds), and the if it can, it updates the cache bounds explicitly,
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
        fPath->setIsConvex(fDegenerate);
        if (fEmpty) {
            fPath->fBounds = fRect;
            fPath->fBoundsIsDirty = false;
        } else if (!fDirty) {
            joinNoEmptyChecks(&fPath->fBounds, fRect);
            fPath->fBoundsIsDirty = false;
        }
    }

private:
    SkPath* fPath;
    SkRect  fRect;
    bool    fDirty;
    bool    fDegenerate;
    bool    fEmpty;

    // returns true if we should proceed
    void init(SkPath* path) {
        fPath = path;
        fDirty = SkToBool(path->fBoundsIsDirty);
        fDegenerate = is_degenerate(*path);
        fEmpty = path->isEmpty();
        // Cannot use fRect for our bounds unless we know it is sorted
        fRect.sort();
    }
};

static void compute_pt_bounds(SkRect* bounds, const SkTDArray<SkPoint>& pts) {
    if (pts.count() <= 1) {  // we ignore just 1 point (moveto)
        bounds->set(0, 0, 0, 0);
    } else {
        bounds->set(pts.begin(), pts.count());
//        SkDebugf("------- compute bounds %p %d", &pts, pts.count());
    }
}

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
    : fFillType(kWinding_FillType)
    , fBoundsIsDirty(true) {
    fConvexity = kUnknown_Convexity;
    fSegmentMask = 0;
    fLastMoveToIndex = INITIAL_LASTMOVETOINDEX_VALUE;
    fIsOval = false;
#ifdef SK_BUILD_FOR_ANDROID
    fGenerationID = 0;
    fSourcePath = NULL;
#endif
}

SkPath::SkPath(const SkPath& src) {
    SkDEBUGCODE(src.validate();)
    *this = src;
#ifdef SK_BUILD_FOR_ANDROID
    // the assignment operator above increments the ID so correct for that here
    fGenerationID = src.fGenerationID;
    fSourcePath = NULL;
#endif
}

SkPath::~SkPath() {
    SkDEBUGCODE(this->validate();)
}

SkPath& SkPath::operator=(const SkPath& src) {
    SkDEBUGCODE(src.validate();)

    if (this != &src) {
        fBounds         = src.fBounds;
        fPts            = src.fPts;
        fVerbs          = src.fVerbs;
        fFillType       = src.fFillType;
        fBoundsIsDirty  = src.fBoundsIsDirty;
        fConvexity      = src.fConvexity;
        fSegmentMask    = src.fSegmentMask;
        fLastMoveToIndex = src.fLastMoveToIndex;
        fIsOval         = src.fIsOval;
        GEN_ID_INC;
    }
    SkDEBUGCODE(this->validate();)
    return *this;
}

bool operator==(const SkPath& a, const SkPath& b) {
    // note: don't need to look at isConvex or bounds, since just comparing the
    // raw data is sufficient.

    // We explicitly check fSegmentMask as a quick-reject. We could skip it,
    // since it is only a cache of info in the fVerbs, but its a fast way to
    // notice a difference

    return &a == &b ||
        (a.fFillType == b.fFillType && a.fSegmentMask == b.fSegmentMask &&
         a.fVerbs == b.fVerbs && a.fPts == b.fPts);
}

void SkPath::swap(SkPath& other) {
    SkASSERT(&other != NULL);

    if (this != &other) {
        SkTSwap<SkRect>(fBounds, other.fBounds);
        fPts.swap(other.fPts);
        fVerbs.swap(other.fVerbs);
        SkTSwap<uint8_t>(fFillType, other.fFillType);
        SkTSwap<uint8_t>(fBoundsIsDirty, other.fBoundsIsDirty);
        SkTSwap<uint8_t>(fConvexity, other.fConvexity);
        SkTSwap<uint8_t>(fSegmentMask, other.fSegmentMask);
        SkTSwap<int>(fLastMoveToIndex, other.fLastMoveToIndex);
        SkTSwap<SkBool8>(fIsOval, other.fIsOval);
        GEN_ID_INC;
    }
}

#ifdef SK_BUILD_FOR_ANDROID
uint32_t SkPath::getGenerationID() const {
    return fGenerationID;
}

const SkPath* SkPath::getSourcePath() const {
    return fSourcePath;
}

void SkPath::setSourcePath(const SkPath* path) {
    fSourcePath = path;
}
#endif

void SkPath::reset() {
    SkDEBUGCODE(this->validate();)

    fPts.reset();
    fVerbs.reset();
    GEN_ID_INC;
    fBoundsIsDirty = true;
    fConvexity = kUnknown_Convexity;
    fSegmentMask = 0;
    fLastMoveToIndex = INITIAL_LASTMOVETOINDEX_VALUE;
    fIsOval = false;
}

void SkPath::rewind() {
    SkDEBUGCODE(this->validate();)

    fPts.rewind();
    fVerbs.rewind();
    GEN_ID_INC;
    fConvexity = kUnknown_Convexity;
    fBoundsIsDirty = true;
    fSegmentMask = 0;
    fLastMoveToIndex = INITIAL_LASTMOVETOINDEX_VALUE;
    fIsOval = false;
}

bool SkPath::isEmpty() const {
    SkDEBUGCODE(this->validate();)
    return 0 == fVerbs.count();
}

bool SkPath::isLine(SkPoint line[2]) const {
    int verbCount = fVerbs.count();
    int ptCount = fPts.count();
    
    if (2 == verbCount && 2 == ptCount) {
        const uint8_t* verbs = fVerbs.begin();
        if (kMove_Verb == verbs[0] && kLine_Verb == verbs[1]) {
            if (line) {
                const SkPoint* pts = fPts.begin();
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
  1: horizontal right
  2: vertical down
  3: horizontal left
 
A rectangle cycles up/right/down/left or up/left/down/right.

The test fails if:
  The path is closed, and followed by a line.
  A second move creates a new endpoint.
  A diagonal line is parsed.
  There's more than four changes of direction.
  There's a discontinuity on the line (e.g., a move in the middle)
  The line reverses direction.
  The rectangle doesn't complete a cycle.
  The path contains a quadratic or cubic.
  The path contains fewer than four points.
  The final point isn't equal to the first point.
  
It's OK if the path has:
  Several colinear line segments composing a rectangle side.
  Single points on the rectangle side.
  
The direction takes advantage of the corners found since opposite sides
must travel in opposite directions.

FIXME: Allow colinear quads and cubics to be treated like lines.
FIXME: If the API passes fill-only, return true if the filled stroke
       is a rectangle, though the caller failed to close the path.
 */
bool SkPath::isRect(SkRect* rect) const {
    SkDEBUGCODE(this->validate();)

    int corners = 0;
    SkPoint first, last;
    first.set(0, 0);
    last.set(0, 0);
    int firstDirection = 0;
    int lastDirection = 0;
    int nextDirection = 0;
    bool closedOrMoved = false;
    bool autoClose = false;
    const uint8_t* verbs = fVerbs.begin();
    const uint8_t* verbStop = fVerbs.end();
    const SkPoint* pts = fPts.begin();
    while (verbs != verbStop) {
        switch (*verbs++) {
            case kClose_Verb:
                pts = fPts.begin();
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
                nextDirection = (left != right) << 0 |
                    (left < right || top < bottom) << 1;
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
            case kCubic_Verb:
                return false; // quadratic, cubic not allowed
            case kMove_Verb:
                last = *pts++;
                closedOrMoved = true;
                break;
        }
        lastDirection = nextDirection;
    }
    // Success if 4 corners and first point equals last
    bool result = 4 == corners && first == last;
    if (result && rect) {
        *rect = getBounds();
    }
    return result;
}

int SkPath::getPoints(SkPoint copy[], int max) const {
    SkDEBUGCODE(this->validate();)

    SkASSERT(max >= 0);
    int count = fPts.count();
    if (copy && max > 0 && count > 0) {
        memcpy(copy, fPts.begin(), sizeof(SkPoint) * SkMin32(max, count));
    }
    return count;
}

SkPoint SkPath::getPoint(int index) const {
    if ((unsigned)index < (unsigned)fPts.count()) {
        return fPts[index];
    }
    return SkPoint::Make(0, 0);
}

bool SkPath::getLastPt(SkPoint* lastPt) const {
    SkDEBUGCODE(this->validate();)

    int count = fPts.count();
    if (count > 0) {
        if (lastPt) {
            *lastPt = fPts[count - 1];
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

    int count = fPts.count();
    if (count == 0) {
        this->moveTo(x, y);
    } else {
        fIsOval = false;
        fPts[count - 1].set(x, y);
        GEN_ID_INC;
    }
}

void SkPath::computeBounds() const {
    SkDEBUGCODE(this->validate();)
    SkASSERT(fBoundsIsDirty);

    fBoundsIsDirty = false;
    compute_pt_bounds(&fBounds, fPts);
}

void SkPath::setConvexity(Convexity c) {
    if (fConvexity != c) {
        fConvexity = c;
        GEN_ID_INC;
    }
}

//////////////////////////////////////////////////////////////////////////////
//  Construction methods

#define DIRTY_AFTER_EDIT                 \
    do {                                 \
        fBoundsIsDirty = true;           \
        fConvexity = kUnknown_Convexity; \
        fIsOval = false;                 \
    } while (0)

#define DIRTY_AFTER_EDIT_NO_CONVEXITY_CHANGE    \
    do {                                        \
        fBoundsIsDirty = true;                  \
    } while (0)

void SkPath::incReserve(U16CPU inc) {
    SkDEBUGCODE(this->validate();)

    fVerbs.setReserve(fVerbs.count() + inc);
    fPts.setReserve(fPts.count() + inc);

    SkDEBUGCODE(this->validate();)
}

void SkPath::moveTo(SkScalar x, SkScalar y) {
    SkDEBUGCODE(this->validate();)

    int      vc = fVerbs.count();
    SkPoint* pt;

    // remember our index
    fLastMoveToIndex = fPts.count();

    pt = fPts.append();
    *fVerbs.append() = kMove_Verb;
    pt->set(x, y);

    GEN_ID_INC;
    DIRTY_AFTER_EDIT_NO_CONVEXITY_CHANGE;
}

void SkPath::rMoveTo(SkScalar x, SkScalar y) {
    SkPoint pt;
    this->getLastPt(&pt);
    this->moveTo(pt.fX + x, pt.fY + y);
}

void SkPath::injectMoveToIfNeeded() {
    if (fLastMoveToIndex < 0) {
        SkScalar x, y;
        if (fVerbs.count() == 0) {
            x = y = 0;
        } else {
            const SkPoint& pt = fPts[~fLastMoveToIndex];
            x = pt.fX;
            y = pt.fY;
        }
        this->moveTo(x, y);
    }
}

void SkPath::lineTo(SkScalar x, SkScalar y) {
    SkDEBUGCODE(this->validate();)

    this->injectMoveToIfNeeded();

    fPts.append()->set(x, y);
    *fVerbs.append() = kLine_Verb;
    fSegmentMask |= kLine_SegmentMask;

    GEN_ID_INC;
    DIRTY_AFTER_EDIT;
}

void SkPath::rLineTo(SkScalar x, SkScalar y) {
    SkPoint pt;
    this->getLastPt(&pt);
    this->lineTo(pt.fX + x, pt.fY + y);
}

void SkPath::quadTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2) {
    SkDEBUGCODE(this->validate();)

    this->injectMoveToIfNeeded();

    SkPoint* pts = fPts.append(2);
    pts[0].set(x1, y1);
    pts[1].set(x2, y2);
    *fVerbs.append() = kQuad_Verb;
    fSegmentMask |= kQuad_SegmentMask;

    GEN_ID_INC;
    DIRTY_AFTER_EDIT;
}

void SkPath::rQuadTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2) {
    SkPoint pt;
    this->getLastPt(&pt);
    this->quadTo(pt.fX + x1, pt.fY + y1, pt.fX + x2, pt.fY + y2);
}

void SkPath::cubicTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2,
                     SkScalar x3, SkScalar y3) {
    SkDEBUGCODE(this->validate();)

    this->injectMoveToIfNeeded();

    SkPoint* pts = fPts.append(3);
    pts[0].set(x1, y1);
    pts[1].set(x2, y2);
    pts[2].set(x3, y3);
    *fVerbs.append() = kCubic_Verb;
    fSegmentMask |= kCubic_SegmentMask;

    GEN_ID_INC;
    DIRTY_AFTER_EDIT;
}

void SkPath::rCubicTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2,
                      SkScalar x3, SkScalar y3) {
    SkPoint pt;
    this->getLastPt(&pt);
    this->cubicTo(pt.fX + x1, pt.fY + y1, pt.fX + x2, pt.fY + y2,
                  pt.fX + x3, pt.fY + y3);
}

void SkPath::close() {
    SkDEBUGCODE(this->validate();)

    int count = fVerbs.count();
    if (count > 0) {
        switch (fVerbs[count - 1]) {
            case kLine_Verb:
            case kQuad_Verb:
            case kCubic_Verb:
            case kMove_Verb:
                *fVerbs.append() = kClose_Verb;
                GEN_ID_INC;
                break;
            default:
                // don't add a close if it's the first verb or a repeat
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

void SkPath::addRect(const SkRect& rect, Direction dir) {
    this->addRect(rect.fLeft, rect.fTop, rect.fRight, rect.fBottom, dir);
}

void SkPath::addRect(SkScalar left, SkScalar top, SkScalar right,
                     SkScalar bottom, Direction dir) {
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

#define CUBIC_ARC_FACTOR    ((SK_ScalarSqrt2 - SK_Scalar1) * 4 / 3)

void SkPath::addRoundRect(const SkRect& rect, SkScalar rx, SkScalar ry,
                          Direction dir) {
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

    SkAutoPathBoundsUpdate apbu(this, rect);

    if (skip_hori) {
        rx = halfW;
    } else if (skip_vert) {
        ry = halfH;
    }

    SkScalar    sx = SkScalarMul(rx, CUBIC_ARC_FACTOR);
    SkScalar    sy = SkScalarMul(ry, CUBIC_ARC_FACTOR);

    this->incReserve(17);
    this->moveTo(rect.fRight - rx, rect.fTop);
    if (dir == kCCW_Direction) {
        if (!skip_hori) {
            this->lineTo(rect.fLeft + rx, rect.fTop);       // top
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
            this->lineTo(rect.fRight - rx, rect.fBottom);   // bottom
        }
        this->cubicTo(rect.fRight - rx + sx, rect.fBottom,
                      rect.fRight, rect.fBottom - ry + sy,
                      rect.fRight, rect.fBottom - ry);      // bot-right
        if (!skip_vert) {
            this->lineTo(rect.fRight, rect.fTop + ry);
        }
        this->cubicTo(rect.fRight, rect.fTop + ry - sy,
                      rect.fRight - rx + sx, rect.fTop,
                      rect.fRight - rx, rect.fTop);         // top-right
    } else {
        this->cubicTo(rect.fRight - rx + sx, rect.fTop,
                      rect.fRight, rect.fTop + ry - sy,
                      rect.fRight, rect.fTop + ry);         // top-right
        if (!skip_vert) {
            this->lineTo(rect.fRight, rect.fBottom - ry);
        }
        this->cubicTo(rect.fRight, rect.fBottom - ry + sy,
                      rect.fRight - rx + sx, rect.fBottom,
                      rect.fRight - rx, rect.fBottom);      // bot-right
        if (!skip_hori) {
            this->lineTo(rect.fLeft + rx, rect.fBottom);    // bottom
        }
        this->cubicTo(rect.fLeft + rx - sx, rect.fBottom,
                      rect.fLeft, rect.fBottom - ry + sy,
                      rect.fLeft, rect.fBottom - ry);       // bot-left
        if (!skip_vert) {
            this->lineTo(rect.fLeft, rect.fTop + ry);       // left
        }
        this->cubicTo(rect.fLeft, rect.fTop + ry - sy,
                      rect.fLeft + rx - sx, rect.fTop,
                      rect.fLeft + rx, rect.fTop);          // top-left
        if (!skip_hori) {
            this->lineTo(rect.fRight - rx, rect.fTop);      // top
        }
    }
    this->close();
}

static void add_corner_arc(SkPath* path, const SkRect& rect,
                           SkScalar rx, SkScalar ry, int startAngle,
                           SkPath::Direction dir, bool forceMoveTo) {
    rx = SkMinScalar(SkScalarHalf(rect.width()), rx);
    ry = SkMinScalar(SkScalarHalf(rect.height()), ry);

    SkRect   r;
    r.set(-rx, -ry, rx, ry);

    switch (startAngle) {
        case   0:
            r.offset(rect.fRight - r.fRight, rect.fBottom - r.fBottom);
            break;
        case  90:
            r.offset(rect.fLeft - r.fLeft,   rect.fBottom - r.fBottom);
            break;
        case 180: r.offset(rect.fLeft - r.fLeft,   rect.fTop - r.fTop); break;
        case 270: r.offset(rect.fRight - r.fRight, rect.fTop - r.fTop); break;
        default: SkDEBUGFAIL("unexpected startAngle in add_corner_arc");
    }

    SkScalar start = SkIntToScalar(startAngle);
    SkScalar sweep = SkIntToScalar(90);
    if (SkPath::kCCW_Direction == dir) {
        start += sweep;
        sweep = -sweep;
    }

    path->arcTo(r, start, sweep, forceMoveTo);
}

void SkPath::addRoundRect(const SkRect& rect, const SkScalar rad[],
                          Direction dir) {
    // abort before we invoke SkAutoPathBoundsUpdate()
    if (rect.isEmpty()) {
        return;
    }

    SkAutoPathBoundsUpdate apbu(this, rect);

    if (kCW_Direction == dir) {
        add_corner_arc(this, rect, rad[0], rad[1], 180, dir, true);
        add_corner_arc(this, rect, rad[2], rad[3], 270, dir, false);
        add_corner_arc(this, rect, rad[4], rad[5],   0, dir, false);
        add_corner_arc(this, rect, rad[6], rad[7],  90, dir, false);
    } else {
        add_corner_arc(this, rect, rad[0], rad[1], 180, dir, true);
        add_corner_arc(this, rect, rad[6], rad[7],  90, dir, false);
        add_corner_arc(this, rect, rad[4], rad[5],   0, dir, false);
        add_corner_arc(this, rect, rad[2], rad[3], 270, dir, false);
    }
    this->close();
}

bool SkPath::hasOnlyMoveTos() const {
    const uint8_t* verbs = fVerbs.begin();
    const uint8_t* verbStop = fVerbs.end();
    while (verbs != verbStop) {
        if (*verbs == kLine_Verb ||
            *verbs == kQuad_Verb ||
            *verbs == kCubic_Verb) {
            return false;
        }
        ++verbs;
    }
    return true;
}

void SkPath::addOval(const SkRect& oval, Direction dir) {
    /* If addOval() is called after previous moveTo(),
       this path is still marked as an oval. This is used to
       fit into WebKit's calling sequences.
       We can't simply check isEmpty() in this case, as additional
       moveTo() would mark the path non empty.
     */
    fIsOval = hasOnlyMoveTos();

    SkAutoDisableOvalCheck adoc(this);

    SkAutoPathBoundsUpdate apbu(this, oval);

    SkScalar    cx = oval.centerX();
    SkScalar    cy = oval.centerY();
    SkScalar    rx = SkScalarHalf(oval.width());
    SkScalar    ry = SkScalarHalf(oval.height());
#if 0   // these seem faster than using quads (1/2 the number of edges)
    SkScalar    sx = SkScalarMul(rx, CUBIC_ARC_FACTOR);
    SkScalar    sy = SkScalarMul(ry, CUBIC_ARC_FACTOR);

    this->incReserve(13);
    this->moveTo(cx + rx, cy);
    if (dir == kCCW_Direction) {
        this->cubicTo(cx + rx, cy - sy, cx + sx, cy - ry, cx, cy - ry);
        this->cubicTo(cx - sx, cy - ry, cx - rx, cy - sy, cx - rx, cy);
        this->cubicTo(cx - rx, cy + sy, cx - sx, cy + ry, cx, cy + ry);
        this->cubicTo(cx + sx, cy + ry, cx + rx, cy + sy, cx + rx, cy);
    } else {
        this->cubicTo(cx + rx, cy + sy, cx + sx, cy + ry, cx, cy + ry);
        this->cubicTo(cx - sx, cy + ry, cx - rx, cy + sy, cx - rx, cy);
        this->cubicTo(cx - rx, cy - sy, cx - sx, cy - ry, cx, cy - ry);
        this->cubicTo(cx + sx, cy - ry, cx + rx, cy - sy, cx + rx, cy);
    }
#else
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
#endif
    this->close();
}

bool SkPath::isOval(SkRect* rect) const {
    if (fIsOval && rect) {
        *rect = getBounds();
    }

    return fIsOval;
}

void SkPath::addCircle(SkScalar x, SkScalar y, SkScalar r, Direction dir) {
    if (r > 0) {
        SkRect  rect;
        rect.set(x - r, y - r, x + r, y + r);
        this->addOval(rect, dir);
    }
}

#include "SkGeometry.h"

static int build_arc_points(const SkRect& oval, SkScalar startAngle,
                            SkScalar sweepAngle,
                            SkPoint pts[kSkBuildQuadArcStorage]) {
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
          sweepAngle > 0 ? kCW_SkRotationDirection : kCCW_SkRotationDirection,
                          &matrix, pts);
}

void SkPath::arcTo(const SkRect& oval, SkScalar startAngle, SkScalar sweepAngle,
                   bool forceMoveTo) {
    if (oval.width() < 0 || oval.height() < 0) {
        return;
    }

    SkPoint pts[kSkBuildQuadArcStorage];
    int count = build_arc_points(oval, startAngle, sweepAngle, pts);
    SkASSERT((count & 1) == 1);

    if (fVerbs.count() == 0) {
        forceMoveTo = true;
    }
    this->incReserve(count);
    forceMoveTo ? this->moveTo(pts[0]) : this->lineTo(pts[0]);
    for (int i = 1; i < count; i += 2) {
        this->quadTo(pts[i], pts[i+1]);
    }
}

void SkPath::addArc(const SkRect& oval, SkScalar startAngle,
                    SkScalar sweepAngle) {
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

    this->incReserve(count);
    this->moveTo(pts[0]);
    for (int i = 1; i < count; i += 2) {
        this->quadTo(pts[i], pts[i+1]);
    }
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

void SkPath::addPath(const SkPath& path, SkScalar dx, SkScalar dy) {
    SkMatrix matrix;

    matrix.setTranslate(dx, dy);
    this->addPath(path, matrix);
}

void SkPath::addPath(const SkPath& path, const SkMatrix& matrix) {
    this->incReserve(path.fPts.count());

    fIsOval = false;

    RawIter iter(path);
    SkPoint pts[4];
    Verb    verb;

    SkMatrix::MapPtsProc proc = matrix.getMapPtsProc();

    while ((verb = iter.next(pts)) != kDone_Verb) {
        switch (verb) {
            case kMove_Verb:
                proc(matrix, &pts[0], &pts[0], 1);
                this->moveTo(pts[0]);
                break;
            case kLine_Verb:
                proc(matrix, &pts[1], &pts[1], 1);
                this->lineTo(pts[1]);
                break;
            case kQuad_Verb:
                proc(matrix, &pts[1], &pts[1], 2);
                this->quadTo(pts[1], pts[2]);
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
    }
}

///////////////////////////////////////////////////////////////////////////////

static const uint8_t gPtsInVerb[] = {
    1,  // kMove
    1,  // kLine
    2,  // kQuad
    3,  // kCubic
    0,  // kClose
    0   // kDone
};

// ignore the initial moveto, and stop when the 1st contour ends
void SkPath::pathTo(const SkPath& path) {
    int i, vcount = path.fVerbs.count();
    if (vcount == 0) {
        return;
    }

    this->incReserve(vcount);

    fIsOval = false;

    const uint8_t*  verbs = path.fVerbs.begin();
    const SkPoint*  pts = path.fPts.begin() + 1;    // 1 for the initial moveTo

    SkASSERT(verbs[0] == kMove_Verb);
    for (i = 1; i < vcount; i++) {
        switch (verbs[i]) {
            case kLine_Verb:
                this->lineTo(pts[0].fX, pts[0].fY);
                break;
            case kQuad_Verb:
                this->quadTo(pts[0].fX, pts[0].fY, pts[1].fX, pts[1].fY);
                break;
            case kCubic_Verb:
                this->cubicTo(pts[0].fX, pts[0].fY, pts[1].fX, pts[1].fY,
                              pts[2].fX, pts[2].fY);
                break;
            case kClose_Verb:
                return;
        }
        pts += gPtsInVerb[verbs[i]];
    }
}

// ignore the last point of the 1st contour
void SkPath::reversePathTo(const SkPath& path) {
    int i, vcount = path.fVerbs.count();
    if (vcount == 0) {
        return;
    }

    this->incReserve(vcount);

    fIsOval = false;

    const uint8_t*  verbs = path.fVerbs.begin();
    const SkPoint*  pts = path.fPts.begin();

    SkASSERT(verbs[0] == kMove_Verb);
    for (i = 1; i < vcount; i++) {
        int n = gPtsInVerb[verbs[i]];
        if (n == 0) {
            break;
        }
        pts += n;
    }

    while (--i > 0) {
        switch (verbs[i]) {
            case kLine_Verb:
                this->lineTo(pts[-1].fX, pts[-1].fY);
                break;
            case kQuad_Verb:
                this->quadTo(pts[-1].fX, pts[-1].fY, pts[-2].fX, pts[-2].fY);
                break;
            case kCubic_Verb:
                this->cubicTo(pts[-1].fX, pts[-1].fY, pts[-2].fX, pts[-2].fY,
                              pts[-3].fX, pts[-3].fY);
                break;
            default:
                SkDEBUGFAIL("bad verb");
                break;
        }
        pts -= gPtsInVerb[verbs[i]];
    }
}

void SkPath::reverseAddPath(const SkPath& src) {
    this->incReserve(src.fPts.count());

    const SkPoint* startPts = src.fPts.begin();
    const SkPoint* pts = src.fPts.end();
    const uint8_t* startVerbs = src.fVerbs.begin();
    const uint8_t* verbs = src.fVerbs.end();

    fIsOval = false;

    bool needMove = true;
    bool needClose = false;
    while (verbs > startVerbs) {
        uint8_t v = *--verbs;
        int n = gPtsInVerb[v];

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
            case kCubic_Verb:
                this->cubicTo(pts[2], pts[1], pts[0]);
                break;
            case kClose_Verb:
                needClose = true;
                break;
            default:
                SkASSERT(!"unexpected verb");
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
        matrix.mapPoints(dst->fPts.begin(), dst->fPts.count());
    } else {
        // remember that dst might == this, so be sure to check
        // fBoundsIsDirty before we set it
        if (!fBoundsIsDirty && matrix.rectStaysRect() && fPts.count() > 1) {
            // if we're empty, fastbounds should not be mapped
            matrix.mapRect(&dst->fBounds, fBounds);
            dst->fBoundsIsDirty = false;
        } else {
            GEN_ID_PTR_INC(dst);
            dst->fBoundsIsDirty = true;
        }

        if (this != dst) {
            dst->fVerbs = fVerbs;
            dst->fPts.setCount(fPts.count());
            dst->fFillType = fFillType;
            dst->fSegmentMask = fSegmentMask;
            dst->fConvexity = fConvexity;
            dst->fIsOval = fIsOval;
        }

        matrix.mapPoints(dst->fPts.begin(), fPts.begin(), fPts.count());

        if (fIsOval) {
            // It's an oval only if it stays a rect.
            dst->fIsOval = matrix.rectStaysRect();
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
    fPts = path.fPts.begin();
    fVerbs = path.fVerbs.begin();
    fVerbStop = path.fVerbs.end();
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

    if (kMove_Verb == *verbs) {
        verbs += 1; // skip the initial moveto
    }

    while (verbs < stop) {
        unsigned v = *verbs++;
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
        unsigned verb = *fVerbs;
        switch (verb) {
            case kMove_Verb:
                // Keep a record of this most recent move
                lastMoveVerb = fVerbs;
                lastMovePt = fPts;
                lastPt = fPts[0];
                fVerbs++;
                fPts++;
                break;

            case kClose_Verb:
                // A close when we are in a segment is always valid
                if (fSegmentState == kAfterPrimitive_SegmentState) {
                    return;
                }
                // A close at any other time must be ignored
                fVerbs++;
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
                fVerbs++;
                fPts++;
                break;

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
                fVerbs++;
                fPts += 2;
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
                fVerbs++;
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

    unsigned        verb = *fVerbs++;
    const SkPoint* SK_RESTRICT srcPts = fPts;
    SkPoint* SK_RESTRICT       pts = ptsParam;

    switch (verb) {
        case kMove_Verb:
            if (fNeedClose) {
                fVerbs -= 1;
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
                fVerbs -= 1;
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
    fPts = path.fPts.begin();
    fVerbs = path.fVerbs.begin();
    fVerbStop = path.fVerbs.end();
    fMoveTo.fX = fMoveTo.fY = 0;
    fLastPt.fX = fLastPt.fY = 0;
}

SkPath::Verb SkPath::RawIter::next(SkPoint pts[4]) {
    if (fVerbs == fVerbStop) {
        return kDone_Verb;
    }

    unsigned        verb = *fVerbs++;
    const SkPoint*  srcPts = fPts;

    switch (verb) {
        case kMove_Verb:
            if (pts) {
                pts[0] = *srcPts;
            }
            fMoveTo = srcPts[0];
            fLastPt = fMoveTo;
            srcPts += 1;
            break;
        case kLine_Verb:
            if (pts) {
                pts[0] = fLastPt;
                pts[1] = srcPts[0];
            }
            fLastPt = srcPts[0];
            srcPts += 1;
            break;
        case kQuad_Verb:
            if (pts) {
                pts[0] = fLastPt;
                memcpy(&pts[1], srcPts, 2 * sizeof(SkPoint));
            }
            fLastPt = srcPts[1];
            srcPts += 2;
            break;
        case kCubic_Verb:
            if (pts) {
                pts[0] = fLastPt;
                memcpy(&pts[1], srcPts, 3 * sizeof(SkPoint));
            }
            fLastPt = srcPts[2];
            srcPts += 3;
            break;
        case kClose_Verb:
            fLastPt = fMoveTo;
            if (pts) {
                pts[0] = fMoveTo;
            }
            break;
    }
    fPts = srcPts;
    return (Verb)verb;
}

///////////////////////////////////////////////////////////////////////////////

/*
    Format in flattened buffer: [ptCount, verbCount, pts[], verbs[]]
*/

void SkPath::flatten(SkWriter32& buffer) const {
    SkDEBUGCODE(this->validate();)

    buffer.write32(fPts.count());
    buffer.write32(fVerbs.count());
    buffer.write32((fFillType << 8) | fSegmentMask);
    buffer.writeMul4(fPts.begin(), sizeof(SkPoint) * fPts.count());
    buffer.writePad(fVerbs.begin(), fVerbs.count());
}

void SkPath::unflatten(SkReader32& buffer) {
    fPts.setCount(buffer.readS32());
    fVerbs.setCount(buffer.readS32());
    uint32_t packed = buffer.readS32();
    fFillType = packed >> 8;
    fSegmentMask = packed & 0xFF;
    buffer.read(fPts.begin(), sizeof(SkPoint) * fPts.count());
    buffer.read(fVerbs.begin(), fVerbs.count());

    GEN_ID_INC;
    DIRTY_AFTER_EDIT;

    SkDEBUGCODE(this->validate();)
}

///////////////////////////////////////////////////////////////////////////////

void SkPath::dump(bool forceClose, const char title[]) const {
    Iter    iter(*this, forceClose);
    SkPoint pts[4];
    Verb    verb;

    SkDebugf("path: forceClose=%s %s\n", forceClose ? "true" : "false",
             title ? title : "");

    while ((verb = iter.next(pts, false)) != kDone_Verb) {
        switch (verb) {
            case kMove_Verb:
#ifdef SK_CAN_USE_FLOAT
                SkDebugf("  path: moveTo [%g %g]\n",
                        SkScalarToFloat(pts[0].fX), SkScalarToFloat(pts[0].fY));
#else
                SkDebugf("  path: moveTo [%x %x]\n", pts[0].fX, pts[0].fY);
#endif
                break;
            case kLine_Verb:
#ifdef SK_CAN_USE_FLOAT
                SkDebugf("  path: lineTo [%g %g]\n",
                        SkScalarToFloat(pts[1].fX), SkScalarToFloat(pts[1].fY));
#else
                SkDebugf("  path: lineTo [%x %x]\n", pts[1].fX, pts[1].fY);
#endif
                break;
            case kQuad_Verb:
#ifdef SK_CAN_USE_FLOAT
                SkDebugf("  path: quadTo [%g %g] [%g %g]\n",
                        SkScalarToFloat(pts[1].fX), SkScalarToFloat(pts[1].fY),
                        SkScalarToFloat(pts[2].fX), SkScalarToFloat(pts[2].fY));
#else
                SkDebugf("  path: quadTo [%x %x] [%x %x]\n",
                         pts[1].fX, pts[1].fY, pts[2].fX, pts[2].fY);
#endif
                break;
            case kCubic_Verb:
#ifdef SK_CAN_USE_FLOAT
                SkDebugf("  path: cubeTo [%g %g] [%g %g] [%g %g]\n",
                        SkScalarToFloat(pts[1].fX), SkScalarToFloat(pts[1].fY),
                        SkScalarToFloat(pts[2].fX), SkScalarToFloat(pts[2].fY),
                        SkScalarToFloat(pts[3].fX), SkScalarToFloat(pts[3].fY));
#else
                SkDebugf("  path: cubeTo [%x %x] [%x %x] [%x %x]\n",
                         pts[1].fX, pts[1].fY, pts[2].fX, pts[2].fY,
                         pts[3].fX, pts[3].fY);
#endif
                break;
            case kClose_Verb:
                SkDebugf("  path: close\n");
                break;
            default:
                SkDebugf("  path: UNKNOWN VERB %d, aborting dump...\n", verb);
                verb = kDone_Verb;  // stop the loop
                break;
        }
    }
    SkDebugf("path: done %s\n", title ? title : "");
}

void SkPath::dump() const {
    this->dump(false);
}

#ifdef SK_DEBUG
void SkPath::validate() const {
    SkASSERT(this != NULL);
    SkASSERT((fFillType & ~3) == 0);
    fPts.validate();
    fVerbs.validate();

    if (!fBoundsIsDirty) {
        SkRect bounds;
        compute_pt_bounds(&bounds, fPts);
        if (fPts.count() <= 1) {
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

    uint32_t mask = 0;
    for (int i = 0; i < fVerbs.count(); i++) {
        switch (fVerbs[i]) {
            case kLine_Verb:
                mask |= kLine_SegmentMask;
                break;
            case kQuad_Verb:
                mask |= kQuad_SegmentMask;
                break;
            case kCubic_Verb:
                mask |= kCubic_SegmentMask;
        }
    }
    SkASSERT(mask == fSegmentMask);
}
#endif

///////////////////////////////////////////////////////////////////////////////

static int sign(SkScalar x) { return x < 0; }
#define kValueNeverReturnedBySign   2

static int CrossProductSign(const SkVector& a, const SkVector& b) {
    return SkScalarSignAsInt(SkPoint::CrossProduct(a, b));
}

// only valid for a single contour
struct Convexicator {
    Convexicator() : fPtCount(0), fConvexity(SkPath::kConvex_Convexity) {
        fSign = 0;
        // warnings
        fCurrPt.set(0, 0);
        fVec0.set(0, 0);
        fVec1.set(0, 0);
        fFirstVec.set(0, 0);

        fDx = fDy = 0;
        fSx = fSy = kValueNeverReturnedBySign;
    }

    SkPath::Convexity getConvexity() const { return fConvexity; }

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
                fCurrPt = pt;
                if (++fPtCount == 2) {
                    fFirstVec = fVec1 = vec;
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
        fVec0 = fVec1;
        fVec1 = vec;
        int sign = CrossProductSign(fVec0, fVec1);
        if (0 == fSign) {
            fSign = sign;
        } else if (sign) {
            if (fSign != sign) {
                fConvexity = SkPath::kConcave_Convexity;
            }
        }
    }

    SkPoint             fCurrPt;
    SkVector            fVec0, fVec1, fFirstVec;
    int                 fPtCount;   // non-degenerate points
    int                 fSign;
    SkPath::Convexity   fConvexity;
    int                 fDx, fDy, fSx, fSy;
};

SkPath::Convexity SkPath::ComputeConvexity(const SkPath& path) {
    SkPoint         pts[4];
    SkPath::Verb    verb;
    SkPath::Iter    iter(path, true);

    int             contourCount = 0;
    int             count;
    Convexicator    state;

    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        switch (verb) {
            case kMove_Verb:
                if (++contourCount > 1) {
                    return kConcave_Convexity;
                }
                pts[1] = pts[0];
                count = 1;
                break;
            case kLine_Verb: count = 1; break;
            case kQuad_Verb: count = 2; break;
            case kCubic_Verb: count = 3; break;
            case kClose_Verb:
                state.close();
                count = 0;
                break;
            default:
                SkDEBUGFAIL("bad verb");
                return kConcave_Convexity;
        }

        for (int i = 1; i <= count; i++) {
            state.addPt(pts[i]);
        }
        // early exit
        if (kConcave_Convexity == state.getConvexity()) {
            return kConcave_Convexity;
        }
    }
    return state.getConvexity();
}

///////////////////////////////////////////////////////////////////////////////

class ContourIter {
public:
    ContourIter(const SkTDArray<uint8_t>& verbs, const SkTDArray<SkPoint>& pts);

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
    bool fDone;
    SkDEBUGCODE(int fContourCounter;)
};

ContourIter::ContourIter(const SkTDArray<uint8_t>& verbs,
                         const SkTDArray<SkPoint>& pts) {
    fStopVerbs = verbs.begin() + verbs.count();
    
    fDone = false;
    fCurrPt = pts.begin();
    fCurrVerb = verbs.begin();
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

    SkASSERT(SkPath::kMove_Verb == fCurrVerb[0]);
    int ptCount = 1;    // moveTo
    const uint8_t* verbs = fCurrVerb;

    for (++verbs; verbs < fStopVerbs; ++verbs) {
        switch (*verbs) {
            case SkPath::kMove_Verb:
                goto CONTOUR_END;
            case SkPath::kLine_Verb:
                ptCount += 1;
                break;
            case SkPath::kQuad_Verb:
                ptCount += 2;
                break;
            case SkPath::kCubic_Verb:
                ptCount += 3;
                break;
            default:    // kClose_Verb, just keep going
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

static bool crossToDir(SkScalar cross, SkPath::Direction* dir) {
    if (dir) {
        *dir = cross > 0 ? SkPath::kCW_Direction : SkPath::kCCW_Direction;
    }
    return true;
}

#if 0
#include "SkString.h"
#include "../utils/SkParsePath.h"
static void dumpPath(const SkPath& path) {
    SkString str;
    SkParsePath::ToSVGString(path, &str);
    SkDebugf("%s\n", str.c_str());
}
#endif

/*
 *  We loop through all contours, and keep the computed cross-product of the
 *  contour that contained the global y-max. If we just look at the first
 *  contour, we may find one that is wound the opposite way (correctly) since
 *  it is the interior of a hole (e.g. 'o'). Thus we must find the contour
 *  that is outer most (or at least has the global y-max) before we can consider
 *  its cross product.
 */
bool SkPath::cheapComputeDirection(Direction* dir) const {
//    dumpPath(*this);
    // don't want to pay the cost for computing this if it
    // is unknown, so we don't call isConvex()
    const Convexity conv = this->getConvexityOrUnknown();

    ContourIter iter(fVerbs, fPts);

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
        if (kConvex_Convexity == conv) {
            // we loop, skipping over degenerate or flat segments that will
            // return 0 for the cross-product
            for (int i = 0; i < n - 2; ++i) {
                cross = cross_prod(pts[i], pts[i + 1], pts[i + 2]);
                if (cross) {
                    // early-exit, as kConvex is assumed to have only 1
                    // non-degenerate contour
                    return crossToDir(cross, dir);
                }
            }
        } else {
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
                // if we get a zero, but the pts aren't on top of each other, then
                // we can just look at the direction
                if (0 == cross) {
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
    }

    return ymaxCross ? crossToDir(ymaxCross, dir) : false;
}
