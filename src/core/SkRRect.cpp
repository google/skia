/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <cmath>
#include "SkRRectPriv.h"
#include "SkScopeExit.h"
#include "SkBuffer.h"
#include "SkMalloc.h"
#include "SkMatrix.h"
#include "SkRectPriv.h"
#include "SkScaleToSides.h"

static void set_inner_to_rect(const SkRect& r, SkPoint inner[4]) {
    inner[SkRRect::kUpperLeft_Corner]  = { r.fLeft,  r.fTop };
    inner[SkRRect::kUpperRight_Corner] = { r.fRight, r.fTop };
    inner[SkRRect::kLowerLeft_Corner]  = { r.fLeft,  r.fBottom };
    inner[SkRRect::kLowerRight_Corner] = { r.fRight, r.fBottom };
}

static void fixup_inner(SkPoint dst[4], const SkPoint src[4], const SkRect& r) {
    dst[SkRRect::kUpperLeft_Corner].fX = std::max(src[SkRRect::kUpperLeft_Corner].fX, r.fLeft);
    dst[SkRRect::kUpperLeft_Corner].fY = std::max(src[SkRRect::kUpperLeft_Corner].fY, r.fTop);

    dst[SkRRect::kUpperRight_Corner].fX = std::min(src[SkRRect::kUpperRight_Corner].fX, r.fRight);
    dst[SkRRect::kUpperRight_Corner].fY = std::max(src[SkRRect::kUpperRight_Corner].fY, r.fTop);

    dst[SkRRect::kLowerRight_Corner].fX = std::min(src[SkRRect::kLowerRight_Corner].fX, r.fRight);
    dst[SkRRect::kLowerRight_Corner].fY = std::min(src[SkRRect::kLowerRight_Corner].fY, r.fBottom);

    dst[SkRRect::kUpperLeft_Corner].fX = std::max(src[SkRRect::kUpperLeft_Corner].fX, r.fLeft);
    dst[SkRRect::kUpperLeft_Corner].fY = std::min(src[SkRRect::kUpperLeft_Corner].fY, r.fBottom);
}

SkVector SkRRect::radii(Corner corner) const {
    SkVector v;
    switch (corner) {
        case kUpperLeft_Corner:
            v = {
                fInner[kUpperLeft_Corner].fX - fRect.fLeft,
                fInner[kUpperLeft_Corner].fY - fRect.fTop,
            };
            break;
        case kUpperRight_Corner:
            v = {
                fRect.fRight - fInner[kUpperRight_Corner].fX,
                fInner[kUpperRight_Corner].fY - fRect.fTop,
            };
            break;
        case kLowerLeft_Corner:
            v = {
                fInner[kLowerLeft_Corner].fX - fRect.fLeft,
                fRect.fBottom - fInner[kLowerLeft_Corner].fY,
            };
            break;
        case kLowerRight_Corner:
            v = {
                fRect.fRight - fInner[kLowerRight_Corner].fX,
                fRect.fBottom - fInner[kLowerRight_Corner].fY,
            };
            break;
        default:
            SkASSERT(false);
            v = {0, 0};
    }
    SkASSERT(v.fX >= 0 && v.fY >= 0);
    return v;
}

void SkRRect::getRadii(SkVector radii[4]) const {
    radii[kUpperLeft_Corner] = {
        fInner[kUpperLeft_Corner].fX - fRect.fLeft,
        fInner[kUpperLeft_Corner].fY - fRect.fTop,
    };
    radii[kUpperRight_Corner] = {
        fRect.fRight - fInner[kUpperRight_Corner].fX,
        fInner[kUpperRight_Corner].fY - fRect.fTop,
    };
    radii[kLowerLeft_Corner] = {
        fInner[kLowerLeft_Corner].fX - fRect.fLeft,
        fRect.fBottom - fInner[kLowerLeft_Corner].fY,
    };
    radii[kLowerRight_Corner] = {
        fRect.fRight - fInner[kLowerRight_Corner].fX,
        fRect.fBottom - fInner[kLowerRight_Corner].fY,
    };
}

