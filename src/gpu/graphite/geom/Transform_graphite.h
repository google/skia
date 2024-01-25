/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_geom_Transform_DEFINED
#define skgpu_graphite_geom_Transform_DEFINED

#include "include/core/SkM44.h"

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
        // but there is no need to perform perspective division or w-plane clipping. This also
        // includes orthographic projections.
        kAffine,
        // The matrix includes perspective and requires further projection to 2D, so care must be
        // taken when w is less than or near 0, and homogeneous division and perspective-correct
        // interpolation are needed when rendering.
        kPerspective,
        // The matrix is not invertible or not finite, so should not be used to draw.
        kInvalid,
    };

    explicit Transform(const SkM44& m);
    Transform(const Transform& t) = default;

    static constexpr Transform Identity() {
        return Transform(SkM44(), SkM44(), Type::kIdentity, 1.f, 1.f);
    }
    static constexpr Transform Invalid() {
        return Transform(SkM44(SkM44::kNaN_Constructor), SkM44(SkM44::kNaN_Constructor),
                         Type::kInvalid, 1.f, 1.f);
    }

    static inline Transform Translate(float x, float y) {
        if (x == 0.f && y == 0.f) {
            return Identity();
        } else if (SkScalarsAreFinite(x, y)) {
            return Transform(SkM44::Translate(x, y), SkM44::Translate(-x, -y),
                             Type::kSimpleRectStaysRect, 1.f, 1.f);
        } else {
            return Invalid();
        }
    }

    static inline Transform Inverse(const Transform& t) {
        return Transform(t.fInvM, t.fM, t.fType, 1.f / t.fMaxScaleFactor, 1.f / t.fMinScaleFactor);
    }

    Transform& operator=(const Transform& t) = default;

    operator const SkM44&() const { return fM; }
    operator SkMatrix() const { return fM.asM33(); }

    bool operator!=(const Transform& t) const { return !(*this == t); }
    bool operator==(const Transform& t) const {
        return this->valid() == t.valid() && (!this->valid() || fM == t.fM);
    }

    const SkM44& matrix() const { return fM; }
    const SkM44& inverse() const { return fInvM; }

    Type type() const { return fType; }
    bool valid() const { return fType != Type::kInvalid; }

    // Return the {min,max} scale factor at the pre-transformed location 'p'. A unit circle about
    // 'p' transformed by this Transform will be contained in an ellipse with radii equal to 'min'
    // and 'max', e.g. moving 1 local unit will move at least 'min' pixels and at most 'max' pixels
    std::pair<float, float> scaleFactors(const SkV2& p) const;

    // This is valid for non-projection types and 1.0 for projection matrices.
    float maxScaleFactor() const {
        SkASSERT(this->valid());
        return fMaxScaleFactor;
    }

    // Return the minimum distance needed to move in local (pre-transform) space to ensure that the
    // transformed coordinates are at least 1px away from the original mapped point. This minimum
    // distance is specific to the given local 'bounds' since the scale factors change with
    // perspective.
    //
    // If the bounds would be clipped by the w=0 plane or otherwise is ill-conditioned, this will
    // return positive infinity.
    float localAARadius(const Rect& bounds) const;

    Rect mapRect(const Rect& rect) const;
    Rect inverseMapRect(const Rect& rect) const;

    void mapPoints(const Rect& localRect, SkV4 deviceOut[4]) const;
    void mapPoints(const SkV2* localIn, SkV4* deviceOut, int count) const;

    void mapPoints(const SkV4* localIn, SkV4* deviceOut, int count) const;
    void inverseMapPoints(const SkV4* deviceIn, SkV4* localOut, int count) const;

    // Returns a transform equal to the pre- or post-translation of this matrix
    Transform preTranslate(float x, float y) const {
        return this->concat(SkM44::Translate(x, y));
    }
    Transform postTranslate(float x, float y) const {
        return Translate(x, y).concat(*this);
    }

    // Returns a transform equal to (this * t)
    Transform concat(const Transform& t) const {
        SkASSERT(this->valid());
        return Transform(fM * t.fM);
    }
    Transform concat(const SkM44& t) const {
        SkASSERT(this->valid());
        return Transform(fM * t);
    }

    // Returns a transform equal to (this * t^-1)
    Transform concatInverse(const Transform& t) const {
        SkASSERT(this->valid());
        return Transform(fM * t.fInvM);
    }
    Transform concatInverse(const SkM44& t) const {
        SkASSERT(this->valid());
        // Saves a multiply compared to inverting just 't' and calculating both fM*t^-1 and t*fInvM
        // (t * this^-1)^-1 = this * t^-1
        return Inverse(Transform(t * fInvM));
    }

private:
    // Used for static factories that have known properties
    constexpr Transform(const SkM44& m, const SkM44& invM, Type type,
                        float minScale, float maxScale)
            : fM(m)
            , fInvM(invM)
            , fType(type)
            , fMinScaleFactor(minScale)
            , fMaxScaleFactor(maxScale) {}

    SkM44 fM;
    SkM44 fInvM; // M^-1
    Type  fType;

    // These are cached for non-projection transforms since they are constant; projection matrices
    // must be computed per point, and these values are ignored.
    float fMinScaleFactor = 1.f;
    float fMaxScaleFactor = 1.f;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_geom_Transform_DEFINED
