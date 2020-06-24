/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPathBuilder.h"
#include "include/core/SkRRect.h"
#include "include/private/SkPathRef.h"
#include "include/private/SkSafe32.h"

SkPathBuilder::SkPathBuilder() {
    this->reset();
}

SkPathBuilder::~SkPathBuilder() {
}

SkPathBuilder& SkPathBuilder::reset() {
    fPts.reset();
    fVerbs.reset();
    fConicWeights.reset();
    fFillType = SkPathFillType::kWinding;
    fIsVolatile = false;

    // these are internal state

    fSegmentMask = 0;
    fLastMovePoint = {0, 0};
    fNeedsMoveVerb = true;

    return *this;
}

void SkPathBuilder::incReserve(int extraPtCount, int extraVbCount) {
    fPts.setReserve(  Sk32_sat_add(fPts.count(),   extraPtCount));
    fVerbs.setReserve(Sk32_sat_add(fVerbs.count(), extraVbCount));
}

/*
 *  Some old behavior in SkPath -- should we keep it?
 *
 *  After each edit (i.e. adding a verb)
        this->setConvexityType(SkPathConvexityType::kUnknown);
        this->setFirstDirection(SkPathPriv::kUnknown_FirstDirection);
 */

SkPathBuilder& SkPathBuilder::moveTo(SkPoint pt) {
    fPts.push_back(pt);
    fVerbs.push_back((uint8_t)SkPathVerb::kMove);

    fLastMovePoint = pt;
    fNeedsMoveVerb = false;
    return *this;
}

SkPathBuilder& SkPathBuilder::lineTo(SkPoint pt) {
    this->ensureMove();

    fPts.push_back(pt);
    fVerbs.push_back((uint8_t)SkPathVerb::kLine);

    fSegmentMask |= kLine_SkPathSegmentMask;
    return *this;
}

SkPathBuilder& SkPathBuilder::quadTo(SkPoint pt1, SkPoint pt2) {
    this->ensureMove();

    SkPoint* p = fPts.append(2);
    p[0] = pt1;
    p[1] = pt2;
    fVerbs.push_back((uint8_t)SkPathVerb::kQuad);

    fSegmentMask |= kQuad_SkPathSegmentMask;
    return *this;
}

SkPathBuilder& SkPathBuilder::conicTo(SkPoint pt1, SkPoint pt2, SkScalar w) {
    this->ensureMove();

    SkPoint* p = fPts.append(2);
    p[0] = pt1;
    p[1] = pt2;
    fVerbs.push_back((uint8_t)SkPathVerb::kConic);
    fConicWeights.push_back(w);

    fSegmentMask |= kConic_SkPathSegmentMask;
    return *this;
}

SkPathBuilder& SkPathBuilder::cubicTo(SkPoint pt1, SkPoint pt2, SkPoint pt3) {
    this->ensureMove();

    SkPoint* p = fPts.append(3);
    p[0] = pt1;
    p[1] = pt2;
    p[2] = pt3;
    fVerbs.push_back((uint8_t)SkPathVerb::kCubic);

    fSegmentMask |= kCubic_SkPathSegmentMask;
    return *this;
}

SkPathBuilder& SkPathBuilder::close() {
    this->ensureMove();

    fVerbs.push_back((uint8_t)SkPathVerb::kClose);

    // fLastMovePoint stays where it is -- the previous moveTo
    fNeedsMoveVerb = true;
    return *this;
}

SkPath SkPathBuilder::snapshot() {
    return SkPath(sk_sp<SkPathRef>(new SkPathRef(fPts, fVerbs, fConicWeights, fSegmentMask)),
                  fFillType, fIsVolatile);
}

SkPath SkPathBuilder::detach() {
    auto path = SkPath(sk_sp<SkPathRef>(new SkPathRef(std::move(fPts),
                                                      std::move(fVerbs),
                                                      std::move(fConicWeights),
                                                      fSegmentMask)),
                       fFillType, fIsVolatile);
    this->reset();
    return path;
}

///////////////////////////////////////////////////////////////////////////////////////////

namespace {
    template <unsigned N> class PointIterator {
    public:
        PointIterator(SkPathDirection dir, unsigned startIndex)
            : fCurrent(startIndex % N)
            , fAdvance(dir == SkPathDirection::kCW ? 1 : N - 1)
        {}

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
        RectPointIterator(const SkRect& rect, SkPathDirection dir, unsigned startIndex)
        : PointIterator(dir, startIndex) {

            fPts[0] = SkPoint::Make(rect.fLeft, rect.fTop);
            fPts[1] = SkPoint::Make(rect.fRight, rect.fTop);
            fPts[2] = SkPoint::Make(rect.fRight, rect.fBottom);
            fPts[3] = SkPoint::Make(rect.fLeft, rect.fBottom);
        }
    };