void SkRRect::getCenters(SkPoint dst[4]) const {
    if (dst) {
        memcpy(dst, fInner, 4 * sizeof(SkPoint));
    }
}

void SkRRect::setRect(const SkRect& rect) {
    if (!this->initializeRect(rect)) {
        return;
    }

    set_inner_to_rect(fRect, fInner);
    fType = kRect_Type;

    SkASSERT(this->isValid());
}

void SkRRect::setRectXY(const SkRect& rect, SkScalar rx, SkScalar ry) {
    SkVector radii[] = {
        { rx, ry }, { rx, ry }, { rx, ry }, { rx, ry },
    };
    this->setRectRadii(rect, radii);
}

void SkRRect::setNinePatch(const SkRect& rect, SkScalar leftRad, SkScalar topRad,
                           SkScalar rightRad, SkScalar bottomRad) {
    if (!this->initializeRect(rect)) {
        return;
    }

    const SkScalar array[4] = { leftRad, topRad, rightRad, bottomRad };
    if (!SkScalarsAreFinite(array, 4)) {
        this->setRect(rect);    // devolve into a simple rect
        return;
    }

    leftRad = SkMaxScalar(leftRad, 0);
    topRad = SkMaxScalar(topRad, 0);
    rightRad = SkMaxScalar(rightRad, 0);
    bottomRad = SkMaxScalar(bottomRad, 0);

    SkScalar scale = SK_Scalar1;
    if (leftRad + rightRad > fRect.width()) {
        scale = fRect.width() / (leftRad + rightRad);
    }
    if (topRad + bottomRad > fRect.height()) {
        scale = SkMinScalar(scale, fRect.height() / (topRad + bottomRad));
    }

    if (scale < SK_Scalar1) {
        leftRad *= scale;
        topRad *= scale;
        rightRad *= scale;
        bottomRad *= scale;
    }

    if (leftRad == rightRad && topRad == bottomRad) {
        if (leftRad >= SkScalarHalf(fRect.width()) && topRad >= SkScalarHalf(fRect.height())) {
            fType = kOval_Type;
        } else if (0 == leftRad || 0 == topRad) {
            // If the left and (by equality check above) right radii are zero then it is a rect.
            // Same goes for top/bottom.
            fType = kRect_Type;
            leftRad = 0;
            topRad = 0;
            rightRad = 0;
            bottomRad = 0;
        } else {
            fType = kSimple_Type;
        }
    } else {
        fType = kNinePatch_Type;
    }

    const SkScalar L = fRect.fLeft;
    const SkScalar T = fRect.fTop;
    const SkScalar R = fRect.fRight;
    const SkScalar B = fRect.fBottom;
    fInner[kUpperLeft_Corner].set( L + leftRad,  T + topRad);
    fInner[kUpperRight_Corner].set(R - rightRad, T + topRad);
    fInner[kLowerRight_Corner].set(R - rightRad, B - bottomRad);
    fInner[kLowerLeft_Corner].set( L + leftRad,  B - bottomRad);

    SkASSERT(this->isValid());
}

// These parameters intentionally double. Apropos crbug.com/463920, if one of the
// radii is huge while the other is small, single precision math can completely
// miss the fact that a scale is required.
static double compute_min_scale(double rad1, double rad2, double limit, double curMin) {
    if ((rad1 + rad2) > limit) {
        return SkTMin(curMin, limit / (rad1 + rad2));
    }
    return curMin;
}

