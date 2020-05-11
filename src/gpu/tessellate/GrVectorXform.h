/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVectorXform_DEFINED
#define GrVectorXform_DEFINED

#include "include/core/SkMatrix.h"
#include "include/private/SkNx.h"

// We enclose this class in the anonymous namespace so it can have Sk2f/Sk4f members.
namespace {  // NOLINT(google-build-namespaces)

// Represents the upper-left 2x2 matrix of an affine transform for applying to vectors:
//
//     VectorXform(p1 - p0) == M * float3(p1, 1) - M * float3(p0, 1)
//
class GrVectorXform {
public:
    explicit GrVectorXform() : fType(Type::kIdentity) {}
    explicit GrVectorXform(const SkMatrix& m) {
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
    }
    Sk2f operator()(const Sk2f& vector) const {
        switch (fType) {
            case Type::kIdentity:
                return vector;
            case Type::kScale:
                return fScaleXY * vector;
            case Type::kAffine:
                return fScaleXSkewY * vector[0] + fSkewXScaleY * vector[1];
        }
        SkUNREACHABLE;
    }
    Sk4f operator()(const Sk4f& vectors) const {
        switch (fType) {
            case Type::kIdentity:
                return vectors;
            case Type::kScale:
                return vectors * fScaleXYXY;
            case Type::kAffine:
                return fScaleXYXY * vectors + fSkewXYXY * SkNx_shuffle<1,0,3,2>(vectors);
        }
        SkUNREACHABLE;
    }
private:
    enum class Type { kIdentity, kScale, kAffine } fType;
    union { Sk2f fScaleXY, fScaleXSkewY; };
    Sk2f fSkewXScaleY;
    Sk4f fScaleXYXY;
    Sk4f fSkewXYXY;
};

}  // namespace

#endif
