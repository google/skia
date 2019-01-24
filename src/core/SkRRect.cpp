/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRRectPriv.h"
#include "SkBuffer.h"
#include "SkMalloc.h"
#include "SkMatrix.h"
#include "SkScaleToSides.h"

#include <cmath>
#include <utility>

static bool is_span_legal(SkScalar a, SkScalar b, SkScalar min, SkScalar max) {
    return min <= a && a <= b && b <= max;
}

static bool are_pts_legal(const SkRect& r, const SkPoint pts[4]) {
    return  is_span_legal(pts[0].fX, pts[1].fX, r.fLeft, r.fRight) &&
            is_span_legal(pts[3].fX, pts[2].fX, r.fLeft, r.fRight) &&
            is_span_legal(pts[0].fY, pts[3].fY, r.fTop, r.fBottom) &&
            is_span_legal(pts[1].fY, pts[2].fY, r.fTop, r.fBottom);
}

static void init_rect_pts(const SkRect& r, SkPoint pts[4]) {
    pts[0].set(r.fLeft,  r.fTop);
    pts[1].set(r.fRight, r.fTop);
    pts[2].set(r.fRight, r.fBottom);
    pts[3].set(r.fLeft,  r.fBottom);
}

// we want min <= a <= b <= max using proportional scaling if need be
static void ensure_ordered_proportionally(SkScalar& a, SkScalar& b, SkScalar min, SkScalar max) {
    SkASSERT(min < max);
    SkASSERT(min <= a && a <= max);
    SkASSERT(min <= b && b <= max);
    if (a > b) {
        SkScalar span = max - min;
        SkScalar da = a - min;
        SkScalar db = max - b;
        SkScalar scale = span / (da + db);
        SkScalar middle = min + da * scale; // logically the same as max - db * scale
        if (SkScalarIsFinite(middle)) {
            a = b = SkScalarPin(middle, min, max);
        } else {
            // crazy math, set a, b, to their limits
            a = min;
            b = max;
        }
    }
}

static void legalize_rrect_pts(const SkRect& r, SkPoint pts[4]) {
    // modify pts to ensure they are contained inside the rect
    for (int i = 0; i < 4; ++i) {
        pts[i].fX = SkScalarPin(pts[i].fX, r.fLeft, r.fRight);
        pts[i].fY = SkScalarPin(pts[i].fY, r.fTop, r.fBottom);
    }

    // now modify pts to ensure they don't cross (left/right or top/bottom)
    ensure_ordered_proportionally(pts[0].fX, pts[1].fX, r.fLeft, r.fRight);
    ensure_ordered_proportionally(pts[3].fX, pts[2].fX, r.fLeft, r.fRight);
    ensure_ordered_proportionally(pts[0].fY, pts[3].fY, r.fTop, r.fBottom);
    ensure_ordered_proportionally(pts[1].fY, pts[2].fY, r.fTop, r.fBottom);
}

static void compute_4_radii(const SkRect& r, const SkPoint pts[4], SkVector radii[4]) {
    radii[0].set(pts[0].fX - r.fLeft,  pts[0].fY - r.fTop);
    radii[1].set(r.fRight - pts[1].fX, pts[1].fY - r.fTop);
    radii[2].set(r.fRight - pts[2].fX, r.fBottom - pts[2].fY);
    radii[3].set(pts[3].fX - r.fLeft,  r.fBottom - pts[3].fY);
}

void SkRRect::legalize() {
    // Check this before sorting because sorting can hide nans.
    if (!fRect.isFinite()) {
        this->setEmpty();
        return;
    }
    fRect = fRect.makeSorted();
    if (fRect.isEmpty()) {
        this->setEmpty();
        return;
    }

    if (!SkScalarsAreFinite(&fPts[0].fX, 8)) {
        this->setRect(fRect);
        return;
    }

    legalize_rrect_pts(fRect, fPts);

    Type type;
    SkAssertResult(ComputeType(fRect, fPts, &type));
    fType = static_cast<int32_t>(type);

    SkASSERT(this->isValid());
}

///////////////////////////////////////////////////////////////////////////////

void SkRRect::setRect(const SkRect& rect) {
    fRect = rect;
    init_rect_pts(fRect, fPts);

    this->legalize();
}

