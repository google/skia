/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRRect.h"
#include "SkMatrix.h"

///////////////////////////////////////////////////////////////////////////////

void SkRRect::setRectXY(const SkRect& rect, SkScalar xRad, SkScalar yRad) {
    if (rect.isEmpty()) {
        this->setEmpty();
        return;
    }

    if (xRad <= 0 || yRad <= 0) {
        // all corners are square in this case
        this->setRect(rect);
        return;
    }

    if (rect.width() < xRad+xRad || rect.height() < yRad+yRad) {
        SkScalar scale = SkMinScalar(SkScalarDiv(rect.width(), xRad + xRad),
                                     SkScalarDiv(rect.height(), yRad + yRad));
        SkASSERT(scale < SK_Scalar1);
        xRad = SkScalarMul(xRad, scale);
        yRad = SkScalarMul(yRad, scale);
    }

    fRect = rect;
    for (int i = 0; i < 4; ++i) {
        fRadii[i].set(xRad, yRad);
    }
    fType = kSimple_Type;
    if (xRad >= SkScalarHalf(fRect.width()) && yRad >= SkScalarHalf(fRect.height())) {
        fType = kOval_Type;
        // TODO: assert that all the x&y radii are already W/2 & H/2
    }

    SkDEBUGCODE(this->validate();)
}

