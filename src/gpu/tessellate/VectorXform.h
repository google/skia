/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef tessellate_VectorXform_DEFINED
#define tessellate_VectorXform_DEFINED

#include "include/core/SkMatrix.h"
#include "include/private/SkVx.h"

namespace skgpu::tess {

// Represents the upper-left 2x2 matrix of an affine transform for applying to vectors:
//
//     VectorXform(p1 - p0) == M * float3(p1, 1) - M * float3(p0, 1)
//
class VectorXform {
public:
    using float2 = skvx::Vec<2, float>;
    using float4 = skvx::Vec<4, float>;
    explicit VectorXform() : fType(Type::kIdentity) {}
    explicit VectorXform(const SkMatrix& m) { *this = m; }
    VectorXform& operator=(const SkMatrix& m) {
        SkASSERT(!m.hasPerspective());
        if (m.getType() & SkMatrix::kAffine_Mask) {
            fType = Type::kAffine;
            fScaleXSkewY = {m.getScaleX(), m.getSkewY()};
            fSkewXScaleY = {m.getSkewX(), m.getScaleY()};
            fScaleXYXY = {m.getScaleX(), m.getScaleY(), m.getScaleX(), m.getScaleY()};
            fSkewXYXY = {m.getSkewX(), m.getSkewY(), m.getSkewX(), m.getSkewY()};
        } else if (m.getType() & SkMatrix::kScale_Mask) {
            fType = Type::kScale;
            fScaleXY = {m.getScaleX(), m.getScaleY()};
            fScaleXYXY = {m.getScaleX(), m.getScaleY(), m.getScaleX(), m.getScaleY()};
        } else {
            SkASSERT(!(m.getType() & ~SkMatrix::kTranslate_Mask));
            fType = Type::kIdentity;
        }
        return *this;
    }
    float2 operator()(float2 vector) const {
        switch (fType) {
            case Type::kIdentity:
                return vector;
            case Type::kScale:
                return fScaleXY * vector;
            case Type::kAffine:
                return fScaleXSkewY * float2(vector[0]) + fSkewXScaleY * vector[1];
        }
        SkUNREACHABLE;
    }
    float4 operator()(float4 vectors) const {
        switch (fType) {
            case Type::kIdentity:
                return vectors;
            case Type::kScale:
                return vectors * fScaleXYXY;
            case Type::kAffine:
                return fScaleXYXY * vectors + fSkewXYXY * skvx::shuffle<1,0,3,2>(vectors);
        }
        SkUNREACHABLE;
    }
private:
    enum class Type { kIdentity, kScale, kAffine } fType;
    union { float2 fScaleXY, fScaleXSkewY; };
    float2 fSkewXScaleY;
    float4 fScaleXYXY;
    float4 fSkewXYXY;
};

}  // namespace skgpu::tess

#endif  // tessellate_VectorXform_DEFINED