void SkRRect::setOval(const SkRect& oval) {
    fRect = oval;
    const SkPoint center = {oval.centerX(), oval.centerY()};
    for (auto& pt : fPts) {
        pt = center;
    }

    this->legalize();
}

void SkRRect::setRectXY(const SkRect& rect, SkScalar rx, SkScalar ry) {
    fRect = rect;
    fPts[0].set(fRect.fLeft  + rx, fRect.fTop    + ry);
    fPts[1].set(fRect.fRight - rx, fRect.fTop    + ry);
    fPts[2].set(fRect.fRight - rx, fRect.fBottom - ry);
    fPts[3].set(fRect.fLeft  + rx, fRect.fBottom - ry);

    this->legalize();
}

void SkRRect::setNinePatch(const SkRect& rect, SkScalar leftRad, SkScalar topRad,
                           SkScalar rightRad, SkScalar bottomRad) {
    fRect = rect;
    fPts[0].set(fRect.fLeft  + leftRad,  fRect.fTop    + topRad);
    fPts[1].set(fRect.fRight - rightRad, fRect.fTop    + topRad);
    fPts[2].set(fRect.fRight - rightRad, fRect.fBottom - bottomRad);
    fPts[3].set(fRect.fLeft  + leftRad,  fRect.fBottom - bottomRad);

    this->legalize();
}

void SkRRect::setRectRadii(const SkRect& rect, const SkVector radii[4]) {
    fRect = rect;
    fPts[0].set(fRect.fLeft  + radii[0].fX, fRect.fTop    + radii[0].fY);
    fPts[1].set(fRect.fRight - radii[1].fX, fRect.fTop    + radii[1].fY);
    fPts[2].set(fRect.fRight - radii[2].fX, fRect.fBottom - radii[2].fY);
    fPts[3].set(fRect.fLeft  + radii[3].fX, fRect.fBottom - radii[3].fY);

    this->legalize();
}

SkVector SkRRect::radii(Corner corner) const {
    const SkRect& r = fRect;
    SkVector rad = {0, 0};
    switch (corner) {
        case kUpperLeft_Corner:  rad = {fPts[0].fX - r.fLeft,  fPts[0].fY - r.fTop}; break;
        case kUpperRight_Corner: rad = {r.fRight - fPts[1].fX, fPts[1].fY - r.fTop}; break;
        case kLowerRight_Corner: rad = {r.fRight - fPts[2].fX, r.fBottom - fPts[2].fY}; break;
        case kLowerLeft_Corner:  rad = {fPts[3].fX - r.fLeft,  r.fBottom - fPts[3].fY}; break;
    }
    SkASSERT(rad.fX >= 0 && rad.fY >= 0);
    return rad;
}

// This method determines if a point known to be inside the RRect's bounds is
// inside all the corners.
bool SkRRect::checkCornerContainment(SkScalar x, SkScalar y) const {
    Corner index = kUpperLeft_Corner;

    // If we're an oval, any corner will do. If not, we need to check x,y
    if (kOval_Type != this->type()) {
        if (x < fPts[kUpperLeft_Corner].fX && y < fPts[kUpperLeft_Corner].fY) {
            index = kUpperLeft_Corner;
        } else if (x < fPts[kLowerLeft_Corner].fX && y > fPts[kLowerLeft_Corner].fY) {
            index = kLowerLeft_Corner;
        } else if (x > fPts[kUpperRight_Corner].fX && y < fPts[kUpperRight_Corner].fY) {
            index = kUpperRight_Corner;
        } else if (x > fPts[kLowerRight_Corner].fX && y > fPts[kLowerRight_Corner].fY) {
            index = kLowerRight_Corner;
        } else {
            // not in any of the corners
            return true;
        }
    }

    // A point is in an ellipse (in standard position) if:
    //      x^2     y^2
    //     ----- + ----- <= 1
    //      a^2     b^2
    // or :
    //     b^2*x^2 + a^2*y^2 <= (ab)^2
    SkVector radius = this->radii(index);
    SkScalar dist =  SkScalarSquare(fPts[index].fX) * SkScalarSquare(radius.fY) +
                     SkScalarSquare(fPts[index].fY) * SkScalarSquare(radius.fX);
    return dist <= SkScalarSquare(radius.fX * radius.fY);
}

