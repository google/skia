/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "SkQuadClipper.h"
#include "SkGeometry.h"

static bool chopMonoQuadAtY(SkPoint pts[3], SkScalar y, SkScalar* t) {
    /* Solve F(t) = y where F(t) := [0](1-t)^2 + 2[1]t(1-t) + [2]t^2
     *  We solve for t, using quadratic equation, hence we have to rearrange
     * our cooefficents to look like At^2 + Bt + C
     */
    SkScalar A = pts[0].fY - pts[1].fY - pts[1].fY + pts[2].fY;
    SkScalar B = 2*(pts[1].fY - pts[0].fY);
    SkScalar C = pts[0].fY - y;
    
    SkScalar roots[2];  // we only expect one, but make room for 2 for safety
    int count = SkFindUnitQuadRoots(A, B, C, roots);
    if (count) {
        *t = roots[0];
        return true;
    }
    return false;
}

SkQuadClipper::SkQuadClipper() {}

void SkQuadClipper::setClip(const SkIRect& clip) {
    // conver to scalars, since that's where we'll see the points
    fClip.set(clip);
}

/*  If we somehow returned the fact that we had to flip the pts in Y, we could
    communicate that to setQuadratic, and then avoid having to flip it back
    here (only to have setQuadratic do the flip again)
 */
bool SkQuadClipper::clipQuad(const SkPoint srcPts[3], SkPoint dst[3]) {
    bool reverse;

    // we need the data to be monotonically descending in Y
    if (srcPts[0].fY > srcPts[2].fY) {
        dst[0] = srcPts[2];
        dst[1] = srcPts[1];
        dst[2] = srcPts[0];
        reverse = true;
    } else {
        memcpy(dst, srcPts, 3 * sizeof(SkPoint));
        reverse = false;
    }

    // are we completely above or below
    const SkScalar ctop = fClip.fTop;
    const SkScalar cbot = fClip.fBottom;
    if (dst[2].fY <= ctop || dst[0].fY >= cbot) {
        return false;
    }
    
    SkScalar t;
    SkPoint tmp[5]; // for SkChopQuadAt
    
    // are we partially above
    if (dst[0].fY < ctop && chopMonoQuadAtY(dst, ctop, &t)) {
        SkChopQuadAt(dst, tmp, t);
        dst[0] = tmp[2];
        dst[1] = tmp[3];
    }
    
    // are we partially below
    if (dst[2].fY > cbot && chopMonoQuadAtY(dst, cbot, &t)) {
        SkChopQuadAt(dst, tmp, t);
        dst[1] = tmp[1];
        dst[2] = tmp[2];
    }
    
    if (reverse) {
        SkTSwap<SkPoint>(dst[0], dst[2]);
    }
    return true;
}

