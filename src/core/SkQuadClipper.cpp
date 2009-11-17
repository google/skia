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

///////////////////////////////////////////////////////////////////////////////

// Modify pts[] in place so that it is clipped in Y to the clip rect
static void chop_quad_in_Y(SkPoint pts[3], const SkRect& clip) {
    SkScalar t;
    SkPoint tmp[5]; // for SkChopQuadAt

    // are we partially above
    if (pts[0].fY < clip.fTop) {
        if (chopMonoQuadAtY(pts, clip.fTop, &t)) {
            // take the 2nd chopped quad
            SkChopQuadAt(pts, tmp, t);
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

/*  src[] must be monotonic in Y. This routine copies src into dst, and sorts
    it to be increasing in Y. If it had to reverse the order of the points,
    it returns true, otherwise it returns false
 */
static bool sort_increasing_Y(SkPoint dst[], const SkPoint src[]) {
    // we need the data to be monotonically increasing in Y
    if (src[0].fY > src[2].fY) {
        SkASSERT(src[0].fY >= src[1].fY);
        SkASSERT(src[1].fY >= src[2].fY);
        dst[0] = src[2];
        dst[1] = src[1];
        dst[2] = src[0];
        return true;
    } else {
        SkASSERT(src[2].fY >= src[1].fY);
        SkASSERT(src[1].fY >= src[0].fY);
        memcpy(dst, src, 3 * sizeof(SkPoint));
        return false;
    }
}

// srcPts[] must be monotonic in X and Y
void SkQuadClipper2::clipMonoQuad(const SkPoint srcPts[3], const SkRect& clip) {
    SkPoint pts[3];
    bool reverse = sort_increasing_Y(pts, srcPts);

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
            pts[0] = tmp[2];
            pts[1] = tmp[3];
        } else {
            // if chopMonoQuadAtY failed, then we may have hit inexact numerics
            // so we just clamp against the left
            this->appendVLine(clip.fLeft, pts[0].fY, pts[2].fY, reverse);
        }
    }
    
    // are we partially to the right
    if (pts[2].fX > clip.fRight) {
        if (chopMonoQuadAtX(pts, clip.fRight, &t)) {
            SkChopQuadAt(pts, tmp, t);
            this->appendQuad(tmp, reverse);
            this->appendVLine(clip.fRight, tmp[2].fY, tmp[4].fY, reverse);
        } else {
            // if chopMonoQuadAtY failed, then we may have hit inexact numerics
            // so we just clamp against the right
            this->appendVLine(clip.fRight, pts[0].fY, pts[3].fY, reverse);
        }
    } else {    // wholly inside the clip
        this->appendQuad(pts, reverse);
    }
}

static bool quick_reject_quad(const SkPoint srcPts[3], const SkRect& clip) {
    return (srcPts[0].fY <= clip.fTop &&
            srcPts[1].fY <= clip.fTop &&
            srcPts[2].fY <= clip.fTop)
            ||
           (srcPts[0].fY >= clip.fBottom &&
            srcPts[1].fY >= clip.fBottom &&
            srcPts[2].fY >= clip.fBottom);
}

bool SkQuadClipper2::clipQuad(const SkPoint srcPts[3], const SkRect& clip) {
    fCurrPoint = fPoints;
    fCurrVerb = fVerbs;

    if (!quick_reject_quad(srcPts, clip)) {
        SkPoint monoY[5];
        int countY = SkChopQuadAtYExtrema(srcPts, monoY);
        for (int y = 0; y <= countY; y++) {
            SkPoint monoX[5];
            int countX = SkChopQuadAtXExtrema(&monoY[y * 2], monoX);
            SkASSERT(countY + countX <= 3);
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

void SkQuadClipper2::appendVLine(SkScalar x, SkScalar y0, SkScalar y1,
                                 bool reverse) {
    *fCurrVerb++ = SkPath::kLine_Verb;

    if (reverse) {
        SkTSwap<SkScalar>(y0, y1);
    }
    fCurrPoint[0].set(x, y0);
    fCurrPoint[1].set(x, y1);
    fCurrPoint += 2;
}

void SkQuadClipper2::appendQuad(const SkPoint pts[3], bool reverse) {
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

SkPath::Verb SkQuadClipper2::next(SkPoint pts[]) {
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
        case SkPath::kDone_Verb:
            break;
        default:
            SkASSERT(!"unexpected verb in quadclippper2 iter");
            break;
    }
    return verb;
}