bool SkRRectPriv::AllCornersCircular(const SkRRect& rr, SkScalar tolerance) {
    for (int i = 0; i < 4; ++i) {
        SkVector rad = rr.radii((SkRRect::Corner)i);
        if (!SkScalarNearlyEqual(rad.fX, rad.fY, tolerance)) {
            return false;
        }
    }
    return true;
}

bool SkRRect::contains(const SkRect& rect) const {
    if (!this->getBounds().contains(rect)) {
        // If 'rect' isn't contained by the RR's bounds then the
        // RR definitely doesn't contain it
        return false;
    }

    if (this->isRect()) {
        // the prior test was sufficient
        return true;
    }

    // At this point we know all four corners of 'rect' are inside the
    // bounds of of this RR. Check to make sure all the corners are inside
    // all the curves
    return this->checkCornerContainment(rect.fLeft, rect.fTop) &&
           this->checkCornerContainment(rect.fRight, rect.fTop) &&
           this->checkCornerContainment(rect.fRight, rect.fBottom) &&
           this->checkCornerContainment(rect.fLeft, rect.fBottom);
}

static bool radii_are_nine_patch(const SkVector radii[4]) {
    return radii[SkRRect::kUpperLeft_Corner].fX == radii[SkRRect::kLowerLeft_Corner].fX &&
           radii[SkRRect::kUpperLeft_Corner].fY == radii[SkRRect::kUpperRight_Corner].fY &&
           radii[SkRRect::kUpperRight_Corner].fX == radii[SkRRect::kLowerRight_Corner].fX &&
           radii[SkRRect::kLowerLeft_Corner].fY == radii[SkRRect::kLowerRight_Corner].fY;
}

#define RETURN_VALID_TYPE(theType)  \
    do { if (type) *type = (theType); return true; } while (0)

bool SkRRect::ComputeType(const SkRect& rect, const SkPoint pts[4], Type* type) {
    SkASSERT(rect.isSorted());

    if (rect.isEmpty()) {
        const SkPoint corner = {rect.fLeft, rect.fTop};
        for (int i = 0; i < 4; ++i) {
            if (corner != pts[i]) {
                return false;
            }
        }
        RETURN_VALID_TYPE(kEmpty_Type);
    }

    SkVector radii[4];
    compute_4_radii(rect, pts, radii);

    bool allRadiiEqual = true; // are all x radii equal and all y radii?
    bool allCornersSquare = 0 == radii[0].fX || 0 == radii[0].fY;
    for (int i = 1; i < 4; ++i) {
        if (0 != radii[i].fX && 0 != radii[i].fY) {
            // if either radius is zero the corner is square so both have to
            // be non-zero to have a rounded corner
            allCornersSquare = false;
        }
        if (radii[i].fX != radii[i-1].fX || radii[i].fY != radii[i-1].fY) {
            allRadiiEqual = false;
        }
    }

    if (allCornersSquare) {
        RETURN_VALID_TYPE(kRect_Type);
    }

    if (allRadiiEqual) {
        if (radii[0].fX >= SkScalarHalf(rect.width()) &&
            radii[0].fY >= SkScalarHalf(rect.height())) {
            RETURN_VALID_TYPE(kOval_Type);
        } else {
            RETURN_VALID_TYPE(kSimple_Type);
        }
    }

    if (radii_are_nine_patch(radii)) {
        RETURN_VALID_TYPE(kNinePatch_Type);
    } else {
        RETURN_VALID_TYPE(kComplex_Type);
    }
}

bool SkRRect::transform(const SkMatrix& matrix, SkRRect* dst) const {
    if (nullptr == dst) {
        return false;
    }

    // Assert that the caller is not trying to do this in place, which
    // would violate const-ness. Do not return false though, so that
    // if they know what they're doing and want to violate it they can.
    SkASSERT(dst != this);

    if (matrix.isIdentity()) {
        *dst = *this;
        return true;
    }

    // If transform supported 90 degree rotations (which it could), we could
    // use SkMatrix::rectStaysRect() to check for a valid transformation.
    if (!matrix.isScaleTranslate()) {
        return false;
    }

    SkRect rect;
    if (!matrix.mapRect(&rect, fRect)) {
        return false;
    }
    dst->fRect = rect;
    matrix.mapPoints(dst->fPts, fPts, 4);
    dst->legalize();
    return true;
}

