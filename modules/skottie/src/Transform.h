/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieTransform_DEFINED
#define SkottieTransform_DEFINED

#include "modules/skottie/src/SkottieAdapter.h"

#include "include/core/SkMatrix.h"
#include "include/core/SkMatrix44.h"

namespace skottie {
namespace internal {

class TransformAdapter2D final : public SkNVRefCnt<TransformAdapter2D> {
public:
    explicit TransformAdapter2D(sk_sp<sksg::Matrix<SkMatrix>>);
    ~TransformAdapter2D();

    ADAPTER_PROPERTY(AnchorPoint, SkPoint , SkPoint::Make(0, 0))
    ADAPTER_PROPERTY(Position   , SkPoint , SkPoint::Make(0, 0))
    ADAPTER_PROPERTY(Scale      , SkVector, SkPoint::Make(100, 100))
    ADAPTER_PROPERTY(Rotation   , SkScalar, 0)
    ADAPTER_PROPERTY(Skew       , SkScalar, 0)
    ADAPTER_PROPERTY(SkewAxis   , SkScalar, 0)

    SkMatrix totalMatrix() const;

private:
    void apply();

    sk_sp<sksg::Matrix<SkMatrix>> fMatrixNode;
};

class TransformAdapter3D : public SkRefCnt {
public:
    TransformAdapter3D();
    ~TransformAdapter3D() override;

    struct Vec3 {
        float fX, fY, fZ;

        explicit Vec3(const VectorValue&);

        bool operator==(const Vec3& other) const {
            return fX == other.fX && fY == other.fY && fZ == other.fZ;
        }
        bool operator!=(const Vec3& other) const { return !(*this == other); }
    };

    ADAPTER_PROPERTY(AnchorPoint, Vec3, Vec3({  0,   0,   0}))
    ADAPTER_PROPERTY(Position   , Vec3, Vec3({  0,   0,   0}))
    ADAPTER_PROPERTY(Rotation   , Vec3, Vec3({  0,   0,   0}))
    ADAPTER_PROPERTY(Scale      , Vec3, Vec3({100, 100, 100}))

    sk_sp<sksg::Transform> refTransform() const;

protected:
    void apply();

private:
    virtual SkMatrix44 totalMatrix() const;

    sk_sp<sksg::Matrix<SkMatrix44>> fMatrixNode;

    using INHERITED = SkRefCnt;
};

} // namespace internal
} // namespace skottie

#endif // SkottieTransform_DEFINED