void SkRRect::setRectRadii(const SkRect& rect, const SkVector origRadii[4]) {
    if (!this->initializeRect(rect)) {
        return;
    }

    if (!SkScalarsAreFinite(&origRadii[0].fX, 8)) {
        this->setRect(rect);    // devolve into a simple rect
        return;
    }

    SkVector radii[4];
    memcpy(radii, origRadii, sizeof(radii));

    bool allCornersSquare = true;

    // Clamp negative radii to zero
    for (int i = 0; i < 4; ++i) {
        if (radii[i].fX <= 0 || radii[i].fY <= 0) {
            // In this case we are being a little fast & loose. Since one of
            // the radii is 0 the corner is square. However, the other radii
            // could still be non-zero and play in the global scale factor
            // computation.
            radii[i].set(0, 0);
        } else {
            allCornersSquare = false;
        }
    }

    if (allCornersSquare) {
        this->setRect(rect);
        return;
    }

    this->scaleRadii(radii);
}

bool SkRRect::initializeRect(const SkRect& rect) {
    // Check this before sorting because sorting can hide nans.
    if (!rect.isFinite()) {
        *this = SkRRect();
        return false;
    }
    fRect = rect.makeSorted();
    if (fRect.isEmpty()) {
        memset(fInner, 0, sizeof(fInner));
        fType = kEmpty_Type;
        return false;
    }
    return true;
}

void SkRRect::scaleRadii(const SkVector rad[4]) {

    // Proportionally scale down all radii to fit. Find the minimum ratio
    // of a side and the radii on that side (for all four sides) and use
    // that to scale down _all_ the radii. This algorithm is from the
    // W3 spec (http://www.w3.org/TR/css3-background/) section 5.5 - Overlapping
    // Curves:
    // "Let f = min(Li/Si), where i is one of { top, right, bottom, left },
    //   Si is the sum of the two corresponding radii of the corners on side i,
    //   and Ltop = Lbottom = the width of the box,
    //   and Lleft = Lright = the height of the box.
    // If f < 1, then all corner radii are reduced by multiplying them by f."
    double scale = 1.0;

    // The sides of the rectangle may be larger than a float.
    double width = (double)fRect.fRight - (double)fRect.fLeft;
    double height = (double)fRect.fBottom - (double)fRect.fTop;
    scale = compute_min_scale(rad[0].fX, rad[1].fX, width,  scale);
    scale = compute_min_scale(rad[1].fY, rad[2].fY, height, scale);
    scale = compute_min_scale(rad[2].fX, rad[3].fX, width,  scale);
    scale = compute_min_scale(rad[3].fY, rad[0].fY, height, scale);

    const SkScalar L = fRect.fLeft;
    const SkScalar T = fRect.fTop;
    const SkScalar R = fRect.fRight;
    const SkScalar B = fRect.fBottom;

    fInner[0].fX = std::min(L + SkDoubleToScalar(rad[0].fX * scale), R);
    fInner[3].fX = std::min(L + SkDoubleToScalar(rad[3].fX * scale), R);
    fInner[1].fX = std::max(R - SkDoubleToScalar(rad[1].fX * scale), fInner[0].fX);
    fInner[2].fX = std::max(R - SkDoubleToScalar(rad[2].fX * scale), fInner[3].fX);

    fInner[0].fY = std::min(T + SkDoubleToScalar(rad[0].fY * scale), B);
    fInner[1].fY = std::min(T + SkDoubleToScalar(rad[1].fY * scale), B);
    fInner[2].fY = std::max(B - SkDoubleToScalar(rad[2].fY * scale), fInner[1].fY);
    fInner[3].fY = std::max(B - SkDoubleToScalar(rad[3].fY * scale), fInner[0].fY);

    // At this point we're either oval, simple, or complex (not empty or rect).
    this->computeType();

    SkASSERT(this->isValid());
}