SkRRect SkRRect::makeOffset(SkScalar dx, SkScalar dy) const {
    SkRRect rr;
    this->transform(SkMatrix::MakeTrans(dx, dy), &rr);
    return rr;
}

///////////////////////////////////////////////////////////////////////////////

void SkRRect::inset(SkScalar dx, SkScalar dy, SkRRect* dst) const {
    SkRect r = fRect.makeInset(dx, dy);
    bool degenerate = false;
    if (r.fRight <= r.fLeft) {
        degenerate = true;
        r.fLeft = r.fRight = SkScalarAve(r.fLeft, r.fRight);
    }
    if (r.fBottom <= r.fTop) {
        degenerate = true;
        r.fTop = r.fBottom = SkScalarAve(r.fTop, r.fBottom);
    }
    if (degenerate) {
        dst->fRect = r;
        init_rect_pts(r, dst->fPts);
        dst->fType = kEmpty_Type;
        return;
    }
    if (!r.isFinite()) {
        *dst = SkRRect();
        return;
    }

    dst->fPts[0].fX += dx; dst->fPts[0].fY += dy;
    dst->fPts[1].fX -= dx; dst->fPts[1].fY += dy;
    dst->fPts[2].fX -= dx; dst->fPts[2].fY -= dy;
    dst->fPts[3].fX += dx; dst->fPts[3].fY -= dy;
    dst->legalize();
}

///////////////////////////////////////////////////////////////////////////////

size_t SkRRect::writeToMemory(void* buffer) const {
    // Serialize only the rect and corners, but not the derived type tag.
    memcpy(buffer, this, kSizeInMemory);
    return kSizeInMemory;
}

void SkRRectPriv::WriteToBuffer(const SkRRect& rr, SkWBuffer* buffer) {
    // Serialize only the rect and corners, but not the derived type tag.
    buffer->write(&rr, SkRRect::kSizeInMemory);
}

size_t SkRRect::readFromMemory(const void* buffer, size_t length) {
    if (length < kSizeInMemory) {
        return 0;
    }

    SkRRect raw;
    memcpy(&raw, buffer, kSizeInMemory);
    this->setRectPoints(raw.fRect, raw.fPts);
    return kSizeInMemory;
}

bool SkRRectPriv::ReadFromBuffer(SkRBuffer* buffer, SkRRect* rr) {
    if (buffer->available() < SkRRect::kSizeInMemory) {
        return false;
    }
    SkRRect storage;
    return buffer->read(&storage, SkRRect::kSizeInMemory) &&
           (rr->readFromMemory(&storage, SkRRect::kSizeInMemory) == SkRRect::kSizeInMemory);
}

#include "SkString.h"
#include "SkStringUtils.h"

void SkRRect::dump(bool asHex) const {
    SkScalarAsStringType asType = asHex ? kHex_SkScalarAsStringType : kDec_SkScalarAsStringType;

    fRect.dump(asHex);
    SkString line("const SkPoint corners[] = {\n");
    for (int i = 0; i < 4; ++i) {
        SkString strX, strY;
        SkAppendScalar(&strX, fPts[i].x(), asType);
        SkAppendScalar(&strY, fPts[i].y(), asType);
        line.appendf("    { %s, %s },", strX.c_str(), strY.c_str());
        if (asHex) {
            line.appendf(" /* %f %f */", fPts[i].x(), fPts[i].y());
        }
        line.append("\n");
    }
    line.append("};");
    SkDebugf("%s\n", line.c_str());
}

bool SkRRect::isValid() const {
    if (!fRect.isFinite() || !SkScalarsAreFinite(&fPts[0].fX, 8)) {
        return false;
    }
    if (!fRect.isSorted()) {
        return false;
    }
    if (!are_pts_legal(fRect, fPts)) {
        return false;
    }

    Type type;
    if (!ComputeType(fRect, fPts, &type) || (int)type != fType) {
        return false;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

SkVector* SkRRectPriv::GetRadiiArray(const SkRRect& rr, SkVector radii[4]) {
    compute_4_radii(rr.fRect, rr.fPts, radii);
    return radii;
}
