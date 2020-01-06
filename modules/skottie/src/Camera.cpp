/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/Camera.h"

#include "include/utils/Sk3D.h"
#include "modules/sksg/include/SkSGTransform.h"

namespace skottie {
namespace internal {

CameraAdapter:: CameraAdapter(const SkSize& viewport_size, Type type)
    : fViewportSize(viewport_size)
    , fType(type)
{}

CameraAdapter::~CameraAdapter() = default;

sk_sp<CameraAdapter> CameraAdapter::MakeDefault(const SkSize &viewport_size) {
    auto adapter = sk_make_sp<CameraAdapter>(viewport_size, Type::kOneNode);

    static constexpr float kDefaultAEZoom = 879.13f;
    const auto center = SkVector::Make(viewport_size.width()  * 0.5f,
                                       viewport_size.height() * 0.5f);
    adapter->setZoom(kDefaultAEZoom);
    adapter->setPosition   (TransformAdapter3D::Vec3({center.fX, center.fY, -kDefaultAEZoom}));

    return adapter;
}

SkPoint3 CameraAdapter::poi() const {
    // AE supports two camera types:
    //
    //   - one-node camera: does not auto-orient, and starts off perpendicular to the z = 0 plane,
    //     facing "forward" (decreasing z).
    //
    //   - two-node camera: has a point of interest (encoded as the anchor point), and auto-orients
    //                      to point in its direction.
    return fType == Type::kOneNode
            ? SkPoint3{ this->getPosition().fX,
                        this->getPosition().fY,
                       -this->getPosition().fZ - 1 }
            : SkPoint3{ this->getAnchorPoint().fX,
                        this->getAnchorPoint().fY,
                       -this->getAnchorPoint().fZ};
}

SkMatrix44 CameraAdapter::totalMatrix() const {
    // Camera parameters:
    //
    //   * location          -> position attribute
    //   * point of interest -> anchor point attribute (two-node camera only)
    //   * orientation       -> rotation attribute
    //
    const auto pos = SkPoint3{ this->getPosition().fX,
                               this->getPosition().fY,
                              -this->getPosition().fZ },
                up = SkPoint3{ 0, 1, 0 };

    // Initial camera vector.
    SkMatrix44 cam_t;
    Sk3LookAt(&cam_t, pos, this->poi(), up);

    // Rotation origin is camera position.
    {
        SkMatrix44 rot;
        rot.setRotateDegreesAbout(1, 0, 0,  this->getRotation().fX);
        cam_t.postConcat(rot);
        rot.setRotateDegreesAbout(0, 1, 0,  this->getRotation().fY);
        cam_t.postConcat(rot);
        rot.setRotateDegreesAbout(0, 0, 1, -this->getRotation().fZ);
        cam_t.postConcat(rot);
    }

    // Flip world Z, as it is opposite of what Sk3D expects.
    cam_t.preScale(1, 1, -1);

    // View parameters:
    //
    //   * size     -> composition size (TODO: AE seems to base it on width only?)
    //   * distance -> "zoom" camera attribute
    //
    const auto view_size     = SkTMax(fViewportSize.width(), fViewportSize.height()),
               view_distance = this->getZoom(),
               view_angle    = std::atan(sk_ieee_float_divide(view_size * 0.5f, view_distance));

    SkMatrix44 persp_t;
    Sk3Perspective(&persp_t, 0, view_distance, 2 * view_angle);
    persp_t.postScale(view_size * 0.5f, view_size * 0.5f, 1);

    SkMatrix44 t;
    t.setTranslate(fViewportSize.width() * 0.5f, fViewportSize.height() * 0.5f, 0);
    t.preConcat(persp_t);
    t.preConcat(cam_t);

    return t;
}

} // namespace internal
} // namespace skottie
