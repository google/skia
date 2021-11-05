/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef tessellate_AffineMatrix_DEFINED
#define tessellate_AffineMatrix_DEFINED

#include "include/core/SkMatrix.h"
#include "src/gpu/tessellate/Tessellation.h"

namespace skgpu {

// Applies an affine 2d transformation to points. Uses SIMD, but takes care to map points
// identically, regardless of which method is called.
//
// This class stores redundant data, so it is best used only as a stack-allocated object at the
// point of use.
class AffineMatrix {
public:
    AffineMatrix() = default;
    AffineMatrix(const SkMatrix& m) { *this = m; }

    AffineMatrix& operator=(const SkMatrix& m) {
        SkASSERT(!m.hasPerspective());
        // Duplicate the matrix in float4.lo and float4.hi so we can map two points at once.
        fScale = float2(m.getScaleX(), m.getScaleY()).xyxy();
        fSkew = float2(m.getSkewX(), m.getSkewY()).xyxy();
        fTrans = float2(m.getTranslateX(), m.getTranslateY()).xyxy();
        return *this;
    }

    SK_ALWAYS_INLINE float4 map2Points(float4 p0p1) const {
        return fScale * p0p1 + (fSkew * p0p1.yxwz() + fTrans);
    }

    SK_ALWAYS_INLINE float4 map2Points(const SkPoint pts[2]) const {
        return this->map2Points(float4::Load(pts));
    }

    SK_ALWAYS_INLINE float4 map2Points(SkPoint p0, SkPoint p1) const {
        return this->map2Points(float4(skvx::bit_pun<float2>(p0), skvx::bit_pun<float2>(p1)));
    }

    SK_ALWAYS_INLINE float2 mapPoint(float2 p) const {
        return fScale.lo * p + (fSkew.lo * p.yx() + fTrans.lo);
    }

    SK_ALWAYS_INLINE float2 map1Point(const SkPoint pt[1]) const {
        return this->mapPoint(float2::Load(pt));
    }

    SK_ALWAYS_INLINE SkPoint mapPoint(SkPoint p) const {
        return skvx::bit_pun<SkPoint>(this->mapPoint(skvx::bit_pun<float2>(p)));
    }

private:
    float4 fScale;
    float4 fSkew;
    float4 fTrans;
};

}  // namespace skgpu

#endif  // tessellate_AffineMatrix_DEFINED