    class OvalPointIterator : public PointIterator<4> {
    public:
        OvalPointIterator(const SkRect& oval, SkPathDirection dir, unsigned startIndex)
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
        RRectPointIterator(const SkRRect& rrect, SkPathDirection dir, unsigned startIndex)
            : PointIterator(dir, startIndex)
        {
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


SkPathBuilder& SkPathBuilder::addRect(const SkRect& rect, SkPathDirection dir, unsigned index) {
    const int kPts   = 4;   // moveTo + 3 lines
    const int kVerbs = 5;   // moveTo + 3 lines + close
    this->incReserve(kPts, kVerbs);

    RectPointIterator iter(rect, dir, index);

    this->moveTo(iter.current());
    this->lineTo(iter.next());
    this->lineTo(iter.next());
    this->lineTo(iter.next());
    return this->close();
}

SkPathBuilder& SkPathBuilder::addOval(const SkRect& oval, SkPathDirection dir, unsigned index) {
    const int kPts   = 9;   // moveTo + 4 conics(2 pts each)
    const int kVerbs = 6;   // moveTo + 4 conics + close
    this->incReserve(kPts, kVerbs);

    OvalPointIterator ovalIter(oval, dir, index);
    RectPointIterator rectIter(oval, dir, index + (dir == SkPathDirection::kCW ? 0 : 1));

    // The corner iterator pts are tracking "behind" the oval/radii pts.

    this->moveTo(ovalIter.current());
    for (unsigned i = 0; i < 4; ++i) {
        this->conicTo(rectIter.next(), ovalIter.next(), SK_ScalarRoot2Over2);
    }
    return this->close();
}

SkPathBuilder& SkPathBuilder::addRRect(const SkRRect& rrect, SkPathDirection dir, unsigned index) {
    const SkRect& bounds = rrect.getBounds();

    if (rrect.isRect() || rrect.isEmpty()) {
        // degenerate(rect) => radii points are collapsing
        this->addRect(bounds, dir, (index + 1) / 2);
    } else if (rrect.isOval()) {
        // degenerate(oval) => line points are collapsing
        this->addOval(bounds, dir, index / 2);
    } else {
        // we start with a conic on odd indices when moving CW vs. even indices when moving CCW
        const bool startsWithConic = ((index & 1) == (dir == SkPathDirection::kCW));
        const SkScalar weight = SK_ScalarRoot2Over2;

        const int kVerbs = startsWithConic
            ? 9   // moveTo + 4x conicTo + 3x lineTo + close
            : 10; // moveTo + 4x lineTo + 4x conicTo + close
        this->incReserve(kVerbs);

        RRectPointIterator rrectIter(rrect, dir, index);
        // Corner iterator indices follow the collapsed radii model,
        // adjusted such that the start pt is "behind" the radii start pt.
        const unsigned rectStartIndex = index / 2 + (dir == SkPathDirection::kCW ? 0 : 1);
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
    }
    return *this;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

struct PathInfo {
    bool     valid;
    int      points, weights;
    unsigned segmentMask;
};

static PathInfo validate_verbs(const uint8_t vbs[], int verbCount) {
    PathInfo info = {false, 0, 0, 0};

    bool needMove = true;
    bool invalid = false;
    for (int i = 0; i < verbCount; ++i) {
        switch ((SkPathVerb)vbs[i]) {
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
    info.valid = !invalid;
    return info;
}

SkPath SkPathBuilder::Make(const SkPoint pts[], int pointCount,
                           const uint8_t vbs[], int verbCount,
                           const SkScalar ws[], int wCount,
                           SkPathFillType ft, bool isVolatile) {
    if (verbCount <= 0) {
        return SkPath();
    }

    const auto info = validate_verbs(vbs, verbCount);
    if (!info.valid || info.points > pointCount || info.weights > wCount) {
        SkDEBUGFAIL("invalid verbs and number of points/weights");
        return SkPath();
    }

    return SkPath(sk_sp<SkPathRef>(new SkPathRef(SkTDArray<SkPoint>(pts, info.points),
                                                 SkTDArray<uint8_t>(vbs, verbCount),
                                                 SkTDArray<SkScalar>(ws, info.weights),
                                                 info.segmentMask)),
                  ft, isVolatile);
}

