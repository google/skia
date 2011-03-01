/* libs/graphics/effects/SkCullPoints.cpp
**
** Copyright 2006, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
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

