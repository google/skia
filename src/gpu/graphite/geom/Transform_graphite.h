/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_geom_Transform_DEFINED
#define skgpu_graphite_geom_Transform_DEFINED

#include "include/core/SkM44.h"

#include <algorithm>

namespace skgpu::graphite {

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
        // The matrix includes perspective or modifies Z and requires further projection to 2D,
        // so care must be taken when w is less than or near 0, and homogeneous division and
        // perspective-correct interpolation are needed when rendering.
        kProjection,
        // The matrix is not invertible or not finite, so should not be used to draw.
        kInvalid,
    };

    explicit Transform(const SkM44& m);
    Transform(const Transform& t) = default;

    static constexpr Transform Identity() {
        return Transform(SkM44(), SkM44(), Type::kIdentity, {1.f, 1.f});
    }
    static constexpr Transform Invalid() {
        return Transform(SkM44(SkM44::kNaN_Constructor), SkM44(), Type::kInvalid, {1.f, 1.f});
    }

    static inline Transform Translate(float x, float y) {
        if (SkScalarsAreFinite(x, y)) {
            return Transform(SkM44::Translate(x, y), SkM44::Translate(-x, -y),
                             Type::kSimpleRectStaysRect, {1.f, 1.f});
        } else {
            return Invalid();
        }
    }

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

    void mapPoints(const SkV4* localIn, SkV4* deviceOut, int count) const;
    void inverseMapPoints(const SkV4* deviceIn, SkV4* localOut, int count) const;

    // Returns a transform equal to the pre- or post-translating this matrix
    Transform preTranslate(float x, float y) const;
    Transform postTranslate(float x, float y) const;

    // Returns a transform equal to (this * t)
    Transform concat(const Transform& t) const;
    Transform concat(const SkM44& t) const { return Transform(fM * t); }

    // Returns a transform equal to (this * t^-1)
    Transform concatInverse(const Transform& t) const;
    Transform concatInverse(const SkM44& t) const;

private:
    constexpr Transform(const SkM44& m, const SkM44& invM, Type type, const SkV2 scale)
            : fM(m), fInvM(invM), fType(type), fScale(scale) {}

    SkM44 fM;
    SkM44 fInvM; // M^-1
    Type  fType;
    // TODO: It would be nice to have a scale factor for perspective too, and there is
    // SkMatrixPriv::DifferentialAreaScale but that requires a specific location.
    SkV2  fScale; // always > 0
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_geom_Transform_DEFINED
