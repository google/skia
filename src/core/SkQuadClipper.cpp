
/*
 * Copyright 2009 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkQuadClipper.h"
#include "SkGeometry.h"

static inline void clamp_le(SkScalar& value, SkScalar max) {
    if (value > max) {
        value = max;
    }
}

static inline void clamp_ge(SkScalar& value, SkScalar min) {
    if (value < min) {
        value = min;
    }
}

SkQuadClipper::SkQuadClipper() {
    fClip.setEmpty();
}

void SkQuadClipper::setClip(const SkIRect& clip) {
    // conver to scalars, since that's where we'll see the points
    fClip.set(clip);
}

///////////////////////////////////////////////////////////////////////////////

static bool chopMonoQuadAt(SkScalar c0, SkScalar c1, SkScalar c2,
                           SkScalar target, SkScalar* t) {
    /* Solve F(t) = y where F(t) := [0](1-t)^2 + 2[1]t(1-t) + [2]t^2
     *  We solve for t, using quadratic equation, hence we have to rearrange
     * our cooefficents to look like At^2 + Bt + C
     */
    SkScalar A = c0 - c1 - c1 + c2;
    SkScalar B = 2*(c1 - c0);
    SkScalar C = c0 - target;

    SkScalar roots[2];  // we only expect one, but make room for 2 for safety
    int count = SkFindUnitQuadRoots(A, B, C, roots);
    if (count) {
        *t = roots[0];
        return true;
    }
    return false;
}

static bool chopMonoQuadAtY(SkPoint pts[3], SkScalar y, SkScalar* t) {
    return chopMonoQuadAt(pts[0].fY, pts[1].fY, pts[2].fY, y, t);
}

///////////////////////////////////////////////////////////////////////////////

/*  If we somehow returned the fact that we had to flip the pts in Y, we could
 communicate that to setQuadratic, and then avoid having to flip it back
 here (only to have setQuadratic do the flip again)
 */
bool SkQuadClipper::clipQuad(const SkPoint srcPts[3], SkPoint dst[3]) {
    bool reverse;

    // we need the data to be monotonically increasing in Y
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
    if (dst[0].fY < ctop) {
        if (chopMonoQuadAtY(dst, ctop, &t)) {
            // take the 2nd chopped quad
            SkChopQuadAt(dst, tmp, t);
            dst[0] = tmp[2];
            dst[1] = tmp[3];
        } else {
            // if chopMonoQuadAtY failed, then we may have hit inexact numerics
            // so we just clamp against the top
            for (int i = 0; i < 3; i++) {
                if (dst[i].fY < ctop) {
                    dst[i].fY = ctop;
                }
            }
        }
    }

    // are we partially below
    if (dst[2].fY > cbot) {
        if (chopMonoQuadAtY(dst, cbot, &t)) {
            SkChopQuadAt(dst, tmp, t);
            dst[1] = tmp[1];
            dst[2] = tmp[2];
        } else {
            // if chopMonoQuadAtY failed, then we may have hit inexact numerics
            // so we just clamp against the bottom
            for (int i = 0; i < 3; i++) {
                if (dst[i].fY > cbot) {
                    dst[i].fY = cbot;
                }
            }
        }
    }

    if (reverse) {
        SkTSwap<SkPoint>(dst[0], dst[2]);
    }
    return true;
}

