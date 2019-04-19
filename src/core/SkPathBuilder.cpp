/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPathBuilder.h"
#include "SkRRect.h"
#include "SkSafe32.h"

SkPathBuilder::SkPathBuilder() {
}

SkPathBuilder::~SkPathBuilder() {
}

void SkPathBuilder::incReserve(int extraPtCount) {
    fPts.setReserve(  Sk32_sat_add(fPts.count(),   extraPtCount));
    fVerbs.setReserve(Sk32_sat_add(fVerbs.count(), extraPtCount));
}

SkPathBuilder& SkPathBuilder::moveTo(SkPoint pt) {
    fPts.push_back(pt);
    fVerbs.push_back(kMove_SkPathVerb);
    return *this;
}

SkPathBuilder& SkPathBuilder::lineTo(SkPoint pt) {
    fPts.push_back(pt);
    fVerbs.push_back(kLine_SkPathVerb);
    fSegmentMask |= kLine_SkPathSegmentMask;
    return *this;
}

SkPathBuilder& SkPathBuilder::quadTo(SkPoint pt1, SkPoint pt2) {
    fPts.push_back(pt1);
    fPts.push_back(pt2);
    fVerbs.push_back(kQuad_SkPathVerb);
    fSegmentMask |= kQuad_SkPathSegmentMask;
    return *this;
}

SkPathBuilder& SkPathBuilder::conicTo(SkPoint pt1, SkPoint pt2, SkScalar w) {
    fPts.push_back(pt1);
    fPts.push_back(pt2);
    fVerbs.push_back(kConic_SkPathVerb);
    fConicWeights.push_back(w);
    fSegmentMask |= kConic_SkPathSegmentMask;
    return *this;
}

SkPathBuilder& SkPathBuilder::cubicTo(SkPoint pt1, SkPoint pt2, SkPoint pt3) {
    fPts.push_back(pt1);
    fPts.push_back(pt2);
    fPts.push_back(pt3);
    fVerbs.push_back(kCubic_SkPathVerb);
    fSegmentMask |= kCubic_SkPathSegmentMask;
    return *this;
}

SkPathBuilder& SkPathBuilder::close() {
    fVerbs.push_back(kClose_SkPathVerb);
    return *this;
}

SkPath SkPathBuilder::snapshot() {
 //   return SkPath(fPts, fVerbs, fConicWeights);
    return SkPath();
}

SkPath SkPathBuilder::detach() {
//    return SkPath(std::move(fPts), std::move(fVerbs), std::move(fConicWeights));
    return SkPath();
}

///////////////////////////////////////////////////////////////////////////////////////////

namespace {
    template <unsigned N> class PointIterator {
    public:
        PointIterator(SkPathDirection dir, unsigned startIndex)
            : fCurrent(startIndex % N)
            , fAdvance(dir == kCW_SkPathDirection ? 1 : N - 1)
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

    static void assert_known_direction(int dir) {
        SkASSERT(kCW_SkPathDirection == dir || kCCW_SkPathDirection == dir);
    }
} // anonymous namespace


SkPathBuilder& SkPathBuilder::addRect(const SkRect& rect, SkPathDirection dir, unsigned index) {
    assert_known_direction(dir);

#if 0
    fFirstDirection = this->hasOnlyMoveTos() ?
    (SkPathPriv::FirstDirection)dir : SkPathPriv::kUnknown_FirstDirection;
    SkAutoDisableDirectionCheck addc(this);
    SkAutoPathBoundsUpdate apbu(this, rect);
#endif

    SkDEBUGCODE(int initialVerbCount = this->countVerbs());

    const int kVerbs = 5; // moveTo + 3x lineTo + close
    this->incReserve(kVerbs);

    RectPointIterator iter(rect, dir, index);

    this->moveTo(iter.current());
    this->lineTo(iter.next());
    this->lineTo(iter.next());
    this->lineTo(iter.next());
    this->close();

    SkASSERT(this->countVerbs() == initialVerbCount + kVerbs);
    return *this;
}

SkPathBuilder& SkPathBuilder::addOval(const SkRect&, SkPathDirection dir) {
    return *this;
}
SkPathBuilder& SkPathBuilder::addRRect(const SkRRect&, SkPathDirection dir) {
    return *this;
}


