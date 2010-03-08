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

#include "SkEdgeClipper.h"
#include "SkGeometry.h"

static bool quick_reject(const SkRect& bounds, const SkRect& clip) {
    return bounds.fTop >= clip.fBottom || bounds.fBottom <= clip.fTop;
}

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

/*  src[] must be monotonic in Y. This routine copies src into dst, and sorts
 it to be increasing in Y. If it had to reverse the order of the points,
 it returns true, otherwise it returns false
 */
static bool sort_increasing_Y(SkPoint dst[], const SkPoint src[], int count) {
    // we need the data to be monotonically increasing in Y
    if (src[0].fY > src[count - 1].fY) {
        for (int i = 0; i < count; i++) {
            dst[i] = src[count - i - 1];
        }
        return true;
    } else {
        memcpy(dst, src, count * sizeof(SkPoint));
        return false;
    }
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

static bool chopMonoQuadAtX(SkPoint pts[3], SkScalar x, SkScalar* t) {
    return chopMonoQuadAt(pts[0].fX, pts[1].fX, pts[2].fX, x, t);
}

// Modify pts[] in place so that it is clipped in Y to the clip rect
static void chop_quad_in_Y(SkPoint pts[3], const SkRect& clip) {
    SkScalar t;
    SkPoint tmp[5]; // for SkChopQuadAt

    // are we partially above
    if (pts[0].fY < clip.fTop) {
        if (chopMonoQuadAtY(pts, clip.fTop, &t)) {
            // take the 2nd chopped quad
            SkChopQuadAt(pts, tmp, t);
            clamp_ge(tmp[2].fY, clip.fTop);
            clamp_ge(tmp[3].fY, clip.fTop);
            pts[0] = tmp[2];
            pts[1] = tmp[3];
        } else {
            // if chopMonoQuadAtY failed, then we may have hit inexact numerics
            // so we just clamp against the top
            for (int i = 0; i < 3; i++) {
                if (pts[i].fY < clip.fTop) {
                    pts[i].fY = clip.fTop;
                }
            }
        }
    }
    
    // are we partially below
    if (pts[2].fY > clip.fBottom) {
        if (chopMonoQuadAtY(pts, clip.fBottom, &t)) {
            SkChopQuadAt(pts, tmp, t);
            clamp_le(tmp[1].fY, clip.fBottom);
            clamp_le(tmp[2].fY, clip.fBottom);
            pts[1] = tmp[1];
            pts[2] = tmp[2];
        } else {
            // if chopMonoQuadAtY failed, then we may have hit inexact numerics
            // so we just clamp against the bottom
            for (int i = 0; i < 3; i++) {
                if (pts[i].fY > clip.fBottom) {
                    pts[i].fY = clip.fBottom;
                }
            }
        }
    }
}

// srcPts[] must be monotonic in X and Y
void SkEdgeClipper::clipMonoQuad(const SkPoint srcPts[3], const SkRect& clip) {
    SkPoint pts[3];
    bool reverse = sort_increasing_Y(pts, srcPts, 3);

    // are we completely above or below
    if (pts[2].fY <= clip.fTop || pts[0].fY >= clip.fBottom) {
        return;
    }
    
    // Now chop so that pts is contained within clip in Y
    chop_quad_in_Y(pts, clip);

    if (pts[0].fX > pts[2].fX) {
        SkTSwap<SkPoint>(pts[0], pts[2]);
        reverse = !reverse;
    }
    SkASSERT(pts[0].fX <= pts[1].fX);
    SkASSERT(pts[1].fX <= pts[2].fX);

    // Now chop in X has needed, and record the segments

    if (pts[2].fX <= clip.fLeft) {  // wholly to the left
        this->appendVLine(clip.fLeft, pts[0].fY, pts[2].fY, reverse);
        return;
    }
    if (pts[0].fX >= clip.fRight) {  // wholly to the right
        this->appendVLine(clip.fRight, pts[0].fY, pts[2].fY, reverse);
        return;
    }

    SkScalar t;
    SkPoint tmp[5]; // for SkChopQuadAt

    // are we partially to the left
    if (pts[0].fX < clip.fLeft) {
        if (chopMonoQuadAtX(pts, clip.fLeft, &t)) {
            SkChopQuadAt(pts, tmp, t);
            this->appendVLine(clip.fLeft, tmp[0].fY, tmp[2].fY, reverse);
            clamp_ge(tmp[2].fX, clip.fLeft);
            clamp_ge(tmp[3].fX, clip.fLeft);
            pts[0] = tmp[2];
            pts[1] = tmp[3];
        } else {
            // if chopMonoQuadAtY failed, then we may have hit inexact numerics
            // so we just clamp against the left
            this->appendVLine(clip.fLeft, pts[0].fY, pts[2].fY, reverse);
            return;
        }
    }
    
    // are we partially to the right
    if (pts[2].fX > clip.fRight) {
        if (chopMonoQuadAtX(pts, clip.fRight, &t)) {
            SkChopQuadAt(pts, tmp, t);
            clamp_le(tmp[1].fX, clip.fRight);
            clamp_le(tmp[2].fX, clip.fRight);
            this->appendQuad(tmp, reverse);
            this->appendVLine(clip.fRight, tmp[2].fY, tmp[4].fY, reverse);
        } else {
            // if chopMonoQuadAtY failed, then we may have hit inexact numerics
            // so we just clamp against the right
            this->appendVLine(clip.fRight, pts[0].fY, pts[2].fY, reverse);
        }
    } else {    // wholly inside the clip
        this->appendQuad(pts, reverse);
    }
}

bool SkEdgeClipper::clipQuad(const SkPoint srcPts[3], const SkRect& clip) {
    fCurrPoint = fPoints;
    fCurrVerb = fVerbs;

    SkRect  bounds;
    bounds.set(srcPts, 3);
    
    if (!quick_reject(bounds, clip)) {
        SkPoint monoY[5];
        int countY = SkChopQuadAtYExtrema(srcPts, monoY);
        for (int y = 0; y <= countY; y++) {
            SkPoint monoX[5];
            int countX = SkChopQuadAtXExtrema(&monoY[y * 2], monoX);
            for (int x = 0; x <= countX; x++) {
                this->clipMonoQuad(&monoX[x * 2], clip);
                SkASSERT(fCurrVerb - fVerbs < kMaxVerbs);
                SkASSERT(fCurrPoint - fPoints <= kMaxPoints);
            }
        }
    }

    *fCurrVerb = SkPath::kDone_Verb;
    fCurrPoint = fPoints;
    fCurrVerb = fVerbs;
    return SkPath::kDone_Verb != fVerbs[0];
}

///////////////////////////////////////////////////////////////////////////////

static SkScalar eval_cubic_coeff(SkScalar A, SkScalar B, SkScalar C,
                                 SkScalar D, SkScalar t) {
    return SkScalarMulAdd(SkScalarMulAdd(SkScalarMulAdd(A, t, B), t, C), t, D);
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
//    SkDebugf("-- evalCubicAt %d delta %g\n", i, eval_cubic_coeff(A, B, C, D, *t));
    return true;
}

static bool chopMonoCubicAtY(SkPoint pts[4], SkScalar y, SkScalar* t) {
    return chopMonoCubicAt(pts[0].fY, pts[1].fY, pts[2].fY, pts[3].fY, y, t);
}

static bool chopMonoCubicAtX(SkPoint pts[4], SkScalar x, SkScalar* t) {
    return chopMonoCubicAt(pts[0].fX, pts[1].fX, pts[2].fX, pts[3].fX, x, t);
}

// Modify pts[] in place so that it is clipped in Y to the clip rect
static void chop_cubic_in_Y(SkPoint pts[4], const SkRect& clip) {
    SkScalar t;
    SkPoint tmp[7]; // for SkChopCubicAt
    
    // are we partially above
    if (pts[0].fY < clip.fTop) {
        if (chopMonoCubicAtY(pts, clip.fTop, &t)) {
            SkChopCubicAt(pts, tmp, t);
            // given the imprecision of computing t, we just slam our Y coord
            // to the top of the clip. This also saves us in the bad case where
            // the t was soooo bad that the entire segment could have been
            // below fBottom
            tmp[3].fY = clip.fTop;
            clamp_ge(tmp[4].fY, clip.fTop);
            clamp_ge(tmp[5].fY, clip.fTop);
            pts[0] = tmp[3];
            pts[1] = tmp[4];
            pts[2] = tmp[5];
        } else {
            // if chopMonoCubicAtY failed, then we may have hit inexact numerics
            // so we just clamp against the top
            for (int i = 0; i < 4; i++) {
                clamp_ge(pts[i].fY, clip.fTop);
            }
        }
    }
    
    // are we partially below
    if (pts[3].fY > clip.fBottom) {
        if (chopMonoCubicAtY(pts, clip.fBottom, &t)) {
            SkChopCubicAt(pts, tmp, t);
            clamp_le(tmp[1].fY, clip.fBottom);
            clamp_le(tmp[2].fY, clip.fBottom);
            clamp_le(tmp[3].fY, clip.fBottom);
            pts[1] = tmp[1];
            pts[2] = tmp[2];
            pts[3] = tmp[3];
        } else {
            // if chopMonoCubicAtY failed, then we may have hit inexact numerics
            // so we just clamp against the bottom
            for (int i = 0; i < 4; i++) {
                clamp_le(pts[i].fY, clip.fBottom);
            }
        }
    }
}

// srcPts[] must be monotonic in X and Y
void SkEdgeClipper::clipMonoCubic(const SkPoint src[4], const SkRect& clip) {
    SkPoint pts[4];
    bool reverse = sort_increasing_Y(pts, src, 4);
    
    // are we completely above or below
    if (pts[3].fY <= clip.fTop || pts[0].fY >= clip.fBottom) {
        return;
    }

    // Now chop so that pts is contained within clip in Y
    chop_cubic_in_Y(pts, clip);

    if (pts[0].fX > pts[3].fX) {
        SkTSwap<SkPoint>(pts[0], pts[3]);
        SkTSwap<SkPoint>(pts[1], pts[2]);
        reverse = !reverse;
    }
    
    // Now chop in X has needed, and record the segments
    
    if (pts[3].fX <= clip.fLeft) {  // wholly to the left
        this->appendVLine(clip.fLeft, pts[0].fY, pts[3].fY, reverse);
        return;
    }
    if (pts[0].fX >= clip.fRight) {  // wholly to the right
        this->appendVLine(clip.fRight, pts[0].fY, pts[3].fY, reverse);
        return;
    }
    
    SkScalar t;
    SkPoint tmp[7];
    
    // are we partially to the left
    if (pts[0].fX < clip.fLeft) {
        if (chopMonoCubicAtX(pts, clip.fLeft, &t)) {
            SkChopCubicAt(pts, tmp, t);
            this->appendVLine(clip.fLeft, tmp[0].fY, tmp[3].fY, reverse);
            clamp_ge(tmp[3].fX, clip.fLeft);
            clamp_ge(tmp[4].fX, clip.fLeft);
            clamp_ge(tmp[5].fX, clip.fLeft);
            pts[0] = tmp[3];
            pts[1] = tmp[4];
            pts[2] = tmp[5];
        } else {
            // if chopMonocubicAtY failed, then we may have hit inexact numerics
            // so we just clamp against the left
            this->appendVLine(clip.fLeft, pts[0].fY, pts[3].fY, reverse);
            return;
        }
    }
    
    // are we partially to the right
    if (pts[3].fX > clip.fRight) {
        if (chopMonoCubicAtX(pts, clip.fRight, &t)) {
            SkChopCubicAt(pts, tmp, t);
            clamp_le(tmp[1].fX, clip.fRight);
            clamp_le(tmp[2].fX, clip.fRight);
            clamp_le(tmp[3].fX, clip.fRight);
            this->appendCubic(tmp, reverse);
            this->appendVLine(clip.fRight, tmp[3].fY, tmp[6].fY, reverse);
        } else {
            // if chopMonoCubicAtX failed, then we may have hit inexact numerics
            // so we just clamp against the right
            this->appendVLine(clip.fRight, pts[0].fY, pts[3].fY, reverse);
        }
    } else {    // wholly inside the clip
        this->appendCubic(pts, reverse);
    }
}

bool SkEdgeClipper::clipCubic(const SkPoint srcPts[4], const SkRect& clip) {
    fCurrPoint = fPoints;
    fCurrVerb = fVerbs;
    
    SkRect  bounds;
    bounds.set(srcPts, 4);
    
    if (!quick_reject(bounds, clip)) {
        SkPoint monoY[10];
        int countY = SkChopCubicAtYExtrema(srcPts, monoY);
        for (int y = 0; y <= countY; y++) {
        //    sk_assert_monotonic_y(&monoY[y * 3], 4);
            SkPoint monoX[10];
            int countX = SkChopCubicAtXExtrema(&monoY[y * 3], monoX);
            for (int x = 0; x <= countX; x++) {
            //    sk_assert_monotonic_y(&monoX[x * 3], 4);
            //    sk_assert_monotonic_x(&monoX[x * 3], 4);
                this->clipMonoCubic(&monoX[x * 3], clip);
                SkASSERT(fCurrVerb - fVerbs < kMaxVerbs);
                SkASSERT(fCurrPoint - fPoints <= kMaxPoints);
            }
        }
    }
    
    *fCurrVerb = SkPath::kDone_Verb;
    fCurrPoint = fPoints;
    fCurrVerb = fVerbs;
    return SkPath::kDone_Verb != fVerbs[0];
}

///////////////////////////////////////////////////////////////////////////////

void SkEdgeClipper::appendVLine(SkScalar x, SkScalar y0, SkScalar y1,
                                bool reverse) {
    *fCurrVerb++ = SkPath::kLine_Verb;
    
    if (reverse) {
        SkTSwap<SkScalar>(y0, y1);
    }
    fCurrPoint[0].set(x, y0);
    fCurrPoint[1].set(x, y1);
    fCurrPoint += 2;
}

void SkEdgeClipper::appendQuad(const SkPoint pts[3], bool reverse) {
    *fCurrVerb++ = SkPath::kQuad_Verb;
    
    if (reverse) {
        fCurrPoint[0] = pts[2];
        fCurrPoint[2] = pts[0];
    } else {
        fCurrPoint[0] = pts[0];
        fCurrPoint[2] = pts[2];
    }
    fCurrPoint[1] = pts[1];
    fCurrPoint += 3;
}

void SkEdgeClipper::appendCubic(const SkPoint pts[4], bool reverse) {
    *fCurrVerb++ = SkPath::kCubic_Verb;
    
    if (reverse) {
        for (int i = 0; i < 4; i++) {
            fCurrPoint[i] = pts[3 - i];
        }
    } else {
        memcpy(fCurrPoint, pts, 4 * sizeof(SkPoint));
    }
    fCurrPoint += 4;
}

SkPath::Verb SkEdgeClipper::next(SkPoint pts[]) {
    SkPath::Verb verb = *fCurrVerb;

    switch (verb) {
        case SkPath::kLine_Verb:
            memcpy(pts, fCurrPoint, 2 * sizeof(SkPoint));
            fCurrPoint += 2;
            fCurrVerb += 1;
            break;
        case SkPath::kQuad_Verb:
            memcpy(pts, fCurrPoint, 3 * sizeof(SkPoint));
            fCurrPoint += 3;
            fCurrVerb += 1;
            break;
        case SkPath::kCubic_Verb:
            memcpy(pts, fCurrPoint, 4 * sizeof(SkPoint));
            fCurrPoint += 4;
            fCurrVerb += 1;
            break;
        case SkPath::kDone_Verb:
            break;
        default:
            SkASSERT(!"unexpected verb in quadclippper2 iter");
            break;
    }
    return verb;
}

///////////////////////////////////////////////////////////////////////////////

#ifdef SK_DEBUG
static void assert_monotonic(const SkScalar coord[], int count) {
    if (coord[0] > coord[(count - 1) * 2]) {
        for (int i = 1; i < count; i++) {
            SkASSERT(coord[2 * (i - 1)] >= coord[i * 2]);
        }
    } else if (coord[0] < coord[(count - 1) * 2]) {
        for (int i = 1; i < count; i++) {
            SkASSERT(coord[2 * (i - 1)] <= coord[i * 2]);
        }
    } else {
        for (int i = 1; i < count; i++) {
            SkASSERT(coord[2 * (i - 1)] == coord[i * 2]);
        }
    }
}

void sk_assert_monotonic_y(const SkPoint pts[], int count) {
    if (count > 1) {
        assert_monotonic(&pts[0].fY, count);
    }
}

void sk_assert_monotonic_x(const SkPoint pts[], int count) {
    if (count > 1) {
        assert_monotonic(&pts[0].fX, count);
    }
}
#endif