// This method determines if a point known to be inside the RRect's bounds is
// inside all the corners.
bool SkRRect::checkCornerContainment(SkScalar x, SkScalar y) const {
    SkPoint canonicalPt; // (x,y) translated to one of the quadrants
    int index;

    if (kOval_Type == this->type()) {
        canonicalPt.set(x - fRect.centerX(), y - fRect.centerY());
        index = kUpperLeft_Corner;  // any corner will do in this case
    } else {
        if (x < fInner[kUpperLeft_Corner].fX && y < fInner[kUpperLeft_Corner].fY) {
            // UL corner
            index = kUpperLeft_Corner;
            canonicalPt.set(x - (fInner[kUpperLeft_Corner].fX),
                            y - (fInner[kUpperLeft_Corner].fY));
            SkASSERT(canonicalPt.fX < 0 && canonicalPt.fY < 0);
        } else if (x < fInner[kLowerLeft_Corner].fX && y > fInner[kLowerLeft_Corner].fY) {
            // LL corner
            index = kLowerLeft_Corner;
            canonicalPt.set(x - (fInner[kLowerLeft_Corner].fX),
                            y - (fInner[kLowerLeft_Corner].fY));
            SkASSERT(canonicalPt.fX < 0 && canonicalPt.fY > 0);
        } else if (x > fInner[kUpperRight_Corner].fX && y < fInner[kUpperRight_Corner].fY) {
            // UR corner
            index = kUpperRight_Corner;
            canonicalPt.set(x - (fInner[kUpperRight_Corner].fX),
                            y - (fInner[kUpperRight_Corner].fY));
            SkASSERT(canonicalPt.fX > 0 && canonicalPt.fY < 0);
        } else if (x > fInner[kLowerRight_Corner].fX && y > fInner[kLowerRight_Corner].fY) {
            // LR corner
            index = kLowerRight_Corner;
            canonicalPt.set(x - (fInner[kLowerRight_Corner].fX),
                            y - (fInner[kLowerRight_Corner].fY));
            SkASSERT(canonicalPt.fX > 0 && canonicalPt.fY > 0);
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
    //
    SkVector rad = this->radii((Corner)index);
    SkScalar dist =  SkScalarSquare(canonicalPt.fX) * SkScalarSquare(rad.fY) +
                     SkScalarSquare(canonicalPt.fY) * SkScalarSquare(rad.fX);
    return dist <= SkScalarSquare(rad.fX * rad.fY);
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

// There is a simplified version of this method in setRectXY
void SkRRect::computeType() {
    if (fRect.isEmpty()) {
        SkASSERT(fRect.isSorted());
        for (size_t i = 0; i < SK_ARRAY_COUNT(fInner); ++i) {
            SkASSERT((fInner[i] == SkVector{0, 0}));
        }
        fType = kEmpty_Type;
        return;
    }

    SkVector radii[4];
    this->getRadii(radii);

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
        fType = kRect_Type;
        return;
    }

    if (allRadiiEqual) {
        if (fInner[0].fX == fRect.centerX() && fInner[0].fY == fRect.centerY()) {
            fType = kOval_Type;
        } else {
            fType = kSimple_Type;
        }
        return;
    }

    if (radii_are_nine_patch(radii)) {
        fType = kNinePatch_Type;
    } else {
        fType = kComplex_Type;
    }
}

static bool matrix_only_scale_and_translate(const SkMatrix& matrix) {
    const SkMatrix::TypeMask m = (SkMatrix::TypeMask) (SkMatrix::kAffine_Mask
                                    | SkMatrix::kPerspective_Mask);
    return (matrix.getType() & m) == 0;
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
    if (!matrix_only_scale_and_translate(matrix)) {
        return false;
    }

    SkRect newRect;
    if (!matrix.mapRect(&newRect, fRect)) {
        return false;
    }

    // The matrix may have scaled us to zero (or due to float madness, we now have collapsed
    // some dimension of the rect, so we need to check for that. Note that matrix must be
    // scale and translate and mapRect() produces a sorted rect. So an empty rect indicates
    // loss of precision.
    if (!newRect.isFinite() || newRect.isEmpty()) {
        return false;
    }

    // At this point, this is guaranteed to succeed, so we can modify dst.
    dst->fRect = newRect;
    matrix.mapPoints(dst->fInner, fInner, 4);
    dst->computeType();
    return true;
#if 0
    // Since the only transforms that were allowed are scale and translate, the type
    // remains unchanged.
    dst->fType = fType;

    if (kRect_Type == fType) {
        SkASSERT(dst->isValid());
        return true;
    }
    if (kOval_Type == fType) {
        for (int i = 0; i < 4; ++i) {
            dst->fRadii[i].fX = SkScalarHalf(newRect.width());
            dst->fRadii[i].fY = SkScalarHalf(newRect.height());
        }
        SkASSERT(dst->isValid());
        return true;
    }

    // Now scale each corner
    SkScalar xScale = matrix.getScaleX();
    const bool flipX = xScale < 0;
    if (flipX) {
        xScale = -xScale;
    }
    SkScalar yScale = matrix.getScaleY();
    const bool flipY = yScale < 0;
    if (flipY) {
        yScale = -yScale;
    }

    // Scale the radii without respecting the flip.
    for (int i = 0; i < 4; ++i) {
        dst->fRadii[i].fX = fRadii[i].fX * xScale;
        dst->fRadii[i].fY = fRadii[i].fY * yScale;
    }

    // Now swap as necessary.
    if (flipX) {
        if (flipY) {
            // Swap with opposite corners
            SkTSwap(dst->fRadii[kUpperLeft_Corner], dst->fRadii[kLowerRight_Corner]);
            SkTSwap(dst->fRadii[kUpperRight_Corner], dst->fRadii[kLowerLeft_Corner]);
        } else {
            // Only swap in x
            SkTSwap(dst->fRadii[kUpperRight_Corner], dst->fRadii[kUpperLeft_Corner]);
            SkTSwap(dst->fRadii[kLowerRight_Corner], dst->fRadii[kLowerLeft_Corner]);
        }
    } else if (flipY) {
        // Only swap in y
        SkTSwap(dst->fRadii[kUpperLeft_Corner], dst->fRadii[kLowerLeft_Corner]);
        SkTSwap(dst->fRadii[kUpperRight_Corner], dst->fRadii[kLowerRight_Corner]);
    }

    if (!AreRectAndRadiiValid(dst->fRect, dst->fRadii)) {
        return false;
    }

    dst->scaleRadii();
    dst->isValid();

    return true;
#endif
}

///////////////////////////////////////////////////////////////////////////////

void SkRRect::inset(SkScalar dx, SkScalar dy, SkRRect* dst) const {
    if (!SkScalarsAreFinite(dx, dy)) {
        dst->setEmpty();
        return;
    }
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
        set_inner_to_rect(r, dst->fInner);
        dst->fType = kEmpty_Type;
        return;
    }
    if (!r.isFinite()) {
        dst->setEmpty();
        return;
    }

    auto if_eq_update = [](SkScalar oldv, SkScalar oldlimit, SkScalar newlimit) {
        return oldv == oldlimit ? newlimit : oldv;
    };
#define IF_EQ_UPDATE(index, xEdge, yEdge) \
    dst->fInner[index].fX = if_eq_update(fInner[index].fX, fRect. xEdge, r. xEdge);   \
    dst->fInner[index].fY = if_eq_update(fInner[index].fY, fRect. yEdge, r. yEdge)

    // The inner points stay put ... unless they were at their respective edge, in which case
    // they should move with the edge.
    IF_EQ_UPDATE(kUpperLeft_Corner,  fLeft,  fTop);
    IF_EQ_UPDATE(kUpperRight_Corner, fRight, fTop);
    IF_EQ_UPDATE(kLowerRight_Corner, fRight, fBottom);
    IF_EQ_UPDATE(kLowerLeft_Corner,  fLeft,  fBottom);

    if (0) fixup_inner(dst->fInner, fInner, r);
    dst->fRect = r;
    dst->computeType();
}

SkRRect SkRRect::makeOffset(SkScalar dx, SkScalar dy) const {
    SkRRect dst;
    (void)this->transform(SkMatrix::MakeTrans(dx, dy), &dst);
    return dst;
}

///////////////////////////////////////////////////////////////////////////////

size_t SkRRect::writeToMemory(void* buffer) const {
    // Serialize only the rect and corners, but not the derived type tag.
    memcpy(buffer, this, kSizeInMemory);
    return kSizeInMemory;
}

void SkRRect::writeToBuffer(SkWBuffer* buffer) const {
    // Serialize only the rect and corners, but not the derived type tag.
    buffer->write(this, kSizeInMemory);
}

size_t SkRRect::readFromMemory_radii_version(const void* buffer, size_t length) {
    if (length < kSizeInMemory) {
        return 0;
    }

    SkRect bounds;
    memcpy(&bounds, buffer, sizeof(bounds));
    SkVector radii[4];
    memcpy(radii, (const char*)buffer + sizeof(bounds), sizeof(radii));

    this->setRectRadii(bounds, radii);
    return kSizeInMemory;
}

size_t SkRRect::readFromMemory(const void* buffer, size_t length) {
    if (length < kSizeInMemory) {
        return 0;
    }

    SkRect bounds;
    memcpy(&bounds, buffer, sizeof(bounds));
    SkPoint inner[4];
    memcpy(inner, (const char*)buffer + sizeof(bounds), sizeof(inner));

    if (!AreRectAndRadiiValid(bounds, inner)) {
        return 0;
    }

    fRect = bounds;
    memcpy(fInner, inner, sizeof(inner));
    this->computeType();
    return kSizeInMemory;
}

bool SkRRect::readFromBuffer(SkRBuffer* buffer) {
    if (buffer->available() < kSizeInMemory) {
        return false;
    }
    SkRRect storage;
    return buffer->read(&storage, kSizeInMemory) &&
           (this->readFromMemory(&storage, kSizeInMemory) == kSizeInMemory);
}

#include "SkString.h"
#include "SkStringUtils.h"

void SkRRect::dump(bool asHex) const {
    SkScalarAsStringType asType = asHex ? kHex_SkScalarAsStringType : kDec_SkScalarAsStringType;

    fRect.dump(asHex);
    SkString line("const SkPoint corners[] = {\n");
    for (int i = 0; i < 4; ++i) {
        SkString strX, strY;
        SkAppendScalar(&strX, fInner[i].x(), asType);
        SkAppendScalar(&strY, fInner[i].y(), asType);
        line.appendf("    { %s, %s },", strX.c_str(), strY.c_str());
        if (asHex) {
            line.appendf(" /* %f %f */", fInner[i].x(), fInner[i].y());
        }
        line.append("\n");
    }
    line.append("};");
    SkDebugf("%s\n", line.c_str());
}

///////////////////////////////////////////////////////////////////////////////

bool SkRRect::isValid() const {
    if (!AreRectAndRadiiValid(fRect, fInner)) {
        return false;
    }
    SkRRect tmp(*this);
    tmp.computeType();
    if (tmp.fType != fType) {
        return false;
    }
    return true;
}

bool SkRRect::AreRectAndRadiiValid(const SkRect& rect, const SkPoint inner[4]) {
    if (!rect.isFinite() || !rect.isSorted()) {
        return false;
    }
    if (!SkScalarsAreFinite(&inner[0].fX, 8)) {
        return false;
    }
    for (int i = 0; i < 4; ++i) {
        if (!SkRectPriv::InclusiveNoEmptyCheckContains(rect, inner[i].fX, inner[i].fY)) {
            return false;
        }
    }
    // check that the center points are monotonic in each direction
    if (inner[kUpperLeft_Corner].fX  > inner[kUpperRight_Corner].fX ||
        inner[kLowerLeft_Corner].fX  > inner[kLowerRight_Corner].fX ||
        inner[kUpperLeft_Corner].fY  > inner[kLowerLeft_Corner].fY  ||
        inner[kUpperRight_Corner].fY > inner[kLowerRight_Corner].fY) {
        return false;
    }
    return true;
}

