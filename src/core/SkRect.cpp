/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkRect.h"

#include "include/core/SkM44.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkTPin.h"
#include "src/core/SkRectPriv.h"

class SkMatrix;

bool SkIRect::intersect(const SkIRect& a, const SkIRect& b) {
    SkIRect tmp = {
        std::max(a.fLeft,   b.fLeft),
        std::max(a.fTop,    b.fTop),
        std::min(a.fRight,  b.fRight),
        std::min(a.fBottom, b.fBottom)
    };
    if (tmp.isEmpty()) {
        return false;
    }
    *this = tmp;
    return true;
}

void SkIRect::join(const SkIRect& r) {
    // do nothing if the params are empty
    if (r.fLeft >= r.fRight || r.fTop >= r.fBottom) {
        return;
    }

    // if we are empty, just assign
    if (fLeft >= fRight || fTop >= fBottom) {
        *this = r;
    } else {
        if (r.fLeft < fLeft)     fLeft = r.fLeft;
        if (r.fTop < fTop)       fTop = r.fTop;
        if (r.fRight > fRight)   fRight = r.fRight;
        if (r.fBottom > fBottom) fBottom = r.fBottom;
    }
}

/////////////////////////////////////////////////////////////////////////////

void SkRect::toQuad(SkPoint quad[4]) const {
    SkASSERT(quad);

    quad[0].set(fLeft, fTop);
    quad[1].set(fRight, fTop);
    quad[2].set(fRight, fBottom);
    quad[3].set(fLeft, fBottom);
}

#include "src/base/SkVx.h"

bool SkRect::setBoundsCheck(const SkPoint pts[], int count) {
    SkASSERT((pts && count > 0) || count == 0);

    if (count <= 0) {
        this->setEmpty();
        return true;
    }

    skvx::float4 min, max;
    if (count & 1) {
        min = max = skvx::float2::Load(pts).xyxy();
        pts   += 1;
        count -= 1;
    } else {
        min = max = skvx::float4::Load(pts);
        pts   += 2;
        count -= 2;
    }

    skvx::float4 accum = min * 0;
    while (count) {
        skvx::float4 xy = skvx::float4::Load(pts);
        accum = accum * xy;
        min = skvx::min(min, xy);
        max = skvx::max(max, xy);
        pts   += 2;
        count -= 2;
    }

    const bool all_finite = all(accum * 0 == 0);
    if (all_finite) {
        this->setLTRB(std::min(min[0], min[2]), std::min(min[1], min[3]),
                      std::max(max[0], max[2]), std::max(max[1], max[3]));
    } else {
        this->setEmpty();
    }
    return all_finite;
}

void SkRect::setBoundsNoCheck(const SkPoint pts[], int count) {
    if (!this->setBoundsCheck(pts, count)) {
        this->setLTRB(SK_FloatNaN, SK_FloatNaN, SK_FloatNaN, SK_FloatNaN);
    }
}

#define CHECK_INTERSECT(al, at, ar, ab, bl, bt, br, bb) \
    float L = std::max(al, bl);                         \
    float R = std::min(ar, br);                         \
    float T = std::max(at, bt);                         \
    float B = std::min(ab, bb);                         \
    do { if (!(L < R && T < B)) return false; } while (0)
    // do the !(opposite) check so we return false if either arg is NaN

bool SkRect::intersect(const SkRect& r) {
    CHECK_INTERSECT(r.fLeft, r.fTop, r.fRight, r.fBottom, fLeft, fTop, fRight, fBottom);
    this->setLTRB(L, T, R, B);
    return true;
}

bool SkRect::intersect(const SkRect& a, const SkRect& b) {
    CHECK_INTERSECT(a.fLeft, a.fTop, a.fRight, a.fBottom, b.fLeft, b.fTop, b.fRight, b.fBottom);
    this->setLTRB(L, T, R, B);
    return true;
}

