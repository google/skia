/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCullPoints.h"

static bool cross_product_is_neg(const SkIPoint& v, int dx, int dy) {
#if 0
    return v.fX * dy - v.fY * dx < 0;
#else
    return sk_64_mul(v.fX, dy) < sk_64_mul(dx, v.fY);
#endif
}

bool SkCullPoints::sect_test(int x0, int y0, int x1, int y1) const {
    const SkIRect& r = fR;

    if ((x0 < r.fLeft    && x1 < r.fLeft) ||
        (x0 > r.fRight   && x1 > r.fRight) ||
        (y0 < r.fTop     && y1 < r.fTop) ||
        (y0 > r.fBottom  && y1 > r.fBottom)) {
        return false;
    }

    // since the crossprod test is a little expensive, check for easy-in cases first
    if (r.contains(x0, y0) || r.contains(x1, y1)) {
        return true;
    }

    // At this point we're not sure, so we do a crossprod test
    SkIPoint           vec;
    const SkIPoint*    rAsQuad = fAsQuad;

    vec.set(x1 - x0, y1 - y0);
    bool isNeg = cross_product_is_neg(vec, x0 - rAsQuad[0].fX, y0 - rAsQuad[0].fY);
    for (int i = 1; i < 4; i++) {
        if (cross_product_is_neg(vec, x0 - rAsQuad[i].fX, y0 - rAsQuad[i].fY) != isNeg) {
            return true;
        }
    }
    return false;   // we didn't intersect
}

static void toQuad(const SkIRect& r, SkIPoint quad[4]) {
    SkASSERT(quad);

    quad[0].set(r.fLeft, r.fTop);
    quad[1].set(r.fRight, r.fTop);
    quad[2].set(r.fRight, r.fBottom);
    quad[3].set(r.fLeft, r.fBottom);
}

SkCullPoints::SkCullPoints() {
    SkIRect    r;
    r.setEmpty();
    this->reset(r);
}

SkCullPoints::SkCullPoints(const SkIRect& r) {
    this->reset(r);
}

void SkCullPoints::reset(const SkIRect& r) {
    fR = r;
    toQuad(fR, fAsQuad);
    fPrevPt.set(0, 0);
    fPrevResult = kNo_Result;
}

void SkCullPoints::moveTo(int x, int y) {
    fPrevPt.set(x, y);
    fPrevResult = kNo_Result;   // so we trigger a movetolineto later
}

SkCullPoints::LineToResult SkCullPoints::lineTo(int x, int y, SkIPoint line[]) {
    SkASSERT(line != nullptr);

    LineToResult result = kNo_Result;
    int x0 = fPrevPt.fX;
    int y0 = fPrevPt.fY;

    // need to upgrade sect_test to chop the result
    // and to correctly return kLineTo_Result when the result is connected
    // to the previous call-out
    if (this->sect_test(x0, y0, x, y)) {
        line[0].set(x0, y0);
        line[1].set(x, y);

        if (fPrevResult != kNo_Result && fPrevPt.equals(x0, y0)) {
            result = kLineTo_Result;
        } else {
            result = kMoveToLineTo_Result;
        }
    }

    fPrevPt.set(x, y);
    fPrevResult = result;

    return result;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "SkPath.h"

SkCullPointsPath::SkCullPointsPath()
    : fCP(), fPath(nullptr) {
}

SkCullPointsPath::SkCullPointsPath(const SkIRect& r, SkPath* dst)
    : fCP(r), fPath(dst) {
}

void SkCullPointsPath::reset(const SkIRect& r, SkPath* dst) {
    fCP.reset(r);
    fPath = dst;
}

void SkCullPointsPath::moveTo(int x, int y) {
    fCP.moveTo(x, y);
}

void SkCullPointsPath::lineTo(int x, int y) {
    SkIPoint   pts[2];

    switch (fCP.lineTo(x, y, pts)) {
    case SkCullPoints::kMoveToLineTo_Result:
        fPath->moveTo(SkIntToScalar(pts[0].fX), SkIntToScalar(pts[0].fY));
        // fall through to the lineto case
    case SkCullPoints::kLineTo_Result:
        fPath->lineTo(SkIntToScalar(pts[1].fX), SkIntToScalar(pts[1].fY));
        break;
    default:
        break;
    }
}

///////////////////////////////////////////////////////////////////////////////

#include "SkMatrix.h"
#include "SkRegion.h"

bool SkHitTestPath(const SkPath& path, SkRect& target, bool hires) {
    if (target.isEmpty()) {
        return false;
    }

    bool isInverse = path.isInverseFillType();
    if (path.isEmpty()) {
        return isInverse;
    }

    SkRect bounds = path.getBounds();

    bool sects = SkRect::Intersects(target, bounds);
    if (isInverse) {
        if (!sects) {
            return true;
        }
    } else {
        if (!sects) {
            return false;
        }
        if (target.contains(bounds)) {
            return true;
        }
    }

    SkPath devPath;
    const SkPath* pathPtr;
    SkRect        devTarget;

    if (hires) {
        const SkScalar coordLimit = SkIntToScalar(16384);
        const SkRect limit = { 0, 0, coordLimit, coordLimit };

        SkMatrix matrix;
        matrix.setRectToRect(bounds, limit, SkMatrix::kFill_ScaleToFit);

        path.transform(matrix, &devPath);
        matrix.mapRect(&devTarget, target);

        pathPtr = &devPath;
    } else {
        devTarget = target;
        pathPtr = &path;
    }

    SkIRect iTarget;
    devTarget.round(&iTarget);
    if (iTarget.isEmpty()) {
        iTarget.fLeft = SkScalarFloorToInt(devTarget.fLeft);
        iTarget.fTop = SkScalarFloorToInt(devTarget.fTop);
        iTarget.fRight = iTarget.fLeft + 1;
        iTarget.fBottom = iTarget.fTop + 1;
    }

    SkRegion clip(iTarget);
    SkRegion rgn;
    return rgn.setPath(*pathPtr, clip) ^ isInverse;
}

bool SkHitTestPath(const SkPath& path, SkScalar x, SkScalar y, bool hires) {
    const SkScalar half = SK_ScalarHalf;
    const SkScalar one = SK_Scalar1;
    SkRect r = SkRect::MakeXYWH(x - half, y - half, one, one);
    return SkHitTestPath(path, r, hires);
}