void SkRRect::setNinePatch(const SkRect& rect, SkScalar leftRad, SkScalar topRad,
                           SkScalar rightRad, SkScalar bottomRad) {
    if (rect.isEmpty()) {
        this->setEmpty();
        return;
    }

    leftRad = SkMaxScalar(leftRad, 0);
    topRad = SkMaxScalar(topRad, 0);
    rightRad = SkMaxScalar(rightRad, 0);
    bottomRad = SkMaxScalar(bottomRad, 0);

    SkScalar scale = SK_Scalar1;
    if (leftRad + rightRad > rect.width()) {
        scale = SkScalarDiv(rect.width(), leftRad + rightRad);
    }
    if (topRad + bottomRad > rect.height()) {
        scale = SkMinScalar(scale, SkScalarDiv(rect.width(), leftRad + rightRad));
    }

    if (scale < SK_Scalar1) {
        leftRad = SkScalarMul(leftRad, scale);
        topRad = SkScalarMul(topRad, scale);
        rightRad = SkScalarMul(rightRad, scale);
        bottomRad = SkScalarMul(bottomRad, scale);
    }

    if (leftRad == rightRad && topRad == bottomRad) {
        if (leftRad >= SkScalarHalf(rect.width()) && topRad >= SkScalarHalf(rect.height())) {
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

    fRect = rect;
    fRadii[kUpperLeft_Corner].set(leftRad, topRad);
    fRadii[kUpperRight_Corner].set(rightRad, topRad);
    fRadii[kLowerRight_Corner].set(rightRad, bottomRad);
    fRadii[kLowerLeft_Corner].set(leftRad, bottomRad);

    SkDEBUGCODE(this->validate();)
}


void SkRRect::setRectRadii(const SkRect& rect, const SkVector radii[4]) {
    if (rect.isEmpty()) {
        this->setEmpty();
        return;
    }

    fRect = rect;
    memcpy(fRadii, radii, sizeof(fRadii));

    bool allCornersSquare = true;

    // Clamp negative radii to zero
    for (int i = 0; i < 4; ++i) {
        if (fRadii[i].fX <= 0 || fRadii[i].fY <= 0) {
            // In this case we are being a little fast & loose. Since one of
            // the radii is 0 the corner is square. However, the other radii
            // could still be non-zero and play in the global scale factor
            // computation.
            fRadii[i].fX = 0;
            fRadii[i].fY = 0;
        } else {
            allCornersSquare = false;
        }
    }

    if (allCornersSquare) {
        this->setRect(rect);
        return;
    }

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
    SkScalar scale = SK_Scalar1;

    if (fRadii[0].fX + fRadii[1].fX > rect.width()) {
        scale = SkMinScalar(scale,
                            SkScalarDiv(rect.width(), fRadii[0].fX + fRadii[1].fX));
    }
    if (fRadii[1].fY + fRadii[2].fY > rect.height()) {
        scale = SkMinScalar(scale,
                            SkScalarDiv(rect.height(), fRadii[1].fY + fRadii[2].fY));
    }
    if (fRadii[2].fX + fRadii[3].fX > rect.width()) {
        scale = SkMinScalar(scale,
                            SkScalarDiv(rect.width(), fRadii[2].fX + fRadii[3].fX));
    }
    if (fRadii[3].fY + fRadii[0].fY > rect.height()) {
        scale = SkMinScalar(scale,
                            SkScalarDiv(rect.height(), fRadii[3].fY + fRadii[0].fY));
    }

    if (scale < SK_Scalar1) {
        for (int i = 0; i < 4; ++i) {
            fRadii[i].fX = SkScalarMul(fRadii[i].fX, scale);
            fRadii[i].fY = SkScalarMul(fRadii[i].fY, scale);
        }
    }

    // At this point we're either oval, simple, or complex (not empty or rect)
    // but we lazily resolve the type to avoid the work if the information
    // isn't required.
    fType = (SkRRect::Type) kUnknown_Type;

    SkDEBUGCODE(this->validate();)
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
        if (x < fRect.fLeft + fRadii[kUpperLeft_Corner].fX &&
            y < fRect.fTop + fRadii[kUpperLeft_Corner].fY) {
            // UL corner
            index = kUpperLeft_Corner;
            canonicalPt.set(x - (fRect.fLeft + fRadii[kUpperLeft_Corner].fX),
                            y - (fRect.fTop + fRadii[kUpperLeft_Corner].fY));
            SkASSERT(canonicalPt.fX < 0 && canonicalPt.fY < 0);
        } else if (x < fRect.fLeft + fRadii[kLowerLeft_Corner].fX &&
                   y > fRect.fBottom - fRadii[kLowerLeft_Corner].fY) {
            // LL corner
            index = kLowerLeft_Corner;
            canonicalPt.set(x - (fRect.fLeft + fRadii[kLowerLeft_Corner].fX),
                            y - (fRect.fBottom - fRadii[kLowerLeft_Corner].fY));
            SkASSERT(canonicalPt.fX < 0 && canonicalPt.fY > 0);
        } else if (x > fRect.fRight - fRadii[kUpperRight_Corner].fX &&
                   y < fRect.fTop + fRadii[kUpperRight_Corner].fY) {
            // UR corner
            index = kUpperRight_Corner;
            canonicalPt.set(x - (fRect.fRight - fRadii[kUpperRight_Corner].fX),
                            y - (fRect.fTop + fRadii[kUpperRight_Corner].fY));
            SkASSERT(canonicalPt.fX > 0 && canonicalPt.fY < 0);
        } else if (x > fRect.fRight - fRadii[kLowerRight_Corner].fX &&
                   y > fRect.fBottom - fRadii[kLowerRight_Corner].fY) {
            // LR corner
            index = kLowerRight_Corner;
            canonicalPt.set(x - (fRect.fRight - fRadii[kLowerRight_Corner].fX),
                            y - (fRect.fBottom - fRadii[kLowerRight_Corner].fY));
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
    SkScalar dist =  SkScalarMul(SkScalarSquare(canonicalPt.fX), SkScalarSquare(fRadii[index].fY)) +
                     SkScalarMul(SkScalarSquare(canonicalPt.fY), SkScalarSquare(fRadii[index].fX));
    return dist <= SkScalarSquare(SkScalarMul(fRadii[index].fX, fRadii[index].fY));
}

bool SkRRect::allCornersCircular() const {
    return fRadii[0].fX == fRadii[0].fY &&
        fRadii[1].fX == fRadii[1].fY &&
        fRadii[2].fX == fRadii[2].fY &&
        fRadii[3].fX == fRadii[3].fY;
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
void SkRRect::computeType() const {
    SkDEBUGCODE(this->validate();)

    if (fRect.isEmpty()) {
        fType = kEmpty_Type;
        return;
    }

    bool allRadiiEqual = true; // are all x radii equal and all y radii?
    bool allCornersSquare = 0 == fRadii[0].fX || 0 == fRadii[0].fY;

    for (int i = 1; i < 4; ++i) {
        if (0 != fRadii[i].fX && 0 != fRadii[i].fY) {
            // if either radius is zero the corner is square so both have to
            // be non-zero to have a rounded corner
            allCornersSquare = false;
        }
        if (fRadii[i].fX != fRadii[i-1].fX || fRadii[i].fY != fRadii[i-1].fY) {
            allRadiiEqual = false;
        }
    }

    if (allCornersSquare) {
        fType = kRect_Type;
        return;
    }

    if (allRadiiEqual) {
        if (fRadii[0].fX >= SkScalarHalf(fRect.width()) &&
            fRadii[0].fY >= SkScalarHalf(fRect.height())) {
            fType = kOval_Type;
        } else {
            fType = kSimple_Type;
        }
        return;
    }

    if (radii_are_nine_patch(fRadii)) {
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
    if (NULL == dst) {
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

    // At this point, this is guaranteed to succeed, so we can modify dst.
    dst->fRect = newRect;

    // Since the only transforms that were allowed are scale and translate, the type
    // remains unchanged.
    dst->fType = fType;

    if (kOval_Type == fType) {
        for (int i = 0; i < 4; ++i) {
            dst->fRadii[i].fX = SkScalarHalf(newRect.width());
            dst->fRadii[i].fY = SkScalarHalf(newRect.height());
        }
        SkDEBUGCODE(dst->validate();)
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
        dst->fRadii[i].fX = SkScalarMul(fRadii[i].fX, xScale);
        dst->fRadii[i].fY = SkScalarMul(fRadii[i].fY, yScale);
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

    SkDEBUGCODE(dst->validate();)

    return true;
}

///////////////////////////////////////////////////////////////////////////////

void SkRRect::inset(SkScalar dx, SkScalar dy, SkRRect* dst) const {
    SkRect r = fRect;

    r.inset(dx, dy);
    if (r.isEmpty()) {
        dst->setEmpty();
        return;
    }

    SkVector radii[4];
    memcpy(radii, fRadii, sizeof(radii));
    for (int i = 0; i < 4; ++i) {
        if (radii[i].fX) {
            radii[i].fX -= dx;
        }
        if (radii[i].fY) {
            radii[i].fY -= dy;
        }
    }
    dst->setRectRadii(r, radii);
}

///////////////////////////////////////////////////////////////////////////////

size_t SkRRect::writeToMemory(void* buffer) const {
    SkASSERT(kSizeInMemory == sizeof(SkRect) + sizeof(fRadii));

    memcpy(buffer, &fRect, sizeof(SkRect));
    memcpy((char*)buffer + sizeof(SkRect), fRadii, sizeof(fRadii));
    return kSizeInMemory;
}

size_t SkRRect::readFromMemory(const void* buffer, size_t length) {
    if (length < kSizeInMemory) {
        return 0;
    }

    SkScalar storage[12];
    SkASSERT(sizeof(storage) == kSizeInMemory);

    // we make a local copy, to ensure alignment before we cast
    memcpy(storage, buffer, kSizeInMemory);

    this->setRectRadii(*(const SkRect*)&storage[0],
                       (const SkVector*)&storage[4]);
    return kSizeInMemory;
}

#ifdef SK_DEVELOPER
void SkRRect::dump() const {
    SkDebugf("Rect: ");
    fRect.dump();
    SkDebugf(" Corners: { TL: (%f, %f), TR: (%f, %f), BR: (%f, %f), BL: (%f, %f) }",
             fRadii[kUpperLeft_Corner].fX,  fRadii[kUpperLeft_Corner].fY,
             fRadii[kUpperRight_Corner].fX, fRadii[kUpperRight_Corner].fY,
             fRadii[kLowerRight_Corner].fX, fRadii[kLowerRight_Corner].fY,
             fRadii[kLowerLeft_Corner].fX,  fRadii[kLowerLeft_Corner].fY);
}
#endif

///////////////////////////////////////////////////////////////////////////////

#ifdef SK_DEBUG
void SkRRect::validate() const {
    bool allRadiiZero = (0 == fRadii[0].fX && 0 == fRadii[0].fY);
    bool allCornersSquare = (0 == fRadii[0].fX || 0 == fRadii[0].fY);
    bool allRadiiSame = true;

    for (int i = 1; i < 4; ++i) {
        if (0 != fRadii[i].fX || 0 != fRadii[i].fY) {
            allRadiiZero = false;
        }

        if (fRadii[i].fX != fRadii[i-1].fX || fRadii[i].fY != fRadii[i-1].fY) {
            allRadiiSame = false;
        }

        if (0 != fRadii[i].fX && 0 != fRadii[i].fY) {
            allCornersSquare = false;
        }
    }
    bool patchesOfNine = radii_are_nine_patch(fRadii);

    switch (fType) {
        case kEmpty_Type:
            SkASSERT(fRect.isEmpty());
            SkASSERT(allRadiiZero && allRadiiSame && allCornersSquare);
            break;
        case kRect_Type:
            SkASSERT(!fRect.isEmpty());
            SkASSERT(allRadiiZero && allRadiiSame && allCornersSquare);
            break;
        case kOval_Type:
            SkASSERT(!fRect.isEmpty());
            SkASSERT(!allRadiiZero && allRadiiSame && !allCornersSquare);

            for (int i = 0; i < 4; ++i) {
                SkASSERT(SkScalarNearlyEqual(fRadii[i].fX, SkScalarHalf(fRect.width())));
                SkASSERT(SkScalarNearlyEqual(fRadii[i].fY, SkScalarHalf(fRect.height())));
            }
            break;
        case kSimple_Type:
            SkASSERT(!fRect.isEmpty());
            SkASSERT(!allRadiiZero && allRadiiSame && !allCornersSquare);
            break;
        case kNinePatch_Type:
            SkASSERT(!fRect.isEmpty());
            SkASSERT(!allRadiiZero && !allRadiiSame && !allCornersSquare);
            SkASSERT(patchesOfNine);
            break;
        case kComplex_Type:
            SkASSERT(!fRect.isEmpty());
            SkASSERT(!allRadiiZero && !allRadiiSame && !allCornersSquare);
            SkASSERT(!patchesOfNine);
            break;
        case kUnknown_Type:
            // no limits on this
            break;
    }
}
#endif // SK_DEBUG

///////////////////////////////////////////////////////////////////////////////