void SkRect::join(const SkRect& r) {
    if (r.isEmpty()) {
        return;
    }

    if (this->isEmpty()) {
        *this = r;
    } else {
        fLeft   = std::min(fLeft, r.fLeft);
        fTop    = std::min(fTop, r.fTop);
        fRight  = std::max(fRight, r.fRight);
        fBottom = std::max(fBottom, r.fBottom);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////

#include "include/core/SkString.h"
#include "src/core/SkStringUtils.h"

static const char* set_scalar(SkString* storage, float value, SkScalarAsStringType asType) {
    storage->reset();
    SkAppendScalar(storage, value, asType);
    return storage->c_str();
}

SkString SkRect::dumpToString(bool asHex) const {
    SkScalarAsStringType asType = asHex ? kHex_SkScalarAsStringType : kDec_SkScalarAsStringType;

    SkString line;
    if (asHex) {
        SkString tmp;
        line.printf( "SkRect::MakeLTRB(%s, /* %f */\n", set_scalar(&tmp, fLeft, asType), fLeft);
        line.appendf("                 %s, /* %f */\n", set_scalar(&tmp, fTop, asType), fTop);
        line.appendf("                 %s, /* %f */\n", set_scalar(&tmp, fRight, asType), fRight);
        line.appendf("                 %s  /* %f */);", set_scalar(&tmp, fBottom, asType), fBottom);
    } else {
        SkString strL, strT, strR, strB;
        SkAppendScalarDec(&strL, fLeft);
        SkAppendScalarDec(&strT, fTop);
        SkAppendScalarDec(&strR, fRight);
        SkAppendScalarDec(&strB, fBottom);
        line.printf("SkRect::MakeLTRB(%s, %s, %s, %s);",
                    strL.c_str(), strT.c_str(), strR.c_str(), strB.c_str());
    }
    return line;
}

void SkRect::dump(bool asHex) const {
    SkDebugf("%s\n", this->dumpToString(asHex).c_str());
}

////////////////////////////////////////////////////////////////////////////////////////////////

template<typename R>
static bool subtract(const R& a, const R& b, R* out) {
    if (a.isEmpty() || b.isEmpty() || !R::Intersects(a, b)) {
        // Either already empty, or subtracting the empty rect, or there's no intersection, so
        // in all cases the answer is A.
        *out = a;
        return true;
    }

    // 4 rectangles to consider. If the edge in A is contained in B, the resulting difference can
    // be represented exactly as a rectangle. Otherwise the difference is the largest subrectangle
    // that is disjoint from B:
    // 1. Left part of A:   (A.left,  A.top,    B.left,  A.bottom)
    // 2. Right part of A:  (B.right, A.top,    A.right, A.bottom)
    // 3. Top part of A:    (A.left,  A.top,    A.right, B.top)
    // 4. Bottom part of A: (A.left,  B.bottom, A.right, A.bottom)
    //
    // Depending on how B intersects A, there will be 1 to 4 positive areas:
    //  - 4 occur when A contains B
    //  - 3 occur when B intersects a single edge
    //  - 2 occur when B intersects at a corner, or spans two opposing edges
    //  - 1 occurs when B spans two opposing edges and contains a 3rd, resulting in an exact rect
    //  - 0 occurs when B contains A, resulting in the empty rect
    //
    // Compute the relative areas of the 4 rects described above. Since each subrectangle shares
    // either the width or height of A, we only have to divide by the other dimension, which avoids
    // overflow on int32 types, and even if the float relative areas overflow to infinity, the
    // comparisons work out correctly and (one of) the infinitely large subrects will be chosen.
    float aHeight = (float) a.height();
    float aWidth = (float) a.width();
    float leftArea = 0.f, rightArea = 0.f, topArea = 0.f, bottomArea = 0.f;
    int positiveCount = 0;
    if (b.fLeft > a.fLeft) {
        leftArea = (b.fLeft - a.fLeft) / aWidth;
        positiveCount++;
    }
    if (a.fRight > b.fRight) {
        rightArea = (a.fRight - b.fRight) / aWidth;
        positiveCount++;
    }
    if (b.fTop > a.fTop) {
        topArea = (b.fTop - a.fTop) / aHeight;
        positiveCount++;
    }
    if (a.fBottom > b.fBottom) {
        bottomArea = (a.fBottom - b.fBottom) / aHeight;
        positiveCount++;
    }

    if (positiveCount == 0) {
        SkASSERT(b.contains(a));
        *out = R::MakeEmpty();
        return true;
    }

    *out = a;
    if (leftArea > rightArea && leftArea > topArea && leftArea > bottomArea) {
        // Left chunk of A, so the new right edge is B's left edge
        out->fRight = b.fLeft;
    } else if (rightArea > topArea && rightArea > bottomArea) {
        // Right chunk of A, so the new left edge is B's right edge
        out->fLeft = b.fRight;
    } else if (topArea > bottomArea) {
        // Top chunk of A, so the new bottom edge is B's top edge
        out->fBottom = b.fTop;
    } else {
        // Bottom chunk of A, so the new top edge is B's bottom edge
        SkASSERT(bottomArea > 0.f);
        out->fTop = b.fBottom;
    }

    // If we have 1 valid area, the disjoint shape is representable as a rectangle.
    SkASSERT(!R::Intersects(*out, b));
    return positiveCount == 1;
}

bool SkRectPriv::Subtract(const SkRect& a, const SkRect& b, SkRect* out) {
    return subtract<SkRect>(a, b, out);
}

bool SkRectPriv::Subtract(const SkIRect& a, const SkIRect& b, SkIRect* out) {
    return subtract<SkIRect>(a, b, out);
}


bool SkRectPriv::QuadContainsRect(const SkMatrix& m,
                                  const SkIRect& a,
                                  const SkIRect& b,
                                  float tol) {
    return QuadContainsRect(SkM44(m), SkRect::Make(a), SkRect::Make(b), tol);
}

bool SkRectPriv::QuadContainsRect(const SkM44& m, const SkRect& a, const SkRect& b, float tol) {
    return all(QuadContainsRectMask(m, a, b, tol));
}

skvx::int4 SkRectPriv::QuadContainsRectMask(const SkM44& m,
                                            const SkRect& a,
                                            const SkRect& b,
                                            float tol) {
    SkDEBUGCODE(SkM44 inverse;)
    SkASSERT(m.invert(&inverse));
    // With empty rectangles, the calculated edges could give surprising results. If 'a' were not
    // sorted, its normals would point outside the sorted rectangle, so lots of potential rects
    // would be seen as "contained". If 'a' is all 0s, its edge equations are also (0,0,0) so every
    // point has a distance of 0, and would be interpreted as inside.
    if (a.isEmpty()) {
        return skvx::int4(0); // all "false"
    }
    // However, 'b' is only used to define its 4 corners to check against the transformed edges.
    // This is valid regardless of b's emptiness or sortedness.

    // Calculate the 4 homogenous coordinates of 'a' transformed by 'm' where Z=0 and W=1.
    auto ax = skvx::float4{a.fLeft, a.fRight, a.fRight, a.fLeft};
    auto ay = skvx::float4{a.fTop, a.fTop, a.fBottom, a.fBottom};

    auto max = m.rc(0,0)*ax + m.rc(0,1)*ay + m.rc(0,3);
    auto may = m.rc(1,0)*ax + m.rc(1,1)*ay + m.rc(1,3);
    auto maw = m.rc(3,0)*ax + m.rc(3,1)*ay + m.rc(3,3);

    if (all(maw < 0.f)) {
        // If all points of A are mapped to w < 0, then the edge equations end up representing the
        // convex hull of projected points when A should in fact be considered empty.
        return skvx::int4(0); // all "false"
    }

    // Cross product of adjacent vertices provides homogenous lines for the 4 sides of the quad
    auto lA = may*skvx::shuffle<1,2,3,0>(maw) - maw*skvx::shuffle<1,2,3,0>(may);
    auto lB = maw*skvx::shuffle<1,2,3,0>(max) - max*skvx::shuffle<1,2,3,0>(maw);
    auto lC = max*skvx::shuffle<1,2,3,0>(may) - may*skvx::shuffle<1,2,3,0>(max);

    // Before transforming, the corners of 'a' were in CW order, but afterwards they may become CCW,
    // so the sign corrects the direction of the edge normals to point inwards.
    float sign = (lA[0]*lB[1] - lB[0]*lA[1]) < 0 ? -1.f : 1.f;

    // Calculate distance from 'b' to each edge. Since 'b' has presumably been transformed by 'm'
    // *and* projected, this assumes W = 1.
    SkRect bInset = b.makeInset(tol, tol);
    auto d0 = sign * (lA*bInset.fLeft  + lB*bInset.fTop    + lC);
    auto d1 = sign * (lA*bInset.fRight + lB*bInset.fTop    + lC);
    auto d2 = sign * (lA*bInset.fRight + lB*bInset.fBottom + lC);
    auto d3 = sign * (lA*bInset.fLeft  + lB*bInset.fBottom + lC);

    // 'b' is contained in the mapped rectangle if all distances are >= 0
    return (d0 >= 0.f) & (d1 >= 0.f) & (d2 >= 0.f) & (d3 >= 0.f);
}

SkIRect SkRectPriv::ClosestDisjointEdge(const SkIRect& src, const SkIRect& dst) {
    if (src.isEmpty() || dst.isEmpty()) {
        return SkIRect::MakeEmpty();
    }

    int l = src.fLeft;
    int r = src.fRight;
    if (r <= dst.fLeft) {
        // Select right column of pixels in crop
        l = r - 1;
    } else if (l >= dst.fRight) {
        // Left column of 'crop'
        r = l + 1;
    } else {
        // Regular intersection along X axis.
        l = SkTPin(l, dst.fLeft, dst.fRight);
        r = SkTPin(r, dst.fLeft, dst.fRight);
    }

    int t = src.fTop;
    int b = src.fBottom;
    if (b <= dst.fTop) {
        // Select bottom row of pixels in crop
        t = b - 1;
    } else if (t >= dst.fBottom) {
        // Top row of 'crop'
        b = t + 1;
    } else {
        t = SkTPin(t, dst.fTop, dst.fBottom);
        b = SkTPin(b, dst.fTop, dst.fBottom);
    }

    return SkIRect::MakeLTRB(l,t,r,b);
}
