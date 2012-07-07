
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkCullPoints.h"
#include "Sk64.h"

static bool cross_product_is_neg(const SkIPoint& v, int dx, int dy) {
#if 0
    return v.fX * dy - v.fY * dx < 0;
#else
    Sk64   tmp0, tmp1;
    
    tmp0.setMul(v.fX, dy);
    tmp1.setMul(dx, v.fY);
    tmp0.sub(tmp1);
    return tmp0.isNeg() != 0;
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
    SkASSERT(line != NULL);

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
    : fCP(), fPath(NULL) {
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

///////////////////////////////////////////////////////////////////////////////

#include "SkGeometry.h"

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
static bool chopMonoCubicAt(SkScalar c0, SkScalar c1, SkScalar c2, SkScalar c3,
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
    return true;
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

    SkScalar t, xt;
    if (chopMonoCubicAt(pts[0].fY, pts[1].fY, pts[2].fY, pts[3].fY, y, &t)) {
        xt = eval_cubic_pts(pts[0].fX, pts[1].fX, pts[2].fX, pts[3].fX, t);
    } else {
        SkScalar mid = SkScalarAve(pts[0].fY, pts[3].fY);
        xt = y < mid ? pts[0].fX : pts[3].fX;
    }
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

    // bounds check on X (not required, but maybe faster)
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

bool SkHitTestPathEx(const SkPath& path, SkScalar x, SkScalar y) {
    bool isInverse = path.isInverseFillType();
    if (path.isEmpty()) {
        return isInverse;
    }
    
    const SkRect& bounds = path.getBounds();
    if (!bounds.contains(x, y)) {
        return isInverse;
    }

    SkPath::Iter iter(path, true);
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
            case SkPath::kCubic_Verb:
                w += winding_cubic(pts, x, y);
                break;
            case SkPath::kDone_Verb:
                done = true;
                break;
        }
    } while (!done);

    switch (path.getFillType()) {
        case SkPath::kEvenOdd_FillType:
        case SkPath::kInverseEvenOdd_FillType:
            w &= 1;
            break;
    }
    return SkToBool(w);
}

