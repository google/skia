/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRectPriv_DEFINED
#define SkRectPriv_DEFINED

#include "include/core/SkRect.h"
#include "src/base/SkMathPriv.h"
#include "src/base/SkVx.h"

class SkM44;
class SkMatrix;

class SkRectPriv {
public:
    // Returns an irect that is very large, and can be safely round-trip with SkRect and still
    // be considered non-empty (i.e. width/height > 0) even if we round-out the SkRect.
    static SkIRect MakeILarge() {
        // SK_MaxS32 >> 1 seemed better, but it did not survive round-trip with SkRect and rounding.
        // Also, 1 << 29 can be perfectly represented in float, while SK_MaxS32 >> 1 cannot.
        const int32_t large = 1 << 29;
        return { -large, -large, large, large };
    }

    static SkIRect MakeILargestInverted() {
        return { SK_MaxS32, SK_MaxS32, SK_MinS32, SK_MinS32 };
    }

    static SkRect MakeLargeS32() {
        SkRect r;
        r.set(MakeILarge());
        return r;
    }

    static SkRect MakeLargest() {
        return { SK_ScalarMin, SK_ScalarMin, SK_ScalarMax, SK_ScalarMax };
    }

    static constexpr SkRect MakeLargestInverted() {
        return { SK_ScalarMax, SK_ScalarMax, SK_ScalarMin, SK_ScalarMin };
    }

    static void GrowToInclude(SkRect* r, const SkPoint& pt) {
        r->fLeft  =  std::min(pt.fX, r->fLeft);
        r->fRight =  std::max(pt.fX, r->fRight);
        r->fTop    = std::min(pt.fY, r->fTop);
        r->fBottom = std::max(pt.fY, r->fBottom);
    }

    // Conservative check if r can be expressed in fixed-point.
    // Will return false for very large values that might have fit
    static bool FitsInFixed(const SkRect& r) {
        return SkFitsInFixed(r.fLeft) && SkFitsInFixed(r.fTop) &&
               SkFitsInFixed(r.fRight) && SkFitsInFixed(r.fBottom);
    }

    // Returns r.width()/2 but divides first to avoid width() overflowing.
    static constexpr float HalfWidth(const SkRect& r) {
        return sk_float_midpoint(-r.fLeft, r.fRight);
    }
    // Returns r.height()/2 but divides first to avoid height() overflowing.
    static constexpr float HalfHeight(const SkRect& r) {
        return sk_float_midpoint(-r.fTop, r.fBottom);
    }

    // Evaluate A-B. If the difference shape cannot be represented as a rectangle then false is
    // returned and 'out' is set to the largest rectangle contained in said shape. If true is
    // returned then A-B is representable as a rectangle, which is stored in 'out'.
    static bool Subtract(const SkRect& a, const SkRect& b, SkRect* out);
    static bool Subtract(const SkIRect& a, const SkIRect& b, SkIRect* out);

    // Evaluate A-B, and return the largest rectangle contained in that shape (since the difference
    // may not be representable as rectangle). The returned rectangle will not intersect B.
    static SkRect Subtract(const SkRect& a, const SkRect& b) {
        SkRect diff;
        Subtract(a, b, &diff);
        return diff;
    }
    static SkIRect Subtract(const SkIRect& a, const SkIRect& b) {
        SkIRect diff;
        Subtract(a, b, &diff);
        return diff;
    }

    // Returns true if the quadrilateral formed by transforming the four corners of 'a' contains 'b'
    // 'tol' is in the same coordinate space as 'b', to treat 'b' as 'tol' units inset.
    static bool QuadContainsRect(const SkMatrix& m,
                                 const SkIRect& a,
                                 const SkIRect& b,
                                 float tol=0.f);
    static bool QuadContainsRect(const SkM44& m, const SkRect& a, const SkRect& b, float tol=0.f);
    // Like QuadContainsRect() but returns the edge test masks ordered T, R, B, L.
    static skvx::int4 QuadContainsRectMask(const SkM44& m, const SkRect& a, const SkRect& b,
                                           float tol=0.f);

    // Assuming 'src' does not intersect 'dst', returns the edge or corner of 'src' that is closest
    // to 'dst', e.g. the pixels that would be sampled from 'src' when clamp-tiled into 'dst'.
    //
    // The returned rectangle will not be empty if 'src' is not empty and 'dst' is not empty.
    // At least one of its width or height will be equal to 1 (possibly both if a corner is closest)
    //
    // Returns src.intersect(dst) if they do actually intersect.
    static SkIRect ClosestDisjointEdge(const SkIRect& src, const SkIRect& dst);
};


#endif
