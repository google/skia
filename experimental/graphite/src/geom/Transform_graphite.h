/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_geom_Transform_DEFINED
#define skgpu_geom_Transform_DEFINED

#include "include/core/SkM44.h"

namespace skgpu {

class Rect;

// Transform encapsulates an SkM44 matrix, its inverse, and other properties dependent on the
// original matrix value that are useful when rendering.
class Transform {
public:
    // Type classifies the transform into coarse categories so that certain optimizations or
    // properties can be queried efficiently
    enum class Type : unsigned {
        // Applying the matrix to a vector or point is a no-op, so could be skipped entirely.
        kIdentity,
        // The matrix transforms a rect to another rect, without mirrors or rotations, so both
        // pre-and-post transform coordinates can be exactly represented as rects.
        kSimpleRectStaysRect,
        // The matrix transforms a rect to another rect, but may mirror or rotate the corners
        // relative to each other. This means that the post-transformed rect completely fills
        // that space.
        kRectStaysRect,
        // The matrix transform may have skew or rotation, so a mapped rect does not fill space,
        // but there is no need to perform perspective division or w-plane clipping.
        kAffine,
        // The matrix includes perspective, so care must be taken when w is less than or near 0,
        // and perspective division and interpolation are needed for correct rendering.
        kPerspective,
        // The matrix is not invertible or not finite, so should not be used to draw.
        kInvalid,
    };

    explicit Transform(const SkM44& m);
    Transform(const Transform& t) = default;

    Transform& operator=(const Transform& t) = default;

    operator const SkM44&() const { return fM; }
    operator SkMatrix() const { return fM.asM33(); }

    bool operator==(const Transform& t) const;
    bool operator!=(const Transform& t) const { return !(*this == t); }

    const SkM44& matrix() const { return fM; }
    const SkM44& inverse() const { return fInvM; }

    const SkV2& scaleFactors() const { return fScale; }
    float maxScaleFactor() const { return std::max(fScale.x, fScale.y); }

    Type type() const { return fType; }
    bool valid() const { return fType != Type::kInvalid; }

    Rect mapRect(const Rect& rect) const;
    Rect inverseMapRect(const Rect& rect) const;

    void mapPoints(const Rect& localRect, SkV4 deviceOut[4]) const;
    void mapPoints(const SkV2* localIn, SkV4* deviceOut, int count) const;

private:
    SkM44 fM;
    SkM44 fInvM; // M^-1
    Type  fType;
    // TODO: It would be nice to have a scale factor for perspective too, and there is
    // SkMatrixPriv::DifferentialAreaScale but that requires a specific location.
    SkV2  fScale; // always > 0
};

} // namespace skgpu

#endif // skgpu_geom_Transform_DEFINED
